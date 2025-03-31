/*
 * Copyright (c) CompanyNameMagicTag. 2023-2024. All rights reserved.
  文 件 名   : hmac_11r.h
  作    者   :
  生成日期   : 2023年3月2日
  功能描述   : hmac_11r.c的头文件
 */

#ifndef HMAC_11R_H
#define HMAC_11R_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_roam_main.h"
#include "oal_net_cfg80211.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11R_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define FT_NETBUF_CB_LENTH  48
#define WAL_WIFI_FEATURE_SUPPORT_11R 2
#define MAC_MAX_FTE_LEN 257
#define FT_CAPABILITY_OFFSET 4

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
typedef struct {
    osal_u8                    bssid[WLAN_MAC_ADDR_LEN];
    osal_u16                   ft_ie_len;
    osal_u8                    ft_ie_buff[0];
} hmac_roam_ft_stru;

typedef struct {
    osal_u16 mdid; /* Mobile Domain ID */
    osal_u16 len;  /* FTE 的长度 */
    osal_u8 ie[MAC_MAX_FTE_LEN];
} mac_cfg80211_ft_ies_stru;

typedef struct {
    oal_cfg80211_bss_stru *bss;
    const osal_u8 *ie;
    osal_u8 ie_len;
    oal_nl80211_auth_type_enum_uint8 auth_type;
    const osal_u8 *key;
    osal_u8 key_len;
    osal_u8 key_idx;
} mac_cfg80211_auth_req_stru;

typedef struct {
    osal_u8 enable_11r;
    osal_u8 enable_11r_over_ds;
    osal_u8 resv[2];
} hmac_11r_vap_info_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 inline函数定义
*****************************************************************************/

/*****************************************************************************
  11 函数声明
*****************************************************************************/
static osal_u32 hmac_11r_init_weakref(osal_void) __attribute__ ((weakref("hmac_11r_init"), used));
static osal_void hmac_11r_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_11r_deinit"), used));

typedef osal_void (*hmac_11r_set_md_ie_cb)(hmac_vap_stru *hmac_vap, osal_u8 **buffer, osal_u8 *ie_len);
typedef osal_void (*hmac_11r_set_authentication_mode_cb)(hmac_vap_stru *hmac_vap,
    const mac_conn_security_stru *conn_sec);
typedef osal_void (*hmac_11r_mib_init_cfg_cb)(hmac_vap_stru *hmac_vap, const mac_conn_security_stru *conn_sec,
    mac_bss_dscr_stru *bss_dscr);
typedef osal_void (*hmac_11r_init_ft_cfg_cb)(hmac_vap_stru *hmac_vap, mac_conn_security_stru *conn_sec);
typedef osal_void (*hmac_11r_encap_auth_req_ft_ie_cb)(hmac_vap_stru *hmac_vap, osal_u8 *mgmt_frame,
    osal_u16 *auth_req_len);
typedef osal_u32 (*hmac_roam_auth_seq2_11r_process_cb)(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info,
    mac_rx_ctl_stru *rx_ctrl, osal_u8 *mac_hdr);
typedef osal_u32 (*hmac_11r_process_assoc_rsp_cb)(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info);
typedef osal_u32 (*hmac_11r_roam_connect_start_etc_cb)(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info,
    mac_bss_dscr_stru *bss_dscr);
typedef osal_void (*hmac_11r_roam_connect_to_bss_cb)(hmac_vap_stru *hmac_vap, const hmac_roam_info_stru *roam_info,
    oal_bool_enum_uint8 *need_to_stop_user);
typedef osal_void (*hmac_11r_roam_change_app_ie_cb)(hmac_vap_stru *hmac_vap, app_ie_type_uint8 *app_ie_type);
typedef osal_void (*hmac_11r_roam_connect_fsm_init_cb)(
    hmac_roam_fsm_func hmac_roam_connect_fsm_func[][ROAM_CONNECT_FSM_EVENT_TYPE_BUTT]);
typedef osal_void (*hmac_get_11r_cap_cb)(hmac_vap_stru *hmac_vap, osal_s32 *pl_value);
typedef osal_void (*hmac_set_11r_md_ft_over_ds_cb)(hmac_vap_stru *hmac_vap, osal_u8 *ies, osal_u32 ie_len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_11r.h */
