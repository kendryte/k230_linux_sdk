/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: cali_online_tx_pwr.c 的头文件
 * Date: 2022-10-20
 */

#ifndef __CALI_ONLINE_TX_PWR_H__
#define __CALI_ONLINE_TX_PWR_H__
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "fe_init_pow.h"
#include "hal_gp_reg.h"
#include "hal_power.h"
#include "fe_power_host.h"
#include "power_ppa_ctrl_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ONLINE_CALI_UPC_RE_ADJUST_TH0         (10)
#define ONLINE_CALI_UPC_RE_ADJUST_TH1         (10)

#define ONLINE_CALI_DBB_SCALING_MIN_STEP      (1)     /* 动态校准最小步进，对应最小阈值使用 */
#define ONLINE_CALI_DBB_SCALING_MAX_STEP      (5)     /* 动态校准最大步进，对应最大阈值使用 */
#define ONLINE_CALI_DBB_SCALING_MAX_TH        (50)    /* 动态校准DBB SCALING补偿范围最大阈值 */
#define ONLINE_CALI_DBB_SCALING_MIN_TH        (-50)   /* 动态校准DBB SCALING补偿范围最小阈值 */
#define ONLINE_CALI_DBB_SCALING_MAX_GAP       (5)     /* 动态校准DBB SCALING使用最大阈值 */
#define ONLINE_CALI_DBB_SCALING_INIT_VAL      (0x80)  /* DBB SCALING SHADOW默认值 */

/* 获取动态校准是否未使能 */
#define ONLINE_CALI_PER_FRAME_TIMER_PERIOD    (10000) /* 动态校准每帧定时器10s定时 */
#define ONLINE_CALI_INTVL_TIMER_PERIOD        (200)   /* 每帧最小间隔时间 */
#define ONLINE_CALI_PWR_MAX_TH                (15)    /* 动态校准最大阈值 */
#define ONLINE_CALI_PWR_MIN_TH                (3)     /* 动态校准最小阈值 */
#define ONLINE_CALI_PWR_DIFF_INVALID          (100)

#define ONLINE_CALI_ACCU_ABNORMAL_CNT         (5)
#define ONLINE_CALI_DSCR_INTERVAL             (0x1) /* 帧间隔 初值超过0x400溢出 */
#define ONLINE_CALI_COMPLETE_INTVL_FACTOR     (5)     /* 校准结束调整帧间隔 左移5位 放大2^5=32倍 */
/* 芯片无效动态功率 */
#define ONLINE_CALI_POW_INVALID               (250)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    ONLINE_CALI_PDET_ADJUST_INIT = 0,
    ONLINE_CALI_PDET_ADJUST_ASCEND,  /* while real_pdet < expect_pdet */
    ONLINE_CALI_PDET_ADJUST_DECLINE, /* while real_pdet > expect_pdet */
    ONLINE_CALI_PDET_ADJUST_VARIED,
    ONLINE_CALI_PDET_ADJUST_BUTT
} online_cali_adj_type_enum;

/* 调制方式枚举 */
typedef enum {
    ONLINE_CALI_MOD_DSSS,
    ONLINE_CALI_MOD_OFDM,

    ONLINE_CALI_MOD_BUTT
} online_cali_mod_enum;
typedef osal_u8 online_cali_mod_enum_uint8;
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
/* 寄存器读取pdet的数据结构 */
typedef union {
    struct {
        osal_s32 q : 13; /* [12..0] */
        osal_s32 i : 13; /* [25..13] */
        osal_s32 reserved : 6; /* [31..26] */
    } bits;
    osal_u32 u32;
} cali_online_pdet_stru;

typedef osal_void (*hal_flush_single_phy_txgain_reg)(const hal_device_stru *, osal_u16, osal_u16);
typedef struct {
    oal_bool_enum_uint8 upc_code_tobe_updated;    /* 是否需要更新当前通道的upc_code的lut表 */
    osal_s8 base_upc_idx;
    osal_s8 target_gain_num;
    osal_s8 rsv;
    osal_s16 upc_amend[WLAN_BAND_BUTT];
    cali_pow_crtl_table_stru *ppa_lut;
    /* online校准 */
    osal_s8 base_upc_idx_online;
    osal_s8 target_gain_num_online;
    osal_s16 rsv2;
    cali_pow_crtl_table_stru *upc_idx2code_online;
} online_cali_pow_ctrl_stru;

/* 会影响目标pdet值的参数集合 */
typedef union {
    struct {
        osal_s16 tx_pow;
        wlan_channel_band_enum_uint8 freq;
        wlan_bw_cap_enum_uint8 bw;
        osal_u16 channel;
        online_cali_mod_enum_uint8 mod;
        osal_u8 rsv[1];
    } rf_core_para;
    osal_u64 para;
} online_cali_record_union;
typedef struct {
    online_cali_record_union record_para;
    osal_s32 real_pow;
    osal_s32 exp_pow;
    osal_s32 pdet_cal; /* IQ两路计算得到的pdet */
    osal_s32 previous_pdet_val;

    osal_u8 cur_protocol;
    osal_u8 upc_idx;
    osal_u8 rate_idx;
    oal_bool_enum_uint8 cali_pdet_adjust_flag;
    osal_s16 delt_dbb_scale_dyn_val;
    osal_u16 rsv;
} online_cali_info_stru;
typedef struct {
    oal_bool_enum_uint8     dyn_pow_cali_comp_flag[CALI_STREAM_TYPE_BUTT][CALI_TXPWR_GAIN_LVL_NUM]; /* 校准收敛标志 */
    oal_bool_enum_uint8     ppa_lut_adjusted_flag[CALI_STREAM_TYPE_BUTT][CALI_TXPWR_GAIN_LVL_NUM]; /* ppa lut调整过 */
    osal_u8 rsv[2];
} online_cali_chain_status_stru;
typedef struct alg_dyn_cali_val {
    frw_timeout_stru dyn_cali_per_frame_timer;           /* 动态校准每帧的定时器 */
    frw_timeout_stru dyn_cali_intvl_timer;               /* 最小帧间隔200ms的定时器 */
    osal_u16 dyn_cali_en_cnt[WLAN_BAND_BUTT];
    osal_u16 dyn_cali_en_intvl[WLAN_BAND_BUTT];          /* 两次动态校准间隔描述符个数 */

    osal_s32 dyn_cali_val_min_th;                        /* 动态校准极小阈值  */
    osal_s32 dyn_cali_val_max_th;                        /* 动态校准极大阈值  */

    osal_u16 s_temp;                                     /* Tsensor检测温度 */
    osal_u8 tx_dyn_cali_chain_sel;                       /* 当前tx流程下的动态校准校准通道号 */
    oal_bool_enum_uint8 dyn_cali_adjst;                  /* 动态校准使能补偿开关 */

    online_cali_info_stru dyn_cali_pow_info[CALI_STREAM_TYPE_BUTT];
    online_cali_chain_status_stru dyn_pow_cali_status;

    osal_u32 curr_rx_comp_isr_count;                     /* 一轮间隔内，接收完成中断的产生个数 */
    osal_u32 rx_comp_isr_interval;                       /* 间隔多少个接收完成中断打印一次pdet信息 */

    osal_u8 dyn_cali_complete_chain;                     /* 当前动态校准调整rf通道号 */
    wlan_channel_band_enum_uint8 dyn_cali_complete_band; /* 当前动态校准频段 */
    oal_bool_enum_uint8 en_temp_change_flag;
    osal_s8 cali_channel_index;                          /* 校准频点,用于判断是否切信道 */

    osal_u8 en_dync_cali_done_ch;
    oal_bool_enum_uint8 pdet_debug_switch;               /* 打印芯片上报pdet值的日志开关 */
    oal_bool_enum_uint8 dyn_pow_cali_flag;
    oal_bool_enum_uint8 adjust_pow_method;
    oal_bool_enum_uint8 com_upc_switch;
    osal_s16 dc_offset[CALI_DC_OFFSET_NUM];
    osal_u8 resv[1];                                     /* 3 字节对齐 */
    oal_bool_enum_uint8 pow_accum_flag[WLAN_RF_CHANNEL_NUMS];
    oal_bool_enum_uint8 pow_accum_times[WLAN_RF_CHANNEL_NUMS];
    osal_u16 pow_too_low_times[WLAN_RF_CHANNEL_NUMS];     /* 记录功率异常低次数 */
    osal_u16 pow_too_high_times[WLAN_RF_CHANNEL_NUMS];    /* 记录功率异常高次数 */
} online_cali_dyn_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
online_cali_pow_ctrl_stru *online_cali_get_txpow_ctrl(osal_u8 chain);
osal_void online_cali_init_per_frame_timer(hal_to_dmac_device_stru *device);
osal_void online_cali_pow_cali_set_dscr_intvl(online_cali_dyn_stru *dyn_cali,
    wlan_channel_band_enum_uint8 band, osal_u16 val);
osal_u16 online_cali_get_cus_dyn_cali_intvl(wlan_channel_band_enum_uint8 band);
osal_void online_cali_dyn_pow_state_entry(osal_void *p_ctx);
osal_void online_cali_dyn_pow_state_exit(osal_void *p_ctx);
osal_u32 online_cali_dyn_pow_event(osal_void *p_ctx, osal_u16 us_event,
    osal_u16 us_event_data_len, osal_void *p_event_data);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __CALI_ONLINE_TX_PWR_H__