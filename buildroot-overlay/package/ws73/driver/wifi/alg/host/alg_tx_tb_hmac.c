/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm tx tb
 */

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "hmac_device.h"
#include "hal_common_ops.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"
#include "alg_tx_tb_hmac.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_TX_TB_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/******************************************************************************
 功能描述  : DBAC参数同步，入参有效性由notify函数保证
******************************************************************************/
OAL_STATIC osal_u32 alg_tx_tb_para_sync_fill(alg_param_sync_stru *data)
{
    alg_param_sync_tx_tb_stru *tx_tb_para = &data->tx_tb_para;
    memset_s(tx_tb_para, sizeof(alg_param_sync_tx_tb_stru), 0, sizeof(alg_param_sync_tx_tb_stru));

    /* init alg parameters to default value */
    tx_tb_para->suspend_tx_flag = OAL_TRUE;
    tx_tb_para->tx_tb_in_ms_th = ALG_TX_TB_IN_MS_TH;
    tx_tb_para->tx_tb_out_ms_th = ALG_TX_TB_OUT_MS_TH;
    tx_tb_para->tx_tb_in_th = ALG_TX_TB_IN_TH;
    tx_tb_para->tx_tb_out_th = ALG_TX_TB_OUT_TH;
    return OAL_SUCC;
}
/******************************************************************************
 功能描述  : ALG DBAC HMAC模块初始化总入口
******************************************************************************/
osal_void alg_tx_tb_hmac_init(osal_void)
{
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_TX_TB, alg_tx_tb_para_sync_fill);
}

/******************************************************************************
 功能描述  : ALG DBAC HMAC模块卸载
******************************************************************************/
osal_void alg_tx_tb_hmac_exit(osal_void)
{
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_TX_TB);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
