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
#include <linux/list.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/v4l2-ctrls.h>
#include <media/videobuf2-dma-contig.h>
#include "vvcam_v4l2_common.h"

#define VVCAM_ISP_NAME "vvcam-isp-subdev"

#define VVCAM_ISP_WIDTH_ALIGN 16
#define VVCAM_ISP_HEIGHT_ALIGN 8
#define VVCAM_ISP_WIDTH_MIN 32
#define VVCAM_ISP_HEIGHT_MIN 16

#define VVCAM_ISP_PORT_NR  4
enum vvcam_isp_port_pad_e {
	VVCAM_ISP_PORT_PAD_SINK = 0,
	VVCAM_ISP_PORT_PAD_SOURCE_MP,
	VVCAM_ISP_PORT_PAD_SOURCE_SP1,
	VVCAM_ISP_PORT_PAD_SOURCE_SP2,
	VVCAM_ISP_PORT_PAD_SOURCE_RAW,
	VVCAM_ISP_PORT_PAD_NR,
};

#define VVCAM_ISP_PAD_NR (VVCAM_ISP_PORT_NR * VVCAM_ISP_PORT_PAD_NR)

struct vvcam_isp_mbus_fmt {
	uint32_t code;
};

struct vvcam_isp_pad_data {
	uint32_t sink_detected;
	struct v4l2_mbus_framefmt format;
	struct v4l2_fract frmival_min;
    struct v4l2_fract frmival_max;
	uint32_t num_formats;
	struct vvcam_isp_mbus_fmt *mbus_fmt;
	struct list_head queue;
	spinlock_t qlock;
	uint32_t stream;
};

struct vvcam_isp_event_shm {
	struct mutex event_lock;
	uint64_t phy_addr;
	void *virt_addr;
	uint32_t size;
};

struct vvcam_isp_sensor_info {
	char sensor[32];
	uint8_t mode;
	char xml[64];
	char manu_json[128];
	char auto_json[128];
};

struct vvcam_isp_dev {
    phys_addr_t paddr;
    uint32_t regs_size;
    void __iomem *base;
    void __iomem *reset;
    int  id;
    int fe_irq;
    int isp_irq;
    int mi_irq;
    struct device *dev;
    struct mutex mlock;
    uint32_t refcnt;
    struct v4l2_subdev sd;
	struct media_pad pads[VVCAM_ISP_PAD_NR];
	struct v4l2_async_notifier notifier;
#ifdef VVCAM_PLATFORM_REGISTER
	struct fwnode_handle fwnode;
#endif
	struct vvcam_isp_pad_data pad_data[VVCAM_ISP_PAD_NR];

	struct vvcam_isp_event_shm event_shm;
	struct v4l2_ctrl_handler ctrl_handler;
	struct mutex ctrl_lock;
	uint32_t ctrl_pad;

	unsigned long pde;
	struct vvcam_isp_sensor_info sensor_info[VVCAM_ISP_PORT_NR];

};

#endif
