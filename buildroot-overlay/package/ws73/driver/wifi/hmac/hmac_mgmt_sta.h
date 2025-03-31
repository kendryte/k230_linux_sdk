/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_mgmt_sta.h
 * 生成日期   : 2013年6月18日
 * 功能描述   : hmac_mgmt_sta.c 的头文件
 */

#ifndef __HMAC_MGMT_STA_H__
#define __HMAC_MGMT_STA_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_mgmt_bss_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_STA_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_WMM_QOS_PARAMS_HDR_LEN        8
#define HMAC_WMM_QOSINFO_AND_RESV_LEN      2
#define HMAC_WMM_AC_PARAMS_RECORD_LEN      4
#define DATARATES_80211G_NUM 12
#define HMAC_WMM_VO_DEFAULT_DECA_AIFSN 2

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/* 加入请求参数 */
typedef struct {
    mac_bss_dscr_stru   bss_dscr;            /* 要加入的bss网络 */
    osal_u16          join_timeout;        /* 加入超时 */
    osal_u16          probe_delay;
} hmac_join_req_stru;

/* 认证请求参数 */
typedef struct {
    osal_u16                  timeout;
    osal_u8                   auc_resv[2];
} hmac_auth_req_stru;

/* 关联请求参数 */
typedef struct {
    osal_u16                  assoc_timeout;
    osal_u8                   auc_resv[2];
} hmac_asoc_req_stru;

/* 加入结果 */
typedef struct {
    hmac_mgmt_status_enum_uint8 result_code;
    osal_u8                   auc_resv[3];
} hmac_join_rsp_stru;

/* 认证结果 */
typedef struct {
    osal_u8                   peer_sta_addr[6];   /* mesh下peer station的地址 */
    osal_u16                  status_code;         /* 认证结果 */
} hmac_auth_rsp_stru;

/* 去关联原因 */
typedef struct {
    hmac_mgmt_status_enum_uint8  disasoc_reason_code;
    osal_u8                    auc_resv[3];
} hmac_disasoc_rsp_stru;

typedef struct {
    hmac_mgmt_status_enum_uint8  result_code;
    osal_u8                    auc_resv[3];
} hmac_ap_start_rsp_stru;

typedef struct {
    osal_u8   expand_rate; /* 扩展速率集 */
    osal_u8   mac_rate; /* MAC对应速率 */
    osal_u8   phy_rate; /* PHY对应速率 */
    osal_u8   protocol;     /* 协议 */
} hmac_data_rate_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 hmac_sta_up_update_ht_params(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 frame_len,
    hmac_user_stru *hmac_user);
osal_u32 hmac_sta_up_update_vht_params(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 frame_len,
    hmac_user_stru *hmac_user);
osal_void hmac_chan_multi_select_channel_mac(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 bandwidth);

osal_u32 hmac_sta_up_update_he_oper_params(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 frame_len,
    hmac_user_stru *hmac_user);
#ifdef _PRE_WLAN_FEATURE_11AX
osal_void hmac_sta_update_affected_acs_machw_direct(osal_u8 *payload, hmac_vap_stru *hmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_OM
osal_s32 hmac_omi_rx_he_rom_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_rx_he_trig_event(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_u32 hmac_sta_wait_join_etc(hmac_vap_stru *hmac_vap, osal_void *msg);
extern osal_u32 hmac_sta_wait_auth_etc(hmac_vap_stru *hmac_vap, osal_void *msg);
extern osal_u32 hmac_sta_wait_asoc_etc(hmac_vap_stru *hmac_vap, osal_void *msg);
extern osal_u32 hmac_sta_wait_asoc_rx_etc(hmac_vap_stru *hmac_vap, osal_void *msg);
extern osal_u32 hmac_sta_wait_auth_seq2_rx_etc(hmac_vap_stru *hmac_vap, osal_void *msg);
extern osal_u32 hmac_sta_wait_auth_seq4_rx_etc(hmac_vap_stru *hmac_vap, osal_void *p_msg);
extern osal_u32 hmac_sta_auth_timeout_etc(hmac_vap_stru *hmac_vap, osal_void *p_param);
extern osal_u32 hmac_sta_up_rx_mgmt_etc(hmac_vap_stru *hmac_vap_sta, osal_void *p_param);
extern osal_u32 hmac_sta_wait_asoc_timeout_etc(hmac_vap_stru *hmac_vap, osal_void *p_param);
extern osal_void hmac_sta_handle_disassoc_rsp_etc(hmac_vap_stru *hmac_vap, osal_u16 disasoc_reason_code);

#if defined(_PRE_WLAN_FEATURE_HS20) || defined(_PRE_WLAN_FEATURE_P2P)
extern osal_u32 hmac_sta_not_up_rx_mgmt_etc(hmac_vap_stru *hmac_vap_sta, osal_void *p_param);
#endif
osal_u32 hmac_scan_get_user_protocol_etc(mac_bss_dscr_stru *bss_dscr, wlan_protocol_enum_uint8 *protocol_mode);
extern osal_u32 hmac_sta_get_user_protocol_etc(mac_bss_dscr_stru *bss_dscr,
    wlan_protocol_enum_uint8  *pen_protocol_mode);
osal_void hmac_sta_update_wmm_info(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *wmm_ie);
extern osal_u32 hmac_sta_up_update_edca_params_machw_etc(hmac_vap_stru  *hmac_vap,
    mac_wmm_set_param_type_enum_uint8 type);
extern osal_void hmac_sta_up_update_edca_params_etc(
    osal_u8               *payload,
    osal_u16               msg_len,
    hmac_vap_stru           *hmac_vap,
    osal_u8                frame_sub_type,
    hmac_user_stru          *hmac_user);
extern oal_bool_enum_uint8  hmac_is_support_11grate_etc(osal_u8 *rates, osal_u8 rate_num);
extern oal_bool_enum_uint8  hmac_is_support_11brate_etc(osal_u8 *rates, osal_u8 rate_num);
extern osal_u32 hmac_process_assoc_rsp_etc(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user,
    osal_u8 *mac_hdr,
    osal_u16 hdr_len,
    osal_u8 *payload,
    osal_u16 msg_len);
extern osal_u32 hmac_sta_sync_vap(hmac_vap_stru *hmac_vap, mac_channel_stru *channel,
    wlan_protocol_enum_uint8 protocol);
extern osal_u8 *hmac_sta_find_ie_in_probe_rsp_etc(hmac_vap_stru *hmac_vap, osal_u8 eid, osal_u16 *pus_index);
extern osal_u32 hmac_sta_get_min_rate(hmac_set_rate_stru *rate_params, hmac_join_req_stru *join_req);

#ifdef _PRE_WLAN_FEATURE_11AX
osal_u32 hmac_sta_up_update_mu_edca_params_machw(hmac_vap_stru *hmac_vap,
    mac_wmm_set_param_type_enum_uint8 type, osal_u8 qos_info_count);
#endif
osal_u32 hmac_sta_wait_auth(hmac_vap_stru *hmac_vap, osal_void *msg);
osal_u32 hmac_report_ext_auth_event(hmac_vap_stru *hmac_vap);
osal_void hmac_tx_delete_ba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_void hmac_sta_print_last_reason_code(osal_void);
osal_void hmac_sta_set_last_reason_code(osal_u16 reason_code);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mgmt_sta.h */
