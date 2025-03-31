/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: plat log level header file.
 * Author: Huanghe
 * Create: 2023-02-01
 */

#ifndef __PLAT_SERVICE_H__
#define __PLAT_SERVICE_H__

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    DEV_PANIC = 1,
    DRIVER_HUNG,
    EVENT_UNKNOWN,
} wifi_driver_event;

typedef int (*wifi_driver_event_cb)(wifi_driver_event event);

typedef struct {
    osal_u32 id;
    osal_u32 val;
} gpio_param_t;

osal_void plat_kernel_loglevel_set(osal_s32 log_level);
osal_s32 wifi_driver_event_callback_register(wifi_driver_event_cb event_cb);
osal_u32 plat_pm_func_enable_switch(osal_s32 pm_switch);
osal_s32 pm_board_power_reset(td_void);
osal_s32 plat_reset_reinit_etc(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
