/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc flow ctrl module completion.
 * Author: CompanyName
 * Create: 2021-06-30
 */

#ifndef HCC_FLOW_CTRL_HEADER
#define HCC_FLOW_CTRL_HEADER

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
#include "td_base.h"
#include "hcc_comm.h"
#include "hcc_queue.h"
#include "soc_errno.h"

#define HCC_FC_PRE_PROC_WAIT_TIME 1000

typedef osal_u32 (*hcc_flow_ctrl_credit_cb)(osal_void);
td_void hcc_flow_ctrl_credit_register(td_void *cb);
td_u16 hcc_flow_ctrl_credit_update(hcc_handler *hcc, hcc_trans_queue *queue, td_u16 remain_pkt_nums);
td_bool hcc_flow_ctrl_sched_check(hcc_trans_queue *queue);
ext_errno hcc_flow_ctrl_process(hcc_handler *hcc, hcc_trans_queue *queue);
td_u32 hcc_flowctrl_on_proc(td_u8 *data);
td_u32 hcc_flowctrl_off_proc(td_u8 *data);
td_void hcc_flowctrl_msg_init(hcc_channel_name channel_name);
ext_errno hcc_flow_ctrl_pre_proc(hcc_handler *hcc, hcc_transfer_param *param, hcc_trans_queue *hcc_queue);
td_void hcc_adapt_tx_start_subq(hcc_handler *hcc, hcc_trans_queue *hcc_queue);
ext_errno hcc_flow_ctrl_module_init(hcc_handler *hcc);
td_void hcc_flow_ctrl_module_deinit(hcc_handler *hcc);
#endif

#endif /* HCC_FLOW_CTRL_HEADER */