/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准模块的规格定义(各个芯片差异化定制)
 * Create: 2022-10-11
 */

#ifndef __CALI_SPEC_H__
#define __CALI_SPEC_H__

#include "osal_types.h"
#include "wlan_types_base_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// 校准模块支持的band
#define WLAN_CALI_BAND_BUTT (WLAN_BAND_2G + 1)

// 校准支持的天线数
#define CALI_RF_CHANNEL_NUMS (1)
/* 校准信道个数 */
#define CALI_2G_OTHER_CHANNEL_NUM 1
#define CALI_2G_CHANNEL_NUM 3
// 校准类型定义 对应定制化cali_mask
typedef enum {
    WLAN_ABB_CALI                = 0,
    WLAN_RF_CALI_RC              = 1,
    WLAN_RF_CALI_R               = 2,
    WLAN_RF_CALI_C               = 3,
    WLAN_RF_CALI_PPF             = 4,
    WLAN_RF_CALI_LOGEN           = 5,
    WLAN_RF_CALI_LODIV           = 6,
    WLAN_RF_CALI_PA_ICAL         = 7,
    WLAN_RF_CALI_RXDC            = 8,
    WLAN_RF_CALI_TX_PWR          = 9,
    WLAN_RF_CALI_TXDC            = 10,
    WLAN_RF_CALI_TXIQ            = 11,
    WLAN_RF_CALI_RXIQ            = 12,
    WLAN_RF_CALI_DPD_COMP_40M    = 13,
    WLAN_RF_CALI_DPD_CAL         = 14,
    WLAN_RF_CALI_RX_GAIN         = 15,
    WLAN_RF_CALI_TYPE_BUTT,
} wlan_rf_cali_type_enum;
typedef osal_u8 wlan_rf_cali_type_enum_uint8;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
