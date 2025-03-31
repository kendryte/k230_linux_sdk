/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag pkt
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_pkt.h"
#include "zdiag_adapt_layer.h"
ext_errno zdiag_check_mux_pkt(msp_mux_packet_head_stru *mux, td_u16 size)
{
    td_u16 crc_val;
    uapi_unused(size);
    if (mux->start_flag != MUX_START_FLAG) {
        return EXT_ERR_FAILURE;
    }

    crc_val = diag_adapt_crc16(0, mux->puc_packet_data, mux->packet_data_size);
    if (crc_val != mux->crc16) {
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

ext_errno zdiag_check_hcc_pkt(td_u8 *data, td_u16 size)
{
    uapi_unused(data);
    uapi_unused(size);

    return EXT_ERR_SUCCESS;
}
