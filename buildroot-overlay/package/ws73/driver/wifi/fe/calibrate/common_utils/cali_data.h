/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准数据
 * Create: 2022-09-13
 */

#ifndef __CALI_DATA_H__
#define __CALI_DATA_H__

#include "cali_data_type.h"
#include "fe_rf_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#ifdef BOARD_ASIC_WIFI
#define R_CODE_DEFAULT 0x3
#define C_CODE_DEFAULT 0x3
#define RC_CODE_DEFAULT 0x2C
#define RC_CODE_DEFAULT_20M 0x2C
#define RX_DC_CODE_DEFAULT 0x8080
#define TX_PWR_PPA_CAP_DEFAULT 0x14
#define TX_PWR_UPC_CODE_DEFAULT 0x1A2D
#define TX_PWR_UPC_CODE_DEFAULT2 0x132D
#define LOGEN_CODE_BAND_0_DEFAULT 0x6
#define LOGEN_CODE_BAND_1_DEFAULT 0x7
#define LOGEN_CODE_BAND_2_DEFAULT 0x8
#else
#define R_CODE_DEFAULT 0x3
#define C_CODE_DEFAULT 0x3
#define RC_CODE_DEFAULT 0x3B
#define RC_CODE_DEFAULT_20M 0x40
#define RX_DC_CODE_DEFAULT 0x8080
#define TX_PWR_PPA_CAP_DEFAULT 0x18
#define TX_PWR_UPC_CODE_DEFAULT 0x50
#define LOGEN_CODE_BAND_0_DEFAULT 0x3
#define LOGEN_CODE_BAND_1_DEFAULT 0x3
#define LOGEN_CODE_BAND_2_DEFAULT 0x3
#define TX_PWR_PPA_CAP_DEFAULT_HIGH_BAND 0x10
#endif
#define RX_DC_CALI_TEMP 0x19
typedef struct {
    osal_u16 cali_data_len;
    osal_u8 rf_id;
    osal_u8 rsv;
} cali_sync_param_header_stru;
typedef struct {
    cali_sync_param_header_stru msg;
    cali_param_stru cali_data[CALI_RF_CHANNEL_NUMS];
} cali_sync_param_stru;
cali_param_stru *cali_get_cali_data(osal_u32 rf_idx);
osal_void cali_data_init_default(osal_u8 rf_id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
