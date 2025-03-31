/*
 * Copyright (c) CompanyNameMagicTag. 2021-2023. All rights reserved.
 * Description: OS Abstract Layer.
 * Author: CompanyName
 * Create: 2023-01-04
 */

#ifndef __SOC_OSAL_H__
#define __SOC_OSAL_H__

#ifndef CONFIG_NON_OS
#include <stdarg.h>
#endif
#ifdef CONFIG_NO_SUPPORT_OSAL
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#endif
#include "td_type.h"
#include "osal_types.h"
#include "osal_list.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OSAL_SUCCESS  0
#define OSAL_FAILURE  (-1)

// uranus depend
#define ERESTARTSYS 512

#ifdef CONFIG_NON_OS
#define OSAL_NONEOS_DEFAULT_RET 0
#define EXT_IRQ_FLAG_PRI0       0
#define EXT_IRQ_FLAG_PRI1       1
#define EXT_IRQ_FLAG_PRI2       2
#define EXT_IRQ_FLAG_PRI3       3
#define EXT_IRQ_FLAG_PRI4       4
#define EXT_IRQ_FLAG_PRI5       5
#define EXT_IRQ_FLAG_PRI6       6
#define EXT_IRQ_FLAG_PRI7       7
#define EXT_IRQ_FLAG_PRI8       8
#define EXT_IRQ_FLAG_PRI9       9
#define EXT_IRQ_FLAG_PRI10      10
#define EXT_IRQ_FLAG_PRI11      11
#define EXT_IRQ_FLAG_PRI12      12
#define EXT_IRQ_FLAG_PRI13      13
#define EXT_IRQ_FLAG_PRI14      14
#define EXT_IRQ_FLAG_PRI15      15
#endif

#ifdef CONFIG_NO_SUPPORT_OSAL
#define OSAL_GFP_KERNEL GFP_KERNEL
#define OSAL_GFP_ATOMIC GFP_ATOMIC
#define OSAL_GFP_DMA GFP_DMA
#define OSAL_GFP_ZERO __GFP_ZERO
#define osal_kmalloc kmalloc
#define osal_kfree kfree
#define osal_vmalloc vmalloc
#define osal_vfree vfree
#define osal_copy_from_user copy_from_user
#define osal_copy_to_user copy_to_user
#define osal_access_ok access_ok
#define osal_phys_to_virt phys_to_virt
#define osal_virt_to_phys virt_to_phys
#else
#define OSAL_GFP_ZERO      (0x1)
#define OSAL_GFP_ATOMIC    (0x1 << 1)
#define OSAL_GFP_DMA       (0x1 << 2)
#define OSAL_GFP_KERNEL    (0x1 << 3)
/* osal sub module support switch */
/* not define #define OSAL_API_SUPPORT_DCACHE */
/* not define #define OSAL_API_SUPPORT_MEMMAP */

/**************************************** mem api ****************************************/
/*
 * osal_kmalloc() - This API is used to allocate a memory block of which the size is specified.
 * Support System: linux liteos freertos.
 */
void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag);
/*
 * osal_kfree() - This API is used to free specified dynamic memory that has been allocated.
 * Support System: linux liteos freertos.
 */
void osal_kfree(void *addr);
/*
 * osal_vmalloc() - This API is used to allocate memory space with consecutive virtual addresses.
 * Support System: linux liteos freertos.
 */
void *osal_vmalloc(unsigned long size);

/*
 * Description: This API is used to frees up memory blocks starting from addr.
 * Support System: linux liteos freertos.
 */
void osal_vfree(void *addr);
void *osal_phys_to_virt(unsigned long addr);
unsigned long osal_virt_to_phys(const void *virt_addr);
unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n);
unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n);
int osal_access_ok(int type, const void *addr, unsigned long size);
#endif

/*
 * osal_kmalloc_align() - This API is used to allocate a Auto-align memory block of which the size is specified.
 * Support System:  liteos.
 */
void *osal_kmalloc_align(unsigned long size, unsigned int osal_gfp_flag, unsigned int boundary);

/*
 * Description: This API is used to init mem pool.
 * Support System: liteos, seliteos.
 */
int osal_pool_mem_init(void *pool, unsigned int size);

/*
 * Description: This API is used to alloc a memory block from the @pool.
 * Support System: liteos, seliteos.
 */
void *osal_pool_mem_alloc(void *pool, unsigned long size);

/*
 * Description: This API is used to alloc a memory block from the @pool.
 * Support System: liteos, seliteos.
 */
void *osal_pool_mem_alloc_align(void *pool, unsigned long size, unsigned int boundary);

/*
 * Description: This API is used to free @addr to the @pool.
 * Support System: liteos, seliteos.
 */
void osal_pool_mem_free(void *pool, const void *addr);

/*
 * Description: This API is used to deinit mem pool.
 * Support System: liteos, seliteos.
 */
int osal_pool_mem_deinit(void *pool);

/* *************************************** atomic api *************************************** */
#ifdef CONFIG_NO_SUPPORT_OSAL
typedef atomic_t osal_atomic;
static inline void osal_atomic_init(osal_atomic *atomic)
{
    atomic_set(atomic, 0);
}
static inline void osal_atomic_destroy(osal_atomic *atomic)
{
    atomic_set(atomic, 0);
}
#define osal_atomic_read atomic_read
#define osal_atomic_set atomic_set
#define osal_atomic_inc_return atomic_inc_return
#define osal_atomic_dec_return atomic_dec_return
#define osal_atomic_inc atomic_inc
#define osal_atomic_dec atomic_dec
static inline void osal_atomic_add(osal_atomic *atomic, int val)
{
    atomic_add(val, atomic);
}
#define osal_atomic_dec_and_test atomic_dec_and_test
#define osal_atomic_inc_and_test atomic_inc_and_test
#define osal_atomic_inc_not_zero atomic_inc_not_zero
#else
typedef struct {
    volatile int counter;
} osal_atomic;

/*
 * osal_atomic_init() - This API is used to implement the atomic init.
 * Attention: atomic->atomic must be null when call this function
 * Support System: linux liteos freertos.
 */
int osal_atomic_init(osal_atomic *atomic);

/*
 * osal_atomic_destroy() - This API is used to implement the atomic destroy.
 * Support System: linux liteos freertos.
 */
void osal_atomic_destroy(osal_atomic *atomic);

/*
 * osal_atomic_read() - This API is used to implement the atomic read and
 * return the value read from the input parameter v.
 * Support System: linux liteos freertos.
 */
int osal_atomic_read(osal_atomic *atomic);

/*
 * osal_atomic_set() - This API is used to implement the atomic setting operation.
 * Support System: linux liteos freertos.
 */
void osal_atomic_set(osal_atomic *atomic, int i);

/*
 * osal_atomic_inc_return() - This API is used to implement the atomic self-addition and
 * return the self-addition result.
 * Support System: linux liteos freertos.
 */
int osal_atomic_inc_return(osal_atomic *atomic);

/*
 * osal_atomic_dec_return() - This API is used to implement the atomic self-decrement and
 * return the self-decrement result.
 * Support System: linux liteos freertos.
 */
int osal_atomic_dec_return(osal_atomic *atomic);

/*
 * osal_atomic_inc)() - This API is used to implement the atomic self-addition.
 * Support System: linux liteos freertos.
 */
void osal_atomic_inc(osal_atomic *atomic);

/*
 * osal_atomic_dec() - This API is used to implement the atomic self-decrement.
 * Support System: linux liteos freertos.
 */
void osal_atomic_dec(osal_atomic *atomic);

/*
 * osal_atomic_dec_and_test() - This API is used to subtract 1 atomically from the variable v of the atomic type
 * and checks whether the result is 0. If yes, true is returned. Otherwise, false is returned.
 * Support System: linux.
 */
int osal_atomic_dec_and_test(osal_atomic *atomic);

/*
 * osal_atomic_inc_and_test() - This API is used to add 1 atomically from the variable v of the atomic type
 * and checks whether the result is 0. If yes, true is returned. Otherwise, false is returned.
 * Support System: linux.
 */
int osal_atomic_inc_and_test(osal_atomic *atomic);

/*
 * osal_atomic_inc_not_zero() - This API is used to add an integer and check whether the returned value is 0.
 * Support System: linux.
 */
int osal_atomic_inc_not_zero(osal_atomic *atomic);

void osal_atomic_add(osal_atomic *atomic, int i);
#endif

/* *************************************** semaphore api *************************************** */
#define OSAL_SEM_WAIT_FOREVER (-1)

#ifdef CONFIG_NO_SUPPORT_OSAL
typedef struct semaphore osal_semaphore;
static inline int osal_sem_init(osal_semaphore *sem, int val)
{
    sema_init(sem, val);
    return 0;
}
#define osal_sem_down down
#define osal_sem_down_timeout down_timeout
#define osal_sem_down_interruptible down_interruptible
#define osal_sem_trydown down_trylock
#define osal_sem_up up
static inline void osal_sem_destroy(osal_semaphore *sem)
{
    (void)sem;
}
#else
typedef struct {
    void *sem;
} osal_semaphore;

/*
 * osal_sem_init() - This API is used to initialization of semaphore.
 * Support System: linux liteos freertos.
 */
int osal_sem_init(osal_semaphore *sem, int val);

/*
 * osal_sem_down() - This API is used to attempt to obtain the semaphore.
 * @sem: the semaphore to be acquired.
 * Acquires the semaphore.  If no more tasks are allowed to acquire the
 * semaphore, calling this function will put the task to sleep until the
 * semaphore is released.
 * Support System: linux liteos freertos.
 */
int osal_sem_down(osal_semaphore *sem);

/*
 * osal_sem_down_timeout() - acquire the semaphore within a specified time.
 * that specifies the timeout period.
 * @sem: the semaphore to be acquired.
 * @timeout: how long to wait before failing.
 * Support System: liteos.
 */
int osal_sem_down_timeout(osal_semaphore *sem, unsigned int timeout);

/*
 * osal_sem_down_interruptible() - acquire the semaphore unless interrupted.
 * @sem: the semaphore to be acquired.
 * Attempts to acquire the semaphore.  If no more tasks are allowed to
 * acquire the semaphore, calling this function will put the task to sleep.
 * Support System: linux liteos.
 */
int osal_sem_down_interruptible(osal_semaphore *sem);

/*
 * osal_sem_trydown() - try to acquire the semaphore, without waiting.
 * @sem: the semaphore to be acquired.
 * If the semaphore cannot be obtained, the system returns 1 instead of sleeping.
 * If the value 0 is returned, the semaphore is obtained.
 * Support System: linux liteos.
 */
int osal_sem_trydown(osal_semaphore *sem);

/*
 * osal_sem_up() - release the semaphore.
 * @sem: the semaphore to release.
 * Release the semaphore.  Unlike mutexes, osal_sem_up() may be called from any
 * context and even by tasks which have never called osal_sem_down().
 * Support System: linux liteos freertos.
 */
void osal_sem_up(osal_semaphore *sem);

/*
 * osal_sem_destroy() - This API is used to destroy semaphore and release space.
 * @sem: the semaphore to destroyed.
 * Support System: linux liteos freertos.
 */
void osal_sem_destroy(osal_semaphore *sem);
#endif

/*
 * osal_sem_binary_sem_init() - This API is used to create a binary semaphore control structure
 * according to the initial number of available semaphores specified by count
 * and return the ID of this semaphore control structure.
 * Support System: liteos freertos.
 */
int osal_sem_binary_sem_init(osal_semaphore *sem, int val);

/* *************************************** mutex api *************************************** */
#define OSAL_MUTEX_WAIT_FOREVER (-1)

#ifdef CONFIG_NO_SUPPORT_OSAL
typedef struct mutex osal_mutex;
static inline int osal_mutex_init(osal_mutex *mutex)
{
    mutex_init(mutex);
    return 0;
}
#define osal_mutex_lock mutex_lock
#define osal_mutex_lock_interruptible mutex_lock_interruptible
#define osal_mutex_trylock mutex_trylock
#define osal_mutex_unlock mutex_unlock
#define osal_mutex_destroy mutex_destroy
#define osal_mutex_is_locked mutex_is_locked
#else
typedef struct {
    void *mutex;
} osal_mutex;

/*
 * osal_mutex_init() - This API is used to initialization of mutex.
 * @mutex: the mutex to be initialized
 * Initialize the mutex to unlocked state.
 * It is not allowed to initialize an already locked mutex.
 * Support System: linux liteos freertos.
 */
int osal_mutex_init(osal_mutex *mutex);

/*
 * osal_mutex_lock() - acquire the mutex.
 * Lock the mutex exclusively for this task. If the mutex is not
 * available right now, it will sleep until it can get it.
 *
 * The mutex must later on be released by the same task that
 * acquired it. Recursive locking is not allowed. The task
 * may not exit without first unlocking the mutex. Also, kernel
 * memory where the mutex resides must not be freed with
 * the mutex still locked. The mutex must first be initialized
 * (or statically defined) before it can be locked. memset()-ing
 * the mutex to 0 is not allowed.
 *
 * This function is similar to (but not equivalent to) down().
 * Support System: linux liteos freertos.
 */
int osal_mutex_lock(osal_mutex *mutex);

/*
 * osal_mutex_lock_timeout() - This API is used to wait for a specified period of time to lock a mutex.
 * @mutex: The mutex to be acquired.
 * @timeout: how long to wait before failing.
 * Support System: liteos.
 */
int osal_mutex_lock_timeout(osal_mutex *mutex, unsigned int timeout);

/*
 * osal_mutex_lock_interruptible() - Acquire the mutex, interruptible by signals.
 * @mutex: The mutex to be acquired.
 * Lock the mutex like mutex_lock(). If a signal is delivered while the
 * process is sleeping, this function will return without acquiring the
 * mutex.
 * Support System: linux liteos.
 */
int osal_mutex_lock_interruptible(osal_mutex *mutex);

/*
 * osal_mutex_trylock() - try to acquire the mutex, without waiting.
 * @mutex: The mutex to be acquired.
 * Support System: linux liteos.
 */
int osal_mutex_trylock(osal_mutex *mutex);

/*
 * osal_mutex_unlock() - release the mutex.
 * @mutex: the mutex to be released
 * Unlock a mutex that has been locked by this task previously.
 *
 * This function must not be used in interrupt context. Unlocking
 * of a not locked mutex is not allowed.
 *
 * This function is similar to (but not equivalent to) up().
 * Support System: linux liteos freertos.
 */
void osal_mutex_unlock(osal_mutex *mutex);

/*
 * osal_mutex_is_locked() - is the mutex locked.
 * @mutex: The mutex to be acquired.
 * Returns true if the mutex is locked, false if unlocked.
 * Support System: linux liteos(NULL) freertos.
 */
int osal_mutex_is_locked(osal_mutex *mutex);


// notice:must be called when kmod exit, other wise will lead to memory leak;
/*
 * osal_mutex_destroy() - This API is used to destroy the mutex.
 * @mutex: The mutex to be destroyed.
 * Support System: linux liteos freertos.
 */
void osal_mutex_destroy(osal_mutex *mutex);
#endif

#ifdef CONFIG_NO_SUPPORT_OSAL
typedef spinlock_t osal_spinlock;
static inline int osal_spin_lock_init(osal_spinlock *lock)
{
    spin_lock_init(lock);
    return 0;
}
#define osal_spin_lock spin_lock
#define osal_spin_lock_bh spin_lock_bh
#define osal_spin_unlock spin_unlock
#define osal_spin_unlock_bh spin_unlock_bh
static inline void osal_spin_lock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
    spin_lock_irqsave(lock, *flags);
}
static inline void osal_spin_unlock_irqrestore(osal_spinlock *lock, unsigned long *flags)
{
    spin_unlock_irqrestore(lock, *flags);
}

static inline void osal_spin_lock_destroy(osal_spinlock *lock)
{
    (void)lock;
}
#else
/**************************************** spin lock api ****************************************/
typedef struct {
    void *lock;
} osal_spinlock;

/*
 * osal_spin_lock_init() - This API is used to initialization of spin_lock.
 * @lock: the lock to be initialized.
 * Support System: linux liteos.
 */
int osal_spin_lock_init(osal_spinlock *lock);

/*
 * osal_spin_lock() - acquire the spin_lock.
 * @lock: the lock to be acquired.
 * Support System: linux liteos.
 */
void osal_spin_lock(osal_spinlock *lock);

/*
 * Support System: linux/liteos.
 */
void osal_spin_lock_bh(osal_spinlock *lock);

/*
 * osal_spin_trylock() - try to acquire the spin_lock.
 * @lock: the lock to be acquired.
 * Support System: linux liteos.
 */
int osal_spin_trylock(osal_spinlock *lock);

/*
 * osal_spin_trylock_irq() - try to acquire the spin_lock and disables the CPU interrupt.
 * @lock: the lock to be acquired.
 * Support System: linux.
 */
int osal_spin_trylock_irq(osal_spinlock *lock);

/*
 * osal_spin_trylock_irqsave() - Saves the current IRQ status of the CPU, try to acquire the spin_lock,
 * and disables the interrupts of the CPU.
 * @lock: the lock to be acquired.
 * Support System: linux.
 */
void osal_spin_trylock_irqsave(osal_spinlock *lock, unsigned long *flags);

/*
 * osal_spin_unlock() - release the mutex.
 * @lock: the lock to be released.
 * Support System: linux liteos.
 */
void osal_spin_unlock(osal_spinlock *lock);

/*
 * Support System: linux/liteos.
 */
void osal_spin_unlock_bh(osal_spinlock *lock);

/*
 * osal_spin_lock_irqsave() - Saves the current IRQ status of the CPU, obtains the specified spin_lock,
 * and disables the interrupts of the CPU.
 * @lock: the lock to be acquired.
 * Support System: linux liteos.
 */
void osal_spin_lock_irqsave(osal_spinlock *lock, unsigned long *flags);

/*
 * osal_spin_unlock_irqrestore() - Releases the specified spin_lock and restores the interrupt status of the CPU.
 * @lock: the lock to be released.
 * Support System: linux liteos.
 */
void osal_spin_unlock_irqrestore(osal_spinlock *lock, unsigned long *flags);

/*
 * osal_spin_lock_destroy() - Destroy the spin_lock.
 * @lock: the lock to be destroyed.
 * Support System: linux liteos.
 * notice:must be called when kmod exit, other wise will lead to memory leak;
 */
void osal_spin_lock_destroy(osal_spinlock *lock);
#endif

/**************************************** wait api ****************************************/
#define OSAL_WAIT_FOREVER 0xFFFFFFFF
#define OSAL_WAIT_CONDITION_TRUE 1
typedef struct {
    void *wait;
} osal_wait;

typedef int (*osal_wait_condition_func)(const void *param);

/*
 * osal_wait_init() - This API is used to initialization of spin_lock.
 * @wait: the wait to be initialized.
 * Support System: linux liteos.
 */
int osal_wait_init(osal_wait *wait);

/*
 * osal_wait_interruptible() - sleep until a condition gets true.
 * The process is put to sleep (TASK_INTERRUPTIBLE) until the
 * @condition evaluates to true or a signal is received.
 * The @condition is checked each time the waitqueue @wait is woken up.
 * Support System: linux liteos.
 */
int osal_wait_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param);

/*
 * osal_wait_uninterruptible() - sleep until a condition gets true.
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wait is woken up.
 * Support System: linux liteos.
 */
int osal_wait_uninterruptible(osal_wait *wait, osal_wait_condition_func func, const void *param);

/*
 * osal_wait_timeout_interruptible() - sleep until a condition gets true or a timeout elapses.
 * @wait: the waitqueue to wait on
 * @func: a C expression for the event to wait for
 * @ms: timeout, in ms
 * The process is put to sleep (TASK_INTERRUPTIBLE) until the
 * @condition evaluates to true or a signal is received.
 * The @condition is checked each time the waitqueue @wait is woken up.
 * Support System: linux liteos.
 */
int osal_wait_timeout_interruptible(osal_wait *wait,
    osal_wait_condition_func func, const void *param, unsigned long ms);

/*
 * osal_wait_timeout_uninterruptible() - sleep until a condition gets true or a timeout elapses.
 * @wait: the waitqueue to wait on
 * @func: a C expression for the event to wait for
 * @ms: timeout, in ms
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wait is woken up.
 * Support System: linux liteos.
 * same to osal_wait_timeout_interruptible on liteos
 */
int osal_wait_timeout_uninterruptible(osal_wait *wait,
    osal_wait_condition_func func, const void *param, unsigned long ms);

/*
 * osal_wait_wakeup() - Wake-up wait queue,pair to @wait_event.
 * @wait: the wait to be wake up.
 * Support System: linux liteos.
 */
void osal_wait_wakeup(osal_wait *wait);  // same as wake_up_all
void osal_wait_destroy(osal_wait *wait);

/***********************************event api************************************/
// Event reading mode: The task waits for all its expected events to occur.
#define OSAL_WAITMODE_AND 4U
// Event reading mode: The task waits for any of its expected events to occur.
#define OSAL_WAITMODE_OR  2U
// Event reading mode: The event flag is immediately cleared after the event is read.
#define OSAL_WAITMODE_CLR 1U

typedef struct {
    void *event;
} osal_event;

int osal_event_init(osal_event *event);
int osal_event_write(osal_event *event, unsigned int event_bits);
int osal_event_read(osal_event *event_obj, unsigned int mask, unsigned int timeout_ms, unsigned int mode);
int osal_event_clear(osal_event *event, unsigned int event_bits);
int osal_event_destroy(osal_event *event);

/************************************workqueue api******************************/
typedef struct osal_workqueue_ {
    int  queue_flag;
    void *work;
    void (*handler)(struct osal_workqueue_ *workqueue);
} osal_workqueue;
typedef void (*osal_workqueue_handler)(osal_workqueue *workqueue);

int osal_workqueue_init(osal_workqueue *work, osal_workqueue_handler handler);
int osal_workqueue_schedule(osal_workqueue *work);
/*
 * osal_workqueue_destroy() - This API is used to destroy workqueue.
 * @work: the work to be destroyed.
 * Support System: linux liteos.
 */
void osal_workqueue_destroy(osal_workqueue *work);

/*
 * osal_workqueue_flush() - wait for a work to finish executing the last queueing instance.
 * @work: the work to flush.
 * Wait until @work has finished execution.  @work is guaranteed to be idle
 * on return if it hasn't been requeued since flush started.
 * Support System: linux liteos.
 */
int osal_workqueue_flush(osal_workqueue *work);

/**************************************** delayed work api ****************************************/
typedef struct osal_delayedwork_ {
    void *work;
    void (*handler)(struct osal_delayedwork_ *delayedwork);
} osal_delayedwork;
typedef void (*osal_delayedwork_handler)(osal_delayedwork *delayedwork);

/*
 * osal_delayedwork_init() - This API is used to initialization of delayedwork.
 * @work: the delayedwork to be initialized.
 * Support System: linux.
 */
int osal_delayedwork_init(osal_delayedwork *work, osal_delayedwork_handler handler);

/*
 * osal_delayedwork_destroy() - This API is used to destroy the delayedwork.
 * @work: the delayedwork to be destroyed.
 * Support System: linux.
 */
void osal_delayedwork_destroy(osal_delayedwork *work);

/*
 * osal_delayedwork_schedule() - put work task in global workqueue after delay.
 * @work: job to be done.
 * @timeout: number of jiffies to wait or 0 for immediate execution.
 * After waiting for a given time this puts a job in the kernel-global
 * workqueue.
 * Support System: linux.
 */
int osal_delayedwork_schedule(osal_delayedwork *work, int timeout);

/*
 * osal_delayedwork_cancel_sync() - cancel a delayed work and wait for it to finish.
 * @work: the delayed work cancel.
 * Support System: linux.
 */
int osal_delayedwork_cancel_sync(osal_delayedwork *work);

/**************************************** bitmap api ****************************************/
#define OSAL_BITS_PER_BYTE       8
#define OSAL_BITS_PER_LONG       (OSAL_BITS_PER_BYTE * sizeof(unsigned long))
#define OSAL_BIT_MASK(nr)        (1UL << ((nr) % OSAL_BITS_PER_LONG))
#define OSAL_BIT_WORD(nr)        ((nr) / OSAL_BITS_PER_LONG)
#define OSAL_DIV_ROUND_UP(n, d)   (((n) + (d) - 1) / (d))
#define OSAL_BITS_TO_LONGS(nr)   OSAL_DIV_ROUND_UP(nr, OSAL_BITS_PER_LONG)
#define OSAL_DECLARE_BITMAP(name, bits) unsigned long name[OSAL_BITS_TO_LONGS(bits)]
#define osal_for_each_set_bit(bit, addr, size) \
    for ((bit) = osal_bitmap_find_first_bit((addr), (size));        \
    (bit) < (size);                    \
    (bit) = osal_bitmap_find_next_bit((addr), (size), (bit) + 1))

#define osal_for_each_clear_bit(bit, addr, size) \
    for ((bit) = osal_bitmap_find_first_zero_bit((addr), (size));       \
    (bit) < (size);                   \
    (bit) = osal_bitmap_find_next_zero_bit((addr), (size), (bit) + 1))

/*
 * osal_bitmap_set_bit() - Used to set bit for bitmap.
 * @nr: bit number in bit array.
 * @addr: address of the bit array to be set.
 * Support System: linux.
 */
void osal_bitmap_set_bit(int nr, unsigned long *addr);

/*
 * osal_bitmap_clear_bit() - Used to clear bit for bitmap.
 * @nr: bit number in bit array.
 * @addr: address of the bit array to be clear.
 * Support System: linux.
 */
void osal_bitmap_clear_bit(int nr, unsigned long *addr);

/*
 * osal_bitmap_change_bit() - Used to change bit for bitmap.
 * @nr: bit number in bit array.
 * @addr: address of the bit array to be change.
 * Support System: linux.
 */
void osal_bitmap_change_bit(int nr, unsigned long *addr);

/*
 * osal_bitmap_test_bit() - Tests whether a given bit in an array of bits is set.
 * @nr: bit number in bit array.
 * @addr: address of the bit array to be change.
 * Support System: linux.
 */
int osal_bitmap_test_bit(int nr, unsigned long *addr);

/*
 * osal_bitmap_test_and_set_bit() - Tests whether a given bit in the bit array is set and then set to the bit.
 * @nr: bit number in bit array.
 * @addr: address of the bit array to be change.
 * Support System: linux.
 */
int osal_bitmap_test_and_set_bit(int nr, unsigned long *addr);

/*
 * osal_bitmap_test_and_clear_bit() - Tests whether a given bit in the bit array is set and then clear to the bit.
 * @nr: bit number in bit array.
 * @addr: address of the bit array to be change.
 * Support System: linux.
 */
int osal_bitmap_test_and_clear_bit(int nr, unsigned long *addr);

/*
 * osal_bitmap_test_and_change_bit() - Tests whether a given bit in the bit array is set and then change to the bit.
 * @nr: bit number in bit array.
 * @addr: address of the bit array to be change.
 * Support System: linux.
 */
int osal_bitmap_test_and_change_bit(int nr, unsigned long *addr);

/*
 * osal_bitmap_find_first_zero_bit() - find the first cleared bit in a memory region.
 * @name: The name to base the search on
 * @size: The bitmap size in bits
 * Support System: linux.
 */
int osal_bitmap_find_first_zero_bit(const unsigned long *name, unsigned size);

/*
 * osal_bitmap_find_first_bit() - find the first set bit in a memory region.
 * @name: The name to base the search on
 * @size: The bitmap size in bits
 * Support System: linux.
 */
int osal_bitmap_find_first_bit(const unsigned long *name, unsigned size);

/*
 * osal_bitmap_find_next_zero_bit() - find the next cleared bit in a memory region.
 * @name: The name to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 * Support System: linux.
 */
int osal_bitmap_find_next_zero_bit(const unsigned long *name, int size, int offset);

/*
 * osal_bitmap_find_next_bit() - find the next set bit in a memory region.
 * @name: The name to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 * Support System: linux.
 */
int osal_bitmap_find_next_bit(const unsigned long *name, unsigned siz, int offset);

/**************************************** timer api ****************************************/
typedef struct {
    void *timer;
    void (*handler)(td_uintptr_t);
    unsigned long data;     // data for handler
    unsigned int interval;  // timer timing duration, unit: ms.
} osal_timer;

typedef struct {
    long tv_sec;
    long tv_usec;
} osal_timeval;

/*
 * osal_timer_init() - Initialize the timer.
 * @timer: the timer to be initialized
 * Support System: linux liteos.
 */
int osal_timer_init(osal_timer *timer);

/*
 * osal_timer_start() - start a timer.
 * @timer: the timer to be added
 * The kernel will do a ->function(@timer) callback from the
 * timer interrupt at the ->expires point in the future. The
 * current time is 'jiffies'.
 *
 * The timer's ->expires, ->function fields must be set prior calling this
 * function.
 *
 * Timers with an ->expires field in the past will be executed in the next
 * timer tick.
 * Support System: linux liteos freertos.
 */
int  osal_timer_start(osal_timer *timer);

/*
 * osal_timer_mod() - modify a timer's timeout.
 * @timer: the timer to be modified
 * @interval: new timeout in jiffies
 * osal_timer_mod() is a more efficient way to update the expire field of an
 * active timer (if the timer is inactive it will be activated)
 *
 * Support System: linux liteos.
 */
int osal_timer_mod(osal_timer *timer, unsigned int interval);  // unit: ms
/*
 * osal_timer_stop() - deactivate a timer.
 * @timer: the timer to be deactivated
 * osal_timer_stop() deactivates a timer - this works on both active and inactive
 * timers.
 * Support System: linux liteos.
 */
int osal_timer_stop(osal_timer *timer);

/*
 * osal_timer_destroy() - destroy the timer.
 * @timer: the timer to be destroyed
 * Support System: linux liteos.
 */
int osal_timer_destroy(osal_timer *timer);

#ifdef CONFIG_NO_SUPPORT_OSAL
#define osal_msleep msleep_interruptible
#define osal_msleep_uninterruptible msleep
#define osal_udelay udelay
#define osal_mdelay mdelay
#define osal_jiffies_to_msecs jiffies_to_msecs
#define osal_msecs_to_jiffies msecs_to_jiffies
#else
/*
 * osal_msleep() - sleep waiting for signals.
 * @msecs: Time in milliseconds to sleep for
 * Support System: linux liteos.
 */
unsigned long osal_msleep(unsigned int msecs);

/*
 * osal_msleep_uninterruptible() - sleep safely even with waitqueue interruptions.
 * @msecs: Time in milliseconds to sleep for
 * Support System: linux liteos.
 */
void osal_msleep_uninterruptible(unsigned int msecs);

/*
 * osal_udelay() - Microsecond-level busy waiting.
 * @usecs: Time in microseconds to wait for
 * Support System: linux liteos.
 */
void osal_udelay(unsigned int usecs);

/*
 * osal_mdelay() - millisecond-level busy waiting.
 * @msecs: Time in milliseconds to wait for
 * Support System: linux liteos.
 */
void osal_mdelay(unsigned int msecs);
/*
 * osal_jiffies_to_msecs() - jiffies to msecs.
 * Support System: linux.
 */
unsigned int osal_jiffies_to_msecs(const unsigned int n);
/*
 * osal_msecs_to_jiffies() - msecs to jiffies.
 * Support System: linux.
 */
unsigned long osal_msecs_to_jiffies(const unsigned int m);
#endif

/*
 * Support System: noneos.
 */
void osal_timer_udelay(unsigned int usecs);

/*
 * osal_get_jiffies() - returns jiffies.
 * Support System: linux liteos.
 */
unsigned long long osal_get_jiffies(void);

/*
 * osal_get_cycle_per_tick() - This API is used to obtain the number of cycles in one tick.
 * Support System: liteos.
 */
unsigned int osal_get_cycle_per_tick(void);

/*
 * osal_get_timeofday() - Obtaining the Current System Kernel Time.
 * Support System: linux liteos.
 */
void osal_gettimeofday(osal_timeval *tv);

typedef struct {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} osal_rtc_time;

/*
 * osal_rtc_time_to_tm() - Calculate the values such as the year, month, and day in the time to the tm.
 * Support System: linux liteos.
 */
void osal_rtc_time_to_tm(unsigned long time, osal_rtc_time *tm);

/****************************************task api**********************************/
#if defined(__LITEOS__)
#define OSAL_TASK_PRIORITY_HIGH 3
#define OSAL_TASK_PRIORITY_MIDDLE 6
#define OSAL_TASK_PRIORITY_LOW 10
#elif defined(__KERNEL__)
#define OSAL_TASK_PRIORITY_HIGH 99
#define OSAL_TASK_PRIORITY_MIDDLE 50
#define OSAL_TASK_PRIORITY_LOW 10
#else
#define OSAL_TASK_PRIORITY_HIGH 0
#define OSAL_TASK_PRIORITY_MIDDLE 3
#define OSAL_TASK_PRIORITY_LOW 6
#endif

typedef int (*osal_kthread_handler)(void *data);

#ifdef CONFIG_NO_SUPPORT_OSAL
typedef struct task_struct osal_task;
#else
typedef struct {
    void *task;
} osal_task;
#endif

osal_task *osal_kthread_create(osal_kthread_handler handler, void *data,
    const char *name, unsigned int stack_size); // 0 mean default
int osal_kthread_set_priority(osal_task *task, unsigned int priority);
void osal_kthread_set_affinity(osal_task *task, int cpu_mask);
int osal_kthread_should_stop(void);
void osal_kthread_lock(void);
void osal_kthread_unlock(void);
void osal_kthread_destroy(osal_task *task, unsigned int stop_flag);

long osal_get_current_tid(void);

/***********************************interrupt api********************************/
#define OSAL_CPU_ALL 0
#define OSAL_CPU_0 (1<<1)
#define OSAL_CPU_1 (1<<2)
#define OSAL_CPU_2 (1<<3)
#define OSAL_CPU_3 (1<<4)

typedef struct {
    void *tasklet;
    void (*handler)(unsigned long data);
    unsigned long data;
} osal_tasklet;

typedef int (*osal_irq_handler)(int, void *);

int osal_irq_request(unsigned int irq,
    osal_irq_handler irq_handler,
    osal_irq_handler thread_handler,
    const char *name,
    void *dev);
void osal_irq_free(unsigned int irq, void *dev);

/*
 * Support System: liteos.
 */
int osal_irq_active(void);

/*
 * Support System: liteos.
 */
int osal_irq_set_priority(unsigned int irq, unsigned short priority);
int osal_irq_set_affinity(unsigned int irq, const char *name, int cpu_mask);
/*
 * Support System: liteos freertos.
 */
void osal_irq_enable(unsigned int irq);
/*
 * Support System: liteos freertos.
 */
void osal_irq_disable(unsigned int irq);
unsigned int osal_irq_lock(void);
unsigned int osal_irq_unlock(void);
void osal_irq_restore(unsigned int int_save);
#ifdef CONFIG_NO_SUPPORT_OSAL
#define osal_in_interrupt in_interrupt
#else
int osal_in_interrupt(void);
#endif
td_bool osal_irq_is_pending(td_u32 vector);
unsigned int osal_irq_clear(unsigned int vector);


int osal_tasklet_init(osal_tasklet *tasklet);
int osal_tasklet_schedule(osal_tasklet *tasklet);
int osal_tasklet_kill(osal_tasklet *tasklet);
int osal_tasklet_update(osal_tasklet *tasklet);
/*******************************osal addr*******************************/
#ifdef OSAL_API_SUPPORT_MEMMAP

typedef enum {
    OSAL_BLOCKMEM_VALID = 0,
    OSAL_BLOCKMEM_INVALID_PHYADDR = 1,
    OSAL_BLOCKMEM_INVALID_SIZE = 2,

    OSAL_BLOCKMEM_MAX,
} osal_blockmem_status;

osal_blockmem_status osal_blockmem_get_status(unsigned long phyaddr, unsigned int size);
void *osal_blockmem_vmap(unsigned long phys_addr, unsigned long size);
void osal_blockmem_vunmap(const void *virt_addr);

void *osal_ioremap(unsigned long phys_addr, unsigned long size);
void *osal_ioremap_nocache(unsigned long phys_addr, unsigned long size);
void *osal_ioremap_cached(unsigned long phys_addr, unsigned long size);
void osal_iounmap(void *addr, unsigned long size);
void *osal_ioremap_wc(unsigned long phys_addr, unsigned long size);
#endif

#define osal_readl(x) (*((volatile unsigned int *)(x)))
#define osal_writel(v, x) (*((volatile unsigned int *)(x)) = (v))

#define OSAL_VERIFY_READ   0
#define OSAL_VERIFY_WRITE  1

/***************************************cache****************************************/
#ifdef OSAL_API_SUPPORT_DCACHE
void osal_dcache_region_wb(void *kvirt, unsigned long phys_addr, unsigned long size);
void osal_dcache_region_inv(void *addr, unsigned long size);
void osal_dcache_region_clean(void *addr, unsigned int size);
void osal_arch_dcache_flush_by_va(void *baseAddr, unsigned int size);
void osal_arch_dcahce_inv_by_va(void *baseAddr, unsigned int size);

void osal_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long size);
#endif

/***************************************math*****************************************/
unsigned long long osal_div_u64(unsigned long long dividend, unsigned int divisor);
long long osal_div_s64(long long dividend, int divisor);
unsigned long long osal_div64_u64(unsigned long long dividend, unsigned long long divisor);
long long osal_div64_s64(long long dividend, long long divisor);
unsigned long long osal_div_u64_rem(unsigned long long dividend, unsigned int divisor);
long long osal_div_s64_rem(long long dividend, int divisor);
unsigned long long osal_div64_u64_rem(unsigned long long dividend, unsigned long long divisor);
unsigned int osal_get_random_int(void);
#define osal_max(x, y)                 \
    ({                                 \
        __typeof__(x)_max1 = (x);      \
        __typeof__(y)_max2 = (y);      \
        (void)(&_max1 == &_max2);      \
        _max1 > _max2 ? _max1 : _max2; \
    })

#define osal_min(x, y)                 \
    ({                                 \
        __typeof__(x)_min1 = (x);      \
        __typeof__(y)_min2 = (y);      \
        (void)(&_min1 == &_min2);      \
        _min1 < _min2 ? _min1 : _min2; \
    })

#define osal_abs(x)                           \
    ({                                        \
        long ret;                             \
        if (sizeof(x) == sizeof(long)) {      \
            long __x = (x);                   \
            ret = (__x < 0) ? (-__x) : (__x); \
        } else {                              \
            int __x = (x);                    \
            ret = (__x < 0) ? (-__x) : (__x); \
        }                                     \
        ret;                                  \
    })

/* ******************************************barrier******************************** */
void osal_mb(void);
void osal_rmb(void);
void osal_wmb(void);
void osal_smp_mb(void);
void osal_smp_rmb(void);
void osal_smp_wmb(void);
void osal_isb(void);
void osal_dsb(void);
void osal_dmb(void);

/* *************************************** Proc Api ******************************** */
#define OSAL_PROC_NAME_LENGTH 32
#define PROC_CMD_SINGEL_LENGTH_MAX 32
#define PROC_CMD_ALL_LENGTH_MAX 128
#define PROC_CMD_NUM_MAX 16

typedef struct osal_proc_cmd_ {
    char name[OSAL_PROC_NAME_LENGTH];
    int (*handler)(unsigned int argc, char (*argv)[PROC_CMD_SINGEL_LENGTH_MAX], void *private_data);
} osal_proc_cmd;

typedef struct osal_proc_dir_entry {
    char name[OSAL_PROC_NAME_LENGTH];
    unsigned int cmd_cnt;
    osal_proc_cmd *cmd_list;
    void *proc_dir_entry;
    int (*open)(struct osal_proc_dir_entry *entry);
    int (*read)(struct osal_proc_dir_entry *entry);
    int (*write)(struct osal_proc_dir_entry *entry, const char *buf, int count, long long *);
    void *private_data;
    void *seqfile;
    struct osal_list_head node;
} osal_proc_entry;
void osal_proc_init(const char *name);
void osal_proc_exit(const char *name);
void osal_remove_proc_entry(const char *name, osal_proc_entry *parent);
osal_proc_entry *osal_create_proc_entry(const char *name, osal_proc_entry *parent);

#if defined(__LITEOS__)
#define osal_seq_printf(seqfile, fmt, ...) seq_printf(seqfile, fmt, ##__VA_ARGS__)
#else
void osal_seq_printf(void *seqfile, const char *fmt, ...);
#endif

/* *************************************** device api************************************** */
#define OSAL_POLLIN 0x0001U
#define OSAL_POLLPRI 0x0002U
#define OSAL_POLLOUT 0x0004U
#define OSAL_POLLERR 0x0008U
#define OSAL_POLLHUP 0x0010U
#define OSAL_POLLNVAL 0x0020U
#define OSAL_POLLRDNORM 0x0040U
#define OSAL_POLLRDBAND 0x0080U
#define OSAL_POLLWRNORM 0x0100U

typedef struct osal_poll_ {
    void *poll_table;
    void *data;
} osal_poll;

typedef struct osal_ioctl_cmd_ {
    unsigned int cmd;
    int (*handler)(unsigned int cmd, void *arg, void *private_data);
} osal_ioctl_cmd;

typedef struct osal_vm_ {
    void *vm;
} osal_vm;

typedef struct osal_fileops_ {
    int (*open)(void *private_data);
    int (*read)(char *buf, int size, long *offset, void *private_data);
    int (*write)(const char *buf, int size, long *offset, void *private_data);
    long (*llseek)(long offset, int whence, void *private_data);
    int (*release)(void *private_data);
    int (*poll)(osal_poll *osal_poll, void *private_data);
    int (*mmap)(osal_vm *vm, unsigned long start, unsigned long end, unsigned long vm_pgoff, void *private_data);
    int (*fasync)(int fd, void *filp, int mode);
    osal_ioctl_cmd *cmd_list;
    unsigned int cmd_cnt;
} osal_fileops;

typedef struct osal_pmops_ {
    int (*pm_suspend)(void *private_data);
    int (*pm_resume_early)(void *private_data);
    int (*pm_resume)(void *private_data);
    int (*pm_lowpower_enter)(void *private_data);
    int (*pm_lowpower_exit)(void *private_data);
    int (*pm_poweroff)(void *private_data);
    void *private_data;
} osal_pmops;

#define OSAL_DEV_NAME_LEN 32
typedef struct osal_dev_ {
    char name[OSAL_DEV_NAME_LEN];
    int minor;
    unsigned int parent_minor;
    osal_fileops *fops;
    osal_pmops *pmops;
    void *dev;
} osal_dev;

#define OSAL_NOCACHE 0
#define OSAL_CACHE 1

typedef enum osal_kobject_action_ {
    OSAL_KOBJ_ADD,
    OSAL_KOBJ_REMOVE,
    OSAL_KOBJ_CHANGE,
    OSAL_KOBJ_MOVE,
    OSAL_KOBJ_ONLINE,
    OSAL_KOBJ_OFFLINE,
    OSAL_KOBJ_BIND,
    OSAL_KOBJ_UNBIND,
    OSAL_KOBJ_MAX
} osal_kobject_action;

void osal_pm_lowpower_enter(void);
void osal_pm_lowpower_exit(void);
osal_dev *osal_dev_create(const char *name);
int osal_dev_destroy(osal_dev *dev);
int osal_dev_register(osal_dev *dev);
void osal_dev_unregister(osal_dev *dev);
void osal_device_set_async(unsigned int minor);
void osal_poll_wait(osal_poll *table, osal_wait *wait);
int osal_remap_pfn_range(osal_vm *vm, unsigned long addr, unsigned long pfn, unsigned long size);
int osal_try_to_freeze(void);
int osal_set_freezable(void);
int osal_kobject_uevent_env(osal_dev *dev, osal_kobject_action action, char *envp[]);
int osal_fasync_helper(int fd, void *filp, int mode, void **fapp);
void osal_fasync_notify(void **fapp, int sig, int band);
void osal_pgprot_noncached(osal_vm *vm);
void osal_pgprot_cached(osal_vm *vm);
void osal_pgprot_writecombine(osal_vm *vm);

typedef struct {
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
    unsigned long a4;
    unsigned long a5;
    unsigned long a6;
    unsigned long a7;
} osal_smccc_info;

typedef struct {
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
} osal_smccc_res;

void osal_smccc_smc(const osal_smccc_info *info, osal_smccc_res *res);


/***************************************kfile api***************************************/
#define    OSAL_O_RDONLY    00000000
#define    OSAL_O_WRONLY    00000001
#define    OSAL_O_RDWR      00000002
#define    OSAL_O_ACCMODE   00000003

#define    OSAL_O_CREAT     00000100
#define    OSAL_O_EXCL      00000200
#define    OSAL_O_TRUNC     00001000
#define    OSAL_O_APPEND    00002000
#define    OSAL_O_CLOEXEC   02000000

#define    OSAL_SEEK_SET    0
#define    OSAL_SEEK_CUR    1
#define    OSAL_SEEK_END    2

void *osal_klib_fopen(const char *file, int flags, int mode);
void osal_klib_fclose(void *filp);
int osal_klib_fwrite(const char *buf, unsigned long size, void *filp);
int osal_klib_fread(char *buf, unsigned long size, void *filp);
void osal_klib_fsync(void *filp);
int osal_klib_fseek(long long offset, int whence, void *filp);
/*
 * Sopurt System: seliteos
 */
int osal_klib_unlink(const char *path);
/*
 * Sopurt System: seliteos
 */
int osal_klib_ftruncate(void *filp, unsigned long len);
int osal_klib_get_store_path(char *path, unsigned int path_size);
void osal_klib_set_store_path(char *path);

/***************************************string***************************************/
#define OSAL_BASE_DEC 10
#define OSAL_BASE_HEX 16

#ifdef CONFIG_NO_SUPPORT_OSAL
static inline int osal_memncmp(const void *buf1, unsigned long size1, const void *buf2, unsigned long size2)
{
    return memcmp(buf1, buf2, size1);
}
#define osal_memncmp_tmp_new memcmp
#define osal_strcmp strcmp
static inline int osal_strncmp(const char *s1, unsigned long size1, const char *s2, unsigned long size2)
{
    return strncmp(s1, s2, size1);
}
#define osal_strncmp_tmp_new strncmp
#define osal_strcasecmp strcasecmp
static inline int osal_strncasecmp(const char *s1, unsigned long size1, const char *s2, unsigned long size2)
{
    return strncasecmp(s1, s2, size1);
}
#define osal_strncasecmp_tmp_new strncasecmp
#define osal_strchr strchr
#define osal_strnchr strnchr
#define osal_strrchr strrchr
#define osal_strstr strstr
#define osal_strnstr strnstr
#define osal_strlen strlen
#define osal_strnlen strnlen
#define osal_strpbrk strpbrk
#define osal_strsep strsep
#define osal_strspn strspn
#define osal_strcspn strcspn
#define osal_memscan memscan
#define osal_memcmp memcmp
#define osal_memchr memchr
#define osal_memchr_inv memchr_inv
#define osal_strtoull simple_strtoull
#define osal_strtoul simple_strtoul
#define osal_strtol simple_strtol
#define osal_strtoll simple_strtoll
#else
int osal_memncmp(const void *buf1, unsigned long size1, const void *buf2, unsigned long size2);
int osal_memncmp_tmp_new(const void *buf1, const void *buf2, unsigned long size);
int osal_strcmp(const char *s1, const char *s2);
#if !defined(_PRE_WLAN_FEATURE_WS63) && !defined(UT_TEST) && !defined(_PRE_WLAN_FEATURE_WS92_MERGE)
int osal_strncmp(const char *s1, unsigned long size1, const char *s2, unsigned long size2);
#else
#define osal_strncmp osal_strncmp_tmp_new
#endif
int osal_strncmp_tmp_new(const char *str1, const char *str2, unsigned long size);
int osal_strcasecmp(const char *s1, const char *s2);
int osal_strncasecmp(const char *s1, unsigned long size1, const char *s2, unsigned long size2);
int osal_strncasecmp_tmp_new(const char *str1, const char *str2, unsigned long size);
char *osal_strchr(const char *s, int n);
char *osal_strnchr(const char *s, int count, int c);
char *osal_strrchr(const char *s, int c);
char *osal_strstr(const char *s1, const char *s2);
char *osal_strnstr(const char *s1, const char *s2, int n);
int osal_strlen(const char *s);
int osal_strnlen(const char *s, int size);
char *osal_strpbrk(const char *s1, const char *s2);
char *osal_strsep(char **s, const char *ct);
int osal_strspn(const char *s, const char *accept);
int osal_strcspn(const char *s, const char *reject);
void *osal_memscan(void *addr, int c, int size);
int osal_memcmp(const void *cs, const void *ct, int count);
void *osal_memchr(const void *s, int c, int n);
void *osal_memchr_inv(const void *s, int c, int n);
unsigned long long osal_strtoull(const char *cp, char **endp, unsigned int base);
unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base);
long osal_strtol(const char *cp, char **endp, unsigned int base);
long long osal_strtoll(const char *cp, char **endp, unsigned int base);
#endif

/* **************************************drivermgr api************************************** */
/*
 * Sopurt System: seliteos
 */
unsigned int osal_drvmgr_switch_to_drvbox(unsigned int drv_id);
/*
 * Sopurt System: seliteos
 */
void osal_drvmgr_switch_to_usrbox(void);

/* **************************************debug************************************** */
#ifdef OSAL_DEBUG
#define OSAL_BUG() \
    do {           \
    } while (1)
#endif

#define OSAL_ASSERT(expr)                         \
    do {                                          \
        if (!(expr)) {                            \
            osal_printk("\nASSERT failed at:\n"   \
                        "  >Condition: %s\n",     \
                #expr);                           \
            OSAL_BUG();                           \
        }                                         \
    } while (0)


#define OSAL_BUG_ON(expr)                                                               \
    do {                                                                                \
        if (expr) {                                                                     \
            osal_printk("BUG: failure at %d/%s()!\n", __LINE__, __func__);              \
            OSAL_BUG();                                                                 \
        }                                                                               \
    } while (0)

#ifdef CONFIG_NO_SUPPORT_OSAL
#define osal_printk printk
#define osal_panic panic
#define osal_dump_stack dump_stack
#else
void osal_printk(const char *fmt, ...);
void osal_panic(const char *fmt, const char *fun, int line, const char *cond);
void osal_dump_stack(void);
#endif
void osal_bug_on(unsigned char condition);

/* bitmap */
#define OSAL_BITS_PER_BYTE       8
#define OSAL_BITS_PER_LONG       (OSAL_BITS_PER_BYTE * sizeof(unsigned long))
#define OSAL_BIT_MASK(nr)        (1UL << ((nr) % OSAL_BITS_PER_LONG))
#define OSAL_BIT_WORD(nr)        ((nr) / OSAL_BITS_PER_LONG)
#define OSAL_DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))
#define OSAL_BITS_TO_LONGS(nr) OSAL_DIV_ROUND_UP(nr, OSAL_BITS_PER_LONG)
#define OSAL_DECLARE_BITMAP(name, bits) unsigned long name[OSAL_BITS_TO_LONGS(bits)]

#ifdef __KERNEL__
#define OSAL_THIS_MODULE THIS_MODULE
#define osal_module_export(_symbol) EXPORT_SYMBOL(_symbol)
#define osal_module_param(name, type, perm) module_param(name, type, perm)
#define OSAL_MODULE_PARM_DESC(_parm, desc) MODULE_PARM_DESC(_perm, desc)
#define osal_module_init(x) module_init(x)
#define osal_module_exit(x) module_exit(x)
#define OSAL_MODULE_AUTHOR(_author) MODULE_AUTHOR(_author)
#define OSAL_MODULE_DESCRIPTION(_description) MODULE_DESCRIPTION(_description)
#define osal_module_license(_license) MODULE_LICENSE(_license)
#define osal_module_version(_version) MODULE_VERSION(_version)
#else
#define OSAL_THIS_MODULE
#define osal_module_export(_symbol)
#define osal_module_param(name, type, perm)
#define OSAL_MODULE_PARM_DESC(_parm, desc)
#define osal_module_init(x)
#define osal_module_exit(x)
#define OSAL_MODULE_AUTHOR(_author)
#define OSAL_MODULE_DESCRIPTION(_description)
#define osal_module_license(_license)
#define osal_module_version(_version)
#endif

/***************************************msg queue***************************************/
int osal_msg_queue_create(char *name, unsigned short queue_len, unsigned long *queue_id,
    unsigned int flags, unsigned short max_msgsize);
int osal_msg_queue_write_copy(unsigned long queue_id, void *buffer_addr,
    unsigned int buffer_size, unsigned int timeout);
int osal_msg_queue_read_copy(unsigned long queue_id, void *buffer_addr,
    unsigned int *buffer_size, unsigned int timeout);
int osal_msg_queue_write_head_copy(unsigned long queue_id, void *buffer_addr,
    unsigned int buffer_size, unsigned int timeout);
void osal_msg_queue_delete(unsigned long queue_id);
int osal_msg_queue_is_full(unsigned long queue_id);
unsigned int osal_msg_queue_get_msg_num(unsigned long queue_id);

#ifdef _PRE_WLAN_FEATURE_WS63
static inline unsigned long long uapi_get_time_us(void)
{
    return 0;
}

static inline unsigned int uapi_hrtimer_stop(unsigned int hrtimer_handle)
{
    hrtimer_handle = hrtimer_handle;
    return 0;
}

static inline unsigned int uapi_hrtimer_start(unsigned int hrtimer_handle, unsigned int expire,
    void* hrtimer_func, unsigned int data)
{
    hrtimer_handle = hrtimer_handle;
    expire = expire;
    hrtimer_func = hrtimer_func;
    data = data;
    return 0;
}

static inline unsigned int uapi_hrtimer_create(unsigned int *hrtimer_handle)
{
    hrtimer_handle = hrtimer_handle;
    return 0;
}

static inline unsigned int uapi_hrtimer_delete(unsigned int hrtimer_handle)
{
    hrtimer_handle = hrtimer_handle;
    return 0;
}

static inline void uapi_watchdog_kick(void)
{
    return;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __SOC_OSAL_H__ */
