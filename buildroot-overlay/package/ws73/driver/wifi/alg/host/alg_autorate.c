/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: autorate algorithm rom.
 */
#ifdef _PRE_WLAN_FEATURE_AUTORATE

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "alg_autorate.h"
#include "frw_util.h"
#include "hmac_alg_notify.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_AUTORATE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

OAL_STATIC osal_void alg_autorate_para_sync_fill_fix_rate(alg_autorate_dev_stru *autorate_dev)
{
    osal_u8 idx;
    for (idx = 0; idx < HAL_TX_RATE_MAX_NUM; idx++) {
        autorate_dev->slp_frame_rate[idx].bit_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        autorate_dev->slp_frame_rate[idx].bit_freq_bw = WLAN_BAND_ASSEMBLE_20M;
        autorate_dev->slp_frame_rate[idx].bit_preamble = WLAN_PHY_PREAMBLE_SHORT_HTMF;
        autorate_dev->slp_frame_rate[idx].bit_fec_coding = WLAN_BCC_CODE;
        autorate_dev->slp_frame_rate[idx].bit_gi_type = 0;
        autorate_dev->slp_frame_rate[idx].bit_ltf_type = 0;
        autorate_dev->slp_frame_rate[idx].bit_tx_count = 1;
        autorate_dev->slp_frame_rate[idx].bit_ppdu_dcm = 0;
        autorate_dev->slp_frame_rate[idx].bit_nss_rate = WLAN_PHY_RATE_6M;

        autorate_dev->fix_rate.tx_rate[idx].bit_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
        autorate_dev->fix_rate.tx_rate[idx].bit_freq_bw = WLAN_BAND_ASSEMBLE_20M;
        autorate_dev->fix_rate.tx_rate[idx].bit_preamble = WLAN_PHY_PREAMBLE_SHORT_HTMF;
        autorate_dev->fix_rate.tx_rate[idx].bit_fec_coding = WLAN_LDPC_CODE;
        autorate_dev->fix_rate.tx_rate[idx].bit_gi_type = 0;
        autorate_dev->fix_rate.tx_rate[idx].bit_ltf_type = 0;
        autorate_dev->fix_rate.tx_rate[idx].bit_tx_count = ALG_AUTOARTE_EACH_RATE_MIN_TX_COUNT;
        autorate_dev->fix_rate.tx_rate[idx].bit_ppdu_dcm = 0;
        autorate_dev->fix_rate.tx_rate[idx].bit_nss_rate = 0;
    }
}

OSAL_STATIC osal_void alg_autorate_para_sync_fill_part(alg_autorate_dev_stru *autorate_dev)
{
    autorate_dev->bw_probe_pktcnt_min_thrd = ALG_AUTORATE_BW_MIN_PROBE_INTVL_PKTNUM;
    autorate_dev->min_probe_intvl_pktnum = ALG_AUTORATE_MIN_PROBE_INTVL_PKTNUM;
    autorate_dev->max_probe_intvl_pktnum = ALG_AUTORATE_MAX_PROBE_INTVL_PKTNUM;
    autorate_dev->mcs_up_resv_rate_better_probe_thrd = ALG_AUTORATE_MCS_UP_RESERVE_RATE_BETTER_PROBE_THRD;
    autorate_dev->probe_intvl_keep_times = ALG_AUTORATE_PROBE_INTVL_KEEP_TIMES;

    autorate_dev->normal_pkt_init_per_limit = ALG_AUTORATE_NROMAL_PKT_INIT_PER_LIMIT;
    autorate_dev->aggr_pkt_init_per_limit = ALG_AUTORATE_AGGR_PKT_INIT_PER_LIMIT;
    autorate_dev->aggr_min_mpdu_cnt_thrd = ALG_AUTORATE_AGGR_MIN_MPDU_CNT_THRD;
    autorate_dev->small_ampdu_short_per_smooth = ALG_AR_SMALL_AMPDU_SHORT_PER_SMOOTH;
    autorate_dev->small_ampdu_long_per_smooth = ALG_AR_SMALL_AMPDU_LONG_PER_SMOOTH;
    autorate_dev->smooth_per_opt_th = ALG_AR_SMOOTH_PER_OPT_TH;
    autorate_dev->smooth_per_opt_diff_th = ALG_AR_SMOOTH_PER_OPT_DIFF_TH;
    autorate_dev->smooth_per_opt_th_short_change = ALG_AR_SMOOTH_PER_OPT_SHORT_CHANGE;
    autorate_dev->smooth_per_opt_th_long_change = ALG_AR_SMOOTH_PER_OPT_LONG_CHANGE;
    autorate_dev->smooth_per_bad_th = ALG_AR_SMOOTH_PER_BAD_TH;
    autorate_dev->probe_per_better_th = ALG_AR_PROBE_PER_BETTER_TH;
    autorate_dev->probe_smooth_record_per_th = ALG_AUTORATE_PROBE_SMOOTH_RECORD_PER_TH;
    autorate_dev->probe_mcs_large_aggr_smooth = ALG_AUTORATE_PROBE_MCS_LARGE_AGGR_SMOOTH_PARAM;
    autorate_dev->probe_mcs_small_aggr_smooth = ALG_AUTORATE_PROBE_MCS_SMALL_AGGR_SMOOTH_PARAM;
    autorate_dev->probe_bw_per_smooth = ALG_AUTORATE_PROBE_BW_PER_SMOOTH_PARAM;
    autorate_dev->probe_protocol_per_smooth = ALG_AUTORATE_PROBE_PROTOCOL_PER_SMOOTH_PARAM;

    autorate_dev->initial_stage_rssi_tolerant = ALG_AR_INITIAL_STAGE_RSSI_TOLERANT;
    autorate_dev->rssi_sudden_per_th = ALG_AR_RSSI_SUDDEN_PER_TH;
    autorate_dev->rssi_sudden_th = ALG_AR_RSSI_SUDDEN_TH;
    autorate_dev->probe_down_per_th = ALG_AUTORATE_PROBE_DOWN_PER_TH;
    autorate_dev->continue_probe_down_per_th = ALG_AR_PROBE_DOWN_CONTINUE_PER_TH;
    autorate_dev->cross_gi_probe_stop_per_th = ALG_AUTORATE_PROBE_UP_SHORT_GI_STOP_PROBE_PER_THRD;
    autorate_dev->stat_aging_time = ALG_AUTORATE_PROBE_AGING_TIME_MS;
    autorate_dev->max_probe_wait_cnt = ALG_AUTORATE_MAX_PROBE_WAIT_COUNT;
    autorate_dev->probe_min_mpdu_num = ALG_AUTORATE_PROBE_MIN_MPDU_NUM;

    alg_autorate_para_sync_fill_fix_rate(autorate_dev);
}

OSAL_STATIC osal_u32 alg_autorate_para_sync_fill(alg_param_sync_stru *data)
{
    alg_param_sync_autorate_stru *sync_data = &data->autorate_para;
    alg_autorate_dev_stru  *autorate_dev = &sync_data->autorate_dev;

    memset_s(autorate_dev, sizeof(alg_autorate_dev_stru), 0, sizeof(alg_autorate_dev_stru));

    autorate_dev->enable = OSAL_TRUE;
    autorate_dev->fix_rate_mode = OSAL_FALSE;
    autorate_dev->enable_cross_protocol = OSAL_TRUE;
    autorate_dev->enable_auto_bw = OSAL_TRUE;
    autorate_dev->enable_ar_log = OSAL_FALSE;
    autorate_dev->enable_event_trig_descend_bw = OSAL_TRUE;
    autorate_dev->initial_stage_pkt_cnt_thrd = ALG_AR_INITIAL_STAGE_PKT_CNT_THRD;

    autorate_dev->consecutive_all_fail_th = ALG_AUTORATE_CONSECUTIVE_ALL_FAIL_DEFAULT_THRD;
    autorate_dev->collision_tolerant_ppdu_cnt_thrd = ALG_AUTORATE_COLLISION_TOLERANT_PPDU_CNT_THRD;
    autorate_dev->collision_instant_per_th = ALG_AUTORATE_COLLISION_INSTANT_PER_THRD;

    autorate_dev->rate_aging_time_ms = ALG_AUTORATE_RATE_AGING_TIME_MS;
    autorate_dev->short_stat_shift = ALG_AUTORATE_SHORT_STAT_SHIFT;
    autorate_dev->long_stat_shift = ALG_AUTORATE_LONG_STAT_SHIFT;
    autorate_dev->non_aggr_smooth_scale = ALG_AUTORATE_NON_AGGR_SMOOTH_SCALE;

    autorate_dev->en_probe_per_init_strategy = ALG_AUTORATE_PROBE_PER_INIT_STRATEGY_WITH_EXISTING_RATE_SET_INFO;
    autorate_dev->sudden_good_delta_gdpt_ratio = ALG_AUTORATE_DELTA_GOODPUT_RATIO;
    autorate_dev->sudden_bad_delta_gdpt_ratio = ALG_AUTORATE_DELTA_BAD_GOODPUT_RATIO;

    autorate_dev->mcs_up_reserve_rate_better_per_thrd = ALG_AUTORATE_MCS_UP_RESERVE_RATE_BETTER_PER_THRD;
    autorate_dev->probe_best_update_same_nss_gi_th = ALG_AR_PROBE_SAME_NSS_GI_GAIN_TH;
    autorate_dev->probe_best_update_diff_nss_th = ALG_AR_PROBE_DIFF_NSS_GAIN_TH;
    autorate_dev->probe_best_update_other_nss_gain_th = ALG_AR_PROBE_OTHER_NSS_GAIN_BEYOND_BEST_NSS_TH;
    autorate_dev->probe_best_update_gi_th = ALG_AR_GI_GOODPUT_GAIN_TH;

    autorate_dev->descend_protocol_per_th = ALG_AUTORATE_DESCEND_PROTOCOL_PER_TH;
    autorate_dev->descend_protocol_11b_rssi_th = (osal_s8)ALG_AUTORATE_11B_DESCEND_PROTOCOL_RSSI_TH;
    autorate_dev->ar_11b_ascend_protocol_per_th = ALG_AUTORATE_11B_ASCEND_PROTOCOL_PER_TH;
    autorate_dev->ar_11a_ascend_protocol_per_th = ALG_AUTORATE_11A_ASCEND_PROTOCOL_PER_TH;
    autorate_dev->adjch_intf_type = HAL_ALG_INTF_DET_ADJINTF_NO;
    autorate_dev->sec20_is_busy = OAL_FALSE;
    autorate_dev->intf_adj_min_th = ALG_AUTORATE_INTF_ADJ_MIN_TH;
    autorate_dev->intf_adj_max_th = ALG_AUTORATE_INTF_ADJ_MAX_TH;

    autorate_dev->descend_bw_gdpt_better_thrd = ALG_AUTORATE_DESCEND_BW_GDPT_BETTER_THRD;
    autorate_dev->ascend_bw_gdpt_better_thrd = ALG_AUTORATE_ASCEND_BW_GDPT_BETTER_THRD;
    autorate_dev->descend_bw_event_trig_per_thrd = ALG_AUTORATE_DESCEND_BW_EVENT_TRIG_PER_THRD;
    autorate_dev->descend_bw_event_trig_cnt_thrd = ALG_AUTORATE_DESCEND_BW_MAX_EVENT_TRIG_PKTCNT_THRD;
    autorate_dev->ascend_bw_rate_keep_cnt_thrd = ALG_AUTORATE_ASCEND_BW_RATE_KEEPCNT_THRD;
    autorate_dev->descend_bw_rate_keep_cnt_thrd = ALG_AUTORATE_DESCEND_BW_RATE_KEEPCNT_THRD;

    alg_autorate_para_sync_fill_part(autorate_dev);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_u32 alg_autorate_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_AUTORATE_DMAC_START) ||
        ((alg_param->alg_cfg >= MAC_ALG_CFG_AUTORATE_END) && (alg_param->alg_cfg <= MAC_ALG_CFG_GLA_END))) {
        return OAL_SUCC;
    }
    oam_warning_log2(0, OAM_SF_ANY, "{alg_autorate_hmac_config_param: alg_hmac_autorate_config ID[%d] value[%d]!}",
        alg_param->alg_cfg, alg_param->value);
    if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_TRUE) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_autorate_hmac_config_param::frw_send_msg_to_device failed!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

alg_rx_rate_stats_stru g_rx_11ax_rate_stats[WLAN_HE_MCS_BUTT];
alg_rx_rate_stats_stru g_rx_11n_rate_stats[WLAN_HT_MCS_BUTT];
alg_rx_rate_stats_stru g_rx_legacy_rate_stats[ALG_LEGACY_RATE_STATS_CNT];

/* 入参有效性由notify接口保证 */
osal_u32 alg_autorate_rx_data_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf, hal_rx_statistic_stru *rx_stats)
{
    osal_u8 mcs;
    osal_u8 protocol_mode;
    dmac_rx_ctl_stru *cb_ctrl = OSAL_NULL;

    cb_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(buf);
    if (cb_ctrl->rx_status.he_flag != 0) {
        protocol_mode = WLAN_HE_SU_FORMAT;
    } else {
        protocol_mode = rx_stats->nss_rate.legacy_rate.protocol_mode;
    }

    if (protocol_mode >= WLAN_PHY_PROTOCOL_BUTT) {
        return OAL_FAIL;
    }
    if (protocol_mode > WLAN_VHT_PHY_PROTOCOL_MODE) {
        mcs = rx_stats->nss_rate.he_nss_mcs.he_mcs;
        if (mcs >= WLAN_HE_MCS_BUTT) {
            return OAL_FAIL;
        }
        g_rx_11ax_rate_stats[mcs].cnt++;
        return OAL_SUCC;
    }
    if (protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE) {
        mcs = rx_stats->nss_rate.ht_rate.ht_mcs;
        if (mcs == WLAN_HT_MCS32) {
            mcs = 0;
        }
        if (mcs >= WLAN_HT_MCS_BUTT) {
            return OAL_FAIL;
        }
        g_rx_11n_rate_stats[mcs].cnt++;
        return OAL_SUCC;
    }
    if (protocol_mode <= WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE) {
        g_rx_legacy_rate_stats[0].cnt++;
        return OAL_SUCC;
    }
    unref_param(hmac_vap);
    unref_param(hmac_user);
    return OAL_SUCC;
}

/* 入参由函数调用者保证有效性 */
osal_void alg_autorate_get_rx_rate_stats(alg_rx_rate_all_stats_stru *stats)
{
    if (memcpy_s(stats->he_rx_rate_stats, sizeof(stats->he_rx_rate_stats), g_rx_11ax_rate_stats,
        sizeof(g_rx_11ax_rate_stats)) != EOK) {
        return;
    }

    if (memcpy_s(stats->ht_rx_rate_stats, sizeof(stats->ht_rx_rate_stats), g_rx_11n_rate_stats,
        sizeof(g_rx_11n_rate_stats)) != EOK) {
        return;
    }

    if (memcpy_s(stats->legacy_rx_rate_stats, sizeof(stats->legacy_rx_rate_stats), g_rx_legacy_rate_stats,
        sizeof(g_rx_legacy_rate_stats)) != EOK) {
        return;
    }
}

osal_void alg_autorate_get_rx_rate_stats_clear(osal_void)
{
    (osal_void)memset_s(g_rx_11ax_rate_stats, sizeof(g_rx_11ax_rate_stats), 0, sizeof(g_rx_11ax_rate_stats));
    (osal_void)memset_s(g_rx_11n_rate_stats, sizeof(g_rx_11n_rate_stats), 0, sizeof(g_rx_11n_rate_stats));
    (osal_void)memset_s(g_rx_legacy_rate_stats, sizeof(g_rx_legacy_rate_stats), 0, sizeof(g_rx_legacy_rate_stats));
}
#endif

osal_void alg_hmac_autorate_init(osal_void)
{
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_AUTORATE, alg_autorate_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_AUTORATE, alg_autorate_hmac_config_param);
#endif
}
osal_void alg_hmac_autorate_exit(osal_void)
{
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_AUTORATE);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_AUTORATE);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_AUTORATE */
