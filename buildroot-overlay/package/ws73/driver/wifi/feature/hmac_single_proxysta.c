/*
 * Copyright (c) CompanyNameMagicTag 2017-2023. All rights reserved.
 * 文 件 名   : hmac_single_proxysta.c
 * 生成日期   : 2017年5月15日
 * 功能描述   : Single Proxy STA 特性驱动相关函数: 只创建一个STA，通过IP MAC表格实现报文转发
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_single_proxysta.h"
#include "frw_util_notifier.h"
#include "mac_vap_ext.h"
#include "hmac_feature_interface.h"
#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
#include "hmac_ccpriv.h"
#include "hmac_tx_data.h"
#include "hmac_hook.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_SINGLE_PROXYSTA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

hmac_single_proxysta_stru g_single_proxysta;
osal_bool g_pkt_trace = 0;
#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
OSAL_STATIC osal_u32 hmac_bridge_rx_data_process(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_void hmac_bridge_control_addbr(const osal_char *param);
OSAL_STATIC osal_void hmac_bridge_control_delbr(const osal_char *param);
OSAL_STATIC osal_void hmac_bridge_control_show_bridge(const osal_char *param);

hmac_brctl_stru g_bridge_ctrl;
#endif

#define hmac_bridge_loop_all_node_safe(head_loop, pos, n, head)  \
    for ((head_loop) = 0; (head_loop) < osal_array_size(head); (head_loop)++) \
        osal_list_for_each_safe((pos), (n), &(head)[(head_loop)])

#define hmac_bridge_loop_all_node(head_loop, pos, head)  \
    for ((head_loop) = 0; (head_loop) < osal_array_size(head); (head_loop)++) \
        osal_list_for_each((pos), &(head)[(head_loop)])


static osal_void debug_mac_addr(const osal_char *addr_str, const osal_u8 *addr)
{
    wifi_printf("\r\n%s=[%x:%x:%x:%x:**:**]\r\n", addr_str,
        addr[0], addr[1], addr[2], addr[3]);  /* 打印mac地址第0 1 2 3位 */
}

static osal_void debug_ip_addr(const osal_char *addr_str, const osal_u8 *addr)
{
    wifi_printf("\r\n%s=[%d.%d.%d.**]\r\n", addr_str, addr[0], addr[1], addr[2]); /* 打印ip地址第0 1 2位 */
}

static osal_void debug_arp_addr(const hmac_vap_stru *hmac_vap, mac_ether_header_stru *ether_header,
    oal_eth_arphdr_stru *arp_package)
{
    debug_mac_addr("RX_DHCP:bsta_mac", hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id);
    debug_mac_addr("RX_ARP:src_mac", ether_header->ether_shost);
    debug_mac_addr("RX_ARP:dst_mac", ether_header->ether_dhost);
    debug_mac_addr("RX_ARP:sender_hw", arp_package->ar_sha);
    debug_ip_addr("RX_ARP:sender_ip", arp_package->ar_sip);
    debug_mac_addr("RX_ARP:target_hw", arp_package->ar_tha);
    debug_ip_addr("RX_ARP:target_ip", arp_package->ar_tip);
}

static osal_void debug_dhcp_addr(const hmac_vap_stru *hmac_vap, mac_ether_header_stru *ether_header,
    dhcp_message_stru *dhcp_package)
{
    debug_mac_addr("RX_DHCP:bsta_mac", hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id);
    debug_mac_addr("RX_DHCP:src_mac", ether_header->ether_shost);
    debug_mac_addr("RX_DHCP:dst_mac", ether_header->ether_dhost);
    debug_mac_addr("RX_DHCP:chaddr", dhcp_package->chaddr);
}

static osal_void debug_icmp_addr(const hmac_vap_stru *hmac_vap, mac_ether_header_stru *ether_header,
    const oal_ip_header_stru *ip_header)
{
    if (ip_header->protocol == MAC_ICMP_PROTOCAL) {
        debug_mac_addr("RX_DHCP:bsta_mac", hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id);
        debug_mac_addr("RX_ICMP:dst_mac", ether_header->ether_dhost);
        debug_mac_addr("RX_ICMP:src_mac", ether_header->ether_shost);
    }
}

/*****************************************************************************
 函 数 名  : hmac_bridge_delete_disassociation_mac
 功能描述  : 删除MAP表中已经与Repeater AP去关联的STA的MAC地址记录
*****************************************************************************/
OSAL_STATIC osal_bool hmac_single_proxysta_user_del(osal_void *notify_data)
{
    hmac_vap_bridge_stru *vap_bridge = g_single_proxysta.vap_bridge;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_vap_stru *hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    struct osal_list_head *dlist_entry = OSAL_NULL;
    struct osal_list_head *temp = OSAL_NULL;
    hmac_bridge_ipv4_hash_stru *hash_ipv4 = OSAL_NULL;
    hmac_bridge_unknow_hash_stru *hash_unknow = OSAL_NULL;
    osal_u8 *mac_addr = hmac_user->user_mac_addr;
    osal_u8 i;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP || vap_bridge == OSAL_NULL) {
        return OSAL_TRUE;
    }

    if (vap_bridge->map_ipv4_num != 0) {
        osal_spin_lock(&vap_bridge->map_lock);
        hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_ipv4_head) {
            hash_ipv4 = osal_list_entry(dlist_entry, hmac_bridge_ipv4_hash_stru, entry);
            if (memcmp(hash_ipv4->mac, mac_addr, WLAN_MAC_ADDR_LEN) == 0) {
                oam_info_log4(0, OAM_SF_PROXYSTA, "{hmac_single_proxysta_user_del:del ipv4 map mac[%x:%x:%x:%x:**:**]}",
                    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);     // 打印mac地址第0 1 2 3位
                osal_dlist_delete_entry(dlist_entry);
                oal_free(hash_ipv4);
                vap_bridge->map_ipv4_num--;
                osal_spin_unlock(&vap_bridge->map_lock);
                return OSAL_TRUE;
            }
        }
        osal_spin_unlock(&vap_bridge->map_lock);
    }

    if (vap_bridge->map_unknow_num != 0) {
        osal_spin_lock(&vap_bridge->map_lock);
        hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_unknow_head) {
            hash_unknow = osal_list_entry(dlist_entry, hmac_bridge_unknow_hash_stru, entry);
            if (memcmp(hash_unknow->mac, mac_addr, WLAN_MAC_ADDR_LEN) == 0) {
                osal_dlist_delete_entry(dlist_entry);
                oal_free(hash_unknow);
                vap_bridge->map_unknow_num--;
                osal_spin_unlock(&vap_bridge->map_lock);
                return OSAL_TRUE;
            }
        }
        osal_spin_unlock(&vap_bridge->map_lock);
    }

    /* 未查找到对应记录，返回成功 */
    oam_info_log4(0, OAM_SF_PROXYSTA, "{hmac_single_proxysta_user_del:not find map[%x:%x:%x:%x:**:**]}",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3]);  // 打印mac地址第0 1 2 3位
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_find_unknow_mac
 功能描述  : 根据协议类型地址从MAP表格中获取对应的MAC地址
*****************************************************************************/
static osal_u32 hmac_bridge_find_unknow_mac(hmac_vap_bridge_stru *vap_bridge, osal_u16 protocol,
    osal_u8 *mac_addr)
{
    osal_u8 hash_tmp;
    struct osal_list_head *dlist_entry = OSAL_NULL;
    hmac_bridge_unknow_hash_stru *hash_unknow = OSAL_NULL;

    if (vap_bridge == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_find_unknow_mac:: null param}");
        return OSAL_FAILURE;
    }
    /* 获取HASH桶值，HASH链表 */
    hash_tmp = (osal_u8)hmac_bridge_cal_unknow_hash(protocol);
    osal_spin_lock(&vap_bridge->map_lock);
    osal_list_for_each(dlist_entry, &vap_bridge->map_unknow_head[hash_tmp]) {
        hash_unknow = osal_list_entry(dlist_entry, hmac_bridge_unknow_hash_stru, entry);
        if (hash_unknow->protocol == protocol) {
            if (memcpy_s(mac_addr, WLAN_MAC_ADDR_LEN, hash_unknow->mac, WLAN_MAC_ADDR_LEN) != EOK) {
                osal_spin_unlock(&vap_bridge->map_lock);
                return OSAL_FAILURE;
            }
            hash_unknow->last_active_timestamp = (osal_u32)oal_time_get_stamp_ms();
            osal_spin_unlock(&vap_bridge->map_lock);
            return OSAL_SUCCESS;
        }
    }
    osal_spin_unlock(&vap_bridge->map_lock);
    return OSAL_FAILURE;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_rx_unknow_addr_replace
 功能描述  : 未知协议报文地址转换: 保存一个协议类型与MAC地址的map表，下一次再次发现此协议报文时，直接使用该MAC替换
             存在风险: 协议类型不唯一，存在多个STA发同样的未知协议报文时，STA无法正确收到回复
*****************************************************************************/
static osal_u32 hmac_bridge_rx_unknow_addr_replace(const hmac_vap_stru *hmac_vap,
    mac_ether_header_stru *ether_header, osal_u32 pkt_len)
{
    osal_u32 contig_len = sizeof(mac_ether_header_stru);
    /* 获取以太网的目的MAC和数据段 */
    osal_u8 *des_mac = ether_header->ether_dhost;
    /* 获取以太网帧目的地址是否为多播地址 */
    osal_u8 is_mcast = ether_is_multicast(des_mac);
    osal_u8 puc_oma[WLAN_MAC_ADDR_LEN] = {0};

    unref_param(hmac_vap);

    if (pkt_len < contig_len) {
        oam_error_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_rx_unknow_addr_replace::The length of buf is invalid.}");
        return OSAL_FAILURE;
    }

    /* 多播报文不需要替换目的地址 */
    if (is_mcast == OSAL_TRUE) {
        return OSAL_SUCCESS;
    }

    /* 查找MAP表格中protocol对应的mac地址 */
    if (hmac_bridge_find_unknow_mac(g_single_proxysta.vap_bridge, ether_header->ether_type, puc_oma) != OSAL_SUCCESS) {
        oam_error_log1(0, OAM_SF_PROXYSTA,
            "{hmac_bridge_rx_unknow_addr_replace:: can't find mac addr of unknow protocol:0x%x.}",
            ether_header->ether_type);
        return OSAL_FAILURE;
    }
    /* 更新以太网的目的地址为实际的STA MAC地址 */
    if (memcpy_s(des_mac, WLAN_MAC_ADDR_LEN, puc_oma, WLAN_MAC_ADDR_LEN) != EOK) {
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_find_ipv4_mac
 功能描述  : 根据IPV4的IP地址从MAP表格中获取对应的MAC地址
*****************************************************************************/
static osal_u32 hmac_bridge_find_ipv4_mac(hmac_vap_bridge_stru *vap_bridge,
    const osal_u8 *ip_addr, osal_u8 *mac_addr)
{
    osal_u8 hash_tmp;
    struct osal_list_head *dlist_entry = OSAL_NULL;
    hmac_bridge_ipv4_hash_stru *hash_ipv4 = OSAL_NULL;

    if (vap_bridge == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_find_ipv4_mac:: vap_proxysta  null param.}");
        return OSAL_FAILURE;
    }

    /* 获取HASH桶值，HASH链表 */
    hash_tmp = (osal_u8)hmac_bridge_cal_ipv4_hash(ip_addr);
    osal_spin_lock(&vap_bridge->map_lock);
    osal_list_for_each(dlist_entry, &vap_bridge->map_ipv4_head[hash_tmp]) {
        hash_ipv4 = osal_list_entry(dlist_entry, hmac_bridge_ipv4_hash_stru, entry);
        if (memcmp(hash_ipv4->ipv4, ip_addr, ETH_TARGET_IP_ADDR_LEN) == 0) {
            if (memcpy_s(mac_addr, WLAN_MAC_ADDR_LEN, hash_ipv4->mac, WLAN_MAC_ADDR_LEN) != EOK) {
                osal_spin_unlock(&vap_bridge->map_lock);
                return OSAL_FAILURE;
            }
            hash_ipv4->last_active_timestamp = (osal_u32)oal_time_get_stamp_ms();
            osal_spin_unlock(&vap_bridge->map_lock);
            return OSAL_SUCCESS;
        }
    }
    osal_spin_unlock(&vap_bridge->map_lock);
    return OSAL_FAILURE;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_rx_arp_addr_replace
 功能描述  : 根据arp包中的IP地址替换查找路由表进行MAC地址转换
*****************************************************************************/
static osal_u32 hmac_bridge_rx_arp_addr_replace(const hmac_vap_stru *hmac_vap,
    mac_ether_header_stru *ether_header, osal_u32 pkt_len)
{
    /****************************************************************************/
    /*                      ARP Frame Format                                    */
    /* ------------------------------------------------------------------------ */
    /* |以太网目的地址|以太网源地址|帧类型|硬件类型|协议类型|硬件地址长度|      */
    /* ------------------------------------------------------------------------ */
    /* | 6 (待替换)   |6           |2     |2       |2       |1           |      */
    /* ------------------------------------------------------------------------ */
    /* |协议地址长度|op|发送端以太网地址|发送端IP地址|目的以太网地址|目的IP地址 */
    /* ------------------------------------------------------------------------ */
    /* | 1          |2 |6               |4           |6 (待替换)    |4          */
    /* ------------------------------------------------------------------------ */
    /*                                                                          */
    /****************************************************************************/

    osal_u32 contig_len = sizeof(mac_ether_header_stru);
    /* 获取以太网的目的MAC和数据段 */
    osal_u8 *des_mac = ether_header->ether_dhost;
    osal_u8 *eth_body = (osal_u8 *)(ether_header + 1);
    /* 获取以太网帧目的地址是否为多播地址 */
    osal_u8 is_mcast = ether_is_multicast(des_mac);
    /* ARP包地址转换 */
    oal_eth_arphdr_stru *arp_package = (oal_eth_arphdr_stru *)eth_body;
    osal_u8 puc_oma[WLAN_MAC_ADDR_LEN] = {0};

    if (g_pkt_trace == OSAL_TRUE) {
        debug_arp_addr(hmac_vap, ether_header, arp_package);
    }

    contig_len += sizeof(oal_eth_arphdr_stru);
    if (pkt_len < contig_len) {
        oam_error_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_rx_arp_addr_replace::The length of buf is invalid.}");
        return OSAL_FAILURE;
    }

    if (hmac_bridge_find_ipv4_mac(g_single_proxysta.vap_bridge, arp_package->ar_tip, puc_oma) != OSAL_SUCCESS) {
        return OSAL_FAILURE;
    }

    /* 替换arp报文中的目的mac地址为真实的mac地址 */
    if (memcpy_s(arp_package->ar_tha, WLAN_MAC_ADDR_LEN, puc_oma, WLAN_MAC_ADDR_LEN) != EOK) {
        return OSAL_FAILURE;
    }
    /* 单播报文需要替换以太网目的地址 */
    if (is_mcast == OSAL_FALSE) {
        if (memcpy_s(des_mac, WLAN_MAC_ADDR_LEN, puc_oma, WLAN_MAC_ADDR_LEN) != EOK) {
            return OSAL_FAILURE;
        }
    }
    return OSAL_SUCCESS;
}

static osal_void hmac_bridge_dhcp_checksum(mac_udp_header_stru *udp_header, dhcp_message_stru *dhcp_package)
{
    osal_u16 old_flag;
    osal_u32 new_sum;

    old_flag = dhcp_package->flags;
    dhcp_package->flags = oal_host2net_short(DHCP_FLAG_BCAST);
    /* 修改后重新计算UDP的校验和 */
    new_sum = (osal_u32)udp_header->check_sum;
    new_sum += old_flag + (~(dhcp_package->flags) & 0XFFFF);
    new_sum = (new_sum >> 16) + (new_sum & 0XFFFF);             // 左移16位重新计算checksum
    udp_header->check_sum = (osal_u16)((new_sum >> 16) + new_sum);  // 左移16位重新计算checksum
}

/*****************************************************************************
 函 数 名  : hmac_bridge_rx_ip_addr_replace
 功能描述  : ip包地址转换，主要包括以下两种报文的处理:
             1.DHCP报文的处理；
             2.其他IP类型报文的处理
*****************************************************************************/
static osal_u32 hmac_bridge_rx_ip_addr_replace(const hmac_vap_stru *hmac_vap,
    mac_ether_header_stru *ether_header, osal_u32 pkt_len)
{
    osal_u32 contig_len = sizeof(mac_ether_header_stru);
    /* 获取以太网的目的MAC和数据段 */
    osal_u8 *des_mac = ether_header->ether_dhost;
    oal_ip_header_stru *ip_header = (oal_ip_header_stru *)(ether_header + 1);
    osal_u8 puc_oma[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 *puc_ipv4 = OSAL_NULL;

    /*************************************************************************/
    /*                    IP头格式 (oal_ip_header_stru)                      */
    /* --------------------------------------------------------------------- */
    /* | 版本 | 报头长度 | 服务类型 | 总长度  |标识  |标志  |段偏移量 |      */
    /* --------------------------------------------------------------------- */
    /* | 4bits|  4bits   | 1        | 2       | 2    |3bits | 13bits  |      */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* | 生存期 | 协议        | 头部校验和| 源地址(SrcIp)|目的地址(DstIp)    */
    /* --------------------------------------------------------------------- */
    /* | 1      |  1 (17为UDP)| 2         | 4              | 4               */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    contig_len += sizeof(oal_ip_header_stru);
    if (pkt_len < contig_len) {
        return OSAL_FAILURE;
    }

    /* 如果是UDP包，并且是DHCP协议的报文处理 */
    if (ip_header->protocol == OAL_IPPROTO_UDP) {
        mac_udp_header_stru *udp_header = (mac_udp_header_stru *)(ip_header + 1);
        contig_len += sizeof(mac_udp_header_stru);
        if (pkt_len < contig_len) {
            return OSAL_FAILURE;
        }
        /*************************************************************************/
        /*                      UDP 头 (oal_udp_header_stru)                     */
        /* --------------------------------------------------------------------- */
        /* |源端口号（SrcPort）|目的端口号（DstPort）| UDP长度    | UDP检验和  | */
        /* --------------------------------------------------------------------- */
        /* | 2                 | 2                   |2           | 2          | */
        /* --------------------------------------------------------------------- */
        /*                                                                       */
        /*************************************************************************/
        /* DHCP request UDP Client SP = 68 (bootpc), DP = 67 (bootps) */
        /* Repeater STA接收的DHCP应答报文不会是单播报文 故不区分单播报文 */
        if (oal_host2net_short(udp_header->des_port) == DHCP_PORT_BOOTPC) {
            dhcp_message_stru *dhcp_package = (dhcp_message_stru *)(udp_header + 1);
            contig_len += (sizeof(dhcp_message_stru) - DHCP_OPTION_LEN);
            if (pkt_len < contig_len) {
                return OSAL_FAILURE;
            }

            if (g_pkt_trace) {
                debug_dhcp_addr(hmac_vap, ether_header, dhcp_package);
            }

            /* 客户端发过来的DHCP请求报文 更改标志字段要求服务器以广播形式发送ACK 如果是自己的DHCP则不更改 要求服务器以单播形式回复 */
            /* STA的应用场景应该不会接收到DHCP REQUEST(除非DHCP服务器部署在REPEATER上)， 更不可能收到自己的DHCP REQUEST */
            if (memcmp(hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id,
                       dhcp_package->chaddr, WLAN_MAC_ADDR_LEN) != 0) {
                hmac_bridge_dhcp_checksum(udp_header, dhcp_package);
            }
            return OSAL_SUCCESS;
        }
    }

    /* 多播报文不需要替换目的地址 */
    if (ether_is_multicast(des_mac) == OSAL_TRUE) {
        return OSAL_SUCCESS;
    }

    if (g_pkt_trace) {
        debug_icmp_addr(hmac_vap, ether_header, ip_header);
    }

    puc_ipv4 = (osal_u8 *)(&ip_header->daddr);
    if (hmac_bridge_find_ipv4_mac(g_single_proxysta.vap_bridge, puc_ipv4, puc_oma) != OSAL_SUCCESS) {
        return OSAL_FAILURE;
    }

    /* 更新以太网的目的地址：由原来的proxysta的mac地址替换为查找到的下挂sta的mac地址 */
    if (memcpy_s(des_mac, WLAN_MAC_ADDR_LEN, puc_oma, WLAN_MAC_ADDR_LEN) != EOK) {
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}


/*****************************************************************************
 函 数 名  : hmac_bridge_rx_process
 功能描述  : ARP、DHCP等包，上报网桥前地址转换函数
             目前已知需要转换地址的报文有 IP:0X0800  ARP: 0X0806
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_bridge_rx_process(const oal_netbuf_stru *netbuf,
    const hmac_vap_stru *hmac_vap)
{
    mac_ether_header_stru *ether_header = OSAL_NULL;
    osal_u32 pkt_len;
    osal_u16 ether_type;
    hmac_vap_stru *mac_vap_temp = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;

    if (netbuf == OSAL_NULL || hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_rx_process:null param}");
        return OSAL_FAILURE;
    }

    /* ap与sta不能同时存在的时候都不进入到repeater流程中，g_single_proxysta.vap_id记录的是sta的vap_id */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (mac_device_find_up_ap_etc(hmac_device, &mac_vap_temp) != OAL_SUCC) {
        return  OSAL_SUCCESS;
    }

    if (hmac_vap->vap_id != g_single_proxysta.vap_id) {
        return OSAL_SUCCESS;
    }

    if (!is_legacy_vap(mac_vap_temp)) {
        return OSAL_SUCCESS;
    }

    ether_header = (mac_ether_header_stru *)oal_netbuf_data((oal_netbuf_stru *)netbuf);
    if (ether_header == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_rx_process:null param.}");
        return OSAL_FAILURE;
    }
    pkt_len = oal_netbuf_get_len((oal_netbuf_stru *)netbuf);

    /* 获取以太网报文的数据 PROXYSTA将根据报文类型将数据的目的地址进行替换 */
    /******************************************/
    /*        Ethernet Frame Format           */
    /* -------------------------------------  */
    /* |Dst      MAC | Source MAC   | TYPE |  */
    /* -------------------------------------  */
    /* | 6           | 6            | 2    |  */
    /* -------------------------------------  */
    /*                                        */
    /******************************************/

    ether_type = ether_header->ether_type;
    /* ether_type 小于0x0600非协议报文 不处理 */
    if (oal_host2net_short(ether_type) < ETHER_TYPE_START) {
        return OSAL_SUCCESS;
    }
    if (ether_type == oal_host2net_short(ETHER_TYPE_IP)) {
        /* IP包地址转换 */
        return hmac_bridge_rx_ip_addr_replace(hmac_vap, ether_header, pkt_len);
    } else if (ether_type == oal_host2net_short(ETHER_TYPE_ARP)) {
        /* ARP 包地址转换 */
        return hmac_bridge_rx_arp_addr_replace(hmac_vap, ether_header, pkt_len);
    } else if (ether_type == oal_host2net_short(ETHER_TYPE_IPV6) ||
        ether_type == oal_host2net_short(ETHER_TYPE_IPX) ||
        ether_type == oal_host2net_short(ETHER_TYPE_AARP) ||
        ether_type == oal_host2net_short(ETHER_TYPE_PPP_DISC) ||
        ether_type == oal_host2net_short(ETHER_TYPE_PPP_SES) ||
        ether_type == oal_host2net_short(ETHER_TYPE_PAE) ||
        ether_type == oal_host2net_short(0xe2ae) ||
        ether_type == oal_host2net_short(0xe2af)) {
        /* IPV6、IPX、AARP、PPOE、PAE报文不作地址替换 */
        return OSAL_SUCCESS;
    } else {
        /* 其他未知类型的地址替换 */
        return hmac_bridge_rx_unknow_addr_replace(hmac_vap, ether_header, pkt_len);
    }
}

static osal_u32 hmac_bridge_insert_unknow_mac(hmac_vap_bridge_stru *vap_bridge, osal_u16 protocol, osal_u8 *src_mac,
    osal_u8 hash_tmp)
{
    hmac_bridge_unknow_hash_stru *hash_unknow_new = OSAL_NULL;

    /* 遍历整个MAP未找到，重新申请内存并将节点插入到MAP表格中 */
    /* 查看表格记录数量，若超上限则不再新建 */
    if (vap_bridge->map_ipv4_num + vap_bridge->map_unknow_num > HMAC_BRIDGE_MAP_MAX_NUM) {
        oam_error_log1(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
            "{hmac_bridge_insert_ipv4_mac:: map num exceed max size: %d.}", HMAC_BRIDGE_MAP_MAX_NUM);
        return OSAL_SUCCESS;
    }

    hash_unknow_new = oal_memalloc(sizeof(hmac_bridge_unknow_hash_stru));
    if (hash_unknow_new == OSAL_NULL) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
            "{hmac_bridge_insert_ipv4_mac:: mem alloc null pointer.}");
        return OSAL_FAILURE;
    }
    hash_unknow_new->protocol = protocol;
    if (memcpy_s(hash_unknow_new->mac, WLAN_MAC_ADDR_LEN, src_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        oal_free(hash_unknow_new);
        return OSAL_FAILURE;
    }
    hash_unknow_new->last_active_timestamp = (osal_u32)oal_time_get_stamp_ms();

    /* 加入链表 */
    osal_spin_unlock(&vap_bridge->map_lock);
    osal_list_add_tail(&(hash_unknow_new->entry), &(vap_bridge->map_unknow_head[hash_tmp]));
    vap_bridge->map_unknow_num++;
    osal_spin_unlock(&vap_bridge->map_lock);

    oam_warning_log4(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
        "{hmac_bridge_insert_unknow_mac:: insert unknow map protocol:0x%x-mac:%x:%x:%x:**:**:**.}",
        protocol, src_mac[0], src_mac[1], src_mac[2]);    // 打印mac地址第0 1 2 位

    return OSAL_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_insert_unknow_mac
 功能描述  : 将未知协议报文的报文类型和MAC地址更新到MAP表格中
*****************************************************************************/
static osal_u32 hmac_bridge_update_unknow_mac(hmac_vap_bridge_stru *vap_bridge, osal_u16 protocol, osal_u8 *src_mac)
{
    osal_u8 hash_tmp;
    struct osal_list_head *dlist_entry = OSAL_NULL;
    hmac_bridge_unknow_hash_stru *hash_unknow = OSAL_NULL;

    if (vap_bridge == OSAL_NULL) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA, "{hmac_bridge_insert_unknow_mac:: null param.}");
        return OSAL_FAILURE;
    }

    hash_tmp = (osal_u8)hmac_bridge_cal_unknow_hash(protocol);
    osal_spin_lock(&vap_bridge->map_lock);
    osal_list_for_each(dlist_entry, &vap_bridge->map_unknow_head[hash_tmp]) {
        hash_unknow = osal_list_entry(dlist_entry, hmac_bridge_unknow_hash_stru, entry);
        if (hash_unknow->protocol != protocol) {
            continue;
        }
        /* key:protocol 找到元素 */
        if (memcmp(hash_unknow->mac, src_mac, WLAN_MAC_ADDR_LEN) != 0) {
            /* MAC地址不一致则刷新MAP表格 */
            if (memcpy_s(hash_unknow->mac, WLAN_MAC_ADDR_LEN, src_mac, WLAN_MAC_ADDR_LEN) != EOK) {
                osal_spin_unlock(&vap_bridge->map_lock);
                return OSAL_FAILURE;
            }
        }

        /* 刷新MAP表格对应记录的时间戳 */
        hash_unknow->last_active_timestamp = (osal_u32)oal_time_get_stamp_ms();
        osal_spin_unlock(&vap_bridge->map_lock);
        return OSAL_SUCCESS;
    }
    osal_spin_unlock(&vap_bridge->map_lock);

    return hmac_bridge_insert_unknow_mac(vap_bridge, protocol, src_mac, hash_tmp);
}

/*****************************************************************************
 函 数 名  : hmac_bridge_tx_unknow_addr_insert
 功能描述  : 未知协议报文地址转换: 保存一个协议类型与MAC地址到map表
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_bridge_tx_unknow_addr_insert(hmac_vap_stru *hmac_vap,
    mac_ether_header_stru *ether_header, osal_u32 pkt_len)
{
    /* bridge sta 自身的MAC地址 */
    osal_u8 *bsta_mac = hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id;
    /* 获取以太网源MAC和数据段 */
    osal_u8 *src_mac = ether_header->ether_shost;
    osal_u32 contig_len = sizeof(mac_ether_header_stru);
    if (pkt_len < contig_len) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
                       "{hmac_bridge_tx_unknow_addr_insert::The length of buf is invalid.}");
        return OSAL_FAILURE;
    }

    /* 将协议与MAC地址更新到MAC表格中 */
    hmac_bridge_update_unknow_mac(g_single_proxysta.vap_bridge, ether_header->ether_type, src_mac);
    /* 更新以太网源地址为实际的STA MAC地址 */
    if (memcpy_s(src_mac, WLAN_MAC_ADDR_LEN, bsta_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
                       "{hmac_bridge_tx_unknow_addr_insert::memcpy_s error.}");
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

static osal_u32 hmac_bridge_insert_ipv4_mac(hmac_vap_bridge_stru *vap_bridge, osal_u8 *ip_addr, osal_u8 *src_mac,
    osal_u8 hash_tmp)
{
    hmac_bridge_ipv4_hash_stru *hash_ipv4_new = OSAL_NULL;

    /* 没查找到KEY:IP对应的条目, 需要进行插入操作 */
    /* 查看表格记录数量，若超上限则不再新建 */
    if (vap_bridge->map_ipv4_num + vap_bridge->map_unknow_num > HMAC_BRIDGE_MAP_MAX_NUM) {
        oam_error_log1(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
            "{hmac_bridge_insert_ipv4_mac:: map num exceed max size: %d.}", HMAC_BRIDGE_MAP_MAX_NUM);
        return OSAL_SUCCESS;
    }

    hash_ipv4_new = oal_memalloc(sizeof(hmac_bridge_ipv4_hash_stru));
    if (hash_ipv4_new == OSAL_NULL) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
            "{hmac_bridge_insert_ipv4_mac:: mem alloc null pointer.}");
        return OSAL_FAILURE;
    }

    if (memcpy_s(hash_ipv4_new->ipv4, ETH_TARGET_IP_ADDR_LEN, ip_addr, ETH_TARGET_IP_ADDR_LEN) != EOK) {
        oal_free(hash_ipv4_new);
        return OSAL_FAILURE;
    }
    if (memcpy_s(hash_ipv4_new->mac, WLAN_MAC_ADDR_LEN, src_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        oal_free(hash_ipv4_new);
        return OSAL_FAILURE;
    }
    hash_ipv4_new->last_active_timestamp = (osal_u32)oal_time_get_stamp_ms();

    /* 加入链表 */
    osal_spin_lock(&vap_bridge->map_lock);
    osal_list_add_tail(&(hash_ipv4_new->entry), &(vap_bridge->map_ipv4_head[hash_tmp]));
    vap_bridge->map_ipv4_num++;
    osal_spin_unlock(&vap_bridge->map_lock);

    oam_warning_log3(0, OAM_SF_PROXYSTA, "{hmac_bridge_insert_ipv4_mac: insert ipv4 map ip[%d:%d:%d:**]}",
        ip_addr[0], ip_addr[1], ip_addr[2]);      // 打印IP地址第0 1 2位
    oam_warning_log4(0, OAM_SF_PROXYSTA, "{hmac_bridge_insert_ipv4_mac: insert ipv4 map mac[%x:%x:%x:%x:**:**]}",
        src_mac[0], src_mac[1], src_mac[2], src_mac[3]);       // 打印mac地址第0 1 2 3位

    return OSAL_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_insert_ipv4_mac
 功能描述  : 将IP地址和MAC地址更新到MAP表格中
*****************************************************************************/
static osal_u32 hmac_bridge_update_ipv4_mac(hmac_vap_bridge_stru *vap_bridge, osal_u8 *ip_addr, osal_u8 *src_mac)
{
    osal_u8 hash_tmp;
    struct osal_list_head *dlist_entry = OSAL_NULL;
    hmac_bridge_ipv4_hash_stru *hash_ipv4 = OSAL_NULL;

    if (vap_bridge == OSAL_NULL) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
            "{hmac_bridge_insert_ipv4_mac:: vap_bridge  null param.}");
        return OSAL_FAILURE;
    }

    /* 获取HASH桶值，HASH链表 */
    hash_tmp = (osal_u8)hmac_bridge_cal_ipv4_hash(ip_addr);
    osal_spin_lock(&vap_bridge->map_lock);
    osal_list_for_each(dlist_entry, &vap_bridge->map_ipv4_head[hash_tmp]) {
        hash_ipv4 = osal_list_entry(dlist_entry, hmac_bridge_ipv4_hash_stru, entry);
        if (memcmp(hash_ipv4->ipv4, ip_addr, ETH_TARGET_IP_ADDR_LEN) != 0) {
            continue;
        }

        if (memcmp(hash_ipv4->mac, src_mac, WLAN_MAC_ADDR_LEN) != 0) {
            /* MAC地址不一致则刷新MAP表格 */
            oam_info_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_insert_ipv4_mac:: ip same, but mac not same!}");
            if (memcpy_s(hash_ipv4->mac, WLAN_MAC_ADDR_LEN, src_mac, WLAN_MAC_ADDR_LEN) != EOK) {
                osal_spin_unlock(&vap_bridge->map_lock);
                return OSAL_FAILURE;
            }
        }

        /* 刷新MAP表格对应记录的时间戳 */
        hash_ipv4->last_active_timestamp = (osal_u32)oal_time_get_stamp_ms();
        osal_spin_unlock(&vap_bridge->map_lock);
        return OSAL_SUCCESS;
    }
    osal_spin_unlock(&vap_bridge->map_lock);

    return hmac_bridge_insert_ipv4_mac(vap_bridge, ip_addr, src_mac, hash_tmp);
}

/*****************************************************************************
 函 数 名  : hmac_bridge_tx_arp_addr_insert
 功能描述  : 上行arp包中的IP地址处理，更新ARP包的源MAC地址，并刷新MAP表格
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_bridge_tx_arp_addr_insert(hmac_vap_stru *hmac_vap, mac_ether_header_stru *ether_header,
    osal_u32 pkt_len)
{
    /****************************************************************************/
    /*                      ARP Frame Format                                    */
    /* ------------------------------------------------------------------------ */
    /* |以太网目的地址|以太网源地址|帧类型|硬件类型|协议类型|硬件地址长度|      */
    /* ------------------------------------------------------------------------ */
    /* | 6 (待替换)   |6           |2     |2       |2       |1           |      */
    /* ------------------------------------------------------------------------ */
    /* |协议地址长度|op|发送端以太网地址|发送端IP地址|目的以太网地址|目的IP地址 */
    /* ------------------------------------------------------------------------ */
    /* | 1          |2 |6               |4           |6 (待替换)    |4          */
    /* ------------------------------------------------------------------------ */
    /*                                                                          */
    /****************************************************************************/
    /* bridge sta 自身的MAC地址 */
    osal_u8 *bsta_mac = hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id;
    osal_u32 contig_len = sizeof(mac_ether_header_stru);

    /* 获取以太网源MAC和数据段 */
    osal_u8 *src_mac = ether_header->ether_shost;
    osal_u8 *eth_body = (osal_u8 *)(ether_header + 1);

    /* ARP包地址转换 */
    oal_eth_arphdr_stru *arp_package = (oal_eth_arphdr_stru *)eth_body;

    if (g_pkt_trace) {
        debug_arp_addr(hmac_vap, ether_header, arp_package);
    }

    contig_len += sizeof(oal_eth_arphdr_stru);
    if (pkt_len < contig_len) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
                       "{hmac_bridge_tx_arp_addr_insert::The length of buf is invalid.}");
        return OSAL_FAILURE;
    }
    /* 非IPV4的ARP报文不不处理 */
    if ((arp_package->ar_hln != ETHER_ADDR_LEN) || (oal_host2net_short(arp_package->ar_pro) != ETHER_TYPE_IP)) {
        oam_error_log2(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
                       "{hmac_bridge_tx_arp_addr_insert::arp hln:%d, arp pro: %d ,not to process.}",
                       arp_package->ar_hln, arp_package->ar_pro);
        return OSAL_SUCCESS;
    }

    /* 若源地址为repeater sta的mac地址，则直接返回，不作处理 */
    if (memcmp(bsta_mac, src_mac, WLAN_MAC_ADDR_LEN) == 0) {
        oam_info_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA,
            "{hmac_bridge_tx_arp_addr_insert :: proxysta_mac == src_mac, not need insert map table!}");
        return OSAL_SUCCESS;
    }

    /* 将IP地址和MAC地址更新到MAP表格中，插入MAP表格失败不影响处理结果 */
    if (hmac_ip_is_zero_etc(arp_package->ar_sip) == OSAL_TRUE) {
        /* ARP probe报文 */
        if (hmac_addr_is_zero_etc(arp_package->ar_tha) == OSAL_TRUE &&
            hmac_ip_is_zero_etc(arp_package->ar_tip) != OSAL_TRUE) {
            hmac_bridge_update_ipv4_mac(g_single_proxysta.vap_bridge, arp_package->ar_tip, src_mac);
        }
    } else {
        /* ARP announcement报文 */
        hmac_bridge_update_ipv4_mac(g_single_proxysta.vap_bridge, arp_package->ar_sip, src_mac);
    }

    /* 替换arp报文中的mac地址为proxysta的MAC地址 */
    if (memcpy_s(arp_package->ar_sha, WLAN_MAC_ADDR_LEN, bsta_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        return OSAL_FAILURE;
    }
    /* 替换报文以太网源地址 */
    if (memcpy_s(src_mac, WLAN_MAC_ADDR_LEN, bsta_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_tx_ip_addr_insert
 功能描述  : ip包地址学习，将IP报文中IP MAC地址提取出来 存放MAP，表格:
             1.DHCP报文的处理；
             2.其他IP类型报文的处理
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_bridge_tx_ip_addr_insert(hmac_vap_stru *hmac_vap,
    mac_ether_header_stru *ether_header, osal_u32 pkt_len)
{
    osal_u8 *bsta_mac = hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id; /* bridge sta 自身的MAC地址 */
    osal_u32 contig_len = sizeof(mac_ether_header_stru);
    osal_u8 *src_mac = ether_header->ether_shost;
    oal_ip_header_stru *ip_header = (oal_ip_header_stru *)(ether_header + 1);
    osal_u8 *ip_addr = OSAL_NULL;

    /*************************************************************************/
    /*                    IP头格式 (oal_ip_header_stru)                      */
    /* --------------------------------------------------------------------- */
    /* | 版本 | 报头长度 | 服务类型 | 总长度  |标识  |标志  |段偏移量 |      */
    /* --------------------------------------------------------------------- */
    /* | 4bits|  4bits   | 1        | 2       | 2    |3bits | 13bits  |      */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* | 生存期 | 协议        | 头部校验和| 源地址(SrcIp)|目的地址(DstIp)    */
    /* --------------------------------------------------------------------- */
    /* | 1      |  1 (17为UDP)| 2         | 4              | 4               */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    contig_len += sizeof(oal_ip_header_stru);
    if (pkt_len < contig_len) {
        wifi_printf("hmac_bridge_tx_ip_addr_insert:: pkg_len[%d] contig_len[%d]\n", pkt_len, contig_len);
        return OSAL_FAILURE;
    }

    if (ip_header->protocol == OAL_IPPROTO_UDP) {
        mac_udp_header_stru *udp_header = (mac_udp_header_stru *)(ip_header + 1);
        contig_len += sizeof(mac_udp_header_stru);
        if (pkt_len < contig_len) {
            wifi_printf("hmac_bridge_tx_ip_addr_insert:: pkg_len[%d] contig_len[%d]\n", pkt_len, contig_len);
            return OSAL_FAILURE;
        }
        /*************************************************************************/
        /*                      UDP 头 (oal_udp_header_stru)                     */
        /* --------------------------------------------------------------------- */
        /* |源端口号（SrcPort）|目的端口号（DstPort）| UDP长度    | UDP检验和  | */
        /* --------------------------------------------------------------------- */
        /* | 2                 | 2                   |2           | 2          | */
        /* --------------------------------------------------------------------- */
        /*                                                                       */
        /*************************************************************************/
        /* DHCP request UDP Client SP = 68 (bootpc), DP = 67 (bootps) */
        /* Repeater STA发送的DHCP REQUEST报文中要求DHCP SERVER以广播形式发送ACK报文 */
        /* 经由REPEATER发送的DHCP应答报文不会是单播报文 故不区分单播报文 */
        if (oal_host2net_short(udp_header->des_port) == DHCP_PORT_BOOTPS) {
            dhcp_message_stru *dhcp_package = (dhcp_message_stru *)(udp_header + 1);
            contig_len += (sizeof(dhcp_message_stru) - DHCP_OPTION_LEN);
            if (pkt_len < contig_len) {
                wifi_printf("hmac_bridge_tx_ip_addr_insert:: pkg_len[%d] contig_len[%d]\n", pkt_len, contig_len);
                return OSAL_FAILURE;
            }

            if (g_pkt_trace) {
                debug_dhcp_addr(hmac_vap, ether_header, dhcp_package);
            }

            /* DHCP报文仅需替换源MAC地址即可 */
            if (memcpy_s(src_mac, WLAN_MAC_ADDR_LEN, bsta_mac, WLAN_MAC_ADDR_LEN) != EOK) {
                return OSAL_FAILURE;
            }

            /* 客户端发过来的DHCP请求报文 更改标志字段要求服务器以广播形式发送ACK 如果是自己的DHCP则不更改 要求服务器以单播形式回复 */
            if (memcmp(bsta_mac, dhcp_package->chaddr, WLAN_MAC_ADDR_LEN) != 0) {
                hmac_bridge_dhcp_checksum(udp_header, dhcp_package);
            }
            return OSAL_SUCCESS;
        }
    }

    if (g_pkt_trace) {
        debug_icmp_addr(hmac_vap, ether_header, ip_header);
    }

    /* 本地发出去的报文不做地址替换 */
    if (memcmp(bsta_mac, src_mac, WLAN_MAC_ADDR_LEN) == 0) {
        return OSAL_SUCCESS;
    }

    ip_addr = (osal_u8 *)(&ip_header->saddr);
    /* 将IP地址和MAC地址更新到MAP表格中，插入MAP表格失败不影响处理结果 */
    hmac_bridge_update_ipv4_mac(g_single_proxysta.vap_bridge, ip_addr, src_mac);

    /* 替换源MAC地址 */
    if (memcpy_s(src_mac, WLAN_MAC_ADDR_LEN, bsta_mac, WLAN_MAC_ADDR_LEN) != EOK) {
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_tx_process
 功能描述  : 处理发送报文的源MAC地址替换
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_bridge_tx_process(const oal_netbuf_stru *netbuf,
    hmac_vap_stru *hmac_vap)
{
    mac_ether_header_stru *ether_header = OSAL_NULL;
    osal_u32 pkt_len;
    osal_u16 ether_type;
    hmac_vap_stru *mac_vap_temp = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;

    if (netbuf == OSAL_NULL || hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_PROXYSTA, "{hmac_bridge_tx_process::null param.}");
        return OSAL_FAILURE;
    }

    /* ap与sta不能同时存在的时候都不进入到repeater流程中，g_single_proxysta.vap_id记录的是sta的vap_id */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (mac_device_find_up_ap_etc(hmac_device, &mac_vap_temp) != OAL_SUCC) {
        return OSAL_SUCCESS;
    }
    if (!is_legacy_vap(mac_vap_temp)) {
        return OSAL_SUCCESS;
    }

    if (hmac_vap->vap_id != g_single_proxysta.vap_id) {
        return OSAL_SUCCESS;
    }

    ether_header = (mac_ether_header_stru *)oal_netbuf_header(netbuf);
    pkt_len = oal_netbuf_get_len((oal_netbuf_stru *)netbuf);
    ether_type = ether_header->ether_type;

    /* 根据报文类型作相应的处理 */
    /* IP包地址转换 */
    if (ether_type == oal_host2net_short(ETHER_TYPE_IP)) {
        return hmac_bridge_tx_ip_addr_insert(hmac_vap, ether_header, pkt_len);
    } else if (ether_type == oal_host2net_short(ETHER_TYPE_ARP)) {
        /* ARP 包地址转换 */
        return hmac_bridge_tx_arp_addr_insert(hmac_vap, ether_header, pkt_len);
    } else if (ether_type == oal_host2net_short(ETHER_TYPE_IPV6) ||
        ether_type == oal_host2net_short(ETHER_TYPE_IPX) ||
        ether_type == oal_host2net_short(ETHER_TYPE_AARP) ||
        ether_type == oal_host2net_short(ETHER_TYPE_PPP_DISC) ||
        ether_type == oal_host2net_short(ETHER_TYPE_PPP_SES) ||
        ether_type == oal_host2net_short(ETHER_TYPE_PAE) ||
        ether_type == oal_host2net_short(0xe2ae) ||
        ether_type == oal_host2net_short(0xe2af)) {
        /* IPV6、IPX、AARP、PPOE、PAE报文不作地址替换 */
        return OSAL_SUCCESS;
    } else {
        /* 其他未知类型的地址替换 */
        return hmac_bridge_tx_unknow_addr_insert(hmac_vap, ether_header, pkt_len);
    }
}

/*****************************************************************************
 函 数 名  : hmac_bridge_map_aging_timer
 功能描述  : bridge 的路由表老化定时器到期处理函数 定时扫描MAP表发现有超时的表数据进行删除操作
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_bridge_map_aging_timer(osal_void *param)
{
    hmac_vap_bridge_stru   *vap_bridge = (hmac_vap_bridge_stru *)param;
    osal_u32 present_time = (osal_u32)oal_time_get_stamp_ms();
    osal_u32 i;
    struct osal_list_head *dlist_entry = OSAL_NULL;
    struct osal_list_head *temp = OSAL_NULL;
    hmac_bridge_ipv4_hash_stru *hash_ipv4 = OSAL_NULL;
    hmac_bridge_unknow_hash_stru *hash_unknow = OSAL_NULL;
    osal_u32 map_idle_time = 0;

    if (vap_bridge == OSAL_NULL) {
        return OSAL_FALSE;
    }

    osal_spin_lock(&vap_bridge->map_lock);

    hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_ipv4_head) {
        hash_ipv4 = osal_list_entry(dlist_entry, hmac_bridge_ipv4_hash_stru, entry);
        map_idle_time = (osal_u32)osal_get_runtime(hash_ipv4->last_active_timestamp, present_time);
        if (map_idle_time > HMAC_BRIDGE_MAP_AGING_TIME) {
            oam_info_log3(0, OAM_SF_PROXYSTA, "{start delete ipv4 map ip[%d:%d:%d:**]}",
                hash_ipv4->ipv4[0], hash_ipv4->ipv4[1], hash_ipv4->ipv4[2]);   // 打印IP地址第0 1 2位
            osal_dlist_delete_entry(dlist_entry);
            oal_free(hash_ipv4);
            vap_bridge->map_ipv4_num--;
        }
    }

    hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_unknow_head) {
        hash_unknow = osal_list_entry(dlist_entry, hmac_bridge_unknow_hash_stru, entry);
        map_idle_time = (osal_u32)osal_get_runtime(hash_unknow->last_active_timestamp, present_time);
        if (map_idle_time > HMAC_BRIDGE_MAP_AGING_TIME) {
            osal_dlist_delete_entry(dlist_entry);
            oal_free(hash_unknow);
            vap_bridge->map_unknow_num--;
        }
    }

    osal_spin_unlock(&vap_bridge->map_lock);
    return OSAL_SUCCESS;
}

OSAL_STATIC osal_bool hmac_single_proxysta_vap_start(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_vap_bridge_stru *vap_bridge = OSAL_NULL;
    osal_u8 i;
    hmac_vap_stru *mac_vap_temp = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    /* 只有sta vap都启动的时候，才会尝试去初始化proxysta，并且记录sta_vap的vapid */
    if (is_legacy_sta(hmac_vap)) {
        if ((mac_device_find_up_ap_etc(hmac_device, &mac_vap_temp) != OAL_SUCC) || (!is_legacy_vap(mac_vap_temp))) {
            return OSAL_TRUE;
        }
        g_single_proxysta.vap_id = hmac_vap->vap_id;
    } else if (is_legacy_ap(hmac_vap)) {
        if ((mac_device_find_up_sta_etc(hmac_device, &mac_vap_temp)  != OAL_SUCC) || (!is_legacy_vap(mac_vap_temp))) {
            return OSAL_TRUE;
        }
        g_single_proxysta.vap_id = mac_vap_temp->vap_id;
    } else {
        return OSAL_TRUE;
    }

    if (g_single_proxysta.vap_bridge != OSAL_NULL) {    /* 防止重复创建 */
        return OSAL_TRUE;
    }

    vap_bridge = (hmac_vap_bridge_stru *)oal_memalloc(sizeof(hmac_vap_bridge_stru));
    if (vap_bridge == OSAL_NULL) {
        oam_error_log0(g_single_proxysta.vap_id, OAM_SF_PROXYSTA, "hmac_single_proxysta_vap_add malloc error");
        return OSAL_FALSE;   // single proxy失败,但需要继续其它
    }
    memset_s(vap_bridge, sizeof(hmac_vap_bridge_stru), 0, sizeof(hmac_vap_bridge_stru));

    osal_spin_lock_init(&vap_bridge->map_lock);

    /* 初始化链表 */
    for (i = 0; i < osal_array_size(vap_bridge->map_ipv4_head); i++) {
        OSAL_INIT_LIST_HEAD(&vap_bridge->map_ipv4_head[i]);
    }
    vap_bridge->map_ipv4_num = 0;
    for (i = 0; i < osal_array_size(vap_bridge->map_unknow_head); i++) {
        OSAL_INIT_LIST_HEAD(&vap_bridge->map_unknow_head[i]);
    }
    vap_bridge->map_unknow_num = 0;

    g_single_proxysta.vap_bridge = vap_bridge;

    if (g_single_proxysta.st_bridge_map_timer.is_registerd == OSAL_FALSE) {
        frw_create_timer_entry(&(g_single_proxysta.st_bridge_map_timer), hmac_bridge_map_aging_timer,
            HMAC_BRIDGE_MAP_AGING_TRIGGER_TIME, g_single_proxysta.vap_bridge, OAL_TRUE);
    }

    return OSAL_TRUE;
}

OSAL_STATIC osal_void hmac_single_proxysta_flush_list(hmac_vap_bridge_stru *vap_bridge)
{
    struct osal_list_head *dlist_entry = OSAL_NULL;
    struct osal_list_head *temp = OSAL_NULL;
    hmac_bridge_ipv4_hash_stru *hash_ipv4 = OSAL_NULL;
    hmac_bridge_unknow_hash_stru *hash_unknow = OSAL_NULL;
    osal_u8 i;

    hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_ipv4_head) {
        hash_ipv4 = osal_list_entry(dlist_entry, hmac_bridge_ipv4_hash_stru, entry);
        osal_dlist_delete_entry(dlist_entry);
        oal_free(hash_ipv4);
    }
    vap_bridge->map_ipv4_num = 0;

    hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_unknow_head) {
        hash_unknow = osal_list_entry(dlist_entry, hmac_bridge_unknow_hash_stru, entry);
        osal_dlist_delete_entry(dlist_entry);
        oal_free(hash_unknow);
    }
    vap_bridge->map_unknow_num = 0;
    osal_spin_lock_destroy(&vap_bridge->map_lock);
    oal_free(vap_bridge);
}

OSAL_STATIC osal_bool hmac_single_proxysta_vap_down(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_TRUE;
    }

#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
    if (is_legacy_ap(hmac_vap)) {
        g_bridge_ctrl.repeat_ap = OSAL_NULL;
    } else if (is_legacy_sta(hmac_vap)) {
        g_bridge_ctrl.repeat_sta = OSAL_NULL;
    }
#endif

    /* 仅bridge sta有map表需要释放内存, vap 是非proxy sta, 直接返回 */
    if (hmac_vap->vap_id != g_single_proxysta.vap_id) {
        return OSAL_TRUE;
    }

    /* 关闭超时定时器 */
    if (g_single_proxysta.st_bridge_map_timer.is_registerd) {
        frw_destroy_timer_entry(&(g_single_proxysta.st_bridge_map_timer));
    }

    /* 删除链表并且释放内存 */
    hmac_single_proxysta_flush_list(g_single_proxysta.vap_bridge);
    g_single_proxysta.vap_bridge = OSAL_NULL;
    g_single_proxysta.vap_id = 0;

    return OSAL_TRUE;
}

static osal_u32 hmac_bridge_map_debug(osal_void)
{
    hmac_vap_bridge_stru   *vap_bridge = g_single_proxysta.vap_bridge;
    osal_u32 i;
    struct osal_list_head *dlist_entry = OSAL_NULL;
    struct osal_list_head *temp = OSAL_NULL;
    hmac_bridge_ipv4_hash_stru *hash_ipv4 = OSAL_NULL;
    hmac_bridge_unknow_hash_stru *hash_unknow = OSAL_NULL;

    if (vap_bridge == OSAL_NULL) {
        return OSAL_FALSE;
    }

    osal_spin_lock(&vap_bridge->map_lock);

    hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_ipv4_head) {
        hash_ipv4 = osal_list_entry(dlist_entry, hmac_bridge_ipv4_hash_stru, entry);
        wifi_printf("\r\n[%d][%d.%d.%d.**]=[%x:%x:%x:%x:**:**]\r\n", hash_ipv4->last_active_timestamp,
            hash_ipv4->ipv4[0], hash_ipv4->ipv4[1], hash_ipv4->ipv4[2], /* 打印ip地址第0 1 2位 */
            hash_ipv4->mac[0], hash_ipv4->mac[1], hash_ipv4->mac[2], hash_ipv4->mac[3]); /* 打印mac地址第0 1 2 3位 */
    }

    hmac_bridge_loop_all_node_safe(i, dlist_entry, temp, vap_bridge->map_unknow_head) {
        hash_unknow = osal_list_entry(dlist_entry, hmac_bridge_unknow_hash_stru, entry);
        wifi_printf("\r\n[%d][%d]=[%x:%x:%x:%x:**:**]\r\n", hash_unknow->last_active_timestamp, hash_unknow->protocol,
            hash_unknow->mac[0], hash_unknow->mac[1],
            hash_unknow->mac[2], hash_unknow->mac[3]); /* 打印mac地址第2 3位 */
    }

    osal_spin_unlock(&vap_bridge->map_lock);
    return OSAL_SUCCESS;
}

#define DEBUG_TRACE 0x01
#define DEBUG_TABLE 0x02

OSAL_STATIC osal_s32 hmac_single_proxysta_debug(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 debug_type = 0;

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    debug_type = *(msg->data);
    g_pkt_trace = debug_type & DEBUG_TRACE;
    if ((debug_type & DEBUG_TABLE) != 0) {
        hmac_bridge_map_debug();
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
hmac_netbuf_hook_stru g_bridge_rx_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_H2O,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_bridge_rx_data_process,
};

OSAL_STATIC osal_void hmac_bridge_control_addbr(const osal_char *param)
{
    hmac_vap_stru *mac_vap_temp = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    unref_param(param);

    if (mac_device_find_up_ap_etc(hmac_device, &mac_vap_temp) != OAL_SUCC) {
        return;
    }
    g_bridge_ctrl.repeat_ap = mac_vap_temp;
    if (mac_device_find_up_sta_etc(hmac_device, &mac_vap_temp) != OAL_SUCC) {
        return;
    }
    g_bridge_ctrl.repeat_sta = mac_vap_temp;
    g_bridge_ctrl.br_switch = OSAL_TRUE;
    /* 创建桥 向转发流程注册hook */
    (osal_void)hmac_register_netbuf_hook(&g_bridge_rx_hook);
    return;
}

OSAL_STATIC osal_void hmac_bridge_control_delbr(const osal_char *param)
{
    unref_param(param);

    g_bridge_ctrl.br_switch = OSAL_FALSE;
    /* 销毁桥 转发流程去注册hook */
    (osal_void)hmac_unregister_netbuf_hook(&g_bridge_rx_hook);
    g_bridge_ctrl.repeat_ap = OSAL_NULL;
    g_bridge_ctrl.repeat_sta = OSAL_NULL;
    return;
}

OSAL_STATIC osal_void hmac_bridge_control_show_bridge(const osal_char *param)
{
    unref_param(param);

    if (g_bridge_ctrl.br_switch != OSAL_TRUE) {
        wifi_printf("Bridge is not enabled\r\n");
        return;
    }

    if (g_bridge_ctrl.repeat_sta == OSAL_NULL) {
        wifi_printf("Bridge br0 sta is NULL;");
    } else {
        wifi_printf("Bridge br0 sta vap_id [%d];", g_bridge_ctrl.repeat_sta->vap_id);
    }

    if (g_bridge_ctrl.repeat_ap == OSAL_NULL) {
        wifi_printf("ap is NULL\r\n");
    } else {
        wifi_printf("ap vap_id [%d]\r\n", g_bridge_ctrl.repeat_ap->vap_id);
    }
    return;
}

OAL_STATIC osal_s32 hmac_config_bridge_control(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u8 i;
    osal_s8 arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 ret;
    static const hmac_bridge_cmd_stru bridge_cmd[BRIDGE_CMD_MAX_NUM] = {
        {"addbr", hmac_bridge_control_addbr},
        {"delbr", hmac_bridge_control_delbr},
        {"show",  hmac_bridge_control_show_bridge},
    };

    ret = (osal_u32)hmac_ccpriv_get_one_arg(&param, arg, OAL_SIZEOF(arg));
    if (ret != OAL_SUCC) {
        return ret;
    }

    unref_param(hmac_vap);
    for (i = 0; i < BRIDGE_CMD_MAX_NUM; i++) {
        if (osal_strcmp((osal_char *)arg, bridge_cmd[i].cmd_name) == 0) {
            bridge_cmd[i].func(arg);
            return OAL_SUCC;
        }
    }
    /* 命令字匹配失败 */
    return OAL_FAIL;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_bridge_trans_switch(const hmac_vap_stru *hmac_vap)
{
    if (g_bridge_ctrl.br_switch != OSAL_TRUE || g_bridge_ctrl.repeat_ap == OSAL_NULL ||
        g_bridge_ctrl.repeat_sta == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_id != g_bridge_ctrl.repeat_ap->vap_id && hmac_vap->vap_id != g_bridge_ctrl.repeat_sta->vap_id) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_void hmac_bridge_rx_copy_data(oal_netbuf_stru *netbuf,
    const hmac_vap_stru *hmac_vap)
{
    oal_netbuf_stru *netbuf_copy = OSAL_NULL;
    hmac_vap_stru *hmac_vap_dest;

    netbuf_copy = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, OAL_NETBUF_LEN(netbuf), OAL_NETBUF_PRIORITY_HIGH);
    if (netbuf_copy == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_bridge_rx_copy_data::malloc netbuf fail.");
        return;
    }
    (void)memcpy_s(OAL_NETBUF_CB(netbuf_copy), OAL_NETBUF_CB_SIZE(), OAL_NETBUF_CB(netbuf), OAL_NETBUF_CB_SIZE());
    (void)memcpy_s(oal_netbuf_data(netbuf_copy), OAL_NETBUF_LEN(netbuf), oal_netbuf_data(netbuf),
        OAL_NETBUF_LEN(netbuf));
    oal_netbuf_put(netbuf_copy, oal_netbuf_get_len(netbuf));

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_vap_dest = g_bridge_ctrl.repeat_sta;
    } else {
        hmac_vap_dest = g_bridge_ctrl.repeat_ap;
    }

    frw_host_post_data(FRW_NETBUF_W2H_DATA_FRAME, hmac_vap_dest->vap_id, netbuf_copy);

    return;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_bridge_rx_data_process(oal_netbuf_stru **netbuf,
    hmac_vap_stru *hmac_vap)
{
    mac_ether_header_stru *ether_hdr = OAL_PTR_NULL;
    osal_u32 ret;

    if (hmac_bridge_trans_switch(hmac_vap) != OSAL_TRUE) {
        return OAL_CONTINUE;
    }
    /* 将skb的data指针指向以太网的帧头 */
    /* 由于前面pull了14字节，这个地方要push回去 */
    oal_netbuf_push(*netbuf, ETHER_HDR_LEN);
    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(*netbuf);
    if (ether_is_multicast(ether_hdr->ether_dhost) == OSAL_TRUE) {
        /* 组播数据拷贝 */
        hmac_bridge_rx_copy_data(*netbuf, hmac_vap);
        ret = OAL_CONTINUE;
    } else if (memcmp(ether_hdr->ether_dhost, mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN) != 0) {
        /* 其他报文转发repeater另一个端口 */
        hmac_bridge_rx_copy_data(*netbuf, hmac_vap);
        ret = OAL_SUCC;
    } else {
        /* 发给本机单播报文上报 */
        ret = OAL_CONTINUE;
    }
    /* 恢复原报文指针 */
    oal_netbuf_pull(*netbuf, ETHER_HDR_LEN);
    return ret;
}
#endif

osal_u32 hmac_single_proxysta_init(osal_void)
{
    /* 注册vap监听 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_START_VAP, hmac_single_proxysta_vap_start);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_single_proxysta_vap_down);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_single_proxysta_user_del);

    /* 注册配置面消息 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SINGLE_PROXYSTA, hmac_single_proxysta_debug);

    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_REPEATER_BRIDGE_TX_PROCESS, hmac_bridge_tx_process);
    hmac_feature_hook_register(HMAC_FHOOK_REPEATER_BRIDGE_RX_PROCESS, hmac_bridge_rx_process);

#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
    hmac_ccpriv_register((const osal_s8 *)"brctl", hmac_config_bridge_control);
#endif
    return OSAL_SUCCESS;
}

osal_void hmac_single_proxysta_deinit(osal_void)
{
    /* 注册vap监听 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_START_VAP, hmac_single_proxysta_vap_start);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_single_proxysta_vap_down);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_single_proxysta_user_del);

    /* 注册配置面消息 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SINGLE_PROXYSTA);

    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_REPEATER_BRIDGE_TX_PROCESS);
    hmac_feature_hook_unregister(HMAC_FHOOK_REPEATER_BRIDGE_RX_PROCESS);

#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
    hmac_ccpriv_unregister((const osal_s8 *)"brctl");
#endif
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
