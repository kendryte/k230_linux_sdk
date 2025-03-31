/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal android cmd proc api.
 * Author: Huanghe
 * Create: 2021-07-20
 */

#ifndef __WAL_LINUX_ANDROID_CMD_H__
#define __WAL_LINUX_ANDROID_CMD_H__

#include "oal_ext_if.h"
#include "wlan_types_common.h"
#include "wlan_spec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_ANDROID_CMD_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_s32 uapi_android_priv_cmd_etc(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr, osal_s32 ul_cmd);
osal_s32 uapi_android_set_wps_p2p_ie_inner(oal_net_device_stru *net_dev, app_ie_type_uint8 type,
    osal_u8 *buf, osal_u32 len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
