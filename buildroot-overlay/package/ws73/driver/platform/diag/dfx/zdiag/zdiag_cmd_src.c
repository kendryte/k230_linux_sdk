/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag cmd producer
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_cmd_src.h"
#include "securec.h"
#include "zdiag_mem.h"
#include "zdiag_pkt_router.h"
#include "zdiag_debug.h"
#include "zdiag_adapt_layer.h"

typedef struct {
    td_u16 cmd_id;
    td_u16 data_size;
    td_u8 *data;
    diag_option *option;
} diag_cmd_api_param;

TD_PRV td_pvoid diag_cmd_init_pkt(diag_cmd_api_param *cmd_param, diag_pkt_malloc_result *mem)
{
    zdiag_addr dst = cmd_param->option->peer_addr;
    msp_mux_packet_head_stru *mux_head;
    msp_diag_head_req_stru *cmd_head;
    td_u8 *usr_data;
    diag_pkt_malloc_param param;
    diag_pkt_malloc_param_ipc *ipc;
    diag_addr_attribute attribute = diag_adapt_addr_2_attribute(cmd_param->option->peer_addr);

    param.basic.size = (td_u16)(sizeof(msp_mux_packet_head_stru) + sizeof(msp_diag_head_req_stru) +
                        cmd_param->data_size);
    if (attribute & DIAG_ADDR_ATTRIBUTE_HCC) {
        param.basic.type = DIAG_PKT_MALLOC_TYPE_IPC;
        ipc = &param.diff.ipc;
        ipc->addr = cmd_param->option->peer_addr;
    } else {
        param.basic.type = DIAG_PKT_MALLOC_TYPE_CMN;
    }

    diag_pkt_malloc(&param, mem);
    if (mem->basic.buf == TD_NULL) {
        return TD_NULL;
    }

    mux_head = (msp_mux_packet_head_stru *)(mem->basic.buf);
    cmd_head = (msp_diag_head_req_stru *)(mem->basic.buf + sizeof(msp_mux_packet_head_stru));
    usr_data = (td_u8 *)(mem->basic.buf + sizeof(msp_mux_packet_head_stru) + sizeof(msp_diag_head_req_stru));

    mux_head->start_flag = MUX_START_FLAG;
    mux_head->type = MUX_PKT_CMD;
    mux_head->ver = MUX_PKT_VER;
    mux_head->src = diag_adapt_get_local_addr();
    mux_head->crc16 = 0;
    mux_head->packet_data_size = (td_u16)(sizeof(msp_diag_head_req_stru) + cmd_param->data_size);
    mux_head->cmd_id = cmd_param->cmd_id;
    mux_head->dst = dst;

    cmd_head->cmd_id = cmd_param->cmd_id;
    cmd_head->param_size = cmd_param->data_size;
    cmd_head->sn = 0;
    cmd_head->crc16 = 0;

    if (memcpy_s(usr_data, cmd_param->data_size, cmd_param->data, cmd_param->data_size) != EOK) {
        return TD_NULL;
    }
    return mem->basic.buf;
}

ext_errno uapi_zdiag_run_cmd(td_u16 cmd_id, td_u8 *data, td_u16 data_size, diag_option *option)
{
    diag_pkt_malloc_result mem;
    diag_cmd_api_param cmd_param;
    diag_pkt_process_param process_param;

    cmd_param.cmd_id = cmd_id;
    cmd_param.data = data;
    cmd_param.data_size = data_size;
    cmd_param.option = option;

    diag_cmd_init_pkt(&cmd_param, &mem);
    if (mem.basic.buf == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    process_param.cur_proc = DIAG_PKT_PROC_USR_ASYNC_CMD_IND;
    return diag_pkt_router(&mem, &process_param);
}
