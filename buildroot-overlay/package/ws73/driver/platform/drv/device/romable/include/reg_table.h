/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: 3883 register address macro definition.
 * Author: CompanyName
 * Create: 2021-09-15
 */
#ifndef __REG_TABLE_H__
#define __REG_TABLE_H__

// register base addr for that can not be found in hal_soc_reg.h

/* sdio base addr */
#define SDIO_REG_BASE       0x47000000

/* uart base addr */
#define EXT_UART0_REG_BASE   0x40002000
#define EXT_UART1_REG_BASE   0x40006000

/* trng base addr */
#define REG_TRNG_BASE       0x40003000

/* watch dog base addr */
#define WDG_REG_BASE        0x40017c00

/* dma base addr */
#define DMA_BASE_ADDR           0x47001000

/* uart1 rts cts pin io */
#define UART1_CTS_PINMUX_ADDR       0x4001e008
#define UART1_RTS_PINMUX_ADDR       0x4001e028

/* uart1 rx tx pin io */
#define UART1_RXD_PINMUX_ADDR       0x4001e014
#define UART1_TXD_PINMUX_ADDR       0x4001e018

/* gpio base addr */
#define EXT_GPIO_BASE_0      0x4001b000
#define EXT_GPIO_BASE_1      0x4001c000

/* for base addr that can be found in hal_soc_reg.h */

#define HRTIMER_IDX_0   0
#define HRTIMER_IDX_1   1
#define HRTIMER_IDX_2   2
#define HRTIMER_NUM     3

#define TIMER0_REG_BASE (TIMER_BASE + (HRTIMER_IDX_0 * 0x100))
#define TIMER1_REG_BASE (TIMER_BASE + (HRTIMER_IDX_1 * 0x100))
#define TIMER2_REG_BASE (TIMER_BASE + (HRTIMER_IDX_2 * 0x100))

/* uart clock */
#define CONFIG_UART0_CLOCK                    24000000
#define CONFIG_UART1_CLOCK                    24000000

#define CONFIG_UART_CLOCK_24MHZ               24000000
#define CONFIG_UART_CLOCK_40MHZ               40000000

// clocks
#define RTC_SOURCE_CLK      1000000

#define uapi_reg_write(addr, val)            (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define uapi_reg_read(addr, val)             ((val) = *(volatile unsigned int *)(uintptr_t)(addr))

#endif
