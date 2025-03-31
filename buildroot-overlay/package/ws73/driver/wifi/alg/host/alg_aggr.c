/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm autoaggr
 */

#ifdef _PRE_WLAN_FEATURE_AUTOAGGR
/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "hmac_device.h"
#include "hal_common_ops.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_AGGR_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
/******************************************************************************
 功能描述  : AGGR参数同步，入参有效性由notify函数保证
******************************************************************************/
OSAL_STATIC osal_u32 alg_aggr_para_sync_fill(alg_param_sync_stru *data)
{
    alg_param_sync_aggr_stru *aggr_sync_data = &data->aggr_para;

    aggr_sync_data->aggr_probe_intvl_num          = ALG_AGGR_PROBE_INTVL_NUM;
    aggr_sync_data->max_probe_wait_cnt            = ALG_AGGR_MAX_PROBE_WAIT_COUNT;
    aggr_sync_data->init_avg_stat_num             = ALG_AGGR_INIT_AVG_STAT_NUM;
    aggr_sync_data->intf_up_aggr_stat_num         = ALG_AGGR_INTF_UP_STAT_NUM;
    aggr_sync_data->intf_down_aggr_stat_num       = ALG_AGGR_INTF_DOWN_STAT_NUM;
    aggr_sync_data->intf_up_update_stat_cnt_thd   = ALG_AGGR_INTF_UP_UPDATE_STAT_NUM;
    aggr_sync_data->intf_down_update_stat_cnt_thd = ALG_AGGR_INTF_DOWN_UPDATE_STAT_NUM;
    aggr_sync_data->no_intf_aggr_stat_shift       = ALG_AGGR_STAT_SHIFT;
    aggr_sync_data->intf_aggr_stat_shift          = ALG_AGGR_INTF_STAT_SHIFT;
    aggr_sync_data->aggr_non_probe_pck_num        = ALG_AGGR_NON_PROBE_PKT_NUM;
    aggr_sync_data->aging_time_ms                 = ALG_AGGR_DEFAULT_AGING_TIME_MS;
    aggr_sync_data->dbac_aggr_time_idx            = ALG_AGGR_DBAC_DEFAULT_TIME_IDX;
    aggr_sync_data->aggr_tb_switch                = OSAL_FALSE;
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/******************************************************************************
 功能描述  : AGGR聚合自适应模块参数配置处理函数
******************************************************************************/
OSAL_STATIC osal_u32 alg_aggr_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_AGGR_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_AGGR_END)) {
        return OAL_SUCC;
    }
    /* 对aggr_max_aggr_num 入参进行判断,在1-16之间 */
    if (alg_param->alg_cfg == MAC_ALG_CFG_AGGR_MAX_AGGR_NUM) {
        if ((alg_param->value > WLAN_AMPDU_TX_MAX_NUM) || (alg_param->value <= 0)) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_aggr_hmac_config_param::aggr_max_aggr_num out of range!}");
            return OAL_FAIL;
        }
    }

    if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_FALSE) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_aggr_hmac_config_param::frw_send_msg_to_device failed!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif

/******************************************************************************
 功能描述  : ALG AGGR模块初始化总入口
******************************************************************************/
osal_void alg_aggr_hmac_init(osal_void)
{
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_AGGR, alg_aggr_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_AGGR, alg_aggr_hmac_config_param);
#endif
}

/******************************************************************************
 功能描述  : ALG AGGR模块卸载
******************************************************************************/
osal_void alg_aggr_hmac_exit(osal_void)
{
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_AGGR);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_AGGR);
#endif
}

#endif /* #ifdef _PRE_WLAN_FEATURE_AUTOAGGR */
