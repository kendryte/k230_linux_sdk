/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Header file for oal_plat_type.h.
 */

#ifndef __OAL_PLAT_TYPE_H__
#define __OAL_PLAT_TYPE_H__

#include "osal_adapt.h"
#include "td_base.h"
/*****************************************************************************
  1 Include other Head file
*****************************************************************************/

/*****************************************************************************
  2 Define macro
*****************************************************************************/
#define STATIC  static

#ifndef INLINE__
#ifdef INLINE_TO_FORCEINLINE
#define INLINE__    __inline__ __attribute__((always_inline))
#else
#define INLINE__    __inline
#endif
#endif

#ifdef osal_likely
#undef osal_likely
#endif
#ifdef osal_unlikely
#undef osal_unlikely
#endif
#ifdef HAVE_PCLINT_CHECK
#define osal_likely(x)    (x)
#define osal_unlikely(x)  (x)
#else
#define osal_likely(x) __builtin_expect(!!(x), 1)
#define osal_unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define OAL_LIKELY(_expr)       uapi_likely(_expr)
#define OAL_UNLIKELY(_expr)     uapi_unlikely(_expr)

#define osal_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))

#define osal_tolower(x)          ((x) | 0x20)  /* Works only for digits and letters, but small and fast */
#define osal_makeu64(a, b)       ((osal_u64)(((osal_u32)(a)) | ((osal_u64)((osal_u32)(b))) << 32))

#ifndef NEWLINE
#define NEWLINE                 "\r\n"
#endif

#define osal_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))

#ifndef OSAL_WRITE_BITS /* 与oal_plat_type.h重定义，先屏蔽后整改 */
#define OSAL_WRITE_BITS
static INLINE__ osal_void osal_write_bits(osal_u32 *data, osal_u32 val, osal_u32 bits, osal_u32 pos)
{
    (*data) &= ~((((osal_u32)1 << bits) - 1) << pos);
    (*data) |= ((val & (((osal_u32)1 << bits) - 1)) << pos);
}
#endif
#define osal_abs_of_diff(_l_a, _l_b) (((_l_a) > (_l_b)) ? ((_l_a) - (_l_b)) : ((_l_b) - (_l_a)))

#define osal_bit(nr) (1UL << (nr))

#endif /* __OAL_PLAT_TYPE_H */
