/*
 * Copyright (c) CompanyNameMagicTag. 2015-2024. All rights reserved.
 * 文 件 名   : hmac_roam_connect.c
 * 生成日期   : 2015年3月18日
 * 功能描述   : 漫游connect流程实现
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_roam_connect.h"
#include "oam_ext_if.h"
#include "mac_ie.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_encap_frame_sta.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"
#include "hmac_chan_mgmt.h"
#include "hmac_11i.h"
#include "hmac_user.h"
#include "hmac_roam_main.h"
#include "wlan_msg.h"
#include "hmac_feature_dft.h"
#include "hmac_mbo.h"
#include "hmac_feature_interface.h"
#include "hmac_11r.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ROAM_CONNECT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_roam_fsm_func g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_BUTT][ROAM_CONNECT_FSM_EVENT_TYPE_BUTT];

OAL_STATIC osal_u32 hmac_roam_connect_null_fn(hmac_roam_info_stru *roam_info, osal_void *param);
OAL_STATIC osal_u32 hmac_roam_start_join(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_send_auth_seq1(hmac_roam_info_stru *roam_info);
OAL_STATIC osal_u32 hmac_roam_process_auth_seq2(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_process_assoc_rsp(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_process_action(hmac_roam_info_stru *roam_info, osal_void *p_param);
osal_u32 hmac_roam_connect_succ(hmac_roam_info_stru *roam_info, osal_void *param);
OAL_STATIC osal_u32 hmac_roam_auth_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_assoc_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_handshaking_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param);
osal_u32 hmac_roam_send_reassoc_req(hmac_roam_info_stru *roam_info);
OAL_STATIC osal_u32 hmac_roam_connect_timeout_etc(osal_void *p_arg);

/*****************************************************************************
  3 函数实现
*****************************************************************************/

osal_void hmac_roam_connect_fsm_deinit_etc(osal_void)
{
    osal_u32  state;
    osal_u32  event;

    for (state = 0; state < ROAM_CONNECT_STATE_BUTT; state++) {
        for (event = 0; event < ROAM_CONNECT_FSM_EVENT_TYPE_BUTT; event++) {
            g_hmac_roam_connect_fsm_func[state][event] = hmac_roam_connect_null_fn;
        }
    }
}
/*****************************************************************************
 函 数 名  : hmac_roam_connect_fsm_init_etc
 功能描述  : roam connect模块状态-事件-行为表
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void hmac_roam_connect_fsm_init_etc(osal_void)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_ROAM_CONNECT_FSM_INIT);

    hmac_roam_connect_fsm_deinit_etc();
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_INIT][ROAM_CONNECT_FSM_EVENT_START]                =
        hmac_roam_start_join;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_AUTH_COMP][ROAM_CONNECT_FSM_EVENT_MGMT_RX]    =
        hmac_roam_process_auth_seq2;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_AUTH_COMP][ROAM_CONNECT_FSM_EVENT_TIMEOUT]    =
        hmac_roam_auth_timeout;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_ASSOC_COMP][ROAM_CONNECT_FSM_EVENT_MGMT_RX]   =
        hmac_roam_process_assoc_rsp;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_WAIT_ASSOC_COMP][ROAM_CONNECT_FSM_EVENT_TIMEOUT]   =
        hmac_roam_assoc_timeout;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_HANDSHAKING][ROAM_CONNECT_FSM_EVENT_MGMT_RX]       =
        hmac_roam_process_action;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_HANDSHAKING][ROAM_CONNECT_FSM_EVENT_KEY_DONE]      =
        hmac_roam_connect_succ;
    g_hmac_roam_connect_fsm_func[ROAM_CONNECT_STATE_HANDSHAKING][ROAM_CONNECT_FSM_EVENT_TIMEOUT]       =
        hmac_roam_handshaking_timeout;

    if (fhook != OSAL_NULL) {
        ((hmac_11r_roam_connect_fsm_init_cb)fhook)(g_hmac_roam_connect_fsm_func);
    }
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_fsm_action_etc
 功能描述  : 调用漫游connect状态机函数表
 输入参数  : hmac_vap: hmac vap
             event: 事件类型
             p_param: 输入参数
*****************************************************************************/
osal_u32 hmac_roam_connect_fsm_action_etc(hmac_roam_info_stru *roam_info,
    roam_connect_fsm_event_type_enum event, osal_void *p_param)
{
    if (roam_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->connect.state >= ROAM_CONNECT_STATE_BUTT) {
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    if (event >= ROAM_CONNECT_FSM_EVENT_TYPE_BUTT) {
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    wifi_printf("hmac_roam_connect_fsm_action_etc::roam_conn_state[%d] event[%d]\r\n",
        roam_info->connect.state, event);
    return g_hmac_roam_connect_fsm_func[roam_info->connect.state][event](roam_info, p_param);
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_change_state
 功能描述  : 改变状态机状态
 输入参数  : hmac_vap      : HMAC VAP
             roam_main_state: 要切换到的状态
 输出参数  : 无
*****************************************************************************/
osal_void hmac_roam_connect_change_state(hmac_roam_info_stru *roam_info,
    roam_connect_state_enum_uint8 state)
{
    if (roam_info) {
        oam_warning_log2(0, OAM_SF_ROAM,
            "{hmac_roam_connect_change_state::[%d]->[%d]}", roam_info->connect.state, state);
        roam_info->connect.state = state;
    }
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_check_state
 功能描述  : 参数检查接口
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_connect_check_state(hmac_roam_info_stru *roam_info,
    mac_vap_state_enum_uint8     vap_state,
    roam_main_state_enum_uint8 main_state,
    roam_connect_state_enum_uint8 connect_state)
{
    if (roam_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (roam_info->hmac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    if (roam_info->enable == 0) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if (roam_info->hmac_vap->wpa3_roaming == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ROAM, "hmac_roam_connect_check_state::wpa3 roaming skip state check");
        return OAL_SUCC;
    }

    if ((roam_info->hmac_vap->vap_state != vap_state) ||
        (roam_info->main_state != main_state) ||
        (roam_info->connect.state != connect_state)) {
        oam_warning_log3(0, OAM_SF_ROAM,
            "{hmac_roam_connect_check_state::unexpect vap_state[%d] main_state[%d] connect_state[%d]!}",
            roam_info->hmac_vap->vap_state, roam_info->main_state,
            roam_info->connect.state);
        return OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS;
    }

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_roam_connect_timeout_etc
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_connect_timeout_etc(osal_void *p_arg)
{
    hmac_roam_info_stru *roam_info;

    roam_info = (hmac_roam_info_stru *)p_arg;
    if (roam_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_timeout_etc::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_connect_timeout_etc::MAIN_STATE[%d] CONNECT_STATE[%d].}",
        roam_info->main_state, roam_info->connect.state);

    return hmac_roam_connect_fsm_action_etc(roam_info, ROAM_CONNECT_FSM_EVENT_TIMEOUT, OAL_PTR_NULL);
}
/*****************************************************************************
 函 数 名  : hmac_roam_connect_null_fn
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_connect_null_fn(hmac_roam_info_stru *roam_info, osal_void *param)
{
    unref_param(roam_info);
    unref_param(param);

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_start_timer
 功能描述  : 如果定时器已存在，重启，否则创建定时器
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_void hmac_roam_connect_start_timer(hmac_roam_info_stru *roam_info, osal_u32 timeout)
{
    frw_timeout_stru *timer = &(roam_info->connect.timer);

    oam_info_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_start_timer [%d].}", timeout);

    /* 启动认证超时定时器 */
    frw_create_timer_entry(timer,
                           hmac_roam_connect_timeout_etc,
                           timeout,
                           roam_info,
                           OAL_FALSE);
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_del_timer
 功能描述  : 立即删除roam connect定时器
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_connect_del_timer(hmac_roam_info_stru *roam_info)
{
    frw_destroy_timer_entry(&(roam_info->connect.timer));
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_set_join_reg_etc
 功能描述  : 根据bss更新mib信息
 输入参数  : hmac_vap_stru      *hmac_vap,
             mac_bss_dscr_stru  *bss_dscr
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_roam_connect_set_join_reg_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_ctx_join_req_set_reg_stru    reg_params;
    hmac_join_req_stru                join_req;

    memset_s(&join_req, OAL_SIZEOF(hmac_join_req_stru), 0, OAL_SIZEOF(hmac_join_req_stru));

    /* 设置需要写入寄存器的BSSID信息 */
    oal_set_mac_addr(reg_params.bssid, hmac_vap->bssid);

    /* 填写信道相关信息 */
    reg_params.current_channel.chan_number    = hmac_vap->channel.chan_number;
    reg_params.current_channel.band           = hmac_vap->channel.band;
    reg_params.current_channel.en_bandwidth   = hmac_vap->channel.en_bandwidth;
    reg_params.current_channel.chan_idx       = hmac_vap->channel.chan_idx;

    /* 以old user信息塑造虚假的入网结构体，调用函数 */
    /* 填写速率相关信息 */
    join_req.bss_dscr.num_supp_rates = hmac_user->op_rates.rs_nrates;
    (osal_void)memcpy_s(join_req.bss_dscr.supp_rates, OAL_SIZEOF(osal_u8) * WLAN_MAX_SUPP_RATES,
        hmac_user->op_rates.rs_rates, OAL_SIZEOF(osal_u8) * WLAN_MAX_SUPP_RATES);
    join_req.bss_dscr.ht_capable    = hmac_user->ht_hdl.ht_capable;
    join_req.bss_dscr.vht_capable   = hmac_user->vht_hdl.vht_capable;
    hmac_sta_get_min_rate(&reg_params.min_rate, &join_req);

    /* 设置dtim period信息 */
    reg_params.beacon_period = (osal_u16)mac_mib_get_beacon_period(hmac_vap);

    /* 同步FortyMHzOperationImplemented */
    reg_params.dot11_40mhz_operation_implemented   = mac_mib_get_forty_mhz_operation_implemented(hmac_vap);

    /* 设置beacon filter关闭 */
    reg_params.beacon_filter = OAL_FALSE;

    /* 设置no frame filter打开 */
    reg_params.non_frame_filter = OAL_TRUE;

    hmac_join_set_reg_event_process(hmac_vap, &reg_params);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_roam_connect_msg_to_wpas(const hmac_vap_stru *hmac_vap,
    const hmac_roam_info_stru *roam_info, hmac_roam_rsp_stru *roam_rsp, const hmac_asoc_rsp_stru *asoc_rsp)
{
    frw_msg msg_info = {0};

    if (osal_memcmp(roam_rsp->bssid, roam_info->old_bss.bssid, WLAN_MAC_ADDR_LEN) == 0) {
        /* Reassociation to the same BSSID: report NL80211_CMD_CONNECT event to supplicant instead of NL80211_CMD_ROAM
         * event in case supplicant ignore roam event to the same bssid which will cause 4-way handshake failure */
        /* wpa_supplicant: wlan0: WPA: EAPOL-Key Replay Counter did not increase - dropping packet */
        oam_warning_log4(0, OAM_SF_ROAM,
            "{hmac_roam_connect_notify_wpas::roam to the same ap %02X:%02X:%02X:%02X:XX:XX}",
            /* 0:1:2:3:数组下标 */
            roam_rsp->bssid[0], roam_rsp->bssid[1], roam_rsp->bssid[2], roam_rsp->bssid[3]);

        msg_info.data = (osal_u8 *)asoc_rsp;
        msg_info.data_len = OAL_SIZEOF(hmac_asoc_rsp_stru);

        return frw_asyn_host_post_msg(WLAN_MSG_H2W_ASOC_COMP_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    } else {
        msg_info.data = (osal_u8 *)roam_rsp;
        msg_info.data_len = OAL_SIZEOF(hmac_roam_rsp_stru);

        return frw_asyn_host_post_msg(WLAN_MSG_H2W_ROAM_COMP_STA, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    }
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_notify_wpas
 功能描述  : 将关联成功消息通知wpa_supplicant，以便开始4-way握手流程
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_connect_notify_wpas(hmac_roam_info_stru *roam_info, osal_u8 *mac_hdr,
    osal_u16 msg_len)
{
    hmac_asoc_rsp_stru               asoc_rsp;
    hmac_roam_rsp_stru               roam_rsp;
    hmac_vap_stru                   *hmac_vap;
    hmac_user_stru                  *hmac_user;
    osal_u8                       *mgmt_data;

    memset_s(&asoc_rsp, OAL_SIZEOF(hmac_asoc_rsp_stru), 0, OAL_SIZEOF(hmac_asoc_rsp_stru));
    memset_s(&roam_rsp, OAL_SIZEOF(hmac_roam_rsp_stru), 0, OAL_SIZEOF(hmac_roam_rsp_stru));
    /* 获取AP的mac地址 */
    mac_get_address3(mac_hdr, roam_rsp.bssid);
    mac_get_address3(mac_hdr, asoc_rsp.addr_ap);

    hmac_vap  = roam_info->hmac_vap;
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_notify_wpas, hmac_user[%d] = NULL!}",
            hmac_vap->assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取关联请求帧信息 */
    roam_rsp.asoc_req_ie_buff = hmac_user->assoc_req_ie_buff;
    roam_rsp.asoc_req_ie_len   = hmac_user->assoc_req_ie_len;
    asoc_rsp.asoc_req_ie_buff = hmac_user->assoc_req_ie_buff;
    asoc_rsp.asoc_req_ie_len   = hmac_user->assoc_req_ie_len;

    /* 记录关联响应帧的部分内容，用于上报给内核 */
    /* asoc_rsp 帧拷贝一份上报上层,防止帧内容上报wal侧处理后被hmac侧释放 */
    if (msg_len < OAL_ASSOC_RSP_IE_OFFSET) {
        oam_error_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_handle_asoc_rsp_sta_etc::msg_len is too short, %d.}",
            hmac_vap->vap_id, msg_len);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    mgmt_data = (osal_u8 *)oal_memalloc(msg_len - OAL_ASSOC_RSP_IE_OFFSET);
    if (mgmt_data == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_handle_asoc_rsp_sta_etc::mgmt_data alloc null,size %d.}",
            hmac_vap->vap_id, (msg_len - OAL_ASSOC_RSP_IE_OFFSET));
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    roam_rsp.asoc_rsp_ie_len   = msg_len - OAL_ASSOC_RSP_IE_OFFSET;
    (osal_void)memcpy_s(mgmt_data, roam_rsp.asoc_rsp_ie_len,
        (osal_u8 *)(mac_hdr + OAL_ASSOC_RSP_IE_OFFSET), roam_rsp.asoc_rsp_ie_len);
    roam_rsp.asoc_rsp_ie_buff = mgmt_data;
    asoc_rsp.asoc_rsp_ie_len = roam_rsp.asoc_rsp_ie_len;
    asoc_rsp.asoc_rsp_ie_buff = roam_rsp.asoc_rsp_ie_buff;

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    roam_rsp.st_channel.chan_number = roam_info->connect.bss_dscr->st_channel.chan_number;
#endif

    if (hmac_roam_connect_msg_to_wpas(hmac_vap, roam_info, &roam_rsp, &asoc_rsp) != OAL_SUCC) {
        oal_free(mgmt_data);
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_prepare_join_info(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    hmac_join_req_stru      join_req;
    oal_app_ie_stru         app_ie;
    osal_u8              *pmkid = OSAL_NULL;
    osal_u8               ie_len = 0;
    osal_u32              ret;

    /* 配置join参数 */
    hmac_prepare_join_req_etc(&join_req, bss_dscr);

    ret = hmac_sta_update_join_req_params_etc(hmac_vap, &join_req);
    if ((ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_start_join::hmac_sta_update_join_req_params_etc fail[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    if (hmac_vap->cap_flag.wpa == OSAL_TRUE) {
        /* 设置 WPA Capability IE */
        hmac_set_wpa_ie_etc((osal_void *)hmac_vap, app_ie.ie, &ie_len);
    }

    if (hmac_vap->cap_flag.wpa2 == OSAL_TRUE || hmac_vap->cap_flag.wpa3 == OSAL_TRUE) {
        /* 设置 RSN Capability IE */
        if (hmac_vap->cap_flag.wpa3 != OSAL_TRUE) {
            pmkid = hmac_vap_get_pmksa_etc(hmac_vap, bss_dscr->bssid);
        }
        hmac_set_rsn_ie_etc((osal_void *)hmac_vap, pmkid, app_ie.ie, &ie_len);
    }

    hmac_vap->wpa3_roaming = (hmac_vap->cap_flag.wpa3 == OSAL_TRUE) ? OSAL_TRUE : OSAL_FALSE;
    if (ie_len != 0) {
        app_ie.app_ie_type = (hmac_vap->wpa3_roaming == OSAL_TRUE) ? OAL_APP_ASSOC_REQ_IE :
            OAL_APP_REASSOC_REQ_IE;
        app_ie.ie_len      = ie_len;
        ret = hmac_config_set_app_ie_to_vap_etc(hmac_vap, &app_ie, app_ie.app_ie_type);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ROAM,
                "vap_id[%d] {hmac_roam_start_join::hmac_config_set_app_ie_to_vap_etc fail, ret=[%d].}",
                hmac_vap->vap_id, ret);
        }
    } else {
        hmac_vap_clear_app_ie_etc(hmac_vap, OAL_APP_REASSOC_REQ_IE);
        hmac_vap_clear_app_ie_etc(hmac_vap, OAL_APP_ASSOC_REQ_IE);
    }

    ret = (osal_u32)hmac_join_set_dtim_reg_event_process(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_start_join::hmac_roam_connect_set_dtim_param_etc fail, ret=[%d].}",
            hmac_vap->vap_id, ret);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_complete_etc
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_void hmac_roam_connect_complete_etc(hmac_vap_stru *hmac_vap, osal_u32 result)
{
    hmac_roam_info_stru     *roam_info;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_complete_etc::vap null!}");
        return;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_connect_complete_etc::roam_info null!}",
                       hmac_vap->vap_id);
        return;
    }

    /* 漫游开关没有开时，不处理扫描结果 */
    if (roam_info->enable == 0) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_connect_complete_etc::roam disabled!}",
                       hmac_vap->vap_id);
        return;
    }

    if (result == OAL_SUCC) {
        hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_CONNECT_SUCC, OAL_PTR_NULL);
    } else if (result == OAL_ERR_CODE_ROAM_HANDSHAKE_FAIL) {
        hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_HANDSHAKE_FAIL, OAL_PTR_NULL);
    } else if (result == OAL_ERR_CODE_ROAM_NO_RESPONSE) {
        hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_CONNECT_FAIL, OAL_PTR_NULL);
    } else {
        hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, OAL_PTR_NULL);
    }
}

/*****************************************************************************
 函 数 名  : hmac_roam_start_join
 功能描述  : 启动定时器用来接收tbtt中断
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_start_join(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32              ret;
    hmac_vap_stru          *hmac_vap;
    mac_bss_dscr_stru      *bss_dscr;
#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_user_stru *hmac_user = roam_info->hmac_user;
#endif
    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_INIT);
    if ((ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_start_join::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->hmac_vap;

    bss_dscr = (mac_bss_dscr_stru *)p_param;

    (osal_void)memcpy_s(hmac_vap->supp_rates, bss_dscr->num_supp_rates,
        bss_dscr->supp_rates, bss_dscr->num_supp_rates);
    mac_mib_set_SupportRateSetNums(hmac_vap, bss_dscr->num_supp_rates);

    ret = hmac_roam_prepare_join_info(hmac_vap, bss_dscr);
    if ((ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_start_join::prepare_join_info fail[%d]!}", ret);
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_WPA3
    if (hmac_vap->wpa3_roaming == OSAL_TRUE) {
        hmac_vap->reassoc_flag = OSAL_FALSE;
        mac_mib_set_authentication_mode(hmac_vap, WLAN_WITP_AUTH_SAE);
        oal_set_mac_addr(hmac_user->user_mac_addr, roam_info->connect.bss_dscr->bssid);
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2);
        ret = hmac_report_ext_auth_event(hmac_vap);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_start_join::start WPA3 auth fail[%d]!}", ret);
            return ret;
        }
        return OAL_SUCC;
    }
#endif

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_JOIN);

    ret = hmac_roam_send_auth_seq1(roam_info);
    if ((ret != OAL_SUCC)) {
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_FAIL);

        /* 通知ROAM主状态机 */
        hmac_roam_connect_complete_etc(hmac_vap, OAL_FAIL);
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_roam_process_beacon::hmac_roam_send_auth_seq1 fail[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_encap_auth_proc(hmac_roam_info_stru *roam_info, oal_netbuf_stru **auth_frame,
    osal_u16 *auth_len)
{
    mac_tx_ctl_stru        *tx_ctl;
    hmac_user_stru         *hmac_user = roam_info->hmac_user;
    osal_void *fhook = OSAL_NULL;

    *auth_frame = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (*auth_frame == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_encap_auth_proc::OAL_MEM_NETBUF_ALLOC fail.}",
            roam_info->hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    memset_s(oal_netbuf_cb(*auth_frame), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    memset_s((osal_u8 *)oal_netbuf_header(*auth_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    /* 更新用户mac */
    oal_set_mac_addr(hmac_user->user_mac_addr, roam_info->connect.bss_dscr->bssid);

    *auth_len = hmac_mgmt_encap_auth_req_etc(roam_info->hmac_vap, (osal_u8 *)(OAL_NETBUF_HEADER(*auth_frame)));
    if (*auth_len < OAL_AUTH_IE_OFFSET) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_encap_auth_proc::encap auth req failed.}",
            roam_info->hmac_vap->vap_id);
        oal_netbuf_free(*auth_frame);
        return OAL_ERR_CODE_ROAM_FRAMER_LEN;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_ATTACH_MBO_IE_AUTH);
    if (fhook != OSAL_NULL) {
        ((hmac_roam_attach_mbo_ie_auth_cb)fhook)(roam_info->hmac_vap, (osal_u8 *)(oal_netbuf_header(*auth_frame)),
            auth_len, roam_info->connect.bss_dscr);
    }

    oal_netbuf_put(*auth_frame, *auth_len);

    /* 为填写发送描述符准备参数 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(*auth_frame);
    mac_get_cb_mpdu_len(tx_ctl) = *auth_len;
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_netbuf_num(tx_ctl) = 1;

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_send_auth_seq1
 功能描述  : 发送auth_seq1，并且启动auth超时定时器
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_send_auth_seq1(hmac_roam_info_stru *roam_info)
{
    osal_u32              ret;
    hmac_vap_stru          *hmac_vap  = roam_info->hmac_vap;
    oal_netbuf_stru        *auth_frame;
    osal_u16              auth_len;

    ret = hmac_roam_encap_auth_proc(roam_info, &auth_frame, &auth_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, auth_frame, auth_len);
    if ((ret != OAL_SUCC)) {
        oal_netbuf_free(auth_frame);
        oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_send_auth_seq1::send event failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_AUTH_COMP);

    /* 启动认证超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_AUTH_TIME_MAX);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_encap_reassoc_req(oal_netbuf_stru **assoc_req_frame, osal_u32 *assoc_len,
    hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru          *hmac_vap  = roam_info->hmac_vap;
    hmac_user_stru         *hmac_user = roam_info->hmac_user;
    mac_tx_ctl_stru        *tx_ctl = OSAL_NULL;
    osal_u32                ret;

    *assoc_req_frame = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (*assoc_req_frame == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_send_reassoc_req::paras error.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(oal_netbuf_cb(*assoc_req_frame), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());

    /* 将mac header清零 */
    memset_s((osal_u8 *)oal_netbuf_header(*assoc_req_frame), MAC_80211_FRAME_LEN, 0, MAC_80211_FRAME_LEN);

    hmac_vap->reassoc_flag = OAL_TRUE;

    *assoc_len = hmac_mgmt_encap_asoc_req_sta_etc(hmac_vap,
        (osal_u8 *)(OAL_NETBUF_HEADER(*assoc_req_frame)), roam_info->old_bss.bssid, hmac_vap->bssid);

    oal_netbuf_put(*assoc_req_frame, *assoc_len);

    /* 帧长异常 */
    if (*assoc_len <= OAL_ASSOC_REQ_IE_OFFSET) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_send_reassoc_req::unexpected assoc len[%d].}",
            hmac_vap->vap_id, *assoc_len);
        oal_netbuf_free(*assoc_req_frame);
        return OAL_FAIL;
    }
    hmac_user_free_asoc_req_ie(hmac_user);

    /* 记录关联请求帧的部分内容，用于上报给内核 */
    ret = hmac_user_set_asoc_req_ie(hmac_user, OAL_NETBUF_HEADER(*assoc_req_frame) + OAL_ASSOC_REQ_IE_OFFSET,
        *assoc_len - OAL_ASSOC_REQ_IE_OFFSET, OAL_TRUE);
    if (ret != OAL_SUCC) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, *assoc_req_frame);
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_send_reassoc_req::set ie fail}", hmac_vap->vap_id);
        oal_netbuf_free(*assoc_req_frame);
        return OAL_FAIL;
    }

    /* 为填写发送描述符准备参数 */
    tx_ctl                   = (mac_tx_ctl_stru *)oal_netbuf_cb(*assoc_req_frame);
    mac_get_cb_mpdu_len(tx_ctl)      = (osal_u16)*assoc_len;
    mac_get_cb_tx_user_idx(tx_ctl)   = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_netbuf_num(tx_ctl)    = 1;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_send_assoc_req
 功能描述  : 发送assoc req，并且启动assoc超时定时器
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_send_reassoc_req(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru          *hmac_vap  = roam_info->hmac_vap;
    hmac_user_stru         *hmac_user = roam_info->hmac_user;
    oal_netbuf_stru        *assoc_req_frame;
    osal_u32                assoc_len;
    osal_u32                ret;

    ret = hmac_roam_encap_reassoc_req(&assoc_req_frame, &assoc_len, roam_info);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, assoc_req_frame, (osal_u16)assoc_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(assoc_req_frame);
        hmac_user_free_asoc_req_ie(hmac_user);
        oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_send_reassoc_req::tx_mgmt_send_event failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    /* 启动关联超时定时器 */
    hmac_roam_connect_start_timer(roam_info, ROAM_ASSOC_TIME_MAX);

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_process_auth_seq2
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_process_auth_seq2(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32                   ret;
    hmac_vap_stru              *hmac_vap = OSAL_NULL;
    hmac_user_stru             *hmac_user = OSAL_NULL;
    mac_rx_ctl_stru            *pst_rx_ctrl = OSAL_NULL;
    osal_u8                    *mac_hdr = OSAL_NULL;
    osal_u8                    bssid[WLAN_MAC_ADDR_LEN]   = {0};
    osal_u16                   auth_status;
    osal_u8                    frame_sub_type;
    osal_u16                   auth_seq_num;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_AUTH_SEQ2_PROCESS);

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_WAIT_AUTH_COMP);
    if ((ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_auth_seq2::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap  = roam_info->hmac_vap;
    hmac_user = roam_info->hmac_user;

    pst_rx_ctrl    = (mac_rx_ctl_stru *)oal_netbuf_cb((oal_netbuf_stru *)p_param);
    mac_hdr    = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);

    mac_get_address3(mac_hdr, (osal_u8 *)bssid);
    if (oal_compare_mac_addr(hmac_user->user_mac_addr, (const osal_u8 *)bssid) != 0) {
        return OAL_SUCC;
    }

    frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    auth_seq_num = mac_get_auth_seq_num(mac_hdr, mac_get_rx_cb_payload_len(pst_rx_ctrl));
    auth_status = mac_get_auth_status(mac_hdr);
    /* auth_seq2帧校验，错误帧不处理，在超时中统一处理 */
    if ((frame_sub_type != (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT)) ||
        (auth_seq_num != WLAN_AUTH_TRASACTION_NUM_TWO) || (auth_status != MAC_SUCCESSFUL_STATUSCODE)) {
        return OAL_SUCC;
    }

    if (fhook != OSAL_NULL) {
        ret = ((hmac_roam_auth_seq2_11r_process_cb)fhook)(hmac_vap, roam_info, pst_rx_ctrl, mac_hdr);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }

    if (mac_get_auth_alg(mac_hdr) != WLAN_WITP_AUTH_OPEN_SYSTEM) {
        return OAL_SUCC;
    }

    /* 发送关联请求 */
    ret = hmac_roam_send_reassoc_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_process_auth_seq2::roam_send_assoc_req failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_process_assoc_rsp_check_para(hmac_roam_info_stru *roam_info,
    osal_void *p_param, mac_rx_ctl_stru **rx_ctrl)
{
    osal_u32                       ret;
    hmac_vap_stru                   *hmac_vap = OAL_PTR_NULL;
    hmac_user_stru                  *hmac_user = OAL_PTR_NULL;
    oal_netbuf_stru                 *net_buffer = OAL_PTR_NULL;
    mac_rx_ctl_stru                 *pst_rx_ctrl = OAL_PTR_NULL;
    osal_u8                       *mac_hdr = OAL_PTR_NULL;
    osal_u8                       *payload = OAL_PTR_NULL;
    mac_status_code_enum_uint16      asoc_status;
    osal_u8                        bss_addr[WLAN_MAC_ADDR_LEN];
    osal_u8                        frame_sub_type;

    hmac_vap  = roam_info->hmac_vap;
    hmac_user = roam_info->hmac_user;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_assoc_rsp_check_para::check_state fail[%d]!}", ret);
        return ret;
    }

    net_buffer  = (oal_netbuf_stru *)p_param;
    pst_rx_ctrl    = (mac_rx_ctl_stru *)oal_netbuf_cb(net_buffer);
    mac_hdr    = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);
    payload    = mac_hdr + pst_rx_ctrl->mac_header_len;

    *rx_ctrl = pst_rx_ctrl; /* 把解析出来的pst_rx_ctrl传出去 */

    /* mac地址校验 */
    mac_get_address3(mac_hdr, (osal_u8 *)bss_addr);
    if (oal_compare_mac_addr(hmac_user->user_mac_addr, (const osal_u8 *)bss_addr) != 0) {
        return OAL_SUCC;
    }

    /* assoc帧校验，错误帧处理 */
    frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    asoc_status = mac_get_asoc_status(payload);

    if ((frame_sub_type != (WLAN_FC0_SUBTYPE_REASSOC_RSP | WLAN_FC0_TYPE_MGT)) &&
        (frame_sub_type != (WLAN_FC0_SUBTYPE_ASSOC_RSP | WLAN_FC0_TYPE_MGT))) {
        return OAL_SUCC;
    }

    /* 关联响应帧长度校验 */
    if (pst_rx_ctrl->frame_len <= OAL_ASSOC_RSP_IE_OFFSET) {
        oam_warning_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_process_assoc_rsp_check_para::rsp ie length error, len[%d].}",
            hmac_vap->vap_id, pst_rx_ctrl->frame_len);
        return OAL_ERR_CODE_ROAM_FRAMER_LEN;
    }

    roam_info->connect.status_code = MAC_SUCCESSFUL_STATUSCODE;

    if (asoc_status != MAC_SUCCESSFUL_STATUSCODE) {
        roam_info->connect.status_code = asoc_status;
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_process_assoc_rsp
 功能描述  : 根据关联响应中的ie，更新本地用户能力及vap能力
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_process_assoc_rsp(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32        ret;
    hmac_vap_stru    *hmac_vap;
    hmac_user_stru   *hmac_user;
    mac_rx_ctl_stru  *pst_rx_ctrl = OAL_PTR_NULL;
    osal_u8        *mac_hdr, *payload;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_PROCESS_ASSOC_RSP);

    ret = hmac_roam_process_assoc_rsp_check_para(roam_info, p_param, &pst_rx_ctrl);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    hmac_vap  = roam_info->hmac_vap;
    hmac_user = roam_info->hmac_user;
    mac_hdr   = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);
    payload   = mac_hdr + pst_rx_ctrl->mac_header_len;

    ret = hmac_process_assoc_rsp_etc(hmac_vap, hmac_user, mac_hdr, pst_rx_ctrl->mac_header_len,
        payload, pst_rx_ctrl->frame_len - pst_rx_ctrl->mac_header_len);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_process_assoc_rsp:: process assoc rsp failed[%d]}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg_etc(hmac_vap, hmac_user->assoc_id);

    /* 上报关联成功消息给APP */
    ret = hmac_roam_connect_notify_wpas(roam_info, mac_hdr, pst_rx_ctrl->frame_len);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_process_assoc_rsp:: connect notify wpas failed[%d]}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    if (fhook != OSAL_NULL) {
        ret = ((hmac_11r_process_assoc_rsp_cb)fhook)(hmac_vap, roam_info);
        if (ret == OAL_SUCC) {
            return OAL_SUCC;
        }
    }

    if (mac_mib_get_privacyinvoked(hmac_vap) != OSAL_TRUE) {
        /* 非加密情况下，漫游成功 */
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
        hmac_roam_connect_succ(roam_info, OAL_PTR_NULL);
    } else {
        if (mac_mib_get_rsnaactivated(hmac_vap) == OAL_TRUE) {
            hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
            /* 启动握手超时定时器 */
            hmac_roam_connect_start_timer(roam_info, ROAM_HANDSHAKE_TIME_MAX);
        } else {
            /* 非 WPA 或者 WPA2 加密情况下(WEP_OPEN/WEP_SHARED)，漫游成功 */
            hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
            hmac_roam_connect_succ(roam_info, OAL_PTR_NULL);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_process_action
 功能描述  : 处理漫游过程中的action帧
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_process_action(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32                       ret;
    hmac_vap_stru                   *hmac_vap;
    hmac_user_stru                  *hmac_user;
    oal_netbuf_stru                 *net_buffer;
    mac_rx_ctl_stru                *pst_rx_ctrl;
    osal_u8                       *mac_hdr;
    osal_u8                       *payload;
    osal_u8                        bss_addr[WLAN_MAC_ADDR_LEN];
    osal_u8                        frame_sub_type;

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_HANDSHAKING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_process_action::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap  = roam_info->hmac_vap;
    hmac_user = roam_info->hmac_user;

    net_buffer  = (oal_netbuf_stru *)p_param;
    pst_rx_ctrl    = (mac_rx_ctl_stru *)oal_netbuf_cb(net_buffer);
    mac_hdr    = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);
    payload    = mac_hdr + pst_rx_ctrl->mac_header_len;

    /* mac地址校验 */
    mac_get_address3(mac_hdr, (osal_u8 *)bss_addr);
    if (oal_compare_mac_addr(hmac_user->user_mac_addr, (const osal_u8 *)bss_addr) != 0) {
        return OAL_SUCC;
    }

    frame_sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    if (frame_sub_type != (WLAN_FC0_SUBTYPE_ACTION | WLAN_FC0_TYPE_MGT)) {
        return OAL_SUCC;
    }

    if (payload[MAC_ACTION_OFFSET_CATEGORY] == MAC_ACTION_CATEGORY_BA) {
        oam_warning_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_process_action::BA_ACTION_TYPE [%d].}", hmac_vap->vap_id,
            payload[MAC_ACTION_OFFSET_ACTION]);
        switch (payload[MAC_ACTION_OFFSET_ACTION]) {
            case MAC_BA_ACTION_ADDBA_REQ:
                ret = hmac_mgmt_rx_addba_req_etc(hmac_vap, hmac_user, payload);
                break;

            case MAC_BA_ACTION_ADDBA_RSP:
                ret = hmac_mgmt_rx_addba_rsp_etc(hmac_vap, hmac_user, payload);
                break;

            case MAC_BA_ACTION_DELBA:
                ret = hmac_mgmt_rx_delba_etc(hmac_vap, hmac_user, payload);
                break;

            default:
                break;
        }
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_succ
 功能描述  : 关联过程成功后，通知roam main状态机
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_connect_succ(hmac_roam_info_stru *roam_info, osal_void *param)
{
    osal_u32 ret;

    unref_param(param);

    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_HANDSHAKING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_succ::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_UP);

    /* 删除定时器 */
    ret = hmac_roam_connect_del_timer(roam_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_succ::hmac_roam_connect_del_timer fail[%d]!}", ret);
    }

    /* 通知ROAM主状态机 */
    hmac_roam_connect_complete_etc(roam_info->hmac_vap, OAL_SUCC);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_auth_timeout
 功能描述  : 处理认证超时，漫游期间最多5次auth
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_auth_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32 ret;
    hmac_vap_stru *hmac_vap;

    unref_param(p_param);
    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_WAIT_AUTH_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_auth_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_vap = roam_info->hmac_vap;

    if (++roam_info->connect.auth_num >= MAX_AUTH_CNT) {
        return hmac_roam_connect_fail(roam_info);
    }

    ret = hmac_roam_send_auth_seq1(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_auth_timeout::hmac_roam_send_auth_seq1 failed[%d].}",
            hmac_vap->vap_id, ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_assoc_timeout
 功能描述  : 处理关联超时，漫游期间最多5次auth
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_assoc_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32 ret;

    unref_param(p_param);
    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_WAIT_ASSOC_COMP);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_assoc_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    if ((++roam_info->connect.assoc_num >= MAX_ASOC_CNT) || (roam_info->connect.status_code == MAC_REJECT_TEMP)) {
        return hmac_roam_connect_fail(roam_info);
    }

    ret = hmac_roam_send_reassoc_req(roam_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_assoc_timeout::hmac_roam_send_reassoc_req failed[%d].}", ret);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_handshaking_timeout
 功能描述  : 处理握手超时
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_handshaking_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32 ret;

    unref_param(p_param);
    ret = hmac_roam_connect_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING,
        ROAM_CONNECT_STATE_HANDSHAKING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_handshaking_timeout::check_state fail[%d]!}", ret);
        return ret;
    }

    return hmac_roam_connect_fail(roam_info);
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_stop_etc
 功能描述  : 停止connect状态机，删除connect定时器。并通知roam main状态机
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_connect_fail(hmac_roam_info_stru *roam_info)
{
    hmac_vap_stru                   *hmac_vap = roam_info->hmac_vap;
    roam_connect_state_enum_uint8    connect_state = roam_info->connect.state;

    /* connect状态切换 */
    hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_FAIL);

    /* connect失败时，需要添加到黑名单 */
    hmac_roam_alg_add_blacklist_etc(roam_info, roam_info->connect.bss_dscr->bssid,
        ROAM_BLACKLIST_TYPE_REJECT_AP);
    oam_warning_log1(0, OAM_SF_ROAM,
        "vap_id[%d] {hmac_roam_connect_fail::hmac_roam_alg_add_blacklist_etc!}",
        hmac_vap->vap_id);

    /* 通知ROAM主状态机，BSS回退由主状态机完成 */

    if (connect_state == ROAM_CONNECT_STATE_HANDSHAKING) {
        hmac_roam_connect_complete_etc(hmac_vap, OAL_ERR_CODE_ROAM_HANDSHAKE_FAIL);
    } else if ((connect_state == ROAM_CONNECT_STATE_WAIT_ASSOC_COMP) || \
        (connect_state == ROAM_CONNECT_STATE_WAIT_AUTH_COMP)) {
        hmac_roam_connect_complete_etc(hmac_vap, OAL_ERR_CODE_ROAM_NO_RESPONSE);
    } else {
        hmac_roam_connect_complete_etc(hmac_vap, OAL_FAIL);
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_roam_connect_start_etc
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_connect_start_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    hmac_roam_info_stru                             *roam_info;
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_ROAM_CONNECT_START_ETC);

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start_etc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);

    roam_info->connect.bss_dscr = bss_dscr;
    roam_info->connect.auth_num  = 0;
    roam_info->connect.assoc_num = 0;
    roam_info->connect.ft_num    = 0;

    if (fhook != OSAL_NULL) {
        ret = ((hmac_11r_roam_connect_start_etc_cb)fhook)(hmac_vap, roam_info, bss_dscr);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }

    return hmac_roam_connect_fsm_action_etc(roam_info, ROAM_CONNECT_FSM_EVENT_START, (osal_void *)bss_dscr);
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_stop_etc
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_connect_stop_etc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru              *roam_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_start_etc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    roam_info->connect.state = ROAM_CONNECT_STATE_INIT;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_rx_mgmt_etc
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_connect_rx_mgmt_etc(oal_netbuf_stru *netbuf, hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru              *roam_info;
    osal_u32                        ret;

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL) {
        return OAL_CONTINUE;
    }

    /* 漫游开关没有开时，不处理管理帧接收 */
    if (roam_info->enable == 0) {
        return OAL_CONTINUE;
    }

    ret = hmac_roam_connect_fsm_action_etc(roam_info, ROAM_CONNECT_FSM_EVENT_MGMT_RX, (osal_void *)netbuf);
    if (ret != OAL_CONTINUE) {
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_key_done_etc
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_void hmac_roam_connect_key_done_etc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru              *roam_info;
    osal_u32                        ret;

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL) {
        return;
    }

    /* 漫游开关没有开时，不处理管理帧接收 */
    if (roam_info->enable == 0) {
        return;
    }

    /* WPA3漫游切换状态机状态，否则主状态机为非CONNECTING状态/CONNECT状态机为非UP状态，失败 */
    if (hmac_vap->wpa3_roaming == OSAL_TRUE) {
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_CONNECTING);
        hmac_roam_connect_change_state(roam_info, ROAM_CONNECT_STATE_HANDSHAKING);
    } else if (roam_info->main_state != ROAM_MAIN_STATE_CONNECTING) {
        return;
    }

    ret = hmac_roam_connect_fsm_action_etc(roam_info, ROAM_CONNECT_FSM_EVENT_KEY_DONE, OAL_PTR_NULL);
    if (ret != OAL_SUCC && ret != OAL_CONTINUE) {
        oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_connect_key_done_etc::KEY_DONE FAIL[%d]!}",
            hmac_vap->vap_id, ret);
    }
    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_connect_key_done_etc::KEY_DONE !}");

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
