/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: oal_spinlock.h 的头文件
 * Create: 2022-4-27
 */

#ifndef __OAL_LINUX_SPINLOCK_H__
#define __OAL_LINUX_SPINLOCK_H__

#include <linux/sched.h>
#include <linux/spinlock.h>
#include "oal_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 函数指针，用来指向需要自旋锁保护的的函数 */
typedef osal_u32(*oal_irqlocked_func)(osal_void *);

#define OAL_SPIN_LOCK_MAGIC_TAG (0xdead4ead)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_spinlock.h */
