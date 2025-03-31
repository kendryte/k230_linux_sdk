/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Header file of device and host hmac_ext_if public interface.
 * Create: 2022-11-26
 */

#ifndef __DMAC_EXT_IF_TYPE_H__
#define __DMAC_EXT_IF_TYPE_H__

#include "dmac_ext_if_type_rom.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

typedef enum {
    DMAC_DISASOC_MISC_LINKLOSS             = 0,
    DMAC_DISASOC_MISC_KEEPALIVE            = 1,
    DMAC_DISASOC_MISC_CHANNEL_MISMATCH     = 2,
    DMAC_DISASOC_MISC_LOW_RSSI             = 3,
    DMAC_DISASOC_MISC_GET_CHANNEL_IDX_FAIL = 4,

    DMAC_DISASOC_MISC_BUTT
} dmac_disasoc_misc_reason_enum;
typedef osal_u16 dmac_disasoc_misc_reason_enum_uint16;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef enum {
    BTCOEX_RX_WINDOW_SIZE_INDEX_0 = 0,
    BTCOEX_RX_WINDOW_SIZE_INDEX_1 = 1,
    BTCOEX_RX_WINDOW_SIZE_INDEX_2 = 2,
    BTCOEX_RX_WINDOW_SIZE_INDEX_3 = 3,

    BTCOEX_RX_WINDOW_SIZE_INDEX_BUTT
} btcoex_rx_window_size_index_enum;
typedef osal_u8 btcoex_rx_window_size_index_enum_uint8;

typedef enum {
    BTCOEX_RATE_THRESHOLD_MIN = 0,
    BTCOEX_RATE_THRESHOLD_MAX = 1,

    BTCOEX_RATE_THRESHOLD_BUTT
} btcoex_rate_threshold_enum;
typedef osal_u8 btcoex_rate_threshold_enum_uint8;

typedef enum {
    BTCOEX_RX_WINDOW_SIZE_HOLD = 0,
    BTCOEX_RX_WINDOW_SIZE_DECREASE = 1,
    BTCOEX_RX_WINDOW_SIZE_INCREASE = 2,

    BTCOEX_RX_WINDOW_SIZE_BUTT
} btcoex_rx_window_size_enum;
typedef osal_u8 btcoex_rx_window_size_enum_uint8;

typedef enum {
    BTCOEX_RX_WINDOW_SIZE_GRADE_0 = 0,
    BTCOEX_RX_WINDOW_SIZE_GRADE_1 = 1,

    BTCOEX_RX_WINDOW_SIZE_GRADE_BUTT
} btcoex_rx_window_size_grade_enum;
typedef osal_u8 btcoex_rx_window_size_grade_enum_uint8;

typedef enum {
    BTCOEX_ACTIVE_MODE_A2DP = 0,
    BTCOEX_ACTIVE_MODE_SCO = 1,
    BTCOEX_ACTIVE_MODE_TRANSFER = 2,

    BTCOEX_ACTIVE_MODE_BUTT
} btcoex_active_mode_enum;
typedef osal_u8 btcoex_active_mode_enum_uint8;
#endif

typedef struct {
    osal_u8                            cac_machw_en;          /* 1min cac 发送队列开关状态 */
} dmac_set_cac_machw_info_stru;

/* wait join写寄存器参数的结构体定义 */
typedef struct {
    osal_u32              dtim_period;                  /* dtim period */
    osal_u32              dtim_cnt;                     /* dtim count  */
    osal_u8               bssid[WLAN_MAC_ADDR_LEN];    /* 加入的AP的BSSID  */
    osal_u16              tsf_bit0;                     /* tsf bit0  */
} dmac_ctx_set_dtim_tsf_reg_stru;

typedef struct {
    osal_u8  reason;
    osal_u16 event;
    osal_u8  des_addr[WLAN_MAC_ADDR_LEN];
    osal_u8  resv[3];
} dmac_diasoc_deauth_event;

typedef struct {
    osal_u32  cycles;                     /* 统计间隔时钟周期数 */
    osal_u32  sw_tx_succ_num;             /* 软件统计发送成功ppdu个数 */
    osal_u32  sw_tx_fail_num;             /* 软件统计发送失败ppdu个数 */
    osal_u32  sw_rx_ampdu_succ_num;       /* 软件统计接收成功的ampdu个数 */
    osal_u32  sw_rx_mpdu_succ_num;        /* 软件统计接收成功的mpdu个数 */
    osal_u32  sw_rx_ppdu_fail_num;        /* 软件统计接收失败的ppdu个数 */
    osal_u32  hw_rx_ampdu_fcs_fail_num;   /* 硬件统计接收ampdu fcs校验失败个数 */
    osal_u32  hw_rx_mpdu_fcs_fail_num;    /* 硬件统计接收mpdu fcs校验失败个数 */
} dmac_thruput_info_sync_stru;

typedef enum {
    DMAC_CONFIG_IPV4 = 0,                /* 配置IPv4地址 */
    DMAC_CONFIG_IPV6,                    /* 配置IPv6地址 */
    DMAC_CONFIG_BUTT
} dmac_ip_type;
typedef osal_u8 dmac_ip_type_enum_uint8;

typedef enum {
    DMAC_IP_ADDR_ADD = 0,                /* 增加IP地址 */
    DMAC_IP_ADDR_DEL,                    /* 删除IP地址 */
    DMAC_IP_OPER_BUTT
} dmac_ip_oper;
typedef osal_u8 dmac_ip_oper_enum_uint8;

typedef struct {
    dmac_ip_type_enum_uint8         type;
    dmac_ip_oper_enum_uint8         oper;

    osal_u8                         resv[2];

    osal_u8                         ip_addr[OAL_IP_ADDR_MAX_SIZE];
    osal_u8                         mask_addr[OAL_IP_ADDR_MAX_SIZE];
} dmac_ip_addr_config_stru;

typedef enum {
    COMMON_DEBUG_KEEPALIVE = 0,
    COMMON_DEBUG_BUTT
} mac_common_debug_enum;

typedef enum {
    P2P_PROTOCOL_BGN = 0,
    P2P_PROTOCOL_AX,
    P2P_PROTOCOL_BUTT
} p2p_protocol_debug_enum;
typedef osal_u8 p2p_protocol_debug_enum_uint8;

typedef enum {
    P2P_BAND_20M = 0,
    P2P_BAND_40M,
    P2P_BAND_BUTT
} p2p_band_debug_enum;
typedef osal_u8 p2p_band_debug_enum_uint8;

/* P2P 整体的通用信息 */
typedef struct {
    p2p_protocol_debug_enum_uint8 protocol_mode; /* 协议模式 */
    p2p_band_debug_enum_uint8 band_width;        /* 协议模式 */
    osal_u8 enable;                      /* 判断当前是否使能 */
    osal_u8 resv;
} p2p_common_info_stru;

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif // __DMAC_EXT_IF_TYPE_ROM_H__
