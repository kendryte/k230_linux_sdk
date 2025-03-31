/*
 * Copyright (c) CompanyNameMagicTag 2013-2023. All rights reserved.
 * 文 件 名   : hmac_uapsd.c
 * 生成日期   : 2013年9月18日
 * 功能描述   : UAPSD hmac层处理
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_uapsd.h"
#include "wlan_spec.h"
#include "mac_vap_ext.h"
#include "mac_frame.h"
#include "hmac_mgmt_ap.h"
#include "hmac_encap_frame_ap.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_rx_data.h"
#include "wlan_msg.h"
#include "oal_net.h"
#include "hal_common_ops.h"
#include "hal_ext_if.h"
#include "mac_device_ext.h"
#include "hmac_vap.h"
#include "hmac_main.h"
#include "hmac_user.h"
#include "hmac_psm_ap.h"
#include "hmac_tx_mgmt.h"
#include "hmac_feature_dft.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_tx_mpdu_queue.h"
#include "hmac_feature_interface.h"
#include "frw_util_notifier.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_UAPSD_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

hmac_user_uapsd_info_stru *g_user_uapsd_info[WLAN_USER_MAX_USER_LIMIT];
osal_u8 g_max_queue_len[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    HMAC_UAPSD_QDEPTH_DEFAULT, HMAC_UAPSD_QDEPTH_DEFAULT, HMAC_UAPSD_QDEPTH_DEFAULT, HMAC_UAPSD_QDEPTH_DEFAULT
};

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_uapsd_send_qosnull(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 ac);
OSAL_STATIC osal_void hmac_uapsd_process_trigger(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 ac,
    const oal_netbuf_stru *net_buf);
OSAL_STATIC osal_s32 hmac_uapsd_process_queue(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 ac);
OSAL_STATIC osal_s32 hmac_uapsd_flush_queue(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_s32 hmac_config_set_uapsd_update(hmac_vap_stru *hmac_vap, osal_u8 *data);

OSAL_STATIC osal_u8 hmac_get_uapsd_max_queue_len(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return HMAC_UAPSD_QDEPTH_DEFAULT;
    }
    return g_max_queue_len[vap_id];
}

OSAL_STATIC hmac_user_uapsd_info_stru *hmac_ap_get_user_uapsd_info(osal_u16 assoc_id)
{
    if (hmac_user_assoc_id_param_check(assoc_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }
    return g_user_uapsd_info[assoc_id];
}

/*****************************************************************************
 函 数 名  : hmac_psm_is_uapsd_empty
 功能描述  : 判断某一个用户的uspsd队列是否都为空，如果非空就返回false
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_psm_is_uapsd_empty(hmac_user_stru *hmac_user)
{
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;

    if (hmac_user == OSAL_NULL) {
        return OSAL_TRUE;
    }

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OSAL_TRUE;
    }

    if (osal_adapt_atomic_read(&user_uapsd_info->uapsd_stru.mpdu_num) == 0) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

OAL_STATIC osal_void hmac_uapsd_fill_st_uapsd_status(mac_user_uapsd_status_stru *uapsd_status,
    const osal_u8 *wmm_ie, osal_u8 *uapsd_flag)
{
    osal_u8 ret = OAL_FALSE;
    osal_u8 max_sp;
    uapsd_status->qos_info = wmm_ie[HMAC_UAPSD_WME_LEN];

    /* 8为WMM IE长度 */
    if ((wmm_ie[HMAC_UAPSD_WME_LEN] & BIT0) == BIT0) {
        uapsd_status->ac_trigger_ena[WLAN_WME_AC_VO] = 1;
        uapsd_status->ac_delievy_ena[WLAN_WME_AC_VO] = 1;
        ret = OAL_TRUE;
    }

    if ((wmm_ie[HMAC_UAPSD_WME_LEN] & BIT1) == BIT1) {
        uapsd_status->ac_trigger_ena[WLAN_WME_AC_VI] = 1;
        uapsd_status->ac_delievy_ena[WLAN_WME_AC_VI] = 1;
        ret = OAL_TRUE;
    }

    if ((wmm_ie[HMAC_UAPSD_WME_LEN] & BIT2) == BIT2) {
        uapsd_status->ac_trigger_ena[WLAN_WME_AC_BK] = 1;
        uapsd_status->ac_delievy_ena[WLAN_WME_AC_BK] = 1;
        ret = OAL_TRUE;
    }

    if ((wmm_ie[HMAC_UAPSD_WME_LEN] & BIT3) == BIT3) {
        uapsd_status->ac_trigger_ena[WLAN_WME_AC_BE] = 1;
        uapsd_status->ac_delievy_ena[WLAN_WME_AC_BE] = 1;
        ret = OAL_TRUE;
    }

    if (ret == OAL_TRUE) {
        *uapsd_flag |= HMAC_USR_UAPSD_EN;
    }

    /* 设置max SP长度 */
    max_sp = (wmm_ie[HMAC_UAPSD_WME_LEN] >> 5) & 0x3; /* 5偏移 */
    switch (max_sp) {
        case 1:
            uapsd_status->max_sp_len = 2; /* 2设置max SP长度 */
            break;
        case 2: /* 2类型判断 */
            uapsd_status->max_sp_len = 4; /* 4设置max SP长度 */
            break;
        case 3: /* 3类型判断 */
            uapsd_status->max_sp_len = 6; /* 6设置max SP长度 */
            break;
        default:
            uapsd_status->max_sp_len = HMAC_UAPSD_SEND_ALL;
            break;
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_update_user_para_etc
 功能描述  : uapsd处理关联请求中的WMM IE
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_uapsd_update_user_para_etc(osal_u8 *payload, osal_u8 sub_type, osal_u32 msg_len,
    hmac_user_stru *hmac_user)
{
    osal_s32          ret;
    osal_u8           uapsd_flag = 0;
    mac_user_uapsd_status_stru   uapsd_status;
    hmac_vap_stru       *hmac_vap = OAL_PTR_NULL;
    osal_u16          len;
    osal_u16          len_total;
    osal_u8          *param = OAL_PTR_NULL;
    osal_u8          *wmm_ie = OAL_PTR_NULL;

    unref_param(sub_type);

    wmm_ie = hmac_get_wmm_ie_etc(payload, (osal_u16)msg_len);
    if (wmm_ie == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] Could not find WMM IE in assoc req,user_id[%d]\n",
                         hmac_user->vap_id, hmac_user->assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    memset_s(&uapsd_status, OAL_SIZEOF(mac_user_uapsd_status_stru), 0, OAL_SIZEOF(mac_user_uapsd_status_stru));
    hmac_uapsd_fill_st_uapsd_status(&uapsd_status, wmm_ie, &uapsd_flag);

    /* Send uapsd_flag & uapsd_status syn to dmac */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_user->vap_id);
    if (osal_unlikely(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_uapsd_update_user_para_etc::get mac_vap [%d] null.}",
                       hmac_user->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    len = OAL_SIZEOF(osal_u16);
    len_total = OAL_SIZEOF(osal_u16) + OAL_SIZEOF(osal_u8) + OAL_SIZEOF(mac_user_uapsd_status_stru);
    param = (osal_u8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, len_total, OAL_TRUE);
    if (osal_unlikely(param == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_uapsd_update_user_para_etc::param null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    // user_index
    (osal_void)memcpy_s(param, len, &(hmac_user->assoc_id), len);
    // uapsd_flag
    (osal_void)memcpy_s(param + len, OAL_SIZEOF(osal_u8), &uapsd_flag, OAL_SIZEOF(osal_u8));
    len += OAL_SIZEOF(osal_u8);
    // uapsd_status
    (osal_void)memcpy_s(param + len, OAL_SIZEOF(mac_user_uapsd_status_stru),
        &uapsd_status, OAL_SIZEOF(mac_user_uapsd_status_stru));
    len += OAL_SIZEOF(mac_user_uapsd_status_stru);

    ret = hmac_config_set_uapsd_update(hmac_vap, param);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_uapsd_update_user_para_etc::hmac_config_set_uapsd_update fail %u}",
            hmac_vap->vap_id, ret);
    }
    oal_mem_free(param, OAL_TRUE);
    return OAL_SUCC;
}

OSAL_STATIC osal_u8 hmac_usr_uapsd_use_tim(const hmac_user_stru *hmac_usr)
{
    mac_user_uapsd_status_stru uapsd_status = hmac_ap_get_user_uapsd_info(hmac_usr->assoc_id)->uapsd_status;
    osal_u8 uapsd_flag = hmac_ap_get_user_uapsd_info(hmac_usr->assoc_id)->uapsd_flag;

    return (((uapsd_status.ac_delievy_ena[WLAN_WME_AC_BK] != 0) && ((uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0)) &&
        ((uapsd_status.ac_delievy_ena[WLAN_WME_AC_BE] != 0) && ((uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0)) &&
        ((uapsd_status.ac_delievy_ena[WLAN_WME_AC_VI] != 0) && ((uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0)) &&
        ((uapsd_status.ac_delievy_ena[WLAN_WME_AC_VO] != 0) && ((uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0)));
}

OSAL_STATIC osal_u32 hmac_uapsd_state_trans(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const mac_ieee80211_qos_frame_stru *mac_header)
{
    hmac_user_uapsd_stru *uapsd_stru = OSAL_NULL;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;
    osal_u8 uapsd_flag;
    osal_s32 uapsd_qdepth;

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OSAL_FALSE;
    }
    uapsd_stru = &user_uapsd_info->uapsd_stru;
    uapsd_flag = user_uapsd_info->uapsd_flag;
    if ((((mac_header->frame_control.power_mgmt) == 1) && ((uapsd_flag & HMAC_USR_UAPSD_TRIG) == 0)) ||
        (((mac_header->frame_control.power_mgmt) == 0) && ((uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0))) {
        user_uapsd_info->uapsd_flag &= ~HMAC_USR_UAPSD_SP;
        if (mac_header->frame_control.power_mgmt == 1) {
            /* 从非节能态迁移到节能态 */
            (osal_void)memset_s(&uapsd_stru->uapsd_trigseq[0], sizeof(uapsd_stru->uapsd_trigseq),
                0xff, sizeof(uapsd_stru->uapsd_trigseq));
            user_uapsd_info->uapsd_flag |= HMAC_USR_UAPSD_TRIG;
        } else {
            /* flush 节能队列 */
            uapsd_qdepth = hmac_uapsd_flush_queue(hmac_vap, hmac_user);
            if ((uapsd_qdepth == 0) && (hmac_usr_uapsd_use_tim(hmac_user) != 0)) {
                /* 调用PSM的TIM设置接口 */
                hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);
            }
            /* 从节能态迁移到非节能态 */
            user_uapsd_info->uapsd_flag &= ~HMAC_USR_UAPSD_TRIG;
        }
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_trigger_check
 功能描述  : 接收报文流程中检查是否trigger帧
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_void hmac_uapsd_rx_trigger_check(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const oal_netbuf_stru *net_buf)
{
    mac_ieee80211_qos_frame_stru *mac_header = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    osal_u8 uapsd_flag;
    osal_u32 istrigger = OSAL_FALSE;
    osal_u8 tid;
    osal_u8 ac = WLAN_WME_AC_BE;
    osal_u32 run_time;
    osal_u32 cur_tsf;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL) {
        return;
    }

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return;
    }

    uapsd_flag = user_uapsd_info->uapsd_flag;

    /* 如果当前Usr不是uapsd使能的返回 */
    if ((uapsd_flag & HMAC_USR_UAPSD_EN) == 0) {
        return;
    }

    /* 如果当前Usr处于一个Service Period中，返回 */
    cur_tsf = (osal_u32)osal_get_time_stamp_ms();
    run_time = osal_get_runtime((user_uapsd_info->uapsd_status.last_esop_tsf), cur_tsf);
    if (((uapsd_flag & HMAC_USR_UAPSD_SP) != 0) && run_time <= HMAC_UAPSD_IDLE_INT) {
        return;
    }
    user_uapsd_info->uapsd_flag &= ~HMAC_USR_UAPSD_SP;

    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb_const(net_buf);
    mac_header = (mac_ieee80211_qos_frame_stru *)(mac_get_rx_cb_mac_hdr(rx_ctrl));
    if (hmac_uapsd_state_trans(hmac_vap, hmac_user, mac_header) == OSAL_TRUE) {
        return;
    }

    /* 检查是否是一个trigger帧:QOS DATA or Qos NULL DATA，非trigger帧也要继续处理是否需要状态切换 */
    if ((mac_header->frame_control.type == WLAN_DATA_BASICTYPE) &&
        ((mac_header->frame_control.sub_type == WLAN_QOS_DATA) ||
        (mac_header->frame_control.sub_type == WLAN_QOS_NULL_FRAME))) {
        istrigger = OSAL_TRUE;
        tid = mac_header->qc_tid;
        ac = wlan_wme_tid_to_ac(tid);
    }

    /* AC是trigge_en的且用户当前处于trigger状态，can be trigger */
    if ((istrigger == OSAL_TRUE) &&
        ((user_uapsd_info->uapsd_status.ac_trigger_ena[ac] == OSAL_TRUE) &&
        ((user_uapsd_info->uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0))) {
        hmac_uapsd_process_trigger(hmac_vap, hmac_user, ac, net_buf);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_process_trigger
 功能描述  : trigger帧的处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_uapsd_process_trigger(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 ac,
    const oal_netbuf_stru *net_buf)
{
    mac_ieee80211_qos_frame_stru *mac_header = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
    osal_s32 uapsd_qdepth;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;

    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb_const(net_buf);
    mac_header = (mac_ieee80211_qos_frame_stru *)(mac_get_rx_cb_mac_hdr(rx_ctrl));

    if (hmac_user->is_multi_user == OSAL_FALSE) {
        hmac_tid_resume(&hmac_user->tx_tid_queue[ac], DMAC_TID_PAUSE_RESUME_TYPE_PS);
    }
    hal_tx_disable_peer_sta_ps_ctrl(hmac_user->lut_index);

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return;
    }

    if ((mac_header->frame_control.retry == OSAL_TRUE) &&
        (mac_header->seq_num == user_uapsd_info->uapsd_stru.uapsd_trigseq[ac])) {
        return;
    }
    user_uapsd_info->uapsd_stru.uapsd_trigseq[ac] = mac_header->seq_num;

    uapsd_qdepth = hmac_uapsd_process_queue(hmac_vap, hmac_user, ac);
    /* 在设置DMAC_USR_UAPSD_SP前，需判断是否有发送报文，-1表示没有报文发送，不应该置上DMAC_USR_UAPSD_SP */
    if (uapsd_qdepth == HMAC_UAPSD_NOT_SEND_FRAME) {
        oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_process_trigger::NOT_SEND_FRAME == uapsd_qdepth.}",
            hmac_user->vap_id);
        return;
    }

    user_uapsd_info->uapsd_flag |= HMAC_USR_UAPSD_SP;
    if ((uapsd_qdepth == 0) && hmac_usr_uapsd_use_tim(hmac_user) == OSAL_TRUE) {
        /* 调用PSM的TIM设置接口 */
        oam_info_log2(0, OAM_SF_PWR, "vap_id[%d] hmac_uapsd_process_trigger:set PVB to 0,usr id = %d",
            hmac_user->vap_id, hmac_user->assoc_id);

        hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);
    }
}

OSAL_STATIC osal_void hmac_uapsd_set_frame_hdr(mac_ieee80211_qos_frame_stru *mac_header,
    hmac_user_uapsd_stru *hmac_uapsd, const mac_tx_ctl_stru *tx_ctrl, osal_u8 sp_last, osal_u8 *extra_qosnull)
{
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    osal_u8 is_eosp;

    /* 管理帧没有EOSP位，因此额外发送一个qos null结束USP */
    if (mac_header->frame_control.type == WLAN_MANAGEMENT ||
        mac_header->frame_control.type == WLAN_CONTROL) {
        frame_hdr = mac_get_frame_header_addr(tx_ctrl);
        frame_hdr->frame_control.more_data = 0;
        *extra_qosnull = OSAL_TRUE;
    } else if (mac_header->frame_control.type == WLAN_DATA_BASICTYPE) {
        /*
                每次发送时如果后面还有帧，则将EOSP置0，More Data置1。
                如果到max sp length，后面还有缓存帧，则将EOSP置1，More Data置1。
                如果后面没有帧发送，则将EOSP置1，More Data置0。
                当delivery-enable的AC队列中没有缓存帧，发送一个QoSNull帧，More Data置0。
        */
        is_eosp = OSAL_TRUE;
        if (sp_last == OSAL_FALSE) {
            mac_header->frame_control.more_data = 1;
            is_eosp = OSAL_FALSE;
        } else if (osal_adapt_atomic_read(&hmac_uapsd->mpdu_num) == 1) {
            mac_header->frame_control.more_data = 0;
        } else {
            mac_header->frame_control.more_data = 1;
        }

        if (mac_header->frame_control.sub_type == WLAN_QOS_DATA ||
            mac_header->frame_control.sub_type == WLAN_QOS_NULL_FRAME) {
            mac_header->qc_eosp = is_eosp;
        } else {
            *extra_qosnull = is_eosp;
        }
        oam_info_log4(0, OAM_SF_PWR, "{hmac_uapsd_set_frame_hdr:: more data %d, type %02x, subtype %02x, eosp %d.}",
            mac_header->frame_control.more_data, mac_header->frame_control.type,
            mac_header->frame_control.sub_type, is_eosp);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_tx_pkt
 功能描述  : U-APSD节能队列发包
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_uapsd_tx_pkt(oal_netbuf_stru *net_buf, hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u8 ac, osal_u8 sp_last, osal_u8 *extra_qosnull)
{
    hmac_user_uapsd_stru *hmac_uapsd = OSAL_NULL;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;
    mac_ieee80211_qos_frame_stru *mac_header = OSAL_NULL;

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_uapsd = &user_uapsd_info->uapsd_stru;

    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    tx_ctrl->is_get_from_ps_queue = OSAL_TRUE;

    /* 非flush操作，tid设置为uapsd专用tid，ac设置为trigger的AC */
    tx_ctrl->tid = WLAN_TIDNO_UAPSD;
    tx_ctrl->ac = ac;

    if (osal_unlikely(extra_qosnull == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TXOP, "{hmac_uapsd_tx_pkt::param is null}.");
        return OAL_FAIL;
    }

    mac_header = (mac_ieee80211_qos_frame_stru *)mac_get_frame_header_addr(tx_ctrl);
    hmac_uapsd_set_frame_hdr(mac_header, hmac_uapsd, tx_ctrl, sp_last, extra_qosnull);
    osal_adapt_atomic_dec(&hmac_uapsd->mpdu_num);

    if (mac_header->frame_control.type == WLAN_DATA_BASICTYPE) {
        if (mac_header->qc_eosp == 1) {
            tx_ctrl->need_rsp = OSAL_TRUE;
            user_uapsd_info->uapsd_status.last_esop_tsf = (osal_u32)osal_get_time_stamp_ms();
        }

        /* 发送不成功，则终止本次USP */
        if (hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, net_buf) != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_PWR,
                "vap_id[%d] {hmac_uapsd_tx_pkt::hmac_tx_process_data failed.}", hmac_user->vap_id);
            *extra_qosnull = OSAL_TRUE;
            hmac_tx_excp_free_netbuf(net_buf);
            return OAL_FAIL;
        }
    } else {
        /* 发送不成功，则终止本次USP */
        if (hmac_tx_mgmt(hmac_vap, net_buf, tx_ctrl->mpdu_payload_len + tx_ctrl->frame_header_length, OSAL_TRUE) !=
            OAL_SUCC) {
            oal_netbuf_free(net_buf);
            oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_tx_pkt::hmac_tx_mgmt failed.}", hmac_user->vap_id);
            *extra_qosnull = OSAL_TRUE;
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_process_queue
 功能描述  : U-APSD节能队列发包
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_uapsd_process_queue(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 ac)
{
    osal_u8 send_num, loop, extra_qosnull, sp_last;
    hmac_user_uapsd_stru *hmac_uapsd = OSAL_NULL;
    mac_user_uapsd_status_stru *mac_uapsd_status = OSAL_NULL;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;
    oal_netbuf_stru *net_buf = OSAL_NULL;

    extra_qosnull = sp_last = OSAL_FALSE;

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_uapsd = &user_uapsd_info->uapsd_stru;

    if (osal_adapt_atomic_read(&hmac_uapsd->mpdu_num) == 0) {
        return (hmac_uapsd_send_qosnull(hmac_vap, hmac_user, ac) != OAL_SUCC) ? HMAC_UAPSD_NOT_SEND_FRAME : 0;
    }

    mac_uapsd_status = &user_uapsd_info->uapsd_status;

    send_num = (mac_uapsd_status->max_sp_len < (osal_u8)osal_adapt_atomic_read(&hmac_uapsd->mpdu_num)) ?
        mac_uapsd_status->max_sp_len : (osal_u8)osal_adapt_atomic_read(&hmac_uapsd->mpdu_num);
    for (loop = 0; loop < send_num; loop++) {
        osal_spin_lock(&hmac_uapsd->lock_uapsd);
        net_buf = hmac_tx_dequeue_first_mpdu(&(hmac_uapsd->uapsd_queue_head));
        if (net_buf == OSAL_NULL) {
            osal_spin_unlock(&hmac_uapsd->lock_uapsd);

            oam_error_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_process_queue::net_buf,mpdu_num=%d.}",
                hmac_user->vap_id, osal_adapt_atomic_read(&hmac_uapsd->mpdu_num));
            return osal_adapt_atomic_read(&hmac_uapsd->mpdu_num);
        }

        osal_spin_unlock(&hmac_uapsd->lock_uapsd);

        if (loop == send_num - 1) {
            sp_last = OSAL_TRUE;
        }

        if (hmac_uapsd_tx_pkt(net_buf, hmac_vap, hmac_user, ac, sp_last, &extra_qosnull) != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_process_queue::failed:assoc_id = %d.}",
                hmac_user->vap_id, hmac_user->assoc_id);
            break;
        }
    }

    if (extra_qosnull == OSAL_TRUE) {
        hmac_uapsd_send_qosnull(hmac_vap, hmac_user, ac);
    }

    return osal_adapt_atomic_read(&hmac_uapsd->mpdu_num);
}

OSAL_STATIC osal_void hmac_uapsd_flush_queue_tx_frame(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *net_buf, mac_tx_ctl_stru *tx_ctrl)
{
    mac_ieee80211_frame_stru *frame_hdr = mac_get_frame_header_addr(tx_ctrl);

    if (frame_hdr->frame_control.type == WLAN_DATA_BASICTYPE) {
        /* 发送不成功，则终止本次USP */
        if (hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, net_buf) != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_flush_queue::tx_process_data failed.}",
                hmac_vap->vap_id);
            hmac_tx_excp_free_netbuf(net_buf);
        }
    } else {
        /* 发送不成功，则终止本次USP */
        if (hmac_tx_mgmt(hmac_vap, net_buf, tx_ctrl->mpdu_payload_len + tx_ctrl->frame_header_length, OSAL_TRUE) !=
            OAL_SUCC) {
            oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_flush_queue::hmac_tx_mgmt failed.}",
                hmac_vap->vap_id);
            hmac_tx_excp_free_netbuf(net_buf);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_flush_queue
 功能描述  : U-APSD节能队列flush
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_uapsd_flush_queue(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    oal_netbuf_stru *net_buf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    hmac_user_uapsd_stru *hmac_uapsd = OSAL_NULL;
    hmac_user_uapsd_info_stru *user_uapsd_info;

    if (osal_unlikely(hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TXOP, "{hmac_uapsd_flush_queue::param is null}.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_uapsd = &user_uapsd_info->uapsd_stru;

    while (osal_adapt_atomic_read(&hmac_uapsd->mpdu_num) != 0) {
        osal_spin_lock(&hmac_uapsd->lock_uapsd);
        net_buf = hmac_tx_dequeue_first_mpdu(&(hmac_uapsd->uapsd_queue_head));
        if (net_buf == OSAL_NULL) {
            osal_spin_unlock(&hmac_uapsd->lock_uapsd);

            oam_error_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_uapsd_flush_queue::net_buf null, mpdu_num=%d.}",
                hmac_user->vap_id, osal_adapt_atomic_read(&hmac_uapsd->mpdu_num));
            return osal_adapt_atomic_read(&hmac_uapsd->mpdu_num);
        }

        osal_spin_unlock(&hmac_uapsd->lock_uapsd);

        tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
        tx_ctrl->is_get_from_ps_queue = OSAL_TRUE;

        /* flush流程，视为正常流程的继续，清掉more data bit */
        if (osal_adapt_atomic_read(&hmac_uapsd->mpdu_num) == 1) {
            frame_hdr = mac_get_frame_header_addr(tx_ctrl);
            frame_hdr->frame_control.more_data = 0;
        }

        osal_adapt_atomic_dec(&hmac_uapsd->mpdu_num);

        hmac_uapsd_flush_queue_tx_frame(hmac_vap, net_buf, tx_ctrl);
    }

    return osal_adapt_atomic_read(&hmac_uapsd->mpdu_num);
}

OSAL_STATIC osal_void hmac_uapsd_fill_qosnull_txctl(mac_tx_ctl_stru *tx_ctrl, osal_u8 ac, osal_u8 vap_id,
    osal_u8 assoc_id)
{
    /* 填写tx部分 */
    tx_ctrl->ack_policy = WLAN_TX_NORMAL_ACK;
    tx_ctrl->ac = ac;
    tx_ctrl->retried_num = 0;
    tx_ctrl->tid = WLAN_TIDNO_UAPSD;
    tx_ctrl->tx_vap_index = vap_id;
    /* 填写tx rx公共部分 */
    tx_ctrl->is_get_from_ps_queue = OSAL_TRUE;
    tx_ctrl->frame_header_length = sizeof(mac_ieee80211_qos_frame_stru);
    tx_ctrl->mpdu_num = 1;
    tx_ctrl->msdu_num = 1;
    tx_ctrl->netbuf_num = 1;
    tx_ctrl->mpdu_payload_len = 0;
    tx_ctrl->tx_user_idx = assoc_id;
    tx_ctrl->need_rsp = OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_tx_complete
 功能描述  : U-APSD 发送完成检查
*****************************************************************************/
OSAL_STATIC osal_void hmac_uapsd_tx_complete(hmac_user_stru *hmac_user)
{
    if (hmac_ap_get_user_uapsd_info(hmac_user->assoc_id) == OSAL_NULL) {
        return;
    }
    hmac_ap_get_user_uapsd_info(hmac_user->assoc_id)->uapsd_flag &= ~HMAC_USR_UAPSD_SP;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_send_qosnull
 功能描述  : U-APSD qos null data发送
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_uapsd_send_qosnull(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 ac)
{
    oal_netbuf_stru *net_buf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;
    osal_u32 ret;
    mac_ieee80211_qos_frame_stru *mac_header = OSAL_NULL;

    /* 申请net_buff */
    net_buf = oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (net_buf == OSAL_NULL) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_uapsd_send_qosnull::pnet_buf failed.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    oal_netbuf_reserve(net_buf, (OAL_MAX_MAC_HDR_LEN - sizeof(mac_ieee80211_qos_frame_stru)));
    oal_set_netbuf_prev(net_buf, OSAL_NULL);
    oal_set_netbuf_next(net_buf, OSAL_NULL);

    /* 填写帧头,其中from ds为1，to ds为0，因此frame control的第二个字节为02 */
    (osal_void)memset_s(oal_netbuf_header(net_buf), sizeof(mac_ieee80211_qos_frame_stru),
        0, sizeof(mac_ieee80211_qos_frame_stru));
    mac_null_data_encap(oal_netbuf_header(net_buf),
        (osal_u16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS_NULL) | 0x0200,
        hmac_user->user_mac_addr, hmac_vap->bssid);

    mac_header = (mac_ieee80211_qos_frame_stru *)oal_netbuf_header(net_buf);
    mac_header->qc_tid = wlan_wme_ac_to_tid(ac);
    mac_header->qc_eosp = 1;
    /* 协议规定单播的QOS NULL DATA只允许normal ack */
    mac_header->qc_ack_polocy = WLAN_TX_NORMAL_ACK;

    /* 填写cb字段 */
    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    hmac_uapsd_fill_qosnull_txctl(tx_ctrl, ac, hmac_vap->vap_id, (osal_u8)hmac_user->assoc_id);
    oal_netbuf_put(net_buf, tx_ctrl->frame_header_length);

    ret = hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, net_buf);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_PWR,
            "vap_id[%d] {hmac_uapsd_send_qosnull::tx_process_data failed[%d].}", hmac_vap->vap_id, ret);
        hmac_tx_excp_free_netbuf(net_buf);
        hmac_uapsd_tx_complete(hmac_user);
    }
    hmac_ap_get_user_uapsd_info(hmac_user->assoc_id)->uapsd_status.last_esop_tsf = (osal_u32)osal_get_time_stamp_ms();
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_usr_uapsd_ac_tigger
 功能描述  : BE/BK/VI/VO具有delivery属性，其余ac无需判断
*****************************************************************************/
OSAL_STATIC osal_u8 hmac_usr_uapsd_ac_tigger(wlan_wme_ac_type_enum_uint8 ac, const hmac_user_stru *hmac_usr)
{
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_usr->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OSAL_FALSE;
    }

    return ((ac < WLAN_WME_AC_BUTT) ?
        (((user_uapsd_info->uapsd_status.ac_delievy_ena[ac] != 0) &&
        ((user_uapsd_info->uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0))) :
        ((user_uapsd_info->uapsd_flag & HMAC_USR_UAPSD_TRIG) != 0));
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_tx_need_enqueue
 功能描述  : 发送时报文是否入UAPSD队列
*****************************************************************************/
WIFI_TCM_TEXT OSAL_STATIC osal_u8 hmac_uapsd_tx_need_enqueue(const hmac_vap_stru *hmac_vap,
    const hmac_user_stru *hmac_user, const mac_tx_ctl_stru *tx_ctl)
{
    wlan_wme_ac_type_enum_uint8 ac;

    /* 漫游状态时，强制发送该数据帧 */
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (hmac_vap->vap_state == MAC_VAP_STATE_ROAMING) {
        return OSAL_FALSE;
    }
#endif

    ac = tx_ctl->ac;
    if ((hmac_vap->cap_flag.uapsd == WLAN_FEATURE_UAPSD_IS_OPEN) && (tx_ctl->ismcast == OSAL_FALSE) &&
        (tx_ctl->is_get_from_ps_queue == OSAL_FALSE) && (hmac_usr_uapsd_ac_tigger(ac, hmac_user) == OSAL_TRUE)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_tx_enqueue
 功能描述  : 发送时报文入UAPSD队列
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_uapsd_tx_enqueue(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *net_buf)
{
    oal_netbuf_stru        *first_net_buf = OSAL_NULL;
    oal_netbuf_stru        *netbuf = OSAL_NULL;
    mac_tx_ctl_stru        *tx_ctrl = OSAL_NULL;
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;
    hmac_user_uapsd_stru *uapsd_stru = OSAL_NULL;

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    uapsd_stru = &(user_uapsd_info->uapsd_stru);
    /* 判断是否需要更新bitmap */
    if ((osal_adapt_atomic_read(&uapsd_stru->mpdu_num) == 0) && hmac_usr_uapsd_use_tim(hmac_user) == OSAL_TRUE) {
        hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 1);
    }

    /* 对UAPSD节能队列进行操作，加锁保护 */
    osal_spin_lock(&uapsd_stru->lock_uapsd);

    first_net_buf = net_buf;
    while ((osal_adapt_atomic_read(&uapsd_stru->mpdu_num) < hmac_get_uapsd_max_queue_len(hmac_vap->vap_id)) &&
        (first_net_buf != OSAL_NULL)) {
        /* 从每一个mpdu中第一个net_buf的CB字段获取该mpdu一共包含几个net_buff */
        tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(first_net_buf);

        /* 入队时设置more data bit，减少出队时的操作 */
        frame_hdr = mac_get_frame_header_addr(tx_ctrl);
        frame_hdr->frame_control.more_data = 0x01;

        /* 将该mpdu的每一个net_buff加入到节能队列中 */
        netbuf = first_net_buf;
        while (
            (netbuf != OSAL_NULL)) {
            first_net_buf = oal_get_netbuf_next(netbuf);

            oal_netbuf_add_to_list_tail(netbuf, &uapsd_stru->uapsd_queue_head);

            netbuf = first_net_buf;
        }

        /* 更新节能队列中mpdu的个数 */
        osal_adapt_atomic_inc(&uapsd_stru->mpdu_num);
    }

    osal_spin_unlock(&uapsd_stru->lock_uapsd);

    /*
       判断是mpdu全都入队了还是队列满了，如果是因为队列满了并且还有mpdu没有入队，
       则将剩下的mpdu释放
    */
    if ((osal_adapt_atomic_read(&uapsd_stru->mpdu_num) == hmac_get_uapsd_max_queue_len(hmac_vap->vap_id)) &&
        (first_net_buf != OSAL_NULL)) {
        hmac_tx_excp_free_netbuf(first_net_buf);
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_uapsd_tx_enqueue::some mpdus are released due to queue full.}",
            hmac_user->vap_id);
    }

    return OAL_SUCC;
}

WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_uapsd_enqueue_proc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_tx_ctl_stru *tx_ctl, oal_netbuf_stru *netbuf)
{
    if (osal_unlikely(hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TXOP, "{hmac_uapsd_enqueue_proc::param is null}.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_uapsd_tx_need_enqueue(hmac_vap, hmac_user, tx_ctl) == OSAL_FALSE) {
        return OAL_CONTINUE;
    }

    return hmac_uapsd_tx_enqueue(hmac_vap, hmac_user, netbuf);
}

OSAL_STATIC osal_s32 hmac_config_tx_comp_ps_handle(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u16 aid;

    dmac_crx_ps_tx_status_stru *status_msg = (dmac_crx_ps_tx_status_stru *)msg->data;
    if (hmac_vap == OSAL_NULL || hmac_vap->init_flag == MAC_VAP_INVAILD || !is_ap(hmac_vap)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_tx_comp_ps_handle:: input param invalid!!!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    aid = status_msg->user_idx;
    hmac_user = mac_res_get_hmac_user_etc(aid);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_tx_comp_ps_handle:: user[%d] is null!!!}", aid);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_uapsd_tx_complete(hmac_user);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_uapsd_update
 功能描述  : VAP U-APSD UPDATE
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_set_uapsd_update(hmac_vap_stru *hmac_vap, osal_u8 *data)
{
    osal_u16 user_id;
    osal_u8 uapsd_flag;
    mac_user_uapsd_status_stru *uapsd_status = OSAL_NULL;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 len_tmp;
    hmac_user_uapsd_info_stru *user_uapsd_info = OSAL_NULL;

    user_id = *(osal_u16 *)data;
    len_tmp = sizeof(user_id);
    uapsd_flag = *(data + len_tmp);
    len_tmp += sizeof(uapsd_flag);
    uapsd_status = (mac_user_uapsd_status_stru *)(data + len_tmp);

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_uapsd_update::hmac_user[%d] null.}", hmac_vap->vap_id, user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    if (user_uapsd_info == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memcpy_s(&user_uapsd_info->uapsd_flag, sizeof(osal_u8), &uapsd_flag, sizeof(osal_u8));
    (osal_void)memcpy_s(&user_uapsd_info->uapsd_status, sizeof(mac_user_uapsd_status_stru),
        uapsd_status, sizeof(mac_user_uapsd_status_stru));
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_usr_init
 功能描述  : USR初始化时u-apsd的初始化
*****************************************************************************/
OSAL_STATIC osal_bool hmac_uapsd_user_init(hmac_user_stru *hmac_user)
{
    hmac_user_uapsd_info_stru *user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    hmac_user_uapsd_stru       *uapsd_stru = &(user_uapsd_info->uapsd_stru);

    osal_spin_lock_init(&(uapsd_stru->lock_uapsd));
    oal_netbuf_list_head_init(&(uapsd_stru->uapsd_queue_head));
    osal_adapt_atomic_set(&uapsd_stru->mpdu_num, 0);
    uapsd_stru->uapsd_trigseq[WLAN_WME_AC_BK] = HMAC_UAPSD_INVALID_TRIGGER_SEQ;
    uapsd_stru->uapsd_trigseq[WLAN_WME_AC_BE] = HMAC_UAPSD_INVALID_TRIGGER_SEQ;
    uapsd_stru->uapsd_trigseq[WLAN_WME_AC_VI] = HMAC_UAPSD_INVALID_TRIGGER_SEQ;
    uapsd_stru->uapsd_trigseq[WLAN_WME_AC_VO] = HMAC_UAPSD_INVALID_TRIGGER_SEQ;

    return OAL_SUCC;
}

OSAL_STATIC osal_bool hmac_uapsd_ap_user_add(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u16 assoc_id;

    if (hmac_user == OSAL_NULL) {
        return OSAL_FALSE;
    }

    assoc_id = hmac_user->assoc_id;
    if (assoc_id >= WLAN_USER_MAX_USER_LIMIT) {
        return OSAL_FALSE;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_user->vap_id);
    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OSAL_TRUE;
    }

    if (g_user_uapsd_info[assoc_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_uapsd_ap_user_add mem already malloc!", assoc_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_user_uapsd_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_uapsd_ap_user_add malloc null!", assoc_id);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_user_uapsd_info_stru), 0, sizeof(hmac_user_uapsd_info_stru));
    g_user_uapsd_info[assoc_id] = (hmac_user_uapsd_info_stru *)mem_ptr;

    hmac_uapsd_user_init(hmac_user);

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_uapsd_usr_destroy
 功能描述  : USR删除时u-apsd资源的销毁
*****************************************************************************/
OSAL_STATIC osal_void hmac_uapsd_user_destroy(hmac_user_stru *hmac_user)
{
    hmac_user_uapsd_info_stru *user_uapsd_info = hmac_ap_get_user_uapsd_info(hmac_user->assoc_id);
    hmac_user_uapsd_stru *uapsd_stru = &(user_uapsd_info->uapsd_stru);
    oal_netbuf_stru *net_buf = OSAL_NULL;

    /* 释放节能队列中的mpdu */
    osal_spin_lock(&uapsd_stru->lock_uapsd);
    while (osal_adapt_atomic_read(&uapsd_stru->mpdu_num) != 0) {
        net_buf = hmac_tx_dequeue_first_mpdu(&uapsd_stru->uapsd_queue_head);
        if (net_buf == OSAL_NULL) {
            break;
        }
        osal_adapt_atomic_dec(&uapsd_stru->mpdu_num);
        hmac_tx_excp_free_netbuf(net_buf);
    }
    osal_spin_unlock(&uapsd_stru->lock_uapsd);
    osal_spin_lock_destroy(&(uapsd_stru->lock_uapsd));
    osal_adapt_atomic_set(&uapsd_stru->mpdu_num, 0);
    return;
}

OSAL_STATIC osal_bool hmac_uapsd_ap_user_del(osal_void *notify_data)
{
    hmac_user_stru *hmac_user = (hmac_user_stru *)notify_data;
    osal_u16 assoc_id;

    if (hmac_user == OSAL_NULL) {
        return OSAL_FALSE;
    }

    assoc_id = hmac_user->assoc_id;
    if (assoc_id >= WLAN_USER_MAX_USER_LIMIT) {
        return OSAL_FALSE;
    }

    if (g_user_uapsd_info[assoc_id] == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_uapsd_ap_user_del mem already free!", assoc_id);
        return OSAL_TRUE;
    }

    hmac_uapsd_user_destroy(hmac_user);

    oal_mem_free((osal_void *)g_user_uapsd_info[assoc_id], OAL_TRUE);
    g_user_uapsd_info[assoc_id] = OSAL_NULL;

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_uapsd_ap_vap_add(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OSAL_TRUE;
    }

    hmac_vap->cap_flag.uapsd = OSAL_TRUE;

    return OSAL_TRUE;
}

osal_u32 hmac_uapsd_ap_init(osal_void)
{
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_uapsd_ap_vap_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_uapsd_ap_user_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_uapsd_ap_user_del);

    frw_msg_hook_register(WLAN_MSG_D2H_CRX_PS_TX_STATUS, hmac_config_tx_comp_ps_handle);

    hmac_feature_hook_register(HMAC_FHOOK_AP_UAPSD_UPDATE_USER_PARA, hmac_uapsd_update_user_para_etc);
    hmac_feature_hook_register(HMAC_FHOOK_AP_UAPSD_CHECK_TRIGGER, hmac_uapsd_rx_trigger_check);
    hmac_feature_hook_register(HMAC_FHOOK_AP_UAPSD_FLUSH_QUEUE, hmac_uapsd_flush_queue);
    hmac_feature_hook_register(HMAC_FHOOK_AP_UAPSD_ENQUEUE_PROC, hmac_uapsd_enqueue_proc);
    hmac_feature_hook_register(HMAC_FHOOK_AP_UAPSD_IS_QUEUE_EMPTY, hmac_psm_is_uapsd_empty);
    hmac_feature_hook_register(HMAC_FHOOK_AP_UAPSD_GET_USER_INFO, hmac_ap_get_user_uapsd_info);

    return OAL_SUCC;
}

osal_void hmac_uapsd_ap_deinit(osal_void)
{
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_uapsd_ap_vap_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_USER, hmac_uapsd_ap_user_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_USER, hmac_uapsd_ap_user_del);

    frw_msg_hook_unregister(WLAN_MSG_D2H_CRX_PS_TX_STATUS);

    hmac_feature_hook_unregister(HMAC_FHOOK_AP_UAPSD_UPDATE_USER_PARA);
    hmac_feature_hook_unregister(HMAC_FHOOK_AP_UAPSD_CHECK_TRIGGER);
    hmac_feature_hook_unregister(HMAC_FHOOK_AP_UAPSD_FLUSH_QUEUE);
    hmac_feature_hook_unregister(HMAC_FHOOK_AP_UAPSD_ENQUEUE_PROC);
    hmac_feature_hook_unregister(HMAC_FHOOK_AP_UAPSD_IS_QUEUE_EMPTY);
    hmac_feature_hook_unregister(HMAC_FHOOK_AP_UAPSD_GET_USER_INFO);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
