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
#include "vvcam_isp_wdr.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_wdr_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_WDR_ENABLE:
        case VVCAM_ISP_CID_WDR_RESET:
        case VVCAM_ISP_CID_WDR_MODE:
        case VVCAM_ISP_CID_WDR_AUTO_LEVEL:
        case VVCAM_ISP_CID_WDR_AUTO_GAIN:
        case VVCAM_ISP_CID_WDR_AUTO_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_HIGH_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_LOW_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_GLOBAL_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_CONTRAST:
        case VVCAM_ISP_CID_WDR_AUTO_FLAT_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_FLAT_THR:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_MANU_CONTRAST:
        case VVCAM_ISP_CID_WDR_MANU_GLOBAL_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_HIGH_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_LOW_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_THR:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY:
        case VVCAM_ISP_CID_WDR_MANU_GAMMA_DOWN:
        case VVCAM_ISP_CID_WDR_MANU_GAMMA_PRE:
        case VVCAM_ISP_CID_WDR_MANU_GAMMA_UP:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_wdr_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_WDR_ENABLE:
        case VVCAM_ISP_CID_WDR_RESET:
        case VVCAM_ISP_CID_WDR_MODE:
        case VVCAM_ISP_CID_WDR_AUTO_LEVEL:
        case VVCAM_ISP_CID_WDR_AUTO_GAIN:
        case VVCAM_ISP_CID_WDR_AUTO_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_HIGH_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_LOW_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_GLOBAL_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_CONTRAST:
        case VVCAM_ISP_CID_WDR_AUTO_FLAT_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_FLAT_THR:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_MANU_CONTRAST:
        case VVCAM_ISP_CID_WDR_MANU_GLOBAL_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_HIGH_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_LOW_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_THR:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY:
        case VVCAM_ISP_CID_WDR_MANU_GAMMA_DOWN:
        case VVCAM_ISP_CID_WDR_MANU_GAMMA_PRE:
        case VVCAM_ISP_CID_WDR_MANU_GAMMA_UP:
        case VVCAM_ISP_CID_WDR_STAT_CONTRAST:
        case VVCAM_ISP_CID_WDR_STAT_GLOBAL_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_HIGH_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_LOW_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_FLAT_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_FLAT_THR:
        case VVCAM_ISP_CID_WDR_STAT_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_STAT_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_STAT_ENTROPY:
        case VVCAM_ISP_CID_WDR_STAT_GAMMA_DOWN:
        case VVCAM_ISP_CID_WDR_STAT_GAMMA_PRE:
        case VVCAM_ISP_CID_WDR_STAT_GAMMA_UP:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_wdr_ctrl_ops = {
	.s_ctrl = vvcam_isp_wdr_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_wdr_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_wdr_ctrls[] = {
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_level",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 20,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_HIGH_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_high_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_LOW_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_low_strength",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_GLOBAL_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_global_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* int16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_CONTRAST,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_contrast",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_FLAT_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_flat_strength",
        .step = 1,
        .min  = 0,
        .max  = 19,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_FLAT_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_flat_thr",
        .step = 1,
        .min  = 0,
        .max  = 20,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_ENTROPY_SLOPE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_entropy_slope",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_ENTROPY_BASE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_entropy_base",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {20, 0, 0, 0},
    },

    /*
    manual control type is difference between cam_device and cam_engine
    and some code is not match with doc, need to double check.
    */
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_CONTRAST,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_contrast",
        .step = 1,
        .min  = -1023,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_GLOBAL_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_global_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_HIGH_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_high_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_LOW_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_low_strength",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_FLAT_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_flat_strength",
        .step = 1,
        .min  = 0,
        .max  = 19,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_FLAT_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_flat_thr",
        .step = 1,
        .min  = 0,
        .max  = 20,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_ENTROPY_BASE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_entropy_base",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_ENTROPY_SLOPE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_entropy_slope",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_ENTROPY,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_entropy",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_GAMMA_DOWN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_gamma_down",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_GAMMA_PRE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_gamma_pre",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_GAMMA_UP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_gamma_up",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_CONTRAST,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_contrast",
        .step = 1,
        .min  = -1023,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_GLOBAL_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_global_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_HIGH_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_high_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_LOW_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_low_strength",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_FLAT_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_flat_strength",
        .step = 1,
        .min  = 0,
        .max  = 19,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_FLAT_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_flat_thr",
        .step = 1,
        .min  = 0,
        .max  = 20,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_ENTROPY_BASE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_entropy_base",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_ENTROPY_SLOPE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_entropy_slope",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_ENTROPY,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_entropy",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_GAMMA_DOWN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_gamma_down",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_GAMMA_PRE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_gamma_pre",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_GAMMA_UP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_gamma_up",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {20, 0, 0, 0},
    },
};

int vvcam_isp_wdr_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_wdr_ctrls);
}

int vvcam_isp_wdr_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_wdr_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_wdr_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp wdr ctrl %s failed %d.\n",
                vvcam_isp_wdr_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;
}

