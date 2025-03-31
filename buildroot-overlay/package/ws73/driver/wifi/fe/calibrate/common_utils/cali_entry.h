/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准入口/上层处理
 * Create: 2022-09-13
 */

#ifndef __CALI_ENTRY_H__
#define __CALI_ENTRY_H__

#include "cali_outer_os_if.h"
#include "cali_outer_hal_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void cali_offline_cali_one_rf_channel(hal_device_stru *device);
osal_void cali_offline_cali_entry(osal_void);

osal_void cali_offline_cali_prepare(hal_device_stru *device);
osal_void cali_offline_cali_finish(hal_device_stru *device);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
