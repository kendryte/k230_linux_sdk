/*
 * Copyright (c) CompanyNameMagicTag. 2019-2020. All rights reserved.
 * Description: types header.
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
typedef osal_u32                  osal_handle;
typedef osal_u8                   osal_bool;
typedef void                    osal_void;
typedef void*                   osal_pvoid;

typedef osal_u8                   osal_byte;
typedef osal_byte*                osal_pbyte;

#ifndef __LP64__
typedef osal_u32                  size_t;
typedef unsigned long long      uint64_t;
#endif

#undef ERROR
#define ERROR (-1)

#define OSAL_U8_MAX 0xFF

/* defines */
#undef NULL
#ifndef NULL
#define NULL 0
#endif

#define EXT_CONST               const
#define EXT_REG                 register

#define OSAL_FALSE 0
#define OSAL_TRUE  1

#ifndef NEWLINE
#define NEWLINE                 "\r\n"
#endif


#ifdef __cplusplus
#define OSAL_NULL       0
#else
#define OSAL_NULL    ((void *)0)
#endif

#ifndef CACHE_ALIGNED_SIZE
#define CACHE_ALIGNED_SIZE        32
#endif

#define EXT_ALWAYS_INLINE __attribute__((always_inline)) inline

#define osal_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))

#define osal_unused(var) \
    do { \
        (void)(var); \
    } while (0)

#define osal_is_align_u32(x)   (!((x) & 3))                         /* 判断是否为4字节对齐 */
#define osal_is_unalign_u32(x) ((x) & 3)                            /* 判断是否为4字节对齐 */

#define hi_tolower(x)          ((x) | 0x20)  /* Works only for digits and letters, but small and fast */

#define osal_makeu32(a, b)       ((osal_u32)(((osal_u16)(a)) | ((osal_u32)((osal_u16)(b))) << 16))

#define osal_hiu16(l)            ((osal_u16)(((osal_u32)(l) >> 16) & 0xFFFF))
#define osal_lou16(l)            ((osal_u16)(l))

#define osal_max(a, b)           (((a) > (b)) ? (a) : (b))
#define osal_min(a, b)           (((a) < (b)) ? (a) : (b))
#define osal_byte_align(value, align)            (((value) + (align) - 1) & (~((align) -1)))

#ifndef bit
#define bit(x)                         (1UL << (x))
#endif

#define HALFWORD_BIT_WIDTH              16

#define BYTE_WIDTH                  1
#define HALF_WIDTH                  2
#define WORD_WIDTH                  4

#define osal_reg_write(addr, val)              (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define osal_reg_read(addr, val)               ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define osal_reg_write32(addr, val)            (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define osal_reg_read32(addr, val)             ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define osal_reg_read_val32(addr)              (*(volatile unsigned int*)(uintptr_t)(addr))
#define osal_reg_setbitmsk(addr, msk)          ((osal_reg_read_val32(addr)) |= (msk))
#define osal_reg_clrbitmsk(addr, msk)          ((osal_reg_read_val32(addr)) &= ~(msk))
#define osal_reg_clrbit(addr, pos)             ((osal_reg_read_val32(addr)) &= ~((unsigned int)(1) << (pos)))
#define osal_reg_setbit(addr, pos)             ((osal_reg_read_val32(addr)) |= ((unsigned int)(1) << (pos)))
#define osal_reg_setbits(addr, pos, bits, val) (osal_reg_read_val32(addr) =           \
    (osal_reg_read_val32(addr) & (~((((unsigned int)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned int)((val) & (((unsigned int)1 << (bits)) - 1)) << (pos)))
#define osal_reg_write16(addr, val)      (*(volatile unsigned short *)(uintptr_t)(addr) = (val))
#define osal_reg_read16(addr, val)       ((val) = *(volatile unsigned short *)(uintptr_t)(addr))
#define osal_reg_read_val16(addr)        (*(volatile unsigned short*)(uintptr_t)(addr))
#define osal_reg_clrbit16(addr, pos)       ((osal_reg_read_val16(addr)) &= ~((unsigned short)(1) << (pos)))
#define osal_reg_setbit16(addr, pos)       ((osal_reg_read_val16(addr)) |= ((unsigned short)(1) << (pos)))
#define osal_reg_setbits16(addr, pos, bits, val) (osal_reg_read_val16(addr) =           \
    (osal_reg_read_val16(addr) & (~((((unsigned short)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned short)((val) & (((unsigned short)1 << (bits)) - 1)) << (pos)))
#define osal_reg_getbits16(addr, pos, bits) ((osal_reg_read_val16(addr) >> (pos)) & (((unsigned short)1 << (bits)) - 1))

#define align_next(val, a)          ((((val) + ((a)-1)) & (~((a)-1))))
#define align_length(val, a)        align_next(val, a)

#ifdef HAVE_PCLINT_CHECK
#define osal_likely(x)    (x)
#define osal_unlikely(x)  (x)
#else
#define osal_likely(x) __builtin_expect(!!(x), 1)
#define osal_unlikely(x) __builtin_expect(!!(x), 0)
#endif

#endif /* __OSAL_TYPES_H__ */

