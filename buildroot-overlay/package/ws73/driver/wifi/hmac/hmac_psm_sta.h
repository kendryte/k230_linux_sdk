/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_psm_sta.c 的头文件。
 * Create: 2020-07-03
 */

#ifndef HMAC_PSM_STA_H
#define HMAC_PSM_STA_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "dmac_ext_if_hcm.h"
#include "hmac_twt.h"
#include "msg_psm_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PSM_STA_ROM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_PSM_MAX_SKIP_BCN_CNT       3
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
/* 单通道接收beacon的RSSI门限 */
#define HMAC_PSM_SINGLE_BCN_RX_CHAIN_TH  (-73)
#endif

/* 在Multiple BSSID-Index element中包含DTIM count/period字段的时候，其element len长度为3 */
#define MULTI_DTIM_PERIOD_COUNT_LEN 3
#define EXT_CAPA_GET_MULTI_BSSID_LEN 2
#define MULTI_BSSID_ELEMENT_MIN_LEN 5

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_psm_max_ps_en_sync(const hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
osal_void hmac_psm_update_bcn_rf_chain(const hmac_vap_stru *hmac_vap, osal_char rssi);
#endif
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
osal_u32 hmac_psm_mask_mac_parse_tim(hmac_vap_stru *hmac_vap, osal_u8 mac_parse_tim_switch);
osal_s32 hmac_bcn_no_frm_event_hander(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_u32 hmac_set_psm_param_to_device(const hmac_vap_stru *hmac_vap, osal_u8 is_dtim3_on, osal_u8 dtim3_on,
    osal_u8 is_keep_alive, osal_u8 is_all_vap);
osal_s32 hmac_config_sta_psm_status_sync(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_set_psm_param(hmac_vap_stru *hmac_vap, mac_cfg_ps_param_stru *ps_param);
osal_s32 hmac_config_set_sta_pm_param_cb(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_psm_sta.h */
