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
#include "vvcam_isp_cpd.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_cpd_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_CPD_ENABLE:
        case VVCAM_ISP_CID_CPD_RESET:
        case VVCAM_ISP_CID_CPD_CURVE_X:
        case VVCAM_ISP_CID_CPD_CURVE_Y:
        case VVCAM_ISP_CID_CPD_USE_OUT_Y_CURVE:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_cpd_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_CPD_ENABLE:
        case VVCAM_ISP_CID_CPD_RESET:
        case VVCAM_ISP_CID_CPD_CURVE_X:
        case VVCAM_ISP_CID_CPD_CURVE_Y:
        case VVCAM_ISP_CID_CPD_USE_OUT_Y_CURVE:
        case VVCAM_ISP_CID_CPD_STAT_CURVE_X:
        case VVCAM_ISP_CID_CPD_STAT_CURVE_Y:
        case VVCAM_ISP_CID_CPD_STAT_USE_OUT_Y_CURVE:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_cpd_ctrl_ops = {
    .s_ctrl = vvcam_isp_cpd_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_cpd_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_cpd_ctrls[] = {
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_CURVE_X,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 31,
        .dims = {64},
    },
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {64},
    },
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_USE_OUT_Y_CURVE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_use_out_y_curve",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_STAT_CURVE_X,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_stat_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 31,
        .dims = {64},
    },
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_STAT_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_stat_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 1048575,
        .dims = {64},
    },
    {
        .ops  = &vvcam_isp_cpd_ctrl_ops,
        .id   = VVCAM_ISP_CID_CPD_STAT_USE_OUT_Y_CURVE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cpd_stat_use_out_y_curve",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
};

int vvcam_isp_cpd_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_cpd_ctrls);
}

int vvcam_isp_cpd_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_cpd_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_cpd_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp cpd ctrl %s failed %d.\n",
                vvcam_isp_cpd_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}
