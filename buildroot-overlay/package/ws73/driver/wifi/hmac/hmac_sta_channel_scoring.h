/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: Header file for hmac_sta_channel_scoring.c
 * Create: 2024-08-15
 */
#ifndef __HMAC_STA_CHANNEL_SCORING_H__
#define __HMAC_STA_CHANNEL_SCORING_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_STA_CHANNEL_SCORING_RECORD_MAX (5)     /* 整个模块最多记录5次全信道扫描数据,新的数据循环覆盖久的数据 */
#define HMAC_CHANNEL_SCORING_EFFECTIVE_TIME (60000) /* 60s之内的全信道扫描数据有效 */
#define HMAC_CHANNEL_SCORING_MS_TO_SECOND   (1000)
#define HMAC_CHANNEL_SCORING_US_TO_MS       (1000)

#define HMAC_SURVEY_HAS_CHAN_TIME      (1U << 1)
#define HMAC_SURVEY_HAS_CHAN_TIME_BUSY (1U << 2)
#define HMAC_SURVEY_HAS_CHAN_TIME_RX   (1U << 3)
#define HMAC_SURVEY_HAS_CHAN_TIME_TX   (1U << 4)

#define HMAC_CHANNEL_SCORING_FACTOR_MULTIPLE (1000000)

/*****************************************************************************
  3 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u32 channel_time;
    osal_u32 channel_time_busy;
    osal_u32 channel_time_rx;
    osal_u32 channel_time_tx;
    osal_u32 filled;
} hmac_single_channel_scoring_record_stru;

typedef struct {
    osal_u32 record_time_stamp_h32;
    osal_u32 record_time_stamp_l32;
    osal_u8 is_effective;
    osal_u8  resv[3];
    hmac_single_channel_scoring_record_stru *single_time_record;
} hmac_sta_channel_scoring_single_record_stru;

typedef struct {
    osal_u8 channel_num;
    osal_u8 record_pos;
    osal_u8 resv[2];
    osal_u32 *total_factor;
    hmac_sta_channel_scoring_single_record_stru record[HMAC_STA_CHANNEL_SCORING_RECORD_MAX];
} hmac_sta_channel_socoring_record_stru;

/*****************************************************************************
  4 函数声明
*****************************************************************************/
osal_void hmac_dump_acs_record(osal_void);
osal_s32 hmac_init_channel_scoring_record(osal_void);
osal_void hmac_deinit_channel_scoring_record(osal_void);
osal_u16 hmac_channel_scoring_get_center_freq(osal_u8 channel_num);
osal_s32 hmac_cal_channel_score(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_write_channel_scoring_record(hmac_device_stru *hmac_device, osal_u8 current_scan_channel_num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_sta_channel_scoring.h */
