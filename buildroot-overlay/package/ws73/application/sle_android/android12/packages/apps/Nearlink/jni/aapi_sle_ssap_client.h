/**
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 *
 * Description: sle ssap client api.
 */

/**
 * @defgroup sle ssap client api
 * @ingroup  SLE
 * @{
 */

#ifndef AAPI_SLE_SSAP_CLIENT_H
#define AAPI_SLE_SSAP_CLIENT_H

#include <stdint.h>
#include "sle_errcode.h"
#include "sle_common.h"
#include "sle_ssap_stru.h"
#include "sle_connection_manager.h"
#include "sle_ssap_client.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    AAPI_OUT_STATUS_UNFINISHED,
    AAPI_OUT_STATUS_FINISHED,
};
typedef struct {
    uint8_t state;
    uint8_t resv;
    uint16_t conn_id;
    uint8_t conn_state;
    uint8_t pair_state;
} aapi_conn_state_t;

/* 服务树结构 */
typedef struct {
    uint16_t   handle;
    uint32_t   operate_indication;
    sle_uuid_t uuid;
    uint8_t    descriptors_count;
    uint8_t    descriptors_type[0];
} aapi_ssapc_property_t;

typedef struct aapi_ssapc_property_node {
    struct aapi_ssapc_property_node *next;
    aapi_ssapc_property_t property;
} aapi_ssapc_property_node_t;

typedef struct {
    uint16_t   start_hdl;
    uint16_t   end_hdl;
    sle_uuid_t uuid;
    aapi_ssapc_property_node_t *property_list;
} aapi_ssapc_structure_t;

typedef struct aapi_ssapc_structure_node {
    struct aapi_ssapc_structure_node *next;
    aapi_ssapc_structure_t service;
} aapi_ssapc_structure_node_t;

typedef struct {
    sle_connection_callbacks_t conn_cbks;
    ssapc_callbacks_t ssapc_cbks;
} aapi_callbacks_t;

/**
 * @brief  注册ssap客户端。
 * @par Description: 注册ssap客户端。
 * @param  [in]  app_uuid  上层应用uuid
 * @param  [in]  func 回调函数。
 * @param  [out] client_id 客户端ID
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 */
errcode_t aapi_ssapc_register_client(sle_uuid_t *app_uuid, aapi_callbacks_t *func, uint8_t *client_id);

/**
 * @brief  取消注册ssap客户端。
 * @par Description: 取消注册ssap客户端。
 * @param  [in] client_id 客户端ID
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 */
errcode_t aapi_ssapc_unregister_client(uint8_t client_id);

/**
 * @brief  连接对端。（完成连接对端、配对整个过程。）
 * @par Description: 连接对端。
 * @param  [in]  client_id 客户端ID
 * @param  [in]  addr 远端地址
 * @param  [out] 本次执行状态
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 */
errcode_t aapi_ssapc_connect_peer(uint8_t client_id, const sle_addr_t *addr, aapi_conn_state_t *out_statu);

/**
 * @brief  断链
 * @par Description: 断链。
 * @param  [in]  client_id 客户端ID
 * @param  [in]  addr 远端地址
 * @param  [out] 本次执行状态
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 */
errcode_t aapi_ssapc_disconnect_peer(uint8_t client_id, const sle_addr_t *addr, uint8_t *out_statu);

/**
 * @brief  服务发现（完成整个服务发现过程。）
 * @par Description: 服务发现。
 * @param  [in]  client id 实例ID
 * @param  [in]  conn id 虚拟链路ID
 * @param  [out] 本次执行状态
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 */
errcode_t aapi_ssapc_find_structure(uint8_t client_id, uint8_t conn_id, uint8_t *out_statu);

/**
 * @brief  查询服务
 * @par Description: 查询服务
 * @param  [in]  client id 实例ID
 * @param  [in]  conn id 虚拟链路ID
 * @param  [out] out 虚拟链路ID
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 */
errcode_t aapi_ssapc_get_structure(uint8_t client_id, uint8_t conn_id, aapi_ssapc_structure_node_t **out);

/**
 * @brief  删除服务
 * @par Description: 删除服务
 * @param  [in]  addr 远端地址
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 */
errcode_t aapi_ssapc_del_structure(sle_addr_t *addr);

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
errcode_t aapi_ssapc_read_req_by_uuid(uint8_t client_id, uint16_t conn_id, ssapc_read_req_by_uuid_param_t *param);

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
errcode_t aapi_ssapc_read_req(uint8_t client_id, uint16_t conn_id, uint16_t handle, uint8_t type);

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
errcode_t aapi_ssapc_write_req(uint8_t client_id, uint16_t conn_id, ssapc_write_param_t *param);

/**
 * @if Eng
 * @brief  Read remote device rssi value.
 * @par Description: Read remote device rssi value.
 * @param [in] client_id Client ID.
 * @param [in]  conn_id connection ID.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  读取对端设备rssi值。
 * @par Description: 读取对端设备rssi值。
 * @param [in] client_id 客户端 ID。
 * @param [in]  conn_id 连接 ID。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t aapi_sle_read_remote_device_rssi(uint8_t client_id, uint16_t conn_id);

/**
 * @if Eng
 * @brief  Read remote device rssi value.
 * @par Description: Read remote device rssi value.
 * @param [in] client_id Client ID.
 * @param [in]  conn_id connection ID.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  读取对端设备rssi值。
 * @par Description: 读取对端设备rssi值。
 * @param [in] client_id 客户端 ID。
 * @param [in]  conn_id 连接 ID。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t aapi_enable_sle(void);

/**
 * @if Eng
 * @brief  Remove pairing.
 * @par Description: Remove pairing.
 * @param [in] addr address.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  删除配对。
 * @par Description: 删除配对。
 * @param [in] addr 地址。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t aapi_sle_remove_paired_remote_device(const sle_addr_t *addr);
#ifdef __cplusplus
}
#endif
#endif  /* AAPI_SLE_SSAP_CLIENT_H */
/**
 * @}
 */
