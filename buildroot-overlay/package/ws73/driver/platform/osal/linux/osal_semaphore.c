/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include "soc_osal.h"
#include "osal_inner.h"

int osal_sem_init(osal_semaphore *sem, int val)
{
    struct semaphore *p = NULL;

    if (sem == NULL) {
        osal_log("val=%d parameter invalid!\n", val);
        return OSAL_FAILURE;
    }

    p = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (p == NULL) {
        osal_log("kmalloc error!\n");
        return OSAL_FAILURE;
    }

    sema_init(p, val);
    sem->sem = p;

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_sem_init);

int osal_sem_down(osal_semaphore *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = (struct semaphore *)(sem->sem);
    down(p);

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_sem_down);

int osal_sem_down_timeout(osal_semaphore *sem, unsigned int timeout)
{
    struct semaphore *p = NULL;

    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = (struct semaphore *)(sem->sem);

    return down_timeout(p, (long)timeout);
}
EXPORT_SYMBOL(osal_sem_down_timeout);

int osal_sem_down_interruptible(osal_semaphore *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    p = (struct semaphore *)(sem->sem);

    return down_interruptible(p);
}
EXPORT_SYMBOL(osal_sem_down_interruptible);

int osal_sem_trydown(osal_semaphore *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid!\n");
        return -1;
    }

    p = (struct semaphore *)(sem->sem);

    return down_trylock(p);
}
EXPORT_SYMBOL(osal_sem_trydown);

void osal_sem_up(osal_semaphore *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (struct semaphore *)(sem->sem);
    up(p);
}
EXPORT_SYMBOL(osal_sem_up);

void osal_sem_destroy(osal_semaphore *sem)
{
    struct semaphore *p = NULL;

    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    p = (struct semaphore *)(sem->sem);
    kfree(p);
    sem->sem = NULL;
}
EXPORT_SYMBOL(osal_sem_destroy);
