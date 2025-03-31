/*
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 * Description: BTS GATT SERVER module.
 */

/**
 * @defgroup bluetooth_bts_gatt_server BTS GATT SERVER API
 * @ingroup  bluetooth
 * @{
 */

#ifndef BTS_GATT_SERVER_H
#define BTS_GATT_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "errcode.h"
#include "bts_def.h"
#include "bts_gatt_stru.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief Struct of add characteristic information.
 * @else
 * @brief 添加特征信息。
 * @endif
 */
typedef struct {
    bt_uuid_t chara_uuid;   /*!< @if Eng UUID of GATT characteristic.
                                 @else   GATT 特征 UUID。 @endif */
    uint8_t permissions;    /*!< @if Eng Characteristic permissions, { @ref gatt_attribute_permission_t }.
                                 @else   特征权限， { @ref gatt_attribute_permission_t }。 @endif */
    uint8_t properties;    /*!< @if Eng Characteristic properties, { @ref gatt_characteristic_property_t }.
                                 @else   特征特性， { @ref gatt_characteristic_property_t } 。 @endif */
    uint16_t value_len;     /*!< @if Eng Length of reponse data.
                                 @else   响应的数据长度。 @endif */
    uint8_t *value;         /*!< @if Eng Reponse data.
                                 @else   响应的数据。 @endif */
} gatts_add_chara_info_t;

/**
 * @if Eng
 * @brief Struct of add characteristic descriptor information.
 * @else
 * @brief 添加特征描述符信息。
 * @endif
 */
typedef struct {
    bt_uuid_t desc_uuid;   /*!< @if Eng UUID of GATT descriptor.
                                @else   GATT 描述符 UUID。 @endif */
    uint8_t permissions;   /*!< @if Eng Descriptor permissions, { @ref gatt_attribute_permission_t }.
                                @else   特征权限， { @ref gatt_attribute_permission_t }。 @endif */
    uint16_t value_len;    /*!< @if Eng Length of reponse data.
                                @else   响应的数据长度。 @endif */
    uint8_t *value;        /*!< @if Eng Reponse data.
                                @else   响应的数据。 @endif */
} gatts_add_desc_info_t;

/**
 * @if Eng
 * @brief Struct of read request information.
 * @else
 * @brief 读请求信息。
 * @endif
 */
typedef struct {
    uint16_t request_id;  /*!< @if Eng Request id.
                               @else   请求id。 @endif */
    uint16_t handle;      /*!< @if Eng Attribute handle of the read request.
                               @else   请求读的属性句柄。 @endif */
    uint16_t offset;      /*!< @if Eng Offset of the read request in bytes.
                               @else   请求读的字节偏移。 @endif */
    bool need_rsp;        /*!< @if Eng Whether response is needed.
                               @else   是否需要发送响应。 @endif */
    bool need_authorize;  /*!< @if Eng Whether authorization is needed.
                               @else   是否需要授权。 @endif */
    bool is_long;         /*!< @if Eng Whether request is Long Read.
                               @else   请求是否是读长特征。 @endif */
} gatts_req_read_cb_t;

/**
 * @if Eng
 * @brief Struct of write request information.
 * @else
 * @brief 写请求信息。
 * @endif
 */
typedef struct {
    uint16_t request_id;  /*!< @if Eng Request id.
                               @else   请求id。 @endif */
    uint16_t handle;      /*!< @if Eng Attribute handle of the write request.
                               @else   请求写的属性句柄。 @endif */
    uint16_t offset;      /*!< @if Eng Offset of the write request in bytes.
                               @else   请求写的字节偏移。 @endif */
    bool need_rsp;        /*!< @if Eng Whether response is needed.
                               @else   是否需要发送响应。 @endif */
    bool need_authorize;  /*!< @if Eng Whether authorization is needed.
                               @else   是否需要授权。 @endif */
    bool is_prep;         /*!< @if Eng Whether request is Prepare Write.
                               @else   请求是否是准备写。 @endif */
    uint16_t length;      /*!< @if Eng Length of write request data.
                               @else   请求写的数据长度。 @endif */
    uint8_t *value;       /*!< @if Eng Write request data.
                               @else   请求写的数据。 @endif */
} gatts_req_write_cb_t;

/**
 * @if Eng
 * @brief Struct of send response information.
 * @else
 * @brief 发送响应信息。
 * @endif
 */
typedef struct {
    uint16_t request_id; /*!< @if Eng Request ID.
                              @else   请求 ID。 @endif */
    uint8_t status;      /*!< @if Eng Status code of read/write, { @ref gatt_status_t }.
                              @else   读写结果的状态， { @ref gatt_status_t }。 @endif */
    uint16_t offset;     /*!< @if Eng Offset.
                              @else   属偏移。 @endif */
    uint16_t value_len;  /*!< @if Eng Length of reponse data.
                              @else   响应的数据长度。 @endif */
    uint8_t *value;      /*!< @if Eng Reponse data.
                              @else   响应的数据。 @endif */
} gatts_send_rsp_t;

/**
 * @if Eng
 * @brief Struct of send notification/indication information.
 * @else
 * @brief 发送通知/指示信息。
 * @endif
 */
typedef struct {
    uint16_t attr_handle; /*!< @if Eng Attribute handle.
                               @else   属性句柄。 @endif */
    uint16_t value_len;   /*!< @if Eng Length of notification/indication data.
                               @else   通知/指示数据长度。 @endif */
    uint8_t *value;       /*!< @if Eng Notification/indication data.
                               @else   发送的通知/指示数据。 @endif */
} gatts_ntf_ind_t;

/**
 * @if Eng
 * @brief Struct of send notification/indication by uuid information.
 * @else
 * @brief 通过uuid发送通知/指示信息。
 * @endif
 */
typedef struct {
    bt_uuid_t chara_uuid;  /*!< @if Eng Characteristic UUID.
                                @else   特征UUID。 @endif */
    uint16_t start_handle; /*!< @if Eng start handle.
                                @else   起始句柄。 @endif */
    uint16_t end_handle;   /*!< @if Eng end handle.
                                @else   结束句柄。 @endif */
    uint16_t value_len;    /*!< @if Eng Length of notification/indication data.
                                @else   通知/指示数据长度。 @endif */
    uint8_t *value;        /*!< @if Eng Notification/indication data.
                                @else   发送的通知/指示数据。 @endif */
} gatts_ntf_ind_by_uuid_t;

/**
 * @if Eng
 * @brief Struct of add character callback.
 * @else
 * @brief 添加特征回调信息。
 * @endif
 */
typedef struct {
    uint16_t handle;         /*!< @if Eng decl handle.
                                  @else   特征句柄。 @endif */
    uint16_t value_handle;   /*!< @if Eng value handle.
                                  @else   特征值句柄。 @endif */
} gatts_add_character_result_t;

/**
 * @if Eng
 * @brief Callback invoked when service add.
 * @par Callback invoked when service add.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id server ID.
 * @param  [in]  uuid      service uuid.
 * @param  [in]  handle    service attribute handle.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  服务注册的回调函数。
 * @par    服务注册的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id 服务端 ID。
 * @param  [in]  uuid      服务uuid。
 * @param  [in]  handle    服务属性句柄。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_add_service_callback)(uint8_t server_id, bt_uuid_t *uuid, uint16_t handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when characteristic add.
 * @par Callback invoked when characteristic add.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id      server ID.
 * @param  [in]  uuid           characteristic uuid.
 * @param  [in]  service_handle service attribute handle.
 * @param  [in]  handle         character attribute handle.
 * @param  [in]  status         error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  特征注册的回调函数。
 * @par    特征注册的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id      服务端 ID。
 * @param  [in]  uuid           特征 uuid。
 * @param  [in]  service_handle 服务属性句柄。
 * @param  [in]  handle         特征属性句柄。
 * @param  [in]  status         执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_add_characteristic_callback)(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
    gatts_add_character_result_t *result, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when characteristic descriptor add.
 * @par Callback invoked when characteristic descriptor add.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id      server ID.
 * @param  [in]  uuid           characteristic uuid.
 * @param  [in]  service_handle service attribute handle.
 * @param  [in]  handle         character descriptor attribute handle.
 * @param  [in]  status         error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  特征描述符注册的回调函数。
 * @par    特征描述符注册的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id      服务端 ID。
 * @param  [in]  uuid           特征 uuid。
 * @param  [in]  service_handle 服务属性句柄。
 * @param  [in]  handle         特征描述符属性句柄。
 * @param  [in]  status         执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_add_descriptor_callback)(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
    uint16_t handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when service started.
 * @par Callback invoked when service started.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id server ID.
 * @param  [in]  handle    service attribute handle.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  开始服务的回调函数。
 * @par    开始服务的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id 服务端 ID。
 * @param  [in]  handle    服务属性句柄。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_start_service_callback)(uint8_t server_id, uint16_t handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when service stoped.
 * @par Callback invoked when service stoped.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id server ID.
 * @param  [in]  handle    service attribute handle.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  停止服务的回调函数。
 * @par    停止服务的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id 服务端 ID。
 * @param  [in]  handle    服务属性句柄。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_stop_service_callback)(uint8_t server_id, uint16_t handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when service deleted.
 * @par Callback invoked when service deleted.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id server ID.
 * @param  [in]  handle    service attribute handle.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  删除服务的回调函数。
 * @par    删除服务的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id 服务端 ID。
 * @param  [in]  handle    服务属性句柄。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_delete_service_callback)(uint8_t server_id, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive read request.
 * @par Callback invoked when  receive read request.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id    server ID.
 * @param  [in]  conn_id      connection ID.
 * @param  [in]  read_cb_para read request parameter.
 * @param  [in]  status       error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  收到读请求的回调函数。
 * @par    收到读请求的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id    服务端 ID。
 * @param  [in]  conn_id      连接 ID。
 * @param  [in]  read_cb_para 读请求参数。
 * @param  [in]  status       执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_read_request_callback)(uint8_t server_id, uint16_t conn_id, gatts_req_read_cb_t *read_cb_para,
    errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive write request.
 * @par Callback invoked when  receive write request.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id     server ID.
 * @param  [in]  conn_id       connection ID.
 * @param  [in]  write_cb_para write request parameter.
 * @param  [in]  status        error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  收到写请求的回调函数。
 * @par    收到写请求的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id     服务端 ID。
 * @param  [in]  conn_id       连接 ID。
 * @param  [in]  write_cb_para 写请求参数。
 * @param  [in]  status        执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_write_request_callback)(uint8_t server_id, uint16_t conn_id, gatts_req_write_cb_t *write_cb_para,
    errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when mtu size changed.
 * @par Callback invoked when mtu size changed.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  server_id server ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  mtu_size  mtu size.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  mtu大小改变的回调函数。
 * @par    mtu大小改变的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  server_id 服务端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  mtu_size  mtu 大小。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gatts_mtu_changed_callback)(uint8_t server_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status);

/**
 * @if Eng
 * @brief Struct of GATT server callback function.
 * @else
 * @brief GATT server回调函数接口定义。
 * @endif
 */
typedef struct {
    gatts_add_service_callback add_service_cb;               /*!< @if Eng Service added callback.
                                                                  @else   添加服务回调函数。 @endif */
    gatts_add_characteristic_callback add_characteristic_cb; /*!< @if Eng Characteristc added callback.
                                                                  @else   添加特征回调函数。 @endif */
    gatts_add_descriptor_callback add_descriptor_cb;         /*!< @if Eng Descriptor added callback.
                                                                  @else   添加描述符回调函数。 @endif */
    gatts_start_service_callback start_service_cb;           /*!< @if Eng Service started callback.
                                                                  @else   启动服务回调函数。 @endif */
    gatts_stop_service_callback stop_service_cb;             /*!< @if Eng Service stoped callback.
                                                                  @else   停止服务回调函数。 @endif */
    gatts_delete_service_callback delete_service_cb;         /*!< @if Eng All service deleted callback.
                                                                  @else   删除所有服务回调函数。 @endif */
    gatts_read_request_callback read_request_cb;             /*!< @if Eng Read request received callback.
                                                                  @else   收到远端读请求回调函数。 @endif */
    gatts_write_request_callback write_request_cb;           /*!< @if Eng Write request received callback.
                                                                  @else   收到远端写请求回调函数。 @endif */
    gatts_mtu_changed_callback mtu_changed_cb;               /*!< @if Eng Mtu changed callback.
                                                                  @else   mtu 大小更新回调函数。 @endif */
} gatts_callbacks_t;

/**
 * @if Eng
 * @brief  Register gatt server.
 * @par Description: Register gatt server.
 * @param  [in]  app_uuid  App uuid.
 * @param  [out] server_id Server ID.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  注册gatt服务端。
 * @par Description: 注册gatt服务端。
 * @param  [in]  app_uuid  上层应用uuid。
 * @param  [out] server_id 服务端ID。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_register_server(bt_uuid_t *app_uuid, uint8_t *server_id);

/**
 * @if Eng
 * @brief  Unregister gatt server.
 * @par Description: Unregister gatt server.
 * @param  [in] server_id Server ID.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  注销gatt服务端。
 * @par Description: 注销gatt服务端。
 * @param  [in] server_id 服务端ID。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_unregister_server(uint8_t server_id);

/**
 * @if Eng
 * @brief  Add a gatt service.
 * @par Description: Add a gatt service.
 * @param  [in] server_id Server ID.
 * @param  [in] service_uuid Service uuid.
 * @param  [in] is_primary is primary service or not.
 * @retval error code, the service handle will be returned in { @ref gatts_add_service_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  添加一个gatt服务。
 * @par Description: 添加一个gatt服务。
 * @param  [in] server_id 服务端 ID。
 * @param  [in] service_uuid 服务uuid。
 * @param  [in] is_primary 是否是首要服务。
 * @retval 执行结果错误码，服务句柄将在 { @ref gatts_add_service_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_add_service(uint8_t server_id, bt_uuid_t *service_uuid, bool is_primary);

/**
 * @if Eng
 * @brief  Add a gatt characteristic.
 * @par Description: Add a gatt characteristic.
 * @param  [in] server_id Server ID.
 * @param  [in] service_handle Service handle.
 * @param  [in] character Characteristic.
 * @retval error code, the character handle will be returned in { @ref gatts_add_characteristic_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  添加一个gatt特征。
 * @par Description: 添加一个gatt特征。
 * @param  [in] server_id 服务端 ID。
 * @param  [in] service_handle 服务句柄。
 * @param  [in] character GATT 特征。
 * @retval 执行结果错误码，特征句柄将在 { @ref gatts_add_characteristic_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_add_characteristic(uint8_t server_id, uint16_t service_handle, gatts_add_chara_info_t *character);

/**
 * @if Eng
 * @brief  Add a gatt characteristic descriptor.
 * @par Description: Add a gatt characteristic descriptor.
 * @param  [in] server_id Server ID.
 * @param  [in] service_handle Service handle.
 * @param  [in] descriptor Characteristic descriptor.
 * @retval error code, the descriptor handle will be returned in { @ref gatts_add_descriptor_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  添加一个gatt特征描述符。
 * @par Description: 添加一个gatt特征描述符。
 * @param  [in] server_id 服务端 ID。
 * @param  [in] service_handle 服务句柄。
 * @param  [in] descriptor GATT 特征描述符。
 * @retval 执行结果错误码，特征句柄将在 { @ref gatts_add_descriptor_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_add_descriptor(uint8_t server_id, uint16_t service_handle, gatts_add_desc_info_t *descriptor);

/**
 * @if Eng
 * @brief  Add a gatt service.
 * @par Description: Add a gatt service.
 * @param  [in]  server_id Server ID.
 * @param  [in]  service_uuid Service uuid.
 * @param  [in]  is_primary is primary service or not.
 * @param  [out] handle service handle.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  添加一个gatt服务。
 * @par Description: 添加一个gatt服务。
 * @param  [in]  server_id    服务端 ID。
 * @param  [in]  service_uuid 服务uuid。
 * @param  [in]  is_primary   是否是首要服务。
 * @param  [out] handle       服务句柄。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_add_service_sync(uint8_t server_id, bt_uuid_t *service_uuid, bool is_primary, uint16_t *handle);

/**
 * @if Eng
 * @brief  Add a gatt characteristic.
 * @par Description: Add a gatt characteristic.
 * @param  [in]  server_id      Server ID.
 * @param  [in]  service_handle Service handle.
 * @param  [in]  character      Characteristic.
 * @param  [out] result         Characteristic handle.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  添加一个gatt特征。
 * @par Description: 添加一个gatt特征。
 * @param  [in]  server_id      服务端 ID。
 * @param  [in]  service_handle 服务句柄。
 * @param  [in]  character      GATT 特征。
 * @param  [out] result         特征句柄。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_add_characteristic_sync(uint8_t server_id, uint16_t service_handle, gatts_add_chara_info_t *character,
    gatts_add_character_result_t *result);

/**
 * @if Eng
 * @brief  Add a gatt characteristic descriptor.
 * @par Description: Add a gatt characteristic descriptor.
 * @param  [in]  server_id      Server ID.
 * @param  [in]  service_handle Service handle.
 * @param  [in]  descriptor     Characteristic descriptor.
 * @param  [out] handle         Characteristic descriptor handle.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  添加一个gatt特征描述符。
 * @par Description: 添加一个gatt特征描述符。
 * @param  [in]  server_id      服务端 ID。
 * @param  [in]  service_handle 服务句柄。
 * @param  [in]  descriptor     特征描述符。
 * @param  [out] handle         特征描述符句柄。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_add_descriptor_sync(uint8_t server_id, uint16_t service_handle, gatts_add_desc_info_t *descriptor,
    uint16_t *handle);

/**
 * @if Eng
 * @brief  Start a GATT service.
 * @par Description: Start a GATT service.
 * @param  [in] server_id      server ID.
 * @param  [in] service_handle service handle.
 * @retval error code, the service start result will be returned at { @ref gatts_start_service_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  开始一个GATT服务。
 * @par Description: 开始一个GATT服务。
 * @param  [in] server_id      服务端 ID。
 * @param  [in] service_handle 服务句柄。
 * @retval 执行结果错误码，服务开启结果将在 { @ref gatts_start_service_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_start_service(uint8_t server_id, uint16_t service_handle);

/**
 * @if Eng
 * @brief  Stop a GATT service.
 * @par Description: Stop a GATT service.
 * @param  [in] server_id      server ID.
 * @param  [in] service_handle service handle.
 * @retval error code, the service stop result will be returned at { @ref gatts_stop_service_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  停止一个GATT服务。
 * @par Description: 停止一个GATT服务。
 * @param  [in] server_id      服务端 ID。
 * @param  [in] service_handle 服务句柄。
 * @retval 执行结果错误码，服务停止结果将在 { @ref gatts_stop_service_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_stop_service(uint8_t server_id, uint16_t service_handle);

/**
 * @if Eng
 * @brief  Delete a GATT service.
 * @par Description: Delete a GATT service.
 * @param  [in] server_id      server ID.
 * @param  [in] service_handle service handle.
 * @retval error code, the service delete result will be returned at { @ref gatts_delete_service_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  删除一个GATT服务。
 * @par Description: 删除一个GATT服务。
 * @param  [in] server_id      服务端 ID。
 * @param  [in] service_handle 服务句柄。
 * @retval 执行结果错误码，服务删除结果将在 { @ref gatts_delete_service_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_delete_service(uint8_t server_id, uint16_t service_handle);

/**
 * @if Eng
 * @brief  Delete all GATT service.
 * @par Description: Delete all GATT service.
 * @param  [in] server_id      server ID.
 * @retval error code, the service delete result will be returned at { @ref gatts_delete_service_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  删除所有GATT服务。
 * @par Description: 删除所有GATT服务。
 * @param  [in] server_id      服务端 ID。
 * @retval 执行结果错误码，服务删除结果将在 { @ref gatts_delete_service_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_delete_all_services(uint8_t server_id);

/**
 * @if Eng
 * @brief  Send response when receive the request need to response by user.
 * @par Description: Send response when receive the request need to response by user
                     { @ref gatts_read_request_callback } { @ref gatts_write_request_callback }.
 * @param  [in] server_id server ID.
 * @param  [in] conn_id   connection ID.
 * @param  [in] param     response parameter.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  当收到需要用户回复响应的请求时发送响应。
 * @par Description: 当收到需要用户回复响应的请求时发送响应
                     { @ref gatts_read_request_callback } { @ref gatts_write_request_callback }。
 * @param  [in] server_id 服务端 ID.
 * @param  [in] conn_id   连接ID。
 * @param  [in] param     响应参数.
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_send_response(uint8_t server_id, uint16_t conn_id, gatts_send_rsp_t *param);

/**
 * @if Eng
 * @brief  Send indication or notification to remote device.
 * @par Description: Send indication or notification to remote device,
                     send status depend on character descriptor: client characteristic configuration.
 * @param  [in] server_id server ID.
 * @param  [in] conn_id   connection ID.
 * @param  [in] param     notify/indicate parameter.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  向对端发送通知或指示。
 * @par Description: 向对端发送通知或指示，具体发送状态取决于特征描述符：客户端特征配置。
 * @param  [in] server_id 服务端 ID。
 * @param  [in] conn_id   连接ID。
 * @param  [in] param     通知或指示参数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_notify_indicate(uint8_t server_id, uint16_t conn_id, gatts_ntf_ind_t *param);

/**
 * @if Eng
 * @brief  Send indication or notification to remote device by uuid.
 * @par Description: Send indication or notification to remote device by uuid,
                     send status depend on client characteristic configuration descriptor value,
                     value = 0x0000: notification and indication not allowed,
                     value = 0x0001: notification allowed,
                     value = 0x0002: indication allowed.
 * @param  [in] server_id server ID.
 * @param  [in] conn_id   connection ID.
 * @param  [in] param     notify/indicate parameter.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  向对端发送通知或指示。
 * @par Description: 通过uuid向对端发送通知或指示，具体发送状态取决于客户端特征配置描述符值，
                     value = 0x0000：不允许通知和指示，
                     value = 0x0001：允许通知，
                     value = 0x0002：允许指示。
 * @param  [in] server_id 服务端 ID。
 * @param  [in] conn_id   连接ID。
 * @param  [in] param     通知或指示参数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_notify_indicate_by_uuid(uint8_t server_id, uint16_t conn_id, gatts_ntf_ind_by_uuid_t *param);

/**
 * @if Eng
 * @brief  Set server rx mtu before connected.
 * @par Description: Set server rx mtu before connected.
 * @param  [in] server_id server ID.
 * @param  [in] mtu_size  server rx mtu size.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  在连接之前设置服务端接收mtu。
 * @par Description: 在连接之前设置服务端接收mtu。
 * @param  [in] server_id 服务端ID。
 * @param  [in] mtu_size 服务端接收mtu。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_set_mtu_size(uint8_t server_id, uint16_t mtu_size);

/**
 * @if Eng
 * @brief  Register callbacks.
 * @par Description: Register callbacks.
 * @param  [in] func callback function.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t

 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  注册回调函数。
 * @par Description: 注册回调函数。
 * @param  [in] func 回调函数
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gatts_register_callbacks(gatts_callbacks_t *func);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif

