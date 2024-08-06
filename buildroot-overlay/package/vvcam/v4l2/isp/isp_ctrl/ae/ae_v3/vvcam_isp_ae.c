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
#include "vvcam_isp_ae.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_ae_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AE_ENABLE:
        case VVCAM_ISP_CID_AE_RESET:
        case VVCAM_ISP_CID_AE_STATE:
        case VVCAM_ISP_CID_AE_SEM_MODE:
        case VVCAM_ISP_CID_AE_FLICKER_PERIOD:
        case VVCAM_ISP_CID_AE_SETPOINT:
        case VVCAM_ISP_CID_AE_TOLORENCE:
        case VVCAM_ISP_CID_AE_DAMPOVER:
        case VVCAM_ISP_CID_AE_DAMPOVER_RATIO:
        case VVCAM_ISP_CID_AE_DAMPOVER_GAIN:
        case VVCAM_ISP_CID_AE_DAMPUNDER:
        case VVCAM_ISP_CID_AE_DAMPUNDER_RATIO:
        case VVCAM_ISP_CID_AE_DAMPUNDER_GAIN:
        case VVCAM_ISP_CID_AE_MOTION_FILTER:
        case VVCAM_ISP_CID_AE_MOTION_THRESHOLD:
        case VVCAM_ISP_CID_AE_TARGET_FILTER:
        case VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_REPRESS:
        case VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_GAIN:
        case VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_LEVEL:
        case VVCAM_ISP_CID_AE_LOWLIGHT_HDR_REPRESS:
        case VVCAM_ISP_CID_AE_LOWLIGHT_HDR_GAIN:
        case VVCAM_ISP_CID_AE_LOWLIGHT_HDR_LEVEL:
        case VVCAM_ISP_CID_AE_WDR_CONTRAST_MIN:
        case VVCAM_ISP_CID_AE_WDR_CONTRAST_MAX:
        case VVCAM_ISP_CID_AE_ROI_WEIGHT:
        case VVCAM_ISP_CID_AE_ROI:
        case VVCAM_ISP_CID_AE_HIST:
        case VVCAM_ISP_CID_AE_LUMA:
        case VVCAM_ISP_CID_AE_OBJECT_REGION:
        case VVCAM_ISP_CID_AE_FRAME_CALC_ENABLE:
        case VVCAM_ISP_CID_AE_EXPV2_WINDOW_WEIGHT:
        case VVCAM_ISP_CID_EC_AGAIN:
        case VVCAM_ISP_CID_EC_AGAIN_MAX:
        case VVCAM_ISP_CID_EC_AGAIN_MIN:
        case VVCAM_ISP_CID_EC_AGAIN_STEP:
        case VVCAM_ISP_CID_EC_DGAIN:
        case VVCAM_ISP_CID_EC_DGAIN_MAX:
        case VVCAM_ISP_CID_EC_DGAIN_MIN:
        case VVCAM_ISP_CID_EC_DGAIN_STEP:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME_MAX:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME_MIN:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME_STEP:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_ae_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AE_ENABLE:
        case VVCAM_ISP_CID_AE_RESET:
        case VVCAM_ISP_CID_AE_STATE:
        case VVCAM_ISP_CID_AE_SEM_MODE:
        case VVCAM_ISP_CID_AE_FLICKER_PERIOD:
        case VVCAM_ISP_CID_AE_SETPOINT:
        case VVCAM_ISP_CID_AE_TOLORENCE:
        case VVCAM_ISP_CID_AE_DAMPOVER:
        case VVCAM_ISP_CID_AE_DAMPOVER_RATIO:
        case VVCAM_ISP_CID_AE_DAMPOVER_GAIN:
        case VVCAM_ISP_CID_AE_DAMPUNDER:
        case VVCAM_ISP_CID_AE_DAMPUNDER_RATIO:
        case VVCAM_ISP_CID_AE_DAMPUNDER_GAIN:
        case VVCAM_ISP_CID_AE_MOTION_FILTER:
        case VVCAM_ISP_CID_AE_MOTION_THRESHOLD:
        case VVCAM_ISP_CID_AE_TARGET_FILTER:
        case VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_REPRESS:
        case VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_GAIN:
        case VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_LEVEL:
        case VVCAM_ISP_CID_AE_LOWLIGHT_HDR_REPRESS:
        case VVCAM_ISP_CID_AE_LOWLIGHT_HDR_GAIN:
        case VVCAM_ISP_CID_AE_LOWLIGHT_HDR_LEVEL:
        case VVCAM_ISP_CID_AE_WDR_CONTRAST_MIN:
        case VVCAM_ISP_CID_AE_WDR_CONTRAST_MAX:
        case VVCAM_ISP_CID_AE_ROI_WEIGHT:
        case VVCAM_ISP_CID_AE_ROI:
        case VVCAM_ISP_CID_AE_HIST:
        case VVCAM_ISP_CID_AE_LUMA:
        case VVCAM_ISP_CID_AE_OBJECT_REGION:
        case VVCAM_ISP_CID_AE_FRAME_CALC_ENABLE:
        case VVCAM_ISP_CID_AE_EXPV2_WINDOW_WEIGHT:
        case VVCAM_ISP_CID_EC_AGAIN:
        case VVCAM_ISP_CID_EC_AGAIN_MAX:
        case VVCAM_ISP_CID_EC_AGAIN_MIN:
        case VVCAM_ISP_CID_EC_AGAIN_STEP:
        case VVCAM_ISP_CID_EC_DGAIN:
        case VVCAM_ISP_CID_EC_DGAIN_MAX:
        case VVCAM_ISP_CID_EC_DGAIN_MIN:
        case VVCAM_ISP_CID_EC_DGAIN_STEP:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME_MAX:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME_MIN:
        case VVCAM_ISP_CID_EC_INTEGRATION_TIME_STEP:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_ae_ctrl_ops = {
	.s_ctrl = vvcam_isp_ae_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_ae_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_ae_ctrls[] = {
    {
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_STATE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_state",
        .step = 1,
        .min  = 0,
        .max  = 4,
    },
    {
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_SEM_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_sem_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_FLICKER_PERIOD,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_flicker_period",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        /* float (0,255) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_SETPOINT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_setpoint",
        .step = 1,
        .min  = 1,
        .max  = 254,
        .def  = 1,
    },
    {
        /* float (0,1) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_TOLORENCE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_tolerence",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        /* float (0,1) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_DAMPOVER,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_dampover",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        /* float (1,4) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_DAMPOVER_RATIO,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_dampover_ratio",
        .step = 1,
        .min  = 101,
        .max  = 399,
        .def  = 200,
    },
    {
        /* float [1,128) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_DAMPOVER_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_dampover_gain",
        .step = 1,
        .min  = 100,
        .max  = 12799,
        .def  = 100,
    },
    {
        /* float (0,1) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_DAMPUNDER,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_dampunder",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        /* float (0,1) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_DAMPUNDER_RATIO,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_dampunder_ratio",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        /* float [1,16) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_DAMPUNDER_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_dampunder_gain",
        .step = 1,
        .min  = 100,
        .max  = 1599,
        .def  = 100,
    },
    {
        /* float (0,1) 32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_MOTION_FILTER,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_motion_filter",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        /* float (0,1) 32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_MOTION_THRESHOLD,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_motion_threshold",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        /* float (0,1) 32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_TARGET_FILTER,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_target_filter",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        /* float array 8x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_REPRESS,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_lowlight_linear_repress",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {8},
    },
    {
        /* float array 8x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_lowlight_linear_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {8},
    },
    {
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_LOWLIGHT_LINEAR_LEVEL,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_lowlight_linear_level",
        .step = 1,
        .min  = 0,
        .max  = 16,
    },
    {
        /* float array 8x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_LOWLIGHT_HDR_REPRESS,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_lowlight_hdr_repress",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {8},
    },
    {
        /* float array 8x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_LOWLIGHT_HDR_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_lowlight_hdr_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {8},
    },
    {
        /* int 0~16 */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_LOWLIGHT_HDR_LEVEL,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_lowlight_hdr_level",
        .step = 1,
        .min  = 0,
        .max  = 16,
    },
    {
        /* float (0,255) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_WDR_CONTRAST_MIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_wdr_contrast_min",
        .step = 1,
        .min  = 1,
        .max  = 254,
        .def  = 1,
    },
    {
        /* float (0,255) */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_WDR_CONTRAST_MAX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_wdr_contrast_max",
        .step = 1,
        .min  = 1,
        .max  = 254,
        .def  = 1,
    },
    {
        /* float 25x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_ROI_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_roi_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {25},
    },
    {
        /* int 25x4*16bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_ROI,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_roi",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFF,
        .dims = {25, 4, 0, 0},
    },
    {
        /* uint32_t array 256*32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_HIST,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_hist",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {256},
    },
    {
        /* uint8_t array 32*32*8bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_LUMA,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_luma",
        .step = 1,
        .min  = 0,
        .max  = 0xFF,
        .dims = {1024},
    },
    {
        /* uint8_t array 32*32*8bit*/
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_OBJECT_REGION,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_object_region",
        .step = 1,
        .min  = 0,
        .max  = 0xFF,
        .dims = {1024},
    },
    {
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_FRAME_CALC_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_frame_calc_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* float 32x32x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_AE_EXPV2_WINDOW_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ae_exp_v2_window_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {1024},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_AGAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_again",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_AGAIN_MAX,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_again_max",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_AGAIN_MIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_again_min",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_AGAIN_STEP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_again_step",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_DGAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_dgain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_DGAIN_MAX,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_dgain_max",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_DGAIN_MIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_dgain_min",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_DGAIN_STEP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_dgain_step",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_INTEGRATION_TIME,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_integration_time",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_INTEGRATION_TIME_MAX,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_integration_time_max",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_INTEGRATION_TIME_MIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_integration_time_min",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
    {
        /* float 4x32bit */
        .ops  = &vvcam_isp_ae_ctrl_ops,
        .id   = VVCAM_ISP_CID_EC_INTEGRATION_TIME_STEP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ec_integration_time_step",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4},
    },
};

int vvcam_isp_ae_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_ae_ctrls);
}

int vvcam_isp_ae_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_ae_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_ae_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp ae ctrl %s failed %d.\n",
                vvcam_isp_ae_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

