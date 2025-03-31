/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag ind producer
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_ind_src.h"
#include "securec.h"
#include "zdiag_mem.h"
#include "zdiag_pkt_router.h"
#include "zdiag_debug.h"
#include "soc_diag_cmd_id.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_dfx.h"
#if defined(CONFIG_ZDIAG_VAL) && defined(CONFIG_ZDIAG_CACHE_VAL_IPC) && \
    (CONFIG_ZDIAG_VAL == CONFIG_ZDIAG_CACHE_VAL_IPC)
#include "zdiag_cache.h"
#endif
#include "zdiag_filter.h"
#include "diag_adapt_layer.h"

psd_enable_callback g_psd_enable_callback = TD_NULL;

typedef struct {
    td_u16 cnt : 3;
    td_u16 remote_cache : 1;
    td_u16 sync : 1;
    td_u16 pkt_purpose : 3;
    td_u16 total_size;
    td_u16 cmd_id;
    diag_option *option;
    const td_u8 *buf[4];
    td_u16 size[4];
} diag_out_buf_list;

TD_PRV td_void zdiag_mk_log_pkt(diag_cmd_log_layer_ind_stru *log_pkt, td_u32 module_id, td_u32 msg_id)
{
    TD_PRV td_u32 msg_sn = 0;
    log_pkt->module = module_id;
    log_pkt->dest_mod = diag_adapt_get_local_addr();
    log_pkt->no = msg_sn++;
    log_pkt->id = msg_id;
    log_pkt->time = diag_adapt_get_msg_time(); /* realized in adapt layer */
}

TD_PRV td_bool zdiag_msg_permit(td_u32 module_id, td_u8 level)
{
    TD_PRV td_bool permit = TD_FALSE;
    if (permit == TD_FALSE) {
        if (diag_adapt_get_cur_second() < 5) { /* 5: temp forbid log first 5 second */
            return TD_FALSE;
        }
        permit = TD_TRUE;
    }

    if (zdiag_log_enable(level, module_id) == TD_FALSE) {
        return TD_FALSE;
    }
    return TD_TRUE;
}

TD_PRV td_void diag_make_mux_pkt(diag_pkt_malloc_result *mem, diag_out_buf_list *buf_list)
{
    diag_addr_attribute attribute = diag_adapt_addr_2_attribute(buf_list->option->peer_addr);
    diag_pkt_malloc_param param;
    diag_pkt_malloc_param_ipc *ipc;
    msp_mux_packet_head_stru *mux_head;
    td_u32 offset;
    int i;

    mem->basic.buf = TD_NULL;
    param.basic.size = (td_u16)(buf_list->total_size + sizeof(msp_mux_packet_head_stru));
    if (attribute & DIAG_ADDR_ATTRIBUTE_HCC) {
        ipc = &param.diff.ipc;
        param.basic.type = DIAG_PKT_MALLOC_TYPE_IPC;
        ipc->addr = buf_list->option->peer_addr;
    } else {
        param.basic.type = DIAG_PKT_MALLOC_TYPE_CMN;
    }

    diag_pkt_malloc(&param, mem);
    if (mem->basic.buf == TD_NULL) {
        return;
    }

    mux_head = (msp_mux_packet_head_stru *)mem->basic.buf;
    offset = (td_u32)(sizeof(msp_mux_packet_head_stru));
    for (i = 0; i < buf_list->cnt; i++) {
        if (memcpy_s(mem->basic.buf + offset, mem->basic.size - offset, buf_list->buf[i], buf_list->size[i]) != EOK) {
            continue;
        }
        offset += buf_list->size[i];
    }
    mux_head->start_flag = MUX_START_FLAG;
    mux_head->type = buf_list->pkt_purpose;
    mux_head->ver = MUX_PKT_VER;
    mux_head->src = diag_adapt_get_local_addr();
    mux_head->crc16 = 0;
    mux_head->packet_data_size = buf_list->total_size;
    mux_head->cmd_id = buf_list->cmd_id;
    mux_head->dst = buf_list->option->peer_addr;
    mux_head->ctrl = 0;
    mux_head->pad[0] = 0;

    mux_head->crc16 = diag_adapt_crc16(0, mux_head->puc_packet_data, mux_head->packet_data_size);
}

TD_PRV ext_errno diag_out_put_buf_list(diag_out_buf_list *buf_list)
{
    diag_addr_attribute attribute = diag_adapt_addr_2_attribute(buf_list->option->peer_addr);
    diag_pkt_malloc_result mem;
    diag_pkt_process_param process_param;
    if (attribute & DIAG_ADDR_ATTRIBUTE_HCC) {
#if defined(CONFIG_ZDIAG_VAL) && defined(CONFIG_ZDIAG_CACHE_VAL_IPC) && \
    (CONFIG_ZDIAG_VAL == CONFIG_ZDIAG_CACHE_VAL_IPC)
        if (buf_list->remote_cache) {
            return zdiag_put_data_2_cache(0, buf_list->buf, buf_list->size, buf_list->cnt);
        }
#endif
        diag_make_mux_pkt(&mem, buf_list);
        if (mem.basic.buf == TD_NULL) {
            return EXT_ERR_PTR_NULL;
        }

        process_param.cur_proc = DIAG_PKT_PROC_USR_ASYNC_CMD_IND;
        return diag_pkt_router(&mem, &process_param);
    } else if (attribute & DIAG_ADDR_ATTRIBUTE_HSO_CONNECT) {
        diag_make_mux_pkt(&mem, buf_list);
        if (mem.basic.buf == TD_NULL) {
            return EXT_ERR_PTR_NULL;
        }

        if (buf_list->sync) {
            process_param.cur_proc = DIAG_PKT_PROC_USR_SYNC_CMD_IND;
        } else {
            process_param.cur_proc = DIAG_PKT_PROC_USR_ASYNC_CMD_IND;
        }
        return diag_pkt_router(&mem, &process_param);
    }
    return EXT_ERR_INVALID_PARAM;
}

ext_errno uapi_zdiag_report_packet(td_u16 cmd_id, diag_option *option, TD_CONST td_u8 *packet, td_u16 packet_size,
    td_bool sync)
{
    msp_diag_head_ind_stru ind_head;
    diag_out_buf_list buf_list = { 0 };

    ind_head.cmd_id = cmd_id;
    ind_head.param_size = packet_size;

    buf_list.cnt = 2;
    buf_list.cmd_id = cmd_id;
    buf_list.buf[0] = (td_u8 *)&ind_head;
    buf_list.size[0] = (td_u16)sizeof(msp_diag_head_ind_stru);
    buf_list.total_size += buf_list.size[0];
    buf_list.buf[1] = (const td_u8 *)packet;
    buf_list.size[1] = packet_size;
    buf_list.total_size += buf_list.size[1];

    buf_list.option = option;
    if (buf_list.option->peer_addr == 0) {
        buf_list.option->peer_addr = zdiag_get_connect_tool_addr();
    }

    if (zdiag_is_enable() == TD_FALSE) {
        return EXT_ERR_FAILURE;
    }

    buf_list.remote_cache = TD_FALSE;
    buf_list.sync = sync;
    buf_list.pkt_purpose = MUX_PKT_IND;
    return diag_out_put_buf_list(&buf_list);
}

/**
 * @brief zdiag report packet with buffer
 *
 * @param cmd_head: cmd packet header
 * @param head_size: packet header size
 * @param cmd_body: cmd packet body
 * @param body_size: packet body size
 *
 * @retval
 * ps:should be api; not support remote_cache; not support async, fix syncly report
 */
ext_errno zdiag_report_packet_with_buf(td_u16 cmd_id, TD_CONST td_u8 *cmd_head, td_u16 head_size,
    TD_CONST td_u8 *cmd_body, td_u16 body_size)
{
    msp_diag_head_ind_stru ind_head;
    diag_option option = DIAG_OPTION_INIT_VAL;
    diag_out_buf_list buf_list = { 0 };

    if (zdiag_is_enable() == TD_FALSE) {
        return EXT_ERR_FAILURE;
    }

    ind_head.cmd_id = cmd_id;
    ind_head.param_size = head_size + body_size;

    buf_list.cmd_id = cmd_id;
    buf_list.cnt = 2; /* array cnt */
    buf_list.total_size = 0;

    // fill buf_list.buf[0] with msp_diag_head_ind_stru
    buf_list.buf[0] = (const td_u8 *)&ind_head;
    buf_list.size[0] = (td_u16)(sizeof(msp_diag_head_ind_stru));
    buf_list.total_size += buf_list.size[0];
    zdiag_print("buf_list.total_size: %d, now for mux head! \n", buf_list.total_size);

    // fill buf_list.buf[1] with diag_cmd_ind_stru
    buf_list.buf[1] = (const td_u8 *)cmd_head;
    buf_list.size[1] = head_size; // sizeof (diag_cmd_ind_stru)
    buf_list.total_size += buf_list.size[1];
    zdiag_print("buf_list.total_size: %d, now for cmd head! \n", buf_list.total_size);

    // fill buf_list.buf[2] with cmd_body
    if ((cmd_body != NULL) && (body_size != 0)) {
        buf_list.cnt = 3;
        buf_list.buf[2] = (const td_u8 *)cmd_body;
        buf_list.size[2] = body_size;            /* cmd body size */
        buf_list.total_size += buf_list.size[2]; /* cmd body total_size */
        zdiag_print("buf_list.total_size: %d, now for cmd body! \n", buf_list.total_size);
    }

    buf_list.option = &option;
    if (buf_list.option->peer_addr == 0) {
        buf_list.option->peer_addr = zdiag_get_connect_tool_addr();
    }

    buf_list.remote_cache = TD_FALSE;
    buf_list.sync = TD_TRUE;
    buf_list.pkt_purpose = MUX_PKT_IND;
    return diag_out_put_buf_list(&buf_list);
}

ext_errno uapi_zdiag_report_ack(msp_diag_ack_param *ack, diag_option *option)
{
    diag_out_buf_list buf_list = { 0 };

    buf_list.cnt = 2; /* 2 array cnt */
    buf_list.cmd_id = ack->cmd_id;
    buf_list.buf[0] = (td_u8 *)ack;
    buf_list.size[0] = (td_u16)(sizeof(msp_diag_ack_param) - sizeof(td_u8 *));
    buf_list.total_size += buf_list.size[0];

    buf_list.buf[1] = (td_u8 *)ack->param;
    buf_list.size[1] = ack->param_size;
    buf_list.total_size += buf_list.size[1];
    buf_list.option = option;
    buf_list.remote_cache = TD_FALSE;
    buf_list.sync = TD_TRUE;
    buf_list.pkt_purpose = MUX_PKT_ACK;
    return diag_out_put_buf_list(&buf_list);
}

ext_errno uapi_zdiag_report_sys_msg_instance(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size,
    td_u8 level)
{
    msp_diag_head_ind_stru ind_head;
    diag_cmd_log_layer_ind_stru log_pkt;
    diag_option option = DIAG_OPTION_INIT_VAL;
    diag_out_buf_list buf_list = { 0 };

    zdiag_addr tool_addr = zdiag_get_connect_tool_addr();
    option.peer_addr = tool_addr;

    if (TD_FALSE == zdiag_msg_permit(module_id, level)) {
        return EXT_ERR_INVALID_STATE;
    }

    zdiag_mk_log_pkt(&log_pkt, module_id, msg_id);

    ind_head.cmd_id = DIAG_CMD_MSG_RPT_SYS;
    ind_head.param_size = (td_u16)(sizeof(diag_cmd_log_layer_ind_stru) + buf_size);

    buf_list.cnt = 2; /* buf array cnt */
    buf_list.cmd_id = DIAG_CMD_MSG_RPT_SYS;
    buf_list.buf[0] = (td_u8 *)&ind_head;
    buf_list.size[0] = (td_u16)(sizeof(msp_diag_head_ind_stru));
    buf_list.total_size += buf_list.size[0];

    buf_list.buf[1] = (td_u8 *)&log_pkt;
    buf_list.size[1] = (td_u16)(sizeof(diag_cmd_log_layer_ind_stru));
    buf_list.total_size += buf_list.size[1];

    if ((buf != NULL) && (buf_size != 0)) {
        buf_list.cnt = 3; /* 3 array cnt */
        buf_list.buf[2] = buf;                   /* 2 array idx */
        buf_list.size[2] = buf_size;             /* 2 array idx */
        buf_list.total_size += buf_list.size[2]; /* 2 array idx */
        zdiag_print("buf_list.size[2] buf_size: %d\n", buf_size);
    }

    buf_list.option = &option;
    buf_list.remote_cache = TD_TRUE;
    buf_list.sync = TD_FALSE;
    buf_list.pkt_purpose = MUX_PKT_CMD;
    return diag_out_put_buf_list(&buf_list);
}

td_void uapi_zdiag_psd_enable_register(psd_enable_callback cb)
{
    g_psd_enable_callback = cb;
}

td_void uapi_zdiag_psd_enable_unregister(td_void)
{
    g_psd_enable_callback = TD_NULL;
}

psd_enable_callback uapi_zdiag_psd_enable_get_cb(td_void)
{
    return g_psd_enable_callback;
}

#ifdef _PRE_PRODUCT_ID_MP0X_HOST
EXPORT_SYMBOL(uapi_zdiag_psd_enable_register);
EXPORT_SYMBOL(uapi_zdiag_psd_enable_unregister);
#endif
