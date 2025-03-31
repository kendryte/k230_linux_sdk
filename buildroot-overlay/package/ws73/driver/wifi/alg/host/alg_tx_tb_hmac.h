/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm tx tb
 */

#ifndef __ALG_TX_TB_HMAC_H__
#define __ALG_TX_TB_HMAC_H__

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
#define ALG_TX_TB_IN_TH 2
#define ALG_TX_TB_OUT_TH 0
#define ALG_TX_TB_IN_MS_TH 20
#define ALG_TX_TB_OUT_MS_TH 100
osal_void alg_tx_tb_hmac_init(osal_void);
osal_void alg_tx_tb_hmac_exit(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
