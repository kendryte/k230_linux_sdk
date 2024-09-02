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


#include <media/v4l2-ioctl.h>
#include "vvcam_isp_driver.h"
#include "vvcam_isp_ctrl.h"
#include "vvcam_isp_awb.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_awb_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AWB_ENABLE:
        case VVCAM_ISP_CID_AWB_STATE:
        case VVCAM_ISP_CID_AWB_MODE:
        case VVCAM_ISP_CID_AWB_USE_CC_OFFSET:
        case VVCAM_ISP_CID_AWB_USE_CC_MATRIX:
        case VVCAM_ISP_CID_AWB_USE_DAMPING:
        case VVCAM_ISP_CID_AWB_ROI_WEIGHT:
        case VVCAM_ISP_CID_AWB_ROI:
        case VVCAM_ISP_CID_AWB_CONFIDENCE_THRESH:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_ENABLE:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_A:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_CWF:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_D65:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_ENABLE:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_RG:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_BG:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_THRESH:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_ENABLE:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_RG:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_BG:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_THRESH:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_ENABLE:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_RG:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_BG:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_THRESH:
        case VVCAM_ISP_CID_AWB_LIGHT_WEIGHT_ENABLE:
        case VVCAM_ISP_CID_AWB_LIGHT_WEIGHT_BRIGHT:
        case VVCAM_ISP_CID_AWB_LIGHT_WEIGHT:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_ENABLE:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_BRIGHT:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_R:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_B:
        case VVCAM_ISP_CID_AWB_USE_MANU_DAMP_COEFF:
        case VVCAM_ISP_CID_AWB_MANU_DAMP_COEFF:
        case VVCAM_ISP_CID_AWB_PERFORMANCE_OPTI_MODE:
        case VVCAM_ISP_CID_AWB_LOCK_THRESH:
        case VVCAM_ISP_CID_AWB_UNLOCK_THRESH:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_awb_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AWB_ENABLE:
        case VVCAM_ISP_CID_AWB_STATE:
        case VVCAM_ISP_CID_AWB_MODE:
        case VVCAM_ISP_CID_AWB_USE_CC_OFFSET:
        case VVCAM_ISP_CID_AWB_USE_CC_MATRIX:
        case VVCAM_ISP_CID_AWB_USE_DAMPING:
        case VVCAM_ISP_CID_AWB_ROI_WEIGHT:
        case VVCAM_ISP_CID_AWB_ROI:
        case VVCAM_ISP_CID_AWB_COLOR_TEMP_WEIGHT:
        case VVCAM_ISP_CID_AWB_CONFIDENCE_THRESH:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_ENABLE:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_A:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_CWF:
        case VVCAM_ISP_CID_AWB_TEMP_PREFER_D65:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_ENABLE:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_RG:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_BG:
        case VVCAM_ISP_CID_AWB_CONFOUND_CWF_THRESH:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_ENABLE:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_RG:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_BG:
        case VVCAM_ISP_CID_AWB_CONFOUND_TL84_THRESH:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_ENABLE:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_RG:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_BG:
        case VVCAM_ISP_CID_AWB_CONFOUND_D65_THRESH:
        case VVCAM_ISP_CID_AWB_LIGHT_WEIGHT_ENABLE:
        case VVCAM_ISP_CID_AWB_LIGHT_WEIGHT_BRIGHT:
        case VVCAM_ISP_CID_AWB_LIGHT_WEIGHT:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_ENABLE:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_BRIGHT:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_R:
        case VVCAM_ISP_CID_AWB_GRAY_PREFER_B:
        case VVCAM_ISP_CID_AWB_USE_MANU_DAMP_COEFF:
        case VVCAM_ISP_CID_AWB_MANU_DAMP_COEFF:
        case VVCAM_ISP_CID_AWB_PERFORMANCE_OPTI_MODE:
        case VVCAM_ISP_CID_AWB_LOCK_THRESH:
        case VVCAM_ISP_CID_AWB_UNLOCK_THRESH:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_awb_ctrl_ops = {
	.s_ctrl = vvcam_isp_awb_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_awb_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_awb_ctrls[] = {
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_STATE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_state",
        .step = 1,
        .min  = 0,
        .max  = 4,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_USE_CC_OFFSET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_use_cc_offset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_USE_CC_MATRIX,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_use_cc_matrix",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_USE_DAMPING,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_use_damping",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* float array 25x32bit 0~1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_ROI_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_roi_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {25},
    },
    {
        /* int array 25x4*16bit */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_ROI,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_roi",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFF,
        .dims = {25, 4, 0, 0},
    },
    {
        /* float array 7x32bit [0, 1] */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_COLOR_TEMP_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_color_temp_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {7},
    },
    {
        /* float array 10x32bit [0, 3] */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFIDENCE_THRESH,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confidence_thresh",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {10},
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_TEMP_PREFER_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_temp_prefer_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_TEMP_PREFER_A,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_temp_prefer_a",
        .step = 1,
        .min  = 1,
        .max  = 511,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_TEMP_PREFER_CWF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_temp_prefer_cwf",
        .step = 1,
        .min  = 1,
        .max  = 511,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_TEMP_PREFER_D65,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_temp_prefer_d65",
        .step = 1,
        .min  = 1,
        .max  = 511,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_CWF_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_cwf_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* float (0,4) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_CWF_RG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_cwf_rg",
        .step = 1,
        .min  = 1,
        .max  = 39999,
        .def  = 1,
    },
    {
        /* float (0,4) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_CWF_BG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_cwf_bg",
        .step = 1,
        .min  = 1,
        .max  = 39999,
        .def  = 1,
    },
    {
        /* float (0,3) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_CWF_THRESH,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_cwf_thresh",
        .step = 1,
        .min  = 1,
        .max  = 29999,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_TL84_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_tl84_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* float (0,4) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_TL84_RG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_tl84_rg",
        .step = 1,
        .min  = 1,
        .max  = 39999,
        .def  = 1,
    },
    {
        /* float (0,4) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_TL84_BG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_tl84_bg",
        .step = 1,
        .min  = 1,
        .max  = 39999,
        .def  = 1,
    },
    {
        /* float (0,3) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_TL84_THRESH,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_tl84_thresh",
        .step = 1,
        .min  = 1,
        .max  = 29999,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_D65_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_d65_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* float (0,4) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_D65_RG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_d65_rg",
        .step = 1,
        .min  = 1,
        .max  = 39999,
        .def  = 1,
    },
    {
        /* float (0,4) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_D65_BG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_d65_bg",
        .step = 1,
        .min  = 1,
        .max  = 39999,
        .def  = 1,
    },
    {
        /* float (0,3) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_CONFOUND_D65_THRESH,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_confound_d65_thresh",
        .step = 1,
        .min  = 1,
        .max  = 29999,
        .def  = 1,
    },
    {
        /* bool array 10x32bit */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_LIGHT_WEIGHT_ENABLE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_light_weight_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {10},
    },
    {
        /* float array 10x18x32bit [0, 17] */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_LIGHT_WEIGHT_BRIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_light_weight_bright",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {10, 18, 0, 0},
    },
    {
        /* float array 10x18x32bit [0, 1] */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_LIGHT_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_light_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {10, 18, 0, 0},
    },
    {
        /* bool array 10x32bit */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_GRAY_PREFER_ENABLE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_gray_prefer_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {10},
    },
    {
        /* float array 10x18x32bit [0, 17] */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_GRAY_PREFER_BRIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_gray_prefer_bright",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {10, 18, 0, 0},
    },
    {
        /* uint16_t array 10x18x16bit (0, 512) */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_GRAY_PREFER_R,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_gray_prefer_r",
        .step = 1,
        .min  = 1,
        .max  = 511,
        .def  = 1,
        .dims = {10, 18, 0, 0},
    },
    {
        /* uint16_t array 10x18x16bit (0, 512) */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_GRAY_PREFER_B,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_gray_prefer_b",
        .step = 1,
        .min  = 1,
        .max  = 511,
        .def  = 1,
        .dims = {10, 18, 0, 0},
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_USE_MANU_DAMP_COEFF,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_use_manu_damp_coeff",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* float (0, 1) ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_MANU_DAMP_COEFF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_manu_damp_coeff",
        .step = 1,
        .min  = 1,
        .max  = 9999,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_PERFORMANCE_OPTI_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_performance_opti_mode",
        .step = 1,
        .min  = 0,
        .max  = 2,
    },
    {
        /* float [0, 0.2] ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_LOCK_THRESH,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_lock_thresh",
        .step = 1,
        .min  = 0,
        .max  = 2000,
    },
    {
        /* float [0, 0.2] ratio 10000:1 */
        .ops  = &vvcam_isp_awb_ctrl_ops,
        .id   = VVCAM_ISP_CID_AWB_UNLOCK_THRESH,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_awb_unlock_thresh",
        .step = 1,
        .min  = 0,
        .max  = 2000,
    },
};

int vvcam_isp_awb_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_awb_ctrls);
}

int vvcam_isp_awb_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_awb_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_awb_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp awb ctrl %s failed %d.\n",
                vvcam_isp_awb_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

