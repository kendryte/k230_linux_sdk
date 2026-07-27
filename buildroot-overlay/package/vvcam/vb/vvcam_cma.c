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
#include <linux/dma-mapping.h>

#include "vvcam_cma.h"
#include "vvcam_vb_driver.h"

static int vvcam_cma_alloc(struct file *file,
            unsigned long *paddr, unsigned long size, char *name);
static int vvcam_cma_mmap(struct file *file, struct vm_area_struct *vma);
static int vvcam_cma_free(struct file *file, unsigned long paddr);
static int vvcam_cma_free_all(struct file *file);

static void *vvcam_alloc_cma_buf_attr(struct vvcam_cma *cma)
{
    return devm_kzalloc(cma->dev, sizeof(struct vvcam_cma_buf), GFP_KERNEL);
}

static void vvcam_free_cma_buf_attr(struct vvcam_cma *cma,
            struct vvcam_cma_buf *cma_buf)
{
    devm_kfree(cma->dev, cma_buf);
}

int vvcam_cma_create(void *allocator, struct device *dev)
{
    struct vvcam_cma *cma;
    struct vvcam_vb_mm_allocator *pallocator =
                (struct vvcam_vb_mm_allocator *)allocator;
    int ret = 0;

    cma = (struct vvcam_cma *)devm_kzalloc(dev, sizeof(struct vvcam_cma), GFP_KERNEL);
    if (!cma)
        return -ENOMEM;
    cma->dev = dev;
    
    // if (!dev->dma_mask)
    //     dev->dma_mask = &dev->coherent_dma_mask;
    // ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(32));
    // if (ret) {
    //     return ret;
    // }

    INIT_LIST_HEAD(&cma->buf_list);

    pallocator->mm_dev = cma;
    pallocator->alloc = vvcam_cma_alloc;
    pallocator->mmap = vvcam_cma_mmap;
    pallocator->free = vvcam_cma_free;
    pallocator->free_all = vvcam_cma_free_all;
    return 0;
}

int vvcam_cma_alloc(struct file *file,
             unsigned long *paddr, unsigned long size, char *name)
{
    struct vvcam_cma *cma;
    struct vvcam_vb_dev *vb_dev;
    struct vvcam_cma_buf *cma_buf = NULL;
    unsigned long mcm_size = 0;

    vb_dev = file->private_data;
    cma = vb_dev->allocator.mm_dev;

    mcm_size = size ;//+ (1024*1024);

    cma_buf = vvcam_alloc_cma_buf_attr(cma);
    if (!cma_buf)
        return -ENOMEM;

    
    cma_buf->virt_addr = dma_alloc_coherent(cma->dev, mcm_size, &(cma_buf->phys_addr), GFP_KERNEL | GFP_DMA);
    
    if (!cma_buf->virt_addr) {
        vvcam_free_cma_buf_attr(cma, cma_buf);
        return -ENOMEM;
    }

    cma_buf->size = mcm_size;
    strncpy(cma_buf->name, name, sizeof(cma_buf->name));
    cma_buf->private = file;
    list_add_tail(&cma_buf->list, &cma->buf_list);

    *paddr = cma_buf->phys_addr;

    printk("vvcam_cma_alloc *paddr is %x  size is %ld \n", *paddr, mcm_size);
    
    return 0;
}


static const struct vm_operations_struct mmap_mem_ops = {
#ifdef CONFIG_HAVE_IOREMAP_PROT
	.access = generic_access_phys
#endif
};


static int vvcam_cma_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct device *dev = file->private_data;
    unsigned long size = 0;
    struct vvcam_vb_dev *vb_dev;
    struct vvcam_cma *cma;
    struct vvcam_cma_buf *cma_buf = NULL;
#if 1
    vb_dev = file->private_data;
    cma = vb_dev->allocator.mm_dev;

    if (list_empty(&cma->buf_list))
        return -1;

    size = vma->vm_end - vma->vm_start;

    list_for_each_entry(cma_buf, &cma->buf_list, list) {
        if (!cma_buf) {
            continue;
        }
        if ((cma_buf->phys_addr >> PAGE_SHIFT) == vma->vm_pgoff) {
            break;
        }
    }

    if (!cma_buf)
        return -EAGAIN;

    // vma->vm_pgoff = 0;

    // printk("vvcam_cma_mmap vma->vm_start is %lx vma->vm_end is %lx vma->vm_pgoff is %lx \n",
    //         vma->vm_start, vma->vm_end, vma->vm_pgoff);

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    vma->vm_ops = &mmap_mem_ops;
    

    // return dma_mmap_coherent(dev, vma, cma_buf->virt_addr,
    //         cma_buf->phys_addr, size);

    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}

    return  0;
#else 

    printk("vvcam_cma_mmap vma->vm_start is %lx vma->vm_end is %lx vma->vm_pgoff is %lx \n",
            vma->vm_start, vma->vm_end, vma->vm_pgoff);

    // 设置页保护标志（非缓存模式）
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    // 将物理地址映射到用户虚拟地址
    ret = dma_mmap_coherent(dev, vma, cma_buf->virt_addr, dma_handle);
    if (ret)
        return ret;

    return 0;

#endif 
}

static int vvcam_cma_free(struct file *file, unsigned long paddr)
{
    struct vvcam_vb_dev *vb_dev;
    struct vvcam_cma *cma;
    struct vvcam_cma_buf *cma_buf = NULL;

    vb_dev = file->private_data;
    cma = vb_dev->allocator.mm_dev;

    if (list_empty(&cma->buf_list))
        return -1;

    list_for_each_entry(cma_buf, &cma->buf_list, list) {
        if (cma_buf->phys_addr == paddr) {
            break;
        }
    }

    if (cma_buf == NULL)
        return -1;

    list_del(&cma_buf->list);
    dma_free_coherent(cma->dev, cma_buf->size,
                cma_buf->virt_addr, cma_buf->phys_addr);

    printk("size is %ld cma_buf->phys_addr is %llx \n",cma_buf->size, cma_buf->phys_addr);

    vvcam_free_cma_buf_attr(cma, cma_buf);

    return 0;
}

int vvcam_cma_free_all(struct file *file)
{
    struct vvcam_vb_dev *vb_dev;
    struct vvcam_cma *cma;
    struct vvcam_cma_buf *cma_buf = NULL, *tmp_cma_buf;

    vb_dev = file->private_data;
    cma = vb_dev->allocator.mm_dev;

    if (list_empty(&cma->buf_list))
        return 0;

    list_for_each_entry_safe(cma_buf, tmp_cma_buf, &cma->buf_list, list) {
        if (!cma)
            continue;
        if (cma_buf->private == file) {
            list_del(&cma_buf->list);
            dma_free_coherent(cma->dev, cma_buf->size,
                        cma_buf->virt_addr, cma_buf->phys_addr);
            vvcam_free_cma_buf_attr(cma, cma_buf);
        }
    }

    return 0;
}

int vvcam_cma_cache_sync(struct vvcam_cma *cma, struct file *file,
            unsigned long paddr, unsigned long size,
            enum dma_data_direction dir)
{
    struct vvcam_cma_buf *buf;
    unsigned long offset;

    if (!cma || !file || !paddr || !size)
        return -EINVAL;

    if (paddr + size < paddr)
        return -EINVAL;

    list_for_each_entry(buf, &cma->buf_list, list) {
        unsigned long buf_end;

        if (buf->private != file)
            continue;

        buf_end = buf->phys_addr + buf->size;
        if (paddr < buf->phys_addr || paddr >= buf_end)
            continue;

        offset = paddr - buf->phys_addr;
        if (size > buf->size - offset)
            return -EINVAL;

        if (dir == DMA_TO_DEVICE)
            dma_sync_single_for_device(cma->dev, buf->phys_addr + offset,
                           size, DMA_TO_DEVICE);
        else
            dma_sync_single_for_cpu(cma->dev, buf->phys_addr + offset,
                          size, DMA_FROM_DEVICE);

        return 0;
    }

    return -ENOENT;
}
