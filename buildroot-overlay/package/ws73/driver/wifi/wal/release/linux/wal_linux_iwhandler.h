/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal iwconfig & iwpriv api.
 * Create: 2021-07-16
 */

#ifndef WAL_LINUX_IW_H
#define WAL_LINUX_IW_H

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"
#include "frw_ext_if.h"
#ifdef _PRE_WLAN_WIRELESS_EXT
#include "hmac_ext_if.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_IW_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* IOCTL私有配置命令宏定义 */
#define WAL_IOCTL_PRIV_SETPARAM          (OAL_SIOCIWFIRSTPRIV + 0)
#define WAL_IOCTL_PRIV_GETPARAM          (OAL_SIOCIWFIRSTPRIV + 1)
#define WAL_IOCTL_PRIV_SET_WMM_PARAM     (OAL_SIOCIWFIRSTPRIV + 3)
#define WAL_IOCTL_PRIV_GET_WMM_PARAM     (OAL_SIOCIWFIRSTPRIV + 5)
#define WAL_IOCTL_PRIV_SET_COUNTRY       (OAL_SIOCIWFIRSTPRIV + 8)
#define WAL_IOCTL_PRIV_GET_COUNTRY       (OAL_SIOCIWFIRSTPRIV + 9)

#define WAL_IOCTL_PRIV_GET_MODE     (OAL_SIOCIWFIRSTPRIV + 17)      /* 读取模式 */
#define WAL_IOCTL_PRIV_SET_MODE     (OAL_SIOCIWFIRSTPRIV + 13)      /* 设置模式 包括协议 频段 带宽 */

#define WAL_IOCTL_PRIV_AP_GET_STA_LIST               (OAL_SIOCIWFIRSTPRIV + 21)
#define WAL_IOCTL_PRIV_AP_MAC_FLTR                   (OAL_SIOCIWFIRSTPRIV + 22)
/* netd将此配置命令作为GET方式下发，get方式命令用奇数，set用偶数 */
#define WAL_IOCTL_PRIV_SET_AP_CFG                    (OAL_SIOCIWFIRSTPRIV + 23)
#define WAL_IOCTL_PRIV_AP_STA_DISASSOC               (OAL_SIOCIWFIRSTPRIV + 24)

#define WAL_IOCTL_PRIV_SET_MGMT_FRAME_FILTERS        (OAL_SIOCIWFIRSTPRIV + 28)      /* 设置管理帧过滤 */

#define WAL_IOCTL_PRIV_GET_BLACKLIST        (OAL_SIOCIWFIRSTPRIV + 27)

#ifdef _PRE_WLAN_FEATURE_DFS
#define WAL_IOCTL_PRIV_GET_DFS_CHN_STAT      (OAL_SIOCIWFIRSTPRIV + 29)
#endif

enum iwcofig_mode {
    IWCONFIG_NULL,
    IWCONFIG_AD_HOC,
    IWCONFIG_MANAGED,
    IWCONFIG_MASTER,
    IWCONFIG_REPEATER,
    IWCONFIG_SECONDARY,
    IWCONFIG_MONITOR,
    IWCONFIG_UNKNOW_BUG
};

#ifdef _PRE_WLAN_WIRELESS_EXT
/* wal 需要收到 hmac消息，wal才会将消息传递给其它人 */
typedef struct {
    osal_u8 lock;
    osal_bool wal_hmac_sync_flag;
    osal_u8 status;
    osal_u8 data[SYNC_DATA_MAX];
} wal_hmac_wait_sync_stru;
osal_s32 wal_wait_hmac_sync_data(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
