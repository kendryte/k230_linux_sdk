/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: diag uart utils for linux kernel
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_LINUX_UART_H__
#define __ZDIAG_LINUX_UART_H__

#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_log_adapt.h"

typedef struct {
    td_u32 uart_rx_cnt;
    td_u32 uart_tx_cnt;
} zdiag_uart_ctrl;

ext_errno zdiag_uart_output_buf(td_u8 *hcc_buf, td_u16 len);
ext_errno zdiag_uart_channel_init(td_void);
ext_errno zdiag_uart_channel_exit(td_void);

zdiag_uart_ctrl *zdiag_uart_get_ctrl(td_void);

#endif
