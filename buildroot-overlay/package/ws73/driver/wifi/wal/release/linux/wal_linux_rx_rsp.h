/*
 * Copyright (c) CompanyNameMagicTag 2001-2023. All rights reserved.
 * Description: wal_linux_rx_rsp.c 的头文件
 */

#ifndef __WAL_LINUX_RX_RSP_H__
#define __WAL_LINUX_RX_RSP_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_types.h"
#include "wal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_RX_RSP_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 宏定义
*****************************************************************************/


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
/* 驱动sta上报内核的扫描结果 */
typedef struct {
    osal_s32               l_signal;      /* 信号强度 */

    osal_s16               s_freq;        /* bss所在信道的中心频率 */
    osal_u8               arry[2];

    osal_u32                mgmt_len;   /* 管理帧长度 */
    oal_ieee80211_mgmt_stru  *mgmt;      /* 管理帧起始地址 */
} wal_scanned_bss_info_stru;

/* 驱动sta上报内核的关联结果 */
typedef struct {
    osal_u8       bssid[WLAN_MAC_ADDR_LEN];  /* sta关联的ap mac地址 */
    osal_u16      status_code;                /* ieee协议规定的16位状态码 */

    osal_u8       *rsp_ie;                 /* asoc_req_ie  */
    osal_u8       *req_ie;

    osal_u32      req_ie_len;               /* asoc_req_ie len */
    osal_u32      rsp_ie_len;

    osal_u16      connect_status;
    osal_u8       auc_resv[2];
} oal_connet_result_stru;

/* 驱动sta上报内核的去关联结果 */
typedef struct {
    osal_u16  reason_code;         /* 去关联 reason code */
    osal_u8   auc_resv[2];

    osal_u8  *pus_disconn_ie;        /* 去关联关联帧 ie */
    osal_u32  disconn_ie_len;      /* 去关联关联帧 ie 长度 */
} oal_disconnect_result_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_s32  wal_asoc_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 wal_p2p_listen_timeout_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 wal_mic_failure_proc_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);

extern osal_s32 wal_disasoc_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 wal_scan_comp_proc_sta_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 wal_send_mgmt_to_host_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 wal_connect_new_sta_proc_ap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
extern osal_s32 wal_disconnect_sta_proc_ap_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_report_channel_switch(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 wal_report_channel_switch_csa(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_linux_rx_rsp.h */
