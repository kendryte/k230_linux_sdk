/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: SOC lOG implement internal interface
 */
#ifndef __SOC_LOG_STRATEGY_H__
#define __SOC_LOG_STRATEGY_H__
#include "td_base.h"
#include "stdarg.h"
#include "soc_log.h"

#define SOC_LOG_DEFAULT_LEVEL SOC_TRACE_LEVEL_INFO
#define SOC_LOG_INVALID_LINE 0xFFFFFFFF
#define SOC_LOG_DEFAULT_MODULE_ID 0xFFFF

typedef struct {
    td_u8 type;
    td_u8 level;
    td_u16 module_id;
    TD_CONST td_char *fn_name;
    td_u32 line_num;
}soc_log_param;

typedef enum {
    SOC_LOG_TYPE_INVALID,
    SOC_LOG_TYPE_PRINT,
    SOC_LOG_TYPE_SIMPLE_PRINT,
    SOC_LOG_TYPE_KEY_TRACE,
}soc_log_type;

typedef td_void (*soc_log_hook)(soc_log_param *param, TD_CONST td_char *fmt, va_list args);

td_void soc_log_init(soc_log_hook hook);
#endif