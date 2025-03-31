/*
 * Copyright (c) CompanyNameMagicTag 2016-2023. All rights reserved.
 * 文 件 名   : hmac_opmode.c
 * 生成日期   : 2016年9月1日
 * 功能描述   : opmode notify hmac接口
 */

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_opmode.h"
#include "dmac_alg_rom.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#include "hmac_feature_dft.h"
#include "hmac_psm_ap.h"
#include "dmac_csa_sta_rom.h"
#include "hmac_tx_mgmt.h"
#include "oal_netbuf_data.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_OPMODE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hmac_opmode_cb g_hmac_opmode_rom_cb = {OSAL_NULL};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_check_opmode_notify_etc
 功能描述  : 检查请求关联的STA的Operating Mode Notification
 输入参数  : hmac_vap_stru    *hmac_vap --VAP指针
             osal_u8        *mac_hdr, --帧头指针
             osal_u8        *payload  --payload指针
             osal_u16        info_elem_offset--偏移长度
             osal_u32        msg_len----信息长度
             hmac_user_stru   *hmac_user_sta --用户指针
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_check_opmode_notify_etc(
    hmac_vap_stru                   *hmac_vap,
    osal_u8                       *mac_hdr,
    osal_u8                       *payload_offset,
    osal_u32                       msg_len,
    hmac_user_stru                  *hmac_user)
{
    osal_u8              *opmode_notify_ie = OAL_PTR_NULL;
    mac_opmode_notify_stru *opmode_notify = OAL_PTR_NULL;
    osal_u32              relt;

    if ((mac_mib_get_vht_option_implemented(hmac_vap) == OAL_FALSE) ||
        (mac_mib_get_operating_mode_notification_implemented(hmac_vap) == OAL_FALSE)) {
        return OAL_SUCC;
    }

    opmode_notify_ie = mac_find_ie_etc(MAC_EID_OPMODE_NOTIFY, payload_offset, (osal_s32)msg_len);
    /* STA关联在vht下,且vap在双空间流的情况下才解析此ie */
    if ((opmode_notify_ie != OAL_PTR_NULL) && (opmode_notify_ie[1] == MAC_OPMODE_NOTIFY_LEN)) {
        opmode_notify = (mac_opmode_notify_stru *)(opmode_notify_ie + MAC_IE_HDR_LEN);

        /* SMPS已经解析并更新空间流，因此若空间流不等则SMPS和OPMODE的空间流信息不同 */
        if ((hmac_user->avail_num_spatial_stream > opmode_notify->rx_nss) ||
            ((hmac_user->avail_num_spatial_stream == WLAN_SINGLE_NSS) &&
            (opmode_notify->rx_nss != WLAN_SINGLE_NSS))) {
            oam_warning_log0(0, OAM_SF_OPMODE, "{hmac_check_opmode_notify_etc::SMPS and OPMODE show different nss!}");
            if ((hmac_user->cur_protocol_mode == WLAN_HT_MODE) ||
                (hmac_user->cur_protocol_mode == WLAN_HT_ONLY_MODE)) {
                return OAL_FALSE;
            }
        }

        relt = hmac_ie_proc_opmode_field_etc(hmac_vap, hmac_user, opmode_notify);
        if (OAL_UNLIKELY(relt != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_OPMODE,
                "vap_id[%d] {hmac_check_opmode_notify_etc::hmac_ie_proc_opmode_field_etc failed[%d].}",
                hmac_user->vap_id, relt);
            return relt;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_opmode_notify_frame
 功能描述  : 从空口接收opmode_notify帧的处理函数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_mgmt_rx_opmode_notify_frame_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *pst_netbuf)
{
    mac_opmode_notify_stru     *opmode_notify = OAL_PTR_NULL;
    osal_u8                  *frame_payload = OAL_PTR_NULL;
    mac_rx_ctl_stru           *pst_rx_ctrl;
    osal_u8                  *data = OAL_PTR_NULL;
    osal_u32                  ul_ret;

    if ((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (pst_netbuf == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_OPMODE,
            "{hmac_mgmt_rx_opmode_notify_frame_etc::hmac_vap = [%x],hmac_user = [%x], pst_netbuf = [%x]!}\r\n",
            hmac_vap, hmac_user, pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((mac_mib_get_vht_option_implemented(hmac_vap) == OAL_FALSE) ||
        (mac_mib_get_operating_mode_notification_implemented(hmac_vap) == OAL_FALSE)) {
        oam_info_log1(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_mgmt_rx_opmode_notify_frame_etc::the vap is not support OperatingModeNotification!}\r\n",
            hmac_vap->vap_id);
        return OAL_SUCC;
    }

    pst_rx_ctrl = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 获取帧体指针 */
    data = (osal_u8 *)mac_get_rx_cb_mac_header_addr(pst_rx_ctrl);

    /****************************************************/
    /*   OperatingModeNotification Frame - Frame Body   */
    /* -------------------------------------------------*/
    /* |   Category   |   Action   | OperatingMode| */
    /* -------------------------------------------------*/
    /* |   1          |   1        |   1         | */
    /* -------------------------------------------------*/
    /*                                                  */
    /****************************************************/

    /* 获取payload的指针 */
    frame_payload = (osal_u8 *)data + MAC_80211_FRAME_LEN;
    opmode_notify = (mac_opmode_notify_stru *)(frame_payload + MAC_ACTION_OFFSET_ACTION + 1);

    ul_ret = hmac_ie_proc_opmode_field_etc(hmac_vap, hmac_user, opmode_notify);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_mgmt_rx_opmode_notify_frame_etc::hmac_config_send_event_etc failed[%d].}",
            hmac_user->vap_id, ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_ie_proc_opmode_notify
 功能描述  : 处理Operating Mode字段
*****************************************************************************/
osal_u32 hmac_ie_proc_opmode_notify(hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap,
    mac_opmode_notify_stru *opmode_notify)
{
    osal_u32                  relt;
    wlan_bw_cap_enum_uint8  bwcap_user;     /* user之前的带宽信息 */
    wlan_nss_enum_uint8     avail_bf_nss;   /* 用户支持的Beamforming空间流个数 */
    wlan_nss_enum_uint8     avail_nss;      /* Tx和Rx支持Nss的交集,供算法调用 */

    avail_bf_nss = hmac_user->avail_bf_num_spatial_stream;
    avail_nss = hmac_user->avail_num_spatial_stream;

    bwcap_user = hmac_user->avail_bandwidth;

    relt = hmac_ie_proc_opmode_field_etc(hmac_vap, hmac_user, opmode_notify);
    if (osal_unlikely(relt != OAL_SUCC)) {
        return relt;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_rx_beacon_check_opmode_notify
 功能描述  : 检查请求关联的STA的Operating Mode Notification
*****************************************************************************/
osal_u32 hmac_rx_beacon_check_opmode_notify(
    hmac_vap_stru                   *hmac_vap,
    osal_u8                       *payload,
    osal_u32                       msg_len,
    hmac_user_stru                   *hmac_user)
{
    mac_opmode_notify_stru *opmode_notify = OSAL_NULL;
    osal_u8              *opmode_notify_ie;
    wlan_bw_cap_enum_uint8  bwcap_user;                      /* user之前的带宽信息 */
    osal_u32              change = MAC_NO_CHANGE;

    bwcap_user = hmac_user->avail_bandwidth;

    if ((mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_FALSE)
        || (mac_mib_get_operating_mode_notification_implemented(hmac_vap) == OSAL_FALSE)) {
        return change;
    }

    opmode_notify_ie = mac_find_ie_etc(MAC_EID_OPMODE_NOTIFY, payload, (osal_s32)(msg_len));
    if (opmode_notify_ie == OSAL_NULL) {
        return change;
    }

    if (opmode_notify_ie[1] < MAC_OPMODE_NOTIFY_LEN) {
        oam_warning_log1(0, OAM_SF_OPMODE,
            "{hmac_rx_beacon_check_opmode_notify::invalid opmode notify ie len[%d]!}", opmode_notify_ie[1]);
        return change;
    }

    opmode_notify = (mac_opmode_notify_stru *)(opmode_notify_ie + MAC_IE_HDR_LEN);

    /* SMPS已经解析并更新空间流，因此若空间流不等则SMPS和OPMODE的空间流信息不同 */
    if ((hmac_vap->vap_rx_nss > WLAN_SINGLE_NSS) &&
        (hmac_user->avail_num_spatial_stream > opmode_notify->rx_nss ||
        (hmac_user->avail_num_spatial_stream == WLAN_SINGLE_NSS && opmode_notify->rx_nss != WLAN_SINGLE_NSS))) {
        oam_warning_log0(0, OAM_SF_OPMODE, "{hmac_rx_beacon_check_opmode_notify::SMPS and OPMODE show different nss!}");
        if (hmac_user->cur_protocol_mode == WLAN_HT_MODE || hmac_user->cur_protocol_mode == WLAN_HT_ONLY_MODE) {
            return change;
        }
    }

    if (hmac_ie_proc_opmode_field_etc(hmac_vap, hmac_user, opmode_notify) != OAL_SUCC) {
        return change;
    }

    /* 在信道切换过程，不进行带宽切换 */
    if (hmac_user->avail_bandwidth != bwcap_user) {
        if ((hmac_sta_csa_is_in_waiting(hmac_vap) == OSAL_FALSE) && (mac_vap_bw_fsm_beacon_avail(hmac_vap))) {
            change = MAC_BW_OPMODE_CHANGE;
        } else {
            hmac_user_set_bandwidth_info_etc(hmac_user, bwcap_user, bwcap_user);
            /* 带宽更新 则同步至device */
            hmac_user_sync(hmac_user);
        }
    }
    return change;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_send_opmode_notify_action
 功能描述  : 发送operating mode notify控制帧
*****************************************************************************/
osal_u32  hmac_mgmt_send_opmode_notify_action(hmac_vap_stru *hmac_vap, wlan_nss_enum_uint8 nss,
    oal_bool_enum_uint8 bool_code)
{
    oal_netbuf_stru        *mgmt_buf;
    osal_u16              mgmt_len;
    mac_tx_ctl_stru        *tx_ctl;
    osal_u32              ret;

    /* 申请管理帧内存 */
    mgmt_buf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (mgmt_buf == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_mgmt_send_opmode_notify_action::mgmt_buf null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);

    /* 封装 operating mode notify 帧 */
    hmac_mgmt_encap_opmode_notify_action(hmac_vap, mgmt_buf, &mgmt_len, bool_code, nss);
    if (mgmt_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oal_netbuf_free(mgmt_buf);
        oam_error_log1(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_mgmt_send_opmode_notify_action::encap opmode notify action failed.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    tx_ctl->tx_user_idx      = hmac_vap->assoc_vap_id;
    tx_ctl->ac               = WLAN_WME_AC_MGMT;
    tx_ctl->frame_type       = WLAN_CB_FRAME_TYPE_ACTION;
    tx_ctl->frame_subtype    = WLAN_ACTION_OPMODE_FRAME_SUBTYPE;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, mgmt_buf, mgmt_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
        oam_error_log1(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_mgmt_send_opmode_notify_action::tx opmode notify action failed.}", hmac_vap->vap_id);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_M2S
osal_u32 hmac_user_m2s_info_hdl(hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap,
    mac_opmode_notify_stru *opmode_notify)
{
    oal_bool_enum_uint8 nss_change = OSAL_FALSE;
    oal_bool_enum_uint8 bw_change = OSAL_FALSE;
    osal_u32 ret;

    /* 用户支持的Beamforming空间流个数 */
    wlan_nss_enum_uint8 avail_bf_nss = hmac_user->avail_bf_num_spatial_stream;
    /* Tx和Rx支持Nss的交集,供算法调用 */
    wlan_nss_enum_uint8 avail_nss = hmac_user->avail_num_spatial_stream;
    /* user之前的带宽信息 */
    wlan_bw_cap_enum_uint8 bwcap_user = hmac_user->avail_bandwidth;

    ret = hmac_ie_proc_opmode_field_etc(hmac_vap, hmac_user, opmode_notify);
    if (osal_unlikely(ret != OAL_SUCC)) {
        return ret;
    }

    /* 1.若空间流能力发送变化，则调用算法钩子函数,如果带宽和空间流同时改变，要先调用空间流的算法函数 */
    if ((hmac_user->avail_bf_num_spatial_stream != avail_bf_nss) ||
        (hmac_user->avail_num_spatial_stream != avail_nss)) {
        nss_change = OSAL_TRUE;
    }

    /* 2.opmode带宽改变通知算法,并同步带宽信息到HOST */
    if (hmac_user->avail_bandwidth != bwcap_user) {
        bw_change = OSAL_TRUE;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_mgmt_rx_opmode_notify_frame
 功能描述  : 从空口接收opmode_notify帧的处理函数
*****************************************************************************/
osal_u32 hmac_mgmt_rx_opmode_notify_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf)
{
#ifndef _PRE_WLAN_FEATURE_M2S
    osal_u32 ret;
#endif
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    /* 获取帧体指针 */
    osal_u8 *data = oal_netbuf_rx_data(netbuf);
    mac_opmode_notify_stru *opmode_notify = OSAL_NULL;

    /* 是否需要处理Power Management bit位 */
    osal_u8 power_save = (osal_u8)frame_hdr->frame_control.power_mgmt;
    /* vap是单空间流时不调整空间流 */
    if (is_vap_single_nss(hmac_vap) || is_user_single_nss(hmac_user)) {
        return OAL_SUCC;
    }

    if ((mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_FALSE) ||
        (mac_mib_get_operating_mode_notification_implemented(hmac_vap) == OSAL_FALSE)) {
        oam_info_log1(0, OAM_SF_OPMODE,
            "{hmac_mgmt_rx_opmode_notify_frame::vap[%d] is not support OperatingModeNotification!}", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    /* 如果节能位开启(power_mgmt == 1),抛事件到DMAC，处理用户节能信息 */
    if ((power_save == OSAL_TRUE) && (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP)) {
        if (hmac_user->ps_mode == OSAL_FALSE) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_mgmt_rx_opmode_notify_frame::user changes mode to powersave!}",
                hmac_vap->vap_id);
            hmac_psm_doze(hmac_vap, hmac_user);
        }
    }

    /****************************************************/
    /*   OperatingModeNotification Frame - Frame Body   */
    /* ------------------------------------------------- */
    /* |   Category   |   Action   |   OperatingMode   | */
    /* ------------------------------------------------- */
    /* |   1          |   1        |   1               | */
    /* ------------------------------------------------- */
    /*                                                  */
    /****************************************************/
    /* 获取pst_opmode_notify的指针 */
    opmode_notify = (mac_opmode_notify_stru *)(data + MAC_ACTION_OFFSET_ACTION + 1);

    oam_warning_log4(0, OAM_SF_OPMODE,
        "{hmac_mgmt_rx_opmode_notify_frame::user[%d] nss type[%d],rx nss[%d],bandwidch[%d]!}", hmac_user->assoc_id,
        opmode_notify->rx_nss_type, opmode_notify->rx_nss, opmode_notify->channel_width);

#ifdef _PRE_WLAN_FEATURE_M2S
    return hmac_user_m2s_info_hdl(hmac_user, hmac_vap, opmode_notify);
#else

    ret = hmac_ie_proc_opmode_notify(hmac_user, hmac_vap, opmode_notify);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hmac_mgmt_rx_opmode_notify_frame::call hmac_ie_proc_opmode_notify, ret = [%d]}.", ret);
    }

    return OAL_SUCC;
#endif
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_opmode_notify_action
 功能描述  : dmac组opmode notify action帧
*****************************************************************************/
osal_void hmac_mgmt_encap_opmode_notify_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u16 *len,
    oal_bool_enum_uint8 bool_code, wlan_nss_enum_uint8 nss)
{
    osal_u16 index;
    mac_ieee80211_frame_stru *mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    mac_opmode_notify_action_mgt_stru *payload_addr =
        (mac_opmode_notify_action_mgt_stru *)oal_netbuf_data_offset(netbuf, MAC_80211_FRAME_LEN);

    *len = 0;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 设置 Frame Control field */
    mac_hdr_set_frame_control((osal_u8 *)mac_header,
                              WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /*  power management bit is never sent by an AP */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_header->frame_control.power_mgmt = bool_code;
    }

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number((osal_u8 *)mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID) */
    oal_set_mac_addr((osal_u8 *)mac_header + WLAN_HDR_ADDR1_OFFSET, hmac_vap->bssid);

    /* 设置 address2(发送端): dot11StationID */
    if (hmac_vap->mib_info != OSAL_NULL) {
        oal_set_mac_addr((osal_u8 *)mac_header + WLAN_HDR_ADDR2_OFFSET,
            hmac_vap->mib_info->wlan_mib_sta_config.dot11_station_id);
    }

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr((osal_u8 *)mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /*************************************************************************/
    /*                  operating mode notify Management frame - Frame Body  */
    /* ----------------------------------------------------------------------*/
    /* |Category |VHT Action |Operating mode field|                          */
    /* ----------------------------------------------------------------------*/
    /* |1        |1          |1                   |                          */
    /* ----------------------------------------------------------------------*/
    /*                                                                       */
    /*************************************************************************/
    index = MAC_80211_FRAME_LEN;

    payload_addr->category = MAC_ACTION_CATEGORY_VHT;            /* VHT Category */
    payload_addr->action = MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION;               /* VHT Action */

    index += MAC_IE_HDR_LEN;

    hmac_set_opmode_field_etc((osal_u8 *)hmac_vap, &(payload_addr->opmode_ctl), nss);

    index += MAC_OPMODE_NOTIFY_LEN;

    *len = index;

    if (g_hmac_opmode_rom_cb.encap_opmode_action_cb != OSAL_NULL) {
        g_hmac_opmode_rom_cb.encap_opmode_action_cb(hmac_vap, netbuf, len, nss);
    }
}


/*****************************************************************************
 函 数 名  : hmac_config_set_opmode_notify_etc
 功能描述  : hmac设置工作模式通知能力
*****************************************************************************/
osal_s32 hmac_config_set_opmode_notify_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (mac_mib_get_vht_option_implemented(hmac_vap) == OAL_TRUE) {
        mac_mib_set_operating_mode_notification_implemented(hmac_vap, (oal_bool_enum_uint8)*msg->data);
    } else {
        oam_warning_log1(0, OAM_SF_OPMODE,
            "{hmac_config_set_opmode_notify_etc::mac_vap not support VHT. protocol = [%d].}", hmac_vap->protocol);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

