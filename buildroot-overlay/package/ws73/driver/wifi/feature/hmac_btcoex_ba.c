/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: BA相关操作
 * Date: 2023-02-09 09:51
 */
#ifndef __HMAC_BTCOEX_BA_C__
#define __HMAC_BTCOEX_BA_C__

#include "hmac_roam_if.h"
#include "oal_netbuf_data.h"
#include "hmac_tx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_hook.h"
#include "hmac_feature_interface.h"
#include "hmac_btcoex.h"
#include "hmac_btcoex_ba.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BTCOEX_BA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define WLAN_AMPDU_TX_MAX_NUM_BTOCEX 4   // 共存场景下聚合度

/*****************************************************************************
  函数声明
*****************************************************************************/
static osal_s32 hmac_btcoex_rx_delba_trigger_etc(hmac_vap_stru *hmac_vap,
    hmac_to_hmac_btcoex_rx_delba_trigger_event_stru *btcoex_rx_delba);
/*****************************************************************************
  函数实现
*****************************************************************************/
osal_void hmac_btcoex_update_ba_size(const hmac_vap_stru *hmac_vap, hmac_user_btcoex_delba_stru *btcoex_delba,
    const hal_btcoex_btble_status_stru *btble_status)
{
    const bt_status_stru *bt_status;
    wlan_channel_band_enum_uint8 band;
    wlan_bw_cap_enum_uint8 bandwidth;
    btcoex_active_mode_enum_uint8 bt_active_mode;
    btcoex_rx_window_size_grade_enum_uint8 bt_rx_win_size_grade;

    bt_status = &(btble_status->bt_status.bt_status);

    band = hmac_vap->channel.band;
    hmac_vap_get_bandwidth_cap_etc(hmac_vap, &bandwidth);
    if ((band >= WLAN_BAND_BUTT) || (bandwidth >= WLAN_BW_CAP_BUTT)) {
        oam_error_log3(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_update_ba_size::band %d, bandwidth %d exceed scope!}",
            hmac_vap->vap_id, band, bandwidth);
        return;
    }

    if (btcoex_delba == OSAL_NULL) {
        return;
    }

    if (btcoex_delba->ba_size_expect_index >= BTCOEX_RX_WINDOW_SIZE_INDEX_BUTT) {
        oam_error_log2(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_update_ba_size::ba_size_expect_index %d exceed scope!}", hmac_vap->vap_id,
            btcoex_delba->ba_size_expect_index);
        return;
    }

    if (bt_status->bt_6slot != 0) {
        bt_active_mode = BTCOEX_ACTIVE_MODE_SCO;

        /* 6slot 设备 */
        if (bt_status->bt_6slot == 2) { /* 6slot为2时才将ba_size设为1 */
            btcoex_delba->ba_size = 1;
            return;
        }
    } else if (bt_status->bt_a2dp != 0) {
        bt_active_mode = BTCOEX_ACTIVE_MODE_A2DP;
    } else if (bt_status->bt_transfer != 0) {
        bt_active_mode = BTCOEX_ACTIVE_MODE_TRANSFER;
    } else {
        bt_active_mode = BTCOEX_ACTIVE_MODE_BUTT;
    }

    /* BT没有业务, 聚合64 */
    if (bt_active_mode >= BTCOEX_ACTIVE_MODE_BUTT) {
        bt_active_mode = BTCOEX_ACTIVE_MODE_A2DP;
        btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
    }
    if (btcoex_delba->user_nss_num >= WLAN_DOUBLE_NSS) {
#if defined(WLAN_MAX_NSS_NUM) && defined(WLAN_SINGLE_NSS) && (WLAN_MAX_NSS_NUM != WLAN_SINGLE_NSS)
        bt_rx_win_size_grade = hmac_get_g_rx_win_size_grage_mimo(band, bandwidth, bt_active_mode);
        btcoex_delba->ba_size = hmac_get_g_rx_win_size_mimo(bt_rx_win_size_grade, btcoex_delba->ba_size_expect_index);
#endif
    } else {
        bt_rx_win_size_grade = hmac_get_g_rx_win_size_grage_siso(band, bandwidth, bt_active_mode);
        btcoex_delba->ba_size = hmac_get_g_rx_win_size_siso(bt_rx_win_size_grade, btcoex_delba->ba_size_expect_index);
    }
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_delba_trigger
 功能描述  : Change ba size
*****************************************************************************/
osal_void hmac_btcoex_delba_trigger(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 need_delba,
    hmac_user_btcoex_delba_stru *btcoex_delba)
{
    hmac_to_hmac_btcoex_rx_delba_trigger_event_stru d2h_btcoex_rx_delba;

    if (hmac_vap == OSAL_NULL) {
        return;
    }

    /* 触发了一次主动删建BA，删建标记使能，后续单wifi自己删建BA需要采用共存配置的门限 */
    if (need_delba == OSAL_TRUE) {
        btcoex_delba->delba_trigger = OSAL_TRUE;

        /* 6slot立即删聚合了，需要标记状态，用于后续双链路不要频繁删聚合；删到64时恢复标记 */
        if (btcoex_delba->ba_size == 1) {
            hmac_btcoex_get_vap_info(hmac_vap)->delba_on_6slot = OSAL_TRUE;
        } else if (btcoex_delba->ba_size == 64) { /* when buffer size is 64 */
            hmac_btcoex_get_vap_info(hmac_vap)->delba_on_6slot = OSAL_FALSE;
        }
    }

    d2h_btcoex_rx_delba.need_delba = need_delba;
    d2h_btcoex_rx_delba.ba_size = btcoex_delba->ba_size;
    d2h_btcoex_rx_delba.user_id = hmac_vap->assoc_vap_id;

    (osal_void)hmac_btcoex_rx_delba_trigger_etc(hmac_vap, &d2h_btcoex_rx_delba);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_delba_event_process
 功能描述  : BT音乐和数传下，针对聚合聚合个数调整, 配置聚合个数，在接收速率逻辑进行删除BA操作
             (1)02和03方式处理不一致， 02是主场景，03只是存在兼容性时，删到第二档
             (2)02下音乐和数传处理不同，03下处理一致，都需要延迟删除
             (3)sta gc vap级别定时器处理
*****************************************************************************/
osal_u32 hmac_btcoex_delba_event_process(const hal_btcoex_btble_status_stru *btble_status,
    hmac_vap_stru *hmac_vap)
{
    hmac_vap_btcoex_rx_statistics_stru *btcoex_rx_stat = OSAL_NULL;
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    oal_bool_enum_uint8 need_delba = OSAL_FALSE;

    /* 如果不开启删聚合 直接返回 */
    if (hal_btcoex_check_sw_preempt_delba_on(hmac_vap->hal_device) == OSAL_FALSE) {
        return OAL_SUCC;
    }

    /* c1 siso不需要执行 */
    if (hmac_vap->hal_device->hal_m2s_fsm.oal_fsm.cur_state == HAL_M2S_STATE_SISO &&
        hmac_vap->hal_device->cfg_cap_info->phy_chain == WLAN_PHY_CHAIN_ONE) {
        return OAL_SUCC;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_error_log1(0, 0, "vap_id[%d] hmac_btcoex_delba_event_process::user IS NULL.", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    btcoex_rx_stat = &(hmac_btcoex_get_vap_info(hmac_vap)->hmac_vap_btcoex_rx_statistics);
    btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);

    /* 音乐和数传暂时处理方式一致 */
    /* 正常bt音乐和数传打开和关闭场景下，对聚合进行处理 */
    if ((btble_status->bt_status.bt_status.bt_a2dp != 0) || (btble_status->bt_status.bt_status.bt_transfer != 0)) {
        if (btcoex_delba->ba_size_real_index == BTCOEX_RX_WINDOW_SIZE_INDEX_3) {
            btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_2;
        } else {
            btcoex_delba->ba_size_expect_index = btcoex_delba->ba_size_real_index;
        }

        /* RX速率统计开关 */
        btcoex_rx_stat->rx_rate_statistics_flag = OSAL_TRUE;
    } else {
        /* 只有电话, 音乐和数传都没有的情况才进行恢复成64的聚合 */
        if (btble_status->bt_status.bt_status.bt_6slot == OSAL_FALSE) {
            btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
            btcoex_rx_stat->rx_rate_statistics_flag = OSAL_FALSE;
        } else {
            /* 音乐结束的时候还有电话的场景，在这里不进行BA删建处理，由电话的流程控制 */
            return OAL_SUCC;
        }
    }

    hmac_btcoex_update_ba_size(hmac_vap, btcoex_delba, btble_status);

    oam_warning_log4(0, OAM_SF_COEX,
        "{hmac_btcoex_delba_event_process:: a2dp status:%d, transfer status changed:%d, need_delba:%d,ba_size:%d.}",
        btble_status->bt_status.bt_status.bt_a2dp, btble_status->bt_status.bt_status.bt_transfer, need_delba,
        btcoex_delba->ba_size);

    /* 默认都不立即删除BA, 接收速率统计逻辑中来处理删除BA操作 */
    hmac_btcoex_delba_trigger(hmac_vap, need_delba, btcoex_delba);

    btcoex_rx_stat->rx_rate_statistics_timeout = OSAL_FALSE;

    if (btcoex_rx_stat->bt_coex_statistics_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(btcoex_rx_stat->bt_coex_statistics_timer));
    }
    frw_create_timer_entry(&(btcoex_rx_stat->bt_coex_statistics_timer), hmac_btcoex_rx_rate_statistics_flag_callback,
        BTCOEX_RX_STATISTICS_TIME, (osal_void *)hmac_vap, OSAL_TRUE);

    return OAL_SUCC;
}

osal_u32 hmac_btcoex_config_tx_aggr_num(osal_u8 aggt_num)
{
    osal_u32 ret = OAL_FAIL;
    frw_msg msg = {0};
    msg.data = &aggt_num;
    msg.data_len = (osal_u16)sizeof(osal_u8);
    ret = (osal_u32)frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_BTCOEX_SET_AGGR_NUM, &msg, OSAL_FALSE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_btcoex_config_tx_aggr_num:: frw_send_msg [%d].}", ret);
    }
    return ret;
}

OSAL_STATIC osal_void hmac_btcoex_get_expect_ba_size(hmac_vap_stru *hmac_vap, hmac_user_btcoex_delba_stru *btcoex_delba,
    const hal_btcoex_btble_status_stru *btble_status)
{
    wlan_bw_cap_enum_uint8 bandwidth;
    osal_u8 btcoex_max_aggr_num = WLAN_AMPDU_TX_MAX_NUM;
    hmac_vap_get_bandwidth_cap_etc(hmac_vap, &bandwidth);
    btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;

    if (btble_status->ble_status.ble_status.ble_con_num >= BTCOEX_BLE_CONN_NUM_THOLD ||
        (btble_status->ble_status.ble_status.ble_hid == OSAL_TRUE)) {
        btcoex_delba->ba_size_expect_index = BTCOEX_RX_WINDOW_SIZE_INDEX_1;
        btcoex_max_aggr_num = WLAN_AMPDU_TX_MAX_NUM_BTOCEX;
    }

    btcoex_delba->ba_size = hmac_get_g_rx_ba_win_size(bandwidth, btcoex_delba->ba_size_expect_index);
    hmac_btcoex_config_tx_aggr_num(btcoex_max_aggr_num); /* 发消息到device侧修改tx最大聚合度 */
}

osal_s32 hmac_btcoex_action_dela_ba_handler(hmac_vap_stru *hmac_vap)
{
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hmac_vap_stru *hmac_valid_vap = OSAL_NULL;
    hal_btcoex_btble_status_stru *btble_status = OSAL_NULL;
    osal_u8 vap_idx;
    osal_u8 valid_vap_num;
    hmac_user_btcoex_delba_stru btcoex_delba = {0};
    hal_to_dmac_device_stru *hal_device;

    unref_param(hmac_vap);
    hal_device = &hal_get_chip_stru()->device.hal_device_base;
    btble_status = hal_btcoex_btble_status();
    valid_vap_num = hmac_btcoex_find_all_valid_sta_per_device(hal_device, mac_vap_id,
        WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_idx = 0; vap_idx < valid_vap_num; vap_idx++) {
        hmac_valid_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_idx]);
        if (hmac_valid_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_COEX, "{hmac_btcoex_mul_conn_status_handler::hmac_vap[%d] is NULL.}",
                mac_vap_id[vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
        hmac_btcoex_get_expect_ba_size(hmac_valid_vap, &btcoex_delba, btble_status);
        hmac_btcoex_delba_trigger(hmac_valid_vap, OSAL_TRUE, &btcoex_delba);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_delba_foreach_tid
 功能描述  : 共存删除BA会话
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_btcoex_delba_foreach_tid(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_cfg_delba_req_param_stru *mac_cfg_delba_param)
{
    osal_u32 ul_ret;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    oal_set_mac_addr(mac_cfg_delba_param->mac_addr, hmac_user->user_mac_addr);
    mac_cfg_delba_param->trigger  = MAC_DELBA_TRIGGER_BTCOEX;

    for (mac_cfg_delba_param->tidno = 0; mac_cfg_delba_param->tidno < WLAN_TID_MAX_NUM;
        mac_cfg_delba_param->tidno++) {
        msg_info.data = (osal_u8 *)mac_cfg_delba_param;
        msg_info.data_len = 0;
        ul_ret = (osal_u32)hmac_config_delba_req_etc(hmac_vap, &msg_info);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log3(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_delba_foreach_tid::ul_ret: %d, tid: %d}",
                hmac_vap->vap_id, ul_ret, mac_cfg_delba_param->tidno);
            return ul_ret;
        }
    }

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_delba_foreach_user
 功能描述  : hmac删除BA
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_btcoex_delba_from_user(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    mac_cfg_delba_req_param_stru mac_cfg_delba_param;
    osal_void *fhook;

    memset_s((osal_u8 *)&mac_cfg_delba_param, OAL_SIZEOF(mac_cfg_delba_param), 0, OAL_SIZEOF(mac_cfg_delba_param));

    mac_cfg_delba_param.direction = MAC_RECIPIENT_DELBA;

    /* 属于黑名单AP，并且已经处于电话业务时，不进行删BA逻辑 */
    if (hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.ba_handle_allow == OAL_FALSE) {
        if (hmac_btcoex_get_blacklist_type(hmac_user) == BTCOEX_BLACKLIST_TPYE_FIX_BASIZE) {
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_delba_from_user::DO NOT DELBA.}");
        } else {
            oam_warning_log0(0, OAM_SF_COEX, "{hmac_btcoex_delba_from_user::need to reassoc to READDBA.}");
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_START);
            if (fhook != OSAL_NULL) {
                ((hmac_roam_start_etc_cb)fhook)(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_TRUE, ROAM_TRIGGER_COEX);
            }
            /* 重关联之后，刷新为允许建立聚合 */
            hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.ba_handle_allow = OAL_TRUE;

            /* 保证wifi恢复聚合64 */
            hmac_btcoex_get_user_info(hmac_user)->ba_size = 0;
        }

        return OAL_FAIL;
    }

    /* 开始arp探测统计 */
    hmac_btcoex_get_user_info(hmac_user)->arp_probe_on = OAL_TRUE;

    return hmac_btcoex_delba_foreach_tid(hmac_vap, hmac_user, &mac_cfg_delba_param);
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_rx_delba_trigger_etc
 功能描述  : hmac删除BA
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_btcoex_rx_delba_trigger_etc(hmac_vap_stru *hmac_vap,
    hmac_to_hmac_btcoex_rx_delba_trigger_event_stru *btcoex_rx_delba)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_s32 ul_ret;

    hmac_user = mac_res_get_hmac_user_etc(btcoex_rx_delba->user_id);
    if (OAL_UNLIKELY(hmac_user == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_rx_delba_trigger_etc::hmac_user is null! user_id is %d.}",
            hmac_vap->vap_id, btcoex_rx_delba->user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_btcoex_get_user_info(hmac_user)->ba_size = (osal_u16)btcoex_rx_delba->ba_size;

    if (btcoex_rx_delba->need_delba == OAL_TRUE) {
        /* 刷新共存触发删建BA标记 */
        hmac_btcoex_get_user_info(hmac_user)->delba_btcoex_trigger  = OAL_TRUE;
    }

    oam_warning_log4(0, OAM_SF_COEX,
        "vap_id[%d] {hmac_btcoex_rx_delba_trigger_etc:delba size:%d, need_delba:%d, delba_trigger:%d.}",
        hmac_vap->vap_id, btcoex_rx_delba->ba_size,
        btcoex_rx_delba->need_delba, hmac_btcoex_get_user_info(hmac_user)->delba_btcoex_trigger);

    if (btcoex_rx_delba->need_delba) {
        ul_ret = (osal_s32)hmac_btcoex_delba_from_user(hmac_vap, hmac_user);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_COEX,
                "vap_id[%d] {hmac_btcoex_rx_delba_trigger_etc:delba send failed:ul_ret: %d.}",
                hmac_vap->vap_id, ul_ret);
            return ul_ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_delba_send_timeout
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_btcoex_delba_send_timeout(osal_void *p_arg)
{
    hmac_btcoex_arp_req_process_stru *arp_req_process;
    hmac_user_btcoex_stru            *hmac_user_btcoex;
    hmac_user_stru                   *hmac_user;
    osal_s32                         val;
    osal_void *fhook;

    hmac_user = (hmac_user_stru *)p_arg;
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_COEX, "{hmac_btcoex_delba_send_timeout::hmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user_btcoex = hmac_btcoex_get_user_info(hmac_user);

    arp_req_process = &(hmac_user_btcoex->hmac_btcoex_arp_req_process);

    val = osal_adapt_atomic_read(&(arp_req_process->rx_unicast_pkt_to_lan));
    if (val == 0) {
        hmac_vap_stru *hmac_vap;

        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
        if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
            oam_error_log1(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_delba_send_timeout::hmac_vap is null!}",
                hmac_user->vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        hmac_user_btcoex->rx_no_pkt_count++;

        oam_warning_log2(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_delba_send_timeout::rx_pkt[%d]!}",
            hmac_user->vap_id, hmac_user_btcoex->rx_no_pkt_count);

        if (hmac_user_btcoex->rx_no_pkt_count > BTCOEX_ARP_FAIL_REASSOC_NUM) {
            /* 重关联逻辑暂时关闭，等统计出现哪些场景出现不通，才限制场景放开 */
            oam_warning_log1(0, OAM_SF_COEX, "vap_id[%d] {hmac_btcoex_delba_send_timeout::need to reassoc to resume.}",
                hmac_user->vap_id);

            /* 停止arp探测 */
            hmac_user_btcoex->arp_probe_on = OAL_FALSE;
            /* 发起reassoc req */
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_START);
            if (fhook != OSAL_NULL) {
                ((hmac_roam_start_etc_cb)fhook)(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_TRUE, ROAM_TRIGGER_COEX);
            }
            /* 保证wifi恢复聚合64 */
            hmac_btcoex_get_user_info(hmac_user)->ba_size = 0;
        } else if (hmac_user_btcoex->rx_no_pkt_count > BTCOEX_ARP_FAIL_DELBA_NUM) {
            hmac_btcoex_delba_from_user(hmac_vap, hmac_user);
        }
    } else {
        /* 停止arp探测 */
        hmac_user_btcoex->arp_probe_on = OAL_FALSE;

        hmac_user_btcoex->rx_no_pkt_count = 0;
    }

    osal_adapt_atomic_set(&arp_req_process->rx_unicast_pkt_to_lan, 0);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_arp_fail_delba_process_etc
 功能描述  : 发送方向arp帧成功率统计(发送arp req之后进行rx方向单播数据帧统计，一直收不到，触发删聚合逻辑)
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_arp_fail_delba_process_etc(oal_netbuf_stru **pst_netbuf, hmac_vap_stru *hmac_vap)
{
    hmac_btcoex_arp_req_process_stru *arp_req_process;
    hmac_user_btcoex_stru            *hmac_user_btcoex;
    mac_ether_header_stru            *mac_ether_hdr;
    hmac_user_stru                   *hmac_user;
    osal_u8                         data_type;

    /* 若该STA不存在，则直接返回，不处理，不影响该函数原调用处的流程 */
    if (mac_btcoex_check_valid_sta(hmac_vap) != OAL_TRUE) {
        return OAL_CONTINUE;
    }

    mac_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(*pst_netbuf);

    /* 若该USER不存在，则直接返回，不处理，不影响该函数原调用处的流程 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (OAL_UNLIKELY(hmac_user == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_btcoex_arp_fail_delba_process_etc::hmac_user is null!assoc_vap is %d.}",
            hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        return OAL_CONTINUE;
    }

    /* 只要统计功能打开，就需要做一次探测 */
    hmac_user_btcoex = hmac_btcoex_get_user_info(hmac_user);
    if ((hmac_user_btcoex->ba_size > 0) && (hmac_user_btcoex->arp_probe_on == OAL_TRUE)) {
        /* 参数外面已经做检查，里面没必要再做检查了 */
        data_type =  hmac_get_data_type_from_8023_etc((osal_u8 *)mac_ether_hdr, MAC_NETBUFF_PAYLOAD_ETH);

        arp_req_process = &(hmac_user_btcoex->hmac_btcoex_arp_req_process);

        /* 发送方向创建定时器，多次创建定时器 */
        if ((data_type == MAC_DATA_ARP_REQ) &&
            (arp_req_process->delba_opt_timer.is_registerd == OAL_FALSE)) {
            /* 每次重启定时器之前清零,保证统计的时间 */
            osal_adapt_atomic_set(&(arp_req_process->rx_unicast_pkt_to_lan), 0);

            frw_create_timer_entry(&(arp_req_process->delba_opt_timer),
                hmac_btcoex_delba_send_timeout,
                BTCOEX_ARP_PROBE_TIMEOUT,
                hmac_user,
                OAL_FALSE);
        }
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_arp_fail_delba_process
 功能描述  : 判断bsle是否开启，选择是否执行
 输入参数  : 无
 输出参数  : hmac_btcoex_arp_fail_delba_process_etc返回值
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_btcoex_arp_fail_delba_process(oal_netbuf_stru **pst_netbuf,
    hmac_vap_stru *hmac_vap)
{
    hal_btcoex_btble_status_stru *status = hal_btcoex_btble_status();
    if (status->bt_status.bt_status.bt_on == 0) {
        return OAL_CONTINUE;
    }
    return hmac_btcoex_arp_fail_delba_process_etc(pst_netbuf, hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_config_print_btcoex_status_etc
 功能描述  : 打印共存维测信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_btcoex_check_by_ba_size_etc(hmac_user_stru *hmac_user)
{
    hmac_user_btcoex_stru *hmac_user_btcoex = hmac_btcoex_get_user_info(hmac_user);
    if ((hmac_user_btcoex->ba_size > 0) && (hmac_user_btcoex->ba_size < WLAN_AMPDU_RX_BA_LUT_WSIZE)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_btcoex_tx_addba_rsp_check
 功能描述  : 判断一个帧是否是addba_rsp
*****************************************************************************/
osal_void hmac_btcoex_tx_addba_rsp_check(const oal_netbuf_stru *netbuf, hmac_user_stru *hmac_user)
{
    hmac_user_btcoex_delba_stru *btcoex_delba = OSAL_NULL;
    const osal_u8 *mac_header = oal_netbuf_header_const(netbuf);
    const osal_u8 *mac_payload = oal_netbuf_tx_data_const(netbuf);

    if (mac_payload == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_COEX, "{hmac_btcoex_tx_addba_rsp_check::mac_payload is null!}");
        return;
    }

    /* Management frame */
    if (mac_get_frame_type_and_subtype(mac_header) == (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        if ((mac_payload[0] == MAC_ACTION_CATEGORY_BA) && (mac_payload[1] == MAC_BA_ACTION_ADDBA_RSP)) {
            btcoex_delba = &(hmac_btcoex_get_user_info(hmac_user)->hmac_user_btcoex_delba);
            btcoex_delba->get_addba_req_flag = OSAL_TRUE;

            if (btcoex_delba->delba_trigger == OSAL_TRUE) {
                btcoex_delba->ba_size_real_index = btcoex_delba->ba_size_expect_index;
            } else {
                btcoex_delba->ba_size_real_index = BTCOEX_RX_WINDOW_SIZE_INDEX_3;
            }
        }
    }
}

OSAL_STATIC osal_void hmac_btcoex_adjust_addba_rsp_basize(hmac_vap_stru *hmac_vap, hmac_ba_rx_stru *addba_rsp)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    hmac_user_btcoex_stru *hmac_user_btcoex;

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, addba_rsp->transmit_addr);
    if ((hmac_user == OAL_PTR_NULL) || (mac_btcoex_check_valid_sta(hmac_vap) != OAL_TRUE)) {
        return;
    }

    hmac_user_btcoex = hmac_btcoex_get_user_info(hmac_user);
    /* 1.黑名单用户 */
    if (hmac_user_btcoex->hmac_btcoex_addba_req.ba_handle_allow == OAL_FALSE) {
        if (hmac_btcoex_get_blacklist_type(hmac_user) == BTCOEX_BLACKLIST_TPYE_FIX_BASIZE) {
            addba_rsp->ba_resp_buf_size = (osal_u16)BTCOEX_BLACKLIST_BA_SIZE_LIMIT;
        }
    } else if ((hmac_user_btcoex->delba_btcoex_trigger == OAL_TRUE) && (hmac_user_btcoex->ba_size != 0)) {
        addba_rsp->ba_resp_buf_size = (osal_u8)hmac_user_btcoex->ba_size;
    }
}

hmac_netbuf_hook_stru g_btcoex_netbuf_hook = {
    .hooknum = HMAC_FRAME_DATA_TX_BRIDGE_IN,
    .priority = HMAC_HOOK_PRI_LOW,
    .hook_func = hmac_btcoex_arp_fail_delba_process,
};

osal_u32 hmac_btcoex_ba_init(osal_void)
{
    osal_u32 ret;
    /* 转发接口注册 */
    ret = hmac_register_netbuf_hook(&g_btcoex_netbuf_hook);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_RX,
        "{hmac_btcoex_arp_fail_delba_process:: MGMT RX IN register_netbuf_hooks error!");
        return ret;
    }
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_BA_ADDBA, hmac_btcoex_adjust_addba_rsp_basize);
    hmac_feature_hook_register(HMAC_FHOOK_BTCOEX_BA_CHECK, hmac_btcoex_check_by_ba_size_etc);
    return OAL_SUCC;
}

osal_void hmac_btcoex_ba_deinit(osal_void)
{
    /* 转发接口去注册 */
    hmac_unregister_netbuf_hook(&g_btcoex_netbuf_hook);
    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_BA_ADDBA);
    hmac_feature_hook_unregister(HMAC_FHOOK_BTCOEX_BA_CHECK);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
