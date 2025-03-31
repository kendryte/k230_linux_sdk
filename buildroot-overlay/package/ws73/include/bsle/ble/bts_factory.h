/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: BTS FACTORY module.
 */


#ifndef BTS_FACTORY_H
#define BTS_FACTORY_H

#include <stdint.h>
#include <stdbool.h>
#include "errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup bluetooth_bts_factory BTS FACTORY API
 * @ingroup  bluetooth
 * @{
 */

/**
 * @if Eng
 * @brief  radio frequency physical.
 * @else
 * @brief  射频物理层。
 * @endif
 */
typedef enum {
    BLE_RF_PHY_1M = 0x01,      /*!< @if Eng radio frequency physical: 1M
                                    @else   射频物理层1M @endif */
    BLE_RF_PHY_2M = 0x02,      /*!< @if Eng radio frequency physical: 2M
                                    @else   射频物理层2M @endif */
    BLE_RF_PHY_S8 = 0x03,      /*!< @if Eng radio frequency physical: coded PHY with S=8 data coding
                                    @else   射频物理层 S=8数据编码的编码PHY @endif */
    BLE_RF_PHY_S2 = 0x04,      /*!< @if Eng radio frequency physical: coded PHY with S=2 data coding
                                    @else   射频物理层 S=2数据编码的编码PHY @endif */
} ble_rf_phy_t;

/**
 * @if Eng
 * @brief  radio frequency payload type.
 * @else
 * @brief  信息体类型。
 * @endif
 */
typedef enum {
    BLE_RF_PATLOAD_TYPE_0 = 0x00,    /*!< @if Eng radio frequency payload type:PRBS9
                                          @else   射频信息体类型:PRBS9 @endif */
    BLE_RF_PATLOAD_TYPE_1 = 0x01,    /*!< @if Eng radio frequency payload type:11110000
                                          @else   射频信息体类型:11110000 @endif */
    BLE_RF_PATLOAD_TYPE_2 = 0x02,    /*!< @if Eng radio frequency payload type:10101010
                                          @else   射频信息体类型:10101010 @endif */
    BLE_RF_PATLOAD_TYPE_3 = 0x03,    /*!< @if Eng radio frequency payload type:PRBS15
                                          @else   射频信息体类型:PRBS15 @endif */
    BLE_RF_PATLOAD_TYPE_4 = 0x04,    /*!< @if Eng radio frequency payload type:11111111
                                          @else   射频信息体类型:11111111 @endif */
    BLE_RF_PATLOAD_TYPE_5 = 0x05,    /*!< @if Eng radio frequency payload type:00000000
                                          @else   射频信息体类型:00000000 @endif */
    BLE_RF_PATLOAD_TYPE_6 = 0x06,    /*!< @if Eng radio frequency payload type:00001111
                                          @else   射频信息体类型:00001111 @endif */
    BLE_RF_PATLOAD_TYPE_7 = 0x07,    /*!< @if Eng radio frequency payload type:01010101
                                          @else   射频信息体类型:01010101 @endif */
} ble_rf_payload_type_t;

/**
 * @if Eng
 * @brief Enum of ble radio frequency tx start
 * @else
 * @brief ble开启射频长发参数
 * @endif
 */
typedef struct {
    uint8_t tx_freq;            /*!< @if Eng tx frequency,Scope:0x00~0x27,2402+x*2
                                     @else   发送频率,范围:0x00~0x27,2402+x*2 @endif */
    uint16_t test_data_len;     /*!< @if Eng tx test data len,Scope:0x00~0xFB
                                     @else   发射测试数据长度,取值范围:0x25~0xFB @endif */
    uint8_t payload_type;       /*!< @if Eng palyload type { @ref ble_rf_payload_type_t }
                                     @else   发射测试数据包类型 { @ref ble_rf_payload_type_t } @endif */
    uint8_t tx_phy;             /*!< @if Eng radio frequency physical { @ref ble_rf_phy_t }
                                     @else   射频物理层 { @ref ble_rf_phy_t } @endif */
} ble_rf_tx_start_t;

/**
 * @if Eng
 * @brief Enum of ble radio frequency rx start
 * @else
 * @brief ble开启射频长收参数
 * @endif
 */
typedef struct {
    uint8_t rx_freq;            /*!< @if Eng tx frequency,Scope:0x00~0x27,2402+x*2
                                     @else   发送频率,范围:0x00~0x27,2402+x*2 @endif */
    uint8_t rx_phy;             /*!< @if Eng radio frequency physical { @ref ble_rf_phy_t }
                                     @else   射频物理层 { @ref ble_rf_phy_t } @endif */
    uint8_t modulation_index;   /*!< @if Eng modulation index,0:standard,1:stable
                                     @else   调制指数,0:标准,1:稳定 @endif */
} ble_rf_rx_start_t;

/**
 * @if Eng
 * @brief Enum of ble radio frequency rx start
 * @else
 * @brief ble射频单音参数
 * @endif
 */
typedef struct {
    uint8_t rf_freq;            /*!< @if Eng tx frequency,Scope:0x00~0x4E,2402+x*2
                                     @else   发送频率,范围:0x00~0x4E,2402+x*2 @endif */
    uint8_t rf_mode;            /*!< @if Eng single tone mode,00:start,FF:stop
                                     @else   单音模式,00:开启,FF:关闭 @endif */
} ble_rf_single_tone_t;

/**
 * @if Eng
 * @brief Enum of hci_vendor_productline_cmd subcode
 * @else
 * @brief 产线校准命令的subcode
 * @endif
 */
typedef enum {
    BTH_PRODUCTLINE_XO_TRIM = 0x01,                    // 设置频偏校准值，寄存器控
    BTH_PRODUCTLINE_XO_TRIM_RD_VAL = 0x02,             // 读取频偏校准寄存器值
    BTH_PRODUCTLINE_GET_TSENSOR_TEMPERATURE = 0x03,    // 获取芯片温度
    BTH_PRODUCTLINE_EFUSE_WRITE_XO_TRIM = 0x04,        // 将频偏校准值写入EFUSE
    BTH_PRODUCTLINE_EFUSE_READ_XO_TRIM = 0x05,         // 从EFUSE读取频偏校准值
    BTH_PRODUCTLINE_EFUSE_WRITE_TEMPERATURE = 0x06,    // 将产测时的芯片温度写入EFUSE
    BTH_PRODUCTLINE_EFUSE_READ_TEMPERATURE = 0x07,     // 从EFUSE读取产测温度
    BTH_PRODUCTLINE_PWR_CALI_SET_MEASSURED_PWR = 0x08, // 将实测发送功率发送给驱动
    BTH_PRODUCTLINE_PWR_CALI_GET_COMP_RESULT = 0x09,   // 获取功率校准结果
    BTH_BTH_PRODUCTLINE_PWR_CALI_APPLY_COMP = 0x0A,    // 应用功率校准结果
    BTH_PRODUCTLINE_EFUSE_WRITE_PWR_COMP = 0x0B,       // 将功率校准结果写入EFUSE
    BTH_PRODUCTLINE_EFUSE_READ_PWR_COMP = 0x0C,        // 从EFUSE读取功率校准结果
} ble_pdl_sub_opcode_t;

/* sub_opcode = BTH_PRODUCTLINE_XO_TRIM */
typedef struct {
    uint8_t coarse_val;
    uint8_t fine_val;
} pdl_xo_trim_t;

/* sub_opcode = BTH_PRODUCTLINE_EFUSE_WRITE_TEMPERATURE */
typedef struct {
    int16_t temp;
} pdl_efuse_temp_wr_t;

/* sub_opcode = BTH_PRODUCTLINE_PWR_CALI_SET_MEASSURED_PWR */
typedef struct {
    int16_t target_pwr;
    int16_t pwr;
} pdl_pwr_cali_set_pwr_t;

/* opcode = HCI_VENDOR_PRODUCTLINE_CMD_OPCODE */
typedef struct {
    uint8_t sub_opcode; // subcode
    union {
        pdl_xo_trim_t xo_trim;
        pdl_efuse_temp_wr_t efuse_wr_temp;
        pdl_pwr_cali_set_pwr_t pwr_info;
    };
} ble_vendor_productline_cmd_t;

#pragma pack(1)
/* sub_evtcode = BTH_PRODUCTLINE_XO_TRIM_RD_VAL */
typedef struct {
    uint8_t sel;
    uint8_t coarse_val;
    uint8_t fine_val;
} ble_pdl_rd_xo_trim_t;

/* sub_evtcode = BTH_PRODUCTLINE_GET_TSENSOR_TEMPERATURE */
typedef struct {
    int16_t temp;
} ble_pdl_get_temp_t;

/* sub_evtcode = BTH_PRODUCTLINE_EFUSE_READ_XO_TRIM */
typedef struct {
    uint8_t coarse_val;
    uint8_t fine_val;
} ble_pdl_efuse_rd_xo_trim_t;

/* sub_evtcode = BTH_PRODUCTLINE_EFUSE_READ_TEMPERATURE */
typedef struct {
    uint8_t temp_lvl;
} ble_pdl_efuse_rd_tmp_t;

/* sub_evtcode = BTH_PRODUCTLINE_PWR_CALI_GET_COMP_RESULT */
typedef struct {
    int16_t curve_c_offset;
} ble_pdl_pwr_cali_get_result_t;

/* sub_evtcode = BTH_PRODUCTLINE_EFUSE_READ_PWR_COMP */
typedef struct {
    int16_t curve_c_offset;
} ble_pdl_efuse_rd_pwr_result_t;
 
/* evtcode = HCI_VENDOR_PRODUCTLINE_CMD_OPCODE */
typedef struct {
    uint8_t nb;
    uint16_t opcode;
    uint8_t status;
    uint8_t subcode;
    union {
        ble_pdl_rd_xo_trim_t xo_trim_reg_val;
        ble_pdl_get_temp_t temp;
        ble_pdl_efuse_rd_xo_trim_t xo_trim;
        ble_pdl_efuse_rd_tmp_t temp_lvl;
        ble_pdl_pwr_cali_get_result_t pwr_result;
        ble_pdl_efuse_rd_pwr_result_t efuse_pwr_result;
    };
} ble_hci_vendor_productline_complete_t;
#pragma pack()

/**
 * @if Eng
 * @brief Callback invoked when start radio frequenct tx.
 * @par Callback invoked when start radio frequenct tx.
 * @attention 1.This function is called in BLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the BLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @else
 * @brief  设置射频长发时的回调函数。
 * @par    设置射频长发时的回调函数。
 * @attention  1. 该回调函数运行于BLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由BLE service申请内存，也由BLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @endif
 */
typedef void (*ble_factory_rf_tx_start_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when start radio frequenct rx.
 * @par Callback invoked when start radio frequenct rx.
 * @attention 1.This function is called in BLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the BLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @else
 * @brief  设置射频长收时的回调函数。
 * @par    设置射频长收时的回调函数。
 * @attention  1. 该回调函数运行于BLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由BLE service申请内存，也由BLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @endif
 */
typedef void (*ble_factory_rf_rx_start_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when end radio frequenct tx and rx.
 * @par Callback invoked when end radio frequenct tx and rx.
 * @attention 1.This function is called in BLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the BLE service automatically.
 * @param  [in]  status error code.
 * @param  [in]  num_packets num packets.
 * @par Dependency:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @else
 * @brief  关闭射频收发时的回调函数。
 * @par    关闭射频收发时的回调函数。
 * @attention  1. 该回调函数运行于BLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由BLE service申请内存，也由BLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @param  [in]  num_packets 数据包数。
 * @par 依赖:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @endif
 */
typedef void (*ble_factory_rf_trx_end_callback)(errcode_t status, uint16_t num_packets);

/**
 * @if Eng
 * @brief Callback invoked when reset radio frequenct tx and rx.
 * @par Callback invoked when reset radio frequenct tx and rx.
 * @attention 1.This function is called in BLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the BLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @else
 * @brief  重置射频收发时的回调函数。
 * @par    重置射频收发时的回调函数。
 * @attention  1. 该回调函数运行于BLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由BLE service申请内存，也由BLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @endif
 */
typedef void (*ble_factory_rf_reset_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when cali radio frequenct nv.
 * @par Callback invoked when cali radio frequenct nv.
 * @attention 1.This function is called in BLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the BLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @else
 * @brief  校准射频NV时的回调函数。
 * @par    校准射频NV时的回调函数。
 * @attention  1. 该回调函数运行于BLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由BLE service申请内存，也由BLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @endif
 */
typedef void (*ble_factory_rf_cali_nv_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when set or end radio frequenct single tone mode.
 * @par Callback invoked when set or end radio frequenct single tone mode.
 * @attention 1.This function is called in BLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the BLE service automatically.
 * @param  [in]  status error code.
 * @par Dependency:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @else
 * @brief  打开或关闭射频单音模式时的回调函数。
 * @par    打开或关闭射频单音模式时的回调函数。
 * @attention  1. 该回调函数运行于BLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由BLE service申请内存，也由BLE service释放，回调中不应释放。
 * @param  [in]  status 执行结果错误码。
 * @par 依赖:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @endif
 */
typedef void (*ble_factory_rf_single_tone_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when set factory vendor calibration conmmand.
 * @par Callback invoked when sset factory vendor calibration conmmand.
 * @attention 1.This function is called in BLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the BLE service automatically.
 * @param  [in]  evt complete event.
 * @par Dependency:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @else
 * @brief  产线频偏/功率校准及其补偿值efuse读写的回调函数。
 * @par    产线频偏/功率校准及其补偿值efuse读写的回调函数。
 * @attention  1. 该回调函数运行于BLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由BLE service申请内存，也由BLE service释放，回调中不应释放。
 * @param  [in]  evt 上报完成事件。
 * @par 依赖:
 * @li  bts_def.h
 * @see ble_factory_callbacks_t
 * @endif
 */
typedef void (*ble_factory_vendor_pdl_cmd_callback)(ble_hci_vendor_productline_complete_t *evt);

/**
 * @if Eng
 * @brief Struct of BLE radio referency callback function.
 * @else
 * @brief BLE factory 管理回调函数接口定义。
 * @endif
 */
typedef struct {
    ble_factory_rf_tx_start_callback ble_rf_tx_start_cb;         /*!< @if Eng radio frequency tx start
                                                                              callback.
                                                                      @else   射频长发回调函数。 @endif */
    ble_factory_rf_rx_start_callback ble_rf_rx_start_cb;         /*!< @if Eng radio frequency rx start
                                                                              callback.
                                                                      @else   射频长收回调函数。 @endif */
    ble_factory_rf_trx_end_callback ble_rf_trx_end_cb;           /*!< @if Eng radio frequency tx and rx end
                                                                              callback.
                                                                      @else   射频收发结束回调函数。 @endif */
    ble_factory_rf_reset_callback ble_rf_reset_cb;               /*!< @if Eng radio frequency tx and rx reset
                                                                              callback.
                                                                      @else   射频收发复位回调。 @endif */
    ble_factory_rf_cali_nv_callback ble_rf_cali_nv_cb;           /*!< @if Eng radio frequency cali nv
                                                                              callback.
                                                                      @else   校准射频NV回调。 @endif */
    ble_factory_rf_single_tone_callback ble_rf_single_tone_cb;   /*!< @if Eng radio frequency single tone mode
                                                                              callback.
                                                                      @else   设置射频单音模式回调。 @endif */
    ble_factory_vendor_pdl_cmd_callback ble_vendor_pdl_cmd_cb;   /*!< @if Eng productline calibration vendor cmd
                                                                              callback.
                                                                      @else   产线校准命令回调。 @endif */
} ble_factory_callbacks_t;

/**
 * @if Eng
 * @brief  Register BLE factory manager callbacks.
 * @par Description: Register BLE factory manager callbacks.
 * @param  [in]  func Callback function.
 * @retval error code.
 * @else
 * @brief  注册 BLE factory 管理回调函数。
 * @par Description: 注册 BLE factory 管理回调函数。
 * @param  [in]  func 回调函数。
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t ble_factory_register_callbacks(ble_factory_callbacks_t *func);

/**
 * @if Eng
 * @brief  start radio frequenct tx.
 * @par Description: start radio frequenct tx.
 * @param  [in]  param
 * @retval error code, read result will be returned at { @ref ble_factory_rf_tx_start_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  开启射频长发。
 * @par Description: 开启射频长发。
 * @param  [in]  param
 * @retval 执行结果错误码，读取结果将在{ @ref ble_factory_rf_tx_start_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ble_factory_rf_tx_start(ble_rf_tx_start_t* param);

/**
 * @if Eng
 * @brief  start radio frequenct rx.
 * @par Description: start radio frequenct rx.
 * @param  [in]  param
 * @retval error code, read result will be returned at { @ref ble_factory_rf_rx_start_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  开启射频长发。
 * @par Description: 开启射频长收。
 * @param  [in]  param
 * @retval 执行结果错误码，读取结果将在{ @ref ble_factory_rf_rx_start_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ble_factory_rf_rx_start(ble_rf_rx_start_t* param);

/**
 * @if Eng
 * @brief  end radio frequenct tx and rx.
 * @par Description: end radio frequenct tx and rx.
 * @retval error code, read result will be returned at { @ref ble_factory_rf_trx_end_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  关闭射频收发。
 * @par Description: 关闭射频收发。
 * @retval 执行结果错误码，读取结果将在{ @ref ble_factory_rf_trx_end_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ble_factory_rf_trx_end(void);

/**
 * @if Eng
 * @brief  reset radio frequenct tx and rx.
 * @par Description: reset radio frequenct tx and rx.
 * @retval error code, read result will be returned at { @ref ble_factory_rf_reset_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  重置射频收发。
 * @par Description: 重置射频收发。
 * @retval 执行结果错误码，读取结果将在{ @ref ble_factory_rf_reset_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ble_factory_rf_reset(void);

/**
 * @if Eng
 * @brief  cali radio frequenct nv.
 * @par Description: cali radio frequenct nv.
 * @retval error code, read result will be returned at { @ref ble_factory_rf_cali_nv_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  校准射频NV。
 * @par Description: 校准射频NV。
 * @retval 执行结果错误码，读取结果将在{ @ref ble_factory_rf_cali_nv_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ble_factory_rf_cali_nv(void);

/**
 * @if Eng
 * @brief  set radio frequenct single tone mode.
 * @par Description: set radio frequenct single tone mode.
 * @param  [in]  param param.
 * @retval error code, read result will be returned at { @ref ble_factory_rf_single_tone_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  校准射频NV。
 * @par Description: 校准射频NV。
 * @param  [in]  param 配置参数.
 * @retval 执行结果错误码，读取结果将在{ @ref ble_factory_rf_single_tone_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ble_factory_rf_single_tone(ble_rf_single_tone_t* param);

/**
 * @if Eng
 * @brief  set factory vendor calibration conmmand.
 * @par Description: set factory vendor calibration conmmand.
 * @param  [in]  param param.
 * @retval error code, read result will be returned at { @ref ble_factory_vendor_pdl_cmd_callback }.
 * @par Depends:
 * @li bts_def.h
 * @else
 * @brief  产线校准。
 * @par Description: 产线校准。
 * @param  [in]  param 配置参数.
 * @retval 执行结果错误码，读取结果将在{ @ref ble_factory_vendor_pdl_cmd_callback }中返回。
 * @par 依赖：
 * @li bts_def.h
 * @endif
 */
errcode_t ble_factory_vendor_productline_cmd(ble_vendor_productline_cmd_t* param);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
