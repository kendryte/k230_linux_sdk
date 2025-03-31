/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal ccpriv common api.
 * Create: 2022-03-02
 */

#ifndef __WAL_CCPRIV_COMMON_H__
#define __WAL_CCPRIV_COMMON_H__

#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "oal_ext_if.h"
#include "hmac_ext_if.h"
#include "wal_main.h"
#include "mac_device_ext.h"
#include "mac_vap_ext.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "hmac_csa_ap.h"
#include "msg_alg_rom.h"
#include "wal_common.h"
#include "hmac_11k.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_u32 wal_get_user_by_mac(oal_net_device_stru *net_dev, osal_s8 *param, osal_u16 *user_idx);
osal_u32 uapi_ccpriv_set_one_arg_s32(oal_net_device_stru *net_dev, osal_s8 *param, const osal_char *cmd);
osal_u32 uapi_ccpriv_csi_set_config(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_csi_get_config(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_csi_set_buffer(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_csi_switch(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_send_random_mac_oui(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 wal_ccpriv_check_scan_params(mac_cfg_set_scan_param *scan_param);
osal_u32 uapi_ccpriv_service_control_set(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_service_control_get(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_kick_user(oal_net_device_stru *netdev, osal_s8 *param);
osal_u32 uapi_ccpriv_get_tx_params(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 wal_get_tx_params(hmac_user_stru *hmac_user, osal_u32 *tx_best_rate);

osal_u32 wal_ccpriv_get_mac_addr_etc(osal_s8 *param, osal_u8 mac_addr[], osal_u32 *total_offset);

osal_u32 uapi_ccpriv_set_rate_power(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_set_cal_tone(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_get_nvram_power(oal_net_device_stru *net_dev, osal_s8 *param);
#ifdef _PRE_WLAN_FEATURE_FTM
osal_u32 uapi_ccpriv_ftm(oal_net_device_stru *net_dev, osal_s8 *param);
#endif
osal_u32 wal_parse_csa_cmd(osal_s8 *param, mac_protocol_debug_switch_stru *info);
osal_u32 uapi_ccpriv_multi_bssid_enable(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_set_opmode_notify(oal_net_device_stru *net_dev, osal_s8 *param);
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
osal_u32 uapi_ccpriv_host_sleep_switch(oal_net_device_stru *netdev, osal_s8 *pc_param);
osal_u32 uapi_ccpriv_set_wow(oal_net_device_stru *netdev, osal_s8 *pc_param);
osal_u32 uapi_ccpriv_set_wow_pattern(oal_net_device_stru *netdev, osal_s8 *pc_param);
#endif
osal_u32 uapi_ccpriv_m2u_snoop_enable(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_m2u_snoop_list(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_m2u_snoop_deny_table(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_m2u_snoop_send_igmp(oal_net_device_stru *net_dev, osal_s8 *param);
#ifdef _PRE_WLAN_CFGID_DEBUG
osal_u32 uapi_ccpriv_set_scan_param(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_set_ampdu_amsdu(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_set_keepalive_mode(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_common_debug(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_set_ap_user_aging_time(oal_net_device_stru *net_dev, osal_s8 *param);
#ifdef _PRE_WLAN_SMOOTH_PHASE
osal_u32 uapi_ccpriv_set_smooth_phase_en(oal_net_device_stru *net_dev, osal_s8 *param);
#endif
#endif
osal_u32 wal_handel_radio_meas_rpt_cmd(mac_cfg_radio_meas_info_stru *radio_meas_cfg, osal_s8 *param);
osal_u32  wal_post2hmac_uint32_data(oal_net_device_stru *net_dev, osal_s8 *param, osal_u16 msg_id);
osal_u32 uapi_ccpriv_dhcp_debug(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_set_2040_coext_support(oal_net_device_stru *net_dev, osal_s8 *param);
oal_bool_enum_uint8 check_mac_str(osal_s8 *value);
#ifdef _PRE_WLAN_DFT_STAT
osal_u32 uapi_ccpriv_get_user_conn_record(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_get_vap_sniffer_result(oal_net_device_stru *net_dev, osal_s8 *param);
#endif
osal_u32 wal_ccpriv_parse_ipv4(osal_char *ip_str, osal_u32 *result);
osal_void wal_mem_read(osal_u8 reg32, uintptr_t start_addr, osal_u32 read_number);
osal_u32 uapi_ccpriv_rssi_limit(oal_net_device_stru *net_dev, osal_s8 *param);
#ifdef _PRE_WLAN_FEATURE_11AX
osal_u32  uapi_ccpriv_set_sts_sung_mung(oal_net_device_stru *net_dev, osal_s8 *param);
#endif
osal_u8 wal_ccpriv_check_cfg_vap(oal_net_device_stru *net_dev);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
