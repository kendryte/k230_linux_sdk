/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Source file defined by the corresponding frame structure and operation interface (cannot be called by
 * the HAL module).
 * Create: 2022-11-26
 */

#ifndef __MAC_FRAME_COMMON_H__
#define __MAC_FRAME_COMMON_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_frame_common_rom.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 此文件中定义的结构体与协议相关，需要1字节对齐 */
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TWT
/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9, twt特性需要长度为10 */
#define MAC_XCAPS_EX_TWT_LEN 10
#endif

#define MAC_DHCP_DECLINE 4

#define MAC_DHCP_NAK 6
#define MAC_DHCP_RELEASE 7
#define MAC_DHCP_INFORM 8

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  枚举名  : wlan_ieee80211_frame_type_enum_uint8
  协议表格:
  枚举说明: 802.11 MAC帧头类型
*****************************************************************************/
typedef enum {
    MAC_IEEE80211_BASE_FRAME = 0,      /* 基础802.11帧类型 */
    MAC_IEEE80211_QOS_FRAME,           /* QoS帧类型 */
    MAC_IEEE80211_QOS_HTC_FRAME,       /* QoS + HTC帧类型 */
    MAC_IEEE80211_ADDR4_FRAME,         /* 四地址帧类型 */
    MAC_IEEE80211_QOS_ADDR4_FRAME,     /* QoS四地址帧类型 */
    MAC_IEEE80211_QOS_HTC_ADDR4_FRAME, /* QoS + HTC四地址帧类型 */

    MAC_IEEE80211_FRAME_BUTT
} mac_ieee80211_frame_type_enum;
typedef osal_u8 mac_ieee80211_frame_type_enum_uint8;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 私有管理帧通用的设置参数信息的结构体 */
typedef struct {
    osal_u8 type;
    osal_u8 arg1;      /* 对应的tid序号 */
    osal_u8 arg2;      /* 接收端可接收的最大的mpdu的个数(针对AMPDU_START命令) */
    osal_u8 arg3;      /* 确认策略 */
    osal_u16 user_idx; /* 对应的用户 */
    osal_u8 auc_resv[2]; /* 预留2字节对齐 */
} mac_priv_req_args_stru;

osal_u8 *mac_find_ie_sec_etc(osal_u8 eid, osal_u8 *ies, osal_s32 len, osal_u32 *remain_len);

/*****************************************************************************
 函 数 名  : mac_hdr_set_fragment_number
 功能描述  : 设置MAC头分片序号字段

*****************************************************************************/
static inline osal_void mac_hdr_set_fragment_number(osal_u8 *header, osal_u8 frag_num)
{
    header[WLAN_HDR_FRAG_OFFSET] &= 0xF0;
    header[WLAN_HDR_FRAG_OFFSET] |= (frag_num & 0x0F);
}

/*****************************************************************************
 函 数 名  : mac_set_seq_num
 功能描述  : 设置帧的序列号

*****************************************************************************/
static inline osal_void mac_set_seq_num(osal_u8 *header, osal_u16 seq_num)
{
    header[23] = (osal_u8)seq_num >> 4; /* seq字段高bit偏移23, bit偏移4 */
    header[22] &= 0x0F; /* seq字段高bit偏移22, bit掩码0x0F */
    header[22] |= (osal_u8)(seq_num << 4); /* seq字段高bit偏移22, bit偏移4 */
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_frame_common.h */
