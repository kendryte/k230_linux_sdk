/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: dfx timer
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_TIMER_H__
#define __DFX_TIMER_H__
#include "td_type.h"
#include "soc_errno.h"
#include "dfx_feature_config.h"
#include "soc_osal.h"

typedef osal_timer dfx_timer;
typedef void (*dfx_timer_handler)(uintptr_t data);
static inline ext_errno dfx_timer_init(dfx_timer *timer, dfx_timer_handler handler, uintptr_t data, td_u32 ms)
{
    timer->handler = handler;
    timer->data = data;
    timer->interval = ms;
    return (ext_errno)osal_timer_init(timer);
}

static inline ext_errno dfx_timer_start(dfx_timer *timer, td_u32 ms)
{
    return (ext_errno)osal_timer_mod(timer, ms);
}
static inline ext_errno dfx_timer_stop(dfx_timer *timer)
{
    return (ext_errno)osal_timer_stop(timer);
}

static inline ext_errno dfx_timer_destroy(dfx_timer *timer)
{
    return (ext_errno)osal_timer_destroy(timer);
}

#endif