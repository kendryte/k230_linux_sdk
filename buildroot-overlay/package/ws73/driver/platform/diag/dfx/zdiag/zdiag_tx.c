/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag tx
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_tx.h"
#include "zdiag_channel.h"
#include "zdiag_mem.h"
#include "zdiag_debug.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_dfx.h"

TD_PRV td_void zdiag_modify_pkt_before_2_hso(diag_channel_item *chan, msp_mux_packet_head_stru *mux_head)
{
    if (mux_head->type == MUX_PKT_IND) {
        mux_head->type = MUX_PKT_CMD;
    }

    mux_head->au_id = chan->au_id;
    mux_head->start_flag = MUX_START_FLAG;
    mux_head->ver = 0xfd;
}

TD_PRV ext_errno zdiag_pkt_router_hcc_tx(diag_channel_item *chan, diag_pkt_malloc_result *mem,
    msp_mux_packet_head_stru *mux_head)
{
    ext_errno ret;
    zdiag_printf("zdiag_pkt_router_hcc_tx\r\n");
    uapi_unused(mux_head);
    if (chan->zcopy_tx_hook) {
        ret = chan->zcopy_tx_hook(mem);
        diag_dfx_free_pkt(mem->basic.type, mem->basic.size);
        mem->basic.buf = TD_NULL;
        return ret;
    }
    return EXT_ERR_FAILURE;
}

TD_PRV ext_errno zdiag_pkt_router_hso_tx(diag_channel_item *chan, diag_pkt_malloc_result *mem,
    msp_mux_packet_head_stru *mux_head)
{
    zdiag_printf("zdiag_pkt_router_hso_tx\r\n");
    zdiag_modify_pkt_before_2_hso(chan, mux_head);
    if (chan->tx_hook) {
        return chan->tx_hook(mem);
    }

    return EXT_ERR_FAILURE;
}

ext_errno zdiag_pkt_router_tx(diag_pkt_malloc_result *mem)
{
    ext_errno ret;
    msp_mux_packet_head_stru *mux_head = (msp_mux_packet_head_stru *)mem->basic.buf;
    zdiag_addr dst = mux_head->dst;
    diag_channel_item *chan = zdiag_dst_2_chan(dst);
    diag_addr_attribute attribute = diag_adapt_addr_2_attribute(dst);
    if ((td_u32)attribute & DIAG_ADDR_ATTRIBUTE_HSO_CONNECT) {
        ret = zdiag_pkt_router_hso_tx(chan, mem, mux_head);
    } else {
        ret = zdiag_pkt_router_hcc_tx(chan, mem, mux_head);
    }

    return ret;
}
