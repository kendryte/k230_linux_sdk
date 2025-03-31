/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: WiFi和bt共存特性文件.
 * Create: 2020-7-9
 */

#ifndef __HMAC_BTCOEX_C__
#define __HMAC_BTCOEX_C__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_btcoex.h"
#include "mac_data.h"
#include "hmac_device.h"
#include "hal_rf.h"
#include "hal_device_fsm.h"

#include "hmac_alg_notify.h"
#include "hmac_scan.h"
#include "hmac_m2s.h"
#include "wlan_spec.h"
#include "wlan_msg.h"
#include "hmac_fcs.h"
#include "oal_netbuf_data.h"
#include "hmac_ext_if.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_btcoex_ba.h"
#include "hmac_btcoex_btsta.h"
#include "hmac_btcoex_m2s.h"
#include "hmac_btcoex_notify.h"
#include "hmac_btcoex_ps.h"
#include "hmac_feature_interface.h"
#include "frw_util_notifier.h"
#include "frw_ext_if.h"
#include "msg_btcoex_rom.h"
#include "hmac_ccpriv.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BTCOEX_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_u8 g_btcoex_wlan_busy_status = OSAL_FALSE;
osal_u8 g_btcoex_stat_flag = OSAL_FALSE;
hmac_btcoex_throughput_stru g_btcoex_throughput = {0};

osal_u8 g_rx_win_size_siso[BTCOEX_RX_WINDOW_SIZE_GRADE_BUTT][BTCOEX_RX_WINDOW_SIZE_INDEX_BUTT] = {
    {2, 4, 8, 64}, {4, 8, 32, 64}
};

osal_u8 g_rx_ba_win_size[WLAN_BW_CAP_BUTT][BTCOEX_RX_WINDOW_SIZE_INDEX_BUTT] = {
    {2, 4, 8, 64},  // 20M
    {4, 8, 32, 64}  // 40M
};

/****************************************
              音乐    电话    数传
    2G/20M      0       0       0
    5G/20M      0       0       0
    2G/40M      0       1       1
    5G/40M      0       0       1
    5G/80M      1       1       1
*****************************************/
osal_u8 g_rx_win_size_grage_siso[WLAN_BAND_BUTT][WLAN_BW_CAP_BUTT][BTCOEX_ACTIVE_MODE_BUTT] = {
    /* 2G */
    /* 20M */ /* 40M */
    /* 音乐, 电话, 数传 */
    {{0, 0, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 1}},
    /* 5G */
    /* 20M */ /* 40M */ /* 80M */
    /* 音乐, 电话, 数传 */
    {{0, 0, 0}, {0, 0, 1}, {1, 1, 1}, {1, 1, 1}}
};
osal_u16 g_aus_btcoex_rate_thresholds_siso[WLAN_BAND_BUTT][WLAN_BW_CAP_BUTT][BTCOEX_RATE_THRESHOLD_BUTT] = {
    /* 2G */
    /* 20M */ /* 40M */
    {{23, 60}, {50, 130}, {0, 0},     {0, 0}},
    /* 5G */
    /* 20M */ /* 40M */ /* 80M */
    {{23, 80}, {50, 160}, {108, 340}, {0, 0}}
};

#if defined(WLAN_MAX_NSS_NUM) && defined(WLAN_SINGLE_NSS) && (WLAN_MAX_NSS_NUM != WLAN_SINGLE_NSS)
osal_u8 g_rx_win_size_mimo[BTCOEX_RX_WINDOW_SIZE_GRADE_BUTT][BTCOEX_RX_WINDOW_SIZE_INDEX_BUTT] = {
    {2, 4, 8, 64}, {8, 16, 32, 64}
};
/****************************************
              音乐    电话    数传
    2G/20M      0       0       0
    5G/20M      0       0       0
    2G/40M      0       1       1
    5G/40M      0       0       1
    5G/80M      1       1       1
*****************************************/
osal_u8 g_rx_win_size_grage_mimo[WLAN_BAND_BUTT][WLAN_BW_CAP_BUTT][BTCOEX_ACTIVE_MODE_BUTT] = {
    /* 2G */
    /* 20M */ /* 40M */
    /* 音乐, 电话, 数传 */
    {{1, 0, 1}, {1, 0, 1}, {1, 1, 1}, {1, 1, 1}},
    /* 5G */
    /* 20M */ /* 40M */ /* 80M */
    /* 音乐, 电话, 数传 */
    {{0, 0, 0}, {0, 0, 1}, {1, 1, 1}, {1, 1, 1}}
};

osal_u16 g_aus_btcoex_rate_thresholds_mimo[WLAN_BAND_BUTT][WLAN_BW_CAP_BUTT][BTCOEX_RATE_THRESHOLD_BUTT] = {
    /* 2G */
    /* 20M */ /* 40M */
    {{23, 70}, {50, 170}, {0, 0},     {0, 0}},
    /* 5G */
    /* 20M */ /* 40M */ /* 80M */
    {{23, 90}, {50, 180}, {108, 360}, {0, 0}}
};
#endif

osal_u16 g_occupied_point[BTCOEX_LINKLOSS_OCCUPIED_NUMBER];

hmac_device_btcoex_stru *g_hmac_device_btcoex;
/*****************************************************************************
  函数实现
*****************************************************************************/
osal_u8 hmac_get_g_btcoex_wlan_busy_status(osal_void)
{
    return g_btcoex_wlan_busy_status;
}
osal_void hmac_set_g_btcoex_wlan_busy_status(osal_u8 wlan_busy)
{
    g_btcoex_wlan_busy_status = wlan_busy;
    return;
}
osal_u8 hmac_get_g_rx_ba_win_size(osal_u32 index1, osal_u32 index2)
{
    return g_rx_ba_win_size[index1][index2];
}
osal_u8 hmac_get_g_rx_win_size_siso(osal_u32 index1, osal_u32 index2)
{
    return g_rx_win_size_siso[index1][index2];
}
osal_u8 hmac_get_g_rx_win_size_grage_siso(osal_u32 index1, osal_u32 index2, osal_u32 index3)
{
    return g_rx_win_size_grage_siso[index1][index2][index3];
}
#if (defined(WLAN_MAX_NSS_NUM) && defined(WLAN_SINGLE_NSS) && WLAN_MAX_NSS_NUM != WLAN_SINGLE_NSS)
osal_u8 hmac_get_g_rx_win_size_mimo(osal_u32 index1, osal_u32 index2)
{
    return g_rx_win_size_mimo[index1][index2];
}
osal_u8 hmac_get_g_rx_win_size_grage_mimo(osal_u32 index1, osal_u32 index2, osal_u32 index3)
{
    return g_rx_win_size_grage_mimo[index1][index2][index3];
}
#endif

/*****************************************************************************
 函数名:      hmac_btcoex_find_all_valid_sta_per_device
 功能描述  : hal device上找到所有满足btcoex处理要求的sta。
           (1)暂时只考虑sta和gc(02只考虑legacy sta， 03新增考虑sta gc )
           (2)处理已经up或者pause，关联上的vap,对于速率统计等，都是基于user的，需要关联成功，
           ps机制要处理前景扫描状态，不采用此接口。
*****************************************************************************/
osal_u8 hmac_btcoex_find_all_valid_sta_per_device(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id,
    osal_u16 vap_id_len)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_index;
    osal_u8 up_vap_num;
    osal_u8 valid_sta_num = 0;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    unref_param(vap_id_len);

    /* 找到所有up的vap设备 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX,
                "vap_id[%d] hmac_btcoex_find_all_valid_sta_per_device::hmac_vap IS NULL.", mac_vap_id[vap_index]);
            continue;
        }

        /* 1.非sta的话,不需要考虑 */
        if (is_ap(hmac_vap)) {
            continue;
        }

        /* 2.valid sta 统计(02和03在此做区分) */
        if (mac_btcoex_check_valid_sta(hmac_vap) == OSAL_TRUE) {
            /* 找到sta，进行赋值 */
            vap_id[valid_sta_num++] = hmac_vap->vap_id;
        }
    }

    return valid_sta_num;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_find_all_valid_ap_per_device
 功能描述  : hal device上找到所有满足btcoex处理要求的ap
             (1)暂时只考虑ap和go(02只考虑legacy ap， 03新增考虑sta go )
*****************************************************************************/
osal_u8 hmac_btcoex_find_all_valid_ap_per_device(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id,
    osal_u16 vap_id_size)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_index;
    osal_u8 up_vap_num;
    osal_u8 valid_ap_num = 0;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    unref_param(vap_id_size);

    /* 找到所有up的vap设备 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX,
                "vap_id[%d] hmac_btcoex_find_all_valid_ap_per_device::hmac_vap IS NULL.", mac_vap_id[vap_index]);
            continue;
        }

        /* 1.非ap的话,不需要考虑 */
        if (is_sta(hmac_vap)) {
            continue;
        }

        /* 2.valid ap 统计(02和03在此做区分) */
        if (mac_btcoex_check_valid_ap(hmac_vap) == OSAL_TRUE) {
            /* 找到ap，进行赋值 */
            vap_id[valid_ap_num++] = hmac_vap->vap_id;
        }
    }

    return valid_ap_num;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_rx_average_rate_calculate
 功能描述  : 统计Wifi的平均速率
*****************************************************************************/
static INLINE__ osal_void hmac_btcoex_rx_average_rate_calculate(hmac_user_btcoex_rx_info_stru *btcoex_wifi_rx_rate_info,
    osal_u32 *rx_rate, osal_u16 *rx_count)
{
    unref_param(rx_rate);

    if (btcoex_wifi_rx_rate_info->rx_rate_stat_count == 0) {
        btcoex_wifi_rx_rate_info->rx_rate_mbps = 0;
        *rx_count = 0;
        return;
    }

    /* 计算完之后需要重新清零，做下一次统计计数 */
    (osal_void)memset_s(btcoex_wifi_rx_rate_info, sizeof(hmac_user_btcoex_rx_info_stru),
                      0, sizeof(hmac_user_btcoex_rx_info_stru));
}

osal_u32 hmac_btcoex_rx_rate_statistics_flag_callback(osal_void *arg)
{
    hal_btcoex_btble_status_stru *btcoex_btble_status = OSAL_NULL;
    hmac_vap_btcoex_rx_statistics_stru *btcoex_rx_stat = OSAL_NULL;
    hmac_user_btcoex_rx_info_stru *btcoex_rx_info = OSAL_NULL;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;

    hmac_vap = (hmac_vap_stru *)arg;
    if (mac_btcoex_check_valid_sta(hmac_vap) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_btcoex_rx_rate_statistics_flag_callback::hmac_user[%d] null.}",
            hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    btcoex_btble_status = hal_btcoex_btble_status();
    btcoex_rx_stat = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_rx_statistics);
    btcoex_rx_info = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_rx_info);

    /* BT业务结束 */
    if (btcoex_btble_status->bt_status.bt_status.bt_ba == OSAL_FALSE) {
        hmac_user_btcoex_delba_stru *btcoex_delba;
        btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);
        btcoex_rx_stat->rx_rate_statistics_flag = OSAL_FALSE;
        frw_destroy_timer_entry(&(btcoex_rx_stat->bt_coex_statistics_timer));
        frw_destroy_timer_entry(&(btcoex_rx_stat->bt_coex_low_rate_timer));

        if (btcoex_delba->ba_size_real_index != BTCOEX_RX_WINDOW_SIZE_INDEX_3) {
            btcoex_delba->ba_size_real_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
            btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
            hmac_btcoex_update_ba_size(hmac_vap, btcoex_delba, btcoex_btble_status);
            hmac_btcoex_delba_trigger(hmac_vap, OSAL_TRUE, btcoex_delba);
        }

        (osal_void)memset_s(btcoex_rx_info, sizeof(hmac_user_btcoex_rx_info_stru),
            0, sizeof(hmac_user_btcoex_rx_info_stru));
        return OAL_SUCC;
    }

    if (btcoex_rx_info->rx_rate_stat_count < BTCOEX_RX_COUNT_LIMIT) {
        (osal_void)memset_s(btcoex_rx_info, sizeof(hmac_user_btcoex_rx_info_stru),
            0, sizeof(hmac_user_btcoex_rx_info_stru));
        return OAL_SUCC;
    }

    btcoex_rx_stat->rx_rate_statistics_timeout = OSAL_TRUE;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_btcoex_assoc_state_syn
 功能描述  : 同步共存下mac的状态
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_config_btcoex_assoc_state_syn(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    hal_btcoex_btble_status_stru *status = OSAL_NULL;
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;
    ble_status_stru *ble_status = OSAL_NULL;
    bt_status_stru *bt_status = OSAL_NULL;
    oal_bool_enum_uint8 dbac_running_ret;

    /* 初始化premmpt能力 */
    hmac_btcoex_init_preempt(hmac_vap, hmac_user);

    status = hal_btcoex_btble_status();
    btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);
    btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
    btcoex_delba->ba_size_real_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;

    /* 初始化delba空间流能力，选择合适删减BA门限 */
    btcoex_delba->user_nss_num = hmac_user->avail_num_spatial_stream;

    oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_config_btcoex_assoc_state_syn::user_nss_num[%d].}",
        hmac_vap->vap_id, btcoex_delba->user_nss_num);

    hmac_btcoex_update_rx_rate_threshold(hmac_vap, btcoex_delba);

    /* 更新当前bt状态信息 可能wifi刚打开，bt业务中断没有过来，要自己本地刷新下那些寄存器，来判断蓝牙业务 */
    hal_update_btcoex_btble_status(status);

    ble_status = &status->ble_status.ble_status;
    bt_status = &status->bt_status.bt_status;

    /* 2.删建聚合逻辑判断，当前dbdc和dbac都不处理，sta侧dbac和dbac都会在该接口之前完成，切信道的dbac不管 */
    /* BTCOEX STA, 并且当前没有切换到c1 siso，并且dbdc或者dbac没有启动，才需要处理 */
    dbac_running_ret = mac_is_dbac_running(hmac_device);
    /* dbdc 2g入网vap判断当前状态，无论是c1/c0SISO还是mimo，都需要处理 */
    if (mac_btcoex_check_valid_vap(hmac_vap) == OSAL_TRUE) {
        /* 1.当前6slot/ldac下即使c1 siso下也不是空分，降低聚合时间保证上行性能稳定;
           2.需要提前配置，dbdc/c1 siso也需要处理，因此放置在这里; 3.纯5g场景不需要处理，反而影响性能
           4. 中断标志过来，本身和6slot是互斥的，不冲突 */
        /* ldac场景下，mimo和siso都需要此档位 ，六时隙时是否为2 */
        hmac_alg_bt_aggr_time_uint8 aggr_time = (bt_status->bt_ldac == 1 || bt_status->bt_6slot == 2) ?
            HMAC_ALG_BT_AGGR_TIME_1MS : HMAC_ALG_BT_AGGR_TIME_OFF;
        hmac_alg_cfg_btcoex_state_notify(hal_device, aggr_time);
    }

    /* 1.刷新状态信息通知蓝牙 */
    if (is_sta(hmac_vap) || (is_ap(hmac_vap) && (hmac_vap->user_nums == 1))) {
        /* 关联时刷新状态信息通知bt,sta关联时 */
        hmac_btcoex_set_freq_and_work_state_hal_device(hal_device);
        hmac_btcoex_set_vap_ps_frame(hmac_vap, OSAL_TRUE);
    }

    /* 这两个条件判断返回，需要在通知bt sta关联状态之后，否则影响bt业务优先级配置，出现5G下性能影响 */
    /* BT无影响聚合相关业务时,恢复wifi控制聚合 */
    if (bt_status->bt_transfer == 0 && bt_status->bt_ba == 0) {
        btcoex_delba->ba_size = 0;
        hmac_btcoex_delba_trigger(hmac_vap, OSAL_FALSE, btcoex_delba);
        oam_warning_log1(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_config_btcoex_assoc_state_syn::bt not working, ba size to default.}", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    if (mac_btcoex_check_valid_sta(hmac_vap) == OSAL_FALSE || hal_m2s_check_btcoex_on(hal_device) == OSAL_TRUE ||
        (dbac_running_ret == OSAL_TRUE)) {
        oam_warning_log3(0, OAM_SF_COEX,
            "{hmac_config_btcoex_assoc_state_syn::ba process skip! valid_sta[%d]m2s_on[%d]DBAC[%d].}",
            mac_btcoex_check_valid_sta(hmac_vap), hal_m2s_check_btcoex_on(hal_device), dbac_running_ret);
        return OAL_SUCC;
    }

    hmac_config_btcoex_assoc_state_proc(hmac_vap, ble_status, btcoex_delba, bt_status, status);
    hmac_btcoex_set_dev_ps_frame(hal_device);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_wlan_priority
 功能描述  : 软件设定高优先级保护
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_set_wlan_priority(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 value,
    osal_u8 timeout_ms)
{
    hmac_vap_btcoex_occupied_stru *btcoex_occupied = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_occupied);
    osal_u32 now_ms;
    oal_bool_enum_uint8 set_on = OSAL_FALSE;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (hmac_vap->channel.band == WLAN_BAND_5G) {
        /* 5G不需要拉priority处理 */
        return;
    }

    /* 定时器超时处理函数等流程中，可能vap已经删除，此时才执行，hal device是空指针，需要增加判断保护  */
    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_set_wlan_priority:: hal_device null}", hmac_vap->vap_id);
        return;
    }

    /* 需要拉高prio */
    if (value == OSAL_TRUE) {
        /* 当前没有拉高prio并且可以拉高prio */
        if (btcoex_occupied->prio_occupied_state == OSAL_FALSE) {
            /* 尽量不要和上一次拉低相隔太近 */
            now_ms = (osal_u32)osal_get_time_stamp_ms();
            if (now_ms - btcoex_occupied->timestamp > BTCOEX_PRI_DURATION_TIME) {
                frw_destroy_timer_entry(&btcoex_occupied->bt_coex_priority_timer);
                /* 启动priority定时器 */
                frw_create_timer_entry(&btcoex_occupied->bt_coex_priority_timer,
                    hmac_btcoex_wlan_priority_timeout_callback, timeout_ms, (osal_void *)hmac_vap,
                    OSAL_FALSE);
                btcoex_occupied->prio_occupied_state = OSAL_TRUE;
                set_on = OSAL_TRUE;
            }
        }
    } else { /* 需要拉低prio */
        if (btcoex_occupied->prio_occupied_state == OSAL_TRUE) {
            btcoex_occupied->prio_occupied_state = OSAL_FALSE;
            btcoex_occupied->timestamp = (osal_u32)osal_get_time_stamp_ms();
        }
        set_on = OSAL_TRUE;
    }

    if (set_on == OSAL_TRUE) {
        /* 设置软件配置优先级 */
        hal_set_btcoex_hw_priority_en(((value == OSAL_TRUE) ? OSAL_FALSE : OSAL_TRUE));
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_wlan_priority_timeout_callback()
 功能描述  : 软件设定高优先级保护超时函数
*****************************************************************************/
osal_u32 hmac_btcoex_wlan_priority_timeout_callback(osal_void *arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg;

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_wlan_priority_timeout_callback:: hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_btcoex_set_wlan_priority(hmac_vap, OSAL_FALSE, 0);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 处理vap_up，给BT发送中断
*****************************************************************************/
OSAL_STATIC osal_bool hmac_btcoex_vap_up_handle(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    oal_bool_enum_uint8 state_change = OSAL_FALSE;
    hal_btcoex_btble_status_stru *status = hal_btcoex_btble_status();

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_AP_MODE, OSAL_TRUE);

        /* 状态发生变化 */
        state_change = OSAL_TRUE;
    }

    if (hmac_device->vap_num == 1) {
        hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_ON, OSAL_TRUE);

        /* 状态发生变化 */
        state_change = OSAL_TRUE;
    }

    if (state_change == OSAL_TRUE) {
        /* 状态变化，通知bt */
        hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);
    }

    /* 更新btble状态 */
    hal_update_btcoex_btble_status(status);

    oam_warning_log_alter(0, OAM_SF_COEX,
        "{hmac_btcoex_vap_up_handle::::status resv:%d, send:%d, data_transfer:%d, a2dp:%d, bt_on:%d.}",
        /* 5代表5后面有5个待打印参数 */
        5, status->bt_status.bt_status.bt_data_rcv,
        status->bt_status.bt_status.bt_data_send,
        status->bt_status.bt_status.bt_transfer,
        status->bt_status.bt_status.bt_a2dp,
        status->bt_status.bt_status.bt_on);

    if (status->bt_status.bt_status.bt_on != 0) {
        /* 开启MAC BT共存 */
        hal_set_btcoex_sw_all_abort_ctrl(OSAL_TRUE);
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_update_rx_rate_threshold
 功能描述  : 获取速率门限
*****************************************************************************/
osal_void hmac_btcoex_update_rx_rate_threshold(const hmac_vap_stru *hmac_vap, hmac_user_btcoex_delba_stru *btcoex_delba)
{
    wlan_channel_band_enum_uint8 band;
    wlan_bw_cap_enum_uint8 bandwidth;

    band = hmac_vap->channel.band;
    hmac_vap_get_bandwidth_cap_etc(hmac_vap, &bandwidth);

    if ((band >= WLAN_BAND_BUTT) || (bandwidth >= WLAN_BW_CAP_BUTT)) {
        oam_error_log3(0, 0, "vap_id[%d] {hmac_btcoex_update_rx_rate_threshold::band %d, bw %d exceed scale!}",
            hmac_vap->vap_id, band, bandwidth);
        return;
    }

    if (btcoex_delba->user_nss_num >= WLAN_DOUBLE_NSS) {
#if defined(WLAN_MAX_NSS_NUM) && defined(WLAN_SINGLE_NSS) && (WLAN_MAX_NSS_NUM != WLAN_SINGLE_NSS)
        btcoex_delba->rx_rate_threshold_min =
            g_aus_btcoex_rate_thresholds_mimo[band][bandwidth][BTCOEX_RATE_THRESHOLD_MIN];
        btcoex_delba->rx_rate_threshold_max =
            g_aus_btcoex_rate_thresholds_mimo[band][bandwidth][BTCOEX_RATE_THRESHOLD_MAX];
#endif
    } else {
        btcoex_delba->rx_rate_threshold_min =
            g_aus_btcoex_rate_thresholds_siso[band][bandwidth][BTCOEX_RATE_THRESHOLD_MIN];
        btcoex_delba->rx_rate_threshold_max =
            g_aus_btcoex_rate_thresholds_siso[band][bandwidth][BTCOEX_RATE_THRESHOLD_MAX];
    }

    oam_warning_log4(0, OAM_SF_COEX,
        "{hmac_btcoex_update_rx_rate_threshold:: min: %d, max: %d, band %d, bandwidth %d.}",
        btcoex_delba->rx_rate_threshold_min, btcoex_delba->rx_rate_threshold_max, band, bandwidth);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_user_spatial_stream_change_notify
 功能描述  : user空间流变化，需要刷新bt下速率门限值(siso还是mimo)
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_user_spatial_stream_change_notify(const hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user)
{
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;

    if (mac_btcoex_check_valid_sta(hmac_vap) == OSAL_FALSE) {
        return;
    }

    btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);
    btcoex_delba->user_nss_num = hmac_user->avail_num_spatial_stream;

    /* user空间流发生变化，重新刷新速率门限 */
    hmac_btcoex_update_rx_rate_threshold(hmac_vap, btcoex_delba);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_lower_rate_aggregation_adjust
 功能描述  : 接收降速后处理,调整聚合
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_lower_rate_aggregation_adjust(const hal_to_dmac_device_stru *hal_device,
    const hal_btcoex_btble_status_stru *btble_status, hmac_user_btcoex_delba_stru *btcoex_delba,
    hmac_vap_stru *hmac_vap, osal_u32 rx_rate)
{
    osal_u32 th_min = btcoex_delba->rx_rate_threshold_min;
    osal_u32 th_max = btcoex_delba->rx_rate_threshold_max;

    if (hal_device->hal_alg_stat.alg_distance_stat < HAL_ALG_USER_DISTANCE_FAR) {
        /* BA聚合个数调整 1.DEC下速率小于min；2.INC下速率大于max；3.DEC下实际聚合等于index3，速率小于等于 min+max%2 */
        if ((btcoex_delba->ba_size_tendence == BTCOEX_RX_WINDOW_SIZE_DECREASE &&  (rx_rate < th_min)) ||
            ((btcoex_delba->ba_size_tendence == BTCOEX_RX_WINDOW_SIZE_INCREASE) &&  (rx_rate > th_max)) ||
            ((btcoex_delba->ba_size_tendence == BTCOEX_RX_WINDOW_SIZE_DECREASE) &&
            (btcoex_delba->ba_size_real_index == BTCOEX_RX_WINDOW_SIZE_INDEX_3) &&
            (rx_rate < (th_min + (th_max >> 1))))) {
            btcoex_delba->ba_size_real_index = btcoex_delba->ba_size_expect_index;
            hmac_btcoex_update_ba_size(hmac_vap, btcoex_delba, btble_status);

            /* 主动触发删减BA逻辑，需要在收到addba rsp帧时候恢复此标记 */
            hmac_btcoex_delba_trigger(hmac_vap, OSAL_TRUE, btcoex_delba);

            oam_warning_log_alter(0, OAM_SF_COEX,
                "{hmac_btcoex_lower_rate_callback:change:%d,rate:%d,tendence:%d,indx:%d,threshMin:%d,threshMax:%d}",
                /* 6代表6后面有6个待打印参数 */
                6, btcoex_delba->ba_size, rx_rate, btcoex_delba->ba_size_tendence,
                btcoex_delba->ba_size_real_index, th_min, th_max);
        } else {
            /* 保持现有聚合不变 */
            btcoex_delba->ba_size_expect_index = btcoex_delba->ba_size_real_index;
            hmac_btcoex_update_ba_size(hmac_vap, btcoex_delba, btble_status);
            oam_warning_log3(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_lower_rate_callback:keep at:%d, rate:%d}",
                hmac_vap->vap_id, btcoex_delba->ba_size, rx_rate);
        }
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_lower_rate_callback
 功能描述  : 接收降速后处理
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_lower_rate_callback(osal_void *arg)
{
    osal_u32 rx_rate = 0;
    osal_u16 rx_count = 0;

    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)arg;
    hal_to_dmac_chip_stru *hal_chip = hmac_vap->hal_chip;
    hmac_user_stru *hmac_user = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;
    hal_btcoex_btble_status_stru *btble_status = OSAL_NULL;
    hmac_user_btcoex_rx_info_stru *btcoex_rx_info = OSAL_NULL;

    if (hal_chip == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_init_preempt:: HAL_CHIP null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {DmacBtcoexLowerRateCallback:NULL%d",
            hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_btcoex_lower_rate_callback::hal_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);
    btble_status = hal_btcoex_btble_status();
    btcoex_rx_info = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_rx_info);

    if (btcoex_delba->get_addba_req_flag == OSAL_FALSE) {
        hmac_btcoex_rx_average_rate_calculate(btcoex_rx_info, &rx_rate, &rx_count);
        hmac_btcoex_lower_rate_aggregation_adjust(hal_device, btble_status, btcoex_delba, hmac_vap, rx_rate);
    } else {
        btcoex_delba->ba_size_real_index = btcoex_delba->ba_size_expect_index;
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_btcoex_lower_rate_callback:rece addba req before delba}",
            hmac_vap->vap_id);
    }

    frw_timer_restart_timer(
        &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_rx_statistics.bt_coex_statistics_timer),
        BTCOEX_RX_STATISTICS_TIME, OSAL_TRUE);

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_btcoex_lower_rate_switch(const hmac_vap_stru *hmac_vap,
    hmac_user_btcoex_delba_stru *btcoex_delba, osal_u32 rx_rate, osal_u8 is_siso)
{
    osal_u32 rate_threshold_min = btcoex_delba->rx_rate_threshold_min;
    osal_u32 rate_threshold_max = btcoex_delba->rx_rate_threshold_max;
    switch (btcoex_delba->ba_size_expect_index) {
        case BTCOEX_RX_WINDOW_SIZE_INDEX_3:
            if (rx_rate < (rate_threshold_min + (rate_threshold_max >> 1))) {
                btcoex_delba->ba_size_expect_index--;
                btcoex_delba->ba_size_tendence = BTCOEX_RX_WINDOW_SIZE_DECREASE;
            }
            break;

        /* 降低到2档之后，期望稳定在2档，只能降 */
        case BTCOEX_RX_WINDOW_SIZE_INDEX_2:
            if (rx_rate < rate_threshold_min) {
                btcoex_delba->ba_size_expect_index--;
                btcoex_delba->ba_size_tendence = BTCOEX_RX_WINDOW_SIZE_DECREASE;
            }
            break;

        /* 降低到1档之后，可以退到0档，0档之后不再变化，也可以探测到2档，尽量稳定在2档 */
        case BTCOEX_RX_WINDOW_SIZE_INDEX_1:
            if (rx_rate < rate_threshold_min) {
                btcoex_delba->ba_size_expect_index--;
                btcoex_delba->ba_size_tendence = BTCOEX_RX_WINDOW_SIZE_DECREASE;
            } else if (rx_rate > rate_threshold_max) {
                btcoex_delba->ba_size_expect_index++;
                btcoex_delba->ba_size_tendence = BTCOEX_RX_WINDOW_SIZE_INCREASE;
            }
            break;

        case BTCOEX_RX_WINDOW_SIZE_INDEX_0:
            if ((is_siso == OSAL_TRUE) && (rx_rate > rate_threshold_max)) {
                btcoex_delba->ba_size_expect_index++;
                btcoex_delba->ba_size_tendence = BTCOEX_RX_WINDOW_SIZE_INCREASE;
            } else {
                oam_warning_log1(0, OAM_SF_COEX,
                    "vap_id[%d] {hmac_btcoex_lower_rate_switch::KEEP at BTCOEX_RX_WINDOW_SIZE_INDEX_0.}",
                    hmac_vap->vap_id);
            }
            break;

        default:
            oam_error_log2(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_lower_rate_switch::ba_size_expect_index error:%d.}", hmac_vap->vap_id,
                btcoex_delba->ba_size_expect_index);
            break;
    }
}

OSAL_STATIC osal_void hmac_btcoex_lower_rate_distance(hmac_vap_stru *hmac_vap,
    hmac_user_btcoex_delba_stru *btcoex_delba, osal_u32 rx_rate, osal_u16 rx_count)
{
    hmac_vap_btcoex_rx_statistics_stru *btcoex_rx_stat =
        &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_rx_statistics);

    hal_to_dmac_chip_stru *hal_chip = hmac_vap->hal_chip;
    if (hal_chip == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_btcoex_lower_rate_process:: GET_HAL_CHIP null}", hmac_vap->vap_id);
        return;
    }
    /* 处理近场速率 */
    if (hmac_vap->hal_device->hal_alg_stat.alg_distance_stat < HAL_ALG_USER_DISTANCE_FAR) {
        osal_u8 is_siso = OSAL_FALSE;
        hal_btcoex_btble_status_stru *status = hal_btcoex_btble_status();
        bt_status_stru *bt_status = &(status->bt_status.bt_status);

        /* 1. 电话固定聚合 */
        if (status->bt_status.bt_status.bt_6slot != 0) {
            return;
        }

        /* 初始配置 */
        btcoex_delba->ba_size_tendence = BTCOEX_RX_WINDOW_SIZE_HOLD;
        btcoex_delba->ba_size_expect_index = btcoex_delba->ba_size_real_index;

        /* 2. 音乐2G mimo 20M暂时固定2档,40M固定3档 */
        if ((hmac_vap->channel.band == WLAN_BAND_2G) && (bt_status->bt_a2dp != 0) &&
            (btcoex_delba->user_nss_num == WLAN_DOUBLE_NSS)) {
            hmac_btcoex_lower_rate_switch(hmac_vap, btcoex_delba, rx_rate, is_siso);
        } else if (hmac_vap->channel.band == WLAN_BAND_5G) {
            /* 3.5G不处理 */
        } else { /* 4. siso场景 */
            is_siso = OSAL_TRUE;
            hmac_btcoex_lower_rate_switch(hmac_vap, btcoex_delba, rx_rate, is_siso);
        }

        if (btcoex_delba->ba_size_tendence != BTCOEX_RX_WINDOW_SIZE_HOLD) {
            hmac_btcoex_update_ba_size(hmac_vap, btcoex_delba, status);
            hmac_btcoex_delba_trigger(hmac_vap, OSAL_FALSE, btcoex_delba);
            oam_warning_log4(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_lower_rate_process::ba_size will change to:%d in 5s,rate:%d,count:%d.}",
                hmac_vap->vap_id, btcoex_delba->ba_size, rx_rate, rx_count);
            btcoex_delba->get_addba_req_flag = OSAL_FALSE;
            frw_timer_stop_timer(&(btcoex_rx_stat->bt_coex_statistics_timer));
            frw_create_timer_entry(&(btcoex_rx_stat->bt_coex_low_rate_timer), hmac_btcoex_lower_rate_callback,
                BTCOEX_RX_LOW_RATE_TIME, (osal_void *)hmac_vap, OSAL_FALSE);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_bt_low_rate_process
 功能描述  : 接收降速后处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_lower_rate_process(hmac_vap_stru *hmac_vap)
{
    osal_u32 rx_rate = 0;
    osal_u16 rx_count = 0;
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_vap_btcoex_rx_statistics_stru *btcoex_rx_stat = OSAL_NULL;
    hmac_user_btcoex_rx_info_stru *btcoex_rx_info = OSAL_NULL;
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;

    if (mac_btcoex_check_valid_sta(hmac_vap) == OSAL_FALSE) {
        /* 速率统计发现不存在要求vap时应该对vap下速率统计结果清零，user去关联会自动清 */
        return;
    }

    /* STA模式获取user */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_lower_rate_process::hmac_user[%d] null.}",
            hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        return;
    }

    btcoex_rx_stat = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_rx_statistics);
    btcoex_rx_info = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_rx_info);
    btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);

    /* 累加接收速率 */
    if (btcoex_rx_stat->rx_rate_statistics_flag != OSAL_TRUE) {
        return;
    }
    if (hmac_user->rx_rate != 0) {
        /* 如果用户不存在，去关联，pst_dmac_user_btcoex_rx_info也会被清零 */
        btcoex_rx_info->rx_rate_mbps += hmac_user->rx_rate;
        btcoex_rx_info->rx_rate_stat_count++;
    }

    /* 计算接收速率 */
    if (btcoex_rx_stat->rx_rate_statistics_timeout == 0) {
        return;
    }
    btcoex_rx_stat->rx_rate_statistics_timeout = OSAL_FALSE;
    hmac_btcoex_rx_average_rate_calculate(btcoex_rx_info, &rx_rate, &rx_count);
    hmac_btcoex_lower_rate_distance(hmac_vap, btcoex_delba, rx_rate, rx_count);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_release_rx_prot
 功能描述  : 共存对于关键帧的操作
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_release_rx_prot(hmac_vap_stru *hmac_vap, osal_u8 data_type)
{
    bt_status_stru *bt_status = OSAL_NULL;
    hal_to_dmac_chip_stru *hal_chip = OSAL_NULL;
    hal_btcoex_btble_status_stru *status = OSAL_NULL;

    hal_chip = hmac_vap->hal_chip;
    if (hal_chip == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_btcoex_release_rx_prot:: GET_HAL_CHIP null}", hmac_vap->vap_id);
        return;
    }
    status = hal_btcoex_btble_status();

    bt_status = &(status->bt_status.bt_status);
    if ((bt_status->bt_on == OSAL_FALSE) || (data_type == MAC_DATA_BUTT)) {
        return;
    }

    if (data_type <= MAC_DATA_ARP_RSP) {
        {
            hmac_btcoex_set_wlan_priority(hmac_vap, OSAL_FALSE, 0);
        }
    }
}

/*****************************************************************************
 功能描述  : 共存对于关键帧的发送保护
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_tx_mgmt_frame(const hmac_vap_stru *hmac_vap,
    const mac_ieee80211_frame_stru *mac_header)
{
    hal_to_dmac_chip_stru *hal_chip = hmac_vap->hal_chip;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    hal_btcoex_btble_status_stru *status = OSAL_NULL;
    bt_status_stru *bt_status = OSAL_NULL;
    osal_u8 mgmt_type;

    if ((hal_chip == OSAL_NULL) || (hal_device == OSAL_NULL)) {
        oam_warning_log1(0, 0, "{hmac_btcoex_tx_mgmt_frame:: null point, chip[%x]}", (uintptr_t)hal_chip);
        return;
    }

    /* 当前软件仅对管理帧做保护 */
    status = hal_btcoex_btble_status();
    bt_status = &(status->bt_status.bt_status);
    if ((bt_status->bt_on == OSAL_FALSE) || (mac_header->frame_control.type != WLAN_MANAGEMENT)) {
        return;
    }

    mgmt_type = mac_header->frame_control.sub_type;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* probe req广播帧不收ack，不拉优先级，ldac和sco都可能造成蓝牙卡顿，可能影响wifi扫描个数 */
        if ((mgmt_type == WLAN_AUTH) || (mgmt_type == WLAN_ASSOC_REQ) || (mgmt_type == WLAN_REASSOC_REQ) ||
            (mgmt_type == WLAN_DEAUTH) || (mgmt_type == WLAN_DISASOC) || (mgmt_type == WLAN_ACTION)) {
            hmac_btcoex_set_mgmt_priority(hmac_vap, BTCOEX_PRIO_TIMEOUT_10MS);
        }
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
            /* probe rsp以及P2P相关action帧在空口中会大量发送,拉优先级太久会造成蓝牙卡顿 */
        if ((mgmt_type == WLAN_ASSOC_RSP) || (mgmt_type == WLAN_AUTH) || (mgmt_type == WLAN_BEACON) ||
            (mgmt_type == WLAN_DISASOC) || (mgmt_type == WLAN_DEAUTH) || (mgmt_type == WLAN_REASSOC_RSP)) {
            hmac_btcoex_set_mgmt_priority(hmac_vap, BTCOEX_PRIO_TIMEOUT_10MS);
        }
    }
}

OSAL_STATIC osal_void hmac_btcoex_protect_frame(hmac_vap_stru *hmac_vap, osal_u8 data_type)
{
    switch (data_type) {
        case MAC_DATA_DHCP:
            hmac_btcoex_set_wlan_priority(hmac_vap, OSAL_TRUE, BTCOEX_PRIO_TIMEOUT_100MS);
            break;

        case MAC_DATA_ARP_RSP:
        case MAC_DATA_ARP_REQ:
            /* arp rsp帧发出后没有响应帧来拉低，此时保证发送正常即可，采用管理帧发送保护即可 */
            // 多用户时，arp帧交互转发较多，也可能造成拉时间过长
            hmac_btcoex_set_wlan_priority(hmac_vap, OSAL_TRUE, BTCOEX_PRIO_TIMEOUT_50MS);
            break;

        /* 为提高p2p配对成功率/漫游成功率，在非BT电话场景下，保护EAPOL帧收发 */
        case MAC_DATA_EAPOL:
            /* 需要等待对端响应帧 */
            hmac_btcoex_set_wlan_priority(hmac_vap, OSAL_TRUE, BTCOEX_PRIO_TIMEOUT_100MS);
            break;
        default:
            break;
        /* 三类关键帧发送时间太短，此类关键帧发送状态通知给bt意义不大 */
    }
}


OSAL_STATIC osal_void hmac_btcoex_protect_tx_vip_frame(hmac_vap_stru *hmac_vap, osal_u8 data_type)
{
    hal_btcoex_btble_status_stru *status = hal_btcoex_btble_status();

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_btcoex_protect_frame(hmac_vap, data_type);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA &&
        status->bt_status.bt_status.bt_ldac == OSAL_FALSE) {
        hmac_btcoex_protect_frame(hmac_vap, data_type);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_tx_vip_frame_ect
 功能描述  : 共存对于关键帧的发送保护
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_tx_vip_frame_ect(hmac_vap_stru *hmac_vap, const oal_netbuf_head_stru *buff_head,
    osal_u8 mpdu_num)
{
    hal_to_dmac_chip_stru *hal_chip = OSAL_NULL;
    oal_netbuf_stru *netbuf = OSAL_NULL;
    oal_netbuf_stru *netbuf_tmp = OSAL_NULL;
    osal_u8 data_type;
    mac_tx_ctl_stru *cb = OSAL_NULL;
    osal_u8 mpdu_idx;

    hal_chip = hmac_vap->hal_chip;
    if (hal_chip == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_tx_vip_frame:: GET_HAL_CHIP null}", hmac_vap->vap_id);
        return;
    }

    /* EAPOL帧，DHCP和ARP帧的共存保护 */
    netbuf = (oal_netbuf_stru *)buff_head->next;
    for (mpdu_idx = 0; mpdu_idx < mpdu_num; mpdu_idx++) {
        if (netbuf == OSAL_NULL) {
            return;
        }

        netbuf_tmp = oal_get_netbuf_next(netbuf);
        cb = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        netbuf = netbuf_tmp;
        if (cb->frame_type != WLAN_CB_FRAME_TYPE_DATA) {
            continue;
        }

        data_type = cb->frame_subtype;
        if (data_type > MAC_DATA_ARP_REQ) {
            continue;
        }

        hmac_btcoex_protect_tx_vip_frame(hmac_vap, data_type);
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_void hmac_btcoex_tx_vip_frame(hmac_vap_stru *hmac_vap,
    const oal_netbuf_head_stru *buff_head, osal_u8 mpdu_num)
{
    hal_btcoex_btble_status_stru *status = hal_btcoex_btble_status();
    if (status->bt_status.bt_status.bt_on == 0) {
        return;
    }
    hmac_btcoex_tx_vip_frame_ect(hmac_vap, buff_head, mpdu_num);
}
/*****************************************************************************
 函 数 名  : hmac_btcoex_linkloss_init
 功能描述  : init BTCOEX下linkloss参数
*****************************************************************************/
OSAL_STATIC osal_bool hmac_btcoex_linkloss_init(osal_void *notify_data)
{
    osal_u8 loop_idx;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_vap_btcoex_occupied_stru *vap_btcoex_occupied =
        &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_occupied);
    /* 该字段在高5位，故右移3 */
    g_occupied_point[0] = hmac_vap->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_BT].int_linkloss_threshold >> 3;

    for (loop_idx = 0; loop_idx < BTCOEX_LINKLOSS_OCCUPIED_NUMBER - 1; loop_idx++) {
        g_occupied_point[loop_idx + 1] = g_occupied_point[loop_idx] + g_occupied_point[0];
    }

    vap_btcoex_occupied->linkloss_index = 1;
    vap_btcoex_occupied->linkloss_occupied_times = 0;
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_linkloss_update_threshold
 功能描述  : update BTCOEX下linkloss参数
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_linkloss_update_threshold(hmac_vap_stru *hmac_vap)
{
    osal_u8 loop_idx;
    hmac_vap_btcoex_occupied_stru *btcoex_occupied = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_occupied);

    g_occupied_point[0] = get_current_linkloss_threshold(hmac_vap) >> 3; /* 该字段在高5位，故右移3 */

    for (loop_idx = 0; loop_idx < BTCOEX_LINKLOSS_OCCUPIED_NUMBER - 1; loop_idx++) {
        g_occupied_point[loop_idx + 1] = g_occupied_point[loop_idx] + g_occupied_point[0];
    }

    btcoex_occupied->linkloss_index = 1;
    btcoex_occupied->linkloss_occupied_times = 0;
}

/* 由于host侧tbtt消息存在时延，发消息到device侧，将下个tbtt时间WiFi优先级拉最高 */
OSAL_STATIC osal_void hmac_btcoex_device_occupied(hmac_vap_stru *hmac_vap, osal_u16 utime)
{
    osal_s32 ret;
    frw_msg msg = {0};

    /* 5G不需要拉occupy处理 */
    if (hmac_vap->channel.band == WLAN_BAND_5G) {
        oam_warning_log1(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_device_occupied:: 5G donot need to set occupied.}",
            hmac_vap->vap_id);
        return;
    }

    msg.data = (osal_u8 *)&utime;
    msg.data_len = (osal_u16)sizeof(osal_u16);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_BTCOEX_SYNC_OCCUPIED_TIME, &msg, OSAL_FALSE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_device_occupoed:: frw_send_msg [%d].}", ret);
    }
}

/*****************************************************************************
 功能描述  : BTCOEX下beacon occupied参数更新处理
*****************************************************************************/
OSAL_STATIC osal_bool hmac_btcoex_beacon_occupied_handler(osal_void *notify_data)
{
    osal_u32 beacon_count_new;
    hmac_vap_btcoex_occupied_stru *btcoex_occupied = OSAL_NULL;
    hal_btcoex_btble_status_stru *status = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    status = hal_btcoex_btble_status();
    /* 入参确认不为空 */
    if (status->bt_status.bt_status.bt_on == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 每5个beacon发送失败上报一次BEACON MISS中断，所以在发出了5个beacon后，计数减1 */
    beacon_count_new = hal_btcoex_update_ap_beacon_count();
    btcoex_occupied = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_occupied);
    if (beacon_count_new - btcoex_occupied->ap_beacon_count >= 5) { /* 间隔5个beacon上报一次中断 */
        /* 如果存在大于5的情况，则可能出现过TBTT中断丢失，实际确有发生beacon miss计数无法清零的情况 */
        if (beacon_count_new - btcoex_occupied->ap_beacon_count > 5) { /* 间隔5个beacon上报一次中断 */
            oam_warning_log3(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_beacon_occupied_handler::TBTT interrupt miss: beacon cnt [%d] record[%d]!}",
                hmac_vap->vap_id, beacon_count_new, btcoex_occupied->ap_beacon_count);
        }

        btcoex_occupied->ap_beacon_count = beacon_count_new;
        btcoex_occupied->beacon_miss_cnt = oal_sub(btcoex_occupied->beacon_miss_cnt, 1);
    } else if (beacon_count_new < btcoex_occupied->ap_beacon_count) {
        /* 防止硬件的beacon计数翻转 */
        btcoex_occupied->ap_beacon_count = beacon_count_new;
    }

    if (status->bt_status.bt_status.bt_page != 0) {
        /* 达到30个beacon miss就拉高occupied */
        if (btcoex_occupied->beacon_miss_cnt > 5) { /* 每上报5次Beacon miss对应30个beacon帧丢失 */
            hmac_btcoex_device_occupied(hmac_vap, 10000); // 10000表示10ms
        }
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_beacon_miss_handler
 功能描述  : BTCOEX下beacon miss处理
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_beacon_miss_handler(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_vap_stru *hmac_vap_tmp = OSAL_NULL;
    osal_u8 beacon_miss_cnt;
    osal_u8 vap_idx;
    osal_u8 valid_vap_num;
    osal_u8         mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru     *hmac_vap_all[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {OSAL_NULL};
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;

    unref_param(msg);

    /* valid的vap设备都做对应处理 */
    valid_vap_num = hmac_btcoex_find_all_valid_ap_per_device(hal_device, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);

    for (vap_idx = 0; vap_idx < valid_vap_num; vap_idx++) {
        hmac_vap_all[vap_idx] = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_vap_all[vap_idx] == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_COEX, "{hmac_btcoex_beacon_miss_handler::hmac_vap IS NULL.}");
            return OAL_FAIL;
        }

        hmac_vap_tmp = (hmac_vap_stru *)(hmac_vap_all[vap_idx]);
        hmac_btcoex_get_vap_info(hmac_vap_tmp)->hmac_vap_btcoex_occupied.beacon_miss_cnt++;
        beacon_miss_cnt = hmac_btcoex_get_vap_info(hmac_vap_tmp)->hmac_vap_btcoex_occupied.beacon_miss_cnt;
        oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_beacon_miss_handler:: miss cnt:%d}",
            mac_vap_id[vap_idx], beacon_miss_cnt);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_occupied_period
 功能描述  : 配置occupied_period值，03在5G下不执行
*****************************************************************************/
OSAL_STATIC osal_void  hmac_btcoex_set_occupied_period(const hmac_vap_stru *hmac_vap, osal_u16 occupied_period)
{
    if (hmac_vap->channel.band == WLAN_BAND_5G) {
        /* 5G不需要拉occupy处理 */
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_btcoex_set_occupied_period:: 5G donot need to set occupied.}",
            hmac_vap->vap_id);
    } else {
        hal_set_btcoex_occupied_period(occupied_period);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_linkloss_occupied_process
 功能描述  : BTCOEX下linkloss occupied参数更新处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_linkloss_occupied_process(const hal_to_dmac_chip_stru *hal_chip,
    const hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap)
{
    hmac_vap_linkloss_stru *linkloss_info = OSAL_NULL;
    hmac_vap_btcoex_occupied_stru *btcoex_occupied = OSAL_NULL;
    osal_u16 linkloss_threshold, linkloss, linkloss_tmp;
    osal_u8 linkloss_times;

    if (hal_btcoex_btble_status()->bt_status.bt_status.bt_on == OSAL_FALSE) {
        return;
    }

    linkloss_info = &(hmac_vap->linkloss_info);
    btcoex_occupied = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_occupied);
    linkloss_threshold = get_current_linkloss_threshold(hmac_vap);
    linkloss = get_current_linkloss_cnt(hmac_vap);
    linkloss_times = linkloss_info->linkloss_times;
    unref_param(hal_device);
    unref_param(hal_chip);

    if (linkloss_threshold == 0) {
        return;
    }
    /* linkloss 大于1/4门限值进行处理 */
    if (linkloss < (linkloss_threshold >> 2)) { /* 右移2位，取linkloss_threshold高6位做比较 */
        return;
    }

    /* 门限值为异常值，即beacon周期很大 */
    if (linkloss_threshold <= WLAN_LINKLOSS_MIN_THRESHOLD) {
        hmac_btcoex_set_occupied_period(hmac_vap, COEX_LINKLOSS_OCCUP_PERIOD);
        oam_warning_log4(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_linkloss_occupied_process::occupied time=[%d], linkloss=[%d], threshold=[%d].}",
            hmac_vap->vap_id, COEX_LINKLOSS_OCCUP_PERIOD, linkloss, linkloss_threshold);
        return;
    }

    linkloss_tmp = g_occupied_point[btcoex_occupied->linkloss_index] + btcoex_occupied->occupied_times;
    if (linkloss > linkloss_tmp) {
        btcoex_occupied->occupied_times = 0;
        btcoex_occupied->linkloss_index++;
    } else if (linkloss == linkloss_tmp) {
        /* 在整个linkloss阶段拉高occupied 6-7个阶段, 每个阶段为300ms，每100ms拉一次20ms, 连续拉3次 */
        btcoex_occupied->occupied_times += linkloss_times;
        if (btcoex_occupied->occupied_times > (linkloss_times << 1)) {
            btcoex_occupied->occupied_times = 0;
            btcoex_occupied->linkloss_index++;
            if (btcoex_occupied->linkloss_index > BTCOEX_LINKLOSS_OCCUPIED_NUMBER - 1) {
                btcoex_occupied->linkloss_index = 1;
            }
        }
        hmac_btcoex_device_occupied(hmac_vap, COEX_LINKLOSS_OCCUP_PERIOD);
        oam_warning_log4(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_linkloss_occupied_process::occupied time=[%d], linkloss=[%d], threshold=[%d].}",
            hmac_vap->vap_id, COEX_LINKLOSS_OCCUP_PERIOD, linkloss, linkloss_threshold);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_rx_rate_process_check
 功能描述  : rx方向检查是否需要进行rate统计处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_rx_rate_process_check(hmac_vap_stru *hmac_vap, osal_u8 frame_subtype,
    osal_u8 data_type, oal_bool_enum_uint8 ampdu)
{
    /* 业务数据帧才做统计，qos null data等默认采用最低速率发送，不纳入统计，提高业务数据帧准确性 */
    if (frame_subtype != WLAN_QOS_NULL_FRAME && data_type > MAC_DATA_ARP_REQ) {
        if (ampdu == OSAL_TRUE) {
            hmac_btcoex_lower_rate_process(hmac_vap);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_roam_succ_handler
 功能描述  : BTCOEX下roam成功之后处理
             (1)从2G漫游到5G，03需要清统计信息，认为无影响
             (2)从5G漫游到2G, 03需要重新开启统计信息
             (3)漫游目前只是处理legacy sta, 该接口根据当前band来清状态或者配置参数,02不需要处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_roam_succ_handler(hmac_vap_stru *hmac_vap)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;

    /* 漫游一定是legacy sta，直接按照如下方式获取用户 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_roam_succ_handler::hmac_user[%d] null.}",
            hmac_vap->assoc_vap_id);
        return;
    }

    /* 漫游成功之后，处于2G,需要启动业务配置 */
    if (mac_btcoex_check_valid_sta(hmac_vap) == OSAL_TRUE) {
        hmac_config_btcoex_assoc_state_syn(hmac_vap, hmac_user);

        oam_warning_log1(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_roam_succ_handler::wifi is on 2G, ba size to bt control.}", hmac_vap->vap_id);
    } else { /* 处于5G,需要删除配置，默认5G所有信息删除 */
        hmac_config_btcoex_disassoc_state_syn(hmac_vap);

        /* 有bt业务，需要恢复wifi控 */
        if (hal_btcoex_btble_status()->bt_status.bt_status.bt_on != 0) {
            btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);

            btcoex_delba->ba_size = 0;
            hmac_btcoex_delba_trigger(hmac_vap, OSAL_FALSE, btcoex_delba);

            oam_warning_log1(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_roam_succ_handler::wifi is on 5G, ba size to default.}", hmac_vap->vap_id);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_mgmt_priority
 功能描述  : 软件设定管理帧的高优先级保护, 暂时只是考虑关联管理帧，后续管理帧和控制帧
             需要考虑如果已经在拉高状态，尽量不处理，因为可能是vip帧拉高，避免覆盖
*****************************************************************************/
osal_void hmac_btcoex_set_mgmt_priority(const hmac_vap_stru *hmac_vap, osal_u16 timeout_ms)
{
    if (hmac_vap->channel.band == WLAN_BAND_5G) {
        /* 5G不需要拉priority处理 */
        return;
    }
    oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_set_mgmt_priority::set timeout_us:%d}",
        hmac_vap->vap_id, timeout_ms);

    /* 16'hffff：持续拉高priority    16'h0：拉低priority   others：按配置的时间拉高priority */
    hal_set_btcoex_priority_period(timeout_ms);
}

osal_void hmac_btcoex_restart_ps_timer(hal_to_dmac_device_stru *hal_device)
{
    /* 如果定时器创建了，需要按照新的时间来刷新 */
    if (hal_device->btcoex_powersave_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hal_device->btcoex_powersave_timer));
        /* ps机制启动时，需要根据当前状态，刷新超时定时器时间,因为对应业务存在时，不会再来ps中断，需要此处来刷时间 */
        hmac_btcoex_ps_timeout_update_time(hal_device);
        frw_create_timer_entry(&(hal_device->btcoex_powersave_timer), hmac_btcoex_pow_save_callback,
            hal_device->btcoex_sw_preempt.timeout_ms, (osal_void *)hal_device, OSAL_FALSE);
    }
}

#define WLAN_AMPDU_TX_MAX_NUM        16  // 默认最大聚合度

/* 在ble多连接和遥控器场景下，根据不同的带宽、频段进行BA_Size调整 */
OSAL_STATIC osal_void hmac_btcoex_ble_action_req(hmac_vap_stru *hmac_vap,
    const hal_btcoex_btble_status_stru *status_old, hal_btcoex_btble_status_stru *status_new)
{
    /* BA删建处理 */
    if (status_old->action_req.ble_req.req_ba != status_new->action_req.ble_req.req_ba) {
        hmac_btcoex_action_dela_ba_handler(hmac_vap);
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_ble_action_req::req_ba change to %u}",
            status_new->action_req.ble_req.req_ba);
    }

    /* PS申请处理 */
    if (status_old->action_req.ble_req.req_ps != status_new->action_req.ble_req.req_ps) {
        hmac_btcoex_ps_status_handler();
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_btcoex_ble_action_req::req_ba change to %u}",
            status_new->action_req.ble_req.req_ps);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_ps_status_handler
 功能描述  : BT音乐场景，识别bt ps进行ps操作
*****************************************************************************/
osal_void hmac_btcoex_ps_status_handler(osal_void)
{
    oal_bool_enum_uint8 bt_acl_status;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_chip_stru *hal_chip = hal_get_chip_stru(); /* 获取chip指针 */
    hal_device_stru *hal_device = &(hal_chip->device); /* 暂时只是处理主路的STA */
    hal_to_dmac_device_stru *h2d_device = &hal_device->hal_device_base;

    bt_acl_status = hal_btcoex_btble_status()->action_req.ble_req.req_ps;
    if (h2d_device->btcoex_sw_preempt.last_acl_status == bt_acl_status) {
        return;
    }
    if (bt_acl_status) {
        h2d_device->btcoex_sw_preempt.ps_on_cnt++;
        hmac_btcoex_ps_on_proc(hmac_device, h2d_device, hal_chip);
    } else {
        h2d_device->btcoex_sw_preempt.ps_off_cnt++;
        if (h2d_device->btcoex_powersave_timer.is_registerd == OSAL_TRUE) {
            frw_destroy_timer_entry(&(h2d_device->btcoex_powersave_timer));
        }
        /* 在scan状态时候，扫描不进入低功耗，此时不用担心低功耗，如果扫描来时 */
        if (h2d_device->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_SCAN_STATE) {
            hmac_btcoex_ps_off_scan_state_proc(hmac_device, h2d_device);
        } else {
            hmac_btcoex_ps_off_no_scan_state_proc(h2d_device);
        }

        h2d_device->btcoex_sw_preempt.sw_preempt_type = HAL_BTCOEX_SW_POWSAVE_IDLE; /* preempt机制置为IDLE形式 */
    }
    h2d_device->btcoex_sw_preempt.last_acl_status = bt_acl_status; /* 软件记录上一次的acl状态 */
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_status_change_handler
 功能描述  : 状态寄存器处理消息入口
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_status_change_handler(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    hal_btcoex_btble_status_stru *status_old = hal_btcoex_btble_status_last();
    hal_btcoex_btble_status_stru *status_curr = hal_btcoex_btble_status();
    hal_btcoex_btble_status_msg_stru *status_msg = (hal_btcoex_btble_status_msg_stru *)msg->data;

    if ((msg->data_len != sizeof(hal_btcoex_btble_status_msg_stru)) || (msg->data == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_change_handler::msg invalid len=%d!}",
            msg->data_len);
    }

    hal_btcoex_sync_btble_status(status_msg);
    hmac_btcoex_status_dispatch(hmac_vap, status_old, status_curr);
    hmac_btcoex_ble_action_req(hmac_vap, status_old, status_curr);
    ret = memcpy_s(status_old, sizeof(hal_btcoex_btble_status_stru), status_curr,
        sizeof(hal_btcoex_btble_status_stru));
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_status_change_handler::memcpy_s failed code = %d!}", ret);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_blacklist_handle_init
 功能描述  : 初始化黑名单方案
             (1)黑名单用户时，删用户，并聚合个数为8，再次关联也始终聚合8个
             (2)黑名单用户时，addba rsp为37不聚合，蓝牙业务结束时利用漫游重关联，聚合64恢复wifi性能
             增加一个user结构下开关功能

             方案1缺点: 删用户要断流;蓝牙业务结束，wifi性能不恢复;黑名单用户始终存在，开关wifi性能也不恢复
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_btcoex_blacklist_handle_init(hmac_user_stru *hmac_user)
{
    /* 03采用方案2,02采用方案1 */
    hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.blacklist_tpye = BTCOEX_BLACKLIST_TPYE_NOT_AGGR;

    oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_blacklist_handle_init::blacklist_tpye[%d]!}",
        hmac_user->vap_id,
        hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.blacklist_tpye);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_check_exception_in_list_etc
 功能描述  : hmac检查异常兼容性AP的地址
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_btcoex_check_exception_in_list_etc(osal_u8 *addr)
{
    hmac_btcoex_delba_exception_stru *btcoex_exception;
    osal_u8                         index;

    for (index = 0; index < BTCOEX_BSS_NUM_IN_BLACKLIST; index++) {
        btcoex_exception = &(g_hmac_device_btcoex->hmac_btcoex_delba_exception[index]);
        if ((btcoex_exception->used != 0) &&
            (oal_compare_mac_addr(btcoex_exception->user_mac_addr, addr) == 0)) {
            oam_warning_log4(0, OAM_SF_COEX,
                "{hmac_btcoex_check_exception_in_list_etc::Find in blacklist, addr->%02x:%02x:%02x:%02x:XX:XX.}",
                addr[0], addr[1], addr[2], addr[3]);    /* 2,3 MAC地址 */
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_add_exception_to_list
 功能描述  : hmac记录异常兼容性AP的地址
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_btcoex_add_exception_to_list(hmac_vap_stru *hmac_vap, osal_u8 *auc_mac_addr)
{
    hmac_btcoex_delba_exception_stru *btcoex_exception;
    unref_param(hmac_vap);

    if (g_hmac_device_btcoex->exception_bss_index >= BTCOEX_BSS_NUM_IN_BLACKLIST) {
        oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_add_exception_to_list::already reach max num:%d.}",
            hmac_vap->vap_id, BTCOEX_BSS_NUM_IN_BLACKLIST);
        g_hmac_device_btcoex->exception_bss_index = 0;
    }
    btcoex_exception =
        &(g_hmac_device_btcoex->hmac_btcoex_delba_exception[g_hmac_device_btcoex->exception_bss_index]);
    oal_set_mac_addr(btcoex_exception->user_mac_addr, auc_mac_addr);
    btcoex_exception->type = 0;
    btcoex_exception->used = 1;

    g_hmac_device_btcoex->exception_bss_index++;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc
 功能描述  : hmac记录异常兼容性AP的地址
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, mac_ieee80211_frame_stru *frame_hdr, const osal_u8 *action)
{
    hmac_btcoex_addba_req_stru     *btcoex_addba_req;
    hmac_user_btcoex_stru          *hmac_user_btcoex;
    osal_u16                      baw_start;
    osal_u8                       tid;

    hmac_user_btcoex = hmac_btcoex_get_user_info(hmac_user);

    btcoex_addba_req = &(hmac_user_btcoex->hmac_btcoex_addba_req);

    /* 两次收到addba req的start num一样且不是重传帧，认为对端移窗卡死  */
    if (frame_hdr->frame_control.retry == OAL_TRUE &&
        frame_hdr->seq_num == btcoex_addba_req->last_seq_num) {
        oam_warning_log0(0, OAM_SF_COEX,
            "{hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc::retry addba req.}");
        return;
    }

    /*******************************************************************/
    /*       ADDBA Request Frame - Frame Body                          */
    /* --------------------------------------------------------------- */
    /* | Category | Action | Dialog | Parameters | Timeout | SSN     | */
    /* --------------------------------------------------------------- */
    /* | 1        | 1      | 1      | 2          | 2       | 2       | */
    /* --------------------------------------------------------------- */
    /*                                                                 */
    /*******************************************************************/
    baw_start = (action[7] >> 4) | (action[8] << 4);     /* 7,8 索引 左移右移4位 */
    tid = (action[3] & 0x3C) >> 2;   /* 3 索引,右移2位 */
    if (tid != 0) {
        oam_warning_log0(0, OAM_SF_COEX,
            "{hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc::tid != 0.}");
        return;
    }

    if ((baw_start != 0) && (baw_start == btcoex_addba_req->last_baw_start) &&
        (hmac_user_btcoex->rx_no_pkt_count > 2)) {   /* 计数次数大于2 */
        oam_warning_log1(0, OAM_SF_COEX,
            "{hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc::baw_start:%d, delba will forbidden.}",
            baw_start);

        /* 黑名单ba处理标记 */
        btcoex_addba_req->ba_handle_allow = OAL_FALSE;

        if (hmac_btcoex_check_exception_in_list_etc(frame_hdr->address2) == OAL_FALSE) {
            oam_warning_log0(0, OAM_SF_COEX,
                "{hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc::write down to file.}");
            hmac_btcoex_add_exception_to_list(hmac_vap, frame_hdr->address2);
        }

        if (hmac_btcoex_get_blacklist_type(hmac_user) == BTCOEX_BLACKLIST_TPYE_FIX_BASIZE) {
            /* 发送去认证帧到AP */
            hmac_mgmt_send_disassoc_frame_etc(hmac_vap, frame_hdr->address2,
                MAC_UNSPEC_REASON, hmac_user->cap_info.pmf_active);
            oam_warning_log1(0, OAM_SF_DFT,
                "hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc:send DISASSOC,err code[%d]", MAC_UNSPEC_REASON);

            /* 删除对应用户 */
            hmac_user_del_etc(hmac_vap, hmac_user);

            hmac_sta_handle_disassoc_rsp_etc(hmac_vap, MAC_AUTH_NOT_VALID);
        }
    }

    btcoex_addba_req->last_baw_start = baw_start;
    btcoex_addba_req->last_seq_num = frame_hdr->seq_num;
}

/*****************************************************************************
 功能描述  : user add初始化处理
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_proc_user_add(hmac_user_stru *hmac_user)
{
    hmac_user_btcoex_stru *user_btcoex_info = OSAL_NULL;
    osal_u32 ret;

    user_btcoex_info = (hmac_user_btcoex_stru *)osal_kmalloc(sizeof(hmac_user_btcoex_stru), OSAL_GFP_KERNEL);
    if (user_btcoex_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_btcoex_proc_user_add mem alloc fail");
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(user_btcoex_info, sizeof(hmac_user_btcoex_stru), 0, sizeof(hmac_user_btcoex_stru));

    /* 注册特性数据结构 */
    ret = hmac_user_feature_registered(hmac_user, WLAN_FEATURE_INDEX_BTCOEX, user_btcoex_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_M2S, "hmac_btcoex_proc_user_add register feature fail(%u)", ret);
        osal_kfree(user_btcoex_info);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : user del去初始化处理
*****************************************************************************/
OSAL_STATIC osal_bool hmac_btcoex_proc_user_del(osal_void *notify_data)
{
    hmac_user_btcoex_stru *user_btcoex_info = OSAL_NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;

    user_btcoex_info = (hmac_user_btcoex_stru *)hmac_user_get_feature_ptr(hmac_user, WLAN_FEATURE_INDEX_BTCOEX);
    if (user_btcoex_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_btcoex_proc_user_del get feature fail");
        return OSAL_FALSE;
    }

    hmac_user_feature_unregister(hmac_user, WLAN_FEATURE_INDEX_BTCOEX);
    osal_kfree(user_btcoex_info);
    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : vap add初始化处理
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_proc_vap_add(hmac_vap_stru *hmac_vap)
{
    hmac_vap_btcoex_stru *vap_btcoex_info = OSAL_NULL;
    osal_u32 ret;

    vap_btcoex_info = (hmac_vap_btcoex_stru *)osal_kmalloc(sizeof(hmac_vap_btcoex_stru), OSAL_GFP_KERNEL);
    if (vap_btcoex_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_btcoex_proc_vap_add mem alloc fail");
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(vap_btcoex_info, sizeof(hmac_vap_btcoex_stru), 0, sizeof(hmac_vap_btcoex_stru));

    /* 注册特性数据结构 */
    ret = hmac_vap_feature_registered(hmac_vap, WLAN_FEATURE_INDEX_BTCOEX, vap_btcoex_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_M2S, "hmac_btcoex_proc_vap_add register feature fail(%u)", ret);
        osal_kfree(vap_btcoex_info);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : vap del去初始化处理
*****************************************************************************/
OSAL_STATIC osal_bool hmac_btcoex_proc_vap_del(osal_void *notify_data)
{
    hmac_vap_btcoex_stru *vap_btcoex_info = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    vap_btcoex_info = (hmac_vap_btcoex_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_BTCOEX);
    if (vap_btcoex_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_btcoex_proc_vap_del get feature fail");
        return OSAL_FALSE;
    }

    hmac_vap_feature_unregister(hmac_vap, WLAN_FEATURE_INDEX_BTCOEX);
    osal_kfree(vap_btcoex_info);
    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : device add初始化处理
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_proc_device_add(osal_void)
{
    g_hmac_device_btcoex = (hmac_device_btcoex_stru *)osal_kmalloc(sizeof(hmac_device_btcoex_stru), OSAL_GFP_KERNEL);
    if (g_hmac_device_btcoex == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_btcoex_proc_device_add mem alloc fail");
        return OAL_FAIL;
    }
    (osal_void)memset_s(g_hmac_device_btcoex, sizeof(hmac_device_btcoex_stru), 0, sizeof(hmac_device_btcoex_stru));
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : device del去初始化处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_proc_device_del(osal_void)
{
    if (g_hmac_device_btcoex == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_M2S, "hmac_btcoex_proc_device_del excute fail");
        return;
    }
    osal_kfree(g_hmac_device_btcoex);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_abort_end_time
 功能描述  : HAL层中的btcoex中断对应的事件处理函数
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_btcoex_abort_end_time(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hal_btcoex_statistics_stru *tmp_sta = OSAL_NULL;
    hal_btcoex_statistics_stru *btcoex_statistics = OSAL_NULL;

    unref_param(hmac_vap);
    tmp_sta = (hal_btcoex_statistics_stru *)(msg->data);
    btcoex_statistics = hal_btcoex_statistics();
    btcoex_statistics->xGji_LwFd_GLBihwdC_ = tmp_sta->xGji_LwFd_GLBihwdC_;
    btcoex_statistics->xzyvwRO4SwzRlvoO_ = tmp_sta->xzyvwRO4SwzRlvoO_;
    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_btcoex_recover_coex_priority(hmac_vap_stru *hmac_vap)
{
    hmac_btcoex_set_wlan_priority(hmac_vap, OSAL_FALSE, 0);
    hmac_btcoex_set_occupied_period(hmac_vap, 0);
}

OSAL_STATIC osal_void hmac_btcoex_recover_coex_priority_clear(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device)
{
    hmac_btcoex_recover_coex_priority(hmac_vap);
    /* 若当前删除的vap是最后一个，将WIFI状态清除 */
    if (hmac_device->vap_num == 1) {
        hal_btcoex_clear_reg();
    }
}

OSAL_STATIC osal_void hmac_btcoex_check_exception_in_list(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *addr_sa)
{
    hmac_btcoex_blacklist_handle_init(hmac_user);
    if (hmac_btcoex_check_exception_in_list_etc(addr_sa) == OAL_TRUE) {
        if (hmac_btcoex_get_blacklist_type(hmac_user) == BTCOEX_BLACKLIST_TPYE_FIX_BASIZE) {
            oam_warning_log1(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_check_exception_in_list::mac_addr in blacklist.}",
                hmac_vap->vap_id);
            hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.ba_handle_allow = OAL_FALSE;
        } else {
            hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.ba_handle_allow = OAL_TRUE;
        }
    } else {
        /* 初始允许建立聚合，两个方案保持对齐 */
        hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.ba_handle_allow = OAL_TRUE;
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_void hmac_btcoex_rx_process_ect(hmac_vap_stru *hmac_vap,
    osal_u8 frame_subtype, oal_netbuf_stru *netbuf, oal_bool_enum_uint8 ampdu)
{
    osal_u8 data_type;
    hal_btcoex_btble_status_stru *status = hal_btcoex_btble_status();
    if (status->bt_status.bt_status.bt_on == OSAL_FALSE) {
        return;
    }

    data_type = hmac_get_rx_data_type_etc(netbuf);
    hmac_btcoex_release_rx_prot(hmac_vap, data_type);
    /* ps帧发出去后，还收到对端的数据属于正常，要考虑再发一帧的优化暂时不需要 */
    hmac_btcoex_rx_rate_process_check(hmac_vap, frame_subtype, data_type, ampdu);
}

/* 下面会定义一些调用hal层函数的函数接口，用于解耦 */
OSAL_STATIC osal_u16 hmac_btcoex_hal_get_btstatus_bton(osal_void)
{
    return hal_btcoex_get_bt_run();
}

OSAL_STATIC osal_void hmac_btcoex_hal_blinkloss_clean(hmac_vap_stru *hmac_vap)
{
    hmac_vap_btcoex_occupied_stru *btcoex_occupied = OSAL_NULL;
    hmac_vap_btcoex_stru *btcoex_info = hmac_btcoex_get_vap_info(hmac_vap);
    if (btcoex_info == OSAL_NULL) {
        return;
    }
    btcoex_occupied = &(btcoex_info->hmac_vap_btcoex_occupied);
    btcoex_occupied->linkloss_index = 1;
    btcoex_occupied->linkloss_occupied_times = 0;
}

OSAL_STATIC osal_u32 hmac_btcoex_check_user_req_declined(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    if (mac_btcoex_check_valid_sta(hmac_vap) == OAL_TRUE) {
        if (hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.ba_handle_allow == OAL_FALSE) {
            oam_warning_log1(0, OAM_SF_BA,
                "vap_id[%d] {hmac_mgmt_check_set_rx_ba_ok_etc::btcoex blacklist user, not addba!",
                hmac_vap->vap_id);
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}

OSAL_STATIC osal_u32 hmac_btcoex_hal_set_aging_time(const hmac_vap_stru *hmac_vap, osal_u32 *aging_time)
{
    if (hmac_vap->hal_chip == OSAL_NULL) {
        return OAL_FAIL;
    }
    if (hal_btcoex_btble_status()->bt_status.bt_status.bt_ba != 0) {
        /* bt_ba为true时，aging_time为2倍WLAN_P2PGO_USER_AGING_TIME */
        *aging_time = 2 * WLAN_P2PGO_USER_AGING_TIME;
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_btcoex_set_txop_alg(hal_tx_txop_alg_stru *txop_alg)
{
    if (hal_btcoex_btble_status()->bt_status.bt_status.bt_sco != 0) {
        hal_txop_alg_get_tx_dscr(txop_alg)[0].tx_count = 1;
    }
}

OSAL_STATIC osal_u32 hmac_btcoex_check_sw_preempt_type(const hmac_vap_stru *hmac_vap,
    const hal_to_dmac_device_stru *hal_device)
{
    if (hal_device->btcoex_sw_preempt.sw_preempt_type == HAL_BTCOEX_SW_POWSAVE_WORK) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_is_need_enqueue_fake::BT ps is working.}", hmac_vap->vap_id);
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

OSAL_STATIC osal_void hmac_btcoex_hal_set_btcoex_wifi_status(const hmac_vap_stru *hmac_vap, osal_u32 status)
{
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_SCAN, status);
    if (hmac_vap->p2p_mode == WLAN_P2P_DEV_MODE) {
        hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_P2P_SCAN, status);
    }
    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);
}

OSAL_STATIC osal_u32 hmac_btcoex_hal_end_scan_check_ps(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap)
{
    if (hal_device->btcoex_sw_preempt.sw_preempt_type == HAL_BTCOEX_SW_POWSAVE_WORK) {
        hal_device->btcoex_sw_preempt.sw_preempt_type = HAL_BTCOEX_SW_POWSAVE_SCAN_END;
        oam_warning_log1(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_scan_prepare_end:: normal scan end delay by btcoex!}", hmac_vap->vap_id);
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

OSAL_STATIC osal_void hmac_btcoex_update_bss_list_protocol(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body,
    osal_u16 frame_len)
{
    osal_u8 *ie;
    osal_u8 *ie_a;
    ie = hmac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_RALINK, MAC_WLAN_CHIP_OUI_TYPE_RALINK, frame_body, frame_len);
    ie_a = hmac_find_vendor_ie_etc(MAC_WLAN_CHIP_OUI_RALINK, MAC_WLAN_CHIP_OUI_TYPE_RALINK1, frame_body, frame_len);
    bss_dscr->btcoex_ps_blacklist_chip_oui = ((ie != OAL_PTR_NULL) || (ie_a != OAL_PTR_NULL)) ? OAL_TRUE : OAL_FALSE;
}

OSAL_STATIC osal_u32 hmac_btcoex_check_ap_type_blacklist(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *dscr,
    osal_u8 *mac_addr, wlan_nss_enum_uint8 *support_max_nss)
{
    mac_bss_dscr_stru *bss_dscr = dscr;
    /* 关联ap识别: AP OUI + chip OUI + 双流 + 2G 需要dmac侧刷新ps机制时one pkt帧发送类型 */
    if ((mac_is_dlink_ap(mac_addr) == OSAL_TRUE) || (mac_is_haier_ap(mac_addr) == OSAL_TRUE)) {
        bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap, mac_addr);
        if (bss_dscr != OAL_PTR_NULL) {
#ifdef _PRE_WLAN_FEATURE_M2S
            *support_max_nss = bss_dscr->support_max_nss;
#endif
            if ((bss_dscr->btcoex_ps_blacklist_chip_oui == OSAL_TRUE) && (*support_max_nss == WLAN_DOUBLE_NSS) &&
                (hmac_vap->channel.band == WLAN_BAND_2G)) {
                oam_warning_log1(0, OAM_SF_COEX,
                    "vap_id[%d] hmac_compability_ap_tpye_identify_etc: btcoex ps whitelist!",
                    hmac_vap->vap_id);
                return OSAL_FALSE;
            }
        }
    }
    return OSAL_TRUE;
}

OSAL_STATIC osal_void hmac_btcoex_clear_arp_timer(hmac_user_stru *hmac_user)
{
    /* 清理arp探测timer */
    if (hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_arp_req_process.delba_opt_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_arp_req_process.delba_opt_timer));
    }
}

OSAL_STATIC osal_void hmac_btcoex_keepalive_timer_ect(osal_u32 *aging_time)
{
    if (hal_btcoex_btble_status()->bt_status.bt_status.bt_on != 0) {
        /* 蓝牙共存时，aging_time为2倍WLAN_P2PGO_USER_AGING_TIME */
        *aging_time = 2 * WLAN_P2PGO_USER_AGING_TIME;
    }
}

OSAL_STATIC osal_void hmac_btcoex_compatibility_set_ps_type(hmac_vap_stru *hmac_vap)
{
    hmac_vap_btcoex_stru *btcoex_info = hmac_btcoex_get_vap_info(hmac_vap);
    if (btcoex_info == OSAL_NULL) {
        return;
    }
    btcoex_info->ps_type = HAL_BTCOEX_HW_POWSAVE_NULLDATA;
}

OSAL_STATIC osal_void hmac_btcoex_compatibility_set_ap_type(hmac_vap_stru *hmac_vap, mac_ap_type_enum_uint8 ap_type)
{
    hmac_vap_btcoex_stru *btcoex_info = hmac_btcoex_get_vap_info(hmac_vap);
    if (btcoex_info == OSAL_NULL) {
        return;
    }
    /* 如果是兼容性ap，需要刷新one pkt帧类型 */
    if (ap_type == MAC_AP_TYPE_BTCOEX_PS_BLACKLIST) {
        btcoex_info->ps_type = HAL_BTCOEX_HW_POWSAVE_SELFCTS;
    } else {
        btcoex_info->ps_type = HAL_BTCOEX_HW_POWSAVE_NULLDATA;
    }
}

OSAL_STATIC osal_void hmac_btcoex_hal_user_add_handle(hmac_vap_stru *hmac_vap, mac_user_type_enum_uint8 user_type)
{
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && user_type == MAC_USER_TYPE_DEFAULT) {
        // 入网流程开始,发起auth
        hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_STATE_AUTH_MODE, OSAL_TRUE);
    }
}

OSAL_STATIC osal_void hmac_btcoex_vap_resume_tx_by_chl(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device)
{
    if ((hmac_vap->hal_device != OSAL_NULL) &&
        (hmac_vap->hal_device->btcoex_sw_preempt.sw_preempt_subtype == HAL_BTCOEX_SW_POWSAVE_SUB_CONNECT)) {
        /* 状态恢复 */
        hmac_vap->hal_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
        oam_warning_log1(0, 0, "{hmac_vap_resume_tx_by_chl:: vap[%d] is up because powsave when connect!}",
            hmac_vap->vap_id);
    } else if (osal_likely(hal_device != OSAL_NULL)) {
        oam_warning_log3(0, 0, "{hmac_vap_resume_tx_by_chl::vap id=%d, state[%d], preempt state[%d].}",
            hmac_vap->vap_id, hmac_vap->vap_state,
            hal_device->btcoex_sw_preempt.sw_preempt_type);
    }
}

OSAL_STATIC osal_void hmac_btcoex_vap_config_sw_preempt_subtype(hal_to_dmac_device_stru *hal_device)
{
    if (hal_device->btcoex_sw_preempt.sw_preempt_type == HAL_BTCOEX_SW_POWSAVE_WORK) {
        /* 如果设备在关联过程中已经处于ps状态, 子状态置为connect状态 */
        hal_device->btcoex_sw_preempt.sw_preempt_subtype = HAL_BTCOEX_SW_POWSAVE_SUB_CONNECT;
    }
}

OSAL_STATIC osal_void hmac_btcoex_device_exit_destroy_timer(hal_to_dmac_device_stru *hal_device)
{
    frw_destroy_timer_entry(&(hal_device->btcoex_powersave_timer));
    frw_destroy_timer_entry(&(hal_device->device_btcoex_mgr.s2m_resume_timer));
    frw_destroy_timer_entry(&(hal_device->device_btcoex_mgr.bt_coex_s2m_siso_ap_timer));
}

OSAL_STATIC osal_void hmac_btcoex_report_linkloss_info(const hmac_vap_stru *hmac_sta)
{
    oam_warning_log4(0, OAM_SF_ANY,
        "vap_id[%d] bt_linkloss_info: bt_link_loss_cnt =%d, bt_linkloss_threshold = %d, bt state is bt_on/off =%d",
        hmac_sta->vap_id, hmac_sta->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_BT].link_loss,
        hmac_sta->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_BT].linkloss_threshold,
        hal_btcoex_btble_status()->bt_status.bt_status.bt_on);
}

OSAL_STATIC osal_u32 hmac_btcoex_sw_preempt_type_check(hal_to_dmac_device_stru *hal_device)
{
    /* 如果已经处于btcoex ps状态下, 不需要再执行restore动作 */
    if (hal_device->btcoex_sw_preempt.sw_preempt_type == HAL_BTCOEX_SW_POWSAVE_WORK) {
        oam_warning_log0(0, OAM_SF_M2S, "{hmac_m2s_switch::btcoex ps is working not need to switch end.}");
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
osal_void hmac_btcoex_status_info_dump(const hal_to_dmac_device_stru *hal_device, hal_chip_stru *hal_chip)
{
    hal_btcoex_btble_status_stru *status = OSAL_NULL;
    osal_u32 abort_start = 0;
    osal_u32 abort_done = 0;
    osal_u32 abort_end = 0;
    status = hal_btcoex_btble_status();

    wifi_printf("btcoex info:\r\n");
    wifi_printf("  sw_preempt_type=%u, ps_stop=%u, ps_pause=%u, pri_forbit=%u\r\n",
        hal_device->btcoex_sw_preempt.sw_preempt_type, hal_device->btcoex_sw_preempt.ps_stop,
        hal_device->btcoex_sw_preempt.ps_pause, hal_device->btcoex_sw_preempt.coex_pri_forbit);

    wifi_printf("  bt_status=0x%04x, ble_status=0x%04x\r\n",
        status->bt_status.bt_status_reg,
        status->ble_status.ble_status_reg);

    wifi_printf("  ps_on_cnt=%u ps_off_cnt=%u, ps_timeout_cnt=%u\r\n",
        hal_device->btcoex_sw_preempt.ps_on_cnt,
        hal_device->btcoex_sw_preempt.ps_off_cnt,
        hal_device->btcoex_sw_preempt.ps_timeout_cnt);

    hal_btcoex_get_abort_cnt(&hal_chip->hal_chip_base, &abort_start, &abort_done, &abort_end);
    wifi_printf("  abort_start=%u abort_done=%u, abort_end=%u\r\n",
        abort_start, abort_done, abort_end);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_set_dump
 功能描述  : BTCOEX设置device日志打印
*****************************************************************************/
OSAL_STATIC osal_void hmac_btcoex_set_dump(hal_to_dmac_device_stru *hal_device, osal_u8 flag)
{
    osal_s32 ret;
    frw_msg msg = {0};

    unref_param(hal_device);

    frw_msg_init((osal_u8 *)&flag, sizeof(flag), OSAL_NULL, 0, &msg);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SET_BTCOEX_DUMP, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_COEX, "{hmac_btcoex_set_dump send msg fail, flag=%u ret=%d}",
            flag, ret);
    }
    return;
}

OSAL_STATIC osal_s32 hmac_ccpriv_btcoex_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8 ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    osal_char *end = OSAL_NULL;
    osal_u8 dump_flag = 0;
    osal_u64 value;
    hal_chip_stru *hal_chip = hal_get_chip_stru();

    ret = hmac_ccpriv_get_one_arg(&param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{hmac_ccpriv_btcoex_info::wal_get_cmd_one_arg err [%d]!}", ret);
        return ret;
    }
    value = (osal_u64)osal_strtol((const char *)ac_name, &end, 16);
    dump_flag = *(osal_u8 *)&value; /* 16进制参数解析 */

    /* 函数入参本身非空消息机制已检查 */
    if (hmac_vap == OSAL_NULL || hmac_vap->hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ccpriv_btcoex_info:msg is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_btcoex_status_info_dump(hmac_vap->hal_device, hal_chip);
    hmac_btcoex_set_dump(hmac_vap->hal_device, dump_flag);

    return OAL_SUCC;
}
#endif

osal_void hmac_btcoex_hal_wifi_busy_notify(osal_u32 tx_throughput_mbps, osal_u32 rx_throughput_mbps)
{
    osal_u8 wlan_busy;
    osal_u32 throughput_mbps;
    osal_u8 coex_wifi_status = hmac_get_g_btcoex_wlan_busy_status();

    if (g_btcoex_stat_flag == OSAL_FALSE) {
        g_btcoex_throughput.tx_throughput = tx_throughput_mbps;
        g_btcoex_throughput.rx_throughput = rx_throughput_mbps;
        g_btcoex_stat_flag = OSAL_TRUE;
        return;
    }

    tx_throughput_mbps = (tx_throughput_mbps + g_btcoex_throughput.tx_throughput) / 2; /* 2,计算tx吞吐量平均值 */
    rx_throughput_mbps = (rx_throughput_mbps + g_btcoex_throughput.rx_throughput) / 2; /* 2,计算rx吞吐量平均值 */
    throughput_mbps = tx_throughput_mbps + rx_throughput_mbps;
    g_btcoex_stat_flag = OSAL_FALSE;
    /* 8,如果吞吐量大于8mb/s,通知蓝牙开启调整优先级策略 */
    if (throughput_mbps > 8) {
        wlan_busy = OSAL_TRUE;
    } else if (throughput_mbps < 5) { /* 如果吞吐量小于5mb/s,通知蓝牙关闭调整优先级策略 */
        wlan_busy = OSAL_FALSE;
    } else {
        return;
    }

    if (coex_wifi_status == wlan_busy) {
        return;
    }

    oam_warning_log2(0, 0, "{hmac_btcoex_hal_wifi_busy_notify:: wlan_busy[%d] mbps[%d]!}", wlan_busy, throughput_mbps);
    hal_set_btcoex_wifi_status_notify(HAL_BTCOEX_WIFI_BUSY_STATUS, wlan_busy);
    hmac_set_g_btcoex_wlan_busy_status(wlan_busy);
    hal_coex_sw_irq_set(OSAL_TRUE);
    return;
}

OSAL_STATIC osal_void hmac_btcoex_feature_hook_register(osal_void)
{
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_RECOVER_COEX_PRIORITY, hmac_btcoex_recover_coex_priority);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_RECOVER_COEX_PRIORITY_CLEAR, hmac_btcoex_recover_coex_priority_clear);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_RX_PROCESS_ECT, hmac_btcoex_rx_process_ect);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_SET_WLAN_PRIORITY, hmac_btcoex_set_wlan_priority);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_UPDATE_THRESHOLD, hmac_btcoex_linkloss_update_threshold);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_LINKLOSS_OCCUPIED, hmac_btcoex_linkloss_occupied_process);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_ASSOC_STATE_SYN, hmac_config_btcoex_assoc_state_syn);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_ROAM_SUCC_HANDLER, hmac_btcoex_roam_succ_handler);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_CHECK_IN_LIST, hmac_btcoex_check_exception_in_list);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_PROC_USER_ADD, hmac_btcoex_proc_user_add);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_PROC_VAP_ADD, hmac_btcoex_proc_vap_add);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_TX_MGMT_FRAME, hmac_btcoex_tx_mgmt_frame);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_TX_VIP_FRAME, hmac_btcoex_tx_vip_frame);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_USER_SPATIAL_STREAM_CHANGE,
        hmac_btcoex_user_spatial_stream_change_notify);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_CHECK_RX_SAME_BAW_START_FROM_ADDBA_REQ,
        hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_SET_TXOP_ALG, hmac_btcoex_set_txop_alg);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_UPDATE_BSS_LIST_PROTOCOL, hmac_btcoex_update_bss_list_protocol);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_CHECK_AP_TYPE_BLACKLIST, hmac_btcoex_check_ap_type_blacklist);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_CLEAR_ARP_TIMER, hmac_btcoex_clear_arp_timer);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_KEEPALIVE_TIMER_ECT, hmac_btcoex_keepalive_timer_ect);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_COMPATIBILITY_SET_PS_TYPE, hmac_btcoex_compatibility_set_ps_type);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_COMPATIBILITY_SET_AP_TYPE, hmac_btcoex_compatibility_set_ap_type);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_VAP_RESUME_TX_BY_CHL, hmac_btcoex_vap_resume_tx_by_chl);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_VAP_CONFIG_SW_PREEMPT_SUBTYPE,
        hmac_btcoex_vap_config_sw_preempt_subtype);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_DEVICE_EXIT_DESTROY_TIMER, hmac_btcoex_device_exit_destroy_timer);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_REPORT_LINKLOSS_INFO, hmac_btcoex_report_linkloss_info);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_SW_PREEMPT_TYPE_CHECK, hmac_btcoex_sw_preempt_type_check);

    /* 一些调用hal层函数的对外接口 */
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_GET_BTSTATUS_BTON, hmac_btcoex_hal_get_btstatus_bton);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_LINKLOSS_CLEAN, hmac_btcoex_hal_blinkloss_clean);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_CHECK_USER_REQ_DECLINED, hmac_btcoex_check_user_req_declined);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_CHECK_SET_AGING_TIME, hmac_btcoex_hal_set_aging_time);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_CHECK_SW_PREEMPT_TYPE, hmac_btcoex_check_sw_preempt_type);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_SET_BTCOEX_WIFI_STATUS, hmac_btcoex_hal_set_btcoex_wifi_status);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_END_SCAN_CHECK_PS, hmac_btcoex_hal_end_scan_check_ps);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_USER_ADD_HANDLE, hmac_btcoex_hal_user_add_handle);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_HAL_WIFI_BUSY_NOTIFY, hmac_btcoex_hal_wifi_busy_notify);
}

OSAL_STATIC osal_u32 hmac_btcoex_init_ect(osal_void)
{
    osal_u32 ret = OAL_SUCC;
    /* notify注册 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_FEATURE, hmac_btcoex_proc_user_del);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP_FEATURE, hmac_btcoex_proc_vap_del);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_TBTT_AP, hmac_btcoex_beacon_occupied_handler);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_LINKLOSS_INIT, hmac_btcoex_linkloss_init);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_START_VAP_EXIT, hmac_btcoex_vap_up_handle);
    /* 对外接口注册 */
    hmac_btcoex_feature_hook_register();
    /* 消息接口注册 */
    ret |= (osal_u32)frw_msg_hook_register(WLAN_MSG_D2H_BT_STATUS_CHANGE, hmac_btcoex_status_change_handler);
    ret |= (osal_u32)frw_msg_hook_register(WLAN_MSG_D2H_MISS_BEACON, hmac_btcoex_beacon_miss_handler);
    ret |= (osal_u32)frw_msg_hook_register(WLAN_MSG_D2H_BT_ABORT_END, hmac_btcoex_abort_end_time);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"btcoex_info", hmac_ccpriv_btcoex_info);
#endif
    /* 结构体初始化 */
    ret |= hmac_btcoex_proc_device_add();
    if (ret != OAL_SUCC) {
        wifi_printf("hmac_btcoex_init_etc:: MGMT RX IN register_netbuf_hooks error\r\n");
    }
    return ret;
}

OSAL_STATIC osal_void hmac_btcoex_feature_hook_unregister(osal_void)
{
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_RECOVER_COEX_PRIORITY);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_RECOVER_COEX_PRIORITY_CLEAR);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_RX_PROCESS_ECT);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_SET_WLAN_PRIORITY);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_UPDATE_THRESHOLD);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_LINKLOSS_OCCUPIED);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_ASSOC_STATE_SYN);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_ROAM_SUCC_HANDLER);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_CHECK_IN_LIST);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_PROC_USER_ADD);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_PROC_VAP_ADD);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_TX_MGMT_FRAME);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_TX_VIP_FRAME);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_USER_SPATIAL_STREAM_CHANGE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_CHECK_RX_SAME_BAW_START_FROM_ADDBA_REQ);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_SET_TXOP_ALG);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_UPDATE_BSS_LIST_PROTOCOL);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_CHECK_AP_TYPE_BLACKLIST);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_CLEAR_ARP_TIMER);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_KEEPALIVE_TIMER_ECT);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_COMPATIBILITY_SET_PS_TYPE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_COMPATIBILITY_SET_AP_TYPE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_VAP_RESUME_TX_BY_CHL);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_VAP_CONFIG_SW_PREEMPT_SUBTYPE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_DEVICE_EXIT_DESTROY_TIMER);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_REPORT_LINKLOSS_INFO);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_SW_PREEMPT_TYPE_CHECK);
    /* 一些调用hal层函数的对外接口 */
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_GET_BTSTATUS_BTON);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_LINKLOSS_CLEAN);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_CHECK_USER_REQ_DECLINED);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_CHECK_SET_AGING_TIME);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_CHECK_SW_PREEMPT_TYPE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_SET_BTCOEX_WIFI_STATUS);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_END_SCAN_CHECK_PS);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_USER_ADD_HANDLE);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_HAL_WIFI_BUSY_NOTIFY);
}

OSAL_STATIC osal_void hmac_btcoex_deinit_ect(osal_void)
{
    /* notify去注册 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER_FEATURE, hmac_btcoex_proc_user_del);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP_FEATURE, hmac_btcoex_proc_vap_del);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_TBTT_AP, hmac_btcoex_beacon_occupied_handler);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_LINKLOSS_INIT, hmac_btcoex_linkloss_init);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_START_VAP_EXIT, hmac_btcoex_vap_up_handle);
    /* 对外接口去注册 */
    hmac_btcoex_feature_hook_unregister();
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"btcoex_info");
#endif
    /* 结构体销毁 */
    hmac_btcoex_proc_device_del();
    return;
}

osal_u32 hmac_btcoex_init(osal_void)
{
    osal_u32 ret = OAL_SUCC;
    ret |= hmac_btcoex_init_ect();
    ret |= hmac_btcoex_ba_init();
    ret |= hmac_btcoex_m2s_init();
    ret |= hmac_btcoex_notify_init();
    ret |= hmac_btcoex_ps_init();
    if (ret != OAL_SUCC) {
        wifi_printf("hmac_btcoex_init:: MGMT RX IN register_netbuf_hooks error\r\n");
    }
    return ret;
}

osal_void hmac_btcoex_deinit(osal_void)
{
    hmac_btcoex_deinit_ect();
    hmac_btcoex_ba_deinit();
    hmac_btcoex_m2s_deinit();
    hmac_btcoex_notify_deinit();
    hmac_btcoex_ps_deinit();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_BTCOEX_C__ */

