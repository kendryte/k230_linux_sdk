/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: header for cali rate pow lut
 * Create: 2022-10-14
 */
#ifndef __FE_UPDATE_POW_H__
#define __FE_UPDATE_POW_H__
#include "cali_outer_hal_if.h"
#include "power_ppa_ctrl_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// 根据校准数据中的ppa_code刷新某一tpc_code的ppa_code
osal_u16 cali_get_adjusted_ppa_code(const hal_device_stru *device, osal_u8 cali_subband, osal_u8 tpc_code);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif