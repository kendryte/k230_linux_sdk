/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_device_fsm_rom.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_DEVICE_FSM_H__
#define __HAL_DEVICE_FSM_H__

/*****************************************************************************
    1 头文件包含
*****************************************************************************/
#include "hal_common_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HMAC_HAL_FSM_TIMER_IDLE_TIMEOUT        20
/*****************************************************************************
    函数声明
*****************************************************************************/
osal_void hal_device_fsm_attach(hal_to_dmac_device_stru *hal_device);
osal_void hal_device_fsm_detach(hal_to_dmac_device_stru *hal_device);
osal_void hal_device_handle_event(hal_to_dmac_device_stru *hal_device, osal_u16 type, osal_u16 datalen, osal_u8 *data);
osal_u32 hal_device_fsm_trans_to_state(hal_dev_fsm_stru *hal_dev_fsm, osal_u8 state, osal_u16 event);
#ifdef _PRE_WLAN_DFT_STAT
osal_void hal_device_fsm_info(hal_to_dmac_device_stru *hal_device);
#endif
osal_u8 hal_device_get_mac_pa_switch(osal_void);
osal_void hal_device_set_mac_pa_switch(osal_u8 val);
osal_void hal_dev_sync_work_vap_bitmap(osal_u32 work_vap_bitmap);
osal_void hal_pm_enable_front_end_to_dev(hal_to_dmac_device_stru *hal_dev, osal_bool enable_paldo);
osal_void hal_dev_sync_mac_pa_switch_to_dev(osal_u8 mac_pa_switch);
osal_void hal_dev_req_sync_pm_bitmap(osal_u8 hal_vap_id, osal_u8 work_sub_state);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_device_fsm.h */
