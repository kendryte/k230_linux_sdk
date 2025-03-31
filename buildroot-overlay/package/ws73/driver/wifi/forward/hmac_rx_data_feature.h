/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: Header file for hmac_rx_data_feature.c.
 * Create: 2022-05-20
 */

#ifndef __HMAC_RX_DATA_FEATURE_H__
#define __HMAC_RX_DATA_FEATURE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_resource.h"
#include "oal_netbuf_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_DATA_H

#define HMAC_RX_DATA_ETHER_OFFSET_LENGTH      6   /* 代表发送lan的包，需要填写以太网头，需要向前偏移6 */

osal_u32 hmac_rx_parse_amsdu_etc(oal_netbuf_stru       *pst_netbuf,
    hmac_msdu_stru                     *msdu,
    hmac_msdu_proc_state_stru          *msdu_state,
    mac_msdu_proc_status_enum_uint8    *pen_proc_state);

osal_u32 hmac_rx_lan_frame_classify_amsdu(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    mac_ieee80211_frame_stru *frame_hdr, hmac_user_stru *hmac_user, oal_netbuf_head_stru *w2w_netbuf_hdr);

osal_void hmac_pkt_mem_opt_init_etc(hmac_device_stru *hmac_device);
osal_void hmac_pkt_mem_opt_exit_etc(hmac_device_stru *hmac_device);
osal_void hmac_pkt_mem_opt_rx_pkts_stat(hmac_vap_stru *hmac_vap, oal_ip_header_stru *pst_ip);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
extern osal_u8  g_uc_print_data_wakeup_etc;
osal_void hmac_parse_packet_etc(oal_netbuf_stru *netbuf_eth);

#define WIFI_WAKESRC_TAG "plat:wifi_wake_src,"
struct ieee8021x_hdr {
    osal_u8 version;
    osal_u8 type;
    osal_u16 length;
};
#endif

#ifdef _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING
oal_bool_enum_uint8 hmac_rx_check_arp_spoofing(oal_net_device_stru *device, oal_netbuf_stru *pst_netbuf);
#endif

osal_void hmac_transfer_rx_handle(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    oal_netbuf_head_stru *netbuf_header);

typedef enum {
    HMAC_PKT_DIRECTION_TX = 0,
    HMAC_PKT_DIRECTION_RX = 1,
} hmac_pkt_direction_enum;
osal_void hmac_parse_special_ipv4_packet(const hmac_vap_stru *hmac_vap, osal_u8 *pktdata, osal_u32 datalen,
    hmac_pkt_direction_enum pkt_direction);

/*****************************************************************************
 函 数 名  : hmac_rx_clear_amsdu_last_netbuf_pointer
 功能描述  : 设置amsdu 最后一个 netbuf next指针为null
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_rx_clear_amsdu_last_netbuf_pointer(oal_netbuf_stru *pst_netbuf, osal_u16 num_buf)
{
    oal_netbuf_stru *pst_netbuf_tmp = pst_netbuf;
    osal_u16 num_buf_tmp = num_buf;
    while (pst_netbuf_tmp != OAL_PTR_NULL) {
        num_buf_tmp--;
        if (num_buf_tmp == 0) {
            pst_netbuf_tmp->next = OAL_PTR_NULL;
            break;
        }
        pst_netbuf_tmp = oal_get_netbuf_next(pst_netbuf_tmp);
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_rx_data_feature.h */
