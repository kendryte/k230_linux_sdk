/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: osal types header.
 * This file should be changed only infrequently and with great care.
 */

#ifndef __OSAL_TYPES_H__
#define __OSAL_TYPES_H__

/* 基本数据类型定义 */
typedef unsigned char           osal_uchar;
typedef unsigned char           osal_u8;
typedef unsigned short          osal_u16;
typedef unsigned int            osal_u32;
typedef unsigned long long      osal_u64;
typedef unsigned long           osal_ulong;
typedef char                    osal_char;
typedef signed char             osal_s8;
typedef short                   osal_s16;
typedef int                     osal_s32;
typedef long long               osal_s64;
typedef long                    osal_slong;
typedef float                   osal_float;
typedef double                  osal_double;
typedef unsigned long           osal_size_t;
typedef unsigned long           osal_length_t;
typedef osal_u32                osal_handle;
typedef osal_u8                 osal_bool;
// typedef unsigned int            uintptr_t;
typedef void                    osal_void;
typedef void*                   osal_pvoid;
typedef osal_u8                 osal_byte;
typedef osal_byte*              osal_pbyte;
typedef unsigned long long      uint64_t;
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;

typedef signed char             int8_t;
typedef short                   int16_t;
typedef int                     int32_t;
typedef unsigned int            osal_uintptr_t;
typedef unsigned int            osal_phys_addr_t;
typedef unsigned int            osal_virt_addr_t;

/* defines */
#ifndef NULL
#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif
#endif

#define OSAL_U32_MAX            0xFFFFFFFF
#define OSAL_U64_MAX            0xFFFFFFFFFFFFFFFFUL
#define OSAL_U16_MAX            0xFFFF
#define OSAL_U8_MAX             0xFF
#define OSAL_U4_MAX             0x0f

#define OSAL_FALSE 0
#define OSAL_TRUE  1

#define OSAL_OK 0
#define OSAL_NOK 1

#ifdef __cplusplus
#define OSAL_NULL       0
#else

#ifndef OSAL_NULL
#define OSAL_NULL    ((void *)0)
#endif
#endif
#ifdef BUILD_UT
#define OSAL_STATIC
#else
#define OSAL_STATIC static
#endif
#define OSAL_SYS_WAIT_FOREVER           0xFFFFFFFF

#endif /* __OSAL_TYPES_H__ */