/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag dfx
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_DFX_H__
#define __ZDIAG_DFX_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_dfx_st.h"
#include "soc_zdiag.h"
#include "dfx_feature_config.h"

#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_DFX) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_DFX == DFX_YES)
extern zdiag_dfx_stat g_zdiag_dfx_stat;
#endif

static inline td_void diag_dfx_channel_receive_data(td_u32 chan_id, td_u32 cnt)
{
#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_DFX) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_DFX == DFX_YES)
    if (chan_id < DIAG_DFX_SUPPORT_MAX_CHANNEL) {
        g_zdiag_dfx_stat.channel_receive_data_cnt[chan_id] += cnt;
    }
#endif
}

static inline td_void diag_dfx_alloc_pkt(td_u32 pkt_type, td_u32 size)
{
#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_DFX) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_DFX == DFX_YES)
    if (pkt_type < DIAG_DFX_SUPPORT_MAX_PKT_MEM_TYPE) {
        g_zdiag_dfx_stat.mem_pkt_alloc_size[pkt_type] += size;
    }
#endif
}

static inline td_void diag_dfx_free_pkt(td_u32 pkt_type, td_u32 size)
{
#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_DFX) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_DFX == DFX_YES)
    if (pkt_type < DIAG_DFX_SUPPORT_MAX_PKT_MEM_TYPE) {
        g_zdiag_dfx_stat.mem_pkt_free_size[pkt_type] += size;
    }
#endif
}

#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_DFX) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_DFX == DFX_YES)
#define zdiag_dfx_put_msg_2_cache_fail() g_zdiag_dfx_stat.put_msg_2_cache_fail_times++
#define zdiag_dfx_log_received() g_zdiag_dfx_stat.log_receive_times++
#define zdiag_dfx_log_reported() g_zdiag_dfx_stat.log_reported_times++
#define zdiag_dfx_send_local_q_fail() g_zdiag_dfx_stat.send_local_q_fail++
#endif
zdiag_dfx_stat *uapi_zdiag_get_dfx_stat(td_void);
ext_errno diag_dfx_cmd(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_psd_enable(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);

#endif
