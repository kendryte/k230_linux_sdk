/*
 * Copyright (c) @CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_WOW_ROM_H
#define MSG_WOW_ROM_H

#include "msg_arp_offload_rom.h"
#include "msg_rekey_offload_rom.h"
#include "msg_dhcp_offload_rom.h"

#ifndef OAL_IP_ADDR_MAX_SIZE
#define OAL_IP_ADDR_MAX_SIZE    16
#endif

/* 修改参数 */
#define WOW_NETPATTERN_MAX_NUM 4
#define WOW_NETPATTERN_MAX_LEN 64

typedef enum {
    MAC_IP_ADDR_DEL = 0,                /* 删除IP地址 */
    MAC_IP_ADDR_ADD,                    /* 增加IP地址 */
    MAC_IP_OPER_BUTT
} mac_ip_oper;
typedef osal_u8 mac_ip_oper_enum_uint8;

typedef enum {
    MAC_CONFIG_IPV4 = 0,                /* 配置IPv4地址 */
    MAC_CONFIG_IPV6,                    /* 配置IPv6地址 */
    MAC_CONFIG_BUTT
} mac_ip_type;
typedef osal_u8 mac_ip_type_enum_uint8;

typedef struct {
    mac_ip_type_enum_uint8 type;
    mac_ip_oper_enum_uint8 oper;
    osal_u8 resv[2]; /* 填充2字节 */
    union {
        osal_u32 ipv4; /* 注意：网络字节序、大端。 */
        osal_u8 ipv6[OAL_IP_ADDR_MAX_SIZE];
    } ip;
} mac_ip_addr_config_stru;

typedef struct {
    osal_u8 pattern_data[WOW_NETPATTERN_MAX_LEN];
    osal_u32 pattern_len;
} wow_pattern_stru;

typedef struct {
    wow_pattern_stru pattern[WOW_NETPATTERN_MAX_NUM];
    osal_u16 pattern_map;
    osal_u16 pattern_num;
} wow_pattern_cfg_stru;
typedef struct {
    osal_u8 wow_en;
    osal_u8 res[3];
    osal_u32 wow_event;
    wow_pattern_cfg_stru wow_pattern;
} mac_wow_cfg_stu;

typedef enum {
    MAC_WOW_SLEEP_NOTIFY_MSG,
    MAC_WOW_SYNC_DATA_MSG,
    MAC_WOW_MSG_BUTT,
} mac_wow_msg_enum;
typedef osal_u8 mac_wow_msg_enum_uint8;

typedef struct {
    mac_wow_msg_enum_uint8 msg_type; /* 传输的消息类型 */
    osal_u8 notify_param;            /* 睡眠通知参数 */
    osal_u8 auc_resv[2];             /* 2 BYTE保留字段 */
} mac_h2d_syn_info_hdr_stru;

typedef struct {
    mac_arp_offload_param_sync arp_param;
    mac_dhcp_offload_param_sync dhcp_param;
    mac_rekey_offload_param_sync rekey_param;
    mac_wow_cfg_stu wow_cfg;
} wow_param_sync_stru;

typedef struct {
    osal_u16 file_remain_len;
    osal_u16 block_data_len;
    osal_u8 msg_type;
    osal_u8 block_id;
    osal_u8 rsv[2];
} mac_wow_load_info_stru;

/* WOW_OFFLOAD模块收发包统计的数据结构 */
typedef struct {
    osal_u8 wow_rcv_magic : 1,
            wow_rcv_disasoc : 1,
            wow_rcv_deauth : 1,
            wow_rcv_net_pattern_tcp : 1,
            wow_rcv_net_pattern_udp : 1,
            rsv1 : 3;
    osal_u8 rsv2[3];
    osal_u32 arp_offload_rcv_arp_req;
    osal_u32 arp_offload_rcv_ns;
    osal_u32 rekey_offload_rcv_eapol;
    osal_u32 arp_offload_send_arp_rsp;
    osal_u32 arp_offload_send_ns;
    osal_u32 arp_offload_send_free_arp;
    osal_u32 dhcp_offload_send_dhcp_renew;
    osal_u32 rekey_offload_send_eapol;
} mac_wow_offload_info_stru;

typedef struct {
    mac_wow_offload_info_stru mac_wow_offload_info;
    osal_u8 wakeup_reason;
} wow_result_to_host_stru;

#endif
