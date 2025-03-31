/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 * Description: SLE Device Announce, Seek Manager, module.
 */

/**
 * @defgroup sle_device_announce Device Announce & Seek Manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_DEVICE_ANC_SEK
#define SLE_DEVICE_ANC_SEK

#include <stdint.h>
#include "sle_errcode.h"
#include "sle_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @if Eng
 * @brief  Maximum of announce ID.
 * @else
 * @brief  设备公开多路最大值。
 * @endif
 */
#define SLE_ANNOUNCE_ID_MAX 16

/**
 * @if Eng
 * @brief  Maximum of scan PHY num.
 * @else
 * @brief  设备发现PHY最大值。
 * @endif
 */
#define SLE_SEEK_PHY_NUM_MAX 3

/**
 * @if Eng
 * @brief  announce level.
 * @else
 * @brief  被发现方可发现等级
 * @endif
 */
typedef enum {
    SLE_ANNOUNCE_LEVEL_NONE,     /*!< @if Eng announce level none, reserve
                                       @else   不可见发现，预留 @endif */
    SLE_ANNOUNCE_LEVEL_NORMAL,   /*!< @if Eng announce level normal
                                       @else   一般可发现 @endif */
    SLE_ANNOUNCE_LEVEL_PRIORITY, /*!< @if Eng announce level priority, reserve
                                       @else   优先可发现，预留 @endif */
    SLE_ANNOUNCE_LEVEL_PAIRED,   /*!< @if Eng announce level paired, reserve
                                       @else   被曾配对过的设备发现，预留 @endif */
    SLE_ANNOUNCE_LEVEL_SPECIAL,  /*!< @if Eng announce level special
                                       @else   被指定设备发现 @endif */
} sle_announce_level_t;

/**
 * @if Eng
 * @brief  G/T role negotiation indication.
 * @else
 * @brief  G/T 角色协商指示。
 * @endif
 */
typedef enum {
    SLE_ANNOUNCE_ROLE_T_CAN_NEGO = 0, /*!< @if Eng Expect to be T, negotiable
                                               @else   期望做T可协商 @endif */
    SLE_ANNOUNCE_ROLE_G_CAN_NEGO,     /*!< @if Eng Expect to be G, negotiable
                                               @else   期望做G可协商 @endif */
    SLE_ANNOUNCE_ROLE_T_NO_NEGO,      /*!< @if Eng Expect to be T, non-negotiable
                                               @else   期望做T不可协商 @endif */
    SLE_ANNOUNCE_ROLE_G_NO_NEGO       /*!< @if Eng Expect to be G, non-negotiable
                                               @else   期望做G不可协商 @endif */
} sle_announce_gt_role_t;

/**
 * @if Eng
 * @brief  announce mode.
 * @else
 * @brief  设备公开类型。
 * @endif
 */
typedef enum {
    SLE_ANNOUNCE_MODE_NONCONN_NONSCAN      = 0x00, /*!< @if Eng non-connectable, non-scannable.
                                                         @else   不可连接不可扫描。 @endif */
    SLE_ANNOUNCE_MODE_CONNECTABLE_NONSCAN  = 0x01, /*!< @if Eng connectable, non-scannable.
                                                         @else   可连接不可扫描。 @endif */
    SLE_ANNOUNCE_MODE_NONCONN_SCANABLE     = 0x02, /*!< @if Eng non-connectable, scannable.
                                                         @else   不可连接可扫描。 @endif */
    SLE_ANNOUNCE_MODE_CONNECTABLE_SCANABLE = 0x03, /*!< @if Eng connectable, scannable.
                                                         @else   可连接可扫描。 @endif */
    SLE_ANNOUNCE_MODE_CONNECTABLE_DIRECTED = 0x07, /*!< @if Eng connectable, scannable, directed.
                                                         @else   可连接可扫描定向。 */
} sle_announce_mode_t;

/**
 * @if Eng
 * @brief  Seek phy type.
 * @else
 * @brief  设备发现PHY类型。
 * @endif
 */
typedef enum {
    SLE_SEEK_PHY_1M = 0x1, /*!< @if Eng 1M PHY
                                     @else   1M PHY @endif */
    SLE_SEEK_PHY_2M = 0x2, /*!< @if Eng 2M PHY
                                     @else   2M PHY @endif */
    SLE_SEEK_PHY_4M = 0x4, /*!< @if Eng 4M PHY
                                     @else   4M PHY @endif */
} sle_seek_phy_t;

/**
 * @if Eng
 * @brief  seek type.
 * @else
 * @brief  设备发现类型。
 * @endif
 */
typedef enum {
    SLE_SEEK_PASSIVE = 0x00, /*!< @if Eng passive seek
                                            @else   被动扫描 @endif */
    SLE_SEEK_ACTIVE  = 0x01, /*!< @if Eng active seek
                                            @else   主动扫描 @endif */
} sle_seek_type_t;

/**
 * @if Eng
 * @brief  Seek filter type.
 * @else
 * @brief  设备发现过滤类型。
 * @endif
 */
typedef enum {
    SLE_SEEK_FILTER_ALLOW_ALL   = 0x00, /*!< @if Eng allow all
                                                       @else   允许来自任何人的设备发现数据包 @endif */
    SLE_SEEK_FILTER_ALLOW_WLST  = 0x01, /*!< @if Eng allow only white list, reserve
                                                       @else   允许来自白名单设备的设备发现数据包，预留 @endif */
} sle_seek_filter_t;

/**
 * @if Eng
 * @brief  modulation mode
 * @else
 * @brief  调制模式
 * @endif
 */
typedef enum {
    SLE_MCS_00 = 0,                                     /*!< @if Eng MCS0:  BPSK1/4
                                                             @else   MCS0:  BPSK1/4 @endif */
    SLE_MCS_01,                                         /*!< @if Eng MCS1:  BPSK3/8
                                                             @else   MCS1:  BPSK3/8 @endif */
    SLE_MCS_02,                                         /*!< @if Eng MCS2:  QPSK1/4
                                                             @else   MCS2:  QPSK1/4 @endif */
    SLE_MCS_03,                                         /*!< @if Eng MCS3:  QPSK3/8
                                                             @else   MCS3:  QPSK3/8 @endif */
    SLE_MCS_04,                                         /*!< @if Eng MCS4:  QPSK1/2
                                                             @else   MCS4:  QPSK1/2 @endif */
    SLE_MCS_05,                                         /*!< @if Eng MCS5:  QPSK5/8
                                                             @else   MCS5:  QPSK5/8 @endif */
    SLE_MCS_06,                                         /*!< @if Eng MCS6:  QPSK3/4
                                                             @else   MCS6:  QPSK3/4 @endif */
    SLE_MCS_07,                                         /*!< @if Eng MCS7:  QPSK7/8
                                                             @else   MCS7:  QPSK7/8 @endif */
    SLE_MCS_08,                                         /*!< @if Eng MCS8:  QPSK 1
                                                             @else   MCS8:  QPSK 1 @endif */
    SLE_MCS_09,                                         /*!< @if Eng MCS9:  8PSK5/8
                                                             @else   MCS9:  8PSK5/8 @endif */
    SLE_MCS_10,                                         /*!< @if Eng MCS10: 8PSK3/4
                                                             @else   MCS10: 8PSK3/4 @endif */
    SLE_MCS_11,                                         /*!< @if Eng MCS11: 8PSK7/8
                                                             @else   MCS11: 8PSK7/8 @endif */
    SLE_MCS_12,                                         /*!< @if Eng MCS12：8PSK 1
                                                             @else   MCS12：8PSK 1 @endif */
    SLE_MCS_MAX,
} sle_mcs_t;

/**
 * @if Eng
 * @brief  filtering policies
 * @else
 * @brief  过滤策略
 * @endif
 */
typedef enum {
    SLE_ANNOUNCE_FLT_ANY_SEEK_ANY_CONNECT,          /*!< @if Eng accept all seek_req/conn_req
                                                    @else   接受所有seek_req/conn_req @endif */
    SLE_ANNOUNCE_FLT_WHITE_SEEK_ANY_CONNECT,        /*!< @if Eng only seek_req that meet the filter will be accepted
                                                    @else   只接受符合过滤器的seek_req @endif */
    SLE_ANNOUNCE_FLT_ANY_SEEK_WHITE_CONNECT,        /*!< @if Eng only conn_req that meet the filter will be accepted
                                                    @else   只接受符合过滤器的conn_req @endif */
    SLE_ANNOUNCE_FLT_WHITE_SEEK_WHITE_CONNECT       /*!< @if Eng accept seek_req/conn_req that conform to the filter
                                                    @else   接受符合过滤器的seek_req/conn_req @endif */
} sle_filter_policy_t;

/**
 * @if Eng
 * @brief  Connection parameter, only valid in role G.
 * @else
 * @brief  连接参数，做G时有效。
 * @endif
 */
typedef struct sle_conn_param {
    uint16_t interval_min;          /*!< @if Eng minimum of connection interval
                                         @else   连接间隔最小取值，取值范围[0x001E,0x3E80]，
                                                 时间 = N * 0.25ms, 时间范围[7.5ms,4s] @endif */
    uint16_t interval_max;          /*!< @if Eng maximum of connection interval
                                         @else   连接间隔最大取值，取值范围[0x001E,0x3E80]，
                                                 时间 = N * 0.25ms, 时间范围[7.5ms,4s] @endif */
    uint16_t max_latency;           /*!< @if Eng maximum of sleep connection latency
                                         @else   最大休眠连接间隔，取值范围[0x0000,0x01F3]，默认0x0000 @endif */
    uint16_t supervision_timeout;   /*!< @if Eng maximum of timeout
                                         @else   最大超时时间，取值范围[0x000A,0x0C80]，时间 = N * 10ms，
                                                 时间范围是[100ms,32s] @endif */
    uint16_t min_ce_length;         /*!< @if Eng minimum of connection event
                                         @else   推荐的连接事件的最小取值，取值范围[0x0000,0xFFFF]，
                                                 时间 = N * 0.125ms @endif */
    uint16_t max_ce_length;         /*!< @if Eng maximum of connection event
                                         @else   推荐的连接事件的最大取值，取值范围[0x0000,0xFFFF]，
                                                 时间 = N * 0.125ms @endif */
} sle_conn_param_t;

/**
 * @if Eng
 * @brief  Announce parameter.
 * @else
 * @brief  设备公开参数。
 * @endif
 */
typedef struct sle_announce_param {
    uint8_t  announce_handle;               /*!< @if Eng announce handle
                                                 @else   设备公开句柄，取值范围[0, 0xFF] @endif */
    uint8_t  announce_mode;                 /*!< @if Eng announce mode { @ref sle_announce_mode_t }
                                                 @else   设备公开类型， { @ref sle_announce_mode_t } @endif */
    uint8_t  announce_gt_role;              /*!< @if Eng G/T role negotiation indication
                                                         { @ref sle_announce_gt_role_t }
                                                 @else   G/T 角色协商指示，
                                                         { @ref sle_announce_gt_role_t } @endif */
    uint8_t  announce_level;                /*!< @if Eng announce level
                                                         { @ref sle_announce_level_t }
                                                 @else   发现等级，
                                                         { @ref sle_announce_level_t } @endif */
    uint32_t announce_interval_min;         /*!< @if Eng minimum of announce interval
                                                 @else   最小设备公开周期, 0x000020~0xffffff, 单位125us @endif */
    uint32_t announce_interval_max;         /*!< @if Eng maximum of announce interval
                                                 @else   最大设备公开周期, 0x000020~0xffffff, 单位125us @endif */
    uint8_t  announce_channel_map;          /*!< @if Eng announce channel map
                                                 @else   设备公开信道, 0:76, 1:77, 2:78 @endif */
    int8_t   announce_tx_power;             /*!< @if Eng adv transmit power
                                                 @else   广播发射功率，单位dbm, 取值范围[-127, 20],
                                                         0x7F：不设置特定发送功率 @endif */
    sle_addr_t own_addr;                    /*!< @if Eng own address
                                                 @else   本端地址 @endif */
    sle_addr_t peer_addr;                   /*!< @if Eng peer address
                                                 @else   对端地址 @endif */
    uint16_t conn_interval_min;             /*!< @if Eng minimum of connection interval
                                                 @else   连接间隔最小取值，取值范围[0x001E,0x3E80]，
                                                         announce_gt_role 为 SLE_ANNOUNCE_ROLE_T_NO_NEGO
                                                         时无需配置 @endif */
    uint16_t conn_interval_max;             /*!< @if Eng maximum of connection interval
                                                 @else   连接间隔最大取值，取值范围[0x001E,0x3E80]，
                                                         announce_gt_role 为 SLE_ANNOUNCE_ROLE_T_NO_NEGO
                                                         无需配置 @endif */
    uint16_t conn_max_latency;              /*!< @if Eng max connection latency
                                                 @else   最大休眠连接间隔，取值范围[0x0000,0x01F3]，
                                                         announce_gt_role 为 SLE_ANNOUNCE_ROLE_T_NO_NEGO
                                                         无需配置 @endif */
    uint16_t conn_supervision_timeout;      /*!< @if Eng connect supervision timeout
                                                 @else   最大超时时间，取值范围[0x000A,0x0C80]，
                                                         announce_gt_role 为 SLE_ANNOUNCE_ROLE_T_NO_NEGO
                                                         无需配置 @endif */
    void *ext_param;                        /*!< @if Eng extend parameter, default value is NULL
                                                 @else   扩展设备公开参数, 缺省时置空 @endif */
} sle_announce_param_t;

/**
 * @if Eng
 * @brief  Announce data.
 * @else
 * @brief  设备公开数据。
 * @endif
 */
typedef struct sle_announce_data {
    uint16_t announce_data_len; /*!< @if Eng announce data length
                                      @else   设备公开数据长度 @endif */
    uint16_t seek_rsp_data_len;  /*!< @if Eng scan response data length
                                      @else   扫描响应数据长度 @endif */
    uint8_t  *announce_data;    /*!< @if Eng announce data
                                      @else   设备公开数据 @endif */
    uint8_t  *seek_rsp_data;     /*!< @if Eng scan response data
                                      @else   扫描响应数据 @endif */
} sle_announce_data_t;

/**
 * @if Eng
 * @brief  Announce enable parameter.
 * @else
 * @brief  设备公开使能参数。
 * @endif
 */
typedef struct sle_announce_enable {
    uint8_t enable;               /*!< @if Eng enable flag
                                       @else   0x0：关闭设备公开, 0x1: 使能设备公开 @endif */
    uint8_t announce_handle;     /*!< @if Eng announce handle
                                       @else   设备公开句柄 @endif */
    uint16_t duration;            /*!< @if Eng announce duration
                                       @else   0x0:设备公开时间无限制, 0x1~0xFFFF:设备公开时间=N*10ms @endif */
    uint8_t max_announce_events; /*!< @if Eng maximum of announce events
                                       @else   0x0:设备公开事件个数无限制, 0x1~0xFF:设备公开事件个数限制 @endif */
} sle_announce_enable_t;

/**
 * @if Eng
 * @brief  Seek scan parameter.
 * @else
 * @brief  设备发现扫描参数。
 * @endif
 */
typedef struct sle_seek_params {
    uint8_t own_addr_type;                        /*!< @if Eng own address type
                                                       @else   本端地址类型 @endif */
    uint8_t filter_duplicates;                    /*!< @if Eng duplicates filter
                                                       @else   重复过滤开关，0：关闭，1：开启 @endif */
    uint8_t seek_filter_policy;                   /*!< @if Eng scan filter policy { @ref sle_seek_filter_t }
                                                       @else   扫描设备使用的过滤类型，
                                                               { @ref sle_seek_filter_t } @endif */
    uint8_t seek_phys;                            /*!< @if Eng scan PHY type { @ref sle_seek_phy_t }
                                                       @else   扫描设备所使用的PHY，{ @ref sle_seek_phy_t }
                                                       @endif */
    uint8_t seek_type[SLE_SEEK_PHY_NUM_MAX];      /*!< @if Eng scan type { @ref sle_seek_scan_t }
                                                       @else   扫描类型，{ @ref sle_seek_type_t }
                                                       @endif */
    uint16_t seek_interval[SLE_SEEK_PHY_NUM_MAX]; /*!< @if Eng scan interval
                                                       @else   扫描间隔，取值范围[0x0004, 0xFFFF]，time = N * 0.125ms
                                                       @endif */
    uint16_t seek_window[SLE_SEEK_PHY_NUM_MAX];   /*!< @if Eng scan window
                                                       @else   扫描窗口，取值范围[0x0004, 0xFFFF]，time = N * 0.125ms
                                                       @endif */
} sle_seek_param_t;

/**
 * @if Eng
 * @brief  Seek result.
 * @else
 * @brief  扫描结果报告设备信息。
 * @endif
 */
typedef struct sle_seek_result_info {
    uint8_t event_type;                /*!< @if Eng event type
                                            @else   上报事件类型 @endif */
    sle_addr_t addr;                   /*!< @if Eng address
                                            @else   地址 @endif */
    sle_addr_t direct_addr;            /*!< @if Eng direct address
                                            @else   定向发现地址 @endif */
    int8_t rssi;                      /*!< @if Eng rssi
                                            @else   信号强度指示，取值范围[-127dBm, 20dBm]，0x7F表示不提供信号强度指示
                                            @endif */
    uint8_t data_status;               /*!< @if Eng data status
                                            @else   数据状态 @endif */
    uint8_t data_length;               /*!< @if Eng data length
                                            @else   数据长度 @endif */
    uint8_t *data;                     /*!< @if Eng data
                                            @else   数据 @endif */
} sle_seek_result_info_t;

/**
 * @if Eng
 * @brief Callback invoked when connect announce enabled.
 * @par Callback invoked when connect announce enabled.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] announce_id announce ID.
 * @param [in] status       error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  设备公开使能的回调函数。
 * @par    设备公开使能的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] announce_id 公开ID。
 * @param [in] status       执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_announce_enable_callback)(uint32_t announce_id, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when connect announce disabled.
 * @par Callback invoked when connect announce disabled.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] announce_id announce ID.
 * @param [in] status       error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  设备公开关闭的回调函数。
 * @par    设备公开关闭的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] announce_id 公开ID。
 * @param [in] status       执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_announce_disable_callback)(uint32_t announce_id, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when announce terminated.
 * @par Callback invoked when announce terminated.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] announce_id announce ID.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  设备公开停止的回调函数。
 * @par    设备公开停止的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] announce_id 公开ID。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_announce_terminal_callback)(uint32_t announce_id);

/**
 * @if Eng
 * @brief Callback invoked when announce remove.
 * @par Callback invoked when announce remove.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param  [in] announce_id announce ID.
 * @param  [in]  status       error code.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  删除广播的回调函数。
 * @par    删除广播的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param  [in] announce_id 公开ID。
 * @param  [in]  status       执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_announce_remove_callback)(uint32_t announce_id, errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when seek enabled.
 * @par Callback invoked when seek enabled.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] status error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  扫描使能的回调函数。
 * @par    扫描使能的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] status 执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_start_seek_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when scan disabled.
 * @par Callback invoked when scan disabled.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] status error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  扫描关闭的回调函数。
 * @par    扫描关闭的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] status 执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_seek_disable_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when receive seek data.
 * @par Callback invoked when receive seek data.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] seek_result_data seek result.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  扫描结果上报的回调函数。
 * @par    扫描结果上报的回调函数。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] seek_result_data 扫描结果数据。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_seek_result_callback)(sle_seek_result_info_t *seek_result_data);

/**
 * @if Eng
 * @brief Callback invoked when SLE stack enable.
 * @par Callback invoked when SLE stack enable.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] status error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  SLE协议栈使能。
 * @par    SLE协议栈使能。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] status 执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_enable_callback)(errcode_t status);

/**
 * @if Eng
 * @brief Callback invoked when SLE stack disable.
 * @par Callback invoked when SLE stack disable.
 * @attention 1.This function is called in SLE service context,should not be blocked or do long time waiting.
 * @attention 2.The memories of pointer are requested and freed by the SLE service automatically.
 * @param [in] status error code.
 * @retval #void no return value.
 * @par Dependency:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @else
 * @brief  SLE协议栈去使能。
 * @par    SLE协议栈去使能。
 * @attention  1. 该回调函数运行于SLE service线程，不能阻塞或长时间等待。
 * @attention  2. 指针由SLE service申请内存，也由SLE service释放，回调中不应释放。
 * @param [in] status 执行结果错误码。
 * @retval 无返回值。
 * @par 依赖:
 * @li  sle_common.h
 * @see sle_connection_callbacks_t
 * @endif
 */
typedef void (*sle_disable_callback)(errcode_t status);

/**
 * @if Eng
 * @brief The callback interface for sle dfr function.
 * @retval no return value
 * @else
 * @brief sle协议栈dfr流程
 * @retval 无返回值
 * @endif
 */
typedef void (*sle_dfr_callback)(void);

/**
 * @if Eng
 * @brief Struct of SLE device announce callback function.
 * @else
 * @brief SLE 设备公开回调函数接口定义。
 * @endif
 */
typedef struct {
    sle_enable_callback sle_enable_cb;                      /*!< @if Eng SLE stack enable callback.
                                                                 @else   SLE协议栈使能回调函数。 @endif */
    sle_disable_callback sle_disable_cb;                    /*!< @if Eng SLE stack disable callback.
                                                                 @else   SLE协议栈去使能回调函数。 @endif */
    sle_announce_enable_callback announce_enable_cb;        /*!< @if Eng device announce enable callback.
                                                                 @else   设备公开使能回调函数。 @endif */
    sle_announce_disable_callback announce_disable_cb;      /*!< @if Eng device announce disable callback.
                                                                 @else   设备公开关闭回调函数。 @endif */
    sle_announce_terminal_callback announce_terminal_cb;    /*!< @if Eng device announce terminated callback.
                                                                 @else   设备公开停止回调函数。 @endif */
    sle_announce_remove_callback announce_remove_cb;        /*!< @if Eng device announce remove callback.
                                                                 @else   设备公开停止回调函数。 @endif */
    sle_start_seek_callback seek_enable_cb;                 /*!< @if Eng scan enable callback.
                                                                 @else   扫描使能回调函数。 @endif */
    sle_seek_disable_callback seek_disable_cb;              /*!< @if Eng scan disable callback.
                                                                 @else   扫描关闭回调函数。 @endif */
    sle_seek_result_callback seek_result_cb;                /*!< @if Eng scan result callback.
                                                                 @else   扫描结果回调函数。 @endif */
    sle_dfr_callback sle_dfr_cb;                            /*!< @if Eng General callback for sle dfr.
                                                                 @else dfr回调函数 @endif */
} sle_announce_seek_callbacks_t;

/**
 * @if Eng
 * @brief  Enable SLE stack.
 * @par Description: Enable SLE stack.
 * @param NULL
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  使能SLE协议栈。
 * @par Description: 使能SLE协议栈。
 * @param NULL
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t enable_sle(void);

/**
 * @if Eng
 * @brief  Disable SLE stack.
 * @par Description: Disable SLE stack.
 * @param NULL
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  关闭SLE协议栈。
 * @par Description: 关闭SLE协议栈。
 * @param NULL
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t disable_sle(void);

/**
 * @if Eng
 * @brief  Set local device address.
 * @par Description: Set local device address.
 * @param [in] addr address.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  设置本地设备地址。
 * @par Description: 设置本地设备地址。
 * @param [in] addr 地址。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_set_local_addr(sle_addr_t *addr);

/**
 * @if Eng
 * @brief  Get local device address.
 * @par Description: Get local device address.
 * @param [out] addr address.
 * @retval error code.
 * @par Depends:
 * @li sle_common.h
 * @else
 * @brief  获取本地设备地址。
 * @par Description: 获取本地设备地址。
 * @param [out] addr 地址。
 * @retval 执行结果错误码。
 * @par 依赖：
 * @li sle_common.h
 * @endif
 */
errcode_t sle_get_local_addr(sle_addr_t *addr);

/**
 * @if Eng
 * @brief Use this funtion to set local device name.
 * @par   Use this funtion to set local device name.
 * @attention NULL
 * @param  [in] name local device name.
 * @param  [in] len  local device name length.
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  设置本地设备名称。
 * @par    设置本地设备名称。
 * @attention 无
 * @param  [in] name 本地设备名称。
 * @param  [in] len  本地设备名称长度，包括结束符\0。
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_set_local_name(const uint8_t *name, uint8_t len);

/**
 * @if Eng
 * @brief Use this funtion to get local device name.
 * @par   Use this funtion to get local device name.
 * @attention NULL
 * @param   [out]   name local device name.
 * @param   [inout] len  as input parameter, the buffer size of user,
                         as output parameter, the length of local device name.
 * @retval error code
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  获取本地设备名称。
 * @par    获取本地设备名称。
 * @attention 无
 * @param   [out]   name 本地设备名称。
 * @param   [inout] len  作为入参时为用户分配的内存大小，作为出参时为本地设备名称长度。
 * @retval 执行结果错误码
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_get_local_name(uint8_t *name, uint8_t *len);

/**
 * @if Eng
 * @brief Use this funtion to set announce data.
 * @par   Use this funtion to set announce data
 * @attention NULL
 * @param  [in] announce_id announce ID
 * @param  [in] data         a pointer to the announce data
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  设置设备公开数据。
 * @par    设置设备公开数据。
 * @attention 无
 * @param  [in] announce_id 设备公开 ID
 * @param  [in] data         设备公开数据
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_set_announce_data(uint8_t announce_id, const sle_announce_data_t *data);

/**
 * @if Eng
 * @brief Use this funtion to remove announce.
 * @par   Use this funtion to remove announce
 * @attention NULL
 * @param  [in] announce_id announce ID
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  删除广播。
 * @par    删除广播。
 * @attention 无
 * @param  [in] announce_id 设备公开 ID
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_remove_announce(uint8_t announce_id);

/**
 * @if Eng
 * @brief Use this funtion to set announce parameter.
 * @par   Use this funtion to set announce parameter
 * @attention NULL
 * @param  [in] announce_id announce ID
 * @param  [in] param        a pointer to the announce parameter
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  设置设备公开参数。
 * @par    设置设备公开参数。
 * @attention 无
 * @param  [in] announce_id 设备公开 ID
 * @param  [in] param        设备公开参数
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_set_announce_param(uint8_t announce_id, const sle_announce_param_t *param);

/**
 * @if Eng
 * @brief Use this funtion to start announce.
 * @par   Use this funtion to start announce.
 * @attention NULL
 * @param  [in] announce_id announce ID
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  开始设备公开。
 * @par    开始设备公开。
 * @attention 无
 * @param  [in] announce_id 设备公开 ID
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_start_announce(uint8_t announce_id);

/**
 * @if Eng
 * @brief Use this funtion to stop announce.
 * @par   Use this funtion to stop announce.
 * @attention NULL
 * @param  [in] announce_id announce ID
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  结束设备公开。
 * @par    结束设备公开。
 * @attention 无
 * @param  [in] announce_id 设备公开 ID
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_stop_announce(uint8_t announce_id);

/**
 * @if Eng
 * @brief Use this funtion to set device seek parameter.
 * @par   Use this funtion to set device seek parameter.
 * @attention NULL
 * @param  [in] param device device seek parameter.
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  设置设备公开扫描参数。
 * @par    设置设备公开扫描参数。
 * @attention 无
 * @param  [in] param 设备公开扫描参数。
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_set_seek_param(sle_seek_param_t *param);

/**
 * @if Eng
 * @brief Use this funtion to start device seek.
 * @par   Use this funtion to start device seek.
 * @attention NULL
 * @param  NULL
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  开始设备公开扫描。
 * @par    开始设备公开扫描。
 * @attention 无
 * @param  NULL
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_start_seek(void);

/**
 * @if Eng
 * @brief Use this funtion to stop device seek.
 * @par   Use this funtion to stop device seek.
 * @attention NULL
 * @param  NULL
 * @retval error code.
 * @par Dependency:
 * @li  sle_common.h
 * @else
 * @brief  停止设备公开扫描。
 * @par    停止设备公开扫描。
 * @attention 无
 * @param  NULL
 * @retval 执行结果错误码。
 * @par 依赖:
 * @li  sle_common.h
 * @endif
 */
errcode_t sle_stop_seek(void);

/**
 * @if Eng
 * @brief  Register SLE device device_seek callbacks.
 * @par Description: Register SLE device device_seek callbacks.
 * @param [in] func Callback function.
 * @retval error code.
 * @else
 * @brief  注册SLE设备发现回调函数。
 * @par Description: 注册SLE设备发现回调函数。
 * @param [in] func 回调函数。
 * @retval 执行结果错误码。
 * @endif
 */
errcode_t sle_announce_seek_register_callbacks(sle_announce_seek_callbacks_t *func);

#ifdef __cplusplus
}
#endif
#endif /* SLE_DEVICE_ANC_SEK */
/**
 * @}
 */
