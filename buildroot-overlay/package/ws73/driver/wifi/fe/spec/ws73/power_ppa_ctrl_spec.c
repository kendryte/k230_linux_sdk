/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: power control common
 */
#include "power_ppa_ctrl_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 用于保存ppa(upc) gain target值 */
cali_pow_crtl_table_stru g_ppa_lut[CALI_POW_PPA_LUT_NUM] = {
    { 160,   0x1A2C },  /* Index= 0 */
    { 130,   0x1A14 },  /* Index= 1 */
    { 110,   0x1A12 },  /* Index= 2 */
    { 90,    0x133F },  /* Index= 3 */
    { 80,    0x1336 },  /* Index= 4 */
    { 70,    0x132F },  /* Index= 5 */
    { 60,    0x1322 },  /* Index= 6 */
    { 50,    0x1319 },  /* Index= 7 */
    { 40,    0x1316 },  /* Index= 8 */
    { 10,    0x130C },  /* Index= 9 */
    { -20,   0x1309 },  /* Index= 10 */
};

/*****************************************************************************
    函数定义
*****************************************************************************/
osal_u8 cali_get_normal_upc_unit_default(osal_u8 stream_type, osal_u8 upc_idx)
{
    osal_u8 upc_code_default_unit[CALI_STREAM_TYPE_BUTT][CALI_POW_PPA_LUT_NUM] = {
        {
            0x2C,   /*  Index=0 */
            0x14,   /*  Index=1 */
            0x12,   /*  Index=2 */
            0x3F,   /*  Index=3 */
            0x36,   /*  Index=4 */
            0x2F,   /*  Index=5 */
            0x22,   /*  Index=6 */
            0x19,   /*  Index=7 */
            0x16,   /*  Index=8 */
            0x0C,   /*  Index=9 */
            0x09,   /*  Index=10 */
        },
    };
    if (upc_idx >= CALI_POW_PPA_LUT_NUM) {
        return 0;
    }
    return upc_code_default_unit[stream_type][upc_idx];
}

cali_pow_crtl_table_stru *cali_get_ppa_lut(osal_void)
{
    return g_ppa_lut;
}

osal_u8 cali_get_base_ppa_idx(osal_u8 txpwr_lvl_idx)
{
    osal_u8 base_upc_idx[CALI_TXPWR_GAIN_LVL_NUM] = {
#ifdef BOARD_ASIC_WIFI
        0, 3
#else
        0
#endif
    };
    if (txpwr_lvl_idx >= CALI_TXPWR_GAIN_LVL_NUM) {
        return 0;
    }
    return base_upc_idx[txpwr_lvl_idx];
}

osal_void cali_get_upc_idx_range(osal_u8 txpwr_lvl_idx,
    osal_u8 *start_upc_idx, osal_u8 *end_upc_idx)
{
    osal_u8 start_upc_idx_array[CALI_TXPWR_GAIN_LVL_NUM] = {
#ifdef BOARD_ASIC_WIFI
        0, 2
#else
        0
#endif
    };
    osal_u8 end_upc_idx_array[CALI_TXPWR_GAIN_LVL_NUM] = {
#ifdef BOARD_ASIC_WIFI
        1, CALI_POW_PPA_LUT_NUM - 1
#else
        CALI_POW_PPA_LUT_NUM - 1
#endif
    };
    *start_upc_idx = start_upc_idx_array[txpwr_lvl_idx];
    *end_upc_idx = end_upc_idx_array[txpwr_lvl_idx];
}

// 根据ppa_idx计算power_level
osal_u8 cali_get_pwr_lvl_by_ppa_idx(osal_u8 ppa_idx)
{
    osal_u8 start_idx = 0;
    osal_u8 end_idx = 0;
    osal_u8 level;
    for (level = 0; level < CALI_TXPWR_GAIN_LVL_NUM; level++) {
        cali_get_upc_idx_range(level, &start_idx, &end_idx);
        if (ppa_idx >= start_idx && ppa_idx <= end_idx) {
            return level;
        }
    }
    return 0;
}

osal_u16 cali_get_base_ppa_code(osal_u8 txpwr_lvl_idx)
{
    osal_u8 base_ppa_idx = cali_get_base_ppa_idx(txpwr_lvl_idx);
    return g_ppa_lut[base_ppa_idx].pow_code;
}

osal_u8 cali_get_base_pa_code(osal_u8 txpwr_lvl_idx)
{
    osal_u8 pa_gain_code[CALI_TXPWR_GAIN_LVL_NUM] = {
#ifdef BOARD_ASIC_WIFI
        0x7F, 0x7F
#else
        0x1F
#endif
    };
    if (txpwr_lvl_idx >= CALI_TXPWR_GAIN_LVL_NUM) {
        return 0;
    }
    return pa_gain_code[txpwr_lvl_idx];
}

#ifdef __cplusplus
#if __cplusplus
}

#endif
#endif
