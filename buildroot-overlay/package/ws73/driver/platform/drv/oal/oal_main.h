/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: OAL main header file.
 * Author: Huanghe
 * Create: 2020-10-13
 */

#ifndef __OAL_MAIN_H__
#define __OAL_MAIN_H__

#include "oal_ext_if.h"
#include "oal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WLAN_INIT_DEVICE_RADIO_CAP  (3)  /* WLAN使能2G_5G */
extern oal_uint8  g_auc_wlan_service_device_per_chip[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP];

extern oal_uint32 oal_chip_get_version_etc(oal_void);
extern oal_uint8 oal_chip_get_device_num_etc(oal_uint32 ul_chip_ver);
extern oal_uint8 oal_board_get_service_vap_start_id(oal_void);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_main */
