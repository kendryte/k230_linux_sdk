/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: msg.
 * Create: 2021-11-29
 */

#ifndef MSG_USER_MANAGE_ROM_H
#define MSG_USER_MANAGE_ROM_H

#include "osal_types.h"

/*
 * hmac向dmac同步速率集合信息时，使用的结构体大小超出了事件内存的大小，导致事件同步有可能失败。
 * 单独定义结构体mac_h2d_user_rate_info_stru,保证mac_h2d_usr_info_stru不超过限制
 */
typedef struct {
    osal_u16 user_idx;
    osal_u16 amsdu_maxsize;

    mac_he_hdl_stru he_hdl;
    /* vht速率集信息 */
    mac_vht_hdl_stru vht_hdl;

    /* ht速率集信息 */
    mac_user_ht_hdl_stru ht_hdl;

    /* legacy速率集信息 */
    osal_u8 avail_rs_nrates;
    osal_u8 resv[3]; /* 保留3字节对齐 */
    osal_u8 avail_rs_rates[WLAN_MAX_SUPP_RATES];
} mac_h2d_usr_rate_info_stru;

typedef enum {
    MAC_AP_TYPE_NORMAL = 0,
    MAC_AP_TYPE_GOLDENAP = 1,
    MAC_AP_TYPE_DDC_WHITELIST = 2,
    MAC_AP_TYPE_BTCOEX_PS_BLACKLIST = 3, /* 这里是探测双天线ap，和ddc互斥,可以直接后面添加 */

    MAC_AP_TYPE_BUTT
} mac_ap_type_enum;
typedef osal_u8 mac_ap_type_enum_uint8;

/* 添加用户事件payload结构体 */
typedef struct {
    osal_u16  user_idx;     /* 用户index */
    osal_u8   user_mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u16  sta_aid;
    osal_u8   bssid[WLAN_MAC_ADDR_LEN];
    mac_he_hdl_stru             he_hdl;
    mac_vht_hdl_stru            vht_hdl;
    mac_user_ht_hdl_stru        ht_hdl;
    mac_ap_type_enum_uint8      ap_type;
    osal_s8                     rssi; /* 用户bss的信号强度 */
    mac_user_type_enum_uint8    user_type;
    osal_u8                     lut_index; /* user对应的硬件索引，活跃用户id */
    osal_u8                     rev;
} hmac_ctx_add_user_stru;

/* 删除用户事件结构体 */
typedef struct {
    osal_u16 user_idx; /* 用户index */
    osal_u8 user_mac_addr[WLAN_MAC_ADDR_LEN];
    mac_user_type_enum_uint8 user_type;
    mac_ap_type_enum_uint8 ap_type;
    osal_u8 resv[2]; /* 修复删除del新增成员，结构大小尽量不要超过event第一级内存72字节 */
} hmac_ctx_del_user_stru;

/* 需要从ko侧同步到device侧的结构体成员 */
typedef struct {
    osal_u16 assoc_id;
    osal_u8 user_mac_addr[WLAN_MAC_ADDR_LEN];

    wlan_ciper_protocol_type_enum_uint8 cipher_type;
    /* 算法使用 start */
    osal_u16 amsdu_maxsize; /* 如果支持amsdu 保存协商后amsdu发送的最大值 */

    osal_u8 avail_protocol_mode : 4; /* 用户和VAP协议模式交集, 供算法调用 */
    osal_u8 avail_bandwidth : 4; /* 用户和VAP带宽能力交集,供算法调用 */
    osal_u8 cur_bandwidth         : 4; /* 默认值与en_avail_bandwidth相同,供算法调用修改 */
    osal_u8 avail_num_spatial_stream : 3; /* Tx和Rx支持Nss的交集,供算法调用 */
    osal_u8 vip_flag              : 1; /* 只算法调度用，通过配置命令配置，TRUE: VIP用户, FALSE: 非VIP用户 */
    osal_u8 user_num_spatial_stream     : 3; /* 用户支持的空间流个数 */
    osal_u8 avail_bf_num_spatial_stream : 3; /* 用户支持的Beamforming空间流个数 */
    osal_u8 is_multi_user : 1;
    osal_u8 qos : 1;
    osal_u8 protocol_mode       : 4;
    osal_u8 cur_protocol_mode : 4; /* 默认值与en_avail_protocol_mode值相同, 供算法调用修改 */
    mac_user_asoc_state_enum_uint8 user_asoc_state; /* 用户关联状态 */

    /* 算法使用 end */
    osal_u8 lut_index;
    osal_u8 resv1[3];
    mac_user_tx_param_stru user_tx_info; /* TX相关参数 */
    mac_key_mgmt_stru key_info;
} mac_user_info_h2d_sync_stru;

#endif