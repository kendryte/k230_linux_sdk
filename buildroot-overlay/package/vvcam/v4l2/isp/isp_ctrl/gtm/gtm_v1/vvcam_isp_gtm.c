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

#include <media/v4l2-ioctl.h>
#include "vvcam_isp_driver.h"
#include "vvcam_isp_ctrl.h"
#include "vvcam_isp_gtm.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_gtm_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_GTM_ENABLE:
        case VVCAM_ISP_CID_GTM_BW_CORRECTION_ENABLE:
        case VVCAM_ISP_CID_GTM_RESET:
        case VVCAM_ISP_CID_GTM_MODE:
        case VVCAM_ISP_CID_GTM_AUTO_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_SELECT:
        case VVCAM_ISP_CID_GTM_AUTO_BW_CORRECTION_MIN_LOG:
        case VVCAM_ISP_CID_GTM_AUTO_BW_CORRECTION_MAX_LOG:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_CONTRAST_LIMIT_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_HLC_FACTOR:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_LUMA_PRESERVE_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_MAX_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_MIN_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_STRENGTH:
        case VVCAM_ISP_CID_GTM_AUTO_COLOR_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_LUMA_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_USER_CURVE:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_MODE:
        case VVCAM_ISP_CID_GTM_AUTO_EDR_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_CONTRAST_LIMIT_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_THR_LOG:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_HLC_FACTOR:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_PRESERVE_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_COMPRESS_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_MAX_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_MIN_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_STRENGTH:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_DAMP_COEF:
        case VVCAM_ISP_CID_GTM_AUTO_EDR:
        case VVCAM_ISP_CID_GTM_MANU_RGB_COEF:
        case VVCAM_ISP_CID_GTM_MANU_LIGHTNESS_WEIGHT:
        case VVCAM_ISP_CID_GTM_MANU_COLOR_WEIGHT:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_MODE:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_MIN_LOG:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_MAX_LOG:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_DAMP_COEF:
        case VVCAM_ISP_CID_GTM_MANU_CURVE_SELECT:
        case VVCAM_ISP_CID_GTM_MANU_CURVE_SHIFT_BIT:
        case VVCAM_ISP_CID_GTM_MANU_PWL_KNEE_X_LOG:
        case VVCAM_ISP_CID_GTM_MANU_PWL_KNEE_Y_LOG:
        case VVCAM_ISP_CID_GTM_MANU_PWL_MAX_LOG:
        case VVCAM_ISP_CID_GTM_MANU_PWL_MIN_LOG:
        case VVCAM_ISP_CID_GTM_MANU_LOG_KNEE_X_LOG:
        case VVCAM_ISP_CID_GTM_MANU_LOG_KNEE_SLOPE:
        case VVCAM_ISP_CID_GTM_MANU_USER_CURVE:
        case VVCAM_ISP_CID_GTM_HIST_STATISTIC_MIN:
        case VVCAM_ISP_CID_GTM_HIST_STATISTIC_MAX:
        case VVCAM_ISP_CID_GTM_HIST_STATISTIC_DATA:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_gtm_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_GTM_ENABLE:
        case VVCAM_ISP_CID_GTM_BW_CORRECTION_ENABLE:
        case VVCAM_ISP_CID_GTM_RESET:
        case VVCAM_ISP_CID_GTM_MODE:
        case VVCAM_ISP_CID_GTM_AUTO_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_SELECT:
        case VVCAM_ISP_CID_GTM_AUTO_BW_CORRECTION_MIN_LOG:
        case VVCAM_ISP_CID_GTM_AUTO_BW_CORRECTION_MAX_LOG:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_CONTRAST_LIMIT_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_HLC_FACTOR:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_LUMA_PRESERVE_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_MAX_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_MIN_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_STRENGTH:
        case VVCAM_ISP_CID_GTM_AUTO_COLOR_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_HIST_LUMA_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_USER_CURVE:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_MODE:
        case VVCAM_ISP_CID_GTM_AUTO_EDR_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_CONTRAST_LIMIT_LEVEL:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_THR_LOG:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_HLC_FACTOR:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_PRESERVE_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_COMPRESS_WEIGHT:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_MAX_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_MIN_GAIN:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_STRENGTH:
        case VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_DAMP_COEF:
        case VVCAM_ISP_CID_GTM_AUTO_EDR:
        case VVCAM_ISP_CID_GTM_MANU_RGB_COEF:
        case VVCAM_ISP_CID_GTM_MANU_LIGHTNESS_WEIGHT:
        case VVCAM_ISP_CID_GTM_MANU_COLOR_WEIGHT:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_MODE:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_MIN_LOG:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_MAX_LOG:
        case VVCAM_ISP_CID_GTM_MANU_BW_COR_DAMP_COEF:
        case VVCAM_ISP_CID_GTM_MANU_CURVE_SELECT:
        case VVCAM_ISP_CID_GTM_MANU_CURVE_SHIFT_BIT:
        case VVCAM_ISP_CID_GTM_MANU_PWL_KNEE_X_LOG:
        case VVCAM_ISP_CID_GTM_MANU_PWL_KNEE_Y_LOG:
        case VVCAM_ISP_CID_GTM_MANU_PWL_MAX_LOG:
        case VVCAM_ISP_CID_GTM_MANU_PWL_MIN_LOG:
        case VVCAM_ISP_CID_GTM_MANU_LOG_KNEE_X_LOG:
        case VVCAM_ISP_CID_GTM_MANU_LOG_KNEE_SLOPE:
        case VVCAM_ISP_CID_GTM_MANU_USER_CURVE:
        case VVCAM_ISP_CID_GTM_HIST_STATISTIC_MIN:
        case VVCAM_ISP_CID_GTM_HIST_STATISTIC_MAX:
        case VVCAM_ISP_CID_GTM_HIST_STATISTIC_DATA:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_gtm_ctrl_ops = {
    .s_ctrl = vvcam_isp_gtm_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_gtm_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_gtm_ctrls[] = {
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_BW_CORRECTION_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_bw_cor_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* 0: Manual 1: Auto */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_level",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float array 20x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_SELECT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_select",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_BW_CORRECTION_MIN_LOG,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_bw_cor_min_log",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_BW_CORRECTION_MAX_LOG,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_bw_cor_max_log",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_HIST_CONTRAST_LIMIT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_hist_cll",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20x 0.0 ~ 10.0*/
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_HIST_HLC_FACTOR,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_hist_hf",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20x 0.0 ~ 1.0*/
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_HIST_LUMA_PRESERVE_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_hist_lpw",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20x 0.0 ~ 1048575.0*/
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_HIST_MAX_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_hist_maxg",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20x 0.0 ~ 1048575.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_HIST_MIN_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_hist_ming",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20x20 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_HIST_STRENGTH,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_hist_str",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 20, 0, 0},
    },
    {
        /* uint8_t array 20x3 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_COLOR_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_color_weight",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 3, 0, 0},
    },
    {
        /* uint16_t array 20x4 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_HIST_LUMA_WEIGHT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_hist_luma_weight",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 4, 0, 0},
    },
    {
         // uint32_t array 20x129 0~1048575
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_USER_CURVE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_user_curve",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {20, 129, 0, 0},
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_mode",
        .step = 1,
        .min  = 0,
        .max  = 2,
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_EDR_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_edr_level",
        .step = 1,
        .min  = 0,
        .max  = 20,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_CONTRAST_LIMIT_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_cll",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t array 4x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_THR_LOG,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_ltl",
        .step = 1,
        .min  = 0,
        .max  = 24,
        .dims = {4, 0, 0, 0},
    },
    {
        /* uint16_t array 4x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_WEIGHT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_lw",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {4, 0, 0, 0},
    },
    {
        /* float 0.0 ~ 10.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_HLC_FACTOR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_hf",
        .step = 1,
        .min  = 0,
        .max  = 100,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_LUMA_PRESERVE_WEIGHT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_lpw",
        .step = 1,
        .min  = 0,
        .max  = 10,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_COMPRESS_WEIGHT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_cw",
        .step = 1,
        .min  = 0,
        .max  = 10,
    },
    {
        /* float 0.0 ~ 1048575.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_MAX_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_maxg",
        .step = 1,
        .min  = 0,
        .max  = 10485750,
    },
    {
        /* float 0.0 ~ 1048575.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_MIN_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_ming",
        .step = 1,
        .min  = 0,
        .max  = 10485750,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_STRENGTH,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_str",
        .step = 1,
        .min  = 0,
        .max  = 10,
    },
    {
        /* float 0.0 ~ 1.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_CURVE_HIST_DAMP_COEF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_curve_hist_dc",
        .step = 1,
        .min  = 0,
        .max  = 10,
    },
    {
        /* float 20x  */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_AUTO_EDR,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_auto_edr",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 3x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_RGB_COEF,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_rgb_coef",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_LIGHTNESS_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_lightness_weight",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 3x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_COLOR_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_color_weight",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_BW_COR_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_bw_cor_mode",
        .step = 1,
        .min  = 0,
        .max  = 2,
    },
    {
        /* double  0.0 ~ 24.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_BW_COR_MIN_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_bw_cor_min_log",
        .step = 1,
        .min  = 0,
        .max  = 240,
    },
    {
        /* double  0.0 ~ 24.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_BW_COR_MAX_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_bw_cor_max_log",
        .step = 1,
        .min  = 0,
        .max  = 240,
    },
    {
        /* double  0.0 ~ 1.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_BW_COR_DAMP_COEF,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_bw_cor_damp_coef",
        .step = 1,
        .min  = 0,
        .max  = 10,
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_CURVE_SELECT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_curve_select",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_CURVE_SHIFT_BIT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_curve_shift_bit",
        .step = 1,
        .min  = 0,
        .max  = 4,
    },
    {
        /* double 0~24.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_PWL_KNEE_X_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_pwl_knee_x_log",
        .step = 1,
        .min  = 0,
        .max  = 240,
    },
    {
        /* double 0~20.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_PWL_KNEE_Y_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_pwl_knee_y_log",
        .step = 1,
        .min  = 0,
        .max  = 200,
    },
    {
        /* double 0~24.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_PWL_MAX_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_pwl_max_log",
        .step = 1,
        .min  = 0,
        .max  = 240,
    },
    {
        /* double 0~24.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_PWL_MIN_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_pwl_min_log",
        .step = 1,
        .min  = 0,
        .max  = 240,
    },
    {
        /* float 0~24.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_LOG_KNEE_X_LOG,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_log_knee_x_log",
        .step = 1,
        .min  = 0,
        .max  = 240,
    },
    {
        /* float 0~24.0 */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_LOG_KNEE_SLOPE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_log_knee_slope",
        .step = 1,
        .min  = 0,
        .max  = 240,
    },
    {
        /* uint32_t array 129x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_MANU_USER_CURVE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_manu_user_curve",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {129, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_HIST_STATISTIC_MIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_hist_statistic_min",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
    },
    {
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_HIST_STATISTIC_MAX,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_hist_statistic_max",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
    },
    {
        /* uint32_t array 128x */
        .ops  = &vvcam_isp_gtm_ctrl_ops,
        .id   = VVCAM_ISP_CID_GTM_HIST_STATISTIC_DATA,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_gtm_hist_statistic_data",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {128, 0, 0, 0},
    },
};

int vvcam_isp_gtm_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_gtm_ctrls);
}

int vvcam_isp_gtm_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_gtm_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_gtm_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp gtm ctrl %s failed %d.\n",
                vvcam_isp_gtm_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

