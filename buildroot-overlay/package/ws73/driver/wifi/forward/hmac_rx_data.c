/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Source file defined by the common operation function of
   the received frame and the operation function of the data frame of the DMAC module.
 * Create: 2021-12-15
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_rx_data.h"
#include "hmac_rx_data_feature.h"

#include "oal_ext_if.h"
#include "oam_struct.h"
#include "oam_ext_if.h"
#include "oal_netbuf_ext.h"
#include "mac_frame.h"
#include "mac_data.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_vap.h"
#include "hmac_ext_if.h"
#include "hmac_frag.h"
#include "hmac_11i.h"
#include "mac_vap_ext.h"
#include "common_dft.h"

#ifdef _PRE_WLAN_FEATURE_ISOLATION
#include "hmac_isolation.h"
#endif
#include "hmac_m2u.h"

#include "hmac_blockack.h"
#include "hmac_tcp_opt.h"
#include "hmac_wapi.h"

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif

#include "hmac_btcoex.h"
#include "hmac_latency_stat.h"
#include "hmac_feature_interface.h"

#ifdef _PRE_WLAN_FEATURE_WFA_SUPPORT
#include "hmac_sigma_traffic_stat.h"
#endif

#include "hmac_feature_dft.h"
#include "wlan_thruput_debug.h"
#include "hmac_dfx.h"
#include "hmac_single_proxysta.h"
#include "hmac_feature_interface.h"
#include "hmac_promisc.h"
#include "common_log_dbg_rom.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_RX_DATA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_rx_frame_80211_to_eth
 功能描述  : 将MSDU转化为以太网格式的帧
 输入参数  : pst_netbuf : 指向含有MSDU的netbuf的指针
             da     : 目的地址
             sa     : 源地址
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_frame_80211_to_eth(oal_netbuf_stru *pst_netbuf, osal_u8 *da,
    oal_size_t da_size, osal_u8 *sa, oal_size_t sa_size)
{
    mac_ether_header_stru              *ether_hdr;
    mac_llc_snap_stru                  *snap;
    osal_u16                          ether_type;
    unref_param(da_size);
    unref_param(sa_size);

    snap = (mac_llc_snap_stru *)oal_netbuf_data(pst_netbuf);
    ether_type = snap->ether_type;

    /* 将payload向前扩充6个字节，加上后面8个字节的snap头空间，构成以太网头的14字节空间 */
    oal_netbuf_push(pst_netbuf, HMAC_RX_DATA_ETHER_OFFSET_LENGTH);
    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_netbuf);

    ether_hdr->ether_type = ether_type;
    oal_set_mac_addr(ether_hdr->ether_shost, sa);
    oal_set_mac_addr(ether_hdr->ether_dhost, da);
}

/*****************************************************************************
 函 数 名  : hmac_rx_free_netbuf_list_etc
 功能描述  : 释放指定个数的netbuf
 输入参数  : (1)期望删除的netbuf的起始指针
             (2)需要删除的netbuf的个数
 返 回 值  : 成功或者失败原因
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rx_free_netbuf_etc(oal_netbuf_stru *pst_netbuf, osal_u16 nums)
{
    oal_netbuf_stru    *netbuf_temp;
    osal_u16          us_netbuf_num;
    mac_rx_ctl_stru    *pst_rx_ctrl;                        /* 指向MPDU控制块信息的指针 */
    osal_u32            msdu_cnt = 0;
    oal_netbuf_stru    *netbuf_curr = pst_netbuf;

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_etc::pst_netbuf null.}\r\n");
        return 0;
    }

    for (us_netbuf_num = nums; us_netbuf_num > 0; us_netbuf_num--) {
        netbuf_temp = OAL_NETBUF_NEXT(netbuf_curr);
        /* 获取该MPDU的控制信息 */
        pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf_curr);
        if (pst_rx_ctrl != OAL_PTR_NULL) {
            /* 情况一:不是AMSDU聚合，则该MPDU对应一个MSDU，同时对应一个NETBUF */
            if (pst_rx_ctrl->amsdu_enable == OAL_FALSE) {
                msdu_cnt++;
            } else {
                /* 情况二:AMSDU聚合 */
                msdu_cnt += pst_rx_ctrl->msdu_in_buffer;
            }
        }

        /* 减少netbuf对应的user引用计数 */
        oal_netbuf_free(netbuf_curr);

        netbuf_curr = netbuf_temp;

        if (netbuf_curr == OAL_PTR_NULL) {
            if (OAL_UNLIKELY(us_netbuf_num != 1)) {
                oam_error_log2(0, OAM_SF_RX,
                    "{hmac_rx_free_netbuf_etc::pst_netbuf list broken, us_netbuf_num[%d]nums[%d].}",
                    us_netbuf_num, nums);
                return msdu_cnt;
            }

            break;
        }
    }
    return msdu_cnt;
}


/*****************************************************************************
 函 数 名  : hmac_rx_free_netbuf_list_etc
 功能描述  : for list
*****************************************************************************/
osal_void hmac_rx_free_netbuf_list_etc(oal_netbuf_head_stru *netbuf_hdr, osal_u16 nums_buf)
{
    oal_netbuf_stru   *pst_netbuf;
    osal_u16         idx;

    if (OAL_UNLIKELY(netbuf_hdr == OAL_PTR_NULL)) {
        oam_info_log0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list_etc::pst_netbuf null.}");
        return;
    }

    for (idx = nums_buf; idx > 0; idx--) {
        pst_netbuf = oal_netbuf_delist(netbuf_hdr);
        if (pst_netbuf != OAL_PTR_NULL) {
            oal_netbuf_free(pst_netbuf);
        } else {
            oam_info_log0(0, OAM_SF_RX, "{hmac_rx_free_netbuf_list_etc::pst_netbuf null.}");
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_rx_transmit_to_wlan
 功能描述  : 将数据帧发送到WLAN侧的接口函数，将一个netbuf链抛给发送流程，每个
             netbuf的内容都是一个以太网格式的MSDU
 输入参数  : (1)指向事件头的指针
             (2)指向需要发送的netbuf的第一个元素的指针
 返 回 值  : 成功或者失败原因
*****************************************************************************/
OAL_STATIC osal_u32  hmac_rx_transmit_to_wlan(osal_u8 vap_id,        oal_netbuf_head_stru *netbuf_head)
{
    oal_netbuf_stru            *pst_netbuf;         /* 从netbuf链上取下来的指向netbuf的指针 */
    osal_u32                  netbuf_num;
    osal_u32                  ul_ret;
    oal_netbuf_stru            *buf_tmp;        /* 暂存netbuf指针，用于while循环 */
    mac_tx_ctl_stru            *tx_ctl;
    hmac_vap_stru               *hmac_vap;

    if (OAL_UNLIKELY((netbuf_head == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_transmit_to_wlan::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取链头的net buffer */
    pst_netbuf = oal_netbuf_peek(netbuf_head);

    /* 获取mac vap 结构 */
    ul_ret = hmac_tx_get_mac_vap_etc(vap_id, &hmac_vap);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        netbuf_num = oal_netbuf_list_len(netbuf_head);
        hmac_rx_free_netbuf_etc(pst_netbuf, (osal_u16)netbuf_num);
        oam_warning_log3(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_transmit_to_wlan::failed[%d], free [%d] netbuffer.}",
            vap_id, ul_ret, netbuf_num);
        return ul_ret;
    }

    /* 循环处理每一个netbuf，按照以太网帧的方式处理 */
    while (pst_netbuf != OAL_PTR_NULL) {
        buf_tmp = OAL_NETBUF_NEXT(pst_netbuf);

        OAL_NETBUF_NEXT(pst_netbuf) = OAL_PTR_NULL;
        OAL_NETBUF_PREV(pst_netbuf) = OAL_PTR_NULL;

        tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);
        memset_s(tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));

        /* set the queue map id when wlan to wlan */
        oal_skb_set_queue_mapping(pst_netbuf, WLAN_NORMAL_QUEUE);

        ul_ret = hmac_tx_lan_to_wlan_etc(hmac_vap, pst_netbuf);
        /* 调用失败，自己调用自己释放netbuff内存 */
        if (ul_ret != OAL_SUCC) {
            hmac_free_netbuf_list_etc(pst_netbuf);
        }

        pst_netbuf = buf_tmp;
    }

    return OAL_SUCC;
}


OAL_STATIC osal_u32  hmac_rx_prepare_normal(hmac_vap_stru *hmac_vap,
    oal_netbuf_head_stru *netbuf_hdr,
    oal_netbuf_stru *netbuf)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    mac_rx_ctl_stru *rx_ctrl = OAL_PTR_NULL;
    osal_u8 *addr = OAL_PTR_NULL;
    osal_u8 sa[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 da[WLAN_MAC_ADDR_LEN] = {0};
    mac_ieee80211_frame_stru *frame_hdr = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_FRAG_DERAG_START);
    oal_netbuf_stru *rx_netbuf = netbuf;
    unref_param(hmac_vap);

    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(rx_netbuf);
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(mac_get_rx_cb_ta_user_idx(rx_ctrl));
    if (OAL_UNLIKELY(hmac_user == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (fhook != OSAL_NULL) {
        rx_netbuf = ((hmac_defrag_start_cb)fhook)(hmac_user, rx_netbuf, rx_ctrl->mac_header_len);
        if (rx_netbuf == OAL_PTR_NULL) {
            return OAL_SUCC;
        }
    }
    /* 重新获取该MPDU的控制信息 */
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(rx_netbuf);

    frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);

    /* 从MAC头中获取源地址和目的地址 */
    mac_rx_get_sa(frame_hdr, &addr);
    oal_set_mac_addr(sa, addr);

    mac_rx_get_da(frame_hdr, &addr);
    oal_set_mac_addr(da, addr);

    /* 将netbuf的data指针指向mac frame的payload处，也就是指向了8字节的snap头 */
    oal_netbuf_pull(rx_netbuf, rx_ctrl->mac_header_len);

    /* 将MSDU转化为以太网格式的帧 */
    hmac_rx_frame_80211_to_eth(rx_netbuf, da, sizeof(da), sa, sizeof(sa));

    memset_s(OAL_NETBUF_CB(rx_netbuf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    /* 将MSDU加入到netbuf链的最后 */
    oal_netbuf_add_to_list_tail(rx_netbuf, netbuf_hdr);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_prepare_msdu_list_to_wlan
 功能描述  : 解析MPDU，如果是非AMSDU，则将MSDU还原为以太网格式的帧，并加入到
             netbuf链的最后，如果该MPDU是AMSDU，则解析出每一个MSDU，并且每一
             个MSDU占用一个netbuf
 输入参数  : netbuf_header: 要交给发送流程的netbuf链表头
             pst_netbuf       : 当前要处理的MPDU的第一个netbuf
             frame_hdr    : 当前要处理的MPDU的MAC头
 返 回 值  : 成功或者错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_rx_prepare_msdu_list_to_wlan(hmac_vap_stru *hmac_vap,
    oal_netbuf_head_stru *netbuf_header,
    oal_netbuf_stru *pst_netbuf,
    mac_ieee80211_frame_stru *frame_hdr)
{
    mac_rx_ctl_stru *rx_ctrl = OAL_PTR_NULL; /* 指向MPDU控制块信息的指针 */
    hmac_msdu_stru msdu; /* 保存解析出来的每一个MSDU */
    mac_msdu_proc_status_enum_uint8 process_state = MAC_PROC_BUTT; /* 解析AMSDU的状态 */
    hmac_msdu_proc_state_stru msdu_state = {0}; /* 记录MPDU的处理信息 */
    osal_u32 ul_ret;
    unref_param(frame_hdr);

    if (OAL_UNLIKELY(pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_prepare_msdu_list_to_wlan::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取该MPDU的控制信息 */
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    memset_s(&msdu, OAL_SIZEOF(hmac_msdu_stru), 0, OAL_SIZEOF(hmac_msdu_stru));

    /* 情况一:不是AMSDU聚合，则该MPDU对应一个MSDU，同时对应一个NETBUF,将MSDU还原
       成以太网格式帧以后直接加入到netbuf链表最后
    */
    if (rx_ctrl->amsdu_enable == OAL_FALSE) {
        ul_ret = hmac_rx_prepare_normal(hmac_vap, netbuf_header, pst_netbuf);
    } else { /* 情况二:AMSDU聚合 */
        msdu_state.procd_netbuf_nums    = 0;
        msdu_state.procd_msdu_in_netbuf = 0;

        /* amsdu 最后一个netbuf next指针设为 NULL 出错时方便释放amsdu netbuf */
        hmac_rx_clear_amsdu_last_netbuf_pointer(pst_netbuf, rx_ctrl->buff_nums);

        do {
            /* 获取下一个要转发的msdu */
            ul_ret = hmac_rx_parse_amsdu_etc(pst_netbuf, &msdu, &msdu_state, &process_state);
            if (ul_ret != OAL_SUCC) {
                oam_warning_log2(0, OAM_SF_RX,
                    "vap_id[%d] {hmac_rx_prepare_msdu_list_to_wlan::hmac_rx_parse_amsdu_etc failed[%d].}",
                    hmac_vap->vap_id, ul_ret);
                return ul_ret;
            }

            /* 将MSDU转化为以太网格式的帧 */
            hmac_rx_frame_80211_to_eth(msdu.netbuf, msdu.da, sizeof(msdu.da), msdu.sa, sizeof(msdu.sa));

            /* 将MSDU加入到netbuf链的最后 */
            oal_netbuf_add_to_list_tail(msdu.netbuf, netbuf_header);
        } while (process_state != MAC_PROC_LAST_MSDU);
    }

    return ul_ret;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_transmit_netbuf(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_net_device_stru *device, mac_ether_header_stru  *ether_hdr)
{
    osal_u32 ret = OAL_FAIL;

    /* 报文交OS入口数据包HOOK处理 */
    ret = hmac_call_netbuf_hooks(&netbuf, hmac_vap, HMAC_FRAME_DATA_RX_EVENT_H2O);
    if (ret != OAL_CONTINUE) {
        oal_netbuf_free(netbuf);
        return;
    }

    host_print_rx_timestamp(netbuf);

#ifdef _PRE_WLAN_FEATURE_WFA_SUPPORT
    hmac_rx_sigma_traffic_stat(ether_hdr);
#else
    unref_param(ether_hdr);
#endif

    /* 将skb转发给桥 */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (hmac_get_rxthread_enable_etc() == OAL_TRUE) {
        hmac_rxdata_netbuf_enqueue_etc(netbuf);
        hmac_rxdata_sched_etc();
#ifdef _PRE_WLAN_DFT_STAT
        /* 目前认为发送给内核的报文是成功的，后续解耦后需要深一步的统计 */
        hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_SEND_LAN_SUCC);
#endif
    } else {
#endif
        /* 将skb转发给桥 */
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
        netbuf->dev = device;
        /* 将skb的data指针指向以太网的帧头 */
        /* 由于前面pull了14字节，这个地方要push回去 */
        oal_netbuf_push(netbuf, ETHER_HDR_LEN);
#endif

#ifdef _PRE_WLAN_DFT_STAT
        if (oal_netif_rx(netbuf) == OAL_SUCC) {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_SEND_LAN_SUCC);
        } else {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_SEND_LAN_FAIL);
        }
#else
        (osal_void)oal_netif_rx(netbuf);
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    }
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
    /* 置位net_dev->jiffies变量 */
    OAL_NETDEVICE_LAST_RX(hmac_vap->net_device) = OAL_TIME_JIFFY;
#endif
#endif
    return;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_transmit_msdu_to_lan_stat(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf)
{
    unref_param(hmac_vap);
    unref_param(netbuf);
    /* 增加统计信息 */
    OAM_STAT_VAP_INCR(hmac_vap->vap_id, rx_pkt_to_lan, 1); /* 增加发往LAN的帧的数目 */
    OAM_STAT_VAP_INCR(hmac_vap->vap_id, rx_bytes_to_lan, OAL_NETBUF_LEN(netbuf)); /* 增加发送LAN的字节数 */
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_transmit_virtual_multi_sta_proc(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, hmac_msdu_stru *msdu, oal_netbuf_stru *netbuf)
{
    osal_void *fhook = OSAL_NULL;
    osal_void *m2u_fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_SNOOP_INSPECTING);
    unref_param(msdu);

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    oal_netbuf_stru *tmp = OAL_PTR_NULL;
    hmac_wds_stas_stru *sta = OAL_PTR_NULL;

    if (hmac_wds_find_sta(hmac_vap, msdu->sa, &sta) == OAL_SUCC) {
        tmp = oal_netbuf_copy(netbuf, GFP_ATOMIC); // 需检视验证
    }
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_REPEATER_BRIDGE_RX_PROCESS);
    if (fhook != OSAL_NULL) {
        ((hmac_bridge_rx_process_cb)fhook)(netbuf, hmac_vap);
    }

if (hmac_vap->m2u != OAL_PTR_NULL) {
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    if (hmac_wds_find_sta(hmac_vap, msdu->sa, &sta) == OAL_SUCC) {
        if (m2u_fhook != OSAL_NULL) {
            ((hmac_m2u_snoop_inspecting_cb)m2u_fhook)(hmac_vap, hmac_user, tmp);
        }
    } else {
        if (m2u_fhook != OSAL_NULL) {
            ((hmac_m2u_snoop_inspecting_cb)m2u_fhook)(hmac_vap, hmac_user, netbuf);
        }
    }

    if (tmp != OAL_PTR_NULL) {
        oal_netbuf_free(tmp);
    }
#else
    {
        if (m2u_fhook != OSAL_NULL) {
            ((hmac_m2u_snoop_inspecting_cb)m2u_fhook)(hmac_vap, hmac_user, netbuf);
        }
    }
#endif
}
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_rx_transmit_msdu_to_lan_feature(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, hmac_msdu_stru *msdu, oal_netbuf_stru *netbuf)
{
    /* AP模式下支持防arp欺骗功能 */
#ifdef _PRE_WLAN_FEATURE_PREVENT_ARP_SPOOFING
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && hmac_rx_check_arp_spoofing(hmac_vap->net_device, netbuf))
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_transmit_msdu_to_lan_feature::detect ARP spoofing data .}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    if (ETHER_IS_MULTICAST(msdu->da) == OAL_FALSE && hmac_btcoex_get_user_info(hmac_user) != OSAL_NULL) {
        osal_adapt_atomic_inc(&hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_arp_req_process.rx_unicast_pkt_to_lan);
    }
#endif
    unref_param(hmac_user);
    unref_param(msdu);
    hmac_rx_transmit_msdu_to_lan_stat(hmac_vap, netbuf);
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_transmit_debug(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (OAL_TRUE == g_uc_print_data_wakeup_etc) {
        oam_warning_log0(0, OAM_SF_RX, "rx: hmac_parse_packet_etc!");
        hmac_parse_packet_etc(netbuf);
        g_uc_print_data_wakeup_etc = OAL_FALSE;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
    hmac_parse_special_ipv4_packet(hmac_vap, oal_netbuf_data(netbuf),
        oal_netbuf_get_len(netbuf), HMAC_PKT_DIRECTION_RX);
#endif
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_rx_frame_80211_to_eth_promis(oal_netbuf_stru *netbuf, osal_u8 *da,
    oal_size_t da_size, osal_u8 *sa, oal_size_t sa_size)
{
    mac_ether_header_stru *ether_hdr = OAL_PTR_NULL;
    unref_param(da_size);
    unref_param(sa_size);

    /* 需要预留以太网头的14字节空间 */
    if (oal_netbuf_headroom(netbuf) < 14) {
        oam_warning_log1(0, 0, "headroom %d is not enough, return\r\n", oal_netbuf_headroom(netbuf));
        return OAL_FAIL;
    }
    /* 将payload向前扩充14个字节，构成以太网头的14字节空间 */
    oal_netbuf_push(netbuf, 14);
    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf);

    ether_hdr->ether_type = 0x0800;
    oal_set_mac_addr(ether_hdr->ether_shost, sa);
    oal_set_mac_addr(ether_hdr->ether_dhost, da);

    return OAL_SUCC;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* 混杂模式收到组播广播数据帧时 在netbuf hook中上报内核 */
WIFI_HMAC_TCM_TEXT osal_void hmac_rx_transmit_msdu_to_lan_promis(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf)
{
    mac_ether_header_stru *ether_hdr = OAL_PTR_NULL;
    osal_u8 *addr = OAL_PTR_NULL;
    osal_u8 da[ETHER_ADDR_LEN] = {0};
    osal_u8 sa[ETHER_ADDR_LEN] = {0};
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;

    if (netbuf == NULL) {
        oam_warning_log0(0, 0, "netbuf is null");
        return;
    }
    if ((netbuf->dev == NULL) || (hmac_vap == NULL)) {
        oal_netbuf_free(netbuf);
        return;
    }

    OAL_NETBUF_PREV(netbuf) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(netbuf) = OAL_PTR_NULL;

    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);

    /* 获取源地址和目的地址 */
    mac_rx_get_sa(frame_hdr, &addr);
    oal_set_mac_addr(sa, addr);

    mac_rx_get_da(frame_hdr, &addr);
    oal_set_mac_addr(da, addr);

    /* 报文格式 802.3 header + 802.11 header + qos data ctrl + data, 目前客户用的是qos data */
    if (hmac_rx_frame_80211_to_eth_promis(netbuf, da, sizeof(da), sa, sizeof(sa)) != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        return;
    }

    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    if (OAL_UNLIKELY(ether_hdr == OAL_PTR_NULL)) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oal_netbuf_free(netbuf);
        oam_warning_log0(0, 0, "ether_hdr NULL");
        return;
    }

    memset_s(OAL_NETBUF_CB(netbuf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    OAL_NETBUF_PROTOCOL(netbuf) = oal_eth_type_trans(netbuf, netbuf->dev);
    hmac_rx_transmit_netbuf(hmac_vap, netbuf, netbuf->dev, ether_hdr);

    return;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_rx_transmit_msdu_to_lan
 功能描述  : 将MSDU转发到LAN的接口，包括地址转换等信息的设置
             说明:本函数接收到的netbuf数据域是从snap头开始
 输入参数  : (1)指向vap的指针
             (2)指向需要发送的msdu的指针
 返 回 值  : 成功或者失败原因
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_transmit_msdu_to_lan(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_msdu_stru *msdu)
{
    oal_netbuf_stru *netbuf = msdu->netbuf;
    mac_ether_header_stru *ether_hdr = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_M2U_UNICAST_CONVERT_MULTICAST);
#ifdef _PRE_WLAN_DFT_STAT
    mac_rx_ctl_stru *rx_info = OAL_PTR_NULL;
    unref_param(rx_info);
#endif
    OAL_NETBUF_PREV(netbuf) = OAL_PTR_NULL;
    OAL_NETBUF_NEXT(netbuf) = OAL_PTR_NULL;
    /* STA进行单播转组播处理 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && (ETHER_IS_MULTICAST(msdu->da) == 0)) {
        if (fhook != OSAL_NULL) {
            ((hmac_m2u_unicast_convert_multicast_cb)fhook)(hmac_vap, netbuf, msdu);
        }
    }

    hmac_rx_frame_80211_to_eth(netbuf, msdu->da, sizeof(msdu->da), msdu->sa, sizeof(msdu->sa));
    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(netbuf);
    if (OAL_UNLIKELY(ether_hdr == OAL_PTR_NULL)) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oal_netbuf_free(netbuf);
        common_log_dbg1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_transmit_msdu_to_lan::hdr null.}", hmac_vap->vap_id);
        return;
    }

    hmac_rx_transmit_debug(hmac_vap, netbuf);

    if (hmac_11i_ether_type_filter_etc(hmac_vap, (const hmac_user_stru *)hmac_user, ether_hdr->ether_type) !=
            OAL_SUCC) {
        /* 接收安全数据过滤 */
        /* ether_type为0导致企业级加密失败,添加维测信息看是否报文解析异常 */
        dft_report_eth_frame_etc((osal_u8 *)ether_hdr, (osal_u16)OAL_NETBUF_LEN(netbuf),
            SOC_DIAG_MSG_ID_WIFI_RX_ETH_FRAME);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oal_netbuf_free(netbuf);
        return;
    }

    hmac_rx_transmit_virtual_multi_sta_proc(hmac_vap, hmac_user, msdu, netbuf);

    /* 对protocol模式赋值 */
    OAL_NETBUF_PROTOCOL(netbuf) = oal_eth_type_trans(netbuf, hmac_vap->net_device);

    if (hmac_rx_transmit_msdu_to_lan_feature(hmac_vap, hmac_user, msdu, netbuf) != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        oal_netbuf_free(netbuf);
        return;
    }

    hmac_rx_transmit_netbuf(hmac_vap, netbuf, hmac_vap->net_device, ether_hdr);
}

/* 将MSDU转发到LAN */
WIFI_HMAC_TCM_TEXT osal_void hmac_rx_lan_frame_classify_transmit_msdu_to_lan(hmac_vap_stru *hmac_vap,
    hmac_msdu_stru *msdu, oal_netbuf_head_stru *w2w_netbuf_hdr, hmac_user_stru *hmac_user)
{
    osal_u32 ul_ret;

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 当msdu.da在vap下,进行wlan_to_wlan转发,防止to_lan到内核不进行转发 */
        osal_u16 user_dix = MAC_INVALID_USER_ID;
        ul_ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, msdu->da, &user_dix);
        if (ul_ret == OAL_SUCC) {
            /* 将MSDU转化为以太网格式的帧 */
            hmac_rx_frame_80211_to_eth(msdu->netbuf, msdu->da, sizeof(msdu->da), msdu->sa, sizeof(msdu->sa));
            /* 将MSDU加入到netbuf链的最后 */
            oal_netbuf_add_to_list_tail(msdu->netbuf, w2w_netbuf_hdr);
        } else {
            /* 将MSDU转发到LAN */
            hmac_rx_transmit_msdu_to_lan(hmac_vap, hmac_user, msdu);
        }
    } else {
        /* 将MSDU转发到LAN */
        hmac_rx_transmit_msdu_to_lan(hmac_vap, hmac_user, msdu);
    }
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_rx_lan_frame_classify_normal(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, mac_ieee80211_frame_stru *frame_hdr, hmac_user_stru * hmac_user,
    oal_netbuf_head_stru *w2w_netbuf_hdr)
{
    osal_u8 *addr = OAL_PTR_NULL;
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    osal_u8 datatype;
    hmac_msdu_stru msdu;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_FRAG_DERAG_START);
    osal_void *fhook_wapi = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_RX_FRAME);
    oal_netbuf_stru *rx_netbuf = netbuf;
    mac_ieee80211_frame_stru *rx_frame_hdr = OAL_PTR_NULL;

    memset_s(&msdu, OAL_SIZEOF(hmac_msdu_stru), 0, OAL_SIZEOF(hmac_msdu_stru));
    mac_get_transmit_addr(frame_hdr, &addr);
    oal_set_mac_addr(msdu.ta, addr);

    if (fhook_wapi != OSAL_NULL) {
        rx_netbuf = ((hmac_rx_frame_wapi_cb)fhook_wapi)(hmac_vap, rx_netbuf, frame_hdr, hmac_user, &rx_ctrl);
        if (rx_netbuf == OAL_PTR_NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    if (fhook != OSAL_NULL) {
        rx_netbuf = ((hmac_defrag_start_cb)fhook)(hmac_user, rx_netbuf, rx_ctrl->mac_header_len);
        if (rx_netbuf == OAL_PTR_NULL) {
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    /* 重新获取该MPDU的控制信息 */
    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(rx_netbuf);
    /* 去分片有可能释放netbuf，需要重新获取frame hdr */
    rx_frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);

    /* 打印出关键帧(dhcp)信息 */
    datatype = hmac_get_data_type_from_80211_etc(rx_netbuf, rx_ctrl->mac_header_len);
    if (datatype <= MAC_DATA_ARP_RSP) {
        oam_warning_log4(0, OAM_SF_RX, "{hmac_rx_lan_frame_classify_normal::datatype==%u,user[%d],len==%u,"
            " rx_drop_cnt==%u}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]",
            datatype, mac_get_rx_cb_ta_user_idx(rx_ctrl), rx_ctrl->frame_len, hmac_user->rx_pkt_drop);
    }

#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_DATA_FRAME_RX) != 0) {
        if (datatype <= MAC_DATA_ARP_RSP) {
            wifi_printf("{[SERVICE]hmac_rx_lan_frame_classify_normal::user[%d], datatype==%u, len==%u,"
                        " rx_drop_cnt==%u}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]\r\n",
                        mac_get_rx_cb_ta_user_idx(rx_ctrl), datatype, rx_ctrl->frame_len, hmac_user->rx_pkt_drop);
        }
    }
#endif

    /* 对当前的msdu进行赋值 */
    msdu.netbuf    = rx_netbuf;

    /* 获取源地址和目的地址 */
    mac_rx_get_sa(rx_frame_hdr, &addr);
    oal_set_mac_addr(msdu.sa, addr);

    mac_rx_get_da(rx_frame_hdr, &addr);
    oal_set_mac_addr(msdu.da, addr);

    /* 将netbuf的data指针指向mac frame的payload处 */
    oal_netbuf_pull(rx_netbuf, rx_ctrl->mac_header_len);

    hmac_rx_lan_frame_classify_transmit_msdu_to_lan(hmac_vap, &msdu, w2w_netbuf_hdr, hmac_user);

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_rx_transmit_ap(hmac_vap_stru *hmac_vap, oal_netbuf_head_stru *w2w_netbuf_hdr)
{
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return;
    }
     /* 当msdu.da在vap下,进行wlan_to_wlan转发,防止to_lan到内核不进行转发 */
    /*  将MSDU链表交给发送流程处理 */
    if ((oal_netbuf_list_empty(w2w_netbuf_hdr) == OAL_FALSE) &&
        (oal_netbuf_tail(w2w_netbuf_hdr) != OAL_PTR_NULL) &&
        (oal_netbuf_peek(w2w_netbuf_hdr) != OAL_PTR_NULL)) {
        OAL_NETBUF_NEXT((oal_netbuf_tail(w2w_netbuf_hdr))) = OAL_PTR_NULL;
        OAL_NETBUF_PREV((oal_netbuf_peek(w2w_netbuf_hdr))) = OAL_PTR_NULL;

        hmac_rx_transmit_to_wlan(hmac_vap->vap_id, w2w_netbuf_hdr);
    }
}

/*****************************************************************************
 函 数 名  : hmac_rx_lan_frame_classify_etc
 功能描述  : HMAC接收模块，WLAN到LAN的转发接口
                           内核无路由表，从LAN过来的数据包不会转发出去。增加MSDU DA
                           的判断,当DA在当前VAP下,为防止转发丢包直接通过WLAN转发,不在VAP下调用LAN发送。
                           对于组播这里只进行LAN转发，其WLAN转发在其他流程中已进行。
 输入参数  : (1)对应MPDU的第一个netbuf的指针
             (2)对应的MPDU占用的netbuf的数目
 返 回 值  : 成功或者失败原因
*****************************************************************************/
OAL_STATIC WIFI_HMAC_TCM_TEXT osal_void hmac_rx_lan_frame_classify_etc(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *pst_netbuf, mac_ieee80211_frame_stru *frame_hdr)
{
    mac_rx_ctl_stru *pst_rx_ctrl;                        /* 指向MPDU控制块信息的指针 */
    osal_u32 ret;
    hmac_user_stru *hmac_user;
    oal_netbuf_head_stru w2w_netbuf_hdr;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL) || (frame_hdr == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_lan_frame_classify_etc::params null.}");
        return;
    }

    /* 获取该MPDU的控制信息 */
    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(mac_get_rx_cb_ta_user_idx(pst_rx_ctrl));
    if (OAL_UNLIKELY(hmac_user == OAL_PTR_NULL)) {
        /* 打印此net buf相关信息 */
        oam_error_log4(0, OAM_SF_RX,
            "{hmac_rx_lan_frame_classify_etc::info in cb, vap id=%d mac_hdr_len=%d,"
            " frame_len=%d mac_hdr_start_addr=0x%08x.}",
            pst_rx_ctrl->vap_id, pst_rx_ctrl->mac_header_len, pst_rx_ctrl->frame_len,
            (uintptr_t)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl));
        oam_error_log4(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_lan_frame_classify_etc::user_idx=%d,net_buf ptr addr=0x%08x, cb ptr addr=0x%08x.}",
            hmac_vap->vap_id, mac_get_rx_cb_ta_user_idx(pst_rx_ctrl),
            (uintptr_t)pst_netbuf, (uintptr_t)pst_rx_ctrl);
        return;
    }

    oal_netbuf_list_head_init(&w2w_netbuf_hdr);

    /* 组播帧不应进入接收聚合流程 */
    if (ether_is_multicast(frame_hdr->address1) == OSAL_FALSE) {
        hmac_ba_update_rx_bitmap(hmac_user, frame_hdr);
    }

    /* 情况一:不是AMSDU聚合，则该MPDU对应一个MSDU，同时对应一个NETBUF */
    if (pst_rx_ctrl->amsdu_enable == OAL_FALSE) {
        ret = hmac_rx_lan_frame_classify_normal(hmac_vap, pst_netbuf, frame_hdr, hmac_user, &w2w_netbuf_hdr);
    } else {
        /* 情况二:AMSDU聚合 */
        ret = hmac_rx_lan_frame_classify_amsdu(hmac_vap, pst_netbuf, frame_hdr, hmac_user, &w2w_netbuf_hdr);
    }

    if (ret == OAL_SUCC) {
        hmac_rx_transmit_ap(hmac_vap, &w2w_netbuf_hdr);
    }
}

/*****************************************************************************
 函 数 名  : hmac_rx_copy_netbuff_etc
*****************************************************************************/
OAL_STATIC osal_u32 hmac_rx_copy_netbuff_etc(oal_netbuf_stru **ppst_dest_netbuf, oal_netbuf_stru *src_netbuf,
    osal_u8 vap_id, mac_ieee80211_frame_stru **mac_hdr_start_addr)
{
    errno_t ret;

    if (src_netbuf == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *ppst_dest_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, OAL_NETBUF_LEN(src_netbuf), OAL_NETBUF_PRIORITY_MID);
    if (OAL_UNLIKELY(*ppst_dest_netbuf == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_copy_netbuff_etc::netbuf_copy null.}", vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 信息复制 */
    ret = memcpy_s(oal_netbuf_cb(*ppst_dest_netbuf), OAL_SIZEOF(mac_rx_ctl_stru), oal_netbuf_cb(src_netbuf),
                   OAL_SIZEOF(mac_rx_ctl_stru));
    if (ret != EOK) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_copy_netbuff_etc::memcpy_s cb fail.}", vap_id);
        oal_netbuf_free(*ppst_dest_netbuf);
        return OAL_FAIL;
    }
    ret = memcpy_s(oal_netbuf_data(*ppst_dest_netbuf), OAL_NETBUF_LEN(src_netbuf), oal_netbuf_data(src_netbuf),
                   OAL_NETBUF_LEN(src_netbuf));
    if (ret != EOK) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_copy_netbuff_etc::memcpy_s data fail.}", vap_id);
        oal_netbuf_free(*ppst_dest_netbuf);
        return OAL_FAIL;
    }
    /* 设置netbuf长度、TAIL指针 */
    oal_netbuf_put(*ppst_dest_netbuf, oal_netbuf_get_len(src_netbuf));

    *mac_hdr_start_addr = (mac_ieee80211_frame_stru *)oal_netbuf_data(*ppst_dest_netbuf);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_AMPDU
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_rx_process_data_filter_ampdu(mac_rx_ctl_stru *rx_ctrl,
    oal_netbuf_head_stru *netbuf_hdr, oal_bool_enum_uint8 *is_ba_buf)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    hmac_vap_stru  *hmac_vap = OAL_PTR_NULL;
    osal_u32 ul_ret;

    // make sure ta user idx is exist
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(mac_get_rx_cb_ta_user_idx(rx_ctrl));
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ROAM,
            "{hmac_rx_process_data_filter_etc::hmac_user[%d] null.}", mac_get_rx_cb_ta_user_idx(rx_ctrl));
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(rx_ctrl->mac_vap_id);
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_rx_process_data_filter_ampdu::mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_ba_filter_serv_etc(hmac_vap, hmac_user, rx_ctrl, netbuf_hdr, is_ba_buf);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_rx_process_data_filter_etc
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_process_data_filter_etc(oal_netbuf_head_stru *netbuf_header,
    oal_netbuf_stru *temp_netbuf, osal_u16 us_netbuf_num)
{
    oal_netbuf_stru                    *pst_netbuf;
    mac_rx_ctl_stru                    *rx_ctrl;
    osal_u8                             buf_nums, vap_id, index;
    oal_bool_enum_uint8                 is_ba_buf;
    mac_ieee80211_frame_stru           *frame_hdr = OSAL_NULL;
    osal_u16 netbuf_num = us_netbuf_num;

    while (netbuf_num != 0) {
        is_ba_buf = OAL_FALSE;
        pst_netbuf  = temp_netbuf;
        if (pst_netbuf == OAL_PTR_NULL) {
            break;
        }

        rx_ctrl   = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        buf_nums   = rx_ctrl->buff_nums;
        frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);
        if (osal_unlikely(frame_hdr == OSAL_NULL)) {
            break;
        }

        /* 获取下一个要处理的MPDU */
        oal_netbuf_get_appointed_netbuf(pst_netbuf, buf_nums, &temp_netbuf);
        netbuf_num = OAL_SUB(netbuf_num, buf_nums);

        vap_id = rx_ctrl->mac_vap_id;

        /* 双芯片下，0和1都是配置vap id，因此这里需要采用业务vap 其实id和整板最大vap mac num值来做判断 */
        if (vap_id == 0 || vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
            hmac_rx_free_netbuf_list_etc(netbuf_header, buf_nums);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_AMPDU
        /* 组播包不应进入聚合接收流程 */
        if (hmac_rx_process_data_filter_ampdu(rx_ctrl, netbuf_header, &is_ba_buf) != OAL_SUCC) {
            hmac_rx_free_netbuf_list_etc(netbuf_header, buf_nums);
            continue;
        }
#endif
        if (is_ba_buf == OAL_TRUE) {
            continue;
        }

        /* 如果buff不进reorder队列，则重新挂到链表尾，保序 */
        for (index = 0; index < buf_nums; index++) {
            pst_netbuf = oal_netbuf_delist(netbuf_header);
            if (OAL_LIKELY(pst_netbuf != OAL_PTR_NULL)) {
                oal_netbuf_add_to_list_tail(pst_netbuf, netbuf_header);
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_rx_lan_frame_etc
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_rx_lan_frame_etc(oal_netbuf_head_stru *netbuf_header)
{
    osal_u32                 netbuf_num;
    oal_netbuf_stru           *temp_netbuf;
    oal_netbuf_stru           *pst_netbuf;
    osal_u8                  buf_nums;
    mac_rx_ctl_stru           *pst_rx_ctrl;
    mac_ieee80211_frame_stru  *frame_hdr;
    hmac_vap_stru             *hmac_vap;
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    osal_u8                 *mac_addr;
#endif

    netbuf_num   = oal_netbuf_get_buf_num(netbuf_header);
    temp_netbuf = oal_netbuf_peek(netbuf_header);

    while (netbuf_num != 0) {
        pst_netbuf = temp_netbuf;
        if (pst_netbuf == NULL) {
            break;
        }
        pst_rx_ctrl   = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);
        buf_nums = pst_rx_ctrl->buff_nums;

        oal_netbuf_get_appointed_netbuf(pst_netbuf, buf_nums, &temp_netbuf);
        netbuf_num = OAL_SUB(netbuf_num, buf_nums);

        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_rx_ctrl->mac_vap_id);
        if (hmac_vap == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_RX, "{hmac_rx_lan_frame_etc::mac_res_get_hmac_vap null. vap_id:%u}",
                           pst_rx_ctrl->mac_vap_id);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        mac_rx_get_sa(frame_hdr, &mac_addr);
        /* Also we should drop multicast pkt whose sa belond to lan side(like LanCpe/br0) */
        if ((hmac_vap->wds_table.wds_vap_mode == WDS_MODE_REPEATER_STA) &&
            (hmac_wds_neigh_not_expired(hmac_vap, mac_addr) == OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_RX,
                "vap_id[%d] {hmac_rx_lan_frame_etc::free %d netbuf up to netdevice.}",
                pst_rx_ctrl->mac_vap_id, buf_nums);
            hmac_rx_free_netbuf_etc(pst_netbuf, (osal_u16)buf_nums);
            continue;
        }
#endif

        hmac_rx_lan_frame_classify_etc(hmac_vap, pst_netbuf, frame_hdr);
    }

    return OAL_SUCC;
}

static inline osal_bool oal_netbuf_is_da_multicast(mac_ieee80211_frame_stru *frame_hdr)
{
    osal_u8 *da = OSAL_NULL;

    mac_rx_get_da(frame_hdr, &da);

    return ETHER_IS_MULTICAST(da);
}

OAL_STATIC osal_void hmac_rx_process_multicast_ap(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_netbuf_head_stru *w2w_netbuf_hdr)
{
    oal_netbuf_stru *netbuf_copy = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr_copy = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);

    oam_info_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_multicast_ap::group frame.}", hmac_vap->vap_id);
    OAM_STAT_VAP_INCR(hmac_vap->vap_id, rx_mcast_cnt, 1);

    if (hmac_rx_copy_netbuff_etc(&netbuf_copy, netbuf, rx_ctrl->mac_vap_id, &frame_hdr_copy) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_multicast_ap::copy fail.}", hmac_vap->vap_id);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, rx_no_buff_dropped, 1);
        return;
    }
    hmac_rx_lan_frame_classify_etc(hmac_vap, netbuf, frame_hdr);

    /* 将MPDU解析成单个MSDU，把所有的MSDU组成一个netbuf链 */
    hmac_rx_prepare_msdu_list_to_wlan(hmac_vap, w2w_netbuf_hdr, netbuf_copy, frame_hdr_copy);
}

OAL_STATIC osal_u32 hmac_rx_process_wlan_to_lan_ap(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u16 *user_idx)
{
    osal_u32 ret = OAL_FAIL;
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);
    osal_u8 *da = OSAL_NULL;

    /* 四地址AMSDU报文不进行WLAN_TO_WLAN转发 */
    if ((mac_is_4addr((osal_u8 *)frame_hdr) == OAL_FALSE) ||
        ((mac_is_4addr((osal_u8 *)frame_hdr) == OAL_TRUE) && (rx_ctrl->amsdu_enable == OAL_FALSE))) {
        /* 获取目的地址对应的用户指针 */
        mac_rx_get_da(frame_hdr, &da);
        ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, da, user_idx);
        if (ret == OAL_ERR_CODE_PTR_NULL) { /* 查找用户失败 */
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, rx_ctrl->buff_nums, netbuf);
            /* 释放当前处理的MPDU占用的netbuf */
            hmac_rx_free_netbuf_etc(netbuf, (osal_u16)rx_ctrl->buff_nums);
            OAM_STAT_VAP_INCR(hmac_vap->vap_id, rx_da_check_dropped, 1);
            return OAL_CONTINUE;
        }

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        if (ret != OAL_SUCC) {
            ret = hmac_find_valid_user_by_wds_sta(hmac_vap, da, user_idx);
        }
#endif
    }
    /* 没有找到对应的用户 */
    if (ret != OAL_SUCC) {
        /* 目的用户不在AP的用户表中，调用wlan_to_lan转发接口 */
        hmac_rx_lan_frame_classify_etc(hmac_vap, netbuf, frame_hdr);
        return OAL_CONTINUE;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rx_process_wlan_to_wlan_ap(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    oal_netbuf_head_stru *w2w_netbuf_hdr, osal_u16 user_idx)
{
    mac_rx_ctl_stru *rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);
    osal_u8 *da = OSAL_NULL;
    osal_u32 ret;

    /* 目的用户已在AP的用户表中，进行WLAN_TO_WLAN转发 */
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_da_user[%d] null.}", hmac_vap->vap_id, user_idx);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, rx_da_check_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, rx_ctrl->buff_nums, netbuf);
        hmac_rx_free_netbuf_etc(netbuf, (osal_u16)rx_ctrl->buff_nums);
        return OAL_CONTINUE;
    }

    if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {STA is not associated with ap.send DEAUTH,err code [%d]}",
            hmac_vap->vap_id, MAC_NOT_AUTHED);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, rx_da_check_dropped, 1);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, rx_ctrl->buff_nums, netbuf);
        hmac_rx_free_netbuf_etc(netbuf, (osal_u16)rx_ctrl->buff_nums);
        mac_rx_get_da(frame_hdr, &da);
        hmac_mgmt_send_deauth_frame_etc(hmac_vap, da, MAC_NOT_AUTHED, OAL_FALSE);
        return OAL_CONTINUE;
    }

    /* 将MPDU解析成单个MSDU，把所有的MSDU组成一个netbuf链 */
    ret = hmac_rx_prepare_msdu_list_to_wlan(hmac_vap, w2w_netbuf_hdr, netbuf, frame_hdr);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_wlan_to_wlan failed[%d].}", hmac_vap->vap_id, ret);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_process_data_ap_tcp_ack_opt_etc
 功能描述  : AP模式下，HMAC模块接收WLAN_DRX事件(数据帧)的处理函数
 输入参数  : 事件结构体指针
 返 回 值  : 成功或者失败原因
*****************************************************************************/
osal_void hmac_rx_process_data_ap_tcp_ack_opt_etc(oal_netbuf_head_stru *netbuf_header)
{
    mac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u16 netbuf_num = (osal_u16)oal_netbuf_get_buf_num(netbuf_header);
    oal_netbuf_stru *temp_netbuf = oal_netbuf_peek(netbuf_header);
    oal_netbuf_stru *netbuf;
    oal_netbuf_head_stru w2w_netbuf_hdr;
    hmac_vap_stru *vap = OSAL_NULL;
    osal_u16 user_idx = MAC_INVALID_USER_ID;

    oal_netbuf_list_head_init(&w2w_netbuf_hdr);
    while (netbuf_num != 0 && temp_netbuf != OSAL_NULL) {
        netbuf = temp_netbuf;
        rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
        frame_hdr = (mac_ieee80211_frame_stru *)(uintptr_t)mac_get_rx_cb_mac_header_addr(rx_ctrl);
        oal_netbuf_get_appointed_netbuf(netbuf, rx_ctrl->buff_nums, &temp_netbuf);
        netbuf_num = OAL_SUB(netbuf_num, rx_ctrl->buff_nums);

        vap = (hmac_vap_stru *)mac_res_get_hmac_vap(rx_ctrl->mac_vap_id);
        if (OAL_UNLIKELY(vap == OSAL_NULL)) {
            hmac_rx_free_netbuf_etc(netbuf, rx_ctrl->buff_nums);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
        /* Virtual Multi-STA有自己的能力位方式，不通过4地址来识别 */
        if ((rx_ctrl->amsdu_enable == OAL_FALSE) && (vap->wds_table.wds_vap_mode == WDS_MODE_ROOTAP) &&
            hmac_vmsta_get_user_a4_support(vap, frame_hdr->address2)) {
            hmac_wds_update_table(vap, frame_hdr->address2,
                ((mac_ieee80211_qos_htc_frame_addr4_stru *)frame_hdr)->qos_frame_addr4.address4, WDS_TABLE_ADD_ENTRY);
        }
#endif
#ifdef _PRE_WLAN_FEATURE_ISOLATION
        if (hmac_isolation_filter_drop(vap, netbuf)) {
            hmac_rx_free_netbuf_etc(netbuf, rx_ctrl->buff_nums);
            continue;
        }
#endif
        /* 组播帧处理 */
        if (oal_netbuf_is_da_multicast(frame_hdr) == OSAL_TRUE) {
            hmac_rx_process_multicast_ap(vap, netbuf, &w2w_netbuf_hdr);
            continue;
        }

        /* 目的用户不在AP的用户表中，进行wlan_to_lan转发 */
        if (hmac_rx_process_wlan_to_lan_ap(vap, netbuf, &user_idx) == OAL_CONTINUE) {
            continue;
        }

        /* 目的用户已在AP的用户表中，进行WLAN_TO_WLAN转发 */
        if (hmac_rx_process_wlan_to_wlan_ap(vap, netbuf, &w2w_netbuf_hdr, user_idx) == OAL_CONTINUE) {
            continue;
        }
    }
    /*  将MSDU链表交给发送流程处理 */
    if ((oal_netbuf_list_empty(&w2w_netbuf_hdr) == OAL_FALSE) && (oal_netbuf_tail(&w2w_netbuf_hdr) != OSAL_NULL) &&
        (oal_netbuf_peek(&w2w_netbuf_hdr) != OSAL_NULL)) {
        OAL_NETBUF_NEXT((oal_netbuf_tail(&w2w_netbuf_hdr))) = OSAL_NULL;
        OAL_NETBUF_PREV((oal_netbuf_peek(&w2w_netbuf_hdr))) = OSAL_NULL;
        hmac_rx_transmit_to_wlan(vap->vap_id, &w2w_netbuf_hdr);
    }
}

/*****************************************************************************
 函 数 名  : hmac_rx_process_data_sta
 功能描述  : STA模式下，HMAC模块接收WLAN_DRX事件(数据帧)的处理函数
 输入参数  : 事件结构体指针
 返 回 值  : 成功或者失败原因
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_rx_process_data_sta_tcp_ack_opt_etc(hmac_vap_stru *hmac_vap,
    oal_netbuf_head_stru *netbuf_header)
{
    osal_u32 ret;
    /* 将需要上报的帧逐一出队处理 */
    ret = hmac_rx_lan_frame_etc(netbuf_header);
    if (ret != OAL_SUCC) {
        common_log_dbg1(0, OAM_SF_RX,
            "{hmac_rx_process_data_sta_tcp_ack_opt_etc::hmac_rx_lan_frame_etc failed[%d].}", ret);
    }
    unref_param(hmac_vap);
    return OAL_SUCC;
}
/**********************************************************
函数名: hmac_rx_process_bss_data_event
函数功能: 将所有netbuf加入链表，同时处理BSS组播数据报文
***********************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_rx_process_bss_data_event(oal_netbuf_head_stru *netbuf_header,
    oal_netbuf_stru *netbuf, hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device, osal_u16 *netbuf_num)
{
    oal_netbuf_stru                    *curr_netbuf = netbuf;
    oal_netbuf_stru                    *temp_netbuf = OSAL_NULL;
    osal_u16                          netbuf_num_tmp;              /* netbuf链表的个数 */
    netbuf_num_tmp = *netbuf_num;
    unref_param(hmac_vap);
    unref_param(hmac_device);

 /* 将所有netbuff全部入链表 */
    oal_netbuf_list_head_init(netbuf_header);
    while (netbuf_num_tmp != 0) {
        temp_netbuf = curr_netbuf;
        if (temp_netbuf == OAL_PTR_NULL) {
            break;
        }

        curr_netbuf = OAL_NETBUF_NEXT(temp_netbuf);
        oal_netbuf_add_to_list_tail(temp_netbuf, netbuf_header);
        netbuf_num_tmp--;
    }

    *netbuf_num = netbuf_num_tmp;
}

/*****************************************************************************
 函 数 名  : hmac_rx_process_ack
 功能描述  : HMAC模块接收WLAN_DRX事件(数据帧)的处理函数
 输入参数  : 事件结构体指针
 返 回 值  : 成功或者失败原因
************************ *****************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_rx_process_ack(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    osal_u16 netbuf_num, oal_netbuf_head_stru *netbuf_header)
{
#ifdef _PRE_WLAN_TCP_OPT
    hmac_device_stru *hmac_device = OSAL_NULL;
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_rx_process_data_event::hmac_device null.}");
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, netbuf_num, netbuf);
        hmac_rx_free_netbuf_etc(netbuf, netbuf_num);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (hmac_device->sys_tcp_rx_ack_opt_enable == OAL_TRUE) {
        hmac_transfer_rx_handle(hmac_device, hmac_vap, netbuf_header);
    }
#else
    unref_param(netbuf);
    unref_param(netbuf_num);
#endif

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_rx_process_data_sta_tcp_ack_opt_etc(hmac_vap, netbuf_header);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_rx_process_data_ap_tcp_ack_opt_etc(netbuf_header);
    } else {
        common_log_dbg1(0, OAM_SF_RX, "{hmac_rx_process_data_event::RX FAIL! VAP MODE[%d]!.}",
            hmac_vap->vap_mode);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_data
 功能描述  : HMAC模块接收WLAN_DRX事件(数据帧)的处理函数
 输入参数  : 事件结构体指针
 返 回 值  : 成功或者失败原因
************************ *****************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_rx_data(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u16                          netbuf_num = 1;                 /* 目前不支持组链，默认都是单帧 */
    oal_netbuf_head_stru                netbuf_header;              /* 存储上报给网络层的数据 */
    osal_u32                            ret;
    osal_u16                          netbuf_list_num;

    /* 获取VAP后,引用计数加1 */
    osal_adapt_atomic_inc(&(hmac_vap->use_cnt));

    /* If mib info is null ptr,release the netbuf */
    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        common_log_dbg0(0, OAM_SF_ANY, "{hmac_rx_process_data_event::pst_mib_info null.}");
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, netbuf_num, netbuf);
        hmac_rx_free_netbuf_etc(netbuf, netbuf_num);
        ret = OAL_SUCC;
        goto safe_del_vap;
    }

    /* 将所有netbuf加入链表。 混杂模式打开时，处理netbuf中BSS组播数据报文 */
    netbuf_list_num = netbuf_num;
    hmac_rx_process_bss_data_event(&netbuf_header, netbuf, hmac_vap, hmac_res_get_mac_dev_etc(0), &netbuf_list_num);
    if (osal_unlikely(netbuf_list_num != 0)) {
        oam_error_log1(0, OAM_SF_RX, "{hmac_rx_process_data_event:netbuf_num[%d].}", netbuf_list_num);
    }

    hmac_rx_process_data_filter_etc(&netbuf_header, netbuf, netbuf_num);

    /* 对接收的ack进行过滤 */
    ret = hmac_rx_process_ack(hmac_vap, netbuf, netbuf_num, &netbuf_header);

safe_del_vap :
    /* VAP使用结束自减，并判断是否为0，为0，则VAP进行安全删除 */
    hmac_config_safe_del_vap(hmac_vap);
    return ret;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
