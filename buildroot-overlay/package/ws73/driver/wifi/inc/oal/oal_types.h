/*
 * Copyright (c) CompanyNameMagicTag 2019-2020. All rights reserved.
 * Description: oal type head file.
 */

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifndef __OAL_TYPES_HCM_H__
#define __OAL_TYPES_HCM_H__

#include <osal_types.h>
#include "td_base.h"
#include "oal_types_device.h"
#include "oal_plat_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  3 宏定义
*****************************************************************************/
#ifndef OAL_INLINE
#ifdef INLINE_TO_FORCEINLINE
#define OAL_INLINE      __forceinline
#else
#define OAL_INLINE          inline
#endif
#endif
#undef OAL_STATIC
#define OAL_STATIC          static

#if defined(_PRE_PC_LINT)    /* 仅适用于PC_LINT工程时 */
#define OAL_VOLATILE
#else
#define OAL_VOLATILE        volatile
#endif

#define oal_reference(data)	((void)(data))

#define OAL_CONST const

#ifndef unref_param
#define unref_param(P)  ((P) = (P))
#endif

#ifndef unref_param_prv
#define unref_param_prv(P)  ((P) = (P))
#endif

/* linux错误码 */
#define OAL_ETIMEDOUT           110 /* Connection timed out */

/* MAC ADDR 移位定义 */
#define MAC_ADDR_0  0
#define MAC_ADDR_1  1
#define MAC_ADDR_2  2
#define MAC_ADDR_3  3
#define MAC_ADDR_4  4
#define MAC_ADDR_5  5

/*****************************************************************************
  4 枚举定义
*****************************************************************************/

/* 定义功能开关 */
typedef enum {
    OAL_SWITCH_OFF  = 0,
    OAL_SWITCH_ON   = 1,

    OAL_SWITCH_BUTT
} oal_switch_enum;
typedef osal_u8 oal_switch_enum_uint8;

/*****************************************************************************
  5 全局变量声明
*****************************************************************************/


/*****************************************************************************
  6 消息头定义
*****************************************************************************/


/*****************************************************************************
  7 消息定义
*****************************************************************************/


/*****************************************************************************
  8 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  9 UNION定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of file */

