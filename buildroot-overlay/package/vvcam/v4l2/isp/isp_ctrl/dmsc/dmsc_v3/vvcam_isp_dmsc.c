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
        case VVCAM_ISP_CID_DMSC_DEFALSE_ENABLE:
        case VVCAM_ISP_CID_DMSC_INTERP_CORNER_ENABLE:
        case VVCAM_ISP_CID_DMSC_DECLINE_ENABLE:
        case VVCAM_ISP_CID_DMSC_RESET:
        case VVCAM_ISP_CID_DMSC_MODE:
        case VVCAM_ISP_CID_DMSC_AUTO_LEVEL:
        case VVCAM_ISP_CID_DMSC_AUTO_GAIN:
        case VVCAM_ISP_CID_DMSC_AUTO_INTERP_DIR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_INTERP_LARGE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_DECLINE_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_HIGH_LIGHT_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_CBCR_RANGE:
        case VVCAM_ISP_CID_DMSC_AUTO_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R3:
        case VVCAM_ISP_CID_DMSC_MANU_GRAD_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_GRAD_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_DIFF_COLOR_COEF:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_THR_LOW:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_THR_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_HIGH_LIGH_CONTROL_THR:
        case VVCAM_ISP_CID_DMSC_MANU_HIGH_LIGH_CONTROL_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_DEBURST_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_G_WEIGHT:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_RB_WEIGHT:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_G_WEIGHT_ISO:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_RB_WEIGHT_ISO:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_H_OFFSET:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_V_OFFSET:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_REFINE_G_THR:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_SHIFT1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T3:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R3:
        case VVCAM_ISP_CID_DMSC_MANU_DEFALSE_COLOR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_DEFALSE_CBCR_CLASS_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEFALSE_GRAY_PROTECT_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_GRAD_CONTROL:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_PROTECT_THR_LOW:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_PROTECT_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CBCR_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_COLOR_RANGE_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CB_LOW_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CB_HIGH_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CR_LOW_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CR_HIGH_THR:
        case VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_DIR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_LARGE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_DIR0_INTERP_TYPE:
        case VVCAM_ISP_CID_DMSC_MANU_DECLINE_CORNER_LOW_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DECLINE_CORNER_SHIFT:
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
        case VVCAM_ISP_CID_DMSC_DEFALSE_ENABLE:
        case VVCAM_ISP_CID_DMSC_INTERP_CORNER_ENABLE:
        case VVCAM_ISP_CID_DMSC_DECLINE_ENABLE:
        case VVCAM_ISP_CID_DMSC_RESET:
        case VVCAM_ISP_CID_DMSC_MODE:
        case VVCAM_ISP_CID_DMSC_AUTO_LEVEL:
        case VVCAM_ISP_CID_DMSC_AUTO_GAIN:
        case VVCAM_ISP_CID_DMSC_AUTO_INTERP_DIR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_INTERP_LARGE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_DECLINE_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_HIGH_LIGHT_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_THRESHOLD:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_CBCR_RANGE:
        case VVCAM_ISP_CID_DMSC_AUTO_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_ENABLE:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T3:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R1:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R2:
        case VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R3:
        case VVCAM_ISP_CID_DMSC_MANU_GRAD_THR_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_GRAD_THR_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_DIFF_COLOR_COEF:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_THR_LOW:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_THR_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MIN:
        case VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MAX:
        case VVCAM_ISP_CID_DMSC_MANU_HIGH_LIGH_CONTROL_THR:
        case VVCAM_ISP_CID_DMSC_MANU_HIGH_LIGH_CONTROL_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_DEBURST_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_G_WEIGHT:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_RB_WEIGHT:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_G_WEIGHT_ISO:
        case VVCAM_ISP_CID_DMSC_MANU_PURE_RB_WEIGHT_ISO:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_A_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_B_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_BLUE:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_C_RED:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_H_OFFSET:
        case VVCAM_ISP_CID_DMSC_MANU_CAC_CENTER_V_OFFSET:
        case VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_REFINE_G_THR:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_SHIFT1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_WHITE:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_BLACK:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T3:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R1:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R2:
        case VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R3:
        case VVCAM_ISP_CID_DMSC_MANU_DEFALSE_COLOR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_DEFALSE_CBCR_CLASS_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEFALSE_GRAY_PROTECT_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_GRAD_CONTROL:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_PROTECT_THR_LOW:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_PROTECT_SHIFT:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CBCR_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_COLOR_RANGE_MODE:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CB_LOW_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CB_HIGH_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CR_LOW_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CR_HIGH_THR:
        case VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_DIR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_LARGE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_DIR0_INTERP_TYPE:
        case VVCAM_ISP_CID_DMSC_MANU_DECLINE_CORNER_LOW_THR:
        case VVCAM_ISP_CID_DMSC_MANU_DECLINE_CORNER_SHIFT:
        case VVCAM_ISP_CID_DMSC_STAT_GRAD_THR_MIN:
        case VVCAM_ISP_CID_DMSC_STAT_GRAD_THR_MAX:
        case VVCAM_ISP_CID_DMSC_STAT_DIFF_COLOR_COEF:
        case VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_MODE:
        case VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_THR_LOW:
        case VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_THR_SHIFT:
        case VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_MIN:
        case VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_MAX:
        case VVCAM_ISP_CID_DMSC_STAT_HIGH_LIGH_CONTROL_THR:
        case VVCAM_ISP_CID_DMSC_STAT_HIGH_LIGH_CONTROL_STRENGTH:
        case VVCAM_ISP_CID_DMSC_STAT_DEBURST_MODE:
        case VVCAM_ISP_CID_DMSC_STAT_PURE_G_WEIGHT:
        case VVCAM_ISP_CID_DMSC_STAT_PURE_RB_WEIGHT:
        case VVCAM_ISP_CID_DMSC_STAT_PURE_G_WEIGHT_ISO:
        case VVCAM_ISP_CID_DMSC_STAT_PURE_RB_WEIGHT_ISO:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_A_BLUE:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_A_RED:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_B_BLUE:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_B_RED:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_C_BLUE:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_C_RED:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_CENTER_H_OFFSET:
        case VVCAM_ISP_CID_DMSC_STAT_CAC_CENTER_V_OFFSET:
        case VVCAM_ISP_CID_DMSC_STAT_DEMOIRE_REFINE_G_THR:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_T1:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_SHIFT1:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_R1:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_R2:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_DENOISE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_FACTOR_WHITE:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_FACTOR_BLACK:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CLIP_WHITE:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CLIP_BLACK:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T1:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T2_SHIFT:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T3:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T4_SHIFT:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_R1:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_R2:
        case VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_R3:
        case VVCAM_ISP_CID_DMSC_STAT_DEFALSE_COLOR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_STAT_DEFALSE_CBCR_CLASS_THR:
        case VVCAM_ISP_CID_DMSC_STAT_DEFALSE_GRAY_PROTECT_STRENGTH:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_GRAD_CONTROL:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_PROTECT_THR_LOW:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_PROTECT_SHIFT:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_THR:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CBCR_MODE:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_SAT_SHRINK:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_COLOR_RANGE_MODE:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CB_LOW_THR:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CB_HIGH_THR:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CR_LOW_THR:
        case VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CR_HIGH_THR:
        case VVCAM_ISP_CID_DMSC_STAT_INTERP_CORNER_DIR_STRENGTH:
        case VVCAM_ISP_CID_DMSC_STAT_INTERP_CORNER_LARGE_STRENGTH:
        case VVCAM_ISP_CID_DMSC_STAT_INTERP_CORNER_DIR0_INTERP_TYPE:
        case VVCAM_ISP_CID_DMSC_STAT_DECLINE_CORNER_LOW_THR:
        case VVCAM_ISP_CID_DMSC_STAT_DECLINE_CORNER_SHIFT:
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
        .id   = VVCAM_ISP_CID_DMSC_DEFALSE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_defalse_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_INTERP_CORNER_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_interp_corner_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_DECLINE_ENABLE,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_decline_enable",
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
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_INTERP_DIR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_interp_dir_str",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {20, 4, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_INTERP_LARGE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_interp_large_str",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {20, 2, 0, 0},
    },
    {
        /* bool 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DECLINE_ENABLE,
        .type = V4L2_CTRL_TYPE_U32,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_decline_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_HIGH_LIGHT_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_high_light_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {20, 2, 0, 0},
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
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_THRESHOLD,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_dpl_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
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
        /* uint8_t 20x8x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_DEPURPLE_CBCR_RANGE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_dpl_cbcr_range",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {20, 8, 0, 0},
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
        /* bool 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_ENABLE,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_enable",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x6x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_FACTOR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_fac",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {20, 6, 0, 0},
    },
    {
        /* uint16_t 20x6x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CLIP,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_clip",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {20, 6, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv0_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv0_t2_s",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv0_t3",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_T4_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv0_t4_s",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv0_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 1, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv0_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE0_R3,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv0_r3",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv1_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv1_t2_s",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv1_t3",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_T4_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv1_t4_s",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv1_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 1, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv1_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE1_R3,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv1_r3",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv2_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv2_t2_s",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv2_t3",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint8_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_T4_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv2_t4_s",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv2_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 1, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv2_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        /* uint16_t 20x array */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_AUTO_SHARPEN_CURVE2_R3,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_auto_sharpen_crv_r3",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {20, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_GRAD_THR_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_grad_thr_min",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_GRAD_THR_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_grad_thr_max",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DIFF_COLOR_COEF,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_diff_color_coef",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_adpt_coef_mode",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_THR_LOW,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_adpt_coef_thr_low",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_THR_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_adpt_coef_thr_st",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MIN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_adpt_coef_min",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_ADAP_COEF_MAX,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_adpt_coef_max",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_HIGH_LIGH_CONTROL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_hl_ctrl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_HIGH_LIGH_CONTROL_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_hl_ctrl_str",
        .step = 1,
        .min  = 0,
        .max  = 9,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEBURST_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_deburst_mode",
        .step = 1,
        .min  = 0,
        .max  = 7,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_PURE_G_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_pure_g_wt",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_PURE_RB_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_pure_rb_wt",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_PURE_G_WEIGHT_ISO,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_pure_g_wt_iso",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_PURE_RB_WEIGHT_ISO,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_pure_rb_wt_iso",
        .step = 1,
        .min  = 0,
        .max  = 16,
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
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEMOIRE_REFINE_G_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dmr_refine_g_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
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
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_line_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_LINE_SHIFT1,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_line_st1",
        .step = 1,
        .min  = 0,
        .max  = 11,
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
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_DENOISE_STRENGTH,
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
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_FACTOR_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_fac_wht",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {3, 0, 0, 0},
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
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_clip_wht",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CLIP_BLACK,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_clip_blk",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_curve_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_curve_st2",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_curve_t3",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_T4_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_curve_st4",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_curve_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_curve_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_SHARPEN_CURVE_R3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_sharpen_curve_r3",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEFALSE_COLOR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_defalse_color_str",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEFALSE_CBCR_CLASS_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_defalse_cbcr_ct",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEFALSE_GRAY_PROTECT_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_defalse_gps",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_GRAD_CONTROL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_grad_ctrl",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_PROTECT_THR_LOW,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_prot_thr_low",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_PROTECT_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_prot_shift",
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
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CBCR_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_cbcr_mode",
        .step = 1,
        .min  = 0,
        .max  = 3,
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
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_COLOR_RANGE_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_crm",
        .step = 1,
        .min  = 0,
        .max  = 7,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CB_LOW_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_cb_low_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CB_HIGH_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_cb_high_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CR_LOW_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_cr_low_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DEPURPLE_CR_HIGH_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dpl_cr_high_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_DIR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_ic_dir_str",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {4, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_LARGE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_ic_large_str",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_INTERP_CORNER_DIR0_INTERP_TYPE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_ic_dir0_int_type",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DECLINE_CORNER_LOW_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dc_cor_low_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_MANU_DECLINE_CORNER_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_manu_dc_cor_shift",
        .step = 1,
        .min  = 0,
        .max  = 12,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_GRAD_THR_MIN,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_grad_thr_min",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_GRAD_THR_MAX,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_grad_thr_max",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DIFF_COLOR_COEF,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_diff_color_coef",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_adpt_coef_mode",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_THR_LOW,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_adpt_coef_thr_low",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_THR_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_adpt_coef_thr_st",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_MIN,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_adpt_coef_min",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_ADAP_COEF_MAX,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_adpt_coef_max",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_HIGH_LIGH_CONTROL_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_hl_ctrl_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_HIGH_LIGH_CONTROL_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_hl_ctrl_str",
        .step = 1,
        .min  = 0,
        .max  = 9,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEBURST_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_deburst_mode",
        .step = 1,
        .min  = 0,
        .max  = 7,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_PURE_G_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_pure_g_wt",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_PURE_RB_WEIGHT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_pure_rb_wt",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_PURE_G_WEIGHT_ISO,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_pure_g_wt_iso",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_PURE_RB_WEIGHT_ISO,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_pure_rb_wt_iso",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {1, 0, 0, 0},
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_A_BLUE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_a_blue",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_A_RED,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_a_red",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_B_BLUE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_b_blue",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_B_RED,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_b_red",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_C_BLUE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_c_blue",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* float -16.0~15.9375*/
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_C_RED,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_c_red",
        .step = 1,
        .min  = -256,
        .max  = 255,
    },
    {
        /* according to image width */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_CENTER_H_OFFSET,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_ctr_h_offset",
        .step = 1,
        .min  = -32768,
        .max  = 32767,
    },
    {
        /* according to image height */
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_CAC_CENTER_V_OFFSET,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_cac_ctr_v_offset",
        .step = 1,
        .min  = -32768,
        .max  = 32767,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEMOIRE_REFINE_G_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dmr_refine_g_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_line_str",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_line_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_SHIFT1,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_line_st1",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_line_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_LINE_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_line_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_DENOISE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_denoise_str",
        .step = 1,
        .min  = 0,
        .max  = 32,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_FACTOR_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_fac_wht",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_FACTOR_BLACK,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_fac_blk",
        .step = 1,
        .min  = 0,
        .max  = 511,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CLIP_WHITE,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_clip_wht",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CLIP_BLACK,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_clip_blk",
        .step = 1,
        .min  = 0,
        .max  = 1024,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_curve_t1",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T2_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_curve_st2",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_curve_t3",
        .step = 1,
        .min  = 0,
        .max  = 2047,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_T4_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_curve_st4",
        .step = 1,
        .min  = 0,
        .max  = 11,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_R1,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_curve_r1",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_R2,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_curve_r2",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_SHARPEN_CURVE_R3,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_sharpen_curve_r3",
        .step = 1,
        .min  = 0,
        .max  = 256,
        .dims = {3, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEFALSE_COLOR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_defalse_color_str",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEFALSE_CBCR_CLASS_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_defalse_cbcr_ct",
        .step = 1,
        .min  = 0,
        .max  = 7,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEFALSE_GRAY_PROTECT_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_defalse_gps",
        .step = 1,
        .min  = 0,
        .max  = 128,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_GRAD_CONTROL,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_grad_ctrl",
        .step = 1,
        .min  = 0,
        .max  = 1,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_PROTECT_THR_LOW,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_prot_thr_low",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_PROTECT_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_prot_shift",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CBCR_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_cbcr_mode",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_SAT_SHRINK,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_sat_shrink",
        .step = 1,
        .min  = 0,
        .max  = 8,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_COLOR_RANGE_MODE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_crm",
        .step = 1,
        .min  = 0,
        .max  = 7,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CB_LOW_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_cb_low_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CB_HIGH_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_cb_high_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CR_LOW_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_cr_low_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DEPURPLE_CR_HIGH_THR,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dpl_cr_high_thr",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_INTERP_CORNER_DIR_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_ic_dir_str",
        .step = 1,
        .min  = 0,
        .max  = 255,
        .dims = {4, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_INTERP_CORNER_LARGE_STRENGTH,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_ic_large_str",
        .step = 1,
        .min  = 0,
        .max  = 16,
        .dims = {2, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_INTERP_CORNER_DIR0_INTERP_TYPE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_ic_dir0_int_type",
        .step = 1,
        .min  = 0,
        .max  = 3,
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DECLINE_CORNER_LOW_THR,
        .type = V4L2_CTRL_TYPE_U16,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dc_cor_low_thr",
        .step = 1,
        .min  = 0,
        .max  = 4095,
        .dims = {1, 0, 0, 0},
    },
    {
        .ops  = &vvcam_isp_dmsc_ctrl_ops,
        .id   = VVCAM_ISP_CID_DMSC_STAT_DECLINE_CORNER_SHIFT,
        .type = V4L2_CTRL_TYPE_U8,
        .flags= V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE,
        .name = "isp_dmsc_stat_dc_cor_shift",
        .step = 1,
        .min  = 0,
        .max  = 12,
        .dims = {1, 0, 0, 0},
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

