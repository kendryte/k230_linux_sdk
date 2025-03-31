/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_uapsd_sta的头文件
 * Create: 2023-3-3
 */
#ifndef HMAC_UAPSD_STA_H
#define HMAC_UAPSD_STA_H

#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef osal_void (*hmac_uapsd_sta_set_qos_info_cb)(hmac_vap_stru *hmac_vap, osal_u8 *buffer);
typedef mac_cfg_uapsd_sta_stru* (*hmac_uapsd_sta_get_uapsd_info_cb)(osal_u8 vap_id);

static osal_u32 hmac_uapsd_sta_init_weakref(osal_void) __attribute__ ((weakref("hmac_uapsd_sta_init"), used));
static osal_void hmac_uapsd_sta_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_uapsd_sta_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif