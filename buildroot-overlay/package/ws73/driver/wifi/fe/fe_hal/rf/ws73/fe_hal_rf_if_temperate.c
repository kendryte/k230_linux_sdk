/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 温度补偿用到的rf接口
 */
#include "fe_hal_rf_if_temperate.h"
#include "hal_rf_reg.h"
#include "fe_hal_rf_reg_if_temperate.h"
#include "hal_reg_opt.h"
#include "wlan_types.h"
#include "soc_tsensor.h"
#include "oal_types.h"
#include "oal_util_hcm.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define TEMP_CODE_THRESHOLD_FOR_REFRESHING_COMP (1)   // 刷新温度码配置的温度码偏差阈值
#define TEMPERATURE_THRESHOLD_FOR_REFRESHING_COMP (2) // 刷新温度码配置的温度偏差阈值
#define TEMP_CODE_COUNT (8)                           // 温度码个数
#define TEMP_CODE_COMP_REG_NUM (8)                    // 温度码补偿寄存器的个数
osal_u8 g_temp_code_curr = 3;   // 当前温度码值 默认处于20-40
osal_u8 fe_hal_get_curr_temp_code(osal_void)
{
    return g_temp_code_curr;
}
// 分隔温度码的边界温度 e.g: 低于-20是0档 大于等于-20但是低于0是1档
const osal_s16 g_temp_separation_point[TEMP_CODE_COUNT - 1] = {
    -20, 0, 20, 40, 60, 80, 100
};
const osal_u16 g_temp_comp_values[TEMP_CODE_COUNT][TEMP_CODE_COMP_REG_NUM] = {
    { 0x01C7, 0x61C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
    { 0x01C7, 0x61C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
    { 0x01C7, 0x61C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
    { 0x01C7, 0x61C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
    { 0x01C7, 0x61C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
    { 0x01C7, 0x71C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
    { 0x01C7, 0x81C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
    { 0x01C7, 0x91C7, 0x91C7, 0x71C7, 0x2108, 0x2108, 0x2908, 0x0AB6 },
};

// 是否需要更新温度码的配置 防止温度来回变动的乒乓校验
osal_bool fe_hal_rf_need_config_temp_code(osal_u8 old_code, osal_u8 new_code, osal_s16 cur_temp)
{
    // 异常分支
    if (old_code >= TEMP_CODE_COUNT || new_code >= TEMP_CODE_COUNT) {
        return OSAL_FALSE;
    }
    // 温度码偏差大于1 需要更新
    if (get_abs(old_code - new_code) > TEMP_CODE_THRESHOLD_FOR_REFRESHING_COMP) {
        return OSAL_TRUE;
    }
    // 温度码偏差是1 当前温度相较边界温度偏差大于2 需要更新
    if (old_code > new_code) {
        return (get_abs(cur_temp - g_temp_separation_point[new_code]) > TEMPERATURE_THRESHOLD_FOR_REFRESHING_COMP);
    } else if (new_code > old_code) {
        return (get_abs(cur_temp - g_temp_separation_point[old_code]) > TEMPERATURE_THRESHOLD_FOR_REFRESHING_COMP);
    } else {
        return OSAL_FALSE;
    }
}
// 温度码获取接口
osal_u8 fe_hal_rf_get_temp_code_by_temperate(osal_s16 temperature)
{
    osal_u8 i = 0;
    for (i = 0; i < TEMP_CODE_COUNT - 1; i++) {
        if (temperature < g_temp_separation_point[i]) {
            break;
        }
    }
    return i;
}

/*
 * 将ipa_current校准得到数值经过换算后,写入到d_wb_rf_tx_pa_vb_sel_tb_lut
 * wbtrx_rw_reg_115[4:0]
 * wbtrx_rw_reg_115[9:5]
 * wbtrx_rw_reg_115[14:10]
 * wbtrx_rw_reg_116[4:0]
 * wbtrx_rw_reg_116[9:5]
 * wbtrx_rw_reg_116[14:10]
 * wbtrx_rw_reg_117[4:0]
 * wbtrx_rw_reg_117[9:5]
 */
osal_u16 fe_hal_get_temp_code_value(osal_u8 temp_code, osal_u8 reg_index, osal_bool is_init)
{
    osal_u32 temp_code_value = g_temp_comp_values[temp_code][reg_index];
    u_rf_abb80_union reg80_val = {0};
    osal_u8 bias_comp = 0;
    reg80_val.u16 = hal_reg_read16(HH503_RF_ABB_REG_BASE_0x140);
    bias_comp = reg80_val.bits.xFwTjw_NwLPwuGwqjwCWowSGhdGo_;
    if (is_init == OSAL_TRUE) {
        return (osal_u16)temp_code_value;
    }
    // 低温下优化电流
    if (temp_code < TEMP_CODE_LOW_TEMP_INDEX) {
        bias_comp += 1;
    }
    if ((reg_index == 0x4) || (reg_index == 0x5)) {  // 对应寄存器d115,d116 or d123,d124
        u_rf_abb115_union val = {{bias_comp, bias_comp, bias_comp, 0}};
        temp_code_value = val.u16;
    }
    if (reg_index == 0x6) {  // 对应寄存器d117 or d125
        u_rf_abb117_union val = {0};
        val.u16 = temp_code_value;
        val.bits.xFwTjw_NwLPwuGwqjwCWowLjwyyy00ywodL_ = bias_comp;
        val.bits.x4OqyOwtOR8OGzOPyOsmxORyO333rrrOxSR_ = bias_comp;
        temp_code_value = val.u16;
    }
    return (osal_u16)temp_code_value;
}

// 温度码配置
osal_void fe_hal_rf_config_temp_code_value(osal_u8 rf_id, wlan_channel_band_enum_uint8 band, osal_u8 temp_code,
    osal_u8 bank_sel, osal_bool is_init)
{
    osal_u8 i;
    osal_u32 rf_temp_bank_addr[TEMP_CODE_COMP_BANK_SEL_BUTT][TEMP_CODE_COMP_REG_NUM] = {
        {HH503_RF_ABB_REG_BASE_0x1BC, HH503_RF_ABB_REG_BASE_0x1C0,
         HH503_RF_ABB_REG_BASE_0x1C4, HH503_RF_ABB_REG_BASE_0x1C8,
         HH503_RF_ABB_REG_BASE_0x1CC, HH503_RF_ABB_REG_BASE_0x1D0,
         HH503_RF_ABB_REG_BASE_0x1D4, HH503_RF_ABB_REG_BASE_0x1D8},
        {HH503_RF_ABB_REG_BASE_0x1DC, HH503_RF_ABB_REG_BASE_0x1E0,
         HH503_RF_ABB_REG_BASE_0x1E4, HH503_RF_ABB_REG_BASE_0x1E8,
         HH503_RF_ABB_REG_BASE_0x1EC, HH503_RF_ABB_REG_BASE_0x1F0,
         HH503_RF_ABB_REG_BASE_0x1F4, HH503_RF_ABB_REG_BASE_0x1F8}
    };
    unref_param(rf_id);
    unref_param(band);

    if (temp_code >= TEMP_CODE_COUNT) {
        return;
    }
    g_temp_code_curr = temp_code;   // 刷新温度码配置时 同步刷新温度码
    // 根据当前温度码的配置刷新到寄存器中
    for (i = 0; i < TEMP_CODE_COMP_REG_NUM; i++) {
        hal_reg_write16(rf_temp_bank_addr[bank_sel][i], fe_hal_get_temp_code_value(temp_code, i, is_init));
    }
}
osal_u32 fe_hal_rf_read_temperature(wlan_channel_band_enum_uint8 band, osal_s16 *cur_temp)
{
    unref_param(band);
    return uapi_tsensor_read_temperature(cur_temp);
}
osal_u8 fe_hal_rf_get_rf_temp_code_bank_sel(osal_u8 rf_id, wlan_channel_band_enum_uint8 band)
{
    unref_param(rf_id);
    unref_param(band);
    return (hal_rf_get_abb127_d_wb_rf_temp_code_bank_sel() == TEMP_CODE_COMP_BANK_SEL_0) ?
        TEMP_CODE_COMP_BANK_SEL_1 : TEMP_CODE_COMP_BANK_SEL_0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
