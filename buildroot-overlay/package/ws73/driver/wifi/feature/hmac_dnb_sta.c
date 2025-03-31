/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 动态窄带2.0功能所在文件
 * Date: 2022-10-18
 */

#include "hmac_dnb_sta.h"
#include "hmac_ccpriv.h"
#include "frw_util_notifier.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_DNB_STA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_u8 g_dnb_enable = OSAL_TRUE;

OSAL_STATIC osal_u8 *hmac_get_dnb_enable_status(osal_void)
{
    return &g_dnb_enable;
}

/* 设置hiex 私有IE */
OSAL_STATIC osal_void hmac_set_hiex_cap(osal_u8 *buffer, osal_u8 *ie_len)
{
    mac_hiex_vendor_ie_stru *hiex_vender_ie = OSAL_NULL;

    hiex_vender_ie = (mac_hiex_vendor_ie_stru *)buffer;
    (osal_void)memset_s(hiex_vender_ie, sizeof(mac_hiex_vendor_ie_stru), 0, sizeof(mac_hiex_vendor_ie_stru));

    hiex_vender_ie->vender.element_id = MAC_EID_VENDOR;
    hiex_vender_ie->vender.len = sizeof(mac_hiex_vendor_ie_stru) - MAC_IE_HDR_LEN;
    hiex_vender_ie->vender.oui[0] = (osal_u8)((MAC_WLAN_OUI_PUBLIC >> BIT_OFFSET_16) & 0xff);
    hiex_vender_ie->vender.oui[1] = (osal_u8)((MAC_WLAN_OUI_PUBLIC >> BIT_OFFSET_8) & 0xff);
    hiex_vender_ie->vender.oui[2] = (osal_u8)((MAC_WLAN_OUI_PUBLIC) & 0xff); // 2 oui数组索引
    hiex_vender_ie->vender.oui_type = MAC_PRIV_HIEX_IE;

    hiex_vender_ie->hiex_cap.bit_ersru_enable = OSAL_TRUE;

    *ie_len = sizeof(mac_hiex_vendor_ie_stru);
}

/* 设置私有IE */
OSAL_STATIC osal_void hmac_set_priv_cap(osal_u8 *buffer, osal_u8 *ie_len)
{
    mac_priv_vendor_ie_stru *vender_ie = OSAL_NULL;

    vender_ie = (mac_priv_vendor_ie_stru *)buffer;
    (osal_void)memset_s(vender_ie, sizeof(mac_priv_vendor_ie_stru), 0, sizeof(mac_priv_vendor_ie_stru));

    vender_ie->vender.element_id = MAC_EID_VENDOR;
    vender_ie->vender.len = sizeof(mac_priv_vendor_ie_stru) - MAC_IE_HDR_LEN;
    vender_ie->vender.oui[0] = (osal_u8)((MAC_EXT_VENDER_IE >> BIT_OFFSET_16) & 0xff);
    vender_ie->vender.oui[1] = (osal_u8)((MAC_EXT_VENDER_IE >> BIT_OFFSET_8) & 0xff);
    vender_ie->vender.oui[2] = (osal_u8)((MAC_EXT_VENDER_IE) & 0xff); // 2 oui数组索引
    vender_ie->vender.oui_type = MAC_PRIV_CAP_IE;

    vender_ie->priv_cap.bit_ersru_he_ltf_num = HMAC_MAX_HE_LTF_NUM;

    *ie_len = sizeof(mac_priv_vendor_ie_stru);
}

OSAL_STATIC osal_void hmac_set_dnb_cap(hmac_vap_stru *hmac_vap, osal_u8 **buffer)
{
    osal_u8 ie_len = 0;

    if ((hmac_vap == OSAL_NULL) || (!is_legacy_sta(hmac_vap)) ||
        (*buffer == OSAL_NULL) || (*hmac_get_dnb_enable_status() != OSAL_TRUE)) {
        return;
    }

    /* 设置动态窄带2.0使能位 */
    hmac_set_hiex_cap(*buffer, &ie_len);
    *buffer += ie_len;

    /* 设置最大HE-LTF数量 */
    hmac_set_priv_cap(*buffer, &ie_len);
    *buffer += ie_len;
}

OSAL_STATIC osal_bool hmac_dnb_reset(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;

    if (hmac_vap == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (!is_legacy_sta(hmac_vap)) {
        return OSAL_TRUE;
    }

    *hmac_get_dnb_enable_status() = OSAL_TRUE;

    return OSAL_TRUE;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/* 设置动态窄带2.0使能 */
OSAL_STATIC osal_s32 hmac_config_set_sta_dnb_on(hmac_vap_stru *hmac_vap, osal_u8 dnb_enable)
{
    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!is_legacy_sta(hmac_vap)) {
        oam_warning_log2(0, OAM_SF_DNB, "{hmac_config_set_sta_dnb_on:: non legacy sta:vap mode[%d],p2p mode[%d]}",
            hmac_vap->vap_mode, hmac_vap->p2p_mode);
        return OAL_FAIL;
    }

    *hmac_get_dnb_enable_status() = dnb_enable;

    return OAL_SUCC;
}

OSAL_STATIC osal_s32  hmac_ccpriv_sta_dnb_on(hmac_vap_stru *hmac_vap, const osal_s8 *pc_param)
{
    osal_s32 ul_ret;
    osal_s32 l_ret;
    osal_u8 sta_dnb_open = 0;

    ul_ret = hmac_ccpriv_get_u8_with_check_max(&pc_param, 1, &sta_dnb_open);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
                         "{hmac_ccpriv_sta_dnb_on::get sta_sta_dnb_open return err_code [%d]!}", ul_ret);
        return ul_ret;
    }

    l_ret = hmac_config_set_sta_dnb_on(hmac_vap, sta_dnb_open);
    if (osal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_DNB, "{uapi_ccpriv_sta_dnb_on::return err code [%d]!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}
#endif

osal_u32 hmac_sta_dnb_init(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"set_sta_dnb_on", hmac_ccpriv_sta_dnb_on);
#endif

    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_dnb_reset);

    hmac_feature_hook_register(HMAC_FHOOK_DNB_ADD_PRIVATE_IE, hmac_set_dnb_cap);

    return OAL_SUCC;
}

osal_void hmac_sta_dnb_deinit(osal_void)
{
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"set_sta_dnb_on");
#endif

    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DOWN_VAP, hmac_dnb_reset);

    hmac_feature_hook_unregister(HMAC_FHOOK_DNB_ADD_PRIVATE_IE);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif