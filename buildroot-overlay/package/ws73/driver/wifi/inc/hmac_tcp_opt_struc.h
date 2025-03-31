/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file for hmac_tcp_opt_struc.c.
 * Create: 2021-12-15
 */

#ifndef __HMAC_TCP_OPT_STRUC_H__
#define __HMAC_TCP_OPT_STRUC_H__

#ifdef _PRE_WLAN_TCP_OPT

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_netbuf_ext.h"
#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TCP_OPT_STRUC_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_TCP_STREAM  32
#define TCPDATA_INFO_MAXNUM 4
#define TCPDATA_PSH_INFO_MAXNUM (8 * TCPDATA_INFO_MAXNUM)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum {
    TCP_ACK_FILTER_TYPE  = 1,
    TCP_ACK_DUP_TYPE     = 2,
    TCP_TYPE_ERROR
};
typedef osal_u8 oal_tcp_ack_type_enum_uint8;
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
typedef struct _wlan_perform_tcp_impl_ {
    osal_u64 ull_drop_count;
    osal_u64 ull_send_count;
    osal_u64 ull_dup_ack_count;
} wlan_perform_tcp_impl;

typedef struct _wlan_perform_tcp_impls_ {
    osal_u32 ack_limit;
    osal_u64 ull_ignored_count;
    wlan_perform_tcp_impl tcp_info[HMAC_TCP_STREAM];
} wlan_perform_tcp_impls;

struct skb_tcp_cb {
    osal_u16 us_index;
    osal_u16 reserve;
    osal_u32  tcp_type;
    osal_u32  tcp_ack_no;
};

struct wlan_perform_tcp {
    /* hcc rx/tx tcp ack param */
    oal_netbuf_head_stru hcc_ack_queue[HMAC_TCP_STREAM];
    osal_u32      hcc_ack_count[HMAC_TCP_STREAM];               /* ack num in each queue */
    osal_u32      hcc_duplicate_ack_count[HMAC_TCP_STREAM];
    osal_u32      hcc_tcp_ack_no[HMAC_TCP_STREAM];
    osal_spinlock hmac_tcp_ack_lock;
};
struct wlan_tcp_flow {
    osal_u32 src_ip;
    osal_u32 dst_ip;
    osal_u16 src_port;
    osal_u16 dst_port;
    osal_u8 protocol;
};
struct tcp_list_node {
    struct osal_list_head    list;
    osal_u32             used;         /* 0-free, 1-used */
    struct wlan_tcp_flow   wlan_tcp_info;
    osal_u32             index;
    osal_ulong               last_ts;      /* time of last frame */
};
struct wlan_perform_tcp_list {
    struct tcp_list_node    tcp_pool[HMAC_TCP_STREAM];
    osal_u32              free_count;
    osal_spinlock         tcp_lock;       /* lock for tcp pool */
    osal_spinlock         tcp_list_lock;  /* lock for tcp list */
    struct osal_list_head     tcp_list;
};

typedef enum _hmac_tcp_opt_queue_ {
    HMAC_TCP_ACK_QUEUE = 0,
    HMAC_TCP_OPT_QUEUE_BUTT
} hmac_tcp_opt_queue;

typedef osal_u16 (*hmac_trans_cb_func)(void *hmac_device, hmac_tcp_opt_queue type, hcc_chan_type dir, void *data);

/* tcp_ack优化 */
typedef struct {
    struct wlan_perform_tcp hmac_tcp_ack;
    struct wlan_perform_tcp_list hmac_tcp_ack_list;
    wlan_perform_tcp_impls filter_info;
    hmac_trans_cb_func filter[HMAC_TCP_OPT_QUEUE_BUTT]; // 过滤处理钩子函数
    osal_u64 all_ack_count[HMAC_TCP_OPT_QUEUE_BUTT];    // 丢弃的TCP ACK统计
    osal_u64 drop_count[HMAC_TCP_OPT_QUEUE_BUTT];       // 丢弃的TCP ACK统计
    oal_netbuf_head_stru data_queue[HMAC_TCP_OPT_QUEUE_BUTT];
    osal_spinlock data_queue_lock[HMAC_TCP_OPT_QUEUE_BUTT];
} hmac_tcp_ack_stru;

typedef struct {
    /* 定制化是否打开tcp ack过滤 */
    osal_u8 tcp_ack_filter_en;
    /* 当前状态 */
    osal_u8 cur_filter_status;
    osal_u16 rx_filter_throughput_high;
    osal_u16 rx_filter_throughput_low;
    osal_u8 tcp_ack_max_num_start_process;
    osal_u8 resv;
} mac_tcp_ack_filter_stru;
extern mac_tcp_ack_filter_stru g_st_tcp_ack_filter;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
/*****************************************************************************
  11 内联函数定义
*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif   /* end of _PRE_WLAN_TCP_OPT */

#endif /* end of hmac_tcp_opt_struc.h */
