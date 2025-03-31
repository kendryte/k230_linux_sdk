/*
 * Copyright (c) CompanyNameMagicTag 2019-2020. All rights reserved.
 * Description: types header.
 * This file should be changed only infrequently and with great care.
 */

#ifndef __HI_TYPES_H__
#define __HI_TYPES_H__

#include "hi_errno.h"

#if (defined(KERNEL_WITH_OS_ADAPTATION_LAYER) || defined(CONFIG_DSP_CORE) || defined(CONFIG_CGRA_CORE))
#include "stdint.h"
#endif

#define T_DESC(_desc_, _y_) (_y_)

/* 基本数据类型定义 */
typedef unsigned char           hi_uchar;
typedef unsigned char           hi_u8;
typedef unsigned short          hi_u16;
typedef unsigned int            hi_u32;
typedef unsigned long long      hi_u64;
typedef unsigned long           hi_ulong;
typedef char                    hi_char;
typedef signed char             hi_s8;
typedef short                   hi_s16;
typedef int                     hi_s32;
typedef long long               hi_s64;
typedef long                    hi_slong;
typedef float                   hi_float;
typedef double                  hi_double;
typedef unsigned long           hi_size_t;
typedef unsigned long           hi_length_t;
typedef hi_u32                  hi_handle;
typedef hi_u8                   hi_bool;
#if ((!defined(KERNEL_WITH_OS_ADAPTATION_LAYER)) && (!defined(CONFIG_DSP_CORE)) && (!defined(CONFIG_CGRA_CORE)))
typedef unsigned int            uintptr_t;
typedef unsigned long long      uint64_t;
#endif
typedef void                    hi_void;
typedef void*                   hi_pvoid;

typedef hi_u8                   hi_byte;
typedef hi_byte*                hi_pbyte;
#if (!defined(CONFIG_CGRA_CORE))
typedef hi_u32                  size_t;
#endif

typedef unsigned int            hi_uintptr_t;
typedef unsigned int            hi_phys_addr_t;
typedef unsigned int            hi_virt_addr_t;
typedef volatile hi_u32 hi_u32_reg;

#undef ERROR
#define ERROR (-1)

/* defines */
#ifndef NULL
#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif
#endif

#define HI_CONST    const
#define HI_REG      register

#define HI_U64_MAX  0xFFFFFFFFFFFFFFFFUL
#define HI_U32_MAX  0xFFFFFFFF
#define HI_S32_MAX  0x7FFFFFFF
#define HI_U16_MAX  0xFFFF
#define HI_S16_MAX  0x7FFF
#define HI_U8_MAX   0xFF
#define HI_S8_MAX   0x7F
#define HI_U4_MAX   0x0f

#define HI_S32_MIN  (-0x80000000)
#define HI_S16_MIN  (-0x8000)
#define HI_S8_MIN   (-0x80)

#define HI_U32_BITS         32
#define HI_S32_BITS         32
#define HI_U24_BITS         24
#define HI_U16_BITS         16
#define HI_U8_BITS          8
#define HI_U4_BITS          4

#define HI_U8_BIT_INDEX_MAX 7

#define HI_PUBLIC    extern

#if defined(__BUILD_IN_ROM__) || defined(CONFIG_FEATURE_UT)
#define HI_PRV
#define HI_PRVL HI_PRV HI_INLINE
#else
#define HI_PRV static
#define HI_PRVL
#endif

#define HI_INLINE inline
#define HI_API
#define HI_EXTERN extern
#ifdef __cplusplus
# define HI_CPP_START    extern "C" {
# define HI_CPP_END      }
#else
# define HI_CPP_START
# define HI_CPP_END
#endif

#ifdef NDEBUG
#define HI_ASSERT(ignore)   ((void)0)
#else
#define HI_ASSERT(x)        ((void)0)
#endif

#define HI_START_HEADER    HI_CPP_START
#define HI_END_HEADER      HI_CPP_END

#undef HI_OUT
#undef HI_IN
#undef HI_INOUT
#define HI_OUT
#define HI_IN
#define HI_INOUT

#define HI_FALSE 0
#define HI_TRUE  1


#ifdef __cplusplus
#define HI_NULL       0
#else
#define HI_NULL    ((void *)0)
#endif

#ifndef CACHE_ALIGNED_SIZE
#define CACHE_ALIGNED_SIZE        32
#endif

#define HI_ALWAYS_INLINE __attribute__((always_inline)) inline
#define HI_ALWAYS_STAIC_INLINE __attribute__((always_inline)) static inline

#define hi_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))

#define hi_unused(var) \
    do { \
        (void)(var); \
    } while (0)

#define hi_align_4(x)        ((unsigned int)((x) + 0x3) & (~0x3)) /* 构造4字节地址对齐 */
#define hi_is_align_u32(x)   (!((x) & 3))                         /* 判断是否为4字节对齐 */
#define hi_is_unalign_u32(x) ((x) & 3)                            /* 判断是否为4字节对齐 */

#if defined(HAVE_PCLINT_CHECK)
#define hi_fieldoffset(s, m) (0) /* 结构成员偏移 */
#else
#define hi_fieldoffset(s, m) ((hi_u32) & (((s*)0)->m)) /* 结构成员偏移 */
#endif

#define HI_CHAR_CR             '\r' /* 0x0D */
#define HI_CHAR_LF             '\n' /* 0x0A */
#define hi_tolower(x)          ((x) | 0x20)  /* Works only for digits and letters, but small and fast */

#define hi_makeu16(a, b)       ((hi_u16)(((hi_u8)(a)) | ((hi_u16)((hi_u8)(b))) << 8))
#define hi_makeu32(a, b)       ((hi_u32)(((hi_u16)(a)) | ((hi_u32)((hi_u16)(b))) << 16))
#define hi_makeu64(a, b)       ((hi_u64)(((hi_u32)(a)) | ((hi_u64)((hi_u32)(b))) <<32))
#define hi_joinu32(a, b, c, d) ((a) | ((hi_u32)(b) << 8) | ((hi_u32)(c) << 16) | ((hi_u32)(d) << 24))

#define hi_hiu32(l)            ((hi_u32)(((hi_u64)(l) >> 32) & 0xFFFFFFFF))
#define hi_lou32(l)            ((hi_u32)(l))

#define hi_hiu16(l)            ((hi_u16)(((hi_u32)(l) >> 16) & 0xFFFF))
#define hi_lou16(l)            ((hi_u16)(l))
#define hi_hiu8(l)             ((hi_u8)(((hi_u16)(l) >> 8) & 0xFF))
#define hi_lou8(l)             ((hi_u8)(l))

#define hi_max(a, b)           (((a) > (b)) ? (a) : (b))
#define hi_min(a, b)           (((a) < (b)) ? (a) : (b))
#define hi_sub(a, b)           (((a) > (b)) ? ((a) - (b)) : 0)
#define hi_abs(a)              (((a) > 0) ? (a) : (- (a)))
#define hi_abs_sub(a, b)       (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define hi_byte_align(value, align)            (((value) + (align) - 1) & (~((align) -1)))
#define hi_is_byte_align(value, align)         (((hi_u32)(value) & ((align) - 1))== 0)

#define hi_set_bit_i(val, n)                          ((val) |= (1 << (n)))
#define hi_clr_bit_i(val, n)                          ((val) &= ~(1 << (n)))
#define hi_is_bit_set_i(val, n)                       ((val) & (1 << (n)))
#define hi_is_bit_clr_i(val, n)                       (~((val) & (1 << (n))))
#define hi_switch_bit_i(val, n)                       ((val) ^= (1 << (n)))
#define hi_get_bit_i(val, n)                          (((val) >> (n)) & 1)

#define hi_u8_bit_val(b7, b6, b5, b4, b3, b2, b1, b0) \
    (((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))

#define hi_u16_bit_val(b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0) \
    (hi_u16)(((b12) << 12) | ((b11) << 11) | ((b10) << 10) | ((b9) << 9) | ((b8) << 8) | ((b7) << 7) | \
    ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))

#define hi_set_u32_ptr_val(ptr, offset, val)  (*((hi_u32*)(((hi_u8*)(ptr)) + (offset))) = (val))
#define hi_get_u32_ptr_val(ptr, offset)      *((hi_u32*)(((hi_u8*)(ptr)) + (offset)))

#ifndef bit
#define bit(x)                         (1UL << (x))
#endif
#ifndef BIT0
#define BIT31                          ((hi_u32)(1UL << 31))
#define BIT30                          ((hi_u32)(1 << 30))
#define BIT29                          ((hi_u32)(1 << 29))
#define BIT28                          ((hi_u32)(1 << 28))
#define BIT27                          ((hi_u32)(1 << 27))
#define BIT26                          ((hi_u32)(1 << 26))
#define BIT25                          ((hi_u32)(1 << 25))
#define BIT24                          ((hi_u32)(1 << 24))
#define BIT23                          ((hi_u32)(1 << 23))
#define BIT22                          ((hi_u32)(1 << 22))
#define BIT21                          ((hi_u32)(1 << 21))
#define BIT20                          ((hi_u32)(1 << 20))
#define BIT19                          ((hi_u32)(1 << 19))
#define BIT18                          ((hi_u32)(1 << 18))
#define BIT17                          ((hi_u32)(1 << 17))
#define BIT16                          ((hi_u32)(1 << 16))
#define BIT15                          ((hi_u32)(1 << 15))
#define BIT14                          ((hi_u32)(1 << 14))
#define BIT13                          ((hi_u32)(1 << 13))
#define BIT12                          ((hi_u32)(1 << 12))
#define BIT11                          ((hi_u32)(1 << 11))
#define BIT10                          ((hi_u32)(1 << 10))
#define BIT9                           ((hi_u32)(1 << 9))
#define BIT8                           ((hi_u32)(1 << 8))
#define BIT7                           ((hi_u32)(1 << 7))
#define BIT6                           ((hi_u32)(1 << 6))
#define BIT5                           ((hi_u32)(1 << 5))
#define BIT4                           ((hi_u32)(1 << 4))
#define BIT3                           ((hi_u32)(1 << 3))
#define BIT2                           ((hi_u32)(1 << 2))
#define BIT1                           ((hi_u32)(1 << 1))
#define BIT0                           ((hi_u32)(1 << 0))
#endif

#define HALFWORD_BIT_WIDTH              16

#define BYTE_WIDTH                  1
#define HALF_WIDTH                  2
#define WORD_WIDTH                  4

#define hi_reg_write(addr, val)              (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define hi_reg_read(addr, val)               ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define hi_reg_write32(addr, val)            (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define hi_reg_read32(addr, val)             ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define hi_reg_read_val32(addr)              (*(volatile unsigned int*)(uintptr_t)(addr))
#define hi_reg_setbitmsk(addr, msk)          ((hi_reg_read_val32(addr)) |= (msk))
#define hi_reg_clrbitmsk(addr, msk)          ((hi_reg_read_val32(addr)) &= ~(msk))
#define hi_reg_clrbit(addr, pos)             ((hi_reg_read_val32(addr)) &= ~((unsigned int)(1) << (pos)))
#define hi_reg_setbit(addr, pos)             ((hi_reg_read_val32(addr)) |= ((unsigned int)(1) << (pos)))
#define hi_reg_clrbits(addr, pos, bits)      (hi_reg_read_val32(addr) &= ~((((unsigned int)1 << (bits)) - 1) << (pos)))
#define hi_reg_setbits(addr, pos, bits, val) (hi_reg_read_val32(addr) =           \
    (hi_reg_read_val32(addr) & (~((((unsigned int)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned int)((val) & (((unsigned int)1 << (bits)) - 1)) << (pos)))
#define hi_reg_getbits(addr, pos, bits)      ((hi_reg_read_val32(addr) >> (pos)) & (((unsigned int)1 << (bits)) - 1))

#define hi_reg_write16(addr, val)      (*(volatile unsigned short *)(uintptr_t)(addr) = (val))
#define hi_reg_read16(addr, val)       ((val) = *(volatile unsigned short *)(uintptr_t)(addr))
#define hi_reg_read_val16(addr)        (*(volatile unsigned short*)(uintptr_t)(addr))
#define hi_reg_clrbit16(addr, pos)       ((hi_reg_read_val16(addr)) &= ~((unsigned short)(1) << (pos)))
#define hi_reg_setbit16(addr, pos)       ((hi_reg_read_val16(addr)) |= ((unsigned short)(1) << (pos)))
#define hi_reg_clrbits16(addr, pos, bits) (hi_reg_read_val16(addr) &= ~((((unsigned short)1 << (bits)) - 1) << (pos)))
#define hi_reg_setbits16(addr, pos, bits, val) (hi_reg_read_val16(addr) =           \
    (hi_reg_read_val16(addr) & (~((((unsigned short)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned short)((val) & (((unsigned short)1 << (bits)) - 1)) << (pos)))
#define hi_reg_getbits16(addr, pos, bits) ((hi_reg_read_val16(addr) >> (pos)) & (((unsigned short)1 << (bits)) - 1))

#define hi_reg_write8(addr, val)      (*(volatile unsigned char *)(uintptr_t)(addr) = (val))
#define hi_reg_read8(addr, val)       ((val) = *(volatile unsigned char *)(uintptr_t)(addr))
#define hi_reg_read_val8(addr)        (*(volatile unsigned char*)(uintptr_t)(addr))
#define hi_reg_clrbit8(addr, pos)       ((hi_reg_read_val8(addr)) &= ~((unsigned char)(1) << (pos)))
#define hi_reg_setbit8(addr, pos)       ((hi_reg_read_val8(addr)) |= ((unsigned char)(1) << (pos)))
#define hi_reg_clrbits8(addr, pos, bits) (hi_reg_read_val8(addr) &= ~((((unsigned char)1 << (bits)) - 1) << (pos)))
#define hi_reg_setbits8(addr, pos, bits, val) (hi_reg_read_val8(addr) =           \
    (hi_reg_read_val8(addr) & (~((((unsigned char)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned char)((val) & (((unsigned char)1 << (bits)) - 1)) << (pos)))
#define hi_reg_getbits8(addr, pos, bits) ((hi_reg_read_val8(addr) >> (pos)) & (((unsigned char)1 << (bits)) - 1))

#ifndef align_next
#define align_next(val, a)          ((((val) + ((a)-1)) & (~((a)-1))))
#define align_length(val, a)        align_next(val, a)
#endif

#define BITS_PER_BYTE               8
#define HEXADECIMAL                 16
#define DECIMAL                     10

#define SZ_1KB 1024
#define SZ_1MB (SZ_1KB * SZ_1KB)
#define SZ_4KB 4096
#define HI_SYS_WAIT_FOREVER           0xFFFFFFFF

typedef hi_void  (*hi_void_callback)(hi_void);
typedef unsigned long long      hi_mem_size_t;
typedef long long               hi_mem_handle_t;
typedef struct {
    hi_mem_handle_t mem_handle;
    hi_mem_size_t addr_offset;
} hi_mem_handle;

typedef struct {
    hi_s32 x;
    hi_s32 y;
    hi_u32 width;
    hi_u32 height;
} hi_rect;

#ifdef HAVE_PCLINT_CHECK
#define hi_likely(x)    (x)
#define hi_unlikely(x)  (x)
#else
#define hi_likely(x) __builtin_expect(!!(x), 1)
#define hi_unlikely(x) __builtin_expect(!!(x), 0)
#endif

#endif /* __HI_TYPES_H__ */

