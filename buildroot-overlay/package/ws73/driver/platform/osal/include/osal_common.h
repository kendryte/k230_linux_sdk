/*
 * Copyright (c) CompanyNameMagicTag. 2012-2020. All rights reserved.
 * Description: common type head file.
 * Author: Huanghe
 * Create: 2012-12-22
 */

#ifndef _OSAL_COMMON_H_
#define _OSAL_COMMON_H_
#include <osal_types.h>
#include <osal_errno.h>

#define unref_param(P)  ((P) = (P))

#ifndef UT_TEST
#undef __IRQ
#define __IRQ  __attribute__ ((interrupt ("IRQ")))
#else
#define __IRQ
#endif

#define __align(n) __attribute((aligned(n)))
#define __wfi() __asm("wfi\n")
#define __return_address() ((osal_u32)__builtin_return_address(0))

/* bit mask */
#define MSK_1_B                 0x1
#define MSK_2_B                 0x3
#define MSK_3_B                 0x7
#define MSK_4_B                 0xF
#define MSK_5_B                 0x1F
#define MSK_6_B                 0x3F
#define MSK_7_B                 0x7F
#define MSK_8_B                 0xFF
#define MSK_9_B                 0x1FF
#define MSK_10_B                0x3FF
#define MSK_11_B                0x7FF
#define MSK_12_B                0xFFF
#define MSK_13_B                0x1FFF
#define MSK_14_B                0x3FFF
#define MSK_15_B                0x7FFF
#define MSK_16_B                0xFFFF
#define MSK_17_B                0x1FFFF
#define MSK_18_B                0x3FFFF
#define MSK_19_B                0x7FFFF
#define MSK_20_B                0xFFFFF
#define MSK_21_B                0x1FFFFF
#define MSK_22_B                0x3FFFFF
#define MSK_23_B                0x7FFFFF
#define MSK_24_B                0xFFFFFF
#define MSK_25_B                0x1FFFFFF
#define MSK_26_B                0x3FFFFFF
#define MSK_27_B                0x7FFFFFF
#define MSK_28_B                0xFFFFFFF
#define MSK_29_B                0x1FFFFFFF
#define MSK_30_B                0x3FFFFFFF
#define MSK_31_B                0x7FFFFFFF

/* 位数定义 */
#define NUM_1_BITS 1
#define NUM_2_BITS 2
#define NUM_3_BITS 3
#define NUM_4_BITS 4
#define NUM_5_BITS 5
#define NUM_6_BITS 6
#define NUM_7_BITS 7
#define NUM_8_BITS 8
#define NUM_9_BITS 9
#define NUM_10_BITS 10
#define NUM_11_BITS 11
#define NUM_12_BITS 12
#define NUM_13_BITS 13
#define NUM_14_BITS 14
#define NUM_15_BITS 15
#define NUM_16_BITS 16
#define NUM_17_BITS 17
#define NUM_18_BITS 18
#define NUM_19_BITS 19
#define NUM_20_BITS 20
#define NUM_21_BITS 21
#define NUM_22_BITS 22
#define NUM_23_BITS 23
#define NUM_24_BITS 24
#define NUM_25_BITS 25
#define NUM_26_BITS 26
#define NUM_27_BITS 27
#define NUM_28_BITS 28
#define NUM_29_BITS 29
#define NUM_30_BITS 30
#define NUM_31_BITS 31
#define NUM_32_BITS 32

/* 位偏移定义 */
#define BIT_OFFSET_0 0
#define BIT_OFFSET_1 1
#define BIT_OFFSET_2 2
#define BIT_OFFSET_3 3
#define BIT_OFFSET_4 4
#define BIT_OFFSET_5 5
#define BIT_OFFSET_6 6
#define BIT_OFFSET_7 7
#define BIT_OFFSET_8 8
#define BIT_OFFSET_9 9
#define BIT_OFFSET_10 10
#define BIT_OFFSET_11 11
#define BIT_OFFSET_12 12
#define BIT_OFFSET_13 13
#define BIT_OFFSET_14 14
#define BIT_OFFSET_15 15
#define BIT_OFFSET_16 16
#define BIT_OFFSET_17 17
#define BIT_OFFSET_18 18
#define BIT_OFFSET_19 19
#define BIT_OFFSET_20 20
#define BIT_OFFSET_21 21
#define BIT_OFFSET_22 22
#define BIT_OFFSET_23 23
#define BIT_OFFSET_24 24
#define BIT_OFFSET_25 25
#define BIT_OFFSET_26 26
#define BIT_OFFSET_27 27
#define BIT_OFFSET_28 28
#define BIT_OFFSET_29 29
#define BIT_OFFSET_30 30
#define BIT_OFFSET_31 31

#ifndef __INLINE
#ifdef INLINE_TO_FORCEINLINE
#define __INLINE    __inline__ __attribute__((always_inline))
#else
#define __INLINE    __inline
#endif
#endif

#ifdef UT_TEST
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif
#endif

#endif

