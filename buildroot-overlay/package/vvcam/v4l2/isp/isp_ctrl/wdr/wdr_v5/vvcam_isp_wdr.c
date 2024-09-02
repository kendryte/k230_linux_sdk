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
        case VVCAM_ISP_CID_WDR_GLOBAL_CURVE_ENABLE:
        case VVCAM_ISP_CID_WDR_RESET:
        case VVCAM_ISP_CID_WDR_MODE:
        case VVCAM_ISP_CID_WDR_AUTO_LEVEL:
        case VVCAM_ISP_CID_WDR_AUTO_GAIN:
        case VVCAM_ISP_CID_WDR_AUTO_H_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_L_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_CONTRAST:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_AUTO_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_AUTO_LOG_WEIGHT:
        case VVCAM_ISP_CID_WDR_AUTO_FIXED_WEIGHT:
        case VVCAM_ISP_CID_WDR_AUTO_MAX_GAIN:
        case VVCAM_ISP_CID_WDR_AUTO_SAT_RANGE:
        case VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_DOWN:
        case VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_UP:
        case VVCAM_ISP_CID_WDR_AUTO_DEGAMMA:
        case VVCAM_ISP_CID_WDR_MANU_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_H_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_L_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_CONTRAST:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_ENABLE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_MANU_LUMA_THR:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_MODE:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL_INC:
        case VVCAM_ISP_CID_WDR_MANU_DARK_ATT_LEVEL:
        case VVCAM_ISP_CID_WDR_MANU_FIXED_WEIGHT:
        case VVCAM_ISP_CID_WDR_MANU_CURVE_SELECT:
        case VVCAM_ISP_CID_WDR_MANU_LOG_WEIGHT:
        case VVCAM_ISP_CID_WDR_MANU_ANCHOR_X_LOG:
        case VVCAM_ISP_CID_WDR_MANU_ANCHOR_SLOPE:
        case VVCAM_ISP_CID_WDR_MANU_RATIO:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_THR:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_L_SAT:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_L_FAC:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_H_FAC:
        case VVCAM_ISP_CID_WDR_MANU_GLOBAL_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_MANU_DEGAMMA:
        case VVCAM_ISP_CID_WDR_MANU_MAX_GAIN:
        case VVCAM_ISP_CID_WDR_MANU_MIN_GAIN:
        case VVCAM_ISP_CID_WDR_MANU_H_DIFF:
        case VVCAM_ISP_CID_WDR_MANU_L_DIFF:
        case VVCAM_ISP_CID_WDR_MANU_SAT_RANGE:
        case VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_DOWN:
        case VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_UP:
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
        case VVCAM_ISP_CID_WDR_GLOBAL_CURVE_ENABLE:
        case VVCAM_ISP_CID_WDR_MODE:
        case VVCAM_ISP_CID_WDR_AUTO_LEVEL:
        case VVCAM_ISP_CID_WDR_AUTO_GAIN:
        case VVCAM_ISP_CID_WDR_AUTO_H_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_L_STRENGTH:
        case VVCAM_ISP_CID_WDR_AUTO_CONTRAST:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_AUTO_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_AUTO_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_AUTO_LOG_WEIGHT:
        case VVCAM_ISP_CID_WDR_AUTO_FIXED_WEIGHT:
        case VVCAM_ISP_CID_WDR_AUTO_MAX_GAIN:
        case VVCAM_ISP_CID_WDR_AUTO_SAT_RANGE:
        case VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_DOWN:
        case VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_UP:
        case VVCAM_ISP_CID_WDR_AUTO_DEGAMMA:
        case VVCAM_ISP_CID_WDR_MANU_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_H_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_L_STRENGTH:
        case VVCAM_ISP_CID_WDR_MANU_CONTRAST:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_ENABLE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_MANU_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_MANU_LUMA_THR:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_MODE:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL_INC:
        case VVCAM_ISP_CID_WDR_MANU_DARK_ATT_LEVEL:
        case VVCAM_ISP_CID_WDR_MANU_FIXED_WEIGHT:
        case VVCAM_ISP_CID_WDR_MANU_CURVE_SELECT:
        case VVCAM_ISP_CID_WDR_MANU_LOG_WEIGHT:
        case VVCAM_ISP_CID_WDR_MANU_ANCHOR_X_LOG:
        case VVCAM_ISP_CID_WDR_MANU_ANCHOR_SLOPE:
        case VVCAM_ISP_CID_WDR_MANU_RATIO:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_THR:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_L_SAT:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_L_FAC:
        case VVCAM_ISP_CID_WDR_MANU_CURVE2_H_FAC:
        case VVCAM_ISP_CID_WDR_MANU_GLOBAL_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_MANU_DEGAMMA:
        case VVCAM_ISP_CID_WDR_MANU_MAX_GAIN:
        case VVCAM_ISP_CID_WDR_MANU_MIN_GAIN:
        case VVCAM_ISP_CID_WDR_MANU_H_DIFF:
        case VVCAM_ISP_CID_WDR_MANU_L_DIFF:
        case VVCAM_ISP_CID_WDR_MANU_SAT_RANGE:
        case VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_DOWN:
        case VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_UP:
        case VVCAM_ISP_CID_WDR_STAT_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_H_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_L_STRENGTH:
        case VVCAM_ISP_CID_WDR_STAT_CONTRAST:
        case VVCAM_ISP_CID_WDR_STAT_ENTROPY_ENABLE:
        case VVCAM_ISP_CID_WDR_STAT_ENTROPY_BASE:
        case VVCAM_ISP_CID_WDR_STAT_ENTROPY_SLOPE:
        case VVCAM_ISP_CID_WDR_STAT_LUMA_THR:
        case VVCAM_ISP_CID_WDR_STAT_FLAT_MODE:
        case VVCAM_ISP_CID_WDR_STAT_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_STAT_FLAT_LEVEL_INC:
        case VVCAM_ISP_CID_WDR_STAT_DARK_ATT_LEVEL:
        case VVCAM_ISP_CID_WDR_STAT_FIXED_WEIGHT:
        case VVCAM_ISP_CID_WDR_STAT_CURVE_SELECT:
        case VVCAM_ISP_CID_WDR_STAT_LOG_WEIGHT:
        case VVCAM_ISP_CID_WDR_STAT_ANCHOR_X_LOG:
        case VVCAM_ISP_CID_WDR_STAT_ANCHOR_SLOPE:
        case VVCAM_ISP_CID_WDR_STAT_RATIO:
        case VVCAM_ISP_CID_WDR_STAT_CURVE2_THR:
        case VVCAM_ISP_CID_WDR_STAT_CURVE2_L_SAT:
        case VVCAM_ISP_CID_WDR_STAT_CURVE2_L_FAC:
        case VVCAM_ISP_CID_WDR_STAT_CURVE2_H_FAC:
        case VVCAM_ISP_CID_WDR_STAT_GLOBAL_FLAT_LEVEL:
        case VVCAM_ISP_CID_WDR_STAT_DEGAMMA:
        case VVCAM_ISP_CID_WDR_STAT_MAX_GAIN:
        case VVCAM_ISP_CID_WDR_STAT_MIN_GAIN:
        case VVCAM_ISP_CID_WDR_STAT_H_DIFF:
        case VVCAM_ISP_CID_WDR_STAT_L_DIFF:
        case VVCAM_ISP_CID_WDR_STAT_SAT_RANGE:
        case VVCAM_ISP_CID_WDR_STAT_SAT_THR_GAIN_DOWN:
        case VVCAM_ISP_CID_WDR_STAT_SAT_THR_GAIN_UP:
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
        .id   = VVCAM_ISP_CID_WDR_GLOBAL_CURVE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_global_curve_enable",
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
        .min  = 1,
        .max  = 20,
        .def  = 1,
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
        .id   = VVCAM_ISP_CID_WDR_AUTO_H_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_high_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20}
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_L_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_low_strength",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20}
    },
        {
        /* int16_t 20x array [-1023, 1023]*/
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
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_FLAT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_flat_level",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_LOG_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_log_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_FIXED_WEIGHT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_fixed_weight",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_MAX_GAIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_max_gain",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float 20x array [0.0 ~ 1.0] */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_SAT_RANGE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_sat_range",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_sat_thr_gain_down",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_sat_thr_gain_up",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float 20x array [0.0 ~ 3.0]*/
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_AUTO_DEGAMMA,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_auto_degamma",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
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
        .id   = VVCAM_ISP_CID_WDR_MANU_H_STRENGTH,
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
        .id   = VVCAM_ISP_CID_WDR_MANU_L_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_low_strength",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_CONTRAST,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_contrast",
        .step = 1,
        .min  = -1023,
        .max  = 1023,
        .def  = 500,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_ENTROPY_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_entropy_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
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
        .id   = VVCAM_ISP_CID_WDR_MANU_LUMA_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_luma_thr",
        .step = 1,
        .min  = 1,
        .max  = 1023,
        .def  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_FLAT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_flat_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_flat_level",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 4x17 array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL_INC,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_flat_level_inc",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {4, 17, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_DARK_ATT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_dark_atten_level",
        .step = 1,
        .min  = 0,
        .max  = 14,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_FIXED_WEIGHT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_fixed_weight",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_CURVE_SELECT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_curve_select",
        .step = 1,
        .min  = 0,
        .max  = 2,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_LOG_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_log_weight",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0.0 ~ 20.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_ANCHOR_X_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_anchor_x_log",
        .step = 1,
        .min  = 0,
        .max  = 2000,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_ANCHOR_SLOPE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_anchor_slope",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .def  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float (0.0, 2.0] */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_RATIO,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_actual_ratio",
        .step = 1,
        .min  = 1,
        .max  = 200,
        .def  = 1,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_CURVE2_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_curve2_threshold",
        .step = 1,
        .min  = 0,
        .max  = 100,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_CURVE2_L_SAT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_curve2_low_sat",
        .step = 1,
        .min  = 0,
        .max  = 100,
    },
    {
        /* float (0.0, 2.2] */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_CURVE2_L_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_curve2_low_fac",
        .step = 1,
        .min  = 1,
        .max  = 220,
        .def  = 1,
    },
    {
        /* float (0.0, 2.2] */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_CURVE2_H_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_curve2_high_fac",
        .step = 1,
        .min  = 1,
        .max  = 220,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_GLOBAL_FLAT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_global_flat_level",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0~3.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_DEGAMMA,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_degamma",
        .step = 1,
        .min  = 0,
        .max  = 300,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_MAX_GAIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_max_gain",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_MIN_GAIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_min_gain",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0~100.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_H_DIFF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_high_diff",
        .step = 1,
        .min  = 0,
        .max  = 10000,
    },
    {
        /* float 0~100.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_L_DIFF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_low_diff",
        .step = 1,
        .min  = 0,
        .max  = 10000,
    },
    {
        /* float 0~1 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_SAT_RANGE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_sat_range",
        .step = 1,
        .min  = 0,
        .max  = 100,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_sat_thr_gain_down",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_manu_sat_thr_gain_up",
        .step = 1,
        .min  = 0,
        .max  = 256,
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
        .id   = VVCAM_ISP_CID_WDR_STAT_H_STRENGTH,
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
        .id   = VVCAM_ISP_CID_WDR_STAT_L_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_low_strength",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
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
        .def  = 500,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_ENTROPY_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_entropy_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
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
        .id   = VVCAM_ISP_CID_WDR_STAT_LUMA_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_luma_thr",
        .step = 1,
        .min  = 1,
        .max  = 1023,
        .def  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_FLAT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_flat_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_FLAT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_flat_level",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 4x17 array */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_FLAT_LEVEL_INC,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_flat_level_inc",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {4, 17, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_DARK_ATT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_dark_atten_level",
        .step = 1,
        .min  = 0,
        .max  = 14,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_FIXED_WEIGHT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_fixed_weight",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_CURVE_SELECT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_curve_select",
        .step = 1,
        .min  = 0,
        .max  = 2,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_LOG_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_log_weight",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0.0 ~ 20.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_ANCHOR_X_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_anchor_x_log",
        .step = 1,
        .min  = 0,
        .max  = 2000,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_ANCHOR_SLOPE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_anchor_slope",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .def  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float (0.0, 2.0] */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_RATIO,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_actual_ratio",
        .step = 1,
        .min  = 1,
        .max  = 200,
        .def  = 1,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_CURVE2_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_curve2_threshold",
        .step = 1,
        .min  = 0,
        .max  = 100,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_CURVE2_L_SAT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_curve2_low_sat",
        .step = 1,
        .min  = 0,
        .max  = 100,
    },
    {
        /* float (0.0, 2.2] */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_CURVE2_L_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_curve2_low_fac",
        .step = 1,
        .min  = 1,
        .max  = 220,
        .def  = 1,
    },
    {
        /* float (0.0, 2.2] */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_CURVE2_H_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_curve2_high_fac",
        .step = 1,
        .min  = 1,
        .max  = 220,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_GLOBAL_FLAT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_global_flat_level",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0~3.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_DEGAMMA,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_degamma",
        .step = 1,
        .min  = 0,
        .max  = 300,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_MAX_GAIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_max_gain",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_MIN_GAIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_min_gain",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0~100.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_H_DIFF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_high_diff",
        .step = 1,
        .min  = 0,
        .max  = 10000,
    },
    {
        /* float 0~100.0 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_L_DIFF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_low_diff",
        .step = 1,
        .min  = 0,
        .max  = 10000,
    },
    {
        /* float 0~1 */
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_SAT_RANGE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_sat_range",
        .step = 1,
        .min  = 0,
        .max  = 100,
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_SAT_THR_GAIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_sat_thr_gain_down",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_wdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_WDR_STAT_SAT_THR_GAIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_wdr_stat_sat_thr_gain_up",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
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

