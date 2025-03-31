/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wlan_resource_common_rom.h 仅host侧引用内容
 * Date: 2022-11-26
 */
#ifndef __WLAN_RESOURCE_COMMON_H__
#define __WLAN_RESOURCE_COMMON_H__

#include "wlan_resource_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define     MAC_FCS_MAX_CHL_NUM                 2
#define     MAC_FCS_TIMEOUT_JIFFY  2
#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT2   1024    /* us */
#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT4   16000   /* us */
#define     MAC_ONE_PACKET_TIME_OUT4            2000
#define     MAC_FCS_CTS_MAX_BTCOEX_NOR_DURATION            32767  /* us */
#define     MAC_FCS_CTS_MAX_BTCOEX_LDAC_DURATION           65535  /* us */

/* MAC vap能力特性标识 */
typedef struct {
    osal_u32  uapsd                          : 1,
              txop_ps                        : 1,
              wpa                            : 1,
              wpa2                           : 1,
              dsss_cck_mode_40mhz            : 1,   /* 是否允许在40M上使用DSSS/CCK, 1-允许, 0-不允许 */
              rifs_tx_on                     : 1,
              tdls_prohibited                : 1,   /* tdls全局禁用开关， 0-不关闭, 1-关闭 */
              tdls_channel_switch_prohibited : 1,   /* tdls信道切换全局禁用开关， 0-不关闭, 1-关闭 */
              hide_ssid                      : 1,   /* AP开启隐藏ssid,  0-关闭, 1-开启 */
              wps                            : 1,   /* AP WPS功能:0-关闭, 1-开启 */
              support_11ac2g                         : 1,   /* 2.4G下的11ac:0-关闭, 1-开启 */
              keepalive                      : 1,   /* vap KeepAlive功能开关: 0-关闭, 1-开启 */
              smps                           : 2,   /* vap 当前SMPS能力(暂时不使用) */
              dpd_enbale                     : 1,   /* dpd是否开启 */
              dpd_done                       : 1,   /* dpd是否完成 */
              txbf_11n                        : 1,   /* 11n txbf能力 */
              disable_2ght40                 : 1,   /* 2ght40禁止位，1-禁止，0-不禁止 */
              peer_obss_scan                 : 1,   /* 对端支持obss scan能力: 0-不支持, 1-支持 */
              is_1024qam                        : 1,   /* 对端支持1024qam能力: 0-不支持, 1-支持 */
              wmm_cap                        : 1,   /* 保存与STA关联的AP是否支持wmm能力信息 */
              is_interworking                : 1,   /* 保存与STA关联的AP是否支持interworking能力 */
              ip_filter                      : 1,   /* rx方向ip包过滤的功能 */
              opmode                         : 1,   /* 对端的probe rsp中的extended capbilities 是否支持OPMODE */
              nb                             : 1,   /* 硬件是否支持窄带 */
              autoswitch_2040                : 1,   /* 是否支持随环境自动2040带宽切换 */
              custom_siso_2g                 : 1,   /* 2g是否定制化单天线siso,默认等于0,初始双天线 */
              custom_siso_5g                 : 1,   /* 5g是否定制化单天线siso,默认等于0,初始双天线 */
              wur_enable : 1, /* ap wur 能力开关 */
              wpa3                           : 1,
              bit_resv                           : 2;
} mac_cap_flag_stru;

typedef struct {
    osal_u8 tidno;      /* tidno，部分action帧使用 */
    osal_u8 resv[3];    /* 保留3个字节对齐 */
    osal_u8 initiator;  /* 触发端方向 */
    /* 以下为接收到req帧，发送rsp帧后，需要同步到dmac的内容 */
    osal_u8 status;                         /* rsp帧中的状态 */
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN];   /* 用于DELBA/DELTS查找HMAC用户 */
} dmac_ctx_action_event_stru;

typedef enum {
    MAC_FCS_HOOK_ID_DBAC,
    MAC_FCS_HOOK_ID_ACS,

    MAC_FCS_HOOK_ID_BUTT
} mac_fcs_hook_id_enum;
typedef osal_u8   mac_fcs_hook_id_enum_uint8;

typedef struct {
    osal_u8 ssid[WLAN_SSID_MAX_LEN];
    osal_u8 resv[3];     /* 预留3字节对齐 */
} mac_ssid_stru;

typedef struct {
    mac_fcs_notify_func func;
} mac_fcs_notify_node_stru;

typedef struct {
    mac_fcs_notify_node_stru notify_nodes[MAC_FCS_HOOK_ID_BUTT];
} mac_fcs_notify_chain_stru;

typedef struct {
    osal_u32 offset_addr;
    osal_u32 value[MAC_FCS_MAX_CHL_NUM];
} mac_fcs_reg_record_stru;

/*
 * enum nl80211_mfp - Management frame protection state
 * @NL80211_MFP_NO: Management frame protection not used
 * @NL80211_MFP_REQUIRED: Management frame protection required
 */
typedef enum {
    MAC_NL80211_MFP_NO,
    MAC_NL80211_MFP_REQUIRED,

    MAC_NL80211_MFP_BUTT
} mac_nl80211_mfp_enum;
typedef osal_u8 mac_nl80211_mfp_enum_uint8;

#define SERVICE_CONTROL_MGMT_FRAME_TX BIT0
#define SERVICE_CONTROL_MGMT_FRAME_RX BIT1
#define SERVICE_CONTROL_DATA_FRAME_TX BIT2
#define SERVICE_CONTROL_DATA_FRAME_RX BIT3
#define SERVICE_CONTROL_SCAN BIT4
#define SERVICE_CONTROL_SKIP_PROBE_FRAME BIT5
#define SERVICE_CONTROL_DROP_FRAME BIT6

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_resource_common.h */
