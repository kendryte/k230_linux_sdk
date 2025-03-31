/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 * 文 件 名   : hmac_sta_channel_scoring.c
 * 生成日期   : 2024年8月15日
 * 功能描述   : sta信道评分模块
 */
#include "hmac_user.h"
#include "hmac_vap.h"
#include "hmac_sta_channel_scoring.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_STA_CHANNEL_SCORING_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define HMAC_CHANNEL_SCORING_BASE_WEIGHT         100
#define HMAC_CHANNEL_SCORING_ADJ_WEIGHT          85
#define HMAC_CHANNEL_SCORING_NEXT_ADJ_WEIGHT     55
#define HMAC_CHANNEL_SCORING_U32_MAX             0xFFFFFFFF

OAL_STATIC hmac_sta_channel_socoring_record_stru g_hmac_channel_scoring_record;

OAL_STATIC hmac_sta_channel_socoring_record_stru *hmac_get_channel_scoring_record(osal_void)
{
    return &g_hmac_channel_scoring_record;
}

OAL_STATIC osal_u8 hmac_judge_channel_scoring_record_effective(
    hmac_sta_channel_socoring_record_stru *channel_scoring_record)
{
    osal_u64 current_time_stamp = osal_get_time_stamp_ms();
    osal_u8 effective_cnt = 0;
    osal_u8 i = 0;
    osal_u64 record_time_stamp = 0;

    for (i = 0; i < HMAC_STA_CHANNEL_SCORING_RECORD_MAX; i++) {
        channel_scoring_record->record[i].is_effective = 0;
        record_time_stamp = osal_makeu64(channel_scoring_record->record[i].record_time_stamp_l32,
            channel_scoring_record->record[i].record_time_stamp_h32);
        if ((record_time_stamp != 0) &&
            ((current_time_stamp - record_time_stamp) <= HMAC_CHANNEL_SCORING_EFFECTIVE_TIME)) {
                channel_scoring_record->record[i].is_effective = 1;
                effective_cnt++;
        }
    }

    return effective_cnt;
}

OAL_STATIC osal_u32 hmac_cal_factor(hmac_single_channel_scoring_record_stru single_time_record)
{
    osal_u32 factor = 0;
    osal_u32 busy = 0;
    osal_u32 total = 0;

    /* channel_time 30单位， (channel_time_rx + channel_time_tx) <= channel_time_busy <= channel_time */
    if (single_time_record.filled & HMAC_SURVEY_HAS_CHAN_TIME_BUSY) {
        busy = single_time_record.channel_time_busy;
    } else if (single_time_record.filled & HMAC_SURVEY_HAS_CHAN_TIME_RX) {
        busy = single_time_record.channel_time_rx;
    } else {
        wifi_printf_always("CHANNEL SCORING: data missing.\r\n");
        return 0;
    }

    total = single_time_record.channel_time;
    if (single_time_record.filled & HMAC_SURVEY_HAS_CHAN_TIME_TX) {
        busy = (busy > single_time_record.channel_time_tx) ? (busy - single_time_record.channel_time_tx) : 0;
        total = (total > single_time_record.channel_time_tx) ? (total - single_time_record.channel_time_tx) : 0;
    }
    if (busy * HMAC_CHANNEL_SCORING_FACTOR_MULTIPLE > HMAC_CHANNEL_SCORING_U32_MAX) {
        wifi_printf_always("CHANNEL SCORING: unsigned int overflow.\r\n");
        return 0;
    }

    factor = (total != 0) ? (busy * HMAC_CHANNEL_SCORING_FACTOR_MULTIPLE / total) : 0;
    return factor;
}

OAL_STATIC osal_void hmac_cal_total_factor(osal_u32 *total_factor,
    hmac_sta_channel_socoring_record_stru *channel_scoring_record, osal_u8 channel_num)
{
    osal_u8 i = 0;
    osal_u8 j = 0;
    osal_u32 factor = 0;

    for (i = 0; i < HMAC_STA_CHANNEL_SCORING_RECORD_MAX; i++) {
        if (channel_scoring_record->record[i].is_effective == 1) {
            for (j = 0; j < channel_num; j++) {
                factor = hmac_cal_factor(channel_scoring_record->record[i].single_time_record[j]);
                total_factor[j] += factor;
            }
        }
    }
}

OAL_STATIC osal_s32 hmac_channel_scoring_find_adjacent_freq(osal_s8 channel_num_idx, osal_u8 max_channel_num)
{
    osal_u8 i = 0;
    const mac_freq_channel_map_stru *freq_map_2g = OAL_PTR_NULL;

    freq_map_2g = hmac_regdomain_get_freq_map_2g_etc();
    if (freq_map_2g == OAL_PTR_NULL) {
        return OAL_FAIL;
    }

    for (i = 0; i < max_channel_num; i++) {
        if (channel_num_idx == freq_map_2g[i].idx) {
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}

OAL_STATIC osal_u16 hmac_channel_scoring_find_center_freq(osal_s8 channel_num_idx, osal_u8 max_channel_num)
{
    osal_u8 i = 0;
    const mac_freq_channel_map_stru *freq_map_2g = OAL_PTR_NULL;

    freq_map_2g = hmac_regdomain_get_freq_map_2g_etc();
    if (freq_map_2g == OAL_PTR_NULL) {
        return 0;
    }

    for (i = 0; i < max_channel_num; i++) {
        if (channel_num_idx == freq_map_2g[i].idx) {
            return freq_map_2g[i].freq;
        }
    }
    return 0;
}

OAL_STATIC osal_s32 hmac_find_ideal_channel(hmac_sta_channel_socoring_record_stru *channel_scoring_record,
    osal_u8 effective_cnt)
{
    osal_s8 i = 0;
    osal_u32 total_weight = 0;
    osal_u32 ideal_factor = HMAC_CHANNEL_SCORING_U32_MAX;
    osal_u32 average_factor[MAC_CHANNEL_FREQ_2_BUTT] = {0};
    osal_u32 final_factor[MAC_CHANNEL_FREQ_2_BUTT] = {0};
    osal_s8 ideal_channel_num = 0;
    osal_u16 center_freq = 0;

    for (i = 0; i < channel_scoring_record->channel_num; i++) {
        average_factor[i] = channel_scoring_record->total_factor[i] / effective_cnt;
    }

    /* 计算相邻信道影响         */
    for (i = 0; i < channel_scoring_record->channel_num; i++) {
        total_weight = HMAC_CHANNEL_SCORING_BASE_WEIGHT;
        /* -1:减一个信道 */
        if (hmac_channel_scoring_find_adjacent_freq(i - 1, channel_scoring_record->channel_num) == OAL_SUCC) {
            final_factor[i] += HMAC_CHANNEL_SCORING_ADJ_WEIGHT * average_factor[i - 1]; /* -1:减一个信道 */
            total_weight += HMAC_CHANNEL_SCORING_ADJ_WEIGHT;
        }
        /* -2:减两个信道 */
        if (hmac_channel_scoring_find_adjacent_freq(i - 2, channel_scoring_record->channel_num) == OAL_SUCC) {
            final_factor[i] += HMAC_CHANNEL_SCORING_NEXT_ADJ_WEIGHT * average_factor[i - 2]; /* -2:减两个信道 */
            total_weight += HMAC_CHANNEL_SCORING_NEXT_ADJ_WEIGHT;
        }
        /* +1:加一个信道 */
        if (hmac_channel_scoring_find_adjacent_freq(i + 1, channel_scoring_record->channel_num) == OAL_SUCC) {
            final_factor[i] += HMAC_CHANNEL_SCORING_ADJ_WEIGHT * average_factor[i + 1]; /* +1:加一个信道 */
            total_weight += HMAC_CHANNEL_SCORING_ADJ_WEIGHT;
        }
        /* +2:加两个信道 */
        if (hmac_channel_scoring_find_adjacent_freq(i + 2, channel_scoring_record->channel_num) == OAL_SUCC) {
            final_factor[i] += HMAC_CHANNEL_SCORING_NEXT_ADJ_WEIGHT * average_factor[i + 2]; /* +2:加两个信道 */
            total_weight += HMAC_CHANNEL_SCORING_NEXT_ADJ_WEIGHT;
        }

        final_factor[i] /= total_weight;
        if ((i + 1 == 1) || (i + 1 == 6) || (i + 1 == 11)) { /* 1,6,11 是偏好信道 */
            final_factor[i] = (final_factor[i] * 4) / 5; /* 偏好信道乘以 4/5 让结果更小 */
            wifi_printf_always("CHANNEL SCORING: * channel %d: total interference = 0.%u (bias 0.800000)\r\n", i + 1,
                final_factor[i]);
        } else {
            wifi_printf_always("CHANNEL SCORING: * channel %d: total interference = 0.%u\r\n", i + 1, final_factor[i]);
        }

        if (ideal_factor >= final_factor[i]) {
            ideal_factor = final_factor[i];
            ideal_channel_num = i + 1;
        }
    }

    center_freq = hmac_channel_scoring_find_center_freq(ideal_channel_num - 1, channel_scoring_record->channel_num);
    wifi_printf_always("CHANNEL SCORING: * Ideal channel is %d (%u MHz) with total interference of 0.%u\r\n",
        ideal_channel_num, center_freq, ideal_factor);

    return OAL_SUCC;
}

osal_s32 hmac_cal_channel_score(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_sta_channel_socoring_record_stru *channel_scoring_record = OAL_PTR_NULL;
    osal_u8 effective_cnt = 0;

    channel_scoring_record = hmac_get_channel_scoring_record();
    if (channel_scoring_record == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_cal_channel_score::get record failed.");
        return OAL_FAIL;
    }

    effective_cnt = hmac_judge_channel_scoring_record_effective(channel_scoring_record);
    if ((effective_cnt == 0) || (effective_cnt > HMAC_STA_CHANNEL_SCORING_RECORD_MAX)) {
        wifi_printf_always("hmac_cal_channel_score::In %d seconds, no effective record\r\n",
            HMAC_CHANNEL_SCORING_EFFECTIVE_TIME / HMAC_CHANNEL_SCORING_MS_TO_SECOND);
        return OAL_FAIL;
    }

    if ((channel_scoring_record->total_factor == OAL_PTR_NULL) || (channel_scoring_record->channel_num == 0)) {
        wifi_printf_always("hmac_cal_channel_score::total_factor is NULL or channel_num is 0\r\n");
        return OAL_FAIL;
    }

    hmac_cal_total_factor(channel_scoring_record->total_factor, channel_scoring_record,
        channel_scoring_record->channel_num);
    hmac_find_ideal_channel(channel_scoring_record, effective_cnt);
    (osal_void)memset_s(channel_scoring_record->total_factor,
        OAL_SIZEOF(osal_u32) * channel_scoring_record->channel_num, 0,
        OAL_SIZEOF(osal_u32) * channel_scoring_record->channel_num);

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_free_channel_scoring_record(hmac_sta_channel_socoring_record_stru *channel_scoring_record)
{
    osal_u8 i = 0;

    for (i = 0; i < HMAC_STA_CHANNEL_SCORING_RECORD_MAX; i++) {
        if (channel_scoring_record->record[i].single_time_record != OAL_PTR_NULL) {
            oal_mem_free(channel_scoring_record->record[i].single_time_record, OSAL_TRUE);
            channel_scoring_record->record[i].single_time_record = OAL_PTR_NULL;
        }
    }

    if (channel_scoring_record->total_factor != OAL_PTR_NULL) {
        oal_mem_free(channel_scoring_record->total_factor, OSAL_TRUE);
        channel_scoring_record->total_factor = OAL_PTR_NULL;
    }
}

osal_void hmac_deinit_channel_scoring_record(osal_void)
{
    hmac_sta_channel_socoring_record_stru *channel_scoring_record = OAL_PTR_NULL;

    channel_scoring_record = hmac_get_channel_scoring_record();
    if (channel_scoring_record == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_deinit_channel_scoring_record::get record failed.");
        return;
    }

    hmac_free_channel_scoring_record(channel_scoring_record);
    (osal_void)memset_s(channel_scoring_record, OAL_SIZEOF(hmac_sta_channel_socoring_record_stru), 0,
        OAL_SIZEOF(hmac_sta_channel_socoring_record_stru));
}

OAL_STATIC osal_s32 hmac_channel_scoring_alloc(hmac_sta_channel_socoring_record_stru *channel_scoring_record)
{
    osal_u8 i = 0;

    for (i = 0; i < HMAC_STA_CHANNEL_SCORING_RECORD_MAX; i++) {
        channel_scoring_record->record[i].single_time_record = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            OAL_SIZEOF(hmac_single_channel_scoring_record_stru) * channel_scoring_record->channel_num, OAL_TRUE);
        if (channel_scoring_record->record[i].single_time_record == OAL_PTR_NULL) {
            oam_warning_log0(0, OAM_SF_DFR, "{hmac_channel_scoring_alloc::malloc single_time_record failed.}");
            goto exit;
        }
        (osal_void)memset_s(channel_scoring_record->record[i].single_time_record,
            OAL_SIZEOF(hmac_single_channel_scoring_record_stru) * channel_scoring_record->channel_num, 0,
            OAL_SIZEOF(hmac_single_channel_scoring_record_stru) * channel_scoring_record->channel_num);
    }

    channel_scoring_record->total_factor =
        oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(osal_u32) * channel_scoring_record->channel_num, OAL_TRUE);
    if (channel_scoring_record->total_factor == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_channel_scoring_alloc::malloc total_factor failed.}");
        goto exit;
    }

    (osal_void)memset_s(channel_scoring_record->total_factor,
        OAL_SIZEOF(osal_u32) * channel_scoring_record->channel_num, 0,
        OAL_SIZEOF(osal_u32) * channel_scoring_record->channel_num);
    return OAL_SUCC;

exit:
    hmac_deinit_channel_scoring_record();
    return OAL_FAIL;
}

osal_s32 hmac_init_channel_scoring_record(osal_void)
{
    osal_u8 i = 0;
    osal_u8 channel_num = 0;
    osal_u32 ret = OAL_FAIL;
    hmac_sta_channel_socoring_record_stru *channel_scoring_record = OAL_PTR_NULL;

    channel_scoring_record = hmac_get_channel_scoring_record();
    if (channel_scoring_record == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_init_channel_scoring_record::get record failed.}");
        return OAL_FAIL;
    }

    /* g_hmac_channel_scoring_record结构体清零 */
    (osal_void)memset_s(channel_scoring_record, OAL_SIZEOF(hmac_sta_channel_socoring_record_stru), 0,
        OAL_SIZEOF(hmac_sta_channel_socoring_record_stru));

    /* 获取信道数量 */
    for (i = 1; i <= MAC_CHANNEL_FREQ_2_BUTT; i++) {
        ret = hmac_is_channel_num_valid_etc(WLAN_BAND_2G, i);
        if (ret == OAL_SUCC) {
            channel_num++;
        }
    }

    if (channel_num == 0) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_init_channel_scoring_record::channel num is zero.}");
        return OAL_FAIL;
    }
    channel_scoring_record->channel_num = channel_num;

    /* 根据信道数申请所需空间 */
    return hmac_channel_scoring_alloc(channel_scoring_record);
}

osal_void hmac_write_channel_scoring_record(hmac_device_stru *hmac_device, osal_u8 current_scan_channel_num)
{
    wlan_scan_chan_stats_stru *record = OAL_PTR_NULL;
    osal_u8 i = 0;
    osal_u8 current_pos = 0;
    osal_u64 current_time = 0;
    hmac_sta_channel_socoring_record_stru *channel_scoring_record = OAL_PTR_NULL;

    channel_scoring_record = hmac_get_channel_scoring_record();
    if (channel_scoring_record == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_write_channel_scoring_record::channel num is zero.}");
        return;
    }

    if (current_scan_channel_num != channel_scoring_record->channel_num) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_write_channel_scoring_record::not a full channel scan.}");
        return;
    }

    record = hmac_device->scan_mgmt.scan_record_mgmt.chan_results;
    if (channel_scoring_record->channel_num >
        OAL_SIZEOF(hmac_device->scan_mgmt.scan_record_mgmt.chan_results) / OAL_SIZEOF(wlan_scan_chan_stats_stru)) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_write_channel_scoring_record::channel_num is invalid.}");
        return;
    }

    current_pos = (channel_scoring_record->record_pos) % HMAC_STA_CHANNEL_SCORING_RECORD_MAX;

    if (channel_scoring_record->record[current_pos].single_time_record == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_DFR, "{hmac_write_channel_scoring_record::current pos [%d] element is null.}",
            current_pos);
        return;
    }

    current_time = osal_get_time_stamp_ms();
    channel_scoring_record->record[current_pos].record_time_stamp_l32 = current_time & EXT_TIME_US_MAX_LEN;
    channel_scoring_record->record[current_pos].record_time_stamp_h32 = (osal_u32)(current_time >> 32); /* 右移32位 */

    for (i = 0; i < channel_scoring_record->channel_num; i++) {
        channel_scoring_record->record[current_pos].single_time_record[i].channel_time =
            record[i].total_stats_time_us / HMAC_CHANNEL_SCORING_US_TO_MS;
        channel_scoring_record->record[current_pos].single_time_record[i].channel_time_busy =
            (record[i].total_stats_time_us - record[i].total_free_time_20m_us) / HMAC_CHANNEL_SCORING_US_TO_MS;
        channel_scoring_record->record[current_pos].single_time_record[i].channel_time_rx =
            record[i].total_recv_time_us / HMAC_CHANNEL_SCORING_US_TO_MS;
        channel_scoring_record->record[current_pos].single_time_record[i].channel_time_tx =
            record[i].total_send_time_us / HMAC_CHANNEL_SCORING_US_TO_MS;
        channel_scoring_record->record[current_pos].single_time_record[i].filled = HMAC_SURVEY_HAS_CHAN_TIME |
            HMAC_SURVEY_HAS_CHAN_TIME_BUSY | HMAC_SURVEY_HAS_CHAN_TIME_RX | HMAC_SURVEY_HAS_CHAN_TIME_TX;
    }

    channel_scoring_record->record_pos = current_pos + 1;
}

#endif /* _PRE_WLAN_FEATURE_STA_CHANNEL_SCORING */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
