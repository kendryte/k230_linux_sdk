/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "soc_osal.h"
#include "osal_inner.h"

int osal_wait_init(osal_wait *wait)
{
    wait_queue_head_t *wq = NULL;
    if (wait == NULL || wait->wait != NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }
    wq = (wait_queue_head_t *)kmalloc(sizeof(wait_queue_head_t), GFP_ATOMIC);
    if (wq == NULL) {
        osal_log("kmalloc error!\n");
        return OSAL_FAILURE;
    }
    init_waitqueue_head(wq);
    wait->wait = wq;
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_wait_init);
int osal_wait_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);

    if (func == NULL) {
        return wait_event_interruptible((*wq), OSAL_WAIT_CONDITION_TRUE);
    }

    return wait_event_interruptible((*wq), func(param));
}
EXPORT_SYMBOL(osal_wait_interruptible);

int osal_wait_uninterruptible(osal_wait *wait, osal_wait_condition_func func, const void *param)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    wq = (wait_queue_head_t *)(wait->wait);

    if (func == NULL) {
        wait_event((*wq), OSAL_WAIT_CONDITION_TRUE);
    } else {
        wait_event((*wq), func(param));
    }

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_wait_uninterruptible);

int osal_wait_timeout_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param, unsigned long ms)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    wq = (wait_queue_head_t *)(wait->wait);

    if (func == NULL) {
        return wait_event_interruptible_timeout((*wq), OSAL_WAIT_CONDITION_TRUE, msecs_to_jiffies(ms));
    }

    return wait_event_interruptible_timeout((*wq), func(param), msecs_to_jiffies(ms));
}

EXPORT_SYMBOL(osal_wait_timeout_interruptible);

int osal_wait_timeout_uninterruptible(osal_wait *wait, osal_wait_condition_func func,
    const void *param, unsigned long ms)
{
    int timeout;
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    wq = (wait_queue_head_t *)(wait->wait);

    if (func == NULL) {
        timeout = wait_event_timeout((*wq), OSAL_WAIT_CONDITION_TRUE, msecs_to_jiffies(ms));
    } else {
        timeout = wait_event_timeout((*wq), func(param), msecs_to_jiffies(ms));
    }

    return timeout;
}

EXPORT_SYMBOL(osal_wait_timeout_uninterruptible);

void osal_wait_wakeup(osal_wait *wait)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("wait is NULL!\n");
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);

    wake_up_all(wq);
}
EXPORT_SYMBOL(osal_wait_wakeup);
void osal_wait_destroy(osal_wait *wait)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("wait is NULL!\n");
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    kfree(wq);
    wait->wait = NULL;
}
EXPORT_SYMBOL(osal_wait_destroy);
