/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: header for fe_tpc_rate_pow
 * Create: 2022-10-14
 */
#ifndef __FE_TPC_RATE_POW_H__
#define __FE_TPC_RATE_POW_H__
#include "wlan_types_base_rom.h"
#include "hal_commom_ops_type_rom.h"
#ifdef _PRE_PRODUCT_ID_HOST
#include "wlan_spec.h"
#include "frw_ext_if.h"
#include "hal_ops_common_rom.h"
#include "hcc_cfg_comm.h"
#include "dmac_common_inc_rom.h"
#else
#include "wlan_spec_rom.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define FE_HAL_TPC_RATE_TPC_CODE_NUM 74 // 功率档位的个数 0-147 分给ofdm(0-73)和11b
#define FE_HAL_TPC_RATE_POW_LVL_VAL 5   // 功率档位的间隔 0.5dB
// 给到外部使用的tpc code结构 对齐到tpc_code_params_stru (本地保存的档位是分ofdm和11b的 最大73 送出去是0-147)
typedef struct {
    osal_u16 tpc_code   : 8,
            bit_rsv    : 8;
} tpc_code_level_stru;
typedef struct {
    // bit0-6:tpc code 存储时不考虑11b 使用时根据速率档位判断是否11b 11b增加FE_HAL_TPC_RATE_TPC_CODE_NUM
    osal_u16 tpc_code_level[HAL_POW_LEVEL_NUM];
}fe_tpc_rate_pow_code_table_stru;
#ifdef _PRE_PRODUCT_ID_HOST
osal_void fe_hal_pow_sync_tpc_pow_table_to_dmac(hal_to_dmac_device_stru *hal_device, osal_u8 vap_id);
osal_u8 fe_hal_pow_get_custom_tpc_pow(osal_u8 band, osal_u8 rate_idx);
osal_u8 fe_hal_pow_get_tpc_code_by_pow(osal_u8 band, osal_s16 pow, osal_u8 is_ofdm);
#else
osal_void fe_hal_sync_tpc_pow_rate_table(osal_u8 *data, osal_u16 data_len);
#endif
osal_u16 fe_tpc_rate_pow_get_rate_tpc_code(osal_u8 band, osal_u8 protocol, osal_u8 mcs, osal_u8 bw, osal_u8 pwr_lvl);
osal_void fe_tpc_rate_pow_set_tpc_code_by_rate_idx(osal_u8 band, osal_u8 rate_idx,
    osal_u8 pwr_lvl, osal_u16 tpc_code_st);
osal_void fe_tpc_rate_pow_print_rate_pow_table(osal_void);
osal_u8 fe_tpc_rate_pow_get_rate_idx(osal_u8 protocol, osal_u8 mcs, osal_u8 bw);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __FE_TPC_RATE_POW_H__