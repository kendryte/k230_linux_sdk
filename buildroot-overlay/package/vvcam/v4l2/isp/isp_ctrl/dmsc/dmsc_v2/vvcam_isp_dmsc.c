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
#include "vvcam_isp_dmsc.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_dmsc_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_DMSC_ENABLE:
        case VVCAM_ISP_CID_DMSC_CAC_ENABLE:
        case VVCAM_ISP_CID_DMSC_DEMOIRE_ENABLE:
        case VVCAM_ISP_CID_DMSC_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_DMSC_SHARPEN_ENABLE:
        case VVCAM_ISP_CID_DMSC_SHARPEN_LINE_ENABLE:
        case VVCAM_ISP_CID_DMSC_SKIN_ENABLE:
        case VVCAM_ISP_CID_DMSC_RESET:
        case VVCAM_ISP_CID_DMSC_MODE:
        case VVCAM_ISP_CID_DMSC_AUTO_LEVEL:
        case VVCAM_ISP_CID_DMSC_AUTO_GAIN:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR_BLACK:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR_WHITE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP_BLACK:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP_WHITE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_SIZE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R3:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_CBCR_MODE:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_MANU_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_MANU_DIR_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_DIR_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_AREA_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R3:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T3:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_SIZE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_THR_SHIFT1:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CB_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CB_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CR_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CR_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_Y_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_Y_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CBCR_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_THR:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_H_OFFSET:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_V_OFFSET:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_dmsc_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_DMSC_ENABLE:
        case VVCAM_ISP_CID_DMSC_CAC_ENABLE:
        case VVCAM_ISP_CID_DMSC_DEMOIRE_ENABLE:
        case VVCAM_ISP_CID_DMSC_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_DMSC_SHARPEN_ENABLE:
        case VVCAM_ISP_CID_DMSC_SHARPEN_LINE_ENABLE:
        case VVCAM_ISP_CID_DMSC_SKIN_ENABLE:
        case VVCAM_ISP_CID_DMSC_RESET:
        case VVCAM_ISP_CID_DMSC_MODE:
        case VVCAM_ISP_CID_DMSC_AUTO_LEVEL:
        case VVCAM_ISP_CID_DMSC_AUTO_GAIN:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR_BLACK:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR_WHITE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP_BLACK:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP_WHITE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_SIZE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R3:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_CBCR_MODE:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_MANU_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_MANU_DIR_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_DIR_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_AREA_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R3:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T3:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_SIZE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_THR_SHIFT1:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CB_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CB_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CR_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_CR_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_Y_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_SKIN_Y_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CBCR_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_THR:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_H_OFFSET:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_V_OFFSET:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_dmsc_ctrl_ops = {
    .s_ctrl = vvcam_isp_dmsc_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_dmsc_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_dmsc_ctrls[] = {
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_CAC_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_cac_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_DEMOIRE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_demoire_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_DEPURPLE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_dpl_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_SHARPEN_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_sharpen_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_SHARPEN_LINE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_sharpen_line_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_SKIN_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_skin_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* 0: Manual 1: Auto */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_level",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* bool 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_ENABLE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* bool 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_ENABLE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_dpl_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR_BLACK,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_fac_blk",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_fac_wht",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP_BLACK,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_clip_blk",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_clip_wht",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_t2_shift",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_t3",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_T4_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_t4_shift",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DENOISE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_denoise_str",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_SIZE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_size",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_R3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_r3",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_SAT_SHRINK,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_dpl_sat_shrink",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_CBCR_MODE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_dpl_cbcr_mode",
        .step = 1,
        .min  = 0,
        .max  = 3,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_dpl_threshold",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_threshold",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DIR_THR_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dir_thr_max",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DIR_THR_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dir_thr_min",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_T1,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_t1",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_t2_shift",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_edge_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_edge_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_edge_t1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_EDGE_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_edge_t2_shift",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_AREA_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_area_thr",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_SAT_SHRINK,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_sat_shrink",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_BLACK,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_clip_blk",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_clip_wht",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_BLACK,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_fac_blk",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_fac_wht",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_R3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_r3",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_t3",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_SIZE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_size",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_t2_shift",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_T4_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_t4_shift",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DENOISE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_denoise_str",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_line_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_line_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_line_str",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_line_thr",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_THR_SHIFT1,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_line_t_t1",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        /* int16_t  */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SKIN_CB_THR_MAX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_skin_cb_thr_max",
        .step = 1,
        .min  = -2047,
        .max  = 2047,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SKIN_CB_THR_MIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_skin_cb_thr_min",
        .step = 1,
        .min  = -2047,
        .max  = 2047,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SKIN_CR_THR_MAX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_skin_cr_thr_max",
        .step = 1,
        .min  = -2047,
        .max  = 2047,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SKIN_CR_THR_MIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_skin_cr_thr_min",
        .step = 1,
        .min  = -2047,
        .max  = 2047,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SKIN_Y_THR_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_skin_y_thr_max",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SKIN_Y_THR_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_skin_y_thr_min",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CBCR_MODE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_cbcr_mode",
        .step = 1,
        .min  = 0,
        .max  = 3,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_SAT_SHRINK,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_sat_shrink",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_A_BLUE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_a_blue",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_A_RED,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_a_red",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_B_BLUE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_b_blue",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_B_RED,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_b_red",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_C_BLUE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_c_blue",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_C_RED,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_c_red",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* according to image width */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_H_OFFSET,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_ctr_h_offset",
        .step = 1,
        .min  = -32768,
        .max  = 32767,
    },
    {
        /* according to image height */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_V_OFFSET,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_cac_ctr_v_offset",
        .step = 1,
        .min  = -32768,
        .max  = 32767,
    },
};

int vvcam_isp_dmsc_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_dmsc_ctrls);
}

int vvcam_isp_dmsc_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_dmsc_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_dmsc_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp dmsc ctrl %s failed %d.\n",
                vvcam_isp_dmsc_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;
}

