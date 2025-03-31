/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
  文 件 名   : hmac_blockack.c
  生成日期   : 2014年11月25日
  功能描述   : AMPDU聚合、BA处理接口定义源文件
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_blockack.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"
#include "hmac_main.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_user.h"
#include "hmac_feature_dft.h"
#include "hmac_main.h"
#include "hmac_tid.h"
#include "hmac_user.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_classifier.h"
#include "oal_net.h"
#include "wlan_msg.h"
#include "hmac_btcoex.h"
#include "hmac_feature_interface.h"

#include "hal_chip.h"
#include "msg_ba_manage_rom.h"
#include "common_log_dbg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_BLOCKACK_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数声明
*****************************************************************************/
/*****************************************************************************
  4 函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述  : 获取当前报文seq对应的重排序队列
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC hmac_rx_buf_stru *hmac_ba_buffer_frame_in_reorder(hmac_ba_rx_stru *rx_hdl,
    osal_u16 seq_num, mac_rx_ctl_stru *cb_ctrl)
{
    osal_u16       buf_index;
    hmac_rx_buf_stru *rx_buf = OAL_PTR_NULL;

    buf_index = (seq_num & (WLAN_AMPDU_RX_BUFFER_SIZE - 1));
    rx_buf = &(rx_hdl->re_order_list[buf_index]);

    if (rx_buf->in_use == OAL_TRUE) {
        hmac_rx_free_netbuf_list_etc(&rx_buf->netbuf_head, rx_buf->num_buf);
        rx_hdl->mpdu_cnt--;
        rx_buf->in_use = OAL_FALSE;
        rx_buf->num_buf = 0;
        oam_info_log1(0, OAM_SF_BA, "{hmac_ba_buffer_frame_in_reorder::slot already used, seq[%d].}", seq_num);
    }

    if (cb_ctrl->amsdu_enable == OAL_TRUE) {
        if (cb_ctrl->is_first_buffer == OAL_TRUE) {
            /* num_buf重新赋值时需要释放队列中的buff防止异常情况下内存泄露 */
            hmac_rx_free_netbuf_list_etc(&rx_buf->netbuf_head, rx_buf->num_buf);
            rx_buf->num_buf = 0;
        }
        /* offload下,amsdu帧拆成单帧分别上报 */
        rx_buf->num_buf += cb_ctrl->buff_nums;
        /* 遇到最后一个amsdu buffer 才标记in use 为 1 */
        if (cb_ctrl->is_last_buffer == OAL_TRUE) {
            rx_hdl->mpdu_cnt++;
            rx_buf->in_use = OAL_TRUE;
        }
    } else {
        /* num_buf重新赋值时需要释放队列中的buff防止异常情况下内存泄露 */
        hmac_rx_free_netbuf_list_etc(&rx_buf->netbuf_head, rx_buf->num_buf);
        rx_buf->num_buf   = cb_ctrl->buff_nums;
        rx_hdl->mpdu_cnt++;
        rx_buf->in_use = OAL_TRUE;
    }

    return rx_buf;
}

/*****************************************************************************
 函 数 名  : hmac_ba_send_frames_with_gap
 功能描述  : 冲刷重排序缓冲区至给定的sequence number位置
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_ba_send_frames_with_gap(hmac_ba_rx_stru *ba_rx_hdl,
    oal_netbuf_head_stru *netbuf_header, osal_u16 last_seqnum, hmac_vap_stru *hmac_vap, hmac_tid_stru *tx_tid_info)
{
    osal_u8            num_frms  = 0;
    osal_u16           seq_num;
    hmac_rx_buf_stru    *rx_buf  = OAL_PTR_NULL;
    osal_u8            loop_index;
    oal_netbuf_stru     *pst_netbuf;

    seq_num   = ba_rx_hdl->baw_start;

    oam_info_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_ba_send_frames_with_gap::to seq[%d].}", hmac_vap->vap_id,
        last_seqnum);

    while (seq_num != last_seqnum) {
        rx_buf = hmac_remove_frame_from_reorder_q(ba_rx_hdl, tx_tid_info, seq_num);
        if (rx_buf == OAL_PTR_NULL) {
            seq_num = (osal_u16)hmac_ba_seqno_add(seq_num, 1);
            continue;
        }

        ba_rx_hdl->mpdu_cnt--;
        pst_netbuf = oal_netbuf_peek(&rx_buf->netbuf_head);
        if (osal_unlikely(pst_netbuf == OAL_PTR_NULL)) {
            oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_ba_send_frames_with_gap::gap[%d].\r\n}",
                hmac_vap->vap_id, seq_num);

            seq_num = (osal_u16)hmac_ba_seqno_add(seq_num, 1);
            rx_buf->num_buf = 0;

            continue;
        }

        for (loop_index = 0; loop_index < rx_buf->num_buf; loop_index++) {
            pst_netbuf = oal_netbuf_delist(&rx_buf->netbuf_head);
            if (pst_netbuf != OAL_PTR_NULL) {
                oal_netbuf_add_to_list_tail(pst_netbuf, netbuf_header);
            }
        }
        rx_buf->num_buf = 0;
        num_frms++;

        seq_num = (osal_u16)hmac_ba_seqno_add(seq_num, 1);
    }

    if (ba_rx_hdl->mpdu_cnt != 0) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_ba_send_frames_with_gap::mpdu_cnt=%d.}",
            hmac_vap->vap_id, ba_rx_hdl->mpdu_cnt);
    }

    return num_frms;
}

/*****************************************************************************
 函 数 名  : hmac_ba_send_frames_in_order
 功能描述  : All MSDUs with sequence number starting from the
             start of the BA-Rx window are processed in order and
             are added to the list which will be passed up to hmac.
             Processing is stopped when the first missing MSDU is encountered.
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_u16 hmac_ba_send_frames_in_order(hmac_ba_rx_stru *ba_rx_hdl,
    oal_netbuf_head_stru *netbuf_header, hmac_vap_stru *hmac_vap, hmac_tid_stru *tid_queue_info)
{
    oal_netbuf_stru    *pst_netbuf = OSAL_NULL;
    hmac_rx_buf_stru   *rx_buf = OSAL_NULL;
    osal_u8             loop_idx;
    osal_u16            seq_num;

    seq_num = ba_rx_hdl->baw_start;
    while ((rx_buf = hmac_remove_frame_from_reorder_q(ba_rx_hdl, tid_queue_info, seq_num)) != OSAL_NULL) {
        ba_rx_hdl->mpdu_cnt--;
        seq_num = (osal_u16)hmac_ba_seqno_add(seq_num, 1);
        pst_netbuf = oal_netbuf_peek(&rx_buf->netbuf_head);
        if (pst_netbuf == OSAL_NULL) {
            oam_warning_log2(0, OAM_SF_BA, "vap_id[%d] {hmac_ba_send_frames_in_order::[%d] slot error.}",
                hmac_vap->vap_id, seq_num);
            rx_buf->num_buf = 0;
            continue;
        }

        for (loop_idx = 0; loop_idx < rx_buf->num_buf; loop_idx++) {
            pst_netbuf = oal_netbuf_delist(&rx_buf->netbuf_head);
            if (pst_netbuf != OSAL_NULL) {
                oal_netbuf_add_to_list_tail(pst_netbuf, netbuf_header);
            }
        }

        rx_buf->num_buf = 0;
    }

    return seq_num;
}

/*****************************************************************************
 函 数 名  : hmac_ba_buffer_rx_frame
 功能描述  : 将报文缓存至重排序队列
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  hmac_ba_buffer_rx_frame(hmac_ba_rx_stru *ba_rx_hdl, mac_rx_ctl_stru *cb_ctrl,
    oal_netbuf_head_stru *netbuf_header, osal_u16 seq_num)
{
    hmac_rx_buf_stru   *rx_netbuf = OAL_PTR_NULL;
    oal_netbuf_stru    *pst_netbuf;
    osal_u8           netbuf_index;

    /* Get the pointer to the buffered packet */
    rx_netbuf = hmac_ba_buffer_frame_in_reorder(ba_rx_hdl, seq_num, cb_ctrl);

    /* Update the buffered receive packet details */
    rx_netbuf->seq_num   = seq_num;

    rx_netbuf->rx_time   = (osal_u32)oal_time_get_stamp_ms();

    /* all buffers of this frame must be deleted from the buf list */
    for (netbuf_index = cb_ctrl->buff_nums; netbuf_index > 0; netbuf_index--) {
        pst_netbuf = oal_netbuf_delist(netbuf_header);
        if (osal_unlikely(pst_netbuf != OAL_PTR_NULL)) {
            /* enqueue reorder queue */
            oal_netbuf_add_to_list_tail(pst_netbuf, &rx_netbuf->netbuf_head);
        } else {
            oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_ba_buffer_rx_frame:netbuff error in amsdu.}",
                cb_ctrl->mac_vap_id);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_ba_reorder_rx_data
 功能描述  : 将重排序队列中可以上传的报文加到buf链表的尾部
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  hmac_ba_reorder_rx_data(hmac_ba_rx_stru *ba_rx_hdl,
    oal_netbuf_head_stru *netbuf_header, hmac_vap_stru *hmac_vap, osal_u16 seq_num, hmac_tid_stru *tid_queue_info)
{
    osal_u8 seqnum_pos;
    osal_u16 temp_winstart;

    seqnum_pos = hmac_ba_seqno_bound_chk(ba_rx_hdl->baw_start, ba_rx_hdl->baw_end, seq_num);
    if (seqnum_pos == HMAC_BA_BETWEEN_SEQLO_SEQHI) {
        ba_rx_hdl->baw_start = hmac_ba_send_frames_in_order(ba_rx_hdl, netbuf_header, hmac_vap, tid_queue_info);
        ba_rx_hdl->baw_end   = (osal_u16)hmac_ba_seqno_add(ba_rx_hdl->baw_start, (ba_rx_hdl->baw_size - 1));
    } else if (seqnum_pos == HMAC_BA_GREATER_THAN_SEQHI) {
        temp_winstart = (osal_u16)hmac_ba_seqno_sub(seq_num, (ba_rx_hdl->baw_size - 1));

        hmac_ba_send_frames_with_gap(ba_rx_hdl, netbuf_header, temp_winstart, hmac_vap, tid_queue_info);
        ba_rx_hdl->baw_start = temp_winstart;
        ba_rx_hdl->baw_start = hmac_ba_send_frames_in_order(ba_rx_hdl, netbuf_header, hmac_vap, tid_queue_info);
        ba_rx_hdl->baw_end   = (osal_u16)hmac_ba_seqno_add(ba_rx_hdl->baw_start, (ba_rx_hdl->baw_size - 1));
    } else {
        oam_info_log4(0, OAM_SF_BA, "vap_id[%d] {hmac_ba_reorder_rx_data::else branch seqno[%d] ws[%d] we[%d].}",
            hmac_vap->vap_id, seq_num, ba_rx_hdl->baw_start, ba_rx_hdl->baw_end);
    }
}

/*****************************************************************************
 功能描述  : 冲刷重排序队列
 输入参数  : rx_ba: 接收会话句柄
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OSAL_STATIC osal_void hmac_ba_flush_reorder_q(hmac_ba_rx_stru *rx_ba)
{
    hmac_rx_buf_stru *rx_buf = OAL_PTR_NULL;
    osal_u16        index;

    for (index = 0; index < WLAN_AMPDU_RX_BUFFER_SIZE; index++) {
        rx_buf = &(rx_ba->re_order_list[index]);
        /* 不管是否use均尝试释放一次,防止异常情况下内存泄露 */
        hmac_rx_free_netbuf_list_etc(&rx_buf->netbuf_head, rx_buf->num_buf);
        rx_buf->num_buf = 0;
        if (rx_buf->in_use == OAL_TRUE) {
            rx_buf->in_use = OAL_FALSE;
            rx_ba->mpdu_cnt--;
        }
    }
    if (rx_ba->mpdu_cnt != 0) {
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_flush_reorder_q:: %d mpdu cnt left.}", rx_ba->mpdu_cnt);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ba_need_update_hw_baw
 功能描述  : 判断是否需要更新硬件的BAW
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8  hmac_ba_need_update_hw_baw(hmac_ba_rx_stru *ba_rx_hdl,
    osal_u16 seq_num)
{
    if ((hmac_ba_seqno_lt(seq_num, ba_rx_hdl->baw_start) == OAL_TRUE) &&
        (hmac_ba_rx_seqno_lt(seq_num, ba_rx_hdl->baw_start) == OAL_FALSE)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_ba_filter_serv_dupli_frame_proc
 功能描述  : 丢弃重复报文或继续上报
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u32 hmac_ba_filter_serv_dupli_frame_proc(hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_hdl, osal_u16 seq_num)
{
    /* 上次非定时器上报，直接删除duplicate frame帧，否则，直接上报 */
    if (ba_rx_hdl->timer_triggered == OAL_FALSE && hmac_ba_isset(ba_rx_hdl, seq_num) == OSAL_TRUE) {
        /* 确实已经收到该帧 */
        /* 新增了bitmap记录收包 防止dup误丢包 */
        hmac_user_stats_pkt_incr(hmac_user->rx_pkt_drop, 1);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ba_filter_serv_etc
 功能描述  : 过滤ampdu的每一个mpdu 有未确认报文需要入重传队列
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 非OAL_SUCC:表示异常，后面处理直接Drop
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_ba_filter_serv_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_rx_ctl_stru *cb_ctrl, oal_netbuf_head_stru *netbuf_header, oal_bool_enum_uint8 *is_ba_buf)
{
    hmac_ba_rx_stru *ba_rx_hdl = OSAL_NULL;
    hmac_tid_stru *tid_queue_info = OSAL_NULL;
    osal_u16                  seq_num, baw_start_temp;
    osal_u32                  ret;
    mac_ieee80211_frame_stru   *frame_hdr;

    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_header_addr(cb_ctrl);
    if (osal_unlikely(frame_hdr == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_filter_serv_etc::frame_hdr null.}");
        return OAL_FAIL;
    }
    ret = hmac_ba_filter_serv(hmac_vap, hmac_user, (dmac_rx_ctl_stru*)cb_ctrl, frame_hdr, &tid_queue_info);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    ba_rx_hdl = tid_queue_info->ba_rx_info;
    if (ba_rx_hdl == OSAL_NULL) { // ba会话未建立，报文直接上报
        return OAL_SUCC;
    }

    /* 暂时保存BA窗口的序列号，用于鉴别是否有帧上报 */
    baw_start_temp = ba_rx_hdl->baw_start;

    seq_num = mac_get_seq_num((osal_u8 *)frame_hdr);

    /* 兼容接收方向聚合和分片共存的情况 */
    if ((oal_bool_enum_uint8)frame_hdr->frame_control.more_frag == OAL_TRUE) {
        oam_warning_log2(0, OAM_SF_BA, "vap_id[%d]{get frame[seq_num=%d]When session UP", cb_ctrl->mac_vap_id, seq_num);
        return OAL_SUCC;
    }

    /* duplicate frame判断 */
    if (hmac_ba_rx_seqno_lt(seq_num, ba_rx_hdl->baw_start) == OAL_TRUE) {
        return hmac_ba_filter_serv_dupli_frame_proc(hmac_user, ba_rx_hdl, seq_num);
    }

    /* restart ba timer */
    if (hmac_ba_seqno_lt(ba_rx_hdl->baw_tail, seq_num) == OAL_TRUE) {
        ba_rx_hdl->baw_tail = seq_num;
    }

    /* 接收到的帧的序列号等于BAW_START，并且缓存队列帧个数为0，则直接上报给HMAC */
    if ((seq_num == ba_rx_hdl->baw_start) && (ba_rx_hdl->mpdu_cnt == 0) &&
        /* offload 下amsdu帧由于可能多个buffer组成，一律走重排序 */
        (cb_ctrl->amsdu_enable == OAL_FALSE)) {
        ba_rx_hdl->baw_start = (osal_u16)hmac_ba_seqno_add(ba_rx_hdl->baw_start, 1);
        ba_rx_hdl->baw_end  = (osal_u16)hmac_ba_seqno_add(ba_rx_hdl->baw_end, 1);
    } else {
        /* Buffer the new MSDU */
        *is_ba_buf = OAL_TRUE;

        /* buffer frame to reorder */
        hmac_ba_buffer_rx_frame(ba_rx_hdl, cb_ctrl, netbuf_header, seq_num);

        /* put the reordered netbufs to the end of the list */
        hmac_ba_reorder_rx_data(ba_rx_hdl, netbuf_header, hmac_vap, seq_num, tid_queue_info);

        /* Check for Sync loss and flush the reorder queue when one is detected */
        if ((ba_rx_hdl->baw_tail == (osal_u16)hmac_ba_seqno_sub(ba_rx_hdl->baw_start, 1)) &&
            (ba_rx_hdl->mpdu_cnt > 0)) {
            oam_warning_log1(0, OAM_SF_BA, "vap[%d]{hmac_ba_filter_serv_etc:Sync loss,flush que}", hmac_vap->vap_id);
            hmac_ba_flush_reorder_q(ba_rx_hdl);
        }
    }

    if (baw_start_temp != ba_rx_hdl->baw_start) {
        ba_rx_hdl->timer_triggered = OAL_FALSE;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_reorder_ba_rx_buffer_bar_etc
 功能描述  : This function reorders the Reciver buffer and sends frames to the higher
             layer on reception of a Block-Ack-Request frame. It also updates the
             receiver buffer window.
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_reorder_ba_rx_buffer_bar_etc(hmac_tid_stru *tx_tid_info, osal_u16 start_seq_num,
    hmac_vap_stru *hmac_vap)
{
    oal_netbuf_head_stru    netbuf_head;
    hmac_ba_rx_stru *rx_ba;
    osal_u8               seqnum_pos;

    if (tx_tid_info == OAL_PTR_NULL || tx_tid_info->ba_rx_info == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar_etc::ba session doesnot set up.}");
        return;
    }

    rx_ba = tx_tid_info->ba_rx_info;

    /* 针对 BAR 的SSN和窗口的start_num相等时，不需要移窗 */
    if (rx_ba->baw_start == start_seq_num) {
        oam_info_log0(0, OAM_SF_BA, "{hmac_reorder_ba_rx_buffer_bar_etc::seq is equal to start num.}");
        return;
    }

    oal_netbuf_list_head_init(&netbuf_head);

    seqnum_pos = hmac_ba_seqno_bound_chk(rx_ba->baw_start, rx_ba->baw_end, start_seq_num);
    /* 针对BAR的的SSN在窗口内才移窗 */
    if (seqnum_pos == HMAC_BA_BETWEEN_SEQLO_SEQHI) {
        hmac_ba_send_frames_with_gap(rx_ba, &netbuf_head, start_seq_num, hmac_vap, tx_tid_info);
        rx_ba->baw_start = start_seq_num;
        rx_ba->baw_start = hmac_ba_send_frames_in_order(rx_ba, &netbuf_head, hmac_vap, tx_tid_info);
        rx_ba->baw_end   = (osal_u16)hmac_ba_seqno_add(rx_ba->baw_start, (rx_ba->baw_size - 1));

        oam_info_log4(0, OAM_SF_BA,
            "vap_id[%d] {hmac_reorder_ba_rx_buffer_bar_etc::receive a bar, baw_start=%d baw_end=%d. seq num=%d.}",
            hmac_vap->vap_id, rx_ba->baw_start, rx_ba->baw_end, start_seq_num);

        hmac_rx_lan_frame_etc(&netbuf_head);
    } else if (seqnum_pos == HMAC_BA_GREATER_THAN_SEQHI) {
        /* 异常 */
        oam_warning_log4(0, OAM_SF_BA,
            "vap_id[%d]{hmac_reorder_ba_rx_buffer_bar_etc::ssn is out of winsize, start=%d end=%d, seq num=%d.}",
            hmac_vap->vap_id, rx_ba->baw_start, rx_ba->baw_end, start_seq_num);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ba_rx_prepare_bufflist
 功能描述  : 从重排序队列中获取skb链
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_ba_rx_prepare_bufflist(hmac_vap_stru *hmac_vap, hmac_rx_buf_stru *rx_buf,
    oal_netbuf_head_stru *netbuf_head)
{
    oal_netbuf_stru     *pst_netbuf;
    osal_u8            loop_index;

    pst_netbuf = oal_netbuf_peek(&rx_buf->netbuf_head);
    if (pst_netbuf == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_ba_rx_prepare_bufflist::pst_netbuf null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (loop_index = 0; loop_index < rx_buf->num_buf; loop_index++) {
        pst_netbuf = oal_netbuf_delist(&rx_buf->netbuf_head);
        if (pst_netbuf != OAL_PTR_NULL) {
            oal_netbuf_add_to_list_tail(pst_netbuf, netbuf_head);
        } else {
            oam_warning_log1(0, OAM_SF_BA,
                "vap_id[%d] {hmac_ba_rx_prepare_bufflist::num_buf in reorder list is error.}",
                hmac_vap->vap_id);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ba_send_reorder_timeout
 功能描述  : 上报重排序队列中超时的报文
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_ba_send_reorder_timeout(hmac_ba_rx_stru *rx_ba, hmac_vap_stru *hmac_vap,
    hmac_ba_alarm_stru *alarm_data, osal_u16 *pus_timeout, hmac_tid_stru *tx_tid_info)
{
    osal_u32                  time_diff;
    osal_u32                  rx_timeout;
    oal_netbuf_head_stru        netbuf_head;
    osal_u16                  baw_head;
    osal_u16                  baw_start;   /* 保存最初的窗口起始序列号 */
    hmac_rx_buf_stru           *rx_buf;
    osal_u8                   buff_count = 0;
    osal_u32                  ul_ret;
    osal_u16                  baw_end;

    oal_netbuf_list_head_init(&netbuf_head);
    baw_head     = rx_ba->baw_start;
    baw_start    = rx_ba->baw_start;
    rx_timeout   = (osal_u32)hmac_vap->rx_timeout[wlan_wme_tid_to_ac(alarm_data->tid)];
    baw_end      = (osal_u16)hmac_ba_seqno_add(rx_ba->baw_tail, 1);

    osal_spin_lock(&(tx_tid_info->ba_rx_lock));

    while (baw_head != baw_end) {
        rx_buf = hmac_get_frame_from_reorder_q(rx_ba, baw_head);
        if (rx_buf == OAL_PTR_NULL) {
            buff_count++;
            baw_head = (osal_u16)hmac_ba_seqno_add(baw_head, 1);
            continue;
        }

        time_diff = (osal_u32)oal_time_get_stamp_ms() - rx_buf->rx_time;
        if (time_diff < rx_timeout) {
            *pus_timeout = (osal_u16)(rx_timeout - time_diff);
            break;
        }

        rx_ba->mpdu_cnt--;
        rx_buf->in_use = 0;

        ul_ret = hmac_ba_rx_prepare_bufflist(hmac_vap, rx_buf, &netbuf_head);
        if (ul_ret != OAL_SUCC) {
            buff_count++;
            baw_head = (osal_u16)hmac_ba_seqno_add(baw_head, 1);
            continue;
        }

        buff_count++;
        baw_head = (osal_u16)hmac_ba_seqno_add(baw_head, 1);
        rx_ba->baw_start = (osal_u16)hmac_ba_seqno_add(rx_ba->baw_start, buff_count);
        rx_ba->baw_end   = (osal_u16)hmac_ba_seqno_add(rx_ba->baw_start, (rx_ba->baw_size - 1));

        buff_count = 0;
    }

    osal_spin_unlock(&(tx_tid_info->ba_rx_lock));

    /* 判断本次定时器超时是否有帧上报 */
    if (baw_start != rx_ba->baw_start) {
        rx_ba->timer_triggered = OAL_TRUE;
    }

    hmac_rx_lan_frame_etc(&netbuf_head);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_ba_timeout_get_param(const hmac_ba_alarm_stru *alarm_data, osal_u8 *tid,
    hmac_vap_stru **hmac_vap, hmac_user_stru **hmac_user, hmac_device_stru **hmac_device)
{
    *tid = alarm_data->tid;
    if (*tid >= WLAN_TID_MAX_NUM) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_timeout_get_param::tid [%d] overflow.}", *tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(alarm_data->vap_id);
    if (*hmac_vap == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_timeout_get_param::hmac_vap null. vap id [%d].}", alarm_data->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(alarm_data->mac_user_idx);
    if (*hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_timeout_get_param::hmac_user null. user idx [%d].}",
            alarm_data->mac_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *hmac_device = hmac_res_get_mac_dev_etc((*hmac_vap)->device_id);
    if (*hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_timeout_get_param::hmac_device null. device_id [%d].}",
            (*hmac_vap)->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_ba_timeout_fn_etc
 功能描述  : ba会话超时处理
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_ba_timeout_fn_etc(osal_void *p_arg)
{
    hmac_ba_rx_stru                    *rx_ba;
    hmac_vap_stru                      *hmac_vap;
    hmac_user_stru                     *hmac_user;
    hmac_ba_alarm_stru                 *alarm_data;
    mac_delba_initiator_enum_uint8      direction;
    osal_u8                           tid = 0;
    hmac_device_stru                    *hmac_device;
    osal_u16                          timeout;
    osal_u32                          ret;

    alarm_data = (hmac_ba_alarm_stru *)p_arg;

    ret = hmac_ba_timeout_get_param(alarm_data, &tid, &hmac_vap, &hmac_user, &hmac_device);
    if (ret != OAL_SUCC) {
        return ret;
    }

    direction = alarm_data->direction;

    if (direction == MAC_RECIPIENT_DELBA) {
        rx_ba = (hmac_ba_rx_stru *)alarm_data->ba;

        if (rx_ba == OAL_PTR_NULL) {
            oam_error_log0(0, OAM_SF_BA, "{hmac_ba_timeout_fn_etc::rx_ba is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        timeout = hmac_vap->rx_timeout[wlan_wme_tid_to_ac(tid)];

        if (rx_ba->mpdu_cnt > 0) {
            hmac_ba_send_reorder_timeout(rx_ba, hmac_vap, alarm_data, &timeout, &(hmac_user->tx_tid_queue[tid]));
        }
        frw_create_timer_entry(&(hmac_user->tx_tid_queue[tid].ba_timer), hmac_ba_timeout_fn_etc, timeout,
                               alarm_data, OAL_FALSE);
    } else {
        /* tx ba不删除 */
        frw_create_timer_entry(&(hmac_user->tx_tid_queue[tid].ba_timer), hmac_ba_timeout_fn_etc,
                               hmac_vap->rx_timeout[wlan_wme_tid_to_ac(tid)], alarm_data, OAL_FALSE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ba_reset_rx_handle_etc
 功能描述  : 重置rx ba结构体
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_ba_reset_rx_handle_etc(hmac_device_stru *hmac_device, hmac_user_stru   *hmac_user, osal_u8 tid,
    oal_bool_enum_uint8 is_aging)
{
    hmac_vap_stru    *hmac_vap;
    oal_bool_enum     need_del_lut = OAL_TRUE;
    hmac_ba_rx_stru **ppst_rx_ba = OSAL_NULL;

    unref_param(is_aging);

    if (osal_unlikely(hmac_user == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ppst_rx_ba = &hmac_user->tx_tid_queue[tid].ba_rx_info;

    if (osal_unlikely((*ppst_rx_ba == OAL_PTR_NULL) || ((*ppst_rx_ba)->is_ba != OAL_TRUE))) {
        oam_warning_log0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::rx ba not set yet.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (tid >= WLAN_TID_MAX_NUM) {
        oam_error_log1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::tid %d overflow.}", tid);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap((*ppst_rx_ba)->alarm_data.vap_id);
    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Step1: disable the flag of ba session */
    (*ppst_rx_ba)->is_ba = OAL_FALSE;

    /* Step2: flush reorder q */
    hmac_ba_flush_reorder_q(*ppst_rx_ba);

    if ((*ppst_rx_ba)->lut_index == MAC_INVALID_RX_BA_LUT_INDEX) {
        need_del_lut = OAL_FALSE;
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_reset_rx_handle_etc::no need to del lut index, lut index[%d]}\n",
            (*ppst_rx_ba)->lut_index);
    }

    /* Step3: if lut index is valid, del lut index alloc before */
    if (((*ppst_rx_ba)->ba_policy == MAC_BA_POLICY_IMMEDIATE) && (need_del_lut == OAL_TRUE)) {
        hmac_ba_del_lut_index(hmac_device->rx_ba_lut_idx_table, (*ppst_rx_ba)->lut_index);
    }

    /* Step4: dec the ba session cnt maitence in vap struc */
    hmac_rx_ba_session_decr_etc(hmac_vap, tid);

    /* Step5: Del Timer */
    if (hmac_user->tx_tid_queue[tid].ba_timer.is_registerd == OAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_user->tx_tid_queue[tid].ba_timer));
    }

    /* Step6: Free rx handle */
    oal_mem_free(*ppst_rx_ba, OAL_TRUE);
    *ppst_rx_ba = OAL_PTR_NULL;

    return OAL_SUCC;
}

OAL_STATIC osal_u8 hmac_mgmt_check_rx_ba_type(const hmac_ba_rx_stru *ba_rx_info, hmac_vap_stru *hmac_vap)
{
    /* 立即块确认判断 */
    if (ba_rx_info->ba_policy == MAC_BA_POLICY_IMMEDIATE) {
        if (mac_mib_get_dot11ImmediateBlockAckOptionImplemented(hmac_vap) == OAL_FALSE) {
            /* 不支持立即块确认 */
            oam_warning_log1(0, OAM_SF_BA,
                "vap_id[%d] {hmac_mgmt_check_set_rx_ba_ok_etc::not support immediate Block Ack.}",
                hmac_vap->vap_id);
            return MAC_INVALID_REQ_PARAMS;
        } else {
            if (ba_rx_info->back_var != MAC_BACK_COMPRESSED) {
                /* 不支持非压缩块确认 */
                oam_warning_log1(0, OAM_SF_BA,
                    "vap_id[%d] {hmac_mgmt_check_set_rx_ba_ok_etc::not support non-Compressed Block Ack.}",
                    hmac_vap->vap_id);
                return MAC_REQ_DECLINED;
            }
        }
    } else if (ba_rx_info->ba_policy == MAC_BA_POLICY_DELAYED) {
        /* 延迟块确认不支持 */
        oam_warning_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_check_set_rx_ba_ok_etc::not support delayed Block Ack.}",
            hmac_vap->vap_id);
        return MAC_INVALID_REQ_PARAMS;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_check_set_rx_ba_ok_etc
 功能描述  : 从空口接收ADDBA_REQ帧的处理函数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u8  hmac_mgmt_check_set_rx_ba_ok_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_info, hmac_device_stru *hmac_device, hmac_tid_stru *tid_info)
{
    osal_u8 ret;
    osal_void *fhook = OSAL_NULL;
    ba_rx_info->lut_index = MAC_INVALID_RX_BA_LUT_INDEX;

    ret = hmac_mgmt_check_rx_ba_type(ba_rx_info, hmac_vap);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    if (mac_mib_get_RxBASessionNumber(hmac_vap) > WLAN_MAX_RX_BA) {
        oam_warning_log2(0, OAM_SF_BA,
            "vap_id[%d] {hmac_mgmt_check_set_rx_ba_ok_etc::rx_ba_session_num[%d] is up to max.}",
            hmac_vap->vap_id, mac_mib_get_RxBASessionNumber(hmac_vap));
        return MAC_REQ_DECLINED;
    }

    /* 获取BA LUT INDEX */
    ba_rx_info->lut_index = hmac_ba_get_lut_index(hmac_device->rx_ba_lut_idx_table, 0,
        HAL_MAX_RX_BA_LUT_SIZE);
    /* LUT index表已满 */
    if (ba_rx_info->lut_index == MAC_INVALID_RX_BA_LUT_INDEX) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_mgmt_check_set_rx_ba_ok_etc::ba lut index table full.",
            hmac_vap->vap_id);
        return MAC_REQ_DECLINED;
    }

    /* 共存黑名单用户，不建立聚合，直到对应业务将标记清除 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_CHECK_USER_REQ_DECLINED);
    if (fhook != OSAL_NULL && ((hmac_btcoex_check_user_req_declined_cb)fhook)(hmac_vap, hmac_user) == OAL_TRUE) {
        return MAC_REQ_DECLINED;
    }

    /* 该tid下不允许建BA，配置命令需求 */
    if (tid_info->ba_handle_rx_enable == OAL_FALSE) {
        oam_warning_log2(0, OAM_SF_BA,
            "vap_id[%d] {hmac_mgmt_check_set_rx_ba_ok_etc::tid[%d] is not available.}",
            hmac_vap->vap_id, tid_info->tid);
        return MAC_REQ_DECLINED;
    }

    return MAC_SUCCESSFUL_STATUSCODE;
}

/*****************************************************************************
 函 数 名  : hmac_up_rx_bar_etc
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void  hmac_up_rx_bar_etc(hmac_vap_stru *hmac_vap, dmac_rx_ctl_stru *rx_ctl, oal_netbuf_stru *netbuf)
{
    osal_u8                 *payload;
    mac_ieee80211_frame_stru  *frame_hdr;
    osal_u8                 *sa_addr;
    osal_u8                  tidno;
    hmac_user_stru            *ta_user;
    osal_u16                 start_seqnum;

    unref_param(netbuf);

    frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    sa_addr = frame_hdr->address2;

    /* 获取用户指针 */
    ta_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, sa_addr);
    if (ta_user == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_up_rx_bar_etc::ta_user  is null.}");
        return;
    }

    /* 获取帧头和payload指针 */
    payload = mac_get_rx_payload_addr(&(rx_ctl->rx_info), netbuf);

    /*************************************************************************/
    /*                     BlockAck Request Frame Format                     */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BAR Control|BlockAck Starting    |FCS|  */
    /* |             |        |  |  |           |Sequence number      |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |2          |2                    |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    tidno        = (payload[1] & 0xF0) >> 4;     /* 右移4位 */
    if (tidno >= WLAN_TIDNO_BUTT) {
        return;
    }
    start_seqnum = mac_get_bar_start_seq_num(payload);

    hmac_reorder_ba_rx_buffer_bar_etc(&(ta_user->tx_tid_queue[tidno]), start_seqnum, hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_ba_filter_serv
 功能描述  : 过滤ampdu的每一个mpdu 有未确认报文需要入重传队列
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_ba_filter_serv(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const dmac_rx_ctl_stru *cb_ctrl, const mac_ieee80211_frame_stru *frame_hdr, hmac_tid_stru **tid_queue)
{
    hmac_tid_stru          *tid_queue_tmp = OSAL_NULL;
    hmac_ba_rx_stru        *ba_rx = OSAL_NULL;
    oal_bool_enum_uint8     is_4addr;
    osal_u8               is_tods;
    osal_u8               is_from_ds;
    osal_u32              ret;
    osal_u8 tid;

    ret = hmac_ba_check_rx_aggr(hmac_vap, frame_hdr);
    if (ret != OAL_SUCC) {
        return OAL_SUCC;
    }

    /* 考虑四地址情况获取报文的tid */
    is_tods    = mac_hdr_get_to_ds((osal_u8 *)frame_hdr);
    is_from_ds = mac_hdr_get_from_ds((osal_u8 *)frame_hdr);
    is_4addr   = (is_tods != OSAL_FALSE) && (is_from_ds != OSAL_FALSE);
    tid = mac_get_tid_value((osal_u8 *)frame_hdr, is_4addr);

    tid_queue_tmp = &(hmac_user->tx_tid_queue[tid]);
    ba_rx = tid_queue_tmp->ba_rx_info;
    if (ba_rx == OAL_PTR_NULL) {
        return OAL_SUCC;
    }

    *tid_queue = tid_queue_tmp;

    /* BA会话没有建立 */
    if (ba_rx->ba_status != HMAC_BA_COMPLETE) {
        if (cb_ctrl->rx_status.ampdu == OSAL_TRUE) {
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_ba_filter_serv::ba not created but recv ampdu,ba status: %d.}",
                hmac_vap->vap_id, ba_rx->ba_status);

            tid_queue_tmp->rx_wrong_ampdu_num++;

            if (tid_queue_tmp->rx_wrong_ampdu_num == WLAN_RX_RESET_BA_THREHOLD) {
                tid_queue_tmp->rx_wrong_ampdu_num = 0;

                common_log_dbg2(0, OAM_SF_BA,
                    "vap_id[%d] {hmac_ba_filter_serv: rx_wrong_ampdu_num exceed threshold, Del Ba. tid[%d]}",
                    hmac_vap->vap_id, tid);
                hmac_mgmt_delba(hmac_vap, hmac_user, tid, MAC_RECIPIENT_DELBA, MAC_QSTA_SETUP_NOT_DONE);
            }
        }
        return OAL_SUCC;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 重置发送TX ba窗
*****************************************************************************/
osal_void hmac_ba_reset_tx_handle(hmac_vap_stru *hmac_vap, osal_u16 user_id, osal_u8 tid)
{
    osal_s32                ret;
    frw_msg                 msg_info = {0};
    mac_mgmt_delba_ba_sync  ba_sync_stru = {0};
    hmac_tid_stru           *tid_tx = OSAL_NULL;
    hmac_user_stru          *hmac_user = OSAL_NULL;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_id);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_reset_tx_handle::hmac_user[%d] null.}", user_id);
        return;
    }

    tid_tx = &(hmac_user->tx_tid_queue[tid]);

    /* 删除时，开启TID队列状态 */
    hmac_tid_resume(tid_tx, DMAC_TID_PAUSE_RESUME_TYPE_BA);

    ba_sync_stru.assoc_id = hmac_user->assoc_id;
    ba_sync_stru.tidno = tid;
    ba_sync_stru.initiator = MAC_ORIGINATOR_DELBA;
    ba_sync_stru.is_tx = OSAL_TRUE;

    /* 抛事件至Device侧DMAC，同步重置会话 */
    frw_msg_init((osal_u8 *)&ba_sync_stru, sizeof(mac_mgmt_delba_ba_sync), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_DEVICE_DELBA, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_BA, "{hmac_ba_reset_tx_handle::frw_send_msg_to_device failed[%d].}", ret);
        return;
    }
}

/*****************************************************************************
 功能描述  : 清空qos seq num
*****************************************************************************/
osal_void hmac_clear_tx_qos_seq_num(const hmac_user_stru *hmac_user)
{
    osal_u8 tid_loop;
    machw_tx_sequence_stru tx_seq;

    /* 遍历TID清空，LUT表清空功能会清零所有LUT INDEX */
    for (tid_loop = 0; tid_loop < WLAN_TID_MAX_NUM; tid_loop++) {
        tx_seq.lut_index = hmac_user->lut_index;
        tx_seq.tid = tid_loop;
        tx_seq.qos_flag = OSAL_TRUE;
        hal_set_tx_sequence_num(tx_seq, 0, 0);
    }
}

/*****************************************************************************
 函 数 名  : hmac_clear_tx_nonqos_seq_num
 功能描述  : 清空非qos seq num
*****************************************************************************/
osal_void hmac_clear_tx_nonqos_seq_num(const hmac_vap_stru *hmac_vap)
{
    machw_tx_sequence_stru tx_seq;

    tx_seq.lut_index = 0;
    tx_seq.tid = 0;
    tx_seq.qos_flag = 0;

    hal_set_tx_sequence_num(tx_seq, 0, hmac_vap->hal_vap->vap_id);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
/*****************************************************************************
 功能描述  : 软硬件切换不删减聚合方式入口
*****************************************************************************/
osal_u32 hmac_ba_tx_ampdu_switch(const hmac_vap_stru *hmac_vap, const mac_cfg_ampdu_tx_on_param_stru *ampdu_tx_on)
{
    hal_chip_stru *hal_chip = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device_base = OSAL_NULL;
    frw_msg msg_info = {0};
    osal_s32 ret;

    hal_chip = hal_get_chip_stru();
    hal_device_base = &hal_chip->device.hal_device_base;

    /* 只有主路会硬件聚合 */
    if (ampdu_tx_on->aggr_tx_on == hal_device_base->ampdu_tx_hw) {
        return OAL_SUCC;
    }

    /* 抛事件至Device侧DMAC，同步配置硬聚 */
    frw_msg_init((osal_u8 *)ampdu_tx_on, sizeof(mac_cfg_ampdu_tx_on_param_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_AMPDU_TX_ON, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
            "{hmac_ba_tx_ampdu_switch::frw_send_msg_to_device failed[%d].}", ret);
        return OAL_FAIL;
    }

    /* 使能硬件聚合 */
    hal_device_base->ampdu_tx_hw = ampdu_tx_on->aggr_tx_on;
    hal_device_base->ampdu_partial_resnd = ampdu_tx_on->snd_type;
    hal_set_hw_en_reg_cfg(hal_device_base->ampdu_tx_hw);
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_ba_check_rx_aggr
 功能描述  : 检查是否能做ba重排序处理
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_ba_check_rx_aggr(const hmac_vap_stru *hmac_vap,
    const mac_ieee80211_frame_stru *frame_hdr)
{
    /* 该vap是否是ht */
    if (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OSAL_FALSE) {
        return OAL_FAIL;
    }

    /* 判断该帧是不是qos帧 */
    if ((((osal_u8 *)frame_hdr)[0] != (WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA))) {
        return OAL_FAIL;
    }

    /* 判断该帧是不是组播 */
    if (mac_is_grp_addr(frame_hdr->address1)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_addba_req_etc
 功能描述  : 建立BA会话的配置命令
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_addba_req_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_addba_req_param_stru   *addba_req;
    hmac_user_stru                 *hmac_user;
    mac_action_mgmt_args_stru       action_args;   /* ~{SCSZLnP4~}ACTION~{V!5D2NJ}~} */
    oal_bool_enum_uint8             ampdu_support;

    addba_req = (mac_cfg_addba_req_param_stru *)msg->data;

    /* 获取用户对应的索引 */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, addba_req->mac_addr);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_addba_req_etc::hmac_user null.}",
                         hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 建立BA会话，是否需要判断VAP的AMPDU的支持情况，因为需要实现建立BA会话时，一定发AMPDU */
    ampdu_support = hmac_user_xht_support(hmac_user);
    /* 手动创建ba会话，不受其他额外限制 */
    if (ampdu_support) {
        /*
            建立BA会话时，action_args(ADDBA_REQ)结构各个成员意义如下
            (1)category:action的类别
            (2)action:BA action下的类别
            (3)arg1:BA会话对应的TID
            (4)arg2:BUFFER SIZE大小
            (5)arg3:BA会话的确认策略
            (6)arg4:TIMEOUT时间
        */
        action_args.category = MAC_ACTION_CATEGORY_BA;
        action_args.action   = MAC_BA_ACTION_ADDBA_REQ;
        action_args.arg1     = addba_req->tidno;       /* 该数据帧对应的TID号 */
        action_args.arg2     = addba_req->buff_size;   /* ADDBA_REQ中，buffer_size的默认大小 */
        action_args.arg3     = addba_req->ba_policy;   /* BA会话的确认策略 */
        action_args.arg4     = addba_req->timeout;     /* BA会话的超时时间设置为0 */

        /* 建立BA会话 */
        hmac_mgmt_tx_action_etc(hmac_vap,  hmac_user, &action_args);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_delba_req_etc
 功能描述  : 删除BA会话的配置命令(相当于接收到DELBA帧)
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_delba_req_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_delba_req_param_stru   *delba_req;
    hmac_user_stru                 *hmac_user;
    mac_action_mgmt_args_stru       action_args;   /* ~{SCSZLnP4~}ACTION~{V!5D2NJ}~} */
    hmac_tid_stru                  *hmac_tid;

    delba_req = (mac_cfg_delba_req_param_stru *)msg->data;

    /* 获取用户对应的索引 */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, delba_req->mac_addr);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_delba_req_etc::hmac_user null.}",
                         hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_tid = &(hmac_user->tx_tid_queue[delba_req->tidno]);

    /* 查看会话是否存在 */
    if (delba_req->direction == MAC_RECIPIENT_DELBA) {
        if (delba_req->trigger == MAC_DELBA_TRIGGER_COMM) {
            hmac_tid->ba_handle_rx_enable = OAL_FALSE;
        }

        if (hmac_tid->ba_rx_info == OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_delba_req_etc::the rx hdl is not exist.}",
                             hmac_vap->vap_id);
            return OAL_SUCC;
        }
    } else {
        if (delba_req->trigger == MAC_DELBA_TRIGGER_COMM) {
            hmac_tid->ba_handle_tx_enable = OAL_FALSE;
        }

        if (hmac_tid->ba_tx_info.ba_status == HMAC_BA_INIT) {
            oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_delba_req_etc::the tx hdl is not exist.}",
                             hmac_vap->vap_id);
            return OAL_SUCC;
        }
    }

    /*
        建立BA会话时，action_args(DELBA_REQ)结构各个成员意义如下
        (1)category:action的类别
        (2)action:BA action下的类别
        (3)arg1:BA会话对应的TID
        (4)arg2:删除ba会话的发起端
        (5)arg3:删除ba会话的原因
        (6)arg5:ba会话对应的用户
    */
    action_args.category = MAC_ACTION_CATEGORY_BA;
    action_args.action   = MAC_BA_ACTION_DELBA;
    action_args.arg1     = delba_req->tidno;       /* 该数据帧对应的TID号 */
    action_args.arg2     = delba_req->direction;   /* ADDBA_REQ中，buffer_size的默认大小 */
    action_args.arg3     = MAC_QSTA_TIMEOUT; /* BA会话的确认策略 */
    action_args.arg5    = delba_req->mac_addr;   /* ba会话对应的user */

    /* 建立BA会话 */
    hmac_mgmt_tx_action_etc(hmac_vap,  hmac_user, &action_args);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

