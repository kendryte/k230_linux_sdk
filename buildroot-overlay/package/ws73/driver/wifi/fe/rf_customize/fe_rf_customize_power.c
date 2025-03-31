/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize rx insert loss 接口
 * Create: 2022-12-15
 */
#include "fe_rf_customize_power.h"
#include "wlan_spec.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FE_RF_CUSTOMIZE_POWER
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* rf 相关定制化结构体 */
static fe_custom_power_stru g_rf_fe_power = {
    { 200,
        { 40, 40, 40, 40,
            40, 40, 40, 40, 40, 40, 40, 40,
            40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
            40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40},
        { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
            60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
            60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
            60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60},
        { 60, 60, 60},
        0,
        {0}
    }
};

osal_void fe_custom_set_chip_max_power(osal_u8 band, osal_u8 *power, osal_u16 len)
{
    unref_param(len);
    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_set_chip_max_power::band:%d", band);
        return;
    }

    g_rf_fe_power.power_2g.chip_max_power = *power;
    return;
}
osal_u8 fe_custom_get_chip_max_power(osal_u8 band)
{
    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_get_chip_max_power::band:%d", band);
        return FE_CUSTOMIZE_MAX_CHIP_POWER;
    }
    return g_rf_fe_power.power_2g.chip_max_power;
}
osal_void fe_custom_set_target_power(osal_u8 band, osal_u8 rate, osal_u8 power)
{
    if (band != WLAN_BAND_2G || rate >= FE_CUSTOMIZE_TARGET_POWER_LEN) {
        oam_error_log2(0, OAM_SF_CUSTOM, "fe_custom_get_target_power::band:%d, rate_idx:%d", band, rate);
        return;
    }
    if (power > FE_CUSTOMIZE_MAX_CHIP_POWER) {
        power = FE_CUSTOMIZE_MAX_CHIP_POWER;
    }
    g_rf_fe_power.power_2g.target_power[rate] = power;
}
osal_void fe_custom_config_all_target_power(osal_u8 band, osal_u8 *power, osal_u16 len)
{
    osal_u8 *target_power = g_rf_fe_power.power_2g.target_power;
    osal_u16 max_len = sizeof(g_rf_fe_power.power_2g.target_power);
    osal_s32 ret;
    osal_s32 idx;

    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_config_all_target_power::band:%d", band);
        return;
    }

    ret = memcpy_s(target_power, max_len, power, len);
    for (idx = 0; idx < max_len; idx++) {
        oam_info_log2(0, OAM_SF_CUSTOM, "fe_custom_config_all_target_power::idx:%d, power:%d",
            idx, g_rf_fe_power.power_2g.target_power[idx]);
    }
    if (ret != OAL_SUCC) {
        return;
    }
    return;
}
osal_u8 fe_custom_get_target_power(osal_u8 band, osal_u8 rate_idx)
{
    if (band != WLAN_BAND_2G || rate_idx >= FE_CUSTOMIZE_TARGET_POWER_LEN) {
        oam_error_log2(0, OAM_SF_CUSTOM, "fe_custom_get_target_power::band:%d, rate_idx:%d", band, rate_idx);
        return FE_CUSTOMIZE_MAX_CHIP_POWER;
    }
    return g_rf_fe_power.power_2g.target_power[rate_idx];
}
osal_void fe_custom_set_limit_power(osal_u8 band, osal_u8 *power, osal_u16 len)
{
    osal_u8 *limit_power = g_rf_fe_power.power_2g.limit_power;
    osal_u16 max_len = sizeof(g_rf_fe_power.power_2g.limit_power);
    osal_s32 ret;
    osal_s32 idx;

    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_set_limit_power::band:%d", band);
        return;
    }

    ret = memcpy_s(limit_power, max_len, power, len);
    for (idx = 0; idx < max_len; idx++) {
        oam_info_log2(0, OAM_SF_CUSTOM, "fe_custom_set_limit_power::idx:%d, power:%d",
            idx, g_rf_fe_power.power_2g.limit_power[idx]);
    }
    if (ret != OAL_SUCC) {
        return;
    }
    return;
}
osal_u8 fe_custom_get_limit_power(osal_u8 band, osal_u8 rate_idx, osal_u8 ch_idx)
{
    osal_u16 pow_idx = ch_idx * FE_CUSTOMIZE_LIMIT_POWER_MODE;

    if (rate_idx >= WLAN_POW_11B_RATE_NUM) {
        pow_idx++;
    }
    if (rate_idx >= WLAN_POW_11B_RATE_NUM + WLAN_POW_LEGACY_RATE_NUM) {
        pow_idx++;
    }
    if (rate_idx >= WLAN_POW_11B_RATE_NUM + WLAN_POW_LEGACY_RATE_NUM + WLAN_POW_NONLEGACY_20M_NUM) {
        pow_idx++;
    }

    if (band != WLAN_BAND_2G || pow_idx >= FE_CUSTOMIZE_LIMIT_POWER_LEN) {
        oam_error_log4(0, OAM_SF_CUSTOM, "fe_custom_get_limit_power::band:%d, pow_idx:%d rate_idx:%d ch_idx:%d\n",
            band, pow_idx, rate_idx, ch_idx);
        return FE_CUSTOMIZE_LIMIT_MAX_POWER;
    }
    return g_rf_fe_power.power_2g.limit_power[pow_idx];
}
osal_void fe_custom_set_sar_power(osal_u8 band, osal_u8 *power, osal_u16 len)
{
    osal_u8 *sar_power = g_rf_fe_power.power_2g.sar_power;
    osal_u16 max_len = sizeof(g_rf_fe_power.power_2g.sar_power);
    osal_s32 ret;
    osal_s32 idx;

    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_set_sar_power::band:%d", band);
        return;
    }

    ret = memcpy_s(sar_power, max_len, power, len);
    for (idx = 0; idx < max_len; idx++) {
        oam_info_log2(0, OAM_SF_CUSTOM, "fe_custom_set_sar_power::idx:%d, power:%d",
            idx, g_rf_fe_power.power_2g.sar_power[idx]);
    }
    if (ret != OAL_SUCC) {
        return;
    }
    return;
}
osal_u8 fe_custom_get_sar_power(osal_u8 band, osal_u8 level)
{
    if (band != WLAN_BAND_2G || level >= FE_CUSTOMIZE_SAR_POWER_LEN) {
        oam_error_log2(0, OAM_SF_CUSTOM, "fe_custom_set_sar_power::band:%d level:%d", band, level);
        return FE_CUSTOMIZE_SAR_MAX_POWER;
    }
    return g_rf_fe_power.power_2g.sar_power[level];
}
osal_void fe_custom_set_cta_coef_flag(osal_u8 band, osal_u8 *value, osal_u16 len)
{
    unref_param(len);
    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_set_cta_coef_flag::band:%d", band);
        return;
    }
    g_rf_fe_power.power_2g.special_cta_coef_flag = *value;
    return;
}
osal_u8 fe_custom_get_cta_coef_flag(osal_u8 band)
{
    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_get_cta_coef_flag::band:%d", band);
        return 0;
    }
    return g_rf_fe_power.power_2g.special_cta_coef_flag;
}
#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif