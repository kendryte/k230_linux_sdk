/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Interface of power settings for hal layer.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_mac_reg.h"
#include "hal_tpc.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_TPC_ROM_C

#ifdef _PRE_WIFI_DEBUG
osal_void hal_tpc_query_band_rate_power_debug(hal_to_dmac_device_stru *hal_device)
{
    osal_u32 reg_info;
    unref_param(hal_device);

    reg_info = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xD4);
    wifi_printf("rts reg tx mode:0x%x\n", reg_info);
    reg_info = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xD8);
    wifi_printf("rts reg data rate:0x%x\n", reg_info);

    reg_info = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x10);
    wifi_printf("one packet reg tx mode:0x%x\n", reg_info);
    reg_info = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x14);
    wifi_printf("one packet reg rate:0x%x\n", reg_info);

    reg_info = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xF0);
    wifi_printf("cf end reg tx mode:0x%x\n", reg_info);
    reg_info = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xF4);
    wifi_printf("cf end reg rate:0x%x\n", reg_info);

    reg_info = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xF8);
    wifi_printf("bar reg tx mode:0x%x\n", reg_info);
    reg_info = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xFC);
    wifi_printf("bar reg rate:0x%x\n", reg_info);
}

osal_void hal_tpc_query_ack_power_reg(osal_void)
{
    osal_u8 distance_idx;
    osal_u32 reg_info;

    osal_u32 pow_reg_tbl[WLAN_DISTANCE_BUTT][HAL_RESP_POWER_REG_NUM] = {
        {
            HH503_MAC_CTRL1_BANK_BASE_0xD4,
            HH503_MAC_CTRL1_BANK_BASE_0xE0,
        },
        {
            HH503_MAC_CTRL1_BANK_BASE_0xCC,
            HH503_MAC_CTRL1_BANK_BASE_0xD8,
        },
        {
            HH503_MAC_CTRL1_BANK_BASE_0xD0,
            HH503_MAC_CTRL1_BANK_BASE_0xDC,
        }
    };

    reg_info = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0xE4);
    wifi_printf("mac resp power lv sel:0x%x\n", reg_info);
    for (distance_idx = WLAN_DISTANCE_NEAR; distance_idx < WLAN_DISTANCE_BUTT; distance_idx++) {
        wifi_printf("distance id %u\n", distance_idx);
        reg_info = hal_reg_read(pow_reg_tbl[distance_idx][0]);
        wifi_printf("ack cts reg dsss power lv:0x%x\n", reg_info);
        reg_info = hal_reg_read(pow_reg_tbl[distance_idx][1]);
        wifi_printf("ack cts reg ofdm 24M power lv:0x%x\n", reg_info);
    }
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif