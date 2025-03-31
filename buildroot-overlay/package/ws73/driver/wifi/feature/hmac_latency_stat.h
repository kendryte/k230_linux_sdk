/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Latency Stat hmac function.
 * Create: 2021-08-20
 */

#ifndef __HMAC_LATENCY_STAT_H__
#define __HMAC_LATENCY_STAT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "wlan_types_common.h"
#include "mac_vap_ext.h"
#include "msg_latency_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_LATENCY_STAT_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define latency_report_has_tx(direct)      ((direct) == (LATENCY_RPT_DIR_TX) || (direct) == (LATENCY_RPT_DIR_TXRX))
#define latency_report_has_rx(direct)      ((direct) == (LATENCY_RPT_DIR_RX) || (direct) == (LATENCY_RPT_DIR_TXRX))

#define LATENCY_STAT_SWITCH_ON          ((hmac_latency_get_switch_state()) == OAL_TRUE)
#define LATENCY_NEED_REPORT_TX          (latency_report_has_tx(hmac_latency_get_report_direct()))
#define LATENCY_NEED_REPORT_RX          (latency_report_has_rx(hmac_latency_get_report_direct()))
#define LATENCY_TX_STORAGE_EXIST        (hmac_latency_get_storage_tx() != OSAL_NULL)
#define LATENCY_RX_STORAGE_EXIST        (hmac_latency_get_storage_rx() != OSAL_NULL)

#define LATENCY_TIMESTAMP_MASK 0xffffff
#define LATENCY_STATS_NUM_TIME_SLOTS 6
#define LATENCY_STATS_TIME_SLOT_1 50
#define LATENCY_STATS_TIME_SLOT_2 200
#define LATENCY_STATS_TIME_SLOT_3 1000
#define LATENCY_STATS_TIME_SLOT_4 10000
#define LATENCY_STATS_TIME_SLOT_5 40000

#define STAT_TIME_REG_MAX 0xFFFFFF /* 24位统计寄存器最大长度 */
static inline osal_u32 time_revert(osal_u32 start, osal_u32 end)
{
    return (STAT_TIME_REG_MAX - start + end + 1); /* 时间反转计算 */
}
static inline osal_u32 get_runtime(osal_u32 start, osal_u32 end)
{
    return (start > end) ? time_revert(start, end) : (end - start);
}
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum latency_proc_parts_tx_hmac {
    TX_HMAC_PROC_AMSDU = 0,
    TX_HMAC_TID_QUEUE  = 1,

    TX_HMAC_NUM_PARTS
};
typedef osal_u8 latency_tx_parts_enum_uint8;

enum latency_proc_parts_rx_hmac {
    RX_HMAC_KO_PROC    = 0,
    RX_TRANS_TO_LAN    = 1,

    RX_HMAC_NUM_PARTS
};
typedef osal_u8 latency_rx_parts_enum_uint8;
/*****************************************************************************
  10 函数声明
*****************************************************************************/
typedef osal_void (*hmac_latency_stat_tx_entry_cb)(oal_netbuf_stru *buf);
typedef osal_u32 (*hmac_record_latency_tx_cb)(oal_netbuf_stru *buf, latency_tx_parts_enum_uint8 part);
typedef osal_void (*hmac_latency_stat_rx_entry_cb)(oal_netbuf_stru *buf);
typedef osal_void (*hmac_record_latency_rx_cb)(oal_netbuf_stru *buf, latency_rx_parts_enum_uint8 part);
static osal_u32 hmac_latency_stat_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_latency_stat_init"), used));
static osal_void hmac_latency_stat_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_latency_stat_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __HMAC_LATENCY_STAT_H__ */