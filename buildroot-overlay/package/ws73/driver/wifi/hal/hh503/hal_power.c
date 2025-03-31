/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Interface of power settings for hal layer.
 * Create: 2020-7-3
 */
#include "hal_power.h"
#include "hal_mac.h"
#include "fe_tpc_rate_pow.h"
#include "fe_rf_customize_power.h"
#include "fe_hal_phy_if_host.h"
#ifdef _PRE_WLAN_FEATURE_TPC
#include "alg_tpc.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WIFI_HOST_HAL_POWER_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
osal_u32 g_pow_reg_tbl[WLAN_DISTANCE_BUTT][HAL_RESP_POWER_REG_NUM] = {
    {
        HH503_MAC_CTRL1_BANK_BASE_0xD4,
        HH503_MAC_CTRL1_BANK_BASE_0xE0,
    },
    {
        HH503_MAC_CTRL1_BANK_BASE_0xCC,
        HH503_MAC_CTRL1_BANK_BASE_0xD8,
    },
    {
        HH503_MAC_CTRL1_BANK_BASE_0xD0,
        HH503_MAC_CTRL1_BANK_BASE_0xDC,
    }
};
osal_s16 g_pow_tpc_level_table[HAL_POW_LEVEL_NUM][WLAN_BAND_BUTT] = {
    { 0,  0  },   /* Level 0      0dB */
    { -4,  -4  },   /* Level 1      -3dB */
    { -7,  -7  },   /* Level 2      -6dB */
    { -13, -13 },   /* Level 3      -12dB */
    { -13, -16 },   /* Level 4  very nearby  -12dB/-15dB */
    { 3,   3   },   /* Level 5   rf limit     0dB */
};
osal_u8 g_pow_under_regulation = OSAL_TRUE;
// 记录历史偏移 偏移是固定值 不做叠加 对应 g_fe_tpc_rate_pow_code_table_2g
osal_s8 g_tpc_rate_power_offset[WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN] = {0};
OSAL_STATIC oal_bool_enum_uint8 hal_power_get_under_regulation(osal_void);

osal_u16 hh503_pow_get_pow_table_tpc_code(osal_u8 band, osal_u8 rate, osal_u8 bw, osal_u8 pwr_lvl)
{
    hal_phy_rate_stru *hal_rate = (hal_phy_rate_stru *)&rate;
    osal_u8 protocol_mode = hal_rate->bits.protocol_mode;
    osal_u8 mcs = hal_rate->bits.xICsOwzRm_;
    osal_u16 tpc_code;

    tpc_code = fe_tpc_rate_pow_get_rate_tpc_code(band, protocol_mode, mcs, bw, pwr_lvl);
    return tpc_code;
}

/*****************************************************************************
 函 数 名  : hh503_pow_set_resp_tpc_code_to_pow_mode
 功能描述  : 设置resp tpc code to pow
*****************************************************************************/
osal_void hh503_pow_set_resp_tpc_code_to_pow_mode(hh503_tx_resp_phy_mode_reg_stru *pow_code,
    const tpc_code_params_stru *tpc_code)
{
    pow_code->tpc_ch0 = tpc_code->tpc_ch0;
    pow_code->dpd_tpc_lv_ch0 = tpc_code->dpd_tpc_lv_ch0;
    pow_code->cfr_idx = tpc_code->cfr_idx;
}

/*****************************************************************************
 函 数 名  : hh503_set_resp_frm_phy_tx_mode
 功能描述  : 设置resp帧寄存器
*****************************************************************************/
OSAL_STATIC osal_void hh503_set_resp_frm_phy_tx_mode(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u8 subband_idx, wlan_user_distance_enum_uint8 distance_idx)
{
    osal_u32 index;
    osal_u32 pow_code[HAL_RESP_POWER_REG_NUM];
    osal_u32 tpc_code[HAL_RESP_POWER_REG_NUM];
    osal_u32 i;
    osal_u32 subband_index = subband_idx;

    if (distance_idx >= WLAN_DISTANCE_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "{hh503_set_resp_frm_phy_tx_mode: distance_idx[%d] invalid!}", subband_index);
        return;
    }

    if (band == WLAN_BAND_2G) {
        index = 0;
        tpc_code[0] = hal_device->phy_pow_param.aul_2g_dsss_ack_cts_pow_code[subband_index];
    } else {
        index = 1;
        tpc_code[0] = 0; // not used in 5g mode
        subband_index += WLAN_2G_SUB_BAND_NUM;
    }

    for (i = 0; i < WLAN_OFDM_ACK_CTS_TYPE_BUTT; i++) {
        tpc_code[i + 1] = hal_device->phy_pow_param.ack_cts_pow_code[i][subband_index];
    }
    for (; index < HAL_RESP_POWER_REG_NUM; index++) {
        hh503_pow_set_resp_tpc_code_to_pow_mode((hh503_tx_resp_phy_mode_reg_stru *)&pow_code[index],
            (tpc_code_params_stru *)&tpc_code[index]);
        hal_reg_write(g_pow_reg_tbl[distance_idx][index], pow_code[index]);
    }
}

/*****************************************************************************
 功能描述 : 设置ack cts的功率
*****************************************************************************/
static osal_void hh503_set_ack_cts_pow_code(hal_to_dmac_device_stru *hal_device, wlan_channel_band_enum_uint8 band,
    osal_u8 subband_idx, wlan_user_distance_enum_uint8 distance_idx)
{
    osal_u8 pow_level_idx[WLAN_DISTANCE_BUTT] = {
        HAL_POW_MIN_POW_LEVEL, HAL_POW_MAX_POW_LEVEL, HAL_POW_RF_LIMIT_POW_LEVEL
    };
    osal_u8 pow_level_idx0;
    osal_u8 ack_cts_type_idx;
    osal_u8 rate_code;

    /* 获取该距离下功率等级 */
    pow_level_idx0 = pow_level_idx[distance_idx];

    if (band == WLAN_BAND_2G) {
        /* 设置CTS/ACK功率,由于SIFS帧速率可变，功率只能够按照典型速率配置，而不按实际速率配置 */
        hal_device->phy_pow_param.aul_2g_dsss_ack_cts_pow_code[subband_idx] =
            fe_tpc_rate_pow_get_rate_tpc_code(band, WLAN_11B_PHY_PROTOCOL_MODE,
            HAL_POW_2G_1MBPS_RATE_POW_IDX, WLAN_BAND_ASSEMBLE_20M, pow_level_idx0);
    } else {
        subband_idx += WLAN_2G_SUB_BAND_NUM;
    }

    for (ack_cts_type_idx = WLAN_OFDM_ACK_CTS_TYPE_24M; ack_cts_type_idx < WLAN_OFDM_ACK_CTS_TYPE_BUTT;
        ack_cts_type_idx++) {
        rate_code = hal_device->phy_pow_param.rate_ofdm_ack_cts[ack_cts_type_idx];
        if (rate_code > 0) {
            hal_device->phy_pow_param.ack_cts_pow_code[ack_cts_type_idx][subband_idx] =
                hal_pow_get_pow_table_tpc_code(band, rate_code, WLAN_BAND_ASSEMBLE_20M, pow_level_idx0);
        }
    }
}

/*****************************************************************************
 函 数 名  : hal_pow_set_resp_frame_tx_power
 功能描述  : 设置resp帧的发射功率(三级),如果速率不可能为0的话，这里的大于0判断可以优化掉
*****************************************************************************/
osal_void hal_pow_set_resp_frame_tx_power(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u8 chan_idx)
{
    osal_u8 subband_idx = 0;
    wlan_user_distance_enum_uint8 distance_idx;

    if (hh503_get_subband_index(band, chan_idx, &subband_idx) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_PWR, "{hal_pow_set_resp_frame_tx_power::band[%d]chan_idx[%d] is invalid}",
            band, chan_idx);
        return;
    }

    /* 读取不同帧的速率 */
    hh503_get_spec_frm_rate(hal_device);

    for (distance_idx = WLAN_DISTANCE_NEAR; distance_idx < WLAN_DISTANCE_BUTT; distance_idx++) {
        hh503_set_ack_cts_pow_code(hal_device, band, subband_idx, distance_idx);
        /* 填写寄存器 */
        hh503_set_resp_frm_phy_tx_mode(hal_device, band, subband_idx, distance_idx);
    }
}

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER

typedef struct {
    osal_u32 resp_power_l : 8;
    osal_u32 resp_power_m : 16;
    osal_u32 reserved : 8;
} cfg_resp_power_lut_11b_val;

/*****************************************************************************
 功能描述  : 清除一个用户表项到resp power lut表
*****************************************************************************/
osal_void hh503_pow_del_machw_resp_power_lut_entry(osal_u8 lut_index)
{
    u_peer_addr_lut_config peer_addr_lut_config = {.u32 = 0};
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = lut_index;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW__ = 2;   /* 设为2 */
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = 1;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = 1;
    /* 写RESP POWER Control寄存器 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8, peer_addr_lut_config.u32);
}

/*****************************************************************************
 功能描述  : 设置用户resp帧发送功率lut表
*****************************************************************************/
osal_void hh503_pow_set_user_resp_frame_tx_power(hal_to_dmac_device_stru *hal_device, osal_u8 lut_index,
    osal_u8 rssi_distance)
{
    unref_param(hal_device);
    unref_param(lut_index);
    unref_param(rssi_distance);
}
#endif

/*****************************************************************************
 函 数 名  : hal_pow_set_band_spec_frame_tx_power
 功能描述  : 设置该频带上的特殊帧的发射功率
*****************************************************************************/
osal_void hh503_pow_set_band_spec_frame_tx_power(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u8 chan_idx)
{
    osal_u8 subband_idx = 0;
    osal_u8 pow_level_idx;
    osal_u8 pow_code_idx;

    if (hh503_get_subband_index(band, chan_idx, &subband_idx) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_PWR, "{hh503_pow_set_band_spec_frame_tx_power::band[%d]chan_idx[%d] is invalid}",
            band, chan_idx);
        return;
    }

    /* 获取控制帧功率等级 */
    pow_level_idx = hal_device->control_frm_power_level;

    /* 读取不同帧的速率 */
    hh503_get_spec_frm_rate(hal_device);
    if (band == WLAN_BAND_5G) {
        /* 设置report帧功率， 一般以nonht发送 24M */
        hal_device->phy_pow_param.vht_report_pow_code[subband_idx] =
            hal_pow_get_pow_table_tpc_code(band, hal_device->phy_pow_param.rate_vht_report,
            WLAN_BAND_ASSEMBLE_20M, pow_level_idx);

        pow_code_idx = subband_idx + WLAN_2G_SUB_BAND_NUM;
    } else {
        pow_code_idx = subband_idx;
    }

    /* 设置RTS功率 6M, RTS速率可变，功率按照典型速率配置 */
    hal_device->phy_pow_param.rts_pow_code[pow_code_idx] =
            hal_pow_get_pow_table_tpc_code(band, hal_device->phy_pow_param.rate_rts,
            WLAN_BAND_ASSEMBLE_20M, pow_level_idx);

    /* 设置2.4G共存self-CTS帧功率 24M */
    /* 设置CF-END帧功率 24M */
    hal_device->phy_pow_param.cfend_pow_code[pow_code_idx] =
            hal_pow_get_pow_table_tpc_code(band, hal_device->phy_pow_param.rate_cfend,
            WLAN_BAND_ASSEMBLE_20M, pow_level_idx);

    /* 设置BAR帧功率 24M */
    hal_device->phy_pow_param.bar_pow_code[pow_code_idx] =
            hal_pow_get_pow_table_tpc_code(band, hal_device->phy_pow_param.rate_bar,
            WLAN_BAND_ASSEMBLE_20M, pow_level_idx);
    /* 填写寄存器 */
    hh503_set_spec_frm_phy_tx_mode(hal_device, band, subband_idx);
}

#ifdef _PRE_WLAN_FEATURE_11AX
/************************************ *****************************************
 功能描述  : 初始化HE TB PPDU MCS最大和最小发送功率
*****************************************************************************/
osal_void hh503_tb_mcs_tx_power(wlan_channel_band_enum_uint8 band)
{
    hh503_tb_mcs_tx_power_ext(band);
}

/************************************ *****************************************
 功能描述  : 初始化HE TB PPDU发送功率表
*****************************************************************************/
osal_void hh503_tb_tx_power_init(wlan_channel_band_enum_uint8 band)
{
    hh503_tb_tx_power_init_ext(band);
}
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */
/*****************************************************************************
 函 数 名  : hh503_pow_set_pow_to_pow_code
 功能描述  : 功率转化为对应的pow code
*****************************************************************************/
osal_void hh503_pow_set_pow_to_pow_code(hal_to_dmac_device_stru *hal_device, osal_u8 max_pow,
    const mac_channel_stru *channel)
{
    osal_s16 fcc_txpwr_limit;
    osal_s16 sar_txpwr_limit;
    osal_u8 rate_idx;
    osal_u8 pwr_idx;
    osal_u8 pow_code_table_len;
    osal_s16 tx_pow;
    osal_s16 target_pow;
    osal_u8 max_power = max_pow;
    osal_u8 max_power_tmp = max_pow;
    osal_u8 band = channel->band;
    osal_u16 tpc_code = 0;
    tpc_code_params_stru *tpc_code_params = (tpc_code_params_stru *)&tpc_code;
    // 不支持5G
    if (band != WLAN_BAND_2G) {
        return;
    }
    pow_code_table_len = WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN;
    sar_txpwr_limit =
        fe_custom_get_sar_power(band, 0) * FE_HAL_TPC_RATE_POW_LVL_VAL; // 默认使用0档sar功率 单位0.5dB转换到0.1dB

    max_power = (osal_u8)osal_min(sar_txpwr_limit, max_power);
    oam_info_log4(0, OAM_SF_CUSTOM, "{set_pow_to_pow_code_tpc_cali:ch_idx:%d sar:%d max_pow:%d table len:%d}",
        channel->chan_idx, sar_txpwr_limit, max_power, pow_code_table_len);
    for (rate_idx = 0; rate_idx < pow_code_table_len; rate_idx++) {
        /* limit power 0.5dB 转换到0.1dB */
        fcc_txpwr_limit = fe_custom_get_limit_power(band, rate_idx, channel->chan_idx) * FE_HAL_TPC_RATE_POW_LVL_VAL;

        max_power_tmp = osal_min(fcc_txpwr_limit, max_power);

        /* 获取定制化对应速率级最大发射功率 0.5dB 转换到0.1dB */
        target_pow = fe_hal_pow_get_custom_tpc_pow(band, rate_idx) * FE_HAL_TPC_RATE_POW_LVL_VAL;
        // 如果配置的service api限制功率有效 替换各速率最大功率 配置WLAN_MSG_W2H_CFG_TX_POWER
        if (hal_device->tx_pwr_ctrl.tx_power) {
            target_pow = hal_device->tx_pwr_ctrl.tx_power;
        }
        target_pow += hal_power_get_rate_power_offset(band, rate_idx);
        for (pwr_idx = HAL_POW_LEVEL_0; pwr_idx < HAL_POW_LEVEL_BUTT; pwr_idx++) {
            tx_pow = target_pow + (g_pow_tpc_level_table[pwr_idx][band] * HAL_POWER_CHANGE_COEF);
            if (pwr_idx != HAL_POW_RF_LIMIT_POW_LEVEL || hal_power_get_under_regulation() == OSAL_TRUE) {
                tx_pow = osal_min(max_power_tmp, tx_pow);
            }
            tpc_code_params->tpc_ch0 = fe_hal_pow_get_tpc_code_by_pow(band, tx_pow, OSAL_TRUE);
            fe_tpc_rate_pow_set_tpc_code_by_rate_idx(band, rate_idx, pwr_idx, tpc_code);
        }
    }
    oam_info_log4(0, OAM_SF_CUSTOM, "set_pow_to_pow_code::max_power_tmp:%d limit:%d tgt_pow:%d tpc_code:0x%x",
        max_power_tmp, fcc_txpwr_limit, target_pow, tpc_code);
}
/************************************ *****************************************
 函 数 名  : hh503_set_cta_coef
 功能描述  : 无委认证-特殊滤波系数和flatness系数
*****************************************************************************/
OSAL_STATIC osal_void hh503_set_cta_coef(hal_to_dmac_device_stru *hal_device, const mac_channel_stru *channel)
{
    if (fe_custom_get_cta_coef_flag(channel->band) == OAL_TRUE) {
        fe_hal_phy_set_cta_coef((hal_device_stru *)hal_device, channel->chan_idx, channel->en_bandwidth);
    }
}
/*****************************************************************************
 函 数 名  : hh503_pow_sw_initialize_tx_power
 功能描述  : 初始化tx power软件相关
*****************************************************************************/
osal_void hal_pow_sw_initialize_tx_power(hal_to_dmac_device_stru *hal_device)
{
    /* 初始化rate- pow code table表单 */
    (osal_void)memset_s(&hal_device->tx_pwr_ctrl, sizeof(hal_device_tx_pwr_ctrl_stru),
        0, sizeof(hal_device->tx_pwr_ctrl));
    hal_device->tx_pwr_ctrl.rate_pow_table_2g = OAL_PTR_NULL;
    hal_device->tx_pwr_ctrl.rate_pow_table_5g = OAL_PTR_NULL;
    /* 在vap初始化/刷新时做功率表刷新 这里不再刷新 */
    hh503_pow_sw_initialize_tx_power_ext(hal_device);
}

/*****************************************************************************
 函 数 名  : hh503_initialize_tx_power
 功能描述  : 初始化tx power
*****************************************************************************/
osal_void hh503_pow_initialize_tx_power(hal_to_dmac_device_stru *hal_device)
{
    oal_bool_enum_uint8 rf_linectl;
#ifdef BOARD_ASIC_WIFI
    osal_u32 reg_val;
#endif

    /* 初始化采用最大功率档位 */
    hal_device->fix_power_level  = HAL_POW_MAX_POW_LEVEL;
    hal_device->control_frm_power_level  = HAL_POW_MAX_POW_LEVEL;

    rf_linectl = ((WLAN_TPC_WORK_MODE_ENABLE == WLAN_TPC_WORK_MODE) ? OSAL_FALSE : OSAL_TRUE);

    /* 自适应功率模式打开，功率RF寄存器控不使能 */
    hal_device->pow_rf_reg_ctl_flag = rf_linectl;

#ifdef BOARD_FPGA_WIFI
    hal_reg_write(HH503_PHY_BANK3_BASE_0x2A0, 0x181300);
    /* 设置PA_MODE CODE */
    hh503_phy_set_cfg_pa_mode_code();
#endif

#ifdef BOARD_ASIC_WIFI
    reg_val = hal_reg_read(HH503_PHY_BANK3_BASE_0x2A0) & (~HH503_PHY_TPC_BYPASS_MASK);
    reg_val |= (rf_linectl << HH503_PHY_TPC_BYPASS_OFFSET);
    hal_reg_write(HH503_PHY_BANK3_BASE_0x2A0, reg_val);
#endif
}

osal_void hal_pow_sync_tpc_code_to_dmac(hal_to_dmac_device_stru *hal_device, osal_u8 vap_id)
{
    fe_hal_pow_sync_tpc_pow_table_to_dmac(hal_device, vap_id);
}
/************************************ *****************************************
 函 数 名  : hh503_device_init_vap_pow_code
 功能描述  : 用户功率信息初始化
*****************************************************************************/
osal_void hal_device_init_vap_pow_code(hal_to_dmac_device_stru *hal_device,
    hal_vap_pow_info_stru *vap_pow_info, hal_pow_set_type_enum_uint8 type, const mac_channel_stru *channel)
{
    osal_u8 max_pow;

    max_pow = (vap_pow_info->reg_pow >= HAL_MAX_TXPOWER_MAX) ? HAL_MAX_TXPOWER_MAX : (osal_u8)vap_pow_info->reg_pow;
    vap_pow_info->rate_pow_table = OAL_PTR_NULL;
    /*  转化mcs-pow table为mcs-pow_code table */
    hh503_pow_set_pow_to_pow_code(hal_device, max_pow, channel);
    hh503_pow_init_vap_pow_code_ext(hal_device, vap_pow_info, type, channel);
    hh503_set_cta_coef(hal_device, channel);
}

/************************************ *****************************************
 函 数 名  : hh503_set_rf_limit_power
 功能描述  : 设置rf limit power
*****************************************************************************/
osal_void hh503_set_rf_limit_power(wlan_channel_band_enum_uint8 band, osal_u8 power)
{
    osal_u8 limit_power;

    if (power > 30) { // 30 * 0.1dBm
        limit_power = 30;  // max value is 30 * 0.1dBm
    } else {
        limit_power = power;
    }
    g_pow_tpc_level_table[HAL_POW_RF_LIMIT_POW_LEVEL][band] = limit_power;
    oam_warning_log2(0, OAM_SF_ANY, "{hh503_set_rf_limit_power::band = %d, limit_power = %d}", band, limit_power);
}
/*****************************************************************************
 功能描述  : 读取一个用户表项到resp power lut表
*****************************************************************************/
osal_u32 hal_pow_read_machw_resp_power_lut_entry(osal_u8 lut_index)
{
    osal_u32 val2;
    u_peer_addr_lut_config peer_addr_lut_config = {0};
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = lut_index;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW__ = 0;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = 1;
    peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = 1;
    /* 写RESP POWER Control寄存器 */
    hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8, peer_addr_lut_config.u32);
    val2 = hal_reg_read(HH503_MAC_LUT0_BANK_BASE_0x4);
    return val2;
}
osal_void hal_tpc_rate_pow_print_rate_pow_table(osal_void)
{
    fe_tpc_rate_pow_print_rate_pow_table();
}

#ifdef BOARD_ASIC_WIFI
/************************************ ****************************************
 函 数 名  : hh503_get_cali_param_set_tpc
 功能描述  : 获取cali参数并设置phy TPC
*****************************************************************************/
osal_void hh503_get_cali_param_set_tpc(hal_to_dmac_device_stru *hal_device, wlan_channel_band_enum_uint8 band,
    osal_u8 channel_idx)
{
    unref_param(hal_device);
    unref_param(band);
    unref_param(channel_idx);
}
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
/************************************ *****************************************
 功能描述  : 初始化HE TB PPDU MCS最大和最小发送功率
*****************************************************************************/
osal_void hh503_tb_mcs_tx_power_ext(wlan_channel_band_enum_uint8 band)
{
    osal_u16 tpc_code = 0;
    osal_u8 i;
    u_he_tb_mcs_tx_power0 he_tb_mcs_tx_power0 = {.u32 = 0};
    /* 这些参数由rf提供，需要芯片回来后校准完rf才能确定 */
    mcs_tx_pow_stru mcs_tx_pow[10] = { /* 共10个mcs */
        {20, -11, 0}, {20, -11, 0}, {20, -11, 0}, {20, -11, 0}, /* 默认值:最大值20,最小值-10,偏移0 */
        {20, -11, 0}, {20, -11, 0}, {20, -11, 0}, {20, -11, 0}, /* 默认值:最大值20,最小值-10,偏移0 */
        {20, -11, 0}, {20, -11, 0}  /* 默认值:最大值20,最小值-10,偏移0 */
    };
    // 最大功率 单位0.5dBm
    osal_s16 chip_max_pow = fe_custom_get_chip_max_power(band) / FE_HAL_TPC_RATE_POW_LVL_VAL;
    for (i = 0; i < 10; i++) { /* 共10个mcs */
        // 每个速率的目标tpc
        tpc_code = fe_tpc_rate_pow_get_rate_tpc_code(band, WLAN_HE_SU_FORMAT, i, WLAN_BAND_ASSEMBLE_20M, 0);
        // 每个速率的目标功率dBm=最大功率dBm-tpc*0.5dB 这里填写给芯片的值单位是0.5dBm
        mcs_tx_pow[i].max_pow = (osal_u16)chip_max_pow - tpc_code;
    }

    /* 写HE TB PPDU HE-MCS发送功率参数 */
    he_tb_mcs_tx_power0.bits.x6NgwQWwLjwS6CwLPwuiTW_wy_ = (((osal_u32)mcs_tx_pow[0].max_pow & 0x3f) | /* 第0个 */
        (((osal_u32)mcs_tx_pow[1].max_pow & 0x3f) << 6) |  /* 第1个 偏移6 */
        (((osal_u32)mcs_tx_pow[2].max_pow & 0x3f) << 12) | /* 第2个 偏移12 */
        (((osal_u32)mcs_tx_pow[3].max_pow & 0x3f) << 18) | /* 第3个 偏移18 */
        (((osal_u32)mcs_tx_pow[4].max_pow & 0x3f) << 24)); /* 第4个 偏移24 */
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xE8, he_tb_mcs_tx_power0.u32);
    he_tb_mcs_tx_power0.bits.x6NgwQWwLjwS6CwLPwuiTW_wy_ = (((osal_u32)mcs_tx_pow[5].max_pow & 0x3f) | /* 第5个 */
        (((osal_u32)mcs_tx_pow[6].max_pow & 0x3f) << 6) |  /* 第6个 偏移6 */
        (((osal_u32)mcs_tx_pow[7].max_pow & 0x3f) << 12) | /* 第7个 偏移12 */
        (((osal_u32)mcs_tx_pow[8].max_pow & 0x3f) << 18) | /* 第8个 偏移18 */
        (((osal_u32)mcs_tx_pow[9].max_pow & 0x3f) << 24)); /* 第9个 偏移24 */
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xEC, he_tb_mcs_tx_power0.u32);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xF0, (osal_u32)chip_max_pow);   // 芯片最大功率来填写
}

/************************************ *****************************************
 功能描述  : 初始化HE TB PPDU发送功率表
*****************************************************************************/
osal_void hh503_tb_tx_power_init_ext(wlan_channel_band_enum_uint8 band)
{
    osal_u8 tx_power_set_value[64]; /* 64: 存储发送功率-32dBm~31dBm对应的tpc档位值 (以1dBm递进) */
    osal_u8 idx;
    osal_u32 he_tx_power_set;
    u_peer_addr_lut_config peer_addr_lut_config = {.u32 = 0};
    u_he_tb_ru_tx_power_max tb_ru_tx_power_max = {.u32 = 0};
    osal_u32 max_power = fe_custom_get_chip_max_power(band) / FE_HAL_TPC_RATE_POW_LVL_VAL;  // 最大功率 单位0.5db

    for (idx = 0; idx < 64; idx++) {    /* 64种功率 lut_index = (initial_power - real_power)dB * 2  与 tpc code 一一对应 */
        tx_power_set_value[idx] = idx;
    }

    for (idx = 0; idx < 16; idx++) { /* 64种功率,每4个为一组,共计 16 组 */
        he_tx_power_set = (tx_power_set_value[idx * 4] & 0x7f) |             /* 4:    获取tx_power存于0~6   bit */
                          ((tx_power_set_value[idx * 4 + 1] & 0x7f) << 7) |  /* 4/8:  获取tx_power存于8~14  左移7bit */
                          ((tx_power_set_value[idx * 4 + 2] & 0x7f) << 14) | /* 4/16: 获取tx_power存于16~22 左移14bit */
                          ((tx_power_set_value[idx * 4 + 3] & 0x7f) << 21);  /* 4/24: 获取tx_power存于24~30 左移21bit */
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x4, he_tx_power_set);
        if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8, HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK,
            HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK, HAL_CE_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{hh503_ce_add_peer_macaddr::waiting lut ce idle timeout!}\r\n");
            return;
        }
        peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wWh_ = 1;
        peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW__ = 1;
        peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwiuW_wCWo_ = 3; /* 3 0011 */
        peer_addr_lut_config.bits.x6NgwuWW_wGFF_wodLwBhFWP_ = idx;
        hal_reg_write(HH503_MAC_LUT0_BANK_BASE_0x8, peer_addr_lut_config.u32);
        if (hh503_wait_mac_done_timeout(HH503_MAC_LUT0_BANK_BASE_0x8, HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK,
            HH503_MAC_CFG_PEER_ADDR_LUT_OPER_EN_MASK, HAL_CE_LUT_UPDATE_TIMEOUT) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{hh503_ce_add_peer_macaddr::waiting lut ce idle timeout!}\r\n");
        }
    }
    // 刷新ru的功率限制 按最大值刷新
    tb_ru_tx_power_max.bits.x6NgwQWwLjw_dwLPwuiTW_wSGPwVX_ = max_power;
    tb_ru_tx_power_max.bits.xCt2OAmORyOwSOR8OGvqmwOIz8OaZ_ = max_power;
    tb_ru_tx_power_max.bits.x6NgwQWwLjw_dwLPwuiTW_wSGPw0yX_ = max_power;
    tb_ru_tx_power_max.bits.xCt2OAmORyOwSOR8OGvqmwOIz8OZKZ_ = max_power;
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xF4, tb_ru_tx_power_max.u32);
}
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */

/*****************************************************************************
 函 数 名  : hh503_pow_sw_initialize_tx_power_ext
 功能描述  : 初始化tx power软件相关
*****************************************************************************/
osal_void hh503_pow_sw_initialize_tx_power_ext(hal_to_dmac_device_stru *hal_device)
{
    unref_param(hal_device);
}

/************************************ *****************************************
 函 数 名  : hh503_device_init_vap_pow_code
 功能描述  : 用户功率信息初始化
*****************************************************************************/
osal_void hh503_pow_init_vap_pow_code_ext(hal_to_dmac_device_stru *hal_device,
    hal_vap_pow_info_stru *vap_pow_info, hal_pow_set_type_enum_uint8 type, const mac_channel_stru *channel)
{
    unref_param(hal_device);
    unref_param(vap_pow_info);
    unref_param(type);
    unref_param(channel);
}

static osal_void hh503_get_data_rate_field(osal_u8 *data_rate, osal_u32 addr)
{
    hh503_tx_phy_rate_stru phy_rate;
    hal_phy_rate_stru *hal_rate = (hal_phy_rate_stru *)data_rate;

    phy_rate.u32 = hal_reg_read(addr);
    hal_rate->bits.xICsOwzRm_ = phy_rate.bits.xLPwqW6Li_wS6Cw_GLW_;
    hal_rate->bits.protocol_mode = phy_rate.bits.xR8OPmCRvwOGwvRvCvxOIv4m_;
}

/*****************************************************************************
 功能描述  : 设置ctrl frm帧的txmode
*****************************************************************************/
osal_void hh503_set_ctrl_frm_pow_code(osal_u32 phy_mode_addr, osal_u32 data_rate_addr, osal_u32 pow_code)
{
    osal_u32 tx_phy_mode;
    osal_u32 tx_data_rate;
    tpc_code_params_stru *tpc_code;

    tpc_code = (tpc_code_params_stru *)(&pow_code);

    tx_phy_mode = hal_reg_read(phy_mode_addr);
    tx_data_rate = hal_reg_read(data_rate_addr);

    ((hh503_tx_phy_mode_reg_stru *)(&tx_phy_mode))->xLPwqW6Li_wLu6w6Qy_ = tpc_code->tpc_ch0;

    ((hh503_tx_phy_rate_stru *)(&tx_data_rate))->bits.xR8OPmCRvwORGCO4G4OxPO3_ = tpc_code->dpd_tpc_lv_ch0;
    ((hh503_tx_phy_rate_stru *)(&tx_data_rate))->bits.xR8OPmCRvwOCtwOl48_ = tpc_code->cfr_idx;

    hal_reg_write(phy_mode_addr, tx_phy_mode);
    hal_reg_write(data_rate_addr, tx_data_rate);
}

/*****************************************************************************
功能描述  : 设置resp帧三级功率门限
            接收帧小于等于该rssi值的话选择带有min后缀的响应帧功率值(高8位cfg_resp_power_lv_min_rssi),
            接收帧大于等于该rssi值的话选择带有max后缀的响应帧功率值(低8位cfg_resp_power_lv_max_rssi)
*****************************************************************************/
osal_void hh503_set_resp_pow_level(osal_char near_distance_rssi, osal_char far_distance_rssi)
{
    osal_u16 distance_rssi;

    /* 响应帧功率选择寄存器 */
    distance_rssi = oal_make_word16((osal_u8)near_distance_rssi, (osal_u8)far_distance_rssi);

    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0xE4, (osal_u32)distance_rssi);
}

/*****************************************************************************
 函 数 名  : hh503_set_spec_frm_phy_tx_mode
 功能描述  : 设置spec frm帧的txmode
*****************************************************************************/
osal_void hh503_set_spec_frm_phy_tx_mode(const hal_to_dmac_device_stru *hal_device, osal_u8 band,
    osal_u8 subband_idx)
{
    osal_u32 rts_pow_code;
    osal_u32 cfend_pow_code;
    osal_u32 vhtrpt_pow_code;
    osal_u32 bar_pow_code;

    if (band == WLAN_BAND_5G) {
        /* vht report */
        vhtrpt_pow_code = hal_device->phy_pow_param.vht_report_pow_code[subband_idx];
        hh503_set_ctrl_frm_pow_code(HH503_MAC_CTRL1_BANK_BASE_0x54, HH503_MAC_CTRL1_BANK_BASE_0x58, vhtrpt_pow_code);
        subband_idx += WLAN_2G_SUB_BAND_NUM;
    }

    /* RTS */
    rts_pow_code = hal_device->phy_pow_param.rts_pow_code[subband_idx];
    /* cf-end */
    cfend_pow_code = hal_device->phy_pow_param.cfend_pow_code[subband_idx];
    /* BAR */
    bar_pow_code = hal_device->phy_pow_param.bar_pow_code[subband_idx];

    /* RTS */
    hh503_set_ctrl_frm_pow_code(HH503_MAC_CTRL0_BANK_BASE_0xD4, HH503_MAC_CTRL0_BANK_BASE_0xD8, rts_pow_code);
    /* cf-end */
    hh503_set_ctrl_frm_pow_code(HH503_MAC_CTRL0_BANK_BASE_0xF0, HH503_MAC_CTRL0_BANK_BASE_0xF4, cfend_pow_code);
    /* bar */
    hh503_set_ctrl_frm_pow_code(HH503_MAC_CTRL0_BANK_BASE_0xF8, HH503_MAC_CTRL0_BANK_BASE_0xFC, bar_pow_code);
}
/*****************************************************************************
函 数 名  : hh503_get_spec_frm_rate
功能描述  : 获取spec frm帧的datarate
*****************************************************************************/
osal_void hh503_get_spec_frm_rate(hal_to_dmac_device_stru *hal_device)
{
    /* SIFS帧，54M, 48M, 36M, 24M */
    hal_device->phy_pow_param.rate_ofdm_ack_cts[WLAN_OFDM_ACK_CTS_TYPE_24M] = 0x19; /* 0x19对应24M速率 */

    /* RTS帧,RTS速率可变，功率按照典型速率配置, 6M */
    hh503_get_data_rate_field(&(hal_device->phy_pow_param.rate_rts), HH503_MAC_CTRL0_BANK_BASE_0xD8);

    /* abort selfcts帧 */
    hh503_get_data_rate_field(&(hal_device->phy_pow_param.rate_abort_selfcts), HH503_MAC_CTRL1_BANK_BASE_0x80);

    /* one pkt帧 */
    hh503_get_data_rate_field(&(hal_device->phy_pow_param.rate_one_pkt), HH503_MAC_CTRL1_BANK_BASE_0x14);

    /* cfend帧 */
    hh503_get_data_rate_field(&(hal_device->phy_pow_param.rate_cfend), HH503_MAC_CTRL0_BANK_BASE_0xF4);

    /* bar帧 */
    hh503_get_data_rate_field(&(hal_device->phy_pow_param.rate_bar), HH503_MAC_CTRL0_BANK_BASE_0xFC);

    /* vht report帧 */
    hh503_get_data_rate_field(&(hal_device->phy_pow_param.rate_vht_report), HH503_MAC_CTRL1_BANK_BASE_0x58);
}

/*****************************************************************************
 函 数 名  : hh503_set_tpc_params
 功能描述  : 设置TPC参数
*****************************************************************************/
osal_void hh503_set_tpc_params(hal_to_dmac_device_stru *hal_device, hal_rf_chn_param *rf_chn_param)
{
    osal_u8 subband_idx = 0;
    wlan_channel_band_enum_uint8 band = rf_chn_param->band;

    if (hal_device->pow_rf_reg_ctl_flag == OSAL_TRUE) {
        return;
    }
#ifdef BOARD_FPGA_WIFI  // 仅FPGA使用
    hh503_set_phy_gain();
#endif
    /* 5. 设置控制帧功率 */
    if (hh503_get_subband_index(band, rf_chn_param->ch_idx, &subband_idx) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_PWR, "{hh503_set_tpc_params::band[%d]chan_idx[%d] is invalid}", band,
            rf_chn_param->ch_idx);
        return;
    }
    hh503_set_spec_frm_phy_tx_mode(hal_device, band, subband_idx);
}

osal_void hal_tpc_cali_ftm_updata_channel(hal_to_dmac_device_stru *hal_device, mac_channel_stru *channel)
{
    hal_rf_chn_param rf_chn_param;

#ifdef BOARD_ASIC_WIFI
    hh503_get_cali_param_set_tpc(hal_device, channel->band, channel->chan_idx);
#endif

    /* 设置RF */
    rf_chn_param.band = channel->band;
    rf_chn_param.ch_num = channel->chan_number;
    rf_chn_param.ch_idx = channel->chan_idx;
    rf_chn_param.bw = channel->en_bandwidth;

#ifdef _PRE_WLAN_FEATURE_TPC
    hh503_set_tpc_params(hal_device, &rf_chn_param);
#endif
}

/*****************************************************************************
功能描述  : 设置beacon帧的txmode
*****************************************************************************/
osal_void hh503_set_bcn_phy_tx_mode(const hal_to_dmac_vap_stru *hal_vap, osal_u32 pow_code)
{
    osal_u8 vap_id = hal_vap->vap_id;

    /* beacon TX DATA RATE寄存器 */
    osal_u32 bcn_phy_tx_mode_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0x1C, HH503_MAC_CTRL2_BANK_BASE_0x44,
        HH503_MAC_CTRL2_BANK_BASE_0x13C
    };
    osal_u32 bcn_data_rate_mode_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0x20, HH503_MAC_CTRL2_BANK_BASE_0x48,
        HH503_MAC_CTRL2_BANK_BASE_0x140
    };

    /* 只有ap模式才设置 */
    if ((hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) || (vap_id >= HAL_MAX_VAP_NUM)) {
        oam_warning_log3(0, OAM_SF_ANY,
            "hh503_set_bcn_phy_tx_mode::vap mode [%d]!= WLAN_VAP_MODE_BSS_AP,or hal vap id=%d >= max vap=%d]}",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }

    hh503_set_ctrl_frm_pow_code(bcn_phy_tx_mode_reg[vap_id], bcn_data_rate_mode_reg[vap_id], pow_code);
}

/*****************************************************************************
函 数 名  : hh503_get_bcn_rate
功能描述  : 获取某个vap的beacon datarate
*****************************************************************************/
osal_void hh503_get_bcn_rate(const hal_to_dmac_vap_stru *hal_vap, osal_u8 *data_rate)
{
    osal_u8 vap_id = hal_vap->vap_id;
    hh503_tx_phy_rate_stru phy_rate;
    hal_phy_rate_stru *hal_rate = (hal_phy_rate_stru *)data_rate;

    /* beacon TX DATA RATE寄存器 */
    osal_u32 bcn_tx_rate_reg[] = {
        HH503_MAC_CTRL2_BANK_BASE_0x20, HH503_MAC_CTRL2_BANK_BASE_0x48,
        HH503_MAC_CTRL2_BANK_BASE_0x140
    };
    /* 只有ap模式才能设置 */
    if ((hal_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) || (vap_id >= HAL_MAX_VAP_NUM)) {
        oam_error_log3(0, OAM_SF_ANY,
            "hh503_get_bcn_rate::vap mode [%d]!= WLAN_VAP_MODE_BSS_AP,or hal vap id=%d >= max vap=%d]}",
            hal_vap->vap_mode, vap_id, HAL_MAX_VAP_NUM);
        return;
    }
    phy_rate.u32 = hal_reg_read(bcn_tx_rate_reg[vap_id]);
    hal_rate->bits.xICsOwzRm_ = phy_rate.bits.xLPwqW6Li_wS6Cw_GLW_;
    hal_rate->bits.protocol_mode = phy_rate.bits.xR8OPmCRvwOGwvRvCvxOIv4m_;
}
// 配置速率功率表的偏移 对应rpwr的命令和AT调整
osal_void hal_power_set_rate_power_offset(osal_u8 freq, osal_u8 rate, osal_s8 power_offset)
{
    if (rate < WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN) {
        g_tpc_rate_power_offset[rate] = power_offset;
    }
}
osal_s8 hal_power_get_rate_power_offset(osal_u8 freq, osal_u8 rate)
{
    if (rate < WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN) {
        return g_tpc_rate_power_offset[rate];
    }
    return 0;
}
osal_void hal_power_set_under_regulation(oal_bool_enum_uint8 under_regulation_flag)
{
    g_pow_under_regulation = under_regulation_flag;
}
OSAL_STATIC oal_bool_enum_uint8 hal_power_get_under_regulation(osal_void)
{
    return g_pow_under_regulation;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
