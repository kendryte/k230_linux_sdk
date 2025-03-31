/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: 无.
 * Create: 2020-7-9
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_csa_sta.h"
#include "hal_ext_if.h"
#include "mac_ie.h"
#include "hmac_main.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_mgmt_bss_comm.h"
#include "oam_ext_if.h"
#include "hmac_beacon.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "hmac_psm_sta.h"
#include "hmac_power.h"
#include "oal_netbuf_data.h"
#include "hmac_hook.h"
#include "hmac_scan.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"
#include "common_log_dbg_rom.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CSA_STA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

OSAL_STATIC osal_u32 hmac_sta_csa_fsm_init_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_start_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_switch_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_wait_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_invalid_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_void hmac_sta_csa_fsm_init_entry(osal_void *ctx);
OSAL_STATIC osal_void hmac_sta_csa_fsm_invalid_entry(osal_void *ctx);
OSAL_STATIC osal_void hmac_csa_sta_switch_channel(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_u8 hmac_sta_csa_fsm_get_current_state(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_bool hmac_sta_csa_fsm_is_waiting(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_post_event(hmac_vap_stru *hmac_vap, osal_u16 type, osal_u16 datalen,
    osal_u8 *data);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_vap_csa_stru *g_csa_sta_vapinfo[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

const oal_fsm_state_info g_sta_csa_fsm_info[] = {
    {
        WLAN_STA_CSA_FSM_INIT,
        "INIT",
        hmac_sta_csa_fsm_init_entry,
        OSAL_NULL,
        hmac_sta_csa_fsm_init_event,
    },
    {
        WLAN_STA_CSA_FSM_START,
        "START",
        OSAL_NULL,
        OSAL_NULL,
        hmac_sta_csa_fsm_start_event,
    },
    {
        WLAN_STA_CSA_FSM_SWITCH,
        "SWITCH",
        OSAL_NULL,
        OSAL_NULL,
        hmac_sta_csa_fsm_switch_event,
    },
    {
        WLAN_STA_CSA_FSM_WAIT,
        "WAIT",
        OSAL_NULL,
        OSAL_NULL,
        hmac_sta_csa_fsm_wait_event,
    },
    {
        WLAN_STA_CSA_FSM_INVALID,
        "INVALID",
        hmac_sta_csa_fsm_invalid_entry,
        OSAL_NULL,
        hmac_sta_csa_fsm_invalid_event,
    }
};

static inline hmac_vap_csa_stru *hmac_csa_get_vap_info(const hmac_vap_stru *hmac_vap)
{
    return g_csa_sta_vapinfo[hmac_vap->vap_id];
}

OSAL_STATIC osal_u32 hmac_trigger_csa_init_scan_params(const hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params)
{
    osal_u8 chan_num = 0;
    osal_u8 channel_idx = 0;
    mac_channel_stru *csa_old_channel = &(hmac_csa_get_vap_info(hmac_vap)->old_channel);

    (osal_void)memset_s(scan_params, sizeof(mac_scan_req_stru), 0, sizeof(mac_scan_req_stru));

    /* 1.设置发起扫描的vap id */
    scan_params->vap_id = hmac_vap->vap_id;

    /* 2.设置初始扫描请求的参数 */
    scan_params->bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->scan_type = WLAN_SCAN_TYPE_ACTIVE;
    scan_params->probe_delay = 0;
    scan_params->scan_func = MAC_SCAN_FUNC_BSS; /* 默认扫描bss */
    scan_params->max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    scan_params->max_scan_count_per_channel = 2; /* 最大信道扫描次数为2 */

    /* 根据扫描类型，确定每信道扫描时间 */
    if (scan_params->scan_type == WLAN_SCAN_TYPE_ACTIVE) {
        scan_params->scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    }

    /* CSA扫描通配ssid */
    scan_params->mac_ssid_set[0].ssid[0] = '\0';
    scan_params->ssid_num = 1;
    /* CSA扫描设置Source MAC ADDRESS */
    (osal_void)memcpy_s((osal_void *)(scan_params->sour_mac_addr), WLAN_MAC_ADDR_LEN,
        (osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN);

    /* CSA扫描扫描只指定1个bssid，为广播地址 */
    (osal_void)memcpy_s((osal_void *)(scan_params->bssid[0]), WLAN_MAC_ADDR_LEN,
        (osal_void *)(BROADCAST_MACADDR), WLAN_MAC_ADDR_LEN);
    scan_params->bssid_num = 1;

    /* 设置扫描模式为CSA扫描 */
    scan_params->scan_mode = WLAN_SCAN_MODE_BACKGROUND_CSA;

    /* 准备扫描的信道 */
    /* 判断信道是不是在管制域内 */
    if (hmac_is_channel_num_valid_etc(csa_old_channel->band, csa_old_channel->chan_number) == OAL_SUCC) {
        hmac_get_channel_idx_from_num_etc(csa_old_channel->band, csa_old_channel->chan_number, &channel_idx);
        scan_params->channel_list[chan_num].chan_number =
            csa_old_channel->chan_number;
        scan_params->channel_list[chan_num].band = csa_old_channel->band;
        scan_params->channel_list[chan_num].chan_idx = channel_idx;
        chan_num++;
    }

    if (hmac_is_channel_num_valid_etc(hmac_vap->channel.band, hmac_vap->channel.chan_number) == OAL_SUCC) {
        hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, hmac_vap->channel.chan_number,
            &channel_idx);
        scan_params->channel_list[chan_num].chan_number = hmac_vap->channel.chan_number;
        scan_params->channel_list[chan_num].band = hmac_vap->channel.band;
        scan_params->channel_list[chan_num].chan_idx = channel_idx;
        chan_num++;
    }

    /* 更新本次扫描的信道总数 */
    scan_params->channel_nums = chan_num;

    /* 如果当前扫描信道的总数为0，返回错误，不执行扫描请求 */
    if (scan_params->channel_nums == 0) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_trigger_csa_init_scan_params::scan total channel num is 0.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_scan_prepare_csa_scan_params
 功能描述  : 准备csa切换后的扫描参数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_trigger_csa_scan(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_scan_req_stru scan_params;
    mac_channel_stru *csa_old_channel = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_trigger_csa_scan::param null.}");
        return OAL_FAIL;
    }

    csa_old_channel = &(hmac_csa_get_vap_info(hmac_vap)->old_channel);
    if (csa_old_channel->chan_number == hmac_vap->channel.chan_number) {
        hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OSAL_NULL);
        return OAL_SUCC;
    }

    oam_warning_log0(0, OAM_SF_SCAN, "{hmac_trigger_csa_scan::update csa scan params.}");

    if (hmac_trigger_csa_init_scan_params(hmac_vap, &scan_params) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (hmac_scan_handle_scan_req_entry(hmac_device, hmac_vap, &scan_params) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF,
            "{hmac_trigger_csa_scan::hmac_scan_handle_scan_req_entry err.}");
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :状态机回到init
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_csa_fsm_init_entry(osal_void *ctx)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ctx;
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);
    mac_sta_csa_fsm_info_stru *csa_fsm_info = OSAL_NULL;

    if (hmac_sta_csa_fsm_get_current_state(hmac_vap) == WLAN_STA_CSA_FSM_INIT) {
        return;
    }

    csa_fsm_info = &(csa_vap_info->sta_csa_fsm_info);

    if (csa_fsm_info->csa_handle_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(csa_fsm_info->csa_handle_timer));
    }

    if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) {
        hmac_chan_enable_machw_tx(hmac_vap); // 恢复发送，防止问题ap发的beacon让停止发送
    }

    csa_fsm_info->csa_scan_after_tbtt = 0;
    csa_fsm_info->sta_csa_last_cnt = 0;
    csa_vap_info->old_channel.band = WLAN_BAND_BUTT;
    csa_vap_info->old_channel.chan_number = 0;
    csa_vap_info->old_channel.en_bandwidth = WLAN_BAND_WIDTH_BUTT;
    csa_vap_info->new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    csa_vap_info->mode = 0;

    return;
}

/*****************************************************************************
 功能描述  : 检查是由有vap处在入网阶段
*****************************************************************************/
OSAL_STATIC osal_bool hmac_sta_csa_have_vap_is_in_assoc(const hmac_vap_stru *hmac_vap)
{
    osal_u8 vap_idx;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *index_mac_vap = OSAL_NULL;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        index_mac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (index_mac_vap == OSAL_NULL) {
            return OSAL_TRUE;
        }
        if (((index_mac_vap->vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
            (index_mac_vap->vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC))
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (index_mac_vap->vap_state == MAC_VAP_STATE_ROAMING)
#endif
        ) {
            oam_warning_log3(0, OAM_SF_CSA,
                "vap_id[%d] {hmac_sta_csa_have_vap_is_in_assoc::vap_%d_state=[%d], Return OSAL_TRUE}", hmac_vap->vap_id,
                index_mac_vap->vap_id, index_mac_vap->vap_state);
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 状态机转换函数
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_csa_fsm_trans_to_state(hmac_vap_stru *hmac_vap, osal_u8 state)
{
    oal_bool_enum now_wait_status;
    oal_bool_enum now_next_status;

    if (state == hmac_sta_csa_fsm_get_current_state(hmac_vap)) {
        oam_warning_log2(0, OAM_SF_CSA,
            "vap_id[%d] {hmac_sta_csa_fsm_trans_to_state::now status = %d not need to change}", hmac_vap->vap_id,
            state);
        return;
    }

    oam_warning_log3(0, OAM_SF_CSA, "vap_id[%d] {hmac_sta_csa_fsm_trans_to_state::csa status change from [%d] to [%d]}",
        hmac_vap->vap_id, hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.oal_fsm.cur_state, state);

    now_wait_status = hmac_sta_csa_fsm_is_waiting(hmac_vap);
    oal_fsm_trans_to_state(&(hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.oal_fsm), state);

    now_next_status = hmac_sta_csa_fsm_is_waiting(hmac_vap);
    if (now_wait_status != now_next_status) {
        hmac_set_psm_param_to_device(hmac_vap, OAL_FALSE, 0, OAL_FALSE, OSAL_FALSE);
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        if (hmac_sta_csa_fsm_get_current_state(hmac_vap) != WLAN_STA_CSA_FSM_INIT) {
            hmac_vap->sta_pm_handler.max_skip_bcn_cnt = 0;
            hmac_psm_mask_mac_parse_tim(hmac_vap, OSAL_TRUE);
        } else {
            hmac_vap->sta_pm_handler.max_skip_bcn_cnt = HMAC_PSM_MAX_SKIP_BCN_CNT;
            hmac_psm_mask_mac_parse_tim(hmac_vap, OSAL_FALSE);
        }
#endif
    }
    return;
}

/*****************************************************************************
功能描述  : 超时处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_timeout_fn(osal_void *arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)(arg);

    oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] {hmac_sta_csa_timeout_fn:: To wait tbtt event}", hmac_vap->vap_id);

    hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_SWITCH);
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 创建csa 定时器
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_csa_start_timer(hmac_vap_stru *hmac_vap,
    const oal_ieee80211_channel_sw_ie *const csa_info)
{
    osal_u32 timeout;
    mac_sta_csa_fsm_info_stru *csa_fsm_info = &(hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info);
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);

    if (csa_info->mode == 1) {
        hmac_chan_disable_machw_tx(hmac_vap);
    }

    csa_fsm_info->sta_csa_last_cnt = csa_info->count;
    csa_vap_info->new_channel = csa_info->new_ch_num;
    csa_vap_info->mode = csa_info->mode;
    hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_START);
    timeout = mac_mib_get_beacon_period(hmac_vap) * csa_info->count;
    oam_warning_log3(0, OAM_SF_CSA, "vap_id[%d] {hmac_sta_csa_start_timer::create timer,timeout = [%d](%d)}",
        hmac_vap->vap_id, timeout, csa_info->count);

    if (csa_fsm_info->csa_handle_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(csa_fsm_info->csa_handle_timer));
    }

    if (timeout != 0) {
        frw_create_timer_entry(&(csa_fsm_info->csa_handle_timer), hmac_sta_csa_timeout_fn, timeout, hmac_vap,
            OAL_FALSE);
    } else {
        hmac_sta_csa_timeout_fn((osal_void *)hmac_vap);
    }

    return;
}

/*****************************************************************************
功能描述  :状态机处于START状态，又收到csa ie处理函数
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_csa_fsm_start_get_csa_ie(hmac_vap_stru *hmac_vap,
    const oal_ieee80211_channel_sw_ie * const csa_info)
{
    osal_u32 timeout;
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);
    mac_sta_csa_fsm_info_stru *csa_fsm_info = &csa_vap_info->sta_csa_fsm_info;

    if (csa_info->count >= csa_fsm_info->sta_csa_last_cnt && csa_info->new_ch_num == csa_vap_info->new_channel) {
        /* 检测到异常cnt值 进入invalid状态 防攻击 */
        frw_timer_stop_timer(&(csa_fsm_info->csa_handle_timer));
        hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INVALID);
        return;
    }

    csa_fsm_info->sta_csa_last_cnt = csa_info->count;
    csa_vap_info->new_channel = csa_info->new_ch_num;
    if (csa_info->mode != csa_vap_info->mode) {
        if (csa_info->mode == 1) {
            hmac_chan_disable_machw_tx(hmac_vap);
        } else {
            hmac_chan_enable_machw_tx(hmac_vap);
        }
    }
    csa_vap_info->mode = csa_info->mode;
    timeout = mac_mib_get_beacon_period(hmac_vap) * csa_info->count;
    if (timeout != 0) {
        frw_timer_restart_timer(&(csa_fsm_info->csa_handle_timer), timeout, OSAL_FALSE);
    } else {
        if (csa_fsm_info->csa_handle_timer.is_registerd == OSAL_TRUE) {
            frw_destroy_timer_entry(&(csa_fsm_info->csa_handle_timer));
        }
        hmac_sta_csa_timeout_fn((osal_void *)hmac_vap);
    }

    return;
}

/*****************************************************************************
 功能描述  : 状态机为WLAN_STA_CSA_FSM_INIT 处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_init_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data)
{
    oal_ieee80211_channel_sw_ie *csa_info = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ctx;

    switch (event) {
        case WLAN_STA_CSA_EVENT_GET_IE:
            if ((event_data != OSAL_NULL) && (event_data_len == sizeof(oal_ieee80211_channel_sw_ie))) {
                csa_info = (oal_ieee80211_channel_sw_ie *)event_data;
                hmac_sta_csa_start_timer(hmac_vap, csa_info);
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 状态机为WLAN_STA_CSA_FSM_DTART 处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_start_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data)
{
    oal_ieee80211_channel_sw_ie *csa_info = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ctx;

    switch (event) {
        case WLAN_STA_CSA_EVENT_TO_INIT:
            hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
            break;
        case WLAN_STA_CSA_EVENT_GET_IE:
            if ((event_data != OSAL_NULL) && (event_data_len == sizeof(oal_ieee80211_channel_sw_ie))) {
                csa_info = (oal_ieee80211_channel_sw_ie *)event_data;
                hmac_sta_csa_fsm_start_get_csa_ie(hmac_vap, csa_info);
            }
            break;
        case WLAN_STA_CSA_EVENT_TBTT:
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 状态机为WLAN_STA_CSA_FSM_SWITCH 处理函数;VAP处于UP状态时进行信道切换
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_switch_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ctx;
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);

    if (event_data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_csa_fsm_switch_event::event_data is null!!!}");
        unref_param(event_data_len);
        unref_param(csa_vap_info);
    }
    switch (event) {
        case WLAN_STA_CSA_EVENT_TO_INIT:
            hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
            break;
        case WLAN_STA_CSA_EVENT_TBTT:
            if (hmac_vap->vap_state == MAC_VAP_STATE_UP) {
                if (hmac_sta_csa_have_vap_is_in_assoc(hmac_vap) == OSAL_FALSE) {
                    common_log_dbg3(0, OAM_SF_CSA,
                        "vap_id[%d] {hmac_sta_csa_fsm_switch_event::switch channel from [%d] to [%d]}",
                        hmac_vap->vap_id, hmac_vap->channel.chan_number, csa_vap_info->new_channel);
                    hmac_csa_sta_switch_channel(hmac_vap);
                    hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_WAIT);
                } else {
                    oam_warning_log1(0, OAM_SF_CSA,
                        "vap_id[%d] {hmac_sta_csa_fsm_switch_event::have vap in assoc, csa return INIT}",
                        hmac_vap->vap_id);
                    hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
                }
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : CSA 切换失败后定时器超时停止 处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_stop_timeout_fn(osal_void *arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)(arg);

    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        return OAL_SUCC;
    }
    oam_warning_log0(0, OAM_SF_CSA, "{hmac_sta_csa_stop_timeout_fn::timer timeout!}");
    hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.is_switch_fail = OAL_FALSE;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : STA csa状态机对事件 WLAN_STA_CSA_EVENT_SCAN_END 的处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_scan_end(hmac_vap_stru *hmac_vap)
{
    mac_sta_csa_fsm_info_stru *csa_fsm_info = &(hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info);
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);
    frw_timeout_stru *csa_stop_timer = &(hmac_csa_get_vap_info(hmac_vap)->csa_stop_timer);

    if (csa_fsm_info->associate_channel == 0) { /* 未收到csa扫描 response */
        /* CSA 未扫描到，增加4 次 重试机会 */
        (hmac_csa_get_vap_info(hmac_vap)->csa_switch_fail_trigger_scan_cnt)++;
        if (hmac_csa_get_vap_info(hmac_vap)->csa_switch_fail_trigger_scan_cnt < MAC_CSA_SWITCH_FAIL_TRIGGER_SCAN_MAX) {
            hmac_trigger_csa_scan(hmac_vap);
            return OAL_SUCC;
        }
        hmac_csa_get_vap_info(hmac_vap)->csa_switch_fail_trigger_scan_cnt = 0;
        hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INVALID);
    } else if (hmac_vap->channel.chan_number == csa_fsm_info->associate_channel) { /* 收到csa扫描response */
        hmac_csa_get_vap_info(hmac_vap)->csa_switch_fail_trigger_scan_cnt = 0;
        /* 信道相等 */
        hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
        csa_fsm_info->associate_channel = 0;
    } else {
        hmac_csa_get_vap_info(hmac_vap)->csa_switch_fail_trigger_scan_cnt = 0;
        csa_fsm_info->is_switch_fail = OAL_TRUE;
        /* 启动定时器,2分钟禁止CSA switch */
        if (csa_stop_timer->is_registerd == OAL_TRUE) {
            frw_destroy_timer_entry(csa_stop_timer);
        }
        frw_create_timer_entry(csa_stop_timer, hmac_sta_csa_stop_timeout_fn, MAC_CSA_DISABLE_TIMEOUT, hmac_vap,
            OAL_FALSE);
        csa_vap_info->new_channel = csa_fsm_info->associate_channel;
        csa_vap_info->new_bandwidth = (csa_fsm_info->associate_channel == csa_vap_info->old_channel.chan_number) ?
            csa_vap_info->old_channel.en_bandwidth : hmac_vap->channel.en_bandwidth;
        common_log_dbg3(0, OAM_SF_CSA, "vap_id[%d] {hmac_sta_csa_fsm_scan_end:csa fail, from:%d to:%d}",
            hmac_vap->vap_id, hmac_vap->channel.chan_number, csa_vap_info->new_channel);
        hmac_csa_sta_switch_channel(hmac_vap);
        hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INVALID);
        csa_fsm_info->associate_channel = 0;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 状态机为WLAN_STA_CSA_FSM_WAIT 处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_wait_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data)
{
    osal_u32 ret;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ctx;
    mac_sta_csa_fsm_info_stru *csa_fsm_info = OSAL_NULL;

    unref_param(event_data_len);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_FAIL;
    }

    csa_fsm_info = &(hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info);

    if (event == WLAN_STA_CSA_EVENT_TBTT) {
        csa_fsm_info->csa_scan_after_tbtt++;
        if (csa_fsm_info->csa_scan_after_tbtt >= 2) { /* 2表示界限 */
            /* csa 扫描 */
            if (hmac_sta_csa_have_vap_is_in_assoc(hmac_vap) == OSAL_FALSE) {
                oam_warning_log1(0, 0, "vap_id[%d] {hmac_sta_csa_fsm_wait_event::trigger_csa_scan}", hmac_vap->vap_id);
                csa_fsm_info->associate_channel = 0;
                hmac_trigger_csa_scan(hmac_vap);
            } else {
                oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] {hmac_sta_csa_fsm_wait_event:have vap assoc,return INIT",
                    hmac_vap->vap_id);
                hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
            }
            csa_fsm_info->csa_scan_after_tbtt = 0;
        }
    } else if ((event == WLAN_STA_CSA_EVENT_RCV_BEACON) || (event == WLAN_STA_CSA_EVENT_RCV_PROBE_RSP)) {
        if (event_data != OSAL_NULL) {
            osal_u8 ap_channel = *(osal_u8 *)event_data;
            csa_fsm_info->associate_channel = ap_channel;
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_sta_csa_fsm_wait_event:assoc_ch=%d}", hmac_vap->vap_id,
                ap_channel);
        }
    } else if (event == WLAN_STA_CSA_EVENT_SCAN_END) {
        ret = hmac_sta_csa_fsm_scan_end(hmac_vap);
        if (ret != OAL_CONTINUE) {
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_sta_csa_fsm_wait_event::hmac_sta_csa_fsm_scan_end() ret[%d]}",
                hmac_vap->vap_id, ret);
            return ret;
        }
    } else if (event == WLAN_STA_CSA_EVENT_TO_INIT) {
        hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 状态机回到invalid
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_csa_fsm_invalid_entry(osal_void *ctx)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ctx;
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);
    mac_sta_csa_fsm_info_stru *csa_fsm_info = OSAL_NULL;

    if (hmac_sta_csa_fsm_get_current_state(hmac_vap) == WLAN_STA_CSA_FSM_INVALID) {
        return;
    }

    csa_fsm_info = &(csa_vap_info->sta_csa_fsm_info);

    if (csa_fsm_info->csa_handle_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(csa_fsm_info->csa_handle_timer));
    }

    if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) {
        hmac_chan_enable_machw_tx(hmac_vap);  // 恢复发送，防止问题ap发的beacon让停止发送
    }

    csa_vap_info->new_bandwidth = WLAN_BAND_WIDTH_BUTT;

    return;
}

/*****************************************************************************
 功能描述  : 状态机为WLAN_STA_CSA_FSM_INVALID处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_invalid_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    oal_ieee80211_channel_sw_ie *csa_info = OSAL_NULL;

    hmac_vap = (hmac_vap_stru *)ctx;
    switch (event) {
        case WLAN_STA_CSA_EVENT_TO_INIT:
            hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
            break;
        case WLAN_STA_CSA_EVENT_GET_IE:
            if ((event_data_len == sizeof(oal_ieee80211_channel_sw_ie)) && (event_data != OSAL_NULL)) {
                csa_info = (oal_ieee80211_channel_sw_ie *)event_data;
                if (hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.sta_csa_last_cnt != csa_info->count) {
                    hmac_sta_csa_start_timer(hmac_vap, csa_info);
                }
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : csa fsm 状态机处理函数注册
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_csa_fsm_attach(hmac_vap_stru *hmac_vap)
{
    mac_sta_csa_fsm_info_stru *csa_fsm = OSAL_NULL;
    osal_u8 fsm_name[8] = {0}; /* 8表示fsm_name数组的大小 */
    osal_u32 ret;
    oal_fsm_create_stru fsm_create_stru;

    csa_fsm = &(hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info);
    if (csa_fsm->csa_handle_timer.is_registerd == OSAL_TRUE) {
        oam_error_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_sta_csa_fsm_attach::csa_handle_timer registerd.",
            hmac_vap->vap_id);
        frw_destroy_timer_entry(&(csa_fsm->csa_handle_timer));
    }

    (osal_void) memset_s(csa_fsm, sizeof(mac_sta_csa_fsm_info_stru), 0, sizeof(mac_sta_csa_fsm_info_stru));
    /* 准备一个唯一的fsmname,只能到6,最后给'\0' */
    fsm_name[0] = '0';                    /* core_id固定为0 */
    fsm_name[1] = '0';                    /* chip_id固定为0 */
    fsm_name[2] = '0';                    /* 2表示fsm_name的下标 device_id固定为0 */
    fsm_name[3] = '0' + hmac_vap->vap_id; /* 3表示fsm_name的下标 */
    fsm_name[4] = 'C';                    /* 4表示fsm_name的下标 */
    fsm_name[5] = 'S';                    /* 5表示fsm_name的下标 */
    fsm_name[6] = 'A';                    /* 6表示fsm_name的下标 */

    fsm_create_stru.oshandle = hmac_vap;
    fsm_create_stru.name = fsm_name;
    fsm_create_stru.context = hmac_vap;

    ret = oal_fsm_create(&fsm_create_stru, &(csa_fsm->oal_fsm), WLAN_STA_CSA_FSM_INIT, g_sta_csa_fsm_info,
        sizeof(g_sta_csa_fsm_info) / sizeof(oal_fsm_state_info));
    if (ret != OAL_SUCC) {
        csa_fsm->is_fsm_attached = OSAL_FALSE;
        oam_error_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_sta_csa_fsm_attach::oal_fsm_create fail.", hmac_vap->vap_id);
        return;
    }

    csa_fsm->is_fsm_attached = OSAL_TRUE;
    oam_warning_log4(0, OAM_SF_CSA,
        "vap_id[%d] hmac_sta_csa_fsm_attach::vap_id = %d vap_mode=%d p2p_mode=%d attached succ.", hmac_vap->vap_id,
        hmac_vap->vap_id, hmac_vap->vap_mode, hmac_vap->p2p_mode);

    return;
}

/*****************************************************************************
 功能描述  : csa fsm 状态机处理函数销毁
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_csa_fsm_detach(hmac_vap_stru *hmac_vap)
{
    if (hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.is_fsm_attached == OSAL_FALSE) {
        oam_warning_log2(0, OAM_SF_CSA, "vap_id[%d] hmac_sta_csa_fsm_detach::vap id[%d]pm fsm not attatched",
            hmac_vap->vap_id, hmac_vap->vap_id);
        return;
    }

    hmac_sta_csa_fsm_trans_to_state(hmac_vap, WLAN_STA_CSA_FSM_INIT);
    hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.is_fsm_attached = OSAL_FALSE;

    oam_warning_log3(0, OAM_SF_CSA, "hmac_sta_csa_fsm_detach::vap_id = %d vap_mode=%d p2p_mode=%d detached succ.",
        hmac_vap->vap_id, hmac_vap->vap_mode, hmac_vap->p2p_mode);
    return;
}

/*****************************************************************************
功能描述  : 获取sta csa fsm当前状态
*****************************************************************************/
OSAL_STATIC osal_u8 hmac_sta_csa_fsm_get_current_state(hmac_vap_stru *hmac_vap)
{
    return hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.oal_fsm.cur_state;
}

/*****************************************************************************
 功能描述  :判断csa是否处在切信道过程中
*****************************************************************************/
OSAL_STATIC osal_bool hmac_sta_csa_fsm_is_waiting(hmac_vap_stru *hmac_vap)
{
    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if ((hmac_sta_csa_fsm_get_current_state(hmac_vap) >= WLAN_STA_CSA_FSM_START) &&
        (hmac_sta_csa_fsm_get_current_state(hmac_vap) <= WLAN_STA_CSA_FSM_WAIT)) {
        return OSAL_TRUE;
    } else {
        return OSAL_FALSE;
    }
}

/*****************************************************************************
 功能描述  : 触发状态机事件处理函数
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_sta_csa_fsm_post_event(hmac_vap_stru *hmac_vap, osal_u16 type, osal_u16 datalen,
    osal_u8 *data)
{
    mac_sta_csa_fsm_info_stru *csa_fsm = OSAL_NULL;
    osal_u32 ret;

    if (type >= WLAN_STA_CSA_EVENT_BUTT) {
        oam_error_log2(0, OAM_SF_CSA, "vap_id[%d] hmac_sta_csa_fsm_post_event:: event type[%d] error, NULL!",
            hmac_vap->vap_id, type);
        return OAL_FAIL;
    }

    csa_fsm = &(hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info);
    if (csa_fsm->is_fsm_attached == OSAL_FALSE) {
        oam_warning_log2(0, OAM_SF_CSA, "vap_id[%d] hmac_sta_csa_fsm_post_event::csa_fsm_attached = %d!",
            hmac_vap->vap_id, csa_fsm->is_fsm_attached);
        return OAL_FAIL;
    }

    ret = oal_fsm_event_dispatch(&(csa_fsm->oal_fsm), type, datalen, data);

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_csa_adjust_bandwidth_sta
 功能描述  : 根据AP和(STA)自身的能力，计算准备要切换到的带宽模式
*****************************************************************************/
OAL_STATIC osal_void hmac_csa_adjust_bandwidth_sta(hmac_vap_stru *hmac_vap,
    wlan_channel_bandwidth_enum_uint8 *bandwidth)
{
    wlan_channel_bandwidth_enum_uint8   curr_bandwidth;
    wlan_channel_bandwidth_enum_uint8   announced_bandwidth;
    curr_bandwidth      = hmac_vap->channel.en_bandwidth;
    announced_bandwidth = hmac_csa_get_vap_info(hmac_vap)->new_bandwidth;
    *bandwidth = curr_bandwidth;

    /* 如果当前带宽模式与新带宽模式相同，则直接返回 */
    if (announced_bandwidth == curr_bandwidth) {
        return;
    }

    if (announced_bandwidth == WLAN_BAND_WIDTH_20M) {
        *bandwidth = WLAN_BAND_WIDTH_20M;
    } else { /* 新带宽模式不是20MHz，则STA侧带宽模式需要根据自身能力进行匹配 */
        /* 使能40MHz */
        /* (1) 用户开启"40MHz运行"特性(即STA侧 dot11FortyMHzOperationImplemented为true) */
        /* (2) AP在40MHz运行 */
        if (mac_mib_get_dot11_vap_max_bandwidth(hmac_vap) > WLAN_BW_CAP_20M) {
            switch (announced_bandwidth) {
                case WLAN_BAND_WIDTH_40PLUS:
                case WLAN_BAND_WIDTH_80PLUSPLUS:
                case WLAN_BAND_WIDTH_80PLUSMINUS:
                    *bandwidth = WLAN_BAND_WIDTH_40PLUS;
                    break;

                case WLAN_BAND_WIDTH_40MINUS:
                case WLAN_BAND_WIDTH_80MINUSPLUS:
                case WLAN_BAND_WIDTH_80MINUSMINUS:
                    *bandwidth = WLAN_BAND_WIDTH_40MINUS;
                    break;

                default:
                    *bandwidth = WLAN_BAND_WIDTH_20M;
                    break;
            }
        }

        /* 使能80MHz */
        /* (1) 用户支持80MHz带宽(即STA侧 dot11VHTChannelWidthOptionImplemented为0) */
        /* (2) MAC DEVICE支持80MHz */
        if (mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_TRUE) {
            *bandwidth = hmac_vap_get_bandwith(hmac_vap, announced_bandwidth);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_csa_sta_switch_channel
 功能描述  : STA切换信道
*****************************************************************************/
OSAL_STATIC osal_void hmac_csa_sta_switch_channel(hmac_vap_stru *hmac_vap)
{
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);
    wlan_channel_bandwidth_enum_uint8 new_bandwidth = WLAN_BAND_WIDTH_20M;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_bool is_support_bw;

    if (csa_vap_info->new_bandwidth != WLAN_BAND_WIDTH_BUTT) {
        hmac_csa_adjust_bandwidth_sta(hmac_vap, &new_bandwidth);
    } else {
        /* csa中没有带宽参数变化，使用之前带宽参数 */
        new_bandwidth = hmac_vap->channel.en_bandwidth;
    }

    is_support_bw = hmac_regdomain_channel_is_support_bw(new_bandwidth, csa_vap_info->new_channel);
    if (is_support_bw == OSAL_FALSE) {
        oam_warning_log3(0, OAM_SF_CSA,
            "vap_id[%d] {hmac_csa_sta_switch_channel::channel[%d] is not support bw[%d],set 20MHz}", hmac_vap->vap_id,
            csa_vap_info->new_channel, new_bandwidth);
        new_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    (osal_void)memcpy_s(&(csa_vap_info->old_channel), sizeof(csa_vap_info->old_channel),
        &hmac_vap->channel, sizeof(csa_vap_info->old_channel));

    oam_warning_log4(0, OAM_SF_CSA,
        "{hmac_csa_sta_switch_channel:: csa Switching from chan[%d] BW[%d] mode To chan[%d] BW[%d].}",
        csa_vap_info->old_channel.chan_number, csa_vap_info->old_channel.en_bandwidth,
        csa_vap_info->new_channel, new_bandwidth);

    hmac_vap_linkloss_clean(hmac_vap);

    /* 禁止硬件全部发送直到STA信道切换完毕 */
    hmac_chan_disable_machw_tx(hmac_vap);

    /* 切换信道和带宽 */
    hmac_chan_multi_select_channel_mac(hmac_vap, csa_vap_info->new_channel, new_bandwidth);
    /* 设置该变量，避免STA在信道切换时发生link loss */
    hmac_switch_complete_notify(hmac_vap, OSAL_FALSE);
    /* 信道切换完成按照正常dtim睡眠唤醒 */
    hmac_set_psm_param_to_device(hmac_vap, OAL_FALSE, 0, OAL_FALSE, OSAL_FALSE);
    hmac_chan_enable_machw_tx(hmac_vap);

    /* 切完信道后删除BA，防止切换信道过程中将硬件队列清空导致BA移窗异常 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user != OSAL_NULL) {
        hmac_tx_delete_ba(hmac_vap, hmac_user);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ie_proc_wide_bandwidth_ie
 功能描述  : 处理Wide Bandwidth Channel Switch IE
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_ie_proc_wide_bandwidth_ie(hmac_vap_stru *hmac_vap, const osal_u8 *payload)
{
    osal_u8     new_channel;
    osal_u8     channel_width;
    osal_u8     center_freq_seg0;
    osal_u8     center_freq_seg1;
    osal_u32    check;
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);

    /* 非切换信道过程返回 */
    if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_FALSE) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ie_proc_wide_bandwidth_ie::not waiting shit channel,csa_status=%d}", hmac_vap->vap_id,
            hmac_sta_csa_fsm_get_current_state(hmac_vap));

        return OAL_SUCC;
    }

    new_channel = csa_vap_info->new_channel;

    /* 检查当前管制域是否支持该信道，如果不支持，则直接返回 */
    check = hmac_is_channel_num_valid_etc(hmac_vap->channel.band, new_channel);
    if (check != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ie_proc_wide_bandwidth_ie::hmac_is_channel_num_valid_etc failed[%d], new_chan=%d.}",
            hmac_vap->vap_id, check, new_channel);
        return check;
    }

    /* IE长度检查 */
    if (payload[1] < MAC_WIDE_BW_CH_SWITCH_IE_LEN) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_ie_proc_wide_bandwidth_ie::invalid wide bw ch switch ie len[%d]}",
            hmac_vap->vap_id, payload[1]);
        return OAL_FAIL;
    }

    channel_width = payload[MAC_IE_HDR_LEN];
    center_freq_seg0 = payload[MAC_IE_HDR_LEN + 1];
    center_freq_seg1 = payload[MAC_IE_HDR_LEN + 2]; /* 2表示指向element中的channel_center_freq_seg1位 */

    oam_warning_log4(0, OAM_SF_ANY,
        "vap_id[%d] {hmac_ie_proc_wide_bandwidth_ie:: channel_width=[%d] seg0=[%d] seg1=[%d]}",
        hmac_vap->vap_id, channel_width, center_freq_seg0, center_freq_seg1);
    // 2. 对于80M切换要区分fpga/asic
    if (channel_width > WLAN_MIB_VHT_OP_WIDTH_20_40) {
        csa_vap_info->new_bandwidth = mac_get_bandwith_from_center_freq_seg0_seg1(channel_width, new_channel,
            center_freq_seg0, center_freq_seg1);
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ie_proc_wide_bandwidth_ie::csa get new_bw=[%d] from MAC_EID_WIDE_BW_CH_SWITCH(IE194)}",
            hmac_vap->vap_id, csa_vap_info->new_bandwidth);
    } else {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_ie_proc_wide_bandwidth_ie::unused vht channel with[%d]}",
            hmac_vap->vap_id, channel_width);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ie_proc_csa_bw_ie
 功能描述  : 处理csa bw ie处理
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_ie_proc_csa_bw_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 frame_len)
{
    osal_u8 *ie = OSAL_NULL;
    hmac_vap_csa_stru *csa_vap_info = OSAL_NULL;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (payload == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_CSA, "{hmac_ie_proc_csa_bw_ie::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    csa_vap_info = hmac_csa_get_vap_info(hmac_vap);
    if (osal_unlikely(csa_vap_info == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_CSA,
            "{hmac_ie_proc_csa_bw_ie::param null, csa_vap_info[%p].}", (uintptr_t)csa_vap_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非切换信道过程返回 */
    if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    ie = mac_find_ie_etc(MAC_EID_SEC_CH_OFFSET, payload, frame_len);
    if ((ie != OSAL_NULL) && (ie[1] >= MAC_SEC_CH_OFFSET_IE_LEN)) {
        csa_vap_info->new_bandwidth = mac_get_bandwidth_from_sco(ie[MAC_IE_HDR_LEN]);
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_ie_proc_csa_bw_ie::csa get new_bw=[%d] from MAC_EID_SEC_CH_OFFSET(IE62)}",
            hmac_vap->vap_id, csa_vap_info->new_bandwidth);
    }

    ie = mac_find_ie_etc(MAC_EID_WIDE_BW_CH_SWITCH, payload, frame_len);
    if ((ie != OSAL_NULL) && (mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_TRUE)) {
        hmac_ie_proc_wide_bandwidth_ie(hmac_vap, ie);
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_do_ch_switch(hmac_vap_stru *hmac_vap, osal_u8 ch_sw_mode, osal_u8 new_chan, osal_u8 sw_cnt)
{
    oal_ieee80211_channel_sw_ie csa_info;

    if (hmac_vap->channel.chan_number != new_chan) {
        csa_info.new_ch_num     = new_chan;
        csa_info.mode           = ch_sw_mode;
        csa_info.count          = sw_cnt;

        hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_GET_IE, sizeof(csa_info), (osal_u8*)&csa_info);

        /* 重新设置信道，vap下的带宽切到normal状态 */
        if ((is_legacy_sta(hmac_vap)) && (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) &&
            (mac_vap_get_curren_bw_state(hmac_vap) != HMAC_STA_BW_SWITCH_FSM_NORMAL)) {
            hmac_sta_bw_switch_fsm_post_event(hmac_vap, HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC, 0, OSAL_NULL);
            /* 带宽切换状态机 */
            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_do_ch_switch::VAP CURREN BW STATE[%d].}",
                hmac_vap->vap_id, mac_vap_get_curren_bw_state(hmac_vap));
        }
    }
}

/*****************************************************************************
 功能描述  : 处理(Extended) Channel Switch Announcement IE
*****************************************************************************/
OSAL_STATIC osal_void hmac_ie_proc_ch_switch_ie(hmac_vap_stru *hmac_vap, const osal_u8 *payload,
    mac_eid_enum_uint8 eid_type)
{
    osal_u8    ch_sw_mode = 0;
    osal_u8    new_chan   = 0;
    osal_u8    sw_cnt     = 0;
    osal_u32   check;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (payload == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ie_proc_ch_switch_ie::param null.}");
        return;
    }

    /*************************************************************************/
    /*                    Channel Switch Announcement element                */
    /* --------------------------------------------------------------------- */
    /* |Element ID|Length |Channel switch Mode|New Channel| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1         |1      |1                  |1          |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*                Extended Channel Switch Announcement element           */
    /* --------------------------------------------------------------------- */
    /* |Elem ID|Length|Ch Switch Mode|New Reg Class|New Ch| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1      |1     |1             |1            |1     |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    if (eid_type == MAC_EID_CHANSWITCHANN) {
        if (payload[1] < MAC_CHANSWITCHANN_IE_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_ie_proc_ch_switch_ie::invalid chan switch ann ie len[%d]}",
                payload[1]);
            return;
        }
        /* Channel Switch Announcement element */
        ch_sw_mode = payload[MAC_IE_HDR_LEN];
        new_chan   = payload[MAC_IE_HDR_LEN + 1]; /* 1表示element中的New Channel位 */
        sw_cnt     = payload[MAC_IE_HDR_LEN + 2]; /* 2表示element中的Ch switch count位 */
    } else if (eid_type == MAC_EID_EXTCHANSWITCHANN) {
        if (payload[1] < MAC_EXT_CHANSWITCHANN_IE_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_ie_proc_ch_switch_ie::invalid ext chan switch ann ie len[%d]}",
                payload[1]);
            return;
        }
        /* Extended Channel Switch Announcement element */
        ch_sw_mode = payload[MAC_IE_HDR_LEN];
        /* Skip New Operating Class = payload[MAC_IE_HDR_LEN + 1]; */
        new_chan   = payload[MAC_IE_HDR_LEN + 2]; /* 2表示指向element中的New Channel */
        sw_cnt     = payload[MAC_IE_HDR_LEN + 3]; /* 3表示element中的Ch switch count位 */
    } else {
        return;
    }

    if ((hmac_sta_csa_fsm_get_current_state(hmac_vap) != WLAN_STA_CSA_FSM_INVALID) ||
        (sw_cnt != hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.sta_csa_last_cnt)) {
        oam_warning_log4(0, OAM_SF_ANY,
            "{hmac_ie_proc_ch_switch_ie::csa info  chan=%d mode=%d cnt=%d currt_channel=%d}", new_chan, ch_sw_mode,
            sw_cnt, hmac_vap->channel.chan_number);
    }

    /* 检查当前管制域是否支持该信道，如果不支持，则直接返回 */
    check = hmac_is_channel_num_valid_etc(hmac_vap->channel.band, new_chan);
    if (check != OAL_SUCC) {
        oam_warning_log3(0, 0, "vap_id[%d] {hmac_ie_proc_ch_switch_ie::ch valid failed[%d], new_chan=%d.}",
            hmac_vap->vap_id, check, new_chan);
        return;
    }
    hmac_do_ch_switch(hmac_vap, ch_sw_mode, new_chan, sw_cnt);
}

/*****************************************************************************
 函 数 名  : hmac_chan_update_csw_info
 功能描述  : STA侧处理Beacon帧中的(Extended) Channel Switch Announcement IE
*****************************************************************************/
OSAL_STATIC osal_void hmac_chan_update_csw_info(hmac_vap_stru *hmac_vap, const osal_u8 *payload, osal_u16 frame_len)
{
    osal_u16   index = 0;
    osal_u16   ch_index = 0xFFFF;

    if (mac_mib_get_spectrum_management_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }
    /*************************************************************************/
    /*                    Channel Switch Announcement element                */
    /* --------------------------------------------------------------------- */
    /* |Element ID|Length |Channel switch Mode|New Channel| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1         |1      |1                  |1          |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*                Extended Channel Switch Announcement element           */
    /* --------------------------------------------------------------------- */
    /* |Elem ID|Length|Ch Switch Mode|New Reg Class|New Ch| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1      |1     |1             |1            |1     |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    while (index < frame_len) {
        if (payload[index] == MAC_EID_CHANSWITCHANN) {
            ch_index = index;
            break;
        } else if (payload[index] == MAC_EID_EXTCHANSWITCHANN) {
            ch_index = index;
            break;
        }

        index += (MAC_IE_HDR_LEN + payload[index + 1]);
    }

    if (ch_index != 0xFFFF) {
        hmac_ie_proc_ch_switch_ie(hmac_vap, &payload[ch_index], (mac_eid_enum_uint8)payload[ch_index]);
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_ch_switch
 功能描述  : sta在UP状态下处理"Notify Channel Width"帧上报过滤
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_rx_ch_switch(hmac_vap_stru *hmac_vap, osal_u8 *frame, osal_u16 len)
{
    osal_u8 *ie = OSAL_NULL;
    osal_u8 *frame_body = frame;
    osal_u16 framebody_len = len;

    /* CSA 安全漏洞修改,上一次CSA 切换失败,不处理 CSA Action, 防止频繁切换攻击 */
    if (hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.is_switch_fail == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_up_rx_ch_switch::CSA no handle csa action!}");
        return;
    }

    if (framebody_len <= (MAC_ACTION_OFFSET_ACTION + 1)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_up_rx_ch_switch::framebody_len[%d]}", framebody_len);
        return;
    }

    framebody_len -= (MAC_ACTION_OFFSET_ACTION + 1);
    frame_body += (MAC_ACTION_OFFSET_ACTION + 1);

    ie = mac_find_ie_etc(MAC_EID_CHANSWITCHANN, frame_body, framebody_len);
    if (ie != OSAL_NULL) {
        hmac_ie_proc_ch_switch_ie(hmac_vap, ie, MAC_EID_CHANSWITCHANN);
    }

    hmac_ie_proc_csa_bw_ie(hmac_vap, frame_body, framebody_len);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_ext_ch_switch
 功能描述  : STA up状态接收Extended Channel Switch Announcement帧处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_rx_ext_ch_switch(hmac_vap_stru *hmac_vap, osal_u8 *frame_body, osal_u16 framebody_len)
{
    osal_u16 index;
    osal_u8 *wide_bw_ch_switch_ie = OSAL_NULL;

    /* CSA 安全漏洞修改,上一次CSA 切换失败,不处理 CSA  Action, 防止频繁切换攻击 */
    if (hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.is_switch_fail == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_up_rx_ext_ch_switch::CSA no handle csa action!}");
        return;
    }

    if (mac_mib_get_spectrum_management_implemented(hmac_vap) == OSAL_FALSE) {
        oam_info_log1(0, OAM_SF_RX,
            "vap_id[%d] {hmac_sta_up_rx_ext_ch_switch::Ignoring Spectrum Management frames.}", hmac_vap->vap_id);
        return;
    }

    hmac_ie_proc_ch_switch_ie(hmac_vap, frame_body, MAC_EID_EXTCHANSWITCHANN);

    /* 如果不支持VHT，则忽略后续的Wide Bandwidth Channel Switch IE */
    if (mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }

    index = 6; /* Wide Bandwidth IE的偏移地址为6 */

    wide_bw_ch_switch_ie =
        mac_find_ie_etc(MAC_EID_WIDE_BW_CH_SWITCH, &frame_body[index], (osal_s32)(framebody_len - index));
    if (wide_bw_ch_switch_ie != OSAL_NULL) {
        hmac_ie_proc_wide_bandwidth_ie(hmac_vap, wide_bw_ch_switch_ie);
    }
}
/*****************************************************************************
 函 数 名  : hmac_scan_check_assoc_ap_channel
 功能描述  : 扫描状态下检查ap channel是否变化，变化立即跟随切换
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_scan_check_assoc_ap_channel(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    const dmac_rx_ctl_stru *rx_ctrl = (const dmac_rx_ctl_stru *)oal_netbuf_cb_const(netbuf);
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /* 获取帧信息 */
    const mac_ieee80211_frame_stru *frame_hdr =
        (const mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    osal_u8 *frame_body = oal_netbuf_rx_data(netbuf);
    osal_u16 frame_len = (osal_u16)oal_netbuf_rx_frame_len(netbuf);
    mac_cfg_ssid_param_stru mib_ssid;
    osal_u8 mib_ssid_len = 0;
    osal_u8 frame_channel;
    osal_u8 idx;
    osal_u8 ssid_len = 0;
    osal_u32 ret;
    osal_u8 *ssid = OSAL_NULL;

    memset_s(&mib_ssid, sizeof(mib_ssid), 0, sizeof(mib_ssid));

    /* 非发往本机的管理帧 */
    if (memcmp(frame_hdr->address3, hmac_vap->bssid, WLAN_MAC_ADDR_LEN) != 0) {
        oam_info_log1(0, OAM_SF_SCAN, "{hmac_scan_check_assoc_ap_channel::[0x%02x].}", frame_hdr->address3[0]);
        return OAL_FAIL;
    }

    /* ssid信息不一致 */
    ssid = hmac_get_ssid_etc(frame_body, (osal_s32)(frame_len - MAC_80211_FRAME_LEN), &ssid_len);
    hmac_mib_get_ssid_etc(hmac_vap, &mib_ssid_len, (osal_u8 *)&mib_ssid);
    if ((ssid == OSAL_NULL) || (ssid_len == 0) || (mib_ssid.ssid_len != ssid_len) ||
        (memcmp(mib_ssid.ac_ssid, ssid, ssid_len) != 0)) {
        oam_info_log2(0, OAM_SF_SCAN, "{hmac_scan_check_assoc_ap_channel:: len[%u] mib_len[%u].}",
            ssid_len, mib_ssid.ssid_len);
        return OAL_FAIL;
    }

    /* 管理帧中获取信道信息 */
    frame_channel = hmac_ie_get_chan_num_etc(frame_body, (frame_len - MAC_80211_FRAME_LEN), offset,
                                             rx_ctrl->rx_info.channel_number);
    if (frame_channel == 0) {
        oam_info_log0(0, OAM_SF_SCAN, "{hmac_scan_check_assoc_ap_channel::channel = 0.}");
        return OAL_FAIL;
    }

    /* AP热band变化时，去关联 */
    ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, frame_channel, &idx);
    if (ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_scan_check_assoc_ap_channel::Get channel idx failed, DISASSOC! ch[%d], frame=[%d].}",
            hmac_vap->vap_id, hmac_vap->channel.chan_number, frame_channel);
        hmac_vap_linkloss_clean(hmac_vap);
        hmac_send_disassoc_misc_event(hmac_vap, hmac_vap->assoc_vap_id, DMAC_DISASOC_MISC_GET_CHANNEL_IDX_FAIL);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_scan_check_assoc_ap_channel_event(hmac_vap_stru *hmac_vap,
    scan_check_assoc_channel_enum_uint8 check_mode, osal_u8 frame_channel, osal_u8 channel_number)
{
    oal_ieee80211_channel_sw_ie csa_info;
    hmac_vap_csa_stru *csa_vap_info = hmac_csa_get_vap_info(hmac_vap);

    memset_s(&csa_info, sizeof(csa_info), 0, sizeof(csa_info));
    switch (check_mode) {
        /* LinkLoss 检查 */
        case SCAN_CHECK_ASSOC_CHANNEL_LINKLOSS:
            if (hmac_vap->channel.chan_number == frame_channel) {
                return;
            }
            /* 信道发生变化 */
            /* CSA 信道切换过程中直接退出 */
            if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) {
                hmac_vap_linkloss_clean(hmac_vap);
                oam_warning_log3(0, OAM_SF_SCAN,
                    "vap_id[%d] {hmac_scan_check_assoc_ap_channel_event::channel=[%d],fram=[%d];sta in csa process.}",
                    hmac_vap->vap_id, hmac_vap->channel.chan_number, frame_channel);
                return;
            }
            /* 触发CSA流程  */
            csa_info.new_ch_num = frame_channel;
            csa_info.mode = OSAL_TRUE;
            csa_info.count = 0;
            /* 带宽先启动在20M, Beacon帧返回真正带宽 */
            csa_vap_info->new_bandwidth = WLAN_BAND_WIDTH_20M;
            oam_warning_log4(0, OAM_SF_SCAN,
                "vap_id[%d] {hmac_scan_check_assoc_ap_channel_event::Trig csa, switch ch from [%d] to [%d], bw=[%d]}",
                hmac_vap->vap_id, hmac_vap->channel.chan_number, frame_channel,
                csa_vap_info->new_bandwidth);
            hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_GET_IE, sizeof(csa_info), (osal_u8 *)&csa_info);
            /* 重新设置信道，vap下的带宽切到normal状态 */
            if ((is_legacy_sta(hmac_vap)) && (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) &&
                (mac_vap_get_curren_bw_state(hmac_vap) != HMAC_STA_BW_SWITCH_FSM_NORMAL)) {
                hmac_sta_bw_switch_fsm_post_event(hmac_vap, HMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC, 0, OSAL_NULL);
                /* 带宽切换状态机 */
                oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_ie_proc_ch_switch_ie::VAP CURREN BW STATE[%d].}",
                    hmac_vap->vap_id, mac_vap_get_curren_bw_state(hmac_vap));
            }
            break;
        case SCAN_CHECK_ASSOC_CHANNEL_CSA:
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_scan_check_assoc_ap_channel_event::csa scan probe_rsp_ch=[%d].}",
                hmac_vap->vap_id, frame_channel);

            /* 对于实际信道与IE指示信道不一致时忽略 */
            if (channel_number != frame_channel) {
                oam_warning_log3(0, OAM_SF_CSA,
                    "vap_id[%d] {hmac_scan_check_assoc_ap_channel_event::current channel[%d], frame_channel=[%d]}",
                    hmac_vap->vap_id, channel_number, frame_channel);
                break;
            }
            hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_RCV_PROBE_RSP,
                sizeof(frame_channel), &frame_channel);
            break;
        default:
            break;
    }
    return;
}

/*****************************************************************************
 功能描述  : vap关联的csa数据结构初始化
*****************************************************************************/
OSAL_STATIC osal_bool hmac_csa_sta_vap_add(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_vap_csa_stru *mem_ptr = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;

    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    if (g_csa_sta_vapinfo[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_csa_sta_vap_add mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = (hmac_vap_csa_stru *)osal_kmalloc(sizeof(hmac_vap_csa_stru), OSAL_GFP_KERNEL);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CSA, "hmac_csa_sta_vap_add mem alloc fail");
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_vap_csa_stru), 0, sizeof(hmac_vap_csa_stru));
    /* 注册特性数据结构 */
    g_csa_sta_vapinfo[vap_id] = mem_ptr;

    hmac_sta_csa_fsm_attach(hmac_vap);

    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : vap关联的csa数据结构去初始化
*****************************************************************************/
OSAL_STATIC osal_bool hmac_csa_sta_vap_del(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_vap_csa_stru *csa_vap_info = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;

    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    csa_vap_info = g_csa_sta_vapinfo[vap_id];
    if (csa_vap_info == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CSA, "hmac_csa_sta_vap_del get feature fail");
        return OSAL_FALSE;
    }

    /* 删除CSA保护定时器 */
    if (csa_vap_info->csa_stop_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(csa_vap_info->csa_stop_timer));
    }

    if (csa_vap_info->sta_csa_fsm_info.csa_handle_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(csa_vap_info->sta_csa_fsm_info.csa_handle_timer));
    }
    /* STA CSA状态机删除 */
    hmac_sta_csa_fsm_detach(hmac_vap);

    g_csa_sta_vapinfo[vap_id] = OSAL_NULL;

    osal_kfree(csa_vap_info);

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_csa_sta_users_add(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_vap_csa_stru *csa_vap_info = OSAL_NULL;

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "{hmac_csa_proc_users_del:vap[%d] is null.}", hmac_user->vap_id);
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    csa_vap_info = hmac_csa_get_vap_info(hmac_vap);
    csa_vap_info->new_channel = 0;
    csa_vap_info->new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    memset_s(&csa_vap_info->old_channel, sizeof(mac_channel_stru), 0, sizeof(mac_channel_stru));

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_csa_sta_users_del(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "{hmac_csa_proc_users_del:vap[%d] is null.}", hmac_user->vap_id);
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    /* BEGIN:STA 去关联时，恢复硬件发送，
       避免由于MAC PA 寄存器发送挂起导致不能继续关联 */
    if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) {
        hal_set_machw_tx_resume();
        oam_warning_log0(0, 0, "{hmac_csa_proc_users_del:back to init, resume MAC.}");
    }

    hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OSAL_NULL);

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_csa_sta_tbtt_event(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) {
        hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_TBTT, 0, OSAL_NULL);
    }
    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_csa_sta_vap_state_change(osal_void *notify_data)
{
    hmac_vap_state_change_notify_stru *state_change_notify = (hmac_vap_state_change_notify_stru *)notify_data;
    hmac_vap_stru *hmac_vap = state_change_notify->hmac_vap;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    if (state_change_notify->new_state == MAC_VAP_STATE_UP) {
        hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OSAL_NULL);
    } else if (state_change_notify->new_state == MAC_VAP_STATE_ROAMING) {
        if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) {
            oam_warning_log1(0, 0, "vap_id[%d] hmac_csa_sta_vap_state_change: csa status to INIT ", hmac_vap->vap_id);
            hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OSAL_NULL);
        }
    }

    return OSAL_TRUE;
}

OSAL_STATIC osal_void hmac_csa_sta_rx_beacon_probe_rsp(oal_netbuf_stru *netbuf, hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf); /* 获取帧头信息 */
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    osal_u8 frame_channel_num;
    osal_u16 msg_len;
    osal_u16 ie_offset;
    osal_u8 *payload = oal_netbuf_rx_data(netbuf);

    msg_len = mac_get_rx_cb_payload_len((mac_rx_ctl_stru *)(&(rx_ctl->rx_info))); /* 帧体长度 */
    ie_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    /* 来自其它bss的Beacon不做处理 */
    if (oal_compare_mac_addr(hmac_vap->bssid, frame_hdr->address3) != 0) {
        return;
    }

    if (msg_len <= ie_offset) {
        oam_warning_log3(0, 0, "vap_id[%d] {hmac_csa_sta_rx_beacon_probe_rsp: msg_len[%d] <= ie_offset[%d].",
            hmac_vap->vap_id, msg_len, ie_offset);
        return;
    }

    if (frame_hdr->frame_control.sub_type == WLAN_BEACON) {
        /* 信道切换处理 */
        if (hmac_csa_get_vap_info(hmac_vap)->sta_csa_fsm_info.is_switch_fail == OAL_FALSE) {
            hmac_chan_update_csw_info(hmac_vap, (payload + ie_offset), (msg_len - ie_offset));
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_csa_sta_rx_beacon_probe_rsp::CSA no handle!}");
        }
        /* 带宽参数处理 */
        hmac_ie_proc_csa_bw_ie(hmac_vap, (payload + ie_offset), (msg_len - ie_offset));
    }

    /* 扫描下，收到Beacon或者probe rsp处理 */
    if (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING &&
        (hmac_device->scan_params.scan_func & MAC_SCAN_FUNC_BSS) == OSAL_FALSE) {
        return;
    }

    if (hmac_scan_check_bss_type(payload, &(hmac_device->scan_params)) != OAL_SUCC) {
        return;
    }

    if (hmac_sta_csa_fsm_get_current_state(hmac_vap) != WLAN_STA_CSA_FSM_WAIT) {
        return;
    }

    if (hmac_scan_check_assoc_ap_channel(hmac_vap, netbuf) != OAL_SUCC) {
        return;
    }

    frame_channel_num = hmac_ie_get_chan_num_etc(payload, msg_len, ie_offset, rx_ctl->rx_info.channel_number);
    /* STA扫描且linkloss超过门限1/4时，进行AP信道是否切换的识别 */
    /* 右移2位，获得linkloss的1/4,进行切换识别 */
    if ((get_current_linkloss_threshold(hmac_vap) >> 2) <= get_current_linkloss_cnt(hmac_vap)) {
        hmac_scan_check_assoc_ap_channel_event(hmac_vap, SCAN_CHECK_ASSOC_CHANNEL_LINKLOSS,
            frame_channel_num, rx_ctl->rx_info.channel_number);
    }

    if (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_CSA) {
        hmac_scan_check_assoc_ap_channel_event(hmac_vap, SCAN_CHECK_ASSOC_CHANNEL_CSA,
            frame_channel_num, rx_ctl->rx_info.channel_number);
        /* linkloss CSA扫描不需要上报报文或扫描结果到host */
        // *go_on = OSAL_FALSE;
    }
}

OSAL_STATIC osal_u32 hmac_csa_sta_rx_mgmt(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf); /* 获取帧头信息 */
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    osal_u16 msg_len;
    osal_u8 *payload = OSAL_NULL;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OAL_CONTINUE;
    }

    if (frame_hdr->frame_control.type != WLAN_MANAGEMENT) {
        return OAL_CONTINUE;
    }

    if (frame_hdr->frame_control.sub_type == WLAN_BEACON || frame_hdr->frame_control.sub_type == WLAN_PROBE_RSP) {
        hmac_csa_sta_rx_beacon_probe_rsp(*netbuf, hmac_vap);
    } else if (frame_hdr->frame_control.sub_type == WLAN_ACTION) {
        payload = oal_netbuf_rx_data(*netbuf);
        msg_len = mac_get_rx_cb_payload_len((mac_rx_ctl_stru *)(&(rx_ctl->rx_info))); /* 帧体长度 */
        if (payload[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_PUBLIC &&
            payload[MAC_ACTION_OFFSET_ACTION] == MAC_PUB_EX_CH_SWITCH_ANNOUNCE) {
            hmac_sta_up_rx_ext_ch_switch(hmac_vap, payload, msg_len);
        } else if (payload[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_SPECMGMT &&
            payload[MAC_ACTION_OFFSET_ACTION] == MAC_SPEC_CH_SWITCH_ANNOUNCE) {
            hmac_sta_up_rx_ch_switch(hmac_vap, payload, msg_len);
        }
    }

    return OAL_CONTINUE;
}

OSAL_STATIC osal_bool hmac_csa_sta_scan_end(osal_void *notify_data)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)notify_data;
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    if (hmac_device->scan_params.scan_mode != WLAN_SCAN_MODE_BACKGROUND_CSA) {
        return OSAL_TRUE;
    }

    oam_warning_log0(0, OAM_SF_SCAN, "{hmac_csa_sta_scan_end::scan_mode BACKGROUND_CSA}");
    hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_SCAN_END, 0, OSAL_NULL);
    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_csa_sta_roam_active(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_TRUE;
    }

    /* 防止漫游阶段触发csa扫描 */
    if (hmac_sta_csa_fsm_is_waiting(hmac_vap) == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ROAM,
            "vap_id[%d] hmac_config_roam_enable_active:vap_state into ROAMING, csa back to INIT", hmac_vap->vap_id);
        hmac_sta_csa_fsm_post_event(hmac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OSAL_NULL);
    }
    return OSAL_TRUE;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_s32 hmac_ccpriv_get_channel(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    unref_param(param);

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_get_channel::vap null}.");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    wifi_printf("   current channel: [%d]\r\n",
        (hmac_vap->channel.chan_number == 16) ? 0 : (hmac_vap->channel.chan_number)); // 16信道代替0信道
#else
    wifi_printf("   current channel: [%d]\r\n", hmac_vap->channel.chan_number);
#endif
    wifi_printf("   channel index  : [%d]\r\n", hmac_vap->channel.chan_idx);
    wifi_printf("   channel band   : [%d][0:2G 1:5G]\r\n", hmac_vap->channel.band);
    wifi_printf("   chan bandwidth : [%d]\r\n", hmac_vap->channel.en_bandwidth);
    wifi_printf("   chan bandwidth : [0:20M 1:40+ 2:40- 3:80++ 4:80+- 5:80-+ 6:80-- 7:5 8:10 9:40 10:80]\r\n");

    return OAL_SUCC;
}
#endif

hmac_netbuf_hook_stru g_csa_sta_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_LOW,
    .hook_func = hmac_csa_sta_rx_mgmt,
};

osal_u32 hmac_csa_sta_init(osal_void)
{
    osal_u32 ret;
    /* 注册监听 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_csa_sta_vap_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_csa_sta_vap_del);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_csa_sta_users_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_csa_sta_users_del);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_TBTT_HANDLE, hmac_csa_sta_tbtt_event);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_VAP_STATE_CHANGE, hmac_csa_sta_vap_state_change);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_SCAN_END, hmac_csa_sta_scan_end);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ROAM_ACTIVE, hmac_csa_sta_roam_active);

    ret = hmac_register_netbuf_hook(&g_csa_sta_netbuf_hook);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_csa_sta_init:: MGMT RX IN register_netbuf_hooks error!");
        return ret;
    }
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING, hmac_sta_csa_fsm_is_waiting);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_register((const osal_s8 *)"get_channel", hmac_ccpriv_get_channel);
#endif

    return OAL_SUCC;
}

osal_void hmac_csa_sta_deinit(osal_void)
{
    /* 去注册监听 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_csa_sta_vap_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_csa_sta_vap_del);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_csa_sta_users_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_csa_sta_users_del);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_TBTT_HANDLE, hmac_csa_sta_tbtt_event);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_VAP_STATE_CHANGE, hmac_csa_sta_vap_state_change);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_SCAN_END, hmac_csa_sta_scan_end);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ROAM_ACTIVE, hmac_csa_sta_roam_active);

    hmac_unregister_netbuf_hook(&g_csa_sta_netbuf_hook);
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_unregister((const osal_s8 *)"get_channel");
#endif
    return;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
