/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hmac csa ap
 * Create: 2023-2-17
 */

#include "hmac_csa_ap.h"
#include "hmac_beacon.h"
#include "hmac_chan_mgmt.h"
#include "hmac_tx_mgmt.h"
#include "frw_util_notifier.h"
#include "hmac_feature_dft.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"
#include "wal_utils.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CSA_AP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

OSAL_STATIC osal_u32 hmac_ap_send_csa_action(hmac_vap_stru *hmac_vap);

hmac_csa_ap_info_stru *g_csa_ap_vapinfo[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

OSAL_STATIC hmac_csa_ap_info_stru *hmac_csa_ap_get_vap_info(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_NULL;
    }
    return g_csa_ap_vapinfo[vap_id];
}

OSAL_STATIC osal_bool hmac_csa_ap_is_ch_switch(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (g_csa_ap_vapinfo[vap_id] == OSAL_NULL) {
        return OSAL_FALSE;
    }

    return (g_csa_ap_vapinfo[vap_id]->ch_switch_status == WLAN_CH_SWITCH_STATUS_1) ? OSAL_TRUE : OSAL_FALSE;
}

OSAL_STATIC oal_bool_enum_uint8 hmac_csa_ap_csa_present_in_bcn(osal_u8 vap_id)
{
    if (hmac_vap_id_param_check(vap_id) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    if (g_csa_ap_vapinfo[vap_id] == OSAL_NULL) {
        return OSAL_FALSE;
    }

    return g_csa_ap_vapinfo[vap_id]->csa_present_in_bcn;
}

/*****************************************************************************
 函 数 名  : hmac_csa_ap_switch_new_channel
 功能描述  : 设置VAP信道参数，准备切换至新信道运行
 输入参数  : hmac_vap : MAC VAP结构体指针
             channel  : 新信道号(准备切换到的20MHz主信道号)
             en_bandwidth: 新带宽模式
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
OSAL_STATIC osal_void hmac_csa_ap_switch_new_channel(hmac_vap_stru *hmac_vap, osal_u8 channel,
    wlan_channel_bandwidth_enum_uint8 en_bandwidth,  osal_bool disable_tx, osal_u8 switch_cnt)
{
    hmac_csa_ap_info_stru *ap_csa_info = OSAL_NULL;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE) {
        return;
    }

    ap_csa_info = hmac_csa_ap_get_vap_info(hmac_vap->vap_id);
    if (osal_unlikely(ap_csa_info == OSAL_NULL)) {
        return;
    }
    /* AP准备切换信道 */
    ap_csa_info->csa_mode = disable_tx;
    ap_csa_info->ch_switch_cnt = switch_cnt;
    ap_csa_info->ch_switch_status    = WLAN_CH_SWITCH_STATUS_1;
    ap_csa_info->announced_channel   = channel;
    ap_csa_info->announced_bandwidth = en_bandwidth;
    /* 在Beacon帧中添加Channel Switch Announcement IE */
    ap_csa_info->csa_present_in_bcn  = OAL_TRUE;

    oam_warning_log4(0, OAM_SF_2040,
        "vap_id[%d] {hmac_csa_ap_switch_new_channel::announced_channel=%d,announced_bandwidth=%d csa_cnt=%d}",
        hmac_vap->vap_id, channel, en_bandwidth, ap_csa_info->ch_switch_cnt);

    /* 11n 协议模式才支持CSA action 帧  发送 Channel Switch Announcement 帧 */
    if (mac_mib_get_high_throughput_option_implemented(hmac_vap) == OSAL_TRUE) {
        hmac_ap_send_csa_action(hmac_vap);
    }

    ap_csa_info->delay_update_switch_cnt = OSAL_TRUE;
    /* 发送CSA帧后及时更新Beacon帧 */
    hmac_encap_beacon(hmac_vap, oal_netbuf_header(hmac_vap->beacon_buffer));

    return;
}

/*****************************************************************************
 函 数 名  : hmac_csa_ap_set_bandwidth_ie
 功能描述  : 为11AC模式下，信道切换构建宽带IE
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_csa_ap_vht_set_bw_ie(osal_u8 channel, wlan_channel_bandwidth_enum_uint8 bw, osal_u8 *buffer,
    osal_u8 *output_len)
{
    if ((buffer == OSAL_NULL) || (output_len == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_csa_ap_vht_set_bw_ie::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 默认输出为空 */
    *buffer = '\0';
    *output_len = 0;

    /* 11ac 设置Wide Bandwidth Channel Switch Element                 */
    /******************************************************************/
    /* -------------------------------------------------------------- */
    /* |ID |Length |New Ch width |Center Freq seg1 |Center Freq seg2  */
    /* -------------------------------------------------------------- */
    /* |1  |1      |1            |1                |1                 */
    /*                                                                */
    /******************************************************************/
    buffer[0] = 194; /* 194: Element ID */
    buffer[1] = 3;   /* 3 */
    switch (bw) {
        case WLAN_BAND_WIDTH_20M:
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_40MINUS:
            buffer[MAC_IE_HDR_LEN] = 0;
            buffer[3] = 0; /* 3 */
            break;

        case WLAN_BAND_WIDTH_80PLUSPLUS:
            buffer[MAC_IE_HDR_LEN] = 1;
            buffer[3] = channel + 6; /* 3, 6 */
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            buffer[MAC_IE_HDR_LEN] = 1;
            buffer[3] = channel - 2; /* 3, 2 */
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            buffer[MAC_IE_HDR_LEN] = 1;
            buffer[3] = channel + 2; /* 3, 2 */
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            buffer[MAC_IE_HDR_LEN] = 1;
            buffer[3] = channel - 6; /* 3, 6 */
            break;
        default:
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_csa_ap_vht_set_bw_ie::invalid bandwidth[%d].}", bw);

            return OAL_FAIL;
    }

    buffer[4] = 0; /* 4: reserved. Not support 80M + 80M */

    *output_len = 5;  /* 5 */

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_cas_ap_set_sec_channel_offset_ie
 功能描述  : 构建从20M信道偏移IE
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_cas_ap_set_sec_channel_offset_ie(wlan_channel_bandwidth_enum_uint8 bw, osal_u8 *buffer,
    osal_u8 *len)
{
    if ((buffer == OSAL_NULL) || (len == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_cas_ap_set_sec_channel_offset_ie::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 默认输出为空 */
    *buffer = '\0';
    *len = 0;

    /* 11n 设置Secondary Channel Offset Element */
    /******************************************************************/
    /* -------------------------------------------------------------- */
    /* |Ele. ID |Length |Secondary channel offset |                   */
    /* -------------------------------------------------------------- */
    /* |1       |1      |1                        |                   */
    /*                                                                */
    /******************************************************************/
    buffer[0] = 62; /* 62: Element ID */
    buffer[1] = 1;

    switch (bw) {
        case WLAN_BAND_WIDTH_20M:
            buffer[MAC_IE_HDR_LEN] = 0; /* no secondary channel */
            break;

        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            buffer[MAC_IE_HDR_LEN] = 1; /* secondary 20M channel above */
            break;

        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            buffer[MAC_IE_HDR_LEN] = 3; /* 2, 3: secondary 20M channel below */
            break;

        default:
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_cas_ap_set_sec_channel_offset_ie::invalid bandwidth[%d].}", bw);

            return OAL_FAIL;
    }

    *len = 3; /* 3 */
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_csa_ap_set_channel_ie
 功能描述  : 设置Channel Switch Announcement IE
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_csa_ap_set_channel_ie(osal_u8 mode, osal_u8 channel, osal_u8 csa_cnt, osal_u8 *buffer,
    osal_u8 *ie_len)
{
    if (osal_unlikely((buffer == OSAL_NULL) || (ie_len == OSAL_NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*  Channel Switch Announcement Information Element Format               */
    /* --------------------------------------------------------------------- */
    /* | Element ID | Length | Chnl Switch Mode | New Chnl | Ch Switch Cnt | */
    /* --------------------------------------------------------------------- */
    /* | 1          | 1      | 1                | 1        | 1             | */
    /* --------------------------------------------------------------------- */

    /* 设置Channel Switch Announcement Element */
    buffer[0] = MAC_EID_CHANSWITCHANN;
    buffer[1] = MAC_CHANSWITCHANN_LEN;
    buffer[2] = mode;       /* 2 */
    buffer[3] = channel;    /* 3 */
    buffer[4] = csa_cnt;    /* 4 */

    *ie_len = MAC_IE_HDR_LEN + MAC_CHANSWITCHANN_LEN;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_csa_ap_set_bw_ie
 功能描述  : 设置csa bw 相关IE
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_csa_ap_set_bw_ie(const hmac_vap_stru *hmac_vap, osal_u8 *buf, osal_u8 *ie_len)
{
    wlan_channel_bandwidth_enum_uint8 bw;
    osal_u8 len;
    osal_u8 channel;
    hmac_csa_ap_info_stru *ap_csa_info = OSAL_NULL;
    osal_u8 *buffer = buf;

    if (osal_unlikely((buffer == OSAL_NULL) || (ie_len == OSAL_NULL) || (hmac_vap == OSAL_NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ap_csa_info = hmac_csa_ap_get_vap_info(hmac_vap->vap_id);
    if (osal_unlikely(ap_csa_info == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    bw = ap_csa_info->announced_bandwidth;
    channel = ap_csa_info->announced_channel;
    len = 0;
    /* 封装Second channel offset IE */
    if (hmac_cas_ap_set_sec_channel_offset_ie(bw, buffer, &len) != OAL_SUCC) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_csa_ap_set_bw_ie::set_second_ch_ie failed}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    buffer += len;
    *ie_len = len;

    if (mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_TRUE) {
        /* 11AC Wide Bandwidth Channel Switch IE */
        len = 0;
        if (hmac_csa_ap_vht_set_bw_ie(channel, bw, buffer, &len) != OAL_SUCC) {
            oam_error_log1(0, 0, "vap_id[%d] {hmac_csa_ap_set_bw_ie::set_11ac_wideband_ie failed}", hmac_vap->vap_id);
            return OAL_FAIL;
        }
        *ie_len += len;
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_csa_ap_encap_csa_ie(hmac_vap_stru *hmac_vap, osal_u8 *buf, osal_u8 *ie_len)
{
    osal_u8 len = 0;
    hmac_csa_ap_info_stru *ap_csa_info = hmac_csa_ap_get_vap_info(hmac_vap->vap_id);
    osal_u8 *buffer = buf;

    *ie_len = 0;
    /* 开始封装信道切换所需的各种IE */
    /* 封装CSA IE */
    if (hmac_csa_ap_set_channel_ie(ap_csa_info->csa_mode, ap_csa_info->announced_channel,
        ap_csa_info->ch_switch_cnt, buffer, &len) != OAL_SUCC) {
        *ie_len = 0;
        return OAL_FAIL;
    }
    buffer += len;
    *ie_len += len;

    len = 0;
    hmac_csa_ap_set_bw_ie(hmac_vap, buffer, &len);
    buffer += len;

    *ie_len += len;

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_csa_ap_encap_beacon_csa_ie(hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_csa_ap_info_stru *ap_csa_info = OSAL_NULL;

    if (hmac_vap_mode_param_check(hmac_vap) != OSAL_TRUE || osal_unlikely(ie_len == OSAL_NULL)) {
        return;
    }

    *ie_len = 0;

    ap_csa_info = hmac_csa_ap_get_vap_info(hmac_vap->vap_id);
    if (osal_unlikely(ap_csa_info == OSAL_NULL)) {
        return;
    }

    if (ap_csa_info->csa_present_in_bcn != OSAL_TRUE) {
        return;
    }

    hmac_csa_ap_encap_csa_ie(hmac_vap, buffer, ie_len);
}

/*****************************************************************************
 函 数 名  : hmac_ap_encap_csa_action
 功能描述  : 封装channel switch announcement action 管理帧
*****************************************************************************/
OSAL_STATIC osal_u16 hmac_ap_encap_csa_action(hmac_vap_stru *hmac_vap, oal_netbuf_stru *buffer)
{
    osal_u8 len = 0;
    osal_u8 *mac_header = oal_netbuf_header(buffer);
    osal_u8 *payload_addr = oal_netbuf_data_offset(buffer, MAC_80211_FRAME_LEN);
    osal_u8 *payload_addr_origin = payload_addr;
    osal_u16 frame_length;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置地址1，广播地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, BROADCAST_MACADDR);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, hmac_vap->bssid);

    /* 地址3，为VAP自己的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);

    /*************************************************************************/
    /*                Set the fields in the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*                       Channel Switch Announcement Frame - Frame Body  */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Ch switch IE| 2nd Ch offset|Wide bw IE (11ac only) */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 5          |  3           |5                      */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    /* 设置Action的Category   */
    /* 0: Spectrum Management */
    payload_addr[0] = 0;

    /* 设置Spectrum Management Action Field */
    /* 4: Channel Switch Announcement Frame */
    payload_addr[1] = 4;

    payload_addr += 2; /* 2表示定位到信息元素ACTION位之后的数据 */

    if (hmac_csa_ap_encap_csa_ie(hmac_vap, payload_addr, &len) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_ap_encap_csa_action: build csa ie failed!");
        return 0;
    }
    payload_addr += len;

    frame_length = ((osal_u16)(payload_addr - payload_addr_origin) + MAC_80211_FRAME_LEN);

    oam_warning_log2(0, OAM_SF_DFS, "vap_id[%d] {hmac_ap_encap_csa_action::LEN = %d.}",
        hmac_vap->vap_id, frame_length);

    return frame_length;
}

/*****************************************************************************
 函 数 名  : hmac_ap_send_csa_action
 功能描述  : 发送信道切换广播管理帧
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_ap_send_csa_action(hmac_vap_stru *hmac_vap)
{
    oal_netbuf_stru        *mgmt_buf = OSAL_NULL;
    osal_u16              mgmt_len;
    mac_tx_ctl_stru        *tx_ctl = OSAL_NULL;
    osal_u32              ret;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_ap_send_csa_action::hmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 至少应该是HT(11n)才能发送该帧 */
    if (mac_mib_get_high_throughput_option_implemented(hmac_vap) != OSAL_TRUE) {
        oam_error_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_ap_send_csa_action::vap not in HT mode.}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 申请管理帧内存 */
    mgmt_buf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (mgmt_buf == OSAL_NULL) {
        /* 信道切换action帧没有发送成功，对acs以外的业务引起的带宽切换并无影响。beacon帧中也有信道切换信息。下同 */
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_ap_send_csa_action::mgmt_buf null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(mgmt_buf, OSAL_NULL);
    oal_set_netbuf_next(mgmt_buf, OSAL_NULL);

    /* 封装 Channel Switch Announcement 帧 */
    mgmt_len = hmac_ap_encap_csa_action(hmac_vap, mgmt_buf);
    if (mgmt_len == 0) {
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OSAL_NULL);
        oal_netbuf_free(mgmt_buf);
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_ap_send_csa_action::encap csa action failed.}");
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(mgmt_buf);
    tx_ctl->tx_user_idx  = (osal_u8)hmac_vap->multi_user_idx; /* channel switch帧是广播帧 */
    tx_ctl->ismcast  = OSAL_TRUE;
    tx_ctl->ac = WLAN_WME_AC_MGMT;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, mgmt_buf, mgmt_len, OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(mgmt_buf);
        oam_warning_log1(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_ap_send_csa_action::tx csa action failed.}", hmac_vap->vap_id);
        return ret;
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_bool hmac_csa_ap_tbtt_event(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    hmac_csa_ap_info_stru *ap_csa_info = hmac_csa_ap_get_vap_info(hmac_vap->vap_id);

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OSAL_TRUE;
    }

    if (osal_unlikely(ap_csa_info == OSAL_NULL)) {
        return OSAL_TRUE;
    }

    /* AP准备进行信道切换 */
    if (ap_csa_info->ch_switch_status != WLAN_CH_SWITCH_STATUS_1) {
        return OSAL_TRUE;
    }

    if (ap_csa_info->delay_update_switch_cnt == OSAL_TRUE) {
        ap_csa_info->delay_update_switch_cnt = OSAL_FALSE;
    } else if (ap_csa_info->ch_switch_cnt != 0) {
        /* Beacon帧中Channel Switch Count做递减操作 */
        ap_csa_info->ch_switch_cnt--;
    }

    if (ap_csa_info->ch_switch_cnt == 0) {
        osal_u8   null_addr[6] = {0}; /* index 6 */
        /* 在当前信道挂起beacon帧发送 */
        mac_vap_pause_tx(hmac_vap);
        hal_vap_beacon_suspend(hmac_vap->hal_vap);
        hal_set_machw_tx_suspend();
        /* 去除Beacon帧中的Channel Switch IE信息 */
        ap_csa_info->csa_present_in_bcn = OSAL_FALSE;
        ap_csa_info->delay_update_switch_cnt = OSAL_FALSE;
        ap_csa_info->ch_switch_status = WLAN_CH_SWITCH_DONE;
        (osal_void)memcpy_s(hmac_vap->bssid, WLAN_MAC_ADDR_LEN, null_addr, WLAN_MAC_ADDR_LEN);
        hmac_chan_attempt_new_chan(hmac_vap, ap_csa_info->announced_channel, ap_csa_info->announced_bandwidth);
    }

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_csa_ap_vap_add(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;
    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OSAL_TRUE;
    }

    if (g_csa_ap_vapinfo[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_csa_ap_vap_add mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_csa_ap_info_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_csa_ap_vap_add malloc null!", vap_id);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_csa_ap_info_stru), 0, sizeof(hmac_csa_ap_info_stru));
    g_csa_ap_vapinfo[vap_id] = (hmac_csa_ap_info_stru *)mem_ptr;

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_csa_ap_vap_del(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_u8 vap_id = hmac_vap->vap_id;

    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        return OSAL_TRUE;
    }

    if (g_csa_ap_vapinfo[vap_id] == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_csa_ap_vap_del mem already free!", vap_id);
        return OSAL_TRUE;
    }

    oal_mem_free((osal_void *)g_csa_ap_vapinfo[vap_id], OAL_TRUE);
    g_csa_ap_vapinfo[vap_id] = OSAL_NULL;

    return OSAL_TRUE;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 功能描述  : 设置CSA的控制参数
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_csa_set_control_param(hmac_vap_stru *hmac_vap, mac_protocol_debug_switch_stru *protocol_debug)
{
    hmac_csa_ap_info_stru *ap_csa_info = hmac_csa_ap_get_vap_info(hmac_vap->vap_id);

    if (protocol_debug == OSAL_NULL || ap_csa_info == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_csa_set_control_param::param is NULL!}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* BIT0:band_force_switch, BIT1:2040_ch_swt_prohi, BIT2:40_intol, BIT3: CSA, BIT4:lsig */
    if ((protocol_debug->cmd_bit_map & BIT3) != 0) {
        oam_warning_log4(0, OAM_SF_CFG,
            "{hmac_csa_set_control_param::csa_mode=[%d],csa_channel=[%d],csa_cnt=[%d],debuf_flag=[%d].",
            protocol_debug->csa_debug_bit3.mode, protocol_debug->csa_debug_bit3.channel,
            protocol_debug->csa_debug_bit3.cnt, protocol_debug->csa_debug_bit3.debug_flag);
        if (protocol_debug->csa_debug_bit3.debug_flag == MAC_CSA_FLAG_CANCLE_DEBUG) {
            ap_csa_info->csa_present_in_bcn = OSAL_FALSE;
            ap_csa_info->ch_switch_status = WLAN_CH_SWITCH_DONE;
        } else if (protocol_debug->csa_debug_bit3.debug_flag == MAC_CSA_FLAG_START_DEBUG) {
            ap_csa_info->announced_channel = protocol_debug->csa_debug_bit3.channel;
            ap_csa_info->csa_mode = protocol_debug->csa_debug_bit3.mode;
            ap_csa_info->ch_switch_cnt = protocol_debug->csa_debug_bit3.cnt;
            ap_csa_info->csa_present_in_bcn = OSAL_TRUE;
            ap_csa_info->ch_switch_status = WLAN_CH_SWITCH_DONE;
        } else {
            return OAL_SUCC;
        }
        if (hmac_vap->beacon_buffer != OSAL_NULL) {
            hmac_encap_beacon(hmac_vap, oal_netbuf_header(hmac_vap->beacon_buffer));
        }
    }

    return OAL_SUCC;
}

static osal_s32 hmac_config_csa_channel_check(hmac_vap_stru *hmac_vap,
    mac_protocol_debug_switch_stru *protocol_debug)
{
#ifdef _PRE_WLAN_FEATURE_DFS
    osal_u8 idx;
    osal_u32 ul_ret;
    hmac_device_stru *dev = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
#endif

    if (hmac_is_channel_num_valid_etc(hmac_vap->channel.band,
        protocol_debug->csa_debug_bit3.channel) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_csa_channel_check::hmac_is_channel_num_valid_etc(%d),return.",
            protocol_debug->csa_debug_bit3.channel);
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_FEATURE_DFS
    /* 2.4G不需要信道状态校验 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        return OAL_SUCC;
    }

    ul_ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band,
        protocol_debug->csa_debug_bit3.channel, &idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_csa_channel_check::hmac_is_channel_num_valid_etc(%d),return.",
            protocol_debug->csa_debug_bit3.channel);
        return OAL_FAIL;
    }
    /* 5G频段需要验证该信道是否可用 */
    if ((dev->ap_channel_list[idx].ch_status == MAC_CHAN_NOT_SUPPORT) ||
            (dev->ap_channel_list[idx].ch_status == MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_config_csa_channel_check::The channel is invalid or radar is detected.(%d),return.",
            protocol_debug->csa_debug_bit3.channel);
        return OAL_FAIL;
    }
    /* 检查信道是否为雷达信道且存在dbac */
    if (hmac_is_dfs_with_dbac_check(hmac_vap, dev, protocol_debug->csa_debug_bit3.channel)) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_config_csa_channel_check::The channel(%d) is a radar channel and dbac exists. ",
            protocol_debug->csa_debug_bit3.channel);
        return OAL_FAIL;
    }
#endif
    return OAL_SUCC;
}

static osal_s32 hmac_config_csa_bandwidth_check(hmac_vap_stru *hmac_vap,
    mac_protocol_debug_switch_stru *protocol_debug)
{
    /* 根据device能力对参数进行检查 */
    if (((protocol_debug->csa_debug_bit3.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (mac_mib_get_dot11_vap_max_bandwidth(hmac_vap) < WLAN_BW_CAP_80M)) ||
        ((protocol_debug->csa_debug_bit3.en_bandwidth >= WLAN_BAND_WIDTH_40PLUS) &&
        (mac_mib_get_dot11_vap_max_bandwidth(hmac_vap) < WLAN_BW_CAP_40M))) {
        protocol_debug->csa_debug_bit3.en_bandwidth =
            hmac_vap_get_bandwith(hmac_vap, protocol_debug->csa_debug_bit3.en_bandwidth);

        /* 设置指定带宽，但device能力不支持该带宽， 刷新成合适带宽做业务 */
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d] {config_csa_param::not support this bw, new_bw=%d dot11_vap_max_bw=%d.}", hmac_vap->vap_id,
            protocol_debug->csa_debug_bit3.en_bandwidth, mac_mib_get_dot11_vap_max_bandwidth(hmac_vap));
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }

    if (!is_2g(hmac_vap) || hmac_vap->cap_flag.autoswitch_2040 == OAL_TRUE) {
        return OAL_SUCC;
    }

    oam_info_log3(0, OAM_SF_CFG, "hmac_config_csa_bandwidth_check::csa_channel=%d bw=%d, vap bw=%d",
        protocol_debug->csa_debug_bit3.channel, protocol_debug->csa_debug_bit3.en_bandwidth,
        hmac_vap->channel.en_bandwidth);

    /* 2.4G不支持20/40切换，csa也不允许改变vap频宽 */
    if (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_20M) {
        protocol_debug->csa_debug_bit3.en_bandwidth = hmac_vap->channel.en_bandwidth;
    } else if (hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40PLUS ||
        hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_40MINUS) {
        /* 配置信道5~7，配置频宽40M，根据实际配置下发参数 */
        if ((protocol_debug->csa_debug_bit3.channel >= HMAC_BW_40M_MIN_CHAN_IDX &&
            protocol_debug->csa_debug_bit3.channel <= hmac_chan_get_bw_40m_max_chan_idx()) &&
            (protocol_debug->csa_debug_bit3.en_bandwidth == WLAN_BAND_WIDTH_40PLUS ||
            protocol_debug->csa_debug_bit3.en_bandwidth == WLAN_BAND_WIDTH_40MINUS)) {
            return OAL_SUCC;
        }

        /* 配置信道<5或>7，修改下发的频宽参数为MINUS或PLUS */
        if (protocol_debug->csa_debug_bit3.channel > hmac_chan_get_bw_40m_max_chan_idx()) {
            protocol_debug->csa_debug_bit3.en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
        } else if (protocol_debug->csa_debug_bit3.channel < HMAC_BW_40M_MIN_CHAN_IDX) {
            protocol_debug->csa_debug_bit3.en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
        } else {
            protocol_debug->csa_debug_bit3.en_bandwidth = hmac_vap->channel.en_bandwidth;
        }
    } else {
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_csa_param
 功能描述  : 设置CSA的控制参数
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_set_csa_param(hmac_vap_stru *hmac_vap, mac_protocol_debug_switch_stru *protocol_debug)
{
    osal_s32 ret = OAL_SUCC;
    wlan_channel_bandwidth_enum_uint8 adapt_bandwidth;

    /* BIT0:band_force_switch, BIT1:2040_ch_swt_prohi, BIT2:40_intol, BIT3: CSA, BIT4:lsig */
    if ((protocol_debug->cmd_bit_map & BIT3) != BIT3) {
        return OAL_FAIL;
    }

    if (!is_legacy_ap(hmac_vap) && !is_p2p_go(hmac_vap)) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {config_csa_param::vap mode is not ap,return.", hmac_vap->vap_id);
        return OAL_SUCC;
    }

    ret = hmac_config_csa_channel_check(hmac_vap, protocol_debug);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = hmac_config_csa_bandwidth_check(hmac_vap, protocol_debug);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* channel switch count若小于dtim_period，则设置为dtim_period */
    if (protocol_debug->csa_debug_bit3.cnt < mac_mib_get_dot11_dtim_period(hmac_vap)) {
        oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d] {config_csa_param::cnt[%u] max bw[%u]}", hmac_vap->vap_id,
            protocol_debug->csa_debug_bit3.cnt, mac_mib_get_dot11_vap_max_bandwidth(hmac_vap));
        protocol_debug->csa_debug_bit3.cnt = (osal_u8)mac_mib_get_dot11_dtim_period(hmac_vap);
    }

    /* 检查设置的带宽模式是否合法，若非法，则自适应 */
    adapt_bandwidth = hmac_regdomain_get_support_bw_mode(protocol_debug->csa_debug_bit3.en_bandwidth,
        protocol_debug->csa_debug_bit3.channel);
    oam_warning_log2(0, OAM_SF_CFG, "config_csa_param::csa_new_bandwidth=%d adapt_new_bandwidth=%d",
        protocol_debug->csa_debug_bit3.en_bandwidth, adapt_bandwidth);
    protocol_debug->csa_debug_bit3.en_bandwidth = adapt_bandwidth;

    oam_warning_log4(0, OAM_SF_CFG, "config_set_csa_param::csa_mode=%d csa_channel=%d csa_cnt=%d debug_flag=%d \n",
        protocol_debug->csa_debug_bit3.mode, protocol_debug->csa_debug_bit3.channel,
        protocol_debug->csa_debug_bit3.cnt, protocol_debug->csa_debug_bit3.debug_flag);

    if (protocol_debug->csa_debug_bit3.debug_flag == MAC_CSA_FLAG_NORMAL) {
        hmac_csa_ap_switch_new_channel(hmac_vap, protocol_debug->csa_debug_bit3.channel,
            protocol_debug->csa_debug_bit3.en_bandwidth, protocol_debug->csa_debug_bit3.mode,
            protocol_debug->csa_debug_bit3.cnt);
        return OAL_SUCC;
    }

    /***************************************************************************
        直调接口同步MAC数据, 替代之前的抛事件到DMAC层
    ***************************************************************************/
    ret = hmac_csa_set_control_param(hmac_vap, protocol_debug);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{config_csa_param::hmac_csa_set_control_param fail[%d].", ret);
    }

    return ret;
}

/* 解析csa命令参数 */
OSAL_STATIC osal_u32 hmac_parse_csa_cmd(const osal_s8 *param, mac_protocol_debug_switch_stru *info)
{
    osal_u32 ret, value;
    osal_s8 str[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 解析csa mode , 1: param num should be 1 */
    ret = (osal_u32)hmac_ccpriv_get_digit_with_check_max(&param, 1, &value);
    if (ret != OAL_SUCC) {
        wifi_printf("Error: csa mode %d error!\n", value);
        oam_warning_log0(0, OAM_SF_CFG, "{wal_parse_csa_cmd::csa mode invalid.}");
        return ret;
    }
    info->csa_debug_bit3.mode = (osal_bool)value;

    /* 解析csa channel */
    ret = (osal_u32)hmac_ccpriv_get_one_arg(&param, str, OAL_SIZEOF(str));
    if (ret != OAL_SUCC) {
        wifi_printf("Error: csa channel %s error!\n", str);
        oam_warning_log0(0, OAM_SF_CFG, "{wal_parse_csa_cmd::get csa channel error.}");
        return ret;
    }

    info->csa_debug_bit3.channel = (osal_u8)oal_atoi((const osal_s8 *)str);

    /* 解析bandwidth */
    ret = (osal_u32)hmac_ccpriv_get_digit_with_check_max(&param, WLAN_BAND_WIDTH_BUTT - 1, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_parse_csa_cmd::get bandwidth error.}");
        return ret;
    }
    info->csa_debug_bit3.en_bandwidth = (wlan_channel_bandwidth_enum_uint8)value;

    /* 解析csa cnt, 255: max csa cnt */
    ret = (osal_u32)hmac_ccpriv_get_digit_with_check_max(&param, 255, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_parse_csa_cmd::get csa cnt error.}");
        return ret;
    }
    info->csa_debug_bit3.cnt = (osal_u8)value;

    /* 解析debug flag */
    ret = (osal_u32)hmac_ccpriv_get_digit_with_check_max(&param, MAC_CSA_FLAG_BUTT - 1, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_parse_csa_cmd::get debug flag error.}");
        return ret;
    }

    info->csa_debug_bit3.debug_flag = (mac_csa_flag_enum_uint8)value;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 打印csa调试命令的格式
*****************************************************************************/
static inline osal_void print_csa_cmd_format(osal_void)
{
    oam_warning_log0(0, OAM_SF_ANY, "{CMD format::sh ccpriv.sh 'wlan0 protocol_debug\
    [csa 0(csa mode) 1(csa channel) 10(csa cnt) 1(debug  flag,0:normal channel channel,\
    1:only include csa ie 2:cannel debug)]}");
}

OSAL_STATIC osal_s32 hmac_ccpriv_csa_ctrl(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    mac_protocol_debug_switch_stru protocol_debug;
    osal_s32 ret;

    memset_s(&protocol_debug, OAL_SIZEOF(protocol_debug), 0, OAL_SIZEOF(protocol_debug));

    ret = (osal_s32)hmac_parse_csa_cmd(param, &protocol_debug);
    if (ret != OAL_SUCC) {
        print_csa_cmd_format();
        return ret;
    }

    protocol_debug.cmd_bit_map |= BIT3;

    oam_warning_log1(0, OAM_SF_ANY, "{uapi_ccpriv_csa_ctrl::cmd_bit_map: 0x%08x.}",
                     protocol_debug.cmd_bit_map);

    oam_warning_log4(0, OAM_SF_ANY,
        "{hmac_ccpriv_csa_ctrl::csa_mode=[%d],csa_channel=[%d],csa_cnt=[%d],debuf_flag=[%d].",
        protocol_debug.csa_debug_bit3.mode, protocol_debug.csa_debug_bit3.channel,
        protocol_debug.csa_debug_bit3.cnt, protocol_debug.csa_debug_bit3.debug_flag);

    if (protocol_debug.csa_debug_bit3.debug_flag != MAC_CSA_FLAG_NORMAL) {
        wifi_printf("Error: invalid flag para[%d]!\r\n", protocol_debug.csa_debug_bit3.debug_flag);
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_csa_ctrl::invalid flag para[%d]}",
            protocol_debug.csa_debug_bit3.debug_flag);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* hmac消息处理函数 */
    ret = hmac_config_set_csa_param(hmac_vap, &protocol_debug);
    if (ret != OAL_SUCC) {
        wifi_printf("fail::ret %d\r\n", ret);
        return ret;
    }

    wifi_printf("OK\r\n");
    return OAL_SUCC;
}
#endif

osal_u32 hmac_csa_ap_init(osal_void)
{
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_csa_ap_vap_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_csa_ap_vap_del);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_TBTT_HANDLE, hmac_csa_ap_tbtt_event);

    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_CSA_AP_IS_SWITCH, hmac_csa_ap_is_ch_switch);
    hmac_feature_hook_register(HMAC_FHOOK_CSA_AP_ENCAP_BEACON_CSA_IE, hmac_csa_ap_encap_beacon_csa_ie);
    hmac_feature_hook_register(HMAC_FHOOK_CSA_AP_CSA_PRESENT_IN_BCN, hmac_csa_ap_csa_present_in_bcn);
    hmac_feature_hook_register(HMAC_FHOOK_CSA_AP_SWITCH_NEW_CHANNEL, hmac_csa_ap_switch_new_channel);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_register((const osal_s8 *)"csa_ctrl", hmac_ccpriv_csa_ctrl);
#endif

    return OAL_SUCC;
}

osal_void hmac_csa_ap_deinit(osal_void)
{
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_csa_ap_vap_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_csa_ap_vap_del);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_TBTT_HANDLE, hmac_csa_ap_tbtt_event);

    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_CSA_AP_IS_SWITCH);
    hmac_feature_hook_unregister(HMAC_FHOOK_CSA_AP_ENCAP_BEACON_CSA_IE);
    hmac_feature_hook_unregister(HMAC_FHOOK_CSA_AP_CSA_PRESENT_IN_BCN);
    hmac_feature_hook_unregister(HMAC_FHOOK_CSA_AP_SWITCH_NEW_CHANNEL);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_unregister((const osal_s8 *)"csa_ctrl");
#endif

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
