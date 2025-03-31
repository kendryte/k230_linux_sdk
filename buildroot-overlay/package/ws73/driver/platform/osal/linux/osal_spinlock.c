/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include "soc_osal.h"
#include "osal_inner.h"

int osal_spin_lock_init(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    p = (spinlock_t *)kmalloc(sizeof(spinlock_t), GFP_KERNEL); // 8 is malloc size
    if (p == NULL) {
        osal_log("kmalloc error!\n");
        return OSAL_FAILURE;
    }
    spin_lock_init(p);
    lock->lock = p;
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_spin_lock_init);

void osal_spin_lock(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    spin_lock(p);
}
EXPORT_SYMBOL(osal_spin_lock);

void osal_spin_lock_bh(osal_spinlock *lock)
{
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }
    spin_lock_bh(lock->lock);
}
EXPORT_SYMBOL(osal_spin_lock_bh);

void osal_spin_unlock_bh(osal_spinlock *lock)
{
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }
    spin_unlock_bh(lock->lock);
}
EXPORT_SYMBOL(osal_spin_unlock_bh);

int osal_spin_trylock(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid!\n");
        return -1;
    }

    p = (spinlock_t *)(lock->lock);

    return spin_trylock(p);
}
EXPORT_SYMBOL(osal_spin_trylock);

void osal_spin_unlock(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    spin_unlock(p);
}
EXPORT_SYMBOL(osal_spin_unlock);

void osal_spin_lock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
    spinlock_t *p = NULL;
    unsigned long f;

    if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    spin_lock_irqsave(p, f);

    *flags = f;
}
EXPORT_SYMBOL(osal_spin_lock_irqsave);

void osal_spin_unlock_irqrestore(osal_spinlock *lock, unsigned long *flags)
{
    spinlock_t *p = NULL;

    if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    spin_unlock_irqrestore(p, *flags);
}
EXPORT_SYMBOL(osal_spin_unlock_irqrestore);

void osal_spin_lock_destroy(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    kfree(p);
    lock->lock = NULL;
}
EXPORT_SYMBOL(osal_spin_lock_destroy);

int osal_spin_trylock_irq(osal_spinlock *lock)
{
    spinlock_t *p = NULL;

    if (lock == NULL  || lock->lock == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    p = (spinlock_t *)(lock->lock);

    return spin_trylock_irq(p);
}
EXPORT_SYMBOL(osal_spin_trylock_irq);

void osal_spin_trylock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
    spinlock_t *p = NULL;
    unsigned long f;

    if ((lock == NULL) || (flags == NULL  || lock->lock == NULL)) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return;
    }

    p = (spinlock_t *)(lock->lock);
    spin_trylock_irqsave(p, f);

    *flags = f;
}
EXPORT_SYMBOL(osal_spin_trylock_irqsave);
