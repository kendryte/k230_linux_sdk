/**
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 *
 * Description: SLE low latency.
 */

/**
 * @defgroup SLE low latency API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_LOW_LATENCY_H
#define SLE_LOW_LATENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "sle_errcode.h"
#include "sle_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  low latency support rate
 * @else
 * @brief  低时延调度速率。
 * @endif
 */
typedef enum {
    SLE_LOW_LATENCY_125HZ   = 125,  /*!< @if Eng 125HZ.
                                         @else 125HZ 调度。 @endif */
    SLE_LOW_LATENCY_250HZ   = 250,  /*!< @if Eng 250HZ.
                                         @else 250HZ 调度。 @endif */
    SLE_LOW_LATENCY_500HZ   = 500,  /*!< @if Eng 500HZ.
                                         @else 500HZ 调度。 @endif */
    SLE_LOW_LATENCY_1K      = 1000, /*!< @if Eng 1000HZ.
                                         @else 1000Hz 调度。 @endif */
    SLE_LOW_LATENCY_2K      = 2000, /*!< @if Eng 2000HZ.
                                         @else 2000HZ 调度。 @endif */
    SLE_LOW_LATENCY_4K      = 4000, /*!< @if Eng 4000HZ.
                                         @else 4000Hz 调度。 @endif */
    SLE_LOW_LATENCY_8K      = 8000, /*!< @if Eng 8000HZ.
                                         @else 8000Hz 调度。 @endif */
    SLE_LOW_LATENCY_INVALID = 0,
} sle_low_latency_rate_t;

/**
 * @if Eng
 * @brief  low latency enable.
 * @else
 * @brief  低时延使能。
 * @endif
 */
typedef enum {
    SLE_LOW_LATENCY_DISABLE = 0, /*!< @if Eng close low latency
                                      @else 关闭低时延调度 @endif */
    SLE_LOW_LATENCY_ENABLE       /*!< @if Eng open low latency
                                      @else 打开低时延 @endif */
} sle_low_latency_status_t;

/**
 * @if Eng
 * @brief  low latency enable.
 * @else
 * @brief  低时延使能。
 * @endif
 */
typedef enum {
    SLE_LOW_LATENCY_VALUE_GET_SUCCESS = 0, /*!< @if Eng get mouse value success
                                                @else 获取鼠标数据成功 @endif */
    SLE_LOW_LATENCY_VALUE_GET_FAIL        /*!< @if Eng get mouse value fail，No data is sent after failure
                                               @else 获取鼠标数据失败, 失败后不会发送数据 @endif */
} sle_low_latency_value_set_status_t;

/**
 * @brief  星闪层低时延配置接口
 */
typedef struct {
    uint16_t conn_id; /*!< @if Eng connection ID.
                           @else 连接ID @endif */
    uint8_t  enable;  /*!< @if Eng low latency enable, { @ref sle_low_latency_rate_t }.
                           @else 低时延使能状态, { @ref sle_low_latency_rate_t } @endif */
    uint8_t  rate;    /*!< @if Eng low latency rate, { @ref sle_low_latency_rate_t }.
                           @else 低时延调度速率, { @ref sle_low_latency_rate_t }  @endif */
} sle_set_acb_low_latency_t;

/**
 * @if Eng
 * @brief The callback interface for sending data in mouse mode.
 * @param [out] button_mask button value.
 * @param [out] x Mouse x-coordinate.
 * @param [out] y Mouse y-coordinate.
 * @param [out] wheel Mouse wheel
 * @retval error code, { @ref sle_low_latency_value_set_status_t }
 * @else
 * @brief Mouse模式数据发送回调接口定义。
 * @param [out] button_mask 按键值.
 * @param [out] x 鼠标X坐标.
 * @param [out] y 鼠标Y坐标.
 * @param [out] wheel Mouse 滚轮
 * @retval error code, { @ref sle_low_latency_value_set_status_t } .
 * @endif
 */
typedef errcode_t (*low_latency_key_value_set_callback)(int8_t *button_mask, int16_t *x, int16_t *y, int8_t *wheel);

/**
 * @if Eng
 * @brief The general callback interface for sending TLV data for HID device.
 * @param [inout] len Data length. As an input parameter, len limits the max data length that can send.
 *                    Meanwhile as an output parameter, len means length of data to be sent.
 *                    User should use the input length first to check the length of user data to be sent.
 * @retval Pointer to user data. If pointer is NULL, something wrong happened, then send data.
 * @else
 * @brief 设置HID设备TLV数据的通用接口。
 * @param [inout] len 数据长度。当作为入参时，len解引用后表示最大支持的数据长度。当作为出参时，表示用户需要发送的数据长度。
 * @retval 用户数据指针。如果返回值为NULL，数据不会被发送，否则指针指向的数据会被发送。
 * @endif
 */
typedef uint8_t *(*low_latency_general_tx_callback)(uint8_t *len);

/**
 * @if Eng
 * @brief Definition of the low-latency mouse callback function interface.
 * @else
 * @brief Mouse侧发送low latency数据回调。
 * @endif
 */
typedef struct {
    low_latency_key_value_set_callback set_value_cb;  /*!< @if Eng Set key value and coordinate data callback.
                                                           @else 设置键值坐标数据回调函数。 @endif */
} sle_low_latency_mouse_callbacks_t;

/**
 * @if Eng
 * @brief Definition of the low-latency mouse callback function interface.
 * @else
 * @brief TX侧发送low latency数据回调。
 * @endif
 */
typedef struct {
    low_latency_general_tx_callback low_latency_tx_cb;  /*!< @if Eng General callback for setting TX data.
                                                             @else 设置TX数据的通用接口。 @endif */
} sle_low_latency_tx_callbacks_t;

/**
 * @if Eng
 * @brief  Mouse mode initialization.
 * @par Description: SLE low-latency mouse enable.
 * @retval error code.
 * @else
 * @brief  Mouse模式初始化。
 * @par Description: SLE 低时延鼠标使能。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_mouse_enable(void);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in Mouse Mode.
 * @par Description: Low-Latency Mouse Callback Method Registration.
 * @param [in] func Callback function。
 * @retval error code.
 * @else
 * @brief  Mouse模式数据获取调定义。
 * @par Description: 低时延鼠标回调方法注册。
 * @param [in] func 回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_mouse_register_callbacks(sle_low_latency_mouse_callbacks_t *mouse_cbk);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in Mouse Mode.
 * @par Description: Low-Latency Mouse Callback Method Registration.
 * @param [in] func Callback function。
 * @retval error code.
 * @else
 * @brief  Mouse模式数据获取调定义。
 * @par Description: 低时延鼠标回调方法注册。
 * @param [in] func 回调函数。
 * @retval error code.
 * @endif
 */
typedef void(*low_latency_report_callback)(uint8_t *data, uint8_t len);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking for general using.
 * @param [in] len   data length.
 * @param [in] value data.
 * @retval error code.
 * @else
 * @brief  通用获取数据回调定义。
 * @param [in] len   数据长度。
 * @param [in] value 数据。
 * @retval error code.
 * @endif
 */
typedef void(*low_latency_general_rx_callback)(uint8_t len, uint8_t *value);

/**
 * @if Eng
 * @brief Callback function for dongle to receive data.
 * @else
 * @brief Dongle侧接收low latency数据回调。
 * @endif
 */
typedef struct {
    low_latency_report_callback report_cb;           /*!< @if Eng Get key value and coordinate data callback.
                                                          @else 接收键值坐标数据回调函数。 @endif */
} sle_low_latency_dongle_callbacks_t;

/**
 * @if Eng
 * @brief Callback function for rx side to receive data.
 * @else
 * @brief RX侧接收low latency数据回调。
 * @endif
 */
typedef struct {
    low_latency_general_rx_callback low_latency_rx_cb; /*!< @if Eng General callback for getting RX data.
                                                            @else 接收RX数据的通用接口。 @endif */
} sle_low_latency_rx_callbacks_t;

/**
 * @if Eng
 * @brief  Mouse mode initialization.
 * @par Description: SLE low-latency dongle enable.
 * @attention NULL
 * @retval error code.
 * @else
 * @brief  Mouse模式初始化。
 * @par Description: SLE 低时延Dongle使能。
 * @attention NULL
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_dongle_enable(void);

/**
 * @if Eng
 * @brief  Low-Latency Enabling and Scheduling Parameter Configurationn.
 * @par Description: Low-Latency Enabling and Scheduling Parameter Configuration.
 * @param [in] conn_id  Connect Handle.
 * @param [in] enable   Enable or disable low latency. { @ref sle_low_latency_status_t }
 * @param [in] rate     Report rate. { @ref sle_low_latency_rate_t }.
 * @retval error code.
 * @else
 * @brief  低时延使能和调度参数配置。
 * @par Description: 低时延使能和调度参数配置。
 * @param [in] conn_id  连接句柄.
 * @param [in] enable   是否使能低时延. { @ref sle_low_latency_status_t }
 * @param [in] rate     回报率. { @ref sle_low_latency_rate_t }.
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_set(uint16_t conn_id, uint8_t enable, uint16_t rate);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in Mouse Mode.
 * @par Description: Low-Latency Mouse Callback Method Registration.
 * @param [in] func Callback function。
 * @retval error code.
 * @else
 * @brief  Dongle模式数据获取回调定义。
 * @par Description: 低时延Dongle回调方法注册。
 * @param [in] func 回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_dongle_register_callbacks(sle_low_latency_dongle_callbacks_t *dongle_cbk);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in TX Mode.
 * @par Description: Low-Latency TX Callback Method Registration.
 * @param [in] func Callback function。
 * @retval error code.
 * @else
 * @brief  TX模式数据获取回调定义。
 * @par Description: 低时延TX回调方法注册。
 * @param [in] func 回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_tx_register_callbacks(sle_low_latency_tx_callbacks_t *tx_cbk);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in RX Mode.
 * @par Description: Low-Latency RX Callback Method Registration.
 * @param [in] func Callback function。
 * @retval error code.
 * @else
 * @brief  RX模式数据获取回调定义。
 * @par Description: 低时延RX回调方法注册。
 * @param [in] func 回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_rx_register_callbacks(sle_low_latency_rx_callbacks_t *rx_cbk);

/**
 * @if Eng
 * @brief  TX mode initialization.
 * @par Description: SLE low-latency TX enable.
 * @retval error code.
 * @else
 * @brief  TX模式初始化。
 * @par Description: SLE低时延TX使能。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_tx_enable(void);

/**
 * @if Eng
 * @brief  RX mode initialization.
 * @par Description: SLE low-latency RX enable.
 * @retval error code.
 * @else
 * @brief  RX模式初始化。
 * @par Description: SLE低时延RX使能。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_rx_enable(void);

/**
 * @if Eng
 * @brief Definition of the low-latency mouse callback function interface.
 * @else
 * @brief Mouse侧发送low latency数据回调。
 * @endif
 */
typedef uint8_t *(*sle_low_latency_hid_data_callback)(uint8_t *length, uint16_t *ssap_handle,
    uint8_t *data_type, uint16_t co_handle);

/**
 * @if Eng
 * @brief Callback invoked when ble em data is set.
 * @par Callback invoked when ble em data is set.
 * @attention 1.This function is called in SLE service context, should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] co_handle  connection handle.
 * @param [in] status     error code.
 * @retval #void no return value.
 * @else
 * @brief  设置ble em数据的回调函数。
 * @par    设置ble em数据的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] co_handle  连接句柄。
 * @param [in] status     执行结果错误码。
 * @retval 无返回值。
 * @endif
 */
typedef void (*sle_low_latency_set_em_data_callback)(uint16_t co_handle, uint8_t status);

/**
 * @if Eng
 * @brief Struct of BLE low latency callback function.
 * @else
 * @brief BLE low latency 管理回调函数接口定义。
 * @endif
 */
typedef struct {
    sle_low_latency_hid_data_callback hid_data_cb;              /*!< @if Eng BLE low latency get data callback.
                                                                     @else   BLE低时延数据获取回调函数。 @endif */
    sle_low_latency_set_em_data_callback sle_set_em_data_cb;    /*!< @if Eng Set em data callback.
                                                                     @else   设置em数据回调函数。 @endif */
} sle_low_latency_callbacks_t;

/**
 * @if Eng
 * @brief  SLE low latency callbacks register.
 * @par Description: SLE low latency callbacks register.
 * @param [in] cbks  Callback functions.
 * @retval error code.
 * @else
 * @brief  注册SLE低时延回调。
 * @par Description: 注册SLE低时延回调。
 * @param [in] cbks  回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_register_callbacks(sle_low_latency_callbacks_t *cbks);

/**
 * @if Eng
 * @brief  SLE set low latency mode.
 * @par Description: SLE set low latency mode.
 * @param [in] co_handle  connection handle.
 * @param [in] enable  enable or disable.
 * @retval error code.
 * @else
 * @brief  SLE设置低时延模式。
 * @par Description: SLE设置低时延模式。
 * @param [in] co_handle  connection handle.
 * @param [in] enable  enable or disable.
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_set_em_data(uint16_t co_handle, uint8_t enable);

#ifdef __cplusplus
}
#endif
#endif /* SLE_LOW_LATENCY_H */
/**
 * @}
 */
