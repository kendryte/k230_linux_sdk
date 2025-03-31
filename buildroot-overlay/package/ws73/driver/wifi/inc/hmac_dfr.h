/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hmac_dfr.c 的结构体在HMAC的头文件
 * Date: 2021-04-13
 */

#ifndef __HMAC_DFR_H__
#define __HMAC_DFR_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFR_H

#define MAX_TID_ERROR_NUM  2

#ifdef _PRE_WLAN_FEATURE_DFR
extern hmac_dfr_info_stru g_dfr_recovery_info;
extern hmac_ap_dfr_recovery_info g_dfr_ap_info;
#endif

typedef enum {
    HMAC_DFR_EVENT_NULL = 0,
    HMAC_DFR_TID_QUEUE_HALT_SET = 0x0001 << 0,
    HMAC_DFR_TID_QUEUE_BUSY_SET = 0x0001 << 1,
    HMAC_DFR_TX_DSCR_QUEUE_HALT_SET = 0x001 << 2,
    HMAC_DFR_TX_DSCR_QUEUE_FAIL_SET = 0x001 << 3,
    HMAC_DFR_RX_DSCR_QUEUE_HALT_SET = 0x0001 << 4,
    HMAC_DFR_RX_DSCR_QUEUE_FAIL_SET = 0x0001 << 5,
    HMAC_DFR_FRW_QUEUE_HALT_SET = 0x0001 << 6,
    HMAC_DFR_FRW_QUEUE_FAIL_SET = 0x0001 << 7,
    HMAC_DFR_MEM_POOL_FAIL_SET = 0x0001 << 8,
    HMAC_DFR_MEM_POOL_NO_FREE_SET = 0x0001 << 9
} hmac_dfr_event_set_enum;

typedef struct {
    osal_u16 event_type;                /* 上报异常类型 */
    osal_u8 tx_dscr_content;            /* 上报发送描述符队列异常内容 */
    osal_u8 rx_dscr_content;            /* 上报接收描述符队列异常内容 */
    osal_u8 mempool_content;            /* 上报内存池异常内容 */
    osal_u8 hmac_dfr_event_reserve;     /* 4字节对齐保留 */
    /* 上报事件队列队列异常内容,WLAN_FRW_MAX_NUM_CORES定义为1 */
    osal_u16 frw_content[WLAN_FRW_MAX_NUM_CORES];
    osal_u8 tid_content[MAC_RES_MAX_USER_LIMIT][MAX_TID_ERROR_NUM];
    osal_u32 hmac_user_id_info;         /* TID的USER_ID的信息 */
} hmac_dfr_event;

typedef enum {
    HMAC_DFR_TID_REPORT = 0,
    HMAC_DFR_TX_REPORT = 1,
    HMAC_DFR_RX_REPORT = 2,
    HMAC_DFR_FRW_REPORT = 3,
    HMAC_DFR_MEM_POOL_REPORT = 4,
    HMAC_DFR_MAX_NUM = 5,
} hmac_dfr_report_enum;

typedef struct {
    osal_u32 enqueue_total_cnt;
    osal_u32 dequeue_total_cnt;
    osal_u32 queue_fail_cnt;
} hmac_dfr_queue_statistics;

typedef struct {
    osal_u32 alloc_total_cnt;
    osal_u32 free_total_cnt;
    osal_u32 fail_cnt;
} hmac_dfr_mempool_statistics;

/* 查询tid时，返回的结果的信息 */
typedef struct {
    osal_u32 is_paused; /* TID被暂停调度 */
    osal_u32 in_num;   /* 进入队列的包的个数 */
    osal_u32 out_num;  /* 出队列的包的个数 */
    osal_u32 dispatch_num;  /* 发生tid队列调度的次数 */
} hmac_dfr_tid_report_info;

/* ccpriv命令设置下来的参数的信息 */
typedef struct {
    osal_u32 switch_enable; /* 使能的开关 */
    osal_u32 timeout;       /* 周期性任务的时间 */
    osal_u32 cnt_period;    /* 上报或者显示打印的周期间隔 */
} period_check_cfg;

/* ccpriv命令获取信息时的参数信息 */
typedef struct {
    osal_u32 switch_type;   /* 获取哪种类型的信息（TID TX RX FRW等） */
    osal_u32 param1;
    osal_u32 param2;
} period_get_param;

/* 查询时的，tid队列的信息 */
typedef struct {
    osal_u16  tid_user_id;
    osal_u16  tid_index;
} hmac_dfr_tid_event_info;

typedef struct {
    osal_u16 core_id;
    osal_u16 event_index;
} hmac_dfr_frw_info;

/* 查询用到的结构体 */
typedef struct {
    osal_u32 event_type;  /* 决定查询TID还是FRW等 */
    union {
        osal_u32 tx_dscr_index;
        osal_u32 rx_dscr_index;
        osal_u32 mem_pool_index;
        hmac_dfr_frw_info frw_index;
        hmac_dfr_tid_event_info tid_content;
    } hmac_dfr_check_info;
} hmac_dfr_info;

/* 返回到HMAC的查询结果 */
typedef struct {
    osal_u32 event_type;
    union {
        hmac_dfr_queue_statistics tx_dscr_statistics_value;
        osal_u32 rx_dscr_statistics_value;
        hmac_dfr_queue_statistics frw_statistics_value;
        hmac_dfr_tid_report_info tid_content;
        hmac_dfr_mempool_statistics mem_pool_statistics_value;
    } hmac_dfr_check_report_value;
} hmac_dfr_check_report;

typedef struct {
    osal_u8 tid_halt_cur[MAC_RES_MAX_USER_LIMIT][WLAN_TID_MAX_NUM];
    osal_u8 tid_busy_cur[MAC_RES_MAX_USER_LIMIT][WLAN_TID_MAX_NUM];
    osal_u8 tx_halt_cur[HAL_TX_QUEUE_NUM];
    osal_u8 rx_halt_cur[HAL_RX_QUEUE_NUM];
    osal_u8 frw_halt_cur[WLAN_FRW_MAX_NUM_CORES][FRW_POST_PRI_NUM];
    osal_u8 frw_fail_cur[WLAN_FRW_MAX_NUM_CORES][FRW_POST_PRI_NUM];
    osal_u8 mem_fail_cur[OAL_MEM_POOL_ID_BUTT];
    osal_u8 mem_no_free_cur[OAL_MEM_POOL_ID_BUTT];
    osal_u16 hmac_dfr_reserve;     /* 4字节对齐保留 */
} hmac_dfr_error_time;

typedef struct {
    osal_u32 module_set;
    osal_void (*check_exception_func)(hmac_vap_stru *hmac_vap, hmac_dfr_event *dfr_event);
} hmac_dfr_check_module_exception_stru;

typedef struct {
    osal_u32 module_set;
    osal_u32 (*query_module_func)(const hmac_dfr_info *index_info, hmac_dfr_check_report *check_info);
} hmac_dfr_query_module_statistics_stru;

#ifdef _PRE_WLAN_FEATURE_DFR
osal_void hmac_dfr_fill_ap_recovery_info(osal_u16 cfg_id, void *data, hmac_vap_stru *hmac_vap);
osal_u8 hmac_dfr_get_recovery_flag(osal_void);
#endif
osal_void hmac_dfr_record_ap_follow_channel(osal_u8 channel, wlan_channel_bandwidth_enum_uint8 bandwidth);
osal_u8 hmac_dfr_get_reset_process_flag(osal_void);

osal_s32 hmac_get_check_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32 hmac_config_set_period_task(hmac_vap_stru *hmac_vap, frw_msg *msg);
#ifdef _PRE_WLAN_DFR_STAT
osal_s32 hmac_get_dfr_info(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_dft.h */
