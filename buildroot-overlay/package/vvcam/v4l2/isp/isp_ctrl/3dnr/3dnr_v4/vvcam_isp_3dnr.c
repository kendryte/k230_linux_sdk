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
#include "vvcam_isp_3dnr.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_3dnr_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_3DNR_ENABLE:
        case VVCAM_ISP_CID_3DNR_GAMMA_FE_BE_ENABLE:
        case VVCAM_ISP_CID_3DNR_MOTION_DILATE_ENABLE:
        case VVCAM_ISP_CID_3DNR_GAUSS_BLUR_ENABLE:
        case VVCAM_ISP_CID_3DNR_RESET:
        case VVCAM_ISP_CID_3DNR_MODE:
        case VVCAM_ISP_CID_3DNR_AUTO_LEVEL:
        case VVCAM_ISP_CID_3DNR_AUTO_GAIN:
        case VVCAM_ISP_CID_3DNR_AUTO_NMK:
        case VVCAM_ISP_CID_3DNR_AUTO_NMP:
        case VVCAM_ISP_CID_3DNR_AUTO_FIX_CURVE_START:
        case VVCAM_ISP_CID_3DNR_AUTO_NOISE_MODEL_A:
        case VVCAM_ISP_CID_3DNR_AUTO_NOISE_MODEL_B:
        case VVCAM_ISP_CID_3DNR_AUTO_BLS_EXP:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_STRENGTH:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_STRENGTH2:
        case VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN:
        case VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN2:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_SMOOTH_FAC:
        case VVCAM_ISP_CID_3DNR_AUTO_RANGE_H:
        case VVCAM_ISP_CID_3DNR_AUTO_SAD_WEIGHT:
        case VVCAM_ISP_CID_3DNR_AUTO_SQR_DIFF_FAC:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_SMOOTH_LEVEL:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_DILATE_EN:
        case VVCAM_ISP_CID_3DNR_AUTO_DILATE_H:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_MOTION_SLOPE_Y:
        case VVCAM_ISP_CID_3DNR_MANU_VST_FAC:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_STRENGTH:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_STRENGTH2:
        case VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN:
        case VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN2:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_SMOOTH_FAC:
        case VVCAM_ISP_CID_3DNR_MANU_RANGE_H:
        case VVCAM_ISP_CID_3DNR_MANU_SAD_WEIGHT:
        case VVCAM_ISP_CID_3DNR_MANU_DIFF_TYPE:
        case VVCAM_ISP_CID_3DNR_MANU_SQR_DIFF_FAC:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_SMOOTH_LEVEL:
        case VVCAM_ISP_CID_3DNR_MANU_DILATE_H:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_LEVEL:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_SLOPE_THR:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_LUMA_CURVE_X:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_MOTION_SLOPE_Y:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_INPUT_BIT:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_FIX_CURVE_START:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_A:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_B:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_BLS_EXP:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_3dnr_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_3DNR_ENABLE:
        case VVCAM_ISP_CID_3DNR_GAMMA_FE_BE_ENABLE:
        case VVCAM_ISP_CID_3DNR_MOTION_DILATE_ENABLE:
        case VVCAM_ISP_CID_3DNR_GAUSS_BLUR_ENABLE:
        case VVCAM_ISP_CID_3DNR_RESET:
        case VVCAM_ISP_CID_3DNR_MODE:
        case VVCAM_ISP_CID_3DNR_AUTO_LEVEL:
        case VVCAM_ISP_CID_3DNR_AUTO_GAIN:
        case VVCAM_ISP_CID_3DNR_AUTO_NMK:
        case VVCAM_ISP_CID_3DNR_AUTO_NMP:
        case VVCAM_ISP_CID_3DNR_AUTO_FIX_CURVE_START:
        case VVCAM_ISP_CID_3DNR_AUTO_NOISE_MODEL_A:
        case VVCAM_ISP_CID_3DNR_AUTO_NOISE_MODEL_B:
        case VVCAM_ISP_CID_3DNR_AUTO_BLS_EXP:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_STRENGTH:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_STRENGTH2:
        case VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN:
        case VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN2:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_SMOOTH_FAC:
        case VVCAM_ISP_CID_3DNR_AUTO_RANGE_H:
        case VVCAM_ISP_CID_3DNR_AUTO_SAD_WEIGHT:
        case VVCAM_ISP_CID_3DNR_AUTO_SQR_DIFF_FAC:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_SMOOTH_LEVEL:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_DILATE_EN:
        case VVCAM_ISP_CID_3DNR_AUTO_DILATE_H:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_3DNR_AUTO_TNR_MOTION_SLOPE_Y:
        case VVCAM_ISP_CID_3DNR_MANU_VST_FAC:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_STRENGTH:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_STRENGTH2:
        case VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN:
        case VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN2:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_SMOOTH_FAC:
        case VVCAM_ISP_CID_3DNR_MANU_RANGE_H:
        case VVCAM_ISP_CID_3DNR_MANU_SAD_WEIGHT:
        case VVCAM_ISP_CID_3DNR_MANU_DIFF_TYPE:
        case VVCAM_ISP_CID_3DNR_MANU_SQR_DIFF_FAC:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_SMOOTH_LEVEL:
        case VVCAM_ISP_CID_3DNR_MANU_DILATE_H:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_LEVEL:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_SLOPE_THR:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_LUMA_CURVE_X:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_3DNR_MANU_TNR_MOTION_SLOPE_Y:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_INPUT_BIT:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_FIX_CURVE_START:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_A:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_B:
        case VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_BLS_EXP:
        case VVCAM_ISP_CID_3DNR_STAT_VST_FAC:
        case VVCAM_ISP_CID_3DNR_STAT_TNR_STRENGTH:
        case VVCAM_ISP_CID_3DNR_STAT_TNR_STRENGTH2:
        case VVCAM_ISP_CID_3DNR_STAT_FILTER_LEN:
        case VVCAM_ISP_CID_3DNR_STAT_FILTER_LEN2:
        case VVCAM_ISP_CID_3DNR_STAT_MOTION_SMOOTH_FAC:
        case VVCAM_ISP_CID_3DNR_STAT_RANGE_H:
        case VVCAM_ISP_CID_3DNR_STAT_SAD_WEIGHT:
        case VVCAM_ISP_CID_3DNR_STAT_DIFF_TYPE:
        case VVCAM_ISP_CID_3DNR_STAT_SQR_DIFF_FAC:
        case VVCAM_ISP_CID_3DNR_STAT_MOTION_SMOOTH_LEVEL:
        case VVCAM_ISP_CID_3DNR_STAT_DILATE_H:
        case VVCAM_ISP_CID_3DNR_STAT_NOISE_LEVEL:
        case VVCAM_ISP_CID_3DNR_STAT_MOTION_SLOPE_THR:
        case VVCAM_ISP_CID_3DNR_STAT_TNR_LUMA_CURVE_X:
        case VVCAM_ISP_CID_3DNR_STAT_TNR_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_3DNR_STAT_TNR_MOTION_SLOPE_Y:
        case VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_INPUT_BIT:
        case VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_FIX_CURVE_START:
        case VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_A:
        case VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_B:
        case VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_BLS_EXP:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_3dnr_ctrl_ops = {
    .s_ctrl = vvcam_isp_3dnr_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_3dnr_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_3dnr_ctrls[] = {
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_GAMMA_FE_BE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_gamma_febe_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MOTION_DILATE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_mot_dilate_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_GAUSS_BLUR_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_gauss_blur_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_level",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float 0 ~ 100*/
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_NMK,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_nmk",
        .step = 1,
        .min  = 0,
        .max  = 1000,
    },
    {
        /* float 0 ~ 100*/
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_NMP,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_nmp",
        .step = 1,
        .min  = 0,
        .max  = 1000,
    },
    {
        /* uint16_t 0 ~ 4096 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_FIX_CURVE_START,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_fix_curve_start",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {20, 0, 0, 0},
    },
    {
        /* double array 20x  0.1~100*/
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_NOISE_MODEL_A,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_noise_mdl_a",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* double array 20x  0.1~10000*/
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_NOISE_MODEL_B,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_noise_mdl_b",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint32_t array 20x4  */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_BLS_EXP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_bls_exp",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {20, 4, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_TNR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_tnr_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_TNR_STRENGTH2,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_tnr_strength2",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x 1~99 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_filter_len",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 99,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x 1~99 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_FILTER_LEN2,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_filter_len2",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 99,
        .dims = {20, 0, 0, 0},
    },
    {
        /* double array 20x  0.1 ~ 5.0*/
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_MOTION_SMOOTH_FAC,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_mot_smooth_fac",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint32_t array 20x  3 or 6 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_RANGE_H,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_range_h",
        .step = 1,
        .def  = 3,
        .min  = 3,
        .max  = 6,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_SAD_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_sad_weight",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_SQR_DIFF_FAC,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_sqr_diff_fac",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_MOTION_SMOOTH_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_mot_smooth_level",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint32_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_MOTION_DILATE_EN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_mot_dilate_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint32_t array 20x  3 or 6*/
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_DILATE_H,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_dilate_h",
        .step = 1,
        .def  = 3,
        .min  = 3,
        .max  = 6,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_TNR_LUMA_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_tnr_luma_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {20, 12, 0, 0},
    },
    {
        /* uint16_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_TNR_MOTION_SLOPE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_tnr_mot_slope_y",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 12, 0, 0},
    },
    {
        /* double 1.0 ~  1000.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_VST_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_vst_fac",
        .step = 1,
        .def  = 10,
        .min  = 10,
        .max  = 10000,
    },
    {
        /* uint8_t 0 ~ 128 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_TNR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_tnr_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 128 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_TNR_STRENGTH2,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_tnr_strength2",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 1 ~ 99 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_filter_len",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 99,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 1 ~ 99 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_FILTER_LEN2,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_filter_len2",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 99,
        .dims = {1, 0, 0, 0},
    },
    {
        /* double 0.1 ~ 5.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_MOTION_SMOOTH_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_mot_smooth_fac",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 50,
    },
    {
        /* int 3 or 6 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_RANGE_H,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_range_h",
        .step = 1,
        .def  = 3,
        .min  = 3,
        .max  = 6,
    },
    {
        /* uint8_t 0 ~ 16 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_SAD_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_sad_weight",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_DIFF_TYPE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_diff_type",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_SQR_DIFF_FAC,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_sqr_diff_fac",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 0~1 */ 
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_MOTION_SMOOTH_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_mot_smooth_level",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        /* 3 or 6 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_DILATE_H,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_dilate_h",
        .step = 1,
        .def  = 3,
        .min  = 3,
        .max  = 6,
    },
    {
        /* uint16_t 0 ~ 65535 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_NOISE_LEVEL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_noise_level",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4095 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_MOTION_SLOPE_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_mot_slope_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4095 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_TNR_LUMA_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_tnr_luma_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 65535 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_TNR_LUMA_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_tnr_luma_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4095 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_TNR_MOTION_SLOPE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_tnr_mot_slope_y",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t 20 or 24 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_INPUT_BIT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_noise_mdl_in_bit",
        .step = 1,
        .def  = 20,
        .min  = 20,
        .max  = 24,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4096 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_FIX_CURVE_START,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_noise_mdl_fcs",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {1, 0, 0, 0},
    },
    {
        /* double 0.1 ~ 100.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_A,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_noise_mdl_a",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 1000,
    },
    {
        /* double 0.1 ~ 10000.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_B,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_noise_mdl_b",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 100000,
    },
    {
        /* uint32_t  0 ~ 1048575 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_NOISE_MODEL_BLS_EXP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_noise_mdl_bls_exp",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {4, 0, 0, 0},
    },
    {
        /* double 1.0 ~  1000.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_VST_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_vst_fac",
        .step = 1,
        .def  = 10,
        .min  = 10,
        .max  = 10000,
    },
    {
        /* uint8_t 0 ~ 128 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_TNR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_tnr_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 128 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_TNR_STRENGTH2,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_tnr_strength2",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 1 ~ 99 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_FILTER_LEN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_filter_len",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 99,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 1 ~ 99 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_FILTER_LEN2,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_filter_len2",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 99,
        .dims = {1, 0, 0, 0},
    },
    {
        /* double 0.1 ~ 5.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_MOTION_SMOOTH_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_mot_smooth_fac",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 50,
    },
    {
        /* int 3 or 6 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_RANGE_H,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_range_h",
        .step = 1,
        .def  = 3,
        .min  = 3,
        .max  = 6,
    },
    {
        /* uint8_t 0 ~ 16 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_SAD_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_sad_weight",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_DIFF_TYPE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_diff_type",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_SQR_DIFF_FAC,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_sqr_diff_fac",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint8_t 0~1 */ 
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_MOTION_SMOOTH_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_mot_smooth_level",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        /* 3 or 6 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_DILATE_H,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_dilate_h",
        .step = 1,
        .def  = 3,
        .min  = 3,
        .max  = 6,
    },
    {
        /* uint16_t 0 ~ 65535 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_NOISE_LEVEL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_noise_level",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4095 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_MOTION_SLOPE_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_mot_slope_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4095 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_TNR_LUMA_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_tnr_luma_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 65535 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_TNR_LUMA_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_tnr_luma_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4095 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_TNR_MOTION_SLOPE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_tnr_mot_slope_y",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t 20 or 24 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_INPUT_BIT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_noise_mdl_in_bit",
        .step = 1,
        .def  = 20,
        .min  = 20,
        .max  = 24,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t 0 ~ 4096 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_FIX_CURVE_START,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_noise_mdl_fcs",
        .step = 1,
        .min  = 0,
        .max  = 4096,
        .dims = {1, 0, 0, 0},
    },
    {
        /* double 0.1 ~ 100.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_A,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_noise_mdl_a",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 1000,
    },
    {
        /* double 0.1 ~ 10000.0 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_B,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_noise_mdl_b",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 100000,
    },
    {
        /* uint32_t  0 ~ 1048575 */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_NOISE_MODEL_BLS_EXP,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_noise_mdl_bls_exp",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {4, 0, 0, 0},
    },
};

int vvcam_isp_3dnr_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_3dnr_ctrls);
}

int vvcam_isp_3dnr_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_3dnr_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_3dnr_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp 3dnr ctrl %s failed %d.\n",
                vvcam_isp_3dnr_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

