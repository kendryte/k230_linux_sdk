/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Header file for hal_ext_txbf.c
 */

#ifndef __HAL_EXT_TXBF_H__
#define __HAL_EXT_TXBF_H__
/*****************************************************************************
    1 头文件包含
*****************************************************************************/
#include "osal_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
    2 函数声明
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TXBF
osal_void hal_set_bf_rate(osal_u8 rate);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_ext_txbf.h */