/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Header file for hmac_stat.c.
 */
#ifndef __HMAC_STAT_H__
#define __HMAC_STAT_H__

#include "frw_ext_common_rom.h"
#include "frw_osal.h"
#include "frw_msg_rom.h"
#include "osal_adapt.h"
#include "mac_vap_ext.h"

typedef struct {
    osal_u32 pre_time;                  /* 用于差值计算确切时间，排除函数调用时间 */
    frw_timeout_stru hmac_freq_timer;   /* auto freq timer */
} hmac_stat_control_stru;

#define HMAC_STAT_TIMER_PERIOD     200 /* 定时器周期， 单位ms */

void hmac_throughput_timer_init(void);
void hmac_throughput_timer_deinit(void);
osal_s32 hmac_config_pk_mode_th(hmac_vap_stru *hmac_vap, frw_msg *msg);
/* HMAC device级别统计 */
typedef struct hmac_device_stat {
    osal_atomic tx_packets;
    osal_atomic rx_packets;
    osal_atomic rx_bytes;
    osal_atomic tx_bytes;
} hmac_device_stat_stru;
hmac_device_stat_stru *hmac_stat_get_device_stats(void);
OAL_STATIC OAL_INLINE void hmac_stat_device_tx_netbuf(osal_u32 netbuf_len)
{
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
    osal_adapt_atomic_inc(&hmac_device_stats->tx_packets);
    osal_adapt_atomic_add(&hmac_device_stats->tx_bytes, (osal_s32)netbuf_len);
}
OAL_STATIC OAL_INLINE void hmac_stat_device_rx_netbuf(uint32_t netbuf_len)
{
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
    osal_adapt_atomic_inc(&hmac_device_stats->rx_packets);
    osal_adapt_atomic_add(&hmac_device_stats->rx_bytes, (int32_t)netbuf_len);
}
#endif