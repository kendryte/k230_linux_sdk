/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: dfx thread
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_THREAD_H__
#define __DFX_THREAD_H__
#include "td_type.h"
#include "td_base.h"
#include "soc_errno.h"
#include "dfx_feature_config.h"

typedef int (*dfx_kthread_handler)(void *data);
typedef struct {
    td_u16 stack_size;
    td_u8 task_pri;
    void *data;
    dfx_kthread_handler handler;
} dfx_thread_config;

ext_errno dfx_thread_init(dfx_thread_config *configs, td_u8 thread_cnt);

#endif