/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: plat_firmware_uart.c header file
 * Author: @CompanyNameTag
 */

#ifndef __PLAT_FIRMWARE_UART_H__
#define __PLAT_FIRMWARE_UART_H__

#include "osal_types.h"
#include "plat_debug.h"
#include "plat_firmware.h"

#define SOH 0x01 /* 开始字符 */
#define EOT 0x04 /* 发送完成 */
#define ACK 0x06 /* 正确接收应答 */
#define NAK 0x15 /* 校验错误重新发送，通讯开始时用于接收方协商累加校验 */
#define CAN 0x18 /* 结束下载 */

#define MSG_FORM_DRV_G 'G'
#define MSG_FORM_DRV_C 'C'
#define MSG_FORM_DRV_A 'a'
#define MSG_FORM_DRV_N 'n'

/* 以下是发往device命令的关键字 */
#define BAUDRATE_CMD_KEYWORD        "BAUDRATE"
#define READM_CMD_KEYWORD           "READM"
#define WRITEM_CMD_KEYWORD          "WRITEM"

#define VER_EXPECT_VALUE    "BOOTVER001"

#define XMODEM_HEAD_LEN     3
#define XMODEM_DATA_LEN     1024
#define XMODEM_CRC_LEN      2

#define CRC_TABLE_SIZE      256
#define SEND_DATA_MAX_LEN   64
#define RECV_DATA_MAX_LEN   64

typedef struct {
    osal_char sch;      /* 开始字符 */
    osal_u8 num;        /* 包序号 */
    osal_u8 ant;        /* 包序号补码 */
} __attribute__((packed)) xmodem_pkt_header;

typedef struct {
    osal_u8 h;
    osal_u8 l;
} __attribute__((packed)) xmodem_pkt_crc;

typedef struct {
    xmodem_pkt_header header;
    osal_u8 data[XMODEM_DATA_LEN];
    xmodem_pkt_crc crc;
} __attribute__((packed)) xmodem_pkt_t;

osal_s32 uart_download_firmware(osal_void);
osal_s32 uart_cmd_jump(osal_u32 jump_addr);
osal_s32 uart_cmd_read_mem(osal_u32 addr, osal_u32 len);
osal_s32 uart_cmd_write_mem(osal_u32 width, osal_u32 addr, osal_u32 val);
osal_s32 uart_download_firmware(osal_void);
osal_s32 uart_device_detect(osal_void);

#endif /* __PLAT_FIRMWARE_UART_H__ */

