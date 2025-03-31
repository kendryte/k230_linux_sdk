/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_rf.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_RF_H__
#define __HAL_RF_H__

#include "hal_ext_if.h"
#include "hal_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_RF_H
#define HH503_RF_W_C0_REG_BASE_ADDR            0x40040800
#define HH503_RF_W_C1_REG_BASE_ADDR            0x40042800

#define HH503_RF_W_PLL_C0_REG_BASE_ADDR        0x40041000
#define HH503_RF_W_PLL_C1_REG_BASE_ADDR        0x40043000

#define HH503_RF_W_C0_CTL_BASE 0x40040000
#define HH503_RF_W_C0_CTL_BASE_0x204 (HH503_RF_W_C0_CTL_BASE + 0x204)
#define HH503_RF_W_C0_CTL_BASE_0x200 (HH503_RF_W_C0_CTL_BASE + 0x200)
#define HH503_RF_W_C0_CTL_BASE_0x220 (HH503_RF_W_C0_CTL_BASE + 0x220)
#define HH503_RF_W_C0_CTL_BASE_0x218 (HH503_RF_W_C0_CTL_BASE + 0x218)
#define HH503_RF_W_C0_CTL_BASE_0x210 (HH503_RF_W_C0_CTL_BASE + 0x210)
#define HH503_RF_W_C0_CTL_BASE_0x230 (HH503_RF_W_C0_CTL_BASE + 0x230)

#define HH503_RF_W_C1_CTL_BASE 0x40042000
#define HH503_RF_W_C1_CTL_BASE_0x204 (HH503_RF_W_C1_CTL_BASE + 0x204)
#define HH503_RF_W_C1_CTL_BASE_0x200 (HH503_RF_W_C1_CTL_BASE + 0x200)
#define HH503_RF_W_C1_CTL_BASE_0x220 (HH503_RF_W_C1_CTL_BASE + 0x220)
#define HH503_RF_W_C1_CTL_BASE_0x218 (HH503_RF_W_C1_CTL_BASE + 0x218)
#define HH503_RF_W_C1_CTL_BASE_0x210 (HH503_RF_W_C1_CTL_BASE + 0x210)
#define HH503_RF_W_C1_CTL_BASE_0x230 (HH503_RF_W_C1_CTL_BASE + 0x230)

#ifndef RF_CALI_DATA_BUF_LEN
#define RF_CALI_DATA_BUF_LEN              (0x6cd8)
#endif
#define RF_SINGLE_CHAN_CALI_DATA_BUF_LEN  (RF_CALI_DATA_BUF_LEN >> 1)
#define HH503_TEMPERATURE_THRESH         50

#ifdef _PRE_WLAN_03_MPW_RF
#define HH503_TOP_TST_SW2                      30
#endif

#ifdef _PRE_WLAN_03_MPW_RF
#define HH503_PLL_LD_REG34           34
#endif

#define HH503_RF_TCXO_25MHZ          1
#define HH503_RF_TCXO_40MHZ          2
#define HH503_RF_TCXO_384MHZ         3

#define HH503_RF_TCXO_FREQ           HH503_RF_TCXO_384MHZ

#define HH503_RF_LPF_GAIN_DEFAULT_VAL    0x4688
#define HH503_RF_DAC_GAIN_DEFAULT_VAL    0x24

#ifdef _PRE_WLAN_03_MPW_RF
#define HH503_WL_PLL1_RFLDO789_ENABLE      0xFF00
#endif

#define HH503_RF_2G_CHANNEL_NUM        13
#define HH503_RF_5G_CHANNEL_NUM        7

#define HH503_RF_FREQ_2_CHANNEL_NUM  14
#define HH503_RF_FREQ_5_CHANNEL_NUM  29

#define HH503_CALI_CHN_INDEX_2422        0
#define HH503_CALI_CHN_INDEX_2447        1
#define HH503_CALI_CHN_INDEX_2472        2

#define CHANNEL_NUM5_IDX          4
#define CHANNEL_NUM10_IDX          9

#define HH503_CALI_CHN_INDEX_4950        0
#define HH503_CALI_CHN_INDEX_5210        1
#define HH503_CALI_CHN_INDEX_5290        2
#define HH503_CALI_CHN_INDEX_5530        3
#define HH503_CALI_CHN_INDEX_5610        4
#define HH503_CALI_CHN_INDEX_5690        5
#define HH503_CALI_CHN_INDEX_5775        6

#define HH503_RF_TEMP_STS_RSV                  0x7

#define HH503_RF_TEMP_INVALID                   (-100)

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define HH503_TX2G_PA_GATE_VCTL_REG_NUM        (9)
#define HH503_TX2G_PA_VRECT_GATE_THIN_REG_NUM  (32)
#define HH503_TX2G_REG_280_BAND2_3_NUM         (2)
#define HH503_TX2G_PA_GATE_REG_NUM    (HH503_TX2G_PA_GATE_VCTL_REG_NUM + HH503_TX2G_PA_VRECT_GATE_THIN_REG_NUM + \
    HH503_TX2G_REG_280_BAND2_3_NUM)
#endif

typedef enum {
    CCA_ED_TYPE_20TH = 0,
    CCA_ED_TYPE_40TH,
} cca_ed_type_enum;

typedef enum {
    HH503_RF_BAND_CTRL_10M,
    HH503_RF_BAND_CTRL_20M,
    HH503_RF_BAND_CTRL_40M,
    HH503_RF_BAND_CTRL_80M,

    HH503_RF_BAND_CTRL_BUTT
} hh503_rf_bandwidth_enum;
typedef osal_u8 hh503_rf_bandwidth_enum_uint8;

typedef enum {
    RFLDO_EN_OFF,
    RF_EN_OFF,
    RF_OFF_BUTT
} rf_off_flag_enum;
typedef osal_u8 rf_off_flag_enum_uint8;
typedef enum {
    HAL_RF_ALWAYS_POWER_ON,
    HAL_RF_SWITCH_ALL_LDO,
    HAL_RF_SWITCH_PA_PPA,

    HAL_RF_OPS_CFG_BUTT
} hal_rf_ops_cfg_enum;
typedef osal_u8 hal_rf_ops_cfg_uint8;
typedef struct {
    osal_u8   norm2_dpa_idx_5g20m[HH503_RF_FREQ_5_CHANNEL_NUM];
    osal_u8   norm2_dpa_idx_2g[HH503_RF_FREQ_2_CHANNEL_NUM];
    osal_u8   other_dpa_idx_2g;
    osal_u8   other_dpa_idx_5g;
} hh503_cali_chn_idx_stru;
typedef struct {
    wlan_channel_band_enum_uint8       band;
    wlan_channel_bandwidth_enum_uint8  bw;
    osal_u8                              ch_num;
    osal_u8                              ch_idx;
} hal_rf_chn_param;
typedef struct {
    hal_rf_trx_type_enum           trx_type;
    hal_rf_trx_type_enum           trx_type_hist;
    osal_bool                        trx_type_change;
    osal_u8                          rf_enabled     : 1,
                                   ldo_2g_wf0_on  : 1,
                                   ldo_2g_wf1_on  : 1,
                                   ldo_5g_wf0_on  : 1,
                                   ldo_5g_wf1_on  : 1,
                                   resv           : 3;
    hal_rf_chn_param               chn_para;
} hal_rf;

osal_char fe_rf_get_customized_cca_ed(osal_u32 cca_ed_type, wlan_channel_band_enum_uint8 band);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
typedef struct {
    hal_cfg_custom_gain_db_per_band_stru   gain_db_2g[HAL_DEVICE_2G_BAND_NUM_FOR_LOSS];
    hal_cfg_custom_gain_db_per_band_stru   gain_db_5g[HAL_DEVICE_5G_BAND_NUM_FOR_LOSS];
} hh503_cfg_custom_gain_db_rf_stru;

typedef struct {
    osal_s8   cfg_delta_pwr_ref_rssi_2g[HAL_DEVICE_2G_DELTA_RSSI_NUM];
    osal_s8   cfg_delta_pwr_ref_rssi_5g[HAL_DEVICE_5G_DELTA_RSSI_NUM];
} hh503_cfg_custom_delta_pwr_ref_stru;

typedef struct {
    hh503_cfg_custom_gain_db_rf_stru       rf_gain_db_rf[WLAN_RF_CHANNEL_NUMS];
    hh503_cfg_custom_delta_pwr_ref_stru    delta_pwr_ref_cfg[WLAN_RF_CHANNEL_NUMS];
    osal_u16                              aus_dyn_cali_dscr_interval[WLAN_BAND_BUTT];
    osal_u8                               far_dist_pow_gain_switch;
    osal_u8                               band_5g_enable;
    osal_u8                               tone_amp_grade;
    osal_u8                               far_dist_dsss_scale_promote_switch;

    osal_u8                               chn_radio_cap;
    osal_s8                               junction_temp_diff;
    osal_u16                              rsv;

    hal_cfg_custom_cca_stru                 cfg_cus_cca;

    osal_u16                              over_temp_protect_thread;
    osal_u16                              recovery_temp_protect_thread;
} hh503_rf_custom_stru;

typedef struct {
    osal_s32                  power;
    osal_u16                  mode_ctrl;
    osal_u16                  mode_gain;
    osal_u16                  pa_gain;
    osal_u16                  abb_gain;
    osal_u32                  scaling;
} hh503_adjust_tx_power_stru;

typedef struct {
    osal_s32 power;
    osal_u8 tpc_code;
    osal_u8 resrv[3];
} hh503_adjust_tx_power_level_stru;

typedef struct {
    osal_u8 index;
    osal_u8 channel_num_min;
    osal_u8 channel_num_max;
    osal_u8 reseve;
} hh503_band_num_for_loss;

static const hh503_band_num_for_loss g_band_num_for_loss[] = {
    {0, 1, 4, 0}, {1, 6, 11, 0}, {2, 13, 14, 0},
    {0, 184, 196, 0}, {1, 36, 48, 0}, {2, 52, 64, 0},
    {3, 100, 112, 0}, {4, 116, 128, 0}, {5, 132, 140, 0},
    {6, 149, 165, 0}
};

extern hh503_rf_custom_stru g_rf_customize;
#endif // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

osal_void hh503_set_rf_bw(wlan_bw_cap_enum_uint8 bw);

osal_void hh503_dpd_cfr_set_11b_ext(const hal_to_dmac_device_stru *hal_device, osal_u8 is_11b);
osal_void hh503_cali_send_func_ext(hal_to_dmac_device_stru *hal_device, osal_u8* cali_data, osal_u16 frame_len,
    osal_u16 remain);
osal_u32 hh503_config_custom_rf_ext(const osal_u8 *param);
osal_void hh503_config_rssi_for_loss_ext(osal_u8 channel_num, osal_s8 *rssi);
osal_void hh503_set_rf_custom_reg_ext(const hal_to_dmac_device_stru *hal_device);
osal_void hh503_initialize_rf_sys_ext(hal_to_dmac_device_stru *hal_device);
osal_u8 hh503_get_tpc_code_ext(osal_void);
osal_void hh503_adjust_tx_power_level_ext(osal_u8 ch, osal_s8 power);

typedef osal_u8 (*hh503_get_rx_chain_cb)(osal_void);
typedef osal_void (*hh503_set_rf_bw_cb)(wlan_bw_cap_enum_uint8 bw);

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
typedef osal_void (*hh503_read_max_temperature_cb)(osal_s16 *ps_temperature);
#endif
typedef osal_void (*hh503_rf_set_trx_type_cb)(hal_rf_trx_type_enum trx_type);
typedef osal_void (*hh503_dpd_cfr_set_11b_cb)(hal_to_dmac_device_stru *hal_device, osal_u8 is_11b);
typedef osal_void (*hh503_cali_send_func_cb)(hal_to_dmac_device_stru *hal_device, osal_u8* cali_data,
    osal_u16 frame_len, osal_u16 remain);
typedef osal_u32 (*hh503_config_custom_rf_cb)(const osal_u8 *param);
typedef osal_void (*hh503_config_rssi_for_loss_cb)(osal_u8 channel_num, osal_s8 *rssi);
typedef osal_void (*hh503_set_rf_custom_reg_cb)(hal_to_dmac_device_stru *hal_device);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
typedef osal_u32 (*hh503_config_custom_dts_cali_cb)(const osal_u8 *param);
#endif
typedef osal_void (*hh503_initialize_rf_sys_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hh503_adjust_tx_power_cb)(osal_u8 ch, osal_s8 power);
typedef osal_void (*hh503_restore_tx_power_cb)(osal_u8 ch);
typedef osal_u8 (*hh503_rf_get_subband_idx_cb)(wlan_channel_band_enum_uint8 band, osal_u8 channel_idx);
typedef osal_void (*hh503_pow_set_rf_regctl_enable_cb)(hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 rf_linectl);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
osal_void hal_rf_get_rx_ppdu_info(osal_u8 flag);
#endif
osal_void hal_set_cal_tone(osal_u32 tone_cfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
