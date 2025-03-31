/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize power cali 头文件
 */

#ifndef _FE_RF_CUSTOMIZE_POWER_CALI_H_
#define _FE_RF_CUSTOMIZE_POWER_CALI_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
  2 宏定义
*************************************************************************/
#define FE_RF_CUS_CALI_SUBBAND_NUM         (3) /* RF划分子频段数目 */
#define FE_RF_CUS_POLYNOMIAL_NUM           (3) /* 二次曲线参数个数 */
#define FE_RF_CUS_HIGH_LOW_POW_THR         (150) /* 高低功率曲线划分功率 0.1dBm */
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 高低功率 */
typedef enum {
    FE_CUS_HIGH_POW,
    FE_CUS_LOW_POW,
    FE_CUS_POW_BUTT
} fe_custom_power_enum;

/* 曲线对应的协议定义 */
typedef enum {
    FE_CUS_CURVE_11B = 0,
    FE_CUS_CURVE_OFDM20M,
    FE_CUS_CURVE_OFDM40M,
    FE_CUS_CURVE_PROTOCOL_BUTT
} fe_custom_curve_enum;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_s16 pow_par2; /* 二次项系数 */
    osal_s16 pow_par1; /* 一次 */
    osal_s16 pow_par0; /* 常数项 */
    osal_s16 rsv;
} fe_custom_power_curve_stru;

/* 下发的上电期望功率和曲线 */
typedef struct {
    osal_s16 ref_power[FE_CUS_POW_BUTT][FE_RF_CUS_CALI_SUBBAND_NUM]; /* 期望功率 */
    osal_s16 power_curve[FE_CUS_POW_BUTT][FE_CUS_CURVE_PROTOCOL_BUTT][FE_RF_CUS_POLYNOMIAL_NUM]; /* 曲线参数 */
    osal_s16 curve_factor[FE_CUS_POW_BUTT][FE_RF_CUS_POLYNOMIAL_NUM]; /* 曲线放大系数 */
} fe_custom_power_cali_2g_stru;

typedef struct {
    fe_custom_power_cali_2g_stru power_cali_2g;
} fe_custom_power_cali_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void fe_custom_config_power_cali(osal_u8 band, osal_u8 *param);
osal_void fe_custom_config_power_cali_ref_power(osal_s16 *ref_power, osal_u16 date_len);
osal_void fe_custom_config_power_cali_power_curve_high(osal_s16 *power_curve, osal_u16 date_len);
osal_void fe_custom_config_power_cali_power_curve_low(osal_s16 *power_curve, osal_u16 date_len);
osal_void fe_custom_config_power_cali_curve_factor(osal_s16 *curve_factor, osal_u16 date_len);
osal_s16 fe_custom_get_ref_power(osal_u8 band, osal_u8 level_idx, osal_u8 subband_idx);
fe_custom_power_curve_stru fe_custom_get_power_curve(osal_u8 band, osal_u8 level_idx,
    osal_u8 protocol_idx);
fe_custom_power_cali_stru *fe_custom_get_power_cali(osal_u8 band, osal_u16 *len);
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
osal_u8 fe_custom_get_curve_protocol_idx(osal_u8 band, osal_u8 bandwidth, osal_u8 protocol);
osal_s32 fe_custom_set_all_power_curve(osal_u8 band, osal_s32 *param, osal_u16 len, osal_u8 level_idx);
osal_s32 fe_custom_get_all_power_curve(osal_u8 band, osal_s32 *param, osal_u16 len,
    osal_u32 *out_param_len, osal_u8 level_idx);
osal_s32 fe_custom_set_curve_factor(osal_u8 band, osal_s32 *param, osal_u16 len, osal_u8 level_idx);
#endif
fe_custom_power_curve_stru fe_custom_get_curve_factor(osal_u8 band, osal_u8 level_idx);
osal_u32 fe_custom_check_curve(fe_custom_power_curve_stru *power_curve,
    fe_custom_power_curve_stru *factor, osal_s16 p_val);
osal_s32 fe_custom_calc_curve_pow(fe_custom_power_curve_stru *power_curve,
    fe_custom_power_curve_stru *factor, osal_s16 p_val, osal_s16 *power);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE
#endif  // _FE_RF_CUSTOMIZE_POWER_CALI_H_
