/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file for hmac_tx_pk.c.
 * Create: 2021-12-15
 */

#ifndef __HMAC_TX_PK_H__
#define __HMAC_TX_PK_H__

#ifdef _PRE_WLAN_FEATURE_PK
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_mgmt_classifier.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_s32 hmac_bridge_vap_xmit_etc_pk(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif

#endif /* end of hmac_tx_pk.h */
