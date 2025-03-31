/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准path相关
 * Create: 2022-09-20
 */

#ifndef __HAL_CALI_PATH_H__
#define __HAL_CALI_PATH_H__

#include "cali_outer_os_if.h"
#include "cali_outer_hal_if.h"
#include "cali_base_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
osal_void cali_set_up_logen_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_recover_logen_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
// rx dc的射频通路建立
osal_void cali_set_up_rx_dc_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
// rx dc的射频通路恢复
osal_void cali_recover_rx_dc_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
// tx dc的射频通路建立
osal_void cali_set_up_tx_dc_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
// tx dc的射频通路恢复
osal_void cali_recover_tx_dc_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_set_up_txpwr_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_recover_txpwr_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_set_up_rc_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_recover_rc_rf_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
// IQ 校准通路
osal_void cali_set_up_tx_iq_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_set_up_rx_iq_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_recover_tx_iq_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
osal_void cali_recover_rx_iq_path(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
