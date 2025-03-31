/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: wlan through put debug.
 */

#ifndef WLAN_THRUPUT_DEBUG_H
#define WLAN_THRUPUT_DEBUG_H

#include "oal_skbuff.h"

enum {
    STAT_T0 = 0,
    STAT_T1,
    STAT_T2,
    STAT_T3,
    STAT_T4,
    STAT_T5,
    STAT_T6,
    STAT_T7,
    STAT_T8,
    STAT_T9,
    STAT_T10,
    STAT_T11,
    STAT_T12,
    STAT_T13,
    STAT_T14,
    STAT_T15,
    STAT_T16,
    STAT_T17,
    STAT_T18
};

enum {
    DEV_STAT_T0 = 0,
    DEV_STAT_T1,
    DEV_STAT_T2,
    DEV_STAT_T3,
    DEV_STAT_T4,
    DEV_STAT_T5,
    DEV_STAT_T6,
};
#define param_unref(P)  ((P) = (P))

#ifdef TIMESTAMP_RECORD_DEBUG
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
#include "reg_table.h"

#define STAT_TIMER_32M_TICK_REG (TIMER2_REG_BASE + 4) /* 32M时钟定时器用于统计tick的寄存器 */
#define STAT_TIMER_32M_CTRL_REG (TIMER2_REG_BASE + 8) /* 32M时钟定时器控制寄存器 */
#define STAT_TIMER_CTRL_EN 0xc0 /* 32M时钟定时器控制寄存器使能配置位 */

#define STAMP_PRINT osal_printf
#else
#define STAMP_PRINT printk
#endif

#define STAT_TIME_REG_MAX (0xFFFFFFFF - 1) /* 32位统计寄存器最大长度 */
#define stat_time_revert(start, end) (((STAT_TIME_REG_MAX) - (start)) + (end)) /* 时间反转计算 */
#define stat_get_runtime(start, end) \
    ((((start) > (end))  ? (stat_time_revert((start), (end))) : ((end) - (start))) / 32)

#define STAT_CNT_TIME 5000000

static inline osal_u32 stat_get_timer_tick(osal_void)
{
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
    return uapi_reg_read_val32(STAT_TIMER_32M_TICK_REG);
#else
    return 0;
#endif
}
static inline osal_void stat_timer_enable(osal_void)
{
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
    uapi_reg_write(STAT_TIMER_32M_CTRL_REG, STAT_TIMER_CTRL_EN);
#endif
}

static inline osal_void stat_record_timestamp(oal_netbuf_stru *netbuf, osal_u32 index)
{
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
    if (index == 0) {
        netbuf->resv = 0x5; /* record flag */
    }
    netbuf->times[index] = stat_get_timer_tick();
#else
    param_unref(netbuf);
    param_unref(index);
#endif
}

static inline osal_void stat_record_tx_timestamp(oal_netbuf_stru *netbuf, osal_u32 index)
{
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION) && defined(TIME_STAMP_TX_DEBUG)
    stat_record_timestamp(netbuf, index);
#else
    param_unref(netbuf);
    param_unref(index);
#endif
}

static inline osal_void stat_record_rx_timestamp(oal_netbuf_stru *netbuf, osal_u32 index)
{
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION) && defined(TIME_STAMP_RX_DEBUG)
    stat_record_timestamp(netbuf, index);
#else
    param_unref(netbuf);
    param_unref(index);
#endif
}

static inline osal_void host_print_timestamp(oal_netbuf_stru *netbuf)
{
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
    osal_u32 curr_time = stat_get_timer_tick();

    if (netbuf->resv == 0x5) {  /* flag */
        STAMP_PRINT("%u-%u-%u-%u  %u-%u-%u-%u  %u-%u-%u-%u  %u-%u-%u-%u  %u-%u-%u-TL:%u\r\n",
            stat_get_runtime(netbuf->times[STAT_T1], netbuf->times[STAT_T0]),
            stat_get_runtime(netbuf->times[STAT_T2], netbuf->times[STAT_T1]),
            stat_get_runtime(netbuf->times[STAT_T3], netbuf->times[STAT_T2]),
            stat_get_runtime(netbuf->times[STAT_T4], netbuf->times[STAT_T3]),
            stat_get_runtime(netbuf->times[STAT_T5], netbuf->times[STAT_T4]),
            stat_get_runtime(netbuf->times[STAT_T6], netbuf->times[STAT_T5]),
            stat_get_runtime(netbuf->times[STAT_T7], netbuf->times[STAT_T6]),
            stat_get_runtime(netbuf->times[STAT_T8], netbuf->times[STAT_T7]),
            stat_get_runtime(netbuf->times[STAT_T9], netbuf->times[STAT_T8]),
            stat_get_runtime(netbuf->times[STAT_T10], netbuf->times[STAT_T9]),
            stat_get_runtime(netbuf->times[STAT_T11], netbuf->times[STAT_T10]),
            stat_get_runtime(netbuf->times[STAT_T12], netbuf->times[STAT_T11]),
            stat_get_runtime(netbuf->times[STAT_T13], netbuf->times[STAT_T12]),
            stat_get_runtime(netbuf->times[STAT_T14], netbuf->times[STAT_T13]),
            stat_get_runtime(netbuf->times[STAT_T15], netbuf->times[STAT_T14]),
            stat_get_runtime(netbuf->times[STAT_T16], netbuf->times[STAT_T15]),
            stat_get_runtime(netbuf->times[STAT_T17], netbuf->times[STAT_T16]),
            stat_get_runtime(netbuf->times[STAT_T18], netbuf->times[STAT_T17]),
            stat_get_runtime(curr_time, netbuf->times[STAT_T18]),
            stat_get_runtime(curr_time, netbuf->times[STAT_T0]));
    }

    netbuf->resv = 0;
#else
    param_unref(netbuf);
#endif
}

static inline osal_void host_print_rx_timestamp(oal_netbuf_stru *netbuf)
{
#ifdef TIME_STAMP_RX_DEBUG
    host_print_timestamp(netbuf);
#else
    param_unref(netbuf);
#endif
}

static inline osal_void host_print_tx_timestamp(oal_netbuf_stru *netbuf)
{
#ifdef TIME_STAMP_TX_DEBUG
#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
    static osal_u32 host_tx_cnt = 0;
    static osal_u64 host_tx_time = 0;
    osal_u64 curr_time = uapi_get_time_us();
#endif
    host_print_timestamp(netbuf);

#if (_PRE_OS_VERSION_LINUX != _PRE_OS_VERSION)
    host_tx_cnt++;
    if (curr_time - host_tx_time > STAT_CNT_TIME) {
        STAMP_PRINT("host txcnt:%u\r\n", host_tx_cnt);
        host_tx_time = curr_time;
    }
#endif
#else
    param_unref(netbuf);
#endif
}
#else
static inline osal_void stat_timer_enable(osal_void) {}
static inline osal_void stat_record_tx_timestamp(oal_netbuf_stru *netbuf, osal_u32 index)
{
    param_unref(netbuf);
    param_unref(index);
}
static inline osal_void stat_record_rx_timestamp(oal_netbuf_stru *netbuf, osal_u32 index)
{
    param_unref(netbuf);
    param_unref(index);
}
static inline osal_void host_print_rx_timestamp(oal_netbuf_stru *netbuf)
{
    param_unref(netbuf);
}
static inline osal_void host_print_tx_timestamp(oal_netbuf_stru *netbuf)
{
    param_unref(netbuf);
}
#endif
#endif
