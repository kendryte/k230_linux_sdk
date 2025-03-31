/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: wps feature file
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_wps.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_WPS_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_config_set_wps_p2p_ie_etc
 功能描述  : mp02 设置WPS/P2P 信息元素
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_set_wps_p2p_ie_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_w2h_app_ie_stru            *w2h_wps_p2p_ie;
    oal_app_ie_stru                 app_ie;
    osal_u32                      ul_ret;

    w2h_wps_p2p_ie = (oal_w2h_app_ie_stru *)msg->data;

    if ((w2h_wps_p2p_ie->app_ie_type >= OAL_APP_IE_NUM) ||
        (w2h_wps_p2p_ie->ie_len >= WLAN_WPS_IE_MAX_SIZE)) {
        oam_warning_log3(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_set_wps_p2p_ie_etc::app_ie_type=[%d] app_ie_len=[%d],param invalid.}",
                         hmac_vap->vap_id,
                         w2h_wps_p2p_ie->app_ie_type, w2h_wps_p2p_ie->ie_len);
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_wps_p2p_ie_etc::p2p_ie_type=[%d], p2p_ie_len=[%d].}",
                     hmac_vap->vap_id,
                     w2h_wps_p2p_ie->app_ie_type, w2h_wps_p2p_ie->ie_len);

    memset_s(&app_ie, OAL_SIZEOF(app_ie), 0, OAL_SIZEOF(app_ie));
    app_ie.app_ie_type = w2h_wps_p2p_ie->app_ie_type;
    app_ie.ie_len      = w2h_wps_p2p_ie->ie_len;
    if (memcpy_s(app_ie.ie, sizeof(app_ie.ie), w2h_wps_p2p_ie->data_ie, app_ie.ie_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_wps_p2p_ie_etc::memcpy_s error}");
    }

    /* 设置WPS/P2P 信息 */
    ul_ret = hmac_config_set_app_ie_to_vap_etc(hmac_vap, &app_ie, app_ie.app_ie_type);
    if (ul_ret != OAL_SUCC) {
        return (osal_s32)ul_ret;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if (app_ie.app_ie_type == OAL_APP_BEACON_IE) {
        if ((app_ie.ie_len != 0) &&
            hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
            app_ie.ie, (osal_s32)(app_ie.ie_len)) != OAL_PTR_NULL) {
            /* 设置WPS 功能使能 */
            mac_mib_set_WPSActive(hmac_vap, OAL_TRUE);
            oam_info_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_wps_p2p_ie_etc::set wps enable.}",
                          hmac_vap->vap_id);
        } else {
            mac_mib_set_WPSActive(hmac_vap, OAL_FALSE);
        }
    }

    return (osal_s32)ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_wps_ie_etc
 功能描述  : AP 设置WPS 信息元素
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_set_wps_ie_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_app_ie_stru                *wps_ie;
    osal_u8                      *ie;
    osal_u32                      ret;

    wps_ie = (oal_app_ie_stru *)msg->data;

    /* 设置WPS 信息 */
    ret = hmac_config_set_app_ie_to_vap_etc(hmac_vap, wps_ie, wps_ie->app_ie_type);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_wps_ie_etc::ret=[%d].}", hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }

    /* 检测beacon 信息中是否有WPS 信息元素 */
    if ((wps_ie->app_ie_type == OAL_APP_BEACON_IE) && (wps_ie->ie_len != 0)) {
        ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS,
            wps_ie->ie, (osal_s32)(wps_ie->ie_len));
        if (ie != OAL_PTR_NULL) {
            /* 设置WPS 功能使能 */
            mac_mib_set_WPSActive(hmac_vap, OAL_TRUE);
            oam_info_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_wps_ie_etc::set wps enable.}",
                          hmac_vap->vap_id);
        }
    } else if ((wps_ie->ie_len == 0) && (wps_ie->app_ie_type == OAL_APP_BEACON_IE)) {
        mac_mib_set_WPSActive(hmac_vap, OAL_FALSE);
        oam_info_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_wps_ie_etc::set wps disable.}",
                      hmac_vap->vap_id);
    }

    return (osal_s32)ret;
}

/*****************************************************************************
 功能描述 : 查找WPS attribute信息元素
*****************************************************************************/
OAL_STATIC osal_u8 *hmac_find_wps_attribute_etc(osal_u16 eid, osal_u8 *ies, osal_s32 len)
{
    osal_s32 ie_len;

    if (ies == OSAL_NULL) {
        return OSAL_NULL;
    }

    /* 查找WPS attribute, 如果不是直接找下一个, 属性中 eid 两个字节, len 两个字节 */
    while (len > MAC_WPS_ATTRIBUTE_HDR_LEN && ((ies[0] << 8) + ies[1]) != eid) { /* 8: 1 Byte; 小端, 0,1 字节 */
        ie_len = (osal_s32)((ies[2] << 8) + ies[3]); /* 8: 1 Byte; 小端, 2,3 字节 */
        len -= ie_len + MAC_WPS_ATTRIBUTE_HDR_LEN;
        ies += ie_len + MAC_WPS_ATTRIBUTE_HDR_LEN;
    }

    /* 查找到WPS attribute，剩余长度不匹配直接返回空指针 */
    if ((len <= MAC_WPS_ATTRIBUTE_HDR_LEN) ||
        (len < (MAC_WPS_ATTRIBUTE_HDR_LEN + (osal_s32)((ies[2] << 8) + ies[3]))) ||  /* 8: 1 Byte; 小端, 2,3 字节 */
        ((len == MAC_WPS_ATTRIBUTE_HDR_LEN) && (((ies[0] << 8) + ies[1]) != eid))) { /* 8: 1 Byte; 小端, 0,1 字节 */
        return OSAL_NULL;
    }

    return ies;
}

OAL_STATIC osal_u32 hmac_scan_check_wps_status(hmac_vap_stru *hmac_vap, osal_u32 len, osal_u8 *ie, osal_u8 *type)
{
    osal_u8 *wps_ie = OAL_PTR_NULL;
    osal_u8 *request_type_attr = OAL_PTR_NULL;

    /* 查找WPS IE信息 */
    wps_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, ie, (osal_s32)len);
    if ((wps_ie == OSAL_NULL) || (wps_ie[1] < MAC_MIN_WPS_IE_LEN)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_scan_check_wps_status::wps_ie len %d invalid.}",
                         hmac_vap->vap_id, (wps_ie == OSAL_NULL) ? 0xff : wps_ie[1]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 查找WPS request type attribute 信息 */
    request_type_attr = hmac_find_wps_attribute_etc(MAC_WPS_ATTRIBUTE_REQUEST_TYPE,
        wps_ie + WPS_IE_HDR_LEN, (wps_ie[1] - 4)); /* 跳过4个字节, MAC_MIN_WPS_IE_LEN 是否该定义为4? */
    if (request_type_attr == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_scan_check_wps_status::not find request type, wps_ie[1] %d.}",
            hmac_vap->vap_id, wps_ie[1]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* request_type_attr 长度校验, 小端, 2进制高位左移8位切换为10进制, 2,3下标 */
    if (((request_type_attr[2] << 8) + request_type_attr[3]) != MAC_WPS_REQUEST_TYPE_ATTR_LEN) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_scan_check_wps_status::invalid request type ie len[%d].}",
            /* 2进制高位左移8位切换为10进制, 2,3下标 */
            hmac_vap->vap_id, (osal_s32)((request_type_attr[2] << 8) + request_type_attr[3]));
        return OAL_FAIL;
    }

    /* 获取 request_type_attr 信息 */
    *type = request_type_attr[4]; /* 4 下标 */
    oam_info_log3(0, OAM_SF_CFG, "vap[%d]hmac_scan_check_wps_status:type[%d] len[%d]",
        hmac_vap->vap_id, *type, wps_ie[1]);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_del_ie
 功能描述  : 删除wpa_supplicant 下发的IE 中的指定 IE
 输入参数  :      osal_u8 ie_type      需要删除的ie类型
             osal_u8 *ie_buf       wpa_supplicant 下发的ie
             osal_u32 *ie_buf_len  wpa_supplicant 下发的ie 长度
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_config_del_ie(osal_u8 ie_type, osal_u8 *ie_buf, osal_u32 *ie_buf_len)
{
    osal_u8 *del_ie_buf = OSAL_NULL;
    osal_u8 *ie_buf_end = OSAL_NULL;
    osal_u8 *del_ie_end = OSAL_NULL;
    osal_u32 i, del_ie_buf_len;

    mac_ie_oui_stru mac_wlan_ie_oui[] = {
        {MAC_P2P_IE, MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P},
        {MAC_WPS_IE, MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS}
    };

    if ((ie_buf == OSAL_NULL) || (ie_buf_len == OSAL_NULL) || (*ie_buf_len == 0)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_del_ie::param error.}");
        return;
    }

    for (i = 0; i < osal_array_size(mac_wlan_ie_oui); i++) {
        if (mac_wlan_ie_oui[i].ie_type == ie_type) {
            break;
        }
    }

    if (i == osal_array_size(mac_wlan_ie_oui)) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_del_ie::not find the ie[%d].}", ie_type);
        return;
    }

    del_ie_buf = hmac_find_vendor_ie_etc(mac_wlan_ie_oui[i].oui, (osal_u8)mac_wlan_ie_oui[i].oui_type,
        (osal_u8 *)ie_buf, (osal_s32)(*ie_buf_len));
    if (del_ie_buf == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_del_ie::not find the ie[%d] in ie buf.}", ie_type);
        return;
    }

    del_ie_buf_len = del_ie_buf[1] + MAC_IE_HDR_LEN;

    /* 将del ie后面的内容拷贝到del ie现所在位置 */
    ie_buf_end = (ie_buf + *ie_buf_len);
    del_ie_end = (del_ie_buf + del_ie_buf_len);
    if (ie_buf_end < del_ie_end) {
        return;
    }

    if (ie_buf_end > del_ie_end) {
        errno_t ret;
        ret = memmove_s(del_ie_buf, (osal_u32)(ie_buf_end - del_ie_end),
            del_ie_end, (osal_u32)(ie_buf_end - del_ie_end));
        if (ret != EOK) {
            oam_error_log1(0, OAM_SF_CFG, "{hmac_config_del_ie::memmove fail[%d].", ret);
            return;
        }
    }
    *ie_buf_len -= del_ie_buf_len;
}

OAL_STATIC osal_void hmac_scan_del_wps_ie(hmac_vap_stru *hmac_vap, osal_u32 *len, osal_u8 *ie, osal_u8 del_wps_ie)
{
    osal_u8 request_type = 0xff;

#ifdef _PRE_WLAN_FEATURE_P2P
    hmac_config_del_ie(MAC_P2P_IE, ie, len);
#endif

    /* wfa认证要求sta 扫描probe req报文携带wps ie */
    if (del_wps_ie == 0) {
        return;
    }

    if ((hmac_scan_check_wps_status(hmac_vap, *len, ie, &request_type) == OAL_SUCC) && (request_type == 0)) {
        hmac_config_del_ie(MAC_WPS_IE, ie, len);
    }
}

osal_u32 hmac_wps_init(osal_void)
{
    /* 消息注册 */
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_WPS_IE, hmac_config_set_wps_ie_etc);
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_WPS_P2P_IE, hmac_config_set_wps_p2p_ie_etc);

    /* 对外接口注册 */
    hmac_feature_hook_register(HMAC_FHOOK_SCAN_DEL_WPS_IE, hmac_scan_del_wps_ie);

    return OAL_SUCC;
}

osal_void hmac_wps_deinit(osal_void)
{
    /* 消息去注册 */
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_WPS_IE);
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_WPS_P2P_IE);

    /* 对外接口去注册 */
    hmac_feature_hook_unregister(HMAC_FHOOK_SCAN_DEL_WPS_IE);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
