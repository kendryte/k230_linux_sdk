/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac m2s head
 * Create: 2020-7-5
 */

#ifndef __HMAC_M2S_H__
#define __HMAC_M2S_H__

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "frw_ext_if.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "hmac_device.h"
#include "hmac_user.h"
#include "hmac_vap.h"
#include "hmac_feature_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
#define M2S_MIMO_RX_CNT_THRESHOLD 2
#define M2S_SISO_RX_CNT_THRESHOLD 8
#define M2S_RX_UCAST_CNT_THRESHOLD 10

#define M2S_ACTION_SENT_CNT_THRESHOLD 3

#define M2S_RX_STATISTICS_START_TIME 1000

/*****************************************************************************
  STRUCT定义
*****************************************************************************/
typedef struct {
    frw_timeout_stru m2s_delay_switch_statistics_timer;
    osal_u16 rx_nss_mimo_count;
    osal_u16 rx_nss_siso_count;
    osal_u16 rx_nss_ucast_count;
    oal_bool_enum_uint8 rx_nss_statistics_start_flag;
} hmac_vap_m2s_rx_statistics_stru;

typedef struct {
    hmac_vap_m2s_rx_statistics_stru hmac_vap_m2s_rx_statistics; /* STA当前切换初始模式不满足要求时，开启统计变量 */
    oal_bool_enum_uint8 m2s_wait_complete; /* m2s切换是否正在执行，多vap都完成才能执行device状态切换 */
    wlan_m2s_action_type_enum_uint8 m2s_switch_ori_type; /* STA当前关联ap的切换初始模式 */
    oal_bool_enum_uint8 action_send_state;               /* STA action帧发送是否成功 */
    osal_u8 action_send_cnt;
} hmac_vap_m2s_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 hmac_m2s_proc_vap_init(hmac_vap_stru *hmac_vap);
osal_void hmac_m2s_proc_vap_deinit(hmac_vap_stru *hmac_vap);

#ifdef _PRE_WIFI_DEBUG
osal_void hmac_m2s_mgr_param_info(const hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_show_blacklist_in_list(const hal_to_dmac_device_stru *hal_device);
#endif
osal_void hmac_m2s_update_switch_mgr_param(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
wlan_nss_enum_uint8 hmac_m2s_get_bss_max_nss(const hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf,
    osal_u16 frame_len, oal_bool_enum_uint8 assoc_status);
osal_u8 hmac_m2s_scan_get_num_sounding_dim(const oal_netbuf_stru *netbuf, osal_u16 frame_len);
oal_bool_enum_uint8 hmac_m2s_get_bss_support_opmode(const hmac_vap_stru *hmac_vap, osal_u8 *frame_body,
    osal_u16 frame_len);
osal_u32 hmac_m2s_d2h_vap_info_syn(hmac_vap_stru *hmac_vap);
osal_void hmac_m2s_send_action_complete_check(hmac_vap_stru *hmac_vap, const mac_tx_ctl_stru *tx_ctl);
osal_void hmac_m2s_mgr_back_to_mimo_check(hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_update_vap_capbility(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap);
osal_void hmac_m2s_update_user_capbility(hmac_user_stru *hmac_user, const hmac_vap_stru *hmac_vap);
osal_void hmac_m2s_switch_update_vap_capbility(const hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_switch_update_device_capbility(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device);
osal_void hmac_m2s_switch_same_channel_vaps_begin(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    hmac_vap_stru *mac_vap1, hmac_vap_stru *mac_vap2);
#ifdef _PRE_WLAN_FEATURE_DBAC
osal_void hmac_m2s_switch_dbac_vaps_begin(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    hmac_vap_stru *mac_vap1, hmac_vap_stru *mac_vap2);
#endif
osal_void hmac_m2s_switch_update_hal_chain_capbility(hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 set_channel);
oal_bool_enum_uint8 hmac_m2s_custom_switch_check(const hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_command_mode_force_to_mimo(hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_spec_mode_force_to_mimo(hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_spec_mode_band_change_check(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 set_band);
osal_void hmac_m2s_switch_update_vap_tx_chain_capbility(const hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_spec_update_hal_device_capbility(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band);
oal_bool_enum_uint8 hmac_m2s_spec_need_switch_keep_check(const hal_to_dmac_device_stru *hal_device);
oal_bool_enum_uint8 hmac_m2s_spec_support_siso_switch_check(const hal_to_dmac_device_stru *hal_device,
    const hmac_device_stru *hmac_device, wlan_m2s_trigger_mode_enum_uint8 trigger_mode);
osal_void hmac_m2s_spec_update_vap_sw_capbility(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    wlan_nss_enum_uint8 nss_num);
osal_void hmac_config_m2s_assoc_state_syn(const hmac_vap_stru *hmac_vap);
osal_void hmac_config_m2s_disassoc_state_syn(hmac_vap_stru *hmac_vap);
oal_bool_enum_uint8 hmac_m2s_assoc_vap_find_in_device_blacklist(const hal_to_dmac_device_stru *hal_device);
osal_u32 hmac_m2s_check_blacklist_in_list(const hmac_vap_stru *hmac_vap, osal_u8 *index);
osal_void hmac_m2s_action_frame_type_query(const hmac_vap_stru *hmac_vap, wlan_m2s_action_type_enum_uint8 *action_type);
osal_void hmac_m2s_switch_protect_trigger(hmac_vap_stru *hmac_vap);
osal_void hmac_m2s_send_action_frame(hmac_vap_stru *hmac_vap);
osal_void hmac_m2s_rx_rate_nss_process(hmac_vap_stru *hmac_vap, const dmac_rx_ctl_stru *cb_ctrl,
    const mac_ieee80211_frame_stru *frame_hdr);
oal_bool_enum_uint8 hmac_m2s_switch_apply_and_confirm(hal_to_dmac_device_stru *hal_device, osal_u16 m2s_type,
    wlan_m2s_trigger_mode_enum_uint8 trigger_mode);
osal_void hmac_m2s_switch_vap_off(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    hmac_vap_stru *hmac_vap);
osal_u32 hmac_m2s_switch_device_begin(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_switch_device_end(hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_nss_and_bw_alg_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_bool_enum_uint8 nss_change, oal_bool_enum_uint8 bw_change);
osal_u32 hmac_m2s_switch(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event,
    oal_bool_enum_uint8 hw_switch);
osal_void hmac_m2s_mimo_to_miso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
osal_void hmac_m2s_mimo_to_siso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
osal_void hmac_m2s_siso_to_mimo(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
osal_void hmac_m2s_siso_to_miso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
osal_void hmac_m2s_miso_to_mimo(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
osal_void hmac_m2s_miso_to_siso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
osal_void hmac_m2s_siso_to_siso(hal_to_dmac_device_stru *hal_device, hal_m2s_event_tpye_uint16 m2s_event);
osal_u32 hmac_m2s_handle_event(hal_to_dmac_device_stru *hal_device, osal_u16 type, osal_u16 datalen, osal_u8 *data);
osal_void hmac_m2s_fsm_attach(hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_fsm_detach(hal_to_dmac_device_stru *hal_device);
osal_void hmac_m2s_mgmt_switch_judge(hal_to_dmac_device_stru *hal_device, const hmac_vap_stru *hmac_vap,
    osal_char ant0_rssi_in, osal_char ant1_rssi_in);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of _PRE_WLAN_FEATURE_M2S */

#endif /* end of hmac_m2s.h */
