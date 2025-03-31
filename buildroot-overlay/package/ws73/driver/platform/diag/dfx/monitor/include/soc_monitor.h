/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: soc monitor
 * This file should be changed only infrequently and with great care.
 */
#ifndef __SOC_MONITOR_H__
#define __SOC_MONITOR_H__
#include "td_type.h"
#include "td_base.h"
#include "soc_errno.h"

#define EXT_MONITOR_MAX_NAME_CNT 12  /* 名称字符串长度限制 */
#define EXT_MONITOR_INVALID_HANDLE 0 /* 非法句柄值定义 */
#define DFX_MONITOR_INVAL_HANDLE 0

typedef td_u32 monitor_handle; /* monitor句柄 */

/*
描述：获取值用户回调函数
参数 name：监控事件名称
参数 usr_data：用户参数
返回值：当前值
*/
typedef td_u32 (*monitor_get_val_handler)(td_char *name, uintptr_t usr_data);

/*
描述：处理事件用户回调函数
参数 name：监控事件名称
参数 usr_data：用户参数
*/
typedef td_void (*monitor_process_handler)(td_char *name, uintptr_t usr_data);

/* 值不变事件的入参结构 */
typedef struct {
    char *name;                      /* 监控事件名称，不允许重复 */
    uintptr_t usr_data;              /* 用户参数 */
    td_u32 unchange_sec;             /* 值多长时间不发生变化，上报用户函数处理 */
    monitor_get_val_handler get_val; /* 获取值回调函数 */
    monitor_process_handler process; /* 通知用户处理该事件 */
} ext_monitor_unchange_param;

/*
描述:注册值不变监控事件
参数 handle:返回的handle
param:事件参数
返回值:成功 和 其它错误
*/
ext_errno uapi_monitor_register_unchange_item(monitor_handle *handle, ext_monitor_unchange_param *param);

/*
描述:去注册监控事件
参数 handle:事件handle
返回值:成功 和 其它错误
*/
ext_errno uapi_monitor_unregister(monitor_handle handle);
#endif
