/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_device.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_DEVICE_H__
#define __HAL_DEVICE_H__

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "hal_common_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    宏定义
*****************************************************************************/
#define HAL_DEVICE_2G_BAND_NUM_FOR_LOSS (3) /* 2g 插损的band个数，用于插损相关的计算 */
#define HAL_DEVICE_5G_BAND_NUM_FOR_LOSS WLAN_5G_SUB_BAND_NUM
#define HAL_DEVICE_2G_DELTA_RSSI_NUM (2) /* 20M/40M */
#define HAL_DEVICE_5G_DELTA_RSSI_NUM (4) /* 20M/40M/80M/160M */
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
#define HAL_2G_CHANNEL_NUM (15)
#else
#define HAL_2G_CHANNEL_NUM (13)
#endif

#define WLAN_HAL_OHTER_BSS_ID                   6   /* 其他BSS的广播ID */
#define WLAN_HAL_OTHER_BSS_OTHER_ID             7   /* 来自其他BSS的其他帧(组播、单播) */
/* MAC上报的tbtt中断类别最大值，2个ap的tbtt中断(0-1)+3个sta的tbtt中断(4-6) */
#define WLAN_MAC_REPORT_TBTT_IRQ_MAX            7
/*****************************************************************************
    结构体定义
*****************************************************************************/
/* P2P 中断统计 */
typedef struct {
    osal_u32 ct_window_end_count;
    osal_u32 noa_absent_start_count;
    osal_u32 noa_absent_end_count;
    osal_u32 absent_start_count;
    osal_u32 absent_end_count;
} hal_p2p_ps_stat_stru;

/* 设备级 中断统计信息，!!!!本结构中修改任何一个成员都需要通知SDT同步修改，否则解析错误!!!! */
typedef struct {
    osal_u32 radar_detect_cnt; /* 检测到雷达 */
    /* P2P 节能统计 */
    hal_p2p_ps_stat_stru p2p_stat;
} hal_irq_device_stats_stru;

typedef struct tag_hal_hal_device_stru {
    hal_to_dmac_device_stru hal_device_base;
    /* device上挂的指针，使用时需要强转(后续需要使用宏替代，是不是指链表模板) */
    hal_vap_stru *vap_list[HAL_MAX_VAP_NUM];
    hal_irq_device_stats_stru irq_stats;

    osal_u8 cur_cali_chn; /* 当前校准的通道 */
    osal_u8 cali_data_received  : 1, /* device是否收到host下发的校准数据 */
          dbdc_cali           : 1,
          noa_type            : 2, /* 表示软件模拟触发一次雷达中断 */
          wur_ac : 4; /* 标识WUR帧下发的ac队列 */
    osal_u8 vap_num; /* device下的vap的数量 */
    osal_u8 wur_ap_vap_id : 4, /* wur在ap模式下使用，指示当前ap的vap id */
            wur_twbtt_vap_id : 4; /* 指示wur的twbtt功能借用了哪个vap的TSF能力 */
} hal_device_stru;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* customize rf cfg struct */
typedef struct {
    osal_s8 rf_gain_db_mult4; /* 插损值(单位0.25dB) */
    osal_u8 resv[3];            /* 3 byte保留字段 */
} hal_cfg_custom_gain_db_per_band_stru;
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

/*****************************************************************************
    函数声明
*****************************************************************************/
osal_u8 get_bw_idx(osal_u8 bw);

osal_u8 hal_device_calc_up_vap_num(const hal_to_dmac_device_stru *hal_device);
oal_bool_enum_uint8 hal_device_find_is_ap_up(hal_to_dmac_device_stru *hal_device);
osal_void hal_device_state_idle_set_rx_filter_reg(const hal_to_dmac_device_stru *hal_device);
osal_void hal_device_state_work_set_rx_filter_reg(hal_to_dmac_device_stru *hal_device);
osal_void hal_device_state_scan_set_rx_filter_reg(const hal_to_dmac_device_stru *hal_device);

osal_u32 hal_device_update_phy_chain(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 set_channel);
osal_void hal_device_reset_bcn_rf_chain(hal_to_dmac_device_stru *hal_device, osal_u8 up_vap_num);
osal_u8 hal_device_get_m2s_mask(osal_void);

osal_void hal_device_sync_vap(hal_to_dmac_vap_stru *hal_vap);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_void hal_set_nvram_params_by_delt_pwr(hal_to_dmac_device_stru *hal_device, const osal_u8 *param);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_device.h */
