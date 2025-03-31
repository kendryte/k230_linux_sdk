/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize power cali 接口
 */
#include "fe_rf_customize_power_cali.h"
#ifdef _PRE_PRODUCT_ID_HOST
#include "hal_common_ops.h"
#else
#include "hal_common_ops_rom.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FE_RF_CUSTOMIZE_POWER_CALI

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#ifdef BOARD_FPGA_WIFI
/* rf 相关定制化结构体 */
static fe_custom_power_cali_stru g_rf_fe_power_cali = {
    {
        /* 期望功率 */
        {
            {150, 150, 150}, /* 高功率sub band 0~2 */
            {120, 120, 120}, /* 低功率 */
        },

        /* 曲线参数 */
        {
            {
                {0, 341, 0}, /* 高功率11b */
                {0, 341, 0}, /* ofdm20m */
                {0, 341, 0}, /* ofdm40m */
            },
            {
                {0, 341, 0}, /* 低功率11b */
                {0, 341, 0}, /* ofdm20m */
                {0, 341, 0}, /* ofdm40m */
            },
        },
        /* 曲线放大系数 */
        {
            { 16, 10, 0 }, /* 高功率 */
            { 16, 10, 0 }, /* 低功率 */
        }
    }
};
#else
/* rf 相关定制化结构体 */
static fe_custom_power_cali_stru g_rf_fe_power_cali = {
    {
        /* 期望功率 */
        {
            {215, 215, 215}, /* 高功率sub band 0~2 */
            {150, 150, 150}, /* 低功率 */
        },
        /* 曲线参数 */
        {
            {
                {-470, 158, -387}, /* 高功率11b */
                {-470, 158, -387}, /* ofdm20m */
                {-470, 158, -387}, /* ofdm40m */
            },
            {
                {-470, 158, -387}, /* 低功率11b */
                {-470, 158, -387}, /* ofdm20m */
                {-470, 158, -387}, /* ofdm40m */
            },
        },
        /* 曲线放大系数 */
        {
            { 26, 10, 0 }, /* 高功率 */
            { 26, 10, 0 }, /* 低功率 */
        }
    }
};
#endif
osal_void fe_custom_config_power_cali_ref_power(osal_s16 *ref_power, osal_u16 date_len)
{
    if (memcpy_s(g_rf_fe_power_cali.power_cali_2g.ref_power, sizeof(g_rf_fe_power_cali.power_cali_2g.ref_power),
        ref_power, date_len) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "{fe_custom_config_power_cali_ref_power:memcpy fail.}");
        return;
    }
}
osal_void fe_custom_config_power_cali_power_curve_high(osal_s16 *power_curve, osal_u16 date_len)
{
    if (memcpy_s(g_rf_fe_power_cali.power_cali_2g.power_curve[FE_CUS_HIGH_POW],
        sizeof(g_rf_fe_power_cali.power_cali_2g.power_curve[FE_CUS_HIGH_POW]), power_curve, date_len) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "{fe_custom_config_power_cali_power_curve_high:memcpy fail.}");
        return;
    }
}
osal_void fe_custom_config_power_cali_power_curve_low(osal_s16 *power_curve, osal_u16 date_len)
{
    if (memcpy_s(g_rf_fe_power_cali.power_cali_2g.power_curve[FE_CUS_LOW_POW],
        sizeof(g_rf_fe_power_cali.power_cali_2g.power_curve[FE_CUS_LOW_POW]), power_curve, date_len) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "{fe_custom_config_power_cali_power_curve_low:memcpy fail.}");
        return;
    }
}
osal_void fe_custom_config_power_cali_curve_factor(osal_s16 *curve_factor, osal_u16 date_len)
{
    if (memcpy_s(g_rf_fe_power_cali.power_cali_2g.curve_factor, sizeof(g_rf_fe_power_cali.power_cali_2g.curve_factor),
        curve_factor, date_len) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "{fe_custom_config_power_cali_curve_factor:memcpy fail.}");
        return;
    }
}
/*****************************************************************************
 函 数 名  : fe_custom_config_power_cali
 功能描述  : 配置power cali
*****************************************************************************/
osal_void fe_custom_config_power_cali(osal_u8 band, osal_u8 *param)
{
    fe_custom_power_cali_stru *data = (fe_custom_power_cali_stru *)param;

    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_config_power_cali::band:%d", band);
        return;
    }
    fe_custom_config_power_cali_ref_power((osal_s16 *)data->power_cali_2g.ref_power,
        sizeof(data->power_cali_2g.ref_power));
    fe_custom_config_power_cali_power_curve_high((osal_s16 *)data->power_cali_2g.power_curve[FE_CUS_HIGH_POW],
        sizeof(data->power_cali_2g.power_curve[FE_CUS_HIGH_POW]));
    fe_custom_config_power_cali_power_curve_low((osal_s16 *)data->power_cali_2g.power_curve[FE_CUS_LOW_POW],
        sizeof(data->power_cali_2g.power_curve[FE_CUS_LOW_POW]));
    fe_custom_config_power_cali_curve_factor((osal_s16 *)data->power_cali_2g.curve_factor,
        sizeof(data->power_cali_2g.curve_factor));
}
/*****************************************************************************
 函 数 名  : fe_custom_get_ref_power
 功能描述  : 获取期望功率
*****************************************************************************/
osal_s16 fe_custom_get_ref_power(osal_u8 band, osal_u8 level_idx, osal_u8 subband_idx)
{
    if (band != WLAN_BAND_2G || level_idx >= FE_CUS_POW_BUTT || \
        subband_idx >= FE_RF_CUS_CALI_SUBBAND_NUM) {
        oam_error_log3(0, OAM_SF_CUSTOM,
            "fe_rf_custom_get_txpwr_pow_ref::band:%d, level:%d, subband:%d",
            band, level_idx, subband_idx);
        return 0;
    }
    return (osal_s16)(g_rf_fe_power_cali.power_cali_2g.ref_power[level_idx][subband_idx]);
}

/*****************************************************************************
 函 数 名  : fe_custom_get_power_cali
 功能描述  : 获取power cali及其大小
*****************************************************************************/
fe_custom_power_cali_stru *fe_custom_get_power_cali(osal_u8 band, osal_u16 *len)
{
    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_get_txpwr_addr::band:%d", band);
        return NULL;
    }
    *len = sizeof(g_rf_fe_power_cali);
    return &(g_rf_fe_power_cali);
}
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
/*****************************************************************************
 函 数 名  : fe_custom_get_curve_protocol_idx
 功能描述  : 获取协议类型
*****************************************************************************/
osal_u8 fe_custom_get_curve_protocol_idx(osal_u8 band, osal_u8 bandwidth, osal_u8 protocol)
{
    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_get_curve_protocol_idx::band:%d", band);
        return 0;
    }

    if (protocol == WLAN_11B_PHY_PROTOCOL_MODE) {
        return FE_CUS_CURVE_11B;
    } else {
        return (bandwidth == WLAN_BANDWIDTH_20) ? FE_CUS_CURVE_OFDM20M : FE_CUS_CURVE_OFDM40M;
    }
}
/*****************************************************************************
 函 数 名  : fe_custom_set_all_power_curve
 功能描述  : 曲线参数写入定制化
*****************************************************************************/
osal_s32 fe_custom_set_all_power_curve(osal_u8 band, osal_s32 *param, osal_u16 len, osal_u8 level_idx)
{
    osal_u8 count = 0;
    osal_u8 protocal_idx;
    osal_u8 poly_idx;

    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_set_all_power_curve::band:%d.\n", band);
        return OAL_FAIL;
    }
    /* param一个数占32位, power_curve一个数占16位 */
    for (protocal_idx = 0; protocal_idx < FE_CUS_CURVE_PROTOCOL_BUTT; protocal_idx++) {
        for (poly_idx = 0; poly_idx < FE_RF_CUS_POLYNOMIAL_NUM; poly_idx++) {
            if (count >= len) {
                oam_warning_log3(0, OAM_SF_CUSTOM,
                    "{fe_custom_set_all_power_curve:copy fail:level[%d],count[%d] >= len[%d].\n}",
                    level_idx, count, len);
                return OAL_FAIL;
            }
            g_rf_fe_power_cali.power_cali_2g.power_curve[level_idx][protocal_idx][poly_idx] =
                (osal_s16)param[count];
            count++;
        }
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_custom_get_all_power_curve
 功能描述  : 曲线参数从定制化读取
*****************************************************************************/
osal_s32 fe_custom_get_all_power_curve(osal_u8 band, osal_s32 *param, osal_u16 len,
    osal_u32 *out_param_len, osal_u8 level_idx)
{
    osal_u8 count = 0;
    osal_u8 protocal_idx;
    osal_u8 poly_idx;

    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_get_all_power_curve::band:%d.\n", band);
        return OAL_FAIL;
    }
    /* param一个数占32位, power_curve一个数占16位 */
    for (protocal_idx = 0; protocal_idx < FE_CUS_CURVE_PROTOCOL_BUTT; protocal_idx++) {
        for (poly_idx = 0; poly_idx < FE_RF_CUS_POLYNOMIAL_NUM; poly_idx++) {
            if (count >= len) {
                oam_warning_log3(0, OAM_SF_CUSTOM,
                    "{fe_custom_get_all_power_curve:copy fail:level[%d],count[%d] >= len[%d].\n}",
                    level_idx, count, len);
                return OAL_FAIL;
            }
            param[count] =
                g_rf_fe_power_cali.power_cali_2g.power_curve[level_idx][protocal_idx][poly_idx];
            count++;
        }
    }
    *out_param_len = count;
    return OAL_SUCC;
}
#endif
/*****************************************************************************
 函 数 名  : fe_custom_get_power_curve
 功能描述  : 获取曲线参数
*****************************************************************************/
fe_custom_power_curve_stru fe_custom_get_power_curve(osal_u8 band, osal_u8 level_idx,
    osal_u8 protocol_idx)
{
    fe_custom_power_curve_stru curve = {0};
    if (band != WLAN_BAND_2G || level_idx >= FE_CUS_POW_BUTT || \
        protocol_idx >= FE_CUS_CURVE_PROTOCOL_BUTT) {
        oam_error_log3(0, OAM_SF_CUSTOM, "fe_custom_get_power_curve::band:%d, level:%d, protocal:%d",
            band, level_idx, protocol_idx);
        return curve;
    }
    curve.pow_par2 = g_rf_fe_power_cali.power_cali_2g.power_curve[level_idx][protocol_idx][0];
    curve.pow_par1 = g_rf_fe_power_cali.power_cali_2g.power_curve[level_idx][protocol_idx][1];
    curve.pow_par0 =
        g_rf_fe_power_cali.power_cali_2g.power_curve[level_idx][protocol_idx][FE_RF_CUS_POLYNOMIAL_NUM - 1];
    return curve;
}
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
/*****************************************************************************
 函 数 名  : fe_custom_set_all_curve_factor
 功能描述  : 曲线放大系数写入定制化
*****************************************************************************/
osal_s32 fe_custom_set_curve_factor(osal_u8 band, osal_s32 *param, osal_u16 len, osal_u8 level_idx)
{
    osal_u8 count = 0;
    unref_param(len);

    if (band != WLAN_BAND_2G) {
        oam_error_log1(0, OAM_SF_CUSTOM, "fe_custom_set_all_power_curve::band:%d.\n", band);
        return OAL_FAIL;
    }
    /* param一个数占32位, power_curve一个数占16位 */
    for (count = 0; count < FE_RF_CUS_POLYNOMIAL_NUM; count++) {
        g_rf_fe_power_cali.power_cali_2g.curve_factor[level_idx][count] = (osal_s16)param[count];
    }
    return OAL_SUCC;
}
#endif
/*****************************************************************************
 函 数 名  : fe_custom_get_curve_factor
 功能描述  : 获取曲线放大系数
*****************************************************************************/
fe_custom_power_curve_stru fe_custom_get_curve_factor(osal_u8 band, osal_u8 level_idx)
{
    fe_custom_power_curve_stru factor = {0};
    if (band != WLAN_BAND_2G || level_idx >= FE_CUS_POW_BUTT) {
        oam_error_log2(0, OAM_SF_CUSTOM, "fe_custom_get_curve_factor::band:%d, level:%d",
            band, level_idx);
        return factor;
    }
    factor.pow_par2 = g_rf_fe_power_cali.power_cali_2g.curve_factor[level_idx][0];
    factor.pow_par1 = g_rf_fe_power_cali.power_cali_2g.curve_factor[level_idx][1];
    factor.pow_par0 =
        g_rf_fe_power_cali.power_cali_2g.curve_factor[level_idx][FE_RF_CUS_POLYNOMIAL_NUM - 1];
    return factor;
}
/*****************************************************************************
 函 数 名  : fe_custom_calc_curve_pow
 功能描述  : 检查曲线的单调递增性
*****************************************************************************/
osal_u32 fe_custom_check_curve(fe_custom_power_curve_stru *power_curve,
    fe_custom_power_curve_stru *factor, osal_s16 p_val)
{
    osal_s32 extre_point_val = 0x0; /* 二次曲线的极值 */
    if (power_curve->pow_par2 != 0) {
        extre_point_val = -(power_curve->pow_par1) * \
            ((osal_s32)(BIT0 << (factor->pow_par2 - factor->pow_par1))) / \
            (osal_s32)(BIT1 * power_curve->pow_par2);
    }
    if (((power_curve->pow_par2 > 0) && (p_val < extre_point_val)) || \
        ((power_curve->pow_par2 < 0) && (p_val > extre_point_val)) || \
        ((power_curve->pow_par2 == 0) && (power_curve->pow_par1 <= 0))) {
        oam_warning_log4(0, OAM_SF_CALIBRATE, "{fe_custom_check_curve::extre_point[%d], curve[%d %d %d]}",
            extre_point_val, power_curve->pow_par2, power_curve->pow_par1, power_curve->pow_par0);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_custom_calc_curve_pow
 功能描述  : 电压经过二次曲线,计算得到功率
*****************************************************************************/
osal_s32 fe_custom_calc_curve_pow(fe_custom_power_curve_stru *power_curve,
    fe_custom_power_curve_stru *factor, osal_s16 p_val, osal_s16 *power)
{
    osal_s64 real_pdet_1 = ((osal_s64)power_curve->pow_par2 * p_val * p_val) >> factor->pow_par2;
    osal_s64 real_pdet_2 = ((osal_s64)power_curve->pow_par1 * p_val) >> factor->pow_par1;
    osal_s64 real_pdet_3 = (osal_u64)power_curve->pow_par0 >> factor->pow_par0;
    osal_s64 real_pdet = real_pdet_1 + real_pdet_2 + real_pdet_3;
#ifndef _PRE_PRODUCT_ID_HOST
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    osal_printf("%8d %8d %8d %8d\r\n", power_curve->pow_par2, power_curve->pow_par1,
        power_curve->pow_par0, real_pdet);
#endif
#endif
    oam_info_log4(0, OAM_SF_CALIBRATE, "{fe_custom_calc_curve_pow::curve [%d,%d,%d]real power:%lld}\n",
        power_curve->pow_par2, power_curve->pow_par1, power_curve->pow_par0, real_pdet);
    oam_info_log4(0, OAM_SF_CALIBRATE, "{fe_custom_calc_curve_pow::factor [%d,%d,%d]p_val:%d}\n",
        factor->pow_par2, factor->pow_par1, factor->pow_par0, p_val);

    /* 判断是否超出int16，超出则丢弃这次结果不做校准 */
    if (get_abs(real_pdet) > 0x7FFF) {
        oam_info_log1(0, OAM_SF_CALIBRATE,
            "{fe_custom_calc_curve_pow::real power[%d] out of range}\n", real_pdet);
        return OAL_FAIL;
    } else {
        *power = (osal_s16)real_pdet;
    }
    return OAL_SUCC;
}
#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif