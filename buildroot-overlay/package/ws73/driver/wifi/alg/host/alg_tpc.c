/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm tpc
 */

#ifdef _PRE_WLAN_FEATURE_TPC
/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "alg_tpc.h"
#include "alg_main.h"
#include "alg_transplant.h"
#include "hmac_device.h"
#include "hal_common_ops.h"
#include "hmac_alg_notify.h"
#include "hal_ext_if.h"
#include "hal_power.h"
#include "hmac_power.h"
#include "alg_intf_det.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_TPC_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE
#define HMAC_ALG_TPC_FAR_TO_NORMAL_GAP 7                     /* 从远距离切近距离所需RSSI差值 */
#define HMAC_ALG_FAR_DISTANCE_RSSI (-60)
#define HMAC_ALG_TPC_CODE_PROMOTE_DB 3 /* 对满足条件的情况下，提高3db */
#define HMAC_ALG_TPC_CUSTOMIZE_FAR_RSSI_MAX (-50) /* 定制化支持远距离切换最大rssi */
#define HMAC_ALG_TPC_CUSTOMIZE_FAR_RSSI_MIN (-80) /* 定制化支持远距离切换最小rssi */
#define ALG_TPC_INVALID_HOST_FIX_PWR 256 /* 无效的固定功率值 host侧 */

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_u32 alg_tpc_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
OSAL_STATIC osal_void alg_tpc_hmac_display_rsp_para(osal_u16 cfg_type, osal_u8 sub_type, frw_msg *msg);
OSAL_STATIC osal_u32 alg_tpc_hmac_cfg_rsp_para(osal_u16 cfg_type, osal_u8 sub_type, frw_msg *msg);
#endif
osal_u32 alg_tpc_distance_notify_hook(hal_to_dmac_device_stru *hal_device, osal_u8 distance);
/******************************************************************************
 功能描述  : TPC参数同步，入参有效性由notify函数保证
******************************************************************************/
OSAL_STATIC osal_u32 alg_tpc_para_sync_fill(alg_param_sync_stru *data)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    osal_s32 priv_value = 0;
#endif

    alg_param_sync_tpc_stru *tpc_sync_data = &data->tpc_para;
    tpc_sync_data->tpc_normal2near_thres = HMAC_WLAN_NEAR_DISTANCE_RSSI;
    tpc_sync_data->tpc_near2normal_thres = HMAC_WLAN_NORMAL_DISTANCE_RSSI_UP;
    tpc_sync_data->tpc_normal2far_thres = HMAC_ALG_FAR_DISTANCE_RSSI;
    tpc_sync_data->tpc_normal2near_rate_gap = HMAC_ALG_TPC_RATE_DOWN_GAP;
    tpc_sync_data->tpc_normal2near_rate_gap_cnt = HMAC_ALG_TPC_RATE_DOWN_WAIT_COUNT;
    tpc_sync_data->tpc_normal2far_rate_gap = HMAC_ALG_TPC_RATE_UP_GAP;
    tpc_sync_data->near_distance_rssi = HMAC_WLAN_NEAR_DISTANCE_RSSI;
    tpc_sync_data->far_distance_rssi = HMAC_WLAN_FAR_DISTANCE_RSSI;
    tpc_sync_data->fix_power_level = 0;
    tpc_sync_data->tpc_ack_mode = OSAL_TRUE;
    tpc_sync_data->data_rf_limit_enable = OSAL_TRUE;

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (hwifi_get_init_priv_value(WLAN_CFG_PRIV_TPC_FAR_RSSI, &priv_value) == OAL_SUCC) {
        if (priv_value >= HMAC_ALG_TPC_CUSTOMIZE_FAR_RSSI_MIN && priv_value <= HMAC_ALG_TPC_CUSTOMIZE_FAR_RSSI_MAX) {
            tpc_sync_data->tpc_normal2far_thres = (osal_s8)priv_value;
        }
    }
#endif
    tpc_sync_data->tpc_far2normal_thres = tpc_sync_data->tpc_normal2far_thres + HMAC_ALG_TPC_FAR_TO_NORMAL_GAP;
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 用户的距离信息改变，通知tpc算法更新功率
******************************************************************************/
WIFI_TCM_TEXT osal_u32 alg_tpc_distance_notify_hook(hal_to_dmac_device_stru *hal_device, osal_u8 distance)
{
    osal_u8 pow_level_idx[WLAN_DISTANCE_BUTT] = {
        HAL_POW_MIN_POW_LEVEL, HAL_POW_MAX_POW_LEVEL, HAL_POW_RF_LIMIT_POW_LEVEL};

    hal_set_tpc_bf_pow(hal_device);

    /* 距离变化时更新控制帧功率，若用户数大于一，则距离变为normal且不会再改变 */
    hal_device->control_frm_power_level = pow_level_idx[distance];
    hal_pow_set_band_spec_frame_tx_power(hal_device, hal_device->wifi_channel_status.band,
        hal_device->wifi_channel_status.chan_idx);

    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : bfee report帧速率改变，通知tpc算法更新功率
******************************************************************************/
OSAL_STATIC osal_u32 alg_tpc_update_bfee_report_pow(hal_to_dmac_device_stru *hal_device)
{
    hal_set_tpc_bf_pow(hal_device);
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 设置该VAP的beacon发射功率,入参有效性由函数调用者保证
******************************************************************************/
OSAL_STATIC osal_u32 alg_tpc_set_beacon_tx_power(hmac_vap_stru *hmac_vap)
{
    osal_u8 data_rate;
    osal_u32 temp_pow_code;
    hal_to_dmac_vap_stru *hal_vap = hmac_vap->hal_vap;

    if (osal_unlikely(hal_vap == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_TPC, "{alg_tpc_set_beacon_tx_power::[%d] hal vap is null!}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 设置Beacon帧发送模式(含TPC code), 2G 1M, 5G 6M */
    if (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hal_get_bcn_rate(hal_vap, &data_rate);
        temp_pow_code = hal_pow_get_pow_table_tpc_code(hmac_vap->channel.band, data_rate,
            WLAN_BAND_ASSEMBLE_20M, HAL_POW_MAX_POW_LEVEL);
        hal_set_bcn_phy_tx_mode(hal_vap, temp_pow_code);
    }
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 功率表发生变化，更新beacon帧及resp帧功率
******************************************************************************/
OSAL_STATIC osal_u32 alg_tpc_vap_update_frame_tx_power(hmac_vap_stru *hmac_vap)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TPC, "{alg_tpc_vap_update_frame_tx_power:hal_device null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新TXBF */
    hal_set_tpc_bf_pow(hal_device);

    /* 更新RTS功率寄存器 */
    hal_set_tpc_rts_pow();

    /* 更新beacon帧功率 */
    (osal_void)alg_tpc_set_beacon_tx_power(hmac_vap);

    /* 更新resp帧发射功率 */
    hal_pow_set_resp_frame_tx_power(hal_device, hmac_vap->channel.band, hmac_vap->channel.chan_number);

    /* 设置该band的特殊帧的发射功率 */
    hal_pow_set_band_spec_frame_tx_power(hal_device, hmac_vap->channel.band, hmac_vap->channel.chan_idx);
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : AP模式时VAP UP时或者STA模式时接入AP时的处理，都会进入此处理重新初始化beacon帧、控制帧、响应帧功率
******************************************************************************/
OSAL_STATIC osal_u32 alg_tpc_vap_up_process(hmac_vap_stru *hmac_vap)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TPC, "{alg_tpc_vap_up_process:hal_device null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化beacon帧功率 */
    (osal_void)alg_tpc_set_beacon_tx_power(hmac_vap);

    /* 初始化控制帧功率 */
    hal_device->control_frm_power_level = HAL_POW_MAX_POW_LEVEL;
    hal_pow_set_band_spec_frame_tx_power(hal_device, hal_device->wifi_channel_status.band,
        hal_device->wifi_channel_status.chan_idx);

    /* 设置响应帧远近中门限 */
    hal_set_resp_pow_level((HMAC_WLAN_NEAR_DISTANCE_RSSI + WLAN_NEAR_DISTANCE_ADJUST_RSSI),
        HMAC_WLAN_FAR_DISTANCE_RSSI);

    /* 设置resp帧发射功率 */
    hal_pow_set_resp_frame_tx_power(hal_device, hmac_vap->channel.band, hmac_vap->channel.chan_number);
    return OAL_SUCC;
}
/******************************************************************************
 功能描述  : 根据rssi设置距离并下发到device
******************************************************************************/
OSAL_STATIC osal_void alg_tpc_set_rssi_level(hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *cb, hmac_user_stru *hmac_user)
{
    oal_bool_enum_uint8 co_intf = OAL_FALSE;
    hal_alg_intf_det_mode_enum_uint8 adj_intf = HAL_ALG_INTF_DET_ADJINTF_NO;
    osal_s8 rssi = 0;

    if (osal_unlikely(cb == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TPC, "alg_tpc_set_tpc_rssi_level::OAL_ERR_CODE_PTR_NULL.");
        return;
    }

    cb->rssi_level = WLAN_DISTANCE_NORMAL; // 初始化为normal

    /* 如果hmac_user为空，直接返回 */
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        return;
    }
    rssi = oal_get_real_rssi(hmac_user->rx_rssi);

     /* 获取当前的同频/邻叠频干扰状态 */
#ifdef _PRE_WLAN_FEATURE_INTF_DET
    if (osal_unlikely(hmac_vap != OSAL_NULL)) {
        co_intf = alg_intf_det_get_curr_coch_intf_type(hmac_vap->hal_device);
        adj_intf = alg_intf_det_get_curr_adjch_intf_type(hmac_vap->hal_device);
    } else {
        oam_warning_log0(0, OAM_SF_TPC, "alg_tpc_set_tpc_rssi_level::OAL_ERR_CODE_PTR_NULL.");
    }
#endif
/* RSSI DISTANCE仅和距离和干扰有关 */
    if (hmac_user->rx_rssi == OAL_RSSI_INIT_MARKER) {
        cb->rssi_level = WLAN_DISTANCE_NORMAL; /* 如果rssi为初始值，则认为是normal */
    } else if (rssi < HMAC_WLAN_FAR_DISTANCE_RSSI) {
        cb->rssi_level = WLAN_DISTANCE_FAR;
    } else if ((rssi > HMAC_WLAN_NEAR_DISTANCE_RSSI) &&
        (adj_intf == HAL_ALG_INTF_DET_ADJINTF_NO) && (co_intf == OAL_FALSE)) {
        cb->rssi_level = WLAN_DISTANCE_NEAR;
    } else {
        cb->rssi_level = WLAN_DISTANCE_NORMAL; /* 如果不属于上面任意一种距离变化情况，则默认为normal */
    }
}

/******************************************************************************
 功能描述  : ALG TPC模块初始化总入口
******************************************************************************/
osal_void alg_tpc_hmac_init(osal_void)
{
    if (hmac_alg_register(HMAC_ALG_ID_TPC) != OAL_SUCC) {
        return;
    }
    hmac_alg_register_vap_up_notify_func(HMAC_ALG_VAP_UP_TPC, alg_tpc_vap_up_process);
    hmac_alg_register_pow_table_refresh_notify_func(alg_tpc_vap_update_frame_tx_power);
    hmac_alg_register_distance_notify_func(ALG_TPC_BFEE_REPORT_POW_UPDATE_NOTIFY, alg_tpc_distance_notify_hook);
    hmac_alg_register_txbf_pow_update_notify_func(alg_tpc_update_bfee_report_pow);
    hmac_alg_register_para_sync_notify_func(ALG_PARAM_SYNC_TPC, alg_tpc_para_sync_fill);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_register_para_cfg_notify_func(ALG_PARAM_CFG_TPC, alg_tpc_hmac_config_param);
#endif
    hmac_alg_register_update_rssi_level_notify_func(alg_tpc_set_rssi_level);

    /* 初始化rts功率配置寄存器 */
    hal_set_tpc_rts_pow();
}

/******************************************************************************
 功能描述  : ALG TPC模块卸载
******************************************************************************/
osal_void alg_tpc_hmac_exit(osal_void)
{
    if (hmac_alg_unregister(HMAC_ALG_ID_TPC) != OAL_SUCC) {
        return;
    }
    hmac_alg_unregister_vap_up_notify_func(HMAC_ALG_VAP_UP_TPC);
    hmac_alg_unregister_pow_table_refresh_notify_func();
    hmac_alg_unregister_distance_notify_func(ALG_TPC_BFEE_REPORT_POW_UPDATE_NOTIFY);
    hmac_alg_unregister_txbf_pow_update_notify_func();
    hmac_alg_unregister_para_sync_notify_func(ALG_PARAM_SYNC_TPC);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_alg_unregister_para_cfg_notify_func(ALG_PARAM_CFG_TPC);
#endif
    hmac_alg_unregister_update_rssi_level_notify_func();
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_void alg_tpc_query_ack_lut_rate_power_debug(hal_to_dmac_device_stru *hal_device)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    struct osal_list_head *list_pos = OSAL_NULL;
    osal_u8 vap_index;
    osal_u32 reg;
    osal_u8 mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    osal_u8 up_vap_num = hal_device_find_all_up_vap(hal_device, mac_vap_id, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
    for (vap_index = 0; vap_index < up_vap_num; vap_index++) {
        hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id[vap_index]);
        if (hmac_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_TPC, "alg_tpc_set_tpc_mode::hmac_vap[%d] IS NULL.", mac_vap_id[vap_index]);
            continue;
        }

        alg_vap_foreach_user(hmac_user, hmac_vap, list_pos) {
            /* 组播用户不做处理 */
            if (hmac_user->is_multi_user == OSAL_TRUE) {
                continue;
            }
            /* 查询单个用户距离信息 */
            wifi_printf("user id[%u]", hmac_user->assoc_id);
            reg = hal_pow_read_machw_resp_power_lut_entry(hmac_user->lut_index);
            wifi_printf("lut ack reg[0x%08x]\n", reg);
        }
    }
}
OSAL_STATIC osal_void alg_tpc_query_tpc_code_table_debug(hal_to_dmac_device_stru *hal_device)
{
    unref_param(hal_device);
    hal_tpc_rate_pow_print_rate_pow_table();
}
OSAL_STATIC osal_void alg_tpc_query_ack_rate_power_debug(hal_to_dmac_device_stru *hal_device)
{
#ifdef _PRE_WIFI_DEBUG
    hal_tpc_query_ack_power_reg();
#endif
    alg_tpc_query_ack_lut_rate_power_debug(hal_device);
}

OSAL_STATIC osal_void alg_tpc_query_rate_power_debug(hal_to_dmac_device_stru *hal_device)
{
#ifdef _PRE_WIFI_DEBUG
    hal_tpc_query_band_rate_power_debug(hal_device);
#endif
    alg_tpc_query_ack_rate_power_debug(hal_device);
}

OSAL_STATIC osal_void alg_tpc_hmac_query_debug(hal_to_dmac_device_stru *hal_device, osal_u32 query)
{
    if (query == ALG_TPC_QUERY_ALL_POWER) { /* 查询非数据帧速率功率信息 */
        alg_tpc_query_rate_power_debug(hal_device);
    } else if (query == ALG_TPC_QUERY_TPC_CODE_TABLE) { /* 查询逐速率功率表 */
        alg_tpc_query_tpc_code_table_debug(hal_device);
    }
}
OSAL_STATIC osal_void alg_tpc_hmac_display_cfg_para(osal_u8 *buf)
{
#ifdef _PRE_WIFI_PRINTK
    alg_param_sync_tpc_stru *sync_para = (alg_param_sync_tpc_stru *)buf;
    alg_tpc_common_info_stru *tpc_para = (alg_tpc_common_info_stru *)(buf + sizeof(alg_param_sync_tpc_stru));
    wifi_printf("tpc cfg mode:%u\n", tpc_para->tpc_cfg_mode);
    wifi_printf("fix power:%u\n", tpc_para->fix_pwr_code);
    wifi_printf("fix level:%u\n", sync_para->fix_power_level);
    wifi_printf("ack mode:%u\n", sync_para->tpc_ack_mode);
    wifi_printf("normal to near threshold:%d\n", sync_para->tpc_normal2near_thres);
    wifi_printf("near to normal threshold:%d\n", sync_para->tpc_near2normal_thres);
    wifi_printf("normal to far threshold:%d\n", sync_para->tpc_normal2far_thres);
    wifi_printf("far to normal threshold:%d\n", sync_para->tpc_far2normal_thres);
    wifi_printf("normal to near rate gap:%u\n", sync_para->tpc_normal2near_rate_gap);
    wifi_printf("normal to near rate gap count:%u\n", sync_para->tpc_normal2near_rate_gap_cnt);
    wifi_printf("normal to far rate gap:%u\n", sync_para->tpc_normal2far_rate_gap);
    wifi_printf("ack near distacne rssi:%d\n", sync_para->near_distance_rssi);
    wifi_printf("ack far distacne rssi:%d\n", sync_para->far_distance_rssi);
#endif
}
OSAL_STATIC osal_void alg_tpc_hmac_display_status_para(osal_u8 *buf, osal_u16 len)
{
    alg_tpc_common_info_stru *tpc_para = (alg_tpc_common_info_stru *)buf;
    alg_tpc_user_query_status_stru *user_para = NULL;
    osal_u8 num, i;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();
    osal_u8 *buf_tmp = buf;
    osal_u16 len_tmp = len;

    tpc_para->control_frm_power_level = hal_device->control_frm_power_level;

    wifi_printf("tpc mode:%u\n", tpc_para->tpc_mode);
    wifi_printf("over temp:%u\n", tpc_para->is_over_temp);
    wifi_printf("co-intf:%u\n", tpc_para->co_intf_mode);
    wifi_printf("adj-intf level:%u\n", tpc_para->adj_intf_mode);
    wifi_printf("ctrl power level:%u\n", tpc_para->control_frm_power_level);
    buf_tmp += (osal_u16)sizeof(alg_tpc_common_info_stru);
    len_tmp -= (osal_u16)sizeof(alg_tpc_common_info_stru);
    num = (osal_u8)(len_tmp / (osal_u16)sizeof(alg_tpc_user_query_status_stru));
    for (i = 0; i < num; i++) {
        user_para = (alg_tpc_user_query_status_stru *)(buf_tmp + i * sizeof(alg_tpc_user_query_status_stru));
        wifi_printf("user id[%u]\n", user_para->assoc_id);
        wifi_printf("ACK RSSI:%d\n", oal_get_real_rssi(user_para->rssi.tpc_rssi[ALG_TPC_RSSI_ACK]));
        wifi_printf("DATA RSSI:%d\n", oal_get_real_rssi(user_para->rssi.tpc_rssi[ALG_TPC_RSSI_DATA]));
        wifi_printf("BEACON RSSI:%d\n", oal_get_real_rssi(user_para->rssi.tpc_rssi[ALG_TPC_RSSI_MANAG]));

        wifi_printf("current rate index:%u\n", user_para->best_rate_index);
        wifi_printf("per:%u\n", user_para->per);
        wifi_printf("low rate cnt:%u\n", user_para->distance.low_rate_wait_count);
        wifi_printf("rssi distance:%u\n", user_para->distance.rssi_distance);
        wifi_printf("current distance:%u\n", user_para->distance.distance_id);
        wifi_printf("last distance:%u\n", user_para->distance.last_distance_id);
    }
    return;
}

osal_u32 alg_tpc_hmac_cfg_rsp_para(osal_u16 cfg_type, osal_u8 sub_type, frw_msg *msg)
{
    osal_u16 len;
    if ((cfg_type == MAC_ALG_CFG_TPC_QUERY) && (sub_type == ALG_TPC_QUERY_CONFIG_PARA)) {
        len = (osal_u16)sizeof(alg_param_sync_tpc_stru) + (osal_u16)sizeof(alg_tpc_common_info_stru);
        msg->rsp = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, len, OAL_TRUE);
        if (msg->rsp == NULL) {
            return OAL_FAIL;
        }
        memset_s(msg->rsp, len, 0, len);
        msg->rsp_buf_len = len;
    }
    if ((cfg_type == MAC_ALG_CFG_TPC_QUERY) && (sub_type == ALG_TPC_QUERY_STATUS)) {
        len = (osal_u16)(sizeof(alg_tpc_common_info_stru) +
            WLAN_ASSOC_USER_MAX_NUM * sizeof(alg_tpc_user_query_status_stru));
        msg->rsp = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, len, OAL_TRUE);
        if (msg->rsp == NULL) {
            return OAL_FAIL;
        }
        memset_s(msg->rsp, len, 0, len);
        msg->rsp_buf_len = len;
    }
    return OAL_SUCC;
}

osal_void alg_tpc_hmac_display_rsp_para(osal_u16 cfg_type, osal_u8 sub_type, frw_msg *msg)
{
    if ((cfg_type == MAC_ALG_CFG_TPC_QUERY) && (sub_type == ALG_TPC_QUERY_CONFIG_PARA)) {
        alg_tpc_hmac_display_cfg_para(msg->rsp);
        oal_mem_free(msg->rsp, OAL_TRUE);
    }
    if ((cfg_type == MAC_ALG_CFG_TPC_QUERY) && (sub_type == ALG_TPC_QUERY_STATUS)) {
        alg_tpc_hmac_display_status_para(msg->rsp, msg->rsp_len);
        oal_mem_free(msg->rsp, OAL_TRUE);
    }
}
/******************************************************************************
 功能描述  : TPC算法参数配置命令
******************************************************************************/
osal_u32 alg_tpc_hmac_config_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_ioctl_alg_param_stru *alg_param = (mac_ioctl_alg_param_stru *)msg->data;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;

    if ((alg_param->alg_cfg <= MAC_ALG_CFG_TPC_START) || (alg_param->alg_cfg >= MAC_ALG_CFG_TPC_END)) {
        return OAL_SUCC;
    }
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TPC, "{alg_tpc_hmac_config_param: get hal device failed}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (alg_param->alg_cfg == MAC_ALG_CFG_TPC_QUERY) {
        alg_tpc_hmac_query_debug(hal_device, alg_param->value);
    }
    if ((alg_param->alg_cfg == MAC_ALG_CFG_TPC_FIX_PWR_CODE) && (alg_param->value >= ALG_TPC_INVALID_HOST_FIX_PWR)) {
        oam_error_log0(0, OAM_SF_TPC, "{alg_tpc_hmac_config_param: value out of bound}");
        return OAL_FAIL;
    }

    if (alg_tpc_hmac_cfg_rsp_para((osal_u16)alg_param->alg_cfg, (osal_u8)alg_param->value, msg) != OAL_SUCC) {
        return OAL_FAIL;
    }
    if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_PARAM, msg, OSAL_TRUE) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{alg_tpc_hmac_config_param::frw_send_msg_to_device failed!}");
        if (msg->rsp != NULL) {
            oal_mem_free(msg->rsp, OAL_TRUE);
        }
        return OAL_FAIL;
    }
    alg_tpc_hmac_display_rsp_para((osal_u16)alg_param->alg_cfg, (osal_u8)alg_param->value, msg);
    return OAL_SUCC;
}
#endif

osal_void alg_set_api_tpc_api_3db_mode(osal_u8 add_flag, osal_u8 under_ragulation_flag)
{
    if (add_flag == OAL_TRUE) {
        hal_set_rf_limit_power(WLAN_BAND_2G, HMAC_ALG_TPC_CODE_PROMOTE_DB);
    } else {
        hal_set_rf_limit_power(WLAN_BAND_2G, 0);
    }
    hal_power_set_under_regulation(under_ragulation_flag);
}
#endif /* #ifdef _PRE_WLAN_FEATURE_TPC */
