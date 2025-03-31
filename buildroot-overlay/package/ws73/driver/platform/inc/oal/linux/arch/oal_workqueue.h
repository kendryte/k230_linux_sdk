/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: oal workqueue header file.
 * Author: Huanghe
 * Create: 2023-01-06
 */

#ifndef __OAL_WORKQUEUE_H__
#define __OAL_WORKQUEUE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct workqueue_struct          oal_workqueue_stru;
typedef struct work_struct               oal_work_stru;

OAL_STATIC OAL_INLINE oal_workqueue_stru*  oal_create_singlethread_workqueue(
                                                                    const oal_int8 *pc_workqueue_name)
{
    return create_singlethread_workqueue(pc_workqueue_name);
}

OAL_STATIC OAL_INLINE oal_void  oal_destroy_workqueue(oal_workqueue_stru   *pst_workqueue)
{
    destroy_workqueue(pst_workqueue);
}

OAL_STATIC OAL_INLINE oal_int32 oal_work_is_busy(oal_work_stru *pst_work)
{
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    return work_busy(pst_work);
#else
    return work_pending(pst_work);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_workqueue.h */
