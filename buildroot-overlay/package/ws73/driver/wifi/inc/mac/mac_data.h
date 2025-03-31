/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of mac data public interface.
 * Create: 2020-01-01
 */

#ifndef __MAC_DATA_H__
#define __MAC_DATA_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/

#include "mac_regdomain.h"
#include "oal_ext_if.h"
#include "wlan_mib_hcm.h"
#include "mac_user_ext.h"
#include "oam_ext_if.h"
#include "hal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    宏定义
*****************************************************************************/
#define OAL_EAPOL_INFO_POS      13
#define OAL_EAPOL_TYPE_POS      9
#define OAL_EAPOL_TYPE_KEY      3

/*****************************************************************************
    枚举定义
*****************************************************************************/

typedef enum {
    MAC_NETBUFF_PAYLOAD_ETH  = 0,
    MAC_NETBUFF_PAYLOAD_SNAP,

    MAC_NETBUFF_PAYLOAD_BUTT
}mac_netbuff_payload_type;
typedef osal_u8 mac_netbuff_payload_type_uint8;

typedef enum {
    PKT_TRACE_DATA_DHCP                 = 0,
    PKT_TRACE_DATA_ARP_REQ,
    PKT_TRACE_DATA_ARP_RSP,
    PKT_TRACE_DATA_EAPOL,
    PKT_TRACE_DATA_ICMP,
    PKT_TRACE_MGMT_ASSOC_REQ,
    PKT_TRACE_MGMT_ASSOC_RSP,
    PKT_TRACE_MGMT_REASSOC_REQ,
    PKT_TRACE_MGMT_REASSOC_RSP,
    PKT_TRACE_MGMT_DISASOC,
    PKT_TRACE_MGMT_AUTH,
    PKT_TRACE_MGMT_DEAUTH,
    PKT_TRACE_BUTT
}pkt_trace_type_enum;
typedef osal_u8 pkt_trace_type_enum_uint8;

/*****************************************************************************
    全局变量声明
*****************************************************************************/
typedef osal_void (*data_type_from_8023)(const osal_u8 *frame_hdr, mac_netbuff_payload_type hdr_type,
    osal_u8 *datatype);

typedef struct {
    data_type_from_8023        data_type_from_8023_cb;
}mac_data_cb;

/*****************************************************************************
    函数声明
*****************************************************************************/
pkt_trace_type_enum_uint8 hmac_wifi_pkt_should_trace(const oal_netbuf_stru *netbuff, osal_u16 mac_hdr_len);
osal_u16 mac_get_eapol_keyinfo_etc(const oal_netbuf_stru *netbuff);
osal_u8 mac_get_eapol_type_etc(const oal_netbuf_stru *netbuff);

extern oal_bool_enum_uint8 hmac_is_dhcp_port_etc(mac_ip_header_stru *ip_hdr);
extern oal_bool_enum_uint8 hmac_is_dhcp6_etc(oal_ipv6hdr_stru  *ipv6hdr);
extern osal_u8 hmac_get_data_type_from_8023_etc(const osal_u8 *frame_hdr, mac_netbuff_payload_type hdr_type);
oal_bool_enum_uint8 hmac_is_eapol_key_ptk_etc(const mac_eapol_header_stru  *eapol_header);
extern  osal_u8 hmac_get_data_type_from_80211_etc(oal_netbuf_stru *netbuff, osal_u16 mac_hdr_len);

osal_u8 hmac_get_tx_data_type_etc(const oal_netbuf_stru *netbuff);
osal_u8 hmac_get_rx_data_type_etc(const oal_netbuf_stru *netbuff);
#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

#endif /* end of mac_data.h */

