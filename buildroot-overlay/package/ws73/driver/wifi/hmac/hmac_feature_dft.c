/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: DFT相关函数实现.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_feature_dft.h"
#include "oam_struct.h"

#include "hmac_user.h"
#include "oal_netbuf_data.h"
#include "hmac_dfx.h"
#include "hmac_sme_sta.h"
#include "hmac_scan.h"
#include "hmac_ext_if.h"
#include "hmac_btcoex.h"
#include "hmac_feature_interface.h"
#include "hmac_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_FEATURE_DFT_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oam_stat_info_stru g_st_stat_info_etc;

#ifdef _PRE_WLAN_DFT_STAT
osal_u32 *g_hmac_vap_pkts_stat[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT] = {OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL};

/*****************************************************************************
  以下定义移植自dmac_dft_rom.c，待后续功能调试后完全融合
*****************************************************************************/

dft_hw_statis_cnt_stru *g_hw_statis_cnt = OSAL_NULL;       /* 硬件收发包统计，单独成模块 */

/*****************************************************************************
  3 函数实现
*****************************************************************************/
osal_void hmac_hw_stat_display(osal_void)
{
    if (g_hw_statis_cnt == OAL_PTR_NULL) {
        return;
    }
    /* 打印出hw的收发包状态 */
    wifi_printf("====================DMAC TX stat info========================\r\n");
    wifi_printf("tx_from_dmac_mpdu_cnt          = %u\r\n", g_hw_statis_cnt->tx_from_dmac_mpdu_cnt);
    wifi_printf("tx_complete_mpdu_cnt           = %u\r\n", g_hw_statis_cnt->tx_complete_mpdu_cnt);
    wifi_printf("\r\n");
    wifi_printf("==================== HW  TX stat info========================\r\n");
    wifi_printf("tx_high_priority_mpdu_cnt      = %u\r\n", g_hw_statis_cnt->mac_tx_statis.tx_high_priority_mpdu_cnt);
    wifi_printf("tx_normal_priority_mpdu_cnt    = %u\r\n", g_hw_statis_cnt->mac_tx_statis.tx_normal_priority_mpdu_cnt);
    wifi_printf("tx_ampdu_inmpdu_cnt            = %u\r\n", g_hw_statis_cnt->mac_tx_statis.tx_ampdu_inmpdu_cnt);
    wifi_printf("tx_ampdu_cnt                   = %u\r\n", g_hw_statis_cnt->mac_tx_statis.tx_ampdu_cnt);
    wifi_printf("tx_complete_cnt                = %u\r\n", g_hw_statis_cnt->mac_tx_statis.tx_complete_cnt);
    wifi_printf("\r\n");
    wifi_printf("====================DMAC RX stat info========================\r\n");
    wifi_printf("rx_from_hw_mpdu_cnt            = %u\r\n", g_hw_statis_cnt->rx_from_hw_mpdu_cnt);
    wifi_printf("\r\n");
    wifi_printf("==================== HW  RX stat info========================\r\n");
    wifi_printf("rx_ampdu_cnt                   = %u\r\n", g_hw_statis_cnt->mac_rx_statis.rx_ampdu_cnt);
    wifi_printf("rx_suc_mpdu_inampdu_cnt        = %u\r\n", g_hw_statis_cnt->mac_rx_statis.rx_suc_mpdu_inampdu_cnt);
    wifi_printf("rx_err_mpdu_inampdu_cnt        = %u\r\n", g_hw_statis_cnt->mac_rx_statis.rx_err_mpdu_inampdu_cnt);
    wifi_printf("rx_suc_mpdu_cnt                = %u\r\n", g_hw_statis_cnt->mac_rx_statis.rx_suc_mpdu_cnt);
    wifi_printf("rx_err_mpdu_cnt                = %u\r\n", g_hw_statis_cnt->mac_rx_statis.rx_err_mpdu_cnt);
    wifi_printf("rx_filter_mpdu_cnt             = %u\r\n", g_hw_statis_cnt->mac_rx_statis.rx_filter_mpdu_cnt);
    wifi_printf("\r\n");
}

osal_void hmac_vap_stat_rx_display(osal_u8 vap_id)
{
    osal_u32 *hmac_vap_stat = OAL_PTR_NULL;

    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        return;
    }
    if (g_hmac_vap_pkts_stat[vap_id] == OAL_PTR_NULL) {
        return;
    }

    hmac_vap_stat = g_hmac_vap_pkts_stat[vap_id];

    /* 打印返回的统计信息 */
    /* 数据帧 */
    wifi_printf("\r\n");
    wifi_printf("====================vap rx stats=======================\r\n");
    wifi_printf("====================DEVICE DATA RX stats========================\r\n");
    wifi_printf("rx_device_d_from_hw_msdu            = %u\r\n", hmac_vap_stat[RX_DEVICE_D_FROM_HW_MPDU]);
    wifi_printf("rx_device_d_send_hmac_succ          = %u\r\n", hmac_vap_stat[RX_DEVICE_D_SEND_HMAC_SUCC]);
    wifi_printf("====================HMAC DATA RX stats========================\r\n");
    wifi_printf("rx_d_from_device_pkts               = %u\r\n", hmac_vap_stat[RX_D_FROM_DEVICE_PKTS]);
    wifi_printf("rx_d_send_lan_succ                  = %u\r\n", hmac_vap_stat[RX_D_SEND_LAN_SUCC]);
    wifi_printf("rx_d_send_lan_fail                  = %u\r\n", hmac_vap_stat[RX_D_SEND_LAN_FAIL]);
    /* 管理控制帧帧 */
    wifi_printf("====================DEVICE MGMT RX stats========================\r\n");
    wifi_printf("rx_devcie_m_from_hw_msdu            = %u\r\n", hmac_vap_stat[RX_DEVICE_M_FROM_HW_MPDU]);
    wifi_printf("rx_device_m_send_hmac_succ          = %u\r\n", hmac_vap_stat[RX_DEVICE_M_SEND_HMAC_SUCC]);
    wifi_printf("====================HMAC MGMT RX stats========================\r\n");
    wifi_printf("rx_m_from_device_pkts               = %u\r\n", hmac_vap_stat[RX_M_FROM_DEVICE_PKTS]);
    wifi_printf("rx_c_from_device_pkts               = %u\r\n", hmac_vap_stat[RX_C_FROM_DEVICE_PKTS]);
    wifi_printf("rx_m_send_wal_succ_pkts             = %u\r\n", hmac_vap_stat[RX_M_SEND_WAL_SUCC_PKTS]);
    wifi_printf("rx_m_send_wal_fail_pkts             = %u\r\n", hmac_vap_stat[RX_M_SEND_WAL_FAIL_PKTS]);
    wifi_printf("rx_m_send_lan_succ_pkts             = %u\r\n", hmac_vap_stat[RX_M_SEND_LAN_SUCC_PKTS]);
    wifi_printf("rx_m_send_lan_fail_pkts             = %u\r\n", hmac_vap_stat[RX_M_SEND_LAN_FAIL_PKTS]);
    wifi_printf("\r\n");
}

osal_void hmac_vap_stat_tx_display(osal_u8 vap_id)
{
    osal_u32 *hmac_vap_stat = OAL_PTR_NULL;

    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        return;
    }
    if (g_hmac_vap_pkts_stat[vap_id] == OAL_PTR_NULL) {
        return;
    }

    hmac_vap_stat = g_hmac_vap_pkts_stat[vap_id];
    /* 打印返回的统计信息 */
    /* 数据帧 */
    wifi_printf("====================vap tx stats=======================\r\n");
    wifi_printf("====================HMAC DATA TX stats========================\r\n");
    wifi_printf("tx_d_from_lan_pkts                  = %u\r\n", hmac_vap_stat[TX_D_FROM_LAN_PKTS]);
    wifi_printf("tx_d_send_device_succ               = %u\r\n", hmac_vap_stat[TX_D_SEND_DEVICE_SUCC]);
    wifi_printf("tx_d_send_device_fail               = %u\r\n", hmac_vap_stat[TX_D_SEND_DEVICE_FAIL]);
    wifi_printf("====================DEVICE DATA TX stats========================\r\n");
    wifi_printf("tx_device_d_from_hmac_msdu          = %u\r\n", hmac_vap_stat[TX_DEVICE_D_FROM_HMAC_MPDU]);
    wifi_printf("tx_device_d_from_inner              = %u\r\n", hmac_vap_stat[TX_DEVICE_D_FROM_INNER]);
    wifi_printf("tx_device_d_complete_succ           = %u\r\n", hmac_vap_stat[TX_DEVICE_D_COMPLETE_SUCC]);
    /* 管理帧 */
    wifi_printf("====================WAL MGMT TX stats========================\r\n");
    wifi_printf("tx_m_from_kernel_pkts               = %u\r\n", hmac_vap_stat[TX_M_FROM_KERNEL_PKTS]);
    wifi_printf("tx_m_send_hmac_succ_pkts            = %u\r\n", hmac_vap_stat[TX_M_SEND_HMAC_SUCC_PKTS]);
    wifi_printf("tx_m_send_hmac_fail_pkts            = %u\r\n", hmac_vap_stat[TX_M_SEND_HMAC_FAIL_PKTS]);
    wifi_printf("====================HMAC MGMT TX stats========================\r\n");
    wifi_printf("tx_m_from_wal_pkts                  = %u\r\n", hmac_vap_stat[TX_M_FROM_WAL_PKTS]);
    wifi_printf("tx_m_send_device_succ               = %u\r\n", hmac_vap_stat[TX_M_SEND_DEVICE_SUCC]);
    wifi_printf("tx_m_send_device_fail               = %u\r\n", hmac_vap_stat[TX_M_SEND_DEVICE_FAIL]);
    wifi_printf("====================DEVICE MGMT TX stats========================\r\n");
    wifi_printf("tx_device_m_from_hmac_msdu          = %u\r\n", hmac_vap_stat[TX_DEVICE_M_FROM_HMAC_MPDU]);
    wifi_printf("tx_device_m_from_inner              = %u\r\n", hmac_vap_stat[TX_DEVICE_M_FROM_INNER]);
    wifi_printf("tx_device_m_complete_succ           = %u\r\n", hmac_vap_stat[TX_DEVICE_M_COMPLETE_SUCC]);
    wifi_printf("\r\n");
}

osal_s32 hmac_config_vap_stat_enable(dft_vap_statis_command_stru *vap_statis_command)
{
    osal_u8 vap_stat_enable;
    osal_u8 vap_id;
    frw_msg msg_to_device = {0};
    osal_s32 ret;

    if (vap_statis_command == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_vap_stat_enable::vap_statis_common is null.}");
        return OAL_FAIL;
    }

    vap_stat_enable = vap_statis_command->enable;
    vap_id = vap_statis_command->vap_id;

    if (vap_stat_enable > 1) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_vap_stat_enable::vap_stat_enable[%u] invalid.}", vap_stat_enable);
        return OAL_FAIL;
    }
    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_config_vap_stat_enable::parma error vap_id[%d].}", vap_id);
        return OAL_FAIL;
    }

    frw_msg_init((osal_u8 *)vap_statis_command, sizeof(dft_vap_statis_command_stru), OAL_PTR_NULL, 0, &msg_to_device);
    ret = frw_send_msg_to_device(vap_id, WLAN_MSG_H2D_C_CFG_ENABLE_VAT_STAT, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_get_vap_stat_new:: send msg to device fail=%d!", ret);
        return OAL_FAIL;
    }

    if (vap_stat_enable == 0) {
        hmac_dft_vap_stat_deinit(vap_id);
    } else {
        hmac_dft_vap_stat_init(vap_id);
    }
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_pkts_tx_stat(osal_u8 vap_id,
    osal_u32 msdu_num, dft_hmac_vap_stat_enum stat_type)
{
    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT || g_hmac_vap_pkts_stat[vap_id] == OAL_PTR_NULL) {
        return;
    }
    g_hmac_vap_pkts_stat[vap_id][stat_type] += msdu_num;
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_pkts_rx_stat(osal_u8 vap_id,
    osal_u32 msdu_num, dft_hmac_vap_stat_enum stat_type)
{
    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT || g_hmac_vap_pkts_stat[vap_id] == OAL_PTR_NULL) {
        return;
    }
    g_hmac_vap_pkts_stat[vap_id][stat_type] += msdu_num;
}

osal_void hmac_dft_print_drop_frame_info(osal_u32 file_id, osal_u16 line_num, osal_u32 pkt_num,
    oal_netbuf_stru *netbuf)
{
    osal_u32 frame_type;
    osal_u32 frame_subtype;

    /* 调用处无法获取netbuf 传入NULL 此情况不打印frame type */
    if (netbuf == OAL_PTR_NULL) {
        oam_warning_log2(0, OAM_SF_DFT, "netbuf is NULL, [%u:%d] DROP frame", file_id, line_num);
        return;
    }

    frame_type = mac_get_frame_type(oal_netbuf_header(netbuf));
    frame_subtype = mac_frame_get_subtype_value(oal_netbuf_header(netbuf));

    if ((dfx_get_service_control() & SERVICE_CONTROL_DROP_FRAME) != 0) {
        wifi_printf("[SERVICE][%u:%d] DROP frame count [%d] frame type [%d] subtype [%d]\r\n",
            file_id, line_num, pkt_num, frame_type, frame_subtype);
    }

    /* 传入netbuf 解析出帧类型 5:日志参数5个 */
    oam_warning_log2(0, OAM_SF_DFT, "[%u:%d] DROP frame", file_id, line_num);
    oam_warning_log3(0, OAM_SF_DFT, "count [%d] frame type [%d] subtype [%d]", pkt_num, frame_type, frame_subtype);
    return;
}

/* 原dmac dft统计函数开始 */
osal_s32 hmac_get_vap_stat_new(dft_vap_statis_command_stru *vap_statis_command)
{
    osal_u8 vap_id;
    osal_s32 ret;
    frw_msg msg_to_device = {0};
    osal_u32 device_vap_stat[DEVICE_VAP_STAT_NUM] = {0};

    if (vap_statis_command == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{dmac_config_vap_stat_enable:vap_statis_command is null}");
        return OAL_FAIL;
    }

    vap_id = vap_statis_command->vap_id;

    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_get_vap_stat_new::parma error vap_id[%d].}", vap_id);
        return OAL_FAIL;
    }

    if (g_hmac_vap_pkts_stat[vap_id] == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_get_vap_stat_new:: pamar is null !");
        return OAL_FAIL;
    }

    frw_msg_init(&vap_id, sizeof(vap_id), (osal_u8 *)device_vap_stat,
        DEVICE_VAP_STAT_NUM * sizeof(osal_u32), &msg_to_device);
    ret = frw_send_msg_to_device(vap_id, WLAN_MSG_H2D_C_CFG_GET_VAT_STAT, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_get_vap_stat_new:: send msg to device fail=%d!", ret);
        return OAL_FAIL;
    }

    ret = memcpy_s((osal_u8*)(g_hmac_vap_pkts_stat[vap_id] + HOST_PKTS_CNT_TYPE_END),
        DEVICE_VAP_STAT_NUM * sizeof(osal_u32), (osal_u8*)device_vap_stat,
        DEVICE_VAP_STAT_NUM * sizeof(osal_u32));
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_get_vap_stat_new:: memcopy fail!");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rx_vap_stat_etc(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *buf, dmac_rx_ctl_stru *rx_cb)
{
    osal_u16 bytes;
    mac_rx_ctl_stru *rx_info = &(rx_cb->rx_info);
    mac_ieee80211_frame_stru *frame_hdr = OSAL_NULL;

    frame_hdr = (mac_ieee80211_frame_stru *)(oal_netbuf_header(buf));

    if (rx_info->amsdu_enable == 0) {
        /* 此mpdu只有一个msdu，字节数应该去掉snap长度 */
        if (frame_hdr->frame_control.type == WLAN_DATA_BASICTYPE) {
            hmac_vap->query_stats.rx_d_from_hw_pkts++;
            hmac_vap->query_stats.rx_rcv_data_bytes +=
                ((rx_cb->rx_info.frame_len - rx_cb->rx_info.mac_header_len) - SNAP_LLC_FRAME_LEN);
        }
        if (frame_hdr->frame_control.type == WLAN_DATA_BASICTYPE) {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_D_FROM_DEVICE_PKTS);
        } else {
            hmac_pkts_rx_stat(hmac_vap->vap_id, 1, RX_M_FROM_DEVICE_PKTS);
        }
    } else { /* 如果此mpdu里面是amsdu，需要获取sub_msdu个数和每一个sub_msdu的字节数 */
        if (frame_hdr->frame_control.type == WLAN_DATA_BASICTYPE) {
            /* 计算此mpdu的字节数目，所有sub_msdt字节数之和，不包括snap，不包括padding，注意最后一个sub_msdu没有padding */
            bytes = rx_cb->rx_info.frame_len - rx_cb->rx_info.mac_header_len;
            hmac_pkts_rx_stat(hmac_vap->vap_id, rx_cb->rx_info.msdu_in_buffer, RX_D_FROM_DEVICE_PKTS);
            hmac_vap->query_stats.rx_d_from_hw_pkts += rx_cb->rx_info.msdu_in_buffer;
            bytes -= SNAP_LLC_FRAME_LEN * rx_cb->rx_info.msdu_in_buffer;
            hmac_vap->query_stats.rx_rcv_data_bytes += bytes;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_rx_vap_stat(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    dmac_rx_ctl_stru *rx_cb;

    if (hmac_vap == OAL_PTR_NULL || netbuf == OAL_PTR_NULL || *netbuf == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_DFT, "{hmac_rx_vap_stat:input parameter is null}");
        return OAL_CONTINUE;
    }

    if (hmac_vap->vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT || g_hmac_vap_pkts_stat[hmac_vap->vap_id] == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_DFT, "{hmac_rx_vap_stat:vap_id error!}");
        return OAL_CONTINUE;
    }

    rx_cb = (dmac_rx_ctl_stru *)oal_netbuf_cb(*netbuf);
    hmac_rx_vap_stat_etc(hmac_vap, *netbuf, rx_cb);
    return OAL_CONTINUE;
}

hmac_netbuf_hook_stru g_vapstat_data_netbuf_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_rx_vap_stat,
};

hmac_netbuf_hook_stru g_vapstat_mgmt_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_rx_vap_stat,
};

osal_u32 hmac_dft_vap_stat_init(osal_u8 vap_id)
{
    if (g_hmac_vap_pkts_stat[vap_id] == OAL_PTR_NULL) {
        g_hmac_vap_pkts_stat[vap_id] =
             oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(osal_u32) * HMAC_PKTS_CNT_TYPE_BUTT, OAL_TRUE);
        if (g_hmac_vap_pkts_stat[vap_id] == OAL_PTR_NULL) {
            oam_warning_log1(0, OAM_SF_CFG,
                "{hmac_dft_vap_stat_init::g_hmac_vap_pkts_stat[%d] malloc memory fail}", vap_id);
            return OAL_FAIL;
        }
    }
    /* 重新vap使能，若该vap存在，则会重置vap的统计数据 */
    memset_s(g_hmac_vap_pkts_stat[vap_id], sizeof(osal_u32) * HMAC_PKTS_CNT_TYPE_BUTT,
        0, sizeof(osal_u32) * HMAC_PKTS_CNT_TYPE_BUTT);

    (osal_void)hmac_register_netbuf_hook(&g_vapstat_data_netbuf_hook);
    (osal_void)hmac_register_netbuf_hook(&g_vapstat_mgmt_netbuf_hook);

    oam_warning_log1(0, OAM_SF_CFG,
        "{hmac_dft_vap_stat_init::g_hmac_vap_pkts_stat[%d]init succ.}", vap_id);
    return OAL_SUCC;
}

/* 去使能vap收发包统计 */
osal_u32 hmac_dft_vap_stat_deinit(osal_u8 vap_id)
{
    if (vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT) {
        return OAL_FAIL;
    }
    /* 释放该VAP空间 */
    if (g_hmac_vap_pkts_stat[vap_id] != OAL_PTR_NULL) {
        oal_mem_free(g_hmac_vap_pkts_stat[vap_id], OAL_TRUE);
        g_hmac_vap_pkts_stat[vap_id] = OSAL_NULL;
    }

    (osal_void)hmac_unregister_netbuf_hook(&g_vapstat_data_netbuf_hook);
    (osal_void)hmac_unregister_netbuf_hook(&g_vapstat_mgmt_netbuf_hook);

    oam_warning_log0(0, OAM_SF_CFG, "{hmac_dft_vap_stat_deinit::free the memory.}");
    return OAL_SUCC;
}

/*****************************************************************************
  xr 以下函数定义移植自dmac_dft_rom.c，待后续功能调试后完全融合
*****************************************************************************/

/* 获取MAC层硬件统计数据 */
OAL_STATIC osal_void hmac_dft_get_hw_cnt(dft_hw_statis_cnt_stru *hw_statis_cnt)
{
    hal_get_mac_tx_statistics_data(&hw_statis_cnt->mac_tx_statis);
    hal_get_mac_rx_statistics_data(&hw_statis_cnt->mac_rx_statis);
}

WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_u32 hmac_dft_rx_get_dscr_msdu(oal_netbuf_stru *netbuf, osal_u32 netbuf_num)
{
    osal_u8 i;
    osal_u32 pkts = 0;
    oal_netbuf_stru *tmp_netbuf = netbuf;

    for (i = 0; i < netbuf_num; i++) {
        mac_rx_ctl_stru *rx_cb = OSAL_NULL;
        if (tmp_netbuf == NULL) {
            break;
        }

        rx_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(tmp_netbuf);
        if (rx_cb->amsdu_enable == OSAL_FALSE) {
            pkts++;
        } else { /* for amsdu adapt */
            pkts += rx_cb->msdu_in_buffer;
        }
        /* 指向netbuf链表的下一个netbuf */
        tmp_netbuf = tmp_netbuf->next;
    }
    return pkts;
}

/*****************************************************************************
 函 数 名  : hmac_dft_report_linkloss_info
 功能描述  : 通过OTA上报linkloss统计信息
*****************************************************************************/
osal_void  hmac_dft_report_linkloss_info(const hmac_vap_stru *hmac_sta, const hmac_device_stru *hmac_device)
{
    osal_void *fhook = OSAL_NULL;
    osal_u64 last_active_time;
    fhook = hmac_get_feature_fhook(HMAC_FHOOK_BTCOEX_REPORT_LINKLOSS_INFO);
    if (fhook != OSAL_NULL) {
        ((hmac_btcoex_report_linkloss_info_cb)fhook)(hmac_sta);
    }

    last_active_time = osal_makeu64(hmac_sta->linkloss_info.last_active_time_l32,
        hmac_sta->linkloss_info.last_active_time_h32);
    oam_warning_log4(0, OAM_SF_ANY,
        "vap_id[%d] dbac_linkloss_info: dbac_link_loss_cnt =%d, dbac_linkloss_threshold = %d, dbac_runing = %d",
        hmac_sta->vap_id, hmac_sta->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_DBAC].link_loss,
        hmac_sta->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_DBAC].linkloss_threshold,
        hmac_device->dbac_running);

    oam_warning_log3(0, OAM_SF_ANY,
        "vap_id[%d] normal_linkloss_info: normal_link_loss_cnt =%d, normal_linkloss_threshold = %d",
        hmac_sta->vap_id,
        hmac_sta->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_NORMAL].link_loss,
        hmac_sta->linkloss_info.linkloss_info[WLAN_LINKLOSS_MODE_NORMAL].linkloss_threshold);
    oam_warning_log3(0, OAM_SF_ANY,
        "vap_id[%d] linkloss: last_active_time =%llu, current_time = %llu",
        hmac_sta->vap_id, last_active_time, osal_get_time_stamp_ts());
}

osal_s32 hmac_dft_get_vap_stat(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 stat_clr = *msg->data;

    if (stat_clr == 1) {
        memset_s((osal_u8 *)&hmac_vap->query_stats, sizeof(hmac_vap_query_stats_stru),
            0, sizeof(hmac_vap_query_stats_stru));
    } else {
        if (memcpy_s(msg->rsp, msg->rsp_buf_len, (osal_u8 *)&hmac_vap->query_stats, msg->rsp_buf_len) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{hmac_dft_get_vap_stat::memcpy_s error}");
        }
        msg->rsp_len = msg->rsp_buf_len;
    }
    return OAL_SUCC;
}

osal_s32 hmac_config_aggre_info_enable(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    osal_u8 aggre_info_enable = *(msg->data);
    unref_param(hmac_vap);

    if (aggre_info_enable > 1) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_config_aggre_info_enable::aggre_info_enable[%u] invalid.}", aggre_info_enable);
        return OAL_FAIL;
    }

    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_VAP_AGGRE_ENABLE, msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{hmac_config_aggre_info_enable::send msg fail ret[%d].}", ret);
    }
    return ret;
}

/*****************************************************************************
 函 数 名: hmac_get_vap_stat_aggr_info
 功能描述  : 获取最近20个发包的聚合度
*****************************************************************************/
osal_s32 hmac_query_tx_vap_aggre_info(osal_u8 vap_id)
{
    osal_s32 ret, i;
    wlan_tx_aggregation_stats_stru tx_aggre_info = {0};
    frw_msg msg_to_device = {0};

    frw_msg_init(OSAL_NULL, 0, (osal_u8 *)&tx_aggre_info, sizeof(tx_aggre_info), &msg_to_device);
    ret = frw_send_msg_to_device(vap_id, WLAN_MSG_H2D_C_CFG_TX_VAP_AGGRE_INFO, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_get_tx_vap_aggre_info:: send msg to device fail=%d!", ret);
        return OAL_FAIL;
    }

    wifi_printf("====================vap aggregation info begin====================\r\n");
    wifi_printf("Tx:ampdu|amsdu|========\r\n");
    for (i = 0; i < DFT_VAP_AGGRE_REPORT_NUM; i++) {
        wifi_printf("%7u|%5u|\r\n", tx_aggre_info.mpdu_num[i], tx_aggre_info.msdu_num[i]);
    }
    wifi_printf("====================vap aggregation info end====================\r\n");
    return OAL_SUCC;
}

osal_s32 hmac_config_start_sniffer_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 channel;
    osal_u32 ret;
    mac_scan_req_stru scan_req_param = {0};

    unref_param(msg);

    channel = hmac_vap->channel.chan_number;
    hmac_set_scan_param(hmac_vap, channel, &scan_req_param);

    ret = hmac_scan_proc_scan_req_event_etc(hmac_vap, &scan_req_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "hmac_config_start_sniffer_info:hmac_scan_proc_scan_req_event_etc err [%d]", ret);
        return (osal_s32)ret;
    }
    return OAL_SUCC;
}

osal_s32 hmac_config_get_sniffer_info(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_cfg_set_scan_param scan_param = {0};
    errno_t ret_err;
    hmac_device_stru *hmac_device = OSAL_NULL;
    wlan_scan_chan_stats_stru chan_stats;

    hmac_device = hmac_res_get_mac_dev_etc(0);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_config_get_sniffer_info::hmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    chan_stats = hmac_device->scan_mgmt.scan_record_mgmt.chan_results[0];

    ret_err = memcpy_s(msg->rsp, sizeof(chan_stats), (osal_u8 *)&chan_stats, sizeof(chan_stats));
    if (ret_err != EOK) {
        oam_warning_log0(0, OAM_SF_DFR, "{hmac_config_get_sniffer_info::memset_s failed.}");
        return OAL_FAIL;
    }
    msg->rsp_len = (osal_u16)sizeof(chan_stats);

    scan_param.type = SCAN_PARAM_CHANNEL_LIST;
    scan_param.value = 0;
    hmac_config_set_channel_list(hmac_vap, &scan_param);
    return OAL_SUCC;
}

/* 相应的MAC统计寄存器由高电平>>低电平会对寄存器的计数进行清空 */
static osal_void hmac_dft_hw_stat_clear(osal_void)
{
    /* 软件写寄存器未生效 */
    hal_hw_stat_clear();
}
/* 配置使能硬件收发包统计 */
osal_s32 hmac_config_hw_stat_enable(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_u8 hw_stat_enable;
    frw_msg msg_to_device = {0};
    osal_s32 ret;
    unref_param(hmac_vap);

    if (msg == OAL_PTR_NULL || msg->data == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_config_hw_stat_enable:msg is null}");
        return OAL_FAIL;
    }
    hw_stat_enable = *(msg->data);

    if (hw_stat_enable > 1) {
        oam_error_log1(0, OAM_SF_CFG,
            "{hmac_dft_config_hw_stat_enable::hw_stat_enable[%u] invalid.}", hw_stat_enable);
        return OAL_FAIL;
    }
    // 关闭开关并且之前打开过，则释放内存
    if (hw_stat_enable == 0) {
        if (g_hw_statis_cnt == OSAL_NULL) {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_dft_config_hw_stat_enable::before not malloc memory.}");
        } else {
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_dft_config_hw_stat_enable::free the memory.}");
            oal_mem_free(g_hw_statis_cnt, OAL_TRUE);
            g_hw_statis_cnt = OSAL_NULL;
        }
    } else {
        if (g_hw_statis_cnt == OSAL_NULL) {
            g_hw_statis_cnt = (dft_hw_statis_cnt_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
                sizeof(dft_hw_statis_cnt_stru), OAL_TRUE);
            if (g_hw_statis_cnt == OAL_PTR_NULL) {
                oam_warning_log0(0, OAM_SF_CFG, "{hmac_dft_config_hw_stat_enable::g_hw_statis_cnt malloc memory fail}");
                return OAL_FAIL;
            }
            oam_warning_log0(0, OAM_SF_CFG, "{hmac_dft_config_hw_stat_enable::malloc the memory.}");
        }

        frw_msg_init(&hw_stat_enable, sizeof(osal_u8), OAL_PTR_NULL, 0, &msg_to_device);
        ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ENABLE_HW_STAT, &msg_to_device, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_hw_stat_enable:: send msg to device fail=%d!", ret);
            return OAL_FAIL;
        }

        /* 重复再次使能将会清空软件中统计的收发包数据 */
        (osal_void)memset_s((osal_u8 *)g_hw_statis_cnt, sizeof(dft_hw_statis_cnt_stru), 0,
            sizeof(dft_hw_statis_cnt_stru));
        /* 清空mac层统计数据 */
        hmac_dft_hw_stat_clear();
    }
    return OAL_SUCC;
}

osal_s32 hmac_get_hw_stat(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    frw_msg msg_to_device = {0};
    osal_u8 param = 0;
    dft_device_statis_cnt_stru device_hw_stat = {0};
    osal_s32 ret;

    unref_param(hmac_vap);
    if (msg == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG,  "{hmac_get_hw_stat::msg is null.}");
        return OAL_FAIL;
    }
    if (g_hw_statis_cnt == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_CFG,  "{hmac_get_hw_stat::g_hw_statis_cnt is null, please enable hw stat}");
        return OAL_FAIL;
    }
    /* 读取MAC收发包统计数据 */
    hmac_dft_get_hw_cnt(g_hw_statis_cnt);

    frw_msg_init(&param, sizeof(osal_u8), (osal_u8 *)&device_hw_stat,
        sizeof(dft_device_statis_cnt_stru), &msg_to_device);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_GET_HW_STAT, &msg_to_device, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_get_vap_stat_new:: send msg to device fail=%d!", ret);
        return OAL_FAIL;
    }
    g_hw_statis_cnt->rx_from_hw_mpdu_cnt = device_hw_stat.rx_from_hw_mpdu_cnt;
    g_hw_statis_cnt->tx_from_dmac_mpdu_cnt = device_hw_stat.tx_from_dmac_mpdu_cnt;

    /* 打印出hw的收发包状态 */
    hmac_hw_stat_display();

    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
