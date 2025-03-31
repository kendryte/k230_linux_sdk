/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac_keep_alive
 * Author:
 * Create: 2022-09-22
 */
#ifndef __HMAC_KEEP_ALIVE_H__
#define __HMAC_KEEP_ALIVE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_vap_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_KEEP_ALIVE_H

osal_s32 hmac_config_keepalive_sync_timestamp(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_keepalive_mode(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_common_debug(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_config_get_ap_user_aging_time(osal_void);
osal_s32 hmac_config_set_ap_user_aging_time(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_keep_alive.h */

