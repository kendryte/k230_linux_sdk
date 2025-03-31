/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * Description: header file.
 * Author: CompanyName
 * Create: 2023-01-04
 */
#ifndef __CUSTOMIZE_H__
#define __CUSTOMIZE_H__

#include "osal_types.h"

/* TID个数为8,0~7 */
#define WLAN_TID_MAX_NUM                    WLAN_TIDNO_BUTT

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

#define WLAN_MAC_ADDR_LEN                   6           /* MAC地址长度宏 */

#define WLAN_RF_CHANNEL_ZERO        0 /* RF0 */
#define WLAN_RF_CHANNEL_ONE         1 /* RF1 */


#define WLAN_RF_CHANNEL_NUMS   2

#define CALI_DATA_REFRESH_MASK              (0x1)
#define CALI_DATA_REUPLOAD_MASK             (0x2)
#define CALI_POWER_LVL_DBG_MASK             (0x4)
#define CALI_INTVL_MASK                     (0xe0)
#define CALI_INTVL_OFFSET                   (5)


#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* hmac to dmac定制化配置同步消息结构枚举 */
typedef enum {
    CUSTOM_CFGID_NV_ID                 = 0,
    CUSTOM_CFGID_INI_ID,
    CUSTOM_CFGID_DTS_ID,
    CUSTOM_CFGID_PRIV_INI_ID,
    CUSTOM_CFGID_MSG,
    CUSTOM_CFGID_WOW,

    CUSTOM_CFGID_BUTT
} custom_cfgid_enum;
typedef osal_u32 custom_cfgid_enum_uint32;

typedef enum {
    CUSTOM_CFGID_INI_ENDING_ID = 0,
    CUSTOM_CFGID_INI_PERF_ID = 1,
    /* 私有定制 */
    CUSTOM_CFGID_PRIV_INI_BW_MAX_WITH_ID = 2,
    CUSTOM_CFGID_PRIV_INI_SU_BFER_ID = 3,
    CUSTOM_CFGID_PRIV_INI_SU_BFEE_ID = 4,
    CUSTOM_CFGID_PRIV_INI_MU_BFER_ID = 5,
    CUSTOM_CFGID_PRIV_INI_MU_BFEE_ID = 6,
    CUSTOM_CFGID_PRIV_INI_CALI_MASK_ID = 7,
    CUSTOM_CFGID_PRIV_CALI_DATA_MASK_ID = 8,
    CUSTOM_CFGID_PRIV_INI_AUTOCALI_MASK_ID = 9,
    CUSTOM_CFGID_PRIV_INI_LDPC_ID = 10,
    CUSTOM_CFGID_PRIV_INI_BA_32BITMAP_ID = 11,
    CUSTOM_CFGID_PRIV_INI_MTID_AGGR_RX_ID = 12,
    CUSTOM_CFGID_INI_BUTT
} custom_cfgid_h2d_ini_enum;

typedef struct {
    custom_cfgid_enum_uint32          syn_id;        /* 同步配置ID */
    osal_u32                          len;           /* DATA payload长度 */
    osal_u8                           msg_body[4];   /* DATA payload */
} hmac_to_dmac_cfg_custom_data_stru;

typedef struct {
    osal_u8 bus_h2d_sched_count;
    osal_u8 bus_d2h_sched_count;
} bus_cus_config_stru;

/* device侧平台低功耗定制化参数下发 */
typedef struct {
    osal_u32 device_plt_pm_enable : 1;
    osal_u32 device_awake_host_gpio_idx : 8;
    osal_u32 device_awake_host_gpio_level : 1;
    osal_u32 device_usb_link_check : 1;
    osal_u32 rsv : 21;
} device_plt_pm_cfg_stru;

/* 抛往dmac侧消息头的长度 */
#define CUSTOM_MSG_DATA_HDR_LEN      (sizeof(custom_cfgid_enum_uint32) + sizeof(osal_u32))
#endif /* _PRE_PLAT_FEATURE_CUSTOMIZE */

#define HAL_POW_CUSTOM_24G_11B_RATE_NUM            2    /* 定制化11b速率数目 */
#define HAL_POW_CUSTOM_11G_11A_RATE_NUM            5    /* 定制化11g/11a速率数目 */
#define HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM         5    /* 定制化HT20_VHT20速率数目 */
#define HAL_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM     6
#define HAL_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM      6
#define HAL_POW_CUSTOM_5G_VHT80_RATE_NUM           5
/* 定制化全部速率 */
#define HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM         2
#define HAL_POW_CUSTOM_MCS10_11_RATE_NUM           1

#define HAL_POW_CUSTOM_24G_HE20_RATE_NUM           3
#define HAL_POW_CUSTOM_24G_HE40_RATE_NUM           3
#define HAL_POW_CUSTOM_5G_HE20_RATE_NUM            3
#define HAL_POW_CUSTOM_5G_HE40_RATE_NUM            3
#define HAL_POW_CUSTOM_5G_HE80_RATE_NUM            3

#define HAL_POW_CUSTOM_HE_ER_SU_RATE_NUM           4

#define HAL_POW_CUSTOM_HT20_VHT20_DPD_RATE_NUM     5    /* 定制化DPD速率数目 */
#define HAL_POW_CUSTOM_HT40_VHT40_DPD_RATE_NUM     5

/* 定制化相关宏 */
/* NVRAM中存储的各协议速率最大发射功率参数的个数 From:24G_11b_1M To:5G_VHT80_MCS7 */
#define NUM_OF_NV_NORMAL_MAX_TXPOWER   (HAL_POW_CUSTOM_24G_11B_RATE_NUM + \
                                        HAL_POW_CUSTOM_11G_11A_RATE_NUM + HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM + \
                                        HAL_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM + HAL_POW_CUSTOM_11G_11A_RATE_NUM + \
                                        HAL_POW_CUSTOM_HT20_VHT20_RATE_NUM + HAL_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM + \
                                        HAL_POW_CUSTOM_5G_VHT80_RATE_NUM)

#define NUM_OF_NV_MAX_TXPOWER          (NUM_OF_NV_NORMAL_MAX_TXPOWER + HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM  + \
                                        HAL_POW_CUSTOM_MCS10_11_RATE_NUM + HAL_POW_CUSTOM_MCS9_10_11_RATE_NUM + \
                                        HAL_POW_CUSTOM_MCS10_11_RATE_NUM + HAL_POW_CUSTOM_MCS10_11_RATE_NUM + \
                                        HAL_POW_CUSTOM_24G_HE20_RATE_NUM * 5 + HAL_POW_CUSTOM_HE_ER_SU_RATE_NUM * 2)

#define NUM_OF_NV_DPD_MAX_TXPOWER  (HAL_POW_CUSTOM_HT20_VHT20_DPD_RATE_NUM + HAL_POW_CUSTOM_HT40_VHT40_DPD_RATE_NUM)

#endif