/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: auto-aggr algorithm host.
 */

#ifndef __ALG_AGGR_H__
#define __ALG_AGGR_H__

#ifdef _PRE_WLAN_FEATURE_AUTOAGGR

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "frw_ext_if.h"
#include "hmac_alg_if.h"
#include "alg_transplant.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void alg_aggr_hmac_init(osal_void);
osal_void alg_aggr_hmac_exit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_AUTOAGGR */
#endif /* end of alg_tpc.h */
