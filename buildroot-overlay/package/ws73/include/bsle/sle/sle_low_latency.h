/**
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 *
 * Description: SLE low latency.
 */

/**
 * @defgroup sle_low_latency API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_LOW_LATENCY_H
#define SLE_LOW_LATENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "errcode.h"
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
    SLE_LOW_LATENCY_125HZ = 0,  /*!< @if Eng 125HZ.
                                     @else 125HZ 调度。 @endif */
    SLE_LOW_LATENCY_500HZ,      /*!< @if Eng 500HZ.
                                     @else 500HZ 调度。 @endif */
    SLE_LOW_LATENCY_1K,         /*!< @if Eng 1000HZ.
                                     @else 1000Hz 调度。 @endif */
    SLE_LOW_LATENCY_2K,         /*!< @if Eng 2000HZ.
                                     @else 2000HZ 调度。 @endif */
    SLE_LOW_LATENCY_3K,         /*!< @if Eng 3000HZ.
                                     @else 3000HZ 调度。 @endif */
    SLE_LOW_LATENCY_4K,         /*!< @if Eng 4000HZ.
                                     @else 4000Hz 调度。 @endif */
    SLE_LOW_LATENCY_5K,         /*!< @if Eng 5000HZ.
                                     @else 5000Hz 调度。 @endif */
    SLE_LOW_LATENCY_6K,         /*!< @if Eng 6000HZ.
                                     @else 6000Hz 调度。 @endif */
    SLE_LOW_LATENCY_7K,         /*!< @if Eng 7000HZ.
                                     @else 7000Hz 调度。 @endif */
    SLE_LOW_LATENCY_8K,         /*!< @if Eng 8000HZ.
                                     @else 8000Hz 调度。 @endif */
    SLE_LOW_LATENCY_MAX,
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
 * @param  [out] button_mask button value.
 * @param  [out] x Mouse x-coordinate.
 * @param  [out] y Mouse y-coordinate.
 * @param  [out] wheel Mouse wheel
 * @retval error code, { @ref sle_low_latency_value_set_status_t }
 * @else
 * @brief Mouse模式数据发送回调接口定义。
 * @param  [out] button_mask 按键值.
 * @param  [out] x 鼠标X坐标.
 * @param  [out] y 鼠标Y坐标.
 * @param  [out] wheel Mouse 滚轮
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
 * @param  [in]  mouse_cbk Callback function.
 * @retval error code.
 * @else
 * @brief  Mouse模式数据获取调定义。
 * @par Description: 低时延鼠标回调方法注册。
 * @param  [in]  mouse_cbk 回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_mouse_register_callbacks(sle_low_latency_mouse_callbacks_t *mouse_cbk);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in Mouse Mode.
 * @par Description: Low-Latency Mouse Callback Method Registration.
 * @param  [in]  data Callback function。
 * @retval error code.
 * @else
 * @brief  Mouse模式数据获取调定义。
 * @par Description: 低时延鼠标回调方法注册。
 * @param  [in]  data 回调函数。
 * @retval error code.
 * @endif
 */
typedef void(*low_latency_report_callback)(uint8_t *data, uint8_t len);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking for general using.
 * @param  [in]  len   data length.
 * @param  [in]  value data.
 * @retval error code.
 * @else
 * @brief  通用获取数据回调定义。
 * @param  [in]  len   数据长度。
 * @param  [in]  value 数据。
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
 * @param  [in]  conn_id  Connect Handle.
 * @param  [in]  enable   Enable or disable low latency. { @ref sle_low_latency_status_t }
 * @param  [in]  rate     Report rate. { @ref sle_low_latency_rate_t }.
 * @retval error code.
 * @else
 * @brief  低时延使能和调度参数配置。
 * @par Description: 低时延使能和调度参数配置。
 * @param  [in]  conn_id  连接句柄.
 * @param  [in]  enable   是否使能低时延. { @ref sle_low_latency_status_t }
 * @param  [in]  rate     回报率. { @ref sle_low_latency_rate_t }.
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_set(uint16_t conn_id, uint8_t enable, uint16_t rate);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in Mouse Mode.
 * @par Description: Low-Latency Mouse Callback Method Registration.
 * @param  [in]  dongle_cbk Callback function。
 * @retval error code.
 * @else
 * @brief  Dongle模式数据获取回调定义。
 * @par Description: 低时延Dongle回调方法注册。
 * @param  [in]  dongle_cbk 回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_dongle_register_callbacks(sle_low_latency_dongle_callbacks_t *dongle_cbk);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in TX Mode.
 * @par Description: Low-Latency TX Callback Method Registration.
 * @param  [in]  tx_cbk Callback function。
 * @retval error code.
 * @else
 * @brief  TX模式数据获取回调定义。
 * @par Description: 低时延TX回调方法注册。
 * @param  [in]  tx_cbk 回调函数。
 * @retval error code.
 * @endif
 */
errcode_t sle_low_latency_tx_register_callbacks(sle_low_latency_tx_callbacks_t *tx_cbk);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking in RX Mode.
 * @par Description: Low-Latency RX Callback Method Registration.
 * @param  [in]  rx_cbk Callback function。
 * @retval error code.
 * @else
 * @brief  RX模式数据获取回调定义。
 * @par Description: 低时延RX回调方法注册。
 * @param  [in]  rx_cbk 回调函数。
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

#ifdef __cplusplus
}
#endif
#endif /* SLE_LOW_LATENCY_H */
/**
 * @}
 */
