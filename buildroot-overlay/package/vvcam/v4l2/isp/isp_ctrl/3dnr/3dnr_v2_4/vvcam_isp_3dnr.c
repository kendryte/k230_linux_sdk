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
        case VVCAM_ISP_CID_3DNR_RESET:
        case VVCAM_ISP_CID_3DNR_MODE:
        case VVCAM_ISP_CID_3DNR_AUTO_LEVEL:
        case VVCAM_ISP_CID_3DNR_AUTO_GAIN:
        case VVCAM_ISP_CID_3DNR_AUTO_STRENGTH:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_INV:
        case VVCAM_ISP_CID_3DNR_AUTO_DELTA:
        case VVCAM_ISP_CID_3DNR_MANU_STRENGTH:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_INV:
        case VVCAM_ISP_CID_3DNR_MANU_DELTA:
        case VVCAM_ISP_CID_3DNR_MOTION_ROI:
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
        case VVCAM_ISP_CID_3DNR_RESET:
        case VVCAM_ISP_CID_3DNR_MODE:
        case VVCAM_ISP_CID_3DNR_AUTO_LEVEL:
        case VVCAM_ISP_CID_3DNR_AUTO_GAIN:
        case VVCAM_ISP_CID_3DNR_AUTO_STRENGTH:
        case VVCAM_ISP_CID_3DNR_AUTO_MOTION_INV:
        case VVCAM_ISP_CID_3DNR_AUTO_DELTA:
        case VVCAM_ISP_CID_3DNR_MANU_STRENGTH:
        case VVCAM_ISP_CID_3DNR_MANU_MOTION_INV:
        case VVCAM_ISP_CID_3DNR_MANU_DELTA:
        case VVCAM_ISP_CID_3DNR_MOTION_ROI:
        case VVCAM_ISP_CID_3DNR_STAT_STRENGTH:
        case VVCAM_ISP_CID_3DNR_STAT_MOTION_INV:
        case VVCAM_ISP_CID_3DNR_STAT_DELTA:
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
        /* uint8_t 20x  */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* int32_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_MOTION_INV,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_motion_inv",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20x */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_AUTO_DELTA,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_auto_delta",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_MOTION_INV,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_motion_inv",
        .step = 1,
        .min  = 0,
        .max  = 65535,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MANU_DELTA,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_manu_delta",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
    },
    {
        /* 64x array 64x4x32bit */
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_MOTION_ROI,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_motion_roi",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {64, 4, 0, 0},
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_MOTION_INV,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_motion_inv",
        .step = 1,
        .min  = 0,
        .max  = 65535,
    },
    {
        .ops  = &vvcam_isp_3dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_3DNR_STAT_DELTA,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_3dnr_stat_delta",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1, 0, 0, 0},
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

