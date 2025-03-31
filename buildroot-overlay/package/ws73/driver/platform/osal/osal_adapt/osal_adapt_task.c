/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: osal adapt task  file
 */
#ifdef _OSAL_LITEOS_SDK_
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include "los_task.h"
#endif
#include "osal_adapt.h"


osal_task *osal_adapt_kthread_create(osal_kthread_handler thread, void *data,
    const char *name, unsigned int stack_size)
{
#ifdef _OSAL_LITEOS_SDK_
    if (thread == NULL) {
        osal_printk("parameter invalid!\n");
        return NULL;
    }

    osal_task *task = (osal_task *)LOS_MemAlloc((void*)m_aucSysMem0, sizeof(osal_task));
    if (task == NULL) {
        osal_printk("LOS_MemAlloc failed!\n");
        return NULL;
    }

    TSK_INIT_PARAM_S my_task = { 0 };
    my_task.pcName       = (char *)name;
    my_task.uwStackSize  = (stack_size == 0) ? LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE : stack_size;
    my_task.usTaskPrio   = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    my_task.uwResved     = LOS_TASK_STATUS_DETACHED;
    my_task.pfnTaskEntry = (TSK_ENTRY_FUNC)thread;
    my_task.auwArgs[0] = (AARCHPTR)data;

    unsigned int temp_task_id = 0;
    unsigned int ret = LOS_TaskCreate(&temp_task_id, &my_task);
    if (ret != LOS_OK) {
        LOS_MemFree((void*)m_aucSysMem0, (void*)task);
        osal_printk("LOS_TaskCreate failed! ret = %#x.\n", ret);
        return NULL;
    }
    task->task_struct = (void *)temp_task_id;
    return task;
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_kthread_create(thread, data, name, stack_size);
#endif
}

/* LiteOS 系统适配接口 Linux 系统无该接口 */
#ifdef _OSAL_LITEOS_SDK_
void osal_adapt_kthread_lock(void)
{
    LOS_TaskLock();
}
#endif

/* LiteOS 系统适配接口 Linux 系统无该接口 */
#ifdef _OSAL_LITEOS_SDK_
void osal_adapt_kthread_unlock(void)
{
    LOS_TaskUnlock();
}
#endif

long osal_adapt_get_current_tid(void)
{
#ifdef _OSAL_LITEOS_SDK_
    unsigned int task_id = LOS_CurTaskIDGet();
    if (task_id == LOS_ERRNO_TSK_ID_INVALID) {
        osal_printk("LOS_CurTaskIDGet failed!\n");
        return -1;
    }
    return (long)task_id;
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_get_current_tid();
#endif
}

int osal_adapt_kthread_should_stop(void)
{
#ifdef _OSAL_LITEOS_SDK_
    return 0;
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_kthread_should_stop();
#endif
}

void osal_adapt_kthread_destroy(osal_task *task, unsigned int stop_flag)
{
#ifdef _OSAL_LITEOS_SDK_
    osal_kthread_destory(task, stop_flag);
#else /* _OSAL_LITEOS_CFBB_ */
    osal_kthread_destroy(task, stop_flag);
#endif
}

int osal_adapt_kthread_set_priority(osal_task *task, unsigned int priority)
{
#ifdef _OSAL_LITEOS_SDK_
    if (task == NULL) {
        osal_printk("parameter invalid!\n");
        return -1;
    }
    unsigned int ret = LOS_TaskPriSet((unsigned int)task->task_struct, priority);
    if (ret != LOS_OK) {
        osal_printk("LOS_TaskPriSet failed! ret = %#x.\n", ret);
        return -1;
    }
    return 0;
#else /* _OSAL_LITEOS_CFBB_ */
    return osal_kthread_set_priority(task, priority);
#endif
}

/* LiteOS 系统适配接口 Linux 系统无该接口 */
#ifdef _OSAL_LITEOS_SDK_
unsigned int osal_adapt_irq_lock(void)
{
    return LOS_IntLock();
}
#endif

/* LiteOS 系统适配接口 Linux 系统无该接口 */
#ifdef _OSAL_LITEOS_SDK_
void osal_adapt_irq_restore(unsigned int irq_status)
{
    LOS_IntRestore(irq_status);
}
#endif

void osal_adapt_wait_destroy(osal_wait *wait)
{
#ifdef _OSAL_LITEOS_SDK_
    if (wait == NULL || wait->wait == NULL) {
        osal_printk("parameter invalid!\n");
        return;
    }

    wait_queue_head_t *wq = (wait_queue_head_t *)(wait->wait);

    LOS_EventDestroy(&wq->stEvent);

    LOS_MemFree((void *)m_aucSysMem0, (void *)wq);
    wait->wait = NULL;
#else
    osal_wait_destroy(wait);
#endif
}

void osal_adapt_wait_wakeup(osal_wait *wait)
{
#ifdef _OSAL_LITEOS_SDK_
    if (wait == NULL || wait->wait == NULL) {
        osal_printk("parameter invalid!\n");
        return;
    }

    wait_queue_head_t *wq = (wait_queue_head_t *)(wait->wait);
    (VOID)LOS_EventWrite(&wq->stEvent, 0x1);
#else
    osal_wait_wakeup(wait);
#endif
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(osal_adapt_kthread_create);
EXPORT_SYMBOL(osal_adapt_get_current_tid);
EXPORT_SYMBOL(osal_adapt_kthread_should_stop);
EXPORT_SYMBOL(osal_adapt_kthread_destroy);
EXPORT_SYMBOL(osal_adapt_kthread_set_priority);
EXPORT_SYMBOL(osal_adapt_wait_destroy);
EXPORT_SYMBOL(osal_adapt_wait_wakeup);
#endif