/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#ifndef __VVCAM_ISP_HAL_H__
#define __VVCAM_ISP_HAL_H__

#define TPG_ISP_RST_SHIFT                    24
#define TPG_ISP_RST_MASK                     0x01000000

#define ISP_MIS                              0x000005c4
#define ISP_ICR                              0x000005c8

#define MIV1_CTRL                            0x00001400
#define MIV1_MIS                             0x00001500
#define MIV1_ICR                             0x00001504

#define MIV2_CTRL                            0x00001300
#define MIV2_MIS                             0x000016d0
#define MIV2_MIS1                            0x000016d4
#define MIV2_ICR                             0x000016d8
#define MIV2_ICR1                            0x000016dc
#define MIV2_MIS2                            0x000016f0
#define MIV2_ICR2                            0x000016f4
#define MIV2_MIS3                            0x000056d8
#define MIV2_ICR3                            0x000056dc
#define MI_MIS_HDR1                          0x000072c8
#define MI_ICR_HDR1                          0x000072cc

#define ISP_FE_MIS                           0x00003D74
#define ISP_FE_ICR                           0x00003D78
#define ISP_FE_CTL                           0x00003D60
#define ISP_FE_BATCH_MODE_ICR                0x00003DAC
#define ISP_FE_BATCH_MODE_MIS                0x00003DB8


#define MIV1_MIS_FRAME_END_MASK              0x00000001

#define MIV2_CTRL_MCM_RAW_RDMA_START_MASK    0x00008000

#define MIV2_MIS_MCM_RAW_RADY_MASK           0x01000000
#define MIV2_MIS_MCM_RAW1_FRAME_END_MASK     0x00000080
#define MIV2_MIS_MCM_RAW0_FRAME_END_MASK     0x00000040
#define MIV2_MIS_MCM_RAW_FRAME_END_MASK      0x000000c0
#define MIV2_MIS_FRAME_END_MASK              0x0000003f
#define MIV2_MIS_JPD_FRAME_END_MASK          0x00000004U

#define MIV2_MIS2_HDR_RDMA_READY_MASK        0x20000700
#define MIV2_MIS2_HDR_FRAME_END_MASK         0x0800001c

#define MIV2_MIS3_MCM_G2RAW1_FRAME_END_MASK  0x00008000
#define MIV2_MIS3_MCM_G2RAW0_FRAME_END_MASK  0x00004000
#define MIV2_MIS3_MCM_G2RAW_FRAME_END_MASK   0x0000c000

#define MIV2_MIS_MP_FRAME_END_MASK           0x00000003
#define MIV2_MIS_SP_FRAME_END_MASK           0x00000008
#define MIV2_MIS_SP2_FRAME_END_MASK          0x00000030

#define ISP_MIS_FRAME_IN_MASK                0x00000020
#define ISP_MIS_FRAME_OUT_MASK               0x00000002

#define ISP_FE_CFG_SEL_MASK                  0x00000001
#define ISP_FE_CFG_SEL_SHIFT                 0
#define ISP_FE_AHB_WRITE_MASK                0x00000002
#define ISP_FE_AHB_WRITE_SHIFT               1

#define FUSA_ECC_IMSC1                       0x00005b54
#define FUSA_ECC_RIS1                        0x00005b58
#define FUSA_ECC_MIS1                        0x00005b5c
#define FUSA_ECC_ICR1                        0x00005b64
#define FUSA_ECC_IMSC2                       0x00005b68
#define FUSA_ECC_RIS2                        0x00005b6C
#define FUSA_ECC_MIS2                        0x00005b70
#define FUSA_ECC_ICR2                        0x00005b78
#define FUSA_ECC_IMSC3                       0x00005b7c
#define FUSA_ECC_RIS3                        0x00005b80
#define FUSA_ECC_MIS3                        0x00005b84
#define FUSA_ECC_ICR3                        0x00005b8c
#define FUSA_ECC_IMSC4                       0x00005b90
#define FUSA_ECC_RIS4                        0x00005b94
#define FUSA_ECC_MIS4                        0x00005b98
#define FUSA_ECC_ICR4                        0x00005ba0
#define FUSA_ECC_IMSC5                       0x00005ba4
#define FUSA_ECC_RIS5                        0x00005ba8
#define FUSA_ECC_MIS5                        0x00005bac
#define FUSA_ECC_ICR5                        0x00005bb4
#define FUSA_ECC_IMSC6                       0x00005c44
#define FUSA_ECC_RIS6                        0x00005c48
#define FUSA_ECC_MIS6                        0x00005c4c
#define FUSA_ECC_ICR6                        0x00005c54

#define FUSA_DUP_IMSC                        0x00005bb8
#define FUSA_DUP_RIS                         0x00005bbc
#define FUSA_DUP_MIS                         0x00005bc0
#define FUSA_DUP_ICR                         0x00005bc8
#define FUSA_PARITY_IMSC                     0x00005bcc
#define FUSA_PARITY_RIS                      0x00005bd0
#define FUSA_PARITY_MIS                      0x00005bd4
#define FUSA_PARITY_ICR                      0x00005bdc
#define FUSA_LV1_IMSC1                       0x00005b40
#define FUSA_LV1_RIS1                        0x00005b44
#define FUSA_LV1_MIS1                        0x00005b48
#define FUSA_LV1_ICR1                        0x00005b50


#define RDCD_MI_MIS                          0x000080c0
#define RDCD_MI_ICR                          0x000080c4

enum isp_fe_cfg_sel_e {
    ISP_FE_SEL_AHBBUF = 0,
    ISP_FE_SEL_CMDBUF = 1,
};

enum isp_fe_ahb_wr_en_e {
    ISP_FE_AHB_WR_DISABLE = 0,
    ISP_FE_AHB_WR_ENABLE  = 1,
};

#endif
