/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * 文 件 名   : hmac_dhcp_offload.c
 * 生成日期   : 2022年10月14日
 * 功能描述   : DHCP Offloading相关函数实现
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_dhcp_offload.h"
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "wlan_msg.h"
#include "oal_netbuf_data.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_DHCP_OFFLOAD_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
dhcp_record_frame g_dhcp_request = {0};
mac_ieee80211_frame_stru g_dhcp_request_header = {0};
osal_u32 g_lease_time = 0; /* DHCP ACK报文学到的租期时间 以主机序保存 */
osal_u32 g_request_ip = 0; /* DHCP ACK报文学到的本机ip地址 以网络序保存 */
osal_u32 g_server_ip = 0;  /* DHCP ACK报文学到的server ip地址 以网络序保存 */
osal_u32 g_dhcp_offload_timeout = 0;
osal_u32 g_dhcp_renew_retry = 0;
mac_cipher_info_stru g_cipher_info = {0};

osal_void hmac_dhcp_offload_get_param(mac_dhcp_offload_param_sync *dhcp_param)
{
    memcpy_s(&dhcp_param->dhcp_request, sizeof(dhcp_record_frame), &g_dhcp_request, sizeof(dhcp_record_frame));
    memcpy_s(&dhcp_param->dhcp_request_header, sizeof(mac_ieee80211_frame_stru), &g_dhcp_request_header,
        sizeof(mac_ieee80211_frame_stru));
    memcpy_s(&dhcp_param->cipher_info, sizeof(mac_cipher_info_stru), &g_cipher_info,
        sizeof(mac_cipher_info_stru));

    dhcp_param->lease_time = g_lease_time;
    dhcp_param->request_ip = g_request_ip;
    dhcp_param->server_ip = g_server_ip;
    dhcp_param->dhcp_offload_timeout = g_dhcp_offload_timeout;
    dhcp_param->dhcp_renew_retry = g_dhcp_renew_retry;
}

osal_void hmac_wow_save_cipher_info(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
   /* 学习单播数据报文的加密类型 */
    if (!ether_is_multicast(((mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf))->address1)) {
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb((oal_netbuf_stru *)netbuf);
        if (g_cipher_info.cipher_key_type != tx_ctl->cipher_key_type ||
            g_cipher_info.cipher_protocol_type != tx_ctl->cipher_protocol_type ||
            g_cipher_info.cipher_key_id != tx_ctl->cipher_key_id ||
            g_cipher_info.ra_lut_index != tx_ctl->ra_lut_index) {
            g_cipher_info.cipher_key_type = tx_ctl->cipher_key_type;
            g_cipher_info.cipher_protocol_type = tx_ctl->cipher_protocol_type;
            g_cipher_info.cipher_key_id = tx_ctl->cipher_key_id;
            g_cipher_info.ra_lut_index = tx_ctl->ra_lut_index;
        }
    }
}

osal_void hmac_dhcp_offload_set_timeout(osal_u32 dhcp_offload_timeout)
{
    g_dhcp_offload_timeout = dhcp_offload_timeout + ((osal_u32)TIMER_SECONDS_TO_MS * g_lease_time) / 2; /* 2租期的一半 */
}

osal_u8* mac_dhcp_get_type(osal_u8 *pos, const osal_u8 *packet_end, osal_u8 type)
{
    osal_u8 *opt = OSAL_NULL;
    if (pos == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{mac_dhcp_get_type::pos is null.}");
        return OSAL_NULL;
    }

    if (packet_end == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{mac_dhcp_get_type::packet_end is null.}");
        return OSAL_NULL;
    }

    /* 获取 DHCP 类型 */
    while ((pos < packet_end) && (*pos != 0xFF)) {
        opt = pos++;

        /* Padding */
        if (*opt == 0) {
            continue;
        }
        pos += *pos + 1;
        if (pos >= packet_end) {
            break;
        }
        /* Message Type */
        if ((type == *opt) && (opt[1] != 0)) {
            return opt;
        }
    }

    return OSAL_NULL;
}

osal_u32 hmac_dhcp_offload_process_tx_frame(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    mac_llc_snap_stru *snap = OSAL_NULL;
    mac_ip_header_stru *ip_hdr = OSAL_NULL;
    mac_udp_header_stru *udp_hdr = OSAL_NULL;
    dhcp_message_stru *dhcp_message = OSAL_NULL;
    osal_u8 *tmp = OSAL_NULL;
    osal_u32 ip_hdr_len;

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_FAIL;
    }

    hmac_wow_save_cipher_info(hmac_vap, netbuf);

    if (hmac_get_tx_data_type_etc(netbuf) != MAC_DATA_DHCP) {
        return OAL_FAIL;
    }

    snap = (mac_llc_snap_stru *)oal_netbuf_tx_data_const(netbuf);
    if (snap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_dhcp_offload_process_tx_frame::snap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ip_hdr = (mac_ip_header_stru *)(snap + 1);
    ip_hdr_len = mac_ip_hdr_len(ip_hdr);
    udp_hdr = (mac_udp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
    dhcp_message = (dhcp_message_stru *)(udp_hdr + 1);
    tmp = mac_dhcp_get_type(dhcp_message->options,
        (osal_u8 *)(uintptr_t)(dhcp_message->options + sizeof(dhcp_message->options)), DHCP_OPT_MESSAGETYPE);
    if (tmp == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_dhcp_offload_process_tx_frame:tmp is null when checking DHCP REQUEST}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (*(tmp + 2) == MAC_DHCP_REQUEST) { /* 2 偏移量 */
        if (mac_dhcp_get_type(dhcp_message->options, dhcp_message->options + sizeof(dhcp_message->options),
            DHCP_OPT_IPADDRESS) == OSAL_NULL) {
            return OAL_FAIL;
        }
        
        if (memcpy_s(&g_dhcp_request, sizeof(dhcp_record_frame), snap, sizeof(dhcp_record_frame)) != EOK) {
            oam_warning_log0(0, OAM_SF_CFG, "hmac_dhcp_offload_process_tx_frame:: snap memcpy_s fail.");
            return OAL_FAIL;
        }

        if (memcpy_s(&g_dhcp_request_header, sizeof(mac_ieee80211_frame_stru), oal_netbuf_header(netbuf),
            sizeof(mac_ieee80211_frame_stru)) != EOK) {
            oam_warning_log0(0, OAM_SF_CFG, "hmac_dhcp_offload_process_tx_frame:: frame_hdr memcpy_s fail.");
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

osal_u32 hmac_dhcp_offload_process_rx_frame(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    osal_u32 lease_time;
    mac_ip_header_stru *ip_hdr = OSAL_NULL;
    dhcp_message_stru *dhcp_message = OSAL_NULL;
    osal_u8 *tmp = OSAL_NULL;
    mac_llc_snap_stru *snap = OSAL_NULL;
    osal_u32 ip_hdr_len;

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_FAIL;
    }

    if (hmac_get_rx_data_type_etc(netbuf) != MAC_DATA_DHCP) {
        return OAL_FAIL;
    }

    snap = (mac_llc_snap_stru *)oal_netbuf_tx_data_const(netbuf);
    if (snap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_dhcp_offload_process_rx_frame::snap is null.}");
        return OAL_FAIL;
    }

    /* 获取 DHCP 报文头指针 */
    ip_hdr = (mac_ip_header_stru *)(snap + 1);
    ip_hdr_len = mac_ip_hdr_len(ip_hdr);
    dhcp_message = (dhcp_message_stru *)((mac_udp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len) + 1);

    if (memcmp(mac_mib_get_station_id(hmac_vap), dhcp_message->chaddr, WLAN_MAC_ADDR_LEN)) {
        oam_warning_log0(0, OAM_SF_PWR, "hmac_dhcp_offload_process_rx_frame:MAC ADDR check fail");
        return OAL_FAIL;
    }

    tmp = mac_dhcp_get_type(dhcp_message->options,
        (osal_u8*)(dhcp_message->options + sizeof(dhcp_message->options)), DHCP_OPT_MESSAGETYPE);
    if (tmp == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_dhcp_offload_process_rx_frame:tmp is null when checking DHCP ACK}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (*(tmp + 2) == MAC_DHCP_ACK) { /* 2 偏移量 */
        tmp = mac_dhcp_get_type(dhcp_message->options,
            (osal_u8*)(dhcp_message->options + sizeof(dhcp_message->options)), DHCP_OPT_LEASETIME);
            if (tmp == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_PWR, "{hmac_dhcp_offload_process_rx_frame:tmp is null check lease time}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (memcpy_s(&lease_time, sizeof(osal_u32), tmp + 2, 4) != EOK) { /* 2 偏移量 4 复制长度 */
            oam_warning_log0(0, OAM_SF_CFG, "hmac_dhcp_offload_process_rx_frame:: tmp memcpy_s fail.");
            return OAL_FAIL;
        }

        g_lease_time = oal_host2net_long(lease_time);  // oal_net2host_long
        g_request_ip = dhcp_message->yiaddr;
        g_server_ip = ip_hdr->saddr;
        hmac_dhcp_offload_set_timeout((osal_u32)osal_get_time_stamp_ms());
        oam_warning_log2(0, OAM_SF_PWR, "{lease_time=%u,timeout=%llu}", g_lease_time, g_dhcp_offload_timeout);
    }

    return OAL_SUCC;
}

osal_u32 hmac_dhcp_offload_tx_hookfn(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL || netbuf == OSAL_NULL || *netbuf == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    hmac_dhcp_offload_process_tx_frame(hmac_vap, *netbuf);

    return OAL_CONTINUE;
}

osal_u32 hmac_dhcp_offload_rx_hookfn(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL || netbuf == OSAL_NULL || *netbuf == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    hmac_dhcp_offload_process_rx_frame(hmac_vap, *netbuf);

    return OAL_CONTINUE;
}

#else
osal_s32 hmac_config_set_dhcp_offload_enable(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *vap_tmp = OSAL_NULL;
    osal_u8 vap_idx, index;

    for (index = 0; index < hmac_device->vap_num; index++) {
        vap_idx = hmac_device->vap_id[index];
        vap_tmp = mac_res_get_hmac_vap(vap_idx);
        if (vap_tmp == OSAL_NULL) {
            continue;
        }
        if ((vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_AP) && (vap_tmp->vap_state == MAC_VAP_STATE_UP)) {
            return OAL_FAIL;
        }
    }

    (osal_void)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SET_DHCP_OFFLOAD_SWITCH, msg, OSAL_TRUE);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
hmac_netbuf_hook_stru g_offload_rx_netbuf_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_dhcp_offload_rx_hookfn,
};

hmac_netbuf_hook_stru g_offload_tx_netbuf_hook = {
    .hooknum = HMAC_FRAME_DATA_TX_EVENT_H2D,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_dhcp_offload_tx_hookfn,
};
#endif

osal_u32 hmac_dhcp_offload_init(osal_void)
{
    osal_u32 ret = OAL_SUCC;

#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    ret = hmac_register_netbuf_hook(&g_offload_rx_netbuf_hook);
    ret |= hmac_register_netbuf_hook(&g_offload_tx_netbuf_hook);
#else
    frw_msg_hook_register(WLAN_MSG_W2H_C_CFG_SET_DHCPOFFLOAD_SWITCH, hmac_config_set_dhcp_offload_enable);
#endif

    return ret;
}

osal_void hmac_dhcp_offload_deinit(osal_void)
{
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    hmac_unregister_netbuf_hook(&g_offload_rx_netbuf_hook);
    hmac_unregister_netbuf_hook(&g_offload_tx_netbuf_hook);
#else
    frw_msg_hook_unregister(WLAN_MSG_W2H_C_CFG_SET_DHCPOFFLOAD_SWITCH);
#endif
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

