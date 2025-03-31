/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: netbuf enum type head file.
 * Create: 2022-07-04
 */

#ifndef FRW_EXT_COMMON_ROM_H
#define FRW_EXT_COMMON_ROM_H

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

typedef osal_u32 (*frw_timeout_func)(osal_void *);

typedef struct {
    osal_void                   *timeout_arg;      /* 超时处理函数入参 */
    frw_timeout_func            func;             /* 超时处理函数 */
    osal_u32                    time_stamp;      /* 定时器超时截止时间(启动时间+超时周期) */
    osal_u32                    timeout;         /* 过多长时间定时器超时 */
    oal_bool_enum_uint8         is_registerd;    /* 定时器是否已经注册 */
    oal_bool_enum_uint8         is_periodic;     /* 定时器是否为周期的 */
    oal_bool_enum_uint8         is_enabled;      /* 定时器是否使能 */
    osal_u8                     pad;
    osal_u16                    core_id;         /* 绑定的核id */
    osal_u16                    module_id;
    osal_u32                    func_p;          /* 创建定时器的地址 */
    struct osal_list_head       entry;           /* 定期器链表索引 */
} frw_timeout_stru;

typedef struct {
    osal_u32 succ_cnt;         // 执行成功次数
    osal_u32 fail_cnt;         // 执行失败次数
    osal_u32 d2d_msg_cnt;      // hal2dmac消息次数
    osal_u32 h2d_msg_cnt;      // host2device消息次数
    osal_u32 h2d_data_cnt;     // host2device netbuf次数
    osal_u32 d2h_msg_cnt;      // d2h消息个数
    osal_u32 d2h_data_cnt;     // d2hdata个数
    osal_u32 h2d_mem_fail_cnt; // h2d消息申请内存失败次数
    osal_u32 d2d_mem_fail_cnt; // hal2d消息失败次数
    osal_u32 d2h_mem_fail_cnt; // d2h 失败次数
    osal_u32 h2d_msg_not_found_cnt; // 消息找不到次数
    osal_u32 h2d_msg_type_error_cnt; // 消息类型错误次数
    osal_u32 d2h_msg_hcc_send_fail_cnt; // frw消息hcc发送失败次数
    osal_u32 d2h_data_hcc_send_fail_cnt; // d2h消息个数
} device_frw_stat;

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of oal_netbuf_common.h */