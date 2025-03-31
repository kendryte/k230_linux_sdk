/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_tx_amsdu.c
 * 生成日期   : 2012年11月12日
 * 功能描述   : amsdu聚合
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_tx_amsdu.h"

#include "oam_struct.h"
#include "hmac_tx_data.h"
#include "hmac_feature_dft.h"
#include "hmac_tx_encap.h"
#include "hmac_ccpriv.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_TX_AMSDU_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
mac_llc_snap_stru g_st_mac_11c_snap_header = {
    SNAP_LLC_LSAP,
    SNAP_LLC_LSAP,
    LLC_UI,
    {
        SNAP_RFC1042_ORGCODE_0,
        SNAP_RFC1042_ORGCODE_1,
        SNAP_RFC1042_ORGCODE_2,
    },
    0
};

OAL_STATIC osal_u32  hmac_amsdu_tx_timeout_process(osal_void *p_arg);
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tx_amsdu_is_overflow(hmac_amsdu_stru     *amsdu,
    osal_u32          frame_len);
OAL_STATIC osal_u32  hmac_amsdu_send(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user,
    hmac_amsdu_stru *amsdu);

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_amsdu_prepare_to_send
 功能描述  : 发送一个amsdu帧
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC osal_void hmac_amsdu_prepare_to_send(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_amsdu_stru *amsdu)
{
    osal_u32              ul_ret;

    /* 删除定时器 */
    frw_destroy_timer_entry(&amsdu->amsdu_timer);
    ul_ret = hmac_amsdu_send(hmac_vap, hmac_user, amsdu);
    if (osal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_AMSDU,
            "{hmac_amsdu_prepare_to_send::, amsdu send fails. erro[%d], short_pkt_num=%d.}",
            ul_ret, amsdu->msdu_num);
    }
}

#if defined(_PRE_PRODUCT_ID_HOST)
/*****************************************************************************
 函 数 名  : hmac_tx_amsdu_is_overflow
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_tx_amsdu_is_overflow(hmac_amsdu_stru *amsdu,
    osal_u32 frame_len)

{
    /* payload + padmax(3) 不能大于1568 */
    if (((amsdu->amsdu_size + frame_len + SNAP_LLC_FRAME_LEN + 3) >
        (WLAN_LARGE_NETBUF_SIZE - MAC_80211_QOS_HTC_4ADDR_FRAME_LEN)) ||
        ((amsdu->amsdu_size + frame_len + SNAP_LLC_FRAME_LEN) > WLAN_AMSDU_FRAME_MAX_LEN)) {
        oam_info_log3(1, OAM_SF_TX,
            "{hmac_tx_amsdu_is_overflow::amsdu_size=%d framelen=%d msdu_num=%d .}",
            amsdu->amsdu_size, frame_len, amsdu->msdu_num);
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_amsdu_send
 功能描述  : 发送amsdu, 发送失败则释放buf
 输入参数  : user: 用户结构体指针
             amsdu: 要发送的amsdu
 返 回 值  : 成功OAL_SUCC；失败OAL_ERR_CODE_PTR_NULL
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC osal_u32 hmac_amsdu_send(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, hmac_amsdu_stru *amsdu)
{
    osal_u32          ret;
    mac_tx_ctl_stru    *cb;
    oal_netbuf_stru *net_buf = oal_netbuf_delist(&(amsdu->msdu_head));
    osal_u8 msdu_num;

    /* 给dmac传送的amsdu相关的信息以及802.11头挂接 */
    if (osal_unlikely(net_buf == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_AMPDU, "vap_id[%d] {hmac_amsdu_send::net_buf null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cb = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);

    /* amsdu只聚合一个帧时，回退成非amsdu，统一encap接口 */
    if (amsdu->msdu_num == 1) {
        osal_s32 mret;

        mret = memmove_s(OAL_NETBUF_DATA(net_buf) + SNAP_LLC_FRAME_LEN, OAL_MAC_ADDR_LEN + OAL_MAC_ADDR_LEN,
            OAL_NETBUF_DATA(net_buf), OAL_MAC_ADDR_LEN + OAL_MAC_ADDR_LEN);
        if (mret != EOK) {
            oal_netbuf_free(net_buf);
            oam_error_log1(0, OAM_SF_AMPDU, "vap_id[%d] {hmac_amsdu_send::memmove_s fail}", hmac_vap->vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        oal_netbuf_pull(net_buf, SNAP_LLC_FRAME_LEN);

        mac_get_cb_is_amsdu(cb) = OAL_FALSE;
        mac_get_cb_is_first_msdu(cb) = OAL_FALSE;
    }

    /* 把最后一个子帧的PAD去除 */
    oal_netbuf_trim(net_buf, amsdu->last_pad_len);

    mac_get_cb_mpdu_len(cb) = (osal_u16)OAL_NETBUF_LEN(net_buf);
    mac_get_cb_mpdu_num(cb) = 1;
    mac_get_cb_msdu_num(cb) = amsdu->msdu_num;

    /* 清零amsdu结构体信息 */
    msdu_num = amsdu->msdu_num;
    amsdu->amsdu_size = 0;
    amsdu->msdu_num   = 0;

    /* 为整个amsdu封装802.11头 */
    ret = hmac_tx_encap_etc(hmac_vap, hmac_user, net_buf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, msdu_num, OSAL_NULL);
        oal_netbuf_free(net_buf);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        oam_error_log2(0, OAM_SF_AMPDU, "vap_id[%d] {hmac_amsdu_send::tx_encap failed[%d]}", hmac_vap->vap_id, ret);
        return OAL_ERR_CODE_PTR_NULL;
    }

    // 架构变化导致Host侧发包路径变长，原bh锁的范围过长影响性能，将发送报文至dmac.ko侧的处理移至锁外
    osal_spin_unlock_bh(&amsdu->amsdu_lock);
    ret = hmac_tx_send_encap_data(hmac_vap, net_buf);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oal_netbuf_free(net_buf);
        OAM_STAT_VAP_INCR(hmac_vap->vap_id, tx_abnormal_msdu_dropped, 1);
        oam_warning_log2(0, OAM_SF_AMPDU, "vap_id[%d]hmac_amsdu_send::frw_send_data fail[%d]", hmac_vap->vap_id, ret);
    }

    osal_spin_lock_bh(&amsdu->amsdu_lock);

    return  ret;
}

/*****************************************************************************
 函 数 名  : hmac_amsdu_tx_encap_mpdu
 功能描述  : 多个msdu帧组建一个mpdu,进入此函数的msdu肯定能放入netbuf,否则overflow会溢出
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC OAL_INLINE osal_u32 hmac_amsdu_tx_encap_mpdu(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, hmac_amsdu_stru *amsdu, oal_netbuf_stru *buf)
{
    osal_u32              msdu_len;
    osal_u32              frame_len;
    osal_u32              tailroom;
    osal_u16              msdu_offset;    /* 拷贝新msdu帧的偏移地址 */
    oal_netbuf_stru        *dest_buf;
    mac_ether_header_stru  *ether_head;    /* 以太网过来的skb的以太网头 */
    mac_llc_snap_stru      *snap_head;     /* 为填写snap头的临时指针 */

    /* 协议栈来帧原始长 */
    frame_len = oal_netbuf_get_len(buf);
    /* 4字节对齐后的msdu帧的长度 */
    msdu_len = OAL_ROUND_UP(frame_len, 4);
    /* msdu帧长 */
    msdu_len += SNAP_LLC_FRAME_LEN;

    dest_buf = oal_netbuf_peek(&amsdu->msdu_head);
    if (dest_buf == OAL_PTR_NULL) {
        /* 链表中应该有netbuf */
        oam_error_log0(0, OAM_SF_AMSDU, "{hmac_amsdu_tx_encap_mpdu::oal_netbuf_peek return NULL}");
        return HMAC_TX_PASS;
    }

    /* 当期netbuf剩余空间少于msdu长 */
    tailroom = oal_netbuf_tailroom(dest_buf);
    if (tailroom < msdu_len) {
        oam_error_log3(0, OAM_SF_AMSDU, "{hmac_amsdu_tx_encap_mpdu::Notify1,tailroom[%d],msdu[%d],frame[%d]}",
                       tailroom, msdu_len, frame_len);
        /* 如果加上pad超出长度,尝试尾帧去掉pad */
        msdu_len = frame_len + SNAP_LLC_FRAME_LEN;
        if (tailroom < msdu_len) {
            hmac_amsdu_prepare_to_send(hmac_vap, hmac_user, amsdu);
            return HMAC_TX_PASS;
        }
    }

    /* NEW MSDU OFFSET */
    msdu_offset = (osal_u16)oal_netbuf_get_len(dest_buf);

    /* ETH HEAD + LLC + PAYLOAD */
    oal_netbuf_put(dest_buf, msdu_len);

    /* COPY ETH HEADER */
    ether_head = (mac_ether_header_stru *)(oal_netbuf_data(dest_buf) + msdu_offset);
    (osal_void)memcpy_s((osal_u8 *)ether_head, ETHER_HDR_LEN, oal_netbuf_data(buf), ETHER_HDR_LEN);

    /* FILL LLC HEADER */
    snap_head = (mac_llc_snap_stru *)((osal_u8 *)ether_head + ETHER_HDR_LEN);
    (osal_void)memcpy_s((osal_u8 *)snap_head, SNAP_LLC_FRAME_LEN,
        (osal_u8 *)&g_st_mac_11c_snap_header, SNAP_LLC_FRAME_LEN);

    /* change type & length */
    snap_head->ether_type = ether_head->ether_type;
    ether_head->ether_type = oal_byteorder_host_to_net_uint16((osal_u16)(frame_len - ETHER_HDR_LEN +
                                    SNAP_LLC_FRAME_LEN));

    /* COPY MSDU PAYLOAD */
    (osal_void)memcpy_s((osal_u8 *)snap_head + SNAP_LLC_FRAME_LEN, frame_len - ETHER_HDR_LEN,
        oal_netbuf_data(buf) + ETHER_HDR_LEN, frame_len - ETHER_HDR_LEN); /* 函数调用前已校验buf长度 */

    /* 释放旧msdu */
    oal_netbuf_free(buf);

    /* 更新amsdu信息 */
    amsdu->msdu_num++;
    amsdu->amsdu_size += (osal_u16)msdu_len;
    amsdu->last_pad_len = (osal_u8)(msdu_len - SNAP_LLC_FRAME_LEN - frame_len);

    tailroom = oal_netbuf_tailroom(dest_buf);
    /* 当前netbuf剩余空间较少 || 已经达到聚合最大帧数 */
    if ((tailroom < HMAC_AMSDU_TX_MIN_LENGTH) || (amsdu->msdu_num >= amsdu->amsdu_maxnum)) {
        hmac_amsdu_prepare_to_send(hmac_vap, hmac_user, amsdu);
    }

    /* 由于最新的msdu skb已经被释放,不管当前amsdu是否缓存或发送成功/失败,都需要返回TX BUFF */
    return HMAC_TX_BUFF;
}

/*****************************************************************************
 函 数 名  : hmac_amsdu_alloc_netbuf
 功能描述  : 申请netbuf用于聚合组装amsdu
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC OAL_INLINE osal_u32 hmac_amsdu_alloc_netbuf(hmac_amsdu_stru *amsdu, oal_netbuf_stru *buf)
{
    oal_netbuf_stru *dest_buf;
    mac_tx_ctl_stru *cb;

    dest_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (dest_buf == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    /* 为防止后续流程校验不过，直接reserve MAC_80211_QOS_HTC_4ADDR_FRAME_LEN长度的空间 */
    oal_netbuf_reserve(dest_buf, MAC_80211_QOS_HTC_4ADDR_FRAME_LEN);

    /* 子帧链入amsdu尾部 */
    oal_netbuf_add_to_list_tail(dest_buf, &amsdu->msdu_head);

    (osal_void)memcpy_s(oal_netbuf_cb(dest_buf), OAL_SIZEOF(mac_tx_ctl_stru),
        oal_netbuf_cb(buf), OAL_SIZEOF(mac_tx_ctl_stru));

    oal_netbuf_copy_queue_mapping(dest_buf, buf);

    cb = (mac_tx_ctl_stru *)oal_netbuf_cb(dest_buf);
    mac_get_cb_is_first_msdu(cb)    = OAL_TRUE;
    mac_get_cb_is_amsdu(cb)  = OAL_TRUE;
    mac_get_cb_netbuf_num(cb)       = 1;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_amsdu_tx_process_etc
 功能描述  : amsdu聚合发送处理函数
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_u32  hmac_amsdu_tx_process_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf)
{
    osal_u8           tid_no;
    osal_u32          frame_len;
    osal_u32          ul_ret;
    hmac_amsdu_stru    *amsdu;
    mac_tx_ctl_stru    *tx_ctl;

    tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(buf));
    frame_len = oal_netbuf_get_len(buf);
    tid_no    = mac_get_cb_wme_tid_type(tx_ctl);
    amsdu    = &(hmac_user->hmac_amsdu[tid_no]);

    /* amsdu组帧溢出,将链表缓存帧发送并清空,新帧作为amsdu首帧入链表 */
    if (hmac_tx_amsdu_is_overflow(amsdu, frame_len)) {
        hmac_amsdu_prepare_to_send(hmac_vap, hmac_user, amsdu);
    }

    if (amsdu->msdu_num == 0) {
        oal_netbuf_list_head_init(&amsdu->msdu_head);
        /* 申请netbuf用于聚合amsdu */
        if (hmac_amsdu_alloc_netbuf(amsdu, buf) != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_AMSDU, "vap_id[%d] {hmac_amsdu_tx_process_etc::failed to alloc netbuf.}",
                             hmac_vap->vap_id);
            return HMAC_TX_PASS;
        }

        /* 启动定时器 */
        frw_create_timer_entry(&amsdu->amsdu_timer,
                               hmac_amsdu_tx_timeout_process,
                               HMAC_AMSDU_LIFE_TIME,
                               amsdu,
                               OAL_FALSE);
    }

    /* 处理每一个msdu */
    ul_ret = hmac_amsdu_tx_encap_mpdu(hmac_vap, hmac_user, amsdu, buf);
    return ul_ret;
}
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
/*****************************************************************************
 函 数 名  : hmac_tx_encap_large_skb_amsdu
 功能描述  : 大包ampdu+amsdu入口函数
 输入参数  : user: 用户结构体指针
             buf: skb结构体指针
*****************************************************************************/
osal_void hmac_tx_encap_large_skb_amsdu(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf,
    mac_tx_ctl_stru *tx_ctl)
{
    mac_ether_header_stru                    *ether_hdr_temp;
    mac_ether_header_stru                    *ether_hdr;
    osal_u8                                 tid_no;
    osal_u16                                mpdu_len;
    osal_u16                                us_80211_frame_len;

    /* AMPDU+AMSDU功能未开启,由定制化门限决定，高于300Mbps时开启amsdu大包聚合 */
    if (g_st_tx_large_amsdu.cur_amsdu_enable == OAL_FALSE) {
        return;
    }

    /* 针对关闭WMM，非QOS帧处理 */
    if (hmac_user->cap_info.qos == OAL_FALSE) {
        return;
    }

    /* VO、组播队列不开启AMPDU+AMSDU */
    tid_no = mac_get_cb_wme_tid_type(tx_ctl);
    if (tid_no >= WLAN_TIDNO_VOICE) {
        return;
    }

    /* 判断该tid是否支持AMPDU+AMSDU */
    if (hmac_user_is_amsdu_support(hmac_user, tid_no) == OAL_FALSE) {
        return;
    }

    /* 非长帧不进行AMPDU+AMSDU */
    mpdu_len = (osal_u16)oal_netbuf_get_len(buf);
    if ((mpdu_len < MAC_AMSDU_SKB_LEN_DOWN_LIMIT) || (mpdu_len > MAC_AMSDU_SKB_LEN_UP_LIMIT)) {
        return;
    }

    /* 超出分片帧门限不进行AMPDU+AMSDU,计算时考虑需要新增的EHER HEAD LEN和字节对齐,MAC HEAD考虑最长帧头 */
    us_80211_frame_len = mpdu_len + SNAP_LLC_FRAME_LEN + 2 + MAC_80211_QOS_HTC_4ADDR_FRAME_LEN; /* 2长度 */
    if (us_80211_frame_len > mac_mib_get_FragmentationThreshold(hmac_vap)) {
        return;
    }

    /* 已经是小包AMSDU聚合 */
    if (mac_get_cb_is_amsdu(tx_ctl) == OAL_TRUE) {
        return;
    }

    /* ETHER HEAD头部空闲空间,4字节对齐;一般此条件均成立,放置于最后 */
    if (oal_netbuf_headroom(buf) < (SNAP_LLC_FRAME_LEN + 2)) { /* 2长度判断 */
        return;
    }

    /* 80211 FRAME INCLUDE EHTER HEAD */
    mac_get_cb_has_ether_head(tx_ctl) = OAL_TRUE;

    ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(buf);

    /* 预留LLC HEAD长度 */
    oal_netbuf_push(buf, SNAP_LLC_FRAME_LEN);
    ether_hdr_temp = (mac_ether_header_stru *)oal_netbuf_data(buf);
    /* 拷贝mac head */
    (osal_void)memmove_s((osal_u8 *)ether_hdr_temp, ETHER_HDR_LEN, (osal_u8 *)ether_hdr, ETHER_HDR_LEN);
    /* 设置AMSDU帧长度 */
    ether_hdr_temp->ether_type = oal_byteorder_host_to_net_uint16((osal_u16)(
                                            mpdu_len - ETHER_HDR_LEN +
                                            SNAP_LLC_FRAME_LEN));
}
#endif

/*****************************************************************************
 函 数 名 : hmac_amsdu_tid_info_clear
 功能描述 : 删除hmac tid中相关的amsdu信息
 输入参数 : hmac_user : user信息
            tid_index : TID索引0~7, 最大值为WLAN_TID_MAX_NUM = 8
 注意事项 : 内部函数, 不校验入参有效性, 由调用者保证
*****************************************************************************/
OAL_STATIC osal_void hmac_amsdu_tid_info_clear(hmac_user_stru *hmac_user)
{
    hmac_amsdu_stru *amsdu = OAL_PTR_NULL;
    oal_netbuf_stru *amsdu_net_buf = OAL_PTR_NULL;
    osal_u8 loop;

    for (loop = 0; loop < WLAN_TID_MAX_NUM; loop++) {
        amsdu = &(hmac_user->hmac_amsdu[loop]);

        /* tid锁, 禁软中断 */
        osal_spin_lock_bh(&amsdu->amsdu_lock);

        if (amsdu->amsdu_timer.is_registerd == OAL_TRUE) {
            frw_destroy_timer_entry(&(amsdu->amsdu_timer));
        }

        /* 清空聚合队列 */
        if (amsdu->msdu_num != 0) {
            while (oal_netbuf_list_empty(&amsdu->msdu_head) != OAL_TRUE) {
                amsdu_net_buf = oal_netbuf_delist(&(amsdu->msdu_head));
                oal_netbuf_free(amsdu_net_buf);
                amsdu_net_buf = OAL_PTR_NULL;
            }
            amsdu->msdu_num = 0;
            amsdu->amsdu_size = 0;
        }

        /* tid解锁, 使能软中断 */
        osal_spin_unlock_bh(&amsdu->amsdu_lock);
    }
    return;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_amsdu_notify_check_param(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, const mac_ether_header_stru *ether_header, osal_u8 tid_no)
{
    oal_ip_header_stru *pst_ip = OAL_PTR_NULL;

    /* 检查amsdu开关是否打开,amsdu_tx_on 0/1; VAP 是否支持聚合 */
    if ((mac_mib_get_CfgAmsduTxAtive(hmac_vap) != OAL_TRUE) ||
        (mac_mib_get_AmsduAggregateAtive(hmac_vap) != OAL_TRUE)) {
        return HMAC_TX_PASS;
    }

    /* 判断该tid是否在ampdu情况下支持amsdu的发送,ampdu_amsdu 0/1 */
    if (hmac_user_is_amsdu_support(hmac_user, tid_no) == OAL_FALSE) {
        return HMAC_TX_PASS;
    }

    if (ether_header->ether_type == oal_host2net_short(ETHER_TYPE_IP)) {
        pst_ip = (oal_ip_header_stru *)(ether_header + 1);
        /* 允许TCP ACK聚合 */
        /* 为了解决业务量小时ping包延迟的问题，ICMP不聚合 */
        if (oal_netbuf_is_icmp_etc(pst_ip) == OAL_TRUE) {
            return HMAC_TX_PASS;
        }
    }
    /* 检查用户是否是HT/VHT */
    if (hmac_user_xht_support(hmac_user) == OAL_FALSE) {
        oam_info_log1(0, OAM_SF_AMSDU, "vap_id[%d] {hmac_amsdu_notify_etc::user is not qos in amsdu notify}",
            hmac_vap->vap_id);
        return HMAC_TX_PASS;
    }

    if (osal_unlikely(tid_no >= WLAN_TID_MAX_NUM)) {
        oam_error_log1(0, OAM_SF_AMSDU, "vap_id[%d]{hmac_amsdu_notify_etc::invalid tid from] asmdu notify}",
            hmac_vap->vap_id);
        return HMAC_TX_PASS;
    }

    if (tid_no == WLAN_TIDNO_VOICE) {
        oam_info_log3(0, OAM_SF_AMSDU,
            "vap_id[%d] {hmac_amsdu_notify_etc::VO TID NOT SUPPORT AMSDU tid_no=%d amsdu_supported=%d",
            hmac_vap->vap_id, tid_no, hmac_user->amsdu_supported);
        return HMAC_TX_PASS;
    }
    return OAL_CONTINUE;
}
/*****************************************************************************
 函 数 名  : hmac_amsdu_notify_etc
 功能描述  : amsdu入口函数
 输入参数  : user: 用户结构体指针
             buf: skb结构体指针
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_amsdu_notify_etc(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    osal_u8           tid_no;
    osal_u32          ul_ret;         /* 所调用函数的返回值 */
    mac_tx_ctl_stru    *tx_ctl;
    hmac_amsdu_stru    *amsdu;

    mac_ether_header_stru  *ether_header = (mac_ether_header_stru *)oal_netbuf_data(buf);

    /* 获取cb中的tid信息 */
    tx_ctl = (mac_tx_ctl_stru *)(oal_netbuf_cb(buf));
    tid_no    = mac_get_cb_wme_tid_type(tx_ctl);

    /* 针对关闭WMM，非QOS帧处理 */
    if (hmac_user->cap_info.qos == OAL_FALSE) {
        oam_info_log0(0, OAM_SF_TX, "{hmac_amsdu_notify_etc::UnQos Frame pass!!}");
        return HMAC_TX_PASS;
    }
    /* 组播转单播数据不聚合 */
    if (tx_ctl->is_m2u_data == OSAL_TRUE) {
        return HMAC_TX_PASS;
    }
#ifdef _PRE_WLAN_FEATURE_WS53
    if (tx_ctl->is_tcp_ack == OSAL_TRUE) {
        return HMAC_TX_PASS;
    }
#endif
    ul_ret = hmac_amsdu_notify_check_param(hmac_vap, hmac_user, ether_header, tid_no);
    if (ul_ret != OAL_CONTINUE) {
        return ul_ret;
    }

    amsdu = &(hmac_user->hmac_amsdu[tid_no]);
    osal_spin_lock_bh(&amsdu->amsdu_lock);

    /* 新来帧是大帧,需将缓存帧发完 */
    if (oal_netbuf_get_len(buf) > WLAN_MSDU_MAX_LEN) {
        /* 防止乱序,应该先发送旧帧 */
        if (amsdu->msdu_num != 0) {
            hmac_amsdu_prepare_to_send(hmac_vap, hmac_user, amsdu);
        }
        osal_spin_unlock_bh(&amsdu->amsdu_lock);
        return HMAC_TX_PASS;
    }

    ul_ret = hmac_amsdu_tx_process_etc(hmac_vap, hmac_user, buf);
    osal_spin_unlock_bh(&amsdu->amsdu_lock);
    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_amsdu_tx_timeout_process
 功能描述  : 时钟中断事件的处理函数
 输入参数  : hmac_vap:
*****************************************************************************/
WIFI_TCM_TEXT OAL_STATIC osal_u32 hmac_amsdu_tx_timeout_process(osal_void *p_arg)
{
    hmac_amsdu_stru         *temp_amsdu;
    mac_tx_ctl_stru         *cb;
    hmac_user_stru          *hmac_user;
    osal_u32               ul_ret;
    oal_netbuf_stru         *pst_netbuf;
    hmac_vap_stru           *hmac_vap;

    if (osal_unlikely(p_arg == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_AMPDU, "{hmac_amsdu_tx_timeout_process::input null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    temp_amsdu = (hmac_amsdu_stru *)p_arg;

    osal_spin_lock_bh(&temp_amsdu->amsdu_lock);

    /* 根据要发送的amsdu下第一个msdu子帧的cb字段的信息寻找对应用户结构体 */
    pst_netbuf = oal_netbuf_peek(&temp_amsdu->msdu_head);
    if (pst_netbuf == OAL_PTR_NULL) {
        oam_info_log1(0, OAM_SF_AMSDU, "hmac_amsdu_tx_timeout_process::pst_netbuf NULL. msdu_num[%d]",
                      temp_amsdu->msdu_num);
        osal_spin_unlock_bh(&temp_amsdu->amsdu_lock);
        return OAL_ERR_CODE_PTR_NULL;
    }

    cb          = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    hmac_vap    = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_get_cb_tx_vap_index(cb));
    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        osal_spin_unlock_bh(&temp_amsdu->amsdu_lock);
        oam_error_log0(0, OAM_SF_AMPDU, "{hmac_amsdu_tx_timeout_process::hmac_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(mac_get_cb_tx_user_idx(cb));
    if (osal_unlikely(hmac_user == OAL_PTR_NULL)) {
        osal_spin_unlock_bh(&temp_amsdu->amsdu_lock);
        oam_error_log1(0, OAM_SF_AMPDU, "{hmac_amsdu_tx_timeout_process::user[%d] null}",
                       mac_get_cb_tx_user_idx(cb));
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_amsdu_send(hmac_vap, hmac_user, temp_amsdu);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_AMSDU, "vap_id[%d] hmac_amsdu_tx_timeout_process::hmac_amsdu_send fail[%d]",
                         hmac_vap->vap_id, ul_ret);
    }

    osal_spin_unlock_bh(&temp_amsdu->amsdu_lock);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_amsdu_init_user_etc
 功能描述  : 初始化用户tid队列的AMSDU特性值
 输入参数  : hmac_user_stru *hmac_user_sta
 返 回 值  : OAL_STATIC OAL_INLINE osal_void
*****************************************************************************/
OSAL_STATIC osal_void hmac_amsdu_init_user_etc(hmac_user_stru *hmac_user)
{
    osal_u32           amsdu_idx;
    hmac_amsdu_stru     *amsdu;

    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_AMPDU, "{hmac_amsdu_init_user_etc::hmac_user_sta null}");
        return;
    }

    hmac_user->amsdu_maxsize = WLAN_AMSDU_FRAME_MAX_LEN_LONG;

    hmac_user->amsdu_supported = AMSDU_ENABLE_ALL_TID;

    /* 设置amsdu域 */
    for (amsdu_idx = 0; amsdu_idx < WLAN_TID_MAX_NUM; amsdu_idx++) {
        amsdu = &(hmac_user->hmac_amsdu[amsdu_idx]);

        osal_spin_lock_init(&amsdu->amsdu_lock);
        oal_netbuf_list_head_init(&(amsdu->msdu_head));
        amsdu->amsdu_size    = 0;
        amsdu->msdu_num      = 0;
        hmac_amsdu_set_maxnum(amsdu, HMAC_AMSDU_MAX_NUM);
    }
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_s32 hmac_ccpriv_amsdu_tx_on(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8                        ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32                       ret;
    osal_u8                        aggr_tx_on;

    ret = hmac_ccpriv_get_one_arg(&param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_amsdu_tx_on::get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    aggr_tx_on = (osal_u8)oal_atoi((const osal_s8 *)ac_name);

    mac_mib_set_CfgAmsduTxAtive(hmac_vap, aggr_tx_on);
    oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_amsdu_tx_on_etc::ENABLE[%d].}",
                     hmac_vap->vap_id, aggr_tx_on);

    return OAL_SUCC;
}
#endif

osal_u32 hmac_tx_amsdu_init(osal_void)
{
    /* 注册对外接口 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    hmac_feature_hook_register(HMAC_FHOOK_AMSDU_TX_ENCAP_LARGE_SKB, hmac_tx_encap_large_skb_amsdu);
#endif
    hmac_feature_hook_register(HMAC_FHOOK_AMSDU_TX_NOTIFY_ETC, hmac_amsdu_notify_etc);
    hmac_feature_hook_register(HMAC_FHOOK_AMSDU_TX_INIT_USER_ETC, hmac_amsdu_init_user_etc);
    hmac_feature_hook_register(HMAC_FHOOK_AMSDU_TID_INFO_CLEAR, hmac_amsdu_tid_info_clear);

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册, 开启或关闭ampdu发送功能 ccpriv "vap0 amsdu_tx_on 0\1" */
    hmac_ccpriv_register((const osal_s8 *)"amsdu_tx_on", hmac_ccpriv_amsdu_tx_on);
#endif

    return OAL_SUCC;
}

osal_void hmac_tx_amsdu_deinit(osal_void)
{
    /* 去注册对外接口 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    hmac_feature_hook_unregister(HMAC_FHOOK_AMSDU_TX_ENCAP_LARGE_SKB);
#endif
    hmac_feature_hook_unregister(HMAC_FHOOK_AMSDU_TX_NOTIFY_ETC);
    hmac_feature_hook_unregister(HMAC_FHOOK_AMSDU_TX_INIT_USER_ETC);
    hmac_feature_hook_unregister(HMAC_FHOOK_AMSDU_TID_INFO_CLEAR);

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"amsdu_tx_on");
#endif

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

