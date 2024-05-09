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

#include "vvcam_isp_driver.h"
#include "vvcam_isp.h"
#include "vvcam_event.h"
#include "vvcam_isp_procfs.h"

extern void vvcam_isp_irq_stat_tasklet(unsigned long);

static irqreturn_t vvcam_isp_irq_handler(int irq, void *isp_dev)
{
    return vvcam_isp_irq_process(isp_dev);
}

static irqreturn_t vvcam_isp_mi_irq_handler(int irq, void *isp_dev)
{
    return vvcam_isp_mi_irq_process(isp_dev);
}

static irqreturn_t vvcam_isp_fe_irq_handler(int irq, void *isp_dev)
{
    return vvcam_isp_fe_irq_process(isp_dev);
}

static int vvcam_isp_open(struct inode *inode, struct file *file)
{
    struct miscdevice *pmisc_dev = file->private_data;
    struct vvcam_isp_dev *isp_dev;
    struct vvcam_isp_fh *isp_fh;

    isp_dev = container_of(pmisc_dev, struct vvcam_isp_dev, miscdev);
    if (!isp_dev)
        return -ENOMEM;

    dev_info(isp_dev->dev, "%s:%d\n", __func__, __LINE__);

    isp_fh = kzalloc(sizeof(struct vvcam_isp_fh), GFP_KERNEL);
    if (!isp_fh)
        return -ENOMEM;

    isp_fh->isp_dev = isp_dev;
    vvcam_event_fh_init( &isp_dev->event_dev, &isp_fh->event_fh);
    file->private_data = isp_fh;

    mutex_lock(&isp_dev->mlock);
    isp_dev->refcnt++;
    pm_runtime_get_sync(isp_dev->dev);
    mutex_unlock(&isp_dev->mlock);

    return 0;
}

static int vvcam_isp_release(struct inode *inode, struct file *file)
{
    struct vvcam_isp_dev *isp_dev;
    struct vvcam_isp_fh *isp_fh;

    isp_fh = file->private_data;
    isp_dev = isp_fh->isp_dev;

    dev_info(isp_dev->dev, "%s:%d\n", __func__, __LINE__);

    vvcam_event_unsubscribe_all(&isp_fh->event_fh);

    vvcam_event_fh_destroy(&isp_dev->event_dev, &isp_fh->event_fh);

    mutex_lock(&isp_dev->mlock);
    isp_dev->refcnt--;
    pm_runtime_put(isp_dev->dev);
    mutex_unlock(&isp_dev->mlock);

    kfree(isp_fh);

    return 0;
}

static long vvcam_isp_ioctl(struct file *file,
                        unsigned int cmd, unsigned long arg)
{
    struct vvcam_isp_dev *isp_dev;
    struct vvcam_isp_fh *isp_fh;
    uint32_t reset;
    vvcam_isp_reg_t isp_reg;
    vvcam_subscription_t sub;
    vvcam_event_t event;
    int ret = 0;

    isp_fh = file->private_data;
    isp_dev = isp_fh->isp_dev;

    mutex_lock(&isp_dev->mlock);

    switch(cmd) {
    case VVCAM_ISP_RESET:
        ret = copy_from_user(&reset, (void __user *)arg, sizeof(reset));
        if (ret)
            break;
        ret = vvcam_isp_reset(isp_dev, reset);
        break;
    case VVCAM_ISP_READ_REG:
        ret = copy_from_user(&isp_reg, (void __user *)arg, sizeof(isp_reg));
        if (ret)
            break;
        ret = vvcam_isp_read_reg(isp_dev, &isp_reg);
        if (ret)
            break;
        ret = copy_to_user((void __user *)arg, &isp_reg, sizeof(isp_reg));
        break;
    case VVCAM_ISP_WRITE_REG:
        ret = copy_from_user(&isp_reg, (void __user *)arg, sizeof(isp_reg));
        if (ret)
            break;
        ret = vvcam_isp_write_reg(isp_dev, isp_reg);
        break;
    case VVCAM_ISP_SUBSCRIBE_EVENT:
        ret = copy_from_user(&sub, (void __user *)arg, sizeof(sub));
        if (ret)
            break;
        ret = vvcam_event_subscribe(&isp_fh->event_fh,
                    &sub, VVCAM_ISP_EVENT_ELEMS);
        break;
    case VVCAM_ISP_UNSUBSCRIBE_EVENT:
        ret = copy_from_user(&sub, (void __user *)arg, sizeof(sub));
        if (ret)
            break;
        ret = vvcam_event_unsubscribe(&isp_fh->event_fh, &sub);
        break;
    case VVCAM_ISP_DQEVENT:
        ret = vvcam_event_dequeue(&isp_fh->event_fh, &event);
        if (ret)
            break;
        ret = copy_to_user((void __user *)arg, &event, sizeof(event));
        break;
    default:
        ret = -EINVAL;
        break;
    }

    mutex_unlock(&isp_dev->mlock);

    return ret;
}

static unsigned int vvcam_isp_poll(struct file *file, poll_table *wait)
{
    struct vvcam_isp_fh *isp_fh;

    isp_fh = file->private_data;

    return vvcam_event_poll(file, &isp_fh->event_fh, wait);
}

int vvcam_isp_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct vvcam_isp_dev *isp_dev;
    struct vvcam_isp_fh *isp_fh;

    isp_fh = file->private_data;
    isp_dev = isp_fh->isp_dev;

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    return vm_iomap_memory(vma, isp_dev->paddr, isp_dev->regs_size);
}

static struct file_operations vvcam_isp_fops = {
	.owner          = THIS_MODULE,
	.open           = vvcam_isp_open,
	.release        = vvcam_isp_release,
	.unlocked_ioctl = vvcam_isp_ioctl,
    .poll           = vvcam_isp_poll,
    .mmap           = vvcam_isp_mmap,

};

static int vvcam_isp_parse_params(struct vvcam_isp_dev *isp_dev,
                        struct platform_device *pdev)
{
    struct resource *res;

    res =  platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
		dev_err(&pdev->dev, "can't fetch device resource info\n");
		return -EIO;
	}
    isp_dev->paddr = res->start;
    isp_dev->regs_size = resource_size(res);
    dev_info(&pdev->dev, "isp addr: %08llx, size: %u\n", isp_dev->paddr, isp_dev->regs_size);
    isp_dev->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(isp_dev->base)) {
        dev_err(&pdev->dev, "can't remap device resource info\n");
        return PTR_ERR(isp_dev->base);
    }

    isp_dev->isp_irq = platform_get_irq(pdev, 0);
    if (isp_dev->isp_irq < 0) {
		dev_err(&pdev->dev, "can't get irq resource\n");
		return -ENXIO;
	} else {
        dev_info(&pdev->dev, "isp irq: %d\n", isp_dev->isp_irq);
    }

    isp_dev->mi_irq = platform_get_irq(pdev, 1);
    if (isp_dev->mi_irq < 0) {
		dev_err(&pdev->dev, "can't get mi irq resource\n");
		return -ENXIO;
	} else {
        dev_info(&pdev->dev, "mi irq: %d\n", isp_dev->mi_irq);
    }

    isp_dev->fe_irq = platform_get_irq(pdev, 2);
    if (isp_dev->fe_irq < 0) {
		dev_err(&pdev->dev, "can't get fe irq resource\n");
		return -ENXIO;
	} else {
        dev_info(&pdev->dev, "fe irq: %d\n", isp_dev->fe_irq);
    }

    isp_dev->reset = devm_reset_control_get(&pdev->dev, NULL);
    if (IS_ERR(isp_dev->reset)) {
        dev_err(&pdev->dev, "can't get device reset control\n");
        // ignore
        // return PTR_ERR(isp_dev->reset);
    }

    return 0;
}

static int vvcam_isp_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev;
    char *ispdev_name;

    isp_dev = devm_kzalloc(&pdev->dev,
                sizeof(struct vvcam_isp_dev), GFP_KERNEL);
    if (!isp_dev)
        return -ENOMEM;

    ret = vvcam_isp_parse_params(isp_dev, pdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to parse params\n");
        return -EINVAL;
    }

    mutex_init(&isp_dev->mlock);
    spin_lock_init(&isp_dev->stat_lock);
    vvcam_event_dev_init(&isp_dev->event_dev);
	platform_set_drvdata(pdev, isp_dev);

    isp_dev->dev = &pdev->dev;
    isp_dev->id  = pdev->id;

    ispdev_name = devm_kzalloc(&pdev->dev, 16, GFP_KERNEL);
    if (!ispdev_name)
        return -ENOMEM;
    snprintf(ispdev_name, 16, "%s.%d", VVCAM_ISP_NAME, 0/*pdev->id*/);

    isp_dev->miscdev.minor = MISC_DYNAMIC_MINOR;
    isp_dev->miscdev.name  = ispdev_name;
    isp_dev->miscdev.fops  = &vvcam_isp_fops;

    ret = misc_register(&isp_dev->miscdev);
    if (ret) {
        dev_err(&pdev->dev, "failed to register device\n");
        return -EINVAL;
    }

    ret = vvcam_isp_procfs_register(isp_dev, &isp_dev->pde);
    if (ret) {
        dev_err(&pdev->dev, "isp register procfs failed.\n");
        goto error_request_isp_irq;
    }

    tasklet_init(&isp_dev->stat_tasklet,
        vvcam_isp_irq_stat_tasklet, (unsigned long)isp_dev);

    ret = devm_request_irq(&pdev->dev, isp_dev->isp_irq, vvcam_isp_irq_handler,
                IRQF_TRIGGER_HIGH | IRQF_SHARED, dev_name(&pdev->dev), isp_dev);
    if (ret) {
        dev_err(&pdev->dev, "can't request isp irq %d\n", isp_dev->isp_irq);
        goto error_request_isp_irq;
    }

    ret = devm_request_irq(&pdev->dev, isp_dev->mi_irq, vvcam_isp_mi_irq_handler,
                IRQF_TRIGGER_HIGH | IRQF_SHARED, dev_name(&pdev->dev), isp_dev);
    if (ret) {
        dev_err(&pdev->dev, "can't request mi irq %d\n", isp_dev->mi_irq);
        goto error_request_mi_irq;
    }

    ret = devm_request_irq(&pdev->dev, isp_dev->fe_irq, vvcam_isp_fe_irq_handler,
                IRQF_TRIGGER_HIGH | IRQF_SHARED, dev_name(&pdev->dev), isp_dev);
    if (ret) {
        dev_err(&pdev->dev, "can't request fe irq %d\n", isp_dev->fe_irq);
        goto error_request_fe_irq;
    }

    dev_info(&pdev->dev, "vvcam isp driver probe success\n");

    return 0;

error_request_fe_irq:
    devm_free_irq(&pdev->dev, isp_dev->mi_irq, isp_dev);
error_request_mi_irq:
    devm_free_irq(&pdev->dev, isp_dev->isp_irq, isp_dev);;
error_request_isp_irq:
    misc_deregister(&isp_dev->miscdev);

    return ret;
}

static int vvcam_isp_remove(struct platform_device *pdev)
{
    struct vvcam_isp_dev *isp_dev;

    isp_dev = platform_get_drvdata(pdev);

    vvcam_isp_procfs_unregister(isp_dev->pde);

    misc_deregister(&isp_dev->miscdev);
    devm_free_irq(&pdev->dev, isp_dev->isp_irq, isp_dev);
    devm_free_irq(&pdev->dev, isp_dev->mi_irq, isp_dev);
    devm_free_irq(&pdev->dev, isp_dev->fe_irq, isp_dev);

    return 0;
}

static int vvcam_isp_system_suspend(struct device *dev)
{
    int ret = 0;
    ret = pm_runtime_force_suspend(dev);
    if (ret) {
        dev_err(dev, "force suspend %s failed\n", dev_name(dev));
        return ret;
    }
	return ret;
}

static int vvcam_isp_system_resume(struct device *dev)
{
    int ret = 0;
    ret = pm_runtime_force_resume(dev);
    if (ret) {
        dev_err(dev, "force resume %s failed\n", dev_name(dev));
        return ret;
    }
	return ret;
}

static int vvcam_isp_runtime_suspend(struct device *dev)
{
    return 0;
}

static int vvcam_isp_runtime_resume(struct device *dev)
{
    return 0;
}

static const struct dev_pm_ops vvcam_isp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(vvcam_isp_system_suspend, vvcam_isp_system_resume)
	SET_RUNTIME_PM_OPS(vvcam_isp_runtime_suspend, vvcam_isp_runtime_resume, NULL)
};

static const struct of_device_id vvcam_isp_of_match[] = {
	{.compatible = "verisilicon,isp",},
	{ /* sentinel */ },
};

static struct platform_driver vvcam_isp_driver = {
	.probe  = vvcam_isp_probe,
	.remove = vvcam_isp_remove,
	.driver = {
		.name           = VVCAM_ISP_NAME,
		.owner          = THIS_MODULE,
        .of_match_table = vvcam_isp_of_match,
        .pm             = &vvcam_isp_pm_ops,
	}
};

static int __init vvcam_isp_init_module(void)
{
    int ret;
    ret = platform_driver_register(&vvcam_isp_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register isp driver\n");
        return ret;
    }
#if 0
#ifdef VVCAM_PLATFORM_REGISTER
    ret = vvcam_isp_platform_device_register();
    if (ret) {
		platform_driver_unregister(&vvcam_isp_driver);
		printk(KERN_ERR "Failed to register vvcam isp platform devices\n");
		return ret;
	}
#endif
#endif
    return ret;
}

static void __exit vvcam_isp_exit_module(void)
{
    platform_driver_unregister(&vvcam_isp_driver);
#if 0
#ifdef VVCAM_PLATFORM_REGISTER
    vvcam_isp_platform_device_unregister();
#endif
#endif
}

module_init(vvcam_isp_init_module);
module_exit(vvcam_isp_exit_module);

MODULE_DESCRIPTION("Verisilicon isp driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
