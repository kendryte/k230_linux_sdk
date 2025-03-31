/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag tx proc
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_TX_PROC_H__
#define __ZDIAG_TX_PROC_H__

#include "td_base.h"
#include "td_type.h"
#include "diag_queue.h"
#include "soc_osal.h"
#include "soc_errno.h"

#define ZDIAG_TX_PROC_TASK_PRIORITY   11
#define ID_MSG_ZDIAG_BASE 0x100
enum {
    ID_MSG_ZDIAG_CHIP_LOG_TX = (ID_MSG_ZDIAG_BASE + 1), /* Packet reporting */
    ID_MSG_ZDIAG_HOST_LOG_TX,                           /* User data packet report */
    ID_MSG_ZDIAG_ACK_RX,
    ID_MSG_ZDIAG_ACK_TX, /* ACK packet, need to release the parameter pointer */
    ID_MSG_ZDIAG_RX,
    ID_MSG_ZDIAG_TIMER,
    ID_MSG_ZDIAG_ACK_MDM_RX,
    ID_MSG_ZDIAG_UART_RX
};

//
// Zdiag Tx Proc Thread
//
typedef struct {
    diag_queue *tx_queue;
    osal_spinlock tx_q_spinlock;
    osal_u32 tx_fail_cnt;
    osal_u32 tx_all_cnt;
    osal_u32 tx_succ_cnt;

    osal_task* diag_tx_proc_task;
    osal_wait diag_tx_wait;
} zdiag_ctx_stru;

ext_errno zdiag_tx_send_host_log2diag(td_u8 *hcc_buf, td_u16 len);
ext_errno zdiag_send_oam_log(td_u8 *buf, td_u16 buf_len, td_u32 msg_id);
ext_errno zdiag_tx_proc_init(td_void);
ext_errno zdiag_tx_proc_exit(td_void);
zdiag_ctx_stru *zdiag_get_ctx(osal_void);
#endif
