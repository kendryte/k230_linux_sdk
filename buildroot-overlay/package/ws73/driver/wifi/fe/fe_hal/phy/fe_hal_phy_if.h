/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy功能接口
 * Date: 2022-09-13
 */

#ifndef __FE_HAL_PHY_IF_H__
#define __FE_HAL_PHY_IF_H__

#include "cali_outer_hal_if.h"
#include "cali_complex_num.h"
#include "fe_hal_phy_reg_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define CALI_IQ_COEF_NUM 5
typedef enum {
    ADC_FS_80_M  = 80,
    ADC_FS_160_M = 160,
    ADC_FS_320_M = 320
} cali_adc_fs;

typedef enum {
    DAC_FS_160_M = 160,
    DAC_FS_320_M = 320,
    DAC_FS_480_M = 480
} cali_dac_fs;
/* 单音信号配置结构 */
typedef struct {
    osal_u16 iq_cali_delay;                           /* 延迟iq_cali_delay个TXclk */
    osal_u8 tone_sign;                                /* 0:正频率 1:负频率 */
    cali_single_tone_freq_enum_uint8 tone_mode;       /* 频率模式 */
    cali_single_tone_mask_enum_uint8 tone_mask;       /* 信号输出屏蔽选择 */
    cali_single_tone_amp_enum_uint8 tone_amp;         /* 信号幅度档位 */
    osal_u8 iq_pn_flag;                               /* 正负单音信号指示 */
    osal_u8 cfg_tone_en;                              /* 使能 */
} cali_single_tone_stru;
// phy al tx param
typedef struct {
    osal_u8 protocol_mode; /* 发送协议模式 wlan_phy_protocol_enum_uint8 */
    /*
     * 0000: BW20
     * 0001: SU ER 106 tone
     * 0100: BW40
     * 0101: BW_20 – Duplicate Legacy in 40MHz
     * 1000: BW80
     * 1001: BW_20 – Duplicate Legacy in 80MHz
     * 其他：保留
     */
    osal_u8 bandwidth;     /* 带宽 fe_hal_phy_al_tx_bw_enum_uint8 */
    osal_u8 rf_id;         /* 通道 wlan_phy_chain_sel_enum_uint8 */
    /*
     * 发送帧的速率
     * vht/he时，【21:20】表示Nss=1，固定值0；【19:16】跟协议保持一致：0代表mcs0，最高mcs9。
     * ht时，【21:20】表示Nss=1，固定值0；【19:16】跟协议保持一致：0代表mcs0，最高mcs7；
     * ht 40m下6'b10_0000表示发mcs32速率。
     * 11a/g时：
     *  1011 -> 6 Mbps 1111 -> 9 Mbps 1010 -> 12 Mbps 1110 -> 18 Mbps 1001 -> 24 Mbps 1101 -> 36 Mbps
     *  1000 -> 48 Mbps 1100 -> 54 Mbps
     * 11b时：
     *  0000 -> 1 Mbps 0001 -> 2 Mbps 0010 -> 5.5 Mbps 0011 -> 11 Mbps
     */
    osal_u8 nss_mcs_rate; /* 速率 wlan_he_mcs_enum_uint8 wlan_ht_mcs_enum_uint8 wlan_11b_rate_index_enum_uint8 */
    osal_u32 psdu_len;    /* 帧长 */
    osal_u16 legacy_len;
    osal_u8 gi_type;    /* GI类型 he:wlan_phy_he_gi_type_enum_uint8 other:wlan_gi_type_enum_uint8 */
    osal_u8 ltf_type;   /* LTF类型 wlan_phy_he_ltf_type_enum_uint8 */
    osal_u16 frame_gap; /* 帧间隔 */
    osal_u16 frame_num; /* 发帧帧数(0表示一直发直到关闭) */
    osal_u8 tpc_code;
    osal_u8 pre_fec_padding_factor;
    osal_u8 pe_duration;
    osal_u8 fec_coding; // 编码 bcc ldpc wlan_channel_code_enum_uint8
    osal_u8 pdet_en;    // 动态功率校准使能
    osal_u8 dpd_tpc_lvl;
    osal_u8 preamble; /* 11b 0:short 1:long  11n 0:mixed mode 1:gf mode other 0:default 1:rsv  */
    osal_u8 rsv; /* 4字节对齐，1字节保留字段 */
} fe_hal_phy_al_tx_para_stru;

// 分多档校准的时候，每档的校准配置信息
typedef struct {
    osal_s16 target_power;       // 每一档的目标功率
    osal_u16 default_ppa_code;   // 默认的ppa_code
    osal_u16 init_dbb_scale;     // 每档校准时使用的dbb_scale
    osal_u8 default_fb_idx;
    osal_u8 default_lpf_idx;
    osal_u8 default_dac_idx;
    osal_u8 gm_code_max;
    osal_u8 gm_code_min;
    osal_u8 default_gm_code;
    osal_u8 unit_code_max;
    osal_u8 unit_code_min;
} cali_txpwr_lvl_cfg_stru;

// 生成单音
osal_void fe_hal_phy_generate_signal_tone(const cali_single_tone_stru *single_tone);

osal_void fe_hal_phy_cfg_cali_prepare(const hal_device_stru *device);
osal_void fe_hal_phy_config_trx_fifo(const hal_device_stru *device, osal_u8 fifo_en);
// 设置测试模式
osal_void fe_hal_phy_set_wlbb_testmode(const hal_device_stru *device, osal_u32 wlbb_testmode);
// 设置phy的测试开始/结束
osal_void fe_hal_phy_set_test_start(const hal_device_stru *device, osal_u32 test_start);
// FFT
osal_u32 fe_hal_phy_get_fft_by_freq(const hal_device_stru *device, cali_complex_stru *tone_fft,
    cali_complex_stru *image_fft, osal_s32 tone_freq);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
osal_void fe_hal_phy_config_tone_amp_debug(osal_u8 amp);
#endif
// TRX IQ
osal_u32 fe_hal_phy_get_fft_of_multi_tones(const hal_device_stru *device,
    cali_complex_stru *multi_tone_fft, osal_u16 tone_fft_size, cali_single_tone_stru tone_cfg);
osal_void fe_hal_phy_config_single_tone_beta_comp_chip(const hal_device_stru *device,
    cali_complex_stru *comp, osal_u16 comp_size, osal_u8 enable);
osal_void fe_hal_phy_config_ls_fir_tx_comp(const hal_device_stru *device, cali_complex_stru *comp, osal_u8 lvl_idx);
osal_void fe_hal_phy_config_ls_fir_rx_comp(const hal_device_stru *device, cali_complex_stru *comp, osal_u8 lvl_idx);
osal_void fe_hal_phy_config_tx_iq_comp(const hal_device_stru *device, osal_u32 bypass);
osal_void fe_hal_phy_config_rx_iq_comp(const hal_device_stru *device, osal_u32 bypass);
osal_void fe_hal_phy_iq_set_cali_path(const hal_device_stru *device);
osal_void fe_hal_phy_iq_recover_cali_path(const hal_device_stru *device);
// abb 时钟配置
osal_void fe_hal_phy_txpwr_comp_bypass(const hal_device_stru *device);

// 应用rx_dc数字补偿
osal_void fe_hal_phy_apply_rx_dc_digital_comp(const hal_device_stru *device, cali_complex_stru *rx_dc_digital_comp);
// tx_dc校准phy的准备配置
osal_void fe_hal_phy_tx_dc_env_prepare(const hal_device_stru *device);
// TX DC校准中，发射端由CBB添加一个可配的DC3（如15mV）到I/Q两路上
osal_void fe_hal_phy_tx_dc_add_dc(const hal_device_stru *device, osal_u16 reg_val_i, osal_u16 reg_val_q);
// tx_dc校准phy配置恢复
osal_void fe_hal_phy_tx_dc_env_restore(const hal_device_stru *device);
osal_void fe_hal_phy_set_tx_dc_comp_by_idx(const hal_device_stru *device, osal_u16 txdc_cmp_i,
    osal_u16 txdc_cmp_q, osal_u8 idx);
osal_void fe_hal_phy_rx_dc_set_up(const hal_device_stru *device);
osal_void fe_hal_phy_rx_dc_restore(const hal_device_stru *device);
osal_void fe_hal_phy_set_rx_dc_comp(const hal_device_stru *device, osal_u16 *comp, osal_u8 comp_size, osal_u8 idx);
// 上电功率的硬件常发
osal_void fe_hal_get_txpwr_cali_hw_al_tx_cfg(fe_hal_phy_al_tx_para_stru *param);
osal_void fe_hal_phy_config_hw_al_tx(const hal_device_stru *device, fe_hal_phy_al_tx_para_stru *param);
osal_void fe_hal_phy_recover_hw_al_tx(const hal_device_stru *device);

// 上电功率校准前配置dbb_scale
osal_void fe_hal_phy_set_dbb_scale_pre(osal_u16 dbb_scale);

// rc校准
osal_void fe_hal_phy_rc_set_up_regs(const hal_device_stru *device);
osal_void fe_hal_phy_rc_recover_regs(const hal_device_stru *device);
osal_u32 fe_hal_phy_get_cal_trx_iq_ctrl1_rpt_rx_fft_done(const hal_device_stru *device);
cali_complex_stru fe_hal_phy_get_cal_trxiq_fft_0_rpt_fft_idx0(const hal_device_stru *device);
osal_void fe_hal_phy_rc_start_sa(const hal_device_stru *device, osal_u16 index);
osal_void fe_hal_phy_rc_recover_sa(const hal_device_stru *device);
osal_void fe_hal_phy_rc_bypass_dc(const hal_device_stru *device);
osal_u8 fe_hal_phy_get_dac_fs_cfg_dac_fs(const hal_device_stru *device);

osal_u32 fe_hal_phy_get_cal_accum_rpt_ready_rpt_accum_ready(const hal_device_stru *device);
osal_u32 fe_hal_phy_get_cal_accum_i_cal0_accum_i(const hal_device_stru *device);
osal_u32 fe_hal_phy_get_cal_accum_q_cal0_accum_q(const hal_device_stru *device);
osal_void fe_hal_phy_set_tx_control_cfg_en_cont_tx(const hal_device_stru *device, osal_u32 val);
osal_void fe_hal_phy_set_tx_power_accum_delay_power_accum_delay(const hal_device_stru *device, osal_u32 val);
osal_void fe_hal_phy_set_cali_new_add_reg_cfg_fb_pwr_calc_prd(const hal_device_stru *device, osal_u32 val);
osal_u32 fe_hal_phy_get_cal_accum_i_rpt_accum_i_ready(const hal_device_stru *device);
osal_u32 fe_hal_phy_get_cal_accum_q_rpt_accum_q_ready(const hal_device_stru *device);
osal_void fe_hal_phy_set_cal_accum_en_cfg_accum_en(const hal_device_stru *device, osal_u32 val);
// soc 校准相关配置
osal_void fe_hal_soc_config_cali_path(const hal_device_stru *device, osal_u8 cali_type, osal_u8 open);
osal_u32 fe_hal_phy_get_single_pa_lut(const hal_device_stru *device, osal_u16 index);
osal_u32 fe_hal_phy_get_power_dist_by_tpc_code(osal_u8 tpc_code);
// TRX IQ补偿的档位界限值
osal_void fe_hal_phy_set_trx_iq_thres(hal_device_stru *device);
osal_void fe_hal_phy_set_rx_dc_thres(hal_device_stru *device);
typedef osal_void (*fe_hal_phy_set_reg_func)(osal_u32);
typedef osal_u32 (*fe_hal_phy_get_reg_func)(osal_void);
typedef osal_void (*fe_hal_common_set_reg_path_func)(osal_u8);
osal_u16 fe_hal_phy_get_fft_freq_to_index(osal_s32 tone_freq);
#ifdef BOARD_ASIC_WIFI
osal_void cali_tx_pwr_init_lvl_cfg(cali_txpwr_lvl_cfg_stru* lvl_cfg);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __CALI_PHY_IF_H__