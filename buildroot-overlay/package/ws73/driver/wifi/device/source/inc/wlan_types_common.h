/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header files shared by wlan_types.h and wlan_types_rom.h.
 * Create: 2021-09-18
 */

#ifndef __WLAN_TYPES_COMMON_H__
#define __WLAN_TYPES_COMMON_H__

#include "wlan_types.h"
#include "oal_types_device.h"
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

/* 认证的transaction number */
typedef enum {
    WLAN_AUTH_TRASACTION_NUM_ONE    = 0x0001,
    WLAN_AUTH_TRASACTION_NUM_TWO    = 0x0002,
    WLAN_AUTH_TRASACTION_NUM_THREE  = 0x0003,
    WLAN_AUTH_TRASACTION_NUM_FOUR   = 0x0004,

    WLAN_AUTH_TRASACTION_NUM_BUTT
} wlan_auth_transaction_number_enum;
typedef osal_u16 wlan_auth_transaction_number_enum_uint16;


/* Protection mode for MAC */
typedef enum {
    WLAN_PROT_NO,           /* Do not use any protection       */
    WLAN_PROT_ERP,          /* Protect all ERP frame exchanges */
    WLAN_PROT_HT,           /* Protect all HT frame exchanges  */
    WLAN_PROT_GF,           /* Protect all GF frame exchanges  */

    WLAN_PROT_BUTT
} wlan_prot_mode_enum;
typedef osal_u8 wlan_prot_mode_enum_uint8;

typedef enum {
    WLAN_RTS_RATE_MODE_REG,     /* 0: RTS速率 = PROT_DATARATE的配置                           */
    WLAN_RTS_RATE_MODE_DESC,    /* 1: RTS速率 = 硬件根据TX描述符计算出的值                    */
    WLAN_RTS_RATE_MODE_MIN,     /* 2: RTS速率 = min(PROT_DATARATE,硬件根据TX描述符计算出的值) */
    WLAN_RTS_RATE_MODE_MAX,     /* 3: RTS速率 = max(PROT_DATARATE,硬件根据TX描述符计算出的值) */
    WLAN_RTS_RATE_SELECT_MODE_BUTT
} wlan_rts_rate_select_mode_enum;

typedef enum {
    NARROW_BW_10M = 0x80,
    NARROW_BW_5M = 0x81,
    NARROW_BW_1M = 0x82,
    NARROW_BW_BUTT
} mac_narrow_bw_enum;
typedef osal_u8 mac_narrow_bw_enum_uint8;

typedef enum {
    WLAN_ADDBA_MODE_AUTO,
    WLAN_ADDBA_MODE_MANUAL,

    WLAN_ADDBA_MODE_BUTT
} wlan_addba_mode_enum;
typedef osal_u8 wlan_addba_mode_enum_uint8;


/* 频带能力枚举 */
typedef enum {
    WLAN_BAND_CAP_2G,        /* 只支持2G */
    WLAN_BAND_CAP_5G,        /* 只支持5G */
    WLAN_BAND_CAP_2G_5G,     /* 支持2G 5G */

    WLAN_BAND_CAP_BUTT
} wlan_band_cap_enum;
typedef osal_u8 wlan_band_cap_enum_uint8;

/* WIFI协议类型定义 */
/* 重要:代表VAP的preamble协议能力的使用该枚举，0表示long preamble; 1表示short preamble */
typedef enum {
    WLAN_LEGACY_11B_MIB_LONG_PREAMBLE    = 0,
    WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE   = 1
} wlan_11b_mib_preamble_enum;
typedef osal_u8 wlan_11b_mib_preamble_enum_uint8;

/*****************************************************************************
  3.3 HT枚举类型
*****************************************************************************/

typedef enum {
    WLAN_CH_SWITCH_DONE     = 0,   /* 信道切换已经完成，AP在新信道运行 */
    WLAN_CH_SWITCH_STATUS_1 = 1,   /* AP还在当前信道，准备进行信道切换(发送CSA帧/IE) */

    WLAN_CH_SWITCH_BUTT
} wlan_ch_switch_status_enum;
typedef osal_u8 wlan_ch_switch_status_enum_uint8;

typedef enum {
    WLAN_BW_SWITCH_DONE     = 0,    /* 频宽切换已完成 */
    WLAN_BW_SWITCH_40_TO_20 = 1,    /* 从40MHz带宽切换至20MHz带宽 */
    WLAN_BW_SWITCH_20_TO_40 = 2,    /* 从20MHz带宽切换至40MHz带宽 */

    /* 后续添加 */
    WLAN_BW_SWITCH_BUTT
} wlan_bw_switch_status_enum;
typedef osal_u8 wlan_bw_switch_status_enum_uint8;

typedef enum {
    WLAN_HT_MIXED_PREAMBLE          = 0,
    WLAN_HT_GF_PREAMBLE             = 1,

    WLAN_HT_PREAMBLE_BUTT
} wlan_ht_preamble_enum;
typedef osal_u8 wlan_ht_preamble_enum_uint8;

/*****************************************************************************
  3.4 算法宏,枚举类型
*****************************************************************************/
typedef enum {
    WLAN_NON_TXBF                   = 0,
    WLAN_EXPLICIT_TXBF              = 1,
    WLAN_LEGACY_TXBF                = 2,

    WLAN_TXBF_BUTT
} wlan_txbf_enum;
typedef osal_u8 wlan_txbf_enum_uint8;

/* Trig帧帧子类型 */
typedef enum {
    WLAN_HE_BASIC_TRIG              = 0,
    WLAN_BEAM_REPORT_POLL           = 1,
    WLAN_MU_BAR                     = 2,
    WLAN_MU_RTS                     = 3,
    WLAN_BUFFER_STATUS_REPORT_POLL  = 4,
    WLAN_GCR_MU_BAR                 = 5,
    WLAN_BW_QUERY_REPORT_POLL       = 6,
    WLAN_NDP_FEEDBACK_REPORT_POLL   = 7,

    WLAN_HE_TRIG_TYPE_BUTT
} wlan_frame_trig_type_enum;

/* Android P 增加，是否启动并发扫描标志位 */
typedef enum {
    WLAN_SCAN_FLAG_LOW_PRIORITY  = 0,
    WLAN_SCAN_FLAG_LOW_FLUSH     = 1,
    WLAN_SCAN_FLAG_AP            = 2,
    WLAN_SCAN_FLAG_RANDOM_ADDR   = 3,
    WLAN_SCAN_FLAG_LOW_SPAN      = 4, /* 并发扫描 */
    WLAN_SCAN_FLAG_LOW_POWER     = 5,
    WLAN_SCAN_FLAG_HIFH_ACCURACY = 6, /* 顺序扫描,非并发 */
    WLAN_SCAN_FLAG_BUTT
} wlan_scan_flag_enum;

/* 内核定义 flag标志位 */
typedef enum {
    WLAN_NL80211_SCAN_FLAG_BIT_LOW_PRIORITY   = 0,
    WLAN_NL80211_SCAN_FLAG_BIT_FLUSH          = 1,
    WLAN_NL80211_SCAN_FLAG_BIT_AP             = 2,
    WLAN_NL80211_SCAN_FLAG_BIT_RANDOM_ADDR    = 3,
    WLAN_NL80211_SCAN_FLAG_BIT_LOW_SPAN       = 8,
    WLAN_NL80211_SCAN_FLAG_BIT_LOW_POWER      = 9,
    WLAN_NL80211_SCAN_FLAG_BIT_HIGH_ACCURACY  = 10,
    WLAN_NL80211_SCAN_FLAG_BIT_BUTT
} wlan_nl80211_scan_flag_bit_enum;

/* 扫描结果枚举 */
typedef enum {
    WLAN_SCAN_EVENT_COMPLETE    = 0,
    WLAN_SCAN_EVENT_FAILED,
    WLAN_SCAN_EVENT_ABORT,  /* 强制终止，比如卸载 */
    WLAN_SCAN_EVENT_TIMEOUT,
    WLAN_SCAN_EVENT_BUTT
} dmac_scan_event_enum;
typedef osal_u8 wlan_scan_event_enum_uint8;

/*****************************************************************************
  3.7 加密枚举
*****************************************************************************/

/* pmf的能力 */
typedef enum {
    MAC_PMF_DISABLED  = 0, /* 不支持pmf能力 */
    MAC_PMF_ENABLED,       /* 支持pmf能力，且不强制 */
    MAC_PMF_REQUIRED,       /* 严格执行pmf能力 */

    MAC_PMF_BUTT
} wlan_pmf_cap_status;
typedef osal_u8 wlan_pmf_cap_status_uint8;

/*****************************************************************************
  3.8 linkloss场景枚举
*****************************************************************************/

/* linkloss场景枚举 */
typedef enum {
    WLAN_LINKLOSS_MODE_BT = 0,
    WLAN_LINKLOSS_MODE_DBAC,
    WLAN_LINKLOSS_MODE_NORMAL,

    WLAN_LINKLOSS_MODE_BUTT
} wlan_linkloss_mode_enum;
typedef osal_u8 wlan_linkloss_mode_enum_uint8;

typedef enum {
    WALN_LINKLOSS_SCAN_SWITCH_CHAN_DISABLE = 0,
    WALN_LINKLOSS_SCAN_SWITCH_CHAN_EN      = 1,

    WALN_LINKLOSS_SCAN_SWITCH_CHAN_BUTT
} wlan_linkloss_scan_switch_chan_enum;
typedef osal_u8 wlan_linkloss_scan_switch_chan_enum_uint8;

/*****************************************************************************
  3.9 roc场景枚举
*****************************************************************************/

typedef enum {
    IEEE80211_ROC_TYPE_NORMAL  = 0,
    IEEE80211_ROC_TYPE_MGMT_TX,
    IEEE80211_ROC_TYPE_BUTT
} wlan_ieee80211_roc_type;
typedef osal_u8 wlan_ieee80211_roc_type_uint8;

/*****************************************************************************
  3.10 roam场景枚举
*****************************************************************************/
/* 漫游切换状态 */
typedef enum {
    WLAN_ROAM_MAIN_BAND_STATE_2TO2          = 0,
    WLAN_ROAM_MAIN_BAND_STATE_5TO2          = 1,
    WLAN_ROAM_MAIN_BAND_STATE_2TO5          = 2,
    WLAN_ROAM_MAIN_BAND_STATE_5TO5          = 3,

    WLAN_ROAM_MAIN_BAND_STATE_BUTT
} wlan_roam_main_band_state_enum;
typedef osal_u8  wlan_roam_main_band_state_enum_uint8;

/*****************************************************************************
  3.11 m2s切换枚举
*****************************************************************************/
typedef enum {
    WLAN_SPECIAL_FRM_RTS,
    WLAN_SPECIAL_FRM_ONE_PKT,
    WLAN_SPECIAL_FRM_ABORT_SELFCTS,
    WLAN_SPECIAL_FRM_ABORT_CFEND,
    WLAN_SPECIAL_FRM_CFEND,
    WLAN_SPECIAL_FRM_NDP,
    WLAN_SPECIAL_FRM_VHT_REPORT,
    WLAN_SPECIAL_FRM_ABORT_NULL_DATA,
    WLAN_SPECIAL_FRM_BUTT
} wlan_special_frm_enum;
typedef osal_u8 wlan_special_frm_enum_uint8;

/*****************************************************************************
    STRUCT定义
*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_types_common.h */
