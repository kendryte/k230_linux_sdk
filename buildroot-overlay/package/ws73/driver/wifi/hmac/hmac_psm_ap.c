/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: PSM特性处理.
 * Create: 2020-7-9
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_psm_ap.h"
#include "oal_net.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "mac_vap_ext.h"
#include "mac_device_ext.h"
#include "hmac_vap.h"
#include "hmac_main.h"
#include "hmac_user.h"
#include "hmac_uapsd.h"
#include "hmac_tx_mgmt.h"
#include "hal_ext_if.h"
#include "hmac_blockack.h"
#include "hmac_beacon.h"
#include "hmac_alg_notify.h"
#include "oal_netbuf_data.h"

#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_twt.h"

#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif
#include "hmac_feature_dft.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_PSM_AP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 函数声明
*****************************************************************************/
static oal_netbuf_stru* hmac_psm_dequeue_first_mpdu(hmac_user_ps_stru  *ps_structure);
static osal_void hmac_psm_set_more_data(oal_netbuf_stru *net_buf);
static osal_void hmac_change_null_data_rate(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 *protocol_mode, osal_u8 *legacy_rate);
static osal_void hmac_psm_set_ucast_mgmt_tx_rate(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    osal_u8 legacy_rate, wlan_phy_protocol_enum_uint8 protocol_mode);

/*****************************************************************************
 功能描述  : 从节能队列取出一个缓存帧(一个mpdu)，然后调用相应接口发送出去
*****************************************************************************/
static osal_u32 hmac_psm_queue_send(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret;
    hmac_user_ps_stru *structure = &hmac_user->ps_structure;
    osal_u8 vap_id = hmac_vap->vap_id;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_s32 ps_mpdu_num;
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;
    oal_netbuf_stru *net_buf;

    unref_param(vap_id);
    unref_param(hmac_device);
    net_buf = hmac_psm_dequeue_first_mpdu(structure); /* 从节能队列中取出一个mpdu,并将队列中mpdu数目减1 */
    if (net_buf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_psm_queue_send::net_buf null.}", vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断节能队列中是否还有缓存帧，如果有，设置帧头里的MORE DATA位为1，如果队列
       已空，并且此次发送的是单播缓存帧，则关闭tim_bitmap的相应位 */
    ps_mpdu_num = osal_adapt_atomic_read(&structure->mpdu_num);
    if (is_ap(hmac_vap)) {
        if (ps_mpdu_num != 0) {
            hmac_psm_set_more_data(net_buf);
        } else if (hmac_user->is_multi_user != OSAL_TRUE) { /* 广播在dtim count为0的beacon组帧完成后置0 */
            hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);
        }
    }

    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    /* 组播数据的判断方式应该采用以下形式 */
    /* 将节能缓存帧设为节能AC，从组播队列发送 */
    tx_ctrl->ac = (tx_ctrl->ismcast == OSAL_TRUE) ? WLAN_WME_AC_PSM : tx_ctrl->ac;

    /* 置这一位的原因是:接下来要调用dmac_tx_process_data，而进入这个函数之后会判断
       是否需要将mpdu入队，如果没有下面的这个判断，会导致循环入队，这个mpdu在pspoll
       节能机制下将永远发不出去 */
    tx_ctrl->is_get_from_ps_queue = OSAL_TRUE;

    /* 根据帧的类型，调用相应发送接口 */
    if (mac_get_wlan_frame_type(tx_ctrl) != WLAN_DATA_BASICTYPE) {
        goto hmac_tx_mgmt;
    }

    /* 防止数据发送出去后，BA窗没有移动，造成后续收到数据出现窗错误 */
    if ((hmac_user->is_multi_user == OSAL_FALSE) &&
        (hmac_user_get_ps_mode(hmac_user) == OSAL_TRUE)) {
        hmac_tid_resume(&hmac_user->tx_tid_queue[tx_ctrl->tid], DMAC_TID_PAUSE_RESUME_TYPE_PS);
    }

    /* aput keepalive, 发送null data 帧调用dmac_tx_mgmt 函数 */
    if (mac_get_wlan_frame_subtype(tx_ctrl) == WLAN_NULL_FRAME) {
        goto hmac_tx_mgmt;
    }

    ret = hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, net_buf);
    if (ret != OAL_SUCC) {
        oam_info_log3(0, OAM_SF_PWR, "vap_id[%d] {hmac_psm_queue_send::tx_process_data failed[%d],dev_mpdu_num = %d}",
            vap_id, ret, hmac_device->total_mpdu_num);
        oam_info_log4(0, OAM_SF_PWR, "{hmac_psm_queue_send:: be = %d, bk = %d, vi = %d, vo = %d}",
            hmac_device->aus_ac_mpdu_num[WLAN_WME_AC_BE], hmac_device->aus_ac_mpdu_num[WLAN_WME_AC_BK],
            hmac_device->aus_ac_mpdu_num[WLAN_WME_AC_VI], hmac_device->aus_ac_mpdu_num[WLAN_WME_AC_VO]);
        hmac_tx_excp_free_netbuf(net_buf);
    }
    return ret;

hmac_tx_mgmt:
    ret = hmac_tx_mgmt(hmac_vap, net_buf, tx_ctrl->mpdu_payload_len + tx_ctrl->frame_header_length, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(net_buf);
        oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_psm_queue_send::hmac_tx_mgmt failed[%d].}",
            vap_id, ret);
    }
    return ret;
}

/*****************************************************************************
 功能描述  : ap收到用户改变节能模式为非节能的帧，
                             将为用户缓存的所有帧都发给用户
*****************************************************************************/
osal_void hmac_psm_queue_flush(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_s32 ps_mpdu_num, ps_mpdu_send_succ, ps_mpdu_send_fail;
    osal_u32 tid_mpdu_num, ret;
    ps_mpdu_send_succ = ps_mpdu_send_fail = 0;

    ps_mpdu_num = osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num);
    if (ps_mpdu_num == 0) {
        /* tid和ps队列都没有包清pvb */
        if (is_ap(hmac_vap)) {
            tid_mpdu_num = hmac_psm_tid_mpdu_num(hmac_user);
            if (tid_mpdu_num == 0) {
                hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);
            } else {
                oam_warning_log4(0, OAM_SF_PWR,
                    "vap_id[%d] hmac_psm_queue_flush:: user[%d] has ps[%d] & Total mpdus[%d] in TID",
                    hmac_vap->vap_id, hmac_user->assoc_id, ps_mpdu_num, tid_mpdu_num);
            }
        }
        return;
    }

    /*  背景扫描或者DBAC切信道后，VAP处于PAUSE状态，节能队列里的数据帧
        如果是广播帧，会进高优先级硬件队列发错信道
        如果是单播数据帧，也会被重新进TID队列
        需要在VAP PAUSE的时候挡掉节能队列里数据帧的发送 */
    if (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) {
        oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] hmac_psm_queue_flush:vap pause, userid[%d]",
            hmac_vap->vap_id, hmac_user->assoc_id);
        return;
    }

    /* 循环将节能队列中的所有帧发给用户 */
    while (ps_mpdu_num-- > 0) {
        ret = hmac_psm_queue_send(hmac_vap, hmac_user);
        if (ret != OAL_SUCC) {
            oam_info_log1(0, OAM_SF_PWR, "{hmac_psm_queue_flush::hmac_psm_queue_send fail[%d].}", ret);
            ps_mpdu_send_fail++;
        } else {
            ps_mpdu_send_succ++;
        }
    }

    oam_info_log3(0, OAM_SF_PWR, "{hmac_psm_queue_flush::user[%d] send %d & fail %d}",
        hmac_user->assoc_id, ps_mpdu_send_succ, ps_mpdu_send_fail);

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    /* 用户处于WUR，单播/组播缓存数据需要清除标记 */
    if (is_ap(hmac_vap)) {
        if ((hmac_wur_get_user_status(hmac_user) >= HMAC_USER_WUR_MODE_ON) &&
            (hmac_user->wur_info.unicast_buffered == OSAL_TRUE)) {
            hmac_user->wur_info.unicast_buffered = OSAL_FALSE;
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_psm_queue_flush::wur unicast flush}");
        } else if (hmac_user->wur_info.multi_buffered == OSAL_TRUE) {
            hmac_user->wur_info.multi_buffered = OSAL_FALSE;
            oam_warning_log0(0, OAM_SF_11AX, "{hmac_psm_queue_flush::wur multicast flush}");
        }
    }
#endif
}

/*****************************************************************************
 函 数 名  : hmac_psm_reset
 功能描述  : 复位一个用户的节能状态
             1) 用户状态置为不节能
             2) 清空用户节能队列
*****************************************************************************/
osal_s32 hmac_psm_reset(hmac_vap_stru *vap, osal_u16 user_id)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;
    hmac_vap_stru *hmac_vap = vap;
    osal_s32 ret;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));
    cfg_msg_init((osal_u8 *)&user_id, OAL_SIZEOF(user_id), OAL_PTR_NULL, 0, &msg_info);

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_psm_reset::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_id);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_psm_reset::hmac_user[%d] null.", hmac_vap->vap_id, user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log1(0, OAM_SF_PWR, "{hmac_psm_reset::hmac_user->ps_mode is %u.}", hmac_user->ps_mode);
    hal_tx_disable_peer_sta_ps_ctrl(hmac_user->lut_index);

    /* snyc ps mode to device */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_RESET_AP_PSM, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_PWR,
            "{hmac_config_set_sta_ps_mode::frw_send_msg_to_device failed[%d].}", ret);
    }

    if (hmac_user->ps_mode == OSAL_FALSE) {
        /* 用户是非节能的，直接返回成功 */
        return OAL_SUCC;
    }

    hmac_user->ps_mode = OSAL_FALSE;

    /* 修改用户在节能情况下，不去关联直接重新认证、关联导致节能状态未清除 */
    hmac_vap->ps_user_num = oal_sub(hmac_vap->ps_user_num, 1); /* 节能用户个数清除 */
    hmac_user_resume(hmac_user);                               /* 恢复user，恢复该user的每一个tid */
    hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);         /* 该修改体现在beacon中的信息 */

    hmac_psm_queue_flush(hmac_vap, hmac_user);

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_AP_UAPSD_FLUSH_QUEUE);
    if (fhook != OSAL_NULL && ((hmac_uapsd_flush_queue_cb)fhook)(hmac_vap, hmac_user) < 0) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_psm_reset::uapsd_flush_queue:: return value is -1.}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_awake
 功能描述  : user从节能状态转化为非节能状态时ap的相关处理
*****************************************************************************/
static osal_u32 hmac_psm_awake(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_user->ps_mode = OSAL_FALSE;
    hmac_user->ps_structure.ps_time_count = 0;
    hmac_vap->ps_user_num--;

    /* 恢复user，恢复该user的每一个tid */
    hmac_user_resume(hmac_user);

    /* 将所有的缓存帧发送出去 */
    hmac_psm_queue_flush(hmac_vap, hmac_user);

    /* 发送完节能队列里的缓存帧后，设置了本地bitmap，及时更新beacon的tim信息元素 */
    if (hmac_vap->beacon_buffer != OSAL_NULL) {
        hmac_encap_beacon(hmac_vap, oal_netbuf_header(hmac_vap->beacon_buffer));
    }

    oam_info_log2(0, OAM_SF_PWR, "vap_id[%d] {hmac_psm_awake::user[%d] is active.}", hmac_user->vap_id,
        hmac_user->assoc_id);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_doze
 功能描述  : 用户进入节能状态，ap做相应的一些处理
*****************************************************************************/
osal_u32 hmac_psm_doze(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_user->ps_mode = OSAL_TRUE;
    hmac_user->ps_structure.ps_time_count = 0;
    hmac_vap->ps_user_num++;

    /* pause该用户,但是不能pause uapsd专用tid */
    hmac_user_pause(hmac_user);
    hmac_tid_resume(&hmac_user->tx_tid_queue[WLAN_TIDNO_UAPSD], DMAC_TID_PAUSE_RESUME_TYPE_PS);

    /* 如果psm/tid不为空，则设置PVB */
    if ((hmac_psm_is_psm_empty(hmac_user) == OSAL_FALSE) || (hmac_psm_is_tid_empty(hmac_user) == OSAL_FALSE)) {
        hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 1);
        oam_info_log2(0, OAM_SF_PWR, "{hmac_psm_doze::user[%d].%d mpdu in tid.}", hmac_user->assoc_id,
            osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num));
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_rx_process_data
 功能描述  : AP接收到数据帧后，要对帧头的节能位进行检查，修改相应的用户节能
             模式，并决定是否需要发送缓存帧
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_psm_rx_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *net_buf)
{
    mac_ieee80211_frame_stru *mac_header = OSAL_NULL;
    mac_rx_ctl_stru *rx_ctrl = OSAL_NULL;
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    osal_u32 ret;
#endif

    if (osal_unlikely(hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL || net_buf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_psm_rx_process_data::param is null.}.");
        return;
    }

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    ret = hmac_wur_psm_rx_process(hmac_vap, hmac_user, net_buf);
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_psm_rx_process:wur_psm_rx_process return [%d]", ret);
        return;
    }
#endif

    rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(net_buf);
    mac_header = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(rx_ctrl);
    /*
       如果节能位开启(power_mgmt == 1),同时之前处于非节能模式，则修改节能
       模式为节能,否则什么都不做，因为用户仍在节能，ap继续为其缓存帧就可以了;
       如果节能位关闭(power_mgmt == 0),同时之前处于节能模式，则修改节能模
       式为非节能，并且将相应节能队列中的所有缓存帧都发给该用户，否则，什么都
       不做，用户一直都是非节能的，ap与用户之间正常收发数据
    */
    if ((mac_header->frame_control.power_mgmt == OSAL_TRUE) && (hmac_user->ps_mode == OSAL_FALSE)) {
        hmac_psm_doze(hmac_vap, hmac_user);
    }
    if ((mac_header->frame_control.power_mgmt == OSAL_FALSE) && (hmac_user->ps_mode == OSAL_TRUE)) {
        hmac_psm_awake(hmac_vap, hmac_user);
    } else if ((mac_header->frame_control.power_mgmt == OSAL_FALSE) && (hmac_user->ps_mode == OSAL_FALSE)) {
        /* 用户之前是非节能的，但是可能由于VAP PAUSE导致节能队列帧未及时发送出去 */
        if (osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num) > 0) {
            oam_warning_log1(0, OAM_SF_PWR, "vap_id[%d] hmac_psm_rx_process:force flush psm queue", hmac_vap->vap_id);
            /* 将所有的缓存帧发送出去 */
            hmac_psm_queue_flush(hmac_vap, hmac_user);
            /* 发送完节能队列里的缓存帧后，设置了本地bitmap，及时更新beacon的tim信息元素 */
            if (hmac_vap->beacon_buffer != OSAL_NULL) {
                hmac_encap_beacon(hmac_vap, oal_netbuf_header(hmac_vap->beacon_buffer));
            }
        }
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        if (hmac_wur_get_user_status(hmac_user) == HMAC_USER_WUR_MODE_SUSPEND) {
            hmac_user->wur_info.wur_ps_enqueue = OSAL_FALSE; /* 用户切回主收发，取消缓存，后续沿用fast-ps过程 */
        }
#endif
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P GO关连设备停止节能，P2P OppPS暂停 */
    if ((mac_header->frame_control.power_mgmt == OSAL_FALSE) && (is_p2p_go(hmac_vap)) &&
        (is_p2p_oppps_enabled(hmac_vap))) {
        hmac_vap->p2p_ops_param.pause_ops = OSAL_TRUE;
    }
    /* P2P GO关连设备使能节能，允许P2P OppPS */
    if ((mac_header->frame_control.power_mgmt == OSAL_TRUE) && (is_p2p_go(hmac_vap)) &&
        (is_p2p_oppps_enabled(hmac_vap))) {
        hmac_vap->p2p_ops_param.pause_ops = OSAL_FALSE;
    }
#endif
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_rx_data_ps_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf)
{
    osal_void *fhook = OSAL_NULL;

    if (hmac_vap_get_bss_type(hmac_vap) == WLAN_VAP_MODE_BSS_AP) {
        /* psm节能处理 */
        hmac_psm_rx_process(hmac_vap, hmac_user, netbuf);

        /* uapsd节能特性 */
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_AP_UAPSD_CHECK_TRIGGER);
        if (fhook != OSAL_NULL) {
            ((hmac_uapsd_rx_trigger_check_cb)fhook)(hmac_vap, hmac_user, netbuf);
        }
    }

    return OAL_SUCC;
}

static osal_void hmac_psm_fill_null_data(oal_netbuf_stru *net_buf, hmac_vap_stru *hmac_vap,
    hmac_user_stru *hmac_user, const oal_bool_enum_uint8 ps, oal_bool_enum_uint8 is_from_ps_queue)
{
    osal_u16 tx_direction;
    mac_ieee80211_frame_stru *mac_header = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;

    oal_set_netbuf_prev(net_buf, OSAL_NULL);
    oal_set_netbuf_next(net_buf, OSAL_NULL);

    /* 将mac header清零 */
    mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(net_buf);
    (osal_void)memset_s((osal_u8 *)mac_header, sizeof(mac_ieee80211_frame_stru), 0, sizeof(mac_ieee80211_frame_stru));

    /* null帧发送方向From AP || To AP */
    tx_direction = (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) ? WLAN_FRAME_FROM_AP : WLAN_FRAME_TO_AP;
    /* 填写帧头,其中from ds为1，to ds为0，因此frame control的第二个字节为02 */
    mac_null_data_encap((osal_u8 *)mac_header,
        ((osal_u16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_NODATA) | tx_direction),
        hmac_user->user_mac_addr, mac_mib_get_station_id(hmac_vap));

    /*  NB: power management bit is never sent by an AP */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_header->frame_control.power_mgmt = ps;
    }

    /* 填写cb字段 */
    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    /* 填写tx部分 */
    tx_ctrl->ack_policy = WLAN_TX_NORMAL_ACK;
    tx_ctrl->retried_num = 0;
    tx_ctrl->tid = WLAN_TID_FOR_DATA;
    tx_ctrl->tx_vap_index = hmac_vap->vap_id;
    tx_ctrl->tx_user_idx = (osal_u8)hmac_user->assoc_id;

    /* aput keepalive, null data tx cb is_get_from_ps_queue 赋值为FALSE */
    if (is_ap(hmac_vap) && is_from_ps_queue == OSAL_FALSE) {
        tx_ctrl->is_get_from_ps_queue = OSAL_FALSE; /* AP 发送null 帧放入节能队列 */
    } else {
        tx_ctrl->is_get_from_ps_queue = OSAL_TRUE;
    }
    tx_ctrl->frame_header_length = sizeof(mac_ieee80211_frame_stru);
    tx_ctrl->mpdu_num = 1;
    tx_ctrl->netbuf_num = 1;
    tx_ctrl->mpdu_payload_len = 0;
    tx_ctrl->ac = WLAN_WME_AC_MGMT;
    oal_netbuf_put(net_buf, tx_ctrl->frame_header_length);
}

/*****************************************************************************
 功能描述  : 1.ap收到ps-poll，节能队列为空，ap向sta发送Null Data ,2.ap定时KeepAlive流程
*****************************************************************************/
osal_u32 hmac_psm_send_null_data(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_bool_enum_uint8 ps, oal_bool_enum_uint8 is_from_ps_queue)
{
    oal_netbuf_stru *net_buf = OSAL_NULL;
    osal_u32 ret;
    osal_u8 legacy_rate;
    wlan_phy_protocol_enum_uint8 protocol_mode;
    osal_u8 null_protocol_mode = 0;
    osal_u8 null_legacy_rate = 0;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* begin: 解决null 帧发错信道，修改为扫描状态不发送null 帧 */
    if ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
        (hmac_vap->vap_state == MAC_VAP_STATE_PAUSE)) {
        oam_warning_log2(0, OAM_SF_PWR,
            "vap_id[%d] hmac_psm_send_null_data: scaning and vap status is pause, do not send null!vap_mode[%d]",
            hmac_vap->vap_id, hmac_vap->vap_mode);
        return OAL_FAIL;
    }

    /* end: 解决null 帧发错信道，修改为扫描状态不发送null 帧 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (hmac_get_p2p_mode_etc(hmac_vap) == WLAN_P2P_DEV_MODE) {
        return OAL_FAIL;
    }
#endif

    /* 申请net_buff */
    net_buf = oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (net_buf == OSAL_NULL) {
        /* staut要保证null内存可以申请到,error级别 */
        /* BT和WLAN共存进行iperf冲包，1)Beacon帧中携带PVB不正常
        2)STAUT醒来的情况下Beacon中携带PVB置为1，发送nulldata失败，分析无影响，将日志降级 */
        oam_warning_log1(0, OAM_SF_PWR, "vap_id[%d] {hmac_psm_send_null_data::alloc netbuf fail.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    hmac_psm_fill_null_data(net_buf, hmac_vap, hmac_user, ps, is_from_ps_queue);

    /* 2.4G初始化为11b 1M, long preable, 发送null帧时修改为11g/a的速率 */
    legacy_rate =
        hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_ucast[WLAN_BAND_2G]))[0].nss_rate.legacy_rate.legacy_rate;
    protocol_mode = hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_ucast[WLAN_BAND_2G]))[0].protocol_mode;

    hmac_change_null_data_rate(hmac_vap, hmac_user, &null_protocol_mode, &null_legacy_rate);

    hmac_psm_set_ucast_mgmt_tx_rate(hmac_vap, WLAN_BAND_2G, null_legacy_rate, null_protocol_mode);

    ret = hmac_tx_mgmt(hmac_vap, net_buf, sizeof(mac_ieee80211_frame_stru), OSAL_FALSE);

    /* 发送完成后恢复默认值: 2.4G初始化为11b 1M, long preable */
    hmac_psm_set_ucast_mgmt_tx_rate(hmac_vap, WLAN_BAND_2G, legacy_rate, protocol_mode);

    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_PWR,
            "vap_id[%d] {hmac_psm_send_null_data::hmac_tx_mgmt failed[%d].}", hmac_vap->vap_id, ret);

        hmac_tx_excp_free_netbuf(net_buf);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_sch_psm_queue
 功能描述  : 收到sta的pspoll，tid为空，从节能队列调度一个包出来发送
*****************************************************************************/
static osal_u32 hmac_psm_sch_psm_queue(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_s32 ps_mpdu_num;
    osal_u32 ret;

    ps_mpdu_num = osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num);
    if (ps_mpdu_num > 0) {
        ret = hmac_psm_queue_send(hmac_vap, hmac_user);
        if (ret != OAL_SUCC) {
            oam_warning_log3(0, OAM_SF_PWR, "{hmac_psm_sch_psm_queue::user[%d] send fail[%d] & %d left.}",
                hmac_user->assoc_id, ret, ps_mpdu_num - 1);
            return ret;
        }
        oam_info_log2(0, OAM_SF_PWR, "{hmac_psm_sch_psm_queue::user[%d] send 1 & %d left}",
            hmac_user->assoc_id, ps_mpdu_num - 1);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_sch_tid_queue
 功能描述  : 收到sta发的pspoll，tid里面有数据，调度一个发出去，
             如果tid中还有其他数据或者psm队列不为空。设置moredata标志
*****************************************************************************/
static osal_u32 hmac_psm_sch_tid_queue(hmac_user_stru *hmac_user, osal_u32 ps_mpdu_num)
{
    oal_netbuf_stru *mgmt_buf = OSAL_NULL;
    osal_u8 tid_idx;
    hmac_tid_stru *txtid = OSAL_NULL;

    for (tid_idx = 0; tid_idx < WLAN_TID_MAX_NUM; tid_idx++) {
        osal_u32 irq_status;
        txtid = &hmac_user->tx_tid_queue[tid_idx];
        irq_status = frw_osal_irq_lock();
        if (oal_netbuf_list_empty(&txtid->buff_head) == OSAL_TRUE) {
            frw_osal_irq_restore(irq_status);
            continue;
        }

        for (mgmt_buf = txtid->buff_head.next;
            mgmt_buf != (oal_netbuf_stru *)(&txtid->buff_head); mgmt_buf = mgmt_buf->next) {
            /* 节能队列非空或该用户TID队列还有其他mpdu，需要设置moredata */
            if (ps_mpdu_num > 1) {
                hmac_psm_set_more_data(mgmt_buf);
            }

            /* 发送的是该用户任意非空TID队列中的第一个 */
            hmac_tid_resume(txtid, DMAC_TID_PAUSE_RESUME_TYPE_PS);
            frw_osal_irq_restore(irq_status);
            return OAL_SUCC;
        }
        frw_osal_irq_restore(irq_status);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_handle_pspoll
 功能描述  : ap收到sta的pspoll，缓存不为空，回复一个缓存帧
*****************************************************************************/
static osal_u32 hmac_psm_handle_pspoll(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 *extra_qosnull)
{
    osal_u32 ret = OAL_SUCC;
    osal_u32 tid_mpud_num;

    /* 如果tid不为空 */
    tid_mpud_num = hmac_psm_tid_mpdu_num(hmac_user);
    if (tid_mpud_num != 0) {
        tid_mpud_num += (osal_u32)osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num);
        ret = hmac_psm_sch_tid_queue(hmac_user, tid_mpud_num);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_PWR, "{hmac_psm_handle_pspoll::hmac_psm_sch_tid_queue fail[%d].}", ret);
            return ret;
        }
        if (tid_mpud_num == 1) {
            hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);
        }
    } else if (osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num) != 0) { /* 如果psm节能不为空 */
        ret = hmac_psm_sch_psm_queue(hmac_vap, hmac_user);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_PWR, "{hmac_psm_handle_pspoll::hmac_psm_sch_psm_queue fail[%d].}", ret);
            return ret;
        }
        if (osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num) == 0) {
            hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);
        }
    } else {
        *extra_qosnull = 1;
        return OAL_SUCC;
    }

    return ret;
}

/*****************************************************************************
 功能描述  : ap收到sta发的ps-poll，从相应的节能队列中取出一个缓存帧（mpdu）
             发给sta，如果节能队列为空，则发送一个Null Data给sta。并设置相应
             的more data位和tim_bitmap
*****************************************************************************/
osal_u32 hmac_psm_resv_ps_poll(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32 ret;
    osal_u8 extra_qosnull = 0;

    /* 检查vap模式 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(0, 0, "{hmac_psm_resv_ps_poll::vap[%d] is not in ap mode.}", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    ret = hmac_psm_handle_pspoll(hmac_vap, hmac_user, &extra_qosnull);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_PWR,
            "vap_id[%d] {hmac_psm_resv_ps_poll::hand_pspoll return [%d].}", hmac_vap->vap_id, ret);
        return ret;
    }

    if (extra_qosnull != 0) {
        hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 0);
        ret = hmac_psm_send_null_data(hmac_vap, hmac_user, OSAL_FALSE, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_PWR,
                "vap_id[%d] {hmac_psm_resv_ps_poll::send_null fail[%d].}", hmac_vap->vap_id, ret);
        }
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 从队列中取出第一个mpdu,将它从队列删除并返回，一个mpdu中可能有多个skb
*****************************************************************************/
static oal_netbuf_stru *hmac_psm_dequeue_first_mpdu(hmac_user_ps_stru *ps_structure)
{
    oal_netbuf_stru *first_net_buf;
    oal_netbuf_stru *tmp_net_buf = OSAL_NULL;
    oal_netbuf_stru *net_buf = OSAL_NULL;
    mac_tx_ctl_stru *tx_ctrl;
    osal_u8 netbuf_num_per_mpdu;
    oal_netbuf_head_stru *ps_queue_head;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    ps_queue_head = &ps_structure->ps_queue_head;

    first_net_buf = ps_queue_head->next;
    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(first_net_buf);
    /* mpdu不是a-msdu，返回第一个net_buff即可 */
    if (tx_ctrl->is_amsdu == OSAL_FALSE) {
        first_net_buf = oal_netbuf_delist(ps_queue_head);
        osal_adapt_atomic_dec(&ps_structure->mpdu_num);
        hmac_device->psm_mpdu_num -= 1;
        return first_net_buf;
    }

    if (tx_ctrl->is_first_msdu == OSAL_FALSE) {
        /* 这个错误只有踩内存才会出现，如果出现就无法恢复，以太网来的包无法释放，
           软件复位也没用，内存也会泄漏
        */
        oam_error_log0(0, OAM_SF_PWR, "{hmac_dequeue_first_mpdu::not the first msdu.}");
        return OSAL_NULL;
    }

    /* 节能队列中的第一个mpdu是a-msdu，获取skb个数 */
    netbuf_num_per_mpdu = tx_ctrl->netbuf_num;

    /* 将第一个mpdu中的所有skb从节能队列中取出，然后组成一个net_buff链 */
    first_net_buf = oal_netbuf_delist(ps_queue_head);
    netbuf_num_per_mpdu--;

    tmp_net_buf = first_net_buf;
    while (netbuf_num_per_mpdu != 0) {
        net_buf = oal_netbuf_delist(ps_queue_head);
        oal_set_netbuf_prev(net_buf, tmp_net_buf);
        oal_set_netbuf_next(net_buf, OSAL_NULL);

        oal_set_netbuf_next(tmp_net_buf, net_buf);

        tmp_net_buf = net_buf;

        netbuf_num_per_mpdu--;
    }

    osal_adapt_atomic_dec(&ps_structure->mpdu_num);
    hmac_device->psm_mpdu_num -= 1;

    return first_net_buf;
}

/*****************************************************************************
 函 数 名  : hmac_psm_delete_ps_queue_head
 功能描述  : 删除节能队列中的n个mpdu
*****************************************************************************/
osal_void hmac_psm_delete_ps_queue_head(hmac_user_stru *hmac_user, osal_u32 psm_delete_num)
{
    hmac_user_ps_stru *ps_structure;
    oal_netbuf_stru *net_buf = OSAL_NULL;
    osal_u32 delete_num = psm_delete_num;

    ps_structure = &(hmac_user->ps_structure);
    osal_spin_lock(&ps_structure->lock_ps);
    /* 节能队列不为空的情况下，释放节能队列中的mpdu */
    while ((osal_adapt_atomic_read(&ps_structure->mpdu_num) != 0) && (delete_num != 0)) {
        /* 由于节能队列中的mpdu可能来自wlan也可能来自lan，在释放的时候需要区分，
           因此不能将队列中的mpdu一次性释放，而是应该以mpdu为单位进行释放
        */
        delete_num--;
        net_buf = hmac_psm_dequeue_first_mpdu(ps_structure);
        hmac_tx_excp_free_netbuf(net_buf);

        osal_spin_unlock(&ps_structure->lock_ps);
        osal_spin_lock(&ps_structure->lock_ps);
    }
    osal_spin_unlock(&ps_structure->lock_ps);
}
/*****************************************************************************
 函 数 名  : hmac_psm_clear_ps_queue
 功能描述  : 清空节能队列的数据
*****************************************************************************/
osal_void hmac_psm_clear_ps_queue(hmac_user_stru *hmac_user)
{
    hmac_user_ps_stru *ps_structure;
    oal_netbuf_stru *net_buf = OSAL_NULL;

    ps_structure = &(hmac_user->ps_structure);

    /* 节能队列不为空的情况下，释放节能队列中的mpdu */
    while (osal_adapt_atomic_read(&ps_structure->mpdu_num) != 0) {
        osal_spin_lock(&ps_structure->lock_ps);

        /* 由于节能队列中的mpdu可能来自wlan也可能来自lan，在释放的时候需要区分，
           因此不能将队列中的mpdu一次性释放，而是应该以mpdu为单位进行释放
        */
        net_buf = hmac_psm_dequeue_first_mpdu(ps_structure);
        hmac_tx_excp_free_netbuf(net_buf);

        osal_spin_unlock(&ps_structure->lock_ps);
    }
}

/*****************************************************************************
 功能描述  : 删除用户前，将用户结构体下的节能结构释放掉
*****************************************************************************/
osal_void hmac_psm_user_ps_structure_destroy(hmac_user_stru *hmac_user)
{
    hmac_user_ps_stru *ps_structure = OSAL_NULL;
    oal_netbuf_stru *net_buf = OSAL_NULL;

    ps_structure = &(hmac_user->ps_structure);

    /* 节能队列不为空的情况下，释放节能队列中的mpdu */
    while (osal_adapt_atomic_read(&ps_structure->mpdu_num) != 0) {
        osal_spin_lock(&ps_structure->lock_ps);

        /* 由于节能队列中的mpdu可能来自wlan也可能来自lan，在释放的时候需要区分，
           因此不能将队列中的mpdu一次性释放，而是应该以mpdu为单位进行释放
        */
        net_buf = hmac_psm_dequeue_first_mpdu(ps_structure);
        hmac_tx_excp_free_netbuf(net_buf);

        osal_spin_unlock(&ps_structure->lock_ps);
    }
    osal_spin_lock_destroy(&(ps_structure->lock_ps));
    osal_adapt_atomic_set(&ps_structure->mpdu_num, 0);
}

/*****************************************************************************
 函 数 名  : hmac_psm_set_more_data
 功能描述  : 如果节能队列中还有缓存帧，则设置帧头的more data位为1
*****************************************************************************/
static osal_void hmac_psm_set_more_data(oal_netbuf_stru *net_buf)
{
    mac_tx_ctl_stru     *tx_ctrl;
    mac_ieee80211_frame_stru    *frame_hdr;

    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    frame_hdr = mac_get_frame_header_addr(tx_ctrl);
    frame_hdr->frame_control.more_data = 0x01;
}

/*****************************************************************************
 函 数 名  : hmac_psm_set_local_bitmap
 功能描述  : 设置tim_bitmap中的相应位
*****************************************************************************/
osal_void hmac_psm_set_local_bitmap(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user, osal_u8 bitmap_flg)
{
    osal_u8 tim_byte_idx, tim_bit_mask, tim_offset, pv_bitmap_len, tim_min_offset, tim_max_offset, index;

    if ((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL) || (hmac_vap->tim_bitmap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_psm_set_local_bitmap::param null.}");
        return;
    }
    /***************************************************************************
     ---------------------------------------------------------------------------
     |TIM bitmap len |Bitmap Control            |Partial Virtual Bitmap|
     ---------------------------------------------------------------------------
     |1              |1 (bit1-7=offset bit0=BMC)|1~251                 |
     ---------------------------------------------------------------------------
    ***************************************************************************/
    /* the Partial Virtual Bitmap field consists of octets numbered N1 to N2
       of the traffic indication virtual bitmap, where N1 is the largest even
       number such that bits numbered 1 to (N1 * 8) - 1 in the bitmap are all
       0 and N2 is the smallest number such that bits numbered (N2 + 1) * 8 to
       2007 in the bitmap are all 0. In this  case, the Bitmap Offset subfield
       value contains the number N1/2 */
    pv_bitmap_len = hmac_vap->tim_bitmap[0];
    tim_offset = 2 + (hmac_vap->tim_bitmap[1] & (osal_u8)(~BIT0)); /* 2表示加上头的长度 */
    /* 长度校验 */
    if (pv_bitmap_len + tim_offset > hmac_vap->tim_bitmap_len) {
        oam_error_log3(0, OAM_SF_PWR, "{hmac_psm_set_local_bitmap::tim_offset[%d] + len[%d] >= bitmap_len[%d].}",
            tim_offset, pv_bitmap_len, hmac_vap->tim_bitmap_len);
        (osal_void)memset_s(hmac_vap->tim_bitmap, hmac_vap->tim_bitmap_len, 0, hmac_vap->tim_bitmap_len);
        /* TIM bitmap len is default 1 */
        hmac_vap->tim_bitmap[0] = 1;
        pv_bitmap_len = 1;
    }

    /* 如果是组播用户，修改Bitmap Control的bit0，否则根据用户关联id找到用户在Partial Virtual Bitmap中的字节位置与比特位置 */
    tim_byte_idx = (hmac_user->is_multi_user == OSAL_TRUE) ? 1 :
        (2 + (osal_u8)((hmac_user->assoc_id) >> 3)); /* 2，3用于找到字节位置 */
    tim_bit_mask = (hmac_user->is_multi_user == OSAL_TRUE) ? (osal_u8)(BIT0) :
        (osal_u8)(BIT0 << ((hmac_user->assoc_id) & 0x07));

    if (tim_byte_idx >= hmac_vap->tim_bitmap_len) {
        oam_error_log3(0, OAM_SF_PWR, "{hmac_psm_set_local_bitmap::usr[%d] tim_byte_idx[%d] >= bitmap_len[%d].}",
            hmac_user->assoc_id, tim_byte_idx, hmac_vap->tim_bitmap_len);
        return;
    }

    /* 修改相应的bit的值 */
    hmac_vap->tim_bitmap[tim_byte_idx] = (bitmap_flg == 0) ? (hmac_vap->tim_bitmap[tim_byte_idx] &
        (osal_u8)(~tim_bit_mask)) : (hmac_vap->tim_bitmap[tim_byte_idx] | (osal_u8)tim_bit_mask);

    /* 若是组播，则不涉及修改dtim */
    if (tim_byte_idx == 1) {
        return;
    }
    tim_min_offset = osal_min(tim_byte_idx, tim_offset);
    tim_max_offset = osal_max(tim_byte_idx, tim_offset + pv_bitmap_len - 1);
    /* 2表示赋初始值，找到最小的非0的作为tim_offset(必须是偶数)，为了减少循环次数，只比较本次修改所涉及的字节 */
    tim_offset = 2;
    for (index = tim_min_offset; index <= tim_max_offset; index++) {
        if (hmac_vap->tim_bitmap[index] != 0) {
            tim_offset = index & (~1);
            break;
        }
    }

    /* 找到最大的非0的用来计算PVBitmap_len */
    for (index = tim_max_offset; index > tim_offset; index--) {
        if (hmac_vap->tim_bitmap[index] != 0) {
            break;
        }
    }
    /* 更新PVBitmap_len及Bitmap Control(bit1-bit7表示offset；bit0表示BMC */
    hmac_vap->tim_bitmap[0] = (index - tim_offset) + 1;
    hmac_vap->tim_bitmap[1] &= (osal_u8)(BIT0);
    hmac_vap->tim_bitmap[1] += tim_offset - 2; /* 2表示还要对tim_offset减2 */
}

/*****************************************************************************
 功能描述  : 初始化user的节能结构
*****************************************************************************/
osal_void hmac_psm_user_ps_structure_init(hmac_user_stru *hmac_user)
{
    hmac_user_ps_stru *ps_structure;
    ps_structure = &(hmac_user->ps_structure);

    osal_spin_lock_init(&(ps_structure->lock_ps));
    oal_netbuf_list_head_init(&(ps_structure->ps_queue_head));
    osal_adapt_atomic_set(&ps_structure->mpdu_num, 0);

    ps_structure->ps_time_count = 0;
}

WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_psm_ucast_need_buff(const hmac_device_stru *hmac_device,
    const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf, osal_u32 *res)
{
    /* 如果是单播帧，则有两种情况下该帧不需要缓存:
        1、用户不节能
        2、不处于noa节能状态 */
    if (is_ap(hmac_vap)) {
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        if ((hmac_wur_get_user_status(hmac_user) >= HMAC_USER_WUR_MODE_ON) &&
            hmac_wur_is_ps_enqueue(hmac_user) == OSAL_TRUE) {
            *res = OSAL_TRUE; /* 用户建立WUR，需要缓存 */
            return OSAL_TRUE;
        }
#endif

        if ((hmac_user->ps_mode == OSAL_TRUE) && mac_is_ftm_related_frame(netbuf) == OSAL_TRUE) {
            return OSAL_TRUE;
        }

        if ((hmac_user->ps_mode == OSAL_FALSE) && (hmac_device->st_p2p_info.p2p_ps_pause == OSAL_FALSE)) {
            *res = OSAL_FALSE;
            return OSAL_TRUE;
        }
    } else if (is_p2p_cl(hmac_vap) && (hmac_device->st_p2p_info.p2p_ps_pause == OSAL_FALSE)) {
        *res = OSAL_FALSE;
        return OSAL_TRUE;
    }

    /* 如果是AP 发送的keepalive null 帧，则直接放入节能队列 */
    /* 首先必须满足用户睡眠了 */
    if (is_ap(hmac_vap) && (mac_frame_is_null_data(netbuf) == OSAL_TRUE)) {
        *res = OSAL_TRUE;
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

WIFI_TCM_TEXT OSAL_STATIC osal_u32 hmac_psm_pkt_need_buff_ext(const hmac_device_stru *hmac_device,
    const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user, const oal_netbuf_stru *netbuf,
    const mac_tx_ctl_stru *tx_ctrl)
{
    const mac_ieee80211_frame_stru *mac_header = OSAL_NULL;
    osal_u8 subtype;
    osal_u8 frame_type;
    const osal_u8 *data = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;
    unref_param(hmac_device);
    unref_param(hmac_user);

    /* TWT使能且处于TWT SP之内，无需缓存 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_NOT_NEED_BUFF);
    if (fhook != OSAL_NULL && ((hmac_is_twt_processed_not_need_buff_cb)fhook)(hmac_vap)) {
        return OSAL_FALSE;
    }

    /* 1.ACS AP扫描发probe req时不需要节能
     * 2.CSA ACTION帧不入节能队列 */
    data = oal_netbuf_tx_data_const(netbuf);
    mac_header = (const mac_ieee80211_frame_stru *)oal_netbuf_header_const(netbuf);
    subtype = mac_frame_get_subtype_value((const osal_u8 *)mac_header);
    frame_type = mac_frame_get_type_value((const osal_u8 *)mac_header);
    if ((tx_ctrl->ac == WLAN_WME_AC_MGMT) && (frame_type == WLAN_MANAGEMENT)) {
        if (subtype == WLAN_PROBE_REQ) {
            return OSAL_FALSE;
        }
        /* 0: Spectrum Management, 4: Channel Switch Announcement Frame */
        if (subtype == WLAN_ACTION && data[0] == 0x0 && data[1] == 0x4) {
            return OSAL_FALSE;
        }
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P Go主动断开时,不处于p2p节能模式，无需管sta是否处于节能模式, 发disassoc 应无需缓存 */
    if (is_ap(hmac_vap) && (is_p2p_go(hmac_vap)) &&
        (hmac_device->st_p2p_info.p2p_ps_pause == OSAL_FALSE) && (frame_type == WLAN_MANAGEMENT) &&
        (subtype == WLAN_DISASOC)) {
        /* 恢复该用户的硬件队列的发送 */
        hal_tx_disable_peer_sta_ps_ctrl(hmac_user->lut_index);
        return OSAL_FALSE;
    }
#endif

    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_psm_pkt_need_buff
 功能描述  : 判断一个帧是否需要入节能队列，入参的net_buff有可能是一个net_buff
             链，这个链肯定是发给同一个用户的，所有的net_buff有关节能的特征
             都是相同的，因此只需要判断第一个net_buff就可以了。
*****************************************************************************/
WIFI_TCM_TEXT osal_u8 hmac_psm_pkt_need_buff(const hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
    mac_tx_ctl_stru *tx_ctrl = OSAL_NULL;
    osal_u32 res = OSAL_TRUE;
    osal_void *fhook = OSAL_NULL;

    /* p2p noa节能和TWT节能也需要缓存,原因是节能回退IV号乱序导致丢包 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_IS_SESSION_ENABLE);
    if ((is_legacy_sta(hmac_vap) == OSAL_TRUE) && ((fhook == OSAL_NULL) ||
        ((hmac_twt_is_session_enable_cb)fhook)(hmac_vap) == OSAL_FALSE)) {
        return OSAL_FALSE;
    }

    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    if (tx_ctrl->is_get_from_ps_queue == OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (tx_ctrl->ismcast == OSAL_FALSE) {
        if (hmac_psm_ucast_need_buff(hmac_device, hmac_vap, hmac_user, netbuf, &res) == OSAL_TRUE) {
            return (osal_u8)res;
        }
    } else {
        /* 如果是组播帧，则有三种情况下该帧不需要缓存:
           1、所有关联用户都不节能
           2、不处于noa节能状态 */
        /* p2p noa节能也需要缓存 */
        if (is_ap(hmac_vap)) {
#ifdef _PRE_WLAN_FEATURE_WUR_TX
            if (hmac_vap->cur_wur_user_num > 0) {
                return OSAL_TRUE; /* 当前仍有用户处于WUR收发状态 */
            }
#endif
            if ((hmac_vap->ps_user_num == 0) && (hmac_device->st_p2p_info.p2p_ps_pause == OSAL_FALSE)) {
                return OSAL_FALSE;
            }
        } else if (is_p2p_cl(hmac_vap) && (hmac_device->st_p2p_info.p2p_ps_pause == OSAL_FALSE)) {
            return OSAL_FALSE;
        }
    }

    return (osal_u8)hmac_psm_pkt_need_buff_ext(hmac_device, hmac_vap, hmac_user, netbuf, tx_ctrl);
}

/*****************************************************************************
 功能描述  : 把管理帧/组播帧入节能队列
*****************************************************************************/
osal_u32 hmac_psm_ps_enqueue(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *net_buffer)
{
    oal_netbuf_stru        *next_net_buf     = OSAL_NULL;
    mac_tx_ctl_stru        *tx_ctrl          = OSAL_NULL;
    osal_s32               ps_mpdu_num;
    oal_netbuf_stru *net_buf = net_buffer;
    hmac_device_stru       *hmac_device = hmac_res_get_mac_dev_etc(0);

    tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    /* APUT需要设置bitmap的条件:1.单播数据但用户节能 2.组播数据并且至少有1个用户节能 */
    if (is_ap(hmac_vap) &&
        (((hmac_user->ps_mode == OSAL_TRUE) && (tx_ctrl->ismcast == OSAL_FALSE)) ||
        ((tx_ctrl->ismcast == OSAL_TRUE) && (hmac_vap->ps_user_num != 0)))) {
        hmac_psm_set_local_bitmap(hmac_vap, hmac_user, 1);
    }

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    /* WUR用户存在，进入节能队列的处理 */
    if (hmac_vap->cur_wur_user_num > 0) {
        hmac_wur_ps_enqueue_process(hmac_vap, hmac_user, tx_ctrl);
    }
#endif

    /* 对节能队列进行操作，加锁保护 */
    osal_spin_lock(&hmac_user->ps_structure.lock_ps);

    /* 更新节能队列中mpdu的个数 */
    ps_mpdu_num = osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num);
    ps_mpdu_num += tx_ctrl->mpdu_num;
    if (ps_mpdu_num > MAX_MPDU_NUM_IN_PS_QUEUE) {
        osal_spin_unlock(&hmac_user->ps_structure.lock_ps);
        oam_warning_log2(0, OAM_SF_PWR, "{hmac_psm_enqueue:: drop pkt user[%d] total %d.}",
            hmac_user->assoc_id, ps_mpdu_num);
        return OAL_FAIL;
    }
    hmac_device->psm_mpdu_num += tx_ctrl->mpdu_num;
    osal_adapt_atomic_set(&hmac_user->ps_structure.mpdu_num, ps_mpdu_num);

    /* 将所有netbuf都挂到节能队列尾部 */
    while (net_buf != OSAL_NULL) {
        next_net_buf = oal_get_netbuf_next(net_buf);
        oal_netbuf_add_to_list_tail(net_buf, &hmac_user->ps_structure.ps_queue_head);
        net_buf = next_net_buf;
    }

    osal_spin_unlock(&hmac_user->ps_structure.lock_ps);

    oam_info_log3(0, OAM_SF_PWR, "{hmac_psm_enqueue::user[%d] enqueue %d & total %d.}",
                  hmac_user->assoc_id, tx_ctrl->netbuf_num, ps_mpdu_num);

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_psm_tx_set_more_data
 功能描述  : 发送流程中，判断如果用户处于节能状态，则需要设置当前帧的more
             data，告诉sta是否还有节能缓存帧
*****************************************************************************/
osal_u32 hmac_psm_tx_set_more_data(hmac_user_stru *hmac_user, mac_tx_ctl_stru *tx_cb)
{
    osal_s32               ps_mpdu_num;
    oal_bool_enum_uint8     tid_empty;

    /* u-apsd pkt more data bit is already set */
    if (hmac_user->ps_mode == OSAL_TRUE && tx_cb->tid != WLAN_TIDNO_UAPSD) {
        ps_mpdu_num = osal_adapt_atomic_read(&hmac_user->ps_structure.mpdu_num);
        tid_empty  = hmac_psm_is_tid_empty(hmac_user);
        if ((ps_mpdu_num != 0) || (tid_empty == OSAL_FALSE)) {
            mac_get_frame_header_addr(tx_cb)->frame_control.more_data = 0x01;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_psm_set_ucast_mgmt_tx_rate
 功能描述  : 设置vap结构体下单播管理帧发送速率
*****************************************************************************/
static osal_void hmac_psm_set_ucast_mgmt_tx_rate(hmac_vap_stru *hmac_vap, wlan_channel_band_enum_uint8 band,
    osal_u8 legacy_rate, wlan_phy_protocol_enum_uint8 protocol_mode)
{
    /* 参数合法性检查 */
    if ((hmac_vap == OSAL_NULL) || (band >= WLAN_BAND_BUTT)) {
        oam_error_log2(0, OAM_SF_PWR, "{hmac_psm_set_ucast_mgmt_tx_rate::input param error, hmac_vap[%p], band[%d].}",
            (uintptr_t)hmac_vap, band);
        return;
    }
    /* 只需要设置0级速率,1 2 3还是原来的值,发不成功硬件可以降速发送null帧 */
    hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_ucast[band]))[0].nss_rate.legacy_rate.legacy_rate = legacy_rate;
    hal_txop_alg_get_tx_dscr(&(hmac_vap->tx_mgmt_ucast[band]))[0].protocol_mode = protocol_mode;
    return;
}

/*****************************************************************************
 函 数 名  : hmac_change_null_data_rate
 功能描述  : 1.根据不同的协议模式更改null frame 的速率
*****************************************************************************/
static osal_void hmac_change_null_data_rate(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 *protocol_mode, osal_u8 *legacy_rate)
{
    switch (hmac_user->avail_protocol_mode) {
        /* 11b 1M */
        case WLAN_LEGACY_11B_MODE:
            if (hmac_vap->channel.band == WLAN_BAND_2G) {
                *protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
                *legacy_rate   = 0x1;
#ifdef _PRE_WLAN_FEATURE_P2P
                /* P2P 设备接收到管理帧 */
                if (!is_legacy_vap((hmac_vap))) {
                    *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
                    *legacy_rate   = 0xB;
                }
#endif
            } else {
                *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
                *legacy_rate   = 0xB;
            }
            break;

        /* OFDM 6M */
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_HT_MODE:
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
        case WLAN_VHT_MODE:
        case WLAN_HT_11G_MODE:
        case WLAN_HE_MODE:
            *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            *legacy_rate   = 0xB;
            break;

        /* OFDM 24M */
        case WLAN_HT_ONLY_MODE:
        case WLAN_VHT_ONLY_MODE:
            *protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            *legacy_rate   = 0x9;
            break;

        default:
            oam_warning_log2(0, OAM_SF_TX, "vap_id[%d] {hmac_change_null_data_rate::invalid protocol[%d].}",
                hmac_vap->vap_id, hmac_user->avail_protocol_mode);
            return;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
