/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: hal alg func.
 * Create: 2022-1-12
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_alg_tpc.h"
#include "hal_ext_if.h"
#include "wlan_spec.h"
#include "hal_mac_reg.h"
#include "mac_frame.h"
#include "hmac_ext_if.h"
#include "hal_mac.h"
#include "hal_power.h"
#include "fe_tpc_rate_pow.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_ALG_TPC_WS73_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define RTS_TPC_REG_COUNT     7 /* RTS_AUTORATE寄存器5~11，共7个寄存器,1、2、12单独进行配置 */
#define RTS_TPC_ONE_REG_NUM   2 /* 每个寄存器可以配置两个功率值 共需配置RTS速率档位*3个功率值 */
#define RTS_TPC_ONE_REG_PARAM 2 /* 每个寄存器的每个功率值对应两个参数：距离 速率 */
static osal_u8 g_rts_tpc_table[RTS_TPC_REG_COUNT][RTS_TPC_ONE_REG_NUM][RTS_TPC_ONE_REG_PARAM] = {
    /* {high_bits, low_bits} = {{distance rts_rate_rank}, {distance rts_rate_rank}}
       distance:near=0 normal=1 far=2 rts_rate_rank:0~5 暂时只有6档RTS速率                 */
    {{0, 1}, {1, 1}}, {{2, 1}, {0, 2}}, {{1, 2}, {2, 2}}, {{0, 3}, {1, 3}},
    {{2, 3}, {0, 4}}, {{1, 4}, {2, 4}}, {{0, 5}, {1, 5}}
};
static osal_u32 g_rts_autorate_addr[] = {HH503_MAC_CTRL1_BANK_BASE_0x14C, HH503_MAC_CTRL1_BANK_BASE_0x150, HH503_MAC_CTRL1_BANK_BASE_0x154,
    HH503_MAC_CTRL1_BANK_BASE_0x158, HH503_MAC_CTRL1_BANK_BASE_0x15C, HH503_MAC_CTRL1_BANK_BASE_0x160, HH503_MAC_CTRL1_BANK_BASE_0x164};
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TPC
/*****************************************************************************
功能描述  : 设置action no ack的功率
*****************************************************************************/
osal_void hh503_set_tpc_bf_pow(hal_to_dmac_device_stru *hal_device)
{
    osal_u32 pow_code, tx_phy_mode, tx_data_rate;
    osal_u8 pow_level, rate;
    hal_alg_user_distance_enum_uint8 distance;
    tpc_code_params_stru *tpc_code = OSAL_NULL;

    /* 入参检查 */
    if (osal_unlikely(alg_is_null_ptr1(hal_device))) {
        oam_warning_log0(0, OAM_SF_TPC, "alg_tpc_adjust_bf_report::null pointers");
        return;
    }

    distance = hal_device->hal_alg_stat.alg_distance_stat;

    pow_level = ((distance == HAL_ALG_USER_DISTANCE_NEAR) ? HAL_POW_MIN_POW_LEVEL :
        ((distance == HAL_ALG_USER_DISTANCE_NORMAL) ? HAL_POW_MAX_POW_LEVEL : HAL_POW_RF_LIMIT_POW_LEVEL));

    tx_phy_mode = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x138);
    tx_data_rate = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x58);

    rate = ((hh503_tx_phy_rate_stru *)(&tx_data_rate))->bits.xLPwqW6Li_wS6Cw_GLW_;
    pow_code = fe_tpc_rate_pow_get_rate_tpc_code(hal_device->wifi_channel_status.band, WLAN_HE_SU_FORMAT,
        rate, WLAN_BAND_ASSEMBLE_20M, pow_level);
    tpc_code = (tpc_code_params_stru *)(&pow_code);

    ((hh503_tx_phy_mode_reg_stru *)(&tx_phy_mode))->xLPwqW6Li_wLu6w6Qy_ = tpc_code->tpc_ch0;
    ((hh503_tx_phy_rate_stru *)(&tx_data_rate))->bits.xR8OPmCRvwORGCO4G4OxPO3_ = tpc_code->dpd_tpc_lv_ch0;
    ((hh503_tx_phy_rate_stru *)(&tx_data_rate))->bits.xR8OPmCRvwOCtwOl48_ = tpc_code->cfr_idx;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x138, tx_phy_mode);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x58, tx_data_rate);
}

/*****************************************************************************
功能描述  : 设置RTS的功率
*****************************************************************************/
static osal_void hh503_set_tpc_rts_autorate_1_2(hal_to_dmac_device_stru *hal_device, osal_u8 rate)
{
    osal_u32 pow_code;
    osal_u8 pow_level;
    tpc_code_params_stru *tpc_code = OSAL_NULL;

    u_rts_autorate1 rts_autorate1;
    u_rts_autorate2 rts_autorate2;

    rts_autorate1.u32 = 0x0;
    rts_autorate2.u32 = 0x0;
    /* 配置rts速率档位0功率 */
    /* RTS_AUTORATE1 保护帧速率功率选择模式 rts速率档位0 near */
    pow_level = HAL_POW_MIN_POW_LEVEL; /* near */
    pow_code = fe_tpc_rate_pow_get_rate_tpc_code(hal_device->wifi_channel_status.band,
        WLAN_11B_PHY_PROTOCOL_MODE, rate, WLAN_BAND_ASSEMBLE_20M, pow_level);
    tpc_code = (tpc_code_params_stru *)(&pow_code);
    rts_autorate1.bits.x6Ngw_LCwuiTW_whWG_wqGowLu6w6Q_ = tpc_code->tpc_ch0;
    rts_autorate1.bits.xCt2OwRsOGvqmwOomzwOPzxO4G4ORGCOxP_ = tpc_code->dpd_tpc_lv_ch0;
    rts_autorate1.bits.x6Ngw_LCwuiTW_whWG_wqGow6N_wBFP_ = tpc_code->cfr_idx;
    rts_autorate1.bits.x6Ngw_LCwLu6w_GLWwCWLwSiFW_ = 0; /* 0：新增Autorate模式 1：原始Autorate模式 */
    /* RTS_AUTORATE2 rts速率档位0 normal far */
    pow_level = HAL_POW_MAX_POW_LEVEL; /* normal */
    pow_code = fe_tpc_rate_pow_get_rate_tpc_code(hal_device->wifi_channel_status.band,
        WLAN_11B_PHY_PROTOCOL_MODE, rate, WLAN_BAND_ASSEMBLE_20M, pow_level);
    tpc_code = (tpc_code_params_stru *)(&pow_code);
    rts_autorate2.bits.xCt2OwRsOGvqmwOovwIzxOPzxORGCOCA_ = tpc_code->tpc_ch0;
    rts_autorate2.bits.x6Ngw_LCwuiTW_whi_SGowqGowFuFwLu6woq_ = tpc_code->dpd_tpc_lv_ch0;
    rts_autorate2.bits.xCt2OwRsOGvqmwOovwIzxOPzxOCtwOl48_ = tpc_code->cfr_idx;
    pow_level = HAL_POW_RF_LIMIT_POW_LEVEL; /* far */
    pow_code = fe_tpc_rate_pow_get_rate_tpc_code(hal_device->wifi_channel_status.band,
        WLAN_11B_PHY_PROTOCOL_MODE, rate, WLAN_BAND_ASSEMBLE_20M, pow_level);
    tpc_code = (tpc_code_params_stru *)(&pow_code);
    rts_autorate2.bits.x6Ngw_LCwuiTW_wNG_wqGowLu6w6Q_ = tpc_code->tpc_ch0;
    rts_autorate2.bits.xCt2OwRsOGvqmwOtzwOPzxO4G4ORGCOxP_ = tpc_code->dpd_tpc_lv_ch0;
    rts_autorate2.bits.x6Ngw_LCwuiTW_wNG_wqGow6N_wBFP_ = tpc_code->cfr_idx;

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x13C, rts_autorate1.u32);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x140, rts_autorate2.u32);
}

static osal_void hh503_set_tpc_rts_autorate(hal_to_dmac_device_stru *hal_device, u_rts_autorate3 rts_autorate3,
    u_rts_autorate4 rts_autorate4)
{
    osal_u8 rate[] = {rts_autorate3.bits.x6Ngw_LCw_GLWw_GhKwqGoywS6C_, rts_autorate3.bits.xCt2OwRsOwzRmOwzodOPzxrOICs_,
        rts_autorate3.bits.x6Ngw_LCw_GLWw_GhKwqGoVwS6C_, rts_autorate3.bits.x6Ngw_LCw_GLWw_GhKwqGokwS6C_,
        rts_autorate4.bits.xCt2OwRsOwzRmOwzodOPzxKOICs_, rts_autorate4.bits.x6Ngw_LCw_GLWw_GhKwqGoxwS6C_};
    osal_u8 reg_index;
    osal_u8 bits_index;
    osal_u8 rate_index;
    osal_u32 pow_code;
    osal_u8 pow_level;
    tpc_code_params_stru *tpc_code = OSAL_NULL;
    u_rts_autorate  rts_autorate;
    osal_u32 reg_bits_offset;

    for (reg_index = 0; reg_index < RTS_TPC_REG_COUNT; reg_index++) {
        rts_autorate.u32 = 0x0;
        for (bits_index = 0; bits_index < RTS_TPC_ONE_REG_NUM; bits_index++) {
            if (bits_index == 0) {
                reg_bits_offset = 16; /* high bits offset: 16 */
            } else {
                reg_bits_offset = 0; /* low bits offset: 0   */
            }
            rate_index = rate[g_rts_tpc_table[reg_index][bits_index][1]];
            pow_level = ((g_rts_tpc_table[reg_index][bits_index][0] == HAL_ALG_USER_DISTANCE_NEAR) ?
                HAL_POW_MIN_POW_LEVEL :((g_rts_tpc_table[reg_index][bits_index][0] == HAL_ALG_USER_DISTANCE_NORMAL) ?
                HAL_POW_MAX_POW_LEVEL : HAL_POW_RF_LIMIT_POW_LEVEL));
            pow_code = fe_tpc_rate_pow_get_rate_tpc_code(hal_device->wifi_channel_status.band,
                WLAN_11B_PHY_PROTOCOL_MODE, rate_index, WLAN_BAND_ASSEMBLE_20M, pow_level);
            tpc_code = (tpc_code_params_stru *)(&pow_code);
            rts_autorate.u32 |= (tpc_code->tpc_ch0 << (reg_bits_offset));            /* bit: 0~7 16~23   */
            rts_autorate.u32 |= (tpc_code->dpd_tpc_lv_ch0 << (reg_bits_offset + 8)); /* bit: 8~9 24~25   */
            rts_autorate.u32 |= (tpc_code->cfr_idx << (reg_bits_offset + 10));       /* bit: 10~11 26~27 */
        }
        hal_reg_write(g_rts_autorate_addr[reg_index], rts_autorate.u32);
    }
}

osal_void hh503_set_tpc_rts_pow(osal_void)
{
    osal_u8 rate;
    osal_u32 pow_code;
    osal_u8 pow_level;
    osal_u8 rate_pow_idx = HAL_POW_2G_1MBPS_RATE_POW_IDX; /* 速率初始化为2g 1M，避免查找不到速率索引 */
    tpc_code_params_stru *tpc_code = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    u_rts_autorate3 rts_autorate3;
    u_rts_autorate4 rts_autorate4;
    u_rts_autorate  rts_autorate;

    /* 获取hal_device */
    hal_device = hal_chip_get_hal_device();
    /* 入参检查 */
    if (osal_unlikely(alg_is_null_ptr1(hal_device))) {
        oam_warning_log0(0, OAM_SF_TPC, "alg_tpc_adjust_rts_report::null pointers");
        return;
    }

    /* 获得RTS速率 */
    rts_autorate3.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x144);
    rts_autorate4.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x148);

    /* 配置RTS_AUTORATE1/RTS_AUTORATE2 (rts_rate_rank0) */
    rate = rts_autorate3.bits.x6Ngw_LCw_GLWw_GhKwqGoywS6C_;
    hh503_set_tpc_rts_autorate_1_2(hal_device, rate);

    /* 配置RTS_AUTORATE5~RTS_AUTORATE11 */
    hh503_set_tpc_rts_autorate(hal_device, rts_autorate3, rts_autorate4);

    /* 配置RTS_AUTORATE12 由于寄存器12配置包含速率6(暂不支持)，因此单独配置 */
    rate = rts_autorate4.bits.x6Ngw_LCw_GLWw_GhKwqGoxwS6C_;
    rts_autorate.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x168);
    pow_level = HAL_POW_RF_LIMIT_POW_LEVEL; /* far */
    pow_code = fe_tpc_rate_pow_get_rate_tpc_code(hal_device->wifi_channel_status.band,
        WLAN_11B_PHY_PROTOCOL_MODE, rate_pow_idx, WLAN_BAND_ASSEMBLE_20M, pow_level);
    tpc_code = (tpc_code_params_stru *)(&pow_code);
    rts_autorate.bits.xCt2OwRsOGvqmwOAl2AOPzxORGCOCA_ = tpc_code->tpc_ch0;
    rts_autorate.bits.x6Ngw_LCwuiTW_wQBgQwqGowFuFwLu6woq_ = tpc_code->dpd_tpc_lv_ch0;
    rts_autorate.bits.xCt2OwRsOGvqmwOAl2AOPzxOCtwOl48_ = tpc_code->cfr_idx;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x168, rts_autorate.u32);
}
#endif /* end of _PRE_WLAN_FEATURE_TPC */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
