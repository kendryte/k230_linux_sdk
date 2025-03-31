/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: oal_net.c
 * Create: 2012年11月30日
 */
#include "oal_net.h"
#include "mac_frame.h"
#include "mac_data.h"
#include "oal_cfg80211.h"

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <net/genetlink.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_NET_C

#define oal_high_half_byte(a) (((a) & 0xF0) >> 4)
#define oal_low_half_byte(a)  ((a) & 0x0F)

#define wlan_tos_to_hcc_queue(_tos) ( \
        (((_tos) == 0) || ((_tos) == 3)) ? WLAN_UDP_BE_QUEUE : \
        (((_tos) == 1) || ((_tos) == 2)) ? WLAN_UDP_BK_QUEUE : \
        (((_tos) == 4) || ((_tos) == 5)) ? WLAN_UDP_VI_QUEUE : \
        WLAN_UDP_VO_QUEUE)

#define WLAN_DATA_VIP_QUEUE (WLAN_HI_QUEUE)

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : 判断是否是dhcp帧 port
*****************************************************************************/
INLINE__ oal_bool_enum_uint8 oal_netbuf_is_dhcp_port_etc(const oal_udp_header_stru *udp_hdr)
{
    if (((oal_host2net_short(udp_hdr->source) == 68) /* 68 是DHCP端口号 */
        && (oal_host2net_short(udp_hdr->dest) == 67)) /* 67 是DHCP端口号 */
        ||((oal_host2net_short(udp_hdr->source) == 67) /* 67 是DHCP端口号 */
        && (oal_host2net_short(udp_hdr->dest) == 68))) { /* 68 是DHCP端口号 */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 判断是否是DHCP6帧
*****************************************************************************/
oal_bool_enum_uint8 oal_netbuf_is_dhcp6_etc(oal_ipv6hdr_stru  *ipv6hdr)
{
    oal_udp_header_stru *udp_header = OSAL_NULL;

    if (MAC_UDP_PROTOCAL == ipv6hdr->nexthdr) {
        udp_header = (oal_udp_header_stru *)(ipv6hdr + 1);

        if (((oal_host2net_short(udp_header->source) == MAC_IPV6_UDP_SRC_PORT) &&
            (oal_host2net_short(udp_header->dest) == MAC_IPV6_UDP_DES_PORT)) ||
            ((oal_host2net_short(udp_header->source) == MAC_IPV6_UDP_DES_PORT) &&
            (oal_host2net_short(udp_header->dest) == MAC_IPV6_UDP_SRC_PORT))) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 判断ipv4 tcp报文是否为tcp 关键帧，包括TCP ACK\TCP SYN帧等
*****************************************************************************/
oal_bool_enum_uint8 oal_netbuf_is_tcp_ack_etc(oal_ip_header_stru *ip_hdr)
{
    oal_tcp_header_stru    *tcp_hdr;
    osal_u32              ip_pkt_len;
    osal_u32              ip_hdr_len;
    osal_u32              tcp_hdr_len;

    ip_pkt_len   = oal_net2host_short(ip_hdr->tot_len);
    ip_hdr_len   = (oal_low_half_byte(ip_hdr->ihl)) << 2; /* 左移2位 */
    tcp_hdr      = (oal_tcp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
    tcp_hdr_len  = (oal_high_half_byte(tcp_hdr->offset)) << 2; /* 左移2位 */

    if (tcp_hdr_len + ip_hdr_len == ip_pkt_len) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 判断ipv4 是否为icmp报文
*****************************************************************************/
oal_bool_enum_uint8 oal_netbuf_is_icmp_etc(oal_ip_header_stru *ip_hdr)
{
    osal_u8  protocol;
    protocol = ip_hdr->protocol;

    /* ICMP报文检查 */
    if (MAC_ICMP_PROTOCAL == protocol) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 判断ipv6 tcp报文是否为tcp ack
*****************************************************************************/
oal_bool_enum_uint8 oal_netbuf_is_tcp_ack6_etc(oal_ipv6hdr_stru *ipv6hdr)
{
    oal_tcp_header_stru *tcp_hdr;
    osal_u32 ip_pkt_len;
    osal_u32 tcp_header_len;

    tcp_hdr = (oal_tcp_header_stru *)(ipv6hdr + 1);
    ip_pkt_len = oal_net2host_short(ipv6hdr->payload_len); /* ipv6 净载荷, ipv6报文头部固定为40字节 */
    tcp_header_len = (oal_high_half_byte(tcp_hdr->offset)) << 2; /* 左移2位 */

    if (tcp_header_len == ip_pkt_len) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 功能描述  : 为以太网来包选择合的subqueue入队
*****************************************************************************/
osal_u16 oal_netbuf_select_queue_etc(oal_netbuf_stru *buf)
{
    oal_ether_header_stru  *ether_header;
    oal_ip_header_stru     *pst_ip;
    oal_ipv6hdr_stru       *ipv6;
    oal_udp_header_stru    *udp_hdr;
    osal_u32              ipv6_hdr;
    osal_u8               queue = WLAN_NORMAL_QUEUE;
    osal_u32              ip_hdr_len;

    /* 获取以太网头 */
    ether_header = (oal_ether_header_stru *)oal_netbuf_data(buf);

    switch (ether_header->ether_type) {
        case oal_host2net_short(ETHER_TYPE_IP):

            pst_ip = (oal_ip_header_stru *)(ether_header + 1); /* 偏移一个以太网头，取ip头 */

            /* 对udp报文区分qos入队 */
            if (MAC_UDP_PROTOCAL == pst_ip->protocol) {
                queue = WLAN_UDP_DATA_QUEUE;
                /* 如果是DHCP帧，则进入DATA_HIGH_QUEUE */
                ip_hdr_len = pst_ip->ihl << 2;       /* 左移2bit:值对应4字节 */
                udp_hdr = (oal_udp_header_stru *)((osal_u8 *)pst_ip + ip_hdr_len);
                /* 增加dhcp不是分片报文的判断 */
                if ((0 == (pst_ip->frag_off & 0xFF1F)) && (OAL_TRUE == oal_netbuf_is_dhcp_port_etc(udp_hdr))) {
                    queue = WLAN_DATA_VIP_QUEUE;
                }
            } else if (MAC_TCP_PROTOCAL == pst_ip->protocol) { /* 区分TCP ack与TCP data报文 */
                if (OAL_TRUE == oal_netbuf_is_tcp_ack_etc(pst_ip)) {
                    queue = WLAN_TCP_ACK_QUEUE;
                } else {
                    queue = WLAN_TCP_DATA_QUEUE;
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
            ipv6    = (oal_ipv6hdr_stru *)(ether_header + 1); /* 偏移一个以太网头，取ip头 */
            ipv6_hdr = (*(osal_u32 *)ipv6);

            if (MAC_UDP_PROTOCAL == ipv6->nexthdr) { /* UDP报文 */
                queue = WLAN_UDP_DATA_QUEUE;
            } else if (MAC_TCP_PROTOCAL == ipv6->nexthdr) { /* TCP报文 */
                if (OAL_TRUE == oal_netbuf_is_tcp_ack6_etc(ipv6)) {
                    queue = WLAN_TCP_ACK_QUEUE;
                } else {
                    queue = WLAN_TCP_DATA_QUEUE;
                }
                /* 如果是DHCPV6帧，则进入WLAN_DATA_VIP_QUEUE队列缓存 */
            } else if (OAL_TRUE == oal_netbuf_is_dhcp6_etc((oal_ipv6hdr_stru *)(ether_header + 1))) {
                queue = WLAN_DATA_VIP_QUEUE;
            }
            break;

        /* TDLS帧处理，建链保护，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_TDLS):
            queue = WLAN_DATA_VIP_QUEUE;
            break;

        case oal_host2net_short(ETHER_TYPE_PAE):
            /* 如果是EAPOL帧，则进入VO队列发送 */
            queue = WLAN_DATA_VIP_QUEUE;
            break;

        /* PPPOE帧处理，建链保护(发现阶段, 会话阶段)，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_PPP_DISC):
        case oal_host2net_short(ETHER_TYPE_PPP_SES):
            queue = WLAN_DATA_VIP_QUEUE;
            break;

        case oal_host2net_short(ETHER_TYPE_WAI):
            queue = WLAN_DATA_VIP_QUEUE;
            break;

        case oal_host2net_short(ETHER_TYPE_VLAN):
            queue = WLAN_UDP_DATA_QUEUE;
            break;

        default:
            queue = WLAN_NORMAL_QUEUE;
            break;
    }
    return queue;
}

oal_module_symbol(oal_netbuf_is_tcp_ack6_etc);
oal_module_symbol(oal_netbuf_select_queue_etc);
oal_module_symbol(oal_netbuf_is_dhcp_port_etc);
oal_module_symbol(oal_netbuf_is_dhcp6_etc);
oal_module_symbol(oal_netbuf_is_tcp_ack_etc);
oal_module_symbol(oal_netbuf_is_icmp_etc);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
