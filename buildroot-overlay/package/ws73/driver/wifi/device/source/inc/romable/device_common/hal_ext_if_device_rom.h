/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: Header file of HAL external public interfaces .
 * Create: 2022-2-19
 */

#ifndef __HAL_EXT_IF_DEVICE_ROM_H__
#define __HAL_EXT_IF_DEVICE_ROM_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HAL_RX_LPF_GAIN                (6)
#define HAL_DBM_CH                     (-13)
#define HAL_SINGLE_DOUBLE_SWITCH_GAIN  (3)
#define HAL_RSSI_REF_DIFFERENCE        (20)
#define HAL_RADAR_REF_DIFFERENCE       (24)

#define HAL_PA_ERROR_OFFSET 3

#define HAL_CCA_OPT_ED_HIGH_20TH_DEF       (-62) /* CCA 20M检测门限寄存器默认值 */
#define HAL_CCA_OPT_ED_HIGH_40TH_DEF       (-59) /* CCA 40M检测门限寄存器默认值 */

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
typedef enum {
    HAL_DEVICE_EVENT_JOIN_COMP                = 0,          /* staut join完成事件 */
    HAL_DEVICE_EVENT_VAP_UP                   = 1,          /* vap up事件 */
    HAL_DEVICE_EVENT_VAP_DOWN                 = 2,          /* vap down事件 */
    HAL_DEVICE_EVENT_SCAN_BEGIN               = 3,          /* 扫描开始事件 */
    HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF  = 4,          /* 扫描切离信道事件 */
    HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_BACK = 5,          /* 扫描切回信道事件 */
    HAL_DEVICE_EVENT_SCAN_END                 = 6,          /* 扫描结束事件 */
    HAL_DEVICE_EVENT_SCAN_ABORT               = 7,          /* 扫描abort事件 */
    HAL_DEVICE_EVENT_SCAN_PAUSE               = 8,          /* PAUSE SCAN */
    HAL_DEVICE_EVENT_SCAN_RESUME              = 9,          /* RESUME SCAN */

    HAL_DEVICE_EVENT_VAP_CHANGE_TO_ACTIVE,          /* vap 进入active状态事件 */
    HAL_DEVICE_EVENT_VAP_CHANGE_TO_AWAKE,           /* vap 进入awake状态事件 */
    HAL_DEVICE_EVENT_VAP_CHANGE_TO_DOZE,            /* vap 进入doze状态事件 */
    HAL_DEVICE_EVENT_INIT_RX_DSCR,                  /* add rx dscr */
    HAL_DEVICE_EVENT_DESTROY_RX_DSCR,               /* destroy rx dscr */
    HAL_DEVICE_EVENT_TBTT_WAKE_UP,                  /* tbtt唤醒事件，区别于其他唤醒事件 */
    HAL_DEVICE_EVENT_TRANS_TO_WORK_STATE,

    HAL_DEVICE_EVENT_SCAN_PAUSE_FROM_CHAN_CONFLICT,      /* PAUSE SCAN */
    HAL_DEVICE_EVENT_SCAN_RESUME_FROM_CHAN_CONFLICT,     /* RESUME SCAN */

    HAL_DEVICE_EVENT_DBDC_STOP,
    HAL_DEVICE_EVENT_DETATCH,                       /* 状态机detatch */
    HAL_DEVICE_EVENT_SYSTEM_INIT,                   /* 系统初始启动 */
    HAL_DEVICE_EVENT_WOW_PREPARE,                   /* WOW加载 */

    HAL_DEVICE_EVENT_BUTT
} hal_device_event;

typedef enum {
    HAL_DEVICE_INIT_STATE = 0,              /* init状态 */
    HAL_DEVICE_IDLE_STATE = 1,              /* idle状态 */
    HAL_DEVICE_WORK_STATE = 2,              /* work状态vap up */
    HAL_DEVICE_SCAN_STATE = 3,              /* 扫描状态 */
    HAL_DEVICE_BUTT_STATE                   /* 最大状态 */
} hal_device_state_info;

/* CBB频率 */
typedef enum {
    HAL_CLK_0M_FREQ                     = 0,
    HAL_CLK_40M_FREQ                    = 1,
    HAL_CLK_80M_FREQ                    = 2,
    HAL_CLK_160M_FREQ                   = 3,
    HAL_CLK_320M_FREQ                   = 4,
    HAL_CLK_640M_FREQ                   = 5,
    HAL_CLK_960M_FREQ                   = 6,
    HAL_CLK_30M_FREQ                    = 7,
    HAL_CLK_60M_FREQ                    = 8,
    HAL_CLK_120M_FREQ                   = 9,

    HAL_CLK_BUTT_FREQ
} hal_clk_freq_enum;
typedef osal_u8 hal_clk_freq_enum_uint8;

/* ADC/DAC组合频率 */
typedef enum {
    HAL_CLK_ADC80M_DAC160M                      = 0,
    HAL_CLK_ADC160M_DAC160M                     = 1,
    HAL_CLK_ADC320M_DAC480M                     = 2,

    HAL_CLK_ADC_DAC_BUTT
} hal_clk_adc_dac_enum;
typedef osal_u8 hal_clk_adc_dac_enum_uint8;
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/* HAL模块和DMAC模块共用的WLAN RX结构体 */
typedef struct {
    osal_u32 packet_idx;
    osal_u8 payoald[4];                /* 数组大小为4 */
} hal_cali_hal2hmac_payload_stru;

typedef enum {
    HAL_DEVICE_NOA_TYPE_P2P = 0,
    HAL_DEVICE_NOA_TYPE_TWT = 1,
    HAL_DEVICE_NOA_TYPE_WUR = 2,
    HAL_DEVICE_NOA_TYPE_CHBA = 3,
    HAL_DEVICE_NOA_TYPE_MAX,
} hal_device_noa_type_enum;
typedef osal_u8 hal_device_noa_type_enum_uint8;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_ext_if_device.h */
