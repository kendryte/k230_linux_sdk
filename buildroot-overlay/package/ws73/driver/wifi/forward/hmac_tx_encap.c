/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: HMAC Tx Wi-Fi Data.
 * Create: 2022-04-14
 */

#include "hmac_tx_encap.h"
#include "common_dft.h"
#include "oal_net.h"
#include "oal_netbuf_data.h"
#include "mac_frame.h"
#include "mac_vap_ext.h"
#include "wlan_types_common.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_tx_data.h"
#include "hmac_11i.h"
#include "hmac_frag.h"
#include "frw_hmac.h"
#ifdef _PRE_WLAN_FEATURE_STAT
#include "hmac_stat.h"
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#include "hmac_tx_amsdu.h"
#endif
#include "hmac_feature_dft.h"
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif
#include "hmac_auto_adjust_freq.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_tx_mpdu_adapt.h"
#include "wlan_thruput_debug.h"
#include "hmac_feature_interface.h"
#include "hmac_dfx.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_TX_ENCAP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/* 设置非四地址的sta 地址 */
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_set_addresses_sta(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const mac_tx_ctl_stru *tx_ctl, mac_ether_header_stru *ether_hdr, mac_ieee80211_qos_htc_frame_addr4_stru *hdr)
{
    /* From DS标识位设置 */
    mac_hdr_set_from_ds((osal_u8 *)hdr, 0);

    /* to DS标识位设置 */
    mac_hdr_set_to_ds((osal_u8 *)hdr, 1);

    /* Set Address1 field in the WLAN Header with BSSID */
    oal_set_mac_addr(hdr->qos_frame_addr4.address1, hmac_user->user_mac_addr);

    if (ether_hdr->ether_type == oal_byteorder_host_to_net_uint16(ETHER_LLTD_TYPE)) {
        /* Set Address2 field in the WLAN Header with the source address */
        oal_set_mac_addr(hdr->qos_frame_addr4.address2, (osal_u8 *)ether_hdr->ether_shost);
    } else {
        /* Set Address2 field in the WLAN Header with the source address */
        oal_set_mac_addr(hdr->qos_frame_addr4.address2, mac_mib_get_station_id(hmac_vap));
    }

    if (mac_get_cb_is_amsdu(tx_ctl) != 0) { /* AMSDU情况，地址3填写BSSID */
        /* Set Address3 field in the WLAN Header with the BSSID */
        oal_set_mac_addr(hdr->qos_frame_addr4.address3, hmac_user->user_mac_addr);
    } else {
        /* Set Address3 field in the WLAN Header with the destination address */
        oal_set_mac_addr(hdr->qos_frame_addr4.address3, (osal_u8 *)ether_hdr->ether_dhost);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tx_set_addresses
 功能描述  : 设置帧地址控制
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_set_addresses(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_tx_ctl_stru *tx_ctl, mac_ether_header_stru *ether_hdr, mac_ieee80211_qos_htc_frame_addr4_stru  *hdr)
{
    /* 分片号置成0，后续分片特性需要重新赋值 */
    hdr->qos_frame_addr4.frag_num    = 0;
    hdr->qos_frame_addr4.seq_num     = 0;

    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (mac_get_cb_is_4address(tx_ctl) == 0)) { /* From AP */
        /* From DS标识位设置 */
        mac_hdr_set_from_ds((osal_u8 *)hdr, 1);

        /* to DS标识位设置 */
        mac_hdr_set_to_ds((osal_u8 *)hdr, 0);

        /* Set Address1 field in the WLAN Header with destination address */
        oal_set_mac_addr(hdr->qos_frame_addr4.address1, (osal_u8 *)ether_hdr->ether_dhost);

        /* Set Address2 field in the WLAN Header with the BSSID */
        oal_set_mac_addr(hdr->qos_frame_addr4.address2, hmac_vap->bssid);

        if (mac_get_cb_is_amsdu(tx_ctl) != 0) { /* AMSDU情况，地址3填写BSSID */
            /* Set Address3 field in the WLAN Header with the BSSID */
            oal_set_mac_addr(hdr->qos_frame_addr4.address3, hmac_vap->bssid);
        } else {
            /* Set Address3 field in the WLAN Header with the source address */
            oal_set_mac_addr(hdr->qos_frame_addr4.address3, (osal_u8 *)ether_hdr->ether_shost);
        }
    } else if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
               (mac_get_cb_is_4address(tx_ctl) == 0)) {
        return hmac_tx_set_addresses_sta(hmac_vap, hmac_user, tx_ctl, ether_hdr, hdr);
    } else if (mac_get_cb_is_4address(tx_ctl) != 0) { /* WDS */
        /* TO DS标识位设置 */
        mac_hdr_set_to_ds((osal_u8 *)hdr, 1);

        /* From DS标识位设置 */
        mac_hdr_set_from_ds((osal_u8 *)hdr, 1);

        /* 地址1是 RA */
        oal_set_mac_addr(hdr->qos_frame_addr4.address1, hmac_user->user_mac_addr);

        /* 地址2是 TA (当前只有BSSID) */
        oal_set_mac_addr(hdr->qos_frame_addr4.address2, mac_mib_get_station_id(hmac_vap));

        if (mac_get_cb_is_amsdu(tx_ctl) != 0) { /* AMSDU情况，地址3和地址4填写BSSID */
            if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
                /* 地址3是 BSSID */
                oal_set_mac_addr(hdr->qos_frame_addr4.address3, hmac_user->user_mac_addr);

                /* 地址4也是 BSSID */
                oal_set_mac_addr(hdr->qos_frame_addr4.address4, hmac_user->user_mac_addr);
            } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
                /* 地址3是 BSSID */
                oal_set_mac_addr(hdr->qos_frame_addr4.address3, mac_mib_get_station_id(hmac_vap));

                /* 地址4也是 BSSID */
                oal_set_mac_addr(hdr->qos_frame_addr4.address4, mac_mib_get_station_id(hmac_vap));
            }
        } else {
            /* 地址3是 DA */
            oal_set_mac_addr(hdr->qos_frame_addr4.address3, (osal_u8 *)ether_hdr->ether_dhost);

            /* 地址4是 SA */
            oal_set_mac_addr(hdr->qos_frame_addr4.address4, (osal_u8 *)ether_hdr->ether_shost);
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tx_encap_80211_hdr
 功能描述  : 802.11帧头封装 AP模式
 输入参数  : vap－vap结构体
             user－用户结构体
             netbuf－BUF结构体
             tx_ctl -- netbuf控制字段
             ether_hdr -- 以太网头结构指针
 返 回 值  : OAL_PTR_NULL 或者 802.11帧头指针
 调用函数  : hmac_tx_encap_etc
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_encap_80211_hdr(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl, mac_ether_header_stru *ether_hdr)
{
    osal_u8 *hdr_802_11 = OAL_PTR_NULL; /* 802.11头 */
    osal_u32 qos = HMAC_TX_BSS_NOQOS;
    osal_u32 ret = OAL_SUCC;
    mac_ieee80211_frame_stru *frame_head = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_FRAG_START);

    /* 非组播帧，获取用户的QOS能力位信息 */
    if (mac_get_cb_is_mcast(tx_ctl) == OAL_FALSE) {
        /* 根据用户结构体的cap_info，判断是否是QOS站点 */
        qos = hmac_user->cap_info.qos;
        mac_set_cb_is_qos_data(tx_ctl, qos);
    }
    hmac_tx_set_cb_header_len(qos, tx_ctl);

    /* 如果skb中data指针前预留的空间大于802.11 mac head len，则不需要格外申请内存存放802.11头 */
    if (oal_netbuf_headroom(netbuf) >= MAC_80211_QOS_HTC_4ADDR_FRAME_LEN) {
        /* frame_header_length长度在hmac_tx_set_frame_ctrl配置 */
        if (mac_pk_mode(netbuf)) {
            /* 此处直接将802.11头指向MAC header，省去后面调整 */
            hdr_802_11 = (OAL_NETBUF_HEADER(netbuf) - OAL_MAX_MAC_HDR_LEN);
        } else {
            hdr_802_11 = (OAL_NETBUF_HEADER(netbuf) - mac_get_cb_frame_header_length(tx_ctl));
        }
    } else {
        oam_error_log1(0, OAM_SF_TX,
            "vap_id[%d] {hmac_tx_encap_etc::hdr null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 设置帧控制 */
    hmac_tx_set_frame_ctrl(qos, tx_ctl, (mac_ieee80211_qos_htc_frame_addr4_stru *)hdr_802_11);
    /* 设置地址 */
    hmac_tx_set_addresses(hmac_vap, hmac_user, tx_ctl, ether_hdr, (mac_ieee80211_qos_htc_frame_addr4_stru *)hdr_802_11);

    /* 挂接802.11头 */
    frame_head = (mac_ieee80211_frame_stru *)hdr_802_11;
    mac_get_cb_frame_header_addr(tx_ctl) = frame_head;

    /* 分片处理 */
    if (!mac_pk_mode(netbuf) && fhook != OSAL_NULL) {
        ret = ((hmac_frag_start_cb)fhook)(hmac_vap, hmac_user, netbuf, tx_ctl);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_tx_encap_etc
 功能描述  : 802.11帧头封装 AP模式
 输入参数  : hmac_vap－vap结构体
             user－用户结构体
             buf－BUF结构体
 输出参数  : ret_hdr－返回的头部
 返 回 值  : OAL_PTR_NULL 或者 802.11帧头指针
 调用函数  : AMSDU模块以及本文件
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_tx_encap_etc(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    osal_u16 mpdu_len;
    mac_ether_header_stru ether_hdr;
    errno_t rc;

    /* 获取CB */
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(netbuf));
    osal_u8 buf_is_amsdu = mac_get_cb_is_amsdu(tx_ctl);

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    if ((hmac_vap->wds_table.wds_vap_mode == WDS_MODE_REPEATER_STA)
        || ((hmac_vap->wds_table.wds_vap_mode == WDS_MODE_ROOTAP) && (hmac_user->is_wds == OAL_TRUE))) {
        mac_get_cb_is_4address(tx_ctl) = OAL_TRUE;
    }
#endif

    /* 获取以太网头, 原地址，目的地址, 以太网类型 */
    rc = memcpy_s(&ether_hdr, sizeof(ether_hdr), oal_netbuf_data(netbuf), ETHER_HDR_LEN);
    if (rc != EOK) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_tx_encap_etc::memcpy_s failed[%d].}", hmac_vap->vap_id, rc);
        return OAL_FAIL;
    }

    /* 非amsdu帧 */
    if (buf_is_amsdu == OAL_TRUE) {
        ether_hdr.ether_type = 0;
    } else {
        /* len = EHTER HEAD LEN + PAYLOAD LEN */
        mpdu_len = (osal_u16)oal_netbuf_get_len(netbuf);

        /* 更新frame长度，指向skb payload--LLC HEAD */
        mac_get_cb_mpdu_len(tx_ctl) = (mpdu_len - ETHER_HDR_LEN + SNAP_LLC_FRAME_LEN);

        tx_ctl->align_padding_offset = 0;
        mac_get_cb_msdu_num(tx_ctl) = 1;    /* 非AMSDU报文包含的MSDU数目为1 */

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AMSDU_TX_ENCAP_LARGE_SKB);
        if (fhook != OSAL_NULL) {
            ((hmac_tx_encap_large_skb_amsdu_cb)fhook)(hmac_vap, hmac_user, netbuf, tx_ctl);
        }
        if (mac_get_cb_has_ether_head(tx_ctl)) {
            /* 恢复data指针到ETHER HEAD - LLC HEAD */
            oal_netbuf_pull(netbuf, SNAP_LLC_FRAME_LEN);
        }
#endif
        /* 设置LLC HEAD */
        mac_set_snap(netbuf, ether_hdr.ether_type);

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        if (mac_get_cb_has_ether_head(tx_ctl)) {
            /* 恢复data指针到ETHER HEAD */
            oal_netbuf_push(netbuf, ETHER_HDR_LEN);
            /* 保证4bytes对齐 */
            if ((osal_ulong)oal_netbuf_data(netbuf) != OAL_ROUND_DOWN((osal_ulong)oal_netbuf_data(netbuf), 4)) {
                tx_ctl->align_padding_offset = (osal_ulong)oal_netbuf_data(netbuf) -
                    OAL_ROUND_DOWN((osal_ulong)oal_netbuf_data(netbuf), 4); /* 4:长度 */
                oal_netbuf_push(netbuf, tx_ctl->align_padding_offset);
            }
        }
#endif
    }

    return hmac_tx_encap_80211_hdr(hmac_vap, hmac_user, netbuf, tx_ctl, &ether_hdr);
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_process_encap_data(hmac_vap_stru *hmac_vap,
    mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf)
{
    osal_void *fhook = OSAL_NULL;
    if (mac_pk_mode(netbuf)) {
        if (hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, netbuf) != OAL_SUCC) {
            oal_netbuf_free(netbuf);
        }
        return OAL_SUCC;
    }
    /* 发包处理 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_AUTO_FREQ_FRAME_COUNT);
    if (fhook != OSAL_NULL) {
        ((hmac_auto_freq_pps_count_cb)fhook)(1);
    }
#ifdef _PRE_WLAN_FEATURE_STAT
    hmac_stat_device_tx_netbuf(OAL_NETBUF_LEN(netbuf));
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    /* 如果是AMSDU帧类型,需要去掉SKB起始的ETHER HEAD */
    if (tx_ctl->ether_head_including == OSAL_TRUE) {
        tx_ctl->mpdu_payload_len -= (ETHER_HDR_LEN + tx_ctl->align_padding_offset);
    }
#endif

    /* 维测，输出一个关键帧打印 */
    if (mac_get_cb_is_vipframe(tx_ctl) && (tx_ctl->frame_subtype <= MAC_DATA_ARP_REQ)) {
        oam_warning_log3(0, OAM_SF_TX,
            "{hmac_tx_process_data_event::datatype=%u,qos=%u,usridx=%d}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]",
            tx_ctl->frame_subtype, mac_get_is_qos_data(tx_ctl), tx_ctl->tx_user_idx);
    }

    if (hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, netbuf) != OAL_SUCC) {
        oal_netbuf_free(netbuf);
    }

    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_u32 hmac_tx_send_encap_data(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u32 ret = OAL_FAIL;
    oal_netbuf_stru *current_netbuf = netbuf;
    oal_netbuf_stru *current_netbuf_tmp = OAL_PTR_NULL;
    mac_tx_ctl_stru *tx_ctl;

    while (current_netbuf != OAL_PTR_NULL) {
        current_netbuf_tmp = current_netbuf;
        current_netbuf = OAL_NETBUF_NEXT(current_netbuf);
        tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(current_netbuf_tmp));
        OAL_NETBUF_NEXT(current_netbuf_tmp) = OSAL_NULL;

        if (!mac_pk_mode(current_netbuf_tmp)) {
            oal_netbuf_push(current_netbuf_tmp, mac_get_cb_frame_header_length(tx_ctl));
            ret = frw_hcc_tx_adjust_header(current_netbuf_tmp, HCC_MSG_TYPE_DATA_FRAME);
            if (OAL_UNLIKELY(ret != OAL_SUCC)) {
                oam_warning_log2(0, OAM_SF_TX, "hmac_hcc_tx_adjust_header[%u] drop:%u!", HCC_MSG_TYPE_DATA_FRAME, ret);
                return OAL_FAIL;
            }
            mac_get_cb_mpdu_len(tx_ctl) = (osal_u16)(OAL_NETBUF_LEN(current_netbuf_tmp)) -
                mac_get_cb_frame_header_length(tx_ctl);
        } else {
            mac_get_cb_mpdu_len(tx_ctl) =  (osal_u16)(OAL_NETBUF_LEN(current_netbuf_tmp));
        }

        ret = hmac_tx_process_encap_data(hmac_vap, tx_ctl, current_netbuf_tmp);
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_TX, "hmac_tx_process_encap_data[%u] drop:%u!", HCC_MSG_TYPE_DATA_FRAME, ret);
        }
    }
    return OAL_SUCC;
}

osal_u32 hmac_tx_send_encap_mgmt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u32 ret = OAL_FAIL;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(netbuf));

    ret = frw_hcc_tx_adjust_header(netbuf, HCC_MSG_TYPE_MGMT_FRAME);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_TX, "hmac_hcc_tx_adjust_header[%u] drop:%u!", HCC_MSG_TYPE_MGMT_FRAME, ret);
        return OAL_FAIL;
    }

    mac_get_cb_mpdu_len(tx_ctl) = (osal_u16)(OAL_NETBUF_LEN(netbuf)) - mac_get_cb_frame_header_length(tx_ctl);
    return hmac_tx_process_mgmt_event(hmac_vap, netbuf);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
