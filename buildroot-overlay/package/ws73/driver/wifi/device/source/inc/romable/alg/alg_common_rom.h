/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: vap / user common info for host and device
 */

#ifndef __ALG_COMMON_ROM_H__
#define __ALG_COMMON_ROM_H__

#include "alg_autorate_common_rom.h"
#include "alg_common_macros_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  3 结构体
*****************************************************************************/
/* GLA功能 各算法ID，唯一识别某个算法，ID=2的整数值 */
typedef enum {
    ALG_GLA_ID_AUTORATE    = 0x00000001,
    ALG_GLA_ID_RTS         = 0x00000002,
    ALG_GLA_ID_AGGR        = 0x00000004,
    ALG_GLA_ID_DBAC        = 0x00000008,
    ALG_GLA_ID_TPC         = 0x00000010,
    ALG_GLA_ID_TRAFFIC_CTL = 0x00000020,
    ALG_GLA_ID_SCHEDULE    = 0x00000040,
    ALG_GLA_ID_INTF_DET   = 0x00000080,
    ALG_GLA_ID_WEAK_IMMUNE = 0x00000100,
    ALG_GLA_ID_CCA         = 0x00000200,
    ALG_GLA_ID_EDCA        = 0x00000400,
    ALG_GLA_ID_TEMP_PRTECT = 0x00000800,
    ALG_GLA_ID_TXBF        = 0x00001000,

    ALG_GLA_ID_BUTT
} alg_gla_id_enum;
typedef osal_u32 alg_gla_id_enum_uint32;

/* TXMODE LOG 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_enum_uint16 alg_cfg;      /* 配置命令枚举 */
    osal_u8 ac_no;                       /* AC类型 */
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    osal_u8 auc_resv1[1];
    osal_u16 value; /* 配置参数值 */
} mac_ioctl_alg_txbf_log_param_stru;

/* 算法配置命令接口 */
typedef struct {
    osal_u8 argc;
    osal_u8 argv_offset[DMAC_ALG_CONFIG_MAX_ARG];
} mac_ioctl_alg_config_stru;

/* TPC LOG 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_enum_uint16 alg_cfg;      /* 配置命令枚举 */
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN]; /* MAC地址 */
    osal_u8 ac_no;                       /* AC类型 */
    osal_u8 rev[1];
    osal_u16 value;                      /* 配置参数值 */
    osal_char *frame_name;               /* 获取特定帧功率使用该变量 */
} mac_ioctl_alg_tpc_log_param_stru;

/* cca opt LOG 算法参数枚举，参数值 */
typedef struct {
    mac_alg_cfg_enum_uint16 alg_cfg; /* 配置命令枚举 */
    osal_u16 value;                 /* 统计总时间 */
} mac_ioctl_alg_intfdet_log_param_stru;

/* 传到device侧的消息内容 */
typedef struct {
    oal_bool_enum_uint8 coch_intf_state; /* 同频干扰的检测状态 */
    oal_bool_enum_uint8 coch_intf_state_last; /* 指示之前同频干扰的检测状态 */
    hal_alg_intf_det_mode_enum_uint8 adjch_intf_type; /* 指示邻频干扰模式 */
    hal_alg_intf_det_mode_enum_uint8 adjch_intf_type_last; /* 指示之前的邻频干扰模式 */
    alg_intf_det_type_enum_uint8 intf_det_type;    /* 通知device时指示当前干扰检测类型 */
    osal_u8 resv[3];                    /* 对齐 */
    osal_u16 duty_cyc_ratio_20;         /* pri20干扰繁忙度(千分之x) */
    osal_u16 duty_cyc_ratio_sec20;      /* sec20干扰繁忙度(千分之x) */
} alg_intf_det_notify_info_stru;

typedef osal_u32 (*p_alg_anti_intf_switch_func)(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 alg_enable);
typedef osal_u32 (*p_alg_anti_intf_tx_time_notify_func)(hal_to_dmac_device_stru *hal_device);
typedef osal_u32 (*p_alg_bfee_report_pow_adjust_notify)(hal_to_dmac_device_stru *hal_device);
typedef enum {
    ALG_TPC_QUERY_CONFIG_PARA = 0,
    ALG_TPC_QUERY_STATUS = 1,
    ALG_TPC_QUERY_ALL_POWER = 2,
    ALG_TPC_QUERY_TPC_CODE_TABLE = 3,
    ALG_TPC_QUERY_ACK_MODE_OPEN = 4,
    ALG_TPC_QUERY_ACK_MODE_CLOSE = 5,
    ALG_TPC_QUERY_STUB_INTF = 6,
    ALG_TPC_QUERY_STUB_INTF_CLEAR = 7,
    ALG_TPC_QUERY_STUB_OVER_TEMP = 8,
    ALG_TPC_QUERY_STUB_OVER_TEMP_CLEAR = 9,
    ALG_TPC_QUERY_STUB_CHANGE_CHANNEL = 10,
    ALG_TPC_QUERY_STUB_ACK_RSSI_NEAR = 11,
    ALG_TPC_QUERY_STUB_ACK_RSSI_FAR = 12,
    ALG_TPC_QUERY_STUB_ACK_RSSI_CLEAR = 13,
    ALG_TPC_QUERY_TYPE_BUTT
} alg_tpc_query_type_enum;
typedef enum {
    ALG_TPC_DBG_OFF  = 0,    /* 关闭调试日志 */
    ALG_TPC_DBG_DIST = 1,    /* 只在距离变化时打印 */
    ALG_TPC_DBG_PROBE = 2,   /* 打印探测相关信息 */
    ALG_TPC_DBG_PKT  = 3,    /* 打印每一包信息 */
    ALG_TPC_DBG_BUTT
} alg_tpc_dbg_enum;
typedef osal_u8 alg_tpc_dbg_enum_uint8;
typedef enum {
    CFG_DBAC_LED_VAP_IDX = 0,
    CFG_DBAC_FLW_VAP_IDX,

    CFG_DBAC_VAP_IDX_BUTT
} alg_dbac_vap_idx_enum;
typedef osal_u8 alg_dbac_vap_idx_enum_uint8;
/* TPC需要从host同步参数的结构体 */
typedef struct {
    /* para start */
    osal_s8 tpc_normal2near_thres;
    osal_s8 tpc_near2normal_thres;
    osal_s8 tpc_normal2far_thres;
    osal_s8 tpc_far2normal_thres;
    osal_s8 near_distance_rssi;
    osal_s8 far_distance_rssi;
    osal_u8 tpc_normal2near_rate_gap_cnt;
    osal_u8 tpc_normal2near_rate_gap;
    osal_u8 tpc_normal2far_rate_gap;
    osal_u8 tpc_ack_mode : 1;
    osal_u8 fix_power_level : 3;
    osal_u8 data_rf_limit_enable : 1;
    osal_u8 resv_bit : 3;
    osal_u8 resv[2];
    /* para end */
} alg_param_sync_tpc_stru;
typedef struct {
    osal_u16 config_dscr_restore_thres; /* 强制配置的预留长度，如果不为0，优先使用配置值 */
    osal_u16 dscr_default_thres;        /* 默认预留长度，算法未打开时使用默认值 */
    osal_u16 pkt_low_thres;
    osal_u16 pkt_high_thres;
    osal_u16 pkt_busy_thres;
    osal_u16 dscr_min_thres;
    osal_u16 dscr_low_thres;
    osal_u16 dscr_high_thres;
    osal_u16 dscr_busy_thres;
    osal_u16 resv[1];
} alg_traffic_rx_ctl_stru;

typedef struct {
    osal_u8 flowctl_flag; /* true: dont send, false: send */
    osal_u8 resv[1];
    osal_u16 tx_continue_on_cycle; /* if flowctl keep on, inc 1 */
    osal_u16 tx_on_cnt; /* flowctl count */
    osal_u16 tx_off_cnt; /* flowctl count */
    osal_u16 tx_stop_thres;
    osal_u16 tx_start_thres;
} alg_traffic_tx_ctl_stru;

typedef struct {
    osal_u8 vap_id;
    osal_u8 hal_vap_id;
    osal_u8 vap_state;
    osal_u8 hal_vap_state;
    osal_u8 vap_ppdu_count[HAL_TX_QUEUE_NUM];
    osal_u8 vap_mpdu_count[HAL_TX_QUEUE_NUM];
    osal_u8 vap_fake_ppdu_count[HAL_TX_QUEUE_NUM];
    osal_u8 vap_fake_mpdu_count[HAL_TX_QUEUE_NUM];
} alg_traffic_vap_stats;

typedef struct {
    /* tx_stats */
    osal_u32 netbuf_alloc_fail;
    osal_u32 netbuf_alloc_succ;
    osal_u32 netbuf_release;
    osal_u32 tx_max_flowcnt_cycle;
} alg_traffic_stats_stru;

typedef struct {
    /* rx_stats */
    osal_u8  normal_used;
    osal_u8  normal_free;
    osal_u8  small_free;
    osal_u8  high_free;
    osal_u8  rx_normal_thres;
    osal_u8  rx_normal_element_cnt;
    osal_u8  rx_small_thres;
    osal_u8  rx_small_element_cnt;
    osal_u32 rx_normal_q_empty;
    osal_u32 rx_small_q_empty;
    osal_u32 rx_high_q_empty;
    osal_u16 rx_normal_smooth_dscr;
    osal_u16 rx_normal_smooth_pkt;
    osal_u16 rx_small_smooth_dscr;
    osal_u16 rx_small_smooth_pkt;
    /* vap stats */
    alg_traffic_vap_stats vap_stats[2];
    /* hw stats */
    osal_u8 hw_mpdu_count[HAL_TX_QUEUE_NUM];
    osal_u8 hw_ppdu_count[HAL_TX_QUEUE_NUM];
} alg_traffic_status_stru;

typedef struct {
    wlan_tpc_mode_enum_uint8 tpc_mode;         /* TPC当前工作模式 */
    osal_u8 is_over_temp      : 1;             /* 是否超温 */
    osal_u8 is_tpc_registered : 1;             /* 算法注册标志 */
    osal_u8 resvbit1          : 2;
    wlan_tpc_mode_enum_uint8 tpc_cfg_mode : 4; /* 存储TPC配置的工作模式 */
    alg_tpc_dbg_enum_uint8 debug_flag;         /* 调试开关等级 */
    osal_u8 fix_pwr_code;          /* 固定发送功率码 */
    osal_u8 adj_intf_mode : 4;    /* CCA上报的邻频干扰模式 */
    osal_u8 co_intf_mode : 1;     /* 同频干扰模式通知 */
    osal_u8 test_nearackrssi : 1;      /* 上板ST使用 */
    osal_u8 test_farackrssi : 1;       /* 上板ST使用 */
    osal_u8 need_distance_change : 1;  /* 需要触发一次距离变化 */
    osal_u8 control_frm_power_level;
    osal_u8 intrf_mode_flag;
    osal_u8 rev;
#ifdef _PRE_WLAN_TPC_DO_PROBE
    osal_u8 min_probe_intvl_pktnum;
    osal_u8 max_probe_intvl_pktnum;
    /* 指向已分配地址空间的TPC可调速率门限值 */
    alg_tpc_target_rate_index_stru alg_tpc_target_rate_index_dev;
#endif
} alg_tpc_common_info_stru;
typedef struct {
    osal_s16 tpc_rssi[ALG_TPC_RSSI_BUTT];
    osal_u8 resv;
    osal_u8 wlan_bw;             /* 带宽 */
} alg_tpc_user_rssi_stru;
/* 用户距离信息结构体 */
typedef struct {
    wlan_user_distance_enum_uint8 distance_id;
    oal_bool_enum_uint8 distance_id_valid;
    wlan_user_distance_enum_uint8 rssi_distance;   /* 根据当前rssi判断用户的距离 */
    wlan_user_distance_enum_uint8 last_distance_id;
    osal_u8 low_rate_wait_count;                  /* 低速率保持次数 */
    osal_u8 resv[3];
} alg_tpc_user_distance_stru;
typedef struct {
    osal_s16 assoc_id;
    osal_u8 per;
    osal_u8 best_rate_index;
    alg_tpc_user_rssi_stru rssi;
    alg_tpc_user_distance_stru distance;
} alg_tpc_user_query_status_stru;
typedef struct {
    osal_u8 traffic_ctl_enable;          /* 拥塞控制开关 */
    osal_u8 log_debug_enable;            /* oam日志记录开关 */
    osal_u8 tx_traffic_ctl_enable;       /* TX方向拥塞控制开关 */
    osal_u8 rx_traffic_ctl_enable;       /* RX方向拥塞控制开关 */
    osal_u16 traffic_ctl_cycle_ms;       /* 拥塞控制定时器周期 */

    osal_u8 window_size;                 /* 发送，回滞区间 */
    osal_u8 netbuf_reserve_size;         /* 发送，预留长度 */
    osal_u8 busy_ctl;                    /* 发送, 控制是否需要根据业务忙碌减少拥塞控制次数 */
    osal_u8 shift_size;                  /* 接收，控制平滑系数 */
    osal_u16 tx_max_flowctl_cycle;       /* 控制发送方向最长持续流控次数，等于0xFFFF时表示不限制次数 */
    alg_traffic_rx_ctl_stru rx_ctl[HAL_RX_DSCR_QUEUE_ID_BUTT];     /* 接收拥塞控制阈值和参数 */
} alg_param_sync_traffic_ctl_stru;
typedef struct {
    osal_u16 vap_tbtt_channel_mismatch;
    osal_u16 vap_tx_cb_check_mismatch_cnt;
    osal_u16 vap_tx_pause_cnt;
    osal_u16 vap_tx_vap_id_mismatch_cnt;
    osal_u16 vap_tx_noa_start_bias_cnt;
    osal_u16 vap_tx_noa_end_bias_cnt;
    osal_u16 vap_one_packet_send_fail_cnt;
    osal_u16 vap_preempt_cnt;
    osal_u16 vap_tbtt_isr_cnt;
    osal_u16 vap_channel_switch_cnt;
} alg_dbac_vap_stats_info;

typedef struct {
    osal_u16 sync_sta_tsf_to_ap_cnt;
    osal_u16 sync_go_tsf_to_sta_cnt;
    osal_u16 tx_netbuf_alloc_fail_cnt;
    osal_u16 led_tbtt_channel_err_cnt;
    osal_u16 noa_slot_num_err_cnt;
    osal_u16 timer_isr_cnt;
    osal_u16 noa_start_isr_cnt;
    osal_u16 noa_end_isr_cnt;
    osal_u32 noa_start_tsf;
} alg_dbac_dev_stats_info;
typedef struct {
    osal_u8 dbac_type : 4;
    osal_u8 dbac_state : 4;
    osal_u8 dbac_pause : 4;
    osal_u8 dual_sta_mode : 4;
    osal_u8 vap_idx[CFG_DBAC_VAP_IDX_BUTT];
    alg_dbac_dev_stats_info dev_stats;
    alg_dbac_vap_stats_info vap_stats[CFG_DBAC_VAP_IDX_BUTT];
    osal_u8 resv_netbuf_cnt[CFG_DBAC_VAP_IDX_BUTT];
    osal_u8 resv;
} alg_dbac_stats_info;

/* rts rate 结构体 */
typedef struct {
    osal_u8 non_erp_rate  : 4;
    osal_u8 erp_rate      : 4;
} alg_rts_rate_elem_stru;
typedef struct {
    alg_rts_rate_elem_stru rate_elem[HAL_TX_RATE_MAX_NUM];
} alg_rts_rate_tbl_stru;
typedef struct {
    osal_u32 tolerant_rts_all_fail_cnt       : 4;    /* 可容忍的RTS All fail次数 */
    osal_u32 per_multiple                    : 11;   /* per扩大的倍数，默认扩大1024倍 */
    osal_u32 loss_ratio_max_num              : 9;    /* 首包错包率最大门限，大于该门限清零 */
    osal_u32 loss_ratio_rts_retry_num        : 4;    /* 首包错误率统计重传次数门限，小于该重传次数时，保持1024 */
    osal_u32 loss_ratio_stat_retry_num       : 4;    /* 首包错误率统计重传次数门限，小于该重传次数时，统计值减半 */

    osal_s16 per_stat_far_rssi_thr;                  /* 更新PER时判定当前是否为强信号的门限 */
    osal_s16 per_stat_normal_rssi_thr;               /* 更新PER时判定当前是否为强信号的门限 */

    osal_u32 first_pkt_stat_shift            : 4;    /* 首包错误率的默认统计除2位移值(只针对最优速率) */
    osal_u32 first_pkt_stat_intvl            : 8;    /* 默认首包错误率的统计间隔包数目 */
    osal_u32 legacy_1st_loss_ratio_th        : 10;   /* LEGACY帧的默认首包错误率(单位:千分数) */
    osal_u32 ht_vht_1st_loss_ratio_th        : 10;   /* HT/VHT帧的默认首包错误率(单位:千分数) */

    osal_s16 open_rts_rssi_high_thr;                 /* RSSI小于该门限时，打开RTS */
    osal_s16 open_rts_rssi_low_thr;                  /* RSSI大于该门限时，打开RTS */

    osal_u32 txop_duration_rts_threshold_max : 10;   /* dot11TxopRTSThreshold的最大值 */
    osal_u32 txop_duration_rts_threshold_on  : 1;    /* dot11TxopRTSThreshold是否开启 */
    osal_u32 rts_threshold                   : 12;   /* dot11RTSThreshold的最大值 */
    osal_u32 kbps_to_bpus                    : 10;   /* kbps转换到bpus */

    osal_u32 rts_duration_multiple           : 10;   /* 与rts空口相加，与ppdu空口比较，判断ppdu空口是否需要rts保护 */
    osal_u32 rts_duration_open               : 1;    /* 是否开启用ppdu空口开销决策是否开关rts */
    osal_u32 rts_rssi_open                   : 1;    /* 是否启用弱信号场景打开RTS的功能 */
    osal_u32 rts_rate_descend                : 1;    /* RTS速率是否主动降档 */

    osal_u32 reserved                        : 18;   /* 4字节对齐 */

    alg_rts_rate_tbl_stru rts_11b_tbl[ALG_RTS_11B_RATE_NUM];      /* 数据帧11b rts速率表 */
    alg_rts_rate_tbl_stru rts_11g_tbl[ALG_RTS_11G_RATE_NUM];      /* 数据帧11g rts速率表 */
    alg_rts_rate_tbl_stru rts_11n_tbl[ALG_RTS_11N_RATE_NUM];      /* 数据帧11n rts速率表 */
    alg_rts_rate_tbl_stru rts_11ax_tbl[ALG_RTS_11AX_RATE_NUM];    /* 数据帧11ax rts速率表 */
} alg_param_sync_rts_stru;

/* rts 配置命令同步结构体 */
typedef struct {
    osal_u32                rts_enable           : 1;    /* rts算法使能开关 */
    osal_u32                rts_fixed_mode       : 1;    /* 配置命令决定是否固定rts模式 */
    osal_u32                enable_rts_log       : 1;    /* 使能日志的标志位 */
    osal_u32                rts_mode             : 3;    /* RTS模式 */

    osal_u32                res                  : 26;   /* 4字节对齐 */
} alg_config_param_sync_rts_stru;

typedef struct {
    hal_fcs_protect_cnt_enum_uint8 protect_cnt;     /* 保护帧发送次数 */
    osal_u8 vip_frame_protect_threshold;            /* vip帧的保护切信道次数 */
    osal_u16 sw_protect_timeout;                    /* 软件超时等待值 */
    osal_u16 hw_protect_timeout;                    /* FCS芯片超时值 */
    osal_u16 dbac_tbtt_offset;                      /* TBTT 提前上报量 */
    osal_u16 timer_adjust;                          /* Timer的提前量 */
    osal_u16 noa_start_adjust;                      /* NOA start time的提前量 */
    osal_u16 desired_tbtt_adjust;                   /* 设置理想tbtt的调整量 */
    osal_u8 customize_sta_gc_slot_ratio;            /* 定制化文件配置的gc共存vap占用空口的比例 */
    osal_u8 customize_sta_go_slot_ratio;            /* 定制化文件配置的go共存vap占用空口的比例 */
    osal_u16 sync_tsf_threshold;                    /* 同步tsf的阈值 */
    osal_u8 resv_netbuf_threshod;                   /* 预留netbuf门限 */
    osal_u8 protect_gc_slot_period;                 /* gc保护周期 */
} alg_param_sync_dbac_stru;

typedef struct {
    osal_u32 max_lock_times;
    osal_u32 lock_timestamp_th;
} alg_param_sync_common_stru;
typedef struct {
    osal_u32 aging_time_ms;                /* 聚合per统计老化时间 */
    osal_u8 dbac_aggr_time_idx;            /* DBAC约束的最大聚合时间对应的g_aaggr_time_list数组下标 */
    osal_u8 max_probe_wait_cnt;            /* 等待探测结果的最大发送完成中断次数(避免中断丢失导致状态异常) */
    osal_u8 aggr_non_probe_pck_num;        /* 最优速率发送变化后，不进行探测的包数目 */
    osal_u8 aggr_probe_intvl_num;          /* 聚合时间探测对应的间隔包数目 */

    osal_u8 intf_aggr_stat_shift;          /* 干扰场景下聚合时间探测的统计delta PER的包数目除2位移值 */
    osal_u8 no_intf_aggr_stat_shift;       /* 无干扰时聚合时间探测的统计delta PER的包数目 */
    osal_u8 init_avg_stat_num;             /* 聚合初始per统计的包数目 */
    osal_u8 aggr_tb_switch : 1;            /* TB场景是否采用最优聚合度 */
    osal_u8 resv :           7;            /* 对齐 */

    osal_u8 intf_up_aggr_stat_num;         /* 干扰场景向上聚合时间探测的统计delta PER的包数目 */
    osal_u8 intf_down_aggr_stat_num;       /* 干扰场景向下聚合时间探测的统计delta PER的包数目 */
    osal_u8 intf_up_update_stat_cnt_thd;   /* 干扰场景聚合时间向上探测的开始更新state num数目 */
    osal_u8 intf_down_update_stat_cnt_thd; /* 干扰场景聚合时间向下探测的开始更新state num数目 */
} alg_param_sync_aggr_stru;
typedef struct {
    osal_u8 tx_tb_in_th;
    osal_u8 tx_tb_out_th : 7;
    osal_u8 suspend_tx_flag : 1;
    osal_u8 tx_tb_in_ms_th;
    osal_u8 tx_tb_out_ms_th;
} alg_param_sync_tx_tb_stru;
/* autorate模块需要从HOST同步的参数 */
typedef struct {
    alg_autorate_dev_stru autorate_dev;
} alg_param_sync_autorate_stru;
typedef struct {
    osal_u8 tx_time_method;
    osal_u8 rev;
    osal_u16 report_period;
} alg_param_sync_sch_stru;
/* 注意4字节对齐 */
typedef struct {
    osal_u32 head_magic_num;
    alg_param_sync_tpc_stru tpc_para;
    alg_param_sync_traffic_ctl_stru traffic_ctl_para;
    alg_param_sync_rts_stru rts_para;
    alg_param_sync_dbac_stru dbac_para;
    alg_param_sync_common_stru common_para;
    alg_param_sync_aggr_stru aggr_para;
    alg_param_sync_autorate_stru autorate_para;
    alg_param_sync_tx_tb_stru tx_tb_para;
    alg_param_sync_sch_stru sch_para;
    osal_u32 gla_enable_bitmap[ALG_GLA_SWITCH_TYPE_BUTT];
    osal_u32 tail_magic_num;
} alg_param_sync_stru;

typedef struct {
    /* 统计计数 */
    osal_u32 tx_cnt;        /* 统计周期内总发包个数计数 */
    osal_u32 collision_cnt; /* 统计周期内碰撞个数计数 */
    osal_u32 pkt_short_goodput_sum;
} alg_edca_det_ac_info_stru;
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
typedef struct {
    osal_u16 short_per;
    osal_u8 user_lut_idx;          /* 用户id */
    osal_u8 res;                  /* 对齐 */
    osal_u32 goodput;      /* 对应带宽下该速率的理论goodput */
} alg_anti_intf_ar_info_stru;

typedef struct {
    osal_u8 user_num;               /* 有效用户数 */
    osal_u8 resv[3];                /* 对齐 */
    alg_anti_intf_ar_info_stru user_ar_info[ALG_MAX_USER_NUM_BUTT];
} alg_anti_intf_info_stru;

typedef struct {
    osal_u16 user_bitmap;           /* device侧收到包的用户 */
    osal_s8 rssi_min;           /* device侧最小rssi */
    osal_s8 res;                    /* 对齐 */
} alg_anti_intf_tx_comp_info_stru;
#endif
#ifdef _PRE_WLAN_FEATURE_SCHEDULE
/* device调度同步host侧调度 */
typedef struct {
    osal_u8 user_num;
    osal_u8 reserve[3];     /* 预留3字节对齐 */

    osal_u32 bcast_time;
    osal_u32 bcast_bytes;
} dmac_to_hmac_sch_user_info_header_stru;
typedef struct {
    osal_u8 user_id;
    osal_u8 resv;
    osal_u16 dequeue_mpdu_num;
    osal_u32 total_tx_time;
    osal_u32 tx_mpdu_bytes;

    osal_u32 resv1;
} dmac_to_hmac_user_rate_stru;
typedef struct {
    dmac_to_hmac_sch_user_info_header_stru header;
    dmac_to_hmac_user_rate_stru user_info[ALG_MAX_USER_NUM_BUTT];
} dmac_to_hmac_sch_user_info_stru;
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of alg_common_rom.h */
