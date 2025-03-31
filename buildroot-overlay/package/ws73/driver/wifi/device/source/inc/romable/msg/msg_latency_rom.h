/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: Header file
 */

#ifndef MSG_LATENCY_ROM_H
#define MSG_LATENCY_ROM_H

#include "osal_types.h"

typedef enum {
    LATENCY_REPORT_MODE_TRACE,
    LATENCY_REPORT_MODE_STATS,

    LATENCY_REPORT_MODE_NUM
} latency_report_mode;
typedef osal_u8 latency_mode_enum_uint8;

typedef enum {
    LATENCY_RPT_DIR_TX,
    LATENCY_RPT_DIR_RX,
    LATENCY_RPT_DIR_TXRX,

    LATENCY_REPORT_DIRECT_NUM
} latency_report_direct;
typedef osal_u8 latency_direct_enum_uint8;

typedef struct {
    osal_u16                    trace_cnt;
    osal_u8                     report_enable;
    latency_direct_enum_uint8   direct;
    latency_mode_enum_uint8     report_mode;
} latency_stat_switch_stru;
#endif