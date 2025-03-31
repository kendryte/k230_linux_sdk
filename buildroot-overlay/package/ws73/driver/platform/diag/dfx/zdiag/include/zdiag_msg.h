/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag msg
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_MSG_H__
#define __ZDIAG_MSG_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_common.h"
#include "soc_zdiag_channel.h"

typedef struct {
    td_u8 cur_proc;
    diag_pkt_malloc_result mem;
} diag_pkt_msg;

ext_errno diag_msg_proc(td_u16 msg_id, td_u8 *msg, td_u32 msg_len);
#endif
