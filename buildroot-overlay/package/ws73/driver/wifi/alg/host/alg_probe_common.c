/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: common algorithm rom.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "alg_autorate.h"
#include "frw_util.h"
#include "hmac_alg_notify.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_PROBE_COMMON_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
  2 函数定义
******************************************************************************/
OSAL_STATIC osal_u32 alg_common_para_sync_fill(alg_param_sync_stru *data)
{
    alg_param_sync_common_stru *sync_data = &data->common_para;
    sync_data->max_lock_times = ALG_LOCK_TIMES_TH;
    sync_data->lock_timestamp_th = ALG_LOCK_TIMESTAMP_TH;
    return OAL_SUCC;
}
osal_s32 alg_probe_common_init(osal_void)
{
    /* 注册初始化变量同步函数 */
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_COMMON, alg_common_para_sync_fill);
    return OAL_SUCC;
}
osal_void alg_probe_common_exit(osal_void)
{
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_COMMON);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

