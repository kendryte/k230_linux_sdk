/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include "soc_osal.h"
#include "osal_inner.h"

int osal_mutex_init(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex != NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = kmalloc(sizeof(struct mutex), GFP_KERNEL);
    if (p == NULL) {
        osal_log("kmalloc error!\n");
        return OSAL_FAILURE;
    }

    mutex_init(p);
    mutex->mutex = p;

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_mutex_init);

int osal_mutex_lock(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = (struct mutex *)(mutex->mutex);
    mutex_lock(p);

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_mutex_lock);

int osal_mutex_lock_interruptible(osal_mutex *mutex)
{
    struct mutex *p = NULL;
    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = (struct mutex *)(mutex->mutex);

    return mutex_lock_interruptible(p);
}
EXPORT_SYMBOL(osal_mutex_lock_interruptible);

int osal_mutex_trylock(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid!\n");
        return FALSE;
    }

    p = (struct mutex *)(mutex->mutex);

    return mutex_trylock(p);
}
EXPORT_SYMBOL(osal_mutex_trylock);

void osal_mutex_unlock(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (struct mutex *)(mutex->mutex);

    mutex_unlock(p);
}
EXPORT_SYMBOL(osal_mutex_unlock);

void osal_mutex_destroy(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }
    p = (struct mutex *)(mutex->mutex);
    kfree(p);
    mutex->mutex = NULL;
}
EXPORT_SYMBOL(osal_mutex_destroy);

int osal_mutex_is_locked(osal_mutex *mutex)
{
    struct mutex *p = NULL;

    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("mutex is null!\n");
        return OSAL_FAILURE;
    }

    p = (struct mutex *)(mutex->mutex);
    return mutex_is_locked(p);
}
EXPORT_SYMBOL(osal_mutex_is_locked);