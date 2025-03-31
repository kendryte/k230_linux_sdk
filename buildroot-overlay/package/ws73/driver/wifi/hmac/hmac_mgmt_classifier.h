/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_mgmt_classifier.h
 * 生成日期   : 2012年11月20日
 * 功能描述   : hmac_mgmt_classifier.c 的头文件
 */

#ifndef HMAC_MGMT_CLASSIFIER_H
#define HMAC_MGMT_CLASSIFIER_H

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
#define THIS_FILE_ID OAM_FILE_ID_HMAC_MGMT_CLASSIFIER_H
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
    hmac_device_stru *device;
    hmac_vap_stru *hmac_vap;
    mac_tx_ctl_stru *tx_ctl;
} hmac_tx_action_params;

typedef struct {
    mac_ieee80211_frame_stru *frame_hdr;
    hmac_vap_stru *hmac_vap;
    oal_netbuf_stru *netbuf;
} hmac_rx_mgmt_deal_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
extern osal_u32  hmac_mgmt_tx_action_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_action_mgmt_args_stru *action_args);
extern osal_s32 hmac_mgmt_rx_delba_event_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_void hmac_mgmt_send_disasoc_msg(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code);
extern osal_s32 hmac_mgmt_send_disasoc_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_void hmac_mgmt_send_deauth_event(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code);
osal_void hmac_send_disasoc_misc_msg(hmac_vap_stru *hmac_vap, osal_u16 user_idx, osal_u16 disasoc_reason);
osal_s32 hmac_handle_disasoc_misc_msg(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_send_disassoc_misc_event(hmac_vap_stru *hmac_vap, osal_u16 user_idx, osal_u16 disasoc_reason);

osal_s32 hmac_rx_process_report_mgmt_ctrl(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
osal_s32 hmac_rx_process_ba_sync_event(hmac_vap_stru *hmac_vap, hmac_ctx_action_event_stru *crx_action_sync);
osal_s32 hmac_join_set_dtim_reg_event_process(hmac_vap_stru *hmac_vap);
osal_s32 hmac_join_set_reg_event_process(hmac_vap_stru *hmac_vap, hmac_ctx_join_req_set_reg_stru *reg_params);
osal_s32 hmac_set_multi_bssid(hmac_vap_stru *hmac_vap, mac_scanned_all_bss_info *all_bss_info);
osal_s32 hmac_rx_process_mgmt_ctrl(oal_netbuf_stru *netbuf);

osal_void hmac_mgmt_connect_set_channel(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device,
    hmac_vap_stru *up_vap, mac_channel_stru *channel);

osal_s32 hmac_rx_cut_process_mgmt_ctrl_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_process_del_sta(osal_u8 vap_id);
osal_u32 hmac_tx_process_mgmt_event(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
osal_s32 hmac_rx_mgmt_event_adapt(hmac_vap_stru *hmac_vap, frw_msg *msg);
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
osal_void hmac_rx_send_report_frame_msg(hmac_vap_stru *hmac_vap, const mac_rx_ctl_stru *rx_info,
    osal_s8 rssi, osal_u16 msg_id);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_mgmt_classifier.h */
