/**
 * Copyright (c) @CompanyNameMagicTag 2022. All rights reserved.
 *
 * Description: SLE Hadm Manager module.
 */

/**
 * @defgroup sle_hadm_manager hadm manager API
 * @ingroup  SLE
 * @{
 */

#ifndef SLE_HADM_MANAGER_H
#define SLE_HADM_MANAGER_H

#include <stdint.h>
#include "sle_errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SLE_CS_CAPS_LEN 0x15 /*!< @if Eng size of channel sounding capability set
                                    @else channel sounding 能力集大小 @endif */
#define SLE_CS_IQ_REPORT_COUNT 20 /*!< @if Eng Channel sounding IQ report
                                    @else channel sounding iq 上报 @endif */
/**
 * @if Eng
 * @brief Enum of sle channel sounding enable state.
 * @else
 * @brief channel sounding 使能状态
 * @endif
 */
typedef enum {
    SLE_CHANNEL_SOUNDING_DISABLE,
    SLE_CHANNEL_SOUNDING_ENABLE,
    SLE_CHANNEL_SOUNDING_MAX,
} sle_channel_sounding_state_t;

/**
 * @if Eng
 * @brief Enum of set sle channel sounding param.
 * @else
 * @brief 设置channel sounding 参数
 * @endif
 */
typedef struct {
    uint8_t  is_cs_param_chg; /*!< @if Eng Whether ranging connection parameters need to be modified.
                                @else 是否需要更改测距连接参数 @endif */
    uint8_t  freq_space; /*!< @if Eng Frequency spacing.
                                @else 频率间隔 @endif */
    uint8_t  con_anchor_num; /*!< @if Eng Number of anchors to be connected.
                                @else 需要连接的锚点数量 @endif */
    uint8_t  refresh_rate; /*!< @if Eng Refresh rate.
                                @else 刷新率 @endif */
    uint16_t acb_interval; /*!< @if Eng ACB link period calculated based on parameters.
                                @else 根据参数计算得到的acb链路周期 @endif */
    uint16_t cs_interval;  /*!< @if Eng Ranging period calculated based on parameters.
                                @else 根据参数计算得到的测距周期 @endif */
    uint16_t posalg_freq;  /*!< @if Eng Posalg Frequency.
                                @else 算法频率 @endif */
    uint8_t  glp_mode;     /*!< @if Eng glp mode.
                                @else 2.4GHz 模式 @endif */
} sle_set_channel_sounding_param_ex_t;

/**
 * @if Eng
 * @brief Enum of sle channel sounding iq report
 * @else
 * @brief channel sounding IQ 上报
 * @endif
 */
typedef struct {
    uint8_t samp_cnt;
    uint8_t report_idx;
    uint16_t es_sn;
    uint32_t timestamp_sn; // 对齐master的时间
    uint8_t rssi[SLE_CS_IQ_REPORT_COUNT];
    uint8_t freq[SLE_CS_IQ_REPORT_COUNT];
    uint16_t i_data[SLE_CS_IQ_REPORT_COUNT];
    uint16_t q_data[SLE_CS_IQ_REPORT_COUNT];
    uint32_t tof_result;
} sle_channel_sounding_iq_report_t;

/**
 * @if Eng
 * @brief Enum of sle channel sounding capability set.
 * @else
 * @brief channel sounding 能力集
 * @endif
 */
typedef struct {
    uint8_t caps[SLE_CS_CAPS_LEN];
} sle_channel_sounding_caps_t;

typedef void (*sle_read_local_channel_sounding_callback)(
    sle_channel_sounding_caps_t *caps, errcode_t status);

typedef void (*sle_read_remote_channel_sounding_callback)(uint16_t conn_id,
    sle_channel_sounding_caps_t *caps, errcode_t status);

typedef void (*sle_channel_sounding_state_changed_callback)(errcode_t status);

typedef void (*sle_channel_sounding_iq_report_callback)(uint16_t conn_id,
    sle_channel_sounding_iq_report_t *report);
typedef struct {
    sle_read_local_channel_sounding_callback read_local_cs_caps_cb;
    sle_read_remote_channel_sounding_callback read_remote_cs_caps_cb;
    sle_channel_sounding_state_changed_callback cs_state_changed_cb;
    sle_channel_sounding_iq_report_callback cs_iq_report_cb;
} sle_hadm_callbacks_t;

errcode_t sle_hadm_register_callbacks(sle_hadm_callbacks_t *func);

errcode_t sle_read_local_channel_sounding_caps(void);
errcode_t sle_read_remote_channel_sounding_caps(uint16_t conn_id);
errcode_t sle_set_channel_sounding_param_ex(uint16_t conn_id, sle_set_channel_sounding_param_ex_t *param);
errcode_t sle_set_channel_sounding_enable(uint16_t conn_id);
errcode_t sle_set_channel_sounding_disable(uint16_t conn_id);


#ifdef __cplusplus
}
#endif
#endif /* SLE_HADM_MANAGER */
/**
 * @}
 */
