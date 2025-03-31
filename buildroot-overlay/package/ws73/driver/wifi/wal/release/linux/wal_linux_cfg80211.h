/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 * Description: wal_linux_cfg80211.c 的头文件
 * Create: 2012年11月8日
 */

#ifndef __WAL_LINUX_CFG80211_H__
#define __WAL_LINUX_CFG80211_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_types.h"
#include "wal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_ext_if.h"
#include "wal_linux_scan.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFG80211_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define     WAL_MAX_SCAN_TIME_PER_CHANNEL  400

#define     WAL_MAX_SCAN_TIME_PER_SCAN_REQ (5 * 1000)      /* wpa_s下发扫描请求，超时时间为5s，单位为ms */

/* channel index and frequence */
#define WAL_MIN_CHANNEL_2G      1
#define WAL_MAX_CHANNEL_2G      14

#define WAL_MIN_CHANNEL_5G      36
#define WAL_MAX_CHANNEL_5G      165

#define WAL_MIN_CHANNEL_4_9G    184
#define WAL_MAX_CHANNEL_4_9G    196

#define WAL_MIN_FREQ_2G    (2412 + 5*(WAL_MIN_CHANNEL_2G - 1))
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
#define WAL_MAX_FREQ_2G    2512
#else
#define WAL_MAX_FREQ_2G    2484
#endif
#define WAL_MIN_FREQ_5G    (5000 + 5*(WAL_MIN_CHANNEL_5G))
#define WAL_MAX_FREQ_5G    (5000 + 5*(WAL_MAX_CHANNEL_5G))

/* channel nums */
#define WAL_SCAN_CHANNEL_MAX_NUM \
    ((WAL_MAX_CHANNEL_2G - WAL_MIN_CHANNEL_2G + 1) + (WAL_MAX_CHANNEL_4_9G - WAL_MIN_CHANNEL_5G + 1))

/* wiphy 结构体初始化变量 */
#define WAL_MAX_SCAN_IE_LEN        1000
/* 802.11n HT 能力掩码 */
#define IEEE80211_HT_CAP_LDPC_CODING        0x0001
#define IEEE80211_HT_CAP_SUP_WIDTH_20_40    0x0002
#define IEEE80211_HT_CAP_SM_PS          0x000C
#define IEEE80211_HT_CAP_SM_PS_SHIFT    2
#define IEEE80211_HT_CAP_GRN_FLD        0x0010
#define IEEE80211_HT_CAP_SGI_20         0x0020
#define IEEE80211_HT_CAP_SGI_40         0x0040
#define IEEE80211_HT_CAP_TX_STBC        0x0080
#define IEEE80211_HT_CAP_RX_STBC        0x0300
#define IEEE80211_HT_CAP_DELAY_BA       0x0400
#define IEEE80211_HT_CAP_MAX_AMSDU      0x0800
#define IEEE80211_HT_CAP_DSSSCCK40      0x1000
#define IEEE80211_HT_CAP_RESERVED       0x2000
#define IEEE80211_HT_CAP_40MHZ_INTOLERANT   0x4000
#define IEEE80211_HT_CAP_LSIG_TXOP_PROT     0x8000

/* 802.11ac VHT Capabilities */
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895          0x00000000
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991          0x00000001
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454         0x00000002
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ        0x00000004
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ   0x00000008
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK          0x0000000C
#define IEEE80211_VHT_CAP_RXLDPC                0x00000010
#define IEEE80211_VHT_CAP_SHORT_GI_80               0x00000020
#define IEEE80211_VHT_CAP_SHORT_GI_160              0x00000040
#define IEEE80211_VHT_CAP_TXSTBC                0x00000080
#define IEEE80211_VHT_CAP_RXSTBC_1              0x00000100
#define IEEE80211_VHT_CAP_RXSTBC_2              0x00000200
#define IEEE80211_VHT_CAP_RXSTBC_3              0x00000300
#define IEEE80211_VHT_CAP_RXSTBC_4              0x00000400
#define IEEE80211_VHT_CAP_RXSTBC_MASK               0x00000700
#define IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE         0x00000800
#define IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE         0x00001000
#define IEEE80211_VHT_CAP_BEAMFORMER_ANTENNAS_MAX       0x00006000
#define IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MAX       0x00030000
#define IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE         0x00080000
#define IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE         0x00100000
#define IEEE80211_VHT_CAP_VHT_TXOP_PS               0x00200000
#define IEEE80211_VHT_CAP_HTC_VHT               0x00400000
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT  23
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK   \
    (7 << IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT)
#define IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_UNSOL_MFB 0x08000000
#define IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB   0x0c000000
#define IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN            0x10000000
#define IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN            0x20000000

/* management */
#define IEEE80211_STYPE_ASSOC_REQ       0x0000
#define IEEE80211_STYPE_ASSOC_RESP      0x0010
#define IEEE80211_STYPE_REASSOC_REQ     0x0020
#define IEEE80211_STYPE_REASSOC_RESP    0x0030
#define IEEE80211_STYPE_PROBE_REQ       0x0040
#define IEEE80211_STYPE_PROBE_RESP      0x0050
#define IEEE80211_STYPE_BEACON          0x0080
#define IEEE80211_STYPE_ATIM            0x0090
#define IEEE80211_STYPE_DISASSOC        0x00A0
#define IEEE80211_STYPE_AUTH            0x00B0
#define IEEE80211_STYPE_DEAUTH          0x00C0
#define IEEE80211_STYPE_ACTION          0x00D0

#define WAL_COOKIE_ARRAY_SIZE           8       /* 采用8bit 的map 作为保存cookie 的索引状态 */
#define WAL_MGMT_TX_TIMEOUT_MSEC        500     /* WAL 发送管理帧超时时间, 考虑到发送过程可能依赖于节能缓存机制，
                                                    需要将等待时间延长至几倍于TBTT，实际上，wpa_supplicant层发送未关联时的管理帧时，超时时间确实是1000ms，
                                                    比如Device Discoverability Request帧 */
#define WAL_MGMT_TX_RETRY_CNT           8       /* WAL 发送管理帧最大重传次数 */

#define IEEE80211_FCTL_FTYPE            0x000c
#define IEEE80211_FCTL_STYPE            0x00f0
#define IEEE80211_FTYPE_MGMT            0x0000

#define WAL_GET_STATION_THRESHOLD 1000 /* 固定时间内允许一次抛事件读DMAC RSSI */

typedef struct cookie_arry {
    osal_u64  ull_cookie;
    osal_u32  record_time;
} cookie_arry_stru;


#define ratetab_ent(_rate, _rateid, _flags)     \
{                                                               \
    .bitrate        = (_rate),                                  \
    .hw_value       = (_rateid),                                \
    .flags          = (_flags),                                 \
}

#define chan2g(_channel, _freq, _flags)  \
{                       \
    .band                   = OAL_IEEE80211_BAND_2GHZ,          \
    .center_freq            = (_freq),                      \
    .hw_value               = (_channel),                   \
    .flags                  = (_flags),                     \
    .max_antenna_gain       = 0,                            \
    .max_power              = 30,                           \
}

#define chan5g(_channel, _flags) \
{                                              \
    .band                   = OAL_IEEE80211_BAND_5GHZ,          \
    .center_freq            = 5000 + (5 * (_channel)),      \
    .hw_value               = (_channel),                   \
    .flags                  = (_flags),                     \
    .max_antenna_gain       = 0,                            \
    .max_power              = 30,                           \
}

#define chan4_9g(_channel, _flags) \
{                                              \
    .band                   = OAL_IEEE80211_BAND_5GHZ,          \
    .center_freq            = 4000 + (5 * (_channel)),      \
    .hw_value               = (_channel),                   \
    .flags                  = (_flags),                     \
    .max_antenna_gain       = 0,                            \
    .max_power              = 30,                           \
}


#define WAL_MIN_RTS_THRESHOLD 256
#define WAL_MAX_RTS_THRESHOLD 0xFFFF

#define WAL_MAX_FRAG_THRESHOLD 7536
#define WAL_MIN_FRAG_THRESHOLD 256

#define WAL_MAX_WAIT_TIME 3000
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
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2015年1月7日
    修改内容   : 新生成函数
  2.日    期   : 2015年1月7日
    修改内容   : 判断是否是probe response
    check if IEEE80211_FTYPE_MGMT && IEEE80211_STYPE_PROBE_RESP
    @fc: frame control bytes in little-endian byteorder

*****************************************************************************/
static inline osal_u32 oal_ieee80211_is_probe_resp(osal_u16 fc)
{
    return (fc & (IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);
}

extern osal_void  wal_cfg80211_exit_etc(osal_void);

extern osal_u32  wal_cfg80211_init_etc(osal_void);

extern osal_s32  wal_cfg80211_mgmt_tx_status_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32 wal_cfg80211_cac_report(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef _PRE_WLAN_FEATURE_ROAM
extern osal_s32  wal_roam_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif /* _PRE_WLAN_FEATURE_ROAM */
#ifdef _PRE_WLAN_FEATURE_11R
extern osal_s32 wal_ft_event_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif /* _PRE_WLAN_FEATURE_11R */

extern osal_void wal_cfg80211_unregister_netdev_etc(oal_net_device_stru *net_dev);

extern osal_u32 wal_del_p2p_group_etc(hmac_device_stru *hmac_device);


extern osal_u32 wal_cfg80211_add_vap_etc(hmac_cfg_add_vap_param_stru *add_vap_param);

extern osal_void wal_cfg80211_reset_bands_etc(osal_u8 dev_id);
extern osal_void wal_cfg80211_save_bands_etc(osal_u8 dev_id);
#ifdef _PRE_WLAN_FEATURE_M2S
extern osal_s32 wal_cfg80211_m2s_status_report(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
osal_s32 uapi_cfg80211_external_auth_status(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev,
    oal_external_auth_stru *param);

osal_s32 wal_cfg80211_trigger_external_auth(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_cfg80211_report_owe_info(hmac_vap_stru *hmac_vap, frw_msg *msg);

#endif

osal_s32 wal_report_csi_msg(hmac_vap_stru *hmac_vap, frw_msg *msg);
oal_ieee80211_supported_band *wal_cfg80211_get_ieee80211_supported_band(void);
osal_s32 wal_cfg80211_change_virtual_intf(oal_net_device_stru *net_dev, hmac_cfg_del_vap_param_stru *del_vap_param,
    hmac_cfg_add_vap_param_stru *add_vap_param, enum nl80211_iftype type);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_linux_cfg80211.h */
