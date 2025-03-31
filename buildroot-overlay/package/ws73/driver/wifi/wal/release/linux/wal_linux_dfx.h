/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal_linux_dfx
 * Create: 2022-09-23
 */
#ifndef __WAL_LINUX_DFX_H__
#define __WAL_LINUX_DFX_H__

#ifdef _PRE_WLAN_FEATURE_DFR
#include "oal_ext_if.h"
#include "oal_types.h"
#include "plat_pm_wlan.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_DFX_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define WAL_P2P_CHANGE_TIME_MAX 40
#define WAL_P2P_CHANGE_WAIT_MS 500
#define WAL_DFR_AP_WAIT_START_TIME_MS 100
#define WAL_DFR_WAIT_KICK_USER_MS 2000

typedef enum {
    WAL_DFR_RECOVERY_INIT = 0,
    WAL_DFR_START_RECOVERY,
    WAL_DFR_P2P_RECOVERY,
    WAL_DFR_STA_AP_RECOVERY,
    WAL_DFR_FINISH_RECOVERY
} wal_dfr_status_enum;

osal_u32 uapi_ccpriv_dfr_start(oal_net_device_stru *net_dev, osal_s8 *param);
osal_u32 wal_dfx_init(void);
osal_void wal_dfx_exit(void);
osal_void wal_dfr_main_reinit_later(osal_void);
wal_dfr_status_enum wal_dfr_get_status(osal_void);
osal_void wal_dfr_save_p2p_change_record(hmac_vap_stru *hmac_vap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
#endif
