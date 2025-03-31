/*
 * Copyright (c) CompanyNameMagicTag. 2021-2021. All rights reserved.
 * Description: ints_table head.
 * Author: Huanghe
 * Create: 2021-07-05
 */

#ifndef _INTS_TABLE_H
#define _INTS_TABLE_H


    /*  Cortex-M7 Processor Exceptions Numbers */

    /*  Cortex-M7 Processor IRQ Numbers */

#define WDT_IRQ_NUM                     12
#define RISCV_SYS_VECTOR_CNT            26

#define LOSCFG_HWI_PRIO_LIMIT           7

#define NUM_INTERRUPT_TIMER0            (0 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_TIMER1            (1 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_TIMER2            (2 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_RTC0              (3 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_RTC1              (4 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_UART0             (5 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_UART1             (6 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_GPIO0             (7 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_GPIO1             (8 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_GLE               (9 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_SOFT0             (10 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_SOFT1             (11 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_SOFT2             (12 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_DMA               (13 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_COEX_WL           (14 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_COEX_BT           (15 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_WLAN_RESUME       (16 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_WLAN_GLB_UART_RX_WAKE       (17 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_WP0               (18 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_WLMAC             (19 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_BLE               (20 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_OSC_EN            (21 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_TSENSOR           (22 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_PMU_CMU_ERR       (23 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_DIAG              (24 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_MAC_MONITOR       (25 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_SDCC_WAKUP        (26 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_USB_WAKUP         (26 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_SDIO              (27 + RISCV_SYS_VECTOR_CNT)
#define IRQ_USB                         (27 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_USB_BIT1          (28 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_USB_TURBO_ERR     (29 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_HOST_REG_INT0     (30 + RISCV_SYS_VECTOR_CNT)
#define NUM_INTERRUPT_HOST_REG_INT1     (31 + RISCV_SYS_VECTOR_CNT)

#define NUM_INTERRUPT_MAX               (NUM_INTERRUPT_HOST_REG_INT1 + 1)

#endif

