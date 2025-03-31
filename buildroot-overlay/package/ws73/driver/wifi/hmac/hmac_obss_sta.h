/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_beacon_rom.c 的头文件
 * Date: 2020-07-07
 */

#ifndef HMAC_STA_OBSS_H
#define HMAC_STA_OBSS_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "mac_resource_ext.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
typedef struct {
    osal_u32             chan_report_for_te_a;  /* Channel Bit Map to register TE-A */
    oal_bool_enum_uint8  te_b;                  /* 20/40M intolerant for TE-B */
} hmac_obss_te_a_b_stru;
#endif

/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
osal_u8 hmac_mgmt_need_obss_scan(hmac_vap_stru *hmac_vap);
osal_void hmac_update_join_req_params_2040_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr);
osal_u32 hmac_ie_proc_obss_scan_ie_etc(hmac_vap_stru *hmac_vap, const osal_u8 *payload);
osal_void hmac_mgmt_obss_scan_notify(hmac_vap_stru *hmac_vap);
osal_void hmac_scan_start_obss_timer(hmac_vap_stru *hmac_vap);
osal_void hmac_scan_destroy_obss_timer(hmac_vap_stru *hmac_vap);
osal_void hmac_send_2040_coext_mgmt_frame_sta(hmac_vap_stru *hmac_vap);
osal_void hmac_sta_up_rx_2040_coext(hmac_vap_stru *hmac_vap, const osal_u8 *frame_body);
osal_s32 hmac_config_get_2040bss_sw(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_scan_proc_obss_scan_complete_event(hmac_vap_stru *hmac_vap);
osal_void hmac_rx_notify_channel_width(hmac_vap_stru *hmac_vap, const osal_u8 *data, hmac_user_stru *hmac_user);
osal_void hmac_detect_2040_te_a_b(hmac_vap_stru *hmac_vap, osal_u8 *frame_body, osal_u16 frame_len,
    osal_u16 offset, osal_u8 curr_chan);
#endif
static osal_u32 hmac_sta_obss_init_weakref(osal_void) __attribute__ ((weakref("hmac_sta_obss_init"), used));
static osal_void hmac_sta_obss_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_sta_obss_deinit"), used));
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_sta_obss.h */
