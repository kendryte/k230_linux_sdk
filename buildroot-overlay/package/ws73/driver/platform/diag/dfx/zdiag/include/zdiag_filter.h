/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: diag filter
 * This file should be changed only infrequently and with great care.
 */
#ifndef __ZDIAG_FILTER_H__
#define __ZDIAG_FILTER_H__
#include "td_type.h"
#include "td_base.h"
#include "soc_zdiag.h"
#include "zdiag_common.h"

#define ZDIAG_FILTER_GROUP_NUM 10

typedef struct {
    td_bool enable;
    td_u8 level;
    zdiag_addr tool_addr;
    td_u32 last_rev_time;
    td_u16 enable_id[ZDIAG_FILTER_GROUP_NUM];
} zdiag_filter_ctrl;

typedef td_void (*zdiag_filter_notify_hook)(td_bool enable);

zdiag_filter_ctrl *zdiag_get_filter_ctrl(td_void);

td_void zdiag_set_enable(td_bool enabl, zdiag_addr addr);
td_bool zdiag_is_enable(td_void);
td_void zdiag_set_level_enable(td_u8 level, td_bool enable);
td_void zdiag_set_id_enable(td_u32 id, td_bool enable);

td_bool zdiag_log_enable(td_u8 level, td_u32 id);
td_void zdiag_filter_register_notify_hook(zdiag_filter_notify_hook hook);
td_void zdiag_filter_init(td_void);

td_void zdiag_state_beat_heart_pkt_recv(zdiag_addr peer_addr);
td_u32 zdiag_state_get_last_recv_time(zdiag_addr peer_addr);
zdiag_addr zdiag_get_connect_tool_addr(td_void);
#endif
