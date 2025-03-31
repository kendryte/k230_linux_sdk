/*
 * Copyright (c) CompanyNameMagicTag. 2021-2024. All rights reserved.
 * Description: Header files shared by wlan_types.h and wlan_types_rom.h.
 * Create: 2021-09-18
 */

#ifndef __WLAN_TYPES_BASE_ROM_H__
#define __WLAN_TYPES_BASE_ROM_H__

#include "td_base.h"

#include "osal_types.h"

#include "wlan_custom_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    枚举定义
*****************************************************************************/
/*****************************************************************************
  3.1 基本枚举类型
*****************************************************************************/
/* 管理帧子类型 */
typedef enum {
    WLAN_ASSOC_REQ              = 0,    /* 0000 */
    WLAN_ASSOC_RSP              = 1,    /* 0001 */
    WLAN_REASSOC_REQ            = 2,    /* 0010 */
    WLAN_REASSOC_RSP            = 3,    /* 0011 */
    WLAN_PROBE_REQ              = 4,    /* 0100 */
    WLAN_PROBE_RSP              = 5,    /* 0101 */
    WLAN_TIMING_AD              = 6,    /* 0110 */
    WLAN_MGMT_SUBTYPE_RESV1     = 7,    /* 0111 */
    WLAN_BEACON                 = 8,    /* 1000 */
    WLAN_ATIM                   = 9,    /* 1001 */
    WLAN_DISASOC                = 10,   /* 1010 */
    WLAN_AUTH                   = 11,   /* 1011 */
    WLAN_DEAUTH                 = 12,   /* 1100 */
    WLAN_ACTION                 = 13,   /* 1101 */
    WLAN_ACTION_NO_ACK          = 14,   /* 1110 */
    WLAN_MGMT_SUBTYPE_RESV2     = 15,   /* 1111 */

    WLAN_MGMT_SUBTYPE_BUTT      = 16    /* 一共16种管理帧子类型 */
} wlan_frame_mgmt_subtype_enum;

/* 不应该BUTT后续存在枚举 */
typedef enum {
    WLAN_WME_AC_BE = 0,    /* best effort */
    WLAN_WME_AC_BK = 1,    /* background */
    WLAN_WME_AC_VI = 2,    /* video */
    WLAN_WME_AC_VO = 3,    /* voice */

    WLAN_WME_AC_BUTT = 4,
    WLAN_WME_AC_MGMT = WLAN_WME_AC_BUTT,   /* 管理AC，协议没有,对应硬件高优先级队列 */

    WLAN_WME_AC_PSM = 5,    /* 节能AC, 协议没有，对应硬件组播队列 */
} wlan_wme_ac_type_enum;
typedef osal_u8 wlan_wme_ac_type_enum_uint8;

/* TID编号类别 */
typedef enum {
    WLAN_TIDNO_BEST_EFFORT              = 0, /* BE业务 */
    WLAN_TIDNO_BACKGROUND               = 1, /* BK业务 */
    WLAN_TIDNO_UAPSD                    = 2, /* U-APSD */
    WLAN_TIDNO_ANT_TRAINING_LOW_PRIO    = 3, /* 智能天线低优先级训练帧 */
    WLAN_TIDNO_ANT_TRAINING_HIGH_PRIO   = 4, /* 智能天线高优先级训练帧 */
    WLAN_TIDNO_VIDEO                    = 5, /* VI业务 */
    WLAN_TIDNO_VOICE                    = 6, /* VO业务 */
    WLAN_TIDNO_BCAST                    = 7, /* 广播用户的广播或者组播报文 */

    WLAN_TIDNO_BUTT
} wlan_tidno_enum;
typedef osal_u8 wlan_tidno_enum_uint8;

/* TID编号类别放入平台 */

/* VAP的工作模式 */
typedef enum {
    WLAN_VAP_MODE_CONFIG,        /* 配置模式 */
    WLAN_VAP_MODE_BSS_STA,       /* BSS STA模式 */
    WLAN_VAP_MODE_BSS_AP,        /* BSS AP模式 */
    WLAN_VAP_MODE_WDS,           /* WDS模式 */
    WLAN_VAP_MODE_MONITOER,      /* 侦听模式 */
    WLAN_VAP_HW_TEST,

    WLAN_VAP_MODE_BUTT
} wlan_vap_mode_enum;
typedef osal_u8 wlan_vap_mode_enum_uint8;

typedef enum {
    WLAN_BAND_2G,
    WLAN_BAND_5G,

    WLAN_BAND_BUTT
} wlan_channel_band_enum;
typedef osal_u8 wlan_channel_band_enum_uint8;

/* 认证算法 */
typedef enum {
    WLAN_WITP_AUTH_OPEN_SYSTEM = 0,
    WLAN_WITP_AUTH_SHARED_KEY,
    WLAN_WITP_AUTH_FT,
    WLAN_WITP_AUTH_SAE, /* 802.11-2016协议查得认证帧中Auth Alg mode 字段3为SAE认证 */
    WLAN_WITP_AUTH_NETWORK_EAP,
    WLAN_WITP_AUTH_NUM,
    WLAN_WITP_AUTH_MAX = WLAN_WITP_AUTH_NUM - 1,
    WLAN_WITP_AUTH_AUTOMATIC,

    WLAN_WITP_AUTH_BUTT
} wlan_auth_alg_mode_enum;
typedef osal_u8 wlan_auth_alg_mode_enum_uint8;

typedef enum {
    /* 注意: wlan_cipher_key_type_enum和hal_key_type_enum 值一致, 如果硬件有改变，则应该在HAL 层封装 */
    /* 此处保持和协议规定额秘钥类型一致 */
    WLAN_KEY_TYPE_TX_GTK              = 0,  /* TX GTK */
    WLAN_KEY_TYPE_PTK                 = 1,  /* PTK */
    WLAN_KEY_TYPE_RX_GTK              = 2,  /* RX GTK */
    WLAN_KEY_TYPE_RX_GTK2             = 3,  /* RX GTK2 51不用 */
    WLAN_KEY_TYPE_BUTT
} wlan_cipher_key_type_enum;
typedef osal_u8 wlan_cipher_key_type_enum_uint8;

typedef enum {
    /* 注意: wlan_key_origin_enum_uint8和hal_key_origin_enum_uint8 值一致, 如果硬件有改变，则应该在HAL 层封装 */
    WLAN_AUTH_KEY = 0,      /* Indicates that for this key, this STA is the authenticator */
    WLAN_SUPP_KEY = 1,      /* Indicates that for this key, this STA is the supplicant */

    WLAN_KEY_ORIGIN_BUTT
} wlan_key_origin_enum;
typedef osal_u8 wlan_key_origin_enum_uint8;

typedef enum {
    /* 按照80211-2012/ 11ac-2013 协议 Table 8-99 Cipher suite selectors 定义 */
    WLAN_80211_CIPHER_SUITE_GROUP_CIPHER = 0,
    WLAN_80211_CIPHER_SUITE_WEP_40       = 1,
    WLAN_80211_CIPHER_SUITE_TKIP         = 2,
    WLAN_80211_CIPHER_SUITE_RSV          = 3,
    WLAN_80211_CIPHER_SUITE_NO_ENCRYP    = WLAN_80211_CIPHER_SUITE_RSV, /* 采用协议定义的保留值做不加密类型 */
    WLAN_80211_CIPHER_SUITE_CCMP         = 4,
    WLAN_80211_CIPHER_SUITE_WEP_104      = 5,
    WLAN_80211_CIPHER_SUITE_BIP          = 6,
    WLAN_80211_CIPHER_SUITE_GROUP_DENYD  = 7,
    WLAN_80211_CIPHER_SUITE_GCMP         = 8,             /* GCMP-128 default for a DMG STA */
    WLAN_80211_CIPHER_SUITE_GCMP_256     = 9,
    WLAN_80211_CIPHER_SUITE_CCMP_256     = 10,
    WLAN_80211_CIPHER_SUITE_BIP_GMAC_128 = 11,           /* BIP GMAC 128 */
    WLAN_80211_CIPHER_SUITE_BIP_GMAC_256 = 12,           /* BIP GMAC 256 */
    WLAN_80211_CIPHER_SUITE_BIP_CMAC_256 = 13,           /* BIP CMAC 256 */

    WLAN_80211_CIPHER_SUITE_WAPI         = 14            /* 随意定，不影响11i即可 */
} wlan_ciper_protocol_type_enum;
typedef osal_u8 wlan_ciper_protocol_type_enum_uint8;

typedef osal_u8 witp_wpa_versions_enum_uint8;

typedef struct {
    wlan_cipher_key_type_enum_uint8          cipher_key_type;      /* 密钥ID/类型 */
    wlan_ciper_protocol_type_enum_uint8      cipher_protocol_type;
    osal_u8                                  cipher_key_id;
    osal_u8                                  auc_resv[1];
} wlan_security_txop_params_stru;

/* 调制方式枚举 */
typedef enum {
    WLAN_MOD_DSSS,
    WLAN_MOD_OFDM,

    WLAN_MOD_BUTT
} wlan_mod_enum;
typedef osal_u8 wlan_mod_enum_uint8;

/*****************************************************************************
  3.4 VHT枚举类型
*****************************************************************************/
typedef enum {
    WLAN_VHT_MCS0,
    WLAN_VHT_MCS1,
    WLAN_VHT_MCS2,
    WLAN_VHT_MCS3,
    WLAN_VHT_MCS4,
    WLAN_VHT_MCS5,
    WLAN_VHT_MCS6,
    WLAN_VHT_MCS7,
    WLAN_VHT_MCS8,
    WLAN_VHT_MCS9,
#ifdef _PRE_WLAN_FEATURE_1024QAM
    WLAN_VHT_MCS10,
    WLAN_VHT_MCS11,
#endif

    WLAN_VHT_MCS_BUTT
} wlan_vht_mcs_enum;
typedef osal_u8 wlan_vht_mcs_enum_uint8;

typedef enum {
    WLAN_LEGACY_11B_RESERVED1       = 0,
    WLAN_SHORT_11B_2M_BPS           = 1,
    WLAN_SHORT_11B_5M5_BPS          = 2,
    WLAN_SHORT_11B_11M_BPS          = 3,

    WLAN_LONG_11B_1M_BPS            = 4,
    WLAN_LONG_11B_2M_BPS            = 5,
    WLAN_LONG_11B_5M5_BPS           = 6,
    WLAN_LONG_11B_11M_BPS           = 7,

    WLAN_LEGACY_OFDM_48M_BPS        = 8,
    WLAN_LEGACY_OFDM_24M_BPS        = 9,
    WLAN_LEGACY_OFDM_12M_BPS        = 10,
    WLAN_LEGACY_OFDM_6M_BPS         = 11,
    WLAN_LEGACY_OFDM_54M_BPS        = 12,
    WLAN_LEGACY_OFDM_36M_BPS        = 13,
    WLAN_LEGACY_OFDM_18M_BPS        = 14,
    WLAN_LEGACY_OFDM_9M_BPS         = 15,

    WLAN_LEGACY_RATE_VALUE_BUTT
} wlan_legacy_rate_value_enum;
typedef osal_u8 wlan_legacy_rate_value_enum_uint8;

/*****************************************************************************
  3.3 HT枚举类型
*****************************************************************************/
// 扩展或者删除字段时，务必同步调整 g_phy_bandwidth_sec_offset_table \ mac_vap_bw_mode_to_bw \ hal_mac_change_bw_mode_to_bw_index
typedef enum {
    WLAN_BAND_WIDTH_20M,
    WLAN_BAND_WIDTH_40PLUS,                     /* 从20信道+1 */
    WLAN_BAND_WIDTH_40MINUS,                    /* 从20信道-1 */
    WLAN_BAND_WIDTH_80PLUSPLUS,                 /* 从20信道+1, 从40信道+1 */
    WLAN_BAND_WIDTH_80PLUSMINUS,                /* 从20信道+1, 从40信道-1 */
    WLAN_BAND_WIDTH_80MINUSPLUS,                /* 从20信道-1, 从40信道+1 */
    WLAN_BAND_WIDTH_80MINUSMINUS,               /* 从20信道-1, 从40信道-1 */
    WLAN_BAND_WIDTH_5M,
    WLAN_BAND_WIDTH_10M,
    WLAN_BAND_WIDTH_40M,
    WLAN_BAND_WIDTH_80M,
    WLAN_BAND_WIDTH_BUTT
} wlan_channel_bandwidth_enum;
typedef osal_u8 wlan_channel_bandwidth_enum_uint8;

/* 因为要用作预编译，所以由枚举改成宏，为了便于理解，下面的类型转义先不变 */
typedef osal_u8 wlan_nss_enum_uint8;

typedef enum {
    WLAN_HT_NON_STBC                   = 0,
    WLAN_HT_ADD_ONE_NTS                = 1,
    WLAN_HT_ADD_TWO_NTS                = 2,

    WLAN_HT_STBC_BUTT
} wlan_ht_stbc_enum;
typedef osal_u8 wlan_ht_stbc_enum_uint8;

typedef struct {
    osal_u8                               group_id;      /* group_id   */
    osal_u8                               txop_ps_not_allowed;
    osal_u16                              partial_aid;   /* partial_aid */
} wlan_groupid_partial_aid_stru;

/* channel结构体 */
typedef struct {
    osal_u8                             chan_number;     /* 主20MHz信道号 */
    wlan_channel_band_enum_uint8        band;            /* 频段 */
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;    /* 带宽模式 */
    osal_u8                             chan_idx;        /* 信道索引号 */
} mac_channel_stru;

/* channel switch结构体 */
typedef struct {
    mac_channel_stru                    mac_channel;
    osal_bool                           clear_fifo;
} mac_switch_channel_stru;

/*****************************************************************************
  3.4 算法宏,枚举类型
*****************************************************************************/

/* TPC工作模式 */
typedef enum {
    WLAN_TPC_WORK_MODE_DISABLE        = 0,   /* 禁用TPC动态调整功率模式: 直接采用固定功率模式，数据帧的Data0采用配置的, Data1~3以及管理帧、控制帧都用最大功率 */
    WLAN_TPC_WORK_MODE_ENABLE         = 1,   /* 自适应功率模式: 数据帧的Data0采用自适应功率, Data1~3以及管理帧、控制帧都用最大功率 */

    WLAN_TPC_WORK_MODE_BUTT
} wlan_tpc_work_mode_enum;
typedef osal_u8 wlan_tpc_mode_enum_uint8;

/*****************************************************************************
  3.5 WME枚举类型
*****************************************************************************/

/* WMM枚举类型放入平台 */

/* 帧类型 (2-bit) */
typedef enum {
    WLAN_MANAGEMENT            = 0,
    WLAN_CONTROL               = 1,
    WLAN_DATA_BASICTYPE        = 2,
    WLAN_RESERVED              = 3,

    WLAN_FRAME_TYPE_BUTT
} wlan_frame_type_enum;
typedef osal_u8 wlan_frame_type_enum_uint8;

/* 帧子类型 (4-bit) */
/* 管理帧子类型放入平台SPEC */

/* 控制帧帧子类型 */
typedef enum {
    /* 0~6 reserved */
    WLAN_HE_TRIG_FRAME          = 2,     /* 0010 */
    WLAN_VHT_NDPA               = 5,     /* 0101 */
    WLAN_CONTROL_WRAPPER        = 7,
    WLAN_BLOCKACK_REQ           = 8,
    WLAN_BLOCKACK               = 9,
    WLAN_PS_POLL                = 10,
    WLAN_RTS                    = 11,
    WLAN_CTS                    = 12,
    WLAN_ACK                    = 13,
    WLAN_CF_END                 = 14,
    WLAN_CF_END_CF_ACK          = 15,

    WLAN_CONTROL_SUBTYPE_BUTT   = 16,
} wlan_frame_control_subtype_enum;

/* 数据帧子类型 */
typedef enum {
    WLAN_DATA                   = 0,
    WLAN_DATA_CF_ACK            = 1,
    WLAN_DATA_CF_POLL           = 2,
    WLAN_DATA_CF_ACK_POLL       = 3,
    WLAN_NULL_FRAME             = 4,
    WLAN_CF_ACK                 = 5,
    WLAN_CF_POLL                = 6,
    WLAN_CF_ACK_POLL            = 7,
    WLAN_QOS_DATA               = 8,
    WLAN_QOS_DATA_CF_ACK        = 9,
    WLAN_QOS_DATA_CF_POLL       = 10,
    WLAN_QOS_DATA_CF_ACK_POLL   = 11,
    WLAN_QOS_NULL_FRAME         = 12,
    WLAN_DATA_SUBTYPE_RESV1     = 13,
    WLAN_QOS_CF_POLL            = 14,
    WLAN_QOS_CF_ACK_POLL        = 15,

    WLAN_DATA_SUBTYPE_MGMT      = 16,
} wlan_frame_data_subtype_enum;

/* ACK类型定义 */
typedef enum {
    WLAN_TX_NORMAL_ACK = 0,
    WLAN_TX_NO_ACK,
    WLAN_TX_NO_EXPLICIT_ACK,
    WLAN_TX_BLOCK_ACK,

    WLAN_TX_NUM_ACK_BUTT
} wlan_tx_ack_policy_enum;
typedef osal_u8   wlan_tx_ack_policy_enum_uint8;

/*****************************************************************************
  3.6 信道枚举
*****************************************************************************/

/* 信道编码方式 */
typedef enum {
    WLAN_BCC_CODE                   = 0,
    WLAN_LDPC_CODE                  = 1,
    WLAN_CHANNEL_CODE_BUTT
} wlan_channel_code_enum;
typedef osal_u8 wlan_channel_code_enum_uint8;

/* 扫描类型 */
typedef enum {
    WLAN_SCAN_TYPE_PASSIVE       = 0,
    WLAN_SCAN_TYPE_ACTIVE        = 1,

    WLAN_SCAN_TYPE_BUTT
} wlan_scan_type_enum;
typedef osal_u8 wlan_scan_type_enum_uint8;

/* 扫描模式 */
typedef enum {
    WLAN_SCAN_MODE_FOREGROUND     = 0,          /* 前景扫描不分AP/STA(即初始扫描，连续式) */
    WLAN_SCAN_MODE_BACKGROUND_STA = 1,      /* STA背景扫描 */
    WLAN_SCAN_MODE_BACKGROUND_AP  = 2,       /* AP背景扫描(间隔式) */
    WLAN_SCAN_MODE_BACKGROUND_OBSS = 3,     /* 20/40MHz共存的obss扫描 */
    WLAN_SCAN_MODE_BACKGROUND_ALG_INTF_DET  = 4,
    WLAN_SCAN_MODE_BACKGROUND_PNO  = 5,      /* PNO调度扫描 */
    WLAN_SCAN_MODE_RRM_BEACON_REQ  = 6,
    WLAN_SCAN_MODE_BACKGROUND_CSA  = 7,      /* 信道切换扫描 */
    WLAN_SCAN_MODE_BACKGROUND_HILINK = 8,   /* hilink扫描未关联用户 */
    WLAN_SCAN_MODE_FTM_REQ      = 9,
    WLAN_SCAN_MODE_GNSS_SCAN    = 10,
    WLAN_SCAN_MODE_ROAM_SCAN    = 11,
    WLAN_SCAN_MODE_BACKGROUND_SDP = 12,

    WLAN_SCAN_MODE_BUTT
} wlan_scan_mode_enum;
typedef osal_u8 wlan_scan_mode_enum_uint8;

typedef enum {
    WLAN_LEGACY_VAP_MODE     = 0,    /* 非P2P设备 */
    WLAN_P2P_GO_MODE         = 1,    /* P2P_GO */
    WLAN_P2P_DEV_MODE        = 2,    /* P2P_Device */
    WLAN_P2P_CL_MODE         = 3,    /* P2P_CL */

    WLAN_P2P_BUTT
} wlan_p2p_mode_enum;
typedef osal_u8 wlan_p2p_mode_enum_uint8;

/*****************************************************************************
  3.7 加密枚举
*****************************************************************************/

/* 用户距离状态 */
typedef enum {
    WLAN_DISTANCE_NEAR       = 0,
    WLAN_DISTANCE_NORMAL     = 1,
    WLAN_DISTANCE_FAR        = 2,

    WLAN_DISTANCE_BUTT
} wlan_user_distance_enum;
typedef osal_u8 wlan_user_distance_enum_uint8;

/*****************************************************************************
  3.8 linkloss场景枚举
*****************************************************************************/


/*****************************************************************************
  3.9 roc场景枚举
*****************************************************************************/

/*****************************************************************************
  3.10 roam场景枚举
*****************************************************************************/

/*****************************************************************************
  3.10 m2s切换枚举
*****************************************************************************/
/* mimo-siso切换tpye */
typedef enum {
    WLAN_M2S_TYPE_SW = 0,
    WLAN_M2S_TYPE_HW = 1,

    WLAN_M2S_TYPE_BUTT
} wlan_m2s_tpye_enum;
typedef osal_u8 wlan_m2s_type_enum_uint8;

/* mimo-siso切换tpye */
typedef enum {
    WLAN_M2S_TRIGGER_MODE_DBDC      = BIT0,
    WLAN_M2S_TRIGGER_MODE_FAST_SCAN = BIT1,
    WLAN_M2S_TRIGGER_MODE_RSSI      = BIT2,
    WLAN_M2S_TRIGGER_MODE_BTCOEX    = BIT3,
    WLAN_M2S_TRIGGER_MODE_COMMAND   = BIT4,
    WLAN_M2S_TRIGGER_MODE_TEST      = BIT5,
    WLAN_M2S_TRIGGER_MODE_CUSTOM    = BIT6,   /* 定制化优先级高，只能被蓝牙打断 */
    WLAN_M2S_TRIGGER_MODE_SPEC      = BIT7,   /* spec优先级最高，不允许打断 */

    WLAN_M2S_TRIGGER_MODE_BUTT
} wlan_m2s_trigger_mode_enum;
typedef osal_u8 wlan_m2s_trigger_mode_enum_uint8;

/*****************************************************************************
  3.11 m2s切换枚举
*****************************************************************************/

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
/* m2s触发条件 */
typedef struct {
    osal_u8   dbdc         : 1,        /* dbdc触发 */
              fast_on      : 1,        /* 并发扫描触发 */
              rssi_snr     : 1,        /* rssi/snr触发 */
              btcoex       : 1,        /* btcoex触发 */
              command      : 1,        /* 上层命令触发 */
              test         : 1,        /* 测试命令触发 */
              custom       : 1,        /* 定制化触发 */
              spec         : 1;        /* RF规格触发 */
} wlan_m2s_mode_stru;

/* action帧发送类型枚举 */
typedef enum {
    WLAN_M2S_ACTION_TYPE_SMPS      = 0,         /* action采用smps */
    WLAN_M2S_ACTION_TYPE_OPMODE    = 1,         /* action采用opmode */
    WLAN_M2S_ACTION_TYPE_NONE      = 2,         /* 切换不发action帧 */

    WLAN_M2S_ACTION_TYPE_BUTT
} wlan_m2s_action_type_enum;
typedef osal_u8 wlan_m2s_action_type_enum_uint8;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_types.h */
