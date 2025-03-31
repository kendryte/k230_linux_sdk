/*
 * Copyright (c) @CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: HMAC rx filter data.
 */

#include "hmac_rx_data_filter.h"
#include "oal_net.h"
#include "oam_ext_if.h"
#include "hmac_psm_ap.h"
#include "hmac_feature_dft.h"
#include "hmac_blockack.h"
#include "hmac_beacon.h"
#include "hmac_11i.h"
#include "hmac_alg_notify.h"
#include "hmac_uapsd.h"
#include "wlan_thruput_debug.h"
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#include "hmac_mgmt_classifier.h"
#include "common_log_dbg_rom.h"

osal_u32 g_deauth_flow_control_ms = 0;
#define DEAUTH_INTERVAL_MIN 300 /* 报文过滤过程中，发送去认证的最小间隔 300ms */

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_RX_DATA_FILTER_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_rx_data_user_is_null
 功能描述  : 收到数据帧找不到用户时的处理
*****************************************************************************/
osal_void hmac_rx_data_user_is_null(hmac_vap_stru *hmac_vap, mac_ieee80211_frame_stru *frame_hdr)
{
    osal_u32 cur_tick_ms;
    osal_u32 delta_time;

    cur_tick_ms = (osal_u32)osal_get_time_stamp_ms();
    delta_time = (osal_u32)osal_get_runtime(g_deauth_flow_control_ms, cur_tick_ms);
    if (delta_time < DEAUTH_INTERVAL_MIN) {
        oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_rx_data_user_is_null::deauth interval(%d) is too short}",
                         hmac_vap->vap_id, delta_time);
        return;
    }

    g_deauth_flow_control_ms = cur_tick_ms;

    oam_warning_log4(0, OAM_SF_RX,
                     "hmac_rx_data_user_is_null: ta: %02X:%02X:%02X:%02X:XX:XX. post sending deauth event to hmac",
                     /* 打印address2的第0、第1、第2和第3元素 */
                     frame_hdr->address2[0], frame_hdr->address2[1], frame_hdr->address2[2], frame_hdr->address2[3]);

    oam_warning_log1(0, OAM_SF_DFT, "hmac_rx_data_user_is_null:can't find user,send DEAUTH,err code[%d]",
        MAC_NOT_AUTHED);

    hmac_mgmt_send_deauth_event(hmac_vap, frame_hdr->address2, MAC_NOT_AUTHED);
}

/*****************************************************************************
 函 数 名  : hmac_rx_process_data_frame
 功能描述  : 执行DMAC模块中，接收流程的特性
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_u32 hmac_rx_process_data_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    dmac_rx_ctl_stru *cb_ctrl, oal_netbuf_stru *netbuf)
{
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);

    if (hmac_user == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_rssi_smooth(&(hmac_vap->query_stats.signal_mgmt_data), cb_ctrl->rx_statistic.rssi_dbm);

    /* 过滤二:算法过滤，MPDU占用的netbuf数目可以通过mac_rx_ctl_stru结构获得 */
    hmac_alg_rx_notify(hmac_vap, hmac_user, netbuf, &cb_ctrl->rx_statistic);

    /* 过滤三:DMAC接收低功耗的过滤 */
    if (hmac_rx_data_ps_process(hmac_vap, hmac_user, netbuf) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 过滤四:删除类型为空帧的MPDU */
    if ((frame_hdr->frame_control.sub_type == WLAN_NULL_FRAME) ||
        (frame_hdr->frame_control.sub_type == WLAN_QOS_NULL_FRAME)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_filter_frame_sta
 功能描述  : sta侧接收帧过滤
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_u32 hmac_rx_filter_frame_sta(hmac_vap_stru *hmac_vap, dmac_rx_ctl_stru *cb_ctrl,
    mac_ieee80211_frame_stru *frame_hdr, hmac_user_stru *hmac_user)
{
    osal_u8 *addr = OSAL_NULL;

    /* 获取用户、帧信息 */
    if (hmac_user == OSAL_NULL) {
        /* 返回OAL_FAIL 表示数据帧需要过滤 */
        hmac_rx_data_user_is_null(hmac_vap, frame_hdr);

        return OAL_FAIL;
    } else {
        mac_get_transmit_addr(frame_hdr, &addr);
        /* TA不是非本BSS的数据帧，不上报，直接返回失败，外面释放内存，不相等则直接返回 */
        if (memcmp(hmac_user->user_mac_addr, addr, WLAN_MAC_ADDR_LEN) != 0) {
            return OAL_FAIL;
        }
    }

    /* 更新用户的RSSI统计信息 */
    oal_rssi_smooth(&(hmac_user->rx_rssi), cb_ctrl->rx_statistic.rssi_dbm);

    /* 数据帧更新SNR */
    hmac_vap_update_snr_info(hmac_vap, cb_ctrl);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ap_rssi_process
 功能描述  : AP模式下，对接收到的low rssi帧进行处理
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_ap_rssi_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const dmac_rx_ctl_stru *cb_ctrl)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 更新用户的RSSI统计信息 */
    if (hmac_user != OSAL_NULL) {
        oal_rssi_smooth(&(hmac_user->rx_rssi), cb_ctrl->rx_statistic.rssi_dbm);
    }

    if (osal_unlikely(hmac_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_ap_rssi_process::hmac_device is null pointer}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_device->rssi_limit.rssi_limit_enable_flag == OSAL_FALSE) {
        return OAL_SUCC;  // rssi_limit开关未打开，直接返回succ
    }

    if (hmac_user != OSAL_NULL) {
        /* 用户存在, 处理已关联用的数据帧 */
        // 若帧的rssi小于门限值，就踢掉用户，并返回fail
        // 获取user真实的rssi dbm
        osal_s8 user_real_rssi_dbm = oal_get_real_rssi(hmac_user->rx_rssi);
        if (user_real_rssi_dbm < hmac_device->rssi_limit.rssi) {
            oam_warning_log4(
                0, OAM_SF_ANY,
                "vap_id[%d] {hmac_low_rssi_process::c_user_real_rssi_dbm[%d], limit_rssi[%d], user_rssi[%d].}",
                hmac_vap->vap_id, user_real_rssi_dbm, hmac_device->rssi_limit.rssi, hmac_user->rx_rssi);
            /* 踢掉用户 */
            hmac_send_disasoc_misc_msg(hmac_vap, hmac_user->assoc_id, DMAC_DISASOC_MISC_LOW_RSSI);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_rx_filter_data_frame_ap(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    dmac_rx_ctl_stru *cb_ctrl, mac_ieee80211_frame_stru *frame_hdr)
{
    osal_u32 cur_tick_ms;
    osal_u32 delta_time;

    unref_param(cb_ctrl);

    if (hmac_user == OSAL_NULL) { /* 用户不存在 */
        /* 返回OAL_FAIL 表示数据帧需要过滤 */
        hmac_rx_data_user_is_null(hmac_vap, frame_hdr);
        return OAL_FAIL;
    }

    if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) { /* 用户未关联 */
        /* 返回OAL_FAIL 表示数据帧需要过滤 */
        oam_warning_log3(0, OAM_SF_RX, "vap_id[%d] hmac_rx_filter_frame_ap:user assoc_id:%d,state:%d not assoc",
            hmac_vap->vap_id, hmac_user->assoc_id, hmac_user->user_asoc_state);

        oam_warning_log4(0, OAM_SF_RX, "hmac_rx_filter_frame_ap:user mac:%02X:%02X:%02X:%02X:XX:XX",
            /* 打印用户mac地址的第0和第1元素 */
            hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1],
            /* 打印用户mac地址的第2和第3元素 */
            hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]);

        cur_tick_ms = (osal_u32)osal_get_time_stamp_ms();
        delta_time = (osal_u32)osal_get_runtime(g_deauth_flow_control_ms, cur_tick_ms);
        if (delta_time < DEAUTH_INTERVAL_MIN) {
            oam_warning_log1(0, OAM_SF_PWR, "hmac_rx_filter_frame_ap: interval:%d < 300ms", delta_time);
            return OAL_FAIL;
        }

        g_deauth_flow_control_ms = cur_tick_ms;

        oam_warning_log1(0, OAM_SF_DFT, "hmac_rx_filter_frame_ap:rx frame,usr not assoc,send DISASSOC,err code[%d]",
            MAC_NOT_ASSOCED);
        hmac_mgmt_send_disasoc_msg(hmac_vap, frame_hdr->address2, MAC_NOT_ASSOCED);

        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_filter_frame_ap
 功能描述  : AP模式下，对MPDU的帧头信息过滤
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_rx_filter_frame_ap(hmac_vap_stru *hmac_vap, dmac_rx_ctl_stru *cb_ctrl,
    mac_ieee80211_frame_stru *frame_hdr)
{
    osal_u32 ret = OAL_SUCC;

    /* 获取用户、帧信息 */
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(cb_ctrl->rx_info.ta_user_idx);
    ret = hmac_rx_filter_data_frame_ap(hmac_vap, hmac_user, cb_ctrl, frame_hdr);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* low rssi判断 */
    if (hmac_ap_rssi_process(hmac_vap, hmac_user, cb_ctrl) != OAL_SUCC) {  // 返回不为succ，说明要踢出用户，并丢包处理
        return OAL_FAIL;
    }

    return ret;
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_u32 hmac_rx_filter_frame(hmac_vap_stru *hmac_vap, dmac_rx_ctl_stru *cb_ctrl,
    mac_ieee80211_frame_stru *frame_hdr, hmac_user_stru *hmac_user)
{
    if (hmac_vap_get_bss_type(hmac_vap) == WLAN_VAP_MODE_BSS_AP) {
        return hmac_rx_filter_frame_ap(hmac_vap, cb_ctrl, frame_hdr);
    } else if (hmac_vap_get_bss_type(hmac_vap) == WLAN_VAP_MODE_BSS_STA) {
        return hmac_rx_filter_frame_sta(hmac_vap, cb_ctrl, frame_hdr, hmac_user);
    }

    return OAL_FAIL;
}

/*****************************************************************************
函 数 名  : hmac_rx_check_mgmt_replay_failure
功能描述  : 检测是否是管理帧的重放攻击
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_rx_check_mgmt_replay_failure(const dmac_rx_ctl_stru *cb_ctrl)
{
    switch (cb_ctrl->rx_status.dscr_status) {
        case HAL_RX_CCMP_REPLAY_FAILURE:
        case HAL_RX_TKIP_REPLAY_FAILURE:
        case HAL_RX_BIP_REPLAY_FAILURE: {
            return OSAL_TRUE;
        }
        default: {
            return OSAL_FALSE;
        }
    }
}

OSAL_STATIC osal_u32 hmac_rx_process_status_failure(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    dmac_rx_ctl_stru *cb_ctrl, oal_netbuf_stru *netbuf, osal_u16 dscr_status)
{
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)(oal_netbuf_header(netbuf));
    osal_u8 *dest_addr = frame_hdr->address1;
    osal_u8 *transmit_addr = frame_hdr->address2;
    oal_nl80211_key_type key_type;

    /* 接收描述符status为key search fail时的异常处理 */
    if (dscr_status == HAL_RX_KEY_SEARCH_FAILURE) {
        /* 开启waveapp测试时，用户状态需要跟BA状态保持一致 */
        hmac_user_key_search_fail_handler(hmac_vap, hmac_user, frame_hdr);
        return OAL_FAIL;
    }
    /* 接收描述符status为 tkip mic faile 时的异常处理 */
    if (dscr_status == HAL_RX_TKIP_MIC_FAILURE) {
        oam_warning_log3(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_process_frame_failure:tkip mic err from src addr=%02x:%02x}",
            hmac_vap->vap_id, transmit_addr[0], transmit_addr[1]); /* 打印transmit addr的第0和第1元素 */
        /* 判断密钥类型,peerkey 暂不处理 */
        key_type = (ether_is_multicast(dest_addr) != 0) ? NL80211_KEYTYPE_GROUP : NL80211_KEYTYPE_PAIRWISE;
        hmac_11i_tkip_mic_failure_handler(hmac_vap, transmit_addr, key_type);

        return OAL_FAIL;
    }

    if ((frame_hdr->frame_control.type != MAC_IEEE80211_FTYPE_MGMT) &&
        (hmac_rx_check_mgmt_replay_failure(cb_ctrl) != OSAL_TRUE)) {
        /* 硬件上报了重放错误，软件需要暂时规避，跟硬件解决后再放开 */
        oam_warning_log1(0, OAM_SF_RX, "{hmac_rx_process_frame_failure::dscr_status=%d.}", dscr_status);
        return OAL_FAIL;
    }

    return OAL_CONTINUE;
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_u32 hmac_rx_process_frame_failure(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, dmac_rx_ctl_stru *cb_ctrl, oal_netbuf_stru *netbuf, osal_u16 dscr_status)
{
    if (hmac_user != OSAL_NULL) {
        /* 更新keepalive用户时间戳 */
        hmac_user->last_active_timestamp = osal_get_time_stamp_ms();

        /* 更新linkloss用户时间戳 */
        hmac_vap_linkloss_clean(hmac_vap);
        /* 信息上报统计，提供给linux内核PKT生成 */
        hmac_user->query_stats.drv_rx_pkts += 1;
        hmac_user->query_stats.drv_rx_bytes +=
            (cb_ctrl->rx_info.frame_len - cb_ctrl->rx_info.mac_header_len - SNAP_LLC_FRAME_LEN);
    }

    if (dscr_status != HAL_RX_SUCCESS) {
        if (hmac_rx_process_status_failure(hmac_vap, hmac_user, cb_ctrl, netbuf, dscr_status) != OAL_CONTINUE) {
            return OAL_FAIL;
        }
    }

    return OAL_CONTINUE;
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_u32 hmac_rx_process_frame_filter(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, dmac_rx_ctl_stru *cb_ctrl, oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
    /* 获取802.11头指针、发送端地址 */
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);

    /* 过滤一:针对发送端地址过滤(包括关联状态、AMPDU特性、加密处理) */
    ret = hmac_rx_filter_frame(hmac_vap, cb_ctrl, frame_hdr, hmac_user);
    if (ret != OAL_SUCC) { /* 程序异常 */
        return OAL_FAIL;
    }

    /* 增加:FTM使能的时,ap处理管理帧的唤醒信息 */
#ifdef _PRE_WLAN_FEATURE_FTM
    if (mac_check_ftm_enable(hmac_vap) != MAC_FTM_DISABLE_MODE)
#endif
    {
        /* AGC绑定通道设定  */
        ret = hmac_rx_process_data_frame(hmac_vap, hmac_user, cb_ctrl, netbuf);
        if (ret != OAL_SUCC) {
            return OAL_FAIL;
        }
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_rx_process_frame
 功能描述  : 处理每一个帧的操作的函数,主要针对接收流程中的帧的各种过滤处理
*****************************************************************************/
WIFI_HMAC_TCM_TEXT dmac_rx_frame_ctrl_enum_uint8 hmac_rx_deal_frame_filter(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, dmac_rx_ctl_stru *cb_ctrl, oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
    osal_u16 dscr_status = cb_ctrl->rx_status.dscr_status;

    ret = hmac_rx_process_frame_failure(hmac_vap, hmac_user, cb_ctrl, netbuf, dscr_status);
    if (ret != OAL_CONTINUE) {
        common_log_dbg1(0, OAM_SF_RX, "{hmac_rx_process_frame_failure::drop dscr_status=%d.}", dscr_status);
        goto rx_pkt_drop;
    }

    ret = hmac_rx_process_frame_filter(hmac_vap, hmac_user, cb_ctrl, netbuf);
    if (ret != OAL_CONTINUE) {
        common_log_dbg0(0, OAM_SF_RX, "{hmac_rx_process_frame_filter::drop.}");
        goto rx_pkt_drop;
    }

    return DMAC_RX_FRAME_CTRL_BUTT;
rx_pkt_drop:
    /* 丢帧返回 */
    if (hmac_user != OSAL_NULL) {
        /* 信息上报统计，提供给linux内核PKT生成 */
        hmac_user->query_stats.rx_dropped_misc += 1;
    }

    return DMAC_RX_FRAME_CTRL_DROP;
}
