/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Initialization and calibration for ABB RF.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_rf.h"
#include "hal_soc.h"
#include "hal_mac.h"
#include "hal_phy.h"
#include "hal_ext_if.h"
#include "hal_power.h"
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "hal_coex_reg.h"
#endif
#include "hal_reset.h"
#include "mac_resource_ext.h"
#include "mac_regdomain.h"
#include "wlan_msg.h"
#include "hal_soc_reg.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "fe_rf_customize_rx_insert_loss.h"
#include "fe_rf_customize_power.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_RF_C

/*****************************************************************************
  1 宏定义
*****************************************************************************/
#define HAL_RF_STB_REG_NUM   50

hh503_adjust_tx_power_level_stru g_adjust_tx_power_level_table[] = {
    {-70, 0}, /* -70：发射功率；0：功率对应的tpccode */
    {-50, 20}, /* -50：发射功率；20：功率对应的tpccode */
    {-30, 40}, /* -30：发射功率；40：功率对应的tpccode */
    {-10, 60}, /* -10：发射功率；60：功率对应的tpccode */
    {10, 80}, /* 10：发射功率；80：功率对应的tpccode */
    {15, 85}, /* 15：发射功率；85：功率对应的tpccode */
};

/* 目前用于设置wifi aware action帧的发射功率 */
osal_u8 g_tpc_code = 0xff;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* rf 相关定制化结构体 */
hh503_rf_custom_stru g_rf_customize = {
    /* 插损 */
    /* gain_db_mult4  gain_db_mult10  reserv */
    /* 2g_band 1~3 */
    /* rf0 */
    {   {   {{-12, {0}}, {-12, {0}}, {-12, {0}}},
            /* 5g_band 1~7 */
            {{-8, {0}}, {-8, {0}}, {-8, {0}}, {-8, {0}}, {-8, {0}}, {-8, {0}}, {-8, {0}}}
        },
    },
    /* delta rssi */
    {{{0, 0}, {0, 0, 0, 0}}},

    /* 动态校准开关 2.4g 5g */
    {0, 0},

    /* far_dist_pow_gain_switch */
    1,

    /* 5g_enable   tone_amp_grade */
    1,          0,

    /* far_dist_dsss_scale_promote_switch */
    1,
    /* chann_radio_cap  jucntion temp diff   rsv */
    0xF,                0,                0,

    /* cca ed thr */
    /* delta cca_20M_2g   delta cca_40M_2g   delta cca_20M_5g   delta cca_40M_5g */
    {0,                0,                 0,                 0},

    /* over_temp_protect_thread   recovery_temp_protect_thread */
    110,                       100,
};
#endif // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

/*****************************************************************************
  3 函数实现
*****************************************************************************/
osal_u8 hh503_get_rf_subband_idx(osal_u8 band, osal_u8 ch_idx)
{
    // 不支持5G
    if (band != WLAN_BAND_2G) {
        return 0;
    }
    if (ch_idx < CHANNEL_NUM5_IDX) {
        return HH503_CALI_CHN_INDEX_2422;
    } else if (ch_idx < CHANNEL_NUM10_IDX) {
        return HH503_CALI_CHN_INDEX_2447;
    } else {
        return HH503_CALI_CHN_INDEX_2472;
    }
    return HH503_CALI_CHN_INDEX_2422;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
 功能描述  : 获取定制化cca 门限值
*****************************************************************************/
osal_void hh503_config_get_cus_cca_param(hal_cfg_custom_cca_stru **cfg_cca)
{
    *cfg_cca = &g_rf_customize.cfg_cus_cca;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
osal_u32 uapi_tsensor_read_temperature(osal_s16 *temperature);
/*****************************************************************************
 功能描述  : 获取温度值
*****************************************************************************/
osal_u32 hh503_read_max_temperature(osal_s16 *ps_temperature)
{
    osal_u32 ret;
    ret = uapi_tsensor_read_temperature(ps_temperature);
    *ps_temperature += g_rf_customize.junction_temp_diff;
    return ret;
}
#endif

#ifdef _PRE_WLAN_ONLINE_DPD
osal_void hh503_dpd_cfr_set_11b(hal_to_dmac_device_stru *hal_device, osal_u8 is_11b)
{
    hh503_dpd_cfr_set_11b_ext(hal_device, is_11b);
}
#endif
osal_void hh503_cali_send_func(hal_to_dmac_device_stru *hal_device, osal_u8* cali_data, osal_u16 frame_len,
    osal_u16 remain)
{
    hh503_cali_send_func_ext(hal_device, cali_data, frame_len, remain);
}
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_void hh503_set_custom_rx_insert_loss(osal_u8 band, osal_u8 *insert_loss, osal_u8 len)
{
    fe_custom_set_rx_insert_loss(band, insert_loss, len);
}
osal_u8 hh503_get_custom_rx_insert_loss(osal_u8 band, osal_u8 ch_idx)
{
    osal_u8 rf_subband = hal_get_rf_subband_idx(band, ch_idx);
    return fe_custom_get_rx_insert_loss(band, rf_subband);
}
osal_void hh503_set_custom_power(osal_u8 band, osal_u8 *power, osal_u16 len)
{
    // 内存排布见 wlan_cust_rf_fe_power_params 根据实际发送的数据解析
    osal_u16 rsv_mem_len = len; // 还剩下未解析的长度
    osal_u16 set_mem_len = 0;   // 已解析的长度

    if (rsv_mem_len >= FE_CUSTOMIZE_MAX_CHIP_POWER_LEN) {
        fe_custom_set_chip_max_power(band, power + set_mem_len, FE_CUSTOMIZE_MAX_CHIP_POWER_LEN);
        set_mem_len += FE_CUSTOMIZE_MAX_CHIP_POWER_LEN;
        rsv_mem_len -= FE_CUSTOMIZE_MAX_CHIP_POWER_LEN;
    }
    if (rsv_mem_len >= FE_CUSTOMIZE_TARGET_POWER_LEN) {
        fe_custom_config_all_target_power(band, power + set_mem_len, FE_CUSTOMIZE_TARGET_POWER_LEN);
        set_mem_len += FE_CUSTOMIZE_TARGET_POWER_LEN;
        rsv_mem_len -= FE_CUSTOMIZE_TARGET_POWER_LEN;
    }
    if (rsv_mem_len >= FE_CUSTOMIZE_LIMIT_POWER_LEN) {
        fe_custom_set_limit_power(band, power + set_mem_len, FE_CUSTOMIZE_LIMIT_POWER_LEN);
        set_mem_len += FE_CUSTOMIZE_LIMIT_POWER_LEN;
        rsv_mem_len -= FE_CUSTOMIZE_LIMIT_POWER_LEN;
    }
    if (rsv_mem_len >= FE_CUSTOMIZE_SAR_POWER_LEN) {
        fe_custom_set_sar_power(band, power + set_mem_len, FE_CUSTOMIZE_SAR_POWER_LEN);
        set_mem_len += FE_CUSTOMIZE_SAR_POWER_LEN;
        rsv_mem_len -= FE_CUSTOMIZE_SAR_POWER_LEN;
    }
    if (rsv_mem_len >= FE_CUSTOMIZE_CTA_COEF_FLAG_LEN) {
        fe_custom_set_cta_coef_flag(band, power + set_mem_len, FE_CUSTOMIZE_CTA_COEF_FLAG_LEN);
        set_mem_len += FE_CUSTOMIZE_CTA_COEF_FLAG_LEN;
        rsv_mem_len -= FE_CUSTOMIZE_CTA_COEF_FLAG_LEN;
    }
}
#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
 功能描述  : 03 rf定制化
*****************************************************************************/
osal_u32 hh503_config_custom_rf(const osal_u8 *param)
{
    return hh503_config_custom_rf_ext(param);
}

/*****************************************************************************
 功能描述  : RSSI插损值
*****************************************************************************/
osal_void hh503_config_rssi_for_loss(osal_u8 channel_num, osal_s8 *rssi)
{
    hh503_config_rssi_for_loss_ext(channel_num, rssi);
}

/*****************************************************************************
 功能描述  : RF初始化
*****************************************************************************/
osal_void hh503_initialize_rf_sys(hal_to_dmac_device_stru *hal_device)
{
    hh503_initialize_rf_sys_ext(hal_device);
}

/*****************************************************************************
功能描述  : 调整tx power
*****************************************************************************/
osal_void hh503_adjust_tx_power(osal_u8 ch, osal_s8 power)
{
    hh503_adjust_tx_power_level_ext(ch, power);
}

/*****************************************************************************
函 数 名  : hal_get_tpc_code
功能描述  : 获取tpc_code的值
*****************************************************************************/
osal_u8 hh503_get_tpc_code(osal_void)
{
    return hh503_get_tpc_code_ext();
}

/*****************************************************************************
功能描述  : 恢复tx power
*****************************************************************************/
osal_void hh503_restore_tx_power(osal_u8 ch)
{
    unref_param(ch);
}

/*****************************************************************************
 函 数 名  : hh503_set_pow_rf_linectl_enable
 功能描述  : 设置发送功率是否rf寄存器控
*****************************************************************************/
osal_void hh503_pow_set_rf_regctl_enable(hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 rf_linectl)
{
    unref_param(hal_device);
    unref_param(rf_linectl);
}

static osal_void hh503_set_tpc_code(osal_u8 value);

osal_void hh503_cali_send_func_ext(hal_to_dmac_device_stru *hal_device, osal_u8* cali_data,
    osal_u16 frame_len, osal_u16 remain)
{
    unref_param(hal_device);
    unref_param(cali_data);
    unref_param(frame_len);
    unref_param(remain);
}

/*****************************************************************************
 函 数 名  : hh503_initialize_rf_sys
 功能描述  : RF初始化
*****************************************************************************/
osal_void hh503_initialize_rf_sys_ext(hal_to_dmac_device_stru *hal_device)
{
    unref_param(hal_device);
    /* 停止PA和PHY的工作 */
    hh503_disable_machw_phy_and_pa();
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
 函 数 名  : hh503_config_custom_rf
 功能描述  : 03 rf定制化
*****************************************************************************/
osal_u32 hh503_config_custom_rf_ext(const osal_u8 *param)
{
    osal_u32 offset = 0;
    osal_s32 ret = OAL_SUCC;
    if (param == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hh503_config_custom_rf::param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 注意copy的顺序，新增加的往后写即可 */
    ret += memcpy_s(&g_rf_customize.rf_gain_db_rf, sizeof(g_rf_customize.rf_gain_db_rf),
        param, sizeof(g_rf_customize.rf_gain_db_rf));
    offset += sizeof(g_rf_customize.rf_gain_db_rf);

    ret += memcpy_s(&g_rf_customize.delta_pwr_ref_cfg, sizeof(g_rf_customize.delta_pwr_ref_cfg),
        param + offset, sizeof(g_rf_customize.delta_pwr_ref_cfg));
    offset += sizeof(g_rf_customize.delta_pwr_ref_cfg);

    ret += memcpy_s(&g_rf_customize.far_dist_pow_gain_switch, sizeof(g_rf_customize.far_dist_pow_gain_switch),
        param + offset, sizeof(g_rf_customize.far_dist_pow_gain_switch));
    offset += sizeof(g_rf_customize.far_dist_pow_gain_switch);

    ret += memcpy_s(&g_rf_customize.far_dist_dsss_scale_promote_switch,
        sizeof(g_rf_customize.far_dist_dsss_scale_promote_switch), param + offset,
        sizeof(g_rf_customize.far_dist_dsss_scale_promote_switch));
    offset += sizeof(g_rf_customize.far_dist_dsss_scale_promote_switch);

    ret += memcpy_s(&g_rf_customize.chn_radio_cap, sizeof(osal_u8), param + offset, sizeof(osal_u8));
    offset += sizeof(g_rf_customize.chn_radio_cap);

    ret += memcpy_s(&g_rf_customize.junction_temp_diff, sizeof(osal_u8), param + offset, sizeof(osal_u8));
    offset += sizeof(g_rf_customize.junction_temp_diff);

    ret += memcpy_s(&g_rf_customize.cfg_cus_cca, sizeof(g_rf_customize.cfg_cus_cca),
        param + offset, sizeof(g_rf_customize.cfg_cus_cca));
    offset += sizeof(g_rf_customize.cfg_cus_cca);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hh503_config_custom_rf_ext:memcpy fail.}");
    }
    /* refresh cca ed threshold after phy init */
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
    hh503_set_ed_high_th(HAL_CCA_OPT_ED_HIGH_20TH_DEF + g_rf_customize.cfg_cus_cca.delta_cca_ed_high_20th_2g,
        HAL_CCA_OPT_ED_HIGH_40TH_DEF + g_rf_customize.cfg_cus_cca.delta_cca_ed_high_40th_2g, OSAL_TRUE);
#endif
    oam_warning_log4(0, OAM_SF_CFG, "hh503_config_custom_rf::delt_20_2g=%d,delt_40_2g=%d,delt_20_5g=%d,delt_40_5g=%d.",
        g_rf_customize.cfg_cus_cca.delta_cca_ed_high_20th_2g, g_rf_customize.cfg_cus_cca.delta_cca_ed_high_40th_2g,
        g_rf_customize.cfg_cus_cca.delta_cca_ed_high_20th_5g, g_rf_customize.cfg_cus_cca.delta_cca_ed_high_40th_5g);
    oam_warning_log4(0, OAM_SF_CFG,
        "hh503_config_custom_rf::far_dist_pow_gain[%d],far_dist_dsss_scale_promote[%d],chn_radio_cap[%d],temp[%d]\r\n",
        g_rf_customize.far_dist_pow_gain_switch, g_rf_customize.far_dist_dsss_scale_promote_switch,
        g_rf_customize.chn_radio_cap, g_rf_customize.junction_temp_diff);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 根据信道号获取RSSI插损值的索引值
*****************************************************************************/
OSAL_STATIC osal_u32 hh503_rf_get_subband_idx_for_loss(osal_u8 channel_num)
{
    osal_u32 index;
    for (index = 0; index < oal_array_size(g_band_num_for_loss); index++) {
        if (channel_num >= g_band_num_for_loss[index].channel_num_min &&
            channel_num <= g_band_num_for_loss[index].channel_num_max) {
            return g_band_num_for_loss[index].index;
        }
    }

    return OAL_FAIL;
}
/*****************************************************************************
 功能描述  : 设置RSSI插损值
*****************************************************************************/
osal_void hh503_config_rssi_for_loss_ext(osal_u8 channel_num, osal_s8 *rssi)
{
    osal_u32 band_idx;
    osal_u8 band;
    osal_char rssi_for_loss;

    if (rssi == NULL) {
        oam_error_log0(0, 0, "{hh503_config_custom_rssi_for_loss::rssi == NULL}");
        return;
    }

    band = ((channel_num > HH503_RF_FREQ_2_CHANNEL_NUM) ? WLAN_BAND_5G : WLAN_BAND_2G);
    band_idx = hh503_rf_get_subband_idx_for_loss(channel_num);
    if (band_idx == OAL_FAIL) {
        return;
    }
    if (band == WLAN_BAND_2G) {
        rssi_for_loss = g_rf_customize.rf_gain_db_rf[0].gain_db_2g[band_idx].rf_gain_db_mult4;
    } else {
        rssi_for_loss = g_rf_customize.rf_gain_db_rf[0].gain_db_5g[band_idx].rf_gain_db_mult4;
    }
    if (rssi_for_loss < 0 || rssi_for_loss > 100) { /* 补偿值范围0-100 */
        return;
    }
    if (((osal_s8)(*rssi + (rssi_for_loss / 10)) > OAL_RSSI_SIGNAL_MIN) && /* 10:去掉小数 */
        ((osal_s8)(*rssi + (rssi_for_loss / 10)) < OAL_RSSI_SIGNAL_MAX)) { /* 10:去掉小数 */
        /* 补偿后的有效范围是 5 ~ -103      */
        *rssi += (rssi_for_loss / 10); /* 10:去掉小数 */
    }
    return;
}

#endif // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

#if defined (BOARD_FPGA_WIFI) || defined (BOARD_ASIC_WIFI)
osal_void hh503_adjust_tx_power_level_ext(osal_u8 ch, osal_s8 power)
{
    osal_u32 index;
    oam_warning_log2(0, OAM_SF_ANY, "{hh503_adjust_tx_power_level_ext::ch=%d power=%d}", ch, power);

    for (index = 0; index < osal_array_size(g_adjust_tx_power_level_table); index++) {
        if (power <= g_adjust_tx_power_level_table[index].power) {
            break;
        }
    }
    if (index >= osal_array_size(g_adjust_tx_power_level_table)) {
        index = osal_array_size(g_adjust_tx_power_level_table) - 1;
    }

    /* 根据命令中的功率去查询tpc_code并赋值 */
    hh503_set_tpc_code(g_adjust_tx_power_level_table[index].tpc_code);
}

osal_u8 hh503_get_tpc_code_ext(osal_void)
{
    return g_tpc_code;
}

static osal_void hh503_set_tpc_code(osal_u8 value)
{
    g_tpc_code = value;
}
#endif

/*                                      delta cca_20M_2g   delta cca_40M_2g   delta cca_20M_5g   delta cca_40M_5g */
hal_cfg_custom_cca_stru g_cfg_cus_cca = {0,                0,                 0,                 0};

// 获取定制化数据中的cca门限默认值
osal_char fe_rf_get_customized_cca_ed(osal_u32 cca_ed_type, wlan_channel_band_enum_uint8 band)
{
    hal_cfg_custom_cca_stru *cca = &(g_cfg_cus_cca);
    if (cca_ed_type == CCA_ED_TYPE_20TH) {
        return (band == WLAN_BAND_2G) ? (cca->delta_cca_ed_high_20th_2g + HAL_CCA_OPT_ED_HIGH_20TH_DEF) :
        (cca->delta_cca_ed_high_20th_5g + HAL_CCA_OPT_ED_HIGH_20TH_DEF);
    }
    return (band == WLAN_BAND_2G) ? (cca->delta_cca_ed_high_40th_2g + HAL_CCA_OPT_ED_HIGH_40TH_DEF) :
        (cca->delta_cca_ed_high_40th_5g + HAL_CCA_OPT_ED_HIGH_40TH_DEF);
}
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
osal_void hal_rf_get_rx_ppdu_info(osal_u8 flag)
{
#ifdef _PRE_WIFI_PRINTK
    osal_u32 mac_val_ok = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x48);
    osal_u32 mac_val_err = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x4C);
    osal_u32 mac_ax_val_ok = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x13C);
    osal_u32 mac_ax_val_err = hal_reg_read(HH503_MAC_RD1_BANK_BASE_0x140);
    osal_u32 phy_dotb_ok = hal_reg_read(HH503_PHY_BANK1_BASE_0x1DC);
    osal_u32 phy_dotb_err = hal_reg_read(HH503_PHY_BANK1_BASE_0x1EC);
    osal_u32 phy_ht_ok = hal_reg_read(HH503_PHY_BANK1_BASE_0x1E0);
    osal_u32 phy_11b_err = hal_reg_read(HH503_PHY_BANK1_BASE_0x1F0);
    osal_u32 phy_vht_ok = hal_reg_read(HH503_PHY_BANK1_BASE_0x1E4);
    osal_u32 phy_vht_err = hal_reg_read(HH503_PHY_BANK1_BASE_0x1F4);
    osal_u32 phy_lega_ok = hal_reg_read(HH503_PHY_BANK1_BASE_0x1E8);
    osal_u32 phy_lega_err = hal_reg_read(HH503_PHY_BANK1_BASE_0x1F8);

    wifi_printf_always("mac mpdu[%u,%u] ampdu[%u,%u]\r\n", mac_val_ok, mac_val_err, mac_ax_val_ok, mac_ax_val_err);
    wifi_printf_always("phy dotb[%u,%u] ht[%u,%u] vht[%u,%u] lega[%u,%u]\r\n",
        phy_dotb_ok, phy_dotb_err, phy_ht_ok, phy_11b_err, phy_vht_ok, phy_vht_err, phy_lega_ok, phy_lega_err);
#endif
    if (flag == OAL_TRUE) {
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x13C, 0xffffffff);
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x13C, 0x0);
        hal_reg_write(HH503_PHY_BANK1_BASE_0x1B8, 0xffffffff);
        hal_reg_write(HH503_PHY_BANK1_BASE_0x1B8, 0x0);
    }
}
#endif
osal_void hal_set_cal_tone(osal_u32 tone_cfg)
{
    u_cal_tone_gen1 tone = {0};
    osal_u32 dac_fs = (160 << (hal_reg_read(HH503_PHY_BANK1_BASE_0x14) & 0x3));    // 最小160Mhz
    osal_u32 fc = (tone_cfg & 0xffffff);
    tone.bits.cfg_tone_en = ((tone_cfg >> BIT_OFFSET_24) & 0x1);
    tone.bits.x6NgwLihWwGSu_ = 0x1;  // -18dbfs
    if (fc > 0x7fffff) {
        tone.bits.x6NgwLihWwCBgh_ = 1;    // 负单音配置1
        fc = 0x1000000 - fc;    // 转换为正数频率
    }
    tone.bits.x6NgwLihWwSiFWwdCW__ = fc * 512 / dac_fs / 1000;    // fc * 512 / ffs /1000 单位转换KHz->MHz
    hal_reg_write(HH503_PHY_BANK3_BASE_0x10, tone.u32);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
