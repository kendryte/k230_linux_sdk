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
#include "vvcam_isp_2dnr.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_2dnr_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_2DNR_ENABLE:
        case VVCAM_ISP_CID_2DNR_LSC_COMP_ENABLE:
        case VVCAM_ISP_CID_2DNR_MOTION_FAC_ENABLE:
        case VVCAM_ISP_CID_2DNR_LUMA_ENABLE:
        case VVCAM_ISP_CID_2DNR_RESET:
        case VVCAM_ISP_CID_2DNR_MODE:
        case VVCAM_ISP_CID_2DNR_AUTO_LEVEL:
        case VVCAM_ISP_CID_2DNR_AUTO_GAIN:
        case VVCAM_ISP_CID_2DNR_AUTO_VST_FAC:
        case VVCAM_ISP_CID_2DNR_AUTO_BLEND_STATIC:
        case VVCAM_ISP_CID_2DNR_AUTO_BLEND_MOTION:
        case VVCAM_ISP_CID_2DNR_AUTO_BLEND_SLOPE:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_OFFSET:
        case VVCAM_ISP_CID_2DNR_AUTO_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_AUTO_LSC_COMP_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_AUTO_MOTION_FAC_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_AUTO_MOTION_ANCHOR_X:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_SCALE:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_FAC:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_FAC_MUL:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_FAC_MOTION_MAX:
        case VVCAM_ISP_CID_2DNR_MANU_BLEND_STATIC:
        case VVCAM_ISP_CID_2DNR_MANU_BLEND_MOTION:
        case VVCAM_ISP_CID_2DNR_MANU_BLEND_SLOPE:
        case VVCAM_ISP_CID_2DNR_MANU_VST_FAC:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_SCALE:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MUL:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MOTION_MAX:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MOTION_MIN:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_OFFSET:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_FAC:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_X:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_INT_MODE:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_X:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_INT_MODE:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_ANCHOR_X:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_X:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_INT_MODE:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_2dnr_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_2DNR_ENABLE:
        case VVCAM_ISP_CID_2DNR_LSC_COMP_ENABLE:
        case VVCAM_ISP_CID_2DNR_MOTION_FAC_ENABLE:
        case VVCAM_ISP_CID_2DNR_LUMA_ENABLE:
        case VVCAM_ISP_CID_2DNR_RESET:
        case VVCAM_ISP_CID_2DNR_MODE:
        case VVCAM_ISP_CID_2DNR_AUTO_LEVEL:
        case VVCAM_ISP_CID_2DNR_AUTO_GAIN:
        case VVCAM_ISP_CID_2DNR_AUTO_VST_FAC:
        case VVCAM_ISP_CID_2DNR_AUTO_BLEND_STATIC:
        case VVCAM_ISP_CID_2DNR_AUTO_BLEND_MOTION:
        case VVCAM_ISP_CID_2DNR_AUTO_BLEND_SLOPE:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_OFFSET:
        case VVCAM_ISP_CID_2DNR_AUTO_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_AUTO_LSC_COMP_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_AUTO_MOTION_FAC_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_AUTO_MOTION_ANCHOR_X:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_SCALE:
        case VVCAM_ISP_CID_2DNR_AUTO_STATIC_FAC:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_FAC_MUL:
        case VVCAM_ISP_CID_2DNR_AUTO_SIGMA_FAC_MOTION_MAX:
        case VVCAM_ISP_CID_2DNR_MANU_BLEND_STATIC:
        case VVCAM_ISP_CID_2DNR_MANU_BLEND_MOTION:
        case VVCAM_ISP_CID_2DNR_MANU_BLEND_SLOPE:
        case VVCAM_ISP_CID_2DNR_MANU_VST_FAC:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_SCALE:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MUL:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MOTION_MAX:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MOTION_MIN:
        case VVCAM_ISP_CID_2DNR_MANU_SIGMA_OFFSET:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_MANU_STATIC_FAC:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_X:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_INT_MODE:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_X:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_INT_MODE:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_ANCHOR_X:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_X:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_INT_MODE:
        case VVCAM_ISP_CID_2DNR_STAT_BLEND_STATIC:
        case VVCAM_ISP_CID_2DNR_STAT_BLEND_MOTION:
        case VVCAM_ISP_CID_2DNR_STAT_BLEND_SLOPE:
        case VVCAM_ISP_CID_2DNR_STAT_VST_FAC:
        case VVCAM_ISP_CID_2DNR_STAT_SIGMA_SCALE:
        case VVCAM_ISP_CID_2DNR_STAT_SIGMA_FAC_MUL:
        case VVCAM_ISP_CID_2DNR_STAT_SIGMA_FAC_MOTION_MAX:
        case VVCAM_ISP_CID_2DNR_STAT_SIGMA_FAC_MOTION_MIN:
        case VVCAM_ISP_CID_2DNR_STAT_SIGMA_OFFSET:
        case VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_THR:
        case VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_BOOST_THR:
        case VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_BOOST:
        case VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_CLIP_THR:
        case VVCAM_ISP_CID_2DNR_STAT_STATIC_FAC:
        case VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_X:
        case VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_INT_MODE:
        case VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_X:
        case VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_INT_MODE:
        case VVCAM_ISP_CID_2DNR_STAT_MOTION_ANCHOR_X:
        case VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_X:
        case VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_Y:
        case VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_PX:
        case VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_INT_MODE:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_2dnr_ctrl_ops = {
    .s_ctrl = vvcam_isp_2dnr_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_2dnr_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_2dnr_ctrls[] = {
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_LSC_COMP_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_lsc_comp_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MOTION_FAC_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_mot_fac_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_LUMA_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_luma_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* manual/auto */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_level",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float array 20*32bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20*32bit 1.0~1000.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_VST_FAC,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_vst_fac",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20*32bit 0~100.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_BLEND_STATIC,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_blend_stc",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20*32bit 0~100.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_BLEND_MOTION,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_blend_mot",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float array 20*32bit 0~32.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_BLEND_SLOPE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_blend_slope",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_SIGMA_OFFSET,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_sigma_offset",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_LUMA_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_luma_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {20, 12, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_LSC_COMP_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_lsc_comp_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {20, 12, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_MOTION_FAC_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_mot_fac_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {20, 12, 0, 0},
    },
    {
        /* uint16_t array 20*2*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_MOTION_ANCHOR_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_mot_anchor_x",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 2, 0, 0},
    },
    {
        /* uint16_t array 20*2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_stc_dtl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 2, 3, 0},
    },
    {
        /* uint16_t array 20*2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_BOOST_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_stc_dtl_boost_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 2, 3, 0},
    },
    {
        /* float array 20*2*3*16bit 1.0~4.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_BOOST,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_stc_dtl_boost",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 2, 3, 0},
    },
    {
        /* uint16_t array 20*2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_STATIC_DETAIL_CLIP_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_stc_dtl_clip_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 2, 3, 0},
    },
    {
        /* uint16_t array 20*2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_mov_dtl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 2, 3, 0},
    },
    {
        /* uint16_t array 20*2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_BOOST_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_mov_dtl_boost_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 2, 3, 0},
    },
    {
        /* float array 20*2*3*32bit 1.0~4.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_BOOST,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_mov_dtl_boost",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 2, 3, 0},
    },
    {
        /* uint16_t array 20*2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_MOVING_DETAIL_CLIP_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_mov_dtl_clip_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 2, 3, 0},
    },
    {
        /* float array 20*3*32bit 0.001~100.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_SIGMA_SCALE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_sigma_scale",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 3, 0, 0},
    },
    {
        /* float array 20*3*32bit 0.01~1.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_STATIC_FAC,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_stc_fac",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 3, 0, 0},
    },
    {
        /* float array 20*3*32bit 0.1~10.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_SIGMA_FAC_MUL,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_sigma_fac_mul",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 3, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_AUTO_SIGMA_FAC_MOTION_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_auto_sigma_fac_mot_max",
        .step = 1,
        .min  = 1,
        .max  = 1024,
        .def  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* float 0.0~100.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_BLEND_STATIC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_blend_stc",
        .step = 1,
        .min  = 0,
        .max  = 1000,
    },
    {
        /* float 0.0~100.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_BLEND_MOTION,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_blend_mot",
        .step = 1,
        .min  = 0,
        .max  = 1000,
    },
    {
        /* float 0.0~32.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_BLEND_SLOPE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_blend_slope",
        .step = 1,
        .min  = 0,
        .max  = 320,
    },
    {
        /* float 1.0~1000.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_VST_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_vst_fac",
        .step = 1,
        .def  = 10,
        .min  = 10,
        .max  = 10000,
    },
    {
        /* float array 3*32bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_SIGMA_SCALE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_sigma_scale",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* float array 3*32bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MUL,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_sigma_fac_mul",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MOTION_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_sigma_fac_mot_max",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_SIGMA_FAC_MOTION_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_sigma_fac_mot_min",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_SIGMA_OFFSET,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_sigma_offset",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_stc_dtl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_BOOST_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_stc_dtl_boost_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* float array 2*3*32bit 1.0~4.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_BOOST,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_stc_dtl_boost",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_STATIC_DETAIL_CLIP_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_stc_dtl_clip_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mov_dtl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_BOOST_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mov_dtl_boost_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* float array 2*3*32bit 1.0~4.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_BOOST,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mov_dtl_boost",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOVING_DETAIL_CLIP_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mov_dtl_clip_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* float array 3*32bit 0.01~1.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_STATIC_FAC,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_stc_fac",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_luma_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_luma_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_PX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_luma_curve_px",
        .step = 1,
        .min  = 0,
        .max  = 12,
        .dims = {12, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LUMA_CURVE_INT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_luma_curve_im",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_lsc_comp_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_lsc_comp_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_PX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_lsc_comp_curve_px",
        .step = 1,
        .min  = 0,
        .max  = 12,
        .dims = {12, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_LSC_COMP_CURVE_INT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_lsc_comp_curve_im",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* uint16_t array 2*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOTION_ANCHOR_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mot_anchor_x",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {2, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mot_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mot_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_PX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mot_curve_px",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {12, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_MANU_MOTION_CURVE_INT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_manu_mot_curve_im",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* float 0.0~100.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_BLEND_STATIC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_blend_stc",
        .step = 1,
        .min  = 0,
        .max  = 1000,
    },
    {
        /* float 0.0~100.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_BLEND_MOTION,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_blend_mot",
        .step = 1,
        .min  = 0,
        .max  = 1000,
    },
    {
        /* float 0.0~32.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_BLEND_SLOPE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_blend_slope",
        .step = 1,
        .min  = 0,
        .max  = 320,
    },
    {
        /* float 1.0~1000.0 */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_VST_FAC,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_vst_fac",
        .step = 1,
        .def  = 10,
        .min  = 10,
        .max  = 10000,
    },
    {
        /* float array 3*32bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_SIGMA_SCALE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_sigma_scale",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* float array 3*32bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_SIGMA_FAC_MUL,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_sigma_fac_mul",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_SIGMA_FAC_MOTION_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_sigma_fac_mot_max",
        .step = 1,
        .def  = 1,
        .min  = 1,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_SIGMA_FAC_MOTION_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_sigma_fac_mot_min",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_SIGMA_OFFSET,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_sigma_offset",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_stc_dtl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_BOOST_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_stc_dtl_boost_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* float array 2*3*32bit 1.0~4.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_BOOST,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_stc_dtl_boost",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_STATIC_DETAIL_CLIP_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_stc_dtl_clip_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mov_dtl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_BOOST_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mov_dtl_boost_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* float array 2*3*32bit 1.0~4.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_BOOST,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mov_dtl_boost",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* uint16_t array 2*3*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOVING_DETAIL_CLIP_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mov_dtl_clip_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {2, 3, 0, 0},
    },
    {
        /* float array 3*32bit 0.01~1.0*/
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_STATIC_FAC,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_stc_fac",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {3, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_luma_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_luma_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_PX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_luma_curve_px",
        .step = 1,
        .min  = 0,
        .max  = 12,
        .dims = {12, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LUMA_CURVE_INT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_luma_curve_im",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_lsc_comp_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_lsc_comp_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_PX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_lsc_comp_curve_px",
        .step = 1,
        .min  = 0,
        .max  = 12,
        .dims = {12, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_LSC_COMP_CURVE_INT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_lsc_comp_curve_im",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        /* uint16_t array 2*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOTION_ANCHOR_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mot_anchor_x",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {2, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_X,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mot_curve_x",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_Y,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mot_curve_y",
        .step = 1,
        .min  = 0,
        .max  = 65535,
        .dims = {12, 0, 0, 0},
    },
    {
        /* uint16_t array 12*16bit */
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_PX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mot_curve_px",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {12, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_2dnr_ctrl_ops,
        .id   = VVCAM_ISP_CID_2DNR_STAT_MOTION_CURVE_INT_MODE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_2dnr_stat_mot_curve_im",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
};

int vvcam_isp_2dnr_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_2dnr_ctrls);
}

int vvcam_isp_2dnr_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_2dnr_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_2dnr_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp 2dnr ctrl %s failed %d.\n",
                vvcam_isp_2dnr_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;

}

