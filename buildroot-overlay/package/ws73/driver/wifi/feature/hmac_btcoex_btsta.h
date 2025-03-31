/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac_btcoex_btsta.c的头文件
 * Date: 2023-01-16 15:01
 */

#ifndef __HMAC_BTCOEX_BTSTA_H__
#define __HMAC_BTCOEX_BTSTA_H__

#include "oam_ext_if.h"
#include "mac_vap_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void hmac_btcoex_status_dispatch(hmac_vap_stru *hmac_vap,
    const hal_btcoex_btble_status_stru *status_old, hal_btcoex_btble_status_stru *status_new);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_BTCOEX_BTSTA_H__ */
