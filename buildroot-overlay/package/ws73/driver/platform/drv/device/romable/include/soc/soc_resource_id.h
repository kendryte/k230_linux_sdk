/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: PCORE RESOURCE ID.
 */

#ifndef __SOC_RESOURCE_ID_H__
#define __SOC_RESOURCE_ID_H__

typedef enum {
    EXT_UART_IDX_0,      /* SHARE UART. */
    EXT_UART_IDX_1,      /* Physical port number 1. */
    EXT_UART_IDX_2,      /* Physical port number 2. */
    EXT_UART_IDX_3,      /* Physical port number 3. */
    EXT_UART_IDX_LP,
    EXT_UART_IDX_MAX,
    EXT_UART_IDX_INVALID_ID = 0xFF, /* Physical port number invalid id  */
} ext_uart_idx;

typedef enum {
    EXT_DMA_PERIPHERAL_MEMORY   = 0,
    EXT_DMA_PERIPHERAL_UART0_TX = 1,
    EXT_DMA_PERIPHERAL_UART0_RX = 2,
    EXT_DMA_PERIPHERAL_UART1_TX = 3,
    EXT_DMA_PERIPHERAL_UART1_RX = 4,
    EXT_DMA_PERIPHERAL_MAX_NUM,
} ext_dma_peripheral;

typedef enum {
    EXT_DMA_CHANNEL_NONE = 0xFF,
    EXT_DMA_CHANNEL_0 = 0,
    EXT_DMA_CHANNEL_1,
    EXT_DMA_CHANNEL_MAX_NUM,
} ext_dma_channel;

typedef enum {
    EXT_DMA_CH_PRIORITY_HIGHEST = 0,
    EXT_DMA_CH_PRIORITY_HIGH_1 = 1,
    EXT_DMA_CH_PRIORITY_HIGH_2 = 2,
    EXT_DMA_CH_PRIORITY_LOWEST = 3,
    EXT_DMA_CH_PRIORITY_RESERVED = 0xFF
} ext_dma_ch_priority;

#define uapi_dma_ch_priority_check_invalid(pri) (((ext_dma_ch_priority)(pri)) > EXT_DMA_CH_PRIORITY_LOWEST)

typedef enum {
    EXT_CLK_CPU     = 0,
    EXT_CLK_TIMER0  = 1,
    EXT_CLK_TIMER1  = 2,
    EXT_CLK_TIMER2  = 3,
    EXT_CLK_WDT     = 4,
    EXT_CLK_GPIO    = 5,
    EXT_CLK_TSENSOR = 6,
    EXT_CLK_ID_MAX,
    EXT_CLK_ID_INVALID_ID = 0xFFFFFFFF,
} ext_clk_id;
#endif
