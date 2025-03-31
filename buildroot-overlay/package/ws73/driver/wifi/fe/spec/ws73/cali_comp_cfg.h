/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准补偿相关配置
 * Create: 2022-10-19
 */

#ifndef __CALI_COMP_CFG_H__
#define __CALI_COMP_CFG_H__

#include "cali_outer_os_if.h"
#include "cali_outer_hal_if.h"
#include "cali_spec.h"
#include "cali_base_def.h"
#include "fe_rf_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 校准补偿函数指针定义 */
typedef osal_void (*cali_comp_func_ptr)(const hal_rf_dev *, wlan_channel_band_enum_uint8, osal_u8,
    cali_bandwidth_enum_uint8);

cali_comp_func_ptr cali_get_cali_comp_func(wlan_channel_band_enum_uint8 band, wlan_rf_cali_type_enum_uint8 cali_type);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
