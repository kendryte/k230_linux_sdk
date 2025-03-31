/*
 * Copyright (c) @CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: frw线程实现头文件.
 * Author: Huanghe
 * Create: 2020-07-08
 */

#ifndef __FRW_THREAD_H__
#define __FRW_THREAD_H__

#include "osal_list.h"
#include "frw_msg_rom.h"
#include "osal_types.h"
#include "osal_adapt.h"
#include "oal_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_THREAD_H

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define FRW_THREAD_TASK_PRIORITY 4     /* liteos支持32个优先级[0-31]，最高优先级为0，最低优先级为31 */
#define FRW_TXDATA_TASK_PRIORITY 4
#else
#if defined(_PRE_WLAN_FEATURE_WS73) && defined(WSCFG_BUS_SDIO)
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
#define FRW_THREAD_TASK_PRIORITY 50
#define FRW_TXDATA_TASK_PRIORITY 50
#else
#define FRW_THREAD_TASK_PRIORITY 10    /* 该优先级不能比内核的sdio_rx(内核默认FIFO 50)优先级高 */
#define FRW_TXDATA_TASK_PRIORITY 9     /* 数据帧线程优先级比消息线程低 */
#endif // end of (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
#else
#define FRW_THREAD_TASK_PRIORITY 99    /* 83和86的frw任务优先级待测试后进一步优化 */
#define FRW_TXDATA_TASK_PRIORITY 98    /* 数据帧线程优先级比消息线程低 */
#endif
#endif

#define FRW_MSG_TASK_STACK_SIZE 0x1400
#define FRW_TX_DATA_TASK_STACK_SIZE 0x1000

osal_s32 frw_thread_init(osal_void);
osal_s32 frw_thread_exit(osal_bool thread_stop);
osal_void frw_thread_dump(osal_void);
osal_s32 frw_host_post_msg_async(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id, frw_msg *msg);
osal_s32 frw_host_post_msg_sync(osal_u16 msg_id, osal_u8 vap_id, osal_u16 time_out, frw_msg *msg);
osal_s32 frw_send_cfg_to_device_sync(osal_u16 msg_id, osal_u8 vap_id, osal_u16 time_out, frw_msg *msg);
osal_s32 frw_msg_check_frw_deadlock(osal_s32 msg_id);
osal_s32 frw_msg_check_hcc_deadlock(osal_s32 msg_id);
osal_s32 frw_host_post_msg(osal_u16 msg_id, frw_post_pri_enum_uint8 pri, osal_u8 vap_id, frw_msg *msg);
osal_u32 frw_pm_queue_empty_check(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of frw_thread.h */
