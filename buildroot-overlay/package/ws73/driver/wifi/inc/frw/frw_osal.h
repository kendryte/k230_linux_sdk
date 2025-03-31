/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wifi dmac os adapt layer
 * Author: Huanghe
 * Create: 2021-08-05
 */

#ifndef __FRW_OSAL_H__
#define __FRW_OSAL_H__

#include "osal_types.h"
#include "oal_types.h"
#include "osal_adapt.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include<linux/interrupt.h>
#endif
/* 禁止所有中断 */
static inline osal_u32 frw_osal_irq_lock(osal_void)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    local_irq_disable();
    return OAL_SUCC;
#else
    return osal_adapt_irq_lock();
#endif
}

/* 恢复中断 */
static inline osal_void frw_osal_irq_restore(osal_u32 irq_status)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    unref_param(irq_status);
    local_irq_enable();
    return;
#else
    return osal_adapt_irq_restore(irq_status);
#endif
}

#endif // endif __FRW_OSAL_H__

