/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm cca optimaize
 */

#ifdef _PRE_WLAN_FEATURE_CCA_OPT
/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_cca_optimize.h"
#include "hmac_vap.h"
#include "hmac_alg_if.h"
#include "hmac_device.h"

#include "hal_device.h"

#include "hmac_config.h"
#ifdef _PRE_WLAN_FEATURE_INTF_DET
#include "alg_intf_det.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hal_rf.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "oal_ext_if.h"
#include "alg_common_rom.h"
#include "alg_gla.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_CCA_OPTIMIZE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
  2 宏定义
******************************************************************************/
#define ALG_CCA_OPT_ED_HIGH_40TH_LOW_TH (-79)
#define ALG_CCA_OPT_ED_OFST (5)

#define ALG_CCA_OPT_STRONG_INTF_ADJ_STEP 5    /* CCA 检测门限在强干扰下的调整幅值 */
#define ALG_CCA_OPT_MEDIUM_INTF_ADJ_STEP_20 4 /* CCA 20M检测门限在中等干扰下的调整幅值 */
#define ALG_CCA_OPT_MEDIUM_INTF_ADJ_STEP_40 5 /* CCA 40M检测门限在中等干扰下的调整幅值 */

#define ALG_CCA_OPT_ED_HIGH_INTF_TH (-62)     /* CCA 能量门限在底噪大于该阈值时需调整 */
#define ALG_CCA_OPT_ED_HIGH_INTF_ADJUST 5   /* CCA 能量门限在底噪大于阈值时的调整幅值 */
#define ALG_CCA_OPT_ED_HIGH_INTF_ENTER_TH 3 /* CCA 进入高底噪判断条件 */
#define ALG_CCA_OPT_ED_HIGH_INTF_EXIT_TH 5  /* CCA 退出高底噪判断条件 */

/******************************************************************************
  3 STRUCT定义
******************************************************************************/
/* CCA参数配置类型 */
typedef enum {
    ALG_CCA_SET_ED20 = 0,    /* 设置ED20门限 */
    ALG_CCA_SET_ED40 = 1,    /* 设置ED40门限 */
    ALG_CCA_SET_DSSS = 2,    /* 设置DSSS门限 */
    ALG_CCA_SET_OFDM = 3,    /* 设置OFDM门限 */
    ALG_CCA_SET_DEFAUL = 4,  /* 恢复缺省门限 */
    ALG_CCA_SET_BUTT = 5,
} alg_cca_set_param_type_enum;
/******************************************************************************
  4 函数声明
******************************************************************************/
OSAL_STATIC osal_u32 alg_cca_opt_rx_data_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf, hal_rx_statistic_stru *rx_stats);
OSAL_STATIC osal_u32 alg_cca_opt_rx_mgmt_process(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf);
OSAL_STATIC osal_u32 alg_cca_opt_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_u32 alg_cca_opt_change_channel_num_process(hmac_vap_stru *hmac_vap,
    mac_alg_channel_bw_chg_type_uint8 type);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_u32 alg_cca_opt_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
OSAL_STATIC osal_u32 alg_cca_opt_intf_notify_process(hal_to_dmac_device_stru *hal_device, osal_u8 old_intf_mode,
    osal_u8 cur_intf_mode);
OSAL_STATIC osal_void alg_cca_opt_rx_chk_noise(osal_s8 avg_rssi_20, oal_bool_enum_uint8 coch_intf_state_sta,
    osal_u8 cur_intf);
/******************************************************************************
  5 全局变量定义
******************************************************************************/
/*************************************************
    device级CCA结构体
**************************************************/
static alg_cca_opt_stru g_cca_opt;

/******************************************************************************
  6 函数实现
******************************************************************************/
/******************************************************************************
 功能描述  : CCA门限优化模块初始化
******************************************************************************/
/*
 * 函 数 名   : alg_cca_opt_set_th_default
 * 功能描述   : 在不同工作场景下计算cca门限（函数拆分）
 */
osal_void alg_cca_opt_set_th_default(alg_cca_opt_stru *cca_opt, wlan_channel_band_enum_uint8 freq_band)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_cfg_custom_cca_stru *cfg_cus_cca = OSAL_NULL;
    osal_s8 ed_20_default;
    osal_s8 ed_40_default;
#endif

    cca_opt->sd_cca_20th_dsss = HAL_CCA_OPT_ED_LOW_TH_DSSS_DEF;
    cca_opt->sd_cca_20th_ofdm = HAL_CCA_OPT_ED_LOW_TH_OFDM_DEF;
    cca_opt->ed_high_20th_reg = HAL_CCA_OPT_ED_HIGH_20TH_DEF;
    cca_opt->ed_high_40th_reg = HAL_CCA_OPT_ED_HIGH_40TH_DEF;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_config_get_cus_cca_param(&cfg_cus_cca);
    ed_20_default = (osal_s8)fe_rf_get_customized_cca_ed(CCA_ED_TYPE_20TH, freq_band);
    ed_40_default = (osal_s8)fe_rf_get_customized_cca_ed(CCA_ED_TYPE_40TH, freq_band);
    cca_opt->ed_high_20th_reg_default = ed_20_default;
    cca_opt->ed_high_40th_reg_default = ed_40_default;
    cca_opt->ed_high_20th_reg = cca_opt->ed_high_20th_reg_default;
    cca_opt->ed_high_40th_reg = cca_opt->ed_high_40th_reg_default;
#endif
}

osal_s32 alg_cca_opt_init(osal_void)
{
    alg_cca_opt_stru *cca_opt = OSAL_NULL;
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_internal_hook_stru* callback = hmac_alg_get_notify_if();

    ret = hmac_alg_register(HMAC_ALG_ID_CCA_OPT);
    if (ret != OAL_SUCC) {
        return (osal_s32)ret;
    }

    hal_device = hal_chip_get_hal_device();
    cca_opt = &g_cca_opt;
    memset_s(cca_opt, sizeof(alg_cca_opt_stru), 0, sizeof(alg_cca_opt_stru));

    /* 初始化device的私有信息 */
    cca_opt->is_cca_opt_registered = OSAL_TRUE;
    cca_opt->ed_high_20th_reg_adj_debug = 0;
    cca_opt->cca_opt_debug_mode = OSAL_FALSE;
    cca_opt->cca_opt_en = WLAN_CCA_OPT_ENABLE;
    cca_opt->rx_data_rssi = OAL_RSSI_INIT_MARKER;
    cca_opt->rx_mgmt_rssi = OAL_RSSI_INIT_MARKER;

    /* init use 2g */
    alg_cca_opt_set_th_default(cca_opt, WLAN_BAND_2G);

    /* 注册device私有信息 */
    ret = hmac_alg_register_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_CCA_OPT, (osal_void *)cca_opt);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CCA_OPT, "{alg_cca_opt_init: dmac_alg_register_device_priv_stru fail!}");
    }

    /* register rx hook, calc rx mgmt rssi & data rssi */
    hmac_alg_register_rx_notify_func(HMAC_ALG_RX_CCA_OPT, alg_cca_opt_rx_data_process);
    hmac_alg_register_rx_mgmt_notify_func(HMAC_ALG_RX_MGMT_CCA_OPT, alg_cca_opt_rx_mgmt_process);
    hmac_alg_register_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_CCA, alg_cca_opt_user_process);
    hmac_alg_register_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_CCA, alg_cca_opt_user_process);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_CCA_OPT, alg_cca_opt_config_param);
#endif
    callback->cfg_intf_det_cca_notify_func = alg_cca_opt_intf_notify_process;
    callback->cfg_intf_det_cca_chk_noise_notify_func = alg_cca_opt_rx_chk_noise;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hmac_alg_register_cfg_channel_notify_func(HMAC_ALG_CFG_CHANNEL_NOTIFY_CCA_OPT,
        alg_cca_opt_change_channel_num_process);
#endif
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : CCA门限优化模块退出函数
******************************************************************************/
osal_void alg_cca_opt_exit(osal_void)
{
    alg_cca_opt_stru *cca_opt = OSAL_NULL;
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_internal_hook_stru* callback = hmac_alg_get_notify_if();
    ret = hmac_alg_unregister(HMAC_ALG_ID_CCA_OPT);
    if (ret != OAL_SUCC) {
        return;
    }

    hal_device = hal_chip_get_hal_device();
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_CCA_OPT, (osal_void **)&cca_opt);
    if (ret == OAL_SUCC) {
        /* 注销并释放hal device级别下的结构体 */
        hmac_alg_unregister_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_CCA_OPT);
    }

    /* unregister rx hook, calc rx mgmt rssi & data rssi */
    hmac_alg_unregister_rx_notify_func(HMAC_ALG_RX_CCA_OPT);
    hmac_alg_unregister_rx_mgmt_notify_func(HMAC_ALG_RX_MGMT_CCA_OPT);
    hmac_alg_unregister_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_CCA);
    hmac_alg_unregister_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_CCA);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_CCA_OPT);
#endif
    callback->cfg_intf_det_cca_notify_func = OSAL_NULL;
    callback->cfg_intf_det_cca_chk_noise_notify_func = OSAL_NULL;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hmac_alg_unregister_cfg_channel_notify_func(HMAC_ALG_CFG_CHANNEL_NOTIFY_CCA_OPT);
#endif
#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
    hmac_alg_unregister_tx_notify_func(HMAC_ALG_TX_CCA_OPT);
#endif
}

/*
 * 函 数 名   : alg_cca_get_cca_opt
 * 功能描述   : 获取cca opt全局变量，UT测试使用
 */
alg_cca_opt_stru *alg_cca_get_cca_opt(osal_void)
{
    return &g_cca_opt;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*
 * 函 数 名   : alg_cca_set_th
 * 功能描述   : 配置CCA门限值
 */
OAL_STATIC osal_void alg_cca_set_th(alg_cca_opt_stru *cca_opt, alg_cca_set_param_type_enum type, osal_s8 value)
{
    if (cca_opt->cca_opt_en == WLAN_CCA_OPT_ENABLE || type == ALG_CCA_SET_DEFAUL) {
        /* init use 2g */
        alg_cca_opt_set_th_default(cca_opt, WLAN_BAND_2G);
    }

    if (cca_opt->cca_opt_en == WLAN_CCA_OPT_ENABLE) {
        oam_warning_log0(0, OAM_SF_CCA_OPT, "{alg_cca_set_th: cca_opt is on!}");
        return;
    }

    switch (type) {
        case ALG_CCA_SET_ED20:
            cca_opt->ed_high_20th_reg = value;
            break;
        case ALG_CCA_SET_ED40:
            cca_opt->ed_high_40th_reg = value;
            break;
        case ALG_CCA_SET_DSSS:
            cca_opt->sd_cca_20th_dsss = value;
            break;
        case ALG_CCA_SET_OFDM:
            cca_opt->sd_cca_20th_ofdm = value;
            break;
        default:
            break;
    }

    hal_set_ed_high_th(cca_opt->ed_high_20th_reg, cca_opt->ed_high_40th_reg, OSAL_TRUE);
    hal_set_cca_prot_th(cca_opt->sd_cca_20th_dsss, cca_opt->sd_cca_20th_ofdm);
}

/******************************************************************************
 功能描述  : 单个参数配置函数
******************************************************************************/
osal_u32 alg_cca_opt_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    alg_cca_opt_stru *cca_opt = &g_cca_opt;
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;
    unref_param(hmac_vap);
    if ((alg_param->alg_cfg <= MAC_ALG_CFG_CCA_OPT_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_CCA_OPT_END)) {
        return OAL_SUCC;
    }
    oam_warning_log2(0, OAM_SF_CCA_OPT, "{alg_cca_opt_config_param::cfg type %d, value %2x}",
        alg_param->alg_cfg, (osal_u8)alg_param->value);
    /* 配置对应参数 */
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_CCA_OPT_ALG_EN_MODE:
            cca_opt->cca_opt_en = (osal_u8)alg_param->value;
            alg_cca_set_th(cca_opt, ALG_CCA_SET_DEFAUL, 0);
            break;

        case MAC_ALG_CFG_CCA_OPT_DEBUG_MODE:
            cca_opt->cca_opt_debug_mode = (osal_u8)alg_param->value;
            break;

        case MAC_ALG_CFG_CCA_OPT_SET_CCA_TH_DEBUG:
            cca_opt->ed_high_20th_reg_adj_debug = (osal_u8)alg_param->value;
            break;

        case MAC_ALG_CFG_CCA_OPT_SET_CCA_ED20:
            alg_cca_set_th(cca_opt, ALG_CCA_SET_ED20, (osal_s8)alg_param->value);
            break;

        case MAC_ALG_CFG_CCA_OPT_SET_CCA_ED40:
            alg_cca_set_th(cca_opt, ALG_CCA_SET_ED40, (osal_s8)alg_param->value);
            break;

        case MAC_ALG_CFG_CCA_OPT_SET_CCA_DSSS:
            alg_cca_set_th(cca_opt, ALG_CCA_SET_DSSS, (osal_s8)alg_param->value);
            break;

        case MAC_ALG_CFG_CCA_OPT_SET_CCA_OFDM:
            alg_cca_set_th(cca_opt, ALG_CCA_SET_OFDM, (osal_s8)alg_param->value);
            break;

        default:
            return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC osal_u32 alg_cca_opt_rx_update_rssi(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hal_rx_statistic_stru *rx_stats, osal_u8 is_mgmt)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_cca_opt_stru *cca_opt = OSAL_NULL;
    oal_bool_enum_uint8 is_cca_registered;
    osal_s8 rx_rssi;
    osal_u32 ret;

    if (osal_unlikely(alg_is_null_ptr2(hmac_vap, hmac_user))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 多播用户无须处理 */
    /* 算法未注册或者组播用户或者用户未关联则无须处理 */
    is_cca_registered = hmac_alg_is_registered(HMAC_ALG_ID_CCA_OPT);
    if (is_cca_registered == OSAL_FALSE || hmac_user->is_multi_user == OSAL_TRUE ||
        hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
        return OAL_SUCC;
    }

    hal_device = hal_chip_get_hal_device();
    /* 仅在单用户时统计RX rssi */
    if (hal_device->assoc_user_nums != 1) {
        return OAL_SUCC;
    }

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_CCA_OPT, (osal_void **)&cca_opt);
    if (ret != OAL_SUCC) {
        return ret;
    }

    rx_rssi = (osal_s8)rx_stats->rssi_dbm;

    if (is_mgmt == OSAL_TRUE) {
        oal_rssi_smooth(&(cca_opt->rx_mgmt_rssi), rx_rssi);
    } else {
        oal_rssi_smooth(&(cca_opt->rx_data_rssi), rx_rssi);
    }
    return OAL_SUCC;
}

osal_u32 alg_cca_opt_rx_data_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf, hal_rx_statistic_stru *rx_stats)
{
    unref_param(buf);
    return alg_cca_opt_rx_update_rssi(hmac_vap, hmac_user, rx_stats, OSAL_FALSE);
}

osal_u32 alg_cca_opt_rx_mgmt_process(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(buf);
    hal_rx_statistic_stru *rx_stats = &(rx_ctl->rx_statistic);
    return alg_cca_opt_rx_update_rssi(hmac_vap, hmac_user, rx_stats, OSAL_TRUE);
}

/*
 * 函 数 名   : alg_cca_opt_calc_prot_th
 * 功能描述   : 计算协议门限
 */
OSAL_STATIC osal_void alg_cca_opt_calc_prot_th(const hal_to_dmac_device_stru *hal_device, alg_cca_opt_stru *cca_opt,
    oal_bool_enum_uint8 switch_enable)
{
    osal_s16 rx_mgmt_rssi = oal_get_real_rssi(cca_opt->rx_mgmt_rssi);
    osal_s16 rx_data_rssi = oal_get_real_rssi(cca_opt->rx_data_rssi);

    /* 弱信号时,将协议门限调低,避免PHY识别不了Wi-Fi信号导致的空口碰撞 */
    if ((hal_device->assoc_user_nums != 1) ||
        (rx_mgmt_rssi >= (HAL_CCA_OPT_ED_HIGH_20TH_DEF - ALG_CCA_OPT_ED_OFST))) {
        return;
    }

    /* 更新OFDM信号和DSSS信号的协议CCA门限 */
    if (hal_device->wifi_channel_status.band == WLAN_BAND_2G) {
        cca_opt->sd_cca_20th_dsss =
            (osal_s8)osal_min(rx_mgmt_rssi - ALG_CCA_OPT_ED_OFST, cca_opt->sd_cca_20th_dsss);
        cca_opt->sd_cca_20th_dsss = (osal_s8)osal_max(cca_opt->sd_cca_20th_dsss, HAL_CCA_OPT_ED_LOW_TH_DSSS_MIN);
    }

    /* 更新OFDM信号的协议CCA门限 */
    cca_opt->sd_cca_20th_ofdm =
        (osal_s8)osal_min(rx_data_rssi - ALG_CCA_OPT_ED_OFST, cca_opt->sd_cca_20th_ofdm);
    cca_opt->sd_cca_20th_ofdm = (osal_s8)osal_max(cca_opt->sd_cca_20th_ofdm, HAL_CCA_OPT_ED_LOW_TH_OFDM_MIN);
    if ((cca_opt->cca_opt_debug_mode == OSAL_TRUE) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log2(0, OAM_SF_CCA_OPT, "[GLA][ALG][CCA_PROTOCOL]:dsss=%d,ofdm=%d",
            cca_opt->sd_cca_20th_dsss, cca_opt->sd_cca_20th_ofdm);
    }
}

#ifdef _PRE_WLAN_FEATURE_INTF_DET
OAL_STATIC osal_void alg_cca_opt_set_th_reg_distance_far(alg_intf_det_stru *intf_det, alg_cca_opt_stru *cca_opt)
{
    osal_s8 ed_high_20th_reg_val;
    osal_s8 ed_high_40th_reg_val;
    osal_s16 rx_mgmt_rssi = oal_get_real_rssi(cca_opt->rx_mgmt_rssi);

    if ((intf_det->adjch_intf_stat.intf_det_avg_rssi_20 == 0) ||
        (intf_det->adjch_intf_stat.intf_det_avg_rssi_20 < ALG_CCA_OPT_ED_HIGH_20TH_LOW_TH)) {
        ed_high_20th_reg_val = ALG_CCA_OPT_ED_HIGH_20TH_LOW_TH;
    } else {
        ed_high_20th_reg_val = intf_det->adjch_intf_stat.intf_det_avg_rssi_20;
    }
    if ((intf_det->adjch_intf_stat.intf_det_avg_rssi_40 == 0) ||
        (intf_det->adjch_intf_stat.intf_det_avg_rssi_40 < ALG_CCA_OPT_ED_HIGH_40TH_LOW_TH)) {
        ed_high_40th_reg_val = ALG_CCA_OPT_ED_HIGH_40TH_LOW_TH;
    } else {
        ed_high_40th_reg_val = intf_det->adjch_intf_stat.intf_det_avg_rssi_40;
    }
    ed_high_20th_reg_val = (osal_s8)osal_max(rx_mgmt_rssi - ALG_CCA_OPT_ED_OFST, ed_high_20th_reg_val);
    ed_high_20th_reg_val = (osal_s8)osal_min(ed_high_20th_reg_val, HAL_CCA_OPT_ED_HIGH_20TH_DEF);
    ed_high_40th_reg_val = (osal_s8)osal_max(rx_mgmt_rssi - ALG_CCA_OPT_ED_OFST, ed_high_40th_reg_val);
    ed_high_40th_reg_val = (osal_s8)osal_min(ed_high_40th_reg_val, HAL_CCA_OPT_ED_HIGH_40TH_DEF);

    cca_opt->ed_high_20th_reg = ed_high_20th_reg_val;
    cca_opt->ed_high_40th_reg = ed_high_40th_reg_val;
}

OAL_STATIC osal_void alg_cca_opt_set_th_reg_intf_strong(alg_intf_det_stru *intf_det, alg_cca_opt_stru *cca_opt,
    wlan_bw_cap_enum_uint8 device_mode)
{
    osal_s8 ed_high_20th_reg_val;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_s8 ed_high_40th_reg_val = cca_opt->ed_high_40th_reg_default;
#else
    osal_s8 ed_high_40th_reg_val = HAL_CCA_OPT_ED_HIGH_40TH_DEF;
#endif

    unref_param(intf_det);
    unref_param(device_mode);

    /* 强干扰和中等强度干扰门限设置 */
    ed_high_20th_reg_val = (osal_s8)(HAL_CCA_OPT_ED_HIGH_20TH_DEF + ALG_CCA_OPT_STRONG_INTF_ADJ_STEP +
        cca_opt->ed_high_20th_reg_adj_debug);

    /* 高底噪场景时，按底噪值+5dBm作为CCA门限值 */
    if (cca_opt->high_intf_flag == OSAL_TRUE) {
        ed_high_20th_reg_val = intf_det->adjch_intf_stat.intf_det_avg_rssi_20 + ALG_CCA_OPT_ED_HIGH_INTF_ADJUST;
        oam_info_log1(0, OAM_SF_CCA_OPT, "{alg_cca_opt_set_th_reg_intf_strong::in high intf mode, ed_high_20_val[%d]}",
            ed_high_20th_reg_val);
    }

    cca_opt->ed_high_20th_reg = ed_high_20th_reg_val;
    cca_opt->ed_high_40th_reg = ed_high_40th_reg_val;
}

OAL_STATIC osal_void alg_cca_opt_set_th_reg_intf_medium(alg_intf_det_stru *intf_det, alg_cca_opt_stru *cca_opt,
    wlan_bw_cap_enum_uint8 device_mode)
{
    osal_s8 ed_high_20th_reg_val;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_s8 ed_high_40th_reg_val = cca_opt->ed_high_40th_reg_default;
#else
    osal_s8 ed_high_40th_reg_val = HAL_CCA_OPT_ED_HIGH_40TH_DEF;
#endif

    unref_param(device_mode);

    ed_high_20th_reg_val = (osal_s8)(intf_det->adjch_intf_stat.intf_det_avg_rssi_20 +
        ALG_CCA_OPT_MEDIUM_INTF_ADJ_STEP_20 + cca_opt->ed_high_20th_reg_adj_debug);
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    ed_high_20th_reg_val += HAL_CCA_OPT_ED_HIGH_20TH_DEF - cca_opt->ed_high_20th_reg_default;
    ed_high_40th_reg_val += HAL_CCA_OPT_ED_HIGH_40TH_DEF - cca_opt->ed_high_40th_reg_default;
#endif
    /* 调节门限范围:[default,default+5] */
    ed_high_20th_reg_val = (osal_s8)osal_min(ed_high_20th_reg_val,
        HAL_CCA_OPT_ED_HIGH_20TH_DEF + ALG_CCA_OPT_STRONG_INTF_ADJ_STEP + cca_opt->ed_high_20th_reg_adj_debug);
    ed_high_20th_reg_val = (osal_s8)osal_max(ed_high_20th_reg_val, HAL_CCA_OPT_ED_HIGH_20TH_DEF);
    ed_high_40th_reg_val = (osal_s8)osal_min(ed_high_40th_reg_val,
        HAL_CCA_OPT_ED_HIGH_40TH_DEF + ALG_CCA_OPT_STRONG_INTF_ADJ_STEP + cca_opt->ed_high_20th_reg_adj_debug);
    ed_high_40th_reg_val = (osal_s8)osal_max(ed_high_40th_reg_val, HAL_CCA_OPT_ED_HIGH_40TH_DEF);

    cca_opt->ed_high_20th_reg = ed_high_20th_reg_val;
    cca_opt->ed_high_40th_reg = ed_high_40th_reg_val;
}

/******************************************************************************
 功能描述  : 根据高底噪场景判断调整CCA能量门限
******************************************************************************/
osal_void alg_cca_opt_rx_chk_noise(osal_s8 avg_rssi_20, oal_bool_enum_uint8 coch_intf_state_sta, osal_u8 cur_intf)
{
    alg_cca_opt_stru *cca_opt = alg_cca_get_cca_opt();

    /* 判断条件：1. 底噪大于-55dBm; 2. 同频干扰; 3. 强邻频干扰 */
    if ((avg_rssi_20 >= ALG_CCA_OPT_ED_HIGH_INTF_TH) && ((coch_intf_state_sta == OSAL_TRUE) ||
        (cur_intf != HAL_ALG_INTF_DET_ADJINTF_NO))) {
        if (cca_opt->high_intf_flag == OSAL_FALSE) {
            cca_opt->high_intf_enter_cnt += 1;
        } else {
            cca_opt->high_intf_exit_cnt = 0;
        }
    } else {
        if (cca_opt->high_intf_flag == OSAL_TRUE) {
            cca_opt->high_intf_exit_cnt += 1;
        } else {
            cca_opt->high_intf_enter_cnt = 0;
        }
    }

    /* 连续五次满足条件时，则进入高底噪场景；连续五次不满足条件时，则退出高底噪场景 */
    if (cca_opt->high_intf_enter_cnt >= ALG_CCA_OPT_ED_HIGH_INTF_ENTER_TH) {
        cca_opt->high_intf_flag = OSAL_TRUE;
        cca_opt->high_intf_enter_cnt = 0;
    } else if (cca_opt->high_intf_exit_cnt >= ALG_CCA_OPT_ED_HIGH_INTF_EXIT_TH) {
        cca_opt->high_intf_flag = OSAL_FALSE;
        cca_opt->high_intf_exit_cnt = 0;
    }
}

/******************************************************************************
 功能描述  : 调整CCA门限
******************************************************************************/
OAL_STATIC osal_u32 alg_cca_opt_set_cca_th_reg(const hal_to_dmac_device_stru *dev, alg_cca_opt_stru *cca_opt,
    osal_u8 cur_intf)
{
    const hal_alg_stat_info_stru *hal_alg_stat = &dev->hal_alg_stat;
    alg_intf_det_stru *intf_det = OSAL_NULL;
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_CCA, ALG_GLA_USUAL_SWITCH);

    if (hmac_alg_get_device_priv_stru(dev, HAL_ALG_DEVICE_STRU_ID_INTF_DET, (osal_void **)&intf_det) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* cca门限初始值默认为无干扰情况下的门限 */
    alg_cca_opt_set_th_default(cca_opt, WLAN_BAND_2G);

    /* 根据干扰检测结果调整CCA能量门限 */
    if ((dev->assoc_user_nums == 1) && (cur_intf != HAL_ALG_INTF_DET_ADJINTF_CERTIFY) &&
        (hal_alg_stat->alg_distance_stat == HAL_ALG_USER_DISTANCE_FAR)) {
        alg_cca_opt_set_th_reg_distance_far(intf_det, cca_opt);
    } else if (cur_intf == HAL_ALG_INTF_DET_ADJINTF_STRONG) {
        alg_cca_opt_set_th_reg_intf_strong(intf_det, cca_opt, hmac_get_device_bw_mode(dev));
    } else if (cur_intf == HAL_ALG_INTF_DET_ADJINTF_MEDIUM) {
        alg_cca_opt_set_th_reg_intf_medium(intf_det, cca_opt, hmac_get_device_bw_mode(dev));
    }

    /* 计算CCA协议门限 */
    alg_cca_opt_calc_prot_th(dev, cca_opt, switch_enable);

    /* cca使能关闭，则直接return，不设置寄存器 */
    if (cca_opt->cca_opt_en != WLAN_CCA_OPT_ENABLE) {
        return OAL_SUCC;
    }

    if ((cca_opt->cca_opt_debug_mode == OSAL_TRUE) || (switch_enable == OSAL_TRUE)) {
        oam_warning_log3(0, OAM_SF_CCA_OPT, "[GLA][ALG][CCA_ENERGY]:th20M=%d,rssi20=%d,ratio_20=%d",
            cca_opt->ed_high_20th_reg, intf_det->adjch_intf_stat.intf_det_avg_rssi_20,
            intf_det->adjch_intf_stat.duty_cyc_ratio_20);
    }

    hal_set_ed_high_th(cca_opt->ed_high_20th_reg, cca_opt->ed_high_40th_reg, OSAL_TRUE);
    hal_set_cca_prot_th(cca_opt->sd_cca_20th_dsss, cca_opt->sd_cca_20th_ofdm);

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 干扰模式改变处理函数
******************************************************************************/
osal_u32 alg_cca_opt_intf_notify_process(hal_to_dmac_device_stru *hal_device, osal_u8 old_intf_mode,
    osal_u8 cur_intf_mode)
{
    alg_cca_opt_stru *cca_opt = OSAL_NULL;
    osal_u32 ret;
    unref_param(old_intf_mode);

    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_CCA_OPT, (osal_void **)&cca_opt);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    ret = alg_cca_opt_set_cca_th_reg(hal_device, cca_opt, cur_intf_mode);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CCA_OPT, "{alg_cca_opt_intf_notify_process: set cca reg fail!ret = %d!}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/******************************************************************************
 功能描述  : device切信道时调用本钩子更新对应device下的alg_cca_opt_stru结构体中的
             1)c_ed_high_20th_reg_default
             2)c_ed_high_40th_reg_default
******************************************************************************/
osal_u32 alg_cca_opt_change_channel_num_process(hmac_vap_stru *hmac_vap, mac_alg_channel_bw_chg_type_uint8 type)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    alg_cca_opt_stru *cca_opt = OSAL_NULL;
    osal_u32 ret;
    wlan_channel_band_enum_uint8 freq_band;
    oal_bool_enum_uint8 is_cca_opt;
    unref_param(type);

    hal_device = hal_chip_get_hal_device();
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CCA_OPT, "{alg_cca_opt_change_channel_num_process:: hal device null pointer ! }");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 只支持2.4G和5G */
    freq_band = hal_device->wifi_channel_status.band;
    if ((freq_band != WLAN_BAND_2G) && (freq_band != WLAN_BAND_5G)) {
        oam_error_log1(0, OAM_SF_CCA_OPT, "{alg_cca_opt_change_channel_num_process::vap[%d] band not support!}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 如果算法没被使能，直接返回 */
    is_cca_opt = hmac_alg_is_registered(HMAC_ALG_ID_CCA_OPT);
    if (is_cca_opt == OSAL_FALSE) {
        return OAL_SUCC;
    }

    /* 获取cca算法结构体 */
    ret = hmac_alg_get_device_priv_stru(hal_device, HAL_ALG_DEVICE_STRU_ID_CCA_OPT, (osal_void **)&cca_opt);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* rssi统计清零 */
    cca_opt->rx_data_rssi = OAL_RSSI_INIT_MARKER;
    cca_opt->rx_mgmt_rssi = OAL_RSSI_INIT_MARKER;

    /* 更新cca能量门限默认值 */
    alg_cca_opt_set_th_default(cca_opt, freq_band);
    return OAL_SUCC;
}
#endif

osal_u32 alg_cca_opt_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    alg_cca_opt_stru *cca_opt = OSAL_NULL;

    unref_param(hmac_user);

    cca_opt = alg_cca_get_cca_opt();
    alg_cca_opt_set_th_default(cca_opt, hmac_vap->channel.band);

    /* cca使能关闭，则直接return，不设置寄存器 */
    if (cca_opt->cca_opt_en != WLAN_CCA_OPT_ENABLE) {
        return OAL_SUCC;
    }

    hal_set_ed_high_th(cca_opt->ed_high_20th_reg, cca_opt->ed_high_40th_reg, OSAL_TRUE);
    hal_set_cca_prot_th(cca_opt->sd_cca_20th_dsss, cca_opt->sd_cca_20th_ofdm);
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_CCA_OPT */
