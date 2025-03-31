/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: hcc_uart_host header file
 * Author: @CompanyNameTag
 */

#ifndef __HCC_UART_HOST_H__
#define __HCC_UART_HOST_H__

#ifdef CONFIG_HCC_SUPPORT_UART
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/serial_core.h>
#include "td_type.h"
#include "hcc_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define UART_DEV_NAME_LEN 32
#define UART_BAUD_RATE_DBG 115200
#define UART_BAUD_RATE_1M  1000000
#define UART_BAUD_RATE_2M  2000000
#define UART_BAUD_RATE_3M  3000000
#define UART_BAUD_RATE_4M  4000000
#define UART_BAUD_RATE_5M  5000000
#define UART_BAUD_RATE_6M  6000000
#define UART_BAUD_RATE_7M  7000000
#define UART_BAUD_RATE_8M  8000000
#define UART_BAUD_RATE_10M 10000000
#if defined(CONFIG_SUPPORT_Hi3798MV320)
#define HCC_UART_FW_BAUDRATE UART_BAUD_RATE_4M
#elif defined(CONFIG_SUPPORT_Hi3518V300)
#define HCC_UART_FW_BAUDRATE UART_BAUD_RATE_1M
#else
#define HCC_UART_FW_BAUDRATE UART_BAUD_RATE_10M
#endif
/*
 * struct uart_dn_plat_s - the PM internal data, embedded as the
 *  platform's drv data. One for each PS device in the system.
 */
typedef struct hcc_uart_priv_data_t {
    td_u8 dev_name[UART_DEV_NAME_LEN];
    td_u32 baud_rate;
    td_u32 uart_index;
    td_u32 flow_cntrl;
    td_u32 ldisc_num;

    // TX
    osal_mutex tx_lock;
    // RX
    osal_spinlock rx_lock;
    osal_wait rx_wait;
    osal_wait tx_wait;
    td_u8 *rx_raw_buf;      // 本模块分配的8K rx buffer
    td_u8 *rx_decode_buf;   // 指向数据解析的buffer, 可能为tty驱动buffer或者rx_raw_buf
    td_u32 rx_max_len;      // 8K
    td_u32 rx_cur_len;      // RX当前累计长度
    td_u32 rx_decode_len;   // 某一数据类型累计解析数据长度
    td_u32 rx_decode_total;  // 总的数据解析长度
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    td_u8 ldisc_install;
    struct completion ldisc_installed;
    struct tty_struct *tty_handler;
#endif
} hcc_uart_priv_data;

/* 函数声明 */
td_u32 tty_read_timeout_get(td_void);
td_void tty_read_timeout_set(td_u32 timeout_ms);
td_s32 ps_write_tty(const td_u8 *data, td_u32 count, td_bool hcc_flag);
td_s32 ps_read_tty(td_u8 *data, td_s32 len);
void hcc_uart_set_transfer_mode(td_u8 mode);
td_void hcc_uart_tx_thread_wakeup(td_void);
hcc_uart_priv_data *hcc_get_uart_priv_data(td_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
#endif // __HCC_UART_HOST_H__

