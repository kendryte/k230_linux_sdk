/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Create: 2022-05-09
 */
#include "hmac_tx_mpdu_adapt.h"
#include "frw_hmac_adapt.h"
#include "wlan_thruput_debug.h"
#include "hmac_thruput_test.h"
#include "hmac_psm_ap.h"
#include "hmac_alg_notify.h"
#include "hmac_twt.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "oal_netbuf_data.h"
#include "hal_ext_if.h"
#include "hmac_tx_mpdu_queue.h"
#include "hmac_feature_dft.h"

#include "hmac_latency_stat.h"
#include "hmac_feature_interface.h"
#include "hmac_uapsd.h"
#include "hmac_btcoex.h"
#include "hmac_dfx.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HMAC_TX_MPDU_ADAPT_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

OAL_STATIC osal_u32 hmac_alg_tx_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_tx_ctl_stru *cb,
    hal_tx_txop_alg_stru *txop_alg);

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC void hmac_tx_send_netbuf(hmac_vap_stru *hmac_vap,
    hcc_msg_type type, oal_netbuf_stru *netbuf)
{
    osal_u32 ret;
#ifdef _PRE_WLAN_DFT_STAT
    osal_u8 pkt_type, pkt_subtype;
    osal_u32 msdu_num, stat_tyte;
#endif
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    if ((hmac_is_thruput_test() == OSAL_TRUE)) {
        hmac_update_thruput(tx_ctl->frame_header_length + tx_ctl->mpdu_payload_len);
    }

    /* 发包HOOK回调处理 */
    ret = hmac_call_netbuf_hooks(&netbuf, hmac_vap, HMAC_FRAME_DATA_TX_EVENT_H2D);
    if (ret != OAL_CONTINUE) {
        return;
    }

#ifdef _PRE_WLAN_DFT_STAT
    /* 提前保存流id，报文数量和报文类型，避免内存释放后再访问 */
    msdu_num = tx_ctl->msdu_num;
    pkt_type = mac_get_frame_type(oal_netbuf_header(netbuf));
    pkt_subtype = mac_get_frame_sub_type(oal_netbuf_header(netbuf));
#endif
    ret = frw_hmac_send_data(netbuf, hmac_vap->vap_id, type);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
    }
#ifdef _PRE_WLAN_DFT_STAT
    if (pkt_type == WLAN_FC0_TYPE_DATA && pkt_subtype != WLAN_FC0_SUBTYPE_NODATA) {
        stat_tyte = (ret == OAL_SUCC) ? TX_D_SEND_DEVICE_SUCC : TX_D_SEND_DEVICE_FAIL;
        hmac_pkts_tx_stat(hmac_vap->vap_id, msdu_num, stat_tyte);
    } else {
        stat_tyte = (ret == OAL_SUCC) ? TX_M_SEND_DEVICE_SUCC : TX_M_SEND_DEVICE_FAIL;
        hmac_pkts_tx_stat(hmac_vap->vap_id, 1, stat_tyte);
    }
#endif
}

#ifdef _PRE_WIFI_DEBUG
void pkt_drop_debug(const mac_tx_ctl_stru *tx_ctl, const osal_char *drop_reason)
{
    if (hal_tx_data_type_debug_on() == OSAL_FALSE) {
        return;
    }

    if (tx_ctl != OSAL_NULL) {
        wifi_printf("[TX][PKT_SUBTYPE:%d][DROP:%s]\r\n", tx_ctl->frame_subtype, drop_reason);
    }
}

typedef struct {
    osal_u8 type;
    osal_u8 sub_type;
    osal_char *sub_type_name;
} pkt_type_name_stru;

const pkt_type_name_stru g_type_name_array[] = {
    /* mgmt frame */
    { WLAN_MANAGEMENT, WLAN_ASSOC_REQ, "WLAN_ASSOC_REQ" },
    { WLAN_MANAGEMENT, WLAN_ASSOC_RSP, "WLAN_ASSOC_RSP" },
    { WLAN_MANAGEMENT, WLAN_REASSOC_REQ, "WLAN_REASSOC_REQ" },
    { WLAN_MANAGEMENT, WLAN_REASSOC_RSP, "WLAN_REASSOC_RSP" },
    { WLAN_MANAGEMENT, WLAN_PROBE_REQ, "WLAN_PROBE_REQ" },
    { WLAN_MANAGEMENT, WLAN_PROBE_RSP, "WLAN_PROBE_RSP" },
    { WLAN_MANAGEMENT, WLAN_TIMING_AD, "WLAN_TIMING_AD" },
    { WLAN_MANAGEMENT, WLAN_MGMT_SUBTYPE_RESV1, "WLAN_MGMT_SUBTYPE_RESV1" },
    { WLAN_MANAGEMENT, WLAN_ATIM, "WLAN_ATIM" },
    { WLAN_MANAGEMENT, WLAN_DISASOC, "WLAN_DISASOC" },
    { WLAN_MANAGEMENT, WLAN_AUTH, "WLAN_AUTH" },
    { WLAN_MANAGEMENT, WLAN_DEAUTH, "WLAN_DEAUTH" },
    { WLAN_MANAGEMENT, WLAN_ACTION, "WLAN_ACTION" },
    { WLAN_MANAGEMENT, WLAN_ACTION_NO_ACK, "WLAN_ACTION_NO_ACK" },
    { WLAN_MANAGEMENT, WLAN_MGMT_SUBTYPE_RESV2, "WLAN_MGMT_SUBTYPE_RESV2" },
    /* data frame */
    { WLAN_DATA_BASICTYPE, MAC_DATA_ARP_REQ, "MAC_DATA_ARP_REQ"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_ARP_RSP, "MAC_DATA_ARP_RSP"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_DHCP, "MAC_DATA_DHCP"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_DHCPV6, "MAC_DATA_DHCPV6"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_ICMP_REQ, "MAC_DATA_ICMP_REQ"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_ICMP_RSP, "MAC_DATA_ICMP_RSP"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_ICMP_OTH, "MAC_DATA_ICMP_OTH"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_EAPOL, "MAC_DATA_EAPOL"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_TDLS, "MAC_DATA_TDLS"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_ND, "MAC_DATA_ND"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_PPPOE, "MAC_DATA_PPPOE"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_WAPI, "MAC_DATA_WAPI"},
    { WLAN_DATA_BASICTYPE, MAC_DATA_VLAN, "MAC_DATA_VLAN"},
};

OAL_STATIC void pkt_trace_debug_print(oal_netbuf_stru *netbuf, const osal_char *direct)
{
    osal_u8 i = 0;
    osal_u8 type;
    osal_u8 subtype;
    struct mac_ieee80211_frame *mac_hdr = OSAL_NULL;

    mac_hdr = (struct mac_ieee80211_frame *)((osal_u8 *)oal_netbuf_header(netbuf));
    type = mac_hdr->frame_control.type;
    if (type == WLAN_MANAGEMENT) {
        subtype = mac_hdr->frame_control.sub_type;
    } else if (type == WLAN_DATA_BASICTYPE) {
        subtype = hmac_get_tx_data_type_etc(netbuf);
    } else if (type == WLAN_CONTROL) {
        wifi_printf("[TX][%s]action subtype:%d\r\n", direct, mac_hdr->frame_control.sub_type);
        return;
    } else {
        return;
    }

#ifdef _PRE_WLAN_DFT_STAT
    if (((dfx_get_service_control() & SERVICE_CONTROL_SKIP_PROBE_FRAME) != 0) &&
        ((type == WLAN_MANAGEMENT) && ((subtype == WLAN_PROBE_REQ) || (subtype == WLAN_PROBE_RSP)))) {
        return;
    }
#endif

    for (i = 0; i < sizeof(g_type_name_array) / sizeof(g_type_name_array[0]); i++) {
        if (type == g_type_name_array[i].type && subtype == g_type_name_array[i].sub_type) {
            wifi_printf("[TX][%s]%s\r\n", direct, g_type_name_array[i].sub_type_name);
        }
    }
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT void pkt_trace_debug(oal_netbuf_stru *netbuf, osal_char *direct)
{
    if (hal_tx_data_type_debug_on() != OSAL_TRUE) {
        return;
    }

    pkt_trace_debug_print(netbuf, direct);
}
#endif

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_tx_send_mpdu(hmac_vap_stru *hmac_vap, hcc_msg_type type,
    oal_netbuf_stru *net_buffer, osal_u8 mpdu_num)
{
    osal_u8 mpdu_idx;
    oal_netbuf_stru *netbuf = net_buffer;
    oal_netbuf_stru *netbuf_next = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    for (mpdu_idx = 0; mpdu_idx < mpdu_num; mpdu_idx++) {
        if (netbuf == OSAL_NULL) {
            return;
        }
#ifdef _PRE_WIFI_DEBUG
        pkt_trace_debug(netbuf, "HMAC_TO_HCC");
#endif
        /* 统计在TID队列的时延 */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_RECORD_LATENCY_TX);
        if (fhook != OSAL_NULL) {
            ((hmac_record_latency_tx_cb)fhook)(netbuf, TX_HMAC_TID_QUEUE);
        }

        netbuf_next = oal_get_netbuf_next(netbuf);
        hmac_tx_send_netbuf(hmac_vap, type, netbuf);
        netbuf = netbuf_next;
    }
}

/*****************************************************************************
 功能描述  : 根据保护开关,控制发送帧是否开启lsig_txop，入参有效性由调用者保证
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_tx_set_protection_lsig_txop(mac_tx_ctl_stru *tx_ctl,
    const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    const mac_protection_stru *protection = &hmac_vap->protection;
    if (osal_unlikely(protection->lsig_txop_protect_mode == EXT_SWITCH_ON)) {
        if (hmac_user->cur_protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE &&
            tx_ctl->frame_type == WLAN_CB_FRAME_TYPE_DATA) {
            if (tx_ctl->ismcast == 0 && tx_ctl->is_bcast == 0) {
                tx_ctl->lsig_txop = OSAL_TRUE;
                return;
            }
        }
    }
    tx_ctl->lsig_txop = OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 填写发送帧partial_aid和txop_ps_not_allowed，入参有效性由调用者保证
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_tx_set_ps_param(mac_tx_ctl_stru *tx_ctl,
    const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    /* 以下根据Draft P802.11ac_D3.1.pdf 9.17a填写 */
    if (tx_ctl->ismcast != 0) {
        tx_ctl->partial_aid = 0;
    } else {
        tx_ctl->partial_aid = hmac_user->partial_aid;
    }

    switch (hmac_vap->vap_mode) {
        case WLAN_VAP_MODE_BSS_STA:
            tx_ctl->txop_ps_not_allowed = OSAL_TRUE;
            break;

        case WLAN_VAP_MODE_BSS_AP:
            if (mac_mib_get_txopps(hmac_vap) == OSAL_TRUE) {
                /* 对于AP,如果支持TXOP PS则始终允许STA睡眠 */
                tx_ctl->txop_ps_not_allowed = OSAL_FALSE;
            } else {
                tx_ctl->txop_ps_not_allowed = OSAL_TRUE;
            }
            break;

        default:
            oam_error_log1(0, OAM_SF_ANY, "{hmac_tx_set_ps_param: unvalid vap mode[%d]}",
                hmac_vap->vap_mode);
            break;
    }
}

/*****************************************************************************
 功能描述  : 填写发送帧的加密控制信息，入参有效性由调用者保证
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_void hmac_tx_set_cipher_param(mac_tx_ctl_stru *tx_ctl,
    const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    unref_param(hmac_vap);

    /* 获取TXOP特性发送参数 */
    tx_ctl->cipher_key_id = hmac_user->key_info.default_index;
    tx_ctl->cipher_key_type = hmac_user->user_tx_info.security.cipher_key_type;
    tx_ctl->cipher_protocol_type = hal_cipher_suite_to_ctype(hmac_user->key_info.cipher_type);

    /* begin:如果接收到的EAPOL-KEY不加密，则STA 发送的EAPOL-KEY也不加密 */
    if (tx_ctl->is_eapol_key_ptk == OSAL_TRUE && hmac_user->is_rx_eapol_key_open == OSAL_TRUE) {
        tx_ctl->cipher_protocol_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    }
    /* end:如果接收到的EAPOL-KEY不加密，则STA 发送的EAPOL-KEY也不加密 */
}

/*****************************************************************************
 功能描述  : 填写连续netbuf的第一帧的CB信息，入参有效性由调用者保证
*****************************************************************************/
WIFI_HMAC_TCM_TEXT osal_void hmac_tx_fill_first_cb_info(mac_tx_ctl_stru *tx_ctl,
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_tx_set_protection_lsig_txop(tx_ctl, hmac_vap, hmac_user);
    hmac_tx_set_ps_param(tx_ctl, hmac_vap, hmac_user);

#ifdef _PRE_WLAN_FEATURE_11AX
    tx_ctl->ax_bss_color = hmac_user->he_hdl.he_oper_ie.bss_color.bss_color;
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP &&
        mac_mib_get_he_oper_bss_color_disable(hmac_vap) == OSAL_FALSE) {
        /* AP模式直接从mib获取bss color */
        tx_ctl->ax_bss_color = mac_mib_get_he_oper_bss_color(hmac_vap);
    }
#endif

    hmac_tx_set_cipher_param(tx_ctl, hmac_vap, hmac_user);
    tx_ctl->ra_lut_index = hmac_user->lut_index;
    tx_ctl->tx_hal_vap_index = hmac_vap->hal_vap->vap_id;
}

/*****************************************************************************
 功能描述  : 填写连续netbuf的第一帧之后的CB信息，入参有效性由调用者保证
*****************************************************************************/
OAL_STATIC WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_tx_fill_more_cb_info(mac_tx_ctl_stru *first_cb,
    mac_tx_ctl_stru *more_cb)
{
    more_cb->lsig_txop = first_cb->lsig_txop;
    more_cb->partial_aid = first_cb->partial_aid;
    more_cb->txop_ps_not_allowed = first_cb->txop_ps_not_allowed;
    more_cb->ax_bss_color = first_cb->ax_bss_color;
    more_cb->cipher_key_id = first_cb->cipher_key_id;
    more_cb->cipher_key_type = first_cb->cipher_key_type;
    more_cb->cipher_protocol_type = first_cb->cipher_protocol_type;
    more_cb->ra_lut_index = first_cb->ra_lut_index;
    more_cb->tx_hal_vap_index = first_cb->tx_hal_vap_index;
}

/*****************************************************************************
 功能描述  : 填写发送帧的CB参数
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT OAL_STATIC osal_void hmac_tx_fill_cb_param(oal_netbuf_head_stru *buff_head,
    hmac_tx_data_stat_stru *tx_data)
{
    oal_netbuf_stru *netbuf_next = buff_head->next;
    mac_tx_ctl_stru *fisrt_cb = OSAL_NULL;
    mac_tx_ctl_stru *cb = OSAL_NULL;

    if (osal_unlikely(netbuf_next == OSAL_NULL)) {
        return;
    }
    /* 填写首个CB */
    fisrt_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_next);
    hmac_tx_fill_first_cb_info(fisrt_cb, tx_data->hmac_vap, tx_data->user);

    netbuf_next = oal_get_netbuf_next(netbuf_next);
    while (netbuf_next != OSAL_NULL) {
        cb = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf_next);
        hmac_tx_fill_more_cb_info(fisrt_cb, cb);
        netbuf_next = oal_get_netbuf_next(netbuf_next);
    }
}

/*****************************************************************************
 功能描述  : 通知算法处理，填写后续CB参数
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_tx_data_send(hmac_tx_data_stat_stru *tx_data,
    oal_netbuf_head_stru *buff_head)
{
    hmac_vap_stru *hmac_vap = tx_data->hmac_vap;
    oal_netbuf_stru *netbuf = buff_head->next;
    osal_void *fhook = OSAL_NULL;

    /* 通知算法 */
    hmac_alg_tx_notify(hmac_vap, tx_data->user, tx_data->tx_ctl, OSAL_NULL);

    /* 如果用户处于节能状态，则需要考虑设置当前帧的more data */
    if (tx_data->user->ps_mode == OSAL_TRUE) {
        hmac_psm_tx_set_more_data(tx_data->user, tx_data->tx_ctl);
    }

    /* to do */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
#ifdef _PRE_WLAN_FEATURE_11AX
        hmac_vap->hal_device->current_11ax_working = (tx_data->user->he_hdl.he_capable != 0 &&
        tx_data->user->vht_hdl.vht_capable != 0 && tx_data->user->ht_hdl.ht_capable != 0) ?
        OSAL_TRUE : OSAL_FALSE;
#else
        hmac_vap->hal_device->current_11ax_working = OSAL_FALSE;
#endif
    }

    hmac_tx_fill_cb_param(buff_head, tx_data);
    /* 包括共存场景下, 对EAPOL帧处理 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_TX_VIP_FRAME);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_tx_vip_frame_cb)fhook)(tx_data->hmac_vap, buff_head, tx_data->tx_ctl->mpdu_num);
    }

    hmac_tx_send_mpdu(tx_data->hmac_vap, HCC_MSG_TYPE_DATA_FRAME, netbuf, tx_data->tx_ctl->mpdu_num);
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_process_data_check(hal_to_dmac_device_stru *hal_device,
    hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user, mac_tx_ctl_stru *tx_ctl)
{
    unref_param(hmac_device);
    if ((hal_device == OSAL_NULL) || (hmac_user == OSAL_NULL)) {
#ifdef _PRE_WIFI_DEBUG
        if (hal_device == OSAL_NULL) {
            pkt_drop_debug(tx_ctl, "hal_device_null");
        }
#endif
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, tx_ctl->msdu_num, OSAL_NULL);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((hmac_tx_is_tx_force(hmac_vap) == OSAL_FALSE) &&
        (((hmac_vap->vap_state != MAC_VAP_STATE_UP) && (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE)) ||
        (hmac_user->user_asoc_state == MAC_USER_STATE_BUTT))) {
#ifdef _PRE_WIFI_DEBUG
        pkt_drop_debug(tx_ctl, "tx_state_error");
#endif
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, tx_ctl->msdu_num, OSAL_NULL);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态发送帧时，如果发送帧VAP所在信道非当前工作信道则释放帧 */ /* 5G发11b,后期可删 */
    if (hmac_tx_enable_in_roaming(hal_device, hmac_vap) == OSAL_FALSE) {
#ifdef _PRE_WIFI_DEBUG
        pkt_drop_debug(tx_ctl, "vap_roaming_drop");
#endif
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, tx_ctl->msdu_num, OSAL_NULL);
        return OAL_FAIL;
    }
#endif

    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_tx_process_all_data_psm(const hmac_device_stru *hmac_device,
    hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf)
{
    osal_void *fhook = OSAL_NULL;
    osal_u32 ret;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_AP_UAPSD_ENQUEUE_PROC);
    if (fhook != OSAL_NULL) {
        ret = ((hmac_uapsd_enqueue_proc_cb)fhook)(hmac_vap, hmac_user, tx_ctl, netbuf);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }

    /* 判断要被发送的帧是否需要入节能缓存队列,判断条件1.AP有节能用户，2.P2P noa节能 3. TWT节能 */
    /* 4. 当前有处于WUR状态的用户且不是WUR的帧 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_NEED_BUFF);
    if ((hmac_vap->ps_user_num != 0) || (fhook != OSAL_NULL && ((hmac_is_twt_need_buff_cb)fhook)(hmac_vap))
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        || (hmac_vap->cur_wur_user_num != 0 && tx_ctl->is_wur == 0)
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
        || (((osal_u8)is_p2p_ps_enabled(hmac_vap) == OSAL_TRUE) && (hmac_device->st_p2p_info.p2p_ps_pause == OSAL_TRUE))
#endif
        ) {
        if (hmac_psm_pkt_need_buff(hmac_device, hmac_vap, hmac_user, netbuf) == OSAL_TRUE) {
#ifdef _PRE_WIFI_DEBUG
            pkt_trace_debug(netbuf, "HMAC_PSM_ENQUE");
#endif
            return hmac_psm_ps_enqueue(hmac_vap, hmac_user, netbuf);
        }
    }

    return OAL_CONTINUE;
}


/*****************************************************************************
 函 数 名  : alg_data_tx_notify
 功能描述  : 调用挂接到TX流程上的算法钩子
*****************************************************************************/
WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_alg_tx_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_tx_ctl_stru *cb, hal_tx_txop_alg_stru *txop_alg)
{
    hmac_alg_stru *alg_stru;
    osal_u32 index;
    if (osal_unlikely((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL) || (cb == OSAL_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_alg_tx_notify:: ERROR INFO: vap=%p, buf=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)hmac_user);
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_stru = (hmac_alg_stru *)hmac_alg_get_algorithm_main();
    /* 检查是否有算法注册 */
    if (alg_stru->alg_bitmap == 0) {
        return OAL_SUCC;
    }

    /* 管理帧不调用算法钩子 */
    if (cb->ac >= WLAN_WME_AC_MGMT) {
        return OAL_SUCC;
    }

    if (hmac_is_thruput_enable(THRUPUT_ALG_BYPASS) != 0) {
        return OAL_SUCC;
    }

    /* 调用钩子函数 */
    for (index = HMAC_ALG_TX_START; index < HMAC_ALG_TX_NOTIFY_BUTT; index++) {
        if (alg_stru->pa_tx_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_tx_notify_func[index](hmac_user, cb, txop_alg);
        }
    }

    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT OAL_STATIC osal_u32 hmac_direct_tx_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf)
{
    oal_netbuf_head_stru buff_head;
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_tx_data_stat_stru tx_data = {0};

    oal_netbuf_list_head_init(&buff_head);
    oal_netbuf_add_to_list_tail(netbuf, &buff_head);

    /* 报文链表最后必须置空 */
    oal_set_netbuf_next(netbuf, OSAL_NULL);

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 设置为漫游数据，标志以便发送完成时算法不进行统计 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        tx_ctl->roam_data = OSAL_TRUE;
    } else {
        tx_ctl->roam_data = OSAL_FALSE;
    }
#endif
    /* 正常发送流程 */
    tx_data.hmac_vap = hmac_vap;
    tx_data.user = hmac_user;
    tx_data.tx_ctl = tx_ctl;

    tx_data.tx_ctl->mpdu_num = 1;
    hmac_tx_data_send(&tx_data, &buff_head);
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_u32 hmac_tx_process_data(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_tx_ctl_stru *tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    hmac_user_stru *hmac_user = mac_res_get_hmac_user_etc(tx_ctl->tx_user_idx);
    osal_u8 hal_q = hal_ac_to_q_num(tx_ctl->ac); /* HAL Q NUM */
    hmac_tid_stru *tid_queue = OSAL_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_RECORD_LATENCY_TX);
    osal_u32 ret;

    if (mac_pk_mode(netbuf)) {
        hmac_alg_tx_notify(hmac_vap, hmac_user, tx_ctl, OSAL_NULL);
        ret = frw_hmac_send_data(netbuf, hmac_vap->vap_id, HCC_MSG_TYPE_DATA_FRAME);
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oal_netbuf_free(netbuf);
        }
        return OAL_SUCC;
    }

    ret = hmac_tx_process_data_check(hal_device, hmac_device, hmac_vap, hmac_user, tx_ctl);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_TX, "{hmac_tx_process_data: check err.}");
        return ret;
    }
    /* 统计处理时延 */
    if (fhook != OSAL_NULL) {
        ((hmac_record_latency_tx_cb)fhook)(netbuf, TX_HMAC_PROC_AMSDU);
    }

    /* host侧TX方向管理帧上报到HSO */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_FRAME_REPORT_HSO);
    if (fhook != OSAL_NULL) {
        ((dfx_frame_report_hso_cb)fhook)(netbuf, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    }

    ret = hmac_tx_process_all_data_psm(hmac_device, hmac_vap, hmac_user, tx_ctl, netbuf);
    if (ret != OAL_CONTINUE) {
#ifdef _PRE_WLAN_DFT_STAT
        if (ret != OAL_SUCC) {
#ifdef _PRE_WIFI_DEBUG
            pkt_drop_debug(tx_ctl, "psm_enque_fail");
#endif
            /* 数据报文丢弃 */
            hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, tx_ctl->msdu_num, netbuf);
        }
#endif
        return ret;
    }

    /* 获取用户的特定TID队列 */
    tid_queue = &(hmac_user->tx_tid_queue[tx_ctl->tid]);
    ret = hmac_tx_process_tid_queue(hal_device, hmac_vap, netbuf, tid_queue, hal_q);
    if (ret != OAL_CONTINUE) {
        return ret;
    }
#ifdef _PRE_WIFI_DEBUG
    pkt_trace_debug(netbuf, "HMAC_DATA_DIRECT_TX");
#endif
    return hmac_direct_tx_process(hmac_vap, hmac_user, netbuf);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
