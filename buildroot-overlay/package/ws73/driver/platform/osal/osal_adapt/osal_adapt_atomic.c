/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: osal adapt atomic file
 */
#ifdef _OSAL_LITEOS_SDK_
#include <asm/atomic.h>
#endif
#include "osal_adapt.h"

int osal_adapt_atomic_init(osal_atomic *atomic)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        return OSAL_FAILURE;
    }
    atomic->counter = 0;
    return OSAL_SUCCESS;
#else /* _OSAL_LITEOS_CFBB_ */
    osal_atomic_set(atomic, 0);
    return OSAL_SUCCESS;
#endif
}

void osal_adapt_atomic_destroy(osal_atomic *atomic)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        return;
    }
    atomic->counter = 0;
#else /* _OSAL_LITEOS_CFBB_ */
    osal_atomic_set(atomic, 0);
#endif
}

int osal_adapt_atomic_read(osal_atomic *atomic)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        osal_printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    /* CFBB 使用 LOS_AtomicRead 接口 adapt LiteOs 系统不支持该接口 */
    return atomic_read((atomic_t *)&(atomic->counter));
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_atomic_read(atomic);
#endif
}

void osal_adapt_atomic_set(osal_atomic *atomic, int val)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        osal_printk("parameter invalid!\n");
        return;
    }
    /* CFBB 使用 LOS_AtomicSet 接口 adapt LiteOs 系统不支持该接口 */
    atomic_set((atomic_t *)&(atomic->counter), val);
#else /* _OSAL_LITEOS_CFBB_ */
    osal_atomic_set(atomic, val);
#endif
}

int osal_adapt_atomic_inc_return(osal_atomic *atomic)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        osal_printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    return LOS_AtomicIncRet((atomic_t *)&(atomic->counter));
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_atomic_inc_return(atomic);
#endif
}

int osal_adapt_atomic_dec_return(osal_atomic *atomic)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        osal_printk("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    return LOS_AtomicDecRet((atomic_t *)&(atomic->counter));
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_atomic_dec_return(atomic);
#endif
}

void osal_adapt_atomic_inc(osal_atomic *atomic)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        osal_printk("parameter invalid!\n");
        return;
    }

    LOS_AtomicInc((atomic_t *)&(atomic->counter));
#else /* _OSAL_LITEOS_CFBB_ */
    osal_atomic_inc(atomic);
#endif
}

void osal_adapt_atomic_dec(osal_atomic *atomic)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        osal_printk("parameter invalid!\n");
        return;
    }

    LOS_AtomicDec((atomic_t *)&(atomic->counter));
#else /* _OSAL_LITEOS_CFBB_ */
    osal_atomic_dec(atomic);
#endif
}
void osal_adapt_atomic_add(osal_atomic *atomic, int val)
{
#ifdef _OSAL_LITEOS_SDK_
    if (atomic == NULL) {
        osal_printk("parameter invalid!\n");
        return;
    }

    LOS_AtomicAdd((atomic_t *)&(atomic->counter), val);
#else /* _OSAL_LITEOS_CFBB_ */
    osal_atomic_add(atomic, val);
#endif
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(osal_adapt_atomic_init);
EXPORT_SYMBOL(osal_adapt_atomic_destroy);
EXPORT_SYMBOL(osal_adapt_atomic_read);
EXPORT_SYMBOL(osal_adapt_atomic_set);
EXPORT_SYMBOL(osal_adapt_atomic_inc_return);
EXPORT_SYMBOL(osal_adapt_atomic_dec_return);
EXPORT_SYMBOL(osal_adapt_atomic_inc);
EXPORT_SYMBOL(osal_adapt_atomic_dec);
EXPORT_SYMBOL(osal_adapt_atomic_add);
#endif