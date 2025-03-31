/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: header for cali rate pow lut
 * Create: 2022-10-20
 */
#ifndef __FE_EXTERN_IF_HOST_H__
#define __FE_EXTERN_IF_HOST_H__

#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void online_cali_init(hal_to_dmac_device_stru *hal_device);
osal_s32 online_cali_tx_complete_dyn_process(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void cali_pow_initialize_tx_power(hal_to_dmac_device_stru *hal_to_dmac_device);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif