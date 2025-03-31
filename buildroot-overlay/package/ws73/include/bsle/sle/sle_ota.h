/**
 * Copyright (c) @CompanyNameMagicTag 2023-2024. All rights reserved.
 *
 * Description: SLE OTA service.
 */

#ifndef SLE_OTA_H
#define SLE_OTA_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup sle_ota API
 * @ingroup  SLE
 * @{
 */

/**
 * @if Eng
 * @brief  SLE ota server init.
 * @attention  NULL
 * @retval ERRCODE_SUCC     Success
 * @retval Other            Failure，For details, see @ref errcode_t
 * @par Dependency:
 * @li sle_ssap_server.h
 * @else
 * @brief  SLE OTA服务器初始化。
 * @attention  NULL
 * @retval ERRCODE_SUCC     成功
 * @retval Other            失败，参考 @ref errcode_t
 * @par 依赖:
 * @li sle_ssap_server.h
 * @endif
 */
errcode_t sle_ota_service_init(uint8_t server_id);

/**
 * @if Eng
 * @brief  SLE ota server send data ack.
 * @attention  NULL
 * @retval ERRCODE_SUCC     Success
 * @retval Other            Failure，For details, see @ref errcode_t
 * @par Dependency:
 * @li sle_ssap_server.h
 * @else
 * @brief  SLE OTA服务器发送数据ack。
 * @attention  NULL
 * @retval ERRCODE_SUCC     成功
 * @retval Other            失败，参考 @ref errcode_t
 * @par 依赖:
 * @li sle_ssap_server.h
 * @endif
 */
errcode_t sle_ota_data_ack(uint16_t value_len, uint8_t *value);

/**
 * @if Eng
 * @brief  Definition of Data Obtaining and Invoking for general using.
 * @param [in] data_ptr data.
 * @param [in] data_len data length.
 * @retval error code.
 * @else
 * @brief  通用获取数据回调定义。
 * @param [in] data_ptr 数据。
 * @param [in] data_len 数据长度。
 * @retval error code.
 * @endif
 */
typedef void (*sle_ota_chan_data_report) (const uint8_t *data_ptr,  const uint16_t data_len);

/**
 * @if Eng
 * @brief Callback function for server to receive ota data.
 * @else
 * @brief 服务端接收ota升级数据回调。
 * @endif
 */
void sle_ota_reg_chan_data_report_cbk(sle_ota_chan_data_report data_report);

#ifdef __cplusplus
}
#endif
#endif /* SLE_SRV_OTA_H */

/**
* @}
*/