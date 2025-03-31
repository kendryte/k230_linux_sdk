/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: dmac beacon rom
 * Create: 2020-7-8
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_beacon.h"
#include "wlan_spec.h"
#include "wlan_types_common.h"
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hal_ext_if.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_vap_ext.h"
#include "oal_netbuf_data.h"
#include "hmac_main.h"
#include "hmac_device.h"
#include "hmac_chan_mgmt.h"
#include "hmac_mgmt_sta.h"
#include "hmac_mgmt_ap.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_scan.h"
#include "hmac_dfx.h"
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_device_ext.h"
#endif
#include "hmac_feature_dft.h"
#include "hmac_psm_sta.h"
#include "hmac_sta_pm.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_btcoex.h"
#include "hmac_btcoex_ps.h"
#include "hmac_vap.h"
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "hmac_smps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "hmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#include "mac_resource_ext.h"
#ifdef _PRE_WLAN_FEATURE_WUR_TX
#include "hmac_wur_ap.h"
#endif
#include "wlan_msg.h"
#include "common_dft.h"
#include "hmac_psm_ap.h"
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
#include "hmac_ant_switch.h"
#endif
#include "hmac_alg_notify.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_protection.h"
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "hmac_sniffer.h"
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_sta.h"
#include "hmac_obss_ap.h"
#endif
#include "frw_util_notifier.h"
#include "hmac_roam_if.h"
#include "hmac_csa_ap.h"
#include "hmac_csa_sta.h"
#include "hmac_sr_sta.h"
#include "hmac_feature_interface.h"
#include "hh503_phy_reg.h"
#include "hal_mac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_BEACON_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

#define BCN_TIMEOUT_ADJ_TH      5   /* 连续bcn接收超时上调bcn超时时间的次数 */
#define BCN_RX_ADJ_TH           2   /* 连续bcn接收成功下调bcn超时时间的次数 */
#define BCN_TIMEOUT_ADJ_STEP    2   /* bcn超时时间调整门限 */
#define BCN_TIMEOUT_VALUE_TH    30  /* bcn超时时间调整的最大门限单位ms */
#define PERIOD_100_MS           100 /* beacon的默认周期100ms */
#define ALG_ANTI_INF_BEACON_LOSS_NUM_TH 7       /* 连续X个beacon帧没有收到，重置agc门限 */
#define WLAN_LINKLOSS_MAX_TH    280 /* 默认最大Linkloss门限，超出则认为时间戳获取存在异常 */

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* linkloss各模式老化门限初始值，BT 200，DBAC 200 正常模式 100 */
osal_u8 g_linkloss_threshold[WLAN_LINKLOSS_MODE_BUTT] = {200, 200, 100};
/* linkloss模式开始发送probe request帧时间比例，默认为5，表示5/10即一半时间时开始发送 */
osal_u8 g_linkloss_send_probereq_ratio = 5;

/*****************************************************************************
  3 函数声明
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_process_erp_ie(osal_u8 *payload, osal_u16 msg_len, hmac_user_stru *hmac_user);
OSAL_STATIC oal_bool_enum_uint8 hmac_sta_edca_is_changed(hmac_vap_stru *hmac_vap, osal_u8 *frame_body,
    osal_u16 frame_len);
OSAL_STATIC osal_void hmac_protection_start_timer(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_u32 hmac_protection_set_ht_protection(hmac_vap_stru *hmac_vap,
    const hmac_user_stru *hmac_user, osal_u8 flag);
OSAL_STATIC osal_void hmac_sta_update_slottime(hmac_vap_stru* hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 *payload, osal_u16 msg_len);
OSAL_STATIC osal_void hmac_sta_up_update_protection_mode(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user);
OSAL_STATIC osal_void hmac_update_dtim_count_ap(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_void hmac_set_tim_ie(const hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len);
OSAL_STATIC osal_u32 hmac_protection_update_mode_ap(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_void hmac_init_dtim_count_ap(hmac_vap_stru *hmac_vap);
/*****************************************************************************
 函 数 名  : hmac_set_tx_chain_mask
 功能描述  : 设置发送天线的硬件寄存器参数
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_tx_chain_mask(const hmac_vap_stru *hmac_vap, osal_u32 *tx_chain_mask)
{
    /* 判断是否是mimo状态 */
    if (hmac_vap->hal_device->cfg_cap_info->phy2dscr_chain == WLAN_TX_CHAIN_DOUBLE) {
        /* 判断是否开启了双路轮流发送 */
        if (hmac_vap->beacon_tx_policy == HMAC_BEACON_TX_POLICY_SWITCH) {
            /* 第一个beacon帧由通道0发送 */
            *tx_chain_mask = WLAN_TX_CHAIN_ZERO;
        } else if (hmac_vap->beacon_tx_policy == HMAC_BEACON_TX_POLICY_DOUBLE) { /* 双通道发送 */
            if (hmac_vap->channel.band == WLAN_BAND_2G) {
                // 2g模式，11b只支持单通道发送，强制单通道
                *tx_chain_mask = hmac_vap->hal_device->cfg_cap_info->single_tx_chain;
            } else {
                *tx_chain_mask = WLAN_TX_CHAIN_DOUBLE;
            }
        } else { /* 单通道发送 */
            *tx_chain_mask = hmac_vap->hal_device->cfg_cap_info->single_tx_chain;
        }
    } else {
        *tx_chain_mask = hmac_vap->hal_device->cfg_cap_info->phy2dscr_chain;
    }
}

OSAL_STATIC osal_void hmac_vap_send_beacon_pkt_msg(osal_u8 vap_id, hal_beacon_tx_params_stru *beacon_tx_params)
{
    errno_t ret;
    frw_msg msg_info = {0};
    tx_beacon_data beacon_data;
    osal_s32 msg_ret;

    (osal_void)memset_s(&beacon_data, sizeof(beacon_data), 0, sizeof(beacon_data));
    ret = memcpy_s(beacon_data.beacon_data, WLAN_MGMT_NETBUF_SIZE,
                   (osal_void *)beacon_tx_params->pkt_ptr, beacon_tx_params->pkt_len);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_ANY, "{beacon_ddata copy failed!!ret = 0x%x}", ret);
    }
    ret = memcpy_s(&beacon_data.tx_param, sizeof(hal_tx_txop_alg_stru),
                   beacon_tx_params->tx_param, sizeof(hal_tx_txop_alg_stru));
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_ANY, "{tx_param copy failed!!ret = 0x%x}", ret);
    }

    beacon_data.pkt_len = beacon_tx_params->pkt_len;
    beacon_data.tx_chain_mask = beacon_tx_params->tx_chain_mask;

    frw_msg_init((osal_u8 *)&beacon_data, sizeof(tx_beacon_data), OSAL_NULL, 0, &msg_info);
    msg_ret = frw_send_msg_to_device(vap_id, WLAN_MSG_H2D_C_CFG_SEND_BEACON, &msg_info, OSAL_TRUE);
    if (msg_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hmac_vap_send_beacon_pkt_msg:send beacon to device fail[%d]", msg_ret);
    }
}
/*****************************************************************************
 函 数 名  : hmac_beacon_alloc
 功能描述  : 申请beacon帧内存
*****************************************************************************/
osal_u32 hmac_beacon_alloc(hmac_vap_stru *hmac_vap)
{
    hal_tx_txop_alg_stru *tx_param = OSAL_NULL;
    hal_beacon_tx_params_stru beacon_tx_params;
    osal_u16 beacon_len;
    wlan_channel_band_enum_uint8 band;
    osal_u8 vap_id = hmac_vap->vap_id;

    /*
     * BSS运行中的动态RESTART均不再需要申请beacon，场景包括：
     * -ACS初始信道
     * -AP/STA信道跟随
     * -proxysta信道跟随
     */
    if (hmac_vap->beacon_buffer != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_WIFI_BEACON,
            "vap_id[%d] {hmac_beacon_alloc::both of vap beacon buf have been alloc.}", vap_id);
        return OAL_SUCC;
    } else {
        /* 申请第一个beacon帧内存 */
        hmac_vap->beacon_buffer = (oal_netbuf_stru *)oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE,
            OAL_NETBUF_PRIORITY_HIGH);
        if (hmac_vap->beacon_buffer == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_WIFI_BEACON, "vap_id[%d] {hmac_beacon_alloc::pauc_beacon_buf[0] null.}", vap_id);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
    }
    /* 第一次初始化DTIM count,规范化使其从period -1开始 */
    hmac_init_dtim_count_ap(hmac_vap);

    /* 初始beacon组帧 */
    beacon_len = hmac_encap_beacon(hmac_vap, oal_netbuf_header(hmac_vap->beacon_buffer));

    /* 填写发送Beacon帧参数 */
    band = hmac_vap->channel.band;
    tx_param = &hmac_vap->tx_mgmt_bmcast[band];
    beacon_tx_params.pkt_ptr = (uintptr_t)(oal_netbuf_header(hmac_vap->beacon_buffer));
    beacon_tx_params.pkt_len = beacon_len;
    beacon_tx_params.tx_param = tx_param;

    /* 设置beacon period */
    hal_vap_set_machw_beacon_period(hmac_vap->hal_vap, (osal_u16)mac_mib_get_beacon_period(hmac_vap));

    hmac_set_tx_chain_mask(hmac_vap, &beacon_tx_params.tx_chain_mask);

    /* HAL发送Beacon帧接口 */
    hmac_vap_send_beacon_pkt_msg(vap_id, &beacon_tx_params);
    /* 开启tsf */
    hal_enable_tsf_tbtt(hmac_vap->hal_vap, OSAL_FALSE);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_encap_beacon
 功能描述  : beacon帧组帧
*****************************************************************************/
osal_u16 hmac_encap_beacon(hmac_vap_stru *hmac_vap, osal_u8 *beacon_buffer)
{
    osal_u8 *beacon_buf = beacon_buffer;
    osal_u8 ie_len;
    osal_u8 *bf_origin = beacon_buf;
    osal_u16 app_ie_len = 0;
    osal_u8 dsss_channel_num;
    mac_quiet_info quiet_info;
    osal_void *fhook = OSAL_NULL;
    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 设置Frame Control字段 */
    mac_hdr_set_frame_control(beacon_buf, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_BEACON);

    /* 设置duration为0，由硬件填 */
    mac_hdr_set_duration(beacon_buf, 0);

    /* 设置address1, DA是一个广播地址 */
    (osal_void)memcpy_s(beacon_buf + WLAN_HDR_ADDR1_OFFSET, WLAN_MAC_ADDR_LEN,
        BROADCAST_MACADDR, WLAN_MAC_ADDR_LEN);

    /* 设置address2，SA是mac地址 */
    (osal_void)memcpy_s(beacon_buf + WLAN_HDR_ADDR2_OFFSET, WLAN_MAC_ADDR_LEN,
        mac_mib_get_station_id(hmac_vap), WLAN_MAC_ADDR_LEN);

    /* 设置address3, BSSID */
    (osal_void)memcpy_s(beacon_buf + WLAN_HDR_ADDR3_OFFSET, WLAN_MAC_ADDR_LEN,
        hmac_vap->bssid, WLAN_MAC_ADDR_LEN);

    /* 设置分片序号为0，beacon帧没有sequence number */
    mac_hdr_set_fragment_number(beacon_buf, 0);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/
    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* ---------------------------------------------------------------------- */
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet|TIM  |CountryElem | */
    /* ---------------------------------------------------------------------- */
    /* |8        |2     |2      |2-34|3-10    |3         |6-256|8-256       | */
    /* ---------------------------------------------------------------------- */
    /* |PowerConstraint |Quiet|TPC Report|ERP |RSN  |WMM |Extended Sup Rates| */
    /* ---------------------------------------------------------------------- */
    /* |3               |8    |4         |3   |4-255|26  | 3-257            | */
    /* ---------------------------------------------------------------------- */
    /* |BSS Load |HT Capabilities |HT Operation |Overlapping BSS Scan       | */
    /* ---------------------------------------------------------------------- */
    /* |7        |28              |24           |16                         | */
    /* ---------------------------------------------------------------------- */
    /* |Extended Capabilities |                                              */
    /* ---------------------------------------------------------------------- */
    /* |3-8                   |                                              */
    /*************************************************************************/
    /* timestamp由硬件设置 */
    beacon_buf += MAC_80211_FRAME_LEN;
    (osal_void)memset_s(beacon_buf, MAC_TIME_STAMP_LEN, 0, MAC_TIME_STAMP_LEN); /* 清空8个字节的时间buf */

    /* Initialize index */
    beacon_buf += MAC_TIME_STAMP_LEN;

    /* 设置beacon interval */
    hmac_set_beacon_interval_field_etc(hmac_vap, beacon_buf);
    beacon_buf += MAC_BEACON_INTERVAL_LEN;

    /* 设置capability information */
    hmac_set_cap_info_ap_etc(hmac_vap, beacon_buf);
    beacon_buf += MAC_CAP_INFO_LEN;

    /* 设置ssid element,如果打开隐藏ssid开关的话，则不填ssid */
    hmac_set_ssid_ie_etc(hmac_vap, beacon_buf, &ie_len, WLAN_FC0_SUBTYPE_BEACON);
    beacon_buf += ie_len;

    /* 设置支持的速率集 */
    hmac_set_supported_rates_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 获取dsss ie内的channel num */
    dsss_channel_num = hmac_get_dsss_ie_channel_num(hmac_vap, 0);

    /* 设置dsss参数集 */
    mac_set_dsss_params_etc(hmac_vap, beacon_buf, &ie_len, dsss_channel_num);
    beacon_buf += ie_len;

    /* 填充tim信息 */
    hmac_set_tim_ie(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

#ifdef _PRE_WLAN_FEATURE_11D
    /* 填充country信息 */
    mac_set_country_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;
#endif

    /* 填充power constraint信息 */
    mac_set_pwrconstraint_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    quiet_info.count = MAC_QUIET_COUNT;
    quiet_info.duration = MAC_QUIET_DURATION;
    quiet_info.period = MAC_QUIET_PERIOD;
    quiet_info.offset = MAC_QUIET_OFFSET;

    /* 填充quiet信息 */
    mac_set_quiet_ie_etc(hmac_vap, beacon_buf, &quiet_info, &ie_len);
    beacon_buf += ie_len;

    /* 填充TPC Report信息 */
    mac_set_tpc_report_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充erp信息 */
    mac_set_erp_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充RSN 安全相关信息 */
    hmac_set_rsn_ie_etc(hmac_vap, OSAL_NULL, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充WPA 安全相关信息 */
    hmac_set_wpa_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充wmm信息 */
    hmac_set_wmm_params_ie_etc(hmac_vap, beacon_buf, mac_mib_get_dot11_qos_option_implemented(hmac_vap), &ie_len);
    beacon_buf += ie_len;

    /* 填充extended supported rates信息 */
    hmac_set_exsup_rates_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充bss load信息 */
    mac_set_bssload_ie(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充HT Capabilities信息 */
    hmac_set_ht_capabilities_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充HT Operation信息 */
    hmac_set_ht_opern_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充Overlapping BSS Scan信息 */
    hmac_set_obss_scan_params_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充Extended Capabilities信息 */
    hmac_set_ext_capabilities_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;
#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_set_rsnx_ie((osal_void *)hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;
#endif
    /* 填充vht cap信息 */
    hmac_set_vht_capabilities_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

    /* 填充vht opern信息 */
    hmac_set_vht_opern_ie_etc(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;

#ifdef _PRE_WLAN_FEATURE_11AX
    hmac_set_he_capabilities_ie(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;
    hmac_set_he_operation_ie(hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_ENCAP_BEACON_CSA_IE);
    if (fhook != OSAL_NULL) {
        ((hmac_csa_ap_encap_beacon_csa_ie_cb)fhook)(hmac_vap, beacon_buf, &ie_len);
        beacon_buf += ie_len;
    }

    /* 填充WPS信息 */
    hmac_add_app_ie_etc((osal_void *)hmac_vap, beacon_buf, &app_ie_len, OAL_APP_BEACON_IE);
    beacon_buf += app_ie_len;

    /* 填充扩展IE信息 */
    mac_add_extend_app_ie_etc(hmac_vap, &beacon_buf, BEACON_IE);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* 填充p2p noa Attribute */
    if ((is_p2p_go(hmac_vap)) && (is_p2p_noa_enabled(hmac_vap) || is_p2p_oppps_enabled(hmac_vap))) {
        mac_set_p2p_noa(hmac_vap, beacon_buf, &ie_len);
        beacon_buf += ie_len;
    }
#endif
    /* 增加1024 QAM IE */
#ifdef _PRE_WLAN_FEATURE_1024QAM
    hmac_set_1024qam_vendor_ie((osal_void *)hmac_vap, beacon_buf, &ie_len);
    beacon_buf += ie_len;
#endif

   /* wur特性下新增wur cap ie/wur oper ie */
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    if (hmac_vap->cap_flag.wur_enable) {
        mac_set_wur_capabilities_ie((osal_void *)hmac_vap, beacon_buf, &ie_len);
        beacon_buf += ie_len;
        mac_set_wur_operation_ie((osal_void *)hmac_vap, beacon_buf, &ie_len);
        beacon_buf += ie_len;

        /* WUR下对BSS参数更新处理 */
        hmac_wur_bss_update_process(hmac_vap);
    }
#endif

    return (osal_u16)(beacon_buf - bf_origin);
}

/*****************************************************************************
 函 数 名  : hmac_handle_tbtt_chan_mgmt_ap
 功能描述  : AP侧TBTT中断中进行信道管理
*****************************************************************************/
OSAL_STATIC osal_void hmac_handle_tbtt_chan_mgmt_ap(hmac_vap_stru *hmac_vap)
{
    mac_ch_switch_info_stru *csi = &(hmac_vap->ch_switch_info);
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_IS_SWITCH);

    if (fhook != OSAL_NULL && ((hmac_csa_ap_is_ch_switch_cb)fhook)(hmac_vap->vap_id) == OSAL_TRUE) {
        return;
    }

    if (csi->bw_switch_status == WLAN_BW_SWITCH_40_TO_20 || csi->bw_switch_status == WLAN_BW_SWITCH_20_TO_40) {
        if (hmac_vap->vap_state == MAC_VAP_STATE_UP) {
            csi->bw_switch_status = WLAN_BW_SWITCH_DONE;

            /* 现在VAP下的"带宽模式"应该为 WLAN_BAND_WIDTH_20MHz */
            /* 兼容sta+ap共存，出现信道变化可能需要切换到dbac或者dbdc，需要使用multi接口 */
            hmac_chan_multi_select_channel_mac(hmac_vap, hmac_vap->channel.chan_number,
                hmac_vap->channel.en_bandwidth);
            hmac_switch_complete_notify(hmac_vap, OSAL_FALSE);
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_set_linkloss_params
 功能描述  :  设置当前linkloss参数
*****************************************************************************/
osal_s32 hmac_config_set_linkloss_params(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    wifi_linkloss_params *linkloss_para = OSAL_NULL;

    unref_param(hmac_vap);
    if (msg == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_linkloss_params:: msg NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    linkloss_para = (wifi_linkloss_params *)msg->data;
    /* 参数配置限制，linkloss老化时间必须在50~250之间，对应基础老化时间5s到25s */
    if (linkloss_para->linkloss_threshold < 50 || linkloss_para->linkloss_threshold > 250) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_set_linkloss_params:: linkloss_threshold out of range[%u]!}",
            linkloss_para->linkloss_threshold);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 参数配置限制，发送probe request帧时间比例必须在1~10之间，对应N/10的时间发送 */
    if (linkloss_para->send_proberequest_ratio < 1 || linkloss_para->send_proberequest_ratio > 10) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_set_linkloss_params:: send_probeRequest_ratio out of range[%u]!}",
            linkloss_para->send_proberequest_ratio);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }
    /* 将linkloss老化门限配置为设置值，dbac与蓝牙共存模式下处理逻辑保持不变，依然翻倍,但最大值限制依然为255 */
    g_linkloss_threshold[WLAN_LINKLOSS_MODE_NORMAL] = linkloss_para->linkloss_threshold;
    g_linkloss_threshold[WLAN_LINKLOSS_MODE_BT] = osal_min(linkloss_para->linkloss_threshold * 2, 0xff); /* 2: 翻倍 */
    g_linkloss_threshold[WLAN_LINKLOSS_MODE_DBAC] = osal_min(linkloss_para->linkloss_threshold * 2, 0xff); /* 2: 翻倍 */

    g_linkloss_send_probereq_ratio = linkloss_para->send_proberequest_ratio;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_linkloss_update_mode
 功能描述  :  设置当前linkloss的场景
              优先级:BT > DBAC > NORMAL
*****************************************************************************/
OSAL_STATIC osal_void hmac_linkloss_update_mode(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    oal_bool_enum_uint8 bt_on = OSAL_FALSE;
    oal_bool_enum_uint8 dbac_running;
    wlan_linkloss_mode_enum_uint8 old_linkloss_mode;
    osal_void *fhook = OSAL_NULL;

    old_linkloss_mode = hmac_vap->linkloss_info.linkloss_mode;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_GET_BTSTATUS_BTON);
    if (fhook != OSAL_NULL) {
        bt_on = ((hmac_btcoex_hal_get_btstatus_bton_cb)fhook)();
    }

    dbac_running = mac_is_dbac_running(hmac_device);

    if (bt_on == OSAL_FALSE) {
        /* 普通 场景 */
        if (dbac_running == OSAL_FALSE) {
            hmac_vap->linkloss_info.linkloss_mode = WLAN_LINKLOSS_MODE_NORMAL;
        } else { /* DBAC 场景 */
            hmac_vap->linkloss_info.linkloss_mode = WLAN_LINKLOSS_MODE_DBAC;
        }
    } else { /* BT 场景优先级高，只要BT开了，就认为是BT 场景 */
        hmac_vap->linkloss_info.linkloss_mode = WLAN_LINKLOSS_MODE_BT;
    }

    if (old_linkloss_mode != hmac_vap->linkloss_info.linkloss_mode) {
        oam_warning_log4(0, OAM_SF_RX,
            "{hmac_linkloss_update_mode::changed!,old_mode[%d] --> new_mode[%d], linkloss[%d], linkloss_threshold[%d]}",
            old_linkloss_mode, hmac_vap->linkloss_info.linkloss_mode, get_current_linkloss_cnt(hmac_vap),
            get_current_linkloss_threshold(hmac_vap));
    }
}

static osal_u8 get_current_linkloss_send_probereq_threshold(const hmac_vap_stru *hmac_vap)
{
    osal_u16 tmp;
    /* u8 乘上 1~10不用考虑溢出 */
    tmp = get_current_linkloss_int_threshold(hmac_vap) * g_linkloss_send_probereq_ratio ;
    /* 除以10得到发送probe req的门限，该值一定小于255，不用考虑截断 */
    return (osal_u8)(tmp / 10);
}

/*****************************************************************************
 函 数 名  : hmac_linkloss_send_probe_req
 功能描述  :  非背景扫描阶段，LinkLoss计数器超过一半，准备开始发送单播探测帧接口
*****************************************************************************/
OSAL_STATIC osal_void hmac_linkloss_send_probe_req(hmac_vap_stru *hmac_vap, const hmac_device_stru *hmac_device)
{
    osal_u8 send_prob_period;

    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);

    /* 扫描、雷达、2040切信道时候，linkloss计数器不计数,所以不额外发送单播探测帧 */
    if (fhook != OSAL_NULL && ((hmac_sta_csa_fsm_is_waiting_cb)fhook)(hmac_vap) == OSAL_TRUE) {
        return;
    }

    if ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING)) {
        return;
    }

    if ((get_current_linkloss_int_threshold(hmac_vap) > LINKLOSS_PROBE_REQ_MIN_CNT) &&
        (get_current_linkloss_int_threshold(hmac_vap) <= LINKLOSS_PORBE_REQ_MAX_CNT)) {
        /* linkloss阈值为10到60时对3取余发送probe request */
        hmac_vap->linkloss_info.send_probe_req_period = LINKLOSS_PROBE_REQ_MIN_MOD;
    } else {
        hmac_vap->linkloss_info.send_probe_req_period = LINKLOSS_PROBE_REQ_MAX_MOD;
    }

    send_prob_period = (get_current_linkloss_cnt(hmac_vap) % hmac_vap->linkloss_info.send_probe_req_period);
    if ((get_current_linkloss_cnt(hmac_vap) >= get_current_linkloss_send_probereq_threshold(hmac_vap)) &&
        (send_prob_period == 0)) {
        oam_warning_log4(0, OAM_SF_ANY,
            "vap_id[%d] hmac_linkloss_send_probe_req:: curr_cnt[%d] curr_threshold[%d] send_prob_period[%d]",
            hmac_vap->vap_id, get_current_linkloss_cnt(hmac_vap),
            get_current_linkloss_int_threshold(hmac_vap), hmac_vap->linkloss_info.send_probe_req_period);
#ifdef _PRE_WIFI_DEBUG
        hal_show_linkloss_reg_info();
#endif
        /* 根据索引号，准备probe req帧的ssid信息 */
        hmac_scan_send_probe_req_frame(hmac_vap, hmac_vap->bssid, (osal_char *)(mac_mib_get_desired_ssid(hmac_vap)),
                                       WLAN_SSID_MAX_LEN, OAL_TRUE);
    }
}

/*****************************************************************************
 功能描述  : 更新linkloss计数器值
*****************************************************************************/
OSAL_STATIC osal_void hmac_calc_linkloss_cnt(hmac_vap_stru *hmac_vap, osal_u8 old_th_base, osal_u16 old_linkloss_cnt)
{
    osal_u64 current_time, last_active_time;
    /* linkloss计数缩减因子, 左移10位用于将浮点转成整数 */
    osal_u32 scale_factor = 1 << 0x10;
    /* 前后门限一致或者是初始化时，不需要进行linkloss计数切换 */
    if (old_th_base == get_current_linkloss_threshold(hmac_vap) || old_th_base == 0) {
        return;
    }
    /* 阈值基准变化，计算缩减因子并重新计算linkloss计数器值 */
    scale_factor = ((scale_factor * get_current_linkloss_threshold(hmac_vap)) / old_th_base);
    hmac_vap->linkloss_info.linkloss_info[hmac_vap->linkloss_info.linkloss_mode].link_loss =
        (osal_u16)((old_linkloss_cnt * scale_factor) >> 0x10);
    current_time = osal_get_time_stamp_ts();
    /* 左移10位近似于乘1000，将毫秒单位转换为微秒单位 */
    last_active_time =
        (current_time > ((get_current_linkloss_cnt(hmac_vap) * mac_mib_get_beacon_period(hmac_vap)) << 0xA)) ?
        (current_time - ((get_current_linkloss_cnt(hmac_vap) * mac_mib_get_beacon_period(hmac_vap)) << 0xA)) : 0;
    /* 低32位直接按位与上U32 MAX */
    hmac_vap->linkloss_info.last_active_time_l32 = last_active_time & EXT_TIME_US_MAX_LEN;
    /* 右移32位取高32位 */
    hmac_vap->linkloss_info.last_active_time_h32 = (osal_u32)(last_active_time >> 32);
    oam_warning_log4(0, OAM_SF_RX,
        "vap_id[%d] {hmac_calc_linkloss_cnt::old_cnt[%d] change to new_cnt[%d] last_active_time[%llu]!}",
        hmac_vap->vap_id, old_linkloss_cnt, get_current_linkloss_cnt(hmac_vap), last_active_time);
}

/*****************************************************************************
 函 数 名  : hmac_linkloss_update_threshold
 功能描述  : 更新linkloss门限值
*****************************************************************************/
OSAL_STATIC osal_void hmac_linkloss_update_threshold(hmac_vap_stru *hmac_vap)
{
    osal_u8 mode;
    osal_u8 old_th_base = get_current_linkloss_threshold(hmac_vap);

    for (mode = WLAN_LINKLOSS_MODE_BT; mode < WLAN_LINKLOSS_MODE_BUTT; mode++) {
        /* 门限初始值，最大255.数字100用于单位转换 */
        if ((mac_mib_get_powermanagementmode(hmac_vap)) == WLAN_MIB_PWR_MGMT_MODE_PWRSAVE) {
            hmac_vap->linkloss_info.linkloss_info[mode].linkloss_threshold = g_linkloss_threshold[mode];
        } else {
            hmac_vap->linkloss_info.linkloss_info[mode].linkloss_threshold = ((PERIOD_100_MS *
                g_linkloss_threshold[mode] / hmac_vap->linkloss_info.dot11_beacon_period) < 0xFF) ?
                (osal_u8)(PERIOD_100_MS * g_linkloss_threshold[mode] / hmac_vap->linkloss_info.dot11_beacon_period) :
                0xFF;
        }

        /* 门限初始值，最小10 */
        if (hmac_vap->linkloss_info.linkloss_info[mode].linkloss_threshold < WLAN_LINKLOSS_MIN_THRESHOLD) {
            hmac_vap->linkloss_info.linkloss_info[mode].linkloss_threshold = WLAN_LINKLOSS_MIN_THRESHOLD;
        }

        /* 门限初始值，只随beacon周期改变 */
        hmac_vap->linkloss_info.linkloss_info[mode].int_linkloss_threshold =
            hmac_vap->linkloss_info.linkloss_info[mode].linkloss_threshold;

        oam_warning_log3(0, OAM_SF_RX,
            "vap_id[%d] {hmac_linkloss_update_threshold::linkloss_mode[%d], int_threshold[%d]!}",
            hmac_vap->vap_id, mode, hmac_vap->linkloss_info.linkloss_info[mode].linkloss_threshold);
    }
    /* 阈值改变，更新linkloss计数器值 */
    hmac_calc_linkloss_cnt(hmac_vap, old_th_base, get_current_linkloss_cnt(hmac_vap));
}

/*****************************************************************************
 函 数 名  : hmac_vap_linkloss_init
 功能描述  :  初始化linkloss检测工具
*****************************************************************************/
osal_void hmac_vap_linkloss_init(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
        return;
    }

    hmac_vap_linkloss_clean(hmac_vap);

    /* 尚未关联，预先设置对端ap的beacon周期为100 */
    hmac_vap->linkloss_info.dot11_beacon_period = WLAN_BEACON_INTVAL_DEFAULT;

    /* 初始化状态 */
    hmac_vap->linkloss_info.state = (osal_u8)mac_mib_get_powermanagementmode(hmac_vap);

    hmac_linkloss_update_mode(hmac_vap, hmac_device);

    hmac_vap->linkloss_info.linkloss_times = 0;

    hmac_vap->linkloss_info.linkloss_rate = LINKLOSS_CNT_UNDER_8;

    hmac_linkloss_update_threshold(hmac_vap);

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_LINKLOSS_INIT, hmac_vap);
}
/*****************************************************************************
 函 数 名  : hmac_vap_linkloss_incr
 功能描述  : 对linkloss计数器加1
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_linkloss_incr(hmac_vap_stru *hmac_vap, const hmac_device_stru *hmac_device)
{
    osal_u64 current_time, last_active_time;
    osal_u32 linkloss_cnt;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);
    osal_void *fhook_pm = hmac_get_feature_fhook(HMAC_FHOOK_PM_STA_STAT);

    /* 当前host定时器触发时间精度有限，需要用系统时间要校准linkloss cnt */
    current_time = osal_get_time_stamp_ts();
    /* 扫描、雷达、2040切信道时候，linkloss计数器不计数 */
    if ((fhook == OSAL_NULL || ((hmac_sta_csa_fsm_is_waiting_cb)fhook)(hmac_vap) == OSAL_FALSE) &&
        (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING) &&
        (fhook_pm == OSAL_NULL || ((hmac_sta_pm_get_wlan_dev_state_cb)fhook_pm)() == HMAC_DISALLOW_SLEEP)) {
        last_active_time = osal_makeu64(hmac_vap->linkloss_info.last_active_time_l32,
            hmac_vap->linkloss_info.last_active_time_h32);
        /* linkloss计算cnt值时，微妙需要转换成tu再跟beacon周期计算（微秒右移10位转为tu），保证计数对应时间的准确 */
        if (current_time > last_active_time) {
            linkloss_cnt = (osal_u32)((current_time - last_active_time) >> 10) / /* 10表示微秒右移10位转为tu */
                (mac_mib_get_beacon_period(hmac_vap));
            /* 设置一个最大门限，如果算出来一个超大值，有可能是SNTP时间同步导致，此时不能刷新时间，否则会导致异常linkloss */
            if (linkloss_cnt > WLAN_LINKLOSS_MAX_TH) {
                return;
            /* 同时设置最大跳变个数为5个，如果超过了，说明时间跳变 or 长时间没有调用定时器，需要HSO日志记录异常，但是仍然赋值 */
            } else if (linkloss_cnt - get_current_linkloss_cnt(hmac_vap) > 5) {
                oam_warning_log2(0, OAM_SF_ANY,
                    "{hmac_vap_linkloss_incr:: last_linkloss_cnt[%u] now_linkloss_cnt[%u]!}",
                    linkloss_cnt, get_current_linkloss_cnt(hmac_vap));
            }
            hmac_vap->linkloss_info.linkloss_info[hmac_vap->linkloss_info.linkloss_mode].link_loss = linkloss_cnt;
        }
        return;
    }

    /* 当可能出现linkloss情况时(linkloss计数已经大于10)，统计linkloss计数器不计数的情况，以便推算linkloss总时间是否正常 */
    if (get_current_linkloss_cnt(hmac_vap) > 10) {
        oam_info_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_vap_linkloss_incr::scan state[%u], skip linkloss incr.}",
            hmac_vap->vap_id, hmac_device->curr_scan_state);
    }
    /* linkloss不计数时，对应的上次活跃时间应该为当前时间减去cnt个beacon间隔(tu左移10位转为微秒) */
    last_active_time = current_time - (get_current_linkloss_cnt(hmac_vap) *
        (mac_mib_get_beacon_period(hmac_vap) * (1 << 10))); /* 10: 左移10位 */
    /* 低32位直接按位与上U32 MAX */
    hmac_vap->linkloss_info.last_active_time_l32 = last_active_time & EXT_TIME_US_MAX_LEN;
    /* 右移32位取高32位 */
    hmac_vap->linkloss_info.last_active_time_h32 = (osal_u32)(last_active_time >> 32);
}

/*****************************************************************************
 函 数 名  : hmac_vap_linkloss_update_threshold
 功能描述  :  更新linkloss门限值
*****************************************************************************/
OSAL_STATIC osal_void hmac_vap_linkloss_update_threshold(hmac_vap_stru *hmac_vap, const hmac_device_stru *hmac_device)
{
    osal_u32 dot11_beacon_period;
    hmac_vap_linkloss_stru *linkloss_info = OSAL_NULL;
    osal_bool period_status = OSAL_TRUE;
    osal_void *fhook = OSAL_NULL;

    if (hmac_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_vap_linkloss_update_threshold:: mac_device is null!!!}");
        return;
    }
    dot11_beacon_period = mac_mib_get_beacon_period(hmac_vap);

    linkloss_info = &(hmac_vap->linkloss_info);
    /* 低功耗场景linkloss值不改 */
    if ((mac_mib_get_powermanagementmode(hmac_vap)) == WLAN_MIB_PWR_MGMT_MODE_PWRSAVE) {
        /* 状态改变更新阈值 */
        if (linkloss_info->state != WLAN_MIB_PWR_MGMT_MODE_PWRSAVE) {
            linkloss_info->state = WLAN_MIB_PWR_MGMT_MODE_PWRSAVE;
            hmac_linkloss_update_threshold(hmac_vap);
        }
        period_status =  OSAL_FALSE;
    }

    /* 状态改变更新阈值 */
    if (period_status && linkloss_info->state == WLAN_MIB_PWR_MGMT_MODE_PWRSAVE) {
        linkloss_info->state =  WLAN_MIB_PWR_MGMT_MODE_ACTIVE;
        hmac_linkloss_update_threshold(hmac_vap);
        period_status =  OSAL_FALSE;
    }

    /* 无效beacon间隔(为0)，强行按照beacon间隔为100ms计算linkloss门限 */
    if (dot11_beacon_period == 0) {
        dot11_beacon_period = WLAN_BEACON_INTVAL_DEFAULT;
    }

    /* beacon>400, 按照beacon间隔为400ms计算linkloss门限 */
    if (dot11_beacon_period > LINKLOSS_THRESHOLD_BEACON_MAX_INTVAL) {
        dot11_beacon_period = LINKLOSS_THRESHOLD_BEACON_MAX_INTVAL;
    }

    /* beacon周期改变需要更新门限 */
    if (dot11_beacon_period == linkloss_info->dot11_beacon_period) {
        return;
    }

    linkloss_info->dot11_beacon_period = dot11_beacon_period;

    linkloss_info->linkloss_times = (osal_u8)((dot11_beacon_period < WLAN_BEACON_INTVAL_DEFAULT) ?
        (WLAN_BEACON_INTVAL_DEFAULT / dot11_beacon_period) : 1);

    oam_warning_log3(0, OAM_SF_RX,
        "vap_id[%d] {hmac_vap_linkloss_update_threshold::dot11BeaconPeriod[%d] is changing to %d}",
        hmac_vap->vap_id, linkloss_info->dot11_beacon_period, dot11_beacon_period);
    if (period_status) {
        hmac_linkloss_update_threshold(hmac_vap);
    }
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_UPDATE_THRESHOLD);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_linkloss_update_threshold_cb)fhook)(hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_vap_linkloss_clean
 功能描述  :  对linkloss计数器清0
*****************************************************************************/
WIFI_TCM_TEXT osal_void hmac_vap_linkloss_clean(hmac_vap_stru *hmac_vap)
{
    osal_void *fhook = OSAL_NULL;
    osal_u64 last_active_time;
    if (hmac_vap == OSAL_NULL) {
        return;
    }

    if (hmac_vap->vap_state != MAC_VAP_STATE_UP) {
        oam_info_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_vap_linkloss_clean::vap_state[%d] is not MAC_VAP_STATE_UP, no need to clean.}",
            hmac_vap->vap_id, hmac_vap->vap_state);
        return;
    }

    hmac_vap->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_BT].link_loss = 0;
    hmac_vap->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_DBAC].link_loss = 0;
    hmac_vap->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_NORMAL].link_loss = 0;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_SET_LINKLOSS_DONE);
    if (fhook != OSAL_NULL) {
        ((hmac_roam_set_linkloss_done_cb)fhook)(hmac_vap->vap_id, OSAL_FALSE);
    }

    /* 门限清零时等价于linkloss时间重置，更新时间戳 */
    last_active_time = osal_get_time_stamp_ts();
    /* 低32位直接按位与上U32 MAX */
    hmac_vap->linkloss_info.last_active_time_l32 = last_active_time & EXT_TIME_US_MAX_LEN;
    /* 右移32位取高32位 */
    hmac_vap->linkloss_info.last_active_time_h32 = (osal_u32)(last_active_time >> 32);

    /* linkloss到达门限一半之后，默认10个TBTT作为一个频率发送单播探测帧 */
    hmac_vap->linkloss_info.send_probe_req_period = 10;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_HAL_LINKLOSS_CLEAN);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_hal_blinkloss_clean_cb)fhook)(hmac_vap);
    }
}

OSAL_STATIC osal_void hmac_sta_tbtt_prior_linkloss_handler(hmac_vap_stru *hmac_vap)
{
    dmac_linkloss_rate_uint8 linkloss_rate = LINKLOSS_RATE_BUTT;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);

    hmac_linkloss_update_mode(hmac_vap, hmac_device);
    hmac_vap_linkloss_update_threshold(hmac_vap, hmac_device);
    hmac_vap_linkloss_incr(hmac_vap, hmac_device);

    if (get_current_linkloss_cnt(hmac_vap) <= ALG_ANTI_INF_BEACON_LOSS_NUM_TH) {
        linkloss_rate = LINKLOSS_CNT_UNDER_8;
    } else if (get_current_linkloss_cnt(hmac_vap) * 3 <= get_current_linkloss_threshold(hmac_vap)) { /* 3代表三分之一 */
        linkloss_rate = LINKLOSS_RATE_BETWEEN_8_AND_ONE_THIRD;
    } else if (get_current_linkloss_cnt(hmac_vap) * 2 <= get_current_linkloss_threshold(hmac_vap)) { /* 2代表二分之一 */
        linkloss_rate = LINKLOSS_RATE_BETWEEN_ONE_THIRD_AND_A_HALF;
    } else if (get_current_linkloss_cnt(hmac_vap) <= get_current_linkloss_threshold(hmac_vap)) {
        linkloss_rate = LINKLOSS_RATE_MORE_THAN_A_HALF;
    }
    if ((linkloss_rate != LINKLOSS_RATE_BUTT) && (linkloss_rate != hmac_vap->linkloss_info.linkloss_rate)) {
        hmac_vap->linkloss_info.linkloss_rate = linkloss_rate;
        hmac_vap_sync(hmac_vap);
    }
    /* linkloss计数超过一半,准备发送单播探测帧 */
    hmac_linkloss_send_probe_req(hmac_vap, hmac_device);

    /* linkloss每统计达到10次，上报一次维测信息，扫描或切信道时刻不打印 */
    if ((get_current_linkloss_cnt(hmac_vap) % WLAN_LINKLOSS_REPORT == 0) &&
        (get_current_linkloss_cnt(hmac_vap) != 0) &&
        (fhook == OSAL_NULL || ((hmac_sta_csa_fsm_is_waiting_cb)fhook)(hmac_vap) == OSAL_FALSE) &&
        (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING)) {
#ifdef _PRE_WLAN_DFT_STAT
        /* 打印link_loss计数 */
        hmac_dft_report_linkloss_info(hmac_vap, hmac_device);
#endif
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_misc
 功能描述  : up状态下, misc事件处理.向ap发去关联。如果sta与ap处于关联状态，应
             使用去关联踢掉对方。
 ****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_misc(hmac_vap_stru *hmac_vap)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_TRIGGER_SEND);

    hmac_sta_tbtt_prior_linkloss_handler(hmac_vap);

    if (fhook != OSAL_NULL) {
        ((hmac_sta_roam_trigger_event_send_cb)fhook)(hmac_vap);
    }

    /* sta与ap丢失连接超过上限处理 */
    if (hmac_is_linkloss(hmac_vap) == OSAL_TRUE) {
        oam_warning_log4(0, OAM_SF_ANY,
            "{hmac_sta_up_misc::Oh no,LinkLoss!! BSSID[%02X:%02X:%02X:%02X:XX:XX]} ", hmac_vap->bssid[0],
            /* bssid index 1, bssid index 2, bssid index 3 */
            hmac_vap->bssid[1], hmac_vap->bssid[2], hmac_vap->bssid[3]);
        wifi_printf("Oh no,LinkLoss!! BSSID[%02X:%02X:%02X:%02X:XX:XX]\r\n", hmac_vap->bssid[0],
            /* bssid index 1, bssid index 2, bssid index 3 */
            hmac_vap->bssid[1], hmac_vap->bssid[2], hmac_vap->bssid[3]);
#ifdef _PRE_WLAN_DFT_STAT
        /* 打印link_loss计数 */
        hmac_dft_report_linkloss_info(hmac_vap, hmac_res_get_mac_dev_etc(0));
#endif
        hmac_vap_linkloss_clean(hmac_vap);
        oam_warning_log1(0, OAM_SF_DFT, "hmac_sta_up_misc:linkloss,send DISASSOC,err code[%d]", MAC_ASOC_NOT_AUTH);
#ifdef _PRE_WIFI_DEBUG
        hal_show_linkloss_reg_info();
#endif
        hmac_send_disassoc_misc_event(hmac_vap, hmac_vap->assoc_vap_id,
            DMAC_DISASOC_MISC_LINKLOSS);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_tbtt_event_sta
 功能描述  : sta tbtt中断处理, 抛到hmac去处理
*****************************************************************************/
osal_u32 hmac_tbtt_event_sta(osal_void *arg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    osal_u32 present_time, runtime;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);

    hmac_vap = (hmac_vap_stru *)arg;

    if (hmac_vap->hal_device == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] hmac_tbtt_event_sta:hal_device null.", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    present_time = (osal_u32)osal_get_time_stamp_ms();
    runtime = osal_get_runtime(hmac_vap->sta_tbtt.last_tbtt_time, present_time);
    if (runtime < STA_TBTT_INTERVAL_TH) {
        oam_info_log1(0, 0, "vap_id[%d] hmac_tbtt_event_sta:tbtt interval too short.", hmac_vap->vap_id);
        return OAL_SUCC;
    }
    /* 定时器间隔超过2倍beacon周期是异常 */
    if (runtime > 2 * mac_mib_get_beacon_period(hmac_vap)) {
        oam_warning_log2(0, 0, "vap_id[%d] hmac_tbtt_event_sta:tbtt interval too long time[%u].",
            hmac_vap->vap_id, runtime);
    }
    hmac_vap->sta_tbtt.last_tbtt_time = present_time;

    if ((hmac_vap->vap_state == MAC_VAP_STATE_UP) || (mac_is_dbac_running(hmac_device) != OSAL_FALSE) ||
        (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING &&
        hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_ROAM_SCAN)) {
        hmac_sta_up_misc(hmac_vap);
    } else {
        oam_info_log4(0, 0, "vap_id[%d] hmac_tbtt_event_sta:vap_state[%u] dbac[%u] scan_state[%u].", hmac_vap->vap_id,
            hmac_vap->vap_state, mac_is_dbac_running(hmac_device), hmac_device->curr_scan_state);
    }

    /* 扫描、雷达、2040切信道时候，不进入bt防护wifi linkloss流程 */
    if ((fhook == OSAL_NULL || ((hmac_sta_csa_fsm_is_waiting_cb)fhook)(hmac_vap) == OSAL_FALSE) &&
        (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING)) {
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_LINKLOSS_OCCUPIED);
        if (fhook != OSAL_NULL) {
            ((hmac_btcoex_linkloss_occupied_process_cb)fhook)(hmac_vap->hal_chip, hmac_vap->hal_device, hmac_vap);
        }
    }
    hmac_alg_anti_intf_tbtt_handler(hmac_vap, hmac_vap->hal_device);
    present_time = (osal_u32)osal_get_time_stamp_ms();
    runtime = osal_get_runtime(hmac_vap->sta_tbtt.last_tbtt_time, present_time);
    if (runtime > mac_mib_get_beacon_period(hmac_vap)) {
        oam_warning_log2(0, 0, "vap_id[%d] hmac_tbtt_event_sta:tbtt sta callback exec too long time[%u].",
            hmac_vap->vap_id, runtime);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_tbtt_event_ap_bottom_half
 功能描述  : aput tbtt中断下半部操作
*****************************************************************************/
OSAL_STATIC osal_void hmac_tbtt_event_ap_bottom_half(hmac_vap_stru *hmac_vap)
{
    osal_u32 offset;
    osal_u32 led_tbtt_timer = 0;
    hmac_user_stru *hmac_multi_user = OSAL_NULL;
    osal_u32 ps_mpdu_num = 0;
    /* 当前DTIM周期未到，不会进行信道管理 */
    if (hmac_vap->dtim_count != 0) {
        return;
    }

    offset = hmac_vap->in_tbtt_offset;
    hal_vap_read_tbtt_timer(hmac_vap->hal_vap, &led_tbtt_timer);
    hmac_multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
    if (hmac_multi_user == OSAL_NULL) {
        oam_warning_log2(0, 0, "vap_id[%d] {multi user[%d] fail.}", hmac_vap->vap_id, hmac_vap->multi_user_idx);
        return;
    }

    /* 硬件发送beacon之前需要将准备好节能队列中的发送组播/广播，否则下次唤醒再发送 */
    if (led_tbtt_timer < offset) {
        /* 如果此次tbtt中断对应的是DTIM周期，则需要发送组播/广播节能队列中的缓存帧 */
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        /* 若存在WUR用户，组播缓存下发特殊处理 */
        if (hmac_wur_is_multi_flush(hmac_vap, hmac_multi_user)) {
#endif
            /* 解决当前PSM未开启时引入的节能队列刷新问题 */
            hmac_psm_queue_flush(hmac_vap, hmac_multi_user);
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        }
#endif
    } else {
        ps_mpdu_num = osal_adapt_atomic_read(&hmac_multi_user->ps_structure.mpdu_num); // dtim = 0, dtim force clear
        if (ps_mpdu_num != 0) {
            hmac_psm_set_local_bitmap(hmac_vap, hmac_multi_user, 1);
        }
        oam_warning_log3(0, OAM_SF_WIFI_BEACON,
            "vap_id[%d] {Delay to flush, tbtt_timer %d > tbtt_offset %d}", hmac_vap->vap_id, led_tbtt_timer, offset);
    }

    /* 遍历全部节能用户，检查节能队列是否有包，连续5个dtim周期有包，且数目大于门限，则进行丢弃 */
    if ((hmac_vap->user_nums >= 1) &&
#ifdef _PRE_WLAN_FEATURE_WUR_TX
        ((hmac_vap->ps_user_num != 0) || (hmac_vap->cur_wur_user_num != 0)) /* 当前有处于WUR的用户，也复用丢包操作 */
#else
        (hmac_vap->ps_user_num != 0)
#endif
    ) {
        hmac_user_ps_queue_overrun_notify(hmac_vap);
    }

    /* 信道管理 */
    hmac_handle_tbtt_chan_mgmt_ap(hmac_vap);
}

/*****************************************************************************
 功能描述  : ap tbtt中断处理,
           tbtt中断来临时beacon已经发送, tbtt中断中组的beacon帧在下一次beacon周期发送;
*****************************************************************************/
OSAL_STATIC osal_void hmac_tbtt_event_ap(hmac_vap_stru *hmac_vap)
{
    hal_beacon_tx_params_stru beacon_tx_params;
    osal_u16 beacon_len;
    osal_u8 *bcn_payload = OSAL_NULL;
    hal_tx_txop_alg_stru *tx_param = OSAL_NULL;
    osal_void *fhook = OSAL_NULL;

    if ((hmac_vap->vap_state != MAC_VAP_STATE_UP) && (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
        return;
    }

    if (osal_unlikely(hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_WIFI_BEACON, "vap_id[%d] {hmac_tbtt_event_ap::hal_device is null!}", hmac_vap->vap_id);
        return;
    }

    /* 软件要更新的beacon帧 */
    if (hmac_vap->beacon_buffer == OSAL_NULL) {
        /* vap down时候会执行dmac_beacon_free，此时beacon内存会释放，另一方面dmac down中vap状态现在需要hmac来同步，
         * hmac侧INIT状态还没有即使置位同步到dmac，上半部就打断，此时为空属于正常 */
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_tbtt_event_ap::beacon buf = NULL}", hmac_vap->vap_id);
        return;
    }
    bcn_payload = oal_netbuf_header(hmac_vap->beacon_buffer);

    /* 对于还未组beacon帧，需要tim cnt提前减1 */
    hmac_update_dtim_count_ap(hmac_vap);

    tx_param = &(hmac_vap->tx_mgmt_bmcast[hmac_vap->channel.band]);
    /* 更新beacon帧 */
    beacon_len = hmac_encap_beacon(hmac_vap, bcn_payload);

    /* dtim count为0，bitmap组播对应bit置0 */
    if (hmac_vap->dtim_count == 0) {
        hmac_user_stru *hmac_multi_user = mac_res_get_hmac_user_etc(hmac_vap->multi_user_idx);
        if (hmac_multi_user == OSAL_NULL) {
            oam_warning_log2(0, 0, "vap_id[%d] {multi user[%d] fail.}", hmac_vap->vap_id, hmac_vap->multi_user_idx);
            return;
        }
        hmac_psm_set_local_bitmap(hmac_vap, hmac_multi_user, 0);
    }

    /* 将更新的beacon帧写入硬件beacon帧寄存器 */
    /* 填写发送Beacon帧参数 */
    beacon_tx_params.tx_param = tx_param;
    beacon_tx_params.pkt_ptr = (uintptr_t)bcn_payload;
    beacon_tx_params.pkt_len = beacon_len;

    hmac_set_tx_chain_mask(hmac_vap, &beacon_tx_params.tx_chain_mask);

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_TBTT_AP, hmac_vap);

    /* HAL发送Beacon帧接口 */
    hmac_vap_send_beacon_pkt_msg(hmac_vap->vap_id, &beacon_tx_params);
    /* host侧TX方向BEACON帧上报到HSO */
    oal_netbuf_set_len(hmac_vap->beacon_buffer, beacon_len);
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_FRAME_REPORT_HSO);
    if (fhook != OSAL_NULL) {
        ((dfx_frame_report_hso_cb)fhook)(hmac_vap->beacon_buffer, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    }
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    hmac_sniffer_80211_frame(bcn_payload, MAC_80211_FRAME_LEN, bcn_payload + MAC_80211_FRAME_LEN, beacon_len,
        SOC_DIAG_MSG_ID_WIFI_TX_BEACON);
#endif
}

/*****************************************************************************
 函 数 名  : hmac_tbtt_event_handler
 功能描述  : TBTT中断事件处理函数
*****************************************************************************/
osal_s32 hmac_tbtt_event_handler(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* vap删除，tbtt中断发现hal_device为空, tbtt中断下半部执行时候，vap已经删除，
     * 此时该vap的tbtt中断不处理，打印出来vap id确认下是否是删除的vap，然后直接返回即可
     */
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    unref_param(msg);

    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] hmac_tbtt_event_handler:hal_device null.", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->al_tx_flag == EXT_SWITCH_ON) {
        return OAL_SUCC;
    }

    frw_util_notifier_notify(WLAN_UTIL_NOTIFIER_EVENT_TBTT_HANDLE, hmac_vap);

    if (is_ap(hmac_vap)) {
        if (hal_device->tbtt_update_beacon_func != OAL_PTR_NULL) {
            hal_device->tbtt_update_beacon_func(hmac_vap->vap_id);
        } else {
            oam_error_log0(0, OAM_SF_IRQ, "hmac_tbtt_event_handler::p_tbtt_update_beacon_func is NULL!!!");
        }
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        hmac_tbtt_event_ap_bottom_half(hmac_vap);
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_sta_align_tbtt_timer(hmac_vap, OSAL_TRUE);
    }

#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
    hmac_ant_tbtt_process(hal_device);
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_irq_tbtt_ap_isr
 功能描述  : 在中断上半段执行的beacon帧更新操作
*****************************************************************************/
osal_u32 hmac_irq_tbtt_ap_isr(osal_u8 mac_vap_id)
{
    hmac_vap_stru *hmac_vap;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_WIFI_BEACON, "vap_id[%d] {hmac_irq_tbtt_ap_isr::hmac_vap null!}", mac_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (!is_ap(hmac_vap)) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_irq_tbtt_ap_isr::hmac_vap is nor ap_mode!}", mac_vap_id);
        return OAL_FAIL;
    }

    hmac_tbtt_event_ap(hmac_vap);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_vap_linkloss_channel_clean
 功能描述  : 在和对端相同的信道收到本BSS的beacon或者prob rsp，清空linkloss
*****************************************************************************/
osal_void hmac_vap_linkloss_channel_clean(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u8 frame_channel;
    dmac_rx_ctl_stru *rx_ctl;
    mac_rx_ctl_stru *rx_info;
    osal_u8 *payload;
    osal_u16 msg_len;

    rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    rx_info = (mac_rx_ctl_stru *)(&(rx_ctl->rx_info));
    payload = oal_netbuf_rx_data(netbuf);
    msg_len = mac_get_rx_cb_payload_len(rx_info); /* 帧体长度 */

    frame_channel = hmac_ie_get_chan_num_etc(payload,
        msg_len, // MAC_GET_RX_CB_PAYLOAD_LEN中已经减去了帧头的长度
        MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN, rx_ctl->rx_info.channel_number);
    /* 判断接收到的beacon 帧和本VAP 是相同信道,才清零linkloss 计数 */
    if ((hmac_vap->channel.chan_number == frame_channel) && (frame_channel != 0)) {
        hmac_vap_linkloss_clean(hmac_vap);
    }
}

/****************************************************************************
 函 数 名  : hmac_sta_set_bandwith_hander
 功能描述  : STAUT设置带宽
*****************************************************************************/
osal_u32 hmac_sta_set_bandwith_handler(hmac_vap_stru *hmac_vap, wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth)
{
    osal_void *fhook = OSAL_NULL;

    if (hmac_vap_set_bw_check(hmac_vap, sta_new_bandwidth) == MAC_BW_CHANGE) {
        /* 只会做带宽切换，如果有信道切换需要调用dmac_chan_multi_select_channel_mac接口 */
        hmac_chan_select_channel_mac(hmac_vap, hmac_vap->channel.chan_number, mac_vap_get_cap_bw(hmac_vap));

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_PS_STOP_CHECK);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_ps_stop_check_and_notify_cb)fhook)(hmac_vap->hal_device);
    }

        oam_warning_log3(0, OAM_SF_ASSOC,
            "vap_id[%d] {hmac_sta_set_bandwith_handler::change BW. channel[%d], bandwidth[%d].}", hmac_vap->vap_id,
            hmac_vap->channel.chan_number, sta_new_bandwidth);
    }

    if (hmac_vap->ch_switch_info.wait_bw_change == OSAL_TRUE) {
        hmac_vap->ch_switch_info.wait_bw_change = OSAL_FALSE;
    }

    return OAL_SUCC;
}

osal_void hmac_sta_post_bw_verify_switch(const hmac_vap_stru *hmac_vap,
    hmac_sta_bw_switch_type_enum_enum_uint8 verify_reslt)
{
    if (verify_reslt == HMAC_STA_BW_VERIFY_20M_TO_40M) {
        /* 带宽切换状态机 */
        hmac_sta_bw_switch_fsm_post_event(hmac_vap, HMAC_STA_BW_SWITCH_EVENT_BEACON_40M, 0, OSAL_NULL);
    }

    if (verify_reslt == HMAC_STA_BW_VERIFY_40M_TO_20M) {
        hmac_sta_bw_switch_fsm_post_event(hmac_vap, HMAC_STA_BW_SWITCH_EVENT_BEACON_20M, 0, OSAL_NULL);
    }

    return;
}

OSAL_STATIC osal_u32 hmac_sta_up_rx_beacon_check_change(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, osal_u16 msg_len, osal_u8 stream_change)
{
    osal_u32 change_flag = MAC_NO_CHANGE;
    osal_u8 *payload = oal_netbuf_rx_data(netbuf);
    osal_void *fhook = OSAL_NULL;

    /* user自身空间流能力 */
#ifdef _PRE_WLAN_FEATURE_M2S
    wlan_nss_enum_uint8 spatial_stream = hmac_user->user_num_spatial_stream;
#endif
    unref_param(stream_change);
    payload += MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /* 有non_erp站点时使用long slottime */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        hmac_sta_update_slottime(hmac_vap, hmac_user, payload, msg_len);
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    /* 是否需要OBSS扫描 */
    hmac_mgmt_obss_scan_notify(hmac_vap);
#endif

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);
    if (fhook == OSAL_NULL || ((hmac_sta_csa_fsm_is_waiting_cb)fhook)(hmac_vap) == OSAL_FALSE) {
        /* 处理HT 相关信息元素 */
        change_flag |= hmac_sta_up_update_ht_params(hmac_vap, payload, msg_len, hmac_user);

        /* 处理 VHT 相关信息元素 */
        change_flag |= hmac_sta_up_update_vht_params(hmac_vap, payload, msg_len, hmac_user);

#ifdef _PRE_WLAN_FEATURE_11AX
        fhook = hmac_get_feature_fhook(HMAC_FHOOK_SR_UPDATE_IE_INFO);
        if (fhook != OSAL_NULL) {
            ((hmac_sta_up_process_spatial_reuse_ie_cb)fhook)(hmac_vap, payload, msg_len, OSAL_FALSE);
        }

        change_flag |= hmac_sta_up_update_he_oper_params(hmac_vap, payload, msg_len, hmac_user);
#endif
    }

    /* 保护模式相关 */
    hmac_sta_up_process_erp_ie(payload, msg_len, hmac_user);

    hmac_sta_up_update_protection_mode(hmac_vap, hmac_user);

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 对于STA收到beacon帧之后，user的smps能力被刷新之后，需要重新更新空间流能力并通知算法 */
    if (!is_vap_single_nss(hmac_vap) && !is_user_single_nss(hmac_user)) {
        hmac_check_smps_field(hmac_vap, payload, msg_len, hmac_user);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 当vap是双空间流时，处理Operating Mode Notification 信息元素 */
    change_flag |= hmac_rx_beacon_check_opmode_notify(hmac_vap, payload, msg_len, hmac_user);
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 对端AP assoc rx方向的vht能力可能根据我们刷成了双流，此处up之后需要对空间流能力进行校正 */
    hmac_user->user_num_spatial_stream = hmac_m2s_get_bss_max_nss(hmac_vap, netbuf, msg_len, OSAL_TRUE);
    if ((spatial_stream != hmac_user->user_num_spatial_stream) || (stream_change == OSAL_TRUE)) {
        oam_warning_log4(0, OAM_SF_M2S, "vap_id[%d] {hmac_sta_up_rx_beacon_check_change::nss[%d] to [%d] change[%d].}",
            hmac_vap->vap_id, spatial_stream, hmac_user->user_num_spatial_stream, stream_change);
        /* user空间流信息变化，同步至device */
        hmac_user_sync(hmac_user);
    }
#endif

    return change_flag;
}

/*****************************************************************************
 功能描述  : 解析beacon后发现bandwidth需要改变的处理
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_rx_beacon_check_bw_change(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u32 change_flag)
{
    osal_u8 verify_reslt;
    wlan_bw_cap_enum_uint8 user_bw_cap;
    wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth;
    osal_u32 ret;

    sta_new_bandwidth = hmac_vap_get_ap_usr_opern_bandwidth(hmac_vap, hmac_user);

    /* opmode带宽发生变化，已更新在user的avail_bandwidth中,此修改仅用于过OPMODE认证 */
    if ((change_flag & MAC_BW_OPMODE_CHANGE) != 0) {
        sta_new_bandwidth = hmac_user->avail_bandwidth;
    }

    user_bw_cap = hmac_vap_bw_mode_to_bw(sta_new_bandwidth);
    verify_reslt = hmac_sta_bw_switch_need_new_verify(hmac_vap, user_bw_cap);
    /* 20M、40M带宽校验，tx=20M&rx=40M */
    if (verify_reslt == HMAC_STA_BW_VERIFY_20M_TO_40M) {
        user_bw_cap = WLAN_BW_CAP_20M;
    } else if (verify_reslt == HMAC_STA_BW_VERIFY_40M_TO_20M) {
        user_bw_cap = WLAN_BW_CAP_20M;
        sta_new_bandwidth = mac_vap_get_cap_bw(hmac_vap);
    }

    oam_warning_log4(0, OAM_SF_ASSOC,
        "{hmac_sta_up_rx_beacon_check_bw_change::change BW.change[0x%x] verify_reslt[%d] cap[%d] sta_bandwidth[%d].}",
        change_flag, verify_reslt, user_bw_cap, sta_new_bandwidth);

    ret = hmac_sta_set_bandwith_handler(hmac_vap, sta_new_bandwidth);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_ASSOC, "{hmac_sta_up_rx_beacon_check_bw_change::set bw handler FAIL}");
    }
    hmac_user_set_bandwith_handler(hmac_vap, hmac_user, user_bw_cap);

    /* 带宽切换 */
    hmac_sta_post_bw_verify_switch(hmac_vap, verify_reslt);
}

OSAL_STATIC osal_void hmac_sta_up_rx_beacon_check_bw(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    osal_u32 flag, oal_netbuf_stru *netbuf)
{
    osal_u32 change_flag = flag;
    wlan_channel_bandwidth_enum_uint8 sta_new_bandwidth;
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_rx_ctl_stru *rx_info = (mac_rx_ctl_stru *)(&(rx_ctl->rx_info));
    osal_u8 *payload = oal_netbuf_rx_data(netbuf);
    osal_u16 msg_len = mac_get_rx_cb_payload_len(rx_info); /* 帧体长度 */
    osal_u16 ie_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_STA_FSM_IS_WAITING);

    /* csa处理过程不跟随带宽变化 */
    if (fhook != OSAL_NULL && ((hmac_sta_csa_fsm_is_waiting_cb)fhook)(hmac_vap) == OSAL_TRUE) {
        return;
    }

    payload += ie_offset;
    msg_len -= ie_offset;

    if (mac_vap_bw_fsm_beacon_avail(hmac_vap) == OSAL_TRUE) {
        change_flag |= hmac_vap_check_ap_usr_opern_bandwidth(hmac_vap, hmac_user);
        if (((MAC_VHT_CHANGE | MAC_HT_CHANGE | MAC_BW_DIFF_AP_USER | MAC_BW_OPMODE_CHANGE) & change_flag) != 0) {
            hmac_sta_up_rx_beacon_check_bw_change(hmac_vap, hmac_user, change_flag);
        } else { /* 带宽未改变,检查notify channel width的action帧是否通知过算法改变带宽 */
            if (hmac_vap->ch_switch_info.wait_bw_change == OSAL_TRUE) {
                hmac_vap->ch_switch_info.wait_bw_change = OSAL_FALSE;
                /* Aciton notify通知改变，但是实际带宽未改变，需要切回来 */
                sta_new_bandwidth = hmac_vap_get_ap_usr_opern_bandwidth(hmac_vap, hmac_user);
                hmac_user_set_bandwith_handler(hmac_vap, hmac_user, hmac_vap_bw_mode_to_bw(sta_new_bandwidth));
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_sta_up_rx_beacon
 功能描述  : 判断beacon帧是否需要上报HMAC
*****************************************************************************/
osal_u32 hmac_sta_up_rx_beacon_check(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, osal_u8 *go_on)
{
    osal_u16 beacon_period, ie_offset;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u8 stream_change;
    osal_u32 change_flag;
    dmac_rx_ctl_stru *rx_ctl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    mac_ieee80211_frame_stru *frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(rx_ctl->rx_info));
    osal_u8 *payload = oal_netbuf_rx_data(netbuf);
    osal_u16 msg_len = mac_get_rx_cb_payload_len((mac_rx_ctl_stru *)(&(rx_ctl->rx_info))); /* 帧体长度 */

    *go_on = OSAL_FALSE;

    /* 来自其它bss的Beacon不做处理 */
    if (oal_compare_mac_addr(hmac_vap->bssid, frame_hdr->address3) != 0) {
        return OAL_SUCC;
    }

    /* 检查beacon周期是否改变 */
    beacon_period = oal_make_word16(payload[MAC_TIME_STAMP_LEN], payload[MAC_TIME_STAMP_LEN + 1]);
    /* dbac场景暂时不更新，待补充 */
    if (beacon_period != mac_mib_get_beacon_period(hmac_vap) && (beacon_period != 0)) {
        oam_warning_log3(0, OAM_SF_FRAME_FILTER, "vap_id[%d]{hmac_sta_up_rx_beacon:bcn period changed old[%d] new[%d]",
            hmac_vap->vap_id, mac_mib_get_beacon_period(hmac_vap), beacon_period);
        mac_mib_set_beacon_period((hmac_vap_stru *)hmac_vap, beacon_period);
        hmac_sta_align_tbtt_timer(hmac_vap, OSAL_FALSE);
#ifdef _PRE_WLAN_FEATURE_DBAC
        hmac_vap_restart_dbac(hmac_vap);
#endif
    }

    ie_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    if (msg_len > ie_offset) {
        payload += ie_offset;
        msg_len -= ie_offset;
    } else {
        oam_warning_log3(0, 0, "vap_id[%d] {hmac_sta_up_rx_beacon: rx beacon msg_len[%d] <= ie_offset[%d].",
            hmac_vap->vap_id, msg_len, ie_offset);
        return OSAL_FALSE;
    }

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(rx_ctl->rx_info.ta_user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_sta_up_rx_beacon::hmac_user[%d] null}",
            hmac_vap->vap_id, rx_ctl->rx_info.ta_user_idx);
        return OSAL_FALSE;
    }

    stream_change = OSAL_FALSE;
    change_flag = hmac_sta_up_rx_beacon_check_change(hmac_vap, hmac_user, netbuf, msg_len, stream_change);

    hmac_sta_up_rx_beacon_check_bw(hmac_vap, hmac_user, change_flag, netbuf);
    if (hmac_sta_edca_is_changed(hmac_vap, payload, msg_len) == OSAL_TRUE) {
        *go_on = OSAL_TRUE;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_set_protection_mode
 功能描述  : 设置vap的保护类型
*****************************************************************************/
osal_u32 hmac_set_protection_mode(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    wlan_prot_mode_enum_uint8 prot_mode)
{
    osal_u32 ret = OAL_SUCC;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_set_protection_mode::hmac_vap null.}", hmac_vap->vap_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 相同的保护模式已经被设置，直接返回 */
    if (prot_mode == hmac_vap->protection.protection_mode) {
        return ret;
    }

    /* 关闭之前的保护模式 */
    if (hmac_vap->protection.protection_mode == WLAN_PROT_HT) {
        ret = hmac_protection_set_ht_protection(hmac_vap, hmac_user, EXT_SWITCH_OFF);
        if (ret != OAL_SUCC) {
            return ret;
        }
    } else {
        /* ERP保护需要更改RTS的速率，在RTS模块单独处理，GF保护和无保护无需额外操作 */
    }

    hmac_vap->protection.protection_mode = prot_mode;

    /* 开启新的保护模式 */
    if (prot_mode == WLAN_PROT_HT) {
        ret = hmac_protection_set_ht_protection(hmac_vap, hmac_user, EXT_SWITCH_ON);
        if (ret != OAL_SUCC) {
            return ret;
        }
    } else {
        /* ERP保护需要更改RTS的速率，在RTS模块单独处理, GF保护和无保护无需额外操作 */
    }

    /* 更新保护模式后 同步ERP相关信息至device */
    hmac_protection_sync(hmac_vap, prot_mode);

    /* 只有AP需要的操作 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 保护老化定时器的开启和关闭 */
        if ((prot_mode == WLAN_PROT_ERP) || (prot_mode == WLAN_PROT_HT)) {
            /* 启动OBSS保护老化定时器 */
            hmac_protection_start_timer(hmac_vap);
        } else {
            /* 关闭OBSS保护老化定时器 */
            hmac_protection_stop_timer(hmac_vap);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
函 数 名  : hmac_sta_protection_mode_is_changed
功能描述  : STA接收到beacon帧后,检查protection_mode是否改变
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_update_protection_mode(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user)
{
    osal_u32 ret;
    wlan_prot_mode_enum_uint8 protection_mode;

    /* 如果保护机制不启用,直接返回 */
    if (hmac_vap_protection_autoprot_is_enabled_etc(hmac_vap) == EXT_SWITCH_OFF) {
        return;
    }

    protection_mode = mac_vap_get_user_protection_mode_etc(hmac_vap, hmac_user);

    ret = hmac_set_protection_mode(hmac_vap, hmac_user, protection_mode);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_sta_up_update_protection_mode::ret:%u}", hmac_vap->vap_id, ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_protection_update_mib_ap
 功能描述  : 更新AP保护相关mib量
*****************************************************************************/
osal_u32 hmac_protection_update_mib_ap(hmac_vap_stru *hmac_vap)
{
    mac_protection_stru *protection = OSAL_NULL;
    oal_bool_enum_uint8 protect_activated;
    oal_bool_enum_uint8 rifs_mode = OSAL_FALSE;
    wlan_mib_ht_protection_enum_uint8 ht_protection = WLAN_MIB_HT_PROTECTION_BUTT;
    wlan_mib_ht_protection_enum_uint8 old_en_ht_protection;

    protection = &(hmac_vap->protection);

    /* 更新vap的en_dot11LSIGTXOPFullProtectionActivated字段 */
    protect_activated =
        ((hmac_vap->user_nums != 0) && (protection->sta_no_lsig_txop_num == 0)) ? OSAL_TRUE : OSAL_FALSE;

    mac_mib_set_lsig_txop_full_protection_activated(hmac_vap, protect_activated);

    /* 更新vap的en_dot11HTProtection和en_dot11RIFSMode字段 */
    /* The HT Protection field may be set to 20 MHz protection mode only if the following are true:
     * All STAs detected (by any means) in the primary channel and all the secondary channel are HT STAs
     * and all STAs that are memberThis BSS is a 20/40 MHz BSS,
     * and There is at least one 20 MHz HT STA associated with this BSS.
     */
    if ((protection->obss_non_erp_present == OSAL_FALSE) && (protection->obss_non_ht_present == OSAL_FALSE) &&
        (protection->sta_non_ht_num == 0) && (protection->sta_non_erp_num == 0) &&
        (hmac_vap->channel.en_bandwidth != WLAN_BAND_WIDTH_20M) && (protection->sta_20m_only_num != 0)) {
        ht_protection = WLAN_MIB_HT_20MHZ_PROTECTION;
        rifs_mode = OSAL_TRUE;
    /* The HT Protection field may be set to nonmember protection mode only if the following are true:
     * A non-HT STA is detected (by any means) in either the primary or the secondary channel or in both
     * the primary and secondary channels, that is not known by the transmitting STA to be a member of
     * this BSS, and
     * All STAs that are known by the transmitting STA to be a member of this BSS are HT STAs.
     */
    } else if ((protection->obss_non_ht_present == OSAL_TRUE) && (protection->sta_non_ht_num == 0) &&
        (protection->sta_non_erp_num == 0)) {
        ht_protection = WLAN_MIB_HT_NONMEMBER_PROTECTION;
        rifs_mode = OSAL_FALSE;
    /* The HT Protection field may be set to no protection mode only if the following are true:
     * All STAs detected (by any means) in the primary or the secondary channel are HT STAs, and
     * All STAs that are known by the transmitting STA to be a member of this BSS are either
     * 20/40 MHz HT STAs in a 20/40 MHz BSS, or
     * 20 MHz HT STAs in a 20 MHz BSS.
     */
    } else if ((protection->obss_non_erp_present == OSAL_FALSE) && (protection->obss_non_ht_present == OSAL_FALSE) &&
        (protection->sta_non_ht_num == 0) && (protection->sta_non_erp_num == 0)) {
        if ((hmac_vap->channel.en_bandwidth == WLAN_BAND_WIDTH_20M) || (protection->sta_20m_only_num == 0)) {
            ht_protection = WLAN_MIB_HT_NO_PROTECTION;
            rifs_mode = OSAL_TRUE;
        }
    } else {
        ht_protection = WLAN_MIB_HT_NON_HT_MIXED;
        rifs_mode = OSAL_FALSE;
    }

    old_en_ht_protection = mac_mib_get_ht_protection(hmac_vap);
    if (old_en_ht_protection != ht_protection) {
        mac_mib_set_ht_protection(hmac_vap, ht_protection);
        mac_mib_set_rifs_mode(hmac_vap, rifs_mode);
    }

    return hmac_protection_update_mode_ap(hmac_vap);
}

/*****************************************************************************
 函 数 名  : hmac_protection_update_mode_ap
 功能描述  : 根据mac vap结构体统计值更新AP保护模式
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_protection_update_mode_ap(hmac_vap_stru *hmac_vap)
{
    wlan_prot_mode_enum_uint8 protection_mode;
    mac_protection_stru *protection = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    protection = &(hmac_vap->protection);

    /* 如果保护机制不启用， 直接返回 */
    if (hmac_vap_protection_autoprot_is_enabled_etc(hmac_vap) == EXT_SWITCH_OFF) {
        return OAL_SUCC;
    }

    /* 在2G频段下，如果有non erp站点与AP关联， 或者OBSS中存在non erp站点， 设置为erp保护 */
    if ((hmac_vap->channel.band == WLAN_BAND_2G) &&
        ((protection->sta_non_erp_num != 0) || (protection->obss_non_erp_present == OSAL_TRUE))) {
        protection_mode = WLAN_PROT_ERP;
    /* 如果有non ht站点与AP关联， 或者OBSS中存在non ht站点， 设置为ht保护 */
    } else if ((protection->sta_non_ht_num != 0) || (protection->obss_non_ht_present == OSAL_TRUE)) {
        protection_mode = WLAN_PROT_HT;
    } else if (protection->sta_non_gf_num != 0) { /* 如果有non gf站点与AP关联， 设置为gf保护 */
        protection_mode = WLAN_PROT_GF;
    } else { /* 剩下的情况不做保护 */
        protection_mode = WLAN_PROT_NO;
    }

    /* 设置具体保护模式 */
    return hmac_set_protection_mode(hmac_vap, OSAL_NULL, protection_mode);
}

/*****************************************************************************
 函 数 名  : hmac_protection_obss_aging_ap
 功能描述  : OBSS老化处理， 本函数定期执行(5000ms 执行一次)
*****************************************************************************/
OSAL_STATIC osal_void hmac_protection_obss_aging_ap(hmac_vap_stru *hmac_vap)
{
    osal_bool em_update_protection = OSAL_FALSE; /* 指示是否需要更新vap的protection */

    /* 更新ERP老化计数 */
    if (hmac_vap->protection.obss_non_erp_present == OSAL_TRUE) {
        hmac_vap->protection.obss_non_erp_aging_cnt++;
        if (hmac_vap->protection.obss_non_erp_aging_cnt >= WLAN_PROTECTION_NON_ERP_AGING_THRESHOLD) {
            hmac_vap->protection.obss_non_erp_present = OSAL_FALSE;
            em_update_protection = OSAL_TRUE;
            hmac_vap->protection.obss_non_erp_aging_cnt = 0;
        }
    }

    /* 更新HT老化计数 */
    if (hmac_vap->protection.obss_non_ht_present == OSAL_TRUE) {
        hmac_vap->protection.obss_non_ht_aging_cnt++;

        if (hmac_vap->protection.obss_non_ht_aging_cnt >= WLAN_PROTECTION_NON_HT_AGING_THRESHOLD) {
            hmac_vap->protection.obss_non_ht_present = OSAL_FALSE;
            em_update_protection = OSAL_TRUE;
            hmac_vap->protection.obss_non_ht_aging_cnt = 0;
        }
    }

    /* 需要更新保护模式 */
    if (em_update_protection == OSAL_TRUE) {
        hmac_protection_update_mib_ap(hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_protection_obss_update_timer
 功能描述  : OBSS老化处理， 本函数定期对VAP进行保护老化处理(5000ms 执行一次)
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_protection_obss_update_timer(osal_void *arg)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;

    if (arg == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_protection_obss_update_timer::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)arg;

    hmac_protection_obss_aging_ap(hmac_vap);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_protection_start_timer
 功能描述  : 启动OBSS老化处理定时器
*****************************************************************************/
OSAL_STATIC osal_void hmac_protection_start_timer(hmac_vap_stru *hmac_vap)
{
    /* 启动OBSS保护老化定时器 定时器已开启，则不用再开启 */
    if (hmac_vap->obss_aging_timer.is_registerd == OSAL_FALSE) {
        frw_create_timer_entry(&(hmac_vap->obss_aging_timer), hmac_protection_obss_update_timer,
            WLAN_USER_AGING_TRIGGER_TIME, hmac_vap, OSAL_TRUE);
    }
}

/*****************************************************************************
 函 数 名  : hmac_protection_stop_timer
 功能描述  : 停止OBSS老化处理定时器
*****************************************************************************/
osal_void hmac_protection_stop_timer(hmac_vap_stru *hmac_vap)
{
    /* 关闭OBSS保护老化定时器 定时器已关闭，则不用再关闭 */
    if (hmac_vap->obss_aging_timer.is_registerd == OSAL_FALSE) {
        return;
    }
    frw_destroy_timer_entry(&(hmac_vap->obss_aging_timer));
}

/*****************************************************************************
 函 数 名  : hmac_ap_is_olbc_present
 功能描述  : 判断obss中是否存在non_ERP站点存在
*****************************************************************************/
OSAL_STATIC osal_bool hmac_ap_is_olbc_present(osal_u8 *payload_data, osal_u32 len)
{
    osal_u32 payload_len = len;
    osal_u8 *payload = payload_data;
    osal_u8 num_rates = 0;
    mac_erp_params_stru *erp_params = OSAL_NULL;
    osal_u8 *ie = OSAL_NULL;

    if (payload_len <= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ap_is_olbc_present::payload_len[%d]}", payload_len);
        return OSAL_FALSE;
    }

    payload_len -= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    payload += (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    ie = mac_find_ie_etc(MAC_EID_ERP, payload, (osal_s32)payload_len);
    if (ie != OSAL_NULL) {
        erp_params = (mac_erp_params_stru *)(ie + MAC_IE_HDR_LEN);
        /* 如果use protection被置为1， 返回TRUE */
        if (erp_params->non_erp == OSAL_TRUE) {
            return OSAL_TRUE;
        }
    }

    ie = mac_find_ie_etc(MAC_EID_RATES, payload, (osal_s32)payload_len);
    if (ie != OSAL_NULL) {
        num_rates += ie[1];
    }

    ie = mac_find_ie_etc(MAC_EID_XRATES, payload, (osal_s32)payload_len);
    if (ie != OSAL_NULL) {
        num_rates += ie[1];
    }

    /* 如果基本速率集数目小于或等于11b协议支持的最大速率集个数， 返回TRUE */
    if (num_rates <= MAC_NUM_DR_802_11B) {
        oam_info_log1(0, OAM_SF_ANY, "{hmac_ap_is_olbc_present::invalid num_rates[%d].}", num_rates);
        return OSAL_TRUE;
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_ap_process_obss_erp_ie
 功能描述  : 处理obss来的beacon帧/probe response帧中的ERP信息元素
*****************************************************************************/
OSAL_STATIC osal_void hmac_ap_process_obss_erp_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload, osal_u32 payload_len)
{
    /* 存在non erp站点 */
    if (hmac_ap_is_olbc_present(payload, payload_len) == OSAL_TRUE) {
        hmac_vap->protection.obss_non_erp_present = OSAL_TRUE;
        hmac_vap->protection.obss_non_erp_aging_cnt = 0;
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_is_obss_non_ht_present
 功能描述  : 判断obss中是否存在non_ht站点存在
*****************************************************************************/
OSAL_STATIC osal_bool hmac_ap_is_obss_non_ht_present(osal_u8 *payload_data, osal_u32 len)
{
    mac_ht_opern_stru *ht_operation = OSAL_NULL;
    osal_u8 *ie = OSAL_NULL;
    osal_u32 payload_len = len;
    osal_u8 *payload = payload_data;

    if (payload_len <= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ap_is_olbc_present::payload_len[%d]}", payload_len);
        return OSAL_FALSE;
    }

    payload_len -= (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);
    payload += (MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN);

    ie = mac_find_ie_etc(MAC_EID_HT_OPERATION, payload, (osal_s32)payload_len);
    if (ie != OSAL_NULL) {
        ht_operation = (mac_ht_opern_stru *)(ie + MAC_IE_HDR_LEN);
        if (ht_operation->obss_nonht_sta_present == OSAL_TRUE) {
            /* 如果OBSS Non-HT STAs Present被置为1， 返回TRUE */
            return OSAL_TRUE;
        }
    }

    ie = mac_find_ie_etc(MAC_EID_HT_CAP, payload, (osal_s32)payload_len);
    if (ie != OSAL_NULL) {
        /* 如果有HT capability信息元素，返回FALSE */
        return OSAL_FALSE;
    }

    /* 如果没有HT capability信息元素，返回TRUE */
    return OSAL_TRUE;
}

/*****************************************************************************
 函 数 名  : hmac_ap_process_obss_ht_operation_ie
 功能描述  : 处理obss来的beacon帧/probe response帧中的ht信息元素
*****************************************************************************/
OSAL_STATIC osal_void hmac_ap_process_obss_ht_operation_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    osal_u32 payload_len)
{
    /* 如果存在non-ht站点 */
    if (hmac_ap_is_obss_non_ht_present(payload, payload_len) == OSAL_TRUE) {
        hmac_vap->protection.obss_non_ht_present = OSAL_TRUE;
        hmac_vap->protection.obss_non_ht_aging_cnt = 0;
    }
}

/*****************************************************************************
 函 数 名  : hmac_ap_up_rx_obss_beacon
 功能描述  : 处理obss来的beacon帧/probe response帧
*****************************************************************************/
osal_void hmac_ap_up_rx_obss_beacon(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf)
{
    osal_u8 *payload = OSAL_NULL;
    osal_u16 payload_len;
    /* 获取帧体指针 */
    payload = oal_netbuf_rx_data(netbuf);
    /* 获取帧体长度 */
    payload_len = (osal_u16)oal_netbuf_rx_frame_len(netbuf);

    /* 处理ERP相关 */
    if (hmac_vap->channel.band == WLAN_BAND_2G) {
        hmac_ap_process_obss_erp_ie(hmac_vap, payload, payload_len);
    }

    /* 处理HT operation相关 */
    hmac_ap_process_obss_ht_operation_ie(hmac_vap, payload, payload_len);

    /* 更新AP中保护相关mib量 */
    hmac_protection_update_mib_ap(hmac_vap);

    /* 判断是否需要切20M */
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
    if (hmac_vap->cap_flag.autoswitch_2040 == OSAL_TRUE) {
        if ((mac_mib_get_2040_switch_prohibited(hmac_vap) == OSAL_FALSE)) {
            hmac_ap_handle_40_intol(hmac_vap, payload, payload_len);
            hmac_ap_process_obss_bandwidth(hmac_vap, payload, payload_len);
        }
    }
#endif
}

/*****************************************************************************
 函 数 名  : hmac_init_dtim_count_ap
 功能描述  : 初始化设置DTIM count为DTIM period -1;次函数需在beacon alloc时，
             encap beacon前调用，否则会造成 Beacon 的第一帧的DTIM count 不正常
*****************************************************************************/
OSAL_STATIC osal_void hmac_init_dtim_count_ap(hmac_vap_stru *hmac_vap)
{
    if (mac_mib_get_dot11_dtim_period(hmac_vap) == 0) {
        oam_error_log1(0, OAM_SF_WIFI_BEACON,
            "vap_id[%d] {hmac_init_dtim_count_ap::DTIM period error!}", hmac_vap->vap_id);
        return;
    }
    hmac_vap->dtim_count = (osal_u8)(mac_mib_get_dot11_dtim_period(hmac_vap) - 1);
}

/*****************************************************************************
 函 数 名  : hmac_update_dtim_count_ap
 功能描述  : 在tbtt中断中，更新dtim count，保证每次tbtt中断来，dtim count自动减1
*****************************************************************************/
OSAL_STATIC osal_void hmac_update_dtim_count_ap(hmac_vap_stru *hmac_vap)
{
    /* 将vap中保存的dtim_count值赋给DTIM Count字段，每次都会减1，在区间[0,DTIM_period]中循环 */
    if (hmac_vap->dtim_count == 0) {
        hmac_vap->dtim_count = (osal_u8)(mac_mib_get_dot11_dtim_period(hmac_vap) - 1);
    } else {
        hmac_vap->dtim_count--;
    }
}

/*****************************************************************************
 函 数 名  : hmac_set_tim_ie
 功能描述  : vap启动时以及每次tbtt中断，ap把本地的tim_bitmap拷贝到beacon帧里的TIM信息元素空间
*****************************************************************************/
OSAL_STATIC osal_void hmac_set_tim_ie(const hmac_vap_stru *hmac_vap, osal_u8 *buffer, osal_u8 *ie_len)
{
    osal_u8       tim_offset;
    osal_u8       pv_bitmap_len;

    if (hmac_vap == OSAL_NULL || hmac_vap->tim_bitmap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_WIFI_BEACON, "{hmac_set_tim_ie::hmac_vap is null!}");
        return;
    }

    /***************************************************************************
     ---------------------------------------------------------------------------
     |EID |Len |DTIM Count |DTIM Period |Bitmap Control |Partial Virtual Bitmap|
     ---------------------------------------------------------------------------
     |1   |1   |1          |1           |1              |1~251                 |
     ---------------------------------------------------------------------------
    ***************************************************************************/
    pv_bitmap_len = hmac_vap->tim_bitmap[0];
    tim_offset    = 2 + (hmac_vap->tim_bitmap[1] & (osal_u8)(~BIT0)); /* 2表示头部的长度 */
    if (pv_bitmap_len + tim_offset > hmac_vap->tim_bitmap_len) {
        oam_error_log2(0, OAM_SF_WIFI_BEACON, "{hmac_set_tim_ie::tim_bitmap = 0x%x bitmap_len[%d] .}",
            oal_ntoh_32(*(osal_u32 *)(hmac_vap->tim_bitmap)), hmac_vap->tim_bitmap_len);
        pv_bitmap_len = 1;
    }

    buffer[0] = MAC_EID_TIM; /* 0表示帧信息元素第0位EID */
    buffer[1] = pv_bitmap_len + MAC_TIM_LEN_EXCEPT_PVB; /* 1表示帧信息元素第1位length */
    buffer[2] = hmac_vap->dtim_count; /* 2表示帧信息元素第2位DTIM Count */
    buffer[3] = (osal_u8)mac_mib_get_dot11_dtim_period(hmac_vap); /* 0表示帧信息元素3位DTIM Period */
    buffer[4] = hmac_vap->tim_bitmap[1]; /* 4表示帧信息元素4位Bitmap Control */

    /* 5表示指向Partial Virtual Bitmap，将vap本地的bitmap不为0的部分复制到beacon帧相应空间，作为Partial Virtual Bitmap */
    (osal_void)memcpy_s(&buffer[5], pv_bitmap_len, hmac_vap->tim_bitmap + tim_offset, pv_bitmap_len);

    *ie_len = MAC_IE_HDR_LEN + MAC_TIM_LEN_EXCEPT_PVB + pv_bitmap_len;
}

/*****************************************************************************
 函 数 名  : hmac_beacon_free
 功能描述  : 释放beacon帧内存
*****************************************************************************/
osal_u32  hmac_beacon_free(hmac_vap_stru *hmac_vap)
{
    oal_netbuf_free(hmac_vap->beacon_buffer);
    hmac_vap->beacon_buffer = OSAL_NULL;

    /* 关闭tsf */
    hal_disable_tsf_tbtt(hmac_vap->hal_vap);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_sta_edca_is_changed
 功能描述  : 判断beacon帧中EDCA ie相对于上一个beacon帧是否发生改变
*****************************************************************************/
OSAL_STATIC oal_bool_enum_uint8 hmac_sta_edca_is_changed(hmac_vap_stru *hmac_vap, osal_u8 *frame_body,
    osal_u16 frame_len)
{
    osal_u8         param_set_cnt;
    osal_u8         modify_flag = OSAL_FALSE;
    const osal_u8   *edca_ie;

    edca_ie = hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM, frame_body, frame_len);
    if (edca_ie == OSAL_NULL) {
        return OSAL_FALSE;
    }

    if (hmac_is_wmm_ie_etc((osal_u8*)edca_ie) == OSAL_TRUE) {
        param_set_cnt = edca_ie[HMAC_WMM_QOS_PARAMS_HDR_LEN] & 0x0F;

        if (param_set_cnt != hmac_vap->uc_wmm_params_update_count) {
            oam_warning_log4(0, OAM_SF_FRAME_FILTER,
                "vap_id[%d] hmac_sta_edca_is_changed::edca is changed=%d, old_cnt=%d new_cnt=%d\r\n", hmac_vap->vap_id,
                modify_flag, hmac_vap->uc_wmm_params_update_count, param_set_cnt);

            modify_flag = OSAL_TRUE;
        }
    }

    return modify_flag;
}

/*****************************************************************************
 函 数 名  : hmac_sta_update_slottime
 功能描述  : 在有non_erp站点时更新slottime
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_update_slottime(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 *payload, osal_u16 msg_len)
{
    mac_erp_params_stru    *erp_ie_para = OSAL_NULL;
    const osal_u8            *erp_ie = OSAL_NULL;

    unref_param(hmac_user);
    /* 保存non_erp信息,sta模式下，保存BSS中是否有erp站点的信息 */
    erp_ie = mac_find_ie_etc(MAC_EID_ERP, payload, msg_len);
    if (erp_ie == OSAL_NULL) {
        return;
    }

    /* IE长度异常，不更新slottime类型 */
    if (erp_ie[1] < MAC_ERP_IE_LEN) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_sta_update_slottime::invalid erp ie len[%d].}",
            hmac_vap->vap_id, erp_ie[1]);
        return;
    }

    /* NON_ERP站点进出时更新slottime类型 */
    erp_ie_para = (mac_erp_params_stru*)&(erp_ie[MAC_IE_HDR_LEN]);
    if (hmac_vap->non_erp_exist != erp_ie_para->non_erp) {
        hmac_vap->non_erp_exist = erp_ie_para->non_erp;
        /* 有NON_ERP站点时，使用short slottime发数据；否则使用long slottime */
        hal_cfg_slottime_type(erp_ie_para->non_erp);
    }
}
/*****************************************************************************
 函 数 名  : hmac_sta_up_process_erp_ie
 功能描述  : STA接收到beacon帧后,检查ERP信息元素是否改变
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_process_erp_ie(osal_u8 *payload, osal_u16 msg_len, hmac_user_stru *hmac_user)
{
    mac_erp_params_stru    *erp_params = OSAL_NULL;
    osal_u8              *erp_ie  = OSAL_NULL;

    /************************ ERP Element *************************************
    --------------------------------------------------------------------------
    |EID  |Len  |NonERP_Present|Use_Protection|Barker_Preamble_Mode|Reserved|
    --------------------------------------------------------------------------
    |B0-B7|B0-B7|B0            |B1            |B2                  |B3-B7   |
    --------------------------------------------------------------------------
    ***************************************************************************/
    erp_ie = mac_find_ie_etc(MAC_EID_ERP, payload, msg_len);
    if (erp_ie == OSAL_NULL) {
        return;
    }

    /* IE长度校验 */
    if (erp_ie[1] < MAC_ERP_IE_LEN) {
        oam_warning_log1(0, OAM_SF_RX, "{hmac_sta_up_process_erp_ie::invalid erp ie len[%d].}", erp_ie[1]);
        return;
    }

    erp_params = (mac_erp_params_stru *)(erp_ie + MAC_IE_HDR_LEN);

    if (hmac_user->cap_info.barker_preamble_mode != erp_params->preamble_mode) {
        oam_warning_log2(0, OAM_SF_FRAME_FILTER, "dmac_sta_is_erp_ie_changed::ap new barker mode[%d],defalut[%d]",
                         erp_params->preamble_mode, hmac_user->cap_info.barker_preamble_mode);

        /* 保存preamble mode */
        hmac_user_set_barker_preamble_mode_etc(hmac_user, erp_params->preamble_mode);
    }

    /* 检查erp保护是否改变 */
    if (hmac_user->cap_info.erp_use_protect != erp_params->use_protection) {
        oam_warning_log2(0, OAM_SF_FRAME_FILTER, "dmac_sta_is_erp_ie_changed::ap new erp protection[%d],defalut[%d]",
                         erp_params->use_protection, hmac_user->cap_info.erp_use_protect);

        hmac_user->cap_info.erp_use_protect = erp_params->use_protection;
    }
}
/*****************************************************************************
 函 数 名  : hmac_protection_set_ht_protection
 功能描述  : 设置ht保护是否开启
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_protection_set_ht_protection(hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    osal_u8 flag)
{
    osal_u32      ret = OAL_SUCC;
    osal_bool   lsigtxop;

    lsigtxop = mac_protection_lsigtxop_check_etc(hmac_vap, hmac_user);
    /* 优先使用lsigtxop保护，开销小 */
    if (lsigtxop == OSAL_TRUE) {
        mac_protection_set_lsig_txop_mechanism_etc(hmac_vap, flag);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_sta_stop_tbtt_timer
 功能描述  : 停止host TBTT定时器
*****************************************************************************/
osal_void hmac_sta_stop_tbtt_timer(hmac_vap_stru *hmac_vap)
{
    /* 关闭host TBTT定时器 定时器已关闭，则不用再关闭 */
    if (hmac_vap->sta_tbtt.tbtt_timer.is_registerd == OSAL_FALSE) {
        return;
    }
    frw_destroy_timer_entry(&(hmac_vap->sta_tbtt.tbtt_timer));
}

/*****************************************************************************
 函 数 名  : hmac_sta_align_tbtt_timer
 功能描述  : TBTT定时器对齐中断
*****************************************************************************/
osal_void hmac_sta_align_tbtt_timer(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 need_addition)
{
    /* TBTT定时器对齐中断 定时器已关闭，则不用再对齐 */
    if (hmac_vap->sta_tbtt.tbtt_timer.is_registerd == OSAL_FALSE) {
        return;
    }
    frw_timer_restart_timer(&hmac_vap->sta_tbtt.tbtt_timer, mac_mib_get_beacon_period(hmac_vap), OSAL_TRUE);

    if (need_addition == OSAL_FALSE) {
        return;
    }
    hmac_tbtt_event_sta((osal_void *)hmac_vap);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
