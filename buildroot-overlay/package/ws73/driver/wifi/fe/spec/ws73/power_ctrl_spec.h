/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准模块的规格定义(各个芯片差异化定制)
 */

#ifndef __POWER_CTRL_SPEC_H__
#define __POWER_CTRL_SPEC_H__

#if defined(_PRE_WLAN_ATE) || defined(_PRE_WIFI_EDA)
#include "hal_device_rom.h"
#else
#include "hal_device.h"
#endif
#include "power_ppa_ctrl_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define FE_CUSTOMIZE_MAX_CHIP_POWER 230 // 芯片最大功率
// 目标功率数据长度
#define FE_CUSTOMIZE_TARGET_POWER_11B_LEN 4
#define FE_CUSTOMIZE_TARGET_POWER_11G_LEN 8
#define FE_CUSTOMIZE_TARGET_POWER_20M_LEN 10
#define FE_CUSTOMIZE_TARGET_POWER_40M_LEN 11
#define FE_CUSTOMIZE_TARGET_POWER_LEN (FE_CUSTOMIZE_TARGET_POWER_11B_LEN + FE_CUSTOMIZE_TARGET_POWER_11G_LEN + \
                                        FE_CUSTOMIZE_TARGET_POWER_20M_LEN + FE_CUSTOMIZE_TARGET_POWER_40M_LEN)
#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE

/* 待回片后确定数值 */
#define CALI_DBB_SCALING_PRE_BASE             (128)  /* 2.4g6M 基准功率差值 delt_pwr = 20*ln(256)单位0.01 db */
#define CALI_2G_POWER_RANGE_DSSS_DELT_POW_MAX (0)    /* 30 to 0 */
#define CALI_2G_POWER_RANGE_DSSS_DELT_POW_MIN (-255)
#define CALI_2G_POWER_RANGE_OFDM_DELT_POW_MAX (0)    /* 30 to 0 */
#define CALI_2G_POWER_RANGE_OFDM_DELT_POW_MIN (-315)

#define CALI_POW_DIST_2G_OFDM_TABLE_LEN       (37)
#define CALI_POW_CAL_2G_OFDM_TABLE_LEN        (CALI_POW_DIST_2G_OFDM_TABLE_LEN << 1)
#define CALI_POW_DIST_2G_DSSS_TABLE_LEN       (37)
#define CALI_POW_CAL_2G_DSSS_TABLE_LEN        (CALI_POW_DIST_2G_DSSS_TABLE_LEN << 1)

/* 功率增益表格设置步长为0.5dBm */
#define CALI_DIST_POW_INTERVAL                (5)
#define CALI_DIST_POW_INTV_NUM                (2)

#define CALI_DYN_POW_MAX                      (0)
#define CALI_DYN_POW_MIN                      (-370) /* 0 to -370 */

/* TPC功率表个数，分为11b和ofdm */
#define CALI_POW_TABLE_LEN                    (CALI_POW_TABLE_BUTT)
#define CALI_POW_PA_LUT_NUM                   (11)

#define CALI_DBB_DOUNDS                       (0x7)

typedef struct {
    const cali_pow_distri_ratio_stru *pow_dist;
    osal_s16 pow_gain;  // 0.1dBm
    osal_u8 tpc_idx;
    osal_u8 rsv;
} cali_pow_cal_gain_stru;

cali_pow_cal_gain_stru* cali_get_pow_cal_gain_2g_table(osal_u8 pow_table_idx);
cali_pow_distri_ratio_stru* cali_get_2g_pow_dist_table(osal_u8 tab_idx);
const osal_u8* cali_get_dbb_pre_table(osal_u8 pow_table_idx);
const cali_pow_crtl_table_stru* cali_get_pa_pow_ctrl(const hal_device_stru *device);
const osal_u8 cali_get_dbb_shadow_bounds(osal_u8 bound_idx);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __POWER_CTRL_SPEC_H__
