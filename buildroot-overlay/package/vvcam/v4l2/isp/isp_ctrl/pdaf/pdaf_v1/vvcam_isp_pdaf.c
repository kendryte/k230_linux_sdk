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
#include "vvcam_isp_pdaf.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_pdaf_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_PDAF_ENABLE:
        case VVCAM_ISP_CID_PDAF_COR_ENABLE:
        case VVCAM_ISP_CID_PDAF_RESET:
        case VVCAM_ISP_CID_PDAF_COR_L_CHANGE_DOWN:
        case VVCAM_ISP_CID_PDAF_COR_L_CHANGE_UP:
        case VVCAM_ISP_CID_PDAF_COR_L_CHANNEL:
        case VVCAM_ISP_CID_PDAF_COR_L_LIMIT_BASE:
        case VVCAM_ISP_CID_PDAF_COR_R_CHANGE_DOWN:
        case VVCAM_ISP_CID_PDAF_COR_R_CHANGE_UP:
        case VVCAM_ISP_CID_PDAF_COR_R_CHANNEL:
        case VVCAM_ISP_CID_PDAF_COR_R_LIMIT_BASE:
        case VVCAM_ISP_CID_PDAF_COR_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_DUMMY_LINE_HW:
        case VVCAM_ISP_CID_PDAF_ROI:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_pdaf_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_PDAF_ENABLE:
        case VVCAM_ISP_CID_PDAF_COR_ENABLE:
        case VVCAM_ISP_CID_PDAF_COR_L_CHANGE_DOWN:
        case VVCAM_ISP_CID_PDAF_COR_L_CHANGE_UP:
        case VVCAM_ISP_CID_PDAF_COR_L_CHANNEL:
        case VVCAM_ISP_CID_PDAF_COR_L_LIMIT_BASE:
        case VVCAM_ISP_CID_PDAF_COR_R_CHANGE_DOWN:
        case VVCAM_ISP_CID_PDAF_COR_R_CHANGE_UP:
        case VVCAM_ISP_CID_PDAF_COR_R_CHANNEL:
        case VVCAM_ISP_CID_PDAF_COR_R_LIMIT_BASE:
        case VVCAM_ISP_CID_PDAF_COR_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_DUMMY_LINE_HW:
        case VVCAM_ISP_CID_PDAF_ROI:
        case VVCAM_ISP_CID_PDAF_STAT_COR_L_CHANGE_DOWN:
        case VVCAM_ISP_CID_PDAF_STAT_COR_L_CHANGE_UP:
        case VVCAM_ISP_CID_PDAF_STAT_COR_L_CHANNEL:
        case VVCAM_ISP_CID_PDAF_STAT_COR_L_LIMIT_BASE:
        case VVCAM_ISP_CID_PDAF_STAT_COR_R_CHANGE_DOWN:
        case VVCAM_ISP_CID_PDAF_STAT_COR_R_CHANGE_UP:
        case VVCAM_ISP_CID_PDAF_STAT_COR_R_CHANNEL:
        case VVCAM_ISP_CID_PDAF_STAT_COR_R_LIMIT_BASE:
        case VVCAM_ISP_CID_PDAF_STAT_COR_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_STAT_DUMMY_LINE_HW:
        case VVCAM_ISP_CID_PDAF_STAT_ROI:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_pdaf_ctrl_ops = {
    .s_ctrl = vvcam_isp_pdaf_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_pdaf_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_pdaf_ctrls[] = {
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_L_CHANGE_DOWN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_l_change_down",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_L_CHANGE_UP,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_l_change_up",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_L_CHANNEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_l_channel",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_L_LIMIT_BASE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_l_limit_base",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_R_CHANGE_DOWN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_r_change_down",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_R_CHANGE_UP,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_r_change_up",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_R_CHANNEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_r_channel",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_R_LIMIT_BASE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_r_limit_base",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_COR_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_cor_threshold",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_DUMMY_LINE_HW,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_dummy_line_hw",
        .step = 1,
        .min  = 0,
        .max  = 1000,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_ROI,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_roi",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFF,
        .dims = {4},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_L_CHANGE_DOWN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_l_change_down",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_L_CHANGE_UP,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_l_change_up",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_L_CHANNEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_l_channel",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_L_LIMIT_BASE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_l_limit_base",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_R_CHANGE_DOWN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_r_change_down",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_R_CHANGE_UP,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_r_change_up",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_R_CHANNEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_r_channel",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_R_LIMIT_BASE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_r_limit_base",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_COR_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_cor_threshold",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_DUMMY_LINE_HW,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_dummy_line_hw",
        .step = 1,
        .min  = 0,
        .max  = 1000,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_pdaf_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_STAT_ROI,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_stat_roi",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFF,
        .dims = {4},
    },
};

int vvcam_isp_pdaf_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_pdaf_ctrls);
}

int vvcam_isp_pdaf_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_pdaf_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_pdaf_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp ynr ctrl %s failed %d.\n",
                vvcam_isp_pdaf_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}
