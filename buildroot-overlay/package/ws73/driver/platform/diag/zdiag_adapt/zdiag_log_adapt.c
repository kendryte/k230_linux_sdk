/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag oam log
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_log_adapt.h"
#include <linux/slab.h>
#include "soc_diag_cmd_id.h"
#include "securec.h"
#include "zdiag_channel.h"
#include "zdiag_filter.h"
#include "diag_cmd_connect.h"
#include "diag_adapt_layer.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_adapt_os.h"
#include "zdiag_tx_proc.h"
#include "soc_osal.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_DIAG_SUPPORT_LOCAL_LOG)
#include "zdiag_local_log.h"
#endif
#endif

#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
typedef td_u8 zdiag_addr;

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
    static td_u32 msg_sn = 1;
    log_pkt->module = diag_adapt_get_module_id_adjusted(module_id, msg_id); // get module_id from adapt func
    log_pkt->dest_mod = ACORE_ADDR;
    log_pkt->no = msg_sn++;
    log_pkt->id = msg_id;
    log_pkt->time = diag_adapt_get_msg_time();
}

TD_PRV td_void diag_make_mux_pkt(diag_pkt_malloc_result *mem, diag_out_buf_list *buf_list)
{
    msp_mux_packet_head_stru *mux_head;
    td_u16 offset;
    int i;

    mem->basic.buf = TD_NULL;
    mem->basic.size = buf_list->total_size + (td_u16)sizeof(msp_mux_packet_head_stru);
    mem->basic.buf = osal_kmalloc(mem->basic.size, OSAL_GFP_KERNEL);
    if (mem->basic.buf == TD_NULL) {
        return;
    }

    mux_head = (msp_mux_packet_head_stru *)mem->basic.buf;
    offset = (td_u16)sizeof(msp_mux_packet_head_stru);
    for (i = 0; i < buf_list->cnt; i++) {
        if (memcpy_s(mem->basic.buf + offset, mem->basic.size - offset, buf_list->buf[i], buf_list->size[i]) != EOK) {
            break;
        }
        offset += buf_list->size[i];
    }
    mux_head->start_flag = MUX_START_FLAG;
    mux_head->type = buf_list->pkt_purpose;
    mux_head->ver = MUX_PKT_VER;
    mux_head->src = diag_adapt_get_local_addr(); // should always be 100
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
    diag_pkt_malloc_result mem;
    msp_mux_packet_head_stru *mux_head;

    diag_make_mux_pkt(&mem, buf_list);
    if (mem.basic.buf == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    mux_head = (msp_mux_packet_head_stru *)mem.basic.buf;
    if (buf_list->sync) {
        if (mux_head->type == MUX_PKT_IND) {
            mux_head->type = MUX_PKT_CMD;
        }
        mux_head->ver = 0xfd;
        mux_head->au_id = zdiag_get_auid();
    }
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    zdiag_tx_send_host_log2diag(mem.basic.buf, mem.basic.size);
#else
    zdiag_uart_output_buf(mem.basic.buf, mem.basic.size);
#endif
    osal_kfree(mem.basic.buf);
    return EXT_ERR_SUCCESS;
}

td_bool zdiag_msg_permit(td_u32 module_id, td_u8 level)
{
    if (zdiag_log_enable(level, module_id) == TD_FALSE) {
        return TD_FALSE;
    }
    return TD_TRUE;
}

ext_errno zdiag_log_report_sys_msg_instance(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size,
    td_u8 level)
{
    msp_diag_head_ind_stru ind_head;
    diag_cmd_log_layer_ind_stru log_pkt;
    diag_option option = DIAG_OPTION_INIT_VAL;
    diag_out_buf_list buf_list = { 0 };

    if (zdiag_msg_permit(module_id, level) == TD_FALSE) {
        return EXT_ERR_INVALID_STATE;
    }

    buf_list.total_size = 0;

    zdiag_mk_log_pkt(&log_pkt, module_id, msg_id);

    ind_head.cmd_id = DIAG_CMD_MSG_RPT_SYS;
    ind_head.param_size = (td_u16)sizeof(diag_cmd_log_layer_ind_stru) + buf_size;

    buf_list.cnt = 2; /* 3 array cnt */
    buf_list.cmd_id = DIAG_CMD_MSG_RPT_SYS;
    buf_list.buf[0] = (td_u8 *)&ind_head;
    buf_list.size[0] = (td_u16)sizeof(msp_diag_head_ind_stru);
    buf_list.total_size += buf_list.size[0];
    zdiag_print("buf_list.size[0] %u\n", (td_u32)sizeof(msp_diag_head_ind_stru));

    buf_list.buf[1] = (td_u8 *)&log_pkt;
    buf_list.size[1] = (td_u16)sizeof(diag_cmd_log_layer_ind_stru);
    buf_list.total_size += buf_list.size[1];
    zdiag_print("buf_list.size[1] buf_size %u\n", (td_u32)sizeof(diag_cmd_log_layer_ind_stru));

    if ((buf != NULL) && (buf_size != 0)) {
        buf_list.cnt = 3;
        buf_list.buf[2] = buf;                   /* 2 array idx */
        buf_list.size[2] = buf_size;             /* 2 array idx */
        buf_list.total_size += buf_list.size[2]; /* 2 array idx */
        zdiag_print("buf_list.size[2] buf_size %hu\n", buf_size);
    }

    buf_list.option = &option;
    buf_list.remote_cache = TD_TRUE;
    buf_list.sync = TD_FALSE;
    buf_list.pkt_purpose = MUX_PKT_CMD;
    return diag_out_put_buf_list(&buf_list);
}

TD_PRV ext_errno zdiag_log_msg_print(td_u32 msg_id, td_u32 mod_id, td_u8 *buf, td_u32 size)
{
    td_u32 level = msg_id & ZDIAG_LOG_LEVEL_MASK;

#ifdef HOST_SOCKET_DIAG
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && \
    defined(CONFIG_DIAG_SUPPORT_LOCAL_LOG)
    if (!get_hso_connect_status() && (zdiag_log_mode_get() != DIAG_LOG_TO_FILE)) {
        return EXT_ERR_FAILURE;
    }
#else
    if (!get_hso_connect_status()) {
        return EXT_ERR_FAILURE;
    }
#endif
#endif
    return zdiag_log_report_sys_msg_instance(mod_id, msg_id, buf, size, level);
}

ext_errno zdiag_log_print0_press_prv(td_u32 msg_id, td_u32 mod_id)
{
    return zdiag_log_msg_print(msg_id, mod_id, TD_NULL, 0);
}

ext_errno zdiag_log_print1_press_prv(td_u32 msg_id, td_u32 mod_id, td_s32 param_1)
{
    zdiag_log_msg1 msg;
    msg.data0 = (td_u32)param_1;
    return zdiag_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg1));
}

ext_errno zdiag_log_print2_press_prv(td_u32 msg_id, td_u32 mod_id, td_s32 param_1, td_s32 param_2)
{
    zdiag_log_msg2 msg;
    msg.data0 = (td_u32)param_1;
    msg.data1 = (td_u32)param_2;
    return zdiag_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg2));
}

ext_errno zdiag_log_print3_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg3 *olm)
{
    zdiag_log_msg3 msg;
    msg.data0 = olm->data0;
    msg.data1 = olm->data1;
    msg.data2 = olm->data2;
    return zdiag_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg3));
}

ext_errno zdiag_log_print4_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg4 *olm)
{
    zdiag_log_msg4 msg;
    msg.data0 = olm->data0;
    msg.data1 = olm->data1;
    msg.data2 = olm->data2;
    msg.data3 = olm->data3;
    return zdiag_log_msg_print(msg_id, mod_id, (td_u8 *)&msg, sizeof(zdiag_log_msg4));
}

ext_errno zdiag_log_print_buff_press_prv(td_u32 msg_id, td_u32 mod_id, td_u8 *data, td_u32 data_size)
{
    return zdiag_log_msg_print(msg_id, mod_id, data, data_size);
}

ext_errno zdiag_report_packet(td_u16 cmd_id, diag_option *option, TD_CONST td_u8 *packet, td_u16 packet_size,
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

    buf_list.remote_cache = TD_FALSE;
    buf_list.sync = sync;
    buf_list.pkt_purpose = MUX_PKT_IND;
    return diag_out_put_buf_list(&buf_list);
}

EXPORT_SYMBOL(zdiag_log_print0_press_prv);
EXPORT_SYMBOL(zdiag_log_print1_press_prv);
EXPORT_SYMBOL(zdiag_log_print2_press_prv);
EXPORT_SYMBOL(zdiag_log_print3_press_prv);
EXPORT_SYMBOL(zdiag_log_print4_press_prv);
EXPORT_SYMBOL(zdiag_report_packet);

#else
ext_errno zdiag_log_print0_press_prv(td_u32 msg_id, td_u32 mod_id)
{
    uapi_unused(msg_id);
    uapi_unused(mod_id);
    return 0;
}

ext_errno zdiag_log_print1_press_prv(td_u32 msg_id, td_u32 mod_id, td_s32 param_1)
{
    uapi_unused(msg_id);
    uapi_unused(mod_id);
    uapi_unused(param_1);
    return 0;
}

ext_errno zdiag_log_print2_press_prv(td_u32 msg_id, td_u32 mod_id, td_s32 param_1, td_s32 param_2)
{
    uapi_unused(msg_id);
    uapi_unused(mod_id);
    uapi_unused(param_1);
    uapi_unused(param_2);
    return 0;
}

ext_errno zdiag_log_print3_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg3 *olm)
{
    uapi_unused(msg_id);
    uapi_unused(mod_id);
    uapi_unused(olm);
    return 0;
}

ext_errno zdiag_log_print4_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg4 *olm)
{
    uapi_unused(msg_id);
    uapi_unused(mod_id);
    uapi_unused(olm);
    return 0;
}

ext_errno uapi_diag_report_sys_msg(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size, td_u8 level)
{
    uapi_unused(module_id);
    uapi_unused(msg_id);
    uapi_unused(buf);
    uapi_unused(buf_size);
    uapi_unused(level);
    return 0;
}

EXPORT_SYMBOL(zdiag_log_print0_press_prv);
EXPORT_SYMBOL(zdiag_log_print1_press_prv);
EXPORT_SYMBOL(zdiag_log_print2_press_prv);
EXPORT_SYMBOL(zdiag_log_print3_press_prv);
EXPORT_SYMBOL(zdiag_log_print4_press_prv);
EXPORT_SYMBOL(uapi_diag_report_sys_msg);
#endif
