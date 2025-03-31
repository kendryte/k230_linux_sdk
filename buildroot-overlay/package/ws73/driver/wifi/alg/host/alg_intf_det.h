/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: Header file for alg_intf_det.c
 */


#ifndef __ALG_INTF_DET_H__
#define __ALG_INTF_DET_H__

#ifdef _PRE_WLAN_FEATURE_INTF_DET

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "osal_types.h"
#include "oal_fsm.h"
#include "mac_vap_ext.h"
#include "msg_alg_rom.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/******************************************************************************
  2 宏定义
******************************************************************************/
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
#define ALG_INTF_DET_STATISTIC_CNT_TH 10 /* 连续统计门限 */
#define ALG_INTF_DET_GLA_SHIFT        4
#endif
/******************************************************************************
  3 枚举定义
******************************************************************************/
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
typedef enum {
    ALG_INTF_DET_EVENT_DUTY_CYCLE = 0,
    ALG_INTF_DET_EVENT_NEG_GAIN,
    ALG_INTF_DET_EVENT_HOST_PK,
    ALG_INTF_DET_EVENT_HOST_NPK,
    ALG_INTF_DET_EVENT_PKADJ_COMP,
    ALG_INTF_DET_EVENT_DETATCH,

    ALG_INTF_DET_EVENT_BUTT
} alg_intf_det_event_enum;
typedef osal_u16 alg_intf_det_event_enum_uint16;
#endif

/******************************************************************************
  4 STRUCT定义
******************************************************************************/
/* 记录最近10次干扰繁忙度 */
typedef struct {
    osal_u8 cur_index; /* 当前所在位置 */
    osal_u8 total_cnt; /* 当前干扰繁忙度记录的总个数 */
    osal_u8 resv[2];   /* 对齐 */
    osal_u16 intf_duty_cyc_20_recoard[ALG_INTF_DET_STATISTIC_CNT_TH];  /* 记录20M最近10次干扰繁忙度 */
    osal_u16 intf_duty_cyc_40_recoard[ALG_INTF_DET_STATISTIC_CNT_TH];  /* 记录40M最近10次干扰繁忙度 */
    osal_u32 sum_of_intf_duty_cyc_20;   /* 记录20M最近10次干扰繁忙度总数 */
    osal_u32 sum_of_intf_duty_cyc_40;   /* 记录40M最近10次干扰繁忙度总数 */
} alg_intf_det_intf_duty_cyc_info_stru;

/* 保存干扰检测同频干扰相关的信息:eg rx non-direct占空比等 */
typedef struct {
    osal_u16 nondir_duty_cyc_th_sta; /* 判别同频干扰nondir占空比门限(千分之x) */
    osal_u8 intf_cycle_cnt_sta;    /* 连续多少个周期检测到同频干扰 */
    osal_u8 nonintf_cycle_cnt_sta; /* 连续多少个周期没有检测到同频干扰 */

    osal_u16 nonintf_duty_th_sta;              /* 无干扰的占空比门限 */
    oal_bool_enum_uint8 coch_intf_state_sta; /* 同频干扰的检测状态 */
    osal_u8 resv;
} alg_intf_det_coch_info_stru;

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/* 保存干扰检测邻频干扰相关的信息:eg 干扰繁忙度等 */
typedef struct {
    osal_u16 chn_scan_cycle_ms;    /* 设置信道扫描的时间:ms */
    osal_u16 strong_intf_ratio_th; /* 判断是否存在强邻频、叠频干扰的繁忙度阈值 */

    osal_u16 nonintf_ratio_th; /* 判断N个周期为无干扰状态的繁忙度阈值 */
    osal_u16 intf_sync_err_th; /* 判断是否存在邻频、叠频干扰的sync error阈值 */

    osal_s8 intf_aveg_rssi_th;                  /* 判断是否存在中等强度邻频、叠频干扰的pri20/40/80的噪底阈值 */
    hal_alg_intf_det_mode_enum_uint8 intf_type; /* 指示当前的邻频干扰模式 */
    osal_u8 intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_BUTT]; /* 不同干扰检测  模式的连续判断阈值 */
} alg_intf_det_adjch_info_stru;

typedef struct _alg_intf_det_duty_cyc {
    osal_u16 duty_cyc_ratio_20;
    osal_u16 duty_cyc_ratio_40;
} alg_intf_det_duty_cyc_stru;

typedef struct _alg_intf_det_handler {
    oal_fsm_stru oal_fsm; /* 干扰检测状态机 */
    alg_intf_det_duty_cyc_stru duty_cyc_info[ALG_INTF_DET_STATE_BUTT];
    oal_bool_enum_uint8 is_fsm_attached;
    osal_u8 curr_mode_cnt;
    osal_u8 resv[2]; /* 2:数组大小 */
} alg_intf_det_handler_stru;

#else
/* 保存干扰检测邻频干扰相关的信息:eg 干扰繁忙度等 */
typedef struct {
    osal_u16 chn_scan_cycle_ms;    /* 设置信道扫描的时间:ms */
    osal_u16 strong_intf_ratio_th; /* 判断是否存在强邻频、叠频干扰的繁忙度阈值 */

    osal_u16 nonintf_ratio_th; /* 判断N个周期为无干扰状态的繁忙度阈值 */
    osal_u16 intf_sync_err_th; /* 判断是否存在邻频、叠频干扰的sync error阈值 */

    osal_s8 resv;
    osal_s8 intf_aveg_rssi_th;                  /* 判断是否存在中等强度邻频、叠频干扰的pri20/40/80的噪底阈值 */
    hal_alg_intf_det_mode_enum_uint8 intf_type; /* 指示当前的邻频干扰模式 */
    osal_u8 non_intf_cyc_th;                      /* 连续无干扰检测的阈值 */
} alg_intf_det_adjch_info_stru;
#endif

/* 同频干扰统计值 */
typedef struct {
    osal_u32 rx_nondir_time;          /* 周期内接收non-direct包的时间 */
    osal_u32 rx_direct_time;          /* 周期内接收direct包的时间 */
    osal_u32 tx_time;                 /* 周期内处于发送状态的时间 */
    osal_u32 total_stats_time_ms;     /* 统计周期 */
    osal_u16 rx_nondir_duty_cyc;      /* rx non-direct包占空比 */
    osal_u16 rx_direct_duty_cyc;      /* rx direct时间占空比 */
    osal_u16 tx_time_duty_cyc;        /* 发送时间占空比 */
    osal_u16 rx_nondir_duty_cyc_last; /* 上个周期的接收non-direct包占空比 */
    osal_u16 tx_time_duty_cyc_last;   /* 上个周期的发送时间占空比 */
    osal_u16 rx_direct_duty_cyc_last; /* 上个周期的接收dir时间占空比 */

    /* 弱干扰免疫负增益测试 */
    osal_u32 tx_time_last; /* 上个周期的tx time for test */
    osal_u32 tx_time_avg;  /* 多个周期的平均tx time值 for test */
} alg_intf_det_coch_stat_stru;

/* 邻频叠频干扰统计 */
typedef struct {
    osal_u16 duty_cyc_ratio_20; /* pri20干扰繁忙度(千分之x) */
    osal_u16 duty_cyc_ratio_40; /* pri40干扰繁忙度(千分之x) */
    osal_u16 duty_cyc_ratio_sec20; /* sec20干扰繁忙度(千分之x) */
    osal_u16 sync_err_ratio;    /* 同步错误比值(千分之x) */

    osal_u8 intf_det_ilde_cnt;      /* 空闲功率非0值的次数 */
    osal_s8 intf_det_avg_rssi_20; /* pri20 RSSI平均值 */
    osal_s8 intf_det_avg_rssi_40; /* pri40 RSSI平均值 */
    osal_u8 intf_adj_cnt;           /* 降带宽符合条件的周期数 */

    osal_u8 cert_intf_cyc_cnt;                            /* 连续多少个周期没有检测到认证干扰计数 */
    osal_u8 non_intf_cyc_cnt;                             /* 无干扰的连续计数 */
    osal_u8 strong_intf_cyc_cnt;                          /* 强干扰的连续计数 */
    osal_u8 middle_intf_cyc_cnt;                          /* 中干扰的连续计数 */

    /* 共存用 */
    osal_u32 abort_time_us;
} alg_intf_det_adjch_stat_stru;

/* device下抗干扰特性对应的信息结构体 */
typedef struct {
    osal_u8 intf_det_mode;              /* 干扰检测的工作模式 */
    osal_u8 intf_det_debug;             /* 干扰检测日志开关，跟工作模式相对应 */
    oal_bool_enum_uint8 intf_det_log; /* log使能 */
    osal_u8 adjust_cycle_cnt;           /* 判定周期调整时无干扰周期计数 */
    osal_u16 intf_det_cycle_ms;         /* 干扰检测周期 */
    oal_bool_enum_uint8 statistic_update; /* device侧是否有上报数据更新 */
    osal_s8  intf_aveg_rssi_th;    /* 判断是否存在中等强度邻频、叠频干扰的pri20/40/80的噪底阈值 */
    osal_u16 strong_intf_ratio_th; /* 判断是否存在强邻频、叠频干扰的繁忙度阈值 */
    osal_u16 nonintf_ratio_th;     /* 判断N个周期为无干扰状态的繁忙度阈值 */

    /* 主定时器 */
    frw_timeout_stru intf_det_timer;     /* 干扰检测定时器 */
    hal_to_dmac_device_stru *hal_device; /* hal device 指针 */

    /* 同频干扰检测 */
    alg_intf_det_coch_info_stru coch_intf_info; /* 同频干扰相关信息 */
    alg_intf_det_coch_stat_stru coch_intf_stat; /* 同频干扰检测统计信息 */

    /* 邻/叠频干扰检测 */
    alg_intf_det_adjch_info_stru adjch_intf_info; /* 邻频干扰相关信息 */
    alg_intf_det_adjch_stat_stru adjch_intf_stat; /* 邻频叠频干扰检测统计信息 */
    alg_intf_det_notify_info_stru intf_det_notify; /* 干扰检测通知device */

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    /* 干扰模式切换增益检测 */
    oal_bool_enum_uint8 pk_mode;
    hal_alg_pk_intf_stage_enum_uint8 pk_mode_stage;
    oal_bool_enum_uint8 need_recover;
    oal_bool_enum_uint8 has_set;
#endif

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    alg_intf_det_handler_stru intf_det_handler;
#endif
    alg_intf_det_intf_duty_cyc_info_stru last_10_times_intf_duty_cyc[ALG_INTF_DET_STATE_BUTT];  /* 最近10次干扰繁忙度 */
} alg_intf_det_stru;

/******************************************************************************
  5 函数声明
******************************************************************************/
osal_s32 alg_intf_det_init(osal_void);
osal_void alg_intf_det_exit(osal_void);
oal_bool_enum_uint8 alg_intf_det_get_curr_coch_intf_type(hal_to_dmac_device_stru *hal_device);
hal_alg_intf_det_mode_enum_uint8 alg_intf_det_get_curr_adjch_intf_type(hal_to_dmac_device_stru *hal_device);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_INTF_DET */
#endif /* end of alg_intf_det.h */
