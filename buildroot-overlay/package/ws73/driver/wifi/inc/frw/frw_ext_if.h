/*
 * Copyright (c) @CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Header file of frw public interface.
 * Author: Huanghe
 * Create: 2020-01-01
 */

#ifndef __FRW_EXT_IF_H__
#define __FRW_EXT_IF_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_list.h"
#include "oam_ext_if.h"
#include "frw_msg_rom.h"
#include "oal_list.h"
#include "frw_ext_common_rom.h"
#include "frw_hcc_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    OAM_MODULE_ID_ALL,
    OAM_MODULE_ID_WAL,
    OAM_MODULE_ID_HMAC,
    OAM_MODULE_ID_DMAC,

    OAM_MODULE_ID_HAL,
    OAM_MODULE_ID_OAM,
    OAM_MODULE_ID_OAL,
    OAM_MODULE_ID_FRW,
    OAM_MODULE_ID_ALG,
    OAM_MODULE_ID_PLAT,
    OAM_MODULE_ID_SDT,

    OAM_MODULE_ID_BUTT
} oam_module_id_enum;

#undef  THIS_FILE_ID
#define THIS_FILE_ID  OAM_FILE_ID_FRW_EXT_IF_H

typedef osal_u16 oam_module_id_enum_uint16;

#define FRW_TIMER_DEFAULT_TIME 10
#define frw_timer_restart_timer(timeout, timeout_val, is_periodic) \
    frw_timer_restart_timer_inner(timeout, timeout_val, is_periodic, THIS_FILE_ID, __LINE__)
#define frw_destroy_timer_entry(timeout) frw_timer_destroy_timer(THIS_FILE_ID, __LINE__, timeout)
#define frw_create_timer_entry(timeout, timeout_func, timeout_val, timeout_arg, is_periodic) \
    do { \
        frw_init_timer(timeout, timeout_func, timeout_arg, is_periodic); \
        frw_timer_create_timer(THIS_FILE_ID, __LINE__, timeout, timeout_val); \
    } while (0)
/*****************************************************************************
    全局变量声明
*****************************************************************************/
/*****************************************************************************
    函数声明
*****************************************************************************/
/* host  frw_main */
extern osal_s32  frw_main_init_etc(osal_void);
extern osal_void  frw_main_exit_etc(osal_void);
extern osal_void frw_main_destroy(osal_void);
extern osal_void  frw_set_init_state_etc(frw_init_enum_uint16 init_state);
extern frw_init_enum_uint16  frw_get_init_state_etc(osal_void);

extern osal_void  frw_event_process_all_event_etc(osal_ulong data);
extern osal_void frw_timer_restart_timer_inner(frw_timeout_stru *timeout, osal_u32 timeout_val,
    oal_bool_enum_uint8 is_periodic, osal_u32 file_id, osal_u32 line);
extern osal_void  frw_timer_add_timer(frw_timeout_stru *timeout);
extern osal_void frw_timer_create_timer_inner(osal_u32 file_id, osal_u32 line_num,
    frw_timeout_stru *timeout, osal_u32 time_val);
extern osal_void  frw_timer_stop_timer(frw_timeout_stru *timeout);
extern osal_void  frw_timer_delete_all_timer(osal_void);
extern osal_void   frw_timer_clean_timer(oam_module_id_enum_uint16 module_id);

/*****************************************************************************
 函 数 名  : frw_init_timer
 功能描述  : 定时器初始化
*****************************************************************************/
static INLINE__ osal_void frw_init_timer(frw_timeout_stru *timer,
    const frw_timeout_func func, osal_void *arg, oal_bool_enum_uint8 periodic)
{
    timer->func = func;
    timer->timeout_arg = arg;
    timer->is_periodic = periodic;
}

osal_void frw_timer_create_timer(osal_u32 file_id, osal_u32 line_num, frw_timeout_stru *timeout, osal_u32 time_val);
osal_void frw_timer_destroy_timer(osal_u32 file_id, osal_u32 line_num, frw_timeout_stru *timeout);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of frw_ext_if.h */
