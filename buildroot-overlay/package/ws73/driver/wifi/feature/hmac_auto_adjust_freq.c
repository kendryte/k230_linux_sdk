/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: device调频相关函数实现.
 * Create: 2020-7-9
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_auto_adjust_freq.h"
#include "hmac_blockack.h"
#include "hmac_psm_sta.h"
#ifdef _PRE_WLAN_FEATURE_WS73
#include "soc_lowpower.h"
#endif
#include "pm_extern.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_AUTO_ADJUST_FREQ_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#define WLAN_ALL_THRESHOLD_NUM 6
#define MAX_FREQ_ADJUST_NUM 3

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_pps_statistics_stru g_device_pps_statistics = { 0 };
hmac_freq_control_stru g_device_freq_type = { 0 };
osal_u8 g_auto_freq_enable = OSAL_FALSE;

/* 吞吐量统计定时器周期 */
osal_u32 g_wlan_auto_freq_throughput_timeout = 200;
/* 有包时连续10个周期都需要降频才降频 */
osal_u32 g_wlan_auto_freq_data_loop_threshold = 10;
/* 没有包时连续4个周期都需要降频才降频 */
osal_u32 g_wlan_auto_freq_no_data_loop_threshold = 4;

/* 由定制化进行初始化 */
device_pps_freq_level_stru g_device_pps_freq_level[] = {
    /* pps门限     CPU主频level */
    {0,          FREQ_IDLE},
    {0,          FREQ_MIDIUM},
    {0,          FREQ_HIGHEST},
};
/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC hmac_freq_control_stru *hmac_get_auto_freq_handle(osal_void)
{
    return &g_device_freq_type;
}

/*****************************************************************************
 函 数 名  : hmac_get_pps_statistics_handle
 功能描述  : 获取pps统计结构体
*****************************************************************************/
OAL_STATIC hmac_pps_statistics_stru *hmac_get_pps_statistics_handle(osal_void)
{
    return &g_device_pps_statistics;
}

OAL_STATIC osal_u8 hmac_get_g_auto_freq_enable(osal_void)
{
    return g_auto_freq_enable;
}

OAL_STATIC osal_void hmac_set_g_auto_freq_enable(osal_u8 enable_value)
{
    g_auto_freq_enable = enable_value;
    return;
}

/*****************************************************************************
 函 数 名  : hmac_auto_freq_update_level
 功能描述  : device的调频函数
*****************************************************************************/
OAL_STATIC osal_void hmac_auto_freq_update_level(osal_void)
{
#ifdef _PRE_WLAN_FEATURE_WS73
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_u32 ret;
#endif
#endif
    hmac_freq_control_stru *freq_handle = OSAL_NULL;
    hmac_pps_statistics_stru *pps_handle = OSAL_NULL;

    freq_handle = hmac_get_auto_freq_handle();
    /* 相等不需要调频 */
    if (freq_handle->curr_freq_level == freq_handle->req_freq_level) {
        return;
    }

    if (freq_handle->req_freq_level > FREQ_HIGHEST) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_set_auto_freq_level:Wrong freq[%d] larger than max[%d].}",
            freq_handle->req_freq_level, FREQ_HIGHEST);
        return;
    }

    /* 调用平台接口进行调频 */
#ifdef _PRE_WLAN_FEATURE_WS73
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ret = pm_crg_svc_vote(0, freq_handle->req_freq_level); // 0: CRG sid of WiFi
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_set_auto_freq_level: call pm_crg_svc_vote() fail, freq[%d] ret[%u].}",
            freq_handle->req_freq_level, ret);
        return;
    }
#endif
#endif
    pps_handle = hmac_get_pps_statistics_handle();

    oam_warning_log4(0, OAM_SF_ANY,
        "{hmac_set_auto_freq_level:change freq [%d] to [%d] current hcc_txrx_total(%d) pps_rate(%d).}",
        freq_handle->curr_freq_level, freq_handle->req_freq_level, pps_handle->hcc_rxtx_total, pps_handle->pps_rate);

    freq_handle->curr_freq_level = freq_handle->req_freq_level;
}

/*****************************************************************************
 函 数 名  : hmac_auto_freq_pps_process
 功能描述  : device根据吞吐率判断是否需要调频
*****************************************************************************/
OAL_STATIC osal_void hmac_auto_freq_pps_process(osal_u32 pkt_count)
{
    hmac_freq_control_stru *freq_handle = OSAL_NULL;

    freq_handle = hmac_get_auto_freq_handle();
    if (freq_handle->req_freq_level == freq_handle->curr_freq_level) {
        freq_handle->pps_loop_count = 0;
        return;
    }

    if (freq_handle->req_freq_level < freq_handle->curr_freq_level) {
        /* 连续多次都在降频的区间后才降频，保证性能 */
        freq_handle->pps_loop_count++;
        if (pkt_count != 0) {
            if (freq_handle->pps_loop_count >= g_wlan_auto_freq_data_loop_threshold) {
                freq_handle->pps_loop_count = 0;
                hmac_auto_freq_update_level();
            }
        } else {
            if (freq_handle->pps_loop_count >= g_wlan_auto_freq_no_data_loop_threshold) {
                freq_handle->pps_loop_count = 0;
                hmac_auto_freq_update_level();
            }
        }
    } else {
        /* 升频不等待，立即执行保证性能 */
        freq_handle->pps_loop_count = 0;

        /* 当需要升频时，立即升频 */
        if (freq_handle->curr_freq_level < FREQ_MIDIUM) {
            freq_handle->req_freq_level = FREQ_MIDIUM;
        } else {
            freq_handle->req_freq_level = FREQ_HIGHEST;
        }

        hmac_auto_freq_update_level();
    }
}

/*****************************************************************************
 函 数 名  : hmac_auto_freq_set_pps_level
 功能描述  : device根据吞吐率获取对应的cpu调频level
*****************************************************************************/
OAL_STATIC osal_void hmac_auto_freq_set_pps_level(osal_u32 pps_rate)
{
    hmac_freq_control_stru *freq_handle = OSAL_NULL;
    osal_u8 level_idx;

    freq_handle = hmac_get_auto_freq_handle();
    if (pps_rate <= g_device_pps_freq_level[1].speed_level) { /* 1表示第1个等级的的吞吐量门限 */
        level_idx = 0; /* 0表示CPU调频level第0等级 */

    /* 1表示第1个等级的的吞吐量门限 */
    } else if ((pps_rate > g_device_pps_freq_level[1].speed_level) &&
        (pps_rate <= g_device_pps_freq_level[2].speed_level)) { /* 2表示第2个等级的的吞吐量门限 */
        level_idx = 1; /* 1表示CPU调频level第1等级 */
    } else {
        level_idx = 2; /* 2表示CPU调频level第2等级 */
    }

    freq_handle->req_freq_level = (osal_u8)g_device_pps_freq_level[level_idx].cpu_freq_level;
}

/*****************************************************************************
 函 数 名  : hmac_auto_freq_pps_timeout
 功能描述  : device调频吞吐率统计定时器
*****************************************************************************/
OAL_STATIC osal_u32 hmac_auto_freq_pps_timeout(osal_void *prg)
{
    hmac_freq_control_stru *freq_handle = OSAL_NULL;
    hmac_pps_statistics_stru *pps_handle = OSAL_NULL;
    osal_u32 cur_time;
    osal_u32 return_total_count;
    unref_param(prg);

    pps_handle = hmac_get_pps_statistics_handle();
    return_total_count = pps_handle->hcc_rxtx_total;
    cur_time = (osal_u32)osal_get_time_stamp_ms();
    if (cur_time > pps_handle->last_timeout) {
        /* 10表示左移10位获得次数 */
        pps_handle->pps_rate = (return_total_count << 10) / (cur_time - pps_handle->last_timeout);
    }

    freq_handle = hmac_get_auto_freq_handle();
    if (freq_handle->auto_freq_enable == OSAL_TRUE) {
        /* 根据吞吐量获取调频级别 */
        hmac_auto_freq_set_pps_level(pps_handle->pps_rate);

        hmac_auto_freq_pps_process(return_total_count);
    }

    pps_handle->hcc_rxtx_total = 0;
    pps_handle->last_timeout = (osal_u32)osal_get_time_stamp_ms();

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_auto_freq_init
 功能描述  : device调频功能初始化函数
*****************************************************************************/
OAL_STATIC osal_void hmac_auto_freq_init(osal_void)
{
    hmac_pps_statistics_stru *pps_handle = OSAL_NULL;
    frw_timeout_stru *timer = OSAL_NULL;
    hmac_freq_control_stru *freq_handle = OSAL_NULL;
    hmac_pps_value_stru pps_value;

    freq_handle = hmac_get_auto_freq_handle();
    pps_handle = hmac_get_pps_statistics_handle();

    timer = &pps_handle->timer;

    freq_handle->auto_freq_enable = OSAL_TRUE;
    freq_handle->pm_enable = OSAL_FALSE;
    freq_handle->curr_freq_level = FREQ_HIGHEST;
    freq_handle->req_freq_level = FREQ_IDLE;
    freq_handle->pps_loop_count = 0;

    pps_handle->pps_rate = 0;
    pps_handle->last_timeout = 0;
    pps_handle->hcc_rxtx_total = 0;
    if (g_device_pps_freq_level[FREQ_HIGHEST].speed_level == 0) {
        pps_value = hal_get_auto_adjust_freq_pps();
        g_device_pps_freq_level[FREQ_IDLE].speed_level = pps_value.pps_value_0;
        g_device_pps_freq_level[FREQ_MIDIUM].speed_level = pps_value.pps_value_1;
        g_device_pps_freq_level[FREQ_HIGHEST].speed_level = pps_value.pps_value_2;
    }
    /* 初始设为level0 */
    hmac_auto_freq_update_level();
    oam_warning_log2(0, OAM_SF_ANY,
        "{hmac_auto_freq_init:set the freq to level[%d] timer->is_registerd[%d].}", FREQ_IDLE, timer->is_registerd);

    if (timer->is_registerd == OSAL_FALSE) {
        frw_create_timer_entry(timer, hmac_auto_freq_pps_timeout,
            g_wlan_auto_freq_throughput_timeout, OSAL_NULL, OSAL_TRUE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_auto_freq_deinit
 功能描述  : device调频功能去初始化函数
*****************************************************************************/
OAL_STATIC osal_void hmac_auto_freq_deinit(osal_void)
{
    frw_timeout_stru *timer = OSAL_NULL;
    hmac_freq_control_stru *freq_handle = OSAL_NULL;
    hmac_pps_statistics_stru *pps_handle = OSAL_NULL;

    pps_handle = hmac_get_pps_statistics_handle();
    timer = &pps_handle->timer;

    freq_handle = hmac_get_auto_freq_handle();
    freq_handle->auto_freq_enable = OSAL_FALSE;

    oam_warning_log1(0, OAM_SF_ANY,
        "{hmac_auto_freq_deinit:timer->is_registerd[%d].}", timer->is_registerd);

    if (timer->is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(timer);
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_auto_freq_deinit: timer exit.}");
    }
}

/*****************************************************************************
 函 数 名  : hmac_auto_freq_pps_count
 功能描述  : device统计device收发数据帧个数
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC osal_void hmac_auto_freq_pps_count(osal_u32 pkt_cnt)
{
    g_device_pps_statistics.hcc_rxtx_total += pkt_cnt;
}

OAL_STATIC osal_void hmac_get_pps_handle_pps_rate(osal_u32 *pps_rate)
{
    hmac_pps_statistics_stru *pps_handle = hmac_get_pps_statistics_handle();
    *pps_rate = pps_handle->pps_rate;
}

/*****************************************************************************
 功能描述  : 设置device的主频
 输入参数  : osal_u8 enable_switch
 返 回 值 : osal_u32
*****************************************************************************/
OAL_STATIC osal_void hmac_set_auto_freq_enable(osal_u8 enable_switch)
{
    hmac_freq_control_stru *device_freq_handle = hmac_get_auto_freq_handle();
    if (enable_switch >= WLAN_DEVICE_AUTO_FREQ_BUTT) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_set_auto_freq_enable::enable_switch[%u] invalid.}", enable_switch);
        return;
    }
    if (enable_switch == WLAN_DEVICE_AUTO_FREQ_OFF) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_set_auto_freq_enable::begin the hmac_auto_freq_deinit.}");
        hmac_auto_freq_deinit();

        /* 不使能device调频则最高频率运行 */
        device_freq_handle->req_freq_level = FREQ_HIGHEST;
        hmac_auto_freq_update_level();
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_set_auto_freq_enable::begin the hmac_auto_freq_init.}");
        crg_wlan_enable_chg_cpu_freq(OSAL_TRUE);
        hmac_auto_freq_init();
    }
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
// 设置流量门限值g_user_device_pps_freq_level
OAL_STATIC osal_u32 hmac_set_flow_thershold(const osal_u32 thershold[], osal_u8 threshold_length)
{
    osal_u8 i;
    hmac_freq_control_stru *time_handle = OSAL_NULL;
    memset_s(time_handle, sizeof(hmac_freq_control_stru), 0, sizeof(hmac_freq_control_stru));

    if (thershold == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_set_flow_thershold:para error, the input is null.}");
        return OAL_FAIL;
    }
    if (threshold_length != WLAN_ALL_THRESHOLD_NUM) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hmac_set_flow_thershold:para error, the input length is %d.}", threshold_length);
        return OAL_FAIL;
    }

    for (i = 0; i < FREQ_BUTT; i++) {
        if (g_device_pps_freq_level[i].speed_level != thershold[i]) {
            g_device_pps_freq_level[i].speed_level = thershold[i];
        }
    }
    if (g_wlan_auto_freq_throughput_timeout != thershold[i]) {
        g_wlan_auto_freq_throughput_timeout = thershold[i];
    }

    i++;
    if (g_wlan_auto_freq_data_loop_threshold != thershold[i]) {
        g_wlan_auto_freq_data_loop_threshold = thershold[i];
    }

    i++;
    if (g_wlan_auto_freq_no_data_loop_threshold != thershold[i]) {
        g_wlan_auto_freq_no_data_loop_threshold = thershold[i];
    }

    // 除去时间周期，其他的阈值都能在设置完成后生效，如果想要时间周期也生效，则先关闭自动调频，然后再打开
    time_handle = hmac_get_auto_freq_handle();
    if (time_handle->auto_freq_enable == OSAL_TRUE) {
        // 如果之前设置了使能操作,先关闭使能，再次重新打开使能
        hmac_auto_freq_deinit();
        hmac_auto_freq_init();
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_flow_thershold(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u32 thershold[WLAN_ALL_THRESHOLD_NUM];
    osal_s32 ret;
    osal_u32 timer_period;
    osal_s8 i;
    unref_param(hmac_vap);

    for (i = 0; i < WLAN_ALL_THRESHOLD_NUM; i++) {
        ret = hmac_ccpriv_get_one_arg_digit(&param, (osal_s32 *)(uintptr_t)(&thershold[i]));
        if (osal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_ccpriv_set_flow_thershold::return err_code [%d] [%d]!}", i, ret);
            return ret;
        }
    }

    /* 3:设置的定时器周期 */
    timer_period = thershold[3];
    /* 对设置的定时器周期的校验，时间周期为20ms到1000ms */
    if ((timer_period < 20) || (timer_period > 1000)) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_ccpriv_set_flow_thershold:  timer_period [%d] out of range.}", timer_period);
        return OAL_FAIL;
    }
    ret = (osal_s32)hmac_set_flow_thershold(thershold, WLAN_ALL_THRESHOLD_NUM);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_ccpriv_set_flow_thershold: hmac_set_flow_thershold fail[%d].}", ret);
        return OAL_FAIL;
    }
    /* 将设置的参数打印出来，3:自动调频任务周期 */
    oam_warning_log3(0, OAM_SF_CFG,
        "{hmac_ccpriv_set_flow_thershold::timer_period[%d] has_pack_period[%d] no_pack_period[%d]}",
        timer_period, thershold[4], thershold[5]); /* 4:有包情况下，连续多少次才降频，5:无包情况下，连续多少次才降频 */
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_auto_freq_enable(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u8 freq_enable_switch;
    osal_s32 ret;

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &freq_enable_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_set_auto_freq_enable::get arg failed.}");
        return ret;
    }
    /* 上层下发的使能 */
    hmac_set_auto_freq_enable(freq_enable_switch);

    oam_warning_log1(0, OAM_SF_CFG,
        "{hmac_ccpriv_set_auto_freq_enable::enable mode[%u][1:enable,0:disable].}", freq_enable_switch);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_auto_freq_level(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    hmac_freq_control_stru *freq_handle = OSAL_NULL;
    osal_s32 ret;
    osal_u8 level;

    unref_param(hmac_vap);
    /* 开关的取值范围为0-2,对应不同的频率值,做参数合法性判断,对应dmac中的g_freq_adjust */
    ret = hmac_ccpriv_get_u8_with_check_max(&param, MAX_FREQ_ADJUST_NUM - 1, &level);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_get_auto_freq_info::get arg failed.}");
        return ret;
    }

    /* 73在fpga上不支持自动调频，待适配后才能使用自动调频 */
    crg_wlan_enable_chg_cpu_freq(OSAL_TRUE);
    freq_handle = hmac_get_auto_freq_handle();
    freq_handle->req_freq_level = level;
    hmac_auto_freq_update_level();
    oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_set_auto_freq_level::device_freq[%u].}",
        freq_handle->req_freq_level);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_auto_freq_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u8 type;
    osal_s32 ret;

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_u8_with_check_max(&param, EXT_SWITCH_ON, &type);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_get_auto_freq_info::get arg failed.}");
        return ret;
    }

    if (type == 0) {
        hmac_freq_control_stru *freq_handle = OSAL_NULL;
        freq_handle = hmac_get_auto_freq_handle();
        oam_warning_log3(0, OAM_SF_ANY,
            "{hmac_ccpriv_get_auto_freq_info::auto_freq_enable[%u] curr_freq_level[%u] req_freq_level[%u].}",
            freq_handle->auto_freq_enable, freq_handle->curr_freq_level, freq_handle->req_freq_level);
    } else {
        hmac_pps_statistics_stru *pps_handle = OSAL_NULL;
        pps_handle = hmac_get_pps_statistics_handle();
        oam_warning_log2(0, OAM_SF_ANY,
            "{hmac_ccpriv_get_auto_freq_info::pps_rate[%u] hcc_rxtx_total[%u].}",
            pps_handle->pps_rate, pps_handle->hcc_rxtx_total);
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC osal_void hmac_auto_freq_btcoex_handle(osal_u8 bt_ldac)
{
    if (bt_ldac == OSAL_FALSE) {
        /* 开启自动调频 */
        if (hmac_get_g_auto_freq_enable() == OSAL_TRUE) {
            hmac_set_auto_freq_enable(WLAN_DEVICE_AUTO_FREQ_ON);
            hmac_set_g_auto_freq_enable(OSAL_FALSE);
        }
    } else {
        /* 关闭自动调频 */
        hmac_freq_control_stru *freq_handle = hmac_get_auto_freq_handle();
        if ((hmac_get_g_auto_freq_enable() == OSAL_FALSE) && (freq_handle->auto_freq_enable == OSAL_TRUE)) {
            hmac_set_auto_freq_enable(WLAN_DEVICE_AUTO_FREQ_OFF);
            hmac_set_g_auto_freq_enable(OSAL_TRUE);
        }
    }
}

osal_u32 hmac_auto_adjust_freq_init(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"set_device_freq_enable", hmac_ccpriv_set_auto_freq_enable);
    hmac_ccpriv_register((const osal_s8 *)"set_device_flow_threshold", hmac_ccpriv_set_flow_thershold);
    hmac_ccpriv_register((const osal_s8 *)"set_device_freq_value", hmac_ccpriv_set_auto_freq_level);
    hmac_ccpriv_register((const osal_s8 *)"get_device_freq_value", hmac_ccpriv_get_auto_freq_info);
#endif
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_AUTO_FREQ_FRAME_COUNT, hmac_auto_freq_pps_count);
    hmac_feature_hook_register(HMAC_FHOOK_AUTO_FREQ_PPS, hmac_get_pps_handle_pps_rate);
    hmac_feature_hook_register(HMAC_FHOOK_AUTO_FREQ_BTCOEX, hmac_auto_freq_btcoex_handle);
    return OAL_SUCC;
}

osal_void hmac_auto_adjust_freq_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"set_device_freq_enable");
    hmac_ccpriv_unregister((const osal_s8 *)"set_device_flow_threshold");
    hmac_ccpriv_unregister((const osal_s8 *)"set_device_freq_value");
    hmac_ccpriv_unregister((const osal_s8 *)"get_device_freq_value");
#endif
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_AUTO_FREQ_FRAME_COUNT);
    hmac_feature_hook_unregister(HMAC_FHOOK_AUTO_FREQ_PPS);
    hmac_feature_hook_unregister(HMAC_FHOOK_AUTO_FREQ_BTCOEX);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
