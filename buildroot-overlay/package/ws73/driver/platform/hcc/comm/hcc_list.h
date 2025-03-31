/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc list completion.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef __HCC_LIST_H__
#define __HCC_LIST_H__

#include "td_base.h"
#include "hcc_comm.h"
#include "hcc_adapt.h"

#include "soc_osal.h"
#include "osal_list.h"

td_bool hcc_is_list_empty(hcc_data_queue *hcc_queue);
td_void hcc_list_head_deinit(hcc_data_queue *hcc_queue);
td_u32 hcc_list_head_init(hcc_data_queue *hcc_queue);
td_u32 hcc_list_len(TD_CONST hcc_data_queue *hcc_queue);
td_void hcc_list_add_tail(hcc_data_queue *hcc_queue, hcc_unc_struc *unc_buf);
td_void hcc_list_add_head(hcc_data_queue* hcc_queue, hcc_unc_struc *unc_buf);
struct osal_list_head *hcc_list_peek(TD_CONST struct osal_list_head *head);
td_void _hcc_list_unlink(struct osal_list_head *node);
struct osal_list_head *hcc_list_delete_head(hcc_data_queue *TD_CONST hcc_queue);
hcc_unc_struc *hcc_list_dequeue(hcc_data_queue *TD_CONST hcc_queue);
td_void hcc_list_free(hcc_handler *hcc, hcc_data_queue *hcc_queue);
td_void hcc_list_restore(hcc_data_queue *dst_queue, hcc_data_queue *src_queue);
td_void _hcc_list_splice(TD_CONST struct osal_list_head *list, struct osal_list_head *prev,
    struct osal_list_head *next);
td_void hcc_list_splice_init(hcc_data_queue *hcc_queue, hcc_data_queue *head);
#endif /* __HCC_LIST_H__ */
