/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE Connection Manager module.
 */

/**
 * @defgroup sle_connection_manager connection manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_CONNECTION_MANAGER
#define SLE_CONNECTION_MANAGER

#include <stdint.h>
#include "sle_errcode.h"
#include "sle_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief Enum of sle pairing state.
 * @else
 * @brief 星闪配对状态。
 * @endif
 */
typedef enum {
    SLE_PAIR_NONE    = 0x01,    /*!< @if Eng Pair state of none
                                     @else   未配对状态 @endif */
    SLE_PAIR_PAIRING = 0x02,    /*!< @if Eng Pair state of pairing
                                     @else   正在配对 @endif */
    SLE_PAIR_PAIRED  = 0x03     /*!< @if Eng Pair state of paired
                                     @else   已完成配对 @endif */
} sle_pair_state_t;

/**
 * @if Eng
 * @brief Enum of sle pairing state.
 * @else
 * @brief 星闪断链原因。
 * @endif
 */
typedef enum {
    SLE_DISCONNECT_BY_REMOTE = 0x10,    /*!< @if Eng disconnect by remote
                                             @else   远端断链 @endif */
    SLE_DISCONNECT_BY_LOCAL  = 0x11,    /*!< @if Eng disconnect by local
                                             @else   本端断链 @endif */
} sle_disc_reason_t;

/**
 * @if Eng
 * @brief Enum of sle ACB connection state.
 * @else
 * @brief SLE ACB连接状态。
 * @endif
 */
typedef enum {
    SLE_ACB_STATE_NONE          = 0x00,   /*!< @if Eng SLE ACB connect state of none
                                               @else   SLE ACB 未连接状态 @endif */
    SLE_ACB_STATE_CONNECTED     = 0x01,   /*!< @if Eng SLE ACB connect state of connected
                                               @else   SLE ACB 已连接 @endif */
    SLE_ACB_STATE_DISCONNECTED  = 0x02,   /*!< @if Eng SLE ACB connect state of disconnected
                                               @else   SLE ACB 已断接 @endif */
} sle_acb_state_t;

/**
 * @if Eng
 * @brief Enum of sle crytography algorithm.
 * @else
 * @brief 星闪加密算法类型。
 * @endif
 */
typedef enum {
    SLE_CRYTO_ALGO_AC1     = 0x00,   /*!< @if Eng crytography algorithm ac1
                                          @else   AC1加密算法类型 @endif */
    SLE_CRYTO_ALGO_AC2     = 0x01,   /*!< @if Eng crytography algorithm ac2
                                          @else   AC2加密算法类型@endif */
    SLE_CRYTO_ALGO_EA1     = 0x02,   /*!< @if Eng crytography algorithm ea1
                                          @else   EA1加密算法类型 @endif */
    SLE_CRYTO_ALGO_EA2     = 0x03,   /*!< @if Eng crytography algorithm ea2
                                          @else   EA2加密算法类型 @endif */
} sle_crypto_algo_t;

/**
 * @if Eng
 * @brief Enum of sle key derivation algorithm
 * @else
 * @brief 星闪秘钥分发算法类型。
 * @endif
 */
typedef enum {
    SLE_KEY_DERIV_ALGO_HA1     = 0x00,   /*!< @if Eng key derivation algorithm ac1
                                              @else   HA1秘钥分发算法类型 @endif */
    SLE_KEY_DERIV_ALGO_HA2     = 0x01,   /*!< @if Eng key derivation algorithm ac2
                                              @else   HA2秘钥分发算法类型 @endif */
} sle_key_deriv_algo_t;

/**
 * @if Eng
 * @brief Enum of sle integrity check indicator
 * @else
 * @brief 星闪完整性校验指示类型。
 * @endif
 */
typedef enum {
    SLE_ENCRYPTION_ENABLE_INTEGRITY_CHK_ENABLE      = 0x00,   /*!< @if Eng Encryption and integrity check
                                                                           are enabled at the same time.
                                                                   @else   加密和完整性保护同时启动 @endif */
    SLE_ENCRYPTION_DISABLE_INTEGRITY_CHK_ENABLE     = 0x01,   /*!< @if Eng Do not enable encryption, but enable
                                                                           integrity check.
                                                                   @else   不启动加密，启动完整性保护 @endif */
    SLE_ENCRYPTION_ENABLE_INTEGRITY_CHK_DISABLE     = 0x02,   /*!< @if Eng Encryption is enabled, but integrity
                                                                           check is disabled.
                                                                   @else   启动加密，不启动完整性保护 @endif */
    SLE_ENCRYPTION_DISABLE_INTEGRITY_CHK_DISABLE    = 0x03,   /*!< @if Eng Encryption and integrity check
                                                                           are not enabled.
                                                                   @else   不启动加密，不启动完整性保护 @endif */
} sle_integr_chk_ind_t;

/**
 * @if Eng
 * @brief Enum of sle logical link update parameters.
 * @else
 * @brief 星闪逻辑链路更新参数请求
 * @endif
 */
typedef struct {
    uint16_t interval_min;        /*!< @if Eng minimum interval
                                       @else   链路调度最小间隔，单位slot @endif */
    uint16_t interval_max;        /*!< @if Eng maximum interval
                                       @else   链路调度最大间隔，单位slot @endif */
    uint16_t max_latency;         /*!< @if Eng maximum latency
                                       @else   延迟周期，单位slot @endif */
    uint16_t supervision_timeout; /*!< @if Eng timeout
                                       @else   超时时间，单位10ms @endif */
} sle_connection_param_update_req_t;

/**
 * @if Eng
 * @brief Enum of sle logical link update parameters.
 * @else
 * @brief 星闪逻辑链路更新参数
 * @endif
 */
typedef struct {
    uint16_t conn_id;             /*!< @if Eng connection ID
                                       @else   连接ID @endif */
    uint16_t interval_min;        /*!< @if Eng minimum interval
                                       @else   链路调度最小间隔，单位slot @endif */
    uint16_t interval_max;        /*!< @if Eng maximum interval
                                       @else   链路调度最大间隔，单位slot @endif */
    uint16_t max_latency;         /*!< @if Eng maximum latency
                                       @else   延迟周期，单位slot @endif */
    uint16_t supervision_timeout; /*!< @if Eng timeout
                                       @else   超时时间，单位10ms @endif */
} sle_connection_param_update_t;

/**
 * @if Eng
 * @brief Enum of sle logical link update event parameters.
 * @else
 * @brief 星闪逻辑链路更新事件参数
 * @endif
 */
typedef struct {
    uint16_t interval;              /*!< @if Eng interval
                                         @else   链路调度间隔，单位slot @endif */
    uint16_t latency;               /*!< @if Eng latency
                                         @else   延迟周期，单位slot @endif */
    uint16_t supervision;           /*!< @if Eng timeout
                                         @else   超时时间，单位10ms @endif */
} sle_connection_param_update_evt_t;

/**
 * @if Eng
 * @brief Enum of sle authentication result.
 * @else
 * @brief 星闪认证结果
 * @endif
 */
typedef struct {
    uint8_t link_key[SLE_LINK_KEY_LEN];      /*!< @if Eng link key
                                                  @else   链路密钥 @endif */
    uint8_t crypto_algo;                     /*!< @if Eng encryption algorithm type { @ref sle_crypto_algo_t }
                                                  @else   加密算法类型 { @ref sle_crypto_algo_t } @endif */
    uint8_t key_deriv_algo;                  /*!< @if Eng key distribution algorithm type { @ref sle_key_deriv_algo_t }
                                                  @else   秘钥分发算法类型 { @ref sle_key_deriv_algo_t } @endif */
    uint8_t integr_chk_ind;                  /*!< @if Eng integrity check indication { @ref sle_integr_chk_ind_t }
                                                  @else   完整性校验指示 { @ref sle_integr_chk_ind_t } @endif */
} sle_auth_info_evt_t;

/**
 * @brief 星闪无线帧类型，bitmap表示
 */
typedef enum {
    SLE_RADIO_FRAME_1    = 0,      /*!< @if Eng Radio frame type 1.
                                             @else 无线帧类型1。 @endif */
    SLE_RADIO_FRAME_2    = 1,      /*!< @if Eng Eng Radio frame type 2.
                                             @else 无线帧类型2。 @endif */
    SLE_RADIO_FRAME_3_M0 = 2,      /*!< @if Eng Radio frame type 3 and m sequence 0.
                                             @else 无线帧类型3，m序列0。 @endif */
    SLE_RADIO_FRAME_3_M1 = 3,      /*!< @if Eng Radio frame type 3 and m sequence 1.
                                             @else 无线帧类型3，m序列1。 @endif */
    SLE_RADIO_FRAME_3_M2 = 4,      /*!< @if Eng Radio frame type 3 and m sequence 2.
                                             @else 无线帧类型3，m序列2。 @endif */
    SLE_RADIO_FRAME_3_M3 = 5,      /*!< @if Eng Radio frame type 3 and m sequence 3.
                                             @else 无线帧类型3，m序列3。 @endif */
    SLE_RADIO_FRAME_3_M4 = 6,      /*!< @if Eng Radio frame type 3 and m sequence 4.
                                             @else 无线帧类型3，m序列4。 @endif */
    SLE_RADIO_FRAME_3_M5 = 7,      /*!< @if Eng Radio frame type 3 and m sequence 5.
                                             @else 无线帧类型3，m序列5。 @endif */
    SLE_RADIO_FRAME_4_M0 = 8,      /*!< @if Eng Radio frame type 4 and m sequence 0.
                                             @else 无线帧类型4，m序列0 @endif */
    SLE_RADIO_FRAME_4_M1 = 9,      /*!< @if Eng Radio frame type 4 and m sequence 1.
                                             @else 无线帧类型4，m序列1 @endif */
    SLE_RADIO_FRAME_4_M2 = 10,     /*!< @if Eng Radio frame type 4 and m sequence 2.
                                             @else 无线帧类型4，m序列2。 @endif */
    SLE_RADIO_FRAME_4_M3 = 11,     /*!< @if Eng Radio frame type 4 and m sequence 3.
                                             @else 无线帧类型4，m序列3。 @endif */
    SLE_RADIO_FRAME_4_M4 = 12,     /*!< @if Eng Radio frame type 4 and m sequence 4.
                                             @else 无线帧类型4，m序列4。 @endif */
    SLE_RADIO_FRAME_4_M5 = 13,     /*!< @if Eng Radio frame type 4 and m sequence 5.
                                             @else 无线帧类型4，m序列5。 @endif */
    SLE_RADIO_FRAME_END
} sle_radio_frame_t;

/**
 * @brief 星闪发送/接收PHY类型，bitmap表示
 */
typedef enum {
    SLE_PHY_1M = 0x0,          /*!< @if Eng 1M PHY
                                         @else 1M PHY @endif */
    SLE_PHY_2M = 0x1,          /*!< @if Eng 2M PHY
                                         @else 2M PHY @endif */
    SLE_PHY_4M = 0x2,          /*!< @if Eng 4M PHY
                                         @else 4M PHY @endif */
    SLE_PHY_SUPPORT_NUM,
} sle_phy_tx_rx_t;

/**
 * @brief 星闪发送/接收导频密度指示，bitmap表示
 */
typedef enum {
    SLE_PHY_PILOT_DENSITY_4_TO_1  = 0x0,    /*!< @if Eng The pilot density is 4:1.
                                             @else 导频密度为4:1 @endif */
    SLE_PHY_PILOT_DENSITY_8_TO_1  = 0x1,    /*!< @if Eng The pilot density is 8:1.
                                             @else 导频密度为8:1 @endif */
    SLE_PHY_PILOT_DENSITY_16_TO_1 = 0x2,    /*!< @if Eng The pilot density is 16:1.
                                             @else 导频密度为16:1 @endif */
    SLE_PHY_PILOT_DENSITY_NO = 0x3,         /*!< @if Eng No pilot.
                                             @else 无导频 @endif */
    SLE_PHY_PILOT_DENSITY_NUM,
} sle_phy_tx_rx_pilot_density_t;

/**
 * @if Eng
 * @brief Struct of sle phy parameter.
 * @else
 * @brief 星闪phy参数
 * @endif
 */
typedef struct {
    uint8_t tx_format;          /*!< @if Eng Transmitted radio frame type, @ref sle_radio_frame_t
                                     @else 发送无线帧类型，参考 { @ref sle_radio_frame_t }。 @endif */
    uint8_t rx_format;          /*!< @if Eng Received radio frame type, @ref sle_radio_frame_t
                                     @else 接收无线帧类型，参考 { @ref sle_radio_frame_t }。 @endif */
    uint8_t tx_phy;             /*!< @if Eng Transmitted PHY, @ref sle_phy_tx_rx_t
                                     @else 发送PHY，参考 { @ref sle_phy_tx_rx_t }。 @endif */
    uint8_t rx_phy;             /*!< @if Eng Received PHY, @ref sle_phy_tx_rx_t
                                     @else 接收PHY，参考 { @ref sle_phy_tx_rx_t }。 @endif */
    uint8_t tx_pilot_density;   /*!< @if Eng Transmitted pilot density indicator, @ref sle_phy_tx_rx_pilot_density_t
                                     @else 发送导频密度指示，参考 { @ref sle_phy_tx_rx_pilot_density_t }。 @endif */
    uint8_t rx_pilot_density;   /*!< @if Eng Received pilot density indicator, @ref sle_phy_tx_rx_pilot_density_t
                                     @else 接收导频密度指示，参考 { @ref sle_phy_tx_rx_pilot_density_t }。 @endif */
    uint8_t g_feedback;         /*!< @if Eng Indicates the feedback type of the pre-transmitted link.
                                             The value range is 0 to 63.
                                     @else 先发链路反馈类型指示，取值范围0-63。 @endif */
    uint8_t t_feedback;         /*!< @if Eng Indicates the feedback type of the post-transmit link.
                                             The value range is 0-7.
                                     @else 后发链路反馈类型指示，取值范围0-7。 @endif */
} sle_set_phy_t;

/**
 * @if Eng
 * @brief Struct of Default sle connection parameter.
 * @else
 * @brief 星闪默认连接参数
 * @endif
 */
typedef struct {
    uint8_t  enable_filter_policy;      /*!< @if Eng Whether the filtering function is enabled on the link
                                             @else 链路是否打开过滤功能 @endif */
    uint8_t  initiate_phys;             /*!< @if Eng Link scanning communication bandwidth: 1:1M, 2:2M
                                             @else 链路扫描通信带宽： 1:1M, 2:2M @endif */
    uint8_t  gt_negotiate;              /*!< @if Eng Whether G-T interaction is performed during link establishment
                                             @else 链路建立时是否进行G和T交互 @endif */
    uint16_t scan_interval;             /*!< @if Eng Interval for scanning the peer
                                                     device during link establishment
                                             @else 链路建立时扫描对端设备的interval @endif */
    uint16_t scan_window;               /*!< @if Eng Scans the Windows operating system of the
                                                     peer device during link establishment.
                                             @else 链路建立时扫描对端设备的windows @endif */
    uint16_t min_interval;              /*!< @if Eng Minimum link scheduling interval
                                             @else 链路调度最小interval @endif */
    uint16_t max_interval;              /*!< @if Eng Maximum link scheduling interval
                                             @else 链路调度最大interval @endif */
    uint16_t timeout;                   /*!< @if Eng Link Timeout Interval
                                             @else 链路超时时间 @endif */
} sle_default_connect_param_t;

/**
 * @if Eng
 * @brief Callback invoked when connect state changed.
 * @par Callback invoked when connect state changed.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] conn_id    connection ID.
 * @param [in] addr       address.
 * @param [in] conn_state connection state { @ref sle_acb_state_t }.
 * @param [in] pair_state pairing state { @ref sle_pair_state_t }.
 * @param [in] disc_reason the reason of disconnect { @ref sle_disc_reason_t }.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  连接状态改变的回调函数。
 * @par    连接状态改变的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] conn_id    连接 ID。
 * @param [in] addr       地址。
 * @param [in] conn_state 连接状态 { @ref sle_acb_state_t }。
 * @param [in] pair_state 配对状态 { @ref sle_pair_state_t }。
 * @param [in] disc_reason 断链原因 { @ref sle_disc_reason_t }。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_connect_state_changed_callback)(uint16_t conn_id, const sle_addr_t *addr,
    sle_acb_state_t conn_state, sle_pair_state_t pair_state, sle_disc_reason_t disc_reason);

/**
 * @if Eng
 * @brief Callback invoked when connect parameter updated.
 * @par Callback invoked when connect parameter updated.
 * @attention 1.This function is called in SLE service context, should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] conn_id    connection ID.
 * @param [in] addr       address.
 * @param [in] status     error code.
 * @param [in] param      connection param.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  连接参数更新的回调函数。
 * @par    连接参数更新的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] conn_id    连接 ID。
 * @param [in] addr       地址。
 * @param [in] status     执行结果错误码。
 * @param [in] param      连接参数。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_connect_param_update_callback)(uint16_t conn_id, errcode_t status,
    const sle_connection_param_update_evt_t *param);

/**
 * @if Eng
 * @brief Callback invoked before the request for updating the connect parameter is complete.
 * @par Callback invoked before the request for updating the connect parameter is complete.
 * @attention 1.This function is called in SLE service context, should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] conn_id    connection ID.
 * @param [in] status     error code.
 * @param [in] param      connection param.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  连接参数更新请求完成前的回调函数。
 * @par    连接参数更新请求完成前的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] conn_id    连接 ID。
 * @param [in] status     执行结果错误码。
 * @param [in] param      连接参数。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_connect_param_update_req_callback)(uint16_t conn_id, errcode_t status,
    const sle_connection_param_update_req_t *param);

/**
 * @if Eng
 * @brief Callback invoked when authentication complete.
 * @par Callback invoked when authentication complete.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] conn_id connection ID.
 * @param [in] addr    address.
 * @param [in] status  error code.
 * @param [in] evt     authentication event.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  认证完成的回调函数。
 * @par    认证完成的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] conn_id 连接 ID。
 * @param [in] addr    地址。
 * @param [in] status  执行结果错误码。
 * @param [in] evt     认证事件。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_auth_complete_callback)(uint16_t conn_id, const sle_addr_t *addr, errcode_t status,
    const sle_auth_info_evt_t* evt);

/**
 * @if Eng
 * @brief Callback invoked when pairing complete.
 * @par Callback invoked when pairing complete.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] conn_id connection ID.
 * @param [in] addr    address.
 * @param [in] status  error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  配对完成的回调函数。
 * @par    配对完成的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] conn_id 连接 ID。
 * @param [in] addr    地址。
 * @param [in] status  执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_pair_complete_callback)(uint16_t conn_id, const sle_addr_t *addr, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when rssi read complete.
 * @par Callback invoked when rssi read complete.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] conn_id connection ID.
 * @param [in] rssi    rssi.
 * @param [in] status  error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  读取rssi的回调函数。
 * @par    读取rssi的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] conn_id 连接 ID。
 * @param [in] rssi    rssi。
 * @param [in] status  执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_read_rssi_callback)(uint16_t conn_id, int8_t rssi, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when set low latency complete.
 * @par Callback invoked when set low latency complete.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] status result of set low latency.
 * @param [in] addr   remote device address.
 * @param [in] rate   mouse report rate { @ref sle_low_latency_rate_t }.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  设置low latency的回调函数。
 * @par    设置low latency的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] status 设置low latency结果。
 * @param [in] addr   对端设备地址。
 * @param [in] rate   鼠标回报率 { @ref sle_low_latency_rate_t }。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_low_latency_callback)(uint8_t status, sle_addr_t *addr, uint8_t rate);

 /**
 * @if Eng
 * @brief Callback invoked when set PHY complete.
 * @par Callback invoked when set PHY complete.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in]  conn_id connection ID.
 * @param  [in]  status  result of setting the PHY.
 * @param  [in]  param   current PHY parameters { @ref sle_set_phy_t }.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  设置PHY的回调函数。
 * @par    设置PHY的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in]  conn_id 连接 ID。
 * @param  [in]  status  设置PHY结果。
 * @param  [in]  param   当前PHY参数 { @ref sle_set_phy_t }。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_set_phy_callback)(uint16_t conn_id, errcode_t status, const sle_set_phy_t *param);

/**
 * @if Eng
 * @brief Struct of SLE connection manager callback function.
 * @else
 * @brief SLE连接管理回调函数接口定义。
 * @endif
 */
typedef struct {
    sle_connect_state_changed_callback connect_state_changed_cb;         /*!< @if Eng Connect state changed callback.
                                                                            @else   连接状态改变回调函数。 @endif */
    sle_connect_param_update_req_callback connect_param_update_req_cb;   /*!< @if Eng Connect param updated callback.
                                                                            @else   连接参数更新回调函数。 @endif */
    sle_connect_param_update_callback connect_param_update_cb;           /*!< @if Eng Connect param updated callback.
                                                                            @else   连接参数更新回调函数。 @endif */
    sle_auth_complete_callback auth_complete_cb;                         /*!< @if Eng Authentication complete callback.
                                                                            @else   认证完成回调函数。 @endif */
    sle_pair_complete_callback pair_complete_cb;                         /*!< @if Eng Pairing complete callback.
                                                                            @else   配对完成回调函数。 @endif */
    sle_read_rssi_callback read_rssi_cb;                                 /*!< @if Eng Read rssi callback.
                                                                            @else   读取rssi回调函数。 @endif */
    sle_low_latency_callback low_latency_cb;                             /*!< @if Eng Set low latency callback.
                                                                            @else   设置low latency回调函数。 @endif */
    sle_set_phy_callback set_phy_cb;                                     /*!< @if Eng Set PHY callback.
                                                                            @else   设置PHY回调函数。 @endif */
} sle_connection_callbacks_t;

/**
 * @if Eng
 * @brief  Send connect request to remote device.
 * @par Description: Send connect request to remote device.
 * @param [in] addr address.
 * @retval error code, connection state change result will be returned at { @ref sle_connect_state_changed_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  发送连接请求。
 * @par Description: 发送连接请求。
 * @param [in] addr 地址。
 * @retval 执行结果错误码， 连接状态改变结果将在 { @ref sle_connect_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_connect_remote_device(const sle_addr_t *addr);

/**
 * @if Eng
 * @brief  Send disconnect request to remote device.
 * @par Description: Send disconnect request to remote device.
 * @param [in] addr address.
 * @retval error code, connection state change result will be returned at { @ref sle_connect_state_changed_callback }.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  发送断开连接请求。
 * @par Description: 发送断开连接请求。
 * @param [in] addr 地址。
 * @retval 执行结果错误码， 连接状态改变结果将在 { @ref sle_connect_state_changed_callback }中返回。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_disconnect_remote_device(const sle_addr_t *addr);

/**
 * @if Eng
 * @brief  Disconnect all connections.
 * @par Description: Disconnect all connections.
 * @retval ERRCODE_SUCC Success.
 * @retval Other        Failure. For details, see @ref errcode_t
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  断开所有连接。
 * @par Description: 断开所有连接。
 * @retval ERRCODE_SUCC 成功。
 * @retval Other        失败。参考 @ref errcode_t
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_disconnect_all_remote_device(void);

/**
 * @if Eng
 * @brief  Send connection parameter update request to remote device.
 * @par Description: Send connection parameter update request to remote device.
 * @param [in] params connection parameter.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  发送更新连接参数请求。
 * @par Description: 发送更新连接参数请求。
 * @param [in] params 连接参数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_update_connect_param(sle_connection_param_update_t *params);

/**
 * @if Eng
 * @brief  Send pairing request to remote device.
 * @par Description: Send pairing request to remote device.
 * @param [in] addr address.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  发送配对请求。
 * @par Description: 发送配对请求。
 * @param [in] addr 地址。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_pair_remote_device(const sle_addr_t *addr);

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
errcode_t sle_remove_paired_remote_device(const sle_addr_t *addr);

/**
 * @if Eng
 * @brief  Remove all pairing.
 * @par Description: Remove all pairing.
 * @param NULL
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  删除所有配对。
 * @par Description: 删除所有配对。
 * @param NULL
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_remove_all_pairs(void);

/**
 * @if Eng
 * @brief  Get paired device number.
 * @par Description: Get paired device number.
 * @param [out] number device number.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  获取配对设备数量。
 * @par Description: 获取配对设备数量。
 * @param [out] number 设备数量。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_get_paired_devices_num(uint16_t *number);

/**
 * @if Eng
 * @brief  Get paired device.
 * @par Description: Get paired device.
 * @param [out]   addr   linked list of device address.
 * @param [inout] number device number.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  获取配对设备。
 * @par Description: 获取配对设备。
 * @param [out]   addr   设备地址链表。
 * @param [inout] number 设备数量。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_get_paired_devices(sle_addr_t *addr, uint16_t *number);

/**
 * @if Eng
 * @brief  Get bonded device.
 * @par Description: Get bonded device.
 * @param [out]   addr   linked list of device address.
 * @param [inout] number device number.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  获取绑定设备。
 * @par Description: 获取绑定设备。
 * @param [out]   addr   设备地址链表。
 * @param [inout] number 设备数量。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_get_bonded_devices(sle_addr_t *addr, uint16_t *number);

/**
 * @if Eng
 * @brief  Get pair state.
 * @par Description: Get pair state.
 * @param [in]  addr  device address.
 * @param [out] state pair state { @ref sle_pair_state_t }.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  获取配对状态。
 * @par Description: 获取配对状态。
 * @param [in]  addr  设备地址。
 * @param [out] state 配对状态 { @ref sle_pair_state_t }。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_get_pair_state(const sle_addr_t *addr, uint8_t *state);

/**
 * @if Eng
 * @brief  Read remote device rssi value.
 * @par Description: Read remote device rssi value.
 * @param [in]  conn_id connection ID.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  读取对端设备rssi值。
 * @par Description: 读取对端设备rssi值。
 * @param [in]  conn_id 连接 ID。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_read_remote_device_rssi(uint16_t conn_id);

/**
 * @if Eng
 * @brief  Set acb evt param.
 * @par Description: Set acb evt param.
 * @param [in]  conn_id connection ID.
 * @param [in]  evt_intv evt retry interval.
 * @param [in]  evt_num evt retry num.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置ACB链路参数
 * @par Description: 设置ACB链路参数。
 * @param [in]  conn_id 连接 ID。
 * @param [in]  evt_intv 重传间隔。
 * @param [in]  evt_num 重传次数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_acb_evt_param(uint16_t conn_id, uint16_t evt_intv, uint8_t evt_num);

/**
 * @if Eng
 * @brief  Set sle phy.
 * @par Description: Set sle phy.
 * @param [in]  conn_id connection ID.
 * @param [in]  param sle phy param.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置PHY参数
 * @par Description: 设置PHY参数。
 * @param [in]  conn_id 连接 ID。
 * @param [in]  param 星闪的PHY参数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_phy_param(uint16_t conn_id, sle_set_phy_t *param);

/**
 * @if Eng
 * @brief  Set Modulation and Coding Scheme.
 * @par Description: Set Modulation and Coding Scheme.
 * @param [in]  conn_id connection ID.
 * @param [in]  mcs Modulation and Coding Scheme.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置调制与编码策略。
 * @par Description: 设置调制与编码策略。
 * @param [in]  conn_id 连接 ID。
 * @param [in]  mcs 调制与编码策略索引值。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_mcs(uint16_t conn_id, uint8_t mcs);

/**
 * @if Eng
 * @brief  Set Maximum number of preferred transmit payload bytes on the connected link.
 * @par Description: Maximum number of preferred transmit payload bytes on the connected link.
 * @param [in]  conn_id connection ID.
 * @param [in]  tx_octets 连接链路上所偏好的最大传输payload字节数.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置连接链路上所偏好的最大传输payload字节数
 * @par Description: 设置连接链路上所偏好的最大传输payload字节数。
 * @param [in]  conn_id 连接 ID。
 * @param [in]  tx_octets 连接链路上所偏好的最大传输payload字节数.
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_data_len(uint16_t conn_id, uint16_t tx_octets);

/**
 * @if Eng
 * @brief  use this function to set sle default connect param.
 * @par Description: use this function to set sle default connect param.
 * @param [in]  param sle default connect param.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置 sle 默认连接参数
 * @par Description: 设置 sle 默认连接参数。
 * @param [in]  param 星闪的默认连接参数。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_default_connection_param_set(sle_default_connect_param_t *set_param);

/**
 * @if Eng
 * @brief  Register SLE connection manager callbacks.
 * @par Description: Register SLE connection manager callbacks.
 * @param [in] func Callback function.
 * @retval error code.
 * @else
 * @brief  注册SLE连接管理回调函数。
 * @par Description: 注册SLE连接管理回调函数。
 * @param [in] func 回调函数。
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t sle_connection_register_callbacks(sle_connection_callbacks_t *func);

/**
 * @if Eng
 * @brief Use this funtion to config customize information.
 * @par   Use this funtion to config customize information.
 * @attention NULL
 * @param  [in] ble_pwr ble max power.
 * @param  [in] sle_pwr sle max power.
 * @retval error code.
 * @par Dependency:
 * @li  nv_common.h
 * @else
 * @brief  配置定制化信息
 * @par    配置定制化信息
 * @attention 无
 * @param  [in] sle_pwr ble 最大功率.
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  bt_data_config.h
 * @endif
 */
errcode_t sle_customize_max_pwr(int8_t ble_pwr, int8_t sle_pwr);

#ifdef __cplusplus
}
#endif
#endif /* SLE_CONNECTION_MANAGER */
/**
 * @}
 */
