/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: wal common msg api.
 * Author: Huanghe
 * Create: 2021-08-16
 */

#ifndef __WAL_COMMON_H__
#define __WAL_COMMON_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"
#include "hmac_ext_if.h"
#include "wal_ext_if.h"
#include "wal_utils.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_COMMON_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WAL_ATCMDSRB_GET_RX_PCKT (5 * 1000)

#define WAL_MSG_TIME_OUT_MS 4000

/* 协议模式与字符串映射 */
typedef struct {
    osal_char                           *pc_name;        /* 模式名字符串 */
    wlan_protocol_enum_uint8            mode;        /* 协议模式 */
    wlan_channel_band_enum_uint8        band;        /* 频段 */
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;   /* 带宽 */
    osal_u8                           auc_resv[1];
} wal_ioctl_mode_map_stru;

osal_s32 wal_sync_post2hmac_no_rsp(osal_u8 vap_id, osal_u16 msg_id, osal_u8 *data, osal_u32 data_len);
osal_s32 wal_sync_send2device_no_rsp(osal_u8 vap_id, osal_u16 msg_id, osal_u8 *data, osal_u32 data_len);
osal_s32 wal_async_send2device_no_rsp(osal_u8 vap_id, osal_u16 msg_id, osal_u8 *data, osal_u32 data_len);
osal_s32 wal_async_post2hmac_no_rsp(osal_u8 vap_id, osal_u16 msg_id,
    osal_u8 *data, osal_u32 data_len, frw_post_pri_enum_uint8 pri);

#ifdef _PRE_WLAN_FEATURE_P2P
wlan_p2p_mode_enum_uint8 wal_wireless_iftype_to_mac_p2p_mode_etc(enum nl80211_iftype iftype);
#endif
osal_s32 wal_set_ssid(oal_net_device_stru *net_dev, const osal_u8 *ssid_ie, osal_u8 ssid_len);
osal_s32 wal_set_mode(oal_net_device_stru *net_dev, const wal_ioctl_mode_map_stru *mode_map,
    const osal_s8 *mode_str);
osal_s32 wal_set_channel_info(osal_u8 vap_id, osal_u8 channel,
    wlan_channel_band_enum_uint8 band, wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_s32 wal_down_vap(oal_net_device_stru *net_dev, osal_u8 vap_id);
osal_s32 wal_add_vap(oal_net_device_stru *net_dev, osal_u8 vap_id, osal_u8 vap_mode, osal_u8 p2p_mode);

osal_s32 wal_set_ap_max_user(oal_net_device_stru *net_dev, osal_u32 ap_max_user);
osal_s32 wal_get_rx_pckg(hmac_vap_stru *hmac_vap, osal_u32 data_op, osal_u32 print_info);
osal_void wal_set_ac_2g_enable(hmac_cfg_add_vap_param_stru *vap_info);
osal_u32 wal_set_service_control_etc(const service_control_msg *msg);
osal_u32 wal_get_service_control(osal_void);
osal_u32 wal_ccpriv_always_tx_get_param(osal_s8 *param, osal_u32 off_set,
    mac_rf_payload_enum_uint8 *payload_flag, osal_u32 *len, mac_cfg_tx_comp_stru *set_bcast_param);

oal_bool_enum_uint8 wal_is_dhcp_port_etc(const mac_ip_header_stru *ip_hdr);
mac_data_type_enum_uint8 wal_get_arp_type_by_arphdr(const oal_eth_arphdr_stru *rx_arp_hdr);
osal_u8 wal_get_data_type_from_8023_etc(const osal_u8 *frame_hdr, mac_netbuff_payload_type hdr_type);
#ifdef _PRE_WIFI_DEBUG
osal_void wal_print_tx_data_type(mac_ether_header_stru *ether_header);
#endif
osal_void wal_add_vap_set_cust(hmac_cfg_add_vap_param_stru *add_vap_param);

osal_u32 uapi_ccpriv_blacklist_add(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_blacklist_del(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 uapi_ccpriv_blacklist_show(oal_net_device_stru *net_dev, osal_s8 *param);

osal_void wal_print_nvram_list(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
