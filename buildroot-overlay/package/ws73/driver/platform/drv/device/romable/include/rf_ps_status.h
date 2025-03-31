/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 射频的节能状态管理
 */

#ifndef __RF_PS_STATUS_H__
#define __RF_PS_STATUS_H__

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    RF_PS_STATUS_SLEEP  = 0,
    RF_PS_STATUS_WAKE,
} rf_ps_status_enum;
typedef osal_u8 rf_ps_status_uint8;

// 获取bgle的rf低功耗状态
osal_u8 rf_get_bgle_ps_status(osal_void);

// 获取wifi的rf低功耗状态
osal_u8 rf_get_wifi_ps_status(osal_void);

// 设置bgle的rf低功耗状态
osal_void rf_set_bgle_ps_status(rf_ps_status_uint8 status);

// 设置wifi的rf低功耗状态
osal_void rf_set_wifi_ps_status(rf_ps_status_uint8 status);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
