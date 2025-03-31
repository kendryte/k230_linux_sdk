/*
 * Copyright (c) CompanyNameMagicTag. 2015-2024. All rights reserved.
 * 文 件 名   : hmac_roam_main.c
 * 生成日期   : 2015年3月18日
 * 功能描述   : 漫游模块主要对其他模块接口实现
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_roam_main.h"

#include "oal_netbuf_data.h"
#include "oam_struct.h"
#include "mac_ie.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_11i.h"
#include "hmac_roam_connect.h"
#include "hmac_roam_alg.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "wlan_msg.h"
#include "hmac_dfx.h"
#include "hmac_feature_dft.h"
#include "hmac_feature_interface.h"
#include "hmac_btcoex_ps.h"
#include "common_dft.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"
#include "frw_hmac.h"
#include "wlan_msg.h"
#include "hmac_hook.h"
#include "hmac_11v.h"
#include "hmac_11r.h"
#include "hmac_ccpriv.h"
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
#include "nv.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ROAM_MAIN_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_roam_info_stru *g_roam_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

OAL_STATIC hmac_roam_fsm_func g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_BUTT][ROAM_MAIN_FSM_EVENT_TYPE_BUTT];
OAL_STATIC osal_u32 hmac_roam_main_null_fn(hmac_roam_info_stru *roam_info, osal_void *param);
OAL_STATIC osal_u32 hmac_roam_scan_init(hmac_roam_info_stru *roam_info, osal_void *param);
OAL_STATIC osal_u32 hmac_roam_scan_channel(hmac_roam_info_stru *roam_info, osal_void *param);
OAL_STATIC osal_u32 hmac_roam_check_scan_result(hmac_roam_info_stru *roam_info, hmac_bss_mgmt_stru *bss_mgmt,
    mac_bss_dscr_stru **ppst_bss_dscr_out);
OAL_STATIC osal_u32 hmac_roam_to_old_bss(hmac_roam_info_stru *roam_info);
OAL_STATIC osal_u32 hmac_roam_to_new_bss(hmac_roam_info_stru *roam_info, osal_void *param);
OAL_STATIC osal_u32 hmac_roam_main_check_state(hmac_roam_info_stru *roam_info,
    mac_vap_state_enum_uint8 vap_state, roam_main_state_enum_uint8 main_state);
OAL_STATIC osal_u32 hmac_roam_scan_timeout(hmac_roam_info_stru *roam_info, osal_void *param);
OAL_STATIC osal_u32 hmac_roam_connecting_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_connecting_fail(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_handle_fail_handshake_phase(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_handle_scan_result(hmac_roam_info_stru *roam_info, osal_void *p_param);
OAL_STATIC osal_u32 hmac_roam_rssi_trigger_type(hmac_vap_stru *hmac_vap, roam_scenario_enum_uint8 val);
OAL_STATIC osal_s32 hmac_config_roam_hmac_sync_dmac(hmac_vap_stru *hmac_vap, mac_h2d_roam_sync_stru *sync_param);
OAL_STATIC osal_u32 hmac_roam_scan_complete_etc(hmac_vap_stru *hmac_vap);
OAL_STATIC osal_u32 hmac_roam_resume_user_etc(hmac_vap_stru *hmac_vap);
OAL_STATIC osal_u32 hmac_roam_trigger_handle_etc(hmac_vap_stru *hmac_vap, osal_s8 c_rssi,
    oal_bool_enum_uint8 cur_bss_ignore);
OAL_STATIC osal_s32 hmac_config_roam_enable(hmac_vap_stru *hmac_vap, osal_u8 roaming_mode);

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_roam_fsm_init
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/

hmac_roam_info_stru *hmac_get_roam_info(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }
    return g_roam_info[vap_id];
}

OAL_STATIC osal_void hmac_roam_set_linkloss_done(osal_u8 vap_id, osal_u8 val)
{
    if (hmac_get_roam_info(vap_id) != OSAL_NULL) {
        g_roam_info[vap_id]->linkloss_roam_done = val;
    }
}

OAL_STATIC osal_void hmac_roam_fsm_deinit(osal_void)
{
    osal_u32  state;
    osal_u32  event;

    for (state = 0; state < ROAM_MAIN_STATE_BUTT; state++) {
        for (event = 0; event < ROAM_MAIN_FSM_EVENT_TYPE_BUTT; event++) {
            g_hmac_roam_main_fsm_func[state][event] = hmac_roam_main_null_fn;
        }
    }
}

OAL_STATIC osal_void hmac_roam_fsm_init(osal_void)
{
    hmac_roam_fsm_deinit();
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_INIT][ROAM_MAIN_FSM_EVENT_START]              = hmac_roam_scan_init;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_START]           = hmac_roam_scan_channel;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_SCAN_RESULT]     =
        hmac_roam_handle_scan_result;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_TIMEOUT]         = hmac_roam_scan_timeout;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_SCANING][ROAM_MAIN_FSM_EVENT_START_CONNECT]   = hmac_roam_connect_to_bss;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_TIMEOUT]      =
        hmac_roam_connecting_timeout;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_CONNECT_FAIL] = hmac_roam_connecting_fail;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_HANDSHAKE_FAIL] =
        hmac_roam_handle_fail_handshake_phase;
    g_hmac_roam_main_fsm_func[ROAM_MAIN_STATE_CONNECTING][ROAM_MAIN_FSM_EVENT_CONNECT_SUCC] = hmac_roam_to_new_bss;
}

/*****************************************************************************
 函 数 名  : hmac_roam_main_fsm_action_etc
 功能描述  : 调用漫游主状态机函数表
 输入参数  : hmac_vap: hmac vap
             event: 事件类型
             p_param: 输入参数
************* ****************************************************************/
osal_u32 hmac_roam_main_fsm_action_etc(hmac_roam_info_stru *roam_info, roam_main_fsm_event_type_enum event,
    osal_void *p_param)
{
    if (osal_unlikely(roam_info == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->main_state >= ROAM_MAIN_STATE_BUTT) {
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    if (event >= ROAM_MAIN_FSM_EVENT_TYPE_BUTT) {
        return OAL_ERR_CODE_ROAM_EVENT_UXEXPECT;
    }

    wifi_printf("hmac_roam_main_fsm_action_etc::roam_main_state[%d] event[%d]\r\n",
        roam_info->main_state, event);
    return g_hmac_roam_main_fsm_func[roam_info->main_state][event](roam_info, p_param);
}

/*****************************************************************************
 函 数 名  : hmac_roam_main_change_state
 功能描述  : 改变状态机状态
 输入参数  : roam_info
             roam_main_state
*****************************************************************************/
osal_void hmac_roam_main_change_state(hmac_roam_info_stru *roam_info, roam_main_state_enum_uint8 state)
{
    if (roam_info) {
        oam_warning_log2(0, OAM_SF_ROAM,
                         "{hmac_roam_main_change_state::[%d]->[%d]}", roam_info->main_state, state);
        roam_info->main_state = state;
    }
}

/*****************************************************************************
 函 数 名  : hmac_roam_main_timeout
 功能描述  : 超时处理函数
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_main_timeout(osal_void *p_arg)
{
    hmac_roam_info_stru *roam_info;

    if (osal_unlikely(p_arg == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    roam_info = (hmac_roam_info_stru *)p_arg;

    oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_main_timeout::MAIN_STATE[%d] CONNECT_STATE[%d].}",
                     roam_info->main_state, roam_info->connect.state);

    return hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, OAL_PTR_NULL);
}

OAL_STATIC osal_void hmac_roam_renew_privacy_wpa(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    mac_conn_security_stru *conn_sec)
{
    osal_u32 match_suite;

    conn_sec->privacy = OAL_TRUE;
    hmac_ie_get_wpa_cipher(bss_dscr->wpa_ie, &(conn_sec->crypto));

    if (conn_sec->crypto.pair_suite[0] != 0 && conn_sec->crypto.pair_suite[1] != 0) {
        /* wpa/wpa2 混合加密算法按算法最大能力漫游, AES > TKIP */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_roam_renew_privacy_wpa::pair_suite[0]=[0x%x], pair_suite[1]=[0x%x]}",
            conn_sec->crypto.pair_suite[0], conn_sec->crypto.pair_suite[1]);
        conn_sec->crypto.pair_suite[0] = (conn_sec->crypto.pair_suite[0] == MAC_WPA_CHIPER_CCMP) ?
            conn_sec->crypto.pair_suite[0] : conn_sec->crypto.pair_suite[1];
    } else {
        match_suite = mac_mib_wpa_pair_match_suites(hmac_vap, conn_sec->crypto.pair_suite);
        if (match_suite != 0) {
            conn_sec->crypto.pair_suite[0] = match_suite;
        }
    }
    conn_sec->crypto.pair_suite[1] = 0;

    match_suite = mac_mib_wpa_akm_match_suites(hmac_vap, conn_sec->crypto.pair_suite);
    if (match_suite != 0) {
        conn_sec->crypto.akm_suite[0] = match_suite;
    }
    conn_sec->crypto.akm_suite[1] = 0;
}

OAL_STATIC osal_void hmac_roam_renew_privacy_rsn(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    mac_conn_security_stru *conn_sec, osal_u16 *rsn_cap_info)
{
    osal_u32 match_suite;

    conn_sec->privacy = OAL_TRUE;
    hmac_ie_get_rsn_cipher(bss_dscr->rsn_ie, &(conn_sec->crypto));

    if (conn_sec->crypto.pair_suite[0] != 0 && conn_sec->crypto.pair_suite[1] != 0) {
        /* wpa2/wpa3 混合加密算法按算法最大能力漫游, AES > TKIP */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_roam_renew_privacy_rsn::pair_suite[0]=[0x%x], pair_suite[1]=[0x%x]}",
            conn_sec->crypto.pair_suite[0], conn_sec->crypto.pair_suite[1]);
        conn_sec->crypto.pair_suite[0] = (conn_sec->crypto.pair_suite[0] == MAC_RSN_CHIPER_CCMP) ?
            conn_sec->crypto.pair_suite[0] : conn_sec->crypto.pair_suite[1];
    } else {
        match_suite = mac_mib_rsn_pair_match_suites(hmac_vap, conn_sec->crypto.pair_suite);
        if (match_suite != 0) {
            conn_sec->crypto.pair_suite[0] = match_suite;
        }
    }
    conn_sec->crypto.pair_suite[1] = 0;

    if (conn_sec->crypto.akm_suite[0] != 0 && conn_sec->crypto.akm_suite[1] != 0) {
        /* wpa2/wpa3 混合加密方式按最大能力漫游, WPA3 > WPA2 */
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_roam_renew_privacy_rsn::akm_suite[0]=[0x%x], akm_suite[1]=[0x%x]}",
            conn_sec->crypto.akm_suite[0], conn_sec->crypto.akm_suite[1]);
        conn_sec->crypto.akm_suite[0] = (conn_sec->crypto.akm_suite[0] == MAC_RSN_AKM_SAE_SHA256) ?
            conn_sec->crypto.akm_suite[0] : conn_sec->crypto.akm_suite[1];
    } else {
        match_suite = mac_mib_rsn_akm_match_suites(hmac_vap, conn_sec->crypto.pair_suite);
        if (match_suite != 0) {
            conn_sec->crypto.akm_suite[0] = match_suite;
        }
    }
    conn_sec->crypto.akm_suite[1] = 0;
    /* 获取目的AP的RSN CAP信息 */
    *rsn_cap_info = hmac_get_rsn_capability_etc(bss_dscr->rsn_ie);
}

/*****************************************************************************
 函 数 名  : hmac_roam_renew_privacy
 功能描述  : 为了兼容不同加密方式的ap之间的漫游，需要对本vap的加密属性最大初始化
 输入参数  : RSN优先，AKM优先，当前PAIR优先
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_renew_privacy(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr)
{
    osal_u32 ul_ret;
    mac_conn_security_stru conn_sec = {0};
    mac_cap_info_stru *cap_info = OAL_PTR_NULL;
    osal_u16 rsn_cap_info = 0;
    oal_bool_enum_uint8 pmf_cap;
    oal_bool_enum_uint8 pmf_require;

    cap_info = (mac_cap_info_stru *)&bss_dscr->us_cap_info;

    if (cap_info->privacy == 0) {
        return OAL_SUCC;
    }

    memset_s(&conn_sec, OAL_SIZEOF(mac_conn_security_stru), 0, OAL_SIZEOF(mac_conn_security_stru));

    if (bss_dscr->wpa_ie != OAL_PTR_NULL) {
        hmac_roam_renew_privacy_wpa(hmac_vap, bss_dscr, &conn_sec);
    }

    if (bss_dscr->rsn_ie != OAL_PTR_NULL) {
        hmac_roam_renew_privacy_rsn(hmac_vap, bss_dscr, &conn_sec, &rsn_cap_info);
    }

    /* 漫游时使用上次pmf能力位 并判断与漫游的目的ap能力位是否匹配 */
    pmf_cap = mac_mib_get_dot11_rsnamfpc(hmac_vap);
    pmf_require = mac_mib_get_dot11_rsnamfpr(hmac_vap);
    /* AP1 roam to AP2, AP1 PMF=2, AP2 PMF = 0/1, 不支持漫游 */
    if (((hmac_vap->user_rsn_cap & BIT6) != 0) && ((rsn_cap_info & BIT6) == 0)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_renew_privacy::PMF = 2 roam to PMF = 0/1 is not supported}");
        return OAL_FAIL;
    }
    /* AP1 roam to AP2, AP1 PMF=0/1, AP2 PMF = 2, 不支持漫游 */
    if (((hmac_vap->user_rsn_cap & BIT6) == 0) && ((rsn_cap_info & BIT6) != 0)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_renew_privacy::PMF = 0/1 roam to PMF = 2 is not supported}");
        return OAL_FAIL;
    }
    /* 直接赋值上次一次的pmf能力位 不会出现r=1 c=0的异常，故不作判断 */
    conn_sec.mgmt_proteced = hmac_vap->user_pmf_cap;
    conn_sec.pmf_cap = pmf_cap + pmf_require;

    ul_ret = hmac_vap_init_privacy_etc(hmac_vap, &conn_sec);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_roam_renew_privacy:: mac_11i_init_privacy failed[%d]!}\r\n", ul_ret);
        return ul_ret;
    }

    oam_warning_log3(0, OAM_SF_CFG, "{hmac_roam_renew_privacy::wpa_cap[%d], wpa2_cap[%d], wpa3_cap[%d]}",
        hmac_vap->cap_flag.wpa, hmac_vap->cap_flag.wpa2, hmac_vap->cap_flag.wpa3);
    return OAL_SUCC;
}

static osal_void hmac_roam_init_config(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();

    roam_info->config.scan_band =
        (hmac_device_check_5g_enable(hmac_vap->device_id) == OSAL_TRUE) ? (BIT0 | BIT1) : BIT0;
    roam_info->config.scan_orthogonal   = wlan_customize_etc->roam_scan_orthogonal;
    roam_info->config.c_trigger_rssi_2g    = wlan_customize_etc->c_roam_trigger_b;
    roam_info->config.c_trigger_rssi_5g    = wlan_customize_etc->c_roam_trigger_a;
    roam_info->config.c_dense_env_trigger_rssi_2g = wlan_customize_etc->c_dense_env_roam_trigger_b;
    roam_info->config.c_dense_env_trigger_rssi_5g = wlan_customize_etc->c_dense_env_roam_trigger_a;
    roam_info->config.delta_rssi_2g     = (osal_u8)wlan_customize_etc->c_roam_delta_b;
    roam_info->config.delta_rssi_5g     = (osal_u8)wlan_customize_etc->c_roam_delta_a;
    roam_info->config.scenario_enable     = wlan_customize_etc->scenario_enable;
    roam_info->config.c_candidate_good_rssi  = wlan_customize_etc->c_candidate_good_rssi;
    roam_info->config.candidate_good_num  = wlan_customize_etc->candidate_good_num;
    roam_info->config.candidate_weak_num  = wlan_customize_etc->candidate_weak_num;
#else
    unref_param(hmac_vap);
    roam_info->config.scan_band         = ROAM_BAND_2G_BIT | ROAM_BAND_5G_BIT;
    roam_info->config.scan_orthogonal   = ROAM_SCAN_CHANNEL_ORG_BUTT;
    roam_info->config.c_trigger_rssi_2g    = ROAM_RSSI_NE70_DB;
    roam_info->config.c_trigger_rssi_5g    = ROAM_RSSI_NE70_DB;
    roam_info->config.c_dense_env_trigger_rssi_2g = ROAM_RSSI_NE70_DB;
    roam_info->config.c_dense_env_trigger_rssi_5g = ROAM_RSSI_NE68_DB;
    roam_info->config.delta_rssi_2g     = ROAM_RSSI_DIFF_10_DB;
    roam_info->config.delta_rssi_5g     = ROAM_RSSI_DIFF_10_DB;
    roam_info->config.scenario_enable     = OAL_FALSE;
    roam_info->config.c_candidate_good_rssi  = ROAM_ENV_RSSI_NE60_DB;
    roam_info->config.candidate_good_num  = ROAM_ENV_CANDIDATE_GOOD_NUM;
    roam_info->config.candidate_weak_num  = ROAM_ENV_CANDIDATE_WEAK_NUM;
#endif
}

/*****************************************************************************
 函 数 名  : hmac_roam_info_init_etc
 功能描述  : roam过程信息初始化
 输入参数  : hmac_vap 需要初始化roam模块的vap，默认参数配置
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_bool hmac_roam_info_init_etc(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_vap_stru *hmac_vap;
    hmac_roam_info_stru *roam_info;

    if (osal_unlikely(hmac_user == OAL_PTR_NULL)) {
        return OSAL_FALSE;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL) {
        return OSAL_FALSE;
    }

    roam_info->main_state  = ROAM_MAIN_STATE_INIT;
    roam_info->rssi_type = ROAM_ENV_SPARSE_AP;
    roam_info->invalid_scan_cnt = 0;
    roam_info->connect.state = ROAM_CONNECT_STATE_INIT;
    roam_info->alg.max_capacity = 0;
    roam_info->alg.max_capacity_bss = OAL_PTR_NULL;
    roam_info->alg.c_current_rssi = 0;
    roam_info->alg.c_max_rssi = 0;
    roam_info->alg.another_bss_scaned = 0;
    roam_info->alg.invalid_scan_cnt = 0;
    roam_info->alg.candidate_bss_num = 0;
    roam_info->alg.candidate_good_rssi_num = 0;
    roam_info->alg.candidate_weak_rssi_num = 0;
    roam_info->alg.scan_period   = 0;
    roam_info->alg.max_rssi_bss = OAL_PTR_NULL;
    roam_info->hmac_user = hmac_user;
    if (roam_info->hmac_user == OAL_PTR_NULL) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_info_init_etc::assoc_vap_id[%d] can't found.}",
            hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        return OSAL_FALSE;
    }

    roam_info->trigger.trigger_2g = roam_info->config.c_trigger_rssi_2g;
    roam_info->trigger.trigger_5g = roam_info->config.c_trigger_rssi_5g;

    /* 设置门限时，reset统计值，重新设置门限后，可以马上触发一次漫游 */
    roam_info->trigger.cnt = 0;
    roam_info->trigger.time_stamp = 0;

    oam_warning_log3(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_info_init_etc, trigger[%d, %d]}", hmac_vap->vap_id,
        roam_info->trigger.trigger_2g, roam_info->trigger.trigger_5g);

    return OSAL_TRUE;
}


/*****************************************************************************
 函 数 名  : hmac_roam_connect_start_timer
 功能描述  : 如果定时器已存在，重启，否则创建定时器
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_void hmac_roam_main_start_timer(hmac_roam_info_stru *roam_info, osal_u32 timeout)
{
    frw_timeout_stru            *timer = &(roam_info->timer);

    oam_info_log1(0, OAM_SF_ROAM, "{hmac_roam_main_start_timer [%d].}", timeout);

    /* 启动认证超时定时器 */
    frw_create_timer_entry(timer,
                           hmac_roam_main_timeout,
                           timeout,
                           roam_info,
                           OAL_FALSE);
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_del_timer
 功能描述  : 立即删除roam connect定时器
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_void  hmac_roam_main_del_timer(hmac_roam_info_stru *roam_info)
{
    oam_info_log0(0, OAM_SF_ROAM, "{hmac_roam_main_del_timer.}");
    frw_destroy_timer_entry(&(roam_info->timer));
    return;
}

/*****************************************************************************
 函 数 名  : hmac_roam_start_etc
 功能描述  : 发起一次漫游
*****************************************************************************/
osal_u32 hmac_roam_start_etc(hmac_vap_stru *hmac_vap, roam_channel_org_enum scan_type,
    oal_bool_enum_uint8 cur_bss_ignore, roam_trigger_enum_uint8 roam_trigger)
{
    osal_u32             ul_ret;
    hmac_roam_info_stru   *roam_info;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_start_etc::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_start_etc::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    /* 每次漫游前，刷新是否支持漫游到自己的参数 */
    roam_info->config.scan_orthogonal = scan_type;
    roam_info->current_bss_ignore = cur_bss_ignore; /* true表示漫游到自己 */
    roam_info->roam_trigger       = roam_trigger;

    oam_warning_log4(0, OAM_SF_ROAM,
        "vap_id[%d] {hmac_roam_start_etc::START succ, scan_type=%d, cur_bss_ignore=%d roam_trigger=%d.}",
        hmac_vap->vap_id, scan_type, cur_bss_ignore, roam_trigger);

    hmac_roam_alg_init_etc(roam_info, ROAM_RSSI_CMD_TYPE);

    /* 触发漫游是否搭配扫描0表示不扫描 */
    if (scan_type == ROAM_SCAN_CHANNEL_ORG_0) {
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);

        ul_ret = hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_TIMEOUT, OAL_PTR_NULL);
        if (ul_ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_start_etc::START fail[%d].}",
                           hmac_vap->vap_id, ul_ret);
            return ul_ret;
        }
    } else {
        ul_ret = hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_START, OAL_PTR_NULL);
        if (ul_ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_start_etc::START fail[%d].}",
                           hmac_vap->vap_id, ul_ret);
            return ul_ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_show_etc
 功能描述  : 发起一次漫游
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_show_etc(const hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info;
    hmac_roam_static_stru *pst_static;
    hmac_roam_config_stru *info;
    osal_u8 vap_id = hmac_vap->vap_id;

    roam_info = hmac_get_roam_info(vap_id);
    pst_static = &roam_info->st_static;
    info   = &(roam_info->config);

    oam_warning_log4(0, OAM_SF_ROAM,
        "{hmac_roam_show_etc::band[%d] scan_orthogonal[%d], c_trigger_rssi_2g=%d c_trigger_rssi_5g=%d}",
        info->scan_band, info->scan_orthogonal, info->c_trigger_rssi_2g, info->c_trigger_rssi_5g);
    oam_warning_log4(0, OAM_SF_ROAM,
        "{hmac_roam_show_etc::trigger_rssi_cnt[%u] trigger_linkloss_cnt[%u],"
        " dense env c_trigger_rssi_2g=%d c_trigger_rssi_5g=%d.}",
        pst_static->trigger_rssi_cnt, pst_static->trigger_linkloss_cnt, info->c_dense_env_trigger_rssi_2g,
        info->c_dense_env_trigger_rssi_5g);
    oam_warning_log3(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_show_etc::scan_cnt[%u] scan_result_cnt[%u].}",
        vap_id, pst_static->scan_cnt, pst_static->scan_result_cnt);
    oam_warning_log4(0, OAM_SF_ROAM,
        "vap_id[%d] {hmac_roam_show_etc::total_cnt[%u] roam_success_cnt[%u] roam_fail_cnt[%u].}",
        vap_id, pst_static->connect_cnt, pst_static->roam_new_cnt, pst_static->roam_old_cnt);
    oam_warning_log3(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_show_etc::roam_to, scan fail=%d, eap fail=%d}",
        vap_id, roam_info->st_static.roam_scan_fail, roam_info->st_static.roam_eap_fail);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_main_null_fn
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_main_null_fn(hmac_roam_info_stru *roam_info, osal_void *param)
{
    unref_param(roam_info);
    unref_param(param);

    oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_main_null_fn .}");

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_scan_comp_cb
 功能描述  : wpa_supplicant下发的扫描请求的回调函数，用于对扫描完成时对结果的处理
 输入参数  : void  *p_scan_record，扫描记录，包括扫描发起者信息和扫描结果
*****************************************************************************/
OAL_STATIC osal_void  hmac_roam_scan_comp_cb(void  *p_scan_record)
{
    hmac_scan_record_stru           *scan_record = (hmac_scan_record_stru *)p_scan_record;
    hmac_vap_stru                   *hmac_vap = OAL_PTR_NULL;
    hmac_roam_info_stru             *roam_info;
    hmac_device_stru                *hmac_device;
    hmac_bss_mgmt_stru              *scan_bss_mgmt;
    osal_u32 ret;

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_record->vap_id);
    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_roam_scan_comp_cb::hmac_vap is null.");
        return;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete_etc::device null!}");
        return;
    }

    scan_bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL) {
        return;
    }

    /* 漫游开关没有开时，不处理扫描结果 */
    if (roam_info->enable == 0) {
        return;
    }

    oam_info_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_scan_complete_etc::handling scan result!}",
                  scan_record->vap_id);

    ret = hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_SCAN_RESULT, (void *)scan_bss_mgmt);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_scan_complete_etc::hmac_roam_main_fsm_action_etc failed[%d].}",
            scan_record->vap_id, ret);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_roam_scan_init
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_scan_init(hmac_roam_info_stru *roam_info, osal_void *param)
{
    osal_u32              ul_ret;
    mac_scan_req_stru      *scan_params;
    osal_u8              *cur_ssid;
    hmac_vap_stru           *hmac_vap;

    unref_param(param);

    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_init::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    if (mac_is_wep_enabled(roam_info->hmac_vap)) {
        hmac_roam_rssi_trigger_type((roam_info->hmac_vap), ROAM_ENV_LINKLOSS);
        return OAL_SUCC;
    }

    scan_params = &roam_info->scan_params;
    cur_ssid = mac_mib_get_desired_ssid(roam_info->hmac_vap);

    /* 扫描参数初始化 */
    scan_params->bss_type         = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    scan_params->scan_type        = WLAN_SCAN_TYPE_ACTIVE;
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    /* 解决ws92 liteOS 11v扫描结果不更新问题. probe resp从bin到device ko的耗时大于(2*scan_time) */
    scan_params->scan_time        = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
#else
    scan_params->scan_time        = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
#endif
    scan_params->probe_delay      = 0;
    scan_params->scan_func        = MAC_SCAN_FUNC_BSS;               /* 默认扫描bss */
    scan_params->fn_cb             = hmac_roam_scan_comp_cb;
    scan_params->max_send_probe_req_count_per_channel = 2; /* 2表示每次信道发送扫描请求帧的个数 */
    scan_params->max_scan_count_per_channel           = 2; /* 2表示每个信道扫描次数 */
    scan_params->scan_mode        = WLAN_SCAN_MODE_ROAM_SCAN;

    hmac_vap = roam_info->hmac_vap;
    scan_params->desire_fast_scan = hmac_vap->roam_scan_valid_rslt;

    (osal_void)memcpy_s(scan_params->mac_ssid_set[0].ssid, WLAN_SSID_MAX_LEN, cur_ssid, WLAN_SSID_MAX_LEN);
    scan_params->ssid_num                             = 1;

    /* 初始扫描请求只指定1个bssid，为广播地址 */
    memset_s(scan_params->bssid, WLAN_MAC_ADDR_LEN, 0xff, WLAN_MAC_ADDR_LEN);
    scan_params->bssid_num                            = 1;

    ul_ret = hmac_roam_alg_scan_channel_init_etc(roam_info, scan_params);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM,
                       "{hmac_roam_scan_init::hmac_roam_alg_scan_channel_init_etc fail[%d]}", ul_ret);
        return ul_ret;
    }
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_SCANING);

    ul_ret = hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_START, (osal_void *)roam_info);
    if (ul_ret != OAL_SUCC) {
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_FAIL);
        return ul_ret;
    }

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_roam_scan_channel
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_scan_channel(hmac_roam_info_stru *roam_info, osal_void *param)
{
    osal_u32              ul_ret;

    unref_param(param);

    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_channel::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    roam_info->st_static.scan_cnt++;

    /* 发起背景扫描 */
    ul_ret = hmac_fsm_call_func_sta_etc(roam_info->hmac_vap, HMAC_FSM_INPUT_SCAN_REQ,
                                        (osal_void *)(&roam_info->scan_params));
    if (ul_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_channel::start scan failed!}");
    }

    /* 启动扫描超时定时器 */
    hmac_roam_main_start_timer(roam_info, ROAM_SCAN_TIME_MAX);

    return OAL_SUCC;
}

static osal_u32 hmac_roam_process(hmac_vap_stru *hmac_vap, const hmac_device_stru *hmac_device)
{
    hmac_roam_info_stru *roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    hmac_roam_alg_stru *alg = &roam_info->alg;

    if (roam_info->config.scenario_enable != OAL_TRUE) {
        alg->better_rssi_scan_period = 0;
        return OAL_SUCC;
    }
    // full channel scan
    if (hmac_device->scan_mgmt.scan_record_mgmt.chan_numbers >= WLAN_FULL_CHANNEL_NUM) {
        if ((alg->candidate_good_rssi_num >= roam_info->config.candidate_good_num) ||
            (alg->candidate_weak_rssi_num >= roam_info->config.candidate_weak_num)) {
            alg->scan_period = 0;
            oam_warning_log4(0, OAM_SF_ROAM,
                "vap_id[%d] {hmac_roam_process::candidate bss total[%d] good[%d] weak[%d]}", hmac_vap->vap_id,
                alg->candidate_bss_num, alg->candidate_good_rssi_num, alg->candidate_weak_rssi_num);
            if (roam_info->rssi_type != ROAM_ENV_DENSE_AP) {
                hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_DENSE_AP); // dense AP roaming scenario
            }
        } else {
            /* When switch from dense AP scenario to default AP scenario, for 5 times continuous none-dense-AP scan; */
            alg->scan_period++;
            if ((alg->another_bss_scaned != 0) && (alg->scan_period == ROAM_ENV_DENSE_TO_SPARSE_PERIOD)) {
                oam_warning_log4(0, OAM_SF_ROAM,
                    "vap_id[%d] {hmac_roam_process::roam_to sparse AP env: candidate bss total[%d] good[%d] weak[%d]}",
                    hmac_vap->vap_id, alg->candidate_bss_num,
                    alg->candidate_good_rssi_num, alg->candidate_weak_rssi_num);
                hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_SPARSE_AP); // default roaming scenario
            }
        }
    } else if (alg->another_bss_scaned == OSAL_TRUE) {
        hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_SPARSE_AP); // default roaming scenario
    }

    if (hmac_roam_alg_need_to_stop_roam_trigger_etc(roam_info) == OSAL_TRUE) {
        return hmac_roam_rssi_trigger_type(hmac_vap, ROAM_ENV_LINKLOSS); // LINKLOSS roaming scenario
    }

    /* Another Roaming Scenario: current AP >= -75dB, max RSSI AP - current AP >= 20dB */
    if ((alg->c_max_rssi - alg->c_current_rssi >= ROAM_ENV_BETTER_RSSI_DISTANSE) &&
        (alg->c_current_rssi >= ROAM_RSSI_NE75_DB) && // We'd better keep device to trigger roaming
        (alg->c_current_rssi <= ROAM_ENV_RSSI_NE60_DB)) {
        alg->better_rssi_scan_period++;
        oam_warning_log4(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_process::roam_to better AP env:[%d] max_rssi[%d] current_rssi[%d]}",
            hmac_vap->vap_id, alg->better_rssi_scan_period, alg->c_max_rssi, alg->c_current_rssi);
        if (alg->better_rssi_scan_period == ROAM_ENV_BETTER_RSSI_PERIOD) {
            alg->better_rssi_scan_period = 0;
            hmac_roam_start_etc(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, OAL_FALSE, ROAM_TRIGGER_APP);
        }
    } else {
        alg->better_rssi_scan_period = 0;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_check_bkscan_result_etc
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_check_bkscan_result_etc(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record)
{
    hmac_roam_info_stru *roam_info = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_bss_mgmt_stru *scan_bss_mgmt = OAL_PTR_NULL;
    struct osal_list_head *entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *scanned_bss = OAL_PTR_NULL;
    mac_bss_dscr_stru *bss_dscr = OAL_PTR_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (scan_record == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_check_bkscan_result_etc::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!is_legacy_sta(hmac_vap) || (hmac_vap->vap_state != MAC_VAP_STATE_UP)) {
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_check_bkscan_result_etc::device is null}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (mac_is_wep_enabled(hmac_vap)) {
        return OAL_SUCC;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL || roam_info->enable == 0) {
        return OAL_SUCC;
    }

    scan_bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
    roam_info->alg.candidate_bss_num       = 0;
    roam_info->alg.candidate_good_rssi_num = 0;
    roam_info->alg.candidate_weak_rssi_num = 0;
    roam_info->alg.c_max_rssi     = ROAM_RSSI_CMD_TYPE;
    roam_info->alg.c_current_rssi = 0;

    osal_spin_lock(&(scan_bss_mgmt->lock));

    /* 遍历扫描到的bss信息，查找可以漫游的bss */
    osal_list_for_each(entry, &(scan_bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);
        hmac_roam_alg_bss_in_ess_etc(roam_info, bss_dscr);
        bss_dscr = OAL_PTR_NULL;
    }

    osal_spin_unlock(&(scan_bss_mgmt->lock));

    /* Roaming Scenario Recognition (RISK: strong relationship with SCAN, need exclude partial channel scan)
     * dense AP standard: RSSI>=-60dB, candidate num>=5;
     *                    RSSI<-60dB && RSSI >=-75dB, candidate num>=10;
     */
    return hmac_roam_process(hmac_vap, hmac_device);
}

OAL_STATIC osal_u32 hmac_roam_search_bss(hmac_roam_info_stru *roam_info, hmac_bss_mgmt_stru *bss_mgmt)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    hmac_scanned_bss_info *scanned_bss = OAL_PTR_NULL;
    mac_bss_dscr_stru       *bss_dscr = OAL_PTR_NULL;
    oal_bool_enum_uint8 flag_in_scan_rslts = OAL_FALSE;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11V_CHECK_BSS_DSCR);

    osal_spin_lock(&(bss_mgmt->lock));

    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        bss_dscr    = &(scanned_bss->bss_dscr_info);

        if (hmac_roam_alg_bss_check_etc(roam_info, bss_dscr) != OAL_SUCC) {
            continue;
        }

        if (fhook != OSAL_NULL) {
            if (((hmac_11v_check_bss_dscr_cb)fhook)(roam_info, bss_dscr) == OSAL_TRUE) {
                flag_in_scan_rslts = OAL_TRUE;
                break;
            }
        }

        bss_dscr = OAL_PTR_NULL;
    }

    osal_spin_unlock(&(bss_mgmt->lock));
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11V_CHECK_SCAN_RESULT);
    if (fhook != OSAL_NULL) {
        if (((hmac_11v_check_scan_result_cb)fhook)(roam_info, flag_in_scan_rslts) != OSAL_TRUE) {
            return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_check_scan_result
 输出参数  : mac_bss_dscr_stru **ppst_bss_dscr_out
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_roam_check_scan_result(hmac_roam_info_stru *roam_info, hmac_bss_mgmt_stru *bss_mgmt,
    mac_bss_dscr_stru **ppst_bss_dscr_out)
{
    osal_u32 ul_ret;
    hmac_vap_stru *hmac_vap = OAL_PTR_NULL;
    mac_bss_dscr_stru *bss_dscr = OAL_PTR_NULL;

    hmac_vap = roam_info->hmac_vap;
    if ((hmac_vap == OAL_PTR_NULL) || (bss_mgmt == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::vap invalid!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info->st_static.scan_result_cnt++;

    /* 如果扫描到的bss个数为0，退出 */
    if (bss_mgmt->bss_num == 0) {
        oam_warning_log0(0, OAM_SF_ROAM, "{hmac_roam_check_scan_result::no bss scanned}");
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* 遍历扫描到的bss信息，查找可以漫游的bss */
    ul_ret = hmac_roam_search_bss(roam_info, bss_mgmt);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    bss_dscr = hmac_roam_alg_select_bss_etc(roam_info);
    if (bss_dscr == OAL_PTR_NULL) {
        /* 没有扫描到可用的bss，等待定时器超时即可 */
        roam_info->st_static.roam_scan_fail++;
        oam_warning_log1(0, OAM_SF_ROAM, "{roam_to::hmac_roam_check_scan_result::no bss valid, scan fail=%d}",
            roam_info->st_static.roam_scan_fail);

        hmac_vap->roam_scan_valid_rslt = OAL_FALSE;
        return OAL_ERR_CODE_ROAM_NO_VALID_BSS;
    }

    /* renew失败不漫游至该ap */
    ul_ret = hmac_roam_renew_privacy(hmac_vap, bss_dscr);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    ul_ret = hmac_check_capability_mac_phy_supplicant_etc(hmac_vap, bss_dscr);
    if (ul_ret != OAL_SUCC) {
        /* MAC/PHY 能力不做严格检查 */
        oam_warning_log1(0, OAM_SF_ROAM,
            "{hmac_roam_check_scan_result::check mac and phy capability fail[%d]!}\r\n", ul_ret);
    }

    *ppst_bss_dscr_out = bss_dscr;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_handle_scan_result
 功能描述  : 处理漫游扫描结果
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_handle_scan_result(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    mac_bss_dscr_stru       *bss_dscr   = OAL_PTR_NULL;
    hmac_scan_rsp_stru       scan_rsp;
    hmac_device_stru        *hmac_device;
    hmac_vap_stru           *hmac_vap;
    osal_u32               ul_ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11V_TRIGGER_ROAM_CHECK);

    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    hmac_vap = roam_info->hmac_vap;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::hmac_device null.}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    ul_ret = hmac_roam_check_scan_result(roam_info, (hmac_bss_mgmt_stru *)p_param, &bss_dscr);
    if (ul_ret == OAL_SUCC) {
        roam_info->invalid_scan_cnt = 0;
        /* 扫描结果发给sme */
        memset_s(&scan_rsp, OAL_SIZEOF(scan_rsp), 0, OAL_SIZEOF(scan_rsp));

        scan_rsp.result_code = MAC_SCAN_SUCCESS;

        hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_SCAN_RSP, (osal_u8 *)&scan_rsp);

        /* 扫描到可用的bss，开始connect */
        return hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_START_CONNECT,
            (osal_void *)bss_dscr);
    }

    oam_warning_log1(0, OAM_SF_ROAM, "{Roaming conditions are not met, ret[%d].}", ul_ret);
    /* 如果是亮屏的，不暂停漫游 */
    if (hmac_device->uc_in_suspend == OAL_FALSE) {
        roam_info->invalid_scan_cnt = 0;
    } else {
        roam_info->invalid_scan_cnt++;
    }

    /* 多次无效扫描暂停漫游，防止在某些场景下一直唤醒HOST */
    if (roam_info->invalid_scan_cnt >= ROAM_INVALID_SCAN_MAX) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_handle_scan_result::ignore_rssi_trigger after %d invalid_scan.}",
                         roam_info->invalid_scan_cnt);
        roam_info->invalid_scan_cnt = 0;
        hmac_roam_rssi_trigger_type(roam_info->hmac_vap, ROAM_ENV_LINKLOSS);
    }
    /* 删除定时器 */
    hmac_roam_main_del_timer(roam_info);
    hmac_roam_main_clear(roam_info);

    if (fhook != OSAL_NULL) {
        ((hmac_11v_trigger_roam_check_cb)fhook)(roam_info, hmac_vap);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_main_check_state
 功能描述  : 参数检查接口
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_main_check_state(hmac_roam_info_stru *roam_info,
    mac_vap_state_enum_uint8 vap_state, roam_main_state_enum_uint8 main_state)
{
    if (roam_info == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->hmac_user == OAL_PTR_NULL) {
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    if (roam_info->hmac_vap == OAL_PTR_NULL || !is_legacy_sta(roam_info->hmac_vap)) {
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (roam_info->enable == 0) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if (roam_info->hmac_vap->wpa3_roaming == OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_ROAM, "hmac_roam_main_check_state::wpa3 roaming skip state check");
        return OAL_SUCC;
    }

    if ((roam_info->hmac_vap->vap_state != vap_state) ||
        (roam_info->main_state != main_state)) {
        oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_main_check_state::unexpect vap State[%d] main_state[%d]!}",
                         roam_info->hmac_vap->vap_state, roam_info->main_state);
        return OAL_ERR_CODE_ROAM_INVALID_VAP_STATUS;
    }

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_roam_main_clear
 功能描述  : 清理、释放 roam main 相关状态、buff、timer
*****************************************************************************/
osal_void  hmac_roam_main_clear(hmac_roam_info_stru *roam_info)
{
    /* 清理状态 */
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_INIT);

    hmac_roam_connect_stop_etc(roam_info->hmac_vap);
}


/*****************************************************************************
 函 数 名  : hmac_roam_resume_pm
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_resume_pm(hmac_roam_info_stru *roam_info)
{
    osal_u32 ul_ret;
    osal_u8 pm_switch;

    ul_ret = OAL_SUCC;
#ifdef _PRE_WLAN_FEATURE_STA_PM
    pm_switch = (roam_info->hmac_vap->sta_pm_handler.last_ps_status != 0) ?
        MAC_STA_PM_SWITCH_ON : MAC_STA_PM_SWITCH_RESET;
    ul_ret = hmac_config_set_pm_by_module_etc(roam_info->hmac_vap, MAC_STA_PM_CTRL_TYPE_ROAM, pm_switch);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_roam_resume_pm::hmac_config_set_pm_by_module_etc failed[%d].}",
            roam_info->hmac_vap->vap_id, ul_ret);
    }
#endif

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_resume_security_port
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_resume_security_port(hmac_roam_info_stru *roam_info)
{
    mac_h2d_roam_sync_stru  h2d_sync = {0};
    osal_s32 ret;

    if (roam_info == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::roam_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (roam_info->hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    if (roam_info->hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_resume_security_port::user null!}");
        return OAL_ERR_CODE_ROAM_INVALID_USER;
    }

    /* 设置用户8021x端口合法性的状态为合法 */
    hmac_user_set_port_etc(roam_info->hmac_user, OAL_TRUE);

    // 填充同步信息
    h2d_sync.back_to_old = OAL_FALSE;

    ret = hmac_config_roam_hmac_sync_dmac(roam_info->hmac_vap, &h2d_sync);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_resume_security_port::send event failed[%d].}", roam_info->hmac_vap->vap_id,
            ret);
    }

    return (osal_u32)ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_scan_timeout
 功能描述  : 扫描超时
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_scan_timeout(hmac_roam_info_stru *roam_info, osal_void *param)
{
    osal_u32               ul_ret;

    unref_param(param);

    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_timeout::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    ul_ret = hmac_roam_scan_complete_etc(roam_info->hmac_vap);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_scan_timeout::hmac_roam_scan_complete_etc failed[%d].}", ul_ret);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connecting_timeout
 功能描述  : roam 在尝试 connect 之后的错误处理。
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_connecting_timeout(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32       ul_ret;

    unref_param(p_param);
    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connecting_timeout::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    ul_ret = hmac_roam_to_old_bss(roam_info);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM,
                         "{hmac_roam_handle_fail_connect_phase:: hmac_roam_to_old_bss fail[%d]!}", ul_ret);
    }

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ul_ret = hmac_roam_resume_user_etc(roam_info->hmac_vap);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM,
                       "{hmac_roam_handle_fail_connect_phase:: hmac_fsm_call_func_sta_etc fail[%d]!}", ul_ret);
    }

    hmac_roam_main_clear(roam_info);

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connecting_fail
 功能描述  : roam 在尝试 connect 之后的错误处理。
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_connecting_fail(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32 ret;
    oal_bool_enum_uint8 is_protected;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connecting_fail::check_state fail[%d]!}", ret);
        return ret;
    }

    hmac_roam_main_del_timer(roam_info);
    ret =  hmac_roam_connecting_timeout(roam_info, p_param);

    /* auth/assoc被拒绝的处理. 漫游时auth被拒绝不删除用户 */
    if ((roam_info->connect.auth_num >= MAX_AUTH_CNT) && (roam_info->connect.state == ROAM_CONNECT_STATE_INIT)) {
        oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_connecting_fail::auth failed, auth_num=[%d], conn_state=[%d]}",
            roam_info->connect.auth_num, roam_info->connect.state);
        return ret;
    }

    if (roam_info->connect.status_code == MAC_REJECT_TEMP) {
        return ret;
    }

    /* 管理帧加密是否开启 */
    is_protected = roam_info->hmac_user->cap_info.pmf_active;

    /* roam_info结构体会在del user的时候释放掉，提前取出hmac_vap以便后续使用 */
    hmac_vap = roam_info->hmac_vap;

    /* 发去关联帧 */
    hmac_mgmt_send_disassoc_frame_etc(hmac_vap,
        roam_info->hmac_user->user_mac_addr, MAC_DEAUTH_LV_SS, is_protected);
    oam_warning_log1(0, OAM_SF_DFT, "hmac_roam_connecting_fail:send DISASSOC,err code[%d]", MAC_DEAUTH_LV_SS);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(roam_info->hmac_user->vap_id, roam_info->hmac_user->user_mac_addr,
            ROAM_CONNECTING_FAIL, USER_CONN_OFFLINE);
    }

    /* 删除对应用户 */
    hmac_user_del_etc(hmac_vap, roam_info->hmac_user);

    hmac_sta_handle_disassoc_rsp_etc(hmac_vap, MAC_NOT_ASSOCED);

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_roam_handle_fail_handshake_phase
 功能描述  : roam 在尝试 handshake 之后的错误处理。
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_handle_fail_handshake_phase(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    osal_u32 ret;
    oal_bool_enum_uint8 is_protected;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    unref_param(p_param);
    ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_handle_fail_handshake_phase::check_state fail[%d]!}", ret);
        return ret;
    }

    /* roam_info结构体会在del user的时候释放掉，提前取出hmac_vap以便后续使用 */
    hmac_vap = roam_info->hmac_vap;

    ret = hmac_roam_to_old_bss(roam_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ROAM,
                         "vap_id[%d] {hmac_roam_handle_fail_handshake_phase:: hmac_roam_to_old_bss fail[%d]!}",
                         hmac_vap->vap_id, ret);
    }

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ret = hmac_roam_resume_user_etc(hmac_vap);

    hmac_roam_main_clear(roam_info);
    hmac_roam_main_del_timer(roam_info);

    /* 为提高漫游成功的概率，在握手失败时触发立即重新漫游 */
    if (roam_info->alg.c_current_rssi > ROAM_RSSI_MAX_TYPE) {
        /* 如果是弱信号触发的漫游，先把rssi修改成ROAM_RSSI_LINKLOSS_TYPE来将弱信号跟LINKLOSS触发的重漫游归一 */
        if (roam_info->alg.c_current_rssi > ROAM_RSSI_LINKLOSS_TYPE) {
            roam_info->alg.c_current_rssi = ROAM_RSSI_LINKLOSS_TYPE;
        }
        /* 漫游握手失败时，rssi 逐次减1dBm，一直到到ROAM_RSSI_MAX_TYPE。这样可以最多触发5次重漫游 */
        return hmac_roam_trigger_handle_etc(hmac_vap, roam_info->alg.c_current_rssi - 1,
            OAL_FALSE);
    }

    oam_warning_log1(0, OAM_SF_ROAM,
        "{hmac_roam_handle_fail_handshake_phase:: report deauth to wpas! c_current_rssi=%d}",
        roam_info->alg.c_current_rssi);

    /* 管理帧加密是否开启 */
    is_protected = roam_info->hmac_user->cap_info.pmf_active;

    /* 发去关联帧 */
    hmac_mgmt_send_disassoc_frame_etc(hmac_vap,
        roam_info->hmac_user->user_mac_addr, MAC_DEAUTH_LV_SS, is_protected);
    oam_warning_log1(0, OAM_SF_DFT, "hmac_roam_handle_fail_handshake_phase:send DISASSOC,err code[%d]",
        MAC_DEAUTH_LV_SS);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(roam_info->hmac_user->vap_id, roam_info->hmac_user->user_mac_addr,
            ROAM_HANDSHAKE_FAIL, USER_CONN_OFFLINE);
    }
    /* 删除对应用户 */
    hmac_user_del_etc(hmac_vap, roam_info->hmac_user);

    hmac_sta_handle_disassoc_rsp_etc(hmac_vap, MAC_4WAY_HANDSHAKE_TIMEOUT);

    return ret;
}

static osal_void hmac_roam_save_bss(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_roam_info_stru *roam_info)
{
    hmac_roam_old_bss_stru *old_bss = &roam_info->old_bss;

    old_bss->sta_aid = hmac_vap->sta_aid;
    old_bss->protocol_mode = hmac_vap->protocol;
    (osal_void)memcpy_s(&old_bss->cap_info, OAL_SIZEOF(mac_user_cap_info_stru), &hmac_user->cap_info,
        OAL_SIZEOF(mac_user_cap_info_stru));
    (osal_void)memcpy_s(&old_bss->key_info, OAL_SIZEOF(mac_key_mgmt_stru), &(hmac_user->key_info),
        OAL_SIZEOF(mac_key_mgmt_stru));
    (osal_void)memcpy_s(&old_bss->user_tx_info, OAL_SIZEOF(mac_user_tx_param_stru), &(hmac_user->user_tx_info),
        OAL_SIZEOF(mac_user_tx_param_stru));
    (osal_void)memcpy_s(&old_bss->mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru), hmac_vap->mib_info,
        OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
    (osal_void)memcpy_s(&old_bss->op_rates, OAL_SIZEOF(mac_rate_stru), &(hmac_user->op_rates),
        OAL_SIZEOF(mac_rate_stru));
    (osal_void)memcpy_s(&old_bss->ht_hdl, OAL_SIZEOF(mac_user_ht_hdl_stru), &(hmac_user->ht_hdl),
        OAL_SIZEOF(mac_user_ht_hdl_stru));
    (osal_void)memcpy_s(&old_bss->vht_hdl, OAL_SIZEOF(mac_vht_hdl_stru), &(hmac_user->vht_hdl),
        OAL_SIZEOF(mac_vht_hdl_stru));
    old_bss->avail_bandwidth = hmac_user->avail_bandwidth;
    old_bss->cur_bandwidth = hmac_user->cur_bandwidth;
    (osal_void)memcpy_s(&old_bss->st_channel, OAL_SIZEOF(mac_channel_stru), &(hmac_vap->channel),
        OAL_SIZEOF(mac_channel_stru));
    (osal_void)memcpy_s(&old_bss->bssid, WLAN_MAC_ADDR_LEN, &(hmac_vap->bssid), WLAN_MAC_ADDR_LEN);
    old_bss->us_cap_info = hmac_vap->assoc_user_cap_info;
    return;
}

OAL_STATIC osal_u32  hmac_roam_connect_to_bss_check(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    osal_u32 ul_ret;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    hmac_vap_stru *other_vap = OAL_PTR_NULL;
    osal_u8 vap_idx;

    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_SCANING);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    hmac_device = (hmac_device_stru *)hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_connect_to_bss::hmac_device ptr is null!}\r\n",
                       hmac_vap->vap_id);
        return -OAL_EINVAL;
    }

    /* check all vap state in case other vap is connecting now */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        other_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (other_vap == OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_connect_to_bss::vap is null!}",
                             hmac_device->vap_id[vap_idx]);
            continue;
        }

        if ((other_vap->vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) &&
            (other_vap->vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC)) {
            oam_warning_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_connect_to_bss::vap is connecting, state=[%d]!}",
                             other_vap->vap_id,
                             other_vap->vap_state);
            hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_INIT);
            return -OAL_EFAIL;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_connect_to_bss
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32  hmac_roam_connect_to_bss(hmac_roam_info_stru *roam_info, osal_void *p_param)
{
    hmac_vap_stru           *hmac_vap  = roam_info->hmac_vap;
    hmac_user_stru          *hmac_user = roam_info->hmac_user;
    mac_bss_dscr_stru       *bss_dscr  = (mac_bss_dscr_stru *)p_param;
    osal_u32 ul_ret;
    oal_bool_enum_uint8 need_to_stop_user = OSAL_TRUE;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_ROAM_CONNECT_TO_BSS);

    ul_ret = hmac_roam_connect_to_bss_check(hmac_vap, roam_info);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    if (fhook != OSAL_NULL) {
        ((hmac_11r_roam_connect_to_bss_cb)fhook)(hmac_vap, roam_info, &need_to_stop_user);
    }

    if (need_to_stop_user != OSAL_FALSE) {
        /* 切换vap的状态为ROAMING，将用户节能，暂停发送 */
        ul_ret = hmac_roam_pause_user_etc(hmac_vap);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ROAM,
                "{hmac_roam_connect_to_bss::hmac_fsm_call_func_sta_etc fail[%ld]!}", ul_ret);
            return ul_ret;
        }
    }

    /* 原bss信息保存，以便回退 */
    hmac_roam_save_bss(hmac_vap, hmac_user, roam_info);

    /* 切换状态至connecting */
    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_CONNECTING);

    roam_info->st_static.connect_cnt++;

    /* 设置漫游到的bss能力位，重关联请求使用 */
    hmac_vap->assoc_user_cap_info = bss_dscr->us_cap_info;

    /* 启动connect状态机 */
    ul_ret = hmac_roam_connect_start_etc(hmac_vap, bss_dscr);
    if (ul_ret != OAL_SUCC && ul_ret != OAL_CONTINUE) {
        oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_connect_to_bss::hmac_roam_connect_start_etc fail[%ld]!}", ul_ret);
        hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_FAIL);
        return ul_ret;
    }

    /* 启动connect超时定时器 */
    hmac_roam_main_start_timer(roam_info, ROAM_CONNECT_TIME_MAX);

    return OAL_SUCC;
}

static osal_void hmac_roam_restore_bss(hmac_vap_stru *hmac_vap, hmac_roam_old_bss_stru *old_bss,
    hmac_user_stru *hmac_user)
{
    hmac_vap->sta_aid  = old_bss->sta_aid;
    hmac_vap->protocol = old_bss->protocol_mode;
    (osal_void)memcpy_s(&(hmac_user->cap_info), OAL_SIZEOF(mac_user_cap_info_stru), &old_bss->cap_info,
        OAL_SIZEOF(mac_user_cap_info_stru));
    (osal_void)memcpy_s(&(hmac_user->key_info), OAL_SIZEOF(mac_key_mgmt_stru), &old_bss->key_info,
        OAL_SIZEOF(mac_key_mgmt_stru));
    (osal_void)memcpy_s(&(hmac_user->user_tx_info), OAL_SIZEOF(mac_user_tx_param_stru), &old_bss->user_tx_info,
        OAL_SIZEOF(mac_user_tx_param_stru));
    (osal_void)memcpy_s(hmac_vap->mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru), &old_bss->mib_info,
        OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
    (osal_void)memcpy_s(&(hmac_user->op_rates), OAL_SIZEOF(mac_rate_stru), &old_bss->op_rates,
        OAL_SIZEOF(mac_rate_stru));
    (osal_void)memcpy_s(&(hmac_user->ht_hdl), OAL_SIZEOF(mac_user_ht_hdl_stru), &old_bss->ht_hdl,
        OAL_SIZEOF(mac_user_ht_hdl_stru));
    (osal_void)memcpy_s(&(hmac_user->vht_hdl), OAL_SIZEOF(mac_vht_hdl_stru), &old_bss->vht_hdl,
        OAL_SIZEOF(mac_vht_hdl_stru));
    hmac_user->avail_bandwidth = old_bss->avail_bandwidth;
    hmac_user->cur_bandwidth   = old_bss->cur_bandwidth;
    (osal_void)memcpy_s(&(hmac_vap->channel), OAL_SIZEOF(mac_channel_stru), &old_bss->st_channel,
        OAL_SIZEOF(mac_channel_stru));
    (osal_void)memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN, old_bss->bssid, WLAN_MAC_ADDR_LEN);
    hmac_vap->assoc_user_cap_info = old_bss->us_cap_info;
    return;
}

static osal_u32 hmac_roam_to_old_bss_sync_event(hmac_vap_stru *hmac_vap, hmac_roam_old_bss_stru *old_bss,
    hmac_user_stru *hmac_user)
{
    mac_h2d_roam_sync_stru *h2d_sync = OAL_PTR_NULL;
    osal_s32 ret;

    h2d_sync = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_h2d_roam_sync_stru), OAL_TRUE);
    if (h2d_sync == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_to_old_bss::no buff to alloc sync info!}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    // 填充同步信息
    h2d_sync->back_to_old = OAL_TRUE;
    h2d_sync->sta_aid = old_bss->sta_aid;
    (osal_void)memcpy_s(&(h2d_sync->channel), OAL_SIZEOF(mac_channel_stru), &old_bss->st_channel,
        OAL_SIZEOF(mac_channel_stru));
    (osal_void)memcpy_s(&(h2d_sync->cap_info), OAL_SIZEOF(mac_user_cap_info_stru), &old_bss->cap_info,
        OAL_SIZEOF(mac_user_cap_info_stru));
    (osal_void)memcpy_s(&(h2d_sync->key_info), OAL_SIZEOF(mac_key_mgmt_stru), &old_bss->key_info,
        OAL_SIZEOF(mac_key_mgmt_stru));
    (osal_void)memcpy_s(&(h2d_sync->user_tx_info), OAL_SIZEOF(mac_user_tx_param_stru), &old_bss->user_tx_info,
        OAL_SIZEOF(mac_user_tx_param_stru));

    /* 在漫游过程中可能又建立了聚合，因此回退时需要删除掉 */
    hmac_tid_clear_etc(hmac_vap, hmac_user, OSAL_FALSE);

    ret = hmac_config_roam_hmac_sync_dmac(hmac_vap, h2d_sync);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_to_old_bss::hmac_config_roam_hmac_sync_dmac failed[%d].}",
            hmac_vap->vap_id, ret);
    }

    /* 释放同步数据 */
    if (h2d_sync != OAL_PTR_NULL) {
        oal_mem_free(h2d_sync, OAL_TRUE);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PMF
/* 功能描述  : 更新vap中的PMF能力并同步到device */
OAL_STATIC osal_u32 hmac_config_vap_pmf_cap_etc(hmac_vap_stru *hmac_vap, wlan_pmf_cap_status_uint8 pmf_cap)
{
    mac_vap_pmf_mab_stru mac_vap_pmf_tbl[MAC_PMF_BUTT] = {
        { OSAL_FALSE, OSAL_FALSE },
        { OSAL_TRUE,  OSAL_FALSE },
        { OSAL_TRUE,  OSAL_TRUE },
    };

    mac_mib_set_dot11_rsnamfpc(hmac_vap, mac_vap_pmf_tbl[pmf_cap].mac_mib_dot11rsnamfpc);
    mac_mib_set_dot11_rsnamfpr(hmac_vap, mac_vap_pmf_tbl[pmf_cap].mac_mib_dot11rsnamfpr);

    hmac_vap->user_pmf_cap = (pmf_cap != MAC_PMF_DISABLED) ? OSAL_TRUE : OSAL_FALSE;

    oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG, "hmac_config_vap_pmf_cap_etc::vap pmf cap [%d]", pmf_cap);

    return OAL_SUCC;
}

/* 功能描述  : 漫游时，根据对端pmf能力更新vap的pmf能力 */
OAL_STATIC osal_void hmac_sta_roam_update_pmf_etc(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *mac_bss_dscr)
{
    osal_u16 rsn_cap_info;
    wlan_pmf_cap_status_uint8 pmf_cap;

    if (hmac_vap == OSAL_NULL || mac_bss_dscr == OSAL_NULL || hmac_vap->vap_state != MAC_VAP_STATE_ROAMING) {
        return;
    }

    rsn_cap_info = hmac_get_rsn_capability_etc(mac_bss_dscr->rsn_ie);
    pmf_cap = ((rsn_cap_info & BIT7) != 0) ? MAC_PMF_ENABLED : MAC_PMF_DISABLED;

    hmac_config_vap_pmf_cap_etc(hmac_vap, pmf_cap);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_roam_to_old_bss
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_to_old_bss(hmac_roam_info_stru *roam_info)
{
    osal_u32 ul_ret;
    hmac_vap_stru *hmac_vap = roam_info->hmac_vap;
    hmac_user_stru *hmac_user = roam_info->hmac_user;
    hmac_roam_old_bss_stru *old_bss = &roam_info->old_bss;

    if (old_bss == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_to_old_bss::old_bss null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info->st_static.roam_old_cnt++;
    roam_info->st_static.roam_eap_fail++;

    /* 恢复原来bss相关信息 */
    hmac_roam_restore_bss(hmac_vap, old_bss, hmac_user);

    /* 设置用户8021x端口合法性的状态为合法 */
    hmac_user_set_port_etc(hmac_user, OAL_TRUE);

    ul_ret = hmac_config_start_vap_event_etc(hmac_vap, OAL_FALSE);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_to_old_bss::hmac_config_start_vap_event_etc failed[%d].}",
            hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    /* 相关参数需要配置到dmac */
    hmac_roam_connect_set_join_reg_etc(hmac_vap, hmac_user);

    /* 更新用户的mac地址，漫游时mac会更新 */
    oal_set_mac_addr(hmac_user->user_mac_addr, hmac_vap->bssid);

#ifdef _PRE_WLAN_FEATURE_PMF
    /* 此函数要在同步user能力前调用，同步user能力后会开启硬件PMF使能 */
    hmac_config_vap_pmf_cap_etc(hmac_vap, old_bss->cap_info.pmf_active);
#endif

    ul_ret = hmac_config_user_info_syn_etc(hmac_vap, hmac_user);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_to_old_bss::hmac_syn_vap_state failed[%d].}",
            hmac_vap->vap_id, ul_ret);
    }

    /* 回退 bss 时，hmac 2 dmac 同步的相关信息，以便失败的时候回退 */
    ul_ret = hmac_roam_to_old_bss_sync_event(hmac_vap, old_bss, hmac_user);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    /* user已经关联上，抛事件给DMAC，在DMAC层挂用户算法钩子 */
    hmac_user_add_notify_alg_etc(hmac_vap, hmac_user->assoc_id);

    oam_warning_log4(0, OAM_SF_ROAM,
        "{hmac_roam_to_old_bss::now resuming to [%02X:%02X:%02X:%02X:XX:XX]}",
        old_bss->bssid[0], old_bss->bssid[1], /* 0 1:MAC地址 */
        old_bss->bssid[2], old_bss->bssid[3]); /* 2 3:MAC地址 */

    return ul_ret;
}

OAL_STATIC osal_u32 hmac_roam_sync_bssid_and_dbac_status(hmac_vap_stru *hmac_vap)
{
    frw_msg msg_info = {0};
    osal_u8 bssid[WLAN_MAC_ADDR_LEN] = {0};
    osal_u32 ret;
    osal_u8 dbac_running;
#ifdef _PRE_WLAN_FEATURE_DBAC
    hmac_device_stru *hmac_device = OSAL_NULL;
#endif

    (osal_void)memcpy_s((osal_void *)bssid, WLAN_MAC_ADDR_LEN, (osal_void *)(hmac_vap->bssid), WLAN_MAC_ADDR_LEN);

    frw_msg_init((osal_u8 *)bssid, WLAN_MAC_ADDR_LEN, &dbac_running, sizeof(osal_u8), &msg_info);
    ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_UPDATE_BSSID, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_roam_sync_bssid_and_dbac_status::sync msg fail[%d].}", ret);
    }

#ifdef _PRE_WLAN_FEATURE_DBAC
    /* dbac场景，漫游过程中start vap后，device侧dbac已经running，会发消息通知host侧，
    存在漫游成功后host侧还未处理该消息的情况，host侧dbac状态更新不及时，此时restart dbac会失败，
    所以在restart dbac之前查询device侧dbac状态并更新host侧的值 */
    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_roam_sync_bssid_and_dbac_status::device null}");
        return OAL_FAIL;
    }
    hmac_device->en_dbac_running = dbac_running;
#endif

    return ret;
}

#ifdef _PRE_WLAN_SUPPORT_5G
osal_void hmac_roam_to_diff_freq_band_proc(hmac_vap_stru *hmac_vap, hmac_roam_info_stru *roam_info)
{
    hmac_user_stru          *hmac_user = roam_info->hmac_user;
    wlan_roam_main_band_state_enum_uint8  roam_band_state;
    osal_void *fhook = NULL;

        /* 标识漫游类型，传到dmac做对应业务处理 */
    if (roam_info->old_bss.st_channel.band == WLAN_BAND_2G) {
        roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_2TO5;
        if (hmac_vap->channel.band == WLAN_BAND_2G) {
            roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_2TO2;
        }
    } else {
        roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_5TO5;
        if (hmac_vap->channel.band == WLAN_BAND_2G) {
            roam_band_state = WLAN_ROAM_MAIN_BAND_STATE_5TO2;
        }
    }

    /* 如果是从2g漫游到5g,需要清楚vap统计信息 */
    if ((roam_band_state == WLAN_ROAM_MAIN_BAND_STATE_2TO5) || (roam_band_state == WLAN_ROAM_MAIN_BAND_STATE_5TO2)) {
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_ROAM_SUCC_HANDLER);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_roam_succ_handler_cb)fhook)(hmac_vap);
        }
    }

    oam_warning_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_to_diff_freq_proc:: roam_band_state = %d!!}",
        hmac_vap->vap_id, roam_band_state);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_roam_to_new_bss
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_to_new_bss(hmac_roam_info_stru *roam_info, osal_void *param)
{
    hmac_vap_stru *hmac_vap  = roam_info->hmac_vap;
    osal_u32 ul_ret;

    unref_param(param);

    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_ROAMING, ROAM_MAIN_STATE_CONNECTING);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_to_new_bss::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    roam_info->st_static.roam_new_cnt++;
    hmac_roam_alg_add_history_etc(roam_info, hmac_vap->bssid);

    hmac_roam_main_change_state(roam_info, ROAM_MAIN_STATE_UP);

    hmac_roam_main_del_timer(roam_info);

    /* 切换vap的状态为UP，恢复用户节能，恢复发送 */
    ul_ret = hmac_roam_resume_user_etc(roam_info->hmac_vap);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_to_new_bss::hmac_fsm_call_func_sta_etc fail! erro code is %u}",
            hmac_vap->vap_id, ul_ret);
    }

    hmac_vap->roam_scan_valid_rslt = OAL_TRUE;
    hmac_vap->wpa3_roaming = OSAL_FALSE;

    wifi_printf("hmac_roam_to_new_bss::roam to [%02X:%02X:%02X:%02X:XX:XX] succ\r\n",
        hmac_vap->bssid[0], hmac_vap->bssid[1], /* 0 1:MAC地址 */
        hmac_vap->bssid[2], hmac_vap->bssid[3]); /* 2 3:MAC地址 */
    oam_warning_log4(0, OAM_SF_ROAM, "{hmac_roam_to_new_bss::roam to [%02X:%02X:%02X:%02X:XX:XX] succ}",
        hmac_vap->bssid[0], hmac_vap->bssid[1], /* 0 1:MAC地址 */
        hmac_vap->bssid[2], hmac_vap->bssid[3]); /* 2 3:MAC地址 */
    hmac_roam_main_clear(roam_info);

    /* 漫游成功同步bssid到dmac侧，否则dmac组帧会出现错误，
       dbac 发送ps=1的fcs one packet null帧，帧内容只在start vap时更新，
       所以需要重启dbac更新帧的内容 */
    hmac_roam_sync_bssid_and_dbac_status(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_DBAC
    hmac_vap_restart_dbac(hmac_vap);
#endif
#ifdef _PRE_WLAN_SUPPORT_5G
    hmac_roam_to_diff_freq_band_proc(hmac_vap, roam_info);
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_ignore_rssi_trigger_etc
 功能描述  : 暂停漫游rssi检测触发扫描，
             防止在弱信号下非漫游场景时频繁上报trigger事件
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32  hmac_roam_rssi_trigger_type(hmac_vap_stru *hmac_vap, roam_scenario_enum_uint8 val)
{
    hmac_roam_info_stru              *roam_info;
    osal_u8                         vap_id;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_rssi_trigger_type::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->vap_id;
    roam_info = hmac_get_roam_info(vap_id);
    if (roam_info->trigger.trigger_2g == OAL_RSSI_INIT_VALUE &&
        roam_info->trigger.trigger_5g == OAL_RSSI_INIT_VALUE) {
        return OAL_ERR_CODE_ROAM_DISABLED;
    }

    if (roam_info->rssi_type == val) {
        return OAL_SUCC;
    }

    if (val == ROAM_ENV_LINKLOSS) {
        roam_info->trigger.trigger_2g = ROAM_RSSI_LINKLOSS_TYPE;
        roam_info->trigger.trigger_5g = ROAM_RSSI_LINKLOSS_TYPE;
    } else if (val == ROAM_ENV_DENSE_AP) {
        roam_info->trigger.trigger_2g = roam_info->config.c_dense_env_trigger_rssi_2g;
        roam_info->trigger.trigger_5g = roam_info->config.c_dense_env_trigger_rssi_5g;
    } else {
        roam_info->trigger.trigger_2g = roam_info->config.c_trigger_rssi_2g;
        roam_info->trigger.trigger_5g = roam_info->config.c_trigger_rssi_5g;
    }
    /* 设置门限时，reset统计值，重新设置门限后，可以马上触发一次漫游 */
    roam_info->trigger.cnt = 0;
    roam_info->trigger.time_stamp = 0;

    oam_warning_log3(0, OAM_SF_ROAM, "vap_id[%d] {hmac_config_set_roam_trigger, trigger[%d, %d]}", hmac_vap->vap_id,
        roam_info->trigger.trigger_2g, roam_info->trigger.trigger_5g);
    roam_info->rssi_type = val;

    oam_warning_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_rssi_trigger_type::[%d]}", vap_id, val);
    return OAL_SUCC;
}

static osal_u32  hmac_roam_pause_user_check(hmac_vap_stru *hmac_vap)
{
    osal_u8 vap_id;
    oal_net_device_stru *net_device = OAL_PTR_NULL;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_pause_user_etc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->vap_id;

    net_device = hmac_vap->net_device;
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_pause_user_etc::net_device null!}", vap_id);
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 必须保证vap的状态是UP */
    if (hmac_vap->vap_state != MAC_VAP_STATE_UP) {
        oam_warning_log2(0, OAM_SF_ROAM,
                         "vap_id[%d] {hmac_roam_pause_user_etc::vap state = [%d] NOT FOR ROAMING!}", vap_id,
                         hmac_vap->vap_state);
        return OAL_ERR_CODE_ROAM_STATE_UNEXPECT;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_pause_user_etc
 功能描述  : 1、通知内核协议栈暂停数据发送，
             2、关闭低功耗，防止漫游期间进入低功耗模式
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32  hmac_roam_pause_user_etc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru *roam_info = OAL_PTR_NULL;
    oal_net_device_stru *net_device = OAL_PTR_NULL;
    osal_u32 ul_ret;
    osal_s32 ret;
    osal_u8 vap_id;
    osal_u8 roaming_mode = 1;

    ul_ret = hmac_roam_pause_user_check(hmac_vap);
    if (ul_ret != OAL_SUCC) {
        return ul_ret;
    }

    net_device = hmac_vap->net_device;
    vap_id = hmac_vap->vap_id;
    roam_info = hmac_get_roam_info(vap_id);
    /* 暂停所有协议层数据，这样就不需要再hmac搞一个缓存队列了 */
    oal_net_tx_stop_all_queues(net_device);
    oal_net_wake_subqueue(net_device, WLAN_HI_QUEUE);

    /* 清空 HMAC层TID信息 */
    hmac_tid_clear_etc(hmac_vap, roam_info->hmac_user, OSAL_FALSE);

#ifdef _PRE_WLAN_FEATURE_STA_PM
    ul_ret = hmac_config_set_pm_by_module_etc(hmac_vap, MAC_STA_PM_CTRL_TYPE_ROAM, MAC_STA_PM_SWITCH_OFF);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_roam_pause_user_etc::hmac_config_set_pm_by_module_etc failed[%d].}", vap_id, ul_ret);
        oal_net_tx_wake_all_queues(net_device);
        return ul_ret;
    }
#endif
    ret = hmac_config_roam_enable(hmac_vap, roaming_mode);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_roam_pause_user_etc::send event[WLAN_MSG_H2D_C_CFG_SET_ROAMING_MODE] failed[%d].}",
            vap_id, ret);
        oal_net_tx_wake_all_queues(net_device);
        return (osal_u32)ret;
    }

    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_ROAMING);

    oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_pause_user_etc::queues stopped!}", vap_id);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_resume_user_etc
 功能描述  : 1、通知内核协议栈恢复数据发送，
             2、使能低功耗
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32  hmac_roam_resume_user_etc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru              *roam_info;
    oal_net_device_stru              *net_device;
    osal_u32                        ul_ret;
    osal_u8                         vap_id;
    osal_u8                         roaming_mode;
    osal_s32 ret;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_resume_user_etc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->vap_id;
    roam_info = hmac_get_roam_info(vap_id);

    net_device = hmac_vap->net_device;
    if (net_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_resume_user_etc::net_device null!}", vap_id);
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    /* 必须保证vap的状态是roaming */
    if (hmac_vap->vap_state != MAC_VAP_STATE_ROAMING) {
        hmac_roam_resume_pm(roam_info);
        ul_ret = hmac_roam_resume_security_port(roam_info);
        if (ul_ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ROAM, "{hmac_roam_resume_user_etc::roam resume security failed[%d].}", ul_ret);
        }
        oal_net_tx_wake_all_queues(net_device);
        oam_warning_log2(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_roam_resume_user_etc::vap state[%d] NOT ROAMING!}", vap_id, hmac_vap->vap_state);
        return OAL_SUCC;
    }

    roaming_mode = 0;
    ret = hmac_config_roam_enable(hmac_vap, roaming_mode);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_roam_resume_user_etc::send event[WLAN_MSG_H2D_C_CFG_SET_ROAMING_MODE] failed[%d].}", ret);
    }

    hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_UP);

    hmac_roam_resume_pm(roam_info);

    ul_ret = hmac_roam_resume_security_port(roam_info);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ROAM,
            "{hmac_roam_resume_user_etc::hmac_roam_resume_security_port failed[%d].}", ul_ret);
    }

    oal_net_tx_wake_all_queues(net_device);

    oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_resume_user_etc::all_queues awake!}", vap_id);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_scan_complete_etc
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_scan_complete_etc(hmac_vap_stru *hmac_vap)
{
    hmac_roam_info_stru              *roam_info;
    hmac_device_stru                 *hmac_device;
    osal_u8                         vap_id;
    hmac_bss_mgmt_stru               *scan_bss_mgmt;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete_etc::vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    vap_id = hmac_vap->vap_id;
    roam_info = hmac_get_roam_info(vap_id);

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_scan_complete_etc::device null!}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    scan_bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);

    oam_info_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_scan_complete_etc::handling scan result!}", vap_id);
    return hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_SCAN_RESULT, (void *)scan_bss_mgmt);
}

/*****************************************************************************
 函 数 名  : hmac_roam_trigger_handle_etc
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_u32 hmac_roam_trigger_handle_etc(hmac_vap_stru *hmac_vap, osal_s8 c_rssi,
    oal_bool_enum_uint8 cur_bss_ignore)
{
    hmac_roam_info_stru     *roam_info;
    osal_u32               ul_ret;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle_etc::hmac_vap null!}");
        return OAL_ERR_CODE_ROAM_INVALID_VAP;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);

    /* 每次漫游前，刷新是否支持漫游到自己的参数 */
    roam_info->current_bss_ignore = cur_bss_ignore;
    roam_info->config.scan_orthogonal = ROAM_SCAN_CHANNEL_ORG_BUTT;
    roam_info->roam_trigger = ROAM_TRIGGER_DMAC;
    roam_info->connect.ft_force_air = OAL_FALSE;
    roam_info->connect.ft_failed = OAL_FALSE;

    ul_ret = hmac_roam_main_check_state(roam_info, MAC_VAP_STATE_UP, ROAM_MAIN_STATE_INIT);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_trigger_handle_etc::check_state fail[%d]!}", ul_ret);
        return ul_ret;
    }

    hmac_roam_alg_init_etc(roam_info, c_rssi);

    ul_ret = hmac_roam_main_fsm_action_etc(roam_info, ROAM_MAIN_FSM_EVENT_START, OAL_PTR_NULL);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_trigger_handle_etc::START fail[%d].}",
                         hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_roam_rx_mgmt
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_sta_roam_rx_mgmt_etc(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    if (hmac_vap == OSAL_NULL || *netbuf == OSAL_NULL || (hmac_vap->vap_state != MAC_VAP_STATE_ROAMING)) {
        return OAL_CONTINUE;
    }

    return hmac_roam_connect_rx_mgmt_etc(*netbuf, hmac_vap);
}
/*****************************************************************************
 函 数 名  : hmac_roam_add_key_done_etc
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_void  hmac_roam_add_key_done_etc(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 pairwise)
{
    if (hmac_vap == OSAL_NULL || pairwise != OSAL_FALSE) {
        return;
    }

    if ((!is_legacy_sta(hmac_vap)) ||
        (hmac_vap->vap_state != MAC_VAP_STATE_ROAMING && hmac_vap->wpa3_roaming != OSAL_TRUE)) {
        return;
    }
    hmac_roam_connect_key_done_etc(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_roam_wpas_connect_state_notify_etc
 功能描述  : wpas 关联状态通知函数
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_void hmac_roam_wpas_connect_state_notify_etc(hmac_vap_stru *hmac_vap,
    wpas_connect_state_enum_uint32 conn_state)
{
    hmac_roam_info_stru              *roam_info;

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_wpas_connect_state_notify_etc::vap null!}");
        return;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (roam_info == OAL_PTR_NULL) {
        return;
    }

    if (roam_info->connected_state == conn_state) {
        return;
    }

    oam_warning_log3(0, OAM_SF_ROAM,
                     "vap_id[%d] {hmac_roam_wpas_connect_state_notify_etc:: state changed: [%d]-> [%d]}",
                     hmac_vap->vap_id,
                     roam_info->connected_state, conn_state);

    roam_info->connected_state = conn_state;
    if (conn_state == WPAS_CONNECT_STATE_IPADDR_OBTAINED) {
        roam_info->ip_addr_obtained = OAL_TRUE;
    }
    if (conn_state == WPAS_CONNECT_STATE_IPADDR_REMOVED) {
        roam_info->ip_addr_obtained = OAL_FALSE;
    }

    roam_info->trigger.ip_addr_obtained = roam_info->ip_addr_obtained;
    roam_info->trigger.ip_obtain_stamp = (osal_u32)osal_get_time_stamp_ms();
    oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_roam_wpas_connect_state_notify_etc:: ip_addr_obtained = %d!!}",
        hmac_vap->vap_id, roam_info->ip_addr_obtained);
}

OAL_STATIC osal_s32 hmac_roam_set_ip_state(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    wpas_connect_state_enum_uint32 ip_state;

    if (msg == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ip_state = *(osal_u32 *)msg->data;
    hmac_roam_wpas_connect_state_notify_etc(hmac_vap, ip_state);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_framer_modify_bssid
 功能描述  : 修改帧的bssid
*****************************************************************************/
OAL_STATIC osal_void hmac_frame_modify_bssid(oal_netbuf_stru *netbuf, const osal_u8 *bssid)
{
    mac_ieee80211_qos_htc_frame_addr4_stru   *mac_hdr;
    mac_tx_ctl_stru                          *tx_ctl;
    osal_u8                                 is_tods;
    osal_u8                                 is_from_ds;

    mac_hdr = (mac_ieee80211_qos_htc_frame_addr4_stru *)oal_netbuf_tx_data(netbuf);
    tx_ctl  = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    is_tods    = mac_hdr_get_to_ds((osal_u8 *)mac_hdr);
    is_from_ds = mac_hdr_get_from_ds((osal_u8 *)mac_hdr);
    /*************************************************************************/
    /*                  80211 MAC HEADER                                     */
    /* --------------------------------------------------------------------- */
    /* | To   | From |  ADDR1 |  ADDR2 | ADDR3  | ADDR3  | ADDR4  | ADDR4  | */
    /* | DS   |  DS  |        |        | MSDU   | A-MSDU |  MSDU  | A-MSDU | */
    /* ---------------------------------------------------------------------- */
    /* |  0   |   0  |  RA=DA |  TA=SA | BSSID  |  BSSID |   N/A  |   N/A  | */
    /* |  0   |   1  |  RA=DA |TA=BSSID|   SA   |  BSSID |   N/A  |   N/A  | */
    /* |  1   |   0  |RA=BSSID| RA=SA  |   DA   |  BSSID |   N/A  |   N/A  | */
    /* |  1   |   1  |  RA    |   TA   |   DA   |  BSSID |   SA   |  BSSID | */
    /*************************************************************************/
    if ((is_tods == 0) && (is_from_ds == 0)) {
        (osal_void)memcpy_s((osal_void *)(mac_hdr->qos_frame_addr4.address3), WLAN_MAC_ADDR_LEN,
            (osal_void *)(bssid), WLAN_MAC_ADDR_LEN);
        return;
    }

    if ((is_tods == 0) && (is_from_ds == 1)) {
        (osal_void)memcpy_s((osal_void *)(mac_hdr->qos_frame_addr4.address2), WLAN_MAC_ADDR_LEN,
            (osal_void *)(bssid), WLAN_MAC_ADDR_LEN);
        if (tx_ctl->is_amsdu == OSAL_TRUE) {
            (osal_void)memcpy_s((osal_void *)(mac_hdr->qos_frame_addr4.address3), WLAN_MAC_ADDR_LEN,
                (osal_void *)(bssid), WLAN_MAC_ADDR_LEN);
        }
        return;
    }

    if ((is_tods == 1) && (is_from_ds == 0)) {
        (osal_void)memcpy_s((osal_void *)(mac_hdr->qos_frame_addr4.address1), WLAN_MAC_ADDR_LEN,
            (osal_void *)(bssid), WLAN_MAC_ADDR_LEN);
        if (tx_ctl->is_amsdu == OSAL_TRUE) {
            (osal_void)memcpy_s((osal_void *)(mac_hdr->qos_frame_addr4.address3), WLAN_MAC_ADDR_LEN,
                (osal_void *)(bssid), WLAN_MAC_ADDR_LEN);
        }
        return;
    }

    if ((is_tods == 1) && (is_from_ds == 1)) {
        if (tx_ctl->is_amsdu == OSAL_TRUE) {
            (osal_void)memcpy_s((osal_void *)(mac_hdr->qos_frame_addr4.address3), WLAN_MAC_ADDR_LEN,
                (osal_void *)(bssid), WLAN_MAC_ADDR_LEN);
            (osal_void)memcpy_s((osal_void *)(mac_hdr->qos_frame_addr4.address4), WLAN_MAC_ADDR_LEN,
                (osal_void *)(bssid), WLAN_MAC_ADDR_LEN);
        }
        return;
    }

    return;
}
/*****************************************************************************
 函 数 名  : hmac_roam_update_framer
 功能描述  : 更新tid队列的报文
*****************************************************************************/
OAL_STATIC osal_u32  hmac_roam_update_framer(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hal_to_dmac_device_stru                *hal_device;
    hmac_tid_stru                          *tid_queue;
    oal_netbuf_stru                        *netbuf;
    osal_u16                              mpdu_num;
    osal_u8                               tid_idx;

    hal_device = hmac_vap->hal_device;

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_roam_update_framer::hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        osal_u32 irq_status;
        tid_queue = &hmac_user->tx_tid_queue[tid_idx];
        irq_status = frw_osal_irq_lock();
        if (oal_netbuf_list_empty(&tid_queue->buff_head) == OSAL_FALSE) {
            /* netbuf队列非空 */
            mpdu_num = 0;
            for (netbuf = tid_queue->buff_head.next;
                netbuf != (oal_netbuf_stru *)(&tid_queue->buff_head); netbuf = netbuf->next) {
                hmac_frame_modify_bssid(netbuf, hmac_user->user_mac_addr);
                mpdu_num++;
            }
            oam_warning_log2(0, OAM_SF_ROAM, "{hmac_roam_update_framer:: TID[%d]:%d mpdu is updated in buff_q.}",
                tid_idx, mpdu_num);
        }
        frw_osal_irq_restore(irq_status);
    }

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 清除ba会话
输入参数  : hmac_vap_stru *mac_vap
          osal_u8 len
          osal_u8 *param
返 回 值: osal_u32
*****************************************************************************/
OAL_STATIC osal_void hmac_roam_reset_ba(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    frw_msg msg_info = {0};
    osal_s32 ret;
    osal_u16 aid = hmac_user->assoc_id;
    hmac_tid_stru *tid_queue;
    osal_u8 tid_idx;

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_roam_reset_ba::hmac_vap is NULL.}");
    }

    /* 抛事件至Device侧DMAC，同步重置BA */
    frw_msg_init((osal_u8 *)&aid, sizeof(osal_u16), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_RESET_BA, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->vap_id, OAM_SF_CFG,
            "{hmac_roam_reset_ba::frw_send_msg_to_device failed[%d].}", ret);
    }

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        tid_queue = &hmac_user->tx_tid_queue[tid_idx];

        /* 释放BA相关的内容 */
        if (tid_queue->ba_rx_hdl.ba_conn_status == HMAC_BA_COMPLETE) {
            tid_queue->ba_rx_hdl.ba_conn_status = HMAC_BA_INIT;
            oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_reset_ba:: TID[%d]:reset rx ba.}", tid_idx);
        }

        /* 恢复TX方向Tid队列 */
        hmac_tid_resume(tid_queue, DMAC_TID_PAUSE_RESUME_TYPE_BA);
        oam_warning_log1(0, OAM_SF_ROAM, "{hmac_roam_reset_ba:: TID[%d]:reset tx ba.}", tid_idx);
    }

    return;
}

OAL_STATIC osal_void hmac_config_roam_enable_active(hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    osal_void *fhook = NULL;
#endif
    hmac_user_pause(hmac_user);
    hmac_roam_reset_ba(hmac_vap, hmac_user);

    hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_ROAMING);

    /* 通知算法用户下线，在dbac场景下减少在关联期间的信道切换操作 */
    hmac_vap_down_notify(hmac_vap);
    oam_warning_log1(0, OAM_SF_ROAM,
        "vap_id[%d] {hmac_config_roam_enable_active:: [MAC_VAP_STATE_UP]->[MAC_VAP_STATE_ROAMING]}", hmac_vap->vap_id);
    /* 漫游开始，需要重新出将legacy速率信息设为初始值 */
    hmac_vap_init_rate_info(&(hmac_vap->vap_curr_rate));
    hmac_vap_init_tx_frame_params(hmac_vap, OSAL_TRUE);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 需要临时暂停ps机制 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_SET_WLAN_PRIORITY);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_set_wlan_priority_cb)fhook)(hmac_vap, OSAL_TRUE, BTCOEX_PRIO_TIMEOUT_100MS);
    }
#endif
    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_ROAM_ACTIVE, hmac_vap);
}

/*****************************************************************************
功能描述  : 进入/退出漫游模式
输入参数  : hmac_vap_stru *mac_vap
          osal_u8 len
          osal_u8 *param
返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_roam_enable(hmac_vap_stru *hmac_vap, osal_u8 roaming_mode)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_void *fhook = NULL;
    hal_to_dmac_chip_stru *hal_chip = hmac_vap->hal_chip;
    if (hal_chip == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_config_roam_enable:: hal_chip null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_config_roam_enable::hmac_user[%d] null.}", hmac_vap->vap_id,
            hmac_vap->assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((roaming_mode == 1) &&
        (hmac_vap->vap_state == MAC_VAP_STATE_UP || hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)) {
        hmac_config_roam_enable_active(hmac_user, hmac_vap);
    } else if ((roaming_mode == 0) && (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING)) {
        hmac_roam_update_framer(hmac_vap, hmac_user);
        hmac_user_resume(hmac_user);

        oam_warning_log1(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_config_roam_enable:: [MAC_VAP_STATE_ROAMING]->[MAC_VAP_STATE_UP]}", hmac_vap->vap_id);
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);

        /* 漫游结束后，刷掉occupied_period以保证BT竞争到 */
        /* 需要临时暂停ps机制 */
        frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_ROAM_DEACTIVE, hmac_vap);
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_RECOVER_COEX_PRIORITY);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_recover_coex_priority_cb)fhook)(hmac_vap);
        }
    } else {
        oam_warning_log3(0, OAM_SF_ROAM, "vap_id[%d] {hmac_config_roam_enable::unexpect state[%d] or mode[%d]}",
            hmac_vap->vap_id, hmac_vap->vap_state, roaming_mode);
    }
    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : 漫游模式下更新 hmac info 到 dmac
输入参数  : hmac_vap_stru *mac_vap
          osal_u8 len
          osal_u8 *param
返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_roam_hmac_sync_dmac(hmac_vap_stru *hmac_vap, mac_h2d_roam_sync_stru *sync_param)
{
    hmac_user_stru *hmac_user;
    osal_u8 old_chan_number, old_bandwidth;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL) {
        /* 漫游中会遇到 kick user 的情况，降低 log level */
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_roam_hmac_sync_dmac::hmac_user[%d] null.}",
            hmac_vap->vap_id, hmac_vap->assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (sync_param->back_to_old == OSAL_TRUE) {
        old_chan_number = hmac_vap->channel.chan_number;
        old_bandwidth = hmac_vap->channel.en_bandwidth;
        /* 恢复原来bss相关信息 */
        hmac_vap_set_aid_etc(hmac_vap, sync_param->sta_aid);
        (osal_void)memcpy_s(&(hmac_vap->channel), sizeof(mac_channel_stru),
            &sync_param->channel, sizeof(mac_channel_stru));
        (osal_void)memcpy_s(&(hmac_user->cap_info), sizeof(mac_user_cap_info_stru),
            &sync_param->cap_info, sizeof(mac_user_cap_info_stru));
        (osal_void)memcpy_s(&(hmac_user->key_info), sizeof(mac_key_mgmt_stru),
            &sync_param->key_info, sizeof(mac_key_mgmt_stru));
        (osal_void)memcpy_s(&(hmac_user->user_tx_info), sizeof(mac_user_tx_param_stru),
            &sync_param->user_tx_info, sizeof(mac_user_tx_param_stru));
        /* 在漫游过程中可能又建立了聚合，因此回退时需要删除掉 */
        hmac_user_pause(hmac_user);
        hmac_roam_reset_ba(hmac_vap, hmac_user);

        if (old_chan_number != hmac_vap->channel.chan_number || old_bandwidth != hmac_vap->channel.en_bandwidth) {
            hmac_vap_sync(hmac_vap);
        }
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        /* 判断aid有效性 */
        if (sync_param->sta_aid == 0 || sync_param->sta_aid >= 2008) { // sta aid为0或大于等于2008，返回错误退出。
            oam_error_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_config_roam_hmac_sync_dmac::aid invalid[%d]}",
                hmac_vap->vap_id, sync_param->sta_aid);
            return OAL_FAIL;
        }
        /* STA漫游回old bss时重新配置aid寄存器，防止被意外覆盖 */
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hal_set_mac_aid(hmac_vap->hal_vap, hmac_vap->sta_aid);
        }
#endif
    }
    /* 设置用户8021x端口合法性的状态为合法 */
    hmac_user_set_port_etc(hmac_user, OSAL_TRUE);
    oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_config_roam_hmac_sync_dmac:: Sync Done!!}", hmac_vap->vap_id);
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_roam_proc_connect_cmd(hmac_vap_stru *hmac_vap, mac_conn_param_stru *connect_param)
{
    if (hmac_vap == OSAL_NULL || connect_param == OSAL_NULL) {
        return OAL_CONTINUE;
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        /* 通知ROAM主状态机, ABORT Roaming FSM */
        hmac_roam_connect_complete_etc(hmac_vap, OAL_FAIL);

        /* After roam_to_old_bss, hmac_vap->vap_state should be MAC_VAP_STATE_UP,
         * hmac_roam_info_stru     *roam_info;
         * roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
         * roam_info->main_state and roam_info->connect.state should be 0 */
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_UP) {
        oal_bool_enum_uint8 bss_ignore = osal_memcmp(hmac_vap->bssid, connect_param->bssid, OAL_MAC_ADDR_LEN) == 0 ?
            OAL_TRUE : OAL_FALSE;
        /* support wpa_cli/wpa_supplicant reassociate function */
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]", hmac_vap->vap_id);
        oam_warning_log4(0, OAM_SF_CFG, "roaming AP with ressoc frame, %02X:%02X:%02X:%02X:XX:XX",
            /* 0:1:2:3:数组下标 */
            connect_param->bssid[0], connect_param->bssid[1], connect_param->bssid[2], connect_param->bssid[3]);
        return hmac_roam_start_etc(hmac_vap, ROAM_SCAN_CHANNEL_ORG_0, bss_ignore, ROAM_TRIGGER_APP);
    }

    return OAL_CONTINUE;
}

OAL_STATIC osal_s32 hmac_config_get_ip_status(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 ip_status;
    wpas_connect_state_enum_uint32 conn_state;

    if (osal_unlikely(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ROAM, "{hmac_config_get_ip_status::null ptr.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ip_status = (osal_u8)*(msg->data);
    conn_state = (ip_status == 1) ? WPAS_CONNECT_STATE_IPADDR_OBTAINED : WPAS_CONNECT_STATE_IPADDR_REMOVED;

    hmac_roam_wpas_connect_state_notify_etc(hmac_vap, conn_state);
    return OAL_SUCC;
}

OAL_STATIC osal_bool hmac_sta_roam_trigger_check(hmac_vap_stru *hmac_vap, osal_s8 trigger_rssi, osal_u32 *cur_time,
    osal_s8 *current_rssi)
{
    osal_u32 delta_time;
    osal_bool ret = OSAL_FALSE;
    hmac_roam_info_stru *roam_info = hmac_get_roam_info(hmac_vap->vap_id);

    if (*current_rssi >= trigger_rssi) {
        roam_info->trigger.cnt = 0;
    } else if (roam_info->trigger.cnt++ >= ROAM_TRIGGER_COUNT_THRESHOLD) {
        /* 连续若干次rssi小于门限时发起一次漫游  */
        /* 漫游触发事件需要满足上报最小间隔 */
        *cur_time = (osal_u32)osal_get_time_stamp_ms();
        delta_time = (osal_u32)osal_get_runtime(roam_info->trigger.time_stamp, *cur_time);
        if ((*current_rssi <= ROAM_TRIGGER_RSSI_NE80_DB && delta_time >= ROAM_TRIGGER_INTERVAL_10S) ||
            (*current_rssi <= ROAM_TRIGGER_RSSI_NE75_DB && delta_time >= ROAM_TRIGGER_INTERVAL_15S) ||
            (delta_time >= ROAM_TRIGGER_INTERVAL_20S)) {
            ret = OSAL_TRUE;
            roam_info->trigger.cnt = 0;
        }
    }

    /* WLAN0: LINK LOSS计数满足3/4的门限时发起一次漫游，rssi为0表示强制漫游  */
    if ((get_current_linkloss_cnt(hmac_vap) >= (get_current_linkloss_threshold(hmac_vap) >> 1) +
        /* 右移2位 */
        (get_current_linkloss_threshold(hmac_vap) >> 2)) &&  (roam_info->linkloss_roam_done == OSAL_FALSE)) {
        oam_warning_log3(0, OAM_SF_ROAM,
            "vap_id[%d] {hmac_sta_roam_trigger_check:: link cnt[%d] >= 3/4 threshold[%d],start roam scan.}",
            hmac_vap->vap_id,
            get_current_linkloss_cnt(hmac_vap), get_current_linkloss_threshold(hmac_vap));

        /* 计数超过门限3/4,防止重复发起漫游扫描 */
        roam_info->linkloss_roam_done = OSAL_TRUE;

        *current_rssi = ROAM_RSSI_LINKLOSS_TYPE;
        ret = OSAL_TRUE;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_sta_roam_trigger_event_send
 功能描述  : 判断漫游触发是否需要上报HMAC
*****************************************************************************/
OAL_STATIC osal_void hmac_sta_roam_trigger_event_send(hmac_vap_stru *hmac_vap)
{
    osal_u32 cur_time = 0;
    osal_s8 trigger_rssi;
    osal_u32 delta_time;
    osal_s8 current_rssi;
    hmac_roam_info_stru *roam_info;
    osal_u8 is_5g_enable;

    if (hmac_vap == OSAL_NULL || !is_legacy_sta(hmac_vap)) {
        return;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    is_5g_enable = (osal_u8)hmac_device_check_5g_enable(hmac_vap->device_id);
    /* ini中2.4G和5G漫游触发阈值都配置-128,表示关闭弱信号漫游 */
    if (roam_info == OSAL_NULL || roam_info->enable == OSAL_FALSE ||
        (roam_info->trigger.trigger_2g == OAL_RSSI_INIT_VALUE &&
        (roam_info->trigger.trigger_5g == OAL_RSSI_INIT_VALUE || is_5g_enable == OSAL_FALSE))) {
        return;
    }

    current_rssi = (osal_s8)oal_get_real_rssi(hmac_vap->query_stats.signal);
    trigger_rssi = (hmac_vap->channel.band == WLAN_BAND_5G) ?
        roam_info->trigger.trigger_5g : roam_info->trigger.trigger_2g;

    if (hmac_sta_roam_trigger_check(hmac_vap, trigger_rssi, &cur_time, &current_rssi) == OSAL_FALSE) {
        return;
    }

    /* 漫游触发需要与正常关联保持时间间隔，WIFI+切换频繁，与漫游冲突。正常关联，并且 IP ADDR 已获取 */
    delta_time = (osal_u32)osal_get_runtime(roam_info->trigger.ip_obtain_stamp, cur_time);
    if (roam_info->trigger.ip_addr_obtained == OSAL_FALSE || delta_time < ROAM_WPA_CONNECT_INTERVAL_TIME) {
        oam_warning_log2(0, OAM_SF_ROAM,
            "{hmac_sta_roam_trigger_event_send:: ip_addr_check, ip_addr_obtained = %d, delta_time = %d}",
            roam_info->trigger.ip_addr_obtained, delta_time);
        return;
    }

    hmac_roam_trigger_handle_etc(hmac_vap, current_rssi, OAL_FALSE);
    oam_warning_log2(0, OAM_SF_ROAM, "vap_id[%d] {hmac_sta_roam_trigger_event_send::D2H roam trigger SUCC, RSSI=%d}",
        hmac_vap->vap_id, current_rssi);

    roam_info->trigger.time_stamp = (osal_u32)osal_get_time_stamp_ms();
    return;
}

OAL_STATIC osal_void hmac_roam_change_app_ie(hmac_vap_stru *hmac_vap, app_ie_type_uint8 *app_ie_type,
    const osal_u8 *curr_bssid)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_ROAM_CHANGE_APP_IE);

    if (hmac_vap == OSAL_NULL) {
        return;
    }

    if (curr_bssid != OAL_PTR_NULL) {
        *app_ie_type = OAL_APP_REASSOC_REQ_IE;
        if (fhook != OSAL_NULL) {
            ((hmac_11r_roam_change_app_ie_cb)fhook)(hmac_vap, app_ie_type);
        }
    }
}

OAL_STATIC osal_void hmac_roam_update_user_mac(hmac_vap_stru *hmac_vap, hmac_ctx_join_req_set_reg_stru *reg_params)
{
    hmac_user_stru *hmac_user;

    if (hmac_vap == OSAL_NULL || reg_params == OSAL_NULL) {
        return;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user != OSAL_NULL) {
        /* 更新用户mac */
        (osal_void)memcpy_s((osal_void *)(hmac_user->user_mac_addr), WLAN_MAC_ADDR_LEN,
            (osal_void *)(reg_params->bssid), WLAN_MAC_ADDR_LEN);
        hmac_user_del_lut_info(hmac_user->lut_index);
        hmac_user_set_lut_info(hmac_vap->hal_device, hmac_user->lut_index, reg_params->bssid);
        hmac_user_sync(hmac_user);
    }
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 功能描述  :    设置三个漫游配置项的值:
            scenario_enable: 强信号漫游使能开关
            c_trigger_rssi_2g: 弱信号2.4G漫游门限
            c_trigger_rssi_5g: 弱信号5G漫游门限
*****************************************************************************/
OAL_STATIC osal_s32  hmac_config_set_roam_config(hmac_vap_stru *hmac_vap, hmac_roam_config_stru *roam_config)
{
    hmac_roam_info_stru *roam_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info = hmac_get_roam_info(hmac_vap->vap_id);
    if (osal_unlikely(roam_info == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_roam_config::null ptr, roam_info[%p]}",
            hmac_vap->vap_id, (uintptr_t)roam_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    roam_info->config.scenario_enable   = roam_config->scenario_enable;
    if (roam_config->c_trigger_rssi_2g < 0) {
        roam_info->config.c_trigger_rssi_2g = roam_config->c_trigger_rssi_2g;
        roam_info->trigger.trigger_2g = roam_config->c_trigger_rssi_2g;
        roam_info->trigger.cnt = 0;
    }

    if (roam_config->c_trigger_rssi_5g < 0) {
        roam_info->config.c_trigger_rssi_5g = roam_config->c_trigger_rssi_5g;
        roam_info->trigger.trigger_5g = roam_config->c_trigger_rssi_5g;
        roam_info->trigger.cnt = 0;
    }
    oam_warning_log4(0, OAM_SF_CFG,
        "vap_id[%d] {hmac_config_set_roam_config::scenario_enable=[%d], trigger_rssi_2G=[%d], trigger_rssi_5G=[%d]}",
        hmac_vap->vap_id,
        roam_info->config.scenario_enable, roam_info->trigger.trigger_2g, roam_info->trigger.trigger_5g);

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_roam_cfg(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_s32 ret;
    hmac_roam_config_stru roam_cofig = {0};
    osal_s32 value;

    /* 获取第1个参数 scenario_enable */
    ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_roam_cfg:: get scenario_enable return err_code [%d]!}", ret);
        return ret;
    }
    roam_cofig.scenario_enable = (oal_bool_enum_uint8)value;

    /* 获取第2个参数 c_trigger_rssi_2g */
    ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_roam_cfg:: get trigger_rssi_2G return err_code [%d]!}", ret);
        return ret;
    }
    roam_cofig.c_trigger_rssi_2g = (osal_s8)value;

    /* 获取第3个参数 c_trigger_rssi_5g */
    ret = hmac_ccpriv_get_one_arg_digit(&pc_param, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_roam_cfg:: get trigger_rssi_5G return err_code [%d]!}", ret);
        return ret;
    }
    roam_cofig.c_trigger_rssi_5g = (osal_s8)value;

    hmac_config_set_roam_config(hmac_vap, &roam_cofig);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_roam_cfg::return err code [%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC osal_void hmac_roam_info_init(hmac_vap_stru *hmac_vap)
{
    osal_u32 i;
    hmac_roam_info_stru *roam_info = hmac_get_roam_info(hmac_vap->vap_id);

        /* 参数初始化 */
#if defined(CONFIG_MIDDLEWARE_SUPPORT_NV)
    osal_u8 nv_roam_enable;
    osal_u16 nv_roam_enable_len = 0;
    osal_u32 nv_ret;

    nv_ret = uapi_nv_read(NV_ID_ROAM_ENABLE, sizeof(nv_roam_enable), &nv_roam_enable_len, &nv_roam_enable);
    if (nv_ret == OAL_SUCC) {
        roam_info->enable = nv_roam_enable;
        wifi_printf("hmac_roam_info_init nv roam_enable[%d]\r\n", nv_roam_enable);
    }
#elif defined(_PRE_PLAT_FEATURE_CUSTOMIZE)
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
    roam_info->enable = wlan_customize_etc->roam_switch;
#else
    roam_info->enable = 1;
#endif
    roam_info->roam_trigger = ROAM_TRIGGER_DMAC;
    roam_info->main_state = ROAM_MAIN_STATE_INIT;
    roam_info->current_bss_ignore = OAL_FALSE;
    roam_info->hmac_vap = hmac_vap;
    roam_info->hmac_user = OAL_PTR_NULL;
    roam_info->connected_state = WPAS_CONNECT_STATE_INIT;

    hmac_roam_init_config(hmac_vap, roam_info);

    for (i = 0; i < ROAM_LIST_MAX; i++) {
        roam_info->alg.history.bss[i].count_limit = ROAM_HISTORY_COUNT_LIMIT;
        roam_info->alg.history.bss[i].timeout = ROAM_HISTORY_BSS_TIME_OUT;
    }

    for (i = 0; i < ROAM_LIST_MAX; i++) {
        roam_info->alg.blacklist.bss[i].count_limit = ROAM_BLACKLIST_COUNT_LIMIT;
        roam_info->alg.blacklist.bss[i].timeout = ROAM_BLACKLIST_NORMAL_AP_TIME_OUT;
    }
}

/*****************************************************************************
 函 数 名  : hmac_roam_init_etc
 功能描述  : roam模块控制信息初始化
 输入参数  : hmac_vap 需要初始化roam模块的vap，默认参数配置
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_bool hmac_roam_init_etc(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_vap_stru *hmac_vap;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u8 vap_id;

    if (hmac_user == OSAL_NULL) {
        return OSAL_FALSE;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    vap_id = hmac_vap->vap_id;
    if (g_roam_info[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] hmac_roam_init_etc mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_roam_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ROAM, "vap_id[%d] hmac_roam_init_etc malloc null!", vap_id);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_roam_info_stru), 0, sizeof(hmac_roam_info_stru));
    g_roam_info[vap_id] = (hmac_roam_info_stru *)mem_ptr;

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_INIT, hmac_vap);

    hmac_roam_info_init(hmac_vap);

    hmac_roam_fsm_init();
    hmac_roam_connect_fsm_init_etc();

    oam_warning_log1(0, OAM_SF_ROAM,
        "vap_id[%d] {hmac_roam_init_etc::SUCC.}", hmac_vap->vap_id);
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_roam_exit_etc
 功能描述  : roam模块控制信息卸载
 输入参数  : hmac_vap 需要卸载roam模块的vap
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_bool hmac_roam_exit_etc(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_vap_stru *hmac_vap;
    osal_u8 vap_id;

    if (hmac_user == OSAL_NULL) {
        return OSAL_FALSE;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (!is_legacy_vap(hmac_vap)) {
        return OSAL_TRUE;
    }
    vap_id = hmac_vap->vap_id;
    if (g_roam_info[vap_id] == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_exit_etc::pul_roam_info is NULL.}",
                         hmac_vap->vap_id);
        return OSAL_TRUE;
    }

    hmac_roam_show_etc(hmac_vap);

    frw_destroy_timer_entry(&(g_roam_info[vap_id]->connect.timer));
    hmac_roam_main_del_timer(g_roam_info[vap_id]);
    if (hmac_vap->net_device != OAL_PTR_NULL) {
        oal_net_tx_wake_all_queues(hmac_vap->net_device);
    }

    hmac_roam_connect_fsm_deinit_etc();
    hmac_roam_fsm_deinit();

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_11V_VAP_ROAM_INFO_DEINIT, hmac_vap);
    oal_mem_free(g_roam_info[vap_id], OAL_TRUE);
    g_roam_info[vap_id] = OAL_PTR_NULL;

    oam_warning_log1(0, OAM_SF_ROAM, "vap_id[%d] {hmac_roam_exit_etc::SUCC.}",
        hmac_vap->vap_id);

    return OSAL_TRUE;
}

hmac_netbuf_hook_stru roam_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_FEATURE,
    .priority = HMAC_HOOK_PRI_HIGH,
    .hook_func = hmac_sta_roam_rx_mgmt_etc,
};

osal_u32 hmac_roam_sta_init(osal_void)
{
    osal_u32 ret;

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_register((const osal_s8 *)"roam_cfg", hmac_ccpriv_roam_cfg);
#endif

    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_roam_init_etc);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_roam_exit_etc);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_RX_ASSOC_RSP, hmac_roam_info_init_etc);

    frw_msg_hook_register(WLAN_MSG_W2H_CFG_GET_IP_STATUS, hmac_config_get_ip_status);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_IP_STATE, hmac_roam_set_ip_state);

    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_START, hmac_roam_start_etc);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_GET_INFO, hmac_get_roam_info);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_PROC_CONNECT_CMD, hmac_roam_proc_connect_cmd);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_CHECK_BKSCAN, hmac_roam_check_bkscan_result_etc);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_ADD_KEY_DONE, hmac_roam_add_key_done_etc);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_UPDATE_PMF, hmac_sta_roam_update_pmf_etc);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_TRIGGER_SEND, hmac_sta_roam_trigger_event_send);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_CHANGE_APP_IE, hmac_roam_change_app_ie);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_UPDATE_USER_MAC, hmac_roam_update_user_mac);
    hmac_feature_hook_register(HMAC_FHOOK_ROAM_SET_LINKLOSS_DONE, hmac_roam_set_linkloss_done);

    ret = hmac_register_netbuf_hook(&roam_netbuf_hook);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_roam_sta_init:: MGMT RX IN register_netbuf_hooks error!");
        return ret;
    }

    return OAL_SUCC;
}

osal_void hmac_roam_sta_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_unregister((const osal_s8 *)"roam_cfg");
#endif

    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_roam_init_etc);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_roam_exit_etc);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_RX_ASSOC_RSP, hmac_roam_info_init_etc);

    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_GET_IP_STATUS);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_IP_STATE);

    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_START);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_GET_INFO);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_PROC_CONNECT_CMD);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_CHECK_BKSCAN);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_ADD_KEY_DONE);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_UPDATE_PMF);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_TRIGGER_SEND);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_CHANGE_APP_IE);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_UPDATE_USER_MAC);
    hmac_feature_hook_unregister(HMAC_FHOOK_ROAM_SET_LINKLOSS_DONE);

    hmac_unregister_netbuf_hook(&roam_netbuf_hook);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
