/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: APF hmac function.
 * Create: 2022-10-14
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_apf.h"
#include "msg_apf_rom.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_APF_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* APF规则信息全局结构体 */
mac_apf_stru g_hmac_st_apf = {0};
osal_u8      g_hmac_st_apf_ip[0x4] = {0};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 功能描述  : 更新APF过滤规则中自己的IP地址
*****************************************************************************/
OSAL_STATIC osal_void hmac_apf_update_st_apf(osal_void)
{
    frw_msg msg2device = {0};
    osal_s32 ret;
    osal_u32 j;
    osal_u32 i;

    if (g_hmac_st_apf.program_len < APF_PROGRAM_IPV4_BROADCAST_END) {
        return;
    }
    /* 从第76 - 79字符是IPV4地址 */
    for (j = APF_PROGRAM_IPV4_BEGIN, i = 0; j <= APF_PROGRAM_IPV4_END; j++, i++) {
        g_hmac_st_apf.program[j] = ((osal_u8 *)(g_hmac_st_apf_ip))[i];
    }

    /* 替换网段广播地址，从第143 - 145字符是IPV4段广播地址 */
    for (j = APF_PROGRAM_IPV4_BROADCAST_BEGIN, i = 0; j <= APF_PROGRAM_IPV4_BROADCAST_END; j++, i++) {
        g_hmac_st_apf.program[j] = ((osal_u8 *)(g_hmac_st_apf_ip))[i];
    }

    if (g_hmac_st_apf.is_enabled == OAL_TRUE) {
        // 下发事件更新apf规则
        frw_msg_init((osal_u8 *)&g_hmac_st_apf, sizeof(mac_apf_stru), OSAL_NULL, 0, &msg2device);
        ret = send_cfg_to_device(0, WLAN_MSG_H2D_C_CFG_APF_EXEC, &msg2device, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_APF, "{hmac_apf_update_st_apf:: send apf on to device fail [%d]!}", ret);
        }
    }
}

OSAL_STATIC osal_bool hmac_apf_set_ip_addr_etc(osal_void *notify_data)
{
    if (notify_data == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_apf_set_ip_addr_etc::The ip_addr is NULL. }");
        return OSAL_FALSE;
    }
    (osal_void)memcpy_s(g_hmac_st_apf_ip, sizeof(g_hmac_st_apf_ip), notify_data, sizeof(g_hmac_st_apf_ip));
    hmac_apf_update_st_apf();
    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : APF功能开关
*****************************************************************************/
OSAL_STATIC osal_bool hmac_apf_filter_switch(osal_void *notify_data)
{
    frw_msg                     msg2device = {0};
    oal_bool_enum_uint8         device_apf_msg;
    osal_s32                    ret;
    oal_bool_enum_uint8 apf_switch = *(oal_bool_enum_uint8 *)notify_data;
    osal_u32 pps_rate = 0;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AUTO_FREQ_PPS);

    if (apf_switch >= OAL_BUTT) {
        return OSAL_FALSE;
    }

    if (fhook != OSAL_NULL) {
        ((hmac_get_pps_handle_pps_rate_cb)fhook)(&pps_rate);
    }

    /* 如果apf已处状态和设置状态一致 则直接返回 */
    if (g_hmac_st_apf.is_enabled == apf_switch) {
        return OSAL_TRUE;
    }

    /* 开关状态置为关 */
    if (apf_switch == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_APF, "hmac_apf_filter_switch::disable");
        device_apf_msg = OAL_FALSE;
        g_hmac_st_apf.is_enabled = OAL_FALSE;

        // 下发事件将device的apf功能给关了
        frw_msg_init((osal_u8 *)&device_apf_msg, sizeof(oal_bool_enum_uint8), OSAL_NULL, 0, &msg2device);
        /* snyc ps config to device */
        ret = send_cfg_to_device(0, WLAN_MSG_H2D_C_CFG_APF_EXEC, &msg2device, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_APF, "{hmac_apf_filter_switch:: send apf off to device fail [%d]!}", ret);
            return OSAL_FALSE;
        }
        return OSAL_TRUE;
    }

    if ((g_hmac_st_apf.program_len != 0) && (pps_rate <= PPS_VALUE_1)) {
        g_hmac_st_apf.is_enabled = OAL_TRUE;

        oam_warning_log3(0, OAM_SF_APF, "hmac_apf_filter_switch:enable[%hhu] program len[%hu], over[%u]",
            g_hmac_st_apf.is_enabled, g_hmac_st_apf.program_len, PPS_VALUE_1);

        // 下发事件开apf功能
        frw_msg_init((osal_u8 *)&g_hmac_st_apf, sizeof(mac_apf_stru), OSAL_NULL, 0, &msg2device);
        ret = send_cfg_to_device(0, WLAN_MSG_H2D_C_CFG_APF_EXEC, &msg2device, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_APF, "{hmac_apf_filter_switch:: send apf on to device fail [%d]!}", ret);
            return OSAL_FALSE;
        }
    } else {
        oam_warning_log0(0, OAM_SF_APF, "hmac_apf_filter_switch::can't enable apf.");
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述  : 强制停止APF
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_force_stop_filter(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_bool_enum_uint8 uc_switch = OAL_FALSE;
    hmac_device_stru *hmac_device;
    osal_u8 force_stop_filter;

    if (hmac_vap == OAL_PTR_NULL || msg == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_APF, "hmac_config_force_stop_filter:: NULL ptr error.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hwifi_get_apf_enable() == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_APF, "{hmac_config_force_stop_filter: apf isn't supported.}");
        return OAL_SUCC;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_APF, "hmac_config_force_stop_filter::hmac_device NULL error");
        return OAL_ERR_CODE_PTR_NULL;
    }

    force_stop_filter = (*((osal_s32 *)msg->data) == OSAL_SWITCH_ON);
    /* 在暗屏情况下, force_stop_filter不使能时，开启apf过滤; 使能时，强制关闭apf */
    if ((hmac_device->in_suspend == OSAL_TRUE) && (force_stop_filter == OAL_FALSE)) {
        // 打开apf
        uc_switch = OAL_TRUE;
    }
    hmac_apf_filter_switch(&uc_switch);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 配置APF过滤条件
*****************************************************************************/
OSAL_STATIC osal_void hmac_apf_filter_install(const mac_apf_filter_cmd_stru *apf_filter_cmd)
{
    osal_u8 *program = apf_filter_cmd->program;
    osal_u16 program_len = apf_filter_cmd->program_len;

    if (program_len > APF_PROGRAM_MAX_LEN) {
        oam_error_log2(0, OAM_SF_APF, "{hmac_apf_filter_install::program_len [%hu] > APF_PROGRAM_MAX_LEN[%d]!.}",
            program_len, APF_PROGRAM_MAX_LEN);
        return;
    }

    if (memcpy_s(g_hmac_st_apf.program, APF_PROGRAM_MAX_LEN, program, program_len) != EOK) {
        oam_error_log0(0, OAM_SF_APF, "hmac_apf_filter_install::memcpy fail.");
        return;
    }
    g_hmac_st_apf.program_len = program_len;
    hmac_apf_update_st_apf();
    g_hmac_st_apf.install_timestamp = (osal_u32)(osal_get_time_stamp_ms() >> BIT_OFFSET_10); /* 秒级时间戳 */

    oam_warning_log1(0, OAM_SF_APF, "{hmac_apf_filter_install::set_apf_filter succ, program_len[%hu].}",
        g_hmac_st_apf.program_len);
    return;
}

/*****************************************************************************
 功能描述  : 删除APF过滤条件
*****************************************************************************/
OSAL_STATIC osal_bool hmac_apf_filter_del_user(osal_void *notify_data)
{
    frw_msg                     msg2device = {0};
    oal_bool_enum_uint8         device_apf_msg;
    osal_s32                    ret;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "{hmac_apf_filter_del_user:vap[%d] is null.}", hmac_user->vap_id);
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    /* 如果apf正打开，则发事件关闭 */
    if (g_hmac_st_apf.is_enabled == OSAL_TRUE) {
        device_apf_msg = OSAL_FALSE;
        frw_msg_init((osal_u8 *)&device_apf_msg, sizeof(oal_bool_enum_uint8), OSAL_NULL, 0, &msg2device);
        /* snyc ps config to device */
        ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_APF_EXEC, &msg2device, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_APF, "{hmac_apf_filter_uninstall:: send apf off to device fail [%d]!}", ret);
        }
    }
    oam_warning_log1(0, OAM_SF_APF, "dmac_apf_filter_uninstall::program_len[%hu].", g_hmac_st_apf.program_len);
    g_hmac_st_apf.is_enabled = OAL_FALSE;
    g_hmac_st_apf.program_len = 0;
    return OSAL_TRUE;
}

/*****************************************************************************
 功能描述:
 1.当apf_filter_cmd->cmd_type == APF_SET_FILTER_CMD时，
   下发APF过滤规则到dmac
 2.当apf_filter_cmd->cmd_type != APF_SET_FILTER_CMD时，
   上报HMAC侧当前APF过滤规则
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_apf_filter_cmd(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_apf_filter_cmd_stru *apf_filter_cmd = OSAL_NULL;
    frw_msg msg2device;
    osal_s32 ret;
    osal_u32 i;

    if (hmac_vap == OSAL_NULL || msg == OSAL_NULL || msg->data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_APF, "{hmac_config_apf_filter_cmd::mac_vap or param or msg->data is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    apf_filter_cmd = (mac_apf_filter_cmd_stru *)msg->data;
    if (apf_filter_cmd->cmd_type == APF_SET_FILTER_CMD) {
        hmac_apf_filter_install(apf_filter_cmd);
        /* 规则发生变动需要看目前是否开启apf,如果开启则需要重新下发规则给device */
        if (g_hmac_st_apf.is_enabled == OAL_TRUE) {
            frw_msg_init((osal_u8 *)&g_hmac_st_apf, sizeof(mac_apf_stru), OSAL_NULL, 0, &msg2device);
            ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_APF_EXEC, &msg2device, OSAL_TRUE);
            if (ret != OAL_SUCC) {
                oam_error_log1(0, OAM_SF_APF, "{hmac_config_apf_filter_cmd:: send apf on to device fail [%d]!}", ret);
                return ret;
            }
        }
    } else if (apf_filter_cmd->cmd_type == APF_GET_FILTER_CMD) {
        for (i = 0; i < g_hmac_st_apf.program_len; ++i) {
            wifi_printf("%02x", g_hmac_st_apf.program[i]);
        }
    }

    return OAL_SUCC;
}

osal_u32 hmac_apf_init(osal_void)
{
    /* 注册监听 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_apf_filter_del_user);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_INETADDR_NOTIFIER_UP, hmac_apf_set_ip_addr_etc);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_SUSPEND_CHANGE, hmac_apf_filter_switch);
    /* 注册消息 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_FILTER_LIST, hmac_config_apf_filter_cmd);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_FORCE_STOP_FILTER, hmac_config_force_stop_filter);
    return OAL_SUCC;
}

osal_void hmac_apf_deinit(osal_void)
{
    /* 去注册监听 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_apf_filter_del_user);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_INETADDR_NOTIFIER_UP, hmac_apf_set_ip_addr_etc);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_SUSPEND_CHANGE, hmac_apf_filter_switch);
    /* 去注册消息 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_FILTER_LIST);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_FORCE_STOP_FILTER);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
