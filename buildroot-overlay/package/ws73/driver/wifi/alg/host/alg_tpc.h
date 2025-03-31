/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm tpc
 */

#ifndef __ALG_TPC_H__
#define __ALG_TPC_H__

#ifdef _PRE_WLAN_FEATURE_TPC

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "frw_ext_if.h"
#include "hmac_alg_if.h"
#include "alg_transplant.h"
#include "msg_alg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void alg_tpc_hmac_init(osal_void);
osal_void alg_tpc_hmac_exit(osal_void);
osal_u8 alg_get_api_tpc_api_3db_mode(osal_void);
osal_void alg_set_api_tpc_api_3db_mode(osal_u8 add_flag, osal_u8 under_ragulation_flag);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_TPC */
#endif /* end of alg_tpc.h */
