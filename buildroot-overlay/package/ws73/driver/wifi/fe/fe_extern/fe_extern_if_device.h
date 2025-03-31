/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 前端对外提供接口---- 只能包含类型定义，不允许包含实际的函数声明,同样也不能包含校准内部头文件。
 * Create: 2022-10-20
 */

#ifndef __FE_EXTERN_IF_DEVICE_H__
#define __FE_EXTERN_IF_DEVICE_H__
#include "hal_common_ops_rom.h"
#include "hal_commom_ops_type_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HAL_2G_CHANNEL_NUM (13)

typedef enum {
    CCA_ED_TYPE_20TH = 0,
    CCA_ED_TYPE_40TH,
} cca_ed_type_enum;

/* rf dev 状态机状态枚举 */
typedef enum {
    HAL_RF_STATE_IDLE = 0,         /* idle状态 */
    HAL_RF_STATE_CALI = 1,         /* cali状态 */
    HAL_RF_STATE_WORK = 2,         /* work状态 */
    HAL_RF_STATE_PS   = 3,         /* powersave状态 */
    HAL_RF_STATE_BUTT              /* 最大状态 */
} hal_rf_state_info;

/* 信道参数结构体 */
typedef struct {
    wlan_channel_band_enum_uint8       band;          /* 频段(2g,5g) */
    wlan_channel_bandwidth_enum_uint8  bw;            /* 带宽(20M,40M,80M) */
    osal_u8                            ch_num;        /* 信道号 */
    osal_u8                            ch_idx;        /* 信道索引 */
} hal_rf_chn_param;

typedef osal_char (*fe_rf_get_customized_cca_ed_cb)(osal_u32 cca_ed_type, wlan_channel_band_enum_uint8 band);
typedef osal_void (*online_cali_proc_set_channel_cb)(hal_to_dmac_device_stru *device, osal_u8 channel_num,
    wlan_channel_band_enum_uint8 band, osal_u8 channel_idx, wlan_channel_bandwidth_enum_uint8 bandwidth);
typedef osal_void (*online_cali_proc_tx_send_cb)(hal_to_dmac_device_stru *hal_device,
    hal_tx_txop_alg_stru *txop_alg, wlan_frame_type_enum_uint8 frame_type);
typedef osal_void (*online_cali_get_pdet_val_cb)(hal_tx_complete_event_stru *tx_comp_event);
typedef osal_void (*online_cali_proc_tx_complete_cb)(osal_u8 vap_id, hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *curr_dscr, hal_tx_complete_event_stru *tx_comp_event);

// rom化代码调用hook类型定义
typedef osal_void (*fe_rf_disable_abb_cb)(osal_u8 rf_id);
typedef osal_void (*fe_hal_rf_set_channel_info_cb)(hal_rf_chan_info *rf_chan);
typedef osal_void (*fe_rf_dev_set_ops_ext_cb)(osal_u8 cfg);
typedef osal_u32 (*fe_rf_dev_init_cb)(osal_u8 rf_id);
typedef osal_void (*fe_hal_proc_rf_work_cb)(osal_u8 rf_id);
typedef osal_void (*fe_hal_proc_rf_sleep_cb)(osal_u8 rf_id);
typedef osal_void (*fe_hal_rf_open_close_tx_cb)(osal_u8 rf_id, osal_u8 flag);
typedef osal_void (*fe_hal_rf_open_close_pll_cb)(osal_u8 rf_id, osal_u8 pll_idx, osal_u8 flag);
typedef osal_void (*fe_hal_rf_open_close_ldo_cb)(osal_u8 rf_id, osal_u8 flag);
typedef osal_u8 (*fe_rf_fsm_get_state_cb)(osal_u8 rf_id);
typedef osal_void (*fe_rf_set_rf_channel_cb)(hal_rf_chn_param *chn_para);
typedef osal_void (*fe_rf_dev_sleep_cb)(osal_u8 rf_id);
typedef osal_void (*fe_rf_proc_rf_awake_cb)(osal_void);
typedef osal_void (*fe_rf_proc_rf_sleep_cb)(osal_void);
typedef osal_void (*fe_rf_dev_enable_tx_cb)(osal_u8 rf_id);
typedef osal_void (*fe_rf_enable_rf_tx_cb)(osal_void);
typedef osal_void (*fe_rf_disable_rf_tx_cb)(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif