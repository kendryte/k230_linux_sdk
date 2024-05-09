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

#ifndef __VVCAM_ISP_H__
#define __VVCAM_ISP_H__
#include "vvcam_event.h"

enum {
    VVCAM_EID_ISP_MIS            = 0x00,
    VVCAM_EID_MIV2_MIS           = 0x01,
    VVCAM_EID_MIV2_MIS1          = 0x02,
    VVCAM_EID_MIV2_MIS2          = 0x03,
    VVCAM_EID_MIV2_MIS3          = 0x04,
    VVCAM_EID_MI_MIS_HDR1        = 0x05,
    VVCAM_EID_RDMA_MIS           = 0x06,
    VVCAM_EID_MCM_WR_RAW0_MIS    = 0x07,
    VVCAM_EID_MCM_WR_RAW1_MIS    = 0x08,
    VVCAM_EID_MCM_WR_G2_RAW0_MIS = 0x09,
    VVCAM_EID_MCM_WR_G2_RAW1_MIS = 0x10,
    VVCAM_EID_FE_MIS             = 0x11,
    VVCAM_EID_MIV2_MIS2_HDR      = 0x12,
    VVCAM_EID_MIV2_JDP           = 0x13,
    VVCAM_EID_FUSA_ECC_IMSC1     = 0x14,
    VVCAM_EID_FUSA_ECC_IMSC2     = 0x15,
    VVCAM_EID_FUSA_ECC_IMSC3     = 0x16,
    VVCAM_EID_FUSA_ECC_IMSC4     = 0x17,
    VVCAM_EID_FUSA_ECC_IMSC5     = 0x18,
    VVCAM_EID_FUSA_ECC_IMSC6     = 0x19,
    VVCAM_EID_FUSA_DUP_IMSC      = 0x1a,
    VVCAM_EID_FUSA_PARITY_IMSC   = 0x1b,
    VVCAM_EID_FUSA_LV1_IMSC1     = 0x1c,
    VVCAM_EID_MAX                = 0x1d
};


typedef struct {
    uint32_t addr;
    uint32_t value;
} vvcam_isp_reg_t;

#define VVCAM_ISP_IOC_MAGIC 'v'
#define VVCAM_ISP_RESET             _IOW(VVCAM_ISP_IOC_MAGIC, 0x01, uint32_t)
#define VVCAM_ISP_READ_REG          _IOWR(VVCAM_ISP_IOC_MAGIC, 0x02, vvcam_isp_reg_t)
#define VVCAM_ISP_WRITE_REG         _IOW(VVCAM_ISP_IOC_MAGIC, 0x03, vvcam_isp_reg_t)
#define VVCAM_ISP_SUBSCRIBE_EVENT   _IOW(VVCAM_ISP_IOC_MAGIC, 0x04, vvcam_subscription_t)
#define VVCAM_ISP_UNSUBSCRIBE_EVENT _IOW(VVCAM_ISP_IOC_MAGIC, 0x05, vvcam_subscription_t)
#define VVCAM_ISP_DQEVENT           _IOR(VVCAM_ISP_IOC_MAGIC, 0x06, vvcam_event_t)

#endif
