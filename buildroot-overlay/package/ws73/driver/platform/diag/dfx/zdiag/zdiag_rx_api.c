/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag rx
 * This file should be changed only infrequently and with great care.
 */
#include "zdiag_rx_api.h"
#include "securec.h"
#include "zdiag_mem.h"
#include "zdiag_pkt.h"
#include "oal_debug.h"
#include "zdiag_channel.h"
#include "zdiag_pkt_router.h"
#include "zdiag_adapt_layer.h"
#include "zdiag_dfx.h"

#define MUX_START_OFFSET 0
#define MAX_MUX_PARAM_SIZE 512

TD_PRV td_s32 zdiag_hso_find_start_flag(td_u8 *data, td_u16 size)
{
    int i;
    int pos = -1;
    for (i = 0; i < size; i++) {
        td_u32 *flag = (td_u32 *)&data[i];
        if (*flag == MUX_START_FLAG) {
            pos = i;
            break;
        }
    }
    return pos;
}

TD_PRV td_s32 soc_diag_chan_copy_data(diag_channel_item *item, td_u8 *data, td_u16 size, td_s32 *real_size)
{
    td_u16 free_size = item->rx_buf_len - item->rx_buf_pos;
    td_u16 copy_size = uapi_min(size, free_size);
    if (memcpy_s(&item->rx_buf[item->rx_buf_pos], free_size, data, copy_size) != EOK) {
        oal_print_err("soc_diag_chan_copy_data memcpy_s error\n");
        *real_size = 0;
        return EXT_ERR_FAILURE;
    }
    item->rx_buf_pos += copy_size;
    *real_size = copy_size;
    return EXT_ERR_SUCCESS;
}

TD_PRV td_void diag_rest_rx_buf(diag_channel_item *item)
{
    item->rx_buf_pos = MUX_START_OFFSET;
    item->rx_buf_is_using = TD_FALSE;
}

TD_PRV td_u16 diag_get_rx_data_size(diag_channel_item *item)
{
    return item->rx_buf_pos - MUX_START_OFFSET;
}

TD_PRV td_pvoid diag_rx_mux_2_pkt(msp_mux_packet_head_stru *mux, diag_pkt_malloc_result *mem)
{
    zdiag_addr dst = mux->dst;
    diag_pkt_malloc_param param;
    diag_addr_attribute attribute;
    diag_pkt_malloc_param_ipc *ipc;

    if (dst == 0) {
        dst = diag_adapt_get_local_addr();
        mux->dst = dst;
    }

    attribute = diag_adapt_addr_2_attribute(mux->dst);
    if (attribute & DIAG_ADDR_ATTRIBUTE_HCC) {
        param.basic.type = DIAG_PKT_MALLOC_TYPE_IPC;
        ipc = &param.diff.ipc;
        ipc->addr = mux->dst;
    } else {
        param.basic.type = DIAG_PKT_MALLOC_TYPE_CMN;
    }
    param.basic.size = (td_u16)(sizeof(msp_mux_packet_head_stru) + mux->packet_data_size);

    diag_pkt_malloc(&param, mem);
    if (mem->basic.buf == TD_NULL) {
        return TD_NULL;
    }

    if (memcpy_s(mem->basic.buf, mem->basic.size, mux, sizeof(msp_mux_packet_head_stru) + mux->packet_data_size) !=
        EOK) {
        diag_pkt_free(mem);
        return TD_NULL;
    }
    return mem->basic.buf;
}

TD_PRV msp_mux_packet_head_stru *diag_chan_full_pkt_receive(diag_channel_item *item)
{
    ext_errno ret;
    msp_mux_packet_head_stru *mux = (msp_mux_packet_head_stru *)&item->rx_buf[MUX_START_OFFSET];
    if (mux->packet_data_size > MAX_MUX_PARAM_SIZE) {
        diag_rest_rx_buf(item);
        return TD_NULL;
    }

    if (diag_get_rx_data_size(item) < sizeof(msp_mux_packet_head_stru) + mux->packet_data_size) {
        return TD_NULL;
    }

    ret = zdiag_check_mux_pkt(mux, sizeof(msp_mux_packet_head_stru) + mux->packet_data_size);
    if (ret != EXT_ERR_SUCCESS) {
        diag_rest_rx_buf(item);
        return TD_NULL;
    }
    return mux;
}

TD_PRV ext_errno diag_chan_move_buf(diag_channel_item *item, td_u32 dst, td_u32 src, td_u32 len)
{
    if (EOK != memmove_s(&item->rx_buf[dst], item->rx_buf_len, &item->rx_buf[src], len)) {
        return EXT_ERR_FAILURE;
    }
    item->rx_buf_pos -= (td_u16)(src - dst);
    return EXT_ERR_SUCCESS;
}
TD_PRV td_s32 diag_chan_rx_mux_char_data(diag_channel_item *item, td_u8 *data, td_u16 size)
{
    td_s32 real_size = 0;
    td_s32 copyed_size = 0;
    td_s32 start_flag_pos;
    msp_mux_packet_head_stru *mux = TD_NULL;
    diag_pkt_malloc_result mem;
    diag_pkt_process_param process_param;

    while (copyed_size != size && item->rx_buf_pos != item->rx_buf_len) { /* 数据未处理完成 并且 接收缓存有空间 */
        if (soc_diag_chan_copy_data(item, data + copyed_size,
            (td_u16)(size - copyed_size), &real_size) != EXT_ERR_SUCCESS) {
            continue;
        }
        copyed_size += real_size;
        if (item->rx_buf_is_using) {
            return copyed_size;
        }

        if (diag_get_rx_data_size(item) < sizeof(msp_mux_packet_head_stru)) {
            continue;
        }

        start_flag_pos =
            zdiag_hso_find_start_flag(&item->rx_buf[MUX_START_OFFSET], item->rx_buf_pos - MUX_START_OFFSET);
        if (start_flag_pos < 0) {
            diag_rest_rx_buf(item);
            continue;
        } else if (start_flag_pos > 0) {
            diag_chan_move_buf(item, MUX_START_OFFSET, (td_u32)start_flag_pos,
                (td_u32)(item->rx_buf_pos - start_flag_pos));
        }

        if (diag_get_rx_data_size(item) < sizeof(msp_mux_packet_head_stru)) {
            continue;
        }

        mux = diag_chan_full_pkt_receive(item);
        if (mux == TD_NULL) {
            continue;
        }

        item->au_id = mux->au_id;
        mux->src = item->hso_addr;
        mux->ctrl = 0;

        diag_rx_mux_2_pkt(mux, &mem);
        if (mem.basic.buf == TD_NULL) {
            return copyed_size;
        }
        process_param.cur_proc = DIAG_PKT_PROC_USR_SYNC_CMD_IND;
        diag_pkt_router(&mem, &process_param);

        diag_chan_move_buf(item, MUX_START_OFFSET, sizeof(msp_mux_packet_head_stru) + mux->packet_data_size,
            item->rx_buf_pos - (sizeof(msp_mux_packet_head_stru) + mux->packet_data_size));
    }
    return copyed_size;
}

/* 字符串数据接收函数 */
td_s32 soc_diag_channel_rx_mux_char_data(diag_channel_id id, td_u8 *data, td_u16 size)
{
    diag_channel_item *item = diag_chan_idx_2_item(id);
    if (item == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    diag_dfx_channel_receive_data(id, size);

    return diag_chan_rx_mux_char_data(item, data, size);
}

ext_errno soc_diag_channel_rx_mux_data(diag_channel_id id, td_u8 *data, td_u16 size)
{
    ext_errno ret;
    diag_channel_item *item = diag_chan_idx_2_item(id);
    msp_mux_packet_head_stru *mux = (msp_mux_packet_head_stru *)data;
    diag_pkt_malloc_result mem;

    diag_pkt_process_param process_param;

    if (item == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    diag_dfx_channel_receive_data(id, size);

    ret = zdiag_check_mux_pkt(mux, size);
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }

    diag_rx_mux_2_pkt(mux, &mem);
    if (mem.basic.buf == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    process_param.cur_proc = DIAG_PKT_PROC_PORT_PKT;

    return diag_pkt_router(&mem, &process_param);
}
