/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal 11d api.
 * Create: 2021-07-16
 */

#ifndef __WAL_LINUX_11D_H__
#define __WAL_LINUX_11D_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_11D_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_s32  wal_regdomain_update_for_dfs_etc(oal_net_device_stru *net_dev, osal_s8 *pc_country);
osal_s32  wal_regdomain_update_etc(oal_net_device_stru *net_dev, osal_s8 *pc_country);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
