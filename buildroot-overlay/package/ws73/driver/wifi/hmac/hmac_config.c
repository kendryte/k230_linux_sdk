/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: 配置相关实现hmac接口实现源文件.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#ifndef _PRE_OS_VERSION_LINUX
#error _PRE_OS_VERSION_LINUX undefined !
#endif

#ifndef _PRE_OS_VERSION
#error _PRE_OS_VERSION undefined !
#endif

#ifndef _PRE_OS_VERSION_LITEOS
#error _PRE_OS_VERSION_LITEOS undefined !
#endif

#ifndef _PRE_TEST_MODE
#error _PRE_TEST_MODE undefined !
#endif

#ifndef _PRE_TEST_MODE_UT
#error _PRE_TEST_MODE_UT undefined !
#endif

#include "hmac_config.h"
#include "hmac_ccpriv.h"
#include "oal_mem_hcm.h"
#include "oam_struct.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_device.h"
#include "mac_resource_ext.h"
#include "hmac_resource.h"
#include "mac_vap_ext.h"
#include "mac_ie.h"
#include "mac_frame_common_rom.h"
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
#include "hcc_host_if.h"
#endif
#include "mac_user_ext.h"
#include "mac_regdomain.h"
#include "hmac_ext_if.h"
#include "hmac_fsm.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_chan_mgmt.h"
#include "hmac_rx_filter.h"
#include "hmac_psm_ap.h"
#include "hmac_protection.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_data_acq.h"
#include "hmac_rx_filter.h"
#include "hmac_mgmt_sta.h"
#include "hmac_feature_dft.h"
#include "hmac_ampdu_config.h"
#include "hmac_keep_alive.h"
#ifdef _PRE_WLAN_DFR_STAT
#include "hmac_dfr.h"
#endif
#include "hmac_sme_sta.h"
#include "hmac_blacklist.h"

#include "hmac_m2u.h"

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif
#include "hmac_mbo.h"
#include "hmac_scan.h"
#include "hmac_dfs.h"
#include "hmac_reset.h"
#include "hmac_scan.h"
#include "hmac_blockack.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hmac_p2p.h"
#endif
#include "hmac_mgmt_ap.h"
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "plat_firmware.h"
#include "hmac_sample_daq.h"
#include "hmac_sample_daq_phy.h"
#include "hal_soc_reg.h"
#include "hal_mac.h"
#endif
#include "hmac_roam_if.h"

#ifdef _PRE_WLAN_TCP_OPT
#include "mac_data.h"
#include "hmac_tcp_opt.h"
#endif

#include "plat_pm_wlan.h"
#include "hmac_dfx.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#include "fe_hal_equipment.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "fe_rf_customize_power_cali.h"

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
#include "hmac_wds.h"
#endif

#include "hmac_chan_meas.h"

#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "hmac_txopps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#include "hmac_obss_ap.h"
#endif
#include "hmac_sr_sta.h"
#include "wlan_msg.h"
#include "hmac_feature_main.h"
#include "common_dft.h"
#include "mac_device_ext.h"

#include "hmac_feature_dft.h"

#ifdef _PRE_WLAN_FEATURE_DFX_CHR
#include "hmac_chr.h"
#endif

#include "hmac_psm_sta.h"

#include "hmac_uapsd.h"
#include "oal_netbuf_data.h"
#include "hmac_tx_mpdu_adapt.h"
#include "hmac_alg_notify.h"
#include "hal_mac.h"
#include "hmac_beacon.h"
#include "hmac_scan.h"

#ifdef _PRE_WLAN_FEATURE_PMF
#include "hmac_11w.h"
#endif
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
#include "hmac_ant_switch.h"
#endif
#include "hmac_power.h"
#include "hmac_psm_ap.h"
#include "hmac_sta_pm.h"
#include "hmac_tx_mpdu_queue.h"
#include "hmac_thruput_test.h"
#include "hmac_cali_mgmt.h"
#include "frw_util_notifier.h"
#include "hmac_11i.h"
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
#include "alg_anti_interference.h"
#endif
#include "hmac_psd.h"
#include "hmac_promisc.h"
#include "hal_ext_if.h"

#include "hmac_sdp.h"
#include "hmac_al_tx_rx.h"

#ifdef _PRE_BSLE_GATEWAY
#include "hmac_bsle.h"
#endif
#ifdef _PRE_WLAN_ONLINE_CALI
#include "fe_extern_if_host.h"
#endif
#include "hal_phy.h"
#include "msg_device_manage_rom.h"
#include "hmac_version.h"
#include "hmac_wapi.h"
#include "hmac_feature_interface.h"
#include "hmac_11r.h"
#include "hal_device.h"
#if defined(_PRE_WLAN_FEATURE_CENTRALIZE) && defined(BOARD_FPGA_WIFI)
#include "fe_hal_rf_if.h"
#endif
#include "cali_rf_temp_code.h"
#include "hal_mfg.h"
#include "hal_rf.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CONFIG_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define HMAC_HIPRIV_CMD_NAME_MAX_LEN  16                             /* 字符串中每个单词的最大长度 */
#define HMAC_MAX_VERSION_SIZE         200

#define HMAC_INVALID_POWER      (-128)
#define HMAC_INVALID_PS_STATE   2           /* 非法的省功耗状态 */
#define HMAC_CUSTOM_PKT_SEND_TIME 200

#define MAC_CTS_FRAME_LEN 10
#define MAC_CONTROL_FRAME_LEN 16
typedef osal_void  (*hmac_set_dscr_func)(osal_s32, osal_u8, hmac_vap_stru*);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
typedef struct {
    wlan_protocol_enum_uint8   protocol_mode;                /* wid枚举 */
    osal_u8                  auc_resv[3];
    osal_s8                  *protocol_desc;
} hmac_protocol_stru;

frw_timeout_stru g_custom_pkt_send_timer = {0};

osal_s32  hmac_config_set_freq_etc(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_u32  hmac_config_set_mode_etc(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param);
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
static osal_s32 hmac_config_get_fast_result(hmac_vap_stru *hmac_vap, frw_msg *msg);
static osal_s32 hmac_config_reset_fast_result(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
OAL_STATIC osal_s32 hmac_config_mgmt_report_switch(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
OSAL_STATIC osal_s32 hmac_config_sta_rcv_probe_req_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param);
OAL_STATIC osal_u8 g_sta_rcv_probe_req_enable = OSAL_FALSE;
OSAL_STATIC osal_s32 hmac_config_sta_get_conn_status_code(hmac_vap_stru *hmac_vap, const osal_s8 *param);

/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_config_sta_update_rates_etc
 功能描述  : 按照指定的协议模式更新VAP速率集
 输入参数  : hmac_vap : 指向vap
             cfg_mode: 协议模式相关参数
 输出参数  : 无
 OAL_SUCC 或其它错误码
*****************************************************************************/
osal_u32  hmac_config_sta_update_rates_etc(hmac_vap_stru *hmac_vap, mac_cfg_mode_param_stru *cfg_mode,
    mac_bss_dscr_stru *bss_dscr)
{
    osal_u32                    ul_ret;
    hmac_device_stru              *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_sta_update_rates_etc::hmac_device null.}",
                       hmac_vap->vap_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_sta_update_rates_etc::the vap has been deleted.}",
                         hmac_vap->vap_id);

        return OAL_FAIL;
    }

    if (cfg_mode->protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_FALSE);
    }

    hmac_vap_init_by_protocol_etc(hmac_vap, cfg_mode->protocol);
    hmac_vap->channel.band = cfg_mode->band;
    /* vap协议能力赋值后，重新依据对端AP能力刷新带宽能力 */
    if (bss_dscr != OSAL_NULL) {
        hmac_vap->channel.en_bandwidth = hmac_vap_get_bandwith(hmac_vap,
            bss_dscr->channel_bandwidth);
    } else {
        hmac_vap->channel.en_bandwidth = cfg_mode->en_bandwidth;
    }
#ifdef _PRE_WIFI_DMT
    hmac_vap_init_rates_etc(hmac_vap);
#else
    mac_sta_init_bss_rates_etc(hmac_vap, bss_dscr);
#endif

    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ul_ret = hmac_config_start_vap_event_etc(hmac_vap, OAL_FALSE);
    if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_sta_update_rates_etc::hmac_config_send_event_etc failed[%d].}",
                         hmac_vap->vap_id, ul_ret);
        hmac_vap_init_by_protocol_etc(hmac_vap, hmac_vap->preset_para.protocol);
        hmac_vap->channel.band      = hmac_vap->preset_para.band;
        hmac_vap->channel.en_bandwidth = hmac_vap->preset_para.en_bandwidth;
        return ul_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_bss_type_etc
 功能描述  : 设置bss type
 输入参数  : hmac_vap: 指向VAP的指针
             len     : 参数长度
             param  : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_bss_type_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* 设置mib值 */
    hmac_mib_set_bss_type_etc(hmac_vap, (osal_u8)msg->data_len, msg->data);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_bss_type_etc
 功能描述  : 获取bss type
 输入参数  : hmac_vap: 指向vap的指针
 输出参数  : pus_len    : 参数长度
             param  : 参数
*****************************************************************************/
osal_s32 hmac_config_get_bss_type_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* 读取mib值 */
    return (osal_s32)hmac_mib_get_bss_type_etc(hmac_vap, (osal_u8 *)&msg->rsp_len, msg->rsp);
}

/*****************************************************************************
 函 数 名  : hmac_config_get_mode_etc
 功能描述  : 获取模式 包括协议 频段 带宽
 输入参数  : hmac_vap: 指向vap的指针
 输出参数  : pus_len    : 参数长度
             param  : 参数
*****************************************************************************/
osal_s32  hmac_config_get_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_mode_param_stru prot_param;

    prot_param.protocol  = hmac_vap->protocol;
    prot_param.band      = hmac_vap->channel.band;
    prot_param.en_bandwidth = hmac_vap->channel.en_bandwidth;

    if (memcpy_s(msg->rsp, sizeof(prot_param), &prot_param, sizeof(prot_param)) != EOK) {
        oam_warning_log0(0, OAM_SF_TX, "hmac_config_get_mode_etc::memcpy_s err!");
        return OAL_FAIL;
    }
    msg->rsp_len = (osal_u16)OAL_SIZEOF(mac_cfg_mode_param_stru);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_check_bw_and_band(hmac_vap_stru *hmac_vap,
    const mac_cfg_mode_param_stru *prot_param, const hmac_device_stru *hmac_device)
{
    if ((hmac_vap_bw_mode_to_bw(prot_param->en_bandwidth) >= WLAN_BW_CAP_80M) &&
        (mac_mib_get_dot11_vap_max_bandwidth(hmac_vap) < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_check_mode_param::not support 80MHz bandwidth,protocol=%d protocol_cap=%d.}",
            prot_param->en_bandwidth, mac_mib_get_dot11_vap_max_bandwidth(hmac_vap));
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if ((prot_param->band == WLAN_BAND_5G) && (hmac_device->band_cap == WLAN_BAND_CAP_2G)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_check_mode_param::not support 5GHz band,protocol=%d protocol_cap=%d.}",
            prot_param->band, hmac_device->band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((prot_param->band == WLAN_BAND_2G) && (hmac_device->band_cap == WLAN_BAND_CAP_5G)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_check_mode_param::not support 2GHz band,protocol=%d protocol_cap=%d.}",
            prot_param->band, hmac_device->band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_config_check_mode_param
 功能描述  : 设置mode时，协议 频段 带宽参数检查
 输入参数    hmac_device: device结构体
             prot_param: prot_param配置命令下发的参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_config_check_mode_param(hmac_vap_stru *hmac_vap, mac_cfg_mode_param_stru *prot_param)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;

    /* 获取device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_check_mode_param::hmac_device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 根据device能力对参数进行检查 */
    switch (prot_param->protocol) {
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11B_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            break;

        case WLAN_HT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            if (hmac_device->protocol_cap < WLAN_PROTOCOL_CAP_HT) {
                /* 设置11n协议，但device不支持HT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_check_mode_param::not support HT mode,protocol=%d protocol_cap=%d.}",
                    prot_param->protocol, hmac_device->protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            if (hmac_device->protocol_cap < WLAN_PROTOCOL_CAP_VHT) {
                /* 设置11ac协议，但device不支持VHT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_check_mode_param::not support VHT mode,protocol=%d protocol_cap=%d.}",
                    prot_param->protocol, hmac_device->protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
            if (hmac_device->protocol_cap < WLAN_PROTOCOL_CAP_HE) {
                /* 设置11aX协议，但device不支持HE模式 */
                oam_error_log2(0, OAM_SF_CFG,
                               "{hmac_config_check_mode_param::not support HE mode,protocol=%d protocol_cap=%d.}",
                               prot_param->protocol, hmac_device->protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;
#endif

        default:
            /* 就目前来说，不会出现其他MODE的情况 */
            oam_error_log0(0, OAM_SF_CFG, "{hmac_config_check_mode_param::mode param does not in the list.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    return hmac_config_check_bw_and_band(hmac_vap, prot_param, hmac_device);
}

/*****************************************************************************
 函 数 名  : hmac_set_mode_check_bandwith
 功能描述  : 非首次设置带宽时根据已配置带宽检查新配置带宽参数
           取消新配置的带宽必须小于device当前的带宽模式的限制，只要保证主信道一致即可
 输入参数  : bw_device: 首次配置的带宽
             bw_config: 本次配置命令配置的带宽
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_config_set_mode_check_bandwith(
    wlan_channel_bandwidth_enum_uint8 bw_device,
    wlan_channel_bandwidth_enum_uint8 bw_config)
{
    /* 要配置带宽是20M */
    if (bw_config == WLAN_BAND_WIDTH_20M) {
        return OAL_SUCC;
    }

    /* 要配置带宽与首次配置带宽相同 */
    if (bw_device == bw_config) {
        return OAL_SUCC;
    }

    switch (bw_device) {
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            if (bw_config == WLAN_BAND_WIDTH_40PLUS) {
                return OAL_SUCC;
            }
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            if (bw_config == WLAN_BAND_WIDTH_40MINUS) {
                return OAL_SUCC;
            }
            break;

        case WLAN_BAND_WIDTH_40PLUS:
            if ((bw_config == WLAN_BAND_WIDTH_80PLUSPLUS) || (bw_config == WLAN_BAND_WIDTH_80PLUSMINUS)) {
                return OAL_SUCC;
            }
            break;
        case WLAN_BAND_WIDTH_40MINUS:
            if ((bw_config == WLAN_BAND_WIDTH_80MINUSPLUS) || (bw_config == WLAN_BAND_WIDTH_80MINUSMINUS)) {
                return OAL_SUCC;
            }
            break;
        case WLAN_BAND_WIDTH_20M:
            return OAL_SUCC;

        default:
            break;
    }

    return OAL_FAIL;
}

OAL_STATIC osal_u32 hmac_config_mode_check(hmac_vap_stru *hmac_vap,
    mac_cfg_mode_param_stru *prot_param, hmac_device_stru *hmac_device)
{
    osal_u32 ret;
    /* 检查配置参数是否在device能力内 */
    ret = hmac_config_check_mode_param(hmac_vap, prot_param);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_mode_check::hmac_config_check_mode_param failed[%d].}",
                         hmac_vap->vap_id, ret);
        return ret;
    }

    /* device已经配置时，需要校验下频段、带宽是否一致 */
    if ((hmac_device->max_bandwidth != WLAN_BAND_WIDTH_BUTT) && (mac_dbac_enable(hmac_device) == 0) &&
        (hmac_device->vap_num > 1)) {
        if (hmac_device->max_band != prot_param->band) {
            oam_warning_log3(0, OAM_SF_CFG,
                             "vap_id[%d] {hmac_config_mode_check::previous vap band[%d] mismatch with [%d].}",
                             hmac_vap->vap_id, hmac_device->max_band,  prot_param->band);
            return OAL_FAIL;
        }

        ret = hmac_config_set_mode_check_bandwith(hmac_device->max_bandwidth, prot_param->en_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log4(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_config_mode_check::vap_id[%d],previous vap bandwidth[%d, current[%d].}",
                hmac_vap->vap_id, ret, hmac_device->max_bandwidth, prot_param->en_bandwidth);
            return ret;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_config_mode_param_update(hmac_vap_stru *hmac_vap, mac_cfg_mode_param_stru *prot_param,
    hmac_device_stru *hmac_device)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

    if ((hmac_vap->cap_flag.support_11ac2g == OAL_FALSE) &&
        (hmac_vap->protocol == WLAN_VHT_MODE) &&
        (hmac_vap->channel.band == WLAN_BAND_2G)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_mode_etc::11ac2g not supported.");
        prot_param->protocol = WLAN_HT_MODE;
    }

    if ((hmac_vap->channel.band == WLAN_BAND_2G) &&
        (hmac_vap->cap_flag.disable_2ght40 == OAL_TRUE) &&
        (prot_param->en_bandwidth != WLAN_BAND_WIDTH_20M)) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_mode_etc::2ght40 not supported.");
        prot_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
#endif

    /* 更新STA协议配置标志位 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_vap->preset_para.protocol  = prot_param->protocol;
        hmac_vap->preset_para.en_bandwidth = prot_param->en_bandwidth;
        hmac_vap->preset_para.band      = prot_param->band;
    }

    /* 记录协议模式, band, bandwidth到mac_vap下 */
    hmac_vap->protocol  = prot_param->protocol;
    hmac_vap->max_protocol = hmac_get_protocol_mode_by_wlan_protocol(prot_param->protocol);
    hmac_vap->channel.band = prot_param->band;
    hmac_vap->channel.en_bandwidth = prot_param->en_bandwidth;
    hmac_vap->ch_switch_info.user_pref_bandwidth = prot_param->en_bandwidth;

    oam_warning_log4(0, OAM_SF_CFG,
        "vap_id[%d] {hmac_config_set_mode_etc::protocol=%d, band=%d, bandwidth=%d.}", hmac_vap->vap_id,
        prot_param->protocol, hmac_vap->channel.band, hmac_vap->channel.en_bandwidth);

    /* 根据协议更新vap能力 */
    hmac_vap_init_by_protocol_etc(hmac_vap, prot_param->protocol);

    /* 更新device的频段及最大带宽信息 */
    if ((hmac_device->max_bandwidth == WLAN_BAND_WIDTH_BUTT) || (hmac_calc_up_ap_num_etc(hmac_device) == 0)) {
        hmac_device->max_bandwidth = prot_param->en_bandwidth;
        hmac_device->max_band      = prot_param->band;
    }
    return;
}

osal_s32 hmac_config_set_mode_cb(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_mib_by_bw_param_stru cfg;
    hmac_device_stru *mac_device = OAL_PTR_NULL;
    mac_cfg_channel_param_stru chan_info;
    wlan_channel_bandwidth_enum cfg_bw;
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    if (!hmac_vap || !(msg->data)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_mode_cb::null ptr, vap=%p param=%p",
            (uintptr_t)hmac_vap, (uintptr_t)(msg->data));
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    hmac_device_get_channel_etc(mac_device, &chan_info);
    cfg_bw = ((mac_cfg_mode_param_stru *)msg->data)->en_bandwidth;
    if (((mac_cfg_mode_param_stru *)msg->data)->band == WLAN_BAND_2G) {
        if (cfg_bw == WLAN_BAND_WIDTH_40M) {
            if (chan_info.channel > hmac_chan_get_bw_40m_max_chan_idx()) {
                ((mac_cfg_mode_param_stru *)msg->data)->en_bandwidth = WLAN_BAND_WIDTH_40MINUS;
            } else {
                ((mac_cfg_mode_param_stru *)msg->data)->en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
            }
        } else if (((cfg_bw == WLAN_BAND_WIDTH_40PLUS) && (chan_info.channel > hmac_chan_get_bw_40m_max_chan_idx())) ||
                   ((cfg_bw == WLAN_BAND_WIDTH_40MINUS) && (chan_info.channel < HMAC_BW_40M_MIN_CHAN_IDX))) {
            oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_mode_cb::invlaid_param, chan=%d, bw=%d",
                chan_info.channel, cfg_bw);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }
    cfg.band = ((mac_cfg_mode_param_stru *)msg->data)->band;
    cfg.en_bandwidth = ((mac_cfg_mode_param_stru *)msg->data)->en_bandwidth;
    msg_info.data = (osal_u8 *)&cfg;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(cfg);
    hmac_config_set_mib_by_bw(hmac_vap, &msg_info);

    return (osal_s32)hmac_config_set_mode_etc(hmac_vap, msg->data_len, msg->data);
}

/*****************************************************************************
 函 数 名  : hmac_config_set_mode_etc
 功能描述  : 设置模式 包括协议 频段 带宽
 输入参数  : mac_vap: 指向VAP的指针
             len     : 参数长度
             param  : 参数
 输出参数  : 无
 OAL_SUCC 或其它错误码
 修改内容   : 函数整改，hmac配置流程应该是
                        1、参数校验并设置hmac vap下的信息
                        2、设置mac vap下的信息及mib信息
                        3、设置mac device下的信息
                        4、抛配置事件到dmac
*****************************************************************************/
osal_u32  hmac_config_set_mode_etc(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    mac_cfg_mode_param_stru *prot_param = OAL_PTR_NULL;
    osal_u32 ret;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    mac_cfg_mode_param_stru old_param;
    unref_param(len);

    /* 记录老配置 */
    old_param.protocol  = hmac_vap->protocol;
    old_param.band      = hmac_vap->channel.band;
    old_param.en_bandwidth = hmac_vap->channel.en_bandwidth;

    /* 获取device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_mode_etc::mac_device null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 设置模式时，device下必须至少有一个vap */
    if (hmac_device->vap_num == 0) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_mode_etc::no vap in device.}",
                         hmac_vap->vap_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    prot_param = (mac_cfg_mode_param_stru *)param;

    /* 检查配置参数是否在device能力内 */
    ret = hmac_config_mode_check(hmac_vap, prot_param, hmac_device);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_set_mode_etc::hmac_config_mode_check failed[%d].}",
                         hmac_vap->vap_id, ret);
        return ret;
    }

    if (prot_param->protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_FALSE);
    }

    hmac_config_mode_param_update(hmac_vap, prot_param, hmac_device);

    ret = (osal_u32)hmac_config_set_mode(hmac_vap, (const mac_cfg_mode_param_stru *)&old_param);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_mode_etc::hmac_config_set_mode failed[%d].}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_mac_addr_etc
 功能描述  : 设置协议模式
 输入参数  : event_hdr:事件头
             param    :参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_mac_addr_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_staion_id_param_stru *param_temp = (mac_cfg_staion_id_param_stru *)msg->data;

    if ((hmac_vap->mib_info == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_set_mac_addr_etc::vap->mib_info or data is NULL !}",
                       hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 设置mib值, Station_ID */
    mac_mib_set_station_id_etc(hmac_vap, (osal_u8)msg->data_len, msg->data);

    oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d] hmac_config_set_mac_addr_etc::station[%x],[%x]!",
        hmac_vap->vap_id, param_temp->station_id[0], param_temp->station_id[4]); /* 地址的第4位 */

    /* hal设置mac地址 */
    hal_vap_set_macaddr(hmac_vap->hal_vap, param_temp->station_id, WLAN_MAC_ADDR_LEN);
    hmac_vap_sync(hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_ssid_etc
 功能描述  : hmac读SSID
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32 hmac_config_get_ssid_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* ~{6AH!~}mib~{V5~} */
    return (osal_s32)hmac_mib_get_ssid_etc(hmac_vap, (osal_u8 *)&msg->rsp_len, msg->rsp);
}

/*****************************************************************************
 函 数 名  : hmac_config_set_ssid_etc
 功能描述  : hmac设SSID
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_ssid_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* ~{IhVC~}mib~{V5~} */
    hmac_mib_set_ssid_etc(hmac_vap, (osal_u8)msg->data_len, msg->data);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_shpreamble_etc
 功能描述  : 设置短前导码能力位
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_shpreamble_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* 设置mib值 */
    hmac_mib_set_shpreamble_etc(hmac_vap, (osal_u8)msg->data_len, msg->data);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_shpreamble_etc
 功能描述  : 读前导码能力位
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32 hmac_config_get_shpreamble_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* 读取mib值 */
    return (osal_s32)hmac_mib_get_shpreamble_etc(hmac_vap, (osal_u8 *)&msg->rsp_len, msg->rsp);
}

/*****************************************************************************
 函 数 名  : hmac_config_set_shortgi20_etc
 功能描述  : 20M short gi能力设置
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_shortgi20_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 l_value;

    l_value = *((osal_s32 *)msg->data);
    if (l_value != 0) {
        mac_mib_set_short_gi_option_in_twenty_implemented(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_short_gi_option_in_twenty_implemented(hmac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_shortgi40_etc
 功能描述  : 设置40M short gi能力
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_shortgi40_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 l_value;

    l_value = *((osal_s32 *)msg->data);
    if (l_value != 0) {
        mac_mib_set_short_gi_option_in_forty_implemented(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_short_gi_option_in_forty_implemented(hmac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_shortgi80_etc
 功能描述  : 设置80M short gi能力
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_shortgi80_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32       l_value;

    l_value = *((osal_s32 *)msg->data);

    if (l_value != 0) {
        mac_mib_set_vht_short_gi_option_in80_implemented(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_vht_short_gi_option_in80_implemented(hmac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_shortgi20_etc
 功能描述  : 读取short gi
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_shortgi20_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 value;

    value = mac_mib_get_short_gi_option_in_twenty_implemented(hmac_vap);

    *((osal_s32 *)msg->rsp) = value;

    msg->rsp_len = (osal_u16)OAL_SIZEOF(value);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_shortgi40_etc
 功能描述  : 读取short gi
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_shortgi40_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 value;

    value = (osal_s32)mac_mib_get_short_gi_option_in_forty_implemented(hmac_vap);

    *((osal_s32 *)msg->rsp) = value;

    msg->rsp_len = (osal_u16)OAL_SIZEOF(value);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_shortgi80_etc
 功能描述  : 读取short gi
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_shortgi80_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 value;

    value = (osal_s32)mac_mib_get_vht_short_gi_option_in80_implemented(hmac_vap);

    *((osal_s32 *)msg->rsp) = value;

    msg->rsp_len = (osal_u16)OAL_SIZEOF(value);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_auth_mode_etc
 功能描述  : 设置认证模式
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_auth_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_mib_set_authentication_mode(hmac_vap, *msg->data);
#ifdef _PRE_WLAN_FEATURE_DFR
    hmac_dfr_fill_ap_recovery_info(WLAN_MSG_W2H_CFG_AUTH_MODE, msg->data, hmac_vap);
#endif
    oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_auth_mode_etc::set auth mode[%d] succ.}",
                     hmac_vap->vap_id, mac_mib_get_authentication_mode(hmac_vap));
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_auth_mode_etc
 功能描述  : 读取认证模式
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_auth_mode_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    *((osal_s32 *)msg->rsp) = mac_mib_get_authentication_mode(hmac_vap);
    msg->rsp_len = (osal_u16)OAL_SIZEOF(osal_s32);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_dtimperiod_etc
 功能描述  : 设置dtim period
*****************************************************************************/
osal_s32 hmac_config_set_dtimperiod_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* 设置mib值 */
    hmac_mib_set_dtim_period_etc(hmac_vap, (osal_u8)msg->data_len, msg->data);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_dtimperiod_etc
 功能描述  : 读取dtim period
*****************************************************************************/
osal_s32 hmac_config_get_dtimperiod_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    /* 读取mib值 */
    return (osal_s32)hmac_mib_get_dtim_period_etc(hmac_vap, (osal_u8 *)&msg->rsp_len, msg->rsp);
}

/*****************************************************************************
 函 数 名  : hmac_config_set_nobeacon_etc
 功能描述  : 设置no beacon
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_nobeacon_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 l_value;

    l_value = *((osal_s32 *)msg->data);
    hmac_vap->no_beacon = (osal_u8)l_value;

    if (hmac_vap->hal_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_nobeacon_etc::hmac_vap null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (l_value != 0) {
        hal_vap_beacon_suspend(hmac_vap->hal_vap);
    } else {
        hal_vap_beacon_resume(hmac_vap->hal_vap);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_nobeacon_etc
 功能描述  : 读取no beacon
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_nobeacon_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    *((osal_s32 *)msg->rsp) = hmac_vap->no_beacon;
    msg->rsp_len = (osal_u16)OAL_SIZEOF(osal_s32);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_txpower_etc
 功能描述  : 设置发送功率
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
 修改内容   : 设置功率精度到0.1dbm
*****************************************************************************/
osal_s32 hmac_config_set_txpower_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 value;
    osal_s32 ret = OAL_SUCC;

    value = (*((osal_s32 *)msg->data) < 0) ? 0 : (*((osal_s32 *)msg->data));
    hmac_vap_set_tx_power_etc(hmac_vap, (osal_u8)value);

    ret = hmac_config_set_txpower(hmac_vap, (osal_u8)value);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_set_txpower_etc::hmac_config_set_txpower failed[%d]}",
            hmac_vap->vap_id, ret);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_txpower_etc
 功能描述  : 读取发送功率
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_txpower_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    *((osal_s32 *)msg->rsp) = hmac_vap->tx_power;
    msg->rsp_len = (osal_u16)OAL_SIZEOF(osal_s32);

    return OAL_SUCC;
}
osal_s32 hmac_config_set_rate_power_offset(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        return OAL_FAIL;
    }
    return hmac_power_set_rate_power_offset(hmac_vap, msg);
}
osal_s32 hmac_config_set_cal_tone(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (msg == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        return OAL_FAIL;
    }
    hal_set_cal_tone((*(osal_u32 *)(msg->data)));
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_config_set_backoff_delay(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 back_off_time;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_backoff_delay:: mac_vap or msg is Null .}");
        return OAL_FAIL;
    }

    back_off_time = *(osal_u32 *)msg->data;
    hal_set_mac_backoff_delay(back_off_time);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_set_freq_prepro(hmac_vap_stru *hmac_vap, osal_u8 channel)
{
    osal_u32 ret;

    ret = hmac_is_channel_num_valid_etc(hmac_vap->channel.band, channel);
    if (ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_freq_prepro::hmac_is_channel_num_valid_etc[%d] failed[%d].}",
            hmac_vap->vap_id, channel, ret);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 信道14特殊处理，只在11b协议模式下有效 */
    if ((channel == 14) && (hmac_vap->protocol != WLAN_LEGACY_11B_MODE)) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_freq_prepro::channel-14 available in 11bgn/ax, curr protocol=%d.}",
            hmac_vap->vap_id, hmac_vap->protocol);
    }
#endif
    hmac_vap->channel.chan_number = channel;
    ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band,
        channel, &(hmac_vap->channel.chan_idx));
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_set_freq_prepro::hmac_get_channel_idx_from_num_etc fail.band[%u] channel[%u]!}",
            hmac_vap->channel.band, hmac_vap->channel.chan_idx);
        return ret;
    }
    return OAL_CONTINUE;
}


/*****************************************************************************
 函 数 名  : hmac_config_set_freq_etc
 功能描述  : 设置频率
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_freq_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ul_ret;
    osal_u8 channel = *(msg->data);
    mac_cfg_channel_param_stru l_channel_param;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id); /* 获取device */

    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_freq_etc::hmac_device null.}",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_config_set_freq_prepro(hmac_vap, channel);
    if (ul_ret != OAL_CONTINUE) {
        return (osal_s32)ul_ret;
    }

    /* 非DBAC时，首次配置信道时设置到硬件 */
    if ((hmac_device->vap_num == 1) || (hmac_device->max_channel == 0)) {
        hmac_device_get_channel_etc(hmac_device, &l_channel_param);
        l_channel_param.channel = channel;
        hmac_device_set_channel_etc(hmac_device, &l_channel_param);
        /***************************************************************************
            抛事件到DMAC层, 同步DMAC数据
        ***************************************************************************/
        ul_ret = (osal_u32)hmac_config_set_freq(hmac_vap, channel);
        if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_config_set_freq_etc::hmac_config_set_freq failed[%d].}",
                hmac_vap->vap_id, ul_ret);
            return (osal_s32)ul_ret;
        }
#ifdef _PRE_WLAN_FEATURE_DBAC
    } else if (mac_is_dbac_enabled(hmac_device)) {
        /***************************************************************************
           抛事件到DMAC层, 同步DMAC数据
        ***************************************************************************/
        ul_ret = hmac_config_set_freq(hmac_vap, channel);
        if (OAL_UNLIKELY(ul_ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_config_set_freq_etc::hmac_config_set_freq failed[%d].}",
                hmac_vap->vap_id, ul_ret);
            return (osal_s32)ul_ret;
        }

        oam_info_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_freq_etc::do not check channel while DBAC enabled.}",
            hmac_vap->vap_id);
#endif
    } else {
        if (hmac_device->max_channel != channel) {
            oam_warning_log3(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_config_set_freq_etc::previous vap channel number=%d mismatch [%d].}",
                hmac_vap->vap_id, hmac_device->max_channel, channel);

            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_freq_etc
 功能描述  : 读取频率
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32  hmac_config_get_freq_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    msg->rsp_len = (osal_u16)OAL_SIZEOF(osal_u32);

    *((osal_u32 *)msg->rsp) = hmac_vap->channel.chan_number;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_channel_check_param
 功能描述  : 设置Channnel时，协议 频段 带宽参数检查
 输入参数  : hmac_device: device结构体
             prot_param: prot_param配置命令下发的参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_config_set_channel_check_param(hmac_vap_stru *hmac_vap,
    mac_cfg_channel_param_stru *prot_param)
{
    hmac_device_stru   *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_set_channel_check_param::hmac_device null,divice_id=%d.}",
                       hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据device能力对参数进行检查 */
    if ((prot_param->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
        (mac_mib_get_dot11_vap_max_bandwidth(hmac_vap) < WLAN_BW_CAP_80M)) {
        /* 设置80M带宽，但device能力不支持80M，返回错误码 */
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_channel_check_param::not support 80MHz bandwidth,protocol=%d MaxBW=%d.}",
            hmac_vap->vap_id, prot_param->en_bandwidth, mac_mib_get_dot11_vap_max_bandwidth(hmac_vap));
        return OAL_ERR_CODE_CONFIG_BW_EXCEED;
    }

    if ((prot_param->band == WLAN_BAND_5G) && (hmac_device->band_cap == WLAN_BAND_CAP_2G)) {
        /* 设置5G频带，但device不支持5G */
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_set_channel_check_param::not support 5GHz band,protocol=%d protocol_cap=%d.}",
            prot_param->band, hmac_device->band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    } else if ((prot_param->band == WLAN_BAND_2G) && (hmac_device->band_cap == WLAN_BAND_CAP_5G)) {
        /* 设置2G频带，但device不支持2G */
        oam_warning_log2(0, OAM_SF_CFG,
            "{hmac_config_set_channel_check_param::not support 2GHz band,protocol=%d protocol_cap=%d.}",
            prot_param->band, hmac_device->band_cap);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    /* 若vap不支持2g 40M则，返回不支持该带宽的错误码 */
    if ((prot_param->band == WLAN_BAND_2G) && (prot_param->en_bandwidth > WLAN_BAND_WIDTH_20M) &&
        (mac_mib_get_2GFortyMHzOperationImplemented(hmac_vap) == OAL_FALSE)) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_config_set_mib_by_bw
 功能描述  : 根据带宽设置mib能力
*****************************************************************************/
osal_s32  hmac_config_set_mib_by_bw(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_mib_by_bw_param_stru *cfg = (mac_cfg_mib_by_bw_param_stru *)msg->data;

    if (!hmac_vap || !(msg->data)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_mib_by_bw::nul ptr, vap=%p param=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)(msg->data));
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap_change_mib_by_bandwidth_etc(hmac_vap, cfg->band, cfg->en_bandwidth);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_set_channel_host(hmac_vap_stru *hmac_vap, mac_cfg_channel_param_stru *channel_param)
{
    osal_u32 ret;
#if defined(_PRE_PRODUCT_ID_HOST)
    mac_channel_stru set_mac_channel;

    /* 信道跟随检查 */
    if (is_ap(hmac_vap)) {
        set_mac_channel.band = channel_param->band;
        set_mac_channel.en_bandwidth = channel_param->en_bandwidth;
        set_mac_channel.chan_number = channel_param->channel;
        ret = hmac_check_ap_channel_follow_sta(hmac_vap, &set_mac_channel);
        if (ret == OAL_SUCC) {
            oam_warning_log3(0, OAM_SF_CHAN,
                "vap_id[%d] {hmac_config_set_channel_host::channel from %d change to %d}",
                hmac_vap->vap_id, set_mac_channel.chan_number, set_mac_channel.chan_number);
            channel_param->channel = set_mac_channel.chan_number;
            channel_param->en_bandwidth = set_mac_channel.en_bandwidth;
        }
    }
#endif
    /* 检查配置参数是否在device能力内 */
    ret = hmac_config_set_channel_check_param(hmac_vap, channel_param);
    if (ret == OAL_ERR_CODE_CONFIG_BW_EXCEED) {
        channel_param->en_bandwidth = hmac_vap_get_bandwith(hmac_vap, channel_param->en_bandwidth);
    }

    if (ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
        oam_warning_log2(0, OAM_SF_CHAN,
                         "vap_id[%d] {hmac_config_set_channel_host::hmac_config_set_channel_check_param failed[%d].}",
                         hmac_vap->vap_id, ret);
        return ret;
    }

    /* 该错误码表示不支持2g 40M,故更改带宽为20M */
    if (ret == OAL_ERR_CODE_CONFIG_UNSUPPORT) {
        channel_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
    return ret;
}

OAL_STATIC osal_u32 hmac_config_set_mode_flag(hmac_vap_stru *hmac_vap, mac_cfg_channel_param_stru *channel_param,
    hmac_device_stru *hmac_device, osal_u32 up_vap_cnt, oal_bool_enum_uint8 *set_reg)
{
    oal_bool_enum_uint8 override = OAL_FALSE;
    osal_u32 ret;

#ifdef _PRE_WLAN_FEATURE_DFS
    override |= ((mac_vap_get_dfs_enable(hmac_vap) &&
                     mac_dfs_get_cac_enable(hmac_device))) ? OAL_TRUE : OAL_FALSE;
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    override |= (oal_bool_enum_uint8)(((mac_get_2040bss_switch(hmac_device) != 0) && (hmac_vap->bw_fixed == 0)) ?
        OAL_TRUE : OAL_FALSE);
#endif

#ifdef _PRE_WLAN_FEATURE_DBAC
    override &= !mac_is_dbac_enabled(hmac_device) ? OAL_TRUE : OAL_FALSE;
#endif

    override &= (oal_bool_enum_uint8)((up_vap_cnt > 1) ? OAL_TRUE : OAL_FALSE);

    /* 初始扫描使能时，运行时临时设置不同的信道 */
    if (override) {
        hmac_device_get_channel_etc(hmac_device, channel_param);
        oam_warning_log4(0, OAM_SF_CHAN,
            "vap_id[%d] {hmac_config_set_mode_flag::force chan band=%d ch=%d bw=%d}", hmac_vap->vap_id,
            channel_param->band, channel_param->channel, channel_param->en_bandwidth);
    }

    /* 仅在没有VAP up的情况下，配置硬件频带、带宽寄存器 */
    if (up_vap_cnt <= 1) {
        /* 记录首次配置的带宽值 */
        hmac_device_set_channel_etc(hmac_device, channel_param);

        /***************************************************************************
         抛事件到DMAC层, 配置寄存器  置标志位
        ***************************************************************************/
        *set_reg = OAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_DBAC
    } else if (mac_is_dbac_enabled(hmac_device) == OAL_TRUE) {
#endif
    } else {
        /* 信道不是当前信道 */
        if (hmac_device->max_channel != channel_param->channel) {
            oam_warning_log3(0, OAM_SF_CHAN,
                "vap_id[%d] {hmac_config_set_channel_etc::previous vap channel number=%d mismatch [%d].}",
                hmac_vap->vap_id, hmac_device->max_channel, channel_param->channel);
            return OAL_FAIL;
        }
        /* 带宽不能超出已配置的带宽 */
        ret = hmac_config_set_mode_check_bandwith(hmac_device->max_bandwidth, channel_param->en_bandwidth);
        if (ret != OAL_SUCC) {
            oam_warning_log4(0, OAM_SF_CHAN,
                "vap_id[%d] {hmac_config_set_channel_etc::check_bandwith failed[%d],previous vap bw[%d, current[%d].}",
                hmac_vap->vap_id, ret, hmac_device->max_bandwidth, channel_param->en_bandwidth);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_get_set_reg_value(oal_bool_enum_uint8 *set_reg,
    mac_cfg_channel_param_stru *channel_param, hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;
    osal_u8 vap_idx;
    hmac_vap_stru *hmac_vap_tmp = OAL_PTR_NULL;
    unref_param(set_reg);

#ifdef _PRE_WLAN_FEATURE_DBAC
    if (mac_is_dbac_enabled(hmac_device) == OAL_TRUE) {
        hmac_vap->channel.chan_number     = channel_param->channel;
        hmac_vap->channel.band            = channel_param->band;
        hmac_vap->channel.en_bandwidth    = channel_param->en_bandwidth;

        ret = hmac_get_channel_idx_from_num_etc(channel_param->band,
            channel_param->channel, &(hmac_vap->channel.chan_idx));
        if (ret != OAL_SUCC) {
            oam_warning_log4(0, OAM_SF_CHAN,
                "vap_id[%d] {hmac_get_set_reg_value::get_channel_idx failed[%d], band[%d], channel[%d].}",
                hmac_vap->vap_id, ret, channel_param->band, channel_param->channel);
            return OAL_FAIL;
        }

        *set_reg = OAL_TRUE;
    } else {
#endif

        for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
            hmac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
            if (hmac_vap_tmp == OAL_PTR_NULL) {
                continue;
            }
            hmac_vap_tmp->channel.chan_number     = channel_param->channel;
            hmac_vap_tmp->channel.band            = channel_param->band;
            hmac_vap_tmp->channel.en_bandwidth    = channel_param->en_bandwidth;
            ret = hmac_get_channel_idx_from_num_etc(channel_param->band, channel_param->channel,
                &(hmac_vap->channel.chan_idx));
            if (ret != OAL_SUCC) {
                oam_warning_log4(0, OAM_SF_CHAN,
                    "vap_id[%d] {hmac_get_set_reg_value::get_channel_idx failed[%d], band[%d], channel[%d].}",
                    hmac_vap_tmp->vap_id, ret, channel_param->band, channel_param->channel);
                continue;
            }
        }
#ifdef _PRE_WLAN_FEATURE_DBAC
    }
#endif
    oam_warning_log4(0, OAM_SF_CHAN, "vap_id[%d] hmac_get_set_reg_value: channel_num:%d, bw:%d, band:%d",
                     hmac_vap->vap_id, channel_param->channel, channel_param->en_bandwidth,
                     channel_param->band);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_channel_etc
 功能描述  : HMAC 层设置信道信息
 输入参数  : hmac_vap_stru *mac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_channel_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_channel_param_stru *channel_param = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_s32 ret;
    oal_bool_enum_uint8 set_reg = OAL_FALSE;
    osal_u32 up_vap_cnt = 0;

    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL) || (hmac_device == OAL_PTR_NULL)) {
        oam_warning_log3(0, OAM_SF_CHAN, "{hmac_config_set_channel_etc::null ptr,vap=%p param=%p hmac_device=%p",
            (uintptr_t)hmac_vap, (uintptr_t)(msg->data), (uintptr_t)hmac_device);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_config_set_mib_by_bw(hmac_vap, msg);
#ifdef _PRE_WLAN_FEATURE_DFR
    hmac_dfr_fill_ap_recovery_info(WLAN_MSG_W2H_CFG_CFG80211_SET_CHANNEL, msg->data, hmac_vap);
#endif
    channel_param = (mac_cfg_channel_param_stru *)msg->data;

    ret = (osal_s32)hmac_config_set_channel_host(hmac_vap, channel_param);
    if (ret == OAL_ERR_CODE_CONFIG_EXCEED_SPEC) {
        return ret;
    }

    up_vap_cnt = hmac_calc_up_ap_num_etc(hmac_device);
    ret = (osal_s32)hmac_config_set_mode_flag(hmac_vap, channel_param, hmac_device, up_vap_cnt, &set_reg);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CHAN, "{hmac_config_set_channel_etc::hmac_config_set_mode_flag null,divice_id=%d.}",
            hmac_vap->device_id);
        return ret;
    }

    ret = (osal_s32)hmac_get_set_reg_value(&set_reg, channel_param, hmac_device, hmac_vap);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CHAN, "{hmac_config_set_channel_etc::hmac_get_set_reg_value null,divice_id=%d.}",
            hmac_vap->device_id);
        return ret;
    }
    if (set_reg == OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_CHAN, "vap_id[%d] hmac_config_set_channel_etc: post to register", hmac_vap->vap_id);
        ret = hmac_config_set_channel(hmac_vap, channel_param);
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oam_warning_log2(0, OAM_SF_CHAN, "vap_id[%d] {hmac_config_set_channel_etc::set failed[%d].}",
                hmac_vap->vap_id, ret);
            return ret;
        }
    }
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST_AP
    /* hostapd配置带宽记录信息，同步dmac操作40m恢复定时器 */
    hmac_40m_intol_sync_data(hmac_vap, hmac_vap->channel.en_bandwidth, OAL_FALSE);
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_beacon_check_param
 功能描述  : 设置mode时，协议 频段 带宽参数检查
 输入参数  : hmac_device: device结构体
             prot_param: prot_param配置命令下发的参数
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_config_set_beacon_check_param(hmac_device_stru *hmac_device,
    mac_beacon_param_stru *prot_param)
{
    /* 根据device能力对参数进行检查 */
    switch (prot_param->protocol) {
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11B_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            break;

        case WLAN_HT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
            if (hmac_device->protocol_cap < WLAN_PROTOCOL_CAP_HT) {
                /* 设置11n协议，但device不支持HT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_set_beacon_check_param::not support HT mode,protocol=%d protocol_cap=%d.}",
                    prot_param->protocol, hmac_device->protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            if (hmac_device->protocol_cap < WLAN_PROTOCOL_CAP_VHT) {
                /* 设置11ac协议，但device不支持VHT模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_set_beacon_check_param::not support VHT mode,protocol=%d protocol_cap=%d.}",
                    prot_param->protocol, hmac_device->protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;

#ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
            if (hmac_device->protocol_cap < WLAN_PROTOCOL_CAP_HE) {
                /* 设置11ax协议，但device不支持HE模式 */
                oam_warning_log2(0, OAM_SF_CFG,
                    "{hmac_config_set_beacon_check_param::not support HE mode,protocol=%d protocol_cap=%d.}",
                    prot_param->protocol, hmac_device->protocol_cap);
                return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
            }
            break;
#endif

        default:
            /* 目前不会出现其他MODE的情况 */
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_beacon_check_param::mode param does not in the list.}");
            return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_set_beacon_content(hmac_vap_stru *hmac_vap, mac_beacon_param_stru *beacon_param)
{
    osal_u32 ret;
    if (beacon_param->protocol >= WLAN_HT_MODE) {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_TRUE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_TRUE);
    } else {
        mac_mib_set_TxAggregateActived(hmac_vap, OAL_FALSE);
        mac_mib_set_AmsduAggregateAtive(hmac_vap, OAL_FALSE);
    }

    hmac_vap_set_hide_ssid_etc(hmac_vap, beacon_param->hidden_ssid);

    /* 适配新内核start ap和change beacon接口复用此接口，不同的是change beacon时，不再设置beacon周期
       和dtim周期，因此，change beacon时，interval和dtim period参数为全零，此时不应该被设置到mib中 */
    /* 设置VAP beacon interval， dtim_period */
    if ((beacon_param->dtim_period != 0) || (beacon_param->interval != 0)) {
        mac_mib_set_dot11dtimperiod(hmac_vap, (osal_u32)beacon_param->dtim_period);
        mac_mib_set_beacon_period(hmac_vap, (osal_u32)beacon_param->interval);
    }

    /* 设置short gi */
    mac_mib_set_short_gi_option_in_twenty_implemented(hmac_vap, beacon_param->shortgi_20);
    mac_mib_set_short_gi_option_in_forty_implemented(hmac_vap, beacon_param->shortgi_40);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if ((hmac_vap->channel.band == WLAN_BAND_2G) &&
        (hmac_vap->cap_flag.disable_2ght40 == OAL_TRUE)) {
        mac_mib_set_short_gi_option_in_forty_implemented(hmac_vap, OAL_FALSE);
    }
#endif

    mac_mib_set_vht_short_gi_option_in80_implemented(hmac_vap, beacon_param->shortgi_80);

    ret = hmac_vap_set_security(hmac_vap, beacon_param);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_beacon_content::hmac_vap_set_security failed[%d].}", hmac_vap->vap_id, ret);
        return ret;
    }
    hmac_vap_init_by_protocol_etc(hmac_vap, beacon_param->protocol);
    hmac_vap_init_rates_etc(hmac_vap);
#ifdef BOARD_ASIC_WIFI
    /* AP启动在AX协议模式，刷新MAC时钟为160M                  */
    if (hmac_vap->protocol == WLAN_HE_MODE) {
        oam_warning_log1(0, OAM_SF_ANY, "vapid[%d]{hmac_config_set_beacon_content set mac clk 160M}", hmac_vap->vap_id);
        hmac_vap->hal_device->ax_vap_cnt++;
        hal_update_mac_freq(hmac_vap->hal_device, WLAN_BAND_WIDTH_20M);
        hmac_hal_device_sync(hmac_vap->hal_device);
    }
#endif

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_beacon_etc
 功能描述  : HMAC 层设置AP 信息
 输入参数  : hmac_vap_stru *mac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_beacon_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_beacon_param_stru *beacon_param = OAL_PTR_NULL;
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_u32 ret;

    /* 获取device */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_beacon_etc::null param,mac_device=%p, param=%p.}",
            hmac_vap->vap_id, (uintptr_t)hmac_device, (uintptr_t)(msg->data));
        return OAL_ERR_CODE_PTR_NULL;
    }

    beacon_param = (mac_beacon_param_stru *)msg->data;
    /* 检查协议配置参数是否在device能力内 */
    ret = hmac_config_set_beacon_check_param(hmac_device, beacon_param);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_set_beacon_etc::hmac_config_add_beacon_check_param failed[%d].}",
                         hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }

    ret = hmac_config_set_beacon_content(hmac_vap, beacon_param);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_set_beacon_etc::hmac_config_set_beacon_content failed[%d].}",
                         hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }
    hmac_vap_init_tx_frame_params(hmac_vap, OSAL_TRUE);

    /* 解决GO无法发送Beacon帧问题 */
    if (hmac_vap->hal_vap != OSAL_NULL) {
        hal_vap_set_machw_beacon_period(hmac_vap->hal_vap, (osal_u16)mac_mib_get_beacon_period(hmac_vap));
    }

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 同步vap修改信息到device侧 */
    ret = hmac_config_vap_m2s_info_syn(hmac_vap);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_beacon_etc::m2s_info_syn failed[%d].}",
            hmac_vap->vap_id, ret);
    }
#endif

    return (osal_s32)ret;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_random_mac_addr_scan
 功能描述  : 设置随机mac addr扫描开关, 0关闭，1打开
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_set_random_mac_addr_scan(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_stru *wlan_customize_etc = (wlan_customize_stru *)hwifi_wlan_customize_etc();
#endif

    /* 获取hmac device结构体 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_random_mac_addr_scan::hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    wlan_customize_etc->random_mac_addr_scan = *(msg->data);
#else
    hmac_device->scan_mgmt.is_random_mac_addr_scan = *(msg->data);
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_random_mac_oui_etc
 功能描述  : 设置随机mac oui
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_random_mac_oui_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui_etc::hmac_device is null.device_id %d}",
                         hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (msg->data_len < WLAN_RANDOM_MAC_OUI_LEN) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_random_mac_oui_etc::len is short:%d.}", msg->data_len);
        return OAL_FAIL;
    }

    if (memcpy_s(hmac_device->mac_oui, sizeof(hmac_device->mac_oui), msg->data, WLAN_RANDOM_MAC_OUI_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_set_random_mac_oui_etc::memcpy_s error}");
    }

    /* Android 会在启动wifi 后下发非零mac_oui, wps扫描或hilink连接的场景中,将mac_oui清0,
     * mac_oui 非零时生成扫描随机MAC, wifi 扫描时使用该MAC地址作为源地址 */
    if ((hmac_device->mac_oui[0] != 0) || (hmac_device->mac_oui[1] != 0) ||
        (hmac_device->mac_oui[2] != 0)) { /* mac地址数组2表示第三位的mac */
        oal_eth_random_addr(hmac_device->scan_mgmt.random_mac, sizeof(hmac_device->scan_mgmt.random_mac));
        hmac_device->scan_mgmt.random_mac[0] = hmac_device->mac_oui[0] & 0xfe;  /* 保证是单播mac */
        hmac_device->scan_mgmt.random_mac[1] = hmac_device->mac_oui[1];
        hmac_device->scan_mgmt.random_mac[2] = hmac_device->mac_oui[2]; /* mac地址数组第2位mac */

        oam_warning_log4(0, OAM_SF_SCAN,
            "{hmac_config_set_random_mac_oui_etc::rand_mac_addr[%02X:%02X:%02X:%02X:XX:XX].}",
            hmac_device->scan_mgmt.random_mac[0],
            hmac_device->scan_mgmt.random_mac[1],  /* mac地址数组1表示第二位的mac */
            hmac_device->scan_mgmt.random_mac[2],  /* mac地址数组2表示第三位的mac */
            hmac_device->scan_mgmt.random_mac[3]); /* mac地址数组3表示第四位的mac */
        wifi_printf("{hmac_config_set_random_mac_oui_etc::rand_mac_addr[%02X:%02X:%02X:%02X:XX:XX].}\r\n",
            hmac_device->scan_mgmt.random_mac[0],
            hmac_device->scan_mgmt.random_mac[1],  /* mac地址数组1表示第二位的mac */
            hmac_device->scan_mgmt.random_mac[2],  /* mac地址数组2表示第三位的mac */
            hmac_device->scan_mgmt.random_mac[3]); /* mac地址数组3表示第四位的mac */
    }
    oam_warning_log3(0, OAM_SF_ANY, "{hmac_config_set_random_mac_oui_etc::mac_oui:0x%.2x:%.2x:%.2x}\r\n",
        /* 0，1，2分别表示随机mac地址OUI的第0，1，2位 */
        hmac_device->mac_oui[0], hmac_device->mac_oui[1], hmac_device->mac_oui[2]);
    wifi_printf("{hmac_config_set_random_mac_oui_etc::mac_oui:0x%.2x:%.2x:%.2x}\r\n",
        /* 0，1，2分别表示随机mac地址OUI的第0，1，2位 */
        hmac_device->mac_oui[0], hmac_device->mac_oui[1], hmac_device->mac_oui[2]);

    return OAL_SUCC;
}
#endif

OAL_STATIC osal_u32 hmac_config_kick_user_proc(hmac_vap_stru *hmac_vap,
    mac_cfg_kick_user_param_stru *kick_user_param)
{
    struct osal_list_head *entry = OAL_PTR_NULL;
    struct osal_list_head *dlist_tmp = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);
    frw_msg msg_info;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    oam_warning_log4(0, OAM_SF_UM, "hmac_config_kick_user_proc:kick user mac[%02X:%02X:%02X:%02X:XX:XX]",
        kick_user_param->mac_addr[0], kick_user_param->mac_addr[1], /* 打印第 0, 1 位mac */
        kick_user_param->mac_addr[2], kick_user_param->mac_addr[3]); /* 打印第 2, 3 位mac */
    oam_warning_log1(0, OAM_SF_UM,
        "hmac_config_kick_user_proc:send DISASSOC,err code[%d]", kick_user_param->reason_code);

    /* 踢掉全部user */
    if (oal_is_broadcast_ether_addr(kick_user_param->mac_addr) != 0) {
        /* 启动发送广播去关联帧，会因为ap还没有start，没有配置帧的发送参数，造成该帧的发送通道等没有填写，该帧容易发送失败。
           对于可能直接下电再上电，需要在start_vap时候做，这类异常情况DFR已经设计，是在hmac_config_start_vap中设计 */
#if defined(_PRE_TEST_MODE) && defined(_PRE_TEST_MODE_UT) \
            && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
        hmac_mgmt_send_disassoc_frame_etc(hmac_vap, kick_user_param->mac_addr,
            kick_user_param->reason_code, OAL_FALSE);
#endif

        /* 遍历vap下所有用户, 删除用户 */
        osal_list_for_each_safe(entry, dlist_tmp, &(hmac_vap->mac_user_list_head)) {
            hmac_user = osal_list_entry(entry, hmac_user_stru, user_dlist);
            if (hmac_user == OAL_PTR_NULL) {
                oam_error_log1(0, OAM_SF_UM, "vap_id[%d] {hmac_config_kick_user_proc::user_tmp null.}",
                    hmac_vap->vap_id);
                continue;
            }

            /* 发去关联帧 */
            hmac_mgmt_send_disassoc_frame_etc(hmac_vap, hmac_user->user_mac_addr,
                kick_user_param->reason_code, (osal_u8)hmac_user->cap_info.pmf_active);

            /* 修改 state & 删除 user */
            hmac_handle_disconnect_rsp_etc(hmac_vap, hmac_user, kick_user_param->reason_code);

            if (fhook != OSAL_NULL) {
                ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr,
                    CFG_KICK_USER, USER_CONN_OFFLINE);
            }
            /* 删除用户 */
            hmac_user_del_etc(hmac_vap, hmac_user);
        }

        /* VAP下user头指针不应该为空 */
        if (osal_list_empty(&hmac_vap->mac_user_list_head) == OAL_FALSE) {
            oam_error_log1(0, OAM_SF_UM, "vap_id[%d] {hmac_config_kick_user_proc::mac_user_list_head is not empty.}",
                hmac_vap->vap_id);
        }

        return OAL_SUCC;
    }
    return OAL_CONTINUE;
}

OSAL_STATIC osal_void hmac_kick_user_send_frame(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    const mac_cfg_kick_user_param_stru *kick_user_param)
{
    /* auth错误码发去认证帧 */
    if (kick_user_param->reason_code == MAC_AUTH_TIMEOUT) {
        hmac_mgmt_send_deauth_frame_etc(hmac_vap, hmac_user->user_mac_addr,
            kick_user_param->reason_code, (osal_u8)hmac_user->cap_info.pmf_active);
    } else {
        hmac_mgmt_send_disassoc_frame_etc(hmac_vap, hmac_user->user_mac_addr,
            kick_user_param->reason_code, (osal_u8)hmac_user->cap_info.pmf_active);
    }
    /* 修改 state */
    hmac_handle_disconnect_rsp_etc(hmac_vap, hmac_user, kick_user_param->reason_code);

    osal_msleep(3); /* 休眠3ms等待报文发送完成再删除user */
}
/*****************************************************************************
 函 数 名  : hmac_config_kick_user_etc
 功能描述  : 配置命令去关联1个用户
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_kick_user_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_kick_user_param_stru *kick_user_param = OAL_PTR_NULL;
    osal_u16 user_idx;
    osal_u32 ret;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_DFX_USER_CONN_ABNORMAL_RECORD);

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL || msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_kick_user_etc::null param,hmac_vap=%p param=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)(msg->data));
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_error_log1(0, OAM_SF_UM, "vap_id[%d] {vap_mode is WLAN_VAP_MODE_CONFIG.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    kick_user_param = (mac_cfg_kick_user_param_stru *)msg->data;

    ret = hmac_config_kick_user_proc(hmac_vap, kick_user_param);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }

    ret = hmac_vap_find_user_by_macaddr_etc(hmac_vap, kick_user_param->mac_addr, &user_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_UM, "vap_id[%d] {hmac_vap_find_user_by_macaddr_etc failed[%d].}",
            hmac_vap->vap_id, ret);
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        }
        return (osal_s32)ret;
    }

    hmac_user = mac_res_get_hmac_user_etc(user_idx);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_UM, "vap_id[%d] {hmac_user null, user_idx:%d}", hmac_vap->vap_id, user_idx);
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_FAKE_UP);
        }
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
        oam_warning_log2(0, OAM_SF_UM, "vap_id[%d] {user is unassociated,user_idx:%d}", hmac_vap->vap_id, user_idx);
    }

    hmac_kick_user_send_frame(hmac_vap, hmac_user, kick_user_param);

    if (fhook != OSAL_NULL) {
        ((dfx_user_conn_abnormal_record_cb)fhook)(hmac_vap->vap_id, hmac_user->user_mac_addr, CFG_KICK_USER,
            USER_CONN_OFFLINE);
    }
    /* 删除用户 */
    hmac_user_del_etc(hmac_vap, hmac_user);

    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_config_get_ap_user_info_printf(hmac_vap_stru *hmac_vap, osal_s8 *sta_list_buff, osal_u32 len)
{
    osal_u32 remainder_len = 0;
    osal_s8 tmp_buff[256] = {0};
    struct osal_list_head *head = OAL_PTR_NULL;
    hmac_user_stru *hmac_user = OAL_PTR_NULL;

    remainder_len = len;
    /* AP侧的USER信息 */
    osal_list_for_each(head, &(hmac_vap->mac_user_list_head)) {
        /* 找到相应用户 */
        hmac_user = (hmac_user_stru *)osal_list_entry(head, hmac_user_stru, user_dlist);
        if (hmac_user == OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_get_sta_list_etc::mac_user null.}",
                             hmac_vap->vap_id);
            continue;
        }
        /* 检查用户关联状态 */
        if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
            continue;
        }
        (osal_void)snprintf_s((osal_char *)tmp_buff, OAL_SIZEOF(tmp_buff), OAL_SIZEOF(tmp_buff),
            "%02X:%02X:%02X:%02X:XX:XX\n",
            hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], /* 打印第 0, 1 位mac */
            hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]); /* 打印第 2, 3 位mac */

        wifi_printf("hmac_config_get_sta_list_etc,STA:%02X:%02X:%02X:%02X:XX:XX\n",
            hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], /* 打印第 0, 1 位mac */
            hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]); /* 打印第 2, 3 位mac */

        (osal_void)strncat_s((osal_char *)sta_list_buff, remainder_len, (const osal_char *)tmp_buff,
            osal_strlen((const osal_char *)tmp_buff));
        (osal_void)memset_s(tmp_buff, OAL_SIZEOF(tmp_buff), 0, OAL_SIZEOF(tmp_buff));
        remainder_len = remainder_len - osal_strlen((const osal_char *)tmp_buff);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_config_get_sta_list_etc
 功能描述  : 取得关联的STA
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_get_sta_list_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s8 *sta_list_buff = OAL_PTR_NULL;
    oal_netbuf_stru *netbuf = OAL_PTR_NULL;
    osal_u32 netbuf_len;

    /* 事件传递指针值，此处异常返回前将其置为NULL */
    *(osal_ulong *)msg->rsp = (osal_ulong)OAL_PTR_NULL;

    /* AP侧的信息才能打印相关信息 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_get_sta_list_etc::invalid vap_mode[%d].}",
                         hmac_vap->vap_id, hmac_vap->vap_mode);
        return OAL_FAIL;
    }

    sta_list_buff = (osal_s8 *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (sta_list_buff == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] hmac_config_get_sta_list_etc, oal_mem_alloc failed.",
                         hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(sta_list_buff, OAM_REPORT_MAX_STRING_LEN, 0, OAM_REPORT_MAX_STRING_LEN);

    /* AP侧的USER信息 */
    hmac_config_get_ap_user_info_printf(hmac_vap, sta_list_buff, OAM_REPORT_MAX_STRING_LEN);

    netbuf_len = osal_strlen((const osal_char *)sta_list_buff);
    netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, netbuf_len, OAL_NETBUF_PRIORITY_MID);
    if (netbuf != OAL_PTR_NULL) {
        if (memcpy_s(oal_netbuf_put(netbuf, netbuf_len), netbuf_len, sta_list_buff, netbuf_len) != EOK) {
            oam_error_log0(0, OAM_SF_TX, "hmac_config_get_mode_etc::memcpy_s err!");
            oal_mem_free(sta_list_buff, OAL_TRUE);
            return OAL_FAIL;
        }
    } else {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] hmac_config_get_sta_list_etc::Alloc netbuf(size %d) NULL!", hmac_vap->vap_id, netbuf_len);
    }
    *(osal_ulong *)msg->rsp = (osal_ulong)((uintptr_t)netbuf);

    /* 事件传递指针，此处记录指针长度 */
    msg->rsp_len = (osal_u16)OAL_SIZEOF((uintptr_t)netbuf);

    oal_mem_free(sta_list_buff, OAL_TRUE);
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_config_connect_init(mac_conn_security_stru *conn_sec, hmac_vap_stru *hmac_vap,
    mac_conn_param_stru *connect_param)
{
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_SET_AUTHENTICATION_MODE);

    /* 如果允许本VAP 入网，则返回设备忙状态           */
    /* 根据内核下发的关联能力，赋值加密相关的mib 值 */
    /* 根据下发的join,提取出安全相关的内容 */
    (osal_void)memset_s(conn_sec, sizeof(mac_conn_security_stru), 0, sizeof(mac_conn_security_stru));
    conn_sec->wep_key_len = connect_param->wep_key_len;
    conn_sec->auth_type = connect_param->auth_type;
    conn_sec->privacy = connect_param->privacy;
    memcpy_s(&conn_sec->crypto, sizeof(mac_crypto_settings_stru),
        &connect_param->crypto, sizeof(mac_crypto_settings_stru));
    conn_sec->wep_key_index = connect_param->wep_key_index;
    conn_sec->mgmt_proteced = connect_param->mfp;
    if (conn_sec->wep_key_len > WLAN_WEP104_KEY_LEN) {
        oam_error_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_connect_init:: wep_key_len[%d] > WLAN_WEP104_KEY_LEN!}",
            hmac_vap->vap_id, conn_sec->wep_key_len);
        conn_sec->wep_key_len = WLAN_WEP104_KEY_LEN;
    }
    if (memcpy_s(conn_sec->wep_key, sizeof(conn_sec->wep_key), connect_param->wep_key, conn_sec->wep_key_len) != EOK) {
        oam_warning_log0(0, OAM_SF_TX, "hmac_config_connect_init::memcpy_s err!");
    }

#ifdef _PRE_WLAN_FEATURE_PMF
    conn_sec->pmf_cap = hmac_get_pmf_cap_etc(connect_param->ie, connect_param->ie_len);
    oam_warning_log3(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_connect_init:: connect param mfp[%d] pmf_cap[%d]!}",
        hmac_vap->vap_id, conn_sec->mgmt_proteced, conn_sec->pmf_cap);
#endif
    if (hmac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, connect_param->ie,
        (osal_s32)(connect_param->ie_len))) {
        conn_sec->wps_enable = OAL_TRUE;
    }

    mac_mib_set_authentication_mode(hmac_vap, conn_sec->auth_type);

    if (fhook != OSAL_NULL) {
        ((hmac_11r_set_authentication_mode_cb)fhook)(hmac_vap, conn_sec);
    }

    return;
}

OAL_STATIC osal_u32 hmac_config_connect_check_bss_dscr_channel(mac_bss_dscr_stru *bss_dscr, hmac_vap_stru *hmac_vap)
{
    osal_u8 frame_channel = bss_dscr->st_channel.chan_number;

    /* 检查定制化5g开关是否使能 */
    if ((hmac_device_check_5g_enable(hmac_vap->device_id) == OAL_FALSE) && (frame_channel > MAX_CHANNEL_NUM_FREQ_2G)) {
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_connect_check_bss_dscr_channel not support 5G, vap[%d] chan[%d]}",
            hmac_vap->vap_id, frame_channel);
        return OAL_FAIL;
    }

    /* 帧中信道不合法, 可能是wpa supplicant 从linux内核获取的bss 和 驱动链表中保存的bss 都未老化 */
    if (hmac_is_channel_num_valid_etc(mac_get_band_by_channel_num(frame_channel), frame_channel) != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_connect_check_bss_dscr_channel vap[%d] channel[%d] invalid.}",
            hmac_vap->vap_id, frame_channel);
        wifi_printf("{hmac_config_connect_check_bss_dscr_channel vap[%d] channel[%d] invalid.}\r\n",
            hmac_vap->vap_id, frame_channel);
        return OAL_FAIL;
    }
    return OAL_CONTINUE;
}

OAL_STATIC osal_u32 hmac_config_connect_check_bss_dscr(mac_bss_dscr_stru *bss_dscr,
    hmac_vap_stru *hmac_vap, mac_conn_param_stru *connect_param)
{
    osal_u32 ret;
    osal_void *fhook;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BLACKLIST_FILTER_ETC);
    /* check bssid blacklist from Framework/WIFI HAL Configuration */
    if (fhook != OSAL_NULL && ((hmac_blacklist_filter_etc_cb)fhook)(hmac_vap, bss_dscr->bssid) == OAL_TRUE) {
        return OAL_FAIL;
    }

    if (hmac_config_connect_check_bss_dscr_channel(bss_dscr, hmac_vap) != OAL_CONTINUE) {
        return OAL_FAIL;
    }

    if (osal_memcmp(connect_param->ssid, bss_dscr->ac_ssid, (osal_s32)connect_param->ssid_len) != 0) {
        oam_warning_log1(0, OAM_SF_CFG, "{vap[%d] hmac_config_connect_check_bss_dscr ssid not same}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    hmac_vap->reassoc_flag = OAL_FALSE;

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_ROAM_PROC_CONNECT_CMD);
    if (fhook != OSAL_NULL) {
        ret = ((hmac_roam_proc_connect_cmd_cb)fhook)(hmac_vap, connect_param);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_WAPI_CONNECT_BSS_DSCR);
    if (fhook != OSAL_NULL) {
        ret = ((hmac_wapi_connect_check_bss_dscr_cb)fhook)(bss_dscr, hmac_vap, connect_param);
        if (ret != OAL_CONTINUE) {
            return ret;
        }
    }
    return OAL_CONTINUE;
}

OAL_STATIC osal_u32 hmac_config_connect_set_param(mac_bss_dscr_stru *bss_dscr, hmac_vap_stru *hmac_vap,
    mac_conn_param_stru *connect_param, mac_conn_security_stru *conn_sec)
{
    oal_app_ie_stru app_ie;
    osal_u32 ret;
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_SET_MD_FT_OVER_DS);

    app_ie.ie_len = connect_param->ie_len;
    if (memcpy_s(app_ie.ie, sizeof(app_ie.ie), connect_param->ie, app_ie.ie_len) != EOK) {
        oam_warning_log0(0, OAM_SF_TX, "hmac_config_connect_set_param::memcpy_s err!");
    }
    app_ie.app_ie_type = OAL_APP_ASSOC_REQ_IE;
    if (fhook != OSAL_NULL) {
        ((hmac_set_11r_md_ft_over_ds_cb)fhook)(hmac_vap, app_ie.ie, app_ie.ie_len);
    }
    ret = hmac_config_set_app_ie_to_vap_etc(hmac_vap, &app_ie, app_ie.app_ie_type);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_connect_set_param set app ie fail!}");
        return ret;
    }
    mac_mib_set_dot11dtimperiod(hmac_vap, bss_dscr->dtim_period);

    /* 设置关联用户的能力信息 */
    hmac_vap->assoc_user_cap_info = bss_dscr->us_cap_info;
    hmac_vap->ap_11ntxbf = (bss_dscr->en_11ntxbf == OAL_TRUE) ? 1 : 0;
    if (bss_dscr->rsn_ie != OAL_PTR_NULL) {
        hmac_vap->user_rsn_cap = hmac_get_rsn_capability_etc(bss_dscr->rsn_ie);
    }

    /* 入网选择的热点rssi，同步到dmac传给tpc算法做管理报文tpc */
    conn_sec->rssi = bss_dscr->c_rssi;
    conn_sec->ant0_rssi = bss_dscr->c_ant0_rssi;
    conn_sec->ant1_rssi = bss_dscr->c_ant1_rssi;

    ret = hmac_vap_init_privacy_etc(hmac_vap, conn_sec);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_connect_set_param:: mac_11i_init_privacy failed[%d]!}",
            hmac_vap->vap_id, ret);
        return ret;
    }

    mac_mib_set_WPSActive(hmac_vap, conn_sec->wps_enable);

    fhook = hmac_get_feature_fhook(HMAC_FHOOK_11R_MIB_INIT_CFG);
    if (fhook != OSAL_NULL) {
        ((hmac_11r_mib_init_cfg_cb)fhook)(hmac_vap, conn_sec, bss_dscr);
    }

#ifdef _PRE_WLAN_FEATURE_WPA3
    hmac_vap->sae_pwe = connect_param->sae_pwe;
#endif

    hmac_vap->sae_have_pmkid = (osal_u8)connect_param->crypto.have_pmkid;
    oam_warning_log1(0, 0, "hmac_config_connect_set_param: have_pmkid[%d]", hmac_vap->sae_have_pmkid);
    /* MAC/PHY 能力不做严格检查 */
    hmac_check_capability_mac_phy_supplicant_etc(hmac_vap, bss_dscr);
    return OAL_CONTINUE;
}

/*****************************************************************************
 函 数 名  : hmac_config_connect_etc
 功能描述  : hmac连接
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_connect_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    osal_s32 ret1;
    mac_bss_dscr_stru *bss_dscr = OAL_PTR_NULL;
    mac_conn_param_stru *connect_param = (mac_conn_param_stru *)msg->data;
    mac_conn_security_stru conn_sec;
    frw_msg msg_info;
    osal_void *fhook = OSAL_NULL;
    (osal_void)memset_s(&msg_info, OAL_SIZEOF(msg_info), 0, OAL_SIZEOF(msg_info));

    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL) ||
            (msg->data_len != OAL_SIZEOF(mac_conn_param_stru))) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_config_connect_etc::null ptr or unexpected param len[%x]}", msg->data_len);
        return OAL_FAIL;
    }

    hmac_config_connect_init(&conn_sec, hmac_vap, connect_param);
    /* 获取扫描的bss信息 */
    bss_dscr = (mac_bss_dscr_stru *)hmac_scan_get_scanned_bss_by_bssid(hmac_vap, connect_param->bssid);
    if (bss_dscr == OAL_PTR_NULL) {
        oam_warning_log4(0, OAM_SF_CFG,
            "{hmac_config_connect_check_bss_dscr::find bss fail, bssid:%02X:%02X:%02X:%02X:XX:XX}",
            connect_param->bssid[0], connect_param->bssid[1], /* 打印第 0, 1 位mac */
            connect_param->bssid[2], connect_param->bssid[3]); /* 打印第 2, 3 位mac */
        return OAL_FAIL;
    }

    /* MBO STA获取AP的MBO IE中表示不允许关联或re-assoc delay的子元素，STA不与(或re-assoc delay time内不与)AP关联 */
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_MBO_STA_CHECK_IS_ASSOC_ALLOWED);
    if (fhook != OSAL_NULL) {
        ret = ((hmac_mbo_check_is_assoc_or_re_assoc_allowed_cb)fhook)(hmac_vap, connect_param, bss_dscr);
        if (ret != OAL_CONTINUE) {
            return (osal_s32)ret;
        }
    }

    ret = hmac_config_connect_check_bss_dscr(bss_dscr, hmac_vap, connect_param);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }
    wifi_printf("connect:bw[%d] chan[%d] ssid[%s]\r\n",
        bss_dscr->channel_bandwidth, bss_dscr->st_channel.chan_number, bss_dscr->ac_ssid);

    ret = hmac_config_connect_set_param(bss_dscr, hmac_vap, connect_param, &conn_sec);
    if (ret != OAL_CONTINUE) {
        return (osal_s32)ret;
    }
    /***************************************************************************
    抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    msg_info.data = (osal_u8 *)&conn_sec;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(conn_sec);
    ret1 = hmac_config_connect(hmac_vap, &msg_info);
    if (OAL_UNLIKELY(ret1 != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_connect_etc::connect fail[%d]}", hmac_vap->vap_id, ret1);
        return ret1;
    }
    return (osal_s32)hmac_sta_initiate_join_etc(hmac_vap, bss_dscr);
}

#ifdef _PRE_WLAN_FEATURE_11D
/*****************************************************************************
 函 数 名  : hmac_config_set_rd_by_ie_switch_etc
 功能描述  : hmac更改是否根据关联ap设置sta的国家码
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_rd_by_ie_switch_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_vap->updata_rd_by_ie_switch = (osal_u8)(*(osal_u32 *)(msg->data));
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_get_assoc_req_ie_etc
 功能描述  : AP 获取STA 关联请求IE 信息
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 *pus_len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_get_assoc_req_ie_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_net_dev_ioctl_data_stru *assoc_req_ioctl_data;
    hmac_user_stru              *hmac_user;
    osal_u8                   *mac;
    osal_u8                   *assoc_req_ie;
    osal_u32                   ul_ret;
    osal_u32                   len;

    assoc_req_ioctl_data = (oal_net_dev_ioctl_data_stru *)msg->rsp;

    msg->rsp_len = (osal_u16)OAL_SIZEOF(oal_net_dev_ioctl_data_stru);

    /* 根据mac 地址查找用户 */
    mac = (osal_u8 *)assoc_req_ioctl_data->pri_data.assoc_req_ie.mac;
    hmac_user  = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, mac);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_get_assoc_req_ie_etc::hmac_user null.}",
                         hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 拷贝关联请求帧信息 */
    assoc_req_ie = assoc_req_ioctl_data->pri_data.assoc_req_ie.buf;
    len = hmac_user->assoc_req_ie_len;
    if (len > assoc_req_ioctl_data->pri_data.assoc_req_ie.buf_size) {
        len = assoc_req_ioctl_data->pri_data.assoc_req_ie.buf_size;
        oam_warning_log3(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_get_assoc_req_ie_etc::user space[%d] not enough,need[%d].}",
                         hmac_vap->vap_id, assoc_req_ioctl_data->pri_data.assoc_req_ie.buf_size,
                         hmac_user->assoc_req_ie_len);
    }
    ul_ret = osal_copy_to_user(assoc_req_ie, hmac_user->assoc_req_ie_buff, len);
    if (ul_ret != 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_get_assoc_req_ie_etc::hmac osal_copy_to_user fail.");
        return OAL_FAIL;
    }
    assoc_req_ioctl_data->pri_data.assoc_req_ie.buf_size = hmac_user->assoc_req_ie_len;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_app_ie_to_vap_etc
 功能描述  : 将用户态 IE 信息拷贝到内核态中
 输入参数  : hmac_vap_stru *hmac_vap
             oal_net_dev_ioctl_data_stru *ioctl_data
             enum WPS_IE_TYPE type
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_config_set_app_ie_to_vap_etc(hmac_vap_stru *hmac_vap, oal_app_ie_stru *app_ie,
    app_ie_type_uint8 type)
{
    osal_u32                      ul_ret;
    osal_u8                      *ie;
    osal_u32                      remain_len;

    if ((hmac_vap == OAL_PTR_NULL) || (app_ie == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_app_ie_to_vap_etc::scan failed, set ie null ptr, %p, %p.}",
            (uintptr_t)hmac_vap, (uintptr_t)app_ie);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 移除驱动侧重复MAC_EID_EXT_CAPS */
    ie = mac_find_ie_etc(MAC_EID_EXT_CAPS, app_ie->ie, (osal_s32)app_ie->ie_len);
    if (ie != OAL_PTR_NULL) {
        app_ie->ie_len  -= (osal_u32)(ie[1] + MAC_IE_HDR_LEN);
        remain_len = app_ie->ie_len - (osal_u32)(ie - app_ie->ie);
        if (memmove_s(ie, remain_len, ie + (osal_u32)(ie[1] + MAC_IE_HDR_LEN), remain_len) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "hmac_config_set_app_ie_to_vap_etc memmove err!");
        }
    }

    /* 该问题修复 ccpriv命令保留后续使用 */
    /* remove type 为1表示移除该IE，0为恢复IE，不处理即可，支持处理多个IE需在此扩展 */
    if (hmac_vap->remove_ie.type != 0) {
        ie = mac_find_ie_etc(hmac_vap->remove_ie.eid, app_ie->ie,
            (osal_s32)app_ie->ie_len);
        if (ie != OAL_PTR_NULL) {
            app_ie->ie_len  -= (osal_u32)(ie[1] + MAC_IE_HDR_LEN);
            remain_len = app_ie->ie_len - (osal_u32)(ie - app_ie->ie);
            if (memmove_s(ie, remain_len, ie + (osal_u32)(ie[1] + MAC_IE_HDR_LEN), remain_len) != EOK) {
                oam_warning_log0(0, OAM_SF_ANY, "hmac_config_set_app_ie_to_vap_etc memmove p2 err!");
            }
        }
    }
    /*  对比标杆默认不携带该IE 修改通过判断MIB值过滤掉EID 59 */
    ie = mac_find_ie_etc(MAC_EID_OPERATING_CLASS, app_ie->ie, (osal_s32)app_ie->ie_len);
    if ((ie != OAL_PTR_NULL) && (mac_mib_get_dot11_extended_channel_switch_activated(hmac_vap) == OSAL_FALSE)) {
        app_ie->ie_len -= (osal_u32)(ie[1] + MAC_IE_HDR_LEN);
        remain_len = app_ie->ie_len - (osal_u32)(ie - app_ie->ie);
        if (memmove_s(ie, remain_len, ie + (osal_u32)(ie[1] + MAC_IE_HDR_LEN), remain_len) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "hmac_config_set_app_ie_to_vap_etc memmove p3 err!");
        }
    }

    ul_ret = hmac_vap_save_app_ie_etc(hmac_vap, app_ie, type);
    if (ul_ret != OAL_SUCC) {
        oam_error_log4(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_app_ie_to_vap_etc::hmac_vap_save_app_ie_etc fail[%d], type[%d], len[%d].}",
            hmac_vap->vap_id, ul_ret, type, app_ie->ie_len);
        return ul_ret;
    }

    return OAL_SUCC;
}

osal_s32 hmac_config_set_customize_ie(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    oal_app_ie_stru *app_ie = OSAL_NULL;

    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_set_customize_ie::ptr null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* set_app_ie 命令不支持 STA 或 GC 模式下发 */
    if (is_sta(hmac_vap) && (is_legacy_vap(hmac_vap) || is_p2p_cl(hmac_vap))) {
        oam_warning_log3(0, OAM_SF_CFG,
            "{hmac_config_set_customize_ie::cmd not support STA/GC. vap_id=%d, vap_mode=%d, p2p_mode=%d}",
            hmac_vap->vap_id, hmac_vap->vap_mode, hmac_vap->p2p_mode);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    app_ie = (oal_app_ie_stru *)msg->data;
    if (app_ie->app_ie_type == OAL_APP_VENDOR_IE) {
        hmac_vap->mgmt_report = (app_ie->ie_len == 0) ? OAL_FALSE : OAL_TRUE;
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_customize_ie::vap_id=%d, hmac_vap->mgmt_report=%d}",
            hmac_vap->vap_id, hmac_vap->mgmt_report);
    }

    ret = hmac_config_set_app_ie_to_vap_etc(hmac_vap, app_ie, app_ie->app_ie_type);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_customize_ie::vap_id=%d, save_app_ie() ret %d}",
            hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }

    return OAL_SUCC;
}


#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_set_thruput_test(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_thruput_test_stru *thruput_test = (mac_cfg_thruput_test_stru *)msg->data;

    if (thruput_test->thruput_type == THRUPUT_NOACK) {
        hmac_vap->tx_noack = thruput_test->value;
    }

    oam_warning_log2(0, OAM_SF_CFG, "{hmac_config_set_thruput_test::type:%u, value:%u.}",
                     thruput_test->thruput_type, thruput_test->value);
    hmac_set_thruput_test(thruput_test->thruput_type, thruput_test->value);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32  hmac_config_set_2040_coext_support_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if ((*(msg->data) != 0) && (*(msg->data) != 1)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_2040_coext_support_etc::invalid param[%d].",
                         hmac_vap->vap_id, *msg->data);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log1(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_set_2040_coext_support_etc::this is config vap! can't set.}",
                         hmac_vap->vap_id);
        return OAL_FAIL;
    }

    mac_mib_set_2040_bss_coexistence_management_support(hmac_vap, (oal_bool_enum_uint8)(*msg->data));

    oam_info_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_2040_coext_support_etc::end func,param=%d.}",
                  hmac_vap->vap_id, *msg->data);
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_ch_status_sync
 功能描述  : 同步信道信息
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32 hmac_config_ch_status_sync(hmac_device_stru *hmac_device)
{
    osal_u32 ret;
    mac_ap_ch_info_stru  ap_channel_list[MAC_MAX_SUPP_CHANNEL] = {{0}};
    hmac_vap_stru        *hmac_vap;

    if (!hmac_device) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_ch_status_sync::mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[0]);
    if (!hmac_vap) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_ch_status_sync::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (memcpy_s((osal_u8 *)ap_channel_list, sizeof(ap_channel_list), (osal_u8 *)(hmac_device->ap_channel_list),
        OAL_SIZEOF(ap_channel_list)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_ch_status_sync::memcpy_s error}");
    }
    ret = (osal_u32)hmac_config_ch_status_sync_ext(hmac_vap, ap_channel_list, OAL_SIZEOF(ap_channel_list));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG,
                         "vap_id[%d] {hmac_config_ch_status_sync::hmac_config_send_event_etc failed[%d].}",
                         hmac_vap->vap_id, ret);
    }

    return ret;
}

#ifdef _PRE_WLAN_FEATURE_ISOLATION
/*****************************************************************************
 函 数 名  : hmac_config_show_isolation
 功能描述  : 隔离信息打印
 输入参数  : hmac_vap  : hmac_vap_stru
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_show_isolation(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    hmac_show_isolation_info(hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_isolation_mode
 功能描述  : 设置用户隔离模式
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_set_isolation_mode(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    osal_u32                  ul_ret;
    osal_u32                 *pul_cfg_mode;

    pul_cfg_mode = (osal_u32 *)param;
    ul_ret = hmac_isolation_set_mode(hmac_vap, (osal_u8) * pul_cfg_mode);
    if (ul_ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_isolation_set_mode fail: ret=%d; cfg=%d}", hmac_vap->vap_id, ul_ret, *pul_cfg_mode);
        return ul_ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_isolation_type
 功能描述  : 设置用户隔离模式
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_set_isolation_type(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    osal_u32                  ul_ret;

    ul_ret = hmac_isolation_set_type(hmac_vap, param[0], param[1]);
    if (ul_ret != OAL_SUCC) {
        oam_error_log4(0, OAM_SF_ANY,
                       "vap_id[%d] {hmac_isolation_set_type fail: ret=%d; bss_type=%d, isolation_type=%d}",
                       hmac_vap->vap_id, ul_ret, param[0], param[1]);
        return ul_ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_isolation_forword
 功能描述  : 设置用户隔离模式
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_set_isolation_forword(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    osal_u32                  ul_ret;
    osal_u32                 *pul_cfg_forword;

    pul_cfg_forword = (osal_u32 *)param;
    ul_ret = hmac_isolation_set_forward(hmac_vap, (osal_u8) * pul_cfg_forword);
    if (ul_ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_ANY,
                       "vap_id[%d] {hmac_isolation_set_forward fail: ret=%d; cfg=%d}\r\n", hmac_vap->vap_id,
                       ul_ret, *pul_cfg_forword);
        return ul_ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_isolation_clear
 功能描述  : 设置用户隔离模式
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_set_isolation_clear(hmac_vap_stru *hmac_vap, osal_u16 len, osal_u8 *param)
{
    osal_u32                  ul_ret;
    ul_ret = hmac_isolation_clear_counter(hmac_vap);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_isolation_clear_counter fail: ret=%d; cfg=%d}\r\n", hmac_vap->vap_id, ul_ret);
        return ul_ret;
    }
    return OAL_SUCC;
}
#endif  /* _PRE_WLAN_FEATURE_ISOLATION */

/*****************************************************************************
 函 数 名  : hmac_config_scan_abort_etc
 功能描述  : 扫描终止
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_scan_abort_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru        *hmac_device;
    unref_param(msg);

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_config_scan_abort_etc::hmac_device is null, dev_id[%d].}", hmac_vap->vap_id,
            hmac_vap->device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    oam_warning_log3(0, OAM_SF_SCAN,
        "vap_id[%d] {hmac_config_scan_abort_etc::vap_id[%d] scan abort,curr_scan_vap_id:%d}", hmac_vap->vap_id,
        hmac_vap->vap_id, hmac_device->scan_mgmt.scan_record_mgmt.vap_id);

    /* 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */
    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_vap->vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }

    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (hmac_device->scan_mgmt.scan_record_mgmt.vap_last_state != MAC_VAP_STATE_BUTT)) {
        oam_warning_log2(0, OAM_SF_SCAN,
            "vap_id[%d] {hmac_config_scan_abort_etc::vap_last_state:%d}", hmac_vap->vap_id,
            hmac_device->scan_mgmt.scan_record_mgmt.vap_last_state);
        hmac_fsm_change_state_etc(hmac_vap, hmac_device->scan_mgmt.scan_record_mgmt.vap_last_state);
        hmac_device->scan_mgmt.scan_record_mgmt.vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* END: 作为ap ，40M 带宽下执行扫描，扫描完成后VAP 状态修改为扫描前的状态 */

    /* 清除扫描结果上报的回调函数，无需上报 */
    if (hmac_device->scan_mgmt.scan_record_mgmt.vap_id == hmac_vap->vap_id) {
        hmac_device->scan_mgmt.scan_record_mgmt.p_fn_cb = OAL_PTR_NULL;
        /*  扫描终止时，直接清除扫描标志,无需等待devcie上报扫描结束才清除 */
        hmac_device->scan_mgmt.is_scanning = OAL_FALSE;
    }

    hmac_config_scan_abort(hmac_vap);

    return OAL_SUCC;
}

osal_u32 g_time_init_start;

osal_u32 wifi_get_init_start_time(osal_void)
{
    return g_time_init_start;
}

osal_void wifi_set_init_start_time(osal_void)
{
    g_time_init_start = (osal_u32)osal_get_time_stamp_ms();
}

OAL_STATIC osal_s32 hmac_config_offline_cali_finish(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    static osal_u8 wlan_close_flag = 0;
#ifdef _PRE_WLAN_FEATURE_DAQ
    osal_u8 flag = 0;
    frw_msg message = {0};
#endif
    osal_s32 ret = OAL_SUCC;
    osal_u32 start_timestamp, current_timestamp, run_time;
    frw_msg msg_info = {0};

    unref_param(msg);

    /* 由于开机insmod ko时，需要open wlan进行校准，校准结束后需要close wlan, 仅执行一次 */
    if (wlan_close_flag == 0) {
        wlan_close_flag = 1;
        ret = (osal_s32)wlan_close();
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_CFG, "hmac_config_offline_cali_finish::wlan_close failed ret [%d]", ret);
        }
        /* ko完全加载完之后需要通知wal层 */
        ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_INSMOD_KO_FINISH, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_RRM, "{hmac_config_offline_cali_finish: frw_asyn_host_post_msg failed[%d]!}", ret);
        }
    }

#ifdef _PRE_WLAN_FEATURE_DAQ
    if (hwifi_get_data_sample() == OSAL_FALSE) {
        start_timestamp = wifi_get_init_start_time();
        current_timestamp = (osal_u32)osal_get_time_stamp_ms();
        run_time = osal_get_runtime(start_timestamp, current_timestamp);
        wifi_printf_always("wifi_host_init_finish![wifi_cali1 cost %d ms].\n", run_time);
        return OAL_SUCC;
    }

    frw_msg_init((osal_u8 *)&flag, sizeof(flag), OSAL_NULL, 0, &message);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DATA_SAMPLE_RESV_MEM, &message, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "hmac_config_offline_cali_finish::data sample resv mem ret [%d]", ret);
    }
#endif

    start_timestamp = wifi_get_init_start_time();
    current_timestamp = (osal_u32)osal_get_time_stamp_ms();
    run_time = osal_get_runtime(start_timestamp, current_timestamp);
    wifi_printf_always("wifi_host_init_finish![wifi_cali2 cost %d ms].\n", run_time);
    return ret;
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
功能描述  : RF定制化
输入参数  : hmac_vap_stru *mac_vap
          osal_u8 len
          osal_u8 *param
返 回 值: osal_u32
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_set_cus_rf(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    hal_config_custom_rf(msg->data);

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 根据rf定制化，刷新2g5g定制化到chip能力 */
    hal_chip_init_rf_custom_cap(hal_get_chip_stru());
#endif

    return OAL_SUCC;
}

/*****************************************************************************
功能描述  : dts 校准参数
输入参数  : hmac_vap_stru *mac_vap
          osal_u8 len
          osal_u8 *param
返 回 值: osal_u32
*****************************************************************************/
OAL_STATIC osal_s32 hmac_config_set_cus_dts_cali(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    osal_u32 run_time;
    osal_u32 start_timestamp = (osal_u32)osal_get_time_stamp_ms();
    osal_u32 current_timestamp;
    frw_msg msg_info = {0};
    osal_u8 *power_cali_addr;
    osal_u16 power_cali_len = 0;
#if !defined(_PRE_WLAN_ASIC_SMOKE) && !defined(CHIP_EDA)
    static osal_u8 init_flag = OSAL_TRUE;
#endif
    unref_param(msg);
    /* 停止PA和PHY的工作 */
    hal_disable_machw_phy_and_pa();

    /* 初始化RF系统 */
    hal_initialize_rf_sys(hal_device);
#if !defined(_PRE_WLAN_ASIC_SMOKE) && !defined(CHIP_EDA)
    // 校准前刷新温度码字
    rf_temp_code_config_comp_value(hal_device, hmac_vap->channel.band, init_flag);
#endif
    /* host侧在等待，需要device上报WIFI_READY消息给host侧 */
    power_cali_addr = (osal_u8 *)fe_custom_get_power_cali(hmac_vap->channel.band, &power_cali_len);
    frw_msg_init(power_cali_addr, power_cali_len, OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_HCC_SEND_READY_MSG, &msg_info, OSAL_TRUE);
#if !defined(_PRE_WLAN_ASIC_SMOKE) && !defined(CHIP_EDA)
#ifdef _PRE_WLAN_ONLINE_CALI
    /* 动态功率初始化 */
    if (init_flag == OSAL_TRUE) {
        online_cali_init(hal_device);
    }
#endif
    // 校准后定时刷新温度码字
    rf_temp_code_create_config_timer(hal_device);
    init_flag = OSAL_FALSE;
#endif
    current_timestamp = (osal_u32)osal_get_time_stamp_ms();
    run_time = osal_get_runtime(start_timestamp, current_timestamp);
    wifi_printf("vap_id[%d] {hmac_config_set_cus_dts_cali::cali time[%u]ms ret[%u]}", hmac_vap->vap_id, run_time, ret);
    return OAL_SUCC;
}
OAL_STATIC osal_s32 hmac_config_set_rx_insert_loss(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (msg == OSAL_NULL || msg->data == OSAL_NULL || msg->data_len == 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_rx_insert_loss::msg null error!\n");
        return OAL_FAIL;
    }
    hal_set_custom_rx_insert_loss(hmac_vap->channel.band, msg->data, (osal_u8)msg->data_len);
    return OAL_SUCC;
}
OAL_STATIC osal_s32 hmac_config_get_rx_insert_loss(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 rx_loss;
    osal_s32 ret;
    mac_channel_stru channel;
    unref_param(hmac_vap);
    if (msg == OSAL_NULL || msg->data == OSAL_NULL || msg->data_len == 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_get_rx_insert_loss::msg null error!\n");
        return OAL_FAIL;
    }
    channel = *(mac_channel_stru *)msg->data;
    oam_warning_log2(0, OAM_SF_CFG, "hmac_config_get_rx_insert_loss::band:%d channel idx:%d!\n",
        channel.band, channel.chan_idx);
    rx_loss = hal_get_custom_rx_insert_loss(channel.band, channel.chan_idx);
    ret = memcpy_s(msg->rsp, sizeof(rx_loss), &rx_loss, sizeof(rx_loss));
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_get_rx_insert_loss::memcpy_s failed.}");
        return OAL_FAIL;
    }
    msg->rsp_len = sizeof(rx_loss);
    return OAL_SUCC;
}
OAL_STATIC osal_s32 hmac_config_set_power(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (msg == OSAL_NULL || msg->data == OSAL_NULL || msg->data_len == 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_power::msg null error!\n");
        return OAL_FAIL;
    }

    hal_set_custom_power(hmac_vap->channel.band, msg->data, msg->data_len);
    return OAL_SUCC;
}
OAL_STATIC osal_s32 hmac_config_set_power_cali(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (msg == OSAL_NULL || msg->data == OSAL_NULL || msg->data_len == 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_power_cali::msg null error!\n");
        return OAL_FAIL;
    }
    fe_efuse_get_all_power_cali((osal_u8 *)msg->data, msg->data_len); /* 读efuse覆盖消息数据 */
    fe_custom_config_power_cali(hmac_vap->channel.band, msg->data);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_config_set_rssi(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (msg == OSAL_NULL || msg->data == OSAL_NULL || msg->data_len == 0) {
        oam_error_log0(0, OAM_SF_CFG, "hmac_config_set_rssi::msg null error!\n");
        return OAL_FAIL;
    }
    fe_efuse_get_all_rssi_comp((osal_u8 *)msg->data, msg->data_len); /* 读efuse覆盖消息数据 */
    if (frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_PROCESS_RSSI_COMP, msg, OSAL_FALSE) != OAL_SUCC) {
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

osal_void hmac_cfg_init(osal_void)
{
    static const msg_hook_array_stru msg_hook_array[] = {
        {WLAN_MSG_D2H_C_CFG_CFG80211_MGMT_TX_STATUS, hmac_mgmt_tx_event_status_etc},
        {WLAN_MSG_D2H_C_CFG_SYNC_PSM_STATUS, hmac_config_sta_psm_status_sync},
        {WLAN_MSG_D2H_C_CFG_DEL_VAP_SUCCESS, hmac_config_del_vap_success},
#ifdef _PRE_WLAN_ONLINE_CALI
        {WLAN_MSG_D2H_TX_COMPLETE_ONLINE_RF_CALI, online_cali_tx_complete_dyn_process},
#endif
#ifdef _PRE_WLAN_CFGID_DEBUG
        {WLAN_MSG_W2H_C_CFG_SET_KEEPALIVE_MODE, hmac_config_set_keepalive_mode},
        {WLAN_MSG_W2H_C_CFG_SET_AP_USER_AGING_TIME, hmac_config_set_ap_user_aging_time},
        {WLAN_MSG_W2H_C_CFG_COMMON_DEBUG, hmac_config_common_debug},
        {WLAN_MSG_W2H_C_CFG_BEACON_CHAIN_SWITCH, hmac_config_beacon_chain_switch},
#endif
#if defined (_PRE_WLAN_FEATURE_P2P) && defined (_PRE_WLAN_CFGID_DEBUG)
        {WLAN_MSG_W2H_C_CFG_P2P_SET_MODE, hmac_set_p2p_common_info},
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        {WLAN_MSG_W2H_C_CFG_SET_CUS_RF, hmac_config_set_cus_rf},
        {WLAN_MSG_W2H_C_CFG_SET_CUS_DTS_CALI, hmac_config_set_cus_dts_cali},
        {WLAN_MSG_W2H_CFG_SET_RF_FE_RX_INSERT_LOSS, hmac_config_set_rx_insert_loss},
        {WLAN_MSG_W2H_CFG_GET_RF_FE_RX_INSERT_LOSS, hmac_config_get_rx_insert_loss},
        {WLAN_MSG_W2H_CFG_SET_RF_FE_POWER, hmac_config_set_power},
        {WLAN_MSG_W2H_CFG_SET_RF_FE_POWER_CALI, hmac_config_set_power_cali},
        {WLAN_MSG_W2H_CFG_SET_RF_FE_RSSI, hmac_config_set_rssi},
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#ifdef _PRE_WLAN_DFR_STAT
        {WLAN_MSG_D2H_C_CFG_DFR, hmac_get_dfr_info},
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
        {WLAN_MSG_W2H_C_CFG_COUNTRY_FOR_DFS, hmac_config_set_country_for_dfs},
#endif
        {WLAN_MSG_W2H_C_CFG_QUERY_RSSI, hmac_config_query_rssi},
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        {WLAN_MSG_W2H_C_CFG_QUERY_STATUS, hmac_config_query_status},
#endif
#if defined(_PRE_WLAN_FEATURE_M2S) && defined(_PRE_WLAN_CFGID_DEBUG)
        {WLAN_MSG_W2H_C_CFG_SET_M2S_SWITCH, hmac_config_set_m2s_switch},
        {WLAN_MSG_W2H_C_CFG_SET_M2S_MSS, hmac_config_set_m2s_switch_mss},
        {WLAN_MSG_W2H_C_CFG_SET_M2S_BLACKLIST, hmac_config_set_m2s_switch_blacklist},
#endif
#ifdef _PRE_WLAN_DFT_STAT
        {WLAN_MSG_W2H_C_CFG_QUERY_ANI, hmac_config_query_ani},
        {WLAN_MSG_W2H_C_CFG_MAC_DUMP, hmac_config_dump_mac},
        {WLAN_MSG_W2H_C_CFG_PHY_DUMP, hmac_config_dump_phy},
        {WLAN_MSG_W2H_C_CFG_STATS_DUMP, hmac_config_dump_stats},
#endif
        {WLAN_MSG_W2H_C_CFG_SEND_CUSTOM_PKT, hmac_config_send_custom_pkt},
        {WLAN_MSG_W2H_C_CFG_SET_BACKOFF_DELAY, hmac_config_set_backoff_delay},
 
#if defined(_PRE_WLAN_FEATURE_ALWAYS_TX) || defined(_PRE_WLAN_CFGID_DEBUG)
        {WLAN_MSG_W2H_C_CFG_REG_WRITE, hmac_config_reg_write},
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
        {WLAN_MSG_W2H_C_CFG_SET_ALWAYS_TX, hmac_config_set_always_tx},
        {WLAN_MSG_W2H_C_CFG_SET_ALWAYS_RX, hmac_config_set_always_rx},
        {WLAN_MSG_W2H_CFG_GET_RX_PPDU_INFO, hmac_config_get_rx_ppdu_info},
        {WLAN_MSG_W2H_C_CFG_GET_FAST_RESULT, hmac_config_get_fast_result},
        {WLAN_MSG_W2H_C_CFG_RESET_FAST_RESULT, hmac_config_reset_fast_result},
#endif
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
        {WLAN_MSG_W2H_C_CFG_CFG80211_STOP_SCHED_SCAN, hmac_config_stop_sched_scan},
#endif
#ifdef _PRE_WIFI_DEBUG
        {WLAN_MSG_W2H_C_CFG_SET_TX_DATA_TYPE_DEBUG, hmac_config_set_tx_data_type_debug},
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
        {WLAN_MSG_W2H_CFG_MGMT_FRAME_REPORT, hmac_config_mgmt_report_switch},
#endif
        {WLAN_MSG_D2H_OFFLINE_CALI_FINISH, hmac_config_offline_cali_finish},
#ifdef _PRE_WLAN_FEATURE_DAQ
        {WLAN_MSG_D2H_C_CFG_COMPLETE_PHY_EVENT_RPT, hmac_complete_phy_event_rpt},
#endif
#ifdef _PRE_WLAN_WIRELESS_EXT
        {WLAN_MSG_W2H_CFG_IWLIST_SCAN, hmac_config_start_iwlist_scan},
#endif
    };

    frw_msg_hook_register_array(msg_hook_array, osal_array_size(msg_hook_array));
}

osal_void hmac_cfg_ccpriv_init(osal_void)
{
    hmac_ccpriv_register((const osal_s8 *)"sta_rcv_probe_req", hmac_config_sta_rcv_probe_req_switch);
    hmac_ccpriv_register((const osal_s8 *)"get_status_code", hmac_config_sta_get_conn_status_code);
}

osal_void hmac_cfg_ccpriv_exit(osal_void)
{
    hmac_ccpriv_unregister((const osal_s8 *)"sta_rcv_probe_req");
    hmac_ccpriv_unregister((const osal_s8 *)"get_status_code");
}

#ifdef _PRE_WLAN_FEATURE_HS20
/*****************************************************************************
 函 数 名  : hmac_config_set_qos_map
 功能描述  : 设置QoSMap参数
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32  hmac_config_set_qos_map(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8                      idx;
    hmac_cfg_qos_map_param_stru   *qos_map;

    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_ANY, "hmac_config_set_qos_map:: pointer is null: hmac_vap[%d],param[%d]",
                       hmac_vap, msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }

    qos_map = (hmac_cfg_qos_map_param_stru *)msg->data;
    oam_info_log3(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_set_qos_map:num_dscp_except=%d, valid=%d}",
                  hmac_vap->vap_id, qos_map->num_dscp_except, qos_map->valid);

    /* 判断QOS MAP SET的使能开关是否打开 */
    if (!qos_map->valid) {
        return OAL_FAIL;
    }

    /* 检查下发的QoS Map Set参数中的DSCP Exception fields 是否超过最大数目21 */
    if (qos_map->num_dscp_except > MAX_DSCP_EXCEPT) {
        oam_error_log2(0, OAM_SF_CFG,
                       "vap_id[%d] {hmac_config_set_qos_map:: input exceeds maximum : qos_map->num_dscp_except[%d]",
                       hmac_vap->vap_id, qos_map->num_dscp_except);
        return OAL_FAIL;
    }
    /* 判断DSCP Exception fields是否为空 */
    if ((qos_map->num_dscp_except != 0)) {
        hmac_vap->cfg_qos_map_param.num_dscp_except = qos_map->num_dscp_except;
        for (idx = 0; idx < qos_map->num_dscp_except; idx++) {
            hmac_vap->cfg_qos_map_param.dscp_exception[idx] = qos_map->dscp_exception[idx];
            hmac_vap->cfg_qos_map_param.dscp_exception_up[idx] =
                qos_map->dscp_exception_up[idx];
        }
    }

    /* 配置DSCP Exception format中的User Priority的HIGHT和LOW VALUE值 */
    for (idx = 0; idx < MAX_QOS_UP_RANGE; idx++) {
        hmac_vap->cfg_qos_map_param.up_high[idx] = qos_map->up_high[idx];
        hmac_vap->cfg_qos_map_param.up_low[idx] = qos_map->up_low[idx];
    }
    return OAL_SUCC;
}
#endif // _PRE_WLAN_FEATURE_HS20

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_s32 hmac_config_set_all_log_level_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    return OAL_SUCC;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

/*****************************************************************************
 函 数 名  : hmac_config_set_vendor_ie
 功能描述  : AP 设置okc 信息元素
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16 len
             osal_u8 *param
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_vendor_ie(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_app_ie_stru *okc_ie;
    osal_s32 ret;

    okc_ie = (oal_app_ie_stru *)msg->data;

    /* 设置WPS 信息 */
    ret = (osal_s32)hmac_config_set_app_ie_to_vap_etc(hmac_vap, okc_ie, okc_ie->app_ie_type);
    oam_warning_log4(0, OAM_SF_CFG, "hmac_config_set_vendor_ie::vap_id=%d, ie_type=%d, ie_length=%d, ret=[%d]\n",
        hmac_vap->vap_id, okc_ie->app_ie_type, okc_ie->ie_len, ret);
    if (ret != OAL_SUCC) {
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_vendor_cmd_get_channel_list_etc
 功能描述  : hmac读全部信道列表
 输入参数  : hmac_vap  : mac vap 结构体
 输出参数  : pus_len      : 返回长度
             param    : 返回缓冲区
*****************************************************************************/
osal_s32  hmac_config_vendor_cmd_get_channel_list_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8  chan_idx;
    osal_u8  chan_num;
    osal_u8  chan_number;
    osal_u8 *channel_list;
    mac_vendor_cmd_channel_list_stru *pst_channel_list = OAL_PTR_NULL;
    osal_u32 ul_ret;

    if (msg->rsp == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_vendor_cmd_get_channel_list_etc::len or param is NULL"
            "param %p}", hmac_vap->vap_id, (uintptr_t)(msg->rsp));
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_list = (mac_vendor_cmd_channel_list_stru *)msg->rsp;
    msg->rsp_len = (osal_u16)OAL_SIZEOF(mac_vendor_cmd_channel_list_stru);

    /* 获取2G 信道列表 */
    chan_num = 0;
    channel_list   = pst_channel_list->channel_list_2g;

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_2_BUTT; chan_idx++) {
        ul_ret = hmac_is_channel_idx_valid_etc(MAC_RC_START_FREQ_2, chan_idx);
        if (ul_ret == OAL_SUCC) {
            hmac_get_channel_num_from_idx_etc(MAC_RC_START_FREQ_2, chan_idx, &chan_number);
            channel_list[chan_num++] = chan_number;
        }
    }
    pst_channel_list->channel_num_2g = chan_num;

    /* 检查定制化5g开关是否使能 */
    if (hmac_device_check_5g_enable(hmac_vap->device_id) == OAL_FALSE) {
        pst_channel_list->channel_num_5g = 0;
        return OAL_SUCC;
    }

    /* 获取5G 信道列表 */
    chan_num = 0;
    channel_list   = pst_channel_list->channel_list_5g;

    for (chan_idx = 0; chan_idx < MAC_CHANNEL_FREQ_5_BUTT; chan_idx++) {
        ul_ret = hmac_is_channel_idx_valid_etc(MAC_RC_START_FREQ_5, chan_idx);
        if (ul_ret == OAL_SUCC) {
            hmac_get_channel_num_from_idx_etc(MAC_RC_START_FREQ_5, chan_idx, &chan_number);
            channel_list[chan_num++] = chan_number;
        }
    }
    pst_channel_list->channel_num_5g = chan_num;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_remove_app_ie
 功能描述  : 移除或者恢复某个上层配置的IE，目前只支持操作一个IE，多次操作只会保留一个IE信息，
             如有需要可拓展为链表结构支持多个配置
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_remove_app_ie(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    uint8_t                 type;
    uint8_t                 eid;
    cfg_ie_info_stru       *ie_info;

    if (OAL_UNLIKELY((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_remove_app_ie::null param,hmac_vap=%p param=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)(msg->data));
        return OAL_ERR_CODE_PTR_NULL;
    }

    ie_info = (cfg_ie_info_stru *)msg->data;
    type = ie_info->ie_type;
    eid = ie_info->ie_id;
    switch (type) {
        case 0:
        case 1:
            hmac_vap->remove_ie.type = type;
            hmac_vap->remove_ie.eid = eid;
            break;
        default:
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_remove_app_ie::remove type %d is not supported.}", type);
            break;
    }

    return OAL_SUCC;
}

osal_s32 hmac_config_multi_bssid_enable(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8  multi_bssid_switch = *(msg->data);
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_mib_set_multi_bssid_implement(hmac_vap, multi_bssid_switch);
#else
    unref_param(hmac_vap);
#endif

    oam_warning_log1(0, OAM_SF_11AX, "{hmac_config_multi_bssid_enable::multi_bssid_switch=%d}", multi_bssid_switch);
    return OAL_SUCC;
}

osal_void hmac_config_set_channel_list(hmac_vap_stru *hmac_vap, mac_cfg_set_scan_param *scan_param)
{
    osal_u32 ret;
    osal_u16 index;
    osal_u8 channel_idx = 0;
    osal_u8 channel_number;

    hmac_vap->channel_list_2g = 0;
    hmac_vap->channel_list_5g = 0;

    /* 配置的信道个数为0表示关闭信道列表 */
    if (scan_param->value == 0) {
        return;
    }
    /* 信道长度错误 */
    if (scan_param->value > OAL_ARRAY_SIZE(scan_param->chan_list)) {
        oam_warning_log1(0, OAM_SF_CFG,
                         "{hmac_config_set_channel_list::channel list length error[%d].}", scan_param->value);
        return;
    }
    for (index = 0; index < scan_param->value; index++) {
        channel_number = scan_param->chan_list[index];
        /* 设置1-14信道即2.4G信道 */
        if (mac_get_band_by_channel_num(channel_number) == WLAN_BAND_2G) {
            ret = hmac_get_channel_idx_from_num_etc(WLAN_BAND_2G, channel_number, &channel_idx);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_CFG,
                    "{hmac_config_set_channel_list::get 2G channel index fail. channel:%u}", channel_number);
                return;
            }
            /* 将用户配置的2.4G信道1-14设置到频道列表中 */
            hmac_vap->channel_list_2g |= 1 << (osal_u32)channel_idx;
        } else {
            /* 将用户配置的5G信道36-196设置到频道列表中 */
            ret = hmac_get_channel_idx_from_num_etc(WLAN_BAND_5G, channel_number, &channel_idx);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_CFG,
                    "{hmac_config_set_channel_list::get 5G channel index fail. channel:%u}", channel_number);
                return;
            }
            hmac_vap->channel_list_5g |= 1 << (osal_u32)channel_idx;
        }
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_set_scan_params_etc
 功能描述  : 设置扫描参数
*****************************************************************************/
osal_s32 hmac_config_set_scan_params_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_scan_param_stru *scan_param = OAL_PTR_NULL;

    if (hmac_vap == OSAL_NULL || msg == OSAL_NULL) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_set_scan_params_etc::hmac_vap = %p msg = %p}",
            (uintptr_t)hmac_vap, (uintptr_t)msg);
        return OAL_FAIL;
    }

    scan_param = (mac_cfg_scan_param_stru *)msg->data;

    hmac_vap->max_scan_count_per_channel = scan_param->scan_cnt;
    hmac_vap->scan_time = scan_param->scan_time;
    hmac_vap->scan_channel_interval = scan_param->scan_channel_interval;
    hmac_vap->work_time_on_home_channel = scan_param->work_time_on_home_channel;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WPA3
OAL_STATIC osal_u32 hmac_config_external_auth_add_pmksa(hmac_vap_stru *hmac_vap, hmac_external_auth_stru ext_auth)
{
    hmac_sae_pmksa_entry_info *entry =
        hmac_sae_find_pmkid_by_macaddr(&(hmac_vap->pmksa_mgmt), ext_auth.bssid);
    if (entry != OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_config_external_auth_add_pmksa::find pmkid from list.}");
        entry->timestamp = (osal_u32)oal_get_seconds();

        if (memcpy_s(entry->pmkid, WLAN_PMKID_LEN, ext_auth.pmkid, WLAN_PMKID_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_WPA, "{hmac_config_external_auth_add_pmksa::memcpy pmksa fail.}");
            return OAL_FAIL;
        }
    } else {
        hmac_sae_pmksa_entry_info *new_pmksa = hmac_sae_alloc_pmksa_entry();
        if (new_pmksa == OAL_PTR_NULL) {
            oam_error_log0(0, OAM_SF_WPA, "{hmac_config_external_auth_add_pmksa::store pmksa fail.}");
            return OAL_FAIL;
        }
        if (memcpy_s(new_pmksa->pmkid, WLAN_PMKID_LEN, ext_auth.pmkid, WLAN_PMKID_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_WPA, "{hmac_config_external_auth_add_pmksa::memcpy pmksa fail.}");
            return OAL_FAIL;
        }
        if (memcpy_s(new_pmksa->mac_addr, WLAN_MAC_ADDR_LEN, ext_auth.bssid, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_WPA, "{hmac_config_external_auth_add_pmksa::memcpy macaddr fail.}");
            return OAL_FAIL;
        }
        new_pmksa->timestamp = (osal_u32)oal_get_seconds();
        hmac_sae_add_pmksa_to_list(hmac_vap, new_pmksa);
        oam_warning_log4(0, OAM_SF_WPA, "{add new pmksa[%x:%x:%x:%x:XX:XX] to list", new_pmksa->mac_addr[0],
            new_pmksa->mac_addr[1], new_pmksa->mac_addr[2], new_pmksa->mac_addr[3]); // mac addr 1 2 3
    }
    return OAL_SUCC;
}

/* 执行auth 完成状态信息更新，存在用户已先被删除的场景，函数返回SUCC，防止影响消息返回值 */
osal_s32 hmac_config_external_auth(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OAL_PTR_NULL;
    osal_u16 user_id = 0;
    hmac_external_auth_stru ext_auth = *((hmac_external_auth_stru *)msg->data);
    hmac_auth_rsp_stru auth_rsp;
    memset_s(&auth_rsp, sizeof(auth_rsp), 0, sizeof(auth_rsp));

    if (hmac_vap == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_AUTH, "{hmac_config_external_auth::hmac_vap is NULL}");
        return OAL_SUCC;
    }

    if (hmac_vap_find_user_by_macaddr_etc(hmac_vap, ext_auth.bssid, &user_id) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_config_external_auth::find_user_by_macaddr fail.}");
        return OAL_SUCC;
    }
    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(user_id);
    if (hmac_user == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_WPA, "{hmac_config_external_auth::hmac_user get fail.}");
        return OAL_SUCC;
    }
    /* wpa3 auth失败时, 仅在非关联状态删除user(防止auth2报文攻击) */
    if ((ext_auth.status != MAC_SUCCESSFUL_STATUSCODE) && (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC)) {
        oam_warning_log1(0, OAM_SF_WPA, "{hmac_config_external_auth::rx ext auth status[%d].}", ext_auth.status);
        /* 非anit clogging code需上报内核删除user */
        if (ext_auth.status != MAC_ANTI_CLOGGING) {
            hmac_handle_disconnect_rsp_etc(hmac_vap, hmac_user, ext_auth.status);
        }
        /* 删除用户 */
        return (osal_s32)hmac_user_del_etc(hmac_vap, hmac_user);
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hmac_fsm_change_state_etc(hmac_vap, MAC_VAP_STATE_STA_AUTH_COMP);
        auth_rsp.status_code = HMAC_MGMT_SUCCESS;
        hmac_send_rsp_to_sme_sta_etc(hmac_vap, HMAC_SME_AUTH_RSP, (osal_u8 *)&auth_rsp);
    }
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 解决时序问题，当前已关联完成，只是external auth 消息滞后，此时不能设置状态为auth完成 */
        if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
            hmac_user_set_asoc_state(hmac_user, MAC_USER_STATE_AUTH_COMPLETE);
        }
        if (ext_auth.have_pmkid == OSAL_TRUE) {
            osal_u32 ret = hmac_config_external_auth_add_pmksa(hmac_vap, ext_auth);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_WPA, "{hmac_config_external_auth::add pmkid failed[%d].}", ret);
                return OAL_SUCC;
            }
        }
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_rx_dhcp_debug_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_config_rx_dhcp_debug_etc::hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 现在实际上控制的是dns报文是否打印到串口 */
    hmac_vap->dhcp_debug = *((osal_u8 *)msg->data);

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_DFT_STAT
osal_s32 hmac_config_service_control_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    service_control_msg *service_msg = OAL_PTR_NULL;
    osal_u32 tmp_mask, tmp_value;
    osal_u32 tmp = dfx_get_service_control();

    unref_param(hmac_vap);

    if (OAL_UNLIKELY((msg == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL))) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_config_service_control_etc::msg %p null.}", (uintptr_t)msg);
        return OAL_ERR_CODE_PTR_NULL;
    }

    service_msg = (service_control_msg *)msg->data;
    tmp_mask = service_msg->mask;
    tmp_value = service_msg->value;
    if (tmp_mask == 0xffffffff) {
        /* 修改整个变量的值 */
        dfx_set_service_control(tmp_value);
        return OAL_SUCC;
    }

    /* 修改单个bit位的值, 该值为0或1, tmp_mask表示配置哪个bit位,从0~31中的1位 */
    if ((tmp_value != 0) && (tmp_value != 1)) {
        wifi_printf("{hmac_config_service_control_etc::tmp_mask 0x%x, tmp_value 0x%x error}\r\n", tmp_mask, tmp_value);
        return OAL_FAIL;
    }

    if (((tmp >> tmp_mask) & 0x1) == 0) {
        /* 该bit位原值为0, 则 | 上配置的值 */
        tmp |= (tmp_value << tmp_mask);
    } else {
        /* 该bit位原值为1, 则 & 上配置的值 */
        tmp &= (tmp_value << tmp_mask);
    }

    dfx_set_service_control(tmp);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_DAQ
#define HH503_DIAG_CTL_BASE (0x40005000)
#define HH503_DIAG_CTL_BASE_0x200_0 (HH503_DIAG_CTL_BASE + 0x200)
#define HH503_DIAG_CTL_BASE_0x208_0 (HH503_DIAG_CTL_BASE + 0x208)
#define HH503_DIAG_CTL_BASE_0x210_0 (HH503_DIAG_CTL_BASE + 0x210)
#define HH503_DIAG_CTL_BASE_0x218_0 (HH503_DIAG_CTL_BASE + 0x218)
#define HH503_DIAG_CTL_BASE_0x220_0 (HH503_DIAG_CTL_BASE + 0x220)
#define HH503_DIAG_CTL_BASE_0x228_0 (HH503_DIAG_CTL_BASE + 0x228)
#define HH503_DIAG_CTL_BASE_0x230_0 (HH503_DIAG_CTL_BASE + 0x230)
#define HH503_DIAG_CTL_BASE_0x238_0 (HH503_DIAG_CTL_BASE + 0x238)
#define HH503_DIAG_CTL_BASE_0x248_0 (HH503_DIAG_CTL_BASE + 0x248)
#define HH503_DIAG_CTL_BASE_0x288_0 (HH503_DIAG_CTL_BASE + 0x288)
#define HH503_DIAG_CTL_BASE_0x2A0_0       (HH503_DIAG_CTL_BASE + 0x2A0)
#define HH503_DIAG_CTL_BASE_0x2F8_0       (HH503_DIAG_CTL_BASE + 0x2F8)
#define HH503_DIAG_CTL_BASE_0x390_0 (HH503_DIAG_CTL_BASE + 0x390)

#define HH503_MAC_DIAG_SEL 0x4008003c
#define HH503_MAC_MATCH_ADDR 0x40080c04
#define HH503_MAC_MATCH_EN 0x40080c08
#define HH503_MAC_DIAG5_TYPE_MASK1 0x40080c0c
#define HH503_MAC_DIAG5_TYPE_MASK2 0x40080c10
#define HH503_MAC_MATCH_FRM_TYPE 0x40080c14
#define HH503_MAC_MATCH_DIAG_SEL 0x40080c18
#define HH503_SOC_DIAG_CLK 0x4000008c

#define hmac_diag_reg_operation(reg, val, delay) \
    do { \
        hal_reg_write(reg, val); \
        osal_udelay(delay); \
    } while (0)

#endif /* _PRE_WLAN_FEATURE_DAQ */

osal_void h2d_cfg_init(osal_void)
{
    static const msg_hook_array_stru msg_hook_array[] = {
#ifdef _PRE_WLAN_FEATURE_FTM
        {WLAN_MSG_D2H_SEND_FTM_COMPLETE, hmac_process_send_ftm_complete},
#endif
        {WLAN_MSG_D2H_C_CUSTOM_CFG_PRIV_INI, hmac_config_custom_cfg_priv_ini},
        {WLAN_MSG_D2H_C_CFG_TX_COMP_SCHEDULE, hmac_config_tx_comp_schedule},
        {WLAN_MSG_H2H_C_CFG_TID_RESUME, hmac_config_tid_resume_process},
        {WLAN_MSG_D2H_C_CFG_CFG80211_DATA_TX_STATUS, hmac_config_tx_complete_data},
        {WLAN_MSG_D2H_C_CFG_KEEPALIVE_SYNC_TIMESTAMP, hmac_config_keepalive_sync_timestamp},
        {WLAN_MSG_D2H_C_CFG_SET_BEACON_PERIOD, hmac_config_set_beacon_period},
#ifdef _PRE_WLAN_FEATURE_P2P
        {WLAN_MSG_D2H_C_CFG_SET_P2P_NOA, hmac_config_set_p2p_noa_adapt},
#endif
    };

    frw_msg_hook_register_array(msg_hook_array, osal_array_size(msg_hook_array));
}

#define HMAC_CONFIG_ADC_MASK 0xffff
#define HMAC_CONFIG_DAC_MASK 0xffff0000
#define HMAC_CONFIG_DAC_OFFSET 16
#define HMAC_CONFIG_2G_START_CHAN_INDEX 1

#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
#define HMAC_CONFIG_2G_END_CHAN_INDEX 16
#else
#define HMAC_CONFIG_2G_END_CHAN_INDEX 13
#endif

#define HMAC_CONFIG_2G_CHAN_14 14

#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
#define HMAC_CONFIG_2G_CHAN_15 15
#define HMAC_CONFIG_2G_CHAN_16 16
#endif

#define HMAC_CONFIG_2G_START_FREQ 2412
#define HMAC_CONFIG_2G_CHAN_14_FREQ 2484

#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
#define HMAC_CONFIG_2G_CHAN_15_FREQ 2512
#define HMAC_CONFIG_2G_CHAN_16_FREQ 2384
#endif

#define HMAC_CONFIG_5G_START_CHAN_INDEX 36
#define HMAC_CONFIG_5G_END_CHAN_INDEX 184
#define HMAC_CONFIG_5G_START_FREQ 5180

#define HMAC_CONFIG_FREQ_OFFSET 5
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
#define QUICK_SEND_RESULT_SUCCESS   1
osal_u8 g_quick_send_result = QUICK_SEND_RESULT_INVALID;
#endif
typedef enum {
    CURRENT_TEMPERATURE_INDEX_ZERO,
    CURRENT_TEMPERATURE_INDEX_ONE,
    CURRENT_TEMPERATURE_INDEX_TWO,
    CURRENT_TEMPERATURE_INDEX_THREE,
    CURRENT_TEMPERATURE_INDEX_FOUR,
    CURRENT_TEMPERATURE_INDEX_FIVE,
    CURRENT_TEMPERATURE_INDEX_SIX,
    CURRENT_TEMPERATURE_INDEX_SEVEN,
} current_temperature;

#define CURRENT_TEMPERATURE_ACTUAL_VALUE_ZERO (-20)
#define CURRENT_TEMPERATURE_ACTUAL_VALUE_ONE (-7)
#define CURRENT_TEMPERATURE_ACTUAL_VALUE_TWO 18
#define CURRENT_TEMPERATURE_ACTUAL_VALUE_THREE 43
#define CURRENT_TEMPERATURE_ACTUAL_VALUE_FOUR 67
#define CURRENT_TEMPERATURE_ACTUAL_VALUE_FIVE 93
#define CURRENT_TEMPERATURE_ACTUAL_VALUE_SIX 115
#define CURRENT_TEMPERATURE_ACTUAL_VALUE_SEVEN 125

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
 4 函数实现
*****************************************************************************/

/*****************************************************************************
 功能描述  : 填充rsp，返回给对方，无需抛事件
*****************************************************************************/
osal_u32 hmac_fill_sys_rsp(frw_msg *msg, osal_u16 len, const osal_u8 *param)
{
    msg->rsp_len = len;
    if (memcpy_s(msg->rsp, len, param, len) != EOK) {
        oam_warning_log0(0, OAM_SF_TX, "hmac_fill_sys_rsp::memcpy_s err!");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置模式 包括协议 频段 带宽
 输入参数  : mac_vap: 指向VAP的指针
           len     : 参数长度
           param  : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_mode(hmac_vap_stru *hmac_vap, const mac_cfg_mode_param_stru *old_param)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
    if ((hmac_vap == OSAL_NULL) || (hmac_vap->hal_device == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_config_set_mode::hal_device == NULL}");
        return OAL_FAIL;
    }
#endif
#ifdef _PRE_WLAN_ONLINE_DPD
    if (hmac_vap->protocol == WLAN_LEGACY_11B_MODE) {
        hal_dpd_cfr_set_11b(hmac_vap->hal_device, OSAL_TRUE);
    } else {
        hal_dpd_cfr_set_11b(hmac_vap->hal_device, OSAL_FALSE);
    }
#endif

    oam_warning_log3(0, 0, "{hmac_config_set_mode::protocol[%d], band[%d], bw[%d]}", hmac_vap->protocol,
        hmac_vap->channel.band, hmac_vap->channel.en_bandwidth);

    if (hmac_vap->protocol == WLAN_HE_MODE) {
        hal_set_dev_support_11ax(hmac_vap->hal_device, OSAL_TRUE);
    }

    /* 更新device的频段及最大带宽信息 */
    hmac_device->max_bandwidth = hmac_vap->channel.en_bandwidth;
    hmac_device->max_band = hmac_vap->channel.band;
    hmac_chan_select_channel_mac(hmac_vap, hmac_vap->channel.chan_number, mac_vap_get_cap_bw(hmac_vap));

#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_m2s_spec_update_hal_device_capbility(hmac_vap->hal_device, hmac_vap->channel.band);
#endif

#if defined(_PRE_WLAN_FEATURE_DFS_OPTIMIZE)
    if (hmac_device->max_band == WLAN_BAND_5G) {
        hal_radar_config_reg_bw(hmac_vap->hal_device->dfs_radar_filter.radar_type, hmac_vap->channel.en_bandwidth);
    }
#endif

    if (old_param->protocol != hmac_vap->protocol || old_param->band != hmac_vap->channel.band ||
        old_param->en_bandwidth != hmac_vap->channel.en_bandwidth) {
        hmac_vap_sync(hmac_vap);
    }
    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_config_change_chan_idx_2_center_freq(osal_u32 chan_idx, osal_u32 *center_freq)
{
    if ((chan_idx >= HMAC_CONFIG_2G_START_CHAN_INDEX) && (chan_idx <= HMAC_CONFIG_2G_END_CHAN_INDEX)) {
        *center_freq = (chan_idx - HMAC_CONFIG_2G_START_CHAN_INDEX) * HMAC_CONFIG_FREQ_OFFSET +
            HMAC_CONFIG_2G_START_FREQ;
        return OAL_SUCC;
    }

    if ((chan_idx >= HMAC_CONFIG_5G_START_CHAN_INDEX) && (chan_idx <= HMAC_CONFIG_5G_END_CHAN_INDEX)) {
        *center_freq = (chan_idx - HMAC_CONFIG_5G_START_CHAN_INDEX) * HMAC_CONFIG_FREQ_OFFSET +
            HMAC_CONFIG_5G_START_FREQ;
        return OAL_SUCC;
    }

    if (chan_idx == HMAC_CONFIG_2G_CHAN_14) {
        *center_freq = HMAC_CONFIG_2G_CHAN_14_FREQ;
        return OAL_SUCC;
    }
#ifdef _PRE_WLAN_FEATURE_015CHANNEL_EXP
    if (chan_idx == HMAC_CONFIG_2G_CHAN_15) {
        *center_freq = HMAC_CONFIG_2G_CHAN_15_FREQ;
        return OAL_SUCC;
    }

    if (chan_idx == HMAC_CONFIG_2G_CHAN_16) {
        *center_freq = HMAC_CONFIG_2G_CHAN_16_FREQ;
        return OAL_SUCC;
    }
#endif
    return OAL_FAIL;
}

/*****************************************************************************
 功能描述  : 设置信道
 输入参数  : mac_vap: 指向VAP的指针
           len     : 参数长度
           param  : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_freq(hmac_vap_stru *hmac_vap, osal_u8 value)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 channel_idx;
    mac_cfg_channel_param_stru channel_param;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    osal_u32 center_freq;
    osal_void *psd_switch_hook = hmac_get_feature_fhook(HMAC_FHOOK_PSD_GET_SWITCH);
    osal_u8 psd_switch = OSAL_FALSE;
    osal_u8 old_chan_number;

    if (value >= 36) { /* 36:5G起始信道号 */
        hmac_vap->channel.band = WLAN_BAND_5G;
    } else {
        hmac_vap->channel.band = WLAN_BAND_2G;
    }
    if (hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, value, &channel_idx) != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_freq::get ch idx failed, band[%d], channel num[%d].}", hmac_vap->vap_id,
            hmac_vap->channel.band, (osal_u8)value);
        return OAL_FAIL;
    }

    old_chan_number = hmac_vap->channel.chan_number;
    hmac_vap->channel.chan_number = value;
    hmac_vap->channel.chan_idx = channel_idx;
    if (old_chan_number != hmac_vap->channel.chan_number) {
        hmac_vap_sync(hmac_vap);
    }

    hmac_device_get_channel_etc(hmac_device, &channel_param);
    channel_param.channel = (osal_u8)value;
    hmac_device_set_channel_etc(hmac_device, &channel_param);
    /* 非IDLE状态直接配置信道 */
    /* 常收状态下直接配置信道 */
    /* PSD使能状态直接配置信道 */
    if (psd_switch_hook != OSAL_NULL) {
        psd_switch = ((hmac_psd_get_switch_cb)psd_switch_hook)();
    }
    if ((hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_IDLE_STATE) ||
        (hal_device->al_rx_flag == HAL_ALWAYS_RX_RESERVED) || (psd_switch == OSAL_TRUE)) {
        hmac_vap_work_set_channel(hmac_vap);
    }
#ifdef _PRE_WLAN_FEATURE_DFS
    hmac_config_cali_dfs_handle(hal_device, hmac_vap, hmac_device);
#endif
    if (hmac_config_change_chan_idx_2_center_freq((osal_u32)value, &center_freq) != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_change_chan_idx_2_center_freq fail, chan_num[%d].}",
            hmac_vap->vap_id, value);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
/*****************************************************************************
 功能描述  : 停止PNO调度扫描
 输入参数  : mac_vap: 指向VAP的指针
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_stop_sched_scan(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    if (msg->data == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_config_stop_sched_scan::param is NULL.}");
    }

    /* 判断pno是否已经停止，如果停止，则pno扫描的管理结构体指针为null */
    if (hmac_device->pno_sched_scan_mgmt == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_config_stop_sched_scan::pno sched scan already stop.}");
        return OAL_SUCC;
    }

    oam_warning_log0(0, OAM_SF_SCAN, "{hmac_config_stop_sched_scan::pno stop.}");
    /* 删除PNO调度扫描定时器 */
    hmac_scan_stop_pno_sched_scan_timer(hmac_device->pno_sched_scan_mgmt);

    /* 停止本次PNO调度扫描并调用一次扫描结束 */
    if ((hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) &&
        (hmac_device->scan_params.scan_mode == WLAN_SCAN_MODE_BACKGROUND_PNO)) {
        hmac_scan_abort(hmac_device);
    }

    /* 释放PNO管理结构体内存 */
    oal_mem_free(hmac_device->pno_sched_scan_mgmt, OAL_TRUE);
    hmac_device->pno_sched_scan_mgmt = OSAL_NULL;

    oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_stop_sched_scan::stop schedule scan success.}", hmac_vap->vap_id);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 扫描终止
 输入参数  : mac_vap: 指向VAP的指针
           len     : 参数长度
           param  : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_void hmac_config_scan_abort(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 如果扫描正在进行则停止扫描 */
    if (hmac_device->curr_scan_state == MAC_SCAN_STATE_RUNNING) {
        oam_warning_log1(0, OAM_SF_SCAN, "vap_id[%d] {hmac_config_scan_abort::stop scan.}", hmac_vap->vap_id);
        hmac_scan_abort(hmac_device);
    }

    return;
}
/*****************************************************************************
 功能描述  : 是否需要锁定信号桥的信道
 输入参数  : hmac_device_stru    *hmac_device
 返 回 值: oal_bool_enum
*****************************************************************************/
OAL_STATIC oal_bool_enum hmac_config_whether_lock_rpt_channel(const hmac_device_stru *hmac_device)
{
    hmac_vap_stru *index_mac_vap = OSAL_NULL;
    mac_channel_stru *sta_channel = OSAL_NULL;
    osal_u8 vap_idx;

    if (hmac_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_config_whether_lock_rpt_channel:OSAL_NULL == hmac_device}\r\n");
        return OSAL_FALSE;
    }

    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        index_mac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if ((index_mac_vap == OSAL_NULL) || (is_p2p_mode(index_mac_vap->p2p_mode))) {
            continue;
        }
        if ((index_mac_vap->vap_state != MAC_VAP_STATE_UP) && (index_mac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }

        if (index_mac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA) {
            continue;
        }
        /* STA先启动;信号桥后启动 */
        sta_channel = &index_mac_vap->channel;
        oam_warning_log3(0, OAM_SF_ANY,
            "{hmac_config_whether_lock_rpt_channel::STA: number = %d,chan_idx = %d, band = %d(0: 2.4G, 1: 5G)}\r\n",
            sta_channel->chan_number, sta_channel->chan_idx, sta_channel->band);

        if (sta_channel->band == WLAN_BAND_5G) { // 如果STA工作在5G信道
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_whether_lock_rpt_channel::STA is on 5G}\r\n");
            /* 如果国家码的5G信道包含100~120信道(会涉及并发扫描干扰的问题) */
            if (hmac_is_channel_num_valid_etc(WLAN_BAND_5G, 100) == OAL_SUCC) {
                return OSAL_TRUE;
            }
            oam_warning_log0(0, OAM_SF_ANY,
                "{hmac_config_whether_lock_rpt_channel::channels(100~120) are not in Reg Domain}\r\n");
        } else {
            oam_warning_log0(0, OAM_SF_ANY,
                "{hmac_config_whether_lock_rpt_channel::STA is on 2.4G,no need to change repeater channel}\r\n");
        }
    }

    return OSAL_FALSE;
}

OAL_STATIC osal_void hmac_config_set_channel_ext(hmac_vap_stru *hmac_vap, const hmac_device_stru *hmac_device,
    const hal_to_dmac_device_stru *hal_device, mac_cfg_channel_param_stru *channel_param)
{
    unref_param(channel_param);
    unref_param(hmac_device);

    /* disable TSF */
    hal_disable_tsf_tbtt(hmac_vap->hal_vap);

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 使能去使能雷达检测 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (mac_vap_get_dfs_enable(hmac_vap) == OSAL_TRUE)) {
        oal_bool_enum_uint8 enable_dfs;
        enable_dfs = hmac_is_cover_dfs_channel(hmac_device->max_band, hmac_device->max_bandwidth,
            hmac_device->max_channel);
        hal_enable_radar_det(enable_dfs);
#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
        if (osal_likely(hal_device != OSAL_NULL) &&
            hal_device->dfs_radar_filter.dfs_normal_pulse_det.timer.is_enabled == OSAL_TRUE) {
            /* 取消定时器 */
            frw_destroy_timer_entry(&(hal_device->dfs_radar_filter.dfs_normal_pulse_det.timer));
            hal_device->dfs_radar_filter.dfs_normal_pulse_det.timer_start = OSAL_FALSE;
        }
#endif
    }
#endif

    if (channel_param->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_config_set_channel_ext:: fpga is not support 80M.}", hmac_vap->vap_id);
        channel_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    /* 非IDLE状态直接配置信道 */
    if (osal_likely(hal_device != OSAL_NULL) && hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_IDLE_STATE) {
        hmac_vap_work_set_channel(hmac_vap);
    }
    /* 解决在APUT准备好Beacon帧信息之前发送就启动TSF，致使TBTT中断上报后Beacon Buf为空的问题，临时修改方案 */
#ifdef _PRE_WLAN_FEATURE_DBAC
#ifndef _PRE_WLAN_FEATURE_WS92_MERGE
    if (!mac_is_dbac_enabled(hmac_device)) {
        hal_enable_tsf_tbtt(hmac_vap->hal_vap, OSAL_FALSE);
    }
#else
    if (mac_is_dbac_running(hmac_device) == OSAL_FALSE) {
        if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (hmac_vap->p2p_mode == WLAN_P2P_GO_MODE)) {
            hal_enable_tsf_tbtt(hmac_vap->hal_vap, OSAL_FALSE);
        }
    }
#endif
#endif
}

#ifdef _PRE_WLAN_WIRELESS_EXT
static osal_void hmac_handle_wal_wait_rsp(hmac_vap_stru *hmac_vap, hmac_to_wal_sync_data_stru *msg)
{
    frw_msg msg_info = {0};
    osal_s32 ret;

    if (osal_unlikely(hmac_vap == OSAL_NULL || msg == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_ASSOC, "{hmac_handle_scan_rsp_sta_etc::hmac_vap(%p) or msg(%p) is null}",
            (uintptr_t)hmac_vap, (uintptr_t)msg);
        return;
    }

    msg_info.data = (osal_u8*)msg;
    msg_info.data_len = (osal_u16)OAL_SIZEOF(hmac_to_wal_sync_data_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_IWLIST_SCAN_RSP, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_RRM, "{hmac_handle_wal_wait_rsp: frw_asyn_host_post_msg failed[%d]!}", ret);
    }
    return;
}

static osal_void hmac_config_start_iwlist_scan_db(void *scan_record_ptr)
{
    hmac_scan_record_stru *scan_record = OSAL_NULL;
    hmac_vap_stru         *hmac_vap = OSAL_NULL;
    hmac_to_wal_sync_data_stru sync_param;

    /* 判断入参合法性 */
    if (scan_record_ptr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_config_start_iwlist_scan_db: input pointer is null!}");
        return;
    }

    memset_s(&sync_param, sizeof(sync_param), 0, sizeof(sync_param));
    scan_record = (hmac_scan_record_stru *)scan_record_ptr;
    /* 获取hmac VAP */
    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(scan_record->vap_id);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_RRM, "{hmac_config_start_iwlist_scan_db: hmac_vap or rrm_info is null!}");
        return;
    }

    /* 判断扫描状态是否成功 */
    if (scan_record->scan_rsp_status != MAC_SCAN_SUCCESS) {
        wifi_printf("hmac_config_start_iwlist_scan_db:scan fail, status[%d]\r\n", scan_record->scan_rsp_status);

        oam_warning_log2(0, OAM_SF_RRM, "vap_id[%d] hmac_config_start_iwlist_scan_db::scan fail, status[%d].",
            hmac_vap->vap_id, scan_record->scan_rsp_status);
        sync_param.status = OSAL_FALSE;
    } else {
        sync_param.status = OSAL_TRUE;
    }

    /* 填充mode_id */
    sync_param.mode_id = IWLIST_SET_SCAN_MODE;
    hmac_handle_wal_wait_rsp(hmac_vap, &sync_param);
    return;
}

static osal_void hmac_config_iw_scan_fill_chan(mac_scan_req_stru *scan_req)
{
    /* 填充扫描信道，当前触发全频全信道扫描 */
    osal_u8    chan_count;
    osal_u8    chan_idx;
    osal_u8    chan_num = 0;
    osal_u8    chan_avail_idx = 0;
    wlan_channel_band_enum_uint8 chan_band;

    /* 填充2.4G */
    chan_band = WLAN_BAND_2G;
    chan_count = (osal_u8)MAC_CHANNEL_FREQ_2_BUTT;
    for (chan_idx = 0; chan_idx < chan_count; chan_idx++) {
        /* 判断信道是不是在管制域内 */
        if (hmac_is_channel_idx_valid_etc(chan_band, chan_idx) != OAL_SUCC) {
            continue;
        }

        hmac_get_channel_num_from_idx_etc(chan_band, chan_idx, &chan_num);
        if (chan_avail_idx <= WLAN_MAX_CHANNEL_NUM - 1) { // 1 数组下标从0开始
            scan_req->channel_list[chan_avail_idx].chan_number = chan_num;
            scan_req->channel_list[chan_avail_idx].band        = chan_band;
            scan_req->channel_list[chan_avail_idx++].chan_idx  = chan_idx;
        }
    }

    scan_req->channel_nums = chan_avail_idx;
    return;
}

static osal_u32 hmac_config_start_iwlist_scan_fill(mac_scan_req_stru *scan_req, const hmac_vap_stru *hmac_vap,
    iwlist_scan_essid_enable_stru *essid_info)
{
    wlan_vap_mode_enum_uint8 vap_mode = hmac_vap->vap_mode;

    hmac_config_iw_scan_fill_chan(scan_req);
    if (vap_mode == WLAN_VAP_MODE_BSS_STA) {
        scan_req->scan_mode = WLAN_SCAN_MODE_BACKGROUND_STA;
    } else if (vap_mode == WLAN_VAP_MODE_BSS_AP) {
        scan_req->scan_mode = WLAN_SCAN_MODE_BACKGROUND_AP;
    } else {
        oam_error_log1(0, OAM_SF_SCAN,
            "{hmac_config_start_iwlist_scan_fill::vap mode[%d], not support bg scan.}", vap_mode);
        return OAL_FAIL;
    }

    /* 主动扫描, 测量时间 每个信道扫描一次 */
    scan_req->scan_type = WLAN_SCAN_TYPE_ACTIVE;
    scan_req->scan_time = WLAN_LONG_ACTIVE_SCAN_TIME;
    scan_req->bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;

    scan_req->bssid_num = 0;
    scan_req->vap_id = hmac_vap->vap_id;
    scan_req->scan_func = MAC_SCAN_FUNC_BSS;
    scan_req->max_scan_count_per_channel = 1; /* 每个信道扫描一次 */
    scan_req->max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;

    scan_req->need_switch_back_home_channel = OSAL_TRUE;
    scan_req->work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
    memcpy_s(scan_req->sour_mac_addr, sizeof(scan_req->sour_mac_addr), mac_mib_get_station_id(hmac_vap),
        WLAN_MAC_ADDR_LEN);

    if (essid_info->essid_enable == 1) {
        scan_req->ssid_num = 1;
        memcpy_s(scan_req->mac_ssid_set[0].ssid, WLAN_SSID_MAX_LEN, essid_info->essid, WLAN_SSID_MAX_LEN);
    }

    /* 回调函数指针 不需要处理 */
    scan_req->fn_cb = hmac_config_start_iwlist_scan_db;
    return OAL_SUCC;
}

osal_s32 hmac_config_start_iwlist_scan(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    mac_scan_req_stru scan_req;
    hmac_device_stru *hmac_device = OSAL_NULL;
    iwlist_scan_essid_enable_stru *essid_info = OSAL_NULL;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_M2U, "{hmac_config_start_iwlist_scan:null ptr}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(&scan_req, sizeof(mac_scan_req_stru), 0, sizeof(mac_scan_req_stru));

    essid_info = (iwlist_scan_essid_enable_stru *)msg->data;
    ret = hmac_config_start_iwlist_scan_fill(&scan_req, hmac_vap, essid_info);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 保存扫描参数，防止扫描参数被释放 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (osal_unlikely(hmac_device == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memcpy_s((mac_scan_req_stru *)&(hmac_device->scan_params), sizeof(mac_scan_req_stru),
        (mac_scan_req_stru *)&scan_req, sizeof(mac_scan_req_stru));

    /* 直接调用扫描模块扫描请求处理函数 */
    ret = hmac_scan_proc_scan_req_event_etc(hmac_vap, &scan_req);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_RRM, "hmac_config_start_iwlist_scan:hmac_scan_add_req failed, ret=%d", ret);
    }

    return (osal_s32)ret;
}
#endif
/*****************************************************************************
 功能描述  : HOSTAPD 设置信道频段，信道号和带宽
 输入参数  : hmac_vap_stru *mac_vap
           osal_u8 len
           osal_u8 *param
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_channel(hmac_vap_stru *hmac_vap, mac_cfg_channel_param_stru *channel_param)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 channel_idx = 0;
    hal_tx_ctrl_desc_rate_stru *tx_dsrc = OSAL_NULL;
    hh503_tx_phy_rate_stru beacon_rate;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    osal_u32 ret;
    osal_u8 old_bandwith, old_chan_number;
    hal_tx_txop_alg_stru *txop_param = OSAL_NULL;

    if ((hmac_vap->p2p_mode == WLAN_P2P_GO_MODE) && (hmac_vap->app_ie[OAL_APP_BEACON_IE].ie_len == 0)) {
        // 如果是P2P Go模式 且Beancon帧不包含P2P ie即为信号桥
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_set_channel::It is a repeater}\r\n");
        if ((hmac_config_whether_lock_rpt_channel(hmac_device) == (oal_bool_enum)OSAL_TRUE) &&
            (channel_param->band == WLAN_BAND_2G)) {
            channel_param->channel = 1;                        // 1信道
            channel_param->en_bandwidth = WLAN_BAND_WIDTH_20M; // 20M 带宽
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_set_channel::set repeater channel to 1}\r\n");
        }
    }

    ret = hmac_get_channel_idx_from_num_etc(channel_param->band, channel_param->channel, &channel_idx);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_config_set_channel:invalid ch idx:%d}", hmac_vap->vap_id, channel_idx);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    old_bandwith = hmac_vap->channel.en_bandwidth;
    old_chan_number = hmac_vap->channel.chan_number;
    hmac_vap->channel.chan_number = channel_param->channel;
    hmac_vap->channel.band = channel_param->band;
    hmac_vap->channel.en_bandwidth = channel_param->en_bandwidth;

    /* 信道索引号未保存，导致背景扫描结束后恢复时PHY寄存器配置是VAP工作信道，但实际PLL锁频在默认值0，即信道1 */
    hmac_vap->channel.chan_idx = channel_idx;

    if (old_bandwith != hmac_vap->channel.en_bandwidth || old_chan_number != hmac_vap->channel.chan_number) {
        hmac_vap_sync(hmac_vap);
    }

    hmac_device_set_channel_etc(hmac_device, channel_param);

    memset_s(&beacon_rate, sizeof(hh503_tx_phy_rate_stru), 0, sizeof(hh503_tx_phy_rate_stru));
    /* 更新beacon的发送参数 */
    if ((channel_param->band == WLAN_BAND_2G) || (channel_param->band == WLAN_BAND_5G)) {
        txop_param = &(hmac_vap->tx_mgmt_bmcast[channel_param->band]);
        tx_dsrc = (hal_tx_ctrl_desc_rate_stru *)&(hal_txop_alg_get_tx_dscr(txop_param)[0]);
        hal_chan_update_beacon_rate(&beacon_rate, tx_dsrc);
        hal_vap_set_beacon_rate(hmac_vap->hal_vap, *(osal_u32 *)(uintptr_t)(&beacon_rate));
    }
    hmac_config_set_channel_ext(hmac_vap, hmac_device, (const hal_to_dmac_device_stru *)hal_device, channel_param);
    oam_warning_log3(0, OAM_SF_ANY, "{hmac_config_set_channel::channel[%u] bw[%u], beacon rate[%d]}",
        channel_param->channel, channel_param->en_bandwidth, *(osal_u32 *)(uintptr_t)(&beacon_rate));
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
/*****************************************************************************
 功能描述  : sta模式下通过beacon的rssi来判断管理帧是通过c0还是c1发送
 输入参数  : 无
 返 回 值: void
*****************************************************************************/
OAL_STATIC osal_void hmac_connect_ant_change(const hmac_vap_stru *hmac_vap, const mac_conn_security_stru *conn)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    unref_param(conn);
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_RX, "vap_id[%d] {hmac_connect_ant_change::hal_device null.}",
            hmac_vap->vap_id);
        return;
    }

    /* 不为MIMO，不处理 */
    if (hal_device->hal_m2s_fsm.oal_fsm.cur_state != HAL_M2S_STATE_MIMO) {
        return;
    }
#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_m2s_mgmt_switch_judge(hal_device, hmac_vap, conn->ant0_rssi, conn->ant1_rssi);
#endif
}
#endif
/*****************************************************************************
 功能描述  : dmac 处理connect请求，
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_connect(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    mac_conn_security_stru *conn = OSAL_NULL;
    frw_msg msg2dev = {0};

    conn = (mac_conn_security_stru *)msg->data;

    /* 更新上报的RSSI */
    /* 处理connect时，初始化一次rssi的值，保证dmac_vap下保存的值最新 */
    hmac_vap->query_stats.signal = OAL_RSSI_INIT_MARKER;

    oal_rssi_smooth(&(hmac_vap->query_stats.signal), (osal_s8)conn->rssi);

    oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d] hmac_config_connect::c_ant0_rssi[%d],c_ant1_rssi[%d]", hmac_vap->vap_id,
                     conn->ant0_rssi, conn->ant1_rssi);
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
    hmac_connect_ant_change(hmac_vap, conn);
#endif

    /* 抛事件至Device侧DMAC，同步START CONNECT */
    msg2dev.data = (osal_u8 *)&(conn->rssi);
    msg2dev.data_len = (osal_u16)OAL_SIZEOF(osal_s8);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_CONNECT_REQ, &msg2dev, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {frw_send_msg_to_device::fail [%d].}", hmac_vap->vap_id, ret);
        return ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_OS_VERSION_LINUX
/*****************************************************************************
 功能描述  : 降SAR
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_reduce_sar(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    static osal_u8 sar_lvl = 0;
    oam_warning_log1(0, OAM_SF_TPC, "vap_id[%d] {hmac_config_reduce_sar::ENTER!}", hmac_vap->vap_id);
    if (msg == OSAL_NULL || msg->data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TX, "hmac_config_reduce_sar::msg err!");
        return OAL_FAIL;
    }
    // 等级不变，不更新
    if (sar_lvl == (*msg->data)) {
        return OAL_SUCC;
    }
    sar_lvl = (*msg->data);
    /* 刷新发送功率 */
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_REFRESH);

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
osal_void hmac_config_set_quick_result(osal_u8 result)
{
    if ((result == QUICK_SEND_RESULT_INVALID) || (g_quick_send_result != QUICK_SEND_RESULT_SUCCESS)) {
        g_quick_send_result = result;
    }
    return;
}

osal_u8 hmac_config_get_quick_result(osal_void)
{
    return g_quick_send_result;
}

static osal_s32 hmac_config_get_fast_result(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    wifi_printf("hmac_config_get_quick_result: %u.\n", hmac_config_get_quick_result());
    return OAL_SUCC;
}

static osal_s32 hmac_config_reset_fast_result(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    wifi_printf("reset al_tx_fast result succ.\r\n");
    hmac_config_set_quick_result(QUICK_SEND_RESULT_INVALID);
    return OAL_SUCC;
}
#endif
static osal_u8 hmac_config_set_custom_pkt_header_len(osal_u8 sub_type, osal_u8 type)
{
    if (type != WLAN_FC0_TYPE_CTL) {
        return MAC_80211_FRAME_LEN;
    }

    if (sub_type == WLAN_CTS || sub_type == WLAN_ACK) {
        return MAC_CTS_FRAME_LEN;
    }

    return MAC_CONTROL_FRAME_LEN;
}

OAL_STATIC osal_u32 hmac_custom_pkt_send_timeout_fn(void *p_arg)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)p_arg;
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 单STA场景，且非关联状态，非扫描时，关闭PA，切回IDLE状态 */
    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user == OSAL_NULL && hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA &&
        hmac_device_calc_up_vap_num_etc(hmac_device) == 1 &&
        hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_custom_pkt_send_timeout_fn::recovery pa.}");
        hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_VAP_DOWN, sizeof(hal_to_dmac_vap_stru),
            (osal_u8 *)(hmac_vap->hal_vap));
        hal_disable_machw_phy_and_pa();
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hmac_config_prepare_custom_pkt_send(hmac_vap_stru *hmac_vap, osal_u16 user_idx)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    /* 如果非单STA,则直接返回 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA || hmac_device_calc_up_vap_num_etc(hmac_device) > 1) {
        return;
    }

    /* 单STA场景，IDLE状态，发送报文需要打开PA */
    if (hmac_vap->hal_device->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_IDLE_STATE &&
        user_idx == MAC_INVALID_USER_ID) {
        hmac_vap_work_set_channel(hmac_vap);
        hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_JOIN_COMP, sizeof(hal_to_dmac_vap_stru),
                                (osal_u8 *)(hmac_vap->hal_vap));
        hal_enable_machw_phy_and_pa(hmac_vap->hal_device);
        frw_create_timer_entry(&g_custom_pkt_send_timer, hmac_custom_pkt_send_timeout_fn,
            HMAC_CUSTOM_PKT_SEND_TIME, hmac_vap, OSAL_FALSE);
    }
}

/*****************************************************************************
 函 数 名: hmac_config_send_custom_pkt
 功能描述  : 发送用户定制报文到dmac
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_send_custom_pkt(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_netbuf_stru *netbuf = OSAL_NULL;
    osal_u32 len, ret;
    osal_bool data_ready;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;
    osal_u16 user_idx = MAC_INVALID_USER_ID;
    mac_ieee80211_frame_stru *mac_header = OSAL_NULL;   /* 保存用户目的地址的指针 */

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_send_custom_pkt:: hmac_vap or msg is Null .}");
        return OAL_FAIL;
    }
    len = msg->data_len;
    /* 申请管理帧内存 */
    netbuf = oal_netbuf_alloc_ext(OAL_MGMT_NETBUF, len, OAL_NETBUF_PRIORITY_MID);
    if (netbuf == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_send_custom_pkt::alloc netbuf failed len[%d].}", len);
        return OAL_FAIL;
    }
    oal_set_netbuf_prev(netbuf, OSAL_NULL);
    oal_set_netbuf_next(netbuf, OSAL_NULL);
    if (memset_s(oal_netbuf_cb(netbuf), len, 0, len) != EOK) {
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }
    /* 复制用户数据 */
    if (memcpy_s(oal_netbuf_header(netbuf), len, msg->data, len) != EOK) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_send_custom_pkt::pkt->data memcpy fail len[%d].}", len);
        oal_netbuf_free(netbuf);
        return OAL_FAIL;
    }
    oal_netbuf_set_len(netbuf, len);
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);
    tx_ctl->is_custom_rts = (len == 16) ? OSAL_TRUE : OSAL_FALSE; /* rts帧的长度为16 */
    tx_ctl->frame_header_length = hmac_config_set_custom_pkt_header_len(msg->data[0] & 0xF0, msg->data[0] & 0x0F);

    /* 1、判断现在的fc(frame control)字段，to ds or from ds */
    mac_header = (mac_ieee80211_frame_stru *)msg->data;
    /* 2、拿到正确的对端(APUT)的mac地址，调用hmac_vap_find_user_by_macaddr_etc() */
    ret = (mac_header->frame_control.to_ds == 1) ? hmac_vap_find_user_by_macaddr_etc(hmac_vap,
        mac_header->address3, &user_idx) : hmac_vap_find_user_by_macaddr_etc(hmac_vap, mac_header->address1, &user_idx);
    /* 3、tx_user_idx 赋值 */
    tx_ctl->tx_user_idx = user_idx;
    /* 4、如果是未关联状态下，准备报文发送时所需配置 */
    hmac_config_prepare_custom_pkt_send(hmac_vap, user_idx);
    tx_ctl->is_custom = OSAL_TRUE;
    tx_ctl->ismcast = OSAL_TRUE;
    tx_ctl->mpdu_payload_len = (osal_u16)(len - tx_ctl->frame_header_length);
    tx_ctl->ac = WLAN_WME_AC_MGMT;
    tx_ctl->ack_policy = WLAN_TX_NORMAL_ACK;
    /* 之前只有长度为16的RTS帧才为TRUE，现在是所有的任意帧都为TRUE，此种情况下不会对报文内容进行padding操作 */
    data_ready = OSAL_TRUE;

    /* 调用发送管理帧接口 */
    ret = hmac_tx_mgmt(hmac_vap, netbuf, (osal_u16)len, data_ready);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(netbuf);
    }
    return (osal_s32)ret;
}

/*****************************************************************************
 功能描述  : 获取dieid
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_get_dieid(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_show_dieid_stru dieid = { 0 };

    hal_get_dieid(hmac_vap->hal_device, dieid.dieid, &dieid.length);
    return (osal_s32)hmac_fill_sys_rsp(msg, sizeof(mac_cfg_show_dieid_stru), (osal_u8 *)&dieid);
}

/********************************************************
功能描述  : 当前是ax协议，打开rom功能。当前是非ax协议，关闭ax功能。
 *******************************************************/
osal_void hmac_config_rx_he_rom_en_handle(const hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *another_up_vap = OSAL_NULL;

    /* 当前vap是ax 协议,      开启ROM功能 */
    if (mac_vap_is_work_he_protocol(hmac_vap) != 0) {
        hal_set_he_rom_en(OSAL_FALSE);
        hal_set_he_rom_en(OSAL_TRUE);
        return;
    }

    /* another vap ax */
    another_up_vap = mac_device_find_another_up_vap_etc(hmac_device, hmac_vap->vap_id);
    if ((another_up_vap != OSAL_NULL) && (mac_vap_is_work_he_protocol(another_up_vap) != 0)) {
        /* another vap 工作在ax， reset cfg_rx_he_rom_en */
        hal_set_he_rom_en(OSAL_FALSE);
        hal_set_he_rom_en(OSAL_TRUE);
        return;
    }

    hal_set_he_rom_en(OSAL_FALSE);
    return;
}

/*****************************************************************************
 功能描述  : 设置dmac 发送功率
 输入参数  : mac_vap: 指向VAP的指针
           len     : 参数长度
           param  : 参数
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_txpower(hmac_vap_stru *hmac_vap, osal_u8 value)
{
    mac_regclass_info_stru *regdom_info = OSAL_NULL;
    osal_u8 cur_ch_num;
    wlan_channel_band_enum_uint8 freq_band;
    osal_u8 value_tx_power = 0;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_config_set_txpower::hal_device null}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    /* 设置管制域最大功率以控制TPC发送最大功率 */
    freq_band = hmac_vap->channel.band;
    cur_ch_num = hmac_vap->channel.chan_number;

    regdom_info = hmac_get_channel_num_rc_info_etc(freq_band, cur_ch_num);
    if (regdom_info == OSAL_NULL) {
        oam_error_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_txpower::this ch isnot support by country.freq_band = %d,cur_ch_num = %d}",
            hmac_vap->vap_id, freq_band, cur_ch_num);
        return OAL_FAIL;
    }
    hal_device->tx_pwr_ctrl.tx_power = value * HAL_POW_PRECISION_SHIFT; // 转换0.1dBm
    /* 设置vap的发送功率 */
    value_tx_power = (osal_u8)osal_min(value, regdom_info->max_reg_tx_pwr);
    oam_warning_log4(0, OAM_SF_CFG,
        "vap_id[%d] {hmac_config_set_txpower::max_reg_tx_pwr[%d] value_tx_power[%d] l_value[%d]!}",
        hmac_vap->vap_id, regdom_info->max_reg_tx_pwr, value_tx_power, value);
    hmac_vap_set_tx_power_etc(hmac_vap, value_tx_power);

    /* 刷新发送功率 */
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_REFRESH);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_ONLINE_DPD
osal_s32 hmac_dpd_to_hmac(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hal_cali_hal2hmac_event_stru *cali_tx_event = (hal_cali_hal2hmac_event_stru *)msg->data; /* wlan rx事件 */

    return hmac_sdt_up_dpd_data(cali_tx_event->netbuf);
}
#endif

osal_s32 hmac_config_adjust_tx_power(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_adjust_tx_power_stru *adjust_tx_power;
    osal_s32 ret;
    frw_msg msg_info;

    adjust_tx_power = (mac_cfg_adjust_tx_power_stru *)msg->data;
    hal_adjust_tx_power(adjust_tx_power->ch, adjust_tx_power->power);

    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ADJUST_TX_POWER, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SDP, "{hmac_config_adjust_tx_power frw_send_msg_to_device:: failed! ret[%d].}", ret);
    }

    return OAL_SUCC;
}

osal_s32 hmac_config_restore_tx_power(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    frw_msg msg_info;
    osal_u8 ch = *(msg->data);
    hal_restore_tx_power(ch);

    (osal_void)memset_s(&msg_info, sizeof(msg_info), 0, sizeof(msg_info));
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_RESTORE_TX_POWER, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SDP, "{hmac_config_restore_tx_power frw_send_msg_to_device:: failed! ret[%d].}", ret);
    }

    return OAL_SUCC;
}

osal_s32 hmac_config_set_rf_limit_power(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 power = *(msg->data);
    wlan_channel_band_enum_uint8 band = hmac_vap->channel.band;

    hal_set_rf_limit_power(band, power);
    oam_warning_log2(0, OAM_SF_ANY, "{hmac_config_set_rf_limit_power: tx band[%d], power[%d].}", band, power);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_ch_status_sync
 功能描述  : 同步信道信息
*****************************************************************************/
osal_s32 hmac_config_ch_status_sync_ext(hmac_vap_stru *hmac_vap, mac_ap_ch_info_stru *ch_info, osal_u16 info_len)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if ((hmac_vap == OSAL_NULL) || (ch_info == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_ch_status_sync_ext::vap=%p ch_info=%p}",
            (uintptr_t)hmac_vap, (uintptr_t)ch_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (info_len != sizeof(hmac_device->ap_channel_list)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_ch_status_sync_ext::invalid len=%d sizeof=%d}", info_len,
            sizeof(hmac_device->ap_channel_list));
        return OAL_FAIL;
    }

    (osal_void)memcpy_s((osal_u8 *)(hmac_device->ap_channel_list), sizeof(hmac_device->ap_channel_list),
        ch_info, info_len);

    return OAL_SUCC;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC osal_void hmac_get_tx_best_rate(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_rate_info_stru *rate_info)
{
    osal_u32 ret;
    frw_msg cfg_info = {0};
    mac_cfg_ar_tx_params_stru rsp_info = {0};
    mac_cfg_ar_tx_params_stru ar_tx_params = {0};

    ar_tx_params.user_id = hmac_user->assoc_id;
    memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
    cfg_msg_init((osal_u8 *)&ar_tx_params, OAL_SIZEOF(mac_cfg_ar_tx_params_stru),
        (osal_u8 *)&rsp_info, OAL_SIZEOF(mac_cfg_ar_tx_params_stru), &cfg_info);

    ret = (osal_u32)send_cfg_to_device(hmac_vap->vap_id, WLAN_MSG_W2D_C_CFG_GET_TX_PARAMS, &cfg_info, OSAL_TRUE);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_get_tx_best_rate::user_id = %u, ret:%d}",
            hmac_user->assoc_id, ret);
        return;
    }

    rate_info->legacy = rsp_info.tx_best_rate / 100; /* 转换为 100kbps 单位 */

    /* 打印tx_rate */
    oam_info_log2(0, OAM_SF_ANY, "{hmac_get_tx_best_rate::user_id = %u, tx_rate = %u kbps}",
        rsp_info.user_id, rsp_info.tx_best_rate);
    return;
}

/*****************************************************************************
 功能描述  : 获取station扩展统计信息
*****************************************************************************/
osal_void hmac_config_query_extend_info(const hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    hmac_stat_info_rsp_stru *station_info)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    wlan_channel_bandwidth_enum_uint8 bw = hmac_vap->channel.en_bandwidth;

    if (bw >= WLAN_BAND_WIDTH_BUTT) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_query_extend_info::invalid bw=%d.}",
            hmac_vap->vap_id, bw);
        return;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        /* RSSI取管理帧和数据帧平滑后的较大值 */
        osal_s8 signal_mgmt = (osal_s8)oal_get_real_rssi(hmac_vap->query_stats.signal);
        osal_s8 signal_mgmt_data = (osal_s8)oal_get_real_rssi(hmac_vap->query_stats.signal_mgmt_data);
        osal_s8 signal = osal_max(signal_mgmt, signal_mgmt_data);
        /* 上报的rssi都要小于0，否则会导致界面显示的信号格数异常 */
        station_info->signal = (signal < 0) ? signal : -1;

        if (bw == WLAN_BAND_WIDTH_20M) {
            station_info->free_power = hal_device->chan_ratio.free_power_20m;
            station_info->chload = hal_device->chan_ratio.chan_ratio_20m;
        } else if ((bw == WLAN_BAND_WIDTH_40PLUS) || (bw == WLAN_BAND_WIDTH_40MINUS)) {
            station_info->free_power = hal_device->chan_ratio.free_power_40m;
            station_info->chload = hal_device->chan_ratio.chan_ratio_40m;
        } else {
            station_info->free_power = hal_device->chan_ratio.free_power_80m;
            station_info->chload = hal_device->chan_ratio.chan_ratio_80m;
        }
    }

    hmac_get_tx_best_rate(hmac_vap, hmac_user, &(station_info->txrate));
}


/*****************************************************************************
 功能描述  : 处理上层下发的查询vap统计信息的事件
*****************************************************************************/
osal_s32 hmac_config_query_status(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    hmac_stat_info_rsp_stru rsp = {0};
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    dmac_query_request_event *requset = (dmac_query_request_event *)msg->data;

    if (hmac_vap == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_query_status::hmac_vap null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = mac_vap_get_hmac_user_by_addr_etc(hmac_vap, requset->query_sta_addr);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_query_status::hmac_user null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        hal_device = hmac_vap->hal_device;
        if (hal_device == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_query_status::vap id [%d],hal_device null}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        rsp.station_info_extend.distance = hal_device->hal_alg_stat.alg_distance_stat;
        rsp.station_info_extend.cca_intr = hal_device->hal_alg_stat.adj_intf_state;
        /* 从dmac获取RSSI,snr */
        rsp.signal           = (osal_char)oal_get_real_rssi(hmac_vap->query_stats.signal);
        rsp.station_info_extend.snr_ant0 = hmac_vap->query_stats.snr_ant0;
        rsp.station_info_extend.snr_ant1 = hmac_vap->query_stats.snr_ant1;

        /* 数据包统计，与维测不能放在同一个预编译宏下面 */
        rsp.rx_packets      = hmac_vap->query_stats.rx_d_from_hw_pkts;
        rsp.rx_bytes        = hmac_vap->query_stats.rx_rcv_data_bytes;
        rsp.rx_dropped_misc = hmac_vap->query_stats.rx_d_send_hmac_fail;
    } else if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
        /* 数据包统计，与维测不能放在同一个预编译宏下面 */
        rsp.rx_packets      = hmac_user->query_stats.drv_rx_pkts;
        rsp.rx_bytes        = hmac_user->query_stats.drv_rx_bytes;
        rsp.rx_dropped_misc = hmac_user->query_stats.rx_dropped_misc;
    } else {
        oam_warning_log2(0, 0, "vap_id[%d] {hmac_config_query_status::vap_mode[%d] don't support.}", hmac_vap->vap_id,
            hmac_vap->vap_mode);
        return OAL_FAIL;
    }

    hmac_config_query_extend_info(hmac_vap, hmac_user, &rsp);

    hmac_proc_query_response_event(hmac_vap, OAL_SIZEOF(hmac_stat_info_rsp_stru), (osal_u8 *)&rsp);
    if (memcpy_s(msg->rsp, msg->rsp_buf_len, &rsp, sizeof(rsp)) != EOK) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_query_status::memcpy_s fail.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }
    msg->rsp_len = sizeof(rsp);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_get_free_power
 功能描述  : 获取信道的空闲功率
*****************************************************************************/
osal_s8 hmac_config_get_free_power(const hmac_vap_stru *hmac_vap)
{
    osal_s8                            free_power;
    wlan_channel_bandwidth_enum_uint8   bw;
    const wlan_chan_ratio_stru *chan_ratio = OSAL_NULL;

    const hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    bw = hmac_vap->channel.en_bandwidth;
    if (bw >= WLAN_BAND_WIDTH_BUTT) {
        oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_get_free_power::invalid bw=%d.}", hmac_vap->vap_id, bw);
        return HMAC_INVALID_POWER;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_get_free_power::vap id [%d],hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    chan_ratio = &(hal_device->chan_ratio);

    if (bw == WLAN_BAND_WIDTH_20M) {
        free_power = chan_ratio->free_power_20m;
    } else if ((bw == WLAN_BAND_WIDTH_40PLUS) || (bw == WLAN_BAND_WIDTH_40MINUS)) {
        free_power = chan_ratio->free_power_40m;
    } else {
        free_power = chan_ratio->free_power_80m;
    }

    return free_power;
}

/*****************************************************************************
 函 数 名  : hmac_config_query_rssi
 功能描述  : 查询用户RSSI
*****************************************************************************/
osal_s32  hmac_config_query_rssi(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    mac_cfg_query_rssi_stru *param_user = OSAL_NULL;

    param_user = (mac_cfg_query_rssi_stru *)msg->data;
    hmac_user = mac_res_get_hmac_user_etc(param_user->user_id);
    if (hmac_user == OSAL_NULL) {
        param_user->rssi       = -127; /* 127表示当dmac_user为空时，信号强度设置为-127 */
    } else {
        param_user->rssi       = oal_get_real_rssi(hmac_user->rx_rssi);
    }
    param_user->free_power     = hmac_config_get_free_power(hmac_vap);

    if (hmac_fill_sys_rsp(msg, sizeof(mac_cfg_query_rssi_stru), (osal_u8 *)param_user) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_query_rssi::hmac_fill_sys_rsp fail.}",
            hmac_vap->vap_id);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_query_psst
 功能描述  : 查询用户省功耗状态
*****************************************************************************/
osal_s32  hmac_config_query_psst(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    mac_cfg_query_psst_stru *param_user = OSAL_NULL;

    param_user = (mac_cfg_query_psst_stru *)msg->data;
    hmac_user = mac_res_get_hmac_user_etc(param_user->user_id);
    if (hmac_user == OSAL_NULL) {
        param_user->ps_st = HMAC_INVALID_PS_STATE;
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_query_psst::INVALID ps state=%d.}", hmac_vap->vap_id, HMAC_INVALID_PS_STATE);
    } else {
        param_user->ps_st = hmac_user->ps_mode;
    }
    if (hmac_fill_sys_rsp(msg, sizeof(mac_cfg_query_psst_stru), (osal_u8 *)param_user) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_query_psst::hmac_fill_sys_rsp fail.}",
            hmac_vap->vap_id);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT
/*****************************************************************************
 函 数 名  : hmac_config_query_ani
 功能描述  : 查询VAP的抗干扰参数
*****************************************************************************/
osal_s32  hmac_config_query_ani(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    mac_cfg_query_ani_stru *param_user = OSAL_NULL;

    param_user = (mac_cfg_query_ani_stru *)msg->data;

    hal_device = hmac_vap->hal_device;

    param_user->device_distance       = hal_device->hal_alg_stat.alg_distance_stat;
    param_user->intf_state_cca        = hal_device->hal_alg_stat.adj_intf_state;
    param_user->intf_state_co         = hal_device->hal_alg_stat.co_intf_state;

    if (hmac_fill_sys_rsp(msg, sizeof(mac_cfg_query_ani_stru), (osal_u8 *)param_user) != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] hmac_config_query_ani:: hmac_fill_sys_rsp fail", hmac_vap->vap_id);
    }
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 函 数 名  : hmac_config_wfa_cfg_aifsn
 功能描述  : 设置aifsn固定值
*****************************************************************************/
osal_s32  hmac_config_wfa_cfg_aifsn(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_edca_cfg_stru *edca_cfg_param = OSAL_NULL;

    /* 参数合法性判断 */
    if (msg->data == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_wfa_cfg_aifsn::input params is invalid,%p.}",
            (uintptr_t)msg->data);
        return OAL_FAIL;
    }

    unref_param(hmac_vap);

    edca_cfg_param = (mac_edca_cfg_stru *)msg->data;

    hal_vap_set_machw_aifsn_ac_wfa(edca_cfg_param->ac, (osal_u8)edca_cfg_param->val, edca_cfg_param->switch_code);
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_config_wfa_cfg_cw
 功能描述  : 设置cw固定值
*****************************************************************************/
osal_s32  hmac_config_wfa_cfg_cw(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_edca_cfg_stru *edca_cfg_param = OSAL_NULL;

    /* 参数合法性判断 */
    if (msg->data == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_wfa_cfg_cw::input params is invalid,%p.}", (uintptr_t)msg->data);
        return OAL_FAIL;
    }
    unref_param(hmac_vap);

    edca_cfg_param = (mac_edca_cfg_stru *)msg->data;

    hal_vap_set_edca_machw_cw_wfa((osal_u8)edca_cfg_param->val, edca_cfg_param->ac, edca_cfg_param->switch_code);
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
static osal_void hmac_config_reg_info_reg_read(osal_u8 reg_type, osal_u32 start_addr, osal_u32 read_number)
{
    if (reg_type == 4) { /* 4:读32位寄存器 */
        osal_u32 idx, val;
        for (idx = 0; idx < read_number; idx++) {
            val = hal_reg_read((osal_u32)(start_addr + idx * reg_type));
            wifi_printf_always("\r\n0x%08x:0x%08x\r\n", start_addr + idx * reg_type, val);
        }
    } else { /* 4:读16位寄存器 */
        osal_u16 val;
        osal_u32 idx;
        for (idx = 0; idx < read_number; idx++) {
#if defined(_PRE_WLAN_FEATURE_CENTRALIZE) && defined(BOARD_FPGA_WIFI)
            osal_u32 read_addr = start_addr + idx * reg_type;
            if ((read_addr & 0xffff0000) == 0x40040000) { /* 是操作RF扣板的rf/pll寄存器 */
                val = fe_rf_read_16(read_addr);
            } else {
#endif
                val = hal_reg_read16((osal_u32)(start_addr + idx * reg_type));
#if defined(_PRE_WLAN_FEATURE_CENTRALIZE) && defined(BOARD_FPGA_WIFI)
            }
#endif
            wifi_printf_always("\r\n0x%08x:0x%04x\r\n", (start_addr + idx * reg_type), val);
        }
    }
}

/*****************************************************************************
 功能描述  : 读取mac寄存器
*****************************************************************************/
osal_s32 hmac_config_reg_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s8 *end = OSAL_NULL;
    osal_char *ctx = OSAL_NULL;
    const osal_char *sep = " ";
    osal_char *token = OSAL_NULL;
    osal_u8 reg_type;
    osal_u32 start_addr, read_number;

    unref_param(hmac_vap);
    /* 选择读取单位(32/16) */
    token = strtok_s((osal_char *)msg->data, sep, &ctx);
    if (token == OSAL_NULL) {
        return OAL_FAIL;
    }
    reg_type = (strcmp((const osal_char*)token, "16") == 0) ? 2 : 4;   /* 2:16位;4:32位寄存器 */
    if (strcmp((const osal_char*)token, "all") == 0) {
        return OAL_SUCC;
    }
    /* 获取起始地址 */
    token = strtok_s(OSAL_NULL, sep, &ctx);
    if (token == OSAL_NULL) {
        return OAL_FAIL;
    }
    start_addr = (osal_u32)oal_strtol((const osal_s8*)token, &end, 16); /* 1转化成16进制数 */
    /* 获取读取长度 */
    token = strtok_s(OSAL_NULL, sep, &ctx);
    if ((token == OSAL_NULL) || (start_addr % reg_type != 0)) {  /* 地址需是对应字节数的整数倍 */
        return OAL_FAIL;
    }
    read_number = (osal_u32)oal_strtol((const osal_s8*)token, &end, 10); /* 转化成10进制数 */
#ifdef _PRE_WLAN_REG_RW_WHITELIST
    if (reg_rw_check_addr(start_addr, read_number * reg_type) != OAL_SUCC) {
        wifi_printf("Permission denied!\r\n");
        return OAL_FAIL;
    }
#endif
    hmac_config_reg_info_reg_read(reg_type, start_addr, read_number);
    return OAL_SUCC;
}

osal_s32  hmac_config_reg_info_etc(hmac_vap_stru *hmac_vap, frw_msg *msg_info)
{
    osal_s32 ret;
    if (hmac_vap == OAL_PTR_NULL || msg_info == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_reg_info_etc::null pointer [%p] [%p].}",
            (uintptr_t)hmac_vap, (uintptr_t)msg_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***************************************************************************
        直调读取寄存器接口
    ***************************************************************************/
    ret = hmac_config_reg_info(hmac_vap, msg_info);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_reg_info_etc::send_cfg_to_device failed[%d].}",
                         hmac_vap->vap_id, ret);
    }

    return ret;
}
#endif

#if defined(_PRE_WLAN_FEATURE_ALWAYS_TX) || defined(_PRE_WLAN_CFGID_DEBUG)
static osal_s32 hmac_config_reg_write_ext(osal_u8 vap_id, oal_bool_enum_uint8 reg_info32, osal_u32 addr, osal_u32 val)
{
    if (reg_info32 == OSAL_TRUE) {
        /* 寄存器地址是需4的整数倍 */
        if (addr % 4 != 0) {
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_config_reg_write::addr=%x invalid", vap_id, addr);
            return OAL_FAIL;
        }
        hal_reg_write(addr, val);
    } else {
        if (addr % 2 != 0) { /* 该地址应该是2的倍数 */
            oam_warning_log2(0, 0, "vap_id[%d] {hmac_config_reg_write::addr=%d invalid", vap_id, addr);
            return OAL_FAIL;
        }
#if defined(_PRE_WLAN_FEATURE_CENTRALIZE) && defined(BOARD_FPGA_WIFI)
        if ((addr & 0xffff0000) == 0x40040000) { /* 是操作RF扣板的rf/pll寄存器 */
            fe_rf_write_16(addr, (osal_u16)val);
        } else {
#endif
            hal_reg_write16(addr, (osal_u16)val);
#if defined(_PRE_WLAN_FEATURE_CENTRALIZE) && defined(BOARD_FPGA_WIFI)
        }
#endif
    }
    oam_warning_log3(0, 0, "vap_id[%d] {hmac_config_reg_write::addr=0x%08x, val=0x%x", vap_id, addr, val);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 写寄存器接口
*****************************************************************************/
osal_s32 hmac_config_reg_write(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_char *token = OSAL_NULL;
    osal_s8 *end = OSAL_NULL;
    osal_char *ctx = OSAL_NULL;
    const osal_char *sep = " ";
    oal_bool_enum_uint8 reg_info32 = OSAL_TRUE;
    osal_u32 addr, val;

    /* 选择写操作单位(32/16) */
    token = strtok_s((osal_char *)msg->data, sep, &ctx);
    if (token == OSAL_NULL) {
        return OAL_FAIL;
    }
    if (strcmp((const osal_char*)token, "16") == 0) {
        reg_info32 = OSAL_FALSE;
    }

    /* 获取寄存器地址 */
    token = strtok_s(OSAL_NULL, sep, &ctx);
    if (token == OSAL_NULL) {
        return OAL_FAIL;
    }
    addr = (osal_u32)oal_strtol((const osal_s8*)token, &end, 16); /* 16表示将字符串转化成16进制数 */

    /* 获取需要写入的值 */
    token = strtok_s(OSAL_NULL, sep, &ctx);
    if (token == OSAL_NULL) {
        return OAL_FAIL;
    }
    val = (osal_u32)oal_strtol((const osal_s8*)token, &end, 16); /* 16表示将字符串转化成16进制数 */

#ifdef _PRE_WLAN_REG_RW_WHITELIST
    if (reg_rw_check_addr(addr, reg_info32 == OSAL_TRUE ? 4 : 2) != OAL_SUCC) { /* 2:16位; 4:32位寄存器 */
        wifi_printf("Permission denied!\r\n");
        return OAL_FAIL;
    }
#endif
    return hmac_config_reg_write_ext(hmac_vap->vap_id, reg_info32, addr, val);
}
#endif

/*****************************************************************************
 函 数 名  : hmac_config_suspend_action_sync
 功能描述  : 亮暗屏状态
*****************************************************************************/
osal_s32 hmac_config_suspend_action_sync(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_suspend_stru *suspend = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    unref_param(hmac_vap);
    suspend = (mac_cfg_suspend_stru *)msg->data;
    hmac_device->in_suspend = suspend->in_suspend; // 亮暗屏状态

    /* 暗屏实施dtim2策略 */
    hmac_psm_max_ps_en_sync(hmac_vap, hmac_device);
    hmac_device->arpoffload_switch = suspend->arpoffload_switch; // arp 开关
    return OAL_SUCC;
}

osal_s32 hmac_cali_to_hmac(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    if (msg == OSAL_NULL || msg->data == OSAL_NULL || msg->data_len == 0) {
        wifi_printf("hmac_cali_to_hmac::msg is none, len=%u \n", msg->data_len);
        return OAL_FAIL;
    }
#ifndef CONFIG_NO_SUPPORT_INI
    return hmac_save_cali_event_etc(msg->data, msg->data_len);
#else
    return OAL_SUCC;
#endif
}

osal_s32 hmac_config_get_tsf(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 ret;
    osal_u32 high = 0;
    osal_u32 low = 0;
    osal_u64 tsf;
    unref_param(msg);

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_get_tsf::NULL ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_vap_tsf_get_64bit(hmac_vap->hal_vap, &high, &low);
    tsf = ((osal_u64)(high) << 32) + (osal_u64)low;    /* 高字节左移32位 */
    ret = hmac_fill_sys_rsp(msg, sizeof(osal_u64), (osal_u8 *)&tsf);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "vap_id[%u] {hmac_config_get_tsf::hmac_fill_sys_rsp failed[%u].}", hmac_vap->vap_id, ret);
        return (osal_s32)ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :校准维测
*****************************************************************************/
osal_s32 hmac_config_cali_dump_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    return OAL_SUCC;
}

osal_s32 hmac_config_custom_cfg_priv_ini(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_device_capability_stru *device_cap = OSAL_NULL;
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hcc_custom_data *custom_data = (hcc_custom_data *)msg->data;
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_custom_cfg_priv_ini:: hmac_vap is null!!!}");
    }

    device_cap = mac_device_get_capability();

    switch (custom_data->syn_id) {
        case CUSTOM_CFGID_PRIV_INI_BW_MAX_WITH_ID:
            device_cap->channel_width = custom_data->value;
            hmac_device->device_cap.channel_width = custom_data->value;
            break;

        case CUSTOM_CFGID_PRIV_INI_SU_BFER_ID:
            device_cap->su_bfmer_is_supp = custom_data->value;
            hmac_device->device_cap.su_bfmer_is_supp = custom_data->value;
            break;

        case CUSTOM_CFGID_PRIV_INI_SU_BFEE_ID:
            device_cap->su_bfmee_is_supp = custom_data->value;
            hmac_device->device_cap.su_bfmee_is_supp = custom_data->value;
            break;

        case CUSTOM_CFGID_PRIV_INI_MU_BFER_ID:
            device_cap->mu_bfmer_is_supp = custom_data->value;
            hmac_device->device_cap.mu_bfmer_is_supp = custom_data->value;
            break;

        case CUSTOM_CFGID_PRIV_INI_MU_BFEE_ID:
            device_cap->mu_bfmee_is_supp = custom_data->value;
            hmac_device->device_cap.mu_bfmee_is_supp = custom_data->value;
            break;
        case CUSTOM_CFGID_PRIV_INI_LDPC_ID:
            device_cap->ldpc_is_supp = custom_data->value;
            hmac_device->device_cap.ldpc_is_supp = custom_data->value;
            break;
        case CUSTOM_CFGID_PRIV_INI_BA_32BITMAP_ID:
            device_cap->ba_bitmap_support_32bit = custom_data->value;
            hmac_device->device_cap.ba_bitmap_support_32bit = custom_data->value;
            break;
        case CUSTOM_CFGID_PRIV_INI_MTID_AGGR_RX_ID:
            device_cap->mtid_aggregation_rx_support = custom_data->value;
            hmac_device->device_cap.mtid_aggregation_rx_support = custom_data->value;
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

osal_s32 hmac_config_tx_complete_data(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    dmac_tx_status_stru *tx_status_param = (dmac_tx_status_stru *)msg->data;
    hmac_user_stru *hmac_user = OSAL_NULL;
    osal_u32 ret;

    hmac_user = mac_res_get_hmac_user_etc(tx_status_param->user_idx);
    if (hmac_user == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_tx_complete_data:: user[%d] is null!!!}",
            tx_status_param->user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (tx_status_param->dscr_status != DMAC_TX_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_tx_complete_data:: tx failed[%d]!!!}",
            tx_status_param->dscr_status);
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    /* 反馈常发报文发送状态 */
    hmac_config_set_quick_result((osal_u8)tx_status_param->dscr_status);
#endif
    if (tx_status_param->mgmt_frame_id == WLAN_TX_EAPOL_KEY_PTK_4_4) {
        oam_info_log3(0, OAM_SF_ANY, "{hmac_config_tx_complete_data:: PAE, userid=%d, key_open=%d, need_install=%d}",
            tx_status_param->user_idx, hmac_user->is_rx_eapol_key_open, hmac_user->ptk_need_install);
        if (hmac_user->is_rx_eapol_key_open == OSAL_TRUE) {
            oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_tx_complete_data:: rx_eapol_key is open}");
            return OAL_SUCC;
        }
        /* 如果用户需要加密EAPOL-KEY, 且4/4 EAPOL-KEY 发送成功，则设置秘钥 */
        hmac_user->eapol_key_4_4_tx_succ = OSAL_TRUE;
        if (hmac_user->ptk_need_install == OSAL_TRUE) {
            /* 更新单播秘钥 */
            ret = hmac_config_11i_add_key_set_reg(hmac_vap, OSAL_TRUE, hmac_user->ptk_key_idx, hmac_user);
            if (ret != OAL_SUCC) {
                oam_error_log2(0, OAM_SF_TX, "hmac_config_tx_complete_data::set ptk fail %d, key_idx %d.", ret,
                    hmac_user->ptk_key_idx);
                oam_error_log4(0, OAM_SF_TX, "hmac_config_tx_complete_data::%02X:%02X:%02X:%02X:XX:XX",
                    hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], /* 打印第 0, 1 位mac */
                    hmac_user->user_mac_addr[2], hmac_user->user_mac_addr[3]); /* 打印第 2, 3 位mac */
            } else {
                oam_warning_log1(0, OAM_SF_TX, "hmac_config_tx_complete_data::set ptk succ, key_idx %d.",
                    hmac_user->ptk_key_idx);
                oam_warning_log4(0, OAM_SF_TX, "hmac_config_tx_complete_data::%02X:%02X:%02X:%02X:XX:XX",
                    hmac_user->user_mac_addr[0], hmac_user->user_mac_addr[1], /* 打印第 0, 1 位mac */
                    hmac_user->user_mac_addr[1], hmac_user->user_mac_addr[2]); /* 打印第 2, 3 位mac */
            }
            hmac_user->ptk_need_install = OSAL_FALSE;
            hmac_user->eapol_key_4_4_tx_succ = OSAL_FALSE;
            hmac_user->ptk_key_idx = 0;
        }
    }

    return OAL_SUCC;
}

osal_s32 hmac_config_tx_comp_schedule(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;

    tx_comp_schedule *tx_comp_schedule_msg = (tx_comp_schedule *)msg->data;
    if (hmac_vap == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_config_tx_comp_schedule:: hmac_vap is null!!!}");
    }

    ret = (osal_s32)hmac_tx_schedule(hal_chip_get_hal_device(), tx_comp_schedule_msg->ac_num);
    if (ret != OAL_SUCC) {
        oam_info_log1(0, OAM_SF_ANY, "{hmac_config_tx_comp_schedule:: hmac_tx_schedule failed[%d]}", ret);
    }

    return ret;
}

osal_s32 hmac_config_set_beacon_period(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u32 beacon_period = *((osal_u32 *)msg->data);

    mac_mib_set_beacon_period(hmac_vap, beacon_period);
    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_hal_device_sync
 功能描述  : 同步hal_device信息
*****************************************************************************/
osal_s32 hmac_hal_device_sync(hal_to_dmac_device_stru *hal_device)
{
    frw_msg msg_info = {0};
    mac_update_hal_device_stru hal_device_info_sync_stru = {0};
    osal_s32 ret;

    /* 同步hal_device */
    hal_device_info_sync_stru.promis_switch = hal_device->promis_switch;
    hal_device_info_sync_stru.ampdu_tx_hw = hal_device->ampdu_tx_hw;
    hal_device_info_sync_stru.al_tx_flag = hal_device->al_tx_flag;
    hal_device_info_sync_stru.al_rx_flag = hal_device->al_rx_flag;
    hal_device_info_sync_stru.al_tx_pkt_len = hal_device->al_tx_pkt_len;
    hal_device_info_sync_stru.current_chan_number = hal_device->current_chan_number;
    hal_device_info_sync_stru.ax_vap_cnt = hal_device->ax_vap_cnt;
    (osal_void)memcpy_s(&hal_device_info_sync_stru.wifi_channel_status, sizeof(mac_channel_stru),
                        &hal_device->wifi_channel_status, sizeof(mac_channel_stru));

    /* 抛事件至Device侧DMAC，同步dmac_user_stru */
    frw_msg_init((osal_u8 *)&hal_device_info_sync_stru, sizeof(mac_update_hal_device_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_SYNC_HAL_DEVICE, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_hal_device_sync::sync msg failed[%d] return addr[%u].}", ret,
            (osal_u32)(uintptr_t)__builtin_return_address(0));
        wifi_printf("{hmac_hal_device_sync::sync msg failed[%d] return addr[%u].}\r\n", ret,
            (osal_u32)(uintptr_t)__builtin_return_address(0));
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_device_sync
 功能描述  : 同步hal_device信息
*****************************************************************************/
osal_s32 hmac_device_sync(hmac_device_stru *hmac_device)
{
    frw_msg msg_info = {0};
    mac_update_device_stru device_info_sync = {0};
    osal_s32 ret;

    /* 同步hal_device */
    device_info_sync.up_vap_num = hmac_device->up_vap_num;
    device_info_sync.mac_vap_id = hmac_device->mac_vap_id;
    device_info_sync.curr_scan_state = hmac_device->curr_scan_state;
    device_info_sync.p2p_ps_pause = hmac_device->st_p2p_info.p2p_ps_pause;

    /* 抛事件至Device侧DMAC，同步dmac_user_stru */
    frw_msg_init((osal_u8 *)&device_info_sync, sizeof(mac_update_device_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_SYNC_DEV, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG, "{hmac_device_sync::sync msg failed[%d] return address[%u].}", ret,
            (osal_u32)(uintptr_t)__builtin_return_address(0));
        wifi_printf("{hmac_device_sync::sync msg failed[%d] return address[%u].}\r\n", ret,
            (osal_u32)(uintptr_t)__builtin_return_address(0));
    }

    return ret;
}

#ifdef _PRE_WIFI_DEBUG
static osal_u8 g_tx_data_type_debug_on = OSAL_FALSE;
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u8 hal_tx_data_type_debug_on(osal_void)
{
    return g_tx_data_type_debug_on;
}

osal_s32 hmac_config_set_tx_data_type_debug(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 debug_on = *(osal_u8 *)(msg->data);
    unref_param(hmac_vap);

    wifi_printf("Switch tx data type debug %s", (debug_on > 0) ? "ON" : "OFF");

    g_tx_data_type_debug_on = (debug_on > 0) ? OSAL_TRUE : OSAL_FALSE;

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 函 数 名  : hmac_config_rssi_limit
 功能描述  : 配置 rssi_limit时的参数
*****************************************************************************/
osal_s32  hmac_config_rssi_limit(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_cfg_rssi_limit_stru *rssi_limit = OSAL_NULL;

    rssi_limit = (mac_cfg_rssi_limit_stru *)msg->data;
    switch (rssi_limit->rssi_limit_type) {
        case MAC_RSSI_LIMIT_SHOW_INFO:
            oam_warning_log4(0, OAM_SF_ANY,
                "vap_id[%d] {hmac_config_rssi_limit::[show_info] limit_flag[%d],rssi_limit[%d],limit_delta[%d].}",
                hmac_vap->vap_id, hmac_device->rssi_limit.rssi_limit_enable_flag,
                (osal_s32)hmac_device->rssi_limit.rssi, (osal_s32)hmac_device->rssi_limit.rssi_delta);
            break;
        case MAC_RSSI_LIMIT_ENABLE:
            hmac_device->rssi_limit.rssi_limit_enable_flag = rssi_limit->rssi_limit_enable_flag;
            break;
        case MAC_RSSI_LIMIT_DELTA:
            hmac_device->rssi_limit.rssi_delta = rssi_limit->rssi_delta;
            break;
        case MAC_RSSI_LIMIT_THRESHOLD:
            hmac_device->rssi_limit.rssi = rssi_limit->rssi;
            break;
        default:
            break;
    }

    return OAL_SUCC;
}
osal_s32 hmac_config_set_log_level(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    if (osal_unlikely((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL))) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_config_set_log_level:: hmac_vap:%p or param:%p is null!}",
            (uintptr_t)hmac_vap, (uintptr_t)(msg->data));
        return OAL_ERR_CODE_PTR_NULL;
    }

    dft_set_log_level(msg->data[0]);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_beacon_chain_switch
 功能描述  : dmac 配置关闭/开启beacon帧双路轮流发送策略
*****************************************************************************/
osal_s32  hmac_config_beacon_chain_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 value;
    if (hmac_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_beacon_chain_switch::hmac_vap null.}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    value = *((osal_u8 *)msg->data);
    /* 配置beacon帧发送通道，0为普通单通道发送，1为开启双通道轮流发送能力,2为开启双通道发送能力 */
    hmac_vap->beacon_tx_policy = value & 0x3;
    return OAL_SUCC;
}
#endif

osal_s32 hmac_config_set_bandwidth(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_mode_param_stru *prot_param = OAL_PTR_NULL;
    prot_param = (mac_cfg_mode_param_stru *)msg->data;
    prot_param->protocol = hmac_vap->protocol;
    prot_param->band = hmac_vap->channel.band;
    return (osal_s32)hmac_config_set_mode_etc(hmac_vap, msg->data_len, (osal_u8 *)prot_param);
}

#ifdef _PRE_WLAN_DFT_STAT
osal_s32 hmac_config_dump_phy(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    hal_dump_phy_reg();
    return OAL_SUCC;
}

osal_s32 hmac_config_dump_mac(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    hal_dump_mac_reg();
    return OAL_SUCC;
}

osal_s32 hmac_config_dump_stats(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    hal_dump_stats();
    return OAL_SUCC;
}
#endif

#ifndef CONFIG_HAS_EARLYSUSPEND
/*****************************************************************************
 函 数 名  : hmac_config_set_suspend_mode
 功能描述  : 设置亮暗屏状态
 输入参数  : hmac_vap_stru *hmac_vap
             osal_u16  len
             osal_u8 *param
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2017年11月23日
    修改内容   : 新生成函数

*****************************************************************************/
osal_s32 hmac_config_set_suspend_mode(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (!hmac_device) {
        oam_error_log1(0, OAM_SF_CFG, "vap_id[%d] hmac_config_set_suspend_mode:hmac_device is null ptr!",
            hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 0:亮屏 1:暗屏 */
    if (*(msg->data) == 0) {
        hmac_do_suspend_action_etc(hmac_device, OAL_FALSE);
    } else {
        hmac_do_suspend_action_etc(hmac_device, OAL_TRUE);
    }

    return OAL_SUCC;
}
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
OAL_STATIC osal_u8 g_mgmt_report_enable = OSAL_FALSE;
osal_u8 hmac_is_mgmt_report_enable(osal_void)
{
    return g_mgmt_report_enable;
}

OAL_STATIC osal_s32 hmac_config_mgmt_report_switch(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    oam_warning_log1(0, OAM_SF_CFG, "{hmac_set_mgmt_report_switch:switch=%d}", msg->data[0]);
    g_mgmt_report_enable = msg->data[0];
    return OAL_SUCC;
}
#endif

OAL_STATIC osal_void hmac_config_sta_rcv_probe_req_on(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (g_sta_rcv_probe_req_enable == OSAL_TRUE) {
        wifi_printf("hmac_config_sta_rcv_probe_req_on::no need to switch on again\r\n");
        return;
    }

    /* 如果非单STA,则无需后续处理 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA || hmac_device_calc_up_vap_num_etc(hmac_device) > 1) {
        return;
    }

    /* 单STA场景，IDLE状态，发送报文需要打开PA */
    if (hmac_vap->hal_device->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_IDLE_STATE) {
        hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_JOIN_COMP, sizeof(hal_to_dmac_vap_stru),
                                (osal_u8 *)(hmac_vap->hal_vap));
        hal_enable_machw_phy_and_pa(hmac_vap->hal_device);
    }

    g_sta_rcv_probe_req_enable = OSAL_TRUE;

    wifi_printf("sta_rcv_probe_req turned on.\r\n");
}

OAL_STATIC osal_bool hmac_config_sta_rcv_probe_req_off(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);

    if (g_sta_rcv_probe_req_enable != OSAL_TRUE) {
        wifi_printf("hmac_config_sta_rcv_probe_req_off::no need to switch off\r\n");
        return OAL_TRUE;
    }

    /* 如果非单STA,则直接返回 */
    if (hmac_vap->vap_mode != WLAN_VAP_MODE_BSS_STA || hmac_device_calc_up_vap_num_etc(hmac_device) > 1) {
        return OAL_TRUE;
    }

    /* STA down事件 恢复未关联时对任意帧的配置 */
    if ((hmac_vap->vap_state != MAC_VAP_STATE_UP) && (hmac_vap->vap_state != MAC_VAP_STATE_PAUSE) &&
        (hmac_device->curr_scan_state != MAC_SCAN_STATE_RUNNING && hmac_device_calc_up_vap_num_etc(hmac_device) == 0)) {
        hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_VAP_DOWN, sizeof(hal_to_dmac_vap_stru),
            (osal_u8 *)(hmac_vap->hal_vap));
        hal_disable_machw_phy_and_pa();
    }

    g_sta_rcv_probe_req_enable = OSAL_FALSE;

    wifi_printf("sta_rcv_probe_req turned off.\r\n");
    return OAL_TRUE;
}

osal_u8 hmac_config_sta_rcv_probe_req_is_on(osal_void)
{
    return g_sta_rcv_probe_req_enable;
}

OSAL_STATIC osal_s32 hmac_config_sta_rcv_probe_req_switch(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8  ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 ret;
    osal_u8  sta_rcv_probe_req_switch = OSAL_FALSE;

    if ((hmac_vap == OSAL_NULL) || (param == OSAL_NULL)) {
        return OAL_FAIL;
    }

    ret = hmac_ccpriv_get_one_arg(&param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_config_sta_rcv_probe_req_switch::get_cmd_one_arg err[%d]!}", ret);
        return ret;
    }

    sta_rcv_probe_req_switch = (osal_u8)oal_atoi((const osal_s8 *)ac_name);
    if (sta_rcv_probe_req_switch == OSAL_TRUE) {
        hmac_config_sta_rcv_probe_req_on(hmac_vap);
    } else {
        hmac_config_sta_rcv_probe_req_off(hmac_vap);
    }
    return OAL_SUCC;
}

OSAL_STATIC osal_s32 hmac_config_sta_get_conn_status_code(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    if (hmac_vap == OSAL_NULL) {
        return OAL_FAIL;
    }
    unref_param(param);
    hmac_sta_print_last_reason_code();
    return OAL_SUCC;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
