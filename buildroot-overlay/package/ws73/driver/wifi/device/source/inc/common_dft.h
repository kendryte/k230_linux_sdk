/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: common_dft 的头文件
 * Date: 2022-03-08
 */

#ifndef __COMMON_DFT_H__
#define __COMMON_DFT_H__

#include "common_dft_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 该枚举类型不建议频繁添加,只适用于异常原因明确的分支,周期上报异常触发日志 */
typedef enum {
    OAM_HAL_MAC_ERROR_PARA_CFG_ERR =
        0, /* 描述符参数配置异常,包括AMPDU长度配置不匹配,AMPDU中MPDU长度超长,sub msdu num错误 */
    OAM_HAL_MAC_ERROR_RXBUFF_LEN_TOO_SMALL = 1,    /* 接收非AMSDU帧长大于RxBuff大小异常 */
    OAM_HAL_MAC_ERROR_BA_ENTRY_NOT_FOUND = 2,      /* 未找到BA会话表项异常0 */
    OAM_HAL_MAC_ERROR_PHY_TRLR_TIME_OUT = 3,       /* PHY_RX_TRAILER超时 */
    OAM_HAL_MAC_ERROR_PHY_RX_FIFO_OVERRUN = 4,     /* PHY_RX_FIFO满写异常 */
    OAM_HAL_MAC_ERROR_TX_DATAFLOW_BREAK = 5,       /* 发送帧数据断流 */
    OAM_HAL_MAC_ERROR_RX_FSM_ST_TIMEOUT = 6,       /* RX_FSM状态机超时 */
    OAM_HAL_MAC_ERROR_TX_FSM_ST_TIMEOUT = 7,       /* TX_FSM状态机超时 */
    OAM_HAL_MAC_ERROR_RX_HANDLER_ST_TIMEOUT = 8,   /* RX_HANDLER状态机超时 */
    OAM_HAL_MAC_ERROR_TX_HANDLER_ST_TIMEOUT = 9,   /* TX_HANDLER状态机超时 */
    OAM_HAL_MAC_ERROR_TX_INTR_FIFO_OVERRUN = 10,   /* TX 中断FIFO满写 */
    OAM_HAL_MAC_ERROR_RX_INTR_FIFO_OVERRUN = 11,   /* RX中断 FIFO满写 */
    OAM_HAL_MAC_ERROR_HIRX_INTR_FIFO_OVERRUN = 12, /* HIRX中断FIFO满写 */
    OAM_HAL_MAC_ERROR_RX_Q_EMPTY = 13,             /* 接收到普通优先级帧但此时RX BUFFER指针为空 */
    OAM_HAL_MAC_ERROR_HIRX_Q_EMPTY = 14,           /* 接收到高优先级帧但此时HI RX BUFFER指针为空 */
    OAM_HAL_MAC_ERROR_BUS_RLEN_ERR = 15,           /* 总线读请求长度为0异常 */
    OAM_HAL_MAC_ERROR_BUS_RADDR_ERR = 16,          /* 总线读请求地址无效异常 */
    OAM_HAL_MAC_ERROR_BUS_WLEN_ERR = 17,           /* 总线写请求长度为0异常 */
    OAM_HAL_MAC_ERROR_BUS_WADDR_ERR = 18,          /* 总线写请求地址无效异常 */
    OAM_HAL_MAC_ERROR_TX_ACBK_Q_OVERRUN = 19,      /* tx acbk队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_ACBE_Q_OVERRUN = 20,      /* tx acbe队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_ACVI_Q_OVERRUN = 21,      /* tx acvi队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_ACVO_Q_OVERRUN = 22,      /* tx acv0队列fifo满写 */
    OAM_HAL_MAC_ERROR_TX_HIPRI_Q_OVERRUN = 23,     /* tx hipri队列fifo满写 */
    OAM_HAL_MAC_ERROR_MATRIX_CALC_TIMEOUT = 24,    /* matrix计算超时 */
    OAM_HAL_MAC_ERROR_CCA_TIMEOUT = 25,            /* cca超时 */
    OAM_HAL_MAC_ERROR_DCOL_DATA_OVERLAP = 26,      /* 数采overlap告警 */
    OAM_HAL_MAC_ERROR_BEACON_MISS = 27,            /* 连续发送beacon失败 */
    OAM_HAL_MAC_ERROR_UNKOWN_28 = 28,
    OAM_HAL_MAC_ERROR_UNKOWN_29 = 29,
    OAM_HAL_MAC_ERROR_UNKOWN_30 = 30,
    OAM_HAL_MAC_ERROR_UNKOWN_31 = 31,

    OAM_HAL_SOC_ERROR_BUCK_OCP = 32, /* PMU BUCK过流中断 */
    OAM_HAL_SOC_ERROR_BUCK_SCP,      /* PMU BUCK短路中断 */
    OAM_HAL_SOC_ERROR_OCP_RFLDO1,    /* PMU RFLDO1过流中断 */
    OAM_HAL_SOC_ERROR_OCP_RFLDO2,    /* PMU RFLDO2过流中断 */
    OAM_HAL_SOC_ERROR_OCP_CLDO,      /* PMU CLDO过流中断 */
    OAM_HAL_SOC_ERROR_RF_OVER_TEMP,  /* RF过热中断 */
    OAM_HAL_SOC_ERROR_CMU_UNLOCK,    /* CMU PLL失锁中断 */
    OAM_HAL_SOC_ERROR_PCIE_SLV_ERR,

    OAM_EXCP_TYPE_BUTT
} oam_excp_type_enum;

/* 跟踪用户信息变化类型，event类型信息，状态变化驱动 */
typedef enum {
    OAM_USER_INFO_CHANGE_TYPE_ASSOC_STATE = 0, /* 用户关联状态 */
    OAM_USER_INFO_CHANGE_TYPE_TX_PROTOCOL,     /* 发送数据帧使用的协议模式 */
    OAM_USER_INFO_CHANGE_TYPE_RX_PROTOCOL,     /* 接收数据帧使用的协议模式 */
    /* ... */
    OAM_USER_INFO_CHANGE_TYPE_BUTT
} oam_user_info_change_type_enum;

#define OAM_LOG_DEFAULT_LEVEL OAM_LOG_LEVEL_WARNING

typedef enum {
    OAM_EXCP_STATUS_INIT = 0,  /* 初始状态:上报完成后切换至初始状态 */
    OAM_EXCP_STATUS_REFRESHED, /* 有更新，可上报； */
    OAM_EXCP_STATUS_BUTT
} oam_excp_status_enum;

/* 异常统计结构体:当ul_reported_cnt与record_cnt相同时 */
typedef struct {
    osal_u32 status;     /* 每类异常类型是否刷新 */
    osal_u32 record_cnt; /* 记录上一次更新的次数 */
} dft_excp_record;

/* oam异常统计 */
typedef struct {
    osal_u32 status; /* VAP级别异常统计是否有刷新 */
    dft_excp_record excp_record[OAM_EXCP_TYPE_BUTT];
} dft_exception_ctx;

#ifndef DIAG_CMD_ID_WIFI_START
#define DIAG_CMD_ID_WIFI_START 0x7400
#endif

#ifndef DIAG_CMD_ID_WIFI_END
#define DIAG_CMD_ID_WIFI_END 0x7600
#endif

typedef enum {
    DIAG_CMD_ID_RX_DSCR = DIAG_CMD_ID_WIFI_START,
    DIAG_CMD_ID_TX_DSCR,
    DIAG_CMD_ID_RX_80211_FRAME,
    DIAG_CMD_ID_TX_80211_FRAME,
    DIAG_CMD_ID_RX_BEACON,
    DIAG_CMD_ID_TX_BEACON,
    DIAG_CMD_ID_TX_CB,
    DIAG_CMD_ID_RX_CB,

    DIAG_CMD_ID_WIFI_BUTT = DIAG_CMD_ID_WIFI_END
} wifi_diag_cmd_id;

oam_log_level_enum_uint8 dmac_dft_get_log_level_switch(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of common_dft.h */
