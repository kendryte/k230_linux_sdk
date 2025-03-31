/*
 * Copyright (c) CompanyNameMagicTag 2013-2021. All rights reserved.
 * 文 件 名   : hmac_mgmt_bss_comm.h
 * 生成日期   : 2013年4月10日
 * 功能描述   : hmac_mgmt_bss_comm.c 的头文件
 */

#ifndef __HMAC_MGMT_BSS_COMM_H__
#define __HMAC_MGMT_BSS_COMM_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_BSS_COMM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* HMAC_NCW_INHIBIT_THRED_TIME时间内连续HMAC_RECEIVE_NCW_MAX_CNT次接收到ncw,不上报 */
#define HMAC_NCW_INHIBIT_THRED_TIME   60000    /* 单位ms */
#define HMAC_RECEIVE_NCW_THRED_CNT    6

#define HMAC_FTM_SEND_BUF_LEN    200
#define HMAC_CSI_SEND_BUF_LEN    3000

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern const osal_u8 g_auc_avail_protocol_mode_etc[WLAN_PROTOCOL_BUTT][WLAN_PROTOCOL_BUTT];

/*****************************************************************************
  5 消息头定义
*****************************************************************************/
typedef osal_u32  (*encap_csa_action)(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer,
    osal_u8 *data, osal_u16 *pus_len);

/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    hmac_user_stru *hmac_user;
    hmac_ba_rx_hdl_stru *ba_rx_hdl;
} hmac_mgmt_tx_addba;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32  hmac_mgmt_rx_addba_req_etc(
    hmac_vap_stru              *hmac_vap,
    hmac_user_stru             *hmac_user,
    osal_u8                  *payload);
extern osal_u32  hmac_mgmt_rx_addba_rsp_etc(
    hmac_vap_stru              *hmac_vap,
    hmac_user_stru             *hmac_user,
    osal_u8                  *payload);
extern osal_u32  hmac_mgmt_rx_delba_etc(
    hmac_vap_stru              *hmac_vap,
    hmac_user_stru             *hmac_user,
    osal_u8                  *payload);
extern osal_u32  hmac_mgmt_tx_addba_req_etc(
    hmac_vap_stru              *hmac_vap,
    hmac_user_stru             *hmac_user,
    mac_action_mgmt_args_stru  *action_args);
extern osal_u32  hmac_mgmt_tx_addba_rsp_etc(
    hmac_vap_stru              *hmac_vap,
    hmac_user_stru             *hmac_user,
    hmac_ba_rx_stru            *ba_rx_info,
    osal_u8                   tid,
    osal_u8                   status);
extern osal_u32  hmac_mgmt_tx_delba_etc(
    hmac_vap_stru              *hmac_vap,
    hmac_user_stru             *hmac_user,
    mac_action_mgmt_args_stru  *action_args);
extern osal_u32  hmac_mgmt_tx_addba_timeout_etc(osal_void *p_arg);

#ifdef _PRE_WLAN_FEATURE_PMF
extern osal_u32  hmac_sa_query_interval_timeout_etc(osal_void *p_arg);
extern osal_void    hmac_send_sa_query_rsp_etc(hmac_vap_stru *hmac_vap, osal_u8 *hdr,
    oal_bool_enum_uint8 is_protected);
extern osal_u32  hmac_start_sa_query_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_bool_enum_uint8 is_protected);
extern osal_u32  hmac_pmf_check_err_code_etc(const hmac_user_stru *user_base_info,
    oal_bool_enum_uint8 is_protected, const osal_u8 *mac_hdr);

#endif
extern osal_u32 hmac_tx_mgmt_send_event_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *mgmt_frame,
    osal_u16 frame_len);
extern osal_void hmac_mgmt_update_assoc_user_qos_table_etc(
    osal_u8                      *payload,
    osal_u16                      msg_len,
    hmac_user_stru                 *hmac_user);
extern  osal_u32  hmac_check_bss_cap_info_etc(osal_u16 us_cap_info, hmac_vap_stru *hmac_vap);
extern  osal_void hmac_set_user_protocol_mode_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_u32  hmac_mgmt_reset_psm_etc(hmac_vap_stru *hmac_vap, osal_u16 user_id);

#ifdef _PRE_WLAN_FEATURE_PMF
extern osal_void  hmac_rx_sa_query_req_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 is_protected);
extern osal_void  hmac_rx_sa_query_rsp_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 is_protected);
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
osal_void hmac_mgmt_rx_mu_edca_control_etc(
    hmac_vap_stru              *hmac_vap,
    osal_u8                      *payload_len);
#endif
extern osal_void    hmac_send_mgmt_to_host_etc(hmac_vap_stru  *hmac_vap,
    oal_netbuf_stru *buf,
    osal_u16       len,
    osal_slong          l_freq);

#if defined(_PRE_WLAN_FEATURE_HS20) || defined(_PRE_WLAN_FEATURE_P2P) || defined(_PRE_WLAN_FEATURE_WPA3)
extern osal_void hmac_rx_mgmt_send_to_host_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf);
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
extern osal_u32  hmac_interworking_check(hmac_vap_stru *hmac_vap,  osal_u8 *param);
#endif

extern osal_s32  hmac_mgmt_tx_event_status_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_void hmac_user_init_rates_etc(hmac_user_stru *hmac_user);
extern osal_void  hmac_rx_ba_session_decr_etc(hmac_vap_stru *hmac_vap, osal_u8 tidno);
extern osal_void  hmac_tx_ba_session_decr_etc(hmac_vap_stru *hmac_vap, osal_u8 tidno);
extern osal_void  hmac_vap_set_user_avail_rates_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_u32 hmac_proc_vht_cap_ie_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *vht_cap_ie);
extern osal_u32 hmac_ie_proc_assoc_user_legacy_rate(osal_u8 *payload, osal_u32 rx_len,
    hmac_user_stru *hmac_user);

#ifdef _PRE_WLAN_FEATURE_11AX
extern osal_u32 hmac_proc_he_cap_ie(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *he_cap_ie);
extern osal_u32  hmac_proc_he_bss_color_change_announcement_ie(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u8 *bss_color_ie);

#endif

osal_u32  hmac_mgmt_delba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 tid, osal_u8 initiator, osal_u8 reason);
osal_u32  hmac_mgmt_rx_addba_rsp(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const hmac_ctx_action_event_stru *crx_action_event);
osal_u32  hmac_mgmt_rx_delba(const hmac_vap_stru *hmac_vap, const hmac_ctx_action_event_stru *crx_action_event);
osal_u32  hmac_mgmt_tx_addba_req_send(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *event, oal_netbuf_stru *net_buff);
osal_u32  hmac_mgmt_tx_addba_rsp_send(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *net_buff);
osal_u32  hmac_mgmt_tx_delba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *event, oal_netbuf_stru *net_buff);
osal_u32  hmac_mgmt_scan_vap_down(const hmac_vap_stru *hmac_vap);
osal_void hmac_set_cap_info_field(hmac_vap_stru *hmac_vap, osal_u8 *buffer);
osal_u8   hmac_get_dsss_ie_channel_num(const hmac_vap_stru *hmac_vap, osal_u8 scan_flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mgmt_bss_comm.h */
