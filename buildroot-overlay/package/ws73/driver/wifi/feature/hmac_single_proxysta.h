/*
 * Copyright (c) CompanyNameMagicTag 2017-2023. All rights reserved.
 * 文 件 名   : hmac_single_proxysta.h
 * 生成日期   : 2017年5月15日
 * 功能描述   : hmac_single_proxysta.c 的头文件
 */
#ifndef __HMAC_SINGLE_PROXYSTA_H__
#define __HMAC_SINGLE_PROXYSTA_H__
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SINGLE_PROXYSTA_H

/*****************************************************************************
    全局变量定义
*****************************************************************************/
#define HMAC_BRIDGE_MAP_IPV4_HASHSIZE         MAC_VAP_BRIDGE_MAP_MAX_VALUE
#define HMAC_BRIDGE_MAP_UNKNOW_HASHSIZE       MAC_VAP_BRIDGE_MAP_UNKNOW_VALUE
#define HMAC_BRIDGE_MAP_AGING_TIME            120000          /* BRIDGE MAP表格老化时间 120s */
#define HMAC_BRIDGE_MAP_AGING_TRIGGER_TIME    60000           /* MAP老化计时器触发时间 60s */
#define HMAC_BRIDGE_MAP_MAX_NUM               16              /* MAP表格大小，当前限制为16 */
#define DHCP_PORT_BOOTPS   0x0043  /* DHCP 服务端接收报文端口,即客户端发送报文的目的端口 */
#define DHCP_PORT_BOOTPC   0x0044  /* DHCP 客户端接收报文端口,即服务器发送报文的目的端口 */
#define DHCP_FLAG_BCAST    0x8000  /* 请求DHCP服务器以广播形式回复DHCP response */
#define MAC_VAP_BRIDGE_MAP_MAX_VALUE        4              /* BRIDGE HASH表的桶值 目前设置4个桶 */
#define MAC_VAP_BRIDGE_MAP_UNKNOW_VALUE     4              /* BRIDGE HASH表的未知协议报文桶值 目前4个桶 */
#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
#define BRIDGE_CMD_NAME_MAX_LEN             8
#define BRIDGE_PORT_MAX_NUM                 2
#define BRIDGE_CMD_MAX_NUM                  5
#endif
/* 哈希函数定义 */
#define hmac_bridge_cal_ipv4_hash(_puc_ip_addr)     \
    ((_puc_ip_addr)[ETH_TARGET_IP_ADDR_LEN - 1] & (HMAC_BRIDGE_MAP_IPV4_HASHSIZE - 1))

#define hmac_bridge_cal_unknow_hash(_us_protocol)     \
    ((_us_protocol) & (HMAC_BRIDGE_MAP_UNKNOW_HASHSIZE - 1))

/*****************************************************************************
    STRUCT定义
*****************************************************************************/
typedef struct {
    struct osal_list_head   entry;
    osal_u8       ipv4[ETH_TARGET_IP_ADDR_LEN];       /* 记录对应的ipv4地址 */
    osal_u8       mac[WLAN_MAC_ADDR_LEN];             /* 记录对应的mac地址 */
    osal_u8       rsv[2];
    osal_u32      last_active_timestamp;              /* 最近一次发送数据的时间 */
} hmac_bridge_ipv4_hash_stru;

typedef struct {
    struct osal_list_head     entry;
    osal_u16      protocol;                           /* 记录对应的未知协议类型 */
    osal_u8       mac[WLAN_MAC_ADDR_LEN];             /* 记录对应的mac地址 */
    osal_u32      last_active_timestamp;              /* 最近一次发送数据的时间 */
} hmac_bridge_unknow_hash_stru;

typedef struct {
    osal_spinlock         map_lock;                                          /* 读写保护锁 */
    struct osal_list_head map_ipv4_head[MAC_VAP_BRIDGE_MAP_MAX_VALUE];      /* IPV4的map hash表 */
    struct osal_list_head map_unknow_head[MAC_VAP_BRIDGE_MAP_UNKNOW_VALUE]; /* 未知协议的map hash表 */
    osal_u8               map_ipv4_num;        /* 记录ipv4的条数 */
    osal_u8               map_unknow_num;      /* 记录未知协议的条数 */
    osal_u16              resv;
} hmac_vap_bridge_stru;

typedef struct {
    osal_u8                 vap_id;
    hmac_vap_bridge_stru   *vap_bridge;
    frw_timeout_stru       st_bridge_map_timer;
} hmac_single_proxysta_stru;
#ifdef _PRE_WLAN_FEATURE_LOCAL_BRIDGE
typedef osal_void(*hmac_bridge_cmd_func)(const osal_char *param);

typedef struct {
    osal_char cmd_name[BRIDGE_CMD_NAME_MAX_LEN];
    hmac_bridge_cmd_func func;
} hmac_bridge_cmd_stru;

typedef struct {
    hmac_vap_stru *repeat_sta;
    hmac_vap_stru *repeat_ap;
    osal_u8 br_switch;
    osal_u8 rsv[3];
} hmac_brctl_stru;

typedef struct {
    td_char cmd_name[BRIDGE_CMD_NAME_MAX_LEN];
    td_char if_name[BRIDGE_CMD_NAME_MAX_LEN];
} mac_cfg_brctl_stru;
#endif
/*****************************************************************************
    函数声明
*****************************************************************************/
typedef osal_u32 (*hmac_bridge_tx_process_cb)(const oal_netbuf_stru *netbuf, hmac_vap_stru *hmac_vap);
typedef osal_u32 (*hmac_bridge_rx_process_cb)(const oal_netbuf_stru *netbuf, const hmac_vap_stru *hmac_vap);

static osal_u32 hmac_single_proxysta_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_single_proxysta_init"), used));
static osal_void hmac_single_proxysta_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_single_proxysta_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_unique_proxysta.h */
