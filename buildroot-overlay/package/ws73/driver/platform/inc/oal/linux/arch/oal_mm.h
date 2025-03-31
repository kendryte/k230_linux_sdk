/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: oal mm header
 * Author: Huanghe
 * Create: 2023-01-06
 */

#ifndef __OAL_LINUX_MM_H__
#define __OAL_LINUX_MM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

OAL_STATIC OAL_INLINE osal_void* oal_memtry_alloc(oal_uint32 request_maxsize, oal_uint32 request_minsize,
    oal_uint32* actual_size)
{
    oal_uint32  l_flags = 0;
    oal_void   *puc_mem_space;
    oal_uint32  request_size;

    if (WARN_ON(NULL == actual_size)) {
        return NULL;
    }

    *actual_size = 0;

    /* 不睡眠或在中断程序中标志置为GFP_ATOMIC */
    if ((in_interrupt() != 0) || (irqs_disabled() != 0)) {
        l_flags |= GFP_ATOMIC;
    } else {
        l_flags |= GFP_KERNEL;
    }

    l_flags |= __GFP_NOWARN;

    request_size = ((request_maxsize >= request_minsize)? request_maxsize : request_minsize);

    while (request_size > 0) {
        if (request_size <= request_minsize) {
            l_flags &= ~__GFP_NOWARN;
        }
        puc_mem_space = kmalloc(request_size, l_flags);
        if (NULL != puc_mem_space) {
            *actual_size = request_size;
            return puc_mem_space;
        }

        if (request_size <= request_minsize) {
            /* 以最小SIZE申请依然失败返回NULL */
            break;
        }

        /* 申请失败, 折半重新申请 */
        request_size = request_size >> 1;
        request_size = ((request_size >= request_minsize)? request_size : request_minsize);
    }

    return NULL;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_mm.h */

