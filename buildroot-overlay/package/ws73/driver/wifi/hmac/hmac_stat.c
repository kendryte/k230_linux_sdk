/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: throughput statist.
 */
#include "osal_list.h"
#include "hmac_vap.h"
#include "hmac_config.h"
#include "hmac_alg_config.h"
#include "mac_device_ext.h"
#include "oal_types_device_rom.h"
#include "hmac_device.h"
#include "mac_vap_ext.h"
#include "dmac_common_inc_rom.h"
#include "frw_ext_if.h"
#include "hmac_feature_interface.h"
#include "hmac_btcoex.h"
#include "hmac_stat.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_STAT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
#ifdef _PRE_WLAN_FEATURE_PK_MODE
/*****************************************************************************
  2 函数与全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
#define LOW_PK_MODE_TH            50  /* pk切换到非pk模式的下限阈值 */
#define HIGH_PK_MODE_TH           65  /* 非pk切换到pk模式的上限阈值 */
osal_u32 g_low_pk_mode_th = LOW_PK_MODE_TH;
osal_u32 g_high_pk_mode_th = HIGH_PK_MODE_TH;
#define INVALID_PKMODE_TH (0xFFFFFFFF)
#endif
oal_bool_enum_uint8 g_pk_mode_status = OAL_FALSE;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/* 获取ap当前的协议模式 */
OAL_STATIC osal_u8 hmac_update_pk_mode_curr_protocol_cap(const hmac_user_stru *hmac_user)
{
    osal_u8 curr_protocol_cap;
    switch (hmac_user->cur_protocol_mode) {
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11B_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            curr_protocol_cap = WLAN_PROTOCOL_CAP_LEGACY;
            return curr_protocol_cap;
        case WLAN_HT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            curr_protocol_cap = WLAN_PROTOCOL_CAP_HT;
            return curr_protocol_cap;
#ifdef _PRE_WLAN_SUPPORT_VHT
        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            curr_protocol_cap = WLAN_PROTOCOL_CAP_VHT;
            return curr_protocol_cap;
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
            curr_protocol_cap = WLAN_PROTOCOL_CAP_HE;
            return curr_protocol_cap;
#endif
        case WLAN_PROTOCOL_BUTT:
        default:
            curr_protocol_cap = WLAN_PROTOCOL_CAP_BUTT;
            return curr_protocol_cap;
    }
}
/* 更新pk模式信息包括ap当前带宽，流数及协议 */
OAL_STATIC osal_void hmac_update_pk_mode_info(uint8_t *vap_idx, mac_cfg_pk_mode_stru *hmac_pk_mode)
{
    uint8_t vap_id;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_user_stru *hmac_user = NULL;
    struct osal_list_head *entry = NULL;
    struct osal_list_head *dlist_tmp = NULL;

    for (vap_id = 0; vap_id < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; vap_id++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
        if (hmac_vap == NULL) {
            oam_error_log1(0, OAM_SF_ANY, "{hmac_update_pk_mode_info::hmac_vap(%d) is null.}", vap_id);
            continue;
        }

        if (!is_legacy_sta(hmac_vap)) {
            continue;
        }

        osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
            hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
            if (hmac_user == NULL) {
                oam_error_log0(hmac_vap->uc_vap_id, OAM_SF_CFG, "{hmac_update_pk_mode_info::pst_user_tmp null.}");
                continue;
            }

            /* 从用户（ap）信息中获取当前带宽和流数 */
            /* 仅在vap作sta且关联单个ap时支持 */
            hmac_pk_mode->curr_bw_cap = hmac_user->cur_bandwidth;
            hmac_pk_mode->curr_num_spatial_stream = hmac_user->avail_num_spatial_stream;
            /* 获取协议模式 */
            hmac_pk_mode->curr_protocol_cap = hmac_update_pk_mode_curr_protocol_cap(hmac_user);

            *vap_idx = vap_id;
        }
    }

    return;
}
/* 根据带宽流数协议决定pk模式切换的门限值 */
OAL_STATIC oal_bool_enum_uint8 hmac_update_pk_mode_status_by_th(osal_u8 pk_mode, osal_u32 txrx_throughput)
{
    oal_bool_enum_uint8 curr_pk_mode;
    /* 当前仅考虑11ax下20m单流的情况，基数可以调整 */
    if (pk_mode == OAL_TRUE) {
        curr_pk_mode = (txrx_throughput < g_low_pk_mode_th) ? OAL_FALSE : OAL_TRUE;
    } else {
        curr_pk_mode = (txrx_throughput > g_high_pk_mode_th) ? OAL_TRUE : OAL_FALSE;
    }
    return curr_pk_mode;
}
OAL_STATIC osal_void hmac_update_pk_mode(osal_u32 tx_throughput, osal_u32 rx_throughput)
{
    oal_bool_enum_uint8 curr_pk_mode;
    uint8_t vap_id = 0;
    hmac_vap_stru *hmac_vap = NULL;
    osal_u32 ret, txrx_throughput;
    hmac_device_stru *hmac_device = NULL;
    mac_cfg_pk_mode_stru hmac_pk_mode_status = { 0 };

    hmac_device = hmac_res_get_mac_dev_etc(0);
    /* 计算up的vap个数，非单个不考虑 */
    if (hmac_device_calc_up_vap_num_etc(hmac_device) != 1) {
        return;
    }

    /* 更新pk模式所需信息 */
    hmac_update_pk_mode_info(&vap_id, &hmac_pk_mode_status);

    /* 有效判定 */
    if ((hmac_pk_mode_status.curr_bw_cap != WLAN_BW_CAP_20M) ||
        (hmac_pk_mode_status.curr_protocol_cap != WLAN_PROTOCOL_CAP_HE)) {
        return;
    }

    /* 计算吞吐率: rxtx吞吐之和 / 统计时间间隔（单位s） */
    txrx_throughput = tx_throughput + rx_throughput;

    /* 根据带宽流数协议得出 pk模式切换门限值 */
    curr_pk_mode = hmac_update_pk_mode_status_by_th(g_pk_mode_status, txrx_throughput);
    /* PK模式状态没有变化无须通知更改 */
    if (curr_pk_mode == g_pk_mode_status) {
        return;
    }
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_update_pk_mode::hmac_vap(%d) is null.}", vap_id);
        return;
    }

    oam_warning_log4(0, OAM_SF_ANY,
                     "{hmac_update_pk_mode: pk_mode changed (%d) -> (%d), tx_throughput[%d], rx_throughput[%d]!}",
                     g_pk_mode_status, curr_pk_mode, tx_throughput, rx_throughput);
    g_pk_mode_status = curr_pk_mode;
    /***************************************************************************
        抛事件给处理函数，73在host侧
    ***************************************************************************/
    ret = hmac_config_pk_mode(hmac_vap, curr_pk_mode);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG,
                         "{hmac_update_pk_mode::hmac_config_pk_mode failed[%d].}", ret);
    }
    return;
}
#endif

/* pk模式阈值设置 */
osal_s32 hmac_config_pk_mode_th(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_pk_mode_th_reset_stru *pk_th_param = (mac_cfg_pk_mode_th_reset_stru *)msg->data;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_start_vap_etc::param null");
        return OAL_ERR_CODE_PTR_NULL;
    }
    unref_param(hmac_vap);
    g_low_pk_mode_th = pk_th_param->low_th;
    g_high_pk_mode_th = pk_th_param->high_th;
    oam_warning_log2(0, OAM_SF_ANY, "{hmac_config_pk_mode_th: low_th[%d], high_th[%d].}",
                     pk_th_param->low_th, pk_th_param->high_th);
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_STAT
STATIC hmac_stat_control_stru g_hmac_stat_control = { 0 };
hmac_device_stat_stru g_hmac_device_stats;
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT hmac_device_stat_stru *hmac_stat_get_device_stats(void)
{
    return &g_hmac_device_stats;
}

/* 定时器时间判定 */
OAL_STATIC oal_bool_enum_uint8 hmac_adjust_throughput_stat_txrx_bytes(osal_u32 dur_time)
{
    /* 超过一定时长不做统计，排除定时器异常问题 */
    if (dur_time > (HMAC_STAT_TIMER_PERIOD << BIT_OFFSET_2)) {
        hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
        g_hmac_stat_control.pre_time = (osal_u32)osal_get_time_stamp_ms();

        osal_adapt_atomic_set(&hmac_device_stats->rx_bytes, 0);
        osal_adapt_atomic_set(&hmac_device_stats->tx_bytes, 0);
        osal_adapt_atomic_set(&hmac_device_stats->rx_packets, 0);
        osal_adapt_atomic_set(&hmac_device_stats->tx_packets, 0);
        oam_warning_log1(0, OAM_SF_CFG, "hmac_throughput_timer time[%u], reset!.", dur_time);

        return OAL_FALSE;
    }
    return OAL_TRUE;
}

/* 200ms定时器到期处理 */
OAL_STATIC osal_u32 hmac_timeout_stat(osal_void *p_arg)
{
     /* hmac 吞吐量计算 */
    osal_u32 tx_throughput, rx_throughput, dur_time;
    osal_u32 tx_throughput_mbps = 0;
    osal_u32 rx_throughput_mbps = 0;
    hmac_device_stat_stru *hmac_device_stats = hmac_stat_get_device_stats();
    osal_void *fhook = NULL;
    unref_param(p_arg);

    dur_time = (osal_u32)(osal_get_time_stamp_ms() - g_hmac_stat_control.pre_time);
    if (hmac_adjust_throughput_stat_txrx_bytes(dur_time) == OAL_FALSE) {
        return OAL_SUCC;
    }
    /* 总字节数 */
    rx_throughput = (osal_u32)osal_adapt_atomic_read(&hmac_device_stats->rx_bytes);
    tx_throughput = (osal_u32)osal_adapt_atomic_read(&hmac_device_stats->tx_bytes);
    if (dur_time != 0) {
        tx_throughput_mbps = (tx_throughput >> BIT_OFFSET_7) / dur_time;
        rx_throughput_mbps = (rx_throughput >> BIT_OFFSET_7) / dur_time;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_WIFI_BUSY_NOTIFY);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_hal_wifi_busy_notify_cb)fhook)(tx_throughput_mbps, rx_throughput_mbps);
    }
#if defined(_PRE_WLAN_FEATURE_PK_MODE) && defined(_PRE_WLAN_FEATURE_NEGTIVE_DET)
    hmac_update_pk_mode(tx_throughput_mbps, rx_throughput_mbps);
#endif
    /* 清空收发包数据 */
    osal_adapt_atomic_set(&hmac_device_stats->rx_bytes, 0);
    osal_adapt_atomic_set(&hmac_device_stats->tx_bytes, 0);
    osal_adapt_atomic_set(&hmac_device_stats->rx_packets, 0);
    osal_adapt_atomic_set(&hmac_device_stats->tx_packets, 0);

    g_hmac_stat_control.pre_time = (osal_u32)osal_get_time_stamp_ms();
    return OAL_SUCC;
}

/* 100ms定时器初始化 */
void hmac_throughput_timer_init(void)
{
    hmac_device_stat_stru *hmac_device_stats = NULL;
    /* 防止定时器多次注册 */
    if (g_hmac_stat_control.hmac_freq_timer.is_registerd == OSAL_TRUE) {
        return;
    }
    g_hmac_stat_control.pre_time = (osal_u32)osal_get_time_stamp_ms();
    hmac_device_stats = hmac_stat_get_device_stats();
    osal_adapt_atomic_set(&hmac_device_stats->rx_packets, 0);
    osal_adapt_atomic_set(&hmac_device_stats->rx_bytes, 0);
    osal_adapt_atomic_set(&hmac_device_stats->tx_packets, 0);
    osal_adapt_atomic_set(&hmac_device_stats->tx_bytes, 0);

    frw_create_timer_entry(&g_hmac_stat_control.hmac_freq_timer,
                           hmac_timeout_stat,   // p_timeout_func
                           HMAC_STAT_TIMER_PERIOD,       // timeout, 200ms
                           OSAL_NULL,                       // timeout_arg
                           OSAL_TRUE);                  // en_is_periodic
}

/* 100ms定时器关闭 */
void hmac_throughput_timer_deinit(void)
{
    /* 没注册无须删除 */
    if (g_hmac_stat_control.hmac_freq_timer.is_registerd == OSAL_FALSE) {
        return;
    }

    frw_destroy_timer_entry(&g_hmac_stat_control.hmac_freq_timer);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

