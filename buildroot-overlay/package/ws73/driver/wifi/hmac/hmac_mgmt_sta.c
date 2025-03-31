/*
 * Copyright (c) CompanyNameMagicTag. 2013-2024. All rights reserved.
 * 文 件 名   : hmac_mgmt_sta.c
 * 生成日期   : 2013年6月18日
 * 功能描述   : STA侧管理面处理
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_mgmt_sta.h"
#include "wlan_spec.h"
#include "wlan_mib_hcm.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_user_ext.h"
#include "mac_vap_ext.h"
#include "mac_device_ext.h"
#include "hmac_device.h"
#include "hmac_user.h"
#include "hmac_fsm.h"
#include "hmac_rx_data.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_encap_frame_sta.h"
#include "hmac_sme_sta.h"
#include "hmac_scan.h"
#include "hmac_tx_amsdu.h"
#include "hmac_11i.h"
#include "hmac_11w.h"
#include "hmac_protection.h"
#include "hmac_config.h"
#include "hmac_ext_if.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_mgmt_ap.h"
#include "hmac_blockack.h"
#include "frw_ext_if.h"
#include "wlan_msg.h"
#include "common_dft.h"
#include "hmac_psm_sta.h"
#include "hmac_scan.h"
#include "hmac_beacon.h"
#include "hmac_power.h"
#include "hmac_sr_sta.h"
#include "hmac_alg_notify.h"
#include "hmac_main.h"
#include "hmac_dfr.h"
#include "hmac_roam_if.h"
#include "hmac_uapsd_sta.h"
#include "hmac_frag.h"
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#endif
#include "hmac_btcoex.h"
#include "hmac_btcoex_m2s.h"

#include "wlan_msg.h"
#include "securec.h"
#include "hmac_dfx.h"
#include "hmac_feature_dft.h"
#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "hmac_txopps.h"
#endif
#include "msg_blockack_rom.h"

#include "hmac_feature_interface.h"
#include "hmac_sr_sta.h"
#include "hmac_bsrp_nfrp.h"
#include "hmac_11k.h"
#include "hmac_promisc.h"
#include "frw_util_notifier.h"
#include "hmac_11v.h"
#include "hmac_achba.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_MGMT_STA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define mac_addr(_puc_mac)   ((osal_u32)(((osal_u32)(_puc_mac)[2] << 24) | \
                                              ((osal_u32)(_puc_mac)[3] << 16) | \
                                              ((osal_u32)(_puc_mac)[4] << 8) | \
                                              ((osal_u32)(_puc_mac)[5])))

#define WLAN_AUTH_SUITE_SAE_SELECTOR 0X08AC0F00
#define WLAN_REASON_IEEE_802_1X_AUTH_FAILED 23

/*****************************************************************************
  全局变量定义
*****************************************************************************/
/* 与dmac层的同步修改 */
hmac_data_rate_stru g_st_data_rates[DATARATES_80211G_NUM] = {
    {0x82, 0x02, 0x00, WLAN_11B_PHY_PROTOCOL_MODE},  /* 1 Mbps   */
    {0x84, 0x04, 0x01, WLAN_11B_PHY_PROTOCOL_MODE},  /* 2 Mbps   */
    {0x8B, 0x0B, 0x02, WLAN_11B_PHY_PROTOCOL_MODE},  /* 5.5 Mbps */
    {140, 0x0C, 0x0B, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE},  /* 6 Mbps   */
    {146, 0x12, 0x0F, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE},  /* 9 Mbps   */
    {0x96, 0x16, 0x03, WLAN_11B_PHY_PROTOCOL_MODE}, /* 11 Mbps  */
    {152, 0x18, 0x0A, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE}, /* 12 Mbps  */
    {164, 0x24, 0x0E, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE}, /* 18 Mbps  */
    {176, 0x30, 0x09, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE}, /* 24 Mbps  */
    {200, 0x48, 0x0D, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE}, /* 36 Mbps  */
    {224, 0x60, 0x08, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE}, /* 48 Mbps  */
    {236, 0x6C, 0x0C, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE}  /* 54 Mbps  */
};
/* Sta最后一次连接的状态码，如果成功则为0，否则为对端Ap回应的错误码或者驱动产生的私有错误码 */
osal_u16 g_last_reason_code = 0;
/*****************************************************************************
  函数定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC osal_void hmac_update_mu_edca_machw_sta(hmac_vap_stru *hmac_vap);
#endif
OAL_STATIC osal_u32 hmac_sta_rx_deauth_req(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    oal_bool_enum_uint8 is_protected);

osal_void hmac_sta_set_last_reason_code(osal_u16 reason_code)
{
    g_last_reason_code = reason_code;
}

osal_void hmac_sta_print_last_reason_code(osal_void)
{
    wifi_printf("Sta_last_connect Status code is [%d]\r\n", g_last_reason_code);
}

/*****************************************************************************
 函 数 名  : hmac_multi_vap_state_check
 功能描述  : 多状态共存状态检查
*****************************************************************************/
OAL_STATIC osal_u8 hmac_multi_vap_state_check(hmac_device_stru *hmac_device)
{
    hmac_vap_stru *mac_vap1 = OSAL_NULL;
    hmac_vap_stru *mac_vap2 = OSAL_NULL;
    osal_u32 ret;

    ret = hmac_device_find_2up_vap_etc(hmac_device, &mac_vap1, &mac_vap2);
    if (ret != OAL_SUCC) {
        return HMAC_VAP_MVAP_STATE_TRANS_NONE;
    }

    if (mac_is_dbac_running(hmac_device)) {
        if (mac_vap1->channel.chan_number == mac_vap2->channel.chan_number) {
            return HMAC_VAP_MVAP_STATE_TRANS_DBAC_TO_DCHN;
        } else {
            return HMAC_VAP_MVAP_STATE_TRANS_DBAC_TO_DBAC;
        }
    } else {
        if (mac_vap1->channel.chan_number == mac_vap2->channel.chan_number) {
            return HMAC_VAP_MVAP_STATE_TRANS_DCHN_TO_DCHN;
        } else {
            return HMAC_VAP_MVAP_STATE_TRANS_DCHN_TO_DBAC;
        }
    }
}

OAL_STATIC osal_void hmac_chan_multi_switch_channel(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, osal_u8 old_chan_number, osal_u8 change_type)
{
    mac_channel_stru channel_real;
    unref_param(old_chan_number);
    switch (change_type) {
        case HMAC_VAP_MVAP_STATE_TRANS_NONE:
        case HMAC_VAP_MVAP_STATE_TRANS_DBDC_TO_DBDC:
        case HMAC_VAP_MVAP_STATE_TRANS_DCHN_TO_DCHN:
            /* 变量初始化为当前vap信息 */
            channel_real = hmac_vap->channel;

            /* 选择需要设置的信道信息 */
            hmac_chan_select_real_channel(hmac_device, &channel_real, channel_real.chan_number);

            oam_warning_log4(0, OAM_SF_ANY, "vap[%d] hmac_chan_multi_switch_channel:vap bw[%d],new bw[%d],hal bw=[%d]",
                hmac_vap->vap_id, hmac_vap->channel.en_bandwidth, channel_real.en_bandwidth,
                hal_device->wifi_channel_status.en_bandwidth);
            /* 切换信道不需要清fifo，传入FALSE */
            hmac_mgmt_switch_channel(hal_device, &channel_real, OSAL_FALSE);
            break;

        case HMAC_VAP_MVAP_STATE_TRANS_DBAC_TO_DCHN:
            /* DBAC场景下,切换信道后为同信道,需要关闭DBAC,重新设置2个vap最大带宽 */
#ifdef _PRE_WLAN_FEATURE_DBAC
            hmac_alg_update_dbac_fcs_config(hmac_vap);
#endif
            hmac_vap_down_notify(hmac_vap);

            channel_real = hmac_vap->channel;
            hmac_chan_select_real_channel(hmac_device, &channel_real, channel_real.chan_number);

            /* 切换信道不需要清fifo，传入FALSE */
            hmac_mgmt_switch_channel(hal_device, &channel_real, OSAL_FALSE);
            break;

        case HMAC_VAP_MVAP_STATE_TRANS_DBAC_TO_DBAC:
            /* DBAC场景下,切换信道后，仍是异信道,只需要更新dbac参数 */
#ifdef _PRE_WLAN_FEATURE_DBAC
            hmac_alg_update_dbac_fcs_config(hmac_vap);
#endif
            break;

        case HMAC_VAP_MVAP_STATE_TRANS_DCHN_TO_DBAC:
            /* 非DBAC场景下,切换信道后为异信道, 需要启动DBAC */

            hmac_alg_vap_up_notify(hmac_vap);

            oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_chan_multi_switch_channel:: start dbac,hal chan is %d.}",
                hmac_vap->vap_id, hal_device->current_chan_number);

            break;

        case HMAC_VAP_MVAP_STATE_TRANS_DBDC_TO_DBAC:
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_chan_multi_switch_channel: dbdc to dbac send disasoc. type[%d]}",
                hmac_vap->vap_id, change_type);
            hmac_send_disassoc_misc_event(hmac_vap, hmac_vap->assoc_vap_id, DMAC_DISASOC_MISC_CHANNEL_MISMATCH);
            break;
        default:

            oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_chan_multi_switch_channel: change_type[%d]}",
                hmac_vap->vap_id, change_type);
            return;
    }
}

/*****************************************************************************
 函 数 名  : hmac_chan_multi_select_channel_mac
 功能描述  : 遍历device下所有VAP，设置SW/MAC/PHY/RF中的信道和带宽，使VAP工作在新信道上
*****************************************************************************/
osal_void hmac_chan_multi_select_channel_mac(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u8 old_chan_number, old_bandwidth, idx, change_type;

    if (hmac_device->vap_num == 0) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_chan_multi_select_channel_mac::none vap.}", hmac_vap->vap_id);
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_chan_multi_select_channel_mac::vap id [%d],hal_device null}", hmac_vap->vap_id);
        return;
    }

    oam_warning_log4(0, OAM_SF_ANY,
        "{hmac_chan_multi_select_channel_mac:: Switching from chan[%d] BW[%d] mode To chan[%d] BW[%d].}",
        hal_device->current_chan_number, hmac_vap->channel.en_bandwidth, channel, bandwidth);
    /* 如果是ap切信道,则记录新的信道带宽信息用于自愈 */
    if (is_ap(hmac_vap)) {
        hmac_dfr_record_ap_follow_channel(channel, bandwidth);
    }

    /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
    ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, channel, &idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_chan_multi_select_channel_mac::get_channel_idx failed[%d].}", hmac_vap->vap_id, ret);
        return;
    }

    old_chan_number = hmac_vap->channel.chan_number;
    old_bandwidth = hmac_vap->channel.en_bandwidth;
    hmac_vap->channel.chan_number = channel;
    hmac_vap->channel.en_bandwidth = bandwidth;
    hmac_vap->channel.chan_idx = idx;
    hmac_vap_change_mib_by_bandwidth_etc(hmac_vap, hmac_vap->channel.band, bandwidth);
    if (old_chan_number != hmac_vap->channel.chan_number || old_bandwidth != hmac_vap->channel.en_bandwidth) {
        hmac_vap_sync(hmac_vap);
    }

    /* sta和ap的user都需要刷新带宽，并通知算法，并同步到host */
    hmac_chan_update_user_bandwidth(hmac_vap);

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 使能去使能雷达检测 */
    if (is_ap(hmac_vap) && (mac_vap_get_dfs_enable(hmac_vap) == OSAL_TRUE)) {
        hmac_dfs_radar_detect_check(hal_device, hmac_device, hmac_vap);
    }
#endif

    /* 刷新发送功率 */
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_INIT);

    /* 通知算法信道改变 */
    hmac_alg_cfg_channel_notify(hmac_vap, CH_BW_CHG_TYPE_MOVE_WORK);
    change_type = hmac_multi_vap_state_check(hmac_device);
    oam_warning_log2(0, 0, "vap_id[%d] {hmac_chan_multi_select_channel_mac: type[%d]}", hmac_vap->vap_id, change_type);

    hmac_chan_multi_switch_channel(hmac_vap, hmac_device, hal_device, old_chan_number, change_type);
}

/*****************************************************************************
 函 数 名  : hmac_tx_delete_ba
 功能描述  : 删除某个用户下的所有ba会话
*****************************************************************************/
osal_void hmac_tx_delete_ba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_s32 ret;
    osal_u8 tid;
    mac_ctx_event_stru del_ba_event;
    frw_msg msg = {0};

    for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
        /* 需要暂停对应TID队列,待BA删除后再恢复 */
        hmac_tid_pause(&hmac_user->tx_tid_queue[tid], DMAC_TID_PAUSE_RESUME_TYPE_BA);
    }

    del_ba_event.user_index = hmac_user->assoc_id;
    del_ba_event.vap_id = hmac_user->vap_id;
    del_ba_event.cur_protocol = hmac_user->cur_protocol_mode;

    msg.data = (osal_u8 *)&del_ba_event;
    msg.data_len = sizeof(del_ba_event);

    ret = hmac_del_ba_event(hmac_vap, &msg);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_delete_ba::failed}", hmac_user->vap_id);
    }

    /* 删除ba后的处理流程 */
    for (tid = 0; tid < WLAN_TID_MAX_NUM; tid++) {
        /* 删除ba结束后，恢复TID队列 */
        hmac_tid_resume(&hmac_user->tx_tid_queue[tid], DMAC_TID_PAUSE_RESUME_TYPE_BA);
    }
    return;
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 函 数 名  : hmac_sta_up_update_he_oper_params
 功能描述  : STA收到帧后，处理he相关信息元素
*****************************************************************************/
osal_u32 hmac_sta_up_update_he_oper_params(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 frame_len,
    hmac_user_stru *hmac_user)
{
    osal_u8 *he_opern_ie = OSAL_NULL;
    osal_u32 change = MAC_NO_CHANGE;

    if ((hmac_vap == OSAL_NULL) || (hmac_vap->hal_vap == OSAL_NULL)) {
        return change;
    }

    /* 支持11ax，才进行后续的处理 */
    if (mac_mib_get_he_option_implemented(hmac_vap) == OSAL_FALSE) {
        return change;
    }

    he_opern_ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_OPERATION, payload, frame_len);
    if (he_opern_ie == OSAL_NULL) {
        return change;
    }

    change = hmac_ie_proc_he_opern_ie(hmac_vap, he_opern_ie, hmac_user);
    if ((change & MAC_HE_BSS_COLOR_CHANGE) != 0) { /* 更新bss color */
        hal_set_bss_color_enable(OSAL_TRUE);
        hal_set_bss_color(hmac_vap->hal_vap, hmac_user->he_hdl.he_oper_ie.bss_color.bss_color);
    }
    if ((change & MAC_HE_PARTIAL_BSS_COLOR_CHANGE) != 0) { /* 更新 partial bss color */
        hal_set_bss_color_enable(OSAL_TRUE);
        hal_set_partial_bss_color(hmac_vap->hal_vap,
            hmac_user->he_hdl.he_oper_ie.bss_color.partial_bss_color);
    }
    if ((change & MAC_HE_BSS_COLOR_DISALLOW) != 0) { /* 禁用 bss color */
        hal_set_bss_color_enable(OSAL_FALSE);
    }
    if ((change & MAC_RTS_TXOP_THRED_CHANGE) != 0) {
        hmac_config_user_rate_info_syn_etc(hmac_vap, hmac_user);
    }

    if ((change & MAC_HE_CHANGE) != 0) {
        change = MAC_HE_CHANGE;
    } else {
        change = MAC_NO_CHANGE;
    }
    return change;
}
#endif

#ifdef _PRE_WLAN_FEATURE_OM
osal_s32 hmac_omi_rx_he_rom_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_vap_stru *hmac_rom_vap = OSAL_NULL;
    hal_to_dmac_he_rom_update_stru *rom_info = OSAL_NULL;
    wlan_bw_cap_enum_uint8 vap_bw;
    osal_u8 old_avail_bandwidth, old_cur_bandwidth, old_avail_num_spatial_stream;

    unref_param(hmac_vap);
    rom_info = (hal_to_dmac_he_rom_update_stru *)(msg->data);

    hmac_rom_vap = mac_res_get_hmac_vap(rom_info->vap_id);
    if (osal_unlikely(hmac_rom_vap == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = mac_res_get_hmac_user_etc(hmac_rom_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    old_avail_bandwidth = hmac_user->avail_bandwidth;
    old_cur_bandwidth = hmac_user->cur_bandwidth;
    old_avail_num_spatial_stream = hmac_user->avail_num_spatial_stream;

    /* HE ROM中断上报的bw不超过vap bw,才设置avail_bandwidth和cur_bandwidth */
    vap_bw = WLAN_BW_CAP_BUTT;
    hmac_vap_get_bandwidth_cap_etc(hmac_rom_vap, &vap_bw);
    if (rom_info->bw <= vap_bw) {
        hmac_user_set_bandwidth_info_etc(hmac_user, rom_info->bw, rom_info->bw);
    }

    /* HE ROM中断上报的nss不超过vap nss,才设置avail_num_spatial_stream */
    if (rom_info->nss <= hmac_rom_vap->vap_rx_nss) {
        hmac_user_set_avail_num_spatial_stream_etc(hmac_user, rom_info->nss);
    }
    if (old_avail_bandwidth != hmac_user->avail_bandwidth || old_cur_bandwidth != hmac_user->cur_bandwidth ||
        old_avail_num_spatial_stream != hmac_user->avail_num_spatial_stream) {
        /* 空间流/带宽更新 则同步至device */
        hmac_user_sync(hmac_user);
    }

    oam_warning_log3(0, OAM_SF_ANY,
        "{hmac_omi_rx_he_rom_event:: rom info bw[%d], rom info nss[%d], rom info vap id[%d]",
        rom_info->bw, rom_info->nss, rom_info->vap_id);
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
osal_void hmac_sta_update_affected_acs_machw_direct(osal_u8 *payload, hmac_vap_stru *hmac_vap)
{
    osal_u8 affected_acs = 0;
    mac_mu_edca_control *reg_control_param = (mac_mu_edca_control *)&affected_acs;
    mac_mu_edca_control *proto_ctl_param;

    if (mac_mib_get_he_option_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }

    payload += MAC_ACTION_OFFSET_BODY;
    proto_ctl_param = (mac_mu_edca_control *)payload;

    /* 协议中规定的B0~B3分别对应BK、BE、VI、VO，而硬件刚好相反，对应VO、VI、BE、BK，因此要做个翻转 */
    reg_control_param->affected_acs_b0 = proto_ctl_param->affected_acs_b3;
    reg_control_param->affected_acs_b1 = proto_ctl_param->affected_acs_b2;
    reg_control_param->affected_acs_b2 = proto_ctl_param->affected_acs_b1;
    reg_control_param->affected_acs_b3 = proto_ctl_param->affected_acs_b0;

    hal_set_affected_acs(affected_acs);

    return;
}

/*****************************************************************************
 函 数 名: hmac_update_mu_edca_machw_sta
 功能描述  : ax协议下，STA接收到beacon帧更新自身的EDCA参数，涉及到mib值和寄存器
*****************************************************************************/
OAL_STATIC osal_void hmac_update_mu_edca_machw_sta(hmac_vap_stru *hmac_vap)
{
    wlan_wme_ac_type_enum_uint8 ac_type;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        return;
    }

    /* 更新 MU EDCA AIFSN 寄存器 */
    hal_set_mu_edca_aifsn((osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BK),
        (osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BE),
        (osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VI),
        (osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VO));

    /* 更新 MU CW 寄存器 */
    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        hal_set_mu_edca_cw(
            ac_type, (osal_u8)mac_mib_get_qap_mu_edca_table_cwmax(hmac_vap, ac_type),
            (osal_u8)mac_mib_get_qap_mu_edca_table_cwmin(hmac_vap, ac_type));
    }

    /* 更新 MU EDCA Lifetime寄存器 */
    hal_set_mu_edca_lifetime((osal_u8)mac_mib_get_qap_mu_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BK),
        (osal_u8)mac_mib_get_qap_mu_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BE),
        (osal_u8)mac_mib_get_qap_mu_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_VI),
        (osal_u8)mac_mib_get_qap_mu_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_VO));

    /* 更新 MU EDCA条件功能寄存器 */
    hal_set_mu_edca_func_en((osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BK),
        (osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BE),
        (osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VI),
        (osal_u8)mac_mib_get_qap_mu_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VO));
}
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */

OAL_STATIC osal_void hmac_mgmt_wmm_update_edca_machw_sta_process(const hmac_vap_stru *hmac_vap)
{
    wlan_wme_ac_type_enum_uint8 ac_type;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 更新edca寄存器参数 */
    hal_vap_set_machw_aifsn_all_ac((osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BK),
        (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BE),
        (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VI),
        (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VO));

    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        hal_vap_set_edca_machw_cw((osal_u8)mac_mib_get_qap_edca_table_cwmax(hmac_vap, ac_type),
            (osal_u8)mac_mib_get_qap_edca_table_cwmin(hmac_vap, ac_type), ac_type);
    }

    /* TXOP不使能时,同步AP参数;否则使用配置值 */
    if (hmac_device->txop_enable == OSAL_FALSE) {
        hal_vap_set_machw_txop_limit_bkbe(
            (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BE),
            (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BK));
    }

    hal_vap_set_machw_txop_limit_vivo(
        (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_VO),
        (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_VI));
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_update_ht_params
 功能描述  : STA收到Beacon帧后，处理HT相关信息元素
*****************************************************************************/
osal_u32 hmac_sta_up_update_ht_params(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    osal_u16 frame_len, hmac_user_stru *hmac_user)
{
    osal_u32             change             = MAC_NO_CHANGE;
    osal_u8             *ie                = OSAL_NULL;

    ie = mac_find_ie_etc(MAC_EID_HT_OPERATION, payload, frame_len);
    if (ie != OSAL_NULL) {
        change  = hmac_proc_ht_opern_ie_etc(hmac_vap, ie, hmac_user);
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    ie = mac_find_ie_etc(MAC_EID_OBSS_SCAN, payload, frame_len);
    if (ie != OSAL_NULL) {
        /* 处理 Overlapping BSS Scan Parameters IE */
        hmac_ie_proc_obss_scan_ie_etc(hmac_vap, ie);
    } else {
        /* 找不到OBSS IE，将OBSS扫描标志置为False，放在else分支而不放在查找OBSS IE之前是为了避免之前已经置为TRUE，
           实际有OBSS IE，但在查找之前置为FALSE引入其他问题 */
        hmac_vap_set_peer_obss_scan_etc(hmac_vap, OSAL_FALSE);
    }
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */

    return change;
}
/*****************************************************************************
 函 数 名  : hmac_sta_up_update_vht_params
 功能描述  : STA收到Beacon帧后，处理VHT相关信息元素
*****************************************************************************/
osal_u32 hmac_sta_up_update_vht_params(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    osal_u16 frame_len, hmac_user_stru *hmac_user)
{
    osal_u8        *vht_opern_ie = OSAL_NULL;
    osal_u32        change = MAC_NO_CHANGE;

    /* 支持11ac，才进行后续的处理 */
    if (mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_FALSE) {
        return change;
    }

    vht_opern_ie = mac_find_ie_etc(MAC_EID_VHT_OPERN, payload, frame_len);
    if (vht_opern_ie == OSAL_NULL) {
        return change;
    }

    change = hmac_ie_proc_vht_opern_ie_etc(hmac_vap, vht_opern_ie, hmac_user);

    return change;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_timeout_sta
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_mgmt_timeout_sta(osal_void *p_arg)
{
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_mgmt_timeout_param_stru    *timeout_param = OAL_PTR_NULL;

    timeout_param = (hmac_mgmt_timeout_param_stru *)p_arg;
    if (timeout_param == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap      = (hmac_vap_stru *)mac_res_get_hmac_vap(timeout_param->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_TIMER0_OUT, timeout_param);
}

/*****************************************************************************
 函 数 名  : hmac_update_join_req_params_prot_sta
 功能描述  : 在join之前更新协议相关的参数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_update_join_req_params_prot_sta(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req)
{
    osal_void *fhook = OSAL_NULL;

    if (mac_mib_get_desired_bss_type(hmac_vap) == WLAN_MIB_DESIRED_BSSTYPE_INFRA) {
        hmac_vap_set_uapsd_cap_etc(hmac_vap, OSAL_FALSE);
        hmac_vap->cap_flag.wmm_cap   = join_req->bss_dscr.wmm_cap;
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_STA_UAPSD_GET_UAPSD_INFO);
        if (fhook != OSAL_NULL) {
            if ((((hmac_uapsd_sta_get_uapsd_info_cb)fhook)(hmac_vap->vap_id) != OSAL_NULL &&
                ((hmac_uapsd_sta_get_uapsd_info_cb)fhook)(hmac_vap->vap_id)->uapsd_switch == OSAL_TRUE) &&
                join_req->bss_dscr.uapsd_cap == OSAL_TRUE) {
                hmac_vap_set_uapsd_cap_etc(hmac_vap, OSAL_TRUE);
            }
        }
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_update_join_req_params_2040_etc(hmac_vap, &(join_req->bss_dscr));
#endif
}

/*****************************************************************************
 函 数 名  : hmac_sta_bandwidth_down_by_channel
 功能描述  : 在join之前，根据信道约束，更新带宽参数
             主要是基于协议约束，
             (0)暂时不考虑2G 40M情况，主要根据锁频能正常配置为准
             (1)165不支持80M
             (2)部分信道不支持40M,参考g_ast_pll_5g40m_info  针对锁频配置的带宽剔除
             JUMP: CH64 - CH100, 此间不可能存在40M信道
             JUMP:CH144 - CH149, 此间不存在40M信道
             JUMP:CH161 - CH184, 此间不可能存在40M信道
             int allowed[] = { 36, 44, 52, 60, 100, 108, 116, 124, 132, 140,149, 157, 184, 192 };
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_bandwidth_down_by_channel(hmac_vap_stru *hmac_vap)
{
    oal_bool_enum_uint8 bandwidth_to_20m = OAL_FALSE;

    switch (hmac_vap->channel.en_bandwidth) {
        case WLAN_BAND_WIDTH_40PLUS:
            /* 1. 64 144 161信道不支持40+ */ /* 大于等于64小于100信道：不支持40+ */
            if ((hmac_vap->channel.chan_number >= 64 && hmac_vap->channel.chan_number < 100) ||
                /* 大于等于144小于149信道：不支持40+ */
                (hmac_vap->channel.chan_number >= 144 && hmac_vap->channel.chan_number < 149) ||
                /* 大于等于161小于184信道：不支持40+ */
                (hmac_vap->channel.chan_number >= 161 && hmac_vap->channel.chan_number < 184)) {
                bandwidth_to_20m = OAL_TRUE;
            }
            break;

        case WLAN_BAND_WIDTH_40MINUS:
            /* 1. 100 149 184信道不支持40- */ /* 大于64小于等于100信道：不支持40- */
            if ((hmac_vap->channel.chan_number > 64  && hmac_vap->channel.chan_number <= 100) ||
                /* 大于144小于等于149信道：不支持40- */
                (hmac_vap->channel.chan_number > 144 && hmac_vap->channel.chan_number <= 149) ||
                /* 大于161小于等于184信道：不支持40- */
                (hmac_vap->channel.chan_number > 161 && hmac_vap->channel.chan_number <= 184)) {
                bandwidth_to_20m = OAL_TRUE;
            }
            break;

        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            /* 165信道不支持80M, 暂时不考虑出现更多信道异常问题 */
            if (hmac_vap->channel.chan_number == 165) {
                bandwidth_to_20m = OAL_TRUE;
            }
            break;

        /* 160M的带宽校验暂时不考虑 */
        default:
            break;
    }

    /* 需要降带宽 */
    if (bandwidth_to_20m == OAL_TRUE) {
        oam_warning_log3(0, OAM_SF_SCAN,
            "vap_id[%d]{hmac_sta_bandwidth_down_by_channel:channel[%d]not support bandwidth[%d],need to change to 20M}",
            hmac_vap->vap_id,
            hmac_vap->channel.chan_number, hmac_vap->channel.en_bandwidth);

        hmac_vap->channel.en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
}

/*****************************************************************************
 函 数 名  : hmac_is_rate_support_etc
 功能描述  : 判断是否支持某种速率
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_is_rate_support_etc(const osal_u8 *rates, osal_u8 rate_num, osal_u8 rate)
{
    oal_bool_enum_uint8  rate_is_supp = OAL_FALSE;
    osal_u8            loop;

    if (rates == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_is_rate_support_etc::rates null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (loop = 0; loop < rate_num; loop++) {
        if ((rates[loop] & 0x7F) == rate) {
            rate_is_supp = OAL_TRUE;
            break;
        }
    }

    return rate_is_supp;
}

/*****************************************************************************
 函 数 名  : hmac_is_support_11grate_etc
 功能描述  : 是否支持11g速率
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_support_11grate_etc(osal_u8 *rates, osal_u8 rate_num)
{
    if (rates == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_is_rate_support_etc::rates null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((hmac_is_rate_support_etc(rates, rate_num, 0x0C) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x12) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x18) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x24) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x30) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x48) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x60) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x6C) == OAL_TRUE)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


/*****************************************************************************
 函 数 名  : hmac_is_support_11brate_etc
 功能描述  : 是否支持11b速率
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
oal_bool_enum_uint8  hmac_is_support_11brate_etc(osal_u8 *rates, osal_u8 rate_num)
{
    if (rates == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_is_support_11brate_etc::rates null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((hmac_is_rate_support_etc(rates, rate_num, 0x02) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x04) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x0B) == OAL_TRUE) ||
        (hmac_is_rate_support_etc(rates, rate_num, 0x16) == OAL_TRUE)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

osal_u32 hmac_scan_get_user_protocol_etc(mac_bss_dscr_stru *bss_dscr, wlan_protocol_enum_uint8  *protocol_mode)
{
    /* 入参保护 */
    if (bss_dscr == OAL_PTR_NULL || protocol_mode == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_get_user_protocol_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (bss_dscr->he_capable == OAL_TRUE) {
        *protocol_mode = WLAN_HE_MODE;
    } else if (bss_dscr->vht_capable == OAL_TRUE) {
        *protocol_mode = WLAN_VHT_MODE;
    } else if (bss_dscr->ht_capable == OAL_TRUE) {
        *protocol_mode = WLAN_HT_MODE;
    } else if (bss_dscr->st_channel.band == WLAN_BAND_5G) {          /* 判断是否是5G */
        *protocol_mode = WLAN_LEGACY_11A_MODE;
    } else {
        if (hmac_is_support_11grate_etc(bss_dscr->supp_rates, bss_dscr->num_supp_rates) == OAL_TRUE) {
            *protocol_mode = WLAN_LEGACY_11G_MODE;
            if (hmac_is_support_11brate_etc(bss_dscr->supp_rates, bss_dscr->num_supp_rates) ==
                OAL_TRUE) {
                *protocol_mode = WLAN_MIXED_ONE_11G_MODE;
            }
        } else if (hmac_is_support_11brate_etc(bss_dscr->supp_rates, bss_dscr->num_supp_rates) ==
            OAL_TRUE) {
            *protocol_mode = WLAN_LEGACY_11B_MODE;
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_scan_get_user_protocol_etc::get user protocol failed.}");
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_get_user_protocol_etc
 功能描述  : 获取用户的协议模式
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_get_user_protocol_etc(mac_bss_dscr_stru *bss_dscr, wlan_protocol_enum_uint8  *pen_protocol_mode)
{
    /* 入参保护 */
    if (bss_dscr == OAL_PTR_NULL || pen_protocol_mode == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_get_user_protocol_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (bss_dscr->he_capable == OAL_TRUE) {
        *pen_protocol_mode = WLAN_HE_MODE;
    } else if (bss_dscr->vht_capable == OAL_TRUE)
#else
    if (bss_dscr->vht_capable == OAL_TRUE)
#endif
    {
        *pen_protocol_mode = WLAN_VHT_MODE;
#ifndef _PRE_WLAN_FEATURE_11AC2G
        if (bss_dscr->st_channel.band == WLAN_BAND_2G) {
            *pen_protocol_mode = WLAN_HT_MODE;
        }
#endif
    } else if (bss_dscr->ht_capable == OAL_TRUE) {
        *pen_protocol_mode = WLAN_HT_MODE;
    } else if (bss_dscr->st_channel.band == WLAN_BAND_5G) {          /* 判断是否是5G */
        *pen_protocol_mode = WLAN_LEGACY_11A_MODE;
    } else {
        if (hmac_is_support_11grate_etc(bss_dscr->supp_rates, bss_dscr->num_supp_rates) == OAL_TRUE) {
            *pen_protocol_mode = WLAN_LEGACY_11G_MODE;
            if (hmac_is_support_11brate_etc(bss_dscr->supp_rates, bss_dscr->num_supp_rates) ==
                OAL_TRUE) {
                *pen_protocol_mode = WLAN_MIXED_ONE_11G_MODE;
            }
        } else if (hmac_is_support_11brate_etc(bss_dscr->supp_rates, bss_dscr->num_supp_rates) ==
            OAL_TRUE) {
            *pen_protocol_mode = WLAN_LEGACY_11B_MODE;
        } else {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_get_user_protocol_etc::get user protocol failed.}");
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sta_wait_join_pre_proc(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req)
{
    osal_u32 ret;

#ifdef _PRE_WLAN_FEATURE_P2P
    ret = hmac_p2p_check_can_enter_state_etc(hmac_vap, HMAC_FSM_INPUT_ASOC_REQ);
    if (ret != OAL_SUCC) {
        /* 不能进入join状态，返回设备忙 */
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_join_etc fail,device busy: ret=%d}",
            hmac_vap->vap_id, ret);
        return OAL_ERR_CODE_CONFIG_BUSY;
    }
#endif

    /* 更新JOIN REG params 到MIB及MAC寄存器 */
    ret = hmac_sta_update_join_req_params_etc(hmac_vap, join_req);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_wait_join_etc::get hmac_sta_update_join_req_params_etc fail[%d]!}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    /* 先切换STA状态到JOIN_COMP将device从睡眠中唤醒,否则寄存器配置会被重置 */
    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_JOIN_COMP);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_wait_join_etc
 功能描述  : 处理SME发送过来的JOIN_REQ命令，启动JOIN流程，将STA状态设置为WAIT_JOIN
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_wait_join_etc(hmac_vap_stru *hmac_vap, osal_void *msg)
{
    hmac_join_req_stru                  *join_req = OAL_PTR_NULL;
    hmac_join_rsp_stru                   join_rsp;
    osal_u32                           ul_ret;

    if (hmac_vap == OAL_PTR_NULL || msg == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_wait_join_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    join_req = (hmac_join_req_stru *)msg;
    ul_ret = hmac_sta_wait_join_pre_proc(hmac_vap, join_req);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    hmac_join_set_dtim_reg_event_process(hmac_vap);

    join_rsp.result_code = HMAC_MGMT_SUCCESS;
    /* 发送JOIN成功消息给SME */
    hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_JOIN_RSP, (osal_u8 *)&join_rsp);

    oam_info_log4(0, OAM_SF_ASSOC,
        "vap_id[%d] {hmac_sta_wait_join_etc::Join AP[%08x] HT=%d VHT=%d SUCC.}",
        hmac_vap->vap_id, mac_addr(join_req->bss_dscr.bssid),
        join_req->bss_dscr.ht_capable, join_req->bss_dscr.vht_capable);

    oam_warning_log4(0, OAM_SF_ASSOC,
        "{hmac_sta_wait_join_etc::Join AP channel=%d idx=%d, bandwidth=%d Beacon Period=%d SUCC.}",
        join_req->bss_dscr.st_channel.chan_number,
        join_req->bss_dscr.st_channel.chan_idx,
        hmac_vap->channel.en_bandwidth,
        join_req->bss_dscr.beacon_period);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
/*****************************************************************************
 函 数 名  : hmac_report_ext_auth_event
 功能描述  : 上报external auth事件到wal触发wpa开启sae流程
*****************************************************************************/
osal_u32 hmac_report_ext_auth_event(hmac_vap_stru *hmac_vap)
{
    osal_u8 loop;
    hmac_external_auth_stru sta_ext_auth;
    osal_u8 *ssid = OAL_PTR_NULL;
    osal_s32 ret;
    frw_msg msg_info = {0};

    if (mac_mib_get_authentication_mode(hmac_vap) != WLAN_WITP_AUTH_SAE) {
        oam_error_log0(0, 0, "{hmac_report_ext_auth_event::wrong auth mode!}");
        return OAL_FAIL;
    }
    if (mac_mib_get_rsnaactivated(hmac_vap) == OAL_FALSE) {
        oam_error_log0(0, 0, "{hmac_report_ext_auth_event::rsn not activated!}");
        return OAL_FAIL;
    }
    (osal_void)memset_s(&sta_ext_auth, sizeof(sta_ext_auth), 0, sizeof(sta_ext_auth));

    /* 根据MIB 值，设置认证套件内容 */
    for (loop = 0; loop < MAC_AUTHENTICATION_SUITE_NUM; loop++) {
        sta_ext_auth.key_mgmt_suite = mac_mib_get_rsn_akm_suites_direct(hmac_vap, loop);
        if (sta_ext_auth.key_mgmt_suite == WLAN_AUTH_SUITE_SAE_SELECTOR) {
            break;
        }
    }
    sta_ext_auth.status = MAC_SUCCESSFUL_STATUSCODE;
    {
        ssid = mac_mib_get_desired_ssid(hmac_vap);
        sta_ext_auth.ssid_len = (osal_u32)osal_strlen((const osal_char *)ssid);
        sta_ext_auth.ssid = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, (osal_u16)sta_ext_auth.ssid_len, OSAL_TRUE);
        if (sta_ext_auth.ssid == OAL_PTR_NULL) {
            return OAL_FAIL;
        }

        if (memcpy_s(sta_ext_auth.ssid, sta_ext_auth.ssid_len, ssid, sta_ext_auth.ssid_len) != EOK) {
            oal_mem_free(sta_ext_auth.ssid, OSAL_TRUE);
            return OAL_FAIL;
        }
        if (memcpy_s(sta_ext_auth.bssid, WLAN_MAC_ADDR_LEN, hmac_vap->bssid, WLAN_MAC_ADDR_LEN) != EOK) {
            oal_mem_free(sta_ext_auth.ssid, OSAL_TRUE);
            return OAL_FAIL;
        }
    }
    msg_info.data = (osal_u8 *)&sta_ext_auth;
    msg_info.data_len = sizeof(hmac_external_auth_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_EXTERNAL_AUTH, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oal_mem_free(sta_ext_auth.ssid, OSAL_TRUE);
        return OAL_FAIL;
    }
    return (osal_u32)ret;
}
#endif

/*****************************************************************************
 功能描述  : 处理sme发来的auth req请求,优先判断Auth Alg是否是SAE，并且是否携带PMKID
*****************************************************************************/
osal_u32 hmac_sta_wait_auth(hmac_vap_stru *hmac_vap, osal_void *msg)
{
#ifdef _PRE_WLAN_FEATURE_WPA3
    osal_u32 ret;
    osal_u16  user_index = 0;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    if ((mac_mib_get_authentication_mode(hmac_vap) == WLAN_WITP_AUTH_SAE) &&
        (hmac_vap->sae_have_pmkid == OAL_FALSE)) {
        /* 更改状态 */
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2);
        ret = hmac_report_ext_auth_event(hmac_vap);
        if (ret != OAL_SUCC) {
            return ret;
        }
        hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (hmac_user == OAL_PTR_NULL) {
            oam_info_log0(0, OAM_SF_ASSOC, "{hmac_sta_wait_auth_etc::no present ap, alloc new ap.}");
            ret = hmac_user_add_etc(hmac_vap, hmac_vap->bssid, &user_index);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ASSOC, "{hmac_sta_wait_auth_etc::hmac_user_add failed[%d].}", ret);
                return ret;
            }
        }
        return OAL_SUCC;
    }
#endif
    return hmac_sta_wait_auth_etc(hmac_vap, msg);
}

OAL_STATIC osal_u32 hmac_sta_wait_auth_proc(hmac_vap_stru *hmac_vap, hmac_auth_req_stru *auth_req,
    oal_netbuf_stru *auth_frame)
{
    hmac_user_stru *hmac_user_ap = OAL_PTR_NULL;
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    osal_u16 auth_len;
    osal_u32 ret;

    /* 组seq = 1 的认证请求帧 */
    auth_len = hmac_mgmt_encap_auth_req_etc(hmac_vap, (osal_u8 *)(OAL_NETBUF_HEADER(auth_frame)));
    if (auth_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        /* 组帧失败 */
        oam_warning_log0(0, OAM_SF_AUTH, "hmac_sta_wait_auth_proc:hmac_mgmt_encap_auth_req_etc fail.");

        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        oal_netbuf_free(auth_frame);
    } else {
        oal_netbuf_put(auth_frame, auth_len);
        hmac_user_ap = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
        if (hmac_user_ap == OAL_PTR_NULL) {
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, auth_frame);
            oal_netbuf_free(auth_frame);
            oam_error_log1(0, OAM_SF_AUTH, "hmac_sta_wait_auth_proc:user[%d] null", hmac_vap->assoc_vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 为填写发送描述符准备参数 */
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(auth_frame); /* 获取cb结构体 */
        mac_get_cb_mpdu_len(tx_ctl) = auth_len; /* dmac发送需要的mpdu长度 */
        mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user_ap->assoc_id; /* 发送完成需要获取user结构体 */

        /* 如果是WEP，需要将ap的mac地址写入lut */
        if (hmac_init_security_etc(hmac_vap, hmac_user_ap->user_mac_addr) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ASSOC, "hmac_sta_wait_auth_proc:: hmac_init_security_etc failed.");
        }

        /* 抛事件让dmac将该帧发送 */
        ret = hmac_tx_mgmt_send_event_etc(hmac_vap, auth_frame, auth_len);
        if (ret != OAL_SUCC) {
            oal_netbuf_free(auth_frame);
            oam_warning_log1(0, OAM_SF_AUTH, "hmac_sta_wait_auth_proc:: hmac_tx_mgmt_send_event_etc fail[%d].", ret);
            return ret;
        }

        /* 更改状态 */
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2);

        /* 启动认证超时定时器 */
        hmac_vap->mgmt_timetout_param.state = MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2;
        hmac_vap->mgmt_timetout_param.user_index = hmac_user_ap->assoc_id;
        hmac_vap->mgmt_timetout_param.vap_id = hmac_vap->vap_id;
        frw_create_timer_entry(&hmac_vap->mgmt_timer, hmac_mgmt_timeout_sta, auth_req->timeout,
            &hmac_vap->mgmt_timetout_param, OAL_FALSE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_wait_auth_sta
 功能描述  : 处理sme发来的auth req请求。将状态置为WAIT_AUTH_SEQ2 抛事件到dmac发送
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_wait_auth_etc(hmac_vap_stru *hmac_vap, osal_void *msg)
{
    oal_netbuf_stru     *auth_frame = OAL_PTR_NULL;
    osal_u32           ul_ret;

    if ((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_sta_wait_auth_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请认证帧空间 */
    auth_frame = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (auth_frame == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_wait_auth_sta::auth_frame null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(oal_netbuf_cb(auth_frame), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    memset_s((osal_u8 *)oal_netbuf_header(auth_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    ul_ret = hmac_sta_wait_auth_proc(hmac_vap, (hmac_auth_req_stru *)msg, auth_frame);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
OAL_STATIC osal_void hmac_sta_sae_auth_proc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_req)
{
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    osal_u16 auth_seq;
    mac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    if (auth_req == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_sta_sae_auth_proc::[WPA3]netbuf is NULL}");
        return;
    }
    rx_ctrl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(auth_req);
    /* 解析auth transaction number */
    auth_seq = mac_get_auth_seq_num(oal_netbuf_header(auth_req), mac_get_rx_cb_payload_len(rx_ctrl));
    if (auth_seq > WLAN_AUTH_TRASACTION_NUM_TWO) {
        oam_warning_log1(0, OAM_SF_AUTH,
            "{hmac_sta_sae_auth_proc::[WPA3]auth recieve invalid seq, auth seq [%d]}", auth_seq);
        return;
    }
    oam_warning_log1(0, OAM_SF_AUTH, "{hmac_sta_sae_auth_proc::[WPA3]auth recieve valid seq, auth seq [%d]}", auth_seq);
    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(auth_req), (osal_u8 *)mac_addr);
    if (hmac_addr_is_zero_etc((osal_u8 *)mac_addr)) {
        oam_warning_log4(0, OAM_SF_AUTH,
            "{hmac_sta_sae_auth_proc::[WPA3]user mac:%02X:%02X:%02X:%02X:XX:XX is all 0 and invaild!}",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0 1 2 3 元素索引 */
        return;
    }
    hmac_rx_mgmt_send_to_host_etc(hmac_vap, auth_req);
    oam_warning_log4(0, OAM_SF_AUTH,
        "{hmac_sta_sae_auth_proc::[WPA3]report Auth Req to host,user mac:%02X:%02X:%02X:%02X:XX:XX.}",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]); /* 0 1 2 3元素索引 */
}
#endif

OAL_STATIC osal_u32 hmac_sta_auth_resp_proc(hmac_vap_stru *sta, osal_u16 auth_alg, hmac_auth_rsp_stru *auth_rsp,
    osal_u8 *mac_hdr)
{
    oal_netbuf_stru *auth_frame = OAL_PTR_NULL;
    hmac_user_stru *hmac_user_ap = OAL_PTR_NULL;
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;
    osal_u16 auth_frame_len;
    osal_u32 ret;

    /* auth response status_code 成功处理 */
    if (auth_alg == WLAN_WITP_AUTH_OPEN_SYSTEM) {
        /* 收到seq = 2 的认证帧，并且正常处理，取消定时器 */
        frw_destroy_timer_entry(&sta->mgmt_timer);

        /* 将状态更改为AUTH_COMP */
        hmac_fsm_change_state_etc(sta, MAC_VAP_STATE_STA_AUTH_COMP);
        auth_rsp->status_code = HMAC_MGMT_SUCCESS;

        /* 上报给SME认证成功 */
        hmac_send_rsp_to_sme_sta_etc(sta, HMAC_SME_AUTH_RSP, (osal_u8 *)auth_rsp);
    } else if (auth_alg == WLAN_WITP_AUTH_SHARED_KEY) {
        hmac_user_ap = mac_res_get_hmac_user_etc(sta->assoc_vap_id);
        if (hmac_user_ap == OAL_PTR_NULL) {
            oam_error_log2(0, OAM_SF_AUTH, "vap[%d] get user_ap null,hmac_user[%d].", sta->vap_id, sta->assoc_vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }
        /* 准备seq = 3的认证帧 */
        auth_frame = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
        if (auth_frame == OAL_PTR_NULL) {
            /* 复位mac */
            return OAL_ERR_CODE_PTR_NULL;
        }

        memset_s(oal_netbuf_cb(auth_frame), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

        auth_frame_len = hmac_mgmt_encap_auth_req_seq3_etc(sta, (osal_u8 *)OAL_NETBUF_HEADER(auth_frame), mac_hdr);
        oal_netbuf_put(auth_frame, auth_frame_len);

        /* 填写发送和发送完成需要的参数 */
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(auth_frame);
        mac_get_cb_mpdu_len(tx_ctl) = auth_frame_len;                                 /* 发送需要帧长度 */
        mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user_ap->assoc_id;   /* 发送完成要获取用户 */

        /* 抛事件给dmac发送 */
        ret = hmac_tx_mgmt_send_event_etc(sta, auth_frame, auth_frame_len);
        if (ret != OAL_SUCC) {
            oal_netbuf_free(auth_frame);
            oam_warning_log2(0, OAM_SF_AUTH, "vap_id[%d] {sta_auth_resp::mgmt_send failed[%d].}", sta->vap_id, ret);
            return ret;
        }

        /* 收到seq = 2 的认证帧，并且正常处理，取消定时器 */
        frw_destroy_timer_entry(&sta->mgmt_timer);

        /* 更改状态为MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4，并启动定时器 */
        hmac_fsm_change_state_etc(sta, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4);

        frw_create_timer_entry(&sta->mgmt_timer, hmac_mgmt_timeout_sta, sta->mgmt_timer.timeout,
            &sta->mgmt_timetout_param, OAL_FALSE);
    } else {
        /* 收到seq = 2 的认证帧，并且正常处理，取消定时器 */
        frw_destroy_timer_entry(&sta->mgmt_timer);

        /* 接收到AP 回复的auth response 中支持认证算法当前不支持的情况下，status code 却是SUCC,
            认为认证成功，并且继续出发关联 */
        oam_warning_log2(0, OAM_SF_AUTH, "vap[%d] sta_auth_resp:AP auth_alg[%d] not support!", sta->vap_id, auth_alg);
        /* 将状态更改为AUTH_COMP */
        hmac_fsm_change_state_etc(sta, MAC_VAP_STATE_STA_AUTH_COMP);
        auth_rsp->status_code = HMAC_MGMT_SUCCESS;

        /* 上报给SME认证成功 */
        hmac_send_rsp_to_sme_sta_etc(sta, HMAC_SME_AUTH_RSP, (osal_u8 *)auth_rsp);
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sta_wait_auth_seq2_rx_process(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr)
{
    osal_u32 ret;
    hmac_auth_rsp_stru auth_rsp;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    memset_s(&auth_rsp, sizeof(auth_rsp), 0, sizeof(auth_rsp));
#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_MGMT_FRAME_RX) != 0) {
        wifi_printf("[service]vap_id[%d] {hmac_sta_wait_auth_seq2_rx_process:auth_alg[%d].status[%d]}\r\n",
            hmac_vap->vap_id, mac_get_auth_alg(mac_hdr), mac_get_auth_status(mac_hdr));
    }
#endif

    auth_rsp.status_code = mac_get_auth_status(mac_hdr);
    /* 刷新auth resp返回的状态码 */
    hmac_sta_set_last_reason_code(auth_rsp.status_code);

    if (auth_rsp.status_code != MAC_SUCCESSFUL_STATUSCODE) {
        /* 收到seq = 2 的认证帧，并且正常处理，取消定时器 */
        frw_destroy_timer_entry(&hmac_vap->mgmt_timer);

        /* 上报给SME认证结果 */
        hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_AUTH_RSP, (osal_u8 *)&auth_rsp);
        oam_warning_log2(0, OAM_SF_AUTH, "vap_id[%d] {hmac_sta_wait_auth_seq2_rx_etc::AP refuse STA auth reason[%d]!}",
            hmac_vap->vap_id, auth_rsp.status_code);
        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_vap->bssid, STA_CONN_AUTH_RSP_ERR,
                USER_CONN_FAIL);
        }
        return OAL_SUCC;
    }

    ret = hmac_sta_auth_resp_proc(hmac_vap, mac_get_auth_alg(mac_hdr), &auth_rsp, mac_hdr);
    if (ret != OAL_SUCC) {
        return ret;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_sta_wait_auth_seq2_rx_etc
 功能描述  : 处理接收到seq num 等于2 的认证帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_wait_auth_seq2_rx_etc(hmac_vap_stru *hmac_vap, osal_void *msg)
{
    hmac_wlan_crx_event_stru *crx_event = OAL_PTR_NULL;
    mac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;                    /* 每一个MPDU的控制信息 */
    osal_u8 *mac_hdr = OAL_PTR_NULL;
    osal_u16 auth_alg, auth_seq, err_code;
    osal_u32 ul_ret;
    osal_bool is_protected;

    if ((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_sta_wait_auth_seq2_rx_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    crx_event = (hmac_wlan_crx_event_stru *)msg;
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(crx_event->netbuf); /* 每一个MPDU的控制信息 */
    mac_hdr = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);
    /* AUTH alg CHECK */
    auth_alg = mac_get_auth_alg(mac_hdr);
    auth_seq = mac_get_auth_seq_num(mac_hdr, mac_get_rx_cb_payload_len(pst_rx_ctrl));
    if (auth_seq < WLAN_AUTH_TRASACTION_NUM_ONE || auth_seq > WLAN_AUTH_TRASACTION_NUM_BUTT) {
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_FEATURE_WPA3
    /* MESH VAP时收到认证请求帧且认证类型为SAE 时直接发送给WPA_SUPPLICANT处理,不在此处组Auth Response帧 */
    if ((auth_alg == WLAN_WITP_AUTH_SAE) &&
        (mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT))) {
        hmac_sta_sae_auth_proc(hmac_vap, (oal_netbuf_stru *)crx_event->netbuf);
        return OAL_SUCC;
    }
#endif
    if (mac_get_frame_type_and_subtype(mac_hdr) != (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT) ||
        auth_seq != WLAN_AUTH_TRASACTION_NUM_TWO) {
        if ((mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_DEAUTH | WLAN_FC0_TYPE_MGT)) ||
            (mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_DISASSOC | WLAN_FC0_TYPE_MGT))) {
            oam_warning_log0(0, OAM_SF_RX, "{hmac_sta_wait_auth_seq2_rx_etc::recv deauth or disassoc.}");
            is_protected = mac_is_protectedframe(mac_hdr);
            err_code = OAL_MAKE_WORD16(mac_hdr[MAC_80211_FRAME_LEN], mac_hdr[MAC_80211_FRAME_LEN + 1]);
            /* 关联阶段收到deauth或者disassoc需要记录错误码 */
            hmac_sta_set_last_reason_code(err_code);
            hmac_sta_rx_deauth_req(hmac_vap, mac_hdr, is_protected);
        }
        return OAL_SUCC;
    }

    if ((mac_mib_get_authentication_mode(hmac_vap) != auth_alg) &&
        (mac_mib_get_authentication_mode(hmac_vap) != WLAN_WITP_AUTH_AUTOMATIC)) {
        oam_warning_log3(0, OAM_SF_AUTH, "vap_id[%d] {hmac_sta_wait_auth_seq2_rx_etc::rcv unexpected auth alg[%d/%d].}",
            hmac_vap->vap_id,
            auth_alg, mac_mib_get_authentication_mode(hmac_vap));
        /* 接收到auth 后，继续往下执行，避免vap 状态一直为WAIT_AUTH_SEQ2 */
    }
    ul_ret = hmac_sta_wait_auth_seq2_rx_process(hmac_vap, mac_hdr);
    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
OAL_STATIC osal_u32 hmac_sta_wait_asoc_rx_fail_handle_for_sae(hmac_vap_stru *hmac_vap, hmac_asoc_rsp_stru asoc_rsp)
{
    if ((hmac_vap->sae_have_pmkid == OAL_FALSE) ||
        (asoc_rsp.status_code == MAC_SUCCESSFUL_STATUSCODE)) {
        return OAL_SUCC;
    }
    asoc_rsp.result_code = HMAC_MGMT_REFUSED;
    /* 发送关联结果给SME */
    hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_ASOC_RSP, (osal_u8 *)&asoc_rsp);
    return OAL_FAIL;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_sta_wait_auth_seq4_rx_etc
 功能描述  : 处理收到seq = 4 的认证帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_wait_auth_seq4_rx_etc(hmac_vap_stru *hmac_vap, osal_void *p_msg)
{
    hmac_wlan_crx_event_stru *crx_event = OAL_PTR_NULL;
    mac_rx_ctl_stru            *pst_rx_ctrl = OAL_PTR_NULL; /* 每一个MPDU的控制信息 */
    osal_u8                   *mac_hdr = OAL_PTR_NULL;
    hmac_auth_rsp_stru           auth_rsp;

    if ((p_msg == OAL_PTR_NULL) || (hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_sta_wait_auth_seq2_rx_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&auth_rsp, sizeof(auth_rsp), 0, sizeof(auth_rsp));

    crx_event = (hmac_wlan_crx_event_stru *)p_msg;
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(crx_event->netbuf); /* 每一个MPDU的控制信息 */
    mac_hdr = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);
    if (mac_get_frame_type_and_subtype(mac_hdr) == (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT)) {
        if ((mac_get_auth_seq_num(mac_hdr, mac_get_rx_cb_payload_len(pst_rx_ctrl)) == WLAN_AUTH_TRASACTION_NUM_FOUR) &&
            (mac_get_auth_status(mac_hdr) == MAC_SUCCESSFUL_STATUSCODE)) {
            /* 接收到seq = 4 且状态位为succ 取消定时器 */
            frw_destroy_timer_entry(&hmac_vap->mgmt_timer);

            auth_rsp.status_code = HMAC_MGMT_SUCCESS;

            /* 更改sta状态为MAC_VAP_STATE_STA_AUTH_COMP */
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_AUTH_COMP);
            oam_info_log1(0, OAM_SF_AUTH, "vap_id[%d] {hmac_sta_wait_auth_seq4_rx_etc::auth succ.}",
                hmac_vap->vap_id);
            /* 将认证结果上报SME */
            hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_AUTH_RSP, (osal_u8 *)&auth_rsp);
        } else {
            oam_warning_log2(0, OAM_SF_AUTH, "vap_id[%d] {hmac_sta_wait_auth_seq4_rx_etc::transaction num.status[%d]}",
                hmac_vap->vap_id, mac_get_auth_status(mac_hdr));
            /* 等待定时器超时 */
        }

#ifdef _PRE_WLAN_DFT_STAT
        if ((dfx_get_service_control() & SERVICE_CONTROL_MGMT_FRAME_RX) != 0) {
            wifi_printf("[service]vap[%d] {hmac_sta_wait_auth_seq4_rx_etc::transaction num.status[%d]}\r\n",
                hmac_vap->vap_id, mac_get_auth_status(mac_hdr));
        }
#endif
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sta_wait_asoc_create_frame(hmac_vap_stru *hmac_vap, oal_netbuf_stru *asoc_req_frame,
    osal_u32 *asoc_frame_len)
{
    osal_u8 *curr_bssid = OAL_PTR_NULL;
    osal_u32 asoc_req_frame_len;

    /* 组帧 (Re)Assoc_req_Frame */
    if (hmac_vap->reassoc_flag != 0) {
        curr_bssid = hmac_vap->bssid;
    } else {
        curr_bssid = OAL_PTR_NULL;
    }
    asoc_req_frame_len = hmac_mgmt_encap_asoc_req_sta_etc(hmac_vap, (osal_u8 *)(OAL_NETBUF_HEADER(asoc_req_frame)),
        curr_bssid, hmac_vap->bssid);
    oal_netbuf_put(asoc_req_frame, asoc_req_frame_len);
    *asoc_frame_len = asoc_req_frame_len;

    if (asoc_req_frame_len == 0) {
        oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_asoc_etc::hmac_mgmt_encap_asoc_req_sta_etc null.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (osal_unlikely(asoc_req_frame_len < OAL_ASSOC_REQ_IE_OFFSET)) {
        oam_error_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_asoc_etc::invalid asoc_req_ie_len[%u].}",
            hmac_vap->vap_id, asoc_req_frame_len);
        dft_report_params_etc((osal_u8 *)oal_netbuf_header(asoc_req_frame), (osal_u16)asoc_req_frame_len,
            SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sta_wait_asoc_set_ie(hmac_user_stru *hmac_user_ap, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *asoc_req_frame, osal_u32 asoc_frame_len)
{
    mac_tx_ctl_stru *tx_ctl = OAL_PTR_NULL;

    hmac_user_free_asoc_req_ie(hmac_user_ap);
    if (hmac_user_set_asoc_req_ie(hmac_user_ap, OAL_NETBUF_HEADER(asoc_req_frame) + OAL_ASSOC_REQ_IE_OFFSET,
        asoc_frame_len - OAL_ASSOC_REQ_IE_OFFSET, (osal_u8)(hmac_vap->reassoc_flag)) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_asoc_etc::hmac_user_set_asoc_req_ie failed}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(asoc_req_frame);

    mac_get_cb_mpdu_len(tx_ctl)   = (osal_u16)asoc_frame_len;
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user_ap->assoc_id;

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_sta_wait_asoc_fill_timeout_timer_param(hmac_vap_stru *hmac_vap,
    const hmac_user_stru *hmac_user)
{
    hmac_vap->mgmt_timetout_param.state = MAC_VAP_STATE_STA_WAIT_ASOC;
    hmac_vap->mgmt_timetout_param.user_index = hmac_user->assoc_id;
    hmac_vap->mgmt_timetout_param.vap_id = hmac_vap->vap_id;
    hmac_vap->mgmt_timetout_param.status_code = MAC_AUTH_TIMEOUT;
}

/*****************************************************************************
 函 数 名  : hmac_sta_wait_asoc_etc
 功能描述  : 在AUTH_COMP状态接收到SME发过来的ASOC_REQ请求，将STA状态设置为WAIT_ASOC,
             抛事件给DMAC，发送Asoc_req_frame
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_wait_asoc_etc(hmac_vap_stru *hmac_vap, osal_void *msg)
{
    oal_netbuf_stru            *asoc_req_frame = OAL_PTR_NULL;
    hmac_user_stru             *hmac_user = OAL_PTR_NULL;
    osal_u32                  asoc_frame_len;
    osal_u32                  ul_ret;

    if ((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_wait_asoc_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    asoc_req_frame = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (asoc_req_frame == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_asoc_etc::asoc_req_frame null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(oal_netbuf_cb(asoc_req_frame), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    /* 将mac header清零 */
    memset_s((osal_u8 *)oal_netbuf_header(asoc_req_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    if (hmac_sta_wait_asoc_create_frame(hmac_vap, asoc_req_frame, &asoc_frame_len) != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, asoc_req_frame);
        oal_netbuf_free(asoc_req_frame);
        return OAL_FAIL;
    }

    /* Should we change the ie buff from local mem to netbuf ? */
    /* 此处申请的内存，只在上报给内核后释放 */
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, asoc_req_frame);
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_asoc_etc::hmac_user_ap null.}",
            hmac_vap->vap_id);
        oal_netbuf_free(asoc_req_frame);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_sta_wait_asoc_set_ie(hmac_user, hmac_vap, asoc_req_frame, asoc_frame_len) != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, asoc_req_frame);
        oal_netbuf_free(asoc_req_frame);
        return OAL_FAIL;
    }

    /* 抛事件让DMAC将该帧发送 */
    ul_ret = hmac_tx_mgmt_send_event_etc(hmac_vap, asoc_req_frame, (osal_u16)asoc_frame_len);
    if (ul_ret != OAL_SUCC) {
        oal_netbuf_free(asoc_req_frame);
        hmac_user_free_asoc_req_ie(hmac_user);
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_asoc_etc::hmac_tx_mgmt_send_event_etc failed[%d]}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    /* 更改状态 */
    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_WAIT_ASOC);

    /* 启动关联超时定时器, 为对端ap分配一个定时器，如果超时ap没回asoc rsp则启动超时处理 */
    hmac_sta_wait_asoc_fill_timeout_timer_param(hmac_vap, hmac_user);
    frw_create_timer_entry(&(hmac_vap->mgmt_timer), hmac_mgmt_timeout_sta,
        ((hmac_asoc_req_stru *)msg)->assoc_timeout, &(hmac_vap->mgmt_timetout_param), OAL_FALSE);

    return OAL_SUCC;
}

#if defined(_PRE_WLAN_FEATURE_HS20) || defined(_PRE_WLAN_FEATURE_P2P)
OAL_STATIC osal_void hmac_sta_not_up_rx_send_action(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    osal_u8 *data, hmac_wlan_crx_event_stru *mgmt_rx_event)
{
    oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_sta_not_up_rx_send_action:: send to host.}", hmac_vap->vap_id);
    hmac_rx_mgmt_send_to_host_etc(hmac_vap, mgmt_rx_event->netbuf);
}

/*****************************************************************************
 函 数 名  : hmac_sta_not_up_rx_mgmt_etc
 功能描述  : P2P_DEVICE 在监听状态接收到管理帧处理，HS2.0查询过程的ACTION上报(原函数名为hmac_p2p_listen_rx_mgmt)
 输入参数  : hmac_vap_stru  *hmac_vap_sta,
             osal_void       *p_param
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_not_up_rx_mgmt_etc(hmac_vap_stru *hmac_vap_sta, osal_void *p_param)
{
    hmac_wlan_crx_event_stru   *mgmt_rx_event;
    mac_rx_ctl_stru            *rx_info;
    osal_u8                  *mac_hdr;
    osal_u8                  *data;

    if (hmac_vap_sta == OAL_PTR_NULL || p_param == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_sta_not_up_rx_mgmt_etc::PTR_NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_rx_event  = (hmac_wlan_crx_event_stru *)p_param;
    rx_info        = (mac_rx_ctl_stru *)oal_netbuf_cb(mgmt_rx_event->netbuf);
    mac_hdr        = (osal_u8 *)mac_get_rx_cb_mac_header_addr(rx_info);
    if (mac_hdr == OAL_PTR_NULL) {
        oam_error_log3(0, OAM_SF_RX,
            "{hmac_sta_not_up_rx_mgmt_etc::mac_hdr null, vap_id %d,frame_len %d, mac_header_len %d}",
            rx_info->vap_id, rx_info->frame_len, rx_info->mac_header_len);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取帧体指针 */
    data = (osal_u8 *)mac_get_rx_cb_mac_header_addr(rx_info) + rx_info->mac_header_len;
    /* STA在NOT UP状态下接收到各种管理帧处理 */
    switch (mac_get_frame_type_and_subtype(mac_hdr)) {
        /* 判断接收到的管理帧类型 */
        case WLAN_FC0_SUBTYPE_PROBE_REQ | WLAN_FC0_TYPE_MGT:
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 判断为P2P设备,则上报probe req帧到wpa_supplicant */
            if (!is_legacy_vap(hmac_vap_sta)) {
                hmac_rx_mgmt_send_to_host_etc(hmac_vap_sta, mgmt_rx_event->netbuf);
            }
            break;
#endif
        case WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT:
                hmac_sta_not_up_rx_send_action(hmac_vap_sta, mac_hdr, data, mgmt_rx_event);
            break;
        case WLAN_FC0_SUBTYPE_PROBE_RSP | WLAN_FC0_TYPE_MGT:
            /* 如果是probe response帧，则直接上报wpa_supplicant */
            hmac_rx_mgmt_send_to_host_etc(hmac_vap_sta, mgmt_rx_event->netbuf);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_HS20 and _PRE_WLAN_FEATURE_P2P */

/*****************************************************************************
 函 数 名  : hmac_update_vht_opern_ie_sta
 功能描述  : STA更新接收asoc_rsp帧中的Vht operation ie信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_update_vht_opern_ie_sta(
    hmac_vap_stru            *hmac_vap,
    hmac_user_stru          *hmac_user,
    osal_u8               *payload)
{
    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) ||
        (payload == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_update_vht_opern_ie_sta::param null.}");
        return MAC_NO_CHANGE;
    }

    /* 支持11ac，才进行后续的处理 */
    if (mac_mib_get_vht_option_implemented(hmac_vap) == OAL_FALSE) {
        return MAC_NO_CHANGE;
    }

    return hmac_ie_proc_vht_opern_ie_etc(hmac_vap, payload, hmac_user);
}

OAL_STATIC osal_void hmac_sta_up_parse_qos_capability_handle_ie(osal_u8 *ie, hmac_vap_stru *hmac_vap,
    mac_ieee80211_frame_stru *frame_header, osal_char *ssid_list, osal_u8 ssid_len)
{
    #ifdef _PRE_WLAN_FEATURE_11AX
    mac_frame_he_mu_qos_info_stru *qos_info = OSAL_NULL;
    #endif
    osal_u8 *ie_tmp = ie;

    if (ie_tmp != OSAL_NULL) {
        ie_tmp += MAC_IE_HDR_LEN;
#ifdef _PRE_WLAN_FEATURE_11AX
        qos_info = (mac_frame_he_mu_qos_info_stru *)ie_tmp;

        /* 如果param_set_count没有改变, 则STA也不用做任何改变，直接返回即可 */
        if (qos_info->edca_update_count == hmac_vap->he_mu_edca_update_count) {
            return;
        }
#endif
        hmac_scan_send_probe_req_frame(hmac_vap, frame_header->address3,
            (const osal_char *)ssid_list, ssid_len, OAL_TRUE);

        return;
    }
}
/*****************************************************************************
 函 数 名 : hmac_sta_up_parse_qos_capability
 功能描述     : STA解析qos capability信息并做相应处理
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_up_parse_qos_capability(osal_u8 *payload, osal_u16 msg_len, hmac_vap_stru *hmac_vap,
    osal_u8 *mac_hdr)
{
    osal_u8 *ie = OSAL_NULL;

    mac_ieee80211_frame_stru                *frame_header = OSAL_NULL;
    osal_u8                                 *ssid = OSAL_NULL;
    osal_u8                                  ssid_len = 0;
    osal_char                                ssid_list[WLAN_SSID_MAX_LEN];
    osal_u8                                  mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8                                  frame_subtype;
    hmac_device_stru                        *hmac_device = OSAL_NULL;
    hmac_scanned_bss_info                   *old_scanned_bss = OSAL_NULL;
    hmac_bss_mgmt_stru                      *bss_mgmt = OSAL_NULL;

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OSAL_NULL) {
        return;
    }

    frame_header = (mac_ieee80211_frame_stru *)mac_hdr;
    oal_set_mac_addr((osal_u8 *)mac_addr, frame_header->address2);
    frame_subtype = mac_get_frame_sub_type(mac_hdr);
    if (frame_subtype == WLAN_FC0_SUBTYPE_ASSOC_RSP) {
        bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
        old_scanned_bss = hmac_scan_find_scanned_bss_by_bssid_etc(bss_mgmt, (const osal_u8 *)mac_addr);
        if (old_scanned_bss == OSAL_NULL) {
            return;
        }
        ssid_len = (osal_u8)osal_strlen(old_scanned_bss->bss_dscr_info.ac_ssid);
        if (memcpy_s(ssid_list, sizeof(ssid_list), old_scanned_bss->bss_dscr_info.ac_ssid, ssid_len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_up_parse_qos_capability::memcpy_s ASSOC_RSP ssid error}");
        }
        ssid_list[ssid_len] = '\0';
    } else if (frame_subtype == WLAN_FC0_SUBTYPE_BEACON) {
        ssid = hmac_get_ssid_etc(payload, (osal_s32)msg_len, &ssid_len);
        if (memcpy_s(ssid_list, sizeof(ssid_list), ssid, ssid_len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_up_parse_qos_capability::memcpy_s BEACON ssid error}");
        }
        ssid_list[ssid_len] = '\0';
    }
    ie = mac_find_ie_etc(MAC_EID_QOS_CAP, payload, msg_len);
    hmac_sta_up_parse_qos_capability_handle_ie(ie, hmac_vap, frame_header, (osal_char *)ssid_list, ssid_len);
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 函 数 名  : hmac_update_he_opern_ie_sta
 功能描述  : STA更新接收asoc_rsp帧中的he operation ie信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_update_he_opern_ie_sta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload)
{
    if (osal_unlikely((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (payload == OAL_PTR_NULL) ||
        (hmac_vap->hal_vap == OAL_PTR_NULL) || (hmac_vap->hal_device == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_update_he_opern_ie_sta::param null.}");
        return MAC_NO_CHANGE;
    }

    /* 支持11ax，才进行后续的处理 */
    if (mac_mib_get_he_option_implemented(hmac_vap) == OAL_FALSE) {
        return MAC_NO_CHANGE;
    }

    if ((hmac_vap->p2p_mode != WLAN_P2P_DEV_MODE) && (hmac_vap->p2p_mode != WLAN_P2P_CL_MODE)) {
        hal_set_11ax_vap_idx(hmac_vap->hal_vap->vap_id);
    }
#ifdef BOARD_ASIC_WIFI
    oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_update_he_opern_ie_sta flush mac clk -> 160M}", hmac_vap->vap_id);
    hmac_vap->hal_device->ax_vap_cnt++;
    /* AX模式需要160M时钟保证业务功能正常 */
    hal_update_mac_freq(hmac_vap->hal_device, WLAN_BAND_WIDTH_20M);
    hmac_hal_device_sync(hmac_vap->hal_device);
#endif

    return hmac_ie_proc_he_opern_ie(hmac_vap, payload, hmac_user);
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_update_he_edca_params_mib
 功能描述  : STA更新每一个AC的参数
 输入参数  : hmac_vap:处于sta模式的vap
             payload :帧体
 输出参数  : 无
 返 回 值  : OAL_VOID
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_up_update_he_edca_params_mib(hmac_vap_stru *hmac_vap,
    mac_frame_he_mu_ac_parameter_stru *ac_param)
{
    osal_u8               aci;
    osal_u8               mu_edca_timer;
    osal_u32              txop_limit;

    /* ****** AC Parameters Record Format ******* */
    /* ------------------------------------------ */
    /* |     1     |       1       |      1     | */
    /* ------------------------------------------ */
    /* | ACI/AIFSN | ECWmin/ECWmax | MU EDCA Timer | */
    /* ------------------------------------------ */

    /************* ACI/AIFSN Field ***************/
    /*     ---------------------------------- */
    /* bit |   4   |  1  |  2  |    1     |   */
    /*     ---------------------------------- */
    /*     | AIFSN | ACM | ACI | Reserved |   */
    /*     ---------------------------------- */

    aci   = ac_param->ac_index;

    /* ECWmin/ECWmax Field */
    /*     ------------------- */
    /* bit |   4    |   4    | */
    /*     ------------------- */
    /*     | ECWmin | ECWmax | */
    /*     ------------------- */

    /* 在mib库中和寄存器里保存的TXOP值都是以us为单位的
       ==MU EDCA Timer 单位是8TU --8 * 1024us
    */
    mu_edca_timer = ac_param->mu_edca_timer;
    txop_limit = (osal_u32)(mu_edca_timer);

    /* 更新相应的MIB库信息 */
    if (aci < WLAN_WME_AC_BUTT) {
        mac_mib_set_qap_mu_edca_table_cwmin(hmac_vap, aci, ac_param->ecw_min);
        mac_mib_set_qap_mu_edca_table_cwmax(hmac_vap, aci, ac_param->ecw_max);
        mac_mib_set_qap_mu_edca_table_aifsn(hmac_vap, aci, ac_param->aifsn);
        mac_mib_set_qap_mu_edca_table_txop_limit(hmac_vap, aci, txop_limit);
        mac_mib_set_QAPMUEDCATableMandatory(hmac_vap, aci, ac_param->acm);
    }
}


/*****************************************************************************
 函 数 名  : hmac_sta_up_update_he_edca_params
 功能描述  : STA接收到beacon帧或者关联响应帧更新自身的EDCA参数，涉及到mib值
             和寄存器
 输入参数  : payload :帧体
             msg_len  :帧长度
             info_elem_offset :当前指向的帧体位置
             hmac_vap        :指向hmac_vap的指针，vap是sta模式
             frame_sub_type   :帧的次类型
 输出参数  : 无
 返 回 值  : osal_void
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_up_update_he_edca_params(
    osal_u8               *payload,
    osal_u16               msg_len,
    hmac_vap_stru           *hmac_vap,
    osal_u8                frame_sub_type,
    hmac_user_stru          *hmac_user)
{
    osal_u8                                ac_num_loop;
    osal_u32                               ul_ret;
    hmac_device_stru                        *hmac_device = OAL_PTR_NULL;
    osal_u8                               *ie = OAL_PTR_NULL;
    mac_frame_he_mu_edca_parameter_ie_stru   mu_edca_value;
    unref_param(hmac_user);
    if (mac_mib_get_he_option_implemented(hmac_vap) == OAL_FALSE) {
        return;
    }

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "{hmac_sta_up_update_he_edca_params::hmac_res_get_mac_dev_etc fail.device_id[%u]}",
            hmac_vap->device_id);
        return;
    }

    /************************ MU EDCA Parameter Set Element ***************************/
    /* ------------------------------------------------------------------------------------------- */
    /* | EID | LEN | Ext EID|MU Qos Info |MU AC_BE Parameter Record | MU AC_BK Parameter Record  | */
    /* ------------------------------------------------------------------------------------------- */
    /* |  1  |  1  |   1    |    1       |     3                    |        3                   | */
    /* ------------------------------------------------------------------------------------------- */
    /* ------------------------------------------------------------------------------ -------------*/
    /* | MU AC_VI Parameter Record | MU AC_VO Parameter Record                                   | */
    /* ------------------------------------------------------------------------------------------- */
    /* |    3                      |     3                                                       | */

    /******************* QoS Info field when sent from WMM AP *****************/
    /* --------------------------------------------------------------------------------------------*/
    /*    | EDCA Parameter Set Update Count | Q-Ack | Queue Request |TXOP Request | More Data Ack| */
    /*---------------------------------------------------------------------------------------------*/
    /* bit |        0~3                      |  1    |  1            |   1         |     1       | */
    /*---------------------------------------------------------------------------------------------*/
    /**************************************************************************/

    ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_EDCA, payload, msg_len);
    if (ie != OAL_PTR_NULL) {
        memset_s(&mu_edca_value, OAL_SIZEOF(mu_edca_value), 0, OAL_SIZEOF(mu_edca_value));
        ul_ret = hmac_ie_parse_mu_edca_parameter(ie, &mu_edca_value);
        if (ul_ret != OAL_SUCC) {
            return;
        }

        /* 如果收到的是beacon帧，并且param_set_count没有改变
           则STA也不用做任何改变，直接返回即可 */
        if ((frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) &&
            (mu_edca_value.qos_info.edca_update_count ==
            hmac_vap->he_mu_edca_update_count)) {
            return;
        }

        hmac_vap->he_mu_edca_update_count = mu_edca_value.qos_info.edca_update_count;

        /* 针对每一个AC，更新EDCA参数 */
        for (ac_num_loop = 0; ac_num_loop < WLAN_WME_AC_BUTT; ac_num_loop++) {
            hmac_sta_up_update_he_edca_params_mib(hmac_vap,
                &(mu_edca_value.mu_ac_parameter[ac_num_loop]));
        }

        /* 更新EDCA相关的MAC寄存器 */
        hmac_sta_up_update_mu_edca_params_machw(hmac_vap, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA,
            hmac_vap->he_mu_edca_update_count);
        return;
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_update_mu_edca_params_machw
 功能描述  : 当与STA关联的AP不是QoS的时候，STA默认采用VO策略发送数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : osal_void
*****************************************************************************/
osal_u32 hmac_sta_up_update_mu_edca_params_machw(hmac_vap_stru *hmac_vap, mac_wmm_set_param_type_enum_uint8 type,
    osal_u8 qos_info_count)
{
    wlan_mib_dot11qapedcaentry_stru wlan_mib_qap_edac[WLAN_WME_AC_BUTT];

    memset_s(&wlan_mib_qap_edac, sizeof(wlan_mib_qap_edac), 0, sizeof(wlan_mib_qap_edac));

    if (type != MAC_WMM_SET_PARAM_TYPE_DEFAULT) {
        if (memcpy_s((osal_u8 *)&wlan_mib_qap_edac, sizeof(wlan_mib_qap_edac),
            (osal_u8 *)hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac,
            (OAL_SIZEOF(wlan_mib_dot11qapedcaentry_stru) * WLAN_WME_AC_BUTT)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_up_update_mu_edca_params_machw::memcpy_s error}");
        }
    }

    hmac_vap->he_mu_edca_update_count = qos_info_count;

    if (type == MAC_WMM_SET_PARAM_TYPE_DEFAULT) {
        return OAL_SUCC;
    }

    memcpy_s((osal_u8 *)&hmac_vap->wlan_mib_mu_edca.wlan_mib_qap_edac,
        (sizeof(wlan_mib_dot11qapedcaentry_stru) * WLAN_WME_AC_BUTT), (osal_u8 *)&wlan_mib_qap_edac,
        (sizeof(wlan_mib_dot11qapedcaentry_stru) * WLAN_WME_AC_BUTT));

    /* 根据获取的事件，更新EDCA参数寄存器 */
    hmac_update_mu_edca_machw_sta(hmac_vap);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_sta_up_update_edca_params_machw_etc
 功能描述  : 当与STA关联的AP不是QoS的时候，STA默认采用VO策略发送数据
 输入参数  : 无
 输出参数  : 无
 返 回 值  : osal_void
*****************************************************************************/
osal_u32 hmac_sta_up_update_edca_params_machw_etc(hmac_vap_stru *hmac_vap, mac_wmm_set_param_type_enum_uint8 type)
{
    wlan_mib_dot11qapedcaentry_stru wlan_mib_qap_edac[WLAN_WME_AC_BUTT];
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    memset_s(&wlan_mib_qap_edac, sizeof(wlan_mib_qap_edac), 0, sizeof(wlan_mib_qap_edac));

    if (type != MAC_WMM_SET_PARAM_TYPE_DEFAULT) {
        if (memcpy_s((osal_u8 *)&wlan_mib_qap_edac, sizeof(wlan_mib_qap_edac),
            (osal_u8 *)&hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac,
            (OAL_SIZEOF(wlan_mib_dot11qapedcaentry_stru) * WLAN_WME_AC_BUTT)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_up_update_edca_params_machw_etc::memcpy_s error}");
        }
    }

    if (type == MAC_WMM_SET_PARAM_TYPE_DEFAULT) {
        return OAL_SUCC;
    }

    (osal_void)memcpy_s((osal_u8 *)hmac_vap->mib_info->wlan_mib_edca.wlan_mib_qap_edac,
        (sizeof(wlan_mib_dot11qapedcaentry_stru) * WLAN_WME_AC_BUTT), (osal_u8 *)&wlan_mib_qap_edac,
        (sizeof(wlan_mib_dot11qapedcaentry_stru) * WLAN_WME_AC_BUTT));
    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_WMM,
            "vap_id[%d] {hmac_sta_up_update_edca_params_machw_etc:: hal device null!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_enable_machw_edca();
    hmac_mgmt_wmm_update_edca_machw_sta_process(hmac_vap);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_sta_up_update_edca_params_mib
 功能描述  : STA更新每一个AC的参数
 输入参数  : hmac_vap:处于sta模式的vap
             payload :帧体
 输出参数  : 无
 返 回 值  : OAL_VOID
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_up_update_edca_params_mib(hmac_vap_stru  *hmac_vap, const osal_u8  *payload)
{
    osal_u8               aifsn;
    osal_u8               aci;
    osal_u8               ecwmin;
    osal_u8               ecwmax;
    osal_u16              txop_limit;
    oal_bool_enum_uint8     acm;
    /* ******* AC Parameters Record Format ******** */
    /* ------------------------------------------ */
    /* |     1     |       1       |      2     | */
    /* ------------------------------------------ */
    /* | ACI/AIFSN | ECWmin/ECWmax | TXOP Limit | */
    /* ------------------------------------------ */

    /* ************ ACI/AIFSN Field ************** */
    /*     ---------------------------------- */
    /* bit |   4   |  1  |  2  |    1     |   */
    /*     ---------------------------------- */
    /*     | AIFSN | ACM | ACI | Reserved |   */
    /*     ---------------------------------- */

    aifsn = payload[0] & MAC_WMM_QOS_PARAM_AIFSN_MASK;
    acm   = ((payload[0] & BIT4) != 0) ? OSAL_TRUE : OSAL_FALSE;
    aci   = (payload[0] >> MAC_WMM_QOS_PARAM_ACI_BIT_OFFSET) & MAC_WMM_QOS_PARAM_ACI_MASK;

    /* ECWmin/ECWmax Field */
    /*     ------------------- */
    /* bit |   4    |   4    | */
    /*     ------------------- */
    /*     | ECWmin | ECWmax | */
    /*     ------------------- */
    ecwmin = (payload[1] & MAC_WMM_QOS_PARAM_ECWMIN_MASK);
    ecwmax = ((payload[1] & MAC_WMM_QOS_PARAM_ECWMAX_MASK) >> MAC_WMM_QOS_PARAM_ECWMAX_BIT_OFFSET);

    /* 在mib库中和寄存器里保存的TXOP值都是以us为单位的，但是传输的时候是以32us为
       单位进行传输的，因此在解析的时候需要将解析到的值乘以32 */
    /* 2、3：取payload有效位 */
    txop_limit = payload[2] | ((payload[3] & MAC_WMM_QOS_PARAM_TXOPLIMIT_MASK) <<
        MAC_WMM_QOS_PARAM_BIT_NUMS_OF_ONE_BYTE);
    txop_limit = (osal_u16)(txop_limit << MAC_WMM_QOS_PARAM_TXOPLIMIT_SAVE_TO_TRANS_TIMES);

    /* 更新相应的MIB库信息 */
    if (aci < WLAN_WME_AC_BUTT) {
        mac_mib_set_qap_edca_table_cwmin(hmac_vap, aci, ecwmin);
        mac_mib_set_qap_edca_table_cwmax(hmac_vap, aci, ecwmax);
        mac_mib_set_qap_edca_table_aifsn(hmac_vap, aci, aifsn);
        mac_mib_set_qap_edca_table_txop_limit(hmac_vap, aci, txop_limit);
        mac_mib_set_qap_edca_table_mandatory(hmac_vap, aci, acm);
    }
}

OAL_STATIC osal_void hmac_sta_up_update_edca_wmm_ie(hmac_vap_stru *hmac_vap, osal_u8 frame_sub_type,
    hmac_user_stru *hmac_user, osal_u8 *ie, hmac_device_stru *hmac_device)
{
    osal_u8 edca_param_set;
    osal_u8 param_set_cnt;
    osal_u16 msg_offset;
    osal_u8 ac_num_loop;

    /* 解析wmm ie是否携带EDCA参数 */
    edca_param_set = ie[MAC_OUISUBTYPE_WMM_PARAM_OFFSET];
    param_set_cnt = ie[HMAC_WMM_QOS_PARAMS_HDR_LEN] & 0x0F;

    /* 如果收到的是beacon帧，并且param_set_count没有改变，说明AP的WMM参数没有变
        则STA也不用做任何改变，直接返回即可 */
    if ((frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) &&
        (param_set_cnt == hmac_vap->uc_wmm_params_update_count)) {
        return;
    }

    hmac_device->en_wmm = OAL_TRUE;

    if (frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) {
        /* 保存QoS Info */
        hmac_vap_set_wmm_params_update_count_etc(hmac_vap, param_set_cnt);
    }

    if ((ie[HMAC_WMM_QOS_PARAMS_HDR_LEN] & BIT7) != 0) {
        hmac_user_set_apsd_etc(hmac_user, OAL_TRUE);
    } else {
        hmac_user_set_apsd_etc(hmac_user, OAL_FALSE);
    }

    msg_offset = (HMAC_WMM_QOSINFO_AND_RESV_LEN + HMAC_WMM_QOS_PARAMS_HDR_LEN);

    /* wmm ie中不携带edca参数 直接返回 */
    if (edca_param_set != MAC_OUISUBTYPE_WMM_PARAM) {
        return;
    }

    /* 针对每一个AC，更新EDCA参数 */
    for (ac_num_loop = 0; ac_num_loop < WLAN_WME_AC_BUTT; ac_num_loop++) {
        hmac_sta_up_update_edca_params_mib(hmac_vap, &ie[msg_offset]);
        msg_offset += HMAC_WMM_AC_PARAMS_RECORD_LEN;
    }

    /* 更新EDCA相关的MAC寄存器 */
    hmac_sta_up_update_edca_params_machw_etc(hmac_vap, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA);

    return;
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_update_edca_params_etc
 功能描述  : STA接收到beacon帧或者关联响应帧更新自身的EDCA参数，涉及到mib值
             和寄存器
 输入参数  : payload :帧体
             msg_len  :帧长度
             info_elem_offset :当前指向的帧体位置
             hmac_vap        :指向hmac_vap的指针，vap是sta模式
             frame_sub_type   :帧的次类型
 输出参数  : 无
 返 回 值  : osal_void
*****************************************************************************/
osal_void hmac_sta_up_update_edca_params_etc(osal_u8 *payload, osal_u16 msg_len, hmac_vap_stru *hmac_vap,
    osal_u8 frame_sub_type, hmac_user_stru *hmac_user)
{
    osal_u32              ul_ret;
    hmac_device_stru       *hmac_device = OAL_PTR_NULL;
    osal_u8              *ie = OAL_PTR_NULL;

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "{hmac_sta_up_update_edca_params_etc::hmac_res_get_mac_dev_etc fail.device_id[%u]!}",
            hmac_vap->device_id);
        return;
    }

    /************************ WMM Parameter Element ***************************/
    /* ------------------------------------------------------------------------------ */
    /* | EID | LEN | OUI |OUI Type |OUI Subtype |Version |QoS Info |Resd |AC Params | */
    /* ------------------------------------------------------------------------------ */
    /* |  1  |  1  |  3  |    1    |     1      |    1   |    1    |  1  |    16    | */
    /* ------------------------------------------------------------------------------ */

    /******************* QoS Info field when sent from WMM AP *****************/
    /*        --------------------------------------------                    */
    /*          | Parameter Set Count | Reserved | U-APSD |                   */
    /*          --------------------------------------------                  */
    /*   bit    |        0~3          |   4~6    |   7    |                   */
    /*          --------------------------------------------                  */
    /**************************************************************************/
    ie = hmac_get_wmm_ie_etc(payload, msg_len);
    if (ie != OAL_PTR_NULL) {
        hmac_sta_up_update_edca_wmm_ie(hmac_vap, frame_sub_type, hmac_user, ie, hmac_device);

        return;
    }

    ie = mac_find_ie_etc(MAC_EID_HT_CAP, payload, msg_len);
    if (ie != OAL_PTR_NULL) {
        /* 再查找HT CAP能力第2字节BIT 5 short GI for 20M 能力位 */
        /* 2:查找HT CAP能力第2字节BIT 5 short GI for 20M 能力位 */
        if ((ie[1] >= 2) && ((ie[2] & BIT5) > 0)) {
            hmac_vap_init_wme_param_etc(hmac_vap);
            hmac_device->en_wmm = OAL_TRUE;
            /* 更新EDCA相关的MAC寄存器 */
            hmac_sta_up_update_edca_params_machw_etc(hmac_vap, MAC_WMM_SET_PARAM_TYPE_UPDATE_EDCA);

            return;
        }
    }

    if (frame_sub_type == WLAN_FC0_SUBTYPE_ASSOC_RSP) {
        /* 当与STA关联的AP不是QoS的，STA会去使能EDCA寄存器，并默认利用VO级别发送数据 */
        ul_ret = hmac_sta_up_update_edca_params_machw_etc(hmac_vap, MAC_WMM_SET_PARAM_TYPE_DEFAULT);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ASSOC,
                "vap_id[%d] {hmac_sta_up_update_edca_params_etc::hmac_sta_up_update_edca_params_machw_etc failed[%d].}",
                hmac_vap->vap_id, ul_ret);
        }

        hmac_device->en_wmm = OAL_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_find_ie_in_probe_rsp_etc
 功能描述  : 在已保存的probe rsp中寻找指定IE，一般用于在asoc rsp中寻找IE失败时
             再在probe rsp做进一步查找
 输入参数  : hmac_vap : mac vap结构体
             eid: 要查找的EID
 输出参数  : payload: probe rsp帧体，以帧内第一个IE开头
             us_index:目标IE在payload中的相对位置
*****************************************************************************/
osal_u8 *hmac_sta_find_ie_in_probe_rsp_etc(hmac_vap_stru *hmac_vap, osal_u8 eid, osal_u16 *pus_index)
{
    mac_bss_dscr_stru                  *bss_dscr = OAL_PTR_NULL;
    osal_u8                          *ie = OAL_PTR_NULL;
    osal_u8                          *payload = OAL_PTR_NULL;
    osal_u8                           offset;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{find ie fail, hmac_vap is null.}");
        return OAL_PTR_NULL;
    }

    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap, hmac_vap->bssid);
    if (bss_dscr == OAL_PTR_NULL) {
        oam_warning_log4(0, OAM_SF_CFG,
            "hmac_sta_find_ie_in_probe_rsp_etc::find the bss failed by bssid:%02X:%02X:%02X:%02X:XX:XX",
            /* 0:1:2:3:数组下标 */
            hmac_vap->bssid[0], hmac_vap->bssid[1], hmac_vap->bssid[2], hmac_vap->bssid[3]);

        return OAL_PTR_NULL;
    }

    /* 以IE开头的payload，返回供调用者使用 */
    offset = MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    payload = (osal_u8 *)(bss_dscr->mgmt_buff + offset);
    if (bss_dscr->mgmt_len < offset) {
        return OAL_PTR_NULL;
    }

    ie = mac_find_ie_etc(eid, payload, (osal_s32)(bss_dscr->mgmt_len - offset));
    if (ie == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    /* IE长度初步校验 */
    if (*(ie + 1) == 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{IE[%d] len in probe rsp is 0, find ie fail.}", eid);
        return OAL_PTR_NULL;
    }

    *pus_index = (osal_u16)(ie - payload);

    oam_warning_log1(0, OAM_SF_ANY, "{found ie[%d] in probe rsp.}", eid);

    return payload;
}

/*****************************************************************************
 函 数 名  : hmac_sta_check_ht_cap_ie
 功能描述  : 在STA为WAIT_ASOC状态时，解析ht cap IE，分别在asoc rsp和probe rsp
             中查找
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_check_ht_cap_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload, hmac_user_stru *hmac_user,
    osal_u16 *pus_amsdu_maxsize, osal_u16 payload_len)
{
    osal_u8 *ie = OAL_PTR_NULL;
    osal_u8 *ht_cap_chk = OAL_PTR_NULL;
    osal_u16 ht_cap_index;

    if ((hmac_vap == OAL_PTR_NULL) || (payload == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL)) {
        return;
    }

    ie = mac_find_ie_etc(MAC_EID_HT_CAP, payload, payload_len);
    if ((ie == OAL_PTR_NULL) || (ie[1] < MAC_HT_CAP_LEN)) {
        ht_cap_chk = hmac_sta_find_ie_in_probe_rsp_etc(hmac_vap, MAC_EID_HT_CAP, &ht_cap_index);
        if (ht_cap_chk == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_sta_check_ht_cap_ie::ht_cap_chk is null.}");
            return;
        }
        if (ht_cap_chk[ht_cap_index + 1] < MAC_HT_CAP_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_check_ht_cap_ie::invalid ht cap len[%d].}",
                ht_cap_chk[ht_cap_index + 1]);
            return;
        }
        ie = ht_cap_chk + ht_cap_index;      /* 赋值HT CAP IE */
    } else {
        if (ie < payload) {
            return;
        }
        ht_cap_index = (osal_u16)(ie - payload);
        ht_cap_chk = payload;
    }

    /* RX MCS速率集均不支持，则认为MCS 速率集异常，设置对端HT 能力置为不支持 */
    /* 5: rx mcs stream 1偏移量; 6: rx mcs stream 2偏移量; 7: rx mcs stream 3偏移量; 8: rx mcs stream 4偏移量 */
    if ((ie[5] == 0) && (ie[6] == 0) && (ie[7] == 0) && (ie[8] == 0)) {
        oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_update_ht_cap:: invalid mcs set, disable HT.}",
            hmac_vap->vap_id);
        hmac_user_set_ht_capable_etc(hmac_user, OAL_FALSE);
        return;
    }
    /* 支持HT, 默认初始化 */

    /* 根据协议值设置特性，必须在hmac_amsdu_init_user后面调用 */
    mac_ie_proc_ht_sta_etc(hmac_vap, ht_cap_chk, ht_cap_index, hmac_user, pus_amsdu_maxsize);
}

/*****************************************************************************
 函 数 名  : hmac_sta_check_ext_cap_ie
 功能描述  : 在STA为WAIT_ASOC状态时，解析ext cap IE，分别在asoc rsp和probe rsp
             中查找
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_check_ext_cap_ie(hmac_vap_stru    *hmac_vap,
    hmac_user_stru   *hmac_user,
    osal_u8       *payload,
    osal_u16       rx_len)
{
    osal_u8       *ie;
    osal_u8       *payload_proc = OAL_PTR_NULL;
    osal_u16       us_index;

    ie = mac_find_ie_etc(MAC_EID_EXT_CAPS, payload, rx_len);
    if ((ie == OAL_PTR_NULL) || (ie[1] < MAC_MIN_XCAPS_LEN)) {
        payload_proc = hmac_sta_find_ie_in_probe_rsp_etc(hmac_vap, MAC_EID_EXT_CAPS, &us_index);
        if (payload_proc == OAL_PTR_NULL) {
            return;
        }

        if (payload_proc[us_index + 1] < MAC_MIN_XCAPS_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_check_ext_cap_ie::invalid ext cap len[%d].}",
                             payload_proc[us_index + 1]);
            return;
        }
    } else {
        payload_proc = payload;
        if (ie < payload) {
            return;
        }

        us_index = (osal_u16)(ie - payload);
    }

    /* 处理 Extended Capabilities IE */
    hmac_ie_proc_ext_cap_ie_etc(hmac_user, &payload_proc[us_index]);
}

/*****************************************************************************
 函 数 名  : hmac_sta_check_ht_opern_ie
 功能描述  : 在STA为WAIT_ASOC状态时，解析ht opern IE，分别在asoc rsp和probe rsp
             中查找
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_check_ht_opern_ie(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload, osal_u16 rx_len)
{
    osal_u8       *ie;
    osal_u8       *payload_proc = OAL_PTR_NULL;
    osal_u16       us_index;
    osal_u32       change = MAC_NO_CHANGE;

    ie = mac_find_ie_etc(MAC_EID_HT_OPERATION, payload, rx_len);
    if ((ie == OAL_PTR_NULL) || (ie[1] < MAC_HT_OPERN_LEN)) {
        payload_proc = hmac_sta_find_ie_in_probe_rsp_etc(hmac_vap, MAC_EID_HT_OPERATION, &us_index);
        if (payload_proc == OAL_PTR_NULL) {
            return change;
        }

        if (payload_proc[us_index + 1] < MAC_HT_OPERN_LEN) {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_check_ht_opern_ie::invalid ht cap len[%d].}",
                             payload_proc[us_index + 1]);
            return change;
        }
    } else {
        payload_proc = payload;
        if (ie < payload) {
            return change;
        }

        us_index = (osal_u16)(ie - payload);
    }

    change |= hmac_proc_ht_opern_ie_etc(hmac_vap, &payload_proc[us_index], hmac_user);

    return change;
}

/*****************************************************************************
 函 数 名  : hmac_ie_check_ht_sta
 功能描述  : 在STA为WAIT_ASOC状态时，解析asoc rsp 或者reasoc rsp frame，更新相关参数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ie_check_ht_sta(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 rx_len,
    hmac_user_stru *hmac_user, osal_u16 *pus_amsdu_maxsize)
{
    osal_u32 change = MAC_NO_CHANGE;

    if ((hmac_vap == OAL_PTR_NULL) || (payload == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL)) {
        return change;
    }

    /* 初始化HT cap为FALSE，入网时会把本地能力跟随AP能力 */
    hmac_user_set_ht_capable_etc(hmac_user, OAL_FALSE);

    /* 至少支持11n才进行后续的处理 */
    if (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OAL_FALSE) {
        return change;
    }

    hmac_sta_check_ht_cap_ie(hmac_vap, payload, hmac_user, pus_amsdu_maxsize, rx_len);
    /* sta处理AP的 Extended Capability */
    hmac_sta_check_ext_cap_ie(hmac_vap, hmac_user, payload, rx_len);

    change = hmac_sta_check_ht_opern_ie(hmac_vap, hmac_user, payload, rx_len);

    return change;
}

OAL_STATIC osal_void hmac_process_sta_up_update(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *mac_hdr,
    osal_u8 *payload, osal_u16 msg_len)
{
    osal_u32 ret;
    osal_u8 *tmp_ie = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    /* 初始化安全端口过滤参数 */
    ret = hmac_init_user_security_port_etc(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ASSOC, "{hmac_process_sta_up_update::init port failed[%d].}", ret);
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    /* STA模式下的pmf能力来源于WPA_supplicant，只有启动pmf和不启动pmf两种类型 */
    hmac_user_set_pmf_active_etc(hmac_user, hmac_vap->user_pmf_cap);
    oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_process_sta_up_update::set user pmf[%d].}",
        hmac_vap->vap_id, hmac_vap->user_pmf_cap);
#endif

    /* sta更新自身的edca parameters, assoc rsp帧是管理帧 */
    hmac_sta_up_update_edca_params_etc(payload, msg_len, hmac_vap, mac_get_frame_sub_type(mac_hdr), hmac_user);

#ifdef _PRE_WLAN_FEATURE_11AX
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_SR_UPDATE_IE_INFO);
    if (fhook != OSAL_NULL) {
        ((hmac_sta_up_process_spatial_reuse_ie_cb)fhook)(hmac_vap, payload, msg_len, OSAL_TRUE);
    }

    hmac_sta_up_update_he_edca_params(payload, msg_len, hmac_vap, mac_get_frame_sub_type(mac_hdr), hmac_user);

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_NFRP_UPDATE_PARAMS);
    if (fhook != OSAL_NULL) {
        ((hmac_nfrp_sta_update_param_cb)fhook)(payload, msg_len, hmac_vap, mac_get_frame_sub_type(mac_hdr), hmac_user);
    }
#endif

    hmac_sta_up_parse_qos_capability(payload, msg_len, hmac_vap, mac_hdr);

    /* 更新关联用户的 QoS protocol table */
    hmac_mgmt_update_assoc_user_qos_table_etc(payload, msg_len, hmac_user);

    /* 更新关联用户的legacy速率集合 */
    hmac_user_init_rates_etc(hmac_user);
    ret = hmac_ie_proc_assoc_user_legacy_rate(payload, msg_len, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC, "vapid[%d]{hmac_process_sta_up_update:rates err[%d]}", hmac_vap->vap_id, ret);
    }

    /* 更新对应STA的RRM能力 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11K_STA_UPDATE_RRM_CAP);
    if (fhook != OSAL_NULL) {
        ((hmac_sta_up_update_rrm_capability_cb)fhook)(hmac_vap, hmac_user, payload, msg_len);
    }

    /* 更新11ac VHT capabilities ie */
    (osal_void)memset_s(&(hmac_user->vht_hdl), OAL_SIZEOF(mac_vht_hdl_stru), 0, OAL_SIZEOF(mac_vht_hdl_stru));
    tmp_ie = mac_find_ie_etc(MAC_EID_VHT_CAP, payload, msg_len);
    if (tmp_ie != OAL_PTR_NULL) {
        hmac_proc_vht_cap_ie_etc(hmac_vap, hmac_user, tmp_ie);
#ifdef _PRE_WLAN_FEATURE_1024QAM
        tmp_ie = hmac_find_vendor_ie_etc(MAC_EXT_VENDER_IE, MAC_OSAL_1024QAM_IE, payload, msg_len);
        if (tmp_ie != OAL_PTR_NULL) {
            hmac_user->cap_info.cap_1024qam = OAL_TRUE;
        }
#endif
    }
}

OAL_STATIC osal_void hmac_process_update_virtual_multi_sta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *mac_hdr, osal_u16 hdr_len, osal_u16 msg_len)
{
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    /* vap开启4地址能力并调用check接口校验用户是否支持4地址 是则将用户的4地址传输能力位置1 */
    if (hmac_vap->wds_table.wds_vap_mode == WDS_MODE_REPEATER_STA) {
        if (hmac_vmsta_check_user_a4_support(mac_hdr, hdr_len + msg_len)) {
            hmac_user->is_wds = OAL_TRUE;
            oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_ap_up_rx_asoc_req::user surpport 4 address.}",
                hmac_user->vap_id);
        }
    }
#else
    unref_param(hmac_vap);
    unref_param(hmac_user);
    unref_param(mac_hdr);
    unref_param(hdr_len);
    unref_param(msg_len);
#endif // _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
}

/*****************************************************************************
 功能描述  : 根据 HE element 更新 mac_he_hdl->he_capable 的值
*****************************************************************************/
OAL_STATIC osal_void hmac_process_update_he_capable(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *payload, osal_u16 msg_len)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    osal_u8              *tmp_ie = OAL_PTR_NULL;
    mac_he_hdl_stru      *mac_he_hdl = &(hmac_user->he_hdl);

    (osal_void)memset_s(mac_he_hdl, sizeof(mac_he_hdl_stru), 0, sizeof(mac_he_hdl_stru));

    tmp_ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP, payload, msg_len);
    if (tmp_ie != OAL_PTR_NULL) {
        /* 更新 he_capable 的值为 TRUE */
        hmac_proc_he_cap_ie(hmac_vap, hmac_user, tmp_ie);
    }

    oam_warning_log4(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_process_update_he_capable::ht_cap[%d] vht_cap[%d] he_cap[%d]}",
        hmac_vap->vap_id, hmac_user->ht_hdl.ht_capable, hmac_user->vht_hdl.vht_capable, mac_he_hdl->he_capable);
#else
    unref_param(hmac_vap);
    unref_param(hmac_user);
    unref_param(payload);
    unref_param(msg_len);
#endif
}

OAL_STATIC osal_void hmac_process_update_ie(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u32 *change,
    osal_u8 *payload, osal_u16 msg_len)
{
    osal_u8 *tmp_ie = OAL_PTR_NULL;
    osal_u32 vht_ie_offset = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;
    osal_u8 *vht_ie = OAL_PTR_NULL;

    /* 更新11ac VHT operation ie */
    tmp_ie = mac_find_ie_etc(MAC_EID_VHT_OPERN, payload, msg_len);
    if (tmp_ie != OAL_PTR_NULL) {
        *change |= hmac_update_vht_opern_ie_sta(hmac_vap, hmac_user, tmp_ie);
    }

    /* 更新 HT 参数  以及 EXTEND CAPABILITY */
    (osal_void)memset_s(&(hmac_user->ht_hdl), OAL_SIZEOF(mac_user_ht_hdl_stru), 0,
        OAL_SIZEOF(mac_user_ht_hdl_stru));
    *change |= hmac_ie_check_ht_sta(hmac_vap, payload, msg_len, hmac_user, &hmac_user->amsdu_maxsize);

    /* 支持HT后再设置VHT能力 */
    if (hmac_user_ht_support(hmac_user) != OAL_TRUE) {
        /* 解决STA从11ax AP漫游到11a AP ping不通网关问题:配置he_capable以设置正确的user protocol_mode */
        hmac_process_update_he_capable(hmac_vap, hmac_user, payload, msg_len);
        return;
    }
    /* 更新 11ax HE Capabilities ie */
#ifdef _PRE_WLAN_FEATURE_11AX
    (osal_void)memset_s(&(hmac_user->he_hdl), OAL_SIZEOF(mac_he_hdl_stru), 0,
        OAL_SIZEOF(mac_he_hdl_stru));
    tmp_ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_CAP, payload, msg_len);
    if (tmp_ie != OAL_PTR_NULL) {
        hmac_proc_he_cap_ie(hmac_vap, hmac_user, tmp_ie);
    }

    tmp_ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_OPERATION, payload, msg_len);
    if (tmp_ie != OAL_PTR_NULL) {
        *change |= hmac_update_he_opern_ie_sta(hmac_vap, hmac_user, tmp_ie);
    }

    /* 解析BSS Coloe change announcement IE */
    tmp_ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT, payload, msg_len);
    if (tmp_ie != OAL_PTR_NULL) {
        hmac_proc_he_bss_color_change_announcement_ie(hmac_vap, hmac_user, tmp_ie);
    }
#endif

    /* 根据BRCM VENDOR OUI 适配2G 11AC */
    if (hmac_user->vht_hdl.vht_capable != OAL_FALSE) {
        return;
    }
    vht_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE, payload, msg_len);
    if ((vht_ie != OAL_PTR_NULL) && (vht_ie[1] >= vht_ie_offset)) {
        oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_process_update_ie::enable hidden support_11ac2g}",
            hmac_vap->vap_id);

        /* 进入此函数代表user支持2G 11ac */
        tmp_ie = mac_find_ie_etc(MAC_EID_VHT_CAP, vht_ie + vht_ie_offset,
            (osal_s32)(vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
        if (tmp_ie != OAL_PTR_NULL) {
            hmac_proc_vht_cap_ie_etc(hmac_vap, hmac_user, tmp_ie);
        }

        /* 更新11ac VHT operation ie */
        tmp_ie = mac_find_ie_etc(MAC_EID_VHT_OPERN, vht_ie + vht_ie_offset,
            (osal_s32)(vht_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER));
        if (tmp_ie != OAL_PTR_NULL) {
            *change |= hmac_update_vht_opern_ie_sta(hmac_vap, hmac_user, tmp_ie);
        }
    }
}

OAL_STATIC osal_void hmac_process_update_user(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u32 *change)
{
    osal_u32 ret;
    wlan_channel_bandwidth_enum_uint8 bandwidth;
    osal_u8 avail_mode;

    /* 获取用户的协议模式 */
    hmac_set_user_protocol_mode_etc(hmac_vap, hmac_user);

    avail_mode = g_auc_avail_protocol_mode_etc[hmac_vap->protocol][hmac_user->protocol_mode];
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((hmac_vap->protocol == WLAN_HT_MODE) && (hmac_user->protocol_mode == WLAN_VHT_MODE) &&
        (hmac_vap->cap_flag.support_11ac2g == OAL_TRUE) && (hmac_vap->channel.band == WLAN_BAND_2G)) {
        avail_mode = WLAN_VHT_MODE;
    }
#endif

    oam_warning_log4(0, OAM_SF_ASSOC, "{hmac_process_update_user: protocol[%d],mode[%d],11ac2g[%d],avail_mode[%d]}",
        hmac_vap->protocol, hmac_user->protocol_mode, hmac_vap->cap_flag.support_11ac2g, avail_mode);

    hmac_vap->rx_ampduplusamsdu_active = hmac_get_host_rx_ampdu_amsdu();

    /* 获取用户与VAP协议模式交集 */
    hmac_user_set_avail_protocol_mode_etc(hmac_user, avail_mode);
    hmac_user_set_cur_protocol_mode_etc(hmac_user, hmac_user->avail_protocol_mode);

    /* 获取用户和VAP 可支持的11a/b/g 速率交集 */
    hmac_vap_set_user_avail_rates_etc(hmac_vap, hmac_user);

    /* 获取用户和VAP带宽，并判断是否有带宽变化需要通知硬件切带宽 */
    bandwidth = hmac_vap_get_ap_usr_opern_bandwidth(hmac_vap, hmac_user);
    *change |= hmac_vap_set_bw_check(hmac_vap, bandwidth);

    /* 同步用户的带宽能力 */
    hmac_user_set_bandwidth_info_etc(hmac_user, hmac_vap_bw_mode_to_bw(bandwidth), hmac_vap_bw_mode_to_bw(bandwidth));

    /* 获取用户与VAP空间流交集 */
    ret = hmac_user_set_avail_num_space_stream_etc(hmac_user, hmac_vap->vap_rx_nss);
    oam_warning_log1(0, OAM_SF_ASSOC, "hmac_process_update_user:ap max nss[%d]", hmac_user->user_num_spatial_stream);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_process_update_user::mac_user_set_avail_num_space_stream failed[%d].}",
            hmac_vap->vap_id, ret);
    }

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 根据smps更新空间流能力 */
    if (!is_vap_single_nss(hmac_vap) && !is_user_single_nss(hmac_user)) {
        hmac_smps_update_user_status(hmac_vap, hmac_user);
    }
#endif
}

OAL_STATIC osal_void hmac_process_assoc_rsp_syn(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *payload,
    osal_u16 msg_len)
{
    osal_u32 ret;
    unref_param(payload);
    unref_param(msg_len);

    ret = hmac_config_user_info_syn_etc(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_process_assoc_rsp_syn::hmac_syn_vap_state failed[%d].}",
            hmac_vap->vap_id, ret);
    }

    ret = hmac_config_user_rate_info_syn_etc(hmac_vap, hmac_user);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_process_assoc_rsp_syn::hmac_syn_rate_info failed[%d].}",
            hmac_vap->vap_id, ret);
    }

    /* dmac offload架构下，同步STA USR信息到dmac */
    ret = hmac_config_sta_vap_info_syn_etc(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_process_assoc_rsp_syn::hmac_syn_vap_state failed[%d].}",
            hmac_vap->vap_id, ret);
    }

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* sta计算自身的partial aid，写入到mac寄存器 */
    hmac_sta_set_txopps_partial_aid(hmac_vap, hmac_user);
#endif
}

OAL_STATIC osal_void hmac_process_assoc_rsp_sync_cap(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const osal_u32 change)
{
    hmac_chan_sync_etc(hmac_vap, hmac_vap->channel.chan_number, hmac_vap->channel.en_bandwidth, OAL_TRUE);
    oam_warning_log4(0, OAM_SF_ASSOC, "hmac_process_assoc_rsp_sync_cap:change BW.change[0x%x] band[%d]chnl[%d]bw[%d]",
        change, hmac_vap->channel.band, hmac_vap->channel.chan_number, mac_vap_get_cap_bw(hmac_vap));
    oam_warning_log4(0, OAM_SF_ASSOC, "hmac_process_assoc_rsp_sync_cap:mac usr id[%d] bw cap[%d]avil[%d]cur[%d]",
        hmac_user->assoc_id, hmac_user->bandwidth_cap, hmac_user->avail_bandwidth, hmac_user->cur_bandwidth);
}

/* 根据关联响应更新本地数据 */
osal_u32 hmac_process_assoc_rsp_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *mac_hdr,
    osal_u16 hdr_len, osal_u8 *payload, osal_u16 msg_len)
{
    osal_u16 aid;
    osal_u32 change = MAC_NO_CHANGE;
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    osal_u32 ret;
#endif
    osal_void *fhook = OSAL_NULL;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_process_assoc_rsp_etc::hmac_sta null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新关联ID */
    aid = mac_get_asoc_id(payload);
    if ((aid > 0) && (aid <= 2007)) { /* 2007为aid最大有效范围 */
        hmac_vap_set_aid_etc(hmac_vap, aid);
    } else {
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_process_assoc_rsp_etc::invalid sta_aid[%d].}",
            hmac_vap->vap_id, aid);
    }

    payload += (MAC_CAP_INFO_LEN + MAC_STATUS_CODE_LEN + MAC_AID_LEN);
    msg_len -= (MAC_CAP_INFO_LEN + MAC_STATUS_CODE_LEN + MAC_AID_LEN);

    hmac_process_sta_up_update(hmac_vap, hmac_user, mac_hdr, payload, msg_len);
    hmac_process_update_virtual_multi_sta(hmac_vap, hmac_user, mac_hdr, hdr_len, msg_len);
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_PROCESS_UPDATE_INFO);
    if (fhook != OSAL_NULL) {
        ((hmac_process_update_twt_cb)fhook)(hmac_vap, hmac_user, payload, msg_len);
    }
    hmac_process_update_ie(hmac_vap, hmac_user, &change, payload, msg_len);
    hmac_process_update_user(hmac_vap, hmac_user, &change);

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ACHBA_UPDATE_PROCESS);
    if (fhook != OSAL_NULL) {
        ((hmac_process_update_achba_cb)fhook)(hmac_vap, hmac_user, payload, msg_len);
    }

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 处理Operating Mode Notification 信息元素 */
    ret = hmac_check_opmode_notify_etc(hmac_vap, mac_hdr, payload, msg_len, hmac_user);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_process_assoc_rsp_etc::hmac_check_opmode_notify_etc failed[%d].}", hmac_vap->vap_id, ret);
    }
#endif
    hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_ASSOC);
    hmac_process_assoc_rsp_syn(hmac_vap, hmac_user, payload, msg_len);

    /* 以下函数将产生回环，下抛到dmac后，会再次上抛到hmac覆盖user结构体中的带宽信息，故先更新user结构体中的信息 */
    if ((MAC_BW_CHANGE & change) != 0) {
        /* 获取用户与VAP带宽能力交集,通知硬件切带宽 */
        hmac_process_assoc_rsp_sync_cap(hmac_vap, hmac_user, change);
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void set_cfg_mode_param_by_channel(mac_cfg_mode_param_stru *prot_param,
    hmac_vap_stru *hmac_vap, mac_channel_stru *channel_dst, wlan_protocol_enum_uint8 protocol)
{
    prot_param->channel_idx = channel_dst->chan_number;
    prot_param->en_bandwidth   = channel_dst->en_bandwidth;
    prot_param->band        = channel_dst->band;

    // use input protocol if exists
    prot_param->protocol = (protocol >= WLAN_PROTOCOL_BUTT) ? hmac_vap->protocol : protocol;
}

/*****************************************************************************
 函 数 名  : hmac_sta_sync_vap
 功能描述  : repeater关联rootap时，根据sta的能力同步repeater下面的ap(up,down)的信道及模式
 输入参数  : hmac_vap_stru *hmac_vap, mac_channel_stru *channel,wlan_protocol_enum_uint8 protocol
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_sync_vap(hmac_vap_stru *hmac_vap, mac_channel_stru *channel, wlan_protocol_enum_uint8 protocol)
{
    hmac_device_stru                    *hmac_device = OAL_PTR_NULL;
    osal_u32                             ret;
    osal_s32                             ret_set_freq;
    mac_cfg_mode_param_stru              prot_param;
    mac_channel_stru                    *channel_dst = OAL_PTR_NULL;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device指针 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_INIT);
        oam_error_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_sta_sync_vap::hmac_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    channel_dst = channel ? channel : &hmac_vap->channel;

    set_cfg_mode_param_by_channel(&prot_param, hmac_vap, channel_dst, protocol);

    ret = hmac_is_channel_num_valid_etc(prot_param.band, prot_param.channel_idx);
    if (ret != OAL_SUCC) {
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_INIT);
        oam_warning_log1(0, OAM_SF_SCAN, "hmac_sta_sync_vap::hmac_is_channel_num_valid_etc failed[%d].", ret);
        return ret;
    }

    oam_warning_log3(0, OAM_SF_SCAN, "hmac_sta_sync_vap::AP Starting network in Channel: %d, bw: %d protocol: %d.",
        prot_param.channel_idx, prot_param.en_bandwidth, prot_param.protocol);

#ifdef _PRE_WLAN_FEATURE_DBAC
    /* 同时更改多个VAP的信道，此时需要强制清除记录 */
    /* 若启动了DBAC，则按照原始流程进行 */
    if (!mac_is_dbac_enabled(hmac_device)) {
#endif
        hmac_device->max_channel = 0;
        hmac_device->max_bandwidth = WLAN_BAND_WIDTH_BUTT;
#ifdef _PRE_WLAN_FEATURE_DBAC
    }
#endif
    // force channel setting is required
    hmac_vap->channel = *channel_dst;

    msg_info.data = (osal_u8 *)&prot_param.channel_idx;
    msg_info.data_len = OAL_SIZEOF(osal_u8);
    ret_set_freq = hmac_config_set_freq_etc(hmac_vap, &msg_info);
    if (ret_set_freq != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_SCAN, "hmac_sta_sync_vap::hmac_config_set_freq_etc fail[%d].", ret_set_freq);
        return (osal_u32)ret_set_freq;
    }

    ret = hmac_config_set_mode_etc(hmac_vap, (osal_u16)OAL_SIZEOF(prot_param), (osal_u8 *)&prot_param);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_SCAN, "hmac_sta_sync_vap::hmac_config_set_mode_etc failed[%d].", ret);
    }

    return ret;
}

#ifndef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 函 数 名  : hmac_sta_sync_bss_freq
 功能描述  : 运行过程中更新up状态的BSS的信道及模式
 输入参数  : hmac_vap_stru *hmac_vap,
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_sync_bss_freq(hmac_vap_stru *hmac_vap, mac_channel_stru *channel,
    wlan_protocol_enum_uint8 protocol)
{
    mac_cfg_mib_by_bw_param_stru cfg;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    if (hmac_ap_clean_bss_etc(hmac_vap) == OAL_SUCC) {
        cfg.band      = channel->band;
        cfg.en_bandwidth = channel->en_bandwidth;

        /* 设置AP侧状态机为 WAIT_START */
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_AP_WAIT_START);

        /* 更新bss能力信息 */
        msg_info.data = (osal_u8 *)&cfg;
        msg_info.data_len = (osal_u16)OAL_SIZEOF(cfg);
        hmac_config_set_mib_by_bw(hmac_vap, &msg_info);
        return hmac_chan_start_bss_etc(hmac_vap, channel, protocol);
    }

    return OAL_FAIL;
}

OAL_STATIC osal_void hmac_sta_sync_bss_set_band(hmac_vap_stru *hmac_vap, hmac_vap_stru *hmac_vap_tmp,
    hmac_device_stru *hmac_device, mac_channel_stru *mac_channel, wlan_protocol_enum_uint8 *protocol)
{
    unref_param(hmac_device);
    // 本地BSS为WEP加密，rootap为HT使能，则本地BSS维持协议模式不变
    if ((mac_is_wep_enabled(hmac_vap_tmp) == OAL_TRUE) &&
        (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OAL_TRUE)) {
        *protocol = hmac_vap_tmp->protocol;
        oam_warning_log1(0, OAM_SF_CHAN,
            "vap_id[%d] {hmac_sta_sync_bss_freq_all::keep protocol of while wep && HT}",
            hmac_vap_tmp->vap_id);
    } else {
        *protocol = hmac_vap->protocol;
    }

    if (hmac_vap_tmp->bw_fixed != 0) {
        // 固定带宽模式，AP带宽不跟随STA的带宽模式
        mac_channel->en_bandwidth = hmac_vap_tmp->channel.en_bandwidth;
        oam_warning_log2(0, OAM_SF_CHAN,
            "vap_id[%d] {hmac_sta_sync_bss_freq_all::bw_fixed set,keep bw[%d]}",
            hmac_vap_tmp->vap_id, mac_channel->en_bandwidth);
    } else {
        oam_warning_log3(0, OAM_SF_CHAN,
            "vap_id[%d] {bw_fixed not set, bw[%d] should sync to new bw[%d]!}",
            hmac_vap_tmp->vap_id,
            hmac_vap_tmp->channel.en_bandwidth,
            mac_channel->en_bandwidth);
        // Auto带宽模式，但是是WEP加密模式，只支持20M带宽，所以AP带宽也不跟随
        if (mac_is_wep_enabled(hmac_vap_tmp)) {
            mac_channel->en_bandwidth = hmac_vap_tmp->channel.en_bandwidth;
            oam_warning_log2(0, OAM_SF_CHAN,
                "vap_id[%d] {hmac_sta_sync_bss_freq_all::keep bw[%d] due to WEP mode!}",
                hmac_vap_tmp->vap_id,  mac_channel->en_bandwidth);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_sync_bss_freq_all
 功能描述  : STA关联成功时同步信道至相同device下的其他BSS
             参考alg_dbac_start_auto函数
 输入参数  : hmac_vap_stru *hmac_vap,
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_sync_bss_freq_all(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_vap_stru    *hmac_vap_tmp = OAL_PTR_NULL;
    osal_u8         idx;
    mac_channel_stru  channel;
    wlan_protocol_enum_uint8 protocol;

    if (memcpy_s((void *)&channel, sizeof(channel), (void *)&hmac_vap->channel, sizeof(mac_channel_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_sync_bss_freq_all::memcpy_s error}");
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CHAN, "{hmac_sta_sync_bss_freq_all::null mac_dev}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (idx = 0; idx < hmac_device->vap_num; idx++) {
        hmac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL || (hmac_vap_tmp == hmac_vap) ||
            (hmac_vap_tmp->vap_mode != WLAN_VAP_MODE_BSS_AP)) {
            continue;
        }

        if ((hmac_vap_tmp->channel.band != channel.band) ||
            ((hmac_vap_tmp->channel.en_bandwidth == channel.en_bandwidth) &&
            (hmac_vap_tmp->channel.chan_number == channel.chan_number))) {
            continue;
        }

        // sync channel if on same band while channel or bandwidth not same
        hmac_sta_sync_bss_set_band(hmac_vap, hmac_vap_tmp, hmac_device, &channel, &protocol);

        oam_warning_log4(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_sta_sync_bss_freq_all:: mac_vap_pro[%d], cfg_pro[%d], sta_vap_pro[%d].}",
            hmac_vap_tmp->vap_id, hmac_vap_tmp->protocol, protocol,
            hmac_vap->protocol);

        // 如果是down的vap，则同步信道和协议模式
        if (hmac_vap_tmp->vap_state == MAC_VAP_STATE_INIT) {
            if (hmac_sta_sync_vap(hmac_vap_tmp, &channel, protocol) != OAL_SUCC) {
                oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_sync_bss_freq_all::sync down vap%d failed}",
                    hmac_vap_tmp->vap_id, hmac_device->vap_id[idx]);
            }
            continue;
        }

        oam_warning_log1(0, OAM_SF_DFT, "hmac_sta_sync_bss_freq_all:clean bss,send DISASSOC,err code[%d]",
            MAC_DISAS_LV_SS);
        if (hmac_sta_sync_bss_freq(hmac_vap_tmp, &channel, protocol) != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_CHAN, "vap_id[%d] {hmac_sta_sync_bss_freq_all::sync vap%d failed}",
                hmac_vap_tmp->vap_id, hmac_device->vap_id[idx]);
        }
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 功能描述  : multi bssid场景下，关联完成之后，向dmac发事件，配置dmac multi bssid相关参数及对应寄存器
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_asoc_set_multi_bss_info(hmac_vap_stru *hmac_vap)
{
    osal_s32 ret;
    hmac_scanned_bss_info     *scaned_bss   = hmac_vap_get_scan_bss_info(hmac_vap);

    if (scaned_bss == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_asoc_set_multi_bss_info::scaned_bss null.}");
        return;
    }

    if (scaned_bss->bss_dscr_info.all_bss_info.multi_bss_eid_exist != OSAL_TRUE) {
        return;
    }

    ret = hmac_set_multi_bssid(hmac_vap, &scaned_bss->bss_dscr_info.all_bss_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_sta_asoc_set_multi_bss_info::send_cfg_to_device err=%d}", ret);
    }
}
#endif

/*****************************************************************************
 函 数 名  : hmac_sta_wait_asoc_rx_etc
 功能描述  : 在WAIT_ASOC状态下接收到Asoc_rsp_frame的处理函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_sta_wait_asoc_rx_etc(hmac_vap_stru *hmac_vap, osal_void *msg)
{
    mac_status_code_enum_uint16 asoc_status;
    osal_u8 frame_sub_type;
    hmac_wlan_crx_event_stru *mgmt_rx_event = OAL_PTR_NULL;
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru *rx_info = OAL_PTR_NULL;
    osal_u8 *mac_hdr = OAL_PTR_NULL;
    osal_u8 *payload = OAL_PTR_NULL;
    osal_u16 msg_len, hdr_len, user_idx;
    hmac_asoc_rsp_stru asoc_rsp;
    osal_u8 addr_sa[WLAN_MAC_ADDR_LEN];
    osal_u32 rslt, ret;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
    osal_void *fhook_clear_defrag = hmac_get_feature_fhook(HMAC_FHOOK_FRAG_DERAG_CLEAR);

    if ((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_sta_wait_asoc_rx_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_rx_event = (hmac_wlan_crx_event_stru *)msg;
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(mgmt_rx_event->netbuf);
    rx_info = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->rx_info));
    mac_hdr = (osal_u8 *)mac_get_rx_cb_mac_header_addr(rx_info);
    payload = (osal_u8 *)(mac_hdr) + rx_info->mac_header_len;
    msg_len = rx_info->frame_len - rx_info->mac_header_len;   /* 消息总长度,不包括FCS */
    hdr_len = rx_info->mac_header_len;

    frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    asoc_status    = mac_get_asoc_status(payload);

    if ((frame_sub_type != (WLAN_FC0_SUBTYPE_ASSOC_RSP | WLAN_FC0_TYPE_MGT)) &&
        (frame_sub_type != (WLAN_FC0_SUBTYPE_REASSOC_RSP | WLAN_FC0_TYPE_MGT))) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_wait_asoc_rx_etc:: frame_sub_type=0x%02x.}",
            hmac_vap->vap_id, frame_sub_type);
        return OAL_FAIL;
    }
    /* 刷新assoc resp返回的状态码 */
    hmac_sta_set_last_reason_code(asoc_status);
    if ((asoc_status != MAC_SUCCESSFUL_STATUSCODE)
#ifdef _PRE_WLAN_FEATURE_WPA3
        && ((asoc_status != MAC_STATUS_INVALID_PMKID) || (hmac_vap->sae_have_pmkid != OAL_TRUE))
#endif
    ) {
#ifdef _PRE_WLAN_DFT_STAT
        if ((dfx_get_service_control() & SERVICE_CONTROL_MGMT_FRAME_RX) != 0) {
            wifi_printf("[service]vap[%d] {hmac_sta_wait_asoc_rx_etc:: AP refuse STA assoc reason=%d.}\r\n",
                hmac_vap->vap_id, asoc_status);
        }
#endif
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_sta_wait_asoc_rx_etc:: AP refuse STA assoc reason=%d.}",
            hmac_vap->vap_id, asoc_status);

        if (fhook != OSAL_NULL) {
            ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_vap->bssid, STA_CONN_ASOC_RSP_ERR,
                USER_CONN_FAIL);
        }
        /* STA 保存AP 在assoc rsp 帧中的status code */
        hmac_vap->mgmt_timetout_param.status_code = asoc_status;
        return OAL_FAIL;
    }

    if (msg_len < OAL_ASSOC_RSP_FIXED_OFFSET) {
        oam_error_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_wait_asoc_rx_etc::asoc_rsp_body is too short(%d) to going on!}",
            hmac_vap->vap_id, msg_len);

        return OAL_FAIL;
    }

    /* 获取SA 地址 */
    mac_get_address2(mac_hdr, (osal_u8 *)addr_sa);

    /* 根据SA 找到对应AP USER结构 */
    rslt = hmac_vap_find_user_by_macaddr_etc(hmac_vap, (osal_u8 *)addr_sa, &user_idx);
    if (rslt != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_wait_asoc_rx_etc:: hmac_vap_find_user_by_macaddr_etc failed[%d].}",
            hmac_vap->vap_id, rslt);
        return rslt;
    }

    /* 获取STA关联的AP的用户指针 */
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_sta_wait_asoc_rx_etc::hmac_user_ap[%d] null.}",
            hmac_vap->vap_id, user_idx);
        return OAL_FAIL;
    }

    /* 取消定时器 */
    frw_destroy_timer_entry(&(hmac_vap->mgmt_timer));
    asoc_rsp.status_code = asoc_status;
#ifdef _PRE_WLAN_FEATURE_WPA3
    /* SAE assoc失败流程上报 */
    if (hmac_sta_wait_asoc_rx_fail_handle_for_sae(hmac_vap, asoc_rsp) != OAL_SUCC) {
        oam_warning_log1(0, 0, "{hmac_sta_wait_asoc_rx_fail_handle_for_sae::sae rx asoc status[%d].}", asoc_status);
        return OAL_SUCC;
    }
#endif
    ret = hmac_process_assoc_rsp_etc(hmac_vap, hmac_user, mac_hdr, hdr_len, payload, msg_len);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_wait_asoc_rx_etc::hmac_process_assoc_rsp_etc failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    /* 关联上用户之后，初始化黑名单方案 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_CHECK_IN_LIST);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_check_exception_in_list_cb)fhook)(hmac_vap, hmac_user, addr_sa);  /* 更新 ba_resp_buf_size */
    }

    /* STA切换到UP状态 */
    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_UP);

    /* 此时VAP状态为UP，刷新linkloss初始时间戳，计数清零 */
    hmac_vap_linkloss_clean(hmac_vap);
    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg_etc(hmac_vap, user_idx);

#ifdef _PRE_WLAN_FEATURE_11AX
    /* multi bssid场景下，关联完成之后，向dmac发事件，配置dmac multi bssid相关参数及对应寄存器 */
    if (mac_mib_get_multi_bssid_implement(hmac_vap) == OSAL_TRUE) {
        hmac_sta_asoc_set_multi_bss_info(hmac_vap);
    }
#endif

    /* 准备消息，上报给APP */
    asoc_rsp.result_code = HMAC_MGMT_SUCCESS;
    asoc_rsp.status_code = MAC_SUCCESSFUL_STATUSCODE;

    /* 记录关联响应帧的部分内容，用于上报给内核 */
    asoc_rsp.asoc_rsp_ie_len = msg_len - OAL_ASSOC_RSP_FIXED_OFFSET;  /* 除去MAC帧头24字节和FIXED部分6字节 */
    asoc_rsp.asoc_rsp_ie_buff = mac_hdr + OAL_ASSOC_RSP_IE_OFFSET;

    /* 获取AP的mac地址 */
    mac_get_bssid(mac_hdr, asoc_rsp.addr_ap, WLAN_MAC_ADDR_LEN);

    /* 获取关联请求帧信息 */
    asoc_rsp.asoc_req_ie_buff = hmac_user->assoc_req_ie_buff;
    asoc_rsp.asoc_req_ie_len   = hmac_user->assoc_req_ie_len;
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    /* 获取信道中心频率 */
    asoc_rsp.freq = (osal_u16)oal_ieee80211_channel_to_frequency(hmac_vap->channel.chan_number,
        hmac_vap->channel.band);
#endif
    if (fhook_clear_defrag != OSAL_NULL) { /* 重关联流程中清除user下的分片缓存，防止重关联或者rekey流程报文重组attack */
        ((hmac_user_clear_defrag_res_cb)fhook_clear_defrag)(hmac_user);
    }
    hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_ASOC_RSP, (osal_u8 *)(&asoc_rsp));

    /* 上报给Lwip */
    hmac_report_assoc_state_sta(hmac_vap, asoc_rsp.addr_ap, OAL_TRUE);

    if (oal_notice_sta_join_result(hmac_vap->chip_id, OAL_TRUE) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX,
            "vap_id[%d] {hmac_sta_wait_asoc_rx_etc::oal_notice_sta_join_result fail.}",
            hmac_vap->vap_id);
    }

#if defined(_PRE_PRODUCT_ID_HOST)
    /* 信道跟随检查 */
    rslt = hmac_check_ap_channel_follow_sta(hmac_vap, &hmac_vap->channel);
    if (rslt == OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_wait_asoc_rx_etc:: after hmac_check_ap_channel_follow_sta. ap channel change to %d}",
            hmac_vap->vap_id, hmac_vap->channel.chan_number);
    }
#endif

    // will not do any sync if proxysta enabled
    // allow running DBAC on different channels of same band while P2P defined
#ifndef _PRE_WLAN_FEATURE_P2P
    ret = hmac_sta_sync_bss_freq_all(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "{hmac_sta_wait_asoc_rx_etc::hmac_sta_sync_bss_freq_all failed[%d].}", ret);
    }
#endif
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_RX_ASSOC_RSP, hmac_user);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_auth_timeout_etc
 功能描述  : 认证超时处理
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_auth_timeout_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    hmac_auth_rsp_stru            auth_rsp;
    unref_param(p_param);
    memset_s(&auth_rsp, sizeof(auth_rsp), 0, sizeof(auth_rsp));

    /* and send it to the host. */
    if (hmac_vap->vap_state == MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2) {
        auth_rsp.status_code = MAC_AUTH_RSP2_TIMEOUT;
    } else if (hmac_vap->vap_state == MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4) {
        auth_rsp.status_code = MAC_AUTH_RSP4_TIMEOUT;
    } else {
        auth_rsp.status_code = HMAC_MGMT_TIMEOUT;
    }

    /* Send the response to host now. */
    hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_AUTH_RSP, (osal_u8 *)&auth_rsp);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_get_phy_rate_and_protocol
 功能描述  : hmac层获取phy层需要的协议和速率
 输入参数  : pre_rate: mac层的速率
 输出参数  : rate: 转换成phy层的速率
             protocol: 获取到的协议
*****************************************************************************/
OAL_STATIC osal_u32 hmac_get_phy_rate_and_protocol(osal_u8 pre_rate, wlan_protocol_enum_uint8 *protocol,
    osal_u8 *rate_idx)
{
    osal_u32 idx;

    for (idx = 0; idx < DATARATES_80211G_NUM; idx++) {
        if (g_st_data_rates[idx].expand_rate == pre_rate ||
            g_st_data_rates[idx].mac_rate == pre_rate) {
            *rate_idx = (osal_u8)idx;
            *protocol = g_st_data_rates[idx].protocol;
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hmac_sta_get_min_rate
 功能描述  : 获取交集速率中的最小速率填写到入网结构体
 输入参数  : join_req: 入网请求结构体信息
 输出参数  : rate_params: 速率信息
 返 回 值  : OAL_SUCC
*****************************************************************************/
osal_u32 hmac_sta_get_min_rate(hmac_set_rate_stru *rate_params, hmac_join_req_stru *join_req)
{
    osal_u32 idx;
    osal_u8 auc_min_rate_idx[2] = {0}; /* 第一个存储11b协议对应的速率，第二个存储11ag协议对应的速率 */
    osal_u8 min_rate_idx = 0;
    wlan_protocol_enum_uint8 protocol = 0;

    if (osal_unlikely(rate_params == OAL_PTR_NULL || join_req == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_get_min_rate::NULL param}");
        return OAL_FAIL;
    }

    memset_s(rate_params, OAL_SIZEOF(hmac_set_rate_stru), 0, OAL_SIZEOF(hmac_set_rate_stru));

    // get min rate
    for (idx = 0; idx < join_req->bss_dscr.num_supp_rates; idx++) {
        if (hmac_get_phy_rate_and_protocol(join_req->bss_dscr.supp_rates[idx], &protocol,
            &min_rate_idx) != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_SCAN, "{hmac_sta_get_min_rate::hmac_get_rate_protocol failed (%d).[%d]}",
                min_rate_idx, idx);
            continue;
        }

        /* 根据枚举wlan_phy_protocol_enum 填写数组对应位置的值 */
        if ((rate_params->capable_flag.value & ((osal_u32)1 << (protocol))) != 0) {
            if (auc_min_rate_idx[protocol] > min_rate_idx) {
                auc_min_rate_idx[protocol] = min_rate_idx;
            }
        } else {
            auc_min_rate_idx[protocol] = min_rate_idx;
            rate_params->capable_flag.value |= ((osal_u32)1 << (protocol));
        }

        if (min_rate_idx == 0x08) { // data_rates第八个是24M
            rate_params->capable_flag.capable.ht_capable = OAL_TRUE;
            rate_params->capable_flag.capable.vht_capable = OAL_TRUE;
        }
    }

    /* 与储存在扫描结果描述符中的能力进行比较，看能力是否匹配 */
    rate_params->capable_flag.capable.ht_capable &= join_req->bss_dscr.ht_capable;
    rate_params->capable_flag.capable.vht_capable &= join_req->bss_dscr.vht_capable;
    rate_params->min_rate[WLAN_11B_PHY_PROTOCOL_MODE] =
        g_st_data_rates[auc_min_rate_idx[WLAN_11B_PHY_PROTOCOL_MODE]].phy_rate;
    rate_params->min_rate[WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE] =
        g_st_data_rates[auc_min_rate_idx[WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE]].phy_rate;

    oam_warning_log4(0, OAM_SF_ASSOC,
        "{hmac_sta_get_min_rate:: min_rate_idx[%d]capable_flag.value[%d]legacy rate[%d][%d]}",
        min_rate_idx, rate_params->capable_flag.value,
        rate_params->min_rate[WLAN_11B_PHY_PROTOCOL_MODE],
        rate_params->min_rate[WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE]);

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_sta_update_join_fill_basic_params(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req)
{
    osal_u8 *cur_ssid = OAL_PTR_NULL;

    /* 设置BSSID */
    memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN,
             join_req->bss_dscr.bssid, WLAN_MAC_ADDR_LEN);

    /* 更新mib库对应的dot11BeaconPeriod值 */
    mac_mib_set_beacon_period(hmac_vap, (osal_u32)(join_req->bss_dscr.beacon_period));

    /* 更新mib库对应的ul_dot11CurrentChannel值 */
    mac_vap_set_current_channel_etc(hmac_vap, join_req->bss_dscr.st_channel.band,
                                    join_req->bss_dscr.st_channel.chan_number);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 更新sta期望加入的国家字符串 */
    hmac_vap->ac_desired_country[0] = join_req->bss_dscr.ac_country[0];
    hmac_vap->ac_desired_country[1] = join_req->bss_dscr.ac_country[1];
    hmac_vap->ac_desired_country[2] = join_req->bss_dscr.ac_country[2]; /* 国家码第2位 */
#endif

    /* 更新mib库对应的ssid */
    cur_ssid = mac_mib_get_desired_ssid(hmac_vap);
    if (memcpy_s(cur_ssid, WLAN_SSID_MAX_LEN, join_req->bss_dscr.ac_ssid, WLAN_SSID_MAX_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_update_join_fill_basic_params::memcpy_s error}");
    }
    cur_ssid[WLAN_SSID_MAX_LEN - 1] = '\0';

    /* 更新频带、主20MHz信道号，与AP通信 DMAC切换信道时直接调用 */
    mac_vap_set_cap_bw(hmac_vap, hmac_vap_get_bandwith(hmac_vap, join_req->bss_dscr.channel_bandwidth));
    hmac_vap->channel.chan_number = join_req->bss_dscr.st_channel.chan_number;
    hmac_vap->channel.band        = join_req->bss_dscr.st_channel.band;
}

OAL_STATIC osal_u32 hmac_sta_update_join_fill_ht_vht(hmac_join_req_stru *join_req, mac_cfg_mode_param_stru *cfg_mode,
    hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;

    memset_s(cfg_mode, OAL_SIZEOF(mac_cfg_mode_param_stru), 0, OAL_SIZEOF(mac_cfg_mode_param_stru));

    mac_mib_set_high_throughput_option_implemented(hmac_vap, join_req->bss_dscr.ht_capable);
    mac_mib_set_vht_option_implemented(hmac_vap, join_req->bss_dscr.vht_capable);

#ifdef _PRE_WLAN_FEATURE_11AX
    if (is_legacy_sta(hmac_vap) && (join_req->bss_dscr.he_capable == OAL_TRUE) && (is_11ax_vap(hmac_vap))) {
        mac_mib_set_he_option_implemented(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_he_option_implemented(hmac_vap, OAL_FALSE);
    }
#endif

    /* STA更新LDPC和STBC的能力,更新能力保存到Activated mib中 */
    mac_mib_set_LDPCCodingOptionActivated(hmac_vap,
        (oal_bool_enum_uint8)(join_req->bss_dscr.ht_ldpc & mac_mib_get_ldpc_coding_option_implemented(hmac_vap)));
    mac_mib_set_TxSTBCOptionActivated(hmac_vap,
        (oal_bool_enum_uint8)(join_req->bss_dscr.ht_stbc & mac_mib_get_tx_stbc_option_implemented(hmac_vap)));

    /* 关联2G AP，且2ght40禁止位为1时，不学习AP的HT 40能力 */
    if ((hmac_vap->channel.band == WLAN_BAND_2G) && (hmac_vap->cap_flag.disable_2ght40 == OSAL_FALSE)) {
        if (join_req->bss_dscr.bw_cap > WLAN_BW_CAP_20M &&
            mac_mib_get_dot11_vap_max_bandwidth(hmac_vap) > WLAN_BW_CAP_20M) {
            mac_mib_set_forty_mhz_operation_implemented(hmac_vap, OAL_TRUE);
        } else {
            mac_mib_set_forty_mhz_operation_implemented(hmac_vap, OAL_FALSE);
        }
    }

    /* 根据要加入AP的协议模式更新STA侧速率集 */
    ret = hmac_sta_get_user_protocol_etc(&(join_req->bss_dscr), &(cfg_mode->protocol));
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SCAN,
            "{hmac_sta_update_join_fill_ht_vht:hmac_sta_get_user_protocol_etc fail %d}", ret);
        return ret;
    }

    oam_warning_log3(0, OAM_SF_SCAN, "vap_id[%d] {hmac_sta_update_join_fill_ht_vht::mac_vap_pro[%d], cfg_pro[%d]}",
        hmac_vap->vap_id, hmac_vap->protocol, cfg_mode->protocol);

    if (hmac_vap_need_update_protocol(hmac_vap, cfg_mode->protocol) == OSAL_TRUE) {
        cfg_mode->band = join_req->bss_dscr.st_channel.band;
        cfg_mode->en_bandwidth = hmac_vap_get_bandwith(hmac_vap,
            join_req->bss_dscr.channel_bandwidth);
        cfg_mode->channel_idx = join_req->bss_dscr.st_channel.chan_number;
        ret = hmac_config_sta_update_rates_etc(hmac_vap, cfg_mode, (osal_void *)&join_req->bss_dscr);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_SCAN,
                "{hmac_sta_update_join_fill_ht_vht::hmac_config_sta_update_rates_etc fail %d.}", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sta_update_join_fill_proto(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req,
    mac_cfg_mode_param_stru *cfg_mode, hmac_device_stru *hmac_device)
{
    osal_u32 ret;

    /* 有些加密协议只能工作在legacy */
    hmac_sta_protocol_down_by_chipher(hmac_vap, &join_req->bss_dscr);
#ifndef _PRE_WIFI_DMT
    cfg_mode->protocol  = hmac_vap->protocol;
    cfg_mode->band      = hmac_vap->channel.band;
    cfg_mode->en_bandwidth = hmac_vap->channel.en_bandwidth;
    cfg_mode->channel_idx  = join_req->bss_dscr.st_channel.chan_number;
    hmac_config_sta_update_rates_etc(hmac_vap, cfg_mode, (osal_void *)&join_req->bss_dscr);
#endif

    /* STA首先以20MHz运行，如果要切换到40 or 80MHz运行，需要满足一下条件: */
    /* (1) 用户支持40 or 80MHz运行 */
    /* (2) AP支持40 or 80MHz运行(HT Supported Channel Width Set = 1 && VHT Supported Channel Width Set = 0) */
    /* (3) AP在40 or 80MHz运行(SCO = SCA or SCB && VHT Channel Width = 1) */
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    hmac_vap->channel.en_bandwidth   = WLAN_BAND_WIDTH_20M;
#endif
    ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, hmac_vap->channel.chan_number,
        &(hmac_vap->channel.chan_idx));
    if (ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_SCAN,
            "vap_id[%d] {band and channel_num are not compatible.band[%d], channel_num[%d]}",
            hmac_vap->vap_id, hmac_vap->channel.band, hmac_vap->channel.chan_number);
        return ret;
    }

    /* 更新协议相关信息，包括WMM P2P 11I 20/40M等 */
    hmac_update_join_req_params_prot_sta(hmac_vap, join_req);
    /* 入网优化，不同频段下的能力不一样 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        mac_mib_set_short_preamble_option_implemented(hmac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
        mac_mib_set_spectrum_management_required(hmac_vap, OAL_FALSE);
    } else {
        mac_mib_set_short_preamble_option_implemented(hmac_vap, WLAN_LEGACY_11B_MIB_LONG_PREAMBLE);
        mac_mib_set_spectrum_management_required(hmac_vap, OAL_TRUE);
    }

    /* 根据协议信道做带宽约束, 此时已经跟进join参数刷新好带宽和信道，此时直接根据当前信道再次刷新带宽 */
    hmac_sta_bandwidth_down_by_channel(hmac_vap);

    if (hmac_calc_up_ap_num_etc(hmac_device) == 0) {
        hmac_device->max_channel   = hmac_vap->channel.chan_number;
        hmac_device->max_band      = hmac_vap->channel.band;
        hmac_device->max_bandwidth = hmac_vap->channel.en_bandwidth;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_sta_update_join_fill_event(hmac_join_req_stru *join_req,
    hmac_ctx_join_req_set_reg_stru *reg_params, hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;

    /* 设置需要写入寄存器的BSSID信息 */
    oal_set_mac_addr(reg_params->bssid, join_req->bss_dscr.bssid);

    /* 填写信道相关信息 */
    reg_params->current_channel.chan_number    = hmac_vap->channel.chan_number;
    reg_params->current_channel.band           = hmac_vap->channel.band;
    reg_params->current_channel.en_bandwidth   = hmac_vap->channel.en_bandwidth;
    reg_params->current_channel.chan_idx       = hmac_vap->channel.chan_idx;

    /* 填写速率相关信息 */
    ret = hmac_sta_get_min_rate(&reg_params->min_rate, join_req);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ASSOC,
            "{hmac_sta_update_join_req_params_etc::hmac_sta_get_min_rate failed[%d].}", ret);
    }

    /* 设置beaocn period信息 */
    reg_params->beacon_period      = (join_req->bss_dscr.beacon_period);

    /* 同步FortyMHzOperationImplemented */
    reg_params->dot11_40mhz_operation_implemented   = mac_mib_get_forty_mhz_operation_implemented(hmac_vap);

    /* 设置beacon filter关闭 */
    reg_params->beacon_filter    = OAL_FALSE;

    /* 设置no frame filter打开 */
    reg_params->non_frame_filter = OAL_TRUE;

    /* 下发ssid */
    if (memcpy_s(reg_params->ssid, sizeof(reg_params->ssid), join_req->bss_dscr.ac_ssid, WLAN_SSID_MAX_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_sta_update_join_fill_event::memcpy_s error}");
    }
    reg_params->ssid[WLAN_SSID_MAX_LEN - 1] = '\0';
}

#ifdef _PRE_WLAN_FEATURE_M2S
OAL_STATIC osal_void hmac_sta_update_join_m2s(hmac_join_req_stru *join_req, hmac_vap_stru *hmac_vap)
{
    oam_warning_log4(0, OAM_SF_ASSOC,
        "{hmac_sta_update_join_req_params_etc::ap nss[%d],ap band[%d],ap ht cap[%d],ap vht cap[%d].}",
        join_req->bss_dscr.support_max_nss, join_req->bss_dscr.st_channel.band,
        join_req->bss_dscr.ht_capable, join_req->bss_dscr.vht_capable);

    /* STA初始化默认打开opmode，部分三空间流路由器，在HT协议下也不支持OPMODE帧，STA根据对端能力刷新opmode能力 */
    /* 双流及其以下，三四流ht设备都配置为不支持opmode */
    if ((join_req->bss_dscr.support_max_nss == WLAN_SINGLE_NSS) ||
        (join_req->bss_dscr.vht_capable == OAL_FALSE) ||
        ((join_req->bss_dscr.support_max_nss == WLAN_DOUBLE_NSS) &&
        (join_req->bss_dscr.support_opmode == OAL_FALSE))) {
        hmac_vap->cap_flag.opmode = OAL_FALSE;
    } else {
        hmac_vap->cap_flag.opmode = OAL_TRUE;
    }

    /* 同步vap修改信息到device侧 */
    hmac_config_vap_m2s_info_syn(hmac_vap);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_sta_update_join_req_params_etc
 功能描述  : 根据join_request帧更新mib信息和填写相应寄存器
 输入参数  : hmac_vap_stru      *hmac_vap,
             hmac_join_req_stru *join_req
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_sta_update_join_req_params_etc(hmac_vap_stru *hmac_vap, hmac_join_req_stru *join_req)
{
    osal_u32                      ul_ret;
    hmac_device_stru               *hmac_device = OAL_PTR_NULL;
    wlan_mib_ieee802dot11_stru     *pst_mib_info = OAL_PTR_NULL;
    mac_cfg_mode_param_stru         cfg_mode;
    hmac_ctx_join_req_set_reg_stru reg_params;
    osal_void *fhook;

    (osal_void)memset_s(&reg_params, OAL_SIZEOF(reg_params), 0, OAL_SIZEOF(reg_params));

    pst_mib_info     = hmac_vap->mib_info;
    if (pst_mib_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    hmac_sta_update_join_fill_basic_params(hmac_vap, join_req);

    /* STA依据要关联的AP，更新mib库中对应的HT/VHT能力 */
    ul_ret = hmac_sta_update_join_fill_ht_vht(join_req, &cfg_mode, hmac_vap);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    ul_ret = hmac_sta_update_join_fill_proto(hmac_vap, join_req, &cfg_mode, hmac_device);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }
    hmac_sta_update_join_fill_event(join_req, &reg_params, hmac_vap);

    hmac_join_set_reg_event_process(hmac_vap, &reg_params);
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_sta_update_join_m2s(join_req, hmac_vap);
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_UPDATE_PMF);
    if (fhook != OSAL_NULL) {
        ((hmac_sta_roam_update_pmf_etc_cb)fhook)(hmac_vap, &join_req->bss_dscr);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_wait_asoc_timeout_etc
 功能描述  : 关联超时处理函数
 输入参数  : hmac_vap_stru *hmac_vap, osal_void *p_param
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_sta_wait_asoc_timeout_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    hmac_asoc_rsp_stru            asoc_rsp = {0};
    hmac_mgmt_timeout_param_stru *timeout_param = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_sta_wait_asoc_timeout_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    timeout_param = (hmac_mgmt_timeout_param_stru *)p_param;

    /* 填写关联结果 */
    asoc_rsp.result_code = HMAC_MGMT_TIMEOUT;

    /* 关联超时失败,原因码上报wpa_supplicant */
    asoc_rsp.status_code = timeout_param->status_code;

    /* 发送关联结果给SME */
    hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_ASOC_RSP, (osal_u8 *)&asoc_rsp);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC void hmac_close_11ax_related_parameters(hmac_vap_stru *hmac_vap)
{
    if (mac_mib_get_he_option_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }
    hal_set_he_rom_en(OSAL_FALSE);
    hal_close_mu_edca_func();

    return;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_handle_disasoc_rsp_sta
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_sta_handle_disassoc_rsp_etc(hmac_vap_stru *hmac_vap, osal_u16 disasoc_reason_code)
{
    osal_s32 ret;
    frw_msg msg_info = {0};

    /* 通告vap sta去关联 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        oal_notice_sta_join_result(hmac_vap->chip_id, OAL_FALSE);
#ifdef _PRE_WLAN_FEATURE_11AX
        hmac_close_11ax_related_parameters(hmac_vap);
#endif
    }

    msg_info.data = (osal_u8 *)&disasoc_reason_code;
    msg_info.data_len = OAL_SIZEOF(osal_u16);
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_DISASOC_COMP_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_sta_handle_disassoc_rsp_etc::post msg failed.}ret[%d]",
            hmac_vap->vap_id, ret);
    }
    /* 上报给Lwip */
    hmac_report_assoc_state_sta(hmac_vap, BROADCAST_MACADDR, OAL_FALSE);
    return;
}

OAL_STATIC osal_void hmac_sta_deauth_del_bss(hmac_vap_stru *hmac_vap, const osal_u8 *mac_hdr)
{
    osal_u8 bssid[WLAN_MAC_ADDR_LEN] = {0};
    mac_bss_dscr_stru *bss_dscr = OSAL_NULL;
    osal_s8 *current_country = hmac_regdomain_get_country_etc();

    /* 当前工作信道非10、11则不需要考虑AP切换到国家码不支持的信道，导致连接到12,13信道问题 */
    if ((hmac_vap->channel.chan_number != 10) && (hmac_vap->channel.chan_number != 11)) {
        return;
    }

    /* 当前国家码支持12, 13信道则直接返回 */
    if ((hmac_is_channel_num_valid_etc(WLAN_BAND_2G, 12) == OAL_SUCC) &&
        (hmac_is_channel_num_valid_etc(WLAN_BAND_2G, 13) == OAL_SUCC)) { /* 13信道 */
        return;
    }

    mac_get_address2((osal_u8 *)mac_hdr, (osal_u8 *)bssid);

    /* 获取扫描的bss信息 */
    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap, (osal_u8 *)bssid);
    if (bss_dscr == OSAL_NULL) {
        return;
    }
    /* 仅考虑2.4G 信道切换场景 */
    if (bss_dscr->st_channel.band != WLAN_BAND_2G) {
        return;
    }
    /* bss国家码与当前国家码一致直接返回 */
    if ((bss_dscr->ac_country[0] == current_country[0]) && (bss_dscr->ac_country[1] == current_country[1])) {
        return;
    }

    /* bss中的信道已更新到最新信道，说明是支持的信道，直接返回 */
    if (hmac_vap->channel.chan_number != bss_dscr->st_channel.chan_number) {
        return;
    }
    oam_info_log1(0, OAM_SF_AUTH, "{Band is 2.4G and vap channel is [%d], and not same country.}",
        hmac_vap->channel.chan_number);
    oam_info_log4(0, OAM_SF_AUTH, "{delete bss form bssid %2x:%2x:%2x:%2x:xx:xx.}",
        bssid[0], bssid[1], bssid[2], bssid[3]); /* 1,2,3：mac地址位 */
    hmac_scan_delete_bss(hmac_vap, (const osal_u8 *)bssid);
    return;
}

OAL_STATIC osal_u32 hmac_sta_rx_deauth_req_proc(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    oal_bool_enum_uint8 is_protected, hmac_user_stru *hmac_user_vap)
{
    osal_u8 *da = OAL_PTR_NULL;
    osal_u32 promise_ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
    osal_void *promis_fhook = hmac_get_feature_fhook(HMAC_FHOOK_PROMIS_RX_DEAUTH_REQ_PROC);

    /* 如果该用户的管理帧加密属性不一致，丢弃该报文 */
    mac_rx_get_da((mac_ieee80211_frame_stru *)mac_hdr, &da);
    if ((ETHER_IS_MULTICAST(da) != OAL_TRUE) &&
        (is_protected != hmac_user_vap->cap_info.pmf_active)) {
        oam_error_log1(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_sta_rx_deauth_req::PMF check failed.}", hmac_vap->vap_id);

        return OAL_FAIL;
    }
    if (promis_fhook != OSAL_NULL) {
        promise_ret = ((hmac_sta_rx_deauth_req_proc_for_promisc_cb)promis_fhook)(hmac_vap, mac_hdr);
        if (promise_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_AUTH,
                "vap_id[%d] {hmac_sta_rx_deauth_req_proc::promis proc failed.}", hmac_vap->vap_id);
            return promise_ret;
        }
    }

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, hmac_user_vap->user_mac_addr,
            STA_RCV_AP_DEAUTH_OR_DISASOC, USER_CONN_OFFLINE);
    }
    hmac_sta_deauth_del_bss(hmac_vap, mac_hdr);

    /* 删除user */
    if (hmac_user_del_etc(hmac_vap, hmac_user_vap) != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_sta_rx_deauth_req::hmac_user_del_etc failed.}", hmac_vap->vap_id);

        /* 上报内核sta已经和某个ap去关联 */
        hmac_sta_handle_disassoc_rsp_etc(hmac_vap, *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN)));
        return OAL_FAIL;
    }

    /* 上报内核sta已经和某个ap去关联 */
    hmac_sta_handle_disassoc_rsp_etc(hmac_vap, *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN)));

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_sta_rx_deauth_req_etc(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr)
{
    osal_u8 *sa = OAL_PTR_NULL;

    if (hmac_vap == OAL_PTR_NULL || mac_hdr == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_sta_rx_deauth_req::input param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* wps连接收到eap_failuer帧后，不再处理接下来的deauth帧 */
    if (hmac_vap->mib_info->wlan_mib_sta_config.dot11_wps_active == OAL_TRUE &&
        *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN)) == WLAN_REASON_IEEE_802_1X_AUTH_FAILED) {
        return OAL_SUCC;
    }
    /* 增加接收到去认证帧或者去关联帧时的维测信息 */
    mac_rx_get_sa((mac_ieee80211_frame_stru *)mac_hdr, &sa);
    oam_warning_log4(0, OAM_SF_AUTH,
        "{Because of err_code[%d], received deauth or disassoc frame frome source addr, sa %2x:%2x:%2x.}",
        *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN)), sa[0], sa[1], sa[2]); /* 0,1,2：mac地址位 */

#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_MGMT_FRAME_RX) != 0) {
        wifi_printf("{[service]err_code[%d],received deauth or disassoc frame from source addr, sa %2x:%2x:%2x:%2x.}",
            *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN)), sa[0], sa[1], sa[2], sa[3]); /* mac的第1,2,3,4字节 */
    }
#endif
    return OAL_CONTINUE;
}
/*****************************************************************************
函 数 名  : hmac_sta_rx_deauth_req
功能描述  : 处理接收去认证帧
输入参数  : 无
输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_rx_deauth_req(hmac_vap_stru *hmac_vap, osal_u8 *mac_hdr,
    oal_bool_enum_uint8 is_protected)
{
    osal_u8 bssid[6] = {0};
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u16 user_idx = 0xffff;
    osal_u32 ret;

    ret = hmac_sta_rx_deauth_req_etc(hmac_vap, mac_hdr);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    mac_get_address2(mac_hdr, (osal_u8 *)bssid);

    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, (const osal_u8 *)bssid, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_sta_rx_deauth_req::find user failed[%d],other bss deauth frame!}",
            hmac_vap->vap_id, ret);
        return ret;
    }
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_AUTH,
            "vap_id[%d] {hmac_sta_rx_deauth_req::hmac_user_vap[%d] null}", hmac_vap->vap_id,
            user_idx);
        /* 没有查到对应的USER,发送去认证消息 */
        hmac_mgmt_send_deauth_frame_etc(hmac_vap, (osal_u8 *)bssid, MAC_NOT_AUTHED, OAL_FALSE);

        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        /* 上报内核sta已经和某个ap去关联 */
        hmac_sta_handle_disassoc_rsp_etc(hmac_vap, *((osal_u16 *)(mac_hdr + MAC_80211_FRAME_LEN)));
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    /* 检查是否需要发送SA query request */
    if ((hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (hmac_pmf_check_err_code_etc(hmac_user, is_protected, mac_hdr) == OAL_SUCC)) {
        /* 在关联状态下收到未加密的ReasonCode 6/7需要启动SA Query流程 */
        ret = hmac_start_sa_query_etc(hmac_vap, hmac_user, hmac_user->cap_info.pmf_active);
        if (ret != OAL_SUCC) {
            return OAL_ERR_CODE_PMF_SA_QUERY_START_FAIL;
        }

        return OAL_SUCC;
    }

#endif

    return hmac_sta_rx_deauth_req_proc(hmac_vap, mac_hdr, is_protected, hmac_user);
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_beacon
 功能描述  : sta up状态接收beacon帧处理
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_up_rx_beacon(hmac_vap_stru *hmac_vap_sta, oal_netbuf_stru *pst_netbuf)
{
    osal_u32                  ret;
    osal_u16                  frame_len, frame_offset, user_idx;
    osal_u8                   frame_sub_type, addr_sa[WLAN_MAC_ADDR_LEN];
    hmac_user_stru             *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = OSAL_NULL;
    mac_rx_ctl_stru *rx_info = (mac_rx_ctl_stru *)(&(((dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf))->rx_info));
    mac_ieee80211_frame_stru *mac_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(rx_info);
    osal_u8 *frame_body = (osal_u8 *)mac_hdr + rx_info->mac_header_len;

    frame_len        = rx_info->frame_len - rx_info->mac_header_len; /* 帧体长度 */
    frame_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    frame_sub_type = mac_get_frame_sub_type((osal_u8 *)mac_hdr);

    /* 来自其它bss的Beacon不做处理 */
    if (oal_compare_mac_addr(hmac_vap_sta->bssid, mac_hdr->address3) != 0) {
        return OAL_SUCC;
    }

    /* 获取管理帧的源地址SA */
    mac_get_address2((osal_u8 *)mac_hdr, (osal_u8 *)addr_sa);

    /* 根据SA 地地找到对应AP USER结构 */
    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap_sta, (const osal_u8 *)addr_sa, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_sta_up_rx_beacon::find user by macaddr etc failed[%d].}",
            hmac_vap_sta->vap_id, ret);
        return ret;
    }
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_RX, "vap_id[%d]{hmac_sta_up_rx_beacon:user[%d] null}", hmac_vap_sta->vap_id, user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新edca参数 */
    hmac_sta_up_update_edca_params_etc(frame_body + frame_offset, frame_len - frame_offset,
        hmac_vap_sta, frame_sub_type, hmac_user);

#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_sta_up_update_he_edca_params(frame_body + frame_offset, frame_len - frame_offset,
        hmac_vap_sta, frame_sub_type, hmac_user);
#endif
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_NFRP_UPDATE_PARAMS);
    if (fhook != OSAL_NULL) {
        ((hmac_nfrp_sta_update_param_cb)fhook)(frame_body + frame_offset, frame_len - frame_offset, hmac_vap_sta,
            frame_sub_type, hmac_user);
    }

    hmac_sta_up_parse_qos_capability(frame_body, frame_len, hmac_vap_sta, (osal_u8 *)mac_hdr);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名: hmac_sta_up_rx_probe_response
 功能描述  : 驱动接收到响应的probe response帧之后的处理函数。
 日    期  : 2020年5月10日
*****************************************************************************/
OAL_STATIC osal_u32 hmac_sta_up_rx_probe_response(hmac_vap_stru *hmac_vap_sta, oal_netbuf_stru *netbuf)
{
    dmac_rx_ctl_stru           *rx_ctrl;
    mac_rx_ctl_stru            *rx_info;
    mac_ieee80211_frame_stru   *mac_hdr;
    osal_u32                      ret;
    osal_u16                      frame_len;
    osal_u16                      frame_offset;
    osal_u8                      *frame_body;
    osal_u8                       frame_sub_type;
    hmac_user_stru             *hmac_user = OSAL_NULL;
    osal_u8                       addr_sa_list[WLAN_MAC_ADDR_LEN];
    osal_u16                      user_idx;

    rx_ctrl         = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    rx_info         = (mac_rx_ctl_stru *)(&(rx_ctrl->rx_info));
    mac_hdr         = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(rx_info);
    frame_body      = (osal_u8 *)mac_hdr + rx_info->mac_header_len;
    frame_len       = rx_info->frame_len - rx_info->mac_header_len; /* 帧体长度 */

    frame_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    frame_sub_type = mac_get_frame_sub_type((osal_u8 *)mac_hdr);

    /* 来自其它bss的Probe Response帧不做处理 */
    ret = oal_compare_mac_addr(hmac_vap_sta->bssid, mac_hdr->address3);
    if (ret != 0) {
        return OAL_SUCC;
    }

    /* 获取管理帧的源地址SA */
    mac_get_address2((osal_u8 *)mac_hdr, (osal_u8 *)addr_sa_list);

    /* 根据SA 地地找到对应AP USER结构 */
    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap_sta, (const osal_u8 *)addr_sa_list, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_sta_up_rx_beacon::hmac_vap_find_user_by_macaddr_etc failed[%d].}", ret);
        return ret;
    }
    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_sta_up_rx_beacon::hmac_user[%d] null.}", user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新edca参数 */
    hmac_sta_up_update_edca_params_etc(frame_body + frame_offset,
                                       frame_len - frame_offset, hmac_vap_sta, frame_sub_type, hmac_user);

#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_sta_up_update_he_edca_params(frame_body + frame_offset,
                                      frame_len - frame_offset, hmac_vap_sta, frame_sub_type, hmac_user);
#endif

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_sta_up_rx_action_ba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_ieee80211_frame_stru *frame_hdr, osal_u8 *data)
{
    osal_void *fhook = OSAL_NULL;
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_BA_ACTION_ADDBA_REQ:
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_CHECK_RX_SAME_BAW_START_FROM_ADDBA_REQ);
            if (fhook != OSAL_NULL) {
                ((hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc_cb)fhook)(hmac_vap, hmac_user, frame_hdr,
                    data);
            }
            hmac_mgmt_rx_addba_req_etc(hmac_vap, hmac_user, data);
            break;

        case MAC_BA_ACTION_ADDBA_RSP:
            hmac_mgmt_rx_addba_rsp_etc(hmac_vap, hmac_user, data);
            break;

        case MAC_BA_ACTION_DELBA:
            hmac_mgmt_rx_delba_etc(hmac_vap, hmac_user, data);
            break;

        default:
            break;
    }
}

OAL_STATIC osal_void hmac_sta_up_rx_action_wnm(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, osal_u8 *data, oal_bool_enum_uint8 *reported_host)
{
    unref_param(hmac_user);
    unref_param(reported_host);
    unref_param(netbuf);
    oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_sta_up_rx_action::MAC_ACTION_CATEGORY_WNM action=%d.}",
        hmac_vap->vap_id, data[MAC_ACTION_OFFSET_ACTION]);

#ifdef _PRE_WLAN_FEATURE_HS20
    if (data[MAC_ACTION_OFFSET_ACTION] != MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST) {
        /* 上报WNM Notification Request Action帧 */
        hmac_rx_mgmt_send_to_host_etc(hmac_vap, netbuf);
        *reported_host = OSAL_TRUE;
    }
#endif
}

OAL_STATIC osal_void hmac_sta_up_rx_action_public(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    osal_u8 *data, oal_bool_enum_uint8 *reported_host)
{
    unref_param(hmac_vap);
    unref_param(reported_host);
    unref_param(netbuf);
    /* Action */
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_PUB_VENDOR_SPECIFIC:
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (hmac_ie_check_p2p_action_etc(data + MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host_etc(hmac_vap, netbuf);
                *reported_host = OSAL_TRUE;
            }
#endif
            break;

        case MAC_PUB_GAS_INIT_RESP:
        case MAC_PUB_GAS_COMBAK_RESP:
#ifdef _PRE_WLAN_FEATURE_HS20
            /* 上报GAS查询的ACTION帧 */
            hmac_rx_mgmt_send_to_host_etc(hmac_vap, netbuf);
            *reported_host = OSAL_TRUE;

#endif
            break;

        default:
            break;
    }
}

#ifdef _PRE_WLAN_FEATURE_PMF
OAL_STATIC osal_void hmac_sta_up_rx_action_sa_query(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const osal_u8 *data, oal_bool_enum_uint8 is_protected)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_SA_QUERY_ACTION_REQUEST:
            hmac_rx_sa_query_req_etc(hmac_vap, netbuf, is_protected);
            break;
        case MAC_SA_QUERY_ACTION_RESPONSE:
            hmac_rx_sa_query_rsp_etc(hmac_vap, netbuf, is_protected);
            break;

        default:
            break;
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
OAL_STATIC osal_void hmac_sta_up_rx_action_protected_he(hmac_vap_stru *hmac_vap, osal_u8 *data)
{
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
        case MAC_PROTECTED_HE_ACTION_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT:
            break;

        case MAC_PROTECTED_HE_ACTION_MU_EDCA_CONTROL:
            hmac_mgmt_rx_mu_edca_control_etc(hmac_vap, data);
            break;

        default:
            break;
    }
}
#endif

OAL_STATIC osal_void hmac_sta_up_rx_action_vht(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, const osal_u8 *data)
{
    unref_param(hmac_vap);
    unref_param(hmac_user);
    unref_param(netbuf);
    switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        case MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION:
            hmac_mgmt_rx_opmode_notify_frame_etc(hmac_vap, hmac_user, netbuf);
            break;
#endif
        case MAC_VHT_ACTION_BUTT:
        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_action
 功能描述  : STA在UP状态下的接收ACTION帧处理
 输入参数  : hmac_vap: HMAC VAP结构体指针
             pst_netbuf  : Action帧所在的netbuf
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_up_rx_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    oal_bool_enum_uint8 is_protected, oal_bool_enum_uint8 *reported_host)
{
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    osal_u8 *data = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OAL_PTR_NULL;          /* 保存mac帧的指针 */
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P0设备所接受的action全部上报 */
    if (is_p2p_mode(hmac_vap->p2p_mode)) {
        hmac_rx_mgmt_send_to_host_etc(hmac_vap, pst_netbuf);
    }
#endif
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    /* 获取帧头信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(&pst_rx_ctrl->rx_info);

    /* 获取发送端的用户指针 */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, frame_hdr->address2);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_sta_up_rx_action::hmac_vap_find_user_by_macaddr_etc failed.}",
                         hmac_vap->vap_id);
        return;
    }

    /* 获取帧体指针 */
    data = (osal_u8 *)mac_get_rx_cb_mac_header_addr(&pst_rx_ctrl->rx_info) +
               pst_rx_ctrl->rx_info.mac_header_len;

    /* Category */
    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_BA:
            hmac_sta_up_rx_action_ba(hmac_vap, hmac_user, frame_hdr, data);
            break;
        case MAC_ACTION_CATEGORY_WNM:
            hmac_sta_up_rx_action_wnm(hmac_vap, hmac_user, pst_netbuf, data, reported_host);
            break;
        case MAC_ACTION_CATEGORY_PUBLIC:
            hmac_sta_up_rx_action_public(hmac_vap, pst_netbuf, data, reported_host);
            break;
#ifdef _PRE_WLAN_FEATURE_PMF
        case MAC_ACTION_CATEGORY_SA_QUERY:
            hmac_sta_up_rx_action_sa_query(hmac_vap, pst_netbuf, data, is_protected);
            break;
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
        case MAC_ACTION_CATEGORY_PROTECTED_HE:
            hmac_sta_up_rx_action_protected_he(hmac_vap, data);
            break;
#endif
        case MAC_ACTION_CATEGORY_VENDOR:
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0) */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            if (hmac_ie_check_p2p_action_etc(data + MAC_ACTION_CATEGORY_AND_CODE_LEN) == OAL_TRUE) {
                hmac_rx_mgmt_send_to_host_etc(hmac_vap, pst_netbuf);
                *reported_host = OSAL_TRUE;
                break;
            }
#endif
            break;
        case MAC_ACTION_CATEGORY_VHT:
            hmac_sta_up_rx_action_vht(hmac_vap, hmac_user, pst_netbuf, data);
            break;
        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_mgmt_etc
 功能描述  : AP在UP状态下的接收管理帧处理
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_sta_up_rx_mgmt_etc(hmac_vap_stru *hmac_vap_sta, osal_void *p_param)
{
    hmac_wlan_crx_event_stru   *mgmt_rx_event = OAL_PTR_NULL;
    dmac_rx_ctl_stru           *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru            *rx_info = OAL_PTR_NULL;
    osal_u8                  *mac_hdr = OAL_PTR_NULL;
    osal_u8                   mgmt_frm_type;
    oal_bool_enum_uint8         is_protected;
    oal_bool_enum_uint8         reported_host = OSAL_FALSE;

    if ((hmac_vap_sta == OAL_PTR_NULL) || (p_param == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_sta_up_rx_mgmt_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_rx_event   = (hmac_wlan_crx_event_stru *)p_param;
    pst_rx_ctrl         = (dmac_rx_ctl_stru *)oal_netbuf_cb(mgmt_rx_event->netbuf);
    rx_info         = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->rx_info));
    mac_hdr         = (osal_u8 *)mac_get_rx_cb_mac_header_addr(rx_info);
    is_protected     = mac_is_protectedframe(mac_hdr);
    if (is_protected >= OAL_BUTT) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_sta_up_rx_mgmt_etc::is_protected is %d.}", is_protected);
        return OAL_SUCC;
    }

    /* STA在UP状态下 接收到的各种管理帧处理 */
    mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (hmac_get_print_wakeup_mgmt_etc() == OAL_TRUE) {
        hmac_set_print_wakeup_mgmt_etc(OSAL_FALSE);
        oam_warning_log1(0, OAM_SF_RX, "hmac_sta_up_rx_mgmt_etc::wakeup mgmt type[0x%x]", mgmt_frm_type);
    }
#endif

    /* Bar frame proc here */
    if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_CTL) {
        mgmt_frm_type = mac_get_frame_sub_type(mac_hdr);
        if (mgmt_frm_type == WLAN_FC0_SUBTYPE_BAR) {
            hmac_up_rx_bar_etc(hmac_vap_sta, pst_rx_ctrl, mgmt_rx_event->netbuf);
        }
    } else if (mac_get_frame_type(mac_hdr) == WLAN_FC0_TYPE_MGT) {
        if (mgmt_frm_type == WLAN_FC0_SUBTYPE_DEAUTH || mgmt_frm_type == WLAN_FC0_SUBTYPE_DISASSOC) {
            hmac_sta_rx_deauth_req(hmac_vap_sta, mac_hdr, is_protected);
        } else if (mgmt_frm_type == WLAN_FC0_SUBTYPE_BEACON) {
            hmac_sta_up_rx_beacon(hmac_vap_sta, mgmt_rx_event->netbuf);
        } else if (mgmt_frm_type == WLAN_FC0_SUBTYPE_ACTION) {
            hmac_sta_up_rx_action(hmac_vap_sta, mgmt_rx_event->netbuf, is_protected, &reported_host);
        } else if (mgmt_frm_type == WLAN_FC0_SUBTYPE_PROBE_RSP) {
            hmac_sta_up_rx_probe_response(hmac_vap_sta, mgmt_rx_event->netbuf);
        }
    }

    return OAL_SUCC;
}

oal_module_symbol(hmac_check_capability_mac_phy_supplicant_etc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

