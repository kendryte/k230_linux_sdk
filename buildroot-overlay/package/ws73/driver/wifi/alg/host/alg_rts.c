/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: algorithm anti interference
 */
#ifdef _PRE_WLAN_FEATURE_RTS

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "alg_rts.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"
#include "hmac_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_RTS_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
static alg_rts_rate_tbl_stru g_rts_11b_table[] = {
    /*   1st_rank_non_erp   1st_rank_erp           2nd_rank_non_erp     2nd_rank_erp
         3rd_rank_non_erp   3rd_rank_erp           4th_rank_non_erp     4th_rank_erp */
    /* 数据帧11b1M */
    {{{ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}, {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M},
      {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}, {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11b2M */
    {{{ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}, {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M},
      {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}, {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11b5.5M */
    {{{ALG_RTS_11B_5M5,     ALG_RTS_11B_5M5},     {ALG_RTS_11B_5M5,     ALG_RTS_11B_5M5},
      {ALG_RTS_11B_5M5,     ALG_RTS_11B_5M5},     {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11b11M */
    {{{ALG_RTS_11B_11M,     ALG_RTS_11B_11M},     {ALG_RTS_11B_11M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11B_11M,     ALG_RTS_11B_5M5},     {ALG_RTS_LONG_11B_1M, ALG_RTS_LONG_11B_1M}}},
};

static alg_rts_rate_tbl_stru g_rts_11g_table[] = {
    /*   1st_rank_non_erp   1st_rank_erp           2nd_rank_non_erp     2nd_rank_erp
         3rd_rank_non_erp   3rd_rank_erp           4th_rank_non_erp     4th_rank_erp */
    /* 数据帧11g48M */
    {{{ALG_RTS_11G_24M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11g24M */
    {{{ALG_RTS_11G_24M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11g12M */
    {{{ALG_RTS_11G_12M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11g6M */
    {{{ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},
      {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11g54M */
    {{{ALG_RTS_11G_24M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11g36M */
    {{{ALG_RTS_11G_24M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11g18M */
    {{{ALG_RTS_11G_12M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11g9M */
    {{{ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},
      {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
};

static alg_rts_rate_tbl_stru g_rts_11n_and_11ax_table[] = {
    /*   1st_rank_non_erp   1st_rank_erp           2nd_rank_non_erp     2nd_rank_erp
         3rd_rank_non_erp   3rd_rank_erp           4th_rank_non_erp     4th_rank_erp */
    /* 数据帧11n mcs0 */
    {{{ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},
      {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11n mcs1 */
    {{{ALG_RTS_11G_6M,      ALG_RTS_11B_11M},     {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},
      {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11n mcs2 */
    {{{ALG_RTS_11G_6M,      ALG_RTS_11B_11M},     {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},
      {ALG_RTS_11G_6M,      ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11n mcs3 */
    {{{ALG_RTS_11G_12M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* 数据帧11n mcs4 */
    {{{ALG_RTS_11G_12M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
    /* >=数据帧11n mcs5 */
    {{{ALG_RTS_11G_24M,     ALG_RTS_11B_11M},     {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},
      {ALG_RTS_11G_12M,     ALG_RTS_11B_5M5},     {ALG_RTS_11G_6M,      ALG_RTS_LONG_11B_1M}}},
};

static alg_config_param_sync_rts_stru g_rts_config_param_sync;
/******************************************************************************
  3 枚举定义
******************************************************************************/
/* RTS mode */
typedef enum {
    ALG_RTS_MODE_ALL_ENABLE    = 0, /* rate[0..3]都开RTS */
    ALG_RTS_MODE_ALL_DISABLE   = 1, /* rate[0..3]都不开RTS */
    ALG_RTS_MODE_RATE0_DYNAMIC = 2, /* rate[0]动态RTS, rate[1..3]都开RTS */
    ALG_RTS_MODE_RATE0_DISABLE = 3, /* rate[0]不开RTS, rate[1..3]都开RTS */
    ALG_RTS_MODE_THRESHOLD     = 4, /* 根据MIB配置的RTS Threshold字节数门限确定是否开RTS */

    ALG_RTS_MODE_BUTT
} alg_rts_mode_enum;
typedef osal_u8 alg_rts_mode_enum_uint8;
/******************************************************************************
  3 函数定义
******************************************************************************/
/******************************************************
 功能描述  : RTS参数同步，入参有效性由notify函数保证
*******************************************************/
osal_u32 alg_rts_para_sync_fill(alg_param_sync_stru *data)
{
    alg_param_sync_rts_stru *rts_sync_data = &data->rts_para;
    memset_s(rts_sync_data, sizeof(alg_param_sync_rts_stru), 0, sizeof(alg_param_sync_rts_stru));

    rts_sync_data->tolerant_rts_all_fail_cnt = ALG_RTS_TOLERANT_RTS_ALL_FAIL_CNT;
    rts_sync_data->per_multiple = ALG_RTS_PER_MULTIPLE;
    rts_sync_data->loss_ratio_max_num = ALG_RTS_LOSS_RATIO_MAX_NUM;
    rts_sync_data->loss_ratio_rts_retry_num = ALG_RTS_LOSS_RATIO_RTS_RETRY_NUM;
    rts_sync_data->loss_ratio_stat_retry_num = ALG_RTS_LOSS_RATIO_STAT_RETRY_NUM;

    rts_sync_data->per_stat_far_rssi_thr = ALG_RTS_PER_STAT_FAR_RSSI_THR;
    rts_sync_data->per_stat_normal_rssi_thr = ALG_RTS_PER_STAT_NORMAL_RSSI_THR;

    rts_sync_data->first_pkt_stat_shift = ALG_RTS_FIRST_PKT_STAT_SHIFT;
    rts_sync_data->first_pkt_stat_intvl = ALG_RTS_FIRST_PKT_STAT_INTVL;
    rts_sync_data->legacy_1st_loss_ratio_th = ALG_RTS_LEGACY_1ST_LOSS_RATIO_TH;
    rts_sync_data->ht_vht_1st_loss_ratio_th = ALG_RTS_HT_VHT_1ST_LOSS_RATIO_TH;

    rts_sync_data->open_rts_rssi_high_thr = ALG_RTS_OPEN_RTS_RSSI_HIGH_THR;
    rts_sync_data->open_rts_rssi_low_thr = ALG_RTS_OPEN_RTS_RSSI_LOW_THR;

    rts_sync_data->txop_duration_rts_threshold_max = TXOP_DURATION_RTS_THRESHOLD_MAX;
    rts_sync_data->txop_duration_rts_threshold_on = TXOP_DURATION_RTS_THRESHOLD_ON;
    rts_sync_data->rts_threshold = WLAN_RTS_MAX;
    rts_sync_data->kbps_to_bpus = ALG_RTS_KBPS_TO_BPUS;

    rts_sync_data->rts_duration_multiple = ALG_RTS_DURATION_MULTIPLE;
    rts_sync_data->rts_duration_open = ALG_RTS_DURATION_OPEN;
    rts_sync_data->rts_rssi_open = ALG_RTS_RSSI_OPEN;
    rts_sync_data->rts_rate_descend = ALG_RTS_RATE_DESCEND;

    if (memcpy_s(&rts_sync_data->rts_11b_tbl, sizeof(rts_sync_data->rts_11b_tbl),
        &g_rts_11b_table, sizeof(g_rts_11b_table)) != EOK) {
        return OAL_FAIL;
    }
    if (memcpy_s(&rts_sync_data->rts_11g_tbl, sizeof(rts_sync_data->rts_11g_tbl),
        &g_rts_11g_table, sizeof(g_rts_11g_table)) != EOK) {
        return OAL_FAIL;
    }
    if (memcpy_s(&rts_sync_data->rts_11n_tbl, sizeof(rts_sync_data->rts_11n_tbl),
        &g_rts_11n_and_11ax_table, sizeof(g_rts_11n_and_11ax_table)) != EOK) {
        return OAL_FAIL;
    }
    if (memcpy_s(&rts_sync_data->rts_11ax_tbl, sizeof(rts_sync_data->rts_11ax_tbl),
        &g_rts_11n_and_11ax_table, sizeof(g_rts_11n_and_11ax_table)) != EOK) {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/******************************************************************************
 功能描述  : RTS模块参数配置处理函数
******************************************************************************/
OAL_STATIC osal_u32 alg_rts_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 rts_cfg_mode_tbl[] = {1, 0, 2, 3, 4};
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;
    osal_u32 get_mode_result = 0;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_RTS_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_RTS_END)) {
        return OAL_SUCC;
    }

    if (alg_param->alg_cfg == MAC_ALG_CFG_GET_RTS_MODE) {
        msg->rsp_buf_len = (osal_u16)sizeof(osal_u32);
        msg->rsp = (osal_u8 *)&get_mode_result;
    }

    if ((alg_param->alg_cfg != MAC_ALG_CFG_GET_RTS_MODE) && (alg_param->alg_cfg != MAC_ALG_CFG_RTS_RATE)) {
        if (alg_rts_set_host_param(alg_param) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_rts_hmac_config_param::rts_config_param::failed}");
            return OAL_FAIL;
        }
        alg_param->alg_cfg = MAC_ALG_CFG_RTS_HOST;
    }

    if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_TRUE) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_rts_hmac_config_param::frw_send_msg_to_device::failed}");
        return OAL_FAIL;
    }

    if (alg_param->alg_cfg == MAC_ALG_CFG_GET_RTS_MODE) {
        alg_param->value = get_mode_result;
        oam_warning_log2(0, OAM_SF_ANY, "{alg_rts_hmac_config_param::rts cfg: %d  rts_param: %d \r\n",
            alg_param->alg_cfg, rts_cfg_mode_tbl[alg_param->value]);
        return OAL_SUCC;
    }

    oam_warning_log2(0, OAM_SF_ANY, "{alg_rts_hmac_config_param::rts cfg: %d  rts_param: %d \r\n",
        alg_param->alg_cfg, alg_param->value);
    return OAL_SUCC;
}
#endif

/*******************************************************************************
 功能描述  : rts host侧初始化
******************************************************************************/
osal_s32 alg_host_rts_init(osal_void)
{
    /* 注册初始化变量同步函数 */
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_RTS, alg_rts_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_RTS, alg_rts_hmac_config_param);
#endif

    /* 初始化host配置命令结构体 */
    g_rts_config_param_sync.rts_enable = OSAL_TRUE;
    g_rts_config_param_sync.rts_fixed_mode = OSAL_FALSE;
    g_rts_config_param_sync.enable_rts_log = OSAL_FALSE;
    g_rts_config_param_sync.rts_mode = ALG_RTS_MODE_RATE0_DYNAMIC;

    /* 初始化RTS rate寄存器 */
    hal_set_rts_rate_init();
    /* 关闭cfg_rts_fail_tx_psdu_en */
    hal_set_rts_fail_tx_psdu_en(OAL_FALSE);
    return OAL_SUCC;
}

/*******************************************************************************
 功能描述  : rts host侧退出
******************************************************************************/
osal_s32 alg_host_rts_exit(osal_void)
{
    /* 注册初始化变量同步函数 */
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_RTS);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_RTS);
#endif
    return OAL_SUCC;
}

osal_u32 alg_rts_config_threshold(hmac_vap_stru *hmac_vap, osal_u32 rts_threshold)
{
    osal_u32 ret;
    frw_msg msg_info = {0};

    if (osal_unlikely(alg_is_null_ptr1(hmac_vap))) {
        oam_error_log0(0, OAM_SF_RTS, "{alg_rts_config_param::NULL POINTERS!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1 如果是配置命令，等于0表示所有帧均开启RTS保护 */
    frw_msg_init((osal_u8 *)&rts_threshold, sizeof(osal_u32), OSAL_NULL, 0, &msg_info);
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_RTS_THRESHOLD_SYNC, &msg_info,
        OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RTS, "hmac_rts_threshold_sync::sync msg ret [%d]", ret);
        return ret;
    }
    oam_warning_log2(0, OAM_SF_ANY, "{alg_rts_hmac_config_param::rts cfg: %d  rts_param: %d \r\n",
        MAC_ALG_CFG_RTS_MIB, rts_threshold);
    return ret;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*******************************************************************************
 功能描述  : rts host配置命令解析函数
******************************************************************************/
OSAL_STATIC osal_u32 alg_rts_config_set_status(osal_u32 value)
{
    if (value >= OAL_BUTT) {
        return OAL_FAIL;
    }
    g_rts_config_param_sync.rts_enable = (osal_u8)value == 0 ? OSAL_TRUE : OSAL_FALSE;
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 alg_rts_config_set_debug(osal_u32 value)
{
    if (value >= OAL_BUTT) {
        return OAL_FAIL;
    }
    g_rts_config_param_sync.enable_rts_log = (osal_u8)value == 0 ? OSAL_TRUE : OSAL_FALSE;
    return OAL_SUCC;
}

OSAL_STATIC osal_u32 alg_rts_config_set_mode(osal_u32 value)
{
    if (value >= ALG_RTS_MODE_BUTT) {
        return OAL_FAIL;
    }

    g_rts_config_param_sync.rts_fixed_mode = OSAL_TRUE;

    if (value == ALG_RTS_MODE_RATE0_DYNAMIC) {
        g_rts_config_param_sync.rts_fixed_mode = OSAL_FALSE;
    }
    g_rts_config_param_sync.rts_mode = (alg_rts_mode_enum_uint8)value;
    return OAL_SUCC;
}

/*******************************************************************************
 功能描述  : rts host配置命令
******************************************************************************/
osal_u32 alg_rts_set_host_param(mac_ioctl_alg_param_stru *alg_param)
{
    osal_u32 *value = (osal_u32 *)&g_rts_config_param_sync;

    if (osal_unlikely(alg_is_null_ptr1(alg_param))) {
        oam_error_log0(0, OAM_SF_RTS, "{alg_rts_config_param::NULL POINTERS!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 配置对应参数 */
    switch (alg_param->alg_cfg) {
        case MAC_ALG_CFG_RTS_ENABLE:
            if (alg_rts_config_set_status(alg_param->value) != OAL_SUCC) {
                return OAL_FAIL;
            }
            break;

        case MAC_ALG_CFG_RTS_DEBUG:
            if (alg_rts_config_set_debug(alg_param->value) != OAL_SUCC) {
                return OAL_FAIL;
            }
            break;

        case MAC_ALG_CFG_RTS_MODE:
            if (alg_rts_config_set_mode(alg_param->value) != OAL_SUCC) {
                return OAL_FAIL;
            }
            break;

        default :
            oam_warning_log0(0, OAM_SF_RTS, "{alg_rts_set_host_param::config param error!}");
            return OAL_FAIL;
    }
    alg_param->value = *value;
    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_RTS */
