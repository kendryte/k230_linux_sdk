/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 常发常收
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
#include "hmac_al_tx_rx.h"

#include "oal_mem_hcm.h"
#include "hmac_config.h"
#include "oam_struct.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_device.h"
#include "mac_resource_ext.h"
#include "mac_vap_ext.h"
#include "hmac_resource.h"
#include "mac_ie.h"
#include "hmac_alg_config.h"
#include "mac_user_ext.h"
#include "mac_regdomain.h"
#include "hmac_ext_if.h"
#include "hmac_rx_data.h"
#include "hmac_mgmt_classifier.h"
#include "hmac_fsm.h"
#include "hmac_main.h"
#include "hmac_vap.h"
#include "hmac_tx_amsdu.h"
#include "hmac_chan_mgmt.h"
#include "hmac_rx_filter.h"
#include "hmac_psm_ap.h"
#include "hmac_protection.h"
#include "hmac_feature_dft.h"
#include "hmac_ampdu_config.h"
#include "hmac_keep_alive.h"
#include "hmac_mgmt_bss_comm.h"
#include "hmac_data_acq.h"
#include "hmac_rx_filter.h"
#include "hmac_mgmt_sta.h"
#include "hmac_sme_sta.h"
#include "hmac_scan.h"
#include "hmac_reset.h"
#include "hmac_blockack.h"
#include "hmac_mgmt_ap.h"
#include "plat_pm_wlan.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "fe_rf_customize_power_cali.h"

#include "hmac_feature_main.h"
#include "mac_device_ext.h"

#include "hmac_feature_dft.h"

#include "hmac_tx_mpdu_adapt.h"
#include "hmac_alg_notify.h"
#include "hal_mac.h"
#include "hmac_beacon.h"
#include "hmac_cali_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
#include "alg_anti_interference.h"
#endif
#include "hal_device.h"
#if defined(_PRE_WLAN_FEATURE_CENTRALIZE) && defined(BOARD_FPGA_WIFI)
#include "fe_hal_rf_if.h"
#endif
#include "cali_rf_temp_code.h"
#include "hal_mfg.h"
#include "hal_rf.h"
#ifdef _PRE_WLAN_ONLINE_CALI
#include "cali_online_tx_pwr.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_AL_TX_RX_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define SEND_TIMES_PERIOD 30
#define AL_TX_THREAD_PRIORITY 5 // 优先级与frw一致 避免切换任务太慢耗时太长
#define AL_TX_THREAD_STACK_SIZE 0x1000

#define AL_TX_INVALID_TPC_CODE  0xFF
static osal_u8 g_al_tx_tpc_code = AL_TX_INVALID_TPC_CODE;
osal_u32 g_payload_len = 0;
mac_cfg_tx_comp_stru g_al_tx_cfg_param = {0};
osal_u8 g_al_tx_frame_type_switch = 0;
osal_u8 g_al_tx_protocol = 0;
osal_u32 g_al_tx_mcs = 0;

OSAL_STATIC osal_void hmac_config_set_rx_framefilt(osal_u8 al_rx_flag)
{
    hal_mac_interrupt_clear();
    hal_rx_filter_set_alrx_state((hal_device_always_rx_state_enum)al_rx_flag);
}

osal_void hmac_set_al_tx_frame_type_switch(osal_u8 val)
{
    g_al_tx_frame_type_switch = val;
}

osal_u8 hmac_get_al_tx_frame_type_switch(osal_void)
{
    return g_al_tx_frame_type_switch;
}

osal_void hmac_set_al_tx_protocol(osal_u8 val)
{
    g_al_tx_protocol = val;
}

osal_u8 hmac_get_al_tx_protocol(osal_void)
{
    return g_al_tx_protocol;
}

osal_void hmac_set_al_tx_mcs(osal_u32 val)
{
    g_al_tx_mcs = val;
}

osal_u32 hmac_get_al_tx_mcs(osal_void)
{
    return g_al_tx_mcs;
}

/*****************************************************************************
 功能描述  : 常发设置地址
*****************************************************************************/
static osal_void hmac_config_al_tx_set_addresses(const hmac_vap_stru *hmac_vap, mac_ieee80211_frame_stru *hdr)
{
    /* From DS标识位设置 */
    mac_hdr_set_from_ds((osal_u8 *)hdr, 0);

    /* to DS标识位设置 */
    mac_hdr_set_to_ds((osal_u8 *)hdr, 1);

    /* Set Address1 field in the WLAN Header with BSSID */
    (osal_void)memcpy_s((osal_void *)(hdr->address1), WLAN_MAC_ADDR_LEN,
        (osal_void *)(BROADCAST_MACADDR), WLAN_MAC_ADDR_LEN);

    /* Set Address2 field in the WLAN Header with the source address */
    (osal_void)memcpy_s((osal_void *)(hdr->address2), WLAN_MAC_ADDR_LEN,
        (osal_void *)(mac_mib_get_station_id(hmac_vap)), WLAN_MAC_ADDR_LEN);

    /* Set Address3 field in the WLAN Header with the destination address */
    (osal_void)memcpy_s((osal_void *)(hdr->address3), WLAN_MAC_ADDR_LEN,
        (osal_void *)(BROADCAST_MACADDR), WLAN_MAC_ADDR_LEN);
}

// 常收开关算法接口
static osal_void hmac_config_set_always_rx_alg_off(hmac_vap_stru *hmac_vap, osal_u8 al_rx_flag)
{
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    if (al_rx_flag == HAL_ALWAYS_RX_RESERVED) {
        /* Normal receipt, close weak interference immune algorithm and weak intf */
        hmac_alg_anti_intf_switch(hmac_vap->hal_device, OSAL_FALSE);
        hal_set_weak_intf_rssi_th(ALG_ANTI_INF_RSSI_TH_MIN_VAL);
    } else {
        /* 1: very receiving mode, and the weak interference immune algorithm is turned on */
        hmac_alg_anti_intf_switch(hmac_vap->hal_device, OSAL_TRUE);
        hal_set_weak_intf_rssi_th(ALG_ANTI_INF_RSSI_TH_DEFAULT_VAL);
    }
#endif
}

static osal_void hmac_config_always_send_msg(osal_void)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
#ifdef _PRE_WLAN_FEATURE_DAQ
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (hwifi_get_data_sample() == OSAL_TRUE) {
        oam_warning_log0(0, 0, "{hmac_config_always_send_msg:fail for data sample using}");
        return;
    }
#endif
#endif
    frw_msg_init(OSAL_NULL, 0, OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_REINIT_MEM, &msg_info, OSAL_TRUE);
    oam_warning_log1(0, 0, "{hmac_config_always_send_msg:result[%d]}", ret);
}

/*****************************************************************************
 功能描述  : 常发设置frame ctrl字段
*****************************************************************************/
static osal_void hmac_config_al_tx_set_frame_ctrl(mac_tx_ctl_stru *tx_ctl, mac_ieee80211_frame_stru *hdr)
{
    /* 设置帧控制字段 */
    mac_hdr_set_frame_control((osal_u8 *)hdr, WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_DATA);

    tx_ctl->frame_header_length = MAC_80211_FRAME_LEN;
}

/*****************************************************************************
 功能描述  : 常发设置cb字段
*****************************************************************************/
static osal_u32 hmac_config_al_tx_packet(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf, osal_u32 len)
{
    mac_tx_ctl_stru                     *tx_ctl;       /* SKB CB */
    mac_ieee80211_qos_htc_frame_stru    *hdr = OSAL_NULL;

    /* 初始化CB tx rx字段 , CB字段在前面已经被清零， 在这里不需要重复对某些字段赋零值 */
    /* netbuff需要申请内存  */
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(buf);
    (osal_void)memset_s(tx_ctl, sizeof(mac_tx_ctl_stru), 0, sizeof(mac_tx_ctl_stru));
    tx_ctl->mpdu_num               = 1;
    tx_ctl->msdu_num               = 1;
    tx_ctl->netbuf_num             = 1;
    ((mac_ieee80211_frame_stru *)(oal_netbuf_header(buf)))->frame_control.type = WLAN_DATA_BASICTYPE;
    tx_ctl->tx_vap_index        = hmac_vap->vap_id;
    tx_ctl->tx_user_idx         = (osal_u8)hmac_vap->multi_user_idx;
    tx_ctl->frame_header_length = MAC_80211_FRAME_LEN;
    tx_ctl->ismcast        = OSAL_TRUE;

    /* ack policy暂不用 */
    tx_ctl->ack_policy = WLAN_TX_NO_ACK;
    tx_ctl->tid = WLAN_TIDNO_BCAST;
    tx_ctl->ac  = WLAN_WME_AC_MGMT;
    if (g_al_tx_tpc_code != AL_TX_INVALID_TPC_CODE) {
        tx_ctl->tpc_code = g_al_tx_tpc_code;
        tx_ctl->tpc_code_is_valid = OAL_TRUE;
    }

    hdr = (mac_ieee80211_qos_htc_frame_stru *)oal_netbuf_header(buf);
    /* 将mac header清零 */
    (osal_void)memset_s(hdr, sizeof(mac_ieee80211_frame_stru), 0, sizeof(mac_ieee80211_frame_stru));

    tx_ctl->mpdu_payload_len   = (osal_u16)len;
    hdr->qc_amsdu = OSAL_FALSE;

    /* 挂接802.11头,在data_tx里会将802.11头全部清0 */
    hmac_config_al_tx_set_frame_ctrl(tx_ctl, (mac_ieee80211_frame_stru *)(uintptr_t)hdr);
    hmac_config_al_tx_set_addresses(hmac_vap, (mac_ieee80211_frame_stru *)(uintptr_t)hdr);

    return OAL_SUCC;
}

static osal_u32 hmac_config_al_tx_set_user_packet_addr(const hmac_vap_stru *hmac_vap, mac_ieee80211_frame_stru *hdr)
{
    osal_u32 offset = 0;
    osal_u8 *payload;
    unref_param(hmac_vap);
    /* header_control & duration length */
    offset = (osal_u32)sizeof(mac_header_frame_control_stru) + (osal_u32)sizeof(osal_u16);
    payload = (osal_u8 *)g_al_tx_cfg_param.payload;

    /* header_control & duration */
    if (memcpy_s((osal_u8 *)hdr, offset, payload, offset) != EOK) {
        oam_error_log0(0, OAM_SF_TX, "hmac_config_al_tx_set_addresses_fast::mem safe func err!");
        return OAL_FAIL;
    }
    /* Set Address1 field in the WLAN Header with BSSID, 偏移:4 */
    payload = payload + offset;
    if (memcpy_s(hdr->address1, WLAN_MAC_ADDR_LEN, payload, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_TX, "hmac_config_al_tx_set_addresses_fast::mem safe func err!");
        return OAL_FAIL;
    }
    /* Set Address2 field in the WLAN Header with the source address 偏移:10 */
    payload = payload + WLAN_MAC_ADDR_LEN;
    if (memcpy_s(hdr->address2, WLAN_MAC_ADDR_LEN, payload, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_TX, "hmac_config_al_tx_set_addresses_fast::mem safe func err!");
        return OAL_FAIL;
    }
    /* Set Address3 field in the WLAN Header with the destination address, 偏移:16 */
    payload = payload + WLAN_MAC_ADDR_LEN;
    if (memcpy_s(hdr->address3, WLAN_MAC_ADDR_LEN, payload, WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_TX, "hmac_config_al_tx_set_addresses_fast::mem safe func err!");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static osal_u32 hmac_config_al_tx_set_user_packet(const hmac_vap_stru *hmac_vap, oal_netbuf_stru *buf)
{
    osal_u8 send_result = hmac_config_get_quick_result();
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;       /* SKB CB */
    mac_ieee80211_qos_htc_frame_stru *hdr;

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(buf);
    hdr = (mac_ieee80211_qos_htc_frame_stru *)oal_netbuf_header(buf);

    if (send_result == QUICK_SEND_RESULT_INVALID) {
        /* 根据当前发送状态，判断是否进行将报文发送状态上报host */
        tx_ctl->need_rsp = OSAL_TRUE;
    }

    if (g_al_tx_cfg_param.payload_flag == RF_PAYLOAD_DATA) {
        /* 根据用户输入 修改报文mac头 */
        hmac_config_al_tx_set_user_packet_addr(hmac_vap, (mac_ieee80211_frame_stru *)(uintptr_t)hdr);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 广播数据包
*****************************************************************************/
static osal_u32 hmac_config_al_tx_bcast_pkt(hmac_vap_stru *hmac_vap, osal_u32 payload_len)
{
    osal_u32 len = payload_len;
    oal_netbuf_stru *netbuf = OSAL_NULL;
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;

    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_al_tx_bcast_pkt::hal_device null!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

        /* 申请netbuf，HCC只能传递1600数据，大包在device申请 */
    if (len > WLAN_MEM_NETBUF_ALRX) {
        len = WLAN_MEM_NETBUF_ALRX;
    }

    /* 组包 */
    netbuf = hmac_config_create_al_tx_packet(len, (osal_u8)hmac_vap->payload_flag);
    if (netbuf == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_bcast_pkt::return null!}\r\n", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写cb和mac头 */
    ret = hmac_config_al_tx_packet(hmac_vap, netbuf, len);
    /* 添加al_tx的need_nsp字段 添加用户输入mac头 */
    hmac_config_al_tx_set_user_packet(hmac_vap, netbuf);
    oal_netbuf_put(netbuf, MAC_80211_FRAME_LEN + len);
    /* 发送数据 */
    ret = hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, netbuf);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_al_tx_bcast_pkt_test::hmac_tx_process_data failed[%d].}", ret);
        hmac_tx_excp_free_netbuf(netbuf);
    }
#ifdef _PRE_WLAN_ONLINE_CALI
    online_cali_init_per_frame_timer(hal_device);
#endif
    return ret;
}

/*****************************************************************************
 功能描述  : 关闭常发，常发需求，预留接口
*****************************************************************************/
OSAL_STATIC osal_u32 hmac_config_stop_always_tx_ext(hmac_vap_stru *hmac_vap)
{
    unref_param(hmac_vap);
    return OAL_CONTINUE;
}

/*****************************************************************************
 功能描述  : 停止常发
*****************************************************************************/
static osal_void hmac_config_stop_always_tx(hmac_vap_stru *hmac_vap)
{
    osal_u32 ret;
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;

    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_stop_always_tx::hal_device null.}", hmac_vap->vap_id);
        return;
    }

    hmac_vap->cap_flag.keepalive = OSAL_TRUE;

    hal_rf_test_disable_al_tx(hal_device);

    ret = hmac_config_stop_always_tx_ext(hmac_vap); /* 常发需求，关闭常发预留接口 */
    if (ret != OAL_CONTINUE) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_config_stop_always_tx:stop_always_tx_ext return [%d]", ret);
        return;
    }
    hmac_vap->al_tx_flag = EXT_SWITCH_OFF;

    hal_device->al_tx_flag = EXT_SWITCH_OFF;

    /* 释放用户侧存放报文信息内存 */
    g_al_tx_cfg_param.payload_flag = 0;
    if (g_al_tx_cfg_param.payload != OSAL_NULL) {
        oal_mem_free(g_al_tx_cfg_param.payload, OAL_TRUE);
        g_al_tx_cfg_param.payload = OSAL_NULL;
    }
    hal_save_machw_phy_pa_status(hal_device);
    hal_disable_machw_phy_and_pa();
    /* 清fifo和删除tx队列中所有帧 */
    hmac_clear_hw_fifo_tx_queue(hal_device);
    hmac_hal_device_sync(hal_device);
    hal_recover_machw_phy_and_pa(hal_device);
}

/* 设置常发模式标志 */
static osal_void hmac_config_set_al_tx_param(hmac_vap_stru *hmac_vap, mac_cfg_tx_comp_stru *event_set_al_tx)
{
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    hmac_vap->al_tx_flag = event_set_al_tx->param;
    hmac_vap->payload_flag = event_set_al_tx->payload_flag;
    hal_device->al_tx_flag = event_set_al_tx->param;
    hal_device->al_tx_pkt_len = event_set_al_tx->payload_len;
}

/*****************************************************************************
 功能描述  : 设置常收模式
*****************************************************************************/
static osal_void hmac_config_always_rx_set_hal(hal_to_dmac_device_stru *hal_device_base, osal_u8 switch_code)
{
    osal_u32 int_save;
    osal_s32 ret;
    frw_msg msg_info;

    /* 关闭中断 */
    int_save = frw_osal_irq_lock();
    /* 挂起硬件发送 */
    hal_set_machw_tx_suspend();
    /* 停止mac phy接收 */
    hal_disable_machw_phy_and_pa();
    /* 首先清空发送完成事件队列 */
    frw_event_flush_event_queue(WLAN_MSG_D2D_TX);
    frw_event_flush_event_queue(WLAN_MSG_D2D_CRX_RX);
    frw_event_flush_event_queue(WLAN_MSG_D2D_RX);

    /* 清除硬件发送缓冲区 */
    hal_clear_hw_fifo(hal_device_base);
    hal_psm_clear_mac_rx_isr();
    /* 复位macphy */
    hal_reset_phy_machw(HAL_RESET_HW_TYPE_MAC_PHY, HAL_RESET_MAC_ALL, OSAL_FALSE, OSAL_FALSE);

    /* 置标识bit */
    hal_device_base->al_rx_flag = switch_code;

    (osal_void)memset_s(&msg_info, sizeof(frw_msg), 0, sizeof(frw_msg));
    msg_info.data = (osal_u8 *)&switch_code;
    msg_info.data_len = (osal_u16)sizeof(osal_u8);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_RX_DSCR_RECV, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_SDP, "{frw_send_msg_to_device:: failed! ret[%d].}", ret);
    }

    /* 使能中断 */
    frw_osal_irq_restore(int_save);
    /* 恢复 mac phy接收 */
    if (switch_code == HAL_ALWAYS_RX_RESERVED) {
        hal_set_phy_aid(0x1);   // 开启常收的时候 配置下11ax的用户id 用于mu ppdu等常收
        hal_enable_machw_phy_and_pa(hal_device_base);
    } else {
        hal_set_phy_aid(0x0);
        hal_disable_machw_phy_and_pa();
    }
    /* 使能硬件发送 */
    hal_set_machw_tx_resume();
}

static osal_void hmac_config_al_tx_cfg_param(mac_cfg_tx_comp_stru *event_set_al_tx)
{
    osal_u8 *pkt_data = OSAL_NULL;
    osal_u32 pkt_len = 0;
    osal_s32 ret;

    g_al_tx_cfg_param.param = event_set_al_tx->param;
    g_al_tx_cfg_param.protocol_mode = event_set_al_tx->protocol_mode;
    g_al_tx_cfg_param.payload_flag = event_set_al_tx->payload_flag;
    g_al_tx_cfg_param.ack_policy = event_set_al_tx->ack_policy;
    g_al_tx_cfg_param.payload_len = event_set_al_tx->payload_len;
    g_al_tx_cfg_param.al_tx_max = event_set_al_tx->al_tx_max;

    if (event_set_al_tx->payload_flag == RF_PAYLOAD_DATA) {
        pkt_len = event_set_al_tx->pkt_len;
        pkt_data = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, (osal_u16)pkt_len, OAL_TRUE);
        if (pkt_data == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "hmac_config_al_tx_cfg_param oal_mem_alloc failed.");
            return ;
        }
        (osal_void)memset_s(pkt_data, pkt_len, 0, pkt_len);

        ret = memcpy_s(pkt_data, pkt_len, event_set_al_tx->payload, pkt_len);
        if (ret != OAL_SUCC) {
            oal_mem_free(pkt_data, OAL_TRUE);
            return ;
        }
    }
    g_al_tx_cfg_param.payload = pkt_data;
    g_al_tx_cfg_param.pkt_len = pkt_len;
    return ;
}

/*****************************************************************************
 功能描述  : 停止 常发
*****************************************************************************/
static osal_s32 hmac_config_stop_altx(hmac_vap_stru *hmac_vap)
{
    hmac_vap->al_tx_flag = OAL_SWITCH_OFF;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 用户测试11ax Trigger based ppdu反馈
*****************************************************************************/
osal_u32 hmac_config_send_qos_to_queue(hmac_vap_stru *hmac_vap, osal_u32 len, osal_u8 type)
{
    osal_u32 ret;

    mac_tx_ctl_stru *tx_ctl; /* SKB CB */
    mac_ieee80211_qos_htc_frame_stru *hdr;
    /* MAC发帧时会检查队列的帧地址是否与TA地址一致，并且检查当前发帧长度是否小于 trig帧指定帧长，检查PASS才会发送;
       但是当前IQxel无法在single user时设置SA地址,因此qos帧的DA必须设置为0 */
    /* Single user */
    osal_u8 addr[WLAN_MAC_ADDR_LEN] = { 0x06, 0x05, 0x04, 0x03, 0x02, 0x04 };
    /* 组包 */
    oal_netbuf_stru *netbuf = hmac_config_create_al_tx_packet(len, (osal_u8)hmac_vap->payload_flag);
    if (netbuf == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_send_qos_to_queue::buf null!}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写cb和mac头 */
    ret = hmac_config_al_tx_packet(hmac_vap, netbuf, len);

    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(netbuf);

    tx_ctl->frame_header_length = MAC_80211_FRAME_LEN;
    tx_ctl->ismcast = OSAL_TRUE;
    /* 保存常收tid type值 */
    tx_ctl->al_rx_tid_type = type;

    oam_warning_log3(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_send_qos_to_queue:: tid%d ac%d!}\r\n", hmac_vap->vap_id,
                     tx_ctl->tid, tx_ctl->ac);

    hdr = (mac_ieee80211_qos_htc_frame_stru *)oal_netbuf_header(netbuf);
    /* Set Address1 field in the WLAN Header with BSSID */
    (osal_void)memcpy_s((osal_void *)(hdr->address1), WLAN_MAC_ADDR_LEN,
        (osal_void *)(addr), WLAN_MAC_ADDR_LEN);
    /* Set Address3 field in the WLAN Header with the destination address */
    (osal_void)memcpy_s((osal_void *)(hdr->address3), WLAN_MAC_ADDR_LEN,
        (osal_void *)(addr), WLAN_MAC_ADDR_LEN);

    mac_hdr_set_frame_control((osal_u8 *)hdr, WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS);
    /* 设置QOS控制字段 */
    hdr->qc_tid = tx_ctl->tid;
    hdr->qc_eosp = 0;
    /* 此处可控制是否需要对端回复ACK,需要回复则MAC一直在队列中等待发送 */
    hdr->qc_ack_polocy = WLAN_TX_NO_ACK;
    hdr->qc_amsdu = OSAL_FALSE;
    hdr->qos_control.qc_txop_limit = 0;
    oal_netbuf_put(netbuf, MAC_80211_FRAME_LEN + len);

    /* 发送数据 */
    ret = hmac_tx_process_data(hmac_vap->hal_device, hmac_vap, netbuf);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_send_qos_to_queue::hmac_tx_process_data failed[%d].}", ret);
        hmac_tx_excp_free_netbuf(netbuf);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 组一个报文
*****************************************************************************/
oal_netbuf_stru *hmac_config_create_al_tx_packet(osal_u32 size, mac_rf_payload_enum_uint8 payload_flag)
{
    oal_netbuf_stru *buf = OSAL_NULL;
    osal_u8 *data = OSAL_NULL;
    osal_u32 pkt_len = 0;
    osal_u32 tx_size;

    /* 申请netbuf，可申请最大4096byte的netbuf */
    if (size > WLAN_MEM_NETBUF_ALRX || size == 0) {
        tx_size = WLAN_MEM_NETBUF_ALRX;
    } else {
        tx_size = size;
    }

    buf = (tx_size <= WLAN_LARGE_NETBUF_SIZE) ?
        oal_netbuf_alloc_ext(OAL_NORMAL_NETBUF, (osal_u16)tx_size, OAL_NETBUF_PRIORITY_MID) :
        oal_mem_multi_netbuf_alloc((osal_u16)tx_size);
    if (osal_unlikely(buf == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_TX, "hmac_config_create_al_tx_packet::alloc Fail");
        return OSAL_NULL;
    }

    data = oal_netbuf_data_offset(buf, MAC_80211_FRAME_LEN);
    switch (payload_flag) {
        case RF_PAYLOAD_ALL_ZERO:
            (osal_void)memset_s(data, tx_size, 0, tx_size);
            break;
        case RF_PAYLOAD_ALL_ONE:
            (osal_void)memset_s(data, tx_size, 0xFF, tx_size);
            break;
        case RF_PAYLOAD_ALL_AA:
            (osal_void)memset_s(data, tx_size, 0xAA, tx_size);
            break;
        case RF_PAYLOAD_RAND:
             osal_get_trng_bytes(data, size);
            break;
        case RF_PAYLOAD_DATA:
            (osal_void)memset_s(data, tx_size, 0, tx_size);
            if (g_al_tx_cfg_param.pkt_len >= MAC_80211_FRAME_LEN) {
                pkt_len = (g_al_tx_cfg_param.pkt_len - MAC_80211_FRAME_LEN) > tx_size ?
                    tx_size : (g_al_tx_cfg_param.pkt_len - MAC_80211_FRAME_LEN);
                if (memcpy_s(data, pkt_len, g_al_tx_cfg_param.payload + MAC_80211_FRAME_LEN, pkt_len) != EOK) {
                    oal_netbuf_free(buf);
                    return OSAL_NULL;
                }
            }
            break;
        default:
            break;
    }

    buf->next = OSAL_NULL;
    return buf;
}

/*****************************************************************************
 功能描述  : 常发发送完成事件
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_al_tx_complete_event_handler(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *device_vap = OSAL_NULL;

    unref_param(hmac_vap);
    unref_param(msg);

    device_vap = mac_res_get_hmac_vap(hmac_device->vap_id[0]);
    if (device_vap == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_TX, "{hmac_al_tx_complete_event_handler::hmac_vap[%d] null.}", hmac_device->vap_id[0]);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_config_stop_always_tx(device_vap);

    /* 通知hmac关闭常发 */
    return hmac_config_stop_altx(hmac_vap);
}

enum pm_switch_cfg {
    ALL_DISABLE                   = 0,          /* 平台关低功耗+rf和mac pa常开 */
    ALL_ENABLE                    = 1,          /* 平台开低功耗+业务控rf和mac pa */
    MAC_PA_SWTICH_EN_RF_SWTICH_EN = 2,          /* 平台关低功耗+业务控rf和mac pa */
    MAC_PA_SWTICH_EN_RF_ALWAYS_ON = 3,          /* 平台关低功耗+业务控mac pa+rf常开 */
    LIGHT_SLEEP_SWITCH_EN         = 4,          /* 平台关深睡+业务控mac pa+rf常开 */
};

OSAL_STATIC osal_void hmac_set_pm_para(const hmac_vap_stru *hmac_vap, osal_u8 pm_switch, osal_u8 mac_pa_switch,
    osal_u8 rf_cfg)
{
    unref_param(pm_switch);
    hal_device_set_mac_pa_switch(mac_pa_switch);

    if ((hmac_vap != OSAL_NULL) && (hmac_vap->hal_device != OSAL_NULL)) {
        hmac_vap->hal_device->hal_dev_fsm.mac_pa_switch_en = hal_device_get_mac_pa_switch();
        hal_dev_sync_mac_pa_switch_to_dev(mac_pa_switch);
    }
    oam_warning_log3(0, OAM_SF_PWR,
        "hmac_set_pm_para:pm_switch[%d], mac_pa_switch[%d], rf_switch[%d].",
        pm_switch, mac_pa_switch, rf_cfg);
}

osal_u32 hmac_config_set_pm_switch(const hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param)
{
    unref_param(len);
    switch (*param) {
        case ALL_DISABLE:
            hmac_set_pm_para(hmac_vap, OSAL_FALSE, OSAL_FALSE, HAL_RF_ALWAYS_POWER_ON);
            break;
        case ALL_ENABLE:
            hmac_set_pm_para(hmac_vap, OSAL_TRUE, OSAL_TRUE, HAL_RF_SWITCH_ALL_LDO);
            break;
        case MAC_PA_SWTICH_EN_RF_SWTICH_EN:
            hmac_set_pm_para(hmac_vap, OSAL_FALSE, OSAL_TRUE, HAL_RF_SWITCH_ALL_LDO);
            break;
        case MAC_PA_SWTICH_EN_RF_ALWAYS_ON:
            hmac_set_pm_para(hmac_vap, OSAL_FALSE, OSAL_TRUE, HAL_RF_ALWAYS_POWER_ON);
            break;
        default:
            oam_warning_log1(0, OAM_SF_PWR, "hmac_config_set_pm_switch:invalid param %d", *param);
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置常收模式
*****************************************************************************/
osal_s32 hmac_config_set_always_rx(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hal_to_dmac_device_stru *hal_device_base = hmac_vap->hal_device;
    osal_u16 size = (osal_u16)sizeof(hal_to_dmac_vap_stru);
    osal_u8 netbuf_size = 120;
    osal_u8 al_rx_flag = *(oal_bool_enum_uint8 *)msg->data;
    osal_u8 addr[WLAN_MAC_ADDR_LEN] = { 0x06, 0x05, 0x04, 0x03, 0x02, 0x04 }; /* 仪器发trigger帧的mac地址 */
    oal_bool_enum_uint8 pm_switch;
    if (hal_device_base == OSAL_NULL) {
        return OAL_FAIL;
    }
    hal_device_base->al_txrx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM;
    if (*(oal_bool_enum_uint8 *)msg->data == hal_device_base->al_rx_flag) {
        return OAL_SUCC;
    }

    hmac_config_always_send_msg();
    hmac_vap->al_rx_flag = al_rx_flag;
    if (al_rx_flag == HAL_ALWAYS_RX_RESERVED) {
        if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hal_device_handle_event(hal_device_base, HAL_DEVICE_EVENT_JOIN_COMP, size, (osal_u8 *)(hmac_vap->hal_vap));
        }
        hal_device_handle_event(hal_device_base, HAL_DEVICE_EVENT_VAP_UP, size, (osal_u8 *)(hmac_vap->hal_vap));
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
        (osal_void)hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"traffic_dscr_min_thres 8"); // 开常收，则8个大包阈值
#endif
    } else if (al_rx_flag == HAL_ALWAYS_RX_DISABLE) {
        hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_VAP_DOWN, size, (osal_u8 *)(hmac_vap->hal_vap));
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
        (osal_void)hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"traffic_dscr_min_thres 2"); // 关常收，则2个大包阈值
#endif
    }
    pm_switch = ((al_rx_flag == HAL_ALWAYS_RX_DISABLE) ? OSAL_TRUE : OSAL_FALSE);
    /* Switch low power consumption failure to continue to execute the often
    received command without judging the return value */
    (osal_void)hmac_config_set_pm_switch(hmac_vap, 0, &pm_switch);
    hmac_vap_work_set_channel(hmac_vap);
    if ((al_rx_flag == HAL_ALWAYS_RX_RESERVED) || (al_rx_flag == HAL_ALWAYS_RX_DISABLE)) {
        hmac_config_always_rx_set_hal(hal_device_base, al_rx_flag);
    }
    // 常收时处理算法开关
    hmac_config_set_always_rx_alg_off(hmac_vap, al_rx_flag);
    if (hal_device_base->current_11ax_working == OSAL_TRUE) {
        /* testing UL OFDMA for 11ax long reception, suspend the MAC so that the MAC will
        automatically feedback data after receiving the Trigger frame. */
        hal_set_machw_tx_suspend();
        hmac_config_send_qos_to_queue(hmac_vap, netbuf_size, WLAN_WME_AC_BE);
        hmac_config_send_qos_to_queue(hmac_vap, netbuf_size, WLAN_WME_AC_BK);
        hmac_config_send_qos_to_queue(hmac_vap, netbuf_size, WLAN_WME_AC_VI);
        hmac_config_send_qos_to_queue(hmac_vap, netbuf_size, WLAN_WME_AC_VO);
        hal_set_sta_bssid(hmac_vap->hal_vap, addr, WLAN_MAC_ADDR_LEN);
        hal_set_mac_aid(hmac_vap->hal_vap, 1);
        hal_set_bss_color(hmac_vap->hal_vap, 1);
    }
    hmac_config_set_rx_framefilt(al_rx_flag);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置描述符参数
*****************************************************************************/
osal_s32 hmac_config_set_always_tx(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_tx_comp_stru *event_set_al_tx = (mac_cfg_tx_comp_stru *)msg->data;
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    g_al_tx_tpc_code = event_set_al_tx->tpc_code;
    if (g_al_tx_tpc_code != AL_TX_INVALID_TPC_CODE) {
        wifi_printf("Set al_tx tpc_code:%d\r\n", g_al_tx_tpc_code);
    }
    /* 如果新下发的命令开关状态没变，则直接返回 */
    if (event_set_al_tx->param == hmac_vap->al_tx_flag) {
        return OAL_SUCC;
    }

    /* 大包才初始化device侧内存池大小 */
    if (event_set_al_tx->payload_len > WLAN_MEM_NETBUF_ALRX) {
        hmac_config_always_send_msg();
    }
    /* 设置常发模式标志 */
    hmac_config_set_al_tx_param(hmac_vap, event_set_al_tx);
    hmac_hal_device_sync(hal_device);
    if (hal_device->al_tx_flag == HAL_ALWAYS_TX_RF) {
        osal_u8 pm_off = 0;
        if ((hal_device->hal_dev_fsm.oal_fsm.cur_state != HAL_DEVICE_WORK_STATE) &&
            hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_JOIN_COMP, sizeof(hal_to_dmac_vap_stru),
                                    (osal_u8 *)(hmac_vap->hal_vap));
        }
        hmac_config_set_pm_switch(hmac_vap, 0, &pm_off);
        hmac_vap_work_set_channel(hmac_vap);
        hmac_vap->cap_flag.keepalive = OSAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
        /* 常发打开，关闭弱干扰免疫算法,并将agc unlock门限配置为0x0 */
        if (hmac_alg_anti_intf_switch(hal_device, OSAL_FALSE) != OAL_SUCC) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_set_always_tx:anti_intf_switch fail}", hmac_vap->vap_id);
        }
        hal_set_agc_unlock_min_th(0, 0);
#endif
        g_payload_len = event_set_al_tx->payload_len;

        hmac_config_al_tx_cfg_param(event_set_al_tx);
        hmac_config_set_quick_result(QUICK_SEND_RESULT_INVALID);

        if (hmac_config_al_tx_bcast_pkt(hmac_vap, g_payload_len) != OAL_SUCC) {
            wifi_printf("al tx fail\r\n");
        }
    } else if (hal_device->al_tx_flag == HAL_ALWAYS_TX_DISABLE) {
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
        if (hmac_alg_anti_intf_switch(hal_device, OSAL_TRUE) != OAL_SUCC) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_config_set_always_tx:anti_intf_switch fail}", hmac_vap->vap_id);
        }
        hal_set_agc_unlock_min_th(ALG_ANTI_INF_UNLOCK_TX_TH_DEFAULT_VAL, ALG_ANTI_INF_UNLOCK_RX_TH_DEFAULT_VAL);
#endif
        hmac_config_stop_always_tx(hmac_vap);
        hal_device_handle_event(hmac_vap->hal_device, HAL_DEVICE_EVENT_VAP_DOWN, sizeof(hal_to_dmac_vap_stru),
            (osal_u8 *)(hmac_vap->hal_vap));
    }
    return OAL_SUCC;
}

osal_s32 hmac_config_get_rx_ppdu_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_al_rx_info_stru *rx_info = (mac_cfg_al_rx_info_stru *)msg->data;
    osal_u8 clean_flag = OAL_TRUE;
    unref_param(hmac_vap);

    if (rx_info->data_op == 0) {
        clean_flag = OAL_FALSE;
    }
    hal_rf_get_rx_ppdu_info(clean_flag);
    return OAL_SUCC;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
