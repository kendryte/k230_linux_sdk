/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description:  zdiag for sample data.
 */

#ifndef __ZDIAG_SAMPLE_DATA_H__
#define __ZDIAG_SAMPLE_DATA_H__

#ifdef CONFIG_SUPPORT_SAMPLE_DATA
#include "td_base.h"
#include "soc_zdiag.h"

#define ZDIAG_SAMPLE_START 1
#define ZDIAG_SAMPLE_STOP 0

typedef enum {
    ZDIAG_BSLE_SAMPLE_CB = 0,
    ZDIAG_WLAN_SAMPLE_CB,
    ZDIAG_SOC_SAMPLE_CB,
    ZDIAG_WLAN_PHY_SAMPLE_CB,
    ZDIAG_SAMPLE_CB_END,
}diag_sample_cb_enum;

/*********************bsle/soc数采******************/

/* hso配置参数命令下发结构体 */
typedef struct {
    td_u32 flag;        /* 1：启动数采；0:停止数采; */
    td_u32 transmit_id; /* 采集信号源 */
    td_u32 sample_size; /* 存储数采ram大小：0-7 */
    td_u32 sample_type; /* 数采类型：存满停止、循环数采 */
} diag_btsoc_sample_cmd;

/* 配置命令下发回复ack */
typedef struct {
    td_u32 ret;         /* 数采启动结果 */
    td_u32 flag;        /* 1：启动数采；0:停止数采; */
    td_u32 transmit_id; /* 采集信号源 */
} diag_sample_ack;

/* 数采数据上传结构体 */
typedef struct {
    td_u32 transmit_id; /* 采集信号源 */
    td_u32 ret;         /* 执行结果 */
    td_u32 offset;      /* 等于前面所有包总长度 */
    td_u32 size;        /* data的长度 */
    td_u32 crc;         /* 暂不启用 */
    td_u8 data[0];      /* 数采数据 */
} diag_sample_reply_pkt;

/* 数据上报结束 */
typedef struct {
    td_u32 transmit_id; /* 采集信号源 */
    td_u32 state_code;  /* 上报结束：11 */
    td_u32 len;         /* 暂不启用 */
    td_u32 data[0];     /* 暂不启用 */
} diag_sample_notify;

/***********************wifi数采***********************/
/* hso配置参数下发结构体 */
typedef struct {
    td_u32 flag;        /* 1：启动数采; 0：停止数采; */
    td_u32 mode;        /* 维测模式：1,4,8；同时作为transmitid */
    td_u32 sample_size; /* 存储数采ram大小：0-7 */
    td_u32 sample_type; /* 数采类型：存满停止、循环数采 */
    td_u32 sub_mode;    /* 维测子模式：0-31 */
} diag_wlan_sample_cmd;

/***********************wifi PHY数采***********************/
/* hso配置参数下发结构体 */
typedef struct {
    td_u32 flag;            /* 1：启动数采; 0：停止数采; */
    td_u32 node;            /* phy 数采节点 mode */
    td_u32 sample_size;     /* 存储数采ram大小 2k 4k 6k 8k 10k 12k 14k 16k 32k */
    td_u32 sample_type;     /* 数采类型：存满停止 0、循环数采 1 */
    td_u32 trigger_start;   /* 触发起始条件 */
    td_u32 trigger_end;     /* 触发结束条件 */
    td_u32 event_rpt_addr;  /* 事件上报寄存器 */
} diag_wlan_phy_sample_cmd;

typedef struct {
    td_u32 transmit_id;  /* 1：启动数采；0:停止数采; */
    td_u32 cmd_id;       /* 数采类型：wifi/soc/bsle */
    td_u32 offset;       /* 记录当前包 上报数据偏移 */
    td_u32 running;
    td_u32 msg_cnt;      /* 统计当前数采上报包个数 */
} diag_sample_record;

typedef td_u32 (*diag_sample_func_cb)(td_void *param, td_u32 len);
td_void diag_sample_data_register(diag_sample_cb_enum idx, diag_sample_func_cb func);
ext_errno diag_cmd_report_sample_data(td_u8 *buf, td_u32 len);
ext_errno diag_report_wlan_sample_data(TD_CONST td_u8 *buf, td_u32 len, td_u32 msg_id);
ext_errno diag_cmd_start_sample_data(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
#endif
#endif
