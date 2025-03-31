/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: channel config header for ABB 03 mpw RF.
 */

#ifndef __RF_CHANNEL_H__
#define __RF_CHANNEL_H__

#include "hal_common_ops_rom.h"
#include "cali_base_def.h"
#include "fe_rf_dev.h"

#define HH503_CALI_CHN_BAND_INFO_2422    1
#define HH503_CALI_CHN_BAND_INFO_2447    2
#define HH503_CALI_CHN_BAND_INFO_2472    3
// 校准三个频点的信道索引
#define CALI_2G_BAND_0_CHANNEL_INDEX 0
#define CALI_2G_BAND_1_CHANNEL_INDEX 6
#define CALI_2G_BAND_2_CHANNEL_INDEX 12
/* PLL配置结构 */
typedef struct {
    osal_u16 div_int;      /* 分频整数部分 */
    osal_u16 div_frac;     /* 分频小数部分  */
} rf_pll_div_cfg_stru;

#define HAL_PLL_LOCK_TIME           50

osal_void fe_hal_rf_update_band_info(hal_rf_dev *rf_dev, osal_u8 freq, osal_u8 band_info);
rf_pll_div_cfg_stru fe_hal_rf_calc_pll_div_param(osal_u16 pll_idx);
osal_void fe_hal_rf_cfg_pll_div(hal_rf_dev *rf_dev, osal_u16 div_int, osal_u16 div_frac);
osal_void fe_hal_rf_lock_pll(hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    osal_u8 wait_lock_time, osal_u8 locked_now);

#endif
