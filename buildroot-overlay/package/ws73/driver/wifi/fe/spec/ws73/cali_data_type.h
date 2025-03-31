/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: 校准数据的类型定义
 * Create: 2020-7-3
 */

#ifndef CALI_DATA_TYPE_H
#define CALI_DATA_TYPE_H

#include "cali_complex_num.h"
#include "cali_base_def.h"
#include "cali_outer_os_if.h"
#include "cali_outer_hal_if.h"
#include "cali_spec.h"

#define CALI_TXDC_GAIN_LVL_NUM (4)    // TX DC 补偿档位 4档
#define CALI_RXDC_GAIN_LNA_LVL_NUM (8) /* rx dc补偿值lna档位数目 */
#define CALI_RXDC_GAIN_VGA_LVL_NUM (2) /* rx dc补偿值vga档位数目 */
#define CALI_TXPWR_GAIN_LVL_NUM_MAX  (2) /* tx pwr补偿值预留最大档位数目 */
#define CALI_TX_IQ_GAIN_LVL_NUM 2
#define CALI_RX_IQ_GAIN_LVL_NUM 2
#define CALI_DC_OFFSET_NUM      2
#define CALI_IQ_LS_COMP_NUM (5)
#ifdef BOARD_ASIC_WIFI
#define CALI_RF_RX_IQ_BANDWIDTH_NUM 2
#else
#define CALI_RF_RX_IQ_BANDWIDTH_NUM 1
#endif

typedef struct {
    osal_u16 aus_analog_rxdc_siso_cmp[CALI_RXDC_GAIN_LNA_LVL_NUM][CALI_RXDC_GAIN_VGA_LVL_NUM];
    osal_u16 digital_rxdc_cmp_i;
    osal_u16 digital_rxdc_cmp_q;
    osal_s16 cali_temperature;
    osal_u16 resv;
} cali_rx_dc_comp_val_stru;

typedef struct {
    osal_u8 ppa_cmp;
    osal_u16 atx_pwr_cmp[CALI_TXPWR_GAIN_LVL_NUM_MAX];
    osal_s16 tx_power_dc_2g[CALI_DC_OFFSET_NUM];
} cali_tx_power_comp_val_stru;

typedef struct {
    osal_u8 ssb_cmp;
    osal_u8 buf0_cmp;
    osal_u8 buf1_cmp;
    osal_u8 resv;
} cali_logen_comp_val_stru;

typedef struct {
    osal_u16 txdc_cmp_i;
    osal_u16 txdc_cmp_q;
} cali_txdc_comp_val_stru;

typedef struct {
    cali_complex_stru ls_fir[CALI_IQ_LS_COMP_NUM];
} cali_rxiq_tone_comp_val_stru;
typedef struct {
    cali_complex_stru ls_fir[CALI_IQ_LS_COMP_NUM];
} cali_txiq_tone_comp_val_stru;

typedef struct {
    cali_rx_dc_comp_val_stru cali_rx_dc_cmp[CALI_2G_OTHER_CHANNEL_NUM];
    cali_logen_comp_val_stru cali_logen_cmp;
    cali_tx_power_comp_val_stru cali_tx_power_cmp_2g[CALI_2G_CHANNEL_NUM];
    cali_txdc_comp_val_stru txdc_cmp_val[CALI_2G_CHANNEL_NUM][CALI_TXDC_GAIN_LVL_NUM];
    cali_rxiq_tone_comp_val_stru rx_iq_val[CALI_2G_CHANNEL_NUM][CALI_RX_IQ_GAIN_LVL_NUM][CALI_RF_RX_IQ_BANDWIDTH_NUM];
    cali_txiq_tone_comp_val_stru tx_iq_val[CALI_2G_CHANNEL_NUM][CALI_TX_IQ_GAIN_LVL_NUM];
} cali_param_2g_stru;

typedef struct {
    osal_u16 cali_time;
    osal_u16 temperature : 3, chan_idx1_5g : 5, chan_idx2_5g : 5, rev : 3;
} cali_update_channel_stru;

typedef struct {
    osal_u8 rc_cmp_code;
    osal_u8 r_cmp_code; /* 保存PMU的原始5bit R code */
    osal_u8 c_cmp_code; /* 重要: MPW2和PILOT RF公用, mpw2代表c校准值; pilot代表800M rejection补偿code，根据C
                            code计算得到 */
    osal_u8 rc_cmp_code_20m;
} cali_rc_r_c_param_stru;

typedef struct {
    osal_u8 bias_comp;
    osal_u8 reserve[3];
} cali_ipa_current_param_stru;
struct hh503_cali_param_tag {
    cali_param_2g_stru cali_param_2g;
    cali_update_channel_stru cali_update_info;
    cali_ipa_current_param_stru ipa_current_data;
    cali_rc_r_c_param_stru rc_r_c_cali_data;
    oal_bool_enum_uint8 save_all;
    osal_s8 cali_temp;
    osal_u16 resv;
};
#endif /* end of cali_data_type.h */
