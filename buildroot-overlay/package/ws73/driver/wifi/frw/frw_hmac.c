/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Message queue processing of the IPC module.
 * Author: Huanghe
 * Create: 2021-07-02
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/

#include "frw_hmac.h"
#include "frw_hmac_adapt.h"
#include "hmac_vap.h"
#include "oal_mem_hcm.h"
#include "wlan_msg.h"
#include "frw_hmac_hcc_adapt.h"
#include "dmac_common_inc_rom.h"
#include "frw_thread.h"
#include "hmac_dfr.h"
#include "hmac_thruput_test.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_FRW_HMAC_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define frw_dbg(fmt, ...)
#define FRW_MSG_ITEM_NUM  4096

typedef struct {
    frw_msg_callback msg_cb;
#if defined(CONFIG_FRW_SUPPORT_CALLBACK_EXEC_STAT)
    osal_u16 succ_cnt;
    osal_u16 fail_cnt;
#endif
} frw_msg_item;

typedef struct {
    osal_u16 msg_num;
    osal_u16 msg_id_max0;
    osal_u16 msg_id_min1;
    osal_u16 msg_id_max;
    frw_msg_item *msg_table;
} frw_ctrl;

typedef struct {
    osal_u16 msg_type;
    osal_u8 data_type;
    osal_u8 resv;
    frw_msg_callback msg_cb[FRW_NETBUF_2H_NUM];
} frw_netbuf_ctrl;

OAL_STATIC frw_ctrl g_frw_ctrl = {0};
OAL_STATIC frw_netbuf_ctrl g_netbuf_d2h_ctrl = {0};

osal_s32 frw_netbuf_hook_register(osal_u16 netbuf_type, frw_msg_callback msg_cb)
{
    if ((msg_cb == OSAL_NULL) || (netbuf_type >= FRW_NETBUF_2H_NUM)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (g_netbuf_d2h_ctrl.msg_cb[netbuf_type] != OSAL_NULL) {
        return OAL_FAIL;
    }

    g_netbuf_d2h_ctrl.msg_cb[netbuf_type] = msg_cb;
    return OAL_SUCC;
}

osal_void frw_netbuf_hook_unregister(osal_u16 netbuf_type)
{
    if (netbuf_type >= FRW_NETBUF_2H_NUM) {
        return;
    }

    g_netbuf_d2h_ctrl.msg_cb[netbuf_type] = OSAL_NULL;
    return;
}

OAL_STATIC osal_u16 host_frw_msg_convert(osal_u16 msg_id)
{
    osal_u16 real_msg_id = msg_id;
    if ((msg_id >= g_frw_ctrl.msg_id_max0 && msg_id < g_frw_ctrl.msg_id_min1) || msg_id >= g_frw_ctrl.msg_id_max) {
        real_msg_id = g_frw_ctrl.msg_id_max - g_frw_ctrl.msg_id_min1 + g_frw_ctrl.msg_id_max0;
    } else if (msg_id >= g_frw_ctrl.msg_id_min1 && msg_id < g_frw_ctrl.msg_id_max) {
        real_msg_id = msg_id - g_frw_ctrl.msg_id_min1 + g_frw_ctrl.msg_id_max0;
    }
    return real_msg_id;
}

OAL_STATIC WIFI_HMAC_TCM_TEXT osal_s32 hmac_frw_netbuf_exec_callback(hmac_vap_stru *hmac_vap, osal_u8 data_type,
    osal_u16 netbuf_type, frw_msg *msg)
{
    osal_s32 ret = OAL_FAIL;
    g_netbuf_d2h_ctrl.msg_type = netbuf_type;
    g_netbuf_d2h_ctrl.data_type = data_type;
    if (g_netbuf_d2h_ctrl.msg_cb[netbuf_type] != OSAL_NULL) {
        ret = g_netbuf_d2h_ctrl.msg_cb[netbuf_type](hmac_vap, msg);
    }
    return ret;
}

WIFI_HMAC_TCM_TEXT osal_s32 frw_netbuf_exec_callback(osal_u16 netbuf_type, osal_u8 vap_id, frw_msg *msg)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    oal_netbuf_stru *netbuf = OSAL_NULL;

    hmac_vap = (hmac_vap_stru *)hmac_get_valid_vap(vap_id);
    if (hmac_vap == OSAL_NULL) {
        netbuf = *(oal_netbuf_stru **)(msg->data);
        if (netbuf != OSAL_NULL) {
            oal_netbuf_free(netbuf);
        }
        return OAL_FAIL;
    }

    return hmac_frw_netbuf_exec_callback(hmac_vap, 0, netbuf_type, msg);
}

OAL_STATIC osal_void frw_ctrl_init(osal_u16 d2d_msgid_begin, osal_u16 w2d_msgid_end, osal_u16 msg_id_max,
    frw_msg_item *msg_table)
{
    g_frw_ctrl.msg_table = msg_table;
    g_frw_ctrl.msg_id_max = msg_id_max;
    g_frw_ctrl.msg_num = msg_id_max - (w2d_msgid_end - d2d_msgid_begin) + 1;
    g_frw_ctrl.msg_id_max0 = d2d_msgid_begin;
    g_frw_ctrl.msg_id_min1 = w2d_msgid_end;
}

OAL_STATIC osal_void frw_ctrl_exit(osal_void)
{
    g_frw_ctrl.msg_table = OSAL_NULL;
    g_frw_ctrl.msg_id_max = 0;
    g_frw_ctrl.msg_num = 0;
    g_frw_ctrl.msg_id_max0 = 0;
    g_frw_ctrl.msg_id_min1 = 0;
}

osal_s32 frw_msg_host_init(osal_u16 d2d_msgid_begin, osal_u16 w2d_msgid_end, osal_u16 msg_id_max)
{
    frw_msg_item *table = OSAL_NULL;
    osal_u16 msg_num;
    if (msg_id_max >= FRW_MSG_ITEM_NUM || w2d_msgid_end < d2d_msgid_begin || msg_id_max < w2d_msgid_end) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (frw_thread_init() == OAL_FAIL) {
        return OAL_FAIL;
    }
    msg_num = msg_id_max - (w2d_msgid_end - d2d_msgid_begin) + 1;
    table = osal_vmalloc(msg_num * sizeof(frw_msg_item));
    if (table == OSAL_NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    (osal_void)memset_s(table, msg_num * sizeof(frw_msg_item), 0, msg_num * sizeof(frw_msg_item));
    frw_ctrl_init(d2d_msgid_begin, w2d_msgid_end, msg_id_max, table);

    return OAL_SUCC;
}

osal_s32 frw_msg_host_exit(osal_void)
{
    if (g_frw_ctrl.msg_table != OSAL_NULL) {
        osal_vfree(g_frw_ctrl.msg_table);
    }
    frw_ctrl_exit();
    (osal_void)memset_s(&g_netbuf_d2h_ctrl, sizeof(frw_netbuf_ctrl), 0, sizeof(frw_netbuf_ctrl));
    return OAL_SUCC;
}

OAL_STATIC osal_void frw_msg_adapt_convert(frw_msg_adapt_comm *msg_adapt_comm, frw_msg_adapt *msg_adapt)
{
    msg_adapt->msg_id           = msg_adapt_comm->msg_id;
    msg_adapt->vap_id           = msg_adapt_comm->vap_id;
    msg_adapt->sync             = msg_adapt_comm->sync;
    msg_adapt->type             = msg_adapt_comm->type;
    msg_adapt->rsv              = msg_adapt_comm->rsv;
    msg_adapt->cb_ret           = msg_adapt_comm->cb_ret;
    msg_adapt->msg_seq          = msg_adapt_comm->msg_seq;
    msg_adapt->cfg.data         = OSAL_NULL;
    msg_adapt->cfg.rsp          = OSAL_NULL;
    msg_adapt->cfg.data_len     = msg_adapt_comm->cfg.data_len;
    msg_adapt->cfg.rsp_buf_len  = msg_adapt_comm->cfg.rsp_buf_len;
    msg_adapt->cfg.rsp_len      = msg_adapt_comm->cfg.rsp_len;
    msg_adapt->cfg.sync         = msg_adapt_comm->cfg.sync;
    msg_adapt->cfg.type         = msg_adapt_comm->cfg.type;
    msg_adapt->cfg.rsv          = msg_adapt_comm->cfg.rsv;
}

OAL_STATIC frw_host_rcv_callback g_frw_host_rcv_cb = OSAL_NULL;
osal_void frw_host_register_rcv_hook(frw_host_rcv_callback rcv_cb)
{
    g_frw_host_rcv_cb = rcv_cb;
}

OAL_STATIC WIFI_HMAC_TCM_TEXT osal_u32 frw_hmac_rcv_mgnt_data(oal_netbuf_stru *netbuf)
{
    osal_s32 ret;
    osal_s32 netbuf_len;
    frw_msg msg = {0};
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    osal_u32 mac_header_len = 0;
    struct frw_hcc_extend_hdr *hdr = (struct frw_hcc_extend_hdr *)OAL_NETBUF_DATA(netbuf);

    if ((hdr->msg_type != HCC_MSG_TYPE_MGMT_FRAME) && (hdr->msg_type != HCC_MSG_TYPE_DATA_FRAME)) {
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }
    hmac_vap = (hmac_vap_stru *)hmac_get_valid_vap(hdr->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_TX, "{frw_hmac_rcv_mgnt_data::vap[%d] is null.}", hdr->vap_id);
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_netbuf_pull(netbuf, frw_get_head_len());
    ret = memcpy_s(oal_netbuf_cb(netbuf), sizeof(netbuf->cb), oal_netbuf_data(netbuf), OAL_MAX_CB_LEN);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_TX, "{frw_hmac_rcv_mgnt_data::memcpy fail[%d].}", ret);
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_header_len = rx_ctrl->mac_header_len;

    oal_netbuf_pull(netbuf, OAL_MAX_CB_LEN);
#ifndef _PRE_WLAN_FEATURE_WS53
    data = (osal_u8 *)(OAL_NETBUF_DATA(netbuf));
    netbuf_len = oal_netbuf_get_len(netbuf) - OAL_MAX_MAC_HDR_LEN + mac_header_len;
    ret = memmove_s(data + OAL_MAX_MAC_HDR_LEN - mac_header_len, netbuf_len, data, mac_header_len);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_TX, "{frw_hmac_rcv_mgnt_data::memmove fail[%d].}", ret);
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }
#endif
    oal_netbuf_pull(netbuf, OAL_MAX_MAC_HDR_LEN - mac_header_len);

    skb_reset_tail_pointer(netbuf);
    OAL_NETBUF_LEN(netbuf) = 0;
    oal_netbuf_put(netbuf, rx_ctrl->frame_len);

    msg.data = (osal_u8 *)&netbuf;
    msg.data_len = (osal_u16)sizeof(osal_u8 *); /* 指针长度 */
    return (osal_u32)hmac_frw_netbuf_exec_callback(hmac_vap, hdr->msg_type, hdr->msg.msg_id, &msg);
}

osal_u32 frw_rx_msg(osal_u16 msg_id, osal_u8 vap_id, osal_u32 seq, osal_s32 dev_cb_ret, frw_msg *msg)
{
    frw_dbg("\n func[%s] line[%d] msg_id[%d] seq[%d] vap_id[%d] dev_cb_ret[%d] msg[%p]\n",
        __func__, __LINE__, msg_id, seq, vap_id, dev_cb_ret, msg);

    if (g_frw_host_rcv_cb) {
        g_frw_host_rcv_cb(msg_id, vap_id, seq, dev_cb_ret, msg);
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 frw_hmac_rcv_cfg(osal_u8 *buf)
{
    frw_msg_adapt msg_adapt = {0};
    frw_msg_adapt_comm *msg_adapt_comm = OSAL_NULL;
    osal_u8 *cfg_buf = buf + OAL_HCC_HDR_LEN;
    struct frw_hcc_extend_hdr *hdr = (struct frw_hcc_extend_hdr *)cfg_buf;
    if ((hdr->msg_type != HCC_MSG_TYPE_CFG) && (hdr->msg_type != HCC_MSG_TYPE_RSP)) {
        frw_free_hcc_cfg_data(buf);
        return OAL_FAIL;
    }
    msg_adapt_comm = (frw_msg_adapt_comm *)(cfg_buf + frw_get_head_len());
    frw_dbg("\n func[%s] line[%d] msg_id[%d,%d] data_len[%d] rsp_len[%d,%d]\n",
        __func__, __LINE__, msg_adapt_comm->msg_id, msg_adapt_comm->type, msg_adapt_comm->cfg.data_len,
        msg_adapt_comm->cfg.rsp_buf_len, msg_adapt_comm->cfg.rsp_len);

    frw_msg_adapt_convert(msg_adapt_comm, &msg_adapt);
    if ((msg_adapt.type == HCC_MSG_TYPE_RSP) &&
        (msg_adapt.cfg.rsp_len != 0) && (msg_adapt.cfg.rsp_buf_len != 0)) {
        msg_adapt.cfg.rsp = (osal_u8 *)(msg_adapt_comm + 1);
        if (msg_adapt.cfg.rsp_len > msg_adapt.cfg.rsp_buf_len) {
            msg_adapt.cfg.rsp_len = msg_adapt.cfg.rsp_buf_len;
        }
    }

    if ((msg_adapt.type == HCC_MSG_TYPE_CFG) && (msg_adapt.cfg.data_len != 0)) {
        msg_adapt.cfg.data = (osal_u8 *)(msg_adapt_comm + 1);
    }

    frw_dbg("\n func[%s] line[%d] msg_id[%d,%d] data[%p,%d] rsp[%p,%d,%d]\n",
        __func__, __LINE__, msg_adapt.msg_id, msg_adapt.type,
        msg_adapt.cfg.data, msg_adapt.cfg.data_len,
        msg_adapt.cfg.rsp, msg_adapt.cfg.rsp_buf_len, msg_adapt.cfg.rsp_len);

    if (g_frw_host_rcv_cb) {
        g_frw_host_rcv_cb(msg_adapt.msg_id, msg_adapt.vap_id,
            msg_adapt.msg_seq, msg_adapt.cb_ret, &msg_adapt.cfg);
    }
    frw_free_hcc_cfg_data(buf);
    return OAL_SUCC;
}

OAL_STATIC osal_void frw_msg_hook_stat_clear(osal_u16 msg_id)
{
#if defined(CONFIG_FRW_SUPPORT_CALLBACK_EXEC_STAT)
    osal_u16 real_id = host_frw_msg_convert(msg_id);
    g_frw_ctrl.msg_table[real_id].fail_cnt = 0;
    g_frw_ctrl.msg_table[real_id].succ_cnt = 0;
#else
    unref_param(msg_id);
#endif
}

osal_s32 frw_msg_exec_callback(osal_u16 msg_id, osal_u8 vap_id, frw_msg *msg)
{
    osal_s32 ret;
    osal_u16 real_id = host_frw_msg_convert(msg_id);
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    if (g_frw_ctrl.msg_table[real_id].msg_cb == OSAL_NULL) {
        wifi_printf("func[%s] line[%d] msg_id[%d] callback unregistered \r\n", __func__, __LINE__, msg_id);
        return OAL_FAIL;
    }

    hmac_vap = hmac_get_valid_vap(vap_id);
    if (hmac_vap == OSAL_NULL) {
        wifi_printf("func[%s] line[%d] msg_id[%d] hmac_vap is null \r\n", __func__, __LINE__, msg_id);
        return OAL_FAIL;
    }

    ret = g_frw_ctrl.msg_table[real_id].msg_cb(hmac_vap, msg);
#if defined(CONFIG_FRW_SUPPORT_CALLBACK_EXEC_STAT)
    if (ret == OAL_SUCC) {
        g_frw_ctrl.msg_table[real_id].succ_cnt++;
    } else {
        g_frw_ctrl.msg_table[real_id].fail_cnt++;
    }
#endif
    return ret;
}

osal_s32 frw_msg_hook_register_array(const msg_hook_array_stru msg_hook_array[], osal_u32 msg_hook_array_len)
{
    osal_u32 i;
    osal_s32 ret;

    for (i = 0; i < msg_hook_array_len; i++) {
        ret = frw_msg_hook_register(msg_hook_array[i].msg_id, msg_hook_array[i].msg_cb);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_TX, "msg_register_failed[%d]", msg_hook_array[i].msg_id);
        }
    }
    return OAL_SUCC;
}

osal_s32 frw_msg_hook_register(osal_u16 msg_id, frw_msg_callback msg_cb)
{
    osal_u16 real_id = host_frw_msg_convert(msg_id);
    if ((msg_cb == OSAL_NULL) || (msg_id >= g_frw_ctrl.msg_id_max)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    if (g_frw_ctrl.msg_table[real_id].msg_cb != OSAL_NULL) {
        return OAL_ERR_CODE_ALREADY_OPEN; // err_code_rep
    }

    g_frw_ctrl.msg_table[real_id].msg_cb = msg_cb;
    frw_msg_hook_stat_clear(msg_id);
    return OAL_SUCC;
}

osal_void frw_msg_hook_unregister(osal_u16 msg_id)
{
    osal_u16 real_id = host_frw_msg_convert(msg_id);
    if (msg_id >= g_frw_ctrl.msg_id_max) {
        return;
    }

    g_frw_ctrl.msg_table[real_id].msg_cb = OSAL_NULL;
    frw_msg_hook_stat_clear(msg_id);
    return;
}

/*****************************************************************************
 功能描述  : 清空某个事件队列中的所有事件
*****************************************************************************/
osal_void frw_event_flush_event_queue(osal_u16 msg_id)
{
    osal_s32 ret;
    frw_msg msg_info;
    frw_flush_msg flush_event = {0};

    flush_event.vap_flag = OSAL_FALSE;
    flush_event.drop = OSAL_FALSE;
    flush_event.msg_id = msg_id;

    frw_msg_init((osal_u8 *)&flush_event, sizeof(frw_flush_msg), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_FLUSH_EVENT_MSG, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "frw_event_flush_event_queue:send flush_msg to device fail[%d]", ret);
    }
}

osal_s32 frw_send_msg_to_device(osal_u8 vap_id, osal_u16 msg_id, frw_msg *msg, osal_bool sync)
{
    return frw_send_cfg_to_device(msg_id, vap_id, sync, TIME_OUT_MS, msg);
}

OAL_STATIC osal_s32 hmac_frw_check_param(osal_u16 msg_id, osal_u8 vap_id, const frw_msg *msg, osal_bool to_device)
{
    osal_u16 real_msg_id = host_frw_msg_convert(msg_id);
    if (msg == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (g_frw_ctrl.msg_table == OSAL_NULL) {
        return OAL_ERR_CODE_ALREADY_CLOSE;
    }

    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (!to_device) {
        if (msg_id >= g_frw_ctrl.msg_id_max) {
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        if (!g_frw_ctrl.msg_table[real_msg_id].msg_cb) {
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
        }
    }

    return OAL_SUCC;
}

osal_s32 frw_sync_host_post_msg(osal_u16 msg_id, osal_u8 vap_id, osal_u16 time_out, frw_msg *msg)
{
    osal_s32 ret = hmac_frw_check_param(msg_id, vap_id, msg, OSAL_FALSE);
    return (ret == OAL_SUCC) ? frw_host_post_msg_sync(msg_id, vap_id, time_out, msg) : ret;
}

osal_s32 frw_asyn_host_post_msg(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id, frw_msg *msg)
{
    osal_s32 ret = hmac_frw_check_param(msg_id, vap_id, msg, OSAL_FALSE);
    return (ret == OAL_SUCC) ? frw_host_post_msg_async(msg_id, pri, vap_id, msg) : ret;
}

osal_s32 frw_send_cfg_to_device(osal_u16 msg_id, osal_u8 vap_id, osal_bool sync, osal_u16 time_out, frw_msg *msg)
{
    osal_u16 real_id = host_frw_msg_convert(msg_id);
    osal_s32 ret = hmac_frw_check_param(msg_id, vap_id, msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "hmac_frw_check_param fail, ret %d, msg_id %d", ret, msg_id);
        return ret;
    }
#ifdef _PRE_WLAN_FEATURE_DFR
    /* 如果是在自愈流程且设置了禁用hcc消息,此处直接返回succ,避免超时后返回err自愈流程无法走完 */
    if (hmac_dfr_get_reset_process_flag() && frw_get_devmsg_inqueue_state() != OSAL_TRUE) {
        return OAL_SUCC;
    }
#endif
    /* host ko发消息给dmac ko */
    if (g_frw_ctrl.msg_table[real_id].msg_cb != OSAL_NULL) {
        return frw_msg_exec_callback(msg_id, vap_id, msg);
    }

    if (sync && (frw_msg_check_hcc_deadlock(msg_id) != OAL_SUCC)) {
        return OAL_ERR_CODE_THREAD_DEADLOCK;
    }

    return sync ? frw_send_cfg_to_device_sync(msg_id, vap_id, time_out, msg) :
        frw_send_msg_adapt(msg_id, msg, vap_id, 0, OSAL_FALSE);
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 frw_rx_wifi_post_action_check_param(oal_netbuf_stru *hcc_netbuf)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    struct frw_hcc_extend_hdr *extend_hdr = OAL_PTR_NULL;

    if (hcc_netbuf == NULL) {
        return OAL_FAIL;
    }
    extend_hdr = (struct frw_hcc_extend_hdr *)OAL_NETBUF_DATA(hcc_netbuf);
    hmac_vap = (hmac_vap_stru *)hmac_get_valid_vap(extend_hdr->vap_id);
    if (OAL_UNLIKELY(hmac_vap == NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac rx adapt ignored,pst vap is null, vap id:%u", extend_hdr->vap_id);
        oal_netbuf_free(hcc_netbuf);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

osal_u32 frw_rx_wifi_cfg_function(hcc_queue_type queue_id, osal_u8 stype,
    osal_u8 *buf, osal_u32 len, osal_u8 *user_param)
{
    oal_reference(queue_id);
    oal_reference(user_param);
    unref_param(len);

    if (buf == NULL) {
        return OAL_FAIL;
    }

    if (stype == CUSTOM_CFGID_MSG) {
        return frw_hmac_rcv_cfg(buf);
    } else {
        return OAL_FAIL;
    }
}

WIFI_HMAC_TCM_TEXT osal_u32 frw_alloc_pbuf(osal_u32 len, oal_netbuf_stru **ret_buf)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;

#ifdef _PRE_LWIP_ZERO_COPY
    netbuf = oal_pbuf_netbuf_alloc(len - PBUF_ZERO_COPY_RESERVE);
#else
    netbuf = oal_netbuf_alloc(len, 0, 4); /* 按4字节对齐 */
#endif
    if (netbuf == OSAL_NULL) {
        return OAL_FAIL;
    }

    oal_netbuf_put(netbuf, len);
    *ret_buf = netbuf;
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_u32 frw_alloc_netbuf(osal_u32 len, oal_netbuf_stru **ret_buf)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;

    netbuf = oal_netbuf_alloc(len, 0, 4); /* 按4字节对齐 */
    if (netbuf == OSAL_NULL) {
        return OAL_FAIL;
    }

    oal_netbuf_put(netbuf, len);
    *ret_buf = netbuf;
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_u32 frw_hmac_rcv_netbuf(oal_netbuf_stru *rx_netbuf)
{
    osal_u32 ret;
    oal_netbuf_pull(rx_netbuf, OAL_HCC_HDR_LEN);
    ret = frw_rx_wifi_post_action_check_param(rx_netbuf);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return frw_hmac_rcv_mgnt_data(rx_netbuf);
}

osal_u32 frw_rx_wifi_post_action_function(hcc_queue_type queue_id, osal_u8 stype,
    osal_u8 *buf, osal_u32 len, osal_u8 *user_param)
{
    unref_param(queue_id);
    unref_param(stype);
    unref_param(buf);
    unref_param(len);
    return frw_hmac_rcv_netbuf((oal_netbuf_stru *)user_param);
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 align_netbuf_head(oal_netbuf_stru *netbuf, osal_u32 hdr_len)
{
    osal_u32 headroom_add;
    osal_u32 netbuf_old_addr, netbuf_new_addr, addr_offset;
    osal_s32 ret;

    netbuf_old_addr = (osal_u32)(uintptr_t)OAL_NETBUF_DATA(netbuf) + hdr_len;
    netbuf_new_addr = OAL_ROUND_DOWN(netbuf_old_addr, 4); /* HCC需要4字节对齐 */
    addr_offset = netbuf_old_addr - netbuf_new_addr;
    /* 未对齐时在host侧做数据搬移，此处牺牲host，解放device */
    if (addr_offset != 0) {
        headroom_add = oal_netbuf_headroom(netbuf);
        if (addr_offset > headroom_add) {
            if (oal_netbuf_expand_head(netbuf, (td_s32)(addr_offset - headroom_add), 0, GFP_ATOMIC) != OAL_SUCC) {
                oam_error_log0(0, OAM_SF_ANY, "{align_netbuf_head aglin:: alloc head room failed.}");
                return OAL_ERR_CODE_ALLOC_MEM_FAIL;
            }
        }

        ret = memmove_s((osal_u8 *)OAL_NETBUF_DATA(netbuf) - addr_offset, OAL_NETBUF_LEN(netbuf),
            (osal_u8 *)OAL_NETBUF_DATA(netbuf), OAL_NETBUF_LEN(netbuf));
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{align_netbuf_head aglin:: memmove_s failed.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        oal_netbuf_push(netbuf, addr_offset);
        oal_netbuf_trim(netbuf, addr_offset);
    }
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_u32 frw_hcc_tx_adjust_header(oal_netbuf_stru *netbuf, osal_u8 data_type)
{
    mac_tx_ctl_stru *tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuf);
    osal_u32 cb_length, headroom_add, headroom_add_cb_mac, headroom_add_frw_hcc;
    osal_u32 ret;
    osal_s32 sret;
    unref_param(data_type);

    if ((mac_get_cb_frame_header_length(tx_ctrl) == 0) ||
        (mac_get_cb_frame_header_length(tx_ctrl) > OAL_MAX_MAC_HDR_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_hcc_tx_adjust_header MAC HEADER ERR:%u}",
            mac_get_cb_frame_header_length(tx_ctrl));
        return OAL_FAIL;
    }

    cb_length = OAL_MAX_CB_LEN;
    headroom_add_cb_mac = cb_length + OAL_MAX_MAC_HDR_LEN - mac_get_cb_frame_header_length(tx_ctrl);
    headroom_add_frw_hcc = OAL_HCC_HDR_LEN + frw_get_head_len(); /* 长度和为4的倍数 */
    headroom_add = headroom_add_cb_mac + headroom_add_frw_hcc;
    if (headroom_add > oal_netbuf_headroom(netbuf)) {
        sret = oal_netbuf_expand_head(netbuf, (osal_s32)headroom_add - (osal_s32)oal_netbuf_headroom(netbuf),
                                      0, GFP_ATOMIC);
        if (sret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_hcc_tx_adjust_header:: alloc head room fail:%u}", sret);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
    }
    /* push data 指针去对齐 */
    oal_netbuf_push(netbuf, headroom_add_cb_mac + headroom_add_frw_hcc);
    ret = align_netbuf_head(netbuf, 0);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_hcc_tx_adjust_header:: alloc head room fail:%u}", ret);
        return ret;
    }

    /* 设置netbuf data指针到802.11header */
    oal_netbuf_pull(netbuf,
        headroom_add_frw_hcc + cb_length + OAL_MAX_MAC_HDR_LEN - mac_get_cb_frame_header_length(tx_ctrl));

    return OAL_SUCC;
}
