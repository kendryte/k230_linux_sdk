/**
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 *
 * Description: SLE error code.
 */

/**
 * @defgroup sle_error_code Error Code API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_ERRCODE_H
#define SLE_ERRCODE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @if Eng
 * @brief  SLE error code base.
 * @else
 * @brief  SLE 错误码起始。
 * @endif
 */
#define ERRCODE_SLE_COMMON_BASE 0x80006000

/**
 * @if Eng
 * @brief  SLE SSAP error code base.
 * @else
 * @brief  SLE SSAP 错误码起始。
 * @endif
 */
#define ERRCODE_SLE_SSAP_BASE   0x80006100

/**
 * @if Eng
 * @brief  SLE SSAP error code end.
 * @else
 * @brief  SLE SSAP 错误码结束。
 * @endif
 */
#define ERRCODE_SLE_SSAP_END    0x800061FF

/**
 * @if Eng
 * @brief  SLE error code base.
 * @else
 * @brief  SLE 错误码结束。
 * @endif
 */
#define ERRCODE_SLE_COMMON_END  0x800067FF

/**
 * @if Eng
 * @brief  SLE error code.
 * @else
 * @brief  SLE 错误码（非协议相关）。
 * @endif
 */
typedef enum {
    ERRCODE_SLE_SUCCESS = 0,                               /*!< @if Eng error code of success
                                                                @else   执行成功错误码 @endif */
    ERRCODE_SLE_CONTINUE = ERRCODE_SLE_COMMON_BASE,        /*!< @if Eng error code of continue
                                                                @else   继续执行错误码 @endif */
    ERRCODE_SLE_DIRECT_RETURN,                             /*!< @if Eng error code of direct return
                                                                @else   直接返回错误码 @endif */
    ERRCODE_SLE_NO_ATTATION,                               /*!< @if Eng error code of no attention
                                                                @else   错误码 @endif */
    ERRCODE_SLE_PARAM_ERR,                                 /*!< @if Eng error code of parameter error
                                                                @else   参数错误错误码 @endif */
    ERRCODE_SLE_FAIL,                                      /*!< @if Eng error code of configure fail
                                                                @else   配置失败错误码 @endif */
    ERRCODE_SLE_TIMEOUT,                                   /*!< @if Eng error code of configure timeout
                                                                @else   配置超时错误码 @endif */
    ERRCODE_SLE_UNSUPPORTED,                               /*!< @if Eng error code of unsupported parameter
                                                                @else   参数不支持错误码 @endif */
    ERRCODE_SLE_GETRECORD_FAIL,                            /*!< @if Eng error code of get current record fail
                                                                @else   获取当前记录失败错误码 @endif */
    ERRCODE_SLE_POINTER_NULL,                              /*!< @if Eng error code of pointer is NULL
                                                                @else   指针为空错误码 @endif */
    ERRCODE_SLE_NO_RECORD,                                 /*!< @if Eng error code of no record return
                                                                @else   无记录返回错误码 @endif */
    ERRCODE_SLE_STATUS_ERR,                                /*!< @if Eng error code of state error
                                                                @else   状态错误错误码 @endif */
    ERRCODE_SLE_NOMEM,                                     /*!< @if Eng error code of no memory
                                                                @else   内存不足错误码 @endif */
    ERRCODE_SLE_AUTH_FAIL,                                 /*!< @if Eng error code of authentication failure
                                                                @else   认证失败错误码 @endif */
    ERRCODE_SLE_AUTH_PKEY_MISS,                            /*!< @if Eng error code of authentication fail due to pin
                                                                        code or key lost
                                                                @else   PIN码或密钥丢失致认证失败错误码 @endif */
    ERRCODE_SLE_RMT_DEV_DOWN,                              /*!< @if Eng error code of remote device down
                                                                @else   对端设备关闭错误码 @endif */
    ERRCODE_SLE_PAIRING_REJECT,                            /*!< @if Eng error code of pair reject
                                                                @else   配对拒绝错误码 @endif */
    ERRCODE_SLE_BUSY,                                      /*!< @if Eng error code of system busy
                                                                @else   系统繁忙错误码 @endif */
    ERRCODE_SLE_NOT_READY,                                 /*!< @if Eng error code of system not ready
                                                                @else   系统未准备好错误码 @endif */
    ERRCODE_SLE_CONN_FAIL,                                 /*!< @if Eng error code of connect fail
                                                                @else   连接失败错误码 @endif */
    ERRCODE_SLE_OUT_OF_RANGE,                              /*!< @if Eng error code of out of range
                                                                @else   越界错误码 @endif */
    ERRCODE_SLE_MEMCPY_FAIL,                               /*!< @if Eng error code of memcpy fail
                                                                @else   拷贝失败错误码 @endif */
    ERRCODE_SLE_MALLOC_FAIL,                               /*!< @if Eng error code of malloc fail
                                                                @else   内存申请失败错误码 @endif */
    ERRCODE_SLE_MAX = ERRCODE_SLE_COMMON_END               /*!< @if Eng maximum of SLE error code
                                                                @else   SLE 错误码最大值 @endif */
} errcode_sle_t;

/**
 * @if Eng
 * @brief  SLE SSAP protocol error code.
 * @else
 * @brief  SLE SSAP 协议错误码。
 * @endif
 */
typedef enum {
    ERRCODE_SSAP_INVALID_PDU = ERRCODE_SLE_SSAP_BASE + 0x01,                      /*!< @if Eng invalid PDU
                                                                                       @else   服务端接收的 PDU 无效
                                                                                       @endif */
    ERRCODE_SSAP_PDU_NOT_SUPPORT = ERRCODE_SLE_SSAP_BASE + 0x02,                  /*!< @if Eng PDU not support
                                                                                       @else   服务端不支持处理接收的
                                                                                               PDU
                                                                                       @endif */
    ERRCODE_SSAP_UNKNOW = ERRCODE_SLE_SSAP_BASE + 0x03,                           /*!< @if Eng PDU unknow
                                                                                       @else   服务端执行请求时发生
                                                                                               未知错误
                                                                                       @endif */
    ERRCODE_SSAP_INVALID_HANDLE = ERRCODE_SLE_SSAP_BASE + 0x04,                   /*!< @if Eng invalid handle
                                                                                       @else   请求中的句柄无效
                                                                                       @endif */
    ERRCODE_SSAP_INSUFFICIENT_RESOURCES = ERRCODE_SLE_SSAP_BASE + 0x05,           /*!< @if Eng insufficient resources
                                                                                       @else   服务端没有足够资源完成
                                                                                               请求
                                                                                       @endif */
    ERRCODE_SSAP_PROHIBIT_READING = ERRCODE_SLE_SSAP_BASE + 0x06,                 /*!< @if Eng prohibit reading
                                                                                       @else   服务端禁止客户端读取值
                                                                                       @endif */
    ERRCODE_SSAP_PROHIBIT_WRITE = ERRCODE_SLE_SSAP_BASE + 0x07,                   /*!< @if Eng prohibit writing
                                                                                       @else   服务端禁止客户端写入值
                                                                                       @endif */
    ERRCODE_SSAP_CLIENT_NOT_AUTHENTICATED = ERRCODE_SLE_SSAP_BASE + 0x08,         /*!< @if Eng client no authentication
                                                                                       @else   客户端未经过认证
                                                                                       @endif */
    ERRCODE_SSAP_CLIENT_NOT_AUTHORIZATION = ERRCODE_SLE_SSAP_BASE + 0x09,         /*!< @if Eng client no authorizaion
                                                                                       @else   客户端未被授权
                                                                                       @endif */
    ERRCODE_SSAP_BEARER_NOT_ENCRYPTED = ERRCODE_SLE_SSAP_BASE + 0x0A,             /*!< @if Eng bearer not encrypted
                                                                                       @else   传输 PDU 的承载未加密
                                                                                       @endif */
    ERRCODE_SSAP_ENTRIES_NOT_FOUND = ERRCODE_SLE_SSAP_BASE + 0x0B,                /*!< @if Eng entries not found
                                                                                       @else   服务端未找到对应条目
                                                                                       @endif */
    ERRCODE_SSAP_DATA_NOT_FOUND = ERRCODE_SLE_SSAP_BASE + 0x0C,                   /*!< @if Eng data not found
                                                                                       @else   服务端未找到对应类型数据
                                                                                       @endif */
    ERRCODE_SSAP_INCORRECT_DATA_TYPE = ERRCODE_SLE_SSAP_BASE + 0x0D,              /*!< @if Eng data type incorrected
                                                                                       @else   客户端发送写入数据类型
                                                                                               不符的错误
                                                                                       @endif */
    ERRCODE_SSAP_INCORRECT_DATA_VALUE = ERRCODE_SLE_SSAP_BASE + 0x0E,             /*!< @if Eng data value incorrected
                                                                                       @else   客户端发送写入值不符
                                                                                               的错误
                                                                                       @endif */
    ERRCODE_SSAP_VALUE_OUT_OF_RANGE = ERRCODE_SLE_SSAP_BASE + 0x0F,               /*!< @if Eng data value incorrected
                                                                                       @else   客户端写入的值超出范围
                                                                                       @endif */
    ERRCODE_SSAP_UPPERLAYER_APPLICATION_ERROR_MIN = ERRCODE_SLE_SSAP_BASE + 0xAF, /*!< @if Eng reserve
                                                                                       @else   预留给上层协议定义应用
                                                                                               错误
                                                                                       @endif */
    ERRCODE_SSAP_UPPERLAYER_APPLICATION_ERROR_MAX = ERRCODE_SLE_SSAP_BASE + 0xFF, /*!< @if Eng reserve
                                                                                       @else   预留给上层协议定义应用
                                                                                               错误
                                                                                       @endif */
} errcode_sle_ssap_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* SLE_ERRCODE_H */
/**
 * @}
 */
