/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Interface implementation of fe equipment.
 */
#include "fe_equipment.h"
#include "hal_common_ops.h"
#include "wlan_msg.h"
#include "msg_apf_rom.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FE_EQUIPMENT

#ifdef _PRE_WLAN_FEATURE_MFG_TEST

#define FE_MFG_GET_SUBBAND0_RSSI_COMP_BIT   0x4
#define FE_MFG_CALI_SUB_BAND_IDX0           0
#define FE_MFG_CALI_SUB_BAND_IDX1           1
#define FE_MFG_CALI_SUB_BAND_IDX2           2
#define FE_MFG_CHANNEL_IDX4_IDX             3
#define FE_MFG_CHANNEL_IDX9_IDX             8

fe_mfg_power_cali_para_stru g_mfg_power_cali_para = { 0 };

fe_mfg_power_cali_para_stru *fe_mfg_get_cali_para(osal_void)
{
    return &g_mfg_power_cali_para;
}
/*****************************************************************************
 函 数 名  : fe_mfg_set_power_curve
 功能描述  : 产线校准设置单组曲线参数
*****************************************************************************/
OSAL_STATIC osal_s32 fe_mfg_set_power_curve(osal_u8 *curve, osal_u8 power_level, osal_u8 len)
{
    osal_s16 *power_curve = g_mfg_power_cali_para.ipa_para.power_curve[power_level];
    osal_u16 max_len = sizeof(g_mfg_power_cali_para.ipa_para.power_curve[power_level]);

    if (memcpy_s(power_curve, max_len, curve, len) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_set_power_curve:: memcpy fail!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_config_all_curve_factor
 功能描述  : 产线校准设置所有曲线放大系数
*****************************************************************************/
OSAL_STATIC osal_s32 fe_mfg_config_all_curve_factor(osal_u8 *factor, osal_u16 len)
{
    osal_s16 *curve_factor = g_mfg_power_cali_para.ipa_para.curve_factor[0];
    osal_u16 max_len = sizeof(g_mfg_power_cali_para.ipa_para.curve_factor);

    if (memcpy_s(curve_factor, max_len, factor, len) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_config_all_curve_factor:: memcpy fail!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_init_power_cali_para
 功能描述  : 初始化产测数据
*****************************************************************************/
osal_void fe_mfg_init_power_cali_para(osal_u8 band, osal_u8 curve_idx, osal_u8 chain)
{
    osal_u8 power_level = 0;
    osal_u16 len = 0;
    fe_custom_power_cali_stru *power_cali = fe_custom_get_power_cali(band, &len);
    osal_u8 *power_curve = NULL;
    osal_u16 power_curve_len;
    osal_u8 *curve_factor = (osal_u8 *)(power_cali->power_cali_2g.curve_factor);
    osal_u16 curve_facotr_len = sizeof(power_cali->power_cali_2g.curve_factor);

    /* 从定制化读取曲线及放大系数 */
    for (power_level = 0; power_level < FE_CUS_POW_BUTT; power_level++) {
        power_curve = (osal_u8 *)(power_cali->power_cali_2g.power_curve[power_level][curve_idx]);
        power_curve_len = sizeof(power_cali->power_cali_2g.power_curve[power_level][curve_idx]);
        fe_mfg_set_power_curve(power_curve, power_level, power_curve_len);
    }
    fe_mfg_config_all_curve_factor(curve_factor, curve_facotr_len);
    /* 初始化电压和功率 */
    memset_s(g_mfg_power_cali_para.ipa_para.inst_power, sizeof(g_mfg_power_cali_para.ipa_para.inst_power),
        0, sizeof(g_mfg_power_cali_para.ipa_para.inst_power));
    memset_s(g_mfg_power_cali_para.ipa_para.chip_pdet, sizeof(g_mfg_power_cali_para.ipa_para.chip_pdet),
        0, sizeof(g_mfg_power_cali_para.ipa_para.chip_pdet));

    g_mfg_power_cali_para.curve_idx = curve_idx;
    g_mfg_power_cali_para.chain = chain;
    g_mfg_power_cali_para.save_flag = 0;
    g_mfg_power_cali_para.initialized_flag = 1; /* 首次保存数据前,总是执行初始化 */
}
/*****************************************************************************
 函 数 名  : fe_mfg_tx_pow_save_equip_param
 功能描述  : 保存产线功率校准曲线计算需要的芯片反馈电压值和期望值
*****************************************************************************/
osal_void fe_mfg_tx_pow_save_equip_param(online_cali_para_stru *online_cali,
    osal_s16 pdet_cal, osal_s16 exp_pow)
{
    /* 只有产线测试才需要保存参数 */
    fe_mfg_power_cali_para_stru *mfg_power_cali_para = fe_mfg_get_cali_para();
    fe_mfg_power_ipa_para_stru *mfg_power_cali_ipa_para = &(mfg_power_cali_para->ipa_para);
    osal_u8 max_param_num = FE_MFG_POW_PARAM_NUM;
    osal_u8 chain_idx = online_cali->chain_idx;
    osal_u8 curve_idx;
    osal_u8 previous_save_flag = 0;
    osal_u8 i;

    if (online_cali->channel_info.band != WLAN_BAND_2G) {
        /* 2G频段的射频才需要iPa曲线计算 */
        return;
    }

    curve_idx = fe_custom_get_curve_protocol_idx(online_cali->channel_info.band, online_cali->band_width,
        online_cali->cur_protocol);
    mfg_power_cali_para->protocol = online_cali->cur_protocol;

    if (mfg_power_cali_para->curve_idx != curve_idx || mfg_power_cali_para->chain != chain_idx ||
        mfg_power_cali_para->initialized_flag == 0) {
        oam_warning_log4(0, OAM_SF_CALIBRATE,
            "mfg cali para change:bw[%d], protocol[%d], before curve[%u], after curve[%u]\n",
            online_cali->band_width, online_cali->cur_protocol, mfg_power_cali_para->curve_idx, curve_idx);
        fe_mfg_init_power_cali_para(online_cali->channel_info.band, curve_idx, chain_idx);
    }

    for (i = 0; i < max_param_num; i++) {
        /* 功率一致的情况下, 进行保存 */
        if (mfg_power_cali_ipa_para->exp_power[i] != exp_pow) {
            continue;
        }
        previous_save_flag = (mfg_power_cali_para->save_flag >> i) & 1;
        mfg_power_cali_ipa_para->chip_pdet[i] = pdet_cal;
        mfg_power_cali_para->save_flag |= ((osal_u8)(1 << i));
        if (previous_save_flag == 0) {
            wifi_printf_always("mfg pdet power saved: tpc code[%u],flag[0x%x]\n",
                online_cali->tpc_code, mfg_power_cali_para->save_flag);
            oam_warning_log3(0, OAM_SF_CALIBRATE,
                "fe_mfg_tx_pow_save_equip_param:chip_pdet[0x%x],flag[0x%x],tpc[%u]",
                mfg_power_cali_ipa_para->chip_pdet[i], mfg_power_cali_para->save_flag, online_cali->tpc_code);
        }
        break;
    }
}
/*****************************************************************************
 函 数 名  : fe_mfg_calculate_curve_c
 功能描述  : 曲线计算单组方程的常数项c
*****************************************************************************/
OSAL_STATIC osal_s64 fe_mfg_calculate_curve_c(fe_custom_power_curve_stru *curve,
    fe_custom_power_curve_stru *factor, osal_s64 x, osal_s64 y)
{
    /* 方程 aX^2 + bX + c = Y, 获取芯片上报功率 */
    osal_s64 chip_pwr = ((x * x * curve->pow_par2) >> factor->pow_par2) + ((x * curve->pow_par1) >> factor->pow_par1) +
        (curve->pow_par0 >> factor->pow_par0);
    // 计算偏差
    osal_s64 diff = y - chip_pwr;
    // 补偿偏差到原常数项
    return ((diff << factor->pow_par0) + curve->pow_par0);
}
/*****************************************************************************
 函 数 名  : fe_mfg_calculate_power_curve_c
 功能描述  : 计算ipa曲线常数项c
*****************************************************************************/
OSAL_STATIC osal_s32 fe_mfg_calculate_power_curve_c(fe_mfg_power_cali_para_stru *mfg_power_cali_para,
    osal_u8 power_level)
{
    fe_mfg_power_ipa_para_stru *mfg_power_cali_ipa_para = &(mfg_power_cali_para->ipa_para);
    fe_custom_power_curve_stru curve = {0};
    fe_custom_power_curve_stru factor = {0};
    osal_s64 x = mfg_power_cali_ipa_para->chip_pdet[power_level];
    osal_s64 y = mfg_power_cali_ipa_para->inst_power[power_level];
    osal_s64 param_c = 0;
    memcpy_s(&curve, sizeof(curve),
        mfg_power_cali_ipa_para->power_curve[power_level], sizeof(mfg_power_cali_ipa_para->power_curve[power_level]));
    memcpy_s(&factor, sizeof(factor),
        mfg_power_cali_ipa_para->curve_factor[power_level], sizeof(mfg_power_cali_ipa_para->curve_factor[power_level]));
    /* 每组方程分别求解未知数c, 求和取平均值 */
    param_c = fe_mfg_calculate_curve_c(&curve, &factor, x, y);
    oam_info_log4(0, OAM_SF_CALIBRATE,
        "level:%d,p_raw(x):%lld, inst_pow(y):%lld, result c:%d\n", power_level, x, y, param_c);
    if (get_abs(param_c) > 0x7FFF) {
        oam_error_log1(0, OAM_SF_CALIBRATE,
            "{fe_mfg_calculate_power_curve_c::param c[%d] out of range}\n", param_c);
        return OAL_FAIL;
    }
    mfg_power_cali_para->ipa_para.power_curve[power_level][FE_MFG_POLYNOMIAL_NUM - 1] = (osal_s16)param_c;
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_cal_and_save_power_curve_param
 功能描述  : 计算ipa曲线参数
*****************************************************************************/
OSAL_STATIC osal_s32 fe_mfg_cal_and_save_power_curve_param(osal_u8 band)
{
    osal_s32 ret ;
    fe_mfg_power_cali_para_stru *mfg_power_cali_para = fe_mfg_get_cali_para();
    osal_u8 curve_idx = mfg_power_cali_para->curve_idx;
    osal_u16 len = 0;
    fe_custom_power_cali_stru *cust_power_cali = fe_custom_get_power_cali(band, &len);
    osal_u8 cal_flag = 0x3;

    if (cal_flag != mfg_power_cali_para->save_flag) {
        oam_warning_log2(0, OAM_SF_CALIBRATE,
            "{fe_mfg_cal_and_save_power_curve_param::condition cal flag[%x], save flag[%x]!}\n",
            cal_flag, mfg_power_cali_para->save_flag);
        return OAL_FAIL;
    }

    /* 分别计算高低功率下的曲线 */
    ret = fe_mfg_calculate_power_curve_c(mfg_power_cali_para, FE_CUS_HIGH_POW);
    ret |= fe_mfg_calculate_power_curve_c(mfg_power_cali_para, FE_CUS_LOW_POW);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 曲线参数c, 保存到定制化 */
    cust_power_cali->power_cali_2g.power_curve[FE_CUS_HIGH_POW][curve_idx][FE_RF_CUS_POLYNOMIAL_NUM - 1] =
        mfg_power_cali_para->ipa_para.power_curve[FE_CUS_HIGH_POW][FE_RF_CUS_POLYNOMIAL_NUM - 1];
    cust_power_cali->power_cali_2g.power_curve[FE_CUS_LOW_POW][curve_idx][FE_RF_CUS_POLYNOMIAL_NUM - 1] =
        mfg_power_cali_para->ipa_para.power_curve[FE_CUS_LOW_POW][FE_RF_CUS_POLYNOMIAL_NUM - 1];
    g_mfg_power_cali_para.initialized_flag = 0;
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_set_curve_factor
 功能描述  : 产线校准命令, 写到定制化曲线放大系数
*****************************************************************************/
osal_s32 fe_mfg_power_set_curve_factor(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_u8 flag = mfg_param->param[0]; /* 第一个参数flag: 0,1,2 */
    osal_u8 level_idx;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_curve_factor:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    if (flag == 0) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        wlan_cust_rf_fe_params *rf_fe_ini = hwifi_get_rf_fe_custom_ini();
        fe_custom_config_power_cali_curve_factor(rf_fe_ini->rf_power_cali.curve_factor_2g,
            sizeof(rf_fe_ini->rf_power_cali.curve_factor_2g));
#endif
        return OAL_SUCC;
    }
    if (flag > FE_CUS_POW_BUTT) {
        return OAL_FAIL;
    }
    level_idx = flag - 1;
    if (fe_custom_set_curve_factor(hmac_vap->channel.band, mfg_param->param + 1,
        sizeof(mfg_param->param) - sizeof(mfg_param->param[0]), level_idx) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_get_curve_factor
 功能描述  : 产线校准命令, 从定制化读取曲线放大系数
*****************************************************************************/
osal_s32 fe_mfg_power_get_curve_factor(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_u8 level_idx;
    osal_u8 count = 0;
    fe_custom_power_curve_stru output_factor = {0};
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_get_curve_factor:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    for (level_idx = 0; level_idx < FE_CUS_POW_BUTT; level_idx++) {
        output_factor = fe_custom_get_curve_factor(hmac_vap->channel.band, level_idx);
        mfg_param->output_param[count++] = output_factor.pow_par2;
        mfg_param->output_param[count++] = output_factor.pow_par1;
        mfg_param->output_param[count++] = output_factor.pow_par0;
    }
    mfg_param->output_param_num = count;

    oam_info_log1(0, OAM_SF_CALIBRATE, "{fe_mfg_power_get_curve_factor:output_param_num:%d!\n}",
        mfg_param->output_param_num);
    oam_info_log0(0, OAM_SF_CALIBRATE, "factor:\n");
    for (count = 0; count < mfg_param->output_param_num; count++) {
        oam_info_log1(0, OAM_SF_CALIBRATE, "%d\n", mfg_param->output_param[count]);
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_set_curve_param
 功能描述  : 产线校准命令, 写入定制化高功率曲线参数
*****************************************************************************/
osal_s32 fe_mfg_power_set_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_u8 flag = mfg_param->param[0]; /* 第一个参数flag: 0,1 */
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_curve_param:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    if (flag == 0) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        wlan_cust_rf_fe_params *rf_fe_ini = hwifi_get_rf_fe_custom_ini();
        fe_custom_config_power_cali_power_curve_high(rf_fe_ini->rf_power_cali.power_cure_2g[0],
            sizeof(rf_fe_ini->rf_power_cali.power_cure_2g[0]));
#endif
        return OAL_SUCC;
    }
    if (fe_custom_set_all_power_curve(hmac_vap->channel.band, mfg_param->param + 1,
        sizeof(mfg_param->param) - sizeof(mfg_param->param[0]), FE_CUS_HIGH_POW) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_set_low_curve_param
 功能描述  : 低功率曲线参数, 写入定制化
*****************************************************************************/
osal_s32 fe_mfg_power_set_low_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_u8 flag = mfg_param->param[0]; /* 第一个参数flag: 0,1 */
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_low_curve_param:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    if (flag == 0) {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        wlan_cust_rf_fe_params *rf_fe_ini = hwifi_get_rf_fe_custom_ini();
        fe_custom_config_power_cali_power_curve_low(rf_fe_ini->rf_power_cali.power_cure_2g[1],
            sizeof(rf_fe_ini->rf_power_cali.power_cure_2g[1]));
#endif
        return OAL_SUCC;
    }
    if (fe_custom_set_all_power_curve(hmac_vap->channel.band, mfg_param->param + 1,
        sizeof(mfg_param->param) - sizeof(mfg_param->param[0]), FE_CUS_LOW_POW) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_get_power_curve_by_level
 功能描述  : 从定制化读取曲线参数
*****************************************************************************/
OSAL_STATIC osal_s32 fe_mfg_get_power_curve_by_level(hmac_vap_stru *hmac_vap,
    mfg_param_stru *mfg_param, osal_u8 level_idx)
{
    osal_u8 count;
    if (fe_custom_get_all_power_curve(hmac_vap->channel.band,
        mfg_param->output_param, sizeof(mfg_param->output_param),
        &(mfg_param->output_param_num), level_idx) != OAL_SUCC) {
        return OAL_FAIL;
    }

    oam_info_log1(0, OAM_SF_CALIBRATE, "{fe_mfg_get_power_curve_by_level:output_param_num:%d!\n}",
        mfg_param->output_param_num);
    oam_info_log0(0, OAM_SF_CALIBRATE, "curve param:\n");
    for (count = 0; count < mfg_param->output_param_num; count++) {
        oam_info_log1(0, OAM_SF_CALIBRATE, "%d\n", mfg_param->output_param[count]);
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_get_curve_param
 功能描述  : 产线校准命令, 从定制化读取高功率曲线参数
*****************************************************************************/
osal_s32 fe_mfg_power_get_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_s32 ret;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_get_curve_param:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    ret = fe_mfg_get_power_curve_by_level(hmac_vap, mfg_param, FE_CUS_HIGH_POW);
    return ret;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_get_low_curve_param
 功能描述  : 产线校准命令, 从定制化读取低功率曲线参数
*****************************************************************************/
osal_s32 fe_mfg_power_get_low_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_s32 ret;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_get_low_curve_param:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    ret = fe_mfg_get_power_curve_by_level(hmac_vap, mfg_param, FE_CUS_LOW_POW);
    return ret;
}
/*****************************************************************************
 函 数 名  : fe_mfg_set_instrument_power
 功能描述  : 产线校准设置2个实测功率
*****************************************************************************/
OSAL_STATIC osal_s32 fe_mfg_set_instrument_power(osal_s32 *param, osal_u16 len)
{
    osal_u8 tpc_num = FE_MFG_POW_PARAM_NUM;
    osal_s32 param_tmp[FE_MFG_POW_PARAM_NUM] = {0};
    osal_u8 tpc_idx = 0;
    osal_s16 *inst_power;

    if (memcpy_s(param_tmp, sizeof(param_tmp), param, sizeof(param_tmp)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_set_instrument_power:: memcpy fail!}\n");
        return OAL_FAIL;
    }
    /* 实测功率递减 */
    if (param_tmp[FE_CUS_LOW_POW] > param_tmp[FE_CUS_HIGH_POW]) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_set_instrument_power:: param not in discending order!}\n");
        return OAL_FAIL;
    }
    inst_power = g_mfg_power_cali_para.ipa_para.inst_power;
    for (tpc_idx = 0; tpc_idx < tpc_num; tpc_idx++) {
        inst_power[tpc_idx] = (osal_s16)param_tmp[tpc_idx];
        oam_warning_log2(0, OAM_SF_CALIBRATE, "{fe_mfg_set_instrument_power::instrument power[%d] = %d!}\n",
            tpc_idx, inst_power[tpc_idx]);
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_set_tar_power
 功能描述  : 产线校准命令, 设置将要常发的2个目标功率
*****************************************************************************/
osal_s32 fe_mfg_power_set_tar_power(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_u8 tpc_num = FE_MFG_POW_PARAM_NUM;
    osal_s32 param_tmp[FE_MFG_POW_PARAM_NUM] = {0};
    osal_u8 tpc_idx = 0;
    osal_s16 *tar_param;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_tar_power:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    if (memcpy_s(param_tmp, sizeof(param_tmp), mfg_param->param, sizeof(param_tmp)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_tar_power:: memcpy fail!}\n");
        return OAL_FAIL;
    }
    /* 目标功率递减 */
    if (param_tmp[FE_CUS_LOW_POW] > param_tmp[FE_CUS_HIGH_POW]) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_tar_power:: param not in ascending order!}\n");
        return OAL_FAIL;
    }
    tar_param = g_mfg_power_cali_para.ipa_para.exp_power;
    for (tpc_idx = 0; tpc_idx < tpc_num; tpc_idx++) {
        tar_param[tpc_idx] = (osal_u8)param_tmp[tpc_idx];
        oam_warning_log2(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_tar_power::target power[%d] = %d!}\n",
            tpc_idx, tar_param[tpc_idx]);
    }
    /* 刷新tpc code时刷新下flag 重新记录 */
    g_mfg_power_cali_para.save_flag = 0;
    g_mfg_power_cali_para.curve_cal_mode = OAL_TRUE;
    g_mfg_power_cali_para.initialized_flag = 0;
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : fe_mfg_power_set_tar_cali_power
 功能描述  : 产线校准命令, 设置2个实测功率, 并计算曲线参数
*****************************************************************************/
osal_s32 fe_mfg_power_set_tar_cali_power(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    osal_s32 ret = OAL_FAIL;
    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_power_set_tar_cali_power:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    if (fe_mfg_set_instrument_power(mfg_param->param, sizeof(mfg_param->param)) != OAL_SUCC) {
        return OAL_FAIL;
    }
    ret = fe_mfg_cal_and_save_power_curve_param(hmac_vap->channel.band);
    /* 进入曲线计算则不论成功与否 都恢复标志默认状态 根据返回的信息来确认是否执行成功 */
    g_mfg_power_cali_para.curve_cal_mode = OAL_FALSE;
    g_mfg_power_cali_para.save_flag = 0;
    return ret;
}
OSAL_STATIC osal_u8 fe_mfg_get_cali_subband_idx(osal_u8 band, osal_u8 subband_idx)
{
    unref_param(band);
    if (subband_idx <= FE_MFG_CHANNEL_IDX4_IDX) {
        return FE_MFG_CALI_SUB_BAND_IDX0; /* cali_subband_idx0:0~3 */
    }
    if (subband_idx <= FE_MFG_CHANNEL_IDX9_IDX) {
        return FE_MFG_CALI_SUB_BAND_IDX1; /* cali_subband_idx1:4~8 */
    }
    return FE_MFG_CALI_SUB_BAND_IDX2; /* cali_subband_idx2:9~13 */
}
/*****************************************************************************
 函 数 名  : fe_mfg_rssi_set_rssi_offset
 功能描述  : 产线校准命令, 设置RSSI补偿值
*****************************************************************************/
osal_s32 fe_mfg_rssi_set_rssi_offset(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    fe_mfg_rssi_comp_2g_stru rssi = {0};
    frw_msg msg = {0};
    osal_u8 subband_idx = (osal_u8)mfg_param->param[0] - 1; /* 产测第一个byte为channel 1~14 */
    osal_s8 rssi_offset = (osal_s8)mfg_param->param[1]; /* 产测第二个byte为rssi offset */
    osal_u8 cali_subband_idx;
    osal_s32 ret;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_rssi_set_rssi_offset:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    cali_subband_idx = fe_mfg_get_cali_subband_idx(hmac_vap->channel.band, subband_idx);
    rssi.rssi_comp[cali_subband_idx] = rssi_offset;
    rssi.select_subband = (1 << cali_subband_idx); /* bit0~2对应subband0~2设置rssi comp */

    frw_msg_init((osal_u8 *)&rssi, sizeof(rssi), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_PROCESS_RSSI_COMP, &msg, OSAL_TRUE);
    return ret;
}
/*****************************************************************************
 函 数 名  : fe_mfg_rssi_get_rssi_offset
 功能描述  : 产线校准命令, 获取RSSI补偿值
*****************************************************************************/
osal_s32 fe_mfg_rssi_get_rssi_offset(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param)
{
    fe_mfg_rssi_comp_2g_stru rssi = {0};
    frw_msg msg = {0};
    osal_u8 subband_idx = (osal_u8)mfg_param->param[0] - 1; /* 产测第一个byte为channel 1~14 */
    osal_u8 cali_subband_idx;
    osal_s8 rssi_rsp = 0; /* 返回RSSI comp */
    osal_s32 ret;

    if (hmac_vap == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{fe_mfg_rssi_get_rssi_offset:: hmac_vap_stru NULL!}\n");
        return OAL_FAIL;
    }
    cali_subband_idx = fe_mfg_get_cali_subband_idx(hmac_vap->channel.band, subband_idx);
    /* bit4~6对应subband0~2获取rssi comp */
    rssi.select_subband = (1 << (cali_subband_idx + FE_MFG_GET_SUBBAND0_RSSI_COMP_BIT));

    frw_msg_init((osal_u8 *)&rssi, sizeof(rssi), (osal_u8 *)&rssi_rsp, sizeof(rssi_rsp), &msg);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_PROCESS_RSSI_COMP, &msg, OSAL_TRUE);
    mfg_param->output_param[0] = (osal_s8)msg.rsp[0];
    mfg_param->output_param_num = 1;
    oam_info_log2(0, OAM_SF_CALIBRATE,
        "{fe_mfg_rssi_get_rssi_offset:cali subband[%d]rssi offset[%d]!}\n", cali_subband_idx, (osal_s8)msg.rsp[0]);
    return ret;
}
#endif // _PRE_WLAN_FEATURE_MFG_TEST
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif