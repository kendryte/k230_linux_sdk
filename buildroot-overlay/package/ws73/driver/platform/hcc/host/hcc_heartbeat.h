/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: host heartbeat header file \n
 *
 * History: \n
 * 2024-09-12, Create file. \n
 */

#ifndef __PLAT_HEARTBEAT_H__
#define __PLAT_HEARTBEAT_H__

#include "soc_osal.h"
#include "hcc_bus.h"

#define HCC_HEARTBEAT_MUTEX_TIMEOUT_MS  1000
#define HCC_HEARTBEAT_TIMER_INTERVAL_MS 1000
#define HCC_HEARTBEAT_ERROR_THRESHOLD   4
#define UINT32_MAX (0xffffffffu)

struct hcc_heartbeat_data {
    volatile osal_bool start;
    volatile osal_u32 check_cnt;
    osal_mutex lock;
    osal_timer timer;
    osal_workqueue workq;
};

osal_s32 hcc_heartbeat_device_enable(osal_void);
osal_s32 hcc_heartbeat_device_disable(osal_void);

osal_s32 hcc_heartbeat_start(osal_void);
osal_void hcc_heartbeat_stop(osal_void);
osal_s32 hcc_heartbeat_init(osal_void);
osal_void hcc_heartbeat_deinit(osal_void);

#endif
