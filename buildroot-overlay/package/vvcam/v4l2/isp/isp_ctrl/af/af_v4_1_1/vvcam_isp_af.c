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
#include "vvcam_isp_af.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_af_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AF_ENABLE:
        case VVCAM_ISP_CID_AF_RESET:
        case VVCAM_ISP_CID_AF_MODE:
        case VVCAM_ISP_CID_CDAF_WINDOW_WEIGHT:
        case VVCAM_ISP_CID_CDAF_STABLE_TOLERENCE:
        case VVCAM_ISP_CID_CDAF_POINTS_OF_CURVE:
        case VVCAM_ISP_CID_CDAF_MAX_FOCAL:
        case VVCAM_ISP_CID_CDAF_MIN_FOCAL:
        case VVCAM_ISP_CID_CDAF_MOTION_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_PD_CONF_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_PD_SHIFT_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_PD_STABLE_CNT_MAX:
        case VVCAM_ISP_CID_PCDAF_DEFOCUS_FRAME_NUM:
        case VVCAM_ISP_CID_PCDAF_LOSS_CONF_FRAME_NUM:
        case VVCAM_ISP_CID_PCDAF_ACC_FOCUS_STEP:
        case VVCAM_ISP_CID_PCDAF_ACC_FOCUS_ENABLE:
        case VVCAM_ISP_CID_PDAF_ROI_INDEX:
        case VVCAM_ISP_CID_AFMV11_ROI_WEIGHT:
        case VVCAM_ISP_CID_AFMV11_ROI:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_af_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_AF_ENABLE:
        case VVCAM_ISP_CID_AF_STATE:
        case VVCAM_ISP_CID_AF_MODE:
        case VVCAM_ISP_CID_CDAF_WINDOW_WEIGHT:
        case VVCAM_ISP_CID_CDAF_STABLE_TOLERENCE:
        case VVCAM_ISP_CID_CDAF_POINTS_OF_CURVE:
        case VVCAM_ISP_CID_CDAF_MAX_FOCAL:
        case VVCAM_ISP_CID_CDAF_MIN_FOCAL:
        case VVCAM_ISP_CID_CDAF_MOTION_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_PD_CONF_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_PD_SHIFT_THRESHOLD:
        case VVCAM_ISP_CID_PDAF_PD_STABLE_CNT_MAX:
        case VVCAM_ISP_CID_PCDAF_DEFOCUS_FRAME_NUM:
        case VVCAM_ISP_CID_PCDAF_LOSS_CONF_FRAME_NUM:
        case VVCAM_ISP_CID_PCDAF_ACC_FOCUS_STEP:
        case VVCAM_ISP_CID_PCDAF_ACC_FOCUS_ENABLE:
        case VVCAM_ISP_CID_PDAF_ROI_INDEX:
        case VVCAM_ISP_CID_AFMV11_ROI_WEIGHT:
        case VVCAM_ISP_CID_AFMV11_ROI:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_af_ctrl_ops = {
    .s_ctrl = vvcam_isp_af_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_af_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_af_ctrls[] = {
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_AF_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_af_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_AF_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_af_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_AF_STATE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_af_state",
        .step = 1,
        .min  = 0,
        .max  = 5,
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_AF_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_af_mode",
        .step = 1,
        .min  = 0,
        .max  = 2,
    },
    {
        /* float array 9x */
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_CDAF_WINDOW_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cdaf_window_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {9, 0, 0, 0},
    },
    {
        /* float (0,1) */
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_CDAF_STABLE_TOLERENCE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cdaf_stable_tolerance",
        .step = 1,
        .min  = 1,
        .max  = 99,
        .def  = 1,
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_CDAF_POINTS_OF_CURVE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cdaf_points_of_curve",
        .step = 1,
        .min  = 3,
        .max  = 20,
        .def  = 3,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_CDAF_MAX_FOCAL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cdaf_max_focal",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_CDAF_MIN_FOCAL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cdaf_min_focal",
        .step = 1,
        .min  = 0,
        .max  = 1023,
        .dims = {1},
    },
    {
        /* float (0,1) */
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_CDAF_MOTION_THRESHOLD,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_cdaf_motion_threshold",
        .step = 1,
        .min  = 1,
        .max  = 999,
        .def  = 1,
    },
    {
        /* float (0,1023) */
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_PD_CONF_THRESHOLD,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_pd_conf_threshold",
        .step = 1,
        .min  = 1,
        .max  = 10229,
        .def  = 1,
    },
    {
        /* float [0,1023] */
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_PD_SHIFT_THRESHOLD,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_pd_shift_threshold",
        .step = 1,
        .min  = 0,
        .max  = 10230,
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_PD_STABLE_CNT_MAX,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_pd_stable_cnt_max",
        .step = 1,
        .min  = 1,
        .max  = 10,
        .def  = 1,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PCDAF_DEFOCUS_FRAME_NUM,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pcdaf_defocus_frame_num",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PCDAF_LOSS_CONF_FRAME_NUM,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pcdaf_loss_conf_frame_num",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PCDAF_ACC_FOCUS_STEP,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pcdaf_acc_focus_step",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1},
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PCDAF_ACC_FOCUS_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pcdaf_acc_focus_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_PDAF_ROI_INDEX,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_pdaf_roi_index",
        .step = 1,
        .min  = 0,
        .max  = 48,
        .dims = {1},
    },
    {
        /* float 25x array 0~1 */
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFMV11_ROI_WEIGHT,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afmv11_roi_weight",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {25, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_af_ctrl_ops,
        .id   = VVCAM_ISP_CID_AFMV11_ROI,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_afmv11_roi",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFF,
        .dims = {25, 4, 0, 0},
    }
};

int vvcam_isp_af_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_af_ctrls);
}

int vvcam_isp_af_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_af_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_af_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp af ctrl %s failed %d.\n",
                vvcam_isp_af_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

