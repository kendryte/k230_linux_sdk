/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_btcoex_notify.c的头文件
 * Date: 2023-02-01 15:33
 */
#ifndef __HMAC_BTCOEX_NOTIFY_H__
#define __HMAC_BTCOEX_NOTIFY_H__

#include "hal_ops_common_rom.h"
#include "hmac_btcoex.h"
#include "mac_vap_ext.h"
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
/*****************************************************************************
  8 ROM预留回调函数类型定义
*****************************************************************************/
typedef osal_u32 (*hmac_btcoex_update_ps_capability_resv)(hal_to_dmac_device_stru *hal_device,
    hmac_btcoex_ps_stru *ps_param);
/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_btcoex_set_freq_and_work_state_hal_device(hal_to_dmac_device_stru *hal_device);

osal_void hmac_config_btcoex_assoc_state_proc(hmac_vap_stru *hmac_vap, const ble_status_stru *ble_status,
    hmac_user_btcoex_delba_stru *btcoex_delba, const bt_status_stru *bt_status,
    const hal_btcoex_btble_status_stru *btcoex_btble_status);
osal_bool hmac_config_btcoex_disassoc_state_syn(osal_void *notify_data);
#endif

typedef osal_void (*hmac_btcoex_set_wifi_status_irq_notify_cb)(hal_to_dmac_device_stru *hal_device,
    osal_u8 m2s_mode_mask);
typedef osal_void (*hmac_btcoex_notify_inout_siso_cb)(hal_to_dmac_device_stru *hal_device);

osal_u32 hmac_btcoex_notify_init(osal_void);
osal_void hmac_btcoex_notify_deinit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_BTCOEX_NOTIFY_H__ */
