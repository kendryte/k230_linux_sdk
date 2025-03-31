/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: ppa code二级表初始化参数(各个芯片差异化定制)
 */

#ifndef __POWER_PPA_CTRL_SPEC_H__
#define __POWER_PPA_CTRL_SPEC_H__

#include "wlan_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define CALI_2G_TX_UPC_UNIT_VAL_MAX    (0x3F)   /* asic 3F */
#define CALI_2G_TX_UPC_UNIT_VAL_MIN    (0x1)
#define CALI_POW_BASE_2G_UPC_IDX       (0x0)   /* UPC低档位的起始索引 */
#define CALI_POW_PPA_LUT_NUM           (11)    /* ppa LUT表的行数 */
#ifdef BOARD_ASIC_WIFI
#define CALI_TXPWR_GAIN_LVL_NUM        (2)
#else
#define CALI_TXPWR_GAIN_LVL_NUM        (1)
#endif

typedef enum {
    CALI_POW_TABLE_OFDM = 0,
    CALI_POW_TABLE_DSSS = 1,
    CALI_POW_TABLE_BUTT
} cali_pow_table_type_enum;
typedef osal_u8 cali_pow_table_type_enum_uint8;

typedef enum {
    CALI_SINGLE_STREAM_0 = 0,  /* C0 */
    CALI_STREAM_TYPE_BUTT = 1, /* 代表无效的通道类型 */
} cali_stream_type_enum;
typedef osal_u8 cali_stream_type_enum_uint8;

/* 档位分配占比的参数 */
typedef struct {
    osal_u32    fb_idx :        3,      /* [2:0] */
                pa_idx :        5,      /* [7:3] */
                ppa_idx :       4,      /* [11:8] */
                lpf_idx :       3,      /* [14:12] */
                dac_idx :       2,      /* [16:15] */
                dbb_scale_pre : 8,      /* [24:17] */
                fir_1x :        1,      /* [25:25] */
                resev :         6;      /* [31:26] */
} cali_pow_distri_ratio_stru;

typedef union {
    cali_pow_distri_ratio_stru bits;
    osal_u32 val;
} cali_pow_distri_ratio_union;

typedef struct {
    const osal_s16 pow_gain;
    osal_u16 pow_code;
} cali_pow_crtl_table_stru;

typedef union {
    struct {
        osal_u8 unit;          /* [7:0] */
        osal_u8 gm    : 4;     /* [11:8] */
        osal_u8 slice : 4;     /* [15:12] */
    } bits;
    osal_u16 val;
} cali_upc_code;

osal_u8 cali_get_normal_upc_unit_default(osal_u8 stream_type, osal_u8 upc_idx);
cali_pow_crtl_table_stru *cali_get_ppa_lut(osal_void);
osal_u8 cali_get_base_ppa_idx(osal_u8 txpwr_lvl_idx);
osal_void cali_get_upc_idx_range(osal_u8 txpwr_lvl_idx,
    osal_u8 *start_upc_idx, osal_u8 *end_upc_idx);
osal_u16 cali_get_base_ppa_code(osal_u8 txpwr_lvl_idx);
osal_u8 cali_get_base_pa_code(osal_u8 txpwr_lvl_idx);
osal_u8 cali_get_pwr_lvl_by_ppa_idx(osal_u8 ppa_idx);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __POWER_CTRL_SPEC_H__
