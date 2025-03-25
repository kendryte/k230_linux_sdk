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
#include "vvcam_isp_afm.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_afm_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AFM_ENABLE:
        case VVCAM_ISP_CID_AFM_GAMMA_ENABLE:
        case VVCAM_ISP_CID_AFM_RESET:
        case VVCAM_ISP_CID_AFM_BLS:
        case VVCAM_ISP_CID_AFM_HIST_THRESHOLD:
        case VVCAM_ISP_CID_AFM_GAMMA_VAL:
        case VVCAM_ISP_CID_AFM_HW_ROI:
        case VVCAM_ISP_CID_AFM_LDG_IIR_HIGH_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_IIR_HIGH_PASS_X:
        case VVCAM_ISP_CID_AFM_LDG_IIR_LOW_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_IIR_LOW_PASS_X:
        case VVCAM_ISP_CID_AFM_LDG_FIR_HIGH_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_FIR_HIGH_PASS_X:
        case VVCAM_ISP_CID_AFM_LDG_FIR_LOW_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_FIR_LOW_PASS_X:
        case VVCAM_ISP_CID_AFM_CORING_IIR_HIGH_PASS:
        case VVCAM_ISP_CID_AFM_CORING_IIR_LOW_PASS:
        case VVCAM_ISP_CID_AFM_CORING_FIR_HIGH_PASS:
        case VVCAM_ISP_CID_AFM_CORING_FIR_LOW_PASS:
        case VVCAM_ISP_CID_AFM_IIR_DATA_SHIFT:
        case VVCAM_ISP_CID_AFM_IIR_HIGH_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_IIR_LOW_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_IIR_HIGH_PASS_COEFF:
        case VVCAM_ISP_CID_AFM_IIR_LOW_PASS_COEFF:
        case VVCAM_ISP_CID_AFM_FIR_DATA_SHIFT:
        case VVCAM_ISP_CID_AFM_FIR_HIGH_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_FIR_LOW_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_SHARPNESS_HIGH_PASS:
        case VVCAM_ISP_CID_AFM_SHARPNESS_LOW_PASS:
        case VVCAM_ISP_CID_AFM_HIST_LOW_DATA:
        case VVCAM_ISP_CID_AFM_HIST_HIGH_DATA:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_afm_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AFM_ENABLE:
        // case VVCAM_ISP_CID_AFM_GAMMA_ENABLE:
        case VVCAM_ISP_CID_AFM_RESET:
        case VVCAM_ISP_CID_AFM_BLS:
        case VVCAM_ISP_CID_AFM_HIST_THRESHOLD:
        case VVCAM_ISP_CID_AFM_GAMMA_VAL:
        case VVCAM_ISP_CID_AFM_HW_ROI:
        case VVCAM_ISP_CID_AFM_LDG_IIR_HIGH_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_IIR_HIGH_PASS_X:
        case VVCAM_ISP_CID_AFM_LDG_IIR_LOW_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_IIR_LOW_PASS_X:
        case VVCAM_ISP_CID_AFM_LDG_FIR_HIGH_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_FIR_HIGH_PASS_X:
        case VVCAM_ISP_CID_AFM_LDG_FIR_LOW_PASS_Y:
        case VVCAM_ISP_CID_AFM_LDG_FIR_LOW_PASS_X:
        case VVCAM_ISP_CID_AFM_CORING_IIR_HIGH_PASS:
        case VVCAM_ISP_CID_AFM_CORING_IIR_LOW_PASS:
        case VVCAM_ISP_CID_AFM_CORING_FIR_HIGH_PASS:
        case VVCAM_ISP_CID_AFM_CORING_FIR_LOW_PASS:
        case VVCAM_ISP_CID_AFM_IIR_DATA_SHIFT:
        case VVCAM_ISP_CID_AFM_IIR_HIGH_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_IIR_LOW_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_IIR_HIGH_PASS_COEFF:
        case VVCAM_ISP_CID_AFM_IIR_LOW_PASS_COEFF:
        case VVCAM_ISP_CID_AFM_FIR_DATA_SHIFT:
        case VVCAM_ISP_CID_AFM_FIR_HIGH_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_FIR_LOW_PASS_WEIGHT:
        case VVCAM_ISP_CID_AFM_SHARPNESS_HIGH_PASS:
        case VVCAM_ISP_CID_AFM_SHARPNESS_LOW_PASS:
        case VVCAM_ISP_CID_AFM_HIST_LOW_DATA:
        case VVCAM_ISP_CID_AFM_HIST_HIGH_DATA:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_afm_ctrl_ops = {
    .s_ctrl = vvcam_isp_afm_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_afm_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_afm_ctrls[] = {
    {
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_GAMMA_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_gamma_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_BLS,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_bls",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_HIST_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_hist_threshold",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float (0,4] */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_GAMMA_VAL,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_gamma_val",
        .step = 1,
        .min  = 1,
        .max  = 400,
        .def  = 1,
    },
    {
        /* uint16_t 4x array */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_HW_ROI,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_hw_roi",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {4, 0, 0, 0},
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_IIR_HIGH_PASS_Y,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_iir_high_pass_y",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_IIR_HIGH_PASS_X,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_iir_high_pass_x",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {4, 0, 0, 0},
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_IIR_LOW_PASS_Y,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_iir_low_pass_y",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_IIR_LOW_PASS_X,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_iir_low_pass_x",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {4, 0, 0, 0},
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_FIR_HIGH_PASS_Y,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_fir_high_pass_y",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_FIR_HIGH_PASS_X,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_fir_high_pass_x",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {4, 0, 0, 0},
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_FIR_LOW_PASS_Y,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_fir_low_pass_y",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_LDG_FIR_LOW_PASS_X,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_ldg_fir_low_pass_x",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {4, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_CORING_IIR_HIGH_PASS,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_coring_iir_high_pass",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_CORING_IIR_LOW_PASS,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_coring_iir_low_pass",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_CORING_FIR_HIGH_PASS,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_coring_fir_high_pass",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 255 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_CORING_FIR_LOW_PASS,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_coring_fir_low_pass",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint8_t 0 ~ 15 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_IIR_DATA_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_iir_data_shift",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_IIR_HIGH_PASS_WEIGHT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_iir_high_pass_weight",
        .step = 1,
        .min  = 0,
        .max  = 255,
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_IIR_LOW_PASS_WEIGHT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_iir_low_pass_weight",
        .step = 1,
        .min  = 0,
        .max  = 255,
    },
    {
        /* float 3*4 -4095/256 ~ 4095/256 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_IIR_HIGH_PASS_COEFF,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_iir_high_pass_coeff",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 4, 0, 0},
    },
    {
        /* float 3*4 -4095/256 ~ 4095/256 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_IIR_LOW_PASS_COEFF,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_iir_low_pass_coeff",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 4, 0, 0},
    },
    {
        /* uint8_t 0 ~ 15 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_FIR_DATA_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_fir_data_shift",
        .step = 1,
        .min  = 0,
        .max  = 15,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_FIR_HIGH_PASS_WEIGHT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_fir_high_pass_weight",
        .step = 1,
        .min  = 0,
        .max  = 255,
    },
    {
        /* float 0 ~ 15.9375 */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_FIR_LOW_PASS_WEIGHT,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_fir_low_pass_weight",
        .step = 1,
        .min  = 0,
        .max  = 255,
    },
    {
        /* uint32_t 15x15 array */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_SHARPNESS_HIGH_PASS,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_sharpness_high_pass",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {225, 0, 0, 0},
    },
    {
        /* uint32_t 15x15 array */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_SHARPNESS_LOW_PASS,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_sharpness_low_pass",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {225, 0, 0, 0},
    },
    {
        /* uint32_t 15x15 array */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_HIST_LOW_DATA,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_hist_low_data",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {225, 0, 0, 0},
    },
    {
        /* uint32_t 15x15 array */
        .ops  = &vvcam_isp_afm_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFM_HIST_HIGH_DATA,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afm_hist_high_data",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {225, 0, 0, 0},
    },
};

int vvcam_isp_afm_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_afm_ctrls);
}

int vvcam_isp_afm_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_afm_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_afm_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp afm ctrl %s failed %d.\n",
                vvcam_isp_afm_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

