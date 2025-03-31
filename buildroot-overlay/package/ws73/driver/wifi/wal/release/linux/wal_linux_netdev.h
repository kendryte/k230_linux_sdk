/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal net dev api.
 * Create: 2021-07-19
 */

#ifndef __WAL_LINUX_NETDEV_H__
#define __WAL_LINUX_NETDEV_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "wal_linux_util.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_NETDEV_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WAL_SIOCDEVPRIVATE              0x89F0  /* SIOCDEVPRIVATE */

typedef enum {
    NETDEV_STA_MAC_INDEX,
    NETDEV_AP_MAC_INDEX,
    NETDEV_P2P_MAC_INDEX,
    NETDEV_MAC_INDEX_BUTT
}netdev_mac_index_enum;

extern oal_net_device_ops_stru g_st_wal_net_dev_ops_etc;
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
extern oal_ethtool_ops_stru g_st_wal_ethtool_ops_etc;
#endif
wal_ap_config_stru *wal_get_ap_config_info(osal_void);
osal_s32  wal_cfg_vap_h2d_event_etc(oal_net_device_stru *net_dev);
osal_void wlan_get_addr_from_plat(osal_void);
osal_u32 wlan_get_mac(osal_u8 *mac_addr, osal_u8 len, oal_net_device_stru *net_dev);
osal_u32 wlan_set_mac(osal_u8 *mac_addr, osal_u8 len, oal_net_device_stru *net_dev);
osal_s8 *wal_get_wlan_name_config(osal_u8 netdev_index);
osal_void wal_set_wlan_name_config(osal_u8 netdev_index, osal_s8 *name, osal_u8 len);
osal_void wal_set_wlan_name_default(osal_u8 netdev_index);

#if defined(_PRE_PRODUCT_ID_HOST)
osal_s32 wal_start_vap_etc(oal_net_device_stru *net_dev);
osal_s32  wal_stop_vap_etc(oal_net_device_stru *net_dev);
osal_s32 wal_netdev_stop_ap_etc(oal_net_device_stru *net_dev);
osal_s32 wal_init_wlan_vap_etc(oal_net_device_stru *net_dev);
osal_s32 wal_init_wlan_netdev_etc(oal_wiphy_stru *wiphy, enum nl80211_iftype type);
osal_s32  wal_setup_ap_etc(oal_net_device_stru *net_dev);
#endif
osal_void wal_set_ap_power_flag(osal_u8 flag);

osal_s32  wal_host_dev_init_etc(oal_net_device_stru *net_dev);
osal_s32  wal_host_dev_exit(oal_net_device_stru *net_dev);

osal_s32 wal_netdev_stop_etc(oal_net_device_stru *net_dev);
osal_s32 wal_netdev_open_etc(oal_net_device_stru *net_dev, osal_u8 entry_flag);

osal_u8 plat_is_device_in_recovery(osal_void);
#ifdef CONTROLLER_CUSTOMIZATION
netdev_tx_t wal_bridge_vap_xmit_etc(struct sk_buff *buf, struct net_device *dev);
#else
osal_s32  wal_bridge_vap_xmit_etc(oal_netbuf_stru *buf, oal_net_device_stru *dev);
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_void hwifi_config_init_force_etc(osal_void);
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
osal_s32 wal_insmod_ko_finish(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void wal_insmod_ko_begin(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
