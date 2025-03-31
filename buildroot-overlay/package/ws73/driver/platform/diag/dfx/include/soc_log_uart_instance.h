/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: SOC lOG implement using uart out put
 */
#ifndef __SOC_LOG_UART_INSTANCE_H__
#define __SOC_LOG_UART_INSTANCE_H__
#include "td_base.h"
#include "soc_log_strategy.h"

td_void log_out_put_uart_instance(soc_log_param *param, TD_CONST td_char *fmt, va_list args);
#ifdef CONFIG_LAMBORGHINI_DEVICE
#else
#ifndef CONFIG_CGRA_CORE
void UartPuts(const char *s, td_u32 len, td_u32 isLock);
#endif
#endif

#endif