/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: wal_main.h file.
 */

#ifndef __WAL_MAIN_H__
#define __WAL_MAIN_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_MAIN_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST


/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* HOST DRX事件子类型 */
typedef enum {
    WAL_HOST_DRX_SUBTYPE_TX,

    WAL_HOST_DRX_SUBTYPE_BUTT
} wal_host_drx_subtype_enum;
typedef osal_u8 wal_host_drx_subtype_enum_uint8;

/*****************************************************************************
  函数声明
*****************************************************************************/
extern osal_s32  wal_main_init_etc(osal_void);
extern osal_void  wal_main_exit_etc(osal_void);
extern oal_wakelock_stru   g_st_wal_wakelock_etc;
extern osal_void wal_wake_lock(osal_void);
extern osal_void wal_wake_unlock(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_main */
