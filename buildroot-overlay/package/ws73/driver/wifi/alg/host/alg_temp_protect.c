/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm temperature protection
 */

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_temp_protect.h"
#include "hmac_device.h"
#include "alg_schedule_if.h"
#include "hal_common_ops.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"
#include "alg_gla.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_TEMP_PROTECT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
  2 宏定义
******************************************************************************/

/******************************************************************************
  3 函数申明
******************************************************************************/
OSAL_STATIC osal_void alg_temp_protect_init_entry(osal_void *ctx);
OSAL_STATIC osal_void alg_temp_protect_init_exit(osal_void *ctx);
OSAL_STATIC osal_u32 alg_temp_protect_init_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);

OSAL_STATIC osal_void alg_temp_protect_safe_entry(osal_void *ctx);
OSAL_STATIC osal_u32 alg_temp_protect_safe_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);

OSAL_STATIC osal_void alg_temp_protect_normal_entry(osal_void *ctx);
OSAL_STATIC osal_u32 alg_temp_protect_normal_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);

OSAL_STATIC osal_void alg_temp_protect_over_entry(osal_void *ctx);
OSAL_STATIC osal_u32 alg_temp_protect_over_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);

OSAL_STATIC osal_void alg_temp_protect_pa_off_entry(osal_void *ctx);
OSAL_STATIC osal_void alg_temp_protect_pa_off_exit(osal_void *ctx);
OSAL_STATIC osal_u32 alg_temp_protect_pa_off_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);

OSAL_STATIC osal_u32 alg_temp_protect_fsm_trans_to_state(alg_temp_protect_fsm_stru *temp_protect_fsm, osal_u8 state);
OSAL_STATIC osal_u32 alg_temp_protect_fsm_post_event(alg_temp_protect_stru *temp_protect, osal_u16 type,
    osal_u16 datalen, osal_u8 *data);

OSAL_STATIC osal_u32 alg_temp_protect_cur_temp_get(alg_temp_protect_stru *temp_protect, osal_s16 *temperature);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_void alg_temp_protect_config_temp_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param);
OSAL_STATIC osal_void alg_temp_protect_config_debug_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param);
OSAL_STATIC osal_void alg_temp_protect_config_times_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param);
OSAL_STATIC osal_void alg_temp_protect_config_paoff_set(const alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param, hmac_temp_pri_custom_stru *temp_custom);
OSAL_STATIC osal_void alg_temp_protect_config_over_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param, hmac_temp_pri_custom_stru *temp_pri_custom);
OSAL_STATIC osal_void alg_temp_protect_config_safe_set(const alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param, hmac_temp_pri_custom_stru *temp_pri_custom);
OSAL_STATIC osal_void alg_temp_protect_info_get(osal_void);
OSAL_STATIC osal_u32 alg_temp_protect_config_param(hmac_vap_stru *vap, frw_msg *msg);
#endif
OSAL_STATIC osal_u32 alg_temp_protect_send_msg(osal_u8 flag, osal_u16 msg_id);

OSAL_STATIC osal_void alg_temp_stat_record(alg_temp_protect_state cur_state, osal_u32 cur_duty_cyc);
OSAL_STATIC osal_void alg_temp_stat_record_in_act(osal_u32 pre_set_duty_cyc);

OSAL_STATIC osal_void alg_temp_protect_data_clear(osal_void);
OSAL_STATIC osal_u32 alg_temp_protect_timer_hander(osal_void *arg);
OSAL_STATIC osal_u32 alg_temp_protect_duty_cyc_get(const hal_to_dmac_device_stru *hal_device);
OSAL_STATIC alg_temp_protect_state alg_temp_protect_get_will_state(osal_s16 temperature,
    const alg_temp_protect_stru *temp_protect);
OSAL_STATIC osal_void alg_temp_protect_safe_sta_act(alg_temp_protect_stru *temp_protect, osal_u32 cur_duty_cyc);
OSAL_STATIC osal_void alg_temp_protect_over_sta_act(alg_temp_protect_stru *temp_protect, osal_u32 cur_duty_cyc);
OSAL_STATIC osal_void alg_temp_protect_fsm_attach(alg_temp_protect_stru *temp_protect);
OSAL_STATIC osal_void alg_temp_protect_fsm_detach(alg_temp_protect_stru *temp_protect);

hmac_temp_pri_custom_stru g_dmac_temp_protect_pri_custom = {
    /* 过温保护的使能开关 */
    HMAC_TEMP_PROTECT_ENABLE,
    /* 过温保护过程中是否需要降低功率 */
    HMAC_TEMP_PROTECT_REDUCE_ENABLE,
    0, /* reserve */
    /* 过温保护的恢复安全水线 */
    HMAC_TEMP_PROTECT_RECOVER_TH,
    /* 过温保护的PA_OFF水线 */
    HMAC_TEMP_PROTECT_PA_OFF_TH
};

/******************************************************************************
  4 全局变量定义
******************************************************************************/
alg_temp_protect_stru g_hmac_temp_protect;

/* 鍚勭姸鎬佸叆鍙ｅ嚭鍙ｅ嚱鏁? */

static oal_fsm_state_info  g_alg_temp_protect_fsm_info[] = {
    {
        ALG_TEMP_PROTECT_STATE_INIT,
        "INIT",
        alg_temp_protect_init_entry,
        alg_temp_protect_init_exit,
        alg_temp_protect_init_event,
    },
    {
        ALG_TEMP_PROTECT_STATE_SAFE,
        "SAFE",
        alg_temp_protect_safe_entry,
        OSAL_NULL,
        alg_temp_protect_safe_event,
    },
    {
        ALG_TEMP_PROTECT_STATE_NORMAL,
        "NORMAL",
        alg_temp_protect_normal_entry,
        OSAL_NULL,
        alg_temp_protect_normal_event,
    },
    {
        ALG_TEMP_PROTECT_STATE_OVER,
        "OVER",
        alg_temp_protect_over_entry,
        OSAL_NULL,
        alg_temp_protect_over_event,
    },
    {
        ALG_TEMP_PROTECT_STATE_PA_OFF,
        "PA_OFF",
        alg_temp_protect_pa_off_entry,
        alg_temp_protect_pa_off_exit,
        alg_temp_protect_pa_off_event,
    },
};


/******************************************************************************
  5 函数实现
******************************************************************************/
/******************************************************************************
 函 数 名  : hmac_get_pri_custom
 功能描述  : 获取私人定制的配置结构体的指针
******************************************************************************/
OSAL_STATIC hmac_temp_pri_custom_stru *hmac_get_pri_custom(osal_void)
{
    return &g_dmac_temp_protect_pri_custom;
}

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT_CLDO_MODE
/******************************************************************************
 函 数 名  : alg_temp_set_cldo_mode_temp_th
 功能描述  : 设置私人定制的配置结构体的指针
******************************************************************************/
OSAL_STATIC osal_void alg_temp_set_cldo_mode_temp_th(alg_temp_protect_stru *temp_protect)
{
    oal_bool_enum_uint8 is_chip_cldo_mode = hal_chip_is_cldo_mode();
    hmac_temp_pri_custom_stru *temp_pri = hmac_get_pri_custom();

    if (is_chip_cldo_mode == OSAL_TRUE) {
        temp_pri->temp_pro_pa_off_th -= ALG_TEMP_PROTECT_CUSTOM_TH_ADJUST;
        temp_protect->over_th_set -= ALG_TEMP_PROTECT_CUSTOM_TH_ADJUST;
    }
}
#endif

/******************************************************************************
函 数 名  :alg_temp_print_gla_info
功能描述  : 温度保护算法图形化维测打印
 ******************************************************************************/
OSAL_STATIC osal_void alg_temp_print_gla_info(alg_temp_protect_state cur_state)
{
    oam_warning_log1(0, OAM_SF_ANY, "[GLA][ALG][TEMP]:cur_state=%d", cur_state);
}

/******************************************************************************
 函数名     : alg_temp_stat_record
 功能描述  : event进入时 记录当前状态
******************************************************************************/
osal_void alg_temp_stat_record(alg_temp_protect_state cur_state, osal_u32 cur_duty_cyc)
{
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_TEMP_PRTECT, ALG_GLA_USUAL_SWITCH);
    osal_u8 idx;
    idx = temp_protect->record_idx;

    /* 校验idx合法性 */
    if (osal_unlikely(idx >= ALG_TEMP_PROTECT_RECORD_AMOUNT)) {
        temp_protect->record_idx = 0;  // idx重新从0开始记录
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_stat_record::invalid idx.}");
        return;
    }

    /* 校验cur_state合法性 */
    if (osal_unlikely(cur_state > ALG_TEMP_PROTECT_STATE_PA_OFF)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_stat_record::invalid state.}");
        return;
    }

    if (switch_enable) {
        alg_temp_print_gla_info(cur_state);
    }

    /* 记录当前状态 */
    temp_protect->stat_record[idx].ave_temp = temp_protect->ave_temp;
    temp_protect->stat_record[idx].schedule_reduce = temp_protect->schedule_reduce;
    temp_protect->stat_record[idx].pre_set_duty_cyc = temp_protect->pre_set_duty_cyc;
    temp_protect->stat_record[idx].reduce_pwr_enable = temp_protect->reduce_pwr_enable;

    temp_protect->stat_record[idx].temp_pro_aggr_size = temp_protect->temp_pro_aggr_size;
    temp_protect->stat_record[idx].times_th = temp_protect->times_th;
    temp_protect->stat_record[idx].cur_duty_cyc = cur_duty_cyc;
    temp_protect->stat_record[idx].state = cur_state;
    temp_protect->stat_record[idx].is_pa_off = temp_protect->is_pa_off;

    /* 记录index++ */
    temp_protect->record_idx = ((idx + 1) % ALG_TEMP_PROTECT_RECORD_AMOUNT);
}

/******************************************************************************
 函数名     : alg_temp_stat_record_in_act
 功能描述  : 进入探测时 更新占空比预设值
******************************************************************************/
osal_void alg_temp_stat_record_in_act(osal_u32 pre_set_duty_cyc)
{
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;
    osal_u8 idx;

    /* 获取当前temp_protect->record_idx */
    idx = (temp_protect->record_idx + ALG_TEMP_PROTECT_RECORD_AMOUNT - 1) % ALG_TEMP_PROTECT_RECORD_AMOUNT;

    /* 校验pre_set_duty_cyc合法性 */
    if (osal_unlikely(pre_set_duty_cyc > ALG_TEMP_PROTECT_DUTY_CYC_FULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_stat_record_in_act::invalid state.}");
        return;
    }

    /* 更新占空比预设值 */
    temp_protect->stat_record[idx].pre_set_duty_cyc = pre_set_duty_cyc;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_get_will_state
 功能描述  : 获取下一状态
******************************************************************************/
alg_temp_protect_state alg_temp_protect_get_will_state(osal_s16 temperature, const alg_temp_protect_stru *temp_protect)
{
    hmac_temp_pri_custom_stru *temp_pri_custom = OSAL_NULL;
    osal_u16 over_th = temp_protect->over_th_set;

    /* 不同状态温度门限获取 */
    temp_pri_custom = hmac_get_pri_custom();
    /* 温度高于pa_off/over状态 */
    if (temperature >= (temp_pri_custom->temp_pro_pa_off_th)) {
        return ALG_TEMP_PROTECT_STATE_PA_OFF;
    }
    if (temperature >= over_th) {
        return ALG_TEMP_PROTECT_STATE_OVER;
    }

    /* 安全恢复温度可定制修改 */
    if (temperature < temp_pri_custom->temp_pro_safe_th) {
        return ALG_TEMP_PROTECT_STATE_SAFE;
    }

    return ALG_TEMP_PROTECT_STATE_NORMAL;
}
/******************************************************************************
 函 数 名  : alg_temp_protect_init_entry
 功能描述  : init状态的ENTRY
******************************************************************************/
osal_void alg_temp_protect_init_entry(osal_void *ctx)
{
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();

    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_INIT;
    alg_temp_protect_data_clear();

    /* init state entry，定时器采样周期修改为10s */
    frw_create_timer_entry(&temp_protect->temp_protect_timer, alg_temp_protect_timer_hander,
        ALG_TEMP_PROTECT_TIMER_CYCLE_IDLE, (osal_void *)(hal_device), OSAL_TRUE);
    return;
}
/******************************************************************************
 函 数 名  : alg_temp_protect_init_exit
 功能描述  : init状态的exit
******************************************************************************/
osal_void alg_temp_protect_init_exit(osal_void *ctx)
{
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();

    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));

    /* init state exit 退出，定时器采样周期修改为1s 其他状态都是1s一次 */
    frw_create_timer_entry(&temp_protect->temp_protect_timer, alg_temp_protect_timer_hander,
        ALG_TEMP_PROTECT_TIMER_CYCLE, (osal_void *)(hal_device), OSAL_TRUE);
    return;
}
/******************************************************************************
 函 数 名  : alg_temp_protect_state_init_event
 功能描述  : init状态的event接口
******************************************************************************/
osal_u32 alg_temp_protect_init_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void *event_data)
{
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_state will_state;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    unref_param(event_data_len);
    unref_param(event_data);
    /* 记录并打印进入当前状态时的温度保护参数 */
    alg_temp_stat_record(ALG_TEMP_PROTECT_STATE_INIT, 0);
    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    will_state = alg_temp_protect_get_will_state(temp_protect->ave_temp, temp_protect);
    if (osal_unlikely(will_state == ALG_TEMP_PROTECT_BUTT_STATE)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_init_event::invalid will_state.}");
        return OAL_FAIL;
    }

    switch (event) {
        case ALG_TEMP_PROTECT_EVENT_TIMER:
            alg_temp_protect_fsm_trans_to_state(temp_protect_fsm_ctx, will_state);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

osal_u32 alg_temp_protect_send_msg(osal_u8 flag, osal_u16 msg_id)
{
    osal_u32 ret = OAL_FAIL;
    frw_msg msg = {0};
    msg.data = &flag;
    msg.data_len = (osal_u16)sizeof(flag);

    /* 抛事件至Device侧alg，tpc pwr reduce or NOT event */
    ret = (osal_u32)frw_send_msg_to_device(0, msg_id, &msg, OSAL_FALSE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_temp_protect_send_tpc_pwr_msg:: temp send fail [%d].}", ret);
        return ret;
    }
    return ret;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_safe_entry
 功能描述  : safe状态的ENTRY
******************************************************************************/
osal_void alg_temp_protect_safe_entry(osal_void *ctx)
{
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    if (temp_protect->temp_pro_aggr_size == ALG_TEMP_PROTECT_MAX_AGGR_SIZE) {
        /*  safe状态时，聚合度到达max，定时器累加10次抛事件 */
        temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_SAFE;
        alg_temp_protect_data_clear();
    } else {
        /*  safe状态时，聚合度未到达max，定时器累加3次抛事件 */
        temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_OVER;
        alg_temp_protect_data_clear();
    }

    /* 进入safe状态 关闭减少调度使能 减少功率开关 */
    temp_protect->schedule_reduce = OSAL_FALSE;
    temp_protect->reduce_pwr_enable = OSAL_FALSE;
    alg_temp_protect_send_msg(OSAL_FALSE, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_REDUCE_SCHEDULE);
    alg_temp_protect_send_msg(OSAL_FALSE, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_REDUCE_TPC);
    temp_protect->pre_set_duty_cyc = ALG_TEMP_PROTECT_DUTY_CYC_MIN;

    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_safe_event
 功能描述  : safe状态的event接口
******************************************************************************/
osal_u32 alg_temp_protect_safe_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void *event_data)
{
    alg_temp_protect_state temp_state;
    alg_temp_protect_stru *temp_protect;
    osal_u32 cur_duty_cyc;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    unref_param(event_data_len);
    unref_param(event_data);
    hal_device = hal_chip_get_hal_device();
    cur_duty_cyc = alg_temp_protect_duty_cyc_get(hal_device);
    /* 记录进入当前状态时的温度保护参数 */
    alg_temp_stat_record(ALG_TEMP_PROTECT_STATE_SAFE, cur_duty_cyc);
    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    temp_state = alg_temp_protect_get_will_state(temp_protect->ave_temp, temp_protect);

    switch (event) {
        case ALG_TEMP_PROTECT_EVENT_DEVICE_IDLE:
            alg_temp_protect_fsm_trans_to_state(temp_protect_fsm_ctx, ALG_TEMP_PROTECT_STATE_INIT);
            break;
        case ALG_TEMP_PROTECT_EVENT_TIMER:
            if ((temp_state == ALG_TEMP_PROTECT_STATE_OVER) || (temp_state == ALG_TEMP_PROTECT_STATE_PA_OFF)) {
                alg_temp_protect_fsm_trans_to_state(temp_protect_fsm_ctx, temp_state);
            } else if (temp_state == ALG_TEMP_PROTECT_STATE_SAFE) {
                if (temp_protect->temp_pro_aggr_size != ALG_TEMP_PROTECT_MAX_AGGR_SIZE) {
                    alg_temp_protect_safe_sta_act(temp_protect, cur_duty_cyc);
                }
            } else if (temp_state == ALG_TEMP_PROTECT_STATE_NORMAL) {
                /* 不切换状态 do nothing */
            } else {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_safe_event::will sta is wrong.}");
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}
/******************************************************************************
 函 数 名  : alg_temp_protect_safe_act
 功能描述  : safe状态需要做的操作
******************************************************************************/
osal_void alg_temp_protect_safe_sta_act(alg_temp_protect_stru *temp_protect, osal_u32 cur_duty_cyc)
{
    /* 每次进入该接口，预设占空比升高10%,直到升至100% */
    if (temp_protect->pre_set_duty_cyc < ALG_TEMP_PROTECT_DUTY_CYC_FULL) {
        temp_protect->pre_set_duty_cyc = temp_protect->pre_set_duty_cyc + ALG_TEMP_PROTECT_DUTY_CYC_STEP;
    }

    /* 更新预设占空比记录 */
    alg_temp_stat_record_in_act(temp_protect->pre_set_duty_cyc);
    oam_warning_log2(0, OAM_SF_ANY, "{alg_temp_protect_safe_sta_act duty_cyc=%d aggr_size= %d}",
        cur_duty_cyc, temp_protect->temp_pro_aggr_size);

    /* 当前占空比大于预设值，减少聚合个数 */
    if (cur_duty_cyc > temp_protect->pre_set_duty_cyc) {
        /* 区分开写是因为 1 - 2 小于0 聚合度是无符号数 */
        if (temp_protect->temp_pro_aggr_size >=
            ALG_TEMP_PROTECT_MIN_AGGR_SIZE + ALG_TEMP_PROTECT_AGGR_SIZE_STEP) {
            /* 原聚合度 >= 3 */
            temp_protect->temp_pro_aggr_size -= ALG_TEMP_PROTECT_AGGR_SIZE_STEP;
        } else {
            /* 原聚合度 < 3 进入 减少到 1 */
            temp_protect->temp_pro_aggr_size = ALG_TEMP_PROTECT_MIN_AGGR_SIZE;
        }
        /* 聚合度改变后 发消息给device侧 */
        alg_temp_protect_send_msg(temp_protect->temp_pro_aggr_size,
            WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_AGGR_CHANGE);
        return;
    }

    /* 当前占空比 小于 预设值，增加聚合个数 */
    if ((temp_protect->temp_pro_aggr_size) < ALG_TEMP_PROTECT_MAX_AGGR_SIZE) {
        temp_protect->temp_pro_aggr_size += ALG_TEMP_PROTECT_AGGR_SIZE_STEP;
        temp_protect->temp_pro_aggr_size =
            osal_min(temp_protect->temp_pro_aggr_size, ALG_TEMP_PROTECT_MAX_AGGR_SIZE);
        alg_temp_protect_send_msg(temp_protect->temp_pro_aggr_size, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_AGGR_CHANGE);
    }
    if (temp_protect->temp_pro_aggr_size == ALG_TEMP_PROTECT_MAX_AGGR_SIZE) {
        /*  safe状态时，占空比不受聚合个数限制时，定时器累加10次才抛事件 */
        temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_SAFE;
        alg_temp_protect_data_clear();
    }
}

/******************************************************************************
 函 数 名  : alg_temp_protect_normal_entry
 功能描述  : normal状态的ENTRY
******************************************************************************/
osal_void alg_temp_protect_normal_entry(osal_void *ctx)
{
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;

    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    /* normal state entry时，定时器累加3次才抛事件 */
    temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_OVER;
    alg_temp_protect_data_clear();

    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_normal_event
 功能描述  : normal状态的event接口
******************************************************************************/
osal_u32 alg_temp_protect_normal_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void *event_data)
{
    alg_temp_protect_state protect_state;
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    unref_param(event_data_len);
    unref_param(event_data);

    /* 记录进入当前状态时的温度保护参数 */
    alg_temp_stat_record(ALG_TEMP_PROTECT_STATE_NORMAL, 0);

    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    protect_state = alg_temp_protect_get_will_state(temp_protect->ave_temp, temp_protect);

    switch (event) {
        case ALG_TEMP_PROTECT_EVENT_DEVICE_IDLE:
            alg_temp_protect_fsm_trans_to_state(temp_protect_fsm_ctx, ALG_TEMP_PROTECT_STATE_INIT);
            break;
        case ALG_TEMP_PROTECT_EVENT_TIMER:
            if ((protect_state == ALG_TEMP_PROTECT_STATE_SAFE) || (protect_state == ALG_TEMP_PROTECT_STATE_OVER) ||
                (protect_state == ALG_TEMP_PROTECT_STATE_PA_OFF)) {
                alg_temp_protect_fsm_trans_to_state(temp_protect_fsm_ctx, protect_state);
            } else if (protect_state == ALG_TEMP_PROTECT_STATE_NORMAL) {
                /* do nothing */
            } else {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_normal_event::will sta is wrong.}");
            }
            break;
        default:
            break;
    }
    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_over_entry
 功能描述  : over状态的ENTRY
******************************************************************************/
osal_void alg_temp_protect_over_entry(osal_void *ctx)
{
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    /* over state entry时，定时器累加3次抛事件 */
    temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_OVER;
    alg_temp_protect_data_clear();

    temp_protect->pre_set_duty_cyc = ALG_TEMP_PROTECT_DUTY_CYC_MAX;

    /* 首次进入over状态，聚合度大于最大聚合度1/2, 则减少到最大聚合的1/2 */
    if (temp_protect->temp_pro_aggr_size > (ALG_TEMP_PROTECT_MAX_AGGR_SIZE >> 1)) {
        temp_protect->temp_pro_aggr_size = (ALG_TEMP_PROTECT_MAX_AGGR_SIZE >> 1);
        alg_temp_protect_send_msg(temp_protect->temp_pro_aggr_size, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_AGGR_CHANGE);
    }
    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_over_event
 功能描述  : over状态的event接口
******************************************************************************/
osal_u32 alg_temp_protect_over_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void *event_data)
{
    alg_temp_protect_state will_state;
    alg_temp_protect_stru *temp_protect;
    osal_u32 cur_duty_cyc;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_temp_protect_fsm_stru *temp_protect_ctx = (alg_temp_protect_fsm_stru *)ctx;
    unref_param(event_data_len);
    unref_param(event_data);

    hal_device = hal_chip_get_hal_device();
    cur_duty_cyc = alg_temp_protect_duty_cyc_get(hal_device);

    /* 记录进入当前状态时的温度保护参数 */
    alg_temp_stat_record(ALG_TEMP_PROTECT_STATE_OVER, cur_duty_cyc);

    temp_protect = ((alg_temp_protect_stru *)((temp_protect_ctx)->oal_fsm.p_oshandler));
    will_state = alg_temp_protect_get_will_state(temp_protect->ave_temp, temp_protect);

    switch (event) {
        case ALG_TEMP_PROTECT_EVENT_TIMER:
            if ((will_state == ALG_TEMP_PROTECT_STATE_SAFE) || (will_state == ALG_TEMP_PROTECT_STATE_NORMAL) ||
                (will_state == ALG_TEMP_PROTECT_STATE_PA_OFF)) {
                alg_temp_protect_fsm_trans_to_state(temp_protect_ctx, will_state);
            } else if (will_state == ALG_TEMP_PROTECT_STATE_OVER) {
                alg_temp_protect_over_sta_act(temp_protect, cur_duty_cyc);
            } else {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_over_event::will sta is wrong.}");
            }
            break;
        case ALG_TEMP_PROTECT_EVENT_DEVICE_IDLE:
            alg_temp_protect_fsm_trans_to_state(temp_protect_ctx, ALG_TEMP_PROTECT_STATE_INIT);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_over_act
 功能描述  : 过温状态下采用快降聚合缓升聚合的策略，保证温度迅速降下来
******************************************************************************/
osal_void alg_temp_protect_over_sta_act(alg_temp_protect_stru *temp_protect, osal_u32 cur_duty_cyc)
{
    /* 获取当前的占空比 cur_duty_cyc */
    hmac_temp_pri_custom_stru *temp_pri_custom = OSAL_NULL;

    /* 每次进入该接口，预设占空比降低10%,直到降至10% */
    if (temp_protect->pre_set_duty_cyc > ALG_TEMP_PROTECT_DUTY_CYC_STEP) {
        temp_protect->pre_set_duty_cyc -= ALG_TEMP_PROTECT_DUTY_CYC_STEP;
    } else {
        temp_protect->pre_set_duty_cyc = ALG_TEMP_PROTECT_DUTY_CYC_MIN;
    }

    /* 更新预设占空比记录 */
    alg_temp_stat_record_in_act(temp_protect->pre_set_duty_cyc);
    oam_warning_log3(0, OAM_SF_ANY, "{alg_temp_protect_over_sta_act duty_cyc=%d aggr_size= %d schedule reduce = %d}",
        cur_duty_cyc, temp_protect->temp_pro_aggr_size, temp_protect->schedule_reduce);

    /* 当前占空比小于预设值 , 聚合度按照步长 1 逐步加大 */
    if ((cur_duty_cyc < temp_protect->pre_set_duty_cyc) &&
        (temp_protect->temp_pro_aggr_size <= ALG_TEMP_PROTECT_MAX_AGGR_SIZE)) {
        temp_protect->temp_pro_aggr_size += ALG_TEMP_PROTECT_MIN_AGGR_SIZE;
        temp_protect->temp_pro_aggr_size =
            osal_min(temp_protect->temp_pro_aggr_size, ALG_TEMP_PROTECT_MAX_AGGR_SIZE);
        alg_temp_protect_send_msg(temp_protect->temp_pro_aggr_size, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_AGGR_CHANGE);
        return;
    }

    /* 当前占空比大于预设 , 且聚合个数大于等于3，继续减少聚合个数 */
    if (temp_protect->temp_pro_aggr_size >= (ALG_TEMP_PROTECT_MIN_AGGR_SIZE + ALG_TEMP_PROTECT_AGGR_SIZE_STEP)) {
        temp_protect->temp_pro_aggr_size >>= 1;
        alg_temp_protect_send_msg(temp_protect->temp_pro_aggr_size, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_AGGR_CHANGE);
        return;
    }

    /* 当前占空比大于预设 聚合个数小于3 设为最小聚合度 */
    if (temp_protect->temp_pro_aggr_size > ALG_TEMP_PROTECT_MIN_AGGR_SIZE) {
        temp_protect->temp_pro_aggr_size = ALG_TEMP_PROTECT_MIN_AGGR_SIZE;
        alg_temp_protect_send_msg(temp_protect->temp_pro_aggr_size, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_AGGR_CHANGE);
        return;
    }

    /* aggr_size已经减到了1，则减少一次调度 */
    if (temp_protect->schedule_reduce == OSAL_FALSE) {
        temp_protect->schedule_reduce = OSAL_TRUE;
        /* 通知调度算法 */
        alg_temp_protect_send_msg(OSAL_TRUE, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_REDUCE_SCHEDULE);
        /* 定时器累加10次抛事件，目的：拉长等待时间，温度稳定后决定是否降功率 */
        temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_SAFE;
        alg_temp_protect_data_clear();
        return;
    }

    /* 等待一次10次采样后，恢复为3次 */
    temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_OVER;
    alg_temp_protect_data_clear();

    /* 不同状态温度门限获取 */
    temp_pri_custom = hmac_get_pri_custom();
    /* 减少tpc功率判断 */
    if ((temp_protect->reduce_pwr_enable == OSAL_FALSE) && (temp_pri_custom->temp_pro_reduce_pwr_enable == OSAL_TRUE)) {
        temp_protect->reduce_pwr_enable = OSAL_TRUE;
        alg_temp_protect_send_msg(OSAL_TRUE, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_REDUCE_TPC);
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_over_sta_act::alg_tpc_over_temp_reduce_pwr}");
        return;
    }
}

/******************************************************************************
 功能描述  : 同步PA状态到device侧
******************************************************************************/
OSAL_STATIC osal_void alg_temp_protect_sync_pa_off_state(osal_u8 is_pa_off)
{
    osal_u32 ret = OAL_FAIL;
    frw_msg msg = {0};
    msg.data = &is_pa_off;
    msg.data_len = (osal_u16)sizeof(is_pa_off);

    ret = (osal_u32)frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_TEMP_PA_OFF, &msg, OSAL_TRUE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_temp_protect_sync_pa_off_state:: temp send fail [%d].}", ret);
    }
    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_pa_off_entry
 功能描述  : pa_off状态的ENTRY
 ******************************************************************************/
osal_void alg_temp_protect_pa_off_entry(osal_void *ctx)
{
    alg_temp_protect_fsm_stru *temp_protect_fsm;
    hal_to_dmac_device_stru *hal_device;
    alg_temp_protect_stru *temp_protect;

    temp_protect_fsm = (alg_temp_protect_fsm_stru *)ctx;
    temp_protect = (alg_temp_protect_stru *)(temp_protect_fsm->oal_fsm.p_oshandler);

    /* pa_off state entry时，加快采样次数 1次 */
    temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_PA_OFF;
    alg_temp_protect_data_clear();

    hal_device = hal_chip_get_hal_device();
    temp_protect->is_pa_off = OSAL_TRUE;
    hal_device->is_temp_protect_pa_off = OSAL_TRUE;
    alg_temp_protect_sync_pa_off_state(OSAL_TRUE);

    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_pa_off_exit
 功能描述  : pa_off状态的EXIT
******************************************************************************/
osal_void alg_temp_protect_pa_off_exit(osal_void *ctx)
{
    alg_temp_protect_fsm_stru *temp_protect_fsm;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_temp_protect_stru *temp_protect = OSAL_NULL;

    temp_protect_fsm = (alg_temp_protect_fsm_stru *)ctx;
    temp_protect = (alg_temp_protect_stru *)(temp_protect_fsm->oal_fsm.p_oshandler);

    hal_device = hal_chip_get_hal_device();
    temp_protect->is_pa_off = OSAL_FALSE;
    hal_device->is_temp_protect_pa_off = OSAL_FALSE;
    alg_temp_protect_sync_pa_off_state(OSAL_FALSE);

    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_pa_off_event
 功能描述  : serious状态的event接口
******************************************************************************/
osal_u32 alg_temp_protect_pa_off_event(osal_void *ctx, osal_u16 event,
    osal_u16 event_data_len, osal_void *event_data)
{
    alg_temp_protect_state will_state;
    alg_temp_protect_stru *temp_protect;
    alg_temp_protect_fsm_stru *temp_protect_fsm_ctx = (alg_temp_protect_fsm_stru *)ctx;
    unref_param(event_data_len);
    unref_param(event_data);

    /* 记录进入当前状态时的温度保护参数 */
    alg_temp_stat_record(ALG_TEMP_PROTECT_STATE_PA_OFF, 0);

    temp_protect = ((alg_temp_protect_stru *)((temp_protect_fsm_ctx)->oal_fsm.p_oshandler));
    will_state = alg_temp_protect_get_will_state(temp_protect->ave_temp, temp_protect);
    switch (event) {
        case ALG_TEMP_PROTECT_EVENT_DEVICE_IDLE:
            alg_temp_protect_fsm_trans_to_state(temp_protect_fsm_ctx, ALG_TEMP_PROTECT_STATE_INIT);
            break;
        case ALG_TEMP_PROTECT_EVENT_TIMER:
            if ((will_state == ALG_TEMP_PROTECT_STATE_NORMAL) ||
                (will_state == ALG_TEMP_PROTECT_STATE_SAFE) ||
                (will_state == ALG_TEMP_PROTECT_STATE_OVER)) {
                alg_temp_protect_fsm_trans_to_state(temp_protect_fsm_ctx, will_state);
            } else if (will_state == ALG_TEMP_PROTECT_STATE_PA_OFF) {
                /* do nothing */
            } else {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_pa_off_event::will sta is wrong.}");
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_fsm_trans_to_state
 功能描述  : 过温保护状态机状态切换函数
******************************************************************************/
osal_u32 alg_temp_protect_fsm_trans_to_state(alg_temp_protect_fsm_stru *temp_protect_fsm, osal_u8 state)
{
    oal_fsm_stru *oal_fsm = &(temp_protect_fsm->oal_fsm);

    oam_warning_log1(0, OAM_SF_ANY, "{alg_temp_protect_fsm_trans_to_state %d}", state);
    return oal_fsm_trans_to_state(oal_fsm, state);
}
/******************************************************************************
 函 数 名  : alg_temp_protect_fsm_post_event
 功能描述  : 触发状态机事件处理函数
******************************************************************************/
osal_u32 alg_temp_protect_fsm_post_event(alg_temp_protect_stru *temp_protect, osal_u16 type, osal_u16 datalen,
    osal_u8 *data)
{
    osal_u32 ret;
    alg_temp_protect_fsm_stru *handler = OSAL_NULL;

    if (osal_unlikely(type >= ALG_TEMP_PROTECT_EVENT_BUTT)) {
        oam_error_log1(0, OAM_SF_ANY, "alg_temp_protect_fsm_post_event:: event type[%d] error, NULL!", type);
        return OAL_FAIL;
    }

    if (osal_unlikely(temp_protect == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_fsm_post_event::temp_protect null.}");
        return OAL_FAIL;
    }

    handler = &(temp_protect->temp_pro_fsm);
    if (osal_unlikely(handler->is_fsm_attached == OSAL_FALSE)) {
        oam_warning_log1(0, OAM_SF_ANY, "alg_temp_protect_fsm_post_event::temp_fsm_attached = %d!",
            handler->is_fsm_attached);
        return OAL_FAIL;
    }

    ret = oal_fsm_event_dispatch(&(handler->oal_fsm), type, datalen, data);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_fsm_post_event::oal_fsm_event_dispatch fail.}");
        return ret;
    }

    return OAL_SUCC;
}
/******************************************************************************
 函 数 名  : alg_temp_protect_fsm_attach
 功能描述  : 过温度保护创建状态机
******************************************************************************/
osal_void alg_temp_protect_fsm_attach(alg_temp_protect_stru *temp_protect)
{
    osal_u32 ret;
    osal_u8                fsm_name[6] = {0}; // 6:数组大小
    alg_temp_protect_fsm_stru *temp_protect_fsm = OSAL_NULL;
    oal_fsm_create_stru fsm_create_stru;

    if (osal_unlikely(temp_protect->temp_pro_fsm.is_fsm_attached == OSAL_TRUE)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_fsm_attach::hal device id fsm have attached.}");
        return;
    }

    temp_protect_fsm = &(temp_protect->temp_pro_fsm);
    (osal_void)memset_s(temp_protect_fsm, sizeof(alg_temp_protect_fsm_stru), 0, sizeof(alg_temp_protect_fsm_stru));
    fsm_create_stru.oshandle = (osal_void *)temp_protect;
    fsm_create_stru.name = fsm_name;
    fsm_create_stru.context = temp_protect_fsm;

    ret = oal_fsm_create(&fsm_create_stru, &(temp_protect_fsm->oal_fsm), ALG_TEMP_PROTECT_STATE_INIT,
        g_alg_temp_protect_fsm_info, sizeof(g_alg_temp_protect_fsm_info) / sizeof(oal_fsm_state_info));
    if (ret == OAL_SUCC) {
        /* oal fsm create succ */
        temp_protect_fsm->is_fsm_attached = OSAL_TRUE;
        return;
    }
#ifdef _PRE_WLAN_ALG_UART_PRINT
    wifi_printf("temp fsm attach fail!\n");
#endif
    oam_error_log0(0, OAM_SF_ANY, "{alg_temp_protect_fsm_attach::temp fsm attach fail!.}");
}

/******************************************************************************
 函 数 名  : alg_temp_protect_fsm_detach
 功能描述  : 过温度保护注销状态机
******************************************************************************/
osal_void alg_temp_protect_fsm_detach(alg_temp_protect_stru *temp_protect)
{
    alg_temp_protect_fsm_stru *handler = OSAL_NULL;

    handler = &temp_protect->temp_pro_fsm;
    if (osal_unlikely(handler->is_fsm_attached == OSAL_FALSE)) {
        oam_error_log0(0, OAM_SF_ANY, "alg_temp_protect_fsm_detach::device intf det fsm not attatched");
        return;
    }

    /* 不是init状态切换到init状态 */
    if (handler->oal_fsm.cur_state != ALG_TEMP_PROTECT_STATE_INIT) {
        alg_temp_protect_fsm_trans_to_state(handler, ALG_TEMP_PROTECT_STATE_INIT);
    }

    handler->is_fsm_attached = OSAL_FALSE;

    return;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_init
 功能描述  : 过温保护模块初始化
******************************************************************************/
osal_void alg_temp_protect_init(osal_void)
{
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;

    /* 算法注册 */
    if (hmac_alg_register(HMAC_ALG_ID_TEMP_PROTECT) != OAL_SUCC) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("alg temp unregister!\n");
#endif
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_init:: alg temp unregister.}");
        return;
    }
    memset_s(temp_protect, sizeof(alg_temp_protect_stru), 0, sizeof(alg_temp_protect_stru));

    /* 私有结构体赋值 */
    temp_protect->times_th = ALG_TEMP_PROTECT_TIMES_TH_SAFE;
    temp_protect->pre_set_duty_cyc = ALG_TEMP_PROTECT_DUTY_CYC_MAX;
    temp_protect->times_set = ALG_TEMP_PROTECT_TIMES_TH_INIT;
    temp_protect->over_th_set = ALG_TEMP_PROTECT_OVER_TH;

    temp_protect->temp_pro_aggr_size = ALG_TEMP_PROTECT_MAX_AGGR_SIZE;

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT_CLDO_MODE
    /* CLDO模式温度阈值调整 */
    alg_temp_set_cldo_mode_temp_th(temp_protect);
#endif

    /* 初始化状态机 */
    alg_temp_protect_fsm_attach(temp_protect);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_TEMP_PROTECT, alg_temp_protect_config_param);
#endif
}
/******************************************************************************
 函 数 名  : alg_temp_protect_exit
 功能描述  : 过温保护模块退出函数
******************************************************************************/
osal_void alg_temp_protect_exit(osal_void)
{
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;
    osal_u32 ret;
    /* 算法去注册 */
    ret = hmac_alg_unregister(HMAC_ALG_ID_TEMP_PROTECT);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{alg_temp_protect_exit::unregister fail}");
        return;
    }
    /* 删定时器 状态机     */
    alg_temp_protect_fsm_detach(temp_protect);
    frw_destroy_timer_entry(&temp_protect->temp_protect_timer);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_TEMP_PROTECT);
#endif
}

/******************************************************************************
 函 数 名  : alg_temp_protect_cur_temp_get
 功能描述  : 过温保护获取当前温度
******************************************************************************/
osal_u32 alg_temp_protect_cur_temp_get(alg_temp_protect_stru *temp_protect, osal_s16 *temperature)
{
    /* 检测温度 cur_temp */
    if (osal_unlikely((temp_protect->temp_debug_flag == OSAL_TRUE) &&
       (temp_protect->temp_debug_temperature != HMAC_TEMP_PROTECT_INVALID_TEMP))) {
        /* 开启debug开关,且已经设定合理温度后,不再从芯片读取温度,用当前设定温度 */
        *temperature = temp_protect->temp_debug_temperature;
    } else if (osal_unlikely(hal_read_max_temperature(temperature) != OAL_SUCC)) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

osal_void alg_temp_protect_data_clear(osal_void)
{
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;
    temp_protect->accumulate_temp = 0;
    temp_protect->times = 0;
}

osal_u32 alg_temp_protect_timer_hander(osal_void *arg)
{
    osal_s16 cur_temp = 0;
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;
    hmac_temp_pri_custom_stru *temp_pri_custom = OSAL_NULL;

    /* 不同状态温度门限获取 */
    temp_pri_custom = hmac_get_pri_custom();
    unref_param(arg);

    if (temp_protect->times < (temp_protect->times_th)) {
        if (alg_temp_protect_cur_temp_get(temp_protect, &cur_temp) != OAL_SUCC) {
            return OAL_FAIL;
        }
        temp_protect->accumulate_temp = temp_protect->accumulate_temp + cur_temp;
        temp_protect->times = temp_protect->times + 1;
    } else {
        /* times_th 0 校验 */
        if (osal_unlikely(temp_protect->times_th == 0)) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_timer_hander::times_th equals to 0.}");
            return OAL_FAIL;
        }
        temp_protect->ave_temp = (osal_s16)(temp_protect->accumulate_temp / (temp_protect->times_th));
        if (temp_protect->ave_temp >= ALG_TEMP_PROTECT_OVER_TH) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_temp_protect_timer_hander::temperature over %d}",
                temp_protect->ave_temp);
        }
        if (temp_pri_custom->temp_pro_enable == OSAL_TRUE) {
            alg_temp_protect_fsm_post_event(temp_protect, ALG_TEMP_PROTECT_EVENT_TIMER, 0, OSAL_NULL);
        }
        alg_temp_protect_data_clear();
    }
    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_temp_protect_duty_cyc_get
 功能描述  : 获取当前的占空比
******************************************************************************/
osal_u32 alg_temp_protect_duty_cyc_get(const hal_to_dmac_device_stru *hal_device)
{
    return hal_device->duty_ratio;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
osal_void alg_temp_protect_info_get(osal_void)
{
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;
    osal_u8 idx;
    unref_param(temp_protect);

    /* 打印所有20条记录 */
#ifdef _PRE_WLAN_ALG_UART_PRINT
    wifi_printf("-------------------------------------------------------\n");
    for (idx = 0; idx < ALG_TEMP_PROTECT_RECORD_AMOUNT; idx++) {
        /* sdt工具暂时无法使用 临时使用串口打印 */
        wifi_printf("%u ave_temp=%d,cur_state=%u,duty_set=%u,duty_cur=%u,",
            idx,
            temp_protect->stat_record[idx].ave_temp,
            temp_protect->stat_record[idx].state,
            temp_protect->stat_record[idx].pre_set_duty_cyc,
            temp_protect->stat_record[idx].cur_duty_cyc);
        wifi_printf("sche_reduce=%u", temp_protect->stat_record[idx].schedule_reduce);
        wifi_printf("aggr_size=%u,reduce_pwr=%u,pa_off=%u,cur_sample_times=%u\n",
            temp_protect->stat_record[idx].temp_pro_aggr_size,
            temp_protect->stat_record[idx].reduce_pwr_enable,
            temp_protect->stat_record[idx].is_pa_off,
            temp_protect->stat_record[idx].times_th);
    }
#endif
    oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_info_get:---------------------------------------}");
    for (idx = 0; idx < ALG_TEMP_PROTECT_RECORD_AMOUNT; idx++) {
        oam_warning_log1(0, OAM_SF_ANY, "{idx = %u}", idx);
        oam_warning_log4(0, OAM_SF_ANY, "{ave_temp=%d,cur_state=%u,duty_set=%u,duty_cur=%u,}",
            temp_protect->stat_record[idx].ave_temp,
            temp_protect->stat_record[idx].state,
            temp_protect->stat_record[idx].pre_set_duty_cyc,
            temp_protect->stat_record[idx].cur_duty_cyc);
        oam_warning_log1(0, OAM_SF_ANY, "{sche_reduce=%u}", temp_protect->stat_record[idx].schedule_reduce);
        oam_warning_log4(0, OAM_SF_ANY, "aggr_size=%u, reduce_pwr=%u, pa_off=%u, cur_sample_times=%u",
            temp_protect->stat_record[idx].temp_pro_aggr_size,
            temp_protect->stat_record[idx].reduce_pwr_enable,
            temp_protect->stat_record[idx].is_pa_off,
            temp_protect->stat_record[idx].times_th);
    }
}

/*****************************************************************************
 功能描述  : 算法参数配置命令
 *****************************************************************************/
osal_void alg_temp_protect_config_temp_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param)
{
    if ((temp_protect->temp_debug_flag == OSAL_TRUE) &&
        (alg_param->value > 0) && (alg_param->value < HMAC_TEMP_PROTECT_TSENSOR_MAX)) {
        temp_protect->temp_debug_temperature = (osal_u8)alg_param->value;
        /* 配置设定当前温度 清除旧数据 定时器重新开始采样  */
        temp_protect->times = 0;
        temp_protect->accumulate_temp = 0;
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("temp set success\n");
#endif
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_temp_set::temp set success}");
        return;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_over_set::out of range}");
    temp_protect->temp_debug_temperature = HMAC_TEMP_PROTECT_INVALID_TEMP;
    return;
}

osal_void alg_temp_protect_config_debug_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param)
{
    if ((alg_param->value) == OSAL_TRUE) {
        temp_protect->temp_debug_flag = OSAL_TRUE;
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_debug_set::on}");
        return;
    }
    oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_debug_set::off}");
    temp_protect->temp_debug_flag = OSAL_FALSE;
    return;
}

osal_void alg_temp_protect_config_times_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param)
{
    temp_protect->times_set = (osal_u8)alg_param->value;
    if (temp_protect->times_set != temp_protect->times_th) {
        temp_protect->times_th = temp_protect->times_set;
        /* 配置设定采样周期 清除旧数据 定时器重新开始采样  */
        alg_temp_protect_data_clear();
        oam_warning_log1(0, OAM_SF_ANY, "{alg_temp_protect_config_times_set::times_th=%d}", temp_protect->times_th);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_times_set::out of range}");
    }
}

osal_void alg_temp_protect_config_paoff_set(const alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param, hmac_temp_pri_custom_stru *temp_custom)
{
    if ((alg_param->value > temp_protect->over_th_set) &&
    (alg_param->value <= ALG_TEMP_PROTECT_PA_OFF_TH)) {
        temp_custom->temp_pro_pa_off_th = (osal_s16)alg_param->value;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_paoff_set::out of range}");
    }
}

osal_void alg_temp_protect_config_over_set(alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param, hmac_temp_pri_custom_stru *temp_pri_custom)
{
    if ((alg_param->value > (osal_u32)temp_pri_custom->temp_pro_safe_th) &&
        (alg_param->value < (osal_u32)temp_pri_custom->temp_pro_pa_off_th)) {
        temp_protect->over_th_set = (osal_u16)alg_param->value;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_over_set::out of range}");
    }
}

osal_void alg_temp_protect_config_safe_set(const alg_temp_protect_stru *temp_protect,
    const mac_ioctl_alg_param_stru *alg_param,
    hmac_temp_pri_custom_stru *temp_pri_custom)
{
    if (alg_param->value < temp_protect->over_th_set) {
        temp_pri_custom->temp_pro_safe_th = (osal_s8)alg_param->value;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_safe_set::out of range}");
    }
}

osal_u32 alg_temp_protect_config_param(hmac_vap_stru *vap, frw_msg *msg)
{
    hmac_temp_pri_custom_stru *temp_pri_custom = OSAL_NULL;
    alg_temp_protect_stru *temp_protect = &g_hmac_temp_protect;
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_TEMP_PROTECT_START) ||
        (alg_param->alg_cfg >= MAC_ALG_CFG_TEMP_PROTECT_END)) {
        return OAL_SUCC;
    }

    /* 不同状态温度门限获取 */
    temp_pri_custom = hmac_get_pri_custom();

    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_TEMP_PROTECT_GET:
            alg_temp_protect_info_get();
            break;
        case MAC_ALG_CFG_TEMP_PROTECT_SAFE_TH_SET:
            alg_temp_protect_config_safe_set(temp_protect, alg_param, temp_pri_custom);
            break;
        case MAC_ALG_CFG_TEMP_PROTECT_OVER_TH_SET:
            alg_temp_protect_config_over_set(temp_protect, alg_param, temp_pri_custom);
            break;
        case MAC_ALG_CFG_TEMP_PROTECT_PAOFF_TH_SET:
            alg_temp_protect_config_paoff_set(temp_protect, alg_param, temp_pri_custom);
            break;
        case MAC_ALG_CFG_TEMP_PROTECT_TIMES:
            alg_temp_protect_config_times_set(temp_protect, alg_param);
            break;
        case MAC_ALG_CFG_TEMP_PROTECT_DEBUG:
            alg_temp_protect_config_debug_set(temp_protect, alg_param);
            break;
        case MAC_ALG_CFG_TEMP_PROTECT_TEMP_SET:
            alg_temp_protect_config_temp_set(temp_protect, alg_param);
            break;
        default:
            oam_warning_log0(0, OAM_SF_ANY, "{alg_temp_protect_config_param::invalid parameters!}");
            break;
    }
#ifdef _PRE_WLAN_ALG_UART_PRINT
    wifi_printf("param_value:%u after set:safe_th:%d,over_th:%d,pa_off_th:%d debug_on:%u debug_temp:%u \n",
        alg_param->value, temp_pri_custom->temp_pro_safe_th, temp_protect->over_th_set,
        temp_pri_custom->temp_pro_pa_off_th, temp_protect->temp_debug_flag, temp_protect->temp_debug_temperature);
#endif
    oam_warning_log2(0, OAM_SF_ANY,
        "{alg_temp_protect_config_param::param_cfg: %d, param_value:%u", alg_param->alg_cfg, alg_param->value);
    unref_param(vap);
    return OAL_SUCC;
}
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_TEMP_PRO */
