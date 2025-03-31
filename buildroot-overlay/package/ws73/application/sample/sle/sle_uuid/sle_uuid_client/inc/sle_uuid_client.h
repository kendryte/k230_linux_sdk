/*
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE private service register sample of client.
 */

/**
 * @defgroup SLE UUID CLIENT API
 * @ingroup
 * @{
 */

#ifndef SLE_CLIENT_ADV_H
#define SLE_CLIENT_ADV_H

#define BYTE_LEN_128 128

typedef enum {
    STATE_IDLE          = 0,
    STATE_SEEKING       = 1,
    STATE_CONNECTING    = 2,
    STATE_PAIRING       = 3,
    STATE_EXCHANING     = 4,
    STATE_DISCOVERING   = 5,
    STATE_TRANSFER      = 6,
    STATE_MAX           = 7,
} sle_state_t;

typedef enum {
    EVENT_SCAN_TRIG         = 0x00,
    EVENT_CONNECT_TRIG      = 0x01,
    EVENT_SCAN_CMPL         = 0x02,
    EVENT_SCAN_FAIL         = 0x03,
    EVENT_CONNECT_CMPL      = 0x04,
    EVENT_CONNECT_FAIL      = 0x05,
    EVENT_PAIR_CMPL         = 0x06,
    EVENT_PAIR_FAIL         = 0x07,
    EVENT_EXCHANGE_CMPL     = 0x08,
    EVENT_EXCHANGE_FAIL     = 0x09,
    EVENT_DISCOVER_CMPL     = 0x0a,
    EVENT_DISCOVER_FAIL     = 0x0b,
    EVENT_STATE_CHANGE      = 0x0c,
    EVENT_DISCONNECT        = 0x0d,
    EVENT_MAX               = 0x0e,
} sle_event_t;

typedef enum {
    SEND_STATE_IDLE = 0,
    SEND_STATE_SENDING,
    SEND_STATE_EXIT,
} sle_send_data_state_t;

/**
 * @if Eng
 * @brief  sle uuid client init.
 * @attention  NULL
 * @retval ERRCODE_SLE_SUCCESS    Excute successfully
 * @retval ERRCODE_SLE_FAIL       Execute fail
 * @par Dependency:
 * @li NULL
 * @else
 * @brief  sle uuid客户端初始化。
 * @attention  NULL
 * @retval ERRCODE_SLE_SUCCESS    执行成功
 * @retval ERRCODE_SLE_FAIL       执行失败
 * @par 依赖:
 * @li NULL
 * @endif
 */
void sle_client_init(void);

/**
 * @if Eng
 * @brief  sle start scan.
 * @attention  NULL
 * @retval ERRCODE_SLE_SUCCESS    Excute successfully
 * @retval ERRCODE_SLE_FAIL       Execute fail
 * @par Dependency:
 * @li NULL
 * @else
 * @brief  sle启动扫描。
 * @attention  NULL
 * @retval ERRCODE_SLE_SUCCESS    执行成功
 * @retval ERRCODE_SLE_FAIL       执行失败
 * @par 依赖:
 * @li NULL
 * @endif
 */
void sle_start_scan(void);

void sle_stop_scan(void);

void sle_comm_deinit();
void sle_recv_ack(void);
int sle_write_msgq(uint8_t *data, uint8_t len);
int sle_change_phy_2M(uint16_t handle);

#endif