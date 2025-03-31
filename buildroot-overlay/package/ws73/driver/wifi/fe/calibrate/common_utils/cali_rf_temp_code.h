/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 温度码接口头文件
 */
#ifndef __CALI_RF_TEMP_CODE_H__
#define __CALI_RF_TEMP_CODE_H__
#include "hal_common_ops.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
osal_void rf_temp_code_config_comp_value(hal_to_dmac_device_stru *device,
    wlan_channel_band_enum_uint8 band, osal_bool is_init);
osal_void rf_temp_code_create_config_timer(hal_to_dmac_device_stru *device);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __CALI_RF_TEMP_CODE_H__
