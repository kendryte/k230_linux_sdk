/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_VAP_MANAGE_ROM_H
#define MSG_VAP_MANAGE_ROM_H

#include "osal_types.h"
#include "dmac_ext_if_type_rom.h"

/* cfg id对应的参数结构体 */
/* 创建VAP参数结构体, 对应cfgid: WLAN_CFGID_ADD_VAP */
typedef struct {
    wlan_vap_mode_enum_uint8 vap_mode;
    osal_u8 cfg_vap_indx;
    osal_u16 muti_user_id; /* 添加vap 对应的muti user index */

    osal_u8 vap_id;                    /* 需要添加的vap id */
    osal_u8 hal_vap_id;                /* 需要添加的hal vap id */
    wlan_p2p_mode_enum_uint8 p2p_mode; /* 0:非P2P设备; 1:P2P_GO; 2:P2P_Device; 3:P2P_CL */

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_u8 is_11ac2g_enable : 1;
    osal_u8 disable_capab_2ght40 : 1;
    osal_u8 er_su_disable : 1;
    osal_u8 dcm_constellation_tx : 2;
    osal_u8 bandwidth_extended_range : 1;
    osal_u8 bit_reserve : 2;
#else
    osal_u8 resv0[1];
#endif

    osal_u8 chba_mode : 1;
    osal_u8 disable_11r : 1;
    osal_u8 resv1 : 6;
    osal_u8 tim_bitmap_len;
    osal_u8 resv2[2]; /* 预留2字节对齐 */

    oal_bool_enum_uint8 custom_switch_11ax;
    osal_u8 resv4[3]; /* 预留3字节对齐 */
    oal_net_device_stru *net_dev;
} mac_cfg_add_vap_param_stru;

typedef mac_cfg_add_vap_param_stru mac_cfg_del_vap_param_stru;

/* 同步需要从ko侧同步到device侧的dmac_vap结构体成员 */
typedef struct {
    mac_vap_state_enum_uint8 vap_state;
    osal_u16 sta_aid;
    osal_u8 uapsd_cap;
    osal_u8 vap_mac[WLAN_MAC_ADDR_LEN];
    osal_u8 auth_received        : 1; /* 接收了auth */
    osal_u8 assoc_rsp_received   : 1; /* 接收了assoc */
    osal_u8 protocol             : 5; /* 协议模式 */
    osal_u8 keepalive            : 1;
    osal_u8 bcn_tout_max_cnt;    /* beacon连续收不到最大睡眠次数 */
    osal_u16 in_tbtt_offset;     /* 内部tbtt offset配置值 */
    osal_u16 beacon_timeout;     /* beacon timeout配置值 */
    mac_cfg_p2p_ops_param_stru p2p_ops_param;
    mac_cfg_p2p_noa_param_stru p2p_noa_param;
    mac_channel_stru channel; /* vap所在的信道 */
    dmac_linkloss_rate_uint8 linkloss_rate; /* linkloss信息同步 */
    osal_u8 resv;
    osal_u16 beacon_interval;     /* beacon interval配置值 */
} mac_update_dmac_vap_stru;

#endif