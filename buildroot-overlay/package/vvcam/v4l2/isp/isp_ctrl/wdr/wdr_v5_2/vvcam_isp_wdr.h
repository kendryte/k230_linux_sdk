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


#ifndef __VVCAM_ISP_WDR_H__
#define __VVCAM_ISP_WDR_H__

#include "vvcam_isp_ctrl.h"

#define VVCAM_ISP_CID_WDR_ENABLE            (VVCAM_ISP_CID_WDR_BASE + 0x0000)
#define VVCAM_ISP_CID_WDR_RESET             (VVCAM_ISP_CID_WDR_BASE + 0x0001)
#define VVCAM_ISP_CID_WDR_HALO_COLOR_FADING_ENABLE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0002)
#define VVCAM_ISP_CID_WDR_MODE              (VVCAM_ISP_CID_WDR_BASE + 0x0003)
#define VVCAM_ISP_CID_WDR_AUTO_LEVEL        (VVCAM_ISP_CID_WDR_BASE + 0x0004)
#define VVCAM_ISP_CID_WDR_AUTO_GAIN         (VVCAM_ISP_CID_WDR_BASE + 0x0005)
#define VVCAM_ISP_CID_WDR_AUTO_H_STRENGTH   (VVCAM_ISP_CID_WDR_BASE + 0x0006)
#define VVCAM_ISP_CID_WDR_AUTO_L_STRENGTH   (VVCAM_ISP_CID_WDR_BASE + 0x0007)
#define VVCAM_ISP_CID_WDR_AUTO_ENTROPY_BASE (VVCAM_ISP_CID_WDR_BASE + 0x0008)
#define VVCAM_ISP_CID_WDR_AUTO_LUMA_THR     (VVCAM_ISP_CID_WDR_BASE + 0x0009)
#define VVCAM_ISP_CID_WDR_AUTO_ENTROPY_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x000A)
#define VVCAM_ISP_CID_WDR_AUTO_FLAT_LEVEL   (VVCAM_ISP_CID_WDR_BASE + 0x000B)
#define VVCAM_ISP_CID_WDR_AUTO_GLOBAL_FLAT_LEVEL \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x000C)
#define VVCAM_ISP_CID_WDR_AUTO_SAT_RANGE    (VVCAM_ISP_CID_WDR_BASE + 0x000D)
#define VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_DOWN \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x000E)
#define VVCAM_ISP_CID_WDR_AUTO_SAT_THR_GAIN_UP \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x000F)
#define VVCAM_ISP_CID_WDR_AUTO_DEGAMMA      (VVCAM_ISP_CID_WDR_BASE + 0x0010)
#define VVCAM_ISP_CID_WDR_AUTO_HLC_BASE_LOG (VVCAM_ISP_CID_WDR_BASE + 0x0011)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_SAT_L_THR \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0012)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_SAT_H_THR \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0013)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_R_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0014)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_G_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0015)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_B_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0016)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_Y_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0017)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_C_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0018)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_M_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0019)
#define VVCAM_ISP_CID_WDR_AUTO_EDR_LEVEL    (VVCAM_ISP_CID_WDR_BASE + 0x001A)
#define VVCAM_ISP_CID_WDR_AUTO_EDR          (VVCAM_ISP_CID_WDR_BASE + 0x001B)
#define VVCAM_ISP_CID_WDR_AUTO_CONTRAST     (VVCAM_ISP_CID_WDR_BASE + 0x001C)
#define VVCAM_ISP_CID_WDR_AUTO_FIXED_WEIGHT (VVCAM_ISP_CID_WDR_BASE + 0x001D)
#define VVCAM_ISP_CID_WDR_AUTO_LOG_WEIGHT   (VVCAM_ISP_CID_WDR_BASE + 0x001E)
#define VVCAM_ISP_CID_WDR_AUTO_ANCHOR_X_LOG (VVCAM_ISP_CID_WDR_BASE + 0x001F)
#define VVCAM_ISP_CID_WDR_AUTO_ANCHOR_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0020)
#define VVCAM_ISP_CID_WDR_AUTO_MAX_GAIN     (VVCAM_ISP_CID_WDR_BASE + 0x0021)
#define VVCAM_ISP_CID_WDR_AUTO_HLC_SLOPE    (VVCAM_ISP_CID_WDR_BASE + 0x0022)
#define VVCAM_ISP_CID_WDR_AUTO_LIGHT_ENABLE_TBL \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0023)
#define VVCAM_ISP_CID_WDR_AUTO_COLOR_WEIGHT (VVCAM_ISP_CID_WDR_BASE + 0x0024)

#define VVCAM_ISP_CID_WDR_MANU_STRENGTH     (VVCAM_ISP_CID_WDR_BASE + 0x0025)
#define VVCAM_ISP_CID_WDR_MANU_H_STRENGTH \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0026)
#define VVCAM_ISP_CID_WDR_MANU_L_STRENGTH \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0027)
#define VVCAM_ISP_CID_WDR_MANU_CONTRAST     (VVCAM_ISP_CID_WDR_BASE + 0x0028)
#define VVCAM_ISP_CID_WDR_MANU_ENTROPY_ENABLE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0029)
#define VVCAM_ISP_CID_WDR_MANU_ENTROPY_BASE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x002A)
#define VVCAM_ISP_CID_WDR_MANU_ENTROPY_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x002B)
#define VVCAM_ISP_CID_WDR_MANU_LUMA_THR     (VVCAM_ISP_CID_WDR_BASE + 0x002C)
#define VVCAM_ISP_CID_WDR_MANU_FLAT_MODE    (VVCAM_ISP_CID_WDR_BASE + 0x002D)
#define VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL   (VVCAM_ISP_CID_WDR_BASE + 0x002E)
#define VVCAM_ISP_CID_WDR_MANU_FLAT_LEVEL_INC \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x002F)
#define VVCAM_ISP_CID_WDR_MANU_DARK_ATT_LEVEL \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0030)
#define VVCAM_ISP_CID_WDR_MANU_FIXED_WEIGHT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0031)
#define VVCAM_ISP_CID_WDR_MANU_CURVE_SELECT (VVCAM_ISP_CID_WDR_BASE + 0x0032)
#define VVCAM_ISP_CID_WDR_MANU_LOG_WEIGHT   (VVCAM_ISP_CID_WDR_BASE + 0x0033)
#define VVCAM_ISP_CID_WDR_MANU_ANCHOR_X_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0034)
#define VVCAM_ISP_CID_WDR_MANU_ANCHOR_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0035)
#define VVCAM_ISP_CID_WDR_MANU_RATIO        (VVCAM_ISP_CID_WDR_BASE + 0x0036)
#define VVCAM_ISP_CID_WDR_MANU_CURVE2_THR   (VVCAM_ISP_CID_WDR_BASE + 0x0037)
#define VVCAM_ISP_CID_WDR_MANU_CURVE2_L_SAT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0038)
#define VVCAM_ISP_CID_WDR_MANU_CURVE2_L_FAC \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0039)
#define VVCAM_ISP_CID_WDR_MANU_CURVE2_H_FAC \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x003A)
#define VVCAM_ISP_CID_WDR_MANU_GLOBAL_FLAT_LEVEL \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x003B)
#define VVCAM_ISP_CID_WDR_MANU_DEGAMMA      (VVCAM_ISP_CID_WDR_BASE + 0x003C)
#define VVCAM_ISP_CID_WDR_MANU_MAX_GAIN     (VVCAM_ISP_CID_WDR_BASE + 0x003D)
#define VVCAM_ISP_CID_WDR_MANU_MIN_GAIN     (VVCAM_ISP_CID_WDR_BASE + 0x003E)
#define VVCAM_ISP_CID_WDR_MANU_H_DIFF       (VVCAM_ISP_CID_WDR_BASE + 0x003F)
#define VVCAM_ISP_CID_WDR_MANU_L_DIFF       (VVCAM_ISP_CID_WDR_BASE + 0x0040)
#define VVCAM_ISP_CID_WDR_MANU_SAT_RANGE    (VVCAM_ISP_CID_WDR_BASE + 0x0041)
#define VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_DOWN \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0042)
#define VVCAM_ISP_CID_WDR_MANU_SAT_THR_GAIN_UP \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0043)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_SAT_L_THR \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0044)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_SAT_H_THR \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0045)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_R_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0046)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_G_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0047)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_B_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0048)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_Y_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0049)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_C_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x004A)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_M_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x004B)
#define VVCAM_ISP_CID_WDR_MANU_HLC_BASE_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x004C)
#define VVCAM_ISP_CID_WDR_MANU_HLC_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x004D)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_MODE    (VVCAM_ISP_CID_WDR_BASE + 0x004E)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_CURVE_COEF \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x004F)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_CURVE_MAX \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0050)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_CURVE_MIN \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0051)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_AVG_COEF \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0052)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_AVG_MAX \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0053)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_AVG_MIN \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0054)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_COEF_DEC_LIMIT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0055)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_COEF_INC_LIMIT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0056)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_FILTER_SIZE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0057)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_H_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0058)
#define VVCAM_ISP_CID_WDR_MANU_DAMP_L_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0059)
#define VVCAM_ISP_CID_WDR_MANU_RGB_COEF     (VVCAM_ISP_CID_WDR_BASE + 0x005A)
#define VVCAM_ISP_CID_WDR_MANU_LIGHT_WEIGHT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x005B)
#define VVCAM_ISP_CID_WDR_MANU_COLOR_WEIGHT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x005C)

#define VVCAM_ISP_CID_WDR_STAT_STRENGTH     (VVCAM_ISP_CID_WDR_BASE + 0x005D)
#define VVCAM_ISP_CID_WDR_STAT_H_STRENGTH \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x005E)
#define VVCAM_ISP_CID_WDR_STAT_L_STRENGTH \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x005F)
#define VVCAM_ISP_CID_WDR_STAT_CONTRAST     (VVCAM_ISP_CID_WDR_BASE + 0x0060)
#define VVCAM_ISP_CID_WDR_STAT_ENTROPY_ENABLE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0061)
#define VVCAM_ISP_CID_WDR_STAT_ENTROPY_BASE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0062)
#define VVCAM_ISP_CID_WDR_STAT_ENTROPY_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0063)
#define VVCAM_ISP_CID_WDR_STAT_LUMA_THR     (VVCAM_ISP_CID_WDR_BASE + 0x0064)
#define VVCAM_ISP_CID_WDR_STAT_FLAT_MODE    (VVCAM_ISP_CID_WDR_BASE + 0x0065)
#define VVCAM_ISP_CID_WDR_STAT_FLAT_LEVEL   (VVCAM_ISP_CID_WDR_BASE + 0x0066)
#define VVCAM_ISP_CID_WDR_STAT_FLAT_LEVEL_INC \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0067)
#define VVCAM_ISP_CID_WDR_STAT_DARK_ATT_LEVEL \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0068)
#define VVCAM_ISP_CID_WDR_STAT_FIXED_WEIGHT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0069)
#define VVCAM_ISP_CID_WDR_STAT_CURVE_SELECT (VVCAM_ISP_CID_WDR_BASE + 0x006A)
#define VVCAM_ISP_CID_WDR_STAT_LOG_WEIGHT   (VVCAM_ISP_CID_WDR_BASE + 0x006B)
#define VVCAM_ISP_CID_WDR_STAT_ANCHOR_X_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x006C)
#define VVCAM_ISP_CID_WDR_STAT_ANCHOR_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x006D)
#define VVCAM_ISP_CID_WDR_STAT_RATIO        (VVCAM_ISP_CID_WDR_BASE + 0x006E)
#define VVCAM_ISP_CID_WDR_STAT_CURVE2_THR   (VVCAM_ISP_CID_WDR_BASE + 0x006F)
#define VVCAM_ISP_CID_WDR_STAT_CURVE2_L_SAT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0070)
#define VVCAM_ISP_CID_WDR_STAT_CURVE2_L_FAC \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0071)
#define VVCAM_ISP_CID_WDR_STAT_CURVE2_H_FAC \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0072)
#define VVCAM_ISP_CID_WDR_STAT_GLOBAL_FLAT_LEVEL \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0073)
#define VVCAM_ISP_CID_WDR_STAT_DEGAMMA      (VVCAM_ISP_CID_WDR_BASE + 0x0074)
#define VVCAM_ISP_CID_WDR_STAT_MAX_GAIN     (VVCAM_ISP_CID_WDR_BASE + 0x0075)
#define VVCAM_ISP_CID_WDR_STAT_MIN_GAIN     (VVCAM_ISP_CID_WDR_BASE + 0x0076)
#define VVCAM_ISP_CID_WDR_STAT_H_DIFF       (VVCAM_ISP_CID_WDR_BASE + 0x0077)
#define VVCAM_ISP_CID_WDR_STAT_L_DIFF       (VVCAM_ISP_CID_WDR_BASE + 0x0078)
#define VVCAM_ISP_CID_WDR_STAT_SAT_RANGE    (VVCAM_ISP_CID_WDR_BASE + 0x0079)
#define VVCAM_ISP_CID_WDR_STAT_SAT_THR_GAIN_DOWN \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x007A)
#define VVCAM_ISP_CID_WDR_STAT_SAT_THR_GAIN_UP \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x007B)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_SAT_L_THR \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x007C)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_SAT_H_THR \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x007D)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_R_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x007E)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_G_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x007F)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_B_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0080)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_Y_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0081)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_C_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0082)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_M_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0083)
#define VVCAM_ISP_CID_WDR_STAT_HLC_BASE_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0084)
#define VVCAM_ISP_CID_WDR_STAT_HLC_SLOPE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0085)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_MODE    (VVCAM_ISP_CID_WDR_BASE + 0x0086)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_CURVE_COEF \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0087)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_CURVE_MAX \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0088)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_CURVE_MIN \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0089)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_AVG_COEF \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x008A)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_AVG_MAX \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x008B)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_AVG_MIN \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x008C)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_COEF_DEC_LIMIT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x008D)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_COEF_INC_LIMIT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x008E)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_FILTER_SIZE \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x008F)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_H_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0090)
#define VVCAM_ISP_CID_WDR_STAT_DAMP_L_THR_LOG \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0091)
#define VVCAM_ISP_CID_WDR_STAT_RGB_COEF     (VVCAM_ISP_CID_WDR_BASE + 0x0092)
#define VVCAM_ISP_CID_WDR_STAT_LIGHT_WEIGHT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0093)
#define VVCAM_ISP_CID_WDR_STAT_COLOR_WEIGHT \
                                            (VVCAM_ISP_CID_WDR_BASE + 0x0094)

#ifdef __KERNEL__
int vvcam_isp_wdr_ctrl_count(void);
int vvcam_isp_wdr_ctrl_create(struct vvcam_isp_dev *isp_dev);
#endif

#endif
