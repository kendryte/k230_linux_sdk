/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: hmac_config.c 的头文件.
 */

#ifndef __HMAC_CONFIG_H__
#define __HMAC_CONFIG_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_device.h"
#ifdef _PRE_WLAN_DFR_STAT
#include "hmac_dfr.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_MAX_MCS_NUM                    8   /* 单流和双流支持的mac最大个数 */
#define QUICK_SEND_RESULT_INVALID   255

#ifdef _PRE_WLAN_DFT_STAT
#define WAL_CFG_TIME_OUT_MS        5000
#endif
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
/* hmac_vap结构中，一部分成员的大小，这些成员在linux和windows下的定义可能不同 */
typedef struct {
    osal_u32      hmac_vap_cfg_priv_stru_size;
    osal_u32      frw_timeout_stru_size;
    osal_u32      oal_spin_lock_stru_size;
    osal_u32      mac_key_mgmt_stru_size;
    osal_u32      mac_pmkid_cache_stru_size;
    osal_u32      mac_curr_rateset_stru_size;
    osal_u32      hmac_vap_stru_size;
} hmac_vap_member_size_stru;

typedef struct {
    uint8_t  ie_type;
    uint8_t  ie_id;
} cfg_ie_info_stru;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_cfg_init(osal_void);
extern osal_u32  hmac_config_sta_update_rates_etc(hmac_vap_stru *hmac_vap,
    mac_cfg_mode_param_stru *cfg_mode, mac_bss_dscr_stru *bss_dscr);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_s32 hmac_config_set_all_log_level_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

extern osal_s32 hmac_config_remove_app_ie(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef _PRE_WLAN_FEATURE_WPA3
osal_s32 hmac_config_external_auth(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_update_owe_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef _PRE_WLAN_FEATURE_11D
osal_s32  hmac_config_set_rd_by_ie_switch_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

osal_s32 hmac_config_set_mode_cb(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32 hmac_config_set_chan_meas(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s8 hmac_config_get_free_power(const hmac_vap_stru *hmac_vap);
osal_s32 hmac_cfg_vap_init_event(hmac_device_stru *hmac_device);
osal_void hmac_config_set_quick_result(osal_u8 result);
osal_u8 hmac_config_get_quick_result(osal_void);

osal_void h2d_cfg_init(osal_void);
osal_s32 hmac_config_vap_state_syn(hmac_vap_stru *hmac_vap, mac_vap_state_enum_uint8 new_vap_state);

osal_void hmac_config_scan_abort(hmac_vap_stru *hmac_vap);
osal_s32 hmac_config_reduce_sar(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_get_dieid(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_mode(hmac_vap_stru *hmac_vap, const mac_cfg_mode_param_stru *old_param);

osal_s32 hmac_config_set_freq(hmac_vap_stru *hmac_vap, osal_u8 value);
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
osal_s32 hmac_config_stop_sched_scan(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_40m_intol_update_ap(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_channel(hmac_vap_stru *hmac_vap, mac_cfg_channel_param_stru *channel_param);
osal_s32 hmac_config_connect(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_WIRELESS_EXT
osal_s32 hmac_config_start_iwlist_scan(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
osal_s32 hmac_config_set_dyn_cali_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_config_get_fem_pa_status(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_get_g_gul_dual_antenna_enable(osal_void);

osal_s32 hmac_cali_to_hmac(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_ONLINE_DPD
osal_s32 hmac_dpd_to_hmac(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

osal_u32 hmac_fill_sys_rsp(frw_msg *msg, osal_u16 len, const osal_u8 *param);
osal_u32 hmac_config_vap_mib_update(hmac_vap_stru *hmac_vap);
osal_u32 hmac_config_vap_cap_update(hmac_vap_stru *hmac_vap);
#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_rssi_limit(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_config_beacon_chain_switch(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_log_level(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_config_rx_he_rom_en_handle(const hmac_vap_stru *hmac_vap);
#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_vap_info(hmac_vap_stru *hmac_vap);
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_s32 hmac_config_query_status(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_config_send_custom_pkt(hmac_vap_stru *hmac_vap, frw_msg *msg);
#if defined(_PRE_WLAN_FEATURE_ALWAYS_TX) || defined(_PRE_WLAN_CFGID_DEBUG)
osal_s32 hmac_config_reg_write(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
osal_s32 hmac_config_query_rssi(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_query_psst(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_reg_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
osal_s32 hmac_config_wifi_sniffer_switch(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef _PRE_WLAN_DFT_STAT
osal_s32  hmac_config_query_ani(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_dump_phy(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_dump_mac(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_dump_stats(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

osal_s32 hmac_config_ch_status_sync_ext(hmac_vap_stru *hmac_vap, mac_ap_ch_info_stru *ch_info, osal_u16 info_len);
osal_s32 hmac_config_set_txpower(hmac_vap_stru *hmac_vap, osal_u8 value);
#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_wfa_cfg_aifsn(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_wfa_cfg_cw(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

osal_s32 hmac_config_suspend_action_sync(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_s32 hmac_config_adjust_tx_power(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_rate_power_offset(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_cal_tone(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_s32 hmac_config_restore_tx_power(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_rf_limit_power(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_u32 hmac_reset_hw(hal_to_dmac_device_stru *hal_device, const hmac_reset_para_stru *reset_para);

osal_s32 hmac_config_cali_dump_info(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_s32 hmac_config_set_dbm(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_get_tsf(hmac_vap_stru *hmac_vap, frw_msg *msg);

osal_s32 hmac_config_custom_cfg_priv_ini(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_tx_complete_data(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_keepalive_sync_timestamp(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_beacon_period(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_tx_comp_schedule(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_hal_device_sync(hal_to_dmac_device_stru *hal_device);
osal_s32 hmac_device_sync(hmac_device_stru *hmac_device);
#ifdef _PRE_WLAN_FEATURE_M2S
osal_s32 hmac_m2s_switch_protect_comp_event_status(hmac_vap_stru *hmac_vap,
    hmac_m2s_complete_syn_stru *m2s_comp_param);
#endif
#ifdef _PRE_WIFI_DEBUG
osal_s32 hmac_config_set_tx_data_type_debug(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u8 hal_tx_data_type_debug_on(osal_void);
#endif
osal_s32 hmac_config_mac_ant_select(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_phy_ant_select(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_phy_ant_select_enable(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 wifi_get_init_start_time(osal_void);
osal_void wifi_set_init_start_time(osal_void);
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
osal_u8 hmac_is_mgmt_report_enable(osal_void);
#endif
osal_void hmac_cfg_ccpriv_init(osal_void);
osal_void hmac_cfg_ccpriv_exit(osal_void);
osal_u8 hmac_config_sta_rcv_probe_req_is_on(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_main */
