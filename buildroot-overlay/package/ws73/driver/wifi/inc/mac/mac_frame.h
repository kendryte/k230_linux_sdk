/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Source file defined by the corresponding frame structure and operation interface (cannot be called by
 * the HAL module).
 * Create: 2020-7-9
 */

#ifndef __MAC_FRAME_H__
#define __MAC_FRAME_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "wlan_types_common.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oal_netbuf.h"
#include "oal_net.h"
#include "mac_frame_common.h"
#include "wlan_resource_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HTC_INVALID_VALUE            0x3
/* he mu edca param */
#define MAC_HE_SRG_PD_MIN            (-82)
#define MAC_HE_SRG_PD_MAX            (-62)
#define MAC_HE_SRG_BSS_COLOR_BITMAP_LEN 8
#define MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN 8

#define MAC_IEEE80211_FCTL_FTYPE 0x000c /* 帧类型掩码 */
#define MAC_IEEE80211_FCTL_STYPE 0x00f0 /* 帧子类型掩码 */

#define MAC_IEEE80211_FC0_SUBTYPE_SHIFT 4

#define MAC_IEEE80211_FTYPE_MGMT 0x0000 /* 管理帧 */
#define MAC_IEEE80211_FTYPE_CTL 0x0004  /* 控制帧 */
#define MAC_IEEE80211_FTYPE_DATA 0x0008 /* 数据帧 */

#define MAC_SUBMSDU_LENGTH_OFFSET 12 /* submsdu的长度字段的偏移值 */
#define MAC_SUBMSDU_DA_OFFSET 0      /* submsdu的目的地址的偏移值 */
#define MAC_SUBMSDU_SA_OFFSET 6      /* submsdu的源地址的偏移值 */

#define MAC_80211_CTL_HEADER_LEN 16 /* 控制帧帧头长度 */
#define MAC_80211_4ADDR_FRAME_LEN 30
#define MAC_80211_QOS_FRAME_LEN 26
#define MAC_80211_QOS_HTC_FRAME_LEN 30
#define MAC_80211_QOS_4ADDR_FRAME_LEN 32
#define MAC_80211_QOS_HTC_4ADDR_FRAME_LEN 36

/* 信息元素长度定义 */
#define MAC_IE_EXT_HDR_LEN 3          /* 信息元素头部 1字节EID + 1字节长度 + 1字节EXT_EID */
#define MAC_NEIGHBOR_REPORT_IE_LEN 13 /* NEIGHBOR_REPORT长度 */

#define MAC_SSID_OFFSET 12
#define MAC_LISTEN_INT_LEN 2
#define MAC_MIN_XRATE_LEN 1
#define MAC_MIN_RATES_LEN 1
#define MAC_MAX_SUPRATES 8 /* WLAN_EID_RATES最大支持8个速率 */
#define MAC_DSPARMS_LEN 1  /* ds parameter set 长度 */
#define MAC_DEFAULT_TIM_LEN 4
#define MAC_MIN_RSN_LEN 12
#define MAC_MAX_RSN_LEN 64
#define MAC_MIN_WPA_LEN        12
#define MAC_TIM_LEN_EXCEPT_PVB 3 /* DTIM Period、DTIM Count与BitMap Control三个字段的长度 */
#define MAC_CONTRY_CODE_LEN 3    /* 国家码长度为3 */
#define MAC_MIN_COUNTRY_LEN 6
#define MAC_MAX_COUNTRY_LEN 254
#define MAC_PWR_CONSTRAINT_LEN 1 /* 功率限制ie长度为1 */
#define MAC_QUIET_IE_LEN 6       /* quiet信息元素长度 */
#define MAC_TPCREP_IE_LEN 2
#define MAC_ERP_IE_LEN 1
#define MAC_OBSS_SCAN_IE_LEN 14
#define MAC_MIN_XCAPS_LEN 1

#define MAC_XCAPS_EX_FTM_LEN 9 /* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9 */
#define MAC_XCAPS_EX_LEN 8     /* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9 */
#define MAC_WMM_PARAM_LEN 24   /* WMM parameters ie */
#define MAC_WMM_INFO_LEN 7     /* WMM info ie */
#ifdef _PRE_WLAN_FEATURE_WMMAC
#define MAC_WMMAC_INFO_LEN          61    /* WMMAC info ie */
#define MAC_WMMAC_TSPEC_LEN         55    /* TSPEC元素长度 */
#endif
#define MAC_QOS_INFO_LEN 1

#define MAC_BSS_LOAD_IE_LEN 5
#define MAC_COUNTRY_REG_FIELD_LEN 3
#define MAC_LIS_INTERVAL_IE_LEN 2 /* listen interval信息元素长度 */
#define MAC_AID_LEN 2
#define MAC_PWR_CAP_LEN 2
#define MAC_AUTH_ALG_LEN 2
#define MAC_AUTH_TRANS_SEQ_NUM_LEN 2 /* transaction seq num信息元素长度 */
#define MAC_STATUS_CODE_LEN 2
#define MAC_VHT_CAP_IE_LEN 12
#define MAC_VHT_INFO_IE_LEN 5
#define MAC_VHT_CAP_INFO_FIELD_LEN 4
#define MAC_TIMEOUT_INTERVAL_INFO_LEN 5
#define MAC_VHT_CAP_RX_MCS_MAP_FIELD_LEN 2      /* vht cap ie rx_mcs_map field length */
#define MAC_VHT_CAP_RX_HIGHEST_DATA_FIELD_LEN 2 /* vht cap ie rx_highest_data field length */
#define MAC_VHT_CAP_TX_MCS_MAP_FIELD_LEN 2      /* vht cap ie tx_mcs_map field length */
#define MAC_VHT_OPERN_LEN 5                     /* vht opern ie length */
#define MAC_VHT_OPERN_INFO_FIELD_LEN 3          /* vht opern ie infomation field length */
#define MAC_2040_COEX_LEN 1                     /* 20/40 BSS Coexistence element */
#define MAC_2040_INTOLCHREPORT_LEN_MIN 1        /* 20/40 BSS Intolerant Channel Report element */
#define MAC_CHANSWITCHANN_LEN 3                 /* Channel Switch Announcement element */
#define MAC_SA_QUERY_LEN 4                      /* SA Query element len */
#define MAC_RSN_VERSION_LEN 2                   /* wpa/RSN version len */
#define MAC_RSN_CIPHER_COUNT_LEN 2              /* RSN IE Cipher count len */
#define MAC_11N_TXBF_CAP_OFFSET 23
#define MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN 3
#define MAC_RRM_ENABLE_CAP_IE_LEN 5
#define MAC_WFA_TPC_RPT_LEN 7
#define MAC_CHANSWITCHANN_IE_LEN 3
#define MAC_EXT_CHANSWITCHANN_IE_LEN 4
#define MAC_SEC_CH_OFFSET_IE_LEN 1
#define MAC_WIDE_BW_CH_SWITCH_IE_LEN 3
#define MAC_FTMP_LEN 9
#define MAC_MEASUREMENT_REQUEST_IE_OFFSET 5
#define MAC_RM_MEASUREMENT_REQUEST_IE_OFFSET 5
#define MAC_ACTION_CATEGORY_AND_CODE_LEN 2

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#define MAC_OPMODE_NOTIFY_LEN 1 /* Operating Mode Notification element len */
#endif
#define MAC_MOBILITY_DOMAIN_LEN 5

#define MAC_P2P_ATTRIBUTE_HDR_LEN 3   /* P2P_ATTRIBUTE信息元素头部 1字节ATTRIBUTE + 2字节长度 */
#define MAC_P2P_LISTEN_CHN_ATTR_LEN 5 /* LISTEN CHANNEL ATTRIBUTE长度 */
#define MAC_P2P_MIN_IE_LEN 4          /* P2P IE的最小长度 */

#define MAC_MIN_WPS_IE_LEN 5
#define MAC_WPS_ATTRIBUTE_HDR_LEN 4   /* WPS_ATTRIBUTE信息元素头部 2字节ATTRIBUTE TYPE + 2字节长度 */
#define MAC_WPS_ATTRIBUTE_REQUEST_TYPE 0x103a   /* ATTRIBUTE_REQUEST_TYPE 0x103a */
#define WPS_IE_HDR_LEN 6
#define MAC_WPS_REQUEST_TYPE_ATTR_LEN 1 /* ATTRIBUTE_REQUEST_TYPE长度 */

/* Quiet 信息 */
#define MAC_QUIET_PERIOD 0
#define MAC_QUIET_COUNT MAC_QUIET_PERIOD
#define MAC_QUIET_DURATION 0x0000
#define MAC_QUIET_OFFSET 0x0000

/* RSN信息元素相关定义 */
#define MAC_RSN_IE_VERSION 1
#define MAC_RSN_CAP_LEN 2
#define MAC_PMKID_LEN 16

/* WPA 信息元素相关定义 */
#define MAC_WPA_IE_VERSION 1
#define WLAN_AKM_SUITE_WAPI_CERT 0x000FAC12

/* OUI相关定义 */
#define MAC_OUI_LEN 3
#define MAC_OUITYPE_LEN 1

#define MAC_OUITYPE_WPA 1
#define MAC_OUITYPE_WMM 2
#define MAC_OUITYPE_WPS 4
#define MAC_OUITYPE_WFA 8
#define MAC_OUITYPE_P2P 9

#define MAC_WMM_OUI_BYTE_ONE 0x00
#define MAC_WMM_OUI_BYTE_TWO 0x50
#define MAC_WMM_OUI_BYTE_THREE 0xF2
#define MAC_WMM_UAPSD_ALL (BIT0 | BIT1 | BIT2 | BIT3)
#define MAC_OUISUBTYPE_WFA 0x00
#define MAC_OUISUBTYPE_WMM_INFO 0
#define MAC_OUISUBTYPE_WMM_PARAM 1
#define MAC_OUISUBTYPE_WMM_PARAM_OFFSET 6 /* wmm 字段中EDCA_INFO位置,表示是否携带EDCA参数 偏移6 */
#define MAC_WMM_QOS_INFO_POS 8            /* wmm 字段中qos info位置，偏移8 */
#define MAC_OUI_WMM_VERSION 1
#ifdef _PRE_WLAN_FEATURE_WMMAC
#define MAC_OUISUBTYPE_WMMAC_TSPEC  2   /* WMMAC TSPEC OUI subtype */
#endif
#define MAC_HT_CAP_LEN 26           /* HT能力信息长度为26 */
#define MAC_HT_CAPINFO_LEN 2        /* HT Capabilities Info域长度为2 */
#define MAC_HT_AMPDU_PARAMS_LEN 1   /* A-MPDU parameters域长度为1 */
#define MAC_HT_SUP_MCS_SET_LEN 16   /* Supported MCS Set域长度为16 */
#define MAC_HT_EXT_CAP_LEN 2        /* Extended cap.域长度为2 */
#define MAC_HT_EXT_CAP_OPMODE_LEN 8 /* 宣称支持OPMODE的字段是extended cap的第8个字节 */
#define MAC_HT_TXBF_CAP_LEN 4       /* Transmit Beamforming Cap.域长度为4 */
#define MAC_HT_ASEL_LEN 1           /* ASEL Cap.域长度为1 */
#define MAC_HT_OPERN_LEN 22         /* HT Operation信息长度为22 */
#define MAC_HT_BASIC_MCS_SET_LEN 16 /* HT info中的basic mcs set信息的长度 */
#define MAC_HT_CTL_LEN 4            /* HT CONTROL字段的长度 */
#define MAC_QOS_CTL_LEN 2           /* QOS CONTROL字段的长度 */

#define MAC_HE_MAC_CAP_LEN 6
#define MAC_HE_PHY_CAP_LEN 11
#define MAC_HE_OPE_PARAM_LEN 3
#define MAC_HE_OPE_BASIC_MCS_NSS_LEN 2
#define MAC_HE_VHT_OPERATION_INFO_LEN 3
#define MAC_HE_MU_EDCA_PARAMETER_SET_LEN 14
#define MAC_HE_MCS_NSS_MIN_LEN 2
#define MAC_HE_CAP_MIN_LEN 20
#define MAC_HE_OPERAION_MIN_LEN 7
#define MAC_HE_OPERAION_WITH_VHT_LEN 10
/* EID Extertion(1) + Resource Request Buffer Threshold Exponent(1) = 2 */
#define MAC_HE_NFR_PARAM_PAYLOAD_LEN 2
/* EID(1) + Length(1) + EID Extertion(1) = 3 */
#define MAC_HE_NFR_PARAM_HEADER_LEN 3
/* NFR Parameter  默认值256 = 2^8 */
#define MAC_HE_NFR_PARAM_DEFAULT_VAL 8
#define MAC_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT_LEN 3

#define MAC_TAG_PARAM_OFFSET    (MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)

#define MAC_DEVICE_BEACON_OFFSET    (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)

#define MAC_MAX_BSS_INFO_TRANS 5

/* EDCA参数相关的宏 */
#define MAC_WMM_QOS_PARAM_AIFSN_MASK 0x0F
#define MAC_WMM_QOS_PARAM_ACI_BIT_OFFSET 5
#define MAC_WMM_QOS_PARAM_ACI_MASK 0x03
#define MAC_WMM_QOS_PARAM_ECWMIN_MASK 0x0F
#define MAC_WMM_QOS_PARAM_ECWMAX_MASK 0xF0
#define MAC_WMM_QOS_PARAM_ECWMAX_BIT_OFFSET 4
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_MASK 0x00FF
#define MAC_WMM_QOS_PARAM_BIT_NUMS_OF_ONE_BYTE 8
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_SAVE_TO_TRANS_TIMES 5

/* 关闭WMM后，所有帧进入此宏定义的队列 */
#define MAC_WMM_SWITCH_TID 6

/* chartiot信令包通过tcp端口号无法正确识别,chariot软件重启则端口号改变.识别逻辑无效 */
#define MAC_CHARIOT_NETIF_PORT 10115
#define MAC_WFD_RTSP_PORT 7236

/* Wavetest仪器识别 */
#define mac_is_wavetest_sta(bssid) \
    ((0x0 == ((bssid)[0])) && ((0x1 == ((bssid)[1])) || (0x2 == ((bssid)[1]))) && (0x1 == ((bssid)[2])))
/* 申请多个OUI */
#define MAC_WLAN_OUI_PUBLIC                 0x00E0fC
#define MAC_WLAN_OUI_TYPE_CASCADE    0xA0
#define MAC_WLAN_OUI_TYPE_4ADDR      0x40 /* 4地址IE的OUI TYPE字段 */
#ifdef _PRE_OS_VERSION_LITEOS /* HMAC和DMAC OS定义风格不一致 */
#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define MAC_WLAN_OUI_RSN0                   0x00        /* RSNA OUI 定义 0x000FAC */
#define MAC_WLAN_OUI_RSN1                   0x0F
#define MAC_WLAN_OUI_RSN2                   0xAC
#define MAC_WLAN_OUI_MICRO0                 0x00        /* WPA/WMM OUI 定义 0x0050F2 */
#define MAC_WLAN_OUI_MICRO1                 0x50
#define MAC_WLAN_OUI_MICRO2                 0xF2
#define MAC_WLAN_OUI_P2P0                   0x50        /* P2P OUI 定义 0x506F9A */
#define MAC_WLAN_OUI_P2P1                   0x6F
#define MAC_WLAN_OUI_P2P2                   0x9A
#define MAC_OUITYPE_ANY                     0xFD
#define MAC_ACTION_OUI_POS                  1
#define MAC_ACTION_VENDOR_TYPE_POS          4
#define MAC_ACTION_VENDOR_SUBTYPE_POS       5
#define MAC_ACTION_VENDOR_SPECIFIC_IE_POS   6
#endif
#endif
#define MAC_EXT_VENDER_IE 0xAC853D /* 打桩HW IE */

#define MAC_MBO_VENDOR_IE          0x506F9A /* WFA specific OUI */
#define MAC_MBO_IE_OUI_TYPE        0x16     /* Identifying the type and version of the MBO-OCE IE */
#define MAC_MBO_ATTRIBUTE_HDR_LEN  2        /* Attribute ID + Attribute Length */
#define MBO_IE_HEADER              6        /* type + length + oui + oui type */
#define MAC_MBO_OUI_LENGTH         3
#define MAC_MBO_ASSOC_DIS_ATTR_LEN 3

#define MAC_OSAL_1024QAM_IE 0xbd /* 1024QAM IE */

#define MAC_IPV6_UDP_SRC_PORT 546
#define MAC_IPV6_UDP_DES_PORT 547

/* Type4Bytes len4Bytes mac16Bytes mac26Bytes timestamp23Bytes RSSIInfo8Bytes SNR2Bytes */
#define MAC_REPORT_RSSIINFO_LEN 8
#define MAC_REPORT_SNR_LEN 2
#define MAC_REPORT_RSSIINFO_SNR_LEN 10
#define MAC_CSI_LOCATION_INFO_LEN 57
#define MAC_FTM_LOCATION_INFO_LEN 52
#define MAC_CSI_REPORT_HEADER_LEN 53
#define MAC_FTM_REPORT_HEADER_LEN 51
#define MAC_CSI_MAX_REPORT_LEN 1400

/* ARP types, 1: ARP request, 2:ARP response, 3:RARP request, 4:RARP response */
#define MAC_ARP_REQUEST 0x0001
#define MAC_ARP_RESPONSE 0x0002
#define MAC_RARP_REQUEST 0x0003
#define MAC_RARP_RESPONSE 0x0004

/* Neighbor Discovery */
#define MAC_ND_RSOL 133  /* Router Solicitation */
#define MAC_ND_RADVT 134 /* Router Advertisement */
#define MAC_ND_NSOL 135  /* Neighbor Solicitation */
#define MAC_ND_NADVT 136 /* Neighbor Advertisement */
#define MAC_ND_RMES 137  /* Redirect Message */

/* DHCPV6 Message type */
#define MAC_DHCPV6_SOLICIT 1
#define MAC_DHCPV6_ADVERTISE 2
#define MAC_DHCPV6_REQUEST 3
#define MAC_DHCPV6_CONFIRM 4
#define MAC_DHCPV6_RENEW 5
#define MAC_DHCPV6_REBIND 6
#define MAC_DHCPV6_REPLY 7
#define MAC_DHCPV6_RELEASE 8
#define MAC_DHCPV6_DECLINE 9
#define MAC_DHCPV6_RECONFIGURE 10
#define MAC_DHCPV6_INFORM_REQ 11
#define MAC_DHCPV6_RELAY_FORW 12
#define MAC_DHCPV6_RELAY_REPLY 13
#define MAC_DHCPV6_LEASEQUERY 14
#define MAC_DHCPV6_LQ_REPLY 15

#define mac_is_golden_ap(bssid) ((0x0 == (bssid)[0]) && (0x13 == (bssid)[1]) && (0xE9 == (bssid)[2]))
#define mac_is_belkin_ap(bssid) ((0x14 == (bssid)[0]) && (0x91 == (bssid)[1]) && (0x82 == (bssid)[2]))
#define mac_is_netgear_wndr_ap(bssid) \
    ((0x08 == (bssid)[0]) && (0xbd == (bssid)[1]) && (0x43 == (bssid)[2]))
#define mac_is_trendnet_ap(bssid) \
    ((0xd8 == (bssid)[0]) && (0xeb == (bssid)[1]) && (0x97 == (bssid)[2]))
#define mac_is_dlink_ap(bssid) \
    ((0xcc == (bssid)[0]) && (0xb2 == (bssid)[1]) && (0x55 == (bssid)[2]))
#define mac_is_haier_ap(bssid) \
    ((0x08 == (bssid)[0]) && (0x10 == (bssid)[1]) && (0x79 == (bssid)[2]))
#define MAC_WLAN_CHIP_OUI_ATHEROSC 0x00037f
#define MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC 0x1
#define MAC_WLAN_CHIP_OUI_RALINK 0x000c43
#define MAC_WLAN_CHIP_OUI_TYPE_RALINK 0x3
#define MAC_WLAN_CHIP_OUI_TYPE_RALINK1 0x7

/* p2p相关 */
/* GO negotiation */
#define P2P_PAF_GON_REQ 0
#define P2P_PAF_GON_RSP 1
#define P2P_PAF_GON_CONF 2
/* Provision discovery */
#define P2P_PAF_PD_REQ 7
/* P2P IE */
#define P2P_OUI_LEN 4
#define P2P_IE_HDR_LEN 6
#define P2P_ELEMENT_ID_SIZE 1
#define P2P_ATTR_ID_SIZE 1
#define P2P_ATTR_HDR_LEN 3
#define WFA_OUI_BYTE1 0x50
#define WFA_OUI_BYTE2 0x6F
#define WFA_OUI_BYTE3 0x9A
#define WFA_P2P_V1_0 0x09

#define MAC_VHT_CHANGE (BIT1)
#define MAC_HT_CHANGE (BIT2)
#define MAC_BW_CHANGE (BIT3)
#define MAC_HE_CHANGE (BIT4)
#define MAC_HE_BSS_COLOR_CHANGE (BIT7)
#define MAC_HE_PARTIAL_BSS_COLOR_CHANGE (BIT8)
#define MAC_HE_BSS_COLOR_DISALLOW (BIT9)
#define MAC_BW_DIFF_AP_USER (BIT5)
#define MAC_BW_OPMODE_CHANGE (BIT6)
#define MAC_RTS_TXOP_THRED_CHANGE (BIT10)
#define MAC_NO_CHANGE (0)

#define MAC_FRAME_REQ_TYPE 1
#define MAC_TSC_REQ_BIN0_RANGE 10
#define MAC_TSC_TID_MASK(_tid) (((_tid)<<4)&0xF)
#define MAC_RADIO_MEAS_START_TIME_LEN 8

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#define MAC_AMSDU_SKB_LEN_DOWN_LIMIT 1000 /* 超过此长度帧可以AMSDU聚合 */
#define MAC_AMSDU_SKB_LEN_UP_LIMIT 1544

#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
#define MAC_WUR_CAP_SUPPORT_BANDS_LEN 1
#define MAC_WUR_CAP_INFORMATION_LEN 2
#define MAC_WUR_OPER_PARAM_LEN 10
#endif

#define MAC_PRIV_HIEX_IE 0x81
#define MAC_PRIV_CAP_IE 0x20

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* HMAC模块接收流程处理MSDU状态 */
typedef enum {
    MAC_PROC_ERROR = 0,
    MAC_PROC_LAST_MSDU,
    MAC_PROC_MORE_MSDU,
    MAC_PROC_LAST_MSDU_ODD,

    MAC_PROC_BUTT
} mac_msdu_proc_status_enum;
typedef osal_u8 mac_msdu_proc_status_enum_uint8;

/* HT Category下的Action值的枚举 */
typedef enum {
    MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH = 0,
    MAC_HT_ACTION_SMPS = 1,
    MAC_HT_ACTION_PSMP = 2,
    MAC_HT_ACTION_SET_PCO_PHASE = 3,
    MAC_HT_ACTION_CSI = 4,
    MAC_HT_ACTION_NON_COMPRESSED_BEAMFORMING = 5,
    MAC_HT_ACTION_COMPRESSED_BEAMFORMING = 6,
    MAC_HT_ACTION_ASEL_INDICES_FEEDBACK = 7,

    MAC_HT_ACTION_BUTT
} mac_ht_action_type_enum;
typedef osal_u8 mac_ht_action_type_enum_uint8;
/* Timeout_Interval ie中的类型枚举 */
typedef enum {
    MAC_TIE_REASSOCIATION_DEADLINE_INTERVAL  = 1,   /* 毫秒级 */
    MAC_TIE_KEY_LIFETIME_INTERVAL            = 2,   /* 秒级 */
    MAC_TIE_ASSOCIATION_COMEBACK_TIME        = 3,   /* 毫秒级 */

    MAC_TIE_BUTT
} mac_timeout_interval_type_enum;
typedef osal_u8 mac_timeout_interval_type_enum_uint8;

/* SA QUERY Category下的Action值的枚举 */
typedef enum {
    MAC_SA_QUERY_ACTION_REQUEST = 0,
    MAC_SA_QUERY_ACTION_RESPONSE = 1
} mac_sa_query_action_type_enum;
typedef osal_u8 mac_sa_query_action_type_enum_uint8;
typedef enum {
    MAC_FT_ACTION_REQUEST = 1,
    MAC_FT_ACTION_RESPONSE = 2,
    MAC_FT_ACTION_CONFIRM = 3,
    MAC_FT_ACTION_ACK = 4,
    MAC_FT_ACTION_BUTT = 5
} mac_ft_action_type_enum;
typedef osal_u8 mac_ft_action_type_enum_uint8;

/* Protected HE Category下的Action值的枚举 */
typedef enum {
    MAC_PROTECTED_HE_ACTION_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT = 0,
    MAC_PROTECTED_HE_ACTION_MU_EDCA_CONTROL = 1,

    MAC_PROTECTED_HE_ACTION_MAX
} mac_protected_he_action_type;
typedef osal_u8 mac_protected_he_action_type_enum_uint8;

/* WNM Category下的Action值的枚举 */
typedef enum {
    MAC_WNM_ACTION_EVENT_REQUEST = 0,
    MAC_WNM_ACTION_EVENT_REPORT = 1,
    MAC_WNM_ACTION_DIALGNOSTIC_REQUEST = 2,
    MAC_WNM_ACTION_DIALGNOSTIC_REPORT = 3,
    MAC_WNM_ACTION_LOCATION_CONF_REQUEST = 4,
    MAC_WNM_ACTION_LOCATION_CONF_RESPONSE = 5,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_QUERY = 6,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST = 7,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_RESPONSE = 8,
    MAC_WNM_ACTION_FMS_REQUEST = 9,
    MAC_WNM_ACTION_FMS_RESPONSE = 10,
    MAC_WNM_ACTION_COLLOCATED_INTER_REQUEST = 11,
    MAC_WNM_ACTION_COLLOCATEC_INTER_REPORT = 12,
    MAC_WNM_ACTION_TFS_REQUEST = 13,
    MAC_WNM_ACTION_TFS_RESPONSE = 14,
    MAC_WNM_ACTION_TFS_NOTIFY = 15,
    MAC_WNM_ACTION_SLEEP_MODE_REQUEST = 16,
    MAC_WNM_ACTION_SLEEP_MODE_RESPONSE = 17,
    MAC_WNM_ACTION_TIM_BROADCAST_REQUEST = 18,
    MAC_WNM_ACTION_TIM_BROADCAST_RESPONSE = 19,
    MAC_WNM_ACTION_QOS_TRAFFIC_CAP_UPDATE = 20,
    MAC_WNM_ACTION_CHANNEL_USAGE_REQUEST = 21,
    MAC_WNM_ACTION_CHANNEL_USAGE_RESPONSE = 22,
    MAC_WNM_ACTION_DMS_REQUEST = 23,
    MAC_WNM_ACTION_DMS_RESPONSE = 24,
    MAC_WNM_ACTION_TIMING_MEAS_REQUEST = 25,
    MAC_WNM_ACTION_NOTIFICATION_REQUEST = 26,
    MAC_WNM_ACTION_NOTIFICATION_RESPONSE = 27,

    MAC_WNM_ACTION_BUTT
} mac_wnm_action_type_enum;
typedef osal_u8 mac_wnm_action_type_enum_uint8;

/* 校准模式的枚举 */
typedef enum {
    MAC_NOT_SURPPORT_CLB = 0,
    MAC_RSP_CLB_ONLY = 1,
    MAC_SUPPOTR_CLB = 3,
    MAC_CLB_BUTT
} mac_txbf_clb_enum;
typedef osal_u8 mac_txbf_clb_enum_uint8;

/* Spectrum Management Category下的Action枚举值 */
typedef enum {
    MAC_SPEC_CH_SWITCH_ANNOUNCE = 4 /*  Channel Switch Announcement */
} mac_specmgmt_action_type_enum;
typedef osal_u8 mac_specmgmt_action_type_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_WMMAC
/* TS建立的状态枚举 */
typedef enum {
    MAC_TS_NONE        = 0,    /* TS不需要建立 */
    MAC_TS_INIT,               /* TS需要建立，未建立 */
    MAC_TS_INPROGRESS,         /* TS建立过程中 */
    MAC_TS_SUCCESS,            /* TS建立成功 */

    MAC_TS_BUTT
} mac_ts_conn_status_enum;
typedef osal_u8 mac_ts_conn_status_enum_uint8;
#endif

/* Capability Information field bit assignments  */
typedef enum {
    MAC_CAP_ESS = 0x01,             /* ESS capability               */
    MAC_CAP_IBSS = 0x02,            /* IBSS mode                    */
    MAC_CAP_POLLABLE = 0x04,        /* CF Pollable                  */
    MAC_CAP_POLL_REQ = 0x08,        /* Request to be polled         */
    MAC_CAP_PRIVACY = 0x10,         /* WEP encryption supported     */
    MAC_CAP_SHORT_PREAMBLE = 0x20,  /* Short Preamble is supported  */
    MAC_CAP_SHORT_SLOT = 0x400,     /* Short Slot is supported      */
    MAC_CAP_PBCC = 0x40,            /* PBCC                         */
    MAC_CAP_CHANNEL_AGILITY = 0x80, /* Channel Agility              */
    MAC_CAP_SPECTRUM_MGMT = 0x100,  /* Spectrum Management          */
    MAC_CAP_RADIO_MEAS      = 0x1000, /* Radio Measurement           */
    MAC_CAP_DSSS_OFDM = 0x2000 /* DSSS-OFDM                    */
} mac_capability_enum;
typedef osal_u16 mac_capability_enum_uint16;

typedef enum {
    MAC_EID_EXT_FTMSI = 9,
} mac_eid_extension_enum;
typedef osal_u8 mac_eid_extension_enum_uint8;

typedef enum {
    MAC_P2P_ATTRIBUTE_CAP = 2,
    MAC_P2P_ATTRIBUTE_GROUP_OI = 4,
    MAC_P2P_ATTRIBUTE_CFG_TIMEOUT = 5,
    MAC_P2P_ATTRIBUTE_LISTEN_CHAN = 6,
} mac_p2p_attribute_enum;
typedef osal_u8 mac_p2p_attribute_enum_uint8;

typedef enum {
    MAC_SMPS_STATIC_MODE = 0,  /*   静态SMPS   */
    MAC_SMPS_DYNAMIC_MODE = 1, /*   动态SMPS   */
    MAC_SMPS_MIMO_MODE = 3,    /* disable SMPS */

    MAC_SMPS_MODE_BUTT
} mac_mimo_power_save_enum;
typedef osal_u8 mac_mimo_power_save_mode_enum_uint8;

typedef enum {
    MAC_SCN = 0, /* 不存在次信道 */
    MAC_SCA = 1, /* 次信道在主信道之上(Secondary Channel Above) */
    MAC_SCB = 3, /* 次信道在主信道之下(Secondary Channel Below) */

    MAC_SEC_CH_BUTT,
} mac_sec_ch_off_enum;
typedef osal_u8 mac_sec_ch_off_enum_uint8;

/* P2P相关 */
typedef enum {
    P2P_STATUS = 0,
    P2P_MINOR_REASON_CODE = 1,
    P2P_CAPABILITY = 2,
    P2P_DEVICE_ID = 3,
    GROUP_OWNER_INTENT = 4,
    CONFIG_TIMEOUT = 5,
    LISTEN_CHANNEL = 6,
    P2P_GROUP_BSSID = 7,
    EXTENDED_LISTEN_TIMING = 8,
    INTENDED_P2P_IF_ADDR = 9,
    P2P_MANAGEABILITY = 10,
    P2P_CHANNEL_LIST = 11,
    NOTICE_OF_ABSENCE = 12,
    P2P_DEVICE_INFO = 13,
    P2P_GROUP_INFO = 14,
    P2P_GROUP_ID = 15,
    P2P_INTERFACE = 16,
    P2P_OPERATING_CHANNEL = 17,
    INVITATION_FLAGS = 18
} attribute_id_t;

typedef enum {
    P2P_PUB_ACT_OUI_OFF1 = 2,
    P2P_PUB_ACT_OUI_OFF2 = 3,
    P2P_PUB_ACT_OUI_OFF3 = 4,
    P2P_PUB_ACT_OUI_TYPE_OFF = 5,
    P2P_PUB_ACT_OUI_SUBTYPE_OFF = 6,
    P2P_PUB_ACT_DIALOG_TOKEN_OFF = 7,
    P2P_PUB_ACT_TAG_PARAM_OFF = 8
} p2p_pub_act_frm_off;

typedef enum {
    P2P_GEN_ACT_OUI_OFF1 = 1,
    P2P_GEN_ACT_OUI_OFF2 = 2,
    P2P_GEN_ACT_OUI_OFF3 = 3,
    P2P_GEN_ACT_OUI_TYPE_OFF = 4,
    P2P_GEN_ACT_OUI_SUBTYPE_OFF = 5,
    P2P_GEN_ACT_DIALOG_TOKEN_OFF = 6,
    P2P_GEN_ACT_TAG_PARAM_OFF = 7
} p2p_gen_act_frm_off;

typedef enum {
    P2P_NOA = 0,
    P2P_PRESENCE_REQ = 1,
    P2P_PRESENCE_RESP = 2,
    GO_DISC_REQ = 3
} p2p_gen_action_frm_type;

typedef enum {
    P2P_STAT_SUCCESS = 0,
    P2P_STAT_INFO_UNAVAIL = 1,
    P2P_STAT_INCOMP_PARAM = 2,
    P2P_STAT_LMT_REACHED = 3,
    P2P_STAT_INVAL_PARAM = 4,
    P2P_STAT_UNABLE_ACCO_REQ = 5,
    P2P_STAT_PREV_PROT_ERROR = 6,
    P2P_STAT_NO_COMMON_CHAN = 7,
    P2P_STAT_UNKNW_P2P_GRP = 8,
    P2P_STAT_GO_INTENT_15 = 9,
    P2P_STAT_INCOMP_PROV_ERROR = 10,
    P2P_STAT_USER_REJECTED = 11
} p2p_status_code_t;

/* Radio Measurement下的Action枚举值 */
typedef enum {
    MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST = 0,
    MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT = 1,
    MAC_RM_ACTION_LINK_MEASUREMENT_REQUEST = 2,
    MAC_RM_ACTION_LINK_MEASUREMENT_REPORT = 3,
    MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST = 4,
    MAC_RM_ACTION_NEIGHBOR_REPORT_RESPONSE = 5
} mac_rm_action_type_enum;
typedef osal_u8 mac_rm_action_type_enum_uint8;

typedef enum {
    MBO_ATTR_ID_AP_CAPA_IND = 1,
    MBO_ATTR_ID_NON_PREF_CHAN_REPORT = 2,
    MBO_ATTR_ID_CELL_DATA_CAPA = 3,
    MBO_ATTR_ID_ASSOC_DISALLOW = 4,
    MBO_ATTR_ID_CELL_DATA_PREF = 5,
    MBO_ATTR_ID_TRANSITION_REASON = 6,
    MBO_ATTR_ID_TRANSITION_REJECT_REASON = 7,
    MBO_ATTR_ID_ASSOC_RETRY_DELAY = 8,
} mac_mbo_attr_id_enum;
typedef osal_u8 mac_mbo_attr_id_enum_uint8;

#define MAC_WLAN_OUI_WFA 0x506f9a
#define MAC_WLAN_OUI_TYPE_WFA_P2P 9
#define MAC_WLAN_OUI_MICROSOFT 0x0050f2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WPA 1
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WMM 2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WPS 4

#define MAC_WLAN_OUI_VENDOR_VHT_HEADER 5
#define MAC_WLAN_OUI_BROADCOM_EPIGRAM 0x00904c /* Broadcom (Epigram) */
#define MAC_WLAN_OUI_VENDOR_VHT_TYPE 0x04
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE 0x08
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE2 0x00
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE3 0x07

#define MAC_WLAN_OUI_VENDOR_HT_CAPAB_OUI_TYPE 0x33 /* 00-90-4c:0x33 */

/* eapol key 结构宏定义 */

/*****************************************************************************
    全局变量声明
*****************************************************************************/
/* WMM OUI定义 */
extern const osal_u8    g_wmm_oui_etc[MAC_OUI_LEN];

/* P2P OUI 定义 */
extern const osal_u8    g_p2p_oui_etc[MAC_OUI_LEN];

/* 窄带 OUI 定义 */
extern const osal_u8    g_mac_vendor_oui[MAC_OUI_LEN];

/*****************************************************************************
   STRUCT定义
*****************************************************************************/
/* 此文件中结构体与协议一致，要求1字节对齐，统一加OAL_DECLARE_PACKED */
#pragma pack(push, 1)
/* 四地址帧结构体 */
struct mac_ieee80211_frame_addr4 {
    mac_header_frame_control_stru frame_control;
    osal_u16                      duration_value      : 15,
                                  duration_flag       : 1;
    osal_u8                       address1[WLAN_MAC_ADDR_LEN];
    osal_u8                       address2[WLAN_MAC_ADDR_LEN];
    osal_u8                       address3[WLAN_MAC_ADDR_LEN];
    osal_u16                      frag_num            : 4,
                                  seq_num             : 12;
    osal_u8                       address4[WLAN_MAC_ADDR_LEN];
};
typedef struct mac_ieee80211_frame_addr4 mac_ieee80211_frame_addr4_stru;

/* qos四地址帧结构 */
struct mac_ieee80211_qos_frame_addr4 {
    mac_header_frame_control_stru frame_control;
    osal_u16                      duration_value  : 15,           /* duration/id */
                                  duration_flag   : 1;
    osal_u8                       address1[WLAN_MAC_ADDR_LEN];
    osal_u8                       address2[WLAN_MAC_ADDR_LEN];
    osal_u8                       address3[WLAN_MAC_ADDR_LEN];
    osal_u16                      frag_num        : 4,                /* sequence control */
                                  seq_num         : 12;
    osal_u8                       address4[WLAN_MAC_ADDR_LEN];
    osal_u8                       qc_tid          : 4,
                                  qc_eosp         : 1,
                                  qc_ack_polocy   : 2,
                                  qc_amsdu        : 1;
    union {
        osal_u8                   qc_txop_limit;                      /* txop limit字段 */
        osal_u8                   qc_queue_size;                      /* queue size字段 */
        osal_u8                   qc_ps_buf_state_resv        : 1,    /* AP PS Buffer State */
                                  qc_ps_buf_state_inducated   : 1,
                                  qc_hi_priority_buf_ac       : 2,
                                  qc_qosap_buf_load           : 4;
    } qos_control;
};
typedef struct mac_ieee80211_qos_frame_addr4 mac_ieee80211_qos_frame_addr4_stru;

/* qos htc 四地址帧结构 */
struct mac_ieee80211_qos_htc_frame_addr4 {
    mac_ieee80211_qos_frame_addr4_stru qos_frame_addr4;
    osal_u32                      htc;
};
typedef struct mac_ieee80211_qos_htc_frame_addr4 mac_ieee80211_qos_htc_frame_addr4_stru;

/* Ref. 802.11-2012.pdf, 8.4.1.4 Capability information field, 中文注释参考白皮书 */
struct mac_cap_info {
    osal_u16  ess                 : 1,        /* 由BSS中的AP设置为1 */
              ibss                : 1,        /* 由一个IBSS中的站点设置为1，ap总是设置其为0 */
              cf_pollable         : 1,        /* 标识CF-POLL能力 */
              cf_poll_request     : 1,        /* 标识CF-POLL能力  */
              privacy             : 1,        /* 1=需要加密, 0=不需要加密 */
              short_preamble      : 1,        /* 802.11b短前导码 */
              pbcc                : 1,        /* 802.11g */
              channel_agility     : 1,        /* 802.11b */
              spectrum_mgmt       : 1,        /* 频谱管理: 0=不支持, 1=支持 */
              qos                 : 1,        /* QOS: 0=非QOS站点, 1=QOS站点 */
              short_slot_time     : 1,        /* 短时隙: 0=不支持, 1=支持 */
              apsd                : 1,        /* 自动节能: 0=不支持, 1=支持 */
              radio_measurement   : 1,        /* Radio检测: 0=不支持, 1=支持 */
              dsss_ofdm           : 1,        /* 802.11g */
              delayed_block_ack   : 1,        /* 延迟块确认: 0=不支持, 1=支持 */
              immediate_block_ack : 1;        /* 立即块确认: 0=不支持, 1=支持 */
};
typedef struct mac_cap_info mac_cap_info_stru;

/* Ref. 802.11-2012.pdf, 8.4.2.58.2 HT Capabilities Info field */
struct mac_frame_ht_cap {
    osal_u16 ldpc_coding_cap         : 1, /* LDPC 编码 capability   */
             supported_channel_width : 1, /* STA 支持的带宽      */
             sm_power_save           : 2, /* SM 省电模式     */
             ht_green_field          : 1, /* 绿野模式     */
             short_gi_20mhz          : 1, /* 20M下短保护间隔       */
             short_gi_40mhz          : 1, /* 40M下短保护间隔       */
             tx_stbc                 : 1, /* Indicates support for the transmission of PPDUs using STBC */
             rx_stbc                 : 2, /* 支持 Rx STBC   */
             ht_delayed_block_ack    : 1, /* Indicates support for HT-delayed Block Ack opera-tion. */
             max_amsdu_length        : 1, /* Indicates maximum A-MSDU length. */
             dsss_cck_mode_40mhz     : 1, /* 40M下 DSSS/CCK 模式    */
             bit_resv                : 1,
             forty_mhz_intolerant    : 1, /* 接收此信息的AP是否需要禁止40MHz传输 */
             lsig_txop_protection    : 1; /* 支持 L-SIG TXOP 保护    */
};
typedef struct mac_frame_ht_cap mac_frame_ht_cap_stru;
#pragma pack(pop)

struct mac_vht_cap_info {
    osal_u32  bit_max_mpdu_length     : 2,
              supported_channel_width : 2,
              rx_ldpc                 : 1,
              short_gi_80mhz          : 1,
              short_gi_160mhz         : 1,
              tx_stbc                 : 1,
              rx_stbc                 : 3,
              su_beamformer_cap       : 1,
              su_beamformee_cap       : 1,
              num_bf_ant_supported    : 3,
              num_sounding_dim        : 3,
              mu_beamformer_cap       : 1,
              mu_beamformee_cap       : 1,
              vht_txop_ps             : 1,
              htc_vht_capable         : 1,
              max_ampdu_len_exp       : 3,
              vht_link_adaptation     : 2,
              rx_ant_pattern          : 1,
              tx_ant_pattern          : 1,
              bit_resv                    : 2;
};
typedef struct mac_vht_cap_info mac_vht_cap_info_stru;

#pragma pack(push, 1)
#ifdef _PRE_WLAN_FEATURE_11AX
struct mac_frame_he_trig {
    /* byte 0 */
    osal_u16                   trig_type            : 4;
    osal_u16                   ppdu_len             : 12;

    osal_u32                   cascade              : 1;  /* 当前skb是否为amsdu的首个skb */
    osal_u32                   cs_required          : 1;
    osal_u32                   bandwith             : 2;
    osal_u32                   ltf_gi               : 2;
    osal_u32                   mu_mimo_ltf_mode     : 1;
    osal_u32                   num_of_ltfs          : 3;
    osal_u32                   stbc                 : 1;
    osal_u32                   ldpc_extra_symbol    : 1;
    osal_u32                   ap_tx_pwr            : 6;
    osal_u32                   pe_fec_pading        : 2;
    osal_u32                   pe_disambiguity      : 1;

    osal_u32                   spatial_reuse        : 16;
    osal_u32                   dopler               : 1;
    osal_u32                   he_siga              : 9;
    osal_u32                   bit_resv             : 1;
};
typedef struct mac_frame_he_trig  mac_frame_he_trig_stru;

struct mac_frame_trig_depend_user_info {
    osal_u8                    mu_spacing_factor         : 2;
    osal_u8                    multi_tid_aggr_limit      : 3;
    osal_u8                    bit_resv                      : 1;
    osal_u8                    perfer_ac                 : 2;
};
typedef struct mac_frame_trig_depend_user_info  mac_frame_trig_depend_user_info_stru;

struct mac_frame_he_trig_user_info {
    /* byte 0 */
    osal_u32                   user_aid                  : 12;
    osal_u32                   ru_location               : 8;
    osal_u32                   coding_type               : 1;
    osal_u32                   mcs                       : 4;
    osal_u32                   dcm                       : 1;
    osal_u32                   starting_spatial_stream   : 3;
    osal_u32                   nss                       : 3;

    osal_u8                    target_rssi               : 7;
    osal_u8                    bit_resv                      : 1;

    mac_frame_trig_depend_user_info_stru    user_info;
};
typedef struct mac_frame_he_trig_user_info  mac_frame_he_trig_user_info_stru;
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */

struct hmac_color_event_rpt_elements {
    osal_u8 eid;
    osal_u8 length;
    osal_u8 event_token;
    osal_u8 event_type;
    osal_u8 event_report_status;
    osal_u64 event_tsf;
    osal_u64 event_report;
};
typedef struct hmac_color_event_rpt_elements hmac_color_event_rpt_elements_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
/* HE CAP:PPE Thresholds */
struct mac_frame_ppe_thresholds_pre_field {
    osal_u16          nss                                         : 3,
                      ru_index0_mask                              : 1,
                      ru_index1_mask                              : 1,
                      ru_index2_mask                              : 1,
                      ru_index3_mask                              : 1,
                      ppet16_nss1_ru0                             : 3,
                      ppet8_nss1_ru0                              : 3,
                      ppet16_nss1_ru1                             : 3;

    osal_u32          ppet8_nss1_ru1                              : 3,
                      ppet16_nss1_ru2                             : 3,
                      ppet8_nss1_ru2                              : 3,
                      ppet16_nss2_ru0                             : 3,
                      ppet8_nss2_ru0                              : 3,
                      ppet16_nss2_ru1                             : 3,
                      ppet8_nss2_ru1                              : 3,
                      ppet16_nss2_ru2                             : 3,
                      ppet8_nss2_ru2                              : 3,
                      bit_rsv                                     : 5;
};
typedef struct mac_frame_ppe_thresholds_pre_field mac_frame_ppe_thresholds_pre_field_stru;

struct mac_frame_he_mcs_nss {
    osal_u8          mcs                                          : 4,
                     nss                                          : 3,
                     last_mcs_nss                                 : 1;
};
typedef struct mac_frame_he_mcs_nss mac_frame_he_mcs_nss_stru;

/* he mu edca param */
struct mac_frame_he_mu_qos_info {
    osal_u8           edca_update_count                                  : 4,
                      q_ack                                              : 1,
                      queue_request                                      : 1,
                      txop_request                                       : 1,
                      more_data_ack                                      : 1;
};
typedef struct mac_frame_he_mu_qos_info mac_frame_he_mu_qos_info_stru;

struct mac_frame_he_mu_ac_parameter {
    osal_u8           aifsn                                              : 4,
                      acm                                                : 1,
                      ac_index                                           : 2,
                      reserv                                             : 1;
    osal_u8           ecw_min                                            : 4,
                      ecw_max                                            : 4;
    osal_u8           mu_edca_timer; /* 单位 8TU */
};
typedef struct mac_frame_he_mu_ac_parameter mac_frame_he_mu_ac_parameter_stru;

struct mac_frame_he_mu_edca_parameter_ie {
    mac_frame_he_mu_qos_info_stru           qos_info;
    mac_frame_he_mu_ac_parameter_stru       mu_ac_parameter[WLAN_WME_AC_BUTT];
};
typedef struct mac_frame_he_mu_edca_parameter_ie mac_frame_he_mu_edca_parameter_ie_stru;
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */

struct mac_frame_he_sr_control {
    osal_u8           srp_disallowed                                     : 1;
    osal_u8           non_srg_obss_pd_sr_disallowed                      : 1;
    osal_u8           non_srg_offset_present                             : 1;
    osal_u8           srg_information_present                            : 1;
    osal_u8           hesiga_spatial_reuse_value15_allowed               : 1;
    osal_u8           reserved                                           : 3;
};
typedef struct mac_frame_he_sr_control mac_frame_he_sr_control_stru;

struct mac_frame_he_spatial_reuse_parameter_set_ie {
    mac_frame_he_sr_control_stru     sr_control;
    osal_u8                          non_srg_boss_pd_offset_max;
    osal_u8                          srg_obss_pd_offset_min;
    osal_u8                          srg_obss_pd_offset_max;
    osal_u8                          srg_bss_color_bitmap_info[8];
    osal_u8                          ac_srg_partial_bssid_bitmap[8];
};
typedef struct mac_frame_he_spatial_reuse_parameter_set_ie mac_frame_he_spatial_reuse_parameter_set_ie_stru;

struct mac_frame_bss_color_change_annoncement_ie {
    osal_u8                          color_switch_countdown;
    osal_u8                          new_bss_color                       : 6,
                                     reserved                            : 2;
};
typedef struct mac_frame_bss_color_change_annoncement_ie mac_frame_bss_color_change_annoncement_ie_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
/* OCW Range字段一定存在 字节数为1 */
struct mac_frame_ocw_range {
    osal_u8                              eocw_min                        : 3,
                                         eocw_max                        : 3,
                                         reserved                        : 2;
};
typedef struct mac_frame_ocw_range mac_frame_ocw_range_stru;

/* UORA Parameter Set element: 固定长度部分 */
struct mac_frame_uora_parameter_set_ie {
    mac_frame_ocw_range_stru  ocw_range;
};
typedef struct mac_frame_uora_parameter_set_ie mac_frame_uora_parameter_set_ie_stru;

/* Ess Information字段一定存在 字节数为1 */
struct mac_frame_he_ess_report {
    osal_u8               planned_ess                                             : 1,
                          edge_of_ess                                             : 1,
                          recommended_bss_transition_rssi_threshold_within_ess    : 6;
};
typedef struct mac_frame_he_ess_report mac_frame_he_ess_report_stru;

/* ESS Report Element: 固定长度部分 */
struct mac_ess_report_ie {
    mac_frame_he_ess_report_stru      ess_information;
};
typedef struct mac_ess_report_ie mac_ess_report_ie_stru;

/* NDP Feedback Report Parameter Set Element: 固定长度部分 */
struct mac_ndp_feedback_report_parameter_set_ie {
    osal_u8                           resource_request_buffer_threshold_exponent;
};
typedef struct mac_ndp_feedback_report_parameter_set_ie mac_ndp_feedback_report_parameter_set_ie_stru;

/* HE BSS Load Element: 固定长度部分 */
struct mac_he_bss_load_ie {
    osal_u16                          he_sta_count;
    osal_u8                           utilization;
    osal_u8                           frequency_underutilization;
    osal_u8                           spatial_stream_underutilization;
};
typedef struct mac_he_bss_load_ie mac_he_bss_load_ie_stru;

/* Capabilities Information字段一定存在 字节数为2 */
struct mac_frame_capabilities_information {
    osal_u8               minimum_mpdu_start_spacing                              : 3,
                          maximum_a_mpdu_length_exponent                          : 3,
                          maximum_mpdu_length                                     : 2;
    osal_u8               reserved_1                                              : 1,
                          sm_power_save                                           : 2,
                          rd_responder                                            : 1,
                          rx_antenna_pattern_consistency                          : 1,
                          tx_antenna_pattern_consistency                          : 1,
                          reserved_2                                              : 2;
};
typedef struct mac_frame_capabilities_information mac_frame_capabilities_information_stru;

/* HE 6GHz Band Capabilities Element: 固定长度部分 */
struct mac_he_6ghz_band_capabilities_ie {
    mac_frame_capabilities_information_stru     capabilities_information;
};
typedef struct mac_he_6ghz_band_capabilities_ie mac_he_6ghz_band_capabilities_ie_stru;

/* Multiple BSSID Configuration element 固定长度部分 */
struct mac_frame_multiple_bssid_configuration {
    osal_u8                        bssid_count;
    osal_u8                        profile_periodicity;
};
typedef struct mac_frame_multiple_bssid_configuration mac_frame_multiple_bssid_configuration_stru;
#endif /* end _PRE_WLAN_FEATURE_11AX */

struct mac_11ntxbf_info {
    osal_u8       txbf_11n                : 1,         /* 11n txbf  能力 */
                  reserve1                : 7;
    osal_u8       reserve2[3];
};
typedef struct mac_11ntxbf_info mac_11ntxbf_info_stru;

struct mac_11ntxbf_vendor_ie {
    osal_u8                        id;          /* element ID */
    osal_u8                        len;         /* length in bytes */
    osal_u8                        oui[3];
    osal_u8                        ouitype;
    mac_11ntxbf_info_stru          txbf_11n_info;
};
typedef struct mac_11ntxbf_vendor_ie mac_11ntxbf_vendor_ie_stru;

/* 厂家自定义IE 数据结构，摘自linux 内核 */
struct mac_ieee80211_vendor_ie {
    osal_u8 element_id;
    osal_u8 len;
    osal_u8 oui[3];
    osal_u8 oui_type;
};
typedef struct mac_ieee80211_vendor_ie mac_ieee80211_vendor_ie_stru;

typedef struct mac_hiex_cap {
    osal_u32 bit_hiex_cap : 1;
    osal_u32 bit_hiex_version : 3;
    osal_u32 bit_himit_enable : 1;
    osal_u32 bit_ht_himit_enable : 1;
    osal_u32 bit_vht_himit_enable : 1;
    osal_u32 bit_he_himit_enable : 1;
    osal_u32 bit_he_htc_himit_id : 4;
    osal_u32 bit_chip_type : 4;
    osal_u32 bit_himit_version : 2;
    osal_u32 bit_ersru_enable : 1; // 动态窄带2.0使能开关
    osal_u32 bit_resv : 13;
} mac_hiex_cap_stru;

typedef struct mac_priv_cap {
    osal_u32 bit_11ax_support : 1; // 是否支持11AX，此bit位的位置不能改变
    osal_u32 bit_dcm_support : 1; // 是否支持dcm，此bit位的位置不能改变
    osal_u32 bit_p2p_csa_support : 1; // 是否支持p2p_csa，此bit位的位置不能改变
    osal_u32 bit_p2p_scenes : 1; // 是否支持p2p_scenes，此bit位的位置不能改变
    osal_u32 bit_1024qam_cap : 1; // 是否支持1024QAM
    osal_u32 bit_4096qam_cap : 1; // 是否支持4096QAM
    osal_u32 bit_sgi_400ns_cap : 1; // 是否支持SGI_400NS
    osal_u32 bit_vht_3nss_80m_mcs6 : 1; // 是否支持VHT_3NSS_80M_MCS6
    osal_u32 bit_he_ltf : 1;      // 是否支持HE LTF
    osal_u32 bit_sub_wf_cap : 1; // 是否支持子带注水
    osal_u32 bit_vht_3nss_160m_mcs9 : 1; // 是否支持VHT_3NSS_160M_MCS9
    osal_u32 bit_ersru_he_ltf_num : 4; // 动态窄带2.0，STA响应TB帧发送HE LTF最大个数
    osal_u32 bit_11b_plus_cap : 3; // 是否支持11b+速率
    osal_u32 bit_resv : 14;
} mac_priv_cap_stru;

struct mac_hiex_vendor_ie {
    mac_ieee80211_vendor_ie_stru vender;
    mac_hiex_cap_stru hiex_cap;
};
typedef struct mac_hiex_vendor_ie mac_hiex_vendor_ie_stru;

struct mac_priv_vendor_ie {
    mac_ieee80211_vendor_ie_stru vender;
    mac_priv_cap_stru priv_cap;
};
typedef struct mac_priv_vendor_ie mac_priv_vendor_ie_stru;

/* 建立BA会话时，BA参数域结构定义 */
struct mac_ba_parameterset {
    osal_u16    amsdusupported      : 1,  /* B0   amsdu supported */
                bapolicy            : 1,  /* B1   block ack policy */
                tid                 : 4,  /* B2-5   TID */
                buffersize          : 10; /* B6-15  buffer size */
};
typedef struct mac_ba_parameterset mac_ba_parameterset_stru;

/* BA会话过程中的序列号参数域定义 */
struct mac_ba_seqctrl {
    osal_u16  fragnum         : 4,            /* B0-3  fragment number */
              startseqnum     : 12;           /* B4-15  starting sequence number */
};
typedef struct mac_ba_seqctrl mac_ba_seqctrl_stru;

/* Quiet信息元素结构体 */
struct mac_quiet_ie {
    osal_u8     quiet_count;
    osal_u8     quiet_period;
    osal_u16    quiet_duration;
    osal_u16    quiet_offset;
};
typedef struct mac_quiet_ie mac_quiet_ie_stru;

/* erp 信息元素结构体 */
struct mac_erp_params {
    osal_u8   non_erp        : 1,
              use_protection : 1,
              preamble_mode  : 1,
              bit_resv           : 5;
};
typedef struct mac_erp_params mac_erp_params_stru;

/* rsn信息元素 rsn能力字段结构体 */
struct mac_rsn_cap {
    osal_u16  pre_auth            : 1,
              no_pairwise         : 1,
              ptska_relay_counter : 2,
              gtska_relay_counter : 2,
              mfpr                : 1,
              mfpc                : 1,
              rsv0                : 1,
              peer_key            : 1,
              spp_amsdu_capable   : 1,
              spp_amsdu_required  : 1,
              pbac                : 1,
              ext_key_id          : 1,
              rsv1                : 2;
};
typedef struct mac_rsn_cap mac_rsn_cap_stru;

/* obss扫描ie obss扫描参数结构体 */
struct mac_obss_scan_params {
    osal_u16 passive_dwell;
    osal_u16 active_dwell;
    osal_u16 scan_interval;
    osal_u16 passive_total_per_chan;
    osal_u16 active_total_per_chan;
    osal_u16 transition_delay_factor;
    osal_u16 scan_activity_thresh;
};
typedef struct mac_obss_scan_params mac_obss_scan_params_stru;

#ifdef _PRE_WLAN_FEATURE_WUR_TX
struct mac_wur_param_control_field {
    osal_u8 wur_duty_cycle_start_time_present : 1,
            wur_group_id_list_present : 1,
            proposed_wur_param_present : 1,
            rsvd : 5;
};
typedef struct mac_wur_param_control_field mac_wur_param_control_field_stru;

struct mac_wur_param_ap_field {
    osal_u16 wur_id : 12, wur_channel_offset : 3, rsvd : 1;
    osal_u64 wur_duty_cycle_start_time;
    /* WUR Group ID List */
};
typedef struct mac_wur_param_ap_field mac_wur_param_ap_field_stru;

struct mac_proposed_wur_param_field {
    osal_u8 recommend_wur_wakeup_frame_rate : 2,
            recommend_wur_channel_offset : 3,
            requested_keep_alive_wur_farme : 1,
            rsvd : 2;
};
typedef struct mac_proposed_wur_param_field mac_proposed_wur_param_field_stru;

struct mac_wur_param_sta_field {
    osal_u32 wur_duty_cycle_serive_period;
    osal_u16 duty_cycle_period;
    mac_proposed_wur_param_field_stru proposed_wur_param_field;
};
typedef struct mac_wur_param_sta_field mac_wur_param_sta_field_stru;

/* 新增结构体，用以表示WUR Mode Element */
struct mac_wur_mode_ie {
    osal_u8 element_id;
    osal_u8 len;
    osal_u8 element_id_ext;
    osal_u8 action_type;
    osal_u8 wur_mode_rsp_status;
    mac_wur_param_control_field_stru wur_param_control_field;
};
typedef struct mac_wur_mode_ie mac_wur_mode_ie_stru;

/* WUR Mode Setup帧信息 */
struct mac_wur_mode_setup_frame {
    osal_u8 category;
    osal_u8 wur_action;
    osal_u8 dialog_token;
    mac_wur_mode_ie_stru wur_mode_element;
};
typedef struct mac_wur_mode_setup_frame mac_wur_mode_setup_frame_stru;

/* WUR Mode Teardown帧信息 */
struct mac_wur_mode_teardown_frame {
    osal_u8 category;
    osal_u8 wur_action;
};
typedef struct mac_wur_mode_teardown_frame mac_wur_mode_teardown_frame_stru;
#endif

/* qos info字段结构体定义 */
struct mac_qos_info {
    osal_u8   params_count : 4,
              bit_resv         : 3,
              uapsd        : 1;
};
typedef struct mac_qos_info mac_qos_info_stru;
#pragma pack(pop)

/* wmm信息元素 ac参数结构体 */
typedef struct {
    osal_u8   aifsn  : 4,
              acm    : 1,
              aci    : 2,
              bit_resv   : 1;
    osal_u8   ecwmin : 4,
              ecwmax : 4;
    osal_u16  txop;
} mac_wmm_ac_params_stru;

#pragma pack(push, 1)
/* BSS load信息元素结构体 */
struct mac_bss_load {
    osal_u16 sta_count;            /* 关联的sta个数 */
    osal_u8  chan_utilization;     /* 信道利用率 */
    osal_u16 aac;
};
typedef struct mac_bss_load mac_bss_load_stru;

/* country信息元素 管制域字段 */
struct mac_country_reg_field {
    osal_u8 first_channel;         /* 第一个信道号 */
    osal_u8 channel_num;           /* 信道个数 */
    osal_u16 max_tx_pwr;            /* 最大传输功率，dBm */
};
typedef struct mac_country_reg_field mac_country_reg_field_stru;

/* ht capabilities信息元素支持的ampdu parameters字段结构体定义 */
struct mac_ampdu_params {
    osal_u8  max_ampdu_len_exponent  : 2,
             min_mpdu_start_spacing  : 3,
             bit_resv                    : 3;
};
typedef struct mac_ampdu_params mac_ampdu_params_stru;

/* ht cap信息元素 支持的mcs集字段 结构体定义 */
struct mac_sup_mcs_set {
    osal_u8   rx_mcs[10];
    osal_u16  rx_highest_rate : 10,
              resv1           : 6;
    osal_u32  tx_mcs_set_def  : 1,
              tx_rx_not_equal : 1,
              tx_max_stream   : 2,
              tx_unequal_modu : 1,
              resv2           : 27;
};
typedef struct mac_sup_mcs_set mac_sup_mcs_set_stru;
#pragma pack(pop)

/* vht信息元素，支持的mcs集字段 */
typedef struct {
    osal_u32  rx_mcs_map      : 16,
              rx_highest_rate : 13,
              bit_resv        : 3;
    osal_u32  tx_mcs_map      : 16,
              tx_highest_rate : 13,
              resv2           : 3;
} mac_vht_sup_mcs_set_stru;

#pragma pack(push, 1)
/* ht capabilities信息元素支持的extended cap.字段结构体定义 */
struct mac_ext_cap {
    osal_u16  pco            : 1,
              pco_trans_time : 2,
              resv1          : 5,
              mcs_fdbk       : 2,
              htc_sup        : 1,
              rd_resp        : 1,
              resv2          : 4;
};
typedef struct mac_ext_cap mac_ext_cap_stru;
#pragma pack(pop)

/* ht cap信息元素的Transmit Beamforming Capabilities字段结构体定义 */
typedef struct {
    osal_u32  implicit_txbf_rx                 : 1,
              rx_stagg_sounding                : 1,
              tx_stagg_sounding                : 1,
              rx_ndp                           : 1,
              tx_ndp                           : 1,
              implicit_txbf                    : 1,
              calibration                      : 2,
              explicit_csi_txbf                : 1,
              explicit_noncompr_steering       : 1,
              explicit_compr_steering          : 1,
              explicit_txbf_csi_fdbk           : 2,
              explicit_noncompr_bf_fdbk        : 2,
              explicit_compr_bf_fdbk           : 2,
              minimal_grouping                 : 2,
              csi_num_bf_antssup               : 2,
              noncompr_steering_num_bf_antssup : 2,
              compr_steering_num_bf_antssup    : 2,
              csi_maxnum_rows_bf_sup           : 2,
              chan_estimation                  : 2,
              bit_resv                             : 3;
} mac_txbf_cap_stru;

#pragma pack(push, 1)
/* ht cap信息元素的Asel(antenna selection) Capabilities字段结构体定义 */
struct mac_asel_cap {
    osal_u8  asel                         : 1,
             explicit_sci_fdbk_tx_asel    : 1,
             antenna_indices_fdbk_tx_asel : 1,
             explicit_csi_fdbk            : 1,
             antenna_indices_fdbk         : 1,
             rx_asel                      : 1,
             trans_sounding_ppdu          : 1,
             bit_resv                         : 1;
};
typedef struct mac_asel_cap mac_asel_cap_stru;

/* ht opern元素, ref 802.11-2012 8.4.2.59 */
struct mac_ht_opern {
    osal_u8   primary_channel;

    osal_u8   secondary_chan_offset             : 2,
              sta_chan_width                    : 1,
              rifs_mode                         : 1,
              resv1                             : 4;
    osal_u8   ht_protection                     : 2,
              nongf_sta_present                 : 1,
              resv2                             : 1,
              obss_nonht_sta_present            : 1,
              resv3                             : 3;
    osal_u8   resv4                             : 8;
    osal_u8   resv5                             : 6,
              dual_beacon                       : 1,
              dual_cts_protection               : 1;
    osal_u8   secondary_beacon                  : 1,
              lsig_txop_protection_full_support : 1,
              pco_active                        : 1,
              pco_phase                         : 1,
              resv6                             : 4;

    osal_u8   basic_mcs_set[MAC_HT_BASIC_MCS_SET_LEN];
};
typedef struct mac_ht_opern mac_ht_opern_stru;

/* vht opern结构体 */
struct mac_opmode_notify {
    osal_u8   channel_width   : 2,     /* 当前最大允许带宽能力 */
              bit_resv            : 2,     /* Dynamic Extended NSS BW,VHT下set 0 */
              rx_nss          : 3,     /* 当前最大允许空间流能力 */
              rx_nss_type     : 1;     /* 是否为TXBF下的rx nss能力，1-是，0-不是 */
};
typedef struct mac_opmode_notify mac_opmode_notify_stru;

/* vht opern结构体 */
struct mac_vht_opern {
    osal_u8   channel_width;
    osal_u8   channel_center_freq_seg0;
    osal_u8   channel_center_freq_seg1;
    osal_u16  basic_mcs_set;
};
typedef struct mac_vht_opern mac_vht_opern_stru;

#if defined(_PRE_WLAN_FEATURE_FTM)
typedef struct mac_quiet_frm {
    osal_u8   eid;
    osal_u8   len;
    osal_u8   quiet_count;
    osal_u8   quiet_period;
    osal_u16  quiet_duration;
    osal_u16  quiet_offset;
} mac_quiet_frm_stru;

typedef struct mac_pwr_constraint_frm {
    osal_u8   eid;
    osal_u8   len;
    osal_u8   local_pwr_constraint;
} mac_pwr_constraint_frm_stru;

/* Frame request */
typedef struct mac_frm_req {
    osal_u8                           optclass;
    osal_u8                           channum;         /*  请求测量的信道号 */
    osal_u16                          random_ivl;
    osal_u16                          duration;
    osal_u8                           frm_req_type;
    osal_u8                           auc_mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8                           subelm[1];     /* varialbe len sub element fileds */
} mac_frm_req_stru;

/* Frame Report */
typedef struct mac_frm_cnt_rpt {
    osal_u8    tx_addr[WLAN_MAC_ADDR_LEN];
    osal_u8    bssid[WLAN_MAC_ADDR_LEN];
    osal_u8    phy_type;
    osal_u8    avrg_rcpi;
    osal_u8    last_rsni;
    osal_u8    last_rcpi;
    osal_u8    ant_id;
    osal_u16   frm_cnt;
} mac_frm_cnt_rpt_stru;

typedef struct mac_frm_rpt {
    osal_u8    optclass;
    osal_u8    channum;
    osal_u8    act_meas_start_time[MAC_RADIO_MEAS_START_TIME_LEN];
    osal_u16   duration;
    osal_u8    subelm[1];     /* varialbe len sub element fileds */
} mac_frm_rpt_stru;

/* Statistic Report basic */
typedef struct mac_stats_rpt_basic {
    osal_u16   duration;
    osal_u8    group_id;
    osal_u8    group[1];
} mac_stats_rpt_basic_stru;

/* Statistic Report Group0, dot11Counters Group */
typedef struct mac_stats_cnt_rpt {
    osal_u32    tx_frag_cnt;
    osal_u32    multi_tx_cnt;
    osal_u32    fail_cnt;
    osal_u32    rx_frag_cnt;
    osal_u32    multi_rx_cnt;
    osal_u32    fcs_err_cnt;
    osal_u32    tx_cnt;
} mac_stats_cnt_rpt_stru;

/* Statistic Report Group1, dot11MACStatistics Group */
typedef struct mac_stats_mac_rpt {
    osal_u32    retry_cnt;
    osal_u32    multi_retry_cnt;
    osal_u32    dup_cnt;
    osal_u32    rts_succ_cnt;
    osal_u32    trs_fail_cnt;
    osal_u32    ack_fail_cnt;
} mac_stats_mac_rpt_stru;

/* Statistic Report Group2~9 */
typedef struct mac_stats_up_cnt_rpt {
    osal_u32    tx_frag_cnt;
    osal_u32    fail_cnt;
    osal_u32    retry_cnt;
    osal_u32    multi_retry_cnt;
    osal_u32    dup_cnt;
    osal_u32    rts_succ_cnt;
    osal_u32    trs_fail_cnt;
    osal_u32    ack_fail_cnt;
    osal_u32    rx_frag_cnt;
    osal_u32    tx_cnt;
    osal_u32    discard_cnt;
    osal_u32    rx_cnt;
    osal_u32    rx_retry_cnt;
} mac_stats_up_cnt_rpt_stru;

/* TSC Report */
typedef struct mac_tsc_rpt {
    osal_u8    act_meas_start_time[MAC_RADIO_MEAS_START_TIME_LEN];
    osal_u16   duration;
    osal_u8    peer_sta_addr[WLAN_MAC_ADDR_LEN];
    osal_u8    tid;
    osal_u8    rpt_reason;

    osal_u32    tx_frag_cnt;
    osal_u32    fail_cnt;
    osal_u32    retry_cnt;
    osal_u32    multi_retry_cnt;
} mac_tsc_rpt_stru;

/* Statistic Report Group11 */
typedef struct mac_stats_access_delay_rpt {
    osal_u32    avrg_access_delay;
    osal_u32    aul_avrg_access_delay[4];
    osal_u32    sta_cnt;
    osal_u32    channel_utilz;
} mac_stats_access_delay_rpt_stru;

typedef struct mac_ftm_range_req_ie {
    osal_u16                          randomization_interval;
    osal_u8                           minimum_ap_count;
    osal_u8                           ftm_range_subelements[1];
} mac_ftm_range_req_ie_stru;

/* Fine Timing Measurement Parameters */
struct mac_ftm_parameters_ie {
    osal_u8               eid;         /* IEEE80211_ELEMID_MEASRPT */
    osal_u8               len;
    osal_u8               status_indication                         : 2,
                          value                                     : 5,
                          reserved0                                 : 1;
    osal_u8               number_of_bursts_exponent                 : 4,
                          burst_duration                            : 4;
    osal_u8               min_delta_ftm;
    osal_u16              partial_tsf_timer;
    osal_u8               partial_tsf_timer_no_preference           : 1,
                          asap_capable                              : 1,
                          asap                                      : 1,
                          ftms_per_burst                            : 5;
    osal_u8               reserved                                  : 2,
                          format_and_bandwidth                      : 6;
    osal_u16              burst_period;
};
typedef struct mac_ftm_parameters_ie mac_ftm_parameters_ie_stru;
#endif
#pragma pack(pop)

/* ACTION帧的参数格式，注:不同的action帧下对应的参数不同 */
typedef struct {
    osal_u8       category;    /* ACTION的类别 */
    osal_u8       action;      /* 不同ACTION类别下的分类 */
    osal_u8       resv[2];
    osal_u32      arg1;
    osal_u32      arg2;
    osal_u32      arg3;
    osal_u32      arg4;
    osal_u8      *arg5;
} mac_action_mgmt_args_stru;

#pragma pack(push, 1)
struct mac_ts_info {
    osal_u16  traffic_type : 1,
              tsid         : 4,
              direction    : 2,
              acc_policy   : 2,
              aggr         : 1,
              apsd         : 1,
              user_prio    : 3,
              ack_policy   : 2;
    osal_u8   schedule     : 1,
              rsvd         : 7;
};
typedef struct mac_ts_info mac_ts_info_stru;

#ifdef _PRE_WLAN_FEATURE_WMMAC
struct mac_wmm_tspec {
    mac_ts_info_stru  ts_info;
    osal_u16        norminal_msdu_size;
    osal_u16        max_msdu_size;
    osal_u32        min_srv_interval;
    osal_u32        max_srv_interval;
    osal_u32        inactivity_interval;
    osal_u32        suspension_interval;
    osal_u32        srv_start_time;
    osal_u32        min_data_rate;
    osal_u32        mean_data_rate;
    osal_u32        peak_data_rate;
    osal_u32        max_burst_size;
    osal_u32        delay_bound;
    osal_u32        min_phy_rate;
    osal_u16        surplus_bw;
    osal_u16        medium_time;
};
typedef struct mac_wmm_tspec mac_wmm_tspec_stru;
#endif
#pragma pack(pop)

/* SMPS节能控制action帧体 */
typedef struct {
    osal_u8    category;
    osal_u8    action;
    osal_u8    sm_ctl;
    osal_u8    arg;
} mac_smps_action_mgt_stru;

/* operating mode notify控制action帧体 */
typedef struct {
    osal_u8    category;
    osal_u8    action;
    osal_u8    opmode_ctl;
    osal_u8    arg;
} mac_opmode_notify_action_mgt_stru;

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
typedef struct _mac_action_data_stru {
    td_u32 freq;
    td_u32 wait;
    td_u8 dst[WLAN_MAC_ADDR_LEN];
    td_u8 src[WLAN_MAC_ADDR_LEN];
    td_u8 bssid[WLAN_MAC_ADDR_LEN];
    td_u8 resv[2]; /* 2 byte保留字段 */
    td_u8 *data;
    td_u32 data_len;
    td_u32 no_cck;
} mac_action_data_stru;
#endif

#pragma pack(push, 1)
struct mac_assoc_retry_delay_attr_mbo_ie {
    osal_u8 element_id;
    osal_u8 len;
    osal_u8 oui[3];
    osal_u8 oui_type;
    osal_u8 attribute_id;
    osal_u8 attribute_len;
    osal_u32 reassoc_delay_time;
};
typedef struct mac_assoc_retry_delay_attr_mbo_ie mac_assoc_retry_delay_attr_mbo_ie_stru;

/* Beacon Report SubElement: Reported Frame Body Fgrament ID */
struct mac_reported_frame_body_fragment_id {
    osal_u8 sub_eid;
    osal_u8 len;
    osal_u8 bit_beacon_report_id; /* beacon report编号 */
    osal_u8 bit_fragment_id_number : 7; /* beacon report分段编号 */
    osal_u8 bit_more_frame_body_fragments : 1; /* 当前beacon report后续是否还有更多分段，[1:有|0:没有] */
};
typedef struct mac_reported_frame_body_fragment_id mac_reported_frame_body_fragment_id_stru;

/* Beacon Report SubElement: Last Beacon Report Indication */
struct mac_last_beacon_report_indication {
    osal_u8 sub_eid;
    osal_u8 len;
    osal_u8 last_beacon_report_indication; /* 是否是最后一个beacon report，[1:是|0:不是] */
};
typedef struct mac_last_beacon_report_indication mac_last_beacon_report_indication_stru;
#pragma pack(pop)

typedef osal_void (*set_ext_cap_ie)(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_ht_cap_ie)(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_ht_opern_ie)(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_rsn_ie)(osal_void *vap, osal_u8 *pmkid, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_vht_cap_ie)(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_vht_opern_ie)(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_wpa_ie)(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_nb_ie)(osal_u8 *buffer, osal_u8 *ie_len);
typedef osal_void (*set_vht_capinfo_field)(osal_void *vap, osal_u8 *buffer);

typedef osal_u32 (*hmac_set_he_capabilities_ie_resv)(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);

typedef struct {
    set_ext_cap_ie   set_ext_cap_ie_cb;
    set_ht_cap_ie    set_ht_cap_ie_cb;
    set_ht_opern_ie  set_ht_opern_ie_cb;
    set_rsn_ie       set_rsn_ie_cb;
    set_vht_cap_ie   set_vht_cap_ie_cb;
    set_vht_opern_ie set_vht_opern_ie_cb;
    set_wpa_ie       set_wpa_ie_cb;
    set_nb_ie        set_nb_ie_cb;
    set_vht_capinfo_field  set_vht_capinfo_field_cb;
} mac_frame_cb;

typedef struct {
    osal_u16 duration;
    osal_u16 offset;
    osal_u8 count;
    osal_u8 period;
    osal_u8 resv[2];
} mac_quiet_info;

#ifdef _PRE_WLAN_FEATURE_WUR_TX
/* Supported Bands域信息 */
typedef struct {
    osal_u8 support_2g : 1, support_5g : 1, rsvd : 6;
} mac_wur_cap_supported_bands_stru;

/* WUR Capabilities Information域信息 */
typedef struct {
    osal_u8 transition_delay;
    osal_u8 vl_wur_frame_support : 1, wur_group_ids_support : 2, rsvd1 : 1,
            wur_basic_ppdu_hdr_support_20mhz : 1, wur_fdma_support : 1,
            wur_short_wakeup_frame_support : 1, rsvd2 : 1;
} mac_wur_cap_information_stru;

/* WUR Operation Parameters域信息 */
typedef struct {
    osal_u32 min_wakeup_duration : 8, duty_cycle_period_units : 16, wur_operating_class : 8;
    osal_u32 wur_channel : 8, wur_beacon_period : 16, offset_of_twbtt_l : 8;
    osal_u8 offset_of_twbtt_h; /* 为了对齐拆分 */
    osal_u8 counter : 4, common_pn : 1, rsvd : 3;
    osal_u8 auc_resv[2]; /* 预留2字节对齐 */
} mac_wur_oper_param_stru;
#endif

/*****************************************************************************
    OTHERS定义
*****************************************************************************/

/*****************************************************************************
 函 数 名  : mac_hdr_set_from_ds
 功能描述  : This function sets the 'from ds' bit in the MAC header of the input frame
             to the given value stored in the LSB bit.
             The bit position of the 'from ds' in the 'frame control field' of the MAC
             header is represented by the bit pattern 0x00000010.

*****************************************************************************/
static INLINE__ osal_void mac_hdr_set_from_ds(osal_u8 *header, osal_u8 from_ds)
{
    ((mac_header_frame_control_stru *)(header))->from_ds = from_ds;
}

/*****************************************************************************
 函 数 名  : mac_hdr_set_to_ds
 功能描述  : This function sets the 'to ds' bit in the MAC header of the input frame
             to the given value stored in the LSB bit.
             The bit position of the 'to ds' in the 'frame control field' of the MAC
             header is represented by the bit pattern 0x00000001

*****************************************************************************/
static INLINE__ osal_void mac_hdr_set_to_ds(osal_u8 *header, osal_u8 to_ds)
{
    ((mac_header_frame_control_stru *)(header))->to_ds = to_ds;
}

/*****************************************************************************
 函 数 名  : mac_get_tid_value_4addr
 功能描述  : 四地址获取帧头中的tid

*****************************************************************************/
static INLINE__ osal_u8 mac_get_tid_value_4addr(const osal_u8 *header)
{
    return (header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x07); /* B0 - B2 */
}

/*****************************************************************************
 函 数 名  : mac_get_seq_num
 功能描述  : 获取接受侦的seqence number

*****************************************************************************/
static INLINE__ osal_u16 mac_get_seq_num(osal_u8 *header)
{
    osal_u16 seq_num = 0;

    seq_num = header[23];
    seq_num <<= 4;
    seq_num |= (header[22] >> 4);

    return seq_num;
}

/*****************************************************************************
 函 数 名  : mac_get_bar_start_seq_num
 功能描述  : 获取BAR帧中的start seq num值

*****************************************************************************/
static INLINE__ osal_u16 mac_get_bar_start_seq_num(const osal_u8 *payload)
{
    return ((payload[2] & 0xF0) >> 4) | (payload[3] << 4);
}

/*****************************************************************************
 函 数 名  : mac_get_ack_policy_4addr
 功能描述  : 4地址获取qos帧确认策略

*****************************************************************************/
static INLINE__ osal_u8 mac_get_ack_policy_4addr(const osal_u8 *header)
{
    return ((header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x60) >> 5); /* B5 - B6 */
}

/*****************************************************************************
 函 数 名  : mac_get_ack_policy
 功能描述  : 4地址获取qos帧确认策略

*****************************************************************************/
static INLINE__ osal_u8 mac_get_ack_policy(const osal_u8 *header, oal_bool_enum_uint8 is_4addr)
{
    if (is_4addr) {
        return ((header[MAC_QOS_CTRL_FIELD_OFFSET_4ADDR] & 0x60) >> 5); /* B5 - B6 */
    } else {
        return ((header[MAC_QOS_CTRL_FIELD_OFFSET] & 0x60) >> 5); /* B5 - B6 */
    }
}

/*****************************************************************************
 函 数 名  : mac_rx_get_da
 功能描述  : 获取收到的帧的目的地址
             参考协议 <802.11权威指南> 81页

*****************************************************************************/
static INLINE__ osal_void mac_rx_get_da(mac_ieee80211_frame_stru *mac_header, osal_u8 **da)
{
    /* IBSS、from AP */
    if (mac_header->frame_control.to_ds == 0) {
        *da = mac_header->address1;
    }
    /* WDS、to AP */
    else {
        *da = mac_header->address3;
    }
}

/*****************************************************************************
 函 数 名  : mac_rx_get_sa
 功能描述  : 获取收到的帧的源地址
             参考协议 <802.11权威指南> 81页

*****************************************************************************/
static INLINE__ osal_void mac_rx_get_sa(mac_ieee80211_frame_stru *mac_header, osal_u8 **sa)
{
    /* IBSS、to AP */
    if (mac_header->frame_control.from_ds == 0) {
        *sa = mac_header->address2;
    }
    /* from AP */
    else if ((mac_header->frame_control.from_ds == 1) && (mac_header->frame_control.to_ds == 0)) {
        *sa = mac_header->address3;
    }
    /* WDS */
    else {
        *sa = ((mac_ieee80211_frame_addr4_stru *)mac_header)->address4;
    }
}

/*****************************************************************************
 函 数 名  : mac_get_transmitter_addr
 功能描述  : 获取收到的帧的发送端地址
             参考协议 <802.11权威指南> 81页

*****************************************************************************/
static INLINE__ osal_void mac_get_transmit_addr(mac_ieee80211_frame_stru *mac_header, osal_u8 **bssid)
{
    /* 对于IBSS, STA, AP, WDS 场景下，获取发送端地址 */
    *bssid = mac_header->address2;
}

/*****************************************************************************
 函 数 名  : mac_get_submsdu_len
 功能描述  : 获取netbuf中submsdu的长度

*****************************************************************************/
static INLINE__ osal_void mac_get_submsdu_len(const osal_u8 *submsdu_hdr, osal_u16 *submsdu_len)
{
    *submsdu_len = *(submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET);
    *submsdu_len = (osal_u16)((*submsdu_len << 8) + *(submsdu_hdr + MAC_SUBMSDU_LENGTH_OFFSET + 1));
}

#if defined(_PRE_LITTLE_CPU_ENDIAN) && defined(_PRE_CPU_ENDIAN) && (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN)
#define MAC_IP_HDR_LEN_MASK 0x0F
#else
#define MAC_IP_HDR_LEN_MASK 0xF0
#endif
/*****************************************************************************
 函 数 名  : mac_ip_hdr_len
 功能描述  : 获取ip头长度
*****************************************************************************/
static inline osal_u32 mac_ip_hdr_len(const mac_ip_header_stru *ip_hdr)
{
    return (ip_hdr->version_ihl & MAC_IP_HDR_LEN_MASK) << 2;       /* 左移2bit:值对应4字节 */
}

/*****************************************************************************
 函 数 名  : mac_get_submsdu_pad_len
 功能描述  : 获取submsdu需要填充的字节数

*****************************************************************************/
static INLINE__ osal_void mac_get_submsdu_pad_len(osal_u16 msdu_len, osal_u8 *submsdu_pad_len)
{
    *submsdu_pad_len = msdu_len & 0x3;

    if (*submsdu_pad_len != 0) {
        *submsdu_pad_len = (MAC_BYTE_ALIGN_VALUE - *submsdu_pad_len);
    }
}

/*****************************************************************************
 函 数 名  : mac_is_grp_addr
 功能描述  : 判断该帧是否是组播帧

*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_grp_addr(const osal_u8 *addr)
{
    if ((addr[0] & 1) != 0) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : mac_ieeee80211_is_action
 功能描述  :

*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_ieeee80211_is_action(const osal_u8 *header)
{
    return (header[0] & (MAC_IEEE80211_FCTL_FTYPE | MAC_IEEE80211_FCTL_STYPE)) == (WLAN_ACTION << 4);
}

/*****************************************************************************
 函 数 名  : mac_get_frame_protected_flag
 功能描述  : 获取报文的帧保护位

*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_get_frame_protected_flag(const osal_u8 *mac_header)
{
    return (((mac_header[1] & 0x40) != 0) ? OSAL_TRUE : OSAL_FALSE);
}

/*****************************************************************************
 函 数 名  : mac_get_cap_info
 功能描述  : 从asoc rsp帧中获取capability fied

*****************************************************************************/
static INLINE__ osal_u16 mac_get_cap_info(osal_u8 *mac_frame_body)
{
    osal_u16 cap_info = 0;

    cap_info = mac_frame_body[0]; /* mac_frame_body[0]中存放cap_info的低8位 */
    cap_info |= (mac_frame_body[1] << 8); /* mac_frame_body[1]中存放cap_info的低8位 */

    return cap_info;
}

/*****************************************************************************
 功能描述  : 设置LLC SNAP, TX流程上调用
*****************************************************************************/
static INLINE__ osal_void mac_set_snap(oal_netbuf_stru *buf, osal_u16 ether_type)
{
    mac_llc_snap_stru *llc;
    osal_u16         use_btep1;
    osal_u16         use_btep2;

    /* LLC */
    llc = (mac_llc_snap_stru *)(oal_netbuf_data(buf) + ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN);
    llc->llc_dsap      = SNAP_LLC_LSAP;
    llc->llc_ssap      = SNAP_LLC_LSAP;
    llc->control       = LLC_UI;

    use_btep1              = oal_byteorder_host_to_net_uint16(ETHER_TYPE_AARP);
    use_btep2              = oal_byteorder_host_to_net_uint16(ETHER_TYPE_IPX);

    if (osal_unlikely((use_btep1 == ether_type) || (use_btep2 == ether_type))) {
        llc->org_code[0] = SNAP_BTEP_ORGCODE_0; /* 0x0 */
        llc->org_code[1] = SNAP_BTEP_ORGCODE_1; /* 0x0 */
        llc->org_code[2] = SNAP_BTEP_ORGCODE_2; /* 0xf8 */
    } else {
        llc->org_code[0]  = SNAP_RFC1042_ORGCODE_0;  /* 0x0 */
        llc->org_code[1]  = SNAP_RFC1042_ORGCODE_1;  /* 0x0 */
        llc->org_code[2]  = SNAP_RFC1042_ORGCODE_2;  /* 0x0 */
    }

    llc->ether_type = ether_type;

    oal_netbuf_pull(buf, (ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN));
}

/*****************************************************************************
 功能描述  : 获取认证状态字段

*****************************************************************************/
static INLINE__ osal_u16 mac_get_auth_alg(osal_u8 *mac_hdr)
{
    osal_u16 auth_alg = 0;

    auth_alg = mac_hdr[MAC_80211_FRAME_LEN + 1];
    auth_alg = (osal_u16)((auth_alg << 8) | mac_hdr[MAC_80211_FRAME_LEN + 0]);

    return auth_alg;
}

/*****************************************************************************
 函 数 名  : mac_get_auth_status
 功能描述  : 获取认证状态字段

*****************************************************************************/
static INLINE__ osal_u16 mac_get_auth_status(osal_u8 *mac_hdr)
{
    osal_u16 auth_status = 0;

    auth_status = mac_hdr[MAC_80211_FRAME_LEN + 5];
    auth_status = (osal_u16)((auth_status << 8) | mac_hdr[MAC_80211_FRAME_LEN + 4]);

    return auth_status;
}

/*****************************************************************************
 函 数 名  : mac_get_ft_status
 功能描述  : 获取FT response的status字段
 输入参数  : osal_u8 *mac_hdr
 输出参数  : 无
 返 回 值  : OAL_STATIC OAL_INLINE osal_u16
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u16 mac_get_ft_status(osal_u8 *mac_hdr)
{
    osal_u16 auth_status = 0;

    auth_status = mac_hdr[MAC_80211_FRAME_LEN + 15]; /* 偏移15字节 */
    auth_status = (osal_u16)((auth_status << 8) |   /* 左移8位 */
        mac_hdr[MAC_80211_FRAME_LEN + 14]);             /* 偏移14字节 */

    return auth_status;
}

/*****************************************************************************
 功能描述  : 获取认证帧序列号

*****************************************************************************/
static INLINE__ osal_u16 mac_get_auth_seq_num(osal_u8 *mac_hdr, osal_u16 payload_len)
{
    osal_u16 auth_seq = 0;
    /* auth帧payload长度必须大于等于4 */
    if (payload_len < 4) {
        return auth_seq;
    }

    auth_seq = mac_hdr[MAC_80211_FRAME_LEN + 3]; /* offset 3 */
    auth_seq = (osal_u16)((auth_seq << 8) | mac_hdr[MAC_80211_FRAME_LEN + 2]); /* offset 8 2 */

    return auth_seq;
}

/*****************************************************************************
 函 数 名  : mac_set_wep
 功能描述  : 设置protected frame subfield

*****************************************************************************/
static INLINE__ osal_void mac_set_wep(osal_u8 *hdr, osal_u8 wep)
{
    hdr[1] &= 0xBF;
    hdr[1] |= (osal_u8)(wep << 6);
}

/*****************************************************************************
 函 数 名  : mac_set_protectedframe
 功能描述  : 设置帧控制字段的受保护字段

*****************************************************************************/
static INLINE__ osal_void mac_set_protectedframe(osal_u8 *mac_hdr)
{
    mac_hdr[1] |= 0x40;
}
/*****************************************************************************
 函 数 名  : mac_get_protectedframe
 功能描述  : 获取帧头中保护位信息

*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_protectedframe(osal_u8 *mac_hdr)
{
    mac_ieee80211_frame_stru *mac_header = OSAL_NULL;
    mac_header = (mac_ieee80211_frame_stru *)mac_hdr;

    return (oal_bool_enum_uint8)(mac_header->frame_control.protected_frame);
}

/*****************************************************************************
 函 数 名  : mac_get_auth_algo_num
 功能描述  : 获取认证帧的认证算法

*****************************************************************************/
static INLINE__ osal_u16 mac_get_auth_algo_num(oal_netbuf_stru *netbuf)
{
    osal_u16 auth_algo = 0;
    osal_u8 *mac_payload = oal_netbuf_data(netbuf) + MAC_80211_FRAME_LEN;

    auth_algo = mac_payload[1];
    auth_algo = (osal_u16)((auth_algo << 8) | mac_payload[0]);

    return auth_algo;
}

/*****************************************************************************
 函 数 名  : mac_get_auth_ch_text
 功能描述  : 获取认证帧中的challenge txt

*****************************************************************************/
static INLINE__ osal_u8 *mac_get_auth_ch_text(osal_u8 *mac_hdr)
{
    return &(mac_hdr[MAC_80211_FRAME_LEN + 6]);
}

/*****************************************************************************
 函 数 名  : mac_is_4addr
 功能描述  : 是否为4地址

*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_4addr(osal_u8 *mac_hdr)
{
    osal_u8 is_tods;
    osal_u8 is_from_ds;
    oal_bool_enum_uint8 is_4addr;

    is_tods = mac_hdr_get_to_ds(mac_hdr);
    is_from_ds = mac_hdr_get_from_ds(mac_hdr);

    is_4addr = (is_tods != OSAL_FALSE) && (is_from_ds != OSAL_FALSE);

    return is_4addr;
}

/*****************************************************************************
 函 数 名  : mac_get_address2
 功能描述  : 拷贝地址2
*****************************************************************************/
static INLINE__ osal_void mac_get_address2(osal_u8 *mac_hdr, osal_u8 *addr)
{
    /* 10:addr2相对802.11帧头的偏移量 */
    (osal_void)memcpy_s(addr, WLAN_MAC_ADDR_LEN, mac_hdr + 10, WLAN_MAC_ADDR_LEN);
}

/*****************************************************************************
 函 数 名  : mac_get_address3
 功能描述  : 拷贝地址3
*****************************************************************************/
static INLINE__ osal_void mac_get_address3(osal_u8 *mac_hdr, osal_u8 *addr)
{
    /* 16:addr3相对802.11帧头的偏移量 */
    (osal_void)memcpy_s(addr, WLAN_MAC_ADDR_LEN, mac_hdr + 16, WLAN_MAC_ADDR_LEN);
}

/*****************************************************************************
 函 数 名  : mac_get_qos_ctrl
 功能描述  : 获取mac头中的qos ctrl字段
*****************************************************************************/
static INLINE__ osal_void mac_get_qos_ctrl(osal_u8 *mac_hdr, osal_u8 *qos_ctrl)
{
    if (mac_is_4addr(mac_hdr) != OSAL_TRUE) {
        (osal_void)memcpy_s(qos_ctrl, MAC_QOS_CTL_LEN, mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET, MAC_QOS_CTL_LEN);
        return;
    }

    (osal_void)memcpy_s(qos_ctrl, MAC_QOS_CTL_LEN, mac_hdr + MAC_QOS_CTRL_FIELD_OFFSET_4ADDR, MAC_QOS_CTL_LEN);
    return;
}

/*****************************************************************************
 函 数 名  : mac_get_asoc_status
 功能描述  : 获取关联帧中的状态信息

*****************************************************************************/
static INLINE__ mac_status_code_enum_uint16 mac_get_asoc_status(const osal_u8 *mac_payload)
{
    mac_status_code_enum_uint16 asoc_status;

    asoc_status = (mac_payload[3] << 8) | mac_payload[2];

    return asoc_status;
}

/*****************************************************************************
 函 数 名  : mac_get_asoc_status
 功能描述  : 获取关联帧中的关联ID

*****************************************************************************/
static INLINE__ osal_u16 mac_get_asoc_id(const osal_u8 *mac_payload)
{
    osal_u16 asoc_id;

    asoc_id = mac_payload[4] | (mac_payload[5] << 8);
    asoc_id &= 0x3FFF; /* 取低14位 */

    return asoc_id;
}

/*****************************************************************************
 函 数 名  : wlan_get_bssid
 功能描述  : 根据"from ds"bit,从帧中提取bssid(mac地址)

*****************************************************************************/
static INLINE__ osal_void mac_get_bssid(osal_u8 *mac_hdr, osal_u8 *bssid, ssize_t bssid_len)
{
    if (bssid_len > WLAN_MAC_ADDR_LEN) {
        return;
    }
    if (mac_hdr_get_from_ds(mac_hdr) == 1) {
        mac_get_address2(mac_hdr, bssid);
    } else if (mac_hdr_get_to_ds(mac_hdr) == 1) {
        mac_get_address1(mac_hdr, bssid);
    } else {
        mac_get_address3(mac_hdr, bssid);
    }
}

/*****************************************************************************
 函 数 名  : mac_frame_is_eapol
 功能描述  : 判断LLC 帧类型是否为EAPOL 类型帧

*****************************************************************************/
static INLINE__ oal_bool_enum mac_frame_is_eapol(mac_llc_snap_stru *mac_llc_snap)
{
    return (oal_bool_enum)((oal_byteorder_host_to_net_uint16(mac_llc_snap->ether_type) ==
        ETHER_ONE_X_TYPE) ? OSAL_TRUE : OSAL_FALSE);
}

/*****************************************************************************
 函 数 名  : mac_frame_rom_set_ptr_val
 功能描述  : 统一设置指针指向内存区域的值为val
*****************************************************************************/
static INLINE__ osal_void mac_frame_rom_set_ptr_val(dft_80211_frame_ctx *frame_ctx, osal_u8 val)
{
    frame_ctx->frame_cb_switch = val;
    frame_ctx->frame_dscr_switch = val;
    frame_ctx->frame_content_switch = val;
}

/*****************************************************************************
 功能描述  : 判断一个帧是否是ftm frame
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_ftm_frame(oal_netbuf_stru *pst_netbuf)
{
    osal_u8 *mac_header  = oal_netbuf_header(pst_netbuf);
    osal_u8 *mac_payload = oal_netbuf_data(pst_netbuf);

    /* Management frame */
    if (mac_get_frame_type_and_subtype(mac_header) == (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        if ((mac_payload[0] == MAC_ACTION_CATEGORY_PUBLIC) && (mac_payload[1] == MAC_PUB_FTM)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 判断一个帧是否是ftm req frame
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_ftm_req_frame(oal_netbuf_stru *pst_netbuf)
{
    osal_u8 *mac_header  = oal_netbuf_header(pst_netbuf);
    osal_u8 *mac_payload = oal_netbuf_data(pst_netbuf);

    /* Management frame */
    if (mac_get_frame_type_and_subtype(mac_header) == (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        if ((mac_payload[0] == MAC_ACTION_CATEGORY_PUBLIC) && (mac_payload[1] == MAC_PUB_FTM_REQ)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 判断一个帧是否是ftm关联帧
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 mac_is_ftm_related_frame(oal_netbuf_stru *pst_netbuf)
{
    osal_u8 *mac_header  = oal_netbuf_header(pst_netbuf);
    osal_u8 *mac_payload = oal_netbuf_data(pst_netbuf);

    /* Management frame */
    if (mac_get_frame_type_and_subtype(mac_header) == (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        if ((mac_payload[0] == MAC_ACTION_CATEGORY_PUBLIC) && (mac_payload[1] == MAC_PUB_FTM)) {
            return OAL_TRUE;
        }
        if ((mac_payload[0] == MAC_ACTION_CATEGORY_PUBLIC) && (mac_payload[1] == MAC_PUB_FTM_REQ)) {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

/*****************************************************************************
    函数声明
*****************************************************************************/
oal_bool_enum_uint8 mac_is_ftm_req_frame(oal_netbuf_stru *pst_netbuf);
oal_bool_enum_uint8 mac_is_ftm_req_frame(oal_netbuf_stru *pst_netbuf);
oal_bool_enum_uint8 mac_is_ftm_related_frame(oal_netbuf_stru *pst_netbuf);
#ifdef _PRE_WLAN_FEATURE_P2P
osal_u8 *hmac_find_p2p_attribute_etc(osal_u8 eid, osal_u8 *ies, osal_s32 l_len);
#endif
osal_u8 *hmac_find_ie_etc(osal_u8 eid, osal_u8 *ies, osal_s32 l_len);
osal_u8 *hmac_find_vendor_ie_etc(osal_u32 oui, osal_u8 oui_type, osal_u8 *ies, osal_s32 len);
osal_void hmac_set_beacon_interval_field_etc(osal_void *vap, osal_u8 *buffer);
osal_void hmac_set_cap_info_ap_etc(osal_void *vap, osal_u8 *cap_info);
osal_void mac_set_cap_info_sta_etc(osal_void *vap, osal_u8 *cap_info);
osal_void hmac_set_ssid_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u16 frm_type);
osal_void hmac_set_supported_rates_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#ifdef _PRE_WLAN_FEATURE_1024QAM
osal_void hmac_set_1024qam_vendor_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#endif

osal_void hmac_set_exsup_rates_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);

osal_void mac_set_timeout_interval_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u32 type,
    osal_u32 timeout);
osal_void hmac_set_ht_capabilities_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len);
osal_void hmac_set_ht_opern_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void hmac_set_obss_scan_params_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void hmac_set_ext_capabilities_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void hmac_set_wmm_params_ie_etc(osal_void *vap, osal_u8 *buffer, oal_bool_enum_uint8 is_qos,
    osal_u8 *ie_len);
osal_u8 *hmac_get_ssid_etc(osal_u8 *beacon_body, osal_s32 frame_body_len, osal_u8 *ssid_len);
osal_u8 *mac_get_ext_ie_body(osal_u8 *beacon_body, osal_s32 frame_body_len,
    osal_u8 *ext_ie_body_len, osal_u8 ext_eid);
osal_u16 hmac_get_beacon_period_etc(const osal_u8 *beacon_body);
osal_u8 hmac_get_dtim_period_etc(osal_u8 *frame_body, osal_u16 frame_body_len);
osal_u8 hmac_get_dtim_cnt_etc(osal_u8 *frame_body, osal_u16 frame_body_len);
osal_u8 *hmac_get_wmm_ie_etc(osal_u8 *beacon_body, osal_u16 frame_len);
osal_void mac_set_power_cap_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void hmac_set_supported_channel_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len);
osal_void hmac_set_wmm_ie_sta_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#ifdef _PRE_WLAN_FEATURE_WMMAC
extern osal_u16   mac_set_wmmac_ie_sta_etc(osal_void *vap, osal_u8 *buffer, mac_wmm_tspec_stru *addts_args);
#endif /* _PRE_WLAN_FEATURE_WMMAC */
osal_void hmac_set_listen_interval_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#ifdef _PRE_WLAN_FEATURE_WPA3
osal_void hmac_set_rsnx_ie(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#endif
osal_void hmac_set_rsn_ie_etc(osal_void *vap, osal_u8 *pmkid, osal_u8 *buffer, osal_u8 *ie_len);
osal_void hmac_set_wpa_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void hmac_set_status_code_ie_etc(osal_u8 *buffer, mac_status_code_enum_uint16 status_code);
osal_void hmac_set_aid_ie_etc(osal_u8 *buffer, osal_u16 assoc_id);
osal_u8 hmac_get_bss_type_etc(osal_u16 cap_info);
osal_u32 hmac_check_mac_privacy_etc(osal_u16 cap_info, osal_void *vap);
oal_bool_enum_uint8 hmac_is_wmm_ie_etc(const osal_u8 *ie);
osal_void hmac_set_vht_capabilities_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len);
osal_void hmac_set_vht_opern_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len);
#ifdef _PRE_WLAN_FEATURE_11AX
osal_void hmac_set_he_capabilities_ie(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len);
osal_void hmac_set_he_operation_ie(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len);
#endif
osal_u8 *hmac_find_ie_ext_ie(osal_u8 eid, osal_u8 ext_eid, osal_u8 *ie_addr, osal_s32 frame_len);

osal_u16 hmac_get_rsn_capability_etc(const osal_u8 *rsn_ie);

osal_void hmac_add_app_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u16 *ie_len, app_ie_type_uint8 type);

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
osal_void hmac_set_opmode_field_etc(osal_void *vap, osal_u8 *buffer, wlan_nss_enum_uint8 nss);
osal_void hmac_set_opmode_notify_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#endif
#ifdef _PRE_WLAN_FEATURE_PMF
wlan_pmf_cap_status_uint8  hmac_get_pmf_cap_etc(osal_u8 *ie, osal_u32 ie_len);
#endif
osal_u16 hmac_encap_2040_coext_mgmt_etc(osal_void *vap, oal_netbuf_stru *buffer,
    osal_u8 coext_info, osal_u32 chan_report);

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
osal_void mac_set_vender_4addr_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#endif

osal_void mac_set_vendor_vht_ie(osal_void *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void mac_set_vendor_novht_ie(osal_void *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len);

#ifdef _PRE_WLAN_FEATURE_WUR_TX
osal_void mac_set_wur_supported_bands_field(osal_void *vap, osal_u8 *buffer);
osal_void mac_set_wur_cap_information_field(osal_void *vap, osal_u8 *buffer);
osal_void mac_set_wur_capabilities_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void mac_set_wur_oper_parameters_field(osal_void *vap, osal_u8 *buffer);
osal_void mac_set_wur_operation_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#endif

osal_void mac_set_dsss_params_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u8 chan_num);
#ifdef _PRE_WLAN_FEATURE_11D
osal_void mac_set_country_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
#endif
osal_void mac_set_pwrconstraint_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void mac_set_quiet_ie_etc(const osal_void *vap, osal_u8 *buffer,
    const mac_quiet_info *quiet_info, osal_u8 *ie_len);

osal_void mac_set_erp_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void mac_set_bssload_ie(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_void mac_set_tpc_report_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
osal_u8 const *mac_frame_get_p2p_oui(osal_void);

osal_void mac_add_wps_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u16 *ie_len, app_ie_type_uint8 type);
osal_void mac_add_extend_app_ie_etc(const osal_void *vap, osal_u8 **buffer, app_ie_type_uint8 type);
oal_bool_enum_uint8 mac_frame_is_null_data(oal_netbuf_stru *net_buf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_frame.h */
