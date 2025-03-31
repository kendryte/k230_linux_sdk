/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_proxy_arp.c
 * 生成日期   : 2014年7月29日
 * 功能描述   : PROXY ARP
 */


#ifdef _PRE_WLAN_FEATURE_PROXY_ARP

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_proxy_arp.h"
#include "oal_netbuf_ext.h"
#include "wlan_spec.h"
#include "hmac_vap.h"
#include "hmac_ext_if.h"
#include "hmac_rx_data.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_PROXY_ARP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_proxy_get_mac_by_ipv4
 功能描述  : 通过ipv4的地址获取mac
 输入参数  : hmac_vap: hmac_vap指针
             ipv4:ipv4地址
 输出参数  : mac:mac地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_get_mac_by_ipv4(hmac_vap_stru *hmac_vap, osal_u32 ipv4, osal_u8 *mac)
{
    osal_u32                  user_hash_value;
    hmac_proxy_ipv4_hash_stru  *hash  = OAL_PTR_NULL;
    struct osal_list_head        *entry = OAL_PTR_NULL;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (mac == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_PROXYARP, "{hmac_vap_find_user_by_macaddr_etc::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    user_hash_value = hmac_proxy_ipv4_hash(ipv4);

    osal_list_for_each(entry, &(hmac_vap->hmac_vap_proxyarp->ipv4_head[user_hash_value])) {
        hash = (hmac_proxy_ipv4_hash_stru *)entry;
        if (hash->ipv4 != ipv4) {
            continue;
        }

        if (memcpy_s(mac, WLAN_MAC_ADDR_LEN, hash->mac, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_proxy_get_mac_by_ipv4::memcpy_s error}");
        }

        return OAL_SUCC;
    }

    return OAL_FAIL;
}


/*****************************************************************************
 函 数 名  : hmac_proxy_remove_ipv4
 功能描述  : 将ipv4的地址从hash表中删除
 输入参数  : hmac_vap: hmac_vap指针
             ipv4:ipv4地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_remove_ipv4(hmac_vap_stru *hmac_vap, osal_u32 ipv4)
{
    osal_u32                  user_hash_value;
    struct osal_list_head        *entry     = OAL_PTR_NULL;
    hmac_proxy_ipv4_hash_stru  *hash      = OAL_PTR_NULL;
    struct osal_list_head        *dlist_tmp = OAL_PTR_NULL;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_remove_ipv4::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->hmac_vap_proxyarp->ipv4_num == 0) {
        return OAL_SUCC;
    }

    user_hash_value = hmac_proxy_ipv4_hash(ipv4);

    osal_list_for_each_safe(entry, dlist_tmp,
        &(hmac_vap->hmac_vap_proxyarp->ipv4_head[user_hash_value])) {
        hash = (hmac_proxy_ipv4_hash_stru *)entry;

        if (hash->ipv4 != ipv4) {
            continue;
        }
        osal_list_del(entry);
        oal_mem_free(hash, OAL_TRUE);
        hmac_vap->hmac_vap_proxyarp->ipv4_num--;
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_proxy_add_ipv4
 功能描述  : 将ipv4的地址加入hash表，并记录相应的mac地址
 输入参数  : hmac_vap: hmac_vap指针
             ipv4:ipv4地址
             mac:mac地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_add_ipv4(hmac_vap_stru *hmac_vap, osal_u32 ipv4, osal_u8 *mac)
{
    osal_u32                  user_hash_value;
    hmac_proxy_ipv4_hash_stru  *hash = OAL_PTR_NULL;
    oal_err_code_enum_uint32    exist;
    osal_u8                   mac[OAL_MAC_ADDR_LEN];

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (mac == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_add_ipv4::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 查询是否存在 */
    exist = hmac_proxy_get_mac_by_ipv4(hmac_vap, ipv4, mac);
    if (OAL_SUCC == exist) {
        if (!osal_memcmp(mac, mac, OAL_MAC_ADDR_LEN)) {
            return OAL_SUCC;
        }
        /* 如果来自不同的mac，则将前面记录的结点删除，后面流程将新的结点加入 */
        hmac_proxy_remove_ipv4(hmac_vap, ipv4);
    }

    if (hmac_vap->hmac_vap_proxyarp->ipv4_num >= MAC_VAP_USER_HASH_MAX_VALUE) {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    /* 申请内存 */
    hash = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(hmac_proxy_ipv4_hash_stru), OAL_TRUE);
    if (OAL_PTR_NULL == hash) {
        oam_error_log0(0, OAM_SF_PROXYARP, "hmac_proxy_add_ipv4 err! melloc err!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写 */
    hash->ipv4 = ipv4;
    if (memcpy_s(hash->mac, sizeof(hash->mac), mac, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_proxy_add_ipv4::memcpy_s error}");
    }

    /* 加入hash表 */
    user_hash_value = hmac_proxy_ipv4_hash(ipv4);
    osal_list_add(&(hash->entry), &(hmac_vap->hmac_vap_proxyarp->ipv4_head[user_hash_value]));
    hmac_vap->hmac_vap_proxyarp->ipv4_num++;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_get_mac_by_ipv6
 功能描述  : 通过ipv6的地址获取mac
 输入参数  : hmac_vap: hmac_vap指针
             ipv6:ipv6地址
 输出参数  : mac:mac地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_get_mac_by_ipv6(hmac_vap_stru *hmac_vap,
    oal_in6_addr *ipv6, osal_u8 *mac)
{
    osal_u32                  user_hash_value;
    hmac_proxy_ipv6_hash_stru  *hash  = OAL_PTR_NULL;
    struct osal_list_head        *entry = OAL_PTR_NULL;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (ipv6 == OAL_PTR_NULL) || (mac == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_get_mac_by_ipv6::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    user_hash_value = hmac_proxy_ipv6_hash(ipv6);

    osal_list_for_each(entry, &(hmac_vap->hmac_vap_proxyarp->ipv6_head[user_hash_value])) {
        hash = (hmac_proxy_ipv6_hash_stru *)entry;

        if (osal_memcmp(ipv6, &hash->ipv6, OAL_SIZEOF(oal_in6_addr))) {
            continue;
        }
        if (memcpy_s(mac, WLAN_MAC_ADDR_LEN, hash->mac, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_proxy_get_mac_by_ipv6::memcpy_s error}");
        }
        return OAL_SUCC;
    }

    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_remove_ipv6
 功能描述  : 将ipv6的地址从hash表中删除
 输入参数  : hmac_vap: hmac_vap指针
             ipv6:ipv6地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_remove_ipv6(hmac_vap_stru *hmac_vap, oal_in6_addr *ipv6)
{
    osal_u32                  user_hash_value;
    struct osal_list_head        *entry     = OAL_PTR_NULL;
    hmac_proxy_ipv6_hash_stru  *hash      = OAL_PTR_NULL;
    struct osal_list_head        *dlist_tmp = OAL_PTR_NULL;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_remove_ipv4::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->hmac_vap_proxyarp->ipv6_num == 0) {
        return OAL_SUCC;
    }

    user_hash_value = hmac_proxy_ipv6_hash(ipv6);

    osal_list_for_each_safe(entry, dlist_tmp,
        &(hmac_vap->hmac_vap_proxyarp->ipv6_head[user_hash_value])) {
        hash = (hmac_proxy_ipv6_hash_stru *)entry;

        if (osal_memcmp(ipv6, &hash->ipv6, OAL_SIZEOF(oal_in6_addr))) {
            continue;
        }
        osal_list_del(entry);
        oal_mem_free(hash, OAL_TRUE);
        hmac_vap->hmac_vap_proxyarp->ipv6_num--;
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_proxy_add_ipv6
 功能描述  : 将ipv6的地址加入hash表，并记录相应的mac地址
 输入参数  : hmac_vap: hmac_vap指针
             ipv6:ipv6地址
             mac:mac地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_add_ipv6(hmac_vap_stru *hmac_vap, oal_in6_addr *ipv6, osal_u8 *mac)
{
    osal_u32                  user_hash_value;
    hmac_proxy_ipv6_hash_stru  *hash = OAL_PTR_NULL;
    oal_err_code_enum_uint32    exist;
    osal_u8                   mac[OAL_MAC_ADDR_LEN];
    osal_u8                   ipv6_zero_addr[16] = {0};

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (mac == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_add_ipv6::param null.}");

        return OAL_ERR_CODE_PROXY_ND_LEARN_USR_FAIL_INCOMP;
    }

    if (!osal_memcmp(ipv6_zero_addr, ipv6, OAL_SIZEOF(oal_in6_addr))) {
        return OAL_ERR_CODE_PROXY_ND_LEARN_USR_SUCC_COMP;
    }

    /* 查询是否存在,如果存在，看是否来自同一个mac */
    exist = hmac_proxy_get_mac_by_ipv6(hmac_vap, ipv6, mac);
    if (exist == OAL_SUCC) {
        if (!osal_memcmp(mac, mac, OAL_MAC_ADDR_LEN)) {
            return OAL_ERR_CODE_PROXY_ND_LEARN_USR_SUCC_COMP;
        }

        /* 如果来自不同的mac，则将前面记录的结点删除，后面流程将新的结点加入 */
        hmac_proxy_remove_ipv6(hmac_vap, ipv6);
    }

    if (hmac_vap->hmac_vap_proxyarp->ipv6_num >= MAC_VAP_USER_HASH_MAX_VALUE) {
        return OAL_ERR_CODE_PROXY_ND_LEARN_USR_FAIL_INCOMP;
    }

    /* 申请 */
    hash = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(hmac_proxy_ipv6_hash_stru), OAL_TRUE);
    if (OAL_PTR_NULL == hash) {
        return OAL_ERR_CODE_PROXY_ND_LEARN_USR_FAIL_INCOMP;
    }

    /* 填写 */
    hash->ipv6 = *ipv6;
    if (memcpy_s(hash->mac, sizeof(hash->mac), mac, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_proxy_add_ipv6::memcpy_s error}");
    }

    /* 加入hash表 */
    user_hash_value  = 0;
    user_hash_value = hmac_proxy_ipv6_hash(&hash->ipv6);
    osal_list_add(&(hash->entry), &hmac_vap->hmac_vap_proxyarp->ipv6_head[user_hash_value]);
    hmac_vap->hmac_vap_proxyarp->ipv6_num++;
    return OAL_ERR_CODE_PROXY_ND_LEARN_USR_SUCC_COMP;
}

osal_void hmac_proxy_remove_mac_ipv6(hmac_vap_stru *hmac_vap, osal_u8 *mac)
{
    struct osal_list_head        *entry     = OAL_PTR_NULL;
    hmac_proxy_ipv6_hash_stru  *hash_ipv6 = OAL_PTR_NULL;
    struct osal_list_head      *dlist_tmp = OAL_PTR_NULL;
    osal_u32                  loop;

    for (loop = 0; loop < MAC_VAP_USER_HASH_MAX_VALUE; loop++) {
        osal_list_for_each_safe(entry, dlist_tmp,
            &(hmac_vap->hmac_vap_proxyarp->ipv6_head[loop])) {
            hash_ipv6 = (hmac_proxy_ipv6_hash_stru *)entry;
            if (osal_memcmp(mac, &hash_ipv6->mac, OAL_MAC_ADDR_LEN)) {
                continue;
            }

            osal_list_del(entry);
            oal_mem_free(hash_ipv6, OAL_TRUE);
            hmac_vap->hmac_vap_proxyarp->ipv6_num--;
            break;
        }
    }
}

osal_void hmac_proxy_remove_mac_ipv4(hmac_vap_stru *hmac_vap, osal_u8 *mac)
{
    struct osal_list_head        *entry     = OAL_PTR_NULL;
    hmac_proxy_ipv4_hash_stru  *hash_ipv4 = OAL_PTR_NULL;
    struct osal_list_head      *dlist_tmp = OAL_PTR_NULL;
    osal_u32                  loop;

    for (loop = 0; loop < MAC_VAP_USER_HASH_MAX_VALUE; loop++) {
        osal_list_for_each_safe(entry, dlist_tmp,
            &(hmac_vap->hmac_vap_proxyarp->ipv4_head[loop])) {
            hash_ipv4 = (hmac_proxy_ipv4_hash_stru *)entry;
            if (osal_memcmp(mac, &hash_ipv4->mac, OAL_MAC_ADDR_LEN)) {
                continue;
            }

            osal_list_del(entry);
            oal_mem_free(hash_ipv4, OAL_TRUE);
            hmac_vap->hmac_vap_proxyarp->ipv4_num--;
            break;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_proxy_remove_mac
 功能描述  : 通过mac地址将对应ipv4和ipv6 结点删除
 输入参数  : hmac_vap: mac_vap指针
             mac:mac地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_remove_mac(hmac_vap_stru *hmac_vap, osal_u8 *mac)
{
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_remove_mac::vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_LIKELY(hmac_vap->hmac_vap_proxyarp == OAL_PTR_NULL)) {
        oam_info_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_remove_mac::proxyarp null.}");
        return OAL_SUCC;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OAL_SUCC;
    }

    if (hmac_vap->hmac_vap_proxyarp->ipv6_num > 0) {
        hmac_proxy_remove_mac_ipv6(hmac_vap, mac);
    }

    if (hmac_vap->hmac_vap_proxyarp->ipv4_num > 0) {
        hmac_proxy_remove_mac_ipv4(hmac_vap, mac);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_arp_rcv_req
 功能描述  : 处理接收到的arp req
 输入参数  : hmac_vap: hmac_vap指针
             arp_hdr:skb指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_arp_rcv_req(hmac_vap_stru *hmac_vap, oal_net_device_stru *dev,
    oal_eth_arphdr_stru *arp_hdr)
{
    osal_u32 src_ipv4, tgt_ipv4, ret;
    osal_u8 src_mac[ETHER_ADDR_LEN], tgt_mac[ETHER_ADDR_LEN];
    osal_u16 usr_idx;
    oal_err_code_enum_uint32 rst;
    oal_netbuf_stru *netbuff = OAL_PTR_NULL;
    mac_ether_header_stru eth_hdr;

    (osal_void)memcpy_s(src_mac, ETHER_ADDR_LEN, arp_hdr->ar_sha, ETHER_ADDR_LEN);
    (osal_void)memcpy_s(&src_ipv4, ETH_SENDER_IP_ADDR_LEN, arp_hdr->ar_sip, ETH_SENDER_IP_ADDR_LEN);
    (osal_void)memcpy_s(&tgt_ipv4, ETH_TARGET_IP_ADDR_LEN, arp_hdr->ar_tip, ETH_TARGET_IP_ADDR_LEN);

    /* GARP,进行地址学习 */
    if (src_ipv4 == tgt_ipv4) {
        if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, src_mac, &usr_idx) != OAL_SUCC) {
            return OAL_ERR_CODE_PROXY_ARP_LEARN_USR_NOTEXIST_COMP;
        }
        hmac_proxy_add_ipv4(hmac_vap, src_ipv4, src_mac);
        return OAL_ERR_CODE_PROXY_ARP_LEARN_USR_COMP;
    }

    /* 将目的地址不是本BSS的ARP REQ过滤掉 */
    if (hmac_proxy_get_mac_by_ipv4(hmac_vap, tgt_ipv4, tgt_mac) != OAL_SUCC) {
        return OAL_ERR_CODE_PROXY_ARP_FINDUSR_ERR_COMP;
    }

    netbuff = oal_arp_create(OAL_ARPOP_REPLY, ETHER_TYPE_ARP, src_ipv4, dev, tgt_ipv4, src_mac, tgt_mac, src_mac);
    if (netbuff == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PROXY_ARP_CREATE_FAIL_COMP;
    }
    netbuff->dev = dev;

    /* 记录一下，用来做后面的打印 */
    eth_hdr = *(mac_ether_header_stru *)oal_netbuf_data(netbuff);

    /* 找到用户，使用空口发送 */
    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, src_mac, &usr_idx) == OAL_SUCC) {
        /* data指针指向的是以太头 */
        ret = hmac_tx_lan_to_wlan_etc(hmac_vap, netbuff);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_PROXYARP, "{hmac_proxy_arp_rcv_req 2wlan Err[%u]!!}", ret);
            oal_netbuf_free(netbuff);
            netbuff = OAL_PTR_NULL;
        }

        rst = OAL_ERR_CODE_PROXY_ARP_REPLY2BSS_COMP;
    } else {
        /* 送给网桥，需要pull一下，让data指针指向arp头 */
        OAL_NETBUF_PROTOCOL(netbuff) = oal_eth_type_trans(netbuff, dev);
        /* 交给网桥 */
        ret = (osal_u32)oal_netif_rx(netbuff);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_PROXYARP, "vap_id[%d] {hmac_proxy_arp_rcv_req 2br Err[%u]!!}",
                hmac_vap->vap_id, ret);
        }
#ifdef _PRE_WLAN_DFT_STAT
        if (ret == OAL_SUCC) {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_SEND_LAN_SUCC);
        } else {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_SEND_LAN_FAIL);
        }
#endif
        rst = OAL_ERR_CODE_PROXY_ARP_REPLY2ETH_COMP;
    }
    oam_info_log4(0, OAM_SF_PROXYARP, "hmac_proxy_arp_rcv_req! s:%x->d:%x type %x, result %u!",
        eth_hdr.ether_shost[ETHER_ADDR_LEN - 1], eth_hdr.ether_dhost[ETHER_ADDR_LEN - 1], OAL_ARPOP_REPLY, rst);

    return rst;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_arp_rcv_reply
 功能描述  : 处理接收到的arp reply
 输入参数  : hmac_vap: hmac_vap指针
             oal_netbuf_stru:skb指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_arp_rcv_reply(hmac_vap_stru *hmac_vap, oal_eth_arphdr_stru *arp_hdr,
    osal_u8 is_mcst)
{
    osal_u16 usr_idx = 0;
    if (is_mcst) {
        return OAL_ERR_CODE_PROXY_ARP_REPLY_MCAST_COMP;
    }

    /* 如果接口上线不发GARP,通过arp reply将arp reply发送端的ip和mac记下来 */
    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, arp_hdr->ar_sha, &usr_idx) == OAL_SUCC) {
        hmac_proxy_add_ipv4(hmac_vap, *(osal_u32 *)arp_hdr->ar_sip, arp_hdr->ar_sha);
    }

    return OAL_ERR_CODE_PROXY_ARP_REPLY_INCOMP;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_arp
 功能描述  : 处理接收到的arp req和arp reply
 输入参数  : hmac_vap: hmac_vap指针
             oal_netbuf_stru:skb指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_arp(hmac_vap_stru *hmac_vap, oal_net_device_stru *dev,
    oal_netbuf_stru *buf)
{
    mac_ether_header_stru       *ether_hdr; /* 以太网头 */
    osal_u32                   arp_hdr_len;
    oal_eth_arphdr_stru         *arp_hdr;
    osal_u8                    is_mcst;
    mac_ether_header_stru       eth_hdr;

    ether_hdr   = (mac_ether_header_stru *)oal_netbuf_data(buf);
    arp_hdr     = (oal_eth_arphdr_stru *)(ether_hdr + 1);
    arp_hdr_len  = OAL_SIZEOF(oal_eth_arphdr_stru) + OAL_SIZEOF(mac_ether_header_stru);
    /* 此处判断skb中，是否已经有足够的空间保存了eth hdr + arp hdr的信息 */
    if (!oal_pskb_may_pull(buf, arp_hdr_len)) {
        return OAL_ERR_CODE_PROXY_ARP_INVLD_SKB_INCOMP;
    }

    eth_hdr = *(mac_ether_header_stru *)oal_netbuf_data(buf);

    oam_info_log4(0, OAM_SF_PROXYARP, "hmac_proxy_arp! s:%x->d:%x type %x, result %u!",
        eth_hdr.ether_shost[ETHER_ADDR_LEN - 1],
        eth_hdr.ether_dhost[ETHER_ADDR_LEN - 1],
        arp_hdr->ar_op,
        0);

    if (arp_hdr->ar_op == oal_host2net_short(OAL_ARPOP_REQUEST)) {
        return hmac_proxy_arp_rcv_req(hmac_vap, dev, arp_hdr);
    } else if (arp_hdr->ar_op == oal_host2net_short(OAL_ARPOP_REPLY)) {
        is_mcst = ETHER_IS_MULTICAST(ether_hdr->ether_dhost);
        return hmac_proxy_arp_rcv_reply(hmac_vap, arp_hdr, is_mcst);
    }

    return OAL_ERR_CODE_PROXY_ARP_NOT_REQ_REPLY_INCOMP;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_nd_learn_from_ns
 功能描述  : 通过DAD学习地址
 输入参数  : hmac_vap: hmac_vap指针
             oal_netbuf_stru:skb指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_nd_learn_from_ns(hmac_vap_stru *hmac_vap, oal_in6_addr *ipv6_src,
    osal_u8 *mac_src)
{
    osal_u16                  usr_idx = 0;
    osal_u8                   mac[ETHER_ADDR_LEN] = {0};
    oal_err_code_enum_uint32    err;

    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_src, &usr_idx) != OAL_SUCC) {
        return OAL_ERR_CODE_PROXY_ND_LEARN_USR_NOTEXIST_COMP;
    }

    /* 检查ipv6的地址已经存在,并且mac地址不相等，则将此帧发送出去，进行DAD检测 */
    err = hmac_proxy_get_mac_by_ipv6(hmac_vap, ipv6_src, mac);
    if ((err == OAL_SUCC) && (osal_memcmp(mac_src, mac, OAL_MAC_ADDR_LEN))) {
        return OAL_ERR_CODE_PROXY_ND_LEARN_USR_ALREADY_EXIST_INCOMP;
    }

    return hmac_proxy_add_ipv6(hmac_vap, ipv6_src, mac_src);
}

/*****************************************************************************
 函 数 名  : hmac_proxy_create_na
 功能描述  : 构造NA，用以代理回复NS
 输入参数  : hmac_vap: hmac_vap指针
             ether_hdr:指向接收到skb的以太头
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_netbuf_stru *hmac_proxy_create_na(oal_net_device_stru *net_dev, osal_u8 *src_mac, osal_u8 *dst_mac,
    osal_u8 *src_ipv6, osal_u8 *dst_ipv6)
{
    oal_netbuf_stru             *netbuff = OAL_PTR_NULL;
    osal_u32                   size;
    oal_ipv6hdr_stru            *ipv6 = OAL_PTR_NULL;
    oal_nd_msg_stru             *nd = OAL_PTR_NULL;
    osal_u32                   real_nd_msg_stru_len;

    /* sizeof(oal_nd_msg_stru)比实际多了4字节，最后加8字节是因为NA的option长8字节。 */
    real_nd_msg_stru_len = (sizeof(oal_nd_msg_stru) - 4) + 8;
    size = OAL_LL_ALLOCATED_SPACE(net_dev) + sizeof(oal_ipv6hdr_stru) + real_nd_msg_stru_len;
    netbuff  = oal_netbuf_alloc(size, 0, WLAN_MEM_NETBUF_ALIGN);
    if (netbuff == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    netbuff->dev = net_dev;
    netbuff->protocol = oal_host2net_short(ETHER_TYPE_IPV6);
    oal_netbuf_reserve(netbuff, OAL_LL_ALLOCATED_SPACE(net_dev));

    ipv6 = (oal_ipv6hdr_stru *)oal_netbuf_put(netbuff, OAL_SIZEOF(oal_ipv6hdr_stru));

    /* skb->len加上nd_msg的长度 */
    oal_netbuf_put(netbuff, real_nd_msg_stru_len);
    nd = (oal_nd_msg_stru *)(ipv6 + 1);

    /* 1. 填写以太头 */
    if (oal_dev_hard_header(netbuff, netbuff->dev, ETHER_TYPE_IPV6, dst_mac,
                            src_mac, netbuff->len) < 0) {
        oal_netbuf_free(netbuff);
        return OAL_PTR_NULL;
    }

    /* 2. 填写IPV6头 */
    *(osal_u32 *)ipv6 = oal_host2net_long(0x60000000);
    ipv6->payload_len = oal_host2net_short(real_nd_msg_stru_len);
    ipv6->nexthdr = OAL_IPPROTO_ICMPV6;
    ipv6->hop_limit = 0xff;
    memcpy_s(&ipv6->daddr, OAL_SIZEOF(oal_in6_addr), (oal_in6_addr *)dst_ipv6, OAL_SIZEOF(oal_in6_addr));
    memcpy_s(&ipv6->saddr, OAL_SIZEOF(oal_in6_addr), (oal_in6_addr *)src_ipv6, OAL_SIZEOF(oal_in6_addr));

    /* 3. 填写ICMPV6 邻居发现协议的NA */
    memset_s(&nd->icmph, OAL_SIZEOF(oal_icmp6hdr_stru), 0, OAL_SIZEOF(oal_icmp6hdr_stru));
    nd->icmph.icmp6_type = OAL_NDISC_NEIGHBOUR_ADVERTISEMENT;
    nd->icmph.icmp6_dataun.u_nd_advt.solicited = 1;
    memcpy_s(&nd->target, OAL_SIZEOF(oal_in6_addr), (oal_in6_addr *)src_ipv6, OAL_SIZEOF(oal_in6_addr));
    /* ICMPv6 选项 */
    nd->opt[0] = OAL_ND_OPT_TARGET_LL_ADDR;
    nd->opt[1] = 1;
    /* 数组元素为2，是src mac的起始位置 */
    if (memcpy_s(&nd->opt[2], OAL_MAC_ADDR_LEN, src_mac, OAL_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_proxy_create_na::memcpy_s error}");
    }
    nd->icmph.icmp6_cksum = oal_csum_ipv6_magic(&ipv6->saddr,
        &ipv6->daddr, real_nd_msg_stru_len, OAL_IPPROTO_ICMPV6,
        oal_csum_partial(&nd->icmph, real_nd_msg_stru_len, 0));

    return netbuff;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_get_mac_from_opt
 功能描述  : 从icmpv6 option中获取ipv6地址
 输入参数  : opt:  ipmpv6 option指针
             l_optlen: ipmpv6 option长度
             src:   标识获取源链路地址还是目的链路地址 1-源 0-目的
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u8 *hmac_proxy_get_mac_from_opt(osal_u8 *opt, osal_s32 l_optlen, oal_bool_enum_uint8 src)
{
    oal_nd_opt_hdr *ndopt;
    osal_s32       l_ndoptlen;
    osal_u32      offset;
    osal_s32       l_optlen_tmp;

    ndopt = (oal_nd_opt_hdr *)opt;
    l_optlen_tmp = l_optlen;

    while (l_optlen_tmp > 0) {
        if (l_optlen_tmp < (osal_s32)OAL_SIZEOF(oal_nd_opt_hdr)) {
            return OAL_PTR_NULL;
        }
        l_ndoptlen = ndopt->nd_opt_len << 3; /* 左移3位 */
        if (l_optlen_tmp < l_ndoptlen || l_ndoptlen == 0) {
            return OAL_PTR_NULL;
        }

        switch (ndopt->nd_opt_type) {
            case OAL_ND_OPT_TARGET_LL_ADDR: { /* NA中使用 */
                if (!src) {
                    return (osal_u8 *)(ndopt + 1);
                }

                break;
            }

            /* 发送者的链路层地址 */
            case OAL_ND_OPT_SOURCE_LL_ADDR: { /* NS中使用 */
                if (src) {
                    return (osal_u8 *)(ndopt + 1);
                }

                break;
            }
            default: {
                oam_warning_log1(0, OAM_SF_PROXYARP, "{hmac_proxy_get_mac_from_opt::type[%d]}", ndopt->nd_opt_type);
                break;
            }
        }

        l_optlen_tmp -= l_ndoptlen;
        offset = (osal_u32)l_ndoptlen;
        ndopt = (oal_nd_opt_hdr *)((osal_u8 *)ndopt + offset);
    }

    return OAL_PTR_NULL;
}

OAL_STATIC oal_err_code_enum_uint32 hmac_proxy_nd_rcv_ns_proc(hmac_vap_stru *hmac_vap, oal_net_device_stru *dev,
    osal_u8 *src_mac, oal_netbuf_stru *netbuff)
{
    osal_u16 idx;
    osal_u32 ul_ret;
    osal_s32 l_netifrx_rst;
    oal_err_code_enum_uint32 rst;
    mac_ether_header_stru eth_hdr;

    eth_hdr = *(mac_ether_header_stru *)oal_netbuf_data(netbuff);

    /* 没找到将skb上桥，找到了就直接空口发送 */
    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, src_mac, &idx) == OAL_SUCC) {
        ul_ret = hmac_tx_lan_to_wlan_etc(hmac_vap, netbuff);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_PROXYARP, "vap_id[%d] {hmac_proxy_nd_rcv_ns_ext 2wlan Err[%u]!!}",
                hmac_vap->vap_id, ul_ret);
            oal_netbuf_free(netbuff);
            netbuff = OAL_PTR_NULL;
        }

        rst = OAL_ERR_CODE_PROXY_ND_NS_REPLY_NA2BSS_COMP;
    } else {
        OAL_NETBUF_PROTOCOL(netbuff) = oal_eth_type_trans(netbuff, dev);
        l_netifrx_rst = oal_netif_rx(netbuff);
        if (l_netifrx_rst != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_PROXYARP, "vap_id[%d] {hmac_proxy_nd_rcv_ns_ext 2br Err[%d]!!}",
                hmac_vap->vap_id, l_netifrx_rst);
        }
#ifdef _PRE_WLAN_DFT_STAT
        if (l_netifrx_rst == OAL_SUCC) {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_SEND_LAN_SUCC);
        } else {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_SEND_LAN_FAIL);
        }
#endif

        rst = OAL_ERR_CODE_PROXY_ND_NS_REPLY_NA2ETH_COMP;
    }

    oam_info_log4(0, OAM_SF_PROXYARP, "hmac_proxy_nd_rcv_ns! s:%x->d:%x type %x, result %u!",
        eth_hdr.ether_shost[ETHER_ADDR_LEN - 1], eth_hdr.ether_dhost[ETHER_ADDR_LEN - 1],
        OAL_NDISC_NEIGHBOUR_ADVERTISEMENT, rst);

    return rst;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_nd_rcv_ns
 功能描述  : 处理接收到的NS
 输入参数  : hmac_vap: hmac_vap指针
             oal_netbuf_stru:skb指针
 输出参数  : 无
 说     明 :构造NA时，注意: NS源->NA目的
                            NS目的->NA源
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_nd_rcv_ns(hmac_vap_stru *hmac_vap, oal_net_device_stru *dev,
    mac_ether_header_stru *ether_hdr, osal_s32 l_opt_len)
{
    osal_u8               *src_mac = OAL_PTR_NULL;
    osal_u8                dst_mac[ETHER_ADDR_LEN];
    oal_netbuf_stru         *netbuff = OAL_PTR_NULL;

    oal_ipv6hdr_stru *ipv6_hdr = (oal_ipv6hdr_stru *)(ether_hdr + 1);
    osal_s32 l_src_type = oal_ipv6_addr_type(&ipv6_hdr->saddr);
    osal_s32 l_dst_type = oal_ipv6_addr_type(&ipv6_hdr->daddr);
    oal_nd_msg_stru *msg = (oal_nd_msg_stru *)(ipv6_hdr + 1);

    /* DAD检测 */
    if ((l_src_type == (osal_s32)OAL_IPV6_ADDR_ANY) && (l_dst_type & (osal_s32)OAL_IPV6_ADDR_MULTICAST)) {
        return hmac_proxy_nd_learn_from_ns(hmac_vap, &msg->target, ether_hdr->ether_shost);
    }

    /* 获取NS的icmpv6选项中携带的mac地址，即此帧发送者的mac地址 */
    src_mac = hmac_proxy_get_mac_from_opt(msg->opt, l_opt_len, OAL_TRUE);
    if (src_mac == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PROXY_ND_NS_OPT_INVLD_COMP;
    }

    if (hmac_proxy_get_mac_by_ipv6(hmac_vap, &msg->target, dst_mac) != OAL_SUCC) {
        return OAL_ERR_CODE_PROXY_ND_NS_FINDUSR_ERR_COMP;
    }

    /* 构造NA */
    netbuff = hmac_proxy_create_na(dev, dst_mac, src_mac, (osal_u8 *)&msg->target,
        (osal_u8 *)&ipv6_hdr->saddr);
    if (netbuff == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PROXY_ND_NS_CREATE_NA_FAIL_COMP;
    }

    return hmac_proxy_nd_rcv_ns_proc(hmac_vap, dev, src_mac, netbuff);
}

/*****************************************************************************
 函 数 名  : hmac_proxy_ipv6_addr_is_mcast
 功能描述  : 判断ipv6的地址是不是组播地址
 输入参数  : hmac_vap: hmac_vap指针
             buf:      skb指针
 输出参数  : 无
 说    明  :ipv6地址 ff02:xxxx:xxxx  最高为ff，表示组播
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
osal_s32 hmac_proxy_ipv6_addr_is_mcast(oal_in6_addr *addr)
{
    return (addr->s6_addr32[0] & oal_host2net_long(0xFF000000)) == oal_host2net_long(0xFF000000);
}

/*****************************************************************************
 函 数 名  : hmac_proxy_nd_rcv_na
 功能描述  : 处理NA
 输入参数  : hmac_vap: hmac_vap指针
             buf:      skb指针
 输出参数  : 无
 说    明  : 1.NA响应单播请求时S置位,可以带可以不带链路地址
             2.NA响应组播请求时,S不置位，target填成多播，必须要带链路地址
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_nd_rcv_na(hmac_vap_stru *hmac_vap, mac_ether_header_stru *ether_hdr,
    osal_s32 l_opt_len)
{
    oal_ipv6hdr_stru            *ipv6_hdr;
    oal_icmp6hdr_stru           *icmp6;
    oal_bool_enum_uint8          solicited;
    oal_nd_msg_stru             *nd;
    osal_u8                   *src_mac = OAL_PTR_NULL;
    oal_err_code_enum_uint32     err;
    osal_u16                   usr_idx = 0;

    ipv6_hdr        = (oal_ipv6hdr_stru *)(ether_hdr + 1);
    icmp6           = (oal_icmp6hdr_stru *)(ipv6_hdr + 1);
    nd              = (oal_nd_msg_stru *)icmp6;

    /* S在响应组播ns时不应该置位 */
    if ((nd->icmph.icmp6_dataun.u_nd_advt.solicited) && hmac_proxy_ipv6_addr_is_mcast(&nd->target)) {
        return OAL_ERR_CODE_PROXY_ND_NA_INVLD_COMP;
    }

    solicited = (nd->icmph.icmp6_dataun.u_nd_advt.solicited) &&
        !(hmac_proxy_ipv6_addr_is_mcast(&nd->target));

    src_mac = hmac_proxy_get_mac_from_opt(nd->opt, l_opt_len, OAL_FALSE);
    if (src_mac == OAL_PTR_NULL) {
        if (solicited) {
            return OAL_ERR_CODE_PROXY_ND_NA_UCAST_NOT_LLA_INCOMP;
        }
        return OAL_ERR_CODE_PROXY_ND_NA_MCAST_NOT_LLA_COMP;
    }
    /* 从NA中学习一下地址 */
    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, src_mac, &usr_idx) == OAL_SUCC) {
        err = hmac_proxy_add_ipv6(hmac_vap, &nd->target, src_mac);
        if (err != OAL_ERR_CODE_PROXY_ND_LEARN_USR_SUCC_COMP) {
            return err;
        }
    }

    if (solicited) {
        return OAL_ERR_CODE_PROXY_ND_NA_SOLICITED_INCOMP;
    }

    return OAL_ERR_CODE_PROXY_ND_NA_UNSOLICITED_COMP;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_nd
 功能描述  : 处理接收到的NS和NA
 输入参数  : hmac_vap: hmac_vap指针
             oal_netbuf_stru:skb指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_err_code_enum_uint32 hmac_proxy_nd(hmac_vap_stru *hmac_vap, oal_net_device_stru *dev,
    oal_netbuf_stru *buf)
{
    mac_ether_header_stru       *ether_hdr; /* 以太网头 */
    oal_ipv6hdr_stru            *ipv6_hdr;
    oal_icmp6hdr_stru           *icmp6;
    osal_u32                   hdr_len;
    osal_u32                   opt_len;         /* icmpv6选项的长度 */
    ether_hdr   = (mac_ether_header_stru *)oal_netbuf_data(buf);
    ipv6_hdr    = (oal_ipv6hdr_stru *)(ether_hdr + 1);
    icmp6       = (oal_icmp6hdr_stru *)(ipv6_hdr + 1);

    hdr_len       = OAL_SIZEOF(oal_ipv6hdr_stru) + OAL_SIZEOF(mac_ether_header_stru);
    if (!oal_pskb_may_pull(buf, hdr_len)) {
        return OAL_ERR_CODE_PROXY_ND_INVLD_SKB1_INCOMP;
    }
    /* sizeof(oal_nd_msg_stru)比实际多了4字节 */
    opt_len = buf->len - hdr_len - (OAL_SIZEOF(oal_nd_msg_stru) - 4);

    if (ipv6_hdr->nexthdr != OAL_IPPROTO_ICMPV6) {
        return OAL_ERR_CODE_PROXY_ND_NOT_ICMPV6_INCOMP;
    }

    hdr_len += OAL_SIZEOF(oal_icmp6hdr_stru);
    if (!oal_pskb_may_pull(buf, hdr_len)) {
        return OAL_ERR_CODE_PROXY_ND_INVLD_SKB2_INCOMP;
    }

    if ((icmp6->icmp6_type == OAL_NDISC_NEIGHBOUR_SOLICITATION)) {
        return hmac_proxy_nd_rcv_ns(hmac_vap, dev, ether_hdr, (osal_s32)opt_len);
    } else if (icmp6->icmp6_type == OAL_NDISC_NEIGHBOUR_ADVERTISEMENT) {
        return hmac_proxy_nd_rcv_na(hmac_vap, ether_hdr, (osal_s32)opt_len);
    }

    return OAL_ERR_CODE_PROXY_ND_ICMPV6_NOT_NSNA_INCOMP;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_is_need_drop
 功能描述  : 根据错误码决定是否要将处理的skb释放掉
 输入参数  : rst-错误码
 输出参数  : 无
 返 回 值  : 0/1--不需要释放/需要释放
 修改历史      :
  1.日    期   : 2014年8月6日
    修改内容   : 新生成函数
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_proxy_is_need_drop(oal_err_code_enum_uint32 rst)
{
    oal_bool_enum_uint8 is_need_drop = OAL_FALSE;

    switch (rst) {
        case OAL_ERR_CODE_PROXY_ARP_LEARN_USR_COMP:
        case OAL_ERR_CODE_PROXY_ARP_REPLY2BSS_COMP:
        case OAL_ERR_CODE_PROXY_ARP_CREATE_FAIL_COMP:
        case OAL_ERR_CODE_PROXY_ARP_REPLY2ETH_COMP:
        case OAL_ERR_CODE_PROXY_ARP_REPLY_MCAST_COMP:
        case OAL_ERR_CODE_PROXY_ND_LEARN_USR_SUCC_COMP:
        case OAL_ERR_CODE_PROXY_ND_NS_OPT_INVLD_COMP:
        case OAL_ERR_CODE_PROXY_ND_NS_CREATE_NA_FAIL_COMP:
        case OAL_ERR_CODE_PROXY_ND_NS_REPLY_NA2BSS_COMP:
        case OAL_ERR_CODE_PROXY_ND_NS_REPLY_NA2ETH_COMP:
        case OAL_ERR_CODE_PROXY_ARP_LEARN_USR_NOTEXIST_COMP:
        case OAL_ERR_CODE_PROXY_ND_LEARN_USR_NOTEXIST_COMP:     /* 此处和A的流程有点不一样,A此处不drop，继续后面的流程 */
        case OAL_ERR_CODE_PROXY_ND_NA_INVLD_COMP:
        case OAL_ERR_CODE_PROXY_ND_NA_MCAST_NOT_LLA_COMP:
        case OAL_ERR_CODE_PROXY_ND_NA_UNSOLICITED_COMP: {
            is_need_drop = OAL_TRUE;
            break;
        }
        case OAL_ERR_CODE_PROXY_ARP_FINDUSR_ERR_COMP:
        case OAL_ERR_CODE_PROXY_ARP_REPLY_INCOMP:
        case OAL_ERR_CODE_PROXY_ARP_INVLD_SKB_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_LEARN_USR_ALREADY_EXIST_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_LEARN_USR_FAIL_INCOMP:
        case OAL_ERR_CODE_PROXY_ARP_NOT_REQ_REPLY_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_NA_UCAST_NOT_LLA_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_NA_DUP_ADDR_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_NA_SOLICITED_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_INVLD_SKB1_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_NOT_ICMPV6_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_INVLD_SKB2_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_ICMPV6_NOT_NSNA_INCOMP:
        case OAL_ERR_CODE_PROXY_OTHER_INCOMP:
        case OAL_ERR_CODE_PROXY_ND_NS_FINDUSR_ERR_COMP: {
            is_need_drop = OAL_FALSE;
            break;
        }
        default: {
            break;
        }
    }
    return is_need_drop;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_arp_proc
 功能描述  : AP进行ARP和ND代理的主入口函数
 输入参数  : hmac_vap: mac_vap指针
             oal_netbuf_stru:skb指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年7月29日
    修改内容   : 新生成函数
*****************************************************************************/
oal_bool_enum_uint8 hmac_proxy_arp_proc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf)
{
    mac_ether_header_stru      *ether_hdr = OAL_PTR_NULL; /* 以太网头 */
    osal_u16                  ether_type;
    oal_err_code_enum_uint32    rst = OAL_SUCC;

    oam_info_log0(0, OAM_SF_PROXYARP, "{hmac_proxy_arp_proc!}");
    ether_hdr   = (mac_ether_header_stru *)oal_netbuf_data(buf);
    ether_type      = ether_hdr->ether_type;

    switch (ether_type) {
        case oal_host2net_short(ETHER_TYPE_ARP): {
            rst = hmac_proxy_arp(hmac_vap, hmac_vap->net_device, buf);

            break;
        }
        case oal_host2net_short(ETHER_TYPE_IPV6): {
            rst = hmac_proxy_nd(hmac_vap, hmac_vap->net_device, buf);

            break;
        }
        default: {
            return OAL_FALSE;
        }
    }

    return hmac_proxy_is_need_drop(rst);
}

/*****************************************************************************
 函 数 名  : hmac_proxy_arp_init
 功能描述  : 初始化proxy特性所需要的结构
 输入参数  : hmac_vap: mac_vap指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年8月6日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_proxy_arp_init(hmac_vap_stru *hmac_vap)
{
    osal_u32              loop;

    if ((hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) || (hmac_vap->hmac_vap_proxyarp != OAL_PTR_NULL)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 申请内存 */
    hmac_vap->hmac_vap_proxyarp = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_vap_proxyarp_stru), OAL_TRUE);
    if (hmac_vap->hmac_vap_proxyarp == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_PROXYARP, "hmac_proxy_arp_init err! malloc err!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(hmac_vap->hmac_vap_proxyarp, OAL_SIZEOF(mac_vap_proxyarp_stru), 0, OAL_SIZEOF(mac_vap_proxyarp_stru));

    for (loop = 0; loop < MAC_VAP_USER_HASH_MAX_VALUE; loop++) {
        OSAL_INIT_LIST_HEAD(&(hmac_vap->hmac_vap_proxyarp->ipv4_head[loop]));
    }

    for (loop = 0; loop < MAC_VAP_USER_HASH_MAX_VALUE; loop++) {
        OSAL_INIT_LIST_HEAD(&(hmac_vap->hmac_vap_proxyarp->ipv6_head[loop]));
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_proxy_exit
 功能描述  : 退出时释放所有内存
 输入参数  : hmac_vap: mac_vap指针
 输出参数  : 无
 修改历史      :
  1.日    期   : 2014年8月6日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_proxy_exit(hmac_vap_stru *hmac_vap)
{
    struct osal_list_head        *entry     = OAL_PTR_NULL;
    hmac_proxy_ipv4_hash_stru  *hash_ipv4 = OAL_PTR_NULL;
    hmac_proxy_ipv6_hash_stru  *hash_ipv6 = OAL_PTR_NULL;
    struct osal_list_head        *dlist_tmp = OAL_PTR_NULL;
    osal_u32                  loop;

    if ((hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) || (hmac_vap->hmac_vap_proxyarp == OAL_PTR_NULL)) {
        return;
    }

    for (loop = 0; loop < MAC_VAP_USER_HASH_MAX_VALUE; loop++) {
        osal_list_for_each_safe(entry, dlist_tmp,
            &(hmac_vap->hmac_vap_proxyarp->ipv4_head[loop])) {
            hash_ipv4 = (hmac_proxy_ipv4_hash_stru *)entry;
            osal_list_del(entry);
            oal_mem_free(hash_ipv4, OAL_TRUE);
            hmac_vap->hmac_vap_proxyarp->ipv4_num--;
        }
    }

    for (loop = 0; loop < MAC_VAP_USER_HASH_MAX_VALUE; loop++) {
        osal_list_for_each_safe(entry, dlist_tmp,
            &(hmac_vap->hmac_vap_proxyarp->ipv6_head[loop])) {
            hash_ipv6 = (hmac_proxy_ipv6_hash_stru *)entry;
            osal_list_del(entry);
            oal_mem_free(hash_ipv6, OAL_TRUE);
            hmac_vap->hmac_vap_proxyarp->ipv6_num--;
        }
    }

    oal_mem_free(hmac_vap->hmac_vap_proxyarp, OAL_TRUE);
    hmac_vap->hmac_vap_proxyarp = OAL_PTR_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_proxyarp_on
 功能描述  : 设置proxy arp特性开关
 输入参数  : hmac_vap : mac_vap指针
             on:开关值 0/1--关/开
 返 回 值  : 错误码
 修改历史      :
  1.日    期   : 2014年8月8日
    修改内容   : 新生成函数
*****************************************************************************/
osal_s32 hmac_proxyarp_on(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_proxyarp_en_stru *proxyarp_param = OSAL_NULL;
    oal_bool_enum_uint8 on;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    proxyarp_param = (mac_proxyarp_en_stru *)msg->data;
    on = proxyarp_param->proxyarp;

    if (on) {
        if (hmac_proxy_arp_init(hmac_vap) == OAL_SUCC) {
            hmac_vap->hmac_vap_proxyarp->is_proxyarp = on;
        }
        return OAL_SUCC;
    }

    hmac_proxy_exit(hmac_vap);
    return OAL_SUCC;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_PROXY_ARP */

