/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: ftm特性
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_ftm.h"
#include "mac_resource_ext.h"
#include "mac_ie.h"
#include "mac_device_ext.h"
#include "hmac_feature_dft.h"
#include "oal_mem_hcm.h"
#include "oal_netbuf_data.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FTM_ROM_C
#ifdef _PRE_WLAN_FEATURE_FTM
/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
osal_u8 g_ftm_vap_id = 0;

/*****************************************************************************
  4 函数实现
*****************************************************************************/

osal_u32 hmac_ftm_handle_max_power(const ftm_timer_stru *ftm_timer)
{
    osal_u16 letf1 = ftm_timer->ftm_rpt_reg2.reg2.rpt_ftm_max_pre1_power;
    osal_u16 letf2 = ftm_timer->ftm_rpt_reg3.reg3.rpt_ftm_max_pre2_power;
    osal_u16 letf3  = ftm_timer->ftm_rpt_reg3.reg3.rpt_ftm_max_pre3_power;
    osal_u16 letf4 = ftm_timer->ftm_rpt_reg4.reg4.xwGROtRIOIz8OGwmKOGvqmw_;
    osal_u16 letf5 = ftm_timer->ftm_rpt_reg4.reg4.x_uLwNLSwSGPwu_WxwuiTW__;
    osal_u16 max = ftm_timer->ftm_rpt_reg1.reg1.rpt_ftm_max_power;
    osal_char correct;
    /* 8表示max小于该值 */
    if (max < 8) {
        return 255; /* 255 丢弃该测量 */
    }
    /* 3表示left4该power值 */
    if (letf4 >= 3) {
        correct = hmac_ftm_interpolate(letf3, letf4, letf5);
        if (correct != 127) { /* 127表示correct的错误值 */
            return 31 - correct;  /* 31表示返回power的测量值 */
        }
    }
    /* 5表示left3该power值 */
    if (letf3 >= 5) {
        correct = hmac_ftm_interpolate(letf2, letf3, letf4);
        if (correct != 127) { /* 127表示correct的错误值 */
            return 25 - correct;  /* 25表示返回power的测量值 */
        }
    }
    /* 6表示left2该power值 */
    if (letf2 >= 6) {
        correct = hmac_ftm_interpolate(letf2, letf3, letf4);
        if (correct != 127) { /* 127表示correct的错误值 */
            return 19 - correct; /* 19表示返回power的测量值 */
        }
    }
    /* 7表示left1该power值 */
    if (letf1 >= 7) {
        correct = hmac_ftm_interpolate(letf2, letf3, letf4);
        if (correct != 127) { /* 127表示correct的错误值 */
            return 12 - correct; /* 12表示返回power的测量值 */
        }
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_sync_handle_time
 功能描述  : 处理ftm同步方案的t1、t2、t3、t4，并计算时钟偏差
*****************************************************************************/
osal_u32 hmac_ftm_sync_handle_time(ftm_timer_stru *timer, osal_u64 *temp_td, osal_s64 *temp_delta)
{
    osal_u64 temp_t1, temp_t2, temp_t3, temp_t4;

    if (timer == OSAL_NULL) {
        return OAL_FAIL;
    }
    temp_t1 = timer->t1;
    temp_t2 = timer->t2;
    temp_t3 = timer->t3;
    temp_t4 = timer->t4;

    if (((temp_t4 - temp_t1) <= (temp_t3 - temp_t2)) ||
        (temp_t1 == 0 && temp_t4 == 0) || (temp_t2 == 0 && temp_t3 == 0)) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_ftm_sync_handle_time:t1 t2 t3 t4 not available!");
        (osal_void)memset_s(timer, sizeof(ftm_timer_stru), 0, sizeof(ftm_timer_stru));
        return OAL_FAIL;
    }
    // 0x30表示时钟溢出绕回处理
    temp_t4 = (temp_t4 <= temp_t1) ? (temp_t4 + ((osal_u64)1 << 0x30)) : temp_t4;

    // 0x30表示时钟溢出绕回处理
    temp_t3 = (temp_t3 <= temp_t2) ? (temp_t3 + ((osal_u64)1 << 0x30)) : temp_t3;

    // 同步方案时钟误差计算
    if ((temp_t2 + temp_t3) > (temp_t1 + temp_t4)) {
        *temp_td = (temp_t2 + temp_t3) - (temp_t1 + temp_t4);
    } else {
        // 0x31表示时钟溢出绕回处理
        *temp_td = ((temp_t2 + temp_t3 + ((osal_u64)1 << 0x31)) - (temp_t1 + temp_t4));
    }

    *temp_delta = ((osal_s64)(temp_t2 + temp_t3) - (osal_s64)(temp_t1 + temp_t4));
    // 时钟偏差饱和处理 1 << 0x2a表示偏差最大值
    if (*temp_delta > ((osal_s64)1 << 0x2a) - 1) {
        *temp_delta = ((osal_s64)1 << 0x2a) - 1;
    } else if (*temp_delta < (1 - ((osal_s64)1 << 0x2a))) {
        *temp_delta = (1 - ((osal_s64)1 << 0x2a));
    }

    return OAL_SUCC;
}

osal_u32 hmac_set_ftm_correct_time(mac_set_ftm_time_stru ftm_time)
{
    unref_param(ftm_time);
    return OAL_CONTINUE;
}

osal_u64 hmac_ftm_time_limit(osal_u64 t1, osal_u64 t2, osal_u64 t3, osal_u64 t4)
{
    osal_u64 time = 0;
    if ((t4 - t1) > (t3 - t2)) {
        time = ((t4 - t1) - (t3 - t2)) >> 1;
    }
    return time;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_get_distance
 功能描述  : 处理ftm时间戳
*****************************************************************************/
osal_u32 hmac_ftm_get_distance(hmac_vap_stru *hmac_vap, osal_u64 *distance, osal_u8 session_id)
{
    osal_u64 temp_distance, temp_t1, temp_t2, temp_t3, temp_t4, temp_td;
    hmac_ftm_data_report d2h_data;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    osal_u8 index;
    osal_char rssi_dbm;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_init = ftm->ftm_init;
    index = ftm_init[session_id].follow_up_dialog_token % MAC_FTM_TIMER_CNT;
    /* 检测对应的索引的dialog_token是否一致，防止已被覆盖 */
    if (ftm_init[session_id].ftm_timer[index].dialog_token != ftm_init[session_id].follow_up_dialog_token) {
        oam_warning_log2(0, OAM_SF_FTM,
            "{hmac_ftm_get_distance:: devic_dialog_token[%d]!=ftm_follow_updialog_token[%d]!.}",
            ftm_init[session_id].ftm_timer[index].dialog_token, ftm_init[session_id].follow_up_dialog_token);
        return OAL_FAIL;
    }

    rssi_dbm = ftm_init[session_id].ftm_timer[index].rssi_dbm;
    temp_t1 = ftm_init[session_id].ftm_timer[index].t1;
    temp_t2 = ftm_init[session_id].ftm_timer[index].t2;
    temp_t3 = ftm_init[session_id].ftm_timer[index].t3;
    temp_t4 = ftm_init[session_id].ftm_timer[index].t4;

    if ((temp_t4 <= temp_t1) || (temp_t3 <= temp_t2)) {
        oam_warning_log1(0, OAM_SF_FTM,
            "vap_id[%d] hmac_ftm_get_distance::t1 t2 t3 t4 not available!!!", hmac_vap->vap_id);
        (osal_void)memset_s(&ftm_init[session_id].ftm_timer[index], sizeof(ftm_timer_stru), 0, sizeof(ftm_timer_stru));
        return OAL_FAIL;
    }

    temp_td = hmac_ftm_time_limit(temp_t1, temp_t2, temp_t3, temp_t4);
    /* 单位0.25mm，12/10即表示扩大到1.25倍 */
    temp_distance = (temp_td * 12) / 10;

    oam_warning_log_alter(0, OAM_SF_FTM,
        /* 9表示9个参数数目 */
        "ftm data:rtt[%d], reg0[%x], reg1[%x], reg2[%x], reg3[%x], 4[%x], cali[%d], dialog_token[%d], rssi[%d]", 9,
        /* 10表示获得temp_td右移10位 */
        (osal_u32)(temp_td >> 10), ftm_init[session_id].ftm_timer[index].ftm_rpt_reg0.reg_value,
        ftm_init[session_id].ftm_timer[index].ftm_rpt_reg1.reg_value,
        ftm_init[session_id].ftm_timer[index].ftm_rpt_reg2.reg_value,
        ftm_init[session_id].ftm_timer[index].ftm_rpt_reg3.reg_value,
        ftm_init[session_id].ftm_timer[index].ftm_rpt_reg4.reg_value,
        ftm->ftm_cali_time, ftm_init[session_id].ftm_timer[index].dialog_token, rssi_dbm);

    d2h_data.ftm_rtt = temp_td;
    d2h_data.distance = temp_distance;
    d2h_data.time_sync = ftm_init[session_id].timer_syn;
    hmac_config_d2h_ftm_info(hmac_vap, &d2h_data);

    (osal_void)memset_s(&ftm_init[session_id].ftm_timer[index], sizeof(ftm_timer_stru), 0, sizeof(ftm_timer_stru));

    *distance = temp_distance;

    return OAL_CONTINUE;
}

OSAL_STATIC osal_void hmac_ftm_get_delta_clock_cali(osal_s64 clock_time, osal_u64 t1_delta, osal_u64 temp_td,
    osal_u8 time_sync, hmac_vap_stru *hmac_vap)
{
    hmac_ftm_data_report d2h_data;

    // 配置晶振偏差
    hal_set_ftm_crystal_oscillator_offset_update_status(OSAL_FALSE);
    hal_set_ftm_crystal_oscillator_offset((osal_u32)clock_time);
    hal_set_ftm_crystal_oscillator_offset_update_status(OSAL_TRUE);

    d2h_data.t1_delta = t1_delta;
    d2h_data.time_offset = temp_td;
    d2h_data.clock_time = clock_time;
    d2h_data.time_sync = time_sync;
    hmac_config_d2h_ftm_info(hmac_vap, &d2h_data);
}

/*****************************************************************************
 函 数 名  : hmac_ftm_get_delta
 功能描述  : 处理ftm时间戳
*****************************************************************************/
osal_u32 hmac_ftm_get_delta(hmac_vap_stru *hmac_vap,  osal_u8 session_id)
{
    osal_u64 temp_td, t1_delta;
    osal_s64 temp_delta, clock_time;
    hmac_ftm_data_report d2h_data;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    ftm_timer_stru *timer = OSAL_NULL;
    osal_u8 index;
    osal_u32 ret;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_init = ftm->ftm_init;
    index = ftm_init[session_id].follow_up_dialog_token % MAC_FTM_TIMER_CNT;
    timer = &(ftm_init[session_id].ftm_timer[index]);

    if (ftm_init[session_id].time_syn_count != FTM_TIME_SYNC_FTM2) {
        return OAL_CONTINUE;
    }
    /* 检测对应的索引的dialog_token是否一致，防止已被覆盖 */
    if (timer->dialog_token != ftm_init[session_id].follow_up_dialog_token) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_get_delta:devic_dialog_token!=ftm_follow_updialog_token!}");
        return OAL_FAIL;
    }

    ret = hmac_ftm_sync_handle_time(timer, &temp_td, &temp_delta);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_ftm_get_delta: hmac_ftm_sync_handle_time fail!");
        return OAL_FAIL;
    }

    // 配置时钟偏差
    hal_set_ftm_clock_offset_update_status(OSAL_FALSE);
    hal_set_ftm_clock_offset(temp_td);
    hal_set_ftm_clock_offset_update_status(OSAL_TRUE);

    t1_delta = timer->t1 - ftm_init[session_id].time_syn_delta_t1;
    // 0x30表示时钟溢出绕回处理
    t1_delta = (timer->t1 <= ftm_init[session_id].time_syn_delta_t1) ? (t1_delta + ((osal_u64)1 << 0x30)) : t1_delta;

    ftm_init[session_id].time_syn_delta_t1 = timer->t1;

    if (ftm_init[session_id].clock_calibrated == OSAL_TRUE && t1_delta != 0) {
        // 计算晶振偏差，并做饱和处理
        clock_time = hal_sat_crystal_oscillator_offset(temp_delta, (osal_s64)t1_delta);
        hmac_ftm_get_delta_clock_cali(clock_time, t1_delta, temp_td, ftm_init[session_id].timer_syn, hmac_vap);

        wifi_printf("[FTM]hmac_ftm_get_delta: t1_delta=0x%llx time_offset=0x%llx clock_time=0x%llx time_sync=%d \r\n",
            t1_delta, temp_td, clock_time, ftm_init[session_id].timer_syn);
    }

    oam_warning_log_alter(0, OAM_SF_FTM,
        /* 9表示9个参数数目 */
        "ftm_rpt_reg:time[%d], reg0[%x], reg1[%x], reg2[%x], reg3[%x], 4[%x], cali[%d], dialog_token[%d], rssi[%d]", 9,
        /* 10表示获得temp_td右移10位 */
        (osal_u32)(temp_td >> 10), timer->ftm_rpt_reg0.reg_value, timer->ftm_rpt_reg1.reg_value,
        timer->ftm_rpt_reg2.reg_value, timer->ftm_rpt_reg3.reg_value, timer->ftm_rpt_reg4.reg_value, ftm->ftm_cali_time,
        timer->dialog_token, timer->rssi_dbm);

    ftm_init[session_id].clock_calibrated = 1;

    return OAL_CONTINUE;
}
/***********************************************************
* 功能描述:  依据校准模式和业务模式的ADC频率计算数据时延 *
************************************************************/
osal_u32 hmac_ftm_get_time_delay(hmac_vap_stru *hmac_vap, osal_u32 intp_freq)
{
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    osal_u16 adc_cycle = 0;
    osal_u16 cali_delay = 0;
    osal_u16 input_delay = 0;
    osal_u16 dc_remove_delay;
    osal_float digital_agc_delay, lpf1_delay, rxiq_filter_delay;
    osal_u32 digital_scaling_bypass, lpf1_bypass, filter_force_bypass;

    if ((ftm == OSAL_NULL) || (hmac_vap == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_FTM, "{hmac_ftm_get_time_delay::ftm[%p].}", ftm);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 所有delay均以ADC 640M频率下单采样周期时长为1单位 */
    switch (intp_freq) {
        case 0: /* 0: ADC频率80M */
            /* 1个ADC采样周期为8个单位 */
            adc_cycle = 8;
            /* 输入延时为4个单位 */
            input_delay = 4;
            break;
        case 1: /* 1: ADC频率160M */
            /* 1个ADC采样周期为4个单位 */
            adc_cycle = 4;
            /* 输入延时为2个单位 */
            input_delay = 2;
            break;
        case 2: /* 2: ADC频率320M */
            /* 1个ADC采样周期为2个单位 */
            adc_cycle = 2;
            /* 输入延时为2个单位 */
            input_delay = 2;
            break;
        default:
            break;
    }
    switch (ftm->cali_adc_freq) {
        case 0: /* 0: ADC频率80M */
            /* 校准延时为4个单位 */
            cali_delay = 4;
            break;
        case 1: /* 1: ADC频率160M */
        case 2: /* 2: ADC频率320M */
            /*  校准延时均为2个单位 */
            cali_delay = 2;
            break;
        default:
            break;
    }
    dc_remove_delay = adc_cycle; /* dc remove延时固定为一个ADC采样周期 */
    hal_get_rx_digital_scaling_bypass(&digital_scaling_bypass);
    /* digital_scaling_bypass为0时，digital agc延时为2.5(5/2)个ADC采样周期 */
    digital_agc_delay = (digital_scaling_bypass == 0) ? ((osal_float)adc_cycle * 5 / 2) : 0;
    hal_get_rx_lpf1_bypass(&lpf1_bypass);
    /* lpf1_bypass为0时，LPF1延时为10.5(21 / 2)个ADC采样周期 */
    lpf1_delay = (lpf1_bypass == 0) ? ((osal_float)adc_cycle * 21 / 2) : 0;
    hal_get_rxiq_filter_force_bypass(&filter_force_bypass);
    /* filter_force_bypass为0时，rxiq filter延时为11.5(23 / 2)个ADC采样周期 */
    rxiq_filter_delay = (filter_force_bypass == 0) ? ((osal_float)adc_cycle * 23 / 2) : 0;
    /* 最终延时为除校准延时外五项的和减去校准延时。使用时将其转为960M频率下周期单位的一半，即先乘以3，再除4 */
    return (dc_remove_delay + digital_agc_delay + lpf1_delay + rxiq_filter_delay + input_delay - cali_delay) * 3 / 4;
}

osal_u32 hmac_save_tx_ftm_complete_time(hmac_vap_stru *hmac_vap, osal_u8 session_id, osal_u64 ftm_t1, osal_u64 ftm_t4)
{
    osal_u32 fix, time_sync_enable, phy_freq, intp_freq, time_delay;
    ftm_timer_stru *ftm_timer;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    osal_u8 index;
    osal_u64 t1, t4;
    osal_char time_intp;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_timer = ftm->ftm_rspder[session_id].ftm_timer;

    oam_warning_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_save_tx_ftm_complete_time::get t1 t4.}", hmac_vap->vap_id);

    /* 记录dialog token 对应的时间索引 */
    index = ftm->ftm_rspder[session_id].dialog_token_ack % MAC_FTM_TIMER_CNT;

    /* 初始化 */
    ftm_timer[index].t1 = 0;
    ftm_timer[index].t4 = 0;
    ftm_timer[index].dialog_token = ftm->ftm_rspder[session_id].dialog_token_ack;
    /* 写入T1 */
    t1 = hal_get_ftm_time(ftm_t1);
    /* 写入T4 */
    t4 = hal_check_ftm_t4(ftm_t4);
    if ((t1 == 0) || (t4 == 0)) {
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_save_tx_ftm_complete_time:time is zero!", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    // 获取phy时钟频率、ADC采样频率
    hmac_ftm_handle_freq(hmac_vap, &time_sync_enable, &phy_freq, &intp_freq);
    time_delay = hmac_ftm_get_time_delay(hmac_vap, intp_freq);

    /* 计算时间，单位皮秒 */
    ftm_timer[index].t1 = (time_sync_enable == OAL_FALSE) ?
        /* 10表示固定偏差时间左移10位获得皮秒时间 */
        (osal_u64)(((t1 >> phy_freq) * PERIOD_OF_FTM_TIMER + time_delay) << 10) : t1;

    /* 计算插值 统一转为960M 时钟 */
    time_intp = hal_get_ftm_t4_intp(ftm_t4) >> intp_freq;

    hal_get_ftm_rtp_reg(&ftm_timer[index].ftm_rpt_reg0.reg_value, &ftm_timer[index].ftm_rpt_reg1.reg_value,
        &ftm_timer[index].ftm_rpt_reg2.reg_value, &ftm_timer[index].ftm_rpt_reg3.reg_value,
        &ftm_timer[index].ftm_rpt_reg4.reg_value);

    fix = hmac_ftm_handle_max_power(&(ftm_timer[index]));
    /* 255 代表 fix parameter */
    if (fix == 255) {
        ftm_timer[index].t1 = 0;
        ftm_timer[index].t4 = 0;
        oam_warning_log1(0, 0, "vap_id[%d] hmac_save_tx_ftm_complete_time:can not fix ,drop!", hmac_vap->vap_id);
        return OAL_CONTINUE;
    }

    /* 计算时间，单位皮秒 */
    ftm_timer[index].t4 = (time_sync_enable == OAL_FALSE) ?
        /* 统一转为960M 时钟  10表示固定偏差时间左移10位获得皮秒时间 */
        (osal_u64)((osal_u64)(((osal_s64)(t4 >> phy_freq) * PERIOD_OF_FTM_TIMER - time_intp) -
        /* 10表示ftm_cali_time时间左移10位获得时间 */
        time_delay) << 10) : (osal_u64)((osal_s64)t4  - (time_intp << 10));

    /* 10表示ftm_cali_time时间左移10位获得时间 */
    ftm_timer[index].t4 = (osal_u64)(ftm_timer[index].t4 - ((osal_u64)ftm->ftm_cali_time << 10));

    oam_warning_log_alter(0, OAM_SF_FTM,
        /* 参数个数为9 */
        "ftm_rpt_reg:reg0[%x], 1[%x], 2[%x], 3[%x], 4[%x], mac = [%d], dialog_token[%d], time_intp [%d], cali [%d]", 9,
        ftm_timer[index].ftm_rpt_reg0.reg_value, ftm_timer[index].ftm_rpt_reg1.reg_value,
        ftm_timer[index].ftm_rpt_reg2.reg_value, ftm_timer[index].ftm_rpt_reg3.reg_value,
        ftm_timer[index].ftm_rpt_reg4.reg_value, ftm->ftm_rspder[session_id].mac_ra[5], /* 地址第5位数据 */
        ftm_timer[index].dialog_token, time_intp, ftm->ftm_cali_time);

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_process_ftm_ack_complete_process
 功能描述  : 处理发送ftm ack 完成中断事件
*****************************************************************************/
osal_u32 hmac_process_ftm_ack_complete_process(hmac_ftm_initiator_stru *ftm_init, hmac_vap_stru *hmac_vap,
    const hal_wlan_ftm_t2t3_rx_event_stru *ftm_t2t3, osal_u8 index)
{
    osal_u32 time_sync_enable, phy_freq, intp_freq;
    osal_u32 time_delay;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    osal_u64 tmp_t2, tmp_t3;
    osal_char time_intp;

    if ((ftm == OSAL_NULL) || (ftm_init == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 初始化 */
    ftm_init->ftm_timer[index].t2 = 0;
    ftm_init->ftm_timer[index].t3 = 0;

    /* 写入T2 */
    tmp_t2 = hal_check_ftm_t2(ftm_t2t3->t2);
    /* 写入T3 */
    tmp_t3 = hal_get_ftm_time(ftm_t2t3->t3);
    if ((tmp_t2 == 0) || (tmp_t3 == 0)) {
        oam_warning_log1(0, 0, "vap_id[%d] hmac_process_ftm_tx_ack_complete:time is 0", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    // 获取phy时钟频率、ADC采样频率
    hmac_ftm_handle_freq(hmac_vap, &time_sync_enable, &phy_freq, &intp_freq);
    time_delay = hmac_ftm_get_time_delay(hmac_vap, intp_freq);

    time_intp = hal_get_ftm_t2_intp(ftm_t2t3->t2) >> intp_freq;

    hal_get_ftm_rtp_reg(&(ftm_init->ftm_timer[index].ftm_rpt_reg0.reg_value),
        &(ftm_init->ftm_timer[index].ftm_rpt_reg1.reg_value), &(ftm_init->ftm_timer[index].ftm_rpt_reg2.reg_value),
        &(ftm_init->ftm_timer[index].ftm_rpt_reg3.reg_value), &(ftm_init->ftm_timer[index].ftm_rpt_reg4.reg_value));

    osal_u32 fix = hmac_ftm_handle_max_power(&(ftm_init->ftm_timer[index]));
    /* 255: fix parameter */
    if (fix == 255) {
        ftm_init->ftm_timer[index].t2 = 0;
        ftm_init->ftm_timer[index].t3 = 0;
        oam_warning_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_process_ftm_tx_ack_complete:can't fix!",
            hmac_vap->vap_id);
        hal_set_ftm_sample(OSAL_TRUE);
        return OAL_CONTINUE;
    }

    /* 计算t2 */
    ftm_init->ftm_timer[index].t2 = (time_sync_enable == OAL_FALSE) ?
        /* 10表示固定偏差时间左移10位获得皮秒时间 且同步方案不需要减去固定偏差 */
        (osal_u64)((osal_u64)(((osal_s64)(tmp_t2 >> phy_freq) * PERIOD_OF_FTM_TIMER - time_intp) -
            time_delay) << 10) : (osal_u64)((osal_s64)tmp_t2 - (time_intp << 10)); /* 10:表示皮秒时间 */

    /* 计算t3 */
    ftm_init->ftm_timer[index].t3 = (time_sync_enable == OAL_FALSE) ?
        /* 10表示固定偏差时间左移10位获得皮秒时间 且同步方案不需要减去固定偏差 */
        (osal_u64)(((tmp_t3 >> phy_freq) * PERIOD_OF_FTM_TIMER + time_delay) << 10) : tmp_t3; /* 10:表示皮秒时间 */

    ftm_init->ftm_timer[index].t3 =
        /* 10表示校准时间左移10位获得皮秒时间 */
        (osal_u64)(ftm_init->ftm_timer[index].t3 + ((osal_u64)ftm->ftm_cali_time << 10)); /* 10:表示皮秒时间 */

    hal_set_ftm_sample(OSAL_TRUE);

    return OAL_CONTINUE;
}
/***********************************************************************
* 保存FTM测距结果
************************************************************************/
osal_u32 hmac_save_ftm_range(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    if ((ftm == OSAL_NULL) || (session_id > ftm->session_num)) {
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

/***********************************************************************
* 处理FTM 测距请求
************************************************************************/
osal_u32 hmac_ftm_rrm_proc_rm_request(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    if ((hmac_vap == OSAL_NULL) || (netbuf == OSAL_NULL)) {
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}
/***********************************************************************
* 保存错误的FTM 测距会话
************************************************************************/
osal_u32 hmac_save_error_ftm_session(hmac_vap_stru *hmac_vap, mac_ftm_parameters_ie_stru *mac_ftmp,
    osal_u8 session_id, osal_u8 *data)
{
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    unref_param(data);
    if ((ftm == OSAL_NULL) || (mac_ftmp == OSAL_NULL) ||
        (session_id > ftm->session_num)) {
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}
/***********************************************************************
* 发送FTM Range请求
************************************************************************/
osal_u32 hmac_send_ftm_range_req(hmac_vap_stru *hmac_vap, mac_send_ftm_range_req_stru *ftm_range)
{
    if ((hmac_vap == OSAL_NULL) || (ftm_range == OSAL_NULL)) {
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}
/***********************************************************************
* FTM请求下发，参数配置预留接口
************************************************************************/
osal_u32 hmac_send_ftm_req_adjust_param(hmac_vap_stru *hmac_vap, hmac_ftm_initiator_stru *ftm_init,
    osal_u8 session_id)
{
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    if ((ftm == OSAL_NULL) || (ftm_init == OSAL_NULL) || (session_id > ftm->session_num)) {
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

osal_char hmac_ftm_interpolate(osal_u16 right, osal_u16 max, osal_u16 left)
{
    osal_s16 diff;
    osal_s16 sum;

    if (max < right || max < left) {
        /* 条件满足时才做插值，返回127 */
        return 127;
    }
    diff = right - left;
    sum  = max + right + max - left;
    /* 3表示获得sum右移3位值 */
    if (diff < 0 - (sum) + (sum >> 3)) { /* s_diff < -7/8*s_sum + */
        return -6; /* 插值返回-6 */
    }
    /* 2表示获得sum右移2位值 */
    if (diff < 0 - (sum >> 1) - (sum >> 2)) { /* s_diff < -3/4*s_sum */
        return -5; /* 插值返回-5 */
    }
    if (diff < 0 - (sum >> 1)) { /* s_diff < -1/2*s_sum */
        return -4; /* 插值返回-4 */
    }
    /* 3表示获得sum右移3位的值 */
    if (diff < 0 - (sum >> 1) + (sum >> 3)) { /* s_diff < -3/8*s_sum */
        return -3; /* 插值返回-3 */
    }
    /* 2表示获得sum右移2位的值 */
    if (diff < 0 - (sum >> 2)) { /* s_diff < -1/4*s_sum */
        return -2; /* 插值返回-2 */
    }
    /* 3表示获得sum右移3位的值 */
    if (diff < 0 - (sum >> 3)) { /* s_diff < -1/8*s_sum */
        return -1; /* 插值返回-1 */
    }
    /* 3表示获得sum右移3位的值 */
    if (diff < (sum >> 3)) { /* s_diff < 1/8*s_sum */
        return 0; /* 插值返回0 */
    }
    /* 2表示获得sum右移2位的值 */
    if (diff < (sum >> 2)) { /* s_diff < 1/4*s_sum */
        return 1; /* 插值返回1 */
    }
    /* 3表示获得sum右移3位的值 */
    if (diff < (sum >> 1) - (sum >> 3)) { /* s_diff < 3/8*s_sum */
        return 2; /* 插值返回2 */
    }
    if (diff < (sum >> 1)) { /* s_diff < 1/2*s_sum */
        return 3; /* 插值返回3 */
    }
    /* 2表示获得sum右移2位值 */
    if (diff < (sum >> 1) + (sum >> 2)) { /* s_diff < 3/4*s_sum */
        return 4; /* 插值返回4 */
    }
    /* 3表示sum右移3位 */
    if (diff < (sum) - (sum >> 3)) { /* s_diff < 7/8*s_sum */
        return 5; /* 插值返回5 */
    }
    return 6; /* 插值返回6 */
}

/*****************************************************************************
 函 数 名  : hmac_process_ftm_ack_complete
 功能描述  : 处理发送ftm ack 完成中断事件
*****************************************************************************/
osal_s32 hmac_process_ftm_ack_complete(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_netbuf_stru *netbuf = *(oal_netbuf_stru **)(msg->data);
    hal_wlan_ftm_t2t3_rx_event_stru *tmp_ftm_t2t3 = OSAL_NULL;
    hal_wlan_ftm_t2t3_rx_event_stru ftm_t2t3 = {0};
    hmac_ftm_stru *ftm = OSAL_NULL;
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_ftm_vap = mac_res_get_hmac_vap(g_ftm_vap_id);
    osal_u8 session_id, dialog_token, index;
    osal_u32 ret;

    oam_warning_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_process_ftm_tx_ack_complete::get t2 t3.}", hmac_vap->vap_id);

    if (netbuf == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    tmp_ftm_t2t3 = (hal_wlan_ftm_t2t3_rx_event_stru *)(oal_netbuf_data(netbuf));
    if (tmp_ftm_t2t3 == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 尽快取出数据，释放netbuf空间 */
    ftm_t2t3.dialog_token = tmp_ftm_t2t3->dialog_token;
    ftm_t2t3.t2 = tmp_ftm_t2t3->t2;
    ftm_t2t3.t3 = tmp_ftm_t2t3->t3;
    oal_netbuf_free(netbuf);

    if (hmac_ftm_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_ftm_vap, WLAN_FEATURE_INDEX_FTM);
    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_init = ftm->ftm_init;

    /* 获取session_id */
    session_id = ftm->initor_session_id;
    /* 此次FTM帧的dialog */
    dialog_token = ftm_t2t3.dialog_token;
    if ((ftm_init[session_id].dialog_token != dialog_token) || (ftm_init[session_id].ftm_initiator == OSAL_FALSE)) {
        oam_error_log4(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_process_ftm_tx_ack_complete::now token[%d], dialog_token[%d], ftm_initiator[%d].}",
            hmac_ftm_vap->vap_id, dialog_token, ftm_init[session_id].dialog_token,
            ftm_init[session_id].ftm_initiator);
        return OAL_FAIL;
    }

    index = dialog_token % MAC_FTM_TIMER_CNT;
    ftm_init[session_id].ftm_timer[index].dialog_token = dialog_token;

    ret = hmac_process_ftm_ack_complete_process(&ftm_init[session_id], hmac_ftm_vap, &ftm_t2t3, index);
    if (ret != OAL_CONTINUE) {
        oam_error_log2(0, OAM_SF_FTM, "vap_id[%d] {hmac_process_ftm_tx_ack_complete::process_ftm_ack fail:%d.}",
            hmac_ftm_vap->vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_process_send_ftm_complete
 功能描述  : 发送ftm帧完成call事件处理
*****************************************************************************/
osal_s32 hmac_process_send_ftm_complete(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_tx_ftm_frame_stru *ftm_info = (dmac_tx_ftm_frame_stru *)(msg->data);

    if ((hmac_vap == OSAL_NULL) || (ftm_info == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (ftm_info->frame_type == DMAC_TX_FTM_FRAME) {
        if (mac_check_ftm_enable(hmac_vap)) {
            hmac_check_tx_ftm_complete(hmac_vap, ftm_info);
        }
    } else if (ftm_info->frame_type == DMAC_TX_FTM_REQ_FRAME) {
        hal_set_ftm_sample(OSAL_TRUE);
    }

    return OAL_SUCC;
}

osal_void hmac_ftm_handle_freq(hmac_vap_stru *hmac_vap,
    osal_u32 *time_sync_enable, osal_u32 *phy_freq, osal_u32 *intp_freq)
{
    osal_u32 sync_flag;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    // 读取同步特性开关
    hal_get_ftm_new_status(time_sync_enable);
    sync_flag = *time_sync_enable;

    // 读取时钟频率(1: 160M 2: 320M 3: 640M)
    hal_get_ftm_phy_bw_mode(phy_freq);

    // 读取AD采样频率(0: 80M 1: 160M 2: 320M 3: 640M)
    hal_get_ftm_intp_freq(intp_freq);
    if (sync_flag == OSAL_FALSE) {
        // 获取射频校准值
        hmac_ftm_get_cali(hmac_vap, ftm->cali_phy_freq, ftm->cali_adc_freq);
    }

    return;
}

osal_u32 hmac_check_tx_ftm_complete(hmac_vap_stru *hmac_vap, dmac_tx_ftm_frame_stru *ftm_info)
{
    osal_u32                      ret;
    osal_s8                       session_id;
    osal_u8                       session_id_number;
    hmac_ftm_stru                *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_responder_stru      *ftm_rspder = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_rspder = ftm->ftm_rspder;
    session_id = hmac_ftm_find_session_index(hmac_vap, MAC_FTM_RESPONDER_MODE, ftm_info->address);
    if (session_id < 0) {
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_check_tx_ftm::session_id error!}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }
    session_id_number = (osal_u8)session_id;
    if (oal_compare_mac_addr(ftm_rspder[session_id_number].mac_ra, ftm_info->address) != 0) {
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_check_tx_ftm::mac addr error!}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    ftm_rspder[session_id_number].dialog_token_ack = ftm_info->dialog_token_ack;
    ret = hmac_save_tx_ftm_complete_time(hmac_vap, session_id_number, ftm_info->t1, ftm_info->t4);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_check_tx_ftm:save_tx_ftm_time_cb return [%d]", ret);
        return ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_init_scan_for_ftm
 功能描述  : 切到ftm所在信道接收ftm帧
*****************************************************************************/
osal_u32 hmac_ftm_initor_scan_for_ftm(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    osal_u8 chan_number;
    osal_u8 chan_number_ftm;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_init = ftm->ftm_init;

    chan_number = hmac_vap->channel.chan_number;
    chan_number_ftm = ftm_init[session_id].channel_ftm.chan_number;

    if ((chan_number != chan_number_ftm) && (chan_number != 0)) {
        if (hmac_check_ftm_switch_channel(hmac_vap, chan_number_ftm) == OSAL_FALSE) {
            oam_error_log4(0, OAM_SF_FTM,
                "vap_id[%d] {hmac_ftm_initor_scan_for_ftm::mode[%d] work_ch[%d] ftm_ch[%d] ,cann't switch ch!.}",
                hmac_vap->vap_id, hmac_vap->vap_mode, chan_number, chan_number_ftm);

            return OAL_FAIL;
        }
    }

    // 重置时间同步 FTM收帧计数器
    ftm_init[session_id].time_syn_count = 0;

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP || hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA && (chan_number != chan_number_ftm)) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_ftm_initor_scan_for_ftm::not support switch channel send ftmr}",
                hmac_vap->vap_id);
            return OAL_FAIL;
        }
        if (ftm_init[session_id].iftmr == OSAL_TRUE) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_ftm_initor_scan_for_ftm::hmac_send_iftmr}", hmac_vap->vap_id);
            hmac_ftm_initor_send_ftm_req(hmac_vap, session_id);
            ftm_init[session_id].iftmr = OSAL_FALSE;
            return OAL_SUCC;
        }
        if (ftm_init[session_id].ftm_trigger == OSAL_TRUE) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_ftm_initor_scan_for_ftm::ftm_send_trigger}", hmac_vap->vap_id);
            hmac_ftm_initor_send_trigger(hmac_vap, session_id);
            ftm_init[session_id].ftm_trigger = OSAL_FALSE;
            return OAL_SUCC;
        }
    }

    return OAL_SUCC;
}

osal_void hmac_ftm_set_format_and_bandwidth(wlan_bw_cap_enum_uint8 band_cap,
    wlan_phy_protocol_enum_uint8  protocol_mode,
    mac_ftm_parameters_ie_stru *mac_ftmp)
{
    /* Field  Value                Format Bandwidth (MHz)
      0      No preference        No preference
      1-3    Reserved             Reserved
      4      Non-HT               5
      5      Reserved             Reserved
      6      Non-HT               10
      7      Reserved             Reserved
      8      Non-HT,              20
             excluding
             Clause 15 (DSSS
             PHY specification
             for the 2.4 GHz
             band designated
             for ISM
             applications) and
             Clause 16 (High
             rate direct
             sequence spread
             spectrum (HR/
             DSSS) PHY
             specification)
      9      HT mixed             20
      10     VHT                  20
      11     HT mixed             40
      12     VHT                  40
      13     VHT                  80
      14     VHT                  80+80
      15     VHT (two             160
             separate RF LOs)

      16     VHT (single RF       160
             LO)

      17-30  Reserved             Reserved
      31     DMG                  2160
      32–63 Reserved             Reserved */
    if ((band_cap == WLAN_BW_CAP_20M) && (protocol_mode == WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE)) {
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_NO_HT_20M;
    } else if ((band_cap == WLAN_BW_CAP_20M) && (protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE)) {
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_HT_MIXED_20M;
    } else if ((band_cap == WLAN_BW_CAP_20M) && (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE)) {
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_VHT_20M;
    } else if ((band_cap == WLAN_BW_CAP_40M) && (protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE)) {
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_HT_MIXED_40M;
    } else if ((band_cap == WLAN_BW_CAP_40M) && (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE)) {
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_VHT_40M;
    } else if ((band_cap == WLAN_BW_CAP_80M) && (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE)) {
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_VHT_80M;
    } else if ((band_cap == WLAN_BW_CAP_80PLUS80) && (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE)) {
        /* 目前phy暂不支持160M, 所以使用默认HT 20M，实际应为14 (VHT 80+80M) */
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_HT_MIXED_20M;
    } else if ((band_cap == WLAN_BW_CAP_160M) && (protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE)) {
        /* 160M分两种，单RF,和双RF */
        /* phy暂不支持160M, 使用默认HT 20M，实际应为15 (VHT 160M two separate RF LOs) / 16 (VHT 160M single RF LOs) */
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_HT_MIXED_20M;
    } else {
        mac_ftmp->format_and_bandwidth = FTM_FORMAT_HT_MIXED_20M;
    }
}

osal_void hmac_ftm_get_format_and_bandwidth(wlan_bw_cap_enum_uint8 *band_cap,
    wlan_phy_protocol_enum_uint8  *protocol_mode,
    osal_u8 format_and_bandwidth)
{
    /* Field  Value                Format Bandwidth (MHz)
      0      No preference        No preference
      1-3    Reserved             Reserved
      4      Non-HT               5
      5      Reserved             Reserved
      6      Non-HT               10
      7      Reserved             Reserved
      8      Non-HT,              20
             excluding
             Clause 15 (DSSS
             PHY specification
             for the 2.4 GHz
             band designated
             for ISM
             applications) and
             Clause 16 (High
             rate direct
             sequence spread
             spectrum (HR/
             DSSS) PHY
             specification)
      9      HT mixed             20
      10     VHT                  20
      11     HT mixed             40
      12     VHT                  40
      13     VHT                  80
      14     VHT                  80+80
      15     VHT (two             160
             separate RF LOs)

      16     VHT (single RF       160
             LO)

      17-30  Reserved             Reserved
      31     DMG                  2160
      32–63 Reserved             Reserved */
    switch (format_and_bandwidth) {
        case FTM_FORMAT_NO_HT_20M:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            break;
        case FTM_FORMAT_HT_MIXED_20M:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        case FTM_FORMAT_VHT_20M:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case FTM_FORMAT_HT_MIXED_40M:
            *band_cap = WLAN_BW_CAP_40M;
            *protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        case FTM_FORMAT_VHT_40M:
            *band_cap = WLAN_BW_CAP_40M;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case FTM_FORMAT_VHT_80M:
            *band_cap = WLAN_BW_CAP_80M;
            *protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        /**
         * 目前phy暂不支持160M, 所以使用默认HT 20M
         * 14 对应 VHT 80+80M
         * 15 对应 VHT 160M two separate RF LOs
         * 16 对应 VHT 160M single RF LOs
         * */
        case FTM_FORMAT_VHT_80PLUS80M:
        case FTM_FORMAT_VHT_160M_TWO_RF:
        case FTM_FORMAT_VHT_160M_SINGLE_RF:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        default:
            *band_cap = WLAN_BW_CAP_20M;
            *protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_encap_fine_timing_measure_para_element
 功能描述  : 封装 Fine Timing Measurement Parameters element
*****************************************************************************/
osal_void hmac_encap_fine_timing_measure_para_element(osal_u16 index, const osal_u8 *payload_addr,
    osal_u16 burst_cnt, const hmac_ftm_initiator_stru *ftm_init)
{
    mac_ftm_parameters_ie_stru *mac_ftmp = (mac_ftm_parameters_ie_stru *)&(payload_addr[index]);

    if (ftm_init == OSAL_NULL) {
        return;
    }

    mac_ftmp->eid = MAC_EID_FTMP;
    mac_ftmp->len = MAC_FTMP_LEN;

    mac_ftmp->status_indication = 0;                  /* Reserved */
    mac_ftmp->value = 0;                              /* Reserved */

    /* 回合个数(2^number_of_bursts_exponent) */
    mac_ftmp->number_of_bursts_exponent = 0;
    while (burst_cnt >> 1) {
        mac_ftmp->number_of_bursts_exponent++;
        burst_cnt = burst_cnt >> 1;
    }

    mac_ftmp->burst_duration = FTM_BURST_DURATION;
    mac_ftmp->partial_tsf_timer = 0;
    mac_ftmp->partial_tsf_timer_no_preference = 1;
    mac_ftmp->asap_capable = ftm_init->en_asap;
    mac_ftmp->asap = ftm_init->asap;

    /* 获取带宽 */
    hmac_ftm_set_format_and_bandwidth(ftm_init->band_cap, ftm_init->prot_format, mac_ftmp);
    /* 每个burst发ftm帧的个数，ftm帧之间的间隔，以及burst的间隔 */
    mac_ftmp->ftms_per_burst = (ftm_init->ftms_per_burst_cmd > FTM_PER_BURST) ?
                               ftm_init->ftms_per_burst_cmd : FTM_PER_BURST;
    mac_ftmp->min_delta_ftm  = FTM_MIN_DELTA;
    mac_ftmp->burst_period   = ftm_init->burst_period;
}

/*****************************************************************************
 函 数 名  : hmac_encap_ftm_req_mgmt
 功能描述  : 封装 iFTMR
*****************************************************************************/
osal_u16 hmac_encap_ftm_req_mgmt(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *buffer, osal_u8 session_id)
{
    osal_u16                 index = 0;
    osal_u16                 burst_cnt;
    osal_u8                 *mac_header   = oal_netbuf_header(buffer);
    osal_u8                 *payload_addr = oal_netbuf_data_offset(buffer, MAC_80211_FRAME_LEN);
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return (osal_u16)OAL_ERR_CODE_PTR_NULL;
    }

    ftm_init = ftm->ftm_init;
    burst_cnt = ftm_init[session_id].burst_cnt;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置 address1(接收端): 对端 MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, ftm_init[session_id].bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /*************************************************************************************/
    /*                       	FTM Request frame - Frame Body                           */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Trigger | LCI Measurement Request(optional)|            */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1       |Variable                          |            */
    /* --------------------------------------------------------------------------------- */
    /* |LCI Report (optional) |                                                          */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    |                                    */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/
    payload_addr[index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    payload_addr[index++] = MAC_PUB_FTM_REQ;                      /* Public Action */

    payload_addr[index++] = 1;                                    /* Trigger */

    /*****************************************************************************************************/
    /*                   LCI Measurement Request (Measurement Request IE)                                */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* ------------------------------------------------------------------------------------------------- */
    /*                                                                                                   */
    /*****************************************************************************************************/
    /* 封装Measurement Request IE  */
    if (ftm_init[session_id].lci_ie == OSAL_TRUE) {
        payload_addr[index++] = MAC_EID_MEASREQ;                   /* Element ID */
        payload_addr[index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        payload_addr[index++] = 1;                                 /* Measurement Token */
        payload_addr[index++] = 0;                                 /* Measurement Req Mode */
        payload_addr[index++] = RM_RADIO_MEASUREMENT_LCI;          /* Measurement Type */
    }

    /* 封装Measurement Request IE  */
    if (ftm_init[session_id].location_civic_ie == OSAL_TRUE) {
        payload_addr[index++] = MAC_EID_MEASREQ;                   /* Element ID */
        payload_addr[index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        payload_addr[index++] = 1;                                 /* Measurement Token */
        payload_addr[index++] = 0;                                 /* Measurement Req Mode */
        payload_addr[index++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC;    /* Measurement Type */
    }

    /*******************************************************************/
    /*                    Fine Timing Measurement Parameters element   */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters|        */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                |        */
    /* --------------------------------------------------------------- */
    /*                                                                 */
    /*******************************************************************/
    /****************************************************************************************************************/
    /*                    Fine Timing Measurement Parameters                                                         */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* |B0             B1 |B2 B6 |B7       | B8             B11 |B12        B15 |B16       B23 |B24           B39 |  */
    /* ------------------------------------------------------------------------------------------------------------- */
    /* |Status Indication|Value|Reserved|Number of Bursts Exponent|Burst Duration|Min Delta FTM |Partial TSF Timer|  */
    /*                                                                                                           */
    /* ------------------------------------------------------------------------------------------------------------- */
    /* |B40                |B41          |B42  |B43        B47 |B48  B49 |B50      B55 |B56    B71 | */
    /* ------------------------------------------------------------------------------------------------------------- */
    /* |Partial TSF Timer No Preference|ASAP Capable|ASAP|FTMs per Burst|Reserved|Format And Bandwidth|Burst Period| */
    /*                                                                                                         */
    /*************************************************************************************************************/
    hmac_encap_fine_timing_measure_para_element(index, payload_addr, burst_cnt, &ftm_init[session_id]);
    index = index + sizeof(mac_ftm_parameters_ie_stru);

    return (osal_u16)(index + MAC_80211_FRAME_LEN);
}

/*****************************************************************************
 函 数 名  : hmac_ftm_initor_send_ftm_req
 功能描述  : 发送ftm请求帧
*****************************************************************************/
osal_u32 hmac_ftm_initor_send_ftm_req(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u32                     ret;
    osal_u16                     frame_len;
    oal_netbuf_stru             *netbuf = OSAL_NULL;
    mac_tx_ctl_stru             *tx_ctl = OSAL_NULL;
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru     *ftm_init = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_init = ftm->ftm_init;

    /* 申请管理帧内存 */
    netbuf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_sta_send_ftm_req::netbuf null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(netbuf, OSAL_NULL);
    oal_set_netbuf_next(netbuf, OSAL_NULL);

    /* 封装iftmr帧 */
    frame_len = hmac_encap_ftm_req_mgmt(hmac_vap, netbuf, session_id);
    if ((frame_len - MAC_80211_FRAME_LEN) > WLAN_SMGMT_NETBUF_SIZE) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log1(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_sta_send_ftm_req:: probably memory used cross-border.}", hmac_vap->vap_id);
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->mpdu_payload_len = frame_len;

    if (hmac_vap_set_cb_tx_user_idx(hmac_vap, tx_ctl, ftm_init[session_id].bssid) != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_FTM,
            "(hmac_sta_send_ftm_req::fail to find user %x:%x:%x:%x:xx:xx.}",
            ftm_init[session_id].bssid[0], /* 获得bssid第0位数据 */
            ftm_init[session_id].bssid[1], /* 获得bssid第1位数据 */
            ftm_init[session_id].bssid[2], /* 获得bssid第2位数据 */
            ftm_init[session_id].bssid[3]); /* 获得bssid第3位数据 */
    }
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, frame_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_encap_ftm_mgmt
 功能描述  : 封装 ftm
*****************************************************************************/
osal_u16 hmac_encap_ftm_mgmt(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *buffer, osal_u8 session_id)
{
    osal_u16                         index = 0;
    osal_u16                         burst_cnt;
    osal_u8                         *mac_header = oal_netbuf_header(buffer);
    osal_u8                         *payload_addr = oal_netbuf_data_offset(buffer, MAC_80211_FRAME_LEN);
    mac_ftm_parameters_ie_stru      *mac_ftmp;
    osal_u16                        *time_error;
    osal_u32                        *tsf;
    const hmac_ftm_responder_stru   *ftm_rspder;
    hmac_ftm_stru                   *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    if (ftm == OSAL_NULL) {
        return (osal_u16)OAL_ERR_CODE_PTR_NULL;
    }
    ftm_rspder = ftm->ftm_rspder;
    burst_cnt = ftm_rspder[session_id].burst_cnt;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置地址1 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, ftm_rspder[session_id].mac_ra);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 地址3 bssid */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /***************************************************************************************************/
    /*                        Fine Timing Measurement frame format                                     */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token |Follow Up Dialog Token |TOD |TOA |TOD Error |TOA Error | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |1        |1             |1            |1                      |6   |6   |2         |2         | */
    /* ----------------------------------------------------------------------------------------------- */
    /* |LCI Report (optional) |                                                                        */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |FTM Synchronization Information (optional) |                                                   */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /*                                                                                                 */
    /***************************************************************************************************/
    payload_addr[index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    payload_addr[index++] = MAC_PUB_FTM;                          /* Public Action */

    payload_addr[index++] = ftm_rspder[session_id].dialog_token;
    payload_addr[index++] = ftm_rspder[session_id].follow_up_dialog_token;

    (osal_void)memcpy_s(&payload_addr[index], FTM_FRAME_TOD_LENGTH, &ftm_rspder[session_id].tod, FTM_FRAME_TOD_LENGTH);
    index += FTM_FRAME_TOD_LENGTH;

    (osal_void)memcpy_s(&payload_addr[index], FTM_FRAME_TOA_LENGTH, &ftm_rspder[session_id].toa, FTM_FRAME_TOA_LENGTH);
    index += FTM_FRAME_TOA_LENGTH;

    time_error = (osal_u16 *)&payload_addr[index];
    *time_error = ftm_rspder[session_id].tod_error;
    index += FTM_FRAME_TOD_ERROR_LENGTH;

    time_error = (osal_u16 *)&payload_addr[index];
    *time_error = ftm_rspder[session_id].toa_error;
    index += FTM_FRAME_TOA_ERROR_LENGTH;

    /*****************************************************************************************************/
    /*                   LCI Measurement Request (Measurement Request IE)                                */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req | */
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             | */
    /* ------------------------------------------------------------------------------------------------- */
    /*                                                                                                   */
    /*****************************************************************************************************/
    /* 封装Measurement Request IE  */
    if (ftm_rspder[session_id].lci_ie == OSAL_TRUE) {
        payload_addr[index++] = MAC_EID_MEASREQ;                   /* Element ID */
        payload_addr[index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        payload_addr[index++] = 1;                                 /* Measurement Token */
        payload_addr[index++] = 0;                                 /* Measurement Req Mode */
        payload_addr[index++] = RM_RADIO_MEASUREMENT_LCI;          /* Measurement Type */
    }

    /* 封装Measurement Request IE  */
    if (ftm_rspder[session_id].location_civic_ie == OSAL_TRUE) {
        payload_addr[index++] = MAC_EID_MEASREQ;                   /* Element ID */
        payload_addr[index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        payload_addr[index++] = 1;                                 /* Measurement Token */
        payload_addr[index++] = 0;                                 /* Measurement Req Mode */
        payload_addr[index++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC;    /* Measurement Type */
    }

    /*******************************************************************/
    /*                    Fine Timing Measurement Parameters element   */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters|        */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                |        */
    /* --------------------------------------------------------------- */
    /*                                                                 */
    /*******************************************************************/
    /**************************************************************************************************************/
    /*                    Fine Timing Measurement Parameters                                                        */
    /* ------------------------------------------------------------------------------------------------------------ */
    /* |B0             B1 |B2 B6 |B7       | B8              B11 |B12        B15 |B16       B23 |B24         B39 |  */
    /* ------------------------------------------------------------------------------------------------------------ */
    /* |Status Indication|Value|Reserved|Number of Bursts Exponent|Burst Duration|Min Delta FTM|Partial TSF Timer| */
    /*                                                                                                             */
    /* ----------------------------------------------------------------------------------------------------------- */
    /* |B40                      |B41          |B42  |B43        B47 |B48  B49 |B50              B55 |B56    B71 | */
    /* ----------------------------------------------------------------------------------------------------------- */
    /* |Partial TSF Timer No Preference|ASAP Capable|ASAP |FTMs per Burst|Reserved|Format And Bandwidth|Burst Period| */
    /*                                                                                                               */
    /***************************************************************************************************************/
    if (ftm_rspder[session_id].ftm_parameters == OSAL_TRUE) {
        /* 封装 Fine Timing Measurement Parameters element  */
        mac_ftmp = (mac_ftm_parameters_ie_stru *)&(payload_addr[index]);

        mac_ftmp->eid = MAC_EID_FTMP;
        mac_ftmp->len = MAC_FTMP_LEN;
        /* 回合个数(2^number_of_bursts_exponent) */
        mac_ftmp->number_of_bursts_exponent = 0;
        while (burst_cnt >> 1) {
            mac_ftmp->number_of_bursts_exponent++;
            burst_cnt = burst_cnt >> 1;
        }

        mac_ftmp->burst_duration = FTM_BURST_DURATION;
        mac_ftmp->partial_tsf_timer = 0;
        mac_ftmp->partial_tsf_timer_no_preference = 1;
        mac_ftmp->asap_capable = 1;
        mac_ftmp->asap = ftm_rspder[session_id].asap;
        /* 设置带宽协议 */
        hmac_ftm_set_format_and_bandwidth(ftm_rspder[session_id].band_cap,
                                          ftm_rspder[session_id].prot_format,
                                          mac_ftmp);

        /* 每个burst发ftm帧的个数，ftm帧之间的间隔，以及burst的间隔 */
        mac_ftmp->ftms_per_burst = ftm_rspder[session_id].ftms_per_burst;
        mac_ftmp->min_delta_ftm = ftm_rspder[session_id].min_delta_ftm;
        mac_ftmp->burst_period = ftm_rspder[session_id].burst_period;

        mac_ftmp->status_indication = OSAL_TRUE;                                      /* Reserved */
        mac_ftmp->value = 0;                                                          /* Reserved */

        index = index + sizeof(mac_ftm_parameters_ie_stru);
    }

    /*************************************************************/
    /*         FTM Synchronization Information element           */
    /* --------------------------------------------------------- */
    /* |Element ID |Length |Element ID Extension | TSF Sync Info| */
    /* --------------------------------------------------------- */
    /* |1          |1      | 1                   | 4            | */
    /* --------------------------------------------------------- */
    /*                                                           */
    /*************************************************************/
    if (ftm_rspder[session_id].ftm_synchronization_information == OSAL_TRUE) {
        payload_addr[index++] = MAC_EID_FTMSI;                   /* Element ID */
        payload_addr[index++] = 5;                               /* 设置地址长度为5 */
        payload_addr[index++] = MAC_EID_EXT_FTMSI;

        tsf = (osal_u32 *)&payload_addr[index];
        *tsf = ftm_rspder[session_id].tsf;
        index += 4; /* index偏移4字节 */
    }

    return (osal_u16)(index + MAC_80211_FRAME_LEN);
}

osal_u32 hmac_ftm_rspder_send_ftm(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u32                     ret;
    osal_u16                     frame_len;
    oal_netbuf_stru             *netbuf = OSAL_NULL;
    mac_tx_ctl_stru             *tx_ctl;
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_responder_stru     *ftm_rspder = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_rspder = ftm->ftm_rspder;

    /* 申请管理帧内存 */
    netbuf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_send_ftm::pst_netbuf null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(netbuf, OSAL_NULL);
    oal_set_netbuf_next(netbuf, OSAL_NULL);

    /* 封装iftmr帧 */
    frame_len = hmac_encap_ftm_mgmt(hmac_vap, netbuf, session_id);
    if ((frame_len - MAC_80211_FRAME_LEN) > WLAN_SMGMT_NETBUF_SIZE) {
        oam_error_log1(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_send_ftm:: probably memory used cross-border.}", hmac_vap->vap_id);
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->mpdu_payload_len = frame_len;

    ret = hmac_vap_set_cb_tx_user_idx(hmac_vap, tx_ctl, ftm_rspder[session_id].mac_ra);
    if (ret != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_FTM,
            "(hmac_send_ftm::fail to find user by 0x:0x:0x:0x:xx:xx.}",
            ftm_rspder[session_id].mac_ra[0], ftm_rspder[session_id].mac_ra[1], /* 地址第0位和第1位 */
            ftm_rspder[session_id].mac_ra[2], ftm_rspder[session_id].mac_ra[3]); /* 获得地址第2\3位 */
    }

    tx_ctl->ac = WLAN_WME_AC_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, frame_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_encap_ftm_trigger_mgmt
 功能描述  : 封装 ftm_trigger
*****************************************************************************/
osal_u16 hmac_encap_ftm_trigger_mgmt(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *buffer, osal_u8 session_id)
{
    osal_u16                     index               = 0;
    osal_u8                     *mac_header          = oal_netbuf_header(buffer);
    osal_u8                     *payload_addr        = oal_netbuf_data_offset(buffer, MAC_80211_FRAME_LEN);
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru     *ftm_init = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return (osal_u16)OAL_ERR_CODE_PTR_NULL;
    }
    ftm_init = ftm->ftm_init;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置 address1(接收端): 对端 MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, ftm_init[session_id].bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /*************************************************************************************/
    /*                       	FTM Request frame - Frame Body                           */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Trigger |                                               */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1       |                                               */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/
    payload_addr[index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    payload_addr[index++] = MAC_PUB_FTM_REQ;                      /* Public Action */

    payload_addr[index++] = 1;                                    /* Trigger */

    return (osal_u16)(index + MAC_80211_FRAME_LEN);
}

osal_void hmac_get_ftm_burst_duration(hmac_vap_stru *hmac_vap, mac_ftm_mode_enum_uint8 ftm_mode,
    const mac_ftm_parameters_ie_stru *mac_ftmp, osal_u8 session_id)
{
    /* ----------------------- */
    /* Value | Represents     */
    /* ----------------------- */
    /*  0-1 |   Reserved     */
    /* ----------------------- */
    /*  2   |   250 μs      */
    /* ----------------------- */
    /*  3   |   500 μs      */
    /* ----------------------- */
    /*  4   |   1 ms         */
    /* ----------------------- */
    /*  5   |   2 ms         */
    /* ----------------------- */
    /*  6   |   4 ms         */
    /* ----------------------- */
    /*  7   |   8 ms         */
    /* ----------------------- */
    /*  8   |   16 ms        */
    /* ----------------------- */
    /*  9   |   32 ms        */
    /* ----------------------- */
    /*  10  |   64 ms        */
    /* ----------------------- */
    /*  11  |   128 ms       */
    /* ----------------------- */
    /*  12–14| Reserved     */
    /* ----------------------- */
    /*  15    | No preference */
    /* ----------------------- */
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru     *ftm_init = OSAL_NULL;
    hmac_ftm_responder_stru     *ftm_rspder = OSAL_NULL;
    osal_u8                      value;
    if (ftm == OSAL_NULL) {
        return;
    }
    ftm_init = ftm->ftm_init;
    ftm_rspder = ftm->ftm_rspder;

    /* 有效交互时间在4到11之间 */
    if ((mac_ftmp->burst_duration <= 11) && (mac_ftmp->burst_duration >= 4)) {
        value = mac_ftmp->burst_duration;

        if (ftm_mode == MAC_FTM_RESPONDER_MODE) {
            ftm_rspder[session_id].burst_duration = (osal_u8)osal_bit(value - 4); /* 4表示value减去4获取bit */
        }
        if (ftm_mode == MAC_FTM_INITIATOR_MODE) {
            ftm_init[session_id].burst_duration = (osal_u8)osal_bit(value - 4); /* 4表示value减去4获取bit */
        }
    } else {
        if (ftm_mode == MAC_FTM_RESPONDER_MODE) {
            ftm_rspder[session_id].burst_duration = 0;
        }
        if (ftm_mode == MAC_FTM_INITIATOR_MODE) {
            ftm_init[session_id].burst_duration = 0;
        }
    }
}

osal_u32 hmac_ftm_initor_send_trigger(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u32                    ret;
    osal_u16                    mgmt_len;
    oal_netbuf_stru            *mgmt_buf = OSAL_NULL;
    mac_tx_ctl_stru            *tx_ctl = OSAL_NULL;
    hmac_ftm_stru              *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru    *ftm_init = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_init = ftm->ftm_init;

    /* 申请管理帧内存 */
    mgmt_buf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (mgmt_buf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_ftm_send_trigger::alloc netbuf failed.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);

    /* 封装ftm trigger帧 */
    mgmt_len = hmac_encap_ftm_trigger_mgmt(hmac_vap, mgmt_buf, session_id);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    if (oal_memcmp(ftm_init[session_id].bssid, BROADCAST_MACADDR, sizeof(BROADCAST_MACADDR))) {
        /* 发送单播探测帧 */
        tx_ctl->tx_user_idx = (osal_u8)hmac_vap->assoc_vap_id;
        /* p2p模式下，GO属于ap模式 */
        if (is_ap(hmac_vap) == OSAL_TRUE) {
            ret = hmac_vap_set_cb_tx_user_idx(hmac_vap, tx_ctl, ftm_init[session_id].bssid);
            if (ret != OAL_SUCC) {
                oam_warning_log4(0, OAM_SF_FTM, "{hmac_ftm_send_trigger:fail to find user %x:%x:%x:%x:xx:xx.}",
                    ftm_init[session_id].bssid[0], ftm_init[session_id].bssid[1],
                    ftm_init[session_id].bssid[2], ftm_init[session_id].bssid[3]); /* 获得mac地址第2、3位数据 */
            }
        }
    } else {
        tx_ctl->ismcast = OSAL_TRUE;
        tx_ctl->tx_user_idx = (osal_u8)hmac_vap->multi_user_idx; /* ftm trigger帧是广播帧 */
    }

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, mgmt_buf, mgmt_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
        return ret;
    }

    return OAL_SUCC;
}

osal_u16 hmac_ftm_initor_bust_period_func(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init;

    if (ftm == OSAL_NULL) {
        return (osal_u16)OAL_ERR_CODE_PTR_NULL;
    }
    ftm_init = ftm->ftm_init;
    ftm_init[session_id].ftm_trigger = OSAL_TRUE;

    if (hmac_ftm_initor_scan_for_ftm(hmac_vap, session_id) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{hmac_ftm_initor_bust_period_func:: hmac_ftm_initor_scan_for_ftm err!}");
    }

    if (ftm_init[session_id].burst_cnt > 0) {
        ftm_init[session_id].burst_cnt--;
    }

    return OAL_SUCC;
}

osal_u32 hmac_ftm_initor_bust_period_timeout(osal_void *arg)
{
    osal_u32                   ret;
    osal_u8                    session_id;
    hmac_vap_stru             *hmac_vap = OSAL_NULL;
    hmac_ftm_stru             *ftm = OSAL_NULL;
    hmac_ftm_initiator_stru   *ftm_init = OSAL_NULL;
    ftm_timeout_arg_stru      *arg_temp = OSAL_NULL;

    if (arg == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_initor_bust_period_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    arg_temp = (ftm_timeout_arg_stru *)arg;
    hmac_vap = (hmac_vap_stru *)(arg_temp->hmac_vap);
    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_init = ftm->ftm_init;
    session_id = arg_temp->session_id;

    if (hmac_ftm_initor_bust_period_func(hmac_vap, session_id) != OAL_SUCC) {
        oam_error_log0(0,
            OAM_SF_ANTI_INTF, "{hmac_ftm_initor_bust_period_timeout:: hmac_ftm_initor_bust_period_func err!}");
    }
    if (ftm_init[session_id].burst_cnt == 0) {
        frw_destroy_timer_entry(&(ftm_init[session_id].ftm_tsf_timer));
        ret = hmac_save_ftm_range(hmac_vap, session_id);
        if (ret != OAL_CONTINUE) {
            oam_warning_log1(0, OAM_SF_ANY, "hmac_ftm_initor_bust_period_timeout:save_ftm_range_cb return [%d]", ret);
            return ret;
        }
    }

    return OAL_SUCC;
}

osal_void hmac_ftm_initor_start_bust(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u32                     timeout;
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru     *ftm_init = OSAL_NULL;
    ftm_timeout_arg_stru        *arg = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return;
    }
    ftm_init = ftm->ftm_init;
    arg = &(ftm_init[session_id].arg);

    timeout = ftm_init[session_id].burst_period * 100;  /* 100微秒级 */
    oam_warning_log4(0, OAM_SF_FTM,
        "vap_id[%d] {hmac_ftm_initor_start_bust::ftm bust start succ. burst_cnt[%d], ftms_per_burst[%d], timeout[%d]}",
        hmac_vap->vap_id, ftm_init[session_id].burst_cnt, ftm_init[session_id].ftms_per_burst,
        timeout);

    arg->hmac_vap = (osal_void *)hmac_vap;
    arg->session_id = session_id;

    if ((ftm_init[session_id].ftm_tsf_timer.is_registerd == OSAL_FALSE) && (ftm_init[session_id].burst_cnt > 1)) {
        frw_create_timer_entry(&(ftm_init[session_id].ftm_tsf_timer), hmac_ftm_initor_bust_period_timeout, timeout,
            arg, OSAL_TRUE);
    }

    if (ftm_init[session_id].burst_cnt) {
        hmac_ftm_initor_bust_period_func(hmac_vap, session_id);
    }
}

osal_void hmac_ftm_initor_wait_start_burst(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u16                     tsf_start_timeout;
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru     *ftm_init = OSAL_NULL;
    ftm_timeout_arg_stru        *arg = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return;
    }
    ftm_init = ftm->ftm_init;
    arg = &(ftm_init[session_id].arg);

    // 待讨论wxf
    /* 10表示将tsf_sync_info右移10位 */
    tsf_start_timeout = ftm_init[session_id].partial_tsf_timer - (osal_u16)(ftm_init[session_id].tsf_sync_info >> 10);

    oam_warning_log2(0, OAM_SF_FTM, "vap_id[%d] {hmac_ftm_initor_wait_start_burst::wait start,start_timeout[%d].}",
        hmac_vap->vap_id, tsf_start_timeout);

    arg->hmac_vap = (osal_void *)hmac_vap;
    arg->session_id = session_id;

    tsf_start_timeout += ftm->add_tsf;

    if (tsf_start_timeout) {
        if (ftm_init[session_id].ftm_tsf_timer.is_registerd == OSAL_FALSE) {
            frw_create_timer_entry(&(ftm_init[session_id].ftm_tsf_timer), hmac_ftm_initor_wait_start_burst_timeout,
                tsf_start_timeout, arg, OSAL_FALSE);
        }
    } else {
        hmac_ftm_initor_wait_start_burst_timeout((osal_void *)arg);
    }
}

osal_u32 hmac_ftm_initor_wait_start_burst_timeout(osal_void *arg)
{
    hmac_vap_stru             *hmac_vap;
    ftm_timeout_arg_stru      *arg_temp = OSAL_NULL;
    osal_u8                    session_id;

    if (arg == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_initor_wait_start_burst_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    arg_temp = (ftm_timeout_arg_stru *)arg;
    hmac_vap = (hmac_vap_stru *)(arg_temp->hmac_vap);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_initor_wait_start_burst_timeout::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    session_id = arg_temp->session_id;

    hmac_ftm_initor_start_bust(hmac_vap, session_id);

    return OAL_SUCC;
}

osal_void hmac_ftm_get_cali(hmac_vap_stru *hmac_vap, osal_u32 phy_freq, osal_u32 intp_freq)
{
    osal_u32  ftm_cali_rx_time = 0;
    osal_u32  ftm_cali_tx_time = 0;
    osal_u32  rx_intp_time = 0;
    osal_char time_intp;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    hal_get_ftm_cali_rx_time(&ftm_cali_rx_time);
    hal_get_ftm_cali_tx_time(&ftm_cali_tx_time);
    hal_get_ftm_cali_rx_intp_time(&rx_intp_time);

    oam_warning_log4(0, OAM_SF_FTM, "vap_id[%d] {hmac_ftm_get_cali::rx_time0x: %x tx_time0x: %x intp_time0x: %x}",
        hmac_vap->vap_id, ftm_cali_rx_time, ftm_cali_tx_time, rx_intp_time);

    /* 24:20 上报ftm time的插值结果,有符号 */
    if ((rx_intp_time & 0x1F) >= 0x10) {
        time_intp = (osal_char)((rx_intp_time & 0x1F) - 32) >> intp_freq; /* 32表示ftm time的插值结果减去32 */
    } else {
        time_intp = (osal_char)(rx_intp_time & 0x1F) >> intp_freq;
    }
    /* 将时钟统一转为960M时钟 */
    ftm_cali_rx_time = (osal_u32)((osal_s32)((ftm_cali_rx_time >> phy_freq) * PERIOD_OF_FTM_TIMER) - time_intp);
    /* 将时钟统一转为960M时钟 */
    ftm_cali_tx_time = (osal_u32)((ftm_cali_tx_time >> phy_freq) * PERIOD_OF_FTM_TIMER);
    /* 射频校准值绕回处理 */
    if (ftm_cali_rx_time < ftm_cali_tx_time) {
        /* rx 小于 tx时，加0x20做绕回处理 */
        ftm->ftm_cali_time = (osal_u32)(((osal_u64)ftm_cali_rx_time + ((osal_u64)1 << 0x20)) -
            (osal_u64)ftm_cali_tx_time);
    } else {
        ftm->ftm_cali_time = ftm_cali_rx_time - ftm_cali_tx_time;
    }

    oam_warning_log3(0, OAM_SF_FTM,
        "vap_id[%d] {hmac_ftm_get_cali::get ftm_cali_time = [%d], c_time_intp[%d].}",
        hmac_vap->vap_id, ftm->ftm_cali_time, time_intp);
}

/*****************************************************************************
 函 数 名  : hmac_ftm_initor_rx_ftm1
 功能描述  : 设置ftm_init参数
*****************************************************************************/
osal_void hmac_ftm_set_ftm_init_param(hmac_vap_stru *hmac_vap, mac_ftm_parameters_ie_stru *mac_ftmp,
    hmac_ftm_initiator_stru *ftm_init)
{
    ftm_init->burst_cnt = osal_bit(mac_ftmp->number_of_bursts_exponent);

    /* 每个回合FTM帧的个数 */
    if (mac_ftmp->ftms_per_burst > FTM_PER_BURST) {
        ftm_init->ftms_per_burst = mac_ftmp->ftms_per_burst;
    } else {
        ftm_init->ftms_per_burst = FTM_PER_BURST;
    }

    /* FTM帧的间隔时间 */
    ftm_init->min_delta_ftm = (mac_ftmp->min_delta_ftm) ? mac_ftmp->min_delta_ftm : FTM_MIN_DELTA;

    /* FTM 一个会话中ftm有效交互时间 */
    hmac_get_ftm_burst_duration(hmac_vap, MAC_FTM_INITIATOR_MODE, mac_ftmp, session_id);

    /* FTM 一个会话时间 */
    if (mac_ftmp->burst_period) {
        ftm_init->burst_period = mac_ftmp->burst_period;
    } else {
        /* 1000为时间进制 */
        ftm_init->burst_period = (ftm_init->ftms_per_burst * ftm_init->min_delta_ftm / 1000) + 1;
    }

    if (!mac_ftmp->partial_tsf_timer_no_preference) {
        ftm_init->partial_tsf_timer = mac_ftmp->partial_tsf_timer;
    }

    hmac_ftm_get_format_and_bandwidth(&band_cap, &prot_format, mac_ftmp->format_and_bandwidth);
    ftm_init->band_cap = band_cap;
    ftm_init->prot_format = prot_format;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_initor_rx_ftm1
 功能描述  :收到ftm1帧处理
*****************************************************************************/
osal_u32 hmac_ftm_initor_rx_ftm1(hmac_vap_stru *hmac_vap, osal_u8 *data, osal_u16 frame_len, osal_u8 session_id)
{
    osal_u32 ret;
    osal_u8 band_cap, prot_format;
    hmac_ftm_stru  *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    osal_u8 *ie = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_init = &(ftm->ftm_init[session_id]);

    ie = hmac_find_ie_ext_ie(MAC_EID_FTMSI, MAC_EID_EXT_FTMSI, &data[FTM_FRAME_OPTIONAL_IE_OFFSET],
        frame_len - FTM_FRAME_OPTIONAL_IE_OFFSET);
    if (ie != OSAL_NULL) {
        ftm_init->tsf_sync_info = *(osal_u32 *)&ie[FTM_FRAME_TSF_SYNC_INFO_OFFSET];
    }

    ie = mac_find_ie_etc(MAC_EID_FTMP, &data[FTM_FRAME_OPTIONAL_IE_OFFSET], frame_len - FTM_FRAME_OPTIONAL_IE_OFFSET);
    if (ie != OSAL_NULL && ie[1] >= MAC_FTMP_LEN) {
        mac_ftm_parameters_ie_stru *mac_ftmp = (mac_ftm_parameters_ie_stru *)ie;

        if (mac_ftmp->status_indication != OSAL_TRUE) {
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_ftm_initor_rx_ftm1::session ends,status_indication[%d]!}",
                hmac_vap->vap_id, mac_ftmp->status_indication);
            ret = hmac_save_error_ftm_session(hmac_vap, mac_ftmp, session_id, data);
            if (ret != OAL_CONTINUE) {
                oam_warning_log1(0, OAM_SF_ANY, "hmac_ftm_initor_rx_ftm1:save_error_ftm_session_cb return [%d]", ret);
                return ret;
            }
            return OSAL_FALSE;
        }

        /* 回合个数(2^number_of_bursts_exponent) */
        /* 15为最大限制 */
        if (mac_ftmp->number_of_bursts_exponent >= 15) {
            mac_ftmp->number_of_bursts_exponent = 0;
        }

        hmac_ftm_set_ftm_init_param(hmac_vap, mac_ftmp, ftm_init);

        oam_warning_log4(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_initor_rx_ftm1:: burst_cnt[%d], ftms_per_burst[%d], burst_period[%d]100ms.}",
            hmac_vap->vap_id, ftm_init->burst_cnt, ftm_init->ftms_per_burst, ftm_init->burst_period);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_initor_rx_ftmk
 功能描述  : 收到ftm帧处理，记录t2t3并计算距离
*****************************************************************************/
osal_void hmac_ftm_initor_rx_ftmk(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf, osal_u8 session_id)
{
    osal_u64                    distance_rst = 0;
    osal_u32                    ret = OAL_SUCC;
    osal_s32                    rc;
    osal_u8                     index, vap_id;
    dmac_rx_ctl_stru           *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb_const(netbuf);
    hmac_ftm_stru              *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru    *ftm_init = OSAL_NULL;
    osal_u8                    *data = oal_netbuf_rx_data(netbuf); /* 获取帧体指针 */

    if (ftm == OSAL_NULL) {
        return;
    }
    vap_id = hmac_vap->vap_id;
    ftm_init = ftm->ftm_init;

    /********************************************************************************************/
    /*            Fine Timing Measurement frame format                                          */
    /* ----------------------------------------------------------------------------------------- */
    /* |Category|Public Action|Dialog Token|Follow Up Dialog Token|TOD |TOA|TOD Error|TOA Error| */
    /* ----------------------------------------------------------------------------------------- */
    /* | 1      |1            |1           |1                     |6   |6  |2        |2        | */
    /* ----------------------------------------------------------------------------------------- */
    /*                                                                                          */
    /********************************************************************************************/
    /* 记录rssi */
    index = data[FTM_FRAME_DIALOG_TOKEN_OFFSET] % MAC_FTM_TIMER_CNT;
    ftm_init[session_id].ftm_timer[index].rssi_dbm = rx_ctl->rx_statistic.rssi_dbm;

    /* ftm1不处理 */
    if (data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET] == 0) {
        return;
    }

    /* 记录dialog token 对应的时间索引 */
    /* ftm帧记录的t1 和t4为上一次的时间 */
    index = ftm_init[session_id].follow_up_dialog_token % MAC_FTM_TIMER_CNT;

    /* 记录tod(t1) 复制长度为6字节的数据 */
    rc = memcpy_s(&(ftm_init[session_id].ftm_timer[index].t1), sizeof(osal_u64), &data[FTM_FRAME_TOD_OFFSET], 6);
    if (rc != EOK) {
        oam_error_log2(0, OAM_SF_FTM, "vap_id[%d] {memcpy_s failed, rc=%d}", vap_id, rc);
        return;
    }
    ftm_init[session_id].ftm_timer[index].t1 &= FTM_TMIE_MASK;

    if (memcpy_s(&(ftm_init[session_id].ftm_timer[index].t4), sizeof(osal_u64),
        &data[FTM_FRAME_TOA_OFFSET], 6) != EOK) { /* 6:记录toa(t4) 复制长度为6字节的数据 */
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_ftm_initor_rx_ftmk::memcpy_s failed}", vap_id);
        return;
    }
    ftm_init[session_id].ftm_timer[index].t4 &= FTM_TMIE_MASK;

    oam_warning_log3(0, OAM_SF_FTM, "vap_id[%d] {hmac_ftm_initor_rx_ftmk:follow_up_dialog=[%d],get t4-t1 = [%d]}",
        vap_id, ftm_init[session_id].follow_up_dialog_token,
        ftm_init[session_id].ftm_timer[index].t4 - ftm_init[session_id].ftm_timer[index].t1);

    /* 帧中带有tod和toa，可以进行距离计算 */
    if (ftm_init[session_id].timer_syn != OSAL_TRUE) {
        // 进行测距
        ret = hmac_ftm_get_distance(hmac_vap, &distance_rst, session_id);
    } else {
        // 进行同步
        ret = hmac_ftm_get_delta(hmac_vap, session_id);
    }
    if (ret == OAL_CONTINUE) {
        oam_warning_log2(0, OAM_SF_FTM, "vap_id[%d] hmac_ftm_initor_rx_ftmk::distance = %d", vap_id, distance_rst);
        ftm_init[session_id].range = (ftm_init[session_id].range == 0) ? (ftm_init[session_id].range | distance_rst) :
            ((ftm_init[session_id].range + distance_rst) >> 1);
    } else {
        oam_warning_log2(0, OAM_SF_FTM,
            "vap_id[%d] timer_sync val[%d] hmac_ftm_initor_rx_ftmk::fail", vap_id, ftm_init[session_id].timer_syn);
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_initor_rx_ftm
 功能描述  :收到ftm帧处理
*****************************************************************************/
osal_u32 hmac_ftm_find_session_id(const oal_netbuf_stru *netbuf, osal_u8 *data, osal_u16 *frame_len,
    osal_u8 *session_id_num)
{
    const dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    const mac_rx_ctl_stru *rx_info = OSAL_NULL;
    const mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_s8 session_id;

    rx_ctl = (const dmac_rx_ctl_stru *)oal_netbuf_cb_const(netbuf);
    rx_info = (const mac_rx_ctl_stru *)(&(rx_ctl->rx_info));

    /* 获取帧头信息 */
    frame_hdr = (const mac_ieee80211_frame_stru *)_const(&(rx_ctl->rx_info));

    /* 获取帧体指针 */
    data = oal_netbuf_rx_data(netbuf);
    *frame_len = mac_get_rx_cb_payload_len(rx_info);  /* 帧体长度 */

    (osal_void)memcpy_s(bssid, WLAN_MAC_ADDR_LEN, (osal_u8 *)frame_hdr + 10, WLAN_MAC_ADDR_LEN); /* 10偏移 */

    /* 查找session id */
    session_id = hmac_ftm_find_session_index(hmac_vap, MAC_FTM_INITIATOR_MODE, (const osal_u8 *)bssid);
    if (session_id < 0) {
        return OAL_FAIL;
    }
    *session_id_num = (osal_u8)session_id;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_initor_rx_ftm
 功能描述  :收到ftm帧处理，记录t2t3并计算距离
*****************************************************************************/
osal_u32 hmac_ftm_initor_rx_ftm(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    osal_u8 bssid[WLAN_MAC_ADDR_LEN] = {0};
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    osal_u16 frame_len;
    osal_u8 session_id_num;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* ftm initiator */
    if (mac_mib_get_fine_timing_msmt_init_activated(hmac_vap) == OSAL_FALSE) {
        oam_warning_log1(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_initor_rx_ftm::not a ftm_initiator, ignored.}", hmac_vap->vap_id);
        return OAL_SUCC;
    }
    ftm_init = ftm->ftm_init;

    /* 收到ftm帧处理 */
    if (hmac_ftm_find_session_id(netbuf, data, &frame_len, &session_id_num) == OAL_FAIL) {
        return OAL_FAIL;
    }

    if (oal_compare_mac_addr(ftm_init[session_id_num].bssid, bssid) != 0) {
        oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] {hmac_ftm_initor_rx_ftm::session_id error!}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    ftm->initor_session_id = session_id_num;

    g_ftm_vap_id = hmac_vap->vap_id;

    if (data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET] != ftm_init[session_id_num].dialog_token) {
        oam_warning_log3(0, OAM_SF_FTM, "vap_id[%d] hmac_ftm_initor_rx_ftm:error,follow up[%d] is previous token[%d]",
            hmac_vap->vap_id, data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET],
            ftm_init[session_id_num].dialog_token);
    }

    ftm_init[session_id_num].dialog_token = data[FTM_FRAME_DIALOG_TOKEN_OFFSET];
    ftm_init[session_id_num].follow_up_dialog_token = data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET];
    ftm_init[session_id_num].time_syn_count++;

    /* 在FTM信道收到了正确的FTM帧: */
        /* 如果是NON_ASAP的情况则:1.收到的是FTM_1，则需要打断FTM扫描，切回工作信道等待回合开始 */
        /*                        2.收到的是FTM_K(K>1) */
    if ((ftm_init[session_id_num].asap == OSAL_FALSE) && (data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET] == 0)) {
        /* 收到的是FTM_1 */
        (osal_void)memset_s(&(ftm_init[session_id_num].ftm_timer), sizeof(ftm_init[session_id_num].ftm_timer),
            0, sizeof(ftm_init[session_id_num].ftm_timer));

        /* 初始化保存ftm测量结构体 */
        if (hmac_ftm_initor_rx_ftm1(hmac_vap, data, frame_len, session_id_num) == OAL_SUCC) {
            ftm_init[session_id_num].range = 0;
            hmac_ftm_initor_wait_start_burst(hmac_vap, session_id_num);
        }
    }
    hmac_ftm_initor_rx_ftmk(hmac_vap, netbuf, session_id_num);

    return OAL_SUCC;
}

osal_u32 hmac_ftm_rspder_wait_send_ftm_timeout(osal_void *arg)
{
    osal_u8                              session_id;
    osal_u32                             timeout;
    hmac_vap_stru                       *hmac_vap = OSAL_NULL;
    ftm_timeout_arg_stru                *arg_temp = OSAL_NULL;
    hmac_ftm_stru                       *ftm = OSAL_NULL;
    hmac_ftm_responder_stru             *ftm_rspder = OSAL_NULL;

    if (arg == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_wait_send_ftm_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    arg_temp = (ftm_timeout_arg_stru *)arg;
    hmac_vap = (hmac_vap_stru *)arg_temp->hmac_vap;
    session_id = arg_temp->session_id;
    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_rspder = ftm->ftm_rspder;

    if (ftm_rspder[session_id].ftms_per_burst_varied > 0) {
        oam_warning_log3(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_rspder_wait_send_ftm_timeout::send ftm,ftms_per_burst_varied[%d],session_id[%d].}",
            hmac_vap->vap_id, ftm_rspder[session_id].ftms_per_burst_varied, session_id);
        hmac_ftm_rspder_set_parameter(hmac_vap, session_id);
        hmac_ftm_rspder_send_ftm(hmac_vap, session_id);
        ftm_rspder[session_id].ftms_per_burst_varied = ftm_rspder[session_id].ftms_per_burst_varied - 1;
    } else {
        return OAL_SUCC;
    }
    if (ftm_rspder[session_id].ftm_tsf_timer.is_registerd == OSAL_FALSE) {
        timeout = ftm_rspder[session_id].min_delta_ftm / 10; /* FTM帧之间的间隙级别是100毫秒级 */
        oam_warning_log2(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_rspder_wait_send_ftm_timeout::ftm wait send iftmr, time is [%d].}",
            hmac_vap->vap_id, timeout);

        frw_create_timer_entry(&(ftm_rspder[session_id].ftm_tsf_timer), hmac_ftm_rspder_wait_send_ftm_timeout,
            timeout, (osal_void *)arg_temp, OSAL_FALSE);
    }

    return OAL_SUCC;
}

osal_void hmac_ftm_initor_send_remain(hmac_vap_stru *hmac_vap, const mac_send_iftmr_stru *send_iftmr,
    hmac_ftm_initiator_stru *ftm_init, osal_u8 session_id_number)
{
    osal_u32 ret;

    if ((hmac_vap == OSAL_NULL) || (ftm_init == OSAL_NULL) || (send_iftmr == OSAL_NULL)) {
        return;
    }

    ftm_init[session_id_number].lci_ie = OSAL_FALSE;
    ftm_init[session_id_number].location_civic_ie = OSAL_FALSE;
    ftm_init[session_id_number].timer_syn = send_iftmr->measure_req;
    ftm_init[session_id_number].clock_calibrated = OSAL_FALSE;
    ftm_init[session_id_number].ftm_tsf_timer.is_registerd = OSAL_FALSE;

    ftm_init[session_id_number].burst_cnt = osal_bit(send_iftmr->burst_num);
    ftm_init[session_id_number].ftms_per_burst_cmd = send_iftmr->ftms_per_burst;
    ftm_init[session_id_number].en_asap = 0;
    ftm_init[session_id_number].asap = send_iftmr->asap & 0x1;
    ftm_init[session_id_number].min_delta_ftm = FTM_MIN_DELTA;

    if (ftm_init[session_id_number].ftms_per_burst_cmd > FTM_PER_BURST) {
        ftm_init[session_id_number].ftms_per_burst = ftm_init[session_id_number].ftms_per_burst_cmd;
    } else {
        ftm_init[session_id_number].ftms_per_burst = FTM_PER_BURST;
    }
    // 扩大10倍，周期变为秒级
    ftm_init[session_id_number].burst_period = FTM_BURST_PERIOD  * 10;

    ftm_init[session_id_number].range = 0;
    ftm_init[session_id_number].iftmr = OSAL_TRUE;
    /* FTM请求参数调整 */
    ret = hmac_send_ftm_req_adjust_param(hmac_vap, ftm_init, session_id_number);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_ftm_initor_send_remain:ftm_req_adjust_param_cb return [%d]", ret);
        return;
    }

    hal_set_ftm_new_status(OSAL_FALSE);
    // 同步开启，使能同步特性寄存器，复位phy时钟步长
    if (ftm_init[session_id_number].timer_syn == OSAL_TRUE) {
        hal_set_ftm_new_status(OSAL_TRUE);
    }
    // 发送第一次请求前进行环回校准
    hmac_ftm_start_cali(hmac_vap, ftm_init[session_id_number].prot_format, ftm_init[session_id_number].band_cap);

    if (ftm_init[session_id_number].asap == OSAL_TRUE) {
        hmac_ftm_initor_start_bust(hmac_vap, session_id_number);
    } else {
        hmac_ftm_initor_scan_for_ftm(hmac_vap, session_id_number);
    }

    hmac_ftm_initor_wait_end_ftm(hmac_vap, session_id_number);

    return;
}

osal_u32 hmac_ftm_initor_send(hmac_vap_stru *hmac_vap, mac_send_iftmr_stru *send_iftmr)
{
    osal_s8                  session_id;
    osal_u8                  session_id_number, band_cap, prot_format, i;
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;

    ftm_init = ftm->ftm_init;

    /* 如果命令没有指定BSSID，取关联ap的BSSID */
    if (hmac_addr_is_zero_etc(send_iftmr->bssid)) {
        (osal_void)memcpy_s(send_iftmr->bssid, OAL_MAC_ADDR_LEN, hmac_vap->bssid, OAL_MAC_ADDR_LEN);
    }

    /* 查找Session ID */
    session_id = hmac_ftm_find_session_index(hmac_vap, MAC_FTM_INITIATOR_MODE, send_iftmr->bssid);
    if (session_id < 0) {
        return OAL_FAIL;
    }

    session_id_number = (osal_u8)session_id;
    hmac_ftm_enable_session_index(hmac_vap, MAC_FTM_INITIATOR_MODE, send_iftmr->bssid, session_id_number);

    for (i = 0; i < ftm->session_num; i++) {
        if ((ftm_init[i].ftm_all_burst.is_registerd == OSAL_TRUE) ||
            (ftm->ftm_rspder[i].ftm_all_burst.is_registerd == OSAL_TRUE)) {
            oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_initor_send::ftm in running.}");
            return OAL_FAIL;
        }
    }

    if (send_iftmr->channel_num == 0) {
        ftm_init[session_id_number].channel_ftm.chan_number = hmac_vap->channel.chan_number;
        ftm_init[session_id_number].channel_ftm.band = hmac_vap->channel.band;
       /* 2.4G 信道1 ~ 14 */
    } else if ((send_iftmr->channel_num <= 14) && (send_iftmr->channel_num >= 1)) {
        ftm_init[session_id_number].channel_ftm.chan_number = send_iftmr->channel_num;
        ftm_init[session_id_number].channel_ftm.band = WLAN_BAND_2G;
    } else {
        ftm_init[session_id_number].channel_ftm.chan_number = send_iftmr->channel_num;
        ftm_init[session_id_number].channel_ftm.band = WLAN_BAND_5G;
    }
    if (ftm->auto_bandwidth == OSAL_TRUE) {
        /* 由命令控制 */
        ftm_init[session_id_number].channel_ftm.en_bandwidth = hmac_vap->channel.en_bandwidth;
        ftm_init[session_id_number].prot_format = hmac_ftm_get_phy_mode(hmac_vap->protocol, hmac_vap->channel.band);
        ftm_init[session_id_number].band_cap = hmac_ftm_get_band_cap(hmac_vap->channel.en_bandwidth);
    } else if (ftm->auto_bandwidth == OSAL_FALSE) {
        /* 使用11g/a 发帧 */
        ftm_init[session_id_number].channel_ftm.en_bandwidth = WLAN_BAND_WIDTH_20M;
        ftm_init[session_id_number].band_cap = WLAN_BW_CAP_20M;
        ftm_init[session_id_number].prot_format = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    } else {
        /* 依据协议内容定义协议模式与带宽,不关心扩频方向 */
        hmac_ftm_get_format_and_bandwidth(&band_cap, &prot_format, ftm->auto_bandwidth);
        ftm_init[session_id_number].band_cap = band_cap;
        ftm_init[session_id_number].prot_format = prot_format;
    }

    hmac_get_channel_idx_from_num_etc(ftm_init[session_id_number].channel_ftm.band,
        ftm_init[session_id_number].channel_ftm.chan_number, &ftm_init[session_id_number].channel_ftm.chan_idx);

    hmac_ftm_initor_send_remain(hmac_vap, send_iftmr, ftm_init, session_id_number);

    return OAL_SUCC;
}

wlan_bw_cap_enum_uint8 hmac_ftm_get_band_cap(wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    wlan_bw_cap_enum_uint8 band_cap;

    switch (en_bandwidth) {
        case WLAN_BAND_WIDTH_20M:
            band_cap = WLAN_BW_CAP_20M;
            break;
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_40MINUS:
            band_cap = WLAN_BW_CAP_40M;
            break;
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            band_cap = WLAN_BW_CAP_80M;
            break;
        default:
            band_cap = WLAN_BW_CAP_20M;
            break;
    }

    return band_cap;
}

osal_u32 hmac_ftm_rspder_send(hmac_vap_stru *hmac_vap, const osal_u8 mac[WLAN_MAC_ADDR_LEN],
    wlan_phy_protocol_enum prot_format, wlan_bw_cap_enum band_cap)
{
    osal_s8                      session_id;
    osal_u8                      session_id_number;
    oal_bool_enum_uint8          ftm_status;
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_responder_stru     *ftm_rspder = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_rspder = ftm->ftm_rspder;

    /* 查找session id */
    session_id = hmac_ftm_find_session_index(hmac_vap, MAC_FTM_RESPONDER_MODE, mac);
    if (session_id < 0) {
        return OAL_FAIL;
    }

    session_id_number = (osal_u8)session_id;
    ftm_status = ftm_rspder[session_id_number].ftm_responder;
    hmac_ftm_enable_session_index(hmac_vap, MAC_FTM_RESPONDER_MODE, mac, session_id_number);
    /* 2表示dialog_token赋值为2 */
    ftm_rspder[session_id_number].dialog_token = 2;
    ftm_rspder[session_id_number].follow_up_dialog_token = 1;

    /* non_asap */
    ftm_rspder[session_id_number].asap = OSAL_FALSE;
    /* 不携带 ie */
    ftm_rspder[session_id_number].lci_ie = OSAL_FALSE;
    ftm_rspder[session_id_number].location_civic_ie = OSAL_FALSE;

    ftm_rspder[session_id_number].ftm_parameters = OSAL_TRUE;
    ftm_rspder[session_id_number].ftm_synchronization_information = OSAL_TRUE;

    ftm_rspder[session_id_number].tod = 0;
    ftm_rspder[session_id_number].toa = 0;

    ftm_rspder[session_id_number].band_cap = band_cap;
    ftm_rspder[session_id_number].prot_format = prot_format;

    /* 获取TSF */
    hal_vap_tsf_get_32bit(hmac_vap->hal_vap, &ftm_rspder[session_id_number].tsf);

    if (hmac_ftm_rspder_send_ftm(hmac_vap, session_id_number) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{hmac_ftm_rspder_send:: hmac_ftm_rspder_send_ftm err!}");
    }
    /* 恢复session状态 */
    ftm_rspder[session_id_number].ftm_responder = ftm_status;

    return OAL_SUCC;
}

wlan_phy_protocol_enum_uint8 hmac_ftm_get_phy_mode(wlan_protocol_enum_uint8 protocol,
    wlan_channel_band_enum_uint8 band)
{
    wlan_phy_protocol_enum_uint8 phy_protocol_format;
    switch (protocol) {
        case WLAN_LEGACY_11B_MODE:
            phy_protocol_format = WLAN_11B_PHY_PROTOCOL_MODE;
            break;
        case WLAN_LEGACY_11G_MODE:
        case WLAN_LEGACY_11A_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            phy_protocol_format = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            break;
        case WLAN_HT_MODE:
        case WLAN_HT_11G_MODE:
        case WLAN_HT_ONLY_MODE:
            phy_protocol_format = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            phy_protocol_format = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case WLAN_HE_MODE:
            if (band == WLAN_BAND_5G) {
                phy_protocol_format = WLAN_VHT_PHY_PROTOCOL_MODE;
            } else {
                phy_protocol_format = WLAN_HT_PHY_PROTOCOL_MODE;
            }
            break;
        default:
            phy_protocol_format = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    }
    return phy_protocol_format;
}

osal_u32 hmac_ftm_rspder_set_parameter(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u8                  index;
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_responder_stru *ftm_rspder = OSAL_NULL;
    ftm_timer_stru          *ftm_timer = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    ftm_rspder = ftm->ftm_rspder;
    ftm_timer = ftm_rspder[session_id].ftm_timer;

    ftm_rspder[session_id].follow_up_dialog_token = ftm_rspder[session_id].dialog_token;
    ftm_rspder[session_id].dialog_token++;

    index = ftm_rspder[session_id].follow_up_dialog_token % MAC_FTM_TIMER_CNT;

    /* 检测对应的索引的uc_dialog_token是否一致，防止已被覆盖 */
    if (ftm_timer[index].dialog_token != ftm_rspder[session_id].follow_up_dialog_token) {
        oam_warning_log3(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_rspder_set_parameter:: token[%d] != resp_follow_updialog_token[%d]!.}",
            hmac_vap->vap_id, ftm_timer[index].dialog_token,
            ftm_rspder[session_id].follow_up_dialog_token);
        ftm_rspder[session_id].tod = 0;
        ftm_rspder[session_id].toa = 0;
    } else {
        /* 根据每个回合帧数发送FTM */
        ftm_rspder[session_id].tod = ftm_timer[index].t1;
        ftm_rspder[session_id].toa = ftm_timer[index].t4;
    }
    /* 防止:t1 t4没有刷新，下一帧重用t1 t4 */
    ftm_timer[index].t1 = 0;
    ftm_timer[index].t4 = 0;

    /* 不携带 ie */
    ftm_rspder[session_id].lci_ie = OSAL_FALSE;
    ftm_rspder[session_id].location_civic_ie = OSAL_FALSE;
    ftm_rspder[session_id].ftm_parameters = OSAL_FALSE;
    ftm_rspder[session_id].ftm_synchronization_information = OSAL_FALSE;

    return OAL_SUCC;
}

osal_void hmac_ftm_rspder_wait_send_ftm(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_responder_stru *ftm_rspder = OSAL_NULL;
    ftm_timeout_arg_stru    *arg = OSAL_NULL;
    osal_u32                 timeout;

    if (ftm == OSAL_NULL) {
        return;
    }
    ftm_rspder = ftm->ftm_rspder;
    arg = &ftm_rspder[session_id].arg;

    if (ftm_rspder[session_id].ftms_per_burst_varied > 0) {
        oam_warning_log3(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_rspder_wait_send_ftm::ftm rsp send ftm, ftms_per_burst_varied[%d],session_id [%d].}",
            hmac_vap->vap_id, ftm_rspder[session_id].ftms_per_burst_varied, session_id);
        hmac_ftm_rspder_set_parameter(hmac_vap, session_id);
        hmac_ftm_rspder_send_ftm(hmac_vap, session_id);
        ftm_rspder[session_id].ftms_per_burst_varied = ftm_rspder[session_id].ftms_per_burst_varied - 1;
    } else {
        return;
    }

    arg->hmac_vap = (osal_void *)hmac_vap;
    arg->session_id = session_id;
    if (ftm_rspder[session_id].ftm_tsf_timer.is_registerd == OSAL_FALSE) {
        timeout = ftm_rspder[session_id].min_delta_ftm / 10; /* FTM帧之间的间隙级别是100毫秒级 */
        oam_warning_log2(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_rspder_wait_send_ftm::ftm wait send iftmr, time is [%d].}",
            hmac_vap->vap_id, timeout);

        frw_create_timer_entry(&(ftm_rspder[session_id].ftm_tsf_timer), hmac_ftm_rspder_wait_send_ftm_timeout,
            timeout, (osal_void *)arg, OSAL_FALSE);
    }
}

osal_u32 hmac_ftm_rspder_wait_end_ftm_timeout(osal_void *arg)
{
    osal_u8                      session_id;
    hmac_vap_stru               *hmac_vap = OSAL_NULL;
    ftm_timeout_arg_stru        *arg_temp = OSAL_NULL;
    hmac_ftm_stru               *ftm = OSAL_NULL;
    hmac_ftm_responder_stru     *ftm_rspder = OSAL_NULL;

    if (arg == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_wait_end_ftm_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    arg_temp = (ftm_timeout_arg_stru *)arg;
    hmac_vap = (hmac_vap_stru *)(arg_temp->hmac_vap);
    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_rspder = ftm->ftm_rspder;
    session_id = arg_temp->session_id;

    oam_warning_log2(0, OAM_SF_FTM, "vap_id[%d] {hmac_ftm_rspder_wait_end_ftm_timeout::end ftm, session_id[%d].}",
        hmac_vap->vap_id, session_id);

    frw_destroy_timer_entry(&(ftm_rspder[session_id].ftm_all_burst));

    /* 结束ftm */
    hmac_ftm_rspder_end_session(&(ftm_rspder[session_id]));

    return OAL_SUCC;
}

osal_void hmac_ftm_rspder_wait_end_ftm(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u32                     timeout;
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_responder_stru     *ftm_rspder = OSAL_NULL;
    ftm_timeout_arg_stru        *arg = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return;
    }

    ftm_rspder = ftm->ftm_rspder;
    arg = &ftm_rspder[session_id].arg;

    // 依据回合数来定制会话最大时间，并预留1个回合的时间
    timeout = FTM_MAX_SESSION_TIMEOUT * (ftm_rspder[session_id].burst_cnt + 1);
    oam_warning_log2(0, OAM_SF_FTM,
        "{hmac_ftm_rspder_wait_end_ftm::timeout[%d], session_id[%d].}", timeout, session_id);

    arg->hmac_vap = (osal_void *)hmac_vap;
    arg->session_id = session_id;

    frw_create_timer_entry(&(ftm_rspder[session_id].ftm_all_burst), hmac_ftm_rspder_wait_end_ftm_timeout,
        timeout, (osal_void *)arg, OSAL_FALSE);
}

osal_u32 hmac_ftm_initor_wait_end_ftm_timeout(osal_void *arg)
{
    osal_u8                              session_id;
    hmac_vap_stru                       *hmac_vap = OSAL_NULL;
    ftm_timeout_arg_stru                *ftm_timeout_arg = OSAL_NULL;
    hmac_ftm_stru                       *ftm = OSAL_NULL;
    hmac_ftm_initiator_stru             *ftm_init = OSAL_NULL;

    if (arg == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_ftm_initor_wait_end_ftm_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_timeout_arg = (ftm_timeout_arg_stru *)arg;
    hmac_vap = (hmac_vap_stru *)(ftm_timeout_arg->hmac_vap);
    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    if (ftm == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ftm_init = ftm->ftm_init;
    session_id = ftm_timeout_arg->session_id;

    oam_warning_log2(0, OAM_SF_FTM,
        "vap_id[%d] {hmac_ftm_initor_wait_end_ftm_timeout::end ftm, session_id[%d].}",
        hmac_vap->vap_id, session_id);

    frw_destroy_timer_entry(&(ftm_init[session_id].ftm_all_burst));

    /* 结束ftm */
    hmac_ftm_initor_end_session(&(ftm_init[session_id]));

    return OAL_SUCC;
}

osal_void hmac_ftm_initor_wait_end_ftm(hmac_vap_stru *hmac_vap, osal_u8 session_id)
{
    osal_u32                     timeout;
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru     *ftm_init = OSAL_NULL;
    ftm_timeout_arg_stru        *arg = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return;
    }
    ftm_init = ftm->ftm_init;
    arg = &ftm_init[session_id].arg;

    // 依据回合数来定制会话最大时间，并预留1个回合的时间
    timeout = FTM_MAX_SESSION_TIMEOUT * (ftm_init[session_id].burst_cnt + 1 + ftm_init[session_id].asap);
    oam_warning_log2(0, OAM_SF_FTM,
        "{hmac_ftm_initor_wait_end_ftm::timeout[%d], session_id[%d].}", timeout, session_id);

    arg->hmac_vap = (osal_void *)hmac_vap;
    arg->session_id = session_id;
    frw_create_timer_entry(&(ftm_init[session_id].ftm_all_burst), hmac_ftm_initor_wait_end_ftm_timeout, timeout,
        arg, OSAL_FALSE);
}

osal_void hmac_ftm_rspder_filled(hmac_vap_stru *hmac_vap,
    hmac_ftm_responder_stru *ftm_rspder, mac_ftm_parameters_ie_stru *mac_ftmp, osal_u8 session_id)
{
    osal_u8 band_cap, prot_format;

    if ((hmac_vap == OSAL_NULL) || (ftm_rspder == OSAL_NULL)) {
        return;
    }

    ftm_rspder->received_iftmr = OSAL_TRUE;
    ftm_rspder->dialog_token = 0;
    ftm_rspder->follow_up_dialog_token = 0;

    /* 回合个数(2^number_of_bursts_exponent) */
    /* 15为最大限制 */
    if (mac_ftmp->number_of_bursts_exponent >= 15) {
        mac_ftmp->number_of_bursts_exponent = 0;
    }
    ftm_rspder->burst_cnt = osal_bit(mac_ftmp->number_of_bursts_exponent);
    /* 每个回合FTM帧的个数 */
    if (mac_ftmp->ftms_per_burst > FTM_PER_BURST) {
        ftm_rspder->ftms_per_burst = mac_ftmp->ftms_per_burst;
    } else {
        ftm_rspder->ftms_per_burst = FTM_PER_BURST;
    }
    /* FTM帧的间隔时间 */
    if (mac_ftmp->min_delta_ftm) {
        ftm_rspder->min_delta_ftm = mac_ftmp->min_delta_ftm;
    } else {
        ftm_rspder->min_delta_ftm = FTM_MIN_DELTA;
    }

    /* FTM 一个会话中ftm有效交互时间 */
    hmac_get_ftm_burst_duration(hmac_vap, MAC_FTM_RESPONDER_MODE, mac_ftmp, session_id);

    /* 回合间隔 */
    if (mac_ftmp->burst_period) {
        ftm_rspder->burst_period = mac_ftmp->burst_period;
    } else {
        /* 1000为时间进制 */
        ftm_rspder->burst_period = (ftm_rspder->ftms_per_burst * ftm_rspder->min_delta_ftm / 1000) + 1;
    }

    /* asap */
    ftm_rspder->asap = mac_ftmp->asap;

    hmac_ftm_get_format_and_bandwidth(&band_cap, &prot_format, mac_ftmp->format_and_bandwidth);
    ftm_rspder->band_cap = band_cap;
    ftm_rspder->prot_format = prot_format;

    // 收到第一次请求后进行环回校准再进行FTM交互
    hmac_ftm_start_cali(hmac_vap, ftm_rspder->prot_format, ftm_rspder->band_cap);
    oam_warning_log4(0, OAM_SF_FTM,
        "{hmac_ftm_rspder_filled:: burst_cnt[%d], per_burst_save[%d], delta_ftm[%d]0.1ms,period[%d]100ms.}",
        ftm_rspder->burst_cnt, ftm_rspder->ftms_per_burst, ftm_rspder->min_delta_ftm, ftm_rspder->burst_period);

    hmac_ftm_rspder_wait_end_ftm(hmac_vap, session_id);

    oam_warning_log1(0, OAM_SF_FTM, "{hmac_ftm_rspder_filled::asap=[%d]}", ftm_rspder->asap);

    if (ftm_rspder->asap == OSAL_FALSE) {
        /* 获取TSF */
        hal_vap_tsf_get_32bit(hmac_vap->hal_vap, &(ftm_rspder->tsf));
        hmac_ftm_rspder_set_parameter(hmac_vap, session_id);
        ftm_rspder->tod = 0;
        ftm_rspder->toa = 0;
        ftm_rspder->ftm_parameters = OSAL_TRUE;

        hmac_ftm_rspder_send_ftm(hmac_vap, session_id);
    } else {
        ftm_rspder->ftms_per_burst_varied = ftm_rspder->ftms_per_burst;
        hmac_ftm_rspder_wait_send_ftm(hmac_vap, session_id);
    }
}

/*****************************************************************************
 函 数 名  : hmac_ftm_rsp_rx_ftm_req
 功能描述  : ftm rsp收到ftm request帧处理
*****************************************************************************/
osal_void hmac_ftm_rspder_rx_ftm_req(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    osal_u8                      mac_ra[WLAN_MAC_ADDR_LEN] = {0};
    hmac_ftm_stru               *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_responder_stru     *ftm_rspder = OSAL_NULL;
    osal_u8 i;
    const dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    const mac_rx_ctl_stru *rx_info = OSAL_NULL;
    const mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 *payload = OSAL_NULL;
    osal_u16 frame_len;
    osal_s8 session_id;
    osal_u8 session_id_index;
    osal_u8 *data = OSAL_NULL;
    mac_ftm_parameters_ie_stru *mac_ftmp = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return;
    }
    /* ftm responder */
    if (mac_mib_get_fine_timing_msmt_resp_activated(hmac_vap) == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_rx_ftm_req::not a ftm_responder, ignored.}");
        return;
    }

    ftm_rspder = ftm->ftm_rspder;
    for (i = 0; i < ftm->session_num; i++) {
        if (ftm->ftm_init[i].ftm_all_burst.is_registerd == OSAL_TRUE) {
            oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_rx_ftm_req::ftm in running.}");
            return;
        }
    }

    rx_ctl = (const dmac_rx_ctl_stru *)oal_netbuf_cb_const(netbuf);
    rx_info = (const mac_rx_ctl_stru *)(&(rx_ctl->rx_info));

    /* 获取帧头信息 */
    frame_hdr = (const mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);

    /* 获取帧体指针 */
    payload = oal_netbuf_rx_data(netbuf);
    frame_len = mac_get_rx_cb_payload_len(rx_info);  /* 帧体长度 */

    (osal_void)memcpy_s(mac_ra, WLAN_MAC_ADDR_LEN, (osal_u8 *)frame_hdr + 10, WLAN_MAC_ADDR_LEN); /* 10偏移 */

    /* 查找session id */
    session_id = hmac_ftm_find_session_index(hmac_vap, MAC_FTM_RESPONDER_MODE, (const osal_u8 *)mac_ra);
    if (session_id < 0) {
        return;
    }
    session_id_index = (osal_u8)session_id;
    hmac_ftm_enable_session_index(hmac_vap, MAC_FTM_RESPONDER_MODE, (const osal_u8 *)mac_ra, session_id_index);
    if (payload[FTM_REQ_TRIGGER_OFFSET] != OSAL_TRUE) {
        /* 结束ftm */
        hmac_ftm_rspder_end_session(&(ftm_rspder[session_id_index]));
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_rx_ftm_req::end of send ftm!.}");
        return;
    }
    /* FTM Trigger */
    if (ftm_rspder[session_id_index].received_iftmr == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_rx_ftm_req:: rx FTM Trigger.}");
        ftm_rspder[session_id_index].ftms_per_burst_varied = ftm_rspder[session_id_index].ftms_per_burst;
        hmac_ftm_rspder_wait_send_ftm(hmac_vap, session_id_index);
        return;
    }

    /* iFTMR */
    data = payload;
    mac_ftmp = (mac_ftm_parameters_ie_stru *)mac_find_ie_etc(MAC_EID_FTMP,
        &data[FTM_FRAME_IE_OFFSET], (osal_s32)(frame_len - FTM_FRAME_IE_OFFSET));
    if (mac_ftmp == OSAL_NULL) {
        /* 结束ftm */
        hmac_ftm_rspder_end_session(&(ftm_rspder[session_id_index]));
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_rx_ftm_req::ftmp ie is NULL!.}");
        return;
    }

    oam_warning_log0(0, OAM_SF_FTM, "{hmac_ftm_rspder_rx_ftm_req:: rx iFTMr.}");
    hmac_ftm_rspder_filled(hmac_vap, &(ftm_rspder[session_id_index]), mac_ftmp, session_id_index);
}

osal_void hmac_ftm_start_cali(hmac_vap_stru *hmac_vap, wlan_phy_protocol_enum prot_format,
    wlan_bw_cap_enum band_cap)
{
    osal_u32 time_sync_enable = 0;
    osal_u32 phy_freq;
    osal_u32 intp_freq;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    if (ftm == OSAL_NULL) {
        return;
    }

    // 读取同步特性开关
    hal_get_ftm_new_status(&time_sync_enable);

    if (time_sync_enable != OSAL_TRUE) {
        // 进行环回校准后再开始正常的FTM交互
        ftm->cali = OSAL_TRUE;
        hmac_ftm_rspder_send(hmac_vap, mac_mib_get_station_id(hmac_vap), prot_format, band_cap);
        // 读取时钟频率(1: 160M 2: 320M 3: 640M)
        hal_get_ftm_phy_bw_mode(&phy_freq);

        // 读取AD采样频率(0: 80M 1: 160M 2: 320M 3: 640M)
        hal_get_ftm_intp_freq(&intp_freq);

        ftm->cali_phy_freq = phy_freq;
        ftm->cali_adc_freq = intp_freq;
    }
    ftm->cali = OSAL_FALSE;
}

osal_void hmac_tx_set_ftm_ctrl_dscr(hmac_vap_stru *hmac_vap, hal_tx_dscr_stru *tx_dscr,
    const oal_netbuf_stru *netbuf)
{
    hal_to_dmac_device_stru *hal_device;
    hmac_ftm_responder_stru *ftm_rspder;
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;

    if ((ftm == OSAL_NULL) || (hmac_vap->hal_device == OSAL_NULL)) {
        return;
    }
    hal_device = hmac_vap->hal_device;
    ftm_rspder = ftm->ftm_rspder;
    ftm_init = ftm->ftm_init;

    /* 获取帧头信息 */
    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    /* 获取帧体指针 */
    data = oal_netbuf_tx_data(netbuf);
    if ((frame_hdr->frame_control.sub_type != WLAN_ACTION) ||
        ((data[MAC_ACTION_OFFSET_CATEGORY]) != MAC_ACTION_CATEGORY_PUBLIC) ||
        (((data[MAC_ACTION_OFFSET_ACTION]) != MAC_PUB_FTM_REQ) && ((data[MAC_ACTION_OFFSET_ACTION]) != MAC_PUB_FTM))) {
        // 只需处理FTM和FTM req帧
        return;
    }

    tx_ctl->ftm_cali_en = ftm->cali;

    /* init 设置iFTMR帧 tx环回描述符 */
    if ((data[MAC_ACTION_OFFSET_ACTION]) == MAC_PUB_FTM_REQ) {
        osal_s8 session_id = hmac_ftm_find_session_index(hmac_vap, MAC_FTM_INITIATOR_MODE, frame_hdr->address1);
        if (session_id < 0) {
            oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_tx_set_ftm_ctrl_dscr:session_id err1!", hmac_vap->vap_id);
            return;
        }
        osal_u8 session_id_number = (osal_u8)session_id;
        if (oal_compare_mac_addr(ftm_init[session_id_number].bssid, frame_hdr->address1) != 0) {
            oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_tx_set_ftm_ctrl_dscr:mac err1!", hmac_vap->vap_id);
            return;
        }
        // 设置发送描述符中的tx_desc_freq_bandwidth_mode
        tx_ctl->is_ftm = 0x1;
        tx_ctl->ftm_bandwidth = ftm_init[session_id_number].band_cap;
        tx_ctl->ftm_protocol_mode = ftm_init[session_id_number].prot_format;
    } else if ((data[MAC_ACTION_OFFSET_ACTION]) == MAC_PUB_FTM) { /* rsp 设置FTM帧 硬件重传次数 */
        hal_set_ftm_tx_cnt(tx_dscr, 1);

        osal_s8 session_id = hmac_ftm_find_session_index(hmac_vap, MAC_FTM_RESPONDER_MODE, frame_hdr->address1);
        if (session_id < 0) {
            oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_tx_set_ftm_ctrl_dscr:session_id err2!", hmac_vap->vap_id);
            return;
        }
        osal_u8 session_id_number = (osal_u8)session_id;
        if (oal_compare_mac_addr(ftm_rspder[session_id_number].mac_ra, frame_hdr->address1) != 0) {
            oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_tx_set_ftm_ctrl_dscr:mac err2!", hmac_vap->vap_id);
            return;
        }

        // 设置发送描述符中的tx_desc_freq_bandwidth_mode
        tx_ctl->is_ftm = 0x2;
        tx_ctl->ftm_bandwidth = ftm_rspder[session_id_number].band_cap;
        tx_ctl->ftm_protocol_mode = ftm_rspder[session_id_number].prot_format;
    }
    /* 设置FTM帧 通路 */
    tx_ctl->ftm_chain = ftm->tx_chain_selection;
}

/*****************************************************************************
 函 数 名  : hmac_vap_ftm_init
 功能描述  : 初始化vap下的ftm结构体
*****************************************************************************/
osal_u32 hmac_vap_ftm_init(hmac_vap_stru *hmac_vap, osal_u8 session_num)
{
    hmac_ftm_stru *ftm = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_vap_ftm_init: hmac_vap null!");
        return OSAL_FALSE;
    }
    if ((session_num <= 0) || (session_num > MAX_FTM_SESSION)) {
        session_num = DEFAULT_FTM_SESSION;
    }

    ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    if (ftm == OSAL_NULL) {
        osal_void *mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_ftm_stru), OAL_TRUE);
        if (mem_ptr == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_vap_ftm_init ftm malloc null!", hmac_vap->vap_id);
            return OSAL_FALSE;
        }

        (osal_void)memset_s(mem_ptr, sizeof(hmac_ftm_stru), 0, sizeof(hmac_ftm_stru));
        hmac_vap_feature_registered(hmac_vap, WLAN_FEATURE_INDEX_FTM, mem_ptr);

        ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
        ftm->ftm_init = (hmac_ftm_initiator_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            sizeof(hmac_ftm_initiator_stru) * session_num, OAL_TRUE);
        ftm->ftm_rspder = (hmac_ftm_responder_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            sizeof(hmac_ftm_responder_stru) * session_num, OAL_TRUE);
        if ((ftm->ftm_init == OSAL_NULL) || (ftm->ftm_rspder == OSAL_NULL)) {
            oam_error_log1(0, OAM_SF_FTM, "vap_id[%d] hmac_vap_ftm_init session malloc null!", hmac_vap->vap_id);
            hmac_vap_ftm_deinit(hmac_vap);
            return OSAL_FALSE;
        }

        (osal_void)memset_s(ftm->ftm_init, sizeof(hmac_ftm_initiator_stru) * session_num, 0,
            sizeof(hmac_ftm_initiator_stru) * session_num);
        (osal_void)memset_s(ftm->ftm_rspder, sizeof(hmac_ftm_responder_stru) * session_num, 0,
            sizeof(hmac_ftm_responder_stru) * session_num);
        ftm->session_num = session_num;
        oam_warning_log1(0, OAM_SF_FTM, "{hmac_vap_ftm malloc succ, session num [%d]!}", session_num);
    } else {
        oam_warning_log0(0, OAM_SF_FTM, "{hmac_vap_ftm already init, do not Re-enable!}");
    }

    oam_warning_log0(0, OAM_SF_FTM, "{hmac_vap_ftm_init!}");
    // FTM默认采用单天线通道0发送
    ftm->tx_chain_selection = WLAN_PHY_CHAIN_SEL_TYPE_0;
    /* FTM默认使用协商协议和带宽发帧 */
    ftm->auto_bandwidth = OSAL_TRUE;
    return OSAL_TRUE; /* hmac_vap->ftm本身非空;或者dmac_vap->ftm内存申请成功,不释放 */
}

/*****************************************************************************
 函 数 名  : hmac_vap_ftm_deinit
 功能描述  : 释放vap下的ftm结构体内存
*****************************************************************************/
osal_void hmac_vap_ftm_deinit(hmac_vap_stru *hmac_vap)
{
    osal_u8 session_num;
    hmac_ftm_initiator_stru *ftm_init = OAL_PTR_NULL;
    hmac_ftm_responder_stru *ftm_rspder = OAL_PTR_NULL;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    if (ftm == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_FTM, "hmac_vap_ftm_deinit: ftm is null!");
        return;
    }

    session_num = ftm->session_num;
    ftm_init = ftm->ftm_init;
    ftm_rspder = ftm->ftm_rspder;
    /* 防止释放过程中读变量 */
    ftm->ftm_init = OAL_PTR_NULL;
    ftm->ftm_rspder = OAL_PTR_NULL;
    /* 发起者删除tsf定时器、全局定时器、释放内存 */
    if (ftm_init != OAL_PTR_NULL) {
        for (osal_u8 index = 0; index < session_num; index++) {
            if (ftm_init[index].ftm_all_burst.is_registerd == OSAL_TRUE) {
                frw_destroy_timer_entry(&(ftm_init[index].ftm_all_burst));
            }
            if (ftm_init[index].ftm_tsf_timer.is_registerd == OSAL_TRUE) {
                frw_destroy_timer_entry(&(ftm_init[index].ftm_tsf_timer));
            }
        }
        oal_mem_free(ftm_init, OAL_TRUE);
        ftm_init = OAL_PTR_NULL;
    }
    /* 响应者删除tsf定时器、全局定时器、释放内存 */
    if (ftm_rspder != OAL_PTR_NULL) {
        osal_u8 index;
        for (index = 0; index < session_num; index++) {
            if (ftm_rspder[index].ftm_all_burst.is_registerd == OSAL_TRUE) {
                frw_destroy_timer_entry(&(ftm_rspder[index].ftm_all_burst));
            }
            if (ftm_rspder[index].ftm_tsf_timer.is_registerd == OSAL_TRUE) {
                frw_destroy_timer_entry(&(ftm_rspder[index].ftm_tsf_timer));
            }
        }
        oal_mem_free(ftm_rspder, OAL_TRUE);
        ftm_rspder = OAL_PTR_NULL;
    }
    oal_mem_free(ftm, OAL_TRUE);
    ftm = OAL_PTR_NULL;
    hmac_vap_feature_unregister(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    oam_warning_log0(0, OAM_SF_FTM, "{hmac_vap_ftm_deinit!}");
}

osal_void hmac_ftm_initor_end_session(hmac_ftm_initiator_stru *ftm_init)
{
    wifi_printf("FTM initor end session\r\n");
    ftm_init->ftm_initiator = OSAL_FALSE;
    ftm_init->iftmr = OSAL_FALSE;
    ftm_init->ftm_trigger = OSAL_FALSE;
    ftm_init->dialog_token = 0;
    ftm_init->follow_up_dialog_token = 0;
    (osal_void)memset_s(ftm_init->bssid, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
    (osal_void)memset_s(ftm_init->ftm_timer, (sizeof(ftm_timer_stru) * MAC_FTM_TIMER_CNT),
        0, (sizeof(ftm_timer_stru) * MAC_FTM_TIMER_CNT));
}

osal_void hmac_ftm_rspder_end_session(hmac_ftm_responder_stru *ftm_rspder)
{
    wifi_printf("FTM rspder end session\r\n");
    ftm_rspder->ftm_responder = OSAL_FALSE;
    ftm_rspder->received_iftmr = OSAL_FALSE;
    ftm_rspder->dialog_token = 0;
    ftm_rspder->follow_up_dialog_token = 0;
    (osal_void)memset_s(ftm_rspder->mac_ra, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);
    (osal_void)memset_s(ftm_rspder->ftm_timer, (sizeof(ftm_timer_stru) * MAC_FTM_TIMER_CNT),
        0, (sizeof(ftm_timer_stru) * MAC_FTM_TIMER_CNT));
}

/*****************************************************************************
 函 数 名  : hmac_ftm_find_session_index
 功能描述  : 会话过程中通过比较mac选择session_index
*****************************************************************************/
osal_s8 hmac_ftm_find_session_index(hmac_vap_stru *hmac_vap, mac_ftm_mode_enum_uint8 ftm_mode,
    const osal_u8 peer_mac[WLAN_MAC_ADDR_LEN])
{
    osal_u8                  index;
    osal_char                index_number = -1;
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    hmac_ftm_responder_stru *ftm_rspder = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return index_number;
    }
    ftm_init = ftm->ftm_init;
    ftm_rspder = ftm->ftm_rspder;

    if (ftm_mode == MAC_FTM_RESPONDER_MODE) {
        for (index = 0; index < ftm->session_num; index++) {
            if (ftm_rspder[index].ftm_responder != OSAL_TRUE) {
                index_number = (osal_char)index;
                continue;
            }
            if (oal_compare_mac_addr(ftm_rspder[index].mac_ra, peer_mac) == 0) {
                return (osal_char)index;
            }
        }
        if (index_number < 0) {
            oam_error_log1(0, OAM_SF_FTM,
                "vap_id[%d] {hmac_ftm_find_session_index::sessions are all used!!!}", hmac_vap->vap_id);
        }
        return index_number;
    }

    if (ftm_mode == MAC_FTM_INITIATOR_MODE) {
        for (index = 0; index < ftm->session_num; index++) {
            if (ftm_init[index].ftm_initiator != OSAL_TRUE) {
                index_number = (osal_char)index;
                continue;
            }
            if (oal_compare_mac_addr(ftm_init[index].bssid, peer_mac) == 0) {
                return (osal_char)index;
            }
        }
        if (index_number < 0) {
            oam_error_log1(0, OAM_SF_FTM,
                "vap_id[%d] {hmac_ftm_find_session_index::sessions are all used!!!}", hmac_vap->vap_id);
        }
        return index_number;
    }

    oam_error_log2(0, OAM_SF_FTM,
        "vap_id[%d] {hmac_ftm_find_session_index::ftm_mode[%d] error!!!}", hmac_vap->vap_id, ftm_mode);
    return index_number;
}

/*****************************************************************************
 函 数 名  : hmac_ftm_find_unused_session_index
 功能描述  : 会话开始通过结构体下int或resp标志为0选择session_index
*****************************************************************************/
osal_void hmac_ftm_enable_session_index(hmac_vap_stru *hmac_vap, mac_ftm_mode_enum_uint8 ftm_mode,
    const osal_u8 peer_mac[WLAN_MAC_ADDR_LEN], osal_u8 session_id)
{
    hmac_ftm_stru           *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    hmac_ftm_initiator_stru *ftm_init = OSAL_NULL;
    hmac_ftm_responder_stru *ftm_rspder = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return;
    }
    ftm_init = ftm->ftm_init;
    ftm_rspder = ftm->ftm_rspder;

    if (ftm_mode == MAC_FTM_RESPONDER_MODE) {
        if (oal_compare_mac_addr(ftm_rspder[session_id].mac_ra, peer_mac) != 0) {
            (osal_void)memcpy_s(ftm_rspder[session_id].mac_ra, WLAN_MAC_ADDR_LEN, peer_mac, WLAN_MAC_ADDR_LEN);
            ftm_rspder[session_id].received_iftmr = OSAL_FALSE;
        }

        ftm_rspder[session_id].ftm_responder = OSAL_TRUE;
    } else if (ftm_mode == MAC_FTM_INITIATOR_MODE) {
        if (oal_compare_mac_addr(ftm_init[session_id].bssid, peer_mac) != 0) {
            (osal_void)memcpy_s(ftm_init[session_id].bssid, WLAN_MAC_ADDR_LEN, peer_mac, WLAN_MAC_ADDR_LEN);
        }

        ftm_init[session_id].ftm_initiator = OSAL_TRUE;
    } else {
        oam_error_log2(0, OAM_SF_FTM,
            "vap_id[%d] {hmac_ftm_enable_session_index::ftm_mode[%d] error!}",
            hmac_vap->vap_id, ftm_mode);
    }

    return;
}

/*****************************************************************************
 函 数 名  : hmac_check_ftm_switch_channel
 功能描述  : 1.AP作为init 和resp都不容许切信道
             2.sta未关联状态resp数组下都没有与之不同的信道，可以切换。否则不容许切信道
*****************************************************************************/
oal_bool_enum_uint8 hmac_check_ftm_switch_channel(hmac_vap_stru *hmac_vap, osal_u8 chan_number)
{
    osal_u8                          index;
    mac_ftm_mode_enum_uint8          ftm_mode;
    hmac_ftm_stru                   *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    const hmac_ftm_responder_stru   *ftm_rspder = OSAL_NULL;

    if (ftm == OSAL_NULL) {
        return OSAL_FALSE;
    }
    ftm_rspder = ftm->ftm_rspder;
    ftm_mode = mac_check_ftm_enable(hmac_vap);

    /* AP作为init 和resp都不容许切信道.只对STA模式进行处理 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return OSAL_FALSE;
    }

    /* 关联状态作为init 和resp都不容许切信道 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_UP) {
        return OSAL_FALSE;
    }

    /* 未关联状态resp数组下都没有与之不同的信道，可以切换。否则不容许切信道 */
    if (ftm_mode == MAC_FTM_RESPONDER_MODE) {
        for (index = 0; index < ftm->session_num; index++) {
            if ((ftm_rspder[index].ftm_responder == OSAL_TRUE) &&
                (chan_number != ftm_rspder[index].channel_ftm.chan_number)) {
                return OSAL_FALSE;
            } else {
                continue;
            }
        }

        return OSAL_TRUE;
    }

    if ((ftm_mode == MAC_FTM_INITIATOR_MODE) ||
        (ftm_mode == MAC_FTM_MIX_MODE)) {
        return OSAL_FALSE;
    }

    return OSAL_FALSE;
}

osal_void hmac_set_ftm_m2s(hmac_vap_stru *hmac_vap, ftm_m2s_stru m2s)
{
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);
    if (ftm == OSAL_NULL) {
        return;
    }

    ftm->tx_chain_selection = m2s.tx_chain_selection;
    /* chain不能大于3 */
    if ((m2s.tx_chain_selection == 0) || (m2s.tx_chain_selection > 3)) {
        return;
    }

    hal_set_ftm_m2s_phy(m2s.is_mimo, m2s.tx_chain_selection);
}

/*****************************************************************************
 功能描述  : upload ftm message to hmac
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_u32 hmac_config_d2h_ftm_info(const hmac_vap_stru *hmac_vap, const hmac_ftm_data_report *data)
{
    if (data->time_sync == OSAL_TRUE) {
        OAM_IO_PRINTK("{hmac_config_d2h_ftm_info: t1_delta[%lld] time_offset[%lld] clock_time[%d]\r\n",
                      data->t1_delta, data->time_offset, data->clock_time);
        oam_warning_log3(0, OAM_SF_CFG,
                         "{hmac_config_d2h_ftm_info:t1delta = [%lld] timeoffset = [%lld] clocktime = [%lld]\r\n",
                         data->t1_delta, data->time_offset, data->clock_time);
    } else {
        OAM_IO_PRINTK("{hmac_config_d2h_ftm_info:rtt[%lld] distance[%lld]/0.25mm\r\n",
            data->ftm_rtt, data->distance);
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_d2h_vap_ftm_info:rtt[%lld] distance[%lld]/0.25mm\r\n",
                         data->ftm_rtt, data->distance);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : ftm调试命令
*****************************************************************************/
osal_u32 hmac_config_ftm_capacity_enable(hmac_vap_stru *hmac_vap, mac_ftm_debug_switch_stru *ftm_debug)
{
    osal_u32 ret;
    if (ftm_debug->cmd_bit_map & FTM_INITIATOR) { /* ftm_initiator命令 */
        mac_mib_set_fine_timing_msmt_init_activated(hmac_vap, ftm_debug->ftm_initiator);
        hal_set_ftm_sample(ftm_debug->ftm_initiator);
    }
    if (ftm_debug->cmd_bit_map & FTM_RESPONDER) { /* ftm_resp命令 */
        mac_mib_set_fine_timing_msmt_resp_activated(hmac_vap, ftm_debug->ftm_resp);
        hal_set_ftm_ctrl_status(ftm_debug->ftm_resp);
    }
    if (ftm_debug->cmd_bit_map & FTM_RANGE) { /* ftm_range命令 */
        mac_mib_set_fine_timing_msmt_range_req_activated(hmac_vap, ftm_debug->ftm_range);
    }
    if (ftm_debug->cmd_bit_map & FTM_ENABLE) { /* ftm enable命令 */
        if (ftm_debug->ftm_enable.enable == OSAL_TRUE) {
            ret = hmac_vap_ftm_init(hmac_vap, ftm_debug->ftm_enable.session_num);
            if (ret == OSAL_FALSE) {
                oam_warning_log0(0, OAM_SF_FTM, "{hmac_config_ftm_capacity_enable::ftm enable fail!}");
                return OAL_FAIL;
            }
            hal_set_ftm_enable(ftm_debug->ftm_enable.enable);
        } else {
            hmac_vap_ftm_deinit(hmac_vap);
            hal_set_ftm_enable(ftm_debug->ftm_enable.enable);
        }
    }
    return OAL_SUCC;
}

osal_u32 hmac_config_ftm_commond(hmac_vap_stru *hmac_vap, mac_ftm_debug_switch_stru *ftm_debug)
{
    osal_u32 ret = OAL_SUCC;
    hmac_ftm_stru *ftm = (hmac_ftm_stru *)hmac_vap_get_feature_ptr(hmac_vap, WLAN_FEATURE_INDEX_FTM);

    if (ftm == OSAL_NULL) {
        return OAL_FAIL;
    }
    if (ftm_debug->cmd_bit_map & FTM_CALI) { /* ftm cali命令 */
        ftm->cali = ftm_debug->cali;
        if (hmac_vap->al_tx_flag == EXT_SWITCH_ON) {
            hal_set_ftm_cali(hmac_vap->hal_device, NULL, ftm->cali);
        }
    }
    if (ftm_debug->cmd_bit_map & FTM_SEND_FTM) { /* 发送ftm命令 */
        /* 查找session id */
        hmac_ftm_rspder_send(hmac_vap, ftm_debug->send_ftm.mac, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE, WLAN_BW_CAP_20M);
    }
    if ((ftm_debug->cmd_bit_map & FTM_SEND_IFTMR) &&
        (mac_mib_get_fine_timing_msmt_init_activated(hmac_vap) == OSAL_TRUE)) {
        /* 发送iftmr命令 如果命令没有指定BSSID，取关联ap的BSSID */
        ret = hmac_ftm_initor_send(hmac_vap, &ftm_debug->send_iftmr);
        if (ret == OAL_FAIL) {
            oam_warning_log0(0, OAM_SF_FTM, "{hmac_config_ftm_commond::get session id error!}");
        }
    }
    if (ftm_debug->cmd_bit_map & FTM_SET_CORRECT_TIME) { /* 设置校准时间 */
        ret = hmac_set_ftm_correct_time(ftm_debug->ftm_time);
        if (ret != OAL_CONTINUE) {
            oam_warning_log1(0, OAM_SF_FTM, "hmac_config_ftm_commond:set_ftm_correct_time_cb return [%d]", ret);
            return ret;
        }
    }
    if (ftm_debug->cmd_bit_map & FTM_SET_M2S) {
        hmac_set_ftm_m2s(hmac_vap, ftm_debug->m2s);
    }
    if (ftm_debug->cmd_bit_map & FTM_DBG_AUTO_BANDWIDTH) { /* 设置协商带宽使能 */
        ftm->auto_bandwidth = ftm_debug->auto_bandwidth;
    }
    if (ftm_debug->cmd_bit_map & FTM_SEND_RANGE) { /* 发起FTM RANGE 请求 */
        ret = hmac_send_ftm_range_req(hmac_vap, &(ftm_debug->send_ftm_range));
        if (ret != OAL_CONTINUE) {
            oam_warning_log1(0, OAM_SF_FTM, "hmac_config_ftm_commond:send_ftm_range_req_cb return [%d]", ret);
            return ret;
        }
    }
    if (ftm_debug->cmd_bit_map & FTM_ADD_TSF) { /* 补偿的tsf值 */
        ftm->add_tsf = ftm_debug->add_tsf;
    } else {
        ftm->add_tsf = 0;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_ftm_dbg
 功能描述  : 设置ftm_mib为发起端或接收端
*****************************************************************************/
osal_s32 hmac_config_ftm_dbg(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    mac_ftm_debug_switch_stru *ftm_debug = (mac_ftm_debug_switch_stru *)msg->data;

    if (ftm_debug->cmd_bit_map & FTM_SEND_IFTMR) {
        if (hmac_vap->protocol == WLAN_LEGACY_11B_MODE) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_ftm_dbg::fail,11b not supported, %d", hmac_vap->protocol);
            return -OAL_EFAIL;
        }
        if ((ftm_debug->send_iftmr.channel_num == 0) && (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) &&
            (hmac_vap->vap_state != MAC_VAP_STATE_UP) && (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_ftm_dbg::channel number cannot be 0 when not associated !");
            return -OAL_EFAIL;
        }
    }

    /* ftm_initiator命令 */
    if (ftm_debug->cmd_bit_map & FTM_INITIATOR) {
        mac_mib_set_fine_timing_msmt_init_activated(hmac_vap, ftm_debug->ftm_initiator);
    }
    /* ftm_resp命令 */
    if (ftm_debug->cmd_bit_map & FTM_RESPONDER) {
        mac_mib_set_fine_timing_msmt_resp_activated(hmac_vap, ftm_debug->ftm_resp);
    }
    if (ftm_debug->cmd_bit_map & FTM_RANGE) { /* ftm_range命令 */
        mac_mib_set_fine_timing_msmt_range_req_activated(hmac_vap, ftm_debug->ftm_range);
    }

    ret = hmac_config_ftm_capacity_enable(hmac_vap, ftm_debug);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    return (osal_s32)hmac_config_ftm_commond(hmac_vap, ftm_debug);
}

osal_u32 hmac_ftm_rx_process(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap, osal_u8 hooknum, osal_u8 priority)
{
    osal_u8 *data = OSAL_NULL;
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));

    unref_param(hooknum);
    unref_param(priority);

    /* AP & STA处于UP状态，FTM对接收的管理帧进行处理 */
    if (frame_hdr->frame_control.sub_type != WLAN_ACTION || hmac_vap->vap_state != MAC_VAP_STATE_UP ||
        (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP && hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA)) {
        return OAL_CONTINUE;
    }

    data = oal_netbuf_rx_data(*netbuf);
    if (data[MAC_ACTION_OFFSET_CATEGORY] > MAC_ACTION_CATEGORY_VENDOR) {
        return OAL_CONTINUE;
    }

    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_PUBLIC:
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_PUB_FTM: {
                    rx_ctl->rx_info.ftm_frame = 1;
                    hmac_ftm_initor_rx_ftm(hmac_vap, *netbuf);
                    break;
                }
                case MAC_PUB_FTM_REQ: {
                    rx_ctl->rx_info.ftm_frame = 1;
                    hmac_ftm_rspder_rx_ftm_req(hmac_vap, *netbuf);
                    break;
                }
                default:
                    break;
            }
            break;
        case MAC_ACTION_CATEGORY_RADIO_MEASURMENT:
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST: {
                    /* 处理RM request,并将ACTION帧上报 */
                    hmac_ftm_rrm_proc_rm_request(hmac_vap, *netbuf);
                    break;
                }
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函数功能 : FTM HMAC_FRAME_HOOK_TX_PSM_NEED_BUF处理函数
 返回值   : OAL_SUCC:报文需要被缓存
            OAL_CONTINUE:报文不需要被缓存
*****************************************************************************/
osal_u32 hmac_ftm_psm_process(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap, osal_u8 hooknum, osal_u8 priority)
{
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(*netbuf);
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(tx_ctl->tx_user_idx);

    unref_param(hooknum);
    unref_param(priority);

    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_ftm_psm_process::hmac_user[%d] null.}", hmac_vap->vap_id,
            tx_ctl->tx_user_idx);
        return OAL_CONTINUE;
    }

    if (tx_ctl->ismcast == OSAL_FALSE && is_ap(hmac_vap) && mac_is_ftm_related_frame(*netbuf) &&
        (hmac_user->ps_mode == OSAL_TRUE)) {
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
