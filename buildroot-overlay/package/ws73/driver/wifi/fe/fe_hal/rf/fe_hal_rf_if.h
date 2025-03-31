/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准用到的rf接口
 * Create: 2022-09-16
 */

#ifndef __FE_HAL_RF_IF_H__
#define __FE_HAL_RF_IF_H__

#include "osal_types.h"
#include "cali_outer_hal_if.h"
#include "cali_complex_num.h"
#include "cali_outer_os_if.h"
#include "cali_channel.h"
#include "fe_hal_rf_reg_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    osal_u16 reg_addr;
    osal_u16 cfg_val;
    osal_u16 recover_val;
    fe_hal_rf_reg_type_enum_uint8 reg_type;
} cali_path_cfg_reg_stru;

// 配置logen cap
osal_void fe_hal_rf_cali_set_logen_cap_code(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 freq,
    osal_u8 cali_subband_num, osal_u16 reg_val);

// logen校准rf path配置
cali_path_cfg_reg_stru *fe_hal_rf_get_logen_path_cfg(osal_u32* list_count, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);

// 获取txpwr的rf path配置
cali_path_cfg_reg_stru *fe_hal_rf_get_txpwr_path_cfg(osal_u32* list_count, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);

// 获取rx_dc的rf path配置
cali_path_cfg_reg_stru* fe_hal_rf_get_rx_dc_path_cfg(osal_u32* list_count, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);

// 获取tx_dc的rf path配置
cali_path_cfg_reg_stru* fe_hal_rf_get_tx_dc_path_cfg(osal_u32* list_count, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);

// RC-bandwidth校准rf path配置
cali_path_cfg_reg_stru *fe_hal_rf_get_rc_path_cfg(osal_u32* list_count, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
// IQ 校准通路配置
cali_path_cfg_reg_stru* fe_hal_rf_get_tx_iq_path_cfg(osal_u32* list_count, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
cali_path_cfg_reg_stru* fe_hal_rf_get_rx_iq_path_cfg(osal_u32* list_count, wlan_channel_band_enum_uint8 band,
    cali_bandwidth_enum_uint8 bw, osal_u8 chain);
// 相角相关接口
osal_void fe_hal_rf_set_phase_val(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    alg_rf_cali_iq_type_enum_uint8 iq_cali_type, osal_u8 val);
osal_u8 fe_hal_rf_get_phase_curr_val(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    alg_rf_cali_iq_type_enum_uint8 iq_cali_type);
osal_u32 fe_hal_rf_get_phase_config(const hal_device_stru *device, wlan_channel_band_enum_uint8 band,
    alg_rf_cali_iq_type_enum_uint8 iq_cali_type, osal_u8 *reg_val1, osal_u8 *reg_val2);
// 校准完成后恢复rf寄存器为初始值
osal_void fe_hal_rf_init_when_cali_finish(osal_void);

osal_void fe_hal_rf_set_reg_rf_regctl(hal_device_stru *device);
osal_void fe_hal_rf_set_reg_cbb_linectl(hal_device_stru *device);

osal_void fe_hal_rf_txrx_enable_cfg(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_set_channel_cfg(hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    cali_channel_stru *cali_channel, osal_u8 channel_idx, cali_bandwidth_enum_uint8 cali_bandwidth);
osal_void fe_hal_rf_config_fb_gain(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 fb_idx);
osal_void fe_hal_rf_config_lpf_idx(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 lpf_idx);
osal_u8 fe_hal_rf_get_iq_cali_lvl_num(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    osal_u8 iq_type);
osal_u16 fe_hal_rf_get_tx_iq_cali_lvl_tpc_idx(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    osal_u8 lvl_num);
osal_u16 fe_hal_rf_get_rx_iq_cali_lvl_tpc_idx(const hal_rf_dev *rf_dev,
    wlan_channel_band_enum_uint8 band, osal_u8 lvl_num);
osal_void fe_hal_rf_config_tx_iq_cali_rx_gain(const hal_device_stru *device, osal_u8 cali_band, osal_u8 lvl_idx);
osal_u16 fe_hal_rf_get_tx_dc_cali_lvl_tpc_idx(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    osal_u8 lvl_num);
osal_void fe_hal_rf_config_cali_path_tx_gain(const hal_device_stru *device, osal_u8 cali_band,
    osal_u8 chn_idx, osal_u16 tpc_code);
osal_void fe_hal_rf_config_rx_iq_cali_rx_gain(const hal_device_stru *device, osal_u8 cali_band, osal_u8 lvl_idx);
osal_void fe_hal_rf_config_rx_iq_ppa_pa_gain(const hal_device_stru *device, osal_u8 cali_band, osal_u8 chn_idx);
osal_void fe_hal_rf_config_tx_dc_cali_rx_gain(const hal_device_stru *device, osal_u8 cali_band, osal_u8 lvl_idx);
osal_void fe_hal_rf_cali_set_pa_gainctl(const hal_rf_dev *rf_dev, osal_u8 reg_val);
osal_u8 fe_hal_rf_get_upc_ppa_bits(osal_void);
cali_complex_stru fe_hal_rf_get_tx_dc_dc3(osal_void);
osal_u16 fe_hal_rf_update_tx_dc_comp_val(osal_u16 comp, osal_s16 cali_temp);
osal_void fe_hal_rf_cali_set_upc_gc(const hal_rf_dev *rf_dev, osal_u8 freq,
    osal_u8 cali_subband_num, osal_u16 reg_val);
osal_void fe_hal_rf_cali_set_upc_gc_unit(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 freq,
    osal_u8 cali_subband_num, osal_u16 reg_val);
osal_void fe_hal_rf_cali_set_upc_gc_gm(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 freq,
    osal_u8 cali_subband_num, osal_u16 reg_val);
osal_void fe_hal_rf_cali_set_upc_ppa(const hal_rf_dev *rf_dev, osal_u8 freq,
    osal_u8 cali_subband_num, osal_u16 reg_val);
osal_void fe_hal_rf_cali_set_pa_code(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 freq,
    osal_u8 cali_subband_num, osal_u8 reg_val);
osal_void fe_hal_rf_wl_tx_modules_pa_ppa_en(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_wl_tx_modules_pa_ppa_dis(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_wl_cali_modules_en(const hal_rf_dev *rf_dev, osal_u8 reg_val);

// 配置DCOC寄存器控
osal_void fe_hal_rf_cfg_dcoc_reg_ctrl(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// 配置DCOC线控
osal_void fe_hal_rf_cfg_dcoc_line_ctrl(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// 配置DCOC
osal_void fe_hal_rf_cfg_dcoc_rxdc_init_val(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// RX增益设置为最大
osal_void fe_hal_rf_cfg_rx_agc_gain_max(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// RX增益设置为最小
osal_void fe_hal_rf_cfg_rx_agc_gain_min(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_cfg_vga_gain(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 vga_idx);

// 写dcoc寄存器
osal_void fe_hal_rf_write_dcoc(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 dcoc);
// 读dcoc寄存器
osal_void fe_hal_rf_read_dcoc(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 *dcoc);

// 打开lna
osal_void fe_hal_rf_rx_dc_open_lna(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_u16 fe_hal_rf_get_lna_gain_fix_data(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 lna_idx);
// 设置LNA增益
osal_void fe_hal_rf_cfg_lna_gain(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 lna_idx);

// 判断AGC gain是否RF寄存器控
osal_bool fe_hal_rf_is_agc_gain_reg_ctrl(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);

// 读取lna_idx
osal_u8 fe_hal_rf_get_lna_idx(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// 根据txdc档位设置对应的dac & lpf增益
osal_void fe_hal_rf_cfg_dac_lpf_gain_by_idx(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band,
    osal_u8 txdc_lvl_idx);

// disable pa
osal_void fe_hal_rf_disable_pa(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// enable pa
osal_void fe_hal_rf_enable_pa(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_tx_dc_env_restore(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// 判断tx gain是否RF寄存器控
osal_bool fe_hal_rf_is_tx_gain_reg_ctrl(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// 读取当前rf的lpf_idx
osal_u8 fe_hal_rf_get_lpf_idx(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// 读取当前rf的dac_idx
osal_u8 fe_hal_rf_get_dac_idx(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
// 根据rf 的dac 和 lpf idx 获取补偿 idx
osal_u8 fe_hal_rf_get_tx_dc_comp_idx(osal_u8 lpf_idx, osal_u8 dac_idx);
// RC
osal_void fe_hal_rf_rc_set_tx_rc_code(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 val);
osal_void fe_hal_rf_rc_set_rx_rc_code(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 val);
osal_void fe_hal_rf_rc_set_rc_code_complete(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_rc_set_rc_code_vga(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u16 val);
osal_void fe_hal_rf_get_rc_code_bit(const hal_rf_dev *rf_dev,
    wlan_channel_band_enum_uint8 band, osal_u16 *rc_bits);
osal_u8 fe_hal_rf_get_rc_side_freq(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_rx_dc_dcoc_coarsetrim(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
#ifdef BOARD_ASIC_WIFI
// R C iPA current 校准
osal_void fe_hal_rf_set_r_code_comp(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u8 r_code);
osal_void fe_rf_hal_get_efuse_r_code(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u8 *r_code);
osal_void fe_hal_rf_set_c_code_comp(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u8 c_code);
osal_u16 fe_hal_get_r_code_var(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u8 code);
osal_u16 fe_hal_get_c_code(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u8 code);
osal_void fe_hal_rf_set_ipa_current_comp(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band, osal_u8 bias);
osal_void fe_rf_hal_get_efuse_ipa_current_code(const hal_rf_dev *rf_dev,
    wlan_channel_band_enum_uint8 band, osal_u16 *efuse_code);
osal_u8 fe_rf_hal_cal_ipa_current_bias(const hal_rf_dev *rf_dev,
    wlan_channel_band_enum_uint8 band, osal_u16 efuse_code);
#endif
// 前端配置rf channel
osal_void fe_hal_rf_set_channel_info(hal_rf_chan_info *rf_chan);

osal_void fe_rf_initialize(osal_void);

// 前端初始化rf寄存器
osal_void fe_rf_initialize_rf_reg(hal_rf_dev *rf_dev);

osal_u32 fe_rf_get_reg_base(osal_u8 rf_id);
osal_u32 fe_rf_get_pll_reg_base(osal_u8 rf_id);
osal_u32 fe_rf_get_reg_offset(osal_u8 rf_id);

osal_void fe_rf_disable_abb(osal_u8 rf_id);

#ifdef BOARD_FPGA_WIFI
osal_u16 fe_rf_read_16(osal_u32 addr);
osal_void fe_rf_write_16(osal_u32 addr, osal_u16 data);
osal_void chip_ssi_write(hal_rf_dev *rf_dev, osal_u32 addr, osal_u16 data);
osal_void chip_ssi_read(hal_rf_dev *rf_dev, osal_u32 addr, osal_u16 *val);
#endif
osal_void fe_rf_write_reg(const hal_rf_dev *rf_dev, osal_u16 rf_addr_offset, osal_u16 rf_16bit_data,
    fe_hal_rf_reg_type_enum_uint8 reg_type);
osal_void  fe_rf_read_reg(const hal_rf_dev *rf_dev, osal_u16 rf_addr_offset, osal_u16 *reg_val,
    fe_hal_rf_reg_type_enum_uint8 reg_type);
osal_void fe_rf_write_reg_bits(const hal_rf_dev *rf_dev, osal_u16 reg_addr, osal_u8 offset, osal_u8 bits,
    osal_u16 reg_val);

osal_void fe_hal_proc_rf_work(osal_u8 rf_id);
osal_void fe_hal_proc_rf_sleep(osal_u8 rf_id);
osal_void fe_hal_rf_open_close_tx(osal_u8 rf_id, osal_u8 flag);
osal_void fe_hal_rf_open_close_pll(osal_u8 rf_id, osal_u8 pll_idx, osal_u8 flag);
osal_void fe_hal_rf_open_close_ldo(osal_u8 rf_id, osal_u8 flag);
#ifdef BOARD_FPGA_WIFI
osal_void fe_hal_soc_set_power_pgio_txen(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_soc_recover_power_pgio_trxen(const hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
#endif
// pktmem接口
osal_void fe_hal_rf_init_pll_lock(hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
osal_void fe_hal_rf_update_pll_in_out_freq(hal_rf_dev *rf_dev, wlan_channel_band_enum_uint8 band);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif