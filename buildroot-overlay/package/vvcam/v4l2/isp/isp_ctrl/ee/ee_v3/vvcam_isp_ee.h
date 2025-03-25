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


#ifndef __VVCAM_ISP_EE_H__
#define __VVCAM_ISP_EE_H__

#include "vvcam_isp_ctrl.h"

#define VVCAM_ISP_CID_EE_ENABLE             (VVCAM_ISP_CID_EE_BASE + 0x0000)
#define VVCAM_ISP_CID_EE_RESET              (VVCAM_ISP_CID_EE_BASE + 0x0001)
#define VVCAM_ISP_CID_EE_CURVE_ENABLE       (VVCAM_ISP_CID_EE_BASE + 0x0002)
#define VVCAM_ISP_CID_EE_CA_ENABLE          (VVCAM_ISP_CID_EE_BASE + 0x0003)
#define VVCAM_ISP_CID_EE_DEPURPLE_ENABLE    (VVCAM_ISP_CID_EE_BASE + 0x0004)
#define VVCAM_ISP_CID_EE_DCI_ENABLE         (VVCAM_ISP_CID_EE_BASE + 0x0005)
#define VVCAM_ISP_CID_EE_YUV_DOMAIN_ENABLE  (VVCAM_ISP_CID_EE_BASE + 0x0006)
#define VVCAM_ISP_CID_EE_SHARPEN_LIMIT_ENABLE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0007)
#define VVCAM_ISP_CID_EE_SKIN_PROC_ENABLE   (VVCAM_ISP_CID_EE_BASE + 0x0008)
#define VVCAM_ISP_CID_EE_MODE               (VVCAM_ISP_CID_EE_BASE + 0x0009)
#define VVCAM_ISP_CID_EE_AUTO_LEVEL         (VVCAM_ISP_CID_EE_BASE + 0x000A)
#define VVCAM_ISP_CID_EE_AUTO_GAIN          (VVCAM_ISP_CID_EE_BASE + 0x000B)
#define VVCAM_ISP_CID_EE_AUTO_EE_EN         (VVCAM_ISP_CID_EE_BASE + 0x000C)
#define VVCAM_ISP_CID_EE_AUTO_STRENGTH      (VVCAM_ISP_CID_EE_BASE + 0x000D)
#define VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH  (VVCAM_ISP_CID_EE_BASE + 0x000E)
#define VVCAM_ISP_CID_EE_AUTO_SRC_STRENGTH_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x000F)
#define VVCAM_ISP_CID_EE_AUTO_EDGE_NR_LEVEL (VVCAM_ISP_CID_EE_BASE + 0x0010)
#define VVCAM_ISP_CID_EE_AUTO_DETAIL_LEVEL  (VVCAM_ISP_CID_EE_BASE + 0x0011)
#define VVCAM_ISP_CID_EE_AUTO_DETAIL_PRE_ENHANCE_STR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0012)
#define VVCAM_ISP_CID_EE_AUTO_SKIN_PROC_EN  (VVCAM_ISP_CID_EE_BASE + 0x0013)
#define VVCAM_ISP_CID_EE_AUTO_SKIN_DETECT_STR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0014)
#define VVCAM_ISP_CID_EE_AUTO_SKIN_THRESHOLD \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0015)
#define VVCAM_ISP_CID_EE_AUTO_HF_MERGE_CURVE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0016)
#define VVCAM_ISP_CID_EE_AUTO_HF_MERGE_CURVE_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0017)
#define VVCAM_ISP_CID_EE_AUTO_GRAD_THRESHOLD \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0018)
#define VVCAM_ISP_CID_EE_AUTO_GRAD_THRESHOLD_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0019)
#define VVCAM_ISP_CID_EE_AUTO_SHARP_CURVE_LEVEL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x001A)
#define VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_CURVE_LEVEL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x001B)
#define VVCAM_ISP_CID_EE_AUTO_SHARP_GAIN    (VVCAM_ISP_CID_EE_BASE + 0x001C)
#define VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_GAIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x001D)
#define VVCAM_ISP_CID_EE_AUTO_SHARP_UV_GAIN (VVCAM_ISP_CID_EE_BASE + 0x001E)
#define VVCAM_ISP_CID_EE_AUTO_SHARP_LIMIT   (VVCAM_ISP_CID_EE_BASE + 0x001F)
#define VVCAM_ISP_CID_EE_AUTO_SHARP_SKIN_LIMIT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0020)
#define VVCAM_ISP_CID_EE_AUTO_DEPURPLE_EN   (VVCAM_ISP_CID_EE_BASE + 0x0021)
#define VVCAM_ISP_CID_EE_AUTO_DPL_LIMIT     (VVCAM_ISP_CID_EE_BASE + 0x0022)
#define VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_THR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0023)
#define VVCAM_ISP_CID_EE_AUTO_DPL_DETECT_LUMA_DIFF \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0024)
#define VVCAM_ISP_CID_EE_AUTO_DPL_COMP_LUMA_DIFF \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0025)
#define VVCAM_ISP_CID_EE_AUTO_DPL_SAT_STR   (VVCAM_ISP_CID_EE_BASE + 0x0026)
#define VVCAM_ISP_CID_EE_AUTO_DPL_FIX_STR   (VVCAM_ISP_CID_EE_BASE + 0x0027)
#define VVCAM_ISP_CID_EE_AUTO_CA_ENABLE     (VVCAM_ISP_CID_EE_BASE + 0x0028)
#define VVCAM_ISP_CID_EE_AUTO_CA_LUT_CHROMA_TBL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0029)
#define VVCAM_ISP_CID_EE_AUTO_DCI_EN        (VVCAM_ISP_CID_EE_BASE + 0x002A)
#define VVCAM_ISP_CID_EE_AUTO_DCI_CURVE     (VVCAM_ISP_CID_EE_BASE + 0x002B)
#define VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH  (VVCAM_ISP_CID_EE_BASE + 0x0040)
#define VVCAM_ISP_CID_EE_MANU_SRC_STRENGTH_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0041)
#define VVCAM_ISP_CID_EE_MANU_EDGE_NR_LEVEL (VVCAM_ISP_CID_EE_BASE + 0x0042)
#define VVCAM_ISP_CID_EE_MANU_EDGE_SCALER   (VVCAM_ISP_CID_EE_BASE + 0x0043)
#define VVCAM_ISP_CID_EE_MANU_EDGE_USE_AUXI_DIR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0044)
#define VVCAM_ISP_CID_EE_MANU_DETAIL_LEVEL  (VVCAM_ISP_CID_EE_BASE + 0x0045)
#define VVCAM_ISP_CID_EE_MANU_DETAIL_SCALER (VVCAM_ISP_CID_EE_BASE + 0x0046)
#define VVCAM_ISP_CID_EE_MANU_DETAIL_PRE_ENHANCE_STR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0047)
#define VVCAM_ISP_CID_EE_MANU_SKIN_DETECT_STR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0048)
#define VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0049)
#define VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_Y_MAX \
                                            (VVCAM_ISP_CID_EE_BASE + 0x004A)
#define VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x004B)
#define VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CR_MAX \
                                            (VVCAM_ISP_CID_EE_BASE + 0x004C)
#define VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x004D)
#define VVCAM_ISP_CID_EE_MANU_SKIN_H_THR_CB_MAX \
                                            (VVCAM_ISP_CID_EE_BASE + 0x004E)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T0 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x004F)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T1_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0050)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T2 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0051)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_T3_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0052)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T0 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0053)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T1_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0054)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T2 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0055)
#define VVCAM_ISP_CID_EE_MANU_HF_MERGE_CURVE_SKIN_T3_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0056)
#define VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0057)
#define VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0058)
#define VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_EDGE_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0059)
#define VVCAM_ISP_CID_EE_MANU_CUTOFF_GRAD_THR_DETAIL_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x005A)
#define VVCAM_ISP_CID_EE_MANU_ENHANCE_STRENGTH \
                                            (VVCAM_ISP_CID_EE_BASE + 0x005B)
#define VVCAM_ISP_CID_EE_MANU_SHARP_CURVE_LEVEL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x005C)
#define VVCAM_ISP_CID_EE_MANU_SHARP_SKIN_CURVE_LEVEL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x005D)
#define VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UP (VVCAM_ISP_CID_EE_BASE + 0x005E)
#define VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x005F)
#define VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0060)
#define VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_SKIN_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0061)
#define VVCAM_ISP_CID_EE_MANU_SHARP_GAIN_UV (VVCAM_ISP_CID_EE_BASE + 0x0062)
#define VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_TYPE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0063)
#define VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0064)
#define VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0065)
#define VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0066)
#define VVCAM_ISP_CID_EE_MANU_SHARP_LIMIT_SKIN_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0067)
#define VVCAM_ISP_CID_EE_MANU_DPL_DETECT_RANGE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0068)
#define VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0069)
#define VVCAM_ISP_CID_EE_MANU_DPL_U_LIMIT_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x006A)
#define VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x006B)
#define VVCAM_ISP_CID_EE_MANU_DPL_V_LIMIT_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x006C)
#define VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_THR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x006D)
#define VVCAM_ISP_CID_EE_MANU_DPL_DETECT_LUMA_DIFF \
                                            (VVCAM_ISP_CID_EE_BASE + 0x006E)
#define VVCAM_ISP_CID_EE_MANU_DPL_COMP_LUMA_DIFF \
                                            (VVCAM_ISP_CID_EE_BASE + 0x006F)
#define VVCAM_ISP_CID_EE_MANU_DPL_SAT_STR   (VVCAM_ISP_CID_EE_BASE + 0x0070)
#define VVCAM_ISP_CID_EE_MANU_DPL_FIX_STR   (VVCAM_ISP_CID_EE_BASE + 0x0071)
#define VVCAM_ISP_CID_EE_MANU_CA_LUT_CHROMA_TBL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0072)
#define VVCAM_ISP_CID_EE_MANU_CA_MODE       (VVCAM_ISP_CID_EE_BASE + 0x0073)
#define VVCAM_ISP_CID_EE_MANU_DCI_DEGAMMA   (VVCAM_ISP_CID_EE_BASE + 0x0074)
#define VVCAM_ISP_CID_EE_MANU_DCI_LUT_LUMA_TBL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0075)
#define VVCAM_ISP_CID_EE_STAT_SKIN_PROC_ENABLE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x007F)
#define VVCAM_ISP_CID_EE_STAT_SRC_STRENGTH  (VVCAM_ISP_CID_EE_BASE + 0x0080)
#define VVCAM_ISP_CID_EE_STAT_SRC_STRENGTH_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0081)
#define VVCAM_ISP_CID_EE_STAT_EDGE_NR_LEVEL (VVCAM_ISP_CID_EE_BASE + 0x0082)
#define VVCAM_ISP_CID_EE_STAT_EDGE_SCALER   (VVCAM_ISP_CID_EE_BASE + 0x0083)
#define VVCAM_ISP_CID_EE_STAT_EDGE_USE_AUXI_DIR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0084)
#define VVCAM_ISP_CID_EE_STAT_DETAIL_LEVEL  (VVCAM_ISP_CID_EE_BASE + 0x0085)
#define VVCAM_ISP_CID_EE_STAT_DETAIL_SCALER (VVCAM_ISP_CID_EE_BASE + 0x0086)
#define VVCAM_ISP_CID_EE_STAT_DETAIL_PRE_ENHANCE_STR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0087)
#define VVCAM_ISP_CID_EE_STAT_SKIN_DETECT_STR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0088)
#define VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_Y_MIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0089)
#define VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_Y_MAX \
                                            (VVCAM_ISP_CID_EE_BASE + 0x008A)
#define VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CR_MIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x008B)
#define VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CR_MAX \
                                            (VVCAM_ISP_CID_EE_BASE + 0x008C)
#define VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CB_MIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x008D)
#define VVCAM_ISP_CID_EE_STAT_SKIN_H_THR_CB_MAX \
                                            (VVCAM_ISP_CID_EE_BASE + 0x008E)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T0 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x008F)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T1_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0090)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T2 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0091)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_T3_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0092)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T0 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0093)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T1_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0094)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T2 \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0095)
#define VVCAM_ISP_CID_EE_STAT_HF_MERGE_CURVE_SKIN_T3_SHIFT \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0096)
#define VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_EDGE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0097)
#define VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_DETAIL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0098)
#define VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_EDGE_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x0099)
#define VVCAM_ISP_CID_EE_STAT_CUTOFF_GRAD_THR_DETAIL_SKIN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x009A)
#define VVCAM_ISP_CID_EE_STAT_ENHANCE_STRENGTH \
                                            (VVCAM_ISP_CID_EE_BASE + 0x009B)
#define VVCAM_ISP_CID_EE_STAT_SHARP_CURVE_LEVEL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x009C)
#define VVCAM_ISP_CID_EE_STAT_SHARP_SKIN_CURVE_LEVEL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x009D)
#define VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_UP (VVCAM_ISP_CID_EE_BASE + 0x009E)
#define VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x009F)
#define VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_SKIN_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A0)
#define VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_SKIN_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A1)
#define VVCAM_ISP_CID_EE_STAT_SHARP_GAIN_UV (VVCAM_ISP_CID_EE_BASE + 0x00A2)
#define VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_TYPE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A3)
#define VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A4)
#define VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A5)
#define VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_SKIN_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A6)
#define VVCAM_ISP_CID_EE_STAT_SHARP_LIMIT_SKIN_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A7)
#define VVCAM_ISP_CID_EE_STAT_DPL_DETECT_RANGE \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A8)
#define VVCAM_ISP_CID_EE_STAT_DPL_U_LIMIT_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00A9)
#define VVCAM_ISP_CID_EE_STAT_DPL_U_LIMIT_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00AA)
#define VVCAM_ISP_CID_EE_STAT_DPL_V_LIMIT_UP \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00AB)
#define VVCAM_ISP_CID_EE_STAT_DPL_V_LIMIT_DOWN \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00AC)
#define VVCAM_ISP_CID_EE_STAT_DPL_DETECT_LUMA_THR \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00AD)
#define VVCAM_ISP_CID_EE_STAT_DPL_DETECT_LUMA_DIFF \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00AE)
#define VVCAM_ISP_CID_EE_STAT_DPL_COMP_LUMA_DIFF \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00AF)
#define VVCAM_ISP_CID_EE_STAT_DPL_SAT_STR   (VVCAM_ISP_CID_EE_BASE + 0x00B0)
#define VVCAM_ISP_CID_EE_STAT_DPL_FIX_STR   (VVCAM_ISP_CID_EE_BASE + 0x00B1)
#define VVCAM_ISP_CID_EE_STAT_CA_LUT_CHROMA_TBL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00B2)
#define VVCAM_ISP_CID_EE_STAT_CA_MODE       (VVCAM_ISP_CID_EE_BASE + 0x00B3)
#define VVCAM_ISP_CID_EE_STAT_DCI_DEGAMMA   (VVCAM_ISP_CID_EE_BASE + 0x00B4)
#define VVCAM_ISP_CID_EE_STAT_DCI_LUT_LUMA_TBL \
                                            (VVCAM_ISP_CID_EE_BASE + 0x00B5)
#ifdef __KERNEL__
int vvcam_isp_ee_ctrl_count(void);
int vvcam_isp_ee_ctrl_create(struct vvcam_isp_dev *isp_dev);
#endif

#endif
