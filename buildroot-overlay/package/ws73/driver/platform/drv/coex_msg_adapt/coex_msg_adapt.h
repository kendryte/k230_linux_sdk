/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: host coex msg adapt source file
 * Author: BGTP
 * Create: 2023-08-03
 */

#ifndef __COEX_MSG_ADAPT_H__
#define __COEX_MSG_ADAPT_H__

#include "td_type.h"

#ifdef _PRE_BSLE_GATEWAY

/* 共存WiFi发送给BLE消息的类型枚举 */
typedef enum {
    BTCOEX_WIFI_MSG_BW_CHANNEL = 0,  /* wifi发送2.4G带宽及信道信息 */
    BTCOEX_WIFI_MSG_STATUS = 1,  /* 发送WiFi开关及关联状态，及低功耗状态组合信息 */
    BTCOEX_WIFI_MSG_SLOT = 2,  /* 通知蓝牙要提前多少slot通知wifi发送self-cts.(1slot=625us) */
    BTCOEX_WIFI_MSG_RSSI = 3,  /* 发送wifi的rssi值，以便wifi在弱场情况下, 蓝牙可适当降低功率 */
    BTCOEX_WIFI_MSG_BUTT = 8  /* 8是极限值，不可超过8个枚举 */
} mac_btcoex_wifi_msg_type_enum;

/* 共存BLE发送给WiFi消息的类型枚举 */
typedef enum {
    BTCOEX_BLE_MSG_SCAN = 0,       /* 发送蓝牙扫描状态 */
    BTCOEX_BLE_MSG_STATUS = 1,     /* 发送蓝牙开关，连接及数传状态 */
    BTCOEX_BLE_MSG_BROADCAST = 2,  /* 发送蓝牙广播状态 */
    BTCOEX_BLE_MSG_DEVICE_NUM = 3, /* 发送蓝牙当前连接的设备个数 */
    BTCOEX_BLE_MSG_CTS = 4,        /* 通知wifi的发送self-cts帧 */
    BTCOEX_BLE_MSG_AGGR_LEVEL = 5,  /* 通知wifi的聚合档位 */
    BTCOEX_BLE_MSG_BUTT = 8        /* 8是极限值，不可超过8个枚举 */
} mac_btcoex_bsle_msg_type_enum;

#pragma pack(push, 1) /* 1 byte alignment, and push current alignment's config */
/* 接收到蓝牙数据时，解析成如下结构体，再根据msg_type解析成对应的结构体进行赋值 */
typedef union {
    struct {
        uint8_t open_status : 1,          /* wifi开关状态，0关闭，1打开 */
                connect_status : 1,       /* wifi连接状态，0没2.4G设备连接；1有2.4G设备连接 */
                power_status : 1,         /* wifi处于低功耗模式，0表示退出；1表示进入 */
                resv : 2,                 /* 保留位 */
                msg_type : 3;              /* 共存通知的事件类型 */
    } wifi_status_msg;
    struct {
        uint8_t channel : 4,              /* 2.4G信道，1~14 */
                bw : 1,                   /* 2.4G带宽，0: 20M; 1: 40M */
                msg_type : 3;              /* 共存通知的事件类型 */
    } wifi_freq_msg;
    struct {
        uint8_t slot_num : 5,             /* 通知蓝牙要提前多少slot通知wifi发送self-cts */
                msg_type : 3;              /* 共存通知的事件类型 */
    } wifi_slot_msg;
    struct {
        uint8_t rssi_num : 5,             /* 通知蓝牙wifi此时的RSSI档位 */
                msg_type : 3;              /* 共存通知的事件类型 */
    } wifi_rssi_msg;

    struct {
        uint8_t ble_open_status : 1,      /* 蓝牙开关状态，0关闭，1打开 */
                ble_connect_status : 1,   /* 蓝牙连接状态，0没设备连接；1有设备连接 */
                ble_thrans : 1,           /* 蓝牙数据传输状态，0没数据传输；1有数据传输 */
                resv : 2,                 /* 保留位 */
                msg_type : 3;              /* 共存通知的事件类型 */
    } ble_status_msg;
    struct {
        uint8_t ble_scan_status : 1,       /* 蓝牙扫描状态，0扫描结束，1扫描开始 */
                ble_self_cts_duration : 4, /* 蓝牙扫描一帧的扫描业务时长，最多16个slot */
                msg_type : 3;              /* 共存通知的事件类型 */
    } ble_scan_msg;
    struct {
        uint8_t ble_broadcast_status : 1,  /* 蓝牙广播状态，0广播结束，1广播开始 */
                ble_self_cts_duration : 4, /* 蓝牙广播一帧的业务时长，最多16个slot */
                msg_type : 3;              /* 共存通知的事件类型 */
    } ble_broadcast_msg;
    struct {
        uint8_t ble_device_num : 5,        /* 蓝牙连接设备个数 */
                msg_type : 3;              /* 共存通知的事件类型 */
    } ble_device_num_msg;
    struct {
        uint8_t ble_self_cts_duration : 5, /* 通知wifi发送self-cts的业务时长，0表示蓝牙业务已结束 */
                msg_type : 3;              /* 共存通知的事件类型 */
    } ble_self_cts_duration_msg;
    struct {
        uint8_t ble_wifi_aggr_level : 5,   /* 通知wifi发送聚合档位,5档 */
                msg_type : 3;              /* 共存通知的事件类型 */
    } ble_wifi_aggr_msg;

    struct {
        uint8_t val : 5,                  /* 共存通知的事件内容 */
                msg_type : 3;              /* 共存通知的事件类型 */
    } coex_msg;
} mac_bslecoex_event_stru;
#pragma pack(pop) /* restore */

typedef union {
        struct {
            td_u32  bsle_on           : 1;
            td_u32  bsle_scan         : 1;
            td_u32  bsle_adv          : 1;
            td_u32  bsle_init         : 1;
            td_u32  bsle_conn         : 1;
            td_u32  bsle_hid          : 1;
            td_u32  bsle_conn_num     : 5;
            td_u32  bsle_trans        : 1;
            td_u32  bsle_scan_cts_dur : 4;
            td_u32  bsle_adv_cts_dur  : 4;
            td_u32  is_chip_out       : 1;
            td_u32  bsle_delba_level  : 3;
            td_u32  bsle_rsv          : 8;
        };

    td_u32 bsle_status_stru;
} coex_bsle_status_tx_gateway_t;

typedef union {
    /* Define the struct bits */
    struct {
        td_u32     wlan_on               : 1; /* WIFI：1：on,0:off */
        td_u32     wlan_channel_num      : 4; /* 信道：1-14 */
        td_u32     wlan_bandwidth        : 4; /* 带宽: 20/40M */
        td_u32     wlan_connect          : 1; /* 连接 */
        td_u32     wlan_power_status     : 1; /* 是否低功耗模式 */
        td_u32     wlan_busy             : 1; /* 是否wifi忙碌 */
        td_u32     wlan_slot_num         : 5; /* 提前多少帧通知发self-cts */
        td_u32     wlan_rssi_num         : 5; /* wifi rssi 档位 */
        td_u32     wlan_rsv              : 10;
    };

    /* Define an unsigned member */
    td_u32 wifi_status_stru;
} coex_wl_status_rx_gateway_t;

td_void coex_bsle_hcc_msg_plat_host_to_device_init(td_void);
#endif /* _PRE_BSLE_GATEWAY */
#endif /* __COEX_MSG_ADAPT_H__ */