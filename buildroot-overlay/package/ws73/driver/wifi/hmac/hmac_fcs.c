/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: 调度算法
 * Date: 2020-07-14
 */

/*****************************************************************************
  1 头文件包含
 *****************************************************************************/
#include "hmac_fcs.h"
#include "wlan_spec.h"
#include "mac_device_ext.h"
#include "hmac_main.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_regdomain.h"
#include "hmac_tx_mgmt.h"
#include "hmac_sta_pm.h"
#include "hmac_btcoex.h"
#include "hmac_chan_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "hmac_m2s.h"
#endif

#include "frw_osal.h"
#include "wlan_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_FCS_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/* 获取该vap保护帧发送模式 */
osal_u32 hmac_fcs_get_prot_mode(const hmac_vap_stru *hmac_vap, const  hal_one_packet_cfg_stru *one_packet_cfg)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    osal_u32                tx_mode = 0;
    osal_u8                 subband_idx = 0;
    osal_u8                 pow_level_idx = 0;

    if (one_packet_cfg == OSAL_NULL) {
        oam_warning_log1(0, 0, "vap_id[%d] {hmac_fcs_get_prot_mode::one_packet_cfg is NULL!}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_fcs_get_prot_mode: hal_device is NULL.");
        return tx_mode;
    }

    /* 获取控制帧功率等级 */
    pow_level_idx = hal_device->control_frm_power_level;

    /* 获取当前速率 */
    hal_device->phy_pow_param.rate_one_pkt = (one_packet_cfg->tx_data_rate >> 16) & 0xFF; /* 右移16位 */

    /* 刷新one packet帧当前功率 */
    if (hmac_vap->channel.band == WLAN_BAND_5G) {
        subband_idx += WLAN_2G_SUB_BAND_NUM;
    }
    hal_device->phy_pow_param.one_pkt_pow_code[subband_idx] =
        hal_pow_get_pow_table_tpc_code(hmac_vap->channel.band, hal_device->phy_pow_param.rate_one_pkt,
        WLAN_BAND_ASSEMBLE_20M, pow_level_idx);

    tx_mode = hal_device->phy_pow_param.one_pkt_pow_code[subband_idx];

    return tx_mode;
}

/* 配置onepacket帧速率 */
osal_u32 hmac_fcs_set_prot_datarate(const hmac_vap_stru *src_vap)
{
    const hal_to_dmac_device_stru    *hal_device = OSAL_NULL;
    const hmac_vap_stru              *hmac_vap = OSAL_NULL;

    hal_device = ((hmac_vap_stru *)src_vap)->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_fcs_set_prot_datarate::hal_device null.}", src_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 强场环境下按照24M发，弱场环境按照6M/1M发 */
    hmac_vap = (hmac_vap_stru *)src_vap;
    if (oal_get_real_rssi(hmac_vap->query_stats.signal_mgmt_data) > WLAN_FAR_DISTANCE_RSSI) {
        /* 若配置"通道1", 则保护帧只用通道1发送 */
        if (hal_device->cfg_cap_info->single_tx_chain == WLAN_TX_CHAIN_ONE) {
            return WLAN_PROT_DATARATE_CHN1_24M;
        } else {
            /* 若配置"通道0"或"双通道", 则保护帧只用通道0发送 */
            return WLAN_PROT_DATARATE_CHN0_24M;
        }
    }

    if ((!is_legacy_vap(src_vap)) || (src_vap->channel.band == WLAN_BAND_5G)) {
        if (hal_device->cfg_cap_info->single_tx_chain == WLAN_TX_CHAIN_ONE) {
            return WLAN_PROT_DATARATE_CHN1_6M;
        } else {
            return WLAN_PROT_DATARATE_CHN0_6M;
        }
    } else {
        if (hal_device->cfg_cap_info->single_tx_chain == WLAN_TX_CHAIN_ONE) {
            return WLAN_PROT_DATARATE_CHN1_1M;
        } else {
            return WLAN_PROT_DATARATE_CHN0_1M;
        }
    }
}

/* 准备one packet参数 */
osal_void hmac_fcs_prepare_one_packet_cfg(const hmac_vap_stru *hmac_vap, hal_one_packet_cfg_stru *one_packet_cfg,
    osal_u16 protect_time)
{
    osal_u32       duration;
    hmac_user_stru  *hmac_user = OSAL_NULL;
    duration = ((osal_u32)protect_time) << 10; /* 左移10位 */
    if (hmac_vap == OSAL_NULL || (hmac_vap->hal_vap == OSAL_NULL) || one_packet_cfg == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_fcs_prepare_one_packet_cfg:: null pointer.}");
        return;
    }

    one_packet_cfg->protect_type    = mac_fcs_get_protect_type(hmac_vap);
    one_packet_cfg->protect_cnt     = mac_fcs_get_protect_cnt(hmac_vap);
    one_packet_cfg->tx_data_rate    = hmac_fcs_set_prot_datarate(hmac_vap);
    one_packet_cfg->duration        = (osal_u16)osal_min(duration, MAC_FCS_CTS_MAX_DURATION);
    one_packet_cfg->timeout         = MAC_FCS_DEFAULT_PROTECT_TIME_OUT;
    one_packet_cfg->wait_timeout    = MAC_ONE_PACKET_TIME_OUT_DEFAULT;
    one_packet_cfg->tx_mode         = hmac_fcs_get_prot_mode(hmac_vap, one_packet_cfg);

    if (one_packet_cfg->protect_type == HAL_FCS_PROTECT_TYPE_NULL_DATA) {
        /* 背景扫描发送的null data BSSID填错，改为0x1100表示ToAP */
        mac_null_data_encap(one_packet_cfg->protect_frame,
                            (WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_NODATA | 0x1100),
                            hmac_vap->bssid,
                            mac_mib_get_station_id(hmac_vap));
    }

    one_packet_cfg->cfg_one_pkt_tx_vap_index = hmac_vap->hal_vap->vap_id;

    hmac_user = (hmac_user_stru *)mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (hmac_user != OSAL_NULL) {
        one_packet_cfg->cfg_one_pkt_tx_peer_index = hmac_user->lut_index;
    }
}

/* fcs发消息至device执行 */
static osal_void hmac_fcs_start_device(mac_fcs_cfg_stru *fcs_cfg, mac_fcs_type_enum_uint8 fcs_type)
{
    osal_s32 ret;
    frw_msg msg_info = {0};

    if (fcs_cfg == OSAL_NULL) {
        return;
    }

    fcs_cfg->fcs_type = fcs_type;
    fcs_cfg->src_fake_queue = OSAL_NULL; /* 需要显式指空，使用device侧fake队列 */
    frw_msg_init((osal_u8 *)fcs_cfg, sizeof(mac_fcs_cfg_stru), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(fcs_cfg->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_FCS_START, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_ANY, "{hmac_fcs_send_msg_to_device:fcs_type[%d] send_to_device fail[%d]}", fcs_type,
            ret);
    }
}

/* 更新hal_device下wifi_channel_status状态 */
static osal_void hmac_fcs_updata_channel_status(mac_fcs_cfg_stru *fcs_cfg)
{
    hal_to_dmac_device_stru *hal_device;

    if (fcs_cfg == OSAL_NULL) {
        return;
    }

    hal_device = fcs_cfg->hal_device;
    if (hal_device == OSAL_NULL) {
        return;
    }

    hal_device->wifi_channel_status.band = fcs_cfg->dst_chl.band;
    hal_device->wifi_channel_status.chan_idx = fcs_cfg->dst_chl.chan_idx;
    hal_device->wifi_channel_status.chan_number = fcs_cfg->dst_chl.chan_number;
    hal_device->wifi_channel_status.en_bandwidth = fcs_cfg->dst_chl.en_bandwidth;

    /* 切换完成 */
    hal_device->current_chan_number = fcs_cfg->dst_chl.chan_number;

#ifdef _PRE_WLAN_FEATURE_M2S
    hmac_m2s_spec_update_hal_device_capbility(hal_device, fcs_cfg->dst_chl.band);
#endif
}

/* 启动one packet，将发送队列保存到虚假队列，切信道 */
osal_void hmac_fcs_start(mac_fcs_cfg_stru *fcs_cfg)
{
    hmac_fcs_start_device(fcs_cfg, MAC_FCS_START_SWITCH_CHNL);
    hmac_fcs_updata_channel_status(fcs_cfg);
}

/* 增强版FCS接口，启动两次onepacket 用于双STA背景扫描 */
osal_void hmac_fcs_start_enhanced(mac_fcs_cfg_stru *fcs_cfg)
{
    hmac_fcs_start_device(fcs_cfg, MAC_FCS_START_SWITCH_CHNL_ENHANCED);
    hmac_fcs_updata_channel_status(fcs_cfg);
}

/* 同信道启动one packet，暂停tx supend，刷新硬件队列发送描述符 */
osal_void hmac_fcs_start_same_channel(mac_fcs_cfg_stru *fcs_cfg)
{
    hmac_fcs_start_device(fcs_cfg, MAC_FCS_START_SAME_CHNL);
}

/* 增强版同信道FCS接口，启动两次onepacket 用于同信道多vap */
osal_void hmac_fcs_start_enhanced_same_channel(mac_fcs_cfg_stru *fcs_cfg)
{
    hmac_fcs_start_device(fcs_cfg, MAC_FCS_START_SAME_CHNL_ENHANCED);
}

/* 发送一次one packet 报文 */
osal_void hmac_fcs_start_send_one_packet(mac_fcs_cfg_stru *fcs_cfg)
{
    hmac_fcs_start_device(fcs_cfg, MAC_FCS_START_SEND_ONE_PACKET);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
