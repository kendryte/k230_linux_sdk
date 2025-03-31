/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of mac_vap_etc.c.
 * Create: 2020-7-8
 */
#ifndef __MAC_VAP_EXT_H__
#define __MAC_VAP_EXT_H__

/* 文件合一后去掉HMAC、DMAC两个宏 */
#include "mac_device_ext.h"
#include "mac_user_ext.h"
#include "hcc_types.h"
#include "hcc_host_if.h"
#include "oal_schedule.h"
#include "oal_fsm.h"
#include "oal_netbuf_ext.h"
#include "oam_ext_if.h"
#include "hmac_tcp_opt_struc.h"    // tcp_opt优化的特性移除之后，不在关联此头文件
#include "hal_ext_if.h"
#include "alg_common_rom.h"
#include "msg_vap_manage_rom.h"
#include "msg_wow_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* ****************************************************************************
    宏定义
**************************************************************************** */
#define MAC_NUM_DR_802_11A 8  /* 11A 5g模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11A 3  /* 11A 5g模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11A 5 /* 11A 5g模式时的非基本速率(NBR)个数 */

#define MAC_NUM_DR_802_11B 4  /* 11B 2.4G模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11B 2  /* 11B 2.4G模式时的数据速率(BR)个数 */
#define MAC_NUM_NBR_802_11B 2 /* 11B 2.4G模式时的数据速率(NBR)个数 */

#define MAC_NUM_DR_802_11G 8  /* 11G 2.4G模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11G 3  /* 11G 2.4G模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11G 5 /* 11G 2.4G模式时的非基本速率(NBR)个数 */

#define MAC_NUM_DR_802_11G_MIXED 12     /* 11G 混合模式时的数据速率(DR)个数 */
#define MAC_NUM_BR_802_11G_MIXED_ONE 4  /* 11G 混合1模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11G_MIXED_ONE 8 /* 11G 混合1模式时的非基本速率(NBR)个数 */

#define MAC_NUM_BR_802_11G_MIXED_TWO 7  /* 11G 混合2模式时的基本速率(BR)个数 */
#define MAC_NUM_NBR_802_11G_MIXED_TWO 5 /* 11G 混合2模式时的非基本速率(NBR)个数 */

/* 11N MCS相关的内容 */
#define MAC_MAX_RATE_SINGLE_NSS_20M_11N 0 /* 1个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_40M_11N 0 /* 1个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11N 0 /* 1个空间流80MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11N 0 /* 2个空间流20MHz的最大速率 */

/* 11AC MCS相关的内容 */
#define MAC_MAX_SUP_MCS7_11AC_EACH_NSS 0  /* 11AC各空间流支持的最大MCS序号，支持0-7 */
#define MAC_MAX_SUP_MCS8_11AC_EACH_NSS 1  /* 11AC各空间流支持的最大MCS序号，支持0-8 */
#define MAC_MAX_SUP_MCS9_11AC_EACH_NSS 2  /* 11AC各空间流支持的最大MCS序号，支持0-9 */
#define MAC_MAX_UNSUP_MCS_11AC_EACH_NSS 3 /* 11AC各空间流支持的最大MCS序号，不支持n个空间流 */

#ifdef _PRE_WLAN_FEATURE_1024QAM
#define MAC_MAX_RATE_SINGLE_NSS_20M_11AC 107   /* 1个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_SINGLE_NSS_40M_11AC 225   /* 1个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_SINGLE_NSS_80M_11AC 487   /* 1个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_SINGLE_NSS_160M_11AC 975  /* 1个空间流160MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11AC 216   /* 2个空间流20MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11AC 450   /* 2个空间流40MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_80M_11AC 975   /* 2个空间流80MHz的最大Long GI速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_160M_11AC 1950 /* 2个空间流160MHz的最大Long GI速率 */
#else
/* 按照协议要求(9.4.2.158.3章节)，修改为long gi速率 */
#define MAC_MAX_RATE_SINGLE_NSS_20M_11AC 86    /* 1个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_40M_11AC 180   /* 1个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_80M_11AC 390   /* 1个空间流80MHz的最大速率 */
#define MAC_MAX_RATE_SINGLE_NSS_160M_11AC 780  /* 1个空间流160MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_20M_11AC 173   /* 2个空间流20MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_40M_11AC 360   /* 2个空间流40MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_80M_11AC 780   /* 2个空间流80MHz的最大速率 */
#define MAC_MAX_RATE_DOUBLE_NSS_160M_11AC 1560 /* 2个空间流160MHz的最大速率 */
#endif

#define HMAC_FBT_RSSI_ADJUST_VALUE 100
#define HMAC_RSSI_SIGNAL_INVALID 0xff /* 上报RSSI无效值 */
#define HMAC_FBT_RSSI_MAX_VALUE 100

#define MAC_VAP_USER_HASH_INVALID_VALUE 0xFFFFFFFF                /* HSAH非法值 */
#define MAC_VAP_USER_HASH_MAX_VALUE (WLAN_ASSOC_USER_MAX_NUM * 2) /* 2为扩展因子 */

#define MAC_VAP_CAP_ENABLE 1
#define MAC_VAP_CAP_DISABLE 0

#define MAC_VAP_FEATURE_ENABLE 1
#define MAC_VAP_FEATRUE_DISABLE 0

#define MAC_DATA_CONTAINER_HEADER_LEN 4
#define MAC_DATA_CONTAINER_MAX_LEN 512
#define MAC_DATA_CONTAINER_MIN_LEN 8 /* 至少要包含1个事件 */

#define MAC_SEND_TWO_DEAUTH_FLAG 0xf000

#define MAC_DBB_SCALING_2G_RATE_NUM 12              /* 2G rate速率的个数 */
#define MAC_DBB_SCALING_5G_RATE_NUM 8               /* 2G rate速率的个数 */
#define MAC_DBB_SCALING_2G_RATE_OFFSET 0            /* 2G Rate dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_2G_HT20_MCS_OFFSET 12       /* 2G HT20 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_2G_HT40_MCS_OFFSET 20       /* 2G HT40 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_2G_HT40_MCS32_OFFSET 61     /* 2G HT40 mcs32 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_5G_RATE_OFFSET 28           /* 5G Rate dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_5G_HT20_MCS_OFFSET 40       /* 5G HT20 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_5G_HT20_MCS8_OFFSET 36      /* 5G HT20 mcs8 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_5G_HT40_MCS_OFFSET 48       /* 5G HT40 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_5G_HT40_MCS32_OFFSET 60     /* 5G HT40 mcs32 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_5G_HT80_MCS_OFFSET 66       /* 5G HT80 dbb scaling 索引偏移值 */
#define MAC_DBB_SCALING_5G_HT80_MCS0_DELTA_OFFSET 2 /* 5G HT80 mcs0/1 dbb scaling 索引偏移值回退值 */

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define MAC_2G_CHANNEL_NUM 13
#define MAC_NUM_2G_BAND 3            /* 2g band数 */
#define MAC_NUM_5G_BAND 7            /* 5g band数 */
#define MAC_EXT_PA_GAIN_MAX_LVL 2    /* 定制化外置PA最大档位数 */
#define CUS_RF_PA_BIAS_REG_NUM 43    /* 定制化RF部分PA偏置寄存器最大数 */
#define CUS_DY_CALI_PARAMS_NUM 14    /* 动态校准参数个数,2.4g 3个(ofdm 20/40 11b cw),5g 5*2个band */
#define CUS_DY_CALI_PARAMS_TIMES 3   /* 动态校准参数二次项系数个数 */
#define CUS_DY_CALI_DPN_PARAMS_NUM 4 /* 定制化动态校准2.4G DPN参数个数11b OFDM20/40 CW OR 5G 160/80/40/20 */
#define CUS_DY_CALI_NUM_5G_BAND 5    /* 动态校准5g band1 2&3 4&5 6 7 */
#define CUS_DY_CALI_2G_VAL_DPN_MAX (50)  /* 动态校准2g dpn读取nvram最大值 */
#define CUS_DY_CALI_2G_VAL_DPN_MIN (-50) /* 动态校准2g dpn读取nvram最小值 */
#define CUS_DY_CALI_5G_VAL_DPN_MAX (50)  /* 动态校准5g dpn读取nvram最大值 */
#define CUS_DY_CALI_5G_VAL_DPN_MIN (-50) /* 动态校准5g dpn读取nvram最小值 */
#define CUS_NUM_2G_DELTA_RSSI_NUM (2)    /* 20M/40M */
#define CUS_NUM_5G_DELTA_RSSI_NUM (4)    /* 20M/40M/80M/160M */
#endif

#ifdef _PRE_WLAN_ONLINE_DPD
#define MAC_DPD_CALI_CUS_PARAMS_NUM (5)
#endif

#define HMAC_RSP_MSG_MAX_LEN 128 /* get wid返回消息最大长度 */

#define MAC_VAP_INVAILD (0x0) /* 0为vap无效 */
#define MAC_VAP_VAILD (0x2b)

/* ****************************************************************************
    枚举定义
**************************************************************************** */
/* 功率设置维测命令类型枚举 */
typedef enum {
    MAC_SET_POW_RF_REG_CTL = 0, /* 功率是否RF寄存器控 */
    MAC_SET_POW_FIX_LEVEL,      /* 固定功率等级 */
    MAC_SET_POW_MAG_LEVEL,      /* 管理帧功率等级 */
    MAC_SET_POW_CTL_LEVEL,      /* 控制帧功率等级 */
    MAC_SET_POW_AMEND,          /* 修正UPC code */
    MAC_SET_POW_NO_MARGIN,      /* 功率不留余量 */
    MAC_SET_POW_SHOW_LOG,       /* 日志显示 */
    MAC_SET_POW_SAR_LVL_DEBUG,  /* 降sar  */

    MAC_SET_POW_BUTT
} mac_set_pow_type_enum;
typedef osal_u8 mac_set_pow_type_enum_uint8;

typedef enum {
    MAC_VAP_CONFIG_UCAST_DATA = 0,
    MAC_VAP_CONFIG_MCAST_DATA,
    MAC_VAP_CONFIG_BCAST_DATA,
    MAC_VAP_CONFIG_UCAST_MGMT_2G,
    MAC_VAP_CONFIG_UCAST_MGMT_5G,
    MAC_VAP_CONFIG_MBCAST_MGMT_2G,
    MAC_VAP_CONFIG_MBCAST_MGMT_5G,

    MAC_VAP_CONFIG_BUTT,
} mac_vap_config_dscr_frame_type_enum;
typedef osal_u8 mac_vap_config_dscr_frame_type_uint8;

typedef enum {
    MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST,
    MAC_DYN_CALI_CFG_SET_2G_DSCR_INT,
    MAC_DYN_CALI_CFG_SET_5G_DSCR_INT,
    MAC_DYN_CALI_CFG_SET_CHAIN_INT,
    MAC_DYN_CALI_CFG_SET_PDET_MIN_TH,
    MAC_DYN_CALI_CFG_SET_PDET_MAX_TH,
    MAC_DYN_CALI_CFG_BUFF,
} mac_dyn_cali_cfg_enum;
typedef osal_u8 mac_dyn_cali_cfg_enum_uint8;

typedef enum {
    SCAN_PARAM_START,
    SCAN_PARAM_TX_POWER,
    SCAN_PARAM_COUNT,
    SCAN_PARAM_STAY_TIME,
    SCAN_PARAM_ON_HOME_TIME,
    SCAN_PARAM_CHANNEL_LIST,
    SCAN_PARAM_INTERVAL,
    SCAN_PARAM_SCAN_MODE,
    SCAN_PARAM_NOT_SCAN,
    SCAN_PARAM_DEL_SCAN_LIST,
    SCAN_PARAM_MAX
} scan_param_cfg;

typedef enum {
    MAC_SET_BEACON = 0,
    MAC_ADD_BEACON = 1,

    MAC_BEACON_OPERATION_BUTT
} mac_beacon_operation_type;
typedef osal_u8 mac_beacon_operation_type_uint8;

typedef enum {
    MAC_WMM_SET_PARAM_TYPE_DEFAULT,
    MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA,

    MAC_WMM_SET_PARAM_TYPE_BUTT
} mac_wmm_set_param_type_enum;
typedef osal_u8 mac_wmm_set_param_type_enum_uint8;

#define MAC_VAP_AP_STATE_BUTT (MAC_VAP_STATE_AP_WAIT_START + 1)
#define MAC_VAP_STA_STATE_BUTT MAC_VAP_STATE_BUTT

#define H2D_SYNC_MASK_BARK_PREAMBLE (1 << 1)
#define H2D_SYNC_MASK_MIB (1 << 2)
#define H2D_SYNC_MASK_PROT (1 << 3)

#define WAL_HIPRIV_RATE_INVALID 255 /* 无效速率配置值 */

enum {
    PKT_STAT_SET_ENABLE = 0,
    PKT_STAT_SET_START_STAT = 1,
    PKT_STAT_SET_FRAME_LEN = 2,
    PKT_STAT_SET_BUTT
};
typedef osal_u8 oal_pkts_stat_enum_uint8;

enum {
    OAL_SET_ENABLE = 0,
    OAL_SET_MAX_BUFF = 1,
    OAL_SET_THRESHOLD = 2,
    OAL_SET_LOG_PRINT = 3,
    OAL_SET_CLEAR = 10,

    OAL_SET_BUTT
};
typedef osal_u8 oal_stat_enum_uint8;

typedef enum {
    /* 上报hal消息类型，定义和0506保持一致 */
    HMAC_CHAN_MEAS_INIT_REPORT                    = 0,       /* 初始化信息上报 */
    HMAC_CHAN_MEAS_LINK_INFO_REPORT               = 1,       /* 当前链路信息反馈 */
    HMAC_CHAN_MEAS_CHAN_STAT_REPORT               = 2,       /* 探测信道信息反馈 */
    HMAC_CHAN_MEAS_SUCC_REPORT                    = 3,       /* 切换成功反馈 */
    HMAC_CHAN_MEAS_EXTI_REPORT                    = 4,       /* 退出原因上报 */
    HMAC_CHBA_CHAN_SWITCH_REPORT             = 5,       /* 信道切换信息上报 */
    HMAC_CHBA_ROLE_REPORT                    = 6,       /* CHBA 角色信息上报 */
    HMAC_CHBA_GET_BEST_CHANNEL               = 7,       /* CHBA获取最优信道 */
    HMAC_CHBA_REPORT_TYPE_BUTT,
} hmac_info_report_type_enum;

#ifdef _PRE_WLAN_DFR_STAT
typedef struct {
    osal_u8 index;
    osal_u8 mode;
    osal_u8 resv[2]; /* 保留字节 */
} mac_cfg_queue_reset_stru;
#endif
#ifdef _PRE_WLAN_FEATURE_PK_MODE
typedef struct {
    osal_u32 high_th;
    osal_u32 low_th;
} mac_cfg_pk_mode_th_reset_stru;
#endif
typedef struct {
    osal_u8 ch;      /* 信道编号 */
    osal_s8 power;   /* 功率 */
    osal_u8 resv[2]; /* 2保留字节 */
} mac_cfg_adjust_tx_power_stru;

typedef struct {
    /* tx dscr tx_count0/1/2/3 */
    osal_u8 tx_count0;
    osal_u8 tx_count1;
    osal_u8 tx_count2;
    osal_u8 tx_count3;
} mac_hw_retry_num_stru;

typedef struct {
    osal_u32 rate;
    osal_u32 phy_mode;
    osal_u8 addr1[WLAN_MAC_ADDR_LEN];
} mac_cfg_rts_info;


typedef struct {
    osal_u8 ac;
    osal_u16 lifetime;
    osal_u8 resv; /* 1保留字节 */
} mac_cfg_lifetime;

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
typedef enum {
    MAC_WOW_DISABLE,
    MAC_WOW_ENABLE,
    MAC_WOW_EN_BUTT
} mac_wow_en_enum;

typedef enum {
    MAC_WOW_WAKEUP_NOTIFY,
    MAC_WOW_SLEEP_REQUEST,
    MAC_WOW_NOTIFY_TYPE_BUTT
} mac_wow_notify_type_enum;

typedef enum {
    MAC_WOW_FIELD_ALL_CLEAR = 0,             /* Clear all events */
    MAC_WOW_FIELD_MAGIC_PACKET = BIT0,       /* Wakeup on Magic Packet */
    MAC_WOW_FIELD_NETPATTERN_TCP = BIT1,     /* Wakeup on TCP NetPattern */
    MAC_WOW_FIELD_NETPATTERN_UDP = BIT2,     /* Wakeup on UDP NetPattern */
    MAC_WOW_FIELD_DISASSOC = BIT3,           /* 去关联/去认证，Wakeup on Disassociation/Deauth */
} mac_wow_field_enum;

typedef enum {
    MAC_WOW_WKUP_REASON_TYPE_NULL = 0,               /* None */
    MAC_WOW_WKUP_REASON_TYPE_MAGIC_PACKET = 1,       /* Wakeup on Magic Packet */
    MAC_WOW_WKUP_REASON_TYPE_NETPATTERN_TCP = 2,     /* Wakeup on TCP NetPattern */
    MAC_WOW_WKUP_REASON_TYPE_NETPATTERN_UDP = 3,     /* Wakeup on UDP NetPattern */
    MAC_WOW_WKUP_REASON_TYPE_DISASSOC_RX = 4,        /* 对端去关联/去认证，Wakeup on Disassociation/Deauth */
    MAC_WOW_WKUP_REASON_TYPE_DISASSOC_TX = 5,        /* 对端去关联/去认证，Wakeup on Disassociation/Deauth */
    MAC_WOW_WKUP_REASON_TYPE_BUT
} mac_wow_wakeup_reason_type_enum;
#endif

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
typedef enum {
    MAC_TCP_ACK_BUF_ENABLE,
    MAC_TCP_ACK_BUF_TIMEOUT,
    MAC_TCP_ACK_BUF_MAX,

    MAC_TCP_ACK_BUF_TYPE_BUTT
} mac_tcp_ack_buf_cfg_cmd_enum;
typedef osal_u8 mac_tcp_ack_buf_cfg_cmd_enum_uint8;
typedef struct {
    osal_s8 *string;
    mac_tcp_ack_buf_cfg_cmd_enum_uint8 tcp_ack_buf_cfg_id;
    osal_u8 auc_resv[3];
} mac_tcp_ack_buf_cfg_table_stru;
typedef struct {
    mac_tcp_ack_buf_cfg_cmd_enum_uint8 cmd;
    oal_bool_enum_uint8 enable;
    osal_u8 timeout_ms;
    osal_u8 count_limit;
} mac_cfg_tcp_ack_buf_stru;
#endif
typedef struct {
    osal_u8 type;
    osal_u8 chan_list[WLAN_MAX_CHANNEL_NUM];
    osal_u16 value;
    osal_u8 resv[2]; /* 2保留字节 */
} mac_cfg_set_scan_param;

typedef struct {
    osal_u8 scan_cnt;                  /* 扫描次数 */
    osal_u8 scan_time;                 /* 扫描驻留时间 */
    osal_u8 scan_channel_interval;     /* 间隔n个信道，切回工作信道工作一段时间 */
    osal_u8 work_time_on_home_channel; /* 背景扫描时，返回工作信道工作的时间 */
} mac_cfg_scan_param_stru;

typedef struct {
    osal_u16 txop_limit_cycle;
    osal_u8 resv[2]; /* 2保留字节 */
} mac_cfg_txop_limit_cycle;

typedef enum {
    WITP_WPA_VERSION_1 = 1,
    WITP_WPA_VERSION_2 = 2,
    WITP_WAPI_VERSION = 1 << 7,
} witp_wpa_versions_enum;

typedef struct {
    osal_char *car_name;
    mac_rssi_limit_type_enum_uint8 rssi_cfg_id;
    osal_u8 auc_resv[3]; /* 3字节对齐 */
} mac_rssi_cfg_table_stru;

static INLINE__ osal_u32 cipher_suite_selector(osal_u32 a, osal_u32 b, osal_u32 c, osal_u32 d)
{
    return (((osal_u32)d) << 24) | (((osal_u32)c) << 16) | (((osal_u32)b) << 8) | (osal_u32)a;
}
#define MAC_WPA_CHIPER_CCMP cipher_suite_selector(0x00, 0x50, 0xf2, 4)
#define MAC_WPA_CHIPER_TKIP cipher_suite_selector(0x00, 0x50, 0xf2, 2)
#define MAC_WPA_AKM_PSK cipher_suite_selector(0x00, 0x50, 0xf2, 2)
#define MAC_WPA_AKM_PSK_SHA256 cipher_suite_selector(0x00, 0x50, 0xf2, 6)
#define MAC_RSN_CHIPER_CCMP cipher_suite_selector(0x00, 0x0f, 0xac, 4)
#define MAC_RSN_CHIPER_TKIP cipher_suite_selector(0x00, 0x0f, 0xac, 2)
#define MAC_RSN_AKM_PSK cipher_suite_selector(0x00, 0x0f, 0xac, 2)
#define MAC_RSN_AKM_PSK_SHA256 cipher_suite_selector(0x00, 0x0f, 0xac, 6)
#define MAC_RSN_AKM_SAE_SHA256 cipher_suite_selector(0x00, 0x0f, 0xac, 8)
#define MAC_RSN_AKM_OWE cipher_suite_selector(0x00, 0x0f, 0xac, 18)
#define MAC_RSN_CIPHER_SUITE_AES_128_CMAC cipher_suite_selector(0x00, 0x0f, 0xac, 6)
#define MAC_RSN_CIPHER_SUITE_GCMP cipher_suite_selector(0x00, 0x0f, 0xac, 8)
#define MAC_RSN_CIPHER_SUITE_GCMP_256 cipher_suite_selector(0x00, 0x0f, 0xac, 9)
#define MAC_RSN_CIPHER_SUITE_CCMP_256 cipher_suite_selector(0x00, 0x0f, 0xac, 10)
#define MAC_RSN_CIPHER_SUITE_BIP_GMAC_128 cipher_suite_selector(0x00, 0x0f, 0xac, 11)
#define MAC_RSN_CIPHER_SUITE_BIP_GMAC_256 cipher_suite_selector(0x00, 0x0f, 0xac, 12)
#define MAC_RSN_CIPHER_SUITE_BIP_CMAC_256 cipher_suite_selector(0x00, 0x0f, 0xac, 13)

/* ****************************************************************************
    STRUCT定义
**************************************************************************** */
#define SHORTGI_CFG_STRU_LEN 4

typedef struct {
    frw_timeout_stru tbtt_timer;
    osal_u32 last_tbtt_time;
} hmac_sta_tbtt_stru;

typedef struct {
    oal_ieee80211_channel *channel;
    osal_u64 time;
    osal_u64 time_busy;
    osal_u64 time_ext_busy;
    osal_u64 time_rx;
    osal_u64 time_tx;
    osal_u64 time_scan;
    osal_u32 filled;
    osal_s8 noise;

    osal_u8 free_power_cnt;                           /* 信道空闲功率测量次数 */
    osal_s16 free_power_stats_20m;
} hmac_survey_info_stru;

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
#define CCPRIV_CMD_NAME_MAX_LEN 40 /* 字符串中每个单词的最大长度 */
#else
#define CCPRIV_CMD_NAME_MAX_LEN 80 /* 字符串中每个单词的最大长度(原20) */
#endif

#define CCPRIV_MAC_ADDR_STR_LEN 18 /* mac地址字符串长度 12位数字+5个分隔符+1个结束符 */

/* CCPRIV命令参数 */
typedef struct {
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN]; /* 命令名 */
    const osal_s8 *param; /* 参数 */
} hmac_ccpriv_para;

#ifdef _PRE_WLAN_FEATURE_ISOLATION
/* 隔离信息 */
typedef struct {
    osal_u8 single_type;    /* 单bss隔离类型   */
    osal_u8 multi_type;     /* 多bss隔离类型       */
    osal_u8 mode;           /* 隔离模式bit0：广播隔离 bit1：组播隔离 bit2：单播隔离   */
    osal_u8 forward;        /* forwarding方式 */
    osal_u32 counter_bcast; /* 广播隔离计数器 */
    osal_u32 counter_mcast; /* 组播隔离计数器 */
    osal_u32 counter_ucast; /* 单播隔离计数器 */
} mac_isolation_info_stru;
#endif /* _PRE_WLAN_FEATURE_CUSTOM_SECURITY */
#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
typedef struct {
    struct osal_list_head ipv4_head[MAC_VAP_USER_HASH_MAX_VALUE];
    struct osal_list_head ipv6_head[MAC_VAP_USER_HASH_MAX_VALUE];
    osal_u16 ipv4_num; /* 记录ipv4的条数 */
    osal_u16 ipv6_num; /* 记录ipv6的条数 */
    oal_bool_enum_uint8 is_proxyarp;
    oal_bool_enum_uint8 en_2wlan;
} mac_vap_proxyarp_stru;
#endif

typedef struct {
    wlan_bw_switch_status_enum_uint8 bw_switch_status;     /* 带宽切换状态 */
    wlan_channel_bandwidth_enum_uint8 user_pref_bandwidth;
    /* VAP为STA时，特有成员        */
    oal_bool_enum_uint8 te_b;
    osal_u8 wait_bw_change;         /* 收到action帧,等待切换带宽 */
    osal_u32 chan_report_for_te_a;
} mac_ch_switch_info_stru;

typedef struct {
    osal_u8 rs_nrates;   /* 速率个数 */
    osal_u8 auc_resv[3]; /* 保留3字节对齐 */
    mac_data_rate_stru rs_rates[WLAN_MAX_SUPP_RATES];
} mac_rateset_stru;

typedef struct {
    osal_u8 br_rate_num;  /* 基本速率个数 */
    osal_u8 nbr_rate_num; /* 非基本速率个数 */
    osal_u8 max_rate;     /* 最大基本速率 */
    osal_u8 min_rate;     /* 最小基本速率 */
    mac_rateset_stru rate;
} mac_curr_rateset_stru;

/* VAP收发包统计 */
typedef struct {
    /* net_device用统计信息, net_device统计经过以太网的报文 */
    osal_u32 rx_packets_to_lan;                /* 接收流程到LAN的包数 */
    osal_u32 rx_bytes_to_lan;                  /* 接收流程到LAN的字节数 */
    osal_u32 rx_dropped_packets;               /* 接收流程中丢弃的包数 */
    osal_u32 rx_vap_non_up_dropped;            /* vap没有up丢弃的包的个数 */
    osal_u32 rx_dscr_error_dropped;            /* 描述符出错丢弃的包的个数 */
    osal_u32 rx_first_dscr_excp_dropped;       /* 描述符首包异常丢弃的包的个数 */
    osal_u32 rx_alg_filter_dropped;            /* 算法过滤丢弃的包的个数 */
    osal_u32 rx_feature_ap_dropped;            /* AP特性帧过滤丢包个数 */
    osal_u32 rx_null_frame_dropped;            /* 收到NULL帧的数目 */
    osal_u32 rx_transmit_addr_checked_dropped; /* 发送端地址过滤失败丢弃 */
    osal_u32 rx_dest_addr_checked_dropped;     /* 目的地址过滤失败丢弃 */
    osal_u32 rx_multicast_dropped;             /* 组播帧失败(netbuf copy失败)丢弃 */

    osal_u32 tx_packets_from_lan; /* 发送流程LAN过来的包数 */
    osal_u32 tx_bytes_from_lan;   /* 发送流程LAN过来的字节数 */
    osal_u32 tx_dropped_packets;  /* 发送流程中丢弃的包数 */

    /* 其它报文统计信息 */
} mac_vap_stats_stru;

typedef struct {
    osal_u16 user_idx;
    wlan_protocol_enum_uint8 avail_protocol_mode; /* 用户协议模式 */
    wlan_protocol_enum_uint8 cur_protocol_mode;
    wlan_protocol_enum_uint8 protocol_mode;
    osal_u8 auc_resv[3];
} mac_h2d_user_protocol_stru;

typedef struct {
    osal_u16 user_idx;
    osal_u8 arg1;
    osal_u8 arg2;

    /* 协议模式信息 */
    wlan_protocol_enum_uint8 cur_protocol_mode;
    wlan_protocol_enum_uint8 protocol_mode;
    osal_u8 avail_protocol_mode; /* 用户和VAP协议模式交集, 供算法调用 */

    wlan_bw_cap_enum_uint8 bandwidth_cap;   /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 avail_bandwidth; /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 cur_bandwidth;   /* 默认值与avail_bandwidth相同,供算法调用修改 */

    oal_bool_enum_uint8 user_pmf;
    mac_user_asoc_state_enum_uint8 user_asoc_state; /* 用户关联状态 */
} mac_h2d_usr_info_stru;

typedef struct {
    osal_u16 user_idx;
    wlan_protocol_enum_uint8 avail_protocol_mode; /* 用户协议模式 */
    wlan_bw_cap_enum_uint8 bandwidth_cap;         /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 avail_bandwidth;       /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 cur_bandwidth;         /* 默认值与avail_bandwidth相同,供算法调用修改 */
    osal_u8 rsv[2];
} mac_h2d_user_bandwidth_stru;

typedef struct {
    oal_bool_enum_uint8 cap_11ax; /* 是否支持11ax能力 */
    oal_bool_enum_uint8 radar_detector_cap;
    oal_bool_enum_uint8 sounding_11n;
    wlan_bw_cap_enum_uint8 wlan_bw_max;

    osal_u16 beacon_period;
    oal_bool_enum_uint8 green_field;
    oal_bool_enum_uint8 mu_beamformee_cap;

    oal_bool_enum_uint8 txopps_is_supp;
    oal_bool_enum_uint8 su_bfee_num;
    oal_bool_enum_uint8 shortgi_40m;
    oal_bool_enum_uint8 txbf_11n;

    oal_bool_enum_uint8 enable_40m;
    osal_u8 rsv[3];
} mac_d2h_mib_update_info_stru;

typedef struct {
    osal_u8 queue_action;
} mac_d2h_syn_vap_flowctl_stru;

typedef struct {
    osal_u16 user_idx;
    mac_user_asoc_state_enum_uint8 asoc_state;
    osal_u8 rsv[1];
} mac_h2d_user_asoc_state_stru;

typedef struct {
    osal_u8 addr[WLAN_MAC_ADDR_LEN];
    osal_u8 pmkid[WLAN_PMKID_LEN];
    osal_u8 resv0[2];
} mac_pmkid_info_stu;

typedef struct {
    osal_u8 num_elems;
    osal_u8 resv0[3];
    mac_pmkid_info_stu elem[WLAN_PMKID_CACHE_SIZE];
} mac_pmkid_cache_stru;

#ifdef _PRE_WLAN_FEATURE_TXBF
typedef struct {
    osal_u8 imbf_receive_cap : 1, /* 隐式TxBf接收能力 */
        exp_comp_txbf_cap : 1,    /* 应用压缩矩阵进行TxBf的能力 */
        min_grouping : 2,         /* 0=不分组，1=1,2分组，2=1,4分组，3=1,2,4分组 */
        csi_bfee_max_rows : 2,    /* bfer支持的来自bfee的CSI显示反馈的最大行数 */
        channel_est_cap : 2;      /* 信道估计的能力，0=1空时流，依次递增 */
} mac_vap_txbf_add_stru;
#endif

typedef struct {
    /* word 0 */
    wlan_prot_mode_enum_uint8 protection_mode; /* 保护模式 */
    osal_u8 obss_non_erp_aging_cnt;            /* 指示OBSS中non erp 站点的老化时间 */
    osal_u8 obss_non_ht_aging_cnt;             /* 指示OBSS中non ht 站点的老化时间 */
    osal_u8 auto_protection : 1; /* 指示保护策略是否开启，OAL_SWITCH_ON 打开， OAL_SWITCH_OFF 关闭 */
    osal_u8 obss_non_erp_present : 1; /* 指示obss中是否存在non ERP的站点 */
    osal_u8 obss_non_ht_present : 1;  /* 指示obss中是否存在non HT的站点 */
    osal_u8 lsig_txop_protect_mode : 1; /* 指示L-SIG protect是否打开, OAL_SWITCH_ON 打开， OAL_SWITCH_OFF 关闭 */
    osal_u8 reserved : 4;

    /* word 1 */
    osal_u8 sta_no_short_slot_num;     /* 不支持short slot的STA个数 */
    osal_u8 sta_no_short_preamble_num; /* 不支持short preamble的STA个数 */
    osal_u8 sta_non_erp_num;           /* 不支持ERP的STA个数 */
    osal_u8 sta_non_ht_num;            /* 不支持HT的STA个数 */
    /* word 2 */
    osal_u8 sta_non_gf_num;        /* 支持ERP/HT,不支持GF的STA个数 */
    osal_u8 sta_20m_only_num;      /* 只支持20M 频段的STA个数 */
    osal_u8 sta_no_40dsss_cck_num; /* 不用40M DSSS-CCK STA个数  */
    osal_u8 sta_no_lsig_txop_num;  /* 不支持L-SIG TXOP Protection STA个数 */
} mac_protection_stru;

/* 用于同步带宽切换的参数 */
typedef struct {
    wlan_channel_bandwidth_enum_uint8 bandwidth_40m;
    oal_bool_enum_uint8 intol_user_40m;
    osal_u8 auc_resv[2];
} mac_bandwidth_stru;

typedef struct {
    osal_u8 *ie;     /* APP 信息元素 */
    osal_u16 ie_len; /* APP 信息元素长度 */
    osal_u8 ie_type_bitmap;
    osal_u8 resv[1];
} mac_app_ie_stru;

/* 协议参数 对应cfgid: WLAN_CFGID_MODE */
typedef struct {
    wlan_protocol_enum_uint8 protocol;              /* 协议 */
    wlan_channel_band_enum_uint8 band;              /* 频带 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    osal_u8 channel_idx;                            /* 主20M信道号 */
} mac_cfg_mode_param_stru;

typedef struct {
    wlan_channel_band_enum_uint8 band;              /* 频带 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
} mac_cfg_mib_by_bw_param_stru;

typedef struct {
    osal_u8 dscp;
    osal_u8 tid;
    osal_u8 reserve[2];
} mac_map_dscp_to_tid_stru;

typedef struct {
    osal_u8 flags;
    osal_u8 mcs;
    osal_u16 legacy;
    osal_u8 nss;
    osal_u8 bw;
    osal_u8 rsv[3];
} mac_rate_info_stru;

typedef enum mac_rate_info_flags {
    MAC_RATE_INFO_FLAGS_MCS = (1UL << 0),
    MAC_RATE_INFO_FLAGS_VHT_MCS = (1UL << 1),
    MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH = (1UL << 2),
    MAC_RATE_INFO_FLAGS_80_MHZ_WIDTH = (1UL << 3),
    MAC_RATE_INFO_FLAGS_80P80_MHZ_WIDTH = (1UL << 4),
    MAC_RATE_INFO_FLAGS_160_MHZ_WIDTH = (1UL << 5),
    MAC_RATE_INFO_FLAGS_SHORT_GI = (1UL << 6),
    MAC_RATE_INFO_FLAGS_60G = (1UL << 7),
} mac_rate_info_flags;

typedef struct {
    oal_bool_enum_uint8 open;    /* 打开关闭此特性 */
    mac_narrow_bw_enum_uint8 bw; /* 1M,5M,10M */
    osal_u8 rsv[2];
} mac_cfg_narrow_bw_stru;

typedef struct {
    oal_bool_enum_uint8 rate_cfg_tag;
    oal_bool_enum_uint8 tx_ampdu_last;
    osal_u8 reserv[2];
} mac_fix_rate_cfg_para_stru;

typedef struct {
    osal_u8 new_bss_color_info  : 6,
            new_bss_color_exit  : 2;
    osal_u8 rsv[3];
} mac_vap_he_config_info_stru;

#ifdef _PRE_WLAN_FEATURE_WPA3
typedef struct {
    osal_spinlock lock;
    struct osal_list_head pmksa_list_head;
    osal_u32 pmksa_num;
} hmac_sae_pmksa_cache_stru;

typedef struct {
    osal_spinlock lock;
    struct osal_list_head dlist_head;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8 pmkid[WLAN_PMKID_LEN];
    osal_u32 timestamp;
    osal_u16 reverse;
} hmac_sae_pmksa_entry_info;

/* The driver reports an event to trigger WPA to start SAE authentication. */
/* 驱动上报事件触发WPA开始SAE认证 */
typedef struct {
    osal_u8 bssid[WLAN_MAC_ADDR_LEN];
    osal_u8 *ssid;
    osal_u32 ssid_len;
    osal_u32 key_mgmt_suite;
    osal_u16 status;
    osal_u8 pmkid[WLAN_PMKID_LEN];
    osal_u8 have_pmkid;
} hmac_external_auth_stru;

typedef struct {
    osal_u8 peer[WLAN_MAC_ADDR_LEN];
    osal_u16 status;
    osal_u8 *ie;
    osal_u32 ie_len;
} hmac_owe_info;
#endif

typedef struct {
    osal_u8 linkloss_threshold;     /* LinkLoss门限 */
    osal_u16 link_loss;             /* LinkLoss计数器 */
    osal_u8 int_linkloss_threshold; /* beacon计算初始值 */
} vap_linkloss_stru;

typedef struct {
    osal_u8 send_proberequest_ratio; /* 表示linkloss计数达到阈值的（n/10）时，开始发送探测帧保活. */
    osal_u8 linkloss_threshold;   /* 表示linkloss阈值基础时间为(100*n)ms. */
    osal_u8 resv[2];
} wifi_linkloss_params;

/* 按照不同聚合长度分类的索引的枚举值 */
/* 0:1~14 */
/* 1:15~17 */
/* 2:18~30 */
/* 3:31~32 */
typedef enum {
    HMAC_COUNT_BY_AMPDU_LENGTH_INDEX_0,
    HMAC_COUNT_BY_AMPDU_LENGTH_INDEX_1,
    HMAC_COUNT_BY_AMPDU_LENGTH_INDEX_2,
    HMAC_COUNT_BY_AMPDU_LENGTH_INDEX_3,
    HMAC_COUNT_BY_AMPDU_LENGTH_INDEX_BUTT
} hmac_count_by_ampdu_length_enum;

typedef struct {
    vap_linkloss_stru linkloss_info[WLAN_LINKLOSS_MODE_BUTT]; /* linkloss计数结构 */
    osal_u32 dot11_beacon_period;                             /* 记录dot11BeaconPeriod是否变化 */
    osal_u8 linkloss_times; /* 记录linkloss当前门限值相对于正常的倍数，反映beacon的周期倍数 */
    osal_u8 state;          /* 记录状态 */
    osal_u8 send_probe_req_period; /* linkloss发送probe req频率 */
    osal_u8 linkloss_mode : 2;     /* linkloss场景 */
    osal_u8 roam_scan_done : 1;    /* 记录是否发生过漫游扫描 */
    osal_u8 linkloss_rate : 5; /* 记录当前linkloss的已丢失cnt占门限的比例，同步给device侧算法使用 */
    osal_u32 last_active_time_h32;  /* 记录上次活跃时的linkloss时间戳高32位 */
    osal_u32 last_active_time_l32;  /* 记录上次活跃时的linkloss时间戳低32位 */
} hmac_vap_linkloss_stru;

typedef struct {
    osal_s16 signal;           /* 驱动接收包记录的RSSI值 */
    osal_char snr_ant0;        /* 天线0上上报的SNR */
    osal_char snr_ant1;        /* 天线1上上报的SNR */
    osal_s16 signal_mgmt_data; /* 管理帧和数据帧的RSSI平滑值 */

    /* **************************************************************************
                                接收包统计
    ************************************************************************** */
    /* 数据帧统计 */
    osal_u32 rx_d_from_hw_pkts;   /* 驱动接收数据包数目 */
    osal_u32 rx_rcv_data_bytes;   /* 驱动接收字节数，不包括80211头尾 */
    osal_u32 rx_d_send_hmac_fail; /* DMAC发送给HMAC的失败netbuf数目 */
} hmac_vap_query_stats_stru;

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#define FREE_ARP_INTERVAL_DEFAULT_VALUE 2
#endif

/* Update join req 需要配置的速率集参数 */
typedef struct {
    union {
        osal_u8 value;
        struct {
            osal_u8 support_11b : 1;  /* 该AP是否支持11b */
            osal_u8 support_11ag : 1; /* 该AP是否支持11ag */
            osal_u8 ht_capable : 1;   /* 是否支持ht */
            osal_u8 vht_capable : 1;  /* 是否支持vht */
            osal_u8 reserved : 4;
        } capable; /* 与dmac层wlan_phy_protocol_enum对应 */
    } capable_flag;
    osal_u8 min_rate[2]; /* 第1个存储11b协议对应的速率，第2个存储11ag协议对应的速率 */
    osal_u8 reserved1;
} hmac_set_rate_stru;

/* 启用VAP参数结构体 对应cfgid: WLAN_CFGID_START_VAP */
typedef struct {
    oal_bool_enum_uint8 mgmt_rate_init_flag; /* start vap时候，管理帧速率是否需要初始化 */
    osal_u8 protocol;
    osal_u8 band;
    osal_u8 bandwidth;

#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode;
    osal_u8 resv2[3];
#endif
    osal_s32 ifindex;
    oal_net_device_stru *net_dev; /* 此成员仅供Host(WAL&HMAC)使用，Device侧(DMAC&ALG&HAL层)不使用 */
} mac_cfg_start_vap_param_stru;
typedef mac_cfg_start_vap_param_stru mac_cfg_down_vap_param_stru;

/* CFG VAP h2d */
typedef struct {
    oal_net_device_stru *net_dev;
} mac_cfg_vap_h2d_stru;

/* 设置mac地址参数 对应cfgid: WLAN_CFGID_STATION_ID */
typedef struct {
    osal_u8 station_id[WLAN_MAC_ADDR_LEN];
    osal_u8 auc_resv[2];
} mac_cfg_staion_id_param_stru;

/* SSID参数 对应cfgid: WLAN_CFGID_SSID */
typedef struct {
    osal_u8 ssid_len;
    osal_u8 auc_resv[2];
    osal_char ac_ssid[WLAN_SSID_MAX_LEN];
} mac_cfg_ssid_param_stru;

/* 获取的字符串参数 */
typedef struct {
    osal_s32 buff_len;
    osal_u8 buff[WLAN_IWPRIV_MAX_BUFF_LEN];
} mac_cfg_param_char_stru;

/* HOSTAPD 设置工作频段，信道和带宽参数 */
typedef struct {
    wlan_channel_band_enum_uint8 band;              /* 频带 */
    wlan_channel_bandwidth_enum_uint8 en_bandwidth; /* 带宽 */
    osal_u8 channel;                                /* 信道编号 */
    osal_u8 auc_resv[1];                            /* 保留位 */
} mac_cfg_channel_param_stru;

/* HOSTAPD 设置wiphy 物理设备信息，包括RTS 门限值，分片报文门限值 */
typedef struct {
    osal_u8 frag_threshold_changed;
    osal_u8 rts_threshold_changed;
    osal_u8 rsv[2];
    osal_u32 frag_threshold;
    osal_u32 rts_threshold;
} mac_cfg_wiphy_param_stru;

/* HOSTAPD 设置 Beacon 信息 */
typedef struct {
    osal_s32 interval;    /* beacon interval */
    osal_s32 dtim_period; /* DTIM period     */
    oal_bool_enum_uint8 privacy;
    osal_u8 rsn_ie[MAC_MAX_RSN_LEN];
    osal_u8 wpa_ie[MAC_MAX_RSN_LEN];
    oal_bool_enum_uint8 hidden_ssid;
    oal_bool_enum_uint8 shortgi_20;
    oal_bool_enum_uint8 shortgi_40;
    oal_bool_enum_uint8 shortgi_80;
    wlan_protocol_enum_uint8 protocol;
#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    osal_u8 crypto_mode;                             /* WPA/WPA2 */
    osal_u8 group_crypto;                            /* 组播密钥类型 */
    osal_u8 auth_type[MAC_AUTHENTICATION_SUITE_NUM]; /* akm 类型 */
    osal_u8 pairwise_crypto_wpa[MAC_PAIRWISE_CIPHER_SUITES_NUM];
    osal_u8 pairwise_crypto_wpa2[MAC_PAIRWISE_CIPHER_SUITES_NUM];
    osal_u16 rsn_capability;
#endif
    osal_u8 smps_mode;
    mac_beacon_operation_type_uint8 operation_type;
    osal_u8 sae_pwe;
    osal_u8 auc_resv1[1];
} mac_beacon_param_stru;

/* 设置log模块开关的配置命令参数 */
typedef struct {
    osal_u16 mod_id;                 /* 对应的模块id */
    oal_bool_enum_uint8 switch_code; /* 对应的开关设置 */
    osal_u8 auc_resv[1];
} mac_cfg_log_module_param_stru;

/* 用户相关的配置命令参数 */
typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    oal_bool_enum_uint8 ht_cap;          /* ht能力 */
    osal_u8 auc_resv[3];
    osal_u16 user_idx; /* 用户索引 */
} mac_cfg_add_user_param_stru;

typedef mac_cfg_add_user_param_stru mac_cfg_del_user_param_stru;

/* 接收帧的统计信息 */
typedef struct {
    osal_u32 data_op;    /* 数据操作模式: 0:保留 1:清除 */
    osal_u32 print_info; /* 打印数据内容:
                               0:所有数据 1:总帧数 2:self fcs correct
                               3:other fcs correct 4:fcs error */
} mac_cfg_al_rx_info_stru;

/* 剔除用户配置命令参数 */
typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    osal_u16 reason_code;                /* 去关联 reason code */
} mac_cfg_kick_user_param_stru;

/* 暂停tid配置命令参数 */
typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    osal_u8 tid;
    osal_u8 is_paused;
} mac_cfg_pause_tid_param_stru;

/* 配置用户是否为vip */
typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    osal_u8 vip_flag;
} mac_cfg_user_vip_param_stru;

#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
/* 设置host某个队列的每次调度报文个数，low_waterline, high_waterline */
typedef struct {
    osal_u8 queue_type;
    osal_u8 auc_resv[1];
    osal_u16 burst_limit;
    osal_u16 low_waterline;
    osal_u16 high_waterline;
} mac_cfg_flowctl_param_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
/* TSPEC相关的配置命令参数 */
typedef struct {
    mac_ts_info_stru ts_info;
    osal_u8 rsvd;
    osal_u16 norminal_msdu_size;
    osal_u16 max_msdu_size;
    osal_u32 min_data_rate;
    osal_u32 mean_data_rate;
    osal_u32 peak_data_rate;
    osal_u32 min_phy_rate;
    osal_u16 surplus_bw;
    osal_u16 medium_time;
} mac_cfg_wmm_tspec_stru_param_stru;

typedef struct {
    oal_switch_enum_uint8 wmm_ac_switch;
    oal_switch_enum_uint8 auth_flag; /* WMM AC认证开关 */
    osal_u16 timeout_period;         /* 定时器超时时间 */
    osal_u8 factor;                  /* 除数因子，medium_time/2^factor */
    osal_u8 rsv[3];
} mac_cfg_wmm_ac_param_stru;

#endif /* _PRE_WLAN_FEATURE_WMMAC */

typedef struct {
    osal_u8 pcie_pm_level; /* pcie低功耗级别: 0:normal 1:L0S 2:L1 3:L1PM 4:L1s */
    osal_u8 resv[3];
} mac_cfg_pcie_pm_level_stru;

/* 使能qempty命令 */
typedef struct {
    osal_u8 is_on;
    osal_u8 auc_resv[3];
} mac_cfg_resume_qempty_stru;

/* 发送mpdu/ampdu命令参数  */
typedef struct {
    osal_u8 tid;
    osal_u8 packet_num;
    osal_u16 packet_len;
    osal_u8 ra_mac[OAL_MAC_ADDR_LEN];
} mac_cfg_mpdu_ampdu_tx_param_stru;

typedef struct {
    osal_u8 show_profiling_type; /* 0:Rx 1: Tx */
    osal_u8 show_level; /* 0:分段输出正确的 1:每个节点、分段都输出正确的 2每个节点、分段都输出所有的 */
} mac_cfg_show_profiling_param_stru;

/* AMPDU相关的配置命令参数 */
typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    osal_u8 tidno;                       /* 对应的tid号 */
    osal_u8 reserve[1];                  /* 确认策略 */
} mac_cfg_ampdu_start_param_stru;

typedef mac_cfg_ampdu_start_param_stru mac_cfg_ampdu_end_param_stru;

/* BA会话相关的配置命令参数 */
typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* 用户的MAC ADDR */
    osal_u8 tidno;                       /* 对应的tid号 */
    mac_ba_policy_enum_uint8 ba_policy;  /* BA确认策略 */
    osal_u16 buff_size;                  /* BA窗口的大小 */
    osal_u16 timeout;                    /* BA会话的超时时间 */
} mac_cfg_addba_req_param_stru;

typedef struct {
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN];      /* 用户的MAC ADDR */
    osal_u8 tidno;                            /* 对应的tid号 */
    mac_delba_initiator_enum_uint8 direction; /* 删除ba会话的发起端 */
    mac_delba_trigger_enum_uint8 trigger;     /* 删除原因 */
} mac_cfg_delba_req_param_stru;

typedef struct {
    osal_u8 mac_addr[6];
    osal_u8 amsdu_max_num; /* amsdu最大个数 */
    osal_u8 reserve[3];
    osal_u16 amsdu_max_size; /* amsdu最大长度 */
} mac_cfg_amsdu_start_param_stru;

/* 发送功率配置参数 */
typedef struct {
    mac_set_pow_type_enum_uint8 type;
    osal_u8 reserve;
    osal_u8 auc_value[18];
} mac_cfg_set_tx_pow_param_stru;

/* 设置用户配置参数 */
typedef struct {
    osal_u8 function_index;
    osal_u8 reserve[2];
    mac_vap_config_dscr_frame_type_uint8 pkt_type; /* 配置的帧类型 */
    osal_s32 value;
} mac_cfg_set_dscr_param_stru;

/* non-HT协议模式下速率配置结构体 */
typedef struct {
    wlan_legacy_rate_value_enum_uint8 rate;     /* 速率值 */
    wlan_phy_protocol_enum_uint8 protocol_mode; /* 对应的协议 */
    osal_u8 reserve[2];                         /* 保留 */
} mac_cfg_non_ht_rate_stru;

/* 配置发送描述符内部元素结构体 */
typedef enum {
    RF_PAYLOAD_ALL_ZERO = 0,
    RF_PAYLOAD_ALL_ONE,
    RF_PAYLOAD_ALL_AA,
    RF_PAYLOAD_RAND,
    RF_PAYLOAD_DATA,
    RF_PAYLOAD_BUTT
} mac_rf_payload_enum;
typedef osal_u8 mac_rf_payload_enum_uint8;

typedef struct {
    osal_u8 param; /* 查询或配置信息 */
    wlan_phy_protocol_enum_uint8 protocol_mode;
    mac_rf_payload_enum_uint8 payload_flag;
    wlan_tx_ack_policy_enum_uint8 ack_policy;
    osal_u32 payload_len;
    osal_u32 al_tx_max; /* 常发发送的最大个数 */
    osal_u8 *payload;   /* RF_PAYLOAD_DATA使用，记录输入payload字段 */
    osal_u32 pkt_len;   /* RF_PAYLOAD_DATA使用，记录输入字段长度 */
    osal_u8 tpc_code;   /* 软件常发时填写的tpc_code */
} mac_cfg_tx_comp_stru;

typedef struct {
    osal_u32 length;
    osal_u32 dieid[WLAN_DIEID_MAX_LEN];
} mac_cfg_show_dieid_stru;

/* 配置硬件常发 */
typedef struct {
    osal_u32 devid;
    osal_u32 mode;
    osal_u32 rate;
} mac_cfg_al_tx_hw_cfg_stru;

typedef enum {
    HW_PAYLOAD_ALL_ZERO = 0,
    HW_PAYLOAD_ALL_ONE,
    HW_PAYLOAD_RAND,
    HW_PAYLOAD_REPEAT,
    HW_PAYLOAD_BUTT
} mac_hw_payload_enum;
typedef osal_u8 mac_hw_payload_enum_uint8;

/* 硬件常发开关 */
typedef struct {
    osal_u8  switch_code : 1,
             dev_id : 1,
             flag   : 2, /* 常发帧的内容: 0:全0 1:全1 2:随机 3:重复content的数据 */
        rsv : 4;
    osal_u8 content;
    osal_u8 rsv1[2];
    osal_u32 len;
    osal_u32 times; /* 次数 0为无限次 */
    osal_u32 ifs;   /* 常发帧间隔,单位0.1us */
} mac_cfg_al_tx_hw_stru;

typedef struct {
    osal_u8 offset_addr_a;
    osal_u8 offset_addr_b;
    osal_u16 delta_gain;
} mac_cfg_dbb_scaling_stru;

/* 频偏较正命令格式 */
typedef struct {
    osal_u16 idx;             /* 全局数组索引值 */
    osal_u16 chn;             /* 配置信道 */
    osal_s16 as_corr_data[8]; /* 校正数据 */
} mac_cfg_freq_skew_stru;

/* wfa edca参数配置 */
typedef struct {
    oal_bool_enum_uint8 switch_code; /* 开关 */
    wlan_wme_ac_type_enum_uint8 ac;  /* AC */
    osal_u16 val;                    /* 数据 */
} mac_edca_cfg_stru;

/* PPM调整命令格式 */
typedef struct {
    osal_char ppm_val;  /* PPM差值 */
    osal_u8 clock_freq; /* 时钟频率 */
    osal_u8 resv[1];
} mac_cfg_adjust_ppm_stru;

/* 用户信息相关的配置命令参数 */
typedef struct {
    osal_u16 user_idx; /* 用户索引 */
    osal_u8 reserve[2];
} mac_cfg_user_info_param_stru;

/* 获取当前管制域国家码字符配置命令结构体 */
typedef struct {
    osal_char ac_country[3];
    osal_u8 auc_resv[1];
} mac_cfg_get_country_stru;

/* query消息格式:2字节WID x N */
typedef struct {
    wlan_tidno_enum_uint8 tid;
    osal_u8 resv[3];
} mac_cfg_get_tid_stru;

typedef struct {
    osal_u16 user_id;
    osal_s8 rssi;
    osal_s8 free_power;
} mac_cfg_query_rssi_stru;

typedef struct {
    osal_u16 user_id;
    osal_u8 ps_st;
    osal_u8 auc_resv[1];
} mac_cfg_query_psst_stru;

typedef struct {
    osal_u16 user_id;
    osal_u8 auc_resv[2];
    osal_u32 tx_dropped[WLAN_WME_AC_BUTT];
} mac_cfg_query_drop_num_stru;

typedef struct {
    osal_u16 user_id;
    osal_u8 auc_resv[2];
    osal_u32 max_tx_delay; /* 最大发送延时 */
    osal_u32 min_tx_delay; /* 最小发送延时 */
    osal_u32 ave_tx_delay; /* 平均发送延时 */
} mac_cfg_query_tx_delay_stru;

typedef struct {
    osal_u8 vap_id;
    osal_u8 bsd;
    osal_u8 auc_resv[2];
} mac_cfg_query_bsd_stru;

#ifdef _PRE_WLAN_DFT_STAT
typedef struct {
    osal_u8 device_distance;
    osal_u8 intf_state_cca;
    osal_u8 intf_state_co;
    osal_u8 auc_resv[1];
} mac_cfg_query_ani_stru;
#endif

typedef struct {
    osal_u8 link_meas_cmd_type;
    osal_u8 scan_chan;
    osal_u8 scan_band;
    osal_u8 rsv;
    osal_u16 scan_time;
    osal_u16 scan_interval;
} mac_cfg_link_meas_stru;

typedef struct {
    /* 实时的信息，从描述符中获取 */
    osal_u8 channel_number;
    osal_u8 preabmle;
    osal_u8 gi_type;
    osal_u8 nss_rate : 6;
    osal_u8 nss_mode : 2;

    osal_u8 protocol_type;
    osal_s8 rssi_dbm;
    osal_s8 snr_ant0;
    osal_s8 snr_ant1;

    osal_u8 freq_bw;
    osal_u8 freq_bandwidth_mode;
    osal_u16 data_cnt;
} mac_cfg_rx_user_rate_stru;

#ifdef _PRE_WLAN_DFT_STAT
typedef struct {
    osal_u32 tx_mpdu_num;
    osal_u32 tx_msdu_num;
    osal_u32 rx_mpdu_num;
    osal_u32 rx_msdu_num;
} wlan_aggregation_stats_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
typedef struct {
    osal_u32 cfg_id;
    osal_u32 ac;
    osal_u32 value;
} mac_cfg_set_smps_mode_stru;
#endif

/* 以下为解析内核配置参数转化为驱动内部参数下发的结构体 */

/* 解析内核配置的扫描参数后，下发给驱动的扫描参数 */
typedef struct {
    oal_ssids_stru ssids[WLAN_SCAN_REQ_MAX_SSID];
    osal_s32 ssid_num;
    osal_u8 *ie;
    osal_u32 ie_len;

    oal_scan_enum_uint8 scan_type;
    osal_u8 num_channels_2g;
    osal_u8 num_channels_5g;
    osal_u8 num_channels_all;
    osal_u32 *channels_2g;
    osal_u32 *channels_5g;
    osal_u8 rsv_1;
#ifdef _PRE_WLAN_FEATURE_ACS
    osal_u8 acs_scan_flag;
#endif

    /* WLAN/P2P 特性情况下，p2p0 和p2p-p2p0 cl 扫描时候，需要使用不同设备，增加is_p2p0_scan来区分 */
    osal_u8 is_p2p0_scan; /* 是否为p2p0 发起扫描 */
#ifdef _PRE_WLAN_FEATURE_ACS
    osal_u8 rsv; /* 保留位 */
#else
    osal_u8 rsv[2];  /* 保留位 */
#endif
} mac_cfg80211_scan_param_stru;

/* 解析内核配置的connect参数后，下发给驱动的connect参数 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
typedef struct {
    osal_u32 wpa_versions;
    osal_u32 group_suite;
    osal_u32 pair_suite[WLAN_PAIRWISE_CIPHER_SUITES];
    osal_u32 akm_suite[WLAN_AUTHENTICATION_SUITES];
    osal_u32 group_mgmt_suite;
    osal_u32 have_pmkid; /* WPA3关联判断是否是重关联 */
} mac_crypto_settings_stru;
#endif
#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
typedef struct {
    td_u32 wpa_versions;
    td_u32 group_suite;
    td_u32 n_ciphers_pairwise;
    td_u32 pair_suite[WLAN_PAIRWISE_CIPHER_SUITES];
    td_u32 n_akm_suites;
    td_u32 akm_suite[WLAN_AUTHENTICATION_SUITES];
    td_u32 group_mgmt_suite;
    td_u32 control_port;
    td_u32 have_pmkid; /* WPA3关联判断是否是重关联 */
} mac_crypto_settings_stru;
#endif

typedef struct {
    osal_u8 channel;  /* ap所在信道编号，eg 1,2,11,36,40... */
    osal_u8 ssid_len; /* SSID 长度 */
    mac_nl80211_mfp_enum_uint8 mfp;
    osal_u8 wapi;

    osal_u8 *ie;
    osal_u8 *ssid;  /* 期望关联的AP SSID  */
    osal_u8 *bssid; /* 期望关联的AP BSSID  */

    oal_bool_enum_uint8 privacy;                /* 是否加密标志 */
    oal_nl80211_auth_type_enum_uint8 auth_type; /* 认证类型，OPEN or SHARE-KEY */

    osal_u8 wep_key_len;             /* WEP KEY长度 */
    osal_u8 wep_key_index;           /* WEP KEY索引 */
    const osal_u8 *wep_key;          /* WEP KEY密钥 */
    mac_crypto_settings_stru crypto; /* 密钥套件信息 */
    osal_u32 ie_len;
#ifdef _PRE_WLAN_FEATURE_WPA3
    osal_u8 sae_pwe;
    osal_u8 rsv[3]; /* 预留3字节 */
#endif
} mac_conn_param_stru;

typedef struct {
    oal_bool_enum_uint8 privacy;                /* 是否加密标志 */
    oal_nl80211_auth_type_enum_uint8 auth_type; /* 认证类型，OPEN or SHARE-KEY */
    osal_u8 wep_key_len;                        /* WEP KEY长度 */
    osal_u8 wep_key_index;                      /* WEP KEY索引 */
    osal_u8 wep_key[WLAN_WEP104_KEY_LEN];       /* WEP KEY密钥 */
    mac_nl80211_mfp_enum_uint8 mgmt_proteced;   /* 此条链接pmf是否使能 */
    wlan_pmf_cap_status_uint8 pmf_cap;          /* 设备pmf能力 */
    oal_bool_enum_uint8 wps_enable;
    mac_crypto_settings_stru crypto; /* 密钥套件信息 */
    osal_s8 rssi;      /* 关联AP的RSSI信息 */
    osal_s8 ant0_rssi; /* 天线0的rssi */
    osal_s8 ant1_rssi; /* 天线1的rssi */
    osal_u8 rsv[1];
} mac_conn_security_stru;

typedef struct {
    osal_u8 mac_addr[OAL_MAC_ADDR_LEN];
    osal_u8 rsv[2];
} mac_cfg80211_init_port_stru;

/* 解析内核配置的disconnect参数后，下发给驱动的disconnect参数 */
typedef struct mac_cfg80211_disconnect_param_tag {
    osal_u16 reason_code; /* disconnect reason code */
    osal_u16 aid;

    osal_u8 type;
    osal_u8 arry[3];
} mac_cfg80211_disconnect_param_stru;

/* 把mac_cfg_lpm_wow_en_stru定义挪到_PRE_WLAN_CHIP_TEST外面，使得关闭_PRE_WLAN_CHIP_TEST也可以正常编译 */
typedef struct mac_cfg_lpm_wow_en {
    osal_u8 en;        /* 0:off 1:on */
    osal_u8 null_wake; /* 1:null data wakeup */
} mac_cfg_lpm_wow_en_stru;

typedef enum {
    MAC_LPM_SOC_BUS_GATING = 0,
    MAC_LPM_SOC_PCIE_RD_BYPASS = 1,
    MAC_LPM_SOC_MEM_PRECHARGE = 2,
    MAC_LPM_SOC_PCIE_L0_S = 3,
    MAC_LPM_SOC_PCIE_L1_0 = 4,
    MAC_LPM_SOC_AUTOCG_ALL = 5,
    MAC_LPM_SOC_ADC_FREQ = 6,
    MAC_LPM_SOC_PCIE_L1S = 7,

    MAC_LPM_SOC_SET_BUTT
} mac_lpm_soc_set_enum;
typedef osal_u8 mac_lpm_soc_set_enum_uint8;

typedef struct mac_cfg_lpm_soc_set_tag {
    mac_lpm_soc_set_enum_uint8 mode;
    osal_u8 on_off;
    osal_u8 pcie_idle;
    osal_u8 rsv[1];
} mac_cfg_lpm_soc_set_stru;

#ifdef _PRE_WLAN_FEATURE_SMPS
typedef struct {
    wlan_mib_mimo_power_save_enum_uint8 smps_mode;
    osal_u8 trigger_smps_frame;
    osal_u16 user_idx;
} mac_cfg_smps_mode_stru;
#endif

/* 设置以太网开关需要的参数 */
typedef struct {
    osal_u8 frame_direction;
    osal_u8 switch_code;
    osal_u8 user_macaddr[WLAN_MAC_ADDR_LEN];
} mac_cfg_eth_switch_param_stru;

/* 设置probe request和probe response开关需要的参数 */
typedef struct {
    osal_u8 frame_direction;
    osal_u8 frame_switch;
    osal_u8 cb_switch;
    osal_u8 dscr_switch;
} mac_cfg_probe_switch_stru;

/* 获取mpdu数目需要的参数 */
typedef struct {
    osal_u8 user_macaddr[WLAN_MAC_ADDR_LEN];
    osal_u8 auc_resv[2];
} mac_cfg_get_mpdu_num_stru;

#ifdef _PRE_WLAN_DFT_STAT
typedef struct {
    osal_u8 user_macaddr[WLAN_MAC_ADDR_LEN];
    osal_u8 param;
    osal_u8 resv;
} mac_cfg_usr_queue_param_stru;
#endif

typedef struct {
    osal_u16 duration_ms;
    oal_bool_enum_uint8 probe;
    osal_u8 chan_cnt;
    osal_u8 channels[WLAN_MAX_CHANNEL_NUM];
} mac_chan_stat_param_stru;

typedef struct {
    osal_u32 coext_info;
    osal_u32 channel_report;
} mac_cfg_set_2040_coexist_stru;

typedef struct {
    osal_u32 mib_idx;
    osal_u32 mib_value;
} mac_cfg_set_mib_stru;

typedef struct {
    osal_u8 thruput_type;
    osal_u8 value;
    osal_u8 resv[2];
} mac_cfg_thruput_test_stru;

typedef struct {
    osal_u8 performance_log_switch_type;
    osal_u8 value;
    osal_u8 auc_resv[2];
} mac_cfg_set_performance_log_switch_stru;

typedef struct {
    osal_u8 scan_type;
    oal_bool_enum_uint8 current_bss_ignore;
    osal_u8 auc_resv[2];
} mac_cfg_set_roam_start_stru;

typedef struct {
    osal_u32 timeout;
    osal_u8 is_period;
    osal_u8 stop_start;
    osal_u8 auc_resv[2];
} mac_cfg_test_timer_stru;

typedef struct {
    osal_u32 frag_threshold;
} mac_cfg_frag_threshold_stru;

typedef struct {
    osal_u32 rts_threshold;
} mac_cfg_rts_threshold_stru;

typedef struct {
    oal_bool_enum_uint8 default_key;
    osal_u8 key_index;
    osal_u8 key_len;
    osal_u8 wep_key[WLAN_WEP104_KEY_LEN];
} mac_wep_key_param_stru;

typedef struct mac_pmksa_tag {
    osal_u8 bssid[OAL_MAC_ADDR_LEN];
    osal_u8 pmkid[OAL_PMKID_LEN];
} mac_pmksa_stru;

typedef struct {
    osal_u8 key_index;
    oal_bool_enum_uint8 pairwise;
    osal_u8 mac_addr[OAL_MAC_ADDR_LEN];
    mac_key_params_stru key;
} mac_addkey_param_stru;

typedef struct {
    osal_s32 key_len;
    osal_u8 key[OAL_WPA_KEY_LEN];
} mac_key_stru;

typedef struct {
    osal_s32 seq_len;
    osal_u8 seq[OAL_WPA_SEQ_LEN];
} mac_seq_stru;

typedef struct {
    osal_u8 key_index;
    oal_bool_enum_uint8 pairwise;
    osal_u8 mac_addr[OAL_MAC_ADDR_LEN];
    osal_u8 cipher;
    osal_u8 rsv[3];
    mac_key_stru key;
    mac_seq_stru seq;
} mac_addkey_hmac2dmac_param_stru;

typedef struct {
    oal_net_device_stru *netdev;
    osal_u8 key_index;
    oal_bool_enum_uint8 pairwise;
    osal_u8 auc_resv1[2];
    osal_u8 *mac_addr;
    osal_void *cookie;
    osal_void (*callback)(osal_void *cookie, oal_key_params_stru *key_param);
} mac_getkey_param_stru;

typedef struct {
    osal_u8 key_index;
    oal_bool_enum_uint8 pairwise;
    osal_u8 mac_addr[OAL_MAC_ADDR_LEN];
} mac_removekey_param_stru;

typedef struct {
    osal_u8 key_index;
    oal_bool_enum_uint8 unicast;
    oal_bool_enum_uint8 multicast;
    osal_u8 auc_resv1[1];
} mac_setdefaultkey_param_stru;

typedef struct {
    osal_u32 dfs_mode;
    osal_s32 para[3];
} mac_cfg_dfs_param_stru;

typedef struct {
    osal_u8 total_channel_num;
    osal_u8 channel_number[15];
} mac_cfg_dfs_auth_channel_stru;

typedef struct {
    osal_ulong addr;
    osal_u32 len;
} mac_cfg_dump_memory_stru;

/* 动态校准参数枚举，参数值 */
typedef struct {
    mac_dyn_cali_cfg_enum_uint8 dyn_cali_cfg; /* 配置命令枚举 */
    osal_u8 resv;                             /* 字节对齐 */
    osal_u16 value;                           /* 配置参数值 */
} mac_ioctl_dyn_cali_param_stru;

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
/* 发包duration配置 */
typedef struct {
    osal_u8 enable;      /* 开关 */
    osal_u8 resv[3];     /* 字节对齐 */
    osal_u16 be_val;   /* 配置参数值 */
    osal_u16 bk_val;   /* 配置参数值 */
    osal_u16 vi_val;   /* 配置参数值 */
    osal_u16 vo_val;   /* 配置参数值 */
} mac_ioctl_alg_set_duration_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
/* 异频组播转发配置 */
typedef struct {
    osal_u8 m2u_frequency_on;
    osal_u8 rsv[3];
} mac_cfg_m2u_frequency_on_param_stru;

/* 配网模式参数 */
typedef struct {
    osal_u8 m2u_adaptive_on;
    osal_u8 adaptive_num;
    osal_u8 rsv[2];
    osal_u32 threshold_time;
} mac_cfg_m2u_adaptive_on_param_stru;
#endif

typedef struct {
    osal_u32 deny_group_ipv4_addr;
    osal_u8 deny_group_ipv6_addr[OAL_IPV6_ADDR_SIZE];
    osal_u8 cfg_type;
    osal_u8 rsv[3];
} mac_cfg_m2u_deny_table_stru;

/* 组播转单播 发送参数 */
typedef struct {
    osal_u8 m2u_mcast_mode;
    osal_u8 m2u_snoop_on;
} mac_cfg_m2u_snoop_on_param_stru;

/* set snoop tid */
typedef struct {
    osal_u8 m2u_tid_num;
} mac_set_m2u_tid_num_stru;

/* 加组播转单播黑名单 */
typedef struct {
    osal_u32 deny_group_ipv4_addr;
    osal_u8 deny_group_ipv6_addr[OAL_IPV6_ADDR_SIZE];
    oal_bool_enum_uint8 ip_type;
    osal_u8 rsv[3]; /* 保留3字节对齐 */
} mac_add_m2u_deny_table_stru;

/* 删除组播转单播黑名单 */
typedef struct {
    osal_u32 deny_group_ipv4_addr;
    osal_u8 deny_group_ipv6_addr[OAL_IPV6_ADDR_SIZE];
    oal_bool_enum_uint8 ip_type;
    osal_u8 rsv[3]; /* 保留3字节对齐 */
} mac_del_m2u_deny_table_stru;


/* 清空组播转单播黑名单 */
typedef struct {
    osal_u8 m2u_clear_deny_table;
    osal_u8 m2u_show_deny_table;
} mac_clg_m2u_deny_table_stru;


/* print snoop table */
typedef struct {
    osal_u8 m2u_show_snoop_table;
} mac_show_m2u_snoop_table_stru;
typedef struct {
    osal_u8 m2u_show_snoop_deny_table;
    osal_u8 rsv[3]; /* 保留3字节对齐 */
} mac_show_m2u_snoop_deny_table_stru;

/* add snoop table */
typedef struct {
    osal_u8 m2u_add_snoop_table;
} mac_add_m2u_snoop_table_stru;

typedef struct {
    oal_bool_enum_uint8 proxyarp;
    osal_u8 rsv[3];
} mac_proxyarp_en_stru;

typedef struct {
    osal_u8 bssid[WLAN_MAC_ADDR_LEN];
    osal_u8 resv0[2];
    osal_u8 pmkid[WLAN_PMKID_LEN];
} mac_cfg_pmksa_param_stru;

typedef struct {
    osal_u64 cookie;
    osal_u32 listen_duration;                              /* 监听时间   */
    osal_u8 listen_channel;                                /* 监听的信道 */
    wlan_channel_bandwidth_enum_uint8 listen_channel_type; /* 监听信道类型 */
    osal_u8 home_channel;                                  /* 监听结束返回的信道 */
    wlan_channel_bandwidth_enum_uint8 home_channel_type;   /* 监听结束，返回主信道类型 */
    mac_vap_state_enum_uint8 last_vap_state; /* P2P0和P2P_CL 公用VAP 结构.保存进入监听前VAP 的状态
                                                   便于监听结束时恢复该状态 */
    wlan_channel_band_enum_uint8 band;
    wlan_ieee80211_roc_type_uint8 roc_type; /* roc类型 */
    osal_u8 rev;
    oal_ieee80211_channel_stru listen_channel_para;
} mac_remain_on_channel_param_stru;

typedef struct {
    osal_u8 pool_id;
} mac_device_pool_id_stru;

/* WPAS 管理帧发送结构 */
typedef struct {
    osal_s32 channel;
    osal_u8 mgmt_frame_id;
    osal_u8 rsv;
    osal_u16 len;
    const osal_u8 *frame;
} mac_mgmt_frame_stru;

typedef struct {
    osal_u8 tx_switch;
    osal_u8 rx_switch;
    osal_u8 auc_resv[2]; /* 保留2字节对齐 */
} mac_cfg_set_ampdu_amsdu_stru;

typedef struct {
    osal_u8 show_ip_addr : 4;         /* show ip addr */
    osal_u8 show_arpoffload_info : 4; /* show arpoffload 维测 */
} mac_cfg_arpoffload_info_stru;

typedef struct {
    osal_u8 in_suspend;        // 亮暗屏
    osal_u8 arpoffload_switch; // arpoffload开关
} mac_cfg_suspend_stru;

/* P2P 节能控制命令 */
typedef struct {
    osal_u8 p2p_statistics_ctrl; /* 0:清除P2P 统计值 1:打印输出统计值 */
    osal_u8 rsv[3];
} mac_cfg_p2p_stat_param_stru;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* 定制化 power ref 2g 5g配置参数 */
/* customize rf cfg struct */
typedef struct {
    osal_s8 rf_gain_db_mult4; /* 外部PA/LNA bypass时的增益(精度0.25dB) */
    osal_u8 auc_resv[3];
} mac_cfg_gain_db_per_band;

typedef struct {
    mac_cfg_gain_db_per_band ac_gain_db_2g[MAC_NUM_2G_BAND]; /* 2g */
    mac_cfg_gain_db_per_band ac_gain_db_5g[MAC_NUM_5G_BAND]; /* 5g */
} mac_cfg_custom_gain_db_stru;

typedef struct {
    osal_s8 cfg_delta_pwr_ref_rssi_2g[CUS_NUM_2G_DELTA_RSSI_NUM];
    osal_s8 cfg_delta_pwr_ref_rssi_5g[CUS_NUM_5G_DELTA_RSSI_NUM];
} mac_cfg_custom_delta_pwr_ref_stru;

typedef struct {
    osal_u16 lna_on2off_time_ns; /* LNA开到LNA关的时间(ns) */
    osal_u16 lna_off2on_time_ns; /* LNA关到LNA开的时间(ns) */

    osal_s8 lna_bypass_gain_db; /* 外部LNA bypass时的增益(dB) */
    osal_s8 lna_gain_db;        /* 外部LNA增益(dB) */
    osal_s8 pa_gain_b0_db;      /* 外部PA b0 增益(dB) */
    osal_u8 ext_switch_isexist;   /* 是否使用外部switch */

    osal_u8 ext_pa_isexist;  /* 是否使用外部pa */
    osal_u8 ext_lna_isexist; /* 是否使用外部lna */
    osal_s8 pa_gain_b1_db;   /* 外部PA b1增益(dB) */
    osal_u8 pa_gain_lvl_num; /* 外部PA 增益档位数 */
} mac_cfg_custom_ext_rf_stru;

typedef struct {
    mac_cfg_custom_gain_db_stru rf_gain_db_rf[WLAN_RF_CHANNEL_NUMS];           /* 2.4g 5g 插损 */
    mac_cfg_custom_delta_pwr_ref_stru delta_pwr_ref_cfg[WLAN_RF_CHANNEL_NUMS]; /* delta_rssi */
    osal_u8 far_dist_pow_gain_switch;                                          /* 超远距离功率增益开关 */
    osal_u8 far_dist_dsss_scale_promote_switch; /* 超远距11b 1m 2m dbb scale提升使能开关 */
    osal_u8 chn_radio_cap;
    osal_u8 junction_temp_diff; /* 结温偏差补偿值 */

    /* 注意，如果修改了对应的位置，需要同步修改函数: hal_config_custom_rf  */
    osal_s8 delta_cca_ed_high_20th_2g;
    osal_s8 delta_cca_ed_high_40th_2g;
    osal_s8 delta_cca_ed_high_20th_5g;
    osal_s8 delta_cca_ed_high_40th_5g;
} mac_cfg_customize_rf;

typedef struct {
    osal_u16 tx_ratio;        /* tx占空比 */
    osal_u16 tx_pwr_comp_val; /* 发射功率补偿值 */
} mac_tx_ratio_vs_pwr_stru;

/* 定制化TX占空比&温度补偿发射功率的参数 */
typedef struct {
    mac_tx_ratio_vs_pwr_stru txratio2pwr[3]; /* 3档占空比分别对应功率补偿值 */
    osal_u32 more_pwr;                       /* 根据温度额外补偿的发射功率 */
} mac_cfg_customize_tx_pwr_comp_stru;

/* RF寄存器定制化结构体 */
typedef struct {
    osal_u16 rf_reg117;
    osal_u16 rf_reg123;
    osal_u16 rf_reg124;
    osal_u16 rf_reg125;
    osal_u16 rf_reg126;
    osal_u8 auc_resv[2];
} mac_cus_dts_rf_reg;

/* FCC认证 参数结构体 */
typedef struct {
    osal_u8 index;       /* 下标表示偏移 */
    osal_u8 max_txpower; /* 最大发送功率 */
    osal_u16 dbb_scale;  /* dbb scale */
} mac_cus_band_edge_limit_stru;
#endif

/* 定制化 校准配置参数 */
typedef struct {
    /* dts */
    osal_s16 aus_cali_txpwr_pa_dc_ref_2g_val_chan[MAC_2G_CHANNEL_NUM]; /* txpwr分信道ref值 */
    osal_s16 aus_cali_txpwr_pa_dc_ref_5g_val_band[MAC_NUM_5G_BAND];
} mac_cus_cali_stru;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#ifdef _PRE_WLAN_ONLINE_DPD
/* 定制化 DPD校准配置参数 */
typedef struct {
    osal_u32 dpd_cali_cus_dts[MAC_DPD_CALI_CUS_PARAMS_NUM];
} mac_dpd_cus_cali_stru;
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
typedef struct {
    osal_s16 as_dy_cali_base_ratio_params[CUS_DY_CALI_PARAMS_NUM][CUS_DY_CALI_PARAMS_TIMES]; /* 产测定制化参数数组 */
    osal_s8 ac_dy_cali_2g_dpn_params[MAC_2G_CHANNEL_NUM][CUS_DY_CALI_DPN_PARAMS_NUM];
    osal_s8 ac_dy_cali_5g_dpn_params[CUS_DY_CALI_NUM_5G_BAND][WLAN_BW_CAP_80PLUS80];
    osal_s16 as_extre_point_val[CUS_DY_CALI_NUM_5G_BAND];
} mac_cus_dy_cali_param_stru;
#endif

typedef struct {
    mac_cus_cali_stru cali[WLAN_RF_CHANNEL_NUMS];
#ifdef _PRE_WLAN_ONLINE_DPD
    mac_dpd_cus_cali_stru dpd_cali_para[WLAN_RF_CHANNEL_NUMS];
#endif
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    osal_u16 aus_dyn_cali_dscr_interval[WLAN_BAND_BUTT]; /* 动态校准开关2.4g 5g */
#endif /* _PRE_WLAN_FIT_BASED_REALTIME_CALI */
    osal_s8 band_5g_enable;
    osal_u8 tone_amp_grade;
    osal_u8 auc_resv[2];
} mac_cus_dts_cali_stru;

/* perf */
typedef struct {
    signed char ac_used_mem_param[16];
    unsigned char sdio_assem_d2h;
    unsigned char auc_resv[3];
} config_device_perf_h2d_stru;
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

/* wiphy Vendor CMD参数 对应cfgid: WLAN_CFGID_VENDOR_CMD */
typedef struct mac_vendor_cmd_channel_list_info {
    osal_u8 channel_num_2g;
    osal_u8 channel_num_5g;
    osal_u8 channel_list_2g[MAC_CHANNEL_FREQ_2_BUTT];
    osal_u8 channel_list_5g[MAC_CHANNEL_FREQ_5_BUTT];
} mac_vendor_cmd_channel_list_stru;

typedef struct mac_sta_bss_parameters_ {
    td_u8  flags;
    td_u8  dtim_period;
    td_u16 beacon_interval;
} mac_sta_bss_parameters;

typedef struct mac_nl80211_sta_flag_update_ {
    td_u32 mask;
    td_u32 set;
} mac_nl80211_sta_flag_update;

typedef struct mac_station_info_tag {
    td_u32                          filled;
    td_u32                          connected_time;
    td_u32                          inactive_time;
    td_u16                          llid;
    td_u16                          plid;

    td_u64                          rx_bytes;
    td_u64                          tx_bytes;
    mac_rate_info_stru             txrate;
    mac_rate_info_stru             rxrate;

    td_u32                          rx_packets;
    td_u32                          tx_packets;
    td_u32                          tx_retries;
    td_u32                          tx_failed;

    td_u32                          rx_dropped_misc;
    td_s32                          generation;
    mac_sta_bss_parameters         bss_param;
    mac_nl80211_sta_flag_update    sta_flags;

    td_s64                          t_offset;
    const td_u8*                    assoc_req_ies;
    td_u32                          assoc_req_ies_len;
    td_u32                          beacon_loss_count;

    td_u8                           plink_state;
    td_s8                           signal;
    td_s8                           signal_avg;
    td_u8                           resv1;

    td_u32                          resv2;
} mac_station_info_stru;

/* CHR2.0使用的STA统计信息 */
typedef struct {
    osal_u8 distance; /* 算法的tpc距离，对应dmac_alg_tpc_user_distance_enum */
    osal_u8 cca_intr; /* 算法的cca_intr干扰，对应alg_cca_opt_intf_enum */
    osal_s8 snr_ant0; /* 天线0上报的SNR值 */
    osal_s8 snr_ant1; /* 天线1上报的SNR值 */
} station_info_extend_stru;

/* m2s vap信息结构体 */
typedef struct {
    osal_u8 vap_id;                    /* 业务vap id */
    wlan_nss_enum_uint8 vap_rx_nss;    /* vap的接收空间流个数 */
    wlan_m2s_type_enum_uint8 m2s_type; /* 0:软切换 1:硬切换 */
    wlan_mib_mimo_power_save_enum_uint8 sm_power_save;

    oal_bool_enum_uint8 tx_stbc;
    oal_bool_enum_uint8 transmit_stagger_sounding;
    oal_bool_enum_uint8 vht_ctrl_field_supported;
    oal_bool_enum_uint8 vht_number_sounding_dimensions;

    oal_bool_enum_uint8 vht_su_beamformer_optionimplemented;
    oal_bool_enum_uint8 tx_vht_stbc_optionimplemented;
    oal_bool_enum_uint8 support_opmode;
    osal_u8 reserved[1];
} mac_vap_m2s_stru;

typedef struct {
    const osal_char *pstr;
    wlan_channel_bandwidth_enum_uint8 width;
} width_type_info_stru;

#ifdef _PRE_WLAN_FEATURE_PMF
typedef struct {
    osal_bool     mac_mib_dot11rsnamfpc;
    osal_bool     mac_mib_dot11rsnamfpr;
} mac_vap_pmf_mab_stru;
#endif

/* 查询事件ID枚举 */
typedef enum {
    QUERY_ID_KO_VERSION = 0,
    QUERY_ID_PWR_REF = 1,
    QUERY_ID_BCAST_RATE = 2,
    QUERY_ID_STA_INFO = 3,
    QUERY_ID_ANT_RSSI = 4,
    QUERY_ID_STA_DIAG_INFO = 5,
    QUERY_ID_VAP_DIAG_INFO = 6,
    QUERY_ID_SENSING_BSSID_INFO = 7,
    QUERY_ID_BG_NOISE = 8,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    QUERY_ID_TCP_ACK_BUF = 10,
#endif
    QUERY_ID_MODE_BUTT
} hmac_vap_query_enent_id_enum;
typedef osal_u8 hmac_vap_query_enent_id_enumm_uint8;
#define HMAC_MAX_DSCP_VALUE_NUM 64

typedef struct {
    osal_wait wait_queue;
    oal_bool_enum_uint8 mgmt_tx_complete;
    osal_u32 mgmt_tx_status;
    osal_u8 mgmt_frame_id;
} oal_mgmt_tx_stru;

typedef struct {
    osal_u16 user_index;
    mac_vap_state_enum_uint8 state;
    osal_u8 vap_id;
    mac_status_code_enum_uint16 status_code;
    osal_u8 rsv[2];
} hmac_mgmt_timeout_param_stru;

typedef struct {
    osal_u8 type;
    osal_u8 eid;
    osal_u8 auc_resv[2];
} hmac_remove_ie_stru;

#ifdef _PRE_WLAN_FEATURE_HS20
typedef struct {
    osal_u8 up_low[MAX_QOS_UP_RANGE]; /* User Priority */
    osal_u8 up_high[MAX_QOS_UP_RANGE];
    osal_u8 dscp_exception_up[MAX_DSCP_EXCEPT]; /* User Priority of DSCP Exception field */
    osal_u8 valid;
    osal_u8 num_dscp_except;
    osal_u8 dscp_exception[MAX_DSCP_EXCEPT]; /* DSCP exception field  */
} hmac_cfg_qos_map_param_stru;
#endif

/* 装备测试 */
typedef struct {
    osal_u32 rx_pkct_succ_num;    /* 接收成功mpdu数据包数 */
    osal_u32 rx_ampdu_succ_num;    /* 接收成功ampdu数据包数 */
    osal_u32 rx_fail_num;    /* 接收失败数据包数 */
    osal_u32 dbb_num;             /* DBB版本号 */
    osal_u32 check_fem_pa_status; /* fem和pa是否烧毁标志 */
    osal_s16 s_rx_rssi;
    oal_bool_enum_uint8 get_dbb_completed_flag; /* 获取DBB版本号成功上报标志 */
    oal_bool_enum_uint8 check_fem_pa_flag;      /* fem和pa是否烧毁上报标志 */
    oal_bool_enum_uint8 get_rx_pkct_flag;       /* 接收数据包上报标志位 */
    oal_bool_enum_uint8 lte_gpio_check_flag;    /* 接收数据包上报标志位 */
    oal_bool_enum_uint8 report_efuse_reg_flag;  /* efuse 寄存器读取 */
    oal_bool_enum_uint8 report_reg_flag;        /* 单个寄存器读取 */
    osal_u32 reg_value;
    osal_u8  ant_status : 4,
             get_ant_flag : 4;
} hmac_atcmdsrv_get_stats_stru;
typedef struct _mac_sta_collision_handler {
    osal_bool enable_flag;
    osal_bool rpt_count;
    osal_bool resv[2];

    frw_timeout_stru collision_timer;
    osal_u32 collision_timeout;
} mac_sta_collision_handler;

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#define WDS_HASH_NUM (1 << 2)
#define WDS_AGING_TIME (300 * OAL_TIME_HZ)

typedef enum {
    WDS_MODE_NONE = 0,
    WDS_MODE_ROOTAP,
    WDS_MODE_REPEATER_STA,
    WDS_MODE_BUTT
} hmac_vap_wds_vap_mode_enum;
typedef osal_u8 hmac_vap_wds_vap_mode_enum_uint8;

typedef struct {
    oal_rwlock_stru lock;
    frw_timeout_stru wds_timer;
    struct osal_list_head peer_node[WDS_HASH_NUM];
    struct osal_list_head wds_stas[WDS_HASH_NUM];
    struct osal_list_head neigh[WDS_HASH_NUM];
    osal_u32 wds_aging;
    hmac_vap_wds_vap_mode_enum_uint8 wds_vap_mode;
    osal_u8 wds_node_num;
    osal_u16 wds_stas_num;
    osal_u16 neigh_num;
    osal_u8 auc_resv[2];
} hmac_vap_wds_stru;
#endif

typedef struct hmac_chba_vap_tag hmac_chba_vap_stru;

/* wme参数 */
typedef struct {
    osal_u32 aifsn;      /* AIFSN parameters */
    osal_u32 logcwmin;   /* cwmin in exponential form, 单位2^n -1 slot */
    osal_u32 logcwmax;   /* cwmax in exponential form, 单位2^n -1 slot */
    osal_u32 txop_limit; /* txopLimit, us */
} mac_wme_param_stru;

typedef struct hmac_vap_tag {
    /* -->> Public Start */
    osal_u8 device_id;  /* 设备ID */
    osal_u8 chip_id;    /* chip ID */
    osal_u8 user_nums; /* VAP下已挂接的用户个数 */
    osal_atomic use_cnt; /* 用于vap引用计数使用，该值在自增自减时需要使用原子操作 */

    osal_u8 bssid[WLAN_MAC_ADDR_LEN];           /* BSSID，非MAC地址，MAC地址是mib中的auc_dot11StationID */
    mac_vap_state_enum_uint8 vap_state;         /* VAP状态 */
    wlan_protocol_enum_uint8 protocol;          /* 工作的协议模式 */
    wlan_protocol_mode_enum_uint8 max_protocol; /* vap最大工作的协议模式 */
    osal_u8 resv0[3];                           /* 保留3字节对齐 */

    mac_channel_stru channel; /* channel 合并，dmac_config_offload_start_vap修改 */
    mac_ch_switch_info_stru ch_switch_info;

    osal_u8 has_user_bw_limit : 1; /* 该vap是否存在user限速 */
    osal_u8 vap_bw_limit : 1;      /* 该vap是否已限速 */
    osal_u8 voice_aggr : 1;        /* 该vap是否针对VO业务支持聚合 */
    osal_u8 one_tx_tcp_be : 1;     /* 该vap是否只有1路发送TCP BE业务 */
    osal_u8 one_rx_tcp_be : 1;     /* 该vap是否只有1路接收TCP BE业务 */
    osal_u8 no_tcp_or_udp : 1;     /* 该vap没有跑TCP或UDP业务 */
    osal_u8 random_mac : 1;
    osal_u8 roam_scan_valid_rslt : 1;

    osal_u8 al_tx_flag : 1;   /* 常发标志 */
    osal_u8 payload_flag : 3; /* 常发 payload内容:0:全0  1:全1  2:random */
    osal_u8 al_rx_flag : 1;   /* 常收标志 */
    osal_u8 custom_switch_11ax : 1;
    osal_u8 hal_cap_11ax : 1;

    osal_u8 he_mu_edca_update_count;
    osal_u8 tx_power; /* 传输功率, 单位dBm */

    wlan_nss_enum_uint8 vap_rx_nss; /* vap的接收空间流个数 */
    osal_u8 init_flag; /* vap为静态资源，标记VAP有没有被申请,DMAC OFFLOAD模式VAP被删除后过滤缓冲的帧 */
    osal_u8 vap_id;                    /* vap ID 即资源池索引值 */
    wlan_vap_mode_enum_uint8 vap_mode; /* vap模式  */

#if (defined(CONFIG_ATP_FASTIP) && defined(_PRE_WLAN_FASTIP_SUPPORT))
    osal_u32 fastip_idx; /* 用于保存netdev初始化 的 fastip index */
#endif

    /* 加密相关 */
    /* 结构体需要保证4字节对齐 */
    mac_cap_flag_stru cap_flag; /* vap能力特性标识 */

    wlan_mib_ieee802dot11_stru *mib_info;

    mac_curr_rateset_stru curr_sup_rates; /* 当前支持的速率集 */
    osal_spinlock cache_user_lock; /* cache_user lock */

    /* --------- Public STA成员 Start ----------------- */
    osal_u16 sta_aid; /* VAP为STA模式时保存AP分配给STA的AID(从响应帧获取),范围1~2007;VAP为AP模式时，不用此成员变量 */
    osal_u16 assoc_vap_id; /* VAP为STA模式时保存user(ap)的资源池索引；VAP为AP模式时，不用此成员变量 */

    osal_u8 user_pmf_cap : 1; /* STA侧在未创建user前，存储目标user的pmf使能信息 */
    osal_u8 uapsd_cap : 1;    /* 保存与STA关联的AP是否支持uapsd能力信息 */
    osal_u8 wpa3_roaming : 6; /* wpa3漫游标志位 */
    osal_u16 user_rsn_cap; /* 存储关联用户的rsn信息 */
    osal_u8 resv3;

    mac_curr_rateset_stru sta_sup_rates_ie[WLAN_BAND_BUTT]; /* 只在sta全信道扫描时使用，填写支持的速率集ie，分2.4和5G */
    /* --------- Public STA成员 End ------------------- */

    /* --------- Public AP成员 Start ----------------- */
    mac_app_ie_stru app_ie[OAL_APP_IE_NUM];

    osal_u16 multi_user_idx;                        /* 组播用户ID */
    osal_u8 cache_user_mac_addr[WLAN_MAC_ADDR_LEN]; /* cache user对应的MAC地址 */
    osal_u16 cache_user_id;                         /* cache user对应的userID */
    osal_u8 resv4[2];
    struct osal_list_head mac_user_list_head; /* 关联用户节点双向链表,使用USER结构内的DLIST */
    /* --------- Public AP成员 End ------------------- */
    /* -->> Public End */

    /* -->> Private Start */
    oal_net_device_stru *net_device; /* VAP对应的net_devices */
    osal_u8 name[OAL_IF_NAME_SIZE];  /* VAP名字 */
    oal_mgmt_tx_stru mgmt_tx;

    frw_timeout_stru mgmt_timer;
    hmac_mgmt_timeout_param_stru mgmt_timetout_param;

    frw_timeout_stru scan_timeout; /* vap发起扫描时，会启动定时器，做超时保护处理 */
#if (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    frw_timeout_stru scanresult_clean_timeout; /* vap扫描结束时，会启动定时器，做扫描结果老化处理 */
#endif
    hmac_remove_ie_stru remove_ie; /* 配置需要屏蔽的IE信息 */

    osal_u16 rx_timeout[WLAN_WME_AC_BUTT]; /* 不同业务重排序超时时间 */
    osal_u8 beamformee_sts_below_80mhz;

    mac_cfg_mode_param_stru preset_para;     /* STA协议变更时变更前的协议模式 */
    osal_u8 supp_rates[WLAN_MAX_SUPP_RATES]; /* 支持的速率集 */

    osal_u8 tx_noack; /* NOACK发送用于峰值性能测试 */
    osal_u8 protocol_fall : 1; /* 降协议标志位 */
    osal_u8 reassoc_flag : 1;  /* 关联过程中判断是否为重关联动作 */
    /* 表示vap是否支持接收对端的ampdu+amsdu联合聚合的帧，暂时没找到协议中对应的能力位，
       后续看是否需要将该能力定义到mib库中 */
    osal_u8 rx_ampduplusamsdu_active : 1;
    osal_u8 ap_11ntxbf : 1; /* sta要关联的用户的11n txbf能力信息 */
    osal_u8 resv5 : 4;
    /* 要加入的AP的国家字符串，前两个字节为国家字母，第三个为\0 */
    osal_s8 ac_desired_country[3];

    /* 信息上报 */
    osal_wait query_wait_q;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    mac_station_info_stru station_info;
#endif
    hmac_atcmdsrv_get_stats_stru atcmdsrv_get_status;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_proc_dir_entry_stru *proc_dir; /* vap对应的proc目录 */
#endif
    mac_sta_collision_handler collision_handler;
    osal_u8 dscp_tid_map[HMAC_MAX_DSCP_VALUE_NUM];

    osal_u8 max_scan_count_per_channel; /* 每个信道的扫描次数 */
    osal_u8 scan_channel_interval;      /* 间隔n个信道，切回工作信道工作一段时间 */
    osal_u16 channel_list_2g;           /* 2.4G信道列表，共13位，按BIT位设置1-13 */
    osal_u32 channel_list_5g;           /* 5G信道列表，共29位，按BIT位设置36-196 */
    osal_u16 scan_time; /* 扫描在某一信道停留此时间后，扫描结束, ms，必须是10的整数倍 */
    osal_u16 work_time_on_home_channel; /* 背景扫描时，返回工作信道工作的时间 */
    wlan_scan_type_enum_uint8 scan_type; /* 扫描模式, 0-主动模式, 1-被动模式, 供ccpriv命令使用, 跟正常宏定义的值相反 */
    osal_u8 not_scan_flag : 1; /* 对该变量赋值为1, 关联状态下不进行扫描. 配置为0时 恢复扫描; 86是复用扫描时间配置字段 */
    osal_u8 single_probe_send_times : 3; /* 单个probe req发送次数,相当于host侧重发次数,最多7次 */
    osal_u8 scan_rsv : 4; /* 预留字段, 4个bits */

    osal_u8 dhcp_debug;
    osal_u8 dhcp_resv;
    osal_u8 ng16_su_feedback;
    osal_u8 ng16_mu_feedback;

    /* --------- Private STA成员 Start ----------------- */
    osal_u16 assoc_user_cap_info; /* sta要关联的用户的能力信息 */
    oal_bool_enum_uint8 no_beacon;
    osal_u8 ba_dialog_token;                    /* BA会话创建伪随机值 */
    oal_bool_enum_uint8 updata_rd_by_ie_switch; /* 是否根据关联的ap跟新自己的国家码 */
    osal_u8 resv6[2];
    hmac_sta_tbtt_stru sta_tbtt; /* sta_tbtt处理信息 */
    /* --------- Private STA成员 End ------------------- */

    /* --------- Private AP成员 Start ----------------- */
    /* --------- Private AP成员 End ------------------- */
    /* -->> Private End */

    /* -->> Features Start */
    mac_blacklist_info_stru *blacklist_info; /* 黑名单信息 */
#ifdef _PRE_WLAN_FEATURE_ISOLATION
    mac_isolation_info_stru isolation_info; /* 用户隔离信息 */
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    oal_net_device_stru *del_net_device;  /* 指向需要通过cfg80211 接口删除的 net device */
    oal_bool_enum_uint8 wait_roc_end;
    osal_u8 resv7[3];
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_completion roc_end_ready; /* roc end completion */
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
    hmac_cfg_qos_map_param_stru cfg_qos_map_param;
#endif

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    osal_u8 bit_init_flag : 1; /* 常发关闭再次打开标志 */
    osal_u8 ack_policy : 1;    /* ack policy: 0:normal ack 1:normal ack */
    osal_u8 resv9 : 6;
    osal_u8 resv10[3];
#endif

    /* 组播转单播字段 */
    osal_void *m2u;

#ifdef _PRE_WLAN_DFT_STAT
    osal_u8 device_distance;
    osal_u8 intf_state_cca;
    osal_u8 intf_state_co;
    osal_u8 resv11;
#endif

    struct osal_list_head pmksa_list_head;

    station_info_extend_stru station_info_extend; /* CHR2.0使用的STA统计信息 */

#ifdef _PRE_WLAN_TCP_OPT
    hmac_tcp_ack_stru hmac_tcp_ack[HCC_DIR_COUNT];
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
    osal_u8 cfg_sta_pm_manual;    /* 手动设置sta pm mode的标志 */
    osal_u8 ps_mode;
    osal_u8 resv14[2];
#endif

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    hmac_vap_wds_stru wds_table;
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_sae_pmksa_cache_stru pmksa_mgmt;
    frw_timeout_stru pmksa_timer;
#endif

    osal_u8 bw_fixed : 1;          /* AP模式的VAP的带宽是否固定20M */
    osal_u8 en_40m_intol_user : 1; /* ap下是否有关联的40M intolerant的user */
    osal_u8 mgmt_report : 1;
    osal_u8 resv : 3;
    osal_u8 sae_have_pmkid : 1;
    osal_u8 en_radar_detector_is_supp : 1; /* 辅路hal device不支持雷达探测，增加判断 */

    osal_u8 uc_wmm_params_update_count; /* 初始为0，AP模式下，每跟新一次wmm参数这个变量加1,在beacon帧和assoc rsp中会
                                         填写，4bit，不能超过15；STA模式下解析帧并更新这个值 */

    mac_protection_stru protection; /* 与保护相关变量 */
    osal_u8 sae_pwe;
    osal_u8 resv17[3]; /* 预留3字节 */

    /* -->> Features End */

    /* -->> Private Start */
    osal_u8 use_rts_threshold : 1; /* 该vap的RTS策略是否使用协议规定的RTS门限 */
    osal_u8 obss_scan_timer_started : 1;
    osal_u8 bw_cmd : 1;   /* 是否使能配置数据带宽的命令 0:No  1:Yes */
    osal_u8 resv18 : 5;

    osal_u8 dtim_count;

    wlan_mib_dot11edcaentry_stru wlan_mib_mu_edca;

    hmac_vap_linkloss_stru linkloss_info; /* linkloss机制相关信息 */

    osal_u32 obss_scan_timer_remain; /* 02最大定时器超时65s, OBSS扫描定时器可能为几分钟，通过计数器来实现大定时器 */

    /* 统计信息+信息上报新增字段，修改这个字段，必须修改SDT才能解析正确 */
    hmac_vap_query_stats_stru query_stats;

    frw_timeout_stru obss_aging_timer; /* OBSS保护老化定时器 */

    osal_u8 *sta_bw_switch_fsm; /* 带宽切换状态机 */

    mac_scanned_all_bss_info all_bss_info;
    hmac_set_rate_stru vap_curr_rate;

    osal_u16 in_tbtt_offset; /* 内部tbtt offset配置值 */
    osal_u8 resv19[1];

    /* --------- Private STA成员 Start ------------------ */
    osal_u8 resv30 : 1;
    osal_u8 beacon_timeout_times : 7; /* sta等待beacon超时计数 */

    osal_u8 non_erp_exist : 1;      /* sta模式下，是否有non_erp station */
    osal_u8 auth_received : 1;      /* 接收了auth */
    osal_u8 assoc_rsp_received : 1; /* 接收了assoc */
    osal_u8 resv20 : 5;
    osal_u8 resv21[3];
    /* --------- Private STA成员 End ------------------ */

    /* --------- Private AP成员 Start ------------------ */
    osal_u8 resv28 : 1;               /* 当前放入硬件beacon帧索引值 */
    osal_u8 multi_user_multi_ac_flag : 1; /* 多用户多优先级时是否使能拥塞控制 */
    osal_u8 beacon_tx_policy : 2; /* beacon发送策略, 0-单通道, 1-双通道轮流(如果有),2-双通道(如果有) */
    osal_u8 resv22 : 4;
    osal_u8 resv23[3];

    oal_netbuf_stru *beacon_buffer;
    /* --------- Private AP成员 End ------------------ */

    /* -->> Private End */

    /* -->> HAL Start */
    hal_tx_txop_alg_stru tx_mgmt_ucast[WLAN_BAND_BUTT];  /* 单播管理帧参数 */
    hal_tx_txop_alg_stru tx_mgmt_bmcast[WLAN_BAND_BUTT]; /* 组播、广播管理帧参数 */

    hal_to_dmac_vap_stru *hal_vap;       /* hal vap结构 */
    hal_to_dmac_device_stru *hal_device; /* hal device结构体以免二次获取 */
    hal_to_dmac_chip_stru *hal_chip;     /* hal chip结构体以免二次获取 */

    oal_netbuf_queue_header_stru tx_dscr_queue_fake[HAL_TX_QUEUE_NUM];
    hal_vap_pow_info_stru vap_pow_info; /* VAP级别功率信息结构体 */
    /* -->> HAL End */

    /* -->> Features Start */

    osal_u8 intol_user_40m : 1;         /* ap下是否有关联的40M intolerant的user */
    osal_u8 radar_detector_is_supp : 1; /* 辅路hal device不支持雷达探测，增加判断 */
    osal_u8 p2p_mode : 3;               /* 0:非P2P设备; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */
    osal_u8 hml : 1;
    osal_u8 enable_11r : 1;

    osal_u8 resv29 : 1;
    osal_u8 cfg_pm_mode : 3; /* 手动挡保存的低功耗模式 */
    osal_u8 resv24 : 5;

    osal_u8 sta_pm_close_status; /* sta 低功耗状态, 包含多个模块的低功耗控制信息 */
    wlan_channel_bandwidth_enum_uint8 bandwidth_40m; /* 记录ap在切换到20M之前的带宽 */

    osal_u8 *tim_bitmap; /* 本地保存的tim_bitmap，AP模式有效 */

    osal_u8 tim_bitmap_len;
    osal_u8 ps_user_num;      /* 处于节能模式的用户的数目，AP模式有效 */
    osal_u8 bcn_tout_max_cnt; /* beacon连续收不到最大睡眠次数 */
    osal_u8 er_su_disable            : 1;
    osal_u8 dcm_constellation_tx     : 2;
    osal_u8 bandwidth_extended_range : 1;
    osal_u8 resv31                   : 4;
    osal_u8 resv25[2];

    osal_u16 beacon_timeout;      /* beacon timeout配置值 */
#ifdef _PRE_WLAN_FEATURE_P2P
    osal_u8 p2p_listen_channel;           /* P2P Listen channel, 跟之前的变量合一 */
#endif
#ifdef _PRE_WLAN_FEATURE_PMF
    osal_u32 user_pmf_status; /* 记录此vap下user pmf使能的情况，供控制硬件vap是否打开加解密开关 */
#endif
    mac_sta_pm_handler_stru sta_pm_handler; /* sta侧pm管理结构定义 */
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_cfg_p2p_noa_param_stru p2p_noa_param;
    mac_cfg_p2p_ops_param_stru p2p_ops_param;
#endif

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
    mac_vap_ip_entries_stru *ip_addr_info; /* Host侧IPv4和IPv6地址 */
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    frw_timeout_stru recovery_timer_40m; /* 40M恢复定时器 */
#endif
    /* -->> Features End */

    /* -->> Features Array Start */
    osal_void **hmac_vap_feature_arr;
    /* -->> Features Array Start */
} hmac_vap_stru;

static INLINE__ osal_u8 is_ap(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP);
}

static INLINE__ osal_u8 is_sta(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA);
}

static INLINE__ osal_u8 is_p2p_dev(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->p2p_mode == WLAN_P2P_DEV_MODE);
}

static INLINE__ osal_u8 is_p2p_go(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->p2p_mode == WLAN_P2P_GO_MODE);
}

static INLINE__ osal_u8 is_p2p_cl(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->p2p_mode == WLAN_P2P_CL_MODE);
}

static INLINE__ osal_u8 is_legacy_vap(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->p2p_mode == WLAN_LEGACY_VAP_MODE);
}

static INLINE__ osal_u8 is_p2p_mode(osal_u8 value)
{
    return ((value == WLAN_P2P_CL_MODE) || (value == WLAN_P2P_GO_MODE) || (value == WLAN_P2P_DEV_MODE));
}

static INLINE__ osal_u8 is_legacy_sta(const hmac_vap_stru *hmac_vap)
{
    return ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) && (hmac_vap->p2p_mode == WLAN_LEGACY_VAP_MODE));
}

static INLINE__ osal_u8 is_legacy_ap(const hmac_vap_stru *hmac_vap)
{
    return ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (hmac_vap->p2p_mode == WLAN_LEGACY_VAP_MODE));
}

static INLINE__ osal_u8 is_11ax_vap(const hmac_vap_stru *hmac_vap)
{
    return ((hmac_vap->hal_cap_11ax == OSAL_TRUE) && (hmac_vap->custom_switch_11ax == OSAL_TRUE));
}

static INLINE__ osal_u8 mac_vap_is_work_he_protocol(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->protocol == WLAN_HE_MODE);
}

#define is_2g(mac_vap) ((mac_vap)->channel.band == WLAN_BAND_2G)

static INLINE__ osal_u8 mac_btcoex_check_valid_vap(const hmac_vap_stru *hmac_vap)
{
    return ((hmac_vap->channel.band == WLAN_BAND_2G) ? OSAL_TRUE : OSAL_FALSE);
}

static INLINE__ osal_u8 mac_btcoex_check_valid_sta(const hmac_vap_stru *hmac_vap)
{
    return ((is_sta(hmac_vap) && (hmac_vap->channel.band == WLAN_BAND_2G)) ? OSAL_TRUE : OSAL_FALSE);
}

/* ap形态考虑go */
static INLINE__ osal_u8 mac_btcoex_check_valid_ap(const hmac_vap_stru *hmac_vap)
{
    return ((is_ap(hmac_vap) && (hmac_vap->channel.band == WLAN_BAND_2G)) ? OSAL_TRUE : OSAL_FALSE);
}

#ifdef _PRE_WLAN_FEATURE_M2S
static INLINE__ osal_u8 mac_vap_is_2g_spec_siso(const hmac_vap_stru *hmac_vap)
{
    return ((hmac_vap->channel.band == WLAN_BAND_2G) && (hmac_vap->cap_flag.custom_siso_2g == OSAL_TRUE));
}

static INLINE__ osal_u8 mac_vap_is_5g_spec_siso(const hmac_vap_stru *hmac_vap)
{
    return ((hmac_vap->channel.band == WLAN_BAND_5G) && (hmac_vap->cap_flag.custom_siso_5g == OSAL_TRUE));
}

static INLINE__ osal_u8 mac_vap_spec_is_sw_need_m2s_switch(const hmac_vap_stru *hmac_vap)
{
    return (mac_vap_is_2g_spec_siso(hmac_vap) || mac_vap_is_5g_spec_siso(hmac_vap));
}
#endif

static INLINE__ osal_u8 is_user_single_nss(const hmac_user_stru *hmac_user)
{
    return (hmac_user->user_num_spatial_stream == WLAN_SINGLE_NSS);
}

static INLINE__ osal_u8 is_vap_single_nss(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->vap_rx_nss == WLAN_SINGLE_NSS);
}

static INLINE__ wlan_channel_bandwidth_enum_uint8 mac_vap_get_cap_bw(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->channel.en_bandwidth;
}

static INLINE__ osal_void mac_vap_set_cap_bw(hmac_vap_stru *hmac_vap, osal_u8 value)
{
    hmac_vap->channel.en_bandwidth = value;
}

/* ****************************************************************************
    OTHERS定义
**************************************************************************** */
static INLINE__ const osal_char *mac_vap_width_to_str(wlan_channel_bandwidth_enum_uint8 width)
{
    osal_u32 i;
    width_type_info_stru width_tbl[] = {
        {"20",   WLAN_BAND_WIDTH_20M},
        {"40+",  WLAN_BAND_WIDTH_40PLUS},
        {"40-",  WLAN_BAND_WIDTH_40MINUS},
        {"80++", WLAN_BAND_WIDTH_80PLUSPLUS},
        {"80--", WLAN_BAND_WIDTH_80MINUSMINUS},
        {"80+-", WLAN_BAND_WIDTH_80PLUSMINUS},
        {"80-+", WLAN_BAND_WIDTH_80MINUSPLUS}
    };

    for (i = 0; i < sizeof(width_tbl) / sizeof(width_type_info_stru); i++) {
        if (width == width_tbl[i].width) {
            return width_tbl[i].pstr;
        }
    }

    return "unknown";
}

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
typedef struct {
    /* 定制化是否打开amsdu大包聚合 */
    osal_u8 host_large_amsdu_en;
    /* 当前聚合是否为amsdu聚合 */
    osal_u8 cur_amsdu_enable;
    osal_u16 amsdu_throughput_high;
    osal_u16 amsdu_throughput_low;
    osal_u16 resv;
} mac_tx_large_amsdu_ampdu_stru;
extern mac_tx_large_amsdu_ampdu_stru g_st_tx_large_amsdu;
#endif

typedef struct {
    /* 是否使能手动配置接收聚合个数 */
    osal_u8 rx_buffer_size_set_en;
    osal_u8 rx_buffer_size; /* addba rsp回复的聚合个数 */
    osal_u16 resv;
} mac_rx_buffer_size_stru;

typedef struct {
    /* 定制化小包amsdu开关 */
    osal_u8 ini_small_amsdu_en;
    osal_u8 cur_small_amsdu_en;
    osal_u16 small_amsdu_throughput_high;
    osal_u16 small_amsdu_throughput_low;
    osal_u16 small_amsdu_pps_high;
    osal_u16 small_amsdu_pps_low;
    osal_u16 resv;
} mac_small_amsdu_switch_stru;

typedef struct {
    osal_u8 ini_tcp_ack_buf_en;
    osal_u8 cur_tcp_ack_buf_en;
    osal_u16 tcp_ack_buf_throughput_high;
    osal_u16 tcp_ack_buf_throughput_low;
    osal_u16 resv;
} mac_tcp_ack_buf_switch_stru;

typedef enum {
    MAC_VAP_11AX_PARAM_CONFIG_INIT = 0,
    MAC_VAP_11AX_PARAM_CONFIG_UPDAT = 1,

    MAC_VAP_11AX_PARAM_CONFIG_BUTT
} mac_vap_11ax_param_config_enum;
typedef osal_u8 mac_vap_11ax_param_config_enum_uint8;

/* cfg id对应的参数结构体 */
/* 创建VAP参数结构体, 对应cfgid: WLAN_CFGID_ADD_VAP */
typedef struct {
    mac_cfg_add_vap_param_stru add_vap;

    oal_bool_enum_uint8 custom_switch_11ax;
    osal_u8 resv4[3]; /* 预留3字节对齐 */
    oal_net_device_stru *net_dev;
} hmac_cfg_add_vap_param_stru;
typedef hmac_cfg_add_vap_param_stru hmac_cfg_del_vap_param_stru;

#ifdef _PRE_WLAN_FEATURE_11AX
static INLINE__ osal_void mac_mib_set_qap_mu_edca_table_cwmin(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmin = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_mu_edca_table_cwmin(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmin;
}

static INLINE__ osal_void mac_mib_set_qap_mu_edca_table_cwmax(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmax = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_mu_edca_table_cwmax(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmax;
}

static INLINE__ osal_void mac_mib_set_qap_mu_edca_table_aifsn(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_aifsn = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_aifsn;
}

static INLINE__ osal_void mac_mib_set_qap_mu_edca_table_txop_limit(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_txop_limit = (osal_u16)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_mu_edca_table_txop_limit(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_txop_limit;
}

static INLINE__ void mac_mib_set_QAPMUEDCATableMandatory(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u8 val)
{
    hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_mandatory = val;
}

static INLINE__ osal_u8 mac_mib_get_QAPMUEDCATableMandatory(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_mandatory;
}
static INLINE__ osal_void mac_mib_set_ppe_thresholds_required(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 value)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_ppe_thresholds_required = value;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_ppe_thresholds_required(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_ppe_thresholds_required;
}
#endif

/* cfg id对应的get set函数 */
typedef struct {
    wlan_cfgid_enum_uint16 cfgid;
    osal_u8 auc_resv[2]; /* 2字节对齐 */
    osal_u32 (*get_func)(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param);
    osal_u32 (*set_func)(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param);
} mac_cfgid_stru;

typedef struct {
    hmac_vap_stru *hmac_vap;
    osal_char param[4]; /* 查询或配置信息占用4字节 */
} mac_cfg_event_stru;

typedef void (*ch_mib_by_bw)(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);
typedef void (*init_11ac_rates)(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
typedef void (*init_11n_rates)(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
typedef osal_u32 (*init_privacy)(hmac_vap_stru *hmac_vap, mac_conn_security_stru *conn_sec);
typedef void (*init_rates_by_prot)(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 vap_protocol,
    mac_data_rate_stru *rates);
typedef void (*init_11ax_mib)(hmac_vap_stru *hmac_vap);
typedef void (*init_11ax_rates)(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
typedef void (*p_mac_vap_init_mib_11n_cb)(hmac_vap_stru *hmac_vap);
typedef void (*p_mac_init_mib_cb)(hmac_vap_stru *hmac_vap);
typedef osal_u32 (*p_mac_vap_init_by_protocol_cb)(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 protocol);
typedef void (*p_mac_vap_init_11ac_mcs_singlenss_cb)(hmac_vap_stru *hmac_vap,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);
typedef void (*p_mac_vap_init_11ac_mcs_doublenss_cb)(hmac_vap_stru *hmac_vap,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth);

typedef struct {
    p_mac_init_mib_cb mac_init_mib_cb;
    p_mac_vap_init_mib_11n_cb mac_vap_init_mib_11n_cb;
    p_mac_vap_init_by_protocol_cb mac_vap_init_by_protocol_cb;
    p_mac_vap_init_11ac_mcs_singlenss_cb mac_vap_init_11ac_mcs_singlenss_cb;
    p_mac_vap_init_11ac_mcs_doublenss_cb mac_vap_init_11ac_mcs_doublenss_cb;
    ch_mib_by_bw ch_mib_by_bw_cb;
    init_11ac_rates init_11ac_rates_cb;
    init_11n_rates init_11n_rates_cb;
    init_privacy init_privacy_cb;
    init_rates_by_prot init_rates_by_prot_cb;
    init_11ax_rates init_11ax_rates_cb;
    init_11ax_mib init_11ax_mib_cb;
} mac_vap_cb;

void hmac_vap_free_mib_etc(hmac_vap_stru *hmac_vap);
static INLINE__ oal_bool_enum_uint8 mac_mib_get_pre_auth_actived(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.dot11_rsna_preauthentication_activated;
}

static INLINE__ osal_void mac_mib_set_pre_auth_actived(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 pre_auth)
{
    hmac_vap->mib_info->wlan_mib_privacy.dot11_rsna_preauthentication_activated = pre_auth;
}

static INLINE__ osal_u8 mac_mib_get_rsnacfg_ptksareplaycounters(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.dot11_rsna_config_ptksa_replay_counters;
}

static INLINE__ osal_void mac_mib_set_rsnacfg_ptksareplaycounters(hmac_vap_stru *hmac_vap,
    osal_u8 ptksa_replay_counters)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.dot11_rsna_config_ptksa_replay_counters =
        ptksa_replay_counters;
}

static INLINE__ osal_u8 mac_mib_get_rsnacfg_gtksareplaycounters(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.dot11_rsna_config_gtksa_replay_counters;
}

static INLINE__ osal_void mac_mib_set_rsnacfg_gtksareplaycounters(hmac_vap_stru *hmac_vap,
    osal_u8 gtksa_replay_counters)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.dot11_rsna_config_gtksa_replay_counters =
        gtksa_replay_counters;
}

static INLINE__ osal_void mac_mib_init_rsnacfg_suites(hmac_vap_stru *hmac_vap)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_group_suite = MAC_WPA_CHIPER_TKIP;
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_pair_suites[0] = MAC_WPA_CHIPER_CCMP;
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_pair_suites[1] = MAC_WPA_CHIPER_TKIP;

    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_akm_suites[0] = MAC_WPA_AKM_PSK;
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_akm_suites[1] = MAC_WPA_AKM_PSK_SHA256;

    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_group_suite = MAC_RSN_CHIPER_CCMP;
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_pair_suites[0] = MAC_RSN_CHIPER_CCMP;
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_pair_suites[1] = MAC_RSN_CHIPER_TKIP;

    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[0] = MAC_RSN_AKM_PSK;
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[1] = MAC_RSN_AKM_PSK_SHA256;
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_group_mgmt_suite = MAC_RSN_CIPHER_SUITE_AES_128_CMAC;
}

static INLINE__ osal_void mac_mib_set_wpa_group_suite(hmac_vap_stru *hmac_vap, osal_u32 suite)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_group_suite = suite;
}

static INLINE__ osal_void mac_mib_set_rsn_group_suite(hmac_vap_stru *hmac_vap, osal_u32 suite)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_group_suite = suite;
}

static INLINE__ osal_void mac_mib_set_rsn_group_mgmt_suite(hmac_vap_stru *hmac_vap, osal_u32 suite)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_group_mgmt_suite = suite;
}

static INLINE__ osal_u32 mac_mib_get_wpa_group_suite(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_group_suite;
}

static INLINE__ osal_u32 mac_mib_get_rsn_group_suite(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_group_suite;
}

static INLINE__ osal_u32 mac_mib_get_rsn_group_mgmt_suite(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_group_mgmt_suite;
}

static INLINE__ osal_void mac_mib_set_wpa_pair_suites(hmac_vap_stru *hmac_vap, const osal_u32 *suites)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_pair_suites[0] = suites[0];
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_pair_suites[1] = suites[1];
}

static INLINE__ osal_void mac_mib_set_rsn_pair_suites(hmac_vap_stru *hmac_vap, const osal_u32 *suites)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_pair_suites[0] = suites[0];
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_pair_suites[1] = suites[1];
}

static INLINE__ osal_void mac_mib_set_wpa_akm_suites(hmac_vap_stru *hmac_vap, const osal_u32 *suites)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_akm_suites[0] = suites[0];
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_akm_suites[1] = suites[1];
}

static INLINE__ osal_void mac_mib_set_rsn_akm_suites(hmac_vap_stru *hmac_vap, const osal_u32 *suites)
{
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[0] = suites[0];
    hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[1] = suites[1];
}

static INLINE__ osal_u32 mac_mib_get_rsn_akm_suites_direct(hmac_vap_stru *hmac_vap, osal_u8 number)
{
    return hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[number];
}

static INLINE__ osal_u32 mac_mib_wpa_pair_match_suites(const hmac_vap_stru *hmac_vap, const osal_u32 *pul_suites)
{
    osal_u8 idx_local;
    osal_u8 idx_peer;

    for (idx_local = 0; idx_local < WLAN_PAIRWISE_CIPHER_SUITES; idx_local++) {
        for (idx_peer = 0; idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; idx_peer++) {
            if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_pair_suites[idx_local] ==
                pul_suites[idx_peer]) {
                return pul_suites[idx_peer];
            }
        }
    }
    return 0;
}

static INLINE__ osal_u32 mac_mib_rsn_pair_match_suites(const hmac_vap_stru *hmac_vap, osal_u32 *pul_suites)
{
    osal_u8 idx_local;
    osal_u8 idx_peer;

    for (idx_local = 0; idx_local < WLAN_PAIRWISE_CIPHER_SUITES; idx_local++) {
        for (idx_peer = 0; idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; idx_peer++) {
            if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_pair_suites[idx_local] ==
                pul_suites[idx_peer]) {
                return pul_suites[idx_peer];
            }
        }
    }
    return 0;
}

static INLINE__ osal_u32 mac_mib_wpa_akm_match_suites(const hmac_vap_stru *hmac_vap, osal_u32 *pul_suites)
{
    osal_u8 idx_local;
    osal_u8 idx_peer;

    for (idx_local = 0; idx_local < WLAN_PAIRWISE_CIPHER_SUITES; idx_local++) {
        for (idx_peer = 0; idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; idx_peer++) {
            if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_akm_suites[idx_local] ==
                pul_suites[idx_peer]) {
                return pul_suites[idx_peer];
            }
        }
    }
    return 0;
}

static INLINE__ osal_u32 mac_mib_rsn_akm_match_suites(const hmac_vap_stru *hmac_vap, const osal_u32 *pul_suites)
{
    osal_u8 idx_local;
    osal_u8 idx_peer;

    for (idx_local = 0; idx_local < WLAN_PAIRWISE_CIPHER_SUITES; idx_local++) {
        for (idx_peer = 0; idx_peer < WLAN_PAIRWISE_CIPHER_SUITES; idx_peer++) {
            if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[idx_local] ==
                pul_suites[idx_peer]) {
                return pul_suites[idx_peer];
            }
        }
    }
    return 0;
}

static INLINE__ osal_u8 mac_mib_get_wpa_pair_suites(hmac_vap_stru *hmac_vap, osal_u32 *pul_suites, osal_u32 len)
{
    osal_u8 loop;
    osal_u8 num = 0;

    for (loop = 0; loop < WLAN_PAIRWISE_CIPHER_SUITES; loop++) {
        if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_pair_suites[loop] != 0) {
            if (num >= len) {
                break;
            }
            pul_suites[num++] = hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_pair_suites[loop];
        }
    }
    return num;
}

static INLINE__ osal_u8 mac_mib_get_rsn_pair_suites(hmac_vap_stru *hmac_vap, osal_u32 *pul_suites, osal_u32 len)
{
    osal_u8 loop;
    osal_u8 num = 0;

    for (loop = 0; loop < WLAN_PAIRWISE_CIPHER_SUITES; loop++) {
        if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_pair_suites[loop] != 0) {
            if (num >= len) {
                break;
            }
            pul_suites[num++] = hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_pair_suites[loop];
        }
    }
    return num;
}

static INLINE__ osal_u8 mac_mib_get_wpa_akm_suites(hmac_vap_stru *hmac_vap, osal_u32 *pul_suites, osal_u32 len)
{
    osal_u8 num = 0;
    osal_u8 loop;

    for (loop = 0; loop < WLAN_AUTHENTICATION_SUITES; loop++) {
        if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_akm_suites[loop] != 0) {
            if (num >= len) {
                break;
            }
            pul_suites[num++] = hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.wpa_akm_suites[loop];
        }
    }

    return num;
}

static INLINE__ osal_u8 mac_mib_get_rsn_akm_suites(hmac_vap_stru *hmac_vap, osal_u32 *pul_suites, osal_u32 len)
{
    osal_u8 num = 0;
    osal_u8 loop;

    for (loop = 0; loop < WLAN_AUTHENTICATION_SUITES; loop++) {
        if (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[loop] != 0) {
            if (num >= len) {
                break;
            }
            pul_suites[num++] = hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_rsna_cfg.rsn_akm_suites[loop];
        }
    }

    return num;
}

static INLINE__ osal_u32 mac_mib_get_maxmpdu_length(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_max_mpdu_length;
}

static INLINE__ osal_void mac_mib_set_maxmpdu_length(hmac_vap_stru *hmac_vap, osal_u32 maxmpdu_length)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_max_mpdu_length = maxmpdu_length;
}

static INLINE__ osal_u32 mac_mib_get_vht_max_rx_ampdu_factor(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_max_rx_ampdu_factor;
}

static INLINE__ osal_void mac_mib_set_vht_max_rx_ampdu_factor(hmac_vap_stru *hmac_vap, osal_u32 vht_max_rx_ampdu_factor)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_max_rx_ampdu_factor = vht_max_rx_ampdu_factor;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_vht_ctrl_field_cap(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_control_field_supported;
}

static INLINE__ osal_void mac_mib_set_vht_ctrl_field_cap(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 vht_ctrl_field_supported)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_control_field_supported = vht_ctrl_field_supported;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_txopps(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vhttxop_power_save_option_implemented;
}

static INLINE__ osal_void mac_mib_set_txopps(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 vht_txop_ps)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vhttxop_power_save_option_implemented = vht_txop_ps;
}

static INLINE__ osal_u16 mac_mib_get_vht_rx_mcs_map(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_rx_mcs_map;
}

static INLINE__ osal_void *mac_mib_get_ptr_vht_rx_mcs_map(const hmac_vap_stru *hmac_vap)
{
    return &(hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_rx_mcs_map);
}

static INLINE__ osal_void mac_mib_set_vht_rx_mcs_map(const hmac_vap_stru *hmac_vap, osal_u16 vht_mcs_mpa)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_rx_mcs_map = vht_mcs_mpa;
}

static INLINE__ osal_u32 mac_mib_get_us_rx_highest_rate(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_rx_highest_data_rate_supported;
}

static INLINE__ osal_void mac_mib_set_us_rx_highest_rate(const hmac_vap_stru *hmac_vap, osal_u32 rx_highest_rate)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_rx_highest_data_rate_supported = rx_highest_rate;
}

static INLINE__ osal_u16 mac_mib_get_vht_tx_mcs_map(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_tx_mcs_map;
}

static INLINE__ osal_void *mac_mib_get_ptr_vht_tx_mcs_map(const hmac_vap_stru *hmac_vap)
{
    return &(hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_tx_mcs_map);
}

static INLINE__ osal_void mac_mib_set_vht_tx_mcs_map(const hmac_vap_stru *hmac_vap, osal_u16 vht_mcs_mpa)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_tx_mcs_map = vht_mcs_mpa;
}

static INLINE__ osal_u32 mac_mib_get_us_tx_highest_rate(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_tx_highest_data_rate_supported;
}

static INLINE__ osal_void mac_mib_set_us_tx_highest_rate(const hmac_vap_stru *hmac_vap, osal_u32 tx_highest_rate)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_tx_highest_data_rate_supported = tx_highest_rate;
}

#ifdef _PRE_WLAN_FEATURE_SMPS
static INLINE__ wlan_mib_mimo_power_save_enum_uint8 mac_mib_get_smps(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_mimo_power_save;
}

static INLINE__ osal_void mac_mib_set_smps(hmac_vap_stru *hmac_vap, wlan_mib_mimo_power_save_enum_uint8 sm_power_save)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_mimo_power_save = sm_power_save;
}
#endif // _PRE_WLAN_FEATURE_SMPS

static INLINE__ wlan_mib_max_amsdu_lenth_enum_uint16 mac_mib_get_max_amsdu_length(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_max_amsdu_length;
}

static INLINE__ osal_void mac_mib_set_max_amsdu_length(hmac_vap_stru *hmac_vap,
    wlan_mib_max_amsdu_lenth_enum_uint16 max_amsdu_length)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_max_amsdu_length = max_amsdu_length;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_lsig_txop_protection(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_lsig_txop_protection_option_implemented;
}

static INLINE__ void mac_mib_set_lsig_txop_protection(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 lsig_txop_protection)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_lsig_txop_protection_option_implemented = lsig_txop_protection;
}

static INLINE__ osal_u32 mac_mib_get_max_ampdu_len_exponent(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_max_rx_ampdu_factor;
}

static INLINE__ osal_void mac_mib_set_max_ampdu_len_exponent(hmac_vap_stru *hmac_vap, osal_u32 max_ampdu_len_exponent)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_max_rx_ampdu_factor = max_ampdu_len_exponent;
}

static INLINE__ osal_u8 mac_mib_get_min_mpdu_start_spacing(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_minimum_mpdu_start_spacing;
}

static INLINE__ osal_void mac_mib_set_min_mpdu_start_spacing(hmac_vap_stru *hmac_vap, osal_u8 min_mpdu_start_spacing)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_minimum_mpdu_start_spacing = min_mpdu_start_spacing;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_pco_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_pco_option_implemented;
}

static INLINE__ osal_void mac_mib_set_pco_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 pco_option_implemented)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_pco_option_implemented = pco_option_implemented;
}

static INLINE__ osal_u32 mac_mib_get_transition_time(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_transition_time;
}

static INLINE__ osal_void mac_mib_set_transition_time(hmac_vap_stru *hmac_vap, osal_u32 transition_time)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_transition_time = transition_time;
}

static INLINE__ wlan_mib_mcs_feedback_opt_implt_enum_uint8 mac_mib_get_mcs_fdbk(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_mcs_feedback_option_implemented;
}

static INLINE__ osal_void mac_mib_set_mcs_fdbk(hmac_vap_stru *hmac_vap,
    wlan_mib_mcs_feedback_opt_implt_enum_uint8 mcs_fdbk)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_mcs_feedback_option_implemented = mcs_fdbk;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_htc_sup(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_ht_control_field_supported;
}

static INLINE__ osal_void mac_mib_set_htc_sup(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 htc_sup)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_ht_control_field_supported = htc_sup;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_rd_rsp(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_rd_responder_option_implemented;
}

static INLINE__ osal_void mac_mib_set_rd_rsp(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 rd_rsp)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_rd_responder_option_implemented = rd_rsp;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_ft_trainsistion(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_fast_bss_transition_activated;
}

static INLINE__ osal_void mac_mib_set_ft_trainsistion(const hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 ft_trainsistion)
{
    hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_fast_bss_transition_activated = ft_trainsistion;
}

static INLINE__ osal_u16 mac_mib_get_ft_mdid(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_ft_mobility_domain_id;
}

static INLINE__ osal_void mac_mib_set_ft_mdid(const hmac_vap_stru *hmac_vap, osal_u16 ft_mdid)
{
    hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_ft_mobility_domain_id = ft_mdid;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_ft_over_ds(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_ft_over_ds_activated;
}

static INLINE__ osal_void mac_mib_set_ft_over_ds(const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 ft_over_ds)
{
    hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_ft_over_ds_activated = ft_over_ds;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_ft_resource_req(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_ft_resource_request_supported;
}

static INLINE__ osal_void mac_mib_set_ft_resource_req(const hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 ft_resource_req)
{
    hmac_vap->mib_info->wlan_mib_fast_bss_trans_cfg.dot11_ft_resource_request_supported = ft_resource_req;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_privacyinvoked(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.dot11_privacy_invoked;
}

static INLINE__ osal_void mac_mib_set_privacyinvoked(const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 privacyinvoked)
{
    hmac_vap->mib_info->wlan_mib_privacy.dot11_privacy_invoked = privacyinvoked;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_rsnaactivated(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.dot11_rsna_activated;
}

static INLINE__ osal_void mac_mib_set_rsnaactivated(const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 rsnaactivated)
{
    hmac_vap->mib_info->wlan_mib_privacy.dot11_rsna_activated = rsnaactivated;
}

static INLINE__ oal_bool_enum_uint8 mac_is_wep_enabled(const hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(hmac_vap) == OSAL_FALSE) || (mac_mib_get_rsnaactivated(hmac_vap) == OSAL_TRUE)) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

static INLINE__ oal_bool_enum_uint8 mac_is_wep_allowed(const hmac_vap_stru *hmac_vap)
{
    if (mac_mib_get_rsnaactivated(hmac_vap) == OSAL_TRUE) {
        return OSAL_FALSE;
    } else {
        return mac_is_wep_enabled(hmac_vap);
    }
}

static INLINE__ void mac_set_wep_default_keyid(hmac_vap_stru *hmac_vap, osal_u8 default_key_id)
{
    hmac_vap->mib_info->wlan_mib_privacy.dot11_wep_default_key_id = default_key_id;
}

static INLINE__ osal_u8 mac_get_wep_default_keyid(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_privacy.dot11_wep_default_key_id);
}

static INLINE__ osal_u8 mac_get_wep_default_keysize(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_wep_dflt_key[mac_get_wep_default_keyid(hmac_vap)]
                .dot11_wep_default_key_value[WLAN_WEP_SIZE_OFFSET]);
}

/* ****************************************************************************
 功能描述  : 获取指定序列号的wep key的值
**************************************************************************** */
static INLINE__ osal_u8 mac_get_wep_keysize(hmac_vap_stru *hmac_vap, osal_u8 idx)
{
    return (hmac_vap->mib_info->wlan_mib_privacy.wlan_mib_wep_dflt_key[idx]
                .dot11_wep_default_key_value[WLAN_WEP_SIZE_OFFSET]);
}

/* ****************************************************************************
 功能描述  : 获取wep type的值
**************************************************************************** */
static INLINE__ wlan_ciper_protocol_type_enum_uint8 mac_get_wep_type(hmac_vap_stru *hmac_vap, osal_u8 key_id)
{
    wlan_ciper_protocol_type_enum_uint8 cipher_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;

    switch (mac_get_wep_keysize(hmac_vap, key_id)) {
        case 40:
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
        case 104:
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            break;
        default:
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
    }
    return cipher_type;
}

/* ****************************************************************************
 功能描述  : 加密方式是否是tkip only模式
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_is_tkip_only(hmac_vap_stru *hmac_vap)
{
    osal_u8 pair_suites_num;
    osal_u32        pcip[WLAN_PAIRWISE_CIPHER_SUITES] = {0};

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if ((hmac_vap->cap_flag.wpa2 == OSAL_FALSE) && (hmac_vap->cap_flag.wpa == OSAL_FALSE)) {
        return OSAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(hmac_vap) == OSAL_FALSE) || (mac_mib_get_rsnaactivated(hmac_vap) == OSAL_FALSE)) {
        // 不加密或者是WEP加密时，返回false
        return OSAL_FALSE;
    }

    if (hmac_vap->cap_flag.wpa == OSAL_TRUE) {
        pair_suites_num = mac_mib_get_wpa_pair_suites(hmac_vap, pcip, WLAN_PAIRWISE_CIPHER_SUITES);
        if ((pair_suites_num != 1) || (pcip[0] != MAC_WPA_CHIPER_TKIP)) {
            // wpa加密时，若非tkip only，返回false
            return OSAL_FALSE;
        }
    }

    if (hmac_vap->cap_flag.wpa2 == OSAL_TRUE) {
        pair_suites_num = mac_mib_get_rsn_pair_suites(hmac_vap, pcip, WLAN_PAIRWISE_CIPHER_SUITES);
        if ((pair_suites_num != 1) || (pcip[0] != MAC_RSN_CHIPER_TKIP)) {
            // wpa2加密时，若非tkip only，返回false
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

static INLINE__ osal_u8 *mac_mib_get_station_id(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id;
}

static INLINE__ void mac_mib_set_station_id(hmac_vap_stru *hmac_vap, osal_u8 *sta_id)
{
    oal_set_mac_addr(hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id, sta_id);
}

static INLINE__ osal_u32 mac_mib_get_obss_scan_passive_dwell(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_passive_dwell;
}

static INLINE__ osal_void mac_mib_set_obss_scan_passive_dwell(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_passive_dwell = val;
}

static INLINE__ osal_u32 mac_mib_get_obss_scan_active_dwell(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_active_dwell;
}

static INLINE__ osal_void mac_mib_set_obss_scan_active_dwell(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_active_dwell = val;
}

static INLINE__ osal_u32 mac_mib_get_bss_width_trigger_scan_interval(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_bss_width_trigger_scan_interval;
}

static INLINE__ osal_void mac_mib_set_bss_width_trigger_scan_interval(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_bss_width_trigger_scan_interval = val;
}

static INLINE__ osal_u32 mac_mib_get_obss_scan_passive_total_per_channel(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_passive_total_per_channel;
}

static INLINE__ osal_void mac_mib_set_obss_scan_passive_total_per_channel(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_passive_total_per_channel = val;
}

static INLINE__ osal_u32 mac_mib_get_obss_scan_active_total_per_channel(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_active_total_per_channel;
}

static INLINE__ osal_void mac_mib_set_obss_scan_active_total_per_channel(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_active_total_per_channel = val;
}

static INLINE__ osal_u32 mac_mib_get_bss_width_channel_transition_delay_factor(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_bss_width_channel_transition_delay_factor;
}

static INLINE__ osal_void mac_mib_set_bss_width_channel_transition_delay_factor(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_bss_width_channel_transition_delay_factor = val;
}

static INLINE__ osal_u32 mac_mib_get_obss_scan_activity_threshold(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_activity_threshold;
}

static INLINE__ osal_void mac_mib_set_obss_scan_activity_threshold(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_obss_scan_activity_threshold = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_high_throughput_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_high_throughput_option_implemented;
}

static INLINE__ osal_void mac_mib_set_high_throughput_option_implemented(const hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_ht_sta_cfg.dot11_high_throughput_option_implemented = val;
}

static INLINE__ osal_void mac_mib_set_vht_option_implemented(const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_vht_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_vht_option_implemented;
}

#ifdef _PRE_WLAN_FEATURE_11AX
static INLINE__ osal_void mac_mib_set_he_option_implemented(const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_option_implemented;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11TRSOptionImplemented 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_trs_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_trs_option_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11TRSOptionImplemented 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_trs_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_trs_option_implemented;
}

/* ****************************************************************************
 函 数 名  : mac_mib_set_ba_32bit_bitmap_implemented
 功能描述  : 设置MIB项 dot11_he_ba_bitmap_support_32bit 的值

**************************************************************************** */
static INLINE__ void mac_mib_set_ba_32bit_bitmap_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_ba_bitmap_support_32bit = val;
}

/* ****************************************************************************
 函 数 名  : mac_mib_get_ba_32bit_bitmap_implemented
 功能描述  : 获取MIB项 dot11_he_ba_bitmap_support_32bit 的值

**************************************************************************** */
static INLINE__ osal_u8 mac_mib_get_ba_32bit_bitmap_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_ba_bitmap_support_32bit;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HEPSROptionImplemented 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_psr_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_psr_option_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HEPSROptionImplemented 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_psr_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_psr_option_implemented;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HEBQRControlImplemented 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_bqr_control_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_bqr_control_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HEBQRControlImplemented 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_bqr_control_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_bqr_control_implemented;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11_support_response_nfrp_trigger 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_support_response_nfrp_trigger(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_support_response_nfrp_trigger = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11_support_response_nfrp_trigger 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_support_response_nfrp_trigger(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_support_response_nfrp_trigger;
}

static INLINE__ osal_void mac_mib_set_he_ht_control_field_supported(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_heht_control_field_supported = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_ht_control_field_supported(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_heht_control_field_supported;
}

static INLINE__ osal_void mac_mib_set_he_twt_option_activated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_twt_option_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_twt_option_activated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_twt_option_activated;
}

static INLINE__ osal_void mac_mib_set_he_trigger_mac_padding_duration(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_trigger_mac_padding_duration = val;
}

static INLINE__ osal_u8 mac_mib_get_he_trigger_mac_padding_duration(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_trigger_mac_padding_duration;
}

/* ****************************************************************************
 函 数 名  : mac_mib_set_he_mtid_aggr_rx_support
 功能描述  : 设置MIB项 dot11_he_mtid_aggregation_rx_support 的值

**************************************************************************** */
static INLINE__ void mac_mib_set_he_mtid_aggr_rx_support(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_mtid_aggregation_rx_support = val;
}

/* ****************************************************************************
 函 数 名  : mac_mib_get_he_mtid_aggr_rx_support
 功能描述  : 获取MIB项 dot11_he_mtid_aggregation_rx_support 的值

**************************************************************************** */
static INLINE__ osal_u8 mac_mib_get_he_mtid_aggr_rx_support(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_mtid_aggregation_rx_support;
}

static INLINE__ osal_void mac_mib_set_he_max_ampdu_length(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_max_ampdu_length = val;
}

static INLINE__ osal_u8 mac_mib_get_he_max_ampdu_length(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_max_ampdu_length;
}

static INLINE__ void mac_mib_set_he_rx_mcs_map(hmac_vap_stru *hmac_vap, osal_u32 he_mcs_map)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_rx_mcs_map = he_mcs_map;
}

static INLINE__ osal_u32 mac_mib_get_he_rx_mcs_map(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_rx_mcs_map;
}

static INLINE__ osal_void *mac_mib_get_ptr_he_rx_mcs_map(const hmac_vap_stru *hmac_vap)
{
    return &(hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_rx_mcs_map);
}

static INLINE__ void mac_mib_set_he_tx_mcs_map(hmac_vap_stru *hmac_vap, osal_u32 he_mcs_map)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_tx_mcs_map = he_mcs_map;
}

static INLINE__ osal_u32 mac_mib_get_he_tx_mcs_map(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_tx_mcs_map;
}

static INLINE__ osal_void *mac_mib_get_ptr_he_tx_mcs_map(const hmac_vap_stru *hmac_vap)
{
    return &(hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_tx_mcs_map);
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_om_control_support(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_oper_om_implemented;
}

static INLINE__ osal_void mac_mib_set_he_om_control_support(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_oper_om_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_DualBandSupport(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_dual_band_support;
}

static INLINE__ void mac_mib_set_he_DualBandSupport(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_dual_band_support = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_ldpc_coding_in_payload(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_heldpc_coding_in_payload;
}

static INLINE__ osal_void mac_mib_set_he_ldpc_coding_in_payload(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_heldpc_coding_in_payload = val;
}

static INLINE__ osal_void mac_mib_set_he_su_beamformer(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_su_beamformer = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_su_beamformer(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_su_beamformer;
}

static INLINE__ osal_void mac_mib_set_he_su_beamformee(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_su_beamformee = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_su_beamformee(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_su_beamformee;
}

static INLINE__ osal_void mac_mib_set_he_rx_stbc_beamformer(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_hestbc_rx_below_80mhz = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_rx_stbc_beamformer(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_hestbc_rx_below_80mhz;
}

static INLINE__ osal_void mac_mib_set_he_mu_beamformer(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_mu_beamformer = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_mu_beamformer(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_mu_beamformer;
}

static INLINE__ void mac_mib_set_multi_bssid_implement(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_mgmt_option_multi_bssid_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11MgmtOptionMultiBSSIDImplemented 的值
**************************************************************************** */
static INLINE__ osal_u8 mac_mib_get_multi_bssid_implement(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_mgmt_option_multi_bssid_implemented;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HEBSRControlImplemented 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_bsr_control_implemented(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_bsr_control_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HEBSRControlImplemented 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_bsr_control_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_he_bsr_control_implemented;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HTVHTTriggerOptionImplemented 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_ht_vht_trigger_option_implemented(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_ht_vht_trigger_option_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HTVHTTriggerOptionImplemented 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_ht_vht_trigger_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_ht_vht_trigger_option_implemented;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HENumberSoundingDimensionsLessThanOrEqualTo80 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_num_sounding_dims_below_80mhz(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_num_sounding_dims_below_80mhz = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HENumberSoundingDimensionsLessThanOrEqualTo80 的值
**************************************************************************** */
static INLINE__ osal_u32 mac_mib_get_he_num_sounding_dims_below_80mhz(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_num_sounding_dims_below_80mhz;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HENG16MUFeedbackSupport 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_ng16_mufeedback_support(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_ng16_mufeedback_support = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HENG16MUFeedbackSupport 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_ng16_mufeedback_support(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_ng16_mufeedback_support;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HECodebookSizePhi7Psi5MUFeedbackSupport 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_codebooksize_mufeedback_support(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_codebooksize_mufeedback_support = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HECodebookSizePhi7Psi5MUFeedbackSupport 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_codebooksize_mufeedback_support(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_codebooksize_mufeedback_support;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HEPartialBWDLMUMIMOImplemented 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_partial_bw_dlmumimo_implemented(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_partial_bw_dlmumimo_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HEPartialBWDLMUMIMOImplemented 的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_partial_bw_dlmumimo_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_phy_config.dot11_he_partial_bw_dlmumimo_implemented;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HEBssColorDisable的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_oper_bss_color_disable(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_oper_config.dot11_he_bss_color_disable = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HEBssColorDisable的值
**************************************************************************** */
static INLINE__ oal_bool_enum_uint8 mac_mib_get_he_oper_bss_color_disable(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_oper_config.dot11_he_bss_color_disable;
}

/* ****************************************************************************
 功能描述  : 设置MIB项 dot11HEBssColor的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_he_oper_bss_color(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_he_oper_config.dot11_he_bss_color = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11HEBssColor的值
**************************************************************************** */
static INLINE__ osal_u8 mac_mib_get_he_oper_bss_color(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_oper_config.dot11_he_bss_color;
}
#endif  /* ifdef _PRE_WLAN_FEATURE_11AX */

static INLINE__ oal_bool_enum_uint8 mac_mib_get_forty_mhz_operation_implemented(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->channel.band == WLAN_BAND_2G) ?
        hmac_vap->mib_info->wlan_mib_phy_ht.dot11_2g_40mhz_operation_implemented :
        hmac_vap->mib_info->wlan_mib_phy_ht.dot11_5g_40mhz_operation_implemented;
}

static INLINE__ osal_void mac_mib_set_forty_mhz_operation_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        hmac_vap->mib_info->wlan_mib_phy_ht.dot11_2g_40mhz_operation_implemented = val;
    } else {
        hmac_vap->mib_info->wlan_mib_phy_ht.dot11_5g_40mhz_operation_implemented = val;
    }
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_short_gi_option_in_forty_implemented(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->channel.band == WLAN_BAND_2G) ?
        hmac_vap->mib_info->wlan_mib_phy_ht.dot112_g_short_gi_option_in_forty_implemented :
        hmac_vap->mib_info->wlan_mib_phy_ht.dot115_g_short_gi_option_in_forty_implemented;
}

static INLINE__ osal_void mac_mib_set_short_gi_option_in_forty_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        hmac_vap->mib_info->wlan_mib_phy_ht.dot112_g_short_gi_option_in_forty_implemented = val;
    } else {
        hmac_vap->mib_info->wlan_mib_phy_ht.dot115_g_short_gi_option_in_forty_implemented = val;
    }
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_2GFortyMHzOperationImplemented(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_phy_ht.dot11_2g_40mhz_operation_implemented);
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_5GFortyMHzOperationImplemented(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_phy_ht.dot11_5g_40mhz_operation_implemented);
}


static INLINE__ osal_void mac_mib_set_2g_forty_mhz_operation_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_2g_40mhz_operation_implemented = val;
}

static INLINE__ osal_void mac_mib_set_5g_forty_mhz_operation_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_5g_40mhz_operation_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_spectrum_management_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_spectrum_management_implemented;
}

static INLINE__ osal_void mac_mib_set_spectrum_management_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_spectrum_management_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_forty_mhz_intolerant(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_40mhz_intolerant;
}

static INLINE__ osal_void mac_mib_set_forty_mhz_intolerant(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_40mhz_intolerant = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_2040_bss_coexistence_management_support(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot112040_bss_coexistence_management_support;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_rsnamfpc(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.dot11_rsnamfpc;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_rsnamfpr(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_privacy.dot11_rsnamfpr;
}

static INLINE__ osal_void mac_mib_set_dot11_rsnamfpc(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_privacy.dot11_rsnamfpc = val;
}

static INLINE__ osal_void mac_mib_set_dot11_rsnamfpr(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_privacy.dot11_rsnamfpr = val;
}

static INLINE__ osal_u32 mac_mib_get_dot11AssociationSAQueryMaximumTimeout(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_association_sa_query_maximum_timeout;
}

static INLINE__ osal_u32 mac_mib_get_dot11AssociationSAQueryRetryTimeout(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_association_sa_query_retry_timeout;
}

static INLINE__ void mac_mib_set_dot11AssociationSAQueryMaximumTimeout(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_association_sa_query_maximum_timeout = val;
}

static INLINE__ void mac_mib_set_dot11AssociationSAQueryRetryTimeout(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_association_sa_query_retry_timeout = val;
}

static INLINE__ osal_void mac_mib_set_dot11_extended_channel_switch_activated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_extended_channel_switch_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_extended_channel_switch_activated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_extended_channel_switch_activated;
}

static INLINE__ osal_void mac_mib_set_dot11_radio_measurement_activated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_radio_measurement_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_radio_measurement_activated(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_radio_measurement_activated;
}

static INLINE__ osal_void mac_mib_set_dot11_qbss_load_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_qbss_load_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_qbss_load_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_qbss_load_implemented;
}

static INLINE__ osal_void mac_mib_set_dot11_apsd_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_apsd_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_apsd_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_apsd_option_implemented;
}

static INLINE__ osal_void mac_mib_set_dot11_delayed_block_ack_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_delayed_block_ack_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8
mac_mib_get_dot11_delayed_block_ack_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_delayed_block_ack_option_implemented;
}

static INLINE__ osal_void mac_mib_set_dot11ImmediateBlockAckOptionImplemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_immediate_block_ack_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11ImmediateBlockAckOptionImplemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_immediate_block_ack_option_implemented;
}

static INLINE__ osal_void mac_mib_set_dot11_qos_option_implemented(const hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_qos_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_qos_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_qos_option_implemented;
}

static INLINE__ osal_void mac_mib_set_dot11_multi_domain_capability_activated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_multi_domain_capability_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11_multi_domain_capability_activated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_multi_domain_capability_activated;
}

static INLINE__ void mac_mib_set_dot11AssociationResponseTimeOut(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_association_response_time_out = val;
}

static INLINE__ osal_u32 mac_mib_get_dot11AssociationResponseTimeOut(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_association_response_time_out;
}

static INLINE__ void mac_mib_set_MaxAssocUserNums(hmac_vap_stru *hmac_vap, osal_u16 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_max_assoc_user_nums = val;
}

static INLINE__ osal_u16 mac_mib_get_MaxAssocUserNums(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_max_assoc_user_nums;
}

static INLINE__ void mac_mib_set_SupportRateSetNums(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_support_rate_set_nums = val;
}

static INLINE__ osal_u8 mac_mib_get_SupportRateSetNums(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_support_rate_set_nums;
}

static INLINE__ void mac_mib_set_CfgAmsduTxAtive(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_cfg_amsdu_tx_ative = val;
}

static INLINE__ osal_u8 mac_mib_get_CfgAmsduTxAtive(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_cfg_amsdu_tx_ative;
}

static INLINE__ void mac_mib_set_AmsduAggregateAtive(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_amsdu_aggregate_ative = val;
}

static INLINE__ osal_u8 mac_mib_get_AmsduAggregateAtive(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_amsdu_aggregate_ative;
}

static INLINE__ void mac_mib_set_AmsduPlusAmpduActive(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_amsdu_plus_ampdu_active = val;
}

static INLINE__ osal_u8 mac_mib_get_AmsduPlusAmpduActive(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_amsdu_plus_ampdu_active;
}

static INLINE__ void mac_mib_set_WPSActive(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_wps_active = val;
}

static INLINE__ osal_u8 mac_mib_get_WPSActive(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_wps_active;
}

static INLINE__ osal_void mac_mib_set_2040_switch_prohibited(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot112040_switch_prohibited = val;
}

static INLINE__ osal_u8 mac_mib_get_2040_switch_prohibited(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot112040_switch_prohibited;
}

static INLINE__ void mac_mib_set_TxAggregateActived(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_aggregate_actived = val;
}

static INLINE__ osal_u8 mac_mib_get_TxAggregateActived(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_aggregate_actived;
}

static INLINE__ void mac_mib_set_CfgAmpduTxAtive(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_cfg_ampdu_tx_ative = val;
}

static INLINE__ osal_u8 mac_mib_get_CfgAmpduTxAtive(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_cfg_ampdu_tx_ative;
}

static INLINE__ void mac_mib_set_RxBASessionNumber(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rx_ba_session_number = val;
}

static INLINE__ osal_u8 mac_mib_get_RxBASessionNumber(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_rx_ba_session_number;
}

static INLINE__ void mac_mib_incr_RxBASessionNumber(hmac_vap_stru *hmac_vap)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rx_ba_session_number++;
}

static INLINE__ void mac_mib_decr_RxBASessionNumber(hmac_vap_stru *hmac_vap)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rx_ba_session_number--;
}

static INLINE__ void mac_mib_set_TxBASessionNumber(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_ba_session_number = val;
}

static INLINE__ osal_u8 mac_mib_get_TxBASessionNumber(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_ba_session_number;
}

static INLINE__ void mac_mib_incr_TxBASessionNumber(hmac_vap_stru *hmac_vap)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_ba_session_number++;
}

static INLINE__ void mac_mib_decr_TxBASessionNumber(hmac_vap_stru *hmac_vap)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_ba_session_number--;
}

static INLINE__ osal_void mac_mib_set_authentication_mode(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_authentication_mode = val;
}

static INLINE__ osal_u8 mac_mib_get_authentication_mode(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_authentication_mode;
}

static INLINE__ void mac_mib_set_AddBaMode(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_add_ba_mode = val;
}

static INLINE__ osal_u8 mac_mib_get_AddBaMode(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_add_ba_mode;
}

static INLINE__ void mac_mib_set_TxTrafficClassifyFlag(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_traffic_classify_flag = val;
}

static INLINE__ osal_u8 mac_mib_get_TxTrafficClassifyFlag(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_tx_traffic_classify_flag;
}

static INLINE__ void mac_mib_set_StaAuthCount(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_sta_auth_count = val;
}

static INLINE__ osal_u8 mac_mib_get_StaAuthCount(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_sta_auth_count;
}

static INLINE__ void mac_mib_set_StaAssocCount(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_sta_assoc_count = val;
}

static INLINE__ osal_u8 mac_mib_get_StaAssocCount(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_sta_assoc_count;
}

static INLINE__ void mac_mib_incr_StaAuthCount(hmac_vap_stru *hmac_vap)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_sta_auth_count++;
}

static INLINE__ void mac_mib_incr_StaAssocCount(hmac_vap_stru *hmac_vap)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_sta_assoc_count++;
}

static INLINE__ void mac_mib_set_dot11RMBeaconTableMeasurementActivated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_beacon_table_measurement_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconTableMeasurementActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_beacon_table_measurement_activated;
}

static INLINE__ void mac_mib_set_dot11RMBeaconActiveMeasurementActivated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_beacon_active_measurement_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconActiveMeasurementActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_beacon_active_measurement_activated;
}

static INLINE__ void mac_mib_set_dot11RMBeaconPassiveMeasurementActivated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_beacon_passive_measurement_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconPassiveMeasurementActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_beacon_passive_measurement_activated;
}

static INLINE__ void mac_mib_set_dot11RMLinkMeasurementActivated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_link_measurement_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11RMLinkMeasurementActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_link_measurement_activated;
}

static INLINE__ void mac_mib_set_dot11RMChannelLoadMeasurementActivated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_channel_load_measurement_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11RMChannelLoadMeasurementActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_channel_load_measurement_activated;
}

static INLINE__ void mac_mib_set_dot11RMNeighborReportActivated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_neighbor_report_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dot11RMNeighborReportActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_rm_neighbor_report_activated;
}

static INLINE__ osal_void mac_mib_set_2040_bss_coexistence_management_support(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot112040_bss_coexistence_management_support = val;
}

static INLINE__ osal_u32 mac_mib_get_dot11_dtim_period(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_dtim_period;
}

static INLINE__ osal_void mac_mib_set_dot11_vap_max_bandwidth(hmac_vap_stru *hmac_vap, wlan_bw_cap_enum_uint8 bandwidth)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_vap_max_band_width = bandwidth;
}

static INLINE__ wlan_bw_cap_enum_uint8 mac_mib_get_dot11_vap_max_bandwidth(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_vap_max_band_width;
}

static INLINE__ osal_void mac_mib_set_beacon_period(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    if (val != 0) {
        hmac_vap->mib_info->wlan_mib_sta_config.dot11_beacon_period = val;
    }
}
static INLINE__ osal_u32 mac_mib_get_beacon_period(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_beacon_period;
}

static INLINE__ osal_void mac_mib_set_desired_bss_type(const hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_desired_bss_type = val;
}
static INLINE__ osal_u8 mac_mib_get_desired_bss_type(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_desired_bss_type;
}


static INLINE__ osal_u8 *mac_mib_get_desired_ssid(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_desired_ssid;
}

static INLINE__ void mac_mib_set_AuthenticationResponseTimeOut(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_authentication_response_time_out = val;
}
static INLINE__ osal_u32 mac_mib_get_AuthenticationResponseTimeOut(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_authentication_response_time_out;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_mgmt_option_bss_transition_activated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_mgmt_option_bss_transition_activated;
}

static INLINE__ osal_void mac_mib_set_mgmt_option_bss_transition_activated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_mgmt_option_bss_transition_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_mgmt_option_bss_transition_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_mgmt_option_bss_transition_implemented;
}

static INLINE__ osal_void mac_mib_set_mgmt_option_bss_transition_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_mgmt_option_bss_transition_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_wireless_management_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_wireless_management_implemented;
}

static INLINE__ osal_void mac_mib_set_wireless_management_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_wireless_management_implemented = val;
}

#ifdef _PRE_WLAN_FEATURE_FTM
static INLINE__ osal_void mac_mib_set_fine_timing_msmt_init_activated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_fine_timing_msmt_init_activated = val;
}
static INLINE__ oal_bool_enum_uint8 mac_mib_get_fine_timing_msmt_init_activated(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_fine_timing_msmt_init_activated;
}
static INLINE__ osal_void mac_mib_set_fine_timing_msmt_resp_activated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_fine_timing_msmt_resp_activated = val;
}
static INLINE__ oal_bool_enum_uint8 mac_mib_get_fine_timing_msmt_resp_activated(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_fine_timing_msmt_resp_activated;
}
static INLINE__ osal_void mac_mib_set_fine_timing_msmt_range_req_activated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_rm_fine_timing_msmt_range_rep_activated = val;
}
static INLINE__ oal_bool_enum_uint8 mac_mib_get_fine_timing_msmt_range_req_activated(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_wireless_mgmt_op.dot11_rm_fine_timing_msmt_range_rep_activated;
}

#endif

/* ****************************************************************************
 功能描述  : 初始化支持2040共存
**************************************************************************** */
static INLINE__ osal_void mac_mib_init_2040(hmac_vap_stru *hmac_vap)
{
    mac_mib_set_forty_mhz_intolerant(hmac_vap, OSAL_FALSE);
    mac_mib_set_spectrum_management_implemented(hmac_vap, OSAL_TRUE);
    mac_mib_set_2040_bss_coexistence_management_support(hmac_vap, OSAL_TRUE);
}

/* ****************************************************************************
 功能描述  : 初始化OBSS SCAN 参数
**************************************************************************** */
static INLINE__ osal_void mac_mib_init_obss_scan(hmac_vap_stru *hmac_vap)
{
    mac_mib_set_obss_scan_passive_dwell(hmac_vap, 20);
    mac_mib_set_obss_scan_active_dwell(hmac_vap, 10);
    mac_mib_set_bss_width_trigger_scan_interval(hmac_vap, 300);
    mac_mib_set_obss_scan_passive_total_per_channel(hmac_vap, 200);
    mac_mib_set_obss_scan_active_total_per_channel(hmac_vap, 20);
    mac_mib_set_bss_width_channel_transition_delay_factor(hmac_vap, 5);
    mac_mib_set_obss_scan_activity_threshold(hmac_vap, 25);
}

static INLINE__ osal_void mac_mib_set_dot11dtimperiod(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    if (val != 0) {
        hmac_vap->mib_info->wlan_mib_sta_config.dot11_dtim_period = val;
    }
}

static INLINE__ osal_u32 mac_mib_get_powermanagementmode(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_power_management_mode;
}

static INLINE__ osal_void mac_mib_set_powermanagementmode(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    if (val != 0) {
        hmac_vap->mib_info->wlan_mib_sta_config.dot11_power_management_mode = val;
    }
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
static INLINE__ oal_bool_enum_uint8 mac_mib_get_operating_mode_notification_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_operating_mode_notification_implemented;
}

static INLINE__ osal_void mac_mib_set_operating_mode_notification_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_vht_sta_config.dot11_operating_mode_notification_implemented = val;
}
#endif

static INLINE__ oal_bool_enum_uint8 mac_mib_get_lsig_txop_full_protection_activated(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_operation.dot11_lsigtxop_full_protection_activated);
}

static INLINE__ osal_void mac_mib_set_lsig_txop_full_protection_activated(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 lsig_txop_full_protection_activated)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_lsigtxop_full_protection_activated =
        lsig_txop_full_protection_activated;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_non_gf_entities_present(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_operation.dot11_non_gf_entities_present);
}

static INLINE__ osal_void mac_mib_set_non_gf_entities_present(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 non_gf_entities_present)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_non_gf_entities_present = non_gf_entities_present;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_rifs_mode(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_operation.dot11_rifs_mode);
}

static INLINE__ osal_void mac_mib_set_rifs_mode(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 rifs_mode)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_rifs_mode = rifs_mode;
}

static INLINE__ wlan_mib_ht_protection_enum_uint8 mac_mib_get_ht_protection(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_operation.dot11_ht_protection);
}

static INLINE__ osal_void mac_mib_set_dual_cts_protection(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_dual_cts_protection = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dual_cts_protection(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_operation.dot11_dual_cts_protection);
}

static INLINE__ osal_void mac_mib_set_pco_activated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_pco_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_pco_activated(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_operation.dot11_pco_activated);
}

static INLINE__ osal_void mac_mib_set_ht_protection(hmac_vap_stru *hmac_vap,
    wlan_mib_ht_protection_enum_uint8 ht_protection)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_ht_protection = ht_protection;
}

static INLINE__ osal_void mac_mib_set_spectrum_management_required(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_spectrum_management_required = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_spectrum_management_required(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_spectrum_management_required;
}

static INLINE__ osal_void mac_mib_set_2g_short_gi_option_in_forty_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot112_g_short_gi_option_in_forty_implemented = val;
}

static INLINE__ osal_void mac_mib_set_5g_short_gi_option_in_forty_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot115_g_short_gi_option_in_forty_implemented = val;
}

static INLINE__ void mac_mib_set_FragmentationThreshold(hmac_vap_stru *hmac_vap, osal_u32 frag_threshold)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_fragmentation_threshold = frag_threshold;
}

static INLINE__ osal_u32 mac_mib_get_FragmentationThreshold(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_fragmentation_threshold;
}

static INLINE__ osal_void mac_mib_set_rts_threshold(hmac_vap_stru *hmac_vap, osal_u32 rts_threshold)
{
    hmac_vap->mib_info->wlan_mib_operation.dot11_rts_threshold = rts_threshold;
}

static INLINE__ osal_u32 mac_mib_get_rts_threshold(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_operation.dot11_rts_threshold;
}

static INLINE__ osal_void mac_mib_set_antenna_selection_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_antenna_selection_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_antenna_selection_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_antenna_selection_option_implemented;
}

/* ****************************************************************************
 功能描述  : 设置是否使能基于显示CSI(信道状态信息)反馈的发射天线选择能力
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_trans_explicit_csi_feedback_as_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_transmit_explicit_csi_feedback_as_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8
mac_mib_get_trans_explicit_csi_feedback_as_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_transmit_explicit_csi_feedback_as_option_implemented;
}

static INLINE__ osal_void mac_mib_set_trans_indices_feedback_as_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_transmit_indices_feedback_as_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_trans_indices_feedback_as_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_transmit_indices_feedback_as_option_implemented;
}

/* ****************************************************************************
 功能描述  : indicates that the computation of CSI and feedback the results to support the peer to do ASEL is supported
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_explicit_csi_feedback_as_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_explicit_csi_feedback_as_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_explicit_csi_feedback_as_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_explicit_csi_feedback_as_option_implemented;
}

static INLINE__ osal_void mac_mib_set_receive_antenna_selection_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_receive_antenna_selection_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_receive_antenna_selection_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_receive_antenna_selection_option_implemented;
}

static INLINE__ osal_void mac_mib_set_trans_sounding_ppdu_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_transmit_sounding_ppdu_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_trans_sounding_ppdu_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_antenna.dot11_transmit_sounding_ppdu_option_implemented;
}

static INLINE__ wlan_11b_mib_preamble_enum_uint8
mac_mib_get_short_preamble_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return (hmac_vap->mib_info->wlan_mib_phy_dsss.phy_hrdsss.dot11_short_preamble_option_implemented);
}

static INLINE__ osal_void mac_mib_set_short_preamble_option_implemented(hmac_vap_stru *hmac_vap,
    wlan_11b_mib_preamble_enum_uint8 preamble)
{
    hmac_vap->mib_info->wlan_mib_phy_dsss.phy_hrdsss.dot11_short_preamble_option_implemented = preamble;
}

static INLINE__ osal_void mac_mib_set_pbcc_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_dsss.phy_hrdsss.dot11_pbcc_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_pbcc_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_dsss.phy_hrdsss.dot11_pbcc_option_implemented;
}

static INLINE__ osal_void mac_mib_set_channel_agility_present(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_dsss.phy_hrdsss.dot11_channel_agility_present = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_channel_agility_present(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_dsss.phy_hrdsss.dot11_channel_agility_present;
}

static INLINE__ osal_void mac_mib_set_dsss_ofdm_option_activated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_dsss.phy_erp.dot11_dsssofdm_option_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_dsss_ofdm_option_activated(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_dsss.phy_erp.dot11_dsssofdm_option_activated;
}

static INLINE__ osal_void mac_mib_set_short_slot_time_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_dsss.phy_erp.dot11_short_slot_time_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_short_slot_time_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_dsss.phy_erp.dot11_short_slot_time_option_implemented;
}

/* ****************************************************************************
 功能描述  : 是否支持short slot time
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_short_slot_time_option_activated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_dsss.phy_erp.dot11_short_slot_time_option_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_short_slot_time_option_activated(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_dsss.phy_erp.dot11_short_slot_time_option_activated;
}

static INLINE__ osal_void mac_mib_set_receive_stagger_sounding_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_receive_stagger_sounding_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_receive_stagger_sounding_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_receive_stagger_sounding_option_implemented;
}

/* ****************************************************************************
 功能描述  : 表明是否支持staggered sounding帧的发送
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_trans_stagger_sounding_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_transmit_stagger_sounding_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_trans_stagger_sounding_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_transmit_stagger_sounding_option_implemented;
}

static INLINE__ osal_void mac_mib_set_receive_ndp_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_receive_ndp_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_receive_ndp_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_receive_ndp_option_implemented;
}

static INLINE__ osal_void mac_mib_set_transmit_ndp_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_transmit_ndp_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_transmit_ndp_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_transmit_ndp_option_implemented;
}

static INLINE__ osal_void mac_mib_set_implicit_trans_bf_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_implicit_transmit_beamforming_option_implemented =
        val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_implicit_trans_bf_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_implicit_transmit_beamforming_option_implemented;
}

/* ****************************************************************************
 功能描述  : 表明是否支持NDP sounding帧的发送
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_calibration_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_calibration_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_calibration_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_calibration_option_implemented;
}

/* ****************************************************************************
 功能描述  : 表明是否支持CSI显式反馈在txbf中
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_explicit_csi_trans_bf_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_csi_transmit_beamforming_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_explicit_csi_trans_bf_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_csi_transmit_beamforming_option_implemented;
}

static INLINE__ osal_void mac_mib_set_explicit_non_compress_bf_matrix_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_non_compressed_beamforming_matrix_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8
mac_mib_get_explicit_non_compress_bf_matrix_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_non_compressed_beamforming_matrix_option_implemented;
}

static INLINE__ osal_void mac_mib_set_explicit_trans_bf_csi_feedback_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_transmit_beamforming_csi_feedback_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8
mac_mib_get_explicit_trans_bf_csi_feedback_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_transmit_beamforming_csi_feedback_option_implemented;
}

static INLINE__ osal_void mac_mib_set_explicit_non_compressed_bf_feedback_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_non_compressed_beamforming_feedback_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8
mac_mib_get_explicit_non_compressed_bf_feedback_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_non_compressed_beamforming_feedback_option_implemented;
}

static INLINE__ osal_void mac_mib_set_explicit_compressed_bf_feedback_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_compressed_beamforming_feedback_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8
mac_mib_get_explicit_compressed_bf_feedback_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_explicit_compressed_beamforming_feedback_option_implemented;
}

static INLINE__ osal_void mac_mib_set_num_bf_csi_support_antenna(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_number_beam_forming_csi_support_antenna = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_num_bf_csi_support_antenna(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config.dot11_number_beam_forming_csi_support_antenna;
}

static INLINE__ osal_void mac_mib_set_num_non_compressed_bf_matrix_support_antenna(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_number_non_compressed_beamforming_matrix_support_antenna = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_num_non_compressed_bf_matrix_support_antenna(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_number_non_compressed_beamforming_matrix_support_antenna;
}

static INLINE__ osal_void mac_mib_set_num_compressed_bf_matrix_support_antenna(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_number_compressed_beamforming_matrix_support_antenna = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_num_compressed_bf_matrix_support_antenna(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.wlan_mib_txbf_config
        .dot11_number_compressed_beamforming_matrix_support_antenna;
}

static INLINE__ osal_void mac_mib_set_vht_su_bfee_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtsu_beamformee_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_vht_su_bfee_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtsu_beamformee_option_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_su_bfer_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtsu_beamformer_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_vht_su_bfer_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtsu_beamformer_option_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_mu_bfee_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtmu_beamformee_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_vht_mu_bfee_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtmu_beamformee_option_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_mu_bfer_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtmu_beamformer_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_vht_mu_bfer_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vhtmu_beamformer_option_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_num_sounding_dimensions(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vht_number_sounding_dimensions = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_vht_num_sounding_dimensions(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.wlan_mib_vht_txbf_config.dot11_vht_number_sounding_dimensions;
}

static INLINE__ osal_void mac_mib_set_vht_channel_width_option_implemented(hmac_vap_stru *hmac_vap,
    wlan_mib_vht_supp_width_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_channel_width_option_implemented = val;
}

static INLINE__ wlan_mib_vht_supp_width_enum_uint8
mac_mib_get_vht_channel_width_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_channel_width_option_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_short_gi_option_in80_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_short_gi_option_in80_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_vht_short_gi_option_in80_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_short_gi_option_in80_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_ldpc_coding_option_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vhtldpc_coding_option_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_vht_ldpc_coding_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vhtldpc_coding_option_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_tx_stbc_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_tx_stbc_option_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_vht_tx_stbc_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_tx_stbc_option_implemented;
}

static INLINE__ osal_void mac_mib_set_vht_rx_stbc_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_rx_stbc_option_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_vht_rx_stbc_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_vht.dot11_vht_rx_stbc_option_implemented;
}

static INLINE__ osal_void mac_mib_set_ht_greenfield_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_ht_greenfield_option_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_ht_greenfield_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_ht_greenfield_option_implemented;
}

static INLINE__ osal_void mac_mib_set_short_gi_option_in_twenty_implemented(hmac_vap_stru *hmac_vap,
    oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_short_gi_option_in_twenty_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_short_gi_option_in_twenty_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_short_gi_option_in_twenty_implemented;
}

static INLINE__ osal_void mac_mib_set_ldpc_coding_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_ldpc_coding_option_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_ldpc_coding_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_ldpc_coding_option_implemented;
}

static INLINE__ void mac_mib_set_LDPCCodingOptionActivated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_ldpc_coding_option_activated = val;
}

static INLINE__ osal_u8 mac_mib_get_LDPCCodingOptionActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_ldpc_coding_option_activated;
}

static INLINE__ osal_void mac_mib_set_tx_stbc_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_stbc_option_implemented = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_tx_stbc_option_implemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_stbc_option_implemented;
}

static INLINE__ void mac_mib_set_TxSTBCOptionActivated(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_stbc_option_activated = val;
}

static INLINE__ oal_bool_enum_uint8 mac_mib_get_TxSTBCOptionActivated(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_stbc_option_activated;
}

static INLINE__ osal_void mac_mib_set_rx_stbc_option_implemented(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_rx_stbc_option_implemented = val;
}

static INLINE__ osal_u8 mac_mib_get_rx_stbc_option_implemented(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_rx_stbc_option_implemented;
}

static INLINE__ osal_void mac_mib_set_highest_supported_data_rate(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_highest_supported_data_rate = val;
}

static INLINE__ osal_u32 mac_mib_get_highest_supported_data_rate(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_highest_supported_data_rate;
}

static INLINE__ osal_void mac_mib_set_tx_mcs_set_defined(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_mcs_set_defined = val;
}

static INLINE__ osal_u8 mac_mib_get_tx_mcs_set_defined(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_mcs_set_defined;
}

static INLINE__ osal_void mac_mib_set_tx_rx_mcs_set_not_equal(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_rx_mcs_set_not_equal = val;
}

static INLINE__ osal_u8 mac_mib_get_tx_rx_mcs_set_not_equal(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_rx_mcs_set_not_equal;
}

static INLINE__ osal_void mac_mib_set_tx_maximum_num_spatial_streams_supported(hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_maximum_number_spatial_streams_supported = val;
}

static INLINE__ osal_u32 mac_mib_get_tx_maximum_num_spatial_streams_supported(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_maximum_number_spatial_streams_supported;
}

static INLINE__ osal_void mac_mib_set_tx_unequal_modulation_supported(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 val)
{
    hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_unequal_modulation_supported = val;
}

static INLINE__ osal_u8 mac_mib_get_tx_unequal_modulation_supported(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_phy_ht.dot11_tx_unequal_modulation_supported;
}

static INLINE__ void mac_mib_set_SupportedMCSTxValue(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u8 val)
{
    hmac_vap->mib_info->supported_mcs.supported_mcstx.dot11_supported_mcs_tx_value[index] = val;
}

static INLINE__ osal_u8 mac_mib_get_SupportedMCSTxValue(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->supported_mcs.supported_mcstx.dot11_supported_mcs_tx_value[index];
}

static INLINE__ osal_u8 *mac_mib_get_SupportedMCSTx(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->supported_mcs.supported_mcstx.dot11_supported_mcs_tx_value;
}

static INLINE__ osal_void mac_mib_set_supported_mcs_rx_value(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u8 val)
{
    hmac_vap->mib_info->supported_mcs.supported_mcsrx.dot11_supported_mcs_rx_value[index] = val;
}

static INLINE__ osal_u8 mac_mib_get_SupportedMCSRxValue(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->supported_mcs.supported_mcsrx.dot11_supported_mcs_rx_value[index];
}

static INLINE__ osal_u8 *mac_mib_get_supported_mcs_rx_value(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->supported_mcs.supported_mcsrx.dot11_supported_mcs_rx_value;
}


static INLINE__ osal_void mac_mib_set_edca_table_index(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_index = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_edca_table_index(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_index;
}

static INLINE__ osal_void mac_mib_set_edca_table_cwmin(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_c_wmin = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_edca_table_cwmin(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_c_wmin;
}

static INLINE__ osal_void mac_mib_set_edca_table_cwmax(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_c_wmax = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_edca_table_cwmax(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_c_wmax;
}

static INLINE__ osal_void mac_mib_set_edca_table_aifsn(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_aifsn = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_edca_table_aifsn(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_aifsn;
}

static INLINE__ osal_void mac_mib_set_edca_table_txop_limit(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_txop_limit = (osal_u16)val;
}

static INLINE__ osal_u32 mac_mib_get_edca_table_txop_limit(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_txop_limit;
}

static INLINE__ osal_void mac_mib_set_edca_table_mandatory(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_mandatory = val;
}

static INLINE__ osal_u8 mac_mib_get_edca_table_mandatory(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_edca[index].dot11_edca_table_mandatory;
}

static INLINE__ osal_void mac_mib_set_qap_edca_table_cwmin(const hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmin = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_edca_table_cwmin(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmin;
}

static INLINE__ osal_void mac_mib_set_qap_edca_table_cwmax(const hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmax = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_edca_table_cwmax(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_c_wmax;
}

static INLINE__ osal_void mac_mib_set_qap_edca_table_aifsn(const hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_aifsn = (osal_u8)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_edca_table_aifsn(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_aifsn;
}

static INLINE__ osal_void mac_mib_set_qap_edca_table_txop_limit(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_txop_limit = (osal_u16)val;
}

static INLINE__ osal_u32 mac_mib_get_qap_edca_table_txop_limit(const hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_txop_limit;
}

#ifdef _PRE_WLAN_FEATURE_11AX
static INLINE__ osal_void mac_mib_set_txop_duration_rts_threshold(const hmac_vap_stru *hmac_vap, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_txop_duration_rts_threshold = val;
}

static INLINE__ osal_u32 mac_mib_get_txop_duration_rts_threshold(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_he_sta_config.dot11_txop_duration_rts_threshold;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */

static INLINE__ osal_void mac_mib_set_qap_edca_table_mandatory(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_mandatory = val;
}

static INLINE__ osal_u8 mac_mib_get_qap_edca_table_mandatory(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_mandatory;
}

static INLINE__ void mac_mib_set_QAPEDCATableMSDULifetime(hmac_vap_stru *hmac_vap, osal_u8 index, osal_u32 val)
{
    hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_msdu_lifetime = (osal_u16)val;
}

static INLINE__ osal_u32 mac_mib_get_QAPEDCATableMSDULifetime(hmac_vap_stru *hmac_vap, osal_u8 index)
{
    return hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac[index].dot11_qapedca_table_msdu_lifetime;
}

#ifdef _PRE_WLAN_FEATURE_WUR_TX
/* ****************************************************************************
 功能描述  : 设置MIB项 dot11WUROptionImplemented 的值
**************************************************************************** */
static INLINE__ osal_void mac_mib_set_dot11WUROptionImplemented(hmac_vap_stru *hmac_vap, osal_u8 val)
{
    hmac_vap->mib_info->wlan_mib_sta_config.dot11_wur_option_implemented = val;
}

/* ****************************************************************************
 功能描述  : 获取MIB项 dot11WUROptionImplemented 的值
**************************************************************************** */
static INLINE__ osal_u8 mac_mib_get_dot11WUROptionImplemented(const hmac_vap_stru *hmac_vap)
{
    return hmac_vap->mib_info->wlan_mib_sta_config.dot11_wur_option_implemented;
}
#endif

/* ****************************************************************************
 函 数 名  : mac_fcs_get_protect_type
 功能描述  : 根据VAP模式获取保护帧类型

**************************************************************************** */
static INLINE__ hal_fcs_protect_type_enum_uint8 mac_fcs_get_protect_type(const hmac_vap_stru *hmac_vap)
{
    hal_fcs_protect_type_enum_uint8 protect_type;

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        protect_type = HAL_FCS_PROTECT_TYPE_SELF_CTS;
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        protect_type = HAL_FCS_PROTECT_TYPE_NULL_DATA;
    } else {
        protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) {
        protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }
#endif

    return protect_type;
}

static INLINE__ osal_u8 mac_fcs_get_protect_cnt(const hmac_vap_stru *hmac_vap)
{
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        return HAL_FCS_PROTECT_CNT_1;
    }

    /* sta模式主要靠硬件超时来完成one pkt发送，次数可以尽量往高配置，btcoex普通音乐下需要配置更长时间 */
    return HAL_FCS_PROTECT_CNT_20;
}

/* ****************************************************************************
    函数声明
**************************************************************************** */
mac_wme_param_stru *hmac_get_wmm_cfg_etc(wlan_vap_mode_enum_uint8 vap_mode);
void hmac_vap_init_rates_etc(hmac_vap_stru *hmac_vap);
osal_u8 hmac_vap_set_bw_check(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth);
osal_u32 hmac_mib_get_bss_type_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param);
osal_u32 hmac_mib_set_bss_type_etc(hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param);
osal_u32 hmac_mib_set_ssid_etc(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param);
osal_u32 hmac_mib_get_ssid_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param);
osal_u32 hmac_mib_get_dtim_period_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param);
osal_u32 hmac_mib_set_dtim_period_etc(hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param);
osal_u32 hmac_mib_get_shpreamble_etc(hmac_vap_stru *hmac_vap, osal_u8 *len, osal_u8 *param);
osal_u32 hmac_mib_set_shpreamble_etc(hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param);
osal_u32 hmac_vap_find_user_by_macaddr_etc(hmac_vap_stru *hmac_vap, const osal_u8 *sta_mac_addr,
    osal_u16 *user_idx);
osal_u32 hmac_device_find_user_by_macaddr_etc(osal_u8 *sta_mac_addr, osal_u16 *user_idx);
osal_u8 hmac_vap_get_ap_usr_opern_bandwidth(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u8 hmac_vap_get_bandwith(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 bss_cap);
void hmac_vap_set_tx_power_etc(hmac_vap_stru *hmac_vap, osal_u8 tx_power);
void hmac_vap_set_assoc_id_etc(hmac_vap_stru *hmac_vap, osal_u16 assoc_vap_id);
void hmac_vap_set_aid_etc(hmac_vap_stru *hmac_vap, osal_u16 aid);
void hmac_vap_set_multi_user_idx_etc(hmac_vap_stru *hmac_vap, osal_u16 multi_user_idx);
void hmac_vap_set_rx_nss_etc(hmac_vap_stru *hmac_vap, wlan_nss_enum_uint8 rx_nss);
void hmac_vap_set_wmm_params_update_count_etc(hmac_vap_stru *hmac_vap, osal_u8 update_count);
void hmac_vap_set_peer_obss_scan_etc(hmac_vap_stru *hmac_vap, osal_u8 value);
void hmac_vap_set_hide_ssid_etc(hmac_vap_stru *hmac_vap, osal_u8 value);
osal_u8 hmac_vap_get_peer_obss_scan_etc(const hmac_vap_stru *hmac_vap);
osal_u32 hmac_vap_save_app_ie_etc(hmac_vap_stru *hmac_vap, oal_app_ie_stru *app_ie, app_ie_type_uint8 type);
osal_void hmac_vap_clear_app_ie_etc(hmac_vap_stru *hmac_vap, app_ie_type_uint8 type);
void hmac_vap_cap_init_legacy(hmac_vap_stru *hmac_vap);
osal_u32 hmac_vap_cap_init_htvht(hmac_vap_stru *hmac_vap);
osal_u32 hmac_vap_config_vht_ht_mib_by_protocol_etc(hmac_vap_stru *hmac_vap);
void hmac_vap_init_rx_nss_by_protocol_etc(hmac_vap_stru *hmac_vap);
wlan_bw_cap_enum_uint8 hmac_vap_bw_mode_to_bw(wlan_channel_bandwidth_enum_uint8 mode);
void hmac_vap_state_change_etc(hmac_vap_stru *hmac_vap, mac_vap_state_enum_uint8 vap_state);
void hmac_vap_get_bandwidth_cap_etc(const hmac_vap_stru *hmac_vap, wlan_bw_cap_enum_uint8 *cap);
wlan_prot_mode_enum_uint8 mac_vap_get_user_protection_mode_etc(const hmac_vap_stru *mac_vap_sta,
    const hmac_user_stru *hmac_user);
oal_bool_enum mac_protection_lsigtxop_check_etc(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user);
osal_void mac_protection_set_lsig_txop_mechanism_etc(hmac_vap_stru *hmac_vap, osal_u8 flag);
osal_u32 hmac_vap_add_wep_key(hmac_vap_stru *hmac_vap, osal_u8 key_idx, osal_u8 key_len, osal_u8 *key);
osal_u32 hmac_vap_init_privacy_etc(hmac_vap_stru *hmac_vap, mac_conn_security_stru *conn_sec);
hmac_user_stru *hmac_vap_get_user_by_addr_etc(hmac_vap_stru *hmac_vap, const osal_u8 *mac_addr, size_t mac_len);
osal_u32 hmac_vap_set_security(hmac_vap_stru *hmac_vap, mac_beacon_param_stru *beacon_param);
osal_u32 hmac_vap_add_key_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 key_id,
    mac_key_params_stru *key);
void hmac_vap_init_user_security_port_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
void hmac_device_set_vap_id_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap, osal_u8 vap_idx,
    const hmac_cfg_add_vap_param_stru *vap_param, osal_u8 is_add_vap);
osal_u32 hmac_device_calc_up_vap_num_etc(hmac_device_stru *hmac_device);
osal_u32 hmac_device_find_2up_vap_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_up_vap1,
    hmac_vap_stru **hmac_up_vap2);
#ifdef _PRE_WLAN_FEATURE_SMPS
extern osal_u32 hmac_device_find_smps_mode_en(hmac_device_stru *hmac_device,
    wlan_mib_mimo_power_save_enum_uint8 smps_mode);
#endif
void hmac_device_set_channel_etc(hmac_device_stru *hmac_device, mac_cfg_channel_param_stru *channel_param);
void hmac_device_get_channel_etc(hmac_device_stru *hmac_device, mac_cfg_channel_param_stru *channel_param);
extern osal_u32 mac_mib_set_station_id_etc(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param);
extern osal_u32 mac_vap_init_etc(hmac_vap_stru *hmac_vap, osal_u8 chip_id, osal_u8 device_id, osal_u8 vap_id,
    hmac_cfg_add_vap_param_stru *param);
extern void mac_sta_init_bss_rates_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr);
extern osal_u32 hmac_vap_exit_etc(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device);
extern osal_u32 hmac_vap_del_user_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_u32 hmac_vap_add_assoc_user_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);

extern osal_u32 mac_vap_set_bssid_etc(hmac_vap_stru *hmac_vap, osal_u8 *bssid);
extern osal_u32 mac_vap_set_current_channel_etc(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    osal_u8 channel);
extern osal_u32 hmac_vap_init_wme_param_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_vap_init_by_protocol_etc(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 protocol);
#ifdef _PRE_WIFI_DEBUG
extern oal_bool_enum_uint8 mac_vap_check_bss_cap_info_phy_ap_etc(osal_u16 cap_info, hmac_vap_stru *hmac_vap);
#endif
extern void hmac_vap_change_mib_by_bandwidth_etc(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    wlan_channel_bandwidth_enum_uint8 bandwidth);
extern void hmac_vap_set_uapsd_cap_etc(hmac_vap_stru *hmac_vap, osal_u8 uapsd_cap);
extern osal_u32 mac_dump_protection_etc(hmac_vap_stru *hmac_vap);
osal_void mac_mib_set_wep_etc(hmac_vap_stru *hmac_vap, osal_u8 key_id, osal_u8 key_value);
extern osal_u32 mac_device_find_up_vap_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap);
extern osal_u32 mac_device_find_up_ap_etc(hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap);
#ifdef _PRE_WLAN_FEATURE_P2P
extern osal_u32 mac_device_find_up_p2p_go_etc(hmac_device_stru *hmac_device, hmac_vap_stru **phmac_vap);
#endif
extern osal_u32 mac_device_is_p2p_connected_etc(hmac_device_stru *hmac_device);

extern void mac_blacklist_free_pointer(hmac_vap_stru *hmac_vap, mac_blacklist_info_stru *blacklist_info);
extern mac_tcp_ack_buf_switch_stru *mac_get_pst_tcp_ack_buf_switch(osal_void);
extern mac_small_amsdu_switch_stru *mac_get_pst_small_amsdu_switch(osal_void);
extern mac_rx_buffer_size_stru *mac_get_pst_rx_buffer_size_stru(osal_void);
osal_void mac_vap_init_rates_by_protocol_etc(hmac_vap_stru *hmac_vap, wlan_protocol_enum_uint8 vap_protocol,
    mac_data_rate_stru *rates);
osal_u8 *mac_vap_get_uapsd_cap(osal_void);
osal_u32 hmac_vap_check_ap_usr_opern_bandwidth(const hmac_vap_stru *mac_sta, const hmac_user_stru *hmac_user);
osal_u8 hmac_vap_get_default_key_id_etc(const hmac_vap_stru *hmac_vap);
osal_u8 *hmac_vap_get_mac_addr_etc(const hmac_vap_stru *hmac_vap);
osal_u8 hmac_vap_protection_autoprot_is_enabled_etc(const hmac_vap_stru *hmac_vap);

hmac_vap_stru *mac_device_find_another_up_vap_etc(const hmac_device_stru *hmac_device, osal_u8 vap_id_self);
osal_u32 mac_device_find_up_sta_etc(const hmac_device_stru *hmac_device, hmac_vap_stru **hmac_vap);
hmac_vap_stru *mac_find_up_legacy_sta_vap(osal_void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
