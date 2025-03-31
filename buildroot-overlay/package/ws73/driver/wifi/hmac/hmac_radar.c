/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac radar process
 * Create: 2020-7-4
 */

#if defined (_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined (_PRE_WLAN_FEATURE_DFS_ENABLE)

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_radar.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_common_ops.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_device.h"
#include "dmac_resource_rom.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined (_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined (_PRE_WLAN_FEATURE_DFS_ENABLE)

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_RADAR_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define RADAR_FILTER_FCC_CRAZY_REPORT_DET
#define RADAR_FILTER_NORMAL_PULSE_TIMER

osal_u8 g_rptmem_np_fcc[]   = { 5, 8, 8, 8, 7, 5 };
osal_u8 g_rptmem_np_etsi[]  = { 5, 5, 5, 7, 5 };

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_radar_get_pulse_margin
 功能描述  : 检测雷达脉冲间隔的周期性
*****************************************************************************/
osal_u32 hmac_radar_get_pulse_margin(hal_dfs_radar_type_enum_uint8 radar_type, osal_u32 min_pri)
{
    osal_u32              pulse_margin;

    if ((radar_type == HAL_DFS_RADAR_TYPE_FCC)) {
        if (check_radar_fcc_type0_pri(min_pri) == OSAL_TRUE
            || check_radar_fcc_type4_pri(min_pri) == OSAL_TRUE
            || check_radar_fcc_type3_pri(min_pri) == OSAL_TRUE) {
            pulse_margin = RADAR_PULSE_MARGIN_ETSI;
        } else {
            pulse_margin = RADAR_PULSE_MARGIN_FCC;
        }
    } else {
        pulse_margin = RADAR_PULSE_MARGIN_ETSI;
    }

    return pulse_margin;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_pulse_period
 功能描述  : 检测雷达脉冲间隔的周期性
*****************************************************************************/
osal_u32 hmac_radar_check_pulse_period(hal_dfs_radar_type_enum_uint8 radar_type, osal_u32 *pri, osal_u8 cnt,
    osal_u32 min_pri)
{
    osal_u8               loop;
    osal_u32              tmp;
    osal_u32              times;
    osal_u32              pulse_margin;
    osal_u8               diff_num = 0;
    oal_bool_enum_uint8     big_period_limit = OSAL_TRUE;
    osal_u32              tmp_max   = min_pri;
    osal_u32              tmp_min   = min_pri;

    pulse_margin = hmac_radar_get_pulse_margin(radar_type, min_pri);

    if ((radar_type == HAL_DFS_RADAR_TYPE_FCC && check_radar_fcc_type0_pri(min_pri) == OSAL_TRUE) ||
        /* 333表示与min_pri进行差值计算的比较，差值的界限为2 */
        (radar_type == HAL_DFS_RADAR_TYPE_KOREA && oal_absolute_sub(min_pri, 333) <= 2)) {
        big_period_limit = OSAL_FALSE;
    }

    /* 判断间隔的周期性 */
    for (loop = 0; loop < cnt; loop++) {
        times = (pri[loop] + (min_pri >> 1)) / min_pri;
        tmp   = pri[loop] / times;

        if (tmp > tmp_max) {
            tmp_max = tmp;
        }
        if (tmp < tmp_min) {
            tmp_min = tmp;
        }

        if (pulse_margin < oal_absolute_sub(min_pri, tmp)) {
            diff_num++;
        } else {
            if (times > MAX_PULSE_TIMES && big_period_limit == OSAL_TRUE) {
                return RADAR_PULSE_BIG_PERIOD;
            }
        }
    }

    if (pulse_margin < oal_absolute_sub(tmp_max, tmp_min)) {
        return RADAR_PULSE_NO_PERIOD;
    }

    /* 如果4个PRI中，有三个为min_PRI的倍数，一个不成倍数，则识别为不等间隔的误报 */
    if (diff_num == 1) {
        return RADAR_PULSE_ONE_DIFF_PRI;
    } else if (diff_num > 1) {
        return RADAR_PULSE_NO_PERIOD;
    }

    return RADAR_PULSE_NORMAL_PERIOD;
}

osal_void hmac_radar_analysis_set_info(hmac_radar_pulse_analysis_info *res, osal_u32 *pre_timestamp, osal_bool *first,
    hal_pulse_info_stru *info, hmac_radar_pulse_analysis_result_stru *result)
{
    if (info->duration <= STAGGER_MAX_DURATION) {
        res->stagger_cnt++;
    }

    if (*first == OSAL_TRUE) {
        res->min_duration = info->duration;
        res->max_power = info->power;
        res->min_power = info->power;
        res->sum_power += info->power;
        res->max_duration = info->duration;
        res->sum_duration += info->duration;
        *first = OSAL_FALSE;
    } else {
        res->min_duration = (info->duration >= res->min_duration) ? res->min_duration : info->duration;
        res->max_duration = (info->duration <= res->max_duration) ? res->max_duration : info->duration;

        res->max_power = (info->power < res->max_power) ? res->max_power : info->power;
        res->min_power = (info->power > res->min_power) ? res->min_power : info->power;
        res->sum_power += info->power;
        res->sum_duration += info->duration;

        result->pri[res->index] = (info->timestamp >= *pre_timestamp) ? (info->timestamp - *pre_timestamp) :
                                  (info->timestamp + 0xFFFFF - *pre_timestamp);
        res->min_pri = (result->pri[res->index] > res->min_pri) ? res->min_pri : result->pri[res->index];
        res->max_pri = (result->pri[res->index] <= res->max_pri) ? res->max_pri : result->pri[res->index];
        res->index++;
    }

    *pre_timestamp = info->timestamp;
    return;
}

/*****************************************************************************
 函 数 名  : hmac_radar_analysis_pulse_info
 功能描述  : 分析雷达脉冲信息
*****************************************************************************/
osal_u32 hmac_radar_analysis_pulse_info(hal_radar_pulse_info_stru *pulse_info, osal_u32 analysis_cnt, osal_u8 type,
    hmac_radar_pulse_analysis_result_stru *result)
{
    osal_u32 loop = (pulse_info->pulse_cnt >= analysis_cnt) ? pulse_info->pulse_cnt - analysis_cnt : 0;
    osal_bool first = OSAL_TRUE;
    osal_u32 pre_timestamp = 0;
    hal_pulse_info_stru *info = OSAL_NULL;
    hmac_radar_pulse_analysis_info res;
    (osal_void)memset_s(&res, sizeof(hmac_radar_pulse_analysis_info), 0, sizeof(hmac_radar_pulse_analysis_info));
    res.min_pri = 0xFFFFF;
    res.max_pri = 0xFFFFF;

    result->begin = (osal_u8)loop;

    /* 计算脉冲间距，统计stagger脉冲个数 */
    for (; loop < pulse_info->pulse_cnt; loop++) {
        info = &(pulse_info->pulse_info[loop]);

        if (type == info->type) {
            result->begin = (first == OSAL_TRUE) ? (osal_u8)loop : result->begin;
            hmac_radar_analysis_set_info(&res, &pre_timestamp, &first, info, result);
        }
    }

    result->stagger_cnt = res.stagger_cnt;
    result->min_duration = res.min_duration;
    result->avrg_power = (osal_u16)(res.sum_power / (res.index + 1));
    result->pri_cnt = res.index;
    result->min_pri = res.min_pri;
    result->max_power = res.max_power;
    result->duration_diff = res.max_duration - res.min_duration;
    result->pri_diff = res.max_pri - res.min_pri;
    result->power_diff = res.max_power - res.min_power;
    result->avrg_duration = (osal_u16)(res.sum_duration / (res.index + 1));
    /* 秒冲个数等于5 */
    if (result->begin > 1 && analysis_cnt == 5) {
        pre_timestamp = pulse_info->pulse_info[result->begin - 1].timestamp;
        result->extra_pri = (pulse_info->pulse_info[result->begin].timestamp >= pre_timestamp) ?
                            (pulse_info->pulse_info[result->begin].timestamp - pre_timestamp) :
                            (pulse_info->pulse_info[result->begin].timestamp + 0xFFFFF - pre_timestamp);
    }

    return OAL_SUCC;
}

osal_void hmac_radar_get_sys_time(oal_bool_enum_uint8 log_switch)
{
    osal_u32 curr_timestamp;
    curr_timestamp = (osal_u32)osal_get_time_stamp_ms();
    if (log_switch) {
        oam_warning_log1(0, OAM_SF_ANY, "Curren time : %u ms.", curr_timestamp);
    }
}

/*****************************************************************************
 函 数 名  : hmac_radar_pulse_log
 功能描述  : 打印脉冲信息
*****************************************************************************/
osal_void hmac_radar_pulse_log(hal_radar_pulse_info_stru *pulse_info, hmac_radar_pulse_analysis_result_stru *result,
    oal_bool_enum_uint8 log_switch, osal_u8 type)
{
    osal_u8                      loop;
    osal_u8                      begin = result->begin;
    osal_u32                     pri;
    osal_u8                      idx   = 0;

    oam_warning_log4(0, OAM_SF_DFS, "{hmac_radar_pulse_log:min_duration: %d, min_pri: %d, mean_power: %d, begin: %d!",
                     result->min_duration, result->min_pri, result->avrg_power, result->begin);

    for (loop = begin; loop < pulse_info->pulse_cnt; loop++) {
        if (type != pulse_info->pulse_info[loop].type) {
            continue;
        }

        oam_warning_log4(0, OAM_SF_DFS, "{hmac_radar_pulse_log::duration: %d, power: %d, pri: %u, pulse_type: %d",
                         pulse_info->pulse_info[loop].duration, pulse_info->pulse_info[loop].power, result->pri[idx++],
                         pulse_info->pulse_info[loop].type);
    }

    if (log_switch == OSAL_FALSE) {
        return;
    }

    idx = 0;

    if (log_switch == 1 || type == RADAR_CHIRP_PULSE_TYPE) {
        for (loop = begin; loop < pulse_info->pulse_cnt; loop++) {
            if (type != pulse_info->pulse_info[loop].type) {
                continue;
            }
            oam_warning_log4(0, 0, "{hmac_radar_pulse_log::duration: %d, power: %d, intvl: %u, pulse_type: %d}",
                pulse_info->pulse_info[loop].duration, pulse_info->pulse_info[loop].power, result->pri[idx++],
                pulse_info->pulse_info[loop].type);
        }
        /* 2表示转换数量 */
    } else if (log_switch == 2 && type != RADAR_CHIRP_PULSE_TYPE) {
        /* 与3进行比较 */
        begin = (begin >= 3) ? (begin - 3) : 0;

        for (loop = begin; loop < pulse_info->pulse_cnt; loop++) {
            if (loop >= result->begin) {
                pri = result->pri[loop - result->begin];
            } else {
                pri = (pulse_info->pulse_info[loop + 1].timestamp >= pulse_info->pulse_info[loop].timestamp) ?
                      (pulse_info->pulse_info[loop + 1].timestamp - pulse_info->pulse_info[loop].timestamp) :
                      (pulse_info->pulse_info[loop + 1].timestamp + 0xFFFFF - pulse_info->pulse_info[loop].timestamp);
            }

            oam_warning_log4(0, OAM_SF_DFS, "{duration: %d, power: %d, intvl: %u, pulse_type: %d}\r\n",
                             pulse_info->pulse_info[loop].duration, pulse_info->pulse_info[loop].power, pri,
                             pulse_info->pulse_info[loop].type);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_chirp_pri
 功能描述  : 检查chirp脉冲之间的间隔
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_chirp_pri(hal_dfs_radar_type_enum_uint8 radar_type, osal_u32 pri)
{
    osal_u32              min_pri;

    if (radar_type == HAL_DFS_RADAR_TYPE_ETSI) {
        min_pri = MIN_ETSI_CHIRP_PRI;
    } else if (radar_type == HAL_DFS_RADAR_TYPE_FCC) {
        min_pri = MIN_FCC_CHIRP_PRI;
    } else if (radar_type == HAL_DFS_RADAR_TYPE_MKK) {
        min_pri = MIN_MKK_CHIRP_PRI;
    } else {
        min_pri = 0;
    }

    if (pri <= min_pri || min_pri == 0) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_chirp_pulse_num
 功能描述  : 检查chirp脉冲的个数是否符合要求
*****************************************************************************/
osal_u32 hmac_radar_check_chrip_pulse_num(hmac_radar_pulse_analysis_result_stru *result,
    hal_dfs_radar_filter_stru *dfs_radar, oal_bool_enum_uint8 log_switch)
{
    osal_u8                      loop;
    osal_u32                     sum_pri;
    osal_u8                      idx;
    osal_u8                      pulse_num;

    /* 一个中断内如果600ms内脉冲个数超过5个识别为误报 */
    for (loop = 0; loop < result->pri_cnt; loop++) {
        sum_pri = 0;
        pulse_num = 1;
        for (idx = loop; idx < result->pri_cnt; idx++) {
            pulse_num++;
            sum_pri += result->pri[idx];
            /* 表示时间进制为1000 */
            if (sum_pri > MIN_FCC_TOW_TIMES_INT_PRI * 1000) {
                break;
            }
        }

        if ((pulse_num >= MAX_FCC_CHIRP_PULSE_CNT_IN_600US) &&
            /* 时间进制为1000 */
            (sum_pri * MAX_FCC_CHIRP_PULSE_CNT_IN_600US <= MIN_FCC_TOW_TIMES_INT_PRI * 1000 * pulse_num)) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_chrip_duration
 功能描述  : 检查同一个burst中脉冲宽度是否一致
*****************************************************************************/
osal_u32 hmac_radar_check_chrip_duration(hmac_radar_pulse_analysis_result_stru *result,
    hal_radar_pulse_info_stru *pulse_info, oal_bool_enum_uint8 log_switch)
{
    osal_u8           idx;

    /* chirp PRI 1000~2000, 要求duration差异<=4 */
    for (idx = 0; idx + 1 < result->pri_cnt; idx++) {
        if (check_radar_fcc_type5_pri(result->pri[idx]) == OSAL_TRUE &&
            (oal_absolute_sub(pulse_info->pulse_info[idx].duration,
                pulse_info->pulse_info[idx + 1].duration) > RADAR_FCC_CHIRP_PULSE_DURATION_MARGIN)) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_same_duration_num
 功能描述  : 检查同一个burst中连续相等脉冲宽度个数
*****************************************************************************/
osal_u32 hmac_radar_check_eq_duration_num(hmac_radar_pulse_analysis_result_stru *result,
    hal_radar_pulse_info_stru *pulse_info, oal_bool_enum_uint8 log_switch)
{
    osal_u8           idx;
    osal_u8           sub_idx;
    osal_u8           eq_num = 0;
    osal_u8           pulse_num = result->pri_cnt + 1;

    /* 判断连续相等的duration个数 */
    for (idx = 0; idx < pulse_num - 1; idx++) {
        for (sub_idx = idx + 1; sub_idx < pulse_num; sub_idx++) {
            if (oal_absolute_sub(pulse_info->pulse_info[idx].duration,
                pulse_info->pulse_info[sub_idx].duration) <= RADAR_FCC_CHIRP_PULSE_DURATION_MARGIN) {
                eq_num++;
            } else {
                break;
            }
        }
        idx += eq_num;

        if (eq_num >= MAX_FCC_CHIRP_EQ_DURATION_NUM) {
            return OSAL_FALSE;
        } else {
            eq_num = 0;
        }
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_min_chirp_pulse_num
 功能描述  : 检查min chirp脉冲的个数是否符合要求
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_min_chirp_pulse_num(hal_dfs_radar_type_enum_uint8 radar_type, osal_u8 cnt)
{
    osal_u32              pri_num;

    if (radar_type == HAL_DFS_RADAR_TYPE_ETSI) {
        pri_num = MIN_ETSI_CHIRP_PRI_NUM;
    } else if (radar_type == HAL_DFS_RADAR_TYPE_FCC) {
        pri_num = MIN_FCC_CHIRP_PRI_NUM;
    } else if (radar_type == HAL_DFS_RADAR_TYPE_MKK) {
        pri_num = MIN_MKK_CHIRP_PRI_NUM;
    } else {
        pri_num = 0;
    }

    if (cnt < pri_num) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_normal_pulse_num
 功能描述  : 检查非chirp脉冲的个数是否符合要求
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_normal_pulse_num(osal_u8 cnt, hal_dfs_radar_type_enum_uint8 radar_type)
{
    if (cnt < MAX_RADAR_NORMAL_PULSE_ANA_CNT - 1) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}
#ifdef RADAR_FILTER_NORMAL_PULSE_TIMER
oal_bool_enum_uint8 hmac_radar_check_normal_pulse_timer_irq_num(
    hal_dfs_radar_type_enum_uint8 radar_type, osal_u8 radar_type_num,
    osal_u8 irq_cnt, osal_u8 irq_cnt_old)
{
    if (check_radar_etsi_type3_hw(radar_type, radar_type_num)
        && check_radar_etsi_type3_irq_num(irq_cnt, irq_cnt_old) == OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (check_radar_etsi_type2_hw(radar_type, radar_type_num)
        && check_radar_etsi_type2_irq_num(irq_cnt, irq_cnt_old) == OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (check_radar_fcc_type4_hw(radar_type, radar_type_num)
        && check_radar_fcc_type4_irq_num(irq_cnt, irq_cnt_old) == OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (check_radar_fcc_type3_hw(radar_type, radar_type_num)
        && check_radar_fcc_type3_irq_num(irq_cnt, irq_cnt_old) == OSAL_TRUE) {
        return OSAL_FALSE;
    }

    return  OSAL_TRUE;
}
#endif
/*****************************************************************************
 函 数 名  : hmac_radar_check_pulse_power
 功能描述  : 检查脉冲的power
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_pulse_power(osal_u16 power, hal_dfs_radar_type_enum_uint8 radar_type,
    osal_u32 min_pri, osal_u16 min_duration)
{
    if (radar_type == HAL_DFS_RADAR_TYPE_FCC && (check_radar_fcc_type0_pri(min_pri) == OSAL_TRUE ||
        check_radar_fcc_type0_pri(min_pri>>1) == OSAL_TRUE) && power >= MIN_RADAR_PULSE_POWER_FCC_TYPE0) {
        return OSAL_TRUE;
    }

    if (radar_type == HAL_DFS_RADAR_TYPE_ETSI && check_radar_etsi_short_pulse(min_duration) == OSAL_TRUE &&
        power >= MIN_RADAR_PULSE_POWER_ETSI_STAGGER) {
        return OSAL_TRUE;
    }

    if (power < MIN_RADAR_PULSE_POWER) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_chirp_pulse_num
 功能描述  : 检查chirp脉冲的个数是否符合要求
*****************************************************************************/
osal_u32 hmac_radar_check_fcc_chirp(hmac_radar_pulse_analysis_result_stru *result,
    hal_radar_pulse_info_stru *pulse_info,
    hal_dfs_radar_filter_stru *dfs_radar,
    osal_u32 delta_time_for_pulse)
{
    /* 过滤600ms内脉冲个数超过5个的中断 */
    if (hmac_radar_check_chrip_pulse_num(result, dfs_radar, dfs_radar->log_switch) == OSAL_FALSE) {
        dfs_radar->chirp_cnt = 0;
        oam_warning_log0(0, OAM_SF_DFS,
            "{hmac_radar_pulse_log::[DFS]chirp_cnt set 0 due to more than 5 irq in 600ms.!");
        return OSAL_FALSE;
    }

    /* 相邻脉冲duration一致 */
    if (hmac_radar_check_chrip_duration(result, pulse_info, dfs_radar->log_switch) == OSAL_FALSE
        && dfs_radar->dfs_pulse_check_filter.fcc_chirp_duration_diff == OSAL_TRUE) {
        dfs_radar->chirp_cnt = 0;
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_radar_pulse_log::[DFS]chirp_cnt set 0 due to duration diff.!");
        return OSAL_FALSE;
    }

    /* 相等duration个数 */
    if (hmac_radar_check_eq_duration_num(result, pulse_info, dfs_radar->log_switch) == OSAL_FALSE
        && dfs_radar->dfs_pulse_check_filter.fcc_chirp_eq_duration_num == OSAL_TRUE) {
        dfs_radar->chirp_cnt = 0;
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_radar_pulse_log::[DFS]chirp_cnt set 0 due to same duration num.!");
        return OSAL_FALSE;
    }

    /* 检查功率一致性 */
    /* 03为提高短脉冲雷达硬件检测率，power计算有一定偏差，pow diff阈值修改为25 */
    if (check_radar_fcc_chirp_pow_diff(result->power_diff) == OSAL_FALSE
        && dfs_radar->dfs_pulse_check_filter.fcc_chirp_pow_diff == OSAL_TRUE) {
        dfs_radar->chirp_cnt = 0;
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_radar_pulse_log::[DFS]chirp_cnt set 0 due to power diff.!");
        return OSAL_FALSE;
    }

    /* 对第一个中断不处理时间间隔 */
    if (dfs_radar->chirp_cnt_total <= 1) {
        return OSAL_TRUE;
    }

    /* 过滤时间间隔<600ms的中断 */
    /* 过滤时间间隔超过6s的中断，并清零中断计数 */
    if (delta_time_for_pulse < MIN_FCC_TOW_TIMES_INT_PRI
        || delta_time_for_pulse > MAX_FCC_TOW_TIMES_INT_PRI) {
        dfs_radar->chirp_cnt = 0;
        oam_warning_log1(0, OAM_SF_DFS,
            "{hmac_radar_pulse_log::[DFS]chirp_cnt set 0 due to delta_time_for_pulse=%u.!",
            delta_time_for_pulse);
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_chirp_filter
 功能描述  : chirp雷达脉冲过滤检测
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_chirp_filter(hal_dfs_radar_filter_stru *dfs_radar,
    hal_radar_pulse_info_stru *pulse_info)
{
    osal_u32 timestamp = (osal_u32)osal_get_time_stamp_ms();
    osal_u32 delta_time = (osal_u32)osal_get_runtime(dfs_radar->last_burst_timestamp_for_chirp, timestamp);
    osal_u32 delta_time_for_pulse = (osal_u32)osal_get_runtime(dfs_radar->last_timestamp_for_chirp_pulse, timestamp);
    hmac_radar_pulse_analysis_result_stru result;

    (osal_void)memset_s(&result, sizeof(hmac_radar_pulse_analysis_result_stru),
        0, sizeof(hmac_radar_pulse_analysis_result_stru));
    osal_u32 ret = hmac_radar_analysis_pulse_info(pulse_info, pulse_info->pulse_cnt, RADAR_CHIRP_PULSE_TYPE, &result);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_radar_chirp_filter::hmac_radar_analysis_pulse_info failed[%d]}.", ret);
    }

    hmac_radar_pulse_log(pulse_info, &result, dfs_radar->log_switch, RADAR_CHIRP_PULSE_TYPE);

    /* 统计12s内总中断个数 */
    ((dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_FCC || dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_MKK) &&
        (dfs_radar->last_burst_timestamp_for_chirp != 0)) ? dfs_radar->chirp_cnt_total++ : 0;

#ifdef RADAR_FILTER_FCC_CRAZY_REPORT_DET
    (dfs_radar->crazy_report_cnt == OSAL_TRUE) ? dfs_radar->chirp_cnt_for_crazy_report_det++ : 0;
#endif

    /* 1. 检查最少脉冲个数；2. 检查chirp的最小间隔; 3. 检测脉冲power； */
    if ((hmac_radar_check_min_chirp_pulse_num(dfs_radar->radar_type, result.pri_cnt) == OSAL_FALSE) ||
        (hmac_radar_check_pulse_power(result.max_power, dfs_radar->radar_type, result.min_pri,
            result.min_duration) == OSAL_FALSE) ||
        (hmac_radar_check_chirp_pri(dfs_radar->radar_type, result.min_pri) == OSAL_FALSE)) {
        dfs_radar->chirp_cnt = 0;
        return OSAL_TRUE;
    }

    if (dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_ETSI) {
        if ((dfs_radar->last_burst_timestamp_for_chirp != 0 && delta_time < dfs_radar->chirp_time_threshold) ||
            /* 检查chirp周期性 */
            (hmac_radar_check_pulse_period(dfs_radar->radar_type, result.pri, result.pri_cnt, result.min_pri) !=
            RADAR_PULSE_NORMAL_PERIOD)) {
            return OSAL_TRUE;
        }

        dfs_radar->last_burst_timestamp_for_chirp = timestamp;
        return OSAL_FALSE;
    } else if (dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_FCC || dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_MKK) {
        dfs_radar->last_timestamp_for_chirp_pulse = timestamp;

        /* 12s雷达的第一个中断 */
        if (delta_time >= dfs_radar->chirp_time_threshold || dfs_radar->chirp_cnt_total == 0) {
            dfs_radar->last_burst_timestamp_for_chirp = timestamp;
            dfs_radar->chirp_cnt = 1;
            dfs_radar->chirp_cnt_total = 1;

            /* 校验脉宽、功率差和脉冲间隔 */
            hmac_radar_check_fcc_chirp(&result, pulse_info, dfs_radar, delta_time_for_pulse);
            oam_warning_log1(0, OAM_SF_DFS, "{hmac_radar_chirp_filter::chirp_cnt is %d!", dfs_radar->chirp_cnt);
            return OSAL_TRUE;
        }

        /* 校验脉宽、功率差和脉冲间隔 */
        if (hmac_radar_check_fcc_chirp(&result, pulse_info, dfs_radar, delta_time_for_pulse) == OSAL_FALSE) {
            return OSAL_TRUE;
        }

        dfs_radar->chirp_cnt++;
        oam_warning_log1(0, OAM_SF_DFS, "{hmac_radar_chirp_filter::chirp_cnt=%d.!", dfs_radar->chirp_cnt);
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_normal_pulse_pri
 功能描述  : 非chirp雷达脉冲间隔检查
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_normal_pulse_pri(hal_dfs_radar_type_enum_uint8 radar_type, osal_u32 pri)
{
    if (pri < MIN_RADAR_PULSE_PRI) {
        return OSAL_FALSE;
    }

    if (radar_type == HAL_DFS_RADAR_TYPE_ETSI) {
        /* ETSI下非chirp雷达最小脉冲范围检查 */
        if (check_radar_etsi_type1_pri(pri) == OSAL_FALSE && check_radar_etsi_type2_pri(pri) == OSAL_FALSE &&
            check_radar_etsi_type3_pri(pri) == OSAL_FALSE && check_radar_etsi_type5_pri(pri) == OSAL_FALSE &&
            check_radar_etsi_type6_pri(pri) == OSAL_FALSE) {
            return OSAL_FALSE;
        }
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_pulse_mode
 功能描述  : 确定stagger模式类型
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_pulse_mode(osal_u8 stagger_cnt)
{
    if (stagger_cnt >= MAX_RADAR_STAGGER_NUM) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_min_duration
 功能描述  : 检查stagger最小脉冲宽度
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_min_duration(osal_u16 min_duration)
{
    if (min_duration >= MIN_RADAR_STAGGER_DURATION) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_avrg_duration
 功能描述  : 检查stagger最小脉冲宽度
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_avrg_duration(osal_u16 min_duration)
{
    if (min_duration <= MEAN_RADAR_STAGGER_DURATION) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_type6_duration_diff
 功能描述  : 检查stagger脉冲差
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_type6_duration_diff(osal_u16 duration_diff, osal_u32 min_pri)
{
    if (check_radar_etsi_type6_pri(min_pri) == OSAL_TRUE
        && check_radar_etsi_type6_duration_diff(duration_diff) == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_type6_extra_pri
 功能描述  : 检查stagger脉冲差
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_type6_extra_pri(osal_u32 extra_pri, osal_u32 min_pri)
{
    /* 100表示脉冲差相差小于等于100 */
    if (check_radar_etsi_type6_pri(min_pri) == OSAL_TRUE && (extra_pri != 0) && (extra_pri <= min_pri - 100)) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_normal_min_duration
 功能描述  : 检查normal最小脉冲宽度
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_normal_min_duration(osal_u16 min_duration, osal_u32 min_pri,
    hal_dfs_radar_type_enum_uint8 radar_type)
{
    /* FCC type2, duration[1~5us] should <=10us */
    if ((radar_type == HAL_DFS_RADAR_TYPE_FCC || radar_type == HAL_DFS_RADAR_TYPE_MKK) &&
        check_radar_fcc_type2_pri_small(min_pri) == OSAL_TRUE && min_duration > MAX_RADAR_NORMAL_DURATION_FCC_TYPE2) {
        return OSAL_FALSE;
    }

    /* FCC type3/4, duration [4us~22us] */
    /* 芯片上报duration错误,duration min改为0.5us规避软件过滤掉雷达 */
    if ((radar_type == HAL_DFS_RADAR_TYPE_FCC || radar_type == HAL_DFS_RADAR_TYPE_MKK) &&
        check_radar_fcc_type4_pri_small(min_pri) == OSAL_TRUE &&
        check_radar_fcc_type4_duration(min_duration) == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    if ((min_duration >= MIN_RADAR_NORMAL_DURATION && radar_type != HAL_DFS_RADAR_TYPE_MKK)
        || (min_duration >= MIN_RADAR_NORMAL_DURATION_MKK && radar_type == HAL_DFS_RADAR_TYPE_MKK)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_normal_duration_diff
 功能描述  : 检查normal最小脉冲宽度
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_normal_duration_diff(osal_u16 duration_diff, osal_u32 min_pri,
    hal_dfs_radar_type_enum_uint8 radar_type)
{
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_get_stagger_pri_type
 功能描述  : 通过pri, 获取stagger模式类型
*****************************************************************************/
hmac_radar_stagger_type_enum_uint8 hmac_radar_get_stagger_pri_type(osal_u32 t1, osal_u32 t2)
{
    if (check_radar_etsi_type5_pri(t1) == OSAL_TRUE && check_radar_etsi_type5_pri(t2) == OSAL_TRUE) {
        return HMAC_RADAR_STAGGER_TYPE5;
    }

    if (check_radar_etsi_type6_pri(t1) == OSAL_TRUE && check_radar_etsi_type6_pri(t2) == OSAL_TRUE) {
        return HMAC_RADAR_STAGGER_TYPE6;
    }

    return HMAC_RADAR_STAGGER_TYPE_INVALID;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_pps_by_type
 功能描述  : 按照脉冲类型检查stagger模式脉冲间隔之间PPS
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_pps_by_type(osal_u32 t1, osal_u32 t2, hmac_radar_stagger_type_enum_uint8 type)
{
    osal_u32      min_pps_diff;
    osal_u32      max_pps_diff;
    osal_u32      tmp;
    osal_u32      product;

    if (type == HMAC_RADAR_STAGGER_TYPE5) {
        min_pps_diff = RADAR_ETSI_TYPE5_MIN_PPS_DIFF;
        max_pps_diff = RADAR_ETSI_TYPE5_MAX_PPS_DIFF;
    } else if (type == HMAC_RADAR_STAGGER_TYPE6) {
        min_pps_diff = RADAR_ETSI_TYPE6_MIN_PPS_DIFF;
        max_pps_diff = RADAR_ETSI_TYPE6_MAX_PSS_DIFF;
    } else {
        return OSAL_FALSE;
    }

    tmp     = oal_absolute_sub(t1, t2) * RADAR_ONE_SEC_IN_US;
    product = t1 * t2;

    if (min_pps_diff * product < tmp && tmp < max_pps_diff * product) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_pps
 功能描述  : 检查stagger模式脉冲间隔之间PPS
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_pps(osal_u32 t1, osal_u32 t2)
{
    hmac_radar_stagger_type_enum_uint8          type;

    /* T1和T2相等是同一个PRI的脉冲 */
    if (oal_absolute_sub(t1, t2) <= RADAR_PULSE_MARGIN_ETSI) { // RADAR_PULSE_MARGIN)
        return OSAL_TRUE;
    }

    type = hmac_radar_get_stagger_pri_type(t1, t2);
    if (type == HMAC_RADAR_STAGGER_TYPE_INVALID) {
        return OSAL_FALSE;
    }

    return hmac_radar_check_pps_by_type(t1, t2, type);
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_sum
 功能描述  : 确定stagger模式脉冲间隔是否存在和的关系
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_sum(osal_u32 dt1, osal_u32 dt2, osal_u32 dt3, osal_u32 t4)
{
    /* DT3和DT1不都在Type5或Type6 PRI范围内，直接过滤 */
    if ((check_radar_etsi_type5_pri(dt3) != OSAL_TRUE || check_radar_etsi_type5_pri(dt1) != OSAL_TRUE) &&
        (check_radar_etsi_type6_pri(dt3) != OSAL_TRUE || check_radar_etsi_type6_pri(dt1) != OSAL_TRUE)) {
        return OSAL_FALSE;
    }

    /* 如果T1与T2，或者T2, T3之间的pps不满足要求，则认为是误报 */
    if (hmac_radar_check_pps(dt1, dt2) == OSAL_FALSE ||
        hmac_radar_check_pps(dt2, dt3) == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    if (oal_absolute_sub(t4, dt1 + dt2) <= TWO_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, dt1 + dt3) <= TWO_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, dt2 + dt3) <= TWO_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, dt1 + dt2 + dt3) <= THREE_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, two_times(dt1) + dt2 + dt3) <= THREE_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, dt1 + two_times(dt2) + dt3) <= THREE_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, dt1 + dt2 + two_times(dt3)) <= THREE_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, two_times(dt1) + two_times(dt2) + dt3) <= THREE_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, dt1 + two_times(dt2) + two_times(dt3)) <= THREE_TIMES_STAGGER_PULSE_MARGIN ||
        oal_absolute_sub(t4, two_times(dt1) + dt2 + two_times(dt3)) <= THREE_TIMES_STAGGER_PULSE_MARGIN) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_T2_T3
 功能描述  : 检测stagger模式脉冲间隔的周期性
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_t2_t3(osal_u32 t1, osal_u32 t2, osal_u32 t3, osal_u32 t4)
{
    osal_u32              dt3;
    /* 3表示3倍的t1 */
    if ((t2 < two_times(t1)) && (t3 <= 3 * t1) && (t3 > two_times(t1))) {
        /* T2和T1不都在Type5或Type6 PRI范围内，则是雷达误报 */
        if ((check_radar_etsi_type5_pri(t2) != OSAL_TRUE || check_radar_etsi_type5_pri(t1) != OSAL_TRUE) &&
            (check_radar_etsi_type6_pri(t2) != OSAL_TRUE || check_radar_etsi_type6_pri(t1) != OSAL_TRUE)) {
            return OSAL_FALSE;
        }

        dt3 = t3 - t1;

        /* dT3=dT2, 且dt1, dt2满足脉冲间间隔的PPS要求，则是雷达信号 */
        if ((oal_absolute_sub(dt3, t2) <= STAGGER_PULSE_MARGIN) &&
            (hmac_radar_check_pps(t1, t2) == OSAL_TRUE)) {
            return OSAL_TRUE;
        }

        if (hmac_radar_check_stagger_sum(t1, t2, dt3, t4) == OSAL_TRUE) {
            return OSAL_TRUE;
        }

        dt3 = t3 - t2;

        /* dT3=dT1, 且dt1, dt2满足脉冲间间隔的PPS要求，则是雷达信号 */
        if ((oal_absolute_sub(dt3, t1) <= STAGGER_PULSE_MARGIN) &&
            (hmac_radar_check_pps(t1, t2) == OSAL_TRUE)) {
            return OSAL_TRUE;
        }

        if (hmac_radar_check_stagger_sum(t1, t2, dt3, t4) == OSAL_TRUE) {
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_T3_T4
 功能描述  : 检测stagger模式脉冲间隔的周期性
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_t3_t4(osal_u32 t1, osal_u32 t2, osal_u32 t3, osal_u32 t4)
{
    if (t3 < two_times(t1) && t4 > two_times(t1)) {
        /* T3和T1不都在Type5或Type6 PRI范围内，则认为是误报，直接过滤 */
        if ((check_radar_etsi_type5_pri(t3) != OSAL_TRUE || check_radar_etsi_type5_pri(t1) != OSAL_TRUE) &&
            (check_radar_etsi_type6_pri(t3) != OSAL_TRUE || check_radar_etsi_type6_pri(t1) != OSAL_TRUE)) {
            return OSAL_FALSE;
        }

        /* 如果T1与T2，或者T2, T3之间的pps不满足要求，则认为是误报 */
        if (hmac_radar_check_pps(t1, t2) == OSAL_FALSE || hmac_radar_check_pps(t2, t3) == OSAL_FALSE) {
            return OSAL_FALSE;
        }

        if (oal_absolute_sub(t4, t1 + t2) <= TWO_TIMES_STAGGER_PULSE_MARGIN ||
            oal_absolute_sub(t4, t1 + t3) <= TWO_TIMES_STAGGER_PULSE_MARGIN ||
            oal_absolute_sub(t4, t2 + t3) <= TWO_TIMES_STAGGER_PULSE_MARGIN ||
            oal_absolute_sub(t4, t1 + t2 + t3) <= THREE_TIMES_STAGGER_PULSE_MARGIN ||
            oal_absolute_sub(t4, two_times(t1) + t2 + t3) <= FOUR_TIMES_STAGGER_PULSE_MARGIN ||
            oal_absolute_sub(t4, t1 + two_times(t2) + t3) <= FOUR_TIMES_STAGGER_PULSE_MARGIN ||
            oal_absolute_sub(t4, t1 + t2 + two_times(t3)) <= FOUR_TIMES_STAGGER_PULSE_MARGIN) {
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_type6
 功能描述  : 检测stagger模式type6的周期性
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_type6(osal_u32 *pri, osal_u32 *pri_sort, osal_u8 pri_len)
{
    osal_u32  t1, t2, t3, t4;
    osal_u32  time1, time2, time3, time4;
    osal_u8   loop;
    unref_param(pri_len);

    t1 = pri[0];
    t2 = pri[1];
    t3 = pri[2]; /* 获得pri第2位的值 */
    t4 = pri[3]; /* 获得pri第3位的值 */

    time1 = pri_sort[0];
    time2 = pri_sort[1];
    time3 = pri_sort[2]; /* 获得pri_sort第2位的值 */
    time4 = pri_sort[3]; /* 获得pri_sort第3位的值 */

    /* 1. T1~T4均不相同则为误报 */
    /* 4表示pri_sort的长度 */
    for (loop = 1; loop < 4; loop++) {
        if (oal_absolute_sub(pri_sort[loop], pri_sort[loop - 1]) <= STAGGER_PULSE_MARGIN) {
            break;
        }
    }
    /* 4表示pri_sort的长度 */
    if (loop == 4 && oal_absolute_sub(time4, time1 + time2 + time3) <= THREE_TIMES_STAGGER_PULSE_MARGIN) {
        return OSAL_TRUE;
    }

    /* 2.如果T2=T3=T4, 且T2>2T1，是雷达，否则是误报 */
    if (oal_absolute_sub(time2, time3) <= STAGGER_PULSE_MARGIN &&
        oal_absolute_sub(time2, time4) <= STAGGER_PULSE_MARGIN && time2 > two_times(time1)) {
        return OSAL_TRUE;
    }

    /* 3.如果T1=T2, T3=T4, 且t1=t3, t2=t4, 且T3<3T1，则是雷达信号，否则是误报 */
    if (oal_absolute_sub(time1, time2) <= STAGGER_PULSE_MARGIN &&
        oal_absolute_sub(time3, time4) <= STAGGER_PULSE_MARGIN && oal_absolute_sub(t1, t3) <= STAGGER_PULSE_MARGIN &&
        oal_absolute_sub(t2, t4) <= STAGGER_PULSE_MARGIN && time3 < (time1 * 0x3)) {
        return OSAL_TRUE;
    }

    /* 4. 如果t1=t4且(a)T4<2T1 或者 (b)T4是T1, T2, T3中两个不相同的Ts之和，则是雷达，否则是误报 */
    if (oal_absolute_sub(t1, t4) <= STAGGER_PULSE_MARGIN) {
        if (time4 < two_times(time1)) {
            return OSAL_TRUE;
        }
        /* 如果1== loop, 则T1=T2, 比较T4与T1+T3;如果2==loop, 则T1!=T2, T2=T3,比较T4与T1+T3 */
        if (oal_absolute_sub(time4, time1 + time3) <= TWO_TIMES_STAGGER_PULSE_MARGIN && loop <= 2) {
            return OSAL_TRUE;
        }
    }

    /* 5. 如果T3=T4, 且1)T3=T1+T2 or 2)T3<2T1，则是雷达，否则是误报 */
    if (oal_absolute_sub(time3, time4) <= STAGGER_PULSE_MARGIN &&
        (oal_absolute_sub(time3, time1 + time2) <= TWO_TIMES_STAGGER_PULSE_MARGIN || time3 < (time1 * 0x3))) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}
/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_
 功能描述  : 检测stagger模式脉冲是否存在相同的PRI
*****************************************************************************/
hmac_radar_stagger_period_enum_uint8 hmac_radar_check_stagger_pri_equal(osal_u32 *pri, osal_u8 pri_len,
    osal_u8 cnt, osal_u8 *pri_idx, osal_u8 pri_idx_len)
{
    osal_u8           loop;
    oal_bool_enum_uint8 eq_bool  = OSAL_FALSE;
    osal_u32          t1       = pri[0];
    osal_u8           eq_idx   = 0;
    osal_u8           basic_pri_idx = 0;
    unref_param(pri_len);
    unref_param(pri_idx_len);
    for (loop = 1; loop < cnt; loop++) {
        if (oal_absolute_sub(pri[loop], pri[loop - 1]) <= STAGGER_PULSE_MARGIN) {
            eq_bool = OSAL_TRUE;
            /* 记录相同PRI的idx */
            eq_idx = loop - 1;
            break;
        }
    }

    /* 不存在相同的PRI，直接返回 */
    if (eq_bool == OSAL_FALSE) {
        return HMAC_RADAR_STAGGER_PERIOD_NOT_DEC;
    }

    /***************************************************************
       判断从T2,T3,T4;
       如果存在小于2T1, 且
       1)不在stagger的PRI范围内；
       或者2)与T1的pps差异不在范围内；
       或者3)相同的PRI属于基本单元且是连续2个
       则不能判断为雷达信号
    ***************************************************************/
    for (loop = 1; loop < cnt; loop++) {
        if (pri[loop] < two_times(t1)) {
            if (check_radar_etsi_stagger_pri(pri[loop]) == OSAL_FALSE ||
                hmac_radar_check_pps(t1, pri[loop]) == OSAL_FALSE) {
                return HMAC_RADAR_STAGGER_PERIOD_PRI_ERR;
            } else {
                basic_pri_idx = loop;
            }
        }
    }

    /* 如果原始序列连续两个基本单元相同，则识别为误报 */
    if (oal_absolute_sub(pri_idx[eq_idx], pri_idx[eq_idx + 1]) == 1 && eq_idx + 1 <= basic_pri_idx) {
        return HMAC_RADAR_STAGGER_PERIOD_PRI_ERR;
    }

    return HMAC_RADAR_STAGGER_PERIOD_PRI_EQUAL;
}

/*****************************************************************************
 函 数 名  : hmac_radar_pri_sort
 功能描述  : 排序脉冲PRI
*****************************************************************************/
osal_void hmac_radar_pri_sort(osal_u32 *pri, osal_u8 pri_len, osal_u32 cnt, osal_u8 *pri_idx, osal_u8 pri_idx_len)
{
    osal_u8           loop;
    osal_u8           index;
    osal_u8           min_index;
    osal_u32          pri_tmp;
    osal_u8           pri_idx_tmp;
    unref_param(pri_len);
    unref_param(pri_idx_len);
    for (loop = 1; loop < cnt; loop++) {
        min_index = loop - 1;
        for (index = loop; index < cnt; index++) {
            if (pri[index] < pri[min_index]) {
                min_index = index;
            }
        }

        pri_tmp            = pri[loop - 1];
        pri[loop - 1]    = pri[min_index];
        pri[min_index] = pri_tmp;

        pri_idx_tmp            = pri_idx[loop - 1];
        pri_idx[loop - 1]    = pri_idx[min_index];
        pri_idx[min_index] = pri_idx_tmp;
    }
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_period
 功能描述  : 检测stagger模式脉冲的周期性
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_pri_absub(osal_u32 *pri, osal_u8 pri_len, osal_u8 cnt)
{
    osal_u8           loop;
    osal_u32          absub_val;
    unref_param(pri_len);
    for (loop = 0; loop < cnt - 1; loop++) {
        absub_val = oal_absolute_sub(pri[loop + 1], pri[loop]);
        if ((absub_val > RADAR_PULSE_MARGIN_ETSI) && (absub_val <= 0x32)) {
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_check_stagger_period
 功能描述  : 检测stagger模式脉冲的周期性
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_check_stagger_period(osal_u32 *pri, osal_u8 cnt)
{
    osal_u32                              priority[MAX_RADAR_NORMAL_PULSE_ANA_CNT] = {0};
    osal_u8                               pri_idx[MAX_RADAR_NORMAL_PULSE_ANA_CNT - 1] = {0, 1, 2, 3};
    osal_u32                              t1, t2, t3, t4;
    hmac_radar_stagger_period_enum_uint8    stagger_period_ret;

    if (memcpy_s((osal_u32 *)priority, sizeof(priority), (osal_u32 *)pri, cnt * sizeof(osal_u32)) != EOK) {
        return OSAL_FALSE;
    }

    /* PRI排序 */
    hmac_radar_pri_sort(priority, MAX_RADAR_NORMAL_PULSE_ANA_CNT, cnt, pri_idx, MAX_RADAR_NORMAL_PULSE_ANA_CNT - 1);

    /* 最大值比最小值之间倍数大于MAX_STAGGER_PULSE_TIMES，则认为是误报 */
     /* 获得priority第3位的值 */
    if (priority[3] > MAX_STAGGER_PULSE_TIMES * priority[0]) {
        return OSAL_FALSE;
    }

    /* stagger模式下，必须满足T2-T1, T3-T2,T4-T3符合1)<=4;2)>50两个条件之一 */
    if (hmac_radar_check_pri_absub(priority, MAX_RADAR_NORMAL_PULSE_ANA_CNT, cnt) == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    /* 针对type6，排序之后2500>T1>=833 */
    if (check_radar_etsi_type6_pri(priority[0]) == OSAL_TRUE) {
        if (hmac_radar_check_stagger_type6(pri, priority, MAX_RADAR_NORMAL_PULSE_ANA_CNT) == OSAL_FALSE) {
            return OSAL_FALSE;
        }
    }

    /* 存在相同的pri，且PRI符合范围要求，则具有stagger模式周期性 */
    stagger_period_ret = hmac_radar_check_stagger_pri_equal(priority, MAX_RADAR_NORMAL_PULSE_ANA_CNT, cnt,
                                                            pri_idx, MAX_RADAR_NORMAL_PULSE_ANA_CNT - 1);
    if (stagger_period_ret == HMAC_RADAR_STAGGER_PERIOD_PRI_EQUAL) {
        return OSAL_TRUE;
    }

    /* 存在相同的PRI，但PRI不符合范围要求，则认为是误报 */
    if (stagger_period_ret == HMAC_RADAR_STAGGER_PERIOD_PRI_ERR) {
        return OSAL_FALSE;
    }

    t1 = priority[0];
    t2 = priority[1];
    t3 = priority[2]; /* 获得priority第2位的值 */
    t4 = priority[3]; /* 获得priority第3位的值 */

    /* T4 <2*T1或者T2>2*T1, 认为是误报 */
    if (t4 < two_times(t1) || t2 > two_times(t1)) {
        return OSAL_FALSE;
    }

    if (hmac_radar_check_stagger_t3_t4(t1, t2, t3, t4) == OSAL_TRUE) {
        return OSAL_TRUE;
    }

    if (hmac_radar_check_stagger_t2_t3(t1, t2, t3, t4) == OSAL_TRUE) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : stagger模式雷达脉冲过滤处理
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_stagger_filter(hal_dfs_radar_type_enum_uint8 radar_type,
    hmac_radar_pulse_analysis_result_stru  *result)
{
    /* 非ETSI雷达，直接过滤 */
    if (radar_type != HAL_DFS_RADAR_TYPE_ETSI) {
        return OSAL_TRUE;
    }

    /* 最小PRI不在stagger模式脉冲间隔范围内，直接过滤 */
    if (check_radar_etsi_stagger_pri(result->min_pri) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检查是否是stagger模式 */
    if (hmac_radar_check_stagger_pulse_mode(result->stagger_cnt) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检查大于最小脉宽 */
    if (hmac_radar_check_stagger_min_duration(result->min_duration) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检查平均脉宽 */
    if (hmac_radar_check_stagger_avrg_duration(result->avrg_duration) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检查type6 stagger脉宽差 */
    if (hmac_radar_check_stagger_type6_duration_diff(result->duration_diff, result->min_pri) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检查type6 stagger最近一次pri */
    if (hmac_radar_check_stagger_type6_extra_pri(result->extra_pri, result->min_pri) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* stagger模式周期性检测 */
    if (hmac_radar_check_stagger_period(result->pri, result->pri_cnt) == OSAL_TRUE) {
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_get_max_radar_pulse_ant_cnt
 功能描述  : 软件分析的雷达脉冲个数
*****************************************************************************/
osal_u8 hmac_radar_get_max_radar_pulse_ant_cnt(hal_dfs_radar_type_enum_uint8 radar_type,
    osal_u8 radar_det_num, hal_radar_pulse_info_stru *pulse_info)
{
    osal_u8                           max_pulse_cnt;
    osal_u32                          loop;
    hal_pulse_info_stru                *info1            = OSAL_NULL;
    hal_pulse_info_stru                *info2            = OSAL_NULL;
    osal_u32                          pri;
    osal_u32                          first_pulse_idx   = 0;
    osal_u32                          min_pri          = 0;
    const osal_u8                           begin = 0;

    /* 根据类型选择脉冲个数 */
    if (radar_type == HAL_DFS_RADAR_TYPE_FCC && radar_det_num == 0) {
        return MAX_RADAR_NORMAL_PULSE_ANA_CNT;
    }

    /* 判断buf是否存在两个burst */
    for (loop = pulse_info->pulse_cnt - 1; loop >= begin + 1; loop--) {
        info2 = &(pulse_info->pulse_info[loop]);
        info1 = &(pulse_info->pulse_info[loop - 1]);

        pri = (info2->timestamp >= info1->timestamp) ? (info2->timestamp - info1->timestamp) :
            (info2->timestamp + 0xFFFFF - info1->timestamp);

        if (loop == pulse_info->pulse_cnt - 1) {
            min_pri = pri;
        }

        if (pri < min_pri) {
            min_pri = pri;
        }

        if (pri > min_pri * MAX_PULSE_TIMES) {
            first_pulse_idx = loop;
            break;
        }
    }

    max_pulse_cnt = (osal_u8)(pulse_info->pulse_cnt - first_pulse_idx);

    /* check pulse num>=5 */
    max_pulse_cnt = (max_pulse_cnt < MAX_RADAR_NORMAL_PULSE_ANA_CNT) ? MAX_RADAR_NORMAL_PULSE_ANA_CNT : max_pulse_cnt;

    return max_pulse_cnt;
}

/*****************************************************************************
功能描述  : 非chirp雷达脉冲过滤处理
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_normal_pulse_filter(hal_dfs_radar_filter_stru *dfs_radar,
    hal_radar_pulse_info_stru *pulse_info, osal_u8 radar_det_num)
{
    hmac_radar_pulse_analysis_result_stru      result;
    osal_u32                                 ret;
    osal_u8                                  max_pulse_num;

    (osal_void)memset_s(&result, sizeof(hmac_radar_pulse_analysis_result_stru),
        0, sizeof(hmac_radar_pulse_analysis_result_stru));

    /* 获取硬件检测的脉冲个数 */
    max_pulse_num = hmac_radar_get_max_radar_pulse_ant_cnt(dfs_radar->radar_type, radar_det_num, pulse_info);

    if (dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_FCC) {
        hmac_radar_analysis_pulse_info(pulse_info, max_pulse_num, RADAR_NORMAL_PULSE_TYPE, &result);
    } else {
        hmac_radar_analysis_pulse_info(pulse_info, MAX_RADAR_NORMAL_PULSE_ANA_CNT, RADAR_NORMAL_PULSE_TYPE, &result);
    }

    /* 针对ETSI type2/3, 非stagger进一步分析8个脉冲的过滤 */
    if (dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_ETSI && (check_radar_etsi_type3_pri(result.min_pri) == OSAL_TRUE ||
        ((check_radar_etsi_type2_pri(result.min_pri) == OSAL_TRUE &&
        result.pri_diff <= RADAR_PULSE_MARGIN_ETSI * 4)))) { /* 4表示获得间隔最小误差 */
        (osal_void)memset_s(&result, sizeof(hmac_radar_pulse_analysis_result_stru),
            0, sizeof(hmac_radar_pulse_analysis_result_stru));
        hmac_radar_analysis_pulse_info(pulse_info, max_pulse_num, RADAR_NORMAL_PULSE_TYPE, &result);
    }

    dfs_radar->min_pri = result.min_pri;

    hmac_radar_pulse_log(pulse_info, &result, dfs_radar->log_switch, RADAR_NORMAL_PULSE_TYPE);

    /* 检测脉冲个数 */
    if (hmac_radar_check_normal_pulse_num(result.pri_cnt, dfs_radar->radar_type) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检测脉冲power */
    if (hmac_radar_check_pulse_power(result.avrg_power, dfs_radar->radar_type, result.min_pri, result.min_duration) ==
        OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检查大于最小脉宽 */
    if (hmac_radar_check_normal_min_duration(result.min_duration, result.min_pri,
        dfs_radar->radar_type) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /* 检查duration diff */
    if (hmac_radar_check_normal_duration_diff(result.duration_diff, result.min_pri,
        dfs_radar->radar_type) == OSAL_FALSE &&
        dfs_radar->dfs_pulse_check_filter.fcc_type4_duration_diff == OSAL_TRUE) {
        return OSAL_TRUE;
    }

    /* 检查PRI */
    if (hmac_radar_check_normal_pulse_pri(dfs_radar->radar_type, result.min_pri) == OSAL_FALSE) {
        return OSAL_TRUE;
    }

    /********************************************************************
       周期性检查:
       具备周期性，且间隔之间的倍数关系不大于MAX_PULSE_TIMES，则不过滤
       具备周期性，且间隔之间的倍数关系大于MAX_PULSE_TIMES，则直接过滤
       不具备周期性，则进行stagger模式处理。
    *********************************************************************/
    ret = hmac_radar_check_pulse_period(dfs_radar->radar_type, result.pri, result.pri_cnt, result.min_pri);
    if (ret == RADAR_PULSE_NORMAL_PERIOD) {
        return OSAL_FALSE;
    } else if (ret >= RADAR_PULSE_BIG_PERIOD) {
        return OSAL_TRUE;
    } else {
        return hmac_radar_stagger_filter(dfs_radar->radar_type, &result);
    }
}

/*****************************************************************************
 函 数 名  : hmac_radar_filter_timeout
 功能描述  : FCC chirp雷达超时处理函数
*****************************************************************************/
osal_u32 hmac_radar_filter_timeout(osal_void *arg)
{
    hal_dfs_radar_filter_stru   *dfs_radar = OSAL_NULL;
    hmac_device_stru            *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru     *hal_device = OSAL_NULL;
    frw_msg msg = {0};

    hal_device  = hmac_device->hal_device[0];
    dfs_radar   = &(hal_device->dfs_radar_filter);

    dfs_radar->chirp_timeout = OSAL_TRUE;

    msg.data = (osal_u8 *)(hal_radar_det_event_stru *)arg;
    msg.data_len = sizeof(hal_radar_det_event_stru);
    hmac_dfs_radar_detect_event(OSAL_NULL, &msg);

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_filter_timer
 功能描述  : chirp12s定时器
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_filter_timer(hmac_device_stru *hmac_device,
    hal_radar_det_event_stru *radar_det_info,
    hal_dfs_radar_filter_stru *dfs_radar)
{
    if (dfs_radar->radar_type != HAL_DFS_RADAR_TYPE_FCC && dfs_radar->radar_type != HAL_DFS_RADAR_TYPE_MKK) {
        return OSAL_TRUE;
    }

    /* 第一个FCC chirp中断时使能定时器 */
    if (dfs_radar->chirp_cnt_total != 1) {
        return OSAL_TRUE;
    }

    /* 启动12s超时定时器 */
    dfs_radar->timer.timeout = dfs_radar->chirp_time_threshold;
    /* 需要保证radar_det_info指向的内存不会由于函数退出而被释放，否则造成野指针访问异常 */
    frw_create_timer_entry(&(dfs_radar->timer), hmac_radar_filter_timeout,
        dfs_radar->timer.timeout, radar_det_info, OSAL_FALSE);

    return OSAL_TRUE;
}

#ifdef RADAR_FILTER_NORMAL_PULSE_TIMER
osal_u32 hmac_radar_normal_pulse_det_timeout(osal_void *arg)
{
    hal_dfs_radar_filter_stru   *dfs_radar = OSAL_NULL;
    hmac_device_stru            *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_to_dmac_device_stru     *hal_device = OSAL_NULL;
    frw_msg msg = {0};

    hal_device  = hmac_device->hal_device[0];
    dfs_radar   = &(hal_device->dfs_radar_filter);

    dfs_radar->dfs_normal_pulse_det.timeout = OSAL_FALSE;

    if (dfs_radar->dfs_normal_pulse_det.irq_cnt == 0) {
        dfs_radar->dfs_normal_pulse_det.period_cnt++;
        return OSAL_TRUE;
    } else {
        dfs_radar->dfs_normal_pulse_det.period_cnt = 0;
    }

    /* 定时器到期检测到雷达，则触发雷达检测事件 */
    if (dfs_radar->dfs_normal_pulse_det.radar_cnt) {
        dfs_radar->dfs_normal_pulse_det.timeout = OSAL_TRUE;
        msg.data = (osal_u8 *)(hal_radar_det_event_stru *)arg;
        msg.data_len = sizeof(hal_radar_det_event_stru);
        hmac_dfs_radar_detect_event(OSAL_NULL, &msg);
    } else {
        dfs_radar->dfs_normal_pulse_det.radar_cnt = 0;
        dfs_radar->dfs_normal_pulse_det.irq_cnt_old = dfs_radar->dfs_normal_pulse_det.irq_cnt;
        dfs_radar->dfs_normal_pulse_det.irq_cnt = 0;
    }

    return OSAL_TRUE;
}

oal_bool_enum_uint8 hmac_radar_normal_pulse_det_timer(hmac_device_stru *hmac_device,
    hal_radar_det_event_stru *radar_det_info,
    hal_dfs_radar_filter_stru *dfs_radar)
{
    if (dfs_radar->log_switch) {
        /* 启动200m周期性检测定时器 */
        dfs_radar->dfs_normal_pulse_det.timer.timeout = 200;
    } else {
        /* 启动50m周期性检测定时器 */
        dfs_radar->dfs_normal_pulse_det.timer.timeout = 50;
    }

    /* 需要保证radar_det_info指向的内存不会由于函数退出而被释放，否则造成野指针访问异常 */
    frw_create_timer_entry(&(dfs_radar->dfs_normal_pulse_det.timer), hmac_radar_normal_pulse_det_timeout,
        dfs_radar->dfs_normal_pulse_det.timer.timeout, radar_det_info,
        OSAL_FALSE);

    return OSAL_TRUE;
}
#endif

#ifdef RADAR_FILTER_FCC_CRAZY_REPORT_DET
/*****************************************************************************
 函 数 名  : hmac_radar_disable_chirp_det_timeout
 功能描述  : crazy report超时
*****************************************************************************/
osal_u32 hmac_radar_disable_chirp_det_timeout(osal_void *arg)
{
    hal_to_dmac_device_stru   *hal_device;

    hal_device = (hal_to_dmac_device_stru*)arg;

    /* 关闭chirp 1min后，使能chirp检测 */
    hal_radar_enable_chirp_det(1);

    if (hmac_radar_crazy_report_det_timer(hal_device) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF,
            "{hmac_radar_disable_chirp_det_timeout::hmac_radar_crazy_report_det_timer err!}");
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_crazy_report_det_timeout
 功能描述  : crazy report超时
*****************************************************************************/
osal_u32 hmac_radar_crazy_report_det_timeout(osal_void *arg)
{
    hal_dfs_radar_filter_stru *dfs_radar;
    hal_to_dmac_device_stru   *hal_device;
    osal_u8                      max_chirp_cnt;

    hal_device = (hal_to_dmac_device_stru*)arg;
    dfs_radar   = &(hal_device->dfs_radar_filter);

    max_chirp_cnt = (dfs_radar->radar_type == HAL_DFS_RADAR_TYPE_FCC) ?
        MAX_IRQ_CNT_IN_CHIRP_CRAZY_REPORT_DET_FCC : MAX_IRQ_CNT_IN_CHIRP_CRAZY_REPORT_DET_ETSI;

    /* 如果20s内检测到中断个数超过40个，则关闭chirp检测 */
    if (dfs_radar->chirp_cnt_for_crazy_report_det >= max_chirp_cnt) {
        /* 关闭chirp检测 */
        hal_radar_enable_chirp_det(OSAL_FALSE);

        oam_warning_log2(0, OAM_SF_DFS,
            "{hmac_radar_crazy_report:crazy_cnt[%d] in 20s>=40:turn off chirp det and wait %us.}\r\n",
            dfs_radar->chirp_cnt_for_crazy_report_det,
            dfs_radar->timer_disable_chirp_det.timeout / 1000); /* 1000代表时间进制 */
    } else {
        /* 使能chirp检测 */
        hal_radar_enable_chirp_det(OSAL_TRUE);
    }

    dfs_radar->chirp_cnt_for_crazy_report_det = 0;
    dfs_radar->crazy_report_cnt               = OSAL_FALSE;
    frw_create_timer_entry(&(dfs_radar->timer_disable_chirp_det), hmac_radar_disable_chirp_det_timeout,
        dfs_radar->timer_disable_chirp_det.timeout, hal_device, OSAL_FALSE);

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_radar_crazy_report_det_timer
 功能描述  : crazy report定时器
*****************************************************************************/
oal_bool_enum_uint8 hmac_radar_crazy_report_det_timer(hal_to_dmac_device_stru *hal_device)
{
    hal_dfs_radar_filter_stru       *dfs_radar;

    dfs_radar   = &(hal_device->dfs_radar_filter);

    /* 使能20s内中断个数统计 */
    dfs_radar->crazy_report_cnt = OSAL_TRUE;

    /* 启动20s超时定时器 */
    frw_create_timer_entry(&(dfs_radar->timer_crazy_report_det), hmac_radar_crazy_report_det_timeout,
        dfs_radar->timer_crazy_report_det.timeout, hal_device, OSAL_FALSE);

    return OSAL_TRUE;
}

#endif
#endif /* _PRE_WLAN_FEATURE_DFS_OPTIMIZE */


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

