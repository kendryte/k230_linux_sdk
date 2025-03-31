/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file of hmac_chan_meas.c.
 * Author: CompanyName
 * Create: 2021-12-13
 */
#ifndef __HMAC_CHAN_MEAS_H__
#define __HMAC_CHAN_MEAS_H__
#include "hmac_device.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHAN_MEAS_H

/* 扫描信息上报内核 */
typedef struct {
    hmac_info_report_type_enum acs_info_type;    /* 内核用来区分消息类型 */
    osal_u8 scan_chan_sbs;                            /* 当前扫描信道在扫描list中的下标 */
    osal_u8 scan_chan_idx;                            /* 当前扫描信道的信道号 */
    osal_u32 total_stats_time_us;
    osal_u32 total_free_time_20m_us;
    osal_u32 total_free_time_40m_us;
    osal_u32 total_free_time_80m_us;
    osal_u32 total_send_time_us;
    osal_u32 total_recv_time_us;
    osal_u8 free_power_cnt;                           /* 信道空闲功率测量次数 */
    osal_s16 free_power_stats_20m;
    osal_s16 free_power_stats_40m;
    osal_s16 free_power_stats_80m;
} hmac_chba_chan_stat_report_stru;

void hmac_chan_meas_scan_complete_handler(const hmac_scan_record_stru *pst_scan_record,
    osal_u8 uc_scan_idx);
osal_u32 hmac_chan_meas_scan_chan_once(hmac_vap_stru *pst_mac_vap, mac_cfg_link_meas_stru *meas_cmd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
