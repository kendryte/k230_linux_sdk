/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: pkt router
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_PKT_ROUTER_H__
#define __ZDIAG_PKT_ROUTER_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_common.h"
#include "soc_zdiag_channel.h"

typedef struct {
    diag_pkt_proc cur_proc;
} diag_pkt_process_param;

ext_errno diag_pkt_router(diag_pkt_malloc_result *mem, diag_pkt_process_param *process_param);
td_void diag_pkt_msg_proc(td_u32 msg_id, td_u8 *msg, td_u32 msg_len);
#endif
