/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 *
 * Description: SLE HID HOST INTERFACE.
 * Author: @CompanyNameTag
 * History:
 * 2023-11-17, Create file.
 */

/**
 * @defgroup sle hid host api
 * @ingroup  SLE
 * @{
 */

#ifndef AAPI_SLE_HH_INTF_H
#define AAPI_SLE_HH_INTF_H

#include "sle_connection_manager.h"
#include "aapi_sle_ssap_client.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Report types */
typedef enum {
    SLE_HH_INPUT_REPORT = 1,
    SLE_HH_OUTPUT_REPORT,
    SLE_HH_FEATURE_REPORT
} sle_hh_report_type_t;

typedef void (*sle_hh_handshake_callback)(uint16_t conn_id, const sle_addr_t *addr, sle_acb_state_t conn_state);

typedef struct {
    sle_connect_state_changed_callback connection_state_cb;
    sle_hh_handshake_callback handshake_cb;
} sle_hh_callbacks_t;

/**
 * @if Eng
 * @brief  SLE Hid host init.
 * @par Description: SLE Hid host init.
 * @param [in] sle_cbk Callback function.
 * @retval error code.
 * @par Depends:
 * @li  sle_connection_manager.h
 * @else
 * @brief  星闪HID Host端初始化接口，注册Host相关回调。
 * @par Description: 星闪HID Host端初始化接口，注册Host相关回调。
 * @param [in] sle_cbk 回调函数.
 * @retval 执行结果错误码
 * @par 依赖：
 * @li  sle_connection_manager.h
 * @endif
 */
errcode_t sle_hh_init(sle_hh_callbacks_t *sle_cbk);

/**
 * @if Eng
 * @brief  Send connect request to remote hid device.
 * @par Description: Send connect request to hid remote device.
 * @param [in] addr address.
 * @retval error code, connection state change result will be returned at { @ref sle_connect_state_changed_callback }.
 * @par Depends:
 * @li sle_connection_manager.h
 * @else
 * @brief  发送连接请求。
 * @par Description: 发送连接请求。
 * @param [in] addr 地址。
 * @retval 执行结果错误码， 连接状态改变结果将在 { @ref sle_connect_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_connection_manager.h
 * @endif
 */
errcode_t sle_hh_connect(const sle_addr_t *addr, aapi_conn_state_t *conn_state);

/**
 * @if Eng
 * @brief  Send disconnect request to remote hid device.
 * @par Description: Send disconnect request to remote hid device.
 * @param [in] addr address.
 * @retval error code, connection state change result will be returned at { @ref sle_connect_state_changed_callback }.
 * @par Depends:
 * @li sle_connection_manager.h
 * @else
 * @brief  发送断开连接请求。
 * @par Description: 发送断开连接请求。
 * @param [in] addr 地址。
 * @retval 执行结果错误码， 连接状态改变结果将在 { @ref sle_connect_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_connection_manager.h
 * @endif
 */
errcode_t sle_hh_disconnect(const sle_addr_t *addr, uint8_t *state);

/**
 * @if Eng
 * @brief  Send set report request to remote hid device.
 * @par Description: Send set report request to remote hid device.
 * @param [in] addr address.
 * @param [in] reportType report type.
 * @param [in] len data length.
 * @param [in] report report data.
 * @retval error code.
 * @par Depends:
 * @li sle_connection_manager.h
 * @else
 * @brief  发送Set report给远端HID设备。
 * @par Description: 发送Set report给远端HID设备。
 * @param [in] addr 地址.
 * @param [in] reportType 报告类型.
 * @param [in] len 报告数据长度.
 * @param [in] report 报告数据.
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_connection_manager.h
 * @endif
 */
errcode_t sle_hh_set_report(const sle_addr_t *addr, sle_hh_report_type_t reportType, uint16_t len, uint8_t *report);

/**
 * @if Eng
 * @brief  SLE Hid host cleanup.
 * @par Description: SLE Hid host cleanup.
 * @param  NULL
 * @retval NULL.
 * @par Depends:
 * @li  sle_connection_manager.h
 * @else
 * @brief  星闪HID Host清理。
 * @par Description: 星闪HID Host清理。
 * @param  无
 * @retval 无.
 * @par 依赖：
 * @li  sle_connection_manager.h
 * @endif
 */
void sle_hh_cleanup(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

/**
 * @}
 */
