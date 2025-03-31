/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: time 适配头文件.
 */

#ifndef __OAL_SCHEDULE_HCM_H__
#define __OAL_SCHEDULE_HCM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_mm.h"
#include "oal_list.h"
#include "oal_spinlock.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define OAL_TIME_US_MAX_LEN  0xFFFFFFFF

typedef enum {
    OAL_DFT_TRACE_OTHER = 0x0,
    OAL_DFT_TRACE_SUCC,
    OAL_DFT_TRACE_FAIL, /* > fail while dump the key_info to kernel */
    OAL_DFT_TRACE_EXCEP, /* exception */
    OAL_DFT_TRACE_BUTT,
} oal_dft_type;

/* 获取从_ul_start到_ul_end的时间差 */
OAL_STATIC OAL_INLINE osal_u32 oal_time_get_runtime(osal_u32 start, osal_u32 end)
{
    return ((start > end) ? (OAL_TIME_US_MAX_LEN - start + end + 1) : (end - start));
}

struct osal_list_head *oal_get_wakelock_head(void);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_schedule.h */
