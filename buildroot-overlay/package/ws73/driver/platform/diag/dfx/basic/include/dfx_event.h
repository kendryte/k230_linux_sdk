/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: dfx event
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_EVENT_H__
#define __DFX_EVENT_H__
#include "td_type.h"
#include "soc_errno.h"
#include "dfx_feature_config.h"
#include "dfx_resource_id.h"
#define DFX_EVENT_SUPPORT_MAX_BIT_VAL 64
typedef ext_errno (*dfx_event_proces_hook)(td_u32 event_id);
typedef struct {
    td_u64 event_bits;
} dfx_event_wait_item;

typedef struct {
    td_u8 wait_pos;
    dfx_event_proces_hook *process_hook;
} dfx_event_item;

ext_errno dfx_event_write(dfx_event_wr_id event_id);
td_u64 dfx_event_select(dfx_event_group_id group, td_bool wait);
ext_errno dfx_event_wait(dfx_event_wr_id event_id, td_bool wait);
ext_errno dfx_event_init(td_u8 event_cnt);
#endif
