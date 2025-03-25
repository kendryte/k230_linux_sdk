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

#ifndef __VVCAM_ISP_DRIVER_H__
#define __VVCAM_ISP_DRIVER_H__

#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/reset.h>

#include "vvcam_isp.h"
#include "vvcam_event.h"

#define VVCAM_ISP_NAME "vvcam-isp"

#define VVCAM_ISP_EVENT_ELEMS 10
#define VVCAM_VI_EVENT_ELEMS 10
struct vvcam_isp_dev {
    phys_addr_t paddr;
    uint32_t regs_size;
    void __iomem *base;
    // void __iomem *reset;
    struct reset_control* reset;
    int  id;
    int isp_irq;
    int fe_irq;
    int mi_irq;
    // int fusa_irq;
    struct miscdevice miscdev;
    struct device *dev;
    unsigned long pde;
    struct mutex mlock;
    struct vvcam_event_dev event_dev;
    uint32_t irq_mis[VVCAM_EID_MAX];
    struct tasklet_struct stat_tasklet;
    spinlock_t stat_lock;
    uint32_t refcnt;
};

struct vvcam_isp_fh {
    struct vvcam_isp_dev *isp_dev;
    struct vvcam_event_fh event_fh;
};

int vvcam_isp_reset(struct vvcam_isp_dev *isp_dev, uint32_t reset);
int vvcam_isp_write_reg(struct vvcam_isp_dev *isp_dev, vvcam_isp_reg_t isp_reg);
int vvcam_isp_read_reg(struct vvcam_isp_dev *isp_dev, vvcam_isp_reg_t *isp_reg);
irqreturn_t vvcam_isp_irq_process(struct vvcam_isp_dev *isp_dev);
irqreturn_t vvcam_isp_mi_irq_process(struct vvcam_isp_dev *isp_dev);
irqreturn_t vvcam_isp_fe_irq_process(struct vvcam_isp_dev *isp_dev);
irqreturn_t vvcam_isp_fusa_irq_process(struct vvcam_isp_dev *isp_dev);

#endif
