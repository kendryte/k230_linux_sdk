/*
 * Copyright (c) @CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Header file of frw_timer.c.
 * Author: Huanghe
 * Create: 2020-01-01
 */

#ifndef __FRW_TIMER_H__
#define __FRW_TIMER_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_timer.h"
#include "frw_ext_if.h"
#include "mac_vap_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TIMER_H
/*****************************************************************************
    宏定义
*****************************************************************************/
#define FRW_TIMER_ENTRY_MAGIC_NUM 0xDEADBEEF /* 填充给注册的timer，防止被调用处memset清除后导致timer异常 */
/*****************************************************************************
    函数声明
*****************************************************************************/
osal_void frw_timer_init(osal_u32 delay, oal_timer_func func, osal_ulong arg);
osal_void frw_timer_exit(osal_void);
osal_s32 frw_timer_timeout_proc(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_FEATURE_WS73
osal_void frw_timer_timeout_proc_event_etc(uintptr_t arg);
#else
osal_void frw_timer_timeout_proc_event_etc(unsigned long arg);
#endif
osal_void frw_timer_dump(osal_u32 core_id);
/*****************************************************************************
    OTHERS定义
*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of frw_timer.h */
