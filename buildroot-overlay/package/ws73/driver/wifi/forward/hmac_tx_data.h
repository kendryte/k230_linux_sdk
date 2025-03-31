/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file for hmac_tx_data.c.
 * Create: 2021-12-15
 */

#ifndef __HMAC_TX_DATA_H__
#define __HMAC_TX_DATA_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_mgmt_classifier.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_DATA_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WLAN_BA_CNT_INTERVAL 100

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    HMAC_TX_BSS_NOQOS = 0,
    HMAC_TX_BSS_QOS   = 1,

    HMAC_TX_BSS_QOS_BUTT
} hmac_tx_bss_qos_type_enum;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u32          pkt_len;  /* HOST来帧量统计 */
    osal_u32          snd_pkts;  /* 驱动实际发送帧统计 */
    osal_u32          start_time; /* 均速统计开始时间 */
} hmac_tx_pkts_stat_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32 hmac_tx_ucast_process_etc(hmac_vap_stru   *hmac_vap,
    oal_netbuf_stru *netbuf,
    hmac_user_stru  *hmac_user,
    mac_tx_ctl_stru *tx_ctl);

extern osal_void  hmac_tx_ba_setup_etc(hmac_vap_stru  *hmac_vap,
    hmac_user_stru *hmac_user,
    osal_u8       tidno);

osal_void hmac_tx_set_vip_tid(mac_tx_ctl_stru *tx_ctl, osal_u8 type, osal_u8 subtype,
    osal_u8 is_retry, osal_u8 *tid_out);
extern oal_bool_enum_uint8 hmac_tid_need_ba_session(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, osal_u8 tidno, oal_netbuf_stru *netbuf);

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_tx_get_next_mpdu
 功能描述  : 获取下一个MPDU
 输入参数  : buf netbuf结构体;netbuf_num netbuf个数
 调用函数  : hmac_tx_event_process_ap
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_netbuf_stru *hmac_tx_get_next_mpdu(oal_netbuf_stru *buf, osal_u8 netbuf_num)
{
    oal_netbuf_stru       *next_buf = OAL_PTR_NULL;
    osal_u32             netbuf_index;

    if (OAL_UNLIKELY(buf == OAL_PTR_NULL)) {
        return OAL_PTR_NULL;
    }

    next_buf = buf;
    for (netbuf_index = 0; netbuf_index < netbuf_num; netbuf_index++) {
        next_buf = oal_netbuf_list_next(next_buf);
    }

    return next_buf;
}

/*****************************************************************************
 函 数 名  : hmac_tx_netbuf_list_enqueue
 功能描述  : MPDU入队
 输入参数  : head 表头; buf netbuf结构体;netbuf_num netbuf个数
 调用函数  : hmac_tx_event_process_ap
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_tx_netbuf_list_enqueue(oal_netbuf_head_stru *head, oal_netbuf_stru *buf,
    osal_u8 netbuf_num)
{
    osal_u32             netbuf_index;
    oal_netbuf_stru       *buf_tmp = buf;
    oal_netbuf_stru       *buf_next;

    if (OAL_UNLIKELY((head == OAL_PTR_NULL) || (buf_tmp == OAL_PTR_NULL))) {
        return;
    }

    for (netbuf_index = 0; netbuf_index < netbuf_num; netbuf_index++) {
        buf_next = oal_netbuf_list_next(buf_tmp);
        oal_netbuf_add_to_list_tail(buf_tmp, head);
        buf_tmp = buf_next;
    }
}


/*****************************************************************************
 函 数 名  : hmac_tx_get_addr
 功能描述  : 获取原地址和目的地址
 输入参数  : hdr 802.11头结构体
 输出参数  : saddr 原地址 daddr 目的地址
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_tx_get_addr(mac_ieee80211_qos_htc_frame_addr4_stru *hdr,
    osal_u8 *saddr,
    osal_u8 *daddr)
{
    osal_u8  to_ds;
    osal_u8  from_ds;

    to_ds   = mac_hdr_get_to_ds((osal_u8 *)hdr);
    from_ds = mac_hdr_get_from_ds((osal_u8 *)hdr);
    if ((to_ds == 1) && (from_ds == 0)) {
        /* to AP */
        oal_set_mac_addr(saddr, hdr->qos_frame_addr4.address2);
        oal_set_mac_addr(daddr, hdr->qos_frame_addr4.address3);
    } else if ((to_ds == 0) && (from_ds == 0)) {
        /* IBSS */
        oal_set_mac_addr(saddr, hdr->qos_frame_addr4.address2);
        oal_set_mac_addr(daddr, hdr->qos_frame_addr4.address1);
    } else if ((to_ds == 1) && (from_ds == 1)) {
        /* WDS */
        oal_set_mac_addr(saddr, hdr->qos_frame_addr4.address4);
        oal_set_mac_addr(daddr, hdr->qos_frame_addr4.address3);
    } else {
        /* From AP */
        oal_set_mac_addr(saddr, hdr->qos_frame_addr4.address3);
        oal_set_mac_addr(daddr, hdr->qos_frame_addr4.address1);
    }
}

OAL_STATIC OAL_INLINE osal_void hmac_tx_set_cb_header_len(osal_u32 qos, mac_tx_ctl_stru *tx_ctl)
{
    if (qos == HMAC_TX_BSS_QOS) {
        /* 更新帧头长度 */
        if (mac_get_cb_is_4address(tx_ctl) == OAL_FALSE) {
            mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_QOS_FRAME_LEN;
        } else {
            mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_QOS_4ADDR_FRAME_LEN;
        }
    } else {
        /* 非QOS数据帧帧控制字段设置 */
        if (mac_get_cb_is_4address(tx_ctl) != 0) {
            mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_4ADDR_FRAME_LEN;
        } else {
            mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_FRAME_LEN;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_tx_set_frame_ctrl
 功能描述  : 设置帧控制
 输入参数  : qos 是否是QOS站点 tx_ctl CB字段 hdr 802.11头
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_tx_set_frame_ctrl(osal_u32 qos, mac_tx_ctl_stru *tx_ctl,
    mac_ieee80211_qos_htc_frame_addr4_stru *hdr_addr4)
{
    mac_ieee80211_qos_htc_frame_stru *hdr = OAL_PTR_NULL;
    if (qos == HMAC_TX_BSS_QOS) {
        /* 设置帧控制字段 */
        mac_hdr_set_frame_control((osal_u8 *)hdr_addr4, (WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS));

        /* 更新帧头长度 */
        if (mac_get_cb_is_4address(tx_ctl) == OAL_FALSE) {
            hdr = (mac_ieee80211_qos_htc_frame_stru *)(uintptr_t)hdr_addr4;
            /* 设置QOS控制字段 */
            hdr->qc_tid        = mac_get_cb_wme_tid_type(tx_ctl);
            hdr->qc_eosp       = 0;
            hdr->qc_ack_polocy = mac_get_cb_ack_polacy(tx_ctl);
            hdr->qc_amsdu      = mac_get_cb_is_amsdu(tx_ctl);
            hdr->qos_control.qc_txop_limit = 0;
        } else {
            /* 设置QOS控制字段 */
            hdr_addr4->qos_frame_addr4.qc_tid        = mac_get_cb_wme_tid_type(tx_ctl);
            hdr_addr4->qos_frame_addr4.qc_eosp       = 0;
            hdr_addr4->qos_frame_addr4.qc_ack_polocy = mac_get_cb_ack_polacy(tx_ctl);
            hdr_addr4->qos_frame_addr4.qc_amsdu      = mac_get_cb_is_amsdu(tx_ctl);
            hdr_addr4->qos_frame_addr4.qos_control.qc_txop_limit = 0;
        }

        /* 由DMAC考虑是否需要HTC */
    } else {
        /* 设置帧控制字段 */
        mac_hdr_set_frame_control((osal_u8 *)hdr_addr4, WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_DATA);
    }
}

OAL_STATIC OAL_INLINE osal_void hmac_tx_set_frame_htc(mac_tx_ctl_stru *tx_ctl,
    mac_ieee80211_qos_htc_frame_addr4_stru *hdr_addr4)
{
    /* 更新帧头长度 */
    if (mac_get_cb_is_4address(tx_ctl) == OAL_FALSE) {
        mac_ieee80211_qos_htc_frame_stru *frame_hdr = (mac_ieee80211_qos_htc_frame_stru *)hdr_addr4;
        frame_hdr->frame_control.order = 1;
        mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_QOS_HTC_FRAME_LEN;
        frame_hdr->htc = HTC_INVALID_VALUE;
    } else {
        /* 设置QOS控制字段 */
        hdr_addr4->qos_frame_addr4.frame_control.order = 1;
        mac_get_cb_frame_header_length(tx_ctl) = MAC_80211_QOS_HTC_4ADDR_FRAME_LEN;
        hdr_addr4->htc = HTC_INVALID_VALUE;
    }
}

/*****************************************************************************
 函 数 名  : hmac_tid_ba_is_setup
 功能描述  : 判断该用户对应的TID是否已经建立BA会话
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_TRUE代表已经创建了BA会话
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tid_ba_is_setup(const hmac_user_stru *hmac_user,
    osal_u8 tidno)
{
    if (OAL_UNLIKELY(hmac_user == OAL_PTR_NULL || tidno >= WLAN_TID_MAX_NUM)) {
        return OAL_FALSE;
    }
    return (hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status ==
        HMAC_BA_COMPLETE) ? OAL_TRUE : OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_vap_ba_is_setup
 功能描述  : 判断该用户是否已经建立BA会话
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_TRUE代表已经创建了BA会话
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_vap_ba_is_setup(const hmac_user_stru *hmac_user)
{
    osal_u8 tidno;

    if (hmac_user == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    for (tidno = 0; tidno < WLAN_TID_MAX_NUM; tidno++) {
        if (hmac_user->tx_tid_queue[tidno].ba_tx_info.ba_status == HMAC_BA_COMPLETE) {
            return OAL_TRUE;
        }
    }

    return  OAL_FALSE;
}

OAL_STATIC OAL_INLINE osal_void hmac_tx_ba_del(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 tidno)
{
    mac_action_mgmt_args_stru       action_args;   /* 用于填写ACTION帧的参数 */

    action_args.category = MAC_ACTION_CATEGORY_BA;
    action_args.action   = MAC_BA_ACTION_DELBA;
    action_args.arg1     = tidno;
    action_args.arg2     = MAC_ORIGINATOR_DELBA;
    action_args.arg3     = MAC_UNSPEC_REASON;
    action_args.arg5    = hmac_user->user_mac_addr;
    hmac_mgmt_tx_action_etc(hmac_vap,  hmac_user, &action_args);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_tx_data.h */
