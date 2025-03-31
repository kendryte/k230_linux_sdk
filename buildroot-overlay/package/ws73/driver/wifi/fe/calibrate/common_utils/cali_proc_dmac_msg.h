/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description:
 * Create: 2022-08-30
 */

#ifndef __CALI_PROC_DMAC_MSG_H__
#define __CALI_PROC_DMAC_MSG_H__

#include "dmac_ext_if_rom.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
osal_s32 dmac_hal_rf_fsm_sync_state(dmac_vap_stru *dmac_vap, frw_msg *msg);
osal_s32 dmac_hal_rf_work_entry(dmac_vap_stru *dmac_vap, frw_msg *msg);
osal_s32 dmac_config_hcc_send_ready_msg(dmac_vap_stru *dmac_vap, frw_msg *msg);
osal_u32 dmac_config_cali_data_report(dmac_vap_stru *dmac_vap);
osal_s32 dmac_cali_data_buffer_download(dmac_vap_stru *dmac_vap, frw_msg *msg);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
