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
#include "vvcam_isp_hdr.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_hdr_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_ENABLE:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_ENABLE:
        case VVCAM_ISP_CID_HDR_RESET:
        case VVCAM_ISP_CID_HDR_COLOR_WEIGHT:
        case VVCAM_ISP_CID_HDR_RATIO:
        case VVCAM_ISP_CID_HDR_TRANS_RANGE:
        case VVCAM_ISP_CID_HDR_EXTEND_BIT:
        case VVCAM_ISP_CID_HDR_DEGHOST_MOTION_WEIGHT:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_MOTION_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_MOTION_UPPER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_MOTION_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_MOTION_UPPER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_DARK_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_DARK_UPPER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_DARK_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_DARK_UPPER_THR:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_hdr_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_HDR_ENABLE:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_ENABLE:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_ENABLE:
        case VVCAM_ISP_CID_HDR_COLOR_WEIGHT:
        case VVCAM_ISP_CID_HDR_RATIO:
        case VVCAM_ISP_CID_HDR_TRANS_RANGE:
        case VVCAM_ISP_CID_HDR_EXTEND_BIT:
        case VVCAM_ISP_CID_HDR_DEGHOST_MOTION_WEIGHT:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_MOTION_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_MOTION_UPPER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_MOTION_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_MOTION_UPPER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_DARK_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LS_DARK_UPPER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_DARK_LOWER_THR:
        case VVCAM_ISP_CID_HDR_DEGHOST_LSVS_DARK_UPPER_THR:
        case VVCAM_ISP_CID_HDR_STAT_COLOR_WEIGHT:
        case VVCAM_ISP_CID_HDR_STAT_RATIO:
        case VVCAM_ISP_CID_HDR_STAT_TRANS_RANGE:
        case VVCAM_ISP_CID_HDR_STAT_EXTEND_BIT:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_MOTION_WEIGHT:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_MOTION_LOWER_THR:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_MOTION_UPPER_THR:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_MOTION_LOWER_THR:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_MOTION_UPPER_THR:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_DARK_LOWER_THR:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_DARK_UPPER_THR:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_DARK_LOWER_THR:
        case VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_DARK_UPPER_THR:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_hdr_ctrl_ops = {
    .s_ctrl = vvcam_isp_hdr_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_hdr_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_hdr_ctrls[] = {
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LS_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_deghost_ls_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LSVS_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_deghost_lsvs_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* uint8_t 3x array */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_COLOR_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_color_weight",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {3, 0, 0, 0},
    },
    {
        /* float 2x array [1.0, 256.0] */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_RATIO,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_ratio",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {2, 0, 0, 0},
    },
    {
        /* float 4x2 array [0, 1.1] */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_TRANS_RANGE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_trans_range",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4, 2, 0, 0},
    },
    {
        /* int8_t 2x array [-1, 8] */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_EXTEND_BIT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_extend_bit",
        .step = 1,
        .min  = 0,
        .max  = 0xFF,
        .dims = {2, 0, 0, 0},
    },
    {
        /* uint16_t 2x array */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_MOTION_WEIGHT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_motion_weight",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {2, 0, 0, 0},
    },
        {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LS_MOTION_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_ls_motion_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LS_MOTION_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_ls_motion_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LSVS_MOTION_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_lsvs_motion_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LSVS_MOTION_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_lsvs_motion_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LS_DARK_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_ls_dark_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LS_DARK_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_ls_dark_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LSVS_DARK_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_lsvs_dark_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_DEGHOST_LSVS_DARK_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_dg_lsvs_dark_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        /* uint8_t 3x array */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_COLOR_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_color_weight",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {3, 0, 0, 0},
    },
    {
        /* float 2x array [1.0, 256.0] */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_RATIO,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_ratio",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {2, 0, 0, 0},
    },
    {
        /* float 4x2 array [0, 1.1] */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_TRANS_RANGE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_trans_range",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {4, 2, 0, 0},
    },
    {
        /* int8_t 2x array [-1, 8] */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_EXTEND_BIT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_extend_bit",
        .step = 1,
        .min  = 0,
        .max  = 0xFF,
        .dims = {2, 0, 0, 0},
    },
    {
        /* uint16_t 2x array */
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_MOTION_WEIGHT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_mo_weight",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_MOTION_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_ls_mo_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_MOTION_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_ls_mo_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_MOTION_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_lsvs_mo_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_MOTION_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_lsvs_mo_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_DARK_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_ls_dk_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LS_DARK_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_ls_dk_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_DARK_LOWER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_lsvs_dk_lwr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
    {
        .ops  = &vvcam_isp_hdr_ctrl_ops,
        .id   = VVCAM_ISP_CID_HDR_STAT_DEGHOST_LSVS_DARK_UPPER_THR,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_hdr_stat_dg_lsvs_dk_upr_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
    },
};

int vvcam_isp_hdr_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_hdr_ctrls);
}

int vvcam_isp_hdr_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_hdr_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_hdr_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp hdr ctrl %s failed %d.\n",
                vvcam_isp_hdr_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

