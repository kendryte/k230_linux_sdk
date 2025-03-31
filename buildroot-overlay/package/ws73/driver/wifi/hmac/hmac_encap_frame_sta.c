/*
 * Copyright (c) CompanyNameMagicTag. 2013-2024. All rights reserved.
 * 文 件 名   : hmac_encap_frame_sta.c
 * 生成日期   : 2013年6月28日
 * 功能描述   : STA模式组帧，HMAC层 STA特有帧的组帧文件
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_encap_frame_sta.h"
#include "wlan_spec.h"
#include "wlan_mib_hcm.h"
#include "mac_vap_ext.h"
#include "mac_frame.h"
#include "hmac_user.h"
#include "hmac_main.h"
#include "hmac_tx_data.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_sta.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_scan.h"
#include "mac_user_ext.h"
#include "hmac_roam_if.h"
#include "hmac_mbo.h"
#include "hmac_dnb_sta.h"
#include "hmac_feature_interface.h"
#include "hmac_11k.h"
#include "hmac_11r.h"
#include "hmac_achba.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ENCAP_FRAME_STA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : mac_vap_get_scan_bss_info
 功能描述  : 通过mac_vap获取当前将要关联的AP的扫描结果
 输入参数  : hmac_vap结构体
 输出参数  : 无
*****************************************************************************/
hmac_scanned_bss_info* hmac_vap_get_scan_bss_info(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru       *hmac_device;
    hmac_bss_mgmt_stru     *bss_mgmt;
    hmac_scanned_bss_info  *scaned_bss;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_QOS, "vap_id[%d] {mac_vap_get_scan_bss_info::hmac_device null.}",
            hmac_vap->vap_id);
        return OAL_PTR_NULL;
    }
    bss_mgmt = &(hmac_device->scan_mgmt.scan_record_mgmt.bss_mgmt);
    scaned_bss = hmac_scan_find_scanned_bss_by_bssid_etc(bss_mgmt, hmac_vap->bssid);
    if (scaned_bss == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_ASSOC, "vap_id[%d] {mac_tx_qos_enhance_list_init::do not have scan result!!!}",
            hmac_vap->vap_id);
        return OAL_PTR_NULL;
    }
    return scaned_bss;
}

/*****************************************************************************
 函 数 名  : hmac_sta_check_need_set_ext_cap_ie
 功能描述  : 判断是否需要在assoc req 中包含Extended Capability IE
 输入参数  : hmac_vap: 指向vap
 输出参数  : 无
 返 回 值  : OAL_TRUE    需要包含Extended Capability IE
             OAL_FALSE 不需要包含Extended Capability IE
*****************************************************************************/
OAL_STATIC oal_bool_enum hmac_sta_check_need_set_ext_cap_ie(hmac_vap_stru *hmac_vap)
{
    osal_u8       *ext_cap_ie;
    osal_u16       ext_cap_index;

    ext_cap_ie = hmac_sta_find_ie_in_probe_rsp_etc(hmac_vap, MAC_EID_EXT_CAPS, &ext_cap_index);
    if (ext_cap_ie == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}


/*****************************************************************************
 函 数 名  : hmac_set_supported_rates_ie_asoc_req_etc
 功能描述  : 设置速率集
 输入参数  : hmac_vap: 指向vap
             buffer: 指向buffer
 输出参数  : ie_len: element的长度
 返 回 值  : 无
*****************************************************************************/
OSAL_STATIC osal_void  hmac_set_supported_rates_ie_asoc_req_etc(hmac_vap_stru *hmac_vap, osal_u8 *buffer,
    osal_u8 *ie_len)
{
    osal_u8         nrates;
    osal_u8         idx;

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

    nrates = mac_mib_get_SupportRateSetNums(hmac_vap);
    if (nrates > MAC_MAX_SUPRATES) {
        nrates = MAC_MAX_SUPRATES;
    }

    for (idx = 0; idx < nrates; idx++) {
        buffer[MAC_IE_HDR_LEN + idx] = hmac_vap->supp_rates[idx];
    }

    buffer[1] = nrates;

    *ie_len = MAC_IE_HDR_LEN + nrates;
}
/*****************************************************************************
 函 数 名  : hmac_set_exsup_rates_ie_asoc_req_etc
 功能描述  : 填充extended supported rates信息
 输入参数  : hmac_vap: 指向vap
             buffer: 指向buffer
 输出参数  : ie_len: element的长度
 返 回 值  : 无
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_exsup_rates_ie_asoc_req_etc(hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    osal_u8         nrates;
    osal_u8         idx;

    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/
    if (mac_mib_get_SupportRateSetNums(hmac_vap) <= MAC_MAX_SUPRATES) {
        *ie_len = 0;

        return;
    }

    buffer[0] = MAC_EID_XRATES;
    nrates     = mac_mib_get_SupportRateSetNums(hmac_vap) - MAC_MAX_SUPRATES;
    buffer[1] = nrates;

    for (idx = 0; idx < nrates; idx++) {
        buffer[MAC_IE_HDR_LEN + idx] = hmac_vap->supp_rates[idx + MAC_MAX_SUPRATES];
    }

    *ie_len = MAC_IE_HDR_LEN + nrates;
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_encap_asoc_req_sta_etc
 功能描述  : 组帧(Re)assoc
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_mgmt_encap_asoc_req_sta_etc(hmac_vap_stru *hmac_vap, osal_u8 *req_frame,
    osal_u8 *curr_bssid, osal_u8 *dest_addr)
{
    osal_u8               ie_len            = 0;
    osal_u32              asoc_rsq_len      = 0;
    osal_u8              *req_frame_origin;
    hmac_device_stru       *hmac_device;
    osal_u16              app_ie_len        = 0;
    app_ie_type_uint8    app_ie_type;
    hmac_scanned_bss_info  *scaned_bss;
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_user_stru          *hmac_user;
#endif
    osal_void *fhook = OSAL_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (req_frame == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ASSOC, "{hmac_mgmt_encap_asoc_req_sta_etc::null param.}");
        return asoc_rsq_len;
    }

    /* 保存起始地址，方便计算长度 */
    req_frame_origin = req_frame;

    /* 获取device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_mgmt_encap_asoc_req_sta_etc::hmac_device null.}",
            hmac_vap->vap_id);
        return asoc_rsq_len;
    }

    scaned_bss = hmac_vap_get_scan_bss_info(hmac_vap);
    if (scaned_bss == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_mgmt_encap_asoc_req_sta_etc::scaned_bss null.}",
            hmac_vap->vap_id);
        return asoc_rsq_len;
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    /* VHT中的字段需要根据AP的bf能力设置，故在此提前设置MAC_USER的bf能力 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_mgmt_encap_asoc_req_sta_etc::hmac_user null.}",
            hmac_vap->vap_id);
        return asoc_rsq_len;
    }

    hmac_user->vht_hdl.num_sounding_dim = scaned_bss->bss_dscr_info.num_sounding_dim;
#endif

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
    /* 判断是否为reassoc操作 */
    if (curr_bssid != OAL_PTR_NULL) {
        mac_hdr_set_frame_control(req_frame, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT |
            WLAN_FC0_SUBTYPE_REASSOC_REQ);
    } else {
        mac_hdr_set_frame_control(req_frame,
            WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ASSOC_REQ);
    }
    /* 设置 DA address1: AP MAC地址 (BSSID) */
    oal_set_mac_addr(req_frame + WLAN_HDR_ADDR1_OFFSET, dest_addr);

    /* 设置 SA address2: dot11MACAddress */
    oal_set_mac_addr(req_frame + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 DA address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(req_frame + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(req_frame, 0);

    req_frame += MAC_80211_FRAME_LEN;

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Association Request Frame - Frame Body                   */
    /* --------------------------------------------------------------------- */
    /* | Capability Information | Listen Interval | SSID | Supported Rates | */
    /* --------------------------------------------------------------------- */
    /* |2                       |2                |2-34  |3-10             | */
    /* --------------------------------------------------------------------- */

    /* --------------------------------------------------------------------- */
    /* |Externed Surpported rates| Power Capability | Supported Channels   | */
    /* --------------------------------------------------------------------- */
    /* |3-257                    |4                 |4-256                 | */
    /* --------------------------------------------------------------------- */

    /* --------------------------------------------------------------------- */
    /* | RSN   | QoS Capability | HT Capabilities | Extended Capabilities  | */
    /* --------------------------------------------------------------------- */
    /* |36-256 |3               |28               |3-8                     | */
    /* --------------------------------------------------------------------- */

    /* --------------------------------------------------------------------- */
    /* | WPS   | P2P |                                                       */
    /* --------------------------------------------------------------------- */
    /* |7-257  |X    |                                                       */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    mac_set_cap_info_sta_etc((osal_void *)hmac_vap, req_frame);
    req_frame += MAC_CAP_INFO_LEN;

    /* 设置 Listen Interval IE */
    hmac_set_listen_interval_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;

    /* Ressoc组帧设置Current AP address */
    if (curr_bssid != OAL_PTR_NULL) {
        oal_set_mac_addr(req_frame, curr_bssid);
        req_frame += OAL_MAC_ADDR_LEN;
    }
    /* 设置 SSID IE */
    hmac_set_ssid_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len, WLAN_FC0_SUBTYPE_ASSOC_REQ);
    req_frame += ie_len;
    /* 在关联请求时使用扫描时保存的速率集 */
#ifdef _PRE_WIFI_DMT
    /* 设置 Supported Rates IE */
    hmac_set_supported_rates_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;

    /* 设置 Extended Supported Rates IE */
    hmac_set_exsup_rates_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;

#else
    /* 设置 Supported Rates IE */
    hmac_set_supported_rates_ie_asoc_req_etc(hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;

    /* 设置 Extended Supported Rates IE */
    hmac_set_exsup_rates_ie_asoc_req_etc(hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;
#endif
    /* 设置 Power Capability IE */
    mac_set_power_cap_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;

    /* 设置 Supported channel IE */
    hmac_set_supported_channel_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;

    /* 为增加兼容性，02不使用驱动组装的wpa/wpa2 ie，使用上层下发的 */
    /* 改为若内核版本大于2.6使用上层下发的WPA/WPA2字段， */
    /* 内核版本小于2.6上层没有下发wpa字段，仍然使用自己组的字段 */
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) \
    && (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34))
    if (hmac_vap->cap_flag.wpa2 == OAL_TRUE) {
        /* 设置 RSN Capability IE */
        hmac_set_rsn_ie_etc((osal_void *)hmac_vap, OAL_PTR_NULL, req_frame, &ie_len);
        req_frame += ie_len;
    } else if (hmac_vap->cap_flag.wpa == OAL_TRUE) {
        /* 设置 WPA Capability IE */
        hmac_set_wpa_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }
#endif
#endif
    /* 设置 HT Capability IE  */
    hmac_set_ht_capabilities_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;

    /* 设置 RRM CAP IE */
    if (scaned_bss != OAL_PTR_NULL && scaned_bss->bss_dscr_info.support_rrm == OAL_TRUE) {
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_11K_SET_RRM_CAP);
        if (fhook != OSAL_NULL) {
            ((hmac_set_rrm_enabled_cap_field_etc_cb)fhook)((osal_void *)hmac_vap, req_frame, &ie_len);
            req_frame += ie_len;
        }
    }

    /* 设置 Extended Capability IE */
    /* 如果AP 不包含Extended Capability IE，则在assoc req 中不包含该IE */
    if (hmac_sta_check_need_set_ext_cap_ie(hmac_vap) == OAL_TRUE) {
        hmac_set_ext_capabilities_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }

    /* 设置 VHT Capability IE */
    if (((scaned_bss != OAL_PTR_NULL) && (scaned_bss->bss_dscr_info.vht_capable == OAL_TRUE) &&
        (scaned_bss->bss_dscr_info.vendor_vht_capable == OAL_FALSE))) {
        hmac_set_vht_capabilities_ie_etc((osal_void *) hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_set_he_capabilities_ie((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    if (hmac_vap->cap_flag.opmode == OAL_TRUE) {
        hmac_set_opmode_notify_ie_etc((osal_void *)hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
    if ((scaned_bss != OAL_PTR_NULL) && (scaned_bss->bss_dscr_info.support_1024qam == OAL_TRUE)) {
        hmac_set_1024qam_vendor_ie((osal_void *)hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }
#endif

#ifdef _PRE_WLAN_WS86_FPGA
    wifi_printf("FPGA DO NOT support turbo QAM\n");
#else
    /* 填充 BCM Vendor VHT IE,解决与BCM AP的私有协议对通问题 */
    if ((scaned_bss != OAL_PTR_NULL) && (scaned_bss->bss_dscr_info.vendor_vht_capable == OAL_TRUE)) {
        mac_set_vendor_vht_ie(hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }
    /* 5G时，BCM私有vendor ie中不携带vht,需按照此格式组帧 */
    if ((scaned_bss != OAL_PTR_NULL) && (scaned_bss->bss_dscr_info.vendor_novht_capable == OAL_TRUE)) {
        mac_set_vendor_novht_ie(hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }
#endif

    /* MBO特性认证要求关联请求帧带Supported Operating Classes element and mbo element */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_ASSOC_ADD_MBO_SOC_IE);
    if (fhook != OSAL_NULL) {
        ((hmac_mbo_assoc_add_mbo_and_soc_ie_cb)fhook)(hmac_vap, req_frame, &ie_len, scaned_bss);
        req_frame += ie_len;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_SET_MD_IE);
    if (fhook != OSAL_NULL) {
        ((hmac_11r_set_md_ie_cb)fhook)(hmac_vap, &req_frame, &ie_len);
    }

    app_ie_type = OAL_APP_ASSOC_REQ_IE;
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_CHANGE_APP_IE);
    if (fhook != OSAL_NULL) {
        ((hmac_roam_change_app_ie_cb)fhook)(hmac_vap, &app_ie_type, curr_bssid);
    }
    /* 填充P2P/WPS IE 信息 */
    hmac_add_app_ie_etc(hmac_vap, req_frame, &app_ie_len, app_ie_type);
    /* P2P联调 解决发送的关联请求没有P2P WPS IE信息 */
    req_frame += app_ie_len;

    /* multi-sta特性下新增4地址ie */
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    mac_set_vender_4addr_ie((osal_void *)hmac_vap, req_frame, &ie_len);
    req_frame += ie_len;
#endif

    /* 填充CHBA IE 到AssocReq帧 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ACHBA_ADD_CHBA_INFO_IE);
    if (fhook != OSAL_NULL) {
        ie_len = ((hmac_set_achba_cap_ie_cb)fhook)(hmac_vap, req_frame);
        req_frame += ie_len;
    }

    /* 填充WMM element 将WMM置于底部否则认证机构无法识别AssocReq帧 */
    if (hmac_vap->cap_flag.wmm_cap == OAL_TRUE) {
        hmac_set_wmm_ie_sta_etc((osal_void *)hmac_vap, req_frame, &ie_len);
        req_frame += ie_len;
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_DNB_ADD_PRIVATE_IE);
    if (fhook != OSAL_NULL) {
        ((hmac_set_dnb_cap_cb)fhook)(hmac_vap, &req_frame);
    }

    asoc_rsq_len = (osal_u32)(req_frame - req_frame_origin);
    return asoc_rsq_len;
}

OAL_STATIC osal_void hmac_mgmt_encap_auth_req_frame_header(hmac_vap_stru *hmac_sta, osal_u8 *mgmt_frame)
{
    mac_hdr_set_frame_control(mgmt_frame, WLAN_FC0_SUBTYPE_AUTH);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)mgmt_frame)->address1,
        hmac_sta->bssid);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)mgmt_frame)->address2,
        mac_mib_get_station_id(hmac_sta));

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)mgmt_frame)->address3,
        hmac_sta->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mgmt_frame, 0);
}

OAL_STATIC osal_void hmac_mgmt_encap_auth_req_frame_body(hmac_vap_stru *hmac_sta, osal_u8 *mgmt_frame,
    osal_u8 vap_id, osal_u16 *auth_req_len)
{
    osal_u16 auth_type;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_ENCAP_AUTH_REQ_FT_IE);

    if (mac_mib_get_privacyinvoked(hmac_sta) == OAL_FALSE) {
        /* Open System */
        mgmt_frame[MAC_80211_FRAME_LEN]     = 0x00;
        mgmt_frame[MAC_80211_FRAME_LEN + 1] = 0x00;
    } else {
        auth_type = (osal_u16)mac_mib_get_authentication_mode(hmac_sta);
        if (auth_type == WLAN_WITP_AUTH_SHARED_KEY) {
            oam_info_log1(0, OAM_SF_ASSOC, "vap[%d]{hmac_mgmt_encap_auth_req_etc:WLAN_WITP_AUTH_SHARED_KEY}", vap_id);
            auth_type = WLAN_WITP_AUTH_SHARED_KEY;
        } else {
            oam_info_log1(0, OAM_SF_ASSOC, "vap[%d]{hmac_mgmt_encap_auth_req_etc::WLAN_WITP_AUTH_OPEN_SYSTEM}", vap_id);
            auth_type = WLAN_WITP_AUTH_OPEN_SYSTEM;
        }

        mgmt_frame[MAC_80211_FRAME_LEN]     = (auth_type & 0xFF);
        mgmt_frame[MAC_80211_FRAME_LEN + 1] = (osal_u8)((auth_type & 0xFF00) >> 8); /* 右移8位 */
    }

    /* 设置 Authentication Transaction Sequence Number 为 1 */
    mgmt_frame[MAC_80211_FRAME_LEN + 2] = 0x01; /* 索引偏移2位 */
    mgmt_frame[MAC_80211_FRAME_LEN + 3] = 0x00; /* 索引偏移3位 */

    /* 设置 Status Code 为0. 这个包的这个字段没用 . */
    mgmt_frame[MAC_80211_FRAME_LEN + 4] = 0x00; /* 索引偏移4位 */
    mgmt_frame[MAC_80211_FRAME_LEN + 5] = 0x00; /* 索引偏移5位 */

    /* 设置 认证帧的长度 */
    *auth_req_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN + MAC_STATUS_CODE_LEN;

    if (fhook != OSAL_NULL) {
        ((hmac_11r_encap_auth_req_ft_ie_cb)fhook)(hmac_sta, mgmt_frame, auth_req_len);
    }
}

/*****************************************************************************
 函 数 名  : hmac_mgmt_prepare_auth_req
 功能描述  : 组seq = 1 的auth req帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u16  hmac_mgmt_encap_auth_req_etc(hmac_vap_stru *hmac_vap, osal_u8 *mgmt_frame)
{
    osal_u16      auth_req_len, user_index;
    hmac_user_stru *hmac_user;
    osal_u32      ret;
    osal_u8       vap_id = hmac_vap->vap_id;

    /* *********************************************************************** */
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

    hmac_mgmt_encap_auth_req_frame_header(hmac_vap, mgmt_frame);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Authentication Frame (Sequence 1) - Frame Body           */
    /* --------------------------------------------------------------------  */
    /* |Auth Algorithm Number|Auth Transaction Sequence Number|Status Code|  */
    /* --------------------------------------------------------------------  */
    /* | 2                   |2                               |2          |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    hmac_mgmt_encap_auth_req_frame_body(hmac_vap, mgmt_frame, vap_id, &auth_req_len);

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_info_log1(0, OAM_SF_ASSOC, "vap_id[%d]{hmac_mgmt_encap_auth_req_etc::no present ap,alloc new ap}", vap_id);
        ret = hmac_user_add_etc(hmac_vap, hmac_vap->bssid, &user_index);
        if (ret != OAL_SUCC) {
            oam_warning_log2(0, OAM_SF_ASSOC, "vap[%d]{hmac_mgmt_encap_auth_req_etc::add user fail[%d]}", vap_id, ret);
            auth_req_len = 0;
        }
    }

    return auth_req_len;
}

/*****************************************************************************
 函 数 名  : hmac_prepare_auth_req_seq3
 功能描述  : 为shared key准备seq = 3 的认证帧
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u16  hmac_mgmt_encap_auth_req_seq3_etc(hmac_vap_stru *hmac_vap, osal_u8 *mgmt_frame, osal_u8 *mac_hrd)
{
    osal_u8  *data       = OAL_PTR_NULL;
    osal_u16  us_index;
    osal_u16  auth_req_len;
    osal_u8  *ch_text     = OAL_PTR_NULL;
    osal_u8   ch_text_len;

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

    mac_hdr_set_frame_control(mgmt_frame, WLAN_FC0_SUBTYPE_AUTH);

    /* 将帧保护字段置1 */
    mac_set_wep(mgmt_frame, 1);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)mgmt_frame)->address1,
        hmac_vap->bssid);

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)mgmt_frame)->address2,
        mac_mib_get_station_id(hmac_vap));

    oal_set_mac_addr(((mac_ieee80211_frame_stru *)mgmt_frame)->address3,
        hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mgmt_frame, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Authentication Frame (Sequence 3) - Frame Body           */
    /* --------------------------------------------------------------------- */
    /* |Auth Algo Number|Auth Trans Seq Number|Status Code| Challenge Text | */
    /* --------------------------------------------------------------------- */
    /* | 2              |2                    |2          | 3 - 256        | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* 获取认证帧payload */
    us_index = MAC_80211_FRAME_LEN;
    data = (osal_u8 *)(mgmt_frame + us_index);

    /* 设置 认证帧的长度 */
    auth_req_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN +
                      MAC_STATUS_CODE_LEN;

    /* In case of no failure, the frame must be WEP encrypted. 4 bytes must  */
    /* be   left for the  IV  in  that  case. These   fields will  then  be  */
    /* reinitialized, using the correct index, with offset for IV field.     */
    data[0] = WLAN_WITP_AUTH_SHARED_KEY;    /* Authentication Algorithm Number */
    data[1] = 0x00;

    data[2] = 0x03;     /* 2 索引 Authentication Transaction Sequence Number */
    data[3] = 0x00;     /* 3 索引 */

    /* If WEP subfield in the  incoming  authentication frame is 1,  respond */
    /* with  'challenge text failure' status,  since the STA does not expect */
    /* an encrypted frame in this state.                                     */
    if (mac_is_protectedframe(mac_hrd) == 1) {
        data[4] = MAC_CHLNG_FAIL;   /* 4 索引 */
        data[5] = 0x00;             /* 5 索引 */
    } else if (mac_is_wep_enabled(hmac_vap) == OAL_FALSE) {
        /* If the STA does not support WEP, respond with 'unsupported algo'      */
        /* status, since WEP is necessary for Shared Key Authentication.         */
        data[4] = MAC_UNSUPT_ALG;   /* 4 索引 */
        data[5] = 0x00;             /* 5 索引 */
    } else if (mac_get_wep_default_keysize(hmac_vap) == 0) {
        /* If the default WEP key is NULL, respond with 'challenge text failure' */
        /* status, since a NULL key value cannot be used for WEP operations.     */
        data[4] = MAC_CHLNG_FAIL;   /* 4 索引 */
        data[5] = 0x00;            /* 5 索引 */
    } else {
        /* If there is a mapping in dot11WEPKeyMappings matching the address of  */
        /* the AP, and the corresponding key is NULL respond with 'challenge     */
        /* text failure' status. This is currently not being used.               */

        /* No error condition detected */
        /* Set Status Code to 'success' */
        data[4] = MAC_SUCCESSFUL_STATUSCODE;   /* 4 索引 */
        data[5] = 0x00;            /* 5 索引 */

        /* Extract 'Challenge Text' and its 'length' from the incoming       */
        /* authentication frame                                              */
        ch_text_len = mac_hrd[MAC_80211_FRAME_LEN + 7];  /* 7 索引偏移 */
        ch_text     = (osal_u8 *)(&mac_hrd[MAC_80211_FRAME_LEN + 8]); /* 8 索引偏移 */

        /* Challenge Text Element                  */
        /* --------------------------------------- */
        /* |Element ID | Length | Challenge Text | */
        /* --------------------------------------- */
        /* | 1         |1       |1 - 253         | */
        /* --------------------------------------- */

        mgmt_frame[us_index + 6]   = MAC_EID_CHALLENGE;   /* 6 索引偏移 */
        mgmt_frame[us_index + 7]   = ch_text_len;   /* 7 索引偏移 */
        if (memcpy_s(&mgmt_frame[us_index + 8], ch_text_len, ch_text, ch_text_len) != EOK) { /* 8 索引偏移 */
            oam_error_log0(0, OAM_SF_ANY, "{hmac_mgmt_encap_auth_req_seq3_etc::memcpy_s error}");
        }

        /* Add the challenge text element length to the authentication       */
        /* request frame length. The IV, ICV element lengths will be added   */
        /* after encryption.                                                 */
        auth_req_len += (ch_text_len + MAC_IE_HDR_LEN);
    }

    return auth_req_len;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

