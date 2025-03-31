/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy功能接口
 */
#include "fe_hal_phy_if_host.h"
#include "fe_hal_phy_reg_if_host.h"
#include "hal_phy.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
osal_void fe_hal_phy_set_rf_temp_bank_sel(const hal_device_stru *device, osal_u8 rf_id, osal_u8 bank_sel)
{
    const osal_u32 rf_temp_bank_sel_addr_offset = 0x7F;
    unref_param(device);
    unref_param(rf_id);

    hal_set_cfg_reserv_1_reg_2_reserv_1_wr_value_0ch(bank_sel);
    hal_set_cfg_reserv_1_reg_1_reserv_1_wr_addr(rf_temp_bank_sel_addr_offset);
    hal_set_cfg_reserv_1_reg_1_cfg_reserv_1_wr_en(1);
}

osal_void fe_hal_phy_set_cta_coef(hal_device_stru *device, osal_u8 ch_idx,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    const common_cfg_reg_stru reg_20m_tbl[CALI_COEF_FLATNESS_20M_REG_NUM] = {
        {HH503_PHY_BANK5_BASE_0xD4,      0x07fa03fd, 0x000003F8}, // 20M滤波系数
        {HH503_PHY_BANK5_BASE_0xD8,      0x0012000c, 0x00000013},
        {HH503_PHY_BANK5_BASE_0xDC,      0x07e003d9, 0x000003D2},
        {HH503_PHY_BANK5_BASE_0xE0,      0x002e009e, 0x000000A0},
        {HH503_PHY_BANK5_BASE_0xE4,      0x000000e7, 0x00000100},
        {HH503_PHY_BANK5_BASE_0x30, 0x007f6870, 0x00506870}, // 11ax flatness补偿系数
        {HH503_PHY_BANK5_BASE_0x50,  0x2DB6DB6D, 0x3FFFFFFF},
        {HH503_PHY_BANK5_BASE_0x54,  0x36DB5B6D, 0x0003FFFF},
        {HH503_PHY_BANK5_BASE_0x58,  0x07FFFFFF, 0x00000000},
        {HH503_PHY_BANK5_BASE_0x5C,  0x00000000, 0x0000003F},
        {HH503_PHY_BANK5_BASE_0x60,  0x00000000, 0x00000000},
    };
    const common_cfg_reg_stru reg_40m_tbl[CALI_COEF_FLATNESS_40M_REG_NUM] = {
        {HH503_PHY_BANK5_BASE_0xC0,      0x07fa03fd, 0x000003F8}, // 40M滤波系数
        {HH503_PHY_BANK5_BASE_0xC4,      0x0012000c, 0x00000013},
        {HH503_PHY_BANK5_BASE_0xC8,      0x07e003d9, 0x000003D2},
        {HH503_PHY_BANK5_BASE_0xCC,      0x002e009e, 0x000000A0},
        {HH503_PHY_BANK5_BASE_0xD0,      0x000000e7, 0x00000100},
        {HH503_PHY_BANK1_BASE_0xDC,    0x20F7358B, 0x1294A508}, // 40m flatness补偿系数
        {HH503_PHY_BANK1_BASE_0xE0,    0x14A52529, 0x10842108},
        {HH503_PHY_BANK1_BASE_0xE4,    0x00042108, 0x00042108},
    };
    common_cfg_reg_stru *reg_tbl =
        (common_cfg_reg_stru *)((bandwidth == WLAN_BAND_WIDTH_20M) ? reg_20m_tbl : reg_40m_tbl);
    osal_u8 high_ch_idx = (bandwidth == WLAN_BAND_WIDTH_20M) ? CALI_CHANNEL_NUM13_IDX : CALI_CHANNEL_NUM9_IDX;
    osal_u8 reg_num =
        (bandwidth == WLAN_BAND_WIDTH_20M) ? CALI_COEF_FLATNESS_20M_REG_NUM : CALI_COEF_FLATNESS_40M_REG_NUM;
    osal_u32 reg_val;
    osal_u8 index = 0;
    unref_param(device);

    for (index = 0; index < reg_num; index++) {
        reg_val = (ch_idx >= high_ch_idx) ? reg_tbl[index].cfg_val : reg_tbl[index].recover_val;
        hal_reg_write(reg_tbl[index].reg_addr, reg_val);
    }
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
