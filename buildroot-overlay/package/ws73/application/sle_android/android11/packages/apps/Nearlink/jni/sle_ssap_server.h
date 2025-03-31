/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE Service Access Protocol SERVER module.
 */

/**
 * @defgroup sle_ssap_server Service Access Protocol SERVER API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_SSAP_SERVER_H
#define SLE_SSAP_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "sle_errcode.h"
#include "sle_common.h"
#include "sle_ssap_stru.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief Struct of add property information.
 * @else
 * @brief 添加特征信息。
 * @endif
 */
typedef struct {
    sle_uuid_t uuid;             /*!< @if Eng UUID of SSAP property.
                                      @else   SSAP 特征 UUID。 @endif */
    uint16_t permissions;        /*!< @if Eng Properity permissions. { @ref ssap_permission_t }
                                      @else   特征权限。{ @ref ssap_permission_t }。 @endif */
    uint32_t operate_indication; /*!< @if Eng Operate Indication. { @ref ssap_operate_indication_t }
                                      @else   操作指示 { @ref ssap_operate_indication_t } @endif */
    uint16_t value_len;          /*!< @if Eng Length of reponse data.
                                      @else   响应的数据长度。 @endif */
    uint8_t *value;              /*!< @if Eng Reponse data.
                                      @else   响应的数据。 @endif */
} ssaps_property_info_t;

/**
 * @if Eng
 * @brief Struct of add property descriptor information.
 * @else
 * @brief 添加特征描述符信息。
 * @endif
 */
typedef struct {
    sle_uuid_t uuid;             /*!< @if Eng UUID of SSAP descriptor.
                                      @else   SSAP 描述符 UUID。 @endif */
    uint16_t permissions;        /*!< @if Eng descriptor permissions. { @ref ssap_permission_t }.
                                      @else   特征权限。 { @ref ssap_permission_t } @endif */
    uint32_t operate_indication; /*!< @if Eng operate Indication. { @ref ssap_operate_indication_t }
                                      @else   操作指示 { @ref ssap_operate_indication_t } @endif */
    uint8_t type;                /*!< @if Eng descriptor type. { @ref ssap_property_type_t }.
                                      @else   描述符类型。 { @ref ssap_property_type_t } @endif */
    uint16_t value_len;          /*!< @if Eng data length.
                                      @else   数据长度。 @endif */
    uint8_t *value;              /*!< @if Eng data.
                                      @else   数据。 @endif */
} ssaps_desc_info_t;

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
    uint16_t handle;      /*!< @if Eng Properity handle of the read request.
                               @else   请求读的属性句柄。 @endif */
    uint8_t type;         /*!< @if Eng property type { @ref ssap_property_type_t }.
                               @else   属性类型。 @endif  { @ref ssap_property_type_t } */
    bool need_rsp;        /*!< @if Eng Whether response is needed.
                               @else   是否需要发送响应。 @endif */
    bool need_authorize;  /*!< @if Eng Whether authorize is needed.
                               @else   是否授权。 @endif */
} ssaps_req_read_cb_t;

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
    uint16_t handle;      /*!< @if Eng Properity handle of the write request.
                               @else   请求写的属性句柄。 @endif */
    uint8_t type;         /*!< @if Eng property type { @ref ssap_property_type_t }.
                               @else   属性类型。 @endif  { @ref ssap_property_type_t } */
    bool need_rsp;        /*!< @if Eng Whether response is needed.
                               @else   是否需要发送响应。 @endif */
    bool need_authorize;  /*!< @if Eng Whether authorize is needed.
                               @else   是否授权。 @endif */
    uint16_t length;      /*!< @if Eng Length of write request data.
                               @else   请求写的数据长度。 @endif */
    uint8_t *value;       /*!< @if Eng Write request data.
                               @else   请求写的数据。 @endif */
} ssaps_req_write_cb_t;

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
    uint8_t status;      /*!< @if Eng Status code of read/write. Success:
                                      ERRCODE_SLE_SUCCESS. For details about the exception { @ref errcode_sle_ssap_t }
                              @else   读写结果的状态, 成功ERRCODE_SLE_SUCCESS
                                      异常参考{ @ref errcode_sle_ssap_t } @endif */
    uint16_t value_len;  /*!< @if Eng Length of reponse data.
                              @else   响应的数据长度。 @endif */
    uint8_t *value;      /*!< @if Eng Reponse data.
                              @else   响应的数据。 @endif */
} ssaps_send_rsp_t;

/**
 * @if Eng
 * @brief Struct of send notification/indication information.
 * @else
 * @brief 发送通知/指示信息。
 * @endif
 */
typedef struct {
    uint16_t handle;      /*!< @if Eng Properity handle.
                               @else   属性句柄。 @endif */
    uint8_t type;         /*!< @if Eng property type { @ref ssap_property_type_t }.
                               @else   属性类型 { @ref ssap_property_type_t }。 @endif */
    uint16_t value_len;   /*!< @if Eng Length of notification/indication data.
                               @else   通知/指示数据长度。 @endif */
    uint8_t *value;       /*!< @if Eng Notification/indication data.
                               @else   发送的通知/指示数据。 @endif */
} ssaps_ntf_ind_t;

/**
 * @if Eng
 * @brief Struct of send notification/indication by uuid information.
 * @else
 * @brief 通过uuid发送通知/指示信息。
 * @endif
 */
typedef struct {
    sle_uuid_t uuid;       /*!< @if Eng Properity UUID.
                                @else   特征UUID。 @endif */
    uint16_t start_handle; /*!< @if Eng start handle.
                                @else   起始句柄。 @endif */
    uint16_t end_handle;   /*!< @if Eng end handle.
                                @else   结束句柄。 @endif */
    uint8_t type;          /*!< @if Eng property type { @ref ssap_property_type_t }.
                                @else   属性类型。 @endif { @ref ssap_property_type_t } */
    uint16_t value_len;    /*!< @if Eng Length of notification/indication data.
                                @else   通知/指示数据长度。 @endif */
    uint8_t *value;        /*!< @if Eng Notification/indication data.
                                @else   发送的通知/指示数据。 @endif */
} ssaps_ntf_ind_by_uuid_t;

/**
 * @if Eng
 * @brief Callback invoked when service add.
 * @par Callback invoked when service add.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2. The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] server_id server ID.
 * @param [in] uuid      service uuid.
 * @param [in] handle    service attribute handle.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  服务注册的回调函数。
 * @par    服务注册的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] server_id 服务端 ID。
 * @param [in] uuid      服务uuid。
 * @param [in] handle    服务属性句柄。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_add_service_callback)(uint8_t server_id, sle_uuid_t *uuid, uint16_t handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when property add.
 * @par Callback invoked when property add.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2. The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] server_id      server ID.
 * @param [in] uuid           property uuid.
 * @param [in] service_handle service attribute handle.
 * @param [in] handle         character attribute handle.
 * @param [in] status         error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  特征注册的回调函数。
 * @par    特征注册的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] server_id      服务端 ID。
 * @param [in] uuid           特征 uuid。
 * @param [in] service_handle 服务属性句柄。
 * @param [in] handle         特征属性句柄。
 * @param [in] status         执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_add_property_callback)(uint8_t server_id, sle_uuid_t *uuid, uint16_t service_handle,
    uint16_t handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when property descriptor add.
 * @par Callback invoked when property descriptor add.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2. The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] server_id       server ID.
 * @param [in] uuid            property uuid.
 * @param [in] service_handle  service attribute handle.
 * @param [in] property_handle character descriptor attribute handle.
 * @param [in] status          error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  特征描述符注册的回调函数。
 * @par    特征描述符注册的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] server_id       服务端 ID。
 * @param [in] uuid            特征 uuid。
 * @param [in] service_handle  服务属性句柄。
 * @param [in] property_handle 特征描述符属性句柄。
 * @param [in] status          执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_add_descriptor_callback)(uint8_t server_id, sle_uuid_t *uuid, uint16_t service_handle,
    uint16_t property_handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when service started.
 * @par Callback invoked when service started.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2. The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] server_id server ID.
 * @param [in] handle    service attribute handle.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  开始服务的回调函数。
 * @par    开始服务的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] server_id 服务端 ID。
 * @param [in] handle    服务属性句柄。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_start_service_callback)(uint8_t server_id, uint16_t handle, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when all services deleted.
 * @par Callback invoked when all services deleted.
 * @attention 1.This function is called in bts context,should not be blocked or do long time waiting.
 * @attention 2. The memories of <devices> are requested and freed by the bts automatically.
 * @param [in] server_id server ID.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  删除全部服务的回调函数。
 * @par    删除全部服务的回调函数。
 * @attention  1. 该回调函数运行于bts线程，不能阻塞或长时间等待。
 * @attention  2. <devices>由bts申请内存，也由bts释放，回调中不应释放。
 * @param [in] server_id 服务端 ID。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_delete_all_service_callback)(uint8_t server_id, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive read request.
 * @par Callback invoked when  receive read request.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2. The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] server_id    server ID.
 * @param [in] conn_id      connection ID.
 * @param [in] read_cb_para read request parameter.
 * @param [in] status       error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  收到读请求的回调函数。
 * @par    收到读请求的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] server_id    服务端 ID。
 * @param [in] conn_id      连接 ID。
 * @param [in] read_cb_para 读请求参数。
 * @param [in] status       执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_read_request_callback)(uint8_t server_id, uint16_t conn_id, ssaps_req_read_cb_t *read_cb_para,
    errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive write request.
 * @par Callback invoked when  receive write request.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2. The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] server_id     server ID.
 * @param [in] conn_id       connection ID.
 * @param [in] write_cb_para write request parameter.
 * @param [in] status        error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  收到写请求的回调函数。
 * @par    收到写请求的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] server_id     服务端 ID。
 * @param [in] conn_id       连接 ID。
 * @param [in] write_cb_para 写请求参数。
 * @param [in] status        执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_write_request_callback)(uint8_t server_id, uint16_t conn_id, ssaps_req_write_cb_t *write_cb_para,
    errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when mtu size changed.
 * @par Callback invoked when mtu size changed.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2. The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] server_id server ID.
 * @param [in] conn_id   connection ID.
 * @param [in] mtu_size  mtu size.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @else
 * @brief  mtu大小改变的回调函数。
 * @par    mtu大小改变的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] server_id 服务端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] mtu_size  mtu 大小。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_ssap_stru.h
 * @see sle_ssaps_callbacks_t
 * @endif
 */
typedef void (*ssaps_mtu_changed_callback)(uint8_t server_id, uint16_t conn_id,
    ssap_exchange_info_t *mtu_size, errcode_t status);

/**
 * @if Eng
 * @brief Struct of SSAP server callback function.
 * @else
 * @brief SSAP server回调函数接口定义。
 * @endif
 */
typedef struct {
    ssaps_add_service_callback add_service_cb;               /*!< @if Eng Service added callback.
                                                                  @else   添加服务回调函数。 @endif */
    ssaps_add_property_callback add_property_cb;             /*!< @if Eng Characteristc added callback.
                                                                  @else   添加特征回调函数。 @endif */
    ssaps_add_descriptor_callback add_descriptor_cb;         /*!< @if Eng Descriptor added callback.
                                                                  @else   添加描述符回调函数。 @endif */
    ssaps_start_service_callback start_service_cb;           /*!< @if Eng Service started callback.
                                                                  @else   启动服务回调函数。 @endif */
    ssaps_delete_all_service_callback delete_all_service_cb; /*!< @if Eng Service deleted callback.
                                                                  @else   删除服务回调函数。 @endif */
    ssaps_read_request_callback read_request_cb;             /*!< @if Eng Read request received callback.
                                                                  @else   收到远端读请求回调函数。 @endif */
    ssaps_write_request_callback write_request_cb;           /*!< @if Eng Write request received callback.
                                                                  @else   收到远端写请求回调函数。 @endif */
    ssaps_mtu_changed_callback mtu_changed_cb;               /*!< @if Eng Mtu changed callback.
                                                                  @else   mtu 大小更新回调函数。 @endif */
} ssaps_callbacks_t;

/**
 * @if Eng
 * @brief  Register ssap server.
 * @par Description: Register ssap server.
 * @param  [in]  app_uuid  App uuid
 * @param  [out] server_id Server ID
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  注册ssap服务端。
 * @par Description: 注册ssap服务端。
 * @param  [in]  app_uuid  上层应用uuid
 * @param  [out] server_id 服务端ID
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_register_server(sle_uuid_t *app_uuid, uint8_t *server_id);

/**
 * @if Eng
 * @brief  Unregister ssap server.
 * @par Description: Unregister ssap server.
 * @param  [in] server_id Server ID
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  注销ssap服务端。
 * @par Description: 注销ssap服务端。
 * @param  [in] server_id 服务端ID
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_unregister_server(uint8_t server_id);

/**
 * @if Eng
 * @brief  asynchronously add an SSAP service. The registered service handle is returned in add_service_cb.
 * @par Description: Add a ssap service.
 * @param  [in] server_id Server ID.
 * @param  [in] service_uuid Service uuid.
 * @param  [in] is_primary is primary service or not.
 * @retval error code, the service handle will be returned in { @ref ssaps_add_service_callback }.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  异步添加一个ssap服务，注册service handle在add_service_cb中返回。
 * @par Description: 添加一个ssap服务。
 * @param  [in] server_id 服务端 ID。
 * @param  [in] service_uuid 服务uuid。
 * @param  [in] is_primary 是否是首要服务。
 * @retval 执行结果错误码，服务句柄将在 { @ref ssaps_add_service_callback }中返回。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_add_service(uint8_t server_id, sle_uuid_t *service_uuid, bool is_primary);

/**
 * @if Eng
 * @brief  Add an ssap attribute asynchronously. The register attribute handle is returned in add_property_cb.
 * @par Description: Add a ssap property.
 * @param  [in] server_id Server ID.
 * @param  [in] service_handle Service handle.
 * @param  [in] property SSAP Properity.
 * @retval error code, the character handle will be returned in { @ref ssaps_add_property_callback }.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  异步添加一个ssap属性，注册属性 handle在add_property_cb中返回
 * @par Description: 添加一个ssap特征。
 * @param  [in] server_id 服务端 ID。
 * @param  [in] service_handle 服务句柄。
 * @param  [in] property SSAP 特征。
 * @retval 执行结果错误码，特征句柄将在 { @ref ssaps_add_property_callback }中返回。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_add_property(uint8_t server_id, uint16_t service_handle, ssaps_property_info_t *property);

/**
 * @if Eng
 * @brief  An SSAP description is added asynchronously. The registration result is returned in add_descriptor_cb.
 * @par Description: Add a ssap property descriptor.
 * @param  [in] server_id       Server ID.
 * @param  [in] service_handle  Service handle.
 * @param  [in] property_handle Property handle.
 * @param  [in] descriptor      Properity descriptor.
 * @retval error code, the descriptor handle will be returned in { @ref ssaps_add_descriptor_callback }.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  异步添加一个ssap描述，注册描述结果在add_descriptor_cb中返回
 * @par Description: 添加一个ssap特征描述符。
 * @param  [in] server_id       服务端 ID。
 * @param  [in] service_handle  服务句柄。
 * @param  [in] property_handle 特征句柄。
 * @param  [in] descriptor      SSAP 特征描述符。
 * @retval 执行结果错误码，特征句柄将在 { @ref ssaps_add_descriptor_callback } 中返回。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_add_descriptor(uint8_t server_id, uint16_t service_handle, uint16_t property_handle,
    ssaps_desc_info_t *descriptor);

/**
 * @if Eng
 * @brief  Add a ssap service.
 * @par Description: Add a ssap service.
 * @param  [in]  server_id Server ID.
 * @param  [in]  service_uuid Service uuid.
 * @param  [in]  is_primary is primary service or not.
 * @param  [out] handle service handle.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  添加一个ssap服务。
 * @par Description: 添加一个ssap服务。
 * @param  [in]  server_id    服务端 ID。
 * @param  [in]  service_uuid 服务uuid。
 * @param  [in]  is_primary   是否是首要服务。
 * @param  [out] handle       服务句柄.
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_add_service_sync(uint8_t server_id, sle_uuid_t *service_uuid, bool is_primary, uint16_t *handle);

/**
 * @if Eng
 * @brief  Add a ssap property.
 * @par Description: Add a ssap property.
 * @param  [in]  server_id      Server ID.
 * @param  [in]  service_handle Service handle.
 * @param  [in]  property      SSAP Properity Info.
 * @param  [out] handle         Properity handle.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  添加一个ssap特征。
 * @par Description: 添加一个ssap特征。
 * @param  [in]  server_id      服务端 ID。
 * @param  [in]  service_handle 服务句柄。
 * @param  [in]  property      SSAP 特征信息。
 * @param  [out] handle         特征句柄
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_add_property_sync(uint8_t server_id, uint16_t service_handle, ssaps_property_info_t *property,
    uint16_t *handle);

/**
 * @if Eng
 * @brief  Add a ssap property descriptor.
 * @par Description: Add a ssap property descriptor.
 * @param  [in]  server_id       Server ID.
 * @param  [in]  service_handle  Service handle.
 * @param  [in]  property_handle Properity handle corresponding to the descriptor.
 * @param  [in]  descriptor      Properity descriptor.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  添加一个ssap特征描述符。
 * @par Description: 添加一个ssap特征描述符。
 * @param  [in]  server_id       服务端 ID。
 * @param  [in]  service_handle  服务句柄。
 * @param  [in]  property_handle 描述符所在得特征句柄。
 * @param  [in]  descriptor      特征描述符。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_add_descriptor_sync(uint8_t server_id, uint16_t service_handle, uint16_t property_handle,
    ssaps_desc_info_t *descriptor);

/**
 * @if Eng
 * @brief  Start a SSAP service.
 * @par Description: Start a SSAP service.
 * @param  [in] server_id      server ID.
 * @param  [in] service_handle service handle.
 * @retval error code, the service start result will be returned at { @ref ssaps_start_service_callback }.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  开始一个SSAP服务。
 * @par Description: 开始一个SSAP服务。
 * @param  [in] server_id      服务端 ID。
 * @param  [in] service_handle 服务句柄。
 * @retval 执行结果错误码，服务开启结果将在 { @ref ssaps_start_service_callback }中返回。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_start_service(uint8_t server_id, uint16_t service_handle);

/**
 * @if Eng
 * @brief  Delete all SSAP service.
 * @par Description: Delete all SSAP service.
 * @param  [in] server_id      server ID.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  删除所有SSAP服务。
 * @par Description: 删除所有SSAP服务。
 * @param  [in] server_id      服务端 ID。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_delete_all_services(uint8_t server_id);

/**
 * @if Eng
 * @brief  Send response when receive the request need to response by user.
 * @par Description: Send response when receive the request need to response by user
                     { @ref ssaps_read_request_callback } { @ref ssaps_write_request_callback }.
 * @param  [in] server_id server ID.
 * @param  [in] conn_id   connection ID.
 * @param  [in] param     response parameter.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  当收到需要用户回复响应的请求时发送响应。
 * @par Description: 当收到需要用户回复响应的请求时发送响应
                     { @ref ssaps_read_request_callback } { @ref ssaps_write_request_callback }。
 * @param  [in] server_id 服务端 ID.
 * @param  [in] conn_id   连接ID。
 * @param  [in] param     响应参数.
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_send_response(uint8_t server_id, uint16_t conn_id, ssaps_send_rsp_t *param);

/**
 * @if Eng
 * @brief  Send indication or notification to remote device.
 * @par Description: Send indication or notification to remote device,
                     send status depend on character descriptor: client property configuration.
                     value = 0x0000: notification and indication not allowed
                     value = 0x0001: notification allowed
                     value = 0x0002: indication allowed
 * @param  [in] server_id server ID.
 * @param  [in] conn_id   connection ID，To send packets to all peer ends, enter conn_id = 0xffff.
 * @param  [in] param     notify/indicate parameter.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  向对端发送通知或指示。
 * @par Description: 向对端发送通知或指示，具体发送状态取决于特征描述符：客户端特征配置
                     value = 0x0000：不允许通知和指示
                     value = 0x0001：允许通知
                     value = 0x0002：允许指示
 * @param  [in] server_id 服务端 ID。
 * @param  [in] conn_id   连接ID，如果向全部对端发送则输入conn_id = 0xffff；
 * @param  [in] param     通知或指示参数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_notify_indicate(uint8_t server_id, uint16_t conn_id, ssaps_ntf_ind_t *param);

/**
 * @if Eng
 * @brief  Send indication or notification to remote device by uuid.
 * @par Description: Send indication or notification to remote device by uuid,
                     send status depend on client property configuration descriptor value,
                     value = 0x0000: notification and indication not allowed
                     value = 0x0001: notification allowed
                     value = 0x0002: indication allowed
 * @param  [in] server_id server ID.
 * @param  [in] conn_id   connection ID，To send packets to all peer ends, enter conn_id = 0xffff.
 * @param  [in] param     notify/indicate parameter.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  通过uuid向对端发送通知或指示。
 * @par Description: 通过uuid向对端发送通知或指示，具体发送状态取决于客户端特征配置描述符值，
                     value = 0x0000：不允许通知和指示
                     value = 0x0001：允许通知
                     value = 0x0002：允许指示
 * @param  [in] server_id 服务端 ID。
 * @param  [in] conn_id   连接ID，如果向全部对端发送则输入conn_id = 0xffff；
 * @param  [in] param     通知或指示参数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_notify_indicate_by_uuid(uint8_t server_id, uint16_t conn_id, ssaps_ntf_ind_by_uuid_t *param);


/**
 * @if Eng
 * @brief  Set server info before connected.
 * @par Description: Set server info before connected.
 * @param  [in] server_id server ID.
 * @param  [in] info      server info.
 * @retval error code.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  在连接之前设置服务端info。
 * @par Description: 在连接之前设置服务端info。
 * @param  [in] server_id 服务端ID。
 * @param  [in] info      服务端info。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ssaps_set_info(uint8_t server_id, ssap_exchange_info_t *info);

/**
 * @if Eng
 * @brief  Register SSAP server callbacks.
 * @par Description: Register SSAP server callbacks.
 * @param  [in] func callback function.
 * @retval error code.
 * @par Depends:
 * @li sle_ssap_stru.h
 * @else
 * @brief  注册 SSAP server 回调函数。
 * @par Description: 注册回调函数。
 * @param  [in] func 回调函数
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_ssap_stru.h
 * @endif
 */
errcode_t ssaps_register_callbacks(ssaps_callbacks_t *func);

#ifdef __cplusplus
}
#endif
#endif
/** @} */

