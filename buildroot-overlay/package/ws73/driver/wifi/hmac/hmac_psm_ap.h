/*
 * Copyright (c) CompanyNameMagicTag 2015-2023. All rights reserved.
 * 文 件 名   : hmac_psm_ap.h
 * 生成日期   : 2015年1月26日
 * 功能描述   : hmac_psm_ap.c 的头文件
 */


#ifndef __HMAC_PSM_AP_H__
#define __HMAC_PSM_AP_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "hmac_user.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_PSM_AP_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_SMALL_MEMORY
#define MAX_MPDU_NUM_IN_PS_QUEUE    32
#else
#define MAX_MPDU_NUM_IN_PS_QUEUE    128
#endif

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u32 hmac_psm_doze(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u32 hmac_psm_resv_ps_poll(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_void hmac_psm_set_local_bitmap(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 bitmap_flg);
osal_u32 hmac_psm_send_null_data(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_bool_enum_uint8 ps, oal_bool_enum_uint8 is_from_ps_queue);
osal_void hmac_psm_user_ps_structure_init(hmac_user_stru *hmac_user);
osal_void hmac_psm_user_ps_structure_destroy(hmac_user_stru *hmac_user);
osal_void hmac_psm_delete_ps_queue_head(hmac_user_stru *hmac_user, osal_u32 psm_delete_num);
osal_void hmac_psm_clear_ps_queue(hmac_user_stru *hmac_user);
osal_u32 hmac_psm_ps_enqueue(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *net_buffer);
osal_void hmac_psm_queue_flush(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
osal_u8 hmac_psm_pkt_need_buff(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
osal_void hmac_psm_rx_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *net_buf);
osal_s32 hmac_psm_reset(hmac_vap_stru *vap, osal_u16 user_id);
osal_u32 hmac_psm_tx_set_more_data(hmac_user_stru *hmac_user, mac_tx_ctl_stru *tx_cb);
osal_u32 hmac_rx_data_ps_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_psm_ap.h */
