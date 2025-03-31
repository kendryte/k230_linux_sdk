/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Defination of private operation related to chip.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_chip.h"
#include "wlan_spec.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hal_rf.h"
#include "hal_device.h"
#include "frw_osal.h"
#include "hal_mac.h"
#include "hal_soc.h"
#include "hal_device_fsm.h"
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "hal_coex_reg.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_CHIP_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
hal_chip_stru g_hal_chip;

osal_u8 g_rssi_switch = 0x3;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hal_device_dft_stat_init
 功能描述  : hal device统计维测初始化
*****************************************************************************/
osal_void hal_device_dft_stat_init(hal_to_dmac_device_stru *hal_device_base)
{
    hal_alg_stat_info_stru *hal_alg_stat = &hal_device_base->hal_alg_stat;

    hal_alg_stat->adj_intf_state = HAL_ALG_INTF_DET_ADJINTF_NO;
    hal_alg_stat->co_intf_state = OSAL_FALSE;
    hal_alg_stat->alg_distance_stat = HAL_ALG_USER_DISTANCE_NORMAL;
}

osal_void hal_device_rssi_init(hal_to_dmac_device_stru *hal_device_base)
{
#ifdef _PRE_WLAN_FEATURE_ANT_SWITCH
    /* RSSI天线切换根据定制化开启 */
    hal_device_base->rssi.rx_rssi.ant_rssi_sw = g_rssi_switch;

    hal_device_base->rssi.rx_rssi.ant0_rssi_smth = OAL_RSSI_INIT_MARKER;
    hal_device_base->rssi.rx_rssi.ant1_rssi_smth = OAL_RSSI_INIT_MARKER;
    hal_device_base->rssi.rx_rssi.rssi_high_cnt_th = HAL_ANT_SWITCH_RSSI_HIGH_CNT;
    hal_device_base->rssi.rx_rssi.rssi_low_cnt_th = HAL_ANT_SWITCH_RSSI_LOW_CNT;
    hal_device_base->rssi.tbtt_cnt_th = HAL_ANT_SWITCH_RSSI_TBTT_CNT_TH;
    hal_device_base->rssi.mimo_tbtt_open_th = HAL_ANT_SWITCH_RSSI_MIMO_TBTT_OPEN_TH;
    hal_device_base->rssi.mimo_tbtt_close_th = HAL_ANT_SWITCH_RSSI_MIMO_TBTT_CLOSE_TH;
#ifdef _PRE_WLAN_FEATURE_M2S
    hal_rx_ant_rssi_mgmt_stru *hal_rx_ant_rssi_mgmt = OSAL_NULL;
    hal_rx_ant_rssi_mgmt = get_hal_device_rx_ant_rssi_mgmt(hal_device_base);

    hal_rx_ant_rssi_mgmt->ant_rssi_sw = OSAL_TRUE;
    hal_rx_ant_rssi_mgmt->ant0_rssi_smth = OAL_RSSI_INIT_MARKER;
    hal_rx_ant_rssi_mgmt->ant1_rssi_smth = OAL_RSSI_INIT_MARKER;
    hal_rx_ant_rssi_mgmt->rssi_th = HAL_ANT_SWITCH_RSSI_MGMT_STRONG_TH;
    hal_rx_ant_rssi_mgmt->log_print = OSAL_TRUE;
#endif
#endif
    unref_param(hal_device_base);
}

/*****************************************************************************
 功能描述  : hal device alg 私有结构体初始化
*****************************************************************************/
osal_u32 hal_device_alg_init(hal_to_dmac_device_stru *hal_device_base)
{
    hal_alg_device_stru *alg_dev_stru = OSAL_NULL; /* device级别算法的私有数据结构 */

    alg_dev_stru = (hal_alg_device_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hal_alg_device_stru), OAL_TRUE);
    if (alg_dev_stru == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hal_device_alg_init: alloc alg device failed.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    (osal_void)memset_s(alg_dev_stru, sizeof(hal_alg_device_stru), 0, sizeof(hal_alg_device_stru));

    /* 挂接算法主结构体 */
    hal_device_base->alg_priv = alg_dev_stru;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : hal device alg 私有结构体退出
*****************************************************************************/
osal_void hal_device_alg_exit(hal_to_dmac_device_stru *hal_device_base)
{
    hal_alg_device_stru *alg_dev_stru = OSAL_NULL; /* hal device级别算法的私有数据结构 */

    alg_dev_stru = (hal_alg_device_stru *)(hal_device_base->alg_priv);
    if (alg_dev_stru == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_device_alg_exit::OSAL_NULL == p_alg_stru}");
        return;
    }

    oal_mem_free(alg_dev_stru, OAL_TRUE);

    (hal_device_base->alg_priv) = OSAL_NULL;
}

/*****************************************************************************
 功能描述  : hal device 定制配置初始化
*****************************************************************************/
osal_void hal_device_custom_cfg_cap_init(hal_to_dmac_device_stru *hal_device_base)
{
    hal_device_base->rx_normal_dscr_cnt = WLAN_MEM_NORMAL_RX_MAX_CNT;
    hal_device_base->rx_small_dscr_cnt = WLAN_MEM_SMALL_RX_MAX_CNT;
    /* 初始化RSSI值 */
    hal_device_base->always_rx_rssi = OAL_RSSI_INIT_MARKER;
    hal_device_base->rx_last_rssi = OAL_RSSI_INIT_VALUE;

    hal_device_base->cfg_cap_info = hal_device_get_cfg_custom_addr();
}

/*****************************************************************************
 功能描述  : 根据输入的chip id，device id，初始化device level hal的变量，以
             及device level hal函数指针
*****************************************************************************/
osal_u32 hal_device_init(hal_chip_stru *hal_chip)
{
    hal_to_dmac_device_stru *hal_device_base = &hal_chip->device.hal_device_base;

    /* MAC 软复位 */
    hh503_reset_phy_machw(HAL_RESET_HW_TYPE_MAC, HAL_RESET_MAC_LOGIC, OSAL_FALSE, OSAL_FALSE);
    /* 预留hal_device_init接口 */
    if (hal_device_init_resv(hal_device_base) != OAL_SUCC) {
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_DFT_STAT
    hal_device_base->reg_info_flag = 0x1F; // 打印寄存器开关，默认全部打印
#endif

    hal_device_rssi_init(hal_device_base);
    hal_device_custom_cfg_cap_init(hal_device_base);

    /* hal device 私有算法结构体初始化 */
    if (hal_device_alg_init(hal_device_base) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hal_tx_init_dscr_queue(hal_device_base);
    /* hal device 统计信息初始化 */
    hal_device_dft_stat_init(hal_device_base);

    /* 初始化device下设备的vap ID为无效索引             */
    hal_device_base->p2p_hal_vap_idx = 0xff;
    hal_device_base->sta_hal_vap_idx = 0xff;
    hal_device_base->ap_hal_vap_idx = 0xff;
    hal_device_base->ax_vap_cnt = 0;

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 使能硬件聚合 */ /* FPGA暂时关闭聚合硬化 */
    hal_device_base->ampdu_tx_hw = OSAL_FALSE;
    hal_device_base->ampdu_partial_resnd = OSAL_FALSE;
    hal_set_hw_en_reg_cfg(hal_device_base->ampdu_tx_hw);
#endif

    /* hal device 状态机初始化 */
    hal_device_fsm_attach(hal_device_base);

    /* 单通道收beacon开关初始化 */
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
#ifdef BOARD_ASIC_WIFI
    hal_device_base->srb_switch = OSAL_TRUE;
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    hal_device_base->mac_parse_tim = OSAL_TRUE;
#endif

    /* 初始化hal tx power软件资源 */
    hal_pow_sw_initialize_tx_power(hal_device_base);

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_device_btcoex_sw_init(hal_device_base);
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 根据定制化刷新驱动mss功能开关 */
    get_hal_device_m2s_mss_on(hal_device_base) = ((hal_device_get_m2s_mask() & HAL_M2S_MODE_BITMAP_MSS) != 0) ?
        OSAL_TRUE : OSAL_FALSE;
#endif

    hal_device_handle_event(hal_device_base, HAL_DEVICE_EVENT_SYSTEM_INIT, 0, OAL_PTR_NULL);

    hal_device_init_ext(hal_chip); /* 补充hal_device的初始化 */

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
 功能描述  : 根据定制化刷新2g/5g 最大rf chain能力
*****************************************************************************/
osal_void hal_chip_init_rf_custom_cap(hal_chip_stru *hal_chip)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_chip->rf_custom_mgr.support_rf_chain_2g = (g_rf_customize.chn_radio_cap & WLAN_RF_CHAIN_DOUBLE);
    if (hal_chip->rf_custom_mgr.support_rf_chain_2g == WLAN_RF_CHAIN_DOUBLE) {
        hal_chip->rf_custom_mgr.nss_num_2g = WLAN_DOUBLE_NSS;
    } else {
        hal_chip->rf_custom_mgr.nss_num_2g = WLAN_SINGLE_NSS;
    }
    /* 5G信息右移2bit */
    hal_chip->rf_custom_mgr.support_rf_chain_5g = ((g_rf_customize.chn_radio_cap >> 2) & WLAN_RF_CHAIN_DOUBLE);
    if (hal_chip->rf_custom_mgr.support_rf_chain_5g == WLAN_RF_CHAIN_DOUBLE) {
        hal_chip->rf_custom_mgr.nss_num_5g = WLAN_DOUBLE_NSS;
    } else {
        hal_chip->rf_custom_mgr.nss_num_5g = WLAN_SINGLE_NSS;
    }

    oam_warning_log4(0, OAM_SF_ANY,
        "{hal_chip_init_rf_custom_cap::rf custom 2G chain[%d] nss[%d], 5g chain[%d] nss[%d].}",
        hal_chip->rf_custom_mgr.support_rf_chain_2g, hal_chip->rf_custom_mgr.nss_num_2g,
        hal_chip->rf_custom_mgr.support_rf_chain_5g, hal_chip->rf_custom_mgr.nss_num_5g);
#endif
}
#endif

/*****************************************************************************
 函 数 名  : hal_chip_init
 功能描述  : 获得芯片版本，根据芯片版本赋值device个数。调用device初始化
*****************************************************************************/
osal_u32 hal_chip_init(osal_void)
{
    /* 初始化chip rom信息 */
    (osal_void)memset_s(&g_hal_chip, sizeof(hal_chip_stru), 0, sizeof(hal_chip_stru));

    if (hal_device_init(&g_hal_chip) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hal_initialize_machw_common();
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 卸载device的操作函数(调用之前，已从chip上删除)
*****************************************************************************/
osal_void hal_device_exit(hal_device_stru *device)
{
    hal_to_dmac_device_stru *hal_device_base = &(device->hal_device_base);

    /* hal device算法框架退出 */
    hal_device_alg_exit(hal_device_base);
    /* 中断处理退出 */

    /* hal device状态机去初始化 */
    hal_device_fsm_detach(hal_device_base);
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_btcoex_sw_preempt_exit(hal_device_base);
#endif
    /* 并向平台去注册 */
    hal_pm_servid_unregister();
}

/*****************************************************************************
 功能描述  : 删除hal chip对应结构
*****************************************************************************/
osal_void hal_chip_exit(osal_void)
{
    hal_device_exit(&g_hal_chip.device);
    /* 中断处理退出 */
}

hal_chip_stru *hal_get_chip_stru(osal_void)
{
    return &g_hal_chip;
}

/*****************************************************************************
 功能描述  : 根据chipid，和device id获得device对象
*****************************************************************************/
hal_to_dmac_device_stru *hal_chip_get_hal_device(osal_void)
{
    return &(g_hal_chip.device.hal_device_base);
}

/*****************************************************************************
 功能描述  : hal_device_init函数的预留函数
*****************************************************************************/
osal_u32 hal_device_init_resv(hal_to_dmac_device_stru *hal_device_base)
{
    unref_param(hal_device_base);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : hal_device_init函数的补充函数
*****************************************************************************/
osal_void hal_device_init_ext(hal_chip_stru *hal_chip)
{
    unref_param(hal_chip);
#ifdef _PRE_WLAN_FEATURE_WUR_TX
    /* WUR使用的vap信息 */
    hal_chip->device.wur_ap_vap_id = 2; /* 2: 默认soft-ap使用vap2 */
    hal_chip->device.wur_ac = 4; /* 4: 默认使用高优先级队列 */
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
