/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Source file of mac ie.
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "mac_ie.h"
#include "mac_frame.h"
#include "mac_device_ext.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"
#include "hmac_feature_interface.h"
#include "hmac_11v.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_IE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
    全局变量定义
*****************************************************************************/

/*****************************************************************************
    函数实现
*****************************************************************************/
/*****************************************************************************
 功能描述 : 处理ht cap ie中的 supported channel width
*****************************************************************************/
oal_bool_enum_uint8 mac_ie_proc_ht_supported_channel_width_etc(const hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap,
    osal_u8 supp_chan_width, oal_bool_enum prev_asoc_ht)
{
    /* 不支持20/40Mhz频宽 */
    if (supp_chan_width == 0) {
        /*  如果STA之前没有作为HT站点与AP关联， 或者STA之前已经作为支持20/40Mhz频宽的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_FALSE) || (hmac_user->ht_hdl.supported_channel_width == OAL_TRUE)) {
            hmac_vap->protection.sta_20m_only_num++;
        }

        return OAL_FALSE;
    } else { /* 支持20/40Mhz频宽 */
        /*  如果STA之前已经作为不支持20/40Mhz频宽的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_TRUE) && (hmac_user->ht_hdl.supported_channel_width == OAL_FALSE)) {
            hmac_vap->protection.sta_20m_only_num--;
        }

        return OAL_TRUE;
    }
}

/*****************************************************************************
 功能描述 : 处理ht cap ie中的 ht green field BIT4
*****************************************************************************/
oal_bool_enum_uint8 mac_ie_proc_ht_green_field_etc(const hmac_user_stru *hmac_user, hmac_vap_stru *hmac_vap,
    osal_u8 ht_green_field, oal_bool_enum prev_asoc_ht)
{
    /* 不支持Greenfield */
    if (ht_green_field == 0) {
        /*  如果STA之前没有作为HT站点与AP关联， 或者STA之前已经作为支持GF的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_FALSE) || (hmac_user->ht_hdl.ht_green_field == OAL_TRUE)) {
            hmac_vap->protection.sta_non_gf_num++;
        }

        return OAL_FALSE;
    } else { /* 支持Greenfield */
        /*  如果STA之前已经作为不支持GF的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_TRUE) && (hmac_user->ht_hdl.ht_green_field == OAL_FALSE)) {
            hmac_vap->protection.sta_non_gf_num--;
        }

        return OAL_TRUE;
    }
}

/*****************************************************************************
 功能描述 : 处理ht cap ie中的 lsig_txop_protection_support
*****************************************************************************/
oal_bool_enum_uint8 hmac_ie_proc_lsig_txop_protection_support_etc(const hmac_user_stru *hmac_user,
    hmac_vap_stru *hmac_vap, osal_u8 lsig_txop_prot_supp, oal_bool_enum prev_asoc_ht)
{
    /* 不支持L-sig txop protection */
    if (lsig_txop_prot_supp == 0) {
        /*  如果STA之前没有作为HT站点与AP关联， 或者STA之前已经作为支持Lsig txop protection的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_FALSE) || (hmac_user->ht_hdl.lsig_txop_protection == OAL_TRUE)) {
            hmac_vap->protection.sta_no_lsig_txop_num++;
        }

        return OAL_FALSE;
    } else { /* 支持L-sig txop protection */
        /*  如果STA之前已经作为不支持Lsig txop protection的HT站点与AP关联 */
        if ((prev_asoc_ht == OAL_TRUE) && (hmac_user->ht_hdl.lsig_txop_protection == OAL_FALSE)) {
            hmac_vap->protection.sta_no_lsig_txop_num--;
        }

        return OAL_TRUE;
    }
}

static osal_void mac_ie_parse_ht_txbf(mac_user_ht_hdl_stru *ht_hdl, const osal_u8 *payload, osal_u16 offset)
{
    osal_u16 tmp_info_elem;
    osal_u16 tmp_txbf_low;
    osal_u32 tmp_txbf_elem;

    tmp_info_elem = OAL_MAKE_WORD16(payload[offset], payload[offset + 1]);
    tmp_txbf_low = OAL_MAKE_WORD16(payload[offset + 2], payload[offset + 3]);   /* 在offset基础上偏移2字节和3字节 */
    tmp_txbf_elem = OAL_MAKE_WORD32(tmp_info_elem, tmp_txbf_low);
    ht_hdl->imbf_receive_cap = (tmp_txbf_elem & BIT0);
    ht_hdl->receive_staggered_sounding_cap = ((tmp_txbf_elem & BIT1) >> 1);
    ht_hdl->transmit_staggered_sounding_cap = ((tmp_txbf_elem & BIT2) >> 2);    /* 右移2 bit */
    ht_hdl->receive_ndp_cap = ((tmp_txbf_elem & BIT3) >> 3);                    /* 右移3 bit */
    ht_hdl->transmit_ndp_cap = ((tmp_txbf_elem & BIT4) >> 4);                   /* 右移4 bit */
    ht_hdl->imbf_cap = ((tmp_txbf_elem & BIT5) >> 5);                           /* 右移5 bit */
    ht_hdl->calibration = ((tmp_txbf_elem & 0x000000C0) >> 6);                  /* 右移6 bit */
    ht_hdl->exp_csi_txbf_cap = ((tmp_txbf_elem & BIT8) >> 8);                   /* 右移8 bit */
    ht_hdl->exp_noncomp_txbf_cap = ((tmp_txbf_elem & BIT9) >> 9);               /* 右移9 bit */
    ht_hdl->exp_comp_txbf_cap = ((tmp_txbf_elem & BIT10) >> 10);                /* 右移10 bit */
    ht_hdl->exp_csi_feedback = ((tmp_txbf_elem & 0x00001800) >> 11);            /* 右移11 bit */
    ht_hdl->exp_noncomp_feedback = ((tmp_txbf_elem & 0x00006000) >> 13);        /* 右移13 bit */
    ht_hdl->exp_comp_feedback = ((tmp_txbf_elem & 0x0001C000) >> 15);           /* 右移15 bit */
    ht_hdl->min_grouping = ((tmp_txbf_elem & 0x00060000) >> 17);                /* 右移17 bit */
    ht_hdl->csi_bfer_ant_number = ((tmp_txbf_elem & 0x001C0000) >> 19);         /* 右移19 bit */
    ht_hdl->noncomp_bfer_ant_number = ((tmp_txbf_elem & 0x00600000) >> 21);     /* 右移21 bit */
    ht_hdl->comp_bfer_ant_number = ((tmp_txbf_elem & 0x01C00000) >> 23);        /* 右移23 bit */
    ht_hdl->csi_bfee_max_rows = ((tmp_txbf_elem & 0x06000000) >> 25);           /* 右移25 bit */
    ht_hdl->channel_est_cap = ((tmp_txbf_elem & 0x18000000) >> 27);             /* 右移27 bit */
}

/*****************************************************************************
 功能描述 : 搜索asoc rsp frame帧中的HT cap IE
*****************************************************************************/
osal_u32 mac_ie_proc_ht_sta_etc(const hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u16 offset_len,
    hmac_user_stru *mac_user_ap, osal_u16 *amsdu_maxsize)
{
    osal_u8 mcs_bmp_index;
    osal_u8 smps;
    mac_user_ht_hdl_stru *ht_hdl;
    mac_user_ht_hdl_stru ht_hdl_data;
    osal_u16 ht_cap_info;
    osal_u16 offset = offset_len;
    if ((hmac_vap == OSAL_NULL) || (payload == OSAL_NULL) || (amsdu_maxsize == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "mac_ie_proc_ht_sta_etc::param null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ht_hdl = &ht_hdl_data;
    hmac_user_get_ht_hdl_etc(mac_user_ap, ht_hdl);

    /* 带有 HT Capability Element 的 AP，标示它具有HT capable. */
    ht_hdl->ht_capable = OAL_TRUE;

    offset += MAC_IE_HDR_LEN;

    /* 解析 HT Capabilities Info Field */
    ht_cap_info = OAL_MAKE_WORD16(payload[offset], payload[offset + 1]);

    /* 检查STA所支持的LDPC编码能力 B0，0:不支持，1:支持 */
    ht_hdl->ldpc_coding_cap = (ht_cap_info & BIT0);

    /* 提取AP所支持的带宽能力  */
    ht_hdl->supported_channel_width = ((ht_cap_info & BIT1) >> 1);

    /* 检查空间复用节能模式 B2~B3 */
    smps = (osal_u8)((ht_cap_info & (BIT3 | BIT2)) >> 2);
    ht_hdl->sm_power_save = hmac_ie_proc_sm_power_save_field_etc(mac_user_ap, smps);

    /* 提取AP支持Greenfield情况 */
    ht_hdl->ht_green_field = ((ht_cap_info & BIT4) >> 4);               /* 右移4 bit */

    /* 提取AP支持20MHz Short-GI情况 */
    ht_hdl->short_gi_20mhz = ((ht_cap_info & BIT5) >> 5);               /* 右移5 bit */

    /* 提取AP支持40MHz Short-GI情况 */
    ht_hdl->short_gi_40mhz = ((ht_cap_info & BIT6) >> 6);               /* 右移6 bit */

    /* 提取AP支持STBC PPDU情况 */
    ht_hdl->rx_stbc = (osal_u8)((ht_cap_info & (BIT9 | BIT8)) >> 8);    /* 右移8 bit */

    /* 提取AP支持最大A-MSDU长度情况 */
    *amsdu_maxsize = WLAN_MIB_MAX_AMSDU_LENGTH_LONG;
    if ((ht_cap_info & BIT11) == 0) {
        *amsdu_maxsize = WLAN_MIB_MAX_AMSDU_LENGTH_SHORT;
    }

    mac_user_ap->amsdu_maxsize = *amsdu_maxsize;
    ht_hdl->max_amsdu_length = ((ht_cap_info & BIT11) >> 11);           /* 右移11 bit */
    /* 提取AP 40M上DSSS/CCK的支持情况 */
    ht_hdl->dsss_cck_mode_40mhz = ((ht_cap_info & BIT12) >> 12);        /* 右移12 bit */

    /* 提取AP L-SIG TXOP 保护的支持情况 */
    ht_hdl->lsig_txop_protection = ((ht_cap_info & BIT15) >> 15);       /* 右移15 bit */

    offset += MAC_HT_CAPINFO_LEN;

    /* 解析 A-MPDU Parameters Field */
    /* 提取 Maximum Rx A-MPDU factor (B1 - B0) */
    ht_hdl->max_rx_ampdu_factor = (payload[offset] & 0x03);

    /* 提取 Minmum Rx A-MPDU factor (B3 - B2) */
    ht_hdl->min_mpdu_start_spacing = (payload[offset] >> 2) & 0x07;

    offset += MAC_HT_AMPDU_PARAMS_LEN;

    /* 解析 Supported MCS Set Field */
    for (mcs_bmp_index = 0; mcs_bmp_index < WLAN_HT_MCS_BITMASK_LEN; mcs_bmp_index++) {
        ht_hdl->rx_mcs_bitmask[mcs_bmp_index] = (*(osal_u8 *)(payload + offset + mcs_bmp_index));
    }

    ht_hdl->rx_mcs_bitmask[WLAN_HT_MCS_BITMASK_LEN - 1] &= 0x1F;

    offset += MAC_HT_SUP_MCS_SET_LEN;

    /* 解析 HT Extended Capabilities Info Field */
    ht_cap_info = OAL_MAKE_WORD16(payload[offset], payload[offset + 1]);
    /* 提取 HTC support Information */
    if ((ht_cap_info & BIT10) != 0) {
        ht_hdl->htc_support = 1;
    }
    offset += MAC_HT_EXT_CAP_LEN;

    /* 解析 Tx Beamforming Field */
    mac_ie_parse_ht_txbf(ht_hdl, payload, offset);

    hmac_user_set_ht_hdl_etc(mac_user_ap, ht_hdl);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 功能描述 : 检查action帧是不是p2p帧
*****************************************************************************/
oal_bool_enum_uint8 hmac_ie_check_p2p_action_etc(osal_u8 *payload)
{
    /* 找到WFA OUI */
    if ((osal_memcmp(payload, g_p2p_oui_etc, MAC_OUI_LEN) == 0) && (payload[MAC_OUI_LEN] == MAC_OUITYPE_P2P)) {
        /*  找到WFA P2P v1.0 oui type */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
#endif

static osal_u32 mac_ie_check_end(const osal_u8 *end_ie, const osal_u8 *tmp_ie, osal_u8 ie_len)
{
    if (end_ie == tmp_ie) {
        return OAL_SUCC;
    } else if (end_ie < tmp_ie) {
        oam_warning_log1(0, OAM_SF_CFG, "mac_ie_check_end fail, end_ie less then tmp_ie, ie len[%u].", ie_len);
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述 : 检查rsnie的报文格式
*****************************************************************************/
static osal_u32 hmac_ie_check_rsn_cipher_format(const osal_u8 *src_ie, osal_u8 ie_len)
{
    osal_u16 pairwise_suite_count, akm_suite_count, pmkid_count;

    /* 前置条件， RSN-IE的length刚好等于ie缓冲区长度 */
    const osal_u8 *end_ie = (osal_u8 *)src_ie + ie_len;

    const osal_u8 *tmp_ie = src_ie + 2;    /* 在src_ie的基础上后移2字节, Version唯一必选项必须包含, 后续所有字段均可选，随时可以结束 */
    osal_u32 ret = mac_ie_check_end(end_ie, tmp_ie, ie_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    tmp_ie += 4; /* Group Data Cipher Suite 4个字节 */
    ret = mac_ie_check_end(end_ie, tmp_ie, ie_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    pairwise_suite_count = oal_make_word16(tmp_ie[0], tmp_ie[1]);
    tmp_ie += 2 + pairwise_suite_count * 4; /* 后移2字节的pairwise suite 加上4倍pairwise suite个数的字节数 */
    ret = mac_ie_check_end(end_ie, tmp_ie, ie_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    akm_suite_count = oal_make_word16(tmp_ie[0], tmp_ie[1]);
    tmp_ie += 2 + akm_suite_count * 4; /* 后移2字节的akm suite count加上4倍akm suite count个数的字节数 */
    ret = mac_ie_check_end(end_ie, tmp_ie, ie_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    tmp_ie += 2; /* 后移2字节，RSN Capabilities */
    ret = mac_ie_check_end(end_ie, tmp_ie, ie_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    pmkid_count = oal_make_word16(tmp_ie[0], tmp_ie[1]);
    tmp_ie += 2 + pmkid_count * 16; /* 后移2字节加上16倍pmkid count个数的字节数 */
    ret = mac_ie_check_end(end_ie, tmp_ie, ie_len);
    if (ret != OAL_CONTINUE) {
        return ret;
    }

    tmp_ie += 4; /* 后移4字节,加上 Group Management Cipher Suite */
    if (end_ie >= tmp_ie) {
        return OAL_SUCC;
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "mac_ie_check_rsn_cipher_format, Group Management Cipher Suite or len wrong.");
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
}


/*****************************************************************************
 功能描述 : 更新ht cap ie中的 sm power save field B2~B3
*****************************************************************************/
wlan_mib_mimo_power_save_enum_uint8 hmac_ie_proc_sm_power_save_field_etc(hmac_user_stru *hmac_user, osal_u8 smps)
{
    unref_param(hmac_user);
    if (smps == MAC_SMPS_STATIC_MODE) {
        return WLAN_MIB_MIMO_POWER_SAVE_STATIC;
    } else if (smps == MAC_SMPS_DYNAMIC_MODE) {
        return WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC;
    } else {
        return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
 功能描述 : 判断帧类型是否为(重)关联请求/响应
*****************************************************************************/
osal_u32 hmac_check_is_assoc_frame_etc(osal_u8 mgmt_frm_type)
{
    if ((mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_RSP) || (mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_REQ) ||
        (mgmt_frm_type == WLAN_FC0_SUBTYPE_REASSOC_RSP) || (mgmt_frm_type == WLAN_FC0_SUBTYPE_ASSOC_REQ)) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
 函 数 名  : hmac_ie_find_vendor_vht_ie
 功能描述  : 查找11ac 私有 vendor vht ie
*****************************************************************************/
osal_u8  *hmac_ie_find_vendor_vht_ie(osal_u8 *frame, osal_u16 frame_len)
{
    osal_u8    *vendor_ie = OSAL_NULL;
    osal_u8    *vht_ie = OSAL_NULL;
    osal_u16    offset_vendor_vht = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;

    vendor_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_BROADCOM_EPIGRAM,
                                        MAC_WLAN_OUI_VENDOR_VHT_TYPE, frame, frame_len);
    if ((vendor_ie != OSAL_NULL) && (vendor_ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        vht_ie = mac_find_ie_etc(MAC_EID_VHT_CAP,
            vendor_ie + offset_vendor_vht, vendor_ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER);
    }

    return vht_ie;
}
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 功能描述 : 处理HE CAP IE中MAC cap和PHY Cap 固定长度字段
*****************************************************************************/
osal_u32 hmac_ie_parse_he_cap(osal_u8 *he_cap_ie, mac_frame_he_cap_ie_stru *he_cap_value)
{
    osal_u8 *he_buffer;

    /* 解析he cap IE */
    if ((he_cap_ie == OAL_PTR_NULL) || (he_cap_value == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_11AX, "hmac_ie_parse_he_cap::param null.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |EID Extension|HE MAC Capa. Info |HE PHY Capa. Info|
    -------------------------------------------------------------------------
    |1   |1      |1            |6                 |9                |
    -------------------------------------------------------------------------
    |Tx Rx HE MCS NSS Support |PPE Thresholds(Optional)|
    -------------------------------------------------------------------------
    |4or8or12                 |Variable                |
    -------------------------------------------------------------------------
    ***************************************************************************/
    if (he_cap_ie[1] < MAC_HE_CAP_MIN_LEN) {
        oam_warning_log1(0, OAM_SF_11AX, "hmac_proc_he_cap_ie::invalid he cap ie len[%d].", he_cap_ie[1]);
        return OAL_FAIL;
    }

    he_buffer = he_cap_ie + 3;  /* he_cap_ie头部偏移3字节 */
    (osal_void)memcpy_s((osal_u8 *)(he_cap_value), sizeof(mac_frame_he_cap_ie_stru),
        he_buffer, sizeof(mac_frame_he_cap_ie_stru));

    /* HE-MCS and NSS 160MHz PPE thresholds 暂不解析 */

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 解析帧中的HE Oper IE
*****************************************************************************/
osal_u32 hmac_ie_parse_he_oper(osal_u8 *he_oper_ie, mac_frame_he_oper_ie_stru *he_oper_ie_data)
{
    osal_u8 *ie_buffer = OSAL_NULL;
    mac_frame_he_operation_param_stru *he_oper_param = OSAL_NULL;
    mac_frame_he_bss_color_info_stru *bss_color_info = OSAL_NULL;
    mac_frame_he_mcs_nss_bit_map_stru *he_basic_mcs_nss = OSAL_NULL;
    mac_frame_vht_operation_info_stru *vht_operation_info = OSAL_NULL;

    if (osal_unlikely((he_oper_ie == OSAL_NULL) || (he_oper_ie_data == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_11AX, "hmac_ie_parse_he_oper::param null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |EID Extension|HE Operation Parameters|Basic HE MCS Ans NSS Set|
    -------------------------------------------------------------------------
    |1   |1          |1                  |           4                     |              3                       |
    -------------------------------------------------------------------------
    |VHT Operation Info  |MaxBssid Indicator|
    -------------------------------------------------------------------------
    |      0 or 3              |0 or More             |
    -------------------------------------------------------------------------
    ***************************************************************************/
    if (he_oper_ie[1] < MAC_HE_OPERAION_MIN_LEN) {
        oam_warning_log1(0, OAM_SF_11AX, "hmac_ie_parse_he_oper::invalid he oper ie len[%d].", he_oper_ie[1]);
        return OAL_FAIL;
    }

    ie_buffer = he_oper_ie + 3; /* he_oper_ie头部偏移3字节 */

    /* 解析HE Operation Parameters */
    he_oper_param = (mac_frame_he_operation_param_stru *)ie_buffer;

    ie_buffer += MAC_HE_OPE_PARAM_LEN;
    he_oper_ie_data->he_oper_param.default_pe_duration = he_oper_param->default_pe_duration;
    he_oper_ie_data->he_oper_param.twt_required = he_oper_param->twt_required;
    he_oper_ie_data->he_oper_param.txop_duration_rts_threshold = he_oper_param->txop_duration_rts_threshold;
    he_oper_ie_data->he_oper_param.vht_operation_info_present = he_oper_param->vht_operation_info_present;
    he_oper_ie_data->he_oper_param.er_su_disable = he_oper_param->er_su_disable;

    bss_color_info = (mac_frame_he_bss_color_info_stru *)ie_buffer;
    ie_buffer += sizeof(mac_frame_he_bss_color_info_stru);
    (osal_void)memcpy_s((osal_u8 *)(&he_oper_ie_data->bss_color), sizeof(mac_frame_he_bss_color_info_stru),
        bss_color_info, sizeof(mac_frame_he_bss_color_info_stru));

    /* 解析Basic HE MCS And NSS Set */
    he_basic_mcs_nss = (mac_frame_he_mcs_nss_bit_map_stru *)ie_buffer;
    ie_buffer += MAC_HE_OPE_BASIC_MCS_NSS_LEN;
    (osal_void)memcpy_s((osal_u8 *)(&he_oper_ie_data->he_basic_mcs_nss), sizeof(mac_frame_he_mcs_nss_bit_map_stru),
        he_basic_mcs_nss, sizeof(mac_frame_he_mcs_nss_bit_map_stru));

    if ((he_oper_ie_data->he_oper_param.vht_operation_info_present == 1) &&
        (he_oper_ie[1] >= MAC_HE_OPERAION_WITH_VHT_LEN)) {
        vht_operation_info = (mac_frame_vht_operation_info_stru *)ie_buffer;
        ie_buffer += MAC_HE_VHT_OPERATION_INFO_LEN;
        (osal_void)memcpy_s((osal_u8 *)(&he_oper_ie_data->vht_operation_info),
            sizeof(mac_frame_vht_operation_info_stru), vht_operation_info, sizeof(mac_frame_vht_operation_info_stru));
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 解析mu edca parameter IE
*****************************************************************************/
osal_u32 hmac_ie_parse_mu_edca_parameter(osal_u8 *edca_ie, mac_frame_he_mu_edca_parameter_ie_stru *he_mu_edca_value)
{
    mac_frame_he_mu_edca_parameter_ie_stru *he_edca = OSAL_NULL;
    osal_u8 *he_edca_ie = edca_ie;
    if (osal_unlikely((he_edca_ie == OSAL_NULL) || (he_mu_edca_value == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_11AX, "hmac_ie_parse_mu_edca_parameter::param null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /************************ MU EDCA Parameter Set Element ***************************/
    /* ------------------------------------------------------------------------------------------- */
    /* | EID | LEN | Ext EID|MU Qos Info |MU AC_BE Parameter Record | MU AC_BK Parameter Record  | */
    /* ------------------------------------------------------------------------------------------- */
    /* |  1  |  1  |   1    |    1       |     3                    |        3                   | */
    /* ------------------------------------------------------------------------------------------- */
    /* ------------------------------------------------------------------------------ -------------*/
    /* | MU AC_VI Parameter Record | MU AC_VO Parameter Record                                   | */
    /* ------------------------------------------------------------------------------------------- */
    /* |    3                      |     3                                                       | */

    /******************* QoS Info field when sent from WMM AP *****************/
    /* --------------------------------------------------------------------------------------------*/
    /*    | EDCA Parameter Set Update Count | Q-Ack | Queue Request |TXOP Request | More Data Ack| */
    /*---------------------------------------------------------------------------------------------*/
    /* bit |        0~3                      |  1    |  1            |   1         |     1        | */
    /*---------------------------------------------------------------------------------------------*/
    /**************************************************************************/

    if (he_edca_ie[1] != MAC_HE_MU_EDCA_PARAMETER_SET_LEN) {
        oam_warning_log1(0, OAM_SF_11AX, "hmac_ie_parse_mu_edca_parameter::invalid mu edca ie len[%d].", he_edca_ie[1]);
        return OAL_FAIL;
    }

    he_edca_ie = he_edca_ie + 3;    /* he_edca_ie头部偏移3字节 */

    /* 解析HE MU EDCA  Parameters Set Element */
    he_edca = (mac_frame_he_mu_edca_parameter_ie_stru *)he_edca_ie;
    (osal_void)memcpy_s((osal_u8 *)(he_mu_edca_value), sizeof(mac_frame_he_mu_edca_parameter_ie_stru),
        he_edca, sizeof(mac_frame_he_mu_edca_parameter_ie_stru));

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 处理he Ope IE
*****************************************************************************/
osal_u32 hmac_ie_proc_he_opern_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload, hmac_user_stru *hmac_user)
{
    mac_frame_he_oper_ie_stru he_oper_ie_data;
    mac_he_hdl_stru he_hdl;
    wlan_mib_vht_op_width_enum_uint8 channel_width_old;
    osal_u8 freq_seg0_old;
    osal_u8 vap_id;
    osal_u32 rts_txop_thred;
    osal_u32 ret;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (hmac_user == OAL_PTR_NULL) || (payload == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_11AX, "hmac_ie_proc_he_opern_ie::param null.");
        return MAC_NO_CHANGE;
    }
    vap_id = hmac_vap->hal_vap->vap_id;
    rts_txop_thred = mac_mib_get_txop_duration_rts_threshold(hmac_vap);

    (osal_void)memset_s(&he_oper_ie_data, sizeof(he_oper_ie_data), 0, sizeof(he_oper_ie_data));

    ret = hmac_ie_parse_he_oper(payload, &he_oper_ie_data);
    if (ret != OAL_SUCC) {
        return MAC_NO_CHANGE;
    }

    mac_user_get_he_hdl(hmac_user, &he_hdl);

    if ((he_oper_ie_data.bss_color.bss_color != he_hdl.he_oper_ie.bss_color.bss_color) &&
        (he_oper_ie_data.bss_color.bss_color_disable != 1)) {
        ret |= MAC_HE_BSS_COLOR_CHANGE;
        /* 识别bss color需要标记,dmac设置 */
        he_hdl.bss_color = he_oper_ie_data.bss_color.bss_color;
        he_hdl.bss_color_exist = OSAL_TRUE;
    }

    if ((he_oper_ie_data.bss_color.partial_bss_color != he_hdl.he_oper_ie.bss_color.partial_bss_color) &&
        (he_oper_ie_data.bss_color.bss_color_disable != 1)) {
        ret |= MAC_HE_PARTIAL_BSS_COLOR_CHANGE;
    }

    if (he_oper_ie_data.bss_color.bss_color_disable == 1) {
        ret |= MAC_HE_BSS_COLOR_DISALLOW;
    }

    if ((he_oper_ie_data.he_oper_param.txop_duration_rts_threshold != 0) &&
        (rts_txop_thred != he_oper_ie_data.he_oper_param.txop_duration_rts_threshold)) {
        ret |= MAC_RTS_TXOP_THRED_CHANGE;
        mac_mib_set_txop_duration_rts_threshold(hmac_vap, he_oper_ie_data.he_oper_param.txop_duration_rts_threshold);
        if ((hmac_vap->p2p_mode != WLAN_P2P_DEV_MODE) && (hmac_vap->p2p_mode != WLAN_P2P_CL_MODE)) {
            hal_vap_set_txop_duration_threshold(he_oper_ie_data.he_oper_param.txop_duration_rts_threshold, vap_id);
        }
    }

    (osal_void)memcpy_s(&he_hdl.he_oper_ie, sizeof(mac_frame_he_oper_ie_stru), &he_oper_ie_data,
        sizeof(mac_frame_he_oper_ie_stru));

    if (he_oper_ie_data.he_oper_param.vht_operation_info_present == 1) {
        channel_width_old = he_hdl.he_oper_ie.vht_operation_info.channel_width;
        freq_seg0_old = he_hdl.he_oper_ie.vht_operation_info.center_freq_seg0;
        if (he_oper_ie_data.vht_operation_info.channel_width > WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
            oam_warning_log1(0, OAM_SF_11AX, "hmac_ie_proc_he_opern_ie:invalid channel width[%d],use 20M chn width.",
                he_oper_ie_data.vht_operation_info.channel_width);
            he_oper_ie_data.vht_operation_info.channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
        }

        if (he_oper_ie_data.vht_operation_info.channel_width > WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
            oam_warning_log1(0, OAM_SF_11AX, "hmac_ie_proc_he_opern_ie:invalid channel width[%d],use 20M chn width.",
                he_oper_ie_data.vht_operation_info.channel_width);
            he_hdl.channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
        }

        if ((channel_width_old != he_oper_ie_data.vht_operation_info.channel_width) ||
            (freq_seg0_old != he_oper_ie_data.vht_operation_info.center_freq_seg0)) {
            ret = MAC_HE_CHANGE;
            oam_warning_log4(hmac_vap->vap_id, OAM_SF_11AX,
                "hmac_ie_proc_he_opern_ie:usr_bw update channel_width [%d]~[%d],channel_center_freq_seg0 [%d]~[%d]",
                channel_width_old, he_oper_ie_data.vht_operation_info.channel_width,
                freq_seg0_old, he_oper_ie_data.vht_operation_info.center_freq_seg0);
        }
    }

    mac_user_set_he_hdl(hmac_user, &he_hdl);
    return ret;
}

/*****************************************************************************
 功能描述 : 解析bss color
*****************************************************************************/
osal_u32 hmac_ie_parse_he_bss_color_change_announcement_ie(osal_u8 *payload,
    mac_frame_bss_color_change_annoncement_ie_stru *bss_color)
{
    mac_frame_bss_color_change_annoncement_ie_stru *bss_color_info;
    osal_u8 *data;

    if (payload[1] != MAC_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT_LEN) {
        return OAL_FAIL;
    }

    data = payload + 3; /* payload偏移3字节 */

    bss_color_info = (mac_frame_bss_color_change_annoncement_ie_stru *)data;
    (osal_void)memcpy_s(bss_color, OAL_SIZEOF(mac_frame_bss_color_change_annoncement_ie_stru),
        bss_color_info, OAL_SIZEOF(mac_frame_bss_color_change_annoncement_ie_stru));

    return OAL_SUCC;
}

#endif /* _PRE_WLAN_FEATURE_11AX */

/*****************************************************************************
 功能描述 : 处理Extended Capabilities IE
*****************************************************************************/
osal_u32 hmac_ie_proc_ext_cap_ie_etc(hmac_user_stru *hmac_user, osal_u8 *payload)
{
    mac_user_cap_info_stru *cap_info;
    osal_u8 len;
    osal_u8 cap[sizeof(mac_ext_cap_ie_stru)] = {0};
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11V_SET_BSS_TRANSITION);

    if (OAL_UNLIKELY((hmac_user == OAL_PTR_NULL) || (payload == OAL_PTR_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "hmac_ie_proc_ext_cap_ie_etc::param null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cap_info = &(hmac_user->cap_info);
    len = payload[1];
    if (len >= MAC_IE_HDR_LEN && len <= osal_array_size(cap)) {
        /* ie长度域的值本身不包含IE头长度，此处不需要另行减去头长 */
        (osal_void)memcpy_s(cap, len, &payload[MAC_IE_HDR_LEN], len);
    }

    /* 提取 BIT12: 支持proxy arp */
    cap_info->proxy_arp = ((cap[1] & BIT4) == 0) ? OAL_FALSE : OAL_TRUE;

    if (fhook != OSAL_NULL) {
        ((hmac_11v_set_bss_transition_cb)fhook)(cap_info, cap);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
 功能描述 : 检查Operating Mode字段参数是否合理
*****************************************************************************/
static osal_u32 hmac_ie_check_proc_opmode_param(hmac_user_stru *hmac_user, mac_opmode_notify_stru *opmode_notify)
{
    /* USER新限定带宽、空间流不允许大于其能力 */
    if ((hmac_user->bandwidth_cap < opmode_notify->channel_width) ||
        (hmac_user->user_num_spatial_stream < opmode_notify->rx_nss)) {
        oam_warning_log4(0, OAM_SF_OPMODE,
            "hmac_ie_check_proc_opmode_param:bandwidth_cap[%d],opmode bandwidth[%d],spatial stream[%d],rx nss[%d]!",
            hmac_user->bandwidth_cap, opmode_notify->channel_width, hmac_user->user_num_spatial_stream,
            opmode_notify->rx_nss);
        return OAL_FAIL;
    }

    /* Nss Type值为1，则表示beamforming Rx Nss不能超过其声称值 */
    if (opmode_notify->rx_nss_type == 1) {
        if (hmac_user->vht_hdl.num_bf_ant_supported < opmode_notify->rx_nss) {
            oam_warning_log2(0, OAM_SF_OPMODE,
                "hmac_ie_check_proc_opmode_param:rx_nss over limit!num_bf_ant_supported:[%d],rx_nss:[%d]",
                hmac_user->vht_hdl.num_bf_ant_supported, opmode_notify->rx_nss);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 处理Operating Mode字段
*****************************************************************************/
osal_u32 hmac_ie_proc_opmode_field_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_opmode_notify_stru *opmode_notify)
{
    wlan_bw_cap_enum_uint8 bwcap_vap;     /* vap自身带宽能力 */
    wlan_bw_cap_enum_uint8 avail_bw;      /* vap自身带宽能力 */
    osal_u8 old_avail_bandwidth             = hmac_user->avail_bandwidth;
    osal_u8 old_cur_bandwidth               = hmac_user->cur_bandwidth;
    osal_u8 old_avail_bf_num_spatial_stream = hmac_user->avail_bf_num_spatial_stream;
    osal_u8 old_avail_num_spatial_stream    = hmac_user->avail_num_spatial_stream;

    /* 入参指针已经在调用函数保证非空，这里直接使用即可 */
    if (hmac_ie_check_proc_opmode_param(hmac_user, opmode_notify) == OAL_FAIL) {
        oam_warning_log1(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_ie_proc_opmode_field_etc::check_proc_opmode_param return fail!}", hmac_user->vap_id);
        return OAL_FAIL;
    }

    /* 判断channel_width是否与user之前使用channel_width相同 */
    if (opmode_notify->channel_width != hmac_user->avail_bandwidth) {
        oam_info_log3(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_ie_proc_opmode_field_etc::channel_width = [%x], avail_bw = [%x]!}", hmac_vap->vap_id,
            opmode_notify->channel_width, hmac_user->avail_bandwidth);

        /* 获取vap带宽能力与用户带宽能力的交集 */
        hmac_vap_get_bandwidth_cap_etc(hmac_vap, &bwcap_vap);
        avail_bw = osal_min(bwcap_vap, opmode_notify->channel_width);
        hmac_user_set_bandwidth_info_etc(hmac_user, avail_bw, avail_bw);

        oam_info_log3(0, OAM_SF_OPMODE,
            "vap_id[%d] {hmac_ie_proc_opmode_field_etc::change bandwidth. bwcap_vap = [%x], avail_bw = [%x]!}",
            hmac_vap->vap_id, bwcap_vap, hmac_user->avail_bandwidth);
    }

    /* 判断Rx Nss Type是否为beamforming模式 */
    if (opmode_notify->rx_nss_type  == 1) {
        oam_info_log1(0, 0, "vap_id[%d] {hmac_ie_proc_opmode_field_etc::rx_nss_type == 1!}", hmac_vap->vap_id);

        /* 判断Rx Nss是否与user之前使用Rx Nss相同 */
        if (opmode_notify->rx_nss != hmac_user->avail_bf_num_spatial_stream) {
            /* 需要获取vap和更新nss的取小，如果我们不支持mimo了，对端宣传切换mimo也不执行 */
            hmac_user_avail_bf_num_spatial_stream_etc(hmac_user, osal_min(hmac_vap->vap_rx_nss, opmode_notify->rx_nss));
        }
    } else {
        /* 判断Rx Nss是否与user之前使用Rx Nss相同 */
        if (opmode_notify->rx_nss != hmac_user->avail_num_spatial_stream) {
            oam_info_log3(0, OAM_SF_OPMODE,
                "vap_id[%d] {hmac_ie_proc_opmode_field_etc::_rx_nss = [%x], avail_num_spatial_stream = [%x]!}",
                hmac_vap->vap_id, opmode_notify->rx_nss, hmac_user->avail_num_spatial_stream);

            /* 需要获取vap和更新nss的取小，如果我们不支持mimo了，对端宣传切换mimo也不执行 */
            hmac_user_set_avail_num_spatial_stream_etc(hmac_user,
                osal_min(hmac_vap->vap_rx_nss, opmode_notify->rx_nss));

            oam_info_log3(0, OAM_SF_OPMODE,
                "vap_id[%d] {hmac_ie_proc_opmode_field_etc::vap_rx_nss = [%x],avail_num_spatial_stream = [%x]!}",
                hmac_vap->vap_id, hmac_vap->vap_rx_nss, hmac_user->avail_num_spatial_stream);
        }
    }

    if (old_avail_bandwidth != hmac_user->avail_bandwidth || old_cur_bandwidth != hmac_user->cur_bandwidth ||
        old_avail_bf_num_spatial_stream != hmac_user->avail_bf_num_spatial_stream ||
        old_avail_num_spatial_stream != hmac_user->avail_num_spatial_stream) {
        /* 带宽/空间流更新 则同步至device */
        hmac_user_sync(hmac_user);
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述 : 从帧体中解析ie中的chan信息，先在HT operation IE中找chan信息，如果找到
            就返回，如找不到，再在DSSS Param set ie中寻找
*****************************************************************************/
osal_u8 hmac_ie_get_chan_num_etc(osal_u8 *frame_body, osal_u16 frame_len, osal_u16 offset, osal_u8 curr_chan)
{
    osal_u8 chan_num;
    osal_u8 *ie_start_addr;

    /* 在DSSS Param set ie中解析chan num */
    ie_start_addr = mac_find_ie_etc(MAC_EID_DSPARMS, frame_body + offset, frame_len - offset);
    if ((ie_start_addr != OSAL_NULL) && (ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        chan_num = ie_start_addr[MAC_IE_HDR_LEN];
        if (hmac_is_channel_num_valid_etc(mac_get_band_by_channel_num(chan_num), chan_num) == OAL_SUCC) {
            return chan_num;
        }
    }

    /* 在HT operation ie中解析 chan num */
    ie_start_addr = mac_find_ie_etc(MAC_EID_HT_OPERATION, frame_body + offset, frame_len - offset);
    if ((ie_start_addr != OSAL_NULL) && (ie_start_addr[1] >= 1)) {
        chan_num = ie_start_addr[MAC_IE_HDR_LEN];
        if (hmac_is_channel_num_valid_etc(mac_get_band_by_channel_num(chan_num), chan_num) == OAL_SUCC) {
            return chan_num;
        }
    }

    chan_num = curr_chan;
    return chan_num;
}

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
/*****************************************************************************
 函 数 名  : hmac_ie_proc_chwidth_field_etc
 功能描述  : 处理Channel Width Field
*****************************************************************************/
osal_u32 hmac_ie_proc_chwidth_field_etc(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 chwidth)
{
    wlan_bw_cap_enum_uint8 bwcap_vap;  /* vap自身带宽能力 */
    wlan_bw_cap_enum_uint8 bwcap_user; /* user之前的带宽信息 */

    if (osal_unlikely((hmac_user == OSAL_NULL) || (hmac_vap == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_2040, "{hmac_ie_proc_opmode_field_etc:: param null !}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    bwcap_user = hmac_user->avail_bandwidth;

    hmac_vap_get_bandwidth_cap_etc(hmac_vap, &bwcap_vap);
    bwcap_vap = osal_min(bwcap_vap, (wlan_bw_cap_enum_uint8)chwidth);
    hmac_user_set_bandwidth_info_etc(hmac_user, bwcap_vap, bwcap_vap);

    if (bwcap_user != hmac_user->avail_bandwidth) {
        /* 带宽更新 则同步至device */
        hmac_user_sync(hmac_user);
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述 : 处理HT Operation IE
*****************************************************************************/
osal_u32 hmac_proc_ht_opern_ie_etc(hmac_vap_stru *hmac_vap, osal_u8 *payload, hmac_user_stru *hmac_user)
{
    mac_ht_opern_stru *ht_opern = OSAL_NULL;
    mac_user_ht_hdl_stru ht_hdl;
    osal_u32 change = MAC_NO_CHANGE;
    mac_sec_ch_off_enum_uint8 chan_offset_old;
    osal_u8 sta_chan_width_old;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (payload == OSAL_NULL) || (hmac_user == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_proc_ht_opern_ie_etc::param null.}");
        return change;
    }

    /* 长度校验，此处仅用到前6字节，后面Basic MCS Set未涉及 */
    if (payload[1] < 6) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_proc_ht_opern_ie_etc::invalid ht opern ie len[%d].}",
            hmac_vap->vap_id, payload[1]);
        return change;
    }

    hmac_user_get_ht_hdl_etc(hmac_user, &ht_hdl);

    chan_offset_old = ht_hdl.secondary_chan_offset;
    sta_chan_width_old = ht_hdl.sta_chan_width;

    /************************ HT Operation Element *************************************
      ----------------------------------------------------------------------
      |EID |Length |PrimaryChannel |HT Operation Information |Basic MCS Set|
      ----------------------------------------------------------------------
      |1   |1      |1              |5                        |16           |
      ----------------------------------------------------------------------
    ***************************************************************************/

    /************************ HT Information Field ****************************
     |--------------------------------------------------------------------|
     | Primary | Seconday  | STA Ch | RIFS |           reserved           |
     | Channel | Ch Offset | Width  | Mode |                              |
     |--------------------------------------------------------------------|
     |    1    | B0     B1 |   B2   |  B3  |    B4                     B7 |
     |--------------------------------------------------------------------|

     |----------------------------------------------------------------|
     |     HT     | Non-GF STAs | resv      | OBSS Non-HT  | Reserved |
     | Protection |   Present   |           | STAs Present |          |
     |----------------------------------------------------------------|
     | B0     B1  |     B2      |    B3     |     B4       | B5   B15 |
     |----------------------------------------------------------------|

     |-------------------------------------------------------------|
     | Reserved |  Dual  |  Dual CTS  | Seconday | LSIG TXOP Protn |
     |          | Beacon | Protection |  Beacon  | Full Support    |
     |-------------------------------------------------------------|
     | B0    B5 |   B6   |     B7     |     B8   |       B9        |
     |-------------------------------------------------------------|

     |---------------------------------------|
     |  PCO   |  PCO  | Reserved | Basic MCS |
     | Active | Phase |          |    Set    |
     |---------------------------------------|
     |  B10   |  B11  | B12  B15 |    16     |
     |---------------------------------------|
    **************************************************************************/

    ht_opern = (mac_ht_opern_stru *)(&payload[MAC_IE_HDR_LEN]);

    /* 提取HT Operation IE中的"Secondary Channel Offset" */
    ht_hdl.secondary_chan_offset = ht_opern->secondary_chan_offset;

    /* 在2.4G用户声称20M情况下该变量不切换 */
    if ((ht_opern->sta_chan_width == WLAN_BAND_WIDTH_20M) && (hmac_vap->channel.band == WLAN_BAND_2G)) {
        ht_hdl.secondary_chan_offset = MAC_SCN;
    }

    /* 保护相关 */
    ht_hdl.rifs_mode = ht_opern->rifs_mode; /* 发送描述符填写时候需要此值 */
    ht_hdl.ht_protection = ht_opern->ht_protection;
    ht_hdl.nongf_sta_present = ht_opern->nongf_sta_present;
    ht_hdl.obss_nonht_sta_present = ht_opern->obss_nonht_sta_present;
    ht_hdl.lsig_txop_protection_full_support = ht_opern->lsig_txop_protection_full_support;
    ht_hdl.sta_chan_width = ht_opern->sta_chan_width;

    hmac_user_set_ht_hdl_etc(hmac_user, &ht_hdl);

    if ((chan_offset_old != ht_hdl.secondary_chan_offset) || (sta_chan_width_old != ht_hdl.sta_chan_width)) {
        change = MAC_HT_CHANGE;
        oam_warning_log4(0, OAM_SF_ANY,
            "hmac_proc_ht_opern_ie_etc:usr_bw is updated second_chan_offset from %d to %d,chan_with from %d to %d",
            chan_offset_old, ht_hdl.secondary_chan_offset, sta_chan_width_old, ht_hdl.sta_chan_width);
    }

    return change;
}

/*****************************************************************************
 函 数 名  : hmac_ie_proc_vht_opern_ie_etc
 功能描述  : 处理VHT Operation IE
*****************************************************************************/
osal_u32 hmac_ie_proc_vht_opern_ie_etc(const hmac_vap_stru *hmac_vap, osal_u8 *payload, hmac_user_stru *hmac_user)
{
    mac_vht_hdl_stru vht_hdl;
    osal_u8 ret = MAC_NO_CHANGE;
    osal_u16 basic_mcs_set;
    wlan_mib_vht_op_width_enum_uint8 channel_width_old;
    osal_u8 freq_seg0_old;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (payload == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_ie_proc_vht_opern_ie_etc::param null.}");

        return ret;
    }

    /* 长度校验 */
    if (payload[1] < MAC_VHT_OPERN_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ie_proc_vht_opern_ie_etc::invalid vht opern len[%d].}", payload[1]);
        return ret;
    }

    hmac_user_get_vht_hdl_etc(hmac_user, &vht_hdl);

    channel_width_old = vht_hdl.channel_width;
    freq_seg0_old = vht_hdl.channel_center_freq_seg0;

    /* 解析 "VHT Operation Information" */
    vht_hdl.channel_width = payload[MAC_IE_HDR_LEN];
    vht_hdl.channel_center_freq_seg0 = payload[MAC_IE_HDR_LEN + 1];
    vht_hdl.channel_center_freq_seg1 = payload[MAC_IE_HDR_LEN + 2];  /* IE头部偏移2字节 */

    /* 0 -- 20/40M, 1 -- 80M, 2 -- 160M, 3--80M+80M */
    if (vht_hdl.channel_width > WLAN_MIB_VHT_OP_WIDTH_80PLUS80) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ie_proc_vht_opern_ie_etc::invalid channel width[%d], use 20M chn width.}", vht_hdl.channel_width);
        vht_hdl.channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
    }

    /* 解析 "VHT Basic MCS Set field" */
    basic_mcs_set = OAL_MAKE_WORD16(payload[MAC_IE_HDR_LEN + 3], payload[MAC_IE_HDR_LEN + 4]);  /* IE头部偏移3/4字节 */
    vht_hdl.basic_mcs_set = basic_mcs_set;

    hmac_user_set_vht_hdl_etc(hmac_user, &vht_hdl);

    if ((channel_width_old != vht_hdl.channel_width) ||
        (freq_seg0_old != vht_hdl.channel_center_freq_seg0)) {
        ret = MAC_VHT_CHANGE;
        oam_warning_log4(0, OAM_SF_ANY,
            "hmac_ie_proc_vht_opern_ie_etc:usr_bw is updated chanl_with from %d to %d,center_freq_seg0 from %d to %d",
            channel_width_old, vht_hdl.channel_width, freq_seg0_old, vht_hdl.channel_center_freq_seg0);
    }

    return ret;
}

/**********************************************************************************
 功能描述 : 从rsnie中获取加密套件时,校验Pairwise Cipher Suite 或者 AKM Suite 的长度
***********************************************************************************/
static osal_u32 hmac_ie_get_wpa_cipher_check_suite(const osal_u8 *src_ie, osal_u8 **ie, osal_u8 ie_len,
    osal_u32 *suite, const osal_u8 suites_num)
{
    osal_u8 suite_idx;
    osal_u16 suites_count;

    /*************************************************************************/
    /*                  WPA Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length |    WPA OUI    |  Version |  Group Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |   1    |        4      |     2    |         4           */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Pairwise Cipher |  Pairwise Cipher   |                 |              */
    /* Suite Count     |    Suite List      | AKM Suite Count |AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*        2        |          4*m       |         2       |     4*n      */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    suites_count = OAL_MAKE_WORD16((*ie)[0], (*ie)[1]);
    if (suites_count > OAL_NL80211_MAX_NR_CIPHER_SUITES) {
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
    (*ie) += 2;    /* 如上图,2是Pairwise Cipher Suite Count的长度 */

    for (suite_idx = 0; suite_idx < suites_count; suite_idx++) {
        if (mac_ie_remain_len_is_enough(src_ie, *ie, ie_len, 4) == OSAL_TRUE) { /* remain len is 4 */
            if (suite_idx < suites_num) {
                *(suite + suite_idx) = *(osal_u32 *)(*ie);
            }
            (*ie) += 4;    /* 如上图,4是每个Pairwise Cipher Suite 或者 AKM Suite 的长度 */
        } else {
            oam_warning_log2(0, OAM_SF_ANY,
                "hmac_ie_get_wpa_cipher_check_suite: invalid ie len[%d], suites_count[%d]",
                ie_len, suites_count);
            return OAL_ERR_CODE_MSG_LENGTH_ERR;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述 : 从rsnie中获取加密套件
*****************************************************************************/
osal_u32 hmac_ie_get_wpa_cipher(osal_u8 *wpa_ie, mac_crypto_settings_stru *crypto)
{
    osal_u8 ie_len;
    osal_u8 *src_ie = OSAL_NULL;
    osal_u32 ret;
    osal_u8 *ie = wpa_ie;

    if (ie == OSAL_NULL || crypto == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(crypto, sizeof(mac_crypto_settings_stru), 0, sizeof(mac_crypto_settings_stru));
    /*************************************************************************/
    /*                  WPA Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length |    WPA OUI    |  Version |  Group Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |   1    |        4      |     2    |         4           */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Pairwise Cipher |  Pairwise Cipher   |                 |              */
    /* Suite Count     |    Suite List      | AKM Suite Count |AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*        2        |          4*m       |         2       |     4*n      */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    ie_len = ie[1];
    if (ie_len < MAC_MIN_WPA_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_ie_get_wpa_cipher:invalid wpa ie len[%d]", ie_len);
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }

    src_ie = ie + 1 + 1;    /* src_ie 指向 ie 的payload部分 */
    ie += 1 + 1 + 4 + 2;    /* 如上图,1是Element ID长度,1是len长度,4是WPA OUI长度,2是Version长度 */
    crypto->wpa_versions = WITP_WPA_VERSION_1;

    /* Group Cipher Suite */
    crypto->group_suite = *(osal_u32 *)ie;
    ie += 4;    /* 如上图,4是Group Cipher Suite的长度 */

    /* Pairwise Cipher Suite 最多存2个 */
    ret = hmac_ie_get_wpa_cipher_check_suite(src_ie, &ie, ie_len, crypto->pair_suite, WLAN_PAIRWISE_CIPHER_SUITES);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_ie_get_wpa_cipher::check Pairwise Cipher Suite ret [%d]", ret);
        return ret;
    }

    if (mac_ie_remain_len_is_enough(src_ie, ie, ie_len, 2) == OSAL_FALSE) { /* remain len is 2 */
        oam_warning_log1(0, OAM_SF_ANY,
            "hmac_ie_get_wpa_cipher:no enough mem for suites_count, wpa ie len[%d]", ie_len);
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }

    /* AKM Suite 最多存2个 */
    ret = hmac_ie_get_wpa_cipher_check_suite(src_ie, &ie, ie_len, crypto->akm_suite, WLAN_AUTHENTICATION_SUITES);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_ie_get_wpa_cipher::check AKM Suite ret [%d]", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : mac_ie_get_wlan_suite
 功能描述  : Cipher Suite获取

*****************************************************************************/
static osal_void mac_ie_get_wlan_suite(osal_u8 **ie, osal_u32 *suite, osal_u32 size, osal_u8 *src_ie, osal_u8 ie_len)
{
    osal_u16 suite_idx;
    osal_u16 suites_count;

    if (mac_ie_remain_len_is_enough(src_ie, *ie, ie_len, 2) == OSAL_FALSE) { /* 2是存在Pairwise Suite或AKM Suite Count字段 */
        return;
    }

    suites_count = OAL_MAKE_WORD16((*ie)[0], (*ie)[1]);
    *ie += 2; /* 2是Pairwise Suite或者AKM Suite Count的长度 */
    for (suite_idx = 0; suite_idx < suites_count; suite_idx++) {
        if (suite_idx < size && (mac_ie_remain_len_is_enough(src_ie, *ie, ie_len, 4) == OSAL_TRUE)) { /* 4 */
            suite[suite_idx] = *(osal_u32 *)(*ie);
        }
        *ie += 4; /* 4 */
    }

    return;
}

static osal_u32 hmac_ie_get_rsn_cipher_check(osal_u8 *ie, const mac_crypto_settings_stru *crypto)
{
    osal_u8 *src_ie = OAL_PTR_NULL;
    osal_u8 ie_len;

    if (ie == OAL_PTR_NULL || crypto == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ie_len = ie[1];
    src_ie = ie + 2;    /* IE头部偏移2字节 */

    if (hmac_ie_check_rsn_cipher_format(src_ie, ie_len) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_ie_check_rsn_cipher_format, return fail.}");
        return OAL_ERR_CODE_MSG_LENGTH_ERR;
    }
    return OAL_SUCC;
}

static osal_u32 mac_rsn_chiper_oui(osal_u32 _suit_type)
{
    return cipher_suite_selector(0x00, 0x0f, 0xac, _suit_type);
}

/*****************************************************************************
 功能描述 : 从rsnie中获取加密套件
*****************************************************************************/
osal_u32 hmac_ie_get_rsn_cipher(osal_u8 *rsn_ie, mac_crypto_settings_stru *crypto)
{
    osal_u16 suites_count, suite_idx;
    osal_u8 ie_len, *src_ie;
    osal_u32 ret;
    osal_u8 *ie = rsn_ie;

    if (ie == OSAL_NULL || crypto == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(crypto, sizeof(mac_crypto_settings_stru), 0, sizeof(mac_crypto_settings_stru));

    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Suite */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /*  Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       |         4*m                |     2           |   4*n          */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    ret = hmac_ie_get_rsn_cipher_check(ie, crypto);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ie_len = ie[1];
    src_ie = ie + 2;    /* IE头部偏移2字节 */

    if (ie_len < MAC_MIN_RSN_LEN) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_ie_get_rsn_cipher:invalid rsn ie len[%d]", ie_len);
        return OAL_FAIL;
    }

    crypto->wpa_versions = oal_make_word16(src_ie[0], src_ie[1]);
    if (crypto->wpa_versions != WITP_WPA_VERSION_1) {
        return OAL_FAIL;
    }
    ie += 1 + 1 + 2;    /* 2是version字段长度 */
    crypto->wpa_versions = WITP_WPA_VERSION_2;

    /* Group Cipher Suite */
    crypto->group_suite = *(osal_u32 *)ie;
    ie += 4;            /* 4是Group Cipher Suite字段长度 */

    /* Pairwise Cipher Suite 最多存2个 */
    mac_ie_get_wlan_suite(&ie, crypto->pair_suite, WLAN_PAIRWISE_CIPHER_SUITES, src_ie, ie_len);

    /* AKM Suite 最多存2个 */
    mac_ie_get_wlan_suite(&ie, crypto->akm_suite, WLAN_AUTHENTICATION_SUITES, src_ie, ie_len);

    /* 越过RSN Capabilities */
    ie += 2; /* 2 */

    /* 目前PMK信息暂不做处理 */
    if (mac_ie_remain_len_is_enough(src_ie, ie, ie_len, 2) == OSAL_FALSE) {  /* remain len is 2 */
        return OAL_SUCC;
    }

    suites_count = OAL_MAKE_WORD16(ie[0], ie[1]);
    ie += 2;            /* 2是PMKID Count的长度 */
    for (suite_idx = 0; suite_idx < suites_count; suite_idx++) {
        ie += 4;        /* 4是每个PMKID的长度 */
    }

    /* 获取Group Management Cipher Suite信息 */
    if (mac_ie_remain_len_is_enough(src_ie, ie, ie_len, 4) != OSAL_FALSE) {  /* remain len is 4 */
        if (mac_rsn_chiper_oui(0) == ((*(osal_u32 *)ie) & 0xFFFFFF)) {
            crypto->group_mgmt_suite = *(osal_u32 *)ie;
            ie += 4;        /* 4是Group Management Cipher Suite的长度 */
        }
    }

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
