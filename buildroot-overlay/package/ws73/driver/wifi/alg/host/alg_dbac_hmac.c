/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm dbac
 */

#ifdef _PRE_WLAN_FEATURE_DBAC
/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "hmac_device.h"
#include "hal_common_ops.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_DBAC_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
osal_u32 alg_dbac_hmac_cfg_rsp_para(osal_u16 cfg_type, frw_msg *msg);
osal_void alg_dbac_hmac_display_rsp_para(osal_u16 cfg_type, frw_msg *msg);
osal_u32 alg_dbac_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

/******************************************************************************
 功能描述  : ALG HMAC模块判断是否VAP STOP,此处入参为 hal vap id
******************************************************************************/
osal_bool alg_dbac_hmac_is_vap_stop(osal_u8 hal_vap_id)
{
    osal_u32 bitmap;
    if (hal_vap_id >= HAL_MAX_VAP_NUM) {
        oam_error_log1(0, OAM_SF_DBAC, "{vap id[%d] alg_dbac_hmac_is_vap_stop fail}", hal_vap_id);
        return OAL_TRUE;
    }
    bitmap = hal_gp_get_dbac_vap_stop_bitmap();
    if (bitmap & osal_bit(hal_vap_id)) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

/******************************************************************************
 功能描述  : DBAC参数同步，入参有效性由notify函数保证
******************************************************************************/
osal_u32 alg_dbac_para_sync_fill(alg_param_sync_stru *data)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_s32 priv_value = 0;
#endif
    alg_param_sync_dbac_stru *dbac_para = &data->dbac_para;
    memset_s(dbac_para, sizeof(alg_param_sync_dbac_stru), 0, sizeof(alg_param_sync_dbac_stru));

    /* init alg parameters to default value */
    dbac_para->protect_cnt = HAL_FCS_PROTECT_CNT_20;
    dbac_para->hw_protect_timeout = CFG_DBAC_PROTECT_TIME_OUT_MIN;
    dbac_para->sw_protect_timeout = DBAC_ONE_PACKET_SW_WAIT_TIME;
    dbac_para->vip_frame_protect_threshold = DBAC_VIP_PROTECT_THRESHOLD;
    dbac_para->dbac_tbtt_offset = CFG_DBAC_TBTT_OFFSET;
    dbac_para->timer_adjust = CFG_DBAC_TIMER_OFFSET;
    dbac_para->noa_start_adjust = CFG_DBAC_START_NOA_OFFSET;
    dbac_para->desired_tbtt_adjust = CFG_DBAC_DESIRED_TBTT_ADJUST;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (hwifi_get_init_priv_value(WLAN_CFG_PRIV_DBAC_STA_GC_SLOT_RATIO, &priv_value) == OAL_SUCC) {
        dbac_para->customize_sta_gc_slot_ratio = (osal_u8)priv_value;
    }
    if (hwifi_get_init_priv_value(WLAN_CFG_PRIV_DBAC_STA_GO_SLOT_RATIO, &priv_value) == OAL_SUCC) {
        dbac_para->customize_sta_go_slot_ratio = (osal_u8)priv_value;
    }
#endif
    dbac_para->sync_tsf_threshold = CFG_DBAC_TSF_THRESHOLD;
    dbac_para->resv_netbuf_threshod = CFG_DBAC_RESV_NETBUF_THRESHOLD;
    dbac_para->protect_gc_slot_period = CFG_DBAC_PROTECT_GC_SLOT_PERIOD;
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/******************************************************************************
 功能描述  : ALG DBAC HMAC模块参数配置处理函数
******************************************************************************/
osal_u32 alg_dbac_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_DBAC_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_DBAC_END)) {
        return OAL_SUCC;
    }
    if (alg_dbac_hmac_cfg_rsp_para(alg_param->alg_cfg, msg) != OAL_SUCC) {
        return OAL_FAIL;
    }
    if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_TRUE) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_dbac_hmac_config_param::frw_send_msg_to_device failed!}");
        if (msg->rsp != NULL) {
            oal_mem_free(msg->rsp, OAL_TRUE);
        }
        return OAL_FAIL;
    }
    alg_dbac_hmac_display_rsp_para(alg_param->alg_cfg, msg);
    return OAL_SUCC;
}
#endif

/******************************************************************************
 功能描述  : ALG DBAC HMAC模块VAP DOWN处理
******************************************************************************/
osal_u32 alg_dbac_hmac_vap_down_process(hmac_vap_stru *hmac_vap)
{
    frw_msg msg = {0};
    osal_u32 ret;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (osal_unlikely(hmac_vap == OSAL_NULL || hmac_device == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!mac_is_dbac_running(hmac_device)) {
        return OAL_SUCC;
    }

    /* 抛事件至Device侧DMAC，同步删除VAP */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_DBAC_DOWN, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_alg_vap_down_notify::frw_send_msg_to_device failed[%d].}", ret);
    }
    return ret;
}

/******************************************************************************
 功能描述  : ALG DBAC HMAC模块初始化总入口
******************************************************************************/
osal_void alg_dbac_hmac_init(osal_void)
{
    if (hmac_alg_register(HMAC_ALG_ID_DBAC) != OAL_SUCC) {
        return;
    }
    /* 清空DBAC VAP停止寄存器 */
    hal_gp_clear_dbac_vap_stop_bitmap();
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_DBAC, alg_dbac_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_DBAC, alg_dbac_hmac_config_param);
#endif
    hmac_alg_register_vap_down_notify_func(HMAC_ALG_VAP_DOWN_DBAC, alg_dbac_hmac_vap_down_process);
}

/******************************************************************************
 功能描述  : ALG DBAC HMAC模块卸载
******************************************************************************/
osal_void alg_dbac_hmac_exit(osal_void)
{
    if (hmac_alg_unregister(HMAC_ALG_ID_DBAC) != OAL_SUCC) {
        return;
    }
    /* 清空DBAC VAP停止寄存器 */
    hal_gp_clear_dbac_vap_stop_bitmap();
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_DBAC);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_DBAC);
#endif
    hmac_alg_unregister_vap_down_notify_func(HMAC_ALG_VAP_DOWN_DBAC);
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
osal_u32 alg_dbac_hmac_cfg_rsp_para(osal_u16 cfg_type, frw_msg *msg)
{
    osal_u32 len;
    if (cfg_type == MAC_ALG_CFG_DBAC_STATS_INFO) {
        len = (osal_u32)sizeof(alg_dbac_stats_info);
        msg->rsp = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, len, OAL_TRUE);
        if (msg->rsp == NULL) {
            return OAL_FAIL;
        }
        memset_s(msg->rsp, len, 0, len);
        msg->rsp_buf_len = len;
    }
    return OAL_SUCC;
}
osal_void alg_dbac_display_stats_info(osal_u8 *buf)
{
#ifdef _PRE_WIFI_PRINTK
    osal_u8 i, hal_vap_id;
    alg_dbac_stats_info *query_stats = (alg_dbac_stats_info *)buf;
    alg_dbac_vap_stats_info *vap_stats = NULL;
    alg_dbac_dev_stats_info *dev_stats = &(query_stats->dev_stats);
    wifi_printf("dbac type[%u] state[%u] pause[%u] dual_sta_mode[%u]\n",
        query_stats->dbac_type, query_stats->dbac_state, query_stats->dbac_pause, query_stats->dual_sta_mode);
    wifi_printf("timer_cnt[%u], noa_start_cnt[%u] noa_end_cnt[%u] sync_sta_tsf_to_ap_cnt[%u]\n",
        dev_stats->timer_isr_cnt, dev_stats->noa_start_isr_cnt,
        dev_stats->noa_end_isr_cnt, dev_stats->sync_sta_tsf_to_ap_cnt);
    wifi_printf("netbuf_alloc_fail[%u] led_tbtt_channel_recovery[%u] noa_slot_num_err_cnt[%u]\n",
        dev_stats->tx_netbuf_alloc_fail_cnt, dev_stats->led_tbtt_channel_err_cnt, dev_stats->noa_slot_num_err_cnt);
    for (i = 0; i < CFG_DBAC_VAP_IDX_BUTT; i++) {
        vap_stats = &query_stats->vap_stats[i];
        hal_vap_id = query_stats->vap_idx[i];
        wifi_printf("vap[%u], vap_tbtt_isr_cnt[%u], vap_tbtt_channel_mismatch[%u]\n",
            hal_vap_id, vap_stats->vap_tbtt_isr_cnt, vap_stats->vap_tbtt_channel_mismatch);
        wifi_printf("vap[%u], noa_start_bias[%u], noa_end_bias[%u]\n",
            hal_vap_id, vap_stats->vap_tx_noa_start_bias_cnt, vap_stats->vap_tx_noa_end_bias_cnt);
        wifi_printf("vap[%u], cb_mismatch[%u], pause[%u] out_err[%u] netbuf[%u]\n",
            hal_vap_id, vap_stats->vap_tx_cb_check_mismatch_cnt, vap_stats->vap_tx_pause_cnt,
            vap_stats->vap_tx_vap_id_mismatch_cnt, query_stats->resv_netbuf_cnt[i]);
        wifi_printf("vap[%u], channel_switch[%u], channel_preempt[%u] one_packet_fail[%u]\n",
            hal_vap_id, vap_stats->vap_channel_switch_cnt,
            vap_stats->vap_preempt_cnt, vap_stats->vap_one_packet_send_fail_cnt);
    }
#endif
}
osal_void alg_dbac_hmac_display_rsp_para(osal_u16 cfg_type, frw_msg *msg)
{
    if (cfg_type == MAC_ALG_CFG_DBAC_STATS_INFO) {
        alg_dbac_display_stats_info(msg->rsp);
        oal_mem_free(msg->rsp, OAL_TRUE);
    }
}
#endif

/*****************************************************************************
 函 数 名: hmac_alg_update_dbac_fcs_config 功能描述  : 更新dbac fcs config
*****************************************************************************/
osal_s32 hmac_alg_update_dbac_fcs_config(hmac_vap_stru *hmac_vap)
{
    frw_msg msg = {0};
    osal_s32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件至Device侧DMAC，同步删除VAP */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_DBAC_UPDATE_FCS_PARAM, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hmac_alg_update_dbac_fcs_config::frw_send_msg_to_device failed[%d].}", ret);
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_DBAC */
