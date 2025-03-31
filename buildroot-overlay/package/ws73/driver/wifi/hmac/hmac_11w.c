/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: 11w hmac 功能处理
 * Date: 2020-07-14
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_11w.h"
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "wlan_mib_hcm.h"
#include "mac_user_ext.h"
#include "mac_resource_ext.h"
#include "mac_frame.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_11i.h"
#include "hal_ext_if.h"
#include "oal_netbuf_data.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_11W_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
 功能描述  : 检查是否是强健管理帧中的action帧
*****************************************************************************/
static oal_bool_enum_uint8  hmac_11w_robust_action(osal_u8 frame_type, const osal_u8 *payload)
{
    if (frame_type != WLAN_ACTION) {
        return OSAL_FALSE;
    }

    switch (payload[MAC_ACTION_OFFSET_CATEGORY]) {
        case MAC_ACTION_CATEGORY_SPECMGMT:
        case MAC_ACTION_CATEGORY_QOS:
        case MAC_ACTION_CATEGORY_DLS:
        case MAC_ACTION_CATEGORY_BA:
        case MAC_ACTION_CATEGORY_RADIO_MEASURMENT:
        case MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION:
        case MAC_ACTION_CATEGORY_SA_QUERY:
        case MAC_ACTION_CATEGORY_PROTECTED_DUAL_OF_ACTION:
        case MAC_ACTION_CATEGORY_WNM:
        case MAC_ACTION_CATEGORY_MESH:
        case MAC_ACTION_CATEGORY_MULTIHOP:
        case MAC_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED: {
            return OSAL_TRUE;
        }
        default:
            break;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 检查是否为强健管理帧中的非action帧
*****************************************************************************/
static oal_bool_enum_uint8  hmac_11w_robust_non_action(osal_u8 frame_type, const osal_u8 *payload)
{
    mac_reason_code_enum_uint16 reason_code;
    osal_u16 reason_code_calculat;

    switch (frame_type) {
        case WLAN_DISASOC:
        case WLAN_DEAUTH: {
            reason_code_calculat = payload[1];
            reason_code_calculat = (osal_u16)((reason_code_calculat << 8) + payload[0]); /* 取低8位 */
            reason_code = (mac_reason_code_enum_uint16)reason_code_calculat;
            /* 两种特殊情况不用加密 */
            if (reason_code == MAC_NOT_AUTHED || reason_code == MAC_NOT_ASSOCED) {
                return OSAL_FALSE;
            }
            return OSAL_TRUE;
        }

        default:
            break;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述  : 是否为强健管理帧
*****************************************************************************/
static oal_bool_enum_uint8  hmac_11w_robust_frame(const oal_netbuf_stru *mgmt_frame, const osal_u8 *mac_header,
    osal_bool is_tx)
{
    osal_u8 frame_type = mac_frame_get_type_value(mac_header);
    const osal_u8 *payload;

    if (is_tx) {
        payload = oal_netbuf_tx_data_const(mgmt_frame);
    } else {
        payload = oal_netbuf_rx_data_const(mgmt_frame);
    }

    /* 非管理帧 */
    if ((frame_type != WLAN_MANAGEMENT) || (payload == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    frame_type = mac_frame_get_subtype_value(mac_header);
    if ((hmac_11w_robust_action(frame_type, payload) == OSAL_TRUE) ||
        (hmac_11w_robust_non_action(frame_type, payload) == OSAL_TRUE)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_11w_get_pmf_cap
 功能描述  : 获取vap的PMF能力
*****************************************************************************/
static osal_void hmac_11w_get_pmf_cap(const hmac_vap_stru *hmac_vap, wlan_pmf_cap_status_uint8 *pmf_cap)
{
    oal_bool_enum_uint8 mfpc;
    oal_bool_enum_uint8 mfpr;

    *pmf_cap = MAC_PMF_DISABLED;
    mfpc = mac_mib_get_dot11_rsnamfpc(hmac_vap);
    mfpr = mac_mib_get_dot11_rsnamfpr(hmac_vap);
    /* 判断vap的PMF能力 */
    if ((mfpc == OSAL_FALSE) && (mfpr == OSAL_TRUE)) {
        *pmf_cap = MAC_PMF_DISABLED;
    } else if ((mfpc == OSAL_FALSE) && (mfpr == OSAL_FALSE)) {
        *pmf_cap = MAC_PMF_DISABLED;
    } else if ((mfpc == OSAL_TRUE) && (mfpr == OSAL_FALSE)) {
        *pmf_cap = MAC_PMF_ENABLED;
    } else {
        *pmf_cap = MAC_PMF_REQUIRED;
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_11w_check_multicast_mgmt
 功能描述  : 确认是否11w支持的组播管理帧
*****************************************************************************/
static oal_bool_enum_uint8 hmac_11w_check_multicast_mgmt(const oal_netbuf_stru *mgmt_buf, osal_bool is_tx)
{
    const osal_u8 *da;
    const mac_ieee80211_frame_stru *frame_hdr;

    frame_hdr = (const mac_ieee80211_frame_stru *)oal_netbuf_header_const(mgmt_buf);
    da        = frame_hdr->address1;

    if (ether_is_multicast(da) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    return hmac_11w_robust_frame(mgmt_buf, (const osal_u8 *)frame_hdr, is_tx);
}

/*****************************************************************************
 函 数 名  : hmac_11w_check_vap_pmf_cap
 功能描述  : 检测此vap是否具备管理帧加密的前提条件
*****************************************************************************/
static oal_bool_enum_uint8 hmac_11w_check_vap_pmf_cap(const hmac_vap_stru *hmac_vap, wlan_pmf_cap_status_uint8  pmf_cap,
    const hmac_user_stru *multi_user)
{
    /* 具备pmf管理帧加密的前提条件:
       1) mib项打开:RSN Active mib
       2) 能力支持 :user 支持pmf
       3) igtk存在 :用于广播Robust mgmt加密
    */
    if ((mac_mib_get_rsnaactivated(hmac_vap) == OSAL_TRUE) &&
        (pmf_cap != MAC_PMF_DISABLED) &&
        (hmac_check_igtk_exist(multi_user->key_info.igtk_key_index) == OAL_SUCC)) {
        return OSAL_TRUE;
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_11w_bip_crypto
 功能描述  : 11w damc层组播/广播 强健管理帧 加密
*****************************************************************************/
static osal_void hmac_11w_bip_crypto(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf_mgmt,
    wlan_security_txop_params_stru  *security, const hal_tx_mpdu_stru *mpdu)
{
    osal_u8 pmf_igtk_keyid;
    wlan_pmf_cap_status_uint8 pmf_cap;
    hmac_user_stru *multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    unref_param(mpdu);
    if (multi_user == OSAL_NULL) {
        return;
    }

    /* 判断vap的pmf能力 */
    hmac_11w_get_pmf_cap(hmac_vap, &pmf_cap);

    /* 判断是否需要加密的组播/广播 强健管理帧 */
    if ((hmac_11w_check_multicast_mgmt(netbuf_mgmt, OSAL_TRUE) == OSAL_TRUE) &&
        (hmac_11w_check_vap_pmf_cap(hmac_vap, pmf_cap, multi_user) == OSAL_TRUE)) {
        pmf_igtk_keyid = multi_user->key_info.igtk_key_index;

        security->cipher_protocol_type = WLAN_80211_CIPHER_SUITE_BIP;
        security->cipher_key_id = pmf_igtk_keyid;
    }
}

/*****************************************************************************
 函 数 名  : hmac_11w_set_ucast_mgmt_frame
 功能描述  : 链接pmf使能的情况下对tx单播管理帧进行加密
*****************************************************************************/
osal_void hmac_11w_set_ucast_mgmt_frame(hmac_vap_stru *hmac_vap, const mac_tx_ctl_stru *tx_ctl,
    hal_tx_txop_feature_stru *txop_feature, const oal_netbuf_stru *netbuf, mac_ieee80211_frame_stru *frame_hdr)
{
    oal_bool_enum_uint8 mfpc;
    oal_bool_enum_uint8 pmf_active;
    hmac_user_stru     *hmac_user = OSAL_NULL;
    osal_u8           mac_addr[WLAN_MAC_ADDR_LEN];
    osal_u8           *mac_addr_ptr = mac_addr;
    osal_u16          user_idx = MAC_INVALID_USER_ID;

    if (tx_ctl == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY,
            "{hmac_11w_set_ucast_mgmt_frame:: tx_ctl is null!!!}");
    }
    /* disassoc/deauth帧cb字段填写的是invalid值,daihu排查后再修改 */
    if (memcpy_s(mac_addr_ptr, WLAN_MAC_ADDR_LEN, (osal_u8*)frame_hdr + 4, WLAN_MAC_ADDR_LEN) != EOK) {  /* 4偏移 */
        oam_error_log0(0, OAM_SF_ANY, "{hmac_11w_set_ucast_mgmt_frame::memcpy_s error}");
    }
    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, (osal_u8 *)mac_addr, &user_idx) != OAL_SUCC) {
        return;
    }
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OSAL_NULL) {
        return;
    }

    /* 能力检查 */
    mfpc = mac_mib_get_dot11_rsnamfpc(hmac_vap);
    pmf_active = hmac_user->cap_info.pmf_active;
    if ((mfpc == OSAL_FALSE) || (pmf_active == OSAL_FALSE)) {
        return;
    }

    if (hmac_11w_robust_frame(netbuf, (osal_u8*)frame_hdr, OSAL_TRUE) == OSAL_TRUE) {
        mac_set_protectedframe((osal_u8 *)frame_hdr);
        txop_feature->security.cipher_key_type      = hmac_user->user_tx_info.security.cipher_key_type;
        txop_feature->security.cipher_protocol_type =
            hmac_user->user_tx_info.security.cipher_protocol_type;

        return;
    }
}

/*****************************************************************************
 函 数 名  : hmac_11w_set_mcast_mgmt_frame
 功能描述  : 链接pmf使能的情况下对tx组播管理帧进行加密
*****************************************************************************/
osal_void hmac_11w_set_mcast_mgmt_frame(hmac_vap_stru  *hmac_vap, hal_tx_txop_feature_stru *txop_feature,
    oal_netbuf_stru *netbuf, const hal_tx_mpdu_stru *mpdu)
{
    mac_ieee80211_frame_stru   *frame_hdr;

    frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(netbuf);
    if (mac_is_protectedframe((osal_u8 *)frame_hdr) == 0) {
        /* 强健组播管理帧加密 */
        hmac_11w_bip_crypto(hmac_vap, netbuf, &(txop_feature->security), mpdu);
    }
}

/*****************************************************************************
 函 数 名  : hmac_11w_update_users_status
 功能描述  : 更新vap下的所有user的pmf使能的统计信息
*****************************************************************************/
osal_void hmac_11w_update_users_status(hmac_vap_stru  *hmac_vap, hmac_user_stru *hmac_user,
    oal_bool_enum_uint8 user_pmf)
{
    wlan_pmf_cap_status_uint8  pmf_cap;
    osal_u32                 user_pmf_old;
    osal_u32                 flag = BIT0;

    if (hmac_vap == OSAL_NULL || hmac_user == OSAL_NULL) {
        oam_error_log2(0, OAM_SF_PMF, "{hmac_11w_update_users_status::hmac_vap is %p, hmac_user is %p.}",
            (uintptr_t)hmac_vap, (uintptr_t)hmac_user);
        return;
    }
    hmac_user_set_pmf_active_etc(hmac_user, user_pmf);

    user_pmf_old = hmac_vap->user_pmf_status;
    if (hmac_user->cap_info.pmf_active == OSAL_FALSE) {
        hmac_vap->user_pmf_status &= (osal_u32)(~(flag << (hmac_user->assoc_id)));
    } else {
        hmac_vap->user_pmf_status |= flag << (hmac_user->assoc_id);
    }

    /* 如果没有改变 */
    if (user_pmf_old == hmac_vap->user_pmf_status) {
        return;
    }

    /* 硬件PMF控制开关填写 */
    hmac_11w_get_pmf_cap(hmac_vap, &pmf_cap);
    if (pmf_cap != MAC_PMF_DISABLED && hmac_vap->hal_vap != OSAL_NULL) {
        hal_set_pmf_crypto(hmac_vap->hal_vap, (oal_bool_enum_uint8)is_open_pmf_reg(hmac_vap));
    }
}

static osal_u32 hmac_11w_rx_filter_process(const hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf,
    const osal_u8 *da, const mac_ieee80211_frame_stru *frame_hdr, wlan_ciper_protocol_type_enum_uint8  cipher_type)
{
    osal_u32 relt;

    if (hmac_vap == OSAL_NULL || da == OSAL_NULL || frame_hdr == OSAL_NULL) {
        oam_error_log3(0, OAM_SF_PMF, "{hmac_11w_rx_filter_process::hmac_vap is %p, da is %p, frame_hdr is %p.}",
            (uintptr_t)hmac_vap, (uintptr_t)da, (uintptr_t)frame_hdr);
        return OAL_FAIL;
    }

    /* 广播Robust帧过滤 */
    if (ether_is_multicast(da) == OSAL_TRUE) {
        relt = OAL_SUCC;
        unref_param(netbuf);
        if (cipher_type != WLAN_80211_CIPHER_SUITE_BIP) {
            oam_warning_log1(0, OAM_SF_PMF, "{hmac_11w_rx_filter_process::Robust muti frame chipertype is[%d].}",
                cipher_type);
            return OAL_ERR_CODE_PMF_NO_PROTECTED;
        }
        return relt;
    }

    /* pmf使能，对硬件不能过滤的未加密帧进行过滤 */
    if (cipher_type == WLAN_80211_CIPHER_SUITE_NO_ENCRYP) {
        oam_warning_log1(0, OAM_SF_PMF,
            "vap_id[%d] {hmac_11w_rx_filter_process::PMF is open,but the Robust frame is CIPHER_SUITE_NO_ENCRYP.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PMF_NO_PROTECTED;
    }

    /* PMF单播管理帧校验 */
    if ((frame_hdr->frame_control.protected_frame == OSAL_FALSE) ||
        ((cipher_type != WLAN_80211_CIPHER_SUITE_CCMP) && (cipher_type != WLAN_80211_CIPHER_SUITE_CCMP_256))) {
        oam_warning_log3(0, OAM_SF_PMF,
            "vap_id[%d] {hmac_11w_rx_filter_process::robust_action incor. protected_frame[%d], cipher_type[%d].}",
            hmac_vap->vap_id, frame_hdr->frame_control.protected_frame, cipher_type);
        return OAL_ERR_CODE_PMF_NO_PROTECTED;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_11w_rx_filter
 功能描述  : 对接收强健管理帧的过滤操作
*****************************************************************************/
osal_u32 hmac_11w_rx_filter(const hmac_vap_stru *hmac_vap, const oal_netbuf_stru  *netbuf)
{
    const osal_u8 *da;
    const dmac_rx_ctl_stru *rx_ctl;
    const mac_ieee80211_frame_stru *frame_hdr;
    hmac_user_stru *hmac_user;
    osal_u16 ta_user_idx;
    wlan_ciper_protocol_type_enum_uint8 cipher_protocol_type;

    frame_hdr  = (const mac_ieee80211_frame_stru *)oal_netbuf_header_const(netbuf);
    da         = frame_hdr->address1;
    rx_ctl     = (const dmac_rx_ctl_stru *)oal_netbuf_cb_const(netbuf);
    ta_user_idx = rx_ctl->rx_info.ta_user_idx;
    cipher_protocol_type = hal_ctype_to_cipher_suite(rx_ctl->rx_status.cipher_protocol_type);

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(ta_user_idx);
    if (hmac_user == OSAL_NULL) {
        /* 此处warning打印去掉,否则未关联状态下接收管理帧会刷屏 */
        return OAL_SUCC;
    }

    if ((hmac_user->cap_info.pmf_active != OSAL_TRUE) ||
        (hmac_11w_robust_frame(netbuf, (const osal_u8 *)frame_hdr, OSAL_FALSE) != OSAL_TRUE)) {
        return OAL_SUCC;
    }

    return hmac_11w_rx_filter_process(hmac_vap, netbuf, da, frame_hdr, cipher_protocol_type);
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
