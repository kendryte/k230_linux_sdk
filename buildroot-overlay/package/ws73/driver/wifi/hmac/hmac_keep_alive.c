/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac_keep_alive
 * Author:
 * Create: 2022-09-22
 */

#include "hmac_keep_alive.h"
#include "mac_resource_ext.h"
#include "hmac_user.h"
#include "hmac_vap.h"
#include "hmac_btcoex.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_KEEP_ALIVE_C

#define MAX_AP_USER_AGING_TIME 600
#define MIN_AP_USER_AGING_TIME 10
#define TIMER_SECONDS_TO_MS 1000

osal_u32 g_wlan_ap_user_aging_time = WLAN_AP_USER_AGING_TIME;

osal_u32 hmac_config_get_ap_user_aging_time(osal_void)
{
    return g_wlan_ap_user_aging_time;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_set_ap_user_aging_time(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 value = *((osal_u32 *)msg->data);
    unref_param(hmac_vap);

    /* 可用配置当前仅限[10, 600]以内 */
    if (value < MIN_AP_USER_AGING_TIME || value > MAX_AP_USER_AGING_TIME) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_common_debug:value [%d] is out of range [10, 600]}", value);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    g_wlan_ap_user_aging_time = value * TIMER_SECONDS_TO_MS;
    return OAL_SUCC;
}
#endif

osal_s32 hmac_config_keepalive_sync_timestamp(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 active_user_bitmap = *(osal_u32 *)(msg->data);
    osal_u32 idx = 0;
    unref_param(hmac_vap);

    for (; idx < hmac_board_get_max_user(); idx++) {
        if ((active_user_bitmap & (1 << idx)) != 0) {
            hmac_user_stru *hmac_user = mac_res_get_hmac_user_etc((osal_u16)idx);
            if (hmac_user == OSAL_NULL) {
                oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_keepalive_sync_timestamp:: user[%u] is null!!!}", idx);
                return OAL_ERR_CODE_PTR_NULL;
            }
            hmac_user->last_active_timestamp = osal_get_time_stamp_ms();
        }
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 功能描述  : 当设置keepalive模式为2时，打印GO/SoftAp 下挂设备keepalive具体参数
*****************************************************************************/
static osal_void hmac_config_print_keepalive_user_info(hmac_vap_stru *hmac_vap)
{
    osal_u64 present_time = osal_get_time_stamp_ms();
    struct osal_list_head *entry = OSAL_NULL;
    struct osal_list_head *dlist_tmp = OSAL_NULL;

    wifi_printf("present_time[%llu]\r\n", present_time);
    osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
        osal_u64 runtime;
        hmac_user_stru *hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
        if (hmac_user == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_KEEPALIVE, "hmac_config_print_keepalive_user_info:vap_id[%d] user is null.",
                hmac_vap->vap_id);
            return;
        }
        runtime = osal_get_runtime((osal_u32)hmac_user->last_active_timestamp, (osal_u32)present_time);
        wifi_printf("user_aid[%u]\r\n", hmac_user->assoc_id);
        wifi_printf("mac[%02X:%02X:%02X:%02X:XX:XX]\r\n", hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1],
            hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]); /* 2表示mac地址第2位，3表示mac地址第3位 */
        wifi_printf("last_active_time:[%llu] runtime[%llu]\r\n", hmac_user->last_active_timestamp, runtime);
    }
}


/*****************************************************************************
 功能描述  : 当设置keepalive模式为2时，打印当前keepalive linkloss参数
*****************************************************************************/
static osal_void hmac_config_keepalive_debug(hmac_vap_stru *hmac_vap, osal_u8 value)
{
    osal_u32 aging_time, send_null_frame_time;
    frw_msg msg_info;
    osal_s32 ret;
    osal_void *fhook = OSAL_NULL;

    wifi_printf("############keepalive/linkloss query############\r\n");
    if (is_ap(hmac_vap)) {
        aging_time = g_wlan_ap_user_aging_time;
        send_null_frame_time = WLAN_AP_KEEPALIVE_INTERVAL;
        if (hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) {
            wifi_printf("mode:GO\r\n");
            aging_time = WLAN_P2PGO_USER_AGING_TIME;
            send_null_frame_time = WLAN_GO_KEEPALIVE_INTERVAL;
            fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_CHECK_SET_AGING_TIME);
            if (fhook != OSAL_NULL && ((hmac_btcoex_hal_set_aging_time_cb)fhook)(hmac_vap, &aging_time) == OAL_FAIL) {
                return;
            }
        } else {
            wifi_printf("mode:SoftAp\r\n");
        }
        wifi_printf("keepalive info:\r\n");
        wifi_printf("keepalive cap[%d]\r\n", hmac_vap->cap_flag.keepalive);
        wifi_printf("aging_time:%u\r\n", aging_time);
        wifi_printf("send_null_frame_time:%u\r\n", send_null_frame_time);
        hmac_config_print_keepalive_user_info(hmac_vap);
    } else if (is_sta(hmac_vap)) {
        if (hmac_vap->p2p_mode == WLAN_P2P_CL_MODE) {
            wifi_printf("mode:GC\r\n");
        } else {
            wifi_printf("mode:Sta\r\n");
        }
        wifi_printf("linkloss info:\r\n");
        wifi_printf("powersave mode:[%d] (1 active 2 powersave)\r\n", mac_mib_get_powermanagementmode(hmac_vap));
        wifi_printf("linkloss cnt:[%u]\r\n", get_current_linkloss_cnt(hmac_vap));
        wifi_printf("linkloss threshold:[%u]\r\n", get_current_linkloss_threshold(hmac_vap));
    }
    /* SoftAp or GO 相关参数不涉及device侧，可以直接返回 */
    if (is_ap(hmac_vap)) {
        return;
    }
    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));
    /* 当前vap为sta时，要将参数配置到device侧dmac */
    frw_msg_init(&value, sizeof(value), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_KEEPALIVE_DEBUG, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_config_keepalive_debug fail [%d]}", hmac_vap->vap_id, ret);
    }
}

/*****************************************************************************
 功能描述  : 设置keepalive模式 0关闭 1打开 2查询当前状态
*****************************************************************************/
osal_s32 hmac_config_set_keepalive_mode(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 value = *((osal_u8 *)msg->data);
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 可用配置仅限0 1 */
    if (value > 1) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_set_keepalive_mode:value [%d] is out of range}", value);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hmac_vap->cap_flag.keepalive = value;
    /* 对于Ap类的关闭再打开场景，需要再次开启定时器 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (hmac_vap->cap_flag.keepalive == OSAL_TRUE) &&
        (hmac_device->keepalive_timer.is_registerd == OSAL_FALSE)) {
        frw_create_timer_entry(&(hmac_device->keepalive_timer), hmac_user_keepalive_timer,
            WLAN_AP_KEEPALIVE_TRIGGER_TIME, hmac_device, OSAL_TRUE);
    }
    return hmac_vap_sync(hmac_vap);
}

/*****************************************************************************
 功能描述  : 设置debug命令，当前支持参数0---keepalive
*****************************************************************************/
osal_s32 hmac_config_common_debug(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 value = *((osal_u8 *)msg->data);

    /* 可用配置当前仅限COMMON_DEBUG_BUTT以内 */
    if (value >= COMMON_DEBUG_BUTT) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_common_debug:value [%d] is out of range}", value);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    switch (value) {
        case COMMON_DEBUG_KEEPALIVE:
            hmac_config_keepalive_debug(hmac_vap, value);
            break;
        default:
            break;
    }
    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

