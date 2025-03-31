/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准信道相关设置
 * Create: 2022-09-14
 */

#ifndef __CALI_CHANNEL_H__
#define __CALI_CHANNEL_H__

#include "cali_outer_os_if.h"
#include "cali_outer_hal_if.h"
#include "cali_spec.h"
#include "cali_base_def.h"
#include "channel.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// 校准信道配置
typedef struct {
    osal_u8 chn_num;                    // 校准信道数
    osal_u8 start_chn_idx;
    osal_u8 end_chn_idx;
    rf_pll_div_cfg_stru *pll_cfg;  // 每个信道对应的pll配置（跟rf强相关）
    osal_u8 *band_info;                  // 每个信道对应的band_sel值（跟rf强相关）
} cali_channel_stru;
cali_channel_stru *cali_get_rf_channel_cfg(wlan_channel_band_enum_uint8 band, wlan_rf_cali_type_enum_uint8 cali_type);
osal_void cali_cfg_rf_channel(hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 freq,
    cali_channel_stru *cali_channel, osal_u8 channel_idx, cali_bandwidth_enum_uint8 rf_cali_bandwidth);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
