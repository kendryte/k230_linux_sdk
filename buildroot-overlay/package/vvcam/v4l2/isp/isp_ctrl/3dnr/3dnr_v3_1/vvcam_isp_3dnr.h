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


#ifndef __VVCAM_ISP_3DNR_H__
#define __VVCAM_ISP_3DNR_H__

#include "vvcam_isp_ctrl.h"

#define VVCAM_ISP_CID_3DNR_ENABLE           (VVCAM_ISP_CID_3DNR_BASE + 0x0000)
#define VVCAM_ISP_CID_3DNR_GAMMA_FE_BE_ENABLE \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0001)
#define VVCAM_ISP_CID_3DNR_MOTION_DILATE_ENABLE \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0002)
#define VVCAM_ISP_CID_3DNR_MOTION_ERODE_ENABLE \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0003)
#define VVCAM_ISP_CID_3DNR_WORK_MODE        (VVCAM_ISP_CID_3DNR_BASE + 0x0004)
#define VVCAM_ISP_CID_3DNR_RESET            (VVCAM_ISP_CID_3DNR_BASE + 0x0005)
#define VVCAM_ISP_CID_3DNR_MODE             (VVCAM_ISP_CID_3DNR_BASE + 0x0006)
#define VVCAM_ISP_CID_3DNR_AUTO_LEVEL       (VVCAM_ISP_CID_3DNR_BASE + 0x0007)
#define VVCAM_ISP_CID_3DNR_AUTO_GAIN        (VVCAM_ISP_CID_3DNR_BASE + 0x0008)
#define VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN  (VVCAM_ISP_CID_3DNR_BASE + 0x0009)
#define VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN2 (VVCAM_ISP_CID_3DNR_BASE + 0x000A)
#define VVCAM_ISP_CID_3DNR_AUTO_NOISE_LEVEL (VVCAM_ISP_CID_3DNR_BASE + 0x000B)
#define VVCAM_ISP_CID_3DNR_AUTO_MOTION_SLOPE_THR \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x000C)
#define VVCAM_ISP_CID_3DNR_AUTO_SAD_WEIGHT  (VVCAM_ISP_CID_3DNR_BASE + 0x000D)
#define VVCAM_ISP_CID_3DNR_AUTO_MOTION_DILATE_EN \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x000E)
#define VVCAM_ISP_CID_3DNR_AUTO_MOTION_ERODE_EN \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x000F)
#define VVCAM_ISP_CID_3DNR_AUTO_GAMMA_FE_BE_EN \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0010)
#define VVCAM_ISP_CID_3DNR_AUTO_STRENGTH    (VVCAM_ISP_CID_3DNR_BASE + 0x0011)
#define VVCAM_ISP_CID_3DNR_AUTO_RANGE_H     (VVCAM_ISP_CID_3DNR_BASE + 0x0012)
#define VVCAM_ISP_CID_3DNR_AUTO_RANGE_V     (VVCAM_ISP_CID_3DNR_BASE + 0x0013)
#define VVCAM_ISP_CID_3DNR_AUTO_DILATE_H    (VVCAM_ISP_CID_3DNR_BASE + 0x0014)
#define VVCAM_ISP_CID_3DNR_AUTO_PRE_WEIGHT  (VVCAM_ISP_CID_3DNR_BASE + 0x0015)
#define VVCAM_ISP_CID_3DNR_AUTO_SIGMA       (VVCAM_ISP_CID_3DNR_BASE + 0x0016)
#define VVCAM_ISP_CID_3DNR_AUTO_BLEND_STATIC \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0017)
#define VVCAM_ISP_CID_3DNR_AUTO_BLEND_MOTION \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0018)
#define VVCAM_ISP_CID_3DNR_AUTO_BLEND_SLOPE (VVCAM_ISP_CID_3DNR_BASE + 0x0019)
#define VVCAM_ISP_CID_3DNR_MANU_DILATE_H    (VVCAM_ISP_CID_3DNR_BASE + 0x001A)
#define VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN  (VVCAM_ISP_CID_3DNR_BASE + 0x001B)
#define VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN2 (VVCAM_ISP_CID_3DNR_BASE + 0x001C)
#define VVCAM_ISP_CID_3DNR_MANU_NOISE_LEVEL (VVCAM_ISP_CID_3DNR_BASE + 0x001D)
#define VVCAM_ISP_CID_3DNR_MANU_PRE_WEIGHT  (VVCAM_ISP_CID_3DNR_BASE + 0x001E)
#define VVCAM_ISP_CID_3DNR_MANU_RANGE_H     (VVCAM_ISP_CID_3DNR_BASE + 0x001F)
#define VVCAM_ISP_CID_3DNR_MANU_RANGE_V     (VVCAM_ISP_CID_3DNR_BASE + 0x0020)
#define VVCAM_ISP_CID_3DNR_MANU_SAD_WEIGHT  (VVCAM_ISP_CID_3DNR_BASE + 0x0021)
#define VVCAM_ISP_CID_3DNR_MANU_STRENGTH    (VVCAM_ISP_CID_3DNR_BASE + 0x0022)
#define VVCAM_ISP_CID_3DNR_MANU_MOTION_SLOPE_THR \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0023)
#define VVCAM_ISP_CID_3DNR_MANU_BLEND_MOTION \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0024)
#define VVCAM_ISP_CID_3DNR_MANU_BLEND_SLOPE (VVCAM_ISP_CID_3DNR_BASE + 0x0025)
#define VVCAM_ISP_CID_3DNR_MANU_BLEND_STATIC \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0026)
#define VVCAM_ISP_CID_3DNR_MANU_SIGMA       (VVCAM_ISP_CID_3DNR_BASE + 0x0027)
#define VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_A \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0028)
#define VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_B \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0029)
#define VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_FIX_CURVE_START \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x002A)
#define VVCAM_ISP_CID_3DNR_STAT_MOTION_DILATE_ENABLE \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x002B)
#define VVCAM_ISP_CID_3DNR_STAT_MOTION_ERODE_ENABLE \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x002C)
#define VVCAM_ISP_CID_3DNR_STAT_WORK_MODE   (VVCAM_ISP_CID_3DNR_BASE + 0x002D)
#define VVCAM_ISP_CID_3DNR_STAT_DILATE_H    (VVCAM_ISP_CID_3DNR_BASE + 0x002E)
#define VVCAM_ISP_CID_3DNR_STAT_FILTER_LEN  (VVCAM_ISP_CID_3DNR_BASE + 0x002F)
#define VVCAM_ISP_CID_3DNR_STAT_FILTER_LEN2 (VVCAM_ISP_CID_3DNR_BASE + 0x0030)
#define VVCAM_ISP_CID_3DNR_STAT_NOISE_LEVEL (VVCAM_ISP_CID_3DNR_BASE + 0x0031)
#define VVCAM_ISP_CID_3DNR_STAT_PRE_WEIGHT  (VVCAM_ISP_CID_3DNR_BASE + 0x0032)
#define VVCAM_ISP_CID_3DNR_STAT_RANGE_H     (VVCAM_ISP_CID_3DNR_BASE + 0x0033)
#define VVCAM_ISP_CID_3DNR_STAT_RANGE_V     (VVCAM_ISP_CID_3DNR_BASE + 0x0034)
#define VVCAM_ISP_CID_3DNR_STAT_SAD_WEIGHT  (VVCAM_ISP_CID_3DNR_BASE + 0x0035)
#define VVCAM_ISP_CID_3DNR_STAT_STRENGTH    (VVCAM_ISP_CID_3DNR_BASE + 0x0036)
#define VVCAM_ISP_CID_3DNR_STAT_MOTION_SLOPE_THR \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0037)
#define VVCAM_ISP_CID_3DNR_STAT_BLEND_MOTION \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x0038)
#define VVCAM_ISP_CID_3DNR_STAT_BLEND_SLOPE (VVCAM_ISP_CID_3DNR_BASE + 0x0039)
#define VVCAM_ISP_CID_3DNR_STAT_BLEND_STATIC \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x003A)
#define VVCAM_ISP_CID_3DNR_STAT_SIGMA       (VVCAM_ISP_CID_3DNR_BASE + 0x003B)
#define VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_A \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x003C)
#define VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_B \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x003D)
#define VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_FIX_CURVE_START \
                                            (VVCAM_ISP_CID_3DNR_BASE + 0x003E)

#ifdef __KERNEL__
int vvcam_isp_3dnr_ctrl_count(void);
int vvcam_isp_3dnr_ctrl_create(struct vvcam_isp_dev *isp_dev);
#endif

#endif
