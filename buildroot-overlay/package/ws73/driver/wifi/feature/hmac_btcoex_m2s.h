/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_btcoex_m2s.c的头文件
 * Date: 2023-01-31 10:14
 */
#ifndef __HMAC_BTCOEX_M2S_H__
#define __HMAC_BTCOEX_M2S_H__

#include "hal_ops_common_rom.h"
#include "mac_vap_ext.h"
#include "oal_types_device_rom.h"
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_u32 hmac_btcoex_m2s_switch_apply(hal_to_dmac_device_stru *hal_device, osal_u8 m2s_mode_mask);
osal_u32 hmac_btcoex_s2m_switch_apply(hal_to_dmac_device_stru *hal_device, osal_u8 m2s_mode_mask);
osal_u32 hmac_btcoex_s2m_siso_ap_resume_callback(osal_void *arg);
osal_void hmac_btcoex_assoc_ap_check_process(hal_to_dmac_device_stru *hal_device);

typedef oal_bool_enum_uint8 (*hmac_m2s_btcoex_need_switch_check_cb)(const hal_to_dmac_device_stru *hal_device,
    osal_u8 *m2s_mode_mask);
typedef osal_void (*hmac_btcoex_m2s_allow_check_cb)(hal_to_dmac_device_stru *hal_device,
    const hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_btcoex_m2s_back_to_mimo_check_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_bool (*hmac_btcoex_m2s_back_to_mimo_cb)(hal_to_dmac_device_stru *hal_device, osal_u8 *m2s_mode_mask);
typedef osal_bool (*hmac_btcoex_m2s_update_hal_device_cb)(const hal_to_dmac_device_stru *hal_device,
    osal_u8 *m2s_mode_mask);
typedef osal_void (*hmac_btcoex_m2s_choose_mimo_siso_cb)(hal_to_dmac_device_stru *hal_device);
osal_u32 hmac_btcoex_m2s_init(osal_void);
osal_void hmac_btcoex_m2s_deinit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_BTCOEX_M2S_H__ */
