/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * 文 件 名 : hmac_sigma_traffic_stat.h
 * 生成日期 : 2023年1月5日
 * 功能描述 : hmac_sigma_traffic_stat.c 的头文件
 */

#ifndef HMAC_SIGMA_TRAFFIC_STAT_H
#define HMAC_SIGMA_TRAFFIC_STAT_H

#ifdef _PRE_WLAN_FEATURE_WFA_SUPPORT
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  宏定义
*****************************************************************************/
#define MAX_SIGMA_STREAM_CNT 8 /* 支持最大的sigma流数 */
#define MAX_SIGMA_RX_VALUE 32 /* 支持最大的统计位宽long long int类型 */

/*****************************************************************************
  全局变量声明
*****************************************************************************/

/*****************************************************************************
  STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u8   flag; /* 流量统计开关 */
    osal_u8   stream_cnt; /* 流数 */
    osal_u16  resv;
    osal_u16  port[MAX_SIGMA_STREAM_CNT]; /* UDP rx流目的端口号,报文关键识别信息 */
    osal_u16  ip_id[MAX_SIGMA_STREAM_CNT]; /* IP报头的identification字段,同一分片报文识别 */
    osal_u16  frag_offset[MAX_SIGMA_STREAM_CNT]; /* IP分片报文偏移字段,连续分片报文识别 */
    osal_u16  udp_tot_len[MAX_SIGMA_STREAM_CNT]; /* UDP报文总长度,真实流量,不含IP和UDP头 */
    osal_u32  rx_frames[MAX_SIGMA_STREAM_CNT]; /* 接收报文数量 */
    osal_u64  rx_bytes[MAX_SIGMA_STREAM_CNT]; /* 接收bytes数量 */
} hmac_udp_stat_stru;

/*****************************************************************************
  函数声明
*****************************************************************************/
osal_void hmac_rx_sigma_traffic_stat(const mac_ether_header_stru *ether_hdr);
osal_s32 hmac_config_set_traffic_stat(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_add_traffic_port(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  /* end of _PRE_WLAN_FEATURE_WFA_SUPPORT */

#endif /* end of hmac_sigma_traffic_stat.h */
