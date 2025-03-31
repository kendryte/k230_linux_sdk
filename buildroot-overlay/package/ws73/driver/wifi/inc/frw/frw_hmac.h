/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Header file of dmac public interface.
 * Author: Huanghe
 * Create: 2021-07-01
 */

#ifndef __FRW_HMAC_H__
#define __FRW_HMAC_H__

#include "osal_types.h"
#include "oal_netbuf_ext.h"
#include "mac_vap_ext.h"
#include "frw_msg_rom.h"
#include "frw_common.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define TIME_OUT_MS 2000

typedef struct {
    osal_u32 data;  /* 配置消息的输入数据buffer指针，内存由调用者分配, 由frw释放 */
    osal_u32 rsp;     /* 配置消息的同步调用，返回数据的buffer指针，返回数据buffer由接口分配，但需要调用者释放；
                         同步调用时，接口通过此参数向调用者回传rsp地址，异步调用时，接口会将此参数置为NULL。
                      */
    osal_u16 data_len; /* 配置消息的输入数据buffer长度 */
    osal_u16 rsp_buf_len; /* hmac下发到dmac时填写；同步调用，返回的数据长度，异步调用时，此参数会被接口置为0 */
    osal_u16 rsp_len;   /* 实际rsp长度，由dmac返回 */
    osal_u16 sync : 1;   /* 同步标志 */
    osal_u16 type : 7;   /* 消息类型 */
    osal_u16 rsv  : 8;   /* 保留字段 */
} frw_msg_host;

typedef struct {
    osal_u16    msg_id;
    osal_u8     vap_id;
    osal_u8     sync : 1;
    osal_u8     type : 3;
    osal_u8     rsv : 4;
    osal_s32    cb_ret;
    osal_u32    msg_seq;
    frw_msg_host     cfg;
} frw_msg_adapt_comm;

typedef osal_s32 (*frw_msg_callback)(hmac_vap_stru *hmac_vap, frw_msg *msg);

typedef struct {
    osal_u16 msg_id;
    frw_msg_callback msg_cb;
} msg_hook_array_stru;

/*
     约定：对于hmac侧而言：
           send： wal/hmac -> dmac
           post： wal/hmac -> wal/hmac
           rcv ： dmac -> wal/hmac
*/

/*  msg_id：配置消息ID;
    sync: 同步消息标志，true，表示同步消息，此时pri参数被忽略；false，表示异步消息，此时pri参数指定消息优先级
    pri: 消息/配置的优先级;
    用于从wal/hmac发起的异步消息/配置，但消息/配置不会被发往device侧
*/
/* 用于wal->hmac的同步配置 */

osal_s32 frw_msg_hook_register_array(const msg_hook_array_stru msg_hook_array[], osal_u32 msg_hook_array_len);
osal_s32 frw_msg_hook_register(osal_u16 msg_id, frw_msg_callback msg_cb);
osal_void frw_msg_hook_unregister(osal_u16 msg_id);
osal_s32 frw_netbuf_hook_register(osal_u16 netbuf_type, frw_msg_callback msg_cb);
osal_void frw_netbuf_hook_unregister(osal_u16 netbuf_type);

osal_s32 frw_netbuf_exec_callback(osal_u16 netbuf_type, osal_u8 vap_id, frw_msg *msg);
osal_s32 frw_msg_host_init(osal_u16 d2d_msgid_begin, osal_u16 w2d_msgid_end, osal_u16 msg_id_max);

osal_s32 frw_sync_host_post_msg(osal_u16 msg_id, osal_u8 vap_id, osal_u16 time_out, frw_msg *msg);
osal_s32 frw_asyn_host_post_msg(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id, frw_msg *msg);
osal_s32 frw_send_cfg_to_device(osal_u16 msg_id, osal_u8 vap_id, osal_bool sync, osal_u16 time_out, frw_msg *msg);

typedef osal_s32 (*frw_host_rcv_callback)(osal_u16 msg_id, osal_u8 vap_id, osal_u32 seq,
    osal_s32 dev_cb_ret, frw_msg *msg);
osal_void frw_host_register_rcv_hook(frw_host_rcv_callback rcv_cb);

typedef osal_s32 (*frw_d2h_netbuf_callback)(oal_netbuf_stru *netbuf, osal_u32 payload_len, osal_u8 vap_id,
    osal_u16 netbuf_type, osal_u8 data_type);
osal_void frw_event_flush_event_queue(osal_u16 msg_id);
osal_s32 frw_msg_exec_callback(osal_u16 msg_id, osal_u8 vap_id, frw_msg *msg);

osal_u32 frw_rx_wifi_cfg_function(hcc_queue_type queue_id, osal_u8 stype,
    osal_u8 *buf, osal_u32 len, osal_u8 *user_param);
osal_u32 frw_rx_wifi_post_action_function(hcc_queue_type queue_id, osal_u8 stype,
    osal_u8 *buf, osal_u32 len, osal_u8 *user_param);

#ifdef _PRE_WLAN_FEATURE_CENTRALIZE
extern osal_s32 is_msgid_in_device_range(osal_u16 msg_id);
extern osal_s32 dmac_frw_post_async_msg(osal_u16 msg_id, osal_u8 pri, osal_u8 vap_id, frw_msg *msg, osal_u32 seq);
osal_u32 frw_dmac_rcv_mgnt_data(oal_dmac_netbuf_stru *netbuf);
oal_dmac_netbuf_stru *oal_mem_netbuf_alloc(osal_u8 netbuf_id, osal_u16 len, oal_netbuf_priority_enum_uint8 flag);
osal_u8 *oal_dmac_netbuf_payload(const oal_dmac_netbuf_stru *netbuf);
osal_void oal_mem_netbuf_free(oal_dmac_netbuf_stru *netbuf);
#endif
osal_u32 frw_hcc_tx_adjust_header(oal_netbuf_stru *netbuf, osal_u8 data_type);
osal_u32 frw_hmac_rcv_netbuf(oal_netbuf_stru *rx_netbuf);
osal_u32 frw_alloc_netbuf(osal_u32 len, oal_netbuf_stru **ret_buf);
osal_u32 frw_alloc_pbuf(osal_u32 len, oal_netbuf_stru **ret_buf);
osal_s32 frw_msg_host_exit(osal_void);
osal_u8 frw_get_devmsg_inqueue_state(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // __FRW_HMAC_H__
