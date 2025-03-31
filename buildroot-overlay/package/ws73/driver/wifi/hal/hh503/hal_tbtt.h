/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: Header file for hal_tbtt.c.
 * Create: 2023-1-17
 */

#ifndef HAL_TBTT_H
#define HAL_TBTT_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_common_ops.h"
#include "hal_ext_if.h"
#include "hh503_phy_reg.h"
#include "mac_vap_ext.h"
#include "mac_resource_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
   宏定义
*****************************************************************************/
#define HH503_MAC_CFG_TSF_PULSE_SEL_VAP0 0
#define HH503_MAC_CFG_TSF_PULSE_SEL_VAP1 1
#define HH503_MAC_CFG_TSF_PULSE_SEL_VAP2 2

/*****************************************************************************
  STRUCT定义
*****************************************************************************/

/*****************************************************************************
  函数声明
*****************************************************************************/
osal_void hal_tsf_pulse_adjust(osal_void);
osal_void hh503_vap_ap_start_tsf(const hal_to_dmac_vap_stru *hal_vap);
osal_void hh503_enable_sta_tsf_tbtt(const hal_to_dmac_vap_stru *hal_vap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif