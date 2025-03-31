/*
 * Copyright (c) CompanyNameMagicTag 2021. All rights reserved.
 * Description: Header file for hal_psd.c.
 * Create: 2021-11-29
 */

#ifndef __HAL_PSD_H__
#define __HAL_PSD_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_PSD
osal_void hal_psd_enable_etc(osal_void);
osal_void hal_psd_disable_etc(osal_void);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_psd.h */
