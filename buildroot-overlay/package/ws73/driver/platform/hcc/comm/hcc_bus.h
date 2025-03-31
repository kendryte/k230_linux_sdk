/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc adapt bus completion.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_BUS_INTERFACE_HEADER
#define HCC_BUS_INTERFACE_HEADER

#include "td_base.h"
#include "hcc_comm.h"
#include "hcc_cfg.h"
#include "hcc_queue.h"
#include "hcc_bus_types.h"

hcc_bus *hcc_get_channel_bus(hcc_channel_name chl);
bus_dev_ops *hcc_get_bus_ops(hcc_bus *bus);
hcc_bus *hcc_alloc_bus(td_void);
td_void hcc_free_bus(hcc_bus *bus);
td_u32 hcc_bus_load(hcc_bus_type bus_type, hcc_handler *hcc);
td_void hcc_bus_unload(hcc_bus *bus);
td_u32 hcc_bus_tx_proc(hcc_bus *bus, hcc_trans_queue *queue, td_u16 *remain_pkt_nums);
td_bool hcc_bus_is_busy(hcc_bus *bus, hcc_queue_dir dir);
td_s32 hcc_bus_call_rx_message(hcc_bus *bus, hcc_rx_msg_type msg);
td_void hcc_bus_update_credit(hcc_bus *bus, td_u32 credit);
td_s32 hcc_bus_get_credit(hcc_bus *bus, td_u32 *credit);
td_s32 hcc_bus_reinit(hcc_bus *bus);
td_s32 hcc_bus_patch_read(hcc_bus *bus, td_u8 *buff, td_s32 len, td_u32 timeout);
td_s32 hcc_bus_patch_write(hcc_bus *bus, td_u8 *buff, td_s32 len);

/* bus cfg */
hcc_bus_load_func hcc_get_bus_load_func(td_u8 bus_type);
hcc_bus_unload_func hcc_get_bus_unload_func(td_u8 bus_type);
hcc_queue_cfg *hcc_get_queue_cfg(td_u8 *arr_len);
td_char *hcc_get_task_name(hcc_channel_name channel_name);
td_u32 hcc_get_bus_max_trans_size(int bus_type_);
td_void hcc_force_update_queue_id(hcc_bus *bus, td_u8 value);
td_s32 hcc_bus_power_action(hcc_bus *bus, hcc_bus_power_action_type action);
td_s32 hcc_bus_sleep_request(hcc_bus *bus);
td_s32 hcc_bus_wakeup_request(hcc_bus *bus);

#define BUS_LOG_SYMBOL_SIZE 4
#define BUS_LOG_SYMBOL_NUM 5
extern td_s8 g_loglevel[BUS_LOG_SYMBOL_NUM][BUS_LOG_SYMBOL_SIZE];
#endif /* HCC_BUS_INTERFACE_HEADER */
