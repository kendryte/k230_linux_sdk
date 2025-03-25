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


#ifndef __VVCAM_ISP_AE_H__
#define __VVCAM_ISP_AE_H__

#include "vvcam_isp_ctrl.h"

#define VVCAM_ISP_CID_AE_ENABLE                     (VVCAM_ISP_CID_AE_BASE + 0x0000)
#define VVCAM_ISP_CID_AE_RESET                      (VVCAM_ISP_CID_AE_BASE + 0x0001)
#define VVCAM_ISP_CID_AE_STATE                      (VVCAM_ISP_CID_AE_BASE + 0x0002)
#define VVCAM_ISP_CID_AE_SEM_MODE                   (VVCAM_ISP_CID_AE_BASE + 0x0004)
#define VVCAM_ISP_CID_AE_FLICKER_PERIOD             (VVCAM_ISP_CID_AE_BASE + 0x0005)
#define VVCAM_ISP_CID_AE_SETPOINT                   (VVCAM_ISP_CID_AE_BASE + 0x0006)
#define VVCAM_ISP_CID_AE_TOLORENCE                  (VVCAM_ISP_CID_AE_BASE + 0x0007)
#define VVCAM_ISP_CID_AE_DAMPOVER                   (VVCAM_ISP_CID_AE_BASE + 0x0008)
#define VVCAM_ISP_CID_AE_DAMPOVER_RATIO             (VVCAM_ISP_CID_AE_BASE + 0x0009)
#define VVCAM_ISP_CID_AE_DAMPOVER_GAIN              (VVCAM_ISP_CID_AE_BASE + 0x000A)
#define VVCAM_ISP_CID_AE_DAMPUNDER                  (VVCAM_ISP_CID_AE_BASE + 0x000B)
#define VVCAM_ISP_CID_AE_DAMPUNDER_RATIO            (VVCAM_ISP_CID_AE_BASE + 0x000C)
#define VVCAM_ISP_CID_AE_DAMPUNDER_GAIN             (VVCAM_ISP_CID_AE_BASE + 0x000D)
#define VVCAM_ISP_CID_AE_MOTION_FILTER              (VVCAM_ISP_CID_AE_BASE + 0x000E)
#define VVCAM_ISP_CID_AE_MOTION_THRESHOLD           (VVCAM_ISP_CID_AE_BASE + 0x000F)
#define VVCAM_ISP_CID_AE_TARGET_FILTER              (VVCAM_ISP_CID_AE_BASE + 0x0010)
#define VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_REPRESS    (VVCAM_ISP_CID_AE_BASE + 0x0011)
#define VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_GAIN       (VVCAM_ISP_CID_AE_BASE + 0x0012)
#define VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_LEVEL      (VVCAM_ISP_CID_AE_BASE + 0x0013)
#define VVCAM_ISP_CID_AE_LOWLIGHT_HDR_REPRESS       (VVCAM_ISP_CID_AE_BASE + 0x0014)
#define VVCAM_ISP_CID_AE_LOWLIGHT_HDR_GAIN          (VVCAM_ISP_CID_AE_BASE + 0x0015)
#define VVCAM_ISP_CID_AE_LOWLIGHT_HDR_LEVEL         (VVCAM_ISP_CID_AE_BASE + 0x0016)
#define VVCAM_ISP_CID_AE_WDR_CONTRAST_MIN           (VVCAM_ISP_CID_AE_BASE + 0x0017)
#define VVCAM_ISP_CID_AE_WDR_CONTRAST_MAX           (VVCAM_ISP_CID_AE_BASE + 0x0018)
#define VVCAM_ISP_CID_AE_ROI_WEIGHT                 (VVCAM_ISP_CID_AE_BASE + 0x001A)
#define VVCAM_ISP_CID_AE_ROI                        (VVCAM_ISP_CID_AE_BASE + 0x001B)
#define VVCAM_ISP_CID_AE_HIST                       (VVCAM_ISP_CID_AE_BASE + 0x001C)
#define VVCAM_ISP_CID_AE_LUMA                       (VVCAM_ISP_CID_AE_BASE + 0x001D)
#define VVCAM_ISP_CID_AE_OBJECT_REGION              (VVCAM_ISP_CID_AE_BASE + 0x001E)
#define VVCAM_ISP_CID_AE_FRAME_CALC_ENABLE          (VVCAM_ISP_CID_AE_BASE + 0x001F)
#define VVCAM_ISP_CID_AE_EXPV2_WINDOW_WEIGHT        (VVCAM_ISP_CID_AE_BASE + 0x0020)
#define VVCAM_ISP_CID_EC_AGAIN                      (VVCAM_ISP_CID_AE_BASE + 0x0021)
#define VVCAM_ISP_CID_EC_AGAIN_MAX                  (VVCAM_ISP_CID_AE_BASE + 0x0022)
#define VVCAM_ISP_CID_EC_AGAIN_MIN                  (VVCAM_ISP_CID_AE_BASE + 0x0023)
#define VVCAM_ISP_CID_EC_AGAIN_STEP                 (VVCAM_ISP_CID_AE_BASE + 0x0024)
#define VVCAM_ISP_CID_EC_DGAIN                      (VVCAM_ISP_CID_AE_BASE + 0x0025)
#define VVCAM_ISP_CID_EC_DGAIN_MAX                  (VVCAM_ISP_CID_AE_BASE + 0x0026)
#define VVCAM_ISP_CID_EC_DGAIN_MIN                  (VVCAM_ISP_CID_AE_BASE + 0x0027)
#define VVCAM_ISP_CID_EC_DGAIN_STEP                 (VVCAM_ISP_CID_AE_BASE + 0x0028)
#define VVCAM_ISP_CID_EC_INTEGRATION_TIME           (VVCAM_ISP_CID_AE_BASE + 0x0029)
#define VVCAM_ISP_CID_EC_INTEGRATION_TIME_MAX       (VVCAM_ISP_CID_AE_BASE + 0x002A)
#define VVCAM_ISP_CID_EC_INTEGRATION_TIME_MIN       (VVCAM_ISP_CID_AE_BASE + 0x002B)
#define VVCAM_ISP_CID_EC_INTEGRATION_TIME_STEP      (VVCAM_ISP_CID_AE_BASE + 0x002C)
#define VVCAM_ISP_CID_AE_EXP_TABLE                  (VVCAM_ISP_CID_AE_BASE + 0x002D)
#define VVCAM_ISP_CID_AE_EXP_V3_ROI                 (VVCAM_ISP_CID_AE_BASE + 0x002E)
#define VVCAM_ISP_CID_AE_EXP_V3_ROI_WEIGHT          (VVCAM_ISP_CID_AE_BASE + 0x002F)
#define VVCAM_ISP_CID_AE_EXP_V3_ROI_CALC_WEIGHT     (VVCAM_ISP_CID_AE_BASE + 0x0030)
#define VVCAM_ISP_CID_AE_FRONT_GROUND_TYPE          (VVCAM_ISP_CID_AE_BASE + 0x0031)
#define VVCAM_ISP_CID_AE_FRONT_GROUND_FACE_WEIGHT   (VVCAM_ISP_CID_AE_BASE + 0x0032)
#define VVCAM_ISP_CID_AE_FRONT_GROUND_TOUCH_WEIGHT  (VVCAM_ISP_CID_AE_BASE + 0x0033)
#define VVCAM_ISP_CID_AE_EXP_DECOMP_CUSTOM          (VVCAM_ISP_CID_AE_BASE + 0x0034)
#define VVCAM_ISP_CID_AE_PERFORMANCE_OPTI_MODE      (VVCAM_ISP_CID_AE_BASE + 0x0035)

#ifdef __KERNEL__
int vvcam_isp_ae_ctrl_count(void);
int vvcam_isp_ae_ctrl_create(struct vvcam_isp_dev *isp_dev);
#endif

#endif
