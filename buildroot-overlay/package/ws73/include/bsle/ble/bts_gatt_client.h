/*
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: BTS GATT CLIENT module.
 */


#ifndef BTS_GATT_CLIENT_H
#define BTS_GATT_CLIENT_H

#include <stdint.h>
#include "errcode.h"
#include "bts_def.h"
#include "bts_gatt_stru.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup bluetooth_bts_gatt_client BTS GATT CLIENT API
 * @ingroup  bluetooth
 * @{
 */

/**
 * @if Eng
 * @brief Struct of handle value information.
 * @else
 * @brief 句柄值信息。
 * @endif
 */
typedef struct {
    uint16_t handle;   /*!< @if Eng attribute handle.
                            @else   属性句柄。@endif */
    uint16_t data_len; /*!< @if Eng data length.
                            @else   数据长度。@endif */
    uint8_t *data;     /*!< @if Eng data.
                            @else   数据。@endif */
} gattc_handle_value_t;

/**
 * @if Eng
 * @brief Struct of discovery character parameter.
 * @else
 * @brief 发现特征参数。
 * @endif
 */
typedef struct {
    uint16_t service_handle; /*!< @if Eng service start handle.
                                  @else   服务起始句柄。@endif */
    bt_uuid_t uuid;          /*!< @if Eng characteristic uuid, if uuid length is zero, discover all character in
                                          the service, else filter by uuid.
                                  @else   特征uuid，如果uuid长度为0，发现服务内的所有特征，否则按照uuid过滤。@endif */
} gattc_discovery_character_param_t;

/**
 * @if Eng
 * @brief Struct of discovery service result.
 * @else
 * @brief 发现服务结果。
 * @endif
 */
typedef struct {
    uint16_t start_hdl; /*!< @if Eng service start handle.
                             @else   服务起始句柄。@endif */
    uint16_t end_hdl;   /*!< @if Eng service stop handle.
                             @else   服务结束句柄。@endif */
    bt_uuid_t uuid;     /*!< @if Eng service uuid.
                             @else   服务uuid。@endif */
} gattc_discovery_service_result_t;

/**
 * @if Eng
 * @brief Struct of discovery character result.
 * @else
 * @brief 发现特征结果。
 * @endif
 */
typedef struct {
    bt_uuid_t uuid;          /*!< @if Eng character uuid.
                                  @else   特征uuid。@endif */
    uint16_t declare_handle; /*!< @if Eng character declare handle.
                                  @else   特征声明句柄。@endif */
    uint16_t value_handle;   /*!< @if Eng character value handle.
                                  @else   特征值句柄。@endif */
    uint8_t properties;      /*!< @if Eng character properties { @ref gatt_characteristic_property_t }.
                                  @else   特征特性 { @ref gatt_characteristic_property_t }。@endif */
} gattc_discovery_character_result_t;

/**
 * @if Eng
 * @brief Struct of discovery character descriptor result.
 * @else
 * @brief 发现特征描述符结果。
 * @endif
 */
typedef struct {
    uint16_t descriptor_hdl; /*!< @if Eng character descriptor handle.
                                  @else   特征描述符句柄。@endif */
    bt_uuid_t uuid;          /*!< @if Eng character descriptor uuid.
                                  @else   特征描述符uuid。@endif */
} gattc_discovery_descriptor_result_t;

/**
 * @if Eng
 * @brief Struct of send read request by uuid parameter.
 * @else
 * @brief 向对端发送按照uuid读取请求的参数。
 * @endif
 */
typedef struct {
    uint16_t start_hdl; /*!< @if Eng start handle.
                             @else   起始句柄。@endif */
    uint16_t end_hdl;   /*!< @if Eng end handle.
                             @else   结束句柄。@endif */
    bt_uuid_t uuid;     /*!< @if Eng uuid.
                             @else   uuid。@endif */
} gattc_read_req_by_uuid_param_t;

/**
 * @if Eng
 * @brief Callback invoked when service discovery.
 * @par Callback invoked when service discovery.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  service   service information.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  服务发现的回调函数。
 * @par    服务发现的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  service   发现的服务信息。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_discovery_service_callback)(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_service_result_t *service, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when service discovery completed.
 * @par Callback invoked when service discovery completed.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  uuid      input parameter.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  服务发现完成的回调函数。
 * @par    服务发现完成的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  uuid      入参回传。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_discovery_service_complete_callback)(uint8_t client_id, uint16_t conn_id,
    bt_uuid_t *uuid, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when character discovery.
 * @par Callback invoked when character discovery.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  character character information.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  特征发现的回调函数。
 * @par    特征发现的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  character 发现的特征信息。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_discovery_character_callback)(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_character_result_t *character, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when character discovery completed.
 * @par Callback invoked when character discovery completed.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  param     input parameter.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  特征发现完成的回调函数。
 * @par    特征发现完成的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  param     入参回传。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_discovery_character_complete_callback)(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_character_param_t *param, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when character descriptor discovery.
 * @par Callback invoked when character descriptor discovery.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id  client ID.
 * @param  [in]  conn_id    connection ID.
 * @param  [in]  descriptor character descriptor information.
 * @param  [in]  status     error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  特征描述符发现的回调函数。
 * @par    特征描述符发现的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id  客户端 ID。
 * @param  [in]  conn_id    连接 ID。
 * @param  [in]  descriptor 发现的特征描述符信息。
 * @param  [in]  status     执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_discovery_descriptor_callback)(uint8_t client_id, uint16_t conn_id,
    gattc_discovery_descriptor_result_t *descriptor, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when character descriptor discovery completed.
 * @par Callback invoked when character descriptor discovery completed.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id         client ID.
 * @param  [in]  conn_id           connection ID.
 * @param  [in]  character_handle  input parameter.
 * @param  [in]  status            error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  特征描述符发现完成的回调函数。
 * @par    特征描述符发现完成的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id        客户端 ID。
 * @param  [in]  conn_id          连接 ID。
 * @param  [in]  character_handle 入参回传。
 * @param  [in]  status           执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_discovery_descriptor_complete_callback)(uint8_t client_id, uint16_t conn_id,
    uint16_t character_handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive read response.
 * @par Callback invoked when receive read response.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id   client ID.
 * @param  [in]  conn_id     connection ID.
 * @param  [in]  read_result read result.
 * @param  [in]  status      GATT error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  收到读响应的回调函数。
 * @par    收到读响应的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id   客户端 ID。
 * @param  [in]  conn_id     连接 ID。
 * @param  [in]  read_result 读取结果。
 * @param  [in]  status      GATT错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_read_cfm_callback)(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *read_result,
    gatt_status_t status);

/**
 * @if Eng
 * @brief Callback invoked when read by uuid is completed.
 * @par Callback invoked when read by uuid is completed.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  param     input parameter.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  按照uuid读取完成的回调函数。
 * @par    按照uuid读取完成的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  param     入参回传。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_read_by_uuid_complete_callback)(uint8_t client_id, uint16_t conn_id,
    gattc_read_req_by_uuid_param_t *param, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive write response.
 * @par Callback invoked when receive write response.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  handle    write attribute handle.
 * @param  [in]  status    GATT error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  收到写响应的回调函数。
 * @par    收到写响应的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  handle    请求写的句柄。
 * @param  [in]  status    GATT错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_write_cfm_callback)(uint8_t client_id, uint16_t conn_id, uint16_t handle, gatt_status_t status);

/**
 * @if Eng
 * @brief Callback invoked when mtu changed.
 * @par Callback invoked when mtu changed.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  mtu_size  mtu size.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  mtu改变的回调函数。
 * @par    mtu改变的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  mtu_size  mtu大小。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_mtu_changed_callback)(uint8_t client_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive notification.
 * @par Callback invoked when receive notification.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  data      data.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  收到通知的回调函数。
 * @par    收到通知的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  data      数据。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_notification_callback)(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *data,
    errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive indication.
 * @par Callback invoked when receive indication.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2.The memories of devices are requested and freed by the bts automatically.
 * @param  [in]  client_id client ID.
 * @param  [in]  conn_id   connection ID.
 * @param  [in]  data      data.
 * @param  [in]  status    error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @else
 * @brief  收到指示的回调函数。
 * @par    收到指示的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. devices由bts申请内存，也由bts释放，回调中不应释放。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  data      数据。
 * @param  [in]  status    执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see gap_ble_callbacks_t
 * @endif
 */
typedef void (*gattc_indication_callback)(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *data,
    errcode_t status);

/**
 * @if Eng
 * @brief Struct of GATT client callback function.
 * @else
 * @brief GATT client回调函数接口定义。
 * @endif
 */
typedef struct ble_gattc_callbacks {
    gattc_discovery_service_callback discovery_svc_cb;                  /*!< @if Eng Discovery service callback.
                                                                             @else   发现服务回调函数。 @endif */
    gattc_discovery_service_complete_callback discovery_svc_cmp_cb;     /*!< @if Eng Discovery service complete
                                                                                     callback.
                                                                             @else   发现服务完成回调函数。 @endif */
    gattc_discovery_character_callback discovery_chara_cb;              /*!< @if Eng Discovery character callback.
                                                                             @else   发现特征回调函数。 @endif */
    gattc_discovery_character_complete_callback discovery_chara_cmp_cb; /*!< @if Eng Discovery character complete
                                                                                     callback.
                                                                             @else   发现特征完成回调函数。 @endif */
    gattc_discovery_descriptor_callback discovery_desc_cb;              /*!< @if Eng Discovery character descriptor
                                                                                     callback.
                                                                             @else   发现特征描述符回调函数。 @endif */
    gattc_discovery_descriptor_complete_callback discovery_desc_cmp_cb; /*!< @if Eng Discovery character descriptor
                                                                                     complete callback.
                                                                             @else   发现特征描述符完成回调函数。
                                                                             @endif */
    gattc_read_cfm_callback read_cb;                                    /*!< @if Eng Receive read response callback.
                                                                             @else   收到读响应回调函数。 @endif */
    gattc_read_by_uuid_complete_callback read_cmp_cb;                   /*!< @if Eng Read by uuid complete callback.
                                                                             @else   按照uuid读取完成回调函数。
                                                                             @endif */
    gattc_write_cfm_callback write_cb;                                  /*!< @if Eng Receive write response callback.
                                                                             @else   收到写响应回调函数。 @endif */
    gattc_mtu_changed_callback mtu_changed_cb;                          /*!< @if Eng Mtu changed callback,
                                                                                     the serverId parameter is reserved,
                                                                                     has a fixed value of 0.
                                                                             @else   mtu改变回调函数，serverId作为预留参数，
                                                                                     固定为0。 @endif */
    gattc_notification_callback notification_cb;                        /*!< @if Eng Receive notification callback.
                                                                             @else   收到通知回调函数。 @endif */
    gattc_indication_callback indication_cb;                            /*!< @if Eng Receive indication callback.
                                                                             @else   收到指示回调函数。 @endif */
} gattc_callbacks_t;

/**
 * @if Eng
 * @brief  Register gatt client.
 * @par Description: Register gatt client.
 * @param  [in]  app_uuid  App uuid.
 * @param  [out] client_id Client ID.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  注册gatt客户端。
 * @par Description: 注册gatt客户端。
 * @param  [in]  app_uuid  上层应用uuid。
 * @param  [out] client_id 客户端ID。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_register_client(bt_uuid_t *app_uuid, uint8_t *client_id);

/**
 * @if Eng
 * @brief  Unregister gatt client.
 * @par Description: Unregister gatt client.
 * @param  [in] client_id Client ID.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  注销gatt客户端。
 * @par Description: 注销gatt服务端。
 * @param  [in] client_id 客户端ID。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_unregister_client(uint8_t client_id);

/**
 * @if Eng
 * @brief  Discovery service.
 * @par Description: Discovery service.
 * @param  [in]  client_id Client ID.
 * @param  [in]  conn_id   Connection ID.
 * @param  [in]  uuid      Service uuid, if uuid length is zero, discover all service, else filter by uuid.
 * @retval error code, discovery service result will be returned at { @ref gattc_discovery_service_callback } and
           { @ref gattc_discovery_service_complete_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发现服务。
 * @par Description: 发现服务。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  uuid      服务uuid，如果uuid长度为0，发现所有服务，否则按照uuid过滤。
 * @retval 执行结果错误码, 服务发现结果将在 { @ref gattc_discovery_service_callback }和
           { @ref gattc_discovery_service_complete_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_discovery_service(uint8_t client_id, uint16_t conn_id, bt_uuid_t *uuid);

/**
 * @if Eng
 * @brief  Discovery character.
 * @par Description: Discovery character.
 * @param  [in]  client_id Client ID.
 * @param  [in]  conn_id   Connection ID.
 * @param  [in]  param     Parameter for discover character.
 * @retval error code, discovery character result will be returned at { @ref gattc_discovery_character_callback } and
           { @ref gattc_discovery_character_complete_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发现特征。
 * @par Description: 发现特征。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  param     发现特征参数。
 * @retval 执行结果错误码，特征发现结果将在 { @ref gattc_discovery_character_callback }和
           { @ref gattc_discovery_character_complete_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_discovery_character(uint8_t client_id, uint16_t conn_id, gattc_discovery_character_param_t *param);

/**
 * @if Eng
 * @brief  Discovery character descriptor.
 * @par Description: Discovery character descriptor.
 * @param  [in]  client_id        Client ID.
 * @param  [in]  conn_id          Connection ID.
 * @param  [in]  character_handle Character declaration句柄.
 * @retval error code, discovery character descriptor result will be returned at
           { @ref gattc_discovery_descriptor_callback } and
           { @ref gattc_discovery_descriptor_complete_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发现特征描述符。
 * @par Description: 发现特征描述符。
 * @param  [in]  client_id        客户端 ID。
 * @param  [in]  conn_id          连接 ID。
 * @param  [in]  character_handle 特征声明句柄。
 * @retval 执行结果错误码，特征描述符发现结果将在 { @ref gattc_discovery_descriptor_callback }和
           { @ref gattc_discovery_descriptor_complete_callback } 中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_discovery_descriptor(uint8_t client_id, uint16_t conn_id, uint16_t character_handle);

/**
 * @if Eng
 * @brief  Send read by handle request.
 * @par Description: Send read by handle request.
 * @param  [in]  client_id Client ID.
 * @param  [in]  conn_id   Connection ID.
 * @param  [in]  handle    handle.
 * @retval error code, read result will be returned at { @ref gattc_read_cfm_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发起按照句柄读取请求。
 * @par Description: 发起按照句柄读取请求。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  handle    句柄。
 * @retval 执行结果错误码，读取结果将在 { @ref gattc_read_cfm_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_read_req_by_handle(uint8_t client_id, uint16_t conn_id, uint16_t handle);

/**
 * @if Eng
 * @brief  Send read by uuid request.
 * @par Description: Send read by uuid request.
 * @param  [in]  client_id Client ID.
 * @param  [in]  conn_id   Connection ID.
 * @param  [in]  param     Parameter for read request by uuid.
 * @retval error code, read result will be returned at { @ref gattc_read_cfm_callback } and
           { @ref gattc_read_by_uuid_complete_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发起按照uuid读取请求。
 * @par Description: 发起按照uuid读取请求。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  param     按照uuid读取请求参数。
 * @retval 执行结果错误码，读取结果将在 { @ref gattc_read_cfm_callback }和
           { @ref gattc_read_by_uuid_complete_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_read_req_by_uuid(uint8_t client_id, uint16_t conn_id, gattc_read_req_by_uuid_param_t *param);

/**
 * @if Eng
 * @brief  Send write request.
 * @par Description: Send write request.
 * @param  [in]  client_id Client ID.
 * @param  [in]  conn_id   Connection ID.
 * @param  [in]  param     Parameter for write request.
 * @retval error code, write result will be returned at { @ref gattc_write_cfm_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发起写请求。
 * @par Description: 发起写请求。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  param     写请求参数。
 * @retval 执行结果错误码，写结果将在 { @ref gattc_write_cfm_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_write_req(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *param);

/**
 * @if Eng
 * @brief  Send write command.
 * @par Description: Send write command.
 * @param  [in]  client_id Client ID.
 * @param  [in]  conn_id   Connection ID.
 * @param  [in]  param     Parameter for write command.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发起写命令。
 * @par Description: 发起写命令。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  param     写命令参数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_write_cmd(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *param);

/**
 * @if Eng
 * @brief  Send exchange mtu request.
 * @par Description: Send exchange mtu request.
 * @param  [in]  client_id Client ID.
 * @param  [in]  conn_id   Connection ID.
 * @param  [in]  mtu_size  Client rx mtu.
 * @retval error code, mtu change result will be returned at { @ref gattc_mtu_changed_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  发送交换mtu请求。
 * @par Description: 发送交换mtu请求。
 * @param  [in]  client_id 客户端 ID。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  mtu_size  客户端接收mtu。
 * @retval 执行结果错误码， mtu改变结果将在 { @ref gattc_mtu_changed_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_exchange_mtu_req(uint8_t client_id, uint16_t conn_id, uint16_t mtu_size);

/**
 * @if Eng
 * @brief  Register gatt client callbacks.
 * @par Description: Register gatt client callbacks.
 * @param  [in]  func Callback function.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  注册gatt客户端回调函数。
 * @par Description: 注册gatt客户端回调函数。
 * @param  [in]  func 回调函数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t gattc_register_callbacks(gattc_callbacks_t *func);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif

