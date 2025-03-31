/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag adapt os
 * This file should be changed only infrequently and with great care.
 */
#ifndef __ZDIAG_ADAPT_OS_H__
#define __ZDIAG_ADAPT_OS_H__

#include "td_base.h"
#include "td_type.h"
#include "soc_zdiag.h"
#include "zdiag_adapt_layer.h"

#define ACORE_ADDR 100
#define ACORE_HSO_ADDR 101
#define ACORE_HSOB_ADDR 102

#define PCORE_ADDR 200
#define PCORE_HSO_ADDR 201

enum DIAG_LOG_MODE {
    DIAG_LOG_TO_HSO = 0,
    DIAG_LOG_TO_FILE = 1,
    DIAG_LOG_TO_NULL
};

typedef enum {
    DIAG_HSO_CONN_NONE = 0,
    DIAG_HSO_OFFLINE_LOG = 0,
    DIAG_HSO_CONN_SOCK = 1,
    DIAG_HSO_CONN_UART = 2
} diag_hso_conn_stat;

td_u8 zdiag_log_mode_get(void);
void zdiag_log_mode_set(td_u8 dlog_mode);

diag_hso_conn_stat diag_hso_conn_state_get(td_void);
td_void diag_hso_conn_state_set(diag_hso_conn_stat state);

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
td_void diag_hcc_conn_state_set(td_bool state);
#endif
td_u32 diag_adapt_get_msg_time(td_void);
ext_errno diag_hcc_sync_cmd_to_dev(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
td_void *diag_adapt_malloc(unsigned size);
td_void diag_adapt_free(td_void *p);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
ext_errno diag_sync_cfg_cmd_to_dev(td_u32 level);
#endif

#endif
