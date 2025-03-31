/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag dfx
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_DFX_ST_H__
#define __ZDIAG_DFX_ST_H__
#include "td_type.h"
#define DIAG_DFX_SUPPORT_MAX_CHANNEL 4
#define DIAG_DFX_SUPPORT_MAX_PKT_MEM_TYPE 2

typedef struct {
    td_u32 case_id;
    td_u32 data[3];
}diag_dfx_cmd_req_st;

typedef struct {
    td_u32 case_id;
    td_u32 data[3];
}diag_dfx_cmd_ind_st;

typedef struct {
    td_u32 put_msg_2_cache_fail_times;
    td_u32 send_ipc_times;
    td_u32 send_used_size;
    td_u32 log_receive_times;
    td_u32 log_reported_times;
    td_u32 send_local_q_fail;
    td_u32 record_idx;
    td_u32 channel_receive_data_cnt[DIAG_DFX_SUPPORT_MAX_CHANNEL];
    td_u32 mem_pkt_alloc_size[DIAG_DFX_SUPPORT_MAX_PKT_MEM_TYPE];
    td_u32 mem_pkt_free_size[DIAG_DFX_SUPPORT_MAX_PKT_MEM_TYPE];
} zdiag_dfx_stat;
#endif
