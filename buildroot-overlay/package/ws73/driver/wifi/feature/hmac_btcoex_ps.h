/*
 * Copyright: Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_btcoex_ps.c的头文件
 * Date: 2023-01-28 15:24
 */

#ifndef __HMAC_BTCOEX_PS_H__
#define __HMAC_BTCOEX_PS_H__

#include "hal_ext_if.h"
#include "hmac_btcoex.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
osal_void hmac_btcoex_init_preempt(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user);
osal_void hmac_btcoex_set_ps_flag(hal_to_dmac_device_stru *hal_device);
osal_void hmac_btcoex_set_vap_ps_frame(hmac_vap_stru *hmac_vap, osal_u32 enable);
osal_void hmac_btcoex_set_dev_ps_frame(hal_to_dmac_device_stru *hal_device);
osal_void  hmac_btcoex_ps_timeout_update_time(hal_to_dmac_device_stru *hal_device);
osal_void hmac_btcoex_ps_stop_check_and_notify(hal_to_dmac_device_stru *hal_device);
osal_u32 hmac_btcoex_pow_save_callback(osal_void *arg);
osal_void hmac_btcoex_ps_on_proc(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *h2d_device, hal_chip_stru *hal_chip);
osal_void hmac_btcoex_ps_off_scan_state_proc(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *h2d_device);
osal_void hmac_btcoex_ps_off_no_scan_state_proc(hal_to_dmac_device_stru *h2d_device);
#endif
 
typedef osal_u8 hal_coex_sw_preempt_subtype_uint8;
typedef osal_u8 hmac_btcoex_ps_switch_enum_uint8;

typedef osal_u32 (*hmac_btcoex_set_sw_preempt_type_cb)(hal_to_dmac_device_stru *hal_device,
    hal_coex_sw_preempt_subtype_uint8 type);
typedef osal_void (*hmac_btcoex_ps_stop_check_and_notify_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hmac_btcoex_one_pkt_type_and_duration_update_cb)(const mac_fcs_mgr_stru *fcs_mgr,
    hal_one_packet_cfg_stru *one_packet_cfg, hmac_vap_stru *hmac_vap);
osal_u32 hmac_btcoex_ps_init(osal_void);
osal_void hmac_btcoex_ps_deinit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_BTCOEX_PS_H__ */
