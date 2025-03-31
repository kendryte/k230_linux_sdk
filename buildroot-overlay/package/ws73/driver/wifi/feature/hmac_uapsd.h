/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_uapsd.h
 * 生成日期   : 2013年9月18日
 * 功能描述   : hmac_uapsd.c 的头文件
 */

#ifndef __HMAC_UAPSD_H__
#define __HMAC_UAPSD_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_UAPSD_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_UAPSD_SEND_ALL  0xff       /* 发送队列中所有报文,设置为UINT8变量最大值 */
#define HMAC_UAPSD_WME_LEN   8

#define HMAC_UAPSD_NOT_SEND_FRAME   (-1)
#define HMAC_UAPSD_QDEPTH_DEFAULT   32

#define HMAC_USR_UAPSD_EN 0x01   // U-APSD使能
#define HMAC_USR_UAPSD_TRIG 0x02 // U-APSD可以被trigger
#define HMAC_USR_UAPSD_SP 0x04   // u-APSD在一个Service Period处理中

#define HMAC_UAPSD_INVALID_TRIGGER_SEQ 0xffff

#define HMAC_UAPSD_IDLE_INT 1000 // esop trigger处理最大时长

typedef struct {
    osal_spinlock lock_uapsd;            /* uapsd操作所 */
    oal_netbuf_head_stru uapsd_queue_head;    /* uapsd节能队列头 */
    osal_atomic mpdu_num;                      /* 当前节能队列里的mpdu个数 */
    osal_u16 uapsd_trigseq[WLAN_WME_AC_BUTT];   /* 最近一个trigger帧的sequence */
} hmac_user_uapsd_stru;

typedef struct {
    osal_u8  qos_info;                            /* 关联请求中的WMM IE的QOS info field */
    osal_u8  max_sp_len;                          /* 从qos info字段中提取翻译的最大服务长度 */
    osal_u8  auc_resv[2];
    osal_u8  ac_trigger_ena[WLAN_WME_AC_BUTT];    /* 4个AC的trigger能力 */
    osal_u8  ac_delievy_ena[WLAN_WME_AC_BUTT];    /* 4个AC的delivery能力 */
    osal_u32 last_esop_tsf;
} mac_user_uapsd_status_stru;

typedef struct {
    hmac_user_uapsd_stru uapsd_stru;
    mac_user_uapsd_status_stru uapsd_status;
    osal_u8 uapsd_flag;
    osal_u8 resv[3];
} hmac_user_uapsd_info_stru;
/*****************************************************************************
  10 函数声明
*****************************************************************************/

typedef osal_u32 (*hmac_uapsd_update_user_para_etc_cb)(osal_u8 *payload, osal_u8 sub_type, osal_u32 msg_len,
    hmac_user_stru *hmac_user);
typedef osal_void (*hmac_uapsd_rx_trigger_check_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const oal_netbuf_stru *net_buf);
typedef osal_s32 (*hmac_uapsd_flush_queue_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
typedef osal_u32 (*hmac_uapsd_enqueue_proc_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf);
typedef oal_bool_enum_uint8 (*hmac_psm_is_uapsd_empty_cb)(hmac_user_stru *hmac_user);
typedef hmac_user_uapsd_info_stru *(*hmac_ap_get_user_uapsd_info_cb)(osal_u16 assoc_id);

static osal_u32 hmac_uapsd_ap_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_uapsd_ap_init"), used));
static osal_void hmac_uapsd_ap_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_uapsd_ap_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_uapsd.h */
