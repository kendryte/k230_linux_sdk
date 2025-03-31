/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: DFR initialization and adapt interface implementation.
 */

#ifndef MSG_DFR_ROM_H
#define MSG_DFR_ROM_H

#include "hal_commom_ops_type_rom.h"
#include "wlan_spec_hh503_rom.h"

#ifdef _PRE_WLAN_DFR_STAT
typedef struct {
    osal_u16 mem_total_cnt;   /* 本内存池一共有多少内存块 */
    osal_u16 mem_used_cnt;    /* 本内存池已用内存块 */
    osal_u16 reserved;        /* 保留2字节对齐 */
    osal_u16 pending_pkt[HAL_RX_DSCR_QUEUE_ID_BUTT]; /* 记录pinding在乒乓队列里的帧数量 */
    osal_u32 mem_alloc_cnt;   /* 本内存池申请成功内存块统计 */
    osal_u32 mem_fail_cnt;    /* 本内存池申请成功内存块统计 */
    osal_u32 high_cnt;        /* 高优先级frw队列未被调用统计 */
    osal_u32 low_cnt;         /* 低优先级frw队列未被调用统计 */
    hal_rx_dscr_queue_header_stru rx_dscr_queue[HAL_RX_QUEUE_NUM];
    hal_tx_dscr_queue_header_stru tx_dscr_queue[HAL_TX_QUEUE_NUM];
} dfr_info_rsp_stru;
#endif

#endif