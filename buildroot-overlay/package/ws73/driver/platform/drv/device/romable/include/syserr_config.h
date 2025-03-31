/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: SYSERR Library definitions
 */

#ifndef __SYSERR_CONFIG_H__
#define __SYSERR_CONFIG_H__

#include <td_base.h>
#include <soc_task.h>
#include <soc_mem.h>
#if defined(__LITEOS__)
#include <soc_os_stat.h>
#endif
#include <soc_exception.h>
#include "cores_types.h"

/* Features option */
typedef enum panic_id_e {
    // No panic
    PANIC_NONE = 0,
    PANIC_CPU_EXCEPTION = 1,
    PANIC_BTC_MALLOC_FAIL = 2,
    PANIC_WIFI_EXCEPTION = 3,
    PANIC_PLAT_EXCEPTION = 4,
    PANIC_WDT_TIMEOUT = 5,
    PANIC_ID_END = 0xff
} panic_id_e;

#endif /* __SYSERR_CONFIG_H__ */
