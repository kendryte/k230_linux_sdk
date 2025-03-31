/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: header file of alg_autorate.c
 */

#ifndef __ALG_AUTORATE_H__
#define __ALG_AUTORATE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "alg_transplant.h"
#include "alg_main.h"

#include "hal_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_AUTORATE
typedef struct {
    osal_u32 succ;
    osal_u32 fail;
    osal_u32 rts_fail;
    osal_u32 send_cnt;
    osal_u32 retry;
} alg_rate_stats_stru;

#define ALG_LEGACY_RATE_STATS_CNT 1
typedef struct {
    alg_rate_stats_stru he_rate_stats[WLAN_HE_MCS_BUTT];
    alg_rate_stats_stru ht_rate_stats[WLAN_HT_MCS_BUTT];
    alg_rate_stats_stru legacy_rate_stats[ALG_LEGACY_RATE_STATS_CNT];
} alg_rate_all_stats_stru;

typedef struct {
    osal_u32 cnt;
} alg_rx_rate_stats_stru;

typedef struct {
    alg_rx_rate_stats_stru he_rx_rate_stats[WLAN_HE_MCS_BUTT];
    alg_rx_rate_stats_stru ht_rx_rate_stats[WLAN_HT_MCS_BUTT];
    alg_rx_rate_stats_stru legacy_rx_rate_stats[ALG_LEGACY_RATE_STATS_CNT];
} alg_rx_rate_all_stats_stru;

typedef enum {
    ALG_REJISTER_AND_CLEAR_RATE_STATS = 0,
    ALG_GET_RATE_STATS = 1,
    ALG_UNREJISTER_RATE_STATS = 2
} alg_rate_stats_enum;
/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void alg_hmac_autorate_init(osal_void);
osal_void alg_hmac_autorate_exit(osal_void);
osal_void alg_autorate_get_rx_rate_stats(alg_rx_rate_all_stats_stru *stats);
osal_u32 alg_autorate_rx_data_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf, hal_rx_statistic_stru *rx_stats);
osal_void alg_autorate_get_rx_rate_stats_clear(osal_void);
#endif /* #ifdef _PRE_WLAN_FEATURE_AUTORATE */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of alg_autorate.h */
