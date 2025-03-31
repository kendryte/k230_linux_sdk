/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include "soc_osal.h"
#include "securec.h"
#include "osal_inner.h"
int osal_atomic_init(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return OSAL_FAILURE;
    }

    atomic->counter = 0;
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_atomic_init);

void osal_atomic_destroy(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

    atomic->counter = 0;
}
EXPORT_SYMBOL(osal_atomic_destroy);

int osal_atomic_read(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    return atomic_read((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_read);

void osal_atomic_set(osal_atomic *atomic, int i)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    atomic_set((atomic_t *)atomic, i);
}
EXPORT_SYMBOL(osal_atomic_set);

int osal_atomic_inc_return(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    return atomic_inc_return((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_inc_return);

int osal_atomic_dec_return(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    return atomic_dec_return((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_dec_return);

void osal_atomic_inc(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return;
    }

    atomic_inc((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_inc);

void osal_atomic_dec(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return;
    }

    atomic_dec((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_dec);

void osal_atomic_add(osal_atomic *atomic, int val)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return;
    }

    atomic_add(val, (atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_add);

int osal_atomic_dec_and_test(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    return atomic_dec_and_test((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_dec_and_test);
int osal_atomic_inc_and_test(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    return atomic_inc_and_test((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_inc_and_test);

int osal_atomic_inc_not_zero(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    return atomic_inc_not_zero((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_inc_not_zero);
