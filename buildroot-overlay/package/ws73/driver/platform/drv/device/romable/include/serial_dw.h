/*
 * Copyright (c) CompanyNameMagicTag 2020-2021. All rights reserved.
 * Description: Boot uart driver head file.
 * Author: CompanyName
 * Create: 2012-12-22
 */

#ifndef __SERIAL_DW_H__
#define __SERIAL_DW_H__

#include "td_base.h"

/* UART register OFFSET */
#define UART_DR                     0x0
#define UART_RSR_ECR                0x04
#define UART_FR                     0x18
#define UART_ILPR                   0x20
#define UART_IBRD                   0x24
#define UART_FBRD                   0x28
#define UART_LCR_H                  0x2C
#define UART_CR                     0x30
#define UART_IFLS                   0x34
#define UART_IMSC                   0x38
#define UART_RIS                    0x3C
#define UART_MIS                    0x40
#define UART_ICR                    0x44
#define UART_DMACR                  0x48

#define UARTFR_TXFE_MASK            0x80
#define UARTFR_RXFF_MASK            0x40
#define UARTFR_TXFF_MASK            0x20
#define UARTFR_RXFE_MASK            0x10
#define UARTFR_BUSY_MASK            0x08
#define UARTDR_DATA_MASK            0xFF

#define UARTLCR_H_CFG               0x60   /* 8bit, no parity, FIFO disable */
#define UARTLCR_H_CFG_ODD           0x72   /* 8bit, odd parity,FIFO enable */
#define UARTLCR_H_CFG_FIFO          0x70   /* 8bit, no parity, FIFO enable */
#define UARTCR_CFG                  0x301  /* UART tx enable, rx enable, uart enable */
#define UARTCR_CFG_FLOWENABLE       0xC301 /* UART tx enable, rx enable, uart enable, flow control enable */
#define UARTCR_RTX_FC_SHIFT         14
#define UARTCR_CTX_FC_SHIFT         15
#define UARTIMSC_CFG                0x0    /* Disable all uart interrupt */
#define UARTIMSC_CFG_INT            0x50   /* enable rx time out interrupt */
#define UARTIFS_CFG                 0x10A  /* FIFO water mark:Rx 16 Tx 32 RTS 56 */
#define UARTIFS_RXFF_SHIFT          3
#define UARTIFS_RTSFF_SHIFT         6

#define UARTRIS_RXRIS_SHIFT         4

#define UART_RX_FIFO_HALF         32

#define UART_DMA_ENABLE             0X03
#define UART_DMA_DISABLE            0X04
#define UARTICR_CFG                 0x7FF  /* Clear up all uart interrupt */

/* uart1 flowctr */
#define UART1_FLOWCTR_PIN_CONFIG    5
#define UART1_PINMUX_CONFIG         1
#define MAX_BUFF_SIZE               8

#define UART_TIME_OUT               500

typedef struct {
    td_u32 uart_clock;
    td_u32 baudrate;
    td_uchar databit;
    td_uchar stopbit;
    td_uchar parity;
    td_uchar flow_ctrl;
    td_uchar fifoline_tx_int;
    td_uchar fifoline_rx_int;
    td_uchar fifoline_rts;
    td_uchar pad;
} uart_param_stru;

typedef enum {
    UART_BUS_0 = 0,
    UART_BUS_1 = 1,
    UART_BUS_MAX
} uart_bus_t;

#define DEBUG_MSG_UART_BUS UART_BUS_0

/**
* @ingroup  hct_boot_api
* @brief  打印输出信息，只输出字符串
*
* @par 描述:
*         打印输出信息，只输出字符串。
* @attention   无。
*
* @param  s [IN] 类型 #td_char*  待输出的字符串
*
* @retval 无
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see  boot_put_errno|boot_puthex|boot_msg1|boot_msg2|boot_msg4
* @since ws73
 */
td_void debug_message(const td_char *fmt, ...);

/**
* @ingroup  hct_boot_api
* @brief  串口输出禁言
*
* @par 描述:
* 串口输出禁言，即不允许串口输出调试信息，但BOOT_PUT_ERRNO的输出不禁止
* @attention   无。
* @param 无。
*
* @retval 无。
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see  serial_cancel_mute
* @since ws73
 */
td_void serial_set_mute(td_void);

/**
* @ingroup  hct_boot_api
* @brief  串口输出取消禁言
*
* @par 描述:
* 串口输出取消禁言，即允许串口输出调试信息
* @attention 无。
* @param 无。
*
* @retval 无。
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see  serial_set_mute
* @since ws73
 */
td_void serial_cancel_mute(td_void);

/**
* @ingroup  hct_boot_api
* @brief  串口初始化
*
* @par 描述:
* 串口初始化。
* @attention 无。
*
* @param  uart_num [IN] 类型 #td_uart  串口号
* @param  default_uart_param [IN] 类型 #uart_param_stru  串口参数
*
* @retval #EXT_ERR_FAILURE 失败。
* @retval #EXT_ERR_SUCCESS 成功。
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see serial_putc|serial_puts|serial_put_buf|serial_getc|serial_tstc|serial_puthex|serial_getc_timeout
* @since ws73
 */
td_u32 serial_init(uart_bus_t uart, uart_param_stru default_uart_param);

/**
* @ingroup  hct_boot_api
* @brief  串口输出单个字符
*
* @par 描述:
* 串口输出字符。
* @attention   无。
*
* @param  c [IN] 类型 #td_char  待输出的字符
*
* @retval 无。
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see serial_init|serial_puts|serial_put_buf|serial_getc|serial_tstc|serial_puthex|serial_getc_timeout
* @since ws73
 */
td_void serial_putc(uart_bus_t uart, const td_char c);

/**
* @ingroup  hct_boot_api
* @brief  串口输出字符串
*
* @par 描述:
* 串口输出字符串。
* @attention   无。
*
* @param  s [IN] 类型 #td_char*  待输出的字符串
*
* @retval 无。
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see serial_init|serial_putc|serial_put_buf|serial_getc|serial_tstc|serial_puthex|serial_getc_timeout
* @since ws73
 */
td_void serial_puts(uart_bus_t uart, const td_char *s);

/**
* @ingroup  hct_boot_api
* @brief  从串口读取一个字符
*
* @par 描述:
*         从串口读取一个字符。
* @attention
*         本函数为阻塞接口，如果串口没有接收到字符，会一直等待。
* @param  无。
*
* @retval  #字符  从串口读取到的字符
* @retval  #-1    错误
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see serial_init|serial_putc|serial_puts|serial_put_buf|serial_tstc|serial_puthex|serial_getc_timeout
* @since ws73
 */
td_u8 serial_getc(uart_bus_t uart);

/**
* @ingroup  hct_boot_api
* @brief  测试串口是否有字符待接收
*
* @par 描述:
*         测试串口是否有字符待接收。
* @attention  无。
*
* @param  无
*
* @retval #0     串口上无数据
* @retval #1     串口上有数据
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see serial_init|serial_putc|serial_puts|serial_put_buf|serial_getc|serial_puthex|serial_getc_timeout
* @since ws73
 */
td_s32 serial_tstc(uart_bus_t uart);

/**
* @ingroup  hct_boot_api
* @brief  向串口输出16进制数。
*
* @par 描述:
*         向串口输出16进制数。
* @attention   无。
*
* @param  h [IN] 类型 #td_u32  待输出的16进制数
* @param  print_all [IN] 类型 #td_bool  打印时前面是否补0，按照4字节对齐，1表示补,0表示不补。
*
* @retval 无。
*
* @par Dependency:
* <ul><li>soc_boot_rom.h: 该接口声明所在的头文件.</li></ul>
* @see serial_init|serial_putc|serial_puts|serial_put_buf|serial_getc|serial_tstc|serial_getc_timeout
* @since ws73
 */
td_void serial_puthex(uart_bus_t uart, td_u32 h, td_bool print_all);

/*
 * Description:  串口输出字符串
 * 参数：
 *      *buffer: 字符串首地址
 *      length:  字符串长度
 * return：
 *      the length of already send
 */
td_s32 serial_put_buf(uart_bus_t uart, const td_char *buffer, td_s32 length);

td_void boot_msg0(const td_char *s);

td_void boot_msg1(const td_char *s, td_u32 h);

td_void boot_msg2(const td_char *s, td_u32 h1, td_u32 h2);

td_void debug_message(const td_char *fmt, ...);

td_u32 set_uart_param(uart_bus_t uart, uart_param_stru uart_param);

td_void set_uart_int(uart_bus_t uart, td_u8 int_type, td_bool int_state);

td_void set_uart_dma(uart_bus_t uart, td_bool dma_state);

td_u8 get_uart_state(uart_bus_t uart);

td_void clear_uart_state(uart_bus_t uart, td_u8 int_type);

td_bool getc_timeout(uart_bus_t uart, td_u8 *dat);
#endif
