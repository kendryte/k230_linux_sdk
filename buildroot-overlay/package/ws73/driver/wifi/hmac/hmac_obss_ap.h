/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_beacon_rom.c 的头文件
 * Date: 2020-07-07
 */

#ifndef HMAC_AP_OBSS_H
#define HMAC_AP_OBSS_H

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
typedef enum {
    HMAC_OP_ALLOWED  = BIT0,
    HMAC_SCA_ALLOWED = BIT1,
    HMAC_SCB_ALLOWED = BIT2,
} hmac_chan_op_enum;
typedef osal_u8 hmac_chan_op_enum_uint8;

typedef enum {
    HMAC_NETWORK_SCA = 0,
    HMAC_NETWORK_SCB = 1,

    HMAC_NETWORK_BUTT,
} hmac_network_type_enum;
typedef osal_u8 hmac_network_type_enum_uint8;
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u16                 aus_num_networks[HMAC_NETWORK_BUTT];
    hmac_chan_op_enum_uint8    chan_op;
    osal_u8                  auc_resv[3];
} hmac_eval_scan_report_stru;

typedef struct {
    osal_u8 user_chan_idx;
    osal_u8 user_chan_offset;
    osal_u8 chan_offset;
    osal_u8 least_busy_chan_idx;
    osal_u8 num_supp_chan;
    osal_u8 resv[3];  /* 保留  */
} hmac_chan_select_stru;

typedef struct {
    osal_u8 sec_chan_idx;
    osal_u8 pri_chan_idx;
    mac_sec_ch_off_enum_uint8 sec_chan_offset;
    osal_u8 affected_chan_lo;
    osal_u8 affected_chan_hi;
    hmac_network_type_enum_uint8 network_type;
    hmac_chan_op_enum_uint8 allowed_bit;
    osal_u8 resv;
} hmac_chan_mgmt_40mhz_cal;
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
osal_void hmac_chan_start_40m_recovery_timer(hmac_vap_stru *hmac_vap);
osal_void hmac_chan_stop_40m_recovery_timer(hmac_vap_stru *hmac_vap);
osal_void hmac_chan_create_40M_recovery_timer(hmac_vap_stru *hmac_vap);
oal_bool_enum_uint8 hmac_chan_get_2040_op_chan_list(const hmac_vap_stru *hmac_vap,
    hmac_eval_scan_report_stru *chan_scan_report);
osal_u32 hmac_chan_prepare_for_40m_recovery(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth);
oal_bool_enum_uint8  hmac_chan_is_40mhz_sca_allowed(
    const hmac_vap_stru                 *hmac_vap,
    const hmac_eval_scan_report_stru   *chan_scan_report,
    osal_u8                     pri_chan_idx,
    mac_sec_ch_off_enum_uint8     user_chan_offset);
oal_bool_enum_uint8 hmac_chan_is_40mhz_scb_allowed(const hmac_vap_stru *hmac_vap,
    const hmac_eval_scan_report_stru *chan_scan_report, osal_u8 pri_chan_idx, mac_sec_ch_off_enum_uint8 chan_offset);
osal_u32 hmac_chan_select_channel_for_operation(const hmac_vap_stru *hmac_vap, osal_u8 *new_channel,
    wlan_channel_bandwidth_enum_uint8 *new_bandwidth);
osal_void hmac_chan_init_chan_scan_report(const hmac_vap_stru *hmac_vap,
    hmac_eval_scan_report_stru *chan_scan_report, osal_u8 num_supp_chan);
osal_u16  hmac_chan_get_cumulative_networks(
    const hmac_device_stru                     *hmac_device,
    wlan_channel_bandwidth_enum_uint8    band,
    osal_u8                            pri_chan_idx);
osal_void hmac_ap_handle_40_intol(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 payload_len);
osal_void hmac_ap_process_obss_bandwidth(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 payload_len);
osal_void  hmac_ap_up_rx_2040_coext(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf);
osal_void  hmac_chan_multi_switch_to_20mhz_ap(hmac_vap_stru *hmac_vap);
osal_u32 hmac_send_notify_chan_width(hmac_vap_stru *hmac_vap, const osal_u8 *data);
osal_void hmac_chan_update_40m_intol_user_etc(hmac_vap_stru *hmac_vap);
osal_void hmac_40m_intol_sync_data(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bandwidth_40m,
    oal_bool_enum_uint8 intol_user_40m);
osal_void hmac_ap_rx_notify_channel_width(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const osal_u8 *data);
#endif
static osal_u32 hmac_ap_obss_init_weakref(osal_void) __attribute__ ((weakref("hmac_ap_obss_init"), used));
static osal_void hmac_ap_obss_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_ap_obss_deinit"), used));
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_sta_obss.h */