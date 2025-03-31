/*
 * Copyright (c) CompanyNameMagicTag 2019-2020. All rights reserved.
 * Description: oal type head file.
 */

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#ifndef __OAL_TYPES_DEVICE_ROM_H__
#define __OAL_TYPES_DEVICE_ROM_H__

#include "osal_types.h"
#include "common_error_code_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OAL_PTR_NULL (0L) /* 空指针定义，指针判断是否为空时与OAL_PTR_NULL进行比较 */

#ifndef NULL
#define NULL OAL_PTR_NULL
#endif

/* linux错误码 */
#define OAL_EFAIL 1   /* 内核通用错误返回值 -1 */
#define OAL_EINVAL 22 /* Invalid argument */

typedef enum {
    OAL_FALSE = 0,
    OAL_TRUE = 1,
    OAL_BUTT
} oal_bool_enum;
typedef osal_u8 oal_bool_enum_uint8;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_types_device.h */
