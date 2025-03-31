/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: algorithm traffic control
 */

#ifdef _PRE_WLAN_FEATURE_TRAFFIC_CTL

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_traffic_ctl.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#include "wlan_spec.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_TRAFFIC_CTL_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/******************************************************************************
  2 函数定义
******************************************************************************/
OSAL_STATIC osal_u32 alg_traffic_ctl_para_sync_fill(alg_param_sync_stru *data);
OSAL_STATIC osal_void alg_traffic_ctl_display_rsp_para(osal_u16 alg_cfg, osal_u32 value, frw_msg *msg);
OSAL_STATIC osal_u32 alg_traffic_ctl_cfg_rsp_para(osal_u16 alg_cfg, osal_u32 value, frw_msg *msg);
OSAL_STATIC osal_u32 alg_traffic_ctl_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
OSAL_STATIC osal_u32 alg_traffic_ctl_add_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_u32 alg_traffic_ctl_del_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
osal_u8 g_alg_traffic_ctl_rx_dscr_thres_dec_flag;  /* 是否需要减少rx描述符个数 */
osal_u8 g_alg_traffic_exist_legacy; /* 是否存在legacy STAUT */
osal_u32 g_alg_traffic_bypass_tid_quenue_stat = 0;
osal_u32 g_alg_traffic_enqeue_tid_quenue_stat = 0;

/******************************************************
 功能描述  :  拥塞控制参数同步，入参有效性由notify函数保证
*******************************************************/
osal_u32 alg_traffic_ctl_para_sync_fill(alg_param_sync_stru *data)
{
    alg_param_sync_traffic_ctl_stru *traffic_ctl_sync_data = &data->traffic_ctl_para;
    memset_s(traffic_ctl_sync_data, sizeof(alg_param_sync_traffic_ctl_stru),
        0, sizeof(alg_param_sync_traffic_ctl_stru));

    /* 初始化默认配置 */
    traffic_ctl_sync_data->traffic_ctl_enable    = ALG_TRAFFIC_CTL_ENABLE_DEFAULT;     /* 拥塞控制算法默认开 */
    traffic_ctl_sync_data->log_debug_enable      = ALG_TRAFFIC_CTL_LOG_DEBUG_DEFAULT;  /* log功能默认关闭，可通过命令打开 */
    traffic_ctl_sync_data->tx_traffic_ctl_enable = ALG_TRAFFIC_CTL_TX_ENABLE_DEFAULT;  /* 发送方向拥塞控制算法默认开 */
    traffic_ctl_sync_data->rx_traffic_ctl_enable = ALG_TRAFFIC_CTL_RX_ENABLE_DEFAULT;  /* 接收方向拥塞控制算法默认开 */
    traffic_ctl_sync_data->traffic_ctl_cycle_ms  = ALG_TRAFFIC_CTL_TIMEOUT;
    traffic_ctl_sync_data->window_size = ALG_TRAFFIC_CTL_NETBUF_WINDOW_SIZE;
    traffic_ctl_sync_data->netbuf_reserve_size = ALG_TRAFFIC_CTL_NETBUF_RESERVED_SIZE;

    /* 控制参数 */
    traffic_ctl_sync_data->busy_ctl = ALG_TRAFFIC_CTL_BUSY_CTRL_FLAG;
    traffic_ctl_sync_data->shift_size = ALG_TRAFFIC_CTL_MATH_SHIFT_SIZE;
    traffic_ctl_sync_data->tx_max_flowctl_cycle = ALG_TRAFFIC_CTL_MAX_FLOWCTL_CNT;

    /* 初始值，避免TX更新阈值失败 */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].config_dscr_restore_thres =
        (osal_u16)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RX_RESTORE_THRES);
#endif
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].dscr_default_thres  = ALG_TRAFFIC_CTL_RX_DSCR_DEFAULT_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].dscr_default_thres =
        ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_DEFAULT_THRES;

    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].pkt_low_thres   = ALG_TRAFFIC_CTL_RX_PKT_LOW_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].pkt_high_thres  = ALG_TRAFFIC_CTL_RX_PKT_HIGH_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].pkt_busy_thres  = ALG_TRAFFIC_CTL_RX_PKT_BUSY_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].dscr_min_thres  = ALG_TRAFFIC_CTL_RX_DSCR_MIN_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].dscr_low_thres  = ALG_TRAFFIC_CTL_RX_DSCR_LOW_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].dscr_high_thres = ALG_TRAFFIC_CTL_RX_DSCR_HIGH_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_NORMAL_QUEUE].dscr_busy_thres = ALG_TRAFFIC_CTL_RX_DSCR_BUSY_THRES;

    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].pkt_low_thres    = ALG_TRAFFIC_CTL_RX_SMALL_Q_PKT_LOW_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].pkt_high_thres   = ALG_TRAFFIC_CTL_RX_SMALL_Q_PKT_HIGH_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].pkt_busy_thres   = ALG_TRAFFIC_CTL_RX_SMALL_Q_PKT_BUSY_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].dscr_min_thres   = ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_MIN_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].dscr_low_thres   = ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_LOW_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].dscr_high_thres  =
        ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_HIGH_THRES;
    traffic_ctl_sync_data->rx_ctl[HAL_RX_DSCR_SMALL_QUEUE].dscr_busy_thres  =
        ALG_TRAFFIC_CTL_RX_SMALL_Q_DSCR_BUSY_THRES;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_void alg_traffic_ctl_dump_vap_stats(const alg_traffic_status_stru *stats)
{
#ifdef _PRE_WIFI_PRINTK
    osal_u8 q_num;
    osal_u8 vap_idx;
    osal_char *tx_dscr_queue[HAL_TX_QUEUE_NUM] = {
        "BE", "BK", "VI", "VO", "HI", "MC"
    };

    wifi_printf("soft_queue:\r\n");
    for (vap_idx = 0; vap_idx < (sizeof(stats->vap_stats) / sizeof(stats->vap_stats[0])); vap_idx++) {
        wifi_printf("vap_id:%d,state:%d,hal_vap_id:%d,hal_vap_state:%d\r\n",
            stats->vap_stats[vap_idx].vap_id,  stats->vap_stats[vap_idx].vap_state,
            stats->vap_stats[vap_idx].hal_vap_id, stats->vap_stats[vap_idx].hal_vap_state);
        for (q_num = 0; q_num < HAL_TX_QUEUE_NUM; q_num++) {
            wifi_printf("\t[%s]mpdu:%d,ppdu:%d,fake_mpdu:%d,fake_ppdu:%d\r\n", tx_dscr_queue[q_num],
                stats->vap_stats[vap_idx].vap_mpdu_count[q_num],
                stats->vap_stats[vap_idx].vap_ppdu_count[q_num],
                stats->vap_stats[vap_idx].vap_fake_mpdu_count[q_num],
                stats->vap_stats[vap_idx].vap_fake_ppdu_count[q_num]);
        }
    }
    wifi_printf("hw_queue:\r\n");
    for (q_num = 0; q_num < HAL_TX_QUEUE_NUM; q_num++) {
        wifi_printf("\t[%s]mpdu:%d,ppdu:%d\r\n", tx_dscr_queue[q_num], stats->hw_ppdu_count[q_num],
            stats->hw_mpdu_count[q_num]);
    }
#endif
}

OSAL_STATIC osal_void alg_traffic_ctl_info_dump(osal_u8 *msg_rsp, osal_u16 rsp_len)
{
    alg_traffic_tx_ctl_stru *tx_ctl = (alg_traffic_tx_ctl_stru *)(msg_rsp + sizeof(alg_param_sync_traffic_ctl_stru));
    alg_traffic_stats_stru *stats = (alg_traffic_stats_stru *)((osal_u8 *)tx_ctl + sizeof(alg_traffic_tx_ctl_stru));
    const alg_traffic_status_stru *sta = (alg_traffic_status_stru *)((osal_u8 *)stats + sizeof(alg_traffic_stats_stru));
#ifdef _PRE_WIFI_PRINTK
    alg_param_sync_traffic_ctl_stru *param_sync = (alg_param_sync_traffic_ctl_stru *)msg_rsp;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

#ifdef _PRE_LWIP_ZERO_COPY
    wifi_printf("hmac tx netbuf[%u]  rx netbuf[%u]\r\n",
        oal_malloc_netbuf_used(ZERO_COPY_LWIP_DRIECT_TX), oal_malloc_netbuf_used(ZERO_COPY_LWIP_DRIECT_RX));
#endif
    wifi_printf("hmac tid mpdu[%u] bypass[%u], enq[%u] \r\n", hmac_device->total_mpdu_num,
        g_alg_traffic_bypass_tid_quenue_stat, g_alg_traffic_enqeue_tid_quenue_stat);
    g_alg_traffic_bypass_tid_quenue_stat = 0;
    g_alg_traffic_enqeue_tid_quenue_stat = 0;

    wifi_printf("hmac psm mpdu[%d]\r\n", hmac_device->psm_mpdu_num);
    /* cfg */
    wifi_printf("traffic ctl en:%d,rx:%d,tx:%d\r\n",
        param_sync->traffic_ctl_enable, param_sync->rx_traffic_ctl_enable,
        param_sync->tx_traffic_ctl_enable);

    wifi_printf("tx flowctl flag:%d,continue_on:%d,off_cnt:%d,on_cnt:%d,max_on:%d\r\n",
        tx_ctl->flowctl_flag, tx_ctl->tx_continue_on_cycle, tx_ctl->tx_off_cnt, tx_ctl->tx_on_cnt,
        stats->tx_max_flowcnt_cycle);
    wifi_printf("tx flowctl stop_thres:%d,start_thres:%d,window_size:%d,netbuf_resv:%d\r\n",
        tx_ctl->tx_stop_thres, tx_ctl->tx_start_thres, param_sync->window_size,
        param_sync->netbuf_reserve_size);

    wifi_printf("rx normal restore_thres:%d,element_cnt:%d,smooth_dscr:%d,smooth_pkt:%d\r\n",
        sta->rx_normal_thres, sta->rx_normal_element_cnt, sta->rx_normal_smooth_dscr,
        sta->rx_normal_smooth_pkt);
    wifi_printf("rx small restore_thres:%d,element_cnt:%d,smooth_dscr:%d,smooth_pkt:%d\r\n",
        sta->rx_small_thres, sta->rx_small_element_cnt, sta->rx_small_smooth_dscr, sta->rx_small_smooth_pkt);
    wifi_printf("q_empty normal:%d,small:%d,high:%d\r\n", sta->rx_normal_q_empty, sta->rx_small_q_empty,
        sta->rx_high_q_empty);
    /* status */
    wifi_printf("netbuf unused,large:%d,short:%d,mgmt:%d used,large:%d\r\n", sta->normal_free, sta->small_free,
        sta->high_free, sta->normal_used);
    wifi_printf("netbuf alloc fail:%d,succ:%d,release:%d\r\n", stats->netbuf_alloc_fail, stats->netbuf_alloc_succ,
        stats->netbuf_release);
#endif
    /* vap_stats */
    alg_traffic_ctl_dump_vap_stats(sta);
    unref_param(rsp_len);
}
#endif

osal_void alg_traffic_ctl_display_rsp_para(osal_u16 alg_cfg, osal_u32 value, frw_msg *msg)
{
    unref_param(alg_cfg);
    unref_param(value);
    unref_param(msg);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    if (alg_cfg == MAC_ALG_CFG_TRAFFIC_CTL_INFO_DUMP) {
        alg_traffic_ctl_info_dump(msg->rsp, msg->rsp_len);
        oal_mem_free(msg->rsp, OAL_TRUE);
    }
#endif
}

osal_u32 alg_traffic_ctl_cfg_rsp_para(osal_u16 alg_cfg, osal_u32 value, frw_msg *msg)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    osal_u16 len;
    if (alg_cfg == MAC_ALG_CFG_TRAFFIC_CTL_INFO_DUMP) {
        len = (osal_u16)sizeof(alg_param_sync_traffic_ctl_stru) + (osal_u16)sizeof(alg_traffic_tx_ctl_stru) +
            (osal_u16)sizeof(alg_traffic_stats_stru) + (osal_u16)sizeof(alg_traffic_status_stru);
        msg->rsp = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, len, OAL_TRUE);
        if (msg->rsp == NULL) {
            return OAL_FAIL;
        }
        memset_s(msg->rsp, len, 0, len);
        msg->rsp_buf_len = (osal_u16)len;
    }
#endif
    unref_param(alg_cfg);
    unref_param(value);
    unref_param(msg);
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 alg_traffic_ctl_get_mpdu_cnt(osal_void)
{
    return hal_gp_get_mpdu_count();
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void alg_traffic_ctl_bypass_tid_queue(osal_void)
{
    g_alg_traffic_bypass_tid_quenue_stat++;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void alg_traffic_ctl_enqueue_tid_queue(osal_void)
{
    g_alg_traffic_enqeue_tid_quenue_stat++;
}

/******************************************************************************
 功能描述  : 拥塞控制算法参数配置命令
******************************************************************************/
osal_u32 alg_traffic_ctl_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_TRAFFIC_CTL_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_TRAFFIC_CTL_END)) {
        return OAL_SUCC;
    }
#ifndef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    if (alg_param->alg_cfg == MAC_ALG_CFG_TRAFFIC_CTL_INFO_DUMP) {
        return OAL_SUCC;
    }
#endif
    if (alg_traffic_ctl_cfg_rsp_para(alg_param->alg_cfg, alg_param->value, msg) != OAL_SUCC) {
        return OAL_FAIL;
    }
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_TRUE);
    oam_warning_log1(0, OAM_SF_ANY, "{alg_traffic_ctl_hmac_config_param::frw_send_msg_to_device ret %d!}", ret);
    alg_traffic_ctl_display_rsp_para(alg_param->alg_cfg, alg_param->value, msg);
    return (osal_u32)ret;
}

/******************************************************************************
 功能描述  : 拥塞控制算法修改rx描述符最大补充数
******************************************************************************/
OSAL_STATIC osal_u32 alg_traffic_ctl_dscr_busy_thres_modify(hmac_vap_stru *hmac_vap, osal_u8 dec_flag)
{
    osal_u32 ret = OAL_SUCC;
    frw_msg msg_tc = {0};
    mac_ioctl_alg_param_stru *alg_param_tc = OSAL_NULL;

    /* 判断当前是否需要改变rx_dscr_busy_thres，不需要则返回 */
    if (g_alg_traffic_ctl_rx_dscr_thres_dec_flag == dec_flag) {
        return OAL_SUCC;
    }

    alg_param_tc = (mac_ioctl_alg_param_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
        sizeof(mac_ioctl_alg_param_stru), OAL_TRUE);
    if (alg_param_tc == NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{alg_traffic_ctl:alloc fail len [%u]!}", sizeof(mac_ioctl_alg_param_stru));
        return OAL_ERR_CODE_PTR_NULL;
    }
    (void)memset_s(alg_param_tc, sizeof(mac_ioctl_alg_param_stru), 0, sizeof(mac_ioctl_alg_param_stru));
    alg_param_tc->alg_cfg = MAC_ALG_CFG_TRAFFIC_CTL_DSCR_BUSY_THRESHOLD;
    /* 若标志位为true，则减少rx描述符；若为false，则恢复默认值 */
    alg_param_tc->value = (dec_flag == OAL_TRUE) ?
        ALG_TRAFFIC_CTL_RX_DSCR_BUSY_THRES_LEGACY : ALG_TRAFFIC_CTL_RX_DSCR_BUSY_THRES;
    cfg_msg_init((osal_u8 *)alg_param_tc, (osal_u8)sizeof(mac_ioctl_alg_param_stru), OSAL_NULL, 0, &msg_tc);

    ret = alg_traffic_ctl_hmac_config_param(hmac_vap, &msg_tc);
    oal_mem_free((osal_u8*)alg_param_tc, OAL_TRUE);

    /* 下发成功，更改当前标志位 */
    if (ret == OAL_SUCC) {
        g_alg_traffic_ctl_rx_dscr_thres_dec_flag = dec_flag;
    }

    return ret;
}

/******************************************************************************
 功能描述  : 拥塞控制算法添加用户处理
******************************************************************************/
osal_u32 alg_traffic_ctl_add_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret = OAL_SUCC;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    osal_u8 dec_flag = OAL_FALSE;

    /* 入参检查 */
    if (osal_unlikely(alg_is_null_ptr4(hal_device, hmac_vap, hmac_user, hmac_vap->hal_vap))) {
        oam_warning_log0(0, OAM_SF_TRAFFIC, "alg_traffic_ctl_add_user_process::null pointers!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 组播用户不处理 */
    if (hmac_user->is_multi_user == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 非legacy用户、多用户、非STAUT均采用默认值；仅单用户 STAUT legacy场景减少每次补充描述符个数，提高效率 */
    if ((hmac_user->protocol_mode < WLAN_HT_MODE) && (hmac_user->protocol_mode != WLAN_LEGACY_11B_MODE) &&
        (hmac_vap->hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        g_alg_traffic_exist_legacy = OAL_TRUE; /* 存在STAUT legacy用户 记录 */
        if (hal_device->assoc_user_nums == 1) {
            dec_flag = OAL_TRUE;
        }
    }

    /* 下发至device */
    ret = alg_traffic_ctl_dscr_busy_thres_modify(hmac_vap, dec_flag);

    return ret;
}

/******************************************************************************
 功能描述  : 拥塞控制算法删除用户处理
******************************************************************************/
osal_u32 alg_traffic_ctl_del_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret = OAL_SUCC;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    osal_u8 dec_flag = OAL_FALSE;

    /* 入参检查 */
    if (osal_unlikely(alg_is_null_ptr4(hal_device, hmac_vap, hmac_user, hmac_vap->hal_vap))) {
        oam_warning_log0(0, OAM_SF_TRAFFIC, "alg_traffic_ctl_del_user_process::null pointers!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 组播用户不处理 */
    if (hmac_user->is_multi_user == OSAL_TRUE) {
        return OAL_SUCC;
    }

    /* 删掉STAUT legacy用户 记录 */
    if ((hmac_user->protocol_mode < WLAN_HT_MODE) && (hmac_user->protocol_mode != WLAN_LEGACY_11B_MODE) &&
        (hmac_vap->hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        g_alg_traffic_exist_legacy = OAL_FALSE;
    }

    /* 如果删除当前用户后，场景为单用户、STAUT-legacy，减少每次补充描述符个数，提高效率 */
    if (((hal_device->assoc_user_nums - 1) == 1) && (g_alg_traffic_exist_legacy == OAL_TRUE)) {
        dec_flag = OAL_TRUE;
    }

    /* 下发至device */
    ret = alg_traffic_ctl_dscr_busy_thres_modify(hmac_vap, dec_flag);

    return ret;
}

/*******************************************************************************
 功能描述  : 拥塞控制疫模块初始化
******************************************************************************/
osal_s32 alg_hmac_traffic_ctl_init(osal_void)
{
    osal_u32 ret;

    ret = hmac_alg_register(HMAC_ALG_ID_TRAFFIC_CTL);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_TRAFFIC, "{alg traffic control register fail!}");
        return (osal_s32)ret;
    }

    g_alg_traffic_ctl_rx_dscr_thres_dec_flag = OAL_FALSE;
    g_alg_traffic_exist_legacy = OAL_FALSE;
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_TRAFFIC_CTL, alg_traffic_ctl_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_TRAFFIC_CTL, alg_traffic_ctl_hmac_config_param);
#endif
    hmac_alg_register_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_TRAFFIC_CTL, alg_traffic_ctl_add_user_process);
    hmac_alg_register_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_TRAFFIC_CTL, alg_traffic_ctl_del_user_process);
#ifdef _PRE_WLAN_FEATURE_WS73
    hcc_flow_ctrl_credit_register((osal_void *)hal_gp_get_mpdu_count);
#endif
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 拥塞控制模块退出函数
******************************************************************************/
osal_void alg_hmac_traffic_ctl_exit(osal_void)
{
    if (hmac_alg_unregister(HMAC_ALG_ID_TRAFFIC_CTL) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_TRAFFIC, "{alg traffic control unregister fail!}");
        return;
    }
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_TRAFFIC_CTL);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_TRAFFIC_CTL);
#endif
    hmac_alg_unregister_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_TRAFFIC_CTL);
    hmac_alg_unregister_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_TRAFFIC_CTL);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_FEATURE_TRAFFIC_CTL */