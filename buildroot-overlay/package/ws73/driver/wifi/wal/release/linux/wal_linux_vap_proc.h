/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal vap info proc api.
 * Author: Huanghe
 * Create: 2021-07-19
 */

#ifndef __WAL_LINUX_VAP_PROC_H__
#define __WAL_LINUX_VAP_PROC_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_VAP_PROC_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
osal_void wal_add_vap_proc_file(hmac_vap_stru *hmac_vap, const osal_s8 *pc_name);
osal_void wal_del_vap_proc_file(oal_net_device_stru *net_dev);

#elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,59))
osal_void wal_add_vap_proc_file_sec(hmac_vap_stru *hmac_vap, osal_s8 *pc_name);
osal_void wal_del_vap_proc_file_sec(oal_net_device_stru *net_dev);

#endif

#ifdef _PRE_WLAN_FEATURE_SNIFFER
osal_u32 wal_wifi_sniffer_proc_file_sec(void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
