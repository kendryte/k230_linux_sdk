/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: WOW hmac function.
 * Create: 2021-04-15
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_WOW_OFFLOAD
#include "hmac_wow.h"
#include "mac_resource_ext.h"
#include "hmac_vap.h"
#include "frw_ext_if.h"
#include "hmac_main.h"
#include "wlan_msg.h"
#include "hmac_mgmt_classifier.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif
#include "hal_device_fsm.h"
#include "msg_wow_rom.h"
#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
#include "plat_sha256_calc.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_WOW_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_wow_info_stru g_wow_info = {0};
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
mac_wow_offload_info_stru g_wow_offload_info;
osal_u8 g_wow_all_vap_state[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
#endif

/*****************************************************************************
 函 数 名  : hmac_wow_set_host_state
 功能描述  : 设置host侧状态
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_wow_set_host_state(osal_u8 sleep_state)
{
    oam_warning_log2(0, OAM_SF_WOW, "{hmac_wow_set_host_state::state[%d]->[%d]!}",
                     g_wow_info.host_sleep_state, sleep_state);
    g_wow_info.host_sleep_state = sleep_state;
}

/*****************************************************************************
 函 数 名  : hmac_wow_get_host_state
 功能描述  : 获取host侧状态
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u8 hmac_wow_get_host_state(osal_void)
{
    return g_wow_info.host_sleep_state;
}

osal_u32 hmac_wow_delba_foreach_tid(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret;
    osal_u8 tidno;
    mac_action_mgmt_args_stru action_args;

    action_args.category = MAC_ACTION_CATEGORY_BA;
    action_args.action = MAC_BA_ACTION_DELBA;
    action_args.arg2 = MAC_ORIGINATOR_DELBA;
    action_args.arg3 = MAC_UNSPEC_REASON;
    action_args.arg5 = hmac_user->user_mac_addr;

    for (tidno = 0; tidno < WLAN_TID_MAX_NUM; tidno++) {
        /* 填充action帧参数:tidno      */
        action_args.arg1 = tidno;
        oam_warning_log2(0, OAM_SF_WOW, "hmac_wow_delba_foreach_tid::tidno [%d] direction [%d]", tidno,
            action_args.arg2);
        /* 发送delba报文 */
        ret = hmac_mgmt_tx_action_etc(hmac_vap, hmac_user, &action_args);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_WOW, "hmac_wow_delba_foreach_tid::return [%d]", ret);
        }
    }

    return OAL_SUCC;
}

osal_u32 hmac_wow_delba(osal_void)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *dlist_tmp = OAL_PTR_NULL;
    osal_u8 vap_idx = 0;

    hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 遍历device下的所有vap */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(vap_idx);
        /* 遍历vap下的所有user */
        osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
            hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
            if (hmac_user == OAL_PTR_NULL) {
                oam_error_log1(0, OAM_SF_UM, "vap_id[%d] {hmac_wow_delba::user_tmp null.}", hmac_vap->vap_id);
                continue;
            }
            /* 关闭tx方向ba会话 */
            hmac_wow_delba_foreach_tid(hmac_vap, hmac_user);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : Host 侧发给 Device 侧 睡眠/ 唤醒通知
*****************************************************************************/
osal_u32 hmac_wow_host_sleep_wakeup_notify(osal_u8 is_sleep_req)
{
    osal_u32 ret;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    mac_h2d_syn_info_hdr_stru sync_hdr = {0};
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    hmac_vap = mac_res_get_hmac_vap(0);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_WOW, "{hmac_wow_host_sleep_wakeup_notify::hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    sync_hdr.msg_type = MAC_WOW_SLEEP_NOTIFY_MSG;
    sync_hdr.notify_param = is_sleep_req;

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    msg_info.data = (osal_u8 *)&sync_hdr;
    msg_info.data_len = sizeof(mac_h2d_syn_info_hdr_stru);
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_HOST_SLEEP_NOTIFY, &msg_info, OSAL_TRUE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_WOW,
                         "{hmac_config_host_sleep_wakeup_notify::send_event failed[%d],is_sleep_req = %d}",
                         ret, is_sleep_req);
    }

    return ret;
}
#ifndef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
/*****************************************************************************
 函 数 名  : hmac_wow_host_sleep_request_asyn_handle
 功能描述  : 通知DMAC睡眠，本端做去扫描和停止发包动作
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_wow_host_sleep_request_asyn_handle(osal_void)
{
    osal_u32 ret;

    hmac_wow_set_host_state(HMAC_HOST_STATE_SLEPT);
    /* del tx ba */
    hmac_wow_delba();

    ret = hmac_wow_host_sleep_wakeup_notify(MAC_WOW_SLEEP_REQUEST);
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_wow_host_sleep_request
 功能描述  : Host 侧休眠请求
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_wow_host_sleep_request(osal_void)
{
    osal_u8  sleep_state;

    sleep_state = hmac_wow_get_host_state();
    oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_host_sleep_request, cur_state = %d}", sleep_state);

    if (sleep_state == HMAC_HOST_STATE_WAKEUP) {
        hmac_wow_host_sleep_request_asyn_handle();
    } else {
        oam_error_log2(0, OAM_SF_WOW, "{hmac_wow_host_sleep_request:: ERROR_state[%d], Expect_state[%d]!}",
                       sleep_state, HMAC_HOST_STATE_WAKEUP);
    }
}

/*****************************************************************************
 函 数 名  : hmac_wow_host_wakeup_notify_asyn_handle
 功能描述  : 唤醒通知异步处理
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_wow_host_wakeup_notify_asyn_handle(osal_void)
{
    osal_u32 ret;

    hmac_wow_set_host_state(HMAC_HOST_STATE_WAKEUP);
    ret = hmac_wow_host_sleep_wakeup_notify(MAC_WOW_WAKEUP_NOTIFY);
    oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_host_wakeup_notify_asyn_handle, ret = %d}", ret);
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_wow_host_wakeup_notify
 功能描述  : 唤醒通知
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_wow_host_wakeup_notify(osal_void)
{
    osal_u8  sleep_state;

    sleep_state = hmac_wow_get_host_state();
    oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_host_wakeup_notify, sleep_state = %d}", sleep_state);
    if (sleep_state == HMAC_HOST_STATE_SLEPT) {
        hmac_wow_set_host_state(HMAC_HOST_STATE_WAKEUP);
        hmac_wow_host_wakeup_notify_asyn_handle();
    } else {
        oam_error_log2(0, OAM_SF_WOW, "{hmac_wow_host_wakeup_notify:: ERROR_state[%d], Expect_state[%d]!}",
                       sleep_state, HMAC_HOST_STATE_SLEPT);
    }
}

/*****************************************************************************
 函 数 名  : hmac_wow_host_sleep_cmd
 功能描述  : 配置HOST进行睡眠/唤醒
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_wow_host_sleep_cmd(hmac_vap_stru *hmac_vap, osal_u32 is_host_sleep)
{
    unref_param(hmac_vap);

    if (g_wow_info.wow_cfg.wow_en == MAC_WOW_DISABLE) {
        oam_warning_log0(0, OAM_SF_WOW, "{hmac_wow_host_sleep_cmd::wow is not enabled.}");
        return;
    }

    if (is_host_sleep == OSAL_TRUE) {
        hmac_wow_host_sleep_request();
    } else {
        hmac_wow_host_wakeup_notify();
    }
}
#endif
/*****************************************************************************
 函 数 名  : hmac_wow_set_dmac_cfg
 功能描述  : H2D WOW 参数下发
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_wow_set_dmac_cfg(osal_void)
{
    osal_u32 ret;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    oam_warning_log1(0, OAM_SF_WOW, "hmac_wow_set_dmac_cfg, wow_event[0x%X]", g_wow_info.wow_cfg.wow_event);

    hmac_vap = mac_res_get_hmac_vap(0);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_WOW, "{hmac_wow_set_dmac_cfg::hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    msg_info.data = (osal_u8 *)&g_wow_info.wow_cfg;
    msg_info.data_len = sizeof(g_wow_info.wow_cfg);
    ret = send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_SET_WOW_PARAM, &msg_info, OSAL_TRUE);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_set_dmac_cfg::send_event failed[%d]}", ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_wow_set_wow_cmd
 功能描述  : 设置wow命令
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_wow_set_wow_cmd(osal_u32 wow_event)
{
    osal_u32 ret;

    if (g_wow_info.wow_cfg.wow_en == MAC_WOW_DISABLE) {
        oam_warning_log0(0, OAM_SF_WOW, "{hmac_wow_set_wow_cmd::wow is not enabled.}");
        return;
    }

    oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_set_wow_cmd, wow_event[0x%X]}", wow_event);

    g_wow_info.wow_cfg.wow_event = wow_event;

    /* 下发参数到 DMAC */
    ret = hmac_wow_set_dmac_cfg();
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_set_dmac_cfg return NON SUCCESS. ");
    }
}
#ifndef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
/*****************************************************************************
 函 数 名  : hmac_wow_set_wow_en_cmd
 功能描述  : 开启/关闭WOW功能，下发配置到DMAC
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_wow_set_wow_en_cmd(osal_u32 wow_en)
{
    osal_u32 ret;

    oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_set_wow_en_cmd, wow_en[0x%x]}", wow_en);
    g_wow_info.wow_cfg.wow_en = (osal_u8)wow_en;

    if (!g_wow_info.wow_cfg.wow_en) {
        g_wow_info.host_sleep_state = HMAC_HOST_STATE_WAKEUP;
    }

    /* 下发参数到 DMAC */
    ret = hmac_wow_set_dmac_cfg();
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_set_dmac_cfg return NON SUCCESS. ");
    }
}
#endif
/*****************************************************************************
 函 数 名  : hmac_check_if_del_pattern
 功能描述  : 清除指定pattern
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void hmac_check_if_del_pattern(osal_u16 index, const osal_u16 *pattern_num)
{
    if ((1 << index) & g_wow_info.wow_cfg.wow_pattern.pattern_map) {
        g_wow_info.wow_cfg.wow_pattern.pattern_map &= ~(1 << index);
        g_wow_info.wow_cfg.wow_pattern.pattern_num--;
        g_wow_info.wow_cfg.wow_pattern.pattern[index].pattern_len = 0;

        memset_s(g_wow_info.wow_cfg.wow_pattern.pattern[index].pattern_data, WOW_NETPATTERN_MAX_LEN, 0,
                 WOW_NETPATTERN_MAX_LEN);
        oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_set_pattern_cmd,del pattern,Del,totalNum=%d}", *pattern_num);
    } else {
        oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_set_pattern_cmd,No del pattern,Del,totalNum=%d}", *pattern_num);
    }
}

/*****************************************************************************
 函 数 名  : hmac_wow_set_pattern_cmd
 功能描述  : 配置pattern，包括添加、删除和清除所有，需要下发到DMAC
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_wow_set_pattern_cmd(const hmac_cfg_wow_pattern_param_stru *pattern)
{
    osal_u16 *pattern_num = &g_wow_info.wow_cfg.wow_pattern.pattern_num;
    osal_u16  index       = pattern->pattern_index;
    osal_u32  len         = pattern->pattern_len;

    if (pattern->pattern_option == MAC_WOW_PATTERN_PARAM_OPTION_ADD) {
        if ((index >= WOW_NETPATTERN_MAX_NUM) || (len == 0) || (len > WOW_NETPATTERN_MAX_LEN)) {
            oam_error_log2(0, OAM_SF_WOW, "{hmac_wow_set_pattern_cmd::ADD::param Err,index=%d,len=%d}", index, len);
            return OAL_FAIL;
        }
        if (memcpy_s(g_wow_info.wow_cfg.wow_pattern.pattern[index].pattern_data,
            WOW_NETPATTERN_MAX_LEN, pattern->pattern_value, len) != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hmac_wow_set_pattern_cmd:: pattern_value memcpy_s fail.");
            return OAL_FAIL;
        }
        g_wow_info.wow_cfg.wow_pattern.pattern[index].pattern_len = len;

        if (!((1 << index) & g_wow_info.wow_cfg.wow_pattern.pattern_map)) {
            g_wow_info.wow_cfg.wow_pattern.pattern_map |= (osal_u16)(1 << index);
            g_wow_info.wow_cfg.wow_pattern.pattern_num++;
        }

        oam_warning_log4(0, OAM_SF_WOW, "{hmac_wow_set_pattern_cmd, add new pattern,len=%d,total_num=%d,Value[%X][%X]}",
                         len, *pattern_num, pattern->pattern_value[0], pattern->pattern_value[1]);
    } else if (pattern->pattern_option == MAC_WOW_PATTERN_PARAM_OPTION_DEL) {
        if (index >= WOW_NETPATTERN_MAX_NUM) {
            oam_error_log1(0, OAM_SF_WOW, "{hmac_wow_set_pattern_cmd::DEL: param Err,index=%d}", len);
            return OAL_FAIL;
        }
        hmac_check_if_del_pattern(index, pattern_num);
    } else if (pattern->pattern_option == MAC_WOW_PATTERN_PARAM_OPTION_CLR) {
        memset_s(&g_wow_info.wow_cfg.wow_pattern, sizeof(g_wow_info.wow_cfg.wow_pattern), 0,
                 sizeof(g_wow_info.wow_cfg.wow_pattern));
        oam_warning_log0(0, OAM_SF_WOW, "{hmac_wow_set_pattern_cmd, pattern clear}");
    } else {
        oam_warning_log1(0, OAM_SF_WOW, "{hmac_wow_set_pattern_cmd, error option[%d]}", pattern->pattern_option);
    }

    /* 下发参数到 DMAC */
    if (hmac_wow_set_dmac_cfg() != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_set_dmac_cfg return NON SUCCESS. ");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : uapi_wlan_set_wow_event
 功能描述  : 设置强制睡眠功能开关接口
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_void uapi_wlan_set_wow_event(osal_u32 event)
{
    hmac_wow_set_wow_cmd(event);
}

/*****************************************************************************
 函 数 名  : uapi_wlan_add_netpattern
 功能描述  : 强制睡眠netpattern唤醒报文格式的添加API接口
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 uapi_wlan_add_netpattern(osal_u32 netpattern_index, osal_uchar *netpattern_data, osal_u32 netpattern_len)
{
    hmac_cfg_wow_pattern_param_stru pattern;

    if (memset_s(&pattern, sizeof(hmac_cfg_wow_pattern_param_stru), 0,
        sizeof(hmac_cfg_wow_pattern_param_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "uapi_wlan_add_netpattern:: memset_s fail.");
        return OAL_FAIL;
    }

    pattern.pattern_option = MAC_WOW_PATTERN_PARAM_OPTION_ADD;
    pattern.pattern_index = (osal_u16)netpattern_index;
    pattern.pattern_len = netpattern_len;

    if ((pattern.pattern_index >= WOW_NETPATTERN_MAX_NUM) ||
        (pattern.pattern_len == 0) ||
        (pattern.pattern_len > WOW_NETPATTERN_MAX_LEN) ||
        (netpattern_data == OSAL_NULL)) {
        oam_error_log3(0, OAM_SF_WOW,
            "{uapi_wlan_add_netpattern::WOW,ADD::param error, index = %d, len = %d, netpattern_data[%p].}",
            pattern.pattern_index, pattern.pattern_len, (uintptr_t)netpattern_data);
        return OAL_FAIL;
    }

    if (memcpy_s(&pattern.pattern_value[0], WOW_NETPATTERN_MAX_LEN,
        netpattern_data, pattern.pattern_len) != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "uapi_wlan_add_netpattern:: netpattern_data memcpy_s fail.");
        return OAL_FAIL;
    }

    return hmac_wow_set_pattern_cmd(&pattern);
}

/*****************************************************************************
 函 数 名  : uapi_wlan_del_netpattern
 功能描述  : 强制睡眠netpattern唤醒报文格式的删除API接口
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 uapi_wlan_del_netpattern(osal_u32 netpattern_index)
{
    hmac_cfg_wow_pattern_param_stru pattern = {0};
    pattern.pattern_option = MAC_WOW_PATTERN_PARAM_OPTION_DEL;
    pattern.pattern_index = (osal_u16)netpattern_index;
    return hmac_wow_set_pattern_cmd(&pattern);
}

/*****************************************************************************
 函 数 名  : hmac_config_wow_set_param
 功能描述  : 抛事件到DMAC同步WOW参数
*****************************************************************************/
osal_s32 hmac_config_wow_set_param(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_wow_set_param:oal_net_dev_priv(net_dev) is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_wow_set_dmac_cfg();
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_wow_set_dmac_cfg return NON SUCCESS. ");
    }

    return OAL_SUCC;
}
#ifndef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
/*****************************************************************************
 函 数 名  : hmac_config_host_sleep_switch
 功能描述  : 配置sleep开关
*****************************************************************************/
osal_s32 hmac_config_host_sleep_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    osal_u32 is_host_sleep;
    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_host_sleep_switch::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    is_host_sleep = *(osal_u32 *)msg->data;

    if (is_host_sleep == 1) {
        ret = hmac_config_wow_activate_switch(hmac_vap, is_host_sleep);
        hmac_wow_host_sleep_cmd(hmac_vap, is_host_sleep);
    } else {
        hmac_wow_host_sleep_cmd(hmac_vap, is_host_sleep);
        ret = hmac_config_wow_activate_switch(hmac_vap, is_host_sleep);
    }
    
    return ret;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_set_wow
 功能描述  : 配置sleep开关
*****************************************************************************/
osal_s32 hmac_config_set_wow(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 wow_event;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_wow::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wow_event = *(osal_u32 *)msg->data;
    uapi_wlan_set_wow_event(wow_event);
    return OAL_SUCC;
}
#ifndef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
/*****************************************************************************
 函 数 名  : hmac_config_wow_activate_switch
 功能描述  : 配置sleep开关
*****************************************************************************/
osal_s32 hmac_config_wow_activate_switch(hmac_vap_stru *hmac_vap, osal_u32 wow_en)
{
    osal_u8 vap_idx;

    hmac_vap_stru *hmac_vap_tmp = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap_tmp == OSAL_NULL) {
            continue;
        }
        if ((hmac_vap_tmp->vap_mode == WLAN_VAP_MODE_BSS_AP) && (hmac_vap_tmp->vap_state == MAC_VAP_STATE_UP)) {
            oam_warning_log0(0, 0, "hmac_config_wow_activate_switch:: AP EXIST, don't support wowEn");
            return OAL_FAIL;
        }
    }

    hmac_wow_set_wow_en_cmd(wow_en);
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_set_wow_pattern
 功能描述  : 配置WOW pattern
*****************************************************************************/
osal_s32 hmac_config_set_wow_pattern(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_cfg_wow_pattern_param_stru *pattern = OSAL_NULL;
    osal_u32 ret;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_wow_pattern::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pattern = (hmac_cfg_wow_pattern_param_stru *)msg->data;
    if (pattern->pattern_option == MAC_WOW_PATTERN_PARAM_OPTION_ADD) {
        ret = uapi_wlan_add_netpattern((osal_u32)pattern->pattern_index,
                                       &pattern->pattern_value[0], pattern->pattern_len);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_CFG, "uapi_wlan_add_netpattern return NON SUCCESS.");
        }
    } else if (pattern->pattern_option == MAC_WOW_PATTERN_PARAM_OPTION_DEL) {
        uapi_wlan_del_netpattern((osal_u32)pattern->pattern_index);
    } else {
        hmac_wow_set_pattern_cmd(pattern);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
osal_s32 hmac_config_wow_param_sync(hmac_vap_stru *hmac_vap, frw_msg *msg_ori)
{
    wow_param_sync_stru wow_param_sync = { 0 };
    frw_msg msg = { 0 };
    osal_s32 ret;

    unref_param(msg_ori);

    hmac_arp_offload_get_param(&(wow_param_sync.arp_param));
    hmac_dhcp_offload_get_param(&(wow_param_sync.dhcp_param));
    hmac_rekey_offload_get_param(&(wow_param_sync.rekey_param));
    memcpy_s(&(wow_param_sync.wow_cfg), sizeof(mac_wow_cfg_stu), &g_wow_info.wow_cfg, sizeof(mac_wow_cfg_stu));

    msg.data = (osal_u8 *)&wow_param_sync;
    msg.data_len = sizeof(wow_param_sync_stru);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_WOW_PARAM_SYNC, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_wow_param_sync:send_msg_to_device failed[%d].}", ret);
    }
    return ret;
}

osal_s32 hmac_config_dynamic_wow_result_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    wow_result_to_host_stru *wow_result = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || msg==OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    wow_result = (wow_result_to_host_stru *)msg->data;
    g_wow_info.wakeup_reason = wow_result->wakeup_reason;
    memcpy_s(&g_wow_offload_info, sizeof(mac_wow_offload_info_stru), &wow_result->mac_wow_offload_info,
        sizeof(mac_wow_offload_info_stru));

    return OAL_SUCC;
}

osal_s32 hmac_config_clear_wow_offload_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_bool_enum_uint8 clear_wow_offload_info;

    unref_param(hmac_vap);
    clear_wow_offload_info = *(oal_bool_enum_uint8 *)(msg->data);
    wifi_printf("dmac_config_clear_wow_offload_info:[%d]\r\n", clear_wow_offload_info);
    if (clear_wow_offload_info == OSAL_TRUE) {
        /* 清空wow_offload_info */
        if (memset_s(&g_wow_offload_info, sizeof(mac_wow_offload_info_stru), 0,
            sizeof(mac_wow_offload_info_stru)) != EOK) {
            return OAL_FAIL;
        }
    } else {
        /* 打印wow_offload_info */
        oam_warning_log3(0, OAM_SF_CFG, "wow_rcv_magic = %d, wow_rcv_disasoc = %d,wow_rcv_deauth = %d",
            g_wow_offload_info.wow_rcv_magic, g_wow_offload_info.wow_rcv_disasoc,
            g_wow_offload_info.wow_rcv_deauth);
        oam_warning_log2(0, OAM_SF_CFG, "wow_rcv_net_pattern_tcp = %d, wow_rcv_net_pattern_udp = %d",
            g_wow_offload_info.wow_rcv_net_pattern_tcp, g_wow_offload_info.wow_rcv_net_pattern_udp);
        oam_warning_log2(0, OAM_SF_CFG, "arp_offload_rcv_arp_req = %d, arp_offload_rcv_ns = %d",
            g_wow_offload_info.arp_offload_rcv_arp_req, g_wow_offload_info.arp_offload_rcv_ns);
        oam_warning_log1(0, OAM_SF_CFG, "rekey_offload_rcv_eapol = %d",
            g_wow_offload_info.rekey_offload_rcv_eapol);
        oam_warning_log3(0, OAM_SF_CFG, "arp_offload_send_arp_rsp = %d, arp_offload_send_ns = %d,\
            arp_offload_send_free_arp = %d", g_wow_offload_info.arp_offload_send_arp_rsp,
            g_wow_offload_info.arp_offload_send_ns, g_wow_offload_info.arp_offload_send_free_arp);
        oam_warning_log1(0, OAM_SF_CFG, "dhcp_offload_send_dhcp_renew = %d",
            g_wow_offload_info.dhcp_offload_send_dhcp_renew);
        oam_warning_log1(0, OAM_SF_CFG, "rekey_offload_send_eapol = %d",
            g_wow_offload_info.rekey_offload_send_eapol);
        /* 打印后将wow_offload_info中唤醒原因重置为OAL_FALSE */
        g_wow_offload_info.wow_rcv_deauth = OAL_FALSE;
        g_wow_offload_info.wow_rcv_disasoc = OAL_FALSE;
        g_wow_offload_info.wow_rcv_magic = OAL_FALSE;
        g_wow_offload_info.wow_rcv_net_pattern_tcp = OAL_FALSE;
        g_wow_offload_info.wow_rcv_net_pattern_udp = OAL_FALSE;
    }
    return OAL_SUCC;
}

osal_s32 hmac_wow_send_retry(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 retry_time = 0;
    osal_s32 ret = 0;

    for (retry_time = 0; retry_time < WOW_SEND_RETRY; retry_time++) {
        ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_WOW_LOAD, msg, OSAL_TRUE);
        if (ret == OAL_SUCC) {
            return ret;
        }
    }

    return ret;
}

osal_s32 hmac_wow_proc_file(struct file **fp, osal_u32 *file_remain_len)
{
    osal_u32 file_len;

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
    if (firmware_sha256_checksum(CONFIG_FIRMWARE_WOW_PATH) != OAL_SUCC) {
        return OAL_FAIL;
    }
#endif

    /* 打开文件 */
    *fp = osal_klib_fopen(CONFIG_FIRMWARE_WOW_PATH, 0, 0);
    if (!(*fp)) {
        *fp = OSAL_NULL;
        return OAL_FAIL;
    }

    /* 获取file文件大小 */
    file_len = osal_klib_fseek(0, SEEK_END, *fp);
    if (file_len <= 0) {
        osal_klib_fclose(*fp);
        return OAL_FAIL;
    }

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
    /* skip bin's header area */
    *file_remain_len = file_len - SHA256_HEADER_LENGTH;
    osal_klib_fseek(SHA256_HEADER_LENGTH, SEEK_SET, *fp);
#else
    *file_remain_len = file_len;
    osal_klib_fseek(0, SEEK_SET, *fp);
#endif

    return OAL_SUCC;
}

osal_s32 hmac_wow_send(hmac_vap_stru *hmac_vap, osal_u8 *tx_buf)
{
    mac_wow_load_info_stru *load_info = (mac_wow_load_info_stru *)tx_buf;
    struct file *fp = OSAL_NULL;
    osal_u8 *wow_block = tx_buf + sizeof(mac_wow_load_info_stru);
    frw_msg msg = {0};
    osal_u32 file_remain_len;
    osal_s32 rdlen, ret;
    osal_u8 loop = 0;

    if (hmac_wow_proc_file(&fp, &file_remain_len) != OAL_SUCC) {
        return OAL_FAIL;
    }

    load_info->msg_type = MAC_WOW_NOTIFY_LOAD;

    /* 循环读取wowbin数据块 */
    for (loop = 0; loop < WOW_MAX_BLOCK_NUM; loop++) {
        rdlen = osal_klib_fread(wow_block, WOW_BLOCK_SIZE, fp);
        file_remain_len -= rdlen;
        if (rdlen != WOW_BLOCK_SIZE && file_remain_len != 0) {
            osal_klib_fclose(fp);
            return OAL_FAIL;
        }
        load_info->block_data_len = rdlen;
        load_info->block_id = loop;
        load_info->file_remain_len = file_remain_len;

        msg.data = tx_buf;
        msg.data_len = sizeof(mac_wow_load_info_stru) + rdlen;
        ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_WOW_LOAD, &msg, OSAL_TRUE);
        /* 失败 进行最多三次重传 */
        if (ret != OAL_SUCC && hmac_wow_send_retry(hmac_vap, &msg) != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_WOW, "hmac_wow_send::No.[%d] fail, file_remain_len %d", loop, file_remain_len);
            osal_klib_fclose(fp);
            return ret;
        }

        if (file_remain_len == 0) {
            oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_send::file load finish");
            break;
        }
    }

    osal_klib_fclose(fp);
    return OAL_SUCC;
}

osal_void hmac_wow_set_all_vap_state(osal_u8 is_pause)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u8 vap_idx = 0;

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            continue;
        }
        if (is_pause == OSAL_TRUE) {
            g_wow_all_vap_state[vap_idx] = hmac_vap->vap_state;
            hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_PAUSE);
        } else {
            hmac_vap_state_change_etc(hmac_vap, g_wow_all_vap_state[vap_idx]);
        }
    }
}

osal_s32 hmac_wow_autoload(hmac_vap_stru *hmac_vap)
{
    osal_u8 *tx_buf = OSAL_NULL;
    mac_wow_load_info_stru mem_rsv_info;
    hal_dev_fsm_stru *hal_dev_fsm = &(hal_chip_get_hal_device()->hal_dev_fsm);
    frw_msg msg;
    osal_s32 ret;

    memset_s(&mem_rsv_info, sizeof(mac_wow_load_info_stru), 0, sizeof(mac_wow_load_info_stru));
    memset_s(&msg, sizeof(frw_msg), 0, sizeof(frw_msg));

    /* 所有业务vap都要pause 否则共存场景可能申请不到内存 */
    hmac_wow_set_all_vap_state(OSAL_TRUE);
    /* IDLE */
    hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_IDLE_STATE, HAL_DEVICE_EVENT_WOW_PREPARE);

    /* Step1 抛消息至device 预留内存空间 */
    mem_rsv_info.msg_type = MAC_WOW_NOTIFY_MEM_RSV;
    msg.data = (osal_u8 *)&mem_rsv_info;
    msg.data_len = sizeof(mac_wow_load_info_stru);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_WOW_LOAD, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        /* WORK */
        hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_WORK_STATE, HAL_DEVICE_EVENT_WOW_PREPARE);
        hmac_wow_set_all_vap_state(OSAL_FALSE);
        oam_warning_log1(0, OAM_SF_WOW, "hmac_wow_autoload::msg_to_device send fail, ret [%d]", ret);
        return ret;
    }

    /* WORK */
    hal_device_fsm_trans_to_state(hal_dev_fsm, HAL_DEVICE_WORK_STATE, HAL_DEVICE_EVENT_WOW_PREPARE);
    hmac_wow_set_all_vap_state(OSAL_FALSE);
    /* Step2 加载wowbin至device */
    tx_buf = (osal_u8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_wow_load_info_stru) + WOW_BLOCK_SIZE,
        OAL_TRUE);
    if (tx_buf == OSAL_NULL) {
        /* device侧内存需不需要回收 */
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_autoload::alloc tx mem fail");
        return OAL_FAIL;
    }

    /* 调用接口 访问wowbin 发送数据 */
    ret = hmac_wow_send(hmac_vap, tx_buf);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_WOW, "hmac_wow_autoload::hmac_wow_send fail [%d]", ret);
    }

    oal_mem_free(tx_buf, OSAL_TRUE);
    return ret;
}

osal_s32 hmac_wow_unload(hmac_vap_stru *hmac_vap)
{
    mac_wow_load_info_stru unload_info;
    frw_msg msg;
    osal_s32 ret;

    memset_s(&unload_info, sizeof(mac_wow_load_info_stru), 0, sizeof(mac_wow_load_info_stru));
    memset_s(&msg, sizeof(frw_msg), 0, sizeof(frw_msg));

    unload_info.msg_type = MAC_WOW_NOTIFY_MEM_RESTORE;
    msg.data = (osal_u8 *)&unload_info;
    msg.data_len = sizeof(mac_wow_load_info_stru);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_WOW_LOAD, &msg, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_WOW, "hmac_wow_unload::msg_to_device send fail, ret [%d]", ret);
    }

    return ret;
}

osal_s32 hmac_config_wow_autoload(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 load = 0;
    osal_s32 ret = 0;

    if (hmac_vap == OSAL_NULL || msg == OSAL_NULL || msg->data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_config_wow_autoload::null ptr");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hwifi_get_wow_enable() != OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_config_wow_autoload::wow disable");
        return OAL_FAIL;
    }

    load = *(osal_u8 *)msg->data;
    if (load == OSAL_TRUE) {
        /* 加载wowbin */
        ret = hmac_wow_autoload(hmac_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_WOW, "hmac_config_wow_autoload::hmac_wow_autoload ret [%d]", ret);
            return ret;
        }
    } else if (load == OSAL_FALSE) {
        /* 卸载wowbin */
        ret = hmac_wow_unload(hmac_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_WOW, "hmac_config_wow_autoload::hmac_wow_unload ret [%d]", ret);
            return ret;
        }
    } else {
        oam_warning_log1(0, OAM_SF_WOW, "hmac_config_wow_autoload::wow load param [%d]", load);
    }

    return OAL_SUCC;
}
#else
osal_s32 hmac_config_clear_wow_offload_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    (osal_void)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_CLEAR_WOW_OFFLOAD_INFO, msg, OSAL_TRUE);
    return OAL_SUCC;
}
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_void hmac_wow_suspend_cb(osal_void)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_s32 ret = 0;

    if (hwifi_get_wow_enable() != OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_suspend_cb::wow disable");
        return;
    }

    hmac_vap = mac_find_up_legacy_sta_vap();
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_suspend_cb::cant find legacy vap");
        return;
    }
    ret = hmac_wow_autoload(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_WOW, "hmac_wow_suspend_cb::load wow ret [%d]", ret);
    }
    return;
}

osal_void hmac_wow_resume_cb(osal_void)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_s32 ret = 0;

    if (hwifi_get_wow_enable() != OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_resume_cb::wow disable");
        return;
    }

    hmac_vap = mac_find_up_legacy_sta_vap();
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_WOW, "hmac_wow_resume_cb::cant find legacy vap");
        return;
    }
    ret = hmac_wow_unload(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_WOW, "hmac_wow_resume_cb::load wow ret [%d]", ret);
    }
    return;
}
#endif
osal_s32 hmac_config_wow_show_wakeup_reason(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    unref_param(hmac_vap);
    unref_param(msg);
    oam_warning_log1(0, OAM_SF_WOW, "wow wakeup reason = %d}", g_wow_info.wakeup_reason);
#else
    (osal_void)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_WOW_SHOW_WAKEUP_REASON, msg, OSAL_TRUE);
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_wow_init
 功能描述  : WOW初始化
 修改历史      :
  1.日    期   : 2021年4月15日
    修改内容   : 新生成函数
*****************************************************************************/
osal_u32 hmac_wow_init(osal_void)
{
    /* 全局变量初始化配置 */
    if (memset_s(&g_wow_info, sizeof(hmac_wow_info_stru), 0, sizeof(hmac_wow_info_stru)) != EOK) {
        return OAL_FAIL;
    }

    hmac_wow_set_host_state(HMAC_HOST_STATE_WAKEUP);

#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    g_wow_info.wow_cfg.wow_en = MAC_WOW_ENABLE;
    g_wow_info.wow_cfg.wow_event = hwifi_get_wow_event();
#else
    g_wow_info.wow_cfg.wow_en = MAC_WOW_DISABLE;
    g_wow_info.wow_cfg.wow_event = MAC_WOW_FIELD_MAGIC_PACKET |
                                   MAC_WOW_FIELD_NETPATTERN_TCP |
                                   MAC_WOW_FIELD_NETPATTERN_UDP |
                                   MAC_WOW_FIELD_DISASSOC;
#endif

    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_WOW, hmac_config_set_wow);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_WOW_PATTERN, hmac_config_set_wow_pattern);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_WOW_PARAM, hmac_config_wow_set_param);
    frw_msg_hook_register(WLAN_MSG_W2H_C_CFG_WOW_SHOW_WAKEUP_REASON, hmac_config_wow_show_wakeup_reason);
    frw_msg_hook_register(WLAN_MSG_W2H_C_CFG_CLEAR_WOW_OFFLOAD_INFO, hmac_config_clear_wow_offload_info);
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    frw_msg_hook_register(WLAN_MSG_D2H_DYNAMIC_WOW_RESULT_INFO, hmac_config_dynamic_wow_result_info);
    frw_msg_hook_register(WLAN_MSG_D2H_WOW_OFFLOAD_PARAM_SYNC, hmac_config_wow_param_sync);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_WOW_LOAD, hmac_config_wow_autoload);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 注册平台suspend/resume回调 */
    pm_wifi_wkup_cb_host_register(hmac_wow_resume_cb);
#endif
#else
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_HOST_SLEEP_EN, hmac_config_host_sleep_switch);
#endif

    return OAL_SUCC;
}

osal_void hmac_wow_deinit(osal_void)
{
    memset_s(&g_wow_info, sizeof(hmac_wow_info_stru), 0, sizeof(hmac_wow_info_stru));

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 去注册平台suspend/resume回调 */
    pm_wifi_wkup_cb_host_register(NULL);
#endif

    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_WOW);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_WOW_PATTERN);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_WOW_PARAM);
    frw_msg_hook_unregister(WLAN_MSG_W2H_C_CFG_WOW_SHOW_WAKEUP_REASON);
    frw_msg_hook_unregister(WLAN_MSG_W2H_C_CFG_CLEAR_WOW_OFFLOAD_INFO);
#ifdef _PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
    frw_msg_hook_unregister(WLAN_MSG_D2H_DYNAMIC_WOW_RESULT_INFO);
    frw_msg_hook_unregister(WLAN_MSG_D2H_WOW_OFFLOAD_PARAM_SYNC);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_WOW_LOAD);
#else
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_HOST_SLEEP_EN);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // end of _PRE_WLAN_FEATURE_WOW_OFFLOAD
