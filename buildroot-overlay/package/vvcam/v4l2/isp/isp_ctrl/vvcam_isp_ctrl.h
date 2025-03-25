/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2024 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (C) 2014 - 2024 Vivante Corporation
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#ifndef __VVCAM_ISP_CTRL_H__
#define  __VVCAM_ISP_CTRL_H__

#ifdef __KERNEL__
#include "vvcam_isp_driver.h"
#else
#include <linux/videodev2.h>
#endif

#define VVCAM_ISP_CID_AE_BASE       (V4L2_CID_USER_BASE + 0x2000)
#define VVCAM_ISP_CID_AWB_BASE      (V4L2_CID_USER_BASE + 0x2100)
#define VVCAM_ISP_CID_GC_BASE       (V4L2_CID_USER_BASE + 0x2200)
#define VVCAM_ISP_CID_2DNR_BASE     (V4L2_CID_USER_BASE + 0x2300)
#define VVCAM_ISP_CID_3DNR_BASE     (V4L2_CID_USER_BASE + 0x2400)
#define VVCAM_ISP_CID_BLS_BASE      (V4L2_CID_USER_BASE + 0x2500)
#define VVCAM_ISP_CID_CCM_BASE      (V4L2_CID_USER_BASE + 0x2600)
#define VVCAM_ISP_CID_CNR_BASE      (V4L2_CID_USER_BASE + 0x2700)
#define VVCAM_ISP_CID_CPD_BASE      (V4L2_CID_USER_BASE + 0x2800)
#define VVCAM_ISP_CID_CPROC_BASE    (V4L2_CID_USER_BASE + 0x2900)
#define VVCAM_ISP_CID_DG_BASE       (V4L2_CID_USER_BASE + 0x2A00)
#define VVCAM_ISP_CID_DMSC_BASE     (V4L2_CID_USER_BASE + 0x2B00)
#define VVCAM_ISP_CID_DPCC_BASE     (V4L2_CID_USER_BASE + 0x2C00)
#define VVCAM_ISP_CID_DPF_BASE      (V4L2_CID_USER_BASE + 0x2D00)
#define VVCAM_ISP_CID_EE_BASE       (V4L2_CID_USER_BASE + 0x2E00)
#define VVCAM_ISP_CID_GE_BASE       (V4L2_CID_USER_BASE + 0x2F00)
#define VVCAM_ISP_CID_GTM_BASE      (V4L2_CID_USER_BASE + 0x3000)
#define VVCAM_ISP_CID_HDR_BASE      (V4L2_CID_USER_BASE + 0x3100)
#define VVCAM_ISP_CID_LSC_BASE      (V4L2_CID_USER_BASE + 0x3200)
#define VVCAM_ISP_CID_LUT3D_BASE    (V4L2_CID_USER_BASE + 0x3300)
#define VVCAM_ISP_CID_PDAF_BASE     (V4L2_CID_USER_BASE + 0x3400)
#define VVCAM_ISP_CID_AF_BASE       (V4L2_CID_USER_BASE + 0x3500)
#define VVCAM_ISP_CID_RGBIR_BASE    (V4L2_CID_USER_BASE + 0x3600)
#define VVCAM_ISP_CID_WB_BASE       (V4L2_CID_USER_BASE + 0x3700)
#define VVCAM_ISP_CID_WDR_BASE      (V4L2_CID_USER_BASE + 0x3800)
#define VVCAM_ISP_CID_YNR_BASE      (V4L2_CID_USER_BASE + 0x3900)
#define VVCAM_ISP_CID_AFM_BASE      (V4L2_CID_USER_BASE + 0x3A00)
#define VVCAM_ISP_CID_EXP_BASE      (V4L2_CID_USER_BASE + 0x3B00)
#define VVCAM_ISP_CID_GWDR_BASE     (V4L2_CID_USER_BASE + 0x3C00)
#define VVCAM_ISP_CID_EXP_V3_BASE   (V4L2_CID_USER_BASE + 0x3D00)

#ifdef __KERNEL__
int vvcam_isp_ctrl_init(struct vvcam_isp_dev *isp_dev);
int vvcam_isp_ctrl_destroy(struct vvcam_isp_dev *isp_dev);
#endif

#endif
