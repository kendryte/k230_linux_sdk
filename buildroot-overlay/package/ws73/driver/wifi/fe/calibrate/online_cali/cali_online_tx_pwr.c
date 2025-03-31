/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Interface implementation of online tx power calibration.
 * Create: 2022-8-12
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_phy.h"
#include "hal_device.h"
#include "fe_power_host.h"
#include "wlan_msg.h"
#include "frw_ext_if.h"
#include "cali_online.h"
#include "fe_log_calc_rom.h"
#include "power_ctrl_spec.h"
#include "fe_hal_gp_if.h"
#include "fe_rf_customize_power.h"
#include "fe_rf_customize_power_cali.h"
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
#include "fe_equipment.h"
#endif // _PRE_WLAN_FEATURE_MFG_TEST
#include "fe_hal_rf_if_temperate.h"
#ifdef _PRE_WLAN_ONLINE_CALI
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_CALI_ONLINE_TX_PWR
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ONLINE_CALI_NUM_OF_10POWER_NEG_TABLE           (260)         /* 10^(-N/20)线性表 */
#define ONLINE_CALI_NUM_OF_10POWER_POS_TABLE           (200)         /* 10^(N/20)线性表 */
#define ONLINE_CALI_TEMP_PRECISION_FAC                 (10000)       /* 动态校准温度补偿系数放大因子 */
#define ONLINE_CALI_INVALID_TEMP_CODE                  (-10000)      /* 动态校准温度码字非法值 */
#define ONLINE_CALI_TEMP_POW_200                       (200)

#define ONLINE_CALI_PDET_BIT                           (13)
#define ONLINE_CALI_PDET_HALF_MAX_VAL                  ((1 << (ONLINE_CALI_PDET_BIT - 1)) - 1)
#define ONLINE_CALI_PDET_MAX_VAL                       (1 << ONLINE_CALI_PDET_BIT)
#define ONLINE_CALI_PWR_CHECK_TIMES                    (5)   /* 异常功率累积次数 */
#define ONLINE_CALI_DIFF_RATE_CENTUPLICATE             (80)  /* 偏差比例 80倍 */
#define ONLINE_CALI_COEFF_AMP                          (100) /* 放大系数 */
#define ONLINE_CALI_MAX_DYN_TPC_CODE                   (26) /* tpc code>26(power=10dBm)不做校准 */
/*****************************************************************************
    全局变量定义
*****************************************************************************/
online_cali_pow_ctrl_stru g_txpow_ctrl[CALI_STREAM_TYPE_BUTT] = {0};

/* 10^(-N/20)*1024线性表，输入参数为0~259, 精度0.1db */
const osal_u16 g_aus_linear_to_10power_neg_lut[ONLINE_CALI_NUM_OF_10POWER_NEG_TABLE] = {
    1024, 1012, 1001, 989, 978, 967, 956, 945, 934, 923,
    913, 902, 892, 882, 872, 862, 852, 842, 832, 823,
    813, 804, 795, 786, 777, 768, 759, 750, 742, 733,
    725, 717, 708, 700, 692, 684, 677, 669, 661, 654,
    646, 639, 631, 624, 617, 610, 603, 596, 589, 583,
    576, 569, 563, 556, 550, 544, 537, 531, 525, 519,
    513, 507, 502, 496, 490, 485, 479, 473, 468, 463,
    457, 452, 447, 442, 437, 432, 427, 422, 417, 412,
    408, 403, 398, 394, 389, 385, 380, 376, 372, 368,
    363, 359, 355, 351, 347, 343, 339, 335, 331, 328,
    324, 320, 316, 313, 309, 306, 302, 299, 295, 292,
    289, 285, 282, 279, 276, 272, 269, 266, 263, 260,
    257, 254, 251, 248, 246, 243, 240, 237, 235, 232,
    229, 227, 224, 221, 219, 216, 214, 211, 209, 207,
    204, 202, 200, 197, 195, 193, 191, 188, 186, 184,
    182, 180, 178, 176, 174, 172, 170, 168, 166, 164,
    162, 160, 159, 157, 155, 153, 151, 150, 148, 146,
    145, 143, 141, 140, 138, 137, 135, 133, 132, 130,
    129, 127, 126, 125, 123, 122, 120, 119, 118, 116,
    115, 114, 112, 111, 110, 108, 107, 106, 105, 104,
    102, 101, 100, 99, 98, 97, 96, 94, 93, 92,
    91, 90, 89, 88, 87, 86, 85, 84, 83, 82,
    81, 80, 79, 79, 78, 77, 76, 75, 74, 73,
    72, 72, 71, 70, 69, 68, 68, 67, 66, 65,
    65, 64, 63, 62, 62, 61, 60, 60, 59, 58,
    58, 57, 56, 56, 55, 54, 54, 53, 53, 52
};

/* 10^(N/20)*1024线性表，输入参数为0~200, 精度0.1db */
const osal_u16 g_aus_linear_to_10power_pos_lut[ONLINE_CALI_NUM_OF_10POWER_POS_TABLE] = {
    1024,   1036,   1048,   1060,   1072,   1085,   1097,   1110,   1123,   1136,
    1149,   1162,   1176,   1189,   1203,   1217,   1231,   1245,   1260,   1274,
    1289,   1304,   1319,   1334,   1350,   1366,   1381,   1397,   1414,   1430,
    1446,   1463,   1480,   1497,   1515,   1532,   1550,   1568,   1586,   1604,
    1623,   1642,   1661,   1680,   1699,   1719,   1739,   1759,   1780,   1800,
    1821,   1842,   1863,   1885,   1907,   1929,   1951,   1974,   1997,   2020,
    2043,   2067,   2091,   2115,   2139,   2164,   2189,   2215,   2240,   2266,
    2292,   2319,   2346,   2373,   2400,   2428,   2456,   2485,   2514,   2543,
    2572,   2602,   2632,   2663,   2693,   2725,   2756,   2788,   2820,   2853,
    2886,   2919,   2953,   2987,   3022,   3057,   3092,   3128,   3164,   3201,
    3238,   3276,   3314,   3352,   3391,   3430,   3470,   3510,   3551,   3592,
    3633,   3675,   3718,   3761,   3805,   3849,   3893,   3938,   3984,   4030,
    4077,   4124,   4172,   4220,   4269,   4318,   4368,   4419,   4470,   4522,
    4574,   4627,   4681,   4735,   4790,   4845,   4901,   4958,   5015,   5073,
    5132,   5192,   5252,   5313,   5374,   5436,   5499,   5563,   5627,   5692,
    5758,   5825,   5893,   5961,   6030,   6100,   6170,   6242,   6314,   6387,
    6461,   6536,   6611,   6688,   6766,   6844,   6923,   7003,   7084,   7166,
    7249,   7333,   7418,   7504,   7591,   7679,   7768,   7858,   7949,   8041,
    8134,   8228,   8323,   8420,   8517,   8616,   8716,   8817,   8919,   9022,
    9126,   9232,   9339,   9447,   9557,   9667,   9779,   9892,  10007,  10123
};

/*****************************************************************************
    函数定义
*****************************************************************************/
OSAL_STATIC osal_s16 online_cali_pow_div_round(osal_s16 n, osal_s16 d)
{
    return ((n >= 0) ? ((n + (d / (osal_s16)BIT1)) / d) : ((n - (d / (osal_s16)BIT1)) / d));
}
OSAL_STATIC osal_u32 online_cali_pow_right_shift_floor(osal_u32 data, osal_u32 num)
{
    return ((data > 1) ? ((data - 1) >> num) : 0);
}

/*
 * 函 数 名  : online_cali_send_intvl
 * 功能描述  : 立即发送帧间隔
 */
OSAL_STATIC osal_u32 online_cali_send_intvl(online_cali_stru *rf_cali)
{
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    online_cali_pow_notify_para_stru data = {0};

    /* send tx notify */
    data.enable_online_cali_mask = ONLINE_CALI_STATE_DYN_PWR;
    data.cali_band = dyn_cali->dyn_cali_complete_band;
    data.aus_cali_en_interval = dyn_cali->dyn_cali_en_intvl[data.cali_band];
    data.interval_available = 1;
    online_rf_cali_handle_event(rf_cali, ONLINE_CALI_EVENT_TX_NOTIFY, sizeof(data), (osal_u8*)&data);
    return OAL_SUCC;
}

/*
 * 函 数 名  : online_cali_per_frame_timeout_handler
 * 功能描述  : tx notify超时发送帧间隔
 */
OSAL_STATIC osal_u32 online_cali_per_frame_timeout_handler(osal_void *prg)
{
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    unref_param(prg);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log0(0, OAM_SF_CALIBRATE, "{online_cali_per_frame_timeout_handler::no online cali 10sec timeout}\n");
#endif
    /* 发送tx notify */
    online_cali_send_intvl(rf_cali);
    return OAL_SUCC;
}

/*
 * 函 数 名  : online_cali_intvl_timeout_handler
 * 功能描述  : 动态校准未收敛时发送帧间隔
 */
OSAL_STATIC osal_u32 online_cali_intvl_timeout_handler(osal_void *prg)
{
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    unref_param(prg);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log0(0, OAM_SF_CALIBRATE, "{online_cali_intvl_timeout_handler: interval 200 ms timeout.}\n");
#endif
    /* 发送tx notify */
    online_cali_send_intvl(rf_cali);
    return OAL_SUCC;
}

online_cali_pow_ctrl_stru *online_cali_get_txpow_ctrl(osal_u8 chain)
{
    return &g_txpow_ctrl[chain];
};

/*
 * 功能描述   : online校准进入pow状态时配置
 * 1.日    期   : 2020年07月17日
 *   修改内容   : 新生成函数
 */
osal_void online_cali_dyn_pow_state_entry(osal_void *p_ctx)
{
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    hal_device_stru *device = (hal_device_stru *)hal_chip_get_hal_device();
    unref_param(p_ctx);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log0(0, OAM_SF_CALIBRATE, "{PWR entry}\n");
#endif
    /* 设置状态机事件的ID号 */
    online_set_cali_id(rf_cali, 0, rf_cali->uc_device_id);
    /* 配置动态校准寄存器,累加器参数 */
    fe_hal_phy_config_online_pow(device);
    frw_create_timer_entry(&(rf_cali->dyn_cali_val.dyn_cali_intvl_timer), online_cali_intvl_timeout_handler,
        ONLINE_CALI_INTVL_TIMER_PERIOD, device, OAL_FALSE);
    return;
}

/*
 * 功能描述   : online校准退出pow状态时恢复寄存器配置
 * 1.日    期   : 2020年07月17日
 *   修改内容   : 新生成函数
 */
osal_void online_cali_dyn_pow_state_exit(osal_void *p_ctx)
{
    hal_device_stru *device = (hal_device_stru *)hal_chip_get_hal_device();
    unref_param(p_ctx);

    /* 恢复动态校准寄存器,累加器参数 */
    fe_hal_phy_recover_online_pow(device);
    return;
}

/*
 * 功能描述  : 判断当前动态功率校准是否使能
 * 1.日    期  : 2020年07月27日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_u32 online_cali_state_is_in_pwr(const online_cali_stru *rf_cali)
{
    return (rf_cali->st_online_tx_cali_fsm.st_oal_fsm.cur_state == ONLINE_CALI_STATE_DYN_PWR);
}

/*
 * 函 数 名  : online_cali_get_pwr_dist_table
 * 功能描述  : 获取功率增益分配表
 */
OSAL_STATIC osal_u8 online_cali_get_pwr_dist_table(hal_device_stru *device,
    wlan_channel_band_enum_uint8 band, online_cali_para_stru *online_cali, cali_pow_cal_gain_stru **pow_dist_table,
    oal_bool_enum_uint8 is_dsss)
{
    osal_u8 table_len;
    unref_param(device);

    if (is_dsss) {
        table_len = CALI_POW_CAL_2G_DSSS_TABLE_LEN;
        *pow_dist_table = cali_get_pow_cal_gain_2g_table(CALI_POW_TABLE_DSSS);
    } else {
        table_len = CALI_POW_CAL_2G_OFDM_TABLE_LEN;
        *pow_dist_table = cali_get_pow_cal_gain_2g_table(CALI_POW_TABLE_OFDM);
    }

    return table_len;
}

/*
 * 函 数 名  : online_cali_dyn_cali_pwr_valid
 * 功能描述  : 检查校准功率的有效性
 */
OSAL_STATIC oal_bool_enum_uint8 online_cali_dyn_cali_pwr_valid(hal_device_stru *device,
    mac_channel_stru channel_info, const osal_s16 *const ps_tx_pow)
{
    wlan_channel_band_enum_uint8 band = channel_info.band;
    osal_u8 base_power;
    unref_param(device);

    base_power = fe_custom_get_chip_max_power(band);
    if ((*ps_tx_pow > (base_power + CALI_DYN_POW_MAX)) || (*ps_tx_pow < base_power + CALI_DYN_POW_MIN)) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_error_log3(0, OAM_SF_CALIBRATE, "{online_cali_dyn_cali_pwr_valid::tx pow[%d]up limit[%d]low limit[%d]}\n",
            *ps_tx_pow, base_power, base_power + CALI_DYN_POW_MIN);
#endif
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

OAL_STATIC osal_u8 online_cali_is_dsss(const online_cali_para_stru *online_cali)
{
#ifdef BOARD_FPGA_WIFI
    unref_param(online_cali);
    return OSAL_FALSE;
#else
    return (online_cali->tpc_code >= CALI_POW_CAL_2G_OFDM_TABLE_LEN) ? OSAL_TRUE : OSAL_FALSE;
#endif
}

/*
 * 函 数 名  : online_cali_get_target_tx_power
 * 功能描述  : 查表获得期望功率
 */
OSAL_STATIC osal_u32 online_cali_get_target_tx_power(hal_device_stru *device,
    online_cali_para_stru *online_cali, osal_s16 *exp_pow)
{
    cali_pow_cal_gain_stru *pow_dist_table = NULL;
    wlan_channel_band_enum_uint8 band = online_cali->channel_info.band;
    const cali_pow_distri_ratio_stru *pow_dist = NULL;
    oal_bool_enum_uint8 is_dsss = online_cali_is_dsss(online_cali);
    osal_u8 base_power;
    osal_u8 table_len;
    osal_u8 pow_idx = 0;

    /* 获取基准最大发射功率 */
    base_power = fe_custom_get_chip_max_power(band);

    /* 读取各pow_dist增益表 */
    table_len = online_cali_get_pwr_dist_table(device, band, online_cali, &pow_dist_table, is_dsss);

    /* 获取对应的档位增益 */
    while ((pow_idx < table_len) && ((pow_dist_table + pow_idx)->tpc_idx != online_cali->tpc_code)) {
        pow_idx++;
    }
    /* 未找到有效的pow idx,使用默认值 */
    if (pow_idx == table_len) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_error_log4(0, OAM_SF_CALIBRATE,
            "{online_cali_get_target_tx_power::Band[%d]table_len[%d]tpc_code[%d]rate_id[%d]}\n",
            band, table_len, online_cali->tpc_code, online_cali->rate_idx);
#endif
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    pow_dist_table += pow_idx;
    pow_dist = pow_dist_table->pow_dist;
    online_cali->pwr_dist = *(osal_u32 *)(uintptr_t)pow_dist;
    online_cali->upc_idx = pow_dist->ppa_idx;
    *exp_pow = (osal_s16)base_power + pow_dist_table->pow_gain;
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log4(0, OAM_SF_CALIBRATE, "{ppa_code[0x%x],txpow[%d]=base pow[%d]+pow_gain[%d]}\n",
        fe_hal_phy_get_single_ppa_lut(device, online_cali->upc_idx), *exp_pow, base_power, pow_dist_table->pow_gain);
#endif
    /* 计算power合理性检查 */
    if (online_cali_dyn_cali_pwr_valid(device, online_cali->channel_info, exp_pow) == OAL_FALSE) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : online_cali_dyn_pdet_check
 * 功能描述  : 动态校准上报异常检测
 */
OSAL_STATIC osal_u32 online_cali_dyn_pdet_check(online_cali_dyn_stru *dyn_cali, osal_s32 pdet_val)
{
    static osal_u8 cnt = 0;
    osal_u32 ret = OAL_SUCC;
    osal_u8 chain_idx = dyn_cali->dyn_cali_complete_chain;
    online_cali_info_stru *dyn_cali_pow = &dyn_cali->dyn_cali_pow_info[chain_idx];

    /* pdet异常保护，防止pdet上报值不变导致upc一直变化 */
    if (pdet_val == dyn_cali_pow->previous_pdet_val) {
        cnt++;
        if (cnt >= ONLINE_CALI_ACCU_ABNORMAL_CNT) {
            oam_warning_log2(0, OAM_SF_CALIBRATE, "{online_cali_dyn_pdet_check::pdet_val=%d cnt[%d]}", pdet_val, cnt);
            cnt = 0;
            ret = OAL_FAIL;
        }
    } else {
        cnt = 0;
    }

    dyn_cali_pow->previous_pdet_val = pdet_val;
    return ret;
}

OSAL_STATIC osal_u8 online_cali_is_need_flush_pdet_en(online_cali_stru *rf_cali)
{
    online_cali_dyn_stru *dyn_cali_val = &rf_cali->dyn_cali_val;
    wlan_channel_band_enum_uint8 band = dyn_cali_val->dyn_cali_complete_band;

    /*  动态校准未使能，或者状态机不是work态的情况下不进行动态校准 */
    if ((dyn_cali_val->dyn_cali_en_intvl[band] == 0) || online_cali_state_is_in_pwr(rf_cali) == OAL_FALSE) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_error_log3(0, OAM_SF_CALIBRATE, "{online_cali_is_need_flush_pdet_en: inteval[%d]band[%d]state[%d]}\n",
            dyn_cali_val->dyn_cali_en_intvl[band], band, rf_cali->st_online_tx_cali_fsm.st_oal_fsm.cur_state);
#endif
        return OAL_FALSE;
    }
    return OAL_TRUE;
}
osal_void online_cali_init_per_frame_timer(hal_to_dmac_device_stru *device)
{
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    online_cali_dyn_stru *dyn_cali_val = &rf_cali->dyn_cali_val;
    if (rf_cali->st_online_tx_cali_fsm.st_oal_fsm.cur_state != ONLINE_CALI_STATE_DYN_PWR) {
        return;
    }
    frw_timer_stop_timer(&(dyn_cali_val->dyn_cali_per_frame_timer));
    if (device->al_tx_flag == 0) {
        frw_create_timer_entry(&(dyn_cali_val->dyn_cali_per_frame_timer), online_cali_per_frame_timeout_handler,
            ONLINE_CALI_PER_FRAME_TIMER_PERIOD, device, OAL_FALSE);
    } else {
        frw_create_timer_entry(&(dyn_cali_val->dyn_cali_per_frame_timer), online_cali_per_frame_timeout_handler,
            ONLINE_CALI_INTVL_TIMER_PERIOD, device, OAL_FALSE); /* 超时缩短至200ms, 避免产线校准延迟 */
    }
}
/*
 * 函 数 名   : online_cali_pow_tx_notify
 * 功能描述   : 动态功率校准判断
 */
OSAL_STATIC osal_u32 online_cali_pow_tx_notify(online_cali_stru *rf_cali, osal_void *param)
{
    hal_device_stru *device = (hal_device_stru *)hal_chip_get_hal_device();
    online_cali_pow_notify_para_stru *receive_data = (online_cali_pow_notify_para_stru *)param;
    online_cali_pow_notify_para_stru send_data = {0};
    frw_msg msg = {0};
    size_t len = 0;

    /* 条件不满足时，输出FALSE，用于刷新pdet_en */
    if (online_cali_is_need_flush_pdet_en(rf_cali) == OAL_FALSE) {
        return OAL_FALSE;
    }
    send_data.enable_online_cali_mask = receive_data->enable_online_cali_mask; /* 动态功率使能 */
    send_data.aus_cali_en_interval = receive_data->aus_cali_en_interval;
    send_data.cali_band =  receive_data->cali_band;
    send_data.interval_available = receive_data->interval_available;           /* 帧间隔使能 */
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log2(0, OAM_SF_CALIBRATE, "{online_cali_pow_tx_notify::state[%d]invertal[%d]}\n",
        rf_cali->st_online_tx_cali_fsm.st_oal_fsm.cur_state, receive_data->aus_cali_en_interval);
#endif
    msg.data = (osal_u8 *)&send_data;
    len = sizeof(send_data);
    msg.data_len = (osal_u16)len;
    /* 动态校准帧定时器 */
    online_cali_init_per_frame_timer((hal_to_dmac_device_stru *)device);
    if (device->hal_device_base.work_vap_bitmap == 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{online_cali_pow_tx_notify::dev no vap up!}");
        return OAL_FAIL;
    }
    if (frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_FLUSH_DYN_MASK, &msg, OSAL_FALSE) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{online_cali_pow_tx_notify::rts_config_param::failed}");
        return OAL_FAIL;
    }
    fe_hal_phy_config_online_pow(device);
    return OAL_TRUE;
}

/*
 * 函 数 名  : online_cali_get_pow_cali_param
 * 功能描述  : 动态校准参数获取
 */
OSAL_STATIC osal_void online_cali_get_pow_cali_param(hal_device_stru *device,
    online_cali_dyn_stru *dyn_cali, online_cali_para_stru *online_cali)
{
    unref_param(device);
    dyn_cali->dyn_cali_complete_chain = online_cali->chain_idx;
    dyn_cali->dyn_cali_complete_band = online_cali->channel_info.band;
    dyn_cali->dc_offset[0] = online_cali->dc_offset[0];
    dyn_cali->dc_offset[1] = online_cali->dc_offset[1];
}

/*
 * 函 数 名  : online_cali_search_dbb_shadow_index
 * 功能描述  : 根据边界，判断dbb shadow档位
 */
OSAL_STATIC osal_u8 online_cali_search_dbb_shadow_index(const hal_device_stru *device, osal_u32 tpc_code)
{
    osal_u8 i;
    osal_u8 bound_previous;
    osal_u8 bound;
    osal_u8 real_tpc_code = tpc_code;

    if (real_tpc_code >= CALI_POW_CAL_2G_OFDM_TABLE_LEN) {
        real_tpc_code -= CALI_POW_CAL_2G_OFDM_TABLE_LEN;
    }
    unref_param(device);
    /* dbb shadow档位区间为[bound_previous, bound) */
    for (i = 0; i < CALI_DBB_DOUNDS; i++) {
        bound_previous = (i > 0) ? cali_get_dbb_shadow_bounds(i - 1) : 0;
        bound = cali_get_dbb_shadow_bounds(i);
        if (i == 0 && real_tpc_code < bound) {
            return 0;
        }
        if (i > 0 && real_tpc_code >= bound_previous && real_tpc_code < bound) {
            return i;
        }
    }
    return CALI_DBB_DOUNDS;
}

/*
 * 函 数 名: online_cali_init_adjust_dbb_scaling
 * 功能描述  : 初始化dbb scaling调整
 */
OSAL_STATIC osal_void online_cali_init_adjust_dbb_scaling(hal_device_stru *device,
    const online_cali_para_stru *online_cali, osal_u8 stream_type)
{
    osal_u8 dbb_shadow_index = 0;
    osal_u32 tpc_code = online_cali->tpc_code;
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();

    rf_cali->dyn_cali_val.dyn_cali_pow_info[stream_type].cali_pdet_adjust_flag = HAL_DYN_CALI_PDET_ADJUST_INIT;
    rf_cali->dyn_cali_val.dyn_cali_pow_info[stream_type].delt_dbb_scale_dyn_val = 0;
    dbb_shadow_index = online_cali_search_dbb_shadow_index(device, tpc_code);
    fe_hal_cfg_dbb_scale_shadow(device, ONLINE_CALI_DBB_SCALING_INIT_VAL, dbb_shadow_index);
}

/*
 * 函 数 名: online_cali_get_digital_scaling_reg
 * 功能描述  : 初始化dbb scaling调整
 */
OSAL_STATIC osal_u32 online_cali_get_digital_scaling_reg(hal_device_stru *device, online_cali_para_stru *online_cali,
    osal_u8 stream_type)
{
    osal_u8 dbb_shadow_index = 0;
    unref_param(stream_type);

    dbb_shadow_index = online_cali_search_dbb_shadow_index(device, online_cali->tpc_code);
    return fe_hal_get_dbb_scale_shadow(device, dbb_shadow_index);
}

/*
 * 函 数 名: online_cali_adjust_dbb_scaling
 * 功能描述  : 根据期望pdet和实际pdet差值调整dbb scaling
 *             1、先调整UPC，取upc刚好预期值的临界点
 *             2、再调整dbb scaling值
 *             3、dbb scaling的调整保证在变化范围内进行
 */
OSAL_STATIC osal_void online_cali_adjust_dbb_scaling(hal_device_stru *device, online_cali_para_stru *online_cali,
    osal_u8 stream_type, osal_s32 pdet_diff, osal_s32 cur_dbb)
{
    /* 调整只对1dB内的偏差 每次只调整1 */
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    osal_s16 dbb_scaling_adjust_step = ONLINE_CALI_DBB_SCALING_MIN_STEP;
    osal_u32 tpc_code = online_cali->tpc_code;
    osal_u8 dbb_shadow_index = 0;
    osal_u8 new_dbb = 0;

    /*  差值超过阈值,向上调整时不超过最大upc code，向下调整异常翻转保护 */
    if ((pdet_diff > 0) && (cur_dbb - dbb_scaling_adjust_step >
        ONLINE_CALI_DBB_SCALING_MIN_TH + ONLINE_CALI_DBB_SCALING_INIT_VAL)) {
        new_dbb = (osal_u8)(cur_dbb - dbb_scaling_adjust_step);
    } else if ((pdet_diff < 0) && (cur_dbb + dbb_scaling_adjust_step <
        ONLINE_CALI_DBB_SCALING_MAX_TH + ONLINE_CALI_DBB_SCALING_INIT_VAL)) {
        new_dbb = (osal_u8)(cur_dbb + dbb_scaling_adjust_step);
    } else {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_warning_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_adjust_dbb_scaling::abort pdet_diff[%d] real cur_dbb[%d]}", pdet_diff, cur_dbb);
#endif
        online_cali_init_adjust_dbb_scaling(device, online_cali, stream_type);
        return;
    }
    /* 调整功率变化的补偿Scaling值 */
    rf_cali->dyn_cali_val.dyn_cali_pow_info[stream_type].delt_dbb_scale_dyn_val = new_dbb;
    dbb_shadow_index = online_cali_search_dbb_shadow_index(device, tpc_code);
    fe_hal_cfg_dbb_scale_shadow(device, new_dbb, dbb_shadow_index);
    /* 调整功率变化的补偿Scaling值 */
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log3(0, OAM_SF_CALIBRATE, "{online_cali_adjust_dbb_scaling::new_dbb[%d] step[%x] pdet_diff[%d]}",
        new_dbb, dbb_scaling_adjust_step, pdet_diff);
#endif
}

/*
 * 函 数 名  : online_cali_record_union_by_tx_complete
 * 功能描述  : 通过tx complete信息获取dyn_cali_record信息
 */
OSAL_STATIC osal_void online_cali_record_union_by_tx_complete(online_cali_record_union *dyn_cali_info,
    online_cali_para_stru *online_cali, osal_s16 tx_pow)
{
    dyn_cali_info->para = 0x0;
    dyn_cali_info->rf_core_para.freq = online_cali->channel_info.band;
    dyn_cali_info->rf_core_para.channel = online_cali->channel_info.chan_number;
    dyn_cali_info->rf_core_para.bw = online_cali->channel_info.en_bandwidth;
    dyn_cali_info->rf_core_para.tx_pow = tx_pow;
    /* 更新调制方式 */
    dyn_cali_info->rf_core_para.mod = online_cali_is_dsss(online_cali);
}

OSAL_STATIC osal_u32 online_cali_check_pwr_not_need_calc(osal_u64 old_para, osal_u64 new_para, osal_s32 expect_pow)
{
    return ((old_para == new_para) && (expect_pow != 0));
}

// 得到i q两路pdt 13bit有符号数
OSAL_STATIC osal_void online_cali_tx_pow_get_iq_pdet(osal_s32 l_real_pdet, osal_s32 *pl_i_pdet, osal_s32 *pl_q_pdet)
{
    cali_online_pdet_stru real_pdet = {0};
    real_pdet.u32 = (osal_u32)l_real_pdet;
    *pl_i_pdet = real_pdet.bits.i;
    *pl_q_pdet = real_pdet.bits.q;
}

/*
 * 函 数 名   : online_cali_check_invalid_tx_pwr
 * 功能描述   : 调用到该函数，已连续出现异常，返回结果代表是否进行异常处理
 */
OSAL_STATIC osal_u32 online_cali_check_invalid_tx_pwr(online_cali_dyn_stru *dyn_cali)
{
    osal_u8 chain_idx = dyn_cali->dyn_cali_complete_chain;
    if (dyn_cali->pow_too_low_times[chain_idx] >= ONLINE_CALI_PWR_CHECK_TIMES) {
        oam_warning_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_check_invalid_tx_pwr::chain[%d], pow_too_low_times[%d]}",
            chain_idx, dyn_cali->pow_too_low_times[chain_idx]);
        dyn_cali->pow_too_low_times[chain_idx] = 0;
        return OAL_FAIL;
    }

    if (dyn_cali->pow_too_high_times[chain_idx] >= ONLINE_CALI_PWR_CHECK_TIMES) {
        oam_warning_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_check_invalid_tx_pwr::chain[%d], pow_too_high_times[%d]}",
            chain_idx, dyn_cali->pow_too_high_times[chain_idx]);
        dyn_cali->pow_too_high_times[chain_idx] = 0;
        return OAL_FAIL;
    }
    return OAL_FAIL;
}
/*
 * 函 数 名  : online_cali_tx_pow_calc_praw
 * 功能描述  : 动态校准计算P_raw
 */
OSAL_STATIC osal_u32 online_cali_tx_pow_calc_praw(hal_device_stru *device, online_cali_dyn_stru *dyn_cali,
    osal_s32 i_pdet, osal_s32 q_pdet, osal_s32 *ps_p_raw)
{
    osal_u8 chain_idx = dyn_cali->dyn_cali_complete_chain;
    osal_s32 i_vdet_offset = dyn_cali->dc_offset[0];
    osal_s32 q_vdet_offset = dyn_cali->dc_offset[1];
    const osal_s16 pdet_coef = 4; // online上报的pdet数值是上电的4倍
    osal_s32 i_pdet_mv = ((i_pdet / pdet_coef) - i_vdet_offset) / 0x2;  // 2023-08-01版本的曲线,芯片用mV作为单位
    osal_s32 q_pdet_mv = ((q_pdet / pdet_coef) - q_vdet_offset) / 0x2;
    osal_s32 p_raw = fe_calc_20log(i_pdet_mv * q_pdet_mv) / 0x2; // 此处计算的是2000log 而芯片用的是1000log
    unref_param(device);

    /* 判断上报电压是否异常低 */
    if ((q_pdet / pdet_coef < q_vdet_offset) || (i_pdet / pdet_coef < i_vdet_offset)) {
        oam_warning_log4(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pow_calc_praw::i_pdet[%d], q_pdet[%d], i_vdet_offset[%d], q_vdet_offset[%d]}\n",
            i_pdet, q_pdet, i_vdet_offset, q_vdet_offset);
        dyn_cali->pow_too_low_times[chain_idx]++;
        return online_cali_check_invalid_tx_pwr(dyn_cali);
    }
    /* PDET不在范围内不做动态校准 */
    if ((p_raw <= dyn_cali->dyn_cali_val_min_th) || (get_abs(p_raw) > 0x7FFF)) {
        oam_warning_log2(0, OAM_SF_CALIBRATE, "{online_cali_tx_pow_calc_praw::praw[%d] pdet_min_th[%d]}\n",
            p_raw, dyn_cali->dyn_cali_val_min_th);
        oam_warning_log4(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pow_calc_praw::i_pdet[%d], q_pdet[%d], i_vdet_offset[%d], q_vdet_offset[%d]}\n",
            i_pdet, q_pdet, i_vdet_offset, q_vdet_offset);
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_warning_log3(0, OAM_SF_CALIBRATE,
        "{online_cali_tx_pow_calc_praw::i_pdet[%d],q_pdet[%d],p_raw[%d]}\n", i_pdet, q_pdet, p_raw);
#endif
    *ps_p_raw = p_raw;
    return OAL_SUCC;
}
/*
 * 函 数 名   : online_cali_tx_pow_get_praw
 * 功能描述   : 获取动态校准praw
 */
OSAL_STATIC osal_u32 online_cali_tx_pow_get_praw(hal_device_stru *device,
    online_cali_dyn_stru *dyn_cali, online_cali_para_stru *online_cali)
{
    osal_u8 chain_idx = dyn_cali->dyn_cali_complete_chain;
    online_cali_info_stru *pow_info = &dyn_cali->dyn_cali_pow_info[chain_idx];
    osal_s32 i_pdet = 0;
    osal_s32 q_pdet = 0;
    osal_u32 ret;

    /* 动态校准计算IQ两路pdet */
    online_cali_tx_pow_get_iq_pdet(online_cali->pdet_val, &i_pdet, &q_pdet);

    /* 依照带fem与否计算P_raw */
    ret = online_cali_tx_pow_calc_praw(device, dyn_cali, i_pdet, q_pdet, &pow_info->pdet_cal);
    if (ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_CALIBRATE, "{online_cali_tx_pow_get_praw::i[%d]q[%d]pdet_cal[%d]}\n",
            i_pdet, q_pdet, pow_info->pdet_cal);
        return ret;
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 online_cali_flush_get_pdet_para(hal_device_stru *device, online_cali_stru *rf_cali,
    online_cali_para_stru *online_cali, osal_s16 tx_pow)
{
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    fe_mfg_power_cali_para_stru *mfg_power_cali_para = fe_mfg_get_cali_para();
#endif // _PRE_WLAN_FEATURE_MFG_TEST
    osal_u32 ret;
    unref_param(tx_pow);
    /* 计算prw */
    ret = online_cali_tx_pow_get_praw(device, dyn_cali, online_cali);
    if (ret == OAL_FAIL) {
        return ret;
    }
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    /* 仅在产线做ipa 曲线校准的时候开启 用于产线测试进行数据保存 */
    if (mfg_power_cali_para->curve_cal_mode == OAL_TRUE) {
        fe_mfg_tx_pow_save_equip_param(online_cali,
            dyn_cali->dyn_cali_pow_info[online_cali->chain_idx].pdet_cal,
            dyn_cali->dyn_cali_pow_info[online_cali->chain_idx].exp_pow);
        return OAL_FAIL;
    }
#endif // _PRE_WLAN_FEATURE_MFG_TEST
    return OAL_SUCC;
}

/*
 * 函 数 名  : online_cali_get_curve_pow
 * 功能描述  : 将pdet_cal转换到real_pow单位
 */
OSAL_STATIC osal_u32 online_cali_get_curve_pow(hal_device_stru *device, online_cali_para_stru *online_cali,
    online_cali_info_stru *pow_info, osal_s16 *real_pow)
{
    osal_u8 lvl_idx = (pow_info->exp_pow < FE_RF_CUS_HIGH_LOW_POW_THR) ? FE_CUS_LOW_POW: FE_CUS_HIGH_POW;
    osal_u8 power_curve_idx = (online_cali->cur_protocol == WLAN_11B_PHY_PROTOCOL_MODE) ?  FE_CUS_CURVE_11B :
        ((online_cali->band_width == WLAN_BW_CAP_20M) ? FE_CUS_CURVE_OFDM20M : FE_CUS_CURVE_OFDM40M);
    fe_custom_power_curve_stru tx_pow_par =
        fe_custom_get_power_curve(online_cali->channel_info.band, lvl_idx, power_curve_idx);
    fe_custom_power_curve_stru curve_factor =
        fe_custom_get_curve_factor(online_cali->channel_info.band, lvl_idx);
    osal_s16 praw = (osal_s16)pow_info->pdet_cal;
    unref_param(device);

    /* 检查曲线单调性 */
    if (fe_custom_check_curve(&tx_pow_par, &curve_factor, praw) != OAL_SUCC) {
        *real_pow = 0x0;
        return OAL_FAIL;
    }
    /* 根据实际电压估算当前功率 */
    if (fe_custom_calc_curve_pow(&tx_pow_par, &curve_factor, praw, real_pow) != OAL_SUCC) {
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_RF_CALI_DEBUG2
    wifi_printf("tpc[%d]ppa_code[0x%x]dbb[0x%x]dbb_idx[%d]pdet_val[0x%x]praw[%d]curve_pow[%d]\n",
        online_cali->tpc_code, fe_hal_phy_get_single_ppa_lut(device, online_cali->upc_idx),
        online_cali_get_digital_scaling_reg(device, online_cali, 0),
        online_cali_search_dbb_shadow_index(device, online_cali->tpc_code),
        online_cali->pdet_val, pow_info->pdet_cal, *real_pow);
#endif
    return OAL_SUCC;
}

/*
 * 函 数 名  : online_cali_get_pow_comp
 * 功能描述  : ipa温度补偿
 */
OSAL_STATIC osal_void online_cali_temp_comp(osal_s16 temp, osal_s16 pwr_after_cmp, osal_s16 *real_pwr)
{
    osal_s16 temp_delta_pwr = 0;
    osal_s32 temp_fac = 0;       /* 温度补偿因子，默认值用10000表示不补偿 */
    osal_s16 temp_curv[] = {0, ONLINE_CALI_TEMP_PRECISION_FAC}; /* 临时值 */
    osal_u8 temp_code;

    /* ipa 温度补偿实现 补偿值=(200*(temp*A+B)/10000)-200 */
    if (temp != ONLINE_CALI_INVALID_TEMP_CODE) {
        temp_fac = (osal_s16)(temp_curv[0] * temp) + temp_curv[1];
        temp_delta_pwr = (osal_s16)(ONLINE_CALI_TEMP_POW_200 * temp_fac / ONLINE_CALI_TEMP_PRECISION_FAC) -
            ONLINE_CALI_TEMP_POW_200;
    }
    temp_code = fe_hal_get_curr_temp_code();
    temp_delta_pwr = (temp_code < 2) ? (temp_delta_pwr - 10) : temp_delta_pwr;  // 小于温度码2表示低于0度 10表示1dB 补偿电流后功率增大的修正
    *real_pwr = (pwr_after_cmp - temp_delta_pwr);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_warning_log3(0, OAM_SF_CALIBRATE, "{online_cali_temp_comp::temp[%d]delta_pwr[%d]real_pwr[%d]}\n",
        temp, temp_delta_pwr, *real_pwr);
#endif
}

/*
 * 函 数 名  : online_cali_get_pow_comp
 * 功能描述  : 补偿
 */
OSAL_STATIC osal_s32 online_cali_get_pow_comp(hal_device_stru *device,
    online_cali_para_stru *online_cali, osal_s16 s_real_pow)
{
    osal_s16 s_temp = 0;
    osal_s16 s_pwr_after_temp = 0;
    osal_s16 max_pow = 0;
    wlan_channel_band_enum_uint8 band = online_cali->channel_info.band;

    /* 读取温度: 预留fe_rf_read_temperature(&s_temp) */
    fe_hal_rf_read_temperature(0, &s_temp);
    /* 动态校准温度补偿ipa */
    online_cali_temp_comp(s_temp, s_real_pow, &s_pwr_after_temp);
    // he er su实际上报的能量高3dB (帧头高3dB)
    s_pwr_after_temp -= fe_hal_rf_he_er_su_comp(online_cali->cur_protocol);

    max_pow = fe_custom_get_chip_max_power(band); /* ofdm_max_pwr */
    s_pwr_after_temp = (s_pwr_after_temp > max_pow) ? max_pow : s_pwr_after_temp;

    return s_pwr_after_temp;
}

/*
 * 函 数 名  : online_cali_flush_dyn_cali_pwr
 * 功能描述  : 获取本次功率校准参数
 */
OSAL_STATIC osal_u32 online_cali_flush_dyn_cali_pwr(hal_device_stru *device,
    online_cali_para_stru *online_cali, online_cali_info_stru *pow_info, osal_s32 *real_pow)
{
    osal_s32 pwr_after_comp = 0;
    osal_s16 curve_pow = 0;

    /* 实际功率-曲线拟合 */
    if (online_cali_get_curve_pow(device, online_cali, pow_info, &curve_pow) != OAL_SUCC) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_error_log2(0, OAM_SF_CALIBRATE, "{online_cali_flush_dyn_cali_pwr::real pdet[%d], curve pow[%d]}\n",
            pow_info->pdet_cal, curve_pow);
#endif
        return OAL_FAIL;
    }
    /* 实际功率-补偿修正(温补) */
    pwr_after_comp = online_cali_get_pow_comp(device, online_cali, curve_pow);
    *real_pow = pwr_after_comp;

#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log3(0, OAM_SF_CALIBRATE, "{online_cali_flush_dyn_cali_pwr::praw[%d]curve pow[%d]comp pow[%d]}\n",
        pow_info->pdet_cal, curve_pow, *real_pow);
#endif
    return OAL_SUCC;
}

/*
 * 函 数 名  : online_cali_flush_pow
 * 功能描述  : 获取本次功率校准参数
 */
OSAL_STATIC osal_u32 online_cali_flush_pow(hal_device_stru *device, online_cali_stru *rf_cali,
    online_cali_para_stru *online_cali, osal_s16 exp_pow)
{
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    online_cali_record_union dyn_cali_info;
    osal_u8 chain_idx = dyn_cali->dyn_cali_complete_chain;
    online_cali_info_stru *pow_info = &dyn_cali->dyn_cali_pow_info[chain_idx];
    osal_u8 no_change = OAL_FALSE;
    osal_u32 ret;

    pow_info->pdet_cal = online_cali->pdet_val;
    pow_info->exp_pow = exp_pow;
    online_cali_record_union_by_tx_complete(&dyn_cali_info, online_cali, exp_pow);
    /* 期望功率等参数是否变化 */
    no_change = online_cali_check_pwr_not_need_calc(pow_info->record_para.para, dyn_cali_info.para, pow_info->exp_pow);

    ret = online_cali_flush_get_pdet_para(device, rf_cali, online_cali, exp_pow);
    if (ret != OAL_SUCC) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{online_cali_flush_get_pdet_para failed}\n");
#endif
        return ret;
    }

    /* 刷新功率:曲线拟合,温度补偿 */
    ret = online_cali_flush_dyn_cali_pwr(device, online_cali, pow_info, &pow_info->real_pow);
    if (ret != OAL_SUCC) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_warning_log0(0, OAM_SF_CALIBRATE, "{online_cali_flush_dyn_cali_pwr failed}\n");
#endif
        return ret;
    }
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log2(0, OAM_SF_CALIBRATE, "{online_cali_flush_pow:exp_pow[%d]real_pow[%d]}\n",
        pow_info->exp_pow, pow_info->real_pow);
#endif
    pow_info->record_para.para = dyn_cali_info.para;
    /* 当期望功率等参数变化时，恢复为Init状态 */
    if (no_change == 0) {
        online_cali_init_adjust_dbb_scaling(device, online_cali, chain_idx);
    }
    return OAL_SUCC;
}

/*
 * 功能描述  : 配置当前功率校准值
 */
OSAL_STATIC osal_void online_cali_set_curr_upc_code(osal_u8 chain, osal_u8 upc_idx, cali_upc_code upc_code)
{
    online_cali_pow_ctrl_stru *txpow_ctrl = online_cali_get_txpow_ctrl(chain);
    txpow_ctrl->upc_idx2code_online[upc_idx].pow_code = upc_code.val;
}

/*
 * 功能描述  : 获取当前功率校准值
 * 1.日    期  : 2020年08月07日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_u16 online_cali_get_curr_upc_code(osal_u8 chain, osal_u8 upc_idx)
{
    online_cali_pow_ctrl_stru *txpow_ctrl = online_cali_get_txpow_ctrl(chain);
    return txpow_ctrl->upc_idx2code_online[upc_idx].pow_code;
}

/*
 * 功能描述  : 配置当前功率校准值
 */
OSAL_STATIC osal_void online_cali_set_phy_upc_code(hal_device_stru *device, osal_u8 chain, osal_u8 upc_idx)
{
    online_cali_pow_ctrl_stru *txpow_ctrl = online_cali_get_txpow_ctrl(chain);
    fe_hal_phy_set_single_ppa_lut(device, txpow_ctrl->upc_idx2code_online[upc_idx].pow_code, upc_idx);
}

/*
 * 函 数 名  : online_cali_device_rf_get_rf_idx
 * 功能描述  : 根据rf idx获取phy idx根据rf来配置RF通道
 */
OSAL_STATIC osal_u8 online_cali_device_rf_get_rf_idx(hal_device_stru *device, osal_u8 phy_chain_idx)
{
    unref_param(device);
    return phy_chain_idx;
}

/*
 * 函 数 名  : online_cali_pow_cali_set_dscr_intvl
 * 功能描述  : 动态校准描述符间隔数参数配置
 */
osal_void online_cali_pow_cali_set_dscr_intvl(online_cali_dyn_stru *dyn_cali,
    wlan_channel_band_enum_uint8 band, osal_u16 val)
{
    dyn_cali->dyn_cali_en_intvl[band] = val;
}

OSAL_STATIC osal_u32 online_cali_dyn_adjust_pwr_extend(online_cali_stru *rf_cali,
    online_cali_para_stru *online_cali, mac_channel_stru channel_info)
{
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    osal_u8 pdet_chain = dyn_cali->dyn_cali_complete_chain;
    online_cali_info_stru *dyn_cali_pow = &dyn_cali->dyn_cali_pow_info[pdet_chain];
    osal_s32 real_pow = dyn_cali_pow->real_pow;
    osal_s32 expect_pow = dyn_cali_pow->exp_pow;
    unref_param(online_cali);
    unref_param(channel_info);
    if (osal_abs_of_diff(real_pow, expect_pow) > ONLINE_CALI_PWR_DIFF_INVALID) {
        oam_warning_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_dyn_adjust_pwr_extend::real_pow is invalid! real_pow[%d], expect_pow[%d]}",
            real_pow, expect_pow);
        return OAL_FAIL;
    }

    return OAL_CONTINUE;
}

/*
 * 函 数 名  : online_cali_get_dyn_upc_unit_range
 * 功能描述  : 获取upc code的unit调整范围
 */
OSAL_STATIC osal_void online_cali_get_dyn_upc_unit_range(hal_device_stru *device, osal_u8 chain,
    osal_u8 upc_idx, osal_u16 *upc_unit_max, osal_u16 *upc_unit_min)
{
    unref_param(device);
    unref_param(chain);
    unref_param(upc_idx);

    *upc_unit_max = CALI_2G_TX_UPC_UNIT_VAL_MAX;
    *upc_unit_min = CALI_2G_TX_UPC_UNIT_VAL_MIN;
}
/*
 * 函 数 名   : online_cali_calc_gain_code
 * 功能描述   : 基于基准的UPC code和功率差计算新的upc_code
 */
OSAL_STATIC osal_s16 online_cali_calc_gain_code(osal_s16 target_gain, cali_upc_code previous_code,
    osal_u16 max_code, cali_upc_code *actual_code)
{
    cali_upc_code us_code = previous_code;
    osal_u16 abs_tar_gain;
    osal_s32 l_act_val;
    osal_s32 l_base_val;
    osal_s16 s_actual_gain = target_gain;
    osal_u32 calc_10power;

    if (target_gain == 0) {
        *actual_code = previous_code;
        return target_gain;
    }

    /* 降低功率 */
    if (target_gain < 0) {
        /* 查表计算upc_code = (base_upc_code + 1) * 10^(upc_target_gain/20) -1 */
        abs_tar_gain = (osal_u16)(-target_gain);
        if (abs_tar_gain > ONLINE_CALI_NUM_OF_10POWER_NEG_TABLE - 1) {
            abs_tar_gain = ONLINE_CALI_NUM_OF_10POWER_NEG_TABLE - 1;
            s_actual_gain = (osal_s16)(-abs_tar_gain);
            oam_warning_log2(0, OAM_SF_CALIBRATE, "{online_cali_calc_gain_code::target_gain[%d]<[-%d] too small.}",
                target_gain, ONLINE_CALI_NUM_OF_10POWER_NEG_TABLE - 1);
        }
        calc_10power = g_aus_linear_to_10power_neg_lut[abs_tar_gain] * (previous_code.bits.unit + 1);
        us_code.bits.unit = (osal_u8)(online_cali_pow_right_shift_floor(calc_10power, CALI_POW_PRECISION_SHIFT) - 1);
        if (us_code.bits.unit > max_code) { // 翻转的情况
            us_code.bits.unit = 1;
        }
    } else {
        /* 提高功率 */
        /* 查表计算upc_code = (base_upc_code + 1) * 10^(upc_target_gain/20) -1 */
        abs_tar_gain = (osal_u16)target_gain;

        if (abs_tar_gain > ONLINE_CALI_NUM_OF_10POWER_POS_TABLE - 1) {
            abs_tar_gain = ONLINE_CALI_NUM_OF_10POWER_POS_TABLE - 1;
            s_actual_gain = ONLINE_CALI_NUM_OF_10POWER_POS_TABLE - 1;
            oam_warning_log2(0, OAM_SF_CALIBRATE, "{online_cali_calc_gain_code::target_gain[%d]>[%d] too large.}",
                target_gain, ONLINE_CALI_NUM_OF_10POWER_POS_TABLE - 1);
        }
        calc_10power = g_aus_linear_to_10power_pos_lut[abs_tar_gain] * (previous_code.bits.unit + 1);
        us_code.bits.unit = (osal_u8)(online_cali_pow_right_shift_floor(calc_10power, CALI_POW_PRECISION_SHIFT) - 1);

        /* 提高upc_code，避免超出范围 */
        if (us_code.bits.unit > max_code) {
            oam_info_log3(0, OAM_SF_CALIBRATE, "{online_cali_calc_gain_code::us_code[%d]> max_code[%d] or 8*[%d]-1.}",
                us_code.bits.unit, max_code, previous_code.bits.unit);
            us_code.bits.unit = max_code;

            /* 查表计算upc_code对应的实际增益 upc_gain = 20*log((upc_code+1)/(base_upc_code+1)) */
            l_act_val = fe_calc_20log((osal_s32)us_code.bits.unit + 1);
            l_base_val = fe_calc_20log((osal_s32)previous_code.bits.unit + 1);
            s_actual_gain = (osal_s16)(l_act_val - l_base_val);
            /* 精度从0.01恢复为0.1db */
            s_actual_gain = online_cali_pow_div_round(s_actual_gain, CALI_POW_PRECISION_SHIFT);
        }
    }

    /* 输出 */
    *actual_code = us_code;
    return s_actual_gain;
}

/*
 * 函 数 名   : online_cali_calc_gain_by_code
 * 功能描述   : 基于基准的UPC code/dbb_scale和code值计算功率差值
 */
OSAL_STATIC osal_void online_cali_calc_gain_by_code(cali_upc_code curr_code, cali_upc_code base_code,
    osal_s16 *actual_gain)
{
    osal_s32 l_act_val;
    osal_s32 l_base_val;
    osal_s16 s_actual_gain;

    if (curr_code.val == base_code.val) {
        *actual_gain = 0;
        return;
    }

    /* 查表计算upc_code对应的实际增益 upc_gain = 20*log((upc_code+1)/(base_upc_code+1)) */
    l_act_val = fe_calc_20log(((osal_s32)curr_code.bits.unit + 1));
    l_base_val = fe_calc_20log(((osal_s32)base_code.bits.unit + 1));
    s_actual_gain = (osal_s16)(l_act_val - l_base_val);
    // 单位从0.01转换为0.1db 考虑四舍五入
    s_actual_gain = online_cali_pow_div_round(s_actual_gain, CALI_POW_PRECISION_SHIFT);
    *actual_gain = s_actual_gain;
}

/*
 * 函 数 名  : online_cali_read_upc_reg
 * 功能描述  : 读取一个通道的所有档位的upc参数
 */
OSAL_STATIC osal_void online_cali_read_upc_reg(hal_device_stru *device, osal_u8 stream_type)
{
    online_cali_pow_ctrl_stru *txpow_ctrl = online_cali_get_txpow_ctrl(stream_type);
    osal_u8 upc_idx;
    osal_u32 upc_code = 0;
    for (upc_idx = 0; upc_idx < txpow_ctrl->target_gain_num_online; upc_idx++) {
        upc_code = fe_hal_phy_get_single_ppa_lut(device, upc_idx);
        txpow_ctrl->upc_idx2code_online[upc_idx].pow_code = (osal_u16)upc_code;
    }
}

/*
 * 函 数 名  : cali_write_one_level_upc_reg
 * 功能描述  : 设置一个档位的所有upc参数
 */
OSAL_STATIC osal_void cali_write_one_level_upc_reg(hal_device_stru *device, osal_u8 stream_type, osal_u8 lvl_idx)
{
    online_cali_pow_ctrl_stru *txpow_ctrl = online_cali_get_txpow_ctrl(stream_type);
    osal_u8 start_upc_idx = 0;
    osal_u8 end_upc_idx = 0;
    osal_u8 upc_idx;
    cali_get_upc_idx_range(lvl_idx, &start_upc_idx, &end_upc_idx);
    for (upc_idx = start_upc_idx; upc_idx <= end_upc_idx; upc_idx++) {
        fe_hal_phy_set_single_ppa_lut(device, txpow_ctrl->upc_idx2code_online[upc_idx].pow_code, upc_idx);
    }
}

/*
 * 函 数 名   : online_set_rf_cali_tx_pow_accum_wait_para
 * 功能描述   : 动态校准功率调整则等待5帧
 */
OSAL_STATIC osal_void online_set_rf_cali_tx_pow_accum_wait_para(online_cali_dyn_stru *dyn_cali_val, osal_u8 pdet_chain)
{
    dyn_cali_val->pow_accum_flag[pdet_chain] = OAL_FALSE;
    dyn_cali_val->pow_accum_times[pdet_chain] = 5;  // 等待5帧
}

/*
 * 函 数 名   : online_cali_same_channel_index
 * 功能描述   : 是否相同校准频点判断
 */
OSAL_STATIC osal_u32 online_cali_same_channel_index(hal_device_stru *device,
    online_cali_dyn_stru *dyn_cali, osal_u32 ppa_lut_state)
{
    osal_s32 device_chan = 0;
    unref_param(device);
    /* device侧使用的频点0~2 */
    switch (ppa_lut_state & 0xE) {
        case BIT1:
            device_chan = 0;
            break;
        case BIT2:
            device_chan++;
            break;
        case BIT3:
            device_chan++;
            device_chan++;
            break;
        default:
#ifdef _PRE_WLAN_RF_CALI_DEBUG
            oam_error_log1(0, OAM_SF_CALIBRATE,
                "{online_cali_same_channel_index:soc ppat lut lock reg value[%d] was wrong set}\n", ppa_lut_state);
#endif
            break;
    }

    /* device当前信道和上报帧的信道是否一致, 不一致返回 */
    if (device_chan != dyn_cali->cali_channel_index) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_error_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_same_channel_index:channel different abort ppa lut write:curr_chan[%d]dyn_chan[%d]}\n",
            device_chan, dyn_cali->cali_channel_index);
#endif
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名   : online_cali_cfg_one_level_upc_reg
 * 功能描述   : 将全局g_txpow_ctrl写入ppa lut寄存器表
 */
OSAL_STATIC osal_void online_cali_cfg_one_level_upc_reg(hal_device_stru *device,
    online_cali_dyn_stru *dyn_cali, online_cali_para_stru *online_cali, osal_u8 lvl_idx)
{
    osal_u32 ppa_lut_state = fe_hal_gp_get_ppa_lut_lock_state();
    /* 频点发生变化时,取消补偿 */
    if (online_cali_same_channel_index(device, dyn_cali, ppa_lut_state) == OAL_FAIL) {
        return;
    }
    if (cali_wait_ppa_lut_free() == OAL_SUCC) {
        fe_hal_gp_set_ppa_lut_lock_state(BIT0 & ppa_lut_state);
        cali_write_one_level_upc_reg(device, online_cali->chain_idx, lvl_idx);
        fe_hal_gp_set_ppa_lut_lock_state(ppa_lut_state);
    }
}

/*
 * 函 数 名   : online_cali_cfg_single_upc_reg
 * 功能描述   : 将全局g_txpow_ctrl写入单个ppa lut寄存器表
 */
OSAL_STATIC osal_void online_cali_cfg_single_upc_reg(hal_device_stru *device, online_cali_dyn_stru *dyn_cali,
    online_cali_para_stru *online_cali, osal_u8 upc_idx)
{
    osal_u32 ppa_lut_state = fe_hal_gp_get_ppa_lut_lock_state();
    if (online_cali_same_channel_index(device, dyn_cali, ppa_lut_state) == OAL_FAIL) {
        return;
    }
    if (cali_wait_ppa_lut_free() == OAL_SUCC) {
        fe_hal_gp_set_ppa_lut_lock_state(BIT0 & ppa_lut_state);
        online_cali_set_phy_upc_code(device, online_cali->chain_idx, upc_idx);
        fe_hal_gp_set_ppa_lut_lock_state(ppa_lut_state);
    }
}

/*
 * 函 数 名   : online_cali_calc_one_level_upc_code_lut
 * 功能描述   : 刷新某个挡位的所有upc code的unit
 */
OSAL_STATIC osal_void online_cali_calc_one_level_upc_code_lut(hal_device_stru *device,
    const online_cali_dyn_stru *dyn_cali, online_cali_para_stru *online_cali, osal_u8 lvl_idx)
{
    osal_u8 pdet_chain = dyn_cali->dyn_cali_complete_chain;
    cali_upc_code curr_code = {0};
    cali_upc_code bef_code = {0};
    cali_upc_code real_code = {0};
    online_cali_pow_ctrl_stru *txpow_ctrl = online_cali_get_txpow_ctrl(pdet_chain);
    osal_u8 base_upc_idx = 0;
    osal_u8 target_gain_idx = 0;
    osal_s16 pow_diff, real_gain;
    osal_u16 upc_unit_max = 0;
    osal_u16 upc_unit_min = 0;
    osal_u8 start_upc_idx = 0;
    osal_u8 end_upc_idx = 0;
    curr_code.val = online_cali_get_curr_upc_code(pdet_chain, online_cali->upc_idx);
    online_cali_get_dyn_upc_unit_range(device, pdet_chain, online_cali->upc_idx, &upc_unit_max, &upc_unit_min);
    base_upc_idx = cali_get_base_ppa_idx(lvl_idx);
    /* 反推出调整之前的upc code */
    if (online_cali->upc_idx != base_upc_idx) {
        pow_diff = txpow_ctrl->upc_idx2code_online[online_cali->upc_idx].pow_gain -
            txpow_ctrl->upc_idx2code_online[base_upc_idx].pow_gain;
        real_gain = online_cali_calc_gain_code(pow_diff,
            (cali_upc_code)(txpow_ctrl->upc_idx2code_online[base_upc_idx].pow_code), upc_unit_max, &bef_code);
    } else {
        pow_diff = txpow_ctrl->upc_idx2code_online[online_cali->upc_idx].pow_gain -
            txpow_ctrl->upc_idx2code_online[base_upc_idx + 1].pow_gain;
        real_gain = online_cali_calc_gain_code(pow_diff,
            (cali_upc_code)(txpow_ctrl->upc_idx2code_online[base_upc_idx + 1].pow_code), upc_unit_max, &bef_code);
    }
    /* 反算出调整的ppa增益 */
    online_cali_calc_gain_by_code(curr_code, bef_code, &pow_diff);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log4(0, OAM_SF_CALIBRATE, "{update_tpc_lut:upc_idx[%d]bef_code[0x%x]curr_code[0x%x]pow_dif[%d]}\n",
        online_cali->upc_idx, bef_code.val, curr_code.val, pow_diff);
#endif

    /* 更新某个挡位upc code全局 */
    cali_get_upc_idx_range(lvl_idx, &start_upc_idx, &end_upc_idx);
    for (target_gain_idx = start_upc_idx; target_gain_idx <= end_upc_idx; target_gain_idx++) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log2(0, OAM_SF_CALIBRATE, "{update_tpc_lut::upc_idx[%d]code before[0x%x]}\n",
            target_gain_idx, txpow_ctrl->upc_idx2code_online[target_gain_idx].pow_code);
#endif
        pow_diff = pow_diff * ONLINE_CALI_DIFF_RATE_CENTUPLICATE / ONLINE_CALI_COEFF_AMP;
        /* 当前校准档位不计算 */
        if (online_cali->upc_idx != target_gain_idx) {
            curr_code = (cali_upc_code)(txpow_ctrl->upc_idx2code_online[target_gain_idx].pow_code);
            real_gain = online_cali_calc_gain_code(pow_diff, curr_code, upc_unit_max, &real_code);
            txpow_ctrl->upc_idx2code_online[target_gain_idx].pow_code = real_code.val;
        }
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log2(0, OAM_SF_CALIBRATE, "{update_tpc_lut::upc_idx[%d]code after[0x%x]}\n",
            target_gain_idx, txpow_ctrl->upc_idx2code_online[target_gain_idx].pow_code);
#endif
    }
}

/*
 * 函 数 名   : online_cali_update_tpc_lut
 * 功能描述   : 刷新ppa lut寄存器表
 */
OSAL_STATIC osal_void online_cali_update_tpc_lut(online_cali_stru *rf_cali, hal_device_stru *device,
    online_cali_para_stru *online_cali, mac_channel_stru channel_info)
{
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    osal_u8 lvl_idx = 0;

    unref_param(channel_info);
    lvl_idx = cali_get_pwr_lvl_by_ppa_idx(online_cali->upc_idx);
    /* 更新一个挡位upc code全局 */
    online_cali_calc_one_level_upc_code_lut(device, dyn_cali, online_cali, lvl_idx);
    /* 设置一个档位的ppa lut寄存器表 */
    online_cali_cfg_one_level_upc_reg(device, dyn_cali, online_cali, lvl_idx);
}

/*
 * 函 数 名   : online_cali_tx_pwr_invalid_process
 * 功能描述   : upc code unit范围检验
 */
OSAL_STATIC osal_u32 online_cali_tx_pwr_invalid_process(online_cali_dyn_stru *dyn_cali, osal_u16 upc_unit_val,
    osal_u16 upc_unit_max, osal_u16 upc_unit_min)
{
    osal_u8 chain_idx = dyn_cali->dyn_cali_complete_chain;

    if (upc_unit_val < upc_unit_min) {
        dyn_cali->pow_too_low_times[chain_idx]++;
        oam_warning_log4(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pwr_invalid_process::chain[%d], too_low_times[%d], real_unit[%d] < min_unit[%d]}\n",
            dyn_cali->dyn_cali_complete_chain, dyn_cali->pow_too_low_times[chain_idx],
            upc_unit_val, upc_unit_min);
    } else if (upc_unit_val > upc_unit_max) {
        dyn_cali->pow_too_high_times[chain_idx]++;
        oam_warning_log4(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pwr_invalid_process::chain[%d], too_high_times[%d], real_unit[%d] > max_unit[%d]}\n",
            dyn_cali->dyn_cali_complete_chain, dyn_cali->pow_too_high_times[chain_idx],
            upc_unit_val, upc_unit_max);
    } else {
        dyn_cali->pow_too_low_times[chain_idx] = 0;
        dyn_cali->pow_too_high_times[chain_idx] = 0;
        return OAL_SUCC;
    }
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_warning_log0(0, OAM_SF_CALIBRATE, "{online_cali_tx_pwr_invalid_process failed}\n");
#endif
    return OAL_FAIL;
}

/*
 * 功能描述  : 获取定制化配置的校准间隔值
 */
osal_u16 online_cali_get_cus_dyn_cali_intvl(wlan_channel_band_enum_uint8 band)
{
    unref_param(band);
    return ONLINE_CALI_DSCR_INTERVAL;
}

/*
 * 函 数 名  : online_cali_calc_upc_adj
 * 功能描述  : 根据期望pdet和实际pdet差值,根据差值调整upc code
 * 1.日    期  : 2020年04月1日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_u32 online_cali_calc_upc_adj(hal_device_stru *device,
    online_cali_para_stru *online_cali, osal_s32 real_pow, osal_s32 exp_pow, osal_s32 *gain_left)
{
    online_cali_stru *rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    online_cali_dyn_stru *dyn_cali = &(rf_cali->dyn_cali_val);
    cali_upc_code upc_code = {0};
    cali_upc_code upc_code_ori = {0};
    osal_u16 upc_unit_max = 0;
    osal_u16 upc_unit_min = 0;
    osal_u8 pdet_chain = dyn_cali->dyn_cali_complete_chain;
    osal_u8 upc_idx = dyn_cali->dyn_cali_pow_info[pdet_chain].upc_idx;
    osal_s32 pow_diff = exp_pow - real_pow;
    osal_s16 upc_adjust_gain = 0;
    osal_u8 lvl_idx = cali_get_pwr_lvl_by_ppa_idx(online_cali->upc_idx);
    osal_u32 ret;

    /* upc code范围 */
    online_cali_get_dyn_upc_unit_range(device, pdet_chain, upc_idx, &upc_unit_max, &upc_unit_min);
    /* 调整upc */
    upc_code.val = online_cali_get_curr_upc_code(pdet_chain, upc_idx);
    upc_code_ori = upc_code;
    upc_adjust_gain = online_cali_calc_gain_code((osal_s16)pow_diff, upc_code_ori, upc_unit_max, &upc_code);
    /* 范围检验 */
    ret = online_cali_tx_pwr_invalid_process(dyn_cali, upc_code.bits.unit, upc_unit_max, upc_unit_min);
    if (ret != OAL_SUCC) {
        return ret;
    }
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log4(0, OAM_SF_CALIBRATE, "{calc_upc_adj::upc idx[%d]range[<%d]upc_adjust_gain[%d]upc_unit[0x%x]}\n",
        upc_idx, upc_unit_max, upc_adjust_gain, upc_code.bits.unit);
#endif
    /* 保存到全局变量 */
    online_cali_set_curr_upc_code(pdet_chain, upc_idx, upc_code);
    /* 保存UPC code到寄存器 */
    online_cali_cfg_single_upc_reg(device, dyn_cali, online_cali, upc_idx);
    *gain_left = pow_diff - upc_adjust_gain;
    /* 若调节功率则将praw过滤标志置为true */
    dyn_cali->pow_accum_flag[pdet_chain] = OAL_TRUE;
    dyn_cali->dyn_pow_cali_status.ppa_lut_adjusted_flag[pdet_chain][lvl_idx] = OAL_TRUE;
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log2(0, OAM_SF_CALIBRATE, "{calc_upc_adj::lvl_idx[%d]ppa_lut_adjusted_flag[%d]}\n",
        lvl_idx, dyn_cali->dyn_pow_cali_status.ppa_lut_adjusted_flag[pdet_chain][lvl_idx]);
#endif
    if (dyn_cali->dyn_pow_cali_flag) {
        oam_error_log4(0, OAM_SF_CALIBRATE,
            "{online_cali_calc_upc_adj::pdet_chain[%d], pwr[%d], upc_code[%d], upc_code_ori[%d]}",
            pdet_chain, dyn_cali->dyn_cali_pow_info[pdet_chain].record_para.rf_core_para.tx_pow,
            upc_code.val, upc_code_ori.val);
        oam_error_log4(0, OAM_SF_CALIBRATE,
            "{online_cali_calc_upc_adj::exp_pow[%d], real_pow[%d], upc_adjust_gain[%d], gain_left[%d]}",
            exp_pow, real_pow, upc_adjust_gain, *gain_left);
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : online_cali_dyn_adjust_upc_dbb
 * 功能描述  : 调整功率变化，首先调整upc->dbb
 */
OSAL_STATIC osal_void online_cali_dyn_adjust_upc_dbb(online_cali_dyn_stru *dyn_cali,
    online_cali_para_stru *online_cali, hal_device_stru *device, mac_channel_stru channel_info)
{
    osal_u8 pdet_chain = dyn_cali->dyn_cali_complete_chain;
    online_cali_info_stru *dyn_cali_pow = &dyn_cali->dyn_cali_pow_info[pdet_chain];
    osal_s32 real_pow = dyn_cali_pow->real_pow;
    online_cali_adj_type_enum adj_flag = ONLINE_CALI_PDET_ADJUST_BUTT;
    osal_s32 cur_dbb = (osal_s32)online_cali_get_digital_scaling_reg(device, online_cali, pdet_chain);
    osal_s32 gain_diff_left = real_pow - dyn_cali_pow->exp_pow;
    osal_u32 ret;
    unref_param(channel_info);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log4(0, OAM_SF_CALIBRATE, "{dyn_upc_dbb_adj::cur_dbb[0x%x] pdet_chain[%d]real_pow[%d]exp_pow[%d]}",
        cur_dbb, pdet_chain, real_pow, dyn_cali_pow->exp_pow);
#endif

    /* pdet差值在1dB内的进行dbb调整，否则恢复为init状态，先调整upc */
    if (osal_abs_of_diff(real_pow, dyn_cali_pow->exp_pow) > ONLINE_CALI_UPC_RE_ADJUST_TH1) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log2(0, OAM_SF_CALIBRATE, "{dyn_upc_dbb_adj::diff[%d]max_th[%d]}",
            gain_diff_left, ONLINE_CALI_UPC_RE_ADJUST_TH1);
#endif
        online_cali_init_adjust_dbb_scaling(device, online_cali, pdet_chain);
        adj_flag = ONLINE_CALI_PDET_ADJUST_INIT;
        cur_dbb = ONLINE_CALI_DBB_SCALING_INIT_VAL;
    }
    if (adj_flag == ONLINE_CALI_PDET_ADJUST_INIT) {
        /* 首先调整upc */
        ret = online_cali_calc_upc_adj(device, online_cali, real_pow, dyn_cali_pow->exp_pow, &gain_diff_left);
        if (ret == OAL_FAIL) {
            return;
        }
    }
    if (get_abs(gain_diff_left) <= ONLINE_CALI_PWR_MIN_TH) {
        return;
    }
    if (get_abs(gain_diff_left) > ONLINE_CALI_UPC_RE_ADJUST_TH1) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log1(0, OAM_SF_CALIBRATE, "{gain left[%d] > 1dB, init_adjust_dbb_scaling}\n", gain_diff_left);
#endif
        online_cali_init_adjust_dbb_scaling(device, online_cali, pdet_chain);
        return;
    }
    online_cali_adjust_dbb_scaling(device, online_cali, pdet_chain, gain_diff_left, cur_dbb);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log3(0, OAM_SF_CALIBRATE, "{0.3<gain left<=1dB,cur_dbb[0x%x]gain_left[%d]adj_flag[%d]}",
        cur_dbb, gain_diff_left, adj_flag);
#endif
    return;
}

/*
 * 函 数 名  : online_cali_dyn_adjust_pwr
 * 功能描述  : 根据期望pdet和实际pdet差值,根据差值调整tpc code
 */
OSAL_STATIC osal_void online_cali_dyn_adjust_pwr(online_cali_stru *rf_cali, hal_device_stru *device,
    online_cali_para_stru *online_cali, mac_channel_stru channel_info)
{
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    osal_u8 pdet_chain = dyn_cali->dyn_cali_complete_chain;
    online_cali_info_stru *dyn_cali_pow = &dyn_cali->dyn_cali_pow_info[pdet_chain];
    osal_s32 real_pow = dyn_cali_pow->real_pow;
    osal_s32 expect_pow = dyn_cali_pow->exp_pow;
    osal_u8 lvl_idx = cali_get_pwr_lvl_by_ppa_idx(online_cali->upc_idx);

    if (online_cali_dyn_adjust_pwr_extend(rf_cali, online_cali, channel_info) == OAL_FAIL) {
        return;
    }
    if ((osal_s32)osal_abs_of_diff(real_pow, expect_pow) <= dyn_cali->dyn_cali_val_min_th) {
        dyn_cali->dyn_pow_cali_status.dyn_pow_cali_comp_flag[pdet_chain][lvl_idx] = OAL_TRUE;
        if (dyn_cali->dyn_pow_cali_status.ppa_lut_adjusted_flag[pdet_chain][lvl_idx] == OAL_FALSE) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
            oam_info_log0(0, OAM_SF_CALIBRATE, "{online_cali_dyn_adjust_pwr::no need to update ppa lut}\n");
#endif
            return;
        }
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log4(0, OAM_SF_CALIBRATE, "{online_cali_dyn_adjust_pwr::chain[%d]min_th[%d]real_pow[%d]exp_pow[%d]}\n",
            pdet_chain, dyn_cali->dyn_cali_val_min_th, real_pow, expect_pow);
#endif

        online_set_rf_cali_tx_pow_accum_wait_para(dyn_cali, pdet_chain);
        dyn_cali->pow_too_high_times[pdet_chain] = 0;
        dyn_cali->pow_too_low_times[pdet_chain] = 0;
        /* 设置所有档位的upc参数 */
        if (dyn_cali->com_upc_switch == OAL_TRUE) {
            online_cali_update_tpc_lut(rf_cali, device, online_cali, channel_info);
        }
        dyn_cali->dyn_pow_cali_status.ppa_lut_adjusted_flag[pdet_chain][lvl_idx] = OAL_FALSE;
        return;
    }
    dyn_cali->dyn_pow_cali_status.dyn_pow_cali_comp_flag[pdet_chain][lvl_idx] = OAL_FALSE;
    online_cali_dyn_adjust_upc_dbb(dyn_cali, online_cali, device, channel_info);
    return;
}

/*
 * 函 数 名  : online_cali_pow_complete_handler
 * 功能描述  : 动态校准补偿入口函数
 */
OSAL_STATIC osal_void online_cali_pow_complete_handler(online_cali_stru *rf_cali, online_cali_para_stru *online_cali,
    hal_device_stru *device)
{
    osal_u8 chain_idx;
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    oal_bool_enum_uint8 is_dyn_cali_completed = OAL_TRUE;
    wlan_channel_band_enum_uint8 band = dyn_cali->dyn_cali_complete_band;
    osal_u16 dyn_cali_dscr_interval = online_cali_get_cus_dyn_cali_intvl(band);
    osal_u8 rf_idx;
    osal_u8 lvl_idx = cali_get_pwr_lvl_by_ppa_idx(online_cali->upc_idx);
    for (chain_idx = 0; chain_idx < CALI_STREAM_TYPE_BUTT; chain_idx++) {
        rf_idx = online_cali_device_rf_get_rf_idx(device, chain_idx);
        if (dyn_cali->dyn_pow_cali_status.dyn_pow_cali_comp_flag[rf_idx][lvl_idx] == OAL_FALSE) {
            is_dyn_cali_completed = OAL_FALSE;
            break;
        }
    }

    if (is_dyn_cali_completed == OAL_TRUE) {
        /* 校准收敛=校准完成 */
        dyn_cali_dscr_interval <<= ONLINE_CALI_COMPLETE_INTVL_FACTOR;
        for (chain_idx = 0; chain_idx < CALI_STREAM_TYPE_BUTT; chain_idx++) {
            rf_idx = online_cali_device_rf_get_rf_idx(device, chain_idx);
            if (dyn_cali->dyn_pow_cali_status.dyn_pow_cali_comp_flag[rf_idx][lvl_idx] == OAL_TRUE) {
                dyn_cali->dyn_cali_pow_info[rf_idx].cali_pdet_adjust_flag = ONLINE_CALI_PDET_ADJUST_INIT;
            }
        }
        frw_timer_stop_timer(&(dyn_cali->dyn_cali_intvl_timer));
        online_cali_pow_cali_set_dscr_intvl(dyn_cali, band, dyn_cali_dscr_interval);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log1(0, OAM_SF_CALIBRATE, "{dyn_cali_completed::set interval[%d]}\n", dyn_cali_dscr_interval);
#endif
        online_rf_cali_handle_event(rf_cali, ONLINE_CALI_EVENT_DYN_PWR_COMPLETE, 0, NULL);
        return;
    } else if (device->hal_device_base.al_tx_flag != 0) {
        /* 软件常发: 不做200ms帧间隔 */
        frw_timer_stop_timer(&(dyn_cali->dyn_cali_intvl_timer));
        online_cali_pow_cali_set_dscr_intvl(dyn_cali, band, dyn_cali_dscr_interval);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log1(0, OAM_SF_CALIBRATE, "{al_tx_flag::set interval[%d]}\n", dyn_cali_dscr_interval);
#endif
        online_cali_send_intvl(rf_cali);
        return;
    }
    /* 校准不收敛: 等200ms */
    online_cali_pow_cali_set_dscr_intvl(dyn_cali, band, dyn_cali_dscr_interval);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log1(0, OAM_SF_CALIBRATE,
        "{not al_tx_flag and not dyn_cali_completed::set interval[%d]}\n", dyn_cali_dscr_interval);
#endif
}

/*
 * 函 数 名  : online_cali_pow_comp_entrance
 * 功能描述  : 动态校准补偿入口函数
 */
OSAL_STATIC osal_void online_cali_pow_comp_entrance(online_cali_stru *rf_cali,
    hal_device_stru *device, online_cali_para_stru *online_cali, osal_s16 exp_pow)
{
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    osal_u8 chain_idx = dyn_cali->dyn_cali_complete_chain;
    online_cali_info_stru *dyn_cali_pow = &dyn_cali->dyn_cali_pow_info[chain_idx];
    wlan_channel_band_enum_uint8 band = dyn_cali->dyn_cali_complete_band;

    if ((online_cali_dyn_pdet_check(dyn_cali, online_cali->pdet_val) == OAL_FAIL) ||
        (online_cali_flush_pow(device, rf_cali, online_cali, exp_pow) == OAL_FAIL)) {
        if (device->hal_device_base.al_tx_flag != 0) {
            /* 软件常发: 不做200ms帧间隔 */
            frw_timer_stop_timer(&(dyn_cali->dyn_cali_intvl_timer));
            online_cali_pow_cali_set_dscr_intvl(dyn_cali, band, online_cali_get_cus_dyn_cali_intvl(band));
#ifdef _PRE_WLAN_RF_CALI_DEBUG
            oam_info_log1(0, OAM_SF_CALIBRATE,
                "{al_tx_flag::set interval[%d]}\n", online_cali_get_cus_dyn_cali_intvl(band));
#endif
            online_cali_send_intvl(rf_cali);
        }
        return;
    }

    dyn_cali_pow->upc_idx = online_cali->upc_idx;
    online_cali_dyn_adjust_pwr(rf_cali, device, online_cali, online_cali->channel_info);
    online_cali_pow_complete_handler(rf_cali, online_cali, device);
}

/*
 * 函 数 名  : online_cali_init_flush_pow_code
 * 功能描述  : 当信道变化时，从寄存器读取ppa lut
 */
OSAL_STATIC osal_void online_cali_init_flush_pow_code(hal_device_stru *device,
    online_cali_stru *rf_cali, online_cali_para_stru *online_cali)
{
    osal_u32 ppa_lut_state = fe_hal_gp_get_ppa_lut_lock_state();
    osal_s8 device_chan = 0;
    osal_s8 dyn_chan = rf_cali->dyn_cali_val.cali_channel_index;

#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log3(0, OAM_SF_CALIBRATE,
        "{online_cali_init_flush_pow_code::pdet_en[%d]pdet_val[0x%x]tpc_code[%d]}\n",
        online_cali->pdet_enable, online_cali->pdet_val, online_cali->tpc_code);
#endif
    switch (ppa_lut_state & 0xE) {
        case BIT1:
            device_chan = 0;
            break;
        case BIT2:
            device_chan = 0x1;
            break;
        case BIT3:
            device_chan = 0x2;
            break;
        default:
#ifdef _PRE_WLAN_RF_CALI_DEBUG
            oam_error_log1(0, OAM_SF_CALIBRATE, "{soc ppat lut lock reg value[%d] was wrong set}\n", ppa_lut_state);
#endif
            break;
    }
    if (dyn_chan == -1 || device_chan != dyn_chan) {
        rf_cali->dyn_cali_val.cali_channel_index = device_chan;
        online_cali_read_upc_reg(device, online_cali->chain_idx);
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_init_flush_pow_code::dyn_chan[%d]device_chan[%d]read upc reg}\n",
            dyn_chan, device_chan);
#endif
    }
}

OSAL_STATIC osal_u32 online_cali_tx_pow_pre_check(hal_device_stru *device, osal_u8 band,
    online_cali_para_stru *online_cali, online_cali_dyn_stru *dyn_cali)
{
    osal_u8 tpc_code_offset =
        (online_cali->cur_protocol == WLAN_11B_PHY_PROTOCOL_MODE) ? CALI_POW_CAL_2G_OFDM_TABLE_LEN : 0;
    if (device == NULL) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{online_cali_tx_pow_handler::hal dev is NULL}\n");
        return OAL_FAIL;
    }
    if (online_cali->tpc_code > tpc_code_offset + ONLINE_CALI_MAX_DYN_TPC_CODE) {
#ifdef _PRE_WLAN_RF_CALI_DEBUG
        oam_info_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pow_handler::tpc_code[%d] > [%d] (power=10dBm) discard online tx pow}\n",
            online_cali->tpc_code, tpc_code_offset + ONLINE_CALI_MAX_DYN_TPC_CODE);
#endif
        return OAL_FAIL;
    }

    frw_timer_stop_timer(&(dyn_cali->dyn_cali_per_frame_timer)); /* 校准执行则关闭定时器 */
    frw_create_timer_entry(&(dyn_cali->dyn_cali_intvl_timer), online_cali_intvl_timeout_handler,
        ONLINE_CALI_INTVL_TIMER_PERIOD, device, OAL_FALSE);   /* 延迟触发pwr tx notify */

    if (dyn_cali->dyn_cali_en_intvl[band] == 0) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{online_cali_tx_pow_handler::interval = 0.}\n");
        return OAL_FAIL;
    }
    /*  动态校准未使能 */
    if (!online_cali->pdet_enable) {
        oam_error_log3(0, OAM_SF_CALIBRATE,
            "{online_cali_tx_pow_handler::abort chain[%d] pdet_en[0x%x] dyn_cali_chain[%d].}\n",
            online_cali->chain_idx, online_cali->pdet_enable, dyn_cali->tx_dyn_cali_chain_sel);
        return OAL_FAIL;
    }
    /* 功率校准异常打断不使用校准结果 */
    if (online_cali->invalid == OAL_TRUE) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{online_cali_tx_pow_handler::invalid.}\n");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : alg_online_cali_tx_pow_handler
 * 功能描述  : 动态校准总入口函数
 */
OSAL_STATIC osal_u32 online_cali_tx_pow_handler(online_cali_stru *rf_cali, osal_void *param)
{
    online_cali_para_stru *online_cali = (online_cali_para_stru*)param;
    online_cali_dyn_stru *dyn_cali = &rf_cali->dyn_cali_val;
    hal_device_stru *device = (hal_device_stru *)hal_chip_get_hal_device();
    wlan_channel_band_enum_uint8 band = online_cali->channel_info.band;
    osal_s16 exp_pow = ONLINE_CALI_POW_INVALID;
    osal_u32 ret;

    if (online_cali_tx_pow_pre_check(device, band, online_cali, dyn_cali) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 从寄存器读取ppa lut */
    online_cali_init_flush_pow_code(device, rf_cali, online_cali);
    online_cali->freq = band;
    /* 判断当前动态功率校准是否使能 */
    if (online_cali_state_is_in_pwr(rf_cali) == OAL_FALSE) {
        oam_error_log0(0, OAM_SF_CALIBRATE, "{online_cali_tx_pow_handler::not in pwr work state.}\n");
        return OAL_FAIL;
    }
    online_cali_get_pow_cali_param(device, dyn_cali, online_cali);
    ret = online_cali_get_target_tx_power(device, online_cali, &exp_pow);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CALIBRATE,
            "{online_cali_get_target_tx_power failed::ret[%d]exp_pow[%d].}\n", ret, exp_pow);
        return OAL_FAIL;
    }

    /* 补偿使能 */
    if (dyn_cali->dyn_cali_adjst == OAL_TRUE) {
        online_cali_pow_comp_entrance(rf_cali, device, online_cali, exp_pow);
    }
#ifdef _PRE_WLAN_RF_CALI_DEBUG
    oam_info_log0(0, OAM_SF_CALIBRATE, "end\n\n");
#endif
    return OAL_SUCC;
}

/*
 * 功能描述   : 动态功率校准事件处理入口
 */
osal_u32 online_cali_dyn_pow_event(osal_void *p_ctx, osal_u16 us_event,
    osal_u16 us_event_data_len, osal_void *p_event_data)
{
    online_cali_tx_fsm_info_stru *online_tx_cali_fsm = (online_cali_tx_fsm_info_stru *)p_ctx;
    online_cali_stru *rf_cali = NULL;
    unref_param(us_event_data_len);
    rf_cali = online_cali_get_alg_rf_online_cali_ctx();
    switch (us_event) {
        /* 发送时处理 */
        case ONLINE_CALI_EVENT_TX_NOTIFY:
            online_cali_pow_tx_notify(rf_cali, p_event_data);
            break;

        /* 发送完成时处理 */
        case ONLINE_CALI_EVENT_TX_COMPLETE:
            online_cali_tx_pow_handler(rf_cali, p_event_data);
            break;

        /* 动态功率校准完成处理 */
        case ONLINE_CALI_EVENT_DYN_PWR_COMPLETE:
            online_cali_trans_to_next_state(rf_cali);
            break;

        /* 带宽和信道改变时处理 */
        case ONLINE_CALI_EVENT_CHANNEL_CHANGE:
        case ONLINE_CALI_EVENT_BW_CHANGE:
        case ONLINE_CALI_EVENT_EQUIPE_CHANGE:
            return online_rf_cali_fsm_trans_to_state(online_tx_cali_fsm, ONLINE_CALI_STATE_INIT);
        default:
            oam_error_log1(0, OAM_SF_CALIBRATE, "{online_cali_dyn_pow_event:: us_event[%d] INVALID!}", us_event);
            break;
    }

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // _PRE_WLAN_ONLINE_CALI