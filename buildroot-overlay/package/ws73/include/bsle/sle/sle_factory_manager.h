/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE Factory Manager module.
 */

/**
 * @defgroup sle_factory_manager factory manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_FACTORY_MANAGER_H
#define SLE_FACTORY_MANAGER_H
 
#include <stdint.h>
#include "errcode.h"
 
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  radio frequency power level.
 * @else
 * @brief  射频功率等级。
 * @endif
 */
typedef enum {
    SLE_RF_POWER_LEVEL_0 = 0x00,    /*!< @if Eng radio frequency power 0 level:-14dbm
                                         @else   射频功率0等级:-14dbm @endif */
    SLE_RF_POWER_LEVEL_1 = 0x01,    /*!< @if Eng radio frequency power 1 level:-10dbm
                                         @else   射频功率1等级:-10dbm @endif */
    SLE_RF_POWER_LEVEL_2 = 0x02,    /*!< @if Eng radio frequency power 2 level:-6dbm
                                         @else   射频功率2等级:-6dbm @endif */
    SLE_RF_POWER_LEVEL_3 = 0x03,    /*!< @if Eng radio frequency power 3 level:-2dbm
                                         @else   射频功率3等级:-2dbm @endif */
    SLE_RF_POWER_LEVEL_4 = 0x04,    /*!< @if Eng radio frequency power 4 level:2dbm
                                         @else   射频功率4等级:2dbm @endif */
    SLE_RF_POWER_LEVEL_5 = 0x05,    /*!< @if Eng radio frequency power 5 level:6dbm
                                         @else   射频功率5等级:6dbm @endif */
} sle_rf_power_t;

/**
 * @if Eng
 * @brief  radio frequency payload type.
 * @else
 * @brief  信息体类型。
 * @endif
 */
typedef enum {
    SLE_RF_PATLOAD_TYPE_0 = 0x00,    /*!< @if Eng radio frequency payload type:PRBS9
                                          @else   射频信息体类型:PRBS9 @endif */
    SLE_RF_PATLOAD_TYPE_1 = 0x01,    /*!< @if Eng radio frequency payload type:11110000
                                          @else   射频信息体类型:11110000 @endif */
    SLE_RF_PATLOAD_TYPE_2 = 0x02,    /*!< @if Eng radio frequency payload type:10101010
                                          @else   射频信息体类型:10101010 @endif */
    SLE_RF_PATLOAD_TYPE_3 = 0x03,    /*!< @if Eng radio frequency payload type:PRBS15
                                          @else   射频信息体类型:PRBS15 @endif */
    SLE_RF_PATLOAD_TYPE_4 = 0x04,    /*!< @if Eng radio frequency payload type:11111111
                                          @else   射频信息体类型:11111111 @endif */
    SLE_RF_PATLOAD_TYPE_5 = 0x05,    /*!< @if Eng radio frequency payload type:00000000
                                          @else   射频信息体类型:00000000 @endif */
    SLE_RF_PATLOAD_TYPE_6 = 0x06,    /*!< @if Eng radio frequency payload type:00001111
                                          @else   射频信息体类型:00001111 @endif */
    SLE_RF_PATLOAD_TYPE_7 = 0x07,    /*!< @if Eng radio frequency payload type:01010101
                                          @else   射频信息体类型:01010101 @endif */
} sle_rf_payload_type_t;

/**
 * @if Eng
 * @brief  radio frequency physical.
 * @else
 * @brief  射频物理层。
 * @endif
 */
typedef enum {
    SLE_RF_PHY_1M = 0x00,      /*!< @if Eng radio frequency physical 1M
                                         @else   射频物理层1M @endif */
    SLE_RF_PHY_2M = 0x01,      /*!< @if Eng radio frequency physical 2M
                                         @else   射频物理层2M @endif */
    SLE_RF_PHY_4M = 0x04,      /*!< @if Eng radio frequency physical 4M
                                         @else   射频物理层4M @endif */
} sle_rf_phy_t;

/**
 * @if Eng
 * @brief  radio frequency format.
 * @else
 * @brief  射频物理层。
 * @endif
 */
typedef enum {
    SLE_RF_FORMAT_FRAME_TYPE_1 = 0x00,      /*!< @if Eng radio frequency format: Wireless Frame Type 1
                                                 @else   射频格式:无线帧类型1 @endif */
    SLE_RF_FORMAT_FRAME_TYPE_2 = 0x01,      /*!< @if Eng radio frequency format: Wireless Frame Type 2
                                                 @else   射频格式:无线帧类型2 @endif */
} sle_rf_format_t;

/**
 * @if Eng
 * @brief  radio frequency tx rate.
 * @else
 * @brief  射频长发速率。
 * @endif
 */
typedef enum {
    SLE_RF_TX_RATE_GFSK = 0x00,      /*!< @if Eng radio frequency tx rate:GFSK
                                          @else   射频长发速率:GFSK @endif */
    SLE_RF_TX_RATE_QPSK = 0x02,      /*!< @if Eng radio frequency tx rate:QPSK
                                          @else   射频长发速率:QPSK @endif */
    SLE_RF_TX_RATE_8PSK = 0x03,      /*!< @if Eng radio frequency tx rate:8PSK
                                          @else   射频长发速率:8PSK @endif */
} sle_rf_tx_rate_t;

/**
 * @if Eng
 * @brief  radio frequency pilot ratio.
 * @else
 * @brief  射频导频比。
 * @endif
 */
typedef enum {
    SLE_RF_PILOT_RATIO_NO = 0x00,        /*!< @if Eng radio frequency pilot ratio,No
                                              @else   射频导频比:No @endif */
    SLE_RF_PILOT_RATIO_1_1 = 0x01,       /*!< @if Eng radio frequency pilot ratio,1:1
                                              @else   射频导频比:1:1 @endif */
    SLE_RF_PILOT_RATIO_4_1 = 0x02,       /*!< @if Eng radio frequency pilot ratio,4:1
                                              @else   射频导频比:4:1 @endif */
    SLE_RF_PILOT_RATIO_16_1 = 0x03,      /*!< @if Eng radio frequency pilot ratio,16:1
                                              @else   射频导频比:16:1 @endif */
} sle_rf_pilot_ratio_t;

/**
 * @if Eng
 * @brief  radio frequency tx polar.
 * @else
 * @brief  射频发射极化编码
 * @endif
 */
typedef enum {
    SLE_RF_TX_POLAR_NO = 0x00,        /*!< @if Eng radio frequency tx polar,No
                                           @else   射频发射极化编码:No @endif */
    SLE_RF_TX_POLAR_2_3 = 0x01,       /*!< @if Eng radio frequency tx polar,2/3
                                           @else   射频发射极化编码:2/3 @endif */
    SLE_RF_TX_POLAR_3_4 = 0x02,       /*!< @if Eng radio frequency tx polar,3/4
                                           @else   射频发射极化编码:3/4 @endif */
    SLE_RF_TX_POLAR_5_6 = 0x03,       /*!< @if Eng radio frequency tx polar,5/6
                                           @else   射频发射极化编码:5/6 @endif */
} sle_rf_tx_polar_t;

/**
 * @if Eng
 * @brief Enum of sle radio frequency tx start
 * @else
 * @brief sle开启射频长发参数
 * @endif
 */
typedef struct {
    uint8_t tx_freq;            /*!< @if Eng tx frequency,Scope:0x00~0x4E,2402+x
                                     @else   发送频率,范围:0x00~0x4E,2402+x @endif */
    uint8_t tx_power;           /*!< @if Eng tx power { @ref sle_rf_power_t }
                                     @else   发射功率 { @ref sle_rf_power_t } @endif */
    uint16_t test_data_len;     /*!< @if Eng tx test data len,Scope:0x00~0xFF
                                     @else   发射测试数据长度,取值范围:0x00~0xFF @endif */
    uint8_t pk_payload_type;    /*!< @if Eng radio frequency payload type { @ref sle_rf_payload_type_t }
                                     @else   信息体类型 { @ref sle_rf_payload_type_t } @endif */
    uint8_t tx_phy;             /*!< @if Eng radio frequency physical { @ref sle_rf_phy_t }
                                     @else   射频物理层 { @ref sle_rf_phy_t } @endif */
    uint8_t tx_format;          /*!< @if Eng tx format { @ref sle_rf_format_t }
                                     @else   发射格式 { @ref sle_rf_format_t } @endif */
    uint8_t tx_rate;            /*!< @if Eng radio frequency tx rate { @ref sle_rf_tx_rate_t }
                                     @else   射频长发速率 { @ref sle_rf_tx_rate_t } @endif */
    uint8_t tx_pilot_ratio;     /*!< @if Eng radio frequency pilot ratio { @ref sle_rf_pilot_ratio_t }
                                     @else   射频导频比 { @ref sle_rf_pilot_ratio_t } @endif */
    uint8_t tx_polar_r;         /*!< @if Eng radio frequency tx polar { @ref sle_rf_tx_polar_t }
                                     @else   射频发射极化编码 { @ref sle_rf_tx_polar_t } @endif */
    uint16_t tx_interval;       /*!< @if Eng radio frequency tx interval,Scope:0x0006~0xFFFF,
                                             one unit length is 125 us.
                                     @else   射频发射间隔,范围:0x0006~0xFFFF,
                                             1个单位长度为125us @endif */
} sle_rf_tx_start_t;

/**
 * @if Eng
 * @brief Enum of sle radio frequency rx start
 * @else
 * @brief sle开启射频长收参数
 * @endif
 */
typedef struct {
    uint8_t rx_freq;            /*!< @if Eng tx frequency,Scope:0x00~0x4E
                                     @else   发送频率,范围:0x00~0x4E @endif */
    uint8_t rx_phy;             /*!< @if Eng radio frequency physical { @ref sle_rf_phy_t }
                                     @else   射频物理层 { @ref sle_rf_phy_t } @endif */
    uint8_t rx_format;          /*!< @if Eng tx format { @ref sle_rf_format_t }
                                     @else   发射格式 { @ref sle_rf_format_t } @endif */
    uint8_t rx_pilot_ratio;     /*!< @if Eng radio frequency pilot ratio { @ref sle_rf_pilot_ratio_t }
                                     @else   射频导频比 { @ref sle_rf_pilot_ratio_t } @endif */
    uint16_t rx_interval;       /*!< @if Eng radio frequency rx interval,Scope:0x0000~0xFFFF,
                                             one unit length is 125 us.
                                     @else   射频接收间隔,范围:0x0000~0xFFFF,
                                             1个单位长度为125us @endif */
} sle_rf_rx_start_t;

/**
 * @if Eng
 * @brief Enum of sle radio frequency tx and rx end complete event
 * @else
 * @brief sle关闭射频收发回调返回参数
 * @endif
 */
typedef struct {
    uint8_t status;             /*!< @if Eng event complete status
                                     @else   事件完成状态 @endif */
    uint16_t num_packets;       /*!< @if Eng num packets
                                     @else   数据包数 @endif */
    uint8_t rssi;               /*!< @if Eng rssi,Default:-127
                                     @else   接收信号强度指示,默认-127 @endif */
} sle_rf_trx_end_cmp_evt_t;

/**
 * @if Eng
 * @brief Callback invoked when start radio frequenct tx.
 * @par Callback invoked when start radio frequenct tx.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @else
 * @brief  设置射频长发时的回调函数。
 * @par    设置射频长发时的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @endif
 */
typedef void (*sle_rf_tx_start_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when start radio frequenct rx.
 * @par Callback invoked when start radio frequenct rx.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @else
 * @brief  设置射频长收时的回调函数。
 * @par    设置射频长收时的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @endif
 */
typedef void (*sle_rf_rx_start_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when end radio frequenct tx and rx.
 * @par Callback invoked when end radio frequenct tx and rx.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  status error code.
 * @param  [in]  sle_rf_trx_end_cmp_evt_t result struct.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @else
 * @brief  关闭射频收发时的回调函数。
 * @par    关闭射频收发时的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @param  [in]  sle_rf_trx_end_cmp_evt_t 执行结果返回结构体。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @endif
 */
typedef void (*sle_rf_trx_end_callback)(sle_rf_trx_end_cmp_evt_t* cmp_evt);

/**
 * @if Eng
 * @brief Callback invoked when reset radio frequenct tx and rx.
 * @par Callback invoked when reset radio frequenct tx and rx.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @else
 * @brief  重置射频收发时的回调函数。
 * @par    重置射频收发时的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_factory_callbacks_t
 * @endif
 */
typedef void (*sle_rf_reset_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Struct of SLE radio referency callback function.
 * @else
 * @brief SLE factory 管理回调函数接口定义。
 * @endif
 */
typedef struct {
    sle_rf_tx_start_callback rf_tx_start_cb;                          /*!< @if Eng radio frequency tx start
                                                                                   callback.
                                                                           @else   射频长发回调函数。 @endif */
    sle_rf_rx_start_callback rf_rx_start_cb;                          /*!< @if Eng radio frequency rx start
                                                                                   callback.
                                                                           @else   射频长收回调函数。 @endif */
    sle_rf_trx_end_callback rf_trx_end_cb;                            /*!< @if Eng radio frequency tx and rx end
                                                                                   callback.
                                                                           @else   射频收发结束回调函数。 @endif */
    sle_rf_reset_callback rf_reset_cb;                                /*!< @if Eng radio frequency tx and rx reset
                                                                                   callback.
                                                                           @else   射频收发复位回调。 @endif */
} sle_factory_callbacks_t;

/**
 * @if Eng
 * @brief  start radio frequenct tx.
 * @par Description: start radio frequenct tx.
 * @param  [in]  rf_tx_start
 * @retval error code, read result will be returned at { @ref sle_rf_tx_start_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  开启射频长发。
 * @par Description: 开启射频长发。
 * @param  [in]  rf_tx_start
 * @retval 执行结果错误码，读取结果将在{ @ref sle_rf_tx_start_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_rf_tx_start(sle_rf_tx_start_t* rf_tx_start);

/**
 * @if Eng
 * @brief  start radio frequenct rx.
 * @par Description: start radio frequenct rx.
 * @param  [in]  rf_rx_start rx start parameter struct.
 * @retval error code, read result will be returned at { @ref sle_rf_rx_start_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  开启射频长发。
 * @par Description: 开启射频长收。
 * @param  [in]  rf_rx_start 长收设置参数结构体.
 * @retval 执行结果错误码，读取结果将在{ @ref sle_rf_rx_start_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_rf_rx_start(sle_rf_rx_start_t* rf_rx_start);

/**
 * @if Eng
 * @brief  end radio frequenct tx and rx.
 * @par Description: end radio frequenct tx and rx.
 * @retval error code, read result will be returned at { @ref sle_rf_trx_end_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  关闭射频收发。
 * @par Description: 关闭射频收发。
 * @retval 执行结果错误码，读取结果将在{ @ref sle_rf_trx_end_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_rf_trx_end(void);

/**
 * @if Eng
 * @brief  reset radio frequenct tx and rx.
 * @par Description: reset radio frequenct tx and rx.
 * @retval error code, read result will be returned at { @ref sle_rf_reset_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  重置射频收发。
 * @par Description: 重置射频收发。
 * @retval 执行结果错误码，读取结果将在{ @ref sle_rf_reset_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_rf_reset(void);

/**
 * @if Eng
 * @brief  Register SLE factory manager callbacks.
 * @par Description: Register SLE factory manager callbacks.
 * @param  [in]  func Callback function.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @else
 * @brief  注册 SLE factory 管理回调函数。
 * @par Description: 注册 SLE factory 管理回调函数。
 * @param  [in]  func 回调函数。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @endif
 */
errcode_t sle_factory_register_callbacks(sle_factory_callbacks_t *func);


#ifdef __cplusplus
}
#endif
#endif /* SLE_FACTORY_MANAGER_H */
/**
 * @}
 */