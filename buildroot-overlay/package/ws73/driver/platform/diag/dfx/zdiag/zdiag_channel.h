/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag channel
 * This file should be changed only infrequently and with great care.
 */
#ifndef __ZDIAG_CHANNEL_H__
#define __ZDIAG_CHANNEL_H__
#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag_channel.h"

typedef struct {
    td_u32 init : 1;
    td_u32 pad : 31;
    td_u32 name_flag;
    td_u8 *rx_buf;
    td_u32 au_id;
    td_u16 rx_buf_len;
    td_u16 rx_buf_pos;
    td_u16 using_size;
    td_u16 min_pkt_size;
    td_bool rx_buf_is_using;
    td_u8 hso_addr;
    td_u8 pad1[2];
    diag_channel_tx_hook tx_hook;
    diag_channel_zcopy_tx_hook zcopy_tx_hook;
    diag_channel_notify_hook notify_hook;
} diag_channel_item;

diag_channel_item *diag_chan_idx_2_item(diag_channel_id chan);
diag_channel_item *zdiag_dst_2_chan(td_u8 addr);
td_u32 zdiag_get_auid(td_void);

#endif
