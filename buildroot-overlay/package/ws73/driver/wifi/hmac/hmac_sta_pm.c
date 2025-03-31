/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: STA侧节能状态机所在文件
 * Date: 2020-07-14
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_sta_pm.h"
#include "wlan_spec.h"
#include "plat_pm_wlan.h"
#include "hmac_beacon.h"
#include "hal_device_fsm.h"
#include "hmac_ccpriv.h"
#include "hmac_psm_sta.h"
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
#include "hal_pm.h"
#include "hcc_if.h"
#include "hcc_cfg_comm.h"
#include "hcc_channel.h"
#include "hcc_list.h"
#include "soc_osal.h"
#include "hmac_feature_interface.h"
#include "frw_thread.h"
#include "hmac_uapsd_sta.h"
#include "wlan_msg.h"
#include "frw_util_notifier.h"
#include "hal_tbtt.h"
#if (defined ENABLE_LOW_POWER && (ENABLE_LOW_POWER == YES))
#include "pm_sleep_porting.h"
#endif
#endif
#include "hcc_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_STA_PM_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
hmac_sta_pm_ctrl_info g_hmac_pm_ctrl;
#ifdef _PRE_WLAN_POWERSAVE_DEBUG
osal_u16 g_host_sleep_check_debug[HMAC_NOT_SLEEP_INFO_BUTT] = {0};
#endif
#endif
/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_pm_sta_attach
 功能描述  : sta类型VAP创建pm handler
*****************************************************************************/
osal_void hmac_pm_sta_attach(hmac_vap_stru *hmac_vap)
{
    mac_sta_pm_handler_stru *handler  = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2S, "{hmac_pm_sta_attach::param null.}");
        return;
    }

    handler = &hmac_vap->sta_pm_handler;
    if (handler->is_fsm_attached == OSAL_TRUE) {
        oam_error_log1(0, OAM_SF_PWR, "{hmac_pm_sta_attach::vap id[%d]pm fsm aready attached.}",
                       hmac_vap->vap_id);
        return;
    }

    (osal_void)memset_s(handler, sizeof(mac_sta_pm_handler_stru), 0, sizeof(mac_sta_pm_handler_stru));
    handler->vap_ps_mode = NO_POWERSAVE;
    handler->last_ps_status = MAC_STA_PM_SWITCH_OFF;
    handler->is_fsm_attached = OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_sta_ps_mode
 功能描述  : 设置sta ps mode
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_set_sta_ps_mode(hmac_vap_stru *hmac_vap, osal_u8 cfg_pm_mode)
{
    frw_msg msg2device = {0};
    osal_s32 ret;

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{hmac_config_set_sta_ps_mode::mac_res_get_hmac_vap fail,vap_id:%u.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (cfg_pm_mode < NUM_PS_MODE) {
        hmac_vap->cfg_pm_mode = cfg_pm_mode;
        frw_msg_init((osal_u8 *)&cfg_pm_mode, sizeof(osal_u8), OSAL_NULL, 0, &msg2device);
        ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SET_PS_MODE, &msg2device, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_PWR, "{hmac_config_set_sta_ps_mode::frw_send_msg_to_device failed[%d].}", ret);
            return ret;
        }
    } else {
        oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] dmac set pm mode[%d]> max mode fail",
            hmac_vap->vap_id, cfg_pm_mode);
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_set_sta_pm_on(hmac_vap_stru *hmac_vap, mac_sta_pm_handler_stru *sta_pm_handle,
    osal_u8 cfg_pm_mode)
{
    if (sta_pm_handle->is_fsm_attached == OSAL_FALSE) {
        oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_set_sta_pm_on_cb::sta pm fsm not attached.}", hmac_vap->vap_id);
        return;
    }

    /* 如果当前低功耗模式与需设置的模式一样，协议上不再重复设置低功耗 */
    if (cfg_pm_mode == sta_pm_handle->vap_ps_mode) {
        oam_warning_log2(0, OAM_SF_PWR,
                         "vap_id[%d] {hmac_set_sta_pm_on_cb:the same ps mode:[%d]}",
                         hmac_vap->vap_id, sta_pm_handle->vap_ps_mode);
        return;
    }

    if (cfg_pm_mode == NO_POWERSAVE) {
        sta_pm_handle->vap_ps_mode = cfg_pm_mode;
        mac_mib_set_powermanagementmode(hmac_vap, WLAN_MIB_PWR_MGMT_MODE_ACTIVE);
    }

    /* 确保低功耗模式和定时器同步,开低功耗必须在关联上的时候才设置模式,重启activity定时器 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_UP || hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) {
        sta_pm_handle->vap_ps_mode = cfg_pm_mode;
        if (cfg_pm_mode != NO_POWERSAVE) {
            mac_mib_set_powermanagementmode(hmac_vap, WLAN_MIB_PWR_MGMT_MODE_PWRSAVE);
        }
    }

    oam_warning_log4(0, OAM_SF_PWR, "vap_id[%d] {hmac_set_sta_pm_on_cb:ps mode:[%d],aid[%d] close_status[%d]}",
        hmac_vap->vap_id, cfg_pm_mode, hmac_vap->sta_aid, hmac_vap->sta_pm_close_status);
}

/*****************************************************************************
 功能描述  : 开启sta低功耗
 输入参数  : hmac_vap_stru *mac_vap
           osal_u8 len
           osal_u8 *param
 返 回 值: osal_u32
*****************************************************************************/
osal_void hmac_set_sta_pm_on_cb(hmac_vap_stru *hmac_vap, mac_cfg_ps_open_stru *pm_cfg)
{
    mac_sta_pm_handler_stru *sta_pm_handle = &hmac_vap->sta_pm_handler;
    osal_u8 final_sta_pm, cfg_pm_mode;

    /* 配置255则永久关闭低功耗需重启才能恢复低功耗 */
    if ((pm_cfg->pm_enable == MAC_STA_PM_DISABLE_FOREVER) || (hmac_vap->sta_pm_handler.forbiden_pm == OSAL_TRUE)) {
        hmac_vap->sta_pm_handler.forbiden_pm = OSAL_TRUE;
        return;
    }

    /* 支持ON/OFF/RESET配置,其他值不改变控制位 */
    if (pm_cfg->pm_enable == MAC_STA_PM_SWITCH_ON || pm_cfg->pm_enable == MAC_STA_PM_SWITCH_RESET) {
        hmac_vap->sta_pm_close_status &= (osal_u8)(~(osal_u32)(1U << (osal_u32)pm_cfg->pm_ctrl_type)); /* 清空对应模块的bit位 */
    } else if (pm_cfg->pm_enable == MAC_STA_PM_SWITCH_OFF) {
        hmac_vap->sta_pm_close_status |= (1U << (osal_u32)pm_cfg->pm_ctrl_type); /* 置对应模块的bit位 */
    }

    /* 如果有模块关闭低功耗，则关闭低功耗 */
    final_sta_pm = (hmac_vap->sta_pm_close_status != 0 || pm_cfg->pm_enable == MAC_STA_PM_SWITCH_RESET) ?
        MAC_STA_PM_SWITCH_OFF : MAC_STA_PM_SWITCH_ON;

    if ((final_sta_pm == MAC_STA_PM_SWITCH_ON) && (hmac_vap->p2p_mode != WLAN_P2P_CL_MODE)) {
        /* 开低功耗时的pm mode为 NO_POWERSAVE则使用FAST_PS否则使用配置的节能模式 */
        cfg_pm_mode = (hmac_vap->cfg_pm_mode == NO_POWERSAVE) ? MIN_FAST_PS : hmac_vap->cfg_pm_mode;
    } else {
        /* 关闭低功耗时,低功耗模式切回NO_POWERSAVE */
        cfg_pm_mode = NO_POWERSAVE;
    }

    // 记录上一次host开关节能的状态，其他模块需根据此状态开关节能
    if ((pm_cfg->pm_ctrl_type == MAC_STA_PM_CTRL_TYPE_HOST) && (hmac_vap->p2p_mode != WLAN_P2P_CL_MODE)) {
        hmac_vap->sta_pm_handler.last_ps_status = pm_cfg->pm_enable;
    }

    /* 重置低功耗开关后，关闭标志清0, 防止DBAC,ROAM不对称的开关低功耗,导致无法再打开低功耗 */
    if (pm_cfg->pm_enable == MAC_STA_PM_SWITCH_RESET && pm_cfg->pm_ctrl_type == MAC_STA_PM_CTRL_TYPE_HOST) {
        hmac_vap->sta_pm_close_status = 0;
        hmac_vap->sta_pm_handler.last_ps_status = MAC_STA_PM_SWITCH_OFF;
    }

    hmac_set_sta_pm_on(hmac_vap, sta_pm_handle, cfg_pm_mode);
}

/*****************************************************************************
 功能描述  : 打开staut低功耗
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_set_sta_pm_on(hmac_vap_stru *hmac_vap, mac_cfg_ps_open_stru *pm_cfg)
{
    osal_s32 ret;
    osal_u8 ps_mode;
    frw_msg msg = {0};

    if (osal_unlikely(pm_cfg == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_sta_pm_on, pm_cfg is null}");
        return OAL_FAIL;
    }

    /* p2p模式不支持低功耗 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA || !is_legacy_vap(hmac_vap)) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_config_set_sta_pm_on::vap[%d] isn't sta or legacy vap}",
            hmac_vap->vap_id, hmac_vap->vap_mode);
        return OAL_SUCC;
    }

    oam_warning_log4(0, 0, "vap_id[%d] {hmac_config_set_sta_pm_on::module[%d],switch[%d],close status[%d].}",
        hmac_vap->vap_id, pm_cfg->pm_ctrl_type, pm_cfg->pm_enable, hmac_vap->sta_pm_close_status);
    /* 支持ccpriv配置255永久关闭低功耗此配置不需要改变低功耗模式 */
    if (pm_cfg->pm_enable != MAC_STA_PM_DISABLE_FOREVER) {
        /* 切换到手动设置为pspoll模式 */
        if (pm_cfg->pm_enable == MAC_STA_PM_MANUAL_MODE_ON) {
            hmac_vap->cfg_sta_pm_manual = OSAL_TRUE;
        } else if (pm_cfg->pm_enable == MAC_STA_PM_MANUAL_MODE_OFF) {
            /* 关闭手动设置pspoll模式,回到fastps模式 */
            hmac_vap->cfg_sta_pm_manual = 0xFF;
        }

        if (pm_cfg->pm_enable != MAC_STA_PM_SWITCH_RESET) {
            pm_cfg->pm_enable = (pm_cfg->pm_enable > MAC_STA_PM_SWITCH_OFF) ?
                                    MAC_STA_PM_SWITCH_ON : MAC_STA_PM_SWITCH_OFF;
        }

        ps_mode = (pm_cfg->pm_enable == OSAL_TRUE) ?
                  ((hmac_vap->cfg_sta_pm_manual != 0xFF) ? MIN_PSPOLL_PS : hmac_vap->ps_mode) : NO_POWERSAVE;

        oam_warning_log3(0, OAM_SF_PWR, "vap_id[%d] hmac_config_set_sta_pm_on_etc,enable[%d], ps_mode[%d]",
                         hmac_vap->vap_id, pm_cfg->pm_enable, ps_mode);
        /* 先下发设置低功耗模式 */
        ret = hmac_config_set_sta_ps_mode(hmac_vap, ps_mode);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] sta_pm sta_pm mode[%d]fail", hmac_vap->vap_id, ret);
            return ret;
        }
    }

    hmac_set_sta_pm_on_cb(hmac_vap, pm_cfg);

    /* snyc ps mode to device */
    msg.data = (osal_u8 *)pm_cfg;
    msg.data_len = sizeof(mac_cfg_ps_open_stru);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_SET_STA_PM_ON, &msg, OSAL_TRUE);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{hmac_config_set_sta_pm_on::frw_send_msg_to_device failed[%d].}", ret);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_s32 hmac_config_set_sta_pm_mode_cb(hmac_vap_stru *hmac_vap, osal_u8 vap_ps_mode)
{
    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap->ps_mode = vap_ps_mode;
    oam_warning_log1(0, OAM_SF_PWR, "hmac_config_set_sta_pm_mode_cb ps_mode[%d]", hmac_vap->ps_mode);

    return hmac_config_set_sta_ps_mode(hmac_vap, vap_ps_mode);
}
#endif

OSAL_STATIC osal_s32 hmac_config_set_sta_pm_on_cb(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_ps_open_stru *sta_pm_open = OSAL_NULL;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL))) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_config_set_sta_pm_on_cb::hmac_vap / param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    sta_pm_open = (mac_cfg_ps_open_stru *)msg->data;

    return hmac_config_set_sta_pm_on(hmac_vap, sta_pm_open);
}

osal_void hmac_config_mvap_set_sta_pm_on(hmac_vap_stru *hmac_vap, oal_bool_enum mvap_up)
{
    mac_pm_ctrl_type_enum pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_MVAP;
    mac_pm_switch_enum pm_enable = MAC_STA_PM_SWITCH_OFF;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *another_up_vap = OSAL_NULL;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_config_mvap_set_sta_pm_on::hmac vap null}");
        return;
    }

    if (mvap_up && !(hmac_vap->vap_state == MAC_VAP_STATE_UP || hmac_vap->vap_state == MAC_VAP_STATE_PAUSE
#ifdef _PRE_WLAN_FEATURE_P2P
        || (hmac_vap->vap_state == MAC_VAP_STATE_LISTEN  && hmac_vap->user_nums > 0)
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
        || (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING)
#endif
    )) {
        return;
    }

    // 本VAP处理
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (mvap_up == OSAL_TRUE && hmac_device->up_vap_num > 1) {
            hmac_config_set_pm_by_module_etc(hmac_vap, pm_ctrl_type, pm_enable);
        } else if (mvap_up == OSAL_FALSE) {
            hmac_config_set_pm_by_module_etc(hmac_vap, pm_ctrl_type, MAC_STA_PM_SWITCH_RESET);
        }
    }

    // 另一个VAP处理
    another_up_vap = mac_device_find_another_up_vap_etc(hmac_device, hmac_vap->vap_id);
    if (another_up_vap != OSAL_NULL && another_up_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (mvap_up == OSAL_FALSE) {
            pm_enable = (another_up_vap->sta_pm_handler.last_ps_status == MAC_STA_PM_SWITCH_OFF) ?
                MAC_STA_PM_SWITCH_RESET : another_up_vap->sta_pm_handler.last_ps_status;
        }
        hmac_config_set_pm_by_module_etc(another_up_vap, pm_ctrl_type, pm_enable);
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_set_pm_by_module_etc
 功能描述  : 低功耗控制接口
 输入参数  : hmac_vap_stru *hmac_vap
             mac_pm_ctrl_type_enum pm_ctrl_type
             mac_pm_switch_enum pm_enable
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_set_pm_by_module_etc(hmac_vap_stru *hmac_vap, mac_pm_ctrl_type_enum pm_ctrl_type,
    mac_pm_switch_enum pm_enable)
{
    osal_s32 ret;
    mac_cfg_ps_open_stru    ps_open  = {0};

    if ((pm_enable >= MAC_STA_PM_SWITCH_BUTT) || (pm_ctrl_type >= MAC_STA_PM_CTRL_TYPE_BUTT) ||
        (hmac_vap == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY,
            "hmac_config_set_pm_by_module_etc, PARAM ERROR! pm_ctrl_type = %d, pm_enable = %d ",
            pm_ctrl_type, pm_enable);
        return OAL_FAIL;
    }

    ps_open.pm_enable      = pm_enable;
    ps_open.pm_ctrl_type   = pm_ctrl_type;
    ret = hmac_config_set_sta_pm_on(hmac_vap, &ps_open);
    oam_warning_log3(0, OAM_SF_PWR, "hmac_config_set_pm_by_module_etc, pm_module = %d, pm_enable = %d, cfg ret = %d ",
                     pm_ctrl_type, pm_enable, ret);

    return (osal_u32)ret;
}

osal_s32 hmac_sta_pm_on(hmac_vap_stru *hmac_vap, mac_pm_switch_enum_uint8 enable,
    mac_pm_ctrl_type_enum_uint8 ctrl_type)
{
    mac_cfg_ps_open_stru sta_pm_open;

    (osal_void)memset_s(&sta_pm_open, OAL_SIZEOF(sta_pm_open), 0, OAL_SIZEOF(sta_pm_open));
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    sta_pm_open.pm_enable      = enable;
    sta_pm_open.pm_ctrl_type   = ctrl_type;

    hmac_config_set_sta_pm_on(hmac_vap, &sta_pm_open);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 函 数 名  : uapi_ccpriv_sta_psm_param
 功能描述  : 私有命令,sta psm的低功耗定时器周期pm_timer;
            低功耗定时器重启次数pm_timer_cnt,达到该次数后wifi无数据收发则进入休眠;
            等待接收beacon的超时时间beacon_timeout;
            等待接收组播/广播帧的超时时间dtim3_on;
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_get_psm_param(const osal_s8 **param, mac_cfg_ps_param_stru* ps_info)
{
    osal_s32 ret;
    osal_s32 mcast_timeout, sleep_time, tbtt_offset;

    ret = hmac_ccpriv_get_one_arg_digit(param, &mcast_timeout);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_psm_param::get mcast_timeout return err_code [%d]!}", ret);
        return ret;
    }

    ret = hmac_ccpriv_get_one_arg_digit(param, &sleep_time);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_psm_param::get sleep_time return err_code [%d]!}", ret);
        return ret;
    }

    ret = hmac_ccpriv_get_one_arg_digit(param, &tbtt_offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_psm_param::get tbtt_offset return err_code [%d]!}", ret);
        return ret;
    }

    ps_info->mcast_timeout = (osal_u16)mcast_timeout;
    ps_info->sleep_time = (osal_u16)sleep_time;
    ps_info->tbtt_offset = (osal_u16)tbtt_offset;

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_sta_psm_param(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 pm_timer;
    osal_s32 pm_timer_cnt;
    osal_s32 beacon_timeout;
    mac_cfg_ps_param_stru ps_info;

    (osal_void)memset_s(&ps_info, OAL_SIZEOF(ps_info), 0, OAL_SIZEOF(ps_info));

    ret = hmac_ccpriv_get_one_arg_digit(&param, &pm_timer);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_sta_psm_param::get pm_timer return err_code [%d]!}", ret);
        return ret;
    }
    ps_info.pm_timer = (osal_u16)pm_timer;

    ret = hmac_ccpriv_get_one_arg_digit(&param, &pm_timer_cnt);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_sta_psm_param::get pm_timer_cnt return err_code [%d]!}", ret);
        return ret;
    }
    ps_info.pm_timer_cnt = (osal_u16)pm_timer_cnt;

    ret = hmac_ccpriv_get_one_arg_digit(&param, &beacon_timeout);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_sta_psm_param::get beacon_timeout return err_code [%d]!}", ret);
        return ret;
    }
    ps_info.beacon_timeout = (osal_u16)beacon_timeout;

    ret = hmac_ccpriv_get_psm_param(&param, &ps_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_sta_psm_param::get psm_info return err_code [%d]!}", ret);
        return ret;
    }

    hmac_set_psm_param(hmac_vap, &ps_info);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : wal_ccpriv_psm_open
 功能描述  : 私有命令,sta psm的listen interval / tbtt offset
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_sta_pm_on(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_s32                          ul_ret;
    osal_s32                           sta_pm_open;

    ul_ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &sta_pm_open);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_sta_pm_on::get sta_pm_open return err_code [%d]!}", ul_ret);
        return ul_ret;
    }

    hmac_sta_pm_on(hmac_vap, (osal_u8)sta_pm_open, MAC_STA_PM_CTRL_TYPE_HOST);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 查询sta实时节能状况信息
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_get_sta_ps_stat(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    frw_msg msg = {0};
    unref_param(param);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_GET_STA_PS_STAT, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{hmac_config_set_sta_ps_mode::frw_send_msg_to_device failed[%d].}", ret);
    }

    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_sta_ps_mode(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_s32 ret;
    osal_u8 vap_ps_mode;

    ret = hmac_ccpriv_get_u8_with_check_max(&pc_param, MAX_PSPOLL_PS, &vap_ps_mode);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_sta_pm_on::get sta_pm_open return err_code [%d]!}", ret);
        return ret;
    }

    ret = hmac_config_set_sta_pm_mode_cb(hmac_vap, vap_ps_mode);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_PWR, "{hmac_ccpriv_sta_pm_on::return err code [%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_POWERSAVE
/*****************************************************************************
 函 数 名  : hmac_sta_pm_chba_tbtt_switch
 功能描述  : hmac低功耗设置外置TSF开启/关闭
*****************************************************************************/
osal_bool hmac_sta_pm_achba_keepalive(osal_u8 enable)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_id;

    if (hmac_device_calc_up_vap_num_etc(hmac_device) > 1) {
        return OSAL_FALSE;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[0]); /* 已检查当前有且仅有一个VAP */
    if (!is_legacy_sta(hmac_vap)) {
        oam_warning_log0(0, OAM_SF_ACHBA, "hmac_sta_pm_achba_keepalive::hmac_vap is not legacy sta.");
        return OSAL_FALSE;
    }

    vap_id = hmac_vap->hal_vap->vap_id;
    hal_set_ext_tsf_int(vap_id, enable);
    return OSAL_TRUE;
}
/*****************************************************************************
 函 数 名  : hmac_sleep_check_debug_cnt_incr
 功能描述  : hmac低功耗状态同步功能开启/关闭
*****************************************************************************/
static osal_void hmac_sleep_check_debug_cnt_incr(hmac_not_sleep_type reason)
{
#ifdef _PRE_WLAN_POWERSAVE_DEBUG
    g_host_sleep_check_debug[reason]++;
#endif
}

/*****************************************************************************
 函 数 名  : hmac_ccpriv_sta_system_psm_set
 功能描述  : hmac低功耗状态同步功能开启/关闭
*****************************************************************************/
osal_s32  hmac_ccpriv_sta_system_psm_set(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_u32                          ret;
    osal_s32                           sta_system_pm_open;

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &sta_system_pm_open);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_sta_system_psm_set::get sta_system_psm_set return err_code [%d]!}", ret);
        return ret;
    }
    if ((osal_u8)sta_system_pm_open > 1) { /* 参数范围是1/0 */
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_sta_system_psm_set::invalid param!}\n");
        return OAL_FAIL;
    }
    hmac_sta_pm_ctrl_set((osal_u8)sta_system_pm_open);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : hmac配置外置tbtt offset
*****************************************************************************/
osal_s32 hmac_config_set_ext_offset(hmac_vap_stru *hmac_vap, osal_u16 ext_tbtt_offset)
{
    osal_u16 *ext_inner_offset_diff = hal_pm_get_ext_inner_offset_diff();

    if (osal_unlikely(hmac_vap->hal_vap == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_config_set_ext_offset::hal_vap null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (ext_tbtt_offset > hmac_vap->in_tbtt_offset) {
        *ext_inner_offset_diff = ext_tbtt_offset - hmac_vap->in_tbtt_offset;
    } else {
        oam_warning_log1(0, OAM_SF_PWR,
                         "{hmac_config_set_ext_offset::ext_tbtt_offset not less than in_tbtt_offset[%u]}",
                         hmac_vap->in_tbtt_offset);
        return OAL_FAIL;
    }
    hal_init_pm_info_sync(hmac_vap->hal_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : hmac配置外置tbtt offset ccprive命令解析
*****************************************************************************/
osal_s32  hmac_ccpriv_set_ext_offset(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_u32                          ret;
    osal_s32                          ext_tbtt_offset;

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &ext_tbtt_offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_set_ext_offset::get ext_tbtt_offset return err_code [%d]!}", ret);
        return ret;
    }

    return hmac_config_set_ext_offset(hmac_vap, ext_tbtt_offset);
}

/*****************************************************************************
 功能描述  : hmac配置外置tbtt offset uapi接口W2H消息回调
*****************************************************************************/
osal_s32 hmac_config_set_ext_offset_cb(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u16 ext_tbtt_offset;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL))) {
        oam_warning_log0(0, OAM_SF_PWR, "{hmac_config_set_ext_offset_cb::hmac_vap / param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ext_tbtt_offset = *((osal_u16 *)msg->data);

    return hmac_config_set_ext_offset(hmac_vap, ext_tbtt_offset);
}

/*****************************************************************************
 功能描述  : hmac系统低功耗维测
*****************************************************************************/
osal_u32 hmac_system_psm_debug(hmac_vap_stru *hmac_vap, osal_u8 psm_debug_cmd, osal_u8 psm_debug_switch)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    frw_msg msg = {0};
    osal_u8 psm_debug_info[2];
    osal_u32 ret;

    if (psm_debug_cmd == STA_SYS_PM_STATE) { /* 参数范围是1/0 */
        oam_warning_log3(0, OAM_SF_ANY, "{hmac_system_psm_state::enable[%d] dev_state[%d] timeout_cnt[%d]!}",
                         hmac_sta_pm_ctrl->wlan_pm_enable, hmac_sta_pm_ctrl->wlan_dev_state,
                         hmac_sta_pm_ctrl->wdg_timeout_cnt);
#if (defined ENABLE_LOW_POWER && (ENABLE_LOW_POWER == YES))
        pm_sleep_debug_t sleep_debug = pm_port_get_debug_info();
        oam_warning_log4(0, OAM_SF_CFG, "hal_pm_debug plt ls cnt[%lu] ls wk cnt[%lu] ds cnt[%lu] ds wk cnt[%lu]",
            sleep_debug.pm_ls_count, sleep_debug.pm_ls_wkup_count, sleep_debug.pm_ds_count,
            sleep_debug.pm_ds_wkup_count);
        oam_warning_log1(0, OAM_SF_CFG, "hal_pm_debug plt all wkup cnt[%lu]",
            sleep_debug.pm_all_wkup_count);
#endif
    }

    if (psm_debug_cmd == STA_NOT_SLEEP_REASON_DUMP) {
#ifdef _PRE_WLAN_POWERSAVE_DEBUG
        oam_warning_log3(0, OAM_SF_ANY, "{hmac_system_psm_debug::not_sleep_reason scan[%lu] vap_num[%lu] ps_mode[%lu]}",
            g_host_sleep_check_debug[HMAC_NOT_SLEEP_SCAN], g_host_sleep_check_debug[HMAC_NOT_SLEEP_VAP_NUM],
            g_host_sleep_check_debug[HMAC_NOT_SLEEP_PS_MODE]);
        oam_warning_log2(0, OAM_SF_ANY, "{hhmac_system_psm_debug::not_sleep_reason  hcc[%lu] frw[%lu]}",
            g_host_sleep_check_debug[HMAC_NOT_SLEEP_HCC], g_host_sleep_check_debug[HMAC_NOT_SLEEP_FRW]);
        if (psm_debug_switch == OSAL_TRUE) {
            memset_s(g_host_sleep_check_debug, sizeof(g_host_sleep_check_debug), 0, sizeof(g_host_sleep_check_debug));
        }
#else
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_system_psm_debug::STA_NOT_SLEEP_REASON_DUMP not support");
#endif
    }

    oam_warning_log2(0, OAM_SF_CFG, "hmac_system_psm_debug::cmd %d enable %d", psm_debug_cmd, psm_debug_switch);

    psm_debug_info[0] = psm_debug_cmd;
    psm_debug_info[1] = psm_debug_switch;
    /* snyc ps mode to device */
    msg.data = (osal_u8 *)(&psm_debug_info);
    msg.data_len = sizeof(psm_debug_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_GET_SYS_PM_STAT, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{hmac_system_psm_debug::frw_send_msg_to_device failed[%d].}", ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ccpriv_system_psm_debug
 功能描述  : hmac维测功能开启/关闭
*****************************************************************************/
osal_s32  hmac_ccpriv_system_psm_debug(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_u32                          ret;
    osal_s32                          psm_debug_cmd;
    osal_s32                          psm_debug_switch;

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &psm_debug_cmd);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_system_psm_debug::get psm_debug_cmd return err_code [%d]!}", ret);
        return ret;
    }

    if (psm_debug_cmd > STA_PM_DEBUG_INFO_BUTT || psm_debug_cmd < STA_SYS_PM_STATE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_system_psm_debug::get psm_debug_cmd invalid!}");
        return OAL_FAIL;
    }

    ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &psm_debug_switch);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_system_psm_debug::get psm_debug_switch return err_code [%d]!}", ret);
        return ret;
    }

    if (psm_debug_switch > 1) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_system_psm_debug::get psm_debug_cmd invalid!}");
        return OAL_FAIL;
    }

    return hmac_system_psm_debug(hmac_vap, (osal_u8)psm_debug_cmd, (osal_u8)psm_debug_switch);
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_get_ctrl
 功能描述  : 获取pm_ctrl结构体的指针
*****************************************************************************/
WIFI_HMAC_TCM_TEXT hmac_sta_pm_ctrl_info *hmac_sta_pm_get_ctrl(osal_void)
{
    return &g_hmac_pm_ctrl;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_get_wlan_dev_state
 功能描述  : 获取dmac低功耗状态
*****************************************************************************/
osal_u8 hmac_sta_pm_get_wlan_dev_state(osal_void)
{
    osal_u8 ret;
    osal_mutex_lock(&g_hmac_pm_ctrl.sleep_state_mutex);
    ret = g_hmac_pm_ctrl.wlan_dev_state;
    osal_mutex_unlock(&g_hmac_pm_ctrl.sleep_state_mutex);
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_wait_ack_check
 功能描述  : host唤醒device,等待回复结束的条件检查
*****************************************************************************/
osal_s32 hmac_sta_pm_wait_ack_check(const osal_void *para)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = (hmac_sta_pm_ctrl_info *)(uintptr_t)(para);

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if ((hmac_sta_pm_ctrl->pm_stage == HMAC_WKUP_RSP_RCV) ||
        (hmac_sta_pm_ctrl->pm_stage == HMAC_SLEEP_ALLOW_RCV) ||
        (hmac_sta_pm_ctrl->pm_stage == HMAC_SLEEP_DISALLOW_RCV)) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

OAL_STATIC osal_void hmac_psm_sta_incr_activity_cnt(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    hmac_sta_pm_ctrl->pm_tx_activity_cnt++;
}

OAL_STATIC osal_void hmac_psm_sta_reset_activity_cnt(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    hmac_sta_pm_ctrl->pm_tx_activity_cnt = 0;
}

OAL_STATIC osal_u8 hmac_psm_sta_is_activity_cnt_zero(const hmac_sta_pm_ctrl_info * const hmac_sta_pm_ctrl)
{
    if (hmac_sta_pm_ctrl->pm_tx_activity_cnt == 0) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_ipc_cmd_send
 功能描述  : host通过ipc总线下发hcc消息
*****************************************************************************/
osal_u32 hmac_sta_pm_ipc_cmd_send(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl, osal_u8 hcc_ch_id)
{
    osal_u8 rsv = 0;
    osal_u32 ret = EXT_ERR_SUCCESS;

    if (hmac_sta_pm_ctrl->pm_stage == HMAC_WKUP_REQ_SND) {
        ret = hcc_send_message(hcc_ch_id, H2D_MSG_WLAN_WAKE_UP_REQ, rsv);
        if (ret != EXT_ERR_SUCCESS) {
            oam_error_log2(0, OAM_SF_PWR, "hmac_sta_pm_ipc_cmd_send:: ret[%x] pm_stage[%d]",
                           ret, hmac_sta_pm_ctrl->pm_stage);
            return OAL_FAIL;
        }
    }

    if (hmac_sta_pm_ctrl->pm_stage == HMAC_SLEEP_REQ_SND) {
        ret = hcc_send_message(hcc_ch_id, H2D_MSG_WLAN_SLEEP_REQ, rsv);
        if (ret != EXT_ERR_SUCCESS) {
            oam_error_log2(0, OAM_SF_PWR, "hmac_sta_pm_ipc_cmd_send:: ret[%x] pm_stage[%d]",
                           ret, hmac_sta_pm_ctrl->pm_stage);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_cmd_send
 功能描述  : 通过bus类型选择与dmac交互的消息的发送
*****************************************************************************/
osal_u32 hmac_sta_pm_cmd_send(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    osal_u8 hcc_ch_id = HCC_CHANNEL_AP;
    hcc_handler *hcc = hcc_get_handler(hcc_ch_id);

    if ((hcc == OSAL_NULL) || (hcc->bus == OSAL_NULL) ||
        (hcc->bus->bus_type <= HCC_BUS_MIN) || (hcc->bus->bus_type >= HCC_BUS_BUTT)) {
        oam_error_log0(0, OAM_SF_PWR, "hmac_sta_pm_cmd_send:: ptr is null!");
        return OAL_FAIL;
    }

    /* 根据bus的不同选择不同的发送方式 */
    if (hcc->bus->bus_type == HCC_BUS_IPC) {
        return hmac_sta_pm_ipc_cmd_send(hmac_sta_pm_ctrl, hcc_ch_id);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_wakeup_wait_ack
 功能描述  : 等待dmac对于唤醒消息的回复处理
*****************************************************************************/
osal_u32 hmac_sta_pm_wakeup_wait_ack(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    osal_ulong ret;
    oam_warning_log0(0, OAM_SF_PWR, "hmac_sta_pm_wakeup_wait_ack:: start!");
    ret = osal_wait_timeout_interruptible(&hmac_sta_pm_ctrl->wakeup_done,
                                          hmac_sta_pm_wait_ack_check,
                                          (osal_void *)(uintptr_t)hmac_sta_pm_ctrl,
                                          (osal_ulong)(HMAC_WAKUP_MSG_WAIT_TIMEOUT));
    /* 消息等待超时的情况下,检查dmac通过其他方式上报的低功耗状态 */
    if (ret == 0) {
        oam_error_log0(0, OAM_SF_PWR, "hmac_sta_pm_wakeup_wait_ack:: timeout!");
        return OAL_FAIL;
    }
    oam_warning_log0(0, OAM_SF_PWR, "hmac_sta_pm_wakeup_wait_ack:: end!");
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_wakeup_fail_proc
 功能描述  : 唤醒失败情况下的处理
*****************************************************************************/
osal_void hmac_sta_pm_wakeup_fail_proc(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    hmac_sta_pm_ctrl->wakeup_err_count++;
    /* PM唤醒失败超出门限,启动dfr流程 */
    if (hmac_sta_pm_ctrl->wakeup_err_count >= HMAC_FAIL_MAX_TIMES) {
        oam_error_log1(0, OAM_SF_PWR, "hmac_sta_pm_wakeup_fail_proc:: [%d]times wlan_wakeup_fail!",
                       hmac_sta_pm_ctrl->wakeup_err_count);
        hmac_sta_pm_ctrl->wakeup_err_count = 0;
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_wakeup_check
 功能描述  : 检查是否满足唤醒dmac的条件
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_sta_pm_wakeup_check(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    /* 低功耗未使能不进行唤醒 */
    if (hmac_sta_pm_ctrl->wlan_pm_enable == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    /* device处于唤醒状态时不进行唤醒 */
    if (hmac_sta_pm_ctrl->wlan_dev_state == HMAC_DISALLOW_SLEEP) {
        hmac_psm_sta_incr_activity_cnt(hmac_sta_pm_ctrl);
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_wakeup_dev
 功能描述  : host侧唤醒dmac处理主流程
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_sta_pm_wakeup_dev(uintptr_t *buf, osal_u32 len, osal_u32 data_type)
{
    osal_u32 ret = OAL_FAIL;
    osal_u8 i = 0;
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return OAL_FAIL;
    }

    osal_mutex_lock(&hmac_sta_pm_ctrl->sleep_state_mutex);
    /* 检查是否需要唤醒操作 */
    if (hmac_sta_pm_wakeup_check(hmac_sta_pm_ctrl) == OSAL_FALSE) {
        osal_mutex_unlock(&hmac_sta_pm_ctrl->sleep_state_mutex);
        return OAL_SUCC;
    }
    hal_pm_vote2platform(HAL_PM_WORK);
    hmac_sta_pm_ctrl->pm_stage = HMAC_WKUP_REQ_SND;
    for (i = 0; i < HMAC_MSG_RETRY_NUM; i++) {
        ret = hmac_sta_pm_cmd_send(hmac_sta_pm_ctrl);
        if (ret == OAL_FAIL) {
            continue;
        }
        ret = hmac_sta_pm_wakeup_wait_ack(hmac_sta_pm_ctrl);
        if (ret == OAL_SUCC) {
            break;
        }
    }

    if (ret != OAL_SUCC) {
        hmac_sta_pm_wakeup_fail_proc(hmac_sta_pm_ctrl);
        hmac_sta_pm_ctrl->pm_stage = HMAC_PM_STAGE_INIT;
        osal_mutex_unlock(&hmac_sta_pm_ctrl->sleep_state_mutex);
        return OAL_FAIL;
    }

    hmac_sta_pm_feed_wdg(hmac_sta_pm_ctrl);
    hal_pm_set_host_pm_state(OSAL_FALSE);
    hmac_sta_pm_ctrl->wlan_dev_state = HMAC_DISALLOW_SLEEP;
    hmac_sta_pm_ctrl->wakeup_err_count = 0;
    hmac_sta_pm_ctrl->pm_stage = HMAC_WKUP_CMD_SND;
    osal_mutex_unlock(&hmac_sta_pm_ctrl->sleep_state_mutex);
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 获取HOST侧WIFI的低功耗状态，用于host读写寄存器的判断
******************************************************************************/
WIFI_HMAC_TCM_TEXT osal_u32 hmac_wifi_pm_get_host_allow_sleep()
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    osal_u8 ret = 0;

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return OAL_FAIL;
    }

    osal_mutex_lock(&hmac_sta_pm_ctrl->sleep_state_mutex);
    ret = hal_wifi_pm_get_host_allow_sleep();
    osal_mutex_unlock(&hmac_sta_pm_ctrl->sleep_state_mutex);

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_wakeup_done_callback
 功能描述  : hmac收到dmac消息回复的处理流程
*****************************************************************************/
osal_u32 hmac_sta_pm_wakeup_done_callback(osal_u8 *cb_data)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    unref_param(cb_data);

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return EXT_ERR_FAILURE;
    }

    hmac_sta_pm_ctrl->pm_stage = HMAC_WKUP_RSP_RCV;
    osal_wait_wakeup(&hmac_sta_pm_ctrl->wakeup_done);

    return EXT_ERR_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_sleep_wait_ack
 功能描述  : 等待dmac对于睡眠请求消息的回复处理
*****************************************************************************/
osal_u32 hmac_sta_pm_sleep_wait_ack(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    osal_ulong ret;
    oam_warning_log0(0, OAM_SF_PWR, "hmac_sta_pm_sleep_wait_ack:: start!");
    ret = osal_wait_timeout_interruptible(&hmac_sta_pm_ctrl->sleep_request_ack,
                                          hmac_sta_pm_wait_ack_check,
                                          (osal_void *)(uintptr_t)hmac_sta_pm_ctrl,
                                          (osal_ulong)(HMAC_SLEEP_MSG_WAIT_TIMEOUT));
    /* 消息等待超时的情况下,检查dmac通过其他方式上报的低功耗状态 */
    if (ret == 0) {
        oam_error_log0(0, OAM_SF_PWR, "hmac_sta_pm_sleep_wait_ack:: timeout!");
        return OAL_FAIL;
    }
    oam_warning_log0(0, OAM_SF_PWR, "hmac_sta_pm_sleep_wait_ack:: end!");
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_check_hcc_empty
 功能描述  : hmac侧对HCC队列进行检查，防止队列有包/消息
*****************************************************************************/
osal_u32 hmac_sta_pm_check_hcc_empty(osal_void)
{
    osal_u8 hcc_ch_id = HCC_CHANNEL_AP;
    hcc_handler *hcc = hcc_get_handler(hcc_ch_id);
    osal_u8 i;
    hcc_trans_queue *queue = OSAL_NULL;
    hcc_queue_cfg *q_cfg = OSAL_NULL;

    if (hcc == OSAL_NULL) {
        return OSAL_TRUE;
    }

    q_cfg = hcc->que_cfg;
    for (i = 0; i < (hcc->que_max_cnt << 1); i++) {
        if (q_cfg[i].queue_id >= hcc->que_max_cnt) {
            break;
        }
        queue = &hcc->hcc_resource.hcc_queues[q_cfg[i].dir][q_cfg[i].queue_id];
        if (hcc_is_list_empty(&queue->queue_info) == OSAL_FALSE) {
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}

osal_u32 hmac_sta_pm_check_up_vap_ps_enable(hmac_device_stru *hmac_device)
{
    osal_u32 vap_idx;
    hmac_vap_stru *hmac_vap_tmp = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OAL_PTR_NULL) {
            continue;
        }
        if (hmac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_AP) {
            return OSAL_FALSE;
        }
        if (((hmac_vap_tmp->vap_state == MAC_VAP_STATE_UP) || (hmac_vap_tmp->vap_state == MAC_VAP_STATE_PAUSE)) &&
            ((hmac_vap_tmp->sta_pm_handler.vap_ps_mode == NO_POWERSAVE) ||
            (hmac_vap_tmp->sta_pm_handler.vap_ps_mode > MAX_FAST_PS))) {
            return OSAL_FALSE;
        }
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_STA_UAPSD_GET_UAPSD_INFO);
        if (fhook != OSAL_NULL) {
            if (((hmac_uapsd_sta_get_uapsd_info_cb)fhook)(hmac_vap_tmp->vap_id) != OSAL_NULL &&
                ((hmac_uapsd_sta_get_uapsd_info_cb)fhook)(hmac_vap_tmp->vap_id)->uapsd_switch == OSAL_TRUE) {
                return OSAL_FALSE;
            }
        }
        if (hmac_vap_tmp->vap_state == MAC_VAP_STATE_STA_WAIT_SCAN ||
            (hmac_vap_tmp->vap_state >= MAC_VAP_STATE_STA_JOIN_COMP &&
            hmac_vap_tmp->vap_state < MAC_VAP_STATE_BUTT)) {
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_sleep_check
 功能描述  : hmac侧满足进入睡眠的条件检查
*****************************************************************************/
osal_u32 hmac_sta_pm_sleep_check(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (hmac_sta_pm_ctrl->wlan_pm_enable == OSAL_FALSE) {
        return OSAL_FALSE;
    }

    /* device处于睡眠状态时不进行睡眠请求 */
    if (hmac_sta_pm_ctrl->wlan_dev_state == HMAC_ALLOW_SLEEP) {
        return OSAL_FALSE;
    }

    if (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) {
        hmac_sleep_check_debug_cnt_incr(HMAC_NOT_SLEEP_SCAN);
        return OSAL_FALSE;
    }

    if (hmac_device_calc_up_vap_num_etc(hmac_device) > 1) {
        hmac_sleep_check_debug_cnt_incr(HMAC_NOT_SLEEP_VAP_NUM);
        return OSAL_FALSE;
    }

    if (hmac_sta_pm_check_up_vap_ps_enable(hmac_device) == OSAL_FALSE) {
        hmac_sleep_check_debug_cnt_incr(HMAC_NOT_SLEEP_PS_MODE);
        return OSAL_FALSE;
    }

    if (hcc_chan_is_busy(HCC_CHANNEL_AP) == OSAL_TRUE) {
        hmac_sleep_check_debug_cnt_incr(HMAC_NOT_SLEEP_HCC);
        return OSAL_FALSE;
    }

    if (frw_pm_queue_empty_check() == OSAL_FALSE) {
        hmac_sleep_check_debug_cnt_incr(HMAC_NOT_SLEEP_FRW);
        return OSAL_FALSE;
    }

    if (hmac_psm_sta_is_activity_cnt_zero(hmac_sta_pm_ctrl) == OSAL_FALSE) {
        hmac_psm_sta_reset_activity_cnt(hmac_sta_pm_ctrl);
        hmac_sleep_check_debug_cnt_incr(HMAC_NOT_SLEEP_TX_CNT);
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_sleep_fail_proc
 功能描述  : hmac请求睡眠多次超时失败后的处理逻辑
*****************************************************************************/
osal_void hmac_sta_pm_sleep_fail_proc(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    hmac_sta_pm_ctrl->fail_sleep_count++;
    hmac_sta_pm_feed_wdg(hmac_sta_pm_ctrl);
    /* PM请求睡眠失败超出门限 */
    if (hmac_sta_pm_ctrl->fail_sleep_count > HMAC_FAIL_MAX_TIMES) {
        oam_error_log1(0, OAM_SF_PWR, "hmac_sta_pm_sleep_fail_proc:: [%d]times wlan_sleep_fail!",
                       hmac_sta_pm_ctrl->fail_sleep_count);
        hmac_sta_pm_ctrl->fail_sleep_count = 0;
        hmac_sta_pm_stop_wdg(hmac_sta_pm_ctrl);
    }

    hmac_sta_pm_ctrl->pm_stage = HMAC_SLEEP_CMD_SND;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_sleep_stage_proc
 功能描述  : hmac请求睡眠对于dmac回复消息的处理
*****************************************************************************/
osal_void hmac_sta_pm_sleep_stage_proc(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    if (hmac_sta_pm_ctrl->pm_stage == HMAC_SLEEP_ALLOW_RCV) {
        /* 向平台投睡眠票 */
        hal_pm_vote2platform(HAL_PM_DEEPSLEEP);
        hal_pm_set_host_pm_state(OSAL_TRUE);
        hmac_sta_pm_ctrl->wlan_dev_state = HMAC_ALLOW_SLEEP;
        hmac_sta_pm_stop_wdg(hmac_sta_pm_ctrl);
    }

    if (hmac_sta_pm_ctrl->pm_stage == HMAC_SLEEP_DISALLOW_RCV) {
        /* 向平台投阻止睡眠票 */
        hal_pm_vote2platform(HAL_PM_WORK);
        hal_pm_set_host_pm_state(OSAL_FALSE);
        hmac_sta_pm_ctrl->wlan_dev_state = HMAC_DISALLOW_SLEEP;
        hmac_sta_pm_feed_wdg(hmac_sta_pm_ctrl);
    }
    hmac_sta_pm_ctrl->fail_sleep_count = 0;
    hmac_sta_pm_ctrl->pm_stage = HMAC_SLEEP_CMD_SND;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_sleep_stage_proc
 功能描述  : hmac睡眠定时器超时后,发起睡眠请求的的处理主流程
*****************************************************************************/
osal_u32 hmac_sta_pm_sleep_cmd(osal_void)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    osal_u32 ret = OAL_FAIL;
    osal_u8 i = 0;

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return OAL_FAIL;
    }

    /* 检查是否需要睡眠请求操作 */
    osal_mutex_lock(&hmac_sta_pm_ctrl->sleep_state_mutex);
    if (hmac_sta_pm_sleep_check(hmac_sta_pm_ctrl) == OSAL_FALSE) {
        hmac_sta_pm_feed_wdg(hmac_sta_pm_ctrl);
        osal_mutex_unlock(&hmac_sta_pm_ctrl->sleep_state_mutex);
        return OAL_SUCC;
    }
    hal_pm_vote2platform(HAL_PM_WORK);
    hmac_sta_pm_ctrl->pm_stage = HMAC_SLEEP_REQ_SND;
    for (i = 0; i < HMAC_MSG_RETRY_NUM; i++) {
        ret = hmac_sta_pm_cmd_send(hmac_sta_pm_ctrl);
        if (ret == OAL_FAIL) {
            continue;
        }
        ret = hmac_sta_pm_sleep_wait_ack(hmac_sta_pm_ctrl);
        if (ret == OAL_SUCC) {
            break;
        }
    }

    if (ret != OAL_SUCC) {
        hmac_sta_pm_sleep_fail_proc(hmac_sta_pm_ctrl);
        osal_mutex_unlock(&hmac_sta_pm_ctrl->sleep_state_mutex);
        return OAL_FAIL;
    }

    hmac_sta_pm_sleep_stage_proc(hmac_sta_pm_ctrl);
    osal_mutex_unlock(&hmac_sta_pm_ctrl->sleep_state_mutex);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_allow_sleep_callback
 功能描述  : hmac收到dmac允许睡眠消息的处理
*****************************************************************************/
osal_u32 hmac_sta_pm_allow_sleep_callback(osal_u8 *cb_data)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    unref_param(cb_data);

    hmac_sta_pm_ctrl->pm_stage = HMAC_SLEEP_ALLOW_RCV;
    osal_wait_wakeup(&hmac_sta_pm_ctrl->sleep_request_ack);
    return EXT_ERR_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_disallow_sleep_callback
 功能描述  : hmac收到dmac不允许睡眠消息的处理
*****************************************************************************/
osal_u32 hmac_sta_pm_disallow_sleep_callback(osal_u8 *cb_data)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    unref_param(cb_data);

    hmac_sta_pm_ctrl->pm_stage = HMAC_SLEEP_DISALLOW_RCV;
    osal_wait_wakeup(&hmac_sta_pm_ctrl->sleep_request_ack);
    return EXT_ERR_SUCCESS;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_wdg_timeout
 功能描述  : 睡眠定时器超时的处理函数
*****************************************************************************/
osal_u32 hmac_sta_pm_wdg_timeout(osal_void *timeout_arg)
{
    unref_param(timeout_arg);
    return hmac_sta_pm_sleep_cmd();
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_stop_wdg
 功能描述  : 睡眠定时器停止的处理函数
*****************************************************************************/
osal_void hmac_sta_pm_stop_wdg(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    /* 定时器已关闭，则不用再关闭 */
    if (hmac_sta_pm_ctrl->watchdog_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(hmac_sta_pm_ctrl->watchdog_timer));
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_feed_wdg
 功能描述  : 开启睡眠定时器的处理函数
*****************************************************************************/
osal_void hmac_sta_pm_feed_wdg(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl)
{
    frw_create_timer_entry(&(hmac_sta_pm_ctrl->watchdog_timer),
                           hmac_sta_pm_wdg_timeout,
                           hmac_sta_pm_ctrl->wdg_timeout_cnt,
                           (osal_void *)hmac_sta_pm_ctrl,
                           OSAL_TRUE);
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_ctrl_set
 功能描述  : hmac低功耗状态同步功能开启/关闭设置
*****************************************************************************/
osal_u32 hmac_sta_pm_ctrl_set(osal_u32 flag)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return OAL_FAIL;
    }

    if (flag == OSAL_TRUE) {
        hmac_sta_pm_feed_wdg(hmac_sta_pm_ctrl);
    } else {
        if (hmac_sta_pm_wakeup_dev(OSAL_NULL, 0, 0) == OAL_FAIL) {
            return OAL_FAIL;
        }
        hmac_sta_pm_stop_wdg(hmac_sta_pm_ctrl);
    }
    hmac_sta_pm_ctrl->wlan_pm_enable = flag;
    return OAL_SUCC;
}

osal_bool hmac_sta_pm_set_on(osal_void *notify_data)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    if (notify_data == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_sta_pm_set_on::The ip_addr is NULL. }");
        return OSAL_FALSE;
    }

    if (hmac_device_calc_up_vap_num_etc(hmac_device) > 1) {
        return OSAL_FALSE;
    }

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[0]); /* 已检查当前有且仅有一个VAP */
    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_FALSE;
    }

    oam_warning_log0(0, OAM_SF_PWR, "hmac_sta_pm_set_on::Automatic enter power save mode");
    hmac_sta_pm_on(hmac_vap, MAC_STA_PM_SWITCH_ON, MAC_STA_PM_CTRL_TYPE_HOST);
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_sta_pm_ctrl_init
 功能描述  : PM模块初始化
*****************************************************************************/
osal_u32 hmac_sta_pm_ctrl_init(osal_void)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    osal_u8 hcc_ch_id = HCC_CHANNEL_AP;

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return OAL_FAIL;
    }
    memset_s(hmac_sta_pm_ctrl, sizeof(hmac_sta_pm_ctrl_info), 0, sizeof(hmac_sta_pm_ctrl_info));

    /* PM标志初始化 */
    hmac_sta_pm_ctrl->wlan_pm_enable = OSAL_FALSE;
    hmac_sta_pm_ctrl->wlan_dev_state = HMAC_DISALLOW_SLEEP;
    hmac_sta_pm_ctrl->pm_stage = HMAC_PM_STAGE_INIT;
    hmac_sta_pm_ctrl->wdg_timeout_cnt = HMAC_SLEEP_TIMER_PERIOD;
    hmac_sta_pm_ctrl->wakeup_err_count = 0;
    hmac_sta_pm_ctrl->fail_sleep_count = 0;
    osal_wait_init(&hmac_sta_pm_ctrl->wakeup_done);
    osal_wait_init(&hmac_sta_pm_ctrl->sleep_request_ack);
    osal_mutex_init(&hmac_sta_pm_ctrl->sleep_state_mutex);

    /* hcc 消息注册，hcc channal的注册和获取待适配 */
    hcc_message_register(hcc_ch_id, 0, D2H_MSG_WLAN_WAKEUP_SUCC,
        hmac_sta_pm_wakeup_done_callback, (osal_u8 *)hcc_get_handler(hcc_ch_id));
    hcc_message_register(hcc_ch_id, 0, D2H_MSG_WLAN_ALLOW_SLEEP,
        hmac_sta_pm_allow_sleep_callback, (osal_u8 *)hcc_get_handler(hcc_ch_id));
    hcc_message_register(hcc_ch_id, 0, D2H_MSG_WLAN_DISALLOW_SLEEP,
        hmac_sta_pm_disallow_sleep_callback, (osal_u8 *)hcc_get_handler(hcc_ch_id));

    hmac_feature_hook_register(HMAC_FHOOK_PM_STA_STAT, hmac_sta_pm_get_wlan_dev_state);
    hmac_feature_hook_register(HMAC_FHOOK_PM_STA_WAKEUP_DEV, hmac_sta_pm_wakeup_dev);

    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_INETADDR_NOTIFIER_UP, hmac_sta_pm_set_on);

    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_PSM_PARAM, hmac_config_set_ext_offset_cb);

    hmac_ccpriv_register((const osal_s8 *)"set_sta_system_psm", hmac_ccpriv_sta_system_psm_set);
    hmac_ccpriv_register((const osal_s8 *)"set_ext_tbtt_offset", hmac_ccpriv_set_ext_offset);
    hmac_ccpriv_register((const osal_s8 *)"system_psm_debug", hmac_ccpriv_system_psm_debug);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_sta_pm_ctrl_exit
 功能描述  : PM模块退出
*****************************************************************************/
osal_u32 hmac_sta_pm_ctrl_exit(osal_void)
{
    hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl = hmac_sta_pm_get_ctrl();
    osal_u8 hcc_ch_id = HCC_CHANNEL_AP;

    if (hmac_sta_pm_ctrl == OSAL_NULL) {
        return OAL_FAIL;
    }

    /* 停止定时器,唤醒device */
    hmac_sta_pm_ctrl_set(OSAL_FALSE);

    osal_wait_destroy(&hmac_sta_pm_ctrl->wakeup_done);
    osal_wait_destroy(&hmac_sta_pm_ctrl->sleep_request_ack);
    osal_mutex_destroy(&hmac_sta_pm_ctrl->sleep_state_mutex);

    /* hcc 消息去注册，hcc channal的注册和获取待适配 */
    hcc_message_unregister(hcc_ch_id, 0, D2H_MSG_WLAN_WAKEUP_SUCC);
    hcc_message_unregister(hcc_ch_id, 0, D2H_MSG_WLAN_ALLOW_SLEEP);
    hcc_message_unregister(hcc_ch_id, 0, D2H_MSG_WLAN_DISALLOW_SLEEP);

    hmac_feature_hook_unregister(HMAC_FHOOK_PM_STA_STAT);
    hmac_feature_hook_unregister(HMAC_FHOOK_PM_STA_WAKEUP_DEV);

    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_INETADDR_NOTIFIER_UP, hmac_sta_pm_set_on);

    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_PSM_PARAM);

    hmac_ccpriv_unregister((const osal_s8 *)"set_sta_system_psm");
    hmac_ccpriv_unregister((const osal_s8 *)"set_ext_tbtt_offset");
    hmac_ccpriv_unregister((const osal_s8 *)"system_psm_debug");

    return OAL_SUCC;
}
#endif

osal_u32 hmac_sta_pm_init(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"set_psm_para", hmac_ccpriv_sta_psm_param);
    hmac_ccpriv_register((const osal_s8 *)"set_sta_pm", hmac_ccpriv_sta_pm_on);
    hmac_ccpriv_register((const osal_s8 *)"get_sta_ps_stat", hmac_ccpriv_get_sta_ps_stat);
    hmac_ccpriv_register((const osal_s8 *)"set_ps_mode", hmac_ccpriv_sta_ps_mode);
#endif
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_STA_PM_ON, hmac_config_set_sta_pm_on_cb);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_PSM_PARAM, hmac_config_set_sta_pm_param_cb);
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
    hmac_sta_pm_ctrl_init();
#endif
    return OAL_SUCC;
}

osal_void hmac_sta_pm_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"set_psm_para");
    hmac_ccpriv_unregister((const osal_s8 *)"set_sta_pm");
    hmac_ccpriv_unregister((const osal_s8 *)"get_sta_ps_stat");
    hmac_ccpriv_unregister((const osal_s8 *)"set_ps_mode");
#endif
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_STA_PM_ON);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_PSM_PARAM);
#ifdef _PRE_WLAN_FEATURE_POWERSAVE
    hmac_sta_pm_ctrl_exit();
#endif
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
