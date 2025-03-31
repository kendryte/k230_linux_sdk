/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Source file of mac regdomain.
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "mac_regdomain.h"
#include "mac_device_ext.h"
#include "mac_vap_ext.h"
#include "mac_resource_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_REGDOMAIN_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    全局变量定义
*****************************************************************************/
/* 管制域信息全局变量 */
mac_regdomain_info_stru g_mac_regdomain;

#ifdef _PRE_WLAN_SUPPORT_5G
/* 信道号列表，初始化其管制类为无效 */
mac_channel_info_stru g_channel_list_5g[MAC_CHANNEL_FREQ_5_BUTT] = {
    {36,  MAC_INVALID_RC, {0}}, {40,  MAC_INVALID_RC, {0}}, {44,  MAC_INVALID_RC, {0}},
    {48,  MAC_INVALID_RC, {0}}, {52,  MAC_INVALID_RC, {0}}, {56,  MAC_INVALID_RC, {0}},
    {60,  MAC_INVALID_RC, {0}}, {64,  MAC_INVALID_RC, {0}}, {100, MAC_INVALID_RC, {0}},
    {104, MAC_INVALID_RC, {0}}, {108, MAC_INVALID_RC, {0}}, {112, MAC_INVALID_RC, {0}},
    {116, MAC_INVALID_RC, {0}}, {120, MAC_INVALID_RC, {0}}, {124, MAC_INVALID_RC, {0}},
    {128, MAC_INVALID_RC, {0}}, {132, MAC_INVALID_RC, {0}}, {136, MAC_INVALID_RC, {0}},
    {140, MAC_INVALID_RC, {0}}, {144, MAC_INVALID_RC, {0}}, {149, MAC_INVALID_RC, {0}},
    {153, MAC_INVALID_RC, {0}}, {157, MAC_INVALID_RC, {0}}, {161, MAC_INVALID_RC, {0}},
    {165, MAC_INVALID_RC, {0}}, {184, MAC_INVALID_RC, {0}}, {188, MAC_INVALID_RC, {0}},
    {192, MAC_INVALID_RC, {0}}, {196, MAC_INVALID_RC, {0}},
};
#endif

mac_channel_info_stru g_channel_list_2g[MAC_CHANNEL_FREQ_2_BUTT] = {
    {1,  MAC_INVALID_RC, {0}}, {2,  MAC_INVALID_RC, {0}}, {3,  MAC_INVALID_RC, {0}},
    {4,  MAC_INVALID_RC, {0}}, {5,  MAC_INVALID_RC, {0}}, {6,  MAC_INVALID_RC, {0}},
    {7,  MAC_INVALID_RC, {0}}, {8,  MAC_INVALID_RC, {0}}, {9,  MAC_INVALID_RC, {0}},
    {10, MAC_INVALID_RC, {0}}, {11, MAC_INVALID_RC, {0}}, {12, MAC_INVALID_RC, {0}},
    {13, MAC_INVALID_RC, {0}}, {14, MAC_INVALID_RC, {0}},
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    {15, MAC_INVALID_RC, {0}},
    {16, MAC_INVALID_RC, {0}},
#endif
};
const mac_supp_mode_table_stru g_hmac_bw_mode_table_2g[] = {
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    { 1, { WLAN_BAND_WIDTH_20M } },
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    { 1, { WLAN_BAND_WIDTH_20M } },
    { 1, { WLAN_BAND_WIDTH_20M } },
#endif
};

#ifdef _PRE_WLAN_SUPPORT_5G
const mac_supp_mode_table_stru g_hmac_bw_mode_table_5g[] = {
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },  /* 36  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 40  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },  /* 44  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 48  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },  /* 52  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 56  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },  /* 60  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 64  */

    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },  /* 100 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 104 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },  /* 108 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 112 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },  /* 116 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 120 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },  /* 124 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 128 */

    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS } },   /* 132 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 136 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },  /* 140 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 144 */

    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },  /* 149 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 153 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },  /* 157 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 161 */
    { 1, { WLAN_BAND_WIDTH_20M } },                                                        /* 165 */

    /* for JP 4.9G */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS } },   /* 184 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 188 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },  /* 192 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 196 */
};

/* 5G频段 信道与中心频率映射 */
const mac_freq_channel_map_stru g_hmac_freq_map_5g_etc[MAC_CHANNEL_FREQ_5_BUTT] = {
    { 5180, 36, 0},
    { 5200, 40, 1},
    { 5220, 44, 2},
    { 5240, 48, 3},
    { 5260, 52, 4},
    { 5280, 56, 5},
    { 5300, 60, 6},
    { 5320, 64, 7},
    { 5500, 100, 8},
    { 5520, 104, 9},
    { 5540, 108, 10},
    { 5560, 112, 11},
    { 5580, 116, 12},
    { 5600, 120, 13},
    { 5620, 124, 14},
    { 5640, 128, 15},
    { 5660, 132, 16},
    { 5680, 136, 17},
    { 5700, 140, 18},
    { 5720, 144, 19},
    { 5745, 149, 20},
    { 5765, 153, 21},
    { 5785, 157, 22},
    { 5805, 161, 23},
    { 5825, 165, 24},
    /* for JP 4.9G */
    { 4920, 184, 25},
    { 4940, 188, 26},
    { 4960, 192, 27},
    { 4980, 196, 28},
};
#endif

/* 2.4G频段 信道与中心频率映射 */
const mac_freq_channel_map_stru g_hmac_freq_map_2g_etc[MAC_CHANNEL_FREQ_2_BUTT] = {
    { 2412, 1, 0},
    { 2417, 2, 1},
    { 2422, 3, 2},
    { 2427, 4, 3},
    { 2432, 5, 4},
    { 2437, 6, 5},
    { 2442, 7, 6},
    { 2447, 8, 7},
    { 2452, 9, 8},
    { 2457, 10, 9},
    { 2462, 11, 10},
    { 2467, 12, 11},
    { 2472, 13, 12},
    { 2484, 14, 13},
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    { 2512, 15, 14},
    { 2384, 16, 15}
#endif
};

/*****************************************************************************
    函数实现
*****************************************************************************/
const mac_freq_channel_map_stru *hmac_regdomain_get_freq_map_2g_etc(osal_void)
{
    return g_hmac_freq_map_2g_etc;
}

#ifdef _PRE_WLAN_SUPPORT_5G
const mac_freq_channel_map_stru *hmac_regdomain_get_freq_map_5g_etc(osal_void)
{
    return g_hmac_freq_map_5g_etc;
}
#endif

/*****************************************************************************
 函 数 名 : hmac_regdomain_get_channel_to_bw_mode_idx
 功能描述 : 根据信道获取其在g_hmac_bw_mode_table_2g或者g_hmac_bw_mode_table_5g这个表中对应的索引
*****************************************************************************/
OAL_STATIC osal_u8 hmac_regdomain_get_channel_to_bw_mode_idx(osal_u8 channel_number)
{
    osal_u8 idx = 0;

    if (channel_number == 0) {
        oam_error_log1(0, OAM_SF_ANY, "hmac_regdomain_get_channel_to_bw_mode_idx::unknow channel number=%d",
            channel_number);
        return idx;
    }

    if (channel_number <= CHANNEL14) {
        idx = channel_number - 1;
    } else {
        if (channel_number <= CHANNEL64) {
            idx = (osal_u8)((osal_u32)(channel_number - CHANNEL36) >> SHITF_BIT2); // [0,7]
        } else if (channel_number <= CHANNEL144) {
            idx = (osal_u8)((osal_u32)(channel_number - CHANNEL100) >> SHITF_BIT2) + CHANNEL8; //  [8, 19]
        } else if (channel_number <= CHANNEL165) {
            idx = (osal_u8)((osal_u32)(channel_number - CHANNEL149) >> SHITF_BIT2) + CHANNEL20; //  [20, 24]
        } else if (channel_number <= CHANNEL196) {
            idx = (osal_u8)((osal_u32)(channel_number - CHANNEL184) >> SHITF_BIT2) + CHANNEL25; //  [25, 28]
        } else {
            oam_warning_log1(0, OAM_SF_ANY,
                "hmac_regdomain_get_channel_to_bw_mode_idx::unknow channel=%d, force idx = chan 36", channel_number);
            idx = 0;
        }
    }

    return idx;
}

#ifdef _PRE_WLAN_SUPPORT_5G
/* 配置5G带宽模式，结果存储在bw_mode */
OAL_STATIC osal_void hmac_regdomain_get_5g_bw_mode(wlan_channel_bandwidth_enum_uint8 cfg_bw, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 *bw_mode)
{
    osal_u8 idx, i;
    idx = hmac_regdomain_get_channel_to_bw_mode_idx(channel);
    if (cfg_bw == WLAN_BAND_WIDTH_40M) {
        // 未配置带宽扩展方向时，使用该信道支持的40M带宽扩展模式
        *bw_mode = g_hmac_bw_mode_table_5g[idx].aen_supp_bw[1];
    } else if (cfg_bw == WLAN_BAND_WIDTH_80M) {
        // 未配置带宽扩展方向时，使用该信道支持的80M带宽扩展模式
        *bw_mode = g_hmac_bw_mode_table_5g[idx].aen_supp_bw[2];   /* 2 for 80M */
    } else if ((cfg_bw >= WLAN_BAND_WIDTH_40PLUS) && (cfg_bw <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        // 配置了带宽扩展方向时，检查当前信道是否支持该扩展方向
        for (i = 0; i < g_hmac_bw_mode_table_5g[idx].cnt; i++) {
            if (g_hmac_bw_mode_table_5g[idx].aen_supp_bw[i] == cfg_bw) {
                break;
            }
        }

        if (i == g_hmac_bw_mode_table_5g[idx].cnt) {
            // 该信道不支持设置的带宽模式,提示用户带宽扩展方向被驱动自适应调整了
            *bw_mode = ((cfg_bw == WLAN_BAND_WIDTH_40PLUS) || (cfg_bw == WLAN_BAND_WIDTH_40MINUS)) ?
                g_hmac_bw_mode_table_5g[idx].aen_supp_bw[1] : g_hmac_bw_mode_table_5g[idx].aen_supp_bw[2]; // 2 for 80M

            oam_warning_log3(0, OAM_SF_ANY,
                "hmac_regdomain_get_support_bw_mode:current ch:%d 5G not support bw_mode:%d,change to bw_mode:%d",
                channel, cfg_bw, *bw_mode);
        } else {
            *bw_mode = cfg_bw;
        }
    } else {
        oam_error_log1(0, OAM_SF_ANY, "hmac_regdomain_get_support_bw_mode:5G not support bw_mode=%d to 20M", cfg_bw);
    }
}
#endif

/*****************************************************************************
 函 数 名  : hmac_regdomain_get_support_bw_mode
 功能描述  : 根据配置的带宽模式(可能不带带宽扩展方向或者错误的扩展方向)所包含的带宽宽度信息，
             查找该信道支持的带宽模式
*****************************************************************************/
wlan_channel_bandwidth_enum_uint8 hmac_regdomain_get_support_bw_mode(wlan_channel_bandwidth_enum_uint8 cfg_bw,
    osal_u8 channel)
{
    osal_u8 idx, i;
    wlan_channel_bandwidth_enum_uint8 bw_mode = WLAN_BAND_WIDTH_20M;
    osal_u8 channel_num_2g = CHANNEL14;

    if (channel == 0) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_regdomain_get_support_bw_mode::channel not set yet!");
        return bw_mode;
    }

    if (cfg_bw == WLAN_BAND_WIDTH_20M) {
        return bw_mode;
    }

    idx = hmac_regdomain_get_channel_to_bw_mode_idx(channel);

#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    channel_num_2g = CHANNEL16;
#endif
    if (channel <= channel_num_2g) {
        if (cfg_bw == WLAN_BAND_WIDTH_40M) {
            // 未配置带宽扩展方向时，默认用第一种带宽扩展方式
            bw_mode = g_hmac_bw_mode_table_2g[idx].aen_supp_bw[1];
            return bw_mode;
        }

        if ((cfg_bw != WLAN_BAND_WIDTH_40PLUS) && (cfg_bw != WLAN_BAND_WIDTH_40MINUS)) {
            oam_error_log1(0, OAM_SF_ANY,
                "{hmac_regdomain_get_support_bw_mode::2G no sup bw_mode=%d,force 20M", cfg_bw);
            return bw_mode;
        }

        // 配置了带宽扩展方向时，检查当前信道是否支持该扩展方向
        for (i = 0; i < g_hmac_bw_mode_table_2g[idx].cnt; i++) {
            if (g_hmac_bw_mode_table_2g[idx].aen_supp_bw[i] == cfg_bw) {
                break;
            }
        }

        bw_mode = (i == g_hmac_bw_mode_table_2g[idx].cnt) ? g_hmac_bw_mode_table_2g[idx].aen_supp_bw[1] : cfg_bw;
    }  else {
#ifdef _PRE_WLAN_SUPPORT_5G
        hmac_regdomain_get_5g_bw_mode(cfg_bw, channel, &bw_mode);
#endif
    }
    return bw_mode;
}

/*****************************************************************************
 功能描述 : 设置管制域信息，配置命令会调到此处
*****************************************************************************/
osal_u32 hmac_regdomain_set_country_etc(osal_u16 len, osal_u8 *param)
{
    mac_cfg_country_stru *country_param;
    mac_regdomain_info_stru *mac_regdom;
    osal_u8 rc_num;
    osal_u32 size;
    unref_param(len);

    country_param = (mac_cfg_country_stru *)param;

    mac_regdom = (mac_regdomain_info_stru *)country_param->mac_regdom;

    /* 获取管制类的个数 */
    rc_num = mac_regdom->regclass_num;

    /* 计算配置命令 */
    size = (osal_u32)(OAL_SIZEOF(mac_regclass_info_stru) * rc_num + MAC_RD_INFO_LEN);
    /* 更新管制域信息 */
    if (memcpy_s((osal_u8 *)&g_mac_regdomain, sizeof(g_mac_regdomain), (osal_u8 *)mac_regdom, size) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_regdomain_set_country_etc::memcpy_s error}");
    }

    /* 更新信道的管制域信息 */
    hmac_init_channel_list_etc();

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 获取国家字符
*****************************************************************************/
osal_s8 *hmac_regdomain_get_country_etc(void)
{
    return g_mac_regdomain.ac_country;
}

/*****************************************************************************
 函 数 名  : hmac_get_regdomain_info_etc
 功能描述  : 获取管制域信息
*****************************************************************************/
osal_void hmac_get_regdomain_info_etc(mac_regdomain_info_stru **rd_info)
{
    if (rd_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_get_regdomain_info_etc::ppst_rd_info null.}");

        return;
    }

    *rd_info = &g_mac_regdomain;

    return;
}

#ifdef _PRE_WLAN_FEATURE_DFS
/*****************************************************************************
 功能描述 : 获取指定VAP主、次(扩展)信道信息
*****************************************************************************/
osal_void hmac_get_ext_chan_info(osal_u8 pri20_channel_idx, wlan_channel_bandwidth_enum_uint8 bandwidth,
    mac_channel_list_stru *chan_info)
{
    osal_u8 start_idx = pri20_channel_idx;
    switch (bandwidth) {
        case WLAN_BAND_WIDTH_20M:
            chan_info->channel = CHANNEL1;
            break;

        case WLAN_BAND_WIDTH_40PLUS:
            chan_info->channel = CHANNEL2;
            break;

        case WLAN_BAND_WIDTH_40MINUS:
            chan_info->channel = CHANNEL2;
            start_idx = pri20_channel_idx - CHANNEL_IDX_COUNT1;
            break;

        case WLAN_BAND_WIDTH_80PLUSPLUS:
            chan_info->channel = CHANNEL4;
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            chan_info->channel = CHANNEL4;
            start_idx = pri20_channel_idx - CHANNEL_IDX_COUNT2;
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            chan_info->channel = CHANNEL4;
            start_idx = pri20_channel_idx - CHANNEL_IDX_COUNT1;
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            chan_info->channel = CHANNEL4;
            start_idx = pri20_channel_idx - CHANNEL_IDX_COUNT3;
            break;
        default:
            chan_info->channel = 0;
            oam_error_log1(0, OAM_SF_DFS, "hmac_get_ext_chan_info::Invalid bandwidth %d.", bandwidth);
            break;
    }

    if (chan_info->channel) {
#ifdef _PRE_WLAN_SUPPORT_5G
        (osal_void)memcpy_s(chan_info->channels, MAC_MAX_20M_SUB_CH * sizeof(mac_freq_channel_map_stru),
            &g_hmac_freq_map_5g_etc[start_idx], chan_info->channel * sizeof(mac_freq_channel_map_stru));
#endif
    }
}

/*****************************************************************************
 函 数 名  : hmac_is_cover_dfs_channel
 功能描述  : 判断信道当前带宽模式下是否覆盖到DFS信道
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_cover_dfs_channel(osal_u8 band, wlan_channel_bandwidth_enum_uint8 bandwidth,
    osal_u8 channel_num)
{
    mac_channel_list_stru chan_info;
    osal_u8 channel_idx = 0xff;
    osal_u32 i;

    if (band != MAC_RC_START_FREQ_5) {
        return OSAL_FALSE;
    }

    if (hmac_get_channel_idx_from_num_etc(band, channel_num, &channel_idx) != OAL_SUCC) {
        return OSAL_FALSE;
    }

    hmac_get_ext_chan_info(channel_idx, bandwidth, &chan_info);

    for (i = 0; i < chan_info.channel; i++) {
        if (mac_is_ch_in_radar_band(band, chan_info.channels[i].idx)) {
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_regdomain_channel_is_support_bw
 功能描述  : 检查当前信道配置的带宽是否支持
*****************************************************************************/
osal_bool hmac_regdomain_channel_is_support_bw(wlan_channel_bandwidth_enum_uint8 cfg_bw, osal_u8 channel)
{
    osal_u8 idx;
    osal_u8 bw_loop;
    mac_supp_mode_table_stru supp_mode_table;
    wlan_channel_band_enum_uint8 channel_band;

    if (channel == 0) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_regdomain_channel_is_support_bw::channel not set yet!");
        return OSAL_FALSE;
    }

    if (cfg_bw == WLAN_BAND_WIDTH_20M) {
        return OSAL_TRUE;
    }

    channel_band = mac_get_band_by_channel_num(channel);
    idx = hmac_regdomain_get_channel_to_bw_mode_idx(channel);
    if ((channel_band == WLAN_BAND_2G) &&
        (idx < sizeof(g_hmac_bw_mode_table_2g) / sizeof(g_hmac_bw_mode_table_2g[0]))) {
        supp_mode_table = g_hmac_bw_mode_table_2g[idx];
#ifdef _PRE_WLAN_SUPPORT_5G
    } else if ((channel_band == WLAN_BAND_5G) &&
        (idx < sizeof(g_hmac_bw_mode_table_5g) / sizeof(g_hmac_bw_mode_table_5g[0]))) {
        supp_mode_table = g_hmac_bw_mode_table_5g[idx];
#endif
    } else {
        return OSAL_FALSE;
    }

    for (bw_loop = 0; bw_loop < supp_mode_table.cnt; bw_loop++) {
        if (cfg_bw == supp_mode_table.aen_supp_bw[bw_loop]) {
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_init_regdomain_etc
 功能描述  : 初始化默认管制域信息
*****************************************************************************/
osal_void hmac_init_regdomain_etc(osal_void)
{
    osal_s8 ac_default_country[] = "99";
    mac_regclass_info_stru *regclass = OSAL_NULL;

    (osal_void)memcpy_s(g_mac_regdomain.ac_country, WLAN_COUNTRY_STR_LEN * sizeof(osal_char),
        ac_default_country, sizeof(ac_default_country));
    /* 初始默认的管制类个数为2 */
    g_mac_regdomain.regclass_num = 2;

    /*************************************************************************
        初始化管制类1
    *************************************************************************/
    regclass = &(g_mac_regdomain.regclass[0]);

    regclass->start_freq = MAC_RC_START_FREQ_2;
    regclass->ch_spacing = MAC_CH_SPACING_5MHZ;
    regclass->behaviour_bmap = 0;
    regclass->coverage_class = 0;
    regclass->max_reg_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR;
    regclass->max_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR * 10; /* 10用于功率单位转换 */

    regclass->channel_bmap = mac_get_ch_bit(MAC_CHANNEL1) | mac_get_ch_bit(MAC_CHANNEL2) |
        mac_get_ch_bit(MAC_CHANNEL3) | mac_get_ch_bit(MAC_CHANNEL4) | mac_get_ch_bit(MAC_CHANNEL5) |
        mac_get_ch_bit(MAC_CHANNEL6) | mac_get_ch_bit(MAC_CHANNEL7) | mac_get_ch_bit(MAC_CHANNEL8) |
        mac_get_ch_bit(MAC_CHANNEL9) | mac_get_ch_bit(MAC_CHANNEL10) | mac_get_ch_bit(MAC_CHANNEL11) |
        mac_get_ch_bit(MAC_CHANNEL12) | mac_get_ch_bit(MAC_CHANNEL13);

    /*************************************************************************
        初始化管制类2
    *************************************************************************/
    regclass = &(g_mac_regdomain.regclass[1]);

    regclass->start_freq = MAC_RC_START_FREQ_5;
    regclass->ch_spacing = MAC_CH_SPACING_20MHZ;
    regclass->behaviour_bmap = 0;
    regclass->coverage_class = 0;
    regclass->max_reg_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR;
    regclass->max_tx_pwr = MAC_RC_DEFAULT_MAX_TX_PWR * 10; /* 10用于功率单位转换 */

    regclass->channel_bmap = mac_get_ch_bit(MAC_CHANNEL36) | mac_get_ch_bit(MAC_CHANNEL40) |
        mac_get_ch_bit(MAC_CHANNEL44) | mac_get_ch_bit(MAC_CHANNEL48) | mac_get_ch_bit(MAC_CHANNEL52) |
        mac_get_ch_bit(MAC_CHANNEL56) | mac_get_ch_bit(MAC_CHANNEL60) | mac_get_ch_bit(MAC_CHANNEL64) |
        mac_get_ch_bit(MAC_CHANNEL100) | mac_get_ch_bit(MAC_CHANNEL104) | mac_get_ch_bit(MAC_CHANNEL108) |
        mac_get_ch_bit(MAC_CHANNEL112) | mac_get_ch_bit(MAC_CHANNEL116) | mac_get_ch_bit(MAC_CHANNEL120) |
        mac_get_ch_bit(MAC_CHANNEL124) | mac_get_ch_bit(MAC_CHANNEL128) | mac_get_ch_bit(MAC_CHANNEL132) |
        mac_get_ch_bit(MAC_CHANNEL136) | mac_get_ch_bit(MAC_CHANNEL140) | mac_get_ch_bit(MAC_CHANNEL144) |
        mac_get_ch_bit(MAC_CHANNEL149) | mac_get_ch_bit(MAC_CHANNEL153) | mac_get_ch_bit(MAC_CHANNEL157) |
        mac_get_ch_bit(MAC_CHANNEL161) | mac_get_ch_bit(MAC_CHANNEL165) | mac_get_ch_bit(MAC_CHANNEL184) |
        mac_get_ch_bit(MAC_CHANNEL188) | mac_get_ch_bit(MAC_CHANNEL192) | mac_get_ch_bit(MAC_CHANNEL196);
}

#ifdef _PRE_WLAN_FEATURE_11D
/*****************************************************************************
 函 数 名  : mac_set_country_ie_2g_etc
 功能描述  : 2G频段填写国家码
*****************************************************************************/
osal_u32 mac_set_country_ie_2g_etc(const mac_regdomain_info_stru *rd_info, osal_u8 *buffer, osal_u8 *len)
{
    osal_u8 rc_idx;
    osal_u8 lsb_bit_position;
    const mac_regclass_info_stru *reg_class = OSAL_NULL;
    osal_u32 ret;

    osal_u8 len_tmp = 0;

    for (rc_idx = 0; rc_idx < rd_info->regclass_num; rc_idx++) {
        /* 获取 Regulatory Class */
        reg_class = &(rd_info->regclass[rc_idx]);

        /* 如果频段不匹配 */
        if (reg_class->start_freq != MAC_RC_START_FREQ_2) {
            continue;
        }

        /* 异常检查，信道位图为0表示此管制域没有信道存在，不能少 */
        if (reg_class->channel_bmap == 0) {
            continue;
        }

        /* 获取信道位图的最低一位, 返回0代表bit0置1 */
        lsb_bit_position = oal_bit_find_first_bit_four_byte(reg_class->channel_bmap);

        /* 获取信道号，例如Channel_Map为1100，其对应的索引值为2与3，再由索引值找到信道号 */
        ret = hmac_get_channel_num_from_idx_etc(MAC_RC_START_FREQ_2, lsb_bit_position, &buffer[len_tmp++]);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{mac_set_country_ie_2g_etc::hmac_get_channel_num_from_idx_etc failed[%d].}",
                ret);
            return ret;
        }

        /* 获取信道数 */
        buffer[len_tmp++] = (osal_u8)oal_bit_get_num_four_byte(reg_class->channel_bmap);

        /* 获取最大功率 */
        buffer[len_tmp++] = reg_class->max_reg_tx_pwr;
    }

    *len = len_tmp;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_5G
/*****************************************************************************
 函 数 名  : mac_set_country_ie_5g_etc
 功能描述  : 5G频段填写国际码
*****************************************************************************/
osal_u32 mac_set_country_ie_5g_etc(const mac_regdomain_info_stru *rd_info, const osal_u8 *buffer, osal_u8 *len)
{
    mac_regclass_info_stru *reg_class = OSAL_NULL;
    mac_country_reg_field_stru *reg_field = OSAL_NULL;
    osal_u8 chan_idx;
    osal_u8 chan_num = 0;
    osal_u8 len_tmp = 0;

    unref_param(rd_info);
    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) {
        osal_u32 ret;
        reg_class = hmac_get_channel_idx_rc_info_etc(MAC_RC_START_FREQ_5, chan_idx);
        if (reg_class == OSAL_NULL) {
            continue;
        }

        ret = hmac_get_channel_num_from_idx_etc(MAC_RC_START_FREQ_5, chan_idx, &chan_num);
        if (ret != OAL_SUCC) {
            return OAL_FAIL;
        }

        reg_field = (mac_country_reg_field_stru *)buffer;

        reg_field->first_channel = chan_num;
        reg_field->channel_num = 1;
        reg_field->max_tx_pwr = reg_class->max_reg_tx_pwr;

        buffer += MAC_COUNTRY_REG_FIELD_LEN;

        len_tmp += MAC_COUNTRY_REG_FIELD_LEN;
    }

    *len = len_tmp;

    return OAL_SUCC;
}
#endif
#endif

/*****************************************************************************
 函 数 名  : hmac_init_channel_list_etc
 功能描述  : 依据管制域初始化信道列表
*****************************************************************************/
osal_void hmac_init_channel_list_etc(osal_void)
{
    osal_u8 ch_idx;
    osal_u8 rc_num;
    osal_u8 freq;
    osal_u8 rc_idx;
    mac_regdomain_info_stru *rd_info = OSAL_NULL;
    mac_regclass_info_stru *rc_info = OSAL_NULL;

    rd_info = &g_mac_regdomain;

    /* 先初始化所有信道的管制类为无效 */
    for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_2_BUTT; ch_idx++) {
        g_channel_list_2g[ch_idx].reg_class = MAC_INVALID_RC;
    }

#ifdef _PRE_WLAN_SUPPORT_5G
    for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_5_BUTT; ch_idx++) {
        g_channel_list_5g[ch_idx].reg_class = MAC_INVALID_RC;
    }
#endif

    /* 然后根据管制域更新信道的管制类信息 */
    rc_num = rd_info->regclass_num;

    /* 更新2G频段上信道的管制类信息 */
    freq = MAC_RC_START_FREQ_2;

    for (rc_idx = 0; rc_idx < rc_num; rc_idx++) {
        rc_info = &(rd_info->regclass[rc_idx]);

        for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_2_BUTT; ch_idx++) {
            if (mac_is_ch_supp_in_regclass(rc_info, freq, ch_idx) == OSAL_TRUE) {
                g_channel_list_2g[ch_idx].reg_class = rc_idx;
            }
        }
    }

#ifdef _PRE_WLAN_SUPPORT_5G
    /* 更新5G频段上信道的管制类信息 */
    freq = MAC_RC_START_FREQ_5;

    for (rc_idx = 0; rc_idx < rc_num; rc_idx++) {
        rc_info = &(rd_info->regclass[rc_idx]);

        for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_5_BUTT; ch_idx++) {
            if (mac_is_ch_supp_in_regclass(rc_info, freq, ch_idx) == OSAL_TRUE) {
                g_channel_list_5g[ch_idx].reg_class = rc_idx;
            }
        }
    }
#endif
}

/*****************************************************************************
 函 数 名  : hmac_get_channel_num_from_idx_etc
 功能描述  : 由信道索引值返回信道号
*****************************************************************************/
osal_u32 hmac_get_channel_num_from_idx_etc(osal_u8 band, osal_u8 idx, osal_u8 *channel_num)
{
    switch (band) {
        case MAC_RC_START_FREQ_2:
            if (idx >= MAC_CHANNEL_FREQ_2_BUTT) {
                return OAL_ERR_CODE_ARRAY_OVERFLOW;
            }

            *channel_num = g_channel_list_2g[idx].chan_number;
            break;

#ifdef _PRE_WLAN_SUPPORT_5G
        case MAC_RC_START_FREQ_5:
            if (idx >= MAC_CHANNEL_FREQ_5_BUTT) {
                return OAL_ERR_CODE_ARRAY_OVERFLOW;
            }

            *channel_num = g_channel_list_5g[idx].chan_number;
            break;
#endif

        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_get_channel_idx_from_num_etc
 功能描述  : 通过信道号找到信道索引号
*****************************************************************************/
osal_u32 hmac_get_channel_idx_from_num_etc(osal_u8 band, osal_u8 channel_num, osal_u8 *channel_idx)
{
    mac_channel_info_stru *channel = OSAL_NULL;
    osal_u8 total_channel_num = 0;
    osal_u8 idx;

    /* 根据频段获取信道信息 */
    switch (band) {
        case MAC_RC_START_FREQ_2:
            channel = g_channel_list_2g;
            total_channel_num = (osal_u8)MAC_CHANNEL_FREQ_2_BUTT;
            break;

#ifdef _PRE_WLAN_SUPPORT_5G
        case MAC_RC_START_FREQ_5:
            channel = g_channel_list_5g;
            total_channel_num = (osal_u8)MAC_CHANNEL_FREQ_5_BUTT;
            break;
#endif

        default:
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 检查信道索引号 */
    for (idx = 0; idx < total_channel_num; idx++) {
        if (channel[idx].chan_number == channel_num) {
            *channel_idx = idx;
            return OAL_SUCC;
        }
    }

    return OAL_ERR_CODE_INVALID_CONFIG;
}

/*****************************************************************************
 函 数 名  : hmac_is_channel_idx_valid_etc
 功能描述  : 根据管制域，判断信道索引号是否有效
*****************************************************************************/
osal_u32 hmac_is_channel_idx_valid_etc(osal_u8 band, osal_u8 ch_idx)
{
    if (hmac_get_channel_idx_rc_info_etc(band, ch_idx) == OSAL_NULL) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_is_channel_num_valid_etc
 功能描述  : 检测信道号是否合法
*****************************************************************************/
osal_u32 hmac_is_channel_num_valid_etc(osal_u8 band, osal_u8 ch_num)
{
    osal_u8 ch_idx;
    osal_u32 ret;

    ret = hmac_get_channel_idx_from_num_etc(band, ch_num, &ch_idx);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_is_channel_idx_valid_etc(band, ch_idx);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_get_channel_idx_rc_info_etc
 功能描述  : 获取1个信道索引的管制类信息
*****************************************************************************/
mac_regclass_info_stru *hmac_get_channel_idx_rc_info_etc(osal_u8 band, osal_u8 ch_idx)
{
    osal_u8 max_ch_idx;
    mac_channel_info_stru *ch_info = OSAL_NULL;

    switch (band) {
        case MAC_RC_START_FREQ_2:
            max_ch_idx = MAC_CHANNEL_FREQ_2_BUTT;
            ch_info = &(g_channel_list_2g[ch_idx]);
            break;

#ifdef _PRE_WLAN_SUPPORT_5G
        case MAC_RC_START_FREQ_5:
            max_ch_idx = MAC_CHANNEL_FREQ_5_BUTT;
            ch_info = &(g_channel_list_5g[ch_idx]);
            break;
#endif

        default:
            return OSAL_NULL;
    }

    if (ch_idx >= max_ch_idx) {
        return OSAL_NULL;
    }

    if (ch_info->reg_class == MAC_INVALID_RC) {
        return OSAL_NULL;
    }

    return &(g_mac_regdomain.regclass[ch_info->reg_class]);
}

/*****************************************************************************
 函 数 名  : hmac_get_channel_num_rc_info_etc
 功能描述  : 获取信道号管制类信息
*****************************************************************************/
mac_regclass_info_stru *hmac_get_channel_num_rc_info_etc(osal_u8 band, osal_u8 ch_num)
{
    osal_u8 channel_idx;

    if (hmac_get_channel_idx_from_num_etc(band, ch_num, &channel_idx) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY,
            "{hmac_get_channel_num_rc_info_etc::hmac_get_channel_idx_from_num_etc failed. band:%d, ch_num:%d", band,
            ch_num);

        return OSAL_NULL;
    }

    return hmac_get_channel_idx_rc_info_etc(band, channel_idx);
}

/*****************************************************************************
 函 数 名  : ddmac_regdomain_set_max_power_etc
 功能描述  : 设置管制域最大发送功率
*****************************************************************************/
osal_u32 hmac_regdomain_set_max_power_etc(osal_u32 pwr)
{
    osal_u8 rc_idx;
    for (rc_idx = 0; rc_idx < g_mac_regdomain.regclass_num; rc_idx++) {
        g_mac_regdomain.regclass[rc_idx].max_tx_pwr = (osal_u16)pwr * 10; /* 10 */
        g_mac_regdomain.regclass[rc_idx].max_reg_tx_pwr = (osal_u8)pwr;
        oam_warning_log3(0, OAM_SF_TPC, "pwr[%d] exceed reg_tx_pwr[%d], rc_idx[%d]",
            pwr, g_mac_regdomain.regclass[rc_idx].max_reg_tx_pwr, rc_idx);
    }

    return OAL_SUCC;
}

osal_s32 hmac_config_get_channel_list(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_regdomain_info_stru *rd_info = OSAL_NULL;
    mac_regclass_info_stru *rc_info = OSAL_NULL;
    osal_u8 ch_idx;
    osal_u8 rc_num;
    osal_u8 freq;
    osal_u8 rc_idx;
    osal_u8 chan_num;
    unref_param(hmac_vap);
    unref_param(msg);
    hmac_get_regdomain_info_etc(&rd_info);
    if (rd_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_get_channel_list::get_regdomain_info fail.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    rc_num = rd_info->regclass_num;

    freq = MAC_RC_START_FREQ_2;
    wifi_printf("2G ch\r\n");

    for (rc_idx = 0; rc_idx < rc_num; rc_idx++) {
        rc_info = &(rd_info->regclass[rc_idx]);

        for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_2_BUTT; ch_idx++) {
            if (mac_is_ch_supp_in_regclass(rc_info, freq, ch_idx) == OSAL_TRUE) {
                hmac_get_channel_num_from_idx_etc(WLAN_BAND_2G, ch_idx, &chan_num);
                wifi_printf("%d\r\n", chan_num);
            }
        }
    }
#ifdef _PRE_WLAN_SUPPORT_5G
    /* 更新5G频段上信道的管制类信息 */
    freq = MAC_RC_START_FREQ_5;
    wifi_printf("5G ch\r\n");
    for (rc_idx = 0; rc_idx < rc_num; rc_idx++) {
        rc_info = &(rd_info->regclass[rc_idx]);

        for (ch_idx = 0; ch_idx < MAC_CHANNEL_FREQ_5_BUTT; ch_idx++) {
            if (mac_is_ch_supp_in_regclass(rc_info, freq, ch_idx) == OSAL_TRUE) {
                hmac_get_channel_num_from_idx_etc(WLAN_BAND_5G, ch_idx, &chan_num);
                wifi_printf("%d\r\n", chan_num);
            }
        }
    }
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_country_etc
 功能描述  : hmac设置国家码
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_country_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_country_stru *country_param = (mac_cfg_country_stru *)msg->data;
    mac_regdomain_info_stru *mac_regdom = (mac_regdomain_info_stru *)country_param->mac_regdom;
    hmac_device_stru *mac_device = OAL_PTR_NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
#endif

    if (mac_regdom == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap[%d]hmac_config_set_country_etc::mac_regdom null.", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(mac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap[%d]hmac_config_set_country_etc::hmac_device null.", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log4(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_country_etc::country code=%c%c sideband_flag[%d].}",
        hmac_vap->vap_id, (osal_u8)mac_regdom->ac_country[0], (osal_u8)mac_regdom->ac_country[1],
        mac_regdom->sideband_country);
    hmac_regdomain_set_country_etc(msg->data_len, msg->data);
#ifdef _PRE_WLAN_FEATURE_DFS
    /* 只有5G 芯片才进行雷达信道初始化 */
    oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_country_etc::hmac_dfs_channel_list_init_etc.}",
                     hmac_vap->vap_id);
    hmac_dfs_channel_list_init_etc(mac_device);
    hmac_config_ch_status_sync(mac_device);
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_device = hal_chip_get_hal_device();
    /* 更新FCC认证要求国家标志位 */
    hal_device->sideband_flag = mac_regdom->sideband_country;
#endif

    oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_country::succ set country}", hmac_vap->vap_id);
    /* 释放内存 */
    oal_mem_free(mac_regdom, OAL_FALSE);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_country_etc
 功能描述  : hmac读取国际码
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_get_country_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    errno_t ret_err;
    mac_cfg_get_country_stru param;
    mac_regdomain_info_stru *regdomain_info = OAL_PTR_NULL;
    unref_param(hmac_vap);
    hmac_get_regdomain_info_etc(&regdomain_info);
    if (regdomain_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_get_channel_list::get_regdomain_info fail.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret_err = memcpy_s(param.ac_country, sizeof(param.ac_country), regdomain_info->ac_country,
        sizeof(regdomain_info->ac_country));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_config_get_country_etc::memcpy_s failed.}");
        return OAL_FAIL;
    }

    ret_err = memcpy_s(msg->rsp, sizeof(mac_cfg_get_country_stru), &param, sizeof(param));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_config_get_country_etc::memset_s failed.}");
        return OAL_FAIL;
    }
    msg->rsp_len = OAL_SIZEOF(mac_cfg_get_country_stru);
    oam_warning_log2(0, OAM_SF_CFG, "hmac_config_get_country_etc::country code is %d %d\r\n", param.ac_country[0],
        param.ac_country[1]);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_regdomain_pwr_etc
 功能描述  : 设置管制域最大发送功率
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_regdomain_pwr_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);

    hmac_regdomain_set_max_power_etc(*(osal_u32 *)(msg->data));

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_regdomain_pwr_etc
 功能描述  : 读取管制域最大发送功率
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_get_regdomain_pwr_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_regdomain_info_stru *regdomain_info = OAL_PTR_NULL;
    osal_u8 index;
    unref_param(hmac_vap);
    unref_param(msg);
    hmac_get_regdomain_info_etc(&regdomain_info);
    for (index = 0; index < regdomain_info->regclass_num; index++) {
        /* 不支持5G的情况下, 5G的功率信息不打印 */
#ifndef _PRE_WLAN_SUPPORT_5G
        if (regdomain_info->regclass[index].start_freq >= MAC_RC_START_FREQ_5) {
            continue;
        }
#endif
        wifi_printf("regclass[%d] max_tx_pwr[%d] max_regclass_tx_pwr[%d]\r\n", index,
            regdomain_info->regclass[index].max_tx_pwr,
            regdomain_info->regclass[index].max_reg_tx_pwr);
    }
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
