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
#include "vvcam_isp_ee.h"
#include "vvcam_isp_event.h"

static int vvcam_isp_ee_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_EE_ENABLE:
        case VVCAM_ISP_CID_EE_RESET:
        case VVCAM_ISP_CID_EE_CURVE_ENABLE:
        case VVCAM_ISP_CID_EE_CA_ENABLE:
        case VVCAM_ISP_CID_EE_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_EE_DCI_ENABLE:
        case VVCAM_ISP_CID_EE_YUV_DOMAIN_ENABLE:
        case VVCAM_ISP_CID_EE_SHARPEN_LIMIT_ENABLE:
        case VVCAM_ISP_CID_EE_SKIN_PROC_ENABLE:
        case VVCAM_ISP_CID_EE_MODE:
        case VVCAM_ISP_CID_EE_AUTO_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_GAIN:
        case VVCAM_ISP_CID_EE_AUTO_EE_EN:
        case VVCAM_ISP_CID_EE_AUTO_STRENGTH:
        case VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH:
        case VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH_SKIN:
        case VVCAM_ISP_CID_EE_AUTO_EDGE_NR_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_DETAIL_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_DETAIL_PRE_ENHANCE_STR:
        case VVCAM_ISP_CID_EE_AUTO_SKIN_PROC_EN:
        case VVCAM_ISP_CID_EE_AUTO_SKIN_DETECT_STR:
        case VVCAM_ISP_CID_EE_AUTO_SHARP_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_SHARP_UV_GAIN:
        case VVCAM_ISP_CID_EE_AUTO_DEPURPLE_EN:
        case VVCAM_ISP_CID_EE_AUTO_DPL_LIMIT:
        case VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_THR:
        case VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_AUTO_DPL_COMP_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_AUTO_DPL_SAT_STR:
        case VVCAM_ISP_CID_EE_AUTO_DPL_FIX_STR:
        case VVCAM_ISP_CID_EE_AUTO_CA_ENABLE:
        case VVCAM_ISP_CID_EE_AUTO_CA_LUT_CHROMA_TBL:
        case VVCAM_ISP_CID_EE_AUTO_DCI_EN:
        case VVCAM_ISP_CID_EE_AUTO_DCI_CURVE:
        case VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH:
        case VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH_SKIN:
        case VVCAM_ISP_CID_EE_MANU_EDGE_NR_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_EDGE_SCALER:
        case VVCAM_ISP_CID_EE_MANU_EDGE_USE_AUXI_DIR:
        case VVCAM_ISP_CID_EE_MANU_DETAIL_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_DETAIL_SCALER:
        case VVCAM_ISP_CID_EE_MANU_DETAIL_PRE_ENHANCE_STR:
        case VVCAM_ISP_CID_EE_MANU_SKIN_DETECT_STR:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MIN:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MAX:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MIN:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MAX:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MIN:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MAX:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T0:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T1_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T2:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T3_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T0:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T1_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T2:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T3_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE_SKIN:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL_SKIN:
        case VVCAM_ISP_CID_EE_MANU_ENHANCE_STRENGTH:
        case VVCAM_ISP_CID_EE_MANU_SHARP_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_SHARP_SKIN_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_DOWN:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_DOWN:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UV:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_TYPE:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_DOWN:
        case VVCAM_ISP_CID_EE_MANU_DPL_DETECT_RANGE:
        case VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_UP:
        case VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_UP:
        case VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_THR:
        case VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_MANU_DPL_COMP_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_MANU_DPL_SAT_STR:
        case VVCAM_ISP_CID_EE_MANU_DPL_FIX_STR:
        case VVCAM_ISP_CID_EE_MANU_CA_LUT_CHROMA_TBL:
        case VVCAM_ISP_CID_EE_MANU_CA_MODE:
        case VVCAM_ISP_CID_EE_MANU_DCI_DEGAMMA:
        case VVCAM_ISP_CID_EE_MANU_DCI_LUT_LUMA_TBL:
            ret = vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static int vvcam_isp_ee_g_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret = 0;
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id)
    {
        case VVCAM_ISP_CID_EE_ENABLE:
        case VVCAM_ISP_CID_EE_RESET:
        case VVCAM_ISP_CID_EE_CURVE_ENABLE:
        case VVCAM_ISP_CID_EE_CA_ENABLE:
        case VVCAM_ISP_CID_EE_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_EE_DCI_ENABLE:
        case VVCAM_ISP_CID_EE_YUV_DOMAIN_ENABLE:
        case VVCAM_ISP_CID_EE_SHARPEN_LIMIT_ENABLE:
        case VVCAM_ISP_CID_EE_SKIN_PROC_ENABLE:
        case VVCAM_ISP_CID_EE_MODE:
        case VVCAM_ISP_CID_EE_AUTO_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_GAIN:
        case VVCAM_ISP_CID_EE_AUTO_EE_EN:
        case VVCAM_ISP_CID_EE_AUTO_STRENGTH:
        case VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH:
        case VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH_SKIN:
        case VVCAM_ISP_CID_EE_AUTO_EDGE_NR_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_DETAIL_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_DETAIL_PRE_ENHANCE_STR:
        case VVCAM_ISP_CID_EE_AUTO_SKIN_PROC_EN:
        case VVCAM_ISP_CID_EE_AUTO_SKIN_DETECT_STR:
        case VVCAM_ISP_CID_EE_AUTO_SHARP_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_AUTO_SHARP_UV_GAIN:
        case VVCAM_ISP_CID_EE_AUTO_DEPURPLE_EN:
        case VVCAM_ISP_CID_EE_AUTO_DPL_LIMIT:
        case VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_THR:
        case VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_AUTO_DPL_COMP_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_AUTO_DPL_SAT_STR:
        case VVCAM_ISP_CID_EE_AUTO_DPL_FIX_STR:
        case VVCAM_ISP_CID_EE_AUTO_CA_ENABLE:
        case VVCAM_ISP_CID_EE_AUTO_CA_LUT_CHROMA_TBL:
        case VVCAM_ISP_CID_EE_AUTO_DCI_EN:
        case VVCAM_ISP_CID_EE_AUTO_DCI_CURVE:
        case VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH:
        case VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH_SKIN:
        case VVCAM_ISP_CID_EE_MANU_EDGE_NR_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_EDGE_SCALER:
        case VVCAM_ISP_CID_EE_MANU_EDGE_USE_AUXI_DIR:
        case VVCAM_ISP_CID_EE_MANU_DETAIL_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_DETAIL_SCALER:
        case VVCAM_ISP_CID_EE_MANU_DETAIL_PRE_ENHANCE_STR:
        case VVCAM_ISP_CID_EE_MANU_SKIN_DETECT_STR:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MIN:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MAX:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MIN:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MAX:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MIN:
        case VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MAX:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T0:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T1_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T2:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T3_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T0:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T1_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T2:
        case VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T3_SHIFT:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE_SKIN:
        case VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL_SKIN:
        case VVCAM_ISP_CID_EE_MANU_ENHANCE_STRENGTH:
        case VVCAM_ISP_CID_EE_MANU_SHARP_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_SHARP_SKIN_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_DOWN:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_DOWN:
        case VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UV:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_TYPE:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_UP:
        case VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_DOWN:
        case VVCAM_ISP_CID_EE_MANU_DPL_DETECT_RANGE:
        case VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_UP:
        case VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_UP:
        case VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_THR:
        case VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_MANU_DPL_COMP_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_MANU_DPL_SAT_STR:
        case VVCAM_ISP_CID_EE_MANU_DPL_FIX_STR:
        case VVCAM_ISP_CID_EE_MANU_CA_LUT_CHROMA_TBL:
        case VVCAM_ISP_CID_EE_MANU_CA_MODE:
        case VVCAM_ISP_CID_EE_MANU_DCI_DEGAMMA:
        case VVCAM_ISP_CID_EE_MANU_DCI_LUT_LUMA_TBL:
        case VVCAM_ISP_CID_EE_STAT_SKIN_PROC_ENABLE:
        case VVCAM_ISP_CID_EE_STAT_SRC_STRENGTH:
        case VVCAM_ISP_CID_EE_STAT_SRC_STRENGTH_SKIN:
        case VVCAM_ISP_CID_EE_STAT_EDGE_NR_LEVEL:
        case VVCAM_ISP_CID_EE_STAT_EDGE_SCALER:
        case VVCAM_ISP_CID_EE_STAT_EDGE_USE_AUXI_DIR:
        case VVCAM_ISP_CID_EE_STAT_DETAIL_LEVEL:
        case VVCAM_ISP_CID_EE_STAT_DETAIL_SCALER:
        case VVCAM_ISP_CID_EE_STAT_DETAIL_PRE_ENHANCE_STR:
        case VVCAM_ISP_CID_EE_STAT_SKIN_DETECT_STR:
        case VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_Y_MIN:
        case VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_Y_MAX:
        case VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CR_MIN:
        case VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CR_MAX:
        case VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CB_MIN:
        case VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CB_MAX:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T0:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T1_SHIFT:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T2:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T3_SHIFT:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T0:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T1_SHIFT:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T2:
        case VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T3_SHIFT:
        case VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_EDGE:
        case VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_DETAIL:
        case VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_EDGE_SKIN:
        case VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_DETAIL_SKIN:
        case VVCAM_ISP_CID_EE_STAT_ENHANCE_STRENGTH:
        case VVCAM_ISP_CID_EE_STAT_SHARP_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_STAT_SHARP_SKIN_CURVE_LEVEL:
        case VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_UP:
        case VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_DOWN:
        case VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_SKIN_UP:
        case VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_SKIN_DOWN:
        case VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_UV:
        case VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_TYPE:
        case VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_UP:
        case VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_SKIN_UP:
        case VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_SKIN_DOWN:
        case VVCAM_ISP_CID_EE_STAT_DPL_DETECT_RANGE:
        case VVCAM_ISP_CID_EE_STAT_DPL_U_LIMIT_UP:
        case VVCAM_ISP_CID_EE_STAT_DPL_U_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_STAT_DPL_V_LIMIT_UP:
        case VVCAM_ISP_CID_EE_STAT_DPL_V_LIMIT_DOWN:
        case VVCAM_ISP_CID_EE_STAT_DPL_DETECT_LUMA_THR:
        case VVCAM_ISP_CID_EE_STAT_DPL_DETECT_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_STAT_DPL_COMP_LUMA_DIFF:
        case VVCAM_ISP_CID_EE_STAT_DPL_SAT_STR:
        case VVCAM_ISP_CID_EE_STAT_DPL_FIX_STR:
        case VVCAM_ISP_CID_EE_STAT_CA_LUT_CHROMA_TBL:
        case VVCAM_ISP_CID_EE_STAT_CA_MODE:
        case VVCAM_ISP_CID_EE_STAT_DCI_DEGAMMA:
        case VVCAM_ISP_CID_EE_STAT_DCI_LUT_LUMA_TBL:
            ret = vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);
            break;

        default:
            dev_err(isp_dev->dev, "unknow v4l2 ctrl id %d\n", ctrl->id);
            return -EACCES;
    }

    return ret;
}

static const struct v4l2_ctrl_ops vvcam_isp_ee_ctrl_ops = {
	.s_ctrl = vvcam_isp_ee_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_ee_g_ctrl,
};

const struct v4l2_ctrl_config vvcam_isp_ee_ctrls[] = {
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_RESET,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_reset",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_CURVE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_curve_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_CA_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_ca_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_DEPURPLE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_depurple_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_DCI_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_dci_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_YUV_DOMAIN_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_yuv_domain_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_SHARPEN_LIMIT_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_sharpen_lmt_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_SKIN_PROC_ENABLE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_skin_proc_enable",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        /* manual/auto */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_mode",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_level",
        .step = 1,
        .min  = 1,
        .max  = 20,
        .def  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float 20 * 32bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_GAIN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_gain",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFFFFFF,
        .dims = {20, 0, 0, 0},
    },
    {
        /* bool 20 * 32bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_EE_EN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_ee_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*8bit  0 ~ 256 */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_src_strength",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20*8bit 0 ~ 256 */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_src_strength_skin",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_EDGE_NR_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_edge_nr_level",
        .step = 1,
        .min  = 0,
        .max  = 5,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DETAIL_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dtl_level",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DETAIL_PRE_ENHANCE_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dtl_pre_enhance_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SKIN_PROC_EN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_skin_proc_enable",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SKIN_DETECT_STR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_skin_detect_str",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {20, 0, 0, 0},
    },
    {
        /* int16_t array 20*6*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SKIN_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_skin_threshold",
        .step = 1,
        .min  = 0,
        .max  = 0xFFFF,
        .dims = {20, 6, 0, 0},
    },
    {
        /* uint16_t array 20*4*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_HF_MERGE_CURVE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_hf_merge_curve",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 4, 0, 0},
    },
    {
        /* uint16_t array 20*4*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_HF_MERGE_CURVE_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_hf_merge_curve_skin",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 4, 0, 0},
    },
    {
        /* uint16_t array 20*2*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_GRAD_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_grad_threshold",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 2, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_GRAD_THRESHOLD_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_grad_threshold_skin",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 2, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SHARP_CURVE_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_sharp_crv_lv",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_CURVE_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_sharp_skin_crv_lv",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*2*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SHARP_GAIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_sharp_gain",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 2, 0, 0},
    },
    {
        /* uint16_t array 20*2*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_GAIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_sharp_skin_gain",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 2, 0, 0},
    },
    {
        /* uint8_t array 20*8bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SHARP_UV_GAIN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_sharp_uv_gain",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*2*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SHARP_LIMIT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_sharp_lmt",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {20, 2, 0, 0},
    },
    {
        /* uint16_t array 20*2*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_LIMIT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_sharp_skin_limit",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {20, 2, 0, 0},
    },
    {
        /* bool array 20*32bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DEPURPLE_EN,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dpl_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*4*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DPL_LIMIT,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dpl_lmt_down",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 4, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dpl_det_luma_thr",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_DIFF,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dpl_det_luma_diff",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DPL_COMP_LUMA_DIFF,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dpl_comp_luma_diff",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DPL_SAT_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dpl_sat_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DPL_FIX_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dpl_fix_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* bool array 20*32bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_CA_ENABLE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_ca_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*65*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_CA_LUT_CHROMA_TBL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_ca_lut_chroma_tbl",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 65, 0, 0},
    },
    {
        /* bool array 20*32bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DCI_CURVE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dci_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t array 20*65*16bit */
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_AUTO_DCI_CURVE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_auto_dci_curve",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 65, 0, 0},
    },

    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_src_strength",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_src_strength_skin",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_EDGE_NR_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_edge_nr_level",
        .step = 1,
        .min  = 0,
        .max  = 5,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_EDGE_SCALER,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_edge_scaler",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_EDGE_USE_AUXI_DIR,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_edge_use_auxi_dir",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DETAIL_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dtl_level",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DETAIL_SCALER,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dtl_scaler",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DETAIL_PRE_ENHANCE_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dtl_pre_enhance_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SKIN_DETECT_STR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_skin_detect_str",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_skin_h_thr_y_min",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_skin_h_thr_y_max",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_skin_h_thr_cr_min",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MAX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_skin_h_thr_cr_max",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_skin_h_thr_cb_min",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MAX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_skin_h_thr_cb_max",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T0,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_t0",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T1_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_t1_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_t2",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T3_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_t3_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },

    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T0,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_skin_t0",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T1_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_skin_t1_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_skin_t2",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T3_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_hf_mc_skin_t3_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_grad_thr_edge",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_grad_thr_dtl",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_grad_thr_edge_skin",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_grad_thr_dtl_skin",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_ENHANCE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_enhance_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_CURVE_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_crv_level",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_SKIN_CURVE_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_skin_crv_lvl",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_gain_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_gain_down",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_skin_gain_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_skin_gain_dwn",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UV,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_gain_uv",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_TYPE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_lmt_type",
        .step = 1,
        .min  = 0,
        .max  = 2,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_lmt_up",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_lmt_down",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_lmt_skin_up",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_sharp_lmt_skin_down",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_DETECT_RANGE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_detect_range",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_u_lmt_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_u_lmt_down",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_v_lmt_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_v_lmt_down",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_detect_luma_thr",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_DIFF,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_detect_luma_dif",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_COMP_LUMA_DIFF,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_comp_luma_dif",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_SAT_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_sat_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DPL_FIX_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dpl_fix_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_CA_LUT_CHROMA_TBL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_ca_lut_chroma_tbl",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {65, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_CA_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_ca_mode",
        .step = 1,
        .min  = 0,
        .max  = 2,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DCI_DEGAMMA,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dci_degamma",
        .step = 1,
        .def  = 100,
        .min  = 100,
        .max  = 400,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_MANU_DCI_LUT_LUMA_TBL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_manu_dci_lut_luma_tbl",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {65, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_PROC_ENABLE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_proc_enable",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SRC_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_src_strength",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SRC_STRENGTH_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_src_strength_skin",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_EDGE_NR_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_edge_nr_level",
        .step = 1,
        .min  = 0,
        .max  = 5,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_EDGE_SCALER,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_edge_scaler",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_EDGE_USE_AUXI_DIR,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_edge_use_auxi_dir",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DETAIL_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dtl_level",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DETAIL_SCALER,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dtl_scaler",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DETAIL_PRE_ENHANCE_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dtl_pre_enhance_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_DETECT_STR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_detect_str",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_Y_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_h_thr_y_min",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_Y_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_h_thr_y_max",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CR_MIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_h_thr_cr_min",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CR_MAX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_h_thr_cr_max",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CB_MIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_h_thr_cb_min",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CB_MAX,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_skin_h_thr_cb_max",
        .step = 1,
        .min  = -1024,
        .max  = 1023,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T0,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_t0",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T1_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_t1_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_t2",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T3_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_t3_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },

    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T0,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_skin_t0",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T1_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_skin_t1_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_skin_t2",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T3_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_hf_mc_skin_t3_shift",
        .step = 1,
        .min  = 0,
        .max  = 10,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_EDGE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_grad_thr_edge",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_DETAIL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_grad_thr_dtl",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_EDGE_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_grad_thr_edge_skin",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_DETAIL_SKIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_grad_thr_dtl_skin",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_ENHANCE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_enhance_strength",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_CURVE_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_crv_level",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_SKIN_CURVE_LEVEL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_skin_crv_lvl",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_gain_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_gain_down",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_SKIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_skin_gain_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_SKIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_skin_gain_dwn",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_UV,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_gain_uv",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_TYPE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_lmt_type",
        .step = 1,
        .min  = 0,
        .max  = 2,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_lmt_up",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_lmt_down",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_SKIN_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_lmt_skin_up",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_SKIN_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_sharp_lmt_skin_down",
        .step = 1,
        .min  = 0,
        .max  = 512,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_DETECT_RANGE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_det_range",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_U_LIMIT_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_u_lmt_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_U_LIMIT_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_u_lmt_down",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_V_LIMIT_UP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_v_lmt_up",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_V_LIMIT_DOWN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_v_lmt_down",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_DETECT_LUMA_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_det_luma_thr",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_DETECT_LUMA_DIFF,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_det_luma_diff",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_COMP_LUMA_DIFF,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_comp_luma_diff",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_SAT_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_sat_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DPL_FIX_STR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dpl_fix_str",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_CA_LUT_CHROMA_TBL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_ca_lut_chroma_tbl",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {65, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_CA_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_ca_mode",
        .step = 1,
        .min  = 0,
        .max  = 2,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DCI_DEGAMMA,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dci_degamma",
        .step = 1,
        .def  = 100,
        .min  = 100,
        .max  = 400,
    },
    {
        .ops  = &vvcam_isp_ee_ctrl_ops,
        .id   = VVCAM_ISP_CID_EE_STAT_DCI_LUT_LUMA_TBL,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_ee_stat_dci_lut_luma_tbl",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {65, 0, 0, 0},
    },
};

int vvcam_isp_ee_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_ee_ctrls);
}

int vvcam_isp_ee_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_ee_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_ee_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err( isp_dev->dev, "reigster isp ee ctrl %s failed %d.\n",
                vvcam_isp_ee_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;
}

