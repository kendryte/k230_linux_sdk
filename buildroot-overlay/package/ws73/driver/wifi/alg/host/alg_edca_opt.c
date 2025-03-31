/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: edca option algorithm
 */
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_edca_opt.h"
#include "hmac_vap.h"
#include "alg_autorate.h"
#include "hmac_device.h"

#include "hal_device.h"

#include "wlan_msg.h"
#include "hmac_config.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "alg_transplant.h"
#ifdef _PRE_WLAN_FEATURE_INTF_DET
#include "alg_intf_det.h"
#endif
#include "alg_common_rom.h"
#include "alg_gla.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_EDCA_OPT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/******************************************************************************
  2 宏定义
******************************************************************************/
/* 同频干扰识别(EDCA优化)参数 */
#define ALG_EDCA_OPT_TIMER_CYCLE_CNT_MAX 150 /* 定时器周期计数器最大值 */

#define ALG_EDCA_OPT_RTS_NUM_TH 50             /* 判别同频干扰识别的rts总数最小值 */
#define ALG_EDCA_OPT_RTS_FAIL_RATIO_TH_TCP 130 /* tcp业务下判别同频干扰的rts fail率门限(千分之X) */
#define ALG_EDCA_OPT_RTS_FAIL_RATIO_TH_UDP 50  /* udp业务下判别同频干扰的rts fail率门限(千分之X) */
#define ALG_EDCA_OPT_RTS_FAIL_RATIO_TH_STA 120 /* sta模式下判别同频干扰的rts fail率门限(千分之X) */

#define ALG_EDCA_OPT_NONDIR_RATIO_TH_LIMIT 900 /* 判别同频干扰的non-direct占空比上限(千分之X) */
#define ALG_EDCA_OPT_NONDIR_RATIO_TH_TCP 250   /* tcp业务下判别同频干扰的non-direct占空比门限(千分之X) */
#define ALG_EDCA_OPT_NONDIR_RATIO_TH_UDP 120   /* udp业务下判别同频干扰的non-direct占空比门限(千分之X) */
#define ALG_EDCA_OPT_NONDIR_RATIO_TH_STA 150   /* sta模式下判别同频干扰的non-direct占空比门限(千分之X) */
#define ALG_EDCA_OPT_RX_RATIO_TH_STA 200       /* sta模式下动态判别门限的rx比值门限(千分之X) */
#define ALG_EDCA_OPT_TX_RATIO_TH_AP 80         /* ap模式下判别同频干扰的tx time占空比门限(千分之X) */
#define ALG_EDCA_OPT_TX_RATIO_TH_STA 5         /* sta模式下判别同频干扰的tx time占空比门限(千分之X) */

#define ALG_EDCA_OPT_NON_INTF_CYCLE_NUM_TH 4 /* 连续无干扰周期数 */
#define ALG_EDCA_OPT_INTF_CYCLE_NUM_TH_AP 5  /* AP连续有干扰周期数 */
#define ALG_EDCA_OPT_INTF_CYCLE_NUM_TH_STA 4 /* STA连续有干扰周期数 */

#define ALG_EDCA_OPT_ADJ_STEP_DEFAULT 1 /* AP-没有同频干扰下的调整档位数 */


#define ALG_EDCA_OPT_ADJ_STEP_CO_CH_UDP 2 /* AP-同频干扰下的下行UDP调整档位数 */
#define ALG_EDCA_OPT_ADJ_STEP_CO_CH_TCP 3 /* AP-同频干扰下的下行TCP调整档位数 */

#define ALG_EDCA_OPT_MAX_WEIGHT_STA 3 /* STA-weight系数最大值 */
#define ALG_EDCA_OPT_WEIGHT_STA 0     /* STA-weight系数值 */
#define ALG_EDCA_OPT_SMTH_SHIFT_BIT 2 /* 平滑系数shift bit */

#define ALG_WMM_QOS_PARAMS_HDR_LEN 8
#define ALG_WMM_QOSINFO_AND_RESV_LEN 2
#define ALG_WMM_AC_PARAMS_RECORD_LEN 4
#define ALG_EDCA_PARA_SET_COUNT_MASK 0x0F

#define ALG_EDCA_OPT_0X64_STAT_PERIOD_CNT 3         /* 对0x64参数进行发包数统计的周期编号 */
#define ALG_EDCA_OPT_0X21_STAT_PERIOD_CNT 5         /* 对0x21参数进行发包数统计的周期编号 */
#define ALG_EDCA_OPT_0X10_OR_0X66_STAT_PERIOD_CNT 7 /* 对0x10或0x66参数进行发包数统计的周期编号 */
#define ALG_EDCA_OPT_MAX_STAT_PERIOD_CNT 11         /* 进行发包数统计的最大周期编号 */

#define ALG_EDCA_DET_DEFAULT_TH_INDEX 0 /* 默认门限index */
#define ALG_EDCA_DET_OPT_TH_INDEX 1     /* 优化门限index */

#define ALG_EDCA_OPT_DEBUG /* debug mode宏 (打开sdt打印) */
#define ALG_EDCA_DET_DEBUG_MODE 1     /* debug调试宏 */
#define ALG_EDCA_DET_MIN_INTVAL_TH 2  /* 负增益场景，最小探测间隔 */
#define ALG_EDCA_DET_MAX_INTVAL_TH 16 /* 负增益场景，最大的探测间隔 */

#define ALG_EDCA_DET_NEG_NONPROBE_THR 10 /* 检测到负增益后，延迟N个周期切到非负增益场景探测 */
#define ALG_EDCA_DET_COLLISION_RATIO_TH 20 /* 碰撞率判断门限阈值 */
#define ALG_EDCA_DET_GOODPUT_LOSS_TH 8     /* goodput降低门限 */
#define ALG_EDCA_DET_COL_RATIO_SMOOTH_SCALE 2 /* 冲突因子的平滑系数 */
#define ALG_EDCA_DET_GOODPUT_SMOOTH_SCALE 2   /* goodput的平滑系数 */

#define ALG_EDCA_DET_TXOP_OPT_TX_THR_STA 300 // 200   /* sta模式下判断txop优化打开的tx time占空比(千分之x) */
#define ALG_TXOP_OPT_TCP_DOWN_RX_THR_STA 500 // 500   /* sta模式下判断txop优化打开的rx
// time占空比(千分之x),tcp下行打开txop */
#define ALG_TXOP_OPT_TCP_DOWN_TX_THR_STA 50  /* sta模式下判断txop优化打开的tx time占空比(千分之x)，tcp下行打开txop */

#define ALG_EDCA_OPT_AIFSN 1
#define ALG_EDCA_OPT_CWMIN 1
#define ALG_EDCA_OPT_CWMAX 2

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
#define ALG_EDCA_OPT_AIFSN_PK 0
#define ALG_EDCA_OPT_CWMIN_PK 1
#define ALG_EDCA_OPT_CWMAX_PK 2
#endif

#define ALG_EDCA_OPT_MPDU_TH_NO_INTF 200
#define ALG_EDCA_OPT_MPDU_TH_INTF 400
/******************************************************************************
  3 枚举定义
******************************************************************************/
/* 选择设置cwmax or cwmin */
typedef enum {
    ALG_EDCA_OPT_SET_QCWMIN = 0,   /* 设置cwmin寄存器 */
    ALG_EDCA_OPT_SET_QCWMAX = 1,   /* 设置cwmax寄存器  */
    ALG_EDCA_OPT_SET_QCW_BOTH = 2, /* 同时设置cwmin和cwmax寄存器  */
    ALG_EDCA_OPT_SET_QCW_BUTT
} alg_edca_opt_set_qcw_mode_enum;
typedef osal_u8 alg_edca_opt_set_qcw_enum_uint8;

/* CW参数配置类别 */
typedef enum {
    ALG_EDCA_OPT_CW_PARAM_0X64 = 0, /* 设置0x64参数 */
    ALG_EDCA_OPT_CW_PARAM_0X21 = 1, /* 设置0x21参数 */
    ALG_EDCA_OPT_CW_PARAM_0X10 = 2, /* 设置0x10参数 */
    ALG_EDCA_OPT_CW_PARAM_0X66 = 3, /* 设置0x66参数 */
    ALG_EDCA_OPT_CW_PARAM_BUTT
} alg_edca_opt_cw_param_enum;
typedef osal_u8 alg_edca_opt_cw_param_enum_uint8;

/* CW参数配置类别 */
typedef enum {
    ALG_EDCA_OPT_UP = 0,   /* 往大的方向探测 */
    ALG_EDCA_OPT_DOWN = 1, /* 往小的方向探测 */
} alg_edca_opt_drection_enum;
typedef osal_u8 alg_edca_opt_drection_enum_uint8;
/******************************************************************************
  5 函数声明
******************************************************************************/
#ifdef _PRE_WLAN_FEATURE_INTF_DET
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
OSAL_STATIC osal_void alg_edca_opt_sta_pk(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt,
    alg_intf_det_stru *intf_det);
OSAL_STATIC osal_u32 alg_edca_det_handler(osal_void);
#endif
OSAL_STATIC osal_void alg_edca_opt_sta(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt,
    const alg_intf_det_stru *intf_det);
OSAL_STATIC osal_void alg_edca_txop_opt(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt,
    alg_intf_det_stru *intf_det);
#endif

OSAL_STATIC osal_u8 alg_edca_opt_get_vap_num(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id);
OSAL_STATIC osal_u32 alg_edca_para_opt(const hmac_vap_stru *hmac_vap, const hal_to_dmac_device_stru *hal_device);
OSAL_STATIC osal_u8 alg_edca_opt_get_vap_num(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id);
OSAL_STATIC osal_u32 alg_mac_frame_check(const mac_tx_ctl_stru *cb);
OSAL_STATIC osal_u32 alg_edca_opt_tx_process(hmac_user_stru *hmac_user, mac_tx_ctl_stru *cb,
    hal_tx_txop_alg_stru *txop_param);
OSAL_STATIC osal_void alg_edca_det_clean_probe_pkt_info(alg_edca_opt_stru *edca_opt);
OSAL_STATIC osal_u32 alg_edca_opt_rx_mgmt_process(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_u32 alg_edca_opt_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
/******************************************************************************
  6 全局变量定义
******************************************************************************/
/**********************************************
    device级edca_opt结构体
**********************************************/
OSAL_STATIC alg_edca_opt_stru g_edca_opt;

// save ACs traffic num
osal_u16 g_edca_opt_triffic_num[WLAN_WME_AC_BUTT];

/******************************************************************************
  7 函数实现
******************************************************************************/
/*******************************************************************************
 函 数 名  : alg_edca_opt_init
 功能描述  : edca优化模块初始化
******************************************************************************/
osal_s32 alg_edca_opt_init(osal_void)
{
    alg_edca_opt_stru *edca_opt = OSAL_NULL;
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_internal_hook_stru* callback = hmac_alg_get_notify_if();
    ret = hmac_alg_register(HMAC_ALG_ID_EDCA_OPT);
    if (ret != OAL_SUCC) {
        return (osal_s32)ret;
    }

    hal_device = hal_chip_get_hal_device();
    edca_opt = &g_edca_opt;
    memset_s(edca_opt, sizeof(alg_edca_opt_stru), 0, sizeof(alg_edca_opt_stru));

    /* ap/sta 模式下的使能开关 */
    edca_opt->edca_opt_en_sta = WLAN_EDCA_OPT_MODE_STA;
    edca_opt->edca_opt_en_ap = WLAN_EDCA_OPT_MODE_AP;

    /* 初始化device的私有信息 */
    edca_opt->edca_opt_weight_sta = ALG_EDCA_OPT_WEIGHT_STA; // 暂时没有用到
    edca_opt->pk_debug_mode = EDCA_PK_DEBUG_MODE_OFF;
    edca_opt->edca_reg_optimized = OSAL_FALSE;
    edca_opt->txop_limit_en_sta = OSAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    edca_opt->thrpt_loss_det_en = OSAL_TRUE;
    edca_opt->negdet_probe_info.intvl_time_thr = ALG_EDCA_DET_MIN_INTVAL_TH;
    edca_opt->negdet_probe_info.intf_det_probe_stat = ALG_EDCA_DET_PROBE_STAT_OFF;
    edca_opt->negdet_probe_info.neg_nonprobe_thr = ALG_EDCA_DET_NEG_NONPROBE_THR;
    edca_opt->negdet_probe_info.probe_flag = ALG_EDCA_DET_PROBE_OFF;
    edca_opt->negdet_probe_info.last_probe_flag = ALG_EDCA_DET_PROBE_OFF;
    edca_opt->negdet_probe_info.collision_ratio_th = ALG_EDCA_DET_COLLISION_RATIO_TH;
    edca_opt->negdet_probe_info.goodput_loss_th = ALG_EDCA_DET_GOODPUT_LOSS_TH;
#endif

    /* 注册device私有信息 */
    hmac_alg_register_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_EDCA_OPT, (osal_void *)edca_opt);
    /* 注册钩子函数 */
    hmac_alg_register_rx_mgmt_notify_func(HMAC_ALG_RX_MGMT_EDCA_OPT, alg_edca_opt_rx_mgmt_process);
    hmac_alg_register_tx_notify_func(HMAC_ALG_TX_EDCA_OPT, alg_edca_opt_tx_process);
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
    hmac_alg_register_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_EDCA, alg_edca_intrf_mode_process_assoc);
#endif
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_EDCA, alg_edca_opt_config_param);
#endif
    callback->cfg_intf_det_edca_notify_func = alg_edca_det_handler;
    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_edca_opt_exit
 功能描述  : edca opt模块退出函数
******************************************************************************/
osal_void alg_edca_opt_exit(osal_void)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_edca_opt_stru *edca_opt = OSAL_NULL;
    alg_internal_hook_stru* callback = hmac_alg_get_notify_if();
    osal_u32 ret;

    ret = hmac_alg_unregister(HMAC_ALG_ID_EDCA_OPT);
    if (ret != OAL_SUCC) {
        return;
    }

    /* 注销钩子函数 */
    hmac_alg_unregister_rx_mgmt_notify_func(HMAC_ALG_RX_MGMT_EDCA_OPT);
    hmac_alg_unregister_tx_notify_func(HMAC_ALG_TX_EDCA_OPT);
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
    hmac_alg_unregister_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_EDCA);
#endif
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_EDCA);
#endif
    callback->cfg_intf_det_edca_notify_func = OSAL_NULL;
    hal_device = hal_chip_get_hal_device();
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_EDCA_OPT, (osal_void **)&edca_opt);
    if (ret != OAL_SUCC) {
        return;
    }

    /* 注销并释放device级别下的结构体 */
    hmac_alg_unregister_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_EDCA_OPT);
}

/******************************************************************************
函 数 名  :alg_edca_print_cw_gla_info
功能描述  : EDCA算法cw参数图形化维测打印
 ******************************************************************************/
osal_void alg_edca_print_cw_gla_info(osal_u8 ac_type, osal_u8 cwmax, osal_u8 cwmin)
{
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_EDCA, ALG_GLA_USUAL_SWITCH);
    if (switch_enable) {
        switch (ac_type) {
            case WLAN_WME_AC_BE:
                oam_warning_log2(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_BE]:cwmax=%d,cwmin=%d", cwmax, cwmin);
                break;

            case WLAN_WME_AC_BK:
                oam_warning_log2(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_BK]:cwmax=%d,cwmin=%d", cwmax, cwmin);
                break;

            case WLAN_WME_AC_VI:
                oam_warning_log2(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_VI]:cwmax=%d,cwmin=%d", cwmax, cwmin);
                break;

            case WLAN_WME_AC_VO:
                oam_warning_log2(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_VO]:cwmax=%d,cwmin=%d", cwmax, cwmin);
                break;

            default:
                return;
        }
    }
}

/******************************************************************************
函 数 名  :alg_edca_print_txop_gla_info
功能描述  : EDCA算法txop参数图形化维测打印
 ******************************************************************************/
OSAL_STATIC osal_void alg_edca_print_txop_gla_info(osal_u16 txop_be, osal_u16 txop_bk)
{
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_EDCA, ALG_GLA_USUAL_SWITCH);
    if (switch_enable) {
        oam_warning_log1(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_BE]:txop=%d", txop_be);
        oam_warning_log1(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_BK]:txop=%d", txop_bk);
    }
}

/******************************************************************************
函 数 名  :alg_edca_print_aifsn_all_gla_info
功能描述  : EDCA算法aifsn全部ac参数图形化维测打印
 ******************************************************************************/
osal_void alg_edca_print_aifsn_all_gla_info(osal_u8 be, osal_u8 bk, osal_u8 vi, osal_u8 vo)
{
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_EDCA, ALG_GLA_USUAL_SWITCH);
    if (switch_enable) {
        oam_warning_log1(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_BE]:aifsn=%d", be);
        oam_warning_log1(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_BK]:aifsn=%d", bk);
        oam_warning_log1(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_VI]:aifsn=%d", vi);
        oam_warning_log1(0, OAM_SF_EDCA, "[GLA][ALG][EDCA_VO]:aifsn=%d", vo);
    }
}

osal_u32 alg_edca_para_opt(const hmac_vap_stru *hmac_vap, const hal_to_dmac_device_stru *hal_device)
{
#ifdef _PRE_WLAN_FEATURE_INTF_DET
    alg_edca_opt_stru *edca_opt = OSAL_NULL;
    alg_intf_det_stru *intf_det = OSAL_NULL;
    osal_u32 retval;

    retval = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_EDCA_OPT, (osal_void **)&edca_opt);
    if (retval != OAL_SUCC) {
        return retval;
    }

    /* 获取intf det优化的算法结构体 */
    retval = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void **)&intf_det);
    if (retval != OAL_SUCC) {
        return retval;
    }
    /* txop参数优化 */
    alg_edca_txop_opt(hmac_vap, edca_opt, intf_det);

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    if (intf_det->pk_mode == OSAL_TRUE || edca_opt->pk_debug_mode != EDCA_PK_DEBUG_MODE_OFF) {
        if (intf_det->intf_det_handler.oal_fsm.cur_state == ALG_INTF_DET_STATE_PKADJ ||
            ((edca_opt->pk_debug_mode != EDCA_PK_DEBUG_COMPLETE) &&
            edca_opt->pk_debug_mode != EDCA_PK_DEBUG_MODE_OFF)) {
            alg_edca_opt_sta_pk(hmac_vap, edca_opt, intf_det);
        }
    } else {
        alg_edca_opt_sta(hmac_vap, edca_opt, intf_det);
    }
#else
    alg_edca_opt_sta(hmac_vap, edca_opt, intf_det);
#endif
#endif
    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_edca_opt_rx_mgmt_process
 功能描述  : 管理帧处理函数，针对BEACON帧作处理
******************************************************************************/
osal_u32 alg_edca_opt_rx_mgmt_process(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u8 vap_vld_id = 0xFF;
    oal_bool_enum_uint8 is_registered;
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    mac_ieee80211_qos_htc_frame_stru *frame_hdr = OSAL_NULL;

    unref_param(hmac_user);
    /* vap非up状态不做处理 */
    if (hmac_vap->vap_state != MAC_VAP_STATE_UP) {
        return OAL_SUCC;
    }

    /* 如果算法没被注册，直接返回 */
    is_registered = hmac_alg_is_registered(HMAC_ALG_ID_EDCA_OPT);
    if (is_registered == OSAL_FALSE) {
        return OAL_SUCC;
    }

    /* 获取接收控制结构体和mac帧头 */
    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(buf);
    frame_hdr = (mac_ieee80211_qos_htc_frame_stru *)(mac_get_rx_cb_mac_hdr(&rx_ctl->rx_info));
    /* 接收到beacon帧，edca参数优化 */
    if (frame_hdr->frame_control.sub_type != WLAN_BEACON) {
        return OAL_SUCC;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, 0, "vap_id[%d] {alg_edca_opt_rx_mgmt_process: hal_device null!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 未使能算法 */
    if (g_edca_opt.edca_opt_en_sta == OSAL_FALSE) {
        return OAL_SUCC;
    }

    if (alg_edca_opt_get_vap_num(hal_device, &vap_vld_id) != 1) { /* 1表示为当前处于up状态的vap个数 */
        return OAL_SUCC;
    }

    return alg_edca_para_opt(hmac_vap, hal_device);
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_void alg_edca_opt_config_txop_limit_sta_en(alg_edca_opt_stru *edca_opt,
    const mac_ioctl_alg_param_stru *alg_param, const hmac_vap_stru *hmac_vap)
{
    osal_u16 be = 0;
    osal_u16 bk = 0;
    edca_opt->txop_limit_en_sta = (osal_u8)alg_param->value;
    if (edca_opt->txop_limit_en_sta == 0) {
        be = (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BE);
        bk = (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BK);
        hal_vap_set_machw_txop_limit_bkbe(be, bk);
        alg_edca_print_txop_gla_info(be, bk);
    }
    oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_opt_config_param:TxopLimitEnSta:%d}", edca_opt->txop_limit_en_sta);
}

OSAL_STATIC osal_u32 alg_edca_opt_set_param(const mac_ioctl_alg_param_stru *alg_param, hmac_vap_stru *hmac_vap,
    alg_edca_opt_stru *edca_opt)
{
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_EDCA_OPT_STA_EN:
            edca_opt->edca_opt_en_sta = (osal_u8)alg_param->value;
            if (alg_param->value == OSAL_FALSE) {
                edca_opt->edca_reg_optimized = OSAL_FALSE;
                edca_opt->txop_flag = OSAL_FALSE;
            }
            oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_opt_config_param:edcaOptEnSta: %d}", edca_opt->edca_opt_en_sta);
            break;
        case MAC_ALG_CFG_TXOP_LIMIT_STA_EN:
            alg_edca_opt_config_txop_limit_sta_en(edca_opt, alg_param, hmac_vap);
            break;
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
        case MAC_ALG_CFG_EDCA_DET_COLLISION_TH:
            edca_opt->negdet_probe_info.collision_ratio_th = (osal_u8)alg_param->value;
            oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_opt_config_param::negdet collision ratio thr is:%d}",
                edca_opt->negdet_probe_info.collision_ratio_th);
            break;
        case MAC_ALG_CFG_EDCA_NEG_DET_NONPROBE_TH:
            edca_opt->negdet_probe_info.neg_nonprobe_thr = (osal_u8)alg_param->value;
            oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_opt_config_param::net_nonprobe_thr is: =%d}",
                edca_opt->negdet_probe_info.neg_nonprobe_thr);
            break;
#endif
        default:
            oam_warning_log0(0, OAM_SF_EDCA, "{alg_edca_opt_config_param:config param error!}");
            return OAL_FAIL;
    }
    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_edca_opt_config_param
 功能描述  : 单个参数配置函数
******************************************************************************/
osal_u32 alg_edca_opt_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    alg_edca_opt_stru *edca_opt = OSAL_NULL;
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u32 ret;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_EDCA_OPT_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_EDCA_OPT_END)) {
        return OAL_SUCC;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANTI_INTF, "{alg_edca_opt_config_param: get hal device failed}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_EDCA_OPT, (osal_void **)&edca_opt);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 配置对应参数 */
    return alg_edca_opt_set_param(alg_param, hmac_vap, edca_opt);
}
#endif

/******************************************************************************
 函 数 名  : alg_edca_opt_get_vap_num
 功能描述  : 获取当前处于up状态的vap个数
*****************************************************************************/
osal_u8 alg_edca_opt_get_vap_num(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_index;
    osal_u8 vap_vld_num = 0;
    osal_u8 vap_num;
    osal_u8     mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap == OSAL_NULL) {
            oam_warning_log1(0, OAM_SF_ANTI_INTF, "alg_edca_opt_get_vap_num::hmac_vap[%d] IS NULL.",
                             mac_vap_id[vap_index]);
            continue;
        }

        vap_vld_num++;
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
            *vap_id = hmac_vap->vap_id;
        }
    }

    return vap_vld_num;
}

#ifdef _PRE_WLAN_FEATURE_INTF_DET
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
OSAL_STATIC osal_void alg_edca_param_reset_pk(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt,
    hal_alg_pk_intf_stage_enum_uint8 pk_stage, oal_bool_enum_uint8 intf_flag)
{
    wlan_wme_ac_type_enum_uint8 ac_type;
    osal_u8 cwmax = 0;
    osal_u8 cwmin = 0;
    osal_u8 aifsn_bk = 0;
    osal_u8 aifsn_be = 0;
    osal_u8 aifsn_vi = 0;
    osal_u8 aifsn_vo = 0;

    oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_param_reset_pk::PK mode stage[%d].}", pk_stage);

    if (pk_stage == HAL_ALG_INTF_PKADJ_STAGE_TXOP || edca_opt->pk_debug_mode == EDCA_PK_DEBUG_ADJ_TXOP) {
        hal_vap_set_machw_txop_limit_bkbe(0x0, 0x0);
        alg_edca_print_txop_gla_info(0x0, 0x0);
        edca_opt->txop_optimized = OSAL_FALSE;
    } else if (pk_stage == HAL_ALG_INTF_PKADJ_STAGE_AIFSN || edca_opt->pk_debug_mode == EDCA_PK_DEBUG_ADJ_AIFSN) {
        /* 更新edca寄存器参数 */
        aifsn_bk = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BK);
        aifsn_be = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BE);
        aifsn_vi = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VI);
        aifsn_vo = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VO);
        hal_vap_set_machw_aifsn_all_ac(aifsn_bk, aifsn_be, aifsn_vi, aifsn_vo);
        alg_edca_print_aifsn_all_gla_info(aifsn_be, aifsn_bk, aifsn_vi, aifsn_vo);
    } else if (pk_stage == HAL_ALG_INTF_PKADJ_STAGE_CW || edca_opt->pk_debug_mode == EDCA_PK_DEBUG_ADJ_CW) {
        if (intf_flag == OSAL_TRUE || edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_HAS_COCH_ON) {
            for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
                hal_vap_set_edca_machw_cw(ALG_EDCA_OPT_CWMAX, ALG_EDCA_OPT_CWMIN, ac_type);
                alg_edca_print_cw_gla_info(ac_type, ALG_EDCA_OPT_CWMAX, ALG_EDCA_OPT_CWMIN);
            }
        } else {
            for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
                cwmax = (osal_u8)mac_mib_get_qap_edca_table_cwmax(hmac_vap, ac_type);
                cwmin = (osal_u8)mac_mib_get_qap_edca_table_cwmin(hmac_vap, ac_type);
                hal_vap_set_edca_machw_cw(cwmax, cwmin, ac_type);
                alg_edca_print_cw_gla_info(ac_type, cwmax, cwmin);
            }
        }
    } else {
        // error print
        oam_error_log1(0, OAM_SF_EDCA, "{alg_edca_param_reset_pk::PK mode error stage[%d].}", pk_stage);
    }

    return;
}

OSAL_STATIC osal_void alg_edca_param_opt_pk(alg_edca_opt_stru *edca_opt, hal_alg_pk_intf_stage_enum_uint8 pk_stage)
{
    regdomain_enum regdomain = REGDOMAIN_COMMON;
    osal_u8 *country_code = OSAL_NULL;       /* 国家字符串 */
    oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_param_opt_pk::PK mode stage[%d].}", pk_stage);

    if (pk_stage == HAL_ALG_INTF_PKADJ_STAGE_INIT) {
        return;
    } else if (pk_stage == HAL_ALG_INTF_PKADJ_STAGE_AIFSN) {
        /* PK模式下不调整AIFSN       */
    } else if (pk_stage == HAL_ALG_INTF_PKADJ_STAGE_TXOP) {
        /* PK模式下不调整TXOP */
        edca_opt->txop_optimized = OSAL_TRUE;
    } else if (pk_stage == HAL_ALG_INTF_PKADJ_STAGE_CW) {
        country_code = (osal_u8 *)hmac_regdomain_get_country_etc();
        regdomain = hwifi_get_regdomain_from_country_code(country_code);
        /* CE国家认证要求不调整EDCA */
        if (regdomain != REGDOMAIN_ETSI) {
            /* PK模式下仅调整BE队列的CW窗口 */
            /* 低于80M带宽，调整EDCA竞争参数为CW MAX 4,CW MIN 1,避免峰值情况下TX波动过大 */
            hal_vap_set_edca_machw_cw(4, ALG_EDCA_OPT_CWMIN_PK, WLAN_WME_AC_BE);
            alg_edca_print_cw_gla_info(WLAN_WME_AC_BE, 4, ALG_EDCA_OPT_CWMIN_PK); /* 该状态下EDCA调整cwmax为4 */
        }
    } else {
        // error print
        oam_error_log1(0, OAM_SF_EDCA, "{alg_edca_param_opt_pk::PK mode error stage[%d].}", pk_stage);
    }

    edca_opt->edca_reg_optimized = OSAL_TRUE;

    return;
}

osal_void alg_edca_opt_sta_pk(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt, alg_intf_det_stru *intf_det)
{
    oal_bool_enum_uint8 exist_intf;
    if (intf_det->need_recover == OSAL_TRUE || edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_PK_RECOVER_ON) {
        exist_intf = ((intf_det->coch_intf_info.coch_intf_state_sta == OSAL_TRUE) ||
            (intf_det->adjch_intf_info.intf_type != HAL_ALG_INTF_DET_ADJINTF_NO));
        alg_edca_param_reset_pk(hmac_vap, edca_opt, intf_det->pk_mode_stage, exist_intf);

        intf_det->need_recover = OSAL_FALSE;
    } else {
        if (intf_det->has_set == OSAL_FALSE || edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_PK_RECOVER_OFF) {
            alg_edca_param_opt_pk(edca_opt, intf_det->pk_mode_stage);
            intf_det->has_set = OSAL_TRUE;
        }
    }

    return;
}
#endif

osal_void alg_edca_param_reset(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt)
{
    wlan_wme_ac_type_enum_uint8 ac_type;
    osal_u8 cwmax = 0;
    osal_u8 cwmin = 0;
    osal_u8 aifsn_bk = 0;
    osal_u8 aifsn_be = 0;
    osal_u8 aifsn_vi = 0;
    osal_u8 aifsn_vo = 0;

    /* 更新edca寄存器参数 */
    aifsn_bk = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BK);
    aifsn_be = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BE);
    aifsn_vi = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VI);
    aifsn_vo = (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VO);
    hal_vap_set_machw_aifsn_all_ac(aifsn_bk, aifsn_be, aifsn_vi, aifsn_vo);
    alg_edca_print_aifsn_all_gla_info(aifsn_be, aifsn_bk, aifsn_vi, aifsn_vo);

    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        cwmax = (osal_u8)mac_mib_get_qap_edca_table_cwmax(hmac_vap, ac_type);
        cwmin = (osal_u8)mac_mib_get_qap_edca_table_cwmin(hmac_vap, ac_type);
        hal_vap_set_edca_machw_cw(cwmax, cwmin, ac_type);
        alg_edca_print_cw_gla_info(ac_type, cwmax, cwmin);
    }

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    if (edca_opt->txop_optimized == OSAL_TRUE) {
        hal_vap_set_machw_txop_limit_bkbe(0x0, 0x0);
        alg_edca_print_txop_gla_info(0x0, 0x0);
        edca_opt->txop_optimized = OSAL_FALSE;
    }
#endif
    edca_opt->edca_reg_optimized = OSAL_FALSE;

    return;
}

OSAL_STATIC osal_void alg_edca_param_opt(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt)
{
    wlan_wme_ac_type_enum_uint8 ac_type;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_EDCA, "{alg_edca_param_opt::hmac_vap is null.}");
        return;
    }

    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        /* BK不调整 */
        if (ac_type == WLAN_WME_AC_BK) {
            continue;
        }
        hal_vap_set_edca_machw_cw(ALG_EDCA_OPT_CWMAX, ALG_EDCA_OPT_CWMIN, ac_type);
        alg_edca_print_cw_gla_info(ac_type, ALG_EDCA_OPT_CWMAX, ALG_EDCA_OPT_CWMIN);
    }

    edca_opt->edca_reg_optimized = OSAL_TRUE;

    return;
}

/******************************************************************************
 函 数 名  : alg_edca_opt_sta
 功能描述  : sta模式下edca优化
******************************************************************************/
osal_void alg_edca_opt_sta(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt,
    const alg_intf_det_stru *intf_det)
{
    /* 修改为状态机后，看看这个变量edca_reg_optimized是否有用? */
    if (edca_opt->edca_reg_optimized == OSAL_TRUE) {
        if ((edca_opt->edca_opt_en_sta == OSAL_FALSE) ||
            (((intf_det->coch_intf_info.coch_intf_state_sta == OSAL_TRUE) ||
            (intf_det->adjch_intf_info.intf_type != HAL_ALG_INTF_DET_ADJINTF_NO) ||
            edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_HAS_COCH_ON ||
            edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_HAS_ADJCH_ON) == OSAL_FALSE)) {
            alg_edca_param_reset(hmac_vap, edca_opt);
        }
    } else {
        if ((edca_opt->edca_opt_en_sta == OSAL_TRUE) &&
            ((intf_det->coch_intf_info.coch_intf_state_sta == OSAL_TRUE) ||
            (intf_det->adjch_intf_info.intf_type != HAL_ALG_INTF_DET_ADJINTF_NO) ||
            edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_HAS_COCH_ON ||
            edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_HAS_ADJCH_ON)) {
            alg_edca_param_opt(hmac_vap, edca_opt);
        }
    }

    return;
}

OSAL_STATIC osal_void alg_edca_stop_txop_opt(const alg_intf_det_stru *intf_det, hmac_user_stru *hmac_user,
    alg_edca_opt_stru *edca_opt, const hmac_vap_stru *hmac_vap)
{
    osal_u16 be = 0;
    osal_u16 bk = 0;
    if (edca_opt->txop_flag == OSAL_TRUE) {
        be = (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BE);
        bk = (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BK);
        hal_vap_set_machw_txop_limit_bkbe(be, bk);

        edca_opt->txop_flag = OSAL_FALSE;

        /* 11abg 去使能txop内check cca */
        if ((hmac_user != OSAL_NULL) && (hmac_user->avail_protocol_mode <= WLAN_MIXED_TWO_11G_MODE)) {
            hal_set_txop_check_cca(OSAL_FALSE);
        }

        /* 关闭txop时，将原来的统计信息清除; 将探测间隔设置最小 */
        alg_edca_det_clean_probe_pkt_info(edca_opt);

        edca_opt->negdet_probe_info.intvl_time_thr = ALG_EDCA_DET_MIN_INTVAL_TH;
        /* 优化由开启到关闭，清除finish标记 */
        edca_opt->negdet_probe_stat.probe_finish[0] = OSAL_FALSE;
        edca_opt->negdet_probe_stat.probe_finish[1] = OSAL_FALSE;

        oam_warning_log3(0, OAM_SF_EDCA,
            "{alg_edca_txop_opt_sta::txop_limit_en[%d], txop_flag[%d], intf_stat[%d].}",
            edca_opt->txop_limit_en_sta, edca_opt->txop_flag,
            (intf_det->coch_intf_info.coch_intf_state_sta == OSAL_TRUE) ||
            (intf_det->adjch_intf_info.intf_type != HAL_ALG_INTF_DET_ADJINTF_NO));
        alg_edca_print_txop_gla_info(be, bk);
    }
}

OSAL_STATIC osal_void alg_edca_start_txop_opt(alg_intf_det_stru *intf_det, osal_u16 txop_set,
    oal_bool_enum_uint8 is_11bg_tcp_down, const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt)
{
    osal_u16 be = 0;
    osal_u16 bk = 0;

    unref_param(hmac_vap);

    hal_vap_get_machw_txop_limit_bkbe(&be, &bk);
    if ((edca_opt->txop_flag == OSAL_FALSE) || (be != txop_set) || (bk != txop_set)) {
        hal_vap_set_machw_txop_limit_bkbe(txop_set, txop_set);
        edca_opt->txop_flag = OSAL_TRUE;

        /* 11abg 使能txop内check cca */
        if (is_11bg_tcp_down == OSAL_TRUE) {
            hal_set_txop_check_cca(OSAL_TRUE);
        }

        oam_warning_log3(0, OAM_SF_EDCA,
            "{alg_edca_txop_opt_sta::txop enable_limit[%d], last_tx_duty[%d], tx_duty[%d].}", txop_set,
            intf_det->coch_intf_stat.tx_time_duty_cyc_last, intf_det->coch_intf_stat.tx_time_duty_cyc);
        alg_edca_print_txop_gla_info(txop_set, txop_set);
    }
}

OSAL_STATIC osal_void alg_update_txop(const hmac_vap_stru *hmac_vap, const alg_intf_det_stru *intf_det,
    osal_u16 *tx_time_duty_cyc, osal_u16 *txop_set)
{
    oal_bool_enum_uint8 vivo_exist =
        (g_edca_opt_triffic_num[WLAN_WME_AC_VI] + g_edca_opt_triffic_num[WLAN_WME_AC_VO] > 0) ?
        OSAL_TRUE : OSAL_FALSE;
    osal_u16 aus_txop_limit[WLAN_BAND_WIDTH_BUTT] = {
        5400, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008 };
    osal_u16 aus_txop_limit_1[WLAN_BAND_WIDTH_BUTT] = {
        2000, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008 };
    osal_u16 aus_txop_limit_2[WLAN_BAND_WIDTH_BUTT] = {
        1504, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008, 3008 };

    if ((intf_det->coch_intf_info.coch_intf_state_sta == OSAL_TRUE) ||
        (intf_det->adjch_intf_info.intf_type != HAL_ALG_INTF_DET_ADJINTF_NO)) {
        *tx_time_duty_cyc = ALG_EDCA_DET_TXOP_OPT_TX_THR_STA >> 1;
        *txop_set = aus_txop_limit[hmac_vap->channel.en_bandwidth];
        if (vivo_exist == OSAL_TRUE) {
            /* 有比BE更高优先级在发数据 */
            *txop_set = aus_txop_limit_2[hmac_vap->channel.en_bandwidth];
        }
        *txop_set = osal_min(*txop_set, 3008); /* 3008:取最小值 */
    } else {
        *tx_time_duty_cyc = ALG_EDCA_DET_TXOP_OPT_TX_THR_STA;
        *txop_set = aus_txop_limit[hmac_vap->channel.en_bandwidth];
        if (vivo_exist == OSAL_TRUE) {
            /* 有比BE更高优先级在发数据 */
            *txop_set = osal_min(aus_txop_limit_1[hmac_vap->channel.en_bandwidth], 3008); /* 3008:取最小值 */
        }
    }
    memset_s(g_edca_opt_triffic_num, sizeof(g_edca_opt_triffic_num), 0, sizeof(g_edca_opt_triffic_num));
}
/******************************************************************************
 函 数 名  : alg_edca_txop_opt
 功能描述  : sta模式下edca txop使能优化
******************************************************************************/
osal_void alg_edca_txop_opt(const hmac_vap_stru *hmac_vap, alg_edca_opt_stru *edca_opt, alg_intf_det_stru *intf_det)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u16 tx_time_duty_cyc_th = 0; /* 发送时间占空比门限 */
    osal_u16 txop_set = 0;
    oal_bool_enum_uint8 is_11bg_tcp_down = OSAL_FALSE; /* 是否为tcp下行，仅针对11ag */

    /* 1.如果此时已经有优化方案; 2.dbac running; 不做优化 */
    if ((hmac_device->txop_enable == OSAL_TRUE) || (mac_is_dbac_running(hmac_device) == OSAL_TRUE)) {
        return;
    }

    /* 存在干扰时,增加打开txop的概率 */
    alg_update_txop(hmac_vap, intf_det, &tx_time_duty_cyc_th, &txop_set);
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        return;
    }
    /* 11abg tcp下行打开txop，保证性能 */
    if ((hmac_user->avail_protocol_mode <= WLAN_MIXED_TWO_11G_MODE) &&
        (intf_det->coch_intf_stat.rx_direct_duty_cyc > ALG_TXOP_OPT_TCP_DOWN_RX_THR_STA) &&
        (intf_det->coch_intf_stat.rx_direct_duty_cyc_last > ALG_TXOP_OPT_TCP_DOWN_RX_THR_STA) &&
        (intf_det->coch_intf_stat.tx_time_duty_cyc_last > ALG_TXOP_OPT_TCP_DOWN_TX_THR_STA) &&
        (intf_det->coch_intf_stat.tx_time_duty_cyc > ALG_TXOP_OPT_TCP_DOWN_TX_THR_STA)) {
        is_11bg_tcp_down = OSAL_TRUE;
        txop_set = 2000; /* 2000:11abg txop设置为2ms */
    }
    /* TXOP优化开启 */
    if (((intf_det->coch_intf_stat.tx_time_duty_cyc_last > tx_time_duty_cyc_th) &&
        (intf_det->coch_intf_stat.tx_time_duty_cyc > tx_time_duty_cyc_th) &&
        (edca_opt->txop_limit_en_sta == OSAL_TRUE) && (edca_opt->negdet_probe_info.thrpt_loss_stat == OSAL_FALSE)) ||
        (is_11bg_tcp_down == OSAL_TRUE) ||
        (edca_opt->intf_debug_mode == EDCA_INTF_DEBUG_GOODPUT_NEG_OFF)) {
        alg_edca_start_txop_opt(intf_det, txop_set, is_11bg_tcp_down, hmac_vap, edca_opt);
    } else {
        alg_edca_stop_txop_opt(intf_det, hmac_user, edca_opt, hmac_vap);
    }
}
#endif

OSAL_STATIC osal_void alg_edca_inc_triffic_num(osal_u8 ac_type)
{
    if (ac_type >= WLAN_WME_AC_BUTT) {
        return;
    }
    g_edca_opt_triffic_num[ac_type]++;
}

/******************************************************************************
 函 数 名  : alg_mac_frame_check
 功能描述  : 多播帧、管理帧算法判断
******************************************************************************/
osal_u32 alg_mac_frame_check(const mac_tx_ctl_stru *cb)
{
    osal_u32 ret = OAL_FAIL;

    if (cb == OSAL_NULL) {
        return ret;
    }
    if ((cb->ismcast == OSAL_TRUE) || (mac_get_is_data_frame(cb) == OSAL_FALSE) ||
        (cb->probe_data_type == DMAC_USER_ALG_SMARTANT_NULLDATA_PROBE) ||
        (cb->ac == WLAN_WME_AC_VI) || (cb->ac == WLAN_WME_AC_VO)) {
        ret = OAL_SUCC;
    }

    return ret;
}

/******************************************************************************
 函 数 名  : alg_edca_opt_tx_process
 功能描述  : 统计业务流
******************************************************************************/
osal_u32 alg_edca_opt_tx_process(hmac_user_stru *hmac_user, mac_tx_ctl_stru *cb,
    hal_tx_txop_alg_stru *txop_param)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    unref_param(txop_param);

    /* 未使能算法 */
    if (g_edca_opt.edca_opt_en_sta == OSAL_FALSE) {
        return OAL_SUCC;
    }

    /* 多播帧、管理帧算法不设置速率 */
    if (alg_mac_frame_check(cb) == OAL_SUCC) {
        return OAL_SUCC;
    }

    /* STA: tx counts */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_EDCA, "{alg_edca_opt_tx_process: hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((cb->ac >= WLAN_WME_AC_BE) && (cb->ac <= WLAN_WME_AC_VO)) {
        alg_edca_inc_triffic_num(cb->ac);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
/******************************************************************************
 功能描述  : 依据业务类型获取数据统计指针
******************************************************************************/
OSAL_STATIC osal_u32 alg_edca_det_get_traffic_rate_info(alg_edca_opt_stru *edca_opt,
    wlan_wme_ac_type_enum_uint8 traffic_type)
{
    switch (traffic_type) {
        case WLAN_WME_AC_BE:
        case WLAN_WME_AC_BK:
            edca_opt->txac_info = &(edca_opt->txac_info_bebk);
            break;
        case WLAN_WME_AC_VI:
            edca_opt->txac_info = &(edca_opt->txac_info_vi);
            break;
        case WLAN_WME_AC_VO:
            edca_opt->txac_info = &(edca_opt->txac_info_vo);
            break;
        default:
            oam_error_log0(0, OAM_SF_EDCA, "{alg_intf_det_get_traffic_rate_info: traffic_type invalid!}");
            return OAL_FAIL;
    }

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 清除包统计信息
******************************************************************************/
osal_void alg_edca_det_clean_probe_pkt_info(alg_edca_opt_stru *edca_opt)
{
    wlan_wme_ac_type_enum_uint8 ac_index;
    frw_msg msg_to_device = {0};
    osal_s32 ret;

    /* 遍历ac，更新统计信息 */
    for (ac_index = WLAN_WME_AC_BK; ac_index <= WLAN_WME_AC_VO; ac_index++) {
        /* 根据业务，获取速率信息结构体指针 */
        if (alg_edca_det_get_traffic_rate_info(edca_opt, ac_index) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_EDCA, "{alg_intf_det_clean_probe_pkt_info: get_traffic_rate_info err!}");
        }
        if (edca_opt->txac_info == OSAL_NULL) {
            continue;
        }
        /* 清除全局统计 */
        edca_opt->txac_info->collision_cnt = 0;
        edca_opt->txac_info->tx_cnt = 0;
        edca_opt->txac_info->pkt_short_goodput_sum = 0;
    }

    /* 清除device侧数据统计 */
    frw_msg_init(OSAL_NULL, 0, OSAL_NULL, 0, &msg_to_device);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_CLEAR_EDCA_STAT, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_det_clean_probe_pkt_info:: send msg to device fail=%d!", ret);
        return;
    }
}

/******************************************************************************
 功能描述  : 计算goodput平滑切换
******************************************************************************/
static inline osal_u32 alg_edca_det_smooth_goodput(osal_u32 avg_gp, osal_u32 instant_gp, osal_u32 scale)
{
    return ((((osal_u32)((avg_gp) * ((1 << (scale)) - 1))) >> (scale)) + ((instant_gp) >> ((scale))));
}

/******************************************************************************
 功能描述  : 计算速率平滑切换
******************************************************************************/
static inline osal_u16 alg_edca_det_smooth_col_ratio(osal_u16 avg_ratio, osal_u16 instant_ratio, osal_u16 scale)
{
    return ((((osal_u16)((avg_ratio) * ((1 << (scale)) - 1))) >> (scale)) + ((instant_ratio) >> ((scale))));
}

/******************************************************************************
 功能描述  : 计算碰撞率和goodput
******************************************************************************/
OSAL_STATIC osal_void alg_edca_det_calc_stat_info(alg_edca_opt_stru *edca_opt, wlan_wme_ac_type_enum_uint8 traffic_type,
    osal_u8 th_idx)
{
    frw_msg msg_to_device = {0};
    osal_s32 ret;
    alg_edca_det_ac_info_stru edca_det_ac_stat;
    /* 根据业务，获取速率信息结构体指针 */
    if (alg_edca_det_get_traffic_rate_info(edca_opt, traffic_type) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_EDCA, "{alg_intf_det_calc_stat_info: get_traffic_rate_info err!}");
        return;
    }

    /* 读取device侧碰撞率和goodput的接口 */
    memset_s(&edca_det_ac_stat, sizeof(alg_edca_det_ac_info_stru), 0, sizeof(alg_edca_det_ac_info_stru));
    frw_msg_init(&traffic_type, sizeof(traffic_type), (osal_u8 *)&edca_det_ac_stat,
        sizeof(alg_edca_det_ac_info_stru), &msg_to_device);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_GET_EDCA_STAT, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_EDCA, "{alg_edca_det_calc_stat_info:: send msg to device fail=%d!", ret);
        return;
    }

    edca_opt->txac_info->collision_cnt = edca_det_ac_stat.collision_cnt;
    edca_opt->txac_info->pkt_short_goodput_sum = edca_det_ac_stat.pkt_short_goodput_sum;
    edca_opt->txac_info->tx_cnt = edca_det_ac_stat.tx_cnt;

    /* 计算碰撞率和平均碰撞率 */
    if (edca_opt->txac_info->tx_cnt == 0) {
        edca_opt->txac_info->collision_ratio[th_idx] = 0;
        edca_opt->txac_info->avg_goodput[th_idx] = 0;
    } else {
        edca_opt->txac_info->collision_ratio[th_idx] = (osal_u16)alg_edca_det_smooth_col_ratio(
            edca_opt->txac_info->collision_ratio[th_idx],
            (osal_u16)((edca_opt->txac_info->collision_cnt << 10) / edca_opt->txac_info->tx_cnt), /* 10:左移10 */
            ALG_EDCA_DET_COL_RATIO_SMOOTH_SCALE);

        edca_opt->txac_info->avg_goodput[th_idx] = alg_edca_det_smooth_goodput(
            edca_opt->txac_info->avg_goodput[th_idx],
            (edca_opt->txac_info->pkt_short_goodput_sum << 10) / edca_opt->txac_info->tx_cnt, /* 10:左移10 */
            ALG_EDCA_DET_GOODPUT_SMOOTH_SCALE);
    }

    /* 清除全局统计 */
    edca_opt->txac_info->collision_cnt = 0;
    edca_opt->txac_info->tx_cnt = 0;
    edca_opt->txac_info->pkt_short_goodput_sum = 0;
}

/******************************************************************************
 功能描述  : goodput比较
******************************************************************************/
static inline osal_u8 alg_edca_det_goodput_comp(osal_u32 goodput1, osal_u32 goodput2, osal_u8 goodput_th)
{
    return (((goodput2) != 0) && ((goodput2) <= ((goodput1) * (100 - (goodput_th)) / 100)));  /* 100：百分比 */
}

/******************************************************************************
 功能描述  : 速率比较
******************************************************************************/
static inline osal_u8 alg_edca_det_collisopn_ratio_comp(osal_u16 ratio1, osal_u16 ratio2, osal_u8 ratio_th)
{
    return (((ratio2) != 0) && ((ratio2) >= ((ratio1) + (ratio_th))));
}

OSAL_STATIC osal_void alg_edca_det_perform_det_probe_info(alg_edca_opt_stru *edca_opt,
    oal_bool_enum_uint8 cur_loss_stat, osal_u8 th_idx)
{
    wlan_wme_ac_type_enum_uint8 ac_index;
    oal_bool_enum_uint8 cur_loss_stat_tmp = cur_loss_stat;

    /* 遍历AC，更新统计信息 */
    for (ac_index = WLAN_WME_AC_BE; ac_index <= WLAN_WME_AC_VO; ac_index++) {
        /* TXOP使能仅针对BE业务进行负增益检测 */
        if (((edca_opt->negdet_probe_info.probe_flag & ALG_EDCA_DET_PROBE_TXOP) != 0) && (ac_index != WLAN_WME_AC_BE)) {
            continue;
        }

        alg_edca_det_calc_stat_info(edca_opt, ac_index, th_idx);

        /* 1)如果当前处于探测间隔期间，则只更新统计量，而不进行负增益的判断 */
        /* 2)探测过程中，如果任何一个user_ac识别为负增益，则后续不进行负增益的判断 */
        if ((edca_opt->negdet_probe_info.intf_det_probe_stat == ALG_EDCA_DET_PROBE_STAT_DEFAULT_THR) ||
            (edca_opt->negdet_probe_info.intf_det_probe_stat == ALG_EDCA_DET_PROBE_STAT_BEGIN) ||
            (cur_loss_stat_tmp == OSAL_TRUE)) {
            continue;
        }

        /* 识别负增益场景, 任何一个用户的tid业务判断为负增益，则识别当前场景为负增益 */
        if ((alg_edca_det_collisopn_ratio_comp(edca_opt->txac_info->collision_ratio[ALG_EDCA_DET_DEFAULT_TH_INDEX],
            edca_opt->txac_info->collision_ratio[ALG_EDCA_DET_OPT_TH_INDEX],
            edca_opt->negdet_probe_info.collision_ratio_th) == OSAL_TRUE) &&
            (alg_edca_det_goodput_comp(edca_opt->txac_info->avg_goodput[ALG_EDCA_DET_DEFAULT_TH_INDEX],
            edca_opt->txac_info->avg_goodput[ALG_EDCA_DET_OPT_TH_INDEX],
            edca_opt->negdet_probe_info.goodput_loss_th) == OSAL_TRUE)) {
            cur_loss_stat_tmp = OSAL_TRUE;
            edca_opt->negdet_probe_info.intvl_time_thr = ALG_EDCA_DET_MIN_INTVAL_TH;
            break;
        }
    }

    /* 如果当前处于探测间隔期间, 不进行负增益场景的惩罚 */
    if (edca_opt->negdet_probe_info.intf_det_probe_stat == ALG_EDCA_DET_PROBE_STAT_FINISH) {
        edca_opt->negdet_probe_info.thrpt_loss_stat = cur_loss_stat_tmp;

        /* 指示当前状态探测完成 */
        edca_opt->negdet_probe_stat.probe_finish[th_idx] = OSAL_TRUE;

        /* 若当前的环境较好，可以考虑扩大判断间隔; 检测到负增益后进行负增益惩罚，延迟探测 */
        if ((edca_opt->negdet_probe_info.thrpt_loss_stat == OSAL_FALSE) &&
            (edca_opt->negdet_probe_info.intvl_time_thr < ALG_EDCA_DET_MAX_INTVAL_TH)) {
            edca_opt->negdet_probe_info.intvl_time_thr <<= 1;
        } else if (edca_opt->negdet_probe_info.thrpt_loss_stat == OSAL_TRUE) {
            edca_opt->negdet_probe_stat.neg_nonprobe_cnt = 0;
        }
    }
}
#ifdef _PRE_WLAN_FEATURE_INTF_DET
/******************************************************************************
 功能描述  : 负增益优化
******************************************************************************/
OSAL_STATIC osal_u32 alg_edca_det_perform_det(const hal_to_dmac_device_stru *hal_device, alg_intf_det_stru *intf_det,
    alg_edca_opt_stru *edca_opt)
{
    osal_u8 th_idx = 0;
    hmac_vap_stru *hmac_vap;
    osal_u8 up_vap_num;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    oal_bool_enum_uint8 cur_loss_stat = OSAL_FALSE;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    unref_param(intf_det);

    /* 判断场景使用的门限: 0为初始默认值, 1为优化后的值 */
    if ((edca_opt->negdet_probe_info.probe_flag & ALG_EDCA_DET_PROBE_TXOP) != 0) {
        th_idx = (edca_opt->txop_flag == OSAL_FALSE) ? 0 : 1;
    } else {
        return OAL_SUCC;
    }

    /* 默认只在单STA下开启txop使能，DBAC下不开启 */
    if (mac_is_dbac_running(hmac_device) == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 只统计单VAP下的信息 */
    up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[0]);
    if ((up_vap_num != 1) || (hmac_vap == OSAL_NULL) || (!is_legacy_sta(hmac_vap))) {
        return OAL_SUCC;
    }

    /* 识别负增益场景,根据识别结果做处理 */
    alg_edca_det_perform_det_probe_info(edca_opt, cur_loss_stat, th_idx);

#if ALG_EDCA_DET_DEBUG_MODE
    if (edca_opt->thrpt_loss_det_debug == OSAL_TRUE) {
        oam_warning_log4(0, OAM_SF_EDCA,
            "{alg_intf_det_perform_det:thrpt_loss_stat[%d], intvl_time_thr[%d], probe_stat[%d], probe_index[%d].}",
            edca_opt->negdet_probe_info.thrpt_loss_stat, edca_opt->negdet_probe_info.intvl_time_thr,
            edca_opt->negdet_probe_info.intf_det_probe_stat, th_idx);
        oam_warning_log4(0, OAM_SF_EDCA,
            "{alg_intf_det_perform_det:collision0[%d], collision1[%d], avg_goodput0[%d], avg_goodput1[%d].}",
            edca_opt->txac_info->collision_ratio[ALG_EDCA_DET_DEFAULT_TH_INDEX],
            edca_opt->txac_info->collision_ratio[ALG_EDCA_DET_OPT_TH_INDEX],
            edca_opt->txac_info->avg_goodput[ALG_EDCA_DET_DEFAULT_TH_INDEX],
            edca_opt->txac_info->avg_goodput[ALG_EDCA_DET_OPT_TH_INDEX]);
    }
#endif

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 抗干扰探测状态更新
******************************************************************************/
OSAL_STATIC osal_void alg_edca_det_probe_stat_update(const alg_intf_det_stru *intf_det, alg_edca_opt_stru *edca_opt)
{
    /* 1.如果当前已经开启了算法使能,防止负增益下得不到及时反馈; */
    /* 2.或者当前的发包量超过阈值;  则需要进行探测 */
    if ((edca_opt->txop_flag == OSAL_TRUE) ||
        (intf_det->coch_intf_stat.tx_time_duty_cyc_last > ALG_EDCA_DET_TXOP_OPT_TX_THR_STA) ||
        (intf_det->coch_intf_stat.tx_time_duty_cyc > ALG_EDCA_DET_TXOP_OPT_TX_THR_STA)) {
        /* probe flag可后面继续优化其他算法的探测 */
        edca_opt->negdet_probe_info.probe_flag = ALG_EDCA_DET_PROBE_TXOP;
    } else {
        /* tx time不满足，默认关闭算法探测 */
        edca_opt->negdet_probe_info.probe_flag = ALG_EDCA_DET_PROBE_OFF;
    }

    if (edca_opt->negdet_probe_info.probe_flag != ALG_EDCA_DET_PROBE_OFF) {
        if (edca_opt->negdet_probe_stat.intvl_time_cnt == 0) {
            edca_opt->negdet_probe_info.intf_det_probe_stat = ALG_EDCA_DET_PROBE_STAT_DEFAULT_THR;
        } else if (edca_opt->negdet_probe_stat.intvl_time_cnt <= edca_opt->negdet_probe_info.intvl_time_thr) {
            /* 期间进行一次干扰检测的探测，并设置对应的门限 */
            edca_opt->negdet_probe_info.intf_det_probe_stat = ALG_EDCA_DET_PROBE_STAT_BEGIN;
        } else {
            /* 探测结果输出 */
            edca_opt->negdet_probe_info.intf_det_probe_stat = ALG_EDCA_DET_PROBE_STAT_FINISH;
            edca_opt->negdet_probe_stat.intvl_time_cnt = 0;
        }
        edca_opt->negdet_probe_stat.intvl_time_cnt++;
    } else {
        edca_opt->negdet_probe_stat.intvl_time_cnt = 0;
        edca_opt->negdet_probe_info.intf_det_probe_stat = ALG_EDCA_DET_PROBE_STAT_OFF;
    }

#if ALG_EDCA_DET_DEBUG_MODE
    if (edca_opt->thrpt_loss_det_debug == OSAL_TRUE) {
        oam_warning_log4(0, OAM_SF_EDCA,
            "{alg_intf_det_probe_stat_update:probe_stat[%d], probe_flag[%d], probe_cnt[%d], probe_cycle[%d].}",
            edca_opt->negdet_probe_info.intf_det_probe_stat, edca_opt->negdet_probe_info.probe_flag,
            edca_opt->negdet_probe_stat.intvl_time_cnt, edca_opt->negdet_probe_info.intvl_time_thr);
    }
#endif
}
#endif
/******************************************************************************
 功能描述  : 负增益计算定期处理函数
******************************************************************************/
osal_u32 alg_edca_det_handler(osal_void)
{
    osal_u32 ret;
    alg_edca_opt_stru *edca_opt = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_INTF_DET
    alg_intf_det_stru *intf_det = OSAL_NULL;
#endif
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    hal_device = hal_chip_get_hal_device();
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_EDCA_OPT, (osal_void **)&edca_opt);
    if (ret != OAL_SUCC) {
        return ret;
    }
#ifdef _PRE_WLAN_FEATURE_INTF_DET
    /* 获取intf det优化的算法结构体 */
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void **)&intf_det);
    if (ret != OAL_SUCC) {
        return ret;
    }
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    /* 负增益惩罚，延迟probe、取消负增益标记 */
    if (edca_opt->negdet_probe_info.thrpt_loss_stat == OSAL_TRUE) {
        if (edca_opt->negdet_probe_stat.neg_nonprobe_cnt >= edca_opt->negdet_probe_info.neg_nonprobe_thr) {
            edca_opt->negdet_probe_stat.neg_nonprobe_cnt = 0;
            edca_opt->negdet_probe_info.thrpt_loss_stat = OSAL_FALSE;
        } else {
            edca_opt->negdet_probe_stat.neg_nonprobe_cnt++;
        }
    }

    /* 负增益场景下，未达到间隔周期则不进行干扰检测; 有干扰也不进行探测 */
    if ((edca_opt->thrpt_loss_det_en == OSAL_TRUE) &&
        (edca_opt->negdet_probe_info.thrpt_loss_stat == OSAL_FALSE) &&
        (!((intf_det->coch_intf_info.coch_intf_state_sta == OSAL_TRUE) ||
        (intf_det->adjch_intf_info.intf_type != HAL_ALG_INTF_DET_ADJINTF_NO)))) {
            alg_edca_det_probe_stat_update(intf_det, edca_opt);

        if (edca_opt->negdet_probe_info.intf_det_probe_stat != ALG_EDCA_DET_PROBE_STAT_OFF) {
            alg_edca_det_perform_det(hal_device, intf_det, edca_opt);
        }
    }
#endif
#endif

    return OAL_SUCC;
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_EDCA_OPT */
