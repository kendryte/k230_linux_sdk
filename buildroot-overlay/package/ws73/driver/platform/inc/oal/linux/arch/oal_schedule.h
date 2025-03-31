/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: oal schedule header
 * Author: Huanghe
 * Create: 2023-01-06
 */

#ifndef __OAL_LINUX_SCHEDULE_H__
#define __OAL_LINUX_SCHEDULE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <asm/atomic.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <asm/param.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/ktime.h>
#include "arch/oal_mm.h"
#include "osal_list.h"

#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
#include <linux/stacktrace.h>
#endif
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
#include <linux/pm_wakeup.h>
#endif
#include "linux/time.h"
#include "linux/timex.h"
#include "linux/rtc.h"
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
#include  <linux/pm_wakeup.h>
#endif
#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
typedef atomic_t                oal_atomic;
#define OAL_SPIN_LOCK_MAGIC_TAG (0xdead4ead)
typedef struct _oal_spin_lock_stru_
{
    spinlock_t  lock;
#ifdef CONFIG_SPIN_LOCK_MAGIC_DEBUG
    unsigned int  magic;
    unsigned int  rsv;
#endif
}oal_spin_lock_stru;

typedef wait_queue_head_t    oal_wait_queue_head_stru;
typedef struct timer_list              oal_timer_list_stru;

#define oal_module_param_string module_param_string
#define OAL_S_IRUGO         S_IRUGO

#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
#define OAL_IS_ERR_OR_NULL(ptr)  (!(ptr) || IS_ERR(ptr))
#else
static inline bool __must_check OAL_IS_ERR_OR_NULL(__force const void *ptr)
{
    return !ptr || IS_ERR(ptr);
}
#endif

struct osal_list_head *oal_get_wakelock_head(void);

typedef struct proc_dir_entry       oal_proc_dir_entry_stru;

typedef struct mutex                oal_mutex_stru;

typedef struct completion           oal_completion;

typedef struct
{
    oal_int i_sec;
    oal_int i_usec;
}oal_time_us_stru;

typedef ktime_t oal_time_t_stru;

typedef struct _oal_task_lock_stru_
{
    oal_wait_queue_head_stru    wq;
    struct task_struct  *claimer;   /* task that has host claimed */
    oal_spin_lock_stru      lock;       /* lock for claim and bus ops */
    oal_ulong            claim_addr;
    oal_uint32           claimed;
    oal_int32            claim_cnt;
}oal_task_lock_stru;

//与内核struct rtc_time 保持一致
typedef struct _oal_time_stru {
    oal_int32 tm_sec; /* seconds */
    oal_int32 tm_min; /* minutes */
    oal_int32 tm_hour; /* hours */
    oal_int32 tm_mday; /* day of the month */
    oal_int32 tm_mon; /* month */
    oal_int32 tm_year; /* year */
    oal_int32 tm_wday; /* day of the week */
    oal_int32 tm_yday; /* day in the year */
    oal_int32 tm_isdst; /* daylight saving time */
} oal_time_stru;

typedef struct _oal_wakelock_stru_ {
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
    struct wakeup_source        st_wakelock;        // wakelock锁
    oal_spin_lock_stru      lock;    // wakelock锁操作spinlock锁
    struct osal_list_head     list;
#endif
    oal_ulong               lock_count;         // 持有wakelock锁的次数
    oal_ulong               locked_addr; /* the locked address */
    oal_uint32              debug;         // debug flag
} oal_wakelock_stru;

OAL_STATIC OAL_INLINE oal_void  oal_spin_lock_init(oal_spin_lock_stru *pst_lock)
{
    spin_lock_init(&pst_lock->lock);
#ifdef CONFIG_SPIN_LOCK_MAGIC_DEBUG
    pst_lock->magic = OAL_SPIN_LOCK_MAGIC_TAG;
#endif
}

OAL_STATIC OAL_INLINE oal_void  oal_spin_lock_magic_bug(oal_spin_lock_stru *pst_lock)
{
#ifdef CONFIG_SPIN_LOCK_MAGIC_DEBUG
    if (unlikely((oal_uint32)OAL_SPIN_LOCK_MAGIC_TAG != pst_lock->magic))
    {
#ifdef CONFIG_PRINTK
        /* spinlock never init or memory overwrite? */
        printk(KERN_EMERG "[E]SPIN_LOCK_BUG: spinlock:%p on CPU#%d, %s,magic:%08x should be %08x\n", pst_lock,
                            raw_smp_processor_id(),current->comm, pst_lock->magic,OAL_SPIN_LOCK_MAGIC_TAG);
        print_hex_dump(KERN_EMERG, "spinlock_magic: ", DUMP_PREFIX_ADDRESS, 16, 1, // 一次打印16字节
            (oal_uint8 *)((oal_ulong)(uintptr_t)pst_lock - 32), 32 + sizeof(oal_spin_lock_stru) + 32,  // 打印前后32位的地址
            true);
        printk(KERN_EMERG"\n");
#endif
        OAL_WARN_ON(1);
    }
#endif
}

OAL_STATIC OAL_INLINE oal_void  oal_spin_lock(oal_spin_lock_stru *pst_lock)
{
    oal_spin_lock_magic_bug(pst_lock);
    spin_lock(&pst_lock->lock);
}

OAL_STATIC OAL_INLINE oal_void  oal_spin_unlock(oal_spin_lock_stru *pst_lock)
{
    oal_spin_lock_magic_bug(pst_lock);
    spin_unlock(&pst_lock->lock);
}

OAL_STATIC OAL_INLINE oal_void oal_spin_lock_bh(oal_spin_lock_stru *pst_lock)
{
    oal_spin_lock_magic_bug(pst_lock);
    spin_lock_bh(&pst_lock->lock);
}

OAL_STATIC OAL_INLINE oal_void oal_spin_unlock_bh(oal_spin_lock_stru *pst_lock)
{
     oal_spin_lock_magic_bug(pst_lock);
     spin_unlock_bh(&pst_lock->lock);
}

OAL_STATIC OAL_INLINE oal_void  oal_spin_lock_irq_save(oal_spin_lock_stru *pst_lock, oal_uint *pui_flags)
{
    oal_spin_lock_magic_bug(pst_lock);
    spin_lock_irqsave(&pst_lock->lock, *pui_flags);
}

OAL_STATIC OAL_INLINE oal_void  oal_spin_unlock_irq_restore(oal_spin_lock_stru *pst_lock, oal_uint *pui_flags)
{
    oal_spin_lock_magic_bug(pst_lock);
    spin_unlock_irqrestore(&pst_lock->lock, *pui_flags);
}

OAL_STATIC OAL_INLINE oal_time_t_stru oal_ktime_get(oal_void)
{
    return ktime_get();
}

OAL_STATIC OAL_INLINE oal_time_t_stru oal_ktime_sub(const oal_time_t_stru lhs, const oal_time_t_stru rhs)
{
    return ktime_sub(lhs, rhs);
}

OAL_STATIC OAL_INLINE oal_uint32 oal_time_is_before(oal_uint ui_time)
{
    return (oal_uint32)time_is_before_jiffies(ui_time);
}

OAL_STATIC OAL_INLINE oal_uint32 oal_time_after(oal_ulong ul_time_a, oal_ulong ul_time_b)
{
    return (oal_uint32)time_after(ul_time_a, ul_time_b);
}

OAL_STATIC OAL_INLINE oal_uint32  oal_wait_for_completion_timeout(oal_completion *pst_completion, oal_uint32 ul_timeout)
{
    return (oal_uint32)wait_for_completion_timeout(pst_completion, ul_timeout);
}

/*****************************************************************************
 功能描述  : 同步：等待超时检查
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_uint32 oal_wait_for_completion_interruptible_timeout(oal_completion *completion,
    osal_ulong timeout)
{
    return (oal_uint32)wait_for_completion_interruptible_timeout(completion, timeout);
}

OAL_STATIC OAL_INLINE oal_uint64 oal_get_time_stamp_from_timeval(oal_void)
{
    osal_timeval tv;
    osal_gettimeofday(&tv);
    return (((oal_uint64)tv.tv_sec) * 1000 + (unsigned long)tv.tv_usec / 1000);  /* 1s=1000ms */
}

OAL_STATIC OAL_INLINE oal_void oal_wake_lock_init(oal_wakelock_stru *wakelock, char *name)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    (void)memset_s((oal_void*)wakelock, sizeof(oal_wakelock_stru), 0, sizeof(oal_wakelock_stru));

    wakeup_source_init(&wakelock->st_wakelock, name ? name: "wake_lock_null");
    oal_spin_lock_init(&wakelock->lock);
    wakelock->lock_count = 0;
    wakelock->locked_addr = 0;
    osal_list_add_tail(&wakelock->list, oal_get_wakelock_head());
#endif
#endif
}

OAL_STATIC OAL_INLINE oal_void oal_wake_lock_exit(oal_wakelock_stru *wakelock)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    osal_list_del(&wakelock->list);
    wakeup_source_trash(&wakelock->st_wakelock);
#endif
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_lock(oal_wakelock_stru *wakelock)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    oal_ulong flags;

    oal_spin_lock_irq_save(&wakelock->lock, &flags);
    if (!wakelock->lock_count)
    {
        __pm_stay_awake(&wakelock->st_wakelock);
        wakelock->locked_addr = (oal_ulong)_RET_IP_;
    }
    wakelock->lock_count++;

    oal_spin_unlock_irq_restore(&wakelock->lock, &flags);
#endif
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_unlock(oal_wakelock_stru *wakelock)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    oal_ulong flags;

    oal_spin_lock_irq_save(&wakelock->lock, &flags);
    if (wakelock->lock_count) {
        wakelock->lock_count--;
        if (!wakelock->lock_count) {
            __pm_relax(&wakelock->st_wakelock);
            wakelock->locked_addr = (oal_ulong)0x0;
        }
    }
    oal_spin_unlock_irq_restore(&wakelock->lock, &flags);
#endif
#endif
}

OAL_STATIC OAL_INLINE td_s32 oal_wakelock_active(oal_wakelock_stru *wakelock)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0))
    return wakelock->st_wakelock.active;
#endif
#endif
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_schedule.h */
