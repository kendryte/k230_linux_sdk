/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_fsm.c
 * 生成日期   : 2013年6月18日
 * 功能描述   : 全局静态状态机二维函数表
 */

#include "hmac_fsm.h"
#include "frw_ext_if.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_scan.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "hmac_config.h"
#include "hmac_feature_dft.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_FSM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 全局状态机函数表 */
OAL_STATIC hmac_fsm_func g_pa_hmac_ap_fsm_func[MAC_VAP_AP_STATE_BUTT][HMAC_FSM_AP_INPUT_TYPE_BUTT];
OAL_STATIC hmac_fsm_func g_pa_hmac_sta_fsm_func[MAC_VAP_STA_STATE_BUTT][HMAC_FSM_STA_INPUT_TYPE_BUTT];

/*****************************************************************************
 函 数 名  : hmac_fsm_set_state
 功能描述  : 改变状态机状态
 输入参数  : hmac_vap: HMAC VAP
             vap_state: 要切换到的状态
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年6月19日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void hmac_fsm_change_state_etc(hmac_vap_stru *hmac_vap, mac_vap_state_enum_uint8 vap_state)
{
    mac_cfg_mode_param_stru  cfg_mode;
    mac_vap_state_enum_uint8 old_state;
    osal_s32 ret;
    old_state = hmac_vap->vap_state;

    oam_warning_log2(0, 0, "hmac_fsm_change_state_etc: ======= state %d vap %d=========", vap_state, hmac_vap->vap_id);
    wifi_printf("%s state %d, vap_id %d\r\n", __func__, vap_state, hmac_vap->vap_id);

    ret = hmac_config_vap_state_syn(hmac_vap, vap_state);
    if (ret != OAL_SUCC) {
        oam_error_log4(0, 0,
            "vap_id[%d] {hmac_fsm_change_state_etc::hmac_syn_vap_state failed[%d], old_state=%d, new_state=%d.}",
            hmac_vap->vap_id, ret, old_state, vap_state);
    }

    if ((vap_state == MAC_VAP_STATE_STA_FAKE_UP) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA)) {
        cfg_mode.protocol  = hmac_vap->preset_para.protocol;
        cfg_mode.band      = hmac_vap->preset_para.band;
        cfg_mode.en_bandwidth = hmac_vap->preset_para.en_bandwidth;

        /* 信道需要初始化，避免协议带宽与信道不一致情况 */
        hmac_vap->channel.chan_number = 0;

        hmac_config_sta_update_rates_etc(hmac_vap, &cfg_mode, OAL_PTR_NULL);
    }
}

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_fsm_null_fn_etc
 功能描述  : 空函数
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年6月18日
    修改内容   : 新生成函数
*****************************************************************************/
OAL_STATIC osal_u32 hmac_fsm_null_fn_etc(hmac_vap_stru *hmac_vap, osal_void *p_param)
{
    /* 什么都不做 */
    unref_param(hmac_vap);
    unref_param(p_param);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_fsm_init_ap
 功能描述  : 初始化ap状态机函数表
 修改历史      :
  1.日    期   : 2013年6月19日
    修改内容   : 新生成函数
*****************************************************************************/
OAL_STATIC osal_void hmac_fsm_init_ap(osal_void)
{
    osal_u32  state;
    osal_u32  input;

    for (state = 0; state < MAC_VAP_AP_STATE_BUTT; state++) {
        for (input = 0; input < HMAC_FSM_AP_INPUT_TYPE_BUTT; input++) {
            g_pa_hmac_ap_fsm_func[state][input] = hmac_fsm_null_fn_etc;
        }
    }

    /* 接收管理帧输入
    +----------------------------------+---------------------
     | FSM State                        | FSM Function
     +----------------------------------+---------------------
     | MAC_VAP_STATE_INIT               | null_fn
     | MAC_VAP_STATE_UP                 | hmac_ap_up_rx_mgmt_etc
     | MAC_VAP_STATE_PAUSE              | null_fn
     +----------------------------------+---------------------
    */

    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_AP_WAIT_START][HMAC_FSM_INPUT_MISC] = hmac_ap_wait_start_misc_etc;
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_RX_MGMT]         = hmac_ap_up_rx_mgmt_etc;
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_MISC]            = hmac_ap_up_misc_etc;

    /* hmac不区分vap状态是否为pause,都要处理，避免丢帧 */
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_PAUSE][HMAC_FSM_INPUT_RX_MGMT]      = hmac_ap_up_rx_mgmt_etc;
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_PAUSE][HMAC_FSM_INPUT_MISC]         = hmac_ap_wait_start_misc_etc;
#ifdef _PRE_WLAN_FEATURE_P2P
    /* GO 扫描时接收管理帧场景 */
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_LISTEN][HMAC_FSM_INPUT_RX_MGMT]        = hmac_ap_up_rx_mgmt_etc;
    /* GO 监听的场景 */
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_LISTEN][HMAC_FSM_INPUT_LISTEN_REQ]     = hmac_p2p_remain_on_channel_etc;
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_LISTEN_REQ]         = hmac_p2p_remain_on_channel_etc;
    /* GO 监听结束的场景 */
    g_pa_hmac_ap_fsm_func[MAC_VAP_STATE_LISTEN][HMAC_FSM_INPUT_LISTEN_TIMEOUT] = hmac_p2p_listen_timeout_etc;
#endif
}

OAL_STATIC osal_void hmac_fsm_init_sta_scan_table(osal_void)
{
    osal_u32 state;
    /* 初始化扫描请求输入函数 HMAC_FSM_INPUT_SCAN_REQ
     +----------------------------------+---------------------
     | FSM State                        | FSM Function
     +----------------------------------+---------------------
     | MAC_VAP_STATE_INIT               | null_fn
     | MAC_VAP_STATE_UP                 | null_fn
     | MAC_VAP_STATE_FAKE_UP            | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_SCAN          | null_fn
     | MAC_VAP_STATE_SCAN_COMP          | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_JOIN          | null_fn
     | MAC_VAP_STATE_JOIN_COMP          | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | null_fn
     | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | null_fn
     | MAC_VAP_STATE_AUTH_COMP          | hmac_mgmt_scan_req
     | MAC_VAP_STATE_WAIT_ASOC          | null_fn
     | MAC_VAP_STATE_OBSS_SCAN          | null_fn
     | MAC_VAP_STATE_BG_SCAN            | null_fn
     +----------------------------------+---------------------
    */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event_exception_etc;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_INIT][HMAC_FSM_INPUT_SCAN_REQ]          = hmac_scan_proc_scan_req_event_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_SCAN_REQ]   = hmac_scan_proc_scan_req_event_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_JOIN_COMP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_AUTH_COMP][HMAC_FSM_INPUT_SCAN_REQ] = hmac_scan_proc_scan_req_event_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_SCAN_REQ]            = hmac_scan_proc_scan_req_event_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_LISTEN][HMAC_FSM_INPUT_SCAN_REQ]        = hmac_scan_proc_scan_req_event_etc;

    /* Initialize the elements in the JOIN_REQ input column. This input is   */
    /* valid only in the SCAN_COMP state.                                    */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | SCAN_COMP        | hmac_sta_wait_join_etc  |                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_JOIN_REQ] = hmac_fsm_null_fn_etc;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_JOIN_REQ]   = hmac_sta_wait_join_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_JOIN_REQ] = hmac_sta_wait_join_etc;
}

OAL_STATIC osal_void hmac_fsm_init_sta_auth_table(osal_void)
{
    osal_u32 state;
    /* Initialize the elements in the AUTH_REQ input column. This input is   */
    /* valid only in the JOIN_COMP state.                                    */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | JOIN_COMP        | hmac_sta_wait_auth_etc  |                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */

    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_AUTH_REQ] = hmac_fsm_null_fn_etc;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_JOIN_COMP][HMAC_FSM_INPUT_AUTH_REQ] = hmac_sta_wait_auth;

    /* Initialize the elements in the ASOC_REQ input column. This input is   */
    /* valid only in the AUTH_COMP state.                                    */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | AUTH_COMP        | hmac_sta_wait_asoc_etc       |                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */

    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_ASOC_REQ] = hmac_fsm_null_fn_etc;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_AUTH_COMP][HMAC_FSM_INPUT_ASOC_REQ] = hmac_sta_wait_asoc_etc;

    /* Initialize the elements in the HMAC_FSM_INPUT_RX_MGMT input column. The */
    /* functions for all the states with this input is listed below.         */
    /*                                                                       */
    /*
     +----------------------------------+---------------------
     | FSM State                        | FSM Function
     +----------------------------------+---------------------
     | MAC_VAP_STATE_INIT               | null_fn
     | MAC_VAP_STATE_UP                 | hmac_sta_up_rx_mgmt_etc
     | MAC_VAP_STATE_FAKE_UP            | null_fn
     | MAC_VAP_STATE_WAIT_SCAN          | hmac_sta_wait_scan_rx
     | MAC_VAP_STATE_SCAN_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_JOIN          | hmac_sta_wait_join_rx_etc
     | MAC_VAP_STATE_JOIN_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | hmac_sta_wait_auth_seq2_rx_etc
     | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | hmac_sta_wait_auth_seq4_rx_etc
     | MAC_VAP_STATE_AUTH_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_ASOC          | hmac_sta_wait_asoc_rx_etc
     | MAC_VAP_STATE_OBSS_SCAN          | null_fn
     | MAC_VAP_STATE_BG_SCAN            | null_fn
     +----------------------------------+---------------------
    */

    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_RX_MGMT] = hmac_fsm_null_fn_etc;
    }

    /* 增加 HMAC_FSM_INPUT_RX_MGMT事件 处理函数 */
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_wait_auth_seq2_rx_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4][HMAC_FSM_INPUT_RX_MGMT] = hmac_sta_wait_auth_seq4_rx_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_ASOC][HMAC_FSM_INPUT_RX_MGMT]      = hmac_sta_wait_asoc_rx_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_RX_MGMT]                 = hmac_sta_up_rx_mgmt_etc;

    /* 增加 HotSpot中状态机sta侧处理函数 */
#if defined(_PRE_WLAN_FEATURE_HS20) || defined(_PRE_WLAN_FEATURE_P2P)
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_RX_MGMT]      = hmac_sta_not_up_rx_mgmt_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_SCAN][HMAC_FSM_INPUT_RX_MGMT]      = hmac_sta_not_up_rx_mgmt_etc;
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_RX_MGMT]        = hmac_sta_not_up_rx_mgmt_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_LISTEN][HMAC_FSM_INPUT_RX_MGMT]             = hmac_sta_not_up_rx_mgmt_etc;
#endif

    /* 初始化timer0超时请求输入函数 HMAC_FSM_INPUT_TIMER0_OUT
     +----------------------------------+---------------------
     | FSM State                        | FSM Function
     +----------------------------------+---------------------
     | MAC_VAP_STATE_INIT               | null_fn
     | MAC_VAP_STATE_PAUSE              | null_fn
     | MAC_VAP_STATE_FAKE_UP            | null_fn
     | MAC_VAP_STATE_WAIT_SCAN          | null_fn
     | MAC_VAP_STATE_SCAN_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_JOIN          | null_fn
     | MAC_VAP_STATE_JOIN_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | null_fn
     | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | null_fn
     | MAC_VAP_STATE_AUTH_COMP          | null_fn
     | MAC_VAP_STATE_WAIT_ASOC          | null_fn
     | MAC_VAP_STATE_OBSS_SCAN          | null_fn
     | MAC_VAP_STATE_BG_SCAN            | null_fn
     +----------------------------------+---------------------
    */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_fsm_null_fn_etc;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ2][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_sta_auth_timeout_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_AUTH_SEQ4][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_sta_auth_timeout_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_WAIT_ASOC][HMAC_FSM_INPUT_TIMER0_OUT] = hmac_sta_wait_asoc_timeout_etc;
}

OAL_STATIC osal_void hmac_fsm_init_sta_misc_table(osal_void)
{
    osal_u32 state;
    /* 初始化HMAC_FSM_INPUT_MISC事件请求的处理函数
       +----------------------------------+---------------------
       | FSM State                        | FSM Function
       +----------------------------------+---------------------
       | MAC_VAP_STATE_INIT               | null_fn
       | MAC_VAP_STATE_PAUSE              | null_fn
       | MAC_VAP_STATE_FAKE_UP            | null_fn
       | MAC_VAP_STATE_WAIT_SCAN          | null_fn
       | MAC_VAP_STATE_SCAN_COMP          | null_fn
       | MAC_VAP_STATE_WAIT_JOIN          | null_fn
       | MAC_VAP_STATE_JOIN_COMP          | null_fn
       | MAC_VAP_STATE_WAIT_AUTH_SEQ2     | null_fn
       | MAC_VAP_STATE_WAIT_AUTH_SEQ4     | null_fn
       | MAC_VAP_STATE_AUTH_COMP          | null_fn
       | MAC_VAP_STATE_WAIT_ASOC          | null_fn
       | MAC_VAP_STATE_OBSS_SCAN          | null_fn
       | MAC_VAP_STATE_BG_SCAN            | null_fn
       +----------------------------------+---------------------
      */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_MISC] = hmac_fsm_null_fn_etc;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* Initialize the elements in the LISTEN_REQ input column. This input is */
    /* valid in the SCAN_COMP and FAKE_UP state.                             */
    /*                                                                       */
    /* +------------------+---------------------+                            */
    /* | FSM State        | FSM Function        |                            */
    /* +------------------+---------------------+                            */
    /* | FAKE_UP          | hmac_sta_wait_listen|                            */
    /* | SCAN_COMP        | hmac_sta_wait_listen|                            */
    /* | All other states | null_fn             |                            */
    /* +------------------+---------------------+                            */
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_fsm_null_fn_etc;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_LISTEN_REQ]   = hmac_p2p_remain_on_channel_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_LISTEN_REQ] = hmac_p2p_remain_on_channel_etc;
    /* 设置P2P 监听超时定时器, P2P处于Listen状态下无法下发监听命令，导致Listen状态下不能重新出发监听，无法与商用设备关联 */
    // 在监听状态接收到新的监听命令，则执行新的监听。
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_LISTEN][HMAC_FSM_INPUT_LISTEN_REQ]        = hmac_p2p_remain_on_channel_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_UP][HMAC_FSM_INPUT_LISTEN_REQ]            = hmac_p2p_remain_on_channel_etc;

    /* Initialize the elements in the HMAC_FSM_INPUT_LISTEN_TIMEOUT          */
    /* input column. This input is valid in MAC_VAP_STATE_STA_LISTEN state.  */
    /*                                                                       */
    /* +----------------------------+------------------------+               */
    /* | FSM State                  | FSM Function           |               */
    /* +----------------------------+------------------------+               */
    /* | MAC_VAP_STATE_STA_LISTEN   | hmac_p2p_listen_timeout_etc|               */
    /* | All other states           | null_fn                |               */
    /* +----------------------------+------------------------+               */

    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_LISTEN_TIMEOUT] = hmac_fsm_null_fn_etc;
    }
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_LISTEN][HMAC_FSM_INPUT_LISTEN_TIMEOUT] = hmac_p2p_listen_timeout_etc;

#endif  /* _PRE_WLAN_FEATURE_P2P */

    /* Initialize the elements in the SCHED_SCAN_REQ input column. This input is */
    /* valid in the SCAN_COMP and FAKE_UP state.                             */
    /*                                                                       */
    /* +------------------+------------------------------------+             */
    /* | FSM State        | FSM Function                       |             */
    /* +------------------+------------------------------------+             */
    /* | FAKE_UP          | hmac_scan_proc_sched_scan_req_event_etc|             */
    /* | SCAN_COMP        | hmac_scan_proc_sched_scan_req_event_etc|             */
    /* | All other states | null_fn                            |             */
    /* +------------------+------------------------------------+             */
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
    for (state = 0; state < MAC_VAP_STA_STATE_BUTT; state++) {
        g_pa_hmac_sta_fsm_func[state][HMAC_FSM_INPUT_SCHED_SCAN_REQ] = hmac_fsm_null_fn_etc;
    }

    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_FAKE_UP][HMAC_FSM_INPUT_SCHED_SCAN_REQ]   =
        hmac_scan_proc_sched_scan_req_event_etc;
    g_pa_hmac_sta_fsm_func[MAC_VAP_STATE_STA_SCAN_COMP][HMAC_FSM_INPUT_SCHED_SCAN_REQ] =
        hmac_scan_proc_sched_scan_req_event_etc;
#endif
}

/*****************************************************************************
 初始化sta状态机函数表
*****************************************************************************/
OAL_STATIC osal_void hmac_fsm_init_sta(osal_void)
{
    hmac_fsm_init_sta_scan_table();
    hmac_fsm_init_sta_auth_table();
    hmac_fsm_init_sta_misc_table();
}

/*****************************************************************************
 函 数 名  : hmac_fsm_init_etc
 功能描述  : 初始化状态机处理函数
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年6月18日
    修改内容   : 新生成函数

*****************************************************************************/
osal_void  hmac_fsm_init_etc(osal_void)
{
    /* 初始化ap状态机函数表 */
    hmac_fsm_init_ap();

    /* 初始化sta状态机函数表 */
    hmac_fsm_init_sta();
}

/*****************************************************************************
 函 数 名  : hmac_fsm_call_func_ap_etc
 功能描述  : 调用AP状态机函数表
 输入参数  : hmac_vap: hmac vap
             input: 输入类型
             p_param: 输入参数
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年6月18日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 hmac_fsm_call_func_ap_etc(hmac_vap_stru *hmac_vap, hmac_fsm_input_type_enum_uint8 input,
    osal_void *p_param)
{
    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_fsm_call_func_ap_etc::vap is null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 解决VAP已经删除的情况下，HMAC才处理DMAC抛上来的事件导致状态非法问题 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d]{hmac_fsm_call_func_ap_etc::the vap has been deleted its state is STATEBUTT input type is %d}",
            hmac_vap->vap_id, input);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        return OAL_SUCC;
    } else {
        if ((hmac_vap->vap_state >= MAC_VAP_AP_STATE_BUTT) ||
            (input >= HMAC_FSM_AP_INPUT_TYPE_BUTT)) {
            oam_error_log3(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_fsm_call_func_ap_etc::vap state[%d] or input type[%d] is over limit!}",
                hmac_vap->vap_id, hmac_vap->vap_state, input);
            /* 管理帧丢弃 */
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    return g_pa_hmac_ap_fsm_func[hmac_vap->vap_state][input](hmac_vap, p_param);
}

/*****************************************************************************
 函 数 名  : hmac_fsm_call_func_sta_etc
 功能描述  : 调用STA状态机函数表
 输入参数  : hmac_vap: hmac vap
             input: 输入类型
             p_param: 输入参数
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年6月18日
    修改内容   : 新生成函数

*****************************************************************************/
osal_u32 hmac_fsm_call_func_sta_etc(hmac_vap_stru *hmac_vap, hmac_fsm_input_type_enum_uint8 input,
    osal_void *p_param)
{
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_fsm_call_func_sta_etc::vap is null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 解决VAP已经删除的情况下，HMAC才处理DMAC抛上来的事件导致状态非法问题 */
    if (hmac_vap->vap_state == MAC_VAP_STA_STATE_BUTT) {
        oam_warning_log2(0, OAM_SF_ANY,
            "vap_id[%d]{hmac_fsm_call_func_sta_etc::the vap has been deleted its state is STATE_BUTT input type is %d}",
            hmac_vap->vap_id, input);
        /* 管理帧丢弃 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        return OAL_SUCC;
    } else {
        if (hmac_vap->vap_state >= MAC_VAP_STA_STATE_BUTT ||
            input >= HMAC_FSM_STA_INPUT_TYPE_BUTT) {
            oam_error_log3(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_fsm_call_func_sta_etc::vap state[%d] or input type[%d] is over limit!}",
                hmac_vap->vap_id, hmac_vap->vap_state, input);
            /* 管理帧丢弃 */
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    return g_pa_hmac_sta_fsm_func[hmac_vap->vap_state][input](hmac_vap, p_param);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

