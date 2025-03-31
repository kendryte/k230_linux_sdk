/*
* Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
* Description: mac csa
* Create: 2020-7-5
*/

#ifndef HMAC_CSA_AP_H
#define HMAC_CSA_AP_H

#include "wlan_types_common.h"
#include "mac_vap_ext.h"
#include "mac_device_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/* CSA调试开关相关的结构体 */
typedef struct {
    osal_bool mode;
    osal_u8 channel;
    osal_u8 cnt;
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    mac_csa_flag_enum_uint8 debug_flag; /* 0:正常切信道; 1:仅beacon帧中含有csa,信道不切换;2:取消beacon帧中含有csa */
    osal_u8 reserv[3];              /* 预留3字节对齐 */
} mac_csa_debug_stru;

/* 带宽调试开关相关的结构体 */
typedef struct {
    osal_u32            cmd_bit_map;
    oal_bool_enum_uint8 band_force_switch_bit0;         /* 恢复40M带宽命令 */
    oal_bool_enum_uint8 en_2040_ch_swt_proosal_bit1;    /* 不允许20/40带宽切换开关 */
    oal_bool_enum_uint8 intolerant_40_bit2;             /* 不容忍40M带宽开关 */
    osal_u8 resv;
    mac_csa_debug_stru  csa_debug_bit3;
    oal_bool_enum_uint8 lsigtxop_bit5;  /* lsigtxop使能 */
    osal_u8             resv0[3];       /* 预留3字节对齐 */
} mac_protocol_debug_switch_stru;

typedef struct {
    osal_u8 announced_channel;                             /* 新信道号 */
    wlan_channel_bandwidth_enum_uint8 announced_bandwidth; /* 新带宽模式 */
    osal_u8 ch_switch_cnt;                                 /* 信道切换计数 */
    osal_bool delay_update_switch_cnt;                     /* 标记是否延迟更新switch cnt */
    wlan_ch_switch_status_enum_uint8 ch_switch_status;     /* 信道切换状态 */
    oal_bool_enum_uint8 csa_present_in_bcn;                /* Beacon帧中是否包含CSA IE */
    osal_bool csa_mode; /* 0:csa期间允许发包 1:csa期间不允许发包 */
    osal_u8 resv;
} hmac_csa_ap_info_stru;

/*****************************************************************************
  对外函数声明
*****************************************************************************/
typedef osal_bool (*hmac_csa_ap_is_ch_switch_cb)(osal_u8 vap_id);
typedef osal_void (*hmac_csa_ap_encap_beacon_csa_ie_cb)(hmac_vap_stru *hmac_vap, osal_u8 *buffer,
    osal_u8 *ie_len);
typedef oal_bool_enum_uint8 (*hmac_csa_ap_csa_present_in_bcn_cb)(osal_u8 vap_id);
typedef osal_void (*hmac_csa_ap_switch_new_channel_cb)(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth, osal_bool disable_tx, osal_u8 switch_cnt);

static osal_u32 hmac_csa_ap_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_csa_ap_init"), used));
static osal_void hmac_csa_ap_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_csa_ap_deinit"), used));
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
