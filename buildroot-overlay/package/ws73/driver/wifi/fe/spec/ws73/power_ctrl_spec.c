/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 功率控制参数(各个芯片差异化定制)
 */

#include "power_ctrl_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

cali_pow_cal_gain_stru g_ast_pow_cal_gain_2g_ofdm_table[CALI_POW_CAL_2G_OFDM_TABLE_LEN] = { 0 };
// 需要刷新dbb_pre 写入phy ram
cali_pow_distri_ratio_stru g_ast_pow_dist_2g_ofdm_table[CALI_POW_DIST_2G_OFDM_TABLE_LEN] = {
     /* fb, pa, ppa, lpf, dac, dbb_pre, fir_1x, resev */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=0   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=2   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=4   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=6   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=8   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=10 TXPWR_FB=1 TXIQ_FB=2 */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=12   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=14   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=16   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=18   */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=20  RXIQ_FB=5 */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=22   */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=24  RXIQ_FB=6 */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=26   */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=28 TXPWR_FB=1 TXIQ_FB=3 */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=30   */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=32   */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=34   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=36   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=38   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=40   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=42   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=44   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=46   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=48   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=50   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=52   */
    { 1, 4, 9, 2, 1, 0, 0, 0 },  /* TPC=54   */
    { 1, 4, 9, 2, 1, 0, 0, 0 },  /* TPC=56   */
    { 1, 6, 9, 2, 1, 0, 0, 0 },  /* TPC=58   */
    { 1, 6, 9, 2, 1, 0, 0, 0 },  /* TPC=60   */
    { 1, 6, 9, 2, 1, 0, 0, 0 },  /* TPC=62   */
    { 1, 7, 9, 2, 1, 0, 0, 0 },  /* TPC=64   */
    { 1, 7, 9, 2, 1, 0, 0, 0 },  /* TPC=66   */
    { 1, 7, 9, 2, 1, 0, 0, 0 },  /* TPC=68   */
    { 1, 8, 9, 2, 1, 0, 0, 0 },  /* TPC=70   */
    { 1, 8, 9, 2, 1, 0, 0, 0 },  /* TPC=72   */
};

cali_pow_cal_gain_stru g_ast_pow_cal_gain_2g_dsss_table[CALI_POW_CAL_2G_DSSS_TABLE_LEN] = { 0 };
// 需要刷新dbb_pre 写入phy ram
cali_pow_distri_ratio_stru g_ast_pow_dist_2g_dsss_table[CALI_POW_DIST_2G_DSSS_TABLE_LEN] = {
     /* fb, pa, ppa, lpf, dac, dbb_pre, fir_1x, resev */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=74   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=76   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=78   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=80   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=82   */
    { 1, 0, 0, 3, 2, 0, 0, 0 },  /* TPC=84 TXPWR_FB=1 TXIQ_FB=2 */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=86   */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=88   */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=90   */
    { 1, 0, 1, 3, 2, 0, 0, 0 },  /* TPC=92   */
    { 1, 0, 4, 3, 2, 0, 0, 0 },  /* TPC=94  RXIQ_FB=5 */
    { 1, 0, 4, 3, 2, 0, 0, 0 },  /* TPC=96   */
    { 1, 0, 4, 3, 2, 0, 0, 0 },  /* TPC=98  RXIQ_FB=6 */
    { 1, 0, 4, 3, 2, 0, 0, 0 },  /* TPC=100   */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=102 TXPWR_FB=1 TXIQ_FB=3 */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=104   */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=106   */
    { 1, 0, 4, 2, 1, 0, 0, 0 },  /* TPC=108   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=110   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=112   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=114   */
    { 1, 0, 7, 2, 1, 0, 0, 0 },  /* TPC=116   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=118   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=120   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=122   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=124   */
    { 1, 0, 9, 2, 1, 0, 0, 0 },  /* TPC=126   */
    { 1, 4, 9, 2, 1, 0, 0, 0 },  /* TPC=128   */
    { 1, 4, 9, 2, 1, 0, 0, 0 },  /* TPC=130   */
    { 1, 6, 9, 2, 1, 0, 0, 0 },  /* TPC=132   */
    { 1, 6, 9, 2, 1, 0, 0, 0 },  /* TPC=134   */
    { 1, 6, 9, 2, 1, 0, 0, 0 },  /* TPC=136   */
    { 1, 7, 9, 2, 1, 0, 0, 0 },  /* TPC=138   */
    { 1, 7, 9, 2, 1, 0, 0, 0 },  /* TPC=140   */
    { 1, 7, 9, 2, 1, 0, 0, 0 },  /* TPC=142   */
    { 1, 8, 9, 2, 1, 0, 0, 0 },  /* TPC=144   */
    { 1, 8, 9, 2, 1, 0, 0, 0 },  /* TPC=146   */
};

cali_pow_cal_gain_stru *g_pow_cal_gain_2g_table[CALI_POW_TABLE_LEN] = {
    g_ast_pow_cal_gain_2g_ofdm_table,    /* ofdm 定义为第一个元素，顺序不能变 */
    g_ast_pow_cal_gain_2g_dsss_table,
};

cali_pow_distri_ratio_stru *g_pow_dist_2g_table[CALI_POW_TABLE_LEN] = {
    g_ast_pow_dist_2g_ofdm_table,
    g_ast_pow_dist_2g_dsss_table,
};

const osal_u8 g_ast_dbb_pre_ofdm_table[CALI_POW_CAL_2G_OFDM_TABLE_LEN] = {
    255,
    241,
    228,
    215,
    203,
    192,
    181,
    171,
    161,
    152,
    144,
    136,
    128,
    121,
    114,
    108,
    102,
    96,
    91,
    86,
    114,
    108,
    102,
    96,
    91,
    86,
    81,
    76,
    203,
    192,
    181,
    171,
    161,
    152,
    144,
    136,
    181,
    171,
    161,
    152,
    144,
    136,
    128,
    121,
    181,
    171,
    161,
    152,
    144,
    136,
    128,
    121,
    114,
    108,
    128,
    121,
    114,
    108,
    144,
    136,
    128,
    121,
    114,
    108,
    128,
    121,
    114,
    108,
    102,
    96,
    108,
    102,
    96,
    91,
};
const osal_u8 g_ast_dbb_pre_dsss_table[CALI_POW_CAL_2G_DSSS_TABLE_LEN] = {
    191,
    181,
    171,
    161,
    152,
    144,
    136,
    128,
    121,
    114,
    108,
    102,
    136,
    128,
    121,
    114,
    108,
    102,
    96,
    91,
    152,
    144,
    136,
    128,
    121,
    114,
    108,
    102,
    152,
    144,
    136,
    128,
    121,
    114,
    108,
    102,
    136,
    128,
    121,
    114,
    108,
    102,
    96,
    91,
    136,
    128,
    121,
    114,
    108,
    102,
    96,
    91,
    85,
    81,
    96,
    91,
    85,
    81,
    108,
    102,
    96,
    91,
    85,
    81,
    96,
    91,
    85,
    81,
    76,
    72,
    81,
    76,
    72,
    68,
};

const osal_u8 *g_dbb_pre_table[CALI_POW_TABLE_LEN] = {
    g_ast_dbb_pre_ofdm_table,
    g_ast_dbb_pre_dsss_table,
};

/* 用于保存pa gain target值 */
const cali_pow_crtl_table_stru g_pa_pow_ctrl[CALI_POW_PA_LUT_NUM] = {
    { 150, 0x7F },  /* Index= 0 */
    { 150, 0x7F },  /* Index= 1 */
    { 150, 0x7F },  /* Index= 2 */
    { 140, 0x7D },  /* Index= 3 */
    { 130, 0x7B },  /* Index= 4 */
    { 115, 0x79 },  /* Index= 5 */
    { 100, 0x77 },  /* Index= 6 */
    { 80,  0x75 },  /* Index= 7 */
    { 65,  0x74 },  /* Index= 8 */
    { 49,  0x73 },  /* Index= 9 */
    { 28,  0x72 },  /* Index= 10 */
};
/*****************************************************************************
    函数定义
*****************************************************************************/
cali_pow_cal_gain_stru* cali_get_pow_cal_gain_2g_table(osal_u8 pow_table_idx)
{
    return g_pow_cal_gain_2g_table[pow_table_idx];
}

cali_pow_distri_ratio_stru* cali_get_2g_pow_dist_table(osal_u8 tab_idx)
{
    return g_pow_dist_2g_table[tab_idx];
}

const osal_u8* cali_get_dbb_pre_table(osal_u8 pow_table_idx)
{
    return g_dbb_pre_table[pow_table_idx];
}

const cali_pow_crtl_table_stru* cali_get_pa_pow_ctrl(const hal_device_stru *device)
{
    unref_param(device);
    return g_pa_pow_ctrl;
}

/*
 * 函 数 名  : cali_get_dbb_shadow_bounds
 * 功能描述  : dbb shadow边界值
 */
const osal_u8 cali_get_dbb_shadow_bounds(osal_u8 bound_idx)
{
    /* dbb shadow边界, 按照phy寄存器表单初值进行初始化 */
    osal_u8 bounds[CALI_DBB_DOUNDS] = { 0x12, 0x18, 0x24, 0x2C, 0x36, 0x3A, 0x40 };
    if (bound_idx < CALI_DBB_DOUNDS) {
        return bounds[bound_idx];
    }
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
