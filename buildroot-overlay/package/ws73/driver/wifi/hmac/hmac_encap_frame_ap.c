/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: AP模式组帧文件，AP模式特有帧的组帧.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_encap_frame_ap.h"
#if defined (_PRE_OS_VERSION_LINUX) && defined (_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "oal_cfg80211.h"
#endif
#include "oal_netbuf_ext.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "mac_user_ext.h"
#include "mac_vap_ext.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"
#include "hmac_mgmt_ap.h"
#include "hmac_11i.h"
#include "hmac_blockack.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_blacklist.h"

#include "hmac_dfx.h"
#include "hmac_feature_interface.h"
#include "hmac_11k.h"
#include "hmac_tx_amsdu.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ENCAP_FRAME_AP_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
 函 数 名  : mac_get_assoc_comeback_time
 功能描述  : 计算得到 assoc rsp 中所需的assoc comeback time
 输入参数  : hmac_vap   : mac vap 指针
             hmac_user : hamc user 指针
 输出参数  : 无
 返 回 值  : 计算得到的assoc comeback time值
*****************************************************************************/
OAL_STATIC osal_u32 hmac_get_assoc_comeback_time(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u32                     timeout;

    osal_u32                     now_time;
    osal_u32                     passed_time;
    osal_u32                     sa_query_maxtimeout;

    /* 获取现在时间 */
    now_time = (osal_u32)oal_time_get_stamp_ms();

    /* 设置ASSOCIATION_COMEBACK_TIME，使STA在AP完成SA Query流程之后再发送关联请求 */
    /* 获得sa Query Max timeout值 */
    sa_query_maxtimeout = mac_mib_get_dot11AssociationSAQueryMaximumTimeout(hmac_vap);

    /* 是否现在有sa Query流程正在进行 */
    if ((hmac_user->sa_query_info.sa_query_count != 0) &&
        (now_time >= hmac_user->sa_query_info.sa_query_start_time)) {
        /* 待现有SA Query流程结束后才可以接受STA发过来的关联帧 */
        passed_time = now_time - hmac_user->sa_query_info.sa_query_start_time;
        timeout = sa_query_maxtimeout - passed_time;
    } else {
        /* 给接下来的SA Query流程预留时间 */
        timeout = sa_query_maxtimeout;
    }

    return timeout;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_set_supported_rates_ie_asoc_rsp
 功能描述  : 设置速率集
 输入参数  : macuser: 指向user
             buffer: 指向buffer
 输出参数  : ie_len: element的长度
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_set_supported_rates_ie_asoc_rsp(hmac_user_stru *hmac_user, osal_u8 *buffer,
    osal_u8 *ie_len)
{
    osal_u8 nrates;
    osal_u8 idx;
    /**************************************************************************
                        ---------------------------------------
                        |Element ID | Length | Supported Rates|
                        ---------------------------------------
             Octets:    |1          | 1      | 1~8            |
                        ---------------------------------------
    The Information field is encoded as 1 to 8 octets, where each octet describes a single Supported
    Rate or BSS membership selector.
    **************************************************************************/
    buffer[0] = MAC_EID_RATES;
    /* 获取协商的速率个数 */
    nrates = hmac_user->avail_op_rates.rs_nrates;

    if (nrates > MAC_MAX_SUPRATES) {
        nrates = MAC_MAX_SUPRATES;
    }

    for (idx = 0; idx < nrates; idx++) {
        /* 获取协商的速率集 */
        buffer[MAC_IE_HDR_LEN + idx] = hmac_user->avail_op_rates.rs_rates[idx];
    }

    buffer[1] = nrates;
    *ie_len = MAC_IE_HDR_LEN + nrates;
}

/*****************************************************************************
 函 数 名  : hmac_set_exsup_rates_ie_asoc_rsp
 功能描述  : 设置速率集
 输入参数  : macuser: 指向user
             buffer: 指向buffer
 输出参数  : ie_len: element的长度
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_set_exsup_rates_ie_asoc_rsp(hmac_user_stru *hmac_user, osal_u8 *buffer,
    osal_u8 *ie_len)
{
    osal_u8 nrates;
    osal_u8 idx;

    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/

    if (hmac_user->avail_op_rates.rs_nrates <= MAC_MAX_SUPRATES) {
        *ie_len = 0;
        return;
    }

    buffer[0] = MAC_EID_XRATES;
    nrates = hmac_user->avail_op_rates.rs_nrates - MAC_MAX_SUPRATES;
    buffer[1] = nrates;

    for (idx = 0; idx < nrates; idx++) {
        buffer[MAC_IE_HDR_LEN + idx] = hmac_user->avail_op_rates.rs_rates[idx + MAC_MAX_SUPRATES];
    }

    *ie_len = MAC_IE_HDR_LEN + nrates;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
OAL_STATIC osal_void hmac_set_owe_ie_asoc_rsp(hmac_user_stru *hmac_user, osal_u8 *asoc_rsp_original,
    osal_u8 *buffer, osal_u8 *ie_len)
{
    osal_u32 current_asoc_len = 0;
    *ie_len = 0;

    if ((hmac_user->hmac_cap_info.owe != OAL_TRUE) || (hmac_user->owe_ie == OSAL_NULL) ||
        (hmac_user->owe_ie_len == 0)) {
        return;
    }

    current_asoc_len = buffer - asoc_rsp_original;
    if ((current_asoc_len > WLAN_MEM_NETBUF_SIZE2) ||
        (WLAN_MEM_NETBUF_SIZE2 - current_asoc_len < hmac_user->owe_ie_len)) {
        return;
    }
    if (memcpy_s(buffer, hmac_user->owe_ie_len, hmac_user->owe_ie, hmac_user->owe_ie_len) != EOK) {
        return;
    }

    *ie_len = (osal_u8)hmac_user->owe_ie_len;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_chtxt
 功能描述  : 封装chtxt
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_void hmac_mgmt_encap_chtxt(osal_u8 *frame,
    osal_u8 *chtxt, osal_u16 *auth_rsp_len, hmac_user_stru *hmac_user_sta)
{
    /* Challenge Text Element                  */
    /* --------------------------------------- */
    /* |Element ID | Length | Challenge Text | */
    /* --------------------------------------- */
    /* | 1         |1       |1 - 253         | */
    /* --------------------------------------- */
    frame[6] = MAC_EID_CHALLENGE;    /* 6 索引 */
    frame[7] = WLAN_CHTXT_SIZE;    /* 7 索引 */

    /* 将challenge text拷贝到帧体中去 */
    if (memcpy_s(&frame[8], WLAN_CHTXT_SIZE, chtxt, WLAN_CHTXT_SIZE) != EOK) { /* 8 索引 */
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_encap_chtxt::memcpy_s error}");
    }

    /* 认证帧长度增加Challenge Text Element的长度 */
    *auth_rsp_len += (WLAN_CHTXT_SIZE + MAC_IE_HDR_LEN);
    if (hmac_user_sta->ch_text == OSAL_NULL) {
        hmac_user_sta->ch_text = (osal_u8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WLAN_CHTXT_SIZE, OAL_TRUE);
        if (hmac_user_sta->ch_text == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_encap_chtxt::oal_mem_alloc failed}");
            return;
        }
    }
    /* 保存明文的challenge text, 8 索引 */
    if (memcpy_s(hmac_user_sta->ch_text, WLAN_CHTXT_SIZE, &frame[8], WLAN_CHTXT_SIZE) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_encap_chtxt::memcpy_s error}");
    }
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_asoc_rsp_ap_etc
 功能描述  : 组关联响应帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_mgmt_encap_asoc_rsp_ap_etc(hmac_vap_stru *hmac_vap, osal_u8 *sta_addr, hmac_user_stru *hmac_user,
    mac_status_code_enum_uint16 status_code, osal_u8 *asoc_rsp)
{
    osal_u32 asoc_rsp_len = 0;
    osal_u8 ie_len = 0;
    osal_u32 timeout = 0;
    hmac_device_stru *hmac_device;
    osal_u8 *asoc_rsp_original;
    mac_timeout_interval_type_enum tie_type = (status_code == MAC_REJECT_TEMP) ? MAC_TIE_ASSOCIATION_COMEBACK_TIME :
        MAC_TIE_BUTT;
    osal_u16 type = 0;
    osal_u16 app_ie_len;
    osal_void *fhook = OAL_PTR_NULL;

    if (asoc_rsp == OAL_PTR_NULL || hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_rsp_ap_etc::null param.}");
        return asoc_rsp_len;
    }

    type = (hmac_user->mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_REQ) ? WLAN_FC0_SUBTYPE_ASSOC_RSP :
        WLAN_FC0_SUBTYPE_REASSOC_RSP;
    /* 保存起始地址，方便计算长度 */
    asoc_rsp_original = asoc_rsp;

    /* 获取device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_mgmt_encap_asoc_rsp_ap_etc::hmac_device is a null ptr.}", hmac_vap->vap_id);
        return asoc_rsp_len;
    }

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

    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(asoc_rsp, type);

    /* 设置 DA address1: STA MAC地址 */
    oal_set_mac_addr(asoc_rsp + WLAN_HDR_ADDR1_OFFSET, sta_addr);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(asoc_rsp + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(asoc_rsp + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(asoc_rsp, 0);

    asoc_rsp += MAC_80211_FRAME_LEN;

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Association Response Frame - Frame Body                  */
    /* --------------------------------------------------------------------- */
    /* | Capability Information |   Status Code   | AID | Supported  Rates | */
    /* --------------------------------------------------------------------- */
    /* |2                       |2                |2    |3-10              | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* 设置 capability information field */
    hmac_set_cap_info_ap_etc((osal_void *)hmac_vap, asoc_rsp);
    asoc_rsp += MAC_CAP_INFO_LEN;

    /* 设置 Status Code */
    hmac_set_status_code_ie_etc(asoc_rsp, status_code);
    asoc_rsp += MAC_STATUS_CODE_LEN;

    /* 设置 Association ID */
    hmac_set_aid_ie_etc(asoc_rsp, hmac_user->assoc_id);
    asoc_rsp += MAC_AID_LEN;

#ifdef _PRE_WIFI_DMT
    /* 设置 Supported Rates IE */
    hmac_set_supported_rates_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
    asoc_rsp += ie_len;

    /* 设置 Extended Supported Rates IE */
    hmac_set_exsup_rates_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
    asoc_rsp += ie_len;
#else
    /* 设置 Supported Rates IE */
    hmac_set_supported_rates_ie_asoc_rsp(hmac_user, asoc_rsp, &ie_len);
    asoc_rsp += ie_len;

    /* 5G不需要设置extended supported rates字段 */
#ifdef _PRE_WLAN_FEATURE_WPA3
    if ((hmac_vap->channel.band == WLAN_BAND_2G) || (hmac_vap->sae_pwe == NL80211_SAE_PWE_HASH_TO_ELEMENT) ||
        (hmac_vap->sae_pwe == NL80211_SAE_PWE_BOTH)) {
#else
     if (hmac_vap->channel.band == WLAN_BAND_2G) {
#endif
        /* 设置 Extended Supported Rates IE */
        hmac_set_exsup_rates_ie_asoc_rsp(hmac_user, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }
#endif

    /* 设置 EDCA IE */
    hmac_set_wmm_params_ie_etc((osal_void *)hmac_vap, asoc_rsp, hmac_user->cap_info.qos, &ie_len);
    asoc_rsp += ie_len;

    /* 设置 Timeout Interval (Association Comeback time) IE */
#ifdef _PRE_WLAN_FEATURE_PMF
    timeout = hmac_get_assoc_comeback_time(hmac_vap, hmac_user);
#endif
    mac_set_timeout_interval_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len, tie_type, timeout);
    asoc_rsp += ie_len;

    if (hmac_user->ht_hdl.ht_capable == OAL_TRUE) {
        /* 设置 HT-Capabilities Information IE */
        hmac_set_ht_capabilities_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;

        /* 设置 HT-Operation Information IE */
        hmac_set_ht_opern_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;

        /* 设置 Overlapping BSS Scan Parameters Information IE */
        hmac_set_obss_scan_params_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;

        /* 设置 Extended Capabilities Information IE */
        hmac_set_ext_capabilities_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }

    if (hmac_user->vht_hdl.vht_capable == OAL_TRUE) {
        /* 设置 VHT Capabilities IE */
        hmac_set_vht_capabilities_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;

        /* 设置 VHT Opern IE */
        hmac_set_vht_opern_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        /* AP 160M 适配对端能力来设置VHT Oper字段 */

        asoc_rsp += ie_len;

#ifdef _PRE_WLAN_FEATURE_1024QAM
        hmac_set_1024qam_vendor_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
#endif
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (hmac_user->he_hdl.he_capable == OSAL_TRUE) {
        hmac_set_he_capabilities_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
        hmac_set_he_operation_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    if (hmac_user->vht_hdl.vht_capable == OAL_TRUE || hmac_user->he_hdl.he_capable == OSAL_TRUE) {
        hmac_set_opmode_notify_ie_etc((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }
#endif

    /* 填充WPS信息 */
    hmac_add_app_ie_etc((osal_void *)hmac_vap, asoc_rsp, &app_ie_len, OAL_APP_ASSOC_RSP_IE);
    asoc_rsp += app_ie_len;

    /* 填充 BCM Vendor VHT IE,解决与BCM STA的私有协议对通问题 */
    if (hmac_user->user_vendor_vht_capable == OAL_TRUE) {
        mac_set_vendor_vht_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }
    /* 5G时，BCM私有vendor ie中不携带vht,需按照此格式组帧 */
    if (hmac_user->user_vendor_novht_capable == OAL_TRUE) {
        mac_set_vendor_novht_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }

    /* 填充RRM CAP信息 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11K_SET_RRM_CAP);
    if (fhook != OSAL_NULL) {
        ((hmac_set_rrm_enabled_cap_field_etc_cb)fhook)((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }

    /* multi-sta特性下新增4地址ie */
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    mac_set_vender_4addr_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
    asoc_rsp += ie_len;
#endif

    /* wur特性下新增wur cap ie/wur oper ie */
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    if (hmac_vap->cap_flag.wur_enable) {
        mac_set_wur_capabilities_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
        mac_set_wur_operation_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_WPA3
    /* 避免重复添加RSNX */
    if (mac_find_ie_etc(MAC_EID_RSNX, hmac_vap->app_ie[OAL_APP_ASSOC_RSP_IE].ie,
        (osal_s32)hmac_vap->app_ie[OAL_APP_ASSOC_RSP_IE].ie_len) == OSAL_NULL) {
        hmac_set_rsnx_ie((osal_void *)hmac_vap, asoc_rsp, &ie_len);
        asoc_rsp += ie_len;
    }

    hmac_set_owe_ie_asoc_rsp(hmac_user, asoc_rsp_original, asoc_rsp, &ie_len);
    asoc_rsp += ie_len;
#endif

    asoc_rsp_len = (osal_u32)(asoc_rsp - asoc_rsp_original);

    return asoc_rsp_len;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_is_challenge_txt_equal
 功能描述  : 判断两个challenge txt是否相等
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC oal_bool_enum_uint8 hmac_mgmt_is_challenge_txt_equal(osal_u8 *data, const osal_u8 *chtxt)
{
    osal_u8  *ch_text     = 0;
    osal_u16  idx;
    osal_u8   ch_text_len;

    if (data == OAL_PTR_NULL || chtxt == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_is_challenge_txt_equal::param is NULL.}");
        return OAL_FALSE;
    }

    /* Challenge Text Element                  */
    /* --------------------------------------- */
    /* |Element ID | Length | Challenge Text | */
    /* --------------------------------------- */
    /* | 1         |1       |1 - 253         | */
    /* --------------------------------------- */
    ch_text_len = data[1];
    ch_text     = data + 2; /* 2 索引偏移 */

    for (idx = 0; idx < ch_text_len; idx++) {
        /* Return false on mismatch */
        if (ch_text[idx] != chtxt[idx]) {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

OAL_STATIC oal_err_code_enum hmac_encap_auth_rsp_get_user_idx_proc(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr,
    osal_u8 is_seq1, osal_u16 *user_index)
{
    osal_u32 ret;

    /* 在收到第一个认证帧时用户已创建 */
    if (is_seq1 == 0) {
        oam_warning_log1(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_encap_auth_rsp_get_user_idx_proc::user should have been added at seq1!}",
            hmac_vap->vap_id);
        return OAL_FAIL;
    }

    ret = hmac_user_add_etc(hmac_vap, mac_addr, user_index);
    if (ret != OAL_SUCC) {
        if (ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{hmac_encap_auth_rsp_get_user_idx_proc::hmac_vap_add_assoc_user_etc fail, users exceed config spec.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
        } else {
            oam_error_log1(0, OAM_SF_ANY,
                "{hmac_encap_auth_rsp_get_user_idx_proc::hmac_vap_add_assoc_user_etc fail,userid[%d]}", *user_index);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_encap_auth_rsp_get_user_idx_etc
 功能描述  : 获取user idx,如果用户不存在，且resend置位的话，将sta加入ap
 输入参数  : 1.vap指针
             2.sta的mac地址
             3.是否为seq1标志位.如果为真，表示如果用户不存在,需要将sta加入ap
 输出参数  : 1. auth_resend 用户存在的情况下收到seq1,seq1判定为重传帧，
                置位此标志
             2. pus_user_index 返回获取到的user idx

 返 回 值  :获取正常或者失败
*****************************************************************************/
OAL_STATIC oal_err_code_enum  hmac_encap_auth_rsp_get_user_idx_etc(hmac_vap_stru *hmac_vap,
    osal_u8 *mac_addr, osal_u8 is_seq1, osal_u8 *auth_resend, osal_u16 *pus_user_index)
{
    osal_u32 ret;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    hmac_vap_stru *hmac_other_vap = OAL_PTR_NULL;
    osal_u16 user_idx;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    *auth_resend = OAL_FALSE;
    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_addr, pus_user_index);
    /* 找到用户 */
    if (ret == OAL_SUCC) {
        /* 获取hmac用户的状态，如果不是0，说明是重复帧 */
        hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(*pus_user_index);
        if (hmac_user == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {mac_res_get_hmac_user_etc return null}", hmac_vap->vap_id);
            return OAL_FAIL;
        }
        /* user_asoc_state为枚举变量，取值为1~4，初始化为MAC_USER_STATE_BUTT，
         * 应该使用!=MAC_USER_STATE_BUTT作为判断，否则会导致WEP share加密关联不上问题
         */
        if (hmac_user->user_asoc_state != MAC_USER_STATE_BUTT) {
            *auth_resend = OAL_TRUE;
        }

        if (hmac_user->user_asoc_state == MAC_USER_STATE_ASSOC) {
            oal_net_device_stru  *net_device;

            net_device = hmac_vap_get_net_device_etc(hmac_vap->vap_id);
            if (net_device != OAL_PTR_NULL) {
#if defined(_PRE_PRODUCT_ID_HOST)
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
                oal_kobject_uevent_env_sta_leave_etc(net_device, mac_addr);
#endif
#endif
            }
        }
        return OAL_SUCC;
    }

    /* 若在同一chip下的其他VAP下找到给用户，删除之。否则导致业务不通。在DBAC下尤其常见 */
    if (hmac_device_find_user_by_macaddr_etc(mac_addr, &user_idx) == OAL_SUCC) {
        hmac_user = mac_res_get_hmac_user_etc(user_idx);
        if (hmac_user == OAL_PTR_NULL) {
            return hmac_encap_auth_rsp_get_user_idx_proc(hmac_vap, mac_addr, is_seq1, pus_user_index);
        }
        hmac_other_vap  = mac_res_get_hmac_vap(hmac_user->vap_id);
        if (hmac_other_vap != OAL_PTR_NULL && hmac_other_vap != hmac_vap) {
            /* 抛事件上报内核，已经删除某个STA */
            hmac_mgmt_send_disassoc_frame_etc(hmac_other_vap, mac_addr, MAC_DISAS_LV_SS, OAL_FALSE);
            oam_warning_log2(0, OAM_SF_DFT, "hmac_encap_auth_rsp_get_user_idx_etc:other vap[%d], err code[%d]",
                hmac_other_vap->vap_id, MAC_DISAS_LV_SS);
            hmac_handle_disconnect_rsp_ap_etc(hmac_other_vap, hmac_user);
            if (fhook != OSAL_NULL) {
                ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_other_vap->chip_id, hmac_user->user_mac_addr,
                AP_AUTH_RSP_DEL_OTHER_STA, USER_CONN_OFFLINE);
            }
            hmac_user_del_etc(hmac_other_vap, hmac_user);
        }
    }

    return hmac_encap_auth_rsp_get_user_idx_proc(hmac_vap, mac_addr, is_seq1, pus_user_index);
}

/*****************************************************************************
 函 数 名  : hmac_encap_auth_rsp_seq1_etc
 功能描述  : 处理seq1的auth req
 输入参数  : 1.auth_rsp_param 处理auth rsp所需的参数

 输出参数  : 1.code 错误码
             2.usr_ass_stat auth处理完成之后，置相应的user状态

 返 回 值  :获取正常或者失败
*****************************************************************************/
OAL_STATIC hmac_ap_auth_process_code_enum_uint8 hmac_encap_auth_rsp_seq1_etc(hmac_vap_stru *hmac_vap,
    const hmac_auth_rsp_param_stru *auth_rsp_param, osal_u8 *code, hmac_user_stru *hmac_user)
{
    *code = MAC_SUCCESSFUL_STATUSCODE;

    unref_param(hmac_vap);
    hmac_user->user_asoc_state = MAC_USER_STATE_BUTT;
    /* 如果不是重传 */
    if (auth_rsp_param->auth_resend != OAL_TRUE) {
        if (auth_rsp_param->auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM) {
            hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_COMPLETE;

            return HMAC_AP_AUTH_SEQ1_OPEN_ANY;
        }

        if (auth_rsp_param->is_wep_allowed == OAL_TRUE) {
            hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_KEY_SEQ1;
            /* 此处返回后需要wep后操作 */
            return HMAC_AP_AUTH_SEQ1_WEP_NOT_RESEND;
        }

        /* 不支持算法 */
        *code = MAC_UNSUPT_ALG;
        return HMAC_AP_AUTH_BUTT;
    }

    /* 检查用户状态 */
    if ((auth_rsp_param->user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (auth_rsp_param->auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM)) {
        /* 收到已关联用户的Auth帧,不改变关联状态 */
        oam_warning_log1(0, OAM_SF_ANY,
            "hmac_encap_auth_rsp_seq1_etc:rx auth req, user_asoc_state[%d]", hmac_user->user_asoc_state);
        /* 未开启PMF功能，后续关联时将状态改为auth_complete */
        if (mac_mib_get_dot11_rsnamfpc(hmac_vap) != OAL_TRUE) {
            hmac_user->user_asoc_state = MAC_USER_STATE_ASSOC;
            hmac_user->flag_change_user_state_to_auth_complete = OAL_TRUE;
        }

        *code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_DUMMY;
    }

    if (auth_rsp_param->auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM) {
        hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_COMPLETE;

        return HMAC_AP_AUTH_SEQ1_OPEN_ANY;
    }

    if (auth_rsp_param->is_wep_allowed == OAL_TRUE) {
        /* seq为1 的认证帧重传 */
        hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_COMPLETE;
        return HMAC_AP_AUTH_SEQ1_WEP_RESEND;
    }
    /* 不支持算法 */
    *code = MAC_UNSUPT_ALG;
    return HMAC_AP_AUTH_BUTT;
}
/*****************************************************************************
 函 数 名  : hmac_encap_auth_rsp_seq3_etc
 功能描述  : 处理seq3的auth req
 输入参数  : 1.auth_rsp_param 处理auth rsp所需的参数
 输出参数  : 1.code 错误码
             2.usr_ass_stat auth处理完成之后，置相应的user状态
 返 回 值  :获取正常或者失败
*****************************************************************************/
OAL_STATIC hmac_ap_auth_process_code_enum_uint8 hmac_encap_auth_rsp_seq3_etc(hmac_vap_stru *hmac_vap,
    const hmac_auth_rsp_param_stru *auth_rsp_param, osal_u8 *code, hmac_user_stru *hmac_user)
{
    unref_param(hmac_vap);

    /* 如果不存在，返回错误 */
    if (auth_rsp_param->auth_resend == OAL_FALSE) {
        hmac_user->user_asoc_state = MAC_USER_STATE_BUTT;
        *code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_BUTT;
    }
    /* 检查用户状态 */
    if ((auth_rsp_param->user_asoc_state == MAC_USER_STATE_ASSOC) &&
        (auth_rsp_param->auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM)) {
        /* 收到已关联用户的Auth帧,不改变关联状态 */
        oam_warning_log1(0, OAM_SF_ANY,
            "hmac_encap_auth_rsp_seq3_etc:user associated rx auth req, do not change user [%d] state",
            hmac_user->user_asoc_state);
        /* 未开启PMF功能，后续关联时将状态改为auth_complete */
        if (mac_mib_get_dot11_rsnamfpc(hmac_vap) != OAL_TRUE) {
            hmac_user->user_asoc_state = MAC_USER_STATE_ASSOC;
            hmac_user->flag_change_user_state_to_auth_complete = OAL_TRUE;
        }

        *code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_DUMMY;
    }

    if (auth_rsp_param->auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM) {
        hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_COMPLETE;
        *code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_OPEN_ANY;
    }

    if (auth_rsp_param->user_asoc_state == MAC_USER_STATE_AUTH_KEY_SEQ1) {
        hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_COMPLETE;
        *code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_WEP_COMPLETE;
    }

    if (auth_rsp_param->user_asoc_state == MAC_USER_STATE_AUTH_COMPLETE) {
        hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_COMPLETE;
        *code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_WEP_COMPLETE;
    }

    if (auth_rsp_param->user_asoc_state == MAC_USER_STATE_ASSOC) {
        hmac_user->user_asoc_state = MAC_USER_STATE_AUTH_KEY_SEQ1;
        *code = MAC_SUCCESSFUL_STATUSCODE;
        return HMAC_AP_AUTH_SEQ3_WEP_ASSOC;
    }

    /* 不支持算法 */
    hmac_user->user_asoc_state = MAC_USER_STATE_BUTT;
    *code = MAC_UNSUPT_ALG;
    return HMAC_AP_AUTH_BUTT;
}

/*****************************************************************************
 函 数 名  : hmac_encap_auth_rsp_seq3_etc
 功能描述  : 处理seq3的auth req
 输入参数  : 1.auth_rsp_param 处理auth rsp所需的参数数组
 输出参数  : 1.code 错误码
             2.usr_ass_stat auth处理完成之后，置相应的user状态
 返 回 值  :获取正常或者失败
*****************************************************************************/
OAL_STATIC hmac_auth_rsp_fun hmac_encap_auth_rsp_get_func_etc(osal_u16 auth_seq)
{
    hmac_auth_rsp_fun        auth_rsp_fun;
    switch (auth_seq) {
        case WLAN_AUTH_TRASACTION_NUM_ONE:
            auth_rsp_fun = hmac_encap_auth_rsp_seq1_etc;
            break;
        case WLAN_AUTH_TRASACTION_NUM_THREE:
            auth_rsp_fun = hmac_encap_auth_rsp_seq3_etc;
            break;
        default:
            auth_rsp_fun = OAL_PTR_NULL;
            break;
    }
    return auth_rsp_fun;
}

/*****************************************************************************
 函 数 名  : hmac_encap_auth_rsp_support_etc
 功能描述  : 判断认证类型是否支持
 输入参数  : 1.hmac_vap vap指针
             2. auth_type 认证类型
 输出参数  : 无
 返 回 值  :OAL_SUCC-支持，其他-不支持
*****************************************************************************/
OAL_STATIC osal_u32 hmac_encap_auth_rsp_support_etc(hmac_vap_stru *hmac_vap, osal_u16 auth_type)
{
    if (hmac_vap == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检测认证类型是否支持 不支持的话状态位置成UNSUPT_ALG */
    if ((mac_mib_get_authentication_mode(hmac_vap) != auth_type) &&
        (mac_mib_get_authentication_mode(hmac_vap) != WLAN_WITP_AUTH_AUTOMATIC)) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tid_clear_etc
 功能描述  : 删除hmac tid相关的信息
 输入参数  : 无
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_void hmac_tid_clear_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_bool lock_free_flag)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_u8 loop;
    hmac_tid_stru *pst_tid = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_AMSDU_TID_INFO_CLEAR);

    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_tid_clear_etc::hmac_user is null.}");
        return;
    }
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_BA, "vap_id[%d] {hmac_tid_clear_etc::device null.}", hmac_vap->vap_id);
        return;
    }

    if (fhook != OAL_PTR_NULL) {
        ((hmac_amsdu_tid_info_clear_cb)fhook)(hmac_user);
    }

    for (loop = 0; loop < WLAN_TID_MAX_NUM; loop++) {
        pst_tid = &(hmac_user->tx_tid_queue[loop]);

        if (pst_tid->ba_tx_info.addba_timer.is_registerd == OAL_TRUE) {
            frw_destroy_timer_entry(&(pst_tid->ba_tx_info.addba_timer));
        }

        pst_tid->tid        = (osal_u8)loop;

        /* 清除接收方向会话句柄 */
        if (pst_tid->ba_rx_info != OAL_PTR_NULL) {
            hmac_ba_reset_rx_handle_etc(hmac_device, hmac_user, loop, OAL_TRUE);
        }

        if (pst_tid->ba_tx_info.ba_status != HMAC_BA_INIT) {
            /* 存在TX BA会话句柄，要-- */
            hmac_tx_ba_session_decr_etc(hmac_vap, loop);
        }

        /* 初始化ba tx操作句柄 */
        /* 初始化ba tx操作句柄 */
        if (lock_free_flag == OSAL_TRUE) {
            osal_spin_lock_destroy(&pst_tid->ba_tx_info.ba_status_lock);
        }
        pst_tid->ba_tx_info.ba_status         = HMAC_BA_INIT;
        pst_tid->ba_tx_info.addba_attemps     = 0;
        pst_tid->ba_tx_info.dialog_token      = 0;
        pst_tid->ba_tx_info.ba_policy      = 0;
        hmac_user->ba_flag[loop] = 0;
    }
}
OAL_STATIC osal_bool hmac_auth_rsp_judge_req_is_illegal(osal_u8 assoc_state, osal_u16 auth_type, osal_u16 auth_seq)
{
    if (assoc_state == MAC_USER_STATE_ASSOC &&
        ((auth_type != WLAN_WITP_AUTH_OPEN_SYSTEM) ||
        ((auth_type == WLAN_WITP_AUTH_OPEN_SYSTEM) && (auth_seq != WLAN_AUTH_TRASACTION_NUM_ONE)))) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}
/*
 * 函 数 名  : hmac_ap_re_rx_auth_protect_update
 * 功能描述  : 更新seq num和timestamp
 * 修改历史      :
 *  1.日    期   : 2021年11月27日
 *    修改内容   : 新生成函数
 */
OAL_STATIC void hmac_ap_re_rx_auth_protect_update(hmac_user_stru  *hmac_user, osal_u16 frame_seq_num)
{
    hmac_auth_re_rx_protect_stru *auth_re_rx_protect = &hmac_user->auth_re_rx_protect;

    auth_re_rx_protect->rx_auth_timestamp = (osal_u32)oal_time_get_stamp_ms();
    auth_re_rx_protect->rx_auth_seqnum = frame_seq_num;
    auth_re_rx_protect->is_rx_auth = OAL_TRUE;
}
/*****************************************************************************
 函 数 名  : hmac_encap_auth_rsp_ap
 功能描述  : 封装auth rsp帧
 输入参数  : 1. vap指针
             2. auth_req auth_req
             3. challenge txt
 输出参数  : 1. auth_rsp auth_rsp缓冲
*****************************************************************************/
osal_u16  hmac_encap_auth_rsp_etc(hmac_vap_stru *hmac_vap, oal_netbuf_stru *auth_rsp,
    oal_netbuf_stru *auth_req, osal_u8 *chtxt)
{
    osal_u16       auth_rsp_len        = 0;
    hmac_user_stru  *hmac_user              = OAL_PTR_NULL;
    osal_u8       *frame              = OAL_PTR_NULL;
    osal_u16       us_index, auth_type, auth_seq, frame_seq_num;
    osal_u8        is_seq1;
    osal_u8        addr2[6]           = {0};
    osal_u16       user_index          = 0xffff;
    osal_u32       ul_ret, alg_suppt;
    hmac_ap_auth_process_code_enum_uint8       auth_proc_rst; /* 认证方法 */

    osal_u8       *data;
    mac_tx_ctl_stru *tx_ctl;
    mac_rx_ctl_stru *rx_ctl = OSAL_NULL;
    hmac_auth_rsp_handle_stru              auth_rsp_handle;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_BLACKLIST_FILTER_ETC);;

    if (oal_is_null_ptr4(hmac_vap, auth_rsp, auth_req, chtxt)) {
        oam_error_log0(0, OAM_SF_AUTH, "{hmac_encap_auth_rsp_etc::NULL param}");
        return auth_rsp_len;
    }

    data    = (osal_u8 *)OAL_NETBUF_HEADER(auth_rsp);
    tx_ctl  = (mac_tx_ctl_stru *)oal_netbuf_cb(auth_rsp);
    rx_ctl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(auth_req);
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

    /* 设置函数头的frame control字段 */
    mac_hdr_set_frame_control(data, WLAN_FC0_SUBTYPE_AUTH);

    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(auth_req), (osal_u8 *)addr2);

    /* 将DA设置为STA的地址 */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address1, (osal_u8 *)addr2);

    /* 将SA设置为dot11MacAddress */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address2, mac_mib_get_station_id(hmac_vap));
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)data)->address3, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(data, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Authentication Frame - Frame Body                        */
    /* --------------------------------------------------------------------- */
    /* |Auth Algo Number|Auth Trans Seq Number|Status Code| Challenge Text | */
    /* --------------------------------------------------------------------- */
    /* | 2              |2                    |2          | 3 - 256        | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    us_index = MAC_80211_FRAME_LEN;
    frame = (osal_u8 *)(data + us_index);

    /* 计算认证相应帧的长度 */
    auth_rsp_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN + MAC_STATUS_CODE_LEN;

    /* 解析认证类型 */
    auth_type = mac_get_auth_algo_num(auth_req);

    /* 解析auth transaction number */
    auth_seq  = mac_get_auth_seq_num(oal_netbuf_header(auth_req), mac_get_rx_cb_payload_len(rx_ctl));
    if (auth_seq > WLAN_AUTH_TRASACTION_NUM_BUTT || auth_seq < WLAN_AUTH_TRASACTION_NUM_ONE) {
        oam_warning_log1(0, OAM_SF_AUTH,
            "{hmac_encap_auth_rsp_etc::auth recieve invalid seq, auth seq [%d]}", auth_seq);
        return 0;
    }

    /* 设置认证类型IE */
    frame[0] = (auth_type & 0x00FF);
    frame[1] = (osal_u8)((auth_type & 0xFF00) >> 8); /* 右移8位 */

    /* 将收到的transaction number + 1后复制给新的认证响应帧 */
    frame[2] = ((auth_seq + 1) & 0x00FF);    /* 2 索引 */
    frame[3] = (osal_u8)(((auth_seq + 1) & 0xFF00) >> 8); /* 3 索引 右移8位 */

    /* 状态为初始化为成功 */
    frame[4] = MAC_SUCCESSFUL_STATUSCODE;    /* 4 索引 */
    frame[5] = 0;    /* 5 索引 */

    oam_warning_log4(0, OAM_SF_CONN, "{hmac_encap_auth_rsp_etc:: AUTH_RSP tx : user mac:%02X:%02X:%02X:%02X:XX:XX}",
        addr2[0], addr2[1], addr2[2], addr2[3]);    /* 0,1,2,3 MAC索引 */

    /* 判断对端mac地址是否为有效，不能为全0 */
    if (hmac_addr_is_zero_etc((osal_u8 *)addr2)) {
        oam_warning_log4(0, OAM_SF_AUTH,
            "{hmac_encap_auth_rsp_etc::user mac:%02X:%02X:%02X:%02X:XX:XX is all 0 and invaild!}",
            addr2[0], addr2[1], addr2[2], addr2[3]);    /* 0,1,2,3 MAC索引 */
        frame[4] = MAC_UNSPEC_FAIL;    /* 4 索引 */
        mac_get_cb_tx_user_idx(tx_ctl) = MAC_INVALID_USER_ID;
        mac_get_cb_mpdu_len(tx_ctl)    = auth_rsp_len;
        return auth_rsp_len;
    }

    /* 因黑名单拒绝回auth,status code回37 */
    if (fhook != OSAL_NULL && ((hmac_blacklist_filter_etc_cb)fhook)(hmac_vap, addr2) == OAL_TRUE) {
        oam_warning_log4(0, OAM_SF_AUTH,
            "{hmac_encap_auth_rsp_etc::user mac:%02X:%02X:%02X:%02X:XX:XX is all 0 and invaild!}",
            addr2[0], addr2[1], addr2[2], addr2[3]);    /* 0,1,2,3 MAC索引 */
        frame[4] = MAC_REQ_DECLINED;    /* 4 索引 */
        mac_get_cb_tx_user_idx(tx_ctl) = MAC_INVALID_USER_ID;
        mac_get_cb_mpdu_len(tx_ctl)    = auth_rsp_len;
        return auth_rsp_len;
    }

    /* 获取用户idx */
    is_seq1 = (auth_seq == WLAN_AUTH_TRASACTION_NUM_ONE);
    ul_ret = hmac_encap_auth_rsp_get_user_idx_etc(hmac_vap, (osal_u8 *)addr2, is_seq1,
        &auth_rsp_handle.auth_rsp_param.auth_resend, &user_index);
    if (ul_ret != OAL_SUCC) {
        if (ul_ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
            oam_warning_log0(0, OAM_SF_AUTH,
                "{hmac_encap_auth_rsp_etc::hmac_ap_get_user_idx fail, users exceed config spec!}");
            frame[4] = MAC_AP_FULL;    /* 4 索引 */
        } else {
            oam_warning_log0(0, OAM_SF_AUTH, "{hmac_encap_auth_rsp_etc::hmac_ap_get_user_idx Err!}");
            frame[4] = MAC_UNSPEC_FAIL;    /* 4 索引 */
        }
        mac_get_cb_tx_user_idx(tx_ctl) = MAC_INVALID_USER_ID;
        mac_get_cb_mpdu_len(tx_ctl)    = auth_rsp_len;
        return auth_rsp_len;
    }

    /* 获取hmac user指针 */
    hmac_user = mac_res_get_hmac_user_etc(user_index);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_AUTH, "{hmac_encap_auth_rsp_etc::hmac_user_sta[%d] is NULL}", user_index);
        frame[4] = MAC_UNSPEC_FAIL;    /* 4 索引 */
        mac_get_cb_tx_user_idx(tx_ctl) = MAC_INVALID_USER_ID;
        mac_get_cb_mpdu_len(tx_ctl)    = auth_rsp_len;
        return auth_rsp_len;
    }
    frame_seq_num = mac_get_seq_num(oal_netbuf_header(auth_req));
    hmac_ap_re_rx_auth_protect_update(hmac_user, frame_seq_num);
    /* CB字段user idx进行赋值 */
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)user_index;
    mac_get_cb_mpdu_len(tx_ctl)    = auth_rsp_len;
    if (hmac_auth_rsp_judge_req_is_illegal(hmac_user->user_asoc_state, auth_type, auth_seq) == OAL_TRUE) {
        oam_warning_log2(0, OAM_SF_AUTH, "{auth recv invalid seq, auth seq [%d], auth type[%d]}", auth_seq, auth_type);
        return 0;
    }
    /* 判断算法是否支持 */
    alg_suppt = hmac_encap_auth_rsp_support_etc(hmac_vap, auth_type);
    if (alg_suppt != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_AUTH, "vap_id[%d] {hmac_encap_auth_rsp_etc::auth type[%d] not support!}",
            hmac_vap->vap_id, auth_type);
        frame[4] = MAC_UNSUPT_ALG;    /* 4 索引 */
        hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_BUTT);
        return auth_rsp_len;
    }
#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_user->auth_alg = auth_type;
#endif
    /*  初始化处理参数 */
    auth_rsp_handle.auth_rsp_param.is_wep_allowed = mac_is_wep_allowed(hmac_vap);
    auth_rsp_handle.auth_rsp_param.user_asoc_state = hmac_user->user_asoc_state;
    auth_rsp_handle.auth_rsp_param.auth_type = auth_type;
    auth_rsp_handle.auth_rsp_fun  = hmac_encap_auth_rsp_get_func_etc(auth_seq);

    /*  处理seq1或者seq3 */
    if (auth_rsp_handle.auth_rsp_fun != OAL_PTR_NULL) {
        auth_proc_rst = auth_rsp_handle.auth_rsp_fun(hmac_vap, &auth_rsp_handle.auth_rsp_param,
            &frame[4], hmac_user);    /* 4 索引 */
        /* 清空 HMAC层TID信息 */
        hmac_tid_clear_etc(hmac_vap, hmac_user, OSAL_FALSE);
    } else {
        auth_proc_rst = HMAC_AP_AUTH_BUTT;
        hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_BUTT);
        frame[4] = MAC_AUTH_SEQ_FAIL;    /* 4 索引 */
    }

    oam_warning_log1(0, OAM_SF_AUTH, "{hmac_encap_auth_rsp_etc::auth_proc_rst:%d}", auth_proc_rst);

    /*  根据返回的code进行后续处理 */
    switch (auth_proc_rst) {
        case HMAC_AP_AUTH_SEQ1_OPEN_ANY: {
            hmac_user_init_key_etc(hmac_user);
            frw_create_timer_entry(&hmac_user->mgmt_timer,
                                   hmac_mgmt_timeout_ap_etc,
                                   (osal_u16)mac_mib_get_AuthenticationResponseTimeOut(hmac_vap),
                                   hmac_user,
                                   OAL_FALSE);
            break;
        }
        case HMAC_AP_AUTH_SEQ1_WEP_NOT_RESEND: {
            /* 6 长度 */
            hmac_config_11i_add_wep_entry_etc(hmac_vap, hmac_user->user_mac_addr);

            hmac_mgmt_encap_chtxt(frame, chtxt, &auth_rsp_len, hmac_user);
            /* 为该用户启动一个定时器，超时认证失败 */
            frw_create_timer_entry(&hmac_user->mgmt_timer,
                                   hmac_mgmt_timeout_ap_etc,
                                   (osal_u16)mac_mib_get_AuthenticationResponseTimeOut(hmac_vap),
                                   hmac_user, OAL_FALSE);
            hmac_user->key_info.cipher_type =
                mac_get_wep_type(hmac_vap, mac_get_wep_default_keyid(hmac_vap));
            break;
        }
        case HMAC_AP_AUTH_SEQ1_WEP_RESEND: {
            /* seq为1 的认证帧重传 */
            hmac_mgmt_encap_chtxt(frame, chtxt, &auth_rsp_len, hmac_user);

            /* 重启超时定时器 */
            frw_timer_restart_timer(&hmac_user->mgmt_timer, hmac_user->mgmt_timer.timeout, OAL_FALSE);
            break;
        }
        case HMAC_AP_AUTH_SEQ3_OPEN_ANY: {
            hmac_user_init_key_etc(hmac_user);
            break;
        }
        case HMAC_AP_AUTH_SEQ3_WEP_COMPLETE: {
            chtxt   = mac_get_auth_ch_text(oal_netbuf_header(auth_req));
            if (hmac_mgmt_is_challenge_txt_equal(chtxt, (const osal_u8 *)hmac_user->ch_text) == OAL_TRUE) {
                hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
                if (hmac_user->ch_text != OSAL_NULL) {
                    oal_mem_free(hmac_user->ch_text, OAL_TRUE);
                    hmac_user->ch_text = OSAL_NULL;
                }
                /* cancel timer for auth */
                frw_destroy_timer_entry(&hmac_user->mgmt_timer);
            } else {
                oam_warning_log0(0, OAM_SF_AUTH, "{hmac_encap_auth_rsp_etc::challenage txt not equal.}");
                frame[4] = MAC_CHLNG_FAIL;    /* 4 索引 */
                hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_BUTT);
            }
            break;
        }
        case HMAC_AP_AUTH_SEQ3_WEP_ASSOC: {
            hmac_mgmt_encap_chtxt(frame, chtxt, &auth_rsp_len, hmac_user);

            /* 开启超时定时器 */
            frw_create_timer_entry(&hmac_user->mgmt_timer, hmac_mgmt_timeout_ap_etc,
                (osal_u16)mac_mib_get_AuthenticationResponseTimeOut(hmac_vap), hmac_user, OAL_FALSE);
            break;
        }
        case HMAC_AP_AUTH_DUMMY: {
            break;
        }
        case HMAC_AP_AUTH_BUTT:
        default: {
            hmac_user_init_key_etc(hmac_user);
            hmac_user->user_asoc_state = MAC_USER_STATE_BUTT;
            break;
        }
    }

    return auth_rsp_len;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

