/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: header for cali_tx_complete
 * Create: 2022-10-18
 */
#ifndef __CALI_TX_COMPLETE_H__
#define __CALI_TX_COMPLETE_H__
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_common_ops_rom.h"
#include "dmac_ext_if_rom.h"
#include "fe_extern_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    ONLINE_CALI_PDET_DISABLE = 0,          /* PDET不使能 */
    ONLINE_CALI_PDET_ENABLE_C0,            /* 通道0 PDET使能 */
    ONLINE_CALI_PDET_ENABLE_C1,            /* 通道1 PDET使能 */
    ONLINE_CALI_PDET_ENABLE_C2,            /* 通道2 PDET使能 */
    ONLINE_CALI_PDET_ENABLE_C3,            /* 通道3 PDET使能 */

    ONLINE_CALI_PDET_EN_BUTT
} online_cali_pdet_en_enum;

osal_u8 online_cali_get_cali_mask(osal_void);
osal_void online_cali_set_cali_mask(osal_u8 value);
osal_void cali_proc_set_channel(hal_to_dmac_device_stru *device, osal_u8 channel_num,
    wlan_channel_band_enum_uint8 band, osal_u8 channel_idx, wlan_channel_bandwidth_enum_uint8 bandwidth);

osal_void online_cali_proc_tx_send(hal_to_dmac_device_stru *hal_device,
    hal_tx_txop_alg_stru *txop_alg, wlan_frame_type_enum_uint8 frame_type);

/* 动态校准 */
osal_s32 online_cali_flush_dyn_mask(dmac_vap_stru *dmac_vap, frw_msg *msg);

osal_void online_cali_get_pdet_val(hal_tx_complete_event_stru *tx_comp_event);

osal_void online_cali_proc_tx_complete(osal_u8 vap_id, hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *curr_dscr, hal_tx_complete_event_stru *tx_comp_event);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __CALI_TX_COMPLETE_H__