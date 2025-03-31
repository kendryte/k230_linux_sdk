/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: wal common msg api.
 * Create: 2021-08-16
 */

#include "wal_common.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_hcm.h"
#include "frw_hmac.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif

#include "wlan_msg.h"
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "msg_psm_rom.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_WAL_COMMON_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_u32 g_wal_service_control = 0x5; /* 32位全局变量控制32种业务场景, 默认开启低第1位和第3位, 值为5 */

#if defined(_PRE_WLAN_SUPPORT_CCPRIV_CMD) || defined(_PRE_WLAN_WIRELESS_EXT)
/*****************************************************************************
  协议模式字符串定义
*****************************************************************************/
OAL_CONST wal_ioctl_mode_map_stru g_ast_mode_map_etc[] = {
    /* legacy */
    {"11a",                 WLAN_LEGACY_11A_MODE,       WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11b",                 WLAN_LEGACY_11B_MODE,       WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11bg",                WLAN_MIXED_ONE_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},

    {"11g2g5",              WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_5M},
    {"11g2g10",             WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_10M},
    {"11g2g20",             WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},

    {"11g5g40plus",         WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11g5g80plusplus",     WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},

    /* 11n */
    {"11n2g5",              WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_5M},
    {"11n2g10",             WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_10M},
    {"11n2g20",             WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11n2g40",             WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11n2g40plus",         WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11n2g40minus",        WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},

    {"11n5g5",              WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_5M},
    {"11n5g10",             WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_10M},
    {"11n5g20",             WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11n5g40",             WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11n5g40plus",         WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11n5g40minus",        WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11n5g80plusplus",     WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11n5g80plusminus",    WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11n5g80minusplus",    WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11n5g80minusminus",   WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},

    /* 11ac */
    {"11ac5",               WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_5M},
    {"11ac10",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_10M},
    {"11ac20",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11ac40",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ac40plus",          WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ac40minus",         WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ac80",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80M},
    {"11ac80plusplus",      WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11ac80plusminus",     WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11ac80minusplus",     WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11ac80minusminus",    WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},

    /* 协议上不支持11ac 2.4G */
    {"11ac2g5",             WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_5M},
    {"11ac2g10",            WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_10M},
    {"11ac2g20",            WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11ac2g40",            WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11ac2g40plus",        WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ac2g40minus",       WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},

    /* 11n only and 11ac only, 都是20M带宽 */
    {"11nonly2g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11nonly5g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11aconly",            WLAN_VHT_ONLY_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 11ax */
    {"11ax2g20",            WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11ax2g40",            WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11ax2g40plus",        WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ax2g40minus",       WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},

    {"11ax5g20",            WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11ax5g40",            WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ax5g40plus",        WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ax5g40minus",       WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ax5g80",            WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_80M},
    {"11ax5g80plusplus",    WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11ax5g80plusminus",   WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11ax5g80minusplus",   WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11ax5g80minusminus",  WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},
#endif

    {OAL_PTR_NULL}
};
#endif

/* wal post async msg to hmac without response */
osal_s32 wal_async_post2hmac_no_rsp(osal_u8 vap_id, osal_u16 msg_id,
    osal_u8 *data, osal_u32 data_len, frw_post_pri_enum_uint8 pri)
{
    osal_s32 ret;
    frw_msg msg_info;

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = data;
    msg_info.data_len = (osal_u16)data_len;

    ret = frw_asyn_host_post_msg(msg_id, pri, vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "{msg_id[%d] wal_async_post2hmac_no_rsp::frw_asyn_host_post_msg failed[%d].}", msg_id, ret);
    }
    return ret;
}

/* wal post sync msg to hmac without response */
osal_s32 wal_sync_post2hmac_no_rsp(osal_u8 vap_id, osal_u16 msg_id, osal_u8 *data, osal_u32 data_len)
{
    osal_s32 ret;
    frw_msg msg_info;

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = data;
    msg_info.data_len = (osal_u16)data_len;

    ret = frw_sync_host_post_msg(msg_id, vap_id, WAL_MSG_TIME_OUT_MS, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "{msg_id[%d] wal_sync_post2hmac_no_rsp::frw_sync_host_post_msg failed[%d].}", msg_id, ret);
    }
    return ret;
}

/* wal send sync msg to device without response */
osal_s32 wal_sync_send2device_no_rsp(osal_u8 vap_id, osal_u16 msg_id, osal_u8 *data, osal_u32 data_len)
{
    osal_s32 ret;
    frw_msg msg_info = {0};

    msg_info.data = data;
    msg_info.data_len = (osal_u16)data_len;

    ret = frw_send_cfg_to_device(msg_id, vap_id, OSAL_TRUE, WAL_MSG_TIME_OUT_MS, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "{msg_id[%d] wal_sync_send2device_no_rsp::frw_send_cfg_to_device failed[%d].}", msg_id, ret);
        return ret;
    }
    return ret;
}

/* 将W2H和W2D的消息接口对外提供出去 */
oal_module_symbol(wal_sync_post2hmac_no_rsp);
oal_module_symbol(wal_sync_send2device_no_rsp);

/* wal send async msg to device without response */
osal_s32 wal_async_send2device_no_rsp(osal_u8 vap_id, osal_u16 msg_id, osal_u8 *data, osal_u32 data_len)
{
    osal_s32 ret;
    frw_msg msg_info;

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    msg_info.data = data;
    msg_info.data_len = (osal_u16)data_len;

    ret = frw_send_cfg_to_device(msg_id, vap_id, OSAL_FALSE, WAL_MSG_TIME_OUT_MS, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "{msg_id[%d] wal_sync_send2device_no_rsp::frw_send_cfg_to_device failed[%d].}", msg_id, ret);
        return ret;
    }
    return ret;
}
oal_module_symbol(wal_async_send2device_no_rsp);

osal_s32 wal_set_ssid(oal_net_device_stru *net_dev, const osal_u8 *ssid_ie, osal_u8 ssid_len)
{
    mac_cfg_ssid_param_stru ssid_info;

    (osal_void)memset_s(&ssid_info, OAL_SIZEOF(ssid_info), 0, OAL_SIZEOF(ssid_info));
    ssid_info.ssid_len = ssid_len;

    if (memcpy_s(ssid_info.ac_ssid, OAL_SIZEOF(ssid_info.ac_ssid), ssid_ie, ssid_len) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_ssid::memcpy_s error, return.}");
    }

    return wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SSID,
        (osal_u8 *)&ssid_info, OAL_SIZEOF(ssid_info));
}

osal_s32 wal_set_mode(oal_net_device_stru *net_dev, const wal_ioctl_mode_map_stru *mode_map,
    const osal_s8 *mode_str)
{
    osal_u8 prot_idx;
    mac_cfg_mode_param_stru mode_info;

    for (prot_idx = 0; mode_map[prot_idx].pc_name != OAL_PTR_NULL; prot_idx++) {
        if (osal_strcmp((const osal_char *)mode_map[prot_idx].pc_name, (const osal_char *)mode_str) == 0) {
            break;
        }
    }

    if (mode_map[prot_idx].pc_name == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_set_mode::unrecognized protocol string!}");
        return -OAL_EINVAL;
    }

    (osal_void)memset_s(&mode_info, OAL_SIZEOF(mode_info), 0, OAL_SIZEOF(mode_info));
    mode_info.protocol = mode_map[prot_idx].mode;
    mode_info.band = mode_map[prot_idx].band;
    mode_info.en_bandwidth = mode_map[prot_idx].en_bandwidth;

    return wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_MODE,
        (osal_u8 *)&mode_info, OAL_SIZEOF(mode_info));
}

osal_s32 wal_set_channel_info(osal_u8 vap_id, osal_u8 channel,
    wlan_channel_band_enum_uint8 band, wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    mac_cfg_channel_param_stru channel_info;
    (osal_void)memset_s(&channel_info, OAL_SIZEOF(channel_info), 0, OAL_SIZEOF(channel_info));

    channel_info.channel   = channel;
    channel_info.band      = band;
    channel_info.en_bandwidth = bandwidth;

    oam_warning_log4(0, OAM_SF_ANY,
        "vap_id[%d] {wal_set_channel_info::channel = %d, band = %d, en_bandwidth = %d.}", vap_id,
        channel_info.channel,
        channel_info.band,
        channel_info.en_bandwidth);

    return wal_sync_post2hmac_no_rsp(vap_id, WLAN_MSG_W2H_CFG_CFG80211_SET_CHANNEL,
        (osal_u8 *)&channel_info, OAL_SIZEOF(channel_info));
}

osal_s32 wal_down_vap(oal_net_device_stru *net_dev, osal_u8 vap_id)
{
    osal_s32 ret;
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8 p2p_mode;
    oal_wireless_dev_stru *wdev = OAL_PTR_NULL;
#endif
    mac_cfg_start_vap_param_stru vap_info;
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

#ifdef _PRE_WLAN_FEATURE_P2P
    wdev = net_dev->ieee80211_ptr;
    if (wdev == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_down_vap:: wdev null");
        return -OAL_EINVAL;
    }
    p2p_mode = wal_wireless_iftype_to_mac_p2p_mode_etc(wdev->iftype);
    if (p2p_mode == WLAN_P2P_BUTT) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_down_vap::wal_wireless_iftype_to_mac_p2p_mode_etc return BUFF}");
        return -OAL_EINVAL;
    }
    oam_warning_log1(0, OAM_SF_ANY, "{wal_down_vap::p2p_mode=%u}", p2p_mode);

    vap_info.p2p_mode = p2p_mode;
#endif

    vap_info.net_dev = net_dev;

    ret = wal_sync_post2hmac_no_rsp(vap_id, WLAN_MSG_W2H_CFG_DOWN_VAP, (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {wal_down_vap::failed to down vap, error[%d]}", vap_id, ret);
        wifi_printf("vap_id[%d] {wal_down_vap::failed to down vap, error[%d]}\r\n", vap_id, ret);
    }
    return ret;
}

osal_s32 wal_add_vap(oal_net_device_stru *net_dev, osal_u8 vap_id, osal_u8 vap_mode, osal_u8 p2p_mode)
{
    osal_s32 ret;
    hmac_cfg_add_vap_param_stru vap_info;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
#endif
    unref_param(p2p_mode);
    (osal_void)memset_s(&vap_info, OAL_SIZEOF(vap_info), 0, OAL_SIZEOF(vap_info));

    vap_info.net_dev = net_dev;
    vap_info.add_vap.vap_mode  = vap_mode;
    vap_info.add_vap.cfg_vap_indx = vap_id;
#ifdef _PRE_WLAN_FEATURE_P2P
    vap_info.add_vap.p2p_mode  = p2p_mode;
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    vap_info.add_vap.disable_capab_2ght40 = wlan_customize_etc->disable_capab_2ght40;
    wal_add_vap_set_cust(&vap_info);
#endif
    ret = wal_sync_post2hmac_no_rsp(vap_id, WLAN_MSG_W2H_CFG_ADD_VAP, (osal_u8 *)&vap_info, OAL_SIZEOF(vap_info));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {wal_add_vap::failed to add vap, error[%d]}", vap_id, ret);
        wifi_printf("vap_id[%d] {wal_add_vap::failed to add vap, error[%d]}\r\n", vap_id, ret);
    }
    return ret;
}

osal_s32 wal_set_ap_max_user(oal_net_device_stru *net_dev, osal_u32 ap_max_user)
{
    osal_s32                   l_ret;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user:: ap_max_user is : %u.}", ap_max_user);

    if (ap_max_user == 0) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_set_ap_max_user::invalid ap max user(%u),ignore this set.}", ap_max_user);
        return OAL_SUCC;
    }
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_MAX_USER,
        (osal_u8 *)&ap_max_user, OAL_SIZEOF(ap_max_user));
    if (l_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_set_ap_max_user:: wal_sync_post2hmac_no_rsp return err code %d!}", l_ret);
        return l_ret;
    }
    /* o版本先设置最大用户数，再启动aput，需要保存该配置，在启动aput的时候用，不能删除 */
    /* 每次设置最大用户数完成后，都清空为非法值0 */
    return l_ret;
}

osal_s32 wal_get_rx_pckg(hmac_vap_stru *hmac_vap, osal_u32 data_op, osal_u32 print_info)
{
    osal_s32                    ret;
    mac_cfg_al_rx_info_stru    al_rx_info = {0};
    frw_msg                     cfg_info;
    dmac_atcmdsrv_atcmd_response_event fill_msg = {0};
    dmac_atcmdsrv_atcmd_response_event *rx_pkcg_event = OSAL_NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_u8 rx_loss = 0;
    mac_channel_stru channel = hmac_vap->channel;
#endif

    hmac_vap->atcmdsrv_get_status.get_rx_pkct_flag = OAL_FALSE;

    /* 设置配置命令参数 */
    /* 这两个参数在02已经没有意义 */
    al_rx_info.data_op = data_op;
    al_rx_info.print_info = print_info;

    memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
    cfg_msg_init((osal_u8 *)&al_rx_info, OAL_SIZEOF(al_rx_info),
        (osal_u8 *)&fill_msg, OAL_SIZEOF(fill_msg), &cfg_info);

    ret = frw_send_cfg_to_device(WLAN_MSG_W2D_C_CFG_AL_RX_INFO, hmac_vap->vap_id, OSAL_TRUE,
        WAL_ATCMDSRB_GET_RX_PCKT, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_get_rx_pckg:: return err code %d!}", ret);
        return ret;
    }

    rx_pkcg_event = (dmac_atcmdsrv_atcmd_response_event *)(cfg_info.rsp);
    if (rx_pkcg_event->event_id == OAL_ATCMDSRV_GET_RX_PKCG) {
        hmac_vap->atcmdsrv_get_status.rx_pkct_succ_num = rx_pkcg_event->event_para;
        hmac_vap->atcmdsrv_get_status.rx_ampdu_succ_num = rx_pkcg_event->data[0];
        hmac_vap->atcmdsrv_get_status.rx_fail_num = rx_pkcg_event->fail_num;
        hmac_vap->atcmdsrv_get_status.s_rx_rssi = rx_pkcg_event->always_rx_rssi;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
        cfg_msg_init((osal_u8 *)&channel, sizeof(channel), &rx_loss, sizeof(rx_loss), &cfg_info);
        ret = send_sync_cfg_to_host(hmac_vap->vap_id, WLAN_MSG_W2H_CFG_GET_RF_FE_RX_INSERT_LOSS, &cfg_info);
        if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
            oam_error_log1(0, OAM_SF_ANY, "{wal_get_rx_pckg::send_sync_cfg_to_host return err code %d!}", ret);
            return ret;
        }
        rx_loss = *cfg_info.rsp;
        hmac_vap->atcmdsrv_get_status.s_rx_rssi += rx_loss;
        if (hmac_vap->atcmdsrv_get_status.s_rx_rssi > oal_get_real_rssi(OAL_RSSI_INIT_MARKER)) {
            hmac_vap->atcmdsrv_get_status.s_rx_rssi = oal_get_real_rssi(OAL_RSSI_INIT_MARKER);
        }
#endif
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    hmac_vap->atcmdsrv_get_status.get_rx_pkct_flag = OAL_TRUE;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
OAL_STATIC osal_void wal_always_tx_str_to_hex(osal_s8 *param, osal_s16 len, osal_u8 *value)
{
    osal_s16 index;
    osal_u8 hex_num;
    if (param == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "wal_always_tx_str_to_hex input param is NULL.");
        return;
    }
    /* wal_always_tx_str_to_hex 未进行非法字符判断 对于非法字符会置0 */
    for (index = 0; index < len; index++) {
        hex_num = oal_strtohex(param);
        /* 当oal_strtohex输出0xff时 判断为非法字符 */
        if (hex_num == 0xff) {
            hex_num = 0;
        }
        /* 每2位字符转为一个十六进制，第一位乘以16 */
        value[index / 2] = value[index / 2] * 16 * (index % 2) + hex_num;
        param++;
    }
}

OAL_STATIC osal_u32 wal_always_tx_get_payload(osal_s8 *param, osal_u32 off_set,
    mac_cfg_tx_comp_stru *set_bcast_param)
{
    osal_u32 ret;
    osal_s8 *pkt_name = OSAL_NULL;
    osal_u8 *pkt_data = OSAL_NULL;
    osal_u32 pkt_len, tmp_len;

    /* 数组申请内存 */
    pkt_name = (osal_s8 *)oal_mem_alloc(OAL_MGMT_NETBUF, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN, OAL_NETBUF_PRIORITY_MID);
    if (pkt_name == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_always_tx_get_payload::pkt_name malloc fail!}");
        return OAL_FAIL;
    }
    (osal_void)memset_s(pkt_name, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN, 0, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN);

    ret = wal_get_cmd_one_arg(param, pkt_name, WAL_CCPRIV_CMD_PKT_NAME_MAX_LEN, &off_set);
    /* 传入的是十六进制字符，长度为name真实长度的2倍 */
    tmp_len = (osal_u32)osal_strlen((const osal_char *)pkt_name);
    if ((ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_ANY, "{wal_always_tx_get_payload::err %d!, strlen(%d).}", ret, tmp_len);
        oal_mem_free(pkt_name, OAL_TRUE);
        return OAL_FAIL;
    }

    /* 获取报文长度等于字符串长度除以2 */
    pkt_len = tmp_len / 2;
    if ((pkt_len < WAL_CCPRIV_CMD_PKT_MIN_LEN) || (pkt_len > WAL_CCPRIV_CMD_PKT_MAX_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "wal_always_tx_get_payload invalid input len[%d].", tmp_len);
        oal_mem_free(pkt_name, OAL_TRUE);
        return OAL_FAIL;
    }

    /* 申请用户侧内存 存放报文信息 */
    pkt_data = oal_mem_alloc(OAL_MGMT_NETBUF, (osal_u16)pkt_len, OAL_NETBUF_PRIORITY_MID);
    if (pkt_data == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_always_tx_get_payload::data fail(size:%u), return null!}", pkt_len);
        oal_mem_free(pkt_name, OAL_TRUE);
        return OAL_FAIL;
    }
    (osal_void)memset_s(pkt_data, pkt_len, 0, pkt_len);

    wal_always_tx_str_to_hex(pkt_name, (osal_s16)tmp_len, pkt_data);
    oal_mem_free(pkt_name, OAL_TRUE); /* 释放数组内存 */

    oam_warning_log4(0, OAM_SF_ANY, "{wal_always_tx_get_payload::create pkt:len[%d]::%02x xxxxxxx %02x %02x}",
        pkt_len, pkt_data[0], pkt_data[pkt_len - 2], pkt_data[pkt_len - 1]); /* 维测：打印数据包后1 2个字节的信息 */

    set_bcast_param->payload = (osal_u8 *)pkt_data;
    set_bcast_param->pkt_len = pkt_len;
    return OAL_SUCC;
}

osal_u32 wal_ccpriv_always_tx_get_param(osal_s8 *param, osal_u32 off_set,
    mac_rf_payload_enum_uint8 *payload_flag, osal_u32 *len, mac_cfg_tx_comp_stru *set_bcast_param)
{
    osal_u32 ret;
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN];
    osal_u32 payload_len = 0;
    mac_rf_payload_enum_uint8 rf_payload_flag = 0;
    osal_u8 tpc_code = 0xFF;

    param = param + off_set;
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    /* 若后面payload_flag和len参数没有设置，采用默认RF_PAYLOAD_RAND 2000 */
    if (ret == OAL_SUCC) {
        rf_payload_flag = (osal_u8)oal_atoi((const osal_s8 *)name);
        if (rf_payload_flag >= RF_PAYLOAD_BUTT) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_always_tx_get_param::payload flag err[%d]!}", rf_payload_flag);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        /* 获取len参数 */
        param = param + off_set;
        ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_always_tx_get_param::get_cmd_one_arg err_code [%d]!}", ret);
            return ret;
        }
        payload_len = (osal_u16)oal_atoi((const osal_s8 *)name);
        /* 65535防止输入负数 */
        if (payload_len > 65535) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_ccpriv_always_tx_get_param::len [%u] overflow!}", payload_len);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        if (rf_payload_flag == RF_PAYLOAD_DATA) {
            param = param + off_set;
            ret = wal_always_tx_get_payload(param, off_set, set_bcast_param);
            if (ret != OAL_SUCC) {
                return ret;
            }
        }
        /* 获取tpc_code参数 */
        param = param + off_set;
        ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
        if (ret == OAL_SUCC) {
            tpc_code = (osal_u8)oal_atoi((const osal_s8 *)name);
            if (tpc_code >= 148) { // tpc code不能超过148
                wifi_printf("al_tx tpc_code invalid:%d.\r\n", tpc_code);
                tpc_code = 0xFF;
            }
        }
        set_bcast_param->tpc_code = tpc_code;
    }
    *payload_flag = rf_payload_flag;
    *len = payload_len;
    return OAL_CONTINUE;
}
#endif
#ifdef _PRE_WLAN_FEATURE_P2P

/*****************************************************************************
 输入参数  : enum nl80211_iftype iftype
 输出参数  : 无
 返 回 值  : wlan_p2p_mode_enum_uint8

 修改历史      :
  1.日    期   : 2014年12月31日
    修改内容   : 新生成函数

*****************************************************************************/
wlan_p2p_mode_enum_uint8 wal_wireless_iftype_to_mac_p2p_mode_etc(enum nl80211_iftype iftype)
{
    wlan_p2p_mode_enum_uint8 p2p_mode = WLAN_LEGACY_VAP_MODE;

    switch (iftype) {
        case NL80211_IFTYPE_P2P_CLIENT:
            p2p_mode = WLAN_P2P_CL_MODE;
            break;
        case NL80211_IFTYPE_P2P_GO:
            p2p_mode = WLAN_P2P_GO_MODE;
            break;
        case NL80211_IFTYPE_P2P_DEVICE:
            p2p_mode = WLAN_P2P_DEV_MODE;
            break;
#ifdef _PRE_WLAN_FEATURE_MESH
        case NL80211_IFTYPE_MESH_POINT:
#endif
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_STATION:
            p2p_mode = WLAN_LEGACY_VAP_MODE;
            break;
        default:
            p2p_mode = WLAN_P2P_BUTT;
    }
    return p2p_mode;
}
#endif

osal_void wal_set_ac_2g_enable(hmac_cfg_add_vap_param_stru *vap_info)
{
#if (!defined _PRE_WLAN_FEATURE_11AC2G)
    vap_info->add_vap.is_11ac2g_enable = OAL_FALSE;
#else
    vap_info->add_vap.is_11ac2g_enable = OAL_TRUE;
#endif
}

OAL_STATIC osal_void wal_set_service_control(osal_u32 value)
{
    g_wal_service_control = value;

    return;
}

osal_u32 wal_get_service_control(osal_void)
{
    return g_wal_service_control;
}

osal_u32 wal_set_service_control_etc(const service_control_msg *msg)
{
    osal_u32 tmp_mask = msg->mask;
    osal_u32 tmp_value = msg->value;
    osal_u32 tmp = wal_get_service_control();

    if (tmp_mask == 0xffffffff) {
        /* 修改整个变量的值 */
        wal_set_service_control(tmp_value);
        return OAL_SUCC;
    }

    /* 修改单个bit位的值, 该值为0或1, tmp_mask表示配置哪个bit位,从0~31中的1位 */
    if ((tmp_value != 0) && (tmp_value != 1)) {
        wifi_printf("{wal_set_service_control::tmp_mask 0x%x, tmp_value 0x%x error}\r\n", tmp_mask, tmp_value);
        return OAL_FAIL;
    }

    if (((tmp >> tmp_mask) & 0x1) == 0) {
        /* 该bit位原值为0, 则 | 上配置的值 */
        tmp |= (tmp_value << tmp_mask);
    } else {
        /* 该bit位原值为1, 则 & 上配置的值 */
        tmp &= (tmp_value << tmp_mask);
    }

    wal_set_service_control(tmp);
    return OAL_SUCC;
}

#if defined(_PRE_WIFI_DEBUG) || defined(_PRE_WLAN_FEATURE_WFA_CODE)
/*****************************************************************************
 功能描述  : 判断是否是dhcp帧 port
*****************************************************************************/
oal_bool_enum_uint8 wal_is_dhcp_port_etc(const mac_ip_header_stru *ip_hdr)
{
    mac_udp_header_stru *udp_hdr = OSAL_NULL;
    osal_u32 ip_hdr_len;
    /* DHCP判断标准: udp协议，ip头部fragment offset字段为0，目的端口号为67或68 */
    if (ip_hdr->protocol == MAC_UDP_PROTOCAL && ((ip_hdr->frag_off & 0xFF1F) == 0)) {
        ip_hdr_len = mac_ip_hdr_len(ip_hdr);
        udp_hdr = (mac_udp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
        /* 67, 68为DHCP端口号 */
        if (oal_net2host_short(udp_hdr->des_port) == 67 || oal_net2host_short(udp_hdr->des_port) == 68) {
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 获取arp帧的类型：request/responce(暂时不考虑rarp!)
*****************************************************************************/
mac_data_type_enum_uint8 wal_get_arp_type_by_arphdr(const oal_eth_arphdr_stru *rx_arp_hdr)
{
    if (oal_net2host_short(rx_arp_hdr->ar_op) == MAC_ARP_REQUEST) {
        return MAC_DATA_ARP_REQ;
    } else if (oal_net2host_short(rx_arp_hdr->ar_op) == MAC_ARP_RESPONSE) {
        return MAC_DATA_ARP_RSP;
    }

    return MAC_DATA_BUTT;
}

/*****************************************************************************
 功能描述  : 根据数据帧(802.3)的类型，判断帧类型
*****************************************************************************/
osal_u8 wal_get_data_type_from_8023_etc(const osal_u8 *frame_hdr, mac_netbuff_payload_type hdr_type)
{
    mac_ip_header_stru *ip = OSAL_NULL;
    const osal_u8 *frame_body = OSAL_NULL;
    osal_u16 ether_type;
    osal_u8 datatype = MAC_DATA_BUTT;

    if ((frame_hdr == OSAL_NULL) ||
        ((hdr_type != MAC_NETBUFF_PAYLOAD_ETH) && (hdr_type != MAC_NETBUFF_PAYLOAD_SNAP))) {
        return datatype;
    }

    ether_type = (hdr_type == MAC_NETBUFF_PAYLOAD_ETH) ? ((mac_ether_header_stru *)frame_hdr)->ether_type :
                                                         ((mac_llc_snap_stru *)frame_hdr)->ether_type;
    frame_body = (hdr_type == MAC_NETBUFF_PAYLOAD_ETH) ? frame_hdr + (osal_u16)sizeof(mac_ether_header_stru) :
                                                         frame_hdr + (osal_u16)sizeof(mac_llc_snap_stru);

    if (ether_type == oal_host2net_short(ETHER_TYPE_IP)) {
        /* 从IP TOS字段寻找优先级 */
        /* ----------------------------------------------------------------------
            tos位定义
         ----------------------------------------------------------------------
        | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
        | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
         ---------------------------------------------------------------------- */
        ip = (mac_ip_header_stru *)frame_body; /* 偏移一个以太网头，取ip头 */

        if (wal_is_dhcp_port_etc(ip) == OSAL_TRUE) {
            datatype = MAC_DATA_DHCP;
        }

        if (ip->protocol == MAC_ICMP_PROTOCAL) {
            if (*(osal_u8*)(ip + 1) == MAC_ICMP_REQUEST) {
                datatype = MAC_DATA_ICMP_REQ;
            } else if (*(osal_u8*)(ip + 1) == MAC_ICMP_RESPONSE) {
                datatype = MAC_DATA_ICMP_RSP;
            } else {
                datatype = MAC_DATA_ICMP_OTH;
            }
        }
    } else if (ether_type == oal_host2net_short(ETHER_TYPE_ARP)) {
        datatype = (osal_u8)wal_get_arp_type_by_arphdr((oal_eth_arphdr_stru *)frame_body);
    } else if (ether_type == oal_host2net_short(ETHER_TYPE_PAE)) {
        datatype = MAC_DATA_EAPOL; /* eapol */
    } else {
        datatype = MAC_DATA_BUTT;
    }

    return datatype;
}

osal_void wal_print_tx_data_type(mac_ether_header_stru *ether_header)
{
    td_u8 datatype;

    datatype = wal_get_data_type_from_8023_etc((osal_u8 *)ether_header, MAC_NETBUFF_PAYLOAD_ETH);
    switch (datatype) {
        case MAC_DATA_ARP_REQ:
            wifi_printf("[TX][Lwip]PKT_TRACE_ARP_REQ\r\n");
            break;
        case MAC_DATA_ARP_RSP:
            wifi_printf("[TX][Lwip]PKT_TRACE_ARP_RSP\r\n");
            break;
        case MAC_DATA_DHCP:
            wifi_printf("[TX][Lwip]PKT_TRACE_DHCP\r\n");
            break;
        case MAC_DATA_ICMP_REQ:
            wifi_printf("[TX][Lwip]PKT_TRACE_ICMP_REQ\r\n");
            break;
        case MAC_DATA_ICMP_RSP:
            wifi_printf("[TX][Lwip]PKT_TRACE_ICMP_RSP\r\n");
            break;
        case MAC_DATA_ICMP_OTH:
            wifi_printf("[TX][Lwip]PKT_TRACE_ICMP_OTH\r\n");
            break;
        case MAC_DATA_EAPOL:
            wifi_printf("[TX][Lwip]PKT_MAC_DATA_EAPOL\r\n");
            break;
        default:
            break;
    }
}
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_void wal_add_vap_set_cust(hmac_cfg_add_vap_param_stru *add_vap_param)
{
    osal_s32 priv_value = 0;
    osal_s32 ret;
    if (add_vap_param == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_add_vap_set_cust::add_vap_param is null ptr!}");
        return;
    }
    wal_set_ac_2g_enable(add_vap_param);
    /* 两个！使结果为0或1，方便存储在1bit大小成员中 */
    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_ER_SU_DISABLE, &priv_value);
    if (ret == OAL_SUCC) {
        add_vap_param->add_vap.er_su_disable = (priv_value == 0) ? 0 : 1;
    } else {
        add_vap_param->add_vap.er_su_disable = 0;
    }
    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_DCM_CONSTELLATION_TX, &priv_value);
    if (ret == OAL_SUCC) {
        add_vap_param->add_vap.dcm_constellation_tx = priv_value > 0x3 ? 0x3 : (osal_u8)priv_value;
    } else {
        add_vap_param->add_vap.dcm_constellation_tx = 0x3; /* 0:不支持DCM,1:BPSK,2:QPSK,3:16-QAM */
    }
    ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_BANDWIDTH_EXTENDED_RANGE, &priv_value);
    if (ret == OAL_SUCC) {
        add_vap_param->add_vap.bandwidth_extended_range = (priv_value == 0) ? 0 : 1;
    } else {
        add_vap_param->add_vap.bandwidth_extended_range = 1;
    }
}
#endif

#ifdef PRE_WLAN_FEATURE_BLACKLIST
osal_u32 uapi_ccpriv_blacklist_add(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 off_set = 0;
    mac_blacklist_stru blacklist;
    (osal_void)memset_s(&blacklist, OAL_SIZEOF(blacklist), 0, OAL_SIZEOF(blacklist));

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_blacklist_add:wal_get_cmd_one_arg fail!}");
        return ret;
    }

    oal_strtoaddr((const osal_char *)name, blacklist.auc_mac_addr); /* 将字符 ac_name 转换成数组 mac_add[6] */
    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_ADD_BLACK_LIST,
        (osal_u8 *)&blacklist, OAL_SIZEOF(blacklist));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{uapi_ccpriv_blacklist_add:wal_sync_post2hmac_no_rsp return[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

osal_u32 uapi_ccpriv_blacklist_del(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_s8 name[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 off_set = 0;
    osal_u8 mac[OAL_MAC_ADDR_LEN] = {0};

    ret = wal_get_cmd_one_arg(param, name, OAL_SIZEOF(name), &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_ccpriv_blacklist_add:wal_get_cmd_one_arg fail!}");
        return ret;
    }
    oal_strtoaddr((const osal_char *)name, (osal_u8 *)mac); /* 将字符 ac_name 转换成数组 mac_add[6] */

    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_DEL_BLACK_LIST,
        (osal_u8 *)mac, OAL_MAC_ADDR_LEN);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{uapi_ccpriv_blacklist_add:wal_sync_post2hmac_no_rsp return[%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}

osal_u32 uapi_ccpriv_blacklist_show(oal_net_device_stru *net_dev, osal_s8 *param)
{
    osal_u32 ret;
    osal_u32 value = 1;
    unref_param(param);
    ret = (osal_u32)wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_BLACKLIST_SHOW,
        (osal_u8 *)&value, OAL_SIZEOF(value));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{uapi_ccpriv_blacklist_show:wal_sync_post2hmac_no_rsp return [%d].}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

/* tag: 字符头 data: 数据 count: 数据个数 is_hex: 打印格式用16进制 限制: 字符串长度128 */
static osal_void wal_print_tx_power_ini_common(const osal_char *tag, const osal_u8 *data,
    osal_u8 count, osal_u8 is_hex)
{
    osal_u16 idx;
    osal_s32 offset = 0;
    osal_s32 ret = -1;
    char str[NV_PRINT_LEN] = ""; // 字符长度
    if (tag == OSAL_NULL || data == OSAL_NULL || count == 0) {
        return;
    }

    // 根据是否16进制打印输出
    if (is_hex == OSAL_TRUE) {
        ret = snprintf_s(str + offset, NV_PRINT_LEN - offset, NV_PRINT_LEN - offset, "0x%2X", data[0]);
    } else {
        ret = snprintf_s(str + offset, NV_PRINT_LEN - offset, NV_PRINT_LEN - offset, "%u", data[0]);
    }
    if (ret < 0) {
        return;
    }
    offset += ret;
    for (idx = 1; idx < count; idx++) {
        if (is_hex == OSAL_TRUE) {
            ret = snprintf_s(str + offset, NV_PRINT_LEN - offset, NV_PRINT_LEN - offset, ",0x%2X", data[idx]);
        } else {
            ret = snprintf_s(str + offset, NV_PRINT_LEN - offset, NV_PRINT_LEN - offset, ",%u", data[idx]);
        }
        if (ret < 0) {
            return;
        }
        offset += ret;
    }
    wifi_printf("%s=%s;\r\n", tag, str);
}

#ifdef _PRE_WLAN_FEATURE_11D
static osal_void wal_print_tx_power_ini(wlan_cust_rf_fe_power_params rf_power)
{
    osal_u8 offset = 4;
    osal_u8 *data = OSAL_NULL;

    wal_print_tx_power_ini_common("rf_chip_max_power_2g",
        rf_power.chip_max_power_2g, SOC_CUSTOM_RF_FE_MAX_POWER_NUM_2G, OSAL_FALSE);

    wal_print_tx_power_ini_common("target_tx_power_2g_11b", rf_power.target_power_2g, 4, OSAL_TRUE);   // 偏移 0 参数 4 个
    wal_print_tx_power_ini_common("target_tx_power_2g_11g",
        rf_power.target_power_2g + 4, 8, OSAL_TRUE);    // 偏移 4 参数 8 个
    wal_print_tx_power_ini_common("target_tx_power_2g_20m",
        rf_power.target_power_2g + 12, 10, OSAL_TRUE);  // 偏移 12 参数 10 个
    wal_print_tx_power_ini_common("target_tx_power_2g_40m",
        rf_power.target_power_2g + 22, 11, OSAL_TRUE);  // 偏移 22 参数 11 个

    data = rf_power.limit_power_2g;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch1", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch2", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch3", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch4", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch5", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch6", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch7", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch8", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch9", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch10", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch11", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch12", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch13", data, offset, OSAL_TRUE);
    data += offset;
    wal_print_tx_power_ini_common("limit_tx_power_2g_ch14", data, offset, OSAL_TRUE);

    wal_print_tx_power_ini_common("sar_tx_power_2g",
        rf_power.sar_power_2g, SOC_CUSTOM_RF_FE_SAR_POWER_NUM_2G, OSAL_TRUE);
}

/* 打印全部定制化功率 */
osal_void wal_print_nvram_list(osal_void)
{
    wlan_cust_rf_fe_params *rf_fe_ini = hwifi_get_rf_fe_custom_ini();

    wal_print_tx_power_ini(rf_fe_ini->rf_power);
    return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
