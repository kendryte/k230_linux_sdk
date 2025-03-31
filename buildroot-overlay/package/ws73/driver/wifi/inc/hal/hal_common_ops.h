/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: HAL Layer Specifications.
 * Create: 2020-7-3
 */

#ifndef __HAL_COMMON_OPS_H__
#define __HAL_COMMON_OPS_H__

/*****************************************************************************
    1 头文件包含
*****************************************************************************/
#include "oal_types.h"

#include "frw_list.h"
#include "oal_util_hcm.h"
#include "frw_util.h"
#include "oal_mem_hcm.h"
#include "oal_fsm.h"

#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "frw_ext_if.h"
#include "frw_osal.h"
#include "hal_commom_ops_type_rom.h"
#include "hal_common_ops_device.h"
#include "hal_ops_common.h"
#include "hal_ext_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    10.2 对外暴露的配置接口
*****************************************************************************/
#if (defined _PRE_BUILD_ALG_NEW_ARCH && defined _PRE_WLAN_FEATURE_ALG_MASTER_COMPILE)
#define hal_txop_alg_get_tx_dscr(txop_alg)           ((txop_alg)->ast_rank_param)
#else
#define hal_txop_alg_get_tx_dscr(txop_alg)           ((txop_alg)->tx_dsrc)
#endif

osal_void hh503_rx_init_dscr_queue(hal_to_dmac_device_stru *device, osal_u8 set_hw);
osal_void hh503_rx_destroy_dscr_queue(hal_to_dmac_device_stru *device);
hmac_pps_value_stru hh503_get_auto_adjust_freq_pps(osal_void);
osal_void hh503_tx_init_dscr_queue(hal_to_dmac_device_stru *device);

osal_void hh503_tx_fill_basic_ctrl_dscr(const hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *tx_dscr, hal_tx_mpdu_stru *mpdu);
osal_void hh503_tx_ctrl_dscr_link(hal_tx_dscr_stru *tx_dscr_prev, hal_tx_dscr_stru *tx_dscr);

osal_void hh503_tx_ctrl_dscr_copy_addr_info(hal_tx_dscr_stru *tx_dscr_prev, hal_tx_dscr_stru *tx_dscr);
osal_void hh503_tx_ctrl_dscr_get_addr_info(hal_tx_dscr_stru *tx_dscr, hal_tx_addr_info *tx_addr);

osal_u32 hh503_get_tx_vector_word6(osal_void);
osal_u32 hh503_get_rpt_tx_data_word0(osal_void);
osal_void hal_phy_rx_get_snr_info(hal_to_dmac_device_stru *hal_device, uint8_t phy_proto,
    uint16_t origin_data, int8_t *snr);
osal_void hh503_get_tx_dscr_next(hal_tx_dscr_stru *tx_dscr, hal_tx_dscr_stru **tx_dscr_next);
osal_void hh503_tx_ctrl_dscr_unlink(hal_tx_dscr_stru *tx_dscr);
osal_void hh503_tx_ucast_data_set_dscr(hal_to_dmac_device_stru *hal_device, hal_tx_dscr_stru *tx_dscr,
    hal_tx_txop_feature_stru *txop_feature, hal_tx_txop_alg_stru *txop_alg, hal_tx_ppdu_feature_stru *ppdu_feature);
osal_void hh503_tx_non_ucast_data_set_dscr(hal_to_dmac_device_stru *hal_device, hal_tx_dscr_stru *tx_dscr,
    hal_tx_txop_feature_stru *txop_feature, hal_tx_txop_alg_stru *txop_alg, hal_tx_ppdu_feature_stru *ppdu_feature);
osal_void hh503_tx_get_vap_id(const hal_tx_dscr_stru *tx_dscr, osal_u8 *vap_id);
osal_void hal_power_set_under_regulation(oal_bool_enum_uint8 under_regulation_flag);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
osal_void hh503_tx_set_pdet_en(const hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *tx_dscr, oal_bool_enum_uint8 pdet_en_flag);
osal_void hh503_dyn_cali_tx_pow_ch_set(hal_to_dmac_device_stru *hal_device, hal_tx_dscr_stru *tx_dscr);
#endif

osal_void hh503_tx_get_dscr_status(const hal_tx_dscr_stru *tx_dscr, osal_u8 *status);
osal_void hh503_tx_get_dscr_msdu_num(hal_tx_dscr_stru *tx_dscr, osal_u16 *msdu_num);
osal_u32  hh503_get_tx_q_status(osal_u8 qnum);
osal_u32  hh503_get_tx_q_status_empty(osal_void);

osal_void hh503_set_bcn_timeout_multi_q_enable(const hal_to_dmac_vap_stru *hal_vap, osal_u8 enable);

osal_void hh503_get_hal_vap(const hal_to_dmac_device_stru *hal_device_base, osal_u8 vap_id,
                            hal_to_dmac_vap_stru **hal_vap);

#ifdef _PRE_WLAN_RF_AUTOCALI
osal_void hh503_rf_cali_auto_switch(osal_u32 switch_mask);
#endif

osal_void hh503_get_sr_info(osal_u8 param);
osal_void hh503_set_psr_vfs_get(dmac_sr_sta_vfs_stru *hmac_sr_sta_vfs);
osal_void hh503_set_sr_vfs_get(dmac_sr_sta_vfs_stru *hmac_sr_sta_vfs);
osal_void hh503_set_sr_statistics_get(dmac_sr_sta_srg_stru *hmac_sr_sta_srg,
    dmac_sr_sta_non_srg_stru *hmac_sr_sta_non_srg);
osal_void hh503_set_sr_vfs_end(osal_void);
osal_void hh503_set_psr_statistics_end(osal_void);
osal_void hh503_set_sr_statistics_end(osal_void);
osal_void hh503_set_sr_vfs_start(osal_void);
osal_void hh503_set_psr_statistics_start(osal_void);
osal_void hh503_set_sr_statistics_start(osal_void);
osal_void hh503_set_sr_statistics_continue(osal_void);
osal_void hh503_set_psr_statistics_continue(osal_void);
osal_void hh503_set_sr_statistics_stop(osal_void);
osal_void hh503_set_psr_statistics_stop(osal_void);
osal_void hh503_set_sr_vfs_clear(osal_void);
osal_void hh503_set_psr_statistics_clear(osal_void);
osal_void hh503_set_sr_statistics_clear(osal_void);
osal_void hh503_set_psr_ctrl(oal_bool_enum_uint8 flag);
osal_void hh503_set_psrt_htc(osal_void);
osal_void hh503_set_psr_offset(osal_s32 offset);
osal_void hh503_set_bss_color_enable(oal_bool_enum_uint8 flag);
osal_void hh503_set_srg_pd_etc(osal_u32 pd, osal_u32 pw);
osal_void hh503_set_non_pd(osal_u32 pd, osal_u32 pw);
osal_void hh503_set_srg_ctrl(osal_void);
osal_void hh503_set_srg_partial_bssid_bitmap(osal_u32 bitmap_low, osal_u32 bitmap_high);
osal_void hh503_set_srg_bss_color_bitmap(osal_u32 bitmap_low, osal_u32 bitmap_high);
osal_void hh503_set_srg_ele_off(osal_void);
osal_void hh503_set_sr_ctrl(oal_bool_enum_uint8 flag);
osal_void hh503_color_rpt_clr(osal_void);
osal_void hh503_color_rpt_en(oal_bool_enum_uint8 flag);
osal_void hh503_color_area_get(mac_color_area_stru *color_area);

osal_void hh503_set_dev_support_11ax(hal_to_dmac_device_stru *hal_device, osal_u8 reg_value);
osal_void hh503_set_mac_backoff_delay(osal_u32 back_off_time);
osal_void hh503_set_mu_edca_lifetime(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo);
osal_void hh503_set_mu_edca_aifsn(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo);
osal_void hh503_set_mu_edca_func_en(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo);
osal_void hh503_close_mu_edca_func(osal_void);
osal_void hh503_set_affected_acs(osal_u8 affected_acs);
osal_void hh503_set_mu_edca_cw(osal_u8 ac_type, osal_u8 cwmax, osal_u8 cwmin);
osal_void hh503_set_bss_color(const hal_to_dmac_vap_stru *hal_vap, osal_u8 bss_color);
osal_void hh503_set_partial_bss_color(const hal_to_dmac_vap_stru *hal_vap, osal_u8 partial_bss_color);
osal_void hh503_set_phy_aid(osal_u16 aid);
osal_void hh503_set_queue_size_in_ac(osal_u32 queue_size, osal_u8 vap_id, osal_u16 ac_order);
osal_void hh503_set_ndp_status(osal_u8 status);

#ifdef _PRE_WLAN_FEATURE_WUR_TX
osal_void hh503_vap_send_wur_beacon_pkt(const wur_bcn_reg_param_stru *wur_bcn_param);
osal_void hh503_vap_set_wur_twbtt(osal_u16 wur_offset_twbtt, osal_u8 twbtt_en);
osal_void hh503_vap_set_wur_beacon_en(osal_u8 wur_bcn_data_rate, osal_u8 wur_bcn_vap_en);
osal_void hh503_write_wur_beacon(osal_u16 id);
osal_void hh503_write_wur_fl_wake(const wur_wakeup_reg_param_stru *wur_wakeup_param);
osal_void hh503_write_wur_short_wake(osal_u16 wur_id);
osal_void hh503_vap_set_wur(const hal_to_dmac_vap_stru *hal_vap, const wur_reg_param_stru *wur_param,
    osal_u8 count);
#endif

osal_void hal_vap_set_multi_bssid(osal_u8 *trans_bssid, osal_u8 maxbssid_indicator);
osal_void hh503_set_he_rom_en(oal_bool_enum_uint8 rx_om);

osal_void hh503_set_machw_rx_buff_addr(osal_u32 rx_dscr, hal_rx_dscr_queue_id_enum_uint8 queue_num);

osal_void  hh503_rx_add_dscr(hal_to_dmac_device_stru *hal_device, hal_rx_dscr_queue_id_enum_uint8 queue_num);
osal_void hh503_rx_update_dscr(hal_to_dmac_device_stru *hal_device, hal_rx_dscr_queue_id_enum_uint8 queue_num);

oal_bool_enum_uint8 hh503_is_machw_enabled(osal_void);
osal_void hh503_set_machw_tx_suspend(osal_void);
osal_void hal_set_machw_tx_resume(osal_void);
osal_void hh503_reset_phy_machw(hal_reset_hw_type_enum_uint8 type, osal_u8 sub_mod, osal_u8 reset_phy_reg,
    osal_u8 reset_mac_reg);
osal_void hh503_disable_machw_phy_and_pa(osal_void);
osal_void hh503_enable_machw_phy_and_pa(const hal_to_dmac_device_stru *hal_device);
osal_void hh503_recover_machw_phy_and_pa(hal_to_dmac_device_stru *hal_device_base);
osal_void hal_initialize_machw(osal_void);

osal_void hh503_initialize_machw_common(osal_void);
osal_void hal_set_ax_ctl_by_protocol(wlan_protocol_enum_uint8 protocol);

osal_void hh503_set_bandwidth_mode(hal_to_dmac_device_stru *hal_device, wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_void hh503_set_upc_data(hal_to_dmac_device_stru *hal_device);
osal_void hh503_set_txop_check_cca(osal_u8 txop_check_cca);
osal_u8 hh503_set_machw_phy_adc_freq(hal_to_dmac_device_stru *hal_device,
    wlan_channel_bandwidth_enum_uint8 bandwidth);

osal_void hh503_set_ana_dbb_ch_sel(const hal_to_dmac_device_stru *hal_device);
osal_void hh503_update_cbb_cfg(hal_to_dmac_device_stru *hal_device);

osal_void hh503_set_tx_sequence_num(machw_tx_sequence_stru tx_seq, osal_u32 val_write, osal_u8 vap_index);
osal_void hh503_reset_save_mac_reg(osal_void);
osal_void hh503_disable_machw_ack_trans(osal_void);
osal_void hh503_enable_machw_ack_trans(osal_void);
osal_void hh503_disable_machw_cts_trans(osal_void);
osal_void hh503_enable_machw_cts_trans(osal_void);
osal_void hal_initialize_phy(osal_void);
osal_void hh503_radar_config_reg(hal_dfs_radar_type_enum_uint8 radar_type);
osal_void hh503_radar_config_reg_bw(hal_dfs_radar_type_enum_uint8 radar_type,
    wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_void hh503_radar_enable_chirp_det(oal_bool_enum_uint8 chirp_det);
osal_void hh503_initialize_rf_sys(hal_to_dmac_device_stru *hal_device);
osal_void hal_pow_sw_initialize_tx_power(hal_to_dmac_device_stru *hal_device);
osal_void hh503_pow_initialize_tx_power(hal_to_dmac_device_stru *hal_device);
osal_void hh503_pow_set_rf_regctl_enable(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 rf_linectl);
osal_void hal_pow_set_resp_frame_tx_power(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u8 chan_idx);
osal_void hh503_pow_set_band_spec_frame_tx_power(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u8 chan_idx);
osal_void hh503_pow_get_spec_frame_data_rate_idx(osal_u8 rate,  osal_u8 *rate_idx);
osal_u16 hh503_pow_get_pow_table_tpc_code(osal_u8 band, osal_u8 rate, osal_u8 bw, osal_u8 pwr_lvl);
osal_void hh503_pow_get_data_rate_idx(osal_u8 protocol_mode, osal_u8 mcs, wlan_bw_cap_enum_uint8 bw, osal_u8 *rate_idx);
osal_void hh503_pow_set_pow_code_idx_in_tpc(hal_tx_txop_tpc_stru *tpc, osal_u32 *tpc_code, osal_u16 tpc_code_size);
#ifdef _PRE_WLAN_FEATURE_11AX
osal_void hh503_tb_mcs_tx_power(wlan_channel_band_enum_uint8 band);
osal_void hh503_tb_tx_power_init(wlan_channel_band_enum_uint8 band);
#endif
#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
osal_void  hh503_pow_set_user_resp_frame_tx_power(hal_to_dmac_device_stru *hal_device,
    osal_u8 lut_index, osal_u8 rssi_distance);
osal_void hh503_pow_del_machw_resp_power_lut_entry(osal_u8 lut_index);
#endif

osal_void hh503_cali_send_func(hal_to_dmac_device_stru *hal_device,
    osal_u8 *cali_data, osal_u16 frame_len, osal_u16 remain);
osal_void hh503_initialize_soc(hal_to_dmac_device_stru *hal_device);
osal_u32  hh503_get_mac_int_status(osal_void);
osal_void hh503_clear_mac_int_status(osal_u32 status);
osal_void hh503_clear_mac_error_int_status(const hal_error_state_stru *status);
osal_void hh503_show_irq_info(hal_to_dmac_device_stru *hal_device, osal_u8 param);
osal_void hh503_clear_irq_stat(hal_to_dmac_device_stru *hal_device_base);
osal_void hh503_add_vap(hal_to_dmac_device_stru *hal_device_base, wlan_vap_mode_enum_uint8 vap_mode,
    osal_u8 hal_vap_id, hal_to_dmac_vap_stru **hal_vap);
osal_void hh503_del_vap(hal_to_dmac_device_stru *hal_device, osal_u8 vap_id);
#ifdef _PRE_WLAN_FEATURE_M2S
osal_void hh503_update_datarate_by_chain(const hal_to_dmac_device_stru *hal_device, osal_u8 resp_tx_chain);
osal_void hh503_get_reg_he_rom_nss(osal_u32 *peer_rom_nss);
#endif

osal_void hh503_config_eifs_time(wlan_protocol_enum_uint8 protocol);

osal_void hh503_clear_hw_fifo(struct tag_hal_to_dmac_device_stru *hal_device_base);
osal_void hh503_mask_interrupt(osal_u32 offset);
osal_void hh503_unmask_interrupt(osal_u32 offset);
osal_void hh503_reg_info(hal_to_dmac_device_stru *hal_device, osal_u32 addr, osal_u32 *val);
osal_void hh503_reg_info16(hal_to_dmac_device_stru *hal_device, osal_u32 addr, osal_u16 *val);

#ifdef _PRE_WLAN_ONLINE_DPD
osal_void hh503_dpd_cfr_set_tpc(hal_to_dmac_device_stru *hal_device, osal_u8 tpc);
osal_void hh503_dpd_cfr_set_bw(hal_to_dmac_device_stru *hal_device, osal_u8 bw);
osal_void hh503_dpd_cfr_set_mcs(hal_to_dmac_device_stru *hal_device, osal_u8 mcs);
osal_void hh503_dpd_cfr_set_freq(hal_to_dmac_device_stru *hal_device, osal_u8 freq);
osal_void hh503_dpd_cfr_set_11b(hal_to_dmac_device_stru *hal_device, osal_u8 is_11b);
#endif
osal_u32  hh503_get_all_tx_q_status(osal_void);
osal_void hh503_get_dieid(hal_to_dmac_device_stru *hal_device, osal_u32 *dieid, osal_u32 *length);
osal_void hh503_disable_beacon_filter(osal_void);
osal_void hh503_enable_non_frame_filter(osal_void);
osal_void hh503_set_pmf_crypto(hal_to_dmac_vap_stru *hal_vap, oal_bool_enum_uint8 crypto);
osal_void hh503_ce_add_key(hal_security_key_stru *security_key);
osal_void hh503_ce_del_key(const hal_security_key_stru *security_key);
osal_void hh503_ce_get_key(const hal_security_key_stru *security_key);
osal_void hh503_disable_ce(osal_void);
osal_void hh503_ce_add_peer_macaddr(osal_u8 lut_idx, const osal_u8 *addr);
osal_void hh503_ce_del_peer_macaddr(osal_u8 lut_idx);
osal_void hh503_get_rate_80211g_table(const mac_data_rate_stru **rate);
osal_void hh503_get_rate_80211g_num(osal_u32 *rate_num);
osal_void hh503_get_hw_addr(osal_u8 *addr);
osal_void hh503_chan_measure_start(hal_chan_measure_stru *meas);
osal_void hh503_get_ch_statics_result(hal_ch_statics_irq_event_stru *ch_statics);
osal_void hal_get_ch_measurement_result_ram(hal_ch_statics_irq_event_stru *ch_statics);
osal_void hal_get_ch_measurement_result(hal_ch_statics_irq_event_stru *ch_statics);
osal_void hh503_enable_radar_det(osal_u8 enable);
osal_u32 hh503_read_max_temperature(osal_s16 *ps_temperature);
oal_bool_enum_uint8 hh503_chip_is_cldo_mode(osal_void);
osal_void hh503_get_radar_det_result(hal_radar_det_event_stru *radar_info);
osal_void  hh503_get_spec_frm_rate(hal_to_dmac_device_stru *hal_device);
osal_void  hh503_set_bcn_phy_tx_mode(const hal_to_dmac_vap_stru *hal_vap, osal_u32 pow_code);
osal_void hh503_set_spec_frm_phy_tx_mode(const hal_to_dmac_device_stru *hal_device,
    osal_u8 band, osal_u8 subband_idx);
osal_void hh503_set_resp_pow_level(osal_char near_distance_rssi, osal_char far_distance_rssi);
osal_void  hh503_get_bcn_rate(const hal_to_dmac_vap_stru *hal_vap, osal_u8 *data_rate);

#ifdef _PRE_WLAN_FEATURE_TXBF
osal_void hh503_set_h_matrix_timeout(osal_u32 reg_value);
osal_void hh503_set_dl_mumimo_ctrl(oal_bool_enum_uint8 enable);
osal_void hh503_set_mu_aid_matrix_info(const hal_to_dmac_vap_stru *hal_vap, osal_u16 aid);
osal_void hh503_set_bfer_subcarrier_ng(wlan_bw_cap_enum_uint8 user_bw);
osal_void hh503_set_txbf_he_buff_addr(osal_u32 addr, osal_u16 buffer_len);
osal_void hh503_set_bfee_h2v_beamforming_ng(osal_u8 user_bw);
osal_void hh503_set_bfee_grouping_codebook(osal_u8 codebook, osal_u8 min_group);
osal_void hh503_set_bfee_sounding_en(osal_u8 bfee_enable);
osal_void hh503_set_bfee_bypass_clk_gating(osal_u8 enable);
osal_void hh503_set_channel_est_for_txbfee(osal_u8 txbf, wlan_channel_bandwidth_enum_uint8 bandwidth,
    wlan_channel_band_enum_uint8 band);
#endif
/* phy debug信息中trailer信息上报设置寄存器 */

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
osal_void hh503_set_counter1_clear(osal_void);
osal_void hh503_set_mac_clken(oal_bool_enum_uint8 wctrl_enable);
#endif

osal_void hh503_get_phy_statistics_data(hal_mac_key_statis_info_stru *mac_key_statis);
osal_void hh503_get_mac_rx_statistics_data(hal_mac_rx_mpdu_statis_info_stru *mac_rx_statis);
osal_void hh503_get_mac_tx_statistics_data(hal_mac_tx_mpdu_statis_info_stru *mac_tx_statis);
osal_void hh503_hw_stat_clear(osal_void);
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
osal_void hh503_set_ed_high_th(osal_s8 ed_high_20_reg_val, osal_s8 ed_high_40_reg_val,
    oal_bool_enum_uint8 is_default_th);
osal_void hh503_set_cca_prot_th(osal_s8 sd_cca_20th_dsss_val, osal_s8 sd_cca_20th_ofdm_val);
osal_void hh503_enable_sync_error_counter(osal_s32 enable_cnt_reg_val);
osal_void hh503_get_sync_error_cnt(osal_u32 *reg_val);
osal_void hh503_set_sync_err_counter_clear(osal_void);
#endif
osal_void hh503_set_tbtt_offset(const hal_to_dmac_vap_stru *hal_vap, osal_u16 offset);
osal_void hh503_set_psm_ext_tbtt_offset(const hal_to_dmac_vap_stru *hal_vap, osal_u16 offset);
osal_void hh503_set_psm_beacon_period(const hal_to_dmac_vap_stru *hal_vap, osal_u32 beacon_period);
#ifdef _PRE_WLAN_FEATURE_SMPS
osal_void hh503_set_smps_mode(osal_u8 smps_mode);
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
osal_void hh503_set_txop_ps_enable(hal_to_dmac_device_stru *hal_device, osal_u8 on_off);
osal_void hh503_set_txop_ps_condition1(osal_u8 on_off);
osal_void hh503_set_txop_ps_condition2(osal_u8 on_off);
osal_void hh503_set_txop_ps_partial_aid(const hal_to_dmac_vap_stru  *hal_vap, osal_u32 partial_aid);
#endif

osal_void hh503_set_mac_aid(const hal_to_dmac_vap_stru *hal_vap, osal_u16 aid);
#ifndef _PRE_WLAN_FEATURE_WS63
osal_void hh503_psm_clear_mac_rx_isr(osal_void);

osal_void hh503_set_sec_bandwidth_offset(osal_u8 bandwidth);
#endif
osal_void hh503_enable_machw_edca(osal_void);
osal_void hh503_set_phy_max_bw_field(hal_to_dmac_device_stru *hal_device, osal_u32 data,
    hal_phy_max_bw_sect_enmu_uint8 sect);
osal_void hh503_rf_test_disable_al_tx(hal_to_dmac_device_stru *hal_device);
osal_void hh503_set_rx_filter_reg(osal_u32 rx_filter_command);
osal_void hh503_set_beacon_timeout(osal_u16 value);

#define HAL_VAP_LEVEL_FUNC_EXTERN
osal_void hh503_vap_tsf_get_32bit(const hal_to_dmac_vap_stru *hal_vap, osal_u32 *tsf_lo);
osal_void hh503_vap_tsf_get_64bit(const hal_to_dmac_vap_stru *hal_vap, osal_u32 *tsf_high, osal_u32 *tsf_lo);
osal_void hh503_vap_set_beacon_rate(const hal_to_dmac_vap_stru *hal_vap, osal_u32 beacon_rate);
osal_void hh503_vap_beacon_suspend(const hal_to_dmac_vap_stru *hal_vap);
osal_void hh503_vap_beacon_resume(const hal_to_dmac_vap_stru *hal_vap);
osal_void hh503_vap_set_machw_prot_params(hal_to_dmac_vap_stru *hal_vap,
    hal_tx_phy_mode_one_stru *phy_tx_mode, hal_tx_ctrl_desc_rate_stru *data_rate);


osal_void hh503_vap_set_macaddr(const hal_to_dmac_vap_stru *hal_vap, const osal_u8 *mac_addr, osal_u16 mac_addr_len);
osal_void hh503_vap_set_opmode(hal_to_dmac_vap_stru *hal_vap, wlan_vap_mode_enum_uint8 vap_mode);

osal_void hh503_vap_clr_opmode(const hal_to_dmac_vap_stru *hal_vap, wlan_vap_mode_enum_uint8 vap_mode);
osal_void hh503_vap_set_machw_aifsn_all_ac(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo);
osal_void hh503_vap_set_machw_aifsn_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u8 aifs);
osal_void hh503_vap_get_machw_aifsn_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u8 *aifs);
osal_void  hh503_vap_set_machw_aifsn_ac_wfa(wlan_wme_ac_type_enum_uint8 ac, osal_u8 aifs,
    wlan_wme_ac_type_enum_uint8 wfa_lock);
osal_void hh503_vap_set_edca_machw_cw(osal_u8 cwmax, osal_u8 cwmin, osal_u8 ac_type);
osal_void  hh503_vap_set_edca_machw_cw_wfa(osal_u8 cwmaxmin,
    osal_u8 ac_type, wlan_wme_ac_type_enum_uint8   wfa_lock);
osal_void hh503_vap_get_edca_machw_cw(osal_u8 *cwmax, osal_u8 *cwmin, osal_u8 ac_type);
osal_void hh503_vap_set_machw_txop_limit_bkbe(osal_u16 be, osal_u16 bk);
osal_void hh503_vap_get_machw_txop_limit_bkbe(osal_u16 *be, osal_u16 *bk);
osal_void hh503_vap_set_machw_txop_limit_vivo(osal_u16 vo, osal_u16 vi);
osal_void hh503_vap_get_machw_txop_limit_vivo(osal_u16 *vo, osal_u16 *vi);
osal_void hh503_vap_get_machw_txop_limit_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u16 *txop);
osal_void hh503_vap_set_txop_duration_threshold(osal_u32 txop_duration_th, osal_u32 vap_idx);
osal_void hh503_set_11ax_vap_idx(osal_u8 hal_vap_idx);
osal_void hh503_set_mac_freq(hal_clk_freq_enum_uint8 mac_clk_freq);
osal_void hh503_vap_set_machw_edca_bkbe_lifetime(osal_u16 be, osal_u16 bk);
osal_void hh503_vap_get_machw_edca_bkbe_lifetime(osal_u16 *be, osal_u16 *bk);
osal_void hh503_vap_set_machw_edca_vivo_lifetime(osal_u16 vo, osal_u16 vi);
osal_void hh503_vap_get_machw_edca_vivo_lifetime(osal_u16 *vo, osal_u16 *vi);
osal_void hh503_vap_read_tbtt_timer(const hal_to_dmac_vap_stru *hal_vap, osal_u32 *value);
osal_void hh503_vap_set_machw_beacon_period(const hal_to_dmac_vap_stru *hal_vap, osal_u16 beacon_period);
osal_void hh503_set_sta_bssid(const hal_to_dmac_vap_stru *hal_vap, const osal_u8 *bssid);
osal_void hh503_enable_tsf_tbtt(hal_to_dmac_vap_stru *hal_vap, oal_bool_enum_uint8 dbac_enable);
osal_void hh503_disable_tsf_tbtt(const hal_to_dmac_vap_stru *hal_vap);
osal_void hh503_tx_disable_resp_ps_bit_ctrl(osal_u8 lut_index);
osal_void hh503_tx_enable_peer_sta_ps_ctrl(const osal_u8 lut_index);
osal_void hh503_tx_disable_peer_sta_ps_ctrl(const osal_u8 lut_index);
osal_void hh503_cfg_slottime_type(osal_u32 slottime_type);
osal_void hh503_pm_wlan_get_service_id(hal_to_dmac_vap_stru  *hal_vap, osal_u8 *service_id);
osal_void hh503_pm_servid_unregister(osal_void);

osal_void hh503_pm_vote2platform(hal_pm_mode_enum_uint8 pm_mode);
osal_void hh503_init_pm_info(hal_to_dmac_vap_stru *hal_vap);
osal_void hh503_init_pm_info_sync(hal_to_dmac_vap_stru *hal_vap);
osal_u16 *hh503_pm_get_ext_inner_offset_diff(osal_void);
osal_void hh503_pm_set_bcn_rf_chain(hal_to_dmac_vap_stru *hal_vap, osal_u8 bcn_rf_chain);

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
osal_void hh503_tbtt_offset_probe_init(hal_to_dmac_vap_stru *hal_vap);
osal_void hh503_tbtt_offset_probe_destroy(hal_to_dmac_vap_stru *hal_vap);
#endif

#ifdef _PRE_WLAN_DFT_STAT
osal_void hh503_dft_set_phy_stat_node(oam_stats_phy_node_idx_stru *phy_node_idx);
osal_void hh503_dft_print_machw_stat(osal_void);
osal_void hh503_dft_print_phyhw_stat(osal_void);
osal_void hh503_dft_report_all_reg_state(hal_to_dmac_device_stru   *hal_device);
#endif

osal_u32 hh503_rf_get_pll_div_idx(wlan_channel_band_enum_uint8 band, osal_u8  channel_idx,
    wlan_channel_bandwidth_enum_uint8 bandwidth, osal_u8  *pll_div_idx);
osal_void hh503_get_cali_info(hal_to_dmac_device_stru *hal_device, osal_u8 *param);
osal_void hh503_get_rate_idx_pow(hal_to_dmac_device_stru *hal_device, osal_u8 pow_idx,
    wlan_channel_band_enum_uint8 freq_band, osal_u16 *powr, osal_u8 channel_idx);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_u32 hh503_config_custom_rf(const osal_u8 *param);
osal_void hh503_config_rssi_for_loss(osal_u8 channel_num, osal_s8 *rssi);
osal_void hh503_config_get_cus_cca_param(hal_cfg_custom_cca_stru **cfg_cca);
osal_void hh503_set_custom_rx_insert_loss(osal_u8 band, osal_u8 *insert_loss, osal_u8 len);
osal_u8 hh503_get_custom_rx_insert_loss(osal_u8 band, osal_u8 ch_idx);
osal_void hh503_set_custom_power(osal_u8 band, osal_u8 *power, osal_u16 len);
#endif
osal_u8 hh503_get_rf_subband_idx(osal_u8 band, osal_u8 ch_idx);
#ifdef _PRE_WLAN_FEATURE_FTM
osal_u64  hh503_get_ftm_time(osal_u64 time);
osal_u64  hh503_check_ftm_t4(osal_u64 time);
osal_char hh503_get_ftm_t4_intp(osal_u64 time);
osal_u64  hh503_check_ftm_t2(osal_u64 time);
osal_char hh503_get_ftm_t2_intp(osal_u64 time);
osal_u64  hh503_get_ftm_tod(osal_void);
osal_u64  hh503_get_ftm_toa(osal_void);
osal_u64  hh503_get_ftm_t2(osal_void);
osal_u64  hh503_get_ftm_t3(osal_void);
osal_void hh503_get_ftm_ctrl_status(osal_u32 *ftm_status);
osal_void hh503_get_ftm_config_status(osal_u32 *ftm_status);
osal_void hh503_set_ftm_ctrl_status(osal_u32 ftm_status);
osal_void hh503_set_ftm_config_status(osal_u32 ftm_status);
osal_void hh503_set_ftm_enable(oal_bool_enum_uint8 ftm_status);
osal_void hh503_set_ftm_sample(oal_bool_enum_uint8 ftm_status);
osal_u8   hh503_get_ftm_dialog(osal_void);
osal_void hh503_get_ftm_cali_rx_time(osal_u32 *ftm_cali_rx_time);
osal_void hh503_get_ftm_cali_rx_intp_time(osal_u32 *ftm_cali_rx_time);
osal_void hal_get_ftm_cali_tx_time(osal_u32 *ftm_cali_tx_time);
osal_void hal_set_ftm_cali(hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *tx_dscr, oal_bool_enum_uint8 ftm_cali);
osal_void hal_set_ftm_tx_cnt(hal_tx_dscr_stru *tx_dscr, osal_u8 ftm_tx_cnt);
osal_void hal_set_ftm_bandwidth(hal_tx_dscr_stru *tx_dscr, wlan_bw_cap_enum_uint8 band_cap);
osal_void hal_set_ftm_protocol(hal_tx_dscr_stru *tx_dscr, wlan_phy_protocol_enum_uint8 prot_format);
osal_void hal_set_ftm_m2s(hal_tx_dscr_stru *tx_dscr, osal_u8 tx_chain_selection);
osal_void hal_set_ftm_m2s_phy(oal_bool_enum_uint8 is_mimo, osal_u8 tx_chain_selection);
osal_void hal_get_ftm_rtp_reg(osal_u32 *reg0, osal_u32 *reg1, osal_u32 *reg2, osal_u32 *reg3, osal_u32 *reg4);
osal_void hal_get_ftm_intp_status(osal_u32 *ftm_status);
osal_void hal_get_ftm_new_status(osal_u32 *ftm_status);
osal_void hal_get_ftm_pulse_enable_status(osal_u32 *ftm_status);
osal_void hal_get_ftm_cnt_status(osal_u32 *ftm_status);
osal_void hal_get_ftm_crystal_oscillator_offset_update_status(osal_u32 *ftm_status);
osal_void hal_get_ftm_clock_offset_update_status(osal_u32 *ftm_status);
osal_void hal_get_ftm_phy_bw_mode(osal_u32 *ftm_status);
osal_void hal_get_ftm_intp_freq(osal_u32 *ftm_status);
osal_u64 hal_get_ftm_pulse_offset(osal_void);
osal_u64 hal_get_ftm_pulse_freq(osal_void);
osal_u32 hal_get_ftm_crystal_oscillator_offset(osal_void);
osal_u64 hal_get_ftm_clock_offset(osal_void);
osal_void hh503_set_ftm_intp_status(osal_u32 ftm_status);
osal_void hh503_set_ftm_new_status(osal_u32 ftm_status);
osal_void hal_set_ftm_step_count_status(osal_u32 ftm_status);
osal_void hh503_set_ftm_pulse_enable_status(osal_u32 ftm_status);
osal_void hal_set_ftm_cnt_status(osal_u32 ftm_status);
osal_void hal_set_ftm_crystal_oscillator_offset_update_status(osal_u32 ftm_status);
osal_void hh503_set_ftm_clock_offset_update_status(osal_u32 ftm_status);
osal_void hal_set_ftm_crystal_oscillator_offset(osal_u32 ftm_status);
osal_void hal_set_ftm_clock_offset(osal_u64 ftm_status);
osal_s64 hal_sat_crystal_oscillator_offset(osal_s64 clock_offset, osal_s64 delta);
osal_void hal_get_rx_digital_scaling_bypass(osal_u32 *bypass);
osal_void hal_get_rx_lpf1_bypass(osal_u32 *bypass);
osal_void hal_get_rxiq_filter_force_bypass(osal_u32 *bypass);

#endif
osal_void  hh503_vap_get_gtk_rx_lut_idx(const hal_to_dmac_vap_stru *hal_vap, osal_u8 *lut_idx);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
osal_void hh503_rf_init_dyn_cali_reg_conf(hal_to_dmac_device_stru *hal_device);
osal_void hh503_rf_dyn_cali_enable(hal_to_dmac_device_stru *device);
osal_void hh503_rf_dyn_cali_disable(hal_to_dmac_device_stru *device);
osal_void hh503_config_set_dyn_cali_dscr_interval(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u16 param_val);

#endif

osal_void hh503_flush_rx_queue_complete_irq(hal_to_dmac_device_stru *hal_dev,
    hal_rx_dscr_queue_id_enum_uint8 queue_num);

oal_bool_enum_uint8 hh503_check_mac_int_status(osal_void);
osal_void hal_device_init_vap_pow_code(hal_to_dmac_device_stru *hal_device,
    hal_vap_pow_info_stru *vap_pow_info, hal_pow_set_type_enum_uint8 type, const mac_channel_stru *channel);
osal_void hh503_set_hw_en_reg_cfg(osal_u8 hw_en);
osal_void hal_power_set_rate_power_offset(osal_u8 freq, osal_u8 rate, osal_s8 power_offset);
osal_s8 hal_power_get_rate_power_offset(osal_u8 freq, osal_u8 rate);
osal_void hh503_adjust_tx_power(osal_u8 ch, osal_s8 power);
osal_void hh503_restore_tx_power(osal_u8 ch);
osal_void hh503_set_rf_limit_power(wlan_channel_band_enum_uint8 band, osal_u8 power);
osal_u8 hh503_get_tpc_code(osal_void);

osal_void hh503_cfg_txop_cycle(osal_u16 txop_limit);
osal_void hh503_cfg_lifetime(osal_u8 ac, osal_u16 lifetime);
osal_void hh503_bl_sifs_en(osal_u8 sifs_en);
osal_void hh503_rts_info(osal_u32 rate, osal_u32 phy_mode, osal_u8* addr1);
osal_void hh503_bl_cfg_txop_en(osal_u8 txop_en);
osal_void hh503_cfg_tx_rts_mac_cnt(osal_u8 max_cnt);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
osal_u64 hal_get_dyn_cali_send_pack_num(osal_void);
osal_void hal_set_dyn_cali_send_pack_num(osal_u64 value);
#endif
osal_void hh503_set_rx_framefilter(osal_bool enable);

osal_void hh503_gp_set_wifi_flag(osal_u32 val);
osal_u32 hh503_gp_get_wifi_flag(osal_void);
osal_void hh503_gp_set_bfgn_flag(osal_u32 val);
osal_u32 hh503_gp_get_bfgn_flag(osal_void);
osal_u32 hal_gp_get_mpdu_count(osal_void);
osal_void hal_update_mac_freq(hal_to_dmac_device_stru *hal_device, wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_void hh503_set_replay_detect_en(osal_u8 enable);
osal_u8 hh503_get_replay_detect_en(osal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
