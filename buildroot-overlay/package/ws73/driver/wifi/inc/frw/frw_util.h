/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: oal util interface
 * Author: Huanghe
 * Create: 2021-08-06
 */
#ifndef __FRW_UTIL_H__
#define __FRW_UTIL_H__

#include "oal_plat_type.h"
#include "osal_adapt.h"
#include "securec.h"
#include "frw_util_common.h"

#undef osal_max
#define osal_max(a, b)           (((a) > (b)) ? (a) : (b))

#undef osal_min
#define osal_min(a, b)           (((a) < (b)) ? (a) : (b))

#define EXT_TIME_US_MAX_LEN_U64 (0xFFFFFFFFFFFFFFFF)

#define frw_debug(fmt, arg...) wifi_printf("[FRW] "fmt, ##arg)

static INLINE__ osal_u64 osal_get_timeofday_us(osal_void)
{
    osal_timeval tv;
    osal_gettimeofday(&tv);
    return (((osal_u64)tv.tv_sec) * 1000000 + tv.tv_usec);
}

/* 获取毫秒级时间戳  */
static INLINE__ osal_u64 osal_get_time_stamp_ms(osal_void)
{
    return (osal_get_timeofday_us() >> 10); /* 用1024换算ms */
}

/* 获取31.25微秒级时间戳 */
static INLINE__ osal_u64 osal_get_time_stamp_ts(osal_void)
{
    return osal_get_timeofday_us();
}

static INLINE__ osal_u64 osal_get_runtime_u64(osal_u64 _start, osal_u64 _end)
{
    return ((_start > _end) ? (EXT_TIME_US_MAX_LEN_U64 - _start + _end + 1) : (_end - _start));
}

/* 需要osal层统一提供mb()函数 */
static INLINE__ void osal_bus_idle(void)
{
#ifndef LOSCFG_PLATFORM_BSP_RISCV_PLIC
#ifndef BUILD_UT
    __asm ("dsb\n");
#endif
#endif
}

static INLINE__ osal_u32 osal_get_trng_bytes(osal_u8 *data, osal_u32 size)
{
    osal_u32 ret = 0;
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LITEOS)
#if !defined(_PRE_WLAN_FEATURE_WS73) || defined(_PRE_LITEOS_SDK_) // 等待CFBB适配
    // not implemented
#else
    ret = uapi_drv_cipher_trng_get_random_bytes(data, size);
#endif
#endif
    unref_param(data);
    unref_param(size);
    return ret;
}
#endif /* end of frw_util.h */