/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: main head file.
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

#define WLAN_INIT_DEVICE_RADIO_CAP  (1)  /* WLAN使能2G */

extern osal_s32  oal_main_init_etc(osal_void);
extern osal_void  oal_main_exit_etc(osal_void);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_main */
