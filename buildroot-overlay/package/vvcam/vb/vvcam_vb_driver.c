/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/pm_runtime.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

#include "vvcam_vb_driver.h"
#include "vvcam_vb.h"
#include "vvcam_vb_platform.h"
#include "vvcam_mmz.h"
#include "vvcam_cma.h"
#include "vvcam_vb_procfs.h"

static int enable_cma = 1;

static int vvcam_vb_alloc(struct file *file, vvcam_vb_t *vb_mem)
{
    struct vvcam_vb_dev *vb_dev;

    vb_dev = file->private_data;
    if (!vb_dev->allocator.mm_dev || !vb_dev->allocator.alloc)
        return -EPERM;

    return vb_dev->allocator.alloc(file,
            &vb_mem->paddr, vb_mem->size, vb_mem->name);
}

static int vvcam_vb_free(struct file *file, vvcam_vb_t *vb_mem)
{
    struct vvcam_vb_dev *vb_dev;

    vb_dev = file->private_data;
    if (!vb_dev->allocator.mm_dev || !vb_dev->allocator.free)
        return -EPERM;

    return vb_dev->allocator.free(file, vb_mem->paddr);
}

static int vvcam_vb_cache_flush(struct file *file, vvcam_vb_t *vb_mem)
{
    struct vvcam_vb_dev *vb_dev = file->private_data;
    unsigned long pfn = __phys_to_pfn(vb_mem->paddr);
    struct page *page = pfn_to_page(pfn);
    unsigned long offset = offset_in_page(vb_mem->paddr);
    size_t size = PAGE_ALIGN(vb_mem->size);
    dma_addr_t dma_addr = 0;

    dma_addr = dma_map_page(vb_dev->dev, page, offset, size, DMA_TO_DEVICE);
    if (dma_mapping_error(vb_dev->dev, dma_addr)) {
        dev_err(vb_dev->dev, "ma map page failed.\n");
        return -EINVAL;
    }

    dma_sync_single_for_device(vb_dev->dev, dma_addr, size, DMA_TO_DEVICE);

    dma_unmap_page(vb_dev->dev, dma_addr, size, DMA_TO_DEVICE);

    return 0;
}

static int vvcam_vb_cache_invalid(struct file *file, vvcam_vb_t *vb_mem)
{
    struct vvcam_vb_dev *vb_dev = file->private_data;
    unsigned long pfn = __phys_to_pfn(vb_mem->paddr);
    struct page *page = pfn_to_page(pfn);
    unsigned long offset = offset_in_page(vb_mem->paddr);
    size_t size = PAGE_ALIGN(vb_mem->size);
    dma_addr_t dma_addr = 0;

    dma_addr = dma_map_page(vb_dev->dev, page, offset, size, DMA_FROM_DEVICE);
    if (dma_mapping_error(vb_dev->dev, dma_addr)) {
        dev_err(vb_dev->dev, "ma map page failed.\n");
        return -EINVAL;
    }

    dma_sync_single_for_cpu(vb_dev->dev, dma_addr, size, DMA_FROM_DEVICE);

    dma_unmap_page(vb_dev->dev, dma_addr, size, DMA_FROM_DEVICE);

    return 0;
}

static int vvcam_vb_open(struct inode *inode, struct file *file)
{
    struct vvcam_vb_dev *vb_dev;
    struct miscdevice *pmisc_dev = file->private_data;

    vb_dev = container_of(pmisc_dev, struct vvcam_vb_dev, miscdev);
    if (!vb_dev)
        return -ENOMEM;

    file->private_data = vb_dev;

    return 0;
}

static int vvcam_vb_release(struct inode *inode, struct file *file)
{
    struct vvcam_vb_dev *vb_dev;

    vb_dev = file->private_data;
    if (!vb_dev->allocator.mm_dev || !vb_dev->allocator.free_all)
        return -EPERM;

    mutex_lock(&vb_dev->mlock);
    vb_dev->allocator.free_all(file);
    mutex_unlock(&vb_dev->mlock);

    return 0;
}

static long vvcam_vb_ioctl(struct file *file,
                        unsigned int cmd, unsigned long arg)
{
    struct vvcam_vb_dev *vb_dev;
    vvcam_vb_t vb_mem;
    int ret = 0;

    vb_dev = file->private_data;

    mutex_lock(&vb_dev->mlock);

    switch(cmd) {
    case VVCAM_VB_ALLOC:
        ret = copy_from_user(&vb_mem, (void __user *)arg, sizeof(vb_mem));
        if (ret)
            break;
        ret = vvcam_vb_alloc(file, &vb_mem);
        if (ret)
            break;
        ret = copy_to_user((void __user *)arg, &vb_mem, sizeof(vb_mem));
        break;
    case VVCAM_VB_FREE:
        ret = copy_from_user(&vb_mem, (void __user *)arg, sizeof(vb_mem));
        if (ret)
            break;
        ret = vvcam_vb_free(file, &vb_mem);
        break;
    case VVCAM_VB_CACHE_FLUSH:
        ret = copy_from_user(&vb_mem, (void __user *)arg, sizeof(vb_mem));
        if (ret)
            break;
        ret = vvcam_vb_cache_flush(file, &vb_mem);
        break;
    case VVCAM_VB_CACHE_INVALID:
        ret = copy_from_user(&vb_mem, (void __user *)arg, sizeof(vb_mem));
        if (ret)
            break;
        ret = vvcam_vb_cache_invalid(file, &vb_mem);
        break;
    default:
        ret = -EINVAL;
        break;
    }

    mutex_unlock(&vb_dev->mlock);

    return ret;
}

int vvcam_vb_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct vvcam_vb_dev *vb_dev;
    int ret = 0;

    vb_dev = file->private_data;
    if (!vb_dev->allocator.mm_dev || !vb_dev->allocator.mmap)
        return -EPERM;

    mutex_lock(&vb_dev->mlock);
    ret = vb_dev->allocator.mmap(file, vma);
    mutex_unlock(&vb_dev->mlock);

    return ret;
}

static struct file_operations vvcam_vb_fops = {
    .owner          = THIS_MODULE,
	.open           = vvcam_vb_open,
	.release        = vvcam_vb_release,
	.unlocked_ioctl = vvcam_vb_ioctl,
    .mmap           = vvcam_vb_mmap,
};

static int vvcam_vb_create_mm_allocator(struct platform_device *pdev,
        struct vvcam_vb_mm_allocator *allocator, const int type)
{
    int ret = 0;
    struct resource *res;

    switch (type)
    {
    case VVCAM_VB_MMZ:
        res =  platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!res) {
            dev_err(&pdev->dev, "can't fetch device resource info\n");
            return -EIO;
        }

        ret = vvcam_mmz_create(allocator, &pdev->dev,
                                res->start, resource_size(res));
        if (ret) {
            dev_err(&pdev->dev, "failed to create mmz\n");
            return ret;
        }
        break;

    case VVCAM_VB_CMA:
        ret = vvcam_cma_create(allocator, &pdev->dev);
        if (ret) {
            dev_err(&pdev->dev, "failed to create mmz\n");
            return ret;
        }
        break;

    default:
        break;
    }
    return 0;
}

static int vvcam_vb_probe(struct platform_device *pdev)
{
    struct vvcam_vb_dev *vb_dev;
    int ret = 0;

    if (!pdev->dev.dma_mask) {
        pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
    }

    ret = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
    if (ret) {
        dev_err(&pdev->dev, "failed to setup dma.\n");
        return ret;
    }

    vb_dev = devm_kzalloc(&pdev->dev,
                sizeof(struct vvcam_vb_dev), GFP_KERNEL);
    if (!vb_dev)
        return -ENOMEM;

    if (!enable_cma) {
        vb_dev->type = VVCAM_VB_MMZ;
        ret = vvcam_vb_create_mm_allocator(pdev,
                &vb_dev->allocator, VVCAM_VB_MMZ);
        if (ret) {
            dev_err(&pdev->dev, "create vb mmz failed.\n");
            return ret;
        }
    } else {
        vb_dev->type = VVCAM_VB_CMA;
        ret = vvcam_vb_create_mm_allocator(pdev,
                &vb_dev->allocator, VVCAM_VB_CMA);
        if (ret) {
            dev_err(&pdev->dev, "create vb cma failed.\n");
            return ret;
        }
    }

    mutex_init(&vb_dev->mlock);
    vb_dev->dev = &pdev->dev;
    platform_set_drvdata(pdev, vb_dev);

    ret = vvcam_vb_procfs_register(vb_dev, &vb_dev->pde);
    if (ret) {
        dev_err(&pdev->dev, "vb failed to register procfs\n");
        return -EINVAL;
    }

    vb_dev->miscdev.minor = MISC_DYNAMIC_MINOR;
    vb_dev->miscdev.name  = VVCAM_VB_NAME;
    vb_dev->miscdev.fops  = &vvcam_vb_fops;

    ret = misc_register(&vb_dev->miscdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to register device\n");
        return -EINVAL;
    }

    dev_info(&pdev->dev, "vvcam vb probe success\n");
    return 0;

}

static int vvcam_vb_remove(struct platform_device *pdev)
{
    struct vvcam_vb_dev *vb_dev;
    vb_dev = platform_get_drvdata(pdev);

    vvcam_vb_procfs_unregister(&vb_dev->pde);

    misc_deregister(&vb_dev->miscdev);

    return 0;
}

static const struct of_device_id vvcam_vb_of_match[] = {
	{.compatible = "verisilicon,vb",},
	{ /* sentinel */ },
};

static struct platform_driver vvcam_vb_driver = {
	.probe  = vvcam_vb_probe,
	.remove = vvcam_vb_remove,
	.driver = {
		.name           = VVCAM_VB_NAME,
		.owner          = THIS_MODULE,
        .of_match_table = vvcam_vb_of_match,
	}
};

static int __init vvcam_vb_init_module(void)
{
    int ret;

    ret = platform_driver_register(&vvcam_vb_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register vb driver\n");
        return ret;
    }

    ret = vvcam_vb_platform_device_register();
    if (ret) {
		platform_driver_unregister(&vvcam_vb_driver);
		printk(KERN_ERR "Failed to register vvcam vb platform devices\n");
		return ret;
	}

    return ret;
}

static void __exit vvcam_vb_exit_module(void)
{
    platform_driver_unregister(&vvcam_vb_driver);
    vvcam_vb_platform_device_unregister();
}

module_init(vvcam_vb_init_module);
module_exit(vvcam_vb_exit_module);

module_param(enable_cma, int, 0644);

MODULE_DESCRIPTION("Verisilicon video buffer driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
