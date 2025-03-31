/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE Trans Manager module.
 */

/**
 * @defgroup sle_trans_manager trans manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_TM_SIGNAL_H
#define SLE_TM_SIGNAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  连接管理查询能力比特位参数
 */
typedef struct {
    uint32_t relay_capability : 1;         /*!< @if Eng relay capability
                                                @else   中继能力 @endif */
    uint32_t trans_mode : 1;               /*!< @if Eng trans mode
                                                @else   传输模式 @endif */
    uint32_t measurement_capability : 1;   /*!< @if Eng measurement capability
                                                @else   测量能力 @endif */
    uint32_t access_slb : 1;               /*!< @if Eng access slb
                                                @else   slb接入 @endif */
    uint32_t access_sle : 1;               /*!< @if Eng access sle
                                                @else   sle接入 @endif */
    uint32_t mtu : 1;                      /*!< @if Eng max mtu
                                                @else   最大支持mtu @endif */
    uint32_t mps : 1;                      /*!< @if Eng max mps
                                                @else   最大支持mps @endif */
    uint32_t reverse : 25;                 /*!< @if Eng reverse
                                                @else   保留比特位 @endif */
} sle_tm_signal_capability_bit_t;

/**
 * @if Eng
 * @brief  Send signal capability request.
 * @par Description: Send signal capability request.
 * @param  [in]  conn_id   Connection id.
 * @param  [in]  param     Capability info.
 * @retval error code
 * @par Depends:
 * @li  sle_common.h
 * @else
 * @brief  发送连接管理能力查询请求。
 * @par Description: 发送连接管理能力查询请求。
 * @param  [in]  conn_id   连接 ID。
 * @param  [in]  param     查询的能力信息。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_tm_signal_capability_req(uint16_t conn_id, sle_tm_signal_capability_bit_t* param);

#ifdef __cplusplus
}
#endif
#endif /* SLE_TM_SIGNAL */
/**
 * @}
 */