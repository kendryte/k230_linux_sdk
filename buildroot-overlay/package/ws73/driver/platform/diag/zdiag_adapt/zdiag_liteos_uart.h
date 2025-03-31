/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: diag uart utils for linux kernel
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_LITEOS_UART_H__
#define __ZDIAG_LITEOS_UART_H__

#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_log_adapt.h"
#include "soc_osal.h"

typedef struct {
    osal_wait wq;
    osal_task *rx_thread;
    osal_bool rx_task_exit;
    osal_u8 rsv[3];
} zdiag_crx_stru;

ext_errno zdiag_uart_output_buf(td_u8 *hcc_buf, td_u16 len);
ext_errno zdiag_uart_channel_init(td_void);
ext_errno zdiag_uart_channel_exit(td_void);

#endif
