/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag usr adapt
 * This file should be changed only infrequently and with great care.
 */

#ifndef __DIAG_ADAPT_LAYER_H__
#define __DIAG_ADAPT_LAYER_H__
#include "td_base.h"
#include "soc_errno.h"
#ifdef __KERNEL__
#include "linux/kernel.h"
#else
#include "soc_log.h"
#endif


/* Diag 打印接口 */
#ifdef CONFIG_ZDIAG_PRINT_DEBUG

#ifdef CONFIG_ZDIAG_PRINT_SUPPORT_LINUX_OS
#include "oal_util.h"
#define zdiag_print(fmt, arg...) printk(fmt, ##arg)
#elif defined(CONFIG_ZDIAG_PRINT_SUPPORT_NON_OS)
#include "serial_dw.h"
#define zdiag_print(fmt, arg...) debug_message(fmt, ##arg)
#elif defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define zdiag_print(fmt, arg...) dprintf("[ZDIAG] " fmt, ##arg)
#else
#define zdiag_print(fmt, arg...) printf("[ZDIAG] " fmt, ##arg)
#endif /* CONFIG_ZDIAG_PRINT_SUPPORT_LINUX_OS */

#else

#define zdiag_print(fmt, arg...)
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define zdiag_err_print(fmt, arg...) dprintf("[ZDIAG ERR] " fmt, ##arg)
#endif /* _PRE_OS_VERSION_LITEOS */

#endif /* CONFIG_ZDIAG_PRINT_DEBUG */
#endif
