/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd.. 2022. All rights reserved.
 *
 * Description: BLE UUID Server module.
 */

/**
 * @defgroup bluetooth_bts_hid_server HID SERVER API
 * @ingroup
 * @{
 */
#ifndef BLE_UUID_SERVER_H
#define BLE_UUID_SERVER_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define BYTE_LEN_128 128

#include "bts_def.h"
#include "errcode.h"

/* Service UUID */
#define BLE_UUID_UUID_SERVER_SERVICE                 0xABCD
/* Characteristic UUID */
#define BLE_UUID_UUID_SERVER_REPORT                  0xCDEF
/* Characteristic UUID */
#define BLE_UUID_CHARACTERISTIC_UUID_TX              0x2A90
/* Characteristic UUID */
#define BLE_UUID_CHARACTERISTIC_UUID_RX              0x2A8A
/* Client Characteristic Configuration UUID */
#define BLE_UUID_CLIENT_CHARACTERISTIC_CONFIGURATION 0x2902
/* Server ID */
#define BLE_UUID_SERVER_ID 1
/* Default Service Num */
#define BLE_SERVICE_NUM 2

/* Characteristic Property */
#define UUID_SERVER_PROPERTIES   (GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY)

#define SPEED_TEST_RECV FALSE

#if SPEED_TEST_RECV
#define HUNDRED 100
#define RECV_PKT_CNT 100
#endif

/**
 * @if Eng
 * @brief  BLE uuid server inir.
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    Excute successfully
 * @retval BT_STATUS_FAIL       Execute fail
 * @par Dependency:
 * @li bts_def.h
 * @else
 * @brief  BLE UUID服务器初始化。
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    执行成功
 * @retval BT_STATUS_FAIL       执行失败
 * @par 依赖:
 * @li bts_def.h
 * @endif
 */
errcode_t ble_uuid_server_init(void);

/**
 * @if Eng
 * @brief  BLE uuid server deinit.
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    Excute successfully
 * @retval BT_STATUS_FAIL       Execute fail
 * @par Dependency:
 * @li bts_def.h
 * @else
 * @brief  BLE UUID服务器反初始化。
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    执行成功
 * @retval BT_STATUS_FAIL       执行失败
 * @par 依赖:
 * @li bts_def.h
 * @endif
 */
errcode_t ble_uuid_server_deinit(void);
 
/**
 * @if Eng
 * @brief  send data to peer device by handle on uuid server.
 * @attention  NULL
 * @param  [in]  value  send value.
 * @param  [in]  len    Length of send value。
 * @retval BT_STATUS_SUCCESS    Excute successfully
 * @retval BT_STATUS_FAIL       Execute fail
 * @par Dependency:
 * @li bts_def.h
 * @else
 * @brief  通过uuid server 发送数据给对端。
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    执行成功
 * @retval BT_STATUS_FAIL       执行失败
 * @par 依赖:
 * @li bts_def.h
 * @endif
 */
errcode_t ble_uuid_server_send_report_by_handle(uint16_t attr_handle, uint8_t *data, uint8_t len);

#endif

