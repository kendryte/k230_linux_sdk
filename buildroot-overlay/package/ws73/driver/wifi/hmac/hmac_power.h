/*
* Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
* Description: hmac power head
* Create: 2020-7-5
*/

#ifndef __HMAC_POWER_H__
#define __HMAC_POWER_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_pow_set_vap_tx_power(hmac_vap_stru *hmac_vap, hal_pow_set_type_enum_uint8 type);
osal_s32 hmac_power_set_rate_power_offset(hmac_vap_stru *hmac_vap, const frw_msg *msg);
extern osal_s32 hmac_config_host_dev_init_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 hmac_config_host_dev_exit_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_power.h */
