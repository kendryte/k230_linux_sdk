/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_power.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_POWER_H__
#define __HAL_POWER_H__

#include "hal_soc.h"
#include "hal_mac.h"
#include "hal_phy.h"
#include "hal_rf.h"

#include "oal_ext_if.h"
#include "hal_ext_if.h"
#include "wlan_types_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HAL_NUM_OF_CUST_TXPOWER_SCALING_TABLE      1024
#define HAL_INIT_NVM_MAX_TXPWR_BASE_2P4G           190
#define HAL_INIT_NVM_MAX_TXPWR_BASE_5G             170
#define HAL_MAX_TXPOWER_MIN                        130
#define HAL_MAX_TXPOWER_MAX                        238

#define HH503_PHY_POWER_REF_2G_DEFAULT 0x14E8FC
#define HH503_PHY_POWER_REF_5G_DEFAULT 0x0ce0f4

#define HAL_TPC_CH_NUM 32

#define HAL_PGA_STEP_NUM 64

#define HAL_POWER_CHANGE_COEF 10

#define HAL_POW_LPF_LUT_NUM                 8
#define HAL_POW_DAC_LUT_NUM                 4

#define HAL_POW_DIST_5G_TABLE_LEN           13
#define HAL_POW_DIST_2G_TABLE_LEN           16

#ifdef _PRE_WLAN_ONLINE_DPD
#define HH503_DPD_OFF_CFR_ON               2
#define HH503_DPD_LVL_INVALID              3
#endif

#define HAL_RESP_POWER_REG_NUM              2

#define hal_get_cfr_idx_from_tpc(tpc)           ((osal_u8)(oal_get_bits((tpc), NUM_2_BITS, BIT_OFFSET_10)))
#define hal_get_dpd_tpc_lv_ch0_from_tpc(tpc)    ((osal_u8)(oal_get_bits((tpc), NUM_2_BITS, BIT_OFFSET_8)))
#define hal_get_tpc_ch0_from_tpc(tpc)           ((osal_u8)(oal_get_bits((tpc), NUM_8_BITS, BIT_OFFSET_0)))

typedef enum {
    HAL_POW_LEVEL_0 = 0,
    HAL_POW_LEVEL_1,
    HAL_POW_LEVEL_2,
    HAL_POW_LEVEL_3,
    HAL_POW_LEVEL_4,
    HAL_POW_LEVEL_5,
    HAL_POW_LEVEL_BUTT
} hal_pow_lvl_no_enum;


typedef struct {
    osal_u32  dpd_tpc_lv        : 2,
            cfr_index         : 2,
            upc_level         : 1,
            lpf_gain          : 3,
            pa_gain           : 2,
            dac_gain          : 2,
            delta_dbb_scaling : 10,
            dpd_enable        : 1,
            bit_rsv               : 9;
} pow_code_params_stru;

typedef struct {
    osal_u16 tpc_ch0          : 8,
            dpd_tpc_lv_ch0    : 2,
            cfr_idx           : 2,
            bit_rsv           : 4;
} tpc_code_params_stru;


typedef struct {
    wlan_channel_band_enum_uint8 freq_band;
    osal_u8 rate_idx;
    osal_u8 pwr_idx;
    osal_s16 tx_power;
    osal_u8 resv[3];
} hal_tpc_set_stru;

typedef struct {
    wlan_bw_cap_enum_uint8 rpt_trig_bw;
    osal_u8              rpt_trig_type;
    osal_u8              rpt_trig_mcs;
    osal_u8              rpt_trig_nss;
    osal_u8              rpt_trig_target_rssi;
    osal_u8              rpt_trig_ap_tx_power;
    osal_u8              set_tpc;
    osal_u8              rate_idx;
} hal_rpt_trig_frm_params_stru;

typedef struct {
    osal_u16  max_pow  : 6,
            min_pow  : 6,
            offset   : 4;
} mcs_tx_pow_stru;

typedef struct {
    osal_u8   dac_idx;
    osal_u8   lpf_idx;
    osal_u8   upc_idx;
    osal_u8   pa_idx;
} hal_pow_distri_ratio_stru;

typedef struct {
    osal_u8   cfr_idx;
    osal_u8   dpd_idx;
} hal_dpd_cfr_distri_ratio_stru;

typedef struct {
    osal_u32  dbb_scaling_2g    : 10;
    osal_u32  dpd_tpc_lv_ch1    : 2;
    osal_u32  dpd_update_en_ch1 : 1;
    osal_u32  dpd_tpc_lv_ch0    : 2;
    osal_u32  dpd_update_en_ch0 : 1;
    osal_u32  cfr_idx           : 2;
    osal_u32  dbb_scaling_5g    : 10;
    osal_u32  resv0             : 4;
    osal_s16   max_power_2g;
    osal_s16   max_power_5g;
} hal_pow_tpc_param_stru;

typedef struct {
    osal_u32  trig_phy_mode;
    osal_u32  trig_data_rate;
} hal_pow_trig_param_stru;

osal_void hh503_get_cali_param_set_tpc(hal_to_dmac_device_stru *hal_device, wlan_channel_band_enum_uint8 band,
    osal_u8 channel_idx);
osal_void hh503_tb_mcs_tx_power_ext(wlan_channel_band_enum_uint8 band);
osal_void hh503_tb_tx_power_init_ext(wlan_channel_band_enum_uint8 band);
osal_void hh503_pow_sw_initialize_tx_power_ext(hal_to_dmac_device_stru *hal_device);
osal_void hh503_pow_init_vap_pow_code_ext(hal_to_dmac_device_stru *hal_device,
    hal_vap_pow_info_stru *vap_pow_info, hal_pow_set_type_enum_uint8 type, const mac_channel_stru *channel);
osal_void hal_tpc_rate_pow_print_rate_pow_table(osal_void);
osal_void hh503_set_ctrl_frm_pow_code(osal_u32 phy_mode_addr, osal_u32 data_rate_addr, osal_u32 pow_code);
osal_void hh503_set_tpc_params(hal_to_dmac_device_stru *hal_device, hal_rf_chn_param *rf_chn_param);
typedef osal_void (*hh503_get_cali_param_set_tpc_cb)(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u8 channel_idx);
typedef osal_void (*hh503_tb_mcs_tx_power_cb)(wlan_channel_band_enum_uint8 band);
typedef osal_void (*hh503_tb_tx_power_init_cb)(wlan_channel_band_enum_uint8 band);

typedef osal_void (*hh503_set_tx_dscr_power_tpc_cb)(osal_u8 rate_idx, wlan_channel_band_enum_uint8 band,
    osal_s16 tx_power, tpc_code_params_stru *tpc_param);
typedef osal_void (*hh503_pow_sw_initialize_tx_power_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hh503_pow_initialize_tx_power_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hh503_pow_init_vap_pow_code_cb)(hal_to_dmac_device_stru *hal_device,
    hal_vap_pow_info_stru *vap_pow_info, hal_pow_set_type_enum_uint8 type, const mac_channel_stru *channel);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
