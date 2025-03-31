/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_chip.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_CHIP_H__
#define __HAL_CHIP_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_device.h"
#include "hal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  宏定义
*****************************************************************************/
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_CHIP_H


/*****************************************************************************
  STRUCT定义
*****************************************************************************/
/* 定义CHIP下专用寄存器 */
typedef struct tag_hal_hal_chip_stru {
    hal_to_dmac_chip_stru hal_chip_base;
    hal_device_stru device;
    hal_cfg_rf_custom_cap_info_stru rf_custom_mgr;
} hal_chip_stru;

/*****************************************************************************
   函数声明
*****************************************************************************/
osal_u32 hal_chip_init(osal_void);
osal_void hal_chip_exit(osal_void);
osal_void hal_device_hw_init(hal_to_dmac_device_stru *hal_device_base);
#ifdef _PRE_WLAN_FEATURE_M2S
osal_void hal_chip_init_rf_custom_cap(hal_chip_stru *hal_chip);
#endif
hal_chip_stru *hal_get_chip_stru(osal_void);

osal_u32 hal_device_init_resv(hal_to_dmac_device_stru *hal_device_base);
osal_void hal_device_init_ext(hal_chip_stru *hal_chip);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_chip.h */
