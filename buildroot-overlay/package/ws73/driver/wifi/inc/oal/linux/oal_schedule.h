/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Create: 2022-4-27
 */

#ifndef OAL_LINUX_SCHEDULE_H
#define OAL_LINUX_SCHEDULE_H

#include <asm/atomic.h>
#include <asm/param.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/ktime.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0))
#include <linux/stacktrace.h>
#endif
#include "linux/time.h"
#include "linux/timex.h"
#include "linux/rtc.h"

#include "oal_time.h"
#include "oal_spinlock.h"
#include "oal_schedule_hcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct _oal_wakelock_stru_ {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    struct wakeup_source wakelock; /* wakelock锁 */
    osal_spinlock lock; /* wakelock锁操作spinlock锁 */
    struct osal_list_head list;
#endif

    osal_ulong lock_count; /* 持有wakelock锁的次数 */
    osal_ulong locked_addr; /* the locked address */
    osal_u32 debug; /* debug flag */
} oal_wakelock_stru;

typedef atomic_t oal_atomic;


/* 函数指针，用来指向需要自旋锁保护的的函数 */
typedef osal_u32              (*oal_irqlocked_func)(osal_void *);

typedef rwlock_t                oal_rwlock_stru;
typedef struct timer_list              oal_timer_list_stru;


typedef struct tasklet_struct       oal_tasklet_stru;
typedef osal_void                    (*oal_defer_func)(osal_ulong);

/* tasklet声明 */
#define OAL_DECLARE_TASK    DECLARE_TASKLET

/**
 * wait_event_interruptible_timeout - sleep until a condition gets true or a timeout elapses
 * @_wq: the waitqueue to wait on
 * @_condition: a C expression for the event to wait for
 * @_timeout: timeout, in jiffies
 *
 * The process is put to sleep (TASK_INTERRUPTIBLE) until the
 * @condition evaluates to true or a signal is received.
 * The @condition is checked each time the waitqueue @wq is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * The function returns 0 if the @timeout elapsed, -ERESTARTSYS if it
 * was interrupted by a signal, and the remaining jiffies otherwise
 * if the condition evaluated to true before the timeout elapsed.
 */
#define OAL_WAIT_EVENT_INTERRUPTIBLE_TIMEOUT(_st_wq, _condition, _timeout) \
    wait_event_interruptible_timeout(_st_wq, _condition, _timeout)

#define OAL_WAIT_EVENT_TIMEOUT(_st_wq, _condition, _timeout) \
    wait_event_timeout(_st_wq, _condition, _timeout)

#define OAL_WAIT_EVENT_INTERRUPTIBLE(_st_wq, _condition) \
    wait_event_interruptible(_st_wq, _condition)

#define OAL_INIT_COMPLETION(_my_completion) init_completion(_my_completion)

#define OAL_COMPLETE(_my_completion)        complete(_my_completion)

#define oal_in_interrupt()  in_interrupt()

typedef osal_u32 (*oal_module_func_t)(osal_void);


#define oal_module_license(_license_name) MODULE_LICENSE(_license_name)

#define oal_module_param        module_param
#define oal_module_param_string module_param_string

#define OAL_S_IRUGO         S_IRUGO

#define oal_module_init(_module_name)   module_init(_module_name)
#define oal_module_exit(_module_name)   module_exit(_module_name)
#define oal_module_symbol(_symbol)      EXPORT_SYMBOL(_symbol)
#define OAL_MODULE_DEVICE_TABLE(_type, _name) MODULE_DEVICE_TABLE(_type, _name)

#define oal_smp_call_function_single(core, task, info, wait) smp_call_function_single(core, task, info, wait)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#define OAL_IS_ERR_OR_NULL(ptr)  (!(ptr) || IS_ERR(ptr))
#else
static inline bool __must_check OAL_IS_ERR_OR_NULL(__force const void *ptr)
{
    return !ptr || IS_ERR(ptr);
}
#endif

typedef struct proc_dir_entry       oal_proc_dir_entry_stru;

typedef struct mutex                oal_mutex_stru;

typedef struct completion           oal_completion;

/*****************************************************************************
 功能描述  : 读写锁初始化，把读写锁设置为1（未锁状态）。
 输入参数  : lock: 读写锁结构体地址
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_rw_lock_init(oal_rwlock_stru *lock)
{
    rwlock_init(lock);
}

/*****************************************************************************
 功能描述  : 获得指定的读锁
 输入参数  : lock: 读写锁结构体地址
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_rw_lock_read_lock(oal_rwlock_stru *lock)
{
    read_lock(lock);
}

/*****************************************************************************
 功能描述  : 释放指定的读锁
 输入参数  : lock: 读写锁结构体地址
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_rw_lock_read_unlock(oal_rwlock_stru *lock)
{
    read_unlock(lock);
}

/*****************************************************************************
 功能描述  : 获得指定的写锁
 输入参数  : lock: 读写锁结构体地址
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_rw_lock_write_lock(oal_rwlock_stru *lock)
{
    write_lock(lock);
}

/*****************************************************************************
 功能描述  : 释放指定的写锁
 输入参数  : lock: 读写锁结构体地址
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_rw_lock_write_unlock(oal_rwlock_stru *lock)
{
    write_unlock(lock);
}


/*****************************************************************************
 功能描述  : 任务初始化。初始化完成后，任务处于挂起状态。
 输入参数  : task: 任务结构体指针
             func: 任务处理函数入口地址
             p_args: 需进行处理的函数的入参地址
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_task_init(oal_tasklet_stru *task, oal_defer_func p_func, osal_ulong args)
{
    tasklet_init(task, p_func, args);
}

/*****************************************************************************
 功能描述  : 退出task运行
 输入参数  : task: 任务结构体指针
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void oal_task_kill(oal_tasklet_stru *task)
{
    return tasklet_kill(task);
}

/*****************************************************************************
 功能描述  : 任务调度，令任务处于准备就绪状态；当任务执行完后，又回到挂起状
             态。
 输入参数  : task: 任务结构体指针
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_task_sched(oal_tasklet_stru *task)
{
    tasklet_schedule(task);
}

/*****************************************************************************
 功能描述  : 检测tasklet是否等待执行
 输入参数  : task: 任务结构体指针
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_ulong oal_task_is_scheduled(oal_tasklet_stru *task)
{
    return test_bit(TASKLET_STATE_SCHED, (unsigned long const volatile *)&task->state);
}

/*****************************************************************************
 功能描述  : 创建proc_entry结构体
 输入参数  : pc_name: 创建的proc_entry的名字
             mode: 创建模式
             parent: 母proc_entry结构体，继承属性
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_proc_dir_entry_stru *oal_create_proc_entry(const osal_s8 *pc_name, osal_u16 mode,
    oal_proc_dir_entry_stru *parent)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,11))
    return NULL;
#else
    return create_proc_entry(pc_name, mode, parent);
#endif
}

/*****************************************************************************
 功能描述  : 创建proc_entry结构体
 输入参数  : pc_name: 创建的proc_entry的名字
             parent: 母proc_entry结构体，继承属性
*****************************************************************************/
OAL_STATIC OAL_INLINE void oal_remove_proc_entry(const osal_s8 *pc_name, oal_proc_dir_entry_stru *parent)
{
    return remove_proc_entry(pc_name, parent);
}


/*****************************************************************************
 功能描述  : 同步：等待超时检查
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32  oal_wait_for_completion_timeout(oal_completion *completion, osal_u32 timeout)
{
    return (osal_u32)wait_for_completion_timeout(completion, timeout);
}

/*****************************************************************************
 功能描述  : 同步：等待超时检查
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_slong  oal_wait_for_completion_interruptible_timeout(oal_completion *completion,
    osal_ulong timeout)
{
    return (osal_slong)wait_for_completion_interruptible_timeout(completion, timeout);
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_s32  oal_print_all_wakelock_buff(char* buf, osal_s32 buf_len);
osal_s32  oal_set_wakelock_debuglevel(char* name, osal_u32 level);
#endif

OAL_STATIC OAL_INLINE osal_void oal_wake_lock_init(oal_wakelock_stru *wakelock, char* name)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    memset_s((osal_void*)wakelock, sizeof(oal_wakelock_stru), 0, sizeof(oal_wakelock_stru));

    wakeup_source_init(&wakelock->wakelock, name ? name : "wake_lock_null");
    osal_spin_lock_init(&wakelock->lock);
    wakelock->lock_count = 0;
    wakelock->locked_addr = 0;
    /* add to local list */
    osal_list_add_tail(&wakelock->list, oal_get_wakelock_head());
#endif
#endif
}

OAL_STATIC OAL_INLINE osal_void oal_wake_lock_exit(oal_wakelock_stru *wakelock)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    osal_list_del(&wakelock->list);
    osal_spin_lock_destroy(&wakelock->lock);
    wakeup_source_trash(&wakelock->wakelock);
#endif
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_lock(oal_wakelock_stru *wakelock)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    osal_ulong flags;

    osal_spin_lock_irqsave(&wakelock->lock, &flags);
    if (!wakelock->lock_count) {
        __pm_stay_awake(&wakelock->wakelock);
        wakelock->locked_addr = (osal_ulong)_RET_IP_;
    }
    wakelock->lock_count++;
    if (OAL_UNLIKELY(wakelock->debug)) {
        printk(KERN_DEBUG"wakelock[%s] lockcnt:%lu <==%pf\n", wakelock->wakelock.name, wakelock->lock_count,
            (osal_void *)_RET_IP_);
    }
    osal_spin_unlock_irqrestore(&wakelock->lock, &flags);
#endif
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_unlock(oal_wakelock_stru *wakelock)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    osal_ulong flags;

    osal_spin_lock_irqsave(&wakelock->lock, &flags);
    if (wakelock->lock_count) {
        wakelock->lock_count--;
        if (!wakelock->lock_count) {
            __pm_relax(&wakelock->wakelock);
            wakelock->locked_addr = (osal_ulong)0x0;
        }

        if (OAL_UNLIKELY(wakelock->debug)) {
            printk(KERN_DEBUG"wakeunlock[%s] lockcnt:%lu <==%pf\n", wakelock->wakelock.name, wakelock->lock_count,
                (osal_void *)_RET_IP_);
        }
    }
    osal_spin_unlock_irqrestore(&wakelock->lock, &flags);
#endif
#endif
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_schedule.h */
