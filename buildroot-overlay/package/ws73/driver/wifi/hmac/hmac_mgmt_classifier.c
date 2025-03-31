/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * 文 件 名   : hmac_mgmt_classifier.c
 * 生成日期   : 2012年11月20日
 * 功能描述   : HMAC模块管理帧操作函数定义的源文件
 */


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_mgmt_classifier.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_fsm.h"
#include "hmac_sme_sta.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "hmac_roam_if.h"
#include "hmac_sdp.h"
#include "hmac_dfx.h"
#include "hmac_feature_dft.h"
#include "wlan_msg.h"

#include "mac_frame.h"
#include "mac_resource_ext.h"
#include "mac_ie.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_main.h"
#include "hmac_rx_data_filter.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_blockack.h"
#include "hmac_mgmt_ap.h"
#include "hmac_tx_mgmt.h"
#include "hmac_scan.h"
#include "mac_vap_ext.h"
#include "hmac_11w.h"
#include "oal_net.h"
#include "hmac_beacon.h"
#include "hmac_chan_mgmt.h"
#include "hmac_psm_sta.h"
#include "oal_netbuf_data.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_device.h"
#include "hmac_btcoex_m2s.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif

#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif
#include "frw_hmac_hcc_adapt.h"
#include "hmac_psm_ap.h"
#include "hmac_promisc.h"
#include "hmac_alg_notify.h"
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#include "hmac_obss_ap.h"
#endif
#include "hmac_beacon.h"
#include "hmac_hook.h"
#include "hmac_csa_sta.h"
#include "hmac_feature_interface.h"
#include "common_log_dbg_rom.h"
#include "hal_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_MGMT_CLASSIFIER_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (defined(_PRE_PRODUCT_ID_HIMPXX_DEV) && !defined(_PRE_PC_LINT) && !defined(WIN32))
OAL_STATIC osal_u8 g_uc_linkloss_log_switch = 0;
#endif

static oal_bool_enum_uint8 hmac_rx_multi_mgmt_pre_process(hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap,
    osal_u8 channel_number, osal_u8 mgmt_type);

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_mgmt_tx_action_etc
 功能描述  : ACTION帧发送
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_mgmt_tx_action_etc(
    hmac_vap_stru              *hmac_vap,
    hmac_user_stru             *hmac_user,
    mac_action_mgmt_args_stru  *action_args)
{
    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (action_args == OAL_PTR_NULL)) {
        common_log_dbg0(0, OAM_SF_TX, "{hmac_mgmt_tx_action_etc::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (action_args->category) {
        case MAC_ACTION_CATEGORY_BA:
            switch (action_args->action) {
                case MAC_BA_ACTION_ADDBA_REQ:
                    oam_info_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_mgmt_tx_action_etc::MAC_BA_ACTION_ADDBA_REQ.}",
                        hmac_vap->vap_id);
                    hmac_mgmt_tx_addba_req_etc(hmac_vap, hmac_user, action_args);
                    break;

                case MAC_BA_ACTION_DELBA:
                    common_log_dbg1(0, OAM_SF_TX, "vap_id[%d] {hmac_mgmt_tx_action_etc::MAC_BA_ACTION_DELBA.}",
                        hmac_vap->vap_id);
                    hmac_mgmt_tx_delba_etc(hmac_vap, hmac_user, action_args);
                    break;

                default:
                    common_log_dbg2(0, OAM_SF_TX, "vap_id[%d] {hmac_mgmt_tx_action_etc::invalid ba type[%d].}",
                        hmac_vap->vap_id, action_args->action);
                    return OAL_FAIL;    /* 错误类型待修改 */
            }
            break;

        default:
            common_log_dbg2(0, OAM_SF_TX, "vap_id[%d] {hmac_mgmt_tx_action_etc::invalid ba type[%d].}",
                hmac_vap->vap_id, action_args->category);
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_rx_delba_event_etc
 功能描述  : 自身DMAC模块产生DELBA的处理函数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_mgmt_rx_delba_event_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_ctx_action_event_stru         *delba_event = OAL_PTR_NULL;
    osal_u8                            *da = OAL_PTR_NULL;
    hmac_user_stru                     *hmac_user = OAL_PTR_NULL;
    mac_action_mgmt_args_stru           action_args;
    osal_u32 ret;

    if (osal_unlikely(msg->data == OAL_PTR_NULL) || osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_BA, "{hmac_mgmt_rx_delba_event_etc::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    delba_event = (dmac_ctx_action_event_stru *)msg->data;
    /* 获取目的用户的MAC ADDR */
    da = delba_event->mac_addr;

    /* 获取发送端的用户指针 */
    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, da);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_BA,
            "vap_id[%d] {hmac_mgmt_rx_delba_event_etc::hmac_vap_find_user_by_macaddr_etc failed.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    action_args.category = MAC_ACTION_CATEGORY_BA;
    action_args.action   = MAC_BA_ACTION_DELBA;
    action_args.arg1     = delba_event->tidno;     /* 该数据帧对应的TID号 */
    action_args.arg2     = delba_event->initiator; /* DELBA中，触发删除BA会话的发起端 */
    action_args.arg3     = delba_event->status;    /* DELBA中代表删除reason */
    action_args.arg5    = da;                        /* DELBA中代表目的地址 */

    ret = hmac_mgmt_tx_action_etc(hmac_vap, hmac_user, &action_args);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_BA,
            "{hmac_mgmt_rx_delba_event_etc::hmac_mgmt_tx_action_etc failed[%d].}", ret);
    }

    return OAL_SUCC;
}

osal_s32 hmac_rx_process_report_mgmt_ctrl(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_s32 ret = OAL_SUCC;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_rx_ctl_stru *pst_rx_ctrl = OAL_PTR_NULL;
    mac_rx_ctl_stru *rx_info = OAL_PTR_NULL;
    osal_u8 *mac_hdr = OAL_PTR_NULL;
    osal_u8 subtype = 0;
    osal_u8 user_addr[6] = {0};
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_STATISTIC_RECORD);
#endif
   oal_bool_enum_uint8 reported_host = OSAL_FALSE;

    osal_void *sdp_fhook = hmac_get_feature_fhook(HMAC_FHOOK_IS_SDP_FRAME);
    osal_void *sdp_rx_fhook = hmac_get_feature_fhook(HMAC_FHOOK_SDP_PROC_RX_MGMT);
    hmac_wlan_crx_event_stru event = {0};

    mac_vap_intrrupt_disable();
    if (hmac_vap->init_flag == MAC_VAP_INVAILD) {
        mac_vap_intrrupt_enable();
        oal_netbuf_free(netbuf);
        /* 管理帧丢弃 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
        return OAL_FAIL;
    }
    /* 获取VAP后,引用计数加1 */
    osal_adapt_atomic_inc(&(hmac_vap->use_cnt));
    mac_vap_intrrupt_enable();

#ifdef _PRE_WLAN_DFT_STAT
    pst_rx_ctrl         = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    rx_info         = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->rx_info));
    mac_hdr             = (osal_u8 *)mac_get_rx_cb_mac_header_addr(rx_info);
    subtype             = mac_frame_get_subtype_value(mac_hdr);

    mac_get_address2(oal_netbuf_header(netbuf), user_addr);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_statistic_record_cb)fhook)(hmac_vap->vap_id, user_addr, subtype,
            hmac_vap->vap_mode, OAM_OTA_FRAME_DIRECTION_TYPE_RX);
    }
#endif
    if (sdp_fhook != OSAL_NULL && (((hmac_is_sdp_frame_cb)sdp_fhook)(netbuf) == OAL_TRUE)) {
        /* 处理接收到的SDP Action帧 */
        if (sdp_rx_fhook != OSAL_NULL) {
            ret = (osal_s32)(((hmac_sdp_proc_rx_mgmt_cb)sdp_rx_fhook)(hmac_vap, netbuf, &reported_host));
            if (ret != OAL_SUCC) {
                /* 返回不成功，最外层会统一释放，这里不需要释放,
                对于组播publish 会在hmac_rx_multi_mgmt_ctrl_frame释放,
                单播follow up 会在hmac_rx_cut_process_mgmt_ctrl_event释放 */
                oam_warning_log0(0, 0, "{hmac_rx_process_mgmt:: SDP frame process failed}");
                ret = OAL_FAIL;
                /* 管理帧丢弃 */
                hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
                goto safe_del_vap;
            }
        }
        oal_netbuf_free(netbuf);
        ret = OAL_SUCC;
        goto safe_del_vap;
    }

    /* 接收管理帧是状态机的一个输入，调用状态机接口 */
    event.netbuf = netbuf;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ret = (osal_s32)hmac_fsm_call_func_ap_etc(hmac_vap, HMAC_FSM_INPUT_RX_MGMT, &event);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_BA,
                "{hmac_rx_process_mgmt::hmac_fsm_call_func_ap_etc fail.err code1 [%u]}", ret);
        }
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        ret = (osal_s32)hmac_fsm_call_func_sta_etc(hmac_vap, HMAC_FSM_INPUT_RX_MGMT, &event);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_BA,
                "{hmac_rx_process_mgmt::hmac_fsm_call_func_ap_etc fail.err code2 [%u]}", ret);
        }
    } else {
        /* 管理帧丢弃 */
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf);
    }

    /* 管理帧统一释放接口 */
    oal_netbuf_free(netbuf);
    ret = OAL_SUCC;
safe_del_vap:
    /* VAP使用结束自减，并判断是否为0，为0，则VAP进行安全删除 */
    hmac_config_safe_del_vap(hmac_vap);
    return ret;
}

osal_void hmac_mgmt_send_deauth_event(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_vap_stru *up_vap1 = OAL_PTR_NULL;
    hmac_vap_stru *up_vap2 = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
#endif

    hmac_mgmt_send_deauth_frame_etc(hmac_vap, da, err_code, OAL_FALSE); // 非PMF

#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    /* 判断是异频DBAC模式时，无法判断是哪个信道收到的数据帧，两个信道都需要发去认证 */
    if (hmac_device_find_2up_vap_etc(hmac_device, &up_vap1, &up_vap2) != OAL_SUCC) {
        return;
    }

    if (up_vap1->channel.chan_number == up_vap2->channel.chan_number) {
        return;
    }

    /* 获取另一个VAP */
    if (hmac_vap->vap_id != up_vap1->vap_id) {
        up_vap2 = up_vap1;
    }

    /* 另外一个VAP也发去认证帧。error code加上特殊标记，组去认证帧时要修改源地址 */
    hmac_mgmt_send_deauth_frame_etc(up_vap2, da, err_code | MAC_SEND_TWO_DEAUTH_FLAG, OAL_FALSE);
#endif
    return;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_send_disasoc_event
 功能描述  : 去关联/去认证事件处理,数据帧处理流程中删除用户需要抛消息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_mgmt_send_disasoc_msg(hmac_vap_stru *hmac_vap, osal_u8 *da, osal_u16 err_code)
{
    dmac_diasoc_deauth_event disasoc_deauth_event;
    frw_msg msg_info;
    osal_s32 ret;

    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));

    disasoc_deauth_event.reason = (osal_u8)err_code;
    if (memcpy_s(disasoc_deauth_event.des_addr, WLAN_MAC_ADDR_LEN, da, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_warning_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_msg::memcpy_s fail.}");
        return;
    }

    msg_info.data = (osal_u8 *)&disasoc_deauth_event;
    msg_info.data_len = sizeof(disasoc_deauth_event);
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2H_C_DISASOC, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oam_info_log1(0, OAM_SF_ASSOC, "{frw_asyn_host_post_msg:: failed! ret[%d].}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_send_disasoc_event
 功能描述  : 去关联/去认证事件处理
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_mgmt_send_disasoc_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_diasoc_deauth_event *disasoc_deauth_event = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u32 rslt;
    osal_u16 user_idx;
    osal_u16 err_code;
    osal_u8 *da = OAL_PTR_NULL;   /* 保存用户目的地址的指针 */
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    if ((hmac_vap == OSAL_NULL) || (msg == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_event::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    disasoc_deauth_event = (dmac_diasoc_deauth_event *)msg->data;
    da      = disasoc_deauth_event->des_addr;
    err_code = disasoc_deauth_event->reason;

    /* 获取发送端的用户指针 */
    rslt = hmac_vap_find_user_by_macaddr_etc(hmac_vap, da, &user_idx);
    if (rslt != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_RX,
            "{Hmac cannot find USER by addr[%02X:%02X:%02X:%02X:XX:XX], just del DMAC user}",
            da[0], da[1], da[2], da[3]); /* 0、1、2、3表示MAC地址位 */

        /* 找不到用户，说明用户已经删除，直接返回成功，不需要再抛事件到dmac删除用户(统一由hmac_user_del来管理删除用户) */
        return OAL_SUCC;
    }

    /* 获取到hmac user,使用protected标志 */
    hmac_user = mac_res_get_hmac_user_etc(user_idx);

    hmac_mgmt_send_disassoc_frame_etc(hmac_vap, da, err_code,
        ((hmac_user == OAL_PTR_NULL) ? OAL_FALSE : hmac_user->cap_info.pmf_active));
    oam_warning_log0(0, OAM_SF_DFT, "hmac_mgmt_send_disasoc_event:send DISASSOC frame");

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, da, AP_STA_NOUP_RCV_DATA, USER_CONN_OFFLINE);
    }
    /* 删除用户 */
    hmac_user_del_etc(hmac_vap, hmac_user);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_proc_disasoc_misc_event_etc
 功能描述  : 本地断链异常码数转化为通用异常原因码
 输入参数  : disasoc_reason:本地断链异常码
            disasoc_reason_code  :通用异常原因码
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC mac_reason_code_enum_uint16 hmac_disassoc_reason_exchange(
    dmac_disasoc_misc_reason_enum_uint16 disasoc_reason)
{
    switch (disasoc_reason) {
        case DMAC_DISASOC_MISC_LINKLOSS:
        case DMAC_DISASOC_MISC_KEEPALIVE:
        case DMAC_DISASOC_MISC_GET_CHANNEL_IDX_FAIL:
            return  MAC_DEAUTH_LV_SS;

        case DMAC_DISASOC_MISC_CHANNEL_MISMATCH:
            return MAC_UNSPEC_REASON;
        default:
            break;
    }
    oam_warning_log1(0, OAM_SF_ASSOC, "{hmac_disassoc_reason_exchange::Unkown driver_disasoc_reason[%d].}",
        disasoc_reason);

    return MAC_UNSPEC_REASON;
}

static osal_u32 hmac_proc_disasoc_misc_event_ap(hmac_vap_stru *hmac_vap,
    hmac_disasoc_misc_stru *disasoc_misc_stru)
{
    oal_bool_enum_uint8 is_protected;  /* PMF */
    mac_reason_code_enum_uint16 disasoc_reason_code;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    hmac_user = mac_res_get_hmac_user_etc(disasoc_misc_stru->user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d]{hmac_proc_disasoc_misc_event_etc::hmac_user[%d] is null}",
            hmac_vap->vap_id,
            disasoc_misc_stru->user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    is_protected = hmac_user->cap_info.pmf_active;

    /* 抛事件上报内核，已经去关联某个STA */
    hmac_handle_disconnect_rsp_ap_etc(hmac_vap, hmac_user);
    disasoc_reason_code = MAC_ASOC_NOT_AUTH;
    /* 发去关联帧 */
    hmac_mgmt_send_disassoc_frame_etc(hmac_vap,
        hmac_user->user_mac_addr,
        disasoc_reason_code, is_protected);
    oam_warning_log1(0, OAM_SF_DFT, "hmac_proc_disasoc_misc_event_ap:send DISASSOC frame, reason_code = %d.",
        disasoc_reason_code);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, hmac_user->user_mac_addr,
            HMAC_DISASOC_NOTICE, USER_CONN_OFFLINE);
    }
    /* 删除用户 */
    hmac_user_del_etc(hmac_vap, hmac_user);
    return OAL_SUCC;
}

static osal_u32 hmac_proc_disasoc_misc_event_sta(hmac_vap_stru *hmac_vap,
    hmac_disasoc_misc_stru *disasoc_misc_stru)
{
    mac_reason_code_enum_uint16 disasoc_reason_code;
    oal_bool_enum_uint8 is_protected;  /* PMF */
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        /* 和ap侧一样，上层已经删除了的话，属于正常 */
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d]{hmac_proc_disasoc_misc_event_etc::hmac_user[%d] is null}",
            hmac_vap->vap_id,
            hmac_vap->assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    is_protected = hmac_user->cap_info.pmf_active;

    /* 上报断链类型的转化 */
    disasoc_reason_code = hmac_disassoc_reason_exchange(disasoc_misc_stru->disasoc_reason);

    /* STA发现AP信道修改，则删除用户，而不发送去认证帧 */
    if (disasoc_misc_stru->disasoc_reason != DMAC_DISASOC_MISC_CHANNEL_MISMATCH) {
        /* 发送去认证帧到AP */
        hmac_mgmt_send_disassoc_frame_etc(hmac_vap,
            hmac_user->user_mac_addr, disasoc_reason_code, is_protected);
        oam_warning_log1(0, OAM_SF_DFT, "hmac_proc_disasoc_misc_event_sta:send DISASSOC frame, reason_code = %d.",
            disasoc_reason_code);
    }
    /* USB配置概率出现disassoc帧发送比vap被置为fake up还要慢的情况，导致disassoc帧被回收，出现发送时间不符合预期的问题 */
    /* 这里等待20ms，保证disassoc帧先送出 */
    osal_msleep(20);
    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->chip_id, hmac_user->user_mac_addr,
        HMAC_DISASOC_NOTICE, USER_CONN_OFFLINE);
    }
    /* 删除对应用户 */
    hmac_user_del_etc(hmac_vap, hmac_user);
    hmac_sta_handle_disassoc_rsp_etc(hmac_vap, disasoc_reason_code);
    return OAL_SUCC;
}

osal_void hmac_send_disasoc_misc_msg(hmac_vap_stru *hmac_vap, osal_u16 user_idx, osal_u16 disasoc_reason)
{
    hmac_disasoc_misc_stru disasoc_misc;
    frw_msg msg_info;
    osal_s32 ret;
    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));

    (osal_void)memset_s(&disasoc_misc, sizeof(disasoc_misc), 0, sizeof(disasoc_misc));
    disasoc_misc.disasoc_reason = disasoc_reason;
    disasoc_misc.user_idx       = user_idx;

    msg_info.data = (osal_u8 *)&disasoc_misc;
    msg_info.data_len = sizeof(disasoc_misc);
    ret = frw_asyn_host_post_msg(WLAN_MSG_H2H_C_MISC_DISASOC, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oam_info_log1(0, OAM_SF_ASSOC, "{frw_asyn_host_post_msg:: failed! ret[%d].}", ret);
    }
}

osal_s32 hmac_handle_disasoc_misc_msg(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_disasoc_misc_stru *disasoc_misc = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (msg == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_send_disasoc_event::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    disasoc_misc = (hmac_disasoc_misc_stru *)msg->data;

    return (osal_s32)hmac_send_disassoc_misc_event(hmac_vap, disasoc_misc->user_idx, disasoc_misc->disasoc_reason);
}
/*****************************************************************************
 函 数 名  : hmac_send_disassoc_misc_event
 功能描述  : DMAC发送去关联MISC事件
****************************************************************************/
osal_u32 hmac_send_disassoc_misc_event(hmac_vap_stru *hmac_vap, osal_u16 user_idx, osal_u16 disasoc_reason)
{
    osal_u32               ret;
    hmac_disasoc_misc_stru disasoc_misc;

    (osal_void)memset_s(&disasoc_misc, sizeof(disasoc_misc), 0, sizeof(disasoc_misc));
    disasoc_misc.disasoc_reason = disasoc_reason;
    disasoc_misc.user_idx       = user_idx;

    oam_warning_log3(0, OAM_SF_ASSOC,
        "vap_id[%d] {hmac_proc_disassoc_misc_event_etc::Device noticed to disassoc user[%d] within reason[%d]!}",
        hmac_vap->vap_id, disasoc_misc.user_idx, disasoc_misc.disasoc_reason);

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        ret = hmac_proc_disasoc_misc_event_ap(hmac_vap, &disasoc_misc);
    } else {
        ret = hmac_proc_disasoc_misc_event_sta(hmac_vap, &disasoc_misc);
    }

    return ret;
}

static osal_u32 hmac_rx_process_blockack_req(hmac_vap_stru *hmac_vap, hmac_user_stru *ta_user,
    oal_netbuf_stru *netbuf, osal_u8 *go_on)
{
    osal_u8 *payload = OSAL_NULL;
    osal_u8 tidno;
    hmac_tid_stru *tid = OSAL_NULL;
    unref_param(go_on);

    /* 获取帧头和payload指针 */
    payload = oal_netbuf_rx_data(netbuf);

    /*************************************************************************/
    /*                     BlockAck Request Frame Format                     */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BAR Control|BlockAck Starting    |FCS|  */
    /* |             |        |  |  |           |Sequence number      |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |2          |2                    |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    tidno = (payload[1] & 0xF0) >> 4; /* 右移4bit */

    /* drop bar frame when tid num nonsupport */
    if (tidno >= WLAN_TID_MAX_NUM) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_blockack_req::invaild tidno:%d.}",
            hmac_vap->vap_id, (osal_s32)tidno);
        return OAL_SUCC;
    }

    tid = &(ta_user->tx_tid_queue[tidno]);
    if (tid->ba_rx_hdl.ba_conn_status != HMAC_BA_COMPLETE) {
        common_log_dbg1(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_process_blockack_req::ba_rx_hdl null.}", hmac_vap->vap_id);
        hmac_mgmt_delba(hmac_vap, ta_user, tidno, MAC_RECIPIENT_DELBA, MAC_QSTA_SETUP_NOT_DONE);
        return OAL_ERR_CODE_PTR_NULL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_process_ctrl
 功能描述  : 控制帧处理
*****************************************************************************/
static osal_u32 hmac_rx_process_ctrl(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf,
    osal_u8 *go_on)
{
    mac_ieee80211_pspoll_frame_stru *pspoll_frame_hdr = OSAL_NULL;
    osal_u16 user_idx = 0xFFFF;
    /* 获取帧头信息 */
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    osal_u8 *sa_addr = OSAL_NULL;
    hmac_user_stru *ta_user = OSAL_NULL;
    osal_u32 rslt;

    /* HMAC_PROT_HOOK_RX_CTRL_IN */

#ifdef _PRE_WLAN_DFT_STAT
    hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_C_FROM_DEVICE_PKTS);
#endif

    *go_on = OSAL_FALSE; /* 默认不上报control帧 */

    if (hmac_user != OSAL_NULL) {
        hmac_alg_rx_cntl_notify(hmac_vap, hmac_user, netbuf);
    }

    /* 过滤ACK帧 */
    if (frame_hdr->frame_control.sub_type == WLAN_ACK) {
        return OAL_SUCC;
    }

    /* 获取源地址 */
    sa_addr = frame_hdr->address2;

    /*  获取用户指针 */
    rslt = hmac_vap_find_user_by_macaddr_etc(hmac_vap, sa_addr, &user_idx);
    /*
        查找用户失败: 程序异常，返回，在外围释放空间
        没有找到对应的用户: 程序继续执行
    */
    if (rslt != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_ctrl::find_user_by_macaddr failed[%d]}",
            hmac_vap->vap_id, rslt);

        return rslt;
    }

    /* 转化为dmac user */
    ta_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (ta_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_ctrl::ta_user[%d] null.}",
            hmac_vap->vap_id, user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 接收找到用户: 更新时间戳 */
    ta_user->last_active_timestamp = osal_get_time_stamp_ms();

    /* HMAC_FRAME_HOOK_RX_CTRL_PROCESS */

    if ((frame_hdr->frame_control.sub_type) == WLAN_BLOCKACK_REQ) { /* BAR 1000 */
        return hmac_rx_process_blockack_req(hmac_vap, ta_user, netbuf, go_on);
    }

    if ((frame_hdr->frame_control.sub_type) == WLAN_PS_POLL) { /* PS_POLL 1010 */
        /* 如果是ps-poll，则应该把帧头转换为ps-poll的帧头格式 */
        pspoll_frame_hdr = (mac_ieee80211_pspoll_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
        /* AP需要判断PS_POLL里面的AID与AP保存的用户AID是否一致,不一致的话直接忽略 */
        if (pspoll_frame_hdr->aid_value != ta_user->assoc_id) {
            oam_error_log3(0, OAM_SF_RX,
                "vap_id[%d] {hmac_rx_process_ctrl::AID in pspoll and user mismatch, aid=%d assoc_id=%d.}",
                hmac_vap->vap_id, pspoll_frame_hdr->aid_value, ta_user->assoc_id);
            return OAL_FAIL;
        }

        /* 直接调用PS_POLL, 后续使用帧类型即可 */
        rslt = hmac_psm_resv_ps_poll(hmac_vap, ta_user);
        if (rslt != OAL_SUCC) {
            return rslt;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_action_not_report
 功能描述  : AP在UP状态下的接收ACTION帧处理
*****************************************************************************/
static osal_u8 hmac_ap_up_rx_action_not_report(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u16 user_idx)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    osal_u8 go_on = OSAL_FALSE;

    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_ap_up_rx_action_not_report::hmac_user[%d] null.}",
            hmac_vap->vap_id, user_idx);
        return OSAL_FALSE;
    }

    /* 获取帧体指针 */
    /* 上述获取帧体指针的函数有误，获取的指针为帧的MAC头地址 需改为下面方法 */
    data = oal_netbuf_rx_data(netbuf);

    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_BA: {
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_BA_ACTION_ADDBA_REQ:
                    go_on = OSAL_TRUE;
                    break;

                case MAC_BA_ACTION_ADDBA_RSP:
                    go_on = OSAL_TRUE;
                    break;

                case MAC_BA_ACTION_DELBA:
                    go_on = OSAL_TRUE;
                    break;

                default:
                    break;
            }
        }
            break;

        case MAC_ACTION_CATEGORY_PUBLIC: {
            /* Action */
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
                case MAC_PUB_COEXT_MGMT:
                    /* 20/40共存管理帧在dmac处理无需上报host */
                    if (hmac_vap->cap_flag.autoswitch_2040 == OSAL_TRUE) {
                        go_on = OSAL_FALSE;
                        hmac_ap_up_rx_2040_coext(hmac_vap, netbuf);
                    }
                    break;
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP */

                case MAC_PUB_VENDOR_SPECIFIC:
#ifdef _PRE_WLAN_FEATURE_P2P
                    /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0)  */
                    /* 并用hmac_rx_mgmt_send_to_host接口上报 */
                    go_on = OSAL_TRUE;
#endif /* _PRE_WLAN_FEATURE_P2P */
                    break;
                default:
                    break;
            }
        }
            break;

        case MAC_ACTION_CATEGORY_HT: {
            /* Action */
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH:
                    go_on = OSAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
                    hmac_rx_notify_channel_width(hmac_vap, data, hmac_user);
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
                    hmac_ap_rx_notify_channel_width(hmac_vap, hmac_user, data);
#endif
                    break;
#ifdef _PRE_WLAN_FEATURE_SMPS
                case MAC_HT_ACTION_SMPS:
                    /* m2s相关action帧处理下移，需要时候同步到host侧 */
                    go_on = OSAL_FALSE;
                    hmac_mgmt_rx_smps_frame(hmac_vap, hmac_user, data);
                    break;
#endif
                case MAC_HT_ACTION_BUTT:
                default:
                    break;
            }
        }
            break;

#ifdef _PRE_WLAN_FEATURE_PMF
        case MAC_ACTION_CATEGORY_SA_QUERY: {
            /* Action */
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_SA_QUERY_ACTION_REQUEST:
                    go_on = OSAL_TRUE;
                    break;
                case MAC_SA_QUERY_ACTION_RESPONSE:
                    go_on = OSAL_TRUE;
                    break;
                default:
                    break;
            }
        }
            break;
#endif
        case MAC_ACTION_CATEGORY_VHT: {
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
                case MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION:
                    go_on = OSAL_FALSE;
                    hmac_mgmt_rx_opmode_notify_frame(hmac_vap, hmac_user, netbuf);
                    break;
#endif
                case MAC_VHT_ACTION_BUTT:
                default:
                    break;
            }
        }
            break;

#ifdef _PRE_WLAN_FEATURE_WUR_TX
        case MAC_ACTION_CATEGORY_WUR: {
            if (hmac_vap->cap_flag.wur_enable == OSAL_TRUE &&
                (data[MAC_ACTION_OFFSET_ACTION] == MAC_WUR_ACTION_MODE_SETUP ||
                data[MAC_ACTION_OFFSET_ACTION] == MAC_WUR_ACTION_MODE_TEARDOWN)) {
                go_on = OSAL_TRUE;
                break;
            }
        }
            break;
#endif

        case MAC_ACTION_CATEGORY_VENDOR: {
#ifdef _PRE_WLAN_FEATURE_P2P
            /* 查找OUI-OUI type值为 50 6F 9A - 09 (WFA P2P v1.0)  */
            /* 并用hmac_rx_mgmt_send_to_host接口上报 */
            go_on = OSAL_TRUE;
#endif
        }
            break;

        default:
            break;
    }

    return go_on;
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_action_not_report
 功能描述  : STA在UP状态下的接收ACTION帧处理
*****************************************************************************/
static osal_u8 hmac_sta_up_rx_action_not_report(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u16 user_idx)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    osal_u8 go_on = OSAL_TRUE; // action帧默认上报host

    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_sta_up_rx_action_not_report::hmac_user[%d] null.}",
            hmac_vap->vap_id, user_idx);
        return OSAL_FALSE;
    }

    /* 获取帧体指针 */
    data = oal_netbuf_rx_data(netbuf);
    /* Category values 128 - 255 Error */
    if (data[MAC_ACTION_OFFSET_CATEGORY] > MAC_ACTION_CATEGORY_VENDOR) {
        return OSAL_FALSE;
    }

    switch (data[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_HT: {
            /* Action */
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
                case MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH:
                    go_on = OSAL_FALSE;
                    hmac_rx_notify_channel_width(hmac_vap, data, hmac_user);
                    break;
#endif
                case MAC_HT_ACTION_BUTT:
                default:
                    break;
            }
        }
            break;

        case MAC_ACTION_CATEGORY_PUBLIC: {
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
                case MAC_PUB_COEXT_MGMT:
                    go_on = OSAL_FALSE;
                    hmac_sta_up_rx_2040_coext(hmac_vap, data);
                    break;
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */
                case MAC_PUB_EX_CH_SWITCH_ANNOUNCE:
                    go_on = OSAL_FALSE;
                    break;
                default:
                    break;
            }
        }
            break;

        case MAC_ACTION_CATEGORY_SPECMGMT: {
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_SPEC_CH_SWITCH_ANNOUNCE:
                    go_on = OSAL_FALSE;
                    break;
                default:
                    break;
            }
        }
            break;

        case MAC_ACTION_CATEGORY_VHT: {
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
                case MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION:
                    go_on = OSAL_FALSE;
                    // sta模式暂时不支持帧解析
                    break;
#endif
                case MAC_VHT_ACTION_BUTT:
                default:
                    break;
            }
        }
            break;
#ifdef _PRE_WLAN_FEATURE_11AX
        case MAC_ACTION_CATEGORY_PROTECTED_HE: {
            switch (data[MAC_ACTION_OFFSET_ACTION]) {
                case MAC_PROTECTED_HE_ACTION_MU_EDCA_CONTROL:
                    hmac_sta_update_affected_acs_machw_direct(data, hmac_vap);
                    go_on = OSAL_FALSE;
                    break;
                default:
                    break;
            }
        }
            break;
#endif
        default:
            break;
    }

    return go_on;
}

#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
/*****************************************************************************
 函 数 名  : hmac_rx_beacon_ant_change
 功能描述  : sta模式下通过beacon的rssi来判断管理帧是通过c0还是c1发送
*****************************************************************************/
OSAL_STATIC osal_void hmac_rx_beacon_ant_change(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_M2S
    dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
#endif
    unref_param(netbuf);
    hal_device = hmac_vap->hal_device;

    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_RX,
            "vap_id[%d] {hmac_rx_beacon_ant_change::hal_device null.}", hmac_vap->vap_id);
        return;
    }

    /* 管理帧切换没有使能则退出 */
    if ((HAL_ANT_SWITCH_RSSI_MGMT_ENABLE & hal_device->rssi.rx_rssi.ant_rssi_sw) == 0) {
        return;
    }

    /* 不为MIMO，不处理 */
    if (hal_device->hal_m2s_fsm.oal_fsm.cur_state != HAL_M2S_STATE_MIMO) {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_m2s_mgmt_switch_judge(hal_device, hmac_vap, rx_ctl->rx_statistic.ant0_rssi, rx_ctl->rx_statistic.ant1_rssi);
#endif
}
#endif

/*****************************************************************************
 函 数 名  : hmac_rx_sta_up_beacon_mgmt
 功能描述  :beacon帧处理
*****************************************************************************/
static osal_void hmac_rx_sta_up_beacon_mgmt(hmac_vap_stru *hmac_vap, const osal_u8 *bssid,
    oal_netbuf_stru *netbuf, osal_u8 *go_on)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    /* multi-bssid处理:若本sta关联的是multi-bss中的一员，且非transmitted bss，而当前接收的beacon是transmitted bss
       需要将multi_bss_proc标志置1 */
    osal_u32 multi_bss_proc = OSAL_FALSE;

    if ((mac_mib_get_multi_bssid_implement(hmac_vap) == OSAL_TRUE) &&
        (hmac_vap->all_bss_info.multi_bss_eid_exist == OSAL_TRUE) &&
        (oal_memcmp(bssid, hmac_vap->all_bss_info.trans_bssid, WLAN_MAC_ADDR_LEN) == 0)) {
        multi_bss_proc = OSAL_TRUE;
    }
#endif

    /* 获取Beacon帧中的mac地址，即AP的mac地址 */
    if ((oal_memcmp(bssid, hmac_vap->bssid, WLAN_MAC_ADDR_LEN) == 0)
#ifdef _PRE_WLAN_FEATURE_11AX
        || (multi_bss_proc == OSAL_TRUE)
#endif
        ) {
        /* 收到BSSID发送的beacon或者是multi Bssid场景，linkloss计数清零 */
        hmac_vap_linkloss_channel_clean(hmac_vap, netbuf);
#ifdef _PRE_WLAN_FEATURE_P2P
        if (is_p2p_cl((hmac_vap))) {
            /* 获取GO Beacon帧中的NoA资讯 */
            hmac_process_p2p_noa(hmac_vap, netbuf);
        }

        /* P2P WFA 7.1.3，OppPS 节能接收到beacon 帧后， 才使能发送 */
        if ((is_p2p_cl(hmac_vap)) && (is_p2p_oppps_enabled(hmac_vap))) {
            hmac_p2p_oppps_ctwindow_start_event(hmac_vap);
        }
#endif /* #ifdef _PRE_WLAN_FEATURE_P2P */
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
        hmac_rx_beacon_ant_change(hmac_vap, netbuf);
#endif
    }
    /* 优先配置beacon中已改变的信息，避免后续睡眠配置失败  */
    hmac_sta_up_rx_beacon_check(hmac_vap, netbuf, go_on);
}

/*****************************************************************************
 函 数 名  : hmac_rx_ap_mgmt
 功能描述  :接收到ap管理帧处理
*****************************************************************************/
static osal_void hmac_rx_ap_mgmt(const hmac_rx_mgmt_deal_stru *rx_mgmt_deal, const osal_u8 bssid[],
    osal_u32 bssid_len, osal_u8 *go_on, const dmac_rx_ctl_stru *rx_ctl)
{
    osal_u32 ret;
    osal_u8 sub_type = rx_mgmt_deal->frame_hdr->frame_control.sub_type;
    osal_u16 user_idx = 0;
    hmac_vap_stru *hmac_vap = rx_mgmt_deal->hmac_vap;
    unref_param(bssid_len);

    if (hmac_vap->vap_state == MAC_VAP_STATE_UP && hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        if (sub_type == WLAN_PROBE_REQ) {
            *go_on = OSAL_FALSE;
            hmac_ap_up_rx_probe_req(rx_mgmt_deal->hmac_vap, rx_mgmt_deal->netbuf, rx_mgmt_deal->frame_hdr->address2,
                (osal_char)get_rssi(rx_ctl->rx_statistic.rssi_dbm, rx_ctl->rx_statistic.ant0_rssi,
                    rx_ctl->rx_statistic.ant1_rssi), go_on);
        } else if (sub_type == WLAN_AUTH) {
            *go_on = OSAL_TRUE;
        } else if (sub_type == WLAN_ACTION) {
#ifdef _PRE_WLAN_FEATURE_P2P
            /* P2P GO接收到P2P action 管理帧处理 */
            if (is_p2p_go(rx_mgmt_deal->hmac_vap)) {
                *go_on = hmac_p2p_listen_rx_mgmt(rx_mgmt_deal->hmac_vap, rx_mgmt_deal->netbuf);
                oam_warning_log1(0, OAM_SF_RX, "{hmac_rx_ap_mgmt::p2p go rx action go_on[%d]", *go_on);
                return;
            }
#endif
            ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, rx_mgmt_deal->frame_hdr->address2, &user_idx);
            *go_on = (ret != OAL_SUCC) ? OSAL_FALSE :
                hmac_ap_up_rx_action_not_report(hmac_vap, rx_mgmt_deal->netbuf, user_idx);
            hmac_ap_up_rx_mgmt_with_vendor_ie(rx_mgmt_deal->hmac_vap, rx_mgmt_deal->netbuf, go_on);
        } else if ((sub_type == WLAN_DISASOC || sub_type == WLAN_DEAUTH) &&
            (oal_memcmp(bssid, rx_mgmt_deal->hmac_vap->bssid, WLAN_MAC_ADDR_LEN) != 0)) {
            *go_on = OSAL_FALSE;
            hmac_vap = rx_mgmt_deal->hmac_vap;
            oam_info_log4(0, OAM_SF_RX, "{hmac_rx_ap_mgmt::drop disasoc frame! bssid[%x:%x:%x:%x:XX:XX] not match!}",
                /* index 0，index 1，index 2，index 3 */
                hmac_vap->bssid[0], hmac_vap->bssid[1], hmac_vap->bssid[2], hmac_vap->bssid[3]);
        }
    } else if (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE && hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP &&
        sub_type == WLAN_PROBE_REQ) {
        *go_on = OSAL_FALSE;
    }

    if (sub_type == WLAN_BEACON || sub_type == WLAN_PROBE_RSP) {
        if (hmac_vap->vap_state == MAC_VAP_STATE_UP) {
            *go_on = OSAL_FALSE;
            hmac_ap_up_rx_obss_beacon(hmac_vap, rx_mgmt_deal->netbuf);
        } else if (hmac_vap->vap_state == MAC_VAP_STATE_AP_WAIT_START) {
            *go_on = OSAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
            hmac_ap_wait_start_rx_obss_beacon(hmac_res_get_mac_dev_etc(0), hmac_vap, rx_mgmt_deal->netbuf);
#endif
        }
    }
}
/*****************************************************************************
 函 数 名  : hmac_rx_sta_mgmt
 功能描述  :接收到ap管理帧处理
*****************************************************************************/
static osal_void hmac_rx_sta_mgmt(hmac_rx_mgmt_deal_stru *rx_mgmt_deal, const osal_u8 bssid[],
    osal_u32 bssid_len, osal_u8 *go_on, const dmac_rx_ctl_stru *rx_ctl)
{
    osal_u32 ret;
    osal_u16 user_idx;
    osal_bool notify_device = OSAL_FALSE;
    unref_param(bssid_len);

    if (rx_mgmt_deal->hmac_vap->vap_state == MAC_VAP_STATE_UP) {
        if (rx_mgmt_deal->frame_hdr->frame_control.sub_type == WLAN_ACTION) {
#ifdef _PRE_WLAN_FEATURE_P2P
            if (!is_legacy_vap(rx_mgmt_deal->hmac_vap)) {
                *go_on = OSAL_TRUE;
                return;
            }
#endif
            ret = hmac_vap_find_user_by_macaddr_etc(rx_mgmt_deal->hmac_vap,
                rx_mgmt_deal->frame_hdr->address2, &user_idx);
            if (ret != OAL_SUCC) {
                *go_on = OSAL_FALSE;
            } else {
                *go_on = hmac_sta_up_rx_action_not_report(rx_mgmt_deal->hmac_vap, rx_mgmt_deal->netbuf, user_idx);
            }
        }
    }

    if (oal_memcmp(bssid, rx_mgmt_deal->hmac_vap->bssid, WLAN_MAC_ADDR_LEN) == 0) {
        if (rx_mgmt_deal->frame_hdr->frame_control.sub_type == WLAN_PROBE_RSP) {
            /* BEGIN:判断接收到的probe rsp 帧和本VAP 是相同信道,才清零linkloss 计数
               防止由于ap 切换信道后,sta 在其他信道扫到ap 信号不能触发linkloss 去关联 */
            /* 获取probe rsp 中的信道 */
            hmac_vap_linkloss_channel_clean(rx_mgmt_deal->hmac_vap, rx_mgmt_deal->netbuf);
            *go_on = OSAL_FALSE;
            /* END:判断接收到的probe rsp 帧和本VAP 是相同信道,才清零linkloss 计数
               防止由于ap 切换信道后,sta 在其他信道扫到ap 信号不能触发linkloss 去关联 */
        } else {
            /* UP状态下，用非rsp帧以外的管理帧更新RSSI; 非UP状态下，用(re)assoc rsp更新RSSI */
            if ((rx_mgmt_deal->hmac_vap->vap_state == MAC_VAP_STATE_UP) ||
                (rx_mgmt_deal->frame_hdr->frame_control.sub_type == WLAN_ASSOC_RSP) ||
                (rx_mgmt_deal->frame_hdr->frame_control.sub_type == WLAN_REASSOC_RSP)) {
                /* RSSI平滑, 值保存到dmac_vap下 */
                oal_rssi_smooth(&(rx_mgmt_deal->hmac_vap->query_stats.signal), rx_ctl->rx_statistic.rssi_dbm);
                oal_rssi_smooth(&(rx_mgmt_deal->hmac_vap->query_stats.signal_mgmt_data), rx_ctl->rx_statistic.rssi_dbm);

                /* 管理帧更新SNR信息 */
                hmac_vap_update_snr_info(rx_mgmt_deal->hmac_vap, rx_ctl);
            }
        }
        /* STA 接收auth时，置位等待接收auth使能 */
        if (rx_mgmt_deal->frame_hdr->frame_control.sub_type == WLAN_AUTH &&
            rx_mgmt_deal->hmac_vap->auth_received == OSAL_FALSE) {
            rx_mgmt_deal->hmac_vap->auth_received = OSAL_TRUE;
            notify_device = true;
        }
        if (rx_mgmt_deal->frame_hdr->frame_control.sub_type == WLAN_REASSOC_RSP &&
            rx_mgmt_deal->hmac_vap->assoc_rsp_received == OSAL_FALSE) {
            rx_mgmt_deal->hmac_vap->assoc_rsp_received = OSAL_TRUE;
            notify_device = true;
        }

        /* 通知Device不用重传AUTH请求和重关联请求               */
        if (notify_device) {
            hmac_vap_sync(rx_mgmt_deal->hmac_vap);
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_11AX
static osal_void hmac_rx_mu_edca_param_judge(hmac_vap_stru *hmac_vap, osal_u8 *frame_body,
    const mac_ieee80211_frame_stru *frame_hdr, osal_u8 *go_on, const dmac_rx_ctl_stru *rx_ctl)
{
    osal_u8 *ie = OSAL_NULL;
    mac_rx_ctl_stru *rx_info = OSAL_NULL;
    osal_u16 msg_len;
    mac_frame_he_mu_edca_parameter_ie_stru mu_edca_value;
    osal_u32 ret;

    memset_s(&mu_edca_value, sizeof(mu_edca_value), 0, sizeof(mu_edca_value));

    rx_info = (mac_rx_ctl_stru *)(&(rx_ctl->rx_info));
    msg_len = rx_info->frame_len - rx_info->mac_header_len;   /* 消息总长度,不包括FCS */

    /* 来自其它bss的Probe Response帧或者beacon帧不做处理 */
    ret = oal_compare_mac_addr(hmac_vap->bssid, frame_hdr->address3);
    if (ret != 0) {
        return;
    }
    ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_EDCA, frame_body, msg_len);
    if (ie != OSAL_NULL) {
        ret = hmac_ie_parse_mu_edca_parameter(ie, &mu_edca_value);
        if (ret != OAL_SUCC) {
            return;
        }
        if (hmac_vap->he_mu_edca_update_count == mu_edca_value.qos_info.edca_update_count) {
            return;
        }
        *go_on = OSAL_TRUE;
    }

    return;
}

static osal_void hmac_rx_qos_info_judge(hmac_vap_stru *hmac_vap, osal_u8 *frame_body,
    const mac_ieee80211_frame_stru *frame_hdr, osal_u8 *go_on, const dmac_rx_ctl_stru *rx_ctl)
{
    osal_u8 *ie = OSAL_NULL;
    mac_rx_ctl_stru *rx_info = OSAL_NULL;
    osal_u16 msg_len;
    mac_frame_he_mu_qos_info_stru *qos_info = OSAL_NULL;
    osal_u32 ret;

    rx_info = (mac_rx_ctl_stru *)(&(rx_ctl->rx_info));
    msg_len = rx_info->frame_len - rx_info->mac_header_len;   /* 消息总长度,不包括FCS */

    /* 来自其它bss的beacon帧不做处理 */
    ret = oal_compare_mac_addr(hmac_vap->bssid, frame_hdr->address3);
    if (ret != 0 || frame_hdr->frame_control.sub_type == WLAN_PROBE_RSP) {
        return;
    }
    ie = mac_find_ie_etc(MAC_EID_QOS_CAP, frame_body, msg_len);
    if (ie != OSAL_NULL) {
        ie += MAC_IE_HDR_LEN;
        qos_info = (mac_frame_he_mu_qos_info_stru *)ie;
        if (hmac_vap->he_mu_edca_update_count == qos_info->edca_update_count) {
            return;
        }
        *go_on = OSAL_TRUE;
    }

    return;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_rx_filter_mgmt_scan_state
 功能描述  : 对收到的管理帧进行部分过滤处理
*****************************************************************************/
static osal_u32 hmac_rx_filter_mgmt_scan_state(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    osal_u8 *go_on, osal_u8 *report_bss)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 如果当前device处于扫描状态，处理收到的管理帧 */
    if (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) {
        /* 监听处理转移到扫描模块处理时，执行如下代码 */
#ifdef _PRE_WLAN_FEATURE_P2P
        if (hmac_device->scan_params.scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
            /* P2P 设备接收到管理帧 */
            if (!is_legacy_vap((hmac_vap))) {
                *go_on = hmac_p2p_listen_rx_mgmt(hmac_vap, netbuf);
            }
        } else {
#endif
            /* 如果扫描动作关心bss信息，那么进行扫描管理帧过滤，进行对应的处理动作，其它do nothing  */
            if ((hmac_device->scan_params.scan_func & MAC_SCAN_FUNC_BSS) != 0) {
                /* 扫描时也是会偶尔切回工作信道工作,不能过滤掉所有管理帧 */
                /* 扫描状态的帧过滤处理 */
                hmac_scan_mgmt_filter(hmac_vap, netbuf, report_bss, go_on);
            }
            if ((hmac_device->scan_params.scan_func & MAC_SCAN_FUNC_MEAS) != 0) {
                *go_on = OSAL_FALSE;
            }
#ifdef _PRE_WLAN_FEATURE_P2P
        }
#endif
    }

    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_rx_filter_mgmt_comm_state
 功能描述  : 对收到的管理帧进行初始过滤处理
*****************************************************************************/
static osal_u32 hmac_rx_filter_mgmt_comm_state(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    const mac_ieee80211_frame_stru *frame_hdr, const osal_u8 *bssid, osal_u8 *go_on)
{
    /* 获取帧头信息 */
    osal_u8 *frame_body = oal_netbuf_rx_data(netbuf);
    osal_u8 sub_type = frame_hdr->frame_control.sub_type;
    osal_u8 bad_bssid[WLAN_MAC_ADDR_LEN] = {0}; /* 非法bssid，全0 */
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_IS_SDP_FRAME);

    if (fhook != OSAL_NULL && (((hmac_is_sdp_frame_cb)fhook)(netbuf) == OAL_TRUE)) { /* dmac 已经进行了过滤 */
        *go_on = OSAL_TRUE;
        return OAL_SUCC;
    }
    if ((sub_type == WLAN_ACTION) && ((frame_body[MAC_ACTION_OFFSET_CATEGORY] > MAC_ACTION_CATEGORY_VENDOR) ||
        (frame_body[MAC_ACTION_OFFSET_CATEGORY] > MAC_ACTION_CATEGORY_VHT &&
        (ether_is_broadcast(frame_hdr->address1) == OSAL_TRUE) &&
        frame_body[MAC_ACTION_OFFSET_CATEGORY] < MAC_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED))) {
        *go_on = OSAL_FALSE;
        return OAL_SUCC;
    }

    /* xiexiaoming:except action no ack for txbf */
    if (((oal_memcmp(bssid, bad_bssid, WLAN_MAC_ADDR_LEN) == 0) && (sub_type != WLAN_ACTION_NO_ACK))
#ifdef _PRE_WLAN_FEATURE_PMF
        || (hmac_11w_rx_filter(hmac_vap, netbuf) != OAL_SUCC)
#endif
    ) {
        oam_warning_log1(0, 0, "vap_id[%d] {dmac_rx_process_mgmt_not_report::bssid zero packet}", hmac_vap->vap_id);
        *go_on = OSAL_FALSE;
        return OAL_SUCC;
    }

    return OAL_CONTINUE;
}
/*****************************************************************************
 函 数 名  : dmac_rx_process_mgmt_not_report
 功能描述  : 对收到的管理帧进行过滤处理
*****************************************************************************/
static osal_u32 hmac_rx_process_mgmt(hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf, osal_u8 *go_on, osal_u8 *report_bss)
{
    osal_u8 bssid[WLAN_MAC_ADDR_LEN] = {0}; /* sta关联的ap mac地址 */
    osal_u32 ret;

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 获取帧头信息 */
    osal_u8 *frame_body = oal_netbuf_rx_data(netbuf);
#endif
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    osal_u8 sub_type = frame_hdr->frame_control.sub_type;
    hmac_rx_mgmt_deal_stru hmac_rx_mgmt_deal;

    *go_on = OSAL_TRUE; /* 函数返回后是否发到HMAC，初始为true */

    /* hmac_rx_filter_mgmt里可能会更改netbuf内容，所以需先通知算法 */
    hmac_alg_rx_mgmt_notify(hmac_vap, hmac_user, netbuf);

    mac_get_bssid((osal_u8 *)frame_hdr, bssid, sizeof(bssid));

    if (sub_type == WLAN_BEACON) {
        rx_ctl->rx_info.is_beacon = OSAL_TRUE;
    }

    /* STAUT的beacon需优先尽快处理，待机时尽早提前关闭前端节能 */
    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) && (sub_type == WLAN_BEACON)) {
        hmac_rx_sta_up_beacon_mgmt(hmac_vap, (const osal_u8 *)bssid, netbuf, go_on);
    }

    /* HMAC_FRAME_HOOK_RX_MGMT_PRE_FILTER */

    ret =  hmac_rx_filter_mgmt_comm_state(hmac_vap, netbuf, frame_hdr, (const osal_u8 *)bssid, go_on);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    hmac_rx_mgmt_deal.hmac_vap = hmac_vap;
    hmac_rx_mgmt_deal.frame_hdr = frame_hdr;
    hmac_rx_mgmt_deal.netbuf = netbuf;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_rx_ap_mgmt(&hmac_rx_mgmt_deal, (const osal_u8 *)bssid, WLAN_MAC_ADDR_LEN, go_on, rx_ctl);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_rx_sta_mgmt(&hmac_rx_mgmt_deal, (const osal_u8 *)bssid, WLAN_MAC_ADDR_LEN, go_on, rx_ctl);
    }

    ret =  hmac_rx_filter_mgmt_scan_state(hmac_vap, netbuf, go_on, report_bss);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    /* 如果在扫描中已经上报，此处不应该再上报该netbuf,并且不能释放netbuf */
    /* 配置VAP收到的管理报文要丢弃，后续可优化配置VAP收包 */
    if ((*report_bss == OSAL_TRUE) || (hmac_vap->mib_info == OAL_PTR_NULL)) {
        *go_on = OSAL_FALSE;
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if ((sub_type == WLAN_PROBE_RSP || sub_type == WLAN_BEACON) &&
        mac_mib_get_he_option_implemented(hmac_vap) == OSAL_TRUE) {
        hmac_rx_mu_edca_param_judge(hmac_vap, frame_body, frame_hdr, go_on, rx_ctl);
        hmac_rx_qos_info_judge(hmac_vap, frame_body, frame_hdr, go_on, rx_ctl);
    }
#endif

    return OAL_SUCC;
}

static osal_void hmac_rx_mgmt_ctrl_status_record(const hmac_vap_stru *hmac_vap, osal_u8 sub_type,
    oal_bool_enum_uint8 go_on, oal_bool_enum_uint8 report_bss)
{
#ifdef _PRE_WLAN_DFT_STAT
    if ((dfx_get_service_control() & SERVICE_CONTROL_MGMT_FRAME_RX) != 0) {
        if ((sub_type == WLAN_ASSOC_REQ) || (sub_type == WLAN_ASSOC_RSP) || (sub_type == WLAN_AUTH) ||
            (sub_type == WLAN_DEAUTH) || (sub_type == WLAN_DISASOC)) {
            wifi_printf("[service]hmac_rx_mgmt_ctrl_classify_filter vap[%d]mode[%d] recv %d go_on %d report_bss %d\r\n",
                hmac_vap->vap_id, hmac_vap->vap_mode, sub_type, go_on, report_bss);
        }
    }
#endif
}

static osal_void hmac_rx_mgmt_update_user_rssi(hmac_user_stru *hmac_user, dmac_rx_ctl_stru *rx_ctl)
{
    if (hmac_user != OSAL_NULL) { /* 用户存在 */
        /* 更新关联用户的RSSI统计信息 */
        oal_rssi_smooth(&(hmac_user->rx_rssi), rx_ctl->rx_statistic.rssi_dbm);
    }
}

/*****************************************************************************
 函 数 名  : hmac_rx_mgmt_ctrl_classify
 功能描述  : DMAC模块，接收流程管理帧处理入口，主要用于帧在DMAC或者HMAC的分发
*****************************************************************************/
static osal_s32 hmac_rx_mgmt_ctrl_classify(oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
    osal_s32 ret_rx_ctrl;
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf); /* 获取帧头信息 */
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    hmac_user_stru *hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(rx_ctl->rx_info.ta_user_idx);
    oal_bool_enum_uint8 go_on = OSAL_TRUE; /* 是否继续到hmac处理 */
    oal_bool_enum_uint8 report_bss = OSAL_FALSE;
    osal_u8 sub_type = frame_hdr->frame_control.sub_type; /* 报文子帧类型，避免异常分支释放netbuf后再次访问frame_hdr */

    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(rx_ctl->rx_info.mac_vap_id);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_mgmt_ctrl_classify::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 收包HOOK回调处理 */
    ret = hmac_call_netbuf_hooks(&netbuf, hmac_vap, HMAC_FRAME_MGMT_RX_EVENT_FEATURE);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }

    hmac_rx_mgmt_update_user_rssi(hmac_user, rx_ctl);

    /* 控制帧处理，目前没有上报到HMAC，直接return */
    if (frame_hdr->frame_control.type == WLAN_CONTROL) {
        hmac_rx_process_ctrl(hmac_vap, hmac_user, netbuf, &go_on);
    } else if (frame_hdr->frame_control.type == WLAN_MANAGEMENT) { /* DMAC管理帧处理 */
        hmac_rx_process_mgmt(hmac_vap, hmac_user, netbuf, &go_on, &report_bss);
    }

    hmac_rx_mgmt_ctrl_status_record(hmac_vap, sub_type, go_on, report_bss);

    if (go_on == OSAL_FALSE && report_bss == OSAL_FALSE) {
        /* 将netbuf归还内存池 */
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    if (go_on == OSAL_TRUE) {
        ret_rx_ctrl = hmac_rx_process_report_mgmt_ctrl(hmac_vap, netbuf);
        if (ret_rx_ctrl != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_ANY, "vap[%d]{hmac_location_send_event_to_host:failed[%d]",
                hmac_vap->vap_id, ret_rx_ctrl);
        }
        return ret_rx_ctrl;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 处理来自其他BSS的广播管理帧
*****************************************************************************/
static osal_u32 hmac_rx_multi_mgmt_ctrl_frame(oal_netbuf_stru *netbuf, dmac_rx_ctl_stru *cb_ctrl)
{
    oal_netbuf_stru *netbuf_copy = OSAL_NULL;
    dmac_rx_ctl_stru *rx_ctrl = cb_ctrl;
    oal_bool_enum_uint8 orig_netbuffer_used = OSAL_FALSE;
    mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctrl->rx_info));
    osal_u8 channel_number = rx_ctrl->rx_info.channel_number;
    osal_u8 subtype = mac_get_frame_type_and_subtype((osal_u8 *)mac_header);
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 vap_idx;

    /* 其他BSSID的帧 */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        osal_s32 rslt;
        hmac_vap_stru *hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (hmac_vap == OSAL_NULL) {
            oam_warning_log1(0, 0, "{hmac_rx_multi_mgmt_ctrl_frame::hmac_vap null, vap id=%d.",
                hmac_device->vap_id[vap_idx]);
            continue;
        }

        if (hmac_rx_multi_mgmt_pre_process(hmac_device, hmac_vap, channel_number, subtype) == OSAL_FALSE) {
            // beacon do not dispatch to the vap which do not needed.}");
            continue;
        }

        /* 最后一个VAP直接发送原来的netbuf */
        if (vap_idx == hmac_device->vap_num - 1) {
            netbuf_copy = netbuf;
            orig_netbuffer_used = OSAL_TRUE;
        } else {
            osal_u32 frame_body_len;
            netbuf_copy = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
            if (netbuf_copy == OSAL_NULL) {
                oam_warning_log0(0, OAM_SF_RX, "{hmac_rx_multi_mgmt_ctrl_frame::alloc netbuff failed.}");
                continue;
            }
            rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf_copy);
            /* 信息复制 */
            memcpy_s(oal_netbuf_cb(netbuf_copy), sizeof(dmac_rx_ctl_stru),
                oal_netbuf_cb(netbuf), sizeof(dmac_rx_ctl_stru));
            memcpy_s(oal_netbuf_header(netbuf_copy), (osal_u32)(rx_ctrl->rx_info.mac_header_len),
                oal_netbuf_header(netbuf), (osal_u32)(rx_ctrl->rx_info.mac_header_len));
            frame_body_len = (osal_u32)(rx_ctrl->rx_info.frame_len) -
                (osal_u32)(rx_ctrl->rx_info.mac_header_len);
            memcpy_s(oal_netbuf_data_offset(netbuf_copy, rx_ctrl->rx_info.mac_header_len), frame_body_len,
                oal_netbuf_data_offset(netbuf, rx_ctrl->rx_info.mac_header_len), frame_body_len);
            oal_netbuf_put(netbuf_copy, rx_ctrl->rx_info.frame_len);

            /* 调整MAC帧头的指针(copy后，对应的mac header的头已经发生变化) */
            mac_set_rx_cb_mac_hdr(&(rx_ctrl->rx_info), (osal_u32 *)oal_netbuf_header(netbuf_copy));
        }

        /* cb域中的mac_vap_id还是复制前的值，需要更新，后续流程需要使用mac_vap_id */
        rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf_copy);
        rx_ctrl->rx_info.mac_vap_id = hmac_vap->vap_id;

        /* 未使用传入的netbuffer， 直接释放 */
        rslt = hmac_rx_mgmt_ctrl_classify(netbuf_copy);
        if (rslt != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_RX, "{hmac_rx_multi_mgmt_ctrl_frame::classify failed[%d].", rslt);
            oal_netbuf_free(netbuf_copy);
        }
    }

    /* 未使用传入的netbuffer， 直接释放 */
    if (orig_netbuffer_used == OSAL_FALSE) {
        oal_netbuf_free(netbuf);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_process_mgmt_ctrl
 功能描述  : 管理帧总入口
*****************************************************************************/
osal_s32 hmac_rx_process_mgmt_ctrl(oal_netbuf_stru *netbuf)
{
    osal_void *fhook = OSAL_NULL;
    dmac_rx_ctl_stru *cb_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    osal_u8 hal_vap_id = cb_ctrl->rx_info.vap_id; /* 获取该帧对应的VAP ID */

    /* host侧RX管理帧上报HSO */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_FRAME_REPORT_HSO);
    if (fhook != OSAL_NULL) {
        ((dfx_frame_report_hso_cb)fhook)(netbuf, OAM_OTA_FRAME_DIRECTION_TYPE_RX);
    }
    /* HMAC_PROT_HOOK_RX_IN */
    if (hal_vap_id < HAL_MAX_VAP_NUM) { /* 来自本device下的某一BSS的帧 */
        return hmac_rx_mgmt_ctrl_classify(netbuf);
    } else if ((hal_vap_id == WLAN_HAL_OHTER_BSS_ID) ||  /* 来自其他bss的广播管理帧 */
               (hal_vap_id == WLAN_HAL_OTHER_BSS_OTHER_ID)) { /* 来自其他bss的组播管理帧 */
        if (cb_ctrl->rx_info.frame_len > WLAN_MGMT_NETBUF_SIZE) {
            oam_warning_log2(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_mgmt_ctrl: frame exceed mem size.len[%d]}",
                hal_vap_id, cb_ctrl->rx_info.frame_len);
            return OAL_ERR_CODE_MSG_LENGTH_ERR;
        }

        return (osal_s32)hmac_rx_multi_mgmt_ctrl_frame(netbuf, cb_ctrl);
    } else { /* 异常帧 */
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }
}

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
/* 注意：消息接收端需要释放内存 */
osal_void hmac_rx_send_report_frame_msg(hmac_vap_stru *hmac_vap, const mac_rx_ctl_stru *rx_info,
    osal_s8 rssi, osal_u16 msg_id)
{
    frw_msg msg_info = {0};
    osal_s32 ret;
    osal_u8 *payload = OSAL_NULL;
    hmac_rx_mgmt_event_stru frame = {0};

    payload = (osal_u8 *)osal_kmalloc(rx_info->frame_len, 0);
    if (payload == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_report_mgmt_frame::mgmt_data null.}");
        return;
    }
    if (memcpy_s(payload, rx_info->frame_len,
        (const osal_u8 *)mac_get_rx_cb_mac_header_addr((mac_rx_ctl_stru *)rx_info), rx_info->frame_len) != EOK) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_report_mgmt_frame::memcpy_s data error.}");
        osal_kfree(payload);
        return;
    }

    frame.buf = (osal_u8 *)payload;
    frame.len  = rx_info->frame_len;
    frame.rssi = (osal_u8)rssi;

    msg_info.data = (osal_u8 *)&frame;
    msg_info.data_len = OAL_SIZEOF(hmac_rx_mgmt_event_stru);

    ret = frw_asyn_host_post_msg(msg_id, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        osal_kfree(payload);
    }
    return;
}

OAL_STATIC osal_void hmac_rx_report_mgmt_frame(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf)
{
    const dmac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    const mac_rx_ctl_stru *rx_info = OSAL_NULL;
    const osal_u8 *mac_hdr = OSAL_NULL;
    osal_u8 frame_type, sub_type;

    if (hmac_is_mgmt_report_enable() == OSAL_FALSE) {
        return;
    }

    rx_ctl = (const dmac_rx_ctl_stru *)oal_netbuf_cb_const(netbuf);
    rx_info = (const mac_rx_ctl_stru *)(&(rx_ctl->rx_info));
    /* 非本bss帧不上报（广播帧除外） */
    if ((rx_info->vap_id >= HAL_MAX_VAP_NUM) && (rx_info->vap_id != WLAN_HAL_OHTER_BSS_ID)) {
        return;
    }
    mac_hdr = (const osal_u8 *)mac_get_rx_cb_mac_header_addr((mac_rx_ctl_stru *)rx_info);
    frame_type = mac_frame_get_type_value(mac_hdr);
    if (frame_type != WLAN_MANAGEMENT) {
        return;
    }
    sub_type = mac_get_frame_type_and_subtype(mac_hdr);
    /* beacon不上报 */
    if (sub_type == (WLAN_FC0_SUBTYPE_BEACON | WLAN_FC0_TYPE_MGT)) {
        return;
    }

    hmac_rx_send_report_frame_msg(hmac_vap, rx_info, rx_ctl->rx_statistic.rssi_dbm, WLAN_MSG_H2W_UPLOAD_RX_MGMT);

    return;
}
#endif

/*
 * 函 数 名  : hmac_rx_cut_process_mgmt_ctrl_event
 * 功能描述  : 83裁剪版本，从dmac.bin接收rx报文，dmac.ko处理总入口，包括管理帧和控制帧
 *           直接替代原处理dmac_rx_process_mgmt_ctrl
 */
osal_s32 hmac_rx_cut_process_mgmt_ctrl_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    oal_netbuf_stru *netbuf = OSAL_NULL;

    netbuf = *((oal_netbuf_stru **)(msg->data));
    if (osal_unlikely(netbuf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_cut_process_mgmt_ctrl_event::netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 管理帧收包HOOK回调处理 */
    ret = (osal_s32)hmac_call_netbuf_hooks(&netbuf, hmac_vap, HMAC_FRAME_MGMT_RX_EVENT_D2H);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    hmac_rx_report_mgmt_frame(hmac_vap, netbuf);
#endif

    /* 调用管理帧接口 */
    if (hmac_rx_process_mgmt_ctrl(netbuf) != OAL_SUCC) {
        oal_netbuf_free(netbuf);
    }

    return OAL_SUCC;
}

osal_s32 hmac_rx_mgmt_event_adapt(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_netbuf_stru *netbuf;
    osal_s32 ret = frw_asyn_host_post_msg(WLAN_MSG_H2H_CRX_RX, FRW_POST_PRI_HIGH, hmac_vap->vap_id, msg);
    if (ret == OAL_ERR_CODE_ALLOC_MEM_FAIL) {
        netbuf = *((oal_netbuf_stru **)(msg->data));
        oal_netbuf_free(netbuf);
    }
    return ret;
}

/*****************************************************************************
 功能描述  : DMAC模块根据CB字段的subtype，做ACTION发送处理接口
*****************************************************************************/
static osal_u8 hmac_tx_process_action(hmac_vap_stru *hmac_vap, const mac_tx_ctl_stru *tx_ctl,
    oal_netbuf_stru *action_frame)
{
    hmac_ctx_action_event_stru *ctx_action_event = OSAL_NULL;
    osal_u32 ret = OAL_SUCC;
    osal_u32 action_info_offset;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    if (tx_ctl->mpdu_payload_len < sizeof(hmac_ctx_action_event_stru)) {
        common_log_dbg1(0, OAM_SF_TX, "{hmac_tx_process_action::MPDU_LEN[%d] err}", tx_ctl->mpdu_payload_len);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, action_frame);
        oal_netbuf_free(action_frame);
        return OSAL_FALSE;
    }

    /* 管理帧后部保存相关信息 */
    action_info_offset = tx_ctl->mpdu_payload_len - sizeof(hmac_ctx_action_event_stru);
    ctx_action_event = (hmac_ctx_action_event_stru *)(oal_netbuf_tx_data(action_frame) + action_info_offset);

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(ctx_action_event->user_idx);
    if (hmac_user == OSAL_NULL) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, action_frame);
        common_log_dbg1(0, OAM_SF_BA, "{hmac_tx_process_action:dmac user[%d]nul}", ctx_action_event->user_idx);
        oal_netbuf_free(action_frame);
        return OSAL_FALSE;
    }

    if (tx_ctl->frame_subtype == WLAN_ACTION_BA_ADDBA_REQ) {
        ret = hmac_mgmt_tx_addba_req_send(hmac_vap, hmac_user, ctx_action_event, action_frame);
    } else if (tx_ctl->frame_subtype == WLAN_ACTION_BA_ADDBA_RSP) {
        ret = hmac_mgmt_tx_addba_rsp_send(hmac_vap, hmac_user, ctx_action_event, action_frame);
    } else if (tx_ctl->frame_subtype == WLAN_ACTION_BA_DELBA) {
        ret = hmac_mgmt_tx_delba(hmac_vap, hmac_user, ctx_action_event, action_frame);
    } else if ((tx_ctl->frame_subtype == WLAN_ACTION_TWT_SETUP_REQ) ||
               (tx_ctl->frame_subtype == WLAN_ACTION_TWT_TEARDOWN_REQ) ||
               (tx_ctl->frame_subtype == WLAN_ACTION_TWT_INFORMATION_REQ)) {
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_TX_ACTION);
        if (fhook != OSAL_NULL) {
            ret = ((hmac_twt_tx_action_cb)fhook)(hmac_vap, hmac_user, ctx_action_event, action_frame, tx_ctl);
        }
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    } else if (tx_ctl->frame_subtype == WLAN_ACTION_WUR_MODE_SETUP) {
        ret = dmac_mgmt_tx_wur_mode_setup(hmac_vap, hmac_user, ctx_action_event, action_frame);
    } else if (tx_ctl->frame_subtype == WLAN_ACTION_WUR_MODE_TEARDOWN) {
        ret = dmac_mgmt_tx_wur_mode_teardown(hmac_vap, hmac_user, ctx_action_event, action_frame);
#endif
    } else {
        return OSAL_TRUE;
    }

    /* 管理帧发送失败,统一释放内存 */
    if (ret != OAL_SUCC) {
        common_log_dbg3(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_process_action::process action ret[%d] subtype[%d].}",
            hmac_vap->vap_id, ret, tx_ctl->frame_subtype);
        oal_netbuf_free(action_frame);
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_process_mgmt_event
 功能描述  : 通用的（那些不需要dmac做任何业务处理的管理帧，
                              认证关联）处理hmac抛来的发送管理帧事件，填写相应cb参数，发送管理帧
*****************************************************************************/
osal_u32 hmac_tx_process_mgmt_event(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
    osal_u16 frame_len;
    hmac_device_stru *hmac_device = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u8 mgmt_frm_type;
    frw_msg msg_info = {0};

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    frame_len = (osal_u16)(tx_ctl->mpdu_payload_len + tx_ctl->frame_header_length);

    oal_set_netbuf_next(netbuf, OSAL_NULL);
    oal_set_netbuf_prev(netbuf, OSAL_NULL);

    /* 获取device结构的信息 */
    hmac_device = hmac_res_get_mac_dev_etc(0);

    if (tx_ctl->ether_head_including == OSAL_TRUE) {
        oam_error_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_process_mgmt_event::EER.}", hmac_vap->vap_id);
    }

    /* 处理host过来的action帧 */
    if (tx_ctl->frame_type == WLAN_CB_FRAME_TYPE_ACTION) {
        if (hmac_tx_process_action(hmac_vap, tx_ctl, netbuf) != OSAL_TRUE) {
            return OAL_SUCC;
        }
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    mgmt_frm_type = mac_get_frame_type_and_subtype((osal_u8 *)oal_netbuf_header(netbuf));
    if (mgmt_frm_type == (WLAN_FC0_SUBTYPE_AUTH | WLAN_FC0_TYPE_MGT)) {
        hmac_user_stru *hmac_user = mac_res_get_hmac_user_etc(tx_ctl->tx_user_idx);
        if (hmac_user != OSAL_NULL) {
            hmac_tid_clear(hmac_user, hmac_device, OSAL_FALSE);
            /* 同步清除Device侧相关BA信息 */
            frw_msg_init((osal_u8 *)&(hmac_user->assoc_id), sizeof(osal_u16), OSAL_NULL, 0, &msg_info);
            ret = (osal_u32)frw_send_msg_to_device(hmac_vap->vap_id,
                WLAN_MSG_H2D_C_CFG_DEVICE_RESET_BA, &msg_info, OSAL_TRUE);
            if (ret != OAL_SUCC) {
                oam_warning_log2(hmac_vap->vap_id, OAM_SF_CFG,
                    "hmac_tx_process_mgmt_event:vap[%d]frw_send_msg_to_device RESET_BA err[%d]", hmac_vap->vap_id, ret);
            }
        }
    }

    return hmac_tx_mgmt(hmac_vap, netbuf, frame_len, OSAL_TRUE);
}
/*****************************************************************************
 函 数 名  : hmac_rx_process_sync_event
 功能描述  : 收到wlan的Delba和addba rsp用于到dmac的同步
*****************************************************************************/
osal_s32 hmac_rx_process_ba_sync_event(hmac_vap_stru *hmac_vap, hmac_ctx_action_event_stru *crx_action_sync)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (osal_unlikely(hmac_vap == OSAL_NULL) || osal_unlikely(crx_action_sync == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_process_sync_event::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取vap结构信息 */
    oam_info_log2(0, OAM_SF_RX, "{hmac_rx_process_sync_event::category %d, action %d.}",
        crx_action_sync->action_category, crx_action_sync->action);
    switch (crx_action_sync->action_category) {
        case MAC_ACTION_CATEGORY_BA:
            switch (crx_action_sync->action) {
                case MAC_BA_ACTION_ADDBA_RSP:
                    hmac_mgmt_rx_addba_rsp(hmac_device, (hmac_vap_stru *)hmac_vap, crx_action_sync);
                    break;

                case MAC_BA_ACTION_DELBA:
                    hmac_mgmt_rx_delba((hmac_vap_stru *)hmac_vap, crx_action_sync);
                    break;

                default:
                    oam_warning_log1(0, OAM_SF_RX,
                        "vap_id[%d] {hmac_rx_process_sync_event::invalid ba action type.}", hmac_vap->vap_id);
                    break;
            }
            break;

        default:
            oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_rx_process_sync_event::invalid ba action category.}",
                hmac_vap->vap_id);
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_connect_set_channel
 功能描述  : STA入网设信道或者AP设置信道接口
*****************************************************************************/
osal_void hmac_mgmt_connect_set_channel(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device,
    hmac_vap_stru *up_vap, mac_channel_stru *channel)
{
    hal_to_dmac_device_stru *up_vap_hal_device = OSAL_NULL;

    /* 当前没有处在关联状态的VAP，可以直接切信道 */
    if (up_vap == OSAL_NULL) {
        oam_warning_log4(0, OAM_SF_SCAN, "hmac_mgmt_connect_set_channel: vap_band=%d, chan=%d chan_idx=%d, bw=%d",
            channel->band, channel->chan_number, channel->chan_idx, channel->en_bandwidth);
        /* 切换信道需要清fifo，传入TRUE */
        /* set freq命令会清空FIFO，软件长发起来后不能直接切信道，切信道后需要先关长发再开长发才会生效 */
        hmac_mgmt_switch_channel(hal_device, channel, OSAL_TRUE);
        return;
    }

    up_vap_hal_device = ((hmac_vap_stru *)up_vap)->hal_device;

    /* 新入网和已经up的不是在同一个hal device上,后入网的可以直接在自己的hal device上配置信道 */
    /* 工作的vap已经是pause状态,说明已经做过一次保护切离,不需要第二次保护切离,否则会将硬件队列的包搬到此vap,发错信道!!! */
    if ((up_vap_hal_device != hal_device) || (up_vap->vap_state == MAC_VAP_STATE_PAUSE)) {
        oam_warning_log3(0, OAM_SF_DBDC,
            "vap_id[%d] hmac_mgmt_connect_set_channel: up vap[%d]state[%d] set channel directly!!!", up_vap->vap_id,
            up_vap->vap_id, up_vap->vap_state);
        hmac_mgmt_switch_channel(hal_device, channel, OSAL_TRUE);
        return;
    }

    oam_warning_log4(0, OAM_SF_SCAN,
        "hmac_mgmt_connect_set_channel: has up vap. up_vap_chan:%d bw:%d, new_chan:%d bw:%d",
        up_vap->channel.chan_number, up_vap->channel.en_bandwidth, channel->chan_number, channel->en_bandwidth);

    if (channel->chan_number != up_vap->channel.chan_number) {
        hmac_vap_pause_tx(up_vap);
        oam_warning_log2(0, 0, "vap_id[%d] hmac_mgmt_connect_set_channel: diff chan_num. switch off to chan %d.",
            up_vap->vap_id, channel->chan_number);
        /* 发保护帧的信道切离 */
        hmac_switch_channel_off(hmac_device, up_vap, channel, HAL_FCS_PROTECT_CNT_20);
    } else {
        /* 同信道时需要切到更大带宽的信道 */
        if (channel->en_bandwidth > up_vap->channel.en_bandwidth) {
            oam_warning_log3(0, OAM_SF_SCAN,
                "vap_id[%d] hmac_mgmt_connect_set_channel: same chan_num[%d], switch to bw[%d].",
                up_vap->vap_id, channel->chan_number, channel->en_bandwidth);

            /* 切换信道不需要清fifo，传入FALSE */
            hmac_mgmt_switch_channel(hal_device, channel, OSAL_FALSE);
        }
    }
}

OSAL_STATIC osal_void hmac_join_enable_tbtt(hmac_vap_stru *hmac_vap,
    hmac_ctx_join_req_set_reg_stru *reg_params)
{
    /* 同步beacon period */
    mac_mib_set_beacon_period(hmac_vap, reg_params->beacon_period);

    if (mac_mib_get_beacon_period(hmac_vap) != 0) {
        oam_warning_log2(0, OAM_SF_SCAN, "vap_id[%d] {hmac_join_set_reg_event_process_ext::beacon period[%d]}",
            hmac_vap->vap_id, reg_params->beacon_period);
        /* 将beacon的周期写入寄存器 */
        hal_vap_set_psm_beacon_period(hmac_vap->hal_vap, reg_params->beacon_period);
        /* 启动host侧tbtt定时器 */
        frw_create_timer_entry(&(hmac_vap->sta_tbtt.tbtt_timer), hmac_tbtt_event_sta, reg_params->beacon_period,
            hmac_vap, OSAL_TRUE);
    }
    /* 开启sta tbtt定时器 */
    hal_enable_tsf_tbtt(hmac_vap->hal_vap, OSAL_FALSE);

    /* 同步beacon周期至device。解决dbac下，两个vap beacon周期不一致导致时隙分配错误问题 */
    hmac_vap_sync(hmac_vap);
}
static osal_void hmac_join_set_reg_event_process_ext(hmac_vap_stru *hmac_vap,
    hmac_ctx_join_req_set_reg_stru *reg_params)
{
    osal_u8 *cur_ssid = OSAL_NULL;
    osal_void *fhook;

    /* 写STA BSSID寄存器 */
    hal_set_sta_bssid(hmac_vap->hal_vap, reg_params->bssid, WLAN_MAC_ADDR_LEN);

    /* 清除未认证AP前统计到该bsscolor */
    hal_color_rpt_en(OSAL_TRUE);

    hmac_join_enable_tbtt(hmac_vap, reg_params);

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态直接从up->roam,状态同步无法配置信道还在这里设置 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        hmac_vap_work_set_channel(hmac_vap);
    }
#endif // _PRE_WLAN_FEATURE_ROAM

    /* 如果是02模式下，需要将信道信息同步到dmac vap结构体中 */
    (osal_void)memcpy_s(&(hmac_vap->channel), sizeof(mac_channel_stru),
        &(reg_params->current_channel), sizeof(mac_channel_stru));

    /* 初始化默认接收beacon帧 */
    /* 防止其他操作中修改寄存器，再写寄存器接收beacon帧 */
    if (reg_params->beacon_filter == OSAL_FALSE) {
        /* 关闭beacon帧过滤  */
        hal_disable_beacon_filter();
    }

    /* 初始化默认不接收non_direct_frame帧 */
    /* 为防止其他操作中修改寄存器再写寄存器不接收non_direct_frame帧 */
    if (reg_params->non_frame_filter == OSAL_TRUE) {
        /* 打开non frame帧过滤 */
        hal_enable_non_frame_filter();
    }

    /* 入网优化，不同频段下的能力不一样 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        mac_mib_set_spectrum_management_required(hmac_vap, OSAL_FALSE);
    } else {
        mac_mib_set_spectrum_management_required(hmac_vap, OSAL_TRUE);
    }
    /* 同步更新device的FortyMHzOperationImplemented mib项 */
    mac_mib_set_forty_mhz_operation_implemented(hmac_vap, reg_params->dot11_40mhz_operation_implemented);

    /* 更新ssid */
    cur_ssid = mac_mib_get_desired_ssid(hmac_vap);
    (osal_void)memcpy_s(cur_ssid, WLAN_SSID_MAX_LEN, reg_params->ssid, WLAN_SSID_MAX_LEN);
    cur_ssid[WLAN_SSID_MAX_LEN - 1] = '\0';

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_UPDATE_USER_MAC);
    if (fhook != OSAL_NULL) {
        ((hmac_roam_update_user_mac_cb)fhook)(hmac_vap, reg_params);
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* 设置速率 */
        hmac_vap_flush_rate(hmac_vap, &reg_params->min_rate);
    }
}

/*****************************************************************************
 函 数 名  : hmac_join_set_reg_event_process
 功能描述  : 更新join req时，设置相应寄存器
*****************************************************************************/
osal_s32 hmac_join_set_reg_event_process(hmac_vap_stru *hmac_vap, hmac_ctx_join_req_set_reg_stru *reg_params)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_device_stru *hmac_device = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;
#endif
    osal_u8 old_chan_number, old_bandwith;

    if (osal_unlikely(hmac_vap == OSAL_NULL) || osal_unlikely(reg_params == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_IRQ, "{hmac_join_set_reg_event_process::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SCAN, "vap[%d] hmac_join_set_reg_event_process hal_device null.", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap->cap_flag.dpd_enbale = OSAL_TRUE;

    hmac_vap->cap_flag.dpd_done = OSAL_FALSE;

    old_chan_number = hmac_vap->channel.chan_number;
    old_bandwith = hmac_vap->channel.en_bandwidth;
    /* 同步dmac vap的信道信息 */
    hmac_vap->channel = reg_params->current_channel;
    if (old_chan_number != hmac_vap->channel.chan_number || old_bandwith != hmac_vap->channel.en_bandwidth) {
        hmac_vap_sync(hmac_vap);
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        hmac_vap_down_notify(hmac_vap);
    }
#endif

    /* 需要放置在hmac_config_m2s_assoc_state_syn之前，不然单sta关联时候，这里就找到全0地址误认为是黑名单 */
    (osal_void)memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN, reg_params->bssid, WLAN_MAC_ADDR_LEN);

#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_device = hmac_res_get_mac_dev_etc(0);
    /* 根据是否是黑名单ap 刷新miso_hold状态;需要在dbdc start判断之前 */
    hmac_config_m2s_assoc_state_syn(hmac_vap);

    /* sta模式启动，需要在dbdc之后，此时dbdc标志已经置了，如果是异频dbac，需要在set channel来做，5g c1
     * siso的话无法启动dbdc，进行规格siso切换判断 */
    if (mac_vap_spec_is_sw_need_m2s_switch(hmac_vap)) {
        oam_warning_log1(0, OAM_SF_M2S, "vap[%d]hmac_join_set_reg_event_process:update sw to SISO", hmac_vap->vap_id);

        /* 整体方案修订为: 只刷新软件能力，如果有其他业务要执行切换继续执行，优先级冲突通过能力来判断约束 */
        hmac_m2s_spec_update_vap_sw_capbility(hmac_device, hmac_vap, WLAN_SINGLE_NSS);
    }
    /* 如果触发了在siso状态，漫游状态时，如果是从2G到5G，判断系统当前是否需要回mimo */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_M2S_CHOOSE_MIMO_SISO);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_m2s_choose_mimo_siso_cb)fhook)(hal_device);
    }

    /* 如果不是启动在dbdc，如果mss_on,需要考虑会mimo,判断主路hal device; 如果是漫游触发，mss会回mimo，不处理 */
    if (hal_m2s_check_command_on(hal_device) == OSAL_TRUE && hmac_vap->vap_state != MAC_VAP_STATE_ROAMING) {
        /* MSS存在时，不会有其他m2s业务存在，此时直接回mimo即可; 标志和切换保护功能在此回mimo时清 */
        hmac_m2s_command_mode_force_to_mimo(hal_device);
    }
#endif

    hmac_join_set_reg_event_process_ext(hmac_vap, reg_params);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_process_del_sta
 功能描述  : 处理dmac抛来的user del
*****************************************************************************/
osal_u32 hmac_process_del_sta(osal_u8 vap_id)
{
    hmac_vap_stru *vap_up = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 如果有vap up，则恢复到那个vap所在的信道 */
    vap_up = mac_device_find_another_up_vap_etc(hmac_device, vap_id);
    if ((vap_up != OSAL_NULL) && (vap_up->vap_state == MAC_VAP_STATE_PAUSE)) {
        hal_to_dmac_device_stru *hal_device = ((hmac_vap_stru *)vap_up)->hal_device;
        if (hal_device != OSAL_NULL) {
            hal_device->hal_scan_params.home_channel = vap_up->channel;
            oam_warning_log2(0, 0, "hmac_process_del_sta: conn fail, switch to up vap%d, channel num:%d",
                vap_up->vap_id, vap_up->channel.chan_number);
            hmac_scan_switch_channel_back(hmac_device, hal_device);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 配置multi bssid
*****************************************************************************/
osal_s32 hmac_set_multi_bssid(hmac_vap_stru *hmac_vap, mac_scanned_all_bss_info *all_bss_info)
{
    osal_s32 ret;
    frw_msg msg_info = {0};

    if (osal_unlikely(hmac_vap == OSAL_NULL) || osal_unlikely(all_bss_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_set_multi_bssid::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    /* 配置dmac_vap中multi bssid相关信息 */
    hmac_vap->all_bss_info = *all_bss_info;

    /* 配置寄存器 */
    if ((hmac_vap->hal_vap != OSAL_NULL) && (hmac_vap->hal_vap->vap_id != 0)) {
        oam_error_log1(0, OAM_SF_11AX, "{hmac_set_multi_bssid::vap_id[%d] error.}", hmac_vap->hal_vap->vap_id);
        return OAL_FAIL;
    }
    hal_vap_set_multi_bssid(hmac_vap->all_bss_info.trans_bssid, hmac_vap->all_bss_info.maxbssid_indicator);

    cfg_msg_init((osal_u8 *)all_bss_info, OAL_SIZEOF(mac_scanned_all_bss_info), OAL_PTR_NULL, 0, &msg_info);

    /* 抛事件至Device侧DMAC， 同步all_bss_info信息 */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_STA_MULTI_BSSID_SET, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_set_multi_bssid::frw_send_msg_to_device failed[%d].}", ret);
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_rx_multi_mgmt_pre_process
 功能描述  : 预处理来自其他BSS的广播管理帧，根据本vap 状态和帧类型判断是否需要处理。
            probe req 帧(广播)
            （1）如果是go/ap ，接收到probe req 信道和本vap 工作信道不同，则不做复制和进一步处理。
            （2）如果是sta， 接收到prob req ，不做复制和处理。
            （3）如果是p2p dev 或者p2p cl,接收probe req判断如果是监听状态，则复制，否则不复制           beacon 帧
            （1）ap/p2p go, 信道选择时的扫描接收到的beacon 帧需要处理
            （2）sta/p2p gc/p2p dev 一直需要beacon帧        action帧(广播)
            （1）广播action 帧，复制给其他vap
*****************************************************************************/
static oal_bool_enum_uint8 hmac_rx_multi_mgmt_pre_process(hmac_device_stru *hmac_device, const hmac_vap_stru *hmac_vap,
    osal_u8 channel_number, osal_u8 mgmt_type)
{
    oal_bool_enum_uint8 need_copy = OSAL_TRUE;

    /* 判断接收到的beacon / probe req 是否允许复制给其他vap 处理 */
    switch (mgmt_type) {
        case (WLAN_FC0_SUBTYPE_PROBE_REQ | WLAN_FC0_TYPE_MGT):
            if (is_ap(hmac_vap) && channel_number != hmac_vap->channel.chan_number) {
                need_copy = OSAL_FALSE;
            } else if (is_sta(hmac_vap) && (is_legacy_vap(hmac_vap) ||
                (mac_device_is_listening(hmac_device) == OSAL_FALSE))) {
                need_copy = OSAL_FALSE;
            }
            break;
        default:
            need_copy = OSAL_TRUE;
            break;
    }

    return need_copy;
}

/*****************************************************************************
 函 数 名  : hmac_join_set_dtim_reg_event_process
 功能描述  : join时设置dtim相应寄存器
*****************************************************************************/
osal_s32 hmac_join_set_dtim_reg_event_process(hmac_vap_stru *hmac_vap)
{
    if ((hmac_vap == OSAL_NULL) || (hmac_vap->hal_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_join_set_dtim_reg_event_process::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 写bssid 寄存器 */
    hal_set_sta_bssid(hmac_vap->hal_vap, hmac_vap->bssid, WLAN_MAC_ADDR_LEN);

    /* 写tsf 寄存器 */
    hal_enable_tsf_tbtt(hmac_vap->hal_vap, OSAL_FALSE);

    return OAL_SUCC;
}
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(hmac_tx_process_mgmt_event);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
