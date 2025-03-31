/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: dfx adapt layer
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_ADAPT_LAYER_H__
#define __DFX_ADAPT_LAYER_H__
#include "td_type.h"
#include "soc_errno.h"
#include "dfx_feature_config.h"
#include "soc_osal.h"
#include "soc_log.h"
#include "dfx_event.h"
#if defined(CONFIG_DFX_ADAPT_GET_CUR_SEC_TYPE) && defined(DFX_ADAPT_GET_CUR_SEC_TYPE_TIME) && \
    (CONFIG_DFX_ADAPT_GET_CUR_SEC_TYPE == DFX_ADAPT_GET_CUR_SEC_TYPE_TIME)
#include "time.h"
#endif
#if defined(CONFIG_WS73_MCORE)
#include "malloc.h"
#endif

#define DFX_MALLOC_ID_DEFAULT_VAL 0
#define DFX_INTLOCK_ID_DEFAULT_VAL 0

static inline td_u32 dfx_int_lock(td_void)
{
#if defined(__KERNEL__)
    return 0;
#else
#if defined(CONFIG_CGRA_CORE)
    return uapi_int_lock();
#else
    return osal_irq_lock();
#endif
#endif
}

static inline td_void dfx_int_restore(td_u32 state)
{
#if defined(__KERNEL__)
    uapi_unused(state);
#else
#if defined(CONFIG_CGRA_CORE)
    uapi_int_restore(state);
#else
    osal_irq_restore(state);
#endif
#endif
}

static inline td_void *dfx_malloc(td_u32 id, td_u32 size)
{
    uapi_unused(id);
#if defined(CONFIG_CGRA_CORE)
    return malloc(size);
#elif defined(CONFIG_WS73_MCORE)
    return malloc(size);
#else
    return osal_vmalloc(size);
#endif
}

static inline void dfx_free(td_u32 id, void *addr)
{
    uapi_unused(id);
#if defined(CONFIG_CGRA_CORE)
    free(addr);
#else
    osal_vfree(addr);
#endif
}

static inline td_u32 dfx_get_cur_second(td_void)
{
#if defined(CONFIG_DFX_ADAPT_GET_CUR_SEC_TYPE) && defined(DFX_ADAPT_GET_CUR_SEC_TYPE_TIME) && \
    (CONFIG_DFX_ADAPT_GET_CUR_SEC_TYPE == DFX_ADAPT_GET_CUR_SEC_TYPE_TIME)
    time_t sec = time(TD_NULL);
    return (td_u32)sec;
#else
    return 0;
#endif
}

#ifndef dfx_basic_printf
#define dfx_basic_printf(fmt, ...)
#endif

#endif
