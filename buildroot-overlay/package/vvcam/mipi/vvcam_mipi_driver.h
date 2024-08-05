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

#ifndef __VVCAM_MIPI_DRIVER_H__
#define __VVCAM_MIPI_DRIVER_H__

#include "linux/reset.h"
#include <linux/miscdevice.h>
#include "vvcam_mipi.h"
#include "k230_vi.h"

#define VVCAM_MIPI_NAME "vvcam-mipi"

struct vvcam_mipi_dev {
    void __iomem *base;
    #if 0 // unused for DWC CSI
	void __iomem *tpg_reg;
	void __iomem *csi_ctrl_reg;
    void __iomem *csi_mux_reg;
    #endif
    int  id;
    uint8_t bus_width;
    struct miscdevice miscdev;
    struct device *dev;
    struct mutex mlock;
    vvcam_input_dev_attr_t mipi_cfg;
    int irq;
    struct reset_control* reset_csi;
    struct reset_control* reset_sensor;
};

int vvcam_mipi_reset(struct vvcam_mipi_dev *mipi_dev, uint32_t val);
int vvcam_mipi_default_cfg(struct vvcam_mipi_dev *mipi_dev);
int vvcam_mipi_set_cfg(struct vvcam_mipi_dev *mipi_dev,
                    vvcam_input_dev_attr_t mipi_cfg);
int vvcam_mipi_set_stream(struct vvcam_mipi_dev *mipi_dev, uint32_t enable);

#endif
