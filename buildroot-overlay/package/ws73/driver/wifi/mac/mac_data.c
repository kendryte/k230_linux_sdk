/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Used to identify data frames.
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "mac_data.h"
#include "oal_netbuf_data.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_DATA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    全局变量定义
*****************************************************************************/

/*****************************************************************************
    函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述 : 从带mac头的80211帧中获取以太类型
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u8 hmac_get_data_type_from_80211_etc(oal_netbuf_stru *netbuff, osal_u16 mac_hdr_len)
{
    osal_u8 datatype;
    mac_llc_snap_stru *snap;

    if (netbuff == OAL_PTR_NULL) {
        return MAC_DATA_BUTT;
    }

    snap = (mac_llc_snap_stru *)(OAL_NETBUF_DATA(netbuff) + mac_hdr_len);

    datatype = hmac_get_data_type_from_8023_etc((osal_u8 *)snap, MAC_NETBUFF_PAYLOAD_SNAP);

    return datatype;
}

/*****************************************************************************
 功能描述 : 判断该帧是否为4 次握手的EAPOL KEY 单播密钥协商帧
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_eapol_key_ptk_etc(const mac_eapol_header_stru *eapol_header)
{
    mac_eapol_key_stru *key;

    if (eapol_header->type == IEEE802_1X_TYPE_EAPOL_KEY) {
        if ((osal_u16)(oal_net2host_short(eapol_header->length)) >= (osal_u16)OAL_SIZEOF(mac_eapol_key_stru)) {
            key = (mac_eapol_key_stru *)(eapol_header + 1);

            if ((key->key_info[1] & WPA_KEY_INFO_KEY_TYPE) != 0) {
                return OAL_TRUE;
            }
        }
    }
    return OAL_FALSE;
}

/*****************************************************************************
 功能描述 : 判断是否是dhcp帧 port
*****************************************************************************/
WIFI_HMAC_TCM_TEXT oal_bool_enum_uint8 hmac_is_dhcp_port_etc(mac_ip_header_stru *ip_hdr)
{
    mac_udp_header_stru *udp_hdr;
    /* DHCP判断标准: udp协议，ip头部fragment offset字段为0，目的端口号为67或68 */
    if (ip_hdr->protocol == MAC_UDP_PROTOCAL && ((ip_hdr->frag_off & 0xFF1F) == 0)) {
        osal_u32 ip_hdr_len = mac_ip_hdr_len(ip_hdr);
        udp_hdr = (mac_udp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
        if (oal_net2host_short(udp_hdr->des_port) == 67 || /* 67 是DHCP端口号 */
            oal_net2host_short(udp_hdr->des_port) == 68) { /* 68 是DHCP端口号 */
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述 : 判断是否是nd帧
*****************************************************************************/
static oal_bool_enum_uint8 hmac_is_nd_etc(oal_ipv6hdr_stru *ipv6hdr)
{
    oal_icmp6hdr_stru *icmp6hdr;

    if (ipv6hdr->nexthdr == OAL_IPPROTO_ICMPV6) {
        icmp6hdr = (oal_icmp6hdr_stru *)(ipv6hdr + 1);

        if ((icmp6hdr->icmp6_type == MAC_ND_RSOL) || (icmp6hdr->icmp6_type == MAC_ND_RADVT) ||
            (icmp6hdr->icmp6_type == MAC_ND_NSOL) || (icmp6hdr->icmp6_type == MAC_ND_NADVT) ||
            (icmp6hdr->icmp6_type == MAC_ND_RMES)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述 : 判断是否是DHCP6帧
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_dhcp6_etc(oal_ipv6hdr_stru *ipv6hdr)
{
    mac_udp_header_stru *udp_hdr;

    if (ipv6hdr->nexthdr == MAC_UDP_PROTOCAL) {
        udp_hdr = (mac_udp_header_stru *)(ipv6hdr + 1);

        if (udp_hdr->des_port == oal_host2net_short(MAC_IPV6_UDP_DES_PORT) ||
            udp_hdr->des_port == oal_host2net_short(MAC_IPV6_UDP_SRC_PORT)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述 : 获取arp帧的类型：request/responce(暂时不考虑rarp!)
*****************************************************************************/
static mac_data_type_enum_uint8 hmac_get_arp_type_by_arphdr(const oal_eth_arphdr_stru *rx_arp_hdr)
{
    if (oal_net2host_short(rx_arp_hdr->ar_op) == MAC_ARP_REQUEST) {
        return MAC_DATA_ARP_REQ;
    } else if (oal_net2host_short(rx_arp_hdr->ar_op) == MAC_ARP_RESPONSE) {
        return MAC_DATA_ARP_RSP;
    }

    return MAC_DATA_BUTT;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u8 mac_get_data_type_by_ethertype(const osal_u8 *frame_body,
    const osal_u16 ether_type)
{
    mac_ip_header_stru *ip = OAL_PTR_NULL;
    osal_u8 datatype = MAC_DATA_BUTT;

    switch (ether_type) {
        case oal_host2net_short(ETHER_TYPE_IP):
            /* 从IP TOS字段寻找优先级 */
            /*----------------------------------------------------------------------
                tos位定义
             ----------------------------------------------------------------------
            | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
            | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
             ----------------------------------------------------------------------*/
            ip = (mac_ip_header_stru *)frame_body; /* 偏移一个以太网头，取ip头 */

            if (hmac_is_dhcp_port_etc(ip) == OAL_TRUE) {
                datatype = MAC_DATA_DHCP;
            } else if (ip->protocol == MAC_ICMP_PROTOCAL) {
                if (*(osal_u8*)(ip + 1) == MAC_ICMP_RESPONSE) {
                    datatype = MAC_DATA_ICMP_RSP;
                } else if (*(osal_u8*)(ip + 1) == MAC_ICMP_REQUEST) {
                    datatype = MAC_DATA_ICMP_REQ;
                } else {
                    datatype = MAC_DATA_ICMP_OTH;
                }
            }
            break;
        case oal_host2net_short(ETHER_TYPE_IPV6):
            /* 从IPv6 traffic class字段获取优先级 */
            /*----------------------------------------------------------------------
                IPv6包头 前32为定义
             -----------------------------------------------------------------------
            | 版本号 | traffic class   | 流量标识 |
            | 4bit   | 8bit(同ipv4 tos)|  20bit   |
            -----------------------------------------------------------------------*/
            /* 如果是ND帧，则进入VO队列发送 */
            if (hmac_is_nd_etc((oal_ipv6hdr_stru *)frame_body) == OAL_TRUE) {
                datatype = MAC_DATA_ND;
            } else if (hmac_is_dhcp6_etc((oal_ipv6hdr_stru *)frame_body) == OAL_TRUE) { /* 如果是DHCPV6帧 */
                datatype = MAC_DATA_DHCPV6;
            }

            break;

        case oal_host2net_short(ETHER_TYPE_PAE):
            /* 如果是EAPOL帧，则进入VO队列发送 */
            datatype = MAC_DATA_EAPOL; /* eapol */
            break;

        /* TDLS帧处理，建链保护，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_TDLS):
            datatype = MAC_DATA_TDLS;
            break;

        /* PPPOE帧处理，建链保护(发现阶段, 会话阶段)，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_PPP_DISC):
        case oal_host2net_short(ETHER_TYPE_PPP_SES):
            datatype = MAC_DATA_PPPOE;
            break;

        case oal_host2net_short(ETHER_TYPE_WAI):
            datatype = MAC_DATA_WAPI;
            break;

        case oal_host2net_short(ETHER_TYPE_VLAN):
            datatype = MAC_DATA_VLAN;

            break;

        case oal_host2net_short(ETHER_TYPE_ARP):
            /* 如果是ARP帧，则进入VO队列发送 */
            datatype = (osal_u8)hmac_get_arp_type_by_arphdr((oal_eth_arphdr_stru *)frame_body);
            break;

        default:
            datatype = MAC_DATA_BUTT;
            break;
    }

    return datatype;
}

/*****************************************************************************
 功能描述 : 根据数据帧(802.3)的类型，判断帧类型 frame_hdr: 为去除80211头的数
            据帧,可为以太头或snap头 hdr_type: 指针指向数据类型，用来计算获取
            data_type时刻的偏移
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u8 hmac_get_data_type_from_8023_etc(const osal_u8 *frame_hdr, mac_netbuff_payload_type hdr_type)
{
    const osal_u8 *frame_body = OAL_PTR_NULL;
    osal_u16 ether_type;
    osal_u8 datatype = MAC_DATA_BUTT;

    if (frame_hdr == OAL_PTR_NULL) {
        return datatype;
    }

    if (hdr_type == MAC_NETBUFF_PAYLOAD_ETH) {
        ether_type = ((mac_ether_header_stru *)frame_hdr)->ether_type;
        frame_body = frame_hdr + (osal_u16)OAL_SIZEOF(mac_ether_header_stru);
    } else if (hdr_type == MAC_NETBUFF_PAYLOAD_SNAP) {
        ether_type = ((mac_llc_snap_stru *)frame_hdr)->ether_type;
        frame_body = frame_hdr + (osal_u16)OAL_SIZEOF(mac_llc_snap_stru);
    } else {
        return datatype;
    }

    datatype = mac_get_data_type_by_ethertype(frame_body, ether_type);

    return datatype;
}

/*****************************************************************************
 函 数 名  : hmac_get_tx_data_type_etc
 功能描述  : 根据数据帧的类型，判断帧类型
*****************************************************************************/
osal_u8 hmac_get_tx_data_type_etc(const oal_netbuf_stru *netbuff)
{
    osal_u8 datatype;
    const mac_llc_snap_stru *snap = OSAL_NULL;

    if (netbuff == OSAL_NULL) {
        return MAC_DATA_BUTT;
    }

    snap = (mac_llc_snap_stru *)oal_netbuf_tx_data_const(netbuff);
    if (snap == OSAL_NULL) {
        return MAC_DATA_BUTT;
    }

    datatype = hmac_get_data_type_from_8023_etc((const osal_u8 *)snap, MAC_NETBUFF_PAYLOAD_SNAP);

    return datatype;
}

/*****************************************************************************
 函 数 名  : hmac_get_rx_data_type_etc
 功能描述  : 根据数据帧的类型，判断帧类型
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u8 hmac_get_rx_data_type_etc(const oal_netbuf_stru *netbuff)
{
    osal_u8 datatype;
    const mac_llc_snap_stru *snap = OSAL_NULL;

    if (netbuff == OSAL_NULL) {
        return MAC_DATA_BUTT;
    }

    snap = (mac_llc_snap_stru *)oal_netbuf_rx_data_const(netbuff);
    if (snap == OSAL_NULL) {
        return MAC_DATA_BUTT;
    }

    datatype = hmac_get_data_type_from_8023_etc((const osal_u8 *)snap, MAC_NETBUFF_PAYLOAD_SNAP);

    return datatype;
}

/*****************************************************************************
 函 数 名  : mac_get_eapol_keyinfo_etc
 功能描述  : 0x专用，获取eapol keyinfo
*****************************************************************************/
osal_u16 mac_get_eapol_keyinfo_etc(const oal_netbuf_stru *netbuff)
{
    osal_u8 data_type;
    const osal_u8 *payload = OSAL_NULL;

    data_type = hmac_get_rx_data_type_etc(netbuff);
    if (data_type != MAC_DATA_EAPOL) {
        return 0;
    }

    payload = oal_netbuf_rx_data_const(netbuff);
    if (payload == OSAL_NULL) {
        return 0;
    }

    return *(osal_u16 *)(payload + OAL_EAPOL_INFO_POS);
}

/*****************************************************************************
 函 数 名  : mac_get_eapol_keyinfo_etc
 功能描述  : 0x专用，用来区分不同的eapol帧，一般为3
*****************************************************************************/
osal_u8 mac_get_eapol_type_etc(const oal_netbuf_stru *netbuff)
{
    osal_u8 datatype;
    const osal_u8 *payload = OSAL_NULL;

    datatype = hmac_get_rx_data_type_etc(netbuff);
    if (datatype != MAC_DATA_EAPOL) {
        return 0;
    }

    payload = oal_netbuf_rx_data_const(netbuff);
    if (payload == OSAL_NULL) {
        return 0;
    }

    return *(payload + OAL_EAPOL_TYPE_POS);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
