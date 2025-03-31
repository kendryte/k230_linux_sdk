/*
 * Copyright (c) @CompanyNameMagicTag 2014-2021. All rights reserved.
 * 文 件 名   : hmac_frag.c
 * 作    者   : huanghe
 * 生成日期   : 2014年2月15日
 * 功能描述   : 分片去分片功能
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "hmac_ext_if.h"
#include "hmac_tx_data.h"
#include "oam_struct.h"
#include "hmac_11i.h"
#include "hmac_frag.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_FRAG_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
typedef struct {
    frw_timeout_stru defrag_timer;  /* 去分片超时定时器 */
    oal_netbuf_stru *defrag_netbuf; /* 去分片重组报文指针 */
} hmac_frag_user_info_stru;

OAL_STATIC hmac_frag_user_info_stru *g_hmac_defrag_info[WLAN_USER_MAX_USER_LIMIT];

OAL_STATIC WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT hmac_frag_user_info_stru *hmac_frag_get_user_info(osal_u16 user_idx)
{
    if (osal_unlikely(user_idx >= WLAN_USER_MAX_USER_LIMIT)) {
        return OSAL_NULL;
    }
    return g_hmac_defrag_info[user_idx];
}

OAL_STATIC osal_void hmac_frag_set_user_info(osal_u16 user_idx, osal_void *mem_ptr)
{
    g_hmac_defrag_info[user_idx] = (hmac_frag_user_info_stru *)mem_ptr;
}
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FFD
osal_u8 g_frame_pn_enable = 0;

OAL_STATIC osal_s32 hmac_config_frame_pn_enable(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_frame_pn_enable:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    g_frame_pn_enable = *(osal_u8 *)msg->data;
    oam_info_log1(0, OAM_SF_ANY, "{hmac_config_frame_pn_enable:: g_frame_pn_enable[%d]}", g_frame_pn_enable);

    /* 抛事件至Device侧DMAC，释放VAP下的发送队列 */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_FRAG_PN_ENABLE, msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_config_frame_pn_enable:fail, vap_id[%d] ret[%d]", hmac_vap->vap_id, ret);
    }
    return ret;
}
#endif

#ifndef WIFI_LITE_EXTREME
/* 设置分片门限 */
OAL_STATIC osal_s32 hmac_config_frag_threshold(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_frag_threshold_stru *frag_threshold = OAL_PTR_NULL;
    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_frag_threshold:: hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    frag_threshold = (mac_cfg_frag_threshold_stru *)msg->data;
    if (hmac_vap->mib_info == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_frag_threshold: frag_threshold[%d]!}",
            hmac_vap->vap_id, frag_threshold->frag_threshold);
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_mib_set_FragmentationThreshold(hmac_vap, frag_threshold->frag_threshold);
    return OAL_SUCC;
}

/* 获取分片门限 */
OAL_STATIC osal_s32 hmac_config_show_frag_threshold(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(msg);
    oam_warning_log1(0, OAM_SF_ANY, "FragmentationThreshold:%d.", mac_mib_get_FragmentationThreshold(hmac_vap));
    return OAL_SUCC;
}

/* 检查该报文是否需要分片 */
osal_u32 hmac_tx_need_frag(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    osal_u32        threshold;

    /* 判断报文是否需要进行分片 */
    /* 1、长度大于门限          */
    /* 2、是legac协议模式       */
    /* 3、不是广播帧            */
    /* 4、不是聚合帧            */
    /* 6、DHCP帧不进行分片      */

    if (mac_get_cb_is_vipframe(tx_ctl)) {
        return 0;
    }

    threshold = mac_mib_get_FragmentationThreshold(hmac_vap);
    threshold = (threshold & (~(BIT0 | BIT1))) + 2; /* 2:长度 */

    if (((OAL_NETBUF_LEN(netbuf) + mac_get_cb_frame_header_length(tx_ctl)) > threshold) &&
        (mac_get_cb_is_mcast(tx_ctl) == OAL_FALSE) &&
        (mac_get_cb_is_amsdu(tx_ctl) == OAL_FALSE) &&
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        (mac_get_cb_has_ether_head(tx_ctl) == OAL_FALSE) &&
#endif
        (hmac_user->cur_protocol_mode < WLAN_HT_MODE || hmac_vap->protocol < WLAN_HT_MODE) &&
        (hmac_tid_ba_is_setup(hmac_user, mac_get_cb_wme_tid_type(tx_ctl)) == OAL_FALSE)) {
        return threshold;
    }

    return 0;
}

/* 报文分片处理 */
osal_u32 hmac_frag_process(oal_netbuf_stru *netbuf_orig, mac_tx_ctl_stru *tx_ctrl,
    osal_u32 cip_hdrsize, osal_u32 max_tx_unit)
{
    mac_ieee80211_frame_stru *mac_header = mac_get_cb_frame_header_addr(tx_ctrl);
    mac_ieee80211_frame_stru *frag_header = OAL_PTR_NULL;
    oal_netbuf_stru          *netbuf = OAL_PTR_NULL;
    oal_netbuf_stru          *netbuf_prev = OAL_PTR_NULL;
    osal_u32                frag_num = 1;
    osal_u32                frag_size;
    osal_u32                payload;
    mac_tx_ctl_stru          *tx_ctrl_copy = OAL_PTR_NULL;
    osal_u32                mac_hdr_size = mac_get_cb_frame_header_length(tx_ctrl);
    osal_u32                total_hdrsize = mac_hdr_size + cip_hdrsize;
    osal_u32                offset = max_tx_unit - cip_hdrsize - mac_hdr_size;
    osal_s32                 remainder = (osal_s32)(OAL_NETBUF_LEN(netbuf_orig) - offset);

    mac_header->frame_control.more_frag = OAL_TRUE;
    /* 加密字节数包含在分片门限中，预留加密字节长度，由硬件填写加密头 */
    netbuf_prev = netbuf_orig;

    do {
        frag_size = total_hdrsize + (osal_u32)remainder;

        /* 判断是否还有更多的分片 */
        frag_size = (frag_size > max_tx_unit) ? max_tx_unit : frag_size;

        /* 防止反复扩充头部空间预留32字节 */
        netbuf = oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, frag_size, OAL_NETBUF_PRIORITY_MID);
        if (netbuf == OAL_PTR_NULL) {
            /* 在外部释放之前申请的报文 */
            return OAL_ERR_CODE_PTR_NULL;
        }
        oal_netbuf_put(netbuf, frag_size);
        tx_ctrl_copy = (mac_tx_ctl_stru *)OAL_NETBUF_CB(netbuf);
        /* 拷贝cb字段 */
        if (memcpy_s(tx_ctrl_copy, MAC_TX_CTL_SIZE, tx_ctrl, MAC_TX_CTL_SIZE) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_frag_process::memcpy_s fail.}");
        }
        oal_netbuf_copy_queue_mapping(netbuf, netbuf_orig);

        /* netbuf的headroom大于802.11 mac头长度 */
        frag_header = (mac_ieee80211_frame_stru *)OAL_NETBUF_PAYLOAD(netbuf);
        /* 拷贝帧头内容 */
        if (memcpy_s(frag_header, mac_hdr_size, mac_header, mac_hdr_size) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_frag_process::memcpy_s fail.}");
        }
        /* 赋值分片号 */
        frag_header->frag_num = (osal_u16)frag_num++;

        /* 计算分片报文帧体长度 */
        payload = frag_size - total_hdrsize;
        oal_netbuf_pull(netbuf, mac_get_cb_frame_header_length(tx_ctrl_copy));
        oal_netbuf_copydata(netbuf_orig, offset, OAL_NETBUF_PAYLOAD(netbuf), payload, payload);

        oal_netbuf_set_len(netbuf, payload);
        mac_get_cb_frame_header_addr(tx_ctrl_copy) = frag_header;
        mac_get_cb_mpdu_len(tx_ctrl_copy)      = (osal_u16)payload;
        OAL_NETBUF_NEXT(netbuf_prev)          = netbuf;
        netbuf_prev                           = netbuf;

        /* 计算下一个分片报文的长度和偏移 */
        remainder    -= (osal_s32)payload;
        offset       += payload;
    } while (remainder > 0);

    frag_header->frame_control.more_frag = OAL_FALSE;
    OAL_NETBUF_NEXT(netbuf) = OAL_PTR_NULL;

    /* 原始报文作为分片报文的第一个 */
    oal_netbuf_trim(netbuf_orig, OAL_NETBUF_LEN(netbuf_orig) - (max_tx_unit - cip_hdrsize - mac_hdr_size));
    mac_get_cb_mpdu_len(tx_ctrl) = (osal_u16)(OAL_NETBUF_LEN(netbuf_orig));
    return OAL_SUCC;
}

OAL_STATIC WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_frag_start(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl)
{
    osal_u8 ic_header = 0;
    osal_u32 threshold;
    osal_u32 ret = OAL_SUCC;

    if (osal_unlikely(hmac_user == OAL_PTR_NULL || hmac_vap == OAL_PTR_NULL)) {
        return ret;
    }

    if (osal_unlikely(hmac_vap->mib_info == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_frag_start::hmac_vap[%d] mib_info NULL.}", hmac_vap->vap_id);
        return ret;
    }

    /* 非legacy模式不分片 */
    if (hmac_user->cur_protocol_mode >= WLAN_HT_MODE && hmac_vap->protocol >= WLAN_HT_MODE) {
        return ret;
    }

    threshold = hmac_tx_need_frag(hmac_vap, hmac_user, netbuf, tx_ctl);
    if (threshold != 0) {
        ret = hmac_en_mic_etc(hmac_vap, hmac_user, netbuf, &ic_header);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_tx_encap_etc::hmac_en_mic_etc failed[%d].}",
                           hmac_vap->vap_id, ret);
            return ret;
        }
        /* 进行分片处理 */
        ret = hmac_frag_process(netbuf, tx_ctl, (osal_u32)ic_header, threshold);
    }
    return ret;
}
#endif

/* 解分片超时处理 */
OAL_STATIC osal_u32 hmac_defrag_timeout_fn_etc(osal_void *arg)
{
    hmac_user_stru  *hmac_user = (hmac_user_stru *)arg;
    hmac_frag_user_info_stru *frag_user_info = hmac_frag_get_user_info(hmac_user->assoc_id);
    oal_netbuf_stru *netbuf = OAL_PTR_NULL;

    /* 超时后释放正在重组的分片报文 */
    if (frag_user_info->defrag_netbuf) {
        netbuf = frag_user_info->defrag_netbuf;
        oal_netbuf_free(netbuf);
        frag_user_info->defrag_netbuf = OAL_PTR_NULL;
    }
    return OAL_SUCC;
}

OAL_STATIC oal_netbuf_stru *hmac_defrag_first_frag(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    oal_netbuf_stru  *new_buf = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_frag_user_info_stru *frag_user_info = hmac_frag_get_user_info(hmac_user->assoc_id);
#ifdef _PRE_WLAN_FFD
    mac_rx_ctl_stru          *rx_ctl = OSAL_NULL;
#endif

    hmac_device = hmac_res_get_mac_dev_etc((osal_u32)hmac_user->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_defrag_first_frag::user index[%d]}", hmac_user->assoc_id);
        /* user异常，丢弃报文 */
        OAM_STAT_VAP_INCR(hmac_user->vap_id, rx_defrag_process_dropped, 1);
        return OAL_PTR_NULL;
    }

    /* 启动超时定时器，超时释放重组报文 */
    frw_create_timer_entry(&frag_user_info->defrag_timer, hmac_defrag_timeout_fn_etc, HMAC_FRAG_TIMEOUT, hmac_user,
        OAL_FALSE);

    /* 内存池netbuf只有1600 可能不够，参照A公司申请2500操作系统原生态报文 */
#ifdef _PRE_LWIP_ZERO_COPY
    new_buf = oal_pbuf_netbuf_alloc(HMAC_MAX_FRAG_SIZE);
#else
    new_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, HMAC_MAX_FRAG_SIZE, OAL_NETBUF_PRIORITY_MID);
#endif
    if (new_buf == OAL_PTR_NULL) {
        /* 内存申请失败异常返回，也需要释放传入的分片报文 */
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_defrag_first_frag::Alloc new_buf null,size[%d].}",
            hmac_user->vap_id, HMAC_MAX_FRAG_SIZE);
        return OAL_PTR_NULL;
    }

#ifdef _PRE_LWIP_ZERO_COPY
    skb_reserve(new_buf, PBUF_ZERO_COPY_RESERVE);
#endif
    /* 将分片报文拷贝到新申请的报文中并挂接到用户结构体下，释放原有的报文 */
    oal_netbuf_init(new_buf, OAL_NETBUF_LEN(netbuf));

    oal_netbuf_copydata(netbuf, 0, oal_netbuf_data(new_buf), OAL_NETBUF_LEN(netbuf), OAL_NETBUF_LEN(netbuf));
    (osal_void)memcpy_s(OAL_NETBUF_CB(new_buf), MAC_TX_CTL_SIZE, OAL_NETBUF_CB(netbuf), MAC_TX_CTL_SIZE);
#ifdef _PRE_WLAN_FFD
    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(new_buf);
    hmac_user->last_frame_pn = rx_ctl->latency_index;
#endif
    frag_user_info->defrag_netbuf = new_buf; /* 复用latency_index用于上报pn */
    oal_netbuf_free(netbuf);

    return frag_user_info->defrag_netbuf;
}

OAL_STATIC oal_netbuf_stru *hmac_defrag_process(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf, osal_u32 hrdsize)
{
    mac_ieee80211_frame_stru *last_hdr = OAL_PTR_NULL;
    mac_ieee80211_frame_stru *mac_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);
    hmac_frag_user_info_stru *frag_user_info = hmac_frag_get_user_info(hmac_user->assoc_id);
#ifdef _PRE_WLAN_FFD
    mac_rx_ctl_stru *rx_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
#endif
    /* 判断到来的分片报文是否是第一个分片 */
    if (frag_user_info->defrag_netbuf == OAL_PTR_NULL) {
        /* 首片分片的分片号不为0则释放 */
        if (mac_hdr->frag_num != 0) {
            OAM_STAT_VAP_INCR(hmac_user->vap_id, rx_defrag_process_dropped, 1);
            oam_info_log4(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_defrag_process::The first frag_num is not Zero(%d), seq_num[%d], more_frag[%d].}",
                hmac_user->vap_id, mac_hdr->frag_num, mac_hdr->seq_num, mac_hdr->frame_control.more_frag);
            return OAL_PTR_NULL;
        }

        /* 首片分片处理，成功返回首片分片指针，失败返回NULL */
        return hmac_defrag_first_frag(hmac_user, netbuf);
    }
    /* 此分片是期望的到来的分片，重启定时器，并进行重组 */
    frw_timer_restart_timer(&frag_user_info->defrag_timer, HMAC_FRAG_TIMEOUT, OAL_FALSE);
    last_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(frag_user_info->defrag_netbuf);

    /* 记录最新分片报文的分片号,mac_hdr不可能为空，这是队列中的元素，队列不可能为空 */
    last_hdr->seq_num   = mac_hdr->seq_num;
    last_hdr->frag_num  = mac_hdr->frag_num;
#ifdef _PRE_WLAN_FFD
    hmac_user->last_frame_pn = rx_cb->latency_index; /* 复用latency_index用于上报pn */
#endif
    oal_netbuf_pull(netbuf, hrdsize);

    /* 此接口内会调用dev_kfree_skb */
    oal_netbuf_concat(frag_user_info->defrag_netbuf, netbuf);

    return frag_user_info->defrag_netbuf;
}

/* 去分片处理 */
oal_netbuf_stru *hmac_defrag_start_etc(hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf, osal_u32 hrdsize)
{
    mac_ieee80211_frame_stru *mac_hdr   = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);
    mac_ieee80211_frame_stru *last_hdr  = OAL_PTR_NULL;
    oal_bool_enum_uint8       more_frag = (oal_bool_enum_uint8)mac_hdr->frame_control.more_frag;
    hmac_frag_user_info_stru *frag_user_info = hmac_frag_get_user_info(hmac_user->assoc_id);
    oal_netbuf_stru *defrag_netbuf = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FFD
    mac_rx_ctl_stru *rx_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
#endif
    /* 首先检查到来的分片报文是不是属于正在重组的分片报文 */
    if (frag_user_info->defrag_netbuf != OAL_PTR_NULL) {
        frw_timer_restart_timer(&frag_user_info->defrag_timer, HMAC_FRAG_TIMEOUT, OAL_FALSE);
        last_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(frag_user_info->defrag_netbuf);
        /* 如果地址不匹配，序列号不匹配，分片号不匹配则释放现在正在重组的报文 */
        if (mac_hdr->seq_num != last_hdr->seq_num || mac_hdr->frag_num != (last_hdr->frag_num + 1) ||
            (oal_compare_mac_addr(last_hdr->address1, mac_hdr->address1) != 0) ||
            (oal_compare_mac_addr(last_hdr->address2, mac_hdr->address2) != 0)) {
            frw_destroy_timer_entry(&frag_user_info->defrag_timer);
            oal_netbuf_free(frag_user_info->defrag_netbuf);
            frag_user_info->defrag_netbuf = OAL_PTR_NULL;
        }
#ifdef _PRE_WLAN_FFD
        if ((frag_user_info->defrag_netbuf != OAL_PTR_NULL) &&
            (g_frame_pn_enable == OSAL_TRUE) && (rx_cb->latency_index != hmac_user->last_frame_pn + 1)) {
            frw_destroy_timer_entry(&frag_user_info->defrag_timer);
            oal_netbuf_free(frag_user_info->defrag_netbuf);
            frag_user_info->defrag_netbuf = OAL_PTR_NULL;
        }
#endif
    }

    /* 开始重组, 重组成功返回重组报文指针，重组失败则返回空，并释放报文 */
    if (hmac_defrag_process(hmac_user, netbuf, hrdsize) == OAL_PTR_NULL) {
        oal_netbuf_free(netbuf);
        return OAL_PTR_NULL;
    }

    /* 判断是否重组完毕，是则返回组好的报文 */
    if (more_frag == 0) {
        frw_destroy_timer_entry(&frag_user_info->defrag_timer);
        defrag_netbuf = frag_user_info->defrag_netbuf;
        frag_user_info->defrag_netbuf = OAL_PTR_NULL;

        /* 对重组好的报文进行mic检查 */
        if (hmac_de_mic_etc(hmac_user, defrag_netbuf) != OAL_SUCC) {
            oal_netbuf_free(defrag_netbuf);
            oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_defrag_start::mic check failed.}", hmac_user->vap_id);
            return OAL_PTR_NULL;
        }

        mac_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_data(defrag_netbuf);
        mac_hdr->frag_num = 0;
        return defrag_netbuf;
    }
    /* 判断是否重组完毕，如果存在更多报文待重组则返回空指针 */
    return OAL_PTR_NULL;
}

OAL_STATIC WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT oal_netbuf_stru *hmac_defrag_start(hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, osal_u32 hrdsize)
{
    mac_ieee80211_frame_stru *mac_hdr   = (mac_ieee80211_frame_stru *)oal_netbuf_data(netbuf);
    osal_u8                   frag_num = (osal_u8)mac_hdr->frag_num;
    oal_bool_enum_uint8       more_frag = (oal_bool_enum_uint8)mac_hdr->frame_control.more_frag;
    hmac_frag_user_info_stru *frag_user_info = hmac_frag_get_user_info(hmac_user->assoc_id);

    if (osal_unlikely(hrdsize == 0)) {
        oal_netbuf_free(netbuf);
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_defrag_start::mac head len is 0.}", hmac_user->vap_id);
        return OAL_PTR_NULL;
    }

    /* hmac_user已被删除，不做分片重组 */
    if (osal_unlikely(frag_user_info == OSAL_NULL)) {
        return netbuf;
    }

    /* 如果没有什么可以去分片的则直接返回 */
    if ((more_frag == OAL_FALSE) && (frag_num == 0) && (frag_user_info->defrag_netbuf == OAL_PTR_NULL)) {
        return netbuf;
    }

    return hmac_defrag_start_etc(hmac_user, netbuf, hrdsize);
}

/*****************************************************************************
 清除user下的分片缓存，防止重关联或者rekey流程报文重组attack
*****************************************************************************/
osal_void hmac_user_clear_defrag_res(hmac_user_stru *hmac_user)
{
    hmac_frag_user_info_stru *frag_user_info = OSAL_NULL;

    if (hmac_user == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_user_clear_defrag_res::hmac_user is null ptr.}");
        return;
    }

    frag_user_info = hmac_frag_get_user_info(hmac_user->assoc_id);
    if (frag_user_info == OSAL_NULL) {
        return;
    }

    if (frag_user_info->defrag_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&frag_user_info->defrag_timer);
    }

    if (frag_user_info->defrag_netbuf != NULL) {
        oal_netbuf_free(frag_user_info->defrag_netbuf);
        frag_user_info->defrag_netbuf = NULL;
    }
}

OAL_STATIC osal_bool hmac_frag_del_user(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    osal_u16 user_idx = hmac_user->assoc_id;
    hmac_frag_user_info_stru *frag_user_info = hmac_frag_get_user_info(user_idx);
    hmac_user_clear_defrag_res(hmac_user);

    if (frag_user_info != OSAL_NULL) {
        oal_mem_free(frag_user_info, OAL_TRUE);
    }
    hmac_frag_set_user_info(user_idx, OAL_PTR_NULL);
    return OSAL_TRUE;
}

OAL_STATIC osal_bool hmac_frag_add_user(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u16 user_idx = hmac_user->assoc_id;

    if (hmac_frag_get_user_info(user_idx) != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "user_idx[%d] hmac_frag_add_user mem already malloc!", user_idx);
        return OSAL_TRUE;
    }
    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_frag_user_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CSA, "user_idx[%d] hmac_frag_add_user malloc null!", user_idx);
        return OSAL_FALSE;
    }
    (osal_void)memset_s(mem_ptr, sizeof(hmac_frag_user_info_stru), 0, sizeof(hmac_frag_user_info_stru));
    hmac_frag_set_user_info(user_idx, mem_ptr);
    return OSAL_TRUE;
}

osal_u32 hmac_frag_init(osal_void)
{
    /* 消息注册 */
#ifdef _PRE_WLAN_FFD
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_FRAG_PN_ENABLE, hmac_config_frame_pn_enable);
#endif
#ifndef WIFI_LITE_EXTREME
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_FRAG_THRESHOLD_REG, hmac_config_frag_threshold);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SHOW_FRAG_THRESHOLD_REG, hmac_config_show_frag_threshold);
#endif
    /* notify注册 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_frag_add_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_frag_del_user);
#ifndef WIFI_LITE_EXTREME
    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_FRAG_START, hmac_frag_start);
#endif
    hmac_feature_hook_register(HMAC_FHOOK_FRAG_DERAG_START, hmac_defrag_start);
    hmac_feature_hook_register(HMAC_FHOOK_FRAG_DERAG_CLEAR, hmac_user_clear_defrag_res);

    return OAL_SUCC;
}

osal_void hmac_frag_deinit(osal_void)
{
#ifdef _PRE_WLAN_FFD
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_FRAG_PN_ENABLE);
#endif
#ifndef WIFI_LITE_EXTREME
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_FRAG_THRESHOLD_REG);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SHOW_FRAG_THRESHOLD_REG);
#endif

    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_frag_add_user);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_frag_del_user);

#ifndef WIFI_LITE_EXTREME
    hmac_feature_hook_unregister(HMAC_FHOOK_FRAG_START);
#endif
    hmac_feature_hook_unregister(HMAC_FHOOK_FRAG_DERAG_START);
    hmac_feature_hook_unregister(HMAC_FHOOK_FRAG_DERAG_CLEAR);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

