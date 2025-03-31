/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_btcoex_rom.c 的头文件。
 * Create: 2020-07-03
 */

#ifndef __HMAC_BTCOEX_H__
#define __HMAC_BTCOEX_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "frw_ext_if.h"
#include "oal_ext_if.h"
#include "mac_frame.h"

#include "hal_ext_if.h"
#include "mac_resource_ext.h"
#include "mac_frame.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_feature_main.h"
#include "hal_coex_reg.h"
#include "msg_user_manage_rom.h"

#include "hal_chip.h"
#include "hmac_auto_adjust_freq.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WLAN_TIDNO_COEX_QOSNULL     WLAN_TIDNO_ANT_TRAINING_HIGH_PRIO   /* 发送qos null复用智能天线高优先级训练帧 */

#define BTCOEX_RSSI_THRESHOLD               (WLAN_FAR_DISTANCE_RSSI)

#define BTCOEX_RX_LOW_RATE_TIME             (5000)
#define BTCOEX_SCO_CALCULATE_TIME           (500)

#define BTCOEX_PRIO_TIMEOUT_150MS           (150)    // 软件定时器操作精度是ms
#define BTCOEX_PRIO_TIMEOUT_100MS           (100)    // 软件定时器操作精度是ms
#define BTCOEX_PRIO_TIMEOUT_60MS            (60)
#define BTCOEX_PRIO_TIMEOUT_50MS            (50)

#define BTCOEX_PRIO_TIMEOUT_ALWAYS_ON       (0xffff) // 持续拉高
#define BTCOEX_PRIO_TIMEOUT_20MS            (20000)  // 硬件单位为us
#define BTCOEX_PRIO_TIMEOUT_10MS            (10000)
#define BTCOEX_PRIO_TIMEOUT_5MS             (5000)
#define BTCOEX_PRIO_TIMEOUT_ALWAYS_OFF      (0x0)    // 拉低

#define ALL_MID_PRIO_TIME                   (10)    // 5s / BTCOEX_SCO_CALCULATE_TIME
#define ALL_HIGH_PRIO_TIME                  (4)     // 2s / BTCOEX_SCO_CALCULATE_TIME

#define BTCOEX_LINKLOSS_OCCUPIED_NUMBER     (8)
#define BTCOEX_POW_SAVE_CNT                 (5)    /* 连续处于ps=1状态次数，超过时，wifi强制恢复(用于低功耗唤醒时) */

#define BTCOEX_20M_40PLUS_CHAN_OFFSET       (2)
#define BTCOEX_40MINUS_CHAN_OFFSET          (6)
#define BTCOEX_20M_CHAN_BITMASK             (0x1F) /* 20M信道使用5bit掩码，例如ch6占用ch4~8 */
#define BTCOEX_40M_CHAN_BITMASK             (0x1FF) /* 40M信道使用9bit掩码，例如ch6+10占用ch4~12 */
#define BTCOEX_2G_ALL_CHAN_BITMASK          (0x3FFF) /* 1~14信道掩码 */
#define BTCOEX_WORK_MODE_BIT_STA            (1) /* work mode bitmap */
#define BTCOEX_WORK_MODE_BIT_AP             (2)
#define BTCOEX_WORK_MODE_BIT_GC             (4)
#define BTCOEX_WORK_MODE_BIT_GO             (8)

#define BTCOEX_ARP_FAIL_DELBA_NUM           2
#define BTCOEX_ARP_FAIL_REASSOC_NUM         4
#define BTCOEX_ARP_PROBE_TIMEOUT            (300)  // ms

#define BTCOEX_BLE_CONN_NUM_THOLD           4      // BLE多连接场景：调整BA连接数阈值


/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    HMAC_BTCOEX_PS_SWITCH_OFF = 0,
    HMAC_BTCOEX_PS_SWITCH_ON = 1,
    HMAC_BTCOEX_PS_TIMOUT = 2,
    HMAC_BTCOEX_PS_ABORT = 3,  /* 被扫描等事件打断 */
    HMAC_BTCOEX_PS_SWITCH_BUTT
} hmac_btcoex_ps_switch_enum;
typedef osal_u8 hmac_btcoex_ps_switch_enum_uint8;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    mac_header_frame_control_stru   frame_control;
    osal_u16                      duration_value      : 15,           /* duration/id */
                                duration_flag       : 1;
    osal_u8                       address1[WLAN_MAC_ADDR_LEN];
    osal_u8                       address2[WLAN_MAC_ADDR_LEN];
    osal_u8                       address3[WLAN_MAC_ADDR_LEN];
    osal_u16                      sc_frag_num     : 4,                /* sequence control */
                                sc_seq_num      : 12;
    osal_u8                       qc_tid          : 4,
                                qc_eosp         : 1,
                                qc_ack_polocy   : 2,
                                qc_amsdu        : 1;
} hmac_btcoex_qosnull_frame_stru;

typedef struct {
    osal_u8 ps_stop;
    osal_u8 band;
    osal_u8 legacy_connect_state;
    osal_u8 p2p_connect_state;
    osal_u8 channel_num;
    osal_u8 bandwidth;
    osal_u16 chan_bitmap; /* bit0-13表示channel 1-14 */
    osal_u8 work_mode;
    osal_u8 diff_chan;
    osal_u8 protocol_11bgn;
    osal_u8 protocol_11ax;
}hmac_btcoex_ps_stru;

typedef struct {
    osal_u32 rx_rate_threshold_min;
    osal_u32 rx_rate_threshold_max;
    osal_u8 get_addba_req_flag   : 1;
    osal_u8 ba_size_expect_index : 2; /* 期望聚合大小索引 */
    osal_u8 ba_size_real_index   : 2;   /* 实际聚合大小索引 */
    osal_u8 ba_size_tendence     : 2;
    osal_u8 delba_trigger        : 1;
    osal_u8 ba_size;
    wlan_nss_enum_uint8 user_nss_num;
    osal_u8 resv;
} hmac_user_btcoex_delba_stru;

typedef struct {
    osal_u64 rx_rate_mbps;
    osal_u16 rx_rate_stat_count;
    osal_u8 auc_resv[2];  /* 保留2个字节用作对齐 */
} hmac_user_btcoex_rx_info_stru;

typedef struct {
    osal_u16 last_baw_start;       /* 上一次接收到ADDBA REQ中的baw_start值 */
    osal_u16 last_seq_num;         /* 上一次接收到ADDBA REQ中的seq_num值 */
    btcoex_blacklist_type_enum_uint8 blacklist_tpye;     /* 黑名单方案 */
    oal_bool_enum_uint8              ba_handle_allow; /* 黑名单方案1:是否允许共存删建聚合 黑名单方案2:是否允许建立聚合 */
} hmac_btcoex_addba_req_stru;

typedef struct {
    frw_timeout_stru delba_opt_timer;    /* 发送ARP REQ后启动定时器 */
    osal_atomic rx_unicast_pkt_to_lan;    /* 接收到的单播帧个数 */
} hmac_btcoex_arp_req_process_stru;

typedef struct {
    hmac_user_btcoex_delba_stru      hmac_user_btcoex_delba;
    hmac_user_btcoex_rx_info_stru    hmac_user_btcoex_rx_info;
    hmac_btcoex_arp_req_process_stru hmac_btcoex_arp_req_process;
    hmac_btcoex_addba_req_stru       hmac_btcoex_addba_req;
    osal_u16                         ba_size;
    osal_u8                          rx_no_pkt_count;                /* 超时时间内没有收到帧的次数 */
    oal_bool_enum_uint8              delba_btcoex_trigger;           /* 是否btcoex触发删建BA */
    oal_bool_enum_uint8              arp_probe_on;                   /* 是否打开arp统计，做重关联保护 */
} hmac_user_btcoex_stru;

typedef struct {
} hmac_vap_btcoex_mgr_stru;

typedef struct {
    frw_timeout_stru bt_coex_low_rate_timer;
    frw_timeout_stru bt_coex_statistics_timer;
    oal_bool_enum_uint8 rx_rate_statistics_flag;
    oal_bool_enum_uint8 rx_rate_statistics_timeout;
    osal_u8 auc_resv[2]; /* 2byte保留字段 */
} hmac_vap_btcoex_rx_statistics_stru;

typedef struct {
    frw_timeout_stru bt_coex_priority_timer; /* 读取寄存器周期定时器 */
    frw_timeout_stru bt_coex_occupied_timer; /* 周期拉高occupied信号线，保证WiFi不被BT抢占 */
    osal_u32 ap_beacon_count;
    osal_u32 timestamp;
    osal_u8 beacon_miss_cnt;
    osal_u8 occupied_times;
    osal_u8 prio_occupied_state;
    osal_u8 linkloss_occupied_times;
    osal_u8 linkloss_index;
    osal_u8 auc_resv[3]; /* 保留3个字节用于对齐 */
} hmac_vap_btcoex_occupied_stru;

typedef struct {
    osal_u8 null_qosnull_frame[32];     /* null&qos null,取最大长度32 */
    osal_u16 cfg_coex_tx_vap_index : 4; /* 03新增premmpt帧配置参数 */
    osal_u16 cfg_coex_tx_qos_null_tid : 4;
    osal_u16 bit_rsv : 3;
    osal_u16 cfg_coex_tx_peer_index : 5;
} hmac_vap_btcoex_null_preempt_stru;

typedef struct {
    frw_timeout_stru bt_coex_double_chain_timer; /* 双链路延迟处理定时器 delba或者m2s都一样 */
    oal_bool_enum_uint8 bitmap_update_flag;      /* 业务bitmap是否更新标记 */
    oal_bool_enum_uint8 delba_on_6slot;          /* 支持双链接下，6slot业务是否已经删聚合 */
    osal_u8 bitmap_6slot;                        /* 支持双链接下，6slot业务类型 */
    hal_coex_hw_preempt_mode_enum_uint8 ps_type; /* ps机制one pkt帧类型，默认是null data */

    hmac_vap_btcoex_rx_statistics_stru hmac_vap_btcoex_rx_statistics;
    hmac_vap_btcoex_occupied_stru hmac_vap_btcoex_occupied;
    hmac_vap_btcoex_null_preempt_stru null_preempt_param;
    hal_coex_hw_preempt_mode_enum_uint8 all_abort_preempt_type;
    uintptr_t *_rom; /* vap btcoex开启4个字节空间 */
} hmac_vap_btcoex_stru;

typedef struct {
    osal_u8 cfg_btcoex_mode;            /* 0:参数查询模式; 1:参数配置模式 */
    osal_u8 cfg_btcoex_type;            /* 0:门限类型; 1:聚合大小类型 */
    wlan_nss_enum_uint8 btcoex_nss;     /* 0:siso 1:mimo */

    union {
        struct {
            osal_u8 low_20m;   /* 2G 20M low */
            osal_u8 high_20m;  /* 2G 20M high */
            osal_u8 low_40m;   /* 2G 40M low */
            osal_u16 high_40m; /* 2G 40M high */
        } threhold;

        struct {
            osal_u8 grade;    /* 0级或者1级 */
            osal_u8 rx_size0; /* size0大小 */
            osal_u8 rx_size1; /* size1大小 */
            osal_u8 rx_size2; /* size2大小 */
            osal_u8 rx_size3; /* size3大小 */
        } rx_size;
    } pri_data;
} mac_btcoex_mgr_stru;

typedef struct {
    osal_u8 user_mac_addr[WLAN_MAC_ADDR_LEN];     /* 黑名单MAC地址 */
    osal_u8 type;                                  /* 写入黑名单的类型 */
    osal_u8 used;                                  /* 是否已经写过黑名单MAC地址 */
} hmac_btcoex_delba_exception_stru;

typedef struct {
    hmac_btcoex_delba_exception_stru hmac_btcoex_delba_exception[BTCOEX_BSS_NUM_IN_BLACKLIST];
    osal_u8 exception_bss_index;       /* 黑名单MAC地址的数组下标 */
    osal_u8 auc_resv[3];
} hmac_device_btcoex_stru;

typedef struct {
    osal_u16 tx_throughput;
    osal_u16 rx_throughput;
} hmac_btcoex_throughput_stru;

/*****************************************************************************
  8 ROM预留回调函数类型定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_u8 hmac_get_g_rx_ba_win_size(osal_u32 index1, osal_u32 index2);
osal_u8 hmac_get_g_rx_win_size_siso(osal_u32 index1, osal_u32 index2);
osal_u8 hmac_get_g_rx_win_size_grage_siso(osal_u32 index1, osal_u32 index2, osal_u32 index3);
osal_u8 hmac_get_g_rx_win_size_mimo(osal_u32 index1, osal_u32 index2);
osal_u8 hmac_get_g_rx_win_size_grage_mimo(osal_u32 index1, osal_u32 index2, osal_u32 index3);
osal_void hmac_btcoex_set_rx_win_size(const mac_btcoex_mgr_stru *btcoex_mgr);
osal_void hmac_btcoex_restart_ps_timer(hal_to_dmac_device_stru *hal_device);
osal_void hmac_btcoex_ps_status_handler(osal_void);
osal_u32 hmac_btcoex_config_tx_aggr_num(osal_u8 aggt_num);
osal_void hmac_btcoex_set_mgmt_priority(const hmac_vap_stru *hmac_vap, osal_u16 timeout_ms);
osal_void hmac_btcoex_status_info_dump(const hal_to_dmac_device_stru *hal_device, hal_chip_stru *hal_chip);
osal_u32 hmac_btcoex_wlan_priority_timeout_callback(osal_void *arg);
osal_u32 hmac_btcoex_rx_rate_statistics_flag_callback(osal_void *arg);
osal_void hmac_btcoex_update_rx_rate_threshold(const hmac_vap_stru *hmac_vap,
    hmac_user_btcoex_delba_stru *btcoex_delba);
osal_u8 hmac_btcoex_find_all_valid_sta_per_device(const hal_to_dmac_device_stru *hal_device,
    osal_u8 *vap_id, osal_u16 vap_id_len);
osal_u8 hmac_btcoex_find_all_valid_ap_per_device(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id,
    osal_u16 vap_id_size);

osal_void hmac_btcoex_blacklist_handle_init(hmac_user_stru *hmac_user);

static inline hmac_user_btcoex_stru *hmac_btcoex_get_user_info(hmac_user_stru *hmac_user)
{
    return (hmac_user_btcoex_stru *)hmac_user_get_feature_ptr(hmac_user, WLAN_FEATURE_INDEX_BTCOEX);
}
static inline hmac_vap_btcoex_stru *hmac_btcoex_get_vap_info(hmac_vap_stru *hmac_vap)
{
    return (hmac_vap_btcoex_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_BTCOEX);
}
static inline btcoex_blacklist_type_enum_uint8 hmac_btcoex_get_blacklist_type(hmac_user_stru *hmac_user)
{
    return hmac_btcoex_get_user_info(hmac_user)->hmac_btcoex_addba_req.blacklist_tpye;
}

/*****************************************************************************
 功能描述  : 获取指定vap的双链接下6slot业务类型的bitmap
*****************************************************************************/
static INLINE__ osal_u8 hmac_vap_get_vap_btcoex_mgr_6slot(hmac_vap_stru *hmac_vap)
{
    return hmac_btcoex_get_vap_info(hmac_vap)->bitmap_6slot;
}
#endif

typedef osal_void (*hmac_btcoex_recover_coex_priority_cb)(hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_btcoex_recover_coex_priority_clear_cb)(hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device);
typedef osal_void (*hmac_btcoex_set_wlan_priority_cb)(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 value,
    osal_u8 timeout_ms);
typedef osal_void (*hmac_btcoex_rx_process_ect_cb)(hmac_vap_stru *hmac_vap, osal_u8 frame_subtype,
    oal_netbuf_stru *netbuf, oal_bool_enum_uint8 ampdu);
typedef osal_void (*hmac_btcoex_linkloss_update_threshold_cb)(hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_btcoex_linkloss_occupied_process_cb)(
    const hal_to_dmac_chip_stru *hal_chip, const hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap);
typedef osal_u32 (*hmac_config_btcoex_assoc_state_syn_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
typedef osal_void (*hmac_btcoex_roam_succ_handler_cb)(hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_btcoex_check_exception_in_list_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 *addr_sa);
typedef osal_u32 (*hmac_btcoex_proc_user_add_cb)(hmac_user_stru *hmac_user);
typedef osal_u32 (*hmac_btcoex_proc_vap_add_cb)(hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_btcoex_tx_mgmt_frame_cb)(const hmac_vap_stru *hmac_vap,
    const mac_ieee80211_frame_stru *mac_header);
typedef osal_void (*hmac_btcoex_tx_vip_frame_cb)(hmac_vap_stru *hmac_vap, const oal_netbuf_head_stru *buff_headr,
    osal_u8 mpdu_num);
typedef osal_void (*hmac_btcoex_check_rx_same_baw_start_from_addba_req_etc_cb)(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, mac_ieee80211_frame_stru *frame_hdr, const osal_u8 *action);
typedef osal_void (*hmac_btcoex_user_spatial_stream_change_notify_cb)(const hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user);
typedef osal_void (*hmac_btcoex_set_txop_alg_cb)(hal_tx_txop_alg_stru *txop_alg);
typedef osal_void (*hmac_btcoex_update_bss_list_protocol_cb)(mac_bss_dscr_stru *bss_dscr, osal_u8 *frame_body,
    osal_u16 frame_len);
typedef osal_u32 (*hmac_btcoex_check_ap_type_blacklist_cb)(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    osal_u8 *mac_addr, wlan_nss_enum_uint8 *support_max_nss);
typedef osal_void (*hmac_btcoex_clear_arp_timer_cb)(hmac_user_stru *hmac_user);
typedef osal_void (*hmac_btcoex_keepalive_timer_ect_cb)(osal_u32 *aging_time);
typedef osal_u32 (*hmac_btcoex_check_user_req_declined_cb)(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
typedef osal_u32 (*hmac_btcoex_check_sw_preempt_type_cb)(const hmac_vap_stru *hmac_vap,
    const hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hmac_btcoex_compatibility_set_ps_type_cb)(hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_btcoex_compatibility_set_ap_type_cb)(hmac_vap_stru *hmac_vap, mac_ap_type_enum_uint8 ap_type);
typedef osal_void (*hmac_btcoex_vap_resume_tx_by_chl_cb)(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hmac_btcoex_vap_config_sw_preempt_subtype_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hmac_btcoex_device_exit_destroy_timer_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_void (*hmac_btcoex_report_linkloss_info_cb)(const hmac_vap_stru *hmac_sta);
typedef osal_void (*hmac_btcoex_sw_preempt_type_check_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_u16 (*hmac_btcoex_hal_get_btstatus_bton_cb)(osal_void);
typedef osal_void (*hmac_btcoex_hal_blinkloss_clean_cb)(hmac_vap_stru *hmac_vap);
typedef osal_u32 (*hmac_btcoex_hal_set_aging_time_cb)(hmac_vap_stru *hmac_vap,  osal_u32 *aging_time);
typedef osal_void (*hmac_btcoex_hal_set_btcoex_wifi_status_cb)(const hmac_vap_stru *hmac_vap, osal_u32 status);
typedef osal_u32 (*hmac_btcoex_hal_end_scan_check_ps_cb)(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap);
typedef osal_void (*hmac_btcoex_hal_user_add_handle_cb)(hmac_vap_stru *hmac_vap,  mac_user_type_enum_uint8 user_type);
typedef osal_void (*hmac_btcoex_hal_wifi_busy_notify_cb)(osal_u32 tx_throughput_mbps, osal_u32 rx_throughput_mbps);

static osal_u32 hmac_btcoex_init_weakref(osal_void) __attribute__ ((weakref("hmac_btcoex_init"), used));
static osal_void hmac_btcoex_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_btcoex_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __HMAC_BTCOEX_H__ */

