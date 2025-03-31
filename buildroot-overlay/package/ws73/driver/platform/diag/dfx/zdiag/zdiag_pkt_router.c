/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: pkt router
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_pkt_router.h"
#include "securec.h"
#include "zdiag_msg.h"
#include "zdiag_mem.h"
#include "zdiag_cmd_dst.h"
#include "zdiag_ind_dst.h"
#include "zdiag_tx.h"
#include "zdiag_debug.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_dfx.h"
#include "dfx_adapt_layer.h"

TD_PRV td_void diag_pkt_make_router_param(diag_pkt_malloc_result *mem, pkt_router_param *router_param)
{
    router_param->mux_head = (msp_mux_packet_head_stru *)mem->basic.buf;
}

TD_PRV ext_errno diag_pkt_router_forwarding_exec(diag_pkt_malloc_result *mem, pkt_router_param *router_param,
    diag_pkt_process_param *process_param)
{
    uapi_unused(router_param);
    uapi_unused(process_param);
    return zdiag_pkt_router_tx(mem);
}

TD_PRV ext_errno diag_pkt_router_local_exec(diag_pkt_malloc_result *mem, pkt_router_param *router_param,
    diag_pkt_process_param *process_param)
{
    mux_pkt_type mux_type = router_param->mux_head->type;
    diag_option option = DIAG_OPTION_INIT_VAL;
    option.peer_addr = router_param->mux_head->src;

    uapi_unused(mem);
    uapi_unused(process_param);

    zdiag_printf("diag_pkt_router_local_exec mux_type=%d\r\n", mux_type);

    switch (mux_type) {
        case MUX_PKT_CMD:
            return diag_pkt_router_run_cmd((msp_diag_head_req_stru *)(router_param->mux_head->puc_packet_data),
                &option);
        case MUX_PKT_IND:
            return diag_pkt_router_run_ind((msp_diag_head_ind_stru *)(router_param->mux_head->puc_packet_data),
                &option);
        default:
            return EXT_ERR_FAILURE;
    }
}

TD_PRV ext_errno diag_pkt_router_out_put(diag_pkt_malloc_result *mem, pkt_router_param *router_param,
    diag_pkt_process_param *process_param)
{
    diag_pkt_proc cur_proc = process_param->cur_proc;

    zdiag_printf("diag_pkt_router_out_put cur_proc = %d\r\n", cur_proc);
    if (cur_proc == DIAG_PKT_PPOC_OUT_PUT_Q || cur_proc == DIAG_PKT_PROC_USR_SYNC_CMD_IND) {
        return diag_pkt_router_forwarding_exec(mem, router_param, process_param);
    } else {
        zdiag_printf("diag_pkt_router_out_put:no async msg, cur_proc:%d.\n", cur_proc);
        return EXT_ERR_FAILURE;
    }
}

TD_PRV ext_errno diag_pkt_router_local(diag_pkt_malloc_result *mem, pkt_router_param *router_param,
    diag_pkt_process_param *process_param)
{
    switch (process_param->cur_proc) {
        case DIAG_PKT_PROC_PORT_PKT:
        case DIAG_PKT_PROC_USR_ASYNC_CMD_IND:
            zdiag_printf("diag_pkt_router_local:diag_pkt_router_local_queue no queue:%d.\n", process_param->cur_proc);
            return EXT_ERR_FAILURE;
        case DIAG_PKT_PPOC_LOCAL_EXEC_Q:
        case DIAG_PKT_PROC_USR_SYNC_CMD_IND:
            return diag_pkt_router_local_exec(mem, router_param, process_param);
        default:
            return EXT_ERR_FAILURE;
    }
}

ext_errno diag_pkt_router(diag_pkt_malloc_result *mem, diag_pkt_process_param *process_param)
{
    ext_errno ret;
    pkt_router_param router_param;
    zdiag_addr dst_addr, local_addr;

    diag_pkt_make_router_param(mem, &router_param);
    dst_addr = router_param.mux_head->dst;
    local_addr = diag_adapt_get_local_addr();
    if (dst_addr != local_addr) {
        ret = diag_pkt_router_out_put(mem, &router_param, process_param);
    } else {
        ret = diag_pkt_router_local(mem, &router_param, process_param);
    }

    diag_pkt_free(mem);
    return ret;
}

td_void diag_pkt_msg_proc(td_u32 msg_id, td_u8 *msg, td_u32 msg_len)
{
    diag_pkt_msg *pkt_msg = (diag_pkt_msg *)msg;
    diag_pkt_process_param process_param;
    process_param.cur_proc = pkt_msg->cur_proc;
    diag_pkt_router(&pkt_msg->mem, &process_param);
    uapi_unused(msg_id);
    uapi_unused(msg_len);
}
