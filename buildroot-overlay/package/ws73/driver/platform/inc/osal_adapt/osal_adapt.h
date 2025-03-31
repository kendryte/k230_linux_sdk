/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: osal adapt
 * Author: AuthorNameMagicTag
 * Create: 2023-01-11
 */
#ifndef __OSAL_ADAPT_H__
#define __OSAL_ADAPT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#include "td_type.h"
#include "osal_types.h"
#ifdef _OSAL_LITEOS_SDK_
#include "hi_osal.h"
#else /* _OSAL_LITEOS_CFBB_ */
#include "soc_osal.h"
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/module.h>
#include <linux/kernel.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#ifdef _OSAL_LITEOS_SDK_
typedef osal_semaphore_t    osal_semaphore;
typedef osal_mutex_t        osal_mutex;
typedef osal_spinlock_t     osal_spinlock;
typedef osal_task_t         osal_task;
typedef osal_dev_t          osal_dev;
typedef osal_vm_t           osal_vm;
typedef osal_poll_t         osal_poll;
typedef osal_fileops_t      osal_fileops;
typedef osal_pmops_t        osal_pmops;
typedef osal_timer_t        osal_timer;
typedef osal_timeval_t      osal_timeval;
typedef osal_rtc_time_t     osal_rtc_time;
typedef osal_hrtimer_t      osal_hrtimer;
typedef osal_task_t         osal_task;
typedef threadfn_t          osal_kthread_handler;
typedef osal_wait_t         osal_wait;
typedef osal_work_struct_t  osal_workqueue;
typedef void (*osal_workqueue_handler)(struct osal_work_struct *work);
#endif

#ifdef CONFIG_NO_SUPPORT_OSAL
#define osal_adapt_atomic_init osal_atomic_init
#define osal_adapt_atomic_destroy osal_atomic_destroy
#define osal_adapt_atomic_read atomic_read
#define osal_adapt_atomic_set atomic_set
#define osal_adapt_atomic_inc_return atomic_inc_return
#define osal_adapt_atomic_dec_return atomic_dec_return
#define osal_adapt_atomic_inc atomic_inc
#define osal_adapt_atomic_dec atomic_dec
#define osal_adapt_atomic_add osal_atomic_add
#define osal_adapt_strncmp strncmp
#define osal_adapt_kthread_create osal_kthread_create
#define osal_adapt_kthread_should_stop kthread_should_stop
#define osal_adapt_kthread_destroy osal_kthread_destroy
#define osal_adapt_get_current_tid osal_get_current_tid
#define osal_adapt_kthread_set_priority osal_kthread_set_priority
#else
/************************ osal_atomic ************************/
extern int osal_adapt_atomic_init(osal_atomic *atomic);
extern void osal_adapt_atomic_destroy(osal_atomic *atomic);
extern int osal_adapt_atomic_read(osal_atomic *atomic);
extern void osal_adapt_atomic_set(osal_atomic *atomic, int val);
extern int osal_adapt_atomic_inc_return(osal_atomic *atomic);
extern int osal_adapt_atomic_dec_return(osal_atomic *atomic);
extern void osal_adapt_atomic_inc(osal_atomic *atomic);
extern void osal_adapt_atomic_dec(osal_atomic *atomic);
extern void osal_adapt_atomic_add(osal_atomic *atomic, int val);
/************************ osal_string ************************/
extern int osal_adapt_strncmp(const char *str1, const char *str2, unsigned long size);
/************************ osal_task ************************/
extern osal_task *osal_adapt_kthread_create(osal_kthread_handler thread, void *data,
    const char *name, unsigned int stack_size);
extern long osal_adapt_get_current_tid(void);
extern int osal_adapt_kthread_should_stop(void);
extern void osal_adapt_kthread_destroy(osal_task *task, unsigned int stop_flag);
extern int osal_adapt_kthread_set_priority(osal_task *task, unsigned int priority);
#endif

/************************ osal_timer ************************/
extern int osal_adapt_timer_init(osal_timer *timer, void *func, unsigned long data, unsigned int interval);
extern unsigned int osal_adapt_jiffies_to_msecs(const unsigned int n);
extern int osal_adapt_timer_destroy(osal_timer *timer);
extern unsigned long long osal_adapt_get_jiffies(void);
extern int osal_adapt_timer_mod(osal_timer *timer, unsigned int interval);

extern void osal_adapt_kthread_lock(void);
extern void osal_adapt_kthread_unlock(void);
extern int osal_adapt_workqueue_init(osal_workqueue *work, osal_workqueue_handler handler);
extern void osal_adapt_workqueue_destroy(osal_workqueue *work);

/************************ osal_intertupt ************************/
extern unsigned int osal_adapt_irq_lock(void);
extern void osal_adapt_irq_restore(unsigned int irq_status);

#ifdef CONFIG_NO_SUPPORT_OSAL
#define osal_adapt_wait_destroy osal_wait_destroy
#define osal_adapt_wait_wakeup osal_wait_wakeup
#else
/************************ osal_wait ************************/
extern void osal_adapt_wait_destroy(osal_wait *wait);
extern void osal_adapt_wait_wakeup(osal_wait *wait);
#endif

/************************ osal_event ************************/
extern int osal_adapt_event_init(osal_event *event);
extern int osal_adapt_event_write(osal_event *event, unsigned int event_bits);
extern int osal_adapt_event_read(osal_event *event_obj, unsigned int mask, unsigned int timeout_ms, unsigned int mode);
extern int osal_adapt_event_clear(osal_event *event, unsigned int event_bits);
extern int osal_adapt_event_destroy(osal_event *event);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif