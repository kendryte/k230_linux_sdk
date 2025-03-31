/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Header file for alg_probe_common.c
 */

#ifndef __ALG_PROBE_COMMON_H__
#define __ALG_PROBE_COMMON_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "osal_types.h"
#include "oal_fsm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_s32 alg_probe_common_init(osal_void);
osal_void alg_probe_common_exit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
