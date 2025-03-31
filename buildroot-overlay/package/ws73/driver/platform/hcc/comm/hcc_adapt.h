/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: the header file of hcc private interfacer.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_ADAPT_HEADER
#define HCC_ADAPT_HEADER

#include "td_base.h"
#include "hcc_comm.h"
#include "hcc_queue.h"

hcc_unc_struc *hcc_alloc_unc_buf(hcc_handler *hcc);
td_void hcc_free_unc_buf(hcc_handler *hcc, hcc_unc_struc *unc_buf);
hcc_unc_struc* hcc_adapt_alloc_unc_buf(hcc_handler *hcc, td_u32 len, hcc_queue_type queue_id);
td_void hcc_adapt_mem_free(hcc_handler *hcc, hcc_unc_struc *unc_buf);
td_void hcc_unc_buf_enqueue(hcc_handler *hcc, hcc_queue_type queue_id, hcc_unc_struc *unc_buf);
ext_errno hcc_bus_rx_handler(hcc_bus *bus, td_u8 queue_id, hcc_data_queue *assembled_head);
td_u32 hcc_adapt_alloc_priv_buf(
    hcc_handler *hcc, hcc_queue_type queue_id, td_u32 buf_len, td_u8 **buf, td_u8 **user_param);
td_void hcc_adapt_free_priv_buf(hcc_handler *hcc, hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param);
#endif /* HCC_ADAPT_HEADER */
