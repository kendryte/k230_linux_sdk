/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: Source file defined by the common operation function of
   the received frame and the operation function of the data frame of the DMAC module.
 * Create: 2022-05-20
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_rx_data_feature.h"
#include "hmac_rx_data.h"
#include "hmac_tcp_opt.h"
#include "oam_struct.h"
#include "oam_ext_if.h"
#include "hmac_feature_dft.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
#include <linux/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/ieee80211.h>
#include <linux/ipv6.h>
#endif

#ifdef _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING
#include <linux/inetdevice.h>
#endif
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "netinet/ip.h"
#include "netinet/udp.h"
#include "netinet/ip_icmp.h"
#include "netinet/if_ether.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HMAC_RX_DATA_FEATURE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#define DNS_GET_QR_FROM_FLAG(flag)        ((osal_u8)((((flag) & 0x8000U) > 0) ? 1 : 0))

/*****************************************************************************
 函 数 名  : hmac_rx_free_amsdu_netbuf
 功能描述  : 释放amsdu netbuf
*****************************************************************************/
OAL_STATIC osal_void hmac_rx_free_amsdu_netbuf(oal_netbuf_stru *pst_netbuf)
{
    oal_netbuf_stru        *netbuf_cur = pst_netbuf;
    oal_netbuf_stru        *netbuf_next;
    while (netbuf_cur != OAL_PTR_NULL) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf_cur);
        netbuf_next = oal_get_netbuf_next(netbuf_cur);
        oal_netbuf_free(netbuf_cur);
        netbuf_cur = netbuf_next;
    }
}

/*****************************************************************************
 函 数 名  : hmac_parse_amsdu
 功能描述  : 解析出每一个AMSDU中的MSDU
 输入参数  : 指向MPDU的第一个netbuf的指针
 输出参数  : (1)指向当前要转发的MSDU的指针
             (2)用于记录处理当前的MPDU中MSDU的信息
             (3)当前MPDU的处理状态:标识该MPDU是否处理完成
 返 回 值  : 成功或者失败原因
*****************************************************************************/
osal_u32 hmac_rx_parse_amsdu_etc(oal_netbuf_stru                    *pst_netbuf,
    hmac_msdu_stru                     *msdu,
    hmac_msdu_proc_state_stru          *msdu_state,
    mac_msdu_proc_status_enum_uint8    *pen_proc_state)
{
    mac_rx_ctl_stru        *pst_rx_ctrl;                            /* MPDU的控制信息 */
    osal_u8              *buffer_data_addr    = OAL_PTR_NULL; /* 指向netbuf数据域的指针 */
    osal_u16              offset;                              /* submsdu相对于data指针的偏移 */
    osal_u16              submsdu_len          = 0;            /* submsdu的长度 */
    osal_u8               submsdu_pad_len      = 0;            /* submsdu的填充长度 */
    osal_u8              *submsdu_hdr         = OAL_PTR_NULL; /* 指向submsdu头部的指针 */
    oal_netbuf_stru        *netbuf_prev;
    oal_bool_enum_uint8     b_need_free_netbuf;
    osal_u32              need_pull_len;

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_parse_amsdu_etc::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 首次进入该函数解析AMSDU */
    if ((msdu_state->procd_netbuf_nums == 0) &&
        (msdu_state->procd_msdu_in_netbuf == 0)) {
        msdu_state->curr_netbuf      = pst_netbuf;

        /* AMSDU时，首个netbuf的中包含802.11头，对应的payload需要偏移 */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(msdu_state->curr_netbuf);

        msdu_state->curr_netbuf_data   =
            (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl) + pst_rx_ctrl->mac_header_len;
        msdu_state->netbuf_nums_in_mpdu = pst_rx_ctrl->buff_nums;
        msdu_state->msdu_nums_in_netbuf = pst_rx_ctrl->msdu_in_buffer;
        msdu_state->submsdu_offset      = 0;

        /* 使netbuf 指向amsdu 帧头 */
        need_pull_len = (osal_u32)(msdu_state->curr_netbuf_data - oal_netbuf_payload(pst_netbuf));
        oal_netbuf_pull(msdu_state->curr_netbuf, need_pull_len);
    }

    /* 获取submsdu的头指针 */
    buffer_data_addr = msdu_state->curr_netbuf_data;
    offset            = msdu_state->submsdu_offset;
    submsdu_hdr      = buffer_data_addr + offset;

    /* 1个netbuf 只包含一个msdu */
    if (msdu_state->msdu_nums_in_netbuf == 1) {
        mac_get_submsdu_len(submsdu_hdr, &submsdu_len);
        oal_set_mac_addr(msdu->sa, (submsdu_hdr + MAC_SUBMSDU_SA_OFFSET));
        oal_set_mac_addr(msdu->da, (submsdu_hdr + MAC_SUBMSDU_DA_OFFSET));

        /* 指向amsdu帧体 */
        oal_netbuf_pull(msdu_state->curr_netbuf, MAC_SUBMSDU_HEADER_LEN);

        if (submsdu_len > OAL_NETBUF_LEN(msdu_state->curr_netbuf)) {
            *pen_proc_state = MAC_PROC_ERROR;
            oam_warning_log2(0, OAM_SF_RX,
                "{hmac_rx_parse_amsdu_etc::submsdu_len %d is not valid netbuf len=%d.}",
                submsdu_len, OAL_NETBUF_LEN(msdu_state->curr_netbuf));
            hmac_rx_free_amsdu_netbuf(msdu_state->curr_netbuf);
            return OAL_FAIL;
        }

        oal_netbuf_trim(msdu_state->curr_netbuf, OAL_NETBUF_LEN(msdu_state->curr_netbuf));

        oal_netbuf_put(msdu_state->curr_netbuf, submsdu_len);

        /* 直接使用该netbuf上报给内核 免去一次netbuf申请和拷贝 */
        b_need_free_netbuf = OAL_FALSE;
        msdu->netbuf = msdu_state->curr_netbuf;
    } else {
        /* 获取submsdu的相关信息 */
        mac_get_submsdu_len(submsdu_hdr, &submsdu_len);
        mac_get_submsdu_pad_len(MAC_SUBMSDU_HEADER_LEN + submsdu_len, &submsdu_pad_len);
        oal_set_mac_addr(msdu->sa, (submsdu_hdr + MAC_SUBMSDU_SA_OFFSET));
        oal_set_mac_addr(msdu->da, (submsdu_hdr + MAC_SUBMSDU_DA_OFFSET));

        /* 针对当前的netbuf，申请新的subnetbuf，并设置对应的netbuf的信息，赋值给对应的msdu */
#ifdef _PRE_LWIP_ZERO_COPY
        msdu->netbuf = oal_pbuf_netbuf_alloc(MAC_SUBMSDU_HEADER_LEN + submsdu_len + submsdu_pad_len);
#else
        msdu->netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,
            (MAC_SUBMSDU_HEADER_LEN + submsdu_len + submsdu_pad_len), OAL_NETBUF_PRIORITY_MID);
#endif
        if (msdu->netbuf == OAL_PTR_NULL) {
            oam_error_log0(0, OAM_SF_RX, "{hmac_rx_parse_amsdu_etc::pst_netbuf null.}");
            OAM_STAT_VAP_INCR(0, rx_no_buff_dropped, 1);
            hmac_rx_free_amsdu_netbuf(msdu_state->curr_netbuf);
            return OAL_FAIL;
        }

        /* 针对每一个子msdu，修改netbuf的end、data、tail、len指针 */
#ifdef _PRE_LWIP_ZERO_COPY
        oal_netbuf_put(msdu->netbuf, submsdu_len + PBUF_ZERO_COPY_RESERVE); // netbuf->len和tail
        oal_netbuf_pull(msdu->netbuf, PBUF_ZERO_COPY_RESERVE);  // netbuf->len - PBUF_ZERO_COPY_RESERVE
#else
        oal_netbuf_put(msdu->netbuf, submsdu_len + HMAC_RX_DATA_ETHER_OFFSET_LENGTH);
        oal_netbuf_pull(msdu->netbuf, HMAC_RX_DATA_ETHER_OFFSET_LENGTH);
#endif
        if (memcpy_s(msdu->netbuf->data, submsdu_len, (submsdu_hdr + MAC_SUBMSDU_HEADER_LEN), submsdu_len) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_rx_parse_amsdu_etc::memcpy_s fail.}");
        }

        b_need_free_netbuf = OAL_TRUE;
    }

    /* 增加当前已处理的msdu的个数 */
    msdu_state->procd_msdu_in_netbuf++;

    /* 获取当前的netbuf中的下一个msdu进行处理 */
    if (msdu_state->procd_msdu_in_netbuf < msdu_state->msdu_nums_in_netbuf) {
        msdu_state->submsdu_offset += submsdu_len + submsdu_pad_len + MAC_SUBMSDU_HEADER_LEN;
    } else if (msdu_state->procd_msdu_in_netbuf == msdu_state->msdu_nums_in_netbuf) {
        msdu_state->procd_netbuf_nums++;

        netbuf_prev = msdu_state->curr_netbuf;

        /* 获取该MPDU对应的下一个netbuf的内容 */
        if (msdu_state->procd_netbuf_nums < msdu_state->netbuf_nums_in_mpdu) {
            msdu_state->curr_netbuf      = OAL_NETBUF_NEXT(msdu_state->curr_netbuf);
            msdu_state->curr_netbuf_data = oal_netbuf_data(msdu_state->curr_netbuf);

            pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(msdu_state->curr_netbuf);

            msdu_state->msdu_nums_in_netbuf  = pst_rx_ctrl->msdu_in_buffer;
            msdu_state->submsdu_offset       = 0;
            msdu_state->procd_msdu_in_netbuf = 0;

            /* amsdu 第二个netbuf len是0, 先put到最大size */
            oal_netbuf_put(msdu_state->curr_netbuf, WLAN_MEM_NETBUF_SIZE2);
        } else if (msdu_state->procd_netbuf_nums == msdu_state->netbuf_nums_in_mpdu) {
            *pen_proc_state = MAC_PROC_LAST_MSDU;
            if (b_need_free_netbuf) {
                oal_netbuf_free(netbuf_prev);
            }
            return OAL_SUCC;
        } else {
            *pen_proc_state = MAC_PROC_ERROR;
            oam_warning_log0(0, OAM_SF_RX,
                "{hmac_rx_parse_amsdu_etc::pen_proc_state is err for procd_netbuf_nums > netbuf_nums_in_mpdul.}");
            hmac_rx_free_amsdu_netbuf(msdu_state->curr_netbuf);
            return OAL_FAIL;
        }
        if (b_need_free_netbuf) {
            oal_netbuf_free(netbuf_prev);
        }
    } else {
        *pen_proc_state = MAC_PROC_ERROR;
        oam_warning_log0(0, OAM_SF_RX,
            "{hmac_rx_parse_amsdu_etc::pen_proc_state is err for procd_netbuf_nums > netbuf_nums_in_mpdul.}");
        hmac_rx_free_amsdu_netbuf(msdu_state->curr_netbuf);
        return OAL_FAIL;
    }

    *pen_proc_state = MAC_PROC_MORE_MSDU;

    return OAL_SUCC;
}

osal_u32 hmac_rx_lan_frame_classify_amsdu(hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf,
    mac_ieee80211_frame_stru *frame_hdr, hmac_user_stru *hmac_user, oal_netbuf_head_stru *w2w_netbuf_hdr)
{
    mac_msdu_proc_status_enum_uint8 process_state = MAC_PROC_BUTT;   /* 解析AMSDU的状态 */
    hmac_msdu_proc_state_stru msdu_state = {0}; /* 记录MPDU的处理信息 */
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    hmac_msdu_stru msdu;                            /* 保存解析出来的每一个MSDU */
    osal_u32 ul_ret;
    osal_u8 *addr = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    osal_u8 is_first = OAL_TRUE;
#endif
    osal_u8 rfc1042_header[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};

    memset_s(&msdu, OAL_SIZEOF(hmac_msdu_stru), 0, OAL_SIZEOF(hmac_msdu_stru));
    mac_get_transmit_addr(frame_hdr, &addr);
    oal_set_mac_addr(msdu.ta, addr);

    /* amsdu 最后一个netbuf next指针设为 NULL 出错时方便释放amsdu netbuf */
    hmac_rx_clear_amsdu_last_netbuf_pointer(pst_netbuf, rx_ctrl->buff_nums);

    do {
        /* 获取下一个要转发的msdu */
        ul_ret = hmac_rx_parse_amsdu_etc(pst_netbuf, &msdu, &msdu_state, &process_state);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_RX,
                "vap_id[%d] {hmac_rx_lan_frame_classify_amsdu::hmac_rx_parse_amsdu_etc failed[%d].}",
                hmac_vap->vap_id, ul_ret);
            return OAL_FAIL;
        }
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        /* Virtual Multi-STA有自己的能力位方式，不通过4地址来识别 */
        if ((is_first == OAL_TRUE) &&
            (hmac_vap->wds_table.wds_vap_mode == WDS_MODE_ROOTAP) &&
            hmac_vmsta_get_user_a4_support(hmac_vap, frame_hdr->address2)) {
            hmac_wds_update_table(hmac_vap, frame_hdr->address2, msdu.sa, WDS_TABLE_ADD_ENTRY);
            is_first = OAL_FALSE;
        }
#endif
        if (memcmp(msdu.da, rfc1042_header, WLAN_MAC_ADDR_LEN) == 0) {
            hmac_rx_free_amsdu_netbuf(pst_netbuf);
            return OAL_FAIL;
        }
        hmac_rx_lan_frame_classify_transmit_msdu_to_lan(hmac_vap, &msdu, w2w_netbuf_hdr, hmac_user);
    } while (process_state != MAC_PROC_LAST_MSDU);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
/*****************************************************************************
 函 数 名  : hmac_parse_ipv4_packet
 功能描述  : 当报文是IPV4类型时，进一步解析type，ipaddr，port
 输入参数  : (1)netbuf,data指针已偏移，指向eth hdr
*****************************************************************************/
OAL_STATIC osal_void hmac_parse_ipv4_packet(osal_void *eth)
{
    const struct iphdr *iph;
    osal_u32          iphdr_len;
    struct tcphdr      *th;
    struct udphdr      *uh;
    struct icmphdr     *icmph;
    iph = (struct iphdr *)((mac_ether_header_stru *)eth + 1);
    iphdr_len = iph->ihl * 4; /* 4：长度转换 */

    wifi_printf(WIFI_WAKESRC_TAG"src ip:%d.%d.%d.*, dst ip:%d.%d.%d.*\n", ipaddr(iph->saddr), ipaddr(iph->daddr));
    if (iph->protocol == IPPROTO_UDP) {
        uh = (struct udphdr *)((osal_u8 *)iph + iphdr_len);
        wifi_printf(WIFI_WAKESRC_TAG"UDP packet, src port:%d, dst port:%d.\n",
            OAL_NTOH_16(uh->source), OAL_NTOH_16(uh->dest));
    } else if (iph->protocol == IPPROTO_TCP) {
        th = (struct tcphdr *)((osal_u8 *)iph + iphdr_len);
        wifi_printf(WIFI_WAKESRC_TAG"TCP packet, src port:%d, dst port:%d.\n",
            OAL_NTOH_16(th->source), OAL_NTOH_16(th->dest));
    } else if (iph->protocol == IPPROTO_ICMP) {
        icmph = (struct icmphdr *)((osal_u8 *)iph + iphdr_len);
        wifi_printf(WIFI_WAKESRC_TAG"ICMP packet, type(%d):%s, code:%d.\n", icmph->type,
            ((icmph->type == 0) ? "ping reply" : ((icmph->type == 8) ? "ping request" : "other icmp pkt")),
            icmph->code);
    } else if (iph->protocol == IPPROTO_IGMP) {
        wifi_printf(WIFI_WAKESRC_TAG"IGMP packet.\n");
    } else {
        wifi_printf(WIFI_WAKESRC_TAG"other IPv4 packet.\n");
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_parse_ipv4_packet
 功能描述  : 当报文是IPV6类型时，进一步解析type，ipaddr
 输入参数  : (1)netbuf,data指针已偏移，指向eth hdr
*****************************************************************************/
OAL_STATIC osal_void hmac_parse_ipv6_packet(osal_void *eth)
{
    struct ipv6hdr *ipv6h;

    ipv6h = (struct ipv6hdr *)((mac_ether_header_stru *)eth + 1);
    wifi_printf(WIFI_WAKESRC_TAG"version: %d, payload length: %d, nh->nexthdr: %d. \n",
        ipv6h->version, OAL_NTOH_16(ipv6h->payload_len), ipv6h->nexthdr);
    wifi_printf(WIFI_WAKESRC_TAG"ipv6 src addr:%04x:%04x:%02x*:*:*:*:*:* \n", ipaddr6(ipv6h->saddr));
    wifi_printf(WIFI_WAKESRC_TAG"ipv6 dst addr:%04x:%04x:%02x*:*:*:*:*:* \n", ipaddr6(ipv6h->daddr));
    if (ipv6h->nexthdr == OAL_IPPROTO_ICMPV6) {
        oal_nd_msg_stru  *rx_nd_hdr;
        rx_nd_hdr   = (oal_nd_msg_stru *)(ipv6h + 1);
        wifi_printf(WIFI_WAKESRC_TAG"ipv6 nd type: %d. \n", rx_nd_hdr->icmph.icmp6_type);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_parse_arp_packet
 功能描述  : 当报文是arp类型时，进一步解析type，subtype
 输入参数  : (1)netbuf,data指针已偏移，指向eth hdr
*****************************************************************************/
OAL_STATIC osal_void hmac_parse_arp_packet(osal_void *eth)
{
    const struct iphdr *iph;
    int iphdr_len;
    struct arphdr *arp;

    iph = (struct iphdr *)((mac_ether_header_stru *)eth + 1);
    iphdr_len = iph->ihl * 4; /* 4：长度转换 */
    arp = (struct arphdr *)((osal_u8 *)iph + iphdr_len);
    wifi_printf(WIFI_WAKESRC_TAG"ARP packet, hardware type:%d, protocol type:%d, opcode:%d.\n",
                 OAL_NTOH_16(arp->ar_hrd), OAL_NTOH_16(arp->ar_pro), OAL_NTOH_16(arp->ar_op));

    return;
}

/*****************************************************************************
 函 数 名  : parse_8021x_packet
 功能描述  : 当报文是arp类型时，进一步解析type，subtype
 输入参数  : (1)netbuf,data指针已偏移，指向eth hdr
*****************************************************************************/
OAL_STATIC osal_void  hmac_parse_8021x_packet(osal_void *eth)
{
#ifdef _PRE_WIFI_PRINTK
    struct ieee8021x_hdr *hdr = (struct ieee8021x_hdr *)((mac_ether_header_stru *)eth + 1);

    wifi_printf(WIFI_WAKESRC_TAG"802.1x frame: version:%d, type:%d, length:%d\n",
        hdr->version, hdr->type, OAL_NTOH_16(hdr->length));
#endif
    return;
}


/*****************************************************************************
 函 数 名  : hmac_parse_packet_etc
 功能描述  : 当系统被wifi唤醒时，解析数据报文的格式。
 输入参数  : (1)netbuf,data指针已偏移，指向eth hdr
*****************************************************************************/
osal_void hmac_parse_packet_etc(oal_netbuf_stru *netbuf_eth)
{
    osal_u16 type;
    mac_ether_header_stru  *ether_hdr;

    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf_eth);
    if (OAL_UNLIKELY(ether_hdr == OAL_PTR_NULL)) {
        wifi_printf(WIFI_WAKESRC_TAG"ether header is null.\n");
        return;
    }

    type = ether_hdr->ether_type;
    wifi_printf(WIFI_WAKESRC_TAG"protocol type:0x%04x\n", OAL_NTOH_16(type));

    if (type == oal_host2net_short(ETHER_TYPE_IP)) {
        hmac_parse_ipv4_packet((osal_void *)ether_hdr);
    } else if (type == oal_host2net_short(ETHER_TYPE_IPV6)) {
        hmac_parse_ipv6_packet((osal_void *)ether_hdr);
    } else if (type == oal_host2net_short(ETHER_TYPE_ARP)) {
        hmac_parse_arp_packet((osal_void *)ether_hdr);
    } else if (type == oal_host2net_short(ETHER_TYPE_PAE)) {
        hmac_parse_8021x_packet((osal_void *)ether_hdr);
    } else {
        wifi_printf(WIFI_WAKESRC_TAG"receive other packet.\n");
    }

    return;
}

#endif

#ifdef _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING
/*****************************************************************************
 函 数 名  : hmac_rx_check_arp_spoofing
 功能描述  : 判断ARP报文是否是ARP欺骗嗅探报文，当收到的ARP报文中其源地址为本AP的IP地址，则认为是ARP欺诈报文，予以丢弃。
 输入参数  : (1)指向net_device的指针
             (2)指向接收报文数据net_buff的指针
 返 回 值  : 是否为ARP欺骗包
*****************************************************************************/
oal_bool_enum_uint8 hmac_rx_check_arp_spoofing(oal_net_device_stru *device, oal_netbuf_stru *pst_netbuf)
{
    mac_ether_header_stru   *ether_header = OAL_PTR_NULL;
    oal_eth_arphdr_stru     *arp_hdr      = OAL_PTR_NULL;
    osal_u16               ether_type;
    osal_u8               *dev_addr   = OAL_PTR_NULL;
    struct net_bridge_port  *br_port    = OAL_PTR_NULL;
    osal_u32               addr_ret;

    oal_netbuf_push(pst_netbuf, ETHER_HDR_LEN);
    ether_header = (mac_ether_header_stru *)OAL_NETBUF_HEADER(pst_netbuf);
    /* 还原skb的data指针 */
    oal_netbuf_pull(pst_netbuf, ETHER_HDR_LEN);
    ether_type = ether_header->ether_type;
    if (ether_type == oal_host2net_short(ETHER_TYPE_ARP)) {
        /* ARP 包地址转换 */
        arp_hdr = (oal_eth_arphdr_stru *)(ether_header + 1);
        if (arp_hdr->ar_pro == oal_host2net_short(ETHER_TYPE_IP)) {
            /* LAN侧ARP包过滤 */
            br_port = br_port_get_rcu(device);
            if (br_port == OAL_PTR_NULL) {
                return OAL_FALSE;
            }
            /* 获取本AP的硬件IP地址 地址相等则为ARP欺骗报文 */
            addr_ret = (osal_u32)inet_select_addr(br_port->br->dev, 0, RT_SCOPE_LINK);
            dev_addr = (osal_u8 *)(&addr_ret);

            if (!osal_memcmp(dev_addr, arp_hdr->ar_sip, arp_hdr->ar_hln)) {
                return OAL_TRUE;
            }
        }
    }
    return OAL_FALSE;
}
#endif  // _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING

#ifdef _PRE_WLAN_TCP_OPT
OAL_STATIC oal_bool_enum_uint8 hmac_transfer_rx_tcp_ack_handler(hmac_device_stru *hmac_device,
    hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
#ifndef WIN32
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    mac_rx_ctl_stru *pst_rx_ctrl;                        /* 指向MPDU控制块信息的指针 */
    oal_netbuf_stru *mac_llc_snap_netbuf;

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_llc_snap_netbuf = (oal_netbuf_stru *)(netbuf->data + pst_rx_ctrl->mac_header_len);
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
    oam_warning_log1(0, OAM_SF_TX,
                     "{hmac_transfer_rx_handler::mac_header_len = %d}\r\n", pst_rx_ctrl->mac_header_len);
#endif
    if (hmac_judge_rx_netbuf_classify_etc(mac_llc_snap_netbuf) == OAL_TRUE) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log0(0, OAM_SF_TX,
                         "{hmac_transfer_rx_handler::netbuf is tcp ack.}\r\n");
#endif
        osal_spin_lock_bh(&hmac_vap->hmac_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        oal_netbuf_list_tail(&hmac_vap->hmac_tcp_ack[HCC_RX].data_queue[HMAC_TCP_ACK_QUEUE], netbuf);
        osal_spin_unlock_bh(&hmac_vap->hmac_tcp_ack[HCC_RX].data_queue_lock[HMAC_TCP_ACK_QUEUE]);
        return OAL_TRUE;
    }
#endif
#endif
    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_transfer_rx_handle
 功能描述  : hmac接收tcp ack过滤接口
*****************************************************************************/
osal_void hmac_transfer_rx_handle(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    oal_netbuf_head_stru *netbuf_header)
{
    oal_netbuf_head_stru                temp_header;
    oal_netbuf_stru                    *pst_netbuf;

    oal_netbuf_head_init(&temp_header);
    while (!!(pst_netbuf = oal_netbuf_delist(netbuf_header))) {
        if (hmac_transfer_rx_tcp_ack_handler(hmac_device, hmac_vap, pst_netbuf) == OAL_FALSE) {
            oal_netbuf_list_tail(&temp_header, pst_netbuf);
        }
    }
    oal_netbuf_splice_init(&temp_header, netbuf_header);
}
#endif

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
/*****************************************************************************
 函 数 名  : hmac_dhcp_get_option
 功能描述  : 获取指定option type的
 输入参数  : const osal_u8 *opt_buff     dhcp option字段指针
             osal_u32 opt_total_len       dhcp option字段长度
             osal_u8  opt                 要获取的option类型
             osal_s32  l_expect_len

 输出参数  : osal_u32 *pul_option_len        获取到的option长度
 返 回 值  : 获取到的option value
*****************************************************************************/
OAL_STATIC const osal_u8 *hmac_dhcp_get_option(const osal_u8 *opt_buff,
    osal_u32 opt_total_len,
    osal_u8  opt,
    osal_u32 *pul_option_len,
    osal_u32  expect_len)
{
    const osal_u8 *buff = opt_buff;
    const osal_u8 *buff_end = buff + opt_total_len;
    osal_u8 opt_len = 0;
    osal_u8 opt_type;
    const osal_u8 *opt_val = OAL_PTR_NULL;

    /* DHCP Options are in TLV format with T and L each being a single
     * byte.We must make sure there is enough room to read both T and L.
     */
    while (buff + 1 < buff_end) {
        opt_type = *buff;
        buff++; // point to length

        if (opt_type == opt) {
            opt_val = buff + 1;
            opt_len += OAL_MIN(*buff, buff_end - opt_val);
        }
        switch (opt_type) {
            case DHO_PAD:
                continue;
            case DHO_END:
                break;
            default:
                break;
        }

        buff += opt_len;  // skip value, now point to type
    }

    if (expect_len > 0 && opt_len != expect_len) {
        return OAL_PTR_NULL;    // unexpect length of value
    }

    if (pul_option_len) {
        *pul_option_len = opt_len;
    }

    return opt_val;
}

OAL_STATIC osal_s32 hmac_dhcp_get_option_uint32(osal_u32 *pul_option_val, const osal_u8 *opt_buff,
    osal_u32 opt_len, osal_u8 option)
{
    const osal_u8 *puc_val =
        hmac_dhcp_get_option(opt_buff, opt_len, option, OAL_PTR_NULL, OAL_SIZEOF(osal_u32));
    osal_u32 val;

    if (!puc_val) {
        return OAL_FAIL;
    }
    if (memcpy_s(&val, OAL_SIZEOF(val), puc_val, OAL_SIZEOF(val)) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_dhcp_get_option_uint32::memcpy_s fail.}");
    }
    if (pul_option_val) {
        *pul_option_val = OAL_NTOH_32(val);
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_dhcp_get_option_uint8(osal_u8 *option_val, const osal_u8 *opt_buff,
    osal_u32 opt_len, osal_u8 option)
{
    const osal_u8 *puc_val =
        hmac_dhcp_get_option(opt_buff, opt_len, option, OAL_PTR_NULL, OAL_SIZEOF(osal_u8));

    if (!puc_val) {
        return OAL_FAIL;
    }

    if (option_val) {
        *option_val = *(puc_val);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_parse_dns_query
 功能描述  : 解析dhcp报文并打印维测
 输入参数  : osal_u8 *buff     dhcp报文内容
             osal_u32 buflen    报文长度
*****************************************************************************/
OAL_STATIC osal_void hmac_parse_special_dhcp_packet(osal_u8 *buff, osal_u32 buflen,
    const osal_u8 *dst, osal_u8 *dhcp_type)
{
    osal_u8  type = 0;
    osal_u32 req_ip = 0;
    osal_u32 req_srv = 0;
    size_t len;
    oal_dhcp_packet_stru *msg;
    if (buflen <= OAL_SIZEOF(oal_dhcp_packet_stru)) {
        oam_warning_log0(0, OAM_SF_ANY, "wifi ipv4 invalid dhcp packet");
        return;
    }
    msg = (oal_dhcp_packet_stru *)buff;
    len = buflen - OAL_SIZEOF(oal_dhcp_packet_stru);
    /* 4：索引值 */
    if (hmac_dhcp_get_option_uint8(&type, &msg->options[4], len, DHO_MESSAGETYPE) == OAL_FAIL) {
        oam_warning_log0(0, OAM_SF_ANY, "wifi ipv4 get message type failed");
        return;
    }

    if (type == DHCP_DISCOVER) {
        oam_warning_log0(0, OAM_SF_ANY, "wifi ipv4 type: DHCP_DISCOVER");
    } else if (type == DHCP_OFFER) {
        oam_warning_log3(0, OAM_SF_ANY, "wifi ipv4 type: DHCP_OFFER yourip:%d.%d.%d.*",
            ((osal_u8*)&(msg->yiaddr))[0], ((osal_u8*)&(msg->yiaddr))[1], ((osal_u8*)&(msg->yiaddr))[2]); // ip地址0 1 2
        oam_warning_log3(0, OAM_SF_ANY, "wifi ipv4 type: DHCP_OFFER srvip:%d.%d.%d.*",
            ((osal_u8*)&(msg->siaddr))[0], ((osal_u8*)&(msg->siaddr))[1], ((osal_u8*)&(msg->siaddr))[2]); // ip地址0 1 2
        oam_warning_log4(0, OAM_SF_ANY, "wifi ipv4 type: DHCP_OFFER MAC:" HWMACSTR"",
            dst[0], dst[1], dst[2], dst[3]);
    } else if (type == DHCP_REQUEST) {
        hmac_dhcp_get_option_uint32(&req_ip, &msg->options[4], len, DHO_IPADDRESS); /* 4：索引值 */
        hmac_dhcp_get_option_uint32(&req_srv, &msg->options[4], len, DHO_SERVERID); /* 4：索引值 */
        req_ip = OAL_NTOH_32(req_ip);
        req_srv = OAL_NTOH_32(req_srv);
        oam_warning_log0(0, OAM_SF_ANY, "wifi ipv4 type: DHCP_REQUEST");
    } else if (type == DHCP_ACK) {
        oam_warning_log4(0, OAM_SF_ANY, "wifi ipv4 type: DHCP_ACK MAC:" HWMACSTR "",
            dst[0], dst[1], dst[2], dst[3]);
    } else if (type == DHCP_NAK) {
        oam_warning_log4(0, OAM_SF_ANY, "wifi ipv4 type: DHCP_NAK MAC:" HWMACSTR "",
            dst[0], dst[1], dst[2], dst[3]);
    }

    if (dhcp_type != OSAL_NULL) {
        *dhcp_type = type;
    }
}

/* 解析dns domain */
OAL_STATIC OAL_INLINE osal_void hmac_parse_dns_domain(osal_u8 qlen, osal_u8 *buff, const osal_u8 *end, osal_u8 *domain,
    const osal_u8 *domain_end)
{
    osal_u8 *domain_tmp = domain;
    if ((buff + qlen < end) && (domain_tmp + qlen <= domain_end)) {
        if (memcpy_s(domain, qlen, buff, qlen) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_parse_dns_domain::memcpy_s fail.}");
        }
        domain_tmp += qlen;
        *domain_tmp = '.';
        *(domain_tmp + 1) = '\0';
    }
}

/*****************************************************************************
 函 数 名  : hmac_parse_dns_query
 功能描述  : 解析dns query并打印维测
 输入参数  : osal_u8 *msg     dns报文内容
             osal_u32 msg_len  报文长度
             osal_u16 qdcount  question record count
 返 回 值  : 成功或者失败原因
*****************************************************************************/
OAL_STATIC osal_void hmac_parse_dns_query(osal_u8 *msg, osal_u32 msg_len)
{
    osal_u8 *buff = msg;
    osal_u8 *end = msg + msg_len;
    osal_u8  qlen;
    osal_u8  auc_domain[DNS_MAX_DOMAIN_LEN + 2] = {0};
    osal_u8 *puc_domain = auc_domain;
    osal_u8 *domain_end = auc_domain + DNS_MAX_DOMAIN_LEN;

    while (buff < end) {
        qlen = *buff++;
        if (qlen > 0) {
            hmac_parse_dns_domain(qlen, buff, end, puc_domain, domain_end);
            buff += qlen;
        } else {
            // only wifi_printf one record of query item
            break;
        }
    }
    wifi_printf(WIFI_SEPCIAL_IPV4_PKT_TAG"domain name: %s \n", auc_domain);
}

/*****************************************************************************
 函 数 名  : hmac_parse_dns_answer
 功能描述  : 解析dns answer并打印维测
 输入参数  : osal_u8 *msg     报文内容(不包含dns首部)
             osal_u32 msg_len  报文长度
             osal_u16 qdcount  question record count
 返 回 值  : 成功或者失败原因
*****************************************************************************/
OAL_STATIC osal_void hmac_parse_dns_answer(osal_u8 *msg, osal_u32 msg_len, osal_u16 qdcount)
{
    osal_u8 *buff = msg;
    osal_u8 *end = msg + msg_len;
    osal_u8  qlen;
    osal_u16 type = 0;
    osal_u16 rdlen = 0;
    osal_u32 ipv4 = 0;
    osal_u8  auc_domain[DNS_MAX_DOMAIN_LEN + 2] = {0};
    osal_u8  domain_s[DNS_MAX_DOMAIN_LEN + 2] = {0};
    osal_u8 *puc_domain = auc_domain;
    osal_u8 *domain_end = auc_domain + DNS_MAX_DOMAIN_LEN;
    osal_u16 loop_count = qdcount;
    // skip Questions
    while (loop_count > 0 && buff < end) {
        qlen = *buff++;
        if (qlen > 0) {
            hmac_parse_dns_domain(qlen, buff, end, puc_domain, domain_end);
            buff += qlen;
        } else {
            buff += 4;    // 4：class: 2 bytes, type: 2 bytes
            loop_count--;
            /* 数组定义len DNS_MAX_DOMAIN_LEN + 2 */
            (osal_void)memcpy_s(domain_s, sizeof(domain_s), auc_domain, sizeof(auc_domain));
            puc_domain = auc_domain;
            domain_end = auc_domain + DNS_MAX_DOMAIN_LEN;
            memset_s(auc_domain, OAL_SIZEOF(auc_domain), 0, OAL_SIZEOF(auc_domain));
        }
    }

    // parse Answers
    while (buff + 12 < end) { /* 12：长度判断 */
        buff += 2;    // 2：name: 2 bytes
        if (memcpy_s(&type, OAL_SIZEOF(type), buff, OAL_SIZEOF(type)) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_parse_dns_answer::memcpy_s fail.}");
        }
        buff += 8;    // 8：type, class: 2bytes, ttl: 4bytes
        if (memcpy_s(&rdlen, OAL_SIZEOF(rdlen), buff, OAL_SIZEOF(rdlen)) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_parse_dns_answer::memcpy_s fail.}");
        }
        buff += 2; /* 2：跳转长度 */
        type = OAL_NTOH_16(type);
        rdlen = OAL_NTOH_16(rdlen);
        if (type == OAL_NS_T_A && rdlen == 4) { /* 4：长度判断 */
            if (memcpy_s(&ipv4, rdlen, buff, rdlen) != EOK) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_parse_dns_answer::memcpy_s fail.}");
            }
            oam_warning_log3(0, OAM_SF_ANY, "wifi ipv4 domain name: %d.%d.%d.*",
                ((osal_u8*)&(ipv4))[0], ((osal_u8*)&(ipv4))[1], ((osal_u8*)&(ipv4))[2]); // ip地址0 1 2
            return;
        }
        buff += rdlen;
    }
    wifi_printf(WIFI_SEPCIAL_IPV4_PKT_TAG"domain name: %s\n", domain_s);
}

/*****************************************************************************
 函 数 名  : hmac_parse_special_dns_packet
 功能描述  : 解析dns报文并打印维测
 输入参数  : osal_u8 *msg     报文内容(不包含dns首部)
             osal_u32 msg_len  报文长度
 返 回 值  : 成功或者失败原因
*****************************************************************************/
OAL_STATIC osal_void hmac_parse_special_dns_packet(osal_u8 *msg, osal_u32 msg_len)
{
    osal_u8 qr, opcode, rcode;
    osal_u16 flag, qdcount;
    size_t dns_hdr_len = OAL_SIZEOF(oal_dns_hdr_stru);
    oal_dns_hdr_stru *dns_hdr = NULL;

    if (dns_hdr_len >= msg_len) {
        oam_warning_log0(0, OAM_SF_ANY, "wifi ipv4 invalid dns packet");
        return;
    }

    dns_hdr = (oal_dns_hdr_stru *)msg;
    flag = OAL_NTOH_16(dns_hdr->flags);
    qdcount = OAL_NTOH_16(dns_hdr->qdcount);
    qr = DNS_GET_QR_FROM_FLAG(flag);
    opcode = (osal_u8)((flag & 0x7400U) >> 11); /* dns_get_opcode_from_flag 11 */
    rcode = (osal_u8)(flag & 0x000fU); /* dns_get_rcode_from_flag */

    if (qr == OAL_NS_Q_REQUEST) {
        if (opcode == OAL_NS_O_QUERY && qdcount > 0) {
            hmac_parse_dns_query(msg + dns_hdr_len, msg_len - dns_hdr_len);
        } else {
            oam_warning_log2(0, OAM_SF_ANY, "wifi ipv4 dont parse dns request pkt, opcode: %u, qd: %u",
                opcode, qdcount);
        }
    } else if (qr == OAL_NS_Q_RESPONSE) {
        if (opcode == OAL_NS_O_QUERY && rcode == OAL_NS_R_NOERROR) {
            hmac_parse_dns_answer(msg + dns_hdr_len, msg_len - dns_hdr_len, qdcount);
        } else {
            oam_warning_log2(0, OAM_SF_ANY, "wifi ipv4 dont parse dns response pkt, opcode: %u, rcode: %u",
                opcode, rcode);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_parse_special_ipv4_packet
 功能描述  : 解析dhcp&dns报文并打印维测
 输入参数  : osal_u8 *pktdata 以太网报文内容
             osal_u32 datalen  报文长度
             hmac_pkt_direction_enum pkt_direction 报文方向
 返 回 值  : 成功或者失败原因
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_void hmac_parse_special_ipv4_packet(const hmac_vap_stru *hmac_vap, osal_u8 *pktdata,
    osal_u32 datalen, hmac_pkt_direction_enum pkt_direction)
{
    osal_u32 iphdr_len;
    const struct iphdr *iph;
    struct udphdr *uh;
    osal_u16 src_port;
    osal_u16 dst_port;
    osal_u16 udp_len;
    osal_u16 tot_len;
    osal_u16 udphdr_len;
    osal_u8 *uplayer_data;
    osal_u8 dhcp_type = 0xff;

    // invalid ipv4 packet
    if ((pktdata == NULL) || (datalen <= ETH_HLEN + 20)) { /* 20：长度限制 */
        return;
    }
    iph = (struct iphdr *)(pktdata + ETH_HLEN);
    iphdr_len = iph->ihl * 4; /* 4：长度转换 */
    tot_len = OAL_NTOH_16(iph->tot_len);
    // invalid ipv4 packet
    if ((datalen < (osal_u32)(tot_len + ETH_HLEN)) || (iph->protocol != IPPROTO_UDP)) {
        return;
    }

    uh = (struct udphdr *)(pktdata + ETH_HLEN + iphdr_len);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    src_port = OAL_NTOH_16(uh->source);
    dst_port = OAL_NTOH_16(uh->dest);
    udp_len = OAL_NTOH_16(uh->len);
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    src_port = oal_ntoh_16(uh->uh_sport);
    dst_port = oal_ntoh_16(uh->uh_dport);
    udp_len  = oal_ntoh_16(uh->uh_ulen);
#endif

    udphdr_len = (osal_u16)OAL_SIZEOF(struct udphdr);

    // invalid udp packet
    if (udp_len <= udphdr_len) {
        return;
    }

    udp_len -= udphdr_len;  // skip udp header
    uplayer_data = (osal_u8 *)(pktdata + ETH_HLEN + iphdr_len + udphdr_len); // skip udp header
    if ((src_port == DNS_SERVER_PORT) || (dst_port == DNS_SERVER_PORT)) {
        if (hmac_vap != OSAL_NULL && hmac_vap->dhcp_debug == 1) {
            oam_warning_log1(0, OAM_SF_ANY, "wifi ipv4 dhcp:dir[%d][0:tx 1:rx] parse dns packet", pkt_direction);
            hmac_parse_special_dns_packet(uplayer_data, udp_len);
        }
    } else if (((src_port == DHCP_SERVER_PORT) && (dst_port == DHCP_CLIENT_PORT)) ||
               ((dst_port == DHCP_SERVER_PORT) && (src_port == DHCP_CLIENT_PORT))) {
        hmac_parse_special_dhcp_packet(uplayer_data, udp_len, pktdata + ETH_ALEN, &dhcp_type);
        oam_warning_log3(0, 0, "dhcp:dir[%d][0:tx 1:rx] type[%d][1:discovery 2:offer 3:request 5:ack 6:nack] vap[%d]",
            pkt_direction, dhcp_type, hmac_vap->vap_id);
    }
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

