/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: vap / user common info for host and device
 */

#ifndef __WLAN_RESOURCE_COMMON_ROM_H__
#define __WLAN_RESOURCE_COMMON_ROM_H__

#include "td_type.h"
#include "frw_ext_common_rom.h"
#include "wlan_mib_type.h"
#include "hal_ops_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT    5120    /* us */
#define     MAC_FCS_DEFAULT_PROTECT_TIME_OUT3   15000   /* us */
#define     MAC_ONE_PACKET_TIME_OUT_DEFAULT     500
#define     MAC_ONE_PACKET_TIME_OUT             1000
#define     MAC_ONE_PACKET_TIME_OUT3            2000
#define     MAC_FCS_CTS_MAX_DURATION            32767   /* us */

/* Block ack的确认类型 */
typedef enum {
    MAC_BACK_BASIC = 0,
    MAC_BACK_COMPRESSED = 2,
    MAC_BACK_MULTI_TID = 3,

    MAC_BACK_BUTT
} mac_back_variant_enum;
typedef osal_u8 mac_back_variant_enum_uint8;

// AP STA公用状态
typedef enum {
    /* ap sta公共状态 */
    MAC_VAP_STATE_INIT               = 0,
    MAC_VAP_STATE_UP                 = 1, /* VAP UP */
    MAC_VAP_STATE_PAUSE              = 2, /* pause , for ap &sta */
    MAC_VAP_STATE_LISTEN             = 3,

    /* ap 独有状态 */
    MAC_VAP_STATE_AP_WAIT_START      = 4,

    /* sta独有状态 */
    MAC_VAP_STATE_STA_FAKE_UP        = 5,
    MAC_VAP_STATE_STA_WAIT_SCAN      = 6,
    MAC_VAP_STATE_STA_SCAN_COMP      = 7,
    MAC_VAP_STATE_STA_JOIN_COMP      = 8,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2 = 9,
    MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4 = 10,
    MAC_VAP_STATE_STA_AUTH_COMP      = 11,
    MAC_VAP_STATE_STA_WAIT_ASOC      = 12,
    MAC_VAP_STATE_STA_OBSS_SCAN      = 13,
    MAC_VAP_STATE_STA_BG_SCAN        = 14,
    MAC_VAP_STATE_ROAMING            = 15, /* 漫游 */
    MAC_VAP_STATE_BUTT,
} mac_vap_state_enum;
typedef osal_u8  mac_vap_state_enum_uint8;

typedef struct _mac_sta_pm_handler {
    oal_fsm_stru oal_fsm;                    /* 节能状态机 */
    frw_timeout_stru inactive_timer;         /* 定时器 */
    frw_timeout_stru mcast_timer;            /* 接收广播组播超时定时器 */

    osal_u32 tx_rx_activity_cnt;   /* ACTIVE统计值，由超时进入DOZE复位 */
    osal_u32 activity_timeout;     /* 睡眠超时定时器超时时间 */
    osal_u32 ps_keepalive_cnt;     /* STA侧节能状态下keepalive机制统计接收beacon数 */
    osal_u32 ps_keepalive_max_num; /* STA侧节能状态下keepalive机制最大接收beacon数 */

    osal_u8 vap_ps_mode     : 3; /*  sta当前省电模式 */
    osal_u8 can_sta_sleep   : 1;                 /* 协议允许切到doze,是否能投票睡眠 */
    osal_u8 is_fsm_attached : 1; /* 状态机是否已经注册 */
    osal_u8 beacon_counting : 1;
    osal_u8 forbiden_pm     : 1; /* 永久关闭低功耗重启复位仅认证使用 */
    osal_u8 ps_poll_pending : 1; /* P2p function */

    osal_u8 beacon_frame_wait : 1;  /* 提示接收beacon帧 */
    osal_u8 more_data_expected : 1; /* 提示AP中有更多的缓存帧 */
    osal_u8 active_null_wait : 1; /* STA发送NULL帧给AP提示进入ACTIVE 状态 */
    osal_u8 doze_null_wait : 1;   /* STA发送NULL帧给AP提示进入doze状态 */
    osal_u8 direct_change_to_active : 1; /* FAST模式下直接唤醒的数据包切active状态 */
    osal_u8 last_ps_status : 1;            /* 上一次低功耗开启与否 */
    osal_u8 ps_back_active_pause : 1;    /* ps back 延迟发送唤醒null帧 */
    osal_u8 ps_back_doze_pause : 1;      /* ps back 延迟发送睡眠null帧 */

    osal_u8 timer_fail_doze_trans_cnt; /* 超时函数内发null切doze失败次数 */
    osal_u8 state_fail_doze_trans_cnt; /* 切doze时,由于条件不满足失败计数 */

    osal_u8 beacon_fail_doze_trans_cnt; /* 收beacon 投票睡眠却却无法睡下去的计数 */
    osal_u8 doze_event;                 /* 记录切状态的事件类型 */
    osal_u8 awake_event;
    osal_u8 active_event;

    osal_u8 eosp_timeout_cnt; /* uapsd省电中TBTT计数器 */
    osal_u8 uaspd_sp_status;  /* UAPSD的状态 */
    osal_u8 doze_null_retran_cnt;
    osal_u8 active_null_retran_cnt;

    osal_u32 psm_pkt_cnt;

    osal_u8 psm_timer_restart_cnt;   /* 重启睡眠定时器的count */
    osal_u16 mcast_timeout;          /* 接收广播组播定时器超时时间 */
    osal_u8 max_skip_bcn_cnt;        /* 最大允许跳过beacon次数 */

    osal_u8 tbtt_cnt_since_full_bcn; /* 距离上次接收完整beacon的tbtt cnt计数 */
    osal_u16 remain_len_after_tim;   /* 记录beacon中tim ie后剩余字节数 */
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_bool_enum_uint8 flag_null_data_pending;
#endif
    uintptr_t *_rom; /* ROM化后资源扩展指针 */
} mac_sta_pm_handler_stru;

/* P2P OPS 节能配置参数 */
typedef struct {
    osal_u8 ops_ctrl;
    osal_u8 ct_window;
    osal_u8 pause_ops;
    osal_u8 rsv[1];
} mac_cfg_p2p_ops_param_stru;

/* P2P NOA节能配置参数 */
typedef struct {
    osal_u32 start_time;
    osal_u32 duration;
    osal_u32 interval;
    osal_u8 count;
    osal_u8 rsv[3];
} mac_cfg_p2p_noa_param_stru;

/* STA UAPSD 配置命令 */
typedef struct {
    osal_u8 uapsd_switch;
    osal_u8 max_sp_len;
    osal_u8 rsv[2];      /* 保留2字节对齐 */
    osal_u8 delivery_enabled[WLAN_WME_AC_BUTT];
    osal_u8 trigger_enabled[WLAN_WME_AC_BUTT];
} mac_cfg_uapsd_sta_stru;

typedef struct {
    osal_u16 setup_command : 3;
    osal_u16 flow_id : 3;
    osal_u16 flow_type : 1;
    osal_u16 trigger : 1;
    osal_u16 wake_duration_unit : 1; /* duration unit */
    osal_u16 intrval_exponent : 5;   /* interval_exponent */
    osal_u16 resv : 2;

    osal_u16 intrval_mantissa; /* interval_mantissa */

    osal_u8 min_duration; /* wake_duration */
    osal_u8 information_disable;
    osal_u8 resv1[2]; /* 3保留字节 */

    osal_u64 twt; /* us after TSF */
} mac_cfg_twt_basic_param_stru;

/* TWT参数 */
typedef struct {
    osal_u8 twt_session_status;
    osal_u8 dialog_token;  /* HMAC专用 */
    osal_u8 next_twt_size; /* TWT Information帧有用 */
    osal_u8 twt_ps_pause;  /* DMAC专用: TWT 省电队列发送是否处于pause状态, TWT SP期间置为OAL_FALSE */

    osal_u32 twt_interval;
    osal_u32 twt_duration; /* 转换后的Nominal Minimum TWT Wake Duration，单位us */

    mac_cfg_twt_basic_param_stru twt_basic_param;
} mac_cfg_twt_stru;


typedef enum {
    MAC_STA_PM_SWITCH_OFF = 0,        /* 关闭低功耗 */
    MAC_STA_PM_SWITCH_ON = 1,         /* 打开低功耗 */
    MAC_STA_PM_MANUAL_MODE_ON = 2,    /* 开启手动sta pm mode */
    MAC_STA_PM_MANUAL_MODE_OFF = 3,   /* 关闭手动sta pm mode */
    MAC_STA_PM_SWITCH_RESET    = 4,     /* 重置sta pm mode */
    MAC_STA_PM_SWITCH_BUTT,           /* 最大类型 */
    MAC_STA_PM_DISABLE_FOREVER = 255, /* 永久关闭低功耗,仅认证使用 */
} mac_pm_switch_enum;
typedef osal_u8 mac_pm_switch_enum_uint8;

typedef enum {
    MAC_STA_PM_CTRL_TYPE_HOST = 0,    /* 低功耗控制类型 HOST */
    MAC_STA_PM_CTRL_TYPE_MVAP = 1,    /* 低功耗控制类型 多VAP共存 */
    MAC_STA_PM_CTRL_TYPE_MONITOR = 2, /* 低功耗控制类型 MONITOR */
    MAC_STA_PM_CTRL_TYPE_ROAM = 3,    /* 低功耗控制类型 RAOM */
    MAC_STA_PM_CTRL_TYPE_TWT = 4,     /* 低功耗控制类型 TWT */
    MAC_STA_PM_CTRL_TYPE_SDP = 5,     /* 低功耗控制类型 SDP/NAN/WIFI AWARE */
    MAC_STA_PM_CTRL_TYPE_BUTT,        /* 最大类型，应小于 8       */
} mac_pm_ctrl_type_enum;
typedef osal_u8 mac_pm_ctrl_type_enum_uint8;

/* Power save modes specified by the user */
typedef enum {
    NO_POWERSAVE = 0,
    MIN_FAST_PS = 1,
    MAX_FAST_PS = 2,
    MIN_PSPOLL_PS = 3,
    MAX_PSPOLL_PS = 4,
    NUM_PS_MODE = 5
} ps_user_mode_enum;

/* 存储所有的nontrans_bssid_frofile的综合信息 */
typedef struct {
    osal_u8     multi_bss_eid_exist : 1;        /* 置1表示当前bss属于multi bss中的一员 */
    osal_u8     maxbssid_indicator  : 4;        /* 2的n次幂表示集合中bss最大数目 */
    osal_u8     resv                : 3;
    osal_u8     bssid_idx;                      /* 本bss属于multi-bss中的1员时,对应的bssid idx */
    osal_u8     trans_bssid[WLAN_MAC_ADDR_LEN]; /* multi-bss中，发送beacon帧的bss对应的bssid */
} mac_scanned_all_bss_info;
typedef enum {
    MAC_FCS_NOTIFY_TYPE_SWITCH_AWAY    = 0,
    MAC_FCS_NOTIFY_TYPE_SWITCH_BACK,

    MAC_FCS_NOTIFY_TYPE_BUTT
} mac_fcs_notify_type_enum;
typedef osal_u8 mac_fcs_notify_type_enum_uint8;


typedef enum {
    MAC_FCS_STATE_STANDBY = 0, // free to use
    MAC_FCS_STATE_REQUESTED,   // requested by other module, but not in switching
    MAC_FCS_STATE_IN_PROGESS,  // in switching

    MAC_FCS_STATE_BUTT
} mac_fcs_state_enum;
typedef osal_u8 mac_fcs_state_enum_uint8;

typedef enum {
    MAC_FCS_SUCCESS = 0,
    MAC_FCS_ERR_NULL_PTR,
    MAC_FCS_ERR_INVALID_CFG,
    MAC_FCS_ERR_BUSY,
    MAC_FCS_ERR_UNKNOWN_ERR,
} mac_fcs_err_enum;
typedef osal_u8 mac_fcs_err_enum_uint8;


typedef enum {
    MAC_FCS_START_SWITCH_CHNL = 0,
    MAC_FCS_START_SAME_CHNL,
    MAC_FCS_START_SWITCH_CHNL_ENHANCED,
    MAC_FCS_START_SAME_CHNL_ENHANCED,
    MAC_FCS_START_SEND_ONE_PACKET,
    MAC_FCS_START_BUTT
} mac_fcs_type_enum;
typedef osal_u8 mac_fcs_type_enum_uint8;

typedef struct {
    mac_channel_stru dst_chl;
    mac_channel_stru src_chl;
    hal_one_packet_cfg_stru one_packet_cfg;
    mac_fcs_type_enum_uint8         fcs_type;
    osal_u8                         vap_id;
    osal_u8                         resv[2];       /* 预留字节对齐 */
    hal_to_dmac_device_stru *hal_device;
    hal_tx_dscr_queue_header_stru *src_fake_queue; /* 记录此vap自己的fake队列指针 */
    mac_channel_stru src_chl2;
    hal_one_packet_cfg_stru one_packet_cfg2;
} mac_fcs_cfg_stru;

typedef struct {
    mac_fcs_notify_type_enum_uint8 notify_type;
    osal_u8 resv[3]; /* 3 BYTE保留字段 */
    mac_fcs_cfg_stru fcs_cfg;
} mac_fcs_event_stru;

typedef osal_void (*mac_fcs_notify_func)(const mac_fcs_event_stru *);

typedef struct tag_mac_fcs_mgr_stru {
    volatile oal_bool_enum_uint8 fcs_done;
    mac_fcs_state_enum_uint8 fcs_state;
    hal_fcs_service_type_enum_uint8 fcs_service_type;
    osal_u8 resv;

    mac_fcs_cfg_stru *fcs_cfg;
} mac_fcs_mgr_stru;

typedef struct {
    osal_u8 nss_rate;
    osal_u8 protocol_type;
    osal_s8 rssi_dbm;
    osal_s8 snr_ant0;

    osal_u8 freq_bw;
    osal_u8 rate;
    osal_u16 data_cnt;
    osal_u8 gi_type;
    osal_u8 preamble_mode;
    osal_u8 reserved[2]; /* 2：保持字节对齐 */
    osal_u32 transmit_power;
} mac_cfg_tx_user_rate_stru;

typedef enum {
    MAC_MEMINFO_USER = 0,
    MAC_MEMINFO_VAP,
    MAC_MEMINFO_POOL_INFO,
    MAC_MEMINFO_SDIO_TRX,
    MAC_MEMINFO_ALL,

    MAC_MEMINFO_BUTT
} mac_meminfo_cmd_enum;
typedef osal_u8 mac_meminfo_cmd_enum_uint8;

typedef struct {
    mac_meminfo_cmd_enum_uint8 meminfo_type;
    osal_u8 object_index;
} mac_cfg_meminfo_stru;

#ifdef _PRE_WLAN_DFT_STAT
typedef struct {
    osal_u16 user_id;
    osal_u8 auc_resv[2];
    osal_u32 tx_rate;     /* 当前发送速率 */
    osal_u32 rx_rate;     /* 当前接收速率 */
} mac_cfg_query_rate_stru;
#endif

typedef struct {
    osal_u8 data_retry_times;
    osal_u8 mgmt_retry_times;
} mac_cfg_soft_retry_times_stru;

/* resource_rom */
typedef struct {
    osal_ulong user_idx_size;
} mac_res_user_idx_size_stru;

typedef struct {
    osal_u8 user_cnt_size;
} mac_res_user_cnt_size_stru;

typedef union {
    struct {
        osal_u8 trace : 1;
        osal_u8 off   : 1;
        osal_u8 dump  : 1;
        osal_u8 icmp  : 1;
        osal_u8 auth  : 1;
        osal_u8 arp   : 1;
        osal_u8 eapol : 1;
        osal_u8 assoc : 1;
    } bits;
    osal_u8 val;
} mac_pkt_debug_ctl_stru;

#define SERVICE_CONTROL_SDP BIT7

/* 32位全局变量控制32种业务场景 */
typedef struct {
    osal_u32 mask;
    osal_u32 value;
} service_control_msg;

/*****************************************************************************
 功能描述  : 判断是否为相同信道
*****************************************************************************/
static inline oal_bool_enum_uint8 mac_fcs_is_same_channel(const mac_channel_stru *channel_dst,
    const mac_channel_stru *channel_src)
{
    return  channel_dst->chan_number == channel_src->chan_number ? OAL_TRUE : OAL_FALSE;
}

typedef struct {
    osal_u16 user_id;
    osal_u8  resv[2]; /* 预留2字节对齐 */
    osal_u32 tx_best_rate;
} mac_cfg_ar_tx_params_stru;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_resource_common.h */
