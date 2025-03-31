/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
  文 件 名   : hmac_blockack.h
  生成日期   : 2013年4月8日
  功能描述   : hmac_blockack.c 的头文件
 */

#ifndef HMAC_BLOCKACK_H
#define HMAC_BLOCKACK_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_rx_data.h"
#include "hmac_vap.h"
#include "hmac_tx_mgmt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_BLOCKACK_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define HMAC_TX_BUF_BITMAP_LOG2_WORD_SIZE 5 /* log2(32) == 5 */
/* 发送BA窗口记录seq number的bitmap所使用的类型长度 */
#define HMAC_TX_BUF_BITMAP_WORD_SIZE        32

#define HMAC_TX_BUF_BITMAP_WORD_MASK (HMAC_TX_BUF_BITMAP_WORD_SIZE - 1)

/* 重排序队列保留bitmap用于缓存乱序帧 */
#define HMAC_BA_BMP_SIZE                    64

/* 判断index为n 在bitmap中的bit位是否是1 */
#define hmac_ba_bit_isset(_bm, _n)        (((_n) < (HMAC_BA_BMP_SIZE)) &&          \
    (((_bm)[(_n) >> 5] & (1 << ((_n) & 31))) != 0))

/* 判断一个seq num是否在发送窗口内 */
#define hmac_baw_within(_start, _bawsz, _seqno)      \
        ((((osal_u32)((_seqno) - (_start))) & 4095) < (_bawsz))

/* Default values for receive timeout */
#define HMAC_BA_RX_VO_TIMEOUT      40  /* 40 milliseconds */
#define HMAC_BA_RX_VI_TIMEOUT      100 /* 100 milliseconds */
#define HMAC_BA_RX_BE_TIMEOUT      60 /* 100 milliseconds */
#define HMAC_BA_RX_BK_TIMEOUT      100 /* 100 milliseconds */

#define HMAC_BA_SEQNO_MASK                  0x0FFF      /* max sequece number */
#define hmac_ba_seqno_sub(_seq1, _seq2)     ((osal_u32)((_seq1) - (_seq2)) & HMAC_BA_SEQNO_MASK)
#define hmac_ba_seqno_add(_seq1, _seq2)     ((osal_u32)((_seq1) + (_seq2)) & HMAC_BA_SEQNO_MASK)

#define WLAN_RX_RESET_BA_THREHOLD 32 /* BA会话未建立收到一定聚合帧发送DELBA */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


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

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 inline函数定义
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_ba_rx_seqno_lt
 功能描述  : 判断seq1是否小于seq2
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_rx_seqno_lt(osal_u16 seq1, osal_u16 seq2)
{
    if (((seq1 < seq2) && ((seq2 - seq1) < HMAC_BA_MAX_SEQNO_BY_TWO)) ||
        ((seq1 > seq2) && ((seq1 - seq2) > HMAC_BA_MAX_SEQNO_BY_TWO))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_ba_rx_seqno_gt
 功能描述  : 判断seq1是否大于seq2
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_rx_seqno_gt(osal_u16 seq1, osal_u16 seq2)
{
    return hmac_ba_rx_seqno_lt(seq2, seq1);
}

/*****************************************************************************
 函 数 名  : hmac_ba_seqno_lt
 功能描述  : 判断seq1是否小于seq2
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_seqno_lt(osal_u16 seq1, osal_u16 seq2)
{
    if (((seq1 < seq2) && ((seq2 - seq1) < HMAC_BA_MAX_SEQNO_BY_TWO)) ||
        ((seq1 > seq2) && ((seq1 - seq2) > HMAC_BA_MAX_SEQNO_BY_TWO))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_ba_seqno_gt
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_seqno_gt(osal_u16 seq1, osal_u16 seq2)
{
    return hmac_ba_seqno_lt(seq2, seq1);
}

/*****************************************************************************
 函 数 名  : hmac_ba_rx_seqno_leq
 功能描述  : 判断seq1是否小于或等于seq2
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_rx_seqno_leq(osal_u16 seq1, osal_u16 seq2)
{
    if (((seq1 <= seq2) && ((seq2 - seq1) < HMAC_BA_MAX_SEQNO_BY_TWO)) ||
        ((seq1 > seq2) && ((seq1 - seq2) > HMAC_BA_MAX_SEQNO_BY_TWO))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_ba_rx_seqno_geq
 功能描述  : 判断seq1是否大于或等于seq2
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_rx_seqno_geq(osal_u16 seq1, osal_u16 seq2)
{
    return hmac_ba_rx_seqno_leq(seq2, seq1);
}

/*****************************************************************************
 函 数 名  : hmac_ba_isset
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_ba_isset(hmac_ba_rx_stru *ba_rx_hdl, osal_u16 seqno)
{
    osal_u16    us_index;

    if (hmac_baw_within(ba_rx_hdl->baw_head, HMAC_BA_BMP_SIZE, seqno) == OSAL_TRUE) {
        us_index = seqno & (HMAC_BA_BMP_SIZE - 1);

        if (hmac_ba_bit_isset(ba_rx_hdl->rx_buf_bitmap, us_index) == OSAL_TRUE) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_ba_addto_rx_bitmap
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_ba_addto_rx_bitmap(hmac_ba_rx_stru *ba_rx_hdl, osal_u16 seqno)
{
    osal_u16    us_index;

    if (hmac_baw_within(ba_rx_hdl->baw_head, HMAC_BA_BMP_SIZE, seqno) == OSAL_TRUE) {
        us_index = seqno  & (HMAC_BA_BMP_SIZE - 1);
        ba_rx_hdl->rx_buf_bitmap[us_index >> HMAC_TX_BUF_BITMAP_LOG2_WORD_SIZE] |=
            (1 << (us_index & HMAC_TX_BUF_BITMAP_WORD_MASK));
    }
}

/*****************************************************************************
 函 数 名  : hmac_ba_clear_rx_bitmap
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_ba_clear_rx_bitmap(hmac_ba_rx_stru *ba_rx_hdl)
{
    osal_u16    us_index;

    us_index = ((osal_u16)hmac_ba_seqno_sub(ba_rx_hdl->baw_head, 1)) & (osal_u16)(HMAC_BA_BMP_SIZE - 1);
    ba_rx_hdl->rx_buf_bitmap[us_index >> HMAC_TX_BUF_BITMAP_LOG2_WORD_SIZE] &=
        ~((osal_u32)(1 << (us_index & HMAC_TX_BUF_BITMAP_WORD_MASK)));
}

/*****************************************************************************
 函 数 名  : hmac_ba_update_rx_bitmap
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void hmac_ba_update_rx_bitmap(hmac_user_stru *hmac_user,
    mac_ieee80211_frame_stru *frame_hdr)
{
    oal_bool_enum_uint8     is_4addr;
    osal_u8               is_tods;
    osal_u8               is_from_ds;
    osal_u8               tid;
    hmac_ba_rx_stru        *ba_rx_hdl;

    /* 考虑四地址情况获取报文的tid */
    is_tods    = mac_hdr_get_to_ds((osal_u8 *)frame_hdr);
    is_from_ds = mac_hdr_get_from_ds((osal_u8 *)frame_hdr);
    is_4addr   = (is_tods != OSAL_FALSE) && (is_from_ds != OSAL_FALSE);
    tid        = mac_get_tid_value((osal_u8 *)frame_hdr, is_4addr);

    ba_rx_hdl = hmac_user->tx_tid_queue[tid].ba_rx_info;
    if (ba_rx_hdl == OAL_PTR_NULL) {
        return;
    }
    if (ba_rx_hdl->ba_status != HMAC_BA_COMPLETE) {
        return;
    }

    osal_spin_lock(&(hmac_user->tx_tid_queue[tid].ba_rx_lock));

    while (hmac_ba_seqno_lt(ba_rx_hdl->baw_head,
        (osal_u16)hmac_ba_seqno_sub(ba_rx_hdl->baw_start, (HMAC_BA_BMP_SIZE - 1))) == OAL_TRUE) {
        ba_rx_hdl->baw_head = (osal_u16)hmac_ba_seqno_add(ba_rx_hdl->baw_head, 1);
        hmac_ba_clear_rx_bitmap(ba_rx_hdl);
    }

    hmac_ba_addto_rx_bitmap(ba_rx_hdl, mac_get_seq_num((osal_u8 *)frame_hdr));
    osal_spin_unlock(&(hmac_user->tx_tid_queue[tid].ba_rx_lock));

    return;
}

/*****************************************************************************
 函 数 名  : hmac_ba_seqno_bound_chk
 功能描述  : This function compares the given sequence number with the specified
             upper and lower bounds and returns its position relative to them.
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_u8 hmac_ba_seqno_bound_chk(osal_u16 seq_lo, osal_u16 seq_hi, osal_u16 seq)
{
    oal_bool_enum_uint8 lo_chk;
    oal_bool_enum_uint8 hi_chk;
    osal_u8 chk_res = 0;

    lo_chk  = hmac_ba_rx_seqno_leq(seq_lo, seq);
    hi_chk  = hmac_ba_rx_seqno_leq(seq, seq_hi);
    if ((lo_chk == OSAL_TRUE) && (hi_chk == OSAL_TRUE)) {
        chk_res = HMAC_BA_BETWEEN_SEQLO_SEQHI;
    } else if (hi_chk == OSAL_FALSE) {
        chk_res = HMAC_BA_GREATER_THAN_SEQHI;
    }

    return chk_res;
}

/* This function reads out the TX-Dscr indexed by the specified sequence number in */
/* the Retry-Q Ring-Buffer. */
/*****************************************************************************
 函 数 名  : hmac_remove_frame_from_reorder_q
 功能描述  :This function reads out the TX-Dscr indexed by the specified sequence number in
            the Retry-Q Ring-Buffer.
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE hmac_rx_buf_stru *hmac_remove_frame_from_reorder_q(hmac_ba_rx_stru *ba_rx_hdl,
    hmac_tid_stru *tx_tid_info, osal_u16 seq_num)
{
    osal_u16        idx;
    hmac_rx_buf_stru *rx_buff;

    idx = (seq_num & (WLAN_AMPDU_RX_BUFFER_SIZE - 1));

    rx_buff = &(ba_rx_hdl->re_order_list[idx]);

    osal_spin_lock(&tx_tid_info->ba_rx_lock);

    if ((rx_buff->in_use == 0) || (rx_buff->seq_num != seq_num)) {
        osal_spin_unlock(&tx_tid_info->ba_rx_lock);
        return OAL_PTR_NULL;
    }

    rx_buff->in_use = 0;

    osal_spin_unlock(&tx_tid_info->ba_rx_lock);

    return rx_buff;
}

/*****************************************************************************
 函 数 名  : hmac_get_frame_from_reorder_q
 功能描述  :This function reads out the TX-Dscr indexed by the specified sequence number in
            the Retry-Q Ring-Buffer.
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC OAL_INLINE hmac_rx_buf_stru *hmac_get_frame_from_reorder_q(hmac_ba_rx_stru *ba_rx_hdl,
    osal_u16 seq_num)
{
    osal_u16        idx;
    hmac_rx_buf_stru *rx_buff;

    idx = (seq_num & (WLAN_AMPDU_RX_BUFFER_SIZE - 1));

    rx_buff = &(ba_rx_hdl->re_order_list[idx]);

    if ((rx_buff->in_use == 0) || (rx_buff->seq_num != seq_num)) {
        return OAL_PTR_NULL;
    }

    return rx_buff;
}


OAL_STATIC OAL_INLINE osal_u8 hmac_ba_get_lut_index(osal_u8 *ba_lut_index_table, osal_u16 start,
    osal_u16 stop)
{
    return oal_get_lut_index(ba_lut_index_table, MAC_RX_BA_LUT_BMAP_LEN, HAL_MAX_RX_BA_LUT_SIZE, start, stop);
}

OAL_STATIC OAL_INLINE osal_void  hmac_ba_del_lut_index(osal_u8 *ba_lut_index_table, osal_u8 lut_index)
{
    oal_del_lut_index(ba_lut_index_table, lut_index);
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_u32 hmac_ba_filter_serv_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_rx_ctl_stru *cb_ctrl, oal_netbuf_head_stru *netbuf_header, oal_bool_enum_uint8 *is_ba_buf);


extern osal_u32 hmac_ba_timeout_fn_etc(osal_void *p_arg);

extern osal_u8  hmac_mgmt_check_set_rx_ba_ok_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_ba_rx_stru *ba_rx_info, hmac_device_stru *hmac_device, hmac_tid_stru *tid_info);

extern osal_void hmac_reorder_ba_rx_buffer_bar_etc(hmac_tid_stru *tx_tid_info, osal_u16 start_seq_num,
    hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_ba_reset_rx_handle_etc(hmac_device_stru *hmac_device, hmac_user_stru *hmac_user,
    osal_u8 tid, oal_bool_enum_uint8 is_aging);
extern osal_void hmac_up_rx_bar_etc(hmac_vap_stru *hmac_vap, dmac_rx_ctl_stru *rx_ctl,
    oal_netbuf_stru *netbuf);

osal_u32 hmac_ba_filter_serv(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, const dmac_rx_ctl_stru *cb_ctrl,
    const mac_ieee80211_frame_stru *frame_hdr, hmac_tid_stru **tid_queue);
osal_u32 hmac_ba_check_rx_aggr(const hmac_vap_stru *hmac_vap, const mac_ieee80211_frame_stru *frame_hdr);
osal_void hmac_ba_reset_tx_handle(hmac_vap_stru *hmac_vap, osal_u16 user_id, osal_u8 tid);
osal_void hmac_clear_tx_qos_seq_num(const hmac_user_stru *hmac_user);
osal_void hmac_clear_tx_nonqos_seq_num(const hmac_vap_stru *hmac_vap);
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
osal_u32 hmac_ba_tx_ampdu_switch(const hmac_vap_stru *hmac_vap, const mac_cfg_ampdu_tx_on_param_stru *ampdu_tx_on);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_blockack.h */
