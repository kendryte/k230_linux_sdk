/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE Service Access Protocol CLIENT module.
 */

/**
 * @defgroup sle ssap client Service Access Protocol CLient API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_SSAP_CLIENT_H
#define SLE_SSAP_CLIENT_H
 
#include <stdint.h>
#include "sle_errcode.h"
#include "sle_common.h"
#include "sle_ssap_stru.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief Service discovery result.
 * @else
 * @brief 服务发现结果。
 * @endif
 */
typedef struct {
    uint16_t   start_hdl;  /*!< @if Eng service start handle.
                                @else   服务起始句柄。 @endif */
    uint16_t   end_hdl;    /*!< @if Eng service end handle.
                                @else   服务结束句柄。 @endif */
    sle_uuid_t uuid;       /*!< @if Eng service end handle.
                                @else   服务UUID。 @endif */
} ssapc_find_service_result_t;

 /**
 * @if Eng
 * @brief Attribute discovery result.
 * @else
 * @brief 属性发现结果。
 * @endif
 */
typedef struct {
    uint16_t   handle;                 /*!< @if Eng property handle.
                                            @else   属性句柄。 @endif */
    uint32_t   operate_indication;     /*!< @if Eng Operation instructions.
                                            @else   操作指示。 @endif */
    sle_uuid_t uuid;                   /*!< @if Eng UUID.
                                            @else   UUID标识。 @endif */
    uint8_t    descriptors_count;      /*!< @if Eng Attribute Descriptor Type List Length.
                                            @else  属性描述符类型列表长度。 @endif */
    uint8_t    descriptors_type[0];    /*!< @if Eng Attribute descriptor type list.
                                                    { @ref ssap_property_descriptor_type_t }.
                                            @else   属性描述符类型列表，{ @ref ssap_property_descriptor_type_t }类型定义
                                            @endif */
} ssapc_find_property_result_t;

/**
 * @if Eng
 * @brief Results for SSAP handle and value.
 * @else
 * @brief SSAP 句柄值。
 * @endif
 */
typedef struct {
    uint16_t handle;    /*!< @if Eng property handle.
                             @else   属性句柄。 @endif */
    uint8_t  type;      /*!< @if Eng property type.
                             @else   属性类型。 @endif */
    uint16_t data_len;  /*!< @if Eng Data Length.
                             @else   数据长度。 @endif */
    uint8_t  *data;     /*!< @if Eng Data.
                             @else   数据内容。 @endif */
} ssapc_handle_value_t, ssapc_write_param_t;

/**
 * @if Eng
 * @brief Results for SSAP write operation.
 * @else
 * @brief SSAP 写结果。
 * @endif
 */
typedef struct {
    uint16_t handle;  /*!< @if Eng property handle.
                           @else   属性句柄。 @endif */
    uint8_t  type;    /*!< @if Eng property type.
                           @else   属性类型。 @endif */
} ssapc_write_result_t;

/**
 * @if Eng
 * @brief Read by uuid complete.
 * @else
 * @brief 读取by uuid完成。
 * @endif
 */
typedef struct {
    sle_uuid_t uuid; /*!< @if Eng property handle.
                          @else   属性句柄。 @endif */
    uint8_t type;    /*!< @if Eng property type.
                          @else   属性类型。 @endif */
} ssapc_read_by_uuid_cmp_result_t;

/**
 * @if Eng
 * @brief SSAP find operation parameter.
 * @else
 * @brief SSAP 查找参数。
 * @endif
 */
typedef struct {
    uint8_t    type;      /*!< @if Eng find type { @ref ssap_find_type_t }.
                               @else   查找类型 { @ref ssap_find_type_t }。 @endif */
    uint16_t   start_hdl; /*!< @if Eng start handle.
                               @else   起始句柄 @endif */
    uint16_t   end_hdl;   /*!< @if Eng end handle.
                               @else   结束句柄 @endif */
    sle_uuid_t uuid;      /*!< @if Eng uuid, only valid when find structure by uuid.
                               @else   uuid，按照uuid查找时生效，其余不生效 @endif */
    uint8_t    reserve;   /*!< @if Eng reserve, "0" for default value.
                               @else   预留，默认值写0 @endif */
} ssapc_find_structure_param_t;

/**
 * @if Eng
 * @brief Service Discovery Response Parameters
 * @else
 * @brief 服务发现响应参数
 * @endif
 */
typedef struct {
    uint8_t    type;      /*!< @if Eng find type { @ref ssap_find_type_t }.
                               @else   查找类型 { @ref ssap_find_type_t }。 @endif */
    sle_uuid_t uuid;      /*!< @if Eng uuid
                               @else   uuid @endif */
} ssapc_find_structure_result_t;

/**
 * @if Eng
 * @brief Struct of send read request by uuid parameter.
 * @else
 * @brief 向对端发送按照uuid读取请求的参数。
 * @endif
 */
typedef struct {
    uint8_t    type;        /*!< @if Eng find type { @ref ssap_find_type_t }.
                                 @else   查找类型 { @ref ssap_find_type_t }。 @endif */
    uint16_t   start_hdl;   /*!< @if Eng start handle.
                                 @else   起始句柄。@endif */
    uint16_t   end_hdl;     /*!< @if Eng end handle.
                                 @else   结束句柄。@endif */
    sle_uuid_t uuid;        /*!< @if Eng uuid.
                                 @else   uuid@endif */
} ssapc_read_req_by_uuid_param_t;

/**
 * @if Eng
 * @brief Callback invoked when service discovery.
 * @par Callback invoked when service discovery.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id client ID.
 * @param [in] conn_id   connection ID.
 * @param [in] service   service information.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  服务发现的回调函数。
 * @par    服务发现的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] service   发现的服务信息。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
typedef void (*ssapc_find_structure_callback)(uint8_t client_id, uint16_t conn_id,
    ssapc_find_service_result_t *service, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when property discovered.
 * @par Callback invoked when property discovered.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id client ID.
 * @param [in] conn_id   connection ID.
 * @param [in] property  property.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  属性发现的回调函数。
 * @par    属性发现的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] property  特征。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_find_property_callback)(uint8_t client_id, uint16_t conn_id,
    ssapc_find_property_result_t *property, errcode_t status);
 
/**
 * @if Eng
 * @brief Callback invoked when find completed.
 * @par Callback invoked when find completed.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id        client ID.
 * @param [in] conn_id          connection ID.
 * @param [in] structure_result input parameter.
 * @param [in] status           error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  查找完成的回调函数。
 * @par    查找完成的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id        客户端 ID。
 * @param [in] conn_id          连接 ID。
 * @param [in] structure_result 入参回传。
 * @param [in] status           执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_find_structure_complete_callback)(uint8_t client_id, uint16_t conn_id,
    ssapc_find_structure_result_t *structure_result, errcode_t status);
 
/**
 * @if Eng
 * @brief Callback invoked when receive read response.
 * @par Callback invoked when receive read response.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id client ID.
 * @param [in] conn_id   connection ID.
 * @param [in] read_data read data.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  收到读响应的回调函数。
 * @par    收到读响应的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] read_data 读取到的数据。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_read_cfm_callback)(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *read_data,
    errcode_t status);
 
/**
 * @if Eng
 * @brief Callback invoked when read feature completed.
 * @par Callback invoked when read feature completed.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id  client ID.
 * @param [in] conn_id    connection ID.
 * @param [in] cmp_result input parameter.
 * @param [in] status     error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  读取完成的回调函数。
 * @par    读取完成的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id  客户端 ID。
 * @param [in] conn_id    连接 ID。
 * @param [in] cmp_result 入参回传。
 * @param [in] status     执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_read_by_uuid_complete_callback)(uint8_t client_id, uint16_t conn_id,
    ssapc_read_by_uuid_cmp_result_t *cmp_result, errcode_t status);
 
/**
 * @if Eng
 * @brief Callback function for receiving write response.
 * @par Callback function for receiving write response.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id    client ID.
 * @param [in] conn_id      connection ID.
 * @param [in] write_result write result.
 * @param [in] status       error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  收到写响应的回调函数。
 * @par    收到写响应的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id    客户端 ID。
 * @param [in] conn_id      连接 ID。
 * @param [in] write_result 写结果。
 * @param [in] status       执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_write_cfm_callback)(uint8_t client_id, uint16_t conn_id, ssapc_write_result_t *write_result,
    errcode_t status);
 
/**
 * @if Eng
 * @brief Callback invoked when receive exchange info response.
 * @par Callback invoked when receive exchange info response.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id client ID.
 * @param [in] conn_id   connection ID.
 * @param [in] param     exchange info.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  mtu改变的回调函数。
 * @par    mtu改变的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] param     交换信息。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_exchange_info_callback)(uint8_t client_id, uint16_t conn_id, ssap_exchange_info_t *param,
    errcode_t status);
 
/**
 * @if Eng
 * @brief Callback function for receiving a notification.
 * @par Callback function for receiving a notification.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id client ID.
 * @param [in] conn_id   connection ID.
 * @param [in] data      data.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  收到通知的回调函数。
 * @par    收到通知的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] data      数据。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_notification_callback)(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *data,
    errcode_t status);
 
/**
 * @if Eng
 * @brief Callback function for receiving a indication.
 * @par Callback function for receiving a indication.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] client_id client ID.
 * @param [in] conn_id   connection ID.
 * @param [in] data      data.
 * @param [in] status    error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @else
 * @brief  收到指示的回调函数。
 * @par    收到指示的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. pointer由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] data      数据。
 * @param [in] status    执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see ssapc_callbacks_t
 * @endif
 */
typedef void (*ssapc_indication_callback)(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *data,
    errcode_t status);
 
/**
 * @if Eng
 * @brief Struct of ssap client callback function.
 * @else
 * @brief ssap client回调函数接口定义。
 * @endif
 */
typedef struct {
    ssapc_find_structure_callback find_structure_cb;              /*!< @if Eng Discovery structure callback.
                                                                       @else   发现服务回调函数。 @endif */
    ssapc_find_property_callback ssapc_find_property_cbk;         /*!< @if Eng Discovery property callback.
                                                                       @else   发现特征回调函数。 @endif */
    ssapc_find_structure_complete_callback find_structure_cmp_cb; /*!< @if Eng Discovery structure complete callback.
                                                                       @else   发现服务完成回调函数。 @endif */
    ssapc_read_cfm_callback read_cfm_cb;                          /*!< @if Eng Receive write response callback.
                                                                       @else   收到写响应回调函数。 @endif */
    ssapc_read_by_uuid_complete_callback read_by_uuid_cmp_cb;     /*!< @if Eng Callback hook for read property complete.
                                                                       @else   读特征值完成回调钩子。 @endif */
    ssapc_write_cfm_callback write_cfm_cb;                        /*!< @if Eng Receive write response callback.
                                                                       @else   收到写响应回调函数。 @endif */
    ssapc_exchange_info_callback exchange_info_cb;                /*!< @if Eng Callback hook for configure mtu size
                                                                               complete.
                                                                       @else   更新mtu大小回调钩子。 @endif */
    ssapc_notification_callback notification_cb;                  /*!< @if Eng Callback hook for receive notification.
                                                                       @else   通知事件上报钩子。 @endif */
    ssapc_indication_callback indication_cb;                      /*!< @if Eng Callback hook for receive indication.
                                                                       @else   指示事件上报钩子。 @endif */
} ssapc_callbacks_t;
 
/**
 * @if Eng
 * @brief  Register ssap client.
 * @par Description: Register ssap client.
 * @param  [in]  app_uuid  App uuid
 * @param  [out] client_id Client ID
 * @retval error code.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  注册ssap客户端。
 * @par Description: 注册ssap客户端。
 * @param  [in]  app_uuid  上层应用uuid
 * @param  [out] client_id 客户端ID
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_register_client(sle_uuid_t *app_uuid, uint8_t *client_id);
/**
 * @if Eng
 * @brief  Unregister ssap client.
 * @par Description: Unregister ssap client.
 * @param  [in] client_id Client ID
 * @retval error code.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  注销ssap客户端。
 * @par Description: 注销ssap服务端。
 * @param  [in] client_id 客户端ID
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_unregister_client(uint8_t client_id);
 
/**
 * @if Eng
 * @brief  Find structure.
 * @par Description: Find structure.
 * @param [in] client_id Client ID.
 * @param [in] conn_id   Connection ID.
 * @param [in] param     structure uuid, if NULL, discover all structure, else filter by uuid.
 * @retval error code, discovery structure result will be returned at { @ref ssapc_find_structure_callback } and
           { @ref ssapc_find_structure_complete_callback }.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  查找服务、特征、描述符。
 * @par Description: 查找服务、特征、描述符。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] param     查找参数。
 * @retval 执行结果错误码, 服务发现结果将在 { @ref ssapc_find_structure_callback }和
           { @ref ssapc_find_structure_complete_callback }中返回。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_find_structure(uint8_t client_id, uint16_t conn_id, ssapc_find_structure_param_t *param);
 
/**
 * @if Eng
 * @brief  Send read by uuid request.
 * @par Description: Send read by uuid request.
 * @param [in] client_id Client ID.
 * @param [in] conn_id   Connection ID.
 * @param [in] param     Parameter for read request by uuid.
 * @retval error code, read result will be returned at { @ref ssapc_read_cfm_callback } and
           { @ref ssapc_read_by_uuid_complete_callback }.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  发起按照uuid读取请求。
 * @par Description: 发起按照uuid读取请求。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] param     按照uuid读取请求参数。
 * @retval 执行结果错误码，读取结果将在 { @ref ssapc_read_cfm_callback }和
           { @ref ssapc_read_by_uuid_complete_callback }中返回。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_read_req_by_uuid(uint8_t client_id, uint16_t conn_id, ssapc_read_req_by_uuid_param_t *param);

 /**
 * @if Eng
 * @brief  Send read by handle request.
 * @par Description: Send read by handle request.
 * @param [in] client_id Client ID.
 * @param [in] conn_id   Connection ID.
 * @param [in] handle    handle.
 * @param [in] type      property type.
 * @retval error code, read result will be returned at { @ref ssapc_read_cfm_callback }.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  发起按照句柄读取请求。
 * @par Description: 发起按照句柄读取请求。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] handle    句柄。
 * @param [in] type      特征类型。
 * @retval 执行结果错误码，读取结果将在 { @ref ssapc_read_cfm_callback }中返回。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_read_req(uint8_t client_id, uint16_t conn_id, uint16_t handle, uint8_t type);
 
/**
 * @if Eng
 * @brief  Send write request.
 * @par Description: Send write request.
 * @param [in] client_id Client ID.
 * @param [in] conn_id   Connection ID.
 * @param [in] param     Parameter for write request.
 * @retval error code, write result will be returned at { @ref ssapc_write_cfm_callback }.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  发起写请求。
 * @par Description: 发起写请求。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] param     写请求参数。
 * @retval 执行结果错误码，写结果将在 { @ref ssapc_write_cfm_callback }中返回。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_write_req(uint8_t client_id, uint16_t conn_id, ssapc_write_param_t *param);
 
/**
 * @if Eng
 * @brief  Send write command.
 * @par Description: Send write command.
 * @param [in] client_id Client ID.
 * @param [in] conn_id   Connection ID.
 * @param [in] param     Parameter for write command.
 * @retval error code.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  发起写命令。
 * @par Description: 发起写命令。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] param     写命令参数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_write_cmd(uint8_t client_id, uint16_t conn_id, ssapc_write_param_t *param);
 
/**
 * @if Eng
 * @brief  Send exchange info request.
 * @par Description: Send exchange info request.
 * @param [in] client_id Client ID.
 * @param [in] conn_id   Connection ID.
 * @param [in] param     Client info.
 * @retval error code, exchange result will be returned at { @ref ssapc_exchange_info_callback }.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  发送交换info请求。
 * @par Description: 发送交换info请求。
 * @param [in] client_id 客户端 ID。
 * @param [in] conn_id   连接 ID。
 * @param [in] param     客户端info。
 * @retval 执行结果错误码， mtu改变结果将在 { @ref ssapc_exchange_info_callback }中返回。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_exchange_info_req(uint8_t client_id, uint16_t conn_id, ssap_exchange_info_t* param);
 
/**
 * @if Eng
 * @brief  Register SSAP client callbacks.
 * @par Description: Register SSAP client callbacks.
 * @param [in] func Callback function.
 * @retval error code.
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  注册SSAP客户端回调函数。
 * @par Description: 注册SSAP客户端回调函数。
 * @param [in] func 回调函数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t ssapc_register_callbacks(ssapc_callbacks_t *func);
 
#ifdef __cplusplus
}
#endif
#endif /* SLE_SSAP_CLIENT_H */
