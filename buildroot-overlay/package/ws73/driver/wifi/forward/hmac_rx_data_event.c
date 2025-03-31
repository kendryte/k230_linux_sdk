/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: DMAC模块接收帧的公共操作函数以及数据帧的操作函数定义的源文件
 * Date: 2020-07-04
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_rx_data_event.h"
#include "mac_resource_ext.h"
#include "hmac_feature_dft.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_beacon.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_m2s.h"
#include "wlan_thruput_debug.h"
#include "frw_hmac.h"
#include "hmac_rx_data_filter.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_rx_data.h"
#include "oal_netbuf_data.h"
#ifdef _PRE_WLAN_FEATURE_STAT
#include "hmac_stat.h"
#endif
#include "hmac_promisc.h"
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
#include "hmac_ant_switch.h"
#endif

#include "hmac_latency_stat.h"
#include "hmac_feature_interface.h"
#include "hmac_dfx.h"
#include "hmac_btcoex.h"
#include "common_log_dbg_rom.h"
#include "hmac_thruput_test.h"
#include "hmac_hook.h"
#ifdef _PRE_WLAN_PEAK_PERFORMANCE_DFX
#include "hmac_rx_pk.h"
#endif
#include "hmac_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_RX_DATA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE
/*****************************************************************************
 函 数 名  : hmac_rx_update_user_eapol_key_open
 功能描述  : 更新用户is_rx_eapol_key_open 标识。
             如果接收到的eapol-key 是open，设置is_rx_eapol_key_open OSAL_TRUE
             其他值，设置is_rx_eapol_key_open OSAL_FALSE
*****************************************************************************/
OSAL_STATIC osal_void hmac_rx_update_user_eapol_key_open(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    dmac_rx_ctl_stru *rx_cb;
    hmac_user_stru *hmac_user = OSAL_NULL;
    mac_eapol_header_stru *eapol_header = OSAL_NULL;

    rx_cb = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    if (is_sta(hmac_vap) && rx_cb->rx_status.dscr_status == HAL_RX_SUCCESS) {
        eapol_header = (mac_eapol_header_stru *)(oal_netbuf_rx_data(netbuf) + sizeof(mac_llc_snap_stru));
        if (hmac_is_eapol_key_ptk_etc(eapol_header) == OSAL_TRUE) {
            hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
            if (hmac_user == OSAL_NULL) {
                oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_update_user_eapol_key_open::hmac_user:%d is null.}",
                                 hmac_vap->vap_id, hmac_vap->assoc_vap_id);
                return;
            }
            hmac_user->is_rx_eapol_key_open = (rx_cb->rx_status.cipher_protocol_type == HAL_NO_ENCRYP);

            oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] hmac_rx_update_user_eapol_key_open::is_rx_eapol_key_open=%d",
                             hmac_vap->vap_id, hmac_user->is_rx_eapol_key_open);
        }
    }

    return;
}

/* VIP 关键帧上报 */
OSAL_STATIC osal_void hmac_rx_process_frame_eapol_keyinfo(mac_ieee80211_frame_stru *frame_hdr,
    oal_netbuf_stru *curr_netbuf, const hmac_vap_stru *hmac_vap, const mac_rx_ctl_stru *rx_info,
    osal_u8 data_type)
{
    osal_u16 eapol_keyinfo = mac_get_eapol_keyinfo_etc(curr_netbuf);
    unref_param(frame_hdr);
    unref_param(rx_info);
    unref_param(eapol_keyinfo);

    if ((data_type == MAC_DATA_EAPOL) && (mac_get_eapol_type_etc(curr_netbuf) == OAL_EAPOL_TYPE_KEY)) {
        oam_warning_log2(0, OAM_SF_CONN, "vap_id[%d] {hmac_rx_process_frame_eapol_keyinfo::rx eapol, info is %x }",
                         hmac_vap->vap_id, oal_net2host_short(eapol_keyinfo));
        hmac_rx_update_user_eapol_key_open(hmac_vap, curr_netbuf);
    }
}

/*****************************************************************************
 校验接收帧加密方式
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_u8 hmac_rx_data_check_cipher(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u8 *sa_mac_addr = OSAL_NULL;
    osal_u8 *da_mac_addr = OSAL_NULL;
    mac_ieee80211_frame_stru *mac_hdr = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 data_type;
    osal_u8 bssid[WLAN_MAC_ADDR_LEN];

    mac_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    /* 非单播帧不做判断 */
    mac_rx_get_da(mac_hdr, &da_mac_addr);
    if (ether_is_multicast(da_mac_addr) == OSAL_TRUE) {
        return OSAL_TRUE;
    }
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_get_bssid((osal_u8 *)mac_hdr, bssid, sizeof(bssid));
        hmac_user = hmac_vap_get_user_by_addr_etc(hmac_vap, bssid, sizeof(bssid));
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_rx_get_sa(mac_hdr, &sa_mac_addr);
        hmac_user = hmac_vap_get_user_by_addr_etc(hmac_vap, sa_mac_addr, WLAN_MAC_ADDR_LEN);
    } else {
        return OSAL_TRUE;
    }

    if (hmac_user == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_rx_data_check_cipher::dmac_user is NULL.}");
        return OSAL_FALSE;
    }

    /* DHCP 帧不判断: 个别路由器加密连接DHCP 明文发送 */
    data_type = hmac_get_data_type_from_8023_etc((osal_u8 *)mac_hdr, MAC_NETBUFF_PAYLOAD_ETH);
    if (data_type <= MAC_DATA_DHCP) {
        return OSAL_TRUE;
    }

    if ((hmac_user->key_info.cipher_type != WLAN_80211_CIPHER_SUITE_NO_ENCRYP) &&
        (mac_hdr->frame_control.protected_frame == 0)) {
        oam_warning_log2(0, OAM_SF_RX, "{hmac_rx_data_check_cipher::cipher_type[%d] != protected_frame[%d].}",
            hmac_user->key_info.cipher_type, mac_hdr->frame_control.protected_frame);
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : 接收流程数据帧处理总入口
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_u32 hmac_rx_process_data(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
#if defined(_PRE_WLAN_DFT_STAT)
    osal_u32 msdu_cnt = 0;
#endif
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AUTO_FREQ_FRAME_COUNT);

#ifdef _PRE_WLAN_FEATURE_STAT
    hmac_stat_device_rx_netbuf(OAL_NETBUF_LEN(netbuf));
#endif
    /* 收包处理 */
    if (fhook != OSAL_NULL) {
        ((hmac_auto_freq_pps_count_cb)fhook)(1);
    }
    if (hmac_rx_data_check_cipher(hmac_vap, netbuf) != OSAL_TRUE) {
        return OAL_FAIL;
    }

#if defined(_PRE_WLAN_DFT_STAT)
    msdu_cnt = hmac_dft_rx_get_dscr_msdu(netbuf, 1);
#endif
    ret = hmac_rx_data(hmac_vap, netbuf);
    if (ret != OAL_SUCC) {
#ifdef _PRE_WLAN_DFT_STAT
        hmac_vap->query_stats.rx_d_send_hmac_fail += msdu_cnt;
#endif
        common_log_dbg2(0, 0, "vap_id[%d] hmac_rx_process_data:event_dispatch err=%u", hmac_vap->vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_data_dscr_bw_process
 功能描述  : 带宽校验状态下每次接收完成中断上报带宽值
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_void hmac_rx_data_dscr_bw_process(hmac_vap_stru *hmac_vap, dmac_rx_ctl_stru *rx_ctl)
{
    hmac_user_stru *hmac_user = OSAL_NULL;

    /* 带宽校验状态下BW_FSM上报接收完成中断带宽信息统计 */
    if (!((is_legacy_sta(hmac_vap) != 0) && (mac_vap_bw_fsm_verify(hmac_vap) != 0))) {
        return;
    }

    /* 获取用户、帧信息 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(rx_ctl->rx_info.ta_user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_data_dscr_bw_process::hmac_user[%d] null.}",
                         hmac_vap->vap_id, rx_ctl->rx_info.ta_user_idx);
        return;
    }

    /* 广播用户不需要上报 */
    if (hmac_user->is_multi_user == OSAL_TRUE) {
        return;
    }

    /* 带宽校验状态下BW_FSM上报接收完成中断带宽信息统计 */
    hmac_sta_bw_switch_fsm_post_event(hmac_vap, HMAC_STA_BW_SWITCH_EVENT_RX_UCAST_DATA_COMPLETE,
                                      sizeof(dmac_rx_ctl_stru), (osal_u8 *)rx_ctl);
    return;
}

/* ****************************************************************************
 功能描述  : 数据帧一些小特性处理
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_void hmac_rx_process_data_feature(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, dmac_rx_ctl_stru *cb_ctrl, mac_rx_ctl_stru *rx_info)
{
    osal_void *fhook = OSAL_NULL;
    osal_u8 frame_subtype;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_M2S
    osal_u8 data_type;
#endif
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
    hal_to_dmac_device_stru *device = hal_chip_get_hal_device();
#endif

    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    frame_subtype = mac_frame_get_subtype_value((const osal_u8 *)frame_hdr);
    if (frame_subtype == WLAN_NULL_FRAME) {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
    if ((cb_ctrl->rx_status.last_mpdu_flag == OSAL_TRUE) && (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        hmac_ant_rx_frame(device, hmac_vap);
    }
#endif
    /* EAPOL帧处理 */
    if (((mac_llc_snap_stru *)frame_hdr)->ether_type == oal_host2net_short(ETHER_TYPE_PAE)) {
        hmac_rx_process_frame_eapol_keyinfo(frame_hdr, netbuf, hmac_vap, rx_info, MAC_DATA_EAPOL);
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_RX_PROCESS_ECT);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_rx_process_ect_cb)fhook)(hmac_vap, frame_subtype, netbuf, cb_ctrl->rx_status.ampdu);
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    data_type = hmac_get_rx_data_type_etc(netbuf);
    if (data_type <= MAC_DATA_ARP_RSP) {
        cb_ctrl->rx_info.is_key_frame = 1;
    }
    hmac_m2s_rx_rate_nss_process(hmac_vap, cb_ctrl, frame_hdr);
#endif
}

/* ****************************************************************************
 功能描述  : 从dmac.bin接收rx报文，dmac.ko处理总入口，包括管理帧和数据帧
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_s32 hmac_rx_process_data_msg(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    osal_u32 ret = OAL_FAIL;
    osal_u32 hook_ret = OAL_FAIL;
    mac_rx_ctl_stru *rx_info = OSAL_NULL;
    dmac_rx_ctl_stru *cb_ctrl = OSAL_NULL;
    dmac_rx_frame_ctrl_enum_uint8 frame_ctrl;
    osal_u16 user_idx;
    osal_void *fhook = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;

    netbuf = *((oal_netbuf_stru **)(msg->data));
    if (osal_unlikely(netbuf == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_data_msg: netbuf null!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取每一个MPDU的控制信息 */
    cb_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    rx_info = &(cb_ctrl->rx_info);
    /* 获取user 引用计数自增 */
    user_idx = cb_ctrl->rx_info.ta_user_idx;
    hmac_user_use_cnt_inc(user_idx, &hmac_user);

    /* 接收到D2H数据包HOOK处理 */
    hook_ret = hmac_call_netbuf_hooks(&netbuf, hmac_vap, HMAC_FRAME_DATA_RX_EVENT_D2H);
    if (hook_ret != OAL_CONTINUE) {
        /* 目前monitor流程中可能返回非OAL_CONTINUE */
        if (hmac_config_sta_rcv_probe_req_is_on() == OSAL_FALSE) {
            oal_netbuf_free(netbuf);
        }
        hmac_user_use_cnt_dec(hmac_user);
        return (osal_s32)hook_ret;
    }

    /* RSSI插损值 */
    hal_config_rssi_for_loss(rx_info->channel_number, (osal_s8 *)(&cb_ctrl->rx_statistic.rssi_dbm));

    /* rx流程小特性处理 */
    hmac_rx_process_data_feature(hmac_vap, netbuf, cb_ctrl, rx_info);

    frame_ctrl = hmac_rx_deal_frame_filter(hmac_vap, hmac_user, cb_ctrl, netbuf);
    if (frame_ctrl == DMAC_RX_FRAME_CTRL_DROP) {
        oal_netbuf_free(netbuf);
        hmac_user_use_cnt_dec(hmac_user);
        return OAL_FAIL;
    }

    /* host侧RX方向用户速率信息统计 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_RATE_RX_INFO_RECORD);
    if (fhook != OSAL_NULL) {
        ((dfx_user_rate_rx_info_record_cb)fhook)(cb_ctrl->rx_info.ta_user_idx, cb_ctrl);
    }
    hmac_rx_data_dscr_bw_process(hmac_vap, cb_ctrl);

    ret = hmac_rx_process_data(hmac_vap, netbuf);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        hmac_user_use_cnt_dec(hmac_user);
        return OAL_FAIL;
    }

    hmac_user_use_cnt_dec(hmac_user);
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_s32 hmac_rx_process_data_msg_process(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    hmac_delay(3); /* 3:hmac rx时延打点 */
#if defined(_PRE_WLAN_FEATURE_PK) && defined(_PRE_WLAN_PEAK_PERFORMANCE_DFX)
    if (hmac_is_thruput_enable(THRUPUT_RX_PK)) {
        ret = hmac_rx_process_data_msg_pk(hmac_vap, msg);
        hmac_delay(4); /* 4:hmac rx时延打点 */
        return ret;
    }
#endif
    ret = hmac_rx_process_data_msg(hmac_vap, msg);
    hmac_delay(4); /* 4:hmac rx时延打点 */
    return ret;
}

WIFI_HMAC_TCM_TEXT osal_s32 hmac_rx_data_event_adapt(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    dmac_rx_ctl_stru *cb_ctrl = OSAL_NULL;
    oal_netbuf_stru *netbuf = *((oal_netbuf_stru **)(msg->data));

    if (osal_unlikely(netbuf == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_rx_data_event_adapt: netbuf null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_is_thruput_enable(THRUPUT_RX_REPORT) != 0) {
        cb_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        hmac_update_thruput(cb_ctrl->rx_info.frame_len);
    }

    if (hmac_is_thruput_enable(THRUPUT_RX_AFTER_HCC) != 0) {
        cb_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        hmac_update_thruput(cb_ctrl->rx_info.frame_len);
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    if (hmac_is_thruput_enable(THRUPUT_RESUME_FRW_RX_DATA) != 0) {
        return hmac_rx_process_data_msg(hmac_vap, msg);
    }

    ret = frw_host_post_msg(WLAN_MSG_H2H_RX, FRW_POST_PRI_LOW, hmac_vap->vap_id, msg);
    if (ret == OAL_ERR_CODE_ALLOC_MEM_FAIL) {
        oal_netbuf_free(netbuf);
    }
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
