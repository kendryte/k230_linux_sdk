/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: BTS GATT STRU module.
 */

/**
 * @defgroup bluetooth_bts_gatt_stru BTS GATT STRU API
 * @ingroup  bluetooth
 * @{
 */

#ifndef BTS_GATT_STRU_H
#define BTS_GATT_STRU_H

#include <stdint.h>
#include "errcode.h"
#include "bts_def.h"
#include "bts_gatt_stru.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Struct of gatt characteristic Properties bit field.
 * @else
 * @brief  gatt特征特性比特位属性定义。
 * @endif
 */
typedef enum {
    GATT_CHARACTER_PROPERTY_BIT_BROADCAST = 0x01,         /*!< @if Eng Broadcast characteristic value
                                                               @else   广播特征值 @endif */
    GATT_CHARACTER_PROPERTY_BIT_READ = 0x02,              /*!< @if Eng Read characteristic value
                                                               @else   读特征值 @endif */
    GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP = 0x04,      /*!< @if Eng Write characteristic value with no response
                                                               @else   写特征值并且不需要响应 @endif */
    GATT_CHARACTER_PROPERTY_BIT_WRITE = 0x08,             /*!< @if Eng Write characteristic value
                                                               @else   写特征值 @endif */
    GATT_CHARACTER_PROPERTY_BIT_NOTIFY = 0x10,            /*!< @if Eng Notify client characteristic value
                                                               @else   通知特征值 @endif */
    GATT_CHARACTER_PROPERTY_BIT_INDICATE = 0x20,          /*!< @if Eng Indicate characteristic value
                                                               @else   指示特征值 @endif */
    GATT_CHARACTER_PROPERTY_BIT_SIGNED_WRITE = 0x40,      /*!< @if Eng Signed write characteristic value
                                                               @else   签名写特征值 @endif */
    GATT_CHARACTER_PROPERTY_BIT_EXTENDED_PROPERTY = 0x80, /*!< @if Eng Define extern characteristic properties in
                                                                       Characteristic Extended Properties Descriptor
                                                               @else   在特征扩展特性描述符中定义了附加的特征特性
                                                               @endif */
} gatt_characteristic_property_t;

/**
 * @if Eng
 * @brief  Enum of gatt attribute permission.
 * @else
 * @brief  定义了当前Attribute的操作权限。
 * @endif
 */
typedef enum {
    GATT_ATTRIBUTE_PERMISSION_READ = 0x01,                /*!< @if Eng readable
                                                               @else   可读 @endif */
    GATT_ATTRIBUTE_PERMISSION_WRITE = 0x02,               /*!< @if Eng writable
                                                               @else   可写 @endif */
    GATT_ATTRIBUTE_PERMISSION_ENCRYPTION_NEED = 0x04,     /*!< @if Eng need encryption
                                                               @else   需要加密 @endif */
    GATT_ATTRIBUTE_PERMISSION_AUTHENTICATION_NEED = 0x08, /*!< @if Eng need authentication
                                                               @else   需要认证 @endif */
    GATT_ATTRIBUTE_PERMISSION_AUTHORIZATION_NEED = 0x10,  /*!< @if Eng need authorization
                                                               @else   需要授权 @endif */
    GATT_ATTRIBUTE_PERMISSION_MITM_NEED = 0x20,           /*!< @if Eng need MITM protection
                                                               @else   需要MITM保护 @endif */
} gatt_attribute_permission_t;

/**
 * @if Eng
 * @brief  Enum of gatt operation error code.
 * @else
 * @brief  定义gatt操作错误码。
 * @endif
 */
typedef enum {
    GATT_STATUS_SUCCESS = 0x00,                          /*!< @if Eng operation error code of success
                                                              @else   执行成功错误码 @endif */
    GATT_STATUS_INVALID_HANDLE = 0x01,                   /*!< @if Eng operation error code of invalid handle
                                                              @else   无效handle错误码 @endif */
    GATT_STATUS_READ_NOT_PERMITTED = 0x02,               /*!< @if Eng operation error code of read not permitted
                                                              @else   不可读错误码 @endif */
    GATT_STATUS_WRITE_NOT_PERMITTED = 0x03,              /*!< @if Eng operation error code of write not permitted
                                                              @else   不可写错误码 @endif */
    GATT_STATUS_INVALID_PDU = 0x04,                      /*!< @if Eng operation error code of invalid PDU
                                                              @else   无效PDU错误码 @endif */
    GATT_STATUS_INSUFFICIENT_AUTHENTICATION = 0x05,      /*!< @if Eng operation error code of insufficient authorization
                                                              @else   未认证的读写错误码 @endif */
    GATT_STATUS_REQUEST_NOT_SUPPORTED = 0x06,            /*!< @if Eng operation error code of not support the request
                                                                      by the client
                                                              @else 不支持client发送的请求 @endif */
    GATT_STATUS_INVALID_OFFSET = 0x07,                   /*!< @if Eng operation error code of invalid offset
                                                              @else   无效偏移的读写错误码 @endif */
    GATT_STATUS_INSUFFICIENT_AUTHORIZATION = 0x08,       /*!< @if Eng operation error code of insufficient
                                                                      authentication
                                                              @else   未授权错误码 @endif */
    GATT_STATUS_PREPARE_QUEUE_FULL = 0x09,               /*!< @if Eng operation error code of prepare queue full
                                                              @else   排队的prepare writes太多，队列已满错误码 @endif */
    GATT_STATUS_ATTRIBUTE_NOT_FOUND = 0x0A,              /*!< @if Eng operation error code of prepare queue full
                                                              @else   排队的prepare writes太多，队列已满错误码 @endif */
    GATT_STATUS_ATTRIBUTE_NOT_LONG = 0x0B,               /*!< @if Eng operation error code of can not use
                                                                      ATT_READ_BLOB_REQ PDU by not long attribute
                                                              @else   该attribute不是long attribute, 不能使用
                                                                      ATT_READ_BLOB_REQ PDU @endif */
    GATT_STATUS_INSUFFICIENT_ENCRYPTION_KEY_SIZE = 0x0C, /*!< @if Eng operation error code of the encryption key size is
                                                                      insufficient
                                                              @else 用于加密此连接的加密密钥大小不足@endif */
    GATT_STATUS_INVALID_ATTRIBUTE_VALUE_LENGTH = 0x0D,   /*!< @if Eng operation error code of the encryption key size is
                                                                      insufficient
                                                              @else   用于加密此连接的加密密钥大小不足@endif */
    GATT_STATUS_UNLIKELY_ERROR = 0x0E,                   /*!< @if Eng operation error code unlikely
                                                              @else   不可能的错误码@endif */
    GATT_STATUS_INSUFFICIENT_ENCRYPTION = 0x0F,          /*!< @if Eng operation error code of insufficient encryption
                                                              @else   加密不足错误码@endif */
    GATT_STATUS_UNSUPPORTED_GROUP_TYPE = 0x10,           /*!< @if Eng operation error code of unsupport grouping
                                                                      attribute type
                                                              @else   不支持的 grouping attribute错误码@endif */
    GATT_STATUS_INSUFFICIENT_RESOURCES = 0x11,           /*!< @if Eng operation error code of insufficient resource
                                                              @else   资源不足错误码@endif */
    GATT_STATUS_DATABASE_OUT_OF_SYNC = 0x12,             /*!< @if Eng operation error code of database out of sync
                                                              @else   服务端与客户端未同步数据库错误码@endif */
    GATT_STATUS_VALUE_NOT_ALLOWED = 0x13,                /*!< @if Eng operation error code of not allowed value
                                                              @else   非法的参数值错误码@endif */
} gatt_status_t;

/**
 * @if Eng
 * @brief  Max value of ble MTU.
 * @else
 * @brief  协议规定的BLE MTU最大值。
 * @endif
 */
#define SDK_BLE_MTU_MAX 517

/**
 * @if Eng
 * @brief  Min value of ble MTU.
 * @else
 * @brief  用户可配置的BLE MTU最小值。
 * @endif
 */
#define SDK_BLE_MTU_MIN 23

#ifdef __cplusplus
}
#endif
#endif
/** @} */

