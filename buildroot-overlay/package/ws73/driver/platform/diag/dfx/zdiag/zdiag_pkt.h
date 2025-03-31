/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag pkt
 * This file should be changed only infrequently and with great care.
 */
#ifndef __ZDIAG_PKT_H__
#define __ZDIAG_PKT_H__
#include "zdiag_common.h"

ext_errno zdiag_check_mux_pkt(msp_mux_packet_head_stru *mux, td_u16 size);
ext_errno zdiag_check_hcc_pkt(td_u8 *data, td_u16 size);

#endif
