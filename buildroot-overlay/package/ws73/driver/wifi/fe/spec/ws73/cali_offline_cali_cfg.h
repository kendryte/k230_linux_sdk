/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 离线校准相关配置
 * Create: 2022-10-19
 */

#ifndef __CALI_OFFLINE_CALI_CFG_H__
#define __CALI_OFFLINE_CALI_CFG_H__

#include "cali_spec.h"
#include "fe_rf_dev.h"
#include "cali_base_def.h"
#include "cali_outer_hal_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define CALI_RC_CODE_TT (60)
#define CALI_RC_VAR_NUM (32)
// RX DC校准规格
#define CALI_RX_DC_CODE_STEP (4)    // 每个code的调整步长 每次4mv
#define CALI_RX_DC_SPEC (10)        // 校准后的DC残留值规格 10mv

/* 校准函数指针定义 */
typedef osal_void (*cali_func_ptr)(const hal_device_stru *, wlan_channel_band_enum_uint8,
    cali_bandwidth_enum_uint8, osal_u8, osal_u8);
typedef osal_void (*cali_comp_func_ptr)(const hal_rf_dev *, wlan_channel_band_enum_uint8, osal_u8,
    cali_bandwidth_enum_uint8);
typedef osal_void (*band_cfg_func_ptr)(const hal_device_stru *, wlan_channel_band_enum_uint8);
typedef osal_void (*bw_cfg_func_ptr)(const hal_device_stru *, wlan_channel_band_enum_uint8,
    cali_bandwidth_enum_uint8);
typedef osal_void (*path_cfg_func_ptr)(const hal_device_stru *, wlan_channel_band_enum_uint8,
    cali_bandwidth_enum_uint8, osal_u8);
typedef osal_void (*path_recover_func_ptr)(const hal_device_stru *, wlan_channel_band_enum_uint8,
    cali_bandwidth_enum_uint8, osal_u8);

typedef struct {
    cali_func_ptr cali_func;
    band_cfg_func_ptr band_cfg_func;
    bw_cfg_func_ptr bw_cfg_func;
    path_cfg_func_ptr path_cfg_func;
    path_recover_func_ptr path_recover_func;
    osal_u8 start_bw;
    osal_u8 end_bw;
    osal_u16 rsv;
} cali_func_cfg_stru;

// 读取校准相关函数配置
const cali_func_cfg_stru *cali_get_offline_cali_func(wlan_channel_band_enum_uint8 band,
    wlan_rf_cali_type_enum_uint8 cali_type);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
