/*
* Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
* Description: dmac tx opps head
* Create: 2020-7-5
*/

#ifndef HMAC_TXOPPS_H
#define HMAC_TXOPPS_H

#ifdef _PRE_WLAN_FEATURE_TXOPPS
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 软件配置mac txopps使能寄存器需要的三个参数 */
typedef struct {
    osal_u8 machw_txopps_en;         /* sta是否使能txopps */
    osal_u8 machw_txopps_condition1; /* txopps条件1 */
    osal_u8 machw_txopps_condition2; /* txopps条件2 */
    osal_u8 auc_resv[1];
} mac_txopps_machw_param_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32  hmac_txopps_set_machw_en_sta(const hmac_vap_stru *hmac_vap,
    const mac_txopps_machw_param_stru *txopps_machw_param);
osal_u32  hmac_txopps_set_machw(hmac_vap_stru *hmac_vap);
osal_s32  hmac_txopps_set_machw_partialaid(hmac_vap_stru *hmac_vap, osal_u32 partial_aid);

osal_u8 hmac_vap_get_txopps(hmac_vap_stru *hmac_vap);
void hmac_vap_set_txopps(hmac_vap_stru *hmac_vap, osal_u8 value);
osal_void mac_vap_update_txopps(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
extern osal_void hmac_sta_set_txopps_partial_aid(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_txopps.h */
