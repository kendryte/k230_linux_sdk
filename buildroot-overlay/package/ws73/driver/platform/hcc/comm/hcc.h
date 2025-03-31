/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc module completion.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_MODULE_HEADER
#define HCC_MODULE_HEADER

#include "td_base.h"
#include "hcc_comm.h"
#include "soc_errno.h"
#include "hcc_queue.h"
#include "hcc_bus_types.h"

/*
 * 低功耗hcc收发包统计结构
 * 统计数据不准确(未加锁,调度数量不确定),仅用于判断是否可以进低功耗
 */

td_void hcc_exception_reset_work(td_void);
td_void hcc_set_exception_status(td_bool status);
td_bool hcc_get_exception_status(td_void);
td_void hcc_proc_fail_count_add(td_void);
td_void hcc_proc_fail_count_clear(td_void);
td_u32 hcc_get_transfer_packet_num(hcc_handler *hcc);
td_s32 hcc_transfer_thread(td_void *data);
td_u16 hcc_tx_queue_proc(hcc_handler *hcc, hcc_trans_queue *queue);
hcc_service_type hcc_fuzzy_trans_queue_2_service(hcc_handler *hcc, hcc_queue_type queue_id);
hcc_service_type hcc_queue_id_2_service_type(hcc_handler *hcc, hcc_queue_dir dir, hcc_queue_type queue_id);
ext_errno hcc_sched_transfer(hcc_handler *hcc);
td_u16 hcc_rx_queue_proc(hcc_handler *hcc, hcc_trans_queue *queue);
td_void hcc_print_hex_dump(td_u8 *data, td_u32 len, char *str);
td_void hcc_change_state(osal_atomic *atomic, td_u32 state);
td_bool hcc_check_header_vaild(hcc_handler *hcc, hcc_header *hdr);
td_void hcc_init_unc_buf(hcc_unc_struc *unc_buf, td_u8* buf, td_u32 len, hcc_transfer_param *param);
#endif /* HCC_MODULE_HEADER */