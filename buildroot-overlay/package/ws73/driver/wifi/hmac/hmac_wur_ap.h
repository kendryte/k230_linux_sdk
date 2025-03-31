/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_wur_ap.h
 * Date: 2020-07-07
 */

#ifndef __HMAC_WUR_AP_H__
#define __HMAC_WUR_AP_H__

#include "oal_ext_if.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX

/* 2 宏定义 */
#define HMAC_WUR_BITS_OFFSET 32
#define WUR_LDR 0
#define WUR_HDR 1
#define WUR_SHORT 2
#define WUR_TU 1024
#define OOS_DATA_LEN 1500

/* 3 枚举定义 */
typedef enum {
    WUR_TYPE_INVAILD = 0,
    WUR_TYPE_BEACON_LDR = 1,
    WUR_TYPE_BEACON_HDR = 2,
    WUR_TYPE_FL_WAKE_LDR = 3,
    WUR_TYPE_FL_WAKE_HDR = 4,
    WUR_TYPE_SHORT_WAKE_LDR = 5,
    WUR_TYPE_SHORT_WAKE_HDR = 6,
} hmac_wur_frame_type_enum;

typedef enum {
    WUR_UNICAST_WAKEUP = 0,
    WUR_MULTICAST_WAKEUP = 1,
    WUR_BSS_PARAM_WAKEUP = 2,
} hmac_wur_wakeup_type_enum;
typedef osal_u8 hmac_wur_wakeup_type_enum_uint8;

typedef enum {
    WUR_DESC_TYPE_BEACON = 0,
    WUR_DESC_TYPE_FL_WAKE = 1,
    WUR_DESC_TYPE_SHORT_WAKE = 2,
} tx_desc_wur_frame_type_enum;

/* 新增枚举类型, 在更新WUR传事件时，用于区分来源 */
typedef enum {
    MAC_WUR_UPDATE_SOURCE_ASSOC = 0,
    MAC_WUR_UPDATE_SOURCE_SETUP = 1,
    MAC_WUR_UPDATE_SOURCE_TEARDOWN = 2,
    MAC_WUR_UPDATE_SOURCE_MAX
} mac_wur_update_source_enum;
typedef osal_u8 mac_wur_update_source_enum_uint8;

/* WUR action帧类型 */
typedef enum {
    MAC_WUR_ACTION_MODE_SETUP = 0,
    MAC_WUR_ACTION_MODE_TEARDOWN = 1,
    MAC_WUR_ACTION_WAKEUP_INDICATION = 2,
} mac_wur_action_enum;
typedef osal_u8 mac_wur_action_enum_uint8;

/* WUR action type字段类型 */
typedef enum {
    MAC_WUR_ENTER_WUR_MODE_REQ = 0,
    MAC_WUR_ENTER_WUR_MODE_RSP = 1,
    MAC_WUR_ENTER_WUR_MODE_SUSPEND_REQ = 2,
    MAC_WUR_ENTER_WUR_MODE_SUSPEND_RSP = 3,
    MAC_WUR_ENTER_WUR_MODE_SUSPEND = 4,
    MAC_WUR_ENTER_WUR_MODE = 5,
} mac_wur_action_type_enum;
typedef osal_u8 mac_wur_action_type_enum_uint8;

/* WUR Mode response status字段类型 */
typedef enum {
    MAC_WUR_MODE_ACCEPT = 0,
    MAC_WUR_MODE_DENIED_UNSPECIFIED = 1,
    MAC_WUR_MODE_DENIED_PERIOD = 2,
    MAC_WUR_MODE_DENIED_KEEPALIVE = 3,
    MAC_WUR_MODE_RESERVED = 255,
} mac_wur_response_status_enum;
typedef osal_u8 mac_wur_response_status_enum_uint8;

/* WUR更新事件参数 */
typedef struct {
    mac_wur_update_source_enum_uint8 update_source;
    osal_u8 resv;
    osal_u16 user_idx;
    mac_wur_param_htd_stru wur_param_htd;
    mac_user_wur_cap_ie_stru wur_cap_ie;
} hmac_ctx_update_wur_stru;

typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    osal_u8 resv[2];                     /* 2 byte保留字段 */
    mac_wur_mode_setup_frame_stru wur_mode_setup;
} mac_cfg_wur_mode_setup_param_stru;

typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    osal_u8 resv[2];                     /* 2 byte保留字段 */
} mac_cfg_wur_mode_teardown_param_stru;

typedef struct {
    osal_u8 twbtt_bcn_en : 1, /* TWBTT中断硬件发WUR BEACON开关 */
        wur_period_en : 1,    /* WUR周期中断开关 */
        wur_frame_type : 4,   /* WUR唤醒帧类型 0无效 */
        wur_enable : 1,       /* WUR使能开关 */
        rsvd : 1;
    osal_u8 resv[3];                          /* 3 byte保留字段 */
    mac_wur_basic_param_stru wur_basic_param; /* hmac初始化同步到dmac */
} mac_cfg_wur_enable_option_param_stru;

/* 10 函数声明 */
osal_void hmac_wur_set_param_en(osal_u8 wur_twbtt_bcn_en, osal_u8 wur_period_en,
    osal_u8 wur_frame_type);
osal_u8 hmac_wur_get_user_status(const hmac_user_stru *hmac_user);
osal_u8 hmac_wur_is_ps_enqueue(const hmac_user_stru *hmac_user);
osal_void hmac_wur_init_user_info(hmac_user_stru *hmac_user);
osal_void hmac_wur_user_del_handle(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);

osal_s32 hmac_wur_update(hmac_vap_stru *hmac_vap, hmac_ctx_update_wur_stru *crx_action_sync);
osal_u32 dmac_mgmt_tx_wur_mode_setup(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf);
osal_u32 dmac_mgmt_tx_wur_mode_teardown(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, oal_netbuf_stru *netbuf);
osal_u32 hmac_wur_mode_teardown_success(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_void hmac_wur_action_tx_complete_process(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    osal_u8 dscr_status);
osal_u32 hmac_wur_is_all_support_hdr(const hmac_vap_stru *hmac_vap);
osal_void hmac_wur_write_wake_up_frame(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 is_multi, osal_u8 wur_bss_update_cnt, osal_u8 wur_frame_type);
osal_void hmac_wur_unicast_wake_process(hmac_vap_stru *hmac_vap);
osal_void hmac_wur_multi_wake_process(hmac_vap_stru *hmac_vap);
osal_s32 hmac_wur_sp_start_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_wur_sp_end_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_wur_ps_enqueue_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_tx_ctl_stru *tx_ctrl);
osal_u8 hmac_wur_is_multi_flush(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_void hmac_wur_bss_update_process(hmac_vap_stru *hmac_vap);
osal_void hmac_wur_ht_oper_ie_save(hmac_vap_stru *hmac_vap, const mac_ht_opern_stru *ht_opern_tmp);
osal_void hmac_wur_vht_oper_ie_save(hmac_vap_stru *hmac_vap, const mac_vht_opern_stru *vht_opern_tmp);
osal_void hmac_wur_he_oper_ie_save(hmac_vap_stru *hmac_vap, const mac_frame_he_operation_param_stru *he_opern_param,
    const mac_frame_he_bss_color_info_stru *he_bss_color, const mac_frame_he_mcs_nss_bit_map_stru *mcs_bit_map);
osal_u32 hmac_wur_psm_rx_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *net_buf);
osal_u32 hmac_wur_mode_setup_update(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 action_type);
osal_u32 hmac_wur_fill_start_time(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, const oal_netbuf_stru *netbuf);
osal_u32 hmac_wur_setup_succ_update(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_mode_ie_stru *wur_mode_element, const mac_wur_param_ap_field_stru *wur_param_ap_field);
osal_u32 hmac_wur_encap_wake_up_frame(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 is_multi, osal_u8 wur_bss_update_cnt, osal_u8 *wur_frame_type);
osal_u32 hmac_wur_sp_start_process(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_wur_sp_end_process(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_wur_frame_tx_complete_process(osal_u8 dscr_status, mac_tx_ctl_stru *cb,
    hal_to_dmac_device_stru *hal_device);
osal_s32 hmac_config_wur_send_setup(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_wur_send_teardown(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_wur_enable_option(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_u32 hmac_ap_wur_update_event(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_update_source_enum_uint8 wur_update_source);
osal_u32 hmac_mgmt_tx_wur_mode_setup(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_param_htd_stru *wur_param_htd);
osal_u32 hmac_mgmt_tx_wur_mode_teardown(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u32 hmac_ap_rx_wur_mode_setup_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload);
osal_u32 hmac_ap_rx_wur_mode_teardown_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u32 hmac_proc_wur_cap_ie(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *wur_cap_ie);

/* 回调函数声明 */
typedef osal_u32 (*hmac_wur_psm_rx_process_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *net_buf);
typedef osal_u32 (*hmac_wur_mode_setup_update_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 action_type);
typedef osal_u32 (*hmac_wur_fill_start_time_cb)(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const hmac_ctx_action_event_stru *ctx_action_event, const oal_netbuf_stru *netbuf);
typedef osal_u32 (*hmac_wur_setup_succ_update_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_wur_mode_ie_stru *wur_mode_element, const mac_wur_param_ap_field_stru *wur_param_ap_field);
typedef osal_u32 (*hmac_wur_mode_teardown_success_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
typedef osal_u32 (*hmac_wur_encap_wake_up_frame_cb)(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 is_multi, osal_u8 wur_bss_update_cnt, osal_u8 *wur_frame_type);
typedef osal_u8 (*hmac_wur_is_multi_flush_cb)(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
typedef osal_s32 (*hmac_wur_sp_start_process_cb)(hmac_vap_stru *hmac_vap, frw_msg *msg);
typedef osal_s32 (*hmac_wur_sp_end_process_cb)(hmac_vap_stru *hmac_vap, frw_msg *msg);
typedef osal_u32 (*hmac_wur_frame_tx_complete_process_cb)(osal_u8 dscr_status, mac_tx_ctl_stru *cb,
    hal_to_dmac_device_stru *hal_device);

osal_void hmac_ap_up_rx_action_wur(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *data);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_wur_ap.h */