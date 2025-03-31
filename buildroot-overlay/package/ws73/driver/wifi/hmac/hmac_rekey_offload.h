/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * 文 件 名   : hmac_rekey_offload.h
 * 生成日期   : 2022年10月14日
 * 功能描述   : hmac_rekey_offload.c的头文件
 */
#ifndef __HMAC_REKEY_OFFLOAD_H__
#define __HMAC_REKEY_OFFLOAD_H__
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "msg_wow_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_REKEY_ARP_OFFLOAD_H

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST


osal_void hmac_rekey_offload_get_param(mac_rekey_offload_param_sync *rekey_param);

static osal_u32 hmac_rekey_offload_init_weakref(osal_void) __attribute__ ((weakref("hmac_rekey_offload_init"), used));
static osal_void hmac_rekey_offload_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_rekey_offload_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif