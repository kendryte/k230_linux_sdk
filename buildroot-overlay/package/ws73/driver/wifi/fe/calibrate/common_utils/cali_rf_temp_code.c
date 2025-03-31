/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 温度码配置相关接口
 */
#include "cali_rf_temp_code.h"
#include "fe_hal_rf_if_temperate.h"
#include "fe_hal_phy_if_host.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define TEMP_CODE_COMP_TIMER_PERIOD (1000)  // 刷新温度配置的定时器1s
#define TEMP_CODE_BUTT (0xFF)  // 无效温度码
// 温度码配置更新
osal_void rf_temp_code_config_comp_value(hal_to_dmac_device_stru *device,
    wlan_channel_band_enum_uint8 band, osal_bool is_init)
{
    osal_s16 cur_temp = 0;
    osal_u32 ret;
    osal_u8 cur_temp_code;
    static osal_u8 old_temp_code = TEMP_CODE_BUTT; // 设置初始温度码为无效值
    osal_u8 bank_sel;
    hal_device_stru *hal_device = (hal_device_stru *)device;
    osal_u8 rf_id;

    ret = fe_hal_rf_read_temperature(band, &cur_temp);
    if (ret != OAL_SUCC) {
        return;
    }
    cur_temp_code = fe_hal_rf_get_temp_code_by_temperate(cur_temp);
    // 对于非初始化情况 判断是否需要刷新温度码补偿配置 初次进入先刷新一遍 保证使用ate校准的ipa值
    if ((is_init == OSAL_FALSE) && (old_temp_code != TEMP_CODE_BUTT) &&
        (fe_hal_rf_need_config_temp_code(old_temp_code, cur_temp_code, cur_temp) == OSAL_FALSE)) {
        return;
    }
    for (rf_id = 0; rf_id < WLAN_RF_CHANNEL_NUMS; rf_id++) {
        bank_sel = fe_hal_rf_get_rf_temp_code_bank_sel(rf_id, band);
        fe_hal_rf_config_temp_code_value(rf_id, band, cur_temp_code, bank_sel, is_init);
        fe_hal_phy_set_rf_temp_bank_sel(hal_device, rf_id, bank_sel);
    }
    // 非初始化时更新温度码 避免ipa校准值没有更新
    if (is_init == OSAL_FALSE) {
        old_temp_code = cur_temp_code;
    }
}
// 定时处理温度配置补偿
OAL_STATIC osal_u32 rf_temp_code_config_timeout_handler(osal_void *arg)
{
    hal_to_dmac_device_stru *device = arg;
    wlan_channel_band_enum_uint8 band;

    if (arg == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    band = device->wifi_channel_status.band;
    rf_temp_code_config_comp_value(device, band, OSAL_FALSE);
    return OAL_SUCC;
}
osal_void rf_temp_code_create_config_timer(hal_to_dmac_device_stru *device)
{
    static frw_timeout_stru temp_code_timer = {0};

    frw_create_timer_entry(&temp_code_timer,
        rf_temp_code_config_timeout_handler, TEMP_CODE_COMP_TIMER_PERIOD, device, OSAL_TRUE);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif