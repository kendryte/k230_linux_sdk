/*
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 * Description: BT PUBLIC API module.
 */

/**
 * @defgroup bluetooth_bts_def PUBLIC API
 * @ingroup  bluetooth
 * @{
 */

#ifndef BTS_DEF_H
#define BTS_DEF_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Length of bluetooth device address.
 * @else
 * @brief  蓝牙设备的地址长度。
 * @endif
 */
#define BD_ADDR_LEN 6

/**
 * @if Eng
 * @brief  Max length of bluetooth uuid in octets.
 * @else
 * @brief  蓝牙UUID的最大长度，单位字节。
 * @endif
 */
#define BT_UUID_MAX_LEN 16

/**
 * @if Eng
 * @brief  Max length of bluetooth device name.
 * @else
 * @brief  蓝牙设备名称的最大长度，单位字节。
 * @endif
 */
#define BT_DEVICE_NAME_MAX_LEN 32

/**
 * @if Eng
 * @brief  Min value of ble error code.
 * @else
 * @brief  蓝牙错误状态类型最小值。
 * @endif
 */
#define ERRCODE_BT_COMMON_BASE               0x80006000
/**
 * @if Eng
 * @brief  Min value of ble error code.
 * @else
 * @brief  蓝牙错误状态类型最大值。
 * @endif
 */
#define ERRCODE_BT_COMMON_END                 0x800067FF

/**
 * @if Eng
 * @brief  Enum of uuid type.
 * @else
 * @brief  定义UUID的类型。
 * @endif
 */
typedef enum {
    UUID_TYPE_NULL = 0x00, /*!< @if Eng null uuid. @else  空uuid。 @endif */
    UUID_TYPE_16_BIT,      /*!< @if Eng uuid type of 16 bit. @else  16bit长度的uuid。 @endif */
    UUID_TYPE_32_BIT,      /*!< @if Eng uuid type of 32 bit. @else  32bit长度的uuid。 @endif */
    UUID_TYPE_128_BIT,     /*!< @if Eng uuid type of 128 bit. @else  128bit长度的uuid。 @endif */
} uuid_type_t;

/**
 * @if Eng
 * @brief  Enum of bt status error code.
 * @else
 * @brief  定义蓝牙错误状态类型。
 * @endif
 */
typedef enum {
    ERRCODE_BT_SUCCESS = 0x00,                /*!< @if Eng error code of success
                                                   @else   执行成功错误码 @endif */
    ERRCODE_BT_FAIL = ERRCODE_BT_COMMON_BASE, /*!< @if Eng error code of failure
                                                   @else   执行失败错误码 @endif */
    ERRCODE_BT_NOT_READY,                     /*!< @if Eng error code of not ready
                                                   @else   执行状态未就绪错误码 @endif */
    ERRCODE_BT_MALLOC_FAIL,                   /*!< @if Eng error code of memmery not enough
                                                   @else   内存不足错误码 @endif */
    ERRCODE_BT_MEMCPY_FAIL,                   /*!< @if Eng error code of memcpy fail
                                                   @else   内存拷贝错误错误码 @endif */
    ERRCODE_BT_BUSY,                          /*!< @if Eng error code of system is busy
                                                   @else   繁忙无法响应错误码 @endif */
    ERRCODE_BT_DONE,                          /*!< @if Eng error code of complete
                                                   @else   执行完成错误码 @endif */
    ERRCODE_BT_UNSUPPORTED,                   /*!< @if Eng error code of not support
                                                   @else   不支持错误码 @endif */
    ERRCODE_BT_PARAM_ERR,                     /*!< @if Eng error code of invalid param
                                                   @else   无效参数错误码 @endif */
    ERRCODE_BT_STATE_ERR,                     /*!< @if Eng error code of state error
                                                   @else   状态错误 @endif */
    ERRCODE_BT_UNHANDLED,                     /*!< @if Eng error code of unhandle
                                                   @else   未处理错误码 @endif */
    ERRCODE_BT_AUTH_FAIL,                     /*!< @if Eng error code of auth failure
                                                   @else   鉴权失败错误码 @endif */
    ERRCODE_BT_RMT_DEV_DOWN,                  /*!< @if Eng error code of remote device down
                                                   @else   远端设备关闭错误码 @endif */
    ERRCODE_BT_AUTH_REJECTED,                 /*!< @if Eng error code of auth rejected
                                                   @else   鉴权被拒错误码 @endif */
    ERRCODE_BT_OUT_OF_RANGE,                  /*!< @if Eng error code of param out of range
                                                   @else   参数超出范围错误码 @endif */
    ERRCODE_BT_MAX = ERRCODE_BT_COMMON_END    /*!< @if Eng the max of bt error code
                                                   @else   蓝牙错误码最大值 @endif */
} errcode_bt_t;

/**
 * @if Eng
 * @brief  Enum of bluetooth device address type.
 * @else
 * @brief  定义设备的蓝牙地址类型。
 * @endif
 */
typedef enum {
    BT_ADDRESS_TYPE_PUBLIC_DEVICE_ADDRESS          =  0x00, /*!< @if Eng bluetooth public address.
                                                                 @else   蓝牙公有地址。 @endif */
    BT_ADDRESS_TYPE_RANDOM_DEVICE_ADDRESS          =  0x01, /*!< @if Eng bluetooth random address.
                                                                 @else   蓝牙随机地址。 @endif */
} bt_addr_type;

/**
 * @if Eng
 * @brief  Struct of bluetooth device address.
 * @else
 * @brief  定义设备的蓝牙地址。
 * @endif
 */
typedef struct {
    uint8_t addr[BD_ADDR_LEN];     /*!< @if Eng bluetooth device address.
                                        @else   蓝牙地址。 @endif */
    uint8_t type;                  /*!< @if Eng bluetooth device address type, See { @ref bt_addr_type }.
                                        @else   蓝牙地址类型，参考 { @ref bt_addr_type }。 @endif */
} bd_addr_t;

/**
 * @if Eng
 * @brief  Struct of UUID.
 * @else
 * @brief  定义UUID结构。
 * @endif
 */
typedef struct {
    uint8_t uuid_len;               /*!< @if Eng length of uuid.
                                        @else    UUID长度。 @endif */
    uint8_t uuid[BT_UUID_MAX_LEN]; /*!< @if Eng uuid.
                                        @else    UUID字段。 @endif */
} bt_uuid_t;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  /* end of bts_def.h */

