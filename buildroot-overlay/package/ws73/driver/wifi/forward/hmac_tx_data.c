/*
 * @Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: HMAC TX DATA Master File.
 * Create: 2021-12-15
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_tx_data.h"

#include "oal_netbuf.h"
#include "oam_struct.h"
#include "frw_ext_if.h"
#include "hmac_tx_amsdu.h"
#include "mac_frame.h"
#include "mac_data.h"
#include "hmac_frag.h"
#include "hmac_11i.h"
#include "hmac_ext_if.h"
#include "hmac_wmm.h"
#include "hmac_m2u.h"

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif
#include "hmac_wapi.h"

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
#include "hmac_traffic_classify.h"
#endif

#include "hmac_crypto_tkip.h"
#include "hmac_device.h"
#include "hmac_resource.h"

#include "hmac_tcp_opt.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif

#include "hmac_latency_stat.h"
#include "hmac_hook.h"
#include "hmac_feature_interface.h"

#include "hmac_feature_dft.h"
#include "wlan_thruput_debug.h"
#include "common_dft.h"
#include "hmac_tx_encap.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_rx_data_feature.h"
#include "hmac_dfx.h"
#include "hmac_single_proxysta.h"
#include "hmac_thruput_test.h"
#include "common_log_dbg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_TX_DATA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/*
 * definitions of king of games feature
 */
#ifdef CONFIG_NF_CONNTRACK_MARK
#define VIP_APP_VIQUE_TID              WLAN_TIDNO_VIDEO
#define VIP_APP_MARK                   0x5a
#endif

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HOST) && defined(_PRE_WLAN_FEATURE_CLASSIFY)
OAL_STATIC osal_u8 g_uc_ac_new = 0;
#endif
/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_tid_need_ba_session_check(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u8 tidno, oal_netbuf_stru *netbuf);

/*****************************************************************************
 函 数 名  : hmac_tx_ba_check
 功能描述  : 判断是否需要建立BA会话
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_TRUE代表需要建立BA会话
*****************************************************************************/
WIFI_HMAC_TCM_TEXT oal_bool_enum_uint8 hmac_tid_need_ba_session(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u8 tidno, oal_netbuf_stru *netbuf)
{
    wlan_ciper_protocol_type_enum_uint8 cipher_type;
    oal_bool_enum_uint8    ampdu_support;

    /* 该tid下不允许建BA，配置命令需求 */
    if (hmac_user->tx_tid_queue[tidno].ba_handle_tx_enable == OAL_FALSE) {
        return OAL_FALSE;
    }

    if (hmac_tid_ba_is_setup(hmac_user, tidno) == OAL_TRUE) {
        if (mac_mib_get_CfgAmpduTxAtive(hmac_vap) == OAL_TRUE) {
            return OAL_FALSE;
        }

        hmac_tx_ba_del(hmac_vap, hmac_user, tidno);
    }

    /* 配置命令不允许建立聚合时返回 */
    if (mac_mib_get_CfgAmpduTxAtive(hmac_vap) == OAL_FALSE) {
        return OAL_FALSE;
    }

    /* 建立BA会话，需要判断VAP的AMPDU的支持情况，因为需要实现建立BA会话时，一定发AMPDU */
    if (hmac_user_xht_support(hmac_user) == OAL_FALSE) {
        if (hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status != HMAC_BA_INIT) {
            hmac_tx_ba_del(hmac_vap, hmac_user, tidno);
        }
        return OAL_FALSE;
    }

    /* wep/tkip不支持11n及以上协议 */
    cipher_type = hmac_user->key_info.cipher_type;
    ampdu_support = ((cipher_type == WLAN_80211_CIPHER_SUITE_NO_ENCRYP) ||
                        (cipher_type == WLAN_80211_CIPHER_SUITE_CCMP) ||
                        (cipher_type == WLAN_80211_CIPHER_SUITE_CCMP_256) ||
                        (cipher_type == WLAN_80211_CIPHER_SUITE_GCMP) ||
                        (cipher_type == WLAN_80211_CIPHER_SUITE_GCMP_256)) ? OAL_TRUE : OAL_FALSE;
    if (ampdu_support == OAL_FALSE) {
        common_log_dbg1(0, OAM_SF_BA, "vap_id[%d] {hmac_tid_need_ba_session::ampdu_support is false", hmac_vap->vap_id);
        return OAL_FALSE;
    }

    if (mac_mib_get_AddBaMode(hmac_vap) != WLAN_ADDBA_MODE_AUTO) {
        return OAL_FALSE;
    }

    /* 针对VO业务, 根据VAP标志位确定是否建立BA会话 */
    if ((wlan_wme_tid_to_ac(tidno) == WLAN_WME_AC_VO) &&
        (hmac_vap->voice_aggr == OAL_FALSE)) {
        return OAL_FALSE;
    }

    return hmac_tid_need_ba_session_check(hmac_vap, hmac_user, tidno, netbuf);
}

OAL_STATIC oal_bool_enum_uint8 hmac_tid_need_ba_session_check(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u8 tidno, oal_netbuf_stru *netbuf)
{
    hmac_tid_stru   *hmac_tid_info = OAL_PTR_NULL;

    unref_param(netbuf);

    /* 判断HMAC VAP的是否支持聚合 */
    if (!(mac_mib_get_TxAggregateActived(hmac_vap) != 0 ||
        (hmac_vap->cap_flag.rifs_tx_on != 0))) {
        common_log_dbg1(0, OAM_SF_BA, "vap_id[%d] {hmac_tid_need_ba_session_check::tx_aggr_on of vap is off",
            hmac_vap->vap_id);
        return OAL_FALSE;
    }

    if (mac_mib_get_TxBASessionNumber(hmac_vap) >= WLAN_MAX_TX_BA) {
        common_log_dbg1(0, OAM_SF_BA, "{hmac_tid_need_ba_session_check::tx_ba_session_num[%d] exceed spec",
            mac_mib_get_TxBASessionNumber(hmac_vap));
        return OAL_FALSE;
    }

    /* 针对关闭WMM，非QOS帧处理 */
    if (hmac_user->cap_info.qos == OAL_FALSE) {
        common_log_dbg0(0, OAM_SF_BA, "{hmac_tid_need_ba_session_check::UnQos Frame pass!!}");
        return OAL_FALSE;
    }

    hmac_tid_info = &(hmac_user->tx_tid_queue[tidno]);
    /* 虽然上面判断了状态不是COMPLETE,这里状态仍然可能是COMPLETE */
    if ((hmac_tid_info->ba_tx_info.ba_status == HMAC_BA_INIT) &&
        (hmac_tid_info->ba_tx_info.addba_attemps < HMAC_ADDBA_EXCHANGE_ATTEMPTS)) {
        hmac_tid_info->ba_tx_info.ba_status = HMAC_BA_INPROGRESS;
        hmac_tid_info->ba_tx_info.addba_attemps++;
    } else {
        common_log_dbg3(0, OAM_SF_BA, "vap_id[%d] {hmac_tid_need_ba_session_check::addba_attemps[%d] of tid[%d] \
            is COMPLETE}", hmac_vap->vap_id, hmac_tid_info->ba_tx_info.addba_attemps, tidno);
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_report_eth_frame_etc
 功能描述  : 将以太网过来的帧上报SDT
 输入参数  : hmac_vap:mac_vap结构
             pst_netbuf :以太网过来的数据帧
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_void hmac_tx_report_eth_frame_etc(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *pst_netbuf)
{
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL) || OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_report_eth_frame_etc::input null}");
        return;
    }

    /* 统计以太网下来的数据包统计 */
    /* 修复错误 ，1)变量hmac_vap 在之前是用ifdef 定义的。2)rx_pkt_to_lan 应换成rx_bytes_to_lan，统计字节数 */
    OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_pkt_num_from_lan, 1);
    OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_bytes_from_lan, OAL_NETBUF_LEN(pst_netbuf));
}

/*****************************************************************************
 函 数 名  : oal_free_netbuf_list
 功能描述  : 释放一个netbuf链，此链中的skb或者都来自lan，或者都来自wlan
 输入参数  : buf－SKB结构体单向链表，最后一个next指针务必为NULL，否则会出异常
 返 回 值  : 释放的buf数目
*****************************************************************************/
osal_u16 hmac_free_netbuf_list_etc(oal_netbuf_stru *netbuf)
{
    oal_netbuf_stru     *netbuf_tmp;
    oal_netbuf_stru     *netbuf_input = netbuf;
    osal_u16           netbuf_num = 0;

    if (netbuf_input != OAL_PTR_NULL) {
        while (netbuf_input != OAL_PTR_NULL) {
            netbuf_tmp = oal_netbuf_list_next(netbuf_input);
            netbuf_num++;
            oal_netbuf_free(netbuf_input);
            netbuf_input = netbuf_tmp;
        }
    } else {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_free_netbuf_list_etc::netbuf is null}");
    }

    return netbuf_num;
}

#ifdef _PRE_WLAN_FEATURE_HS20
/*****************************************************************************
 函 数 名  : hmac_tx_set_qos_map
 功能描述  : 根据对端发送的QoS_Map项更新TID
 输入参数  : oal_netbuf_stru *netbuf, osal_u8 *puc_tid
*****************************************************************************/
osal_void hmac_tx_set_qos_map(oal_netbuf_stru *netbuf, osal_u8 *puc_tid)
{
    mac_ether_header_stru  *ether_header;
    mac_ip_header_stru     *pst_ip;
    osal_u8               dscp;
    mac_tx_ctl_stru        *tx_ctl;
    hmac_vap_stru          *hmac_vap     = OAL_PTR_NULL;
    osal_u8               idx;

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_get_cb_tx_vap_index(tx_ctl));

    /* 获取以太网头 */
    ether_header = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    /* 参数合法性检查 */
    if ((hmac_vap == OAL_PTR_NULL) || (ether_header == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_HS20,
            "{hmac_tx_set_qos_map::The input parameter of QoS_Map_Configure_frame_with_QoSMap_Set_element"
            " is OAL_PTR_NULL.}");
        return;
    }

    /* 从IP TOS字段寻找DSCP优先级 */
    /*---------------------------------
      tos位定义
      ---------------------------------
    |    bit7~bit2      | bit1~bit0 |
    |    DSCP优先级     | 保留      |
    ---------------------------------*/

    /* 偏移一个以太网头，取ip头 */
    pst_ip = (mac_ip_header_stru *)(ether_header + 1);
    dscp = pst_ip->tos >> WLAN_DSCP_PRI_SHIFT;
    oam_info_log2(0, OAM_SF_HS20, "{hmac_tx_set_qos_map::tos = %d, dscp=%d.}", pst_ip->tos, dscp);

    if ((hmac_vap->cfg_qos_map_param.num_dscp_except > 0) &&
        (hmac_vap->cfg_qos_map_param.num_dscp_except <= MAX_DSCP_EXCEPT) &&
        (hmac_vap->cfg_qos_map_param.valid)) {
        for (idx = 0; idx < hmac_vap->cfg_qos_map_param.num_dscp_except; idx++) {
            if (dscp == hmac_vap->cfg_qos_map_param.dscp_exception[idx]) {
                *puc_tid  = hmac_vap->cfg_qos_map_param.dscp_exception_up[idx];
                mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
                mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_HS20;
                mac_get_cb_is_needretry(tx_ctl)   = OAL_TRUE;
                hmac_vap->cfg_qos_map_param.valid = 0;
                return;
            }
        }
    }

    for (idx = 0; idx < MAX_QOS_UP_RANGE; idx++) {
        if ((dscp < hmac_vap->cfg_qos_map_param.up_high[idx])
            && (dscp > hmac_vap->cfg_qos_map_param.up_low[idx])) {
            *puc_tid = idx;
            mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
            mac_get_cb_frame_subtype(tx_ctl) = MAC_DATA_HS20;
            mac_get_cb_is_needretry(tx_ctl)             = OAL_TRUE;
            hmac_vap->cfg_qos_map_param.valid = 0;
            return;
        } else {
            *puc_tid = 0;
        }
    }
    hmac_vap->cfg_qos_map_param.valid = 0;
    return;
}
#endif // _PRE_WLAN_FEATURE_HS20

#ifdef _PRE_WLAN_FEATURE_CLASSIFY
/* 判断该帧是否为4 次握手的4/4 EAPOL KEY 单播密钥协商帧 */
OAL_STATIC osal_bool hmac_is_eapol_ptk_4_4(mac_eapol_header_stru *eapol_header)
{
    mac_eapol_key_stru *eapol_key = OSAL_NULL;

    eapol_key = (mac_eapol_key_stru *)(eapol_header + 1);
    if (eapol_key->key_data_length[0] == 0 && eapol_key->key_data_length[1] == 0) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

OAL_STATIC osal_void hmac_tx_set_eapol_key_flag(mac_tx_ctl_stru        *tx_ctl)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(mac_get_cb_tx_user_idx(tx_ctl));
    if (hmac_user != OSAL_NULL) {
        hmac_user->eapol_key_4_4_tx_succ = OSAL_FALSE;
    }
}

#ifdef CONFIG_NF_CONNTRACK_MARK
OAL_STATIC osal_u8 hmac_tx_game_vip_mark(oal_netbuf_stru *netbuf, osal_u8 *tid)
{
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u8 skbuff_mark = 0;

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
    skbuff_mark = ((struct sk_buff *)(netbuf))->mark;
#else /* !2.6.0 */
    skbuff_mark = ((struct sk_buff *)(netbuf))->nfmark;
#endif /* 2.6.0 */
#endif
    if (skbuff_mark == VIP_APP_MARK) {
        *tid = VIP_APP_VIQUE_TID;
        tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        tx_ctl->is_needretry = OAL_TRUE;
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}
#endif

OAL_STATIC OAL_INLINE osal_void hmac_tx_eapol_key_proc(hmac_vap_stru *hmac_vap, mac_ether_header_stru *ether_header,
    mac_tx_ctl_stru *tx_ctl, osal_u8 tid)
{
    /* 如果是4 次握手设置单播密钥，则设置tx cb 中bit_is_eapol_key_ptk 置一，dmac 发送不加密 */
    if (hmac_is_eapol_key_ptk_etc((mac_eapol_header_stru *)(ether_header + 1)) == OAL_TRUE) {
        mac_get_cb_is_eapol_key_ptk(tx_ctl) = OAL_TRUE;
        if (hmac_is_eapol_ptk_4_4((mac_eapol_header_stru *)(ether_header + 1)) == OAL_TRUE) {
            mac_get_cb_is_need_resp(tx_ctl) = OAL_TRUE;
            /* 发送eapoly-key 4/4时设置标志位，防止上层重复发帧影响下一次更新单播密钥 */
            hmac_tx_set_eapol_key_flag(tx_ctl);
        }
    }

    oam_warning_log4(0, OAM_SF_CONN, "vap_id[%d] {hmac_tx_eapol_key_proc::tid=%d,IS_EAPOL_KEY_PTK=%d,rsp=%d.}",
        hmac_vap->vap_id, tid, mac_get_cb_is_eapol_key_ptk(tx_ctl), tx_ctl->need_rsp);
}

osal_void hmac_tx_set_vip_tid(mac_tx_ctl_stru *tx_ctl, osal_u8 type, osal_u8 subtype,
    osal_u8 is_retry, osal_u8 *tid_out)
{
    *tid_out = WLAN_DATA_VIP_TID;
    mac_get_cb_frame_type(tx_ctl) = type;
    mac_get_cb_frame_subtype(tx_ctl) = subtype;
    mac_get_cb_is_needretry(tx_ctl) = is_retry;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC OAL_INLINE osal_bool hmac_tx_classify_special(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, osal_u8 *tid)
{
    mac_ether_header_stru *ether_header = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_TX_FRAME);
    osal_bool *m2u_fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_TX_CLASSIFY_SPECIAL);

    if (m2u_fhook != OSAL_NULL) {
        if (((hmac_m2u_tx_classify_special_cb)m2u_fhook)(hmac_vap, tx_ctl, tid) == OSAL_TRUE) {
            return OSAL_TRUE;
        }
    }
#ifdef CONFIG_NF_CONNTRACK_MARK
    /* the king of game feature will mark packets
     * and we will use VI queue to send these packets.
     */
    if (hmac_tx_game_vip_mark(netbuf, tid) == OSAL_TRUE) {
        return OSAL_TRUE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_HS20
    if (ether_header->ether_type == oal_host2net_short(ETHER_TYPE_IP) && hmac_vap->cfg_qos_map_param.valid) {
        hmac_tx_set_qos_map(netbuf, tid);
        return OSAL_TRUE;
    }
#endif

    if (fhook != OSAL_NULL) {
        if (((hmac_wapi_tx_classify_special_cb)fhook)(hmac_vap, ether_header->ether_type, tx_ctl, tid) == OSAL_TRUE) {
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC OAL_INLINE osal_void hmac_tx_classify_ether_ip(hmac_vap_stru *hmac_vap,
    mac_tx_ctl_stru *tx_ctl, mac_ether_header_stru *ether_header, osal_u8 *tid)
{
    /* 从IP TOS字段寻找优先级 */
    /*----------------------------------------------------------------------
        tos位定义
        ----------------------------------------------------------------------
    | bit7~bit5 | bit4 |  bit3  |  bit2  |   bit1   | bit0 |
    | 包优先级  | 时延 | 吞吐量 | 可靠性 | 传输成本 | 保留 |
        ----------------------------------------------------------------------*/
    mac_ip_header_stru *ip_hdr = (mac_ip_header_stru *)(ether_header + 1);
    osal_u8 dscp = ip_hdr->tos >> WLAN_DSCP_PRI_SHIFT;
    if (hmac_vap->dscp_tid_map[dscp] != HMAC_DSCP_VALUE_INVALID) {
        *tid = hmac_vap->dscp_tid_map[dscp];
        return;
    }
    *tid = ip_hdr->tos >> WLAN_IP_PRI_SHIFT;
#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    if (mac_mib_get_TxTrafficClassifyFlag(hmac_vap) == OAL_SWITCH_ON) {
        if (*tid != 0) {
            return;
        }
        hmac_tx_traffic_classify_etc(tx_ctl, ip_hdr, tid);
    }
#endif
    if (hmac_is_dhcp_port_etc(ip_hdr) == OAL_TRUE) {
        hmac_tx_set_vip_tid(tx_ctl, WLAN_CB_FRAME_TYPE_DATA, MAC_DATA_DHCP, OAL_TRUE, tid);
    } else if (ip_hdr->protocol == MAC_TCP_PROTOCAL) {
        osal_u32 ip_hdr_len = mac_ip_hdr_len(ip_hdr);
        mac_tcp_header_stru *tcp = (mac_tcp_header_stru *)((osal_u8 *)ip_hdr + ip_hdr_len);
        /* 对于chariot信令报文进行特殊处理，防止断流 */
        if ((OAL_NTOH_16(tcp->dport) == MAC_CHARIOT_NETIF_PORT) ||
            (OAL_NTOH_16(tcp->sport) == MAC_CHARIOT_NETIF_PORT)) {
            oam_info_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify_lan_to_wlan::chariot tcp.}", hmac_vap->vap_id);
            hmac_tx_set_vip_tid(tx_ctl, WLAN_CB_FRAME_TYPE_DATA, MAC_DATA_CHARIOT_SIG, OAL_TRUE, tid);
        }
        if (oal_netbuf_is_tcp_ack_etc((oal_ip_header_stru *)ip_hdr) == OSAL_TRUE) {
            /* 强制聚合模式下，标记TCP ack帧，TCP ACK帧入VO队列，不走强制聚合发包流程 */
            mac_get_cb_tcp_ack(tx_ctl) = OAL_TRUE;
            if (hmac_is_thruput_enable(THRUPUT_FIX_AGGR) != 0) {
                *tid = WLAN_TIDNO_VOICE;
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_tx_classify_lan_to_wlan
 功能描述  : 从lan过来报文的业务识别
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_tx_classify_lan_to_wlan(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, osal_u8 *tid)
{
    /* 获取以太网头 */
    mac_ether_header_stru *ether_header = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    osal_u16 vap_id = hmac_vap->vap_id;
    oal_vlan_ethhdr_stru *vlan_ethhdr = OSAL_NULL;
    osal_u32 ipv6_hdr, pri;

    if (hmac_tx_classify_special(hmac_vap, netbuf, tid) == OSAL_TRUE) {
        unref_param(vap_id);
        return;
    }

    switch (ether_header->ether_type) {
        case oal_host2net_short(ETHER_TYPE_IP):
            hmac_tx_classify_ether_ip(hmac_vap, tx_ctl, ether_header, tid);
            break;
        case oal_host2net_short(ETHER_TYPE_IPV6):
            /* 从IPv6 traffic class字段获取优先级 */
            /*----------------------------------------------------------------------
                IPv6包头 前32为定义
             -----------------------------------------------------------------------
            | 版本号 | traffic class   | 流量标识 |
            | 4bit   | 8bit(同ipv4 tos)|  20bit   |
            -----------------------------------------------------------------------*/
            ipv6_hdr = *((osal_u32 *)(ether_header + 1));  /* 偏移一个以太网头，取ip头 */
            pri = (oal_net2host_long(ipv6_hdr) & WLAN_IPV6_PRIORITY_MASK) >> WLAN_IPV6_PRIORITY_SHIFT;
            *tid = (osal_u8)(pri >> WLAN_IP_PRI_SHIFT);
            /* 如果是DHCPV6帧，则进入VO队列发送 */
            if (hmac_is_dhcp6_etc((oal_ipv6hdr_stru *)(ether_header + 1)) == OAL_TRUE) {
                oam_info_log0(0, OAM_SF_TX, "{hmac_tx_classify_lan_to_wlan::DHCP6.}");
                hmac_tx_set_vip_tid(tx_ctl, WLAN_CB_FRAME_TYPE_DATA, MAC_DATA_DHCPV6, OAL_TRUE, tid);
            }
            break;
        case oal_host2net_short(ETHER_TYPE_PAE):
            /* 如果是EAPOL帧，则进入VO队列发送 */
            oam_warning_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify_lan_to_wlan::trace_eapol::PAE.}", vap_id);
            hmac_tx_set_vip_tid(tx_ctl, WLAN_CB_FRAME_TYPE_DATA, MAC_DATA_EAPOL, OAL_TRUE, tid);
            hmac_tx_eapol_key_proc(hmac_vap, ether_header, tx_ctl, *tid);
            break;
        /* TDLS帧处理，建链保护，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_TDLS):
            oam_info_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify_lan_to_wlan::TDLS.}", vap_id);
            *tid = WLAN_DATA_VIP_TID;
            break;
        /* PPPOE帧处理，建链保护(发现阶段, 会话阶段)，入高优先级TID队列 */
        case oal_host2net_short(ETHER_TYPE_PPP_DISC):
        case oal_host2net_short(ETHER_TYPE_PPP_SES):
            oam_info_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify_lan_to_wlan::PPP_DISC, PPP_SES.}", vap_id);
            hmac_tx_set_vip_tid(tx_ctl, WLAN_CB_FRAME_TYPE_DATA, MAC_DATA_PPPOE, OAL_TRUE, tid);
            break;
        case oal_host2net_short(ETHER_TYPE_VLAN):
            /* 获取vlan tag的优先级 */
            vlan_ethhdr = (oal_vlan_ethhdr_stru *)oal_netbuf_data(netbuf);
            /*------------------------------------------------------------------
                802.1Q(VLAN) TCI(tag control information)位定义
             -------------------------------------------------------------------
            |Priority | DEI  | Vlan Identifier |
            | 3bit    | 1bit |      12bit      |
             ------------------------------------------------------------------*/

            /* 右移13位，提取高3位优先级 */
            *tid = (osal_u8)(((osal_u16)oal_net2host_short(vlan_ethhdr->h_vlan_TCI)) >> OAL_VLAN_PRIO_SHIFT);
            oam_info_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify_lan_to_wlan::VLAN. tid %d}", vap_id, *tid);
            break;
        default:
            break;
    }
}

OAL_STATIC OAL_INLINE  osal_u8 wlan_tos_to_tid(osal_u8 tos)
{
    return ((((tos) == 0) || ((tos) == 0x3)) ? WLAN_TIDNO_BEST_EFFORT :
        (((tos) == 0x1) || ((tos) == 0x2)) ? WLAN_TIDNO_BACKGROUND :
        (((tos) == 0x4) || ((tos) == 0x5)) ? WLAN_TIDNO_VIDEO : WLAN_TIDNO_VOICE);
}

/*****************************************************************************
 函 数 名  : hmac_tx_update_tid
 功能描述  : 根据算法需求，tid = 1, 3, 5, 7的，分别更新为0, 2, 4, 6
             如果WMM功能关闭，直接填为DMAC_WMM_SWITCH_TID
 输入参数  : puc_tid 注意，此参数为入出参
 输出参数  : puc_tid
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_tx_update_tid(oal_bool_enum_uint8 en_wmm, osal_u8 *puc_tid)
{
    if (OAL_LIKELY(en_wmm == OAL_TRUE)) { /* wmm使能 */
        *puc_tid = (*puc_tid < WLAN_TIDNO_BUTT) ? wlan_tos_to_tid(*puc_tid) : WLAN_TIDNO_BCAST;
    } else { /* wmm不使能 */
        *puc_tid = MAC_WMM_SWITCH_TID;
    }
}

/*****************************************************************************
 函 数 名  : hmac_tx_wmm_acm_etc
 功能描述  : 根据热点配置ACM，重新选择队列
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u8 hmac_tx_wmm_acm_etc(oal_bool_enum_uint8 en_wmm,
    hmac_vap_stru *hmac_vap, osal_u8 *puc_tid)
{
#if defined(_PRE_PRODUCT_ID_HOST)
    osal_u8                   ac;

    if ((hmac_vap == OAL_PTR_NULL) || (puc_tid == OAL_PTR_NULL)) {
        return OAL_FALSE;
    }

    if (en_wmm == OAL_FALSE) {
        return OAL_FALSE;
    }

    ac = wlan_wme_tid_to_ac(*puc_tid);
    g_uc_ac_new = ac;
    while ((g_uc_ac_new != WLAN_WME_AC_BK) &&
           (mac_mib_get_qap_edca_table_mandatory(hmac_vap, g_uc_ac_new) == OAL_TRUE)) {
        switch (g_uc_ac_new) {
            case WLAN_WME_AC_VO:
                g_uc_ac_new = WLAN_WME_AC_VI;
                break;

            case WLAN_WME_AC_VI:
                g_uc_ac_new = WLAN_WME_AC_BE;
                break;

            default:
                g_uc_ac_new = WLAN_WME_AC_BK;
                break;
        }
    }

    if (g_uc_ac_new != ac) {
        *puc_tid = wlan_wme_ac_to_tid(g_uc_ac_new);
    }
#endif /* defined(_PRE_PRODUCT_ID_HOST) */

    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_classify
 功能描述  : 以太网包 业务识别
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void  hmac_tx_classify(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    osal_u8               tid = 0;
    mac_tx_ctl_stru        *tx_ctl;
    hmac_device_stru       *hmac_device = OAL_PTR_NULL;
    osal_u8 tx_wmm_acm;

    /* 非qos下同样需要对EAPOL进行业务识别 */
    hmac_tx_classify_lan_to_wlan(hmac_vap, netbuf, &tid);

    /* 非QoS站点，直接返回 */
    if (OAL_UNLIKELY(hmac_user->cap_info.qos != OAL_TRUE)) {
        oam_info_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify::user is a none QoS station.}",
            hmac_vap->vap_id);
        return;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_user->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify::mac_dev null.}",
            hmac_vap->vap_id);
        return;
    }

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    tx_wmm_acm = hmac_tx_wmm_acm_etc(hmac_device->en_wmm, hmac_vap, &tid);
    if (tx_wmm_acm != OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_classify::hmac_tx_wmm_acm_etc return false.}",
            hmac_vap->vap_id);
    }

    /* 由于芯片支持VO聚合, VIP报文和VO报文都放在tid 6聚合传输, 导致VIP报文无法以基本速率发送。
        因此, 将VIP报文放在tid 7, 与实际VO业务区分, 保证VIP报文的传输可靠性。 */
    /* 正常数据只使用了4个tid:0 1 5 6 */
    if ((!mac_get_cb_is_vipframe(tx_ctl)) || (hmac_device->en_wmm == OAL_FALSE)) {
        hmac_tx_update_tid(hmac_device->en_wmm, &tid);
    }

    /* 所有vip帧都不能进行聚合,此处主要是修改ARP帧MAC_DATA_ARP_REQ/MAC_DATA_ARP_RSP的TID */
    if (mac_get_cb_is_vipframe(tx_ctl)) {
        tid = WLAN_DATA_VIP_TID;
    }

    /* 设置ac和tid到cb字段 */
    mac_get_cb_wme_tid_type(tx_ctl)  = tid;
    mac_get_cb_wme_ac_type(tx_ctl)   = wlan_wme_tid_to_ac(tid);
    /* 强制报文走强制队列 */
    hmac_set_ac_mode(tx_ctl);

    return;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_tx_filter_security
 功能描述  : 针对来自以太网的数据帧做安全过滤
 输入参数  : hmac_vap_stru     *hmac_vap
             oal_netbuf_stru  *netbuf
             hmac_user_stru   *hmac_user
             mac_tx_ctl_stru  *tx_ctl
 输出参数  : OAL_TRUE
 返 回 值  : osal_u32
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_filter_security(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, const hmac_user_stru  *hmac_user)
{
    mac_ether_header_stru *ether_header = OAL_PTR_NULL;
    osal_u32 ul_ret = OAL_SUCC;
    osal_u16 ether_type;

    if (mac_mib_get_rsnaactivated(hmac_vap) == OAL_TRUE) { /* 判断是否使能WPA/WPA2 */
        if (hmac_user->port_valid != OAL_TRUE) { /* 判断端口是否打开 */
            /* 获取以太网头 */
            ether_header = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
            /* 发送数据时，针对非EAPOL 的数据帧做过滤 */
            if (oal_byteorder_host_to_net_uint16(ETHER_TYPE_PAE) != ether_header->ether_type) {
                ether_type = oal_byteorder_host_to_net_uint16(ether_header->ether_type);
                oam_warning_log3(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_filter_security::TYPE 0x%04x, 0x%04x.}",
                                 hmac_vap->vap_id,
                                 ether_type, ETHER_TYPE_PAE);
                ul_ret = OAL_FAIL;
            }
        }
    }
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_tx_ba_setup_etc
 功能描述  : 自动触发BA会话的建立
*****************************************************************************/
osal_void  hmac_tx_ba_setup_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 tidno)
{
    mac_action_mgmt_args_stru   action_args;   /* 用于填写ACTION帧的参数 */

    /*
    建立BA会话时，action_args结构各个成员意义如下
    (1)category:action的类别
    (2)action:BA action下的类别
    (3)arg1:BA会话对应的TID
    (4)arg2:BUFFER SIZE大小
    (5)arg3:BA会话的确认策略
    (6)arg4:TIMEOUT时间
    */
    action_args.category = MAC_ACTION_CATEGORY_BA;
    action_args.action   = MAC_BA_ACTION_ADDBA_REQ;
    action_args.arg1     = tidno;                      /* 该数据帧对应的TID号 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* ADDBA_REQ中，buffer_size的默认大小 */
    action_args.arg2     = (osal_u32)hwifi_get_ampdu_tx_baw_size();
    common_log_dbg1(0, OAM_SF_TX, "hmac_tx_ba_setup_etc::[ba buffer size:%d]", action_args.arg2);
#else
    action_args.arg2     = WLAN_AMPDU_TX_MAX_BUF_SIZE;    /* ADDBA_REQ中，buffer_size的默认大小 */
#endif

    action_args.arg3     = MAC_BA_POLICY_IMMEDIATE;       /* BA会话的确认策略 */
    action_args.arg4     = 0;                             /* BA会话的超时时间设置为0 */

    /* 建立BA会话 */
    hmac_mgmt_tx_action_etc(hmac_vap, hmac_user, &action_args);
}

/*****************************************************************************
 函 数 名  : hmac_tx_ucast_process_ampdu_aspdu
 功能描述  : 单播处理 _PRE_WLAN_FEATURE_AMPDU 或 _PRE_WLAN_FEATURE_AMSDU 特性
 输入参数  : hmac_vap hmac VSP结构体;
           buf netbuf结构体;
           user 用户结构体;
           tx_ctl netbuf控制字段
 调用函数  : hmac_tx_ucast_process_etc
*****************************************************************************/
OAL_STATIC WIFI_HMAC_TCM_TEXT osal_u32 hmac_tx_ucast_process_ampdu_aspdu(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, hmac_user_stru *hmac_user, const mac_tx_ctl_stru *tx_ctl)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AMSDU_TX_NOTIFY_ETC);
    osal_u32 ret = HMAC_TX_PASS;

    /* 如果是EAPOL、DHCP帧，则不允许主动建立BA会话 */
    if (mac_get_cb_is_vipframe(tx_ctl)) {
        return HMAC_TX_PASS;
    }

#ifdef _PRE_WLAN_FEATURE_AMPDU
    if (hmac_tid_need_ba_session(hmac_vap, hmac_user, mac_get_cb_wme_tid_type(tx_ctl), netbuf) == OAL_TRUE) {
        /* 自动触发建立BA会话，设置AMPDU聚合参数信息在DMAC模块的处理addba rsp帧的时刻后面 */
        hmac_tx_ba_setup_etc(hmac_vap, hmac_user, mac_get_cb_wme_tid_type(tx_ctl));
    }
#endif

    if (fhook != OSAL_NULL) {
        ret = ((hmac_amsdu_notify_etc_cb)fhook)(hmac_vap, hmac_user, netbuf);
        if (OAL_UNLIKELY(ret != HMAC_TX_PASS)) {
            return ret;
        }
    }

    return HMAC_TX_PASS;
}

/*****************************************************************************
 函 数 名  : hmac_tx_ucast_process_etc
 功能描述  : 单播处理
 输入参数  : hmac_vap VAP结构体; netbuf netbuf结构体;user 用户结构体
 调用函数  : hmac_tx_mpdu_process_ap
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_tx_ucast_process_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    hmac_user_stru *hmac_user, mac_tx_ctl_stru *tx_ctl)
{
    /* 安全过滤 */
    if (OAL_UNLIKELY(hmac_tx_filter_security(hmac_vap, netbuf, hmac_user) != OAL_SUCC)) {
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_security_check_faild, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_SECURITY_FILTER;
    }

    /* 以太网业务识别 */
#ifdef _PRE_WLAN_FEATURE_CLASSIFY
    hmac_tx_classify(hmac_vap, hmac_user, netbuf);
#endif

    return hmac_tx_ucast_process_ampdu_aspdu(hmac_vap, netbuf, hmac_user, tx_ctl);
}

/*****************************************************************************
 函 数 名  : hmac_tx_mpdu_process
 功能描述  : 单个MPDU处理函数
 输入参数  : event－事件结构体
             hmac_vap－vap结构体
             netbuf－BUF结构体
             tx_ctl－CB结构体
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_lan_mpdu_process_sta(
    hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    hmac_user_stru *hmac_user;      /* 目标STA结构体 */
    osal_u32      ret;
    osal_u16      user_idx;
    osal_u8      *ether_payload;
    mac_ether_header_stru *ether_hdr  = (mac_ether_header_stru *)oal_netbuf_data(netbuf); /* 以太网头 */
    mac_get_cb_tx_vap_index(tx_ctl) = hmac_vap->vap_id;
    user_idx = hmac_vap->assoc_vap_id;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_USER_NULL;
    }
    if (
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        hmac_vap->wds_table.wds_vap_mode == WDS_MODE_REPEATER_STA) {
        hmac_wds_update_neigh(hmac_vap, ether_hdr->ether_shost);
    } else if ((hmac_vap->wds_table.wds_vap_mode == WDS_MODE_NONE) &&
#endif
        (oal_byteorder_host_to_net_uint16(ETHER_TYPE_ARP) == ether_hdr->ether_type)) {
        ether_hdr++;
        ether_payload = (osal_u8 *)ether_hdr;
        /* The source MAC address is modified only if the packet is an   */
        /* ARP Request or a Response. The appropriate bytes are checked. */
        /* Type field (2 bytes): ARP Request (1) or an ARP Response (2)  */
        if ((ether_payload[6] == 0x00) && (ether_payload[7] == 0x02 || ether_payload[7] == 0x01)) { /* 6,7:偏移值 */
            /* Set Address2 field in the WLAN Header with source address */
            oal_set_mac_addr(ether_payload + BIT_OFFSET_8, mac_mib_get_station_id(hmac_vap));
        }
    }

    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)(user_idx & 0x3F); /* 只有6bit保存user_idx */
    ret = hmac_tx_ucast_process_etc(hmac_vap, netbuf, hmac_user, tx_ctl);
    if (OAL_UNLIKELY(ret != HMAC_TX_PASS)) {
        return ret;
    }
    if (mac_pk_mode(netbuf)) {
        /* pk模式提前填写描述符信息 */
        hmac_tx_fill_first_cb_info(tx_ctl, hmac_vap, hmac_user);
    }
    ret = hmac_tx_encap_etc(hmac_vap, hmac_user, netbuf);
    if (OAL_UNLIKELY((ret != OAL_SUCC))) {
        oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_lan_mpdu_process_sta::hmac_tx_encap_etc failed[%d].}", ret);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_80211_ENCAP_FAIL;
    }
    return HMAC_TX_PASS;
}

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
/*****************************************************************************
 函 数 名  : hmac_tx_proxyarp_is_en
 功能描述  : 检查proxyarp功能是否使能
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8  hmac_tx_proxyarp_is_en(hmac_vap_stru *hmac_vap)
{
    return (oal_bool_enum_uint8)((hmac_vap->hmac_vap_proxyarp != OAL_PTR_NULL) &&
        (hmac_vap->hmac_vap_proxyarp->is_proxyarp == OAL_TRUE));
}
#endif

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_lan_mpdu_process_unicast(hmac_vap_stru *hmac_vap, osal_u8 *addr,
    hmac_user_stru **hmac_user, mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
    osal_u16 user_idx;

    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, addr, &user_idx);
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    if (ret != OAL_SUCC) {
        ret = hmac_find_valid_user_by_wds_sta(hmac_vap, addr, &user_idx);
    }
#endif
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log4(0, OAM_SF_TX,
            "{hmac_tx_lan_mpdu_process_ap::hmac_tx_find_user failed %2x:%2x:%2x:%2x:xx:xx}",
            addr[0], addr[1], addr[2], addr[3]); /* 0 1 2 3:索引值 */
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_USER_UNKNOWN;
    }

    /* 转成HMAC的USER结构体 */
    *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (OAL_UNLIKELY(*hmac_user == OAL_PTR_NULL)) {
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_USER_NULL;
    }

    /* 用户状态判断 */
    if (OAL_UNLIKELY((*hmac_user)->user_asoc_state != MAC_USER_STATE_ASSOC)) {
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_USER_INACTIVE;
    }

    /* 目标user指针 */
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)(user_idx & 0x3F); /* 只有6bit保存user_idx */

    ret = hmac_tx_ucast_process_etc(hmac_vap, netbuf, *hmac_user, tx_ctl);
    if (OAL_UNLIKELY(ret != HMAC_TX_PASS)) {
        return ret;
    }
    return HMAC_TX_PASS;
}

OAL_STATIC osal_u32 hmac_tx_lan_mpdu_process_multicast(hmac_vap_stru *hmac_vap, const osal_u8 *addr,
    hmac_user_stru **hmac_user, mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_SNOOP_CONVERT);

    /* 设置组播标识位 */
    mac_get_cb_is_mcast(tx_ctl) = OAL_TRUE;

    /* 更新ACK策略 */
    mac_get_cb_ack_polacy(tx_ctl) = WLAN_TX_NO_ACK;

    /* 获取组播用户 */
    *hmac_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (OAL_UNLIKELY(*hmac_user == OAL_PTR_NULL)) {
        oam_warning_log2(0, OAM_SF_TX,
            "vap_id[%d] {hmac_tx_lan_mpdu_process_ap::get multi user failed[%d].}",
            hmac_vap->vap_id, hmac_vap->multi_user_idx);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_MUSER_NULL;
    }

    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)(hmac_vap->multi_user_idx & 0x3F); /* 只有6bit保存user_idx */
    mac_get_cb_wme_tid_type(tx_ctl)  = WLAN_TIDNO_BCAST;
    mac_get_cb_wme_ac_type(tx_ctl)   = wlan_wme_tid_to_ac(WLAN_TIDNO_BCAST);

    /* 组播转单播 */
    if (((!ETHER_IS_BROADCAST(addr)) && (hmac_vap->m2u != OAL_PTR_NULL)) &&
        ((ether_is_ipv4_multicast(addr) != 0) || (ETHER_IS_IPV6_MULTICAST(addr) != 0))) {
        if (fhook != OSAL_NULL) {
            ret = ((hmac_m2u_snoop_convert_cb)fhook)(hmac_vap, netbuf);
            if (ret != HMAC_TX_PASS) {
                return ret;
            }
        }
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
        /* 组播处理流程 */
        /* 配网STA链表创建 */
        hmac_m2u_adaptive_inspecting(hmac_vap, netbuf);
        ret = hmac_m2u_multicast_drop(hmac_vap, netbuf);
        if (ret != HMAC_TX_PASS) {
            return ret;
        }
#endif
    }

    return HMAC_TX_PASS;
}

/* 特性解耦 */
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
OAL_STATIC osal_void hmac_tx_lan_mpdu_process_ap_multi(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf)
{
    mac_ether_header_stru *ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf); /* 以太网头 */
    osal_u8 *addr = ether_hdr->ether_dhost;      /* 目的地址 */
    osal_u8 *src_addr = ether_hdr->ether_shost;      /* 源地址 */
    /*
     * （WDS需要支持广播报文4地址转发）下行方向，既按3地址发送广播报文，
     * 又给每个从节点按4地址发送一份广播报文
     */
    if ((hmac_vap->wds_table.wds_vap_mode == WDS_MODE_ROOTAP) && ETHER_IS_BROADCAST(addr)) {
        hmac_wds_node_ergodic(hmac_vap, src_addr, hmac_wds_tx_broadcast_pkt, (void *)netbuf);
    }
}
#endif

/*****************************************************************************
 函 数 名  : hmac_tx_mpdu_process
 功能描述  : 单个MPDU处理函数
 输入参数  : event－事件结构体
             hmac_vap－vap结构体
             netbuf－BUF结构体
             tx_ctl－CB结构体
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_lan_mpdu_process_ap(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    hmac_user_stru          *hmac_user = OAL_PTR_NULL;      /* 目标STA结构体 */
    mac_ether_header_stru   *ether_hdr = OAL_PTR_NULL; /* 以太网头 */
    osal_u8               *addr = OAL_PTR_NULL;      /* 目的地址 */
    osal_u32               ul_ret;

    /* 判断是组播或单播,对于lan to wlan的单播帧，返回以太网地址 */
    ether_hdr   = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    addr        = ether_hdr->ether_dhost;

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    /* 确认proxy arp 是否使能 */
    if (hmac_tx_proxyarp_is_en(hmac_vap) == OAL_TRUE && hmac_proxy_arp_proc(hmac_vap, netbuf) == OAL_TRUE) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_PROXY_ARP;
    }
#endif

    /* 单播数据帧 */
    if (OAL_LIKELY(ETHER_IS_MULTICAST(addr) == 0)) {
        ul_ret = hmac_tx_lan_mpdu_process_unicast(hmac_vap, addr, &hmac_user, tx_ctl, netbuf);
        if (ul_ret != HMAC_TX_PASS) {
            return ul_ret;
        }
    } else { /* 组播 or 广播 */
        ul_ret = hmac_tx_lan_mpdu_process_multicast(hmac_vap, addr, &hmac_user, tx_ctl, netbuf);
        oam_info_log4(0, OAM_SF_TX, "{hmac_tx_lan_mpdu_process_ap::dhost mac [%2x][%2x][%2x][%2x][xx][xx]}",
                      addr[0],  /*  目标mac的第0字节 */
                      addr[1],  /*  目标mac的第1字节 */
                      addr[2],  /*  目标mac的第2字节 */
                      addr[3]); /*  目标mac的第3字节 */
        if (ul_ret != HMAC_TX_PASS) {
            return ul_ret;
        }
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        hmac_tx_lan_mpdu_process_ap_multi(hmac_vap, netbuf);
#endif
    }
    if (mac_pk_mode(netbuf)) {
        /* pk模式提前填写描述符信息 */
        hmac_tx_fill_first_cb_info(tx_ctl, hmac_vap, hmac_user);
    }
    /* 封装802.11头 */
    ul_ret = hmac_tx_encap_etc(hmac_vap, hmac_user, netbuf);
    if (OAL_UNLIKELY((ul_ret != OAL_SUCC))) {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_lan_mpdu_process_ap::hmac_tx_encap_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return HMAC_TX_DROP_80211_ENCAP_FAIL;
    }

    return HMAC_TX_PASS;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_lan_to_wlan_send_event(hmac_vap_stru *hmac_vap, osal_u32 tx_action,
    oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    osal_u32 ret = OAL_FAIL;
    unref_param(tx_ctl);

    if (OAL_LIKELY(tx_action == HMAC_TX_PASS)) {
        ret = hmac_tx_send_encap_data(hmac_vap, netbuf);
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] {frw_send_data failed[%d]}",
                hmac_vap->vap_id, ret);
            OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        }
    } else if (OAL_UNLIKELY(tx_action == HMAC_TX_BUFF)) {
        ret = OAL_SUCC;
    } else if (tx_action == HMAC_TX_DONE) {
        ret = OAL_SUCC;
#ifdef _PRE_WLAN_FEATURE_HERA_MCAST
    } else if ((tx_action == HMAC_TX_DROP_NOSMART) || (tx_action == HMAC_TX_DROP_NOADAP)) {
        /* 组播报文不满足组播转发条件(非智能家居设备或非配网模式)，所以丢弃，属正常行为 */
        oam_info_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_lan_to_wlan_send_event::HMAC_TX_DROP.reason[%d]!}",
            hmac_vap->vap_id, tx_action);
#endif
    } else {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_lan_to_wlan_send_event::HMAC_TX_DROP.reason[%d]!}",
            hmac_vap->vap_id, tx_action);
    }

    return ret;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_lan_to_wlan_proc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    mac_tx_ctl_stru *tx_ctl)
{
    osal_u32 ret = HMAC_TX_PASS;
    osal_void *fhook_wapi = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_TX_LAN2WLAN);
    mac_tx_ctl_stru *tx_ctl_tmp = tx_ctl;

#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
    hmac_parse_special_ipv4_packet(hmac_vap, oal_netbuf_data(netbuf),
        oal_netbuf_get_len(netbuf), HMAC_PKT_DIRECTION_TX);
#endif

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /*  处理当前 MPDU */
        if (mac_mib_get_dot11_qos_option_implemented(hmac_vap) == OAL_FALSE) {
            mac_get_cb_wme_ac_type(tx_ctl_tmp)     = WLAN_WME_AC_VO;            /* AP模式 关WMM 入VO队列 */
            mac_get_cb_wme_tid_type(tx_ctl_tmp) =  wlan_wme_ac_to_tid(mac_get_cb_wme_ac_type(tx_ctl_tmp));
        }

        ret = hmac_tx_lan_mpdu_process_ap(hmac_vap, netbuf, tx_ctl_tmp);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 处理当前MPDU */
        mac_get_cb_wme_ac_type(tx_ctl_tmp) = WLAN_WME_AC_VO;                  /* STA模式 非qos帧入VO队列 */
        /* AC和tid队列要保持一致 */
        mac_get_cb_wme_tid_type(tx_ctl_tmp) =  wlan_wme_ac_to_tid(mac_get_cb_wme_ac_type(tx_ctl_tmp));
        ret = hmac_tx_lan_mpdu_process_sta(hmac_vap, netbuf, tx_ctl_tmp);
        if ((ret == HMAC_TX_PASS) && (fhook_wapi != OSAL_NULL)) {
            osal_u32 wapi_ret = ((hmac_tx_lan_to_wlan_wapi_cb)fhook_wapi)(hmac_vap, &netbuf);
            if (wapi_ret != OAL_CONTINUE) {
                return wapi_ret;
            }
            /*  由于wapi可能修改netbuff，此处需要重新获取一下cb */
            tx_ctl_tmp = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        }
    }
    return hmac_tx_lan_to_wlan_send_event(hmac_vap, ret, netbuf, tx_ctl_tmp);
}

/* 数据报文从lan发往wlan前，依据netbuf信息获取用户索引 */
OAL_STATIC WIFI_HMAC_TCM_TEXT osal_u16 hmac_tx_set_cb_user_idx(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    mac_tx_ctl_stru *tx_ctl)
{
    osal_u32 ret = OAL_FAIL;
    mac_ether_header_stru   *ether_hdr = OAL_PTR_NULL; /* 以太网头 */
    osal_u8               *addr = OAL_PTR_NULL;      /* 目的地址 */
    osal_u16 user_idx = MAC_INVALID_USER_ID;

    /* 判断是组播或单播,对于lan to wlan的单播帧，返回以太网地址 */
    ether_hdr   = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    addr        = ether_hdr->ether_dhost;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 单播数据帧 */
        if (OAL_LIKELY(ETHER_IS_MULTICAST(addr) == 0)) {
            ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, addr, &user_idx);
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
            if (ret != OAL_SUCC) {
                ret = hmac_find_valid_user_by_wds_sta(hmac_vap, addr, &user_idx);
            }
#endif
        } else { /* 组播 or 广播 */
            user_idx = hmac_vap->multi_user_idx;
        }
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        user_idx = hmac_vap->assoc_vap_id;
        unref_param(ret);
    }
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)(user_idx & 0x3F); /* 只有6bit保存user_idx */
    return user_idx;
}

WIFI_HMAC_TCM_TEXT OSAL_STATIC osal_bool hmac_vap_state_valid(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_TX,
            "{hmac_tx_lan_to_wlan_no_tcp_opt_etc::hmac_vap null.}");
        return OAL_FALSE;
    }

    /* VAP模式判断 */
    if (OAL_UNLIKELY((hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) &&
        (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA))) {
        oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_lan_to_wlan_no_tcp_opt_etc::vap_mode=%d.}",
            hmac_vap->vap_id, hmac_vap->vap_mode);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return OAL_FALSE;
    }

    /* 如果关联用户数量为0，则丢弃报文 */
    if (OAL_UNLIKELY(hmac_vap->user_nums == 0)) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_lan_to_wlan_no_tcp_opt_etc
 功能描述  : hmac AP模式 处理HOST DRX事件，注册到事件管理模块中
             PAYLOAD是一个NETBUF
 输入参数  : hmac_vap: vap指针
             netbuf: netbuf指针
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32  hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru       *tx_ctl = OAL_PTR_NULL;       /* SKB CB */
    osal_u8              data_type;
    osal_u16 user_idx;
    osal_u32 ret;
    hmac_user_stru *hmac_user = OSAL_NULL;

    if (!hmac_vap_state_valid(hmac_vap, netbuf)) {
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        return OAL_FAIL;
    }

    /* 此处数据可能从内核而来，也有可能由dev报上来再通过空口转出去，注意一下 */
    data_type =  hmac_get_data_type_from_8023_etc((osal_u8 *)oal_netbuf_data(netbuf), MAC_NETBUFF_PAYLOAD_ETH);
    /* 初始化CB tx rx字段 , CB字段在前面已经被清零， 在这里不需要重复对某些字段赋零值 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_get_cb_mpdu_num(tx_ctl)       = 1;
    mac_get_cb_netbuf_num(tx_ctl)     = 1;
    mac_get_cb_wlan_frame_type(tx_ctl)     = WLAN_DATA_BASICTYPE;
    if ((hmac_vap->tx_noack != OSAL_FALSE) && (OAL_NETBUF_LEN(netbuf) > THRUPUT_BYPASS_LEN)) {
        /* NOACK性能测试 */
        mac_get_cb_ack_polacy(tx_ctl)     = WLAN_TX_NO_ACK;
    } else {
        mac_get_cb_ack_polacy(tx_ctl)     = WLAN_TX_NORMAL_ACK;
    }
#ifdef _PRE_WLAN_ONLINE_IQ_CALI
    if (tx_ctl->online_iq_en == 1) {
        mac_get_cb_ack_polacy(tx_ctl)     = WLAN_TX_NO_ACK;     // no ack
    }
#endif
    mac_get_cb_tx_vap_index(tx_ctl)   = hmac_vap->vap_id;
    mac_get_cb_tx_user_idx(tx_ctl)    = MAC_INVALID_USER_ID;
    mac_get_cb_wme_ac_type(tx_ctl)    = WLAN_WME_AC_BE;  /* 初始化入BE队列 */
    mac_get_cb_frame_type(tx_ctl) = WLAN_CB_FRAME_TYPE_DATA;
    mac_get_cb_frame_subtype(tx_ctl) = data_type;

    /* 维测，输出一个关键帧打印 */
    if (!mac_pk_mode(netbuf) && data_type <= MAC_DATA_ARP_REQ) {
        oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d] {send datatype==%u,len==%u}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]",
            hmac_vap->vap_id, data_type, OAL_NETBUF_LEN(netbuf));
    }

    user_idx = hmac_tx_set_cb_user_idx(hmac_vap, netbuf, tx_ctl);
    hmac_user_use_cnt_inc(user_idx, &hmac_user);
    ret = hmac_tx_lan_to_wlan_proc(hmac_vap, netbuf, tx_ctl);
    hmac_user_use_cnt_dec(hmac_user);
    return ret;
}

#ifdef _PRE_WLAN_TCP_OPT
OAL_STATIC osal_u32 hmac_transfer_tx_handler(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru        *tx_ctl;
    osal_u32              ul_ret = OAL_SUCC;

#ifdef _PRE_WLAN_FEATURE_QUE_CLASSIFY
    if (oal_skb_get_queue_mapping(netbuf) == WLAN_TCP_ACK_QUEUE) {
#ifdef _PRE_WLAN_TCP_OPT_DEBUG
        oam_warning_log0(0, OAM_SF_TX, "{hmac_transfer_tx_handler::netbuf is tcp ack.}\r\n");
#endif
        oal_netbuf_list_tail(&hmac_vap->hmac_tcp_ack[HCC_TX].data_queue[HMAC_TCP_ACK_QUEUE], netbuf);
        /* 单纯TCP ACK等待调度, 特殊报文马上发送 */
        if (hmac_judge_tx_netbuf_is_tcp_ack_etc((oal_ether_header_stru *)oal_netbuf_data(netbuf))) {
            if (oal_netbuf_list_len(&hmac_vap->hmac_tcp_ack[HCC_TX].data_queue[HMAC_TCP_ACK_QUEUE]) >=  \
                g_st_tcp_ack_filter.tcp_ack_max_num_start_process) {
                hmac_tcp_ack_process_etc();
            } else {
                tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
                mac_get_cb_tcp_ack(tx_ctl) = OAL_TRUE;
            }
        } else {
            hmac_tcp_ack_process_etc();
        }
    } else {
        ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap, netbuf);
    }
#else
    ul_ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap, netbuf);
#endif
    return ul_ret;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_tx_lan_to_wlan_tcp_opt
 功能描述  : hmac AP模式 处理HOST DRX事件，注册到事件管理模块中
             PAYLOAD是一个NETBUF
 输入参数  : hmac_vap: vap指针
             netbuf: netbuf指针
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
WIFI_HMAC_TCM_TEXT INLINE__ osal_u32 hmac_tx_lan_to_wlan_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u32             ret;
#ifdef _PRE_WLAN_TCP_OPT
    hmac_device_stru      *hmac_device;
#endif
    /* TX方向: user编译选项下不打印，eng编译选项下打印 */
#if defined(_PRE_WLAN_WAKEUP_SRC_PARSE) && defined(PLATFORM_DEBUG_ENABLE)
    if (g_uc_print_data_wakeup_etc == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_TX, "tx: hmac_parse_packet_etc!");
        hmac_parse_packet_etc(netbuf);
        g_uc_print_data_wakeup_etc = OAL_FALSE;
    }
#endif

#ifdef _PRE_WLAN_TCP_OPT
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device->sys_tcp_tx_ack_opt_enable == OAL_TRUE) {
        ret = hmac_transfer_tx_handler(hmac_device, hmac_vap, netbuf);
    } else {
#endif
        ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap, netbuf);
#ifdef _PRE_WLAN_TCP_OPT
    }
#endif

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_vap_discard_pkt
 功能描述  : 判断VAP的状态，如果没有UP/PAUSE，则丢弃报文
 输入参数  : hmac_vap: hmac VAP结构体
            netbuf: SKB结构体,其中data指针指向以太网头
 返 回 值  : osal_u32
 调用函数  : hmac_bridge_vap_xmit_proc
 注意事项  : 内部函数, 不检验入参, 由调用函数保证有效性
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_bridge_vap_discard_pkt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    if (hmac_vap->al_tx_flag == OAL_SWITCH_ON) {
        oam_info_log1(0, OAM_SF_TX, "vap[%d] {hmac_bridge_vap_xmit_etc::the vap alway tx/rx!}", hmac_vap->vap_id);
        return OAL_SUCC;
    }
#endif
    /* 判断VAP的状态，如果ROAM，则丢弃报文 MAC_DATA_DHCP/MAC_DATA_ARP */
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        if (hmac_get_data_type_from_8023_etc((osal_u8 *)oal_netbuf_payload(netbuf), MAC_NETBUFF_PAYLOAD_ETH) !=
            MAC_DATA_EAPOL) {
            return OAL_SUCC;
        }
    } else {
#endif // _PRE_WLAN_FEATURE_ROAM
    /* 判断VAP的状态，如果没有UP/PAUSE，则丢弃报文 */
    if (OAL_UNLIKELY(!((hmac_vap->vap_state == MAC_VAP_STATE_UP) ||
        (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)))) {
        /* filter the tx xmit pkts print */
        if ((hmac_vap->vap_state == MAC_VAP_STATE_INIT) || (hmac_vap->vap_state == MAC_VAP_STATE_BUTT)) {
            oam_warning_log2(0, OAM_SF_TX,
                "vap_id[%d] {hmac_bridge_vap_discard_pkt::vap state[%d] != MAC_VAP_STATE_{UP|PAUSE}!}\r\n",
                hmac_vap->vap_id, hmac_vap->vap_state);
        } else {
            oam_info_log2(0, OAM_SF_TX,
                "vap_id[%d] {hmac_bridge_vap_discard_pkt::vap state[%d] != MAC_VAP_STATE_{UP|PAUSE}!}\r\n",
                hmac_vap->vap_id, hmac_vap->vap_state);
        }
        return OAL_SUCC;
    }
#ifdef _PRE_WLAN_FEATURE_ROAM
    }
#endif
    return OAL_CONTINUE;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 oal_netbuf_reserve_header_pk(oal_netbuf_stru *netbuf)
{
    osal_u32 header_len = oal_netbuf_headroom(netbuf);
    /* 此处保证snap开始时前面是80个字节 */
    header_len = header_len + (ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN);
    if (header_len < (osal_u32)OAL_HDR_TOTAL_LEN) {
        if (oal_netbuf_expand_head(netbuf, (td_s32)(OAL_HDR_TOTAL_LEN - header_len), 0, 0) != 0) {
            return OAL_NETBUF_FAIL;
        }
    }
    return OAL_NETBUF_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_bridge_vap_xmit_proc
 功能描述  : 挂接到VAP对应net_device结构体下的发送函数
 输入参数  : hmac_vap: hmac VAP结构体
            netbuf: SKB结构体,其中data指针指向以太网头
 返 回 值  : oal_net_dev_tx_enum
 调用函数  : hmac_bridge_vap_xmit_etc
 注意事项  : 内部函数, 不检验入参, 由调用函数保证有效性
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_bridge_vap_xmit_proc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
    osal_void *fhook = OSAL_NULL;

    if (mac_pk_mode(netbuf)) {
        if (oal_netbuf_reserve_header_pk(netbuf) != OAL_NETBUF_SUCC) {
            wifi_printf("{hmac_bridge_vap_xmit_proc reserve_header fail}\r\n");
            oal_netbuf_free(netbuf);
            return;
        }
        ret = hmac_tx_lan_to_wlan_no_tcp_opt_etc(hmac_vap, netbuf);
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oal_netbuf_free(netbuf);
        }
        return;
    }

    if (oal_netbuf_reserve_header(netbuf) != OAL_NETBUF_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        oam_warning_log0(0, OAM_SF_TX, "{hmac_bridge_vap_xmit_proc reserve_header fail}\r\n");
        oal_netbuf_free(netbuf);
        return;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_REPEATER_BRIDGE_TX_PROCESS);
    if (fhook != OSAL_NULL) {
        ((hmac_bridge_tx_process_cb)fhook)(netbuf, hmac_vap);
    }

    /* 发送方向的arp_req 统计和删ba的处理 */
    hmac_call_netbuf_hooks(&netbuf, hmac_vap, HMAC_FRAME_DATA_TX_BRIDGE_IN);

    ret = hmac_tx_lan_to_wlan_etc(hmac_vap, netbuf);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        /* 调用失败，要释放内核申请的netbuff内存池 */
        hmac_free_netbuf_list_etc(netbuf);
    }
}

/*****************************************************************************
 函 数 名  : hmac_bridge_vap_xmit_etc
 功能描述  : 挂接到VAP对应net_device结构体下的发送函数
 输入参数  : netbuf: SKB结构体,其中data指针指向以太网头
            dev: net_device结构体
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_s32 hmac_bridge_vap_xmit_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_netbuf_stru *netbuf = *((oal_netbuf_stru **)(msg->data));
    osal_u16 user_idx;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_LATENCY_STAT_TX_ENTRY);
    hmac_user_stru *hmac_user = OSAL_NULL;

    hmac_delay(1); /* 1：发送时延打点 */
    netbuf = oal_netbuf_unshare(netbuf, GFP_ATOMIC);
    if (OAL_UNLIKELY(netbuf == OAL_PTR_NULL)) {
        oam_info_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_bridge_vap_xmit_etc::netbuf NULL!}", hmac_vap->vap_id);
        return OAL_SUCC;
    }
    OAL_NETBUF_NEXT(netbuf) = OAL_PTR_NULL;
    OAL_NETBUF_PREV(netbuf) = OAL_PTR_NULL;
    (osal_void)memset_s(OAL_NETBUF_CB(netbuf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    if (hmac_is_thruput_enable(THRUPUT_HMAC_PK) != 0 && OAL_NETBUF_LEN(netbuf) > THRUPUT_BYPASS_LEN) {
        mac_get_cb_pk_mode((mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuf)) = OSAL_TRUE;
        hmac_bridge_vap_xmit_proc(hmac_vap, netbuf);
        hmac_delay(2); /* 2：发送时延打点 */
        return OAL_SUCC;
    }

    /* 获取VAP后,引用计数加1 */
    osal_adapt_atomic_inc(&(hmac_vap->use_cnt));
    /* user引用计数加1 */
    user_idx = hmac_tx_set_cb_user_idx(hmac_vap, netbuf, (mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuf));
    hmac_user_use_cnt_inc(user_idx, &hmac_user);

    if (hmac_bridge_vap_discard_pkt(hmac_vap, netbuf) != OAL_CONTINUE) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_bridge_vap_xmit_etc::vap_state invalid!}");
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        OAM_STAT_VAP_INCR(0, tx_abnormal_msdu_dropped, 1);
        oal_netbuf_free(netbuf);
        goto safe_del_vap;
    }

#ifdef _PRE_WLAN_DFT_STAT
    hmac_pkts_tx_stat(hmac_vap->vap_id, 1, TX_D_FROM_LAN_PKTS);
#endif
    /* 统计打点函数入口时延 */
    if (fhook != OSAL_NULL) {
        ((hmac_latency_stat_tx_entry_cb)fhook)(netbuf);
    }

    /* 将以太网过来的帧上报SDT */
    hmac_tx_report_eth_frame_etc(hmac_vap, netbuf);
    hmac_bridge_vap_xmit_proc(hmac_vap, netbuf);

safe_del_vap:
    hmac_user_use_cnt_dec(hmac_user);
    /* VAP使用结束自减，并判断是否为0，为0，则VAP进行安全删除 */
    hmac_config_safe_del_vap(hmac_vap);
    hmac_delay(2); /* 2：发送时延打点 */
    return OAL_SUCC;
}

oal_module_symbol(hmac_tx_lan_to_wlan_etc);
oal_module_symbol(hmac_free_netbuf_list_etc);

oal_module_symbol(hmac_tx_report_eth_frame_etc);
oal_module_symbol(hmac_bridge_vap_xmit_etc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
