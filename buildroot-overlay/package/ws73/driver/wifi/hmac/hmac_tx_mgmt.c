/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 无.
 * Create: 2022-05-16
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_tx_mgmt.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_user.h"
#include "hmac_11w.h"
#include "hmac_alg_notify.h"
#include "hmac_feature_dft.h"
#include "hmac_vap.h"
#include "hmac_psm_ap.h"
#include "hmac_uapsd.h"
#include "hmac_fcs.h"
#include "oal_netbuf_data.h"
#include "hmac_power.h"
#include "hmac_config.h"
#include "wlan_types_common.h"
#include "hmac_dfx.h"
#include "hmac_btcoex.h"
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif

#include "hmac_dfr.h"

#include "hmac_sdp.h"
#ifdef _PRE_WLAN_FEATURE_DBAC
#include "alg_dbac_hmac.h"
#endif
#include "hmac_slp.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_TX_BSS_COMM_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

typedef struct {
    hal_to_dmac_device_stru *hal_device;
    hmac_device_stru *hmac_device;
    hal_tx_dscr_stru *mgmt_dscr;
    hal_tx_mpdu_stru *mpdu;
    hal_tx_txop_feature_stru *txop_feature;
    hal_tx_ppdu_feature_stru *ppdu_feature;
    mac_ieee80211_frame_stru *mac_header;
    mac_tx_ctl_stru *tx_ctl;
    hmac_user_stru *hmac_user;
} hmac_tx_mgmt_stru;

typedef struct {
    mac_tx_ctl_stru *tx_cb;
    hal_tx_dscr_stru *tx_dscr;
    oal_netbuf_stru *netbuf;
    osal_u8 *mac_hdr_addr;
    osal_u8 *mac_payload_addr;
    osal_u8 user_macaddr[WLAN_MAC_ADDR_LEN];
    osal_u16 mac_frame_len; /* 帧头+帧体长度 */
    osal_u8 mac_hdr_len;
    osal_u8 frame_switch;
    osal_u8 cb_switch;
    osal_u8 dscr_switch;
} hmac_tx_dump_param_stru;

typedef enum {
    MGMT_PKT_ENQUEUE_NO,   /* 管理帧不入fake队列 */
    MGMT_PKT_ENQUEUE_SLP,      /* SLP业务正在进行，要求管理帧入队 */
    MGMT_PKT_ENQUEUE_SDP,      /* SDP业务正在进行，要求管理帧入队 */
    MGMT_PKT_ENQUEUE_ROAMING,  /* vap处于猫鼬状态，管理帧入队 */
    MGMT_PKT_ENQUEUE_DBAC,     /* dbac状态，管理帧入队 */
    MGMT_PKT_ENQUEUE_SCAN,     /* 扫描状态，管理帧入队 */
    MGMT_PKT_ENQUEUE_P2P,      /* p2p listen状态，管理帧入队 */
    MGMT_PKT_ENQUEUE_M2S,      /* 天线单双切换，管理帧入队  */
    MGMT_PKT_ENQUEUE_BTCOEX,   /* 蓝牙共存切换，管理帧入队  */

    MGMT_PKT_ENQUEUE_BUTT
} mgmt_pkt_enqueue_enum;
typedef osal_u8 mgmt_pkt_enqueue_enum_uint8;
/*****************************************************************************
  4 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_tx_mgmt_set_txop_security_param
 功能描述  : 根据dmac信息，填写txop的参数
*****************************************************************************/
static osal_void hmac_tx_mgmt_set_txop_security_param(hmac_vap_stru *hmac_vap, const mac_tx_ctl_stru *tx_ctl,
    hal_tx_txop_feature_stru *txop_feature, oal_netbuf_stru *netbuf, const hal_tx_mpdu_stru *mpdu)
{
    hmac_user_stru *multi_user = OSAL_NULL;
    mac_ieee80211_frame_stru *mac_header = OSAL_NULL;

    unref_param(tx_ctl);
    unref_param(mpdu);

    mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);

    /* 管理帧默认不加密 */
    txop_feature->security.cipher_key_type = WLAN_KEY_TYPE_TX_GTK;
    txop_feature->security.cipher_protocol_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    txop_feature->security.cipher_key_id = 0;

    /* 漫游状态时，管理帧不加密 */
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        return;
    }
#endif

    /* 如果报文中的加密位置1了,需要设置发送描述符为加密 */
    /* 需要区别如下2种情况下 */
    /* 1、WEP加密套件下，认证的第三帧,此时，用户还没有关联成功 */
    /* 2、WPA2加密套件下，协商11W成功后，11W规定的部分管理帧 */
    if (mac_is_protectedframe((osal_u8 *)mac_header) == 1) {
        multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
        if (osal_unlikely(multi_user == OSAL_NULL)) {
            oam_warning_log1(0, OAM_SF_TX, "{hmac_tx_set_txopps_security_param::multi_user[%d] is NULL!}",
                hmac_vap->multi_user_idx);
            return;
        }

        txop_feature->security.cipher_protocol_type = multi_user->user_tx_info.security.cipher_protocol_type;
        txop_feature->security.cipher_key_type = multi_user->user_tx_info.security.cipher_key_type;

        if (mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) {
            /* 非法值处理 */
            txop_feature->security.cipher_key_id = hmac_vap_get_default_key_id_etc(hmac_vap);
        }
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    if (ether_is_multicast(mac_header->address1) == OSAL_FALSE) {
        /* 设置PMF管理帧单播加密位 */
        hmac_11w_set_ucast_mgmt_frame(hmac_vap, tx_ctl, txop_feature, netbuf, mac_header);
    } else {
        /* 设置PMF组播管理帧加密位 */
        hmac_11w_set_mcast_mgmt_frame(hmac_vap, txop_feature, netbuf, mpdu);
    }
#endif

    return;
}

/*****************************************************************************
 函 数 名  : hmac_tx_updata_probe_rsp_para
 功能描述  : 更新probe resp帧的txop 参数
*****************************************************************************/
static osal_void hmac_tx_updata_probe_rsp_para(const hmac_vap_stru *hmac_vap, hal_tx_txop_alg_stru *txop_alg,
    const mac_ieee80211_frame_stru *mac_header)
{
    osal_void *fhook = OSAL_NULL;
    unref_param(hmac_vap);
    /* 减少probe_rsp重传次数 1X3 */
    if ((mac_header->frame_control.type == WLAN_MANAGEMENT) && (mac_header->frame_control.sub_type == WLAN_PROBE_RSP)) {
        hal_txop_alg_get_tx_dscr(txop_alg)[0].tx_count = 3;  /* 第1个速率发送3次 */
        hal_txop_alg_get_tx_dscr(txop_alg)[1].tx_count = 0;
        hal_txop_alg_get_tx_dscr(txop_alg)[2].tx_count = 0;  /* 2:第3个速率发送次数 */
        hal_txop_alg_get_tx_dscr(txop_alg)[3].tx_count = 0;  /* 3:第4个速率发送次数 */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_SET_TXOP_ALG);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_set_txop_alg_cb)fhook)(txop_alg);
        }
    }
    return;
}

static oal_bool_enum_uint8 hmac_is_need_enqueue_fake_pause_state(hal_to_dmac_device_stru *hal_device,
    const hmac_vap_stru *hmac_vap)
{
    osal_void *fhook = OSAL_NULL;
    if (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) {
#ifdef _PRE_WLAN_FEATURE_P2P
        /* 非上述场景p2p go是否需要缓存 */
        if ((hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) &&
            (hal_device->current_chan_number != hmac_vap->channel.chan_number)) {
            return MGMT_PKT_ENQUEUE_P2P;
        }
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
        if (hal_device->m2s_excute_flag == OSAL_TRUE) {
            return MGMT_PKT_ENQUEUE_M2S;
        }
#endif /* #ifdef _PRE_WLAN_FEATURE_M2S */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_CHECK_SW_PREEMPT_TYPE);
        if (fhook != OSAL_NULL && ((hmac_btcoex_check_sw_preempt_type_cb)fhook)(hmac_vap, hal_device) == OSAL_TRUE) {
            return MGMT_PKT_ENQUEUE_BTCOEX;
        }
    }

    return MGMT_PKT_ENQUEUE_NO;
}

#ifdef _PRE_WLAN_FEATURE_DBAC
/*****************************************************************************
 功能描述  : sta+go dbac是否缓存 auth/asoc rsp帧到FAKE队列
*****************************************************************************/
static oal_bool_enum_uint8 hmac_is_need_enqueue_fake_sta_go_handle(const hmac_vap_stru *hmac_vap,
    const oal_netbuf_stru *netbuf_mgmt)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf_mgmt);
    osal_u8 mgmt_type = mac_frame_get_type_value((osal_u8 *)mac_header);
    osal_u8 mgmt_subtype = mac_frame_get_subtype_value((const osal_u8 *)mac_header);

    if (hmac_vap->p2p_mode != WLAN_P2P_GO_MODE) {
        return MGMT_PKT_ENQUEUE_DBAC;
    }

    /* STA + GO场景下, 刚协商为GO 就进入dbac了,  p2p尚未完成关联, 此处确保关联报文发到device侧, 不进host fake队列缓存 */
    if (mgmt_type == WLAN_MANAGEMENT && (mgmt_subtype == WLAN_AUTH || mgmt_subtype == WLAN_ASSOC_RSP)) {
        return MGMT_PKT_ENQUEUE_NO;
    }
#endif

    unref_param(hmac_vap);
    unref_param(netbuf_mgmt);

    return MGMT_PKT_ENQUEUE_DBAC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_is_need_enqueue_fake
 功能描述  : dbac是否缓存帧到FAKE队列
*****************************************************************************/
static mgmt_pkt_enqueue_enum_uint8 hmac_is_need_enqueue_fake(const hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device, const hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf_mgmt)
{
    osal_void *fhook = OSAL_NULL;
    osal_void *sdp_fhook = hmac_get_feature_fhook(HMAC_FHOOK_SDP_IS_INIT);
    mgmt_pkt_enqueue_enum_uint8 reason = MGMT_PKT_ENQUEUE_NO;

    if (sdp_fhook != OSAL_NULL && (hal_gp_get_sdp_chnl_switch_off() == 1)) {
        oam_info_log0(0, OAM_SF_TX, "{hmac_is_need_enqueue_fake::sdp chnl switch off, save pkt.}");
        return MGMT_PKT_ENQUEUE_SDP;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_SLP_TX_DISABLE);
    if (fhook != OSAL_NULL && ((hmac_slp_tx_disable_cb)fhook)() == OSAL_TRUE) {
        oam_info_log0(0, OAM_SF_TX, "{hmac_is_need_enqueue_fake::slp tx ctrl, save pkt.}");
        return MGMT_PKT_ENQUEUE_SLP;
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        return MGMT_PKT_ENQUEUE_NO;
    }

#ifdef _PRE_WLAN_FEATURE_DBAC
    if (mac_is_dbac_running(hmac_device) && hmac_alg_dbac_is_pause(hal_device)) {
#ifdef _PRE_WLAN_FEATURE_P2P
        if ((hmac_device->scan_params.scan_func == MAC_SCAN_FUNC_P2P_LISTEN) && (is_p2p_mode(hmac_vap->p2p_mode))) {
            oam_warning_log1(0, 0, "hmac_is_need_enqueue_fake: vap_id[%d] p2p listen", hmac_vap->vap_id);
            return MGMT_PKT_ENQUEUE_NO;
        }
#endif
        return MGMT_PKT_ENQUEUE_DBAC;
    }
    /* DBAC/DBDC运行时判断管理帧是否需要缓存 */
    if (mac_is_dbac_running(hmac_device) && alg_dbac_hmac_is_vap_stop(hmac_vap->hal_vap->vap_id) == OSAL_TRUE) {
        return hmac_is_need_enqueue_fake_sta_go_handle(hmac_vap, netbuf_mgmt);
    }
#endif

    unref_param(netbuf_mgmt);

    /* 扫描期间的管理帧是否需要缓存 */
    if (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING &&
        hmac_device->scan_params.scan_func != MAC_SCAN_FUNC_P2P_LISTEN &&
        hmac_device->scan_params.scan_mode != WLAN_SCAN_MODE_FOREGROUND &&
        hmac_device->scan_params.scan_mode != WLAN_SCAN_MODE_BACKGROUND_PNO) {
        /* 在背景扫描时,中间会切回工作信道,此时管理帧不需要缓存,必须是pause的vap,防止进了p2p device的虚假队列出不来 */
        return (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) ? MGMT_PKT_ENQUEUE_SCAN : MGMT_PKT_ENQUEUE_NO;
    }

    reason = hmac_is_need_enqueue_fake_pause_state(hal_device, hmac_vap);
    if (reason != MGMT_PKT_ENQUEUE_NO) {
        return reason;
    }

    return MGMT_PKT_ENQUEUE_NO;
}

/*****************************************************************************
 函 数 名  : hmac_tx_mgmt_buffer_proc
 功能描述  : 发送管理帧缓存处理，DBAC或背景扫描时管理帧需要缓存
*****************************************************************************/
static oal_bool_enum_uint8 hmac_tx_mgmt_buffer_proc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf_mgmt)
{
    oal_netbuf_queue_header_stru *tx_dscr_queue_fake = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u8 q_num;
    mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf_mgmt);
    osal_u8 mgmt_subtype = mac_frame_get_subtype_value((const osal_u8 *)mac_header);
    mgmt_pkt_enqueue_enum_uint8 reason = MGMT_PKT_ENQUEUE_NO;

    reason = hmac_is_need_enqueue_fake(hmac_device, hal_device, hmac_vap, netbuf_mgmt);
    if (reason == MGMT_PKT_ENQUEUE_NO) {
        /* 管理帧不入队，发送报文前尝试fake队列报文出队 */
        hmac_vap_send_fake_queue(hmac_vap);
        return OSAL_FALSE;
    }

    tx_dscr_queue_fake = hmac_vap->tx_dscr_queue_fake;
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_mgmt);
    q_num = hal_ac_to_q_num(tx_ctl->ac);
    /* opmode和smps帧需要放到虚假队列的头部 */
    if (mac_get_cb_is_opmode_frame(tx_ctl) || mac_get_cb_is_smps_frame(tx_ctl)) {
        oal_netbuf_addlist(&tx_dscr_queue_fake[q_num].buff_header, netbuf_mgmt);
    } else {
        oal_netbuf_add_to_list_tail(netbuf_mgmt, &(tx_dscr_queue_fake[q_num].buff_header));
    }
    tx_dscr_queue_fake[q_num].mpdu_cnt++;

    oam_warning_log3(0, OAM_SF_DBAC, "hmac_tx_mgmt_buffer_proc::buffered fakeq. subtype:[%d] state[%d], reason[%d]",
        mgmt_subtype, hmac_vap->vap_state, reason);

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_mgmt_buff_process
 功能描述  : 发送管理帧缓存处理
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_tx_mgmt_buff_process(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf_mgmt)
{
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;
    osal_void *sdp_fhook = hmac_get_feature_fhook(HMAC_FHOOK_IS_SDP_FRAME);
#ifdef _PRE_WLAN_FEATURE_DBAC
    mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf_mgmt);
    osal_u8 mgmt_type = mac_frame_get_type_value((osal_u8 *)mac_header);
    osal_u8 mgmt_subtype = mac_frame_get_subtype_value((const osal_u8 *)mac_header);
#endif

    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_mgmt);
    if (tx_ctrl->is_fake_buffer == OSAL_TRUE) {
        /* fake队列出来的报文直接发送，不缓存 */
        return OSAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_FTM
    /* FTM帧不需要缓存 */
    if (mac_is_ftm_related_frame(netbuf_mgmt) == OSAL_TRUE) {
        return OSAL_FALSE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_DBAC
    if ((mgmt_type == WLAN_MANAGEMENT && mgmt_subtype == WLAN_DISASOC) && mac_is_dbac_running(hmac_device)) {
        /* dbac场景去关联帧保护 */
        hmac_alg_dbac_send_disassoc(hmac_vap, OSAL_TRUE);
        return OSAL_FALSE;
    }
#endif

    /* 不缓存wifi aware报文, 即使已经切离了STA的工作信道 */
    if (sdp_fhook != OSAL_NULL && (((hmac_is_sdp_frame_cb)sdp_fhook)(netbuf_mgmt) == OAL_TRUE)) {
        oam_info_log0(0, OAM_SF_TX, "{hmac_tx_mgmt_buff_process::sdp pkt do not save to mgmt queue.}");
        return OSAL_FALSE;
    }

    return hmac_tx_mgmt_buffer_proc(hmac_device, hmac_vap, netbuf_mgmt);
}

/*****************************************************************************
 函 数 名  : hmac_tx_set_txopps_param
 功能描述  : 把txop ps相关的量填到txop_feature中，后续填到描述符里面
*****************************************************************************/
static osal_void hmac_tx_set_txopps_param(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    hal_tx_txop_feature_stru *txop_feature, oal_bool_enum_uint8 ismcast)
{
    /* 以下根据Draft P802.11ac_D3.1.pdf 9.17a填写 */
    if (ismcast) {
        txop_feature->groupid_partial_aid.group_id = 63; /* 文档group_id 为 63 */
        txop_feature->groupid_partial_aid.partial_aid = 0;
    } else {
        txop_feature->groupid_partial_aid.group_id = hmac_user->groupid;
        txop_feature->groupid_partial_aid.partial_aid = hmac_user->partial_aid;
    }

    switch (hmac_vap->vap_mode) {
        case WLAN_VAP_MODE_BSS_STA:
            txop_feature->groupid_partial_aid.txop_ps_not_allowed = OSAL_TRUE;
            break;

        case WLAN_VAP_MODE_BSS_AP:
            if (mac_mib_get_txopps(hmac_vap) == OSAL_TRUE) {
                /* 对于AP,如果支持TXOP PS则始终允许STA睡眠 */
                txop_feature->groupid_partial_aid.txop_ps_not_allowed = OSAL_FALSE;
            } else {
                txop_feature->groupid_partial_aid.txop_ps_not_allowed = OSAL_TRUE;
            }
            break;

        default:
            oam_error_log1(0, OAM_SF_ANY, "{hmac_tx_set_txopps_param: unvalid vap mode[%d]}",
                hmac_vap->vap_mode);
            break;
    }
}

static osal_u32 hmac_tx_process_all_mgmt_psm(hmac_vap_stru *hmac_vap, hmac_tx_mgmt_stru *mgmt_param,
    oal_netbuf_stru *netbuf_mgmt)
{
    osal_void *fhook = OSAL_NULL;
    osal_u32 ret;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_AP_UAPSD_ENQUEUE_PROC);
    if (fhook != OSAL_NULL) {
        ret = ((hmac_uapsd_enqueue_proc_cb)fhook)(hmac_vap, mgmt_param->hmac_user, mgmt_param->tx_ctl, netbuf_mgmt);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }

    if (hmac_psm_pkt_need_buff(mgmt_param->hmac_device, hmac_vap, mgmt_param->hmac_user, netbuf_mgmt) == OSAL_TRUE) {
        oam_info_log4(0, OAM_SF_TX,
            "vap_id[%d] {hmac_tx_process_all_mgmt_psm::IS_PS_QUEUE[%d], psm_mode[%d], IS_MCAST[%d]}",
            hmac_vap->vap_id, mgmt_param->tx_ctl->is_get_from_ps_queue,
            mgmt_param->hmac_user->ps_mode, mgmt_param->tx_ctl->ismcast);
        return hmac_psm_ps_enqueue(hmac_vap, mgmt_param->hmac_user, netbuf_mgmt);
    }

    return OAL_CONTINUE;
}

static osal_void hmac_get_mgmt_mpdu_param(oal_netbuf_stru *netbuf_mgmt, mac_tx_ctl_stru *tx_ctl, osal_u16 len,
    hal_tx_mpdu_stru *mpdu)
{
    tx_ctl->netbuf_num = 1; /* 管理帧只有一个 */
    tx_ctl->mpdu_num = 1;        /* 管理帧只有一个 */
    tx_ctl->is_amsdu = OSAL_FALSE; /* 管理帧不做amsdu聚合 */
    tx_ctl->roam_data = OSAL_FALSE;

    /* 填写MPDU基本参数 */
    mpdu->wmm.tid_no = 0;
    mpdu->wmm.qos_enable = OSAL_FALSE;
    if (((mac_ieee80211_frame_stru *)(oal_netbuf_header(netbuf_mgmt)))->frame_control.type == WLAN_CONTROL) {
        mpdu->wmm.tid_no = tx_ctl->tid;
        mpdu->mpdu_mac_hdr.mac_hdr_len = MAC_80211_CTL_HEADER_LEN;
    } else {
        mpdu->mpdu_mac_hdr.mac_hdr_len = MAC_80211_FRAME_LEN;
    }

    tx_ctl->frame_header_length = mpdu->mpdu_mac_hdr.mac_hdr_len;

    mpdu->mpdu_mac_hdr.num_sub_msdu = 1;

    /* 在报文长度小于帧头长度时（任意帧报文发送），会发生翻转，避免翻转 */
    if ((tx_ctl->is_custom == OSAL_TRUE) && (len < tx_ctl->frame_header_length)) {
        mpdu->msdu_addr[0].msdu0_len = 0;
    } else {
        mpdu->msdu_addr[0].msdu0_len = len - tx_ctl->frame_header_length;
    }
    mpdu->msdu_addr[0].msdu1_len = 0;
    mpdu->msdu_addr[0].msdu_addr1 = 0;
    mpdu->msdu_addr[0].msdu2_len = 0;
    mpdu->msdu_addr[0].msdu_addr2 = 0;
    mpdu->msdu_addr[0].msdu3_len = 0;
    mpdu->msdu_addr[0].msdu_addr3 = 0;
    if ((tx_ctl->is_custom == OSAL_TRUE) && (len < tx_ctl->frame_header_length)) {
        tx_ctl->mpdu_payload_len = 0;
    } else {
        tx_ctl->mpdu_payload_len = len - tx_ctl->frame_header_length;
    }
    mpdu->mpdu_len = len;
}

static osal_u32 hmac_tx_mgmt_prep(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf_mgmt, osal_u16 len,
    hmac_tx_mgmt_stru *mgmt_param)
{
    osal_u32 ret;
    osal_bool notify_device = OSAL_FALSE;

    mgmt_param->mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf_mgmt);

    /* STA 发送auth时，置位等待接收auth使能 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (mgmt_param->mac_header->frame_control.sub_type == WLAN_AUTH) {
            hmac_vap->auth_received = OSAL_FALSE;
            notify_device = OSAL_TRUE;
        } else if (mgmt_param->mac_header->frame_control.sub_type == WLAN_REASSOC_REQ) {
            hmac_vap->assoc_rsp_received = OSAL_FALSE;
            notify_device = OSAL_TRUE;
        }

        /* 通知Device需要重传AUTH请求和重关联请求               */
        if (notify_device) {
            hmac_vap_sync(hmac_vap);
        }
    }

    // only send probe req while scan
    // when working home channel while bgscan, scanning = true
    if ((mgmt_param->hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
        (mgmt_param->mac_header->frame_control.type == WLAN_MANAGEMENT) &&
        (mgmt_param->mac_header->frame_control.sub_type != WLAN_PROBE_REQ) &&
        (mgmt_param->mac_header->frame_control.sub_type != WLAN_PROBE_RSP) &&
        (mgmt_param->mac_header->frame_control.sub_type != WLAN_ACTION) &&
        (mgmt_param->hal_device->current_chan_number != hmac_vap->channel.chan_number)) {
        oam_warning_log3(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_tx_mgmt_prep::drop mgmt while scan, vap_state=%d type=%d}",
            hmac_vap->vap_id, hmac_vap->vap_state, mgmt_param->mac_header->frame_control.sub_type);
        oal_netbuf_free(netbuf_mgmt);
        return OAL_SUCC;
    }
    mgmt_param->tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_mgmt);

    /* init mpdu basic para 1.保证发送出现异常时候，帧体上报等维测能正常把帧体打印出来; */
    /*                   2.初始化基本字段，预防后续需要使用到基本字段 */
    hmac_get_mgmt_mpdu_param(netbuf_mgmt, mgmt_param->tx_ctl, len, mgmt_param->mpdu);

    /* 查找用户，判断帧是否需要入节能队列，如果需要，则入队，发送流程不再往下走,
       如果用户找不到了也就不涉及节能，直接略过即可 */
    mgmt_param->hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(mgmt_param->tx_ctl->tx_user_idx);
    if (mgmt_param->hmac_user != OSAL_NULL) {
        ret = hmac_tx_process_all_mgmt_psm(hmac_vap, mgmt_param, netbuf_mgmt);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
        /* 获取group id 和 partial aid */
        hmac_tx_set_txopps_param(hmac_vap, mgmt_param->hmac_user, mgmt_param->txop_feature,
            mgmt_param->tx_ctl->ismcast);
    }
    return OAL_CONTINUE;
}

static osal_void hmac_dft_handle_80211_frame(osal_u8 *hdr_addr, osal_u8 hdr_len, const osal_u8 *frame_addr,
    osal_u16 frame_len, osal_u32 msg_id)
{
    unref_param(hdr_addr);
    unref_param(hdr_len);
    unref_param(frame_addr);
    unref_param(frame_len);
    unref_param(msg_id);
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    hmac_sniffer_80211_frame(hdr_addr, hdr_len, frame_addr, frame_len, msg_id);
#endif
}

static osal_u32 hmac_tx_mgmt_queue(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf_mgmt,
    const mac_tx_ctl_stru *tx_ctl, const hal_tx_mpdu_stru *mpdu)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf_mgmt);
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    /*****************************************************************************
        到此发送描述符已填写完毕，接下来放到硬件队列
    *****************************************************************************/
    osal_u8 mgmt_type = mac_frame_get_type_value((const osal_u8 *)mac_header);
    osal_u8 mgmt_subtype = mac_frame_get_subtype_value((const osal_u8 *)mac_header);
     /* probe req或任意帧，在任何情况下都直接发送 */
    if ((mgmt_type == WLAN_MANAGEMENT && mgmt_subtype == WLAN_PROBE_REQ) || tx_ctl->is_custom == OSAL_TRUE) {
        return OAL_CONTINUE;
    }

    /* 判断当前管理帧是否需要缓存 */
    if (hmac_tx_mgmt_buff_process(hmac_device, hmac_vap, netbuf_mgmt) == OSAL_TRUE) {
        /* 将缓存帧直接通过OTA上报，方便问题定位 */
        hmac_dft_handle_80211_frame((osal_u8 *)mac_get_frame_header_addr(tx_ctl),
            tx_ctl->frame_header_length, (osal_u8 *)oal_netbuf_tx_data(netbuf_mgmt), mpdu->mpdu_len,
            SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME);
        return OAL_SUCC;
    }

    if ((hmac_vap->channel.chan_number == 0) || (hal_device->current_chan_number == hmac_vap->channel.chan_number) ||
        (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) || (hmac_vap->vap_state == MAC_VAP_STATE_INIT)) {
        return OAL_CONTINUE;
    }

    /* 当在DBAC场景下时，current_chan_number值不对，报文需要直接发送出去 */
    if ((mac_is_dbac_running(hmac_device) == OSAL_TRUE) &&
        (hal_device->current_chan_number != hmac_vap->channel.chan_number)) {
        return OAL_CONTINUE;
    }

    // 同频offload场景下，SYNC信道时BSS的DISASOC本身无法发送成功，此处避免虚检
    if ((mgmt_type == WLAN_MANAGEMENT) && (mgmt_subtype == WLAN_DISASOC)) {
        oam_warning_log4(0, OAM_SF_TX,
            "vap_id[%d] hmac_tx_mgmt_queue:disasoc dropped from diff channel, vap chan:%d, hal chan:%d. state:%d",
            hmac_vap->vap_id, hmac_vap->channel.chan_number, hal_device->current_chan_number, hmac_vap->vap_state);
    } else {
        oam_warning_log4(0, OAM_SF_TX,
            "vap_id[%d] hmac_tx_mgmt_queue:put tx_dscr in wrong channel. vap chan:%d, hal chan:%d. vap state:%d",
            hmac_vap->vap_id, hmac_vap->channel.chan_number, hal_device->current_chan_number, hmac_vap->vap_state);
        hmac_dft_handle_80211_frame((osal_u8 *)mac_get_frame_header_addr(tx_ctl),
            tx_ctl->frame_header_length, (osal_u8 *)oal_netbuf_tx_data(netbuf_mgmt),
            (tx_ctl->mpdu_payload_len + tx_ctl->frame_header_length),
            SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME);
    }
    hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf_mgmt);
    oal_netbuf_free(netbuf_mgmt);
    return OAL_SUCC; // 返回SUCC以避免重复释放该SKB
}

/*****************************************************************************
 函 数 名  : hmac_tx_fill_cb_info
 功能描述  : 根据txop_feature和txop_alg填写CB
*****************************************************************************/
static osal_void hmac_tx_fill_cb_info(mac_tx_ctl_stru *tx_ctl, hal_tx_txop_alg_stru *txop_alg,
    hal_tx_txop_feature_stru *txop_feature, hal_tx_ppdu_feature_stru *ppdu_feature)
{
    /* 将ppdu_feature和txop_feature字段写入CB */
    tx_ctl->lsig_txop = txop_alg->phy_info.lsig_txop;

    tx_ctl->partial_aid = txop_feature->groupid_partial_aid.partial_aid;
    tx_ctl->txop_ps_not_allowed = txop_feature->groupid_partial_aid.txop_ps_not_allowed;
    tx_ctl->ax_bss_color = ppdu_feature->ax_bss_color;

    tx_ctl->retry_flag_hw_bypass = ppdu_feature->retry_flag_hw_bypass;
    tx_ctl->duration_hw_bypass = ppdu_feature->duration_hw_bypass;
    tx_ctl->timestamp_hw_bypass = ppdu_feature->timestamp_hw_bypass;
    tx_ctl->tx_pn_hw_bypass = ppdu_feature->tx_pn_hw_bypass;

    tx_ctl->cipher_key_id = txop_feature->security.cipher_key_id;
    tx_ctl->cipher_key_type = txop_feature->security.cipher_key_type;
    tx_ctl->cipher_protocol_type = hal_cipher_suite_to_ctype(txop_feature->security.cipher_protocol_type);

    tx_ctl->ra_lut_index = ppdu_feature->ppdu_addr_index.ra_lut_index;
    tx_ctl->tx_hal_vap_index = ppdu_feature->ppdu_addr_index.tx_vap_index;
}

static osal_u32 hmac_tx_mgmt_send(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf_mgmt, hmac_tx_mgmt_stru *param)
{
    hal_tx_txop_alg_stru txop_algorithm;
    hal_tx_txop_alg_stru *txop_alg = OSAL_NULL;
    osal_u32 ret;
    osal_void *fhook = OSAL_NULL;

    /* 设置管理帧txop参数 */
    hmac_tx_mgmt_get_txop_para(hmac_vap, &txop_alg, param->tx_ctl);
    hmac_tx_updata_probe_rsp_para(hmac_vap, txop_alg, param->mac_header);

    /* 设置当前管理帧的发送参数，保存初始化的值不被覆盖参数 */
    (osal_void)memcpy_s(&txop_algorithm, sizeof(hal_tx_txop_alg_stru), txop_alg, sizeof(hal_tx_txop_alg_stru));

    /* 填写管理帧描述符CB域字段 */
    hmac_tx_fill_cb_info(param->tx_ctl, &txop_algorithm, param->txop_feature, param->ppdu_feature);
    mac_get_cb_tx_vap_index(param->tx_ctl) = hmac_vap->vap_id;
    /* 写入rssi_level */
    hmac_alg_update_rssi_level_notify(hmac_vap, param->tx_ctl, param->hmac_user);

    /* 设置ftm帧tx dscr */
#ifdef _PRE_WLAN_FEATURE_FTM
    if (mac_check_ftm_enable(hmac_vap)) {
        hmac_tx_set_ftm_ctrl_dscr(hmac_vap, param->mgmt_dscr, netbuf_mgmt);

        if (mac_is_ftm_frame(netbuf_mgmt) == OSAL_TRUE) {
            hal_set_ftm_sample(OSAL_TRUE);
        }
    }
#endif

    /* host侧TX方向管理帧上报到HSO */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_FRAME_REPORT_HSO);
    if (fhook != OSAL_NULL) {
        ((dfx_frame_report_hso_cb)fhook)(netbuf_mgmt, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    }

#ifdef _PRE_WLAN_FEATURE_SNIFFER
    if (hmac_sniffer_get_switch() == EXT_SWITCH_ON) {
        hmac_sniffer_80211_frame(oal_netbuf_header(netbuf_mgmt), param->mpdu->mpdu_mac_hdr.mac_hdr_len,
            oal_netbuf_tx_data(netbuf_mgmt), param->mpdu->mpdu_len, SOC_DIAG_MSG_ID_WIFI_TX_80211_FRAME);
    }
#endif
    ret = hmac_tx_mgmt_queue(hmac_vap, netbuf_mgmt, param->tx_ctl, param->mpdu);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态发送帧时，如果发送帧VAP所在信道非当前工作信道则释放帧 */
    if (hmac_tx_enable_in_roaming(param->hal_device, hmac_vap) == OSAL_FALSE) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf_mgmt);
        return OAL_FAIL;
    }
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_TX_MGMT_FRAME);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_tx_mgmt_frame_cb)fhook)(hmac_vap, param->mac_header);
    }

    /* 挂硬件队列发送 */
    hmac_tx_send_mpdu(hmac_vap, HCC_MSG_TYPE_MGMT_FRAME, netbuf_mgmt, 1);
    return OAL_SUCC;
}

static osal_u32 hmac_tx_mgmt_move_header(oal_netbuf_stru *netbuf_mgmt, mac_tx_ctl_stru *tx_ctl)
{
    osal_s32 ret;
    osal_u32 header_padding;
    if (tx_ctl->frame_header_length < MAC_80211_FRAME_LEN) {
        header_padding = MAC_80211_FRAME_LEN - tx_ctl->frame_header_length;
        ret = memmove_s(oal_netbuf_header(netbuf_mgmt) + header_padding, header_padding,
            oal_netbuf_header(netbuf_mgmt), header_padding);
        if (ret != EOK) {
            return OAL_FAIL;
        }
        oal_netbuf_pull(netbuf_mgmt, header_padding);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tx_init_ppdu_feature
 功能描述  : 填写ppdu feature 字段
*****************************************************************************/
static osal_void hmac_tx_init_ppdu_feature(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 mpdu_num, hal_tx_ppdu_feature_stru *ppdu_feature)
{
    if (osal_unlikely(hmac_vap->hal_vap == OSAL_NULL)) {
        return;
    }
    /* 单包/管理帧 发送这些字段无效初始化为0 */
    ppdu_feature->mpdu_num = mpdu_num;
    ppdu_feature->ppdu_addr_index.tx_vap_index = hmac_vap->hal_vap->vap_id;
    if (hmac_user != OSAL_NULL) {
        /* addba req需要填写正确的ra lut index 以获取正确的ssn */
        ppdu_feature->ppdu_addr_index.ra_lut_index = hmac_user->lut_index; /* 用户结构体下的lut index */
#ifdef _PRE_WLAN_FEATURE_11AX
        ppdu_feature->ax_bss_color = hmac_user->he_hdl.he_oper_ie.bss_color.bss_color;
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP &&
            mac_mib_get_he_oper_bss_color_disable(hmac_vap) == OSAL_FALSE) {
            /* AP模式直接从mib获取bss color */
            ppdu_feature->ax_bss_color = mac_mib_get_he_oper_bss_color(hmac_vap);
        }
#endif
    }
    ppdu_feature->addba_ssn_hw_bypass = OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_tx_mgmt
 功能描述  : 发送管理帧接口
*****************************************************************************/
osal_u32 hmac_tx_mgmt(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf_mgmt, osal_u16 len, osal_bool data_ready)
{
    hal_tx_mpdu_stru mpdu;
    hal_tx_txop_feature_stru txop_feature;
    hal_tx_ppdu_feature_stru ppdu_feature;
    hmac_tx_mgmt_stru mgmt_param;
    osal_u32 ret;
    osal_u32 data_len;

    memset_s(&txop_feature, sizeof(txop_feature), 0, sizeof(txop_feature));
    memset_s(&ppdu_feature, sizeof(ppdu_feature), 0, sizeof(ppdu_feature));

    memset_s(&mgmt_param, sizeof(mgmt_param), 0, sizeof(mgmt_param));
    mgmt_param.mpdu = &mpdu;
    mgmt_param.txop_feature = &txop_feature;
    mgmt_param.ppdu_feature = &ppdu_feature;
    mgmt_param.hmac_device = hmac_res_get_mac_dev_etc(0);

    if (hmac_vap == OSAL_NULL || hmac_vap->hal_vap == OSAL_NULL || netbuf_mgmt == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_tx_mgmt::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mgmt_param.hal_device = hmac_vap->hal_device;
    if (mgmt_param.hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_mgmt::hal_device null.}", hmac_vap->vap_id);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf_mgmt);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_dfr_get_reset_process_flag() == OSAL_TRUE) {
        oam_error_log1(0, OAM_SF_TX, "vap_id[%d] {hmac_tx_mgmt::RESET_IN_PROGRESS}", hmac_vap->vap_id);
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, netbuf_mgmt);
        return OAL_ERR_CODE_RESET_INPROGRESS;
    }

    oal_set_netbuf_prev(netbuf_mgmt, OSAL_NULL);
    oal_set_netbuf_next(netbuf_mgmt, OSAL_NULL);

    ret = hmac_tx_mgmt_prep(hmac_vap, netbuf_mgmt, len, &mgmt_param);
    if (ret != OAL_CONTINUE) {
        return ret;
    }
    data_len = MAC_80211_FRAME_LEN + mgmt_param.tx_ctl->mpdu_payload_len;
    if (!data_ready) {
        oal_netbuf_put(netbuf_mgmt, data_len);
        ret = hmac_tx_mgmt_move_header(netbuf_mgmt, mgmt_param.tx_ctl);
        if (ret != OAL_SUCC) {
            return OAL_FAIL;
        }
        data_len = mgmt_param.tx_ctl->frame_header_length + mgmt_param.tx_ctl->mpdu_payload_len;
    }

    /* set mgmt frame seq num */
    if ((mgmt_param.mac_header->frame_control.type != WLAN_CONTROL) ||
        (mgmt_param.mac_header->frame_control.sub_type != WLAN_BLOCKACK_REQ)) {
        mgmt_param.mac_header->frag_num = 0;
        mgmt_param.mac_header->seq_num = 0;
    }
    /* init ppdu feature struct */
    hmac_tx_init_ppdu_feature(hmac_vap, mgmt_param.hmac_user, 1, mgmt_param.ppdu_feature);
    /* 不是发给关连用户的管理桢，场景：AP响应未关连用户的probe req;
       王浩: TX描述幅ra lut index要填一个没用过的index, 此处填最大0x1F */
    if ((mgmt_param.tx_ctl)->tx_user_idx == MAC_INVALID_USER_ID) {
        /* v200最大支持294用户,会溢出 */
        mgmt_param.ppdu_feature->ppdu_addr_index.ra_lut_index = (osal_u8)WLAN_INVALID_RA_LUT_IDX;
    }

    /* SET txop security feature */
    hmac_tx_mgmt_set_txop_security_param(hmac_vap, mgmt_param.tx_ctl, mgmt_param.txop_feature, netbuf_mgmt,
        mgmt_param.mpdu);

    return hmac_tx_mgmt_send(hmac_vap, netbuf_mgmt, &mgmt_param);
}

/* 清除fifo并删除tx队列中的所有帧 */
osal_void hmac_clear_hw_fifo_tx_queue(hal_to_dmac_device_stru *hal_device)
{
    frw_msg msg = {0};
    osal_s32 ret;
    unref_param(hal_device);
    /* 抛事件至Device侧DMAC，清除fifo并删除tx队列中的所有帧 */
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_CLEAR_DSCR_QUEUE, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "dmac_clear_hw_fifo_tx_queue:send clear fifo to device fail[%d]", ret);
    }
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
