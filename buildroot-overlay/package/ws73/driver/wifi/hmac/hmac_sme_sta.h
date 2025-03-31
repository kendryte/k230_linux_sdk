/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_sme_sta.h
 * 生成日期   :  2013年6月28日
 * 功能描述   : hmac_sme_sta.c 的头文件
 */


#ifndef __HMAC_SME_STA_H__
#define __HMAC_SME_STA_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_mgmt_sta.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SME_STA_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
typedef osal_void(*hmac_sme_handle_rsp_func)(hmac_vap_stru *hmac_vap, osal_u8 *msg);

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 上报给SME结果 类型定义 */
typedef enum {
    HMAC_SME_SCAN_RSP,
    HMAC_SME_JOIN_RSP,
    HMAC_SME_AUTH_RSP,
    HMAC_SME_ASOC_RSP,
    HMAC_SME_RSP_BUTT
} hmac_sme_rsp_enum;
typedef osal_u8 hmac_sme_rsp_enum_uint8;

typedef enum {
    HMAC_AP_SME_START_RSP = 0,
    HMAC_AP_SME_RSP_BUTT
} hmac_ap_sme_rsp_enum;
typedef osal_u8 hmac_ap_sme_rsp_enum_uint8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
#define     MAX_AUTH_CNT        5
#define     MAX_ASOC_CNT        5

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_void hmac_cfg80211_scan_comp(hmac_scan_record_stru  *p_scan_record);
extern osal_void hmac_send_rsp_to_sme_sta_etc(hmac_vap_stru *hmac_vap, hmac_sme_rsp_enum_uint8 type,
    osal_u8 *msg);
extern osal_void  hmac_send_rsp_to_sme_ap_etc(hmac_vap_stru *hmac_vap, hmac_ap_sme_rsp_enum_uint8 type,
    osal_u8 *msg);
osal_void hmac_handle_scan_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg);
osal_void hmac_handle_join_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg);
osal_void hmac_handle_auth_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg);
osal_void hmac_handle_asoc_rsp_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *msg);
osal_void hmac_prepare_join_req_etc(hmac_join_req_stru *join_req, mac_bss_dscr_stru *bss_dscr);
osal_u32 hmac_sta_update_join_req_params_etc(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req);
osal_void hmac_report_assoc_state_sta(const hmac_vap_stru *hmac_vap, const osal_u8 *mac_addr, osal_u8 assoc);
extern osal_void hmac_set_scan_param(hmac_vap_stru *hmac_vap, osal_u8 channel, mac_scan_req_stru *scan_param);
void hmac_handle_assoc_rsp_succ_sta(const hmac_vap_stru *hmac_vap, hmac_asoc_rsp_stru *asoc_rsp);
osal_void hmac_prepare_auth_req(hmac_vap_stru *hmac_vap, hmac_auth_req_stru *auth_req);
osal_void hmac_report_connect_failed_result_etc(hmac_vap_stru *hmac_vap,
    mac_status_code_enum_uint16 reason_code);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_sme_sta.h */
