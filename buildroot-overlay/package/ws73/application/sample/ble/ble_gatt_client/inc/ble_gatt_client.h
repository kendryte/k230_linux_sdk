/*
 * Copyright (c) @CompanyNameMagicTag. 2022. All rights reserved.
 *
 * Description: BT HID Service Server module.
 */

/**
 * @defgroup bluetooth_bts_hid_server HID SERVER API
 * @ingroup
 * @{
 */
#ifndef BLE_GATT_CLIENT_H
#define BLE_GATT_CLIENT_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define BYTE_LEN_128 128

#define SUPPORT_GATT_C FALSE
#define SPEED_TEST_SEND FALSE

#if SPEED_TEST_SEND
#define PACKAGE_SIZE 200
#define THOUSAND 1000
#define GAP_MAX_TX_OCTETS 250
#define GAP_MAX_TX_TIME 2000
#define MS_100 100000

typedef enum {
    SEND_STATE_IDLE = 0,
    SEND_STATE_SENDING,
    SEND_STATE_EXIT,
} sle_send_data_state_t;
#endif

#if SUPPORT_GATT_C
#include "bts_le_gap.h"
#endif

#include "errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Use this funtion to init gatt client.
 * @par Description:init gatt client.
 * @attention  NULL
 * @param  NULL
 * @retval error code.
 * @else
 * @brief  初始化gatt 客户端。
 * @par 说明:初始化gatt 客户端。
 * @attention  NULL
 * @param  NULL
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t ble_gatt_client_init(void);

/**
 * @if Eng
 * @brief  BLE gatt client deinit.
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    Excute successfully
 * @retval BT_STATUS_FAIL       Execute fail
 * @par Dependency:
 * @li bts_def.h
 * @else
 * @brief  反初始化gatt 客户端。
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    执行成功
 * @retval BT_STATUS_FAIL       执行失败
 * @par 依赖:
 * @li bts_def.h
 * @endif
 */
errcode_t ble_gatt_client_deinit(void);

#if SUPPORT_GATT_C
/**
 * @if Eng
 * @brief  BLE gatt client connect server.
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    Excute successfully
 * @retval BT_STATUS_FAIL       Execute fail
 * @par Dependency:
 * @li bts_def.h
 * @else
 * @brief  连接远端BLE设备。
 * @attention  NULL
 * @par 依赖:
 * @li bts_def.h
 * @endif
 */
static void ble_gatt_client_connect_device(gap_scan_result_data_t *scan_result_data);

/**
 * @if Eng
 * @brief  send data to peer device by handle.
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    Excute successfully
 * @retval BT_STATUS_FAIL       Execute fail
 * @par Dependency:
 * @li bts_def.h
 * @else
 * @brief  通过handle 发送数据给对端。
 * @attention  NULL
 * @retval BT_STATUS_SUCCESS    执行成功
 * @retval BT_STATUS_FAIL       执行失败
 * @par 依赖:
 * @li bts_def.h
 * @endif
 */
static errcode_t ble_gatt_client_send_report(uint16_t handle, uint8_t *data, uint16_t len);
#endif
/**
 * @}
 */
#ifdef __cplusplus
}
#endif
#endif
