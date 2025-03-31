/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: fe_tpc_rate_pow
 * Create: 2022-10-14
 */
#include "fe_tpc_rate_pow.h"
#include "wlan_types.h"
#include "oam_ext_if.h"
#ifdef _PRE_PRODUCT_ID_HOST
#include "frw_util.h"
#include "fe_rf_customize_power.h"
#else
#include "frw_util_rom.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FE_TPC_RATE_POW
static fe_tpc_rate_pow_code_table_stru g_fe_tpc_rate_pow_code_table_2g[WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN];
/* 根据协议模式和带宽 查询速率功率表的偏移值 */
static const osal_u8 g_pow_rate_pow_code_offset[WLAN_POW_PROTOCOL_BUTT][WLAN_BANDWIDTH_BUTT] = {
    /* 20                       40                         80                         160 */
    /* 11b */
    {WLAN_POW_RATE_POW_CODE_11B_OFFSET, WLAN_POW_RATE_POW_CODE_INVALID_OFFSET,
        WLAN_POW_RATE_POW_CODE_INVALID_OFFSET, WLAN_POW_RATE_POW_CODE_INVALID_OFFSET},
    /* 11g/11a */
    {WLAN_POW_RATE_POW_CODE_LEGACY_OFFSET, WLAN_POW_RATE_POW_CODE_INVALID_OFFSET,
        WLAN_POW_RATE_POW_CODE_INVALID_OFFSET, WLAN_POW_RATE_POW_CODE_INVALID_OFFSET},
    /* 11n/11ac/11ax */
    {WLAN_POW_RATE_POW_CODE_NONLEGACY_20M_OFFSET, WLAN_POW_RATE_POW_CODE_NONLEGACY_40M_OFFSET,
        WLAN_POW_RATE_POW_CODE_NONLEGACY_80M_OFFSET, WLAN_POW_RATE_POW_CODE_NONLEGACY_160M_OFFSET}
};
/* 软件legacy rate映射索引 */
static const osal_u8 g_legacy_rate_idx[WLAN_LEGACY_RATE_VALUE_BUTT] = {
    0,  /* WLAN_LEGACY_11B_RESERVED1 */
    1,  /* WLAN_SHORT_11b_2M_BPS */
    2,  /* WLAN_SHORT_11B_5M5_BPS */
    3,  /* WLAN_SHORT_11B_11M_BPS */
    0,  /* WLAN_LONG_11B_1M_BPS */
    1,  /* WLAN_LONG_11B_2M_BPS */
    2,  /* WLAN_SHORT_11B_5M5_BPS */
    3,  /* WLAN_SHORT_11B_11M_BPS */
    10,  /* WLAN_LEGACY_OFDM_48M_BPS */
    8,  /* WLAN_LEGACY_OFDM_24M_BPS */
    6,  /* WLAN_LEGACY_OFDM_12M_BPS */
    4,  /* WLAN_LEGACY_OFDM_6M_BPS */
    11,  /* WLAN_LEGACY_OFDM_54M_BPS */
    9,  /* WLAN_LEGACY_OFDM_36M_BPS */
    7,  /* WLAN_LEGACY_OFDM_18M_BPS */
    5  /* WLAN_LEGACY_OFDM_9M_BPS */
};
/* 根据协议模式 wlan_phy_protocol_enum 和带宽 hal_channel_assemble_enum 查询速率功率表的偏移值 */
static osal_u8 fe_tpc_rate_pow_get_pow_idx_offset(osal_u8 protocol, osal_u8 bw)
{
    wlan_pow_protocol_enum pow_protocol = WLAN_POW_PROTOCOL_BUTT;
    wlan_bandwidth_type_enum bw_type = WLAN_BANDWIDTH_BUTT;
    // 协议转换为功率表使用的协议枚举
    switch (protocol) {
        case WLAN_11B_PHY_PROTOCOL_MODE:
            pow_protocol = WLAN_POW_PROTOCOL_11B;
            break;
        case WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE:
            pow_protocol = WLAN_POW_PROTOCOL_LEGACY_OFDM;
            break;
        case WLAN_HT_PHY_PROTOCOL_MODE:
        case WLAN_VHT_PHY_PROTOCOL_MODE:
        case WLAN_HE_SU_FORMAT:
        case WLAN_HE_MU_FORMAT:
        case WLAN_HE_EXT_SU_FORMAT:
        case WLAN_HE_TRIG_FORMAT:
            pow_protocol = WLAN_POW_PROTOCOL_NONLEGACY_OFDM;
            break;
        default:
            pow_protocol = WLAN_POW_PROTOCOL_BUTT;
            break;
    }
    // 带宽转换为功率表使用的带宽枚举
    switch (bw) {
        case WLAN_BAND_ASSEMBLE_20M:
            bw_type = WLAN_BANDWIDTH_20;
            break;
        case WLAN_BAND_ASSEMBLE_40M:
        case WLAN_BAND_ASSEMBLE_40M_DUP:
            bw_type = WLAN_BANDWIDTH_40;
            break;
        default:
            bw_type = WLAN_BANDWIDTH_20;
            break;
    }
    if (pow_protocol == WLAN_POW_PROTOCOL_BUTT || bw_type == WLAN_BANDWIDTH_BUTT) {
        oam_error_log2(0, OAM_SF_ANY, "fe_tpc_rate_pow_get_pow_idx_offset idx %d,%d err", protocol, bw);
        return WLAN_POW_RATE_POW_CODE_INVALID_OFFSET;
    }

    return g_pow_rate_pow_code_offset[pow_protocol][bw_type];
}
// 取协议速率下的速率档位
// protocol:wlan_phy_protocol_enum bw:hal_channel_assemble_enum
osal_u8 fe_tpc_rate_pow_get_rate_idx(osal_u8 protocol, osal_u8 mcs, osal_u8 bw)
{
    osal_u8 offset_idx;
    osal_u8 rate_idx = WLAN_LEGACY_11B_RESERVED1;

    /* 获取用户的基准速率索引 */
    offset_idx = fe_tpc_rate_pow_get_pow_idx_offset(protocol, bw);
    if (offset_idx == WLAN_POW_RATE_POW_CODE_INVALID_OFFSET) {
        return rate_idx;
    }

    /* 获取11b/11a/g速率的功率索引 */
    if ((protocol  == WLAN_11B_PHY_PROTOCOL_MODE) || (protocol == WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE)) {
        if (mcs >= WLAN_LEGACY_RATE_VALUE_BUTT) {
            oam_error_log2(0, OAM_SF_ANY, "fe_tpc_rate_pow_get_rate_idx idx %d,%d err", protocol, mcs);
            return rate_idx;
        }
        rate_idx = g_legacy_rate_idx[mcs];
    } else if (protocol == WLAN_HT_PHY_PROTOCOL_MODE) { /* 获取11n的功率索引 */
        if (mcs == WLAN_HT_MCS32) {
            rate_idx = WLAN_POW_NONLEGACY_40M_NUM - 1;  // mcs32 放在40M最后
#if (defined(WLAN_DOUBLE_NSS) && defined(WLAN_MAX_NSS_NUM) && WLAN_DOUBLE_NSS <= WLAN_MAX_NSS_NUM)
        } else if (mcs > WLAN_HT_MCS7) {
            rate_idx = mcs - WLAN_HT_MCS8;  // 双流减去偏差
#endif
        } else {
            rate_idx = mcs;
        }
    } else {    /* 获取11ac/11ax的功率索引 */
        rate_idx = mcs;
    }
    return offset_idx + rate_idx;
}
// 取2g频段下的协议速率功率表
static inline fe_tpc_rate_pow_code_table_stru *fe_tpc_get_rate_pow_table_2g(osal_void)
{
    return &g_fe_tpc_rate_pow_code_table_2g[0];
}
#ifdef _PRE_PRODUCT_ID_HOST
osal_void fe_hal_pow_sync_tpc_pow_table_to_dmac(hal_to_dmac_device_stru *hal_device, osal_u8 vap_id)
{
    fe_tpc_rate_pow_code_table_stru *pow_code_table = fe_tpc_get_rate_pow_table_2g();
    osal_u32 table_len = WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN * HAL_POW_LEVEL_NUM;
    const osal_u32 mem_size = table_len * sizeof(osal_u16);
    osal_u16 data[WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN * HAL_POW_LEVEL_NUM];
    frw_msg msg = {0};
    osal_u32 ret;
    osal_u32 rate_idx;
    osal_u32 pwr_idx;

    for (rate_idx = 0; rate_idx < WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN; rate_idx++) {
        for (pwr_idx = 0; pwr_idx < HAL_POW_LEVEL_NUM; pwr_idx++) {
            data[rate_idx * HAL_POW_LEVEL_NUM + pwr_idx] = pow_code_table[rate_idx].tpc_code_level[pwr_idx];
        }
    }

    msg.data = (osal_u8 *)data;
    msg.data_len = mem_size;
    ret = frw_send_msg_to_device(vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SYNC_TPC, &msg, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "hal_pow_sync_tpc_code_to_dmac:send tpc fail[%d]", ret);
    }
}
osal_u8 fe_hal_pow_get_custom_tpc_pow(osal_u8 band, osal_u8 rate_idx)
{
    osal_u8 rate_idx_offset = 0;
    osal_u8 rate_idx_limit = 0xff;
    osal_u8 rate_idx_base = 0;

    if (rate_idx < WLAN_POW_11B_RATE_NUM) {
        rate_idx_offset = rate_idx;
        rate_idx_base = 0;
        rate_idx_limit = FE_CUSTOMIZE_TARGET_POWER_11B_LEN;
    } else if (rate_idx < WLAN_POW_11B_RATE_NUM + WLAN_POW_LEGACY_RATE_NUM) {
        rate_idx_offset = rate_idx - WLAN_POW_11B_RATE_NUM;
        rate_idx_base = FE_CUSTOMIZE_TARGET_POWER_11B_LEN;
        rate_idx_limit = FE_CUSTOMIZE_TARGET_POWER_11G_LEN;
    } else if (rate_idx < WLAN_POW_11B_RATE_NUM + WLAN_POW_LEGACY_RATE_NUM + WLAN_POW_NONLEGACY_20M_NUM) {
        rate_idx_offset = rate_idx - WLAN_POW_11B_RATE_NUM - WLAN_POW_LEGACY_RATE_NUM;
        rate_idx_base = FE_CUSTOMIZE_TARGET_POWER_11B_LEN + FE_CUSTOMIZE_TARGET_POWER_11G_LEN;
        rate_idx_limit = FE_CUSTOMIZE_TARGET_POWER_20M_LEN;
    } else if (rate_idx < WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN) {
        rate_idx_offset = rate_idx - WLAN_POW_11B_RATE_NUM - WLAN_POW_LEGACY_RATE_NUM - WLAN_POW_NONLEGACY_20M_NUM;
        rate_idx_base = FE_CUSTOMIZE_TARGET_POWER_11B_LEN + FE_CUSTOMIZE_TARGET_POWER_11G_LEN +
            FE_CUSTOMIZE_TARGET_POWER_20M_LEN;
        rate_idx_limit = FE_CUSTOMIZE_TARGET_POWER_40M_LEN;
    }
    if (rate_idx_offset >= rate_idx_limit) {
        rate_idx_offset = rate_idx_limit - 1;
    }
    return fe_custom_get_target_power(band, rate_idx_base + rate_idx_offset);
}
// pow:功率值 单位0.1dBm is_ofdm:是否ofdm信号功率 针对非ofdm信号功率 档位需要增加 FE_HAL_TPC_RATE_TPC_CODE_NUM
osal_u8 fe_hal_pow_get_tpc_code_by_pow(osal_u8 band, osal_s16 pow, osal_u8 is_ofdm)
{
    osal_s16 max_pow = fe_custom_get_chip_max_power(band);
    osal_u16 tpc_code = 0;

    if (pow > max_pow) {
        return (osal_u8)tpc_code;
    }

    tpc_code = (max_pow - pow) / FE_HAL_TPC_RATE_POW_LVL_VAL;
    if (tpc_code >= FE_HAL_TPC_RATE_TPC_CODE_NUM) {
        tpc_code = FE_HAL_TPC_RATE_TPC_CODE_NUM - 1;
    }
    return (osal_u8)((is_ofdm) ? tpc_code : (tpc_code + FE_HAL_TPC_RATE_TPC_CODE_NUM));
}
// 打印速率功率表
osal_void fe_tpc_rate_pow_print_rate_pow_table(osal_void)
{
    fe_tpc_rate_pow_code_table_stru *table = fe_tpc_get_rate_pow_table_2g();
    osal_u8 rate_idx;
    osal_u8 pwr_lvl;
    unref_param(table);
    wifi_printf("fe_tpc_rate_pow_print_rate_pow_table begin:\r\n");
    wifi_printf("rate_idx\tpwr_lvl\ttpc_code\r\n");
    for (rate_idx = 0; rate_idx < WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN; rate_idx++) {
        for (pwr_lvl = 0; pwr_lvl < HAL_POW_LEVEL_NUM; pwr_lvl++) {
            wifi_printf("%8d\t%7d\t%8d\r\n", rate_idx, pwr_lvl, table[rate_idx].tpc_code_level[pwr_lvl]);
        }
    }
}
#else
// 刷新同步2g频段下的tpc协议速率功率档位数据
osal_void fe_hal_sync_tpc_pow_rate_table(osal_u8 *data, osal_u16 data_len)
{
    fe_tpc_rate_pow_code_table_stru *table = fe_tpc_get_rate_pow_table_2g();
    osal_u16 *sync_table = (osal_u16 *)data;
    osal_u32 idx;
    osal_u32 pwr_idx;
    osal_u32 max_index;
    osal_u32 table_idx;
    // 长度保护
    max_index = data_len / sizeof(osal_u16);
    for (idx = 0; idx < WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN; idx++) {
        for (pwr_idx = 0; pwr_idx < HAL_POW_LEVEL_NUM; pwr_idx++) {
            table_idx = idx * HAL_POW_LEVEL_NUM + pwr_idx;
            if (table_idx >= max_index) {
                return;
            }
            table[idx].tpc_code_level[pwr_idx] = sync_table[table_idx];
        }
    }
}
#endif
// 取协议速率下的功率档位
// band:wlan_channel_band_enum protocol:wlan_phy_protocol_enum bw:hal_channel_assemble_enum pwr_lvl:HAL_POW_LEVEL_NUM
osal_u16 fe_tpc_rate_pow_get_rate_tpc_code(osal_u8 band, osal_u8 protocol, osal_u8 mcs, osal_u8 bw, osal_u8 pwr_lvl)
{
    osal_u16 tpc_code = 0;   // 默认使用最大功率
    osal_u8 rate_idx;
    fe_tpc_rate_pow_code_table_stru *table = OAL_PTR_NULL;
    tpc_code_level_stru *code = (tpc_code_level_stru *)&tpc_code;

    if (band >= WLAN_BAND_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "fe_tpc_rate_pow_get_rate_tpc_code band %d err", band);
        return tpc_code;
    }

    rate_idx = fe_tpc_rate_pow_get_rate_idx(protocol, mcs, bw);
    if (rate_idx >= WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN || pwr_lvl >= HAL_POW_LEVEL_NUM) {
        oam_error_log2(0, OAM_SF_ANY, "fe_tpc_rate_pow_get_rate_tpc_code idx %d lvl :%d err", rate_idx, pwr_lvl);
        return tpc_code;
    }
    table = fe_tpc_get_rate_pow_table_2g();
    tpc_code = table[rate_idx].tpc_code_level[pwr_lvl];
    if (rate_idx < WLAN_POW_11B_RATE_NUM) {
        code->tpc_code += FE_HAL_TPC_RATE_TPC_CODE_NUM;
    }
    return tpc_code;
}
// 配置指定速率档位下的功率档位配置
// band:wlan_channel_band_enum  rate_idx:WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN pwr_lvl:HAL_POW_LEVEL_NUM
osal_void fe_tpc_rate_pow_set_tpc_code_by_rate_idx(osal_u8 band, osal_u8 rate_idx,
    osal_u8 pwr_lvl, osal_u16 tpc_code_st)
{
    fe_tpc_rate_pow_code_table_stru *table = fe_tpc_get_rate_pow_table_2g();

    if (band >= WLAN_BAND_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "fe_tpc_rate_pow_set_tpc_code_by_rate_idx band %d err", band);
        return;
    }

    if (rate_idx >= WLAN_POW_RATE_POW_CODE_TABLE_2G_LEN || pwr_lvl >= HAL_POW_LEVEL_NUM) {
        oam_error_log2(0, OAM_SF_ANY, "fe_tpc_rate_pow_set_tpc_code_by_rate_idx idx %d lvl %d err", rate_idx, pwr_lvl);
        return;
    }
    table[rate_idx].tpc_code_level[pwr_lvl] = tpc_code_st;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
