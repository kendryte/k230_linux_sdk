/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: time adpt head file.
 */

#ifndef __OAL_TIME_H__
#define __OAL_TIME_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/rtc.h>
#include "oal_util.h"
#include "oal_mm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/* 获取毫秒级时间戳 */
#define oal_time_get_stamp_ms() osal_jiffies_to_msecs(osal_get_jiffies())

/* 获取高精度毫秒时间戳,精度1ms */
#define OAL_TIME_GET_HIGH_PRECISION_MS()  oal_get_time_stamp_from_timeval()


#define OAL_TIME_JIFFY    osal_get_jiffies()

#define OAL_TIME_HZ       HZ

#define oal_msecs_to_jiffies(_msecs)    osal_msecs_to_jiffies(_msecs)

#define oal_jiffies_to_msecs(_jiffies)      osal_jiffies_to_msecs(_jiffies)

#define OAL_GET_REAL_TIME(_pst_tm)          oal_get_real_time(_pst_tm)

/*****************************************************************************
  STRUCT定义
*****************************************************************************/
/* 与内核struct rtc_time 保持一致 */
typedef struct _oal_time_stru {
    osal_s32 tm_sec; /* seconds */
    osal_s32 tm_min; /* minutes */
    osal_s32 tm_hour; /* hours */
    osal_s32 tm_mday; /* day of the month */
    osal_s32 tm_mon; /* month */
    osal_s32 tm_year; /* year */
    osal_s32 tm_wday; /* day of the week */
    osal_s32 tm_yday; /* day in the year */
    osal_s32 tm_isdst; /* daylight saving time */
} oal_time_stru;

typedef struct {
    osal_slong i_sec;
    osal_slong i_usec;
} oal_time_us_stru;

typedef ktime_t oal_time_t_stru;
/*****************************************************************************
  函数声明
*****************************************************************************/
/*****************************************************************************
 功能描述  : 获取微妙精度级的时间戳
 输入参数  : usec: 时间结构体指针
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  oal_time_get_stamp_us(oal_time_us_stru *usec)
{
    osal_timeval ts;
    osal_gettimeofday(&ts);
    usec->i_sec  = ts.tv_sec;
    usec->i_usec = ts.tv_usec;
}

OAL_STATIC OAL_INLINE osal_u32 oal_get_seconds(osal_void)
{
    osal_timeval tv;
    osal_gettimeofday(&tv);
    return tv.tv_sec;
}

OAL_STATIC OAL_INLINE osal_u64 oal_get_ms(osal_void)
{
    osal_timeval tv;
    osal_gettimeofday(&tv);
    /* ms 单位1000 */
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

OAL_STATIC OAL_INLINE osal_u64 oal_get_us(oal_time_us_stru *usec)
{
    osal_timeval tv;
    osal_gettimeofday(&tv);
    usec->i_sec = tv.tv_sec;
    usec->i_usec =  tv.tv_usec;
    /* us 单位1000000 */
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

/*****************************************************************************
 功能描述  : 调用内核函数获取当前时间戳
 输入参数  : osal_void
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_time_t_stru oal_ktime_get(osal_void)
{
    return ktime_get();
}

/*****************************************************************************
 功能描述  : 调用内核函数获取时间差值
 输入参数  : const oal_time_t_stru lhs,
             const oal_time_t_stru rhs
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_time_t_stru oal_ktime_sub(const oal_time_t_stru lhs, const oal_time_t_stru rhs)
{
    return ktime_sub(lhs, rhs);
}

/*****************************************************************************
 功能描述  : 判断时间戳ul_time_a是否在ul_time_b之后:
 输入参数  : osal_ulong time
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32 oal_time_after(osal_ulong time_a, osal_ulong time_b)
{
    return (osal_u32)time_after(time_a, time_b);
}

/*****************************************************************************
 功能描述  : 判断ul_time是否比当前时间早
             若早，表示超时时间已过；若不早，表明还未超时
 输入参数  : osal_ulong time
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32 oal_time_is_before(osal_ulong time)
{
    return (osal_u32)time_is_before_jiffies(time);
}

OAL_INLINE static char* oal_get_current_task_name(osal_void)
{
    return current->comm;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_time.h */

