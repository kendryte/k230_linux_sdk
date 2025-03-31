/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Source file of the structure definition of the corresponding frame.
 * Create: 2020-01-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "mac_ie.h"
#include "dmac_ext_if_hcm.h"
#include "wlan_types_common.h"
#include "common_dft.h"
#ifdef _PRE_WLAN_FEATURE_11AX
#include "mac_resource_ext.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_wur_ap.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif
#include "hmac_dfx.h"
#include "oal_netbuf_data.h"
#include "hmac_feature_interface.h"
#include "hmac_11v.h"
#include "hmac_uapsd_sta.h"
#include "hmac_twt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_MAC_FRAME_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define MAC_DATA_HEADER_LEN 4

/*****************************************************************************
    函数声明
*****************************************************************************/
static osal_void hmac_set_ext_capabilities_ie_update_etc(hmac_vap_stru *hmac_vap,
    mac_ext_cap_ie_stru *ext_cap_ie, osal_u8 *ie_len);
static osal_void hmac_set_ht_cap_ie_rom_cb(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len);
static osal_void hmac_set_ht_opern_ie_rom_cb(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);
static osal_void hmac_set_vht_capinfo_field_cb(osal_void *vap, osal_u8 *buffer);
static osal_void hmac_set_vht_opern_ie_rom_cb(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len);

/*****************************************************************************
    全局变量定义
*****************************************************************************/
mac_frame_cb g_mac_frame_rom_cb = { OSAL_NULL,
                                    hmac_set_ht_cap_ie_rom_cb,       // set_ht_cap_ie
                                    hmac_set_ht_opern_ie_rom_cb,     // set_ht_opern_ie
                                    OSAL_NULL,                        // set_rsn_ie
                                    OSAL_NULL,                        // set_vht_cap_ie_cb
                                    hmac_set_vht_opern_ie_rom_cb,    // set_vht_opern_ie_cb
                                    OSAL_NULL,                        // set_wpa_ie_cb
                                    OSAL_NULL,                        // set_nb_ie_cb
                                    hmac_set_vht_capinfo_field_cb }; // set_vht_capinfo

/* WMM OUI定义 */
const osal_u8 g_wmm_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* WPA OUI 定义 */
const osal_u8 g_wpa_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* WFA TPC RPT OUI 定义 */
const osal_u8 g_wfa_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* P2P OUI 定义 */
const osal_u8 g_p2p_oui_etc[MAC_OUI_LEN] = {0x50, 0x6F, 0x9A};

/* RSNA OUI 定义 */
const osal_u8 g_rsn_oui_etc[MAC_OUI_LEN] = {0x00, 0x0F, 0xAC};

/* WPS OUI 定义 */
const osal_u8 g_wps_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* 窄带 OUI 定义 */
const osal_u8 g_mac_vendor_oui[MAC_OUI_LEN] = {0xac, 0x85, 0x3d};

/*****************************************************************************
    函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : mac_frame_get_p2p_oui
 功能描述  : 外部获取g_p2p_oui_etc地址
*****************************************************************************/
osal_u8 const *mac_frame_get_p2p_oui(osal_void)
{
    return g_p2p_oui_etc;
}

/*****************************************************************************
 函 数 名  : hmac_find_ie_ext_ie
 功能描述  : 查找指定的IE
*****************************************************************************/
osal_u8 *hmac_find_ie_ext_ie(osal_u8 eid, osal_u8 ext_eid, osal_u8 *ie_addr, osal_s32 frame_len)
{
    osal_s32 len = frame_len;
    osal_u8 *ies = ie_addr;

    if (ies == OSAL_NULL) {
        return OSAL_NULL;
    }

    /* 0: eid, 1: eid_len, 2: ext_eid */
    while ((len > MAC_IE_EXT_HDR_LEN) && ((ies[0] != eid) || (ies[2] != ext_eid))) {
        len -= ies[1] + MAC_IE_HDR_LEN;
        ies += ies[1] + MAC_IE_HDR_LEN;
    }

    /* 0: eid, 1: eid_len, 2: ext_eid */
    if ((len > MAC_IE_EXT_HDR_LEN) && (eid == ies[0]) && (ext_eid == ies[2]) &&
        (len >= (MAC_IE_HDR_LEN + ies[1]))) {
        return ies;
    }

    return OSAL_NULL;
}

#ifdef _PRE_WLAN_FEATURE_11AX
/*****************************************************************************
 函 数 名  : mac_set_he_mac_capinfo_field
 功能描述  : 填写he mac capabilities info域
*****************************************************************************/
static osal_void mac_set_he_mac_capinfo_field(hmac_vap_stru *hmac_vap, const osal_u8 *buffer)
{
    mac_frame_he_mac_cap_stru *he_mac_capinfo = (mac_frame_he_mac_cap_stru *)buffer;
    (osal_void)memset_s(he_mac_capinfo, sizeof(mac_frame_he_mac_cap_stru), 0, sizeof(mac_frame_he_mac_cap_stru));

    /*********************** HE MAC 能力信息域 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------------------------|
     | +HTC    | TWT         | TWT         | Fragmentation | Max Num      | Min                |
     | HE        | Requester | Responder   | support       | Fragmented   | Fragment           |
     | Support | Support     |   Support   |               | MSDUs        |     Size           |
     |-----------------------------------------------------------------------------------------|
     | B0        | B1        | B2          |   B3  B4      |  B5 B6 B7    |  B8 B9             |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Trigger Frame | Muti-TID     | HE           | ALL        | UMRS        | BSR            |
     | MAC Padding   | Aggregation  | Link         | ACK        | Support     | Support        |
     | Duration      | Support      | Adaptation   | Support    |             |                |
     |-----------------------------------------------------------------------------------------|
     |    B10   B11  | B12      B14 | B15    B16   |   B17      |    B18      | B19            |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Broadcast | 32-bit BA | MU       | Ack-Enable             | Group Addressed   Multi-STA |
     | TWT       | bitmap    | Cascade  | Multi-TID              | BlockAck In DL MU           |
     | Support   | Support   | Support  | Aggregation Support    | Support                     |
     |-----------------------------------------------------------------------------------------|
     | B20       | B21       | B22      |   B23                  |   B24                       |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | OM        | OFDMA   | Max A-MPDU  | A-MSDU          | Flexible  TWT| Rx Control         |
     | Control   | RA      | Length      | Fragmentation   |  schedule    | frame to           |
     | Support   | Support | Exponent    |  Support        | Support      | MultiBSS           |
     |---------------------------------------------------------------------------------------- |
     | B25       | B26     | B27  B28    |   B29           |   B30        |  B31               |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | NSRP BQRP   | QTP     | BQR     | SR       | NDP     | OPS      | A-MSDU in |           |
     | A-MPDU      |         |         |          |Feedback |  Support | A-MPDU    |           |
     | Aggregation | Support | Support |Responder | Report  |          | Support   |           |
     |-----------------------------------------------------------------------------------------|
     | B32         | B33     | B34     |   B35    |   B36   |  B37     |   B38     |           |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Multi-TID            |                                                                  |
     | Aggregation  Tx      |         Reserved                                                 |
     | Support              |                                                                  |
     |-----------------------------------------------------------------------------------------|
     | B39  B40   B41       |         B42--B47                                                 |
     |-----------------------------------------------------------------------------------------|
    ***************************************************************************/
    /* B0:+HTC support */
    he_mac_capinfo->htc_he_support = mac_mib_get_he_ht_control_field_supported(hmac_vap);

    /* B25:OM control support */
    he_mac_capinfo->om_control_support = mac_mib_get_he_om_control_support(hmac_vap);

    /* B12-B14:Multi-TID Aggregation Rx support */
    he_mac_capinfo->mtid_aggregation_rx_support = mac_mib_get_he_mtid_aggr_rx_support(hmac_vap);

    /* B21:32-bit BA Bitmap Support */
    he_mac_capinfo->ba_bitmap_support_32bit     = mac_mib_get_ba_32bit_bitmap_implemented(hmac_vap);

    /* B27-28:Maximum AMPDU Length Exponent */
    he_mac_capinfo->max_ampdu_length_exponent   = mac_mib_get_he_max_ampdu_length(hmac_vap);
    return;
}

static osal_void mac_set_sta_he_mac_capinfo_field(hmac_vap_stru *hmac_vap, const osal_u8 *buffer)
{
    mac_frame_he_mac_cap_stru *he_mac_capinfo = (mac_frame_he_mac_cap_stru *)buffer;
    osal_void *fhook = OSAL_NULL;

    (osal_void)memset_s(he_mac_capinfo, sizeof(mac_frame_he_mac_cap_stru), 0, sizeof(mac_frame_he_mac_cap_stru));

    /*********************** HE MAC 能力信息域 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------------------------|
     | +HTC    | TWT         | TWT         | Fragmentation | Max Num      | Min                |
     | HE        | Requester | Responder   | support       | Fragmented   | Fragment           |
     | Support | Support     |   Support   |               | MSDUs        |     Size           |
     |-----------------------------------------------------------------------------------------|
     | B0        | B1        | B2          |   B3  B4      |  B5 B6 B7    |  B8 B9             |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Trigger Frame | Muti-TID     | HE           | ALL        | UMRS        | BSR            |
     | MAC Padding   | Aggregation  | Link         | ACK        | Support     | Support        |
     | Duration      | Support      | Adaptation   | Support    |             |                |
     |-----------------------------------------------------------------------------------------|
     |    B10   B11  | B12      B14 | B15    B16   |   B17      |    B18      | B19            |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Broadcast | 32-bit BA | MU       | Ack-Enable             | Group Addressed   Multi-STA |
     | TWT       | bitmap    | Cascade  | Multi-TID              | BlockAck In DL MU           |
     | Support   | Support   | Support  | Aggregation Support    | Support                     |
     |-----------------------------------------------------------------------------------------|
     | B20       | B21       | B22      |   B23                  |   B24                       |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | OM        | OFDMA   | Max A-MPDU  | A-MSDU          | Flexible  TWT| Rx Control         |
     | Control   | RA      | Length      | Fragmentation   |  schedule    | frame to           |
     | Support   | Support | Exponent    |  Support        | Support      | MultiBSS           |
     |---------------------------------------------------------------------------------------- |
     | B25       | B26     | B27  B28    |   B29           |   B30        |  B31               |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | NSRP BQRP   | QTP     | BQR     | SR       | NDP     | OPS      | A-MSDU in |           |
     | A-MPDU      |         |         |          |Feedback |  Support | A-MPDU    |           |
     | Aggregation | Support | Support |Responder | Report  |          | Support   |           |
     |-----------------------------------------------------------------------------------------|
     | B32         | B33     | B34     |   B35    |   B36   |  B37     |   B38     |           |
     |-----------------------------------------------------------------------------------------|
     |-----------------------------------------------------------------------------------------|
     | Multi-TID            |                                                                  |
     | Aggregation  Tx      |         Reserved                                                 |
     | Support              |                                                                  |
     |-----------------------------------------------------------------------------------------|
     | B39  B40   B41       |         B42--B47                                                 |
     |-----------------------------------------------------------------------------------------|
    ***************************************************************************/

    he_mac_capinfo->htc_he_support = mac_mib_get_he_ht_control_field_supported(hmac_vap);

    /* add TWT */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_GET_REQ_BIT);
    if (fhook != OSAL_NULL) {
        he_mac_capinfo->twt_requester_support = ((hmac_twt_get_req_bit_cb)fhook)(hmac_vap->vap_id);
    }

    /* B3-B4:fragmentation support  认证用例:5.30 不支持   */
    he_mac_capinfo->fragmentation_support = 0;

    /* B10-11:Trigger Frame MAC Padding Duration   16us */
    he_mac_capinfo->trigger_mac_padding_duration = mac_mib_get_he_trigger_mac_padding_duration(hmac_vap);

    /* B12-B14:Multi-TID Aggregation Rx support */
    he_mac_capinfo->mtid_aggregation_rx_support = mac_mib_get_he_mtid_aggr_rx_support(hmac_vap);

    /* B17:All Ack Support,MBA */
    he_mac_capinfo->all_ack_support = 0;

    /* B18:A-Control 支持 TSR */
    he_mac_capinfo->trs_support = mac_mib_get_trs_option_implemented(hmac_vap);

    /* B19:BSR Support */
    he_mac_capinfo->bsr_support = mac_mib_get_he_bsr_control_implemented(hmac_vap);

    /* B21:32-bit BA Bitmap Support */
    he_mac_capinfo->ba_bitmap_support_32bit = mac_mib_get_ba_32bit_bitmap_implemented(hmac_vap);

    /* B23:Ack Enabled Aggregation Support */
    he_mac_capinfo->ack_enabled_aggregation_support = 0;

    /* B25:A-Control 支持OMI */
    he_mac_capinfo->om_control_support = mac_mib_get_he_om_control_support(hmac_vap);

    /* B26:OFDMA RA Support */
    he_mac_capinfo->ofdma_ra_support = 0;

    /* B27-28:Maximum AMPDU Length Exponent */
    he_mac_capinfo->max_ampdu_length_exponent = mac_mib_get_he_max_ampdu_length(hmac_vap);

    /* B34:BQR Support */
    he_mac_capinfo->bqr_support = mac_mib_get_he_bqr_control_implemented(hmac_vap);

    /* B35:PSR responder Support */
    he_mac_capinfo->sr_responder = mac_mib_get_he_psr_option_implemented(hmac_vap);

    /* B36:NDP Feedback Report Support */
    he_mac_capinfo->ndp_feedback_report_support = mac_mib_get_support_response_nfrp_trigger(hmac_vap);

    /* B38:Indicates support by a STA to receive an ack-enabled single-TID A-MPDU that carries an A-MSDU
       that is not under a block ack agreement. */
    he_mac_capinfo->amsdu_ampdu_support = 0;

    /* B47:HT And VHT Trigger Frame Rx Support */
    he_mac_capinfo->ht_vht_tf_rx_support = mac_mib_get_he_ht_vht_trigger_option_implemented(hmac_vap);

    return;
}

/*****************************************************************************
 函 数 名  : mac_set_he_phy_capinfo_field
 功能描述  : 填写he phy capabilities info域
*****************************************************************************/
static osal_void mac_set_he_phy_capinfo_field(const hmac_vap_stru *hmac_vap, const osal_u8 *buffer)
{
    mac_frame_he_phy_cap_stru *he_phy_capinfo = (mac_frame_he_phy_cap_stru *)buffer;
    (osal_void)memset_s(he_phy_capinfo, sizeof(mac_frame_he_phy_cap_stru), 0, sizeof(mac_frame_he_phy_cap_stru));

    /* B1-B7:channel width set */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
#ifdef _PRE_WLAN_11AX_20M_ONLY
        he_phy_capinfo->channel_width_set       = 0;
#else
        he_phy_capinfo->channel_width_set       = 0x1;
#endif
    } else {
        he_phy_capinfo->channel_width_set       = 0x2;
    }

#ifdef _PRE_WLAN_FEATURE_LDPC_TX_ONLY
    he_phy_capinfo->ldpc_coding_in_paylod       = 0;
#else
    /* B13:LDPC Coding in payload,认证用例5.24、5.25要求支持 */
    he_phy_capinfo->ldpc_coding_in_paylod       = mac_mib_get_he_ldpc_coding_in_payload(hmac_vap);
#endif

    /* B17:NDP With 4x HELTF And 3.2 us GI */
    he_phy_capinfo->ndp_4xltf_32us               = 1;

    /* B19:stbc_rx_below_80mhz, 从ini中读取 */
    he_phy_capinfo->stbc_rx_below_80mhz         = mac_mib_get_he_rx_stbc_beamformer(hmac_vap);

    /* B24-25: DCM Max Constellation Tx */
    he_phy_capinfo->dcm_max_constellation_tx     = hmac_vap->dcm_constellation_tx;
    /* 11ax 接口定制AR 要求去掉p2p gc和dev的trigger相关能力, 只有station具备该能力 */
    if ((is_p2p_cl(hmac_vap)) || (is_p2p_dev(hmac_vap))) {
        he_phy_capinfo->dcm_max_constellation_tx = 0;
    }

    /* B27-28:DCM Max Constellation Rx */
    he_phy_capinfo->dcm_max_constellation_rx    = 0x0;

    /* B29:DCM Max NSS Rx */
    he_phy_capinfo->dcm_max_nss_rx              = hmac_vap->vap_rx_nss;

    /* B32:SU Beamformee */
    he_phy_capinfo->su_beamformee                = mac_mib_get_he_su_beamformee(hmac_vap);

    /* B34-B36:Beamformee STS ≤ 80MHz */
    he_phy_capinfo->beamformee_sts_below_80mhz = hmac_vap->beamformee_sts_below_80mhz;

    /* B46:Ng = 16 SU Feedback */
    he_phy_capinfo->ng16_su_feedback             = hmac_vap->ng16_su_feedback;

    he_phy_capinfo->codebook_42_su_feedback      = 1;

    /* B53:Partial Bandwidth Extended Range */
    he_phy_capinfo->partial_bandwidth_extended_range = 1;

    /* B55:PPE Threshold Present */
    he_phy_capinfo->ppe_threshold_present       = mac_mib_get_ppe_thresholds_required(hmac_vap);

    /* B58:HE SU PPDU And HE MU PPDU With 4x HE-LTF And 0.8 us GI */
    he_phy_capinfo->he_su_mu_ppdu_4xltf_08us_gi = 1;

    /* B64:HE ER SU PPDU With 4x HE-LTF And 0.8 us GI */
    he_phy_capinfo->he_er_su_ppdu_4xltf_08us_gi  = 1;

    /* B70-71: DCM Max RU */
    he_phy_capinfo->dcm_max_ru = 0;
    if ((he_phy_capinfo->channel_width_set & 0x1) != 0) { // bit1 indicate support 40MHz
        he_phy_capinfo->dcm_max_ru = 0x1;
    } else if ((he_phy_capinfo->channel_width_set & 0x2) != 0) { // bit2 indicate support 40/80MHz
        he_phy_capinfo->dcm_max_ru = 0x2;
    }

    /* B75:Rx 1024-QAM <242-tone RU Support */
    he_phy_capinfo->rx_1024_qam_below_242_tone_ru_support = hmac_vap->cap_flag.is_1024qam;
}

static osal_void mac_set_sta_he_phy_capinfo_field(const hmac_vap_stru *hmac_vap, const osal_u8 *buffer)
{
    mac_frame_he_phy_cap_stru *he_phy_capinfo = (mac_frame_he_phy_cap_stru *)buffer;
    (osal_void)memset_s(he_phy_capinfo, sizeof(mac_frame_he_phy_cap_stru), 0, sizeof(mac_frame_he_phy_cap_stru));

    /* B1-B7:channel width set */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
#ifdef _PRE_WLAN_11AX_20M_ONLY
        he_phy_capinfo->channel_width_set = 0x10; // 0x10 support 20M only
#else
        he_phy_capinfo->channel_width_set = 0x1;
#endif
    } else {
        he_phy_capinfo->channel_width_set = 0x2;
    }

    /* B13:LDPC Coding in payload,认证用例5.24、5.25要求支持 */
#ifdef _PRE_WLAN_FEATURE_LDPC_TX_ONLY
    he_phy_capinfo->ldpc_coding_in_paylod        = 0;
#else
    he_phy_capinfo->ldpc_coding_in_paylod        = mac_mib_get_he_ldpc_coding_in_payload(hmac_vap);
#endif

    /* B17:NDP With 4x HELTF And 3.2 us GI */
    he_phy_capinfo->ndp_4xltf_32us               = 1;

    /* B19:STBC RX <=80 MHZ,从ini中获取 */
    he_phy_capinfo->stbc_rx_below_80mhz          = mac_mib_get_he_rx_stbc_beamformer(hmac_vap);

    /* B22:Full Bandwidth UL MU-MIMO */
    he_phy_capinfo->full_bandwidth_ul_mu_mimo    = mac_mib_get_he_mu_beamformer(hmac_vap);

    /* B23: Partial Bandwidth UL MU-MIMO */
    he_phy_capinfo->partial_bandwidth_ul_mu_mimo = mac_mib_get_he_mu_beamformer(hmac_vap);

    /* B24-25: DCM Max Constellation Tx */
    he_phy_capinfo->dcm_max_constellation_tx     = hmac_vap->dcm_constellation_tx;
    /* 11ax 接口定制AR 要求去掉p2p gc和dev的trigger相关能力, 只有station具备该能力 */
    if ((is_p2p_cl(hmac_vap)) || (is_p2p_dev(hmac_vap))) {
        he_phy_capinfo->dcm_max_constellation_tx = 0;
    }
    /* B26:DCM Max NSS Tx */
    he_phy_capinfo->dcm_max_nss_tx = hmac_vap->vap_rx_nss;

    /* B27-28:DCM Max Constellation Rx */
    he_phy_capinfo->dcm_max_constellation_rx = 0x0;

    /* B29:DCM Max NSS Rx */
    he_phy_capinfo->dcm_max_nss_rx = hmac_vap->vap_rx_nss;

    /* B31:SU Beamformer */
    he_phy_capinfo->su_beamformer                = mac_mib_get_he_su_beamformer(hmac_vap);

    /* B32:SU Beamformee */
    he_phy_capinfo->su_beamformee                = mac_mib_get_he_su_beamformee(hmac_vap);

    /* B33:MU Beamformer */
    he_phy_capinfo->mu_beamformer                = mac_mib_get_he_mu_beamformer(hmac_vap);

    /* B34-B36:Beamformee STS ≤ 80MHz,本地与ap取小,同11ac */
    he_phy_capinfo->beamformee_sts_below_80mhz = hmac_vap->beamformee_sts_below_80mhz;

    /* B40-B42:Number Of Sounding Dimensions ≤ 80 MHz */
    he_phy_capinfo->below_80mhz_sounding_dimensions_num =
        (osal_u8)mac_mib_get_he_num_sounding_dims_below_80mhz(hmac_vap);

    /* B46:Ng = 16 SU Feedback */
    he_phy_capinfo->ng16_su_feedback             = hmac_vap->ng16_su_feedback;

    /* B47:Ng = 16 MU Feedback */
    he_phy_capinfo->ng16_mu_feedback             = mac_mib_get_he_ng16_mufeedback_support(hmac_vap);

    /* B48:CodeBook Size(Φ,Ψ)={7,5} SU Feedback */
    he_phy_capinfo->codebook_42_su_feedback      = 1;
    he_phy_capinfo->codebook_75_mu_feedback      = mac_mib_get_he_codebooksize_mufeedback_support(hmac_vap);

    /* B53:Partial Bandwidth Extended Range */
    he_phy_capinfo->partial_bandwidth_extended_range = 1;

    /* B54:Partial Bandwidth DL MU-MIMO */
    he_phy_capinfo->partial_bandwidth_dl_mu_mimo = mac_mib_get_he_partial_bw_dlmumimo_implemented(hmac_vap);

    /* B55:PPE Threshold Present */
    he_phy_capinfo->ppe_threshold_present        = mac_mib_get_ppe_thresholds_required(hmac_vap);

    /* B56:srp_based_sr_support */
    he_phy_capinfo->srp_based_sr_support         = mac_mib_get_he_psr_option_implemented(hmac_vap);

    /* B58:HE SU PPDU And HE MU PPDU With 4x HE-LTF And 0.8 us GI */
    he_phy_capinfo->he_su_mu_ppdu_4xltf_08us_gi  = 1;

    /* B59:max_nc nc = nss - 1 ,0 : WLAN_SINGLE_NSS  1: WLAN_DOUBLE_NSS */
    he_phy_capinfo->max_nc                       = hmac_vap->vap_rx_nss;

    /* B64:HE ER SU PPDU With 4x HE-LTF And 0.8 us GI */
    he_phy_capinfo->he_er_su_ppdu_4xltf_08us_gi  = 1;

    /* B65:20/40MHz HE PPDU 2.4G */
    he_phy_capinfo->he_ppdu_2g_20mhz_in_40mhz    = 1;

    /* B70-71: DCM Max RU */
    if ((he_phy_capinfo->channel_width_set & 0x1) != 0) { // bit1 indicate support 40MHz
        he_phy_capinfo->dcm_max_ru = 0x1;
    } else if ((he_phy_capinfo->channel_width_set & 0x2) != 0) { // bit2 indicate support 40/80MHz
        he_phy_capinfo->dcm_max_ru = 0x2;
    }

    /* B74:Tx 1024-QAM <242-tone RU Support */
    he_phy_capinfo->tx_1024_qam_below_242_tone_ru_support = hmac_vap->cap_flag.is_1024qam;

    /* B75:Rx 1024-QAM <242-tone RU Support */
    he_phy_capinfo->rx_1024_qam_below_242_tone_ru_support = hmac_vap->cap_flag.is_1024qam;

    /* B76:Rx Full BW SU Using HE MU PPDU With Compressed HE-SIG-B */
    he_phy_capinfo->rx_full_bw_su_using_he_mu_ppdu_compress_he_sig_b     = 1;

    /* B77:Rx Full BW SU Using HE MU PPDU With Non-Compressed HESIG-B */
    he_phy_capinfo->rx_full_bw_su_using_he_mu_ppdu_non_compress_he_sig_b = 1;
    he_phy_capinfo->nominal_packet_padding = 0x2; // support 16us PPE
}

/*****************************************************************************
 函 数 名  : mac_set_he_tx_rx_mcs_nss_field
 功能描述  : 填写he phy capabilities info域
*****************************************************************************/
static osal_u8 mac_set_he_tx_rx_mcs_nss_field(const hmac_vap_stru *hmac_vap, const osal_u8 *buffer)
{
    osal_u8 len;
    mac_fram_he_mac_nsss_set_stru *he_mcs_nss_info = (mac_fram_he_mac_nsss_set_stru *)buffer;

    (osal_void)memset_s(he_mcs_nss_info, sizeof(mac_fram_he_mac_nsss_set_stru),
        0, sizeof(mac_fram_he_mac_nsss_set_stru));

    /* 03仅支持<=80Mhz */
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;

    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_1ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_2ss = MAC_MAX_SUP_MCS11_11AX_EACH_NSS;
    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;

    if (hmac_vap->vap_rx_nss == WLAN_SINGLE_NSS) {
        he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
        if (mac_mib_get_he_ldpc_coding_in_payload(hmac_vap) == OSAL_FALSE ||
            hmac_vap->cap_flag.is_1024qam == OSAL_FALSE) {
            /* BCC/not 1024 qam only support MCS0~9 */
            he_mcs_nss_info->rx_he_mcs_below_80mhz.max_he_mcs_for_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
            he_mcs_nss_info->tx_he_mcs_below_80mhz.max_he_mcs_for_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        }
    }
    len = sizeof(mac_fram_he_mac_nsss_set_stru);

    /* 160MHz相关暂不适配 */

    return len;
}

/*****************************************************************************
 函 数 名  : mac_fill_he_ppe_ru_para
 功能描述  : 填写ppe_thresholds_field
*****************************************************************************/
static osal_void mac_fill_he_ppe_ru_para(mac_frame_ppe_thresholds_pre_field_stru *ppe_thresholds)
{
    if (ppe_thresholds->ru_index0_mask != 0) {
        ppe_thresholds->ppet16_nss1_ru0 = 0;
        ppe_thresholds->ppet8_nss1_ru0 = 7;     /* 7 for None, See IEEE802.11ax Table 9-322d—Constellation index */
        if (ppe_thresholds->nss == WLAN_DOUBLE_NSS) {
            ppe_thresholds->ppet16_nss2_ru0 = 0;
            ppe_thresholds->ppet8_nss2_ru0 = 7;     /* 7 for None, See IEEE802.11ax Table 9-322d—Constellation index */
        }
    }

    if (ppe_thresholds->ru_index1_mask != 0) {
        ppe_thresholds->ppet16_nss1_ru1 = 0;
        ppe_thresholds->ppet8_nss1_ru1 = 7;     /* 7 for None, See IEEE802.11ax Table 9-322d—Constellation index */
        if (ppe_thresholds->nss == WLAN_DOUBLE_NSS) {
            ppe_thresholds->ppet16_nss2_ru1 = 0;
            ppe_thresholds->ppet8_nss2_ru1 = 7;     /* 7 for None, See IEEE802.11ax Table 9-322d—Constellation index */
        }
    }

    if (ppe_thresholds->ru_index2_mask != 0) {
        ppe_thresholds->ppet16_nss1_ru2 = 0;
        ppe_thresholds->ppet8_nss1_ru2 = 7;     /* 7 for None, See IEEE802.11ax Table 9-322d—Constellation index */
        if (ppe_thresholds->nss == WLAN_DOUBLE_NSS) {
            ppe_thresholds->ppet16_nss2_ru2 = 0;
            ppe_thresholds->ppet8_nss2_ru2 = 7;     /* 7 for None, See IEEE802.11ax Table 9-322d—Constellation index */
        }
    }
}

/*****************************************************************************
 函 数 名  : mac_set_he_ppe_thresholds_field
 功能描述  : 填写ppe_thresholds_field域
*****************************************************************************/
static osal_u8 mac_set_he_ppe_thresholds_field(const hmac_vap_stru *hmac_vap, osal_u8 *buffer)
{
    mac_frame_ppe_thresholds_pre_field_stru ppe_thresholds;
    osal_u8 ppe_byte_len;
    osal_u8 ppe_bit_len;
    osal_u8 ru_num;
    osal_u8 nsts;

    (osal_void)memset_s(&ppe_thresholds, sizeof(mac_frame_ppe_thresholds_pre_field_stru),
        0, sizeof(mac_frame_ppe_thresholds_pre_field_stru));

    ppe_thresholds.nss = hmac_vap->vap_rx_nss; /* 空间流 */
    ppe_thresholds.ru_index0_mask = 1;     /* 支持RU-242 */
    ppe_thresholds.ru_index1_mask = 1;     /* 支持RU-484 */
    ppe_thresholds.ru_index2_mask = 1;     /* 支持RU-996 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
#ifdef _PRE_WLAN_11AX_20M_ONLY
        ppe_thresholds.ru_index1_mask = 0;     /* not support RU-484 */
        ppe_thresholds.ru_index2_mask = 0;     /* not support RU-996 */
#else
        ppe_thresholds.ru_index2_mask = 0;     /* not support RU-996 */
#endif
    }

    mac_fill_he_ppe_ru_para(&ppe_thresholds);
    ru_num = ppe_thresholds.ru_index0_mask + ppe_thresholds.ru_index1_mask +
        ppe_thresholds.ru_index2_mask + ppe_thresholds.ru_index3_mask;
    nsts = ppe_thresholds.nss + 1;

    /***************************************************************************
    ----------------------------------------------------------------------------
    |NTST|RU idex|PE16 S1 RUx|PE8 S1 RUx|...|PE16 S2 RUm|PE8 S2 RUm|...|PPE Pad|
    ----------------------------------------------------------------------------
    |3   |4      |3          |3         |...|3          |3         |...|0~7    |
    ----------------------------------------------------------------------------
    ***************************************************************************/
    ppe_bit_len = 7 + nsts * ru_num * 6; // 7 : ppe head fix bit len; 6 : RU fix bit len
    ppe_byte_len = (ppe_bit_len / 8) + ((ppe_bit_len % 8 == 0) ? 0 : 1); // 8 : 1byte = 8bit

    (osal_void)memcpy_s(buffer, ppe_byte_len, &ppe_thresholds, ppe_byte_len);
    return ppe_byte_len;
}

/*****************************************************************************
 函 数 名  : hmac_set_he_capabilities_ie
 功能描述  : 填充he capabilities信息
 *****************************************************************************/
osal_void hmac_set_he_capabilities_ie(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *ie_length = OSAL_NULL;
    osal_u8 info_length;
    osal_u8 *buffer = buf;
    *ie_len = 0;

    if (mac_mib_get_he_option_implemented(hmac_vap) != OSAL_TRUE) {
        return;
    }

    /* PF认证要求对于WEP、TKIP 加密方式不能关联在HE模式 */
    if ((mac_mib_get_he_option_implemented(hmac_vap) != OSAL_TRUE) || ((mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) ||
        (mac_is_tkip_only(hmac_vap) == OSAL_TRUE))) {
        *ie_len = 0;
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |EID Extension|HE MAC Capa. Info |HE PHY Capa. Info|
    -------------------------------------------------------------------------
    |1   |1          |1                  |5                         |9                       |
    -------------------------------------------------------------------------
    |Tx Rx HE MCS NSS Support |PPE Thresholds(Optional)|
    -------------------------------------------------------------------------
    |2 or More                         |Variable                       |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *buffer = MAC_EID_HE;
    ie_length = buffer + 1;

    buffer += MAC_IE_HDR_LEN;
    *ie_len += MAC_IE_HDR_LEN;

    *buffer = MAC_EID_EXT_HE_CAP;
    buffer += 1;

    *ie_len += 1;

    /* 填充HE mac capabilities information域信息 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_set_sta_he_mac_capinfo_field(hmac_vap, buffer);
    } else {
        mac_set_he_mac_capinfo_field(hmac_vap, buffer);
    }
    buffer    += MAC_HE_MAC_CAP_LEN;
    *ie_len   += MAC_HE_MAC_CAP_LEN;

    /* 填充HE PHY Capabilities Information 域信息 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        mac_set_sta_he_phy_capinfo_field(hmac_vap, buffer);
    } else {
        mac_set_he_phy_capinfo_field(hmac_vap, buffer);
    }
    buffer    += MAC_HE_PHY_CAP_LEN;
    *ie_len   += MAC_HE_PHY_CAP_LEN;

    /* 填充 HE tx rx he mcs nss support */
    info_length = mac_set_he_tx_rx_mcs_nss_field(hmac_vap, buffer);
    buffer    += info_length;
    *ie_len   += info_length;

    /* 填充 PPE Thresholds field */
    if (mac_mib_get_ppe_thresholds_required(hmac_vap)) {
        info_length = mac_set_he_ppe_thresholds_field(hmac_vap, buffer);
        buffer += info_length;
        *ie_len += info_length;
    }
    *ie_length = *ie_len - MAC_IE_HDR_LEN;
}

static osal_void mac_fill_he_opern_param(mac_frame_he_operation_param_stru *he_opern_param)
{
    /*******************HE Operation Parameters********************************
    --------------------------------------------------------------------
    |Default PE |TWT      |TXOP Duration |VHT Operation |Co-Hosted |
    |Duration   |Required |RTS Threshold |Info. Present |BSS       |
    --------------------------------------------------------------------
    |3          |1        |10            |1             |1         |
    --------------------------------------------------------------------
    |ER SU   |6 GHz Operation |Reserved |
    |Disable |Info. Present   |         |
    -------------------------------------
    |1       |1               |6        |
    -------------------------------------
    ***************************************************************************/
    /* B0-B2:Default PE Duration, 不支持，5-7reserved */
    he_opern_param->default_pe_duration = 4; // 4: 16us

    /* B3:TWT Required */
    he_opern_param->twt_required = 0;

    /* B4-B13:TXOP Duration RTS Threshold, 1023表示该字段不可用 */
    he_opern_param->txop_duration_rts_threshold = 1023;

    /* B14:VHT Operation Info. Present */
    /* 由于beacon和probe rsp中一定携带VHT信息，故此位置0 */
    he_opern_param->vht_operation_info_present = 0;

    /* B15:Co-Hosted BSS */
    he_opern_param->co_located_bss = 0;

    /* B16:ER SU Disable的取值，0表示支持接收242-tone HE ER SU PPDU */
    he_opern_param->er_su_disable = 0;

    /* B17:6 GHz Operation Info. Present的取值, 不支持6GHz band */
    /* 此位置0,表示HE Operation中6GHz Operation Info字段不存在 */
    he_opern_param->er_6g_oper_info_present = 0;

    /* B18-B23保留 */
    he_opern_param->reserved = 0;
}
osal_void hmac_set_he_operation_ie(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *ie_length = OSAL_NULL;
    mac_frame_he_operation_param_stru *he_opern_param = OSAL_NULL;
    mac_frame_he_bss_color_info_stru *he_bss_color = OSAL_NULL;
    mac_frame_he_mcs_nss_bit_map_stru *mcs_bit_map = OSAL_NULL;
    osal_u8 *buffer = buf;
    *ie_len = 0;

    /* PF认证要求对于WEP、TKIP 加密方式不能关联在HE模式 */
    if ((mac_mib_get_he_option_implemented(hmac_vap) != OSAL_TRUE) ||
        ((mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) || (mac_is_tkip_only(hmac_vap) == OSAL_TRUE))) {
        *ie_len = 0;
        return;
    }

    /*******************HE Operation element**********************************
    --------------------------------------------------------------------
    |EID |Length |EID Extension|HE Operation Parameters |BSS Color Info|
    --------------------------------------------------------------------
    |1   |1      |1            |3                       |1             |
    --------------------------------------------------------------------
    |Basic HE-MCS |VHT Operation |Max Co-Hosted    |6 GHz Operation    |
    |And NSS Set  |Info          |BSSID Indicator  |Info               |
    --------------------------------------------------------------------
    |2            |0 or 3        | 0 or 1          |0 or 4             |
    --------------------------------------------------------------------
    ***************************************************************************/
    *buffer = MAC_EID_HE;
    ie_length = buffer + 1;

    buffer += MAC_IE_HDR_LEN;
    *ie_len += MAC_IE_HDR_LEN;

    *buffer = MAC_EID_EXT_HE_OPERATION;
    buffer += 1;
    *ie_len += 1;

    he_opern_param = (mac_frame_he_operation_param_stru *)buffer;
    mac_fill_he_opern_param(he_opern_param);

    buffer += MAC_HE_OPE_PARAM_LEN;
    *ie_len += MAC_HE_OPE_PARAM_LEN;

    he_bss_color = (mac_frame_he_bss_color_info_stru *)buffer;
    (osal_void)memset_s(he_bss_color, sizeof(mac_frame_he_bss_color_info_stru),
        0, sizeof(mac_frame_he_bss_color_info_stru));
    /*******************BSS Color Info*********************
    -------------------------------------------------------
    |BSS Color |Partial BSS Color    |BSS Color Disabled |
    -------------------------------------------------------
    |6         |1                    |1                  |
    -------------------------------------------------------
    *******************************************************/
    he_bss_color->bss_color_disable = mac_mib_get_he_oper_bss_color_disable(hmac_vap);
    he_bss_color->bss_color = mac_mib_get_he_oper_bss_color(hmac_vap);
    buffer += 1;
    *ie_len += 1;

    /* 填充 Basic HE-MCS And NSS Set, 此字段为AP的基础能力, 适配支持接入AP的最小能力为单流MCS7 */
    mcs_bit_map = (mac_frame_he_mcs_nss_bit_map_stru *)buffer;
    (osal_void)memset_s(mcs_bit_map, sizeof(mac_frame_he_mcs_nss_bit_map_stru),
        0, sizeof(mac_frame_he_mcs_nss_bit_map_stru));

    mcs_bit_map->max_he_mcs_for_1ss = MAC_MAX_SUP_MCS7_11AX_EACH_NSS;
    mcs_bit_map->max_he_mcs_for_2ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    mcs_bit_map->max_he_mcs_for_3ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    mcs_bit_map->max_he_mcs_for_4ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    mcs_bit_map->max_he_mcs_for_5ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    mcs_bit_map->max_he_mcs_for_6ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    mcs_bit_map->max_he_mcs_for_7ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    mcs_bit_map->max_he_mcs_for_8ss = MAC_MAX_SUP_INVALID_11AX_EACH_NSS;
    buffer += MAC_HE_OPE_BASIC_MCS_NSS_LEN;
    *ie_len += MAC_HE_OPE_BASIC_MCS_NSS_LEN;

    *ie_length = *ie_len - MAC_IE_HDR_LEN;

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    hmac_wur_he_oper_ie_save(hmac_vap, he_opern_param, he_bss_color, mcs_bit_map);
#endif

    return;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_vap_set_cb_tx_user_idx
 功能描述  : 设置发送描述符的user_idx
*****************************************************************************/
osal_u32 hmac_vap_set_cb_tx_user_idx(hmac_vap_stru *hmac_vap, mac_tx_ctl_stru *tx_ctl, const osal_u8 *data)
{
    osal_u16 user_idx = MAC_INVALID_USER_ID;
    osal_u32 ret;

    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, data, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log4(0, OAM_SF_ANY,
            "hmac_vap_set_cb_tx_user_idx:: cannot find user_idx from %x:%x:%x:%x:xx:xx.",
            data[0], data[1], data[2], data[3]); /* MAC地址 第1，2，3 byte */
        oam_warning_log1(0, OAM_SF_ANY, "hmac_vap_set_cb_tx_user_idx set TX_USER_IDX %d.", MAC_INVALID_USER_ID);
        tx_ctl->tx_user_idx = MAC_INVALID_USER_ID;
        return ret;
    }

    tx_ctl->tx_user_idx = (osal_u8)user_idx;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
/*****************************************************************************
 功能描述 : 填充vender私有的4地址IE信息
 *****************************************************************************/
osal_void mac_set_vender_4addr_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    *ie_len = 0;

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID | Length |    OUI    |WIFI OUT FOUR ADDR |HE PHY Capa. Info| Version |
    -------------------------------------------------------------------------
    |221 |variable|     3     |         4         |        待定     |         |
    -------------------------------------------------------------------------
    ***************************************************************************/
    /* 直接调用11KV接口的设置IE接口 宏未定义编译失败 */
    #error "the virtual multi-sta feature dependent _PRE_WLAN_FEATURE_11KV_INTERFACE, please define it!"
}
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
/*****************************************************************************
 函 数 名 : hmac_ftm_add_to_ext_capabilities_ie
 功能描述 : 填充extended capabilities element信息
*****************************************************************************/
static osal_void hmac_ftm_add_to_ext_capabilities_ie(hmac_vap_stru *hmac_vap,
    mac_ext_cap_ie_stru *ext_cap_ie, osal_u8 *ie_len)
{
    /* 商用网卡（Tplink6200）关联不上APUT */
    mac_ftm_mode_enum_uint8 ftm_mode = mac_check_ftm_enable(hmac_vap);
    if (ftm_mode == MAC_FTM_DISABLE_MODE) {
        return;
    }

    switch (ftm_mode) {
        case MAC_FTM_RESPONDER_MODE:
            ext_cap_ie->ftm_resp = OSAL_TRUE;
            ext_cap_ie->ftm_int = OSAL_FALSE;
            break;

        case MAC_FTM_INITIATOR_MODE:
            ext_cap_ie->ftm_resp = OSAL_FALSE;
            ext_cap_ie->ftm_int = OSAL_TRUE;
            break;

        case MAC_FTM_MIX_MODE:
            ext_cap_ie->ftm_resp = OSAL_TRUE;
            ext_cap_ie->ftm_int = OSAL_TRUE;
            break;

        default:
            break;
    }

    (*ie_len) = MAC_XCAPS_EX_FTM_LEN;
}
#endif

#ifdef _PRE_WLAN_FEATURE_1024QAM
/*****************************************************************************
函 数 名  : hmac_set_1024qam_vendor_ie
功能描述  : 填充厂商自定义ie
*****************************************************************************/
osal_void hmac_set_1024qam_vendor_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_ieee80211_vendor_ie_stru *vendor_ie = OSAL_NULL;

    if (hmac_vap->cap_flag.is_1024qam != OSAL_TRUE) {
        *ie_len = 0;
        return;
    }

    vendor_ie = (mac_ieee80211_vendor_ie_stru *)buffer;
    vendor_ie->element_id = MAC_EID_VENDOR;
    vendor_ie->len = sizeof(mac_ieee80211_vendor_ie_stru) - MAC_IE_HDR_LEN;

    vendor_ie->oui_type = MAC_OSAL_1024QAM_IE;

    vendor_ie->oui[0] = (osal_u8)((MAC_EXT_VENDER_IE >> 16) & 0xff);     /* vendor IE 右移动16 bit */
    vendor_ie->oui[1] = (osal_u8)((MAC_EXT_VENDER_IE >> 8) & 0xff);      /* vendor IE 右移动8 bit */
    vendor_ie->oui[2] = (osal_u8)((MAC_EXT_VENDER_IE) & 0xff);           /* OUI的第2字节 */

    *ie_len = sizeof(mac_ieee80211_vendor_ie_stru);
}
#endif

/*****************************************************************************
 功能描述 : 填充Broadcom/Epigram 厂商自定义ie
*****************************************************************************/
osal_void mac_set_vendor_vht_ie(osal_void *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    osal_u8 len;

    buffer[0] = MAC_EID_VENDOR;
    buffer[1] = MAC_WLAN_OUI_VENDOR_VHT_HEADER; /* The Vendor OUI, type and subtype */
    buffer[2] = (osal_u8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 16) & 0xff);    /* vendor VHT IE的第2字节. 右移16 bit */
    buffer[3] = (osal_u8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 8) & 0xff);     /* vendor VHT IE的第3字节. 右移8 bit */
    buffer[4] = (osal_u8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM) & 0xff);          /* vendor VHT IE的第4字节 */
    buffer[5] = MAC_WLAN_OUI_VENDOR_VHT_TYPE;                               /* vendor VHT IE的第5字节 */
    buffer[6] = MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE;                            /* vendor VHT IE的第6字节 */

    hmac_set_vht_capabilities_ie_etc(hmac_vap, buffer + buffer[1] + MAC_IE_HDR_LEN, &len);
    if (len != 0) {
        buffer[1] += len;
        *ie_len = buffer[1] + MAC_IE_HDR_LEN;
    } else {
        *ie_len = 0;
    }
}

/*****************************************************************************
 功能描述 : 填充Broadcom/Epigram 厂商自定义ie，适配5g 20M MCS9问题
*****************************************************************************/
osal_void mac_set_vendor_novht_ie(osal_void *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    unref_param(hmac_vap);
    buffer[0] = MAC_EID_VENDOR;
    buffer[1] = MAC_WLAN_OUI_VENDOR_VHT_HEADER; /* The Vendor OUI, type and subtype */
    buffer[2] = (osal_u8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 16) & 0xff);    /* vendor noVHT IE的第2字节. 右移16 bit */
    buffer[3] = (osal_u8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM >> 8) & 0xff);     /* vendor noVHT IE的第3字节. 右移8 bit */
    buffer[4] = (osal_u8)((MAC_WLAN_OUI_BROADCOM_EPIGRAM) & 0xff);          /* vendor noVHT IE的第4字节 */
    buffer[5] = MAC_WLAN_OUI_VENDOR_VHT_TYPE;                               /* vendor noVHT IE的第5字节 */
    buffer[6] = MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE3;                           /* vendor noVHT IE的第6字节 */

    *ie_len = buffer[1] + MAC_IE_HDR_LEN;
}

/*****************************************************************************
 函 数 名  : mac_set_dsss_params_etc
 功能描述  : 填充DS参数集
*****************************************************************************/
osal_void mac_set_dsss_params_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u8 chan_num)
{
    /***************************************************************************
                        ----------------------------------------
                        | Element ID  | Length |Current Channel|
                        ----------------------------------------
              Octets:   | 1           | 1      | 1             |
                        ----------------------------------------
    The DSSS Parameter Set element contains information to allow channel number identification for STAs.
    ***************************************************************************/
    /* 5G下beacon或者probe rsp帧中不携带dsss ie信息，因此,导致有些商用网卡
       扫描到的ap信道号和ap实际配置的信道号概率性不一致，导致入网失败；为增加兼容性，在5G模式下，也
       携带dsss ie信息，并且通过抓包，发现多款商用ap，5G only也携带了dsss ie信息 */
    unref_param(vap);
    buffer[0] = MAC_EID_DSPARMS; /* 0 */
    buffer[1] = MAC_DSPARMS_LEN; /* 1 */
    buffer[2] = chan_num;        /* 2 */

    *ie_len = MAC_IE_HDR_LEN + MAC_DSPARMS_LEN;
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    if (hmac_vap->cap_flag.wur_enable == OSAL_TRUE) {
        /* WUR BSS参数保存 */
        hmac_vap->wur_bss->wur_bss_cur.dsss_channel_num = chan_num;
    }
#endif
}

#ifdef _PRE_WLAN_FEATURE_11D
/*****************************************************************************
 函 数 名  : mac_set_country_ie_etc
 功能描述  : 填充Country信息
*****************************************************************************/
osal_void mac_set_country_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_regdomain_info_stru *rd_info = OSAL_NULL;
    osal_u8 band;
    osal_u8 index;
    osal_u32 ret;
    osal_u8 len = 0;

    if ((mac_mib_get_dot11_multi_domain_capability_activated(hmac_vap) != OSAL_TRUE) &&
        (mac_mib_get_spectrum_management_required(hmac_vap) != OSAL_TRUE) &&
        (mac_mib_get_dot11_radio_measurement_activated(hmac_vap) != OSAL_TRUE)) {
        /* 没有使能管制域ie */
        *ie_len = 0;

        return;
    }

    /***************************************************************************
                               |....These three fields are repeated...|
    -------------------------------------------------------------------------------
    |EID | Len | CountryString | First Channel |Number of |Maximum    | Pad       |
    |    |     |               |   Number/     |Channels/ |Transmit   |(if needed)|
    |    |     |               |   Operating   | Operating|Power Level|           |
    |    |     |               |   Extension   | Class    |/Coverage  |           |
    |    |     |               |   Identifier  |          |Class      |           |
    -------------------------------------------------------------------------------
    |1   |1    |3              |1              |1         |1          |0 or 1     |
    -------------------------------------------------------------------------------
    ***************************************************************************/
    /* 读取管制域信息 */
    hmac_get_regdomain_info_etc(&rd_info);

    /* 获取当前工作频段 */
    band = hmac_vap->channel.band;

    /* 填写EID, 长度最后填 */
    buffer[0] = MAC_EID_COUNTRY;

    /* 初始化填写buffer的位置 */
    index = MAC_IE_HDR_LEN;

    /* 国家码 */
    buffer[index++] = (osal_u8)(rd_info->ac_country[0]);
    buffer[index++] = (osal_u8)(rd_info->ac_country[1]);
    buffer[index++] = ' '; /* 0表示室内室外规定相同 */

    if (band == WLAN_BAND_2G) {
        ret = mac_set_country_ie_2g_etc(rd_info, &(buffer[index]), &len);
#ifdef _PRE_WLAN_SUPPORT_5G
    } else if (band == WLAN_BAND_5G) {
        ret = mac_set_country_ie_5g_etc(rd_info, &(buffer[index]), &len);
#endif
    } else {
        ret = OAL_FAIL;
    }

    if (ret != OAL_SUCC) {
        *ie_len = 0;
        return;
    }

    if (len == 0) {
        /* 无管制域内容 */
        *ie_len = 0;

        return;
    }

    index += len;

    /* 如果总长度为奇数，则补1字节pad */
    if ((index & BIT0) == 1) {
        buffer[index] = 0;
        index += 1;
    }

    /* 设置信息元素长度 */
    buffer[1] = index - MAC_IE_HDR_LEN;
    *ie_len = index;
}
#endif

/*****************************************************************************
 函 数 名  : mac_set_pwrconstraint_ie_etc
 功能描述  : 填充power constraint信息

*****************************************************************************/
osal_void mac_set_pwrconstraint_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    /***************************************************************************
                   -------------------------------------------
                   |ElementID | Length | LocalPowerConstraint|
                   -------------------------------------------
       Octets:     |1         | 1      | 1                   |
                   -------------------------------------------

    向工作站描述其所允许的最大传输功率，此信息元素记录规定最大值
    减去实际使用时的最大值
    ***************************************************************************/
    if (mac_mib_get_spectrum_management_required(hmac_vap) == OSAL_FALSE) {
        *ie_len = 0;

        return;
    }

    *buffer = MAC_EID_PWRCNSTR;
    *(buffer + 1) = MAC_PWR_CONSTRAINT_LEN;

    /* Note that this field is always set to 0 currently. Ideally            */
    /* this field can be updated by having an algorithm to decide transmit   */
    /* power to be used in the BSS by the AP.                                */
    *(buffer + MAC_IE_HDR_LEN) = 0;

    *ie_len = MAC_IE_HDR_LEN + MAC_PWR_CONSTRAINT_LEN;
}

/*****************************************************************************
 函 数 名  : mac_set_quiet_ie_etc
 功能描述  : 填充quiet信息
*****************************************************************************/
osal_void mac_set_quiet_ie_etc(const osal_void *vap, osal_u8 *buffer,
    const mac_quiet_info *quiet_info, osal_u8 *ie_len)
{
    /* 管制域相关 tbd, 需要11h特性进一步分析此ie的设置 */
    mac_quiet_ie_stru *quiet = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    if ((mac_mib_get_spectrum_management_required(hmac_vap) != OSAL_TRUE) &&
        (mac_mib_get_dot11_radio_measurement_activated(hmac_vap) != OSAL_TRUE)) {
        *ie_len = 0;

        return;
    }

    /***************************************************************************
    -----------------------------------------------------------------------------
    |ElementID | Length | QuietCount | QuietPeriod | QuietDuration | QuietOffset|
    -----------------------------------------------------------------------------
    |1         | 1      | 1          | 1           | 2             | 2          |
    -----------------------------------------------------------------------------
    ***************************************************************************/
    if (quiet_info == OSAL_NULL || quiet_info->duration == 0 || quiet_info->count == 0) {
        *ie_len = 0;

        return;
    }

    *buffer = MAC_EID_QUIET;

    *(buffer + 1) = MAC_QUIET_IE_LEN;

    quiet = (mac_quiet_ie_stru *)(buffer + MAC_IE_HDR_LEN);

    quiet->quiet_count = quiet_info->count;
    quiet->quiet_period = quiet_info->period;
    quiet->quiet_duration = quiet_info->duration;
    quiet->quiet_offset = quiet_info->offset;

    *ie_len = MAC_IE_HDR_LEN + MAC_QUIET_IE_LEN;
}

/*****************************************************************************
 函 数 名  : mac_set_tpc_report_ie_etc
 功能描述  : 填充tpc report信息
*****************************************************************************/
osal_void mac_set_tpc_report_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    /***************************************************************************
                -------------------------------------------------
                |ElementID  |Length  |TransmitPower  |LinkMargin|
                -------------------------------------------------
       Octets:  |1          |1       |1              |1         |
                -------------------------------------------------

    TransimitPower, 此帧的传送功率，以dBm为单位
    ***************************************************************************/
    if ((mac_mib_get_spectrum_management_required(hmac_vap) == OSAL_FALSE) &&
        (mac_mib_get_dot11_radio_measurement_activated(hmac_vap) == OSAL_FALSE)) {
        *ie_len = 0;

        return;
    }

    *buffer = MAC_EID_TPCREP;
    *(buffer + 1) = MAC_TPCREP_IE_LEN;          /* 1 */
    *(buffer + 2) = hmac_vap->tx_power;          /* 2 */
    *(buffer + 3) = 0; /* 此字段管理帧中不用 */   /* 3 */

    *ie_len = MAC_IE_HDR_LEN + MAC_TPCREP_IE_LEN;
}

/*****************************************************************************
 函 数 名  : mac_set_erp_ie_etc
 功能描述  : 填充Quiet信息
*****************************************************************************/
osal_void mac_set_erp_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_erp_params_stru *erp_params = OSAL_NULL;

    /***************************************************************************
    --------------------------------------------------------------------------
    |EID  |Len  |NonERP_Present|Use_Protection|Barker_Preamble_Mode|Reserved|
    --------------------------------------------------------------------------
    |B0-B7|B0-B7|B0            |B1            |B2                  |B3-B7   |
    --------------------------------------------------------------------------
    ***************************************************************************/
    if ((hmac_vap->channel.band == WLAN_BAND_5G) || (hmac_vap->protocol == WLAN_LEGACY_11B_MODE)) {
        *ie_len = 0;

        return; /* 5G频段和11b协议模式 没有erp信息 */
    }

    *buffer = MAC_EID_ERP;
    *(buffer + 1) = MAC_ERP_IE_LEN; /* 1 */
    *(buffer + MAC_IE_HDR_LEN) = 0; /* 初始清0 */

    erp_params = (mac_erp_params_stru *)(buffer + MAC_IE_HDR_LEN);

    /* 如果存在non erp站点与ap关联， 或者obss中存在non erp站点 */
    if ((hmac_vap->protection.sta_non_erp_num != 0) || (hmac_vap->protection.obss_non_erp_present == OSAL_TRUE)) {
        erp_params->non_erp = 1;
    } else {
        erp_params->non_erp = 0;
    }

    /* 如果ap已经启用erp保护 */
    if (hmac_vap->protection.protection_mode == WLAN_PROT_ERP) {
        erp_params->use_protection = 1;
    } else {
        erp_params->use_protection = 0;
    }

    /* 如果存在不支持short preamble的站点与ap关联， 或者ap自身不支持short preamble */
    if ((hmac_vap->protection.sta_no_short_preamble_num != 0) ||
        (mac_mib_get_short_preamble_option_implemented(hmac_vap) == OSAL_FALSE)) {
        erp_params->preamble_mode = 1;
    }

    *ie_len = MAC_IE_HDR_LEN + MAC_ERP_IE_LEN;
}

/*****************************************************************************
 函 数 名  : mac_set_bssload_ie
 功能描述  : 填充bss load信息
*****************************************************************************/
osal_void mac_set_bssload_ie(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    mac_bss_load_stru *bss_load = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    if (mac_mib_get_dot11_qos_option_implemented(hmac_vap) == OSAL_FALSE ||
        mac_mib_get_dot11_qbss_load_implemented(hmac_vap) == OSAL_FALSE) {
        *ie_len = 0;

        return;
    }

    /***************************************************************************
    ------------------------------------------------------------------------
    |EID |Len |StationCount |ChannelUtilization |AvailableAdmissionCapacity|
    ------------------------------------------------------------------------
    |1   |1   |2            |1                  |2                         |
    ------------------------------------------------------------------------
    ***************************************************************************/
    buffer[0] = MAC_EID_QBSS_LOAD;
    buffer[1] = MAC_BSS_LOAD_IE_LEN;

    bss_load = (mac_bss_load_stru *)(buffer + MAC_IE_HDR_LEN);

    bss_load->sta_count = hmac_vap->user_nums;

    bss_load->chan_utilization = 0;

    bss_load->aac = 0;

    *ie_len = MAC_IE_HDR_LEN + MAC_BSS_LOAD_IE_LEN;
}

/*****************************************************************************
 功能描述  : 填充用户态下发的扩展信息元素到管理帧中
*****************************************************************************/
osal_void mac_add_extend_app_ie_etc(const osal_void *vap, osal_u8 **buffer, app_ie_type_uint8 type)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u16 app_ie_len = 0;
    osal_u8 ie_index;

    for (ie_index = OAL_APP_EXTEND_IE1; ie_index <= OAL_APP_EXTEND_IE4; ie_index++) {
        if ((hmac_vap->app_ie[ie_index].ie_len > 0) && ((hmac_vap->app_ie[ie_index].ie_type_bitmap & type) != 0)) {
            hmac_add_app_ie_etc((osal_void *)hmac_vap, *buffer, &app_ie_len, ie_index);
            *buffer += app_ie_len;
        }
    }

    return;
}

/*****************************************************************************
 函 数 名  : mac_set_wps_ie
 功能描述  : 填充用户态下发的信息元素到管理帧中
*****************************************************************************/
osal_void mac_add_wps_ie_etc(const osal_void *vap, osal_u8 *buffer, osal_u16 *ie_len, app_ie_type_uint8 type)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *app_ie;
    osal_u8 *wps_ie = OSAL_NULL;
    osal_u32 app_ie_len;

    app_ie = hmac_vap->app_ie[type].ie;
    app_ie_len = hmac_vap->app_ie[type].ie_len;

    if (app_ie_len == 0) {
        *ie_len = 0;
        return;
    }

    wps_ie =
        hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, app_ie, (osal_s32)app_ie_len);
    if ((wps_ie == OSAL_NULL) || (wps_ie[1] < MAC_MIN_WPS_IE_LEN)) {
        *ie_len = 0;
        return;
    }

    /* 将WPS ie 信息拷贝到buffer 中 */
    (osal_void)memcpy_s(buffer, wps_ie[1] + MAC_IE_HDR_LEN, wps_ie, wps_ie[1] + MAC_IE_HDR_LEN);
    *ie_len = wps_ie[1] + MAC_IE_HDR_LEN;

    return;
}

/*****************************************************************************
 函 数 名  : mac_frame_is_null_data
 功能描述  : 该帧是否为NULL DATA帧
*****************************************************************************/
WIFI_TCM_TEXT oal_bool_enum_uint8 mac_frame_is_null_data(oal_netbuf_stru *net_buf)
{
    osal_u8 *mac_header;
    osal_u8 frame_type;
    osal_u8 frame_subtype;
    mac_tx_ctl_stru *tx_ctl;

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(net_buf);
    mac_header = (osal_u8 *)(mac_get_frame_header_addr(tx_ctl));
    frame_type = mac_frame_get_type_value((osal_u8 *)mac_header);
    frame_subtype = mac_frame_get_subtype_value((const osal_u8 *)mac_header);
    if ((frame_type == WLAN_DATA_BASICTYPE) &&
        ((frame_subtype == WLAN_NULL_FRAME) || (frame_subtype == WLAN_QOS_NULL_FRAME))) {
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述 : rom cb填充extended capabilities element信息
*****************************************************************************/
static osal_void hmac_set_ext_capabilities_ie_update_etc(hmac_vap_stru *hmac_vap,
    mac_ext_cap_ie_stru *ext_cap_ie, osal_u8 *ie_len)
{
    osal_void *fhook = OSAL_NULL;

#ifdef _PRE_WLAN_FEATURE_FTM
    hmac_ftm_add_to_ext_capabilities_ie(hmac_vap, ext_cap_ie, ie_len);
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_TWT_SET_EXT_CAP_TWT_REQ);
    if (fhook != OSAL_NULL) {
        ((hmac_set_ext_cap_twt_req_cb)fhook)(hmac_vap, ext_cap_ie, ie_len);
    }
}

/*****************************************************************************
 函 数 名  : hmac_set_ht_cap_ie_rom_cb
 功能描述  : rom cb填充dmac_set_ht_cap_ie_rom_cb信息
*****************************************************************************/
static osal_void hmac_set_ht_cap_ie_rom_cb(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len)
{
    mac_frame_ht_cap_stru *ht_capinfo = OSAL_NULL;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *buffer = buf;
    unref_param(ie_len);

    /* 将buffer地址指向ht_capinfo字段,buffer当前地址在HT_ASEL字段 */
    buffer -= MAC_HT_CAP_LEN - MAC_HT_ASEL_LEN;

    ht_capinfo = (mac_frame_ht_cap_stru *)buffer;
    /* 设置所支持的信道宽度集"，0:仅20MHz运行; 1:20MHz与40MHz运行 */
    if ((mac_vap_get_cap_bw(hmac_vap) > WLAN_BAND_WIDTH_20M) == OSAL_TRUE) {
        ht_capinfo->supported_channel_width = mac_mib_get_forty_mhz_operation_implemented(hmac_vap);
    }

    /* 只有支持40M的情况下，才可以宣称支持40M short GI */
    if (ht_capinfo->supported_channel_width != 0) {
        ht_capinfo->short_gi_40mhz = mac_mib_get_short_gi_option_in_forty_implemented(hmac_vap);
    } else {
        ht_capinfo->short_gi_40mhz = 0;
    }
}

/*****************************************************************************
 功能描述 : 设置vht capinfo中的信息
*****************************************************************************/
static osal_void hmac_set_vht_capinfo_field_cb(osal_void *vap, osal_u8 *buffer)
{
    unref_param(vap);
    unref_param(buffer);
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_vht_cap_info_stru *vht_capinfo = (mac_vht_cap_info_stru *)buffer;

    /* 算法限定mu bfee只在WLAN0开启 */
    if (!is_legacy_sta(hmac_vap)) {
        vht_capinfo->mu_beamformee_cap = OSAL_FALSE;
    }
    if (is_legacy_ap(hmac_vap)) {
        vht_capinfo->supported_channel_width = 0;
        vht_capinfo->short_gi_160mhz = 0;
    }
#endif
}

/*****************************************************************************
 函 数 名  : hmac_set_ht_opern_ie_rom_cb
 功能描述  : rom cb填充dmac_set_ht_opern_ie_rom_cb信息
*****************************************************************************/
static osal_void hmac_set_ht_opern_ie_rom_cb(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap  = (hmac_vap_stru *)vap;
    mac_ht_opern_stru *ht_opern_tmp = (mac_ht_opern_stru *)(buffer + MAC_IE_HDR_LEN);
    unref_param(ie_len);

    /* 设置"次信道偏移量" */
    switch (hmac_vap->channel.en_bandwidth) {
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            ht_opern_tmp->secondary_chan_offset = MAC_SCA;
            break;

        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            ht_opern_tmp->secondary_chan_offset = MAC_SCB;
            break;
        default:
            ht_opern_tmp->secondary_chan_offset = MAC_SCN;
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_set_vht_opern_ie_rom_cb
 功能描述  : rom cb填充dmac_set_vht_opern_ie_rom_cb信息
*****************************************************************************/
static osal_void hmac_set_vht_opern_ie_rom_cb(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap  = (hmac_vap_stru *)vap;
    mac_vht_opern_stru *mac_vht_opern = (mac_vht_opern_stru *)buffer;
    unref_param(ie_len);

    if ((hmac_vap->channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (hmac_vap->channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        mac_vht_opern->channel_width = WLAN_MIB_VHT_OP_WIDTH_80;
    } else {
        mac_vht_opern->channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
    }

    switch (hmac_vap->channel.en_bandwidth) {
        case WLAN_BAND_WIDTH_80PLUSPLUS:
            /***********************************************************************
            | 主20 | 从20 | 从40       |
                        |
                        |中心频率相对于主20偏6个信道
            ************************************************************************/
            mac_vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number + 6; /* 6 */
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            /***********************************************************************
            | 从40        | 主20 | 从20 |
                        |
                        |中心频率相对于主20偏-2个信道
            ************************************************************************/
            mac_vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number - 2; /* 2 */
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            /***********************************************************************
            | 从20 | 主20 | 从40       |
                        |
                        |中心频率相对于主20偏2个信道
            ************************************************************************/
            mac_vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number + 2; /* 2 */
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            /***********************************************************************
            | 从40        | 从20 | 主20 |
                        |
                        |中心频率相对于主20偏-6个信道
            ************************************************************************/
            mac_vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number - 6; /* 6 */
            break;
        default:
            /* 中心频率直接填0  */
            mac_vht_opern->channel_center_freq_seg0 = 0; /* 0 */
            break;
    }
}

#ifdef _PRE_WLAN_FEATURE_P2P
/*****************************************************************************
 功能描述 : 查找P2P attribute信息元素
*****************************************************************************/
osal_u8 *hmac_find_p2p_attribute_etc(osal_u8 eid, osal_u8 *ies, osal_s32 len)
{
    osal_s32 ie_len;

    if (ies == OSAL_NULL) {
        return OSAL_NULL;
    }

    /* 查找P2P IE，如果不是直接找下一个 */
    while (len > MAC_P2P_ATTRIBUTE_HDR_LEN && ies[0] != eid) {
        ie_len = (osal_s32)((ies[MAC_IE_HDR_LEN] << 8) + ies[1]); /* 8: 1 Byte */
        len -= ie_len + MAC_P2P_ATTRIBUTE_HDR_LEN;
        ies += ie_len + MAC_P2P_ATTRIBUTE_HDR_LEN;
    }

    /* 查找到P2P IE，剩余长度不匹配直接返回空指针 */
    ie_len = (osal_s32)((ies[MAC_IE_HDR_LEN] << 8) + ies[1]); /* 8: 1 Byte */
    if ((len < MAC_P2P_ATTRIBUTE_HDR_LEN) || (len < (MAC_P2P_ATTRIBUTE_HDR_LEN + ie_len))) {
        return OSAL_NULL;
    }

    return ies;
}
#endif

/*****************************************************************************
 功能描述 : 查找指定的IE
*****************************************************************************/
osal_u8 *mac_find_ie_etc(osal_u8 eid, osal_u8 *ie_addr, osal_s32 frame_len)
{
    osal_s32 len = frame_len;
    osal_u8 *ies = ie_addr;

    if (ies == OSAL_NULL) {
        return OSAL_NULL;
    }

    while (len > MAC_IE_HDR_LEN && ies[0] != eid) {
        len -= ies[1] + MAC_IE_HDR_LEN;
        ies += ies[1] + MAC_IE_HDR_LEN;
    }

    if ((len < MAC_IE_HDR_LEN) || (len < (MAC_IE_HDR_LEN + ies[1])) || ((len == MAC_IE_HDR_LEN) && (ies[0] != eid))) {
        return OSAL_NULL;
    }

    return ies;
}

/*****************************************************************************
 功能描述 : 查找指定的IE
*****************************************************************************/
osal_u8 *mac_find_ie_sec_etc(osal_u8 eid, osal_u8 *ie_addr, osal_s32 len, osal_u32 *remain_len)
{
    osal_s32 frame_len = len;
    osal_u8 *ies = ie_addr;

    if (ies == OAL_PTR_NULL) {
        return OAL_PTR_NULL;
    }

    if (frame_len > 1500) { /* buffer长度超过1500字节认为入参异常，不做查找操作 */
        return OAL_PTR_NULL;
    }

    while (frame_len > MAC_IE_HDR_LEN && ies[0] != eid) {
        frame_len -= ies[1] + MAC_IE_HDR_LEN;
        ies += ies[1] + MAC_IE_HDR_LEN;
    }

    if ((frame_len < MAC_IE_HDR_LEN) || (frame_len < (MAC_IE_HDR_LEN + ies[1])) ||
        ((frame_len == MAC_IE_HDR_LEN) && (ies[0] != eid))) {
        return OAL_PTR_NULL;
    }

    *remain_len = (osal_u32)frame_len;
    return ies;
}

/*****************************************************************************
 功能描述 : 查找厂家自定义 IE
*****************************************************************************/
osal_u8 *hmac_find_vendor_ie_etc(osal_u32 oui, osal_u8 oui_type, osal_u8 *ies, osal_s32 len)
{
    struct mac_ieee80211_vendor_ie *ie = OSAL_NULL;
    osal_u8 *pos = OSAL_NULL;
    osal_u8 *end = OSAL_NULL;
    osal_u32 ie_oui;

    if (ies == OSAL_NULL) {
        return OSAL_NULL;
    }

    pos = ies;
    end = ies + len;
    while (pos < end) {
        pos = mac_find_ie_etc(MAC_EID_VENDOR, pos, (osal_s32)(end - pos));
        if (pos == OSAL_NULL) {
            return OSAL_NULL;
        }

        ie = (struct mac_ieee80211_vendor_ie *)pos;
        if (ie->len >= (sizeof(*ie) - MAC_IE_HDR_LEN)) {
            ie_oui = (ie->oui[0] << 16) | (ie->oui[1] << 8) | (ie->oui[2]); /* OUI第2字节. 左移16bit和8bit */
            if ((ie_oui == oui) && (ie->oui_type == oui_type)) {
                return pos;
            }
        }
        pos += MAC_IE_HDR_LEN + ie->len;
    }
    return OSAL_NULL;
}

/*****************************************************************************
 函 数 名  : hmac_set_beacon_interval_field_etc
 功能描述  : 设置beacon interval元素
*****************************************************************************/
osal_void hmac_set_beacon_interval_field_etc(osal_void *vap, osal_u8 *buffer)
{
    osal_u16 *bcn_int;
    osal_u32 bcn_int_val;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    /*****************************************************************************
                |Beacon interval|
        Octets:        2
    *****************************************************************************/
    bcn_int = (osal_u16 *)buffer;

    bcn_int_val = mac_mib_get_beacon_period(hmac_vap);

    *bcn_int = (osal_u16)bcn_int_val;
}

/*****************************************************************************
 功能描述 : 作为ap时，根据mib值，设置cap info
*****************************************************************************/
osal_void hmac_set_cap_info_ap_etc(osal_void *vap, osal_u8 *cap_info)
{
    mac_cap_info_stru *cap_info_tmp = (mac_cap_info_stru *)cap_info;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    /**************************************************************************
         -------------------------------------------------------------------
         |B0 |B1  |B2        |B3    |B4     |B5      |B6  |B7     |B8      |
         -------------------------------------------------------------------
         |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|SpecMgmt|
         -------------------------------------------------------------------
         |B9 |B10      |B11 |B12     |B13      |B14        |B15            |
         -------------------------------------------------------------------
         |QoS|ShortSlot|APSD|RM      |DSSS-OFDM|Delayed BA |Immediate BA   |
         -------------------------------------------------------------------
    ***************************************************************************/
    /* 初始清零 */
    cap_info[0] = 0;
    cap_info[1] = 0;

    if (mac_mib_get_desired_bss_type(hmac_vap) == WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT) {
        cap_info_tmp->ibss = 1;
    } else if (mac_mib_get_desired_bss_type(hmac_vap) == WLAN_MIB_DESIRED_BSSTYPE_INFRA) {
        cap_info_tmp->ess = 1;
    }

    /* The Privacy bit is set if WEP is enabled */
    cap_info_tmp->privacy = mac_mib_get_privacyinvoked(hmac_vap);

    /* preamble */
    cap_info_tmp->short_preamble = mac_mib_get_short_preamble_option_implemented(hmac_vap);

    /* packet binary convolutional code (PBCC) modulation */
    cap_info_tmp->pbcc = mac_mib_get_pbcc_option_implemented(hmac_vap);

    /* Channel Agility */
    cap_info_tmp->channel_agility = mac_mib_get_channel_agility_present(hmac_vap);

    /* Spectrum Management */
    cap_info_tmp->spectrum_mgmt = mac_mib_get_spectrum_management_required(hmac_vap);

    /* QoS subfield */
    /* ccb决策修改assoc rsp能力信息中的qos为0; mib里面不修改 */
    cap_info_tmp->qos = 0;

    /* short slot */
    cap_info_tmp->short_slot_time = mac_mib_get_short_slot_time_option_implemented(hmac_vap) &
        mac_mib_get_short_slot_time_option_activated(hmac_vap);

    /* APSD */
    cap_info_tmp->apsd = mac_mib_get_dot11_apsd_option_implemented(hmac_vap);

    /* Radio Measurement */
    cap_info_tmp->radio_measurement = mac_mib_get_dot11_radio_measurement_activated(hmac_vap);

    /* DSSS-OFDM */
    cap_info_tmp->dsss_ofdm = mac_mib_get_dsss_ofdm_option_activated(hmac_vap);

    /* Delayed BA */
    cap_info_tmp->delayed_block_ack = mac_mib_get_dot11_delayed_block_ack_option_implemented(hmac_vap);

    /* Immediate Block Ack 参考STA及AP标杆，此能力一直为0，实际通过addba协商。此处修改为标杆一致。mib值不修改 */
    cap_info_tmp->immediate_block_ack = 0;
}

/*****************************************************************************
 功能描述 : STA根据关联用户的能力信息，设置关联请求帧中的cap info
*****************************************************************************/
osal_void mac_set_cap_info_sta_etc(osal_void *vap, osal_u8 *cap_info)
{
    mac_cap_info_stru *cap_info_tmp = (mac_cap_info_stru *)cap_info;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    /**************************************************************************
         -------------------------------------------------------------------
         |B0 |B1  |B2        |B3    |B4     |B5      |B6  |B7     |B8      |
         -------------------------------------------------------------------
         |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|SpecMgmt|
         -------------------------------------------------------------------
         |B9 |B10      |B11 |B12     |B13      |B14        |B15            |
         -------------------------------------------------------------------
         |QoS|ShortSlot|APSD|RM      |DSSS-OFDM|Delayed BA |Immediate BA   |
         -------------------------------------------------------------------
    ***************************************************************************/
    /* 学习对端的能力信息 */
    (osal_void)memcpy_s(cap_info, sizeof(mac_cap_info_stru),
        (osal_u8 *)(&hmac_vap->assoc_user_cap_info), sizeof(mac_cap_info_stru));

    /* 以下能力位不学习，保持默认值 */
    cap_info_tmp->ibss = 0;
    cap_info_tmp->cf_pollable = 0;
    cap_info_tmp->cf_poll_request = 0;
    cap_info_tmp->radio_measurement =
        (mac_mib_get_dot11_radio_measurement_activated(hmac_vap) & (cap_info_tmp->radio_measurement));
}

/*****************************************************************************
 功能描述 : 设置ssid ie
*****************************************************************************/
osal_void hmac_set_ssid_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u16 frm_type)
{
    osal_u8 *ssid = OSAL_NULL;
    osal_u8 ssid_len;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /***************************************************************************
      A SSID  field  of length 0 is  used  within Probe
      Request management frames to indicate the wildcard SSID.
    ***************************************************************************/
    /* 只有beacon会隐藏ssid */
    if ((hmac_vap->cap_flag.hide_ssid == OSAL_TRUE) && (frm_type == WLAN_FC0_SUBTYPE_BEACON)) {
        /* ssid ie */
        *buffer = MAC_EID_SSID;
        /* ssid len */
        *(buffer + 1) = 0;
        *ie_len = MAC_IE_HDR_LEN;
        return;
    }

    *buffer = MAC_EID_SSID;

    ssid = mac_mib_get_desired_ssid(hmac_vap);

    ssid_len = (osal_u8)osal_strlen((osal_char *)ssid); /* 不包含'\0' */

    *(buffer + 1) = ssid_len;

    (osal_void)memcpy_s(buffer + MAC_IE_HDR_LEN, ssid_len, ssid, ssid_len);

    *ie_len = ssid_len + MAC_IE_HDR_LEN;
}

/*****************************************************************************
 功能描述 : 设置速率集
*****************************************************************************/
osal_void hmac_set_supported_rates_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_rateset_stru *rates_set;
    osal_u8 nrates;
    osal_u8 idx;

    rates_set = &(hmac_vap->curr_sup_rates.rate);
    /* AP模式 or STA全信道扫描时根据频段设置supported rates */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA || hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (hmac_vap->protocol == WLAN_VHT_MODE || hmac_vap->protocol == WLAN_HE_MODE)) {
        if (hmac_vap->channel.band < WLAN_BAND_BUTT) {
            rates_set = &(hmac_vap->sta_sup_rates_ie[hmac_vap->channel.band].rate);
        }
    }

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

    nrates = rates_set->rs_nrates;

    if (nrates > MAC_MAX_SUPRATES) {
        nrates = MAC_MAX_SUPRATES;
    }

    for (idx = 0; idx < nrates; idx++) {
        buffer[MAC_IE_HDR_LEN + idx] = rates_set->rs_rates[idx].mac_rate;
    }
#ifdef _PRE_WLAN_FEATURE_WPA3
    if (nrates < MAC_MAX_SUPRATES) {
        /* support_rate 速率个数小于8，且SAE_PWE = HASH_TO_ELEMENT, SUPPORT_RATE IE中增加SAE_H2E_ONLY字段 */
        if (hmac_vap->sae_pwe == NL80211_SAE_PWE_HASH_TO_ELEMENT) {
            nrates++;
            buffer[MAC_IE_HDR_LEN + idx] = 0x80 | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
        }
    }
#endif
    buffer[1] = nrates;

    *ie_len = MAC_IE_HDR_LEN + nrates;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
/* WPA3 sae pwe填充rsnx */
osal_void hmac_set_rsnx_ie(const osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    *ie_len = 0;
    if ((hmac_vap->sae_pwe == NL80211_SAE_PWE_HASH_TO_ELEMENT) || (hmac_vap->sae_pwe == NL80211_SAE_PWE_BOTH)) {
        buffer[0] = MAC_EID_RSNX;
        buffer[1] = 0x1;
        buffer[MAC_IE_HDR_LEN] = 0x20;
        *ie_len = MAC_IE_HDR_LEN + 0x1;
    }
}
#endif
/*****************************************************************************
 功能描述 : AP 填充rsn信息
*****************************************************************************/
osal_void hmac_set_rsn_ie_etc(osal_void *vap, osal_u8 *pmkid, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_rsn_cap_stru *rsn_cap = OSAL_NULL;
    osal_u8 loop, index, pair_suites_num, akm_suites_num;
    osal_u32 group_suit, group_mgmt_suit;
    osal_u32 pcip[WLAN_PAIRWISE_CIPHER_SUITES] = {0};
    osal_u32 akm[WLAN_AUTHENTICATION_SUITES] = {0};

    if (mac_mib_get_rsnaactivated(hmac_vap) != OSAL_TRUE ||
#ifdef _PRE_WLAN_FEATURE_WPA3
        ((hmac_vap->cap_flag.wpa2 != OSAL_TRUE) && (hmac_vap->cap_flag.wpa3 != OSAL_TRUE))
#else
        (hmac_vap->cap_flag.wpa2 != OSAL_TRUE)
#endif
    ) {
        *ie_len = 0;
        return;
    }

    group_suit = mac_mib_get_rsn_group_suite(hmac_vap);
    group_mgmt_suit = mac_mib_get_rsn_group_mgmt_suite(hmac_vap);
    pair_suites_num = mac_mib_get_rsn_pair_suites(hmac_vap, pcip, sizeof(pcip));
    akm_suites_num = mac_mib_get_rsn_akm_suites(hmac_vap, akm, sizeof(akm));
    if ((pair_suites_num == 0) || (akm_suites_num == 0)) {
        *ie_len = 0;
        return;
    }

    /*************************************************************************/
    /*                  RSN Element Format              */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 2       |      4             |     2           */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite | Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       | 4-m                        |     2          | 4-n             */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |    2           |    2      |16 -s     |         4                 | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    index = MAC_IE_HDR_LEN;

    /* 设置RSN ie的EID */
    buffer[0] = MAC_EID_RSN;

    /* 设置version字段 */
    buffer[index++] = MAC_RSN_IE_VERSION;
    buffer[index++] = 0;

    (osal_void)memcpy_s(&buffer[index], MAC_DATA_HEADER_LEN, (osal_u8 *)(&group_suit), MAC_DATA_HEADER_LEN);
    index += 4; /* 4 */

    /* 设置成对加密套件 */
    buffer[index++] = pair_suites_num;
    buffer[index++] = 0;

    for (loop = 0; loop < pair_suites_num; loop++) {
        (osal_void)memcpy_s(&buffer[index], MAC_DATA_HEADER_LEN, (osal_u8 *)(&pcip[loop]), MAC_DATA_HEADER_LEN);
        index += 4; /* 4 */
    }

    /* 设置认证套件数 */
    buffer[index++] = akm_suites_num;
    buffer[index++] = 0;

    /* 根据MIB 值，设置认证套件内容 */
    for (loop = 0; loop < akm_suites_num; loop++) {
        (osal_void)memcpy_s(&buffer[index], MAC_DATA_HEADER_LEN, (osal_u8 *)(&akm[loop]), MAC_DATA_HEADER_LEN);
        index += 4; /* 4 */
    }

    /* 设置 RSN Capabilities字段 */
    /*************************************************************************/
    /* --------------------------------------------------------------------- */
    /* | B15 - B6  |  B5 - B4      | B3 - B2     |       B1    |     B0    | */
    /* --------------------------------------------------------------------- */
    /* | Reserved  |  GTSKA Replay | PTSKA Replay| No Pairwise | Pre - Auth| */
    /* |           |    Counter    |   Counter   |             |           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* 设置RSN Capabilities 值，包括Pre_Auth, no_pairwise,  */
    /* Replay counters (PTKSA and GTKSA)                    */
    /* WPA 不需要填写RSN Capabilities 字段                  */
    rsn_cap = (mac_rsn_cap_stru *)(buffer + index);
    (osal_void)memset_s(rsn_cap, sizeof(mac_rsn_cap_stru), 0, sizeof(mac_rsn_cap_stru));
    index += MAC_RSN_CAP_LEN;

    rsn_cap->mfpr = mac_mib_get_dot11_rsnamfpr(hmac_vap);
    rsn_cap->mfpc = mac_mib_get_dot11_rsnamfpc(hmac_vap);
    rsn_cap->pre_auth = mac_mib_get_pre_auth_actived(hmac_vap);
    rsn_cap->no_pairwise = 0;
    rsn_cap->ptska_relay_counter = mac_mib_get_rsnacfg_ptksareplaycounters(hmac_vap);
    rsn_cap->gtska_relay_counter = mac_mib_get_rsnacfg_gtksareplaycounters(hmac_vap);

    /* 设置 PMKID 信息 */
    if (pmkid) {
        buffer[index++] = 0x01;
        buffer[index++] = 0x00;
        (osal_void)memcpy_s(&(buffer[index]), WLAN_PMKID_LEN, pmkid, WLAN_PMKID_LEN);
        index += WLAN_PMKID_LEN;
    }

    if ((mac_mib_get_dot11_rsnamfpc((hmac_vap_stru *)hmac_vap) == OSAL_TRUE && (group_mgmt_suit != 0))) {
        /* 如果已经填过pmkid信息，不需要再填，否则需要填写一个空的PMKID */
        if (pmkid == OSAL_NULL) {
            buffer[index++] = 0x00;
            buffer[index++] = 0x00;
        }
        (osal_void)memcpy_s(&buffer[index], 4, (osal_u8 *)(&group_mgmt_suit), 4); /* 拷贝长度4字节 */
        index += 4; /* 偏移4字节 */
    }

    /* 设置RSN element的长度 */
    buffer[1] = index - MAC_IE_HDR_LEN;

    *ie_len = index;

    if (g_mac_frame_rom_cb.set_rsn_ie_cb != OSAL_NULL) {
        g_mac_frame_rom_cb.set_rsn_ie_cb(hmac_vap, pmkid, buffer, ie_len);
    }
}

/*****************************************************************************
 功能描述 : AP 填充rsn信息
*****************************************************************************/
osal_void hmac_set_wpa_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 index;
    osal_u8 pair_suites_num;
    osal_u8 akm_suites_num;
    osal_u8 loop;
    osal_u32 group_suit;
    osal_u32 pcip[WLAN_PAIRWISE_CIPHER_SUITES] = {0};
    osal_u32 akm[WLAN_AUTHENTICATION_SUITES] = {0};

    if (mac_mib_get_rsnaactivated(hmac_vap) != OSAL_TRUE || (hmac_vap->cap_flag.wpa != OSAL_TRUE)) {
        *ie_len = 0;
        return;
    }

    group_suit = mac_mib_get_wpa_group_suite(hmac_vap);
    pair_suites_num = mac_mib_get_wpa_pair_suites(hmac_vap, pcip, sizeof(pcip));
    akm_suites_num = mac_mib_get_wpa_akm_suites(hmac_vap, akm, sizeof(akm));
    if ((pair_suites_num == 0) || (akm_suites_num == 0)) {
        *ie_len = 0;
        return;
    }

    /*************************************************************************/
    /*                  RSN Element Format              */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 2       |      4             |     2           */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite | Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       | 4-m                        |     2          | 4-n             */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |    2           |    2      |16 -s     |         4                 | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    index = MAC_IE_HDR_LEN;

    /* 设置RSN ie的EID */
    buffer[0] = MAC_EID_WPA;

    (osal_void)memcpy_s(buffer + index, MAC_OUI_LEN, g_wpa_oui_etc, MAC_OUI_LEN);

    index += MAC_OUI_LEN;

    buffer[index++] = MAC_OUITYPE_WPA; /* 填充WPA 的OUI 类型 */

    /* 设置version字段 */
    buffer[index++] = MAC_RSN_IE_VERSION;
    buffer[index++] = 0;

    /* 设置Group Cipher Suite */
    /*************************************************************************/
    /*                  Group Cipher Suite                                   */
    /* --------------------------------------------------------------------- */
    /*                  | OUI | Suite type |                                 */
    /* --------------------------------------------------------------------- */
    /*          Octets: |  3  |     1      |                                 */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    (osal_void)memcpy_s(&buffer[index], MAC_DATA_HEADER_LEN, (osal_u8 *)(&group_suit), MAC_DATA_HEADER_LEN);
    index += 4; /* 4 */

    /* 设置成对加密套件 */
    buffer[index++] = pair_suites_num;
    buffer[index++] = 0;

    for (loop = 0; loop < pair_suites_num; loop++) {
        (osal_void)memcpy_s(&buffer[index], MAC_DATA_HEADER_LEN, (osal_u8 *)(&pcip[loop]), MAC_DATA_HEADER_LEN);
        index += 4; /* 4 */
    }

    /* 设置认证套件数 */
    buffer[index++] = akm_suites_num;
    buffer[index++] = 0;

    for (loop = 0; loop < akm_suites_num; loop++) {
        (osal_void)memcpy_s(&buffer[index], MAC_DATA_HEADER_LEN, (osal_u8 *)(&akm[loop]), MAC_DATA_HEADER_LEN);
        index += 4; /* 4 */
    }

    /* 设置wpa element的长度 */
    buffer[1] = index - MAC_IE_HDR_LEN;

    *ie_len = index;

    if (g_mac_frame_rom_cb.set_wpa_ie_cb != OSAL_NULL) {
        g_mac_frame_rom_cb.set_wpa_ie_cb(hmac_vap, buffer, ie_len);
    }
}

/*****************************************************************************
 功能描述 : 设置qos info字段
*****************************************************************************/
static osal_void mac_set_qos_info_field(hmac_vap_stru *hmac_vap, osal_u8 *buffer)
{
    mac_qos_info_stru *qos_info = (mac_qos_info_stru *)buffer;
    osal_void *fhook = OSAL_NULL;

    /* QoS Information field  (AP MODE)            */
    /* ------------------------------------------- */
    /* | B0:B3               | B4:B6    | B7     | */
    /* ------------------------------------------- */
    /* | Parameter Set Count | Reserved | U-APSD | */

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        qos_info->params_count = hmac_vap->uc_wmm_params_update_count;
        qos_info->uapsd = hmac_vap->cap_flag.uapsd;
        qos_info->bit_resv = 0;
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        if (hmac_vap->cap_flag.wur_enable == OSAL_TRUE) {
            /* WUR BSS参数保存 */
            hmac_vap->wur_bss->wur_bss_cur.wmm_params_update_count = qos_info->params_count;
        }
#endif
    }

    /* QoS Information field  (STA MODE)           */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* | B0              | B1              | B2              | B3              | B4      |B5   B6      | B7     | */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* |AC_VO U-APSD Flag|AC_VI U-APSD Flag|AC_BK U-APSD Flag|AC_BE U-APSD Flag|Reserved |Max SP Length|Reserved| */
    /* ---------------------------------------------------------------------------------------------------------- */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        buffer[0] = 0;
        buffer[0] |= 0x0;
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_STA_UAPSD_SET_QOS_INFO);
        if (fhook != OSAL_NULL) {
            ((hmac_uapsd_sta_set_qos_info_cb)fhook)(hmac_vap, buffer);
        }
    }
}

/*****************************************************************************
 功能描述 : 设置一个ac的参数
*****************************************************************************/
static osal_void mac_set_wmm_ac_params(hmac_vap_stru *hmac_vap, osal_u8 *buffer, wlan_wme_ac_type_enum_uint8 ac)
{
    mac_wmm_ac_params_stru *ac_params = (mac_wmm_ac_params_stru *)buffer;

    /* AC_** Parameter Record field               */
    /* ------------------------------------------ */
    /* | Byte 1    | Byte 2        | Byte 3:4   | */
    /* ------------------------------------------ */
    /* | ACI/AIFSN | ECWmin/ECWmax | TXOP Limit | */
    /* ------------------------------------------ */

    /* ACI/AIFSN Field                    */
    /* ---------------------------------- */
    /* | B0:B3 | B4  | B5:B6 | B7       | */
    /* ---------------------------------- */
    /* | AIFSN | ACM | ACI   | Reserved | */
    /* ---------------------------------- */
    /* AIFSN */
    ac_params->aifsn = (osal_u8)mac_mib_get_edca_table_aifsn(hmac_vap, ac);

    /* ACM */
    ac_params->acm = (osal_u8)mac_mib_get_edca_table_mandatory(hmac_vap, ac);

    /* ACI */
    ac_params->aci = (osal_u8)(mac_mib_get_edca_table_index(hmac_vap, ac) - 1);

    ac_params->bit_resv = 0;

    /* ECWmin/ECWmax Field */
    /* ------------------- */
    /* | B0:B3  | B4:B7  | */
    /* ------------------- */
    /* | ECWmin | ECWmax | */
    /* ------------------- */
    /* ECWmin */
    ac_params->ecwmin = (osal_u8)mac_mib_get_edca_table_cwmin(hmac_vap, ac);

    /* ECWmax */
    ac_params->ecwmax = (osal_u8)mac_mib_get_edca_table_cwmax(hmac_vap, ac);

    /* TXOP Limit. The value saved in MIB is in usec while the value to be   */
    /* set in this element should be in multiple of 32us                     */
    ac_params->txop = (osal_u16)(mac_mib_get_edca_table_txop_limit(hmac_vap, ac) >> 5); /* 右移5 bit */
}

/*****************************************************************************
 功能描述 : 设置wmm信息元素 is_qos:是否支持QOS.如果是BEACON/Probe Req/Probe Rsp/ASSOC Req帧,
            则取AP/STA自身的QOS能力;如果ASSOC RSP,则需要根据对方STA的QOS能力,来判断是否带WMM IE.
*****************************************************************************/
osal_void hmac_set_wmm_params_ie_etc(osal_void *vap, osal_u8 *buffer, oal_bool_enum_uint8 is_qos, osal_u8 *ie_len)
{
    osal_u8 index;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    if (is_qos == OSAL_FALSE) {
        *ie_len = 0;

        return;
    }

    /* WMM Parameter Element Format                                          */
    /* --------------------------------------------------------------------- */
    /* | 3Byte | 1        | 1           | 1             | 1        | 1     | */
    /* --------------------------------------------------------------------- */
    /* | OUI   | OUI Type | OUI Subtype | Version field | QoS Info | Resvd | */
    /* --------------------------------------------------------------------- */
    /* | 4              | 4              | 4              | 4              | */
    /* --------------------------------------------------------------------- */
    /* | AC_BE ParamRec | AC_BK ParamRec | AC_VI ParamRec | AC_VO ParamRec | */
    /* --------------------------------------------------------------------- */
    buffer[0] = MAC_EID_WMM;
    buffer[1] = MAC_WMM_PARAM_LEN;

    index = MAC_IE_HDR_LEN;

    /* OUI */
    (osal_void)memcpy_s(&buffer[index], MAC_OUI_LEN, g_wmm_oui_etc, MAC_OUI_LEN);
    index += MAC_OUI_LEN;

    /* OUI Type */
    buffer[index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    buffer[index++] = MAC_OUISUBTYPE_WMM_PARAM;

    /* Version field */
    buffer[index++] = MAC_OUI_WMM_VERSION;

    /* QoS Information Field */
    mac_set_qos_info_field(hmac_vap, &buffer[index]);
    index += MAC_QOS_INFO_LEN;

    /* Reserved */
    buffer[index++] = 0;

    /* Set the AC_BE, AC_BK, AC_VI, AC_VO Parameter Record fields */
    mac_set_wmm_ac_params(hmac_vap, &buffer[index], WLAN_WME_AC_BE);
    index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(hmac_vap, &buffer[index], WLAN_WME_AC_BK);
    index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(hmac_vap, &buffer[index], WLAN_WME_AC_VI);
    index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(hmac_vap, &buffer[index], WLAN_WME_AC_VO);

    *ie_len = MAC_IE_HDR_LEN + MAC_WMM_PARAM_LEN;
}

/*****************************************************************************
 功能描述 : 填充extended supported rates信息
*****************************************************************************/
osal_void hmac_set_exsup_rates_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_rateset_stru *rates_set;
    osal_u8 nrates, rs_nrates, idx;

    /* 5G不需要设置extended supported rates字段 */
    if (hmac_vap->channel.band == WLAN_BAND_5G) {
        *ie_len = 0;
        return;
    }

    rates_set = &(hmac_vap->curr_sup_rates.rate);

    /* STA全信道扫描时根据频段设置supported rates */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA &&
        (hmac_vap->protocol == WLAN_VHT_MODE || hmac_vap->protocol == WLAN_HE_MODE)) {
        rates_set = &(hmac_vap->sta_sup_rates_ie[hmac_vap->channel.band].rate);
    }

    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/
    rs_nrates = rates_set->rs_nrates;
#ifdef _PRE_WLAN_FEATURE_WPA3
    if (hmac_vap->sae_pwe == NL80211_SAE_PWE_HASH_TO_ELEMENT) {
        rs_nrates++;
    }
#endif
    if (rs_nrates <= MAC_MAX_SUPRATES) {
        *ie_len = 0;

        return;
    }

    buffer[0] = MAC_EID_XRATES;
    nrates = rates_set->rs_nrates - MAC_MAX_SUPRATES;
    buffer[1] = nrates;

    for (idx = 0; idx < nrates; idx++) {
        buffer[MAC_IE_HDR_LEN + idx] = rates_set->rs_rates[idx + MAC_MAX_SUPRATES].mac_rate;
    }
#ifdef _PRE_WLAN_FEATURE_WPA3
    if (hmac_vap->sae_pwe == NL80211_SAE_PWE_HASH_TO_ELEMENT) {
        /* support_rate 速率个数大于等于8，且SAE_PWE = HASH_TO_ELEMENT, SUPPORT_RATE IE中增加SAE_H2E_ONLY字段 */
        nrates++;
        buffer[1] = nrates;
        buffer[MAC_IE_HDR_LEN + idx] = 0x80 | BSS_MEMBERSHIP_SELECTOR_SAE_H2E_ONLY;
    }
#endif
    *ie_len = MAC_IE_HDR_LEN + nrates;
}

/*****************************************************************************
 功能描述 : 填写ht capabilities info域
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_ht_capinfo_field(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_frame_ht_cap_stru *ht_capinfo = (mac_frame_ht_cap_stru *)buffer;

    /*********************** HT Capabilities Info field*************************
    ----------------------------------------------------------------------------
     |-------------------------------------------------------------------|
     | LDPC   | Supp    | SM    | Green- | Short  | Short  |  Tx  |  Rx  |
     | Coding | Channel | Power | field  | GI for | GI for | STBC | STBC |
     | Cap    | Wth Set | Save  |        | 20 MHz | 40 MHz |      |      |
     |-------------------------------------------------------------------|
     |   B0   |    B1   |B2   B3|   B4   |   B5   |    B6  |  B7  |B8  B9|
     |-------------------------------------------------------------------|

     |-------------------------------------------------------------------|
     |    HT     |  Max   | DSS/CCK | Reserved | 40 MHz     | L-SIG TXOP |
     |  Delayed  | AMSDU  | Mode in |          | Intolerant | Protection |
     | Block-Ack | Length | 40MHz   |          |            | Support    |
     |-------------------------------------------------------------------|
     |    B10    |   B11  |   B12   |   B13    |    B14     |    B15     |
     |-------------------------------------------------------------------|
    ***************************************************************************/
    /* 初始清0 */
    buffer[0] = 0;
    buffer[1] = 0;
#ifdef _PRE_WLAN_FEATURE_LDPC_TX_ONLY
    ht_capinfo->ldpc_coding_cap = 0;
#else
    ht_capinfo->ldpc_coding_cap = mac_mib_get_ldpc_coding_option_implemented(hmac_vap);
#endif
    /* 设置所支持的信道宽度集"，0:仅20MHz运行; 1:20MHz与40MHz运行 */
    ht_capinfo->supported_channel_width = (mac_vap_get_cap_bw(hmac_vap) > WLAN_BAND_WIDTH_20M) ? OSAL_TRUE : OSAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_SMPS
    ht_capinfo->sm_power_save = hmac_calc_smps_field(mac_mib_get_smps(hmac_vap));
#else
    ht_capinfo->sm_power_save = MAC_SMPS_MIMO_MODE;
#endif

    ht_capinfo->ht_green_field = mac_mib_get_ht_greenfield_option_implemented(hmac_vap);

    ht_capinfo->short_gi_20mhz = mac_mib_get_short_gi_option_in_twenty_implemented(hmac_vap);
    ht_capinfo->short_gi_40mhz = mac_mib_get_short_gi_option_in_forty_implemented(hmac_vap);

    ht_capinfo->tx_stbc = mac_mib_get_tx_stbc_option_implemented(hmac_vap);
    ht_capinfo->rx_stbc = mac_mib_get_rx_stbc_option_implemented(hmac_vap);

    ht_capinfo->ht_delayed_block_ack = mac_mib_get_dot11_delayed_block_ack_option_implemented(hmac_vap);

    ht_capinfo->max_amsdu_length = mac_mib_get_max_amsdu_length(hmac_vap);

    /* 是否在具有40MHz能力，而运行于20/40MHz模式的BSS上使用DSSS/CCK */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        if ((hmac_vap->protocol == WLAN_LEGACY_11B_MODE) || (hmac_vap->protocol == WLAN_MIXED_ONE_11G_MODE) ||
            (hmac_vap->protocol == WLAN_MIXED_TWO_11G_MODE) ||
            (hmac_vap->protocol == WLAN_HT_MODE && (mac_vap_get_cap_bw(hmac_vap) > WLAN_BAND_WIDTH_20M))) {
            ht_capinfo->dsss_cck_mode_40mhz = hmac_vap->cap_flag.dsss_cck_mode_40mhz;
        } else {
            ht_capinfo->dsss_cck_mode_40mhz = 0;
        }
    } else {
        ht_capinfo->dsss_cck_mode_40mhz = 0;
    }

    /* 设置"40MHz不容许"，只在2.4GHz下有效 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        ht_capinfo->forty_mhz_intolerant = mac_mib_get_forty_mhz_intolerant(hmac_vap);
    } else {
        /* 5G 40MHz不容忍设置为0 */
        ht_capinfo->forty_mhz_intolerant = OSAL_FALSE;
    }

    ht_capinfo->lsig_txop_protection = mac_mib_get_lsig_txop_protection(hmac_vap);
}

/*****************************************************************************
 功能描述 : 填充ht a-mpdu parameters域信息
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_ampdu_params_field(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_ampdu_params_stru *ampdu_params = (mac_ampdu_params_stru *)buffer;

    /******************** AMPDU Parameters Field ******************************
    |-----------------------------------------------------------------------|
    | Maximum AMPDU Length Exponent | Minimum MPDU Start Spacing | Reserved |
    |-----------------------------------------------------------------------|
    | B0                         B1 | B2                      B4 | B5     B7|
    |-----------------------------------------------------------------------|
    **************************************************************************/
    /* 初始清0 */
    buffer[0] = 0;

    ampdu_params->max_ampdu_len_exponent = (osal_u8)mac_mib_get_max_ampdu_len_exponent(hmac_vap);
    ampdu_params->min_mpdu_start_spacing = (osal_u8)mac_mib_get_min_mpdu_start_spacing(hmac_vap);
}

/*****************************************************************************
 功能描述 : 填充supported mcs set域信息
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_sup_mcs_set_field(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_sup_mcs_set_stru *sup_mcs_set = (mac_sup_mcs_set_stru *)buffer;

    /************************* Supported MCS Set Field **********************
    |-------------------------------------------------------------------|
    | Rx MCS Bitmask | Reserved | Rx Highest    | Reserved |  Tx MCS    |
    |                |          | Supp Data Rate|          |Set Defined |
    |-------------------------------------------------------------------|
    | B0         B76 | B77  B79 | B80       B89 | B90  B95 |    B96     |
    |-------------------------------------------------------------------|
    | Tx Rx MCS Set  | Tx Max Number     |   Tx Unequal     | Reserved  |
    |  Not Equal     | Spat Stream Supp  | Modulation Supp  |           |
    |-------------------------------------------------------------------|
    |      B97       | B98           B99 |       B100       | B101 B127 |
    |-------------------------------------------------------------------|
    *************************************************************************/

    /* 初始清零 */
    (osal_void)memset_s(buffer, sizeof(mac_sup_mcs_set_stru), 0, sizeof(mac_sup_mcs_set_stru));
    (osal_void)memcpy_s(sup_mcs_set->rx_mcs, WLAN_HT_MCS_BITMASK_LEN,
        mac_mib_get_supported_mcs_rx_value(hmac_vap), WLAN_HT_MCS_BITMASK_LEN);

    sup_mcs_set->rx_highest_rate = (osal_u16)mac_mib_get_highest_supported_data_rate(hmac_vap);

    if (mac_mib_get_tx_mcs_set_defined(hmac_vap) == OSAL_TRUE) {
        sup_mcs_set->tx_mcs_set_def = 1;

        if (mac_mib_get_tx_rx_mcs_set_not_equal(hmac_vap) == OSAL_TRUE) {
            sup_mcs_set->tx_rx_not_equal = 1;

            sup_mcs_set->tx_max_stream = mac_mib_get_tx_maximum_num_spatial_streams_supported(hmac_vap);
            if (mac_mib_get_tx_unequal_modulation_supported(hmac_vap) == OSAL_TRUE) {
                sup_mcs_set->tx_unequal_modu = 1;
            }
        }
    }

    /* reserve位清0 */
    sup_mcs_set->resv1 = 0;
    sup_mcs_set->resv2 = 0;
}

/*****************************************************************************
 函 数 名  : hmac_set_ht_extcap_field
 功能描述  : 填充ht extended capabilities field信息
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_ht_extcap_field(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_ext_cap_stru *ext_cap = (mac_ext_cap_stru *)buffer;

    /***************** HT Extended Capabilities Field **********************
      |-----------------------------------------------------------------|
      | PCO | PCO Trans | Reserved | MCS  |  +HTC   |  RD    | Reserved |
      |     |   Time    |          | Fdbk | Support | Resp   |          |
      |-----------------------------------------------------------------|
      | B0  | B1     B2 | B3    B7 | B8 B9|   B10   |  B11   | B12  B15 |
      |-----------------------------------------------------------------|
    ***********************************************************************/
    /* 初始清0 */
    buffer[0] = 0;
    buffer[1] = 0;

    if (mac_mib_get_pco_option_implemented(hmac_vap) == OSAL_TRUE) {
        ext_cap->pco = 1;
        ext_cap->pco_trans_time = (osal_u16)mac_mib_get_transition_time(hmac_vap);
    }

    ext_cap->mcs_fdbk = mac_mib_get_mcs_fdbk(hmac_vap);
    ext_cap->htc_sup = mac_mib_get_htc_sup(hmac_vap);
    ext_cap->rd_resp = mac_mib_get_rd_rsp(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_set_txbf_cap_field
 功能描述  : 填充tansmit beamforming capbilities域信息
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_txbf_cap_field(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_txbf_cap_stru *txbf_cap = (mac_txbf_cap_stru *)buffer;

    /*************** Transmit Beamforming Capability Field *********************
     |-------------------------------------------------------------------------|
     |   Implicit | Rx Stagg | Tx Stagg  | Rx NDP   | Tx NDP   | Implicit      |
     |   TxBF Rx  | Sounding | Sounding  | Capable  | Capable  | TxBF          |
     |   Capable  | Capable  | Capable   |          |          | Capable       |
     |-------------------------------------------------------------------------|
     |      B0    |     B1   |    B2     |   B3     |   B4     |    B5         |
     |-------------------------------------------------------------------------|
     |              | Explicit | Explicit Non- | Explicit      | Explicit      |
     |  Calibration | CSI TxBF | Compr Steering| Compr steering| TxBF CSI      |
     |              | Capable  | Cap.          | Cap.          | Feedback      |
     |-------------------------------------------------------------------------|
     |  B6       B7 |   B8     |       B9      |       B10     | B11  B12      |
     |-------------------------------------------------------------------------|
     | Explicit Non- | Explicit | Minimal  | CSI Num of | Non-Compr Steering   |
     | Compr BF      | Compr BF | Grouping | Beamformer | Num of Beamformer    |
     | Fdbk Cap.     | Fdbk Cap.|          | Ants Supp  | Ants Supp            |
     |-------------------------------------------------------------------------|
     | B13       B14 | B15  B16 | B17  B18 | B19    B20 | B21        B22       |
     |-------------------------------------------------------------------------|
     | Compr Steering    | CSI Max Num of     |   Channel     |                |
     | Num of Beamformer | Rows Beamformer    | Estimation    | Reserved       |
     | Ants Supp         | Supported          | Capability    |                |
     |-------------------------------------------------------------------------|
     | B23           B24 | B25            B26 | B27       B28 | B29  B31       |
     |-------------------------------------------------------------------------|
    ***************************************************************************/
    /* 初始清零 */
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0; /* buffer第2字节 */
    buffer[3] = 0; /* buffer第3字节 */

    /* 指示STA是否可以接收staggered sounding帧 */
    txbf_cap->rx_stagg_sounding = mac_mib_get_receive_stagger_sounding_option_implemented(hmac_vap);

    /* 指示STA是否可以发送staggered sounding帧. */
    txbf_cap->tx_stagg_sounding = mac_mib_get_trans_stagger_sounding_option_implemented(hmac_vap);

    txbf_cap->rx_ndp = mac_mib_get_receive_ndp_option_implemented(hmac_vap);

    txbf_cap->tx_ndp = mac_mib_get_transmit_ndp_option_implemented(hmac_vap);

    txbf_cap->implicit_txbf = mac_mib_get_implicit_trans_bf_option_implemented(hmac_vap);

    txbf_cap->calibration = mac_mib_get_calibration_option_implemented(hmac_vap);

    txbf_cap->explicit_csi_txbf = mac_mib_get_explicit_csi_trans_bf_option_implemented(hmac_vap);

    txbf_cap->explicit_noncompr_steering = mac_mib_get_explicit_non_compress_bf_matrix_option_implemented(hmac_vap);

    /* Indicates if this STA can apply transmit beamforming using compressed */
    /* beamforming feedback matrix explicit feedback in its tranmission.     */
    /*************************************************************************/
    /*************************************************************************/
    /* No MIB exists, not clear what needs to be set    B10                  */
    /*************************************************************************/
    /*************************************************************************/

    /* Indicates if this receiver can return CSI explicit feedback */
    txbf_cap->explicit_txbf_csi_fdbk = mac_mib_get_explicit_trans_bf_csi_feedback_option_implemented(hmac_vap);

    /* Indicates if this receiver can return non-compressed beamforming      */
    /* feedback matrix explicit feedback.                                    */
    txbf_cap->explicit_noncompr_bf_fdbk =
        mac_mib_get_explicit_non_compressed_bf_feedback_option_implemented(hmac_vap);

    /* Indicates if this STA can apply transmit beamforming using explicit   */
    /* compressed beamforming feedback matrix.                               */
    txbf_cap->explicit_compr_bf_fdbk = mac_mib_get_explicit_compressed_bf_feedback_option_implemented(hmac_vap);

    /* Indicates the minimal grouping used for explicit feedback reports */
    /*************************************************************************/
    /*************************************************************************/
    /*  No MIB exists, not clear what needs to be set       B17              */
    /*************************************************************************/
    /*************************************************************************/
    /* Indicates the maximum number of beamformer antennas the beamformee    */
    /* can support when CSI feedback is required.                            */
    txbf_cap->csi_num_bf_antssup = mac_mib_get_num_bf_csi_support_antenna(hmac_vap);

    /* Indicates the maximum number of beamformer antennas the beamformee    */
    /* can support when non-compressed beamforming feedback matrix is        */
    /* required                                                              */
    txbf_cap->noncompr_steering_num_bf_antssup =
        mac_mib_get_num_non_compressed_bf_matrix_support_antenna(hmac_vap);

    /* Indicates the maximum number of beamformer antennas the beamformee   */
    /* can support when compressed beamforming feedback matrix is required  */
    txbf_cap->compr_steering_num_bf_antssup = mac_mib_get_num_compressed_bf_matrix_support_antenna(hmac_vap) - 1;

    /* Indicates the maximum number of rows of CSI explicit feedback from    */
    /* beamformee that the beamformer can support when CSI feedback is       */
    /* required                                                              */
    /*************************************************************************/
    /*************************************************************************/
    /*  No MIB exists, not clear what needs to be set     B25                */
    /*************************************************************************/
    /*************************************************************************/

    /* Indicates maximum number of space time streams (columns of the MIMO   */
    /* channel matrix) for which channel dimensions can be simultaneously    */
    /* estimated. When staggered sounding is supported this limit applies    */
    /* independently to both the data portion and to the extension portion   */
    /* of the long training fields.                                          */
    /*************************************************************************/
    /*************************************************************************/
    /*      No MIB exists, not clear what needs to be set          B27       */
    /*************************************************************************/
    /*************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TXBF
    txbf_cap->explicit_compr_steering = 0;
    txbf_cap->chan_estimation = 0;
    txbf_cap->minimal_grouping = 0;
    txbf_cap->csi_maxnum_rows_bf_sup = 0;
    txbf_cap->implicit_txbf_rx = 0;
#endif
}

/*****************************************************************************
 功能描述 : 填充asel(antenna selection) capabilities域信息
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_asel_cap_field(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_asel_cap_stru *asel_cap = (mac_asel_cap_stru *)buffer;

    /************** Antenna Selection Capability Field *************************
     |-------------------------------------------------------------------|
     |  Antenna  | Explicit CSI  | Antenna Indices | Explicit | Antenna  |
     | Selection | Fdbk based TX | Fdbk based TX   | CSI Fdbk | Indices  |
     |  Capable  | ASEL Capable  | ASEL Capable    | Capable  | Fdbk Cap.|
     |-------------------------------------------------------------------|
     |    B0     |     B1        |      B2         |    B3    |    B4    |
     |-------------------------------------------------------------------|

     |------------------------------------|
     |  RX ASEL |   Transmit   |          |
     |  Capable |   Sounding   | Reserved |
     |          | PPDU Capable |          |
     |------------------------------------|
     |    B5    |     B6       |    B7    |
     |------------------------------------|
    ***************************************************************************/
    /* 初始清0 */
    buffer[0] = 0;

    /* 指示STA是否支持天线选择 */
    asel_cap->asel = mac_mib_get_antenna_selection_option_implemented(hmac_vap);

    /* 指示STA是否具有基于显示CSI(信道状态信息)反馈的发射天线选择能力 */
    asel_cap->explicit_sci_fdbk_tx_asel = mac_mib_get_trans_explicit_csi_feedback_as_option_implemented(hmac_vap);

    /* 指示STA是否具有基于天线指数反馈的发射天线选择能力 */
    asel_cap->antenna_indices_fdbk_tx_asel = mac_mib_get_trans_indices_feedback_as_option_implemented(hmac_vap);

    /* 指示STA在天线选择的支持下是否能够计算CSI(信道状态信息)并提供CSI反馈 */
    asel_cap->explicit_csi_fdbk = mac_mib_get_explicit_csi_feedback_as_option_implemented(hmac_vap);

    /* Indicates whether or not this STA can conduct antenna indices */
    /* selection computation and feedback the results in support of */
    /* Antenna Selection. */
    asel_cap->antenna_indices_fdbk = mac_mib_get_trans_explicit_csi_feedback_as_option_implemented(hmac_vap);

    /* 指示STA是否具有接收天线选择能力 */
    asel_cap->rx_asel = mac_mib_get_receive_antenna_selection_option_implemented(hmac_vap);

    /* 指示STA是否能够在每一次请求中都可以为天线选择序列发送探测PPDU */
    asel_cap->trans_sounding_ppdu = mac_mib_get_trans_sounding_ppdu_option_implemented(hmac_vap);
}

/*****************************************************************************
 功能描述 : 设置Timeout_Interval信息元素
*****************************************************************************/
osal_void mac_set_timeout_interval_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len, osal_u32 type,
    osal_u32 timeout)
{
#ifdef _PRE_WLAN_FEATURE_PMF
    mac_timeout_interval_type_enum tie_type;

    tie_type = (mac_timeout_interval_type_enum)type;
    *ie_len = 0;

    /* 判断是否需要设置timeout_interval IE */
    if (tie_type >= MAC_TIE_BUTT) {
        return;
    }

    /* Timeout Interval Parameter Element Format
    -----------------------------------------------------------------------
    |ElementID | Length | Timeout Interval Type| Timeout Interval Value  |
    -----------------------------------------------------------------------
    |1         | 1      | 1                    |  4                      |
    -----------------------------------------------------------------------
    */
    buffer[0] = MAC_EID_TIMEOUT_INTERVAL;
    buffer[1] = MAC_TIMEOUT_INTERVAL_INFO_LEN;
    buffer[2] = tie_type; /* buffer第2字节 */

    /* 设置Timeout Interval Value */
    buffer[3] = (osal_u8)(timeout & 0x000000FF);           /* buffer第3字节 */
    buffer[4] = (osal_u8)((timeout & 0x0000FF00) >> 8);    /* buffer第4字节. 右移8 bit */
    buffer[5] = (osal_u8)((timeout & 0x00FF0000) >> 16);   /* buffer第5字节. 右移16 bit */
    buffer[6] = (osal_u8)((timeout & 0xFF000000) >> 24);   /* buffer第6字节. 右移24 bit */

    *ie_len = MAC_IE_HDR_LEN + MAC_TIMEOUT_INTERVAL_INFO_LEN;
#else
    *ie_len = 0;
#endif
    unref_param(vap);
    return;
}

/*****************************************************************************
 功能描述 : 填充ht capabilities信息
 *****************************************************************************/
osal_void hmac_set_ht_capabilities_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *buffer = buf;

    if ((mac_mib_get_high_throughput_option_implemented(hmac_vap) != OSAL_TRUE) ||
        (((mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) || (mac_is_tkip_only(hmac_vap) == OSAL_TRUE)) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP))) {
        *ie_len = 0;
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |HT Capa. Info |A-MPDU Parameters |Supported MCS Set|
    -------------------------------------------------------------------------
    |1   |1      |2             |1                 |16               |
    -------------------------------------------------------------------------
    |HT Extended Cap. |Transmit Beamforming Cap. |ASEL Cap.          |
    -------------------------------------------------------------------------
    |2                |4                         |1                  |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *buffer = MAC_EID_HT_CAP;
    *(buffer + 1) = MAC_HT_CAP_LEN;

    buffer += MAC_IE_HDR_LEN;

    /* 填充ht capabilities information域信息 */
    hmac_set_ht_capinfo_field(vap, buffer);
    buffer += MAC_HT_CAPINFO_LEN;

    /* 填充A-MPDU parameters域信息 */
    hmac_set_ampdu_params_field(vap, buffer);
    buffer += MAC_HT_AMPDU_PARAMS_LEN;

    /* 填充supported MCS set域信息 */
    hmac_set_sup_mcs_set_field(vap, buffer);
    buffer += MAC_HT_SUP_MCS_SET_LEN;

    /* 填充ht extended capabilities域信息 */
    hmac_set_ht_extcap_field(vap, buffer);
    buffer += MAC_HT_EXT_CAP_LEN;

    /* 填充 transmit beamforming capabilities域信息 */
    hmac_set_txbf_cap_field(vap, buffer);
    buffer += MAC_HT_TXBF_CAP_LEN;

    /* 填充asel(antenna selection) capabilities域信息 */
    hmac_set_asel_cap_field(vap, buffer);

    *ie_len = MAC_IE_HDR_LEN + MAC_HT_CAP_LEN;

    if (g_mac_frame_rom_cb.set_ht_cap_ie_cb != OSAL_NULL) {
        g_mac_frame_rom_cb.set_ht_cap_ie_cb(hmac_vap, buffer, ie_len);
    }
}

/*****************************************************************************
 功能描述 : 填充ht operation信息
*****************************************************************************/
osal_void hmac_set_ht_opern_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_ht_opern_stru *ht_opern = OSAL_NULL;
    osal_u8 obss_non_ht = 0;
    wlan_channel_bandwidth_enum_uint8 bandwidth;

    if ((mac_mib_get_high_throughput_option_implemented(hmac_vap) != OSAL_TRUE) ||
        (((mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) || (mac_is_tkip_only(hmac_vap) == OSAL_TRUE)) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP))) {
        *ie_len = 0;
        return;
    }

    /***************************************************************************
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

    *buffer = MAC_EID_HT_OPERATION;

    *(buffer + 1) = MAC_HT_OPERN_LEN;
    ht_opern = (mac_ht_opern_stru *)(buffer + MAC_IE_HDR_LEN);
    (osal_void)memset_s((osal_void *)ht_opern, sizeof(mac_ht_opern_stru), 0, sizeof(mac_ht_opern_stru));

    /* 主信道编号 */
    ht_opern->primary_channel = hmac_vap->channel.chan_number;

    /* 设置"次信道偏移量" */
    bandwidth = hmac_vap->channel.en_bandwidth;
    if ((bandwidth == WLAN_BAND_WIDTH_40PLUS) || (bandwidth == WLAN_BAND_WIDTH_80PLUSPLUS) ||
        (bandwidth == WLAN_BAND_WIDTH_80PLUSMINUS)) {
        ht_opern->secondary_chan_offset = MAC_SCA;
    } else if ((bandwidth == WLAN_BAND_WIDTH_40MINUS) || (bandwidth == WLAN_BAND_WIDTH_80MINUSPLUS) ||
        (bandwidth == WLAN_BAND_WIDTH_80MINUSMINUS)) {
        ht_opern->secondary_chan_offset = MAC_SCB;
    } else {
        ht_opern->secondary_chan_offset = MAC_SCN;
    }

    /* 设置"STA信道宽度"，当BSS运行信道宽度 >= 40MHz时，需要将此field设置为1 */
    ht_opern->sta_chan_width = (hmac_vap->channel.en_bandwidth > WLAN_BAND_WIDTH_20M) ? 1 : 0;

    /* 指示基本服务集里是否允许使用减小的帧间距 */
    ht_opern->rifs_mode = mac_mib_get_rifs_mode(hmac_vap);

    /* B4-B7保留 */
    ht_opern->resv1 = 0;

    /* 指示ht传输的保护要求 */
    ht_opern->ht_protection = mac_mib_get_ht_protection(hmac_vap);

    /* Non-GF STAs */
    ht_opern->nongf_sta_present = mac_mib_get_non_gf_entities_present(hmac_vap);

    /* B3 resv */
    ht_opern->resv2 = 0;

    /* B4  obss_nonht_sta_present */
    if ((hmac_vap->protection.obss_non_ht_present != 0) || (hmac_vap->protection.sta_non_ht_num != 0)) {
        obss_non_ht = 1;
    }
    ht_opern->obss_nonht_sta_present = obss_non_ht;

    /* B5-B15 保留 */
    ht_opern->resv3 = 0;
    ht_opern->resv4 = 0;

    /* B0-B5 保留 */
    ht_opern->resv5 = 0;

    /* B6  dual_beacon */
    ht_opern->dual_beacon = 0;

    /* Dual CTS protection */
    ht_opern->dual_cts_protection = mac_mib_get_dual_cts_protection(hmac_vap);

    /* secondary_beacon: Set to 0 in a primary beacon */
    ht_opern->secondary_beacon = 0;

    /* BSS support L-SIG TXOP Protection */
    ht_opern->lsig_txop_protection_full_support = mac_mib_get_lsig_txop_full_protection_activated(hmac_vap);

    /* PCO active */
    ht_opern->pco_active = mac_mib_get_pco_activated(hmac_vap);

    /* PCO phase */
    ht_opern->pco_phase = 0;

    /* B12-B15  保留 */
    ht_opern->resv6 = 0;

    /* Basic MCS Set: set all bit zero,Indicates the MCS values that are supported by all HT STAs in the BSS. */
    (osal_void)memset_s(ht_opern->basic_mcs_set, MAC_HT_BASIC_MCS_SET_LEN, 0, MAC_HT_BASIC_MCS_SET_LEN);

    *ie_len = MAC_IE_HDR_LEN + MAC_HT_OPERN_LEN;

    if (g_mac_frame_rom_cb.set_ht_opern_ie_cb != OSAL_NULL) {
        g_mac_frame_rom_cb.set_ht_opern_ie_cb(hmac_vap, buffer, ie_len);
    }

#ifdef _PRE_WLAN_FEATURE_WUR_TX
    hmac_wur_ht_oper_ie_save(hmac_vap, ht_opern);
#endif
}

/*****************************************************************************
 功能描述 : 填充overlapping bss scan parameters
*****************************************************************************/
osal_void hmac_set_obss_scan_params_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_obss_scan_params_stru *obss_scan = OSAL_NULL;
    osal_u32 ret;

    if (mac_mib_get_high_throughput_option_implemented(hmac_vap) != OSAL_TRUE) {
        *ie_len = 0;
        return;
    }

    if ((hmac_vap->channel.band != WLAN_BAND_2G) ||
        (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) != OSAL_TRUE) ||
        (mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) != OSAL_TRUE)) {
        *ie_len = 0;
        return;
    }

    /***************************************************************************
     |ElementID |Length |OBSS    |OBSS   |BSS Channel   |OBSS Scan  |OBSS Scan   |
     |          |       |Scan    |Scan   |Width Trigger |Passive    |Active Total|
     |          |       |Passive |Active |Scan Interval |Total Per  |Per         |
     |          |       |Dwell   |Dwell  |              |Channel    |Channel     |
     ----------------------------------------------------------------------------
     |1         |1      |2       |2      |2             |2          |2           |
     ----------------------------------------------------------------------------
     |BSS Width   |OBSS Scan|
     |Channel     |Activity |
     |Transition  |Threshold|
     |Delay Factor|         |
     ------------------------
     |2           |2        |
    ***************************************************************************/
    buffer[0] = MAC_EID_OBSS_SCAN;
    buffer[1] = MAC_OBSS_SCAN_IE_LEN;

    obss_scan = (mac_obss_scan_params_stru *)(buffer + MAC_IE_HDR_LEN);

    ret = mac_mib_get_obss_scan_passive_dwell(hmac_vap);
    obss_scan->passive_dwell = (osal_u16)ret;

    ret = mac_mib_get_obss_scan_active_dwell(hmac_vap);
    obss_scan->active_dwell = (osal_u16)ret;

    ret = mac_mib_get_bss_width_trigger_scan_interval(hmac_vap);
    obss_scan->scan_interval = (osal_u16)ret;

    ret = mac_mib_get_obss_scan_passive_total_per_channel(hmac_vap);
    obss_scan->passive_total_per_chan = (osal_u16)ret;

    ret = mac_mib_get_obss_scan_active_total_per_channel(hmac_vap);
    obss_scan->active_total_per_chan = (osal_u16)ret;

    ret = mac_mib_get_bss_width_channel_transition_delay_factor(hmac_vap);
    obss_scan->transition_delay_factor = (osal_u16)ret;

    ret = mac_mib_get_obss_scan_activity_threshold(hmac_vap);
    obss_scan->scan_activity_thresh = (osal_u16)ret;

    *ie_len = MAC_IE_HDR_LEN + MAC_OBSS_SCAN_IE_LEN;
}

/*****************************************************************************
 功能描述 : 填充extended capabilities element信息
*****************************************************************************/
osal_void hmac_set_ext_capabilities_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_ext_cap_ie_stru ext_cap;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11V_SET_EXT_CAP_BSS_TRANSITION);

    if ((mac_mib_get_high_throughput_option_implemented(hmac_vap) != OSAL_TRUE) ||
        (((mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) || (mac_is_tkip_only(hmac_vap) == OSAL_TRUE)) &&
            is_ap(hmac_vap))) {
        *ie_len = 0;
        return;
    }

    /***************************************************************************
                         ----------------------------------
                         |Element ID |Length |Capabilities|
                         ----------------------------------
          Octets:        |1          |1      |n           |
                         ----------------------------------
    -------------------------------------------------------------------------------------------------------------------
    |  B0       | B1 | B2             | B3   | B4   |  B5  |  B6    |  B7   | ...|  B38    |   B39      |...|  B62
    ----------------------------------------------------------------------------
    |20/40 coex |resv|extended channel| resv | PSMP | resv | S-PSMP | Event |    |TDLS Pro-  TDLS Channel     Operating
                                                                                             Switching        mode
    |mgmt supp  |    |switching       |      |      |      |        |       | ...| hibited | Prohibited |...| notify
    -------------------------------------------------------------------------------------------------------------------
    ***************************************************************************/
    buffer[0] = MAC_EID_EXT_CAPS;

    /* 初始清零 */
    (osal_void)memset_s(&ext_cap, sizeof(mac_ext_cap_ie_stru), 0, sizeof(mac_ext_cap_ie_stru));

    /* 设置20/40 BSS Coexistence Management Support fieid */
    if ((mac_mib_get_2040_bss_coexistence_management_support(hmac_vap) == OSAL_TRUE) &&
        is_2g(hmac_vap) && (mac_mib_get_forty_mhz_operation_implemented(hmac_vap) == OSAL_TRUE)) {
        ext_cap.coexistence_mgmt_2040 = 1;
    }

    /* 设置TDLS prohibited */
    ext_cap.tdls_prhibited = hmac_vap->cap_flag.tdls_prohibited;

    /* 设置TDLS channel switch prohibited */
    ext_cap.tdls_channel_switch_prhibited = hmac_vap->cap_flag.tdls_channel_switch_prohibited;

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 如果是11ac 站点 设置OPMODE NOTIFY标志 */
    if ((mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_TRUE) &&
        (((mac_is_wep_enabled(hmac_vap) == OSAL_FALSE) && (mac_is_tkip_only(hmac_vap) == OSAL_FALSE)) ||
        !is_ap(hmac_vap))) {
        ext_cap.operating_mode_notification = mac_mib_get_operating_mode_notification_implemented(hmac_vap);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    ext_cap.proxyarp = ((hmac_vap->hmac_vap_proxyarp != OSAL_NULL) &&
        (hmac_vap->hmac_vap_proxyarp->is_proxyarp == OSAL_TRUE) && (is_ap(hmac_vap))) ? 1 : 0;
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
    /*  如果支持Hotspot2.0的Interwoking标志  */
    ext_cap.interworking = 1;
#endif

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 配置multi bssid能力位 */
    ext_cap.multiple_bssid = mac_mib_get_multi_bssid_implement(hmac_vap);
#endif

    if (fhook != OSAL_NULL) {
        ((hmac_11v_set_ext_cap_bss_transition_cb)fhook)(hmac_vap, &ext_cap);
    }

    (*ie_len) = MAC_XCAPS_EX_LEN;

    /* 添加ftm, twt等新增ext cap ie */
    hmac_set_ext_capabilities_ie_update_etc(hmac_vap, &ext_cap, ie_len);

    /* 根据ext cap ie实际长度拷贝ie到buffer */
    buffer[1] = *ie_len;
    (osal_void)memcpy_s(buffer + MAC_IE_HDR_LEN, *ie_len, &ext_cap, *ie_len);
    *ie_len += MAC_IE_HDR_LEN;
}

/*****************************************************************************
 函 数 名  : hmac_set_vht_capinfo_field_etc
 功能描述  : 设置vht capinfo域

*****************************************************************************/
OSAL_STATIC osal_void hmac_set_vht_capinfo_field_etc(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_vht_cap_info_stru *vht_capinfo = (mac_vht_cap_info_stru *)buffer;

    /*********************** VHT 能力信息域 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------|
     | Max    | Supp    | RX   | Short GI| Short  | Tx   |  Rx  |  SU        |
     | MPDU   | Channel | LDPC | for 80  | GI for | STBC | STBC | Beamformer |
     | Length | Wth Set |      |         | 160MHz |      |      | Capable    |
     |-----------------------------------------------------------------------|
     | B0 B1  | B2 B3   | B4   |   B5    |   B6   |  B7  |B8 B10|   B11      |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | SU         | Compressed   | Num of    | MU        | MU        | VHT   |
     | Beamformee | Steering num | Sounding  | Beamformer| Beamformee| TXOP  |
     | Capable    | of bf ant sup| Dimensions| Capable   | Capable   | PS    |
     |-----------------------------------------------------------------------|
     |    B12     | B13      B15 | B16    B18|   B19     |    B20    | B21   |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | +HTC   | Max AMPDU| VHT Link  | Rx ANT     | Tx ANT     |   Resv      |
     | VHT    | Length   | Adaptation| Pattern    | Pattern    |             |
     | Capable| Exponent | Capable   | Consistency| Consistency|             |
     |-----------------------------------------------------------------------|
     | B22    | B23  B25 | B26   B27 |   B28      |   B29      |  B30 B31    |
     |-----------------------------------------------------------------------|
    ***************************************************************************/
    vht_capinfo->bit_max_mpdu_length = mac_mib_get_maxmpdu_length(hmac_vap);

    /* 设置"所支持的信道宽度集"，0:neither 160 nor 80+80:; 1:160MHz; 2:160/80+80MHz */
    vht_capinfo->supported_channel_width = mac_mib_get_vht_channel_width_option_implemented(hmac_vap);
#ifdef _PRE_WLAN_FEATURE_LDPC_TX_ONLY
    vht_capinfo->rx_ldpc = 0;
#else
    vht_capinfo->rx_ldpc = mac_mib_get_vht_ldpc_coding_option_implemented(hmac_vap);
#endif
    vht_capinfo->short_gi_80mhz = mac_mib_get_vht_short_gi_option_in80_implemented(hmac_vap);
    vht_capinfo->short_gi_160mhz = OSAL_FALSE;
    vht_capinfo->tx_stbc = mac_mib_get_vht_tx_stbc_option_implemented(hmac_vap);
    vht_capinfo->rx_stbc = (mac_mib_get_vht_rx_stbc_option_implemented(hmac_vap) == OSAL_TRUE) ? 1 : 0;
    vht_capinfo->su_beamformer_cap = mac_mib_get_vht_su_bfer_option_implemented(hmac_vap);
    vht_capinfo->su_beamformee_cap = mac_mib_get_vht_su_bfee_option_implemented(hmac_vap);
    vht_capinfo->num_bf_ant_supported = (is_ap(hmac_vap)) ? 0 : VHT_BFEE_NTX_SUPP_STS_CAP - 1;
    /* sounding dim是bfer的能力 */
#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
    vht_capinfo->num_sounding_dim = mac_mib_get_vht_num_sounding_dimensions(hmac_vap);
#else
    vht_capinfo->num_sounding_dim = 0;
#endif

    vht_capinfo->mu_beamformer_cap = mac_mib_get_vht_mu_bfer_option_implemented(hmac_vap);
    vht_capinfo->mu_beamformee_cap = mac_mib_get_vht_mu_bfee_option_implemented(hmac_vap);
    vht_capinfo->vht_txop_ps = mac_mib_get_txopps(hmac_vap);
    vht_capinfo->htc_vht_capable = mac_mib_get_vht_ctrl_field_cap(hmac_vap);
    vht_capinfo->max_ampdu_len_exp = mac_mib_get_vht_max_rx_ampdu_factor(hmac_vap);

    vht_capinfo->vht_link_adaptation = 0;
    vht_capinfo->rx_ant_pattern = 0; /* 在该关联中不改变天线模式，设为1；改变则设为0 */
    vht_capinfo->tx_ant_pattern = 0; /* 在该关联中不改变天线模式，设为1；改变则设为0 */

    /* resv位清0 */
    vht_capinfo->bit_resv = 0;

    if (g_mac_frame_rom_cb.set_vht_capinfo_field_cb != OSAL_NULL) {
        g_mac_frame_rom_cb.set_vht_capinfo_field_cb(hmac_vap, buffer);
    }
}

/*****************************************************************************
 功能描述 : 设置vht supported mcs set field
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_vht_supported_mcsset_field_etc(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_vht_sup_mcs_set_stru *vht_mcsset = (mac_vht_sup_mcs_set_stru *)buffer;

    /*********************** VHT 支持的MCS集 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------|
     | Rx MCS Map | Rx Highest Supported | Resv    | Tx MCS Map  |
     |            | Long gi Data Rate    |         |             |
     |-----------------------------------------------------------------------|
     | B0     B15 | B16              B28 | B29 B31 | B32     B47 |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | Tx Highest Supported |  Resv   |
     | Long gi Data Rate    |         |
     |-----------------------------------------------------------------------|
     |  B48             B60 | B61 B63 |
     |-----------------------------------------------------------------------|
    ***************************************************************************/
    vht_mcsset->rx_mcs_map = mac_mib_get_vht_rx_mcs_map(hmac_vap);
    vht_mcsset->rx_highest_rate = mac_mib_get_us_rx_highest_rate(hmac_vap);
    vht_mcsset->tx_mcs_map = mac_mib_get_vht_tx_mcs_map(hmac_vap);
    vht_mcsset->tx_highest_rate = mac_mib_get_us_tx_highest_rate(hmac_vap);

    /* resv清0 */
    vht_mcsset->bit_resv = 0;
    vht_mcsset->resv2 = 0;
}

/*****************************************************************************
 功能描述 : 设置vht能力ie
*****************************************************************************/
osal_void hmac_set_vht_capabilities_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *buffer = buf;

    if ((hmac_vap == OSAL_NULL) || (buffer == OSAL_NULL) || (ie_len == OSAL_NULL)) {
        oam_error_log3(0, OAM_SF_ANY, "{hmac_set_vht_capabilities_ie_etc::param null, vap[%p],buffer[%p],ie_len[%p].}",
            (uintptr_t)hmac_vap, (uintptr_t)hmac_vap, (uintptr_t)ie_len);
        return;
    }

    if ((mac_mib_get_vht_option_implemented(hmac_vap) != OSAL_TRUE) ||
        (((mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) || (mac_is_tkip_only(hmac_vap) == OSAL_TRUE)) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP))
#ifdef _PRE_WLAN_FEATURE_11AC2G
        || ((hmac_vap->cap_flag.support_11ac2g == OSAL_FALSE) && (hmac_vap->channel.band == WLAN_BAND_2G))
#endif /* _PRE_WLAN_FEATURE_11AC2G */
       ) {
        *ie_len = 0;
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |VHT Capa. Info |VHT Supported MCS Set|
    -------------------------------------------------------------------------
    |1   |1      | 4             | 8                   |
    -------------------------------------------------------------------------
    ***************************************************************************/
    buffer[0] = MAC_EID_VHT_CAP;
    buffer[1] = MAC_VHT_CAP_IE_LEN;

    buffer += MAC_IE_HDR_LEN;

    hmac_set_vht_capinfo_field_etc(vap, buffer);

    buffer += MAC_VHT_CAP_INFO_FIELD_LEN;

    hmac_set_vht_supported_mcsset_field_etc(vap, buffer);

    *ie_len = MAC_IE_HDR_LEN + MAC_VHT_CAP_IE_LEN;

    if (g_mac_frame_rom_cb.set_vht_cap_ie_cb != OSAL_NULL) {
        g_mac_frame_rom_cb.set_vht_cap_ie_cb(hmac_vap, buffer, ie_len);
    }
}

/*****************************************************************************
 功能描述 : 设置vht操作信息
*****************************************************************************/
osal_void hmac_set_vht_opern_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_vht_opern_stru *vht_opern = OSAL_NULL;
    osal_u8 *buffer = buf;

    if ((mac_mib_get_vht_option_implemented(hmac_vap) != OSAL_TRUE) ||
        (((mac_is_wep_enabled(hmac_vap) == OSAL_TRUE) || (mac_is_tkip_only(hmac_vap) == OSAL_TRUE)) &&
        (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP))
#ifdef _PRE_WLAN_FEATURE_11AC2G
        || ((hmac_vap->cap_flag.support_11ac2g == OSAL_FALSE) && (hmac_vap->channel.band == WLAN_BAND_2G))
#endif /* _PRE_WLAN_FEATURE_11AC2G */
       ) {
        *ie_len = 0;
        return;
    }

    /***********************VHT Operation element*******************************
    -------------------------------------------------------------------------
            |EID |Length |VHT Opern Info |VHT Basic MCS Set|
    -------------------------------------------------------------------------
    Octes:  |1   |1      | 3             | 2               |
    -------------------------------------------------------------------------
    ***************************************************************************/
    buffer[0] = MAC_EID_VHT_OPERN;
    buffer[1] = MAC_VHT_INFO_IE_LEN;

    buffer += MAC_IE_HDR_LEN;

    /**********************VHT Opern Info***************************************
    -------------------------------------------------------------------------
            | Channel Width | Channel Center | Channel Center |
            |               | Freq Seg0      | Freq Seg1      |
    -------------------------------------------------------------------------
    Octes:  |       1       |       1        |       1        |
    -------------------------------------------------------------------------
    ***************************************************************************/
    vht_opern = (mac_vht_opern_stru *)buffer;

    /* channel_width的取值，0 -- 20/40M, 1 -- 80M, 2 -- 160M, 3 -- 80+80M */
    if ((hmac_vap->channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (hmac_vap->channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)) {
        vht_opern->channel_width = WLAN_MIB_VHT_OP_WIDTH_80;
    } else {
        vht_opern->channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
    }

    switch (hmac_vap->channel.en_bandwidth) {
        case WLAN_BAND_WIDTH_80PLUSPLUS:
            /***********************************************************************
            | 主20 | 从20 | 从40       |
                          |
                          |中心频率相对于主20偏6个信道
            ************************************************************************/
            vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number + 6; /* 偏+6个信道 */
            break;
        case WLAN_BAND_WIDTH_80PLUSMINUS: /* fall through */
        case WLAN_BAND_WIDTH_40MINUS:
            /***********************************************************************
            | WLAN_BAND_WIDTH_80PLUSMINUS
            | 从40        | 主20 | 从20 |
                          |
                          |中心频率相对于主20偏-2个信道
            | WLAN_BAND_WIDTH_40MINUS
            | 从40        | 主20 | 从20 |
                          |
                          |中心频率相对于主20偏-2个信道
            ************************************************************************/
            vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number - 2; /* 偏-2个信道 */
            break;
        case WLAN_BAND_WIDTH_80MINUSPLUS: /* fall through */
        case WLAN_BAND_WIDTH_40PLUS:
            /***********************************************************************
            | WLAN_BAND_WIDTH_80MINUSPLUS
            | 从20 | 主20 | 从40       |
                          |
                          |中心频率相对于主20偏2个信道
            | WLAN_BAND_WIDTH_40PLUS
            | 主20 | 从20 |
            |
            | 中心频率相对于主20偏+2个信道
            ************************************************************************/
            vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number + 2; /* 偏+2个信道 */
            break;
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            /***********************************************************************
            | 从40        | 从20 | 主20 |
                          |
                          |中心频率相对于主20偏-6个信道
            ************************************************************************/
            vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number - 6; /* 偏-6个信道 */
            break;
        case WLAN_BAND_WIDTH_20M:
            /* 中心频率就是主信道频率   */
            vht_opern->channel_center_freq_seg0 = hmac_vap->channel.chan_number;
            break;
        default:
            break;
    }

    vht_opern->channel_center_freq_seg1 = 0;
    vht_opern->basic_mcs_set = mac_mib_get_vht_rx_mcs_map(hmac_vap);

    *ie_len = MAC_IE_HDR_LEN + MAC_VHT_INFO_IE_LEN;

    if (g_mac_frame_rom_cb.set_vht_opern_ie_cb != OSAL_NULL) {
        g_mac_frame_rom_cb.set_vht_opern_ie_cb(hmac_vap, buffer, ie_len);
    }
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    hmac_wur_vht_oper_ie_save(hmac_vap, vht_opern);
#endif
}

/*****************************************************************************
 功能描述 : 获取beacon帧中的ssid
*****************************************************************************/
osal_u8 *hmac_get_ssid_etc(osal_u8 *beacon_body, osal_s32 frame_body_len, osal_u8 *ssid_len)
{
    const osal_u8 *ssid_ie = OSAL_NULL;
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TIM elm   | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |4-256     | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /* ssid的长度初始赋值为0 */
    *ssid_len = 0;

    /* 检测beacon帧或者probe rsp帧的长度的合法性 */
    if (frame_body_len <= offset) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_get_ssid_etc:: the length of beacon/probe rsp frame body is invalid.}");
        return OSAL_NULL;
    }

    /* 查找ssid的ie */
    ssid_ie = mac_find_ie_etc(MAC_EID_SSID, (beacon_body + offset), (osal_s32)(frame_body_len - offset));
    if ((ssid_ie != OSAL_NULL) && (ssid_ie[1] < WLAN_SSID_MAX_LEN)) {
        /* 获取ssid ie的长度 */
        *ssid_len = ssid_ie[1];

        return (osal_u8 *)(ssid_ie + MAC_IE_HDR_LEN);
    }

    oam_warning_log0(0, OAM_SF_ANY, "{hmac_get_ssid_etc:: ssid ie isn't found.}");
    return OSAL_NULL;
}

/*****************************************************************************
 功能描述 : 获取beacon帧中的ext_ie_body
*****************************************************************************/
osal_u8 *mac_get_ext_ie_body(osal_u8 *beacon_body, osal_s32 frame_body_len, osal_u8 *ext_ie_body_len, osal_u8 ext_eid)
{
    const osal_u8 *ext_ie_body_ie = OSAL_NULL;

    /***********************************************
    ------------------------------------------------
    |EID |Length |EID Extension |Extension ie body |
    ------------------------------------------------
    |1   |1      |1             |various           |
    ------------------------------------------------
    ***********************************************/
    osal_u16 offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /* Extension Element的长度初始赋值为0 */
    *ext_ie_body_len = 0;

    /* 检测beacon帧或者probe rsp帧的长度的合法性 */
    if (frame_body_len <= offset) {
        oam_warning_log0(0, OAM_SF_ANY,
                         "{mac_get_ext_ie_body_etc:: the length of beacon/probe rsp frame body is invalid.}");
        return OSAL_NULL;
    }

    /* 根据Element ID和Extension ID查找指定的ie */
    ext_ie_body_ie = hmac_find_ie_ext_ie(MAC_EID_HE, ext_eid,
        (beacon_body + offset), (osal_s32)(frame_body_len - offset));
    if (ext_ie_body_ie != OSAL_NULL) {
        /* 获取ext_ie_body ie的长度 */
        *ext_ie_body_len = ext_ie_body_ie[1];

        return (osal_u8 *)(ext_ie_body_ie + MAC_IE_EXT_HDR_LEN);
    }
    return OSAL_NULL;
}

/*****************************************************************************
 功能描述 : 从beacon帧中获得beacon period
*****************************************************************************/
osal_u16 hmac_get_beacon_period_etc(const osal_u8 *beacon_body)
{
    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TIM elm   | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |4-256     | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    return *((osal_u16 *)(beacon_body + MAC_TIME_STAMP_LEN));
}

/*****************************************************************************
 功能描述 : 获取dtim period值
*****************************************************************************/
osal_u8 hmac_get_dtim_period_etc(osal_u8 *frame_body, osal_u16 frame_body_len)
{
    osal_u8 *ie;
    osal_u16 offset;

    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    ie = mac_find_ie_etc(MAC_EID_TIM, frame_body + offset, frame_body_len - offset);
    if ((ie != OSAL_NULL) && (ie[1] >= MAC_MIN_TIM_LEN)) {
        return ie[3];   /* IE的第3字节 */
    }

    return 0;
}

/*****************************************************************************
 功能描述 : 获取dtim cnt值
*****************************************************************************/
osal_u8 hmac_get_dtim_cnt_etc(osal_u8 *frame_body, osal_u16 frame_body_len)
{
    osal_u8 *ie;

    osal_u16 offset;

    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    ie = mac_find_ie_etc(MAC_EID_TIM, frame_body + offset, frame_body_len - offset);
    if ((ie != OSAL_NULL) && (ie[1] >= MAC_MIN_TIM_LEN)) {
        return ie[2];   /* IE的第2字节 */
    }

    return 0;
}

/*****************************************************************************
 功能描述 : 判断是否是wmm ie
*****************************************************************************/
oal_bool_enum_uint8 hmac_is_wmm_ie_etc(const osal_u8 *ie)
{
    /* --------------------------------------------------------------------- */
    /* WMM Information/Parameter Element Format                              */
    /* --------------------------------------------------------------------- */
    /* | OUI | OUIType | OUISubtype | Version | QoSInfo | OUISubtype based | */
    /* --------------------------------------------------------------------- */
    /* |3    | 1       | 1          | 1       | 1       | ---------------- | */
    /* --------------------------------------------------------------------- */
    if ((ie[0] == MAC_EID_WMM) && (ie[2] == MAC_WMM_OUI_BYTE_ONE) &&                   /* IE的第2字节 */
        (ie[3] == MAC_WMM_OUI_BYTE_TWO) &&                                             /* IE的第3字节 */
        (ie[4] == MAC_WMM_OUI_BYTE_THREE) &&                                           /* IE的第4字节 */
        (ie[5] == MAC_OUITYPE_WMM) &&                                                  /* IE的第5字节 */
        ((ie[6] == MAC_OUISUBTYPE_WMM_INFO) || (ie[6] == MAC_OUISUBTYPE_WMM_PARAM)) && /* IE的第6字节 */
        (ie[7] == MAC_OUI_WMM_VERSION)) {                                              /* IE的第7字节 */
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 功能描述 : 从管理帧中获取wmm ie
*****************************************************************************/
osal_u8 *hmac_get_wmm_ie_etc(osal_u8 *beacon_body, osal_u16 frame_len)
{
    osal_u8 *wmmie = OSAL_NULL;

    wmmie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM, beacon_body, frame_len);
    if (wmmie == OSAL_NULL) {
        return OSAL_NULL;
    }

    return (hmac_is_wmm_ie_etc(wmmie) == OSAL_TRUE) ? wmmie : OSAL_NULL;
}

/*****************************************************************************
 功能描述 : 根据rsn_ie获取rsn能力
******************************************************************************/
osal_u16 hmac_get_rsn_capability_etc(const osal_u8 *rsn_ie)
{
    osal_u16 pairwise_count;
    osal_u16 akm_count;
    osal_u16 rsn_capability;
    osal_u16 index = 0;

    if (rsn_ie == OSAL_NULL) {
        return 0;
    }

    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*            |         4*m                |     2           |   4*n     */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite */
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    if (rsn_ie[1] < MAC_MIN_RSN_LEN) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid rsn ie len[%d].}", rsn_ie[1]);
        return 0;
    }

    index += 8; /* 8: 8 Bytes */
    pairwise_count = oal_make_word16(rsn_ie[index], rsn_ie[index + 1]);
    if (pairwise_count > MAC_PAIRWISE_CIPHER_SUITES_NUM) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid pairwise_count[%d].}", pairwise_count);
        return 0;
    }

    index += MAC_IE_HDR_LEN + 4 * (osal_u8)pairwise_count; /* 4: pairwise_count */

    akm_count = OAL_MAKE_WORD16(rsn_ie[index], rsn_ie[index + 1]);
    if (akm_count > WLAN_AUTHENTICATION_SUITES) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid akm_count[%d].}", akm_count);
        return 0;
    }

    index += MAC_IE_HDR_LEN + 4 * (osal_u8)akm_count; /* 4: pairwise_count */
    /* 不带rsn_capability直接返回0 */
    if (index == (rsn_ie[1] + MAC_IE_HDR_LEN)) {
        return 0;
    }

    rsn_capability = OAL_MAKE_WORD16(rsn_ie[index], rsn_ie[index + 1]);
    return rsn_capability;
}

/*****************************************************************************
 功能描述 : 设置power capability信息元素
*****************************************************************************/
osal_void mac_set_power_cap_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_regclass_info_stru *regclass_info;

    if (mac_mib_get_spectrum_management_required(hmac_vap) == OSAL_FALSE &&
        mac_mib_get_dot11_radio_measurement_activated(hmac_vap) == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_TPC,
            "mac_set_power_cap_ie_etc:dot11SpectrumManagementRequired and dot11RadioMeasurementActivated are FALSE!");
        *ie_len = 0;
        return;
    }

    /********************************************************************************************
            ------------------------------------------------------------------------------------
            |ElementID | Length | MinimumTransmitPowerCapability| MaximumTransmitPowerCapability|
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                             | 1                             |
            -------------------------------------------------------------------------------------

    *********************************************************************************************/

    *buffer = MAC_EID_PWRCAP;
    *(buffer + 1) = MAC_PWR_CAP_LEN;

    /* 成功获取管制域信息则根据国家码和TPC设置最大和最小发射功率，否则默认为0 */
    regclass_info = hmac_get_channel_num_rc_info_etc(hmac_vap->channel.band, hmac_vap->channel.chan_number);
    if (regclass_info != OSAL_NULL) {
        *(buffer + 2) =     /* buffer偏移2字节 */
            (osal_u8)((hmac_vap->channel.band == WLAN_BAND_2G) ? 4 : 3);   /* 4: 最小发射功率. 3: 最小发射功率 */
        *(buffer + 3) =     /* buffer偏移3字节 */
            (osal_u8)OAL_MIN(regclass_info->max_reg_tx_pwr, regclass_info->max_tx_pwr / 10); /* 10: 功率单位转换 */
    } else {
        *(buffer + 2) = 0;  /* buffer偏移2字节 */
        *(buffer + 3) = 0;  /* buffer偏移3字节 */
    }
    *ie_len = MAC_IE_HDR_LEN + MAC_PWR_CAP_LEN;
}

/*****************************************************************************
 功能描述 : 设置支持信道信息元素
*****************************************************************************/
osal_void hmac_set_supported_channel_ie_etc(osal_void *vap, osal_u8 *buf, osal_u8 *ie_len)
{
    osal_u8 channel_max_num;
    osal_u8 channel_idx;
    osal_u8 channel_ie_len = 0;
    osal_u8 *ie_len_buffer = 0;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 channel_idx_cnt = 0;
    osal_u8 *buffer = buf;

    if (ie_len == OSAL_NULL) {
        return;
    }

    if ((mac_mib_get_spectrum_management_required(hmac_vap) == OSAL_FALSE) ||
        (mac_mib_get_dot11_extended_channel_switch_activated(hmac_vap) == OSAL_TRUE)) {
        *ie_len = 0;
        return;
    }

    /********************************************************************************************
            长度不定，信道号与信道数成对出现
            ------------------------------------------------------------------------------------
            |ElementID | Length | Fisrt Channel Number| Number of Channels|
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                   | 1                 |
            -------------------------------------------------------------------------------------

    *********************************************************************************************/
    /* 根据支持的频段获取最大信道个数 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        channel_max_num = (osal_u8)MAC_CHANNEL_FREQ_2_BUTT;
    } else if (hmac_vap->channel.band == WLAN_BAND_5G) {
        channel_max_num = (osal_u8)MAC_CHANNEL_FREQ_5_BUTT;
    } else {
        *ie_len = 0;
        return;
    }

    *buffer = MAC_EID_SUPPCHAN;
    buffer++;
    ie_len_buffer = buffer;

    /* 填写信道信息 */
    for (channel_idx = 0; channel_idx < channel_max_num; channel_idx++) {
        /* 修改管制域结构体后，需要增加该是否支持信号的判断 */
        if (hmac_is_channel_idx_valid_etc(hmac_vap->channel.band, channel_idx) == OAL_SUCC) {
            channel_idx_cnt++;
            /* channel_idx_cnt为1的时候表示是第一个可用信道，需要写到Fisrt Channel Number */
            if (channel_idx_cnt == 1) {
                buffer++;

                hmac_get_channel_num_from_idx_etc(hmac_vap->channel.band, channel_idx, buffer);
            } else if ((channel_max_num - 1) == channel_idx) {
                /* 将Number of Channels写入帧体中 */
                buffer++;
                *buffer = channel_idx_cnt;

                channel_ie_len += MAC_IE_HDR_LEN;
            }
        } else {
            /* channel_idx_cnt不为0的时候表示之前有可用信道，需要将可用信道的长度写到帧体中 */
            if (channel_idx_cnt != 0) {
                /* 将Number of Channels写入帧体中 */
                buffer++;
                *buffer = channel_idx_cnt;

                channel_ie_len += MAC_IE_HDR_LEN;
            }
            /* 将Number of Channels统计清零 */
            channel_idx_cnt = 0;
        }
    }

    *ie_len_buffer = channel_ie_len;
    *ie_len = channel_ie_len + MAC_IE_HDR_LEN;
}

/*****************************************************************************
 功能描述 : 设置WMM info element
*****************************************************************************/
osal_void hmac_set_wmm_ie_sta_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    osal_u8 index;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    if ((buffer == OSAL_NULL) || (ie_len == OSAL_NULL)) {
        return;
    }

    /* WMM Information Element Format                                */
    /* ------------------------------------------------------------- */
    /* | 3     | 1        | 1           | 1             | 1        | */
    /* ------------------------------------------------------------- */
    /* | OUI   | OUI Type | OUI Subtype | Version field | QoS Info | */
    /* ------------------------------------------------------------- */

    /* 判断STA是否支持WMM */
    if (mac_mib_get_dot11_qos_option_implemented(hmac_vap) != OSAL_TRUE) {
        *ie_len = 0;
        return;
    }

    buffer[0] = MAC_EID_WMM;
    buffer[1] = MAC_WMM_INFO_LEN;

    index = MAC_IE_HDR_LEN;

    /* OUI */
    (osal_void)memcpy_s(&buffer[index], MAC_OUI_LEN, g_wmm_oui_etc, MAC_OUI_LEN);
    index += MAC_OUI_LEN;

    /* OUI Type */
    buffer[index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    buffer[index++] = MAC_OUISUBTYPE_WMM_INFO;

    /* Version field */
    buffer[index++] = MAC_OUI_WMM_VERSION;

    /* QoS Information Field */
    mac_set_qos_info_field(hmac_vap, &buffer[index]);
    index += MAC_QOS_INFO_LEN;

    /* Reserved */
    buffer[index++] = 0;

    *ie_len = MAC_IE_HDR_LEN + MAC_WMM_INFO_LEN;
}

#ifdef _PRE_WLAN_FEATURE_WMMAC
/*****************************************************************************
 功能描述 : 设置WMMAC TSPEC INFO信息
*****************************************************************************/
osal_void mac_set_tspec_info_field(osal_void *vap, mac_wmm_tspec_stru *addts_args, osal_u8 *buffer)
{
    mac_wmm_tspec_stru *tspec_info;
    /**************************************************************************************************/

    /* TSPEC字段:
              ----------------------------------------------------------------------------------------
              |TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
              ----------------------------------------------------------------------------------------
     Octets:  | 3     |  2              |   2         |4            |4            |
              ----------------------------------------------------------------------------------------
              | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
              ----------------------------------------------------------------------------------------
     Octets:  |4                | 4               | 4               |4             |  4             |
              ----------------------------------------------------------------------------------------
              |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
              ----------------------------------------------------------------------------------------
     Octets:  |4             |4         | 4         | 4          |  2                   |2          |
              ----------------------------------------------------------------------------------------

     TS info字段:
              ----------------------------------------------------------------------------------------
              |Reserved |TSID |Direction |1 |0 |Reserved |PSB |UP |Reserved |Reserved |Reserved |
              ----------------------------------------------------------------------------------------
       Bits:  |1        |4    |2         |  2  |1        |1   |3  |2        |1        |7        |
              ----------------------------------------------------------------------------------------
    ***************************************************************************************************/

    /* 初始化TSPEC结构内存信息 */
    memset_s(buffer, MAC_WMMAC_TSPEC_LEN, 0, MAC_WMMAC_TSPEC_LEN);

    tspec_info = (mac_wmm_tspec_stru *)(buffer); // TSPEC Body

    tspec_info->ts_info.tsid = addts_args->ts_info.tsid;
    tspec_info->ts_info.direction = addts_args->ts_info.direction;
    tspec_info->ts_info.acc_policy = 1; /* bit7 = 1, bit8 = 0 */
    tspec_info->ts_info.apsd = addts_args->ts_info.apsd;
    tspec_info->ts_info.user_prio = addts_args->ts_info.user_prio;

    tspec_info->norminal_msdu_size = addts_args->norminal_msdu_size;
    tspec_info->max_msdu_size = addts_args->max_msdu_size;
    tspec_info->min_data_rate = addts_args->min_data_rate;
    tspec_info->mean_data_rate = addts_args->mean_data_rate;
    tspec_info->peak_data_rate = addts_args->peak_data_rate;
    tspec_info->min_phy_rate = addts_args->min_phy_rate;
    tspec_info->surplus_bw = addts_args->surplus_bw;
}

/*****************************************************************************
 功能描述 : 设置WMMAC IE(TSPEC相关)信息
*****************************************************************************/
osal_u16 mac_set_wmmac_ie_sta_etc(osal_void *vap, osal_u8 *buffer, mac_wmm_tspec_stru *addts_args)
{
    osal_u8 index;

    /************************************************************************************/
    /*                                Set WMM TSPEC 信息:                               */
    /*       ---------------------------------------------------------------------------
             |ID | Length| OUI |OUI Type| OUI subtype| Version| TSPEC body|
             ---------------------------------------------------------------------------
    Octets:  |1  | 1     | 3   |1       | 1          | 1      | 55        |
             ---------------------------------------------------------------------------
    *************************************************************************************/

    buffer[0] = MAC_EID_WMM;
    buffer[1] = MAC_WMMAC_INFO_LEN;

    index = MAC_IE_HDR_LEN;

    /* OUI */
    (osal_void)memcpy_s(&buffer[index], MAC_OUI_LEN, g_wmm_oui_etc, MAC_OUI_LEN);
    index += MAC_OUI_LEN;

    /* OUI Type */
    buffer[index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    buffer[index++] = MAC_OUISUBTYPE_WMMAC_TSPEC;

    /* Version field */
    buffer[index++] = MAC_OUI_WMM_VERSION;

    /* wmmac tspec Field */
    mac_set_tspec_info_field(vap, addts_args, &buffer[index]);

    return (MAC_IE_HDR_LEN + MAC_WMMAC_INFO_LEN);
}

#endif // _PRE_WLAN_FEATURE_WMMAC

/*****************************************************************************
 功能描述 : 设置listen interval信息元素
*****************************************************************************/
osal_void hmac_set_listen_interval_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    unref_param(vap);
    buffer[0] = 0x0a;
    buffer[1] = 0x00;
    *ie_len = MAC_LIS_INTERVAL_IE_LEN;
}

/*****************************************************************************
 功能描述 : 设置状态码信息元素
*****************************************************************************/
osal_void hmac_set_status_code_ie_etc(osal_u8 *buffer, mac_status_code_enum_uint16 status_code)
{
    buffer[0] = (osal_u8)(status_code & 0x00FF);
    buffer[1] = (osal_u8)((status_code & 0xFF00) >> 8); /* 右移8 bit */
}

/*****************************************************************************
 功能描述 : 设置AID（扫描ID）信息元素
*****************************************************************************/
osal_void hmac_set_aid_ie_etc(osal_u8 *buffer, osal_u16 assoc_id)
{
    osal_u16 aid = assoc_id;

    /* The 2 MSB bits of Association ID is set to 1 as required by the standard. */
    aid |= 0xC000;
    buffer[0] = (osal_u8)(aid & 0x00FF);
    buffer[1] = (osal_u8)((aid & 0xFF00) >> 8); /* 右移8 bit */
}

/*****************************************************************************
 功能描述 : 获取BSS的类型
*****************************************************************************/
osal_u8 hmac_get_bss_type_etc(osal_u16 cap_info)
{
    mac_cap_info_stru *cap_info_tmp = (mac_cap_info_stru *)&cap_info;

    if (cap_info_tmp->ess != 0) {
        return (osal_u8)WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    }

    if (cap_info_tmp->ibss != 0) {
        return (osal_u8)WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT;
    }

    return (osal_u8)WLAN_MIB_DESIRED_BSSTYPE_ANY;
}

/*****************************************************************************
 功能描述 : 检查CAP INFO中privacy 是否加密
*****************************************************************************/
osal_u32 hmac_check_mac_privacy_etc(osal_u16 cap_info, osal_void *vap)
{
    hmac_vap_stru *hmac_vap;
    mac_cap_info_stru *cap_info_tmp = (mac_cap_info_stru *)&cap_info;

    if (vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)vap;

    if (mac_mib_get_privacyinvoked(hmac_vap) == OSAL_TRUE) {
        /* 该VAP有Privacy invoked但其他VAP没有 */
        if (cap_info_tmp->privacy == 0) {
            return (osal_u32)OSAL_FALSE;
        }
    }
    /* 考虑兼容性，当vap不支持加密时，不检查用户的能力 */

    return (osal_u32)OSAL_TRUE;
}

/*****************************************************************************
 功能描述 : 填充用户态下发的信息元素到管理帧中
*****************************************************************************/
osal_void hmac_add_app_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u16 *ie_len, app_ie_type_uint8 type)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *app_ie = hmac_vap->app_ie[type].ie;
    osal_u32 app_ie_len = hmac_vap->app_ie[type].ie_len;

    if (app_ie_len == 0) {
        *ie_len = 0;
        return;
    } else {
        (osal_void)memcpy_s(buffer, app_ie_len, app_ie, app_ie_len);
        *ie_len = (osal_u16)app_ie_len;
    }

    return;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
 功能描述 : 设置operating mode field
*****************************************************************************/
osal_void hmac_set_opmode_field_etc(osal_void *vap, osal_u8 *buffer, wlan_nss_enum_uint8 nss)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_opmode_notify_stru *opmode_notify = (mac_opmode_notify_stru *)buffer;
    wlan_bw_cap_enum_uint8 cp_bw = WLAN_BW_CAP_BUTT;

    /**************************************************************/
    /* ---------------------------------------------------------- */
    /* |B0-B1        |B2-B3                  |B4-B6 |B7         | */
    /* ---------------------------------------------------------- */
    /* |Channel Width|Dynamic Extended NSS BW|Rx Nss|Rx Nss Type| */
    /* ---------------------------------------------------------- */
    /**************************************************************/
    /* rx_nss_type目前写定0即可，后续出现三流以上，但是txbf流小于三流时，才需要发送1类型 再扩展 */
    (osal_void)memset_s(opmode_notify, sizeof(mac_opmode_notify_stru), 0, sizeof(mac_opmode_notify_stru));

    hmac_vap_get_bandwidth_cap_etc(hmac_vap, &cp_bw);

    opmode_notify->channel_width = cp_bw;

    /* 要切换到的流，采用vap下的nss能力的话，需要提前置vap的nss能力，接口不灵活，增加入参，提高场景可扩展性 */
    opmode_notify->rx_nss = nss;
    opmode_notify->rx_nss_type = 0;
}

/*****************************************************************************
 功能描述 : 设置operating mode notificatin ie
*****************************************************************************/
osal_void hmac_set_opmode_notify_ie_etc(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;

    /********************************************
            -------------------------------------
            |ElementID | Length | Operating Mode|
            -------------------------------------
    Octets: |1         | 1      | 1             |
            -------------------------------------

    ********************************************/

    /* 11n模式携带此ie,与ac88有兼容性问题 */
    if ((mac_mib_get_vht_option_implemented(hmac_vap) == OSAL_FALSE) ||
        (mac_mib_get_operating_mode_notification_implemented(hmac_vap) == OSAL_FALSE)) {
        *ie_len = 0;
        return;
    }

    buffer[0] = MAC_EID_OPMODE_NOTIFY;
    buffer[1] = MAC_OPMODE_NOTIFY_LEN;

    hmac_set_opmode_field_etc((osal_void *)vap, (buffer + MAC_IE_HDR_LEN), hmac_vap->vap_rx_nss);

    *ie_len = MAC_IE_HDR_LEN + MAC_OPMODE_NOTIFY_LEN;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
 功能描述 : 从RSN ie中获取pmf能力信息
*****************************************************************************/
wlan_pmf_cap_status_uint8 hmac_get_pmf_cap_etc(osal_u8 *ie, osal_u32 ie_len)
{
    osal_u8 *rsn_ie;
    osal_u16 rsn_cap;

    if (osal_unlikely(ie == OSAL_NULL)) {
        return MAC_PMF_DISABLED;
    }

    /* 查找RSN信息元素,如果没有RSN信息元素,则按照不支持处理 */
    rsn_ie = mac_find_ie_etc(MAC_EID_RSN, ie, (osal_s32)(ie_len));
    if (rsn_ie == OSAL_NULL) {
        return MAC_PMF_DISABLED;
    }

    /* 根据RSN信息元素, 判断RSN能力是否匹配 */
    rsn_cap = hmac_get_rsn_capability_etc(rsn_ie);
    if (((rsn_cap & BIT6) != 0) && ((rsn_cap & BIT7) != 0)) {
        return MAC_PMF_REQUIRED;
    }

    if ((rsn_cap & BIT7) != 0) {
        return MAC_PMF_ENABLED;
    }
    return MAC_PMF_DISABLED;
}
#endif

/*****************************************************************************
 功能描述 : 封装 20/40 共存管理帧
*****************************************************************************/
osal_u16 hmac_encap_2040_coext_mgmt_etc(osal_void *vap, oal_netbuf_stru *buffer, osal_u8 coext_info,
    osal_u32 chan_report)
{
    osal_u8 *mac_header = oal_netbuf_header(buffer);
    osal_u8 *payload_addr = oal_netbuf_data_offset(buffer, MAC_80211_FRAME_LEN);
    osal_u8 chan_idx;
    osal_u16 ie_len_idx;
    osal_u16 index = 0;
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    wlan_channel_band_enum_uint8 band = hmac_vap->channel.band;
    osal_u8 max_num_chan = mac_get_num_supp_channel(band);
    osal_u8 channel_num;

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
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID) */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, hmac_vap->bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /*************************************************************************************/
    /*                 20/40 BSS Coexistence Management frame - Frame Body               */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |20/40 BSS Coex IE| 20/40 BSS Intolerant Chan Report IE| */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |3                |Variable                            | */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/

    payload_addr[index++] = MAC_ACTION_CATEGORY_PUBLIC; /* Category */
    payload_addr[index++] = MAC_PUB_COEXT_MGMT;         /* Public Action */

    /* 封装20/40 BSS Coexistence element */
    payload_addr[index++] = MAC_EID_2040_COEXT; /* Element ID */
    payload_addr[index++] = MAC_2040_COEX_LEN;  /* Length */
    payload_addr[index++] = coext_info;         /* 20/40 BSS Coexistence Information field */

    /* 封装20/40 BSS Intolerant Channel Report element */
    /* 只有当STA检测到Trigger Event A时，才包含Operating Class，参见802.11n 10.15.12 */
    payload_addr[index++] = MAC_EID_2040_INTOLCHREPORT; /* Element ID */
    ie_len_idx = index;
    payload_addr[index++] = MAC_2040_INTOLCHREPORT_LEN_MIN; /* Length */
    payload_addr[index++] = 0;                              /* Operating Class */
    if (chan_report > 0) {
        oam_warning_log2(0, OAM_SF_ASSOC, "vap_id[%d] {hmac_encap_2040_coext_mgmt_etc::Channel List = 0x%x.}",
            hmac_vap->vap_id, chan_report);
        /* Channel List */
        for (chan_idx = 0; chan_idx < max_num_chan; chan_idx++) {
            if (((chan_report >> chan_idx) & BIT0) != 0) {
                hmac_get_channel_num_from_idx_etc(band, chan_idx, &channel_num);
                payload_addr[index++] = channel_num;
                payload_addr[ie_len_idx]++;
            }
        }
    }

    return (osal_u16)(index + MAC_80211_FRAME_LEN);
}

#ifdef _PRE_WLAN_FEATURE_WUR_TX
/*****************************************************************************
 函 数 名  : mac_set_wur_supported_bands_field
 功能描述  : 填写wur Capabilities 的 Supported Bands域
*****************************************************************************/
osal_void mac_set_wur_supported_bands_field(osal_void *vap, osal_u8 *buffer)
{
    mac_wur_cap_supported_bands_stru *wur_supported_bands = (mac_wur_cap_supported_bands_stru *)buffer;
    (osal_void)memset_s((osal_u8 *)wur_supported_bands, sizeof(mac_wur_cap_supported_bands_stru),
        0, sizeof(mac_wur_cap_supported_bands_stru));
    /************************
    -------------------------
    |2.4G |5G      |reserved |
    -------------------------
    |B0   |B1      |B2-B7    |
    -------------------------
    ************************/
    /* WUR AP reserved */
    return;
}

/*****************************************************************************
 函 数 名  : mac_set_wur_cap_information_field
 功能描述  : 填写wur Capabilities 的 WUR Capabilities Information域
*****************************************************************************/
osal_void mac_set_wur_cap_information_field(osal_void *vap, osal_u8 *buffer)
{
    mac_wur_cap_information_stru *wur_cap_information = (mac_wur_cap_information_stru *)buffer;
    (osal_void)memset_s((osal_u8 *)wur_cap_information, sizeof(mac_wur_cap_information_stru),
        0, sizeof(mac_wur_cap_information_stru));
    /***************************************************************************
    -------------------------------------------------------------------------------
    | Transition | VL WUR        | WUR Group IDs | reserved | 20MHZ WUR Basic PPDU |
    | Delay      | Frame Support | Support       |          | with HDR Support     |
    -------------------------------------------------------------------------------
    |B0-B7       |B8             |B9-B10         |B11       | B12                  |
    -------------------------------------------------------------------------------
    -------------------------------------------------------------------------------
    | WUR FDAM | WUR Short Wake-up | reserved                                      |
    | Support  | Frame Support     |                                               |
    -------------------------------------------------------------------------------
    |B13       |B14                |B15                                            |
    -------------------------------------------------------------------------------
    ***************************************************************************/
    wur_cap_information->transition_delay = 0; /* WUR AP reserved */
    wur_cap_information->vl_wur_frame_support = 0; /* not support */
    wur_cap_information->wur_group_ids_support = 1; /* support */
    wur_cap_information->wur_basic_ppdu_hdr_support_20mhz = 0; /* WUR AP reserved */
    wur_cap_information->wur_fdma_support = 0; /* not support */
    wur_cap_information->wur_short_wakeup_frame_support = 1; /* support */
}

/*****************************************************************************
 函 数 名  : mac_set_wur_capabilities_ie
 功能描述  : 填写wur Capabilities element
*****************************************************************************/
osal_void mac_set_wur_capabilities_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *ie_length = OSAL_NULL;

    *ie_len = 0;

    if (mac_mib_get_dot11WUROptionImplemented(hmac_vap) != OSAL_TRUE) {
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |EID Extension|Supported Bands |WUR Capabilities Information|
    -------------------------------------------------------------------------
    |1   |1      |1            |1               |2                           |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *buffer = MAC_EID_WUR;
    ie_length = buffer + 1;

    buffer += MAC_IE_HDR_LEN;
    *ie_len += MAC_IE_HDR_LEN;

    *buffer = MAC_EID_EXT_WUR_CAP;
    buffer += 1;

    *ie_len += 1;

    /* 填充Supported Bands域信息 */
    mac_set_wur_supported_bands_field(vap, buffer);
    buffer += MAC_WUR_CAP_SUPPORT_BANDS_LEN;
    *ie_len += MAC_WUR_CAP_SUPPORT_BANDS_LEN;

    /* 填充WUR Capabilities Information域信息 */
    mac_set_wur_cap_information_field(vap, buffer);
    buffer += MAC_WUR_CAP_INFORMATION_LEN;
    *ie_len += MAC_WUR_CAP_INFORMATION_LEN;

    *ie_length = *ie_len - MAC_IE_HDR_LEN;
}

/*****************************************************************************
 函 数 名  : mac_set_wur_oper_parameters_field
 功能描述  : 填写wur Operation 的 WUR Operation Parameters域
*****************************************************************************/
osal_void mac_set_wur_oper_parameters_field(osal_void *vap, osal_u8 *buffer)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    mac_wur_oper_param_stru *wur_oper_param = (mac_wur_oper_param_stru *)buffer;
    (osal_void)memset_s((osal_u8 *)wur_oper_param, sizeof(mac_wur_oper_param_stru),
        0, sizeof(mac_wur_oper_param_stru));
    /***************************************************************************
    -------------------------------------------------------------------------------
    | Minimum Wake-up | Duty Cycle   | WUR Operating | WUR     | WUR Beacon Period |
    | Duration        | Period Units | Class         | Channel |                   |
    -------------------------------------------------------------------------------
    |B0-B7            |B8-B23        |B24-B31        |B32-B39  | B40-B55           |
    -------------------------------------------------------------------------------
    -------------------------------------------------------------------------------
    | Offset Of | Counter | Common PN | Reserved                                   |
    | TWBTT     |         |           |                                            |
    -------------------------------------------------------------------------------
    |B56-B71    |B72-B75  |B76        |B77-B79                                     |
    -------------------------------------------------------------------------------
    ***************************************************************************/
    wur_oper_param->min_wakeup_duration = hmac_vap->wur_basic_param.min_duration; /* unit : 256us */
    wur_oper_param->duty_cycle_period_units = hmac_vap->wur_basic_param.cycle_period_unit; /* unit : 4us */
    wur_oper_param->wur_operating_class = hmac_vap->wur_basic_param.operating_class; /* country有关 */
    wur_oper_param->wur_channel = hmac_vap->channel.chan_number; /* 802.11 主信道 */
    wur_oper_param->wur_beacon_period = hmac_vap->wur_basic_param.wur_beacon_period;
    wur_oper_param->offset_of_twbtt_l = (hmac_vap->wur_basic_param.offset_twbtt & 0xFF);
    wur_oper_param->offset_of_twbtt_h = ((hmac_vap->wur_basic_param.offset_twbtt & 0xFF00) >> 8); /* 右移8 bit */
    wur_oper_param->counter = hmac_vap->wur_bss->wur_bss_last.update_cnt; /* 关联响应不填 */
    wur_oper_param->common_pn = 0; /* not support frame protection */
}

/*****************************************************************************
 函 数 名  : mac_set_wur_operation_ie
 功能描述  : 填写wur Operation element
*****************************************************************************/
osal_void mac_set_wur_operation_ie(osal_void *vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)vap;
    osal_u8 *ie_length = OSAL_NULL;

    *ie_len = 0;

    if (mac_mib_get_dot11WUROptionImplemented(hmac_vap) != OSAL_TRUE) {
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |EID Extension|WUR Operation Parameters                      |
    -------------------------------------------------------------------------
    |1   |1      |1            |10                                            |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *buffer = MAC_EID_WUR;
    ie_length = buffer + 1;

    buffer += MAC_IE_HDR_LEN;
    *ie_len += MAC_IE_HDR_LEN;

    *buffer = MAC_EID_EXT_WUR_OPERATION;
    buffer += 1;

    *ie_len += 1;
    /* 填充WUR Operation Parameters域信息 */
    mac_set_wur_oper_parameters_field(vap, buffer);
    buffer += MAC_WUR_OPER_PARAM_LEN;
    *ie_len += MAC_WUR_OPER_PARAM_LEN;

    *ie_length = *ie_len - MAC_IE_HDR_LEN;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
