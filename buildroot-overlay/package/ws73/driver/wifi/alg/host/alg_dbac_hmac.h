/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm dbac
 */

#ifndef __ALG_DBAC_HMAC_H__
#define __ALG_DBAC_HMAC_H__

#ifdef _PRE_WLAN_FEATURE_DBAC

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

osal_void alg_dbac_hmac_init(osal_void);
osal_void alg_dbac_hmac_exit(osal_void);
osal_bool alg_dbac_hmac_is_vap_stop(osal_u8 hal_vap_id);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_TPC */
#endif /* end of alg_tpc.h */
