/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag channel adapt api header file
 * This file should be changed only infrequently and with great care.
 */
#ifndef __SOC_ZDIAG_CHANNEL_H__
#define __SOC_ZDIAG_CHANNEL_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_config.h"
#include "zdiag_common.h"

typedef enum {
    SOC_DIAG_CHANNEL_ATTR_NEED_RX_BUF = 0x1,
} soc_diag_channel_attribute;

typedef enum {
    DIAG_PKT_MALLOC_TYPE_CMN,
    DIAG_PKT_MALLOC_TYPE_IPC,
} diag_pkt_malloc_type;

typedef struct {
    td_u8 type;
    td_u8 pad;
    td_u16 size;
} diag_pkt_malloc_param_basic;

typedef struct {
    td_u8 info[0];
} diag_pkt_malloc_param_cmn;

typedef struct {
    zdiag_addr addr;
} diag_pkt_malloc_param_ipc;

typedef union {
    diag_pkt_malloc_param_cmn cmn;
    diag_pkt_malloc_param_ipc ipc;
} diag_pkt_malloc_param_diff;

typedef struct {
    diag_pkt_malloc_param_basic basic;
    diag_pkt_malloc_param_diff diff;
} diag_pkt_malloc_param;

typedef struct {
    td_u8 type;
    td_u8 pad;
    td_u16 size;
    td_u8 *buf;
} diag_pkt_malloc_result_basic;

typedef struct {
    td_u8 info[0];
} diag_pkt_malloc_result_cmn;

typedef struct {
    td_u8 *ipc_msg;
} diag_pkt_malloc_result_ipc;

typedef union {
    diag_pkt_malloc_result_cmn cmn;
    diag_pkt_malloc_result_ipc ipc;
} diag_pkt_malloc_result_diff;

typedef struct {
    diag_pkt_malloc_result_basic basic;
    diag_pkt_malloc_result_diff diff;
} diag_pkt_malloc_result;

typedef ext_errno (*diag_channel_tx_hook)(diag_pkt_malloc_result *mem);
typedef ext_errno (*diag_channel_notify_hook)(td_u32 id, td_u32 data);
typedef ext_errno (*diag_channel_zcopy_tx_hook)(diag_pkt_malloc_result *mem);

ext_errno soc_diag_channel_init(diag_channel_id id, td_u32 attribute);
ext_errno soc_diag_channel_exit(diag_channel_id id, td_u32 attribute);

ext_errno soc_diag_channel_set_tx_hook(diag_channel_id id, diag_channel_tx_hook hook);

ext_errno soc_diag_channel_set_notify_hook(diag_channel_id id, diag_channel_notify_hook hook);

ext_errno soc_diag_chanel_set_zcopy_tx_hook(diag_channel_id id, diag_channel_zcopy_tx_hook hook);

ext_errno soc_diag_channel_rx_mux_data(diag_channel_id id, td_u8 *data, td_u16 size);

td_s32 soc_diag_channel_rx_mux_char_data(diag_channel_id id, td_u8 *data, td_u16 size);

ext_errno soc_diag_channel_set_connect_hso_addr(diag_channel_id id, td_u8 hso_addr);
#endif
