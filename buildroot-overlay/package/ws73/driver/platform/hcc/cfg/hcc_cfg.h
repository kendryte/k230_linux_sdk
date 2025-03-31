/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc product configuration.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef __HCC_CFG_H__
#define __HCC_CFG_H__

#include "osal_types.h"
#include "hcc_cfg_comm.h"

#define HCC_TEST_MEMORY_PKG_TOP_CNT 128 // Host 侧内存包数量
#define RX_DATA_FORM CORES_PROTOCOL_CORE
#define TX_DATA_DEST_CORE CORES_PROTOCOL_CORE
#define TX_DATA_SOURCE_CORE CORES_APPLICATION_CORE
#define HCC_TRANS_THREAD_PRIORITY   5
#define HCC_RX_THERAD_PRIORITY   5
#define HCC_TRANS_THREAD_TASK_STACK_SIZE 8192
#define HCC_RX_PROC_TASK_STACK_SIZE 4096

#define HCC_RX_MAX_MESSAGE D2H_MSG_COUNT
#define HCC_TX_MAX_MESSAGE H2D_MSG_COUNT
#if defined(_PRE_WLAN_FEATURE_WS73) && defined(WSCFG_BUS_SDIO)
#define HCC_TASK_PRIORITY 10     /* 该优先级不能比内核的sdio_rx(内核默认FIFO 50)优先级高 */
#else
#define HCC_TASK_PRIORITY 99
#endif

typedef enum _h2d_msg_type hcc_tx_msg_type;
typedef enum _d2h_msg_type hcc_rx_msg_type;

#define FC_MSG_FLOWCTRL_ON  D2H_MSG_FLOWCTRL_ON
#define FC_MSG_FLOWCTRL_OFF D2H_MSG_FLOWCTRL_OFF

typedef enum _hcc_channel_name_ {
    HCC_CHANNEL_HMAC = 0x0,
    HCC_CHANNEL_DMAC = 0x1,
    HCC_CHANNEL_AP = 0x2,
    HCC_CHANNEL_COUNT = 0x3,
    HCC_CHANNEL_MAX = 0xFF,
} hcc_channel_name;
#endif /* __HCC_CFG_H__ */
