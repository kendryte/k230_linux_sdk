/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag mem
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_MEM_H__
#define __ZDIAG_MEM_H__
#include "zdiag_common.h"
#include "soc_zdiag_channel.h"
typedef enum {
    DIAG_MEM_TYPE_INIT,
    DIAG_MEM_TYPE_RX_DATA_BUF,
    DIAG_MEM_TYPE_MUX_BLK_CMD,
    DIAG_MEM_TYPE_MUX_CHAR_CMD,
    DIAG_MEM_TYPE_CALL_CMD,
    DIAG_MEM_TYPE_NORMAL_IND,
    DIAG_MEM_TYPE_MSG_IND,
} diag_mem_type;

typedef enum {
    DIAG_PKT_MALLOC_USR_RUN_CMD,
    DIAG_PKT_MALLOC_USR_REPORT_IND,
    DIAG_PKT_MALLOC_USR_REPORT_ACK,
    DIAG_PKT_MALLOC_USR_REPORT_MSG,
    DIAG_PKT_MALLOC_USR_HSO_MUX_RECV,
} diag_pkt_malloc_usr;

td_pvoid diag_malloc(diag_mem_type type, td_u32 size);
td_void diag_free(TD_CONST td_pvoid ptr);
td_void diag_pkt_malloc(diag_pkt_malloc_param *param, diag_pkt_malloc_result *result);
td_void diag_pkt_free(diag_pkt_malloc_result *result);
#endif
