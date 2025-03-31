/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: header file of tx beamforming algorithm
 */

#ifndef __ALG_TXBF_H__
#define __ALG_TXBF_H__

/*****************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "hmac_tx_mgmt.h"
#include "alg_transplant.h"
#include "alg_autorate.h"
#include "wlan_spec.h"
#include "alg_intf_det.h"
#include "hmac_alg_notify.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF

/*****************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_TXBF_NEAR_RSSI        (-40)
#define ALG_TXBF_FAR_RSSI         (-60)
#define ALG_TXBF_RSSI_GAP         (3)
/*****************************************************************************
  3 枚举定义
******************************************************************************/

/*****************************************************************************
  7 STRUCT定义
******************************************************************************/

/*****************************************************************************
  10 函数声明
******************************************************************************/
osal_s32 alg_hmac_txbf_init(osal_void);
osal_void alg_hmac_txbf_exit(osal_void);

#endif /* #ifdef _PRE_WLAN_FEATURE_TXBF */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of alg_txbf.h */
