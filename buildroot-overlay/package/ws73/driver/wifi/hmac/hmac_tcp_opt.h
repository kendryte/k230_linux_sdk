/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file for hmac_tcp_opt.c.
 * Create: 2021-12-15
 */

#ifndef __HMAC_TCP_OPT_H__
#define __HMAC_TCP_OPT_H__

#ifdef _PRE_WLAN_TCP_OPT
#include "hcc_host_if.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_OPT_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/


/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define FIN_FLAG_BIT    (1 << 0)
#define SYN_FLAG_BIT    (1 << 1)
#define RESET_FLAG_BIT  (1 << 2)
#define URGENT_FLAG_BIT (1 << 5)
#define FILTER_FLAG_MASK (FIN_FLAG_BIT | SYN_FLAG_BIT | RESET_FLAG_BIT | URGENT_FLAG_BIT)
#define MAX_TIME_BY_TWO (1 << 16)
#define HCC_ASSEMBLE_INFO_SIZE          16


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
  10 函数声明
*****************************************************************************/
void hmac_tcp_opt_ack_count_reset_etc(hmac_vap_stru *hmac_vap, hcc_chan_type dir, osal_u16 stream);
struct tcp_list_node *hmac_tcp_opt_find_oldest_node_etc(hmac_vap_stru *hmac_vap, hcc_chan_type dir);
struct tcp_list_node *hmac_tcp_opt_get_buf_etc(hmac_vap_stru *hmac_vap, hcc_chan_type dir);

osal_u32 hmac_tcp_opt_add_node_etc(hmac_vap_stru *hmac_vap, struct wlan_tcp_flow *tcp_info, hcc_chan_type dir);
osal_u32 hmac_tcp_opt_init_filter_tcp_ack_pool_etc(hmac_vap_stru *hmac_vap);
void hmac_tcp_opt_deinit_list_etc(hmac_vap_stru *hmac_vap);
osal_u32 hmac_tcp_opt_get_flow_index_etc(hmac_vap_stru       *hmac_vap,
    oal_ip_header_stru  *ip_hdr,
    oal_tcp_header_stru *tcp_hdr,
    hcc_chan_type        dir);

oal_tcp_ack_type_enum_uint8 hmac_tcp_opt_get_tcp_ack_type_etc(hmac_vap_stru      *hmac_vap,
    oal_ip_header_stru *ip_hdr,
    hcc_chan_type       dir,
    osal_u16          us_index);
oal_bool_enum_uint8 hmac_judge_rx_netbuf_classify_etc(oal_netbuf_stru *netbuff);
oal_bool_enum_uint8 hmac_judge_rx_netbuf_is_tcp_ack_etc(mac_llc_snap_stru *snap);
oal_bool_enum_uint8 hmac_judge_tx_netbuf_is_tcp_ack_etc(oal_ether_header_stru *ps_ethmac_hdr);
oal_tcp_ack_type_enum_uint8  hmac_tcp_opt_rx_get_tcp_ack_etc(oal_netbuf_stru *skb, hmac_vap_stru *hmac_vap,
    osal_u16 *p_us_index, osal_u8 dir);
oal_tcp_ack_type_enum_uint8  hmac_tcp_opt_tx_get_tcp_ack_etc(oal_netbuf_stru *skb, hmac_vap_stru *hmac_vap,
    osal_u16 *p_us_index, osal_u8 dir);
osal_u16 hmac_tcp_opt_tcp_ack_list_filter_etc(hmac_vap_stru *hmac_vap, hmac_tcp_opt_queue type,
    hcc_chan_type dir, oal_netbuf_head_stru  *head);
osal_u16 hmac_tcp_opt_rx_tcp_ack_filter_etc(void *hmac_device, hmac_tcp_opt_queue type, hcc_chan_type dir,
    void *data);
osal_u16 hmac_tcp_opt_tx_tcp_ack_filter_etc(void *hmac_vap, hmac_tcp_opt_queue type, hcc_chan_type dir,
    void *data);
osal_u32 hmac_tcp_opt_tcp_ack_filter_etc(oal_netbuf_stru *skb, hmac_vap_stru *hmac_vap, hcc_chan_type dir);

void hmac_trans_queue_filter_etc(hmac_vap_stru *hmac_vap, oal_netbuf_head_stru *head_t,
    hmac_tcp_opt_queue type, hcc_chan_type dir);
void hmac_tcp_ack_process_hcc_queue_etc(hmac_vap_stru *hmac_vap,
    hcc_chan_type dir, hmac_tcp_opt_queue type);
osal_s32 hmac_tcp_ack_process_etc(void);
oal_bool_enum_uint8 hmac_tcp_ack_need_schedule_etc(void);
osal_void hmac_sched_transfer_etc(void);

/*****************************************************************************
  11 内联函数定义
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif   /* end of _PRE_WLAN_TCP_OPT */

static osal_u32 hmac_tcp_opt_init_weakref(osal_void) __attribute__ ((weakref("hmac_tcp_opt_init"), used));
static osal_void hmac_tcp_opt_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_tcp_opt_deinit"), used));

#endif /* end of hmac_tcp_opt.h */
