/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_tx_amsdu.h
 * 生成日期   : 2012年11月12日
 * 功能描述   : hmac_tx_amsdu.c 的头文件
 */


#ifndef __HMAC_TX_AMSDU_H__
#define __HMAC_TX_AMSDU_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_tx_data.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_AMSDU_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* HT控制信息的amsdu能力位 */
#define HT_CAP_AMSDU_LEN 0x0800

/* amsdu生命周期15ms FPGA 1500 */
#define HMAC_AMSDU_LIFE_TIME    5

/* amsdu聚合最小帧长(对照ack帧长,拍板决定) */
#define HMAC_AMSDU_TX_MIN_PKT_LENGTH 14
/* 发送方向组masdu帧的netbuf剩余tailroom最小值, LLC+PKT LENTH+PAD */
#define HMAC_AMSDU_TX_MIN_LENGTH  (SNAP_LLC_FRAME_LEN + HMAC_AMSDU_TX_MIN_PKT_LENGTH + 3)

/* 短包聚合最大个数 */
#define HMAC_AMSDU_SHORT_PACKET_NUM     0x02

/* 小于500字节的包为短包    */
#define HMAC_AMSDU_SHORT_PACKET_LEN     500

/* 一个amsdu下允许拥有的msdu的最大个数 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define HMAC_AMSDU_MAX_NUM              hwifi_get_amsdu_num()
#else
#define HMAC_AMSDU_MAX_NUM              4
#endif

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
 函 数 名  : hmac_amsdu_set_maxnum
 功能描述  : 配置amsdu子帧最大个数
*****************************************************************************/
OAL_STATIC OAL_INLINE osal_void  hmac_amsdu_set_maxnum(hmac_amsdu_stru *amsdu, osal_u8 max_num)
{
    if (max_num > HMAC_AMSDU_MAX_NUM) {
        amsdu->amsdu_maxnum = HMAC_AMSDU_MAX_NUM;
    } else {
        amsdu->amsdu_maxnum = max_num;
    }
}

/*****************************************************************************
 函 数 名  : hmac_amsdu_is_short_pkt
 功能描述  : amsdu判断是否短包
 输入参数  : osal_u32 frame_len 包长
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 hmac_amsdu_is_short_pkt(osal_u32 frame_len)
{
    if (frame_len < HMAC_AMSDU_SHORT_PACKET_LEN) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
static osal_u32 hmac_tx_amsdu_init_weakref(osal_void) __attribute__ ((weakref("hmac_tx_amsdu_init"), used));
static osal_void hmac_tx_amsdu_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_tx_amsdu_deinit"), used));

typedef osal_u32 (*hmac_amsdu_notify_etc_cb)(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *buf);
typedef osal_void (*hmac_amsdu_init_user_etc_cb)(hmac_user_stru *hmac_user);
typedef osal_void (*hmac_amsdu_tid_info_clear_cb)(hmac_user_stru *hmac_user);
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
typedef osal_void (*hmac_tx_encap_large_skb_amsdu_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf, mac_tx_ctl_stru *tx_ctl);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_tx_amsdu.h */
