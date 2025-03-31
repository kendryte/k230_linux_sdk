/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: ws73 rf reg interface and struct adaption
 * Date: 2022-11-12
*/

#ifndef __FE_HAL_RF_REG_IF_PKTMEM_H__
#define __FE_HAL_RF_REG_IF_PKTMEM_H__
#include "osal_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* efuse读取失败时,设定的默认值 */
#define CALI_RF_DEFAULT_R_CODE 0x0
#define CALI_RF_DEFAULT_RC_CODE 0x2C
#define CALI_RF_DEFAULT_IPA_CURRENT 0x8

osal_u32 hal_rf_get_reg_base(osal_u8 rf_id);
osal_u32 hal_rf_get_pll_reg_base(osal_u8 rf_id);
osal_void hal_rf_set_pll6_d_refbuf_doubler_en(osal_u16 x4OwmtyStO4vSyxmwOmo_);
osal_void hal_rf_set_pll7_d_dtc_r_sel(osal_u16 x4O4RCOwOsmx_);
osal_void hal_rf_set_pll8_d_dtc_sync_r_sel(osal_u16 x4O4RCOsQoCOwOsmx_);
osal_void hal_rf_set_pll8_d_dtc_sync_dly_sel(osal_u16 xFwFL6wCth6wFotwCWo_);
osal_void hal_rf_set_pll8_d_dtc_sync_rst_dly_sel(osal_u16 x4O4RCOsQoCOwsRO4xQOsmx_);
osal_void hal_rf_set_pll15_d_ndiv_pw_sel(osal_u16 xFwhFBqwuTwCWo_);
osal_void hal_rf_set_pll26_twarmup(osal_u16 twarmup);
osal_void hal_rf_set_pll26_tphaseslip(osal_u16 tphaseslip);
osal_void hal_rf_set_pll35_tsettle_fll_cp(osal_u16 xRsmRRxmOtxxOCG_);
osal_void hal_rf_set_pll35_sampling_wait_time(osal_u16 xszIGxlo2OqzlRORlIm_);
osal_void hal_rf_set_pll37_ckref_dcc_en(osal_u16 xCdwmtO4CCOmo_);
osal_void hal_rf_set_pll39_cfg_ckref_dcc_lock(osal_u16 xCt2OCdwmtO4CCOxvCd_);
osal_void hal_rf_set_pll39_cfg_dtc_gain_cal_lock(osal_u16 xCt2O4RCO2zloOCzxOxvCd_);
osal_void hal_rf_set_pll39_cfg_ana_data_lock(osal_u16 xCt2OzozO4zRzOxvCd_);
osal_void hal_rf_set_pll40_alpha_lms_ckrefdcc_high(osal_u16 xGouQGwoSCw6K_WNF66wQBgQ_);
osal_void hal_rf_set_pll41_alpha_lms_dtc_gc_high(osal_u16 xzxGAzOxIsO4RCO2COAl2A_);
osal_void hal_rf_set_pll43_d_reg_ckref_dcc_msb(osal_u16 x4Owm2OCdwmtO4CCOIsy_);
osal_void hal_rf_set_pll44_d_reg_ckref_dcc_lsb(osal_u16 x4Owm2OCdwmtO4CCOxsy_);
osal_void hal_rf_set_pll44_d_reg_dtc_gain_cal(osal_u16 x4Owm2O4RCO2zloOCzx_);
osal_void hal_rf_set_pll49_wait_time_dcc_cal_first(osal_u16 xTGBLwLBSWwF66w6GowNB_CL_);
osal_void hal_rf_set_pll49_wait_time_alpha_lms_ckrefdcc_high(osal_u16 xTGBLwLBSWwGouQGwoSCw6K_WNF66wQBgQ_);
osal_void hal_rf_set_pll49_wait_time_alpha_lms_dtc_gc_high(osal_u16 xqzlRORlImOzxGAzOxIsO4RCO2COAl2A_);
osal_void hal_rf_set_pll50_wait_time_alpha_vos_cal_high(osal_u16 xqzlRORlImOzxGAzOPvsOCzxOAl2A_);
osal_void hal_rf_set_pll50_pll_ld_wait_time(osal_u16 xGxxOx4OqzlRORlIm_);
osal_void hal_rf_set_pll51_wait_time_bg_cal_out(osal_u16 xqzlRORlImOy2OCzxOvSR_);
osal_u16 hal_rf_get_pll56_d_reg_ckref_dcc_msb_rb(osal_void);
osal_u16 hal_rf_get_pll57_d_reg_ckref_dcc_lsb_rb(osal_void);
osal_u16 hal_rf_get_pll57_d_reg_dtc_gain_cal_rb(osal_void);
osal_void hal_set_sysldo_soft_rst_ctl_soft_rst_rf_wb_n(osal_u32 soft_rst_rf_wb_n);
osal_void hal_rf_set_abb80_d_wb_rf_tx_pa_vb_sel_manual(osal_u16 xFwTjw_NwLPwuGwqjwCWowSGhdGo_);
osal_void hal_rf_set_abb109_d_wb_rfabb_dcoc_coarsetrim_to_ls(osal_u16 x4OqyOwtzyyO4CvCOCvzwsmRwlIORvOxs_);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of __FE_HAL_RF_REG_IF_PKTMEM_H__ */
