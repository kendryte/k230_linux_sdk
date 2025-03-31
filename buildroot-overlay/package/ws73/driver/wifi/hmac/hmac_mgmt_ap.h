/*
 * Copyright (c) CompanyNameMagicTag. 2013-2024. All rights reserved.
 * 文 件 名   : hmac_mgmt_ap.h
 * 生成日期   : 2013年6月18日
 * 功能描述   : hmac_mgmt_ap.c 的头文件
 */


#ifndef __HMAC_MGMT_AP_H__
#define __HMAC_MGMT_AP_H__

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
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_AP_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/


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
    osal_u16 mgmt_len;
    osal_u16 netbuf_len;
    osal_u16 frame_len;
    osal_u8 is_p2p_req;
    osal_u8 resv; /* 保留  */
} hmac_ap_up_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
osal_void hmac_set_print_wakeup_mgmt_etc(osal_u8 val);
osal_u8 hmac_get_print_wakeup_mgmt_etc(osal_void);
#endif
extern osal_u32 hmac_ap_up_rx_mgmt_etc(hmac_vap_stru *hmac_vap, osal_void *p_param);
extern osal_u32 hmac_ap_up_misc_etc(hmac_vap_stru *hmac_vap, osal_void *p_param);
extern osal_u32 hmac_ap_wait_start_rx_mgmt(hmac_vap_stru *hmac_vap, osal_void *p_param);
extern osal_u32 hmac_ap_wait_start_misc_etc(hmac_vap_stru *hmac_vap, osal_void *p_param);
extern osal_u32 hmac_mgmt_timeout_ap_etc(osal_void *p_param);
extern osal_void hmac_handle_disconnect_rsp_ap_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern oal_bool_enum hmac_go_is_auth(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_ap_clean_bss_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_ap_save_user_assoc_req(hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u32 payload_len, osal_u8 mgmt_frm_type);
extern osal_void hmac_handle_connect_rsp_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_void hmac_mgmt_update_auth_mib(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_rsp);
#ifdef _PRE_WLAN_FEATURE_WPA3
hmac_sae_pmksa_entry_info *hmac_sae_alloc_pmksa_entry(osal_void);
hmac_sae_pmksa_entry_info *hmac_sae_pmksa_find_oldest_entry(hmac_sae_pmksa_cache_stru *pmksa_mgmt);
osal_void hmac_sae_add_pmksa_to_list(hmac_vap_stru *hmac_vap, hmac_sae_pmksa_entry_info *pmksa_entry);
osal_u32 hmac_sae_clean_expire_pmksa(osal_void *arg);
osal_void hmac_sae_clean_all_pmksa(hmac_vap_stru *hmac_vap);
hmac_sae_pmksa_entry_info *hmac_sae_find_pmkid_by_macaddr(hmac_sae_pmksa_cache_stru *pmksa_mgmt,
    osal_u8 *mac_addr);
#endif

osal_void hmac_ap_up_rx_mgmt_with_vendor_ie(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u8 *go_on);
osal_u32  hmac_ap_up_rx_probe_req(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const osal_u8 *addr, osal_char rssi, osal_u8 *go_on);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mgmt_ap.h */
