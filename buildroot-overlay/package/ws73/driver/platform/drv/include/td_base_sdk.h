/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: types header.
 * Author: CompanyName
 * Create: 2022-11-01
 * This file should be changed only infrequently and with great care.
 */

#ifndef __TD_BASE_SDK_H__
#define __TD_BASE_SDK_H__

#include "td_type.h"

#define TD_CONST    		const
#define TD_REG      		register

#define TD_U64_MAX          0xFFFFFFFFFFFFFFFFUL
#define TD_U32_MAX          0xFFFFFFFF
#define TD_U16_MAX          0xFFFF
#define TD_U8_MAX           0xFF
#define TD_U4_MAX           0x0f
#define TD_S32_MAX          (~(~0 << (32 - 1)))
#define TD_S16_MAX  		0x7FFF
#define TD_S8_MAX   		0x7F
#define TD_S32_MIN  		-0x80000000
#define TD_S16_MIN  		-0x8000
#define TD_S8_MIN   		-0x80

#define TD_U32_BITS         32
#define TD_S32_BITS         32
#define TD_U24_BITS         24
#define TD_U16_BITS         16
#define TD_U8_BITS          8
#define TD_U4_BITS          4

#define TD_U8_BIT_INDEX_MAX 7

#define TD_PUBLIC    		extern

#if defined(__BUILD_IN_ROM__) || defined(CONFIG_FEATURE_UT)
#define TD_PRV
#else
#define TD_PRV static
#endif

#define TD_INLINE 			inline
#define TD_API
#define TD_EXTERN 			extern
#ifdef __cplusplus
# define TD_CPP_START    extern "C" {
# define TD_CPP_END      }
#else
# define TD_CPP_START
# define TD_CPP_END
#endif

#ifdef NDEBUG
#define TD_ASSERT(ignore)   ((void)0)
#else
#define TD_ASSERT(x)        ((void)0)
#endif

#define TD_START_HEADER     TD_CPP_START
#define TD_END_HEADER       TD_CPP_END

#undef TD_OUT
#undef TD_IN
#undef TD_INOUT
#define TD_OUT
#define TD_IN
#define TD_INOUT

#ifndef CACHE_ALIGNED_SIZE
#define CACHE_ALIGNED_SIZE        32
#endif

#define TD_ALWAYS_INLINE __attribute__((always_inline)) inline
#define TD_ALWAYS_STAIC_INLINE __attribute__((always_inline)) static inline

#define uapi_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))

#define uapi_unused(var) \
    do { \
        (void)(var); \
    } while (0)

#define uapi_align_4(x)        	 ((unsigned int)((x) + 0x3) & (~0x3)) /* 构造4字节地址对齐 */
#define uapi_is_align_u32(x)   	 (!((x) & 3))                         /* 判断是否为4字节对齐 */
#define uapi_is_unalign_u32(x)   ((x) & 3)                            /* 判断是否为4字节对齐 */

#if defined(HAVE_PCLINT_CHECK)
#define uapi_fieldoffset(s, m)   (0) /* 结构成员偏移 */
#else
#define uapi_fieldoffset(s, m)   ((td_u32) & (((s*)0)->m)) /* 结构成员偏移 */
#endif

#define TD_CHAR_CR             	 '\r' /* 0x0D */
#define TD_CHAR_LF               '\n' /* 0x0A */
#define uapi_tolower(x)          ((x) | 0x20)  /* Works only for digits and letters, but small and fast */

#define uapi_makeu16(a, b)       ((td_u16)(((td_u8)(a)) | ((td_u16)((td_u8)(b))) << 8))
#define uapi_makeu32(a, b)       ((td_u32)(((td_u16)(a)) | ((td_u32)((td_u16)(b))) << 16))
#define uapi_makeu64(a, b)       ((td_u64)(((td_u32)(a)) | ((td_u64)((td_u32)(b))) <<32))
#define uapi_joinu32(a, b, c, d) ((a) | ((td_u32)(b) << 8) | ((td_u32)(c) << 16) | ((td_u32)(d) << 24))

#define uapi_hiu32(l)            ((td_u32)(((td_u64)(l) >> 32) & 0xFFFFFFFF))
#define uapi_lou32(l)            ((td_u32)(l))

#define uapi_hiu16(l)            ((td_u16)(((td_u32)(l) >> 16) & 0xFFFF))
#define uapi_lou16(l)            ((td_u16)(l))
#define uapi_hiu8(l)             ((td_u8)(((td_u16)(l) >> 8) & 0xFF))
#define uapi_lou8(l)             ((td_u8)(l))

#define uapi_max(a, b)                           (((a) > (b)) ? (a) : (b))
#define uapi_min(a, b)                           (((a) < (b)) ? (a) : (b))
#define uapi_sub(a, b)                           (((a) > (b)) ? ((a) - (b)) : 0)
#define uapi_abs(a)                              (((a) > 0) ? (a) : (- (a)))
#define uapi_abs_sub(a, b)                       (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define uapi_byte_align(value, align)            (((value) + (align) - 1) & (~((align) -1)))
#define uapi_is_byte_align(value, align)         (((td_u32)(value) & ((align) - 1))== 0)

#define uapi_set_bit_i(val, n)                   ((val) |= (1 << (n)))
#define uapi_clr_bit_i(val, n)                   ((val) &= ~(1 << (n)))
#define uapi_is_bit_set_i(val, n)                ((val) & (1 << (n)))
#define uapi_is_bit_clr_i(val, n)                (~((val) & (1 << (n))))
#define uapi_switch_bit_i(val, n)                ((val) ^= (1 << (n)))
#define uapi_get_bit_i(val, n)                   (((val) >> (n)) & 1)

#define td_u8_bit_val(b7, b6, b5, b4, b3, b2, b1, b0) \
    (((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))

#define td_u16_bit_val(b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0) \
    (td_u16)(((b12) << 12) | ((b11) << 11) | ((b10) << 10) | ((b9) << 9) | ((b8) << 8) | ((b7) << 7) | \
    ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))

#define uapi_set_u32_ptr_val(ptr, offset, val)  (*((td_u32*)(((td_u8*)(ptr)) + (offset))) = (val))
#define uapi_get_u32_ptr_val(ptr, offset)      *((td_u32*)(((td_u8*)(ptr)) + (offset)))

#ifndef bit
#define bit(x)                         (1UL << (x))
#endif
#ifndef BIT0
#define BIT31                          ((td_u32)(1UL << 31))
#define BIT30                          ((td_u32)(1 << 30))
#define BIT29                          ((td_u32)(1 << 29))
#define BIT28                          ((td_u32)(1 << 28))
#define BIT27                          ((td_u32)(1 << 27))
#define BIT26                          ((td_u32)(1 << 26))
#define BIT25                          ((td_u32)(1 << 25))
#define BIT24                          ((td_u32)(1 << 24))
#define BIT23                          ((td_u32)(1 << 23))
#define BIT22                          ((td_u32)(1 << 22))
#define BIT21                          ((td_u32)(1 << 21))
#define BIT20                          ((td_u32)(1 << 20))
#define BIT19                          ((td_u32)(1 << 19))
#define BIT18                          ((td_u32)(1 << 18))
#define BIT17                          ((td_u32)(1 << 17))
#define BIT16                          ((td_u32)(1 << 16))
#define BIT15                          ((td_u32)(1 << 15))
#define BIT14                          ((td_u32)(1 << 14))
#define BIT13                          ((td_u32)(1 << 13))
#define BIT12                          ((td_u32)(1 << 12))
#define BIT11                          ((td_u32)(1 << 11))
#define BIT10                          ((td_u32)(1 << 10))
#define BIT9                           ((td_u32)(1 << 9))
#define BIT8                           ((td_u32)(1 << 8))
#define BIT7                           ((td_u32)(1 << 7))
#define BIT6                           ((td_u32)(1 << 6))
#define BIT5                           ((td_u32)(1 << 5))
#define BIT4                           ((td_u32)(1 << 4))
#define BIT3                           ((td_u32)(1 << 3))
#define BIT2                           ((td_u32)(1 << 2))
#define BIT1                           ((td_u32)(1 << 1))
#define BIT0                           ((td_u32)(1 << 0))
#endif

#define HALFWORD_BIT_WIDTH             16

#define BYTE_WIDTH                 	   1
#define HALF_WIDTH                     2
#define WORD_WIDTH                     4

#define uapi_reg_write(addr, val)              (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define uapi_reg_read(addr, val)               ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define uapi_reg_write32(addr, val)            (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define uapi_reg_read32(addr, val)             ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define uapi_reg_read_val32(addr)              (*(volatile unsigned int*)(uintptr_t)(addr))
#define uapi_reg_setbitmsk(addr, msk)          ((uapi_reg_read_val32(addr)) |= (msk))
#define uapi_reg_clrbitmsk(addr, msk)          ((uapi_reg_read_val32(addr)) &= ~(msk))
#define uapi_reg_clrbit(addr, pos)             ((uapi_reg_read_val32(addr)) &= ~((unsigned int)(1) << (pos)))
#define uapi_reg_setbit(addr, pos)             ((uapi_reg_read_val32(addr)) |= ((unsigned int)(1) << (pos)))
#define uapi_reg_clrbits(addr, pos, bits)      (uapi_reg_read_val32(addr) &= ~((((unsigned int)1 << (bits)) - 1) << (pos)))
#define uapi_reg_setbits(addr, pos, bits, val) (uapi_reg_read_val32(addr) =           \
    (uapi_reg_read_val32(addr) & (~((((unsigned int)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned int)((val) & (((unsigned int)1 << (bits)) - 1)) << (pos)))
#define uapi_reg_getbits(addr, pos, bits)      ((uapi_reg_read_val32(addr) >> (pos)) & (((unsigned int)1 << (bits)) - 1))

#define uapi_reg_write16(addr, val)        (*(volatile unsigned short *)(uintptr_t)(addr) = (val))
#define uapi_reg_read16(addr, val)         ((val) = *(volatile unsigned short *)(uintptr_t)(addr))
#define uapi_reg_read_val16(addr)          (*(volatile unsigned short*)(uintptr_t)(addr))
#define uapi_reg_clrbit16(addr, pos)       ((uapi_reg_read_val16(addr)) &= ~((unsigned short)(1) << (pos)))
#define uapi_reg_setbit16(addr, pos)       ((uapi_reg_read_val16(addr)) |= ((unsigned short)(1) << (pos)))
#define uapi_reg_clrbits16(addr, pos, bits) (uapi_reg_read_val16(addr) &= ~((((unsigned short)1 << (bits)) - 1) << (pos)))
#define uapi_reg_setbits16(addr, pos, bits, val) (uapi_reg_read_val16(addr) =           \
    (uapi_reg_read_val16(addr) & (~((((unsigned short)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned short)((val) & (((unsigned short)1 << (bits)) - 1)) << (pos)))
#define uapi_reg_getbits16(addr, pos, bits) ((uapi_reg_read_val16(addr) >> (pos)) & (((unsigned short)1 << (bits)) - 1))

#define uapi_reg_write8(addr, val)        (*(volatile unsigned char *)(uintptr_t)(addr) = (val))
#define uapi_reg_read8(addr, val)         ((val) = *(volatile unsigned char *)(uintptr_t)(addr))
#define uapi_reg_read_val8(addr)          (*(volatile unsigned char*)(uintptr_t)(addr))
#define uapi_reg_clrbit8(addr, pos)       ((uapi_reg_read_val8(addr)) &= ~((unsigned char)(1) << (pos)))
#define uapi_reg_setbit8(addr, pos)       ((uapi_reg_read_val8(addr)) |= ((unsigned char)(1) << (pos)))
#define uapi_reg_clrbits8(addr, pos, bits) (uapi_reg_read_val8(addr) &= ~((((unsigned char)1 << (bits)) - 1) << (pos)))
#define uapi_reg_setbits8(addr, pos, bits, val) (uapi_reg_read_val8(addr) =           \
    (uapi_reg_read_val8(addr) & (~((((unsigned char)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned char)((val) & (((unsigned char)1 << (bits)) - 1)) << (pos)))
#define uapi_reg_getbits8(addr, pos, bits) ((uapi_reg_read_val8(addr) >> (pos)) & (((unsigned char)1 << (bits)) - 1))
#ifndef align_next
#define align_next(val, a)          ((((val) + ((a)-1)) & (~((a)-1))))
#define align_length(val, a)        align_next(val, a)
#endif
#define BITS_PER_BYTE               8
#define HEXADECIMAL                 16
#define DECIMAL                     10

#define osal_padding(x, size) (((x) + (size)-1) & (~((size)-1)))

#define osal_round_down(value, boundary) ((value) & (~((boundary)-1)))
#define osal_round_up(value, boundary) ((((value)-1) | ((boundary)-1)) + 1)

/* Is val aligned to "align" ("align" must be power of 2) */
#ifndef IS_ALIGNED
#define osal_is_aligned(val, align) (((osal_u32)(val) & ((align) - 1)) == 0)
#else
#define osal_is_aligned IS_ALIGNED
#endif


#define SZ_1KB 						1024
#define SZ_1MB 						(SZ_1KB * SZ_1KB)
#define SZ_4KB 						4096
#define TD_SYS_WAIT_FOREVER         0xFFFFFFFF

typedef td_void  (*td_void_callback)(td_void);
typedef struct {
    td_mem_handle_t mem_handle;
    td_mem_size_t   addr_offset;
} ext_mem_handle;

typedef struct {
    td_s32 x;
    td_s32 y;
    td_u32 width;
    td_u32 height;
} ext_rect;

#ifdef HAVE_PCLINT_CHECK
#define uapi_likely(x)    (x)
#define uapi_unlikely(x)  (x)
#else
#define uapi_likely(x) __builtin_expect(!!(x), 1)
#define uapi_unlikely(x) __builtin_expect(!!(x), 0)
#endif

#endif /* __TD_BASE_H__ */

