/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: algorithm intf det
 */

#ifdef _PRE_WLAN_FEATURE_INTF_DET

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_intf_det.h"
#include "hal_chip.h"
#include "alg_main.h"
#include "hal_rf.h"
#include "hmac_scan.h"
#include "wlan_spec.h"
#include "diag_log_common.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_ap.h"
#include "hal_ext_if.h"
#include "alg_common_rom.h"
#include "hmac_alg_notify.h"
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
#include "hmac_sta_pm.h"
#endif
#include "frw_util.h"
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
#include "alg_cca_optimize.h"
#endif
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
#include "alg_edca_opt.h"
#include "hmac_dfr.h"
#include "alg_gla.h"
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "msg_btcoex_rom.h"
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
#include "hmac_obss_ap.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_INTF_DET_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_INTF_DET_CYCLE_MS 1000 /* 干扰检测周期，暂定1000ms内触发一次 */

/* 干扰检测模式,每个bit表示一种模式 */
#define ALG_INTF_DET_MODE_COCHANNEL BIT0  /* 同频干扰检测 */
#define ALG_INTF_DET_MODE_ADJCHANNEL BIT1 /* 邻频干扰检测 */

/* 同频干扰识别参数 */
#define ALG_INTF_DET_NONDIR_RATIO_THR_STA 150   /* sta模式下判别同频干扰nondir占空比门限(千分之x) */
#define ALG_INTF_DET_NOINTF_RATIO_THR_STA 60    /* sta模式下判别同频无干扰占空比门限(千分之x) */

#define ALG_INTF_DET_NONDIR_RATIO_THR_LIMIT 900  /* 判断同频干扰的non-direct占空比上限(千分之x) */
#define ALG_INTF_DET_RX_DIRECT_RATIO_THR_STA 200 /* sta模式下动态判别门限的rx比值门限(千分之x) */

#define ALG_INTF_DET_RATIO_MULTIPLE 1000      /* 比值放大倍数 */
#define ALG_INTF_DET_SMTH_SHIFT_BIT 2         /* 平滑系数shift bit */
#define ALG_INTF_DET_COL_RATIO_SMOOTH_SCALE 2 /* 冲突因子的平滑系数 */
#define ALG_INTF_DET_GOODPUT_SMOOTH_SCALE 2   /* goodput的平滑系数 */

#define ALG_INTF_DET_INTF_CYCLE_NUM_STA 4    /* sta mode连续4个周期干扰检测 */
#define ALG_INTF_DET_NO_INTF_CYCLE_NUM_STA 8 /* sta mode连续8个周期无干扰检测 */
#define ALG_INTF_DET_TX_RATIO_THR_STA 5      /* sta模式下判别同频干扰的tx time占空比门限(千分之x) */
/* 邻频、叠频干扰识别参数 统计rx时间时会计算phy头，导致干扰繁忙度偏小，故干扰检测门限减半 */
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
#define ALG_INTF_DET_ADJINTF_RATIO_TH 60 /* 判断是否存在强邻频、叠频干扰的繁忙度阈值(千分之x) */
#else
#define ALG_INTF_DET_ADJINTF_RATIO_TH 100 /* 判断是否存在强邻频、叠频干扰的繁忙度阈值(千分之x) */
#endif
#define ALG_INTF_DET_11BG_ADJINTF_RATIO_TH 100   /* 11bg模式下判断是否存在强邻频、叠频干扰繁忙度(千分之x) */
#define ALG_INTF_DET_ADJINTF_SYNC_ERR_TH 100     /* 判断是否存在邻频、叠频干扰的sync error阈值(千分之x) */
#define ALG_INTF_DET_CERTIFY_RATIO_TH 500        /* 判断是否处于认证模式的干扰繁忙度 */
#define ALG_INTF_DET_NOADJINTF_RATIO_TH 20       /* 连续N个周期内判断无干扰的门限 */
#define ALG_INTF_DET_NOADJINTF_11BG_RATIO_TH 20 /* 11bg模式下连续N个周期内判断无干扰的门限 */

#define ALG_INTF_DET_AVER_RSSI_TH (-67) /* 判断是否存在邻频、叠频干扰的pri20/40/80的底噪阈值 */

#define ALG_INTF_DET_INTF_CYCLE_TH 3        /* 连续干扰周期数 */
#define ALG_INTF_DET_NOCERT_INTF_CYCLE_TH 3 /* 连续无认证干扰周期数 */
#define ALG_INTF_DET_NOINTF_CYCLE_TH 5      /* 判断连续无干扰周期数 */
#define ALG_INTF_DET_CHN_SCAN_CYC 150       /* 信道扫描的时间:ms */
#define ALG_INTF_DET_IDLE_CNT_TH 7          /* 判断是否计算平均RSSI的空闲功率非0值的次数门限 */
#define ALG_INTF_DET_SERIAL_DEBUG 2          /* 串口打印 */

/* 负增益 */
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
#define ALG_INTF_DET_DEBUG_MODE 1     /* debug调试宏 */
#else
#define ALG_INTF_DET_DEBUG_MODE 0     /* debug调试宏 */
#endif

#define ALG_INTF_DET_ADJ_RATIO_SHIFT 10 /* 计算邻频干扰繁忙度的扩大因子 */
#define ALG_INTF_DET_PERIOD_SHIFT 10 /* 计算邻频干扰繁忙度的扩大因子 */

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
#ifdef BOARD_FPGA_WIFI
#define ALG_INTF_DET_QUICK_CYCLE_MS 1000  /* 干扰繁忙度负增益检测周期 */
#else
#define ALG_INTF_DET_QUICK_CYCLE_MS 250  /* 干扰繁忙度负增益检测周期 */
#endif
#define ALG_INTF_DET_CMP_RATIO_TH 20     /* 没有负增益的变化区间 */
#endif

#define ALG_INTF_DET_MAX_ADJUST_CNT 4   /* 干扰检测周期调整计数 */
#define ALG_INTF_DET_ADJINTF_RSSI_DIFF 8 /* 叠频干扰下不同带宽rssi差值门限 */
#define ALG_INTF_DET_ADJINTF_RSSI_THRES (-100) /* 叠频干扰下rssi干扰检测门限 */
#define ALG_INTF_DET_MAX_CYCLE_MS 3000 /* 干扰检测周期最大值，暂定3000ms */
#define ALG_INTF_ADJ_CNT_TH 3 /* 判断是否存在次20M干扰的次数门限 */
#define ALG_INTF_ADJ_MIN_TH 100 /* 强干扰下邻频、叠频干扰繁忙度最小门限 */
#define ALG_INTF_ADJ_MAX_TH 800 /* 强干扰下邻频、叠频干扰繁忙度最大门限 */
#define ALG_INTF_DET_MAX_INTF_DUTY_CNT 10 /* 干扰繁忙度最大平滑个数 */

#define ALG_INTF_DET_SCAN_CHANNEL_STATICS_PERIOD_US 15 /* MAC信道负载统计周期 15ms */
#define ALG_INTF_DET_SCAN_CHANNEL_MEAS_PERIOD_MS 15    /* PHY空闲信道估计窗长15ms(最大15ms) */
/******************************************************************************
  3 函数声明
******************************************************************************/
/* 同频干扰检测相关 */
OSAL_STATIC osal_void alg_intf_det_coch_identify(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det);

/* 邻频叠频干扰检测相关 */
OSAL_STATIC osal_void alg_intf_det_scan_chn_cb(const osal_void *param);
OSAL_STATIC osal_u32 alg_intf_det_set_cca_stat(mac_scan_req_stru *scan_params, const hmac_vap_stru *hmac_vap,
    const alg_intf_det_stru *intf_det);
OSAL_STATIC osal_void alg_intf_det_prepare_adjch_param(hal_to_dmac_device_stru *hal_device,
    alg_intf_det_stru *intf_det);
OSAL_STATIC osal_u32 alg_intf_det_idy_adjch_intf(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det);
OSAL_STATIC osal_void alg_intf_det_adjch_stat_handler(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det);

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/* 状态机相关函数声明 */
OSAL_STATIC osal_void alg_intf_det_pk_mode_update(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 is_pk_mode);
OSAL_STATIC osal_void alg_intf_det_threshold_reset(alg_intf_det_stru *intf_det);
OSAL_STATIC osal_void alg_intf_det_state_trans(hal_to_dmac_device_stru *hal_device, alg_intf_det_handler_stru *handler,
    alg_intf_det_state_enum_uint8 state, hal_alg_intf_det_mode_enum_uint8 intf_type, osal_u16 event);
OSAL_STATIC osal_void alg_intf_det_pkadj_mode(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det);
OSAL_STATIC osal_void alg_intf_det_state_nointf_entry(osal_void *ctx);
OSAL_STATIC osal_u32 alg_intf_det_intf_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_void alg_intf_det_state_detintf_entry(osal_void *ctx);
OSAL_STATIC osal_void alg_intf_det_state_pkadj_entry(osal_void *ctx);
OSAL_STATIC osal_u32 alg_intf_det_state_pkadj_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
OSAL_STATIC osal_void alg_intf_det_state_pk_entry(osal_void *ctx);
OSAL_STATIC osal_u32 alg_intf_det_state_pk_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len,
    osal_void *event_data);
#endif
OSAL_STATIC osal_u32 alg_intf_det_start_det_msg(const mac_scan_req_stru *scan_req_params);
OSAL_STATIC osal_u32 alg_intf_det_timer_handler(osal_void *void_code);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_u32 alg_intf_det_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
/******************************************************************************
  4 全局变量定义
******************************************************************************/
/**********************************************
    device级intf_det结构体
**********************************************/
alg_intf_det_stru g_intf_det;
#define STATE_TRANSFORM
/* 干扰检测状态机函数表 */
oal_fsm_state_info  g_alg_intf_det_fsm_info[] = {
    {
        ALG_INTF_DET_STATE_NOINTF,
        (osal_char *)"NOINTF",
        alg_intf_det_state_nointf_entry,
        OSAL_NULL,
        alg_intf_det_intf_event,
    },

    {
        ALG_INTF_DET_STATE_DETINTF,
        (osal_char *)"DETINTF",
        alg_intf_det_state_detintf_entry,
        OSAL_NULL,
        alg_intf_det_intf_event,
    },

    {
        ALG_INTF_DET_STATE_PKADJ,
        (osal_char *)"PKADJ",
        alg_intf_det_state_pkadj_entry,
        OSAL_NULL,
        alg_intf_det_state_pkadj_event,
    },

    {
        ALG_INTF_DET_STATE_PK,
        (osal_char *)"PK",
        alg_intf_det_state_pk_entry,
        OSAL_NULL,
        alg_intf_det_state_pk_event,
    },
};

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/******************************************************************************
 功能描述  : 干扰类型判断
******************************************************************************/
OSAL_STATIC uint8_t alg_intf_adjch_mode_check(uint8_t adj_intf_mode)
{
    return (adj_intf_mode > HAL_ALG_INTF_DET_ADJINTF_NO && adj_intf_mode < HAL_ALG_INTF_DET_STATE_PKADJ) ?
        OAL_TRUE : OAL_FALSE;
}

/* 增加信道检测的周期惩罚策略，如果无干扰且非快速检测时增大周期，最大为3000ms；如果有干扰恢复为1000ms */
OSAL_STATIC osal_void alg_intf_det_adjust_cycle(alg_intf_det_stru *intf_det)
{
    uint16_t cycle_ms = intf_det->intf_det_cycle_ms;

    // 1.有邻频干扰直接退出计数
    if ((alg_intf_adjch_mode_check(intf_det->adjch_intf_info.intf_type) == OSAL_TRUE) ||
        (cycle_ms < ALG_INTF_DET_CYCLE_MS)) {
        intf_det->adjust_cycle_cnt = 0;
        return;
    }
    // 2.有同频干扰恢复至1s
    if (intf_det->coch_intf_info.coch_intf_state_sta == OAL_TRUE) {
        intf_det->adjust_cycle_cnt = 0;
        intf_det->intf_det_cycle_ms = ALG_INTF_DET_CYCLE_MS;
        frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, intf_det->intf_det_cycle_ms,
            (osal_void *)(intf_det), OSAL_TRUE);
        return;
    }
    // 3.计数达到门限后进行惩罚操作
    intf_det->adjust_cycle_cnt++;
    if (intf_det->adjust_cycle_cnt >= ALG_INTF_DET_MAX_ADJUST_CNT) {
        intf_det->adjust_cycle_cnt = 0;
        cycle_ms = OAL_MIN(ALG_INTF_DET_MAX_CYCLE_MS, (uint16_t)(cycle_ms << 1));
        intf_det->intf_det_cycle_ms = cycle_ms;
        frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, intf_det->intf_det_cycle_ms,
            (osal_void *)(intf_det), OSAL_TRUE);
    }
}

/*
 * 功能描述   : 根据干扰类型设置状态机
 * 1.日    期   : 2022年7月11日
 *   修改内容   : 新生成函数
 */
OSAL_STATIC alg_intf_det_state_enum_uint8 alg_intfmode_get_statemode(hal_alg_intf_det_mode_enum_uint8 cur_intf_mode)
{
    alg_intf_det_state_enum_uint8 fsm_state = ALG_INTF_DET_STATE_BUTT;
    switch (cur_intf_mode) {
        case HAL_ALG_INTF_DET_ADJINTF_NO:
            fsm_state = ALG_INTF_DET_STATE_NOINTF;
            break;
        case HAL_ALG_INTF_DET_ADJINTF_MEDIUM:
        case HAL_ALG_INTF_DET_ADJINTF_STRONG:
        case HAL_ALG_INTF_DET_ADJINTF_CERTIFY:
            fsm_state = ALG_INTF_DET_STATE_DETINTF;
            break;
        case HAL_ALG_INTF_DET_STATE_PKADJ:
            fsm_state = ALG_INTF_DET_STATE_PKADJ;
            break;
        case HAL_ALG_INTF_DET_STATE_PK:
            fsm_state = ALG_INTF_DET_STATE_PK;
            break;
        default:
            oam_error_log1(0, OAM_SF_ANTI_INTF, "{alg_intfmode_get_statemode:wrong mode [%d] }", cur_intf_mode);
            break;
    }
    return fsm_state;
}

/******************************************************************************
 功能描述  : nointf状态的entry接口
******************************************************************************/
osal_void alg_intf_det_state_nointf_entry(osal_void *ctx)
{
    alg_intf_det_handler_stru *intf_det_handler = (alg_intf_det_handler_stru *)ctx;
    alg_intf_det_stru *intf_det = intf_det_handler->oal_fsm.p_oshandler;

    intf_det->adjch_intf_stat.non_intf_cyc_cnt = intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_NO];
    intf_det->adjch_intf_stat.strong_intf_cyc_cnt = 0;
    intf_det->adjch_intf_stat.middle_intf_cyc_cnt = 0;

    intf_det->pk_mode_stage = HAL_ALG_INTF_PK_STAGE_BUTT;
    intf_det->need_recover = OSAL_FALSE;

    /* 优化为统一接口 */
    intf_det_handler->curr_mode_cnt = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_NOINTF].duty_cyc_ratio_20 = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_NOINTF].duty_cyc_ratio_40 = 0;
    (osal_void)memset_s(&(intf_det->last_10_times_intf_duty_cyc[ALG_INTF_DET_STATE_NOINTF]),
        sizeof(alg_intf_det_intf_duty_cyc_info_stru), 0, sizeof(alg_intf_det_intf_duty_cyc_info_stru));

    if (intf_det_handler->is_fsm_attached == OSAL_TRUE) {
        intf_det->intf_det_cycle_ms = ALG_INTF_DET_CYCLE_MS;
        frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, intf_det->intf_det_cycle_ms,
            (osal_void *)(intf_det), OSAL_TRUE);
    }
}
/******************************************************************************
 功能描述  : nointf和deintf状态event处理
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_event_proc(osal_u16 event, hal_to_dmac_device_stru *hal_device,
    alg_intf_det_stru *intf_det, alg_intf_det_handler_stru *intf_det_handler)
{
    switch (event) {
        case ALG_INTF_DET_EVENT_DUTY_CYCLE:
            alg_intf_det_idy_adjch_intf(hal_device, intf_det);
            alg_intf_det_adjust_cycle(intf_det);
            break;
        case ALG_INTF_DET_EVENT_HOST_PK:
            alg_intf_det_state_trans(hal_device, intf_det_handler, ALG_INTF_DET_STATE_PKADJ,
                HAL_ALG_INTF_DET_STATE_PKADJ, ALG_INTF_DET_EVENT_HOST_PK);
            break;
        default:
            break;
    }
}

/******************************************************************************
 功能描述  : 干扰检测状态的event接口
******************************************************************************/
osal_u32 alg_intf_det_intf_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    alg_intf_det_stru *intf_det = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_intf_det_handler_stru *intf_det_handler = (alg_intf_det_handler_stru *)ctx;
    unref_param(event_data_len);
    unref_param(event_data);

    if (osal_unlikely(intf_det_handler == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_intf_event::intf_det_handler null.}");
        return OAL_FAIL;
    }

    intf_det = intf_det_handler->oal_fsm.p_oshandler;
    if (osal_unlikely(intf_det == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_intf_event::intf_det null.}");
        return OAL_FAIL;
    }

    hal_device = intf_det->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_intf_event::hal_device null.}");
        return OAL_FAIL;
    }

    alg_intf_det_event_proc(event, hal_device, intf_det, intf_det_handler);
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : detintf状态的entry接口
******************************************************************************/
osal_void alg_intf_det_state_detintf_entry(osal_void *ctx)
{
    alg_intf_det_handler_stru *intf_det_handler = (alg_intf_det_handler_stru *)ctx;
    alg_intf_det_stru *intf_det = intf_det_handler->oal_fsm.p_oshandler;

    intf_det_handler->curr_mode_cnt = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_DETINTF].duty_cyc_ratio_20 = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_DETINTF].duty_cyc_ratio_40 = 0;
    (osal_void)memset_s(&(intf_det->last_10_times_intf_duty_cyc[ALG_INTF_DET_STATE_DETINTF]),
        sizeof(alg_intf_det_intf_duty_cyc_info_stru), 0, sizeof(alg_intf_det_intf_duty_cyc_info_stru));

    if (intf_det->adjch_intf_info.intf_type == HAL_ALG_INTF_DET_ADJINTF_CERTIFY) {
        intf_det->adjch_intf_stat.cert_intf_cyc_cnt = 0;
        intf_det->adjch_intf_stat.non_intf_cyc_cnt = 0;
        intf_det->adjch_intf_stat.strong_intf_cyc_cnt = 0;
        intf_det->adjch_intf_stat.middle_intf_cyc_cnt = 0;
    }
    /* 再次进入该状态，惩罚的门限阈值恢复到默认态 */
    alg_intf_det_threshold_reset(intf_det);

    /* 刚进入需要快速探测 */
    intf_det->intf_det_cycle_ms = ALG_INTF_DET_QUICK_CYCLE_MS;
    frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, intf_det->intf_det_cycle_ms,
        (osal_void *)(intf_det), OSAL_TRUE);
}

/******************************************************************************
 功能描述  : pkadj状态的entry接口
******************************************************************************/
osal_void alg_intf_det_state_pkadj_entry(osal_void *ctx)
{
    alg_intf_det_handler_stru *intf_det_handler = (alg_intf_det_handler_stru *)ctx;
    alg_intf_det_stru *intf_det = intf_det_handler->oal_fsm.p_oshandler;

    intf_det_handler->curr_mode_cnt = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PKADJ].duty_cyc_ratio_20 = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PKADJ].duty_cyc_ratio_40 = 0;
    (osal_void)memset_s(&(intf_det->last_10_times_intf_duty_cyc[ALG_INTF_DET_STATE_PKADJ]),
        sizeof(alg_intf_det_intf_duty_cyc_info_stru), 0, sizeof(alg_intf_det_intf_duty_cyc_info_stru));

    intf_det->pk_mode_stage = HAL_ALG_INTF_PKADJ_STAGE_INIT;
    intf_det->need_recover = OSAL_FALSE;

    /* 刚进入需要快速探测 */
    intf_det->intf_det_cycle_ms = ALG_INTF_DET_QUICK_CYCLE_MS;
    frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, intf_det->intf_det_cycle_ms,
        (osal_void *)(intf_det), OSAL_TRUE);
}

/******************************************************************************
 功能描述  : pkadj状态的event接口
******************************************************************************/
osal_u32 alg_intf_det_state_pkadj_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    alg_intf_det_stru *intf_det = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_intf_det_handler_stru *intf_det_handler = (alg_intf_det_handler_stru *)ctx;
    alg_intf_det_state_enum_uint8 cur_state = ALG_INTF_DET_STATE_NOINTF;
    unref_param(event_data);
    unref_param(event_data_len);

    if (osal_unlikely(intf_det_handler == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_state_pkadj_event::intf_det_handler null.}");
        return OAL_FAIL;
    }

    intf_det = intf_det_handler->oal_fsm.p_oshandler;
    if (osal_unlikely(intf_det == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_state_pkadj_event::intf_det null.}");
        return OAL_FAIL;
    }

    hal_device = intf_det->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_state_pkadj_event::hal_device null.}");
        return OAL_FAIL;
    }

    switch (event) {
        case ALG_INTF_DET_EVENT_HOST_NPK:
            cur_state = alg_intfmode_get_statemode(intf_det->adjch_intf_info.intf_type);
            alg_intf_det_state_trans(hal_device, intf_det_handler, cur_state,
                                     intf_det->adjch_intf_info.intf_type, ALG_INTF_DET_EVENT_HOST_NPK);
            break;
        case ALG_INTF_DET_EVENT_DUTY_CYCLE:
            alg_intf_det_pkadj_mode(hal_device, intf_det);
            alg_intf_det_idy_adjch_intf(hal_device, intf_det);
            alg_intf_det_adjust_cycle(intf_det);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : pk状态的entry接口
******************************************************************************/
osal_void alg_intf_det_state_pk_entry(osal_void *ctx)
{
    alg_intf_det_handler_stru *intf_det_handler = (alg_intf_det_handler_stru *)ctx;
    alg_intf_det_stru *intf_det = intf_det_handler->oal_fsm.p_oshandler;

    intf_det_handler->curr_mode_cnt = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PK].duty_cyc_ratio_20 = 0;
    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PK].duty_cyc_ratio_40 = 0;
    (osal_void)memset_s(&(intf_det->last_10_times_intf_duty_cyc[ALG_INTF_DET_STATE_PK]),
        sizeof(alg_intf_det_intf_duty_cyc_info_stru), 0, sizeof(alg_intf_det_intf_duty_cyc_info_stru));
}

/******************************************************************************
 功能描述  : pk状态的event接口
******************************************************************************/
osal_u32 alg_intf_det_state_pk_event(osal_void *ctx, osal_u16 event, osal_u16 event_data_len, osal_void *event_data)
{
    alg_intf_det_stru *intf_det = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_intf_det_handler_stru *intf_det_handler = (alg_intf_det_handler_stru *)ctx;
    alg_intf_det_state_enum_uint8 cur_state = ALG_INTF_DET_STATE_NOINTF;
    unref_param(event_data);
    unref_param(event_data_len);

    if (osal_unlikely(intf_det_handler == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_state_pk_event::intf_det_handler null.}");
        return OAL_FAIL;
    }

    intf_det = intf_det_handler->oal_fsm.p_oshandler;
    if (osal_unlikely(intf_det == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_state_pk_event::intf_det null.}");
        return OAL_FAIL;
    }

    hal_device = intf_det->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_intf_det_state_pk_event::hal_device null.}");
        return OAL_FAIL;
    }

    switch (event) {
        /* ATTENTION: 不处理的事件删除，是否需要在上报事件的地方，直接将该状态排除 */
        case ALG_INTF_DET_EVENT_DUTY_CYCLE:
            alg_intf_det_idy_adjch_intf(hal_device, intf_det);
            alg_intf_det_adjust_cycle(intf_det);
            oam_info_log0(0, OAM_SF_ANY, "{alg_intf_det_state_pk_event::do nothing.}");
            break;

        case ALG_INTF_DET_EVENT_HOST_NPK:
            cur_state = alg_intfmode_get_statemode(intf_det->adjch_intf_info.intf_type);
            alg_intf_det_state_trans(hal_device, intf_det_handler, cur_state,
                                     intf_det->adjch_intf_info.intf_type, ALG_INTF_DET_EVENT_HOST_NPK);
            break;

        default:
            break;
    }
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 同频干扰类型改变发送到dmac
******************************************************************************/
OSAL_STATIC osal_u32 alg_intf_det_send_intf_type_msg(alg_intf_det_notify_info_stru *intf_det_notify)
{
    osal_u32 ret = OAL_SUCC;
    frw_msg msg = {0};
    msg.data = (osal_u8 *)intf_det_notify;
    msg.data_len = (osal_u16)sizeof(alg_intf_det_notify_info_stru);
    /* 抛事件至Device侧alg */
    ret = (osal_u32)frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_INTF_TYPE, &msg, OSAL_FALSE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "{alg_intf_det_send_intf_type_msg:: intf type notify fail [%d].}", ret);
        return ret;
    }
    return ret;
}
/******************************************************************************
 功能描述  : 干扰类型变化通知前准备
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_intf_type_change_notify(hal_to_dmac_device_stru *hal_device,
    alg_intf_det_stru *intf_det, alg_intf_det_notify_info_stru *intf_det_notify,
    hal_alg_intf_det_mode_enum_uint8 intf_type, osal_u16 event)
{
    if (intf_type != intf_det_notify->adjch_intf_type) {
        intf_det_notify->adjch_intf_type_last = intf_det_notify->adjch_intf_type;
        intf_det_notify->adjch_intf_type = intf_type;
        intf_det_notify->intf_det_type = ALG_INTF_DET_ADJCH;
        intf_det_notify->duty_cyc_ratio_20 = intf_det->adjch_intf_stat.duty_cyc_ratio_20;
        intf_det_notify->duty_cyc_ratio_sec20 = intf_det->adjch_intf_stat.duty_cyc_ratio_sec20;
        oam_warning_log3(0, OAM_SF_ANY,
                         "{alg_intf_det_state_trans::hmac event: trans intf type from [%d] to [%d] with event[%d]}",
                         intf_det->adjch_intf_info.intf_type, intf_type, event);
#if ALG_INTF_DET_DEBUG_MODE
        if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("alg_intf_det_state_trans::hmac event: trans intf type from [%d] to [%d] with event[%d]\n}",
                intf_det->adjch_intf_info.intf_type, intf_type, event);
#endif
        }
#endif
        alg_intf_det_send_intf_type_msg(intf_det_notify);
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
        /* 通知CCA模块干扰状态变化 */
        hmac_alg_cfg_intf_det_cca_notify(hal_device, intf_det->adjch_intf_info.intf_type, intf_type);
#endif
        intf_det->adjch_intf_info.intf_type = intf_type;
    }
    return;
}
/******************************************************************************
 功能描述  : 干扰检测状态机的状态切换接口
******************************************************************************/
osal_void alg_intf_det_state_trans(hal_to_dmac_device_stru *hal_device, alg_intf_det_handler_stru *handler,
    alg_intf_det_state_enum_uint8 state, hal_alg_intf_det_mode_enum_uint8 intf_type, osal_u16 event)
{
    oal_fsm_stru *oal_fsm = &handler->oal_fsm;
    alg_intf_det_stru *intf_det = handler->oal_fsm.p_oshandler;
    alg_intf_det_notify_info_stru *intf_det_notify = &intf_det->intf_det_notify;

    if (osal_unlikely(state >= ALG_INTF_DET_STATE_BUTT)) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_intf_det_state_trans:invalid state %d}", state);
        return;
    }
#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == OSAL_TRUE) {
        oam_warning_log2(0, OAM_SF_ANY,
                         "{alg_intf_det_state_trans::hmac event: current state[%d], event[%d]}",
                         handler->oal_fsm.cur_state, event);
        oam_warning_log2(0, OAM_SF_ANY,
                         "{alg_intf_det_state_trans::hmac event: current intf type[%d], event[%d]}",
                         intf_det->adjch_intf_info.intf_type, event);
    } else if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("{alg_intf_det_state_trans::hmac event: current state[%d], event[%d]}\n",
            handler->oal_fsm.cur_state, event);
        wifi_printf("{alg_intf_det_state_trans::hmac event: current intf type[%d], event[%d]}\n",
            intf_det->adjch_intf_info.intf_type, event);
#endif
    }
#endif
    /*
        attention: 这个接口需要确认，哪个模块对前一个状态有兴趣:内部没有对前一个状态有判断的
        通知其他模块状态改变，hal的干扰状态使用情况,hook函数里面赋值
    */
    if ((event == ALG_INTF_DET_EVENT_DUTY_CYCLE) || (event == ALG_INTF_DET_EVENT_NEG_GAIN)) {
        alg_intf_det_intf_type_change_notify(hal_device, intf_det, intf_det_notify, intf_type, event);
    }
    if (state != handler->oal_fsm.cur_state) {
#if ALG_INTF_DET_DEBUG_MODE
        if (intf_det->intf_det_debug == OSAL_TRUE) {
            oam_warning_log3(0, OAM_SF_ANY, "{alg_intf_det_state_trans::hmac event: trans state from [%d] to [%d] \
                state with event[%d]}", handler->oal_fsm.cur_state, state, event);
        } else if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("alg_intf_det_state_trans::hmac event: trans state from [%d] to [%d] state with event[%d]\n",
                handler->oal_fsm.cur_state, state, event);
#endif
        }
#endif
        oal_fsm_trans_to_state(oal_fsm, state);
    }

    return;
}

/******************************************************************************
 功能描述  : 触发状态机事件处理函数
******************************************************************************/
OSAL_STATIC osal_u32 alg_intf_det_fsm_post_event(alg_intf_det_stru *intf_det, osal_u16 type, osal_u16 datalen,
    osal_u8 *data)
{
    alg_intf_det_handler_stru *handler = OSAL_NULL;
    osal_u32 ret;

    if (type >= ALG_INTF_DET_EVENT_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "alg_intf_det_fsm_post_event:: event type[%d] error, NULL!", type);
        return OAL_FAIL;
    }

    handler = &(intf_det->intf_det_handler);

    if (osal_unlikely(handler->is_fsm_attached == OSAL_FALSE)) {
        oam_warning_log1(0, OAM_SF_ANY, "alg_intf_det_fsm_post_event::intf_det_fsm_attached = %d!",
                         handler->is_fsm_attached);
        return OAL_FAIL;
    }

    ret = oal_fsm_event_dispatch(&(handler->oal_fsm), type, datalen, data);

    return ret;
}

/******************************************************************************
 功能描述  : 创建intf det handler
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_attach(alg_intf_det_stru *intf_det)
{
    alg_intf_det_handler_stru *handler = OSAL_NULL;
    osal_u8                    fsm_name[6] = {0}; /* 6:数组大小 */
    osal_u32 ret;
    oal_fsm_create_stru fsm_create_stru;
    handler = &intf_det->intf_det_handler;
    if (handler->is_fsm_attached) {
        oam_error_log0(0, OAM_SF_ANY, "{alg_intf_det_attach::device intf det fsm aready attached.}");
        return;
    }

    (osal_void)memset_s(handler, sizeof(alg_intf_det_handler_stru), 0, sizeof(alg_intf_det_handler_stru));
    fsm_create_stru.oshandle = intf_det;
    fsm_create_stru.name = fsm_name;
    fsm_create_stru.context = handler;
    handler->is_fsm_attached = OSAL_TRUE;

    ret = oal_fsm_create(&fsm_create_stru, &handler->oal_fsm, ALG_INTF_DET_STATE_NOINTF,
        g_alg_intf_det_fsm_info, sizeof(g_alg_intf_det_fsm_info) / sizeof(oal_fsm_state_info));
    if (osal_unlikely(ret != OAL_SUCC)) {
        handler->is_fsm_attached = OSAL_FALSE;
        oam_error_log0(0, OAM_SF_ANY, "alg_intf_det_attach::device intf det fsm attach failed.");
        return;
    }
}

/******************************************************************************
 功能描述  : 销毁intf det handler
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_detach(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det)
{
    alg_intf_det_handler_stru *handler = OSAL_NULL;

    handler = &intf_det->intf_det_handler;
    if (osal_unlikely(handler->is_fsm_attached == OSAL_FALSE)) {
        oam_error_log0(0, OAM_SF_ANY, "alg_intf_det_detach::device intf det fsm not attatched");
        return;
    }

    handler->is_fsm_attached = OSAL_FALSE;

    /* 不是NOINTF状态切换到NOINTF状态 */
    if (handler->oal_fsm.cur_state != ALG_INTF_DET_STATE_NOINTF) {
        alg_intf_det_state_trans(hal_device, handler, ALG_INTF_DET_STATE_NOINTF, HAL_ALG_INTF_DET_ADJINTF_NO,
                                 ALG_INTF_DET_EVENT_DETATCH);
    }

    return;
}
#endif

/******************************************************************************
  3 函数实现
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_para_init(alg_intf_det_stru *intf_det, hal_to_dmac_device_stru *hal_device)
{
    intf_det->strong_intf_ratio_th = ALG_INTF_DET_ADJINTF_RATIO_TH;
    intf_det->nonintf_ratio_th = ALG_INTF_DET_NOADJINTF_RATIO_TH;
    intf_det->intf_aveg_rssi_th = ALG_INTF_DET_AVER_RSSI_TH;

    /* 初始化部分 */
    intf_det->intf_det_mode = BIT0 | BIT1; // 默认开启同频/叠频
    intf_det->intf_det_cycle_ms = ALG_INTF_DET_CYCLE_MS;

    /* 同频干扰相关 */
    intf_det->coch_intf_info.nondir_duty_cyc_th_sta = ALG_INTF_DET_NONDIR_RATIO_THR_STA;
    intf_det->coch_intf_info.nonintf_duty_th_sta = ALG_INTF_DET_NOINTF_RATIO_THR_STA;

    /* 邻/叠频干扰相关 */
    intf_det->adjch_intf_info.chn_scan_cycle_ms = ALG_INTF_DET_CHN_SCAN_CYC;
    intf_det->adjch_intf_info.strong_intf_ratio_th = intf_det->strong_intf_ratio_th;
    intf_det->adjch_intf_info.nonintf_ratio_th = intf_det->nonintf_ratio_th;
    intf_det->adjch_intf_info.intf_sync_err_th = ALG_INTF_DET_ADJINTF_SYNC_ERR_TH;

    intf_det->adjch_intf_info.intf_aveg_rssi_th = intf_det->intf_aveg_rssi_th;

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_NO] = ALG_INTF_DET_NOINTF_CYCLE_TH;
    intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_MEDIUM] = ALG_INTF_DET_INTF_CYCLE_TH;
    intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_STRONG] = ALG_INTF_DET_INTF_CYCLE_TH;
#else
    intf_det->adjch_intf_info.non_intf_cyc_th = ALG_INTF_DET_NOINTF_CYCLE_TH;
#endif
    intf_det->hal_device = hal_device;
}

/*******************************************************************************
 功能描述  : intf det模块初始化
******************************************************************************/
osal_s32 alg_intf_det_init(osal_void)
{
    alg_intf_det_stru *intf_det = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_internal_hook_stru* callback = hmac_alg_get_notify_if();
    osal_u32 ret;

    ret = hmac_alg_register(HMAC_ALG_ID_INTF_DET);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return (osal_s32)ret;
    }

    hal_device = hal_chip_get_hal_device();
    intf_det = &g_intf_det;
    memset_s(intf_det, sizeof(alg_intf_det_stru), 0, sizeof(alg_intf_det_stru));

    alg_intf_det_para_init(intf_det, hal_device);

    frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, intf_det->intf_det_cycle_ms,
        (osal_void *)(intf_det), OSAL_TRUE);
    hmac_alg_register_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void *)intf_det);

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    /* 邻频干扰状态机初始化 */
    alg_intf_det_attach(intf_det);
#endif
#if (defined(_PRE_WLAN_FEATURE_INTF_DET) && defined(_PRE_WLAN_FEATURE_NEGTIVE_DET))
    callback->cfg_intf_det_pk_mode_notify_func = alg_intf_det_pk_mode_update;
#endif

    hmac_alg_register_scan_param_notify(alg_intf_det_start_det_msg);
    hmac_alg_register_scan_ch_complete_notify(alg_intf_det_scan_chn_cb);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_INTF_DET, alg_intf_det_config_param);
#endif
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : intf det模块退出函数
******************************************************************************/
osal_void alg_intf_det_exit(osal_void)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_intf_det_stru *intf_det = OSAL_NULL;
    alg_internal_hook_stru* callback = hmac_alg_get_notify_if();
    osal_u32 ret;

    ret = hmac_alg_unregister(HMAC_ALG_ID_INTF_DET);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return;
    }

    /* 注销钩子函数 */
#if (defined(_PRE_WLAN_FEATURE_INTF_DET) && defined(_PRE_WLAN_FEATURE_NEGTIVE_DET))
    callback->cfg_intf_det_pk_mode_notify_func = OSAL_NULL;
#endif

    hmac_alg_unregister_scan_param_notify();
    hal_device = hal_chip_get_hal_device();

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void **)&intf_det);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return;
    }

    if (osal_unlikely(intf_det == OSAL_NULL)) {
        return;
    }

    /* 停止并注销相关定时器 */
    frw_destroy_timer_entry(&intf_det->intf_det_timer);
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    alg_intf_det_detach(hal_device, intf_det);
#endif

    /* 注销并释放device级别下的结构体 */
    hmac_alg_unregister_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET);
    hmac_alg_unregister_scan_ch_complete_notify();
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_INTF_DET);
#endif
}
#define NEGTIVE_DET
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/******************************************************************************
 功能描述  : 干扰识别不同模式下的阈值加倍
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_threshold_double(alg_intf_det_stru *intf_det,
    hal_alg_intf_det_mode_enum_uint8 cur_intf_mode)
{
    switch (cur_intf_mode) {
        case HAL_ALG_INTF_DET_ADJINTF_MEDIUM:
        case HAL_ALG_INTF_DET_ADJINTF_STRONG:
            intf_det->adjch_intf_info.intf_cyc_th[cur_intf_mode] = ALG_INTF_DET_INTF_CYCLE_TH * 2; /* 2:倍数 */
            break;
        default:
            oam_error_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_threshold_double:wrong mode [%d] }", cur_intf_mode);
            break;
    }
}

/******************************************************************************
 功能描述  : 干扰识别不同模式下的阈值重设
******************************************************************************/
osal_void alg_intf_det_threshold_reset(alg_intf_det_stru *intf_det)

{
    switch (intf_det->adjch_intf_info.intf_type) {
        case HAL_ALG_INTF_DET_ADJINTF_NO:
            intf_det->adjch_intf_info.intf_cyc_th[intf_det->adjch_intf_info.intf_type] = ALG_INTF_DET_NOINTF_CYCLE_TH;
            break;
        case HAL_ALG_INTF_DET_ADJINTF_MEDIUM:
            intf_det->adjch_intf_info.intf_cyc_th[intf_det->adjch_intf_info.intf_type] = ALG_INTF_DET_INTF_CYCLE_TH;
            break;
        case HAL_ALG_INTF_DET_ADJINTF_STRONG:
            intf_det->adjch_intf_info.intf_cyc_th[intf_det->adjch_intf_info.intf_type] = ALG_INTF_DET_INTF_CYCLE_TH;
            break;
        case HAL_ALG_INTF_DET_ADJINTF_CERTIFY:
            break;
        default:
            oam_error_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_threshold_reset:wrong mode [%d] }",
                           intf_det->adjch_intf_info.intf_type);
            break;
    }
}

/******************************************************************************
 功能描述  : 干扰识别算法，切换不同模式后，收益判断函数
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_benefit_check(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det,
    hal_alg_intf_det_mode_enum_uint8 *cur_intf_mode)
{
    alg_intf_det_handler_stru *intf_det_handler;
    alg_intf_det_state_enum_uint8 cur_state;
    alg_intf_det_state_enum_uint8 prev_state;

    intf_det_handler = &intf_det->intf_det_handler;
    cur_state = intf_det_handler->oal_fsm.cur_state;
    prev_state = intf_det_handler->oal_fsm.prev_state;

    /*
       条件1、统计周期小于4，直接返回
       条件2、防止PKADJ状态下，recover由于前后连续的统计导致变化，最终状态混乱
       条件3、PKADJ模式的INIT状态，是为了统计原始的繁忙度，无需增益判断
       条件4、PKADJ模式时，负增益判断只使用4次的结果的平均值
    */
    if ((intf_det_handler->curr_mode_cnt < 4) || (intf_det->need_recover == OSAL_TRUE) || /* 4:小于4 */
        (intf_det->pk_mode_stage == HAL_ALG_INTF_PKADJ_STAGE_INIT) ||
        ((intf_det_handler->curr_mode_cnt != 4) && (cur_state == ALG_INTF_DET_STATE_PKADJ))) { /* 4:cnt值 */
        return;
    }

    /* 当前干扰模式繁忙度小于前面设置的干扰繁忙度，无需修改，返回 */
    if ((intf_det_handler->duty_cyc_info[cur_state].duty_cyc_ratio_20 <
        (intf_det_handler->duty_cyc_info[prev_state].duty_cyc_ratio_20 + ALG_INTF_DET_CMP_RATIO_TH)) &&
        (intf_det_handler->duty_cyc_info[cur_state].duty_cyc_ratio_40 <
        (intf_det_handler->duty_cyc_info[prev_state].duty_cyc_ratio_40 + ALG_INTF_DET_CMP_RATIO_TH))) {
        return;
    }

    oam_warning_log4(0, OAM_SF_ANTI_INTF, "{alg_intf_det_benefit_check:ratio_20_old = %d,ratio_20_new = %d,\
                     ratio_40_old = %d,ratio_40_new = %d.}",
        intf_det_handler->duty_cyc_info[prev_state].duty_cyc_ratio_20,
        intf_det_handler->duty_cyc_info[cur_state].duty_cyc_ratio_20,
        intf_det_handler->duty_cyc_info[prev_state].duty_cyc_ratio_40,
        intf_det_handler->duty_cyc_info[cur_state].duty_cyc_ratio_40);
#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("{alg_intf_det_benefit_check:ratio_20_old = %d, ratio_20_new = %d, ratio_40_old = %d, \
            ratio_40_new = %d.\n}",
            intf_det_handler->duty_cyc_info[prev_state].duty_cyc_ratio_20,
            intf_det_handler->duty_cyc_info[cur_state].duty_cyc_ratio_20,
            intf_det_handler->duty_cyc_info[prev_state].duty_cyc_ratio_40,
            intf_det_handler->duty_cyc_info[cur_state].duty_cyc_ratio_40);
#endif
    }
#endif
    if (intf_det->pk_mode == OSAL_TRUE) {
        intf_det->need_recover = OSAL_TRUE;
    } else {
        /* ATTENTION: 惩罚--无限次的放大后果? 状态切换会恢复 */
        alg_intf_det_threshold_double(intf_det, *cur_intf_mode);
        *cur_intf_mode = HAL_ALG_INTF_DET_ADJINTF_NO;

        /* 都切回到无干扰状态 */
        alg_intf_det_state_trans(hal_device, intf_det_handler, ALG_INTF_DET_STATE_NOINTF, HAL_ALG_INTF_DET_ADJINTF_NO,
                                 ALG_INTF_DET_EVENT_NEG_GAIN);

        oam_warning_log3(0, OAM_SF_ANTI_INTF, "{alg_intf_det_benefit_check:state_old = %d, state_new = %d, event[%d]}",
            intf_det_handler->oal_fsm.prev_state, intf_det_handler->oal_fsm.cur_state, ALG_INTF_DET_EVENT_NEG_GAIN);
    }
}
#define PK_MODE_PARAMETERS_ADJUST
/******************************************************************************
 功能描述  : 更新干扰繁忙度
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_update_duty_cycle(alg_intf_det_stru *intf_det, alg_intf_det_handler_stru *handler,
    alg_intf_det_state_enum_uint8 cur_state)
{
    osal_u8 cur_index = intf_det->last_10_times_intf_duty_cyc[cur_state].cur_index;

    if (intf_det->last_10_times_intf_duty_cyc[cur_state].total_cnt < ALG_INTF_DET_MAX_INTF_DUTY_CNT) {
        intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_20 +=
            intf_det->adjch_intf_stat.duty_cyc_ratio_20;
        intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_40 +=
            intf_det->adjch_intf_stat.duty_cyc_ratio_40;
        (intf_det->last_10_times_intf_duty_cyc[cur_state].total_cnt)++;
    } else {
        intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_20 =
            intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_20 -
            intf_det->last_10_times_intf_duty_cyc[cur_state].intf_duty_cyc_20_recoard[cur_index] +
            intf_det->adjch_intf_stat.duty_cyc_ratio_20;
        intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_40 =
            intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_40 -
            intf_det->last_10_times_intf_duty_cyc[cur_state].intf_duty_cyc_40_recoard[cur_index] +
            intf_det->adjch_intf_stat.duty_cyc_ratio_40;
    }
    intf_det->last_10_times_intf_duty_cyc[cur_state].intf_duty_cyc_20_recoard[cur_index] =
        intf_det->adjch_intf_stat.duty_cyc_ratio_20;
    intf_det->last_10_times_intf_duty_cyc[cur_state].intf_duty_cyc_40_recoard[cur_index] =
        intf_det->adjch_intf_stat.duty_cyc_ratio_40;
    (intf_det->last_10_times_intf_duty_cyc[cur_state].cur_index)++;
    intf_det->last_10_times_intf_duty_cyc[cur_state].cur_index %= ALG_INTF_DET_STATISTIC_CNT_TH;
    handler->duty_cyc_info[cur_state].duty_cyc_ratio_20 =
        (osal_u16)intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_20 /
            intf_det->last_10_times_intf_duty_cyc[cur_state].total_cnt;
    handler->duty_cyc_info[cur_state].duty_cyc_ratio_40 =
        (osal_u16)intf_det->last_10_times_intf_duty_cyc[cur_state].sum_of_intf_duty_cyc_40 /
            intf_det->last_10_times_intf_duty_cyc[cur_state].total_cnt;
#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == OSAL_TRUE) {
        oam_warning_log4(0, OAM_SF_ANY,
                         "{alg_intf_det_update_duty_cycle::current state[%d], times[%d], \
                         duty_cyc_ratio_20[%d], duty_cyc_ratio_40[%d]}",
                         cur_state, intf_det->last_10_times_intf_duty_cyc[cur_state].total_cnt,
                         handler->duty_cyc_info[cur_state].duty_cyc_ratio_20,
                         handler->duty_cyc_info[cur_state].duty_cyc_ratio_40);
    } else if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("alg_intf_det_update_duty_cycle::current state[%d], times[%d], duty_cyc_ratio_20[%d], \
            duty_cyc_ratio_40[%d]\n", cur_state, intf_det->last_10_times_intf_duty_cyc[cur_state].total_cnt,
            handler->duty_cyc_info[cur_state].duty_cyc_ratio_20,
            handler->duty_cyc_info[cur_state].duty_cyc_ratio_40);
#endif
    }
#endif
}

/******************************************************************************
 功能描述  : host下发处理函数
******************************************************************************/
osal_void alg_intf_det_pk_mode_update(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 is_pk_mode)
{
    alg_intf_det_stru *intf_det = OSAL_NULL;
    unref_param(hmac_vap);

    oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_pk_mode_update:host dispatch pk_mode = %d}", is_pk_mode);

    intf_det = &g_intf_det;

    if (is_pk_mode == OSAL_TRUE) {
        intf_det->pk_mode = OSAL_TRUE;
        alg_intf_det_fsm_post_event(intf_det, ALG_INTF_DET_EVENT_HOST_PK, 0, OSAL_NULL);
    } else {
        intf_det->pk_mode = OSAL_FALSE;
        alg_intf_det_fsm_post_event(intf_det, ALG_INTF_DET_EVENT_HOST_NPK, 0, OSAL_NULL);
    }
}

/******************************************************************************
 功能描述  : PK模式参数探测及设置机制
******************************************************************************/
osal_void alg_intf_det_pkadj_mode(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det)
{
    alg_intf_det_handler_stru *intf_det_handler = &intf_det->intf_det_handler;
    alg_intf_det_state_enum_uint8 prev_mode;

    prev_mode = intf_det_handler->oal_fsm.prev_state;

    intf_det_handler->curr_mode_cnt++;

    alg_intf_det_update_duty_cycle(intf_det, intf_det_handler, ALG_INTF_DET_STATE_PKADJ);

    /* attention:以下逻辑优化一下，分支太多 */
    if (intf_det_handler->curr_mode_cnt >= 4) { /* 4:大于或等于4 */
        /* 负增益识别 */
        alg_intf_det_benefit_check(hal_device, intf_det, &(intf_det->adjch_intf_info.intf_type));

        /* 不需要恢复的话，将当前stage的统计值，复制到prev mode，并且将stage下调到下一个阶段 */
        if (intf_det->need_recover == OSAL_FALSE) {
            /* ATTENTION:这部分是否可以移到benefit_check */
            if (intf_det_handler->curr_mode_cnt == 4) { /* 4:cnt值 */
                intf_det_handler->duty_cyc_info[prev_mode].duty_cyc_ratio_20 =
                    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PKADJ].duty_cyc_ratio_20;
                intf_det_handler->duty_cyc_info[prev_mode].duty_cyc_ratio_40 =
                    intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PKADJ].duty_cyc_ratio_40;
            }

            intf_det->pk_mode_stage++;
            intf_det->has_set = OSAL_FALSE;

            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_benefit_check: PK MODE stage change to[%d] }",
                             intf_det->pk_mode_stage);
#ifdef _PRE_WLAN_ALG_UART_PRINT
            wifi_printf("{alg_intf_det_benefit_check: PK MODE stage change to[%d], 0: TXOP, 1: AIFSN, 2: CW\n}",
                intf_det->pk_mode_stage);
#endif
            if (intf_det->pk_mode_stage != HAL_ALG_INTF_PK_STAGE_BUTT) {
                intf_det_handler->curr_mode_cnt = 0;
                intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PKADJ].duty_cyc_ratio_20 = 0;
                intf_det_handler->duty_cyc_info[ALG_INTF_DET_STATE_PKADJ].duty_cyc_ratio_40 = 0;
                (void)memset_s(&intf_det->last_10_times_intf_duty_cyc[ALG_INTF_DET_STATE_PKADJ],
                    sizeof(alg_intf_det_intf_duty_cyc_info_stru), 0, sizeof(alg_intf_det_intf_duty_cyc_info_stru));
            } else {
                /* PK模式参数调整完成之后，信道繁忙度检测恢复到初始值 */
                intf_det->intf_det_cycle_ms = ALG_INTF_DET_CYCLE_MS;
                frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler,
                    intf_det->intf_det_cycle_ms, (osal_void *)(intf_det), OSAL_TRUE);

                alg_intf_det_state_trans(hal_device, intf_det_handler, ALG_INTF_DET_STATE_PK, HAL_ALG_INTF_DET_STATE_PK,
                                         ALG_INTF_DET_EVENT_PKADJ_COMP);
            }
        }
    }
}
#endif

/******************************************************************************
 功能描述  : 干扰检测定时器到期处理函数
******************************************************************************/
osal_u32 alg_intf_det_timer_handler(osal_void *void_code)
{
    alg_intf_det_stru *intf_det = (alg_intf_det_stru *)void_code;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    if (osal_unlikely((intf_det == OSAL_NULL) || (intf_det->hal_device == OSAL_NULL))) {
        oam_error_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_timer_handler: ptr is null! %d}", intf_det == OSAL_NULL);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hal_device = intf_det->hal_device;
#if ALG_INTF_DET_DEBUG_MODE
#ifdef _PRE_WLAN_ALG_UART_PRINT
    if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
        wifi_printf("start timer! intf_det_mode = %d, statistic cycle = %d\r\n", intf_det->intf_det_mode,
            intf_det->intf_det_cycle_ms);
    }
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_POWERSAVE
    /* C核休眠时不触发检测,0表示休眠 */
    if (hmac_sta_pm_get_wlan_dev_state() == HMAC_ALLOW_SLEEP) {
        return OAL_SUCC;
    }
#endif

    /* 同频/邻频叠频干扰统计 */
    if (((intf_det->intf_det_mode & ALG_INTF_DET_MODE_ADJCHANNEL) != 0) ||
        ((intf_det->intf_det_mode & ALG_INTF_DET_MODE_COCHANNEL) != 0)) {
        alg_intf_det_adjch_stat_handler(hal_device, intf_det);
    }
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    /* 通知EDCA模块对goodput和碰撞率进行统计 */
    hmac_alg_cfg_intf_det_edca_notify();
#endif

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/******************************************************************************
 功能描述  : 干扰识别参数配置函数续
******************************************************************************/
OSAL_STATIC osal_u32 alg_intf_det_config_param_resume_second(alg_intf_det_stru *intf_det,
    const mac_ioctl_alg_param_stru *alg_param)
{
    switch (alg_param->alg_cfg) {
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
        case MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH:
            intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_NO] = (osal_u8)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::no adjintf cyc thr is: =%d}",
                             intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_NO]);
            break;
#else
        case MAC_ALG_CFG_INTF_DET_NO_ADJCYC_TH:
            intf_det->adjch_intf_info.non_intf_cyc_th = (osal_u8)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::no adjintf cyc thr is: =%d}",
                             intf_det->adjch_intf_info.non_intf_cyc_th);
            break;
#endif
        case MAC_ALG_CFG_INTF_DET_GET_INTF_TYPE:
            wifi_printf("coch_intf_type = %d, adjch_intf_type = %d, state = %d, noise_floor = %d\n",
                intf_det->coch_intf_info.coch_intf_state_sta, intf_det->adjch_intf_info.intf_type,
                intf_det->intf_det_handler.oal_fsm.cur_state, intf_det->adjch_intf_stat.intf_det_avg_rssi_20);
            break;
        case MAC_ALG_CFG_INTF_DET_SET_PK_MODE:
            intf_det->pk_mode = (osal_u8)alg_param->value;
            if (intf_det->pk_mode == OSAL_TRUE) {
                alg_intf_det_fsm_post_event(intf_det, ALG_INTF_DET_EVENT_HOST_PK, 0, OSAL_NULL);
            } else {
                alg_intf_det_fsm_post_event(intf_det, ALG_INTF_DET_EVENT_HOST_NPK, 0, OSAL_NULL);
            }
            wifi_printf("alg_intf_det_config_param::set_pk_mode: =%d}", intf_det->pk_mode);
            break;
        default:
            oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param:config param error!}");
            return OAL_FAIL;
    }
    return OAL_SUCC;
}
/******************************************************************************
 功能描述  : 干扰识别参数配置函数续
******************************************************************************/
OSAL_STATIC osal_u32 alg_intf_det_config_param_resume_first(alg_intf_det_stru *intf_det,
    const mac_ioctl_alg_param_stru *alg_param)
{
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_INTF_DET_DEBUG:
#if ALG_INTF_DET_DEBUG_MODE
            intf_det->intf_det_debug = (osal_u8)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{config_param::debug mode is: =%d}", intf_det->intf_det_debug);
#endif
            break;

        case MAC_ALG_CFG_INTF_DET_COCH_THR_STA:
            intf_det->coch_intf_info.nondir_duty_cyc_th_sta = (osal_u16)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{sta thr is: =%d}", intf_det->coch_intf_info.nondir_duty_cyc_th_sta);
            break;
        case MAC_ALG_CFG_INTF_DET_COCH_NOINTF_STA:
            intf_det->coch_intf_info.nonintf_duty_th_sta = (osal_u16)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::sta nointf thr is: =%d}",
                             intf_det->coch_intf_info.nonintf_duty_th_sta);
            break;

        case MAC_ALG_CFG_INTF_DET_ADJCH_SCAN_CYC:
            intf_det->adjch_intf_info.chn_scan_cycle_ms = (osal_u16)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::adj intfdet cycle is: =%d(ms)}",
                             intf_det->adjch_intf_info.chn_scan_cycle_ms);
            break;

        case MAC_ALG_CFG_INTF_DET_ADJRATIO_THR:
            intf_det->strong_intf_ratio_th = (osal_u16)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::strong intf thr is: =%d}",
                             intf_det->strong_intf_ratio_th);
            break;

        case MAC_ALG_CFG_INTF_DET_SYNC_THR:
            intf_det->adjch_intf_info.intf_sync_err_th = (osal_u16)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{sync thr is: =%d}", intf_det->adjch_intf_info.intf_sync_err_th);
            break;

        case MAC_ALG_CFG_INTF_DET_AVE_RSSI:
            intf_det->intf_aveg_rssi_th = (osal_s8)((osal_s32)alg_param->value);
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::ave rssi thr is: =%d}",
                             (osal_u8)intf_det->intf_aveg_rssi_th);
            break;

        case MAC_ALG_CFG_INTF_DET_NO_ADJRATIO_TH:
            intf_det->nonintf_ratio_th = (osal_u16)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::no adjintf ratio thr is: =%d}",
                             intf_det->nonintf_ratio_th);
            break;

        default:
            if (alg_intf_det_config_param_resume_second(intf_det, alg_param) != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param:config param error!}");
                return OAL_FAIL;
            }
            return OAL_SUCC;
    }
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 干扰识别参数配置函数
******************************************************************************/
osal_u32 alg_intf_det_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_intf_det_stru *intf_det = OSAL_NULL;
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;
    osal_u32 ret;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_INTF_DET_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_INTF_DET_END)) {
        return OAL_SUCC;
    }
    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == NULL)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param: hal_device null pointer!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void **)&intf_det);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 配置对应参数 */
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_INTF_DET_CYCLE:
            frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, alg_param->value,
                (osal_void *)(intf_det), OSAL_TRUE);
            memset_s(&(intf_det->coch_intf_stat), sizeof(alg_intf_det_coch_stat_stru), 0,
                sizeof(alg_intf_det_coch_stat_stru));
            memset_s(&(intf_det->adjch_intf_stat), sizeof(alg_intf_det_adjch_stat_stru), 0,
                sizeof(alg_intf_det_adjch_stat_stru));
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
            hal_set_counter1_clear();
#endif
            hal_set_sync_err_counter_clear();
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param::det time change to = %d",
                             alg_param->value);
            break;

        case MAC_ALG_CFG_INTF_DET_MODE:
            intf_det->intf_det_mode = (osal_u8)alg_param->value;
            oam_warning_log1(0, OAM_SF_ANTI_INTF,
                "{alg_intf_det_config_param::mode is: =%d}", intf_det->intf_det_mode);
            break;

        default:
            if (alg_intf_det_config_param_resume_first(intf_det, alg_param) != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_config_param:config param error!}");
                return OAL_FAIL;
            }
    }

    return OAL_SUCC;
}
#endif
#define COCH_INTF_DET
/******************************************************************************
 功能描述  : 统计占空比
******************************************************************************/
OSAL_STATIC osal_u32 alg_intf_det_duty_cyc_smth(osal_u32 x, osal_u32 y, osal_u32 a)
{
    return (((x) >> (a)) + (y) - ((y) >> (a)));
}
/******************************************************************************
 功能描述  : 计算同频干扰相关占空比
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_coch_tx_time_duty_cyc(hal_to_dmac_device_stru *hal_device,
    alg_intf_det_stru *intf_det, const wlan_scan_chan_stats_stru *chan_result)
{
    /* MAC测量周期ms */
    intf_det->coch_intf_stat.total_stats_time_ms = chan_result->total_stats_time_us >> ALG_INTF_DET_PERIOD_SHIFT;

    if (chan_result->total_stats_time_us == 0) {
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg chan_result->total_stats_time_us == 0}");
        return;
    }

    /* 弱干扰免疫负增益try:计算平均tx time */
    if (intf_det->coch_intf_stat.tx_time_last != 0) {
        intf_det->coch_intf_stat.tx_time_avg = alg_intf_det_duty_cyc_smth(intf_det->coch_intf_stat.tx_time_last,
            intf_det->coch_intf_stat.tx_time, ALG_INTF_DET_SMTH_SHIFT_BIT);
    } else {
        intf_det->coch_intf_stat.tx_time_avg = intf_det->coch_intf_stat.tx_time;
    }

    intf_det->coch_intf_stat.tx_time_last = intf_det->coch_intf_stat.tx_time;

    /* rx direct和rx nondir都加上进入低功耗之前的统计值，与tx time占空比计算保持一致 */
    intf_det->coch_intf_stat.rx_nondir_duty_cyc =
        (osal_u16)(intf_det->coch_intf_stat.rx_nondir_time /
                 intf_det->coch_intf_stat.total_stats_time_ms);
    intf_det->coch_intf_stat.tx_time_duty_cyc = (osal_u16)(intf_det->coch_intf_stat.tx_time /
        intf_det->coch_intf_stat.total_stats_time_ms);
    intf_det->coch_intf_stat.rx_direct_duty_cyc =
        (osal_u16)(intf_det->coch_intf_stat.rx_direct_time /
                 intf_det->coch_intf_stat.total_stats_time_ms);

    /* 算法统计发送占空比给dmac使用 */
    hal_device->duty_ratio = intf_det->coch_intf_stat.tx_time_duty_cyc;
}
/******************************************************************************
 功能描述  : 同频干扰识别
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_coch_intf_type_identify(alg_intf_det_stru *intf_det, osal_u16 nondir_duty_cyc_smth,
    osal_u16 non_dir_thr_sta)
{
    if ((nondir_duty_cyc_smth > non_dir_thr_sta) && (nondir_duty_cyc_smth < ALG_INTF_DET_NONDIR_RATIO_THR_LIMIT)) {
        intf_det->coch_intf_info.intf_cycle_cnt_sta += 1;

        /* 连续4个周期检测到干扰，才认为有干扰 */
        if (intf_det->coch_intf_info.intf_cycle_cnt_sta >= ALG_INTF_DET_INTF_CYCLE_NUM_STA) {
            intf_det->coch_intf_info.coch_intf_state_sta = OSAL_TRUE;
            intf_det->coch_intf_info.intf_cycle_cnt_sta = ALG_INTF_DET_INTF_CYCLE_NUM_STA;
        }
        intf_det->coch_intf_info.nonintf_cycle_cnt_sta = 0;
    } else if (nondir_duty_cyc_smth < intf_det->coch_intf_info.nonintf_duty_th_sta) {
        intf_det->coch_intf_info.nonintf_cycle_cnt_sta += 1;

        /* 连续8个周期检测不到干扰，认为无干扰 */
        if (intf_det->coch_intf_info.nonintf_cycle_cnt_sta >= ALG_INTF_DET_NO_INTF_CYCLE_NUM_STA) {
            intf_det->coch_intf_info.coch_intf_state_sta = OSAL_FALSE;
            intf_det->coch_intf_info.nonintf_cycle_cnt_sta = ALG_INTF_DET_NO_INTF_CYCLE_NUM_STA;
        }
        intf_det->coch_intf_info.intf_cycle_cnt_sta = 0;
    }
    /* 不满足连续2个周期的tx time大于门限，认为无业务，切回无干扰 */
    if ((intf_det->coch_intf_stat.tx_time_duty_cyc_last < ALG_INTF_DET_TX_RATIO_THR_STA) &&
        (intf_det->coch_intf_stat.tx_time_duty_cyc < ALG_INTF_DET_TX_RATIO_THR_STA)) {
        intf_det->coch_intf_info.coch_intf_state_sta = OSAL_FALSE;
    }
    intf_det->coch_intf_stat.tx_time_duty_cyc_last = intf_det->coch_intf_stat.tx_time_duty_cyc;
    intf_det->coch_intf_stat.rx_direct_duty_cyc_last = intf_det->coch_intf_stat.rx_direct_duty_cyc;
    return;
}
/******************************************************************************
 功能描述  : 同频干扰类型变化通知
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_coch_intf_type_change_notify(alg_intf_det_stru *intf_det,
    oal_bool_enum_uint8 intf_stat_sta_old, osal_u16 non_dir_thr_sta)
{
    unref_param(non_dir_thr_sta);
    if (intf_det->coch_intf_info.coch_intf_state_sta != intf_stat_sta_old) {
        intf_det->intf_det_notify.coch_intf_state_last = intf_det->intf_det_notify.coch_intf_state;
        intf_det->intf_det_notify.coch_intf_state = intf_det->coch_intf_info.coch_intf_state_sta;
        intf_det->intf_det_notify.intf_det_type = ALG_INTF_DET_COCH;
        alg_intf_det_send_intf_type_msg(&intf_det->intf_det_notify);
        oam_warning_log_alter(0, OAM_SF_ANTI_INTF,
            "cur cointf staus = %d! rx_nondir_thr = %d, rx_dir_duty = %d, tx_duty = %d", 4, /* 4:参数个数 */
            intf_det->coch_intf_info.coch_intf_state_sta, non_dir_thr_sta,
            intf_det->coch_intf_stat.rx_direct_duty_cyc, intf_det->coch_intf_stat.tx_time_duty_cyc);
    }
    return;
}
/******************************************************************************
 功能描述  : 同频干扰识别
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_coch_intf_state(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det,
    const hal_ch_mac_statics_stru *mac_stats, osal_u16 nondir_duty_cyc_smth, osal_u16 non_dir_thr_sta)
{
    oal_bool_enum_uint8 intf_stat_sta_old;
    intf_stat_sta_old = intf_det->coch_intf_info.coch_intf_state_sta;
    unref_param(hal_device);
    unref_param(mac_stats);
    alg_intf_det_coch_intf_type_identify(intf_det, nondir_duty_cyc_smth, non_dir_thr_sta);

#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("tx_time_duty_cyc_last = %d, tx_time_duty_cyc = %d\n",
            intf_det->coch_intf_stat.tx_time_duty_cyc_last, intf_det->coch_intf_stat.tx_time_duty_cyc);
#endif
    }
#endif

    /* 同频干扰状态发生变化通知其他模块 */
    alg_intf_det_coch_intf_type_change_notify(intf_det, intf_stat_sta_old, non_dir_thr_sta);

#if ALG_INTF_DET_DEBUG_MODE
    /* debug: 打印统计结果 */
    if (intf_det->intf_det_debug == OSAL_TRUE) {
        oam_warning_log4(0, OAM_SF_ANTI_INTF,
            "rx-dir[us] = %d, rx-non-dir[us] = %d, tx[us] = %d, mac rx-non-dir[us] = %d",
            intf_det->coch_intf_stat.rx_direct_time, intf_det->coch_intf_stat.rx_nondir_time,
            intf_det->coch_intf_stat.tx_time, mac_stats->rx_nondir_time);
        oam_warning_log4(0, OAM_SF_ANTI_INTF, "rx_nondir_duty = %d, rx_nondir_thr = %d, rx_dir_duty = %d, tx_duty = %d",
            nondir_duty_cyc_smth, non_dir_thr_sta, intf_det->coch_intf_stat.rx_direct_duty_cyc,
            intf_det->coch_intf_stat.tx_time_duty_cyc);
        oam_warning_log2(0, OAM_SF_ANTI_INTF, "Detect result = %d, total_stat_time = %d",
            intf_det->coch_intf_info.coch_intf_state_sta, intf_det->coch_intf_stat.total_stats_time_ms);
    } else if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("rx-dir[us] = %d, rx-non-dir[us] = %d, tx[us] = %d, mac rx-non-dir[us] = %d \n",
            intf_det->coch_intf_stat.rx_direct_time, intf_det->coch_intf_stat.rx_nondir_time,
            intf_det->coch_intf_stat.tx_time, mac_stats->rx_nondir_time);
        wifi_printf("rx_nondir_duty = %d, rx_nondir_thr = %d, rx_dir_duty = %d, tx_duty = %d \n",
            nondir_duty_cyc_smth, non_dir_thr_sta, intf_det->coch_intf_stat.rx_direct_duty_cyc,
            intf_det->coch_intf_stat.tx_time_duty_cyc);
        wifi_printf("Detect result = %d, total_stat_time = %d \n",
            intf_det->coch_intf_info.coch_intf_state_sta, intf_det->coch_intf_stat.total_stats_time_ms);
#endif
    }
#endif
}

/******************************************************************************
 功能描述  : 同频干扰识别函数
******************************************************************************/
osal_void alg_intf_det_coch_identify(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det)
{
    hal_ch_mac_statics_stru *mac_stats;
    wlan_scan_chan_stats_stru *chan_result;
    osal_u16 nondir_duty_cyc_smth;
    osal_u16 non_dir_thr_sta;

    non_dir_thr_sta = intf_det->coch_intf_info.nondir_duty_cyc_th_sta;

    chan_result = &(hal_device->chan_result);
    mac_stats = &(hal_device->mac_ch_stats);

    /* 针对时间做检查，时间过短，不更新 */
    if (chan_result->total_stats_time_us < 3000) { /* 3000us */
        return;
    }

    /* rx nondirct 时间需要包括FCS校验错误的时间统计,因此配置为MAC RX TIME - RX DIRECT TIME = RX
     * NON-DIRECT TIME */
    intf_det->coch_intf_stat.rx_direct_time = mac_stats->rx_direct_time;
    intf_det->coch_intf_stat.tx_time = mac_stats->tx_time;
    /* total_recv_time_us统计的时间为RX_PROGRESS_COUNT的值,
       每次开始测量自动清零,测量时PHY中断告知MAC开始统计计时,
       不包括PHY头时间在内,PHY解析到数据部分后告知MAC切换为接收;
       rx_direct_time统计的时间为RPT_RX_DIRECT_FRAME_TIME_CNT,MAC收帧并且CRC校验正确后计算出的当前帧的接收时间,包括PHY头;
       区别:如果当接收帧的一半时开始测量,那么存在如下统计误差,误差约为
       RX_PROGRESS_COUNT只统计剩下半帧的时间,
       RPT_RX_DIRECT_FRAME_TIME_CNT统计为整帧的接收时间 */
    if (chan_result->total_recv_time_us < mac_stats->rx_direct_time) {
        intf_det->coch_intf_stat.rx_nondir_time = mac_stats->rx_nondir_time;
    } else {
        intf_det->coch_intf_stat.rx_nondir_time =
            osal_max((chan_result->total_recv_time_us - mac_stats->rx_direct_time), mac_stats->rx_nondir_time);
        /* 为避免计算出的rx nondir time与mac统计的时间相差较大，导致误检测，做一个约束处理 */
        if ((mac_stats->rx_nondir_time > 1000) && /* 1000:大于1000 */
            /* 2:左移2 */
            ((chan_result->total_recv_time_us - mac_stats->rx_direct_time) >= (mac_stats->rx_nondir_time << 2))) {
            intf_det->coch_intf_stat.rx_nondir_time = mac_stats->rx_nondir_time << 1;
        }
    }

    /* 统计发送和接收占空比 */
    alg_intf_det_coch_tx_time_duty_cyc(hal_device, intf_det, chan_result);

    /* 平滑前后两个周期的结果 */
    nondir_duty_cyc_smth = intf_det->coch_intf_stat.rx_nondir_duty_cyc;

    if (intf_det->coch_intf_stat.rx_nondir_duty_cyc < ALG_INTF_DET_NONDIR_RATIO_THR_LIMIT) {
        if ((intf_det->coch_intf_stat.rx_nondir_duty_cyc_last != 0)) {
            nondir_duty_cyc_smth = (osal_u16)alg_intf_det_duty_cyc_smth(
                intf_det->coch_intf_stat.rx_nondir_duty_cyc_last,
                intf_det->coch_intf_stat.rx_nondir_duty_cyc, ALG_INTF_DET_SMTH_SHIFT_BIT);
        }

        intf_det->coch_intf_stat.rx_nondir_duty_cyc_last = intf_det->coch_intf_stat.rx_nondir_duty_cyc;
    } else {
        intf_det->coch_intf_stat.rx_nondir_duty_cyc_last = 0;
    }

    /* 双向业务优化:动态门限 */
    if (intf_det->coch_intf_stat.rx_direct_duty_cyc > ALG_INTF_DET_RX_DIRECT_RATIO_THR_STA) {
        non_dir_thr_sta = (osal_u16)(intf_det->coch_intf_info.nondir_duty_cyc_th_sta << 1);
    }
#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("non_dir_thr_sta = %d, rx_nondir_duty_cyc = %d, tx_time_avg = %d\n",
            non_dir_thr_sta, nondir_duty_cyc_smth, intf_det->coch_intf_stat.tx_time_avg);
#endif
    }
#endif
    /* non-direct包占空比大于门限，认为有同频干扰，否则无 */
    alg_intf_det_coch_intf_state(hal_device, intf_det, mac_stats, nondir_duty_cyc_smth, non_dir_thr_sta);
}

/******************************************************************************
 功能描述  : 信道测量设置发送到dmac
******************************************************************************/
osal_u32 alg_intf_det_start_det_msg(const mac_scan_req_stru *scan_req_params)
{
    alg_intf_det_ch_stats_param scan_param;
    osal_u32 ret = OAL_SUCC;
    frw_msg msg = {0};

    (osal_void)memset_s(&scan_param, sizeof(alg_intf_det_ch_stats_param), 0, sizeof(alg_intf_det_ch_stats_param));
    scan_param.bss_type = scan_req_params->bss_type;
    scan_param.channel_list[0] = scan_req_params->channel_list[0];
    scan_param.channel_nums = scan_req_params->channel_nums;
    scan_param.max_scan_count_per_channel = scan_req_params->max_scan_count_per_channel;
    scan_param.probe_delay = scan_req_params->probe_delay;
    scan_param.scan_func = scan_req_params->scan_func;
    scan_param.scan_mode = scan_req_params->scan_mode;
    scan_param.scan_time = scan_req_params->scan_time;
    scan_param.scan_type = scan_req_params->scan_type;
    scan_param.vap_id = scan_req_params->vap_id;
    scan_param.measure_period.statics_period = ALG_INTF_DET_SCAN_CHANNEL_STATICS_PERIOD_US;
    scan_param.measure_period.meas_period = ALG_INTF_DET_SCAN_CHANNEL_MEAS_PERIOD_MS;
    msg.data = (osal_u8 *)&scan_param;
    msg.data_len = (osal_u16)sizeof(alg_intf_det_ch_stats_param);
    /* 抛事件至Device侧alg */
    ret = (osal_u32)frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_INTF_DET_START, &msg, OSAL_FALSE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "{alg_intf_det_send_intf_type_msg:: intf type notify fail [%d].}", ret);
        return ret;
    }
    return ret;
}

/******************************************************************************
 功能描述  : 获取同频干扰类型
******************************************************************************/
oal_bool_enum_uint8 alg_intf_det_get_curr_coch_intf_type(hal_to_dmac_device_stru *hal_device)
{
    osal_u32 ret;
    alg_intf_det_stru *intf_det = OSAL_NULL;
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void **)&intf_det);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_get_curr_coch_intf_type: get intf det struct fail!}");
        return OAL_FALSE;
    }
    return intf_det->coch_intf_info.coch_intf_state_sta;
}

/******************************************************************************
 功能描述  : 信道检测参数设置
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_prepare_adjch_param_resume(const alg_intf_det_stru *intf_det,
    const hmac_vap_stru *hmac_vap)
{
    mac_scan_req_stru scan_req_params;
    osal_u32 ret = OAL_SUCC;

    if (alg_intf_det_set_cca_stat(&scan_req_params, hmac_vap, intf_det) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_prepare_adjch_param_resume: set_cca_stat err!}");
    }

    ret = alg_intf_det_start_det_msg(&scan_req_params);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_prepare_adjch_param_resume: send msg err!}");
    }
}

/******************************************************************************
 功能描述  : 同频干扰识别函数
******************************************************************************/
osal_void alg_intf_det_prepare_adjch_param(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 vap_index, up_vap_num;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    oal_bool_enum_uint8 ret = 1;
    ret = mac_is_dbac_running(hmac_device);
    /* DBAC或者当前为扫描状态时，不进行邻频干扰检测 */
    if ((ret == OSAL_TRUE) || (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING)) {
        oam_warning_log2(0, OAM_SF_ANTI_INTF,
            "{alg_intf_det_prepare_adjch_param: dbac is running = %d, curr_scan_state = %d}", ret,
            hmac_device->curr_scan_state);
        return;
    }

    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)hmac_get_vap(mac_vap_id[vap_index]);
        if (osal_unlikely(hmac_vap == OSAL_NULL)) {
            continue;
        }

        if (hmac_vap->protocol < WLAN_HT_MODE) {
            intf_det->adjch_intf_info.strong_intf_ratio_th = ALG_INTF_DET_11BG_ADJINTF_RATIO_TH;
            intf_det->adjch_intf_info.nonintf_ratio_th = ALG_INTF_DET_NOADJINTF_11BG_RATIO_TH;
        } else if (hmac_vap->protocol < WLAN_PROTOCOL_BUTT) {
            intf_det->adjch_intf_info.strong_intf_ratio_th = intf_det->strong_intf_ratio_th;
            intf_det->adjch_intf_info.nonintf_ratio_th = intf_det->nonintf_ratio_th;
        }

        break;
    }

    /* 如果1)DBAC ON; 2)一直未查询到vap; 3)vap工作在不同信道则不发送扫描 */
    /* T1定时器到期:设置扫描参数，发出扫描请求 */
    if (osal_unlikely((hmac_vap == OSAL_NULL) || (hmac_vap->al_tx_flag == EXT_SWITCH_ON))) {
        return;
    }

    alg_intf_det_prepare_adjch_param_resume(intf_det, hmac_vap);
}
#define ADJCH_INTF_DET
/******************************************************************************
 功能描述  : 计算占空比
******************************************************************************/
OSAL_STATIC osal_u16 alg_intf_det_get_duty_cyc_ratio(const wlan_scan_chan_stats_stru *chan_result,
    osal_u32 total_free_time, osal_u32 abort_time_us)
{
    osal_u16 ret = (osal_u16)(((total_free_time + abort_time_us +
                  (chan_result)->total_recv_time_us +
                  (chan_result)->total_send_time_us) >=
                  (chan_result)->total_stats_time_us) ? 0 :
                  ((((chan_result)->total_stats_time_us -
                  total_free_time - abort_time_us -
                  (chan_result)->total_recv_time_us -
                  (chan_result)->total_send_time_us) <<
                  ALG_INTF_DET_ADJ_RATIO_SHIFT) / ((chan_result)->total_stats_time_us)));
    return ret;
}
/******************************************************************************
 功能描述  : 计算干扰繁忙度
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_stat_get_duty_cyc_ratio(alg_intf_det_stru *intf_det,
    const wlan_scan_chan_stats_stru *chan_result)
{
    /* 记录bt工作时间结束值 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    osal_u32 abort_time_us_end;
    const hal_btcoex_statistics_stru *btcoex_statistics = hal_btcoex_statistics();

    /* 1000000:倍数 */
    abort_time_us_end = btcoex_statistics->xzyvwRO4SwzRlvoO_ * 1000000 + btcoex_statistics->xGji_LwFd_GLBihwdC_;
    intf_det->adjch_intf_stat.abort_time_us = (abort_time_us_end >= intf_det->adjch_intf_stat.abort_time_us) ?
                                              (abort_time_us_end - intf_det->adjch_intf_stat.abort_time_us) :
                                              0;
    intf_det->adjch_intf_stat.abort_time_us =
        (intf_det->adjch_intf_stat.abort_time_us < chan_result->total_stats_time_us) ?
        intf_det->adjch_intf_stat.abort_time_us :
        0;
#endif

    /* 信道测量次数 */
    intf_det->adjch_intf_stat.intf_det_ilde_cnt = chan_result->free_power_cnt;

    /* 获取统计的信道结果:繁忙度/千分之X, 若统计小于等于2个周期，则认为无干扰 */
    if (chan_result->total_stats_time_us <= 70000) { /* 70000us:统计边界 */
        intf_det->adjch_intf_stat.duty_cyc_ratio_20 = 0;
        intf_det->adjch_intf_stat.duty_cyc_ratio_40 = 0;
    } else {
        intf_det->adjch_intf_stat.duty_cyc_ratio_20 = (osal_u16)alg_intf_det_get_duty_cyc_ratio(chan_result,
            chan_result->total_free_time_20m_us, intf_det->adjch_intf_stat.abort_time_us);
        intf_det->adjch_intf_stat.duty_cyc_ratio_40 = (osal_u16)alg_intf_det_get_duty_cyc_ratio(chan_result,
            chan_result->total_free_time_40m_us, intf_det->adjch_intf_stat.abort_time_us);
        intf_det->adjch_intf_stat.duty_cyc_ratio_sec20 = (osal_u16)alg_intf_det_get_duty_cyc_ratio(chan_result,
            chan_result->total_free_time_sec20m_us, intf_det->adjch_intf_stat.abort_time_us);
#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("total_stats_time_us = %d, duty_cyc_ratio_20 = %d, duty_cyc_ratio_40 = %d, \
            duty_cyc_ratio_sec20 = %d\n",
            chan_result->total_stats_time_us, intf_det->adjch_intf_stat.duty_cyc_ratio_20,
            intf_det->adjch_intf_stat.duty_cyc_ratio_40, intf_det->adjch_intf_stat.duty_cyc_ratio_sec20);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        wifi_printf("alg_intf_det_stat_get_duty_cyc_ratio: abort_duration_us = %d, abort_duration = %d, \
            abort_time = %d\n", btcoex_statistics->xGji_LwFd_GLBihwdC_, btcoex_statistics->xzyvwRO4SwzRlvoO_,
            intf_det->adjch_intf_stat.abort_time_us);
#endif
#endif
    }
#endif
    }
}
/******************************************************************************
 功能描述  : 获取信道检测相关信息，通知状态机周期性扫描事件
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_get_intf_stat_info(alg_intf_det_stru *intf_det,
    const wlan_scan_chan_stats_stru *chan_result, const hmac_vap_stru *hmac_vap, hal_ch_intf_statics_stru *ch_intf_stat)
{
    unref_param(hmac_vap);
    /* 获取干扰统计信息,供dmac使用 */
    ch_intf_stat->rx_time = chan_result->total_recv_time_us;
    ch_intf_stat->tx_time = chan_result->total_send_time_us;
    ch_intf_stat->free_time = chan_result->total_free_time_20m_us;
    ch_intf_stat->abort_time_us = intf_det->adjch_intf_stat.abort_time_us;
    ch_intf_stat->duty_cyc_ratio_20 = intf_det->adjch_intf_stat.duty_cyc_ratio_20;

    ch_intf_stat->duty_cyc_ratio_40 = intf_det->adjch_intf_stat.duty_cyc_ratio_40;

    ch_intf_stat->sync_err_ratio = intf_det->adjch_intf_stat.sync_err_ratio;

#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == OSAL_TRUE) {
        oam_warning_log4(0, OAM_SF_ANTI_INTF, "{intf det result:MAC_stat_time = %d,recv = %d,send = %d,free_20= %d}",
            chan_result->total_stats_time_us, chan_result->total_recv_time_us, chan_result->total_send_time_us,
            chan_result->total_free_time_20m_us);
        oam_warning_log4(0, OAM_SF_ANTI_INTF,
            "{intf det result:PHY_stat_time = %d,sync err ratio= %d, stat_cnt= %d, bt_abort_time= %d }",
            chan_result->phy_total_stats_time_ms, intf_det->adjch_intf_stat.sync_err_ratio,
            chan_result->free_power_cnt, intf_det->adjch_intf_stat.abort_time_us);
        oam_warning_log2(0, OAM_SF_ANTI_INTF, "{intf det result:channel busy duty: 20M= %d, 40M= %d}",
            intf_det->adjch_intf_stat.duty_cyc_ratio_20, intf_det->adjch_intf_stat.duty_cyc_ratio_40);
        oam_warning_log2(0, OAM_SF_ANTI_INTF,
            "{intf det calcu result:noise floor: 20M= %d, 40M= %d}",
            intf_det->adjch_intf_stat.intf_det_avg_rssi_20, intf_det->adjch_intf_stat.intf_det_avg_rssi_40);
    } else if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("{intf det result:MAC_stat_time = %d,recv = %d,send = %d,free_20= %d}\n",
            chan_result->total_stats_time_us, chan_result->total_recv_time_us, chan_result->total_send_time_us,
            chan_result->total_free_time_20m_us);
        wifi_printf("{intf det result:PHY_stat_time = %d,sync err ratio= %d, stat_cnt= %d, bt_abort_time= %d}\n",
            chan_result->phy_total_stats_time_ms, intf_det->adjch_intf_stat.sync_err_ratio,
            chan_result->free_power_cnt, intf_det->adjch_intf_stat.abort_time_us);
        wifi_printf("{intf det result:channel busy duty: 20M= %d, 40M= %d}\n",
            intf_det->adjch_intf_stat.duty_cyc_ratio_20, intf_det->adjch_intf_stat.duty_cyc_ratio_40);
        wifi_printf("{intf det calcu result:noise floor: 20M= %d, 40M= %d}\n",
            intf_det->adjch_intf_stat.intf_det_avg_rssi_20, intf_det->adjch_intf_stat.intf_det_avg_rssi_40);
#endif
    }
#endif

    intf_det->adjch_intf_info.intf_aveg_rssi_th = intf_det->intf_aveg_rssi_th;

    /* 识别邻频和叠频干扰 */
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    if (alg_intf_det_fsm_post_event(intf_det, ALG_INTF_DET_EVENT_DUTY_CYCLE, 0, OSAL_NULL) == OAL_FAIL) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_scan_chn_cb: det_fsm_post_event err!}");
    }
#endif
}
/******************************************************************************
 功能描述  : 获取信道检测结果，清除测量结果
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_chan_result_info(alg_intf_det_stru *intf_det, hal_to_dmac_device_stru *hal_device,
    hmac_vap_stru *hmac_vap, wlan_scan_chan_stats_stru *chan_result)
{
    hal_ch_intf_statics_stru *ch_intf_stat = OSAL_NULL;
    osal_u32 sync_err_cnt = 0;

    ch_intf_stat = &(hal_device->intf_statics_stru);
    alg_intf_det_stat_get_duty_cyc_ratio(intf_det, chan_result);
    /* 获取统计的信道结果:底噪平均值,如果统计计数为0则不做除法 */
    if (chan_result->free_power_cnt == 0) {
        intf_det->adjch_intf_stat.intf_det_avg_rssi_20 = 0;
        intf_det->adjch_intf_stat.intf_det_avg_rssi_40 = 0;
    } else {
        intf_det->adjch_intf_stat.intf_det_avg_rssi_20 =
            (osal_s8)(chan_result->free_power_stats_20m / (osal_s16)chan_result->free_power_cnt);
        intf_det->adjch_intf_stat.intf_det_avg_rssi_40 =
            (osal_s8)(chan_result->free_power_stats_40m / (osal_s16)chan_result->free_power_cnt);
    }
#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("intf_det_avg_rssi_20 = %d, intf_det_avg_rssi_40 = %d\n",
            intf_det->adjch_intf_stat.intf_det_avg_rssi_20, intf_det->adjch_intf_stat.intf_det_avg_rssi_40);
#endif
    }
#endif
    /* 获取统计的信道结果:读取PHY sync error的寄存器值 */
    hal_get_sync_error_cnt(&sync_err_cnt);
    intf_det->adjch_intf_stat.sync_err_ratio = (osal_u16)sync_err_cnt;

    /* sync error counter 清零 */
    hal_set_sync_err_counter_clear();
    alg_intf_det_get_intf_stat_info(intf_det, chan_result, hmac_vap, ch_intf_stat);
#ifndef _PRE_WLAN_FEATURE_NEGTIVE_DET
    if (alg_intf_det_idy_adjch_intf(hal_device, intf_det) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_scan_chn_cb: det_idy_adjch_intf err!}");
    }
#endif
}
#if ALG_INTF_DET_DEBUG_MODE
/******************************************************************************
 功能描述  : 信道扫描结果打印
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_scan_result_debug(const alg_intf_det_stru *intf_det,
    hal_to_dmac_device_stru *hal_device)
{
    if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("free_power_stats_20m = %d, free_power_stats_40m = %d, phy_total_stats_time_ms = %d, \
            free_power_cnt = %d。\n",
            hal_device->chan_result.free_power_stats_20m, hal_device->chan_result.free_power_stats_40m,
            hal_device->chan_result.phy_total_stats_time_ms, hal_device->chan_result.free_power_cnt);
        wifi_printf("total_free_time_20m_us = %d, total_free_time_40m_us = %d, total_free_time_sec20m_us = %d, \
            total_recv_time_us = %d。\n",
            hal_device->chan_result.total_free_time_20m_us, hal_device->chan_result.total_free_time_40m_us,
            hal_device->chan_result.total_free_time_sec20m_us, hal_device->chan_result.total_recv_time_us);
        wifi_printf("total_send_time_us = %d, total_stats_time_us = %d, rx_direct_time = %d, \
            rx_nondir_time = %d, tx_time = %d。\n",
            hal_device->chan_result.total_send_time_us, hal_device->chan_result.total_stats_time_us,
            hal_device->mac_ch_stats.rx_direct_time, hal_device->mac_ch_stats.rx_nondir_time,
            hal_device->mac_ch_stats.tx_time);
#endif
    } else if (intf_det->intf_det_debug == OSAL_TRUE) {
        oam_warning_log4(0, OAM_SF_ANY,
            "{free_power_stats_20m[%d], free_power_stats_40m[%d], phy_total_stats_time_ms[%d],free_power_cnt[%d]}",
            hal_device->chan_result.free_power_stats_20m, hal_device->chan_result.free_power_stats_40m,
            hal_device->chan_result.phy_total_stats_time_ms, hal_device->chan_result.free_power_cnt);
        oam_warning_log4(0, OAM_SF_ANY,
            "{total_free_time_20m_us[%d], total_free_time_40m_us[%d], total_free_time_sec20m_us[%d], \
            total_recv_time_us[%d]}",
            hal_device->chan_result.total_free_time_20m_us, hal_device->chan_result.total_free_time_40m_us,
            hal_device->chan_result.total_free_time_sec20m_us, hal_device->chan_result.total_recv_time_us);
        oam_warning_log2(0, OAM_SF_ANY, "{total_send_time_us[%d], total_stats_time_us[%d]}",
            hal_device->chan_result.total_send_time_us, hal_device->chan_result.total_stats_time_us);
        oam_warning_log3(0, OAM_SF_ANY,
            "{rx_direct_time[%d], rx_nondir_time[%d], tx_time[%d]}", hal_device->mac_ch_stats.rx_direct_time,
            hal_device->mac_ch_stats.rx_nondir_time, hal_device->mac_ch_stats.tx_time);
    }
    return;
}
#endif

/******************************************************************************
函 数 名  :alg_intf_det_print_gla_info
功能描述  : 干扰检测算法图形化维测打印
 ******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_print_gla_info(alg_intf_det_stru *intf_det)
{
    oam_warning_log4(0, OAM_SF_ANTI_INTF, "[GLA][ALG][INTF_DET][INTF_DET_BITMAP1]:intf_type=0x%x,non_direct_duty=%d, \
        channel_busy_duty_20M=%d,channel_busy_duty_40M=%d",
        ((intf_det->coch_intf_info.coch_intf_state_sta << ALG_INTF_DET_GLA_SHIFT) |
        intf_det->adjch_intf_info.intf_type), intf_det->coch_intf_stat.rx_nondir_duty_cyc,
        intf_det->adjch_intf_stat.duty_cyc_ratio_20, intf_det->adjch_intf_stat.duty_cyc_ratio_40);
}

/******************************************************************************
 功能描述  : 信道扫描回调处理函数
******************************************************************************/
osal_void alg_intf_det_scan_chn_cb(const osal_void *param)
{
    alg_intf_det_stru *intf_det = &g_intf_det;
    wlan_scan_chan_stats_stru *chan_result = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_INTF_DET, ALG_GLA_USUAL_SWITCH);

    /* 判断入参合法性 */
    if (osal_unlikely(param == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_scan_chn_cb: input pointer is null!}");
        return;
    }

    hal_device = hal_chip_get_hal_device();
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        /* CCA扫描逻辑需要移出普通扫描流程，不然这里会出现vap删除之后hal device指针为空，仍进入该逻辑 */
        oam_warning_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_scan_chn_cb: hal_device is null!}");
        return;
    }
#if ALG_INTF_DET_DEBUG_MODE
    alg_intf_det_scan_result_debug(intf_det, hal_device);
#endif
    intf_det->statistic_update = OAL_TRUE;
    if ((intf_det->intf_det_mode & ALG_INTF_DET_MODE_COCHANNEL) != 0) {
        alg_intf_det_coch_identify(hal_device, intf_det);
    }

    chan_result = &(hal_device->chan_result);

    alg_intf_det_chan_result_info(intf_det, hal_device, OSAL_NULL, chan_result);

    if (switch_enable) {
        alg_intf_det_print_gla_info(intf_det);
    }
}

/******************************************************************************
 功能描述  : 设置CCA扫描参数
******************************************************************************/
osal_u32 alg_intf_det_set_cca_stat(mac_scan_req_stru *scan_params, const hmac_vap_stru *hmac_vap,
    const alg_intf_det_stru *intf_det)
{
    (osal_void)memset_s(scan_params, sizeof(mac_scan_req_stru), 0, sizeof(mac_scan_req_stru));

    /* 扫描获取第一个vap即可 */
    scan_params->vap_id = hmac_vap->vap_id;

    scan_params->bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->scan_type = WLAN_SCAN_TYPE_PASSIVE;
    scan_params->probe_delay = 0;
    scan_params->scan_func = MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS;
    scan_params->max_scan_count_per_channel = WLAN_DEFAULT_BG_SCAN_COUNT_PER_CHANNEL;
    scan_params->scan_time = intf_det->adjch_intf_info.chn_scan_cycle_ms;
    scan_params->scan_mode = WLAN_SCAN_MODE_BACKGROUND_ALG_INTF_DET;
    scan_params->channel_nums = 1;
    scan_params->channel_list[0] = hmac_vap->channel;

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 同频干扰识别函数
******************************************************************************/
osal_void alg_intf_det_adjch_stat_handler(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det)
{
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_btcoex_statistics_stru *btcoex_statistics;
    btcoex_statistics = hal_btcoex_statistics();
#endif
    alg_intf_det_prepare_adjch_param(hal_device, intf_det);
    /* sync error counter 清零 */
    hal_set_sync_err_counter_clear();
    /* T1定时器到期,使能寄存器:sync error counter */
    hal_enable_sync_error_counter(OSAL_TRUE);

    /* 若没有更新检测结果则将历史数据清0 */
    if (intf_det->statistic_update == OAL_FALSE) {
        hal_device->duty_ratio = 0;
    } else {
        intf_det->statistic_update = OAL_FALSE;
    }

    /* 记录bt工作时间起始值 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    intf_det->adjch_intf_stat.abort_time_us =
        /* 1000000:倍数 */
        btcoex_statistics->xzyvwRO4SwzRlvoO_ * 1000000 + btcoex_statistics->xGji_LwFd_GLBihwdC_;
#endif
}

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/******************************************************************************
 功能描述  : 邻频/叠频认证干扰识别
******************************************************************************/
OSAL_STATIC osal_bool alg_intf_det_is_adjintf_certify(const alg_intf_det_stru *intf_det)
{
    return ((intf_det->adjch_intf_stat.duty_cyc_ratio_20 >= ALG_INTF_DET_CERTIFY_RATIO_TH) ||
            (intf_det->adjch_intf_stat.duty_cyc_ratio_40 >= ALG_INTF_DET_CERTIFY_RATIO_TH));
}
/******************************************************************************
 功能描述  : 邻频/叠频强干扰识别
******************************************************************************/
OSAL_STATIC osal_bool alg_intf_det_is_adjintf_strong(alg_intf_det_stru *intf_det)
{
    osal_s8    delta_avg_rssi;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();

    if ((intf_det->adjch_intf_stat.duty_cyc_ratio_20 > intf_det->adjch_intf_info.strong_intf_ratio_th) ||
        (intf_det->adjch_intf_stat.duty_cyc_ratio_40 > intf_det->adjch_intf_info.strong_intf_ratio_th)) {
        return OAL_TRUE;
    }

    /* 工作带宽为20M，不采用rssi条件判断强干扰 */
    if (hal_device->wifi_channel_status.en_bandwidth == WLAN_BAND_WIDTH_20M) {
        return OAL_FALSE;
    }

    /* 主20M与主40M rssi均很小 */
    if ((intf_det->adjch_intf_stat.intf_det_avg_rssi_40 <= ALG_INTF_DET_ADJINTF_RSSI_THRES) &&
        (intf_det->adjch_intf_stat.intf_det_avg_rssi_20 <= ALG_INTF_DET_ADJINTF_RSSI_THRES)) {
        return OAL_FALSE;
    }

    delta_avg_rssi = (osal_s8)intf_det->adjch_intf_stat.intf_det_avg_rssi_40 -
        (osal_s8)intf_det->adjch_intf_stat.intf_det_avg_rssi_20;
    if (delta_avg_rssi >= ALG_INTF_DET_ADJINTF_RSSI_DIFF) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
/******************************************************************************
 功能描述  : 邻频/叠频中干扰识别
******************************************************************************/
OSAL_STATIC osal_bool alg_intf_det_is_adjintf_medium(const alg_intf_det_stru *intf_det)
{
    return ((intf_det->adjch_intf_stat.sync_err_ratio > intf_det->adjch_intf_info.intf_sync_err_th) &&
            (intf_det->adjch_intf_stat.intf_det_ilde_cnt > ALG_INTF_DET_IDLE_CNT_TH) &&
            (((intf_det->adjch_intf_stat.intf_det_avg_rssi_20 > intf_det->adjch_intf_info.intf_aveg_rssi_th) &&
            (intf_det->adjch_intf_stat.intf_det_avg_rssi_20 != 0)) ||
            ((intf_det->adjch_intf_stat.intf_det_avg_rssi_40 > intf_det->adjch_intf_info.intf_aveg_rssi_th) &&
            (intf_det->adjch_intf_stat.intf_det_avg_rssi_40 != 0))));
}
/******************************************************************************
 功能描述  : 邻频/叠频无干扰识别
******************************************************************************/
OSAL_STATIC osal_bool alg_intf_det_is_adjintf_no(const alg_intf_det_stru *intf_det)
{
    return ((intf_det->adjch_intf_stat.duty_cyc_ratio_20 <= intf_det->adjch_intf_info.nonintf_ratio_th) &&
            (intf_det->adjch_intf_stat.duty_cyc_ratio_40 <= intf_det->adjch_intf_info.nonintf_ratio_th));
}

/******************************************************************************
 功能描述  : 负增益检测
******************************************************************************/
OSAL_STATIC osal_void alg_intf_det_update_intf_mode(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det,
    alg_intf_det_handler_stru *handler, hal_alg_intf_det_mode_enum_uint8 cur_intf_mode)
{
    alg_intf_det_state_enum_uint8 cur_state = alg_intfmode_get_statemode(cur_intf_mode);
    handler->curr_mode_cnt++;
    alg_intf_det_update_duty_cycle(intf_det, handler, cur_state);

    /* 无干扰状态和认证状态，暂不做负增益检测 */
    if ((cur_intf_mode != HAL_ALG_INTF_DET_ADJINTF_NO) && (cur_intf_mode != HAL_ALG_INTF_DET_ADJINTF_CERTIFY)) {
        alg_intf_det_benefit_check(hal_device, intf_det, &cur_intf_mode);

        /* 快速检测周期为250ms，检测4个周期，检测结束后，恢复到1000ms */
        /* 此处可以优化，无干扰时无需重新设置 */
        if (handler->curr_mode_cnt == 4) { /* 4:cnt数 */
            intf_det->intf_det_cycle_ms = ALG_INTF_DET_CYCLE_MS;
            frw_create_timer_entry(&intf_det->intf_det_timer, alg_intf_det_timer_handler, intf_det->intf_det_cycle_ms,
                (osal_void *)(intf_det), OSAL_TRUE);
        }
    }

#if ALG_INTF_DET_DEBUG_MODE
    if (intf_det->intf_det_debug == OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ANTI_INTF,
            "{alg_intf_det_idy_adjch_intf:: intf type= [%d] [1:medium, 2:strong, 3:certify]!!!}",
            intf_det->adjch_intf_info.intf_type);
    } else if (intf_det->intf_det_debug == ALG_INTF_DET_SERIAL_DEBUG) {
#ifdef _PRE_WLAN_ALG_UART_PRINT
        wifi_printf("alg_intf_det_idy_adjch_intf:: intf type= [%d] [1:medium, 2:strong, 3:certify]!!!\n",
            intf_det->adjch_intf_info.intf_type);
#endif
    }
#endif
}

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
/*
 * 函 数 名   : alg_intf_det_sec20_calc
 * 功能描述   : 邻频叠频的干扰检测计算, 满足条件2G会降带宽
 * 1.日    期   : 2022年7月11日
 *   修改内容   : 新生成函数
 */
OSAL_STATIC void alg_intf_det_sec20_calc(alg_intf_det_stru *intf_det, hmac_vap_stru *hmac_vap)
{
    if ((intf_det->adjch_intf_info.intf_type >= HAL_ALG_INTF_DET_ADJINTF_STRONG) &&
        (intf_det->adjch_intf_stat.duty_cyc_ratio_20 <= ALG_INTF_ADJ_MIN_TH) &&
        (intf_det->adjch_intf_stat.duty_cyc_ratio_sec20 >= ALG_INTF_ADJ_MAX_TH)) {
        intf_det->adjch_intf_stat.intf_adj_cnt++;
        if (intf_det->adjch_intf_stat.intf_adj_cnt > ALG_INTF_ADJ_CNT_TH) {
            hmac_chan_start_40m_recovery_timer(hmac_vap);
            oam_warning_log2(hmac_vap->vap_id, OAM_SF_ANTI_INTF,
                             "{alg_intf_det_sec20_calc: sec20 interfere detected,"
                             "pri20 busy ratio is %d, sec20 busy ratio is %d, switch to 20Mhz}",
                             intf_det->adjch_intf_stat.duty_cyc_ratio_20,
                             intf_det->adjch_intf_stat.duty_cyc_ratio_sec20);
            hmac_chan_multi_switch_to_20mhz_ap(hmac_vap);
        }
    } else {
        intf_det->adjch_intf_stat.intf_adj_cnt = 0;
    }
}

/*
 * 函 数 名   : alg_intf_det_sec20_interfere
 * 功能描述   : 次20M干扰检测, 仅针对2G
 * 1.日    期   : 2022年7月11日
 *   修改内容   : 新生成函数
 */
OSAL_STATIC void alg_intf_det_sec20_interfere(alg_intf_det_stru *intf_det)
{
    hmac_vap_stru       *hmac_vap       = NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    uint8_t              vap_idx;
    oal_bool_enum_uint8  find_match_ap  = OAL_FALSE;

    for (vap_idx = 0; vap_idx < hmac_device->up_vap_num; vap_idx++) {
        hmac_vap = (hmac_vap_stru *)hmac_get_vap(vap_idx);
        if ((hmac_vap == NULL) || (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) ||
            (hmac_vap->vap_state != MAC_VAP_STATE_UP)) {
            continue;
        } else {
            find_match_ap = OAL_TRUE;
            break;
        }
    }
    if (osal_unlikely(hmac_vap == NULL || find_match_ap == OAL_FALSE)) {
        return;
    }
    if ((mac_mib_get_2040_switch_prohibited(hmac_vap) != OAL_FALSE) ||
        ((hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_40PLUS) &&
        (hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_40MINUS))) {
        return;
    }

    alg_intf_det_sec20_calc(intf_det, hmac_vap);
}

/*
 * 函 数 名   : alg_intf_det_bandwidth_switch_interfere
 * 功能描述   : 检测到干扰后降带宽的处理，目前只有2g 40M降20M
 */
OSAL_STATIC void alg_intf_det_bandwidth_switch_interfere(alg_intf_det_stru *intf_det)
{
    /* 对次20检测到强干扰，是否需要降带宽进行判断 */
    alg_intf_det_sec20_interfere(intf_det);
}
#endif
/******************************************************************************
 功能描述  : 邻频/叠频干扰识别
******************************************************************************/
OSAL_STATIC osal_u32 alg_intf_det_adjch_intf_type_idy(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det,
    alg_intf_det_handler_stru *handler, hal_alg_intf_det_mode_enum_uint8 *cur_intf_mode,
    alg_intf_det_state_enum_uint8 state)
{
    unref_param(hal_device);
    /* 判断是否是认证模式，认证情况下设成无干扰模式 */
    if (alg_intf_det_is_adjintf_certify(intf_det)) {
        *cur_intf_mode = HAL_ALG_INTF_DET_ADJINTF_CERTIFY;
        intf_det->adjch_intf_stat.cert_intf_cyc_cnt = 0;
    } else if (intf_det->adjch_intf_info.intf_type == HAL_ALG_INTF_DET_ADJINTF_CERTIFY) {
        if (intf_det->adjch_intf_stat.cert_intf_cyc_cnt < ALG_INTF_DET_NOCERT_INTF_CYCLE_TH) {
            intf_det->adjch_intf_stat.cert_intf_cyc_cnt += 1;
            intf_det->adjch_intf_stat.non_intf_cyc_cnt = 0;
            intf_det->adjch_intf_stat.middle_intf_cyc_cnt = 0;
            intf_det->adjch_intf_stat.strong_intf_cyc_cnt = 0;
            return OAL_SUCC;
        } else {
            /* 认证模式退出要求到无干扰状态 */
            *cur_intf_mode = HAL_ALG_INTF_DET_ADJINTF_NO;
        }
    } else if (alg_intf_det_is_adjintf_strong(intf_det)) {
        /* 判断强干扰，条件: 干扰繁忙度大于阈值 */
        intf_det->adjch_intf_stat.non_intf_cyc_cnt = 0;
        intf_det->adjch_intf_stat.middle_intf_cyc_cnt = 0;

        /* 连续检测到3次干扰才进入干扰模式 */
        if (intf_det->adjch_intf_stat.strong_intf_cyc_cnt <
            intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_STRONG]) {
            intf_det->adjch_intf_stat.strong_intf_cyc_cnt++;
            alg_intf_det_update_duty_cycle(intf_det, handler, state);
            return OAL_SUCC;
        }
        *cur_intf_mode = HAL_ALG_INTF_DET_ADJINTF_STRONG;
    } else if (alg_intf_det_is_adjintf_no(intf_det)) {
        intf_det->adjch_intf_stat.middle_intf_cyc_cnt = 0;
        intf_det->adjch_intf_stat.strong_intf_cyc_cnt = 0;

        if (intf_det->adjch_intf_stat.non_intf_cyc_cnt <
            intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_NO]) {
            intf_det->adjch_intf_stat.non_intf_cyc_cnt++;
            alg_intf_det_update_duty_cycle(intf_det, handler, state);
            return OAL_SUCC;
        }
        *cur_intf_mode = HAL_ALG_INTF_DET_ADJINTF_NO;
    } else if (alg_intf_det_is_adjintf_medium(intf_det)) {
        intf_det->adjch_intf_stat.non_intf_cyc_cnt = 0;
        intf_det->adjch_intf_stat.strong_intf_cyc_cnt = 0;
        /* 连续检测到3次干扰才进入干扰模式 */
        if (intf_det->adjch_intf_stat.middle_intf_cyc_cnt <
            intf_det->adjch_intf_info.intf_cyc_th[HAL_ALG_INTF_DET_ADJINTF_MEDIUM]) {
            intf_det->adjch_intf_stat.middle_intf_cyc_cnt++;
            alg_intf_det_update_duty_cycle(intf_det, handler, state);
            return OAL_SUCC;
        }
        *cur_intf_mode = HAL_ALG_INTF_DET_ADJINTF_MEDIUM;
    } else {
        intf_det->adjch_intf_stat.non_intf_cyc_cnt = 0;
    }
    return OAL_FAIL;
}

/******************************************************************************
 功能描述  : 邻频/叠频干扰识别
******************************************************************************/
osal_u32 alg_intf_det_idy_adjch_intf(hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det)
{
    hal_alg_intf_det_mode_enum_uint8 cur_intf_mode = intf_det->adjch_intf_info.intf_type;
    alg_intf_det_handler_stru *handler = &intf_det->intf_det_handler;
    alg_intf_det_state_enum_uint8 state;
    osal_u32 ret;

    state = alg_intfmode_get_statemode(cur_intf_mode);
    ret = alg_intf_det_adjch_intf_type_idy(hal_device, intf_det, handler, &cur_intf_mode, state);
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
    hmac_alg_cfg_intf_det_cca_chk_noise_notify(intf_det->adjch_intf_stat.intf_det_avg_rssi_20,
        intf_det->coch_intf_info.coch_intf_state_sta, cur_intf_mode);
#endif
    if (ret == OAL_SUCC) {
        return ret;
    }

    state = alg_intfmode_get_statemode(cur_intf_mode);
    if ((handler->oal_fsm.cur_state == ALG_INTF_DET_STATE_NOINTF) ||
        (handler->oal_fsm.cur_state == ALG_INTF_DET_STATE_DETINTF)) {
        alg_intf_det_state_trans(hal_device, handler, state, cur_intf_mode, ALG_INTF_DET_EVENT_DUTY_CYCLE);
        /* 该统计项，由状态机entry接口清零;
        另外。不可以一直累加取平均，因为这样会导致比较的时候不准确，因为时间会导致繁忙度不同
        当前统计次数为ALG_INTF_DET_STATISTIC_CNT_TH
        */
        alg_intf_det_update_intf_mode(hal_device, intf_det, handler, cur_intf_mode);
    } else {
        alg_intf_det_intf_type_change_notify(hal_device, intf_det, &intf_det->intf_det_notify,
            cur_intf_mode, ALG_INTF_DET_EVENT_DUTY_CYCLE);
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
    alg_intf_det_bandwidth_switch_interfere(intf_det); // 强干扰时可能降带宽
#endif
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 获取邻叠频干扰类型
******************************************************************************/
hal_alg_intf_det_mode_enum_uint8 alg_intf_det_get_curr_adjch_intf_type(hal_to_dmac_device_stru *hal_device)
{
    osal_u32 ret;
    alg_intf_det_stru *intf_det = OSAL_NULL;
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void **)&intf_det);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_intf_det_get_curr_adjch_intf_type: get intf det struct fail!}");
        return HAL_ALG_INTF_DET_ADJINTF_NO;
    }
    return intf_det->adjch_intf_info.intf_type;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* _PRE_WLAN_FEATURE_INTF_DET */
