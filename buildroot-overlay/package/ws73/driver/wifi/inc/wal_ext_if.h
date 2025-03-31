/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of wal public interface.
 * Create: 2020-01-01
 */

#ifndef __WAL_EXT_IF_H__
#define __WAL_EXT_IF_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_schedule.h"
#include "wlan_types_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_EXT_IF_H

/* 维测，设置某一种具体event上报开关的参数结构体 */
typedef struct {
    osal_s32                           l_event_type;
    osal_s32                           l_param;
} wal_specific_event_type_param_stru;

/*****************************************************************************
    全局变量
*****************************************************************************/
extern osal_u8         g_uc_custom_cali_done_etc; /* 标识是否是上电第一次校准 */

/*****************************************************************************
    函数声明
*****************************************************************************/
extern osal_s32  wal_main_init_etc(void);
extern void  wal_main_exit_etc(void);

extern osal_u32 wal_ccpriv_register_inetaddr_notifier_etc(void);
extern osal_u32 wal_ccpriv_unregister_inetaddr_notifier_etc(void);
extern osal_u32 wal_ccpriv_register_inet6addr_notifier_etc(void);
extern osal_u32 wal_ccpriv_unregister_inet6addr_notifier_etc(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_ext_if.h */
