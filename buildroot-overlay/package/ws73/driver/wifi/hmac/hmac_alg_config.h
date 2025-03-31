/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Header file for hmac_alg_config.c
 */

#ifndef __HMAC_ALG_CONFIG_H__
#define __HMAC_ALG_CONFIG_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_H

/*****************************************************************************
  2 结构体定义
*****************************************************************************/
/* 配置发送速率 */
typedef struct {
    wlan_phy_protocol_enum_uint8 protocol;
    hal_channel_assemble_enum bw;
    mac_alg_cfg_enum_uint16 alg_cfg;
    osal_u8 fix_rate_mode;
    osal_u8 resv[3];
    osal_u32 rate_index;
} alg_cfg_tx_rate_param_stru;

/*****************************************************************************
  3 函数声明
*****************************************************************************/
static osal_u32 hmac_alg_config_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_alg_config_init"), used));
static osal_void hmac_alg_config_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_alg_config_deinit"), used));
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
osal_s32 alg_intrf_mode_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 alg_intrf_mode_todmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
osal_u32 hmac_config_pk_mode(hmac_vap_stru *hmac_vap, osal_u8 pk_mode);
#endif
osal_s32 hmac_ccpriv_alg_cfg_etc(hmac_vap_stru *hmac_vap, const osal_s8 *param);
osal_void hmac_query_alg_tx_rate_stats(osal_u8 value);
osal_void hmac_query_alg_rx_rate_stats(osal_u8 value);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_alg_config */
