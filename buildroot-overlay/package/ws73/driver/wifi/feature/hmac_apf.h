/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: APF hmac function header.
 * Create: 2022-10-14
 */

#ifndef HMAC_APF_H
#define HMAC_APF_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_apf.h"
#include "hmac_vap.h"
#include "frw_ext_if.h"
#include "oam_struct.h"
#include "oal_mem_hcm.h"
#include "diag_log_common.h"
#include "hmac_auto_adjust_freq.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_APF_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define APF_PROGRAM_IPV4_BEGIN 76
#define APF_PROGRAM_IPV4_END 79
#define APF_PROGRAM_IPV4_BROADCAST_BEGIN 143
#define APF_PROGRAM_IPV4_BROADCAST_END 145

/*****************************************************************************
  10 函数声明
*****************************************************************************/
static osal_u32 hmac_apf_init_weakref(osal_void) __attribute__ ((weakref("hmac_apf_init"), used));
static osal_void hmac_apf_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_apf_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_apf.h */