/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hcc adapt uart.
 * Author: CompanyName
 * Create: 2023-09-19
 */

#ifndef HCC_ADAPT_UART_HEADER
#define HCC_ADAPT_UART_HEADER

#include "osal_types.h"
#include "hcc_bus_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct uart_packet_head_t {
    td_u8 packet_start;
    struct {
        td_u8 type : 3;
        td_u8 queue_id : 5;
    } packet_info;
    td_u8 len_h;
    td_u8 len_l;
} uart_packet_head;

hcc_bus *hcc_adapt_uart_load(td_void);
td_void hcc_adapt_uart_unload(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
