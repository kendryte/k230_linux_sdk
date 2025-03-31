/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac_dnb_sta.c的头文件
 * Date: 2022-10-18
 */

#ifndef __HMAC_DNB_STA_H__
#define __HMAC_DNB_STA_H__

#include "mac_vap_ext.h"
#include "osal_types.h"
#include "frw_msg_rom.h"
#include "mac_frame.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HMAC_MAX_HE_LTF_NUM 4

typedef osal_void (*hmac_set_dnb_cap_cb)(hmac_vap_stru *hmac_vap, osal_u8 **buffer);

static osal_u32 hmac_sta_dnb_init_weakref(osal_void) __attribute__ ((weakref("hmac_sta_dnb_init"), used));
static osal_void hmac_sta_dnb_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_sta_dnb_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
