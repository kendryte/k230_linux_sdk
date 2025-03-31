/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * reserved. Description: hmac tid head
 * Create: 2020-7-5
 */

#ifndef __HMAC_TID_H__
#define __HMAC_TID_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "dmac_ext_if_hcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
typedef struct {
    osal_u16 user_idx;
    osal_u8  tid;
    osal_u8  from_resume;
} hmac_tid_resume_stru;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_tid_pause(hmac_tid_stru *tid, osal_u8 type);
osal_u32 hmac_tid_resume(hmac_tid_stru *tid, osal_u8 type);
osal_void hmac_tid_tx_queue_init(hmac_tid_stru *tx_tid_queue, hmac_user_stru *hmac_user);
osal_void hmac_tid_tx_enqueue_update(hmac_device_stru *hmac_device, hmac_tid_stru *tid_queue, osal_u8 mpdu_num);
osal_void hmac_tid_tx_dequeue_update(hmac_device_stru *hmac_device, hmac_tid_stru *tid_queue, osal_u8 mpdu_num);
osal_u32 hmac_tid_clear(hmac_user_stru *hmac_user, hmac_device_stru *hmac_device, osal_u8 del_user);
osal_void hmac_tid_buffer_status_calc(hmac_user_stru *hmac_user, wlan_tidno_enum_uint8 tid);
#ifdef _PRE_WLAN_FEATURE_BSRP
osal_void hmac_tid_buffer_status_init(osal_void);
#endif
osal_s32 hmac_config_tid_resume_process(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32 hmac_tid_get_mpdu_by_index(hmac_tid_stru *tid_queue, osal_u16 mpdu_num, oal_netbuf_stru **netbuf_stru);
osal_u32  hmac_tid_delete_mpdu_head(hmac_tid_stru *tid_queue, osal_u16 mpdu_num);

static inline osal_void hmac_tx_tid_buffer_len_decrease(hmac_tid_stru *tid_queue, oal_netbuf_stru *buf)
{
    oal_netbuf_stru *tid_netbuf_next = OSAL_NULL;
    mac_tx_ctl_stru *tid_buffer_cb = OSAL_NULL;
    oal_netbuf_stru *netbuf = buf;

    while (netbuf != OSAL_NULL) {
        tid_netbuf_next = oal_get_netbuf_next(netbuf);
        tid_buffer_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        tid_queue->tid_buffer_frame_len = (tid_queue->tid_buffer_frame_len < tid_buffer_cb->mpdu_payload_len) ? 0 :
            (tid_queue->tid_buffer_frame_len - tid_buffer_cb->mpdu_payload_len);
        netbuf = tid_netbuf_next;
    }
}

static inline osal_void hmac_tx_tid_buffer_len_increase(hmac_tid_stru *tid_queue, oal_netbuf_stru *buf)
{
    oal_netbuf_stru *tid_netbuf_next = OSAL_NULL;
    mac_tx_ctl_stru *tid_buffer_cb = OSAL_NULL;
    oal_netbuf_stru *netbuf = buf;

    while (netbuf != OSAL_NULL) {
        tid_netbuf_next = oal_get_netbuf_next(netbuf);
        tid_buffer_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
        tid_queue->tid_buffer_frame_len += tid_buffer_cb->mpdu_payload_len;
        netbuf = tid_netbuf_next;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_tid_rom.h */
