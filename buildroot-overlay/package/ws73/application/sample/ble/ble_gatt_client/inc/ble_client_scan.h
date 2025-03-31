/**
 * Copyright (c) @CompanyNameMagicTag 2024. All rights reserved.
 * Description: BLE config client scan.
 */
 
#ifndef BLE_CLIENT_SCAN_H
#define BLE_CLIENT_SCAN_H

#include "errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Use this funtion to start gatt client scanning.
 * @par Description:start gatt client scanning.
 * @attention  NULL
 * @param  NULL
 * @retval error code.
 * @else
 * @brief  启动gatt 客户端的扫描。
 * @par 说明:启动gatt 客户端的扫描。
 * @attention  NULL
 * @param  NULL
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t ble_start_scan(void);

/**
 * @if Eng
 * @brief BLE scan parameters config.
 * @par Description:set BLE scan parameters.
 * @attention  NULL
 * @param  conn_id connection ID
 * @retval error code.
 * @else
 * @brief  BLE扫描参数配置。
 * @par 说明：设置BLE扫描参数。
 * @attention  NULL
 * @param  NULL
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t ble_start_set_scan_parameters(void);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif
#endif
