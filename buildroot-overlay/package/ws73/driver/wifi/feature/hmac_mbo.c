/*
 * Copyright (c) CompanyNameMagicTag. 2022-2024. All rights reserved.
 * 文 件 名   : hmac_mbo.c
 * 创建日期 : 2022年09月24日
 */

#include "hmac_mbo.h"
#include "oam_ext_if.h"
#include "wlan_types_common.h"
#include "mac_frame.h"
#include "hmac_config.h"
#include "securec.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_MBO_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

mac_mbo_para_stru *g_sta_mbo_para_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};
mac_assoc_retry_delay_attr_mbo_ie_stru *g_assoc_delay_attr_mbo_ie = OSAL_NULL;

OAL_STATIC mac_mbo_para_stru *hmac_mbo_get_param_info(osal_u8 vap_id)
{
    return g_sta_mbo_para_info[vap_id];
}

OAL_STATIC mac_assoc_retry_delay_attr_mbo_ie_stru *hmac_mbo_get_assoc_delay_ie(osal_void)
{
    return g_assoc_delay_attr_mbo_ie;
}

/* 功能描述  : 获取ies中的MBO IE指针地址 */
OAL_STATIC osal_u8 *mac_get_mbo_ie(osal_u8 *ies, osal_s32 ie_len)
{
    osal_u8 *ie = OSAL_NULL;

    if (ie_len == 0) {
        return OSAL_NULL;
    }

    /* 查找私有vendor ie */
    ie = hmac_find_vendor_ie_etc(MAC_MBO_VENDOR_IE, MAC_MBO_IE_OUI_TYPE, ies, ie_len);
    if (ie != OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "mac_get_mbo_ie::Get mbo ie succ!");
    }
    return ie;
}

/* 功能描述  : 填充vendor mbo element信息 MBO IE */
OAL_STATIC osal_void mac_set_mbo_ie(hmac_vap_stru *hmac_vap, osal_u8 *buffer)
{
    /***************************************************************************
                         --------------------------------------------------
                         |Element ID |Length |OUI|OUI Type|MBO Attributes |
                         --------------------------------------------------
          Octets:        |1          |1      |3  |1       |n              |
                         --------------------------------------------------
    ***************************************************************************/
    mac_ieee80211_vendor_ie_stru *vendor_ie = NULL;

    vendor_ie = (mac_ieee80211_vendor_ie_stru *)buffer;

    /* 设置MBO元素的OUI和OUI type = 0x16 */
    vendor_ie->element_id = MAC_EID_VENDOR;
    vendor_ie->len = 7; /* ie长度为7 */
    vendor_ie->oui[0] = (osal_u8)((MAC_MBO_VENDOR_IE >> BIT_OFFSET_16) & 0xff);
    vendor_ie->oui[1] = (osal_u8)((MAC_MBO_VENDOR_IE >> BIT_OFFSET_8) & 0xff);
    vendor_ie->oui[2] = (osal_u8)((MAC_MBO_VENDOR_IE) & 0xff); // 2: OUI byte idex
    vendor_ie->oui_type = MAC_MBO_IE_OUI_TYPE;

    buffer[6] = MBO_ATTR_ID_CELL_DATA_CAPA; /* 6 cell data attr offset */
    buffer[7] = 1; /* 7 cell data attr len offset */
    buffer[8] = hmac_mbo_get_param_info(hmac_vap->vap_id)->mbo_cell_capa; /* 8 cell data attr value offset */
}

/* 功能描述  : 填充Supported Operating Classes element信息 */
OAL_STATIC osal_void mac_set_supported_operating_classes_ie(const hmac_vap_stru *hmac_vap, osal_u8 *buffer,
    osal_u8 *ie_len, const hmac_scanned_bss_info  *scaned_bss)
{
    *ie_len = 0;

    if (scaned_bss == OSAL_NULL) {
        return;
    }

    /*******************************************************************************************
            ------------------------------------------------------------------------------------
            |ElementID | Length | Current Operating Class| Operating Class |
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                      | variable        |
            ------------------------------------------------------------------------------------
            |Current Operating Class Extension Sequence(O) | Operating Class Duple Sequence(O) |
            ------------------------------------------------------------------------------------
    Octets: |              variable                        |             variable              |
            ------------------------------------------------------------------------------------

    *********************************************************************************************/
    buffer[0] = MAC_EID_OPERATING_CLASS;
    buffer[1] = 3; /* length为3 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        buffer[2] = MAC_GLOBAL_OPERATING_CLASS_NUMBER81; // 2: CLASS_NUMBER idex
    }
    if (hmac_vap->channel.band == WLAN_BAND_5G) {
        buffer[2] = MAC_GLOBAL_OPERATING_CLASS_NUMBER115; // 2: CLASS_NUMBER idex
    }
    buffer[3] = MAC_GLOBAL_OPERATING_CLASS_NUMBER81; // 3: CLASS_NUMBER idex
    buffer[4] = MAC_GLOBAL_OPERATING_CLASS_NUMBER115; // 4: CLASS_NUMBER idex

    *ie_len = MAC_IE_HDR_LEN + 3; /* length为3 */
}

/* 功能描述  : 查找mbo attribute信息 */
OAL_STATIC osal_u8 *mac_find_mbo_attribute(osal_u8 eid, osal_u8 *ies, osal_s32 len)
{
    osal_s32 ie_len;
    osal_u8 *frame_ie = ies;

    if (frame_ie == NULL) {
        return NULL;
    }

    /* 查找MBO Attribute，如果不是直接找下一个 */
    ie_len = len;
    while (ie_len > MAC_MBO_ATTRIBUTE_HDR_LEN && frame_ie[0] != eid) {
        ie_len -= (osal_s32)(frame_ie[1] + MAC_MBO_ATTRIBUTE_HDR_LEN);
        frame_ie += frame_ie[1] + MAC_MBO_ATTRIBUTE_HDR_LEN;
    }

    /* 查找到MBO Attribute，剩余长度不匹配直接返回空指针 */
    if ((ie_len < MAC_MBO_ATTRIBUTE_HDR_LEN) || (ie_len < (MAC_MBO_ATTRIBUTE_HDR_LEN + frame_ie[1]))) {
        return NULL;
    }

    return frame_ie;
}

OAL_STATIC osal_void hmac_handle_ie_specific_mbo(osal_u8 *ie_data,
    hmac_neighbor_bss_info_stru *bss_list_alloc, osal_u8 bss_list_index)
{
    unref_param(bss_list_alloc);

    if (memcpy_s((hmac_mbo_get_assoc_delay_ie())[bss_list_index].oui,
        sizeof((hmac_mbo_get_assoc_delay_ie())[bss_list_index].oui),
        ie_data + MAC_IE_HDR_LEN, MAC_MBO_OUI_LENGTH) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_handle_ie_by_data_length::memcpy oui fail!");
        return;
    }
    if ((ie_data[5] == MAC_MBO_IE_OUI_TYPE) && /* 5 MBO OUI TYPE offset */
        (ie_data[6] == MBO_ATTR_ID_ASSOC_RETRY_DELAY)) { /* 6 MBO ASSOC RETRY DELAY ATTR offset */
        (hmac_mbo_get_assoc_delay_ie())[bss_list_index].reassoc_delay_time =
            (((osal_u16)ie_data[9]) << BIT_OFFSET_8) | (ie_data[8]);  /* 8,9 MBO reassoc delay value offset */
    }
    return;
}

/*
 * 函 数 名  : hmac_mbo_check_is_assoc_or_re_assoc_allowed
 * 功能描述  : 携带Association Retry Delay Attribute或Association Disallowed Attribute MBO IE的AP，
 *             与这样的AP关联时需要判断是否满足不允许关联，或在re-assoc delay time时间内不可以关联
 */
OAL_STATIC osal_u32 hmac_mbo_check_is_assoc_or_re_assoc_allowed(hmac_vap_stru *hmac_vap,
    mac_conn_param_stru *connect_param, mac_bss_dscr_stru *bss_dscr)
{
    osal_u32 current_time;

    if (!is_legacy_sta(hmac_vap)) {
        return OAL_CONTINUE;
    }

    /* 先判断此bss是不是含有不允许关联MBO IE的BSS，如果是则返回false */
    if (bss_dscr->bss_assoc_disallowed == OSAL_TRUE) {
        oam_warning_log4(hmac_vap->vap_id, OAM_SF_ANY,
            "{hmac_mbo_check_is_assoc_or_re_assoc_allowed::bssid:%02x:%02x:%02x:%02x:xx:xx mbo assoc disallowed}",
            connect_param->bssid[MAC_ADDR_0], connect_param->bssid[MAC_ADDR_1],
            connect_param->bssid[MAC_ADDR_2], connect_param->bssid[MAC_ADDR_3]);
        return OAL_FAIL;
    }

    /* re-assoc delay不允许关联标标记为0，return true */
    if (hmac_mbo_get_param_info(hmac_vap->vap_id)->disable_connect == OSAL_FALSE) {
        return OAL_CONTINUE;
    }

    /* 判断此BSS是否是re-assoc delay bss，如果是进行相关处理 */
    if (memcmp(hmac_mbo_get_param_info(hmac_vap->vap_id)->reassoc_delay_bss_mac_addr,
        connect_param->bssid, WLAN_MAC_ADDR_LEN) == 0) {
        current_time = (osal_u32)oal_time_get_stamp_ms(); /* 获取当前时间 */
        /* 在收到btm req帧的ul_re_assoc_delay_time内不允许给该bss发重关联请求帧 */
        if ((current_time - hmac_mbo_get_param_info(hmac_vap->vap_id)->btm_req_received_time) <=
             hmac_mbo_get_param_info(hmac_vap->vap_id)->reassoc_delay_time) {
            oam_warning_log3(0, OAM_SF_ANY, "hmac_mbo_check_is_assoc_or_re_assoc_allowed::current time[%u] < \
                             (btm recv time[%u] + reassoc_delay time[%d]) not timed out!bss connect disallowed!",
                             current_time, hmac_mbo_get_param_info(hmac_vap->vap_id)->btm_req_received_time,
                             hmac_mbo_get_param_info(hmac_vap->vap_id)->reassoc_delay_time);
            return OAL_FAIL;
        }

        /* 已超时，不允许关联标志位清零 */
        hmac_mbo_get_param_info(hmac_vap->vap_id)->disable_connect = OSAL_FALSE;
    }

    return OAL_CONTINUE;
}

/* 功能描述  : 判断AP是否携带MBO IE(Association Disallowed Attribute) */
OAL_STATIC osal_void hmac_scan_update_bss_assoc_disallowed_attr(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
    osal_u8 *frame_body, osal_u16 frame_len)
{
    osal_u8 *mbo_ie = NULL;
    osal_u8 *mbo_attribute = NULL;

    unref_param(hmac_vap);

    mbo_ie = hmac_find_vendor_ie_etc(MAC_MBO_VENDOR_IE, MAC_MBO_IE_OUI_TYPE, frame_body, frame_len);
    if (mbo_ie == NULL) {
        bss_dscr->bss_assoc_disallowed = OSAL_FALSE;
        return;
    }
    if (mbo_ie[1] < (MBO_IE_HEADER - MAC_IE_HDR_LEN)) {
        return;
    }
    mbo_attribute = mac_find_mbo_attribute(MBO_ATTR_ID_ASSOC_DISALLOW,
                                           mbo_ie + MBO_IE_HEADER,
                                           mbo_ie[1] - (MBO_IE_HEADER - MAC_IE_HDR_LEN));
    if (mbo_attribute == NULL) {
        /* 不带有Association Disallowed Attribute，允许关联 */
        bss_dscr->bss_assoc_disallowed = OSAL_FALSE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_scan_update_bss_assoc_disallowed_attr::Assoc Disallowed Attr found!}");
        /* 带有Association Disallowed Attribute,AP 不允许被关联 */
        bss_dscr->bss_assoc_disallowed = OSAL_TRUE;
    }

    return;
}

/*
 * 功能描述  : 将btm req的Neighbor元素中的re-assoc delay相关参数保存在MBO结构体下
 */
OAL_STATIC osal_void hmac_set_bss_re_assoc_delay_params(const hmac_bsst_req_info_stru *bsst_req_info,
    const hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 need_roam)
{
    osal_u8 bss_index_loop;

    if (!is_legacy_sta(hmac_vap) || need_roam == OSAL_FALSE) {
        return;
    }

    for (bss_index_loop = 0; bss_index_loop < bsst_req_info->bss_list_num; bss_index_loop++) {
        if (memcmp(hmac_user->user_mac_addr, bsst_req_info->neighbor_bss_list[bss_index_loop].auc_mac_addr,
            WLAN_MAC_ADDR_LEN) == 0) {
            hmac_mbo_get_param_info(hmac_vap->vap_id)->reassoc_delay_time = HMAC_11V_MBO_RE_ASSOC_DALAY_TIME_S_TO_MS *
                (hmac_mbo_get_assoc_delay_ie())[bss_index_loop].reassoc_delay_time;
            hmac_mbo_get_param_info(hmac_vap->vap_id)->btm_req_received_time = (osal_u32)oal_time_get_stamp_ms();
            hmac_mbo_get_param_info(hmac_vap->vap_id)->disable_connect = OSAL_TRUE;

            (osal_void)memcpy_s(hmac_mbo_get_param_info(hmac_vap->vap_id)->reassoc_delay_bss_mac_addr,
                WLAN_MAC_ADDR_LEN, hmac_user->user_mac_addr, WLAN_MAC_ADDR_LEN);
            break;
        }
    }
    return;
}

OAL_STATIC osal_void hmac_roam_attach_mbo_ie_auth(hmac_vap_stru *hmac_vap,
    osal_u8 *mgmt_frame, osal_u16 *auth_len, mac_bss_dscr_stru *bss_dscr)
{
    osal_u8 *mgmt_frame_tail = mgmt_frame + *auth_len;
    osal_u8 *payload = OSAL_NULL;
    osal_s32 payload_len = 0;
    osal_u8 *vendor_ie = OSAL_NULL;

    if (bss_dscr == OSAL_NULL || !is_legacy_sta(hmac_vap)) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_roam_attach_mbo_ie_auth::bss_dscr is null.}");
        return;
    }

    payload = bss_dscr->mgmt_buff + MAC_80211_FRAME_LEN;
    payload_len = (osal_s32)(bss_dscr->mgmt_len - MAC_80211_FRAME_LEN);
    vendor_ie = mac_get_mbo_ie(payload, payload_len);
    if (vendor_ie == OSAL_NULL || (*auth_len + *(vendor_ie + 1)) > WLAN_MEM_NETBUF_SIZE2) {
        return;
    }

    mac_set_mbo_ie(hmac_vap, mgmt_frame_tail);
    *auth_len += 9; // 9: mbo ie fix len
    oam_info_log0(0, OAM_SF_ANY, "hmac_roam_attach_mbo_ie_auth::add mbo ie succ!");
}

/*
 * 函 数 名  : hmac_vap_mbo_update_rrm_scan_params
 * 功能描述  :  解决mbo认证扫描不全的问题: 每个信道停留40ms,发送两次probe req
 */
OAL_STATIC osal_void hmac_vap_mbo_update_rrm_scan_params(const mac_bcn_req_stru *bcn_req, mac_scan_req_stru *scan_req)
{
    /* 解决mbo认证扫描不全的问题: 每个信道停留40ms,发送两次probe req */
    if (bcn_req->mode != RM_BCN_REQ_MEAS_MODE_ACTIVE) {
        return;
    }

    scan_req->scan_time = WLAN_LONG_ACTIVE_SCAN_TIME;
    scan_req->max_scan_count_per_channel = WLAN_DEFAULT_FG_SCAN_COUNT_PER_CHANNEL;
    scan_req->max_send_probe_req_count_per_channel = WLAN_DEFAULT_FG_SCAN_COUNT_PER_CHANNEL;
}

OAL_STATIC osal_void hmac_mbo_modify_chan_param(osal_u8 optclass, wlan_channel_band_enum_uint8 *chan_band,
    osal_u8 *chan_count)
{
    /* 对MBO Beacon Report用例进行打桩,MBO AP请求beacon request的operating class为81(2.4G)或115(5G) */
    if (optclass == MAC_GLOBAL_OPERATING_CLASS_NUMBER81) { /* operating class 81 对应信道集1-13 */
        *chan_band = WLAN_BAND_2G;
        *chan_count = (osal_u8)MAC_CHANNEL_FREQ_2_BUTT;
    } else if (optclass == MAC_GLOBAL_OPERATING_CLASS_NUMBER115) {
        /* operating class 115对应信道集36 40 44 48 */
        *chan_band = WLAN_BAND_5G;
        *chan_count = MAC_GLOBAL_OPERATING_CLASS_115_CH_NUMS;
    }
}

OAL_STATIC osal_void hmac_mbo_scan_add_mbo_ie(hmac_vap_stru *hmac_vap, osal_u8 *app_ie, osal_u8 *ie_len)
{
    *ie_len = 0;

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    mac_set_mbo_ie(hmac_vap, app_ie);
    *ie_len = MAX_STA_MBO_IE_LEN;
}

OAL_STATIC osal_void hmac_mbo_assoc_add_mbo_and_soc_ie(hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len,
    hmac_scanned_bss_info  *scaned_bss)
{
    osal_u8 len = 0;

    *ie_len = 0;

    if (!is_legacy_sta(hmac_vap)) {
        return;
    }

    mac_set_mbo_ie(hmac_vap, buffer);
    *ie_len += MAX_STA_MBO_IE_LEN;

    mac_set_supported_operating_classes_ie(hmac_vap, buffer + *ie_len, &len, scaned_bss);
    *ie_len += len;
}

OAL_STATIC osal_bool hmac_mbo_sta_get_bss_list(osal_void *notify_data)
{
    osal_u8 bss_num = *(osal_u8 *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;

    if (g_assoc_delay_attr_mbo_ie != OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "vap_id[%d] hmac_mbo_sta_get_bss_list mem already malloc!");
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, bss_num * sizeof(mac_assoc_retry_delay_attr_mbo_ie_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "vap_id[%d] hmac_mbo_sta_get_bss_list malloc null!");
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, bss_num * sizeof(mac_assoc_retry_delay_attr_mbo_ie_stru),
        0, bss_num * sizeof(mac_assoc_retry_delay_attr_mbo_ie_stru));
    g_assoc_delay_attr_mbo_ie = mem_ptr;

    return OSAL_TRUE;
}

OAL_STATIC osal_bool hmac_mbo_sta_handled_bss_list(osal_void *notify_data)
{
    unref_param(notify_data);

    if (g_assoc_delay_attr_mbo_ie == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "vap_id[%d] hmac_mbo_sta_handled_bss_list mem already free!");
        return OSAL_TRUE;
    }
    oal_mem_free(g_assoc_delay_attr_mbo_ie, OAL_TRUE);
    g_assoc_delay_attr_mbo_ie = OSAL_NULL;

    return OSAL_TRUE;
}

OAL_STATIC osal_bool hmac_mbo_sta_vap_add(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;

    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    if (g_sta_mbo_para_info[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_mbo_sta_vap_add mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_mbo_para_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_mbo_sta_vap_add malloc null!", vap_id);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(mac_mbo_para_stru), 0, sizeof(mac_mbo_para_stru));
    g_sta_mbo_para_info[vap_id] = mem_ptr;
    g_sta_mbo_para_info[vap_id]->mbo_cell_capa = 0x3; // 0x3: not support cell

    return OSAL_TRUE;
}

OAL_STATIC osal_bool hmac_mbo_sta_vap_del(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_u8 vap_id = hmac_vap->vap_id;

    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    if (g_sta_mbo_para_info[vap_id] == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_mbo_sta_vap_del mem already free!", vap_id);
        return OSAL_TRUE;
    }
    oal_mem_free(g_sta_mbo_para_info[hmac_vap->vap_id], OAL_TRUE);
    g_sta_mbo_para_info[vap_id] = OSAL_NULL;

    return OSAL_TRUE;
}

osal_u32 hmac_mbo_sta_init(osal_void)
{
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_mbo_sta_vap_add);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_mbo_sta_vap_del);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_11V_NEW_BSS_LIST, hmac_mbo_sta_get_bss_list);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_11V_FREE_BSS_LIST, hmac_mbo_sta_handled_bss_list);

    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_ASSOC_ADD_MBO_SOC_IE, hmac_mbo_assoc_add_mbo_and_soc_ie);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_HANDLE_MBO_IE, hmac_handle_ie_specific_mbo);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_CHECK_IS_ASSOC_ALLOWED, hmac_mbo_check_is_assoc_or_re_assoc_allowed);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_UPDATE_IS_ASSOC_ALLOWED, hmac_scan_update_bss_assoc_disallowed_attr);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_SET_DELAY_PARAM, hmac_set_bss_re_assoc_delay_params);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_ATTACH_MBO_IE_AUTH, hmac_roam_attach_mbo_ie_auth);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_UPDATE_RRM_SCAN_PARAM, hmac_vap_mbo_update_rrm_scan_params);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_MODIFY_CHAN_PARAM, hmac_mbo_modify_chan_param);
    hmac_feature_hook_register(HMAC_FHOOK_MBO_STA_SCAN_ADD_MBO_IE, hmac_mbo_scan_add_mbo_ie);

    return OAL_SUCC;
}

osal_void hmac_mbo_sta_deinit(osal_void)
{
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_ADD_VAP, hmac_mbo_sta_vap_add);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_mbo_sta_vap_del);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_11V_NEW_BSS_LIST, hmac_mbo_sta_get_bss_list);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_11V_FREE_BSS_LIST, hmac_mbo_sta_handled_bss_list);

    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_ASSOC_ADD_MBO_SOC_IE);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_HANDLE_MBO_IE);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_CHECK_IS_ASSOC_ALLOWED);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_UPDATE_IS_ASSOC_ALLOWED);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_SET_DELAY_PARAM);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_ATTACH_MBO_IE_AUTH);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_UPDATE_RRM_SCAN_PARAM);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_MODIFY_CHAN_PARAM);
    hmac_feature_hook_unregister(HMAC_FHOOK_MBO_STA_SCAN_ADD_MBO_IE);

    return;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
