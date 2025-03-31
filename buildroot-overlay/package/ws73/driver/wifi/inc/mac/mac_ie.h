/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file of mac_ie.c.
 * Create: 2020-01-01
 */

#ifndef __MAC_IE_H__
#define __MAC_IE_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "wlan_mib_hcm.h"
#include "wlan_types_common.h"
#include "mac_user_ext.h"
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_IE_H

/*****************************************************************************
    宏定义
*****************************************************************************/
#define mac_ie_remain_len_is_enough(src_ie, curr_ie, ie_len, remain_len) \
        (((((curr_ie) - (src_ie)) + (remain_len)) <= (ie_len)) ? OSAL_TRUE : OSAL_FALSE)

/*****************************************************************************
    OTHERS定义
*****************************************************************************/

/*****************************************************************************
 函 数 名  : mac_get_bandwidth_from_sco
 功能描述  : 根据"次信道偏移量"获取对应的带宽模式
*****************************************************************************/
static INLINE__ wlan_channel_bandwidth_enum_uint8 mac_get_bandwidth_from_sco(mac_sec_ch_off_enum_uint8 sec_chan_offset)
{
    switch (sec_chan_offset) {
        case MAC_SCA:   /* Secondary Channel Above */
            return WLAN_BAND_WIDTH_40PLUS;

        case MAC_SCB:   /* Secondary Channel Below */
            return WLAN_BAND_WIDTH_40MINUS;

        default:        /* No Secondary Channel    */
            return WLAN_BAND_WIDTH_20M;
    }
}

/*****************************************************************************
 函 数 名  : mac_get_sco_from_bandwidth
 功能描述  : 根据"带宽模式"获取对应的"次信道偏移量"
*****************************************************************************/
static INLINE__ mac_sec_ch_off_enum_uint8 mac_get_sco_from_bandwidth(wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    switch (bandwidth) {
    case WLAN_BAND_WIDTH_40PLUS:
    case WLAN_BAND_WIDTH_80PLUSPLUS:
    case WLAN_BAND_WIDTH_80PLUSMINUS:
        return MAC_SCA;
    case WLAN_BAND_WIDTH_40MINUS:
    case WLAN_BAND_WIDTH_80MINUSPLUS:
    case WLAN_BAND_WIDTH_80MINUSMINUS:
        return MAC_SCB;
    default:
        return MAC_SCN;
    }
}

/*****************************************************************************
 函 数 名  : mac_get_bandwith_from_center_freq_seg0
 功能描述  : 根据信道中心频点获取对应的"带宽模式"
*****************************************************************************/
static INLINE__ wlan_channel_bandwidth_enum_uint8  mac_get_bandwith_from_center_freq_seg0(osal_u8 chan_width,
    osal_u8 channel, osal_u8 chan_center_freq)
{
    /* 80+80不支持，暂按80M处理 */
    if ((WLAN_MIB_VHT_OP_WIDTH_80 & chan_width) != 0) {
        switch (chan_center_freq - channel) {
            case 6:
                /***********************************************************************
                | 主20 | 从20 | 从40       |
                              |
                              |中心频率相对于主20偏6个信道
                ************************************************************************/
                return WLAN_BAND_WIDTH_80PLUSPLUS;

            case -2:
                /***********************************************************************
                | 从40        | 主20 | 从20 |
                              |
                              |中心频率相对于主20偏-2个信道
                ************************************************************************/
                return WLAN_BAND_WIDTH_80PLUSMINUS;

            case 2:
                /***********************************************************************
                | 从20 | 主20 | 从40       |
                              |
                              |中心频率相对于主20偏2个信道
                ************************************************************************/
                return  WLAN_BAND_WIDTH_80MINUSPLUS;

            case -6:
                /***********************************************************************
                | 从40        | 从20 | 主20 |
                              |
                              |中心频率相对于主20偏-6个信道
                ************************************************************************/
                return WLAN_BAND_WIDTH_80MINUSMINUS;

            default:
                return WLAN_BAND_WIDTH_20M;
        }
    } else {
        return WLAN_BAND_WIDTH_20M;
    }
}

static INLINE__ wlan_channel_bandwidth_enum_uint8  mac_get_bandwith_from_center_freq_seg0_seg1(osal_u8 chan_width,
    osal_u8 channel, osal_u8 chan_center_freq0, osal_u8 chan_center_freq1)
{
    unref_param(chan_center_freq1);
    /* 80+80不支持，暂按80M处理 */
    if ((WLAN_MIB_VHT_OP_WIDTH_80 & chan_width) != 0) {
        switch (chan_center_freq0 - channel) {
            case 2:
                /***********************************************************************
                | 从20 | 主20 | 从40       |
                              |
                              |中心频率相对于主20偏2个信道
                ************************************************************************/
                return  WLAN_BAND_WIDTH_80MINUSPLUS;

            case 6:
                /***********************************************************************
                | 主20 | 从20 | 从40       |
                              |
                              |中心频率相对于主20偏6个信道
                ************************************************************************/
                return WLAN_BAND_WIDTH_80PLUSPLUS;

            case -6:
                /***********************************************************************
                | 从40        | 从20 | 主20 |
                              |
                              |中心频率相对于主20偏-6个信道
                ************************************************************************/
                return WLAN_BAND_WIDTH_80MINUSMINUS;

            case -2:
                /***********************************************************************
                | 从40        | 主20 | 从20 |
                              |
                              |中心频率相对于主20偏-2个信道
                ************************************************************************/
                return WLAN_BAND_WIDTH_80PLUSMINUS;

            default:
                return WLAN_BAND_WIDTH_20M;
        }
    } else {
        return WLAN_BAND_WIDTH_20M;
    }
}

/*****************************************************************************
    函数声明
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
osal_u32 hmac_ie_proc_chwidth_field_etc(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, osal_u8 chwidth);
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
extern osal_u32 hmac_smps_update_user_status(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
#endif
extern  wlan_mib_mimo_power_save_enum_uint8 hmac_ie_proc_sm_power_save_field_etc(hmac_user_stru *hmac_user,
    osal_u8 smps);

extern  oal_bool_enum_uint8 mac_ie_proc_ht_green_field_etc(const hmac_user_stru *hmac_user,
    hmac_vap_stru *hmac_vap, osal_u8 ht_green_field, oal_bool_enum prev_asoc_ht);

extern oal_bool_enum_uint8 mac_ie_proc_ht_supported_channel_width_etc(const hmac_user_stru *hmac_user,
    hmac_vap_stru *hmac_vap, osal_u8 supp_chan_width, oal_bool_enum prev_asoc_ht);

extern oal_bool_enum_uint8 hmac_ie_proc_lsig_txop_protection_support_etc(const hmac_user_stru *hmac_user,
    hmac_vap_stru *hmac_vap, osal_u8 lsig_txop_prot_supp, oal_bool_enum prev_asoc_ht);
extern osal_u32  hmac_ie_proc_ext_cap_ie_etc(hmac_user_stru *hmac_user, osal_u8 *payload);
extern osal_u8 hmac_ie_get_chan_num_etc(osal_u8   *frame_body, osal_u16   frame_len, osal_u16   offset,
    osal_u8    curr_chan);

extern oal_bool_enum_uint8 hmac_ie_check_p2p_action_etc(osal_u8 *payload);
extern osal_u32 mac_ie_proc_ht_sta_etc(const hmac_vap_stru *hmac_vap, osal_u8            *payload,
    osal_u16 offset_len, hmac_user_stru *mac_user_ap, osal_u16 *amsdu_maxsize);
extern osal_u8  *hmac_ie_find_vendor_vht_ie(osal_u8 *frame, osal_u16 frame_len);

#ifdef _PRE_WLAN_FEATURE_11AX
extern osal_u32  hmac_ie_proc_he_opern_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload, hmac_user_stru *hmac_user);
extern osal_u32  hmac_ie_parse_he_cap(osal_u8 *he_cap_ie, mac_frame_he_cap_ie_stru *he_cap_value);
extern osal_u32  hmac_ie_parse_he_oper(osal_u8 *he_oper_ie, mac_frame_he_oper_ie_stru *he_oper_ie_data);
extern osal_u32 hmac_ie_parse_mu_edca_parameter(osal_u8 *edca_ie,
    mac_frame_he_mu_edca_parameter_ie_stru *he_mu_edca_value);
extern osal_u32 hmac_ie_parse_he_bss_color_change_announcement_ie(osal_u8 *payload,
    mac_frame_bss_color_change_annoncement_ie_stru *bss_color);
#endif

extern osal_u32 hmac_proc_ht_opern_ie_etc(hmac_vap_stru *hmac_vap, osal_u8 *payload, hmac_user_stru *hmac_user);
extern osal_u32 hmac_ie_proc_vht_opern_ie_etc(const hmac_vap_stru *hmac_vap, osal_u8 *payload,
    hmac_user_stru *hmac_user);
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
extern osal_u32  hmac_ie_proc_opmode_field_etc(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_opmode_notify_stru *opmode_notify);
extern osal_u32  hmac_check_is_assoc_frame_etc(osal_u8 mgmt_frm_type);
#endif
osal_u32 hmac_ie_get_wpa_cipher(osal_u8 *wpa_ie, mac_crypto_settings_stru *crypto);
osal_u32 hmac_ie_get_rsn_cipher(osal_u8 *rsn_ie, mac_crypto_settings_stru *crypto);

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

#endif /* end of mac_ie.h */
