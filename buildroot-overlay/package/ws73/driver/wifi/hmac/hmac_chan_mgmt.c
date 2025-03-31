/*
 * Copyright (c) CompanyNameMagicTag. 2014-2024. All rights reserved.
 * 文 件 名   : hmac_chan_mgmt.c
 * 作    者   : huanghe
 * 生成日期   : 2014年2月22日
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "hmac_main.h"
#include "hmac_alg_notify.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_scan.h"
#include "hmac_beacon.h"
#include "hmac_mgmt_ap.h"
#include "hmac_psm_sta.h"
#include "hmac_chan_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#if defined (_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined (_PRE_WLAN_FEATURE_DFS_ENABLE)
#include "hmac_radar.h"
#endif
#include "hmac_tx_mgmt.h"
#include "hmac_power.h"

#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_device_ext.h"
#include "hmac_mgmt_sta.h"
#include "hmac_sme_sta.h"
#include "hmac_fsm.h"
#include "hmac_dfs.h"
#include "mac_device_ext.h"
#include "hmac_scan.h"
#include "frw_ext_if.h"
#include "hmac_resource.h"
#include "wlan_msg.h"
#include "hmac_btcoex_ps.h"
#include "hmac_chan_mgmt.h"
#include "hmac_dfr.h"
#include "hmac_csa_ap.h"
#include "hmac_feature_interface.h"
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_ap.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CHAN_MGMT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

#ifdef _PRE_WIFI_DEBUG
static osal_void hmac_dump_chan(const hmac_vap_stru *hmac_vap, const osal_u8 *param);
#endif
static osal_s32 hmac_chan_switch_to_new_chan_complete_etc(hmac_vap_stru *hmac_vap, hmac_set_chan_stru *set_chan);

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_chan_update_user_bandwidth
 功能描述  : 更新ap下所有user的带宽
*****************************************************************************/
osal_void hmac_chan_update_user_bandwidth(hmac_vap_stru *hmac_vap)
{
    wlan_bw_cap_enum_uint8 bwcap_ap;
    wlan_bw_cap_enum_uint8 bandwidth_cap;
    hmac_user_stru *hmac_user = OSAL_NULL;
    wlan_bw_cap_enum_uint8 bwcap_min;
    struct osal_list_head *entry = OSAL_NULL;
    osal_u8 old_avail_bandwidth, old_cur_bandwidth;

    hmac_vap_get_bandwidth_cap_etc(hmac_vap, &bwcap_ap);

    osal_list_for_each(entry, &(hmac_vap->mac_user_list_head))
    {
        hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (osal_unlikely(hmac_user == OSAL_NULL)) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_chan_update_user_bandwidth::user null.}", hmac_vap->vap_id);
            continue;
        }

        if (hmac_user->vap_id == hmac_vap->vap_id) {
            old_avail_bandwidth = hmac_user->avail_bandwidth;
            old_cur_bandwidth   = hmac_user->cur_bandwidth;
            hmac_user_get_sta_cap_bandwidth_etc(hmac_user, &bandwidth_cap);
            bwcap_min = osal_min(bwcap_ap, bandwidth_cap);
            hmac_user_set_bandwidth_info_etc(hmac_user, bwcap_min, bwcap_min);

            oam_warning_log4(0, OAM_SF_2040,
                "hmac_chan_update_user_bandwidth::update assoc_id=[%d] bw_cap=[%d] avail_bw=[%d] cur_bw=[%d].",
                hmac_user->assoc_id, bandwidth_cap, hmac_user->avail_bandwidth, hmac_user->cur_bandwidth);
            if (old_avail_bandwidth != hmac_user->avail_bandwidth || old_cur_bandwidth != hmac_user->cur_bandwidth) {
                /* 带宽更新 则同步至device */
                hmac_user_sync(hmac_user);
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_dfs_update_alg_anti_intf_switch
 功能描述  : DFS信道时，更新弱干扰免疫算法的开关
*****************************************************************************/
osal_void hmac_chan_dfs_update_alg_anti_intf_switch(const hal_to_dmac_device_stru *hal_device,
    const mac_channel_stru *channel)
{
#ifdef _PRE_WLAN_FEATURE_DFS_ENABLE
    oal_bool_enum_uint8 is_nonradar_chan = OSAL_TRUE;
    oal_bool_enum_uint8 switch_nonradar_chan = OSAL_TRUE;
#endif

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chan_dfs_update_alg_anti_intf_switch::hal_device NULL}");
        return;
    }
    if (channel == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chan_dfs_update_alg_anti_intf_switch::channel NULL}");
        return;
    }
#ifdef _PRE_WLAN_FEATURE_DFS_ENABLE
    if (hmac_is_cover_dfs_channel(hal_device->wifi_channel_status.band, hal_device->wifi_channel_status.en_bandwidth,
        hal_device->current_chan_number) == OSAL_TRUE) {
        is_nonradar_chan = OSAL_FALSE;
    }
    if (hmac_is_cover_dfs_channel(channel->band, channel->en_bandwidth, channel->chan_number) == OSAL_TRUE) {
        switch_nonradar_chan = OSAL_FALSE;
    }
#endif
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_switch_channel
 功能描述  : 切换信道和带宽，都需要开关pa
                              切换信道只需关闭mac/phy, 不需要挂起发送
*****************************************************************************/
osal_u32 hmac_mgmt_switch_channel(hal_to_dmac_device_stru *hal_device, mac_channel_stru *channel,
    osal_bool clear_fifo)
{
    frw_msg  msg_info;
    osal_s32 ret;
    mac_switch_channel_stru mac_switch_channel;

    /* 此路为空不执行 */
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_switch_channel::hal_device NULL}");
        return OAL_FAIL;
    }

    hal_device->wifi_channel_status.band = channel->band;
    hal_device->wifi_channel_status.chan_idx = channel->chan_idx;
    hal_device->wifi_channel_status.chan_number = channel->chan_number;
    hal_device->wifi_channel_status.en_bandwidth = channel->en_bandwidth;

    mac_switch_channel.clear_fifo = clear_fifo;
    mac_switch_channel.mac_channel = hal_device->wifi_channel_status;

    /* 在device侧完成信道切换 */
    frw_msg_init((osal_u8 *)&mac_switch_channel, sizeof(mac_switch_channel_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_SWITCH_CHANNEL, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hmac_mgmt_switch_channel:send switch channel to device fail[%d]", ret);
    }

    /* 切换完成 */
    hal_device->current_chan_number = channel->chan_number;
    /* hal_set_primary_channel 移植到devcie tpc等未移植的保留此函数 */
    /* hal_set_freq_band 移植到devcie ftm未移植保留此函数 */
    hal_tpc_cali_ftm_updata_channel(hal_device, channel);
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_m2s_spec_update_hal_device_capbility(hal_device, channel->band);
#endif

    /* DFS信道时，更新弱干扰免疫算法的开关 */
    hmac_chan_dfs_update_alg_anti_intf_switch(hal_device, channel);
    return OAL_SUCC;
}
static osal_void hmac_channel_bw_handle(hmac_vap_stru *hmac_vap, const hmac_device_stru *hmac_device,
    mac_channel_stru *channe_param, hal_to_dmac_device_stru *hal_device)
{
    osal_u32 ret;
    /* 刷新发送功率 */
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_INIT);

    /* 通知算法信道改变 */
    hmac_alg_cfg_channel_notify(hmac_vap, CH_BW_CHG_TYPE_MOVE_WORK);

    /* 选择需要设置的信道信息 */
    hmac_chan_select_real_channel(hmac_device, channe_param, hmac_vap->channel.chan_number);

    /* 切换信道不需要清fifo，传入FALSE */
    ret = hmac_mgmt_switch_channel(hal_device, channe_param, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_channel_bw_handle::hmac_mgmt_switch_channel() fail! ret = [%d].}",
            ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_select_channel_mac
 功能描述  : 设置信道和带宽，使VAP工作在新信道上
                             建议该接口只设置带宽，信道和带宽同时修改采用
                             另一个multi接口，51仍需要使用此接口用于配置信道清fifo
*****************************************************************************/
osal_void hmac_chan_select_channel_mac(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 idx, old_chan_number, old_bandwidth;
    osal_u32 ret;
    mac_channel_stru channe_param;
    osal_bool is_support_bw;
    wlan_channel_bandwidth_enum_uint8 chan_bandwidth = bandwidth;

    if (osal_unlikely(hmac_vap == OSAL_NULL) || osal_unlikely(hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chan_select_channel_mac::hmac_vap or device null.}");
        return;
    }

    hal_device = hmac_vap->hal_device;

    /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
    ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, channel, &idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, 0, "{hmac_chan_select_channel_mac:get_channel_idx failed[%d]}", ret);
        return;
    }

    is_support_bw = hmac_regdomain_channel_is_support_bw(chan_bandwidth, channel);
    if (is_support_bw == OSAL_FALSE) {
        oam_warning_log3(0, 0, "vap_id[%d] {hmac_chan_select_channel_mac::ch[%d] is not support bw[%d],set BW 20M}",
            hmac_vap->vap_id, channel, chan_bandwidth);
        chan_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    old_chan_number = hmac_vap->channel.chan_number;
    old_bandwidth = hmac_vap->channel.en_bandwidth;
    hmac_vap->channel.chan_number = channel;
    hmac_vap->channel.en_bandwidth = chan_bandwidth;
    hmac_vap->channel.chan_idx = idx;
    channe_param = hmac_vap->channel;

    if (old_chan_number != hmac_vap->channel.chan_number || old_bandwidth != hmac_vap->channel.en_bandwidth) {
        hmac_vap_sync(hmac_vap);
    }

#ifdef _PRE_WLAN_FEATURE_DBAC
    if (mac_is_dbac_running(hmac_device)) {
        hmac_alg_update_dbac_fcs_config(hmac_vap);
        return;
    }
#endif

    if (is_ap(hmac_vap)) {
        hmac_chan_update_user_bandwidth(hmac_vap);
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 使能去使能雷达检测 */
    if (is_ap(hmac_vap) && (mac_vap_get_dfs_enable(hmac_vap) == OSAL_TRUE)) {
        hmac_dfs_radar_detect_check(hal_device, hmac_device, hmac_vap);
    }
#endif

    hmac_channel_bw_handle(hmac_vap, hmac_device, &channe_param, hal_device);
}

/*****************************************************************************
 函 数 名  : hmac_chan_select_chan_event_process
 功能描述  : 处理从HMAC模块过来的信道设置请求事件
*****************************************************************************/
OAL_STATIC osal_s32 hmac_chan_sync(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_set_chan_stru *set_chan = OSAL_NULL;
    oal_bool_enum_uint8 teb;

    set_chan = (hmac_set_chan_stru *)msg->data;
    teb = hmac_vap->ch_switch_info.te_b;
    (osal_void)memcpy_s(&hmac_vap->channel, sizeof(mac_channel_stru), &set_chan->channel, sizeof(mac_channel_stru));
    (osal_void)memcpy_s(&hmac_vap->ch_switch_info, sizeof(mac_ch_switch_info_stru),
        &set_chan->ch_switch_info, sizeof(mac_ch_switch_info_stru));
    hmac_vap->ch_switch_info.te_b = teb;

    mac_mib_set_forty_mhz_intolerant(hmac_vap, set_chan->dot11_40mhz_intolerant);
#ifdef _PRE_WIFI_DEBUG
    hmac_dump_chan(hmac_vap, (osal_u8 *)set_chan);
#endif

    if (set_chan->switch_immediately == OSAL_TRUE) {
        hmac_chan_multi_select_channel_mac(hmac_vap, set_chan->channel.chan_number, set_chan->channel.en_bandwidth);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_DFS
/*****************************************************************************
 函 数 名  : hmac_chan_attempt_new_chan
 功能描述  : 设置VAP转移置新信道工作
*****************************************************************************/
osal_void hmac_chan_attempt_new_chan(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    oal_bool_enum_uint8 check_cac;

    /* 选择20/40/80MHz信道 */
    // 需要考虑dbac场景下面的处理
    hmac_chan_multi_select_channel_mac(hmac_vap, channel, bandwidth);

    (osal_void)memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN,
                        mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);

    if (hmac_is_cover_dfs_channel(hmac_vap->channel.band, bandwidth, channel) == OSAL_TRUE) {
        check_cac = OSAL_TRUE;
        /* vap状态pause->pause */
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_PAUSE);
    } else {
        check_cac = OSAL_FALSE;
        /* 在新信道上恢复硬件的发送 */
        hal_set_machw_tx_resume();
        /* 在新信道上恢复Beacon帧的发送 */
        hal_vap_beacon_resume(hmac_vap->hal_vap);
        /* vap状态pause->up */
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);
    }

    /* 上报信道切换完成事件 hmac判断是否需要CAC检测 */
    hmac_switch_complete_notify(hmac_vap, check_cac);
}
#else
osal_void hmac_chan_attempt_new_chan(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    /* 选择20/40/80MHz信道 */
    // 需要考虑dbac场景下面的处理
    hmac_chan_multi_select_channel_mac(hmac_vap, channel, bandwidth);

    (osal_void)memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN,
                        mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);

    /* 在新信道上恢复Beacon帧的发送 */
    hal_vap_beacon_resume(hmac_vap->hal_vap);
    /* 在新信道上恢复硬件的发送 */
    hal_set_machw_tx_resume();

    /* vap状态pause->up */
    hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);

    hmac_switch_complete_notify(hmac_vap, OSAL_FALSE);
}

#endif /* end of _PRE_WLAN_FEATURE_DFS */

#ifdef _PRE_WIFI_DEBUG
static osal_void hmac_dump_chan(const hmac_vap_stru *hmac_vap, const osal_u8 *param)
{
    hmac_set_chan_stru *chan = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;
    unref_param(vap_id);
    chan = (hmac_set_chan_stru*)param;
    oam_info_log4(0, OAM_SF_2040,
                  "chan_number=%d band=%d bandwidth=%d idx=%d\r\n",
                  chan->channel.chan_number, chan->channel.band,
                  chan->channel.en_bandwidth, chan->channel.chan_idx);

    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] bw_switch_status=%d", vap_id, chan->ch_switch_info.bw_switch_status);

    oam_info_log4(0, OAM_SF_2040,
                  "te_b=%d chan_report_for_te_a=%d switch_immediately=%d check_cac=%d\r\n",
                  chan->ch_switch_info.te_b, chan->ch_switch_info.chan_report_for_te_a,
                  chan->switch_immediately, chan->check_cac);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_chan_select_real_channel
 功能描述  : 查找device所有up vap主信道, 如果主信道都相同, 则根据最大带宽设置硬件寄存器
*****************************************************************************/
osal_void hmac_chan_select_real_channel(const hmac_device_stru  *hmac_device,
    mac_channel_stru *channel, osal_u8 dst_chan_num)
{
    osal_u8                vap_idx;
    hmac_vap_stru            *hmac_vap = OSAL_NULL;
    oal_bool_enum_uint8      all_same_channel = OSAL_TRUE;
    oal_bool_enum_uint8      found_channel = OSAL_FALSE;
    mac_channel_stru         real_channel;
    wlan_channel_bandwidth_enum_uint8 max_en_bandwidth = WLAN_BAND_WIDTH_20M;

    if ((channel == OSAL_NULL) || (hmac_device == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_FRAME_FILTER,
            "{hmac_chan_select_real_channel has null point:channel=%p hmac_device=%p}",
            (uintptr_t)channel, (uintptr_t)hmac_device);
        return;
    }

    (osal_void)memset_s(&real_channel, sizeof(mac_channel_stru), 0, sizeof(mac_channel_stru));

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap is null, vap id is %d", hmac_device->vap_id[vap_idx]);
            continue;
        }

        if ((hmac_vap->vap_state == MAC_VAP_STATE_UP) ||
#ifdef _PRE_WLAN_FEATURE_DBAC
            ((mac_is_dbac_running(hmac_device) && hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA)) ||
#endif
            (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) ||
#ifdef _PRE_WLAN_FEATURE_ROAM
            (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) ||
#endif // _PRE_WLAN_FEATURE_ROAM
            (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN && hmac_vap->user_nums > 0) ||
            (hmac_vap->vap_state == MAC_VAP_STATE_AP_WAIT_START)) {
            if (hmac_vap->channel.chan_number != dst_chan_num) {
                all_same_channel = OSAL_FALSE;
                break;
            }

            if ((hmac_vap->channel.en_bandwidth >= max_en_bandwidth) &&
                (hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT)) {
                real_channel = hmac_vap->channel;
                found_channel = OSAL_TRUE;
                max_en_bandwidth = hmac_vap->channel.en_bandwidth;
            }
        }
    }

    if ((all_same_channel == OSAL_TRUE) && (found_channel == OSAL_TRUE)) {
        *channel = real_channel;
    }
}

static osal_void hmac_chan_restart_network_after_switch(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    if (hmac_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY,
                         "{hmac_chan_restart_network_after_switch:: hmac_device is null!!!}");
    }
    /* 在新信道上恢复Beacon帧的发送 */
    hal_vap_beacon_resume(hmac_vap->hal_vap);

    /* 在新信道上恢复硬件的发送 */
    hal_set_machw_tx_resume();
    /* vap状态pause->up */
    hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);
}

OAL_STATIC osal_s32 hmac_chan_restart_network_after_switch_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    unref_param(msg);
    hmac_chan_restart_network_after_switch(hmac_device, hmac_vap);
    return OAL_SUCC;
}

osal_void  hmac_switch_complete_notify(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 check_cac)
{
    osal_s32            ret;
    hmac_set_chan_stru  set_chan;
    (osal_void)memset_s(&set_chan, sizeof(set_chan), 0, sizeof(set_chan));

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
    /* 带宽已成功切换到40M, 停止40M恢复定时器 */
    if ((hmac_vap->channel.en_bandwidth > WLAN_BAND_WIDTH_20M) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        hmac_chan_stop_40m_recovery_timer(hmac_vap);
    }
#endif

    (osal_void)memcpy_s(&set_chan.channel, sizeof(mac_channel_stru), &hmac_vap->channel, sizeof(mac_channel_stru));
    (osal_void)memcpy_s(&set_chan.ch_switch_info, sizeof(mac_ch_switch_info_stru), &hmac_vap->ch_switch_info,
        sizeof(mac_ch_switch_info_stru));
    set_chan.check_cac = check_cac;

#ifdef _PRE_WIFI_DEBUG
    hmac_dump_chan(hmac_vap, (osal_u8*)&set_chan);
#endif

    ret = hmac_chan_switch_to_new_chan_complete_etc(hmac_vap, &set_chan);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_switch_complete_notify::return failed[%d].}", hmac_vap->vap_id, ret);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_chan_disable_machw_tx
 功能描述  : 禁止硬件发送(数据帧、ACK、RTS)
*****************************************************************************/
osal_void  hmac_chan_disable_machw_tx(const hmac_vap_stru *hmac_vap)
{
    hal_to_dmac_device_stru        *hal_device;

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_chan_disable_machw_tx::hal_device null}", hmac_vap->vap_id);
        return;
    }

    /* 禁止硬件发送 */
    hal_set_machw_tx_suspend();

    /* 禁止硬件回ack */
    hal_disable_machw_ack_trans();

    /* 禁止硬件回cts */
    hal_disable_machw_cts_trans();

    oam_warning_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_chan_disable_machw_tx::tx disabled.}", hmac_vap->vap_id);
}

/*****************************************************************************
 功能描述  : 恢复硬件发送(数据帧、ACK、RTS)
*****************************************************************************/
osal_void  hmac_chan_enable_machw_tx(const hmac_vap_stru *hmac_vap)
{
    /* 恢复硬件发送 */
    hal_set_machw_tx_resume();
    /* 恢复硬件回ack */
    hal_enable_machw_ack_trans();
    /* 恢复硬件回cts */
    hal_enable_machw_cts_trans();
    oam_warning_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_chan_enable_machw_tx::tx enabled.}", hmac_vap->vap_id);
}

#ifdef _PRE_WIFI_DEBUG
OAL_STATIC osal_u32 hmac_dump_chan_etc(hmac_vap_stru *hmac_vap, osal_u8 *param)
{
    hmac_set_chan_stru *chan;

    if ((hmac_vap == OAL_PTR_NULL) || (param == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    chan = (hmac_set_chan_stru *)param;
    oam_info_log1(0, OAM_SF_2040, "vap_id[%d] channel mgmt info\r\n", hmac_vap->vap_id);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] chan_number=%d\r\n", hmac_vap->vap_id,
        chan->channel.chan_number);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] band=%d\r\n", hmac_vap->vap_id, chan->channel.band);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] en_bandwidth=%d\r\n", hmac_vap->vap_id,
        chan->channel.en_bandwidth);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] idx=%d\r\n", hmac_vap->vap_id, chan->channel.chan_idx);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] bw_switch_status=%d\r\n",
        hmac_vap->vap_id, chan->ch_switch_info.bw_switch_status);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] user_pref_bandwidth=%d\r\n",
        hmac_vap->vap_id, chan->ch_switch_info.user_pref_bandwidth);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] te_b=%d\r\n",
        hmac_vap->vap_id, chan->ch_switch_info.te_b);
    oam_info_log2(0, OAM_SF_2040, "vap_id[%d] chan_report_for_te_a=%d\r\n",
        hmac_vap->vap_id, chan->ch_switch_info.chan_report_for_te_a);
    return OAL_SUCC;
}
#endif

#if defined(_PRE_PRODUCT_ID_HOST)
OAL_STATIC osal_u32 hmac_check_ap_channel_follow_sta_check_para(const hmac_vap_stru *check_mac_vap,
    const mac_channel_stru *set_mac_channel, hmac_device_stru **hmac_device)
{
    if (set_mac_channel == OAL_PTR_NULL || check_mac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_check_ap_channel_follow_sta_check_para:: input param is null,return}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!is_legacy_vap(check_mac_vap)) {
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_check_ap_channel_follow_sta_check_para:: check_vap_p2p_mode=%d, not need to check}",
            check_mac_vap->vap_id, check_mac_vap->p2p_mode);
        return OAL_FAIL;
    }

    if (!is_sta(check_mac_vap) && !is_ap(check_mac_vap)) {
        oam_error_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_check_ap_channel_follow_sta_check_para::vap_mode=%d,not need to check}",
            check_mac_vap->vap_id, check_mac_vap->vap_mode);
        return OAL_FAIL;
    }

    *hmac_device = hmac_res_get_mac_dev_etc(check_mac_vap->device_id);
    if (*hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_2040,
            "{hmac_check_ap_channel_follow_sta_check_para::get mac_device(%d) is null. Return}",
            check_mac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log4(0, OAM_SF_2040,
        "vap_id[%d] {hmac_check_ap_channel_follow_sta_check_para::vap_state=%d, vap_band=%d vap_channel=%d.}",
        check_mac_vap->vap_id,
        check_mac_vap->vap_state, set_mac_channel->band, set_mac_channel->chan_number);

    return OAL_CONTINUE;
}

/* 2.4G 信道频段相同，40M 若扩频方向不同，以sta的扩频方向为主 */
static osal_u32 hmac_check_follow_sta_bandwitch(const hmac_vap_stru *check_mac_vap,
    mac_channel_stru *set_mac_channel, hmac_vap_stru *index_mac_vap, hmac_device_stru *mac_device)
{
    osal_u8 set_bandwidth;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_SWITCH_NEW_CHANNEL);
    unref_param(mac_device);
    /* 非2.4G 不存在扩频方向的差异 */
    if (set_mac_channel->band != WLAN_BAND_2G) {
        return OAL_CONTINUE;
    }

    /* 如果带宽扩展方向相同，则无需处理 */
    if (set_mac_channel->en_bandwidth == index_mac_vap->channel.en_bandwidth) {
        oam_warning_log1(0, OAM_SF_2040, "hmac_check_follow_sta_bandwitch:: same bandwidth[%d]\n",
            set_mac_channel->en_bandwidth);
        return OAL_CONTINUE;
    }

    /* 20M 不存在扩频方向的问题 */
    if (set_mac_channel->en_bandwidth == WLAN_BAND_WIDTH_20M ||
        index_mac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_20M) {
        oam_warning_log0(0, OAM_SF_2040, "hmac_check_follow_sta_bandwitch:: 20M not check\n");
        return OAL_CONTINUE;
    }

    if (is_sta(check_mac_vap) && is_ap(index_mac_vap)) {
        /* AP先启动;STA后启动 */
        /* CSA */
        if (fhook != OSAL_NULL) {
            ((hmac_csa_ap_switch_new_channel_cb)fhook)(index_mac_vap, set_mac_channel->chan_number,
                set_mac_channel->en_bandwidth, OSAL_FALSE, HMAC_CHANNEL_SWITCH_COUNT);
        }
        return OAL_SUCC;
    } else if (is_ap(check_mac_vap) && is_sta(index_mac_vap)) {
        /* STA先启动;AP后启动 */
        oam_warning_log2(0, OAM_SF_2040,
            "{<vap_current_mode=Ap vap_index_mode=Sta> SoftAp change bindwitch from [%d] To [%d]}.\n",
            set_mac_channel->en_bandwidth, index_mac_vap->channel.en_bandwidth);
        /* 替换频宽值 */
        set_bandwidth = index_mac_vap->channel.en_bandwidth; /* 此时bandwitch使用 sta的 */
        set_mac_channel->en_bandwidth = set_bandwidth;
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}

static osal_u32 hmac_check_ap_channel_follow_sta_etc(const hmac_vap_stru *check_mac_vap,
    mac_channel_stru *set_mac_channel, hmac_vap_stru *index_mac_vap, hmac_device_stru *mac_device)
{
    osal_u8 set_bandwidth;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_SWITCH_NEW_CHANNEL);
    unref_param(mac_device);
    if (is_sta(check_mac_vap) && is_ap(index_mac_vap)) {
        /* AP先启动;STA后启动 */
        /* CSA */
        if (fhook != OSAL_NULL) {
            ((hmac_csa_ap_switch_new_channel_cb)fhook)(index_mac_vap, set_mac_channel->chan_number,
                index_mac_vap->channel.en_bandwidth, OSAL_FALSE, HMAC_CHANNEL_SWITCH_COUNT);
        }
        // dfr模块记录信道信息,用于自愈
        hmac_dfr_record_ap_follow_channel(set_mac_channel->chan_number, index_mac_vap->channel.en_bandwidth);
        return OAL_SUCC;
    } else if (is_ap(check_mac_vap) && is_sta(index_mac_vap)) {
        /* STA先启动;AP后启动 */
        oam_warning_log2(0, OAM_SF_2040,
            "{<vap_current_mode=Ap vap_index_mode=Sta> SoftAp change Channel from [%d] To [%d]}.\n",
            set_mac_channel->chan_number, index_mac_vap->channel.chan_number);
        /* 替换信道值 */
        set_mac_channel->chan_number = index_mac_vap->channel.chan_number;
        set_bandwidth = set_mac_channel->en_bandwidth;
        set_mac_channel->en_bandwidth = set_bandwidth;
        // dfr模块记录信道信息,用于自愈
        hmac_dfr_record_ap_follow_channel(set_mac_channel->chan_number, index_mac_vap->channel.en_bandwidth);
        return OAL_SUCC;
    }
    return OAL_CONTINUE;
}


/*****************************************************************************
 函 数 名  : hmac_check_ap_channel_follow_sta
 功能描述  : 检查ap的信道是否需要跟随sta
 输入参数  :
        vap_id : 触发该检查操作的vapid
        set_mac_channel  : vap_id 的设置参数
 输出参数  : 无
 返 回 值  :
    OAL_SUCC : softap需要跟随sta的信道变化
*****************************************************************************/
osal_u32 hmac_check_ap_channel_follow_sta(const hmac_vap_stru *check_mac_vap,
    mac_channel_stru *set_mac_ch)
{
    osal_u32         ret;
    hmac_device_stru   *hmac_device = OAL_PTR_NULL;
    osal_u8          vap_idx;
    hmac_vap_stru      *idx_mac_vap = OAL_PTR_NULL;

    ret = hmac_check_ap_channel_follow_sta_check_para(check_mac_vap, set_mac_ch, &hmac_device);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        idx_mac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if ((idx_mac_vap == OAL_PTR_NULL) || (check_mac_vap->vap_id == idx_mac_vap->vap_id) ||
            (idx_mac_vap->p2p_mode != WLAN_LEGACY_VAP_MODE)) {
            continue;
        }
        if ((idx_mac_vap->vap_state != MAC_VAP_STATE_UP) && (idx_mac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }
        if (set_mac_ch->band != idx_mac_vap->channel.band) {
            continue;
        }

        /* 信道频段相同，还需要检测扩频方向是否相同，若相同才continue */
        if (set_mac_ch->chan_number == idx_mac_vap->channel.chan_number) {
            if (hmac_check_follow_sta_bandwitch(check_mac_vap, set_mac_ch,
                                                idx_mac_vap, hmac_device) != OAL_CONTINUE) {
                return OAL_SUCC;
            }
            continue;
        }

        oam_warning_log4(0, OAM_SF_2040,
            "{hmac_check_ap_channel_follow_sta::vap_state=%d,mode=%d,band=%d,channel=%d.}", idx_mac_vap->vap_state,
                check_mac_vap->vap_mode, idx_mac_vap->channel.band, idx_mac_vap->channel.chan_number);

        if (hmac_check_ap_channel_follow_sta_etc(check_mac_vap, set_mac_ch,
            idx_mac_vap, hmac_device) != OAL_CONTINUE) {
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_chan_multi_switch_to_new_channel_etc
 功能描述  : 遍历device下所有ap，设置VAP信道参数，准备切换至新信道运行
 输入参数  : hmac_vap : MAC VAP结构体指针
             channel  : 新信道号(准备切换到的20MHz主信道号)
             en_bandwidth: 新带宽模式
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFS
osal_void  hmac_chan_multi_switch_to_new_channel_etc(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    osal_u8          vap_idx;
    hmac_device_stru   *hmac_device;
    hmac_vap_stru      *hmac_vap_tmp;
    osal_u8          chan_idx;
    osal_u32         ul_ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_SWITCH_NEW_CHANNEL);

    oam_info_log3(0, OAM_SF_2040,
        "vap_id[%d] {hmac_chan_multi_switch_to_new_channel_etc::channel=%d,en_bandwidth=%d}", hmac_vap->vap_id,
        channel, en_bandwidth);

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_multi_switch_to_new_channel_etc::device(%d) null.}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return;
    }

    if (hmac_device->vap_num == 0) {
        oam_error_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_multi_switch_to_new_channel_etc::none vap.}",
            hmac_vap->vap_id);
        return;
    }

    ul_ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, channel, &chan_idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_multi_switch_to_new_channel_etc::hmac_get_channel_idx_from_num_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        return;
    }

    /* 遍历device下所有ap，设置ap信道参数，发送CSA帧，准备切换至新信道运行 */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL) {
            oam_error_log2(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_multi_switch_to_new_channel_etc::ap(%d) null.}",
                hmac_vap->vap_id, hmac_device->vap_id[vap_idx]);
            continue;
        }
        if (hmac_vap_tmp->vap_mode != WLAN_VAP_MODE_BSS_AP) {
            continue;
        }

        /* 只有running AP需要发送CSA帧 */
        if (hmac_vap_tmp->vap_state == MAC_VAP_STATE_UP) {
            /* csa处理过程不跟随带宽变化 */
            if (fhook != OSAL_NULL) {
                (hmac_csa_ap_switch_new_channel_cb)fhook)(hmac_vap_tmp, channel, en_bandwidth, OSAL_TRUE,
                    WLAN_CHAN_SWITCH_DETECT_RADAR_CNT);
            }
        } else { /* 其它站点只需要更新信道信息 */
            /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
            hmac_vap_tmp->channel.chan_number    = channel;
            hmac_vap_tmp->channel.chan_idx       = chan_idx;
            hmac_vap_tmp->channel.en_bandwidth   = en_bandwidth;
        }
    }
}
#endif /* end of _PRE_WLAN_FEATURE_DFS */

OAL_STATIC osal_void  hmac_chan_sync_init_etc(hmac_vap_stru *hmac_vap, hmac_set_chan_stru *set_chan)
{
    memset_s(set_chan, OAL_SIZEOF(hmac_set_chan_stru), 0, OAL_SIZEOF(hmac_set_chan_stru));
    if (memcpy_s(&set_chan->channel, sizeof(set_chan->channel),
        &hmac_vap->channel, OAL_SIZEOF(mac_channel_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chan_sync_init_etc::memcpy_s channel error}");
    }
    if (memcpy_s(&set_chan->ch_switch_info, sizeof(set_chan->ch_switch_info), &hmac_vap->ch_switch_info,
        OAL_SIZEOF(mac_ch_switch_info_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_chan_sync_init_etc::memcpy_s ch_switch_info error}");
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_do_sync_etc
 功能描述  : HMAC模块抛事件到DMAC模块，设置SW/MAC/PHY/RF中的信道和带宽，
             使VAP工作在新信道上
 输入参数  : hmac_vap : hmac_vap_stru
             set_chan  : hmac_set_chan_stru
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_void  hmac_chan_do_sync_etc(hmac_vap_stru *hmac_vap, hmac_set_chan_stru *set_chan)
{
    osal_s32 ret1;
    osal_u32 ret;
    osal_u8 idx;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
    ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, set_chan->channel.chan_number,
        &idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d]{hmac_chan_do_sync_etc::hmac_get_channel_idx_from_num_etc fail[%d]}",
            hmac_vap->vap_id, ret);
        return;
    }

    hmac_vap->channel.chan_number    = set_chan->channel.chan_number;
    hmac_vap->channel.en_bandwidth   = set_chan->channel.en_bandwidth;
    hmac_vap->channel.chan_idx       = idx;

    cfg_msg_init((osal_u8 *)set_chan, OAL_SIZEOF(hmac_set_chan_stru), OAL_PTR_NULL, 0, &msg_info);
    ret1 = hmac_chan_sync(hmac_vap, &msg_info);
    if (ret1 != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_chan_sync_etc::return failed[%d].}",
            hmac_vap->vap_id, ret1);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_chan_sync_etc
 功能描述  : HMAC将信道/带宽信息同步到DMAC
 输入参数  : hmac_vap : MAC VAP结构体指针
             channel  : 将要被设置的信道号
             en_bandwidth: 将要被设置的带宽模式
             switch_immediately: DMAC侧收到同步事件之后是否立即切换
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/

osal_void hmac_chan_sync_etc(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth, oal_bool_enum_uint8 switch_immediately)
{
    hmac_set_chan_stru set_chan;

    hmac_chan_sync_init_etc(hmac_vap, &set_chan);
    set_chan.channel.chan_number = channel;
    set_chan.channel.en_bandwidth = en_bandwidth;
    set_chan.switch_immediately = switch_immediately;
    set_chan.dot11_40mhz_intolerant = mac_mib_get_forty_mhz_intolerant(hmac_vap);
    hmac_chan_do_sync_etc(hmac_vap, &set_chan);
}


/*****************************************************************************
 函 数 名  : hmac_chan_multi_select_channel_mac_etc
 功能描述  : 遍历device下所有VAP，设置SW/MAC/PHY/RF中的信道和带宽，使VAP工作在新信道上
 输入参数  : hmac_vap : MAC VAP结构体指针
             channel  : 将要被设置的信道号
             en_bandwidth: 将要被设置的带宽模式
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void  hmac_chan_multi_select_channel_mac_etc(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    osal_u8          vap_idx;
    hmac_device_stru   *hmac_device;
    hmac_vap_stru      *hmac_vap_tmp;

    oam_warning_log3(0, OAM_SF_2040,
        "vap_id[%d]{hmac_chan_multi_select_channel_mac_etc::channel=%d,en_bandwidth=%d}", hmac_vap->vap_id,
        channel, en_bandwidth);

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_multi_select_channel_mac_etc::device null,device_id=%d.}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return;
    }

    if (hmac_device->vap_num == 0) {
        oam_error_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_multi_select_channel_mac_etc::none vap.}",
            hmac_vap->vap_id);
        return;
    }

    if (mac_is_dbac_running(hmac_device)) {
        hmac_chan_sync_etc(hmac_vap, channel, en_bandwidth, OAL_TRUE);
        return;
    }

    /* 遍历device下所有vap， */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL) {
            oam_error_log2(0, OAM_SF_SCAN,
                "vap_id[%d] {hmac_chan_multi_select_channel_mac_etc::hmac_vap null,vap_id=%d.}", hmac_vap->vap_id,
                hmac_device->vap_id[vap_idx]);
            continue;
        }

        hmac_chan_sync_etc(hmac_vap_tmp, channel, en_bandwidth, OAL_TRUE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_get_user_pref_primary_ch
 功能描述  : 获取用户优先设置的信道
 输入参数  ：hmac_device: MAC DEVICE结构体指针
 输出参数  : 无
 返 回 值  : 用户优先设置的信道
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u8  hmac_chan_get_user_pref_primary_ch(hmac_device_stru *hmac_device)
{
    return hmac_device->max_channel;
}

/*****************************************************************************
 函 数 名  : hmac_get_user_pref_bandwidth
 功能描述  : 获取用户优先设置的带宽模式
 输入参数  : hmac_vap: MAC VAP结构体指针
 输出参数  : 无
 返 回 值  : 用户优先设置的带宽模式
*****************************************************************************/
OAL_STATIC OAL_INLINE wlan_channel_bandwidth_enum_uint8  hmac_chan_get_user_pref_bandwidth(hmac_vap_stru *hmac_vap)
{
    return hmac_vap->ch_switch_info.user_pref_bandwidth;
}

/*****************************************************************************
 函 数 名  : hmac_chan_check_chan_avail
 功能描述  : 自动信道选择有效性检查
 输入参数  : band         : 频段(0: 2.4GHz; 1: 5GHz)
 输出参数  : start_ch_idx: 需要扫描的第一条信道索引
             end_ch_idx  : 需要扫描的最后一条信道索引
 返 回 值  : OAL_TRUE : 成功
             OAL_FALSE: 失败
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8  hmac_chan_check_channnel_avaible(wlan_channel_band_enum_uint8 band,
    osal_u8 *start_ch_idx, osal_u8 *end_ch_idx)
{
    osal_s32    l_ch_idx;
    osal_u8    num_supp_chan = mac_get_num_supp_channel(band);
    osal_u32   ul_ret;

    /* 取低有效信道 */
    for (l_ch_idx = *start_ch_idx; l_ch_idx < num_supp_chan; l_ch_idx++) {
        ul_ret = hmac_is_channel_idx_valid_etc(band, (osal_u8)l_ch_idx);
        if (ul_ret == OAL_SUCC) {
            *start_ch_idx = (osal_u8)l_ch_idx;
            break;
        }
    }

    if (l_ch_idx == num_supp_chan) {
        return OAL_FALSE;
    }

    /* 取高有效信道 */
    for (l_ch_idx = *end_ch_idx; l_ch_idx >= 0; l_ch_idx--) {
        ul_ret = hmac_is_channel_idx_valid_etc(band, (osal_u8)l_ch_idx);
        if (ul_ret == OAL_SUCC) {
            *end_ch_idx = (osal_u8)l_ch_idx;
            break;
        }
    }

    if (l_ch_idx < 0) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_start_bss_in_available_channel_etc
 功能描述  : 在指定(可用)信道上启动BSS
 输入参数  : hmac_vap: HMAC VAP指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32  hmac_start_bss_in_available_channel_etc(hmac_vap_stru *hmac_vap)
{
    hmac_ap_start_rsp_stru          ap_start_rsp;
    osal_u32                      ul_ret;

    /* 解决beacon速率集未更新问题 */
    hmac_vap_init_rates_etc(hmac_vap);

    /* 设置AP侧状态机为 UP */
    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_UP);

    /* 调用hmac_config_start_vap_event，启动BSS */
    ul_ret = hmac_config_start_vap_event_etc(hmac_vap, OAL_TRUE);
    if (osal_unlikely(ul_ret != OAL_SUCC)) {
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_INIT);
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_start_bss_in_available_channel_etc::hmac_config_send_event_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    /* 设置bssid */
    memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN,
             mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);

    /* 入网优化，不同频段下的能力不一样 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        mac_mib_set_spectrum_management_required(hmac_vap, OAL_FALSE);
    } else {
        mac_mib_set_spectrum_management_required(hmac_vap, OAL_TRUE);
    }

    /* 将结果上报至sme */
    ap_start_rsp.result_code = HMAC_MGMT_SUCCESS;
    hmac_send_rsp_to_sme_ap_etc(hmac_vap, HMAC_AP_SME_START_RSP, (osal_u8 *)&ap_start_rsp);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_chan_start_bss_etc
 功能描述  : 挑选一条信道(对)，并启动BSS
 输入参数  : hmac_vap: MAC VAP结构体指针
             mac_channel_stru *channel,
             wlan_protocol_enum_uint8 protocol
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32  hmac_chan_start_bss_etc(hmac_vap_stru *hmac_vap, mac_channel_stru *channel,
    wlan_protocol_enum_uint8 protocol)
{
    osal_u32  ul_ret;

    // 同步信道和模式
    ul_ret = hmac_sta_sync_vap(hmac_vap, channel, protocol);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_chan_start_bss_etc::hmac_sta_sync_vap failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    // 启动vap
    return hmac_start_bss_in_available_channel_etc(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_chan_restart_network_after_switch_etc
*****************************************************************************/
osal_u32 hmac_chan_restart_network_after_switch_etc(hmac_vap_stru *hmac_vap)
{
    osal_s32 ret;
    frw_msg msg_info = {0};

    oam_info_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_restart_network_after_switch_etc}", hmac_vap->vap_id);

    ret = hmac_chan_restart_network_after_switch_event(hmac_vap, &msg_info);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_2040, "vap_id[%d] hmac_chan_restart_network_after_switch_etc fail[%d]",
            hmac_vap->vap_id, ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_chan_switch_to_new_chan_comp_sync_chan_info(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device, hmac_set_chan_stru *set_chan, osal_u8 idx)
{
    unref_param(set_chan);
    unref_param(idx);
    /* 同步device信息 */
    hmac_device->max_channel   = hmac_vap->channel.chan_number;
    hmac_device->max_band      = hmac_vap->channel.band;
    hmac_device->max_bandwidth = hmac_vap->channel.en_bandwidth;
}

OAL_STATIC osal_u32 hmac_chan_switch_comp_notify_to_wal(hmac_vap_stru *hmac_vap, const hmac_set_chan_stru *set_chan)
{
    osal_s32 ret;
    frw_msg msg;
    hmac_csa_chan_switch_done_stru chan_switch_done;
    oal_ieee80211_band_enum_uint8 band;

    if (hmac_vap->init_flag == MAC_VAP_INVAILD) {
        oam_warning_log1(0, OAM_SF_CSA,
            "{hmac_chan_switch_comp_notify_to_wal::hmac_vap->init_flag[%d]!}", hmac_vap->init_flag);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (set_chan->channel.band == WLAN_BAND_2G) {
        band = OAL_IEEE80211_BAND_2GHZ;
    } else if (hmac_vap->channel.band == WLAN_BAND_5G) {
        band = OAL_IEEE80211_BAND_5GHZ;
    } else {
        band = OAL_IEEE80211_NUM_BANDS;
    }

    chan_switch_done.freq = oal_ieee80211_channel_to_frequency(set_chan->channel.chan_number, band);
    chan_switch_done.chan_number = set_chan->channel.chan_number;
    chan_switch_done.band = set_chan->channel.band;
    chan_switch_done.bandwidth = set_chan->channel.en_bandwidth;

    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    msg.data = (osal_u8*)&chan_switch_done;
    msg.data_len = OAL_SIZEOF(hmac_csa_chan_switch_done_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_RX_CSA_DONE, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg);
    if (ret != OAL_SUCC) {
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_chan_switch_to_new_chan_comp_proc(hmac_vap_stru *hmac_vap,
    hmac_device_stru *hmac_device, hmac_set_chan_stru *set_chan)
{
    osal_u32 ret;
    unref_param(hmac_device);

#if defined(_PRE_PRODUCT_ID_HOST)
    /* 信道跟随检查 */
    if (is_sta(hmac_vap)) {
        ret = hmac_check_ap_channel_follow_sta(hmac_vap, &hmac_vap->channel);
        if (ret == OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_chan_switch_to_new_chan_comp_proc:: ap channel change to %d}",
                hmac_vap->vap_id, hmac_vap->channel.chan_number);
        }
    }
#endif

    ret = hmac_chan_switch_comp_notify_to_wal(hmac_vap, set_chan);
    if (ret != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_CSA,
            "vap_id[%d] {hmac_chan_switch_to_new_chan_comp_proc::csa done chan[%d],band[%d]}",
            hmac_vap->vap_id, set_chan->channel.chan_number, set_chan->channel.band);
    }

    if (set_chan->check_cac == OAL_FALSE) {
#ifdef _PRE_WLAN_FEATURE_DFS
        /* 定时器使能位 */
        if (mac_device->dfs.dfs_cac_timer.is_registerd == OSAL_TRUE) {
            /* 关闭CAC检测时长定时器 */
            frw_destroy_timer_entry(&(mac_device->dfs.dfs_cac_timer));
            /* CAC超时前,雷达信道切换到非雷达信道,需要恢复硬件队列发送 */
            hmac_cac_chan_ctrl_machw_tx(&(hmac_vap->vap_base_info), DFS_CAC_CTRL_TX_OPEN_WITH_VAP_UP);
        }
#endif
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    if (hmac_dfs_need_for_cac(hmac_device, hmac_vap) == OAL_TRUE) {
        hmac_dfs_cac_start_etc(hmac_device, hmac_vap);
        oam_info_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_switch_to_new_chan_comp_proc::[DFS]CAC START!}",
            hmac_vap->vap_id);

        return OAL_SUCC;
    }

    ret = hmac_chan_restart_network_after_switch_etc(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_switch_to_new_chan_comp_proc::restart network fail[%d]}",
            hmac_vap->vap_id, ret);
    }
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_chan_switch_to_new_chan_complete_etc
 功能描述  : 处理信道/带宽切换完成事件
 输入参数  : frw_event_mem_stru *event_mem
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_chan_switch_to_new_chan_complete_etc(hmac_vap_stru *hmac_vap, hmac_set_chan_stru *set_chan)
{
    hmac_device_stru   *hmac_device;
    osal_u32          ret;
    osal_u8           idx;

    oam_info_log0(0, OAM_SF_2040, "{hmac_chan_restart_network_after_switch_etc}");

    if (osal_unlikely(set_chan == OAL_PTR_NULL) || osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_proc_roam_trigger_event_etc::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log1(0, OAM_SF_2040, "vap_id[%d] hmac_chan_switch_to_new_chan_complete_etc",
        hmac_vap->vap_id);

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_2040, "vap_id[%d] {hmac_chan_switch_to_new_chan_complete_etc::hmac_device null.}",
            hmac_vap->vap_id);

        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WIFI_DEBUG
    ret = hmac_dump_chan_etc(hmac_vap, (osal_u8 *)set_chan);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_2040, "vap_id[%d]{hmac_dump_chan_etc fail[%d]}", hmac_vap->vap_id, ret);
    }
#endif

    ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band,
        set_chan->channel.chan_number, &idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_2040,
            "vap_id[%d] {hmac_chan_switch_to_new_chan_complete_etc::hmac_get_channel_idx_from_num_etc failed[%d].}",
            hmac_vap->vap_id, ret);
        hmac_chan_restart_network_after_switch_etc(hmac_vap);
        return OAL_FAIL;
    }

    hmac_chan_switch_to_new_chan_comp_sync_chan_info(hmac_vap, hmac_device, set_chan, idx);
    ret = hmac_chan_switch_to_new_chan_comp_proc(hmac_vap, hmac_device, set_chan);
    return (osal_s32)ret;
}
#ifdef _PRE_WLAN_FEATURE_DBAC
/*****************************************************************************
 函 数 名  : hmac_dbac_status_notify_etc
 功能描述  : 处理dbac status event
 输入参数  : frw_event_mem_stru *event_mem
*****************************************************************************/
osal_s32 hmac_dbac_status_notify_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru    *hmac_device = OAL_PTR_NULL;
    oal_bool_enum_uint8 *pen_dbac_enable = OAL_PTR_NULL;
    osal_void *fhook = OSAL_NULL;
    hmac_vap_stru *up_vap1 = OAL_PTR_NULL;
    hmac_vap_stru *up_vap2 = OAL_PTR_NULL;

    if (osal_unlikely(msg == OAL_PTR_NULL) || osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_dbac_status_notify_etc::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pen_dbac_enable = (oal_bool_enum_uint8 *)msg->data;
    if (pen_dbac_enable == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_dbac_status_notify_etc::pen_dbac_enable is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log2(0, OAM_SF_2040,
        "vap_id[%d] hmac_dbac_status_notify_etc::dbac switch to enable=%d", hmac_vap->vap_id,
        *pen_dbac_enable);

    hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_device->en_dbac_running = *pen_dbac_enable;

    if (hmac_device->en_dbac_running) {
        /* DBAC场景，为防止host侧vap 是pause状态，无法下包，进入dbac时将vap从pause状态改为up，
        无需通知device侧，device侧切信道时会有暂停恢复动作 */
        if (hmac_device_find_2up_vap_etc(hmac_device, &up_vap1, &up_vap2) == OAL_SUCC) {
            if (up_vap1->vap_state == MAC_VAP_STATE_PAUSE) {
                up_vap1->vap_state = MAC_VAP_STATE_UP;
            }
            if (up_vap2->vap_state == MAC_VAP_STATE_PAUSE) {
                up_vap2->vap_state = MAC_VAP_STATE_UP;
            }
        }
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_STOP_CHECK);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_ps_stop_check_and_notify_cb)fhook)(hmac_vap->hal_device);
    }

    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
