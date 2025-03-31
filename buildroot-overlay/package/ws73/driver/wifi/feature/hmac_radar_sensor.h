/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_radar_sensor.h
 * Date: 2023-02-20
 */

#ifndef __HMAC_RADAR_SENSOR_H__
#define __HMAC_RADAR_SENSOR_H__

#include "wlan_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    osal_u8       timeout_val;
    osal_u8       ifs_sel;
    osal_u8       mac_tx_bypass;
    osal_u32      selfcts_phy_mode;
    osal_u32      selfcts_rate;
    osal_u16      selfcts_duration;
    osal_u8       selfcts_addr[WLAN_MAC_ADDR_LEN];
    osal_u16      t1;
    osal_u16      t2;
    osal_u16      t3;
    osal_u16      t4;
    osal_u16      tx_dly_160m;
    osal_u16      tx_dly_320m;
    osal_u8       ch_num;
    osal_u8       ch0_sel : 2;
    osal_u8       ch1_sel : 2;
    osal_u8       ch2_sel : 2;
    osal_u8       ch3_sel : 2;
    osal_u8       ch0_period : 4;
    osal_u8       ch1_period : 4;
    osal_u8       ch2_period : 4;
    osal_u8       ch3_period : 4;
    osal_u8       ch0_smp_period_start : 4;
    osal_u8       ch0_smp_period_end : 4;
    osal_u8       ch1_smp_period_start : 4;
    osal_u8       ch1_smp_period_end : 4;
    osal_u8       ch2_smp_period_start : 4;
    osal_u8       ch2_smp_period_end : 4;
    osal_u8       ch3_smp_period_start : 4;
    osal_u8       ch3_smp_period_end : 4;
    osal_u8       lna_gain;
    osal_u8       vga_gain;
    osal_u16      rx_dly_160m;
    osal_u16      rx_dly_80m;
    osal_u8       lpf_gain;
    osal_u8       dac_gain;
    osal_u8       pa_gain;
    osal_u16      ppa_gain;
    osal_u8       print_cnt;
    osal_u8       print_cnt_num;
} hmac_radar_sensor_debug_stru;

typedef struct {
    osal_u8        enable;
    osal_u8        rx_nss; /* 接收天线数, n - 1配置 */
    osal_u8        period_cycle_cnt; /* 0:循环发送, 其他值:循环次数 */
    osal_u8        one_per_cnt; /* 单个雷达周期中, PHY侧循环雷达周期数 */
    osal_u8        *radar_data; /* 雷达波形起始地址 */
    osal_u16       radar_data_len; /* 雷达波形长度 */
    osal_u16       radar_period; /* 雷达波形收发间隔, 默认5ms */
} hmac_radar_sensor_cfg_stru;

typedef osal_void (*radar_handle_cb)(uint32_t *rx_mem);

static osal_u32 hmac_radar_sensor_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_radar_sensor_init"), used));
static osal_void hmac_radar_sensor_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_radar_sensor_deinit"), used));

osal_u32 hmac_radar_sensor_register_handle_cb(radar_handle_cb cb);
osal_s16 hmac_radar_sensor_config_rx_gain(osal_u8 lna_gain,
    osal_u8 vga_gain, osal_u16 rx_dly_160m, osal_u16 rx_dly_80m);
osal_s32 hmac_radar_sensor_start(hmac_radar_sensor_cfg_stru *radar_sensor_cfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
