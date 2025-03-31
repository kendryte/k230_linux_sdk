/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: HAL Layer Specifications.
 * Create: 2022-2-17
 */

#ifndef __HAL_COMMON_OPS_DEVICE_H__
#define __HAL_COMMON_OPS_DEVICE_H__

/*****************************************************************************
    1 头文件包含
*****************************************************************************/
#include "hal_common_ops_device_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HAL_POW_CUSTOM_24G_HE40_RATE_NUM           3
#define HAL_POW_CUSTOM_5G_HE20_RATE_NUM            3
#define HAL_POW_CUSTOM_5G_HE40_RATE_NUM            3
#define HAL_POW_CUSTOM_5G_HE80_RATE_NUM            3

#define NUM_OF_NV_TOTAL_MAX_TXPOWER     124

#define HAL_CUS_NUM_5G_BW                        4   /* 定制化5g带宽数 */

#define HAL_NUM_5G_160M_SIDE_BAND                2

#define HAL_POW_MAX_CHAIN_NUM           2           /* 最大通道数 */

#define HAL_2G_POW_UPC_RF_LUT_NUM       256        /* UPC在RF中的档位数目 8bit */
#define HAL_5G_POW_UPC_RF_LUT_NUM       64

#define HAL_POW_11B_LPF_BASE_IDX            1           /* 11B基准LPF Index */

#define HAL_POW_5G_6P5MBPS_RATE_POW_IDX     12              /* 5G 6.5Mbps对应的功率表索引, ht20 mcs0 */


/*****************************************************************************
  3 枚举
*****************************************************************************/

/* 2.4GHz频段: 信道号对应的信道索引值 */
typedef enum {
    HAL_2G_CHANNEL1  = 0,
    HAL_2G_CHANNEL2  = 1,
    HAL_2G_CHANNEL3  = 2,
    HAL_2G_CHANNEL4  = 3,
    HAL_2G_CHANNEL5  = 4,
    HAL_2G_CHANNEL6  = 5,
    HAL_2G_CHANNEL7  = 6,
    HAL_2G_CHANNEL8  = 7,
    HAL_2G_CHANNEL9  = 8,
    HAL_2G_CHANNEL10 = 9,
    HAL_2G_CHANNEL11 = 10,
    HAL_2G_CHANNEL12 = 11,
    HAL_2G_CHANNEL13 = 12,
    HAL_2G_CHANNEL14 = 13,

    HAL_CHANNEL_FREQ_2G_BUTT = 14
} hal_channel_freq_2g_enum;
typedef osal_u8 hal_channel_freq_2g_enum_uint8;

typedef enum {
    HAL_OPER_MODE_NORMAL,
    HAL_OPER_MODE_HUT,

    HAL_OPER_MODE_BUTT
} hal_oper_mode_enum;
typedef osal_u8 hal_oper_mode_enum_uint8;

/* RF测试用，用于指示配置TX描述符字段 */
typedef enum {
    HAL_RF_TEST_DATA_RATE_ZERO,
    HAL_RF_TEST_BAND_WIDTH,
    HAL_RF_TEST_CHAN_CODE,
    HAL_RF_TEST_POWER,
    HAL_RF_TEST_BUTT
} hal_rf_test_sect_enum;
typedef osal_u8 hal_rf_test_sect_enum_uint8;

/* 0~3代表AC发送队列，4代表管理帧、控制帧发送队列 */
#define HAL_TX_QUEUE_MGMT               HAL_TX_QUEUE_HI

/* HAL模块需要抛出的WLAN_CRX事件子类型的定义
   说明:该枚举需要和dmac_wlan_crx_event_sub_type_enum_uint8枚举一一对应 */
typedef enum {
    HAL_WLAN_CRX_EVENT_SUB_TYPE_RX,    /* WLAN CRX 流程 */

#ifdef _PRE_WLAN_FEATURE_FTM
    HAL_EVENT_DMAC_MISC_FTM_ACK_COMPLETE,   /* FTM ACK发送完成中断 */
#endif
    HAL_WLAN_CRX_EVENT_SUB_RPT_HE_ROM,
    HAL_WLAN_CRX_EVENT_SUB_RPT_TRIG_PARA,   /* 收到Trig帧中断 */
    HAL_WLAN_CRX_EVENT_SUB_TYPE_BUTT
} hal_wlan_crx_event_sub_type_enum;
typedef osal_u8 hal_wlan_crx_event_sub_type_enum_uint8;

typedef enum {
    HAL_TX_COMP_SUB_TYPE_TX,
    HAL_TX_COMP_SUB_TYPE_AL_TX,
    HAL_TX_COMP_SUB_TYPE_BUTT
} hal_tx_comp_sub_type_enum;
typedef osal_u8 hal_tx_comp_sub_type_enum_uint8;

/* 功率模式 */
typedef enum {
    HAL_POW_MODE_MARGIN        = 0,    /* 有余量模式: 默认 */
    HAL_POW_MODE_NO_MARGIN     = 1,    /* 没有余量模式 */

    HAL_POW_MODE_BUTT
} hal_pow_mode_enum;
typedef osal_u8 hal_pow_mode_enum_uint8;

typedef enum {
    HAL_LPM_SOC_BUS_GATING       = 0,
    HAL_LPM_SOC_PCIE_RD_BYPASS   = 1,
    HAL_LPM_SOC_MEM_PRECHARGE    = 2,
    HAL_LPM_SOC_PCIE_L0          = 3,
    HAL_LPM_SOC_PCIE_L1_PM       = 4,
    HAL_LPM_SOC_AUTOCG_ALL       = 5,
    HAL_LPM_SOC_ADC_FREQ         = 6,
    HAL_LPM_SOC_PCIE_L1S         = 7,

    HAL_LPM_SOC_SET_BUTT
} hal_lpm_soc_set_enum;
typedef osal_u8   hal_lpm_soc_set_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_SMPS
/*  SMPS模式配置
    00：SMPS_STATIC（始终单路接收）
    01：SMPS_DYNAMIC
    10：reserved
    11：SMPS_DISABLE（始终多路接收） */
typedef enum {
    HAL_SMPS_MODE_STATIC = 0,
    HAL_SMPS_MODE_DYNAMIC = 1,
    HAL_SMPS_MODE_DISABLE = 3,

    HAL_SMPS_MODE_BUTT
} hal_smps_mode_enum;

typedef osal_u8 hal_smps_mode_enum_uint8;
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
typedef enum {
    HAL_RADAR_NOT_REPORT = 0,
    HAL_RADAR_REPORT,
} hal_radar_filter_enum;
typedef osal_u8 hal_radar_filter_enum_uint8;
#endif

/*****************************************************************************
  STRUCT定义
*****************************************************************************/

typedef struct {
    /* PHY TX MODE 2 (第15行) */
    osal_u8                       tx_rts_antenna;          /* 发送RTS使用的天线组合 */
    osal_u8                       rx_ctrl_antenna;         /* 接收CTS/ACK/BA使用的天线组合 */
    osal_u8                       reserve1[1];             /* TX VAP index 不是算法填写，故在此也填0 */

    /* 0代表允许TXOP POWER save，1代表不允许TXOP POWER save */
    osal_u8                       txop_ps_not_allowed : 1;

    /* NAV保护enable字段，1代表Long nav保护，0代表non long nav保护 */
    osal_u8                       long_nav_enable     : 1;

    /* 这个字段暂时由软件填写，最终可能由算法填写，故先列出 */
    osal_u8                       group_id            : 6;
} hal_tx_txop_antenna_params_stru;

/* dmac_pkt_captures使用,tx rx均会使用 */
typedef struct {
    union {
        struct {
            osal_u8  he_mcs        : 4;
            osal_u8  nss_mode      : 2;                    /* wlan_nss_enum_uint8 */
            osal_u8  protocol_mode : 2;
        } he_nss_mcs;
        struct {
            osal_u8   vht_mcs       : 4;
            osal_u8   nss_mode      : 2;
            osal_u8   protocol_mode : 2;
        } vht_nss_mcs;                                   /* 11ac的速率集定义 */
        struct {
            osal_u8   ht_mcs        : 6;
            osal_u8   protocol_mode : 2;
        } ht_rate;                                       /* 11n的速率集定义 */
        struct {
            osal_u8   legacy_rate   : 4;
            osal_u8   reserved1     : 2;
            osal_u8   protocol_mode : 2;
        } legacy_rate;                                   /* 11a/b/g的速率集定义 */
    } nss_rate;

    osal_u8           short_gi;
    osal_u8           bandwidth;

    osal_u8           preamble       : 1,
                    channel_code  : 1,
                    stbc          : 2,
                    reserved2     : 4;
} hal_statistic_stru;

#define RADAR_INFO_FLAG_DUMMY   0x10

/* 脉冲信息结构体 */
typedef struct {
    osal_u32  timestamp;
    osal_u16  duration;
    osal_u16  power;
    osal_u16  max_fft;
    osal_u8   type;
    osal_u8   auc_resv[1];
} hal_pulse_info_stru;

#define MAX_RADAR_PULSE_NUM   32            /* 最大雷达脉冲数 */
#define RADAR_TYPE_CHIRP      10            /* 芯片上报的chirp雷达类型标号 */
#define RADAR_MIN_ETSI_CHIRP_PULSE_NUM  5   /* ETSI chirp最小脉冲个数 */

/* 保存多个脉冲信息结构体 */
typedef struct {
    hal_pulse_info_stru     pulse_info[MAX_RADAR_PULSE_NUM];
    osal_u32              pulse_cnt;
} hal_radar_pulse_info_stru;

/* 对DMAC SCAN模块提供的硬件雷达检测信息结构体 */
typedef struct {
#ifdef _PRE_WLAN_FEATURE_DFS
    hal_radar_pulse_info_stru radar_pulse_info;
#endif
    osal_u8   radar_type;
    osal_u8   radar_freq_offset;
    osal_u8   radar_bw;
    osal_u8   band;
    osal_u8   channel_num;
    osal_u8   working_bw;
    osal_u8   flag;
    osal_u8   resv;
} hal_radar_det_event_stru;

typedef struct {
    osal_u32      reg_band_info;
    osal_u32      reg_bw_info;
    osal_u32      reg_ch_info;
} hal_radar_irq_reg_list_stru;

typedef enum {
    HAL_WOW_PARA_EN                    = BIT0,
    HAL_WOW_PARA_NULLDATA              = BIT1,
    HAL_WOW_PARA_NULLDATA_INTERVAL     = BIT2,
    HAL_WOW_PARA_NULLDATA_AWAKE        = BIT3,
    HAL_WOW_PARA_AP0_PROBE_RESP        = BIT4,
    HAL_WOW_PARA_AP1_PROBE_RESP        = BIT5,
    HAL_WOW_PARA_AP0_IS_FAKE_VAP       = BIT6,
    HAL_WOW_PARA_AP1_IS_FAKE_VAP       = BIT7,

    HAL_WOW_PARA_BUTT
} hal_wow_para_set_enum;
typedef osal_u32 hal_tx_status_enum_uint32;


#ifdef _PRE_WLAN_FEATURE_DFS
typedef struct {
    /* 误报新增过滤条件 */
    osal_u8 fcc_chirp_duration_diff   : 1;
    osal_u8 fcc_chirp_pow_diff        : 1;
    osal_u8 fcc_type4_duration_diff   : 1;
    osal_u8 fcc_chirp_eq_duration_num : 1;
    osal_u8 resv                      : 4;
} hal_dfs_pulse_check_filter_stru;

typedef struct {
    /* normal pulse det */
    osal_u8 irq_cnt;
    osal_u8 radar_cnt;
    osal_u8 irq_cnt_old;
    osal_u8 timeout     : 1;
    osal_u8 timer_start : 1;
    osal_u8 is_enabled  : 1;
    osal_u8 resv        : 5;

    osal_u32           period_cnt;
    frw_timeout_stru timer;
} hal_dfs_normal_pulse_det_stru;

typedef struct {
#ifdef _PRE_WLAN_FEATURE_DFS
    osal_u8 chirp_enable        : 1;
    osal_u8 chirp_wow_wake_flag : 1; /* 在wow的gpio中断上半部标记，表示刚从wow唤醒 */
    osal_u8 log_switch          : 2;
    osal_u8 radar_type          : 3;
    osal_u8 chirp_timeout       : 1;

    osal_u8 chirp_cnt;
    osal_u8 chirp_cnt_total;
    osal_u8 chirp_cnt_for_crazy_report_det;

    osal_u32 last_timestamp_for_chirp_pulse;

    frw_timeout_stru timer;
    osal_u32 min_pri;

    /* 误报过滤条件 */
    hal_dfs_pulse_check_filter_stru dfs_pulse_check_filter;

    /* crazy report det */
    osal_u8 crazy_report_cnt        : 1;
    osal_u8 crazy_report_is_enabled : 1;
    osal_u8 resv0                   : 6;
    osal_u8 resv1[2]; /* 2byte保留字段 */

    frw_timeout_stru timer_crazy_report_det;
    frw_timeout_stru timer_disable_chirp_det;

    /* normal pulse det timer */
    hal_dfs_normal_pulse_det_stru dfs_normal_pulse_det;

#else
    osal_u8 chirp_enable        : 1;
    osal_u8 chirp_wow_wake_flag : 1; /* 在wow的gpio中断上半部标记，表示刚从wow唤醒 */
    osal_u8 resv                : 6;
    osal_u8 resv0[3]; /* 3byte保留字段 */
#endif
    osal_u32           chirp_time_threshold;
    osal_u32           chirp_cnt_threshold;
    osal_u32           time_threshold;
    osal_u32           last_burst_timestamp;
    osal_u32           last_burst_timestamp_for_chirp;
} hal_dfs_radar_filter_stru;
#endif

typedef struct {
    osal_u32                rx_rate;
    osal_u16                rx_rate_stat_count;
    oal_bool_enum_uint8     compatibility_enable;
    oal_bool_enum_uint8     compatibility_stat;
    osal_u32                compatibility_rate_limit[WLAN_BW_CAP_BUTT][WLAN_PROTOCOL_BUTT];
} hal_compatibility_stat_stru;

typedef struct {
    osal_u32 *cnt1;
    osal_u32 *cnt2;
    osal_u32 *cnt3;
    osal_u32 *cnt4;
    osal_u32 *cnt5;
    osal_u32 *cnt6;
} machw_rx_err_count_stru;

typedef struct {
    osal_u8   cali_gear;
    osal_u8   cali_data;
    osal_s8   cali_offset;
    osal_u8   ant;
    osal_u8   pga_unit_deta_one;
    osal_u8   pga_unit_deta_two;
} hal_pow_tpc_cali_stru;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
