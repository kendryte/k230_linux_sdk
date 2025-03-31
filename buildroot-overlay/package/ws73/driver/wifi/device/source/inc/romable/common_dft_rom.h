/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: common_dft_rom.c 的头文件
 * Date: 2022-03-08
 */

#ifndef __COMMON_DFT_ROM_H__
#define __COMMON_DFT_ROM_H__

#include "osal_types.h"
#include "soc_diag_msg_id.h"
#include "wlan_util_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 特性宏的缩写见gst_oam_feature_list */
typedef enum {
    OAM_SF_SCAN = 0,
    OAM_SF_AUTH,
    OAM_SF_ASSOC,
    OAM_SF_FRAME_FILTER,
    OAM_SF_WMM,

    OAM_SF_DFS = 5,
    OAM_SF_NETWORK_MEASURE,
    OAM_SF_ENTERPRISE_VO,
    OAM_SF_HOTSPOTROAM,
    OAM_SF_NETWROK_ANNOUNCE,

    OAM_SF_NETWORK_MGMT = 10,
    OAM_SF_NETWORK_PWS,
    OAM_SF_PROXYARP,
    OAM_SF_TDLS,
    OAM_SF_CALIBRATE,

    OAM_SF_EQUIP_TEST = 15,
    OAM_SF_CRYPTO,
    OAM_SF_WPA,
    OAM_SF_WEP,
    OAM_SF_WPS,

    OAM_SF_PMF = 20,
    OAM_SF_WAPI,
    OAM_SF_BA,
    OAM_SF_AMPDU,
    OAM_SF_AMSDU,

    OAM_SF_STABILITY = 25,
    OAM_SF_TCP_OPT,
    OAM_SF_ACS,
    OAM_SF_AUTORATE,
    OAM_SF_RTS,
    OAM_SF_TXBF,

    OAM_SF_DYN_RECV = 30, /* dynamin recv */
    OAM_SF_VIVO,          /* video_opt voice_opt */
    OAM_SF_MULTI_USER,
    OAM_SF_TRAFFIC,
    OAM_SF_ANTI_INTF,

    OAM_SF_EDCA = 35,
    OAM_SF_SMART_ANTENNA,
    OAM_SF_TPC,
    OAM_SF_TX_CHAIN,
    OAM_SF_RSSI,

    OAM_SF_WOW = 40,
    OAM_SF_GREEN_AP,
    OAM_SF_PWR, /* psm uapsd fastmode */
    OAM_SF_SMPS,
    OAM_SF_TXOP,

    OAM_SF_WIFI_BEACON = 45,
    OAM_SF_KA_AP, /* keep alive ap */
    OAM_SF_MULTI_VAP,
    OAM_SF_2040, /* 20m+40m coex */
    OAM_SF_DBAC,

    OAM_SF_PROXYSTA = 50,
    OAM_SF_UM,  /* user managment */
    OAM_SF_P2P, /* P2P 特性 */
    OAM_SF_M2U,
    OAM_SF_IRQ, /* top half */

    OAM_SF_TX = 55,
    OAM_SF_RX,
    OAM_SF_DUG_COEX,
    OAM_SF_CFG, /* wal dmac config函数 */
    OAM_SF_FRW, /* frw层 */

    OAM_SF_KEEPALIVE = 60,
    OAM_SF_COEX,
    OAM_SF_HS20, /* HotSpot 2.0特性 */
    OAM_SF_MWO_DET,
    OAM_SF_CCA_OPT,

    OAM_SF_ROAM = 65, /* roam module, #ifdef _PRE_WLAN_FEATURE_ROAM */
    OAM_SF_DFT,
    OAM_SF_DFR,
    OAM_SF_RRM,

    OAM_SF_OPMODE = 70,
    OAM_SF_M2S,
    OAM_SF_DBDC,
    OAM_SF_HILINK,
    OAM_SF_WDS, /* WDS 特性 */

    OAM_SF_WMMAC = 75,
    OAM_SF_USER_EXTEND,
    OAM_SF_PKT_CAP, /* 抓包特性 */
    OAM_SF_SOFT_CRYPTO,

    OAM_SF_CAR,  /* 限速特性  */
    OAM_SF_11AX, /* 11AX 特性 */
    OAM_SF_CSA,
    OAM_SF_CONN, /* ONT日志 */
    OAM_SF_CHAN, /* ONT日志 */
    OAM_SF_CUSTOM,
    OAM_SF_ALG_COMMON,
    OAM_SF_ALG_TX_TB,

    OAM_SF_QOS,
    OAM_SF_SDP,
    OAM_SF_SCHEDULE,
    OAM_SF_AUTOAGGR,
    OAM_SF_INTRF_MODE,
    OAM_SF_RESERVE7,
    OAM_SF_RESERVE8,
    OAM_SF_RESERVE9,
    OAM_SF_RESERVE10,
    OAM_SF_ANY, /* rifs protection shortgi frag datarate countrycode
                    coustom_security startup_time lsig monitor wds
                    hidessid */

#ifdef _PRE_WLAN_FEATURE_11V
    OAM_SF_BSSTRANSITION,
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    OAM_SF_FTM,
#endif
#ifdef _PRE_WLAN_FEATURE_CHBA
    OAM_SF_CHBA,
#endif
    OAM_SF_SR,
    OAM_SF_DNB,
    OAM_SF_APF,
    OAM_SF_LATENCY,
    OAM_SF_SLP,
    OAM_SOFTWARE_FEATURE_BUTT
} oam_feature_enum;

/* OTA上报事件类型 */
typedef enum {
    /* WiFi OTA消息类型[0..150] */
    /* 默认关闭上报 */
    OAM_OTA_TYPE_RX_DSCR,     /* 接收描述符 */
    OAM_OTA_TYPE_TX_DSCR,     /* 发送描述符 */
    OAM_OTA_TYPE_80211_FRAME, /* 80211帧(不包括beacon) */
    OAM_OTA_TYPE_BEACON,      /* beacon */
    OAM_OTA_TYPE_ETH_FRAME,   /* 以太网帧 */
    OAM_OTA_TYPE_RX_DMAC_CB,
    OAM_OTA_TYPE_RX_HMAC_CB,
    OAM_OTA_TYPE_TX_CB,
    OAM_OTA_TYPE_TIMER_TRACK, /* 定时器创建删除的行号和文件号追踪,已废弃 */

    /* 默认打开上报 */
    OAM_OTA_TYPE_IRQ,         /* 中断信息 */
    OAM_OTA_TYPE_TIMER,       /* 软件定时器信息 */
    OAM_OTA_TYPE_MEMPOOL,     /* 某一内存池及所有子池使用信息 */
    OAM_OTA_TYPE_MEMBLOCK,    /* 某一内存池的所有内存块使用信息及内容，或者任意连续内存块的内容 */
    OAM_OTA_TYPE_EVENT_QUEUE, /* 当前所有存在事件的事件队列中的事件个数和事件头信息 */
    OAM_OTA_TYPE_MPDU_NUM,
    OAM_OTA_TYPE_PHY_STAT,          /* phy收发包统计值 */
    OAM_OTA_TYPE_MACHW_STAT,        /* mac收发包统计值 */
    OAM_OTA_TYPE_MGMT_STAT,         /* 管理帧统计 */
    OAM_OTA_TYPE_DBB_ENV_PARAM,     /* 空口环境类维测参数 */
    OAM_OTA_TYPE_USR_QUEUE_STAT,    /* 用户队列统计信息 */
    OAM_OTA_TYPE_VAP_STAT,          /* vap吞吐相关统计信息 */
    OAM_OTA_TYPE_USER_THRPUT_PARAM, /* 影响用户实时吞吐相关的统计信息 */
    OAM_OTA_TYPE_AMPDU_STAT,        /* ampdu业务流程统计信息 */
    OAM_OTA_TYPE_HARDWARE_INFO,     /* hal mac相关信息 */
    OAM_OTA_TYPE_USER_QUEUE_INFO,   /* 用户队列信息 */

    /* VAP与USER等结构体的信息，上报整个结构体内存 */
    OAM_OTA_TYPE_HMAC_VAP,
    OAM_OTA_TYPE_DMAC_VAP,
    OAM_OTA_TYPE_HMAC_USER,
    OAM_OTA_TYPE_MAC_USER,
    OAM_OTA_TYPE_DMAC_USER,

    /* hmac 与 dmac vap中部分成员的大小 */
    OAM_OTA_TYPE_HMAC_VAP_MEMBER_SIZE,
    OAM_OTA_TYPE_DMAC_VAP_MEMBER_SIZE,

    /* 三种级别的统计信息,这三个必放到一起!!! */
    OAM_OTA_TYPE_DEV_STAT_INFO,
    OAM_OTA_TYPE_VAP_STAT_INFO,
    OAM_OTA_TYPE_USER_STAT_INFO,
    /* 02寄存器列表 */
    OAM_OTA_TYPE_PHY_BANK1_INFO, /* PHY寄存器 BANK1 信息 */
    OAM_OTA_TYPE_PHY_BANK2_INFO, /* PHY寄存器 BANK2 信息 */
    OAM_OTA_TYPE_PHY_BANK3_INFO, /* PHY寄存器 BANK3 信息 */
    OAM_OTA_TYPE_PHY_BANK4_INFO, /* PHY寄存器 BANK4 信息 */
    /* 03新增PHY寄存器5、6 */
    OAM_OTA_TYPE_PHY_BANK5_INFO, /* PHY寄存器 BANK5 信息 */
    OAM_OTA_TYPE_PHY_BANK6_INFO, /* PHY寄存器 BANK6 信息 */
    OAM_OTA_TYPE_PHY0_CTRL_INFO, /* PHY寄存器 PHY0_CTRL 信息 */
    OAM_OTA_TYPE_PHY1_CTRL_INFO, /* PHY寄存器 PHY1_CTRL 信息 */

    OAM_OTA_TYPE_MAC_CTRL0_INFO, /* MAC寄存器 CTRL0 信息 */
    OAM_OTA_TYPE_MAC_CTRL1_INFO, /* MAC寄存器 CTRL1 信息 */
    OAM_OTA_TYPE_MAC_CTRL2_INFO, /* MAC寄存器 CTRL2 信息 */
    OAM_OTA_TYPE_MAC_RD0_INFO,   /* MAC寄存器 RD0 信息（存在几字节信息不能读取） */
    OAM_OTA_TYPE_MAC_RD1_INFO,   /* MAC寄存器 RD1 信息 */

    OAM_OTA_TYPE_RF_REG_INFO,  /* RF寄存器 */
    OAM_OTA_TYPE_SOC_REG_INFO, /* SOC寄存器 */

    /* 03 MAC寄存器列表 */
    OAM_OTA_TYPE_MAC_CTRL0_BANK_INFO, /* MAC寄存器 CTRL0 信息 */
    OAM_OTA_TYPE_MAC_CTRL1_BANK_INFO, /* MAC寄存器 CTRL1 信息 */
    OAM_OTA_TYPE_MAC_CTRL2_BANK_INFO, /* MAC寄存器 CTRL2 信息 */
    OAM_OTA_TYPE_MAC_RD0_BANK_INFO,   /* MAC寄存器 RD0 信息（存在几字节信息不能读取） */
    OAM_OTA_TYPE_MAC_RD1_BANK_INFO,   /* MAC寄存器 RD1 信息 */
    OAM_OTA_TYPE_MAC_LUT0_BANK_INFO,  /* MAC寄存器 LUT0 信息 */
    OAM_OTA_TYPE_MAC_WLMAC_CTRL_INFO, /* MAC寄存器 wlmac_ctrl 信息 */
    OAM_OTA_TYPE_RF0_REG_INFO,        /* 03 RF0寄存器 */
    OAM_OTA_TYPE_RF1_REG_INFO,        /* 03 RF1寄存器 */

    /* 03 vector维测新增上报 */
    OAM_OTA_TYPE_RX_HDR_WORD0_INFO, /* trailer word0信息 */
    OAM_OTA_TYPE_RX_HDR_WORD1_INFO, /* trailer word1信息 */
    OAM_OTA_TYPE_RX_HDR_WORD2_INFO, /* trailer word2信息 */
    OAM_OTA_TYPE_RX_HDR_WORD3_INFO, /* trailer word3信息 */
    OAM_OTA_TYPE_RX_HDR_WORD4_INFO, /* trailer word4信息 */
    OAM_OTA_TYPE_RX_HDR_WORD5_INFO, /* trailer word5信息 */
    OAM_OTA_TYPE_RX_HDR_WORD6_INFO, /* trailer word6信息 */
    OAM_OTA_TYPE_RX_HDR_WORD7_INFO, /* trailer word7信息 */

    /* 03 trailer维测新增上报 */
    OAM_OTA_TYPE_RX_TRLR_WORD0_INFO,  /* trailer word0信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD1_INFO,  /* trailer word1信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD2_INFO,  /* trailer word2信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD3_INFO,  /* trailer word3信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD4_INFO,  /* trailer word4信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD5_INFO,  /* trailer word5信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD6_INFO,  /* trailer word6信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD7_INFO,  /* trailer word7信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD8_INFO,  /* trailer word8信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD9_INFO,  /* trailer word9信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD10_INFO, /* trailer word10信息 */
    OAM_OTA_TYPE_RX_TRLR_WORD11_INFO, /* trailer word11信息 */

    /* 03新增PHY寄存器ABB_CALI_WL_CTRL */
    OAM_OTA_TYPE_ABB_CALI_WL_CTRL0_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL0 信息 */
    OAM_OTA_TYPE_ABB_CALI_WL_CTRL1_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL1 信息 */

    /* 03 SOC维测新增上报 */
    OAM_OTA_TYPE_SOC_RF_W_C0_CTL_INFO, /* SOC寄存器 RF_W_C0_CTL 信息 */
    OAM_OTA_TYPE_SOC_RF_W_C1_CTL_INFO, /* SOC寄存器 RF_W_C1_CTL 信息 */
    OAM_OTA_TYPE_SOC_W_CTL_INFO,       /* SOC寄存器 W_CTL 信息 */
    OAM_OTA_TYPE_SOC_COEX_CTL_INFO,    /* SOC寄存器 COEX_CTL 信息 */
    OAM_OTA_TYPE_SOC_RESERVE1_INFO,    /* SOC寄存器 Reserve1 信息 */
    OAM_OTA_TYPE_SOC_RESERVE2_INFO,    /* SOC寄存器 Reserve2 信息 */
    OAM_OTA_TYPE_SOC_RESERVE3_INFO,    /* SOC寄存器 Reserve3 信息 */
    OAM_OTA_TYPE_SOC_RESERVE4_INFO,    /* SOC寄存器 Reserve4 信息 */

    /* 03PILOT寄存器 */
    OAM_OTA_TYPE_PHY_BANK1_PILOT_INFO, /* PHY寄存器 BANK1 信息 */
    OAM_OTA_TYPE_PHY_BANK2_PILOT_INFO, /* PHY寄存器 BANK2 信息 */
    OAM_OTA_TYPE_PHY_BANK3_PILOT_INFO, /* PHY寄存器 BANK3 信息 */
    OAM_OTA_TYPE_PHY_BANK4_PILOT_INFO, /* PHY寄存器 BANK4 信息 */
    OAM_OTA_TYPE_PHY_BANK5_PILOT_INFO, /* PHY寄存器 BANK5 信息 */
    OAM_OTA_TYPE_PHY_BANK6_PILOT_INFO, /* PHY寄存器 BANK6 信息 */
    OAM_OTA_TYPE_PHY0_CTRL_PILOT_INFO, /* PHY寄存器 PHY0_CTRL 信息 */
    OAM_OTA_TYPE_PHY1_CTRL_PILOT_INFO, /* PHY寄存器 PHY1_CTRL 信息 */

    OAM_OTA_TYPE_MAC_CTRL0_BANK_PILOT_INFO, /* MAC寄存器 CTRL0 信息 */
    OAM_OTA_TYPE_MAC_CTRL1_BANK_PILOT_INFO, /* MAC寄存器 CTRL1 信息 */
    OAM_OTA_TYPE_MAC_CTRL2_BANK_PILOT_INFO, /* MAC寄存器 CTRL2 信息 */
    OAM_OTA_TYPE_MAC_RD0_BANK_PILOT_INFO,   /* MAC寄存器 RD0 信息（存在几字节信息不能读取） */
    OAM_OTA_TYPE_MAC_RD1_BANK_PILOT_INFO,   /* MAC寄存器 RD1 信息 */
    OAM_OTA_TYPE_MAC_LUT0_BANK_PILOT_INFO,  /* MAC寄存器 LUT0 信息 */
    OAM_OTA_TYPE_MAC_WLMAC_CTRL_PILOT_INFO, /* MAC寄存器 wlmac_ctrl 信息 */

    OAM_OTA_TYPE_ABB_CALI_WL_CTRL0_PILOT_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL0 信息 */
    OAM_OTA_TYPE_ABB_CALI_WL_CTRL1_PILOT_INFO, /* PHY寄存器 ABB_CALI_WL_CTRL1 信息 */

    OAM_OTA_TYPE_SOC_RF_W_C0_CTL_PILOT_INFO, /* SOC寄存器 RF_W_C0_CTL 信息 */
    OAM_OTA_TYPE_SOC_RF_W_C1_CTL_PILOT_INFO, /* SOC寄存器 RF_W_C1_CTL 信息 */
    OAM_OTA_TYPE_SOC_W_CTL_PILOT_INFO,       /* SOC寄存器 W_CTL 信息 */
    OAM_OTA_TYPE_SOC_COEX_CTL_PILOT_INFO,    /* SOC寄存器 COEX_CTL 信息 */

    OAM_OTA_TYPE_RX_DSCR_PILOT, /* 接收描述符 */
    OAM_OTA_TYPE_TX_DSCR_PILOT, /* 发送描述符 */

    OAM_OTA_TYPE_MAC_CTRL3_BANK_PILOT_INFO, /* MAC寄存器 CTRL3 信息 */
    OAM_OTA_TYPE_MAC_CTRL4_BANK_PILOT_INFO, /* MAC寄存器 CTRL4 信息 */
    OAM_OTA_TYPE_MAC_RD2_BANK_PILOT_INFO, /* MAC寄存器 RD2 信息 */

    OAM_OTA_TYPE_PHY_BANK7_PILOT_INFO, /* PHY寄存器 BANK7 信息 */

    /* BT OTA消息类型[151..200] */
    OAM_OTA_TYPE_BT_TEST = 151,

    OAM_OTA_TYPE_BUTT = 255
} oam_ota_type_enum;
typedef osal_u8 oam_ota_type_enum_uint8;

typedef osal_u8 oam_ota_frame_direction_type_enum_uint8;
typedef osal_u8 oam_user_track_frame_type_enum_uint8;

/* 表明帧是属于接收流程还是发送流程的枚举 */
typedef enum {
    OAM_OTA_FRAME_DIRECTION_TYPE_TX = 0,
    OAM_OTA_FRAME_DIRECTION_TYPE_RX,

    OAM_OTA_FRAME_DIRECTION_TYPE_BUTT
} oam_ota_frame_direction_type_enum;
typedef osal_u8 oam_ota_frame_direction_type_enum_uint8;

/* 日志级别 */
typedef enum {
    OAM_LOG_LEVEL_ERROR = 1, /* ERROR级别打印 */
    OAM_LOG_LEVEL_WARNING,   /* WARNING级别打印 */
    OAM_LOG_LEVEL_INFO,      /* INFO级别打印 */

    OAM_LOG_LEVEL_BUTT
} oam_log_level_enum;
typedef osal_u8 oam_log_level_enum_uint8;

/* 单用户跟踪帧类型:数据和管理 */
typedef enum {
    OAM_USER_TRACK_FRAME_TYPE_MGMT = 0,
    OAM_USER_TRACK_FRAME_TYPE_DATA,

    OAM_USER_TRACK_FRAME_TYPE_BUTT
} oam_user_track_frame_type_enum;

typedef enum {
    DFT_SWITCH_OFF = 0,
    DFT_SWITCH_ON = 1,
    DFT_SWITCH_BUTT
} dft_switch;

/* 80211帧上报控制结构 */
typedef struct {
    osal_u8 frame_content_switch;
    osal_u8 frame_cb_switch;
    osal_u8 frame_dscr_switch;
} dft_80211_frame_ctx;

typedef enum {
    UNICAST_FRAME = 0,
    BROADCAST_FRAME,
    FRAME_MODE_BUTT
} dft_frame_mode_stru;

typedef struct {
    dft_80211_frame_ctx frame_type_ctx[OAM_OTA_FRAME_DIRECTION_TYPE_BUTT][OAM_USER_TRACK_FRAME_TYPE_BUTT];
} dft_frame_ctx;

/* 单用户跟踪控制开关结构 */
typedef struct {
    dft_frame_ctx frame_ctx[FRAME_MODE_BUTT];
} dft_user_track_ctx;

typedef struct {
    /* 当前上报日志级别 */
    oam_log_level_enum_uint8 log_level;
    /* 所有帧与描述符上报总开关 */
    osal_u8 global_frame_switch;
    osal_u8 rsv[2];
    /* 单用户跟踪管理上下文 */
    dft_user_track_ctx user_track_ctx;
} dft_mng_ctx;

typedef struct {
    osal_u8 ota_type;
    osal_u8 hdr_len;
    osal_u16 data_len;
    osal_u8 *hdr_buf;
    osal_u8 *data_buf;
} oam_ota_frame;

/* 设置80211帧开关需要的参数 */
typedef struct {
    osal_u8 frame_mode;
    osal_u8 frame_direction;
    osal_u8 frame_type;
    osal_u8 frame_switch;
    osal_u8 cb_switch;
    osal_u8 dscr_switch;
    osal_u8 user_macaddr[6];
} mac_cfg_80211_frame_switch_stru;

/* 适配HSO解析probe rsp or beacaon所需的参数 */
typedef struct {
    osal_u32 msg_id;
    osal_u8 is_probe_rsp_or_beacon;
    osal_u8 rssi;
    osal_u8 channel;
    osal_u8 rsv;
} adjust_hso_param_stru;

osal_u8 dft_get_global_frame_switch(osal_void);
oam_log_level_enum_uint8 dft_get_log_level_switch(osal_void);
osal_u32 dft_set_global_frame_switch(osal_u8 log_switch);

#ifdef _PRE_WLAN_DFT_STAT
osal_void dft_report_netbuf_cb_etc(osal_u8 *netbuf_cb, osal_u16 len, osal_u32 msg_id);
osal_void dft_report_dscr_etc(osal_u8 *dscr_addr, osal_u16 dscr_len, osal_u32 msg_id);
osal_void dft_report_80211_frame(osal_u8 *hdr_addr, osal_u8 hdr_len, const osal_u8 *frame_addr, osal_u16 frame_len,
    adjust_hso_param_stru *adjust_hso_param);
osal_void dft_report_params_etc(osal_u8 *param, osal_u16 param_len, osal_u32 msg_id);
osal_void dft_report_eth_frame_etc(osal_u8 *eth_frame, osal_u16 eth_frame_len, osal_u32 msg_id);
oam_log_level_enum_uint8 dft_get_log_level_switch(osal_void);
osal_u32 dft_get_frame_mode(osal_u8 *hdr);
osal_void dft_get_frame_report_switch(osal_u8 *hdr, osal_u8 direction, dft_80211_frame_ctx *frame_ctx);
osal_u32 dft_report_80211_frame_get_switch(osal_u8 frame_mode, osal_u8 direction, osal_u8 track_frame_type,
    dft_80211_frame_ctx *frame_ctx);
osal_u32 dft_report_80211_frame_set_switch(osal_u8 frame_mode, osal_u8 direction, osal_u8 track_frame_type,
    dft_80211_frame_ctx *frame_ctx);
osal_void dft_set_log_level(osal_u8 level);
#else
static inline osal_void dft_report_80211_frame(osal_u8 *hdr_addr, osal_u8 hdr_len, const osal_u8 *frame_addr,
    osal_u16 frame_len, adjust_hso_param_stru *adjust_hso_param)
{
    unref_param(hdr_addr);
    unref_param(hdr_len);
    unref_param(frame_addr);
    unref_param(frame_len);
    unref_param(adjust_hso_param);
    return;
}
static inline osal_void dft_report_params_etc(osal_u8 *param, osal_u16 param_len, osal_u32 msg_id)
{
    unref_param(param);
    unref_param(param_len);
    unref_param(msg_id);
    return;
}
static inline osal_void dft_report_netbuf_cb_etc(osal_u8 *netbuf_cb, osal_u16 len, osal_u32 msg_id)
{
    unref_param(netbuf_cb);
    unref_param(len);
    unref_param(msg_id);
    return;
}
static inline osal_void dft_report_eth_frame_etc(osal_u8 *eth_frame, osal_u16 eth_frame_len, osal_u32 msg_id)
{
    unref_param(eth_frame);
    unref_param(eth_frame_len);
    unref_param(msg_id);
    return;
}
static inline osal_void dft_set_log_level(osal_u8 level)
{
    unref_param(level);
    return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of common_dft.h */
