/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm antanne interference head
 */

#ifndef __ALG_RTS_H__
#define __ALG_RTS_H__

#ifdef _PRE_WLAN_FEATURE_RTS

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "hmac_alg_if.h"
#include "hal_alg_rts.h"
#include "msg_alg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/******************************************************************************
  2 函数声明
******************************************************************************/
osal_s32 alg_host_rts_init(osal_void);
osal_s32 alg_host_rts_exit(osal_void);
osal_u32 alg_rts_para_sync_fill(alg_param_sync_stru *data);
osal_u32 alg_rts_config_threshold(hmac_vap_stru *hmac_vap, osal_u32 rts_threshold);
osal_u32 alg_rts_set_host_param(mac_ioctl_alg_param_stru *alg_param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_RTS */
#endif /* end of alg_rts.h */