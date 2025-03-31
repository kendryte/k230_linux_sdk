/**
 * Copyright (c) @CompanyNameMagicTag 2024. All rights reserved.
 *
 * Description: BTS device manager module.
 */

/**
 * @defgroup bluetooth_bts_device BTS device manager API
 * @ingroup  bluetooth
 * @{
 */
#ifndef BTS_DEVICE_MANAGER_H
#define BTS_DEVICE_MANAGER_H

#include <stdint.h>
#include "errcode.h"
#include "bts_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief Enum of BT power status.
 * @else
 * @brief BT上电状态枚举.
 * @endif
 */
typedef enum {
    BT_POWER_ON_SUCCESS,       /*!< @if Eng BT power on success.
                                    @else   BT上电成功. @endif */
    BT_POWER_ON_FAIL,          /*!< @if Eng BT power on fail.
                                    @else   BT上电失败. @endif */
} bts_power_on_status_t;

/**
 * @if Eng
 * @brief Enum of BT enable/disable status.
 * @else
 * @brief BT使能/去使能状态枚举.
 * @endif
 */
typedef enum {
    BT_ENABLE_DISABLE_SUCCESS,       /*!< @if Eng BT enable/disable success.
                                          @else   BT使能/去使能成功. @endif */
    BT_ENABLE_DISABLE_FAIL,          /*!< @if Eng BT enable/disable fail.
                                          @else   BT使能/去使能失败. @endif */
} bts_enable_disable_status_t;

/**
 * @if Eng
 * @brief Callback invoked when ble enabled.
 * @par When registered, this callback notifies the upper layer when device power on.
 * @attention 1. This function is called in bts context,should not be blocked or do long time waiting.
 * @param  [in] status device power on status.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_def.h
 * @see bts_dev_manager_callbacks_t
 * @else
 * @brief  设备上电回调函数。
 * @par    注册该回调函数之后，设备上电后上报启动结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @param  [in] status 设备上电状态
 * @retval 无返回值。
 * @par 依赖:
 * @li  bts_def.h
 * @see bts_dev_manager_callbacks_t
 * @endif
 */
typedef void (*bts_power_on_callback)(uint8_t status);

/**
 * @if Eng
 * @brief Callback invoked when ble enabled.
 * @par When registered, this callback notifies the upper layer when ble enabled.
 * @attention 1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in] status enable ble status, { @ref bts_enable_disable_status_t }.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_def.h
 * @see bts_dev_manager_callbacks_t
 * @else
 * @brief  ble协议栈启动回调函数。
 * @par    注册该回调函数之后，BTS在ble协议栈启动后上报启动结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in] status 使能ble状态, { @ref bts_enable_disable_status_t }.
 * @retval 无返回值。
 * @par 依赖:
 * @li  bts_def.h
 * @see bts_dev_manager_callbacks_t
 * @endif
 */
typedef void (*bts_ble_enable_callback)(uint8_t status);

/**
 * @if Eng
 * @brief Callback invoked when ble disabled.
 * @par When registered, this callback notifies the upper layer when ble disabled.
 * @attention 1. This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in] status disable ble status, { @ref bts_enable_disable_status_t }.
 * @retval void no return value.
 * @par Dependency:
 * @li  bts_def.h
 * @see bts_dev_manager_callbacks_t
 * @else
 * @brief  ble协议栈关闭回调函数。
 * @par    注册该回调函数之后，BTS在ble协议栈关闭后上报关闭结果给上层。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in] status 去使能ble状态, { @ref bts_enable_disable_status_t }.
 * @retval 无返回值。
 * @par 依赖:
 * @li  bts_def.h
 * @see bts_dev_manager_callbacks_t
 * @endif
 */
typedef void (*bts_ble_disable_callback)(uint8_t status);

/**
 * @if Eng
 * @brief Struct of BT device manager callback function.
 * @else
 * @brief BT设备管理回调函数定义。
 * @endif
 */
typedef struct {
    bts_power_on_callback power_on_cb;                 /*!< @if Eng Device power on callback
                                                            @else   设备上电回调函数 @endif */
    bts_ble_enable_callback ble_enable_cb;             /*!< @if Eng Ble enable callback
                                                            @else   BLE启动回调函数 @endif */
    bts_ble_disable_callback ble_disable_cb;           /*!< @if Eng Ble disable callback
                                                            @else   BLE关闭回调函数 @endif */
} bts_dev_manager_callbacks_t;

/**
 * @if Eng
 * @brief Use this funtion to enable BLE.
 * @par  Use this funtion to enable BLE.
 * @retval error code, ble enable result will be returned at { @ref bts_ble_enable_callback }.
 * @par Dependency:
 * @li  bts_def.h
 * @else
 * @brief  使能BLE协议栈。
 * @par    使能BLE协议栈。
 * @retval 执行结果错误码，执行结果将在 { @ref bts_ble_enable_callback }中返回。
 * @par 依赖:
 * @li  bts_def.h
 * @endif
 */
errcode_t enable_ble(void);

/**
 * @if Eng
 * @brief Use this funtion to disable BLE stack.
 * @par  Use this funtion to disable BLE stack.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Dependency:
 * @li  bts_def.h
 * @else
 * @brief  去使能BLE协议栈。
 * @par    去使能BLE协议栈。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖:
 * @li  bts_def.h
 * @endif
 */
errcode_t disable_ble(void);

/**
 * @if Eng
 * @brief  Registering the Path for Storing BLE Device Persistence Information.
 * @par    Registering the Path for Storing BLE Device Persistence Information.
 * @attention Applicable to the Linux system.
 * @param [in] path file store path, including the endding 0.
 * @retval error code.
 * @else
 * @brief  注册BLE设备持久化信息存储路径。
 * @par    注册BLE设备持久化信息存储路径。
 * @attention 适用于linux系统。
 * @param [in] path 文件持久化路径，包含结束符'\0'。
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t bts_dev_manager_register_file_path(const uint8_t *path);

/**
 * @if Eng
 * @brief Use this funtion to register callback function of device manager
 * @par   Use this funtion to register callback function of device manager
 * @attention NULL
 * @param  [in] func bts_dev_manager_callbacks_t *, a pointer to the callback functions.
 * @retval error code.
 * @par Dependency:
 * @li  bts_def.h
 * @else
 * @brief  注册BT device manager回调函数。
 * @par    注册BT device manager回调函数。
 * @attention 无
 * @param  [in] func 指向回调函数接口定义的指针
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @endif
 */
errcode_t bts_dev_manager_register_callbacks(bts_dev_manager_callbacks_t *func);

#ifdef __cplusplus
}
#endif
#endif /* BTS_DEVICE_MANAGER_H */
/**
 * @}
 */
