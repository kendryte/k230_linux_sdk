/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc dfx completion.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_DFX_HEADER
#define HCC_DFX_HEADER


#include "td_base.h"
#include "hcc_comm.h"
#include "hcc_cfg_comm.h"
#include "hcc_types.h"
#include "hcc_cfg.h"
#include "soc_osal.h"

typedef struct _hcc_service_stat_ {
    /* 业务申请内存成功、失败的总次数, 业务内存只有RX方向需要hcc申请 */
    td_u32 alloc_succ_cnt; /* 回调业务alloc返回成功次数 */
    td_u32 alloc_cb_cnt;   /* 回调业务alloc次数 */
    td_u32 alloc_fail_cnt; /* 申请内存失败的次数: 其中包含参数错误引起未调用回调场景 */

    /* 回调业务free的次数 */
    td_u32 free_cnt;

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    /* 回调业务start_subq的次数 */
    td_u32 start_subq_cnt;
    /* 回调业务stop_subq的次数 */
    td_u32 stop_subq_cnt;
#endif
    /* 回调业务 rx_proc 统计 */
    td_u32 rx_cnt; /* 调用次数 */
    td_u32 exp_rx_cnt; /* 实际应调用次数 */

    td_u32 bus_tx_succ; /* 通道发送成功包数量 */

    td_u32 rx_err_cnt;
} hcc_service_stat;

typedef struct _hcc_queue_stat_ {
    /* 队列申请内存成功、失败次数 */
    /* 释放次数无法记录，数据可能会不准确，暂不记录 */
    td_u32 alloc_succ_cnt;
    td_u32 alloc_fail_cnt;

    td_u32 total_pkts;  // 当前队列处理帧的数量，成功传递给对端 或 成功传递给业务的包数量
    td_u32 loss_pkts; // 当前队列丢帧数量
} hcc_queue_stat;

typedef struct _hcc_bus_stat_ {
    td_u32 total_tx_pkts;  // 发送成功的数量
    td_u32 loss_tx_pkts; // 发送丢帧数量
} hcc_bus_stat;

typedef struct _hcc_inner_stat_ {
    td_u32 unc_alloc_succ[HCC_DIR_COUNT];
    td_u32 unc_alloc_fail[HCC_DIR_COUNT];
    td_u32 unc_free_cnt;
    td_u32 mem_free_cnt;
} hcc_inner_stat;

typedef struct _hcc_dfx_ {
    osal_spinlock hcc_dfx_lock;
    hcc_service_stat service_stat[HCC_SERVICE_TYPE_MAX];
    hcc_queue_stat   queue_stat[HCC_DIR_COUNT][HCC_QUEUE_COUNT];
    hcc_inner_stat inner_stat;
    td_u8 que_max_cnt;
    td_u8 srv_max_cnt;
    td_u16 rsv;
} hcc_dfx;

td_s32 hcc_dfx_init(td_void);
td_void hcc_dfx_deinit(td_void);
td_void hcc_dfx_queue_total_pkt_increase(hcc_service_type serv_type, hcc_queue_dir dir, hcc_queue_type q_id, td_u8 cnt);
td_void hcc_dfx_queue_loss_pkt_increase(hcc_queue_dir dir, hcc_queue_type q_id);

/*
 * 一个service对应多个队列，记录service的同时记录对应的queue 申请内存成功失败的次数
 */
/* 回调业务alloc次数 */
td_void hcc_dfx_service_alloc_cb_cnt_increase(hcc_service_type service_type);
/* 业务内存alloc返回成功或失败次数，异常时可能会有参数不正确问题 */
td_void hcc_dfx_service_alloc_cnt_increase(hcc_service_type service_type, hcc_queue_type queue_id, td_bool success);
/* 回调业务free次数 */
td_void hcc_dfx_service_free_cnt_increase(hcc_service_type service_type);

/* hcc链表、业务内存释放次数 */
td_void hcc_dfx_mem_free_cnt_increase(td_void);
td_void hcc_dfx_unc_free_cnt_increase(td_void);
/* hcc链表TX/RX方向申请成功或失败次数 */
td_void hcc_dfx_unc_alloc_cnt_increase(hcc_queue_dir direction, td_bool success);

td_void hcc_dfx_service_startsubq_cnt_increase(hcc_service_type service_type);
td_void hcc_dfx_service_stopsubq_cnt_increase(hcc_service_type service_type);
td_void hcc_dfx_service_rx_cnt_increase(hcc_service_type service_type, hcc_queue_type queue_id);
td_void hcc_dfx_service_exp_rx_cnt_increase(hcc_service_type service_type);

td_void hcc_dfx_service_rx_err_cnt_increase(hcc_service_type service_type);
td_void hcc_dfx_queue_info_print(hcc_channel_name chl, hcc_queue_dir dir, hcc_queue_type q_id);
td_void hcc_dfx_service_info_print(hcc_service_type service_type);
td_void hcc_bus_dfx_statics_print(hcc_channel_name chl);
#endif /* HCC_DFX_HEADER */