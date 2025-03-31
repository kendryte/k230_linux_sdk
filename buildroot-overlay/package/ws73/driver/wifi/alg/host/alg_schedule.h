 /*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: schedule algorithm
 */

#ifndef __ALG_SCHEDULE_H__
#define __ALG_SCHEDULE_H__

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_autorate.h"

#ifdef _PRE_WLAN_FEATURE_SCHEDULE

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/******************************************************************************
  2 宏定义
******************************************************************************/
/* 调度日志相关参数 */
#define ALG_SCH_LOG 1 /* 调度日志开关，默认只打开发送完成中断统计信息 */

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
#define ALG_SCH_LOG_EXT1 1              /* 调度日志第1级扩展，增量记录每次调度时各个tid的情况 */
#define ALG_SCH_LOG_EXT2 0              /* 调度日志第2级扩展，增量记录流量控制相关信息 */
#define ALG_SCHEDULE_LOG_MAX_NUM 40     /* 最大可记录日志条目 */
#define ALG_SCHEDULE_LOG_TID_MAX_NUM 16 /* 所有用户非空TID个数 */
#endif

/* 业务调度相关参数 */
#define ALG_SCHEDULE_SCH_CYCLE_MS 100   /* 业务调度定时器周期(ms) */
#define ALG_SCHEUDLE_TIMER_MODIFY_MS 20 /* 定时器执行误差 */

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
/* WMM空口公平性参数 */
#define ALG_SCH_WMM_AIR_USER_PERIOD_RATIO 50 /* 用户占空口总时间比例 */
#define ALG_SCH_WMM_AIR_USER_PERIOD 1000 /* 总用户占空口时间 */
#define ALG_SCH_WMM_DMAC_TIMEOUT_MS 1000 /* dmac每次同步时间 */
#define ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MAX 16 /* 目标周期内的MPDU个数 */
#define ALG_SCH_WMM_AIR_USER_TARGET_TUOTA_MIN 2 /* 目标周期内的MPDU个数 */
#define ALG_SCH_WMM_AIR_USER_PERIOD_TORLERANCE 20 /* 周期时间的容忍度 */
#define ALG_SCH_WMM_AIR_USER_VALID_TXBYTES_MIN 128 /* 用户级最低有效空口发送字节数 */
#define ALG_SCH_WMM_AIR_TOTAL_VALID_TXBYTES_MIN 128 /* 总最低有效空口发送字节数 */
#define ALG_SCH_WMM_AIR_TOTAL_VALID_TXTIME_MIN 2048 /* 总最低有效空口发送时间 单位10us */
#define ALG_SCH_WMM_AIR_MAX_BUF_BYTES 1500 /* 报文大包字节数 */
#define ALG_SCH_WMM_AIR_MOVE_STEP_MPDU 1 /* 每次移动MPDU数 */
#define ALG_SCH_WMM_DMAC_DSCR_CAL_METHOD 1 /* 不用芯片上报的tx time, 使用描述符计算时间 */
/* 统计 */
#define ALG_SCHEUDLE_STAT_TIMER_MS 1000 /* 定时器执行误差 */
#define ALG_SCHEDULE_SCH_NUMS_PER_AC 10 /* 每个周期内各个AC的最小调度次数 */

/* WME 参数调整相关参数 */
#define ALG_SCHEDULE_WME_ADJUST_CYCLE_MS 500 /* WME参数调整周期(ms) */
#define ALG_SCHEDULE_WME_INC_VI_DELAY_MS 45  /* WME往上调整的视频时延阈值(ms) */
#define ALG_SCHEDULE_WME_DEC_VI_DELAY_MS 40  /* WME往下调整的视频时延阈值(ms) */

/* 定义最大链路速率 */
#define ALG_SCHEDULE_MAX_LINK_RATE_KBPS 2000000 /* 2Gbps */

#define oal_dlist_search_for_each_sch(_dlist_pos, i, user_num, _dilst_head) \
    for ((_dlist_pos) = (_dilst_head), (i) = 0; \
         (i) < (user_num) + 1 && (_dlist_pos) != NULL; \
         (_dlist_pos) = (_dlist_pos)->next, (i)++)
#endif

/******************************************************************************
  3 枚举定义
******************************************************************************/
typedef enum {
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    ALG_SCH_STRATEGY_WMM_ENSURE, /* 优先级调度 */
#endif
    ALG_SCH_STRATEGY_ROUND_ROBIN, /* 轮询调度 */
    ALG_SCH_STRATEGY_CERTIFY, /* 轮询调度 */
    ALG_SCH_STRATEGY_BUTT
} alg_sch_strategy_enum;

typedef enum {
    ALG_BUF_DROP_NULL,
    ALG_BUF_DROP_VAP_FLOW_CONTROL,
    ALG_BUF_DROP_USER_FLOW_CONTROL,
    ALG_BUF_DROP_BUFFER_OVERFLOW,

    ALG_BUF_DROP_BUTT
} alg_schedule_buf_drop_reason_enum;
typedef osal_u8 alg_schedule_buf_drop_reason_enum_uint8;

typedef enum {
    ALG_BUF_PROCESS_PASS = 0,

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    ALG_BUF_PROCESS_COMING,
    ALG_BUF_PROCESS_DROP,
    ALG_BUF_PROCESS_SEIZED,
#endif

    ALG_BUF_PROCESS_BUTT
} alg_schedule_buf_process_enum;
typedef osal_u8 alg_schedule_buf_process_enum_uint8;

/******************************************************************************
  4 STRUCT定义
******************************************************************************/
#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
/* 每个user对应的调度私有信息结构体 */
typedef struct {
    struct osal_list_head user_dlist_entry; /* 用户链表头 */

    /* WMM空口公平性参数 */
    osal_u32 target_quota;   /* 设定的该用户发包阈值 */
    osal_u32 remain_quota;   /* 该用户剩余的发包阈值 */
    osal_u32 target_time;
    osal_u8  user_id;
    osal_u8  resv[3];

    osal_u32 stat_out_mpdu_num[WLAN_TIDNO_BUTT]; /* 每个TID在周期内发包个数 */
    osal_u32 stat_sch_num[WLAN_TIDNO_BUTT];  /* 每个TID在周期内调度次数 */
    osal_u32 stat_in_mpdu_num[WLAN_TIDNO_BUTT]; /* 每个TID在周期内发包个数 */
} alg_schedule_user_info_stru;

#endif

typedef struct {
    struct osal_list_head ac_dlist_entry;   /* ac双向链表元素 */
    struct osal_list_head txtid_dlist_head; /* tid双向链表头结点指针 */
} alg_sch_ac_txtid_dlist_stru;

/* 每个device 对应的调度信息结构体 */
typedef struct {
    struct osal_list_head ac_dlist_head[HAL_TX_QUEUE_NUM]; /* 各ac对应的txtid链表头结点 */
    /* 保持各tid的链表头结点,其中WLAN_TID_MAX_NUM = 8专用于节能用户 */
    alg_sch_ac_txtid_dlist_stru ac_txtid_dlist[WLAN_TID_MAX_NUM];

    /* 业务调度相关参数 */
    frw_timeout_stru sch_timer;         /* 调度周期定时器对象指针 */
    oal_bool_enum_uint8 fix_sch_mode; /* 是否采用固定配置的调度方式 */
    osal_u8 user_num;                 /* 当前调度维护的用户个数 */
    osal_u8 resv1[2];                   /* 字节对齐 */

    hal_to_dmac_device_stru *hal_device; // 保存使用的hal device

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    /* wmm 空口公平性调度 */
    osal_u8  temp_cnt;          /* 温保 */
    osal_u8  target_user_id;   /* 调度目标用户 */
    osal_u16 quota_step;       /* 每次移动步长 */
    osal_u32 tolerance;   /* 每次更新容忍度 */
    osal_u32 target_mpdu_total;   /* 所有用户总共的目标调度mpdu个数 */
    osal_u32 sch_nums_per_ac[WLAN_WME_AC_BUTT];    /* 记录每个调度周期内AC的调度次数 */
    /* 统计 */
    frw_timeout_stru sch_stat_timer;

#endif
} alg_schedule_stru;

/* 每个txtid对应的调度私有信息结构体 */
typedef struct {
    osal_u8 is_scheduling : 1,      /* 本tid是否处于被调度状态 */
        bad_link_flag : 1,        /* 该链路是否被认定为badlink */
        vap_id : 6;               /* vap id */
    osal_u8 tidno;                  /* tidno */
    osal_u16 assoc_id;              /* user对应资源池索引值 */

    alg_schedule_stru *schedule;           /* 本tid对应的私有device的指针 */
    struct osal_list_head txtid_dlist_entry; /* 双向链表元素 */

#ifdef _PRE_WLAN_FEATURE_SCH_STRATEGY_WMM_ENSURE
    osal_u8 is_stopped;                           /* 本tid是否被拥塞控制的标志 */
    osal_u8 resv2[3];                           /* 字节对齐 */
    alg_schedule_user_info_stru *sch_user_info; /* 本tid对应的私有USER的信息 */
#endif
} alg_schedule_txtid_info_stru;

/* 每次调度输出信息结构体 */
typedef struct {
    osal_u8 tid_num;       /* 被调度的tid号 */
    osal_u8 mpdu_num;      /* 需要发送的mpdu个数 */
    osal_u8 resv[2];       /* 2:数组大小 字节对齐 */
    hmac_user_stru *user; /* 需要发送的tid队列隶属的user */
} mac_txtid_schedule_output_stru;

typedef struct {
    hmac_user_stru *user;             /* 选出的user */
    hmac_tid_stru *txtid;            /* 选出的tid */
    oal_bool_enum_uint8 sch_flag;    /* 是否立即返回调度结果 */
    osal_u8 resv[3];                   /* 2:数组大小 */
} alg_sch_output_stru;
/******************************************************************************
  5 外部函数声明
******************************************************************************/

/******************************************************************************
  6 函数声明
******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_SCHEDULE */
#endif /* end of alg_schedule.h */
