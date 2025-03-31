/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: 产线涉及到的hal层接口
 */
#include "fe_hal_equipment.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "fe_rf_customize_power_cali.h"
#include "efuse_opt.h"
#include "hal_common_ops.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FE_HAL_EQUIPMENT_C

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define FE_EFUSE_CALI_BAND_NUM 3
// efuse接口
OSAL_STATIC osal_u32 fe_efuse_get_curve_factor_c(osal_u8 *value, osal_u8 index)
{
    osal_u64 state = 0;
    osal_u8 factor_value = 0;
    osal_u8 factor_id = EXT_EFUSE_MAX;
    osal_u8 efuse_id[][FE_CUS_POW_BUTT] = {
        { EXT_EFUSE_CURV_FACTOR_HIGN_1_ID, EXT_EFUSE_CURV_FACTOR_LOW_1_ID },
        { EXT_EFUSE_CURV_FACTOR_HIGN_2_ID, EXT_EFUSE_CURV_FACTOR_LOW_2_ID },
    };
    osal_u8 lock_id[][FE_CUS_POW_BUTT] = {
        { EXT_EFUSE_LOCK_CURV_FACTOR_HIGN_1_ID, EXT_EFUSE_LOCK_CURV_FACTOR_LOW_1_ID },
        { EXT_EFUSE_LOCK_CURV_FACTOR_HIGN_2_ID, EXT_EFUSE_LOCK_CURV_FACTOR_LOW_2_ID },
    };
    osal_u8 read_times = sizeof(efuse_id) / sizeof(efuse_id[0]);
    osal_u8 read_idx;

    if (uapi_efuse_get_lockstat(&state) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{fe_efuse_get_curve_factor_c, get efuse stat fail}");
        return OAL_FAIL;
    }
    for (read_idx = 0; read_idx < read_times; read_idx++) {
        if ((((osal_u64)1 << (lock_id[read_idx][index] - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
            factor_id = efuse_id[read_idx][index]; /* 遍历两组，取最后一组 */
        }
    }
    if (factor_id == EXT_EFUSE_MAX) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{fe_efuse_get_curve_factor_c:lockstat zero index[%d].}", index);
        return OAL_FAIL;
    }
    if (uapi_efuse_read(factor_id, &factor_value, sizeof(osal_u8)) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{fe_efuse_get_curve_factor_c:uapi_efuse_read failed index[%d].}", index);
        return OAL_FAIL;
    }
    *value = factor_value;
    return OAL_SUCC;
}
OSAL_STATIC osal_u32 fe_efuse_get_power_curve_c(osal_s16 *value, osal_u8 index)
{
    osal_u64 state = 0;
    osal_s16 curve_value = 0;
    osal_u8 curve_id = EXT_EFUSE_MAX;
    osal_u8 efuse_id[][FE_CUS_CURVE_PROTOCOL_BUTT * FE_CUS_POW_BUTT] = {
        {
            EXT_EFUSE_11B_HIGN_1_ID, EXT_EFUSE_11B_LOW_1_ID,
            EXT_EFUSE_OFDM_20M_HIGN_1_ID, EXT_EFUSE_OFDM_20M_LOW_1_ID,
            EXT_EFUSE_OFDM_40M_HIGN_1_ID, EXT_EFUSE_OFDM_40M_LOW_1_ID,
        },
        {
            EXT_EFUSE_11B_HIGN_2_ID, EXT_EFUSE_11B_LOW_2_ID,
            EXT_EFUSE_OFDM_20M_HIGN_2_ID, EXT_EFUSE_OFDM_20M_LOW_2_ID,
            EXT_EFUSE_OFDM_40M_HIGN_2_ID, EXT_EFUSE_OFDM_40M_LOW_2_ID,
        }
    };
    osal_u8 lock_id[][FE_CUS_CURVE_PROTOCOL_BUTT * FE_CUS_POW_BUTT] = {
        {
            EXT_EFUSE_LOCK_11B_HIGN_1_ID, EXT_EFUSE_LOCK_11B_LOW_1_ID,
            EXT_EFUSE_LOCK_OFDM_20M_HIGN_1_ID, EXT_EFUSE_LOCK_OFDM_20M_LOW_1_ID,
            EXT_EFUSE_LOCK_OFDM_40M_HIGN_1_ID, EXT_EFUSE_LOCK_OFDM_40M_LOW_1_ID,
        },
        {
            EXT_EFUSE_LOCK_11B_HIGN_2_ID, EXT_EFUSE_LOCK_11B_LOW_2_ID,
            EXT_EFUSE_LOCK_OFDM_20M_HIGN_2_ID, EXT_EFUSE_LOCK_OFDM_20M_LOW_2_ID,
            EXT_EFUSE_LOCK_OFDM_40M_HIGN_2_ID, EXT_EFUSE_LOCK_OFDM_40M_LOW_2_ID,
        }
    };
    osal_u8 read_times = sizeof(efuse_id) / sizeof(efuse_id[0]);
    osal_u8 read_idx;

    if (uapi_efuse_get_lockstat(&state) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{fe_efuse_get_power_curve_c, get efuse stat fail}");
        return OAL_FAIL;
    }
    for (read_idx = 0; read_idx < read_times; read_idx++) {
        if ((((osal_u64)1 << (lock_id[read_idx][index] - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
            curve_id = efuse_id[read_idx][index]; /* 遍历两组，取最后一组 */
        }
    }
    if (curve_id == EXT_EFUSE_MAX) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{fe_efuse_get_power_curve_c:lockstat zero index[%d].}", index);
        return OAL_FAIL;
    }
    if (uapi_efuse_read(curve_id, (osal_u8 *)(&curve_value), sizeof(curve_value)) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{fe_efuse_get_power_curve_c:uapi_efuse_read failed index[%d].}", index);
        return OAL_FAIL;
    }
    *value = curve_value;
    return OAL_SUCC;
}
OSAL_STATIC osal_u32 fe_efuse_get_rssi_comp(osal_u8 *value, osal_u8 index)
{
    osal_u64 state = 0;
    osal_u8 rssi_value = 0;
    osal_u8 rssi_id = EXT_EFUSE_MAX;
    osal_u8 efuse_id[][FE_EFUSE_CALI_BAND_NUM] = {
        { EXT_EFUSE_RSSI_BAND1_1_ID, EXT_EFUSE_RSSI_BAND2_1_ID, EXT_EFUSE_RSSI_BAND3_1_ID },
        { EXT_EFUSE_RSSI_BAND1_2_ID, EXT_EFUSE_RSSI_BAND2_2_ID, EXT_EFUSE_RSSI_BAND3_2_ID },
    };
    osal_u8 lock_id[][FE_EFUSE_CALI_BAND_NUM] = {
        { EXT_EFUSE_LOCK_RSSI_BAND1_1_ID, EXT_EFUSE_LOCK_RSSI_BAND2_1_ID, EXT_EFUSE_LOCK_RSSI_BAND3_1_ID },
        { EXT_EFUSE_LOCK_RSSI_BAND1_2_ID, EXT_EFUSE_LOCK_RSSI_BAND2_2_ID, EXT_EFUSE_LOCK_RSSI_BAND3_2_ID },
    };
    osal_u8 read_times = sizeof(efuse_id) / sizeof(efuse_id[0]);
    osal_u8 read_idx;

    if (uapi_efuse_get_lockstat(&state) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{fe_efuse_get_rssi_comp, get efuse stat fail}");
        return OAL_FAIL;
    }
    for (read_idx = 0; read_idx < read_times; read_idx++) {
        if ((((osal_u64)1 << (lock_id[read_idx][index] - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
            rssi_id = efuse_id[read_idx][index]; /* 遍历两组，取最后一组 */
        }
    }
    if (rssi_id == EXT_EFUSE_MAX) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{fe_efuse_get_rssi_comp:lockstat zero index[%d].}", index);
        return OAL_FAIL;
    }
    if (uapi_efuse_read(rssi_id, &rssi_value, sizeof(osal_u8)) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CALIBRATE, "{fe_efuse_get_rssi_comp:uapi_efuse_read failed index[%d].}", index);
        return OAL_FAIL;
    }
    *value = (osal_u8)((osal_s8)(rssi_value << NUM_3_BITS) >> NUM_3_BITS);
    return OAL_SUCC;
}
osal_u32 fe_efuse_get_all_power_cali(osal_u8 *value, osal_u8 len)
{
    fe_custom_power_cali_stru *power_cali = (fe_custom_power_cali_stru *)value;
    osal_u8 factor_c = 0;
    osal_s16 curve_c = 0;
    osal_u8 param_c_idx = 0x2;
    osal_u8 lvl_idx;
    osal_u8 protocol_idx;
    osal_u32 ret = OAL_SUCC;
    unref_param(len);

    for (lvl_idx = 0; lvl_idx < FE_CUS_POW_BUTT; lvl_idx++) {
        if (fe_efuse_get_curve_factor_c(&factor_c, lvl_idx) != OAL_SUCC) {
            ret |= OAL_FAIL;
            continue;
        }
        power_cali->power_cali_2g.curve_factor[lvl_idx][param_c_idx] = factor_c;
    }
    for (protocol_idx = 0; protocol_idx < FE_CUS_CURVE_PROTOCOL_BUTT; protocol_idx++) {
        for (lvl_idx = 0; lvl_idx < FE_CUS_POW_BUTT; lvl_idx++) {
            if (fe_efuse_get_power_curve_c(&curve_c, protocol_idx * FE_CUS_POW_BUTT + lvl_idx) != OAL_SUCC) {
                ret |= OAL_FAIL;
                continue;
            }
            power_cali->power_cali_2g.power_curve[lvl_idx][protocol_idx][param_c_idx] = curve_c;
        }
    }
    return ret;
}
osal_u32 fe_efuse_get_all_rssi_comp(osal_u8 *value, osal_u8 len)
{
    osal_u32 ret = OAL_SUCC;
    osal_u8 idx;
    unref_param(len);

    for (idx = 0; idx < FE_EFUSE_CALI_BAND_NUM; idx++) {
        ret |= fe_efuse_get_rssi_comp(&value[idx], idx);
    }
    return ret;
}
#endif // _PRE_PLAT_FEATURE_CUSTOMIZE

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif