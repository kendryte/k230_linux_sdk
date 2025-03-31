/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file of HAL external public interfaces .
 * Create: 2020-7-3
 */

#ifndef __HAL_EXT_IF_H__
#define __HAL_EXT_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "wlan_types_common.h"
#include "hal_common_ops.h"
#include "dmac_common_inc_rom.h"
#include "hal_ext_anti_intf.h"
#include "hal_ext_txbf.h"
#include "hal_ext_tpc.h"

#include "wlan_msg.h"
#include "hal_ext_if_device.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "hal_p2p.h"
#endif
#include "hal_reg_opt.h"
#ifdef _PRE_WLAN_FEATURE_WS63
#include "hal_ext_if_rom.h"
#endif
#include "hal_power_save.h"
#include "hal_coex_reg.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_EXT_IF_ROM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define hal_btcoex_check_sw_preempt_delba_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->btcoex_sw_preempt.sw_preempt_mode)) & BIT1) != 0) ? OSAL_TRUE : OSAL_FALSE)
/* 获取当前hal devcie的ps业务是否打开标记 */
#define get_hal_device_btcoex_m2s_mode_bitmap(_pst_handler) \
    (*(osal_u8*)&((_pst_handler)->device_btcoex_mgr.m2s_mode_bitmap)) /* 获取当前hal devcie的btcoex m2s业务bitmap */
#define get_hal_device_btcoex_s2m_mode_bitmap(_pst_handler) \
    (*(osal_u8*)&((_pst_handler)->device_btcoex_mgr.s2m_mode_bitmap)) /* 获取当前hal devcie的btcoex s2m业务bitmap */
#define get_hal_device_btcoex_s2m_wait_bitmap(_pst_handler) \
    (*(osal_u8*)&((_pst_handler)->device_btcoex_mgr.s2m_wait_bitmap))

/* HAL模块和DMAC模块共用的WLAN RX结构体 */
typedef struct {
    oal_netbuf_stru        *netbuf;         /* netbuf链表一个元素 */
    osal_u16              netbuf_num;      /* netbuf链表的个数 */
    osal_u8               auc_resv[2];        /* 2字节对齐 */
} hal_cali_hal2hmac_event_stru;

typedef struct {
    /* CCA能量门限 */
    osal_s8 c_ed_cca_20th_high;    /* CCA 主辅20M带宽 */
    osal_s8 c_ed_cca_40th_high;    /* CCA 主40M带宽 */

    /* 主20M CCA协议门限 */
    osal_s8 c_sd_cca_20th_dsss;    /* CCA DSSS检测门限 */
    osal_s8 c_sd_cca_20th_ofdm;    /* CCA OFDM检测门限,主20M带宽 */
} hal_alg_cca_th_stru;

#ifndef _PRE_WLAN_FEATURE_WS63  //  与device侧同名的宏和函数用63宏控住
#define HAL_PM_DEF_TBTT_OFFSET              10000        /* tbtt提前量默认值单位us */
#ifdef BOARD_FPGA_WIFI
#define HAL_PM_DEF_EXT_TSF_OFFSET           30000       /* 外置tsf唤醒提前量单位us */
#else
#define HAL_PM_DEF_EXT_TSF_OFFSET           12500       /* 外置tsf唤醒提前量单位us */
#endif
#endif

/* 获取当前状态机的上一个状态 */
#define get_hal_m2s_mode_tpye(_pst_handler)         (*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))
osal_s32 hal_hmac_main_init(osal_void);
osal_void  hal_hmac_main_exit(osal_void);
#ifndef _PRE_WLAN_FEATURE_WS63  //  与device侧同名的宏和函数用63宏控住
#define HAL_ANT_SWITCH_RSSI_MGMT_ENABLE             BIT0
#define HAL_ANT_SWITCH_RSSI_DATA_ENABLE             BIT1
/* 低功耗相关宏定义 */

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define hal_cca_opt_get_default_ed_20th(_band, _cust) \
    (((_band) == WLAN_BAND_2G) ? \
     ((_cust)->delta_cca_ed_high_20th_2g + HAL_CCA_OPT_ED_HIGH_20TH_DEF) : \
     ((_cust)->delta_cca_ed_high_20th_5g + HAL_CCA_OPT_ED_HIGH_20TH_DEF))
#define hal_cca_opt_get_default_ed_40th(_band, _cust) \
    (((_band) == WLAN_BAND_2G) ? \
     ((_cust)->delta_cca_ed_high_40th_2g + HAL_CCA_OPT_ED_HIGH_40TH_DEF) : \
     ((_cust)->delta_cca_ed_high_40th_5g + HAL_CCA_OPT_ED_HIGH_40TH_DEF))
#endif

/* 获取work的子状态 */
#define get_hal_dev_current_scan_idx(_pst_hal_device)   ((_pst_hal_device)->hal_scan_params.start_chan_idx + \
    (_pst_hal_device)->hal_scan_params.scan_chan_idx)
#define INVALID_SAR_PWR_LIMIT                 (0XFF)   /* 当前SAR功率值 */
#define HAL_SAR_PWR_LIMIT_THRESHOLD            (15)    /* SAR功率阈值，低于阈值表示正在降SAR，高于阈值表示降SAR结束 */
#define hal_device_state_is_busy(_pst_hal_device)   \
    ((((_pst_hal_device)->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_WORK_STATE) \
      || ((_pst_hal_device)->hal_dev_fsm.oal_fsm.cur_state == HAL_DEVICE_SCAN_STATE)) ? OSAL_TRUE : OSAL_FALSE)
#define hal_device_prev_state_is_busy(_pst_hal_device)   \
    ((((_pst_hal_device)->hal_dev_fsm.oal_fsm.prev_state == HAL_DEVICE_WORK_STATE) \
      || ((_pst_hal_device)->hal_dev_fsm.oal_fsm.prev_state == HAL_DEVICE_SCAN_STATE)) ? OSAL_TRUE : OSAL_FALSE)
#ifdef _PRE_WLAN_FEATURE_M2S
#define get_hal_device_m2s_mgr(_pst_handler) \
    (&(((hal_to_dmac_device_stru *)(_pst_handler))->device_m2s_mgr))
#define get_hal_device_m2s_switch_prot(_pst_handler)       (get_hal_device_m2s_mgr(_pst_handler)->m2s_switch_protect)
#define get_hal_device_m2s_del_swi_miso_hold(_pst_handler) (get_hal_device_m2s_mgr(_pst_handler)->delay_swi_miso_hold)
#define get_hal_device_m2s_mss_on(_pst_handler)            (get_hal_device_m2s_mgr(_pst_handler)->mss_on)
#define get_hal_device_m2s_rssi_mgmt_single_txchain(_pst_handler) \
    (get_hal_device_m2s_mgr(_pst_handler)->rssi_mgmt_single_txchain)
#endif

#define hal_m2s_check_fast_scan_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))& WLAN_M2S_TRIGGER_MODE_FAST_SCAN) != 0) ?    \
        OSAL_TRUE : OSAL_FALSE)
#define hal_m2s_check_rssi_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))& WLAN_M2S_TRIGGER_MODE_RSSI) != 0) ? OSAL_TRUE : OSAL_FALSE)
#define hal_m2s_check_btcoex_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))& WLAN_M2S_TRIGGER_MODE_BTCOEX) != 0) ?    \
        OSAL_TRUE : OSAL_FALSE)
#define hal_m2s_check_command_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))& WLAN_M2S_TRIGGER_MODE_COMMAND) != 0) ?    \
        OSAL_TRUE : OSAL_FALSE)
#define hal_m2s_check_test_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))& WLAN_M2S_TRIGGER_MODE_TEST) != 0) ? OSAL_TRUE : OSAL_FALSE)
#define hal_m2s_check_custom_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))& WLAN_M2S_TRIGGER_MODE_CUSTOM) != 0) ?    \
        OSAL_TRUE : OSAL_FALSE)
#define hal_m2s_check_spec_on(_pst_handler)   \
    ((((*(osal_u8*)&((_pst_handler)->hal_m2s_fsm.m2s_mode))& WLAN_M2S_TRIGGER_MODE_SPEC) != 0) ? OSAL_TRUE : OSAL_FALSE)
#ifdef _PRE_WLAN_FEATURE_M2S
#define get_hal_device_rx_ant_rssi_mgmt(_pst_handler) \
    (&(((hal_to_dmac_device_stru *)(_pst_handler))->hal_rx_ant_rssi_mgmt))
#endif
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
 /* 获取动态校准是否未使能 */
#define get_hal_dyn_cali_disable(_pst_handler) \
    ((_pst_handler)->dyn_cali_val.aus_cali_en_interval[(_pst_handler)->wifi_channel_status.band] == 0)
#endif

/*****************************************************************************
  8 UNION定义
*****************************************************************************/
typedef struct {
    osal_void      (* func)(hal_to_dmac_device_stru  *hal_device, osal_u32 para);
    osal_u32    param;
} hal_error_proc_stru;

#ifdef _PRE_WLAN_FEATURE_DAQ
typedef enum {
    HAL_MAC_DIAG_TEST_INIT = 0,
    HAL_MAC_DIAG_TEST_MATCH,
    HAL_MAC_DIAG_TEST_QUERY = 11,
    HAL_MAC_DIAG_TEST_STOP,
} hal_mac_diag_test_type_enum;
typedef enum {
    HAL_MAC_DIAG_DUG = 0,
    HAL_MAC_DIAG_TEST1,
    HAL_MAC_DIAG_TEST4 = 4,
    HAL_MAC_DIAG_TEST8 = 8,
} hal_mac_diag_test_item_enum;
typedef enum {
    HAL_MAC_TX_DIAG_MATCH_RA_ADDR = 0,
    HAL_MAC_TX_DIAG_MATCH_TA_ADDR,
    HAL_MAC_TX_DIAG_MATCH_FRM_TYPE,
    HAL_MAC_RX_DIAG_MATCH_RA_ADDR,
    HAL_MAC_RX_DIAG_MATCH_TA_ADDR,
    HAL_MAC_RX_DIAG_MATCH_FRM_TYPE,
    HAL_MAC_TX_RX_DIAG_MATCH_NUM,
} hal_mac_diag_test_match_enum;
typedef enum {
    HAL_MAC_DIAG4_NODE_DBAC = 0,
    HAL_MAC_DIAG4_NODE_FSM,
    HAL_MAC_DIAG4_NODE_TX_AMPDU0,
    HAL_MAC_DIAG4_NODE_TSF_VAP3 = 31,
} hal_mac_diag4_node_enum;
typedef enum {
    HAL_MAC_DIAG8_NODE_H = 0,
    HAL_MAC_DIAG8_NODE_HV,
    HAL_MAC_DIAG8_NODE_SNR,
    HAL_MAC_DIAG8_NODE_PHI_PSI,
} hal_mac_diag8_node_enum;
typedef enum {
    HAL_MAC_DIAG_SEL_PKT_RAM_2KB = 0,
    HAL_MAC_DIAG_SEL_PKT_RAM_4KB,
    HAL_MAC_DIAG_SEL_PKT_RAM_6KB,
    HAL_MAC_DIAG_SEL_PKT_RAM_8KB,
    HAL_MAC_DIAG_SEL_PKT_RAM_10KB,
    HAL_MAC_DIAG_SEL_PKT_RAM_12KB,
    HAL_MAC_DIAG_SEL_PKT_RAM_14KB,
    HAL_MAC_DIAG_SEL_PKT_RAM_16KB,
} hal_mac_diag_sel_pkt_ram_size_enum;
#endif
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_u8  hal_get_5g_cur_rf_chn_enable(osal_u8 cur_cali_chn);
#endif // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
hal_cipher_protocol_type_enum_uint8 hal_cipher_suite_to_ctype(wlan_ciper_protocol_type_enum_uint8 cipher_suite);
wlan_ciper_protocol_type_enum_uint8 hal_ctype_to_cipher_suite(hal_cipher_protocol_type_enum_uint8 cipher_type);

/*****************************************************************************
  10.1 芯片级别函数
*****************************************************************************/
hal_to_dmac_device_stru *hal_chip_get_hal_device(osal_void);
osal_void hal_device_inc_assoc_user_nums(hal_to_dmac_device_stru  *hal_device);
hal_cfg_cap_info_stru *hal_device_get_cfg_custom_addr(osal_void);
osal_void hal_device_dec_assoc_user_nums(hal_to_dmac_device_stru  *hal_device);
osal_u8  hal_device_find_all_up_vap(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id, osal_u8 lenth);
osal_void hal_device_handle_event(hal_to_dmac_device_stru *hal_device, osal_u16 type, osal_u16 datalen, osal_u8 *data);
osal_u32  hal_device_find_one_up_vap(hal_to_dmac_device_stru *hal_device, osal_u8 *mac_vap_id);
osal_u32  hal_device_find_another_up_vap(hal_to_dmac_device_stru *hal_device,
    osal_u8 vap_id_self, osal_u8 *mac_vap_id);
osal_u32 hal_device_find_one_up_hal_vap(hal_to_dmac_device_stru *hal_device,  osal_u8 *vap_id);
osal_void  hal_device_get_fix_rate_pow_code_idx(const hal_tx_ctrl_desc_rate_stru *rate_param, osal_u8 *rate_pow_idx);
osal_void hal_device_set_pow_al_tx(const hal_to_dmac_device_stru *hal_device,
    osal_u8 band, hal_tx_txop_alg_stru *txop_alg);
osal_void hal_device_p2p_adjust_upc(hal_to_dmac_device_stru *hal_device, osal_u8 cur_ch_num,
    wlan_channel_band_enum_uint8 freq_band, wlan_channel_bandwidth_enum_uint8 bandwidth);

oal_bool_enum_uint8 hal_get_hal_device_is_work(hal_to_dmac_device_stru *hal_device);
osal_u8 hal_pow_get_legacy_rate(hal_tx_ctrl_desc_rate_stru *tx_dscr);
#define get_hal_dev_current_scan_idx(_pst_hal_device)   ((_pst_hal_device)->hal_scan_params.start_chan_idx + \
    (_pst_hal_device)->hal_scan_params.scan_chan_idx)
#ifdef _PRE_WLAN_FEATURE_DAQ
osal_void hal_set_mac_diag_param(osal_u8 vap_id, mac_cfg_mct_set_diag_stru *event_set_diag,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device);
osal_void hal_set_mac_diag_mode(mac_cfg_mct_set_diag_stru *event_set_diag,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device);
osal_void hal_set_mac_diag_test_query(hal_to_dmac_device_mac_test_stru *hal_mac_test_device);
#endif
/*****************************************************************************
  10.3.2 DEVICE级别   第一个入参类型为 hal_to_dmac_device_stru
*****************************************************************************/
static INLINE__ osal_void hal_rx_init_dscr_queue(hal_to_dmac_device_stru *device, osal_u8 set_hw)
{
    hal_public_hook_func(_rx_init_dscr_queue)(device, set_hw);
}

static INLINE__ osal_void hal_rx_destroy_dscr_queue(hal_to_dmac_device_stru *device)
{
    hal_public_hook_func(_rx_destroy_dscr_queue)(device);
}

/* 填充描述符的基本信息，包括帧长度、mac长度、安全信息 */
static INLINE__ osal_void hal_tx_fill_basic_ctrl_dscr(const hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *tx_dscr, hal_tx_mpdu_stru *mpdu)
{
    hal_public_hook_func(_tx_fill_basic_ctrl_dscr)(hal_device, tx_dscr, mpdu);
}

/* 将两个描述符连起来 */
static INLINE__ osal_void  hal_tx_ctrl_dscr_link(const hal_to_dmac_device_stru * hal_device,
    hal_tx_dscr_stru *tx_dscr_prev, hal_tx_dscr_stru *tx_dscr)
{
    unref_param(hal_device);
    hal_public_hook_func(_tx_ctrl_dscr_link)(tx_dscr_prev, tx_dscr);
}

/* 从描述符的next获取下一个描述符地址 */
static INLINE__ osal_void  hal_get_tx_dscr_next(const hal_to_dmac_device_stru * hal_device, hal_tx_dscr_stru *tx_dscr,
    hal_tx_dscr_stru **tx_dscr_next)
{
    unref_param(hal_device);
    hal_public_hook_func(_get_tx_dscr_next)(tx_dscr, tx_dscr_next);
}

static INLINE__ osal_void  hal_tx_ctrl_dscr_unlink(const hal_to_dmac_device_stru * hal_device,
    hal_tx_dscr_stru *tx_dscr)
{
    unref_param(hal_device);
    hal_public_hook_func(_tx_ctrl_dscr_unlink)(tx_dscr);
}

static INLINE__ osal_void hal_tx_get_vap_id(const hal_to_dmac_device_stru * hal_device,
    const hal_tx_dscr_stru * tx_dscr, osal_u8 *vap_id)
{
    unref_param(hal_device);
    hal_public_hook_func(_tx_get_vap_id)(tx_dscr, vap_id);
}

/*****************************************************************************
 功能描述  : 获取发送队列状态
*****************************************************************************/
static INLINE__ osal_u32 hal_get_tx_q_status(osal_u8 qnum)
{
    return hal_public_hook_func(_get_tx_q_status)(qnum);
}

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
/*****************************************************************************
 函 数 名  : 动态校准配置恢复
*****************************************************************************/
static INLINE__ osal_void hal_rf_init_dyn_cali_reg_conf(hal_to_dmac_device_stru *hal_device)
{
    hal_public_hook_func(_rf_init_dyn_cali_reg_conf)(hal_device);
}

static INLINE__ osal_void  hal_tx_set_pdet_en(const hal_to_dmac_device_stru *hal_device, hal_tx_dscr_stru *tx_dscr,
    oal_bool_enum_uint8 pdet_en_flag)
{
    hal_public_hook_func(_tx_set_pdet_en)(hal_device, tx_dscr, pdet_en_flag);
}

#endif

/*****************************************************************************
 函 数 名  : hal_set_ana_dbb_ch_sel
 功能描述  : 设置天线和phy通道对应关系
*****************************************************************************/
static INLINE__ osal_void  hal_set_ana_dbb_ch_sel(hal_to_dmac_device_stru *hal_device)
{
    hal_public_hook_func(_set_ana_dbb_ch_sel)(hal_device);
}

/*****************************************************************************
 函 数 名  : hal_update_ext_lna_cfg
 功能描述  : 更新EXT LNA
*****************************************************************************/
static INLINE__ osal_void  hal_update_cbb_cfg(hal_to_dmac_device_stru *hal_device)
{
    hal_public_hook_func(_update_cbb_cfg)(hal_device);
}

static INLINE__ osal_void hal_get_hal_vap(const hal_to_dmac_device_stru * hal_device, osal_u8 vap_id,
    hal_to_dmac_vap_stru **hal_vap)
{
    hal_public_hook_func(_get_hal_vap)(hal_device, vap_id, hal_vap);
}

/*****************************************************************************
 功能描述  : 按bit读取32位寄存器的值,寄存器值右移offset后取后bits位
*****************************************************************************/
static INLINE__ osal_u32 hal_reg_read_bit(osal_u32 addr, osal_u8 offset, osal_u8 bits)
{
    osal_u32 value = hal_reg_read(addr);

    return (value >> offset) & (((osal_u32)1 << bits) - 1);
}

/*****************************************************************************
 功能描述  : 写32位寄存器对应bits位的值
*****************************************************************************/
static INLINE__ osal_void hal_reg_write_bits(osal_u32 reg_addr, osal_u8 offset, osal_u8 bits, osal_u32 reg_val)
{
    osal_u32 value = hal_reg_read(reg_addr);
    value &= ~((((osal_u32)1 << bits) - 1) << offset);   /* 先将对应bits位的值清0 */
    value |= ((reg_val & (((osal_u32)1 << bits) - 1)) << offset); /* 将对应bits值改写为配置值 */
    hal_reg_write(reg_addr, value);
}

/*****************************************************************************
 功能描述  : 写16位寄存器对应bits位的值
*****************************************************************************/
static INLINE__ osal_void hal_reg_write16_bits(osal_u32 reg_addr, osal_u8 offset, osal_u8 bits, osal_u32 reg_val)
{
    osal_u16 value = hal_reg_read16(reg_addr);
    value &= ~((((osal_u16)1 << bits) - 1) << offset);   /* 先将对应bits位的值清0 */
    value |= ((reg_val & (((osal_u16)1 << bits) - 1)) << offset); /* 将对应bits值改写为配置值 */
    hal_reg_write16(reg_addr, value);
}

static INLINE__ osal_void hal_reg_write16_or_bits(osal_u32 reg_addr, osal_u32 or_val)
{
    osal_u16 value = hal_reg_read16(reg_addr);
    value |= or_val;
    hal_reg_write16(reg_addr, value);
}

/*****************************************************************************
 功能描述  : 读取表寄存器的值
*****************************************************************************/
static INLINE__ osal_void hal_reg_write_tbl(const hal_reg32_cfg_stru *reg_tbl, osal_u8 tbl_len)
{
    osal_u8 index;
    for (index = 0; index < tbl_len; index++) {
        hal_reg_write(reg_tbl[index].address, reg_tbl[index].value);
    }
}

static INLINE__ osal_void hal_reg_write16_tbl(const hal_reg16_cfg_stru *reg_tbl, osal_u8 tbl_len)
{
    osal_u8 index;
    for (index = 0; index < tbl_len; index++) {
        hal_reg_write16(reg_tbl[index].address, reg_tbl[index].value);
    }
}

/*****************************************************************************
 功能描述  : 写表寄存器的值
*****************************************************************************/
static INLINE__ osal_void hal_reg_read_tbl(hal_reg32_cfg_stru *reg_tbl, osal_u8 tbl_len)
{
    osal_u8 index;
    for (index = 0; index < tbl_len; index++) {
        reg_tbl[index].value = hal_reg_read(reg_tbl[index].address);
    }
}

static INLINE__ osal_void hal_reg_read16_tbl(hal_reg16_cfg_stru *reg_tbl, osal_u8 tbl_len)
{
    osal_u8 index;
    for (index = 0; index < tbl_len; index++) {
        reg_tbl[index].value = hal_reg_read(reg_tbl[index].address);
    }
}

/*****************************************************************************
  hal初始化/退出/复位相关接口
*****************************************************************************/
/*****************************************************************************
 功能描述  : 停止PA和PHY的工作
*****************************************************************************/
static INLINE__ osal_void hal_disable_machw_phy_and_pa(osal_void)
{
    hal_public_hook_func(_disable_machw_phy_and_pa)();
}

/*****************************************************************************
 功能描述  : 停止PA和PHY的工作
*****************************************************************************/
static INLINE__ osal_void hal_enable_machw_phy_and_pa(const hal_to_dmac_device_stru *hal_device)
{
    hal_public_hook_func(_enable_machw_phy_and_pa)(hal_device);
}

/*****************************************************************************
 函 数 名  : hal_recover_machw_phy_and_pa
 功能描述  : 恢复hal_to_dmac_device_stru中记录的pa状态
*****************************************************************************/
static INLINE__ osal_void  hal_recover_machw_phy_and_pa(hal_to_dmac_device_stru *hal_device)
{
    hal_public_hook_func(_recover_machw_phy_and_pa)(hal_device);
}

/*****************************************************************************
  hal MAC hardware初始化接口
*****************************************************************************/
/*****************************************************************************
 功能描述  : MAC hardware初始化接口
*****************************************************************************/
osal_void  hal_initialize_machw(osal_void);

/*****************************************************************************
 功能描述  : MAC hardware 公共区域初始化接口
*****************************************************************************/
static INLINE__ osal_void hal_initialize_machw_common(osal_void)
{
    hal_public_hook_func(_initialize_machw_common)();
}

/*****************************************************************************
 函 数 名  : hal_set_bandwidth_mode
 功能描述  : 设置带宽
*****************************************************************************/
static INLINE__ osal_void  hal_set_bandwidth_mode(hal_to_dmac_device_stru *hal_device,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    hal_public_hook_func(_set_bandwidth_mode)(hal_device, bandwidth);
}

/*****************************************************************************
 功能描述  : 复位mac寄存器保存
*****************************************************************************/
static INLINE__ osal_void hal_reset_save_mac_reg(osal_void)
{
    hal_public_hook_func(_reset_save_mac_reg)();
}

/*****************************************************************************
 功能描述  : 控制硬件在收到RTS时不回CTS
*****************************************************************************/
static INLINE__ osal_void  hal_disable_machw_cts_trans(osal_void)
{
    hal_public_hook_func(_disable_machw_cts_trans)();
}

/*****************************************************************************
 功能描述  : 控制硬件在收到RTS时回CTS
*****************************************************************************/
static INLINE__ osal_void  hal_enable_machw_cts_trans(osal_void)
{
    hal_public_hook_func(_enable_machw_cts_trans)();
}

/*****************************************************************************
 函 数 名  : hal_initialize_phy
*****************************************************************************/
osal_void  hal_initialize_phy(osal_void);

#ifdef _PRE_WLAN_FEATURE_DFS
/*****************************************************************************
 函 数 名  : hal_radar_config_reg
*****************************************************************************/
static INLINE__ osal_void hal_radar_config_reg(hal_dfs_radar_type_enum_uint8 dfs_domain)
{
    hal_public_hook_func(_radar_config_reg)(dfs_domain);
}
static INLINE__ osal_void hal_radar_config_reg_bw(hal_dfs_radar_type_enum_uint8 radar_type,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    hal_public_hook_func(_radar_config_reg_bw)(radar_type, bandwidth);
}
static INLINE__ osal_void hal_radar_enable_chirp_det(oal_bool_enum_uint8 chirp_det)
{
    hal_public_hook_func(_radar_enable_chirp_det)(chirp_det);
}

#endif

static INLINE__ osal_void hal_set_rf_limit_power(wlan_channel_band_enum_uint8 band, osal_u8 power)
{
    hal_public_hook_func(_set_rf_limit_power)(band, power);
}

osal_void  hal_pow_sw_initialize_tx_power(hal_to_dmac_device_stru *hal_device);

static INLINE__ osal_void hal_pow_set_band_spec_frame_tx_power(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u8 chan_idx)
{
    hal_public_hook_func(_pow_set_band_spec_frame_tx_power)(hal_device, band, chan_idx);
}

static INLINE__ osal_void hal_set_resp_pow_level(osal_char near_distance_rssi, osal_char far_distance_rssi)
{
    hal_public_hook_func(_set_resp_pow_level)(near_distance_rssi, far_distance_rssi);
}

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
static INLINE__ osal_void  hal_pow_set_user_resp_frame_tx_power(hal_to_dmac_device_stru *hal_device,
    osal_u8 lut_index, osal_u8 rssi_distance)
{
    hal_public_hook_func(_pow_set_user_resp_frame_tx_power)(hal_device, lut_index, rssi_distance);
}
#endif

static INLINE__ osal_void hal_pow_get_spec_frame_data_rate_idx(osal_u8 rate,  osal_u8 *rate_idx)
{
    hal_public_hook_func(_pow_get_spec_frame_data_rate_idx)(rate, rate_idx);
}
static INLINE__ osal_u16 hal_pow_get_pow_table_tpc_code(osal_u8 band, osal_u8 rate, osal_u8 bw, osal_u8 pwr_lvl)
{
    return hal_public_hook_func(_pow_get_pow_table_tpc_code)(band, rate, bw, pwr_lvl);
}

static INLINE__ osal_void hal_pow_get_data_rate_idx(osal_u8 protocol_mode, osal_u8 mcs, wlan_bw_cap_enum_uint8 bw,
    osal_u8 *rate_idx)
{
    hal_public_hook_func(_pow_get_data_rate_idx)(protocol_mode, mcs, bw, rate_idx);
}

static INLINE__ osal_u32 hal_get_tx_vector_word6(osal_void)
{
    return hal_public_hook_func(_get_tx_vector_word6)();
}
static INLINE__ osal_u32 hal_get_rpt_tx_data_word0(osal_void)
{
    return hal_public_hook_func(_get_rpt_tx_data_word0)();
}

/*
输入vap模式，hal创建vap结构，并标记vap id
*/
static INLINE__ osal_void hal_add_vap(hal_to_dmac_device_stru * hal_device,
    wlan_vap_mode_enum_uint8 vap_mode, osal_u8 hal_vap_id, hal_to_dmac_vap_stru ** hal_vap)
{
    hal_public_hook_func(_add_vap)(hal_device, vap_mode, hal_vap_id, hal_vap);
}

/*
输入vap模式，hal删除vap结构，并标记vap id
*/
static INLINE__ osal_void hal_del_vap(hal_to_dmac_device_stru * hal_device, osal_u8 vap_id)
{
    hal_public_hook_func(_del_vap)(hal_device, vap_id);
}

/*****************************************************************************
 函 数 名  : hal_clear_hw_fifo
 功能描述  : 清除硬件发送缓冲区
*****************************************************************************/
static INLINE__ osal_void hal_clear_hw_fifo(hal_to_dmac_device_stru *hal_device)
{
    /* disable PA之后、clear TX FIFO之前，增加10us延迟，期望硬件有足够时间回填描述符 */
    osal_udelay(10);
    hal_public_hook_func(_clear_hw_fifo)(hal_device);
}

/*****************************************************************************
 功能描述  : 屏蔽指定中断
*****************************************************************************/
static INLINE__ osal_void hal_mask_interrupt(osal_u32 offset)
{
    hal_public_hook_func(_mask_interrupt)(offset);
}

/*****************************************************************************
 功能描述  : 恢复指定中断
*****************************************************************************/
static INLINE__ osal_void hal_unmask_interrupt(osal_u32 offset)
{
    hal_public_hook_func(_unmask_interrupt)(offset);
}

#ifdef _PRE_WLAN_ONLINE_DPD

static INLINE__ osal_void  hal_dpd_cfr_set_bw(hal_to_dmac_device_stru *hal_device,
    wlan_channel_bandwidth_enum_uint8 bandwidth)
{
    hal_public_hook_func(_dpd_cfr_set_bw)(hal_device, bandwidth);
}

static INLINE__ osal_void  hal_dpd_cfr_set_freq(hal_to_dmac_device_stru *hal_device, osal_u8 val)
{
    hal_public_hook_func(_dpd_cfr_set_freq)(hal_device, val);
}
#endif

/*****************************************************************************
 功能描述  : 将密钥写进硬件寄存器
*****************************************************************************/
static  INLINE__ osal_void  hal_ce_add_key(hal_security_key_stru *security_key)
{
    hal_public_hook_func(_ce_add_key)(security_key);
}

/*****************************************************************************
 功能描述  : 读取cipher key值
*****************************************************************************/
static  INLINE__ osal_void  hal_ce_get_key(hal_security_key_stru *security_key)
{
    hal_public_hook_func(_ce_get_key)(security_key);
}

static INLINE__ osal_void hal_chan_measure_start(hal_chan_measure_stru *meas)
{
    hal_public_hook_func(_chan_measure_start)(meas);
}

/*****************************************************************************
 功能描述  : 读取硬件MAC信道统计寄存器
*****************************************************************************/
static INLINE__ osal_void hal_get_ch_statics_result(hal_ch_statics_irq_event_stru *ch_statics)
{
    hal_public_hook_func(_get_ch_statics_result)(ch_statics);
}

/*****************************************************************************
 函 数 名  : hal_enable_radar_det
 功能描述  : 设置硬件PHY雷达检测使能位
*****************************************************************************/
static INLINE__ osal_void hal_enable_radar_det(osal_u8 enable)
{
    hal_public_hook_func(_enable_radar_det)(enable);
}

/*****************************************************************************
 函 数 名  : hal_get_bcn_rate
 功能描述  : 获取某个vap的beacon datarate
*****************************************************************************/
static  INLINE__ osal_void  hal_get_bcn_rate(hal_to_dmac_vap_stru *hal_vap, osal_u8 *data_rate)
{
    hal_public_hook_func(_get_bcn_rate)(hal_vap, data_rate);
}

/*****************************************************************************
 函 数 名  : hal_set_bcn_phy_tx_mode
 功能描述  : 设置beacon帧的txmode
*****************************************************************************/
static  INLINE__ osal_void hal_set_bcn_phy_tx_mode(hal_to_dmac_vap_stru *hal_vap, osal_u32 pow_code)

{
    hal_public_hook_func(_set_bcn_phy_tx_mode)(hal_vap, pow_code);
}

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
/*****************************************************************************
 功能描述  : EDCA优化特性下清空部分mac统计寄存器
*****************************************************************************/
static  INLINE__ osal_void hal_set_counter1_clear(osal_void)
{
    hal_public_hook_func(_set_counter1_clear)();
}
#endif

#define HAL_VAP_LEVEL_FUNC
/*****************************************************************************
 函 数 名  : hal_vap_tsf_get_32bit
 功能描述  : 获取tsf低32位值
*****************************************************************************/
static INLINE__ osal_void  hal_vap_tsf_get_32bit(hal_to_dmac_vap_stru *hal_vap, osal_u32 *tsf_lo)
{
    hal_public_hook_func(_vap_tsf_get_32bit)(hal_vap, tsf_lo);
}

/*****************************************************************************
 函 数 名  : hal_vap_tsf_get_64bit
 功能描述  : 获取整个tsf 64位值
*****************************************************************************/
static INLINE__ osal_void  hal_vap_tsf_get_64bit(const hal_to_dmac_vap_stru *hal_vap, osal_u32 *tsf_high,
    osal_u32 *tsf_lo)
{
    hal_public_hook_func(_vap_tsf_get_64bit)(hal_vap, tsf_high, tsf_lo);
}

/*****************************************************************************
 函 数 名  : hal_vap_beacon_suspend
 功能描述  : 挂起beacon帧发送
*****************************************************************************/
static INLINE__ osal_void  hal_vap_beacon_suspend(hal_to_dmac_vap_stru *hal_vap)
{
    hal_public_hook_func(_vap_beacon_suspend)(hal_vap);
}

/*****************************************************************************
 函 数 名  : hal_vap_beacon_resume
 功能描述  : 恢复beacon帧发送
*****************************************************************************/
static INLINE__ osal_void  hal_vap_beacon_resume(hal_to_dmac_vap_stru *hal_vap)
{
    hal_public_hook_func(_vap_beacon_resume)(hal_vap);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
/*****************************************************************************
 函 数 名  : hal_set_hw_en_reg_cfg
 功能描述  : 设置硬件聚合需要配置的寄存器
*****************************************************************************/
static INLINE__ osal_void hal_set_hw_en_reg_cfg(osal_u8 hw_en)
{
    hal_public_hook_func(_set_hw_en_reg_cfg)(hw_en);
}
#endif
/*****************************************************************************
  hal vap TSF参数配置相关接口
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hal_vap_read_tbtt_timer
 功能描述  : 读取TBTT计数器
*****************************************************************************/
static INLINE__ osal_void  hal_vap_read_tbtt_timer(hal_to_dmac_vap_stru *hal_vap, osal_u32 *value)
{
    hal_public_hook_func(_vap_read_tbtt_timer)(hal_vap, value);
}

/*****************************************************************************
 函 数 名  : hal_vap_set_machw_beacon_period
 功能描述  : 设置vap的Beacon Period
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_machw_beacon_period(hal_to_dmac_vap_stru *hal_vap, osal_u16 beacon_period)
{
    hal_public_hook_func(_vap_set_machw_beacon_period)(hal_vap, beacon_period);
}

/*****************************************************************************
 功能描述  : wlan service 去注册
*****************************************************************************/
static INLINE__ osal_void hal_pm_servid_unregister(osal_void)
{
    hal_public_hook_func(_pm_servid_unregister)();
}

static  INLINE__ osal_void hal_pm_vote2platform(hal_pm_mode_enum_uint8 pm_mode)
{
    hal_public_hook_func(_pm_vote2platform)(pm_mode);
}

static INLINE__ osal_void hal_pm_set_bcn_rf_chain(hal_to_dmac_vap_stru *hal_vap, osal_u8 bcn_rf_chain)
{
    hal_public_hook_func(_pm_set_bcn_rf_chain)(hal_vap, bcn_rf_chain);
}

/*****************************************************************************
 函 数 名  : hal_enable_tsf_tbtt
 功能描述  : 打开tsf tbtt中断
*****************************************************************************/
static  INLINE__ osal_void hal_enable_tsf_tbtt(hal_to_dmac_vap_stru *hal_vap, oal_bool_enum_uint8 dbac_enable)
{
    hal_public_hook_func(_enable_tsf_tbtt)(hal_vap, dbac_enable);
}

/*****************************************************************************
 功能描述  : 发送单音信号
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hal_vap_get_gtk_rx_lut_idx
 功能描述  : hal vap获取lut idx
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_gtk_rx_lut_idx(hal_to_dmac_vap_stru *hal_vap, osal_u8 *lut_idx)
{
    hal_public_hook_func(_vap_get_gtk_rx_lut_idx)(hal_vap, lut_idx);
}

#ifdef _PRE_WLAN_FEATURE_M2S
static INLINE__ osal_void hal_update_datarate_by_chain(hal_to_dmac_device_stru *hal_device, osal_u8 resp_tx_chain)
{
    hal_public_hook_func(_update_datarate_by_chain)(hal_device, resp_tx_chain);
}

/*****************************************************************************
 功能描述  : 获取he rom里指示的nss
*****************************************************************************/
static INLINE__ osal_void hal_get_reg_he_rom_nss(osal_u32 *peer_rom_nss)
{
    hal_public_hook_func(_get_reg_he_rom_nss)(peer_rom_nss);
}
#endif


#ifdef _PRE_WLAN_FEATURE_FTM
/*****************************************************************************
 函 数 名  : hal_get_ftm_time
 功能描述  : 获取ftm time
*****************************************************************************/
static INLINE__ osal_u64  hal_get_ftm_time(osal_u64 time)
{
    return hal_public_hook_func(_get_ftm_time)(time);
}

/*****************************************************************************
 函 数 名  : hal_check_ftm_t4
 功能描述  : 检查ftm time
*****************************************************************************/
static INLINE__ osal_u64  hal_check_ftm_t4(osal_u64 time)
{
    return hal_public_hook_func(_check_ftm_t4)(time);
}

/*****************************************************************************
 函 数 名  : hal_check_ftm_t4
 功能描述  : 检查ftm time
*****************************************************************************/
static INLINE__ osal_char  hal_get_ftm_t4_intp(osal_u64 time)
{
    return hal_public_hook_func(_get_ftm_t4_intp)(time);
}

/*****************************************************************************
 函 数 名  : hal_check_ftm_t2
 功能描述  : 检查ftm time
*****************************************************************************/
static INLINE__ osal_u64  hal_check_ftm_t2(osal_u64 time)
{
    return hal_public_hook_func(_check_ftm_t2)(time);
}

/*****************************************************************************
 函 数 名  : hal_check_ftm_t4
 功能描述  : 检查ftm time
*****************************************************************************/
static INLINE__ osal_char  hal_get_ftm_t2_intp(osal_u64 time)
{
    return hal_public_hook_func(_get_ftm_t2_intp)(time);
}

/*****************************************************************************
 功能描述  : 设置ftm  总开关
*****************************************************************************/
static INLINE__ osal_void hal_set_ftm_enable(oal_bool_enum_uint8 ftm_status)
{
    hal_public_hook_func(_set_ftm_enable)(ftm_status);
}

/*****************************************************************************
 功能描述  : 设置ftm  initiator
*****************************************************************************/
static INLINE__ osal_void hal_set_ftm_sample(oal_bool_enum_uint8 ftm_status)
{
    hal_public_hook_func(_set_ftm_sample)(ftm_status);
}

/*****************************************************************************
 功能描述  : 设置ftm  responser
*****************************************************************************/
static INLINE__ osal_void hal_set_ftm_ctrl_status(oal_bool_enum_uint8 ftm_status)
{
    hal_public_hook_func(_set_ftm_ctrl_status)(ftm_status);
}

/*****************************************************************************
 函 数 名  : hal_get_ftm_cali_rx_time
*****************************************************************************/
static INLINE__ osal_void hal_get_ftm_cali_rx_time(osal_u32 *ftm_cali_rx_time)
{
    hal_public_hook_func(_get_ftm_cali_rx_time)(ftm_cali_rx_time);
}

/*****************************************************************************
 函 数 名  : hal_get_ftm_cali_rx_intp_time
*****************************************************************************/
static INLINE__ osal_void hal_get_ftm_cali_rx_intp_time(osal_u32 *ftm_cali_rx_intp_time)
{
    hal_public_hook_func(_get_ftm_cali_rx_intp_time)(ftm_cali_rx_intp_time);
}

/*****************************************************************************
 函 数 名  : hal_get_ftm_cali_tx_time
*****************************************************************************/
osal_void hal_get_ftm_cali_tx_time(osal_u32 *ftm_cali_tx_time);

/*****************************************************************************
 函 数 名  : hal_set_ftm_cali
 功能描述  : 设置ftm  环回
*****************************************************************************/
osal_void hal_set_ftm_cali(hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *tx_dscr, oal_bool_enum_uint8 ftm_cali);

/*****************************************************************************
 函 数 名  : hal_set_ftm_tx_cnt
 功能描述  : 设置FTM帧 硬件重传次数
*****************************************************************************/
osal_void hal_set_ftm_tx_cnt(hal_tx_dscr_stru *tx_dscr, osal_u8 ftm_tx_cnt);

/*****************************************************************************
 函 数 名  : hal_set_ftm_bandwidth
 功能描述  : 设置发送描述符中的tx_desc_freq_bandwidth_mode
*****************************************************************************/
osal_void hal_set_ftm_bandwidth(hal_tx_dscr_stru *tx_dscr, wlan_bw_cap_enum_uint8 band_cap);

/*****************************************************************************
 函 数 名  : hal_set_ftm_protocol
*****************************************************************************/
osal_void hal_set_ftm_protocol(hal_tx_dscr_stru *tx_dscr, wlan_phy_protocol_enum_uint8 prot_format);

/*****************************************************************************
 函 数 名  : hal_set_ftm_m2s
*****************************************************************************/
osal_void hal_set_ftm_m2s(hal_tx_dscr_stru *tx_dscr, osal_u8 tx_chain_selection);

/*****************************************************************************
 函 数 名  : hal_get_ftm_rtp_reg
*****************************************************************************/
osal_void hal_get_ftm_rtp_reg(osal_u32 *reg0, osal_u32 *reg1, osal_u32 *reg2, osal_u32 *reg3, osal_u32 *reg4);

/*****************************************************************************
 函 数 名  : hal_set_ftm_m2s_phy
*****************************************************************************/
osal_void hal_set_ftm_m2s_phy(  oal_bool_enum_uint8 is_mimo, osal_u8 tx_chain_selection);

/*****************************************************************************
 函 数 名  : hal_get_ftm_intp_status
 功能描述  : 获取ftm intp开关状态
*****************************************************************************/
osal_void hal_get_ftm_intp_status(osal_u32 *ftm_status);

/*****************************************************************************
 函 数 名  : hal_get_ftm_new_status
 功能描述  : 获取ftm 同步特性开关状态
*****************************************************************************/
osal_void hal_get_ftm_new_status(osal_u32 *ftm_status);

/*****************************************************************************
 函 数 名  : hal_get_ftm_cnt_status
 功能描述  : 获取ftm 计数器开关状态
*****************************************************************************/
osal_void hal_get_ftm_cnt_status(osal_u32 *ftm_status);

/*****************************************************************************
 函 数 名  : hal_get_ftm_pluse_enabne_status
 功能描述  : 获取ftm 脉冲开关状态
*****************************************************************************/
osal_void hal_get_ftm_pulse_enable_status(osal_u32 *ftm_status);

/*****************************************************************************
 函 数 名  : hal_get_ftm_crystal_oscillator_offset_update_status
 功能描述  : 获取ftm 更新晶振偏差开关状态
*****************************************************************************/
osal_void hal_get_ftm_crystal_oscillator_offset_update_status(osal_u32 *ftm_status);

/*****************************************************************************
 函 数 名  : hal_get_ftm_clock_offset_update_status
 功能描述  : 获取ftm 更新时钟偏差开关状态
*****************************************************************************/
osal_void hal_get_ftm_clock_offset_update_status(osal_u32 *ftm_status);

/*****************************************************************************
 功能描述  : 获取FTM时钟频率
*****************************************************************************/
osal_void hal_get_ftm_phy_bw_mode(osal_u32 *ftm_status);

/*****************************************************************************
 功能描述  : 获取FTM 插值上报频率
*****************************************************************************/
osal_void hal_get_ftm_intp_freq(osal_u32 *ftm_status);

/*****************************************************************************
 函 数 名  : hal_get_ftm_pulse_offset
 功能描述  : 获取ftm脉冲偏差值
*****************************************************************************/
osal_u64 hal_get_ftm_pulse_offset(osal_void);

/*****************************************************************************
 函 数 名  : hal_get_ftm_pulse_freq
 功能描述  : 获取ftm脉冲周期值
*****************************************************************************/
osal_u64 hal_get_ftm_pulse_freq(osal_void);

/*****************************************************************************
 函 数 名  : hal_get_ftm_crystal_oscillator_offset
 功能描述  : 获取ftm晶振偏差值
*****************************************************************************/
osal_u32 hal_get_ftm_crystal_oscillator_offset(osal_void);

/*****************************************************************************
 函 数 名  : hal_get_ftm_clock_offset
 功能描述  : 获取ftm时钟偏差值
*****************************************************************************/
osal_u64 hal_get_ftm_clock_offset(osal_void);

/*****************************************************************************
 函 数 名  : hal_set_ftm_new_status
 功能描述  : 设置ftm 同步特性开关状态
*****************************************************************************/
static INLINE__ osal_void hal_set_ftm_new_status(osal_u32 ftm_status)
{
    hal_public_hook_func(_set_ftm_intp_status)(ftm_status);
}

/*****************************************************************************
 函 数 名  : hal_set_ftm_new_status
 功能描述  : 设置ftm 同步计数步长复位状态
*****************************************************************************/
osal_void hal_set_ftm_step_count_status(osal_u32 ftm_status);

/*****************************************************************************
 函 数 名  : hal_set_ftm_cnt_status
 功能描述  : 设置ftm 计数器开关状态
*****************************************************************************/
osal_void hal_set_ftm_cnt_status(osal_u32 ftm_status);

/*****************************************************************************
 函 数 名  : hal_set_ftm_crystal_oscillator_offset_update_status
 功能描述  : 设置ftm 更新晶振偏差开关状态
*****************************************************************************/
osal_void hal_set_ftm_crystal_oscillator_offset_update_status(osal_u32 ftm_status);

/*****************************************************************************
 函 数 名  : hal_set_ftm_clock_offset_update_status
 功能描述  : 设置ftm 更新时钟偏差开关状态
*****************************************************************************/
static INLINE__ osal_void hal_set_ftm_clock_offset_update_status(osal_u32 ftm_status)
{
    hal_public_hook_func(_set_ftm_clock_offset_update_status)(ftm_status);
}

/*****************************************************************************
 函 数 名  : hal_set_ftm_crystal_oscillator_offset
 功能描述  : 设置ftm晶振偏差值
*****************************************************************************/
osal_void hal_set_ftm_crystal_oscillator_offset(osal_u32 time);

/*****************************************************************************
 函 数 名  : hal_set_ftm_clock_offset
 功能描述  : 设置ftm时钟偏差值
*****************************************************************************/
osal_void hal_set_ftm_clock_offset(osal_u64 time);

/*****************************************************************************
 函 数 名  : hal_sat_crystal_oscillator_offset
 功能描述  : 晶振偏差处理
*****************************************************************************/
osal_s64 hal_sat_crystal_oscillator_offset(osal_s64 clock_offset, osal_s64 delta);
/*****************************************************************************
 功能描述  : hal_get_rx_digital_scaling_bypass
*****************************************************************************/
osal_void hal_get_rx_digital_scaling_bypass(osal_u32 *bypass);
/*****************************************************************************
 功能描述  : hal_get_rx_lpf1_bypass
*****************************************************************************/
osal_void hal_get_rx_lpf1_bypass(osal_u32 *bypass);
/*****************************************************************************
 功能描述  : hal_get_rxiq_filter_force_bypass
*****************************************************************************/
osal_void hal_get_rxiq_filter_force_bypass(osal_u32 *bypass);

#endif

osal_void hal_flush_tx_complete_irq(hal_to_dmac_device_stru *hal_dev);

/*****************************************************************************
 功能描述  : FPGA单板根据协议模式配置phy的11ax ctl位
*****************************************************************************/
osal_void hal_set_ax_ctl_by_protocol(wlan_protocol_enum_uint8 protocol);

/************************************ *****************************************
 功能描述  : 用户功率信息初始化
*****************************************************************************/
osal_void  hal_device_init_vap_pow_code(hal_to_dmac_device_stru *hal_device,
    hal_vap_pow_info_stru *vap_pow_info, hal_pow_set_type_enum_uint8 type, const mac_channel_stru *channel);

/************************************ *****************************************
 功能描述  : 校准维测信息
*****************************************************************************/
osal_u32 hal_gp_get_mpdu_count(osal_void);
osal_u32 hal_gp_get_dbac_vap_stop_bitmap(osal_void);
osal_void hal_gp_set_dbac_vap_stop_bitmap(osal_u32 val);
osal_void hal_gp_clear_dbac_vap_stop_bitmap(osal_void);

/* 设置除基本信息外的所有其他描述符字段 */
static INLINE__ osal_void hal_tx_ucast_data_set_dscr(hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *tx_dscr, hal_tx_txop_feature_stru *txop_feature,
    hal_tx_txop_alg_stru *txop_alg, hal_tx_ppdu_feature_stru *ppdu_feature)
{
    hal_public_hook_func(_tx_ucast_data_set_dscr)(hal_device, tx_dscr, txop_feature, txop_alg, ppdu_feature);
}
/*****************************************************************************
 函 数 名  : hal_vap_get_machw_txop_limit_vivo
 功能描述  : 获取VI、VO的TXOP上限(单位: 微秒)
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_machw_txop_limit_ac(osal_u8 ac, osal_u16 *txop)
{
    hal_public_hook_func(_vap_get_machw_txop_limit_ac)(ac, txop);
}
#endif     //   #ifndef _PRE_WLAN_FEATURE_WS63
osal_u32 hal_gp_get_sdp_chnl_switch_off(osal_void);
osal_u32 hal_gp_get_slp_tx_ctrl(osal_void);

#ifdef _PRE_WLAN_FEATURE_CCA_OPT
/*****************************************************************************
 函 数 名  : hal_set_ed_high_th
 功能描述  : CCA优化下设置cca的20/40M检测门限
*****************************************************************************/
static  INLINE__ osal_void hal_set_ed_high_th(osal_s8 ed_high_20_reg_val, osal_s8 ed_high_40_reg_val,
    oal_bool_enum_uint8 is_default_th)
{
    hal_public_hook_func(_set_ed_high_th)(ed_high_20_reg_val, ed_high_40_reg_val, is_default_th);
}

static  INLINE__ osal_void hal_set_cca_prot_th(osal_s8 sd_cca_20th_dsss_val, osal_s8 sd_cca_20th_ofdm_val)
{
    hal_public_hook_func(_set_cca_prot_th)(sd_cca_20th_dsss_val, sd_cca_20th_ofdm_val);
}

/*****************************************************************************
 函 数 名  : hal_enable_sync_error_counter
 功能描述  : CCA优化特性下使能sync error统计寄存器
*****************************************************************************/
static  INLINE__ osal_void hal_enable_sync_error_counter(osal_s32 enable_cnt_reg_val)
{
    hal_public_hook_func(_enable_sync_error_counter)(enable_cnt_reg_val);
}

/*****************************************************************************
 函 数 名  : hal_get_sync_error_cnt
 功能描述  : CCA优化特性下获取sync error的统计计数值
*****************************************************************************/
static  INLINE__ osal_void hal_get_sync_error_cnt(osal_u32 *reg_val)
{
    hal_public_hook_func(_get_sync_error_cnt)(reg_val);
}

/*****************************************************************************
 函 数 名  : hal_set_sync_err_counter_clear
 功能描述  : CCA优化特性下清空sync error统计寄存器
*****************************************************************************/
static  INLINE__ osal_void hal_set_sync_err_counter_clear(osal_void)
{
    hal_public_hook_func(_set_sync_err_counter_clear)();
}
#endif

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT
/*****************************************************************************
 函 数 名  : hal_read_max_temperature
 功能描述  : 读取硬件MAC信道统计寄存器
*****************************************************************************/
static INLINE__ osal_u32 hal_read_max_temperature(osal_s16 *ps_temperature)
{
    return hal_public_hook_func(_read_max_temperature)(ps_temperature);
}

#ifdef _PRE_WLAN_FEATURE_TEMP_PROTECT_CLDO_MODE
/*****************************************************************************
 函 数 名  : hal_chip_is_cldo_mode
 功能描述  : 读取芯片是否为CLDO模式
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 hal_chip_is_cldo_mode(osal_void)
{
    return hal_public_hook_func(_chip_is_cldo_mode)();
}
#endif
#endif

#ifdef _PRE_PM_TBTT_OFFSET_PROBE
static INLINE__ osal_void hal_tbtt_offset_probe_init(hal_to_dmac_vap_stru *hal_vap)
{
    hal_public_hook_func(_tbtt_offset_probe_init)(hal_vap);
}
#endif

/*****************************************************************************
 函 数 名  : hal_tx_get_dscr_msdu_num
 功能描述  : 获取描述符中的msdunum
*****************************************************************************/
static INLINE__ osal_void  hal_tx_get_dscr_msdu_num(hal_tx_dscr_stru *tx_dscr, osal_u16 *msdu_num)
{
    hal_public_hook_func(_tx_get_dscr_msdu_num)(tx_dscr, msdu_num);
}

/*****************************************************************************
 功能描述  : 配置long/short slottime
*****************************************************************************/
static INLINE__ osal_void hal_cfg_slottime_type(osal_u32 slottime_type)
{
    hal_public_hook_func(_cfg_slottime_type)(slottime_type);
}

/*****************************************************************************
 功能描述  : 设置次信道带宽
*****************************************************************************/
static INLINE__ osal_void hal_set_sec_bandwidth_offset(osal_u8 bandwidth)
{
    hal_public_hook_func(_set_sec_bandwidth_offset)(bandwidth);
}

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/*****************************************************************************
 函 数 名  : hal_config_custom_rf
 功能描述  : 更新rf定制化参数
*****************************************************************************/
static INLINE__ osal_void hal_config_custom_rf(const osal_u8 *param)
{
    hal_public_hook_func(_config_custom_rf)(param);
}

/*****************************************************************************
 函 数 名  : hal_config_rssi_for_loss
 功能描述  : 设置RSSI插损值
*****************************************************************************/
static INLINE__ osal_void hal_config_rssi_for_loss(osal_u8 chanel_num, osal_s8 *rssi)
{
    hal_public_hook_func(_config_rssi_for_loss)(chanel_num, rssi);
}

static INLINE__ osal_void hal_config_get_cus_cca_param(hal_cfg_custom_cca_stru **cfg_cca)
{
    hal_public_hook_func(_config_get_cus_cca_param)(cfg_cca);
}
static INLINE__ osal_void hal_set_custom_rx_insert_loss(osal_u8 band, osal_u8 *insert_loss, osal_u8 len)
{
    hal_public_hook_func(_set_custom_rx_insert_loss)(band, insert_loss, len);
}
static INLINE__ osal_u8 hal_get_custom_rx_insert_loss(osal_u8 band, osal_u8 ch_idx)
{
    return hal_public_hook_func(_get_custom_rx_insert_loss)(band, ch_idx);
}
static INLINE__ osal_void hal_set_custom_power(osal_u8 band, osal_u8 *power, osal_u16 len)
{
    hal_public_hook_func(_set_custom_power)(band, power, len);
}
#endif // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
static  INLINE__ osal_u8 hal_get_rf_subband_idx(osal_u8 band, osal_u8 ch_idx)
{
    return hal_public_hook_func(_get_rf_subband_idx)(band, ch_idx);
}
/*****************************************************************************
 功能描述  : 将密钥写进硬件寄存器
*****************************************************************************/
static INLINE__ osal_void hal_ce_del_key(hal_security_key_stru *security_key)
{
    hal_public_hook_func(_ce_del_key)(security_key);
}

/*****************************************************************************
 函 数 名  : hal_disable_tsf_tbtt
 功能描述  : 关闭tsf tbtt中断
*****************************************************************************/
static  INLINE__ osal_void hal_disable_tsf_tbtt(const hal_to_dmac_vap_stru *hal_vap)
{
    hal_public_hook_func(_disable_tsf_tbtt)(hal_vap);
}

static INLINE__ hmac_pps_value_stru hal_get_auto_adjust_freq_pps(osal_void)
{
    return hal_public_hook_func(_get_auto_adjust_freq_pps)();
}

static INLINE__ osal_void hal_tx_init_dscr_queue(hal_to_dmac_device_stru *device)
{
    hal_public_hook_func(_tx_init_dscr_queue)(device);
}

/*****************************************************************************
 函 数 名  : hal_get_tx_q_status
 功能描述  : 获取发送队列状态
*****************************************************************************/
static INLINE__ osal_void hal_set_bcn_timeout_multi_q_enable(hal_to_dmac_vap_stru * hal_vap, osal_u8 enable)
{
    hal_public_hook_func(_set_bcn_timeout_multi_q_enable)(hal_vap, enable);
}

static inline osal_void hal_get_sr_info(osal_u8 param)
{
    hal_public_hook_func(_get_sr_info)(param);
}

static inline osal_void hal_set_psr_vfs_get(dmac_sr_sta_vfs_stru *hmac_sr_sta_vfs)
{
    hal_public_hook_func(_set_psr_vfs_get)(hmac_sr_sta_vfs);
}

static inline osal_void hal_set_sr_vfs_get(dmac_sr_sta_vfs_stru *hmac_sr_sta_vfs)
{
    hal_public_hook_func(_set_sr_vfs_get)(hmac_sr_sta_vfs);
}

static inline osal_void hal_set_sr_statistics_get(dmac_sr_sta_srg_stru *sr_sta_srg,
    dmac_sr_sta_non_srg_stru *sr_sta_non_srg)
{
    hal_public_hook_func(_set_sr_statistics_get)(sr_sta_srg, sr_sta_non_srg);
}

static inline osal_void hal_set_psr_statistics_end(osal_void)
{
    hal_public_hook_func(_set_psr_statistics_end)();
}

static inline osal_void hal_set_sr_vfs_end(osal_void)
{
    hal_public_hook_func(_set_sr_vfs_end)();
}

static inline osal_void hal_set_sr_statistics_end(osal_void)
{
    hal_public_hook_func(_set_sr_statistics_end)();
}

static inline osal_void hal_set_psr_statistics_start(osal_void)
{
    hal_public_hook_func(_set_psr_statistics_start)();
}

static inline osal_void hal_set_sr_vfs_start(osal_void)
{
    hal_public_hook_func(_set_sr_vfs_start)();
}

static inline osal_void hal_set_sr_statistics_start(osal_void)
{
    hal_public_hook_func(_set_sr_statistics_start)();
}

static inline osal_void hal_set_psr_statistics_continue(osal_void)
{
    hal_public_hook_func(_set_psr_statistics_continue)();
}

static inline osal_void hal_set_sr_statistics_continue(osal_void)
{
    hal_public_hook_func(_set_sr_statistics_continue)();
}
static inline osal_void hal_set_psr_statistics_stop(osal_void)
{
    hal_public_hook_func(_set_psr_statistics_stop)();
}

static inline osal_void hal_set_sr_statistics_stop(osal_void)
{
    hal_public_hook_func(_set_sr_statistics_stop)();
}

static inline osal_void hal_set_psr_statistics_clear(osal_void)
{
    hal_public_hook_func(_set_psr_statistics_clear)();
}

static inline osal_void hal_set_sr_vfs_clear(osal_void)
{
    hal_public_hook_func(_set_sr_vfs_clear)();
}

static inline osal_void hal_set_sr_statistics_clear(osal_void)
{
    hal_public_hook_func(_set_sr_statistics_clear)();
}

static INLINE__ osal_void hal_set_psr_ctrl(oal_bool_enum_uint8 flag)
{
    hal_public_hook_func(_set_psr_ctrl)(flag);
}

static INLINE__ osal_void hal_set_psrt_htc(osal_void)
{
    hal_public_hook_func(_set_psrt_htc)();
}

static INLINE__ osal_void hal_set_psr_offset(osal_s32 offset)
{
    hal_public_hook_func(_set_psr_offset)(offset);
}

static INLINE__ osal_void hal_set_srg_pd_etc(osal_u32 pd, osal_u32 pw)
{
    hal_public_hook_func(_set_srg_pd_etc)(pd, pw);
}

static INLINE__ osal_void hal_set_non_pd(osal_u32 pd, osal_u32 pw)
{
    hal_public_hook_func(_set_non_pd)(pd, pw);
}

static INLINE__ osal_void hal_set_srg_ctrl(osal_void)
{
    hal_public_hook_func(_set_srg_ctrl)();
}

static INLINE__ osal_void hal_set_srg_partial_bssid_bitmap(osal_u32 bitmap_low, osal_u32 bitmap_high)
{
    hal_public_hook_func(_set_srg_partial_bssid_bitmap)(bitmap_low, bitmap_high);
}

static INLINE__ osal_void hal_set_srg_bss_color_bitmap(osal_u32 bitmap_low, osal_u32 bitmap_high)
{
    hal_public_hook_func(_set_srg_bss_color_bitmap)(bitmap_low, bitmap_high);
}
#ifdef _PRE_WLAN_FEATURE_BSRP
/*****************************************************************************
 功能描述  : 设置NFRP反馈的buff状态
*****************************************************************************/
static INLINE__ osal_void hal_set_ndp_status(osal_u8 status)
{
    hal_public_hook_func(_set_ndp_status)(status);
}
#endif

static INLINE__ osal_void hal_set_srg_ele_off(osal_void)
{
    hal_public_hook_func(_set_srg_ele_off)();
}

static INLINE__ osal_void hal_set_sr_ctrl(oal_bool_enum_uint8 flag)
{
    hal_public_hook_func(_set_sr_ctrl)(flag);
}

static inline osal_void hal_color_rpt_clr(osal_void)
{
    hal_public_hook_func(_color_rpt_clr)();
}

static inline osal_void hal_color_rpt_en(oal_bool_enum_uint8 flag)
{
    hal_public_hook_func(_color_rpt_en)(flag);
}

static inline osal_void hal_color_area_get(mac_color_area_stru *color_area)
{
    hal_public_hook_func(_color_area_get)(color_area);
}

static INLINE__ osal_void hal_set_dev_support_11ax(hal_to_dmac_device_stru *hal_device, osal_u8 reg_value)
{
    hal_public_hook_func(_set_dev_support_11ax)(hal_device, reg_value);
}

static INLINE__ osal_void hal_set_mu_edca_lifetime(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    hal_public_hook_func(_set_mu_edca_lifetime)(bk, be, vi, vo);
}

static INLINE__ osal_void hal_set_mu_edca_aifsn(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    hal_public_hook_func(_set_mu_edca_aifsn)(bk, be, vi, vo);
}

static INLINE__ osal_void hal_set_mu_edca_cw(osal_u8 ac_type, osal_u8 cwmax, osal_u8 cwmin)
{
    hal_public_hook_func(_set_mu_edca_cw)(ac_type, cwmax, cwmin);
}

static INLINE__ osal_void hal_set_affected_acs(osal_u8 affected_acs)
{
    hal_public_hook_func(_set_affected_acs)(affected_acs);
}

static INLINE__ osal_void hal_set_mu_edca_func_en(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    hal_public_hook_func(_set_mu_edca_func_en)(bk, be, vi, vo);
}

static INLINE__ osal_void hal_close_mu_edca_func(osal_void)
{
    hal_public_hook_func(_close_mu_edca_func)();
}

static INLINE__ osal_void hal_set_mac_backoff_delay(osal_u32 back_off_time)
{
    hal_public_hook_func(_set_mac_backoff_delay)(back_off_time);
}

static INLINE__ osal_void hal_set_bss_color(hal_to_dmac_vap_stru *hal_vap, osal_u8 bss_color)
{
    hal_public_hook_func(_set_bss_color)(hal_vap, bss_color);
}

static INLINE__ osal_void hal_set_partial_bss_color(hal_to_dmac_vap_stru *hal_vap, osal_u8 partial_bss_color)
{
    hal_public_hook_func(_set_partial_bss_color)(hal_vap, partial_bss_color);
}

static INLINE__ osal_void hal_set_bss_color_enable(oal_bool_enum_uint8 flag)
{
    hal_public_hook_func(_set_bss_color_enable)(flag);
}

static inline osal_void hal_set_phy_aid(osal_u16 aid)
{
    hal_public_hook_func(_set_phy_aid)(aid);
}

static inline osal_void hal_set_he_rom_en(oal_bool_enum_uint8 rx_om)
{
    hal_public_hook_func(_set_he_rom_en)(rx_om);
}

static inline osal_void hal_set_queue_size_in_ac(osal_u32 queue_size, osal_u8 vap_id, osal_u16 ac_order)
{
    hal_public_hook_func(_set_queue_size_in_ac)(queue_size, vap_id, ac_order);
}

/*****************************************************************************
 功能描述  : 设置硬件接收描述符队列首地址
*****************************************************************************/
static INLINE__ osal_void hal_set_machw_rx_buff_addr(osal_u32 rx_dscr, hal_rx_dscr_queue_id_enum_uint8 queue_num)
{
    hal_public_hook_func(_set_machw_rx_buff_addr)(rx_dscr, queue_num);
}

/*****************************************************************************
 功能描述  : 挂起硬件发送
*****************************************************************************/
static INLINE__ osal_void  hal_set_machw_tx_suspend(osal_void)
{
    hal_public_hook_func(_set_machw_tx_suspend)();
}

/*****************************************************************************
 功能描述  : 复位MAC 和 PHY
*****************************************************************************/
static INLINE__ osal_void  hal_reset_phy_machw(hal_reset_hw_type_enum_uint8 type, osal_u8 sub_mod,
    osal_u8 reset_phy_reg, osal_u8 reset_mac_reg)
{
    hal_public_hook_func(_reset_phy_machw)(type, sub_mod, reset_phy_reg, reset_mac_reg);
}
osal_void hal_tpc_cali_ftm_updata_channel(hal_to_dmac_device_stru *hal_device, mac_channel_stru *channel);
/*****************************************************************************
          RA LUT操作相关接口
*****************************************************************************/
/*****************************************************************************
 功能描述  : 设置TX SEQUENCE NUM
*****************************************************************************/
static INLINE__ osal_void hal_set_tx_sequence_num(machw_tx_sequence_stru tx_seq, osal_u32 val_write, osal_u8 vap_index)
{
    hal_public_hook_func(_set_tx_sequence_num)(tx_seq, val_write, vap_index);
}

/*****************************************************************************
 功能描述  : 控制硬件在收到单播帧时不回ACK
*****************************************************************************/
static INLINE__ osal_void hal_disable_machw_ack_trans(osal_void)
{
    hal_public_hook_func(_disable_machw_ack_trans)();
}

/*****************************************************************************
 功能描述  : 控制硬件在收到单播帧时回ACK
*****************************************************************************/
static INLINE__ osal_void hal_enable_machw_ack_trans(osal_void)
{
    hal_public_hook_func(_enable_machw_ack_trans)();
}

/*****************************************************************************
  RF相关接口
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hal_initialize_rf_sys
*****************************************************************************/
static INLINE__ osal_void  hal_initialize_rf_sys(hal_to_dmac_device_stru * hal_device)
{
    hal_public_hook_func(_initialize_rf_sys)(hal_device);
}

/*****************************************************************************
函 数 名  : hal_adjust_tx_power
功能描述  : 调整tx power
*****************************************************************************/
static INLINE__ osal_void hal_adjust_tx_power(osal_u8 ch, osal_s8 power)
{
    hal_public_hook_func(_adjust_tx_power)(ch, power);
}

/*****************************************************************************
函 数 名  : hal_get_tpc_code
功能描述  : 获取tpc_code的值
*****************************************************************************/
static INLINE__ osal_u8 hal_get_tpc_code(osal_void)
{
    return hal_public_hook_func(_get_tpc_code)();
}

/*****************************************************************************
函 数 名  : hal_restore_tx_power
功能描述  : 恢复tx power
*****************************************************************************/
static INLINE__ osal_void hal_restore_tx_power(osal_u8 ch)
{
    hal_public_hook_func(_restore_tx_power)(ch);
}

static INLINE__ osal_void hal_cfg_txop_cycle(osal_u16 ch)
{
    hal_public_hook_func(_cfg_txop_cycle)(ch);
}

static INLINE__ osal_void hal_cfg_lifetime(osal_u8 ac, osal_u16 lifetime)
{
    hal_public_hook_func(_cfg_lifetime)(ac, lifetime);
}

static INLINE__ osal_void hal_bl_sifs_en(osal_u8 en)
{
    hal_public_hook_func(_bl_sifs_en)(en);
}

static INLINE__ osal_void hal_rts_info(osal_u32 rate, osal_u32 phy_mode, osal_u8 *addr1)
{
    hal_public_hook_func(_rts_info)(rate, phy_mode, addr1);
}

static INLINE__ osal_void hal_cfg_txop_en(osal_u8 en)
{
    hal_public_hook_func(_bl_cfg_txop_en)(en);
}

static INLINE__ osal_void  hal_pow_initialize_tx_power(hal_to_dmac_device_stru * hal_device)
{
    hal_public_hook_func(_pow_initialize_tx_power)(hal_device);
}

#ifdef _PRE_WLAN_FEATURE_11AX
static INLINE__ osal_void hal_tb_mcs_tx_power(wlan_channel_band_enum_uint8 band)
{
    hal_public_hook_func(_tb_mcs_tx_power)(band);
}

static INLINE__ osal_void hal_tb_tx_power_init(wlan_channel_band_enum_uint8 band)
{
    hal_public_hook_func(_tb_tx_power_init)(band);
}
#endif /* #ifdef _PRE_WLAN_FEATURE_11AX */

static  INLINE__ osal_void hal_pow_set_rf_regctl_enable(hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 rf_linectl)
{
    hal_public_hook_func(_pow_set_rf_regctl_enable)(hal_device, rf_linectl);
}

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
static INLINE__ osal_void  hal_pow_del_machw_resp_power_lut_entry(osal_u8 lut_index)
{
    hal_public_hook_func(_pow_del_machw_resp_power_lut_entry)(lut_index);
}
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
/*****************************************************************************
 功能描述  : hal层动态校准描述符间隔数参数配置
*****************************************************************************/
static INLINE__ osal_void hal_config_set_dyn_cali_dscr_interval(hal_to_dmac_device_stru *hal_device,
    wlan_channel_band_enum_uint8 band, osal_u16 param_val)
{
    hal_public_hook_func(_config_set_dyn_cali_dscr_interval)(hal_device, band, param_val);
    return;
}

#endif

/*****************************************************************************
  SoC相关接口
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hal_initialize_soc
 功能描述  : SoC上电初始化接口
*****************************************************************************/
static INLINE__ osal_void  hal_initialize_soc(hal_to_dmac_device_stru * hal_device)
{
    hal_public_hook_func(_initialize_soc)(hal_device);
}

/*****************************************************************************
 函 数 名  : hal_clear_mac_int_status
*****************************************************************************/
static INLINE__ osal_void  hal_clear_mac_int_status(osal_u32 status)
{
    hal_public_hook_func(_clear_mac_int_status)(status);
}

#ifdef _PRE_WLAN_DFR_STAT
/*****************************************************************************
 函 数 名  : hal_clear_mac_error_int_status
*****************************************************************************/
static INLINE__ osal_void hal_clear_mac_error_int_status(hal_error_state_stru *status)
{
    hal_public_hook_func(_clear_mac_error_int_status)(status);
}
#endif

static INLINE__ osal_void  hal_show_irq_info(hal_to_dmac_device_stru * hal_device, osal_u8 param)
{
    hal_public_hook_func(_show_irq_info)(hal_device, param);
}

static INLINE__ osal_void  hal_clear_irq_stat(hal_to_dmac_device_stru * hal_device)
{
    hal_public_hook_func(_clear_irq_stat)(hal_device);
}

static INLINE__ osal_void hal_cali_send_func(hal_to_dmac_device_stru *hal_device, osal_u8* cal_data_write,
    osal_u16 frame_len, osal_u16 remain)
{
    hal_public_hook_func(_cali_send_func)(hal_device, cal_data_write, frame_len, remain);
}


/*****************************************************************************
 功能描述  : 配置EIFS_TIME 寄存器
*****************************************************************************/
static INLINE__ osal_void hal_config_eifs_time(wlan_protocol_enum_uint8 protocol)
{
    hal_public_hook_func(_config_eifs_time)(protocol);
}

#ifdef _PRE_WLAN_ONLINE_DPD
static INLINE__ osal_void  hal_dpd_cfr_set_11b(hal_to_dmac_device_stru *hal_device, osal_u8 is_11b)
{
    hal_public_hook_func(_dpd_cfr_set_11b)(hal_device, is_11b);
}
#endif

/*****************************************************************************
 函 数 名  : hal_get_dieid
 功能描述  : 获取芯片dieid
*****************************************************************************/
static INLINE__ osal_void hal_get_dieid(hal_to_dmac_device_stru * hal_device, osal_u32 *dieid, osal_u32 *length)
{
    hal_public_hook_func(_get_dieid)(hal_device, dieid, length);
}

/*****************************************************************************
 功能描述  : 不过滤beacon帧
*****************************************************************************/
static  INLINE__ osal_void hal_disable_beacon_filter(osal_void)
{
    hal_public_hook_func(_disable_beacon_filter)();
}

/*****************************************************************************
 功能描述  : 使能non frame filter
*****************************************************************************/
static  INLINE__ osal_void hal_enable_non_frame_filter(osal_void)
{
    hal_public_hook_func(_enable_non_frame_filter)();
}

#ifdef _PRE_WLAN_FEATURE_PMF
/*****************************************************************************
 函 数 名  : hal_set_pmf_crypto
 功能描述  : 设置pmf加解密使能位
*****************************************************************************/
static  INLINE__ osal_void  hal_set_pmf_crypto(hal_to_dmac_vap_stru *hal_vap, oal_bool_enum_uint8 crypto)
{
    hal_public_hook_func(_set_pmf_crypto)(hal_vap, crypto);
}
#endif /* #ifdef _PRE_WLAN_FEATURE_PMF  */


/*****************************************************************************
 功能描述  : 关闭硬件加密
*****************************************************************************/
static INLINE__ osal_void hal_disable_ce(osal_void)
{
    hal_public_hook_func(_disable_ce)();
}

/*****************************************************************************
 功能描述  : 将指定的MAC 地址添加到硬件Lut表
*****************************************************************************/
static  INLINE__ osal_void hal_ce_add_peer_macaddr(osal_u8 lut_idx, osal_u8 * addr)
{
    hal_public_hook_func(_ce_add_peer_macaddr)(lut_idx, addr);
}

/*****************************************************************************
 功能描述  : 将指定的MAC 地址从硬件Lut表删除
*****************************************************************************/
static  INLINE__ osal_void hal_ce_del_peer_macaddr(osal_u8 lut_idx)
{
    hal_public_hook_func(_ce_del_peer_macaddr)(lut_idx);
}

/*****************************************************************************
 功能描述  : 获取硬件mac地址，来自eeprom或flash
*****************************************************************************/
static INLINE__ osal_void hal_get_hw_addr(osal_u8 *addr)
{
    hal_public_hook_func(_get_hw_addr)(addr);
}


/*****************************************************************************
 功能描述  : 设置rx过滤常收状态
*****************************************************************************/
osal_void hal_rx_filter_set_alrx_state(hal_device_always_rx_state_enum al_rx_flag);
osal_void hal_mac_interrupt_clear(osal_void);

/*****************************************************************************
 功能描述  : 设置rx帧过滤配置
            注: 入参value：
            bit0=1 :上报组播(广播)数据帧使能标志
            bit1=1 :上报单播数据包使能标志
            bit2=1 :上报组播(广播)管理帧使能标志
            bit3=1 :上报单播管理帧使能标志
*****************************************************************************/
osal_void hal_rx_filter_set_rx_flt_en(osal_u32 value);

#ifdef _PRE_WLAN_FEATURE_TXBF
/*****************************************************************************
 函 数 名  : hal_set_dl_mumimo_ctrl
 功能描述  : 设置MU-MIMO控制寄存器
*****************************************************************************/
static  INLINE__ osal_void hal_set_dl_mumimo_ctrl(oal_bool_enum_uint8 enable)
{
    hal_public_hook_func(_set_dl_mumimo_ctrl)(enable);
}

/*****************************************************************************
 函 数 名  : hal_set_h_matrix_timeout
 功能描述  : 设置TXBF 矩阵超时寄存器
*****************************************************************************/
static  INLINE__ osal_void hal_set_h_matrix_timeout(osal_u32 reg_value)
{
    hal_public_hook_func(_set_h_matrix_timeout)(reg_value);
}

/*****************************************************************************
 功能描述  : 设置mu-mimo aid以及matrix地址
*****************************************************************************/
static  INLINE__ osal_void hal_set_mu_aid_matrix_info(hal_to_dmac_vap_stru *hal_vap, osal_u16 aid)
{
    hal_public_hook_func(_set_mu_aid_matrix_info)(hal_vap, aid);
}

/*****************************************************************************
 功能描述  : 设置VHT matrix buff的地址
*****************************************************************************/
static  INLINE__ osal_void hal_set_txbf_he_buff_addr(osal_u32 addr, osal_u16 buffer_len)
{
    hal_public_hook_func(_set_txbf_he_buff_addr)(addr, buffer_len);
}

/*****************************************************************************
 函 数 名  : hal_set_bfee_bypass_clk_gating
 功能描述  : 异系统共存时,是否开启clk gating
*****************************************************************************/
static  INLINE__ osal_void hal_set_bfee_bypass_clk_gating(osal_u8 enable)
{
    hal_public_hook_func(_set_bfee_bypass_clk_gating)(enable);
}

/*****************************************************************************
 函 数 名  : hal_set_channel_est_for_txbfee
 功能描述  : 使能TXBF时启用MLD,否则使用Zero-Force
*****************************************************************************/
static  INLINE__ osal_void hal_set_channel_est_for_txbfee(osal_u8 txbf,
    wlan_channel_bandwidth_enum_uint8 bandwidth, wlan_channel_band_enum_uint8 band)
{
    hal_public_hook_func(_set_channel_est_for_txbfee)(txbf, bandwidth, band);
}

/*****************************************************************************
 功能描述  : 启动反馈矩阵
*****************************************************************************/
static  INLINE__ osal_void hal_set_bfee_sounding_en(osal_u8 bfee_enable)
{
    hal_public_hook_func(_set_bfee_sounding_en)(bfee_enable);
}

/*****************************************************************************
 功能描述  : 设置ht sounding的压缩V矩阵的子载波分组
*****************************************************************************/
static  INLINE__ osal_void hal_set_bfee_h2v_beamforming_ng(osal_u8 user_bw)
{
    hal_public_hook_func(_set_bfee_h2v_beamforming_ng)(user_bw);
}

/*****************************************************************************
 功能描述  : 设置ht sounding的压缩V矩阵的子载波分组，并且使能ht bfee
*****************************************************************************/
static  INLINE__ osal_void hal_set_bfee_grouping_codebook(osal_u8 codebook, osal_u8 min_group)
{
    hal_public_hook_func(_set_bfee_grouping_codebook)(codebook, min_group);
}

/*****************************************************************************
 功能描述  : 根据带宽设置计算bfer解压缩子载波分组
*****************************************************************************/
static INLINE__ osal_void hal_set_bfer_subcarrier_ng(wlan_bw_cap_enum_uint8 user_bw)
{
    hal_public_hook_func(_set_bfer_subcarrier_ng)(user_bw);
}
#endif  /* _PRE_WLAN_FEATURE_TXBF */

/*****************************************************************************
 功能描述  : 获取mac rx关键统计信息
*****************************************************************************/
static  INLINE__ osal_void hal_get_mac_rx_statistics_data(hal_mac_rx_mpdu_statis_info_stru *mac_rx_statis)
{
    hal_public_hook_func(_get_mac_rx_statistics_data)(mac_rx_statis);
}
/*****************************************************************************
 功能描述  : 获取mac rx关键统计信息
*****************************************************************************/
static  INLINE__ osal_void hal_get_mac_tx_statistics_data(hal_mac_tx_mpdu_statis_info_stru *mac_tx_statis)
{
    hal_public_hook_func(_get_mac_tx_statistics_data)(mac_tx_statis);
}
/*****************************************************************************
 功能描述  : 清除MAC统计信息
*****************************************************************************/
static  INLINE__ osal_void hal_hw_stat_clear(osal_void)
{
    hal_public_hook_func(_hw_stat_clear)();
}

/*****************************************************************************
 功能描述  : 检查mac中断状态
*****************************************************************************/
static INLINE__ oal_bool_enum_uint8 hal_check_mac_int_status(osal_void)
{
    return hal_public_hook_func(_check_mac_int_status)();
}

/*****************************************************************************
 功能描述  : 使能EDCA寄存器
*****************************************************************************/
static INLINE__ osal_void  hal_enable_machw_edca(osal_void)
{
    hal_public_hook_func(_enable_machw_edca)();
}


/*****************************************************************************
 函 数 名  : hal_set_phy_max_bw_field
 功能描述  : 设置phy max bw下三个功能配置
*****************************************************************************/
static  INLINE__ osal_void hal_set_phy_max_bw_field(hal_to_dmac_device_stru *hal_device,
    osal_u32 data, hal_phy_max_bw_sect_enmu_uint8 sect)
{
    hal_public_hook_func(_set_phy_max_bw_field)(hal_device, data, sect);
}
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
/*****************************************************************************
 函 数 名  : hal_rf_test_disable_al_tx
 功能描述  : 禁用常发
*****************************************************************************/
static  INLINE__ osal_void hal_rf_test_disable_al_tx(hal_to_dmac_device_stru *hal_device)
{
    hal_public_hook_func(_rf_test_disable_al_tx)(hal_device);
}
#endif
#ifndef _PRE_WLAN_FEATURE_WS63
/*****************************************************************************
 功能描述  : 配置寄存器帧过滤寄存器某些bit
*****************************************************************************/
static  INLINE__ osal_void hal_set_rx_filter_reg(osal_u32 rx_filter_command)
{
    hal_public_hook_func(_set_rx_filter_reg)(rx_filter_command);
}
#endif

/*****************************************************************************
 函 数 名  : hal_vap_set_beacon_rate
 功能描述  : 设置beacon发送速率
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_beacon_rate(hal_to_dmac_vap_stru        *hal_vap,
    osal_u32 beacon_rate)
{
    hal_public_hook_func(_vap_set_beacon_rate)(hal_vap, beacon_rate);
}

/*****************************************************************************
  10.2 对应一套硬件MAC VAP的静态内联函数
*****************************************************************************/
static INLINE__ osal_void hal_vap_set_macaddr(hal_to_dmac_vap_stru * hal_vap, const osal_u8 *mac_addr,
    osal_u16 mac_addr_len)
{
    hal_public_hook_func(_vap_set_macaddr)(hal_vap, mac_addr, mac_addr_len);
}

/*****************************************************************************
 函 数 名  : hal_vap_set_opmode
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_opmode(hal_to_dmac_vap_stru *hal_vap, wlan_vap_mode_enum_uint8 vap_mode)
{
    hal_public_hook_func(_vap_set_opmode)(hal_vap, vap_mode);
}


/*****************************************************************************
 函 数 名  : hal_vap_clr_opmode
*****************************************************************************/
static INLINE__ osal_void hal_vap_clr_opmode(hal_to_dmac_vap_stru *hal_vap, wlan_vap_mode_enum_uint8 vap_mode)
{
    hal_public_hook_func(_vap_clr_opmode)(hal_vap, vap_mode);
}

/*****************************************************************************
  hal vap EDCA参数配置相关接口
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hal_vap_set_machw_aifsn_all_ac
 功能描述  : 设置所有AC的仲裁帧间距(arbitration inter-frame space, AIFS)
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_machw_aifsn_all_ac(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    hal_public_hook_func(_vap_set_machw_aifsn_all_ac)(bk, be, vi, vo);
}

/*****************************************************************************
 函 数 名  : hal_vap_set_machw_aifsn_ac
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_machw_aifsn_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u8 aifs)
{
    hal_public_hook_func(_vap_set_machw_aifsn_ac)(ac, aifs);
}
/*****************************************************************************
 函 数 名  : hal_vap_set_machw_aifsn_ac
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_machw_aifsn_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u8 *aifs)
{
    hal_public_hook_func(_vap_get_machw_aifsn_ac)(ac, aifs);
}

static INLINE__ osal_void  hal_vap_set_machw_aifsn_ac_wfa(wlan_wme_ac_type_enum_uint8 ac,
    osal_u8 aifs, wlan_wme_ac_type_enum_uint8 wfa_lock)
{
    hal_public_hook_func(_vap_set_machw_aifsn_ac_wfa)(ac, aifs, wfa_lock);
}

static INLINE__ osal_void  hal_vap_set_edca_machw_cw_wfa(osal_u8 cwmaxmin, osal_u8 ec_type,
    wlan_wme_ac_type_enum_uint8 wfa_lock)
{
    hal_public_hook_func(_vap_set_edca_machw_cw_wfa)(cwmaxmin, ec_type, wfa_lock);
}

/*****************************************************************************
 函 数 名  : hal_vap_set_edca_machw_cw
 功能描述  : 设置BE、BK、VI、VO竞争窗口的上下限(CWmin, CWmax)
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_edca_machw_cw(osal_u8 cwmax, osal_u8 cwmin,
    osal_u8 ec_type)
{
    hal_public_hook_func(_vap_set_edca_machw_cw)(cwmax, cwmin, ec_type);
}

/*****************************************************************************
 函 数 名  : hal_vap_get_edca_machw_cw
 功能描述  : 获取BE、BK、VI、VO竞争窗口的上下限(CWmin, CWmax)
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_edca_machw_cw(osal_u8 *cwmax, osal_u8 *cwmin, osal_u8 ec_type)
{
    hal_public_hook_func(_vap_get_edca_machw_cw)(cwmax, cwmin, ec_type);
}

/*****************************************************************************
 函 数 名  : hal_vap_set_machw_txop_limit_bkbe
 功能描述  : 设置BK、BE的TXOP上限(单位: 微秒)
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年5月11日
    修改内容   : 新生成函数

*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_machw_txop_limit_bkbe(osal_u16 be, osal_u16 bk)
{
    hal_public_hook_func(_vap_set_machw_txop_limit_bkbe)(be, bk);
}

/*****************************************************************************
 函 数 名  : hal_vap_get_machw_txop_limit_bkbe
 功能描述  : 获取BK、BE的TXOP上限(单位: 微秒)
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_machw_txop_limit_bkbe(osal_u16 *be, osal_u16 *bk)
{
    hal_public_hook_func(_vap_get_machw_txop_limit_bkbe)(be, bk);
}

/*****************************************************************************
 功能描述  : 设置txop_duration_threshold(单位: 32微秒)
*****************************************************************************/
static INLINE__ osal_void hal_vap_set_txop_duration_threshold(osal_u32 txop_duration_threshold, osal_u8 vap_id)
{
    hal_public_hook_func(_vap_set_txop_duration_threshold)(txop_duration_threshold, vap_id);
}

/*****************************************************************************
 功能描述  : 设置MAC时钟频率
*****************************************************************************/
static INLINE__ osal_void hal_set_mac_freq(hal_clk_freq_enum_uint8 mac_clk_freq)
{
    hal_public_hook_func(_set_mac_freq)(mac_clk_freq);
}

/*****************************************************************************
 功能描述  : 设置AX vap idx(单位: 32微秒)
*****************************************************************************/
static INLINE__ osal_void hal_set_11ax_vap_idx(osal_u8 hal_vap_id)
{
    hal_public_hook_func(_set_11ax_vap_idx)(hal_vap_id);
}

/*****************************************************************************
 函 数 名  : hal_set_txop_check_cca
 功能描述  : 设置txop check cca。
*****************************************************************************/
static INLINE__ osal_void hal_set_txop_check_cca(osal_u8 txop_check_cca)
{
    hal_public_hook_func(_set_txop_check_cca)(txop_check_cca);
}

/*****************************************************************************
 函 数 名  : hal_vap_set_machw_txop_limit_vivo
 功能描述  : 设置VI、VO的TXOP上限(单位: 微秒)
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_machw_txop_limit_vivo(osal_u16 vo, osal_u16 vi)
{
    hal_public_hook_func(_vap_set_machw_txop_limit_vivo)(vo, vi);
}

/*****************************************************************************
 函 数 名  : hal_vap_get_machw_txop_limit_vivo
 功能描述  : 获取VI、VO的TXOP上限(单位: 微秒)
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_machw_txop_limit_vivo(osal_u16 *vo, osal_u16 *vi)
{
    hal_public_hook_func(_vap_get_machw_txop_limit_vivo)(vo, vi);
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 函 数 名  : hal_vap_set_machw_edca_bkbe_lifetime
 功能描述  : 设置BK、BE的生存时限
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_machw_edca_bkbe_lifetime(osal_u16 be, osal_u16 bk)
{
    hal_public_hook_func(_vap_set_machw_edca_bkbe_lifetime)(be, bk);
}

/*****************************************************************************
 函 数 名  : hal_vap_get_machw_edca_bkbe_lifetime
 功能描述  : 获取BK、BE的生存时限
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_machw_edca_bkbe_lifetime(osal_u16 *be, osal_u16 *bk)
{
    hal_public_hook_func(_vap_get_machw_edca_bkbe_lifetime)(be, bk);
}

/*****************************************************************************
 函 数 名  : hal_vap_set_machw_edca_vivo_lifetime
 功能描述  : 设置VI、VO的生存时限
*****************************************************************************/
static INLINE__ osal_void  hal_vap_set_machw_edca_vivo_lifetime(osal_u16 vo, osal_u16 vi)
{
    hal_public_hook_func(_vap_set_machw_edca_vivo_lifetime)(vo, vi);
}

/*****************************************************************************
 函 数 名  : hal_vap_get_machw_edca_vivo_lifetime
 功能描述  : 获取VI、VO的生存时限
*****************************************************************************/
static INLINE__ osal_void  hal_vap_get_machw_edca_vivo_lifetime(osal_u16 *vo, osal_u16 *vi)
{
    hal_public_hook_func(_vap_get_machw_edca_vivo_lifetime)(vo, vi);
}
#endif

/*****************************************************************************
 函 数 名  : hal_set_sta_bssid
 功能描述  : 设置BSSID
*****************************************************************************/
static  INLINE__ osal_void hal_set_sta_bssid(hal_to_dmac_vap_stru *hal_vap, osal_u8 *byte, osal_u16 byte_len)
{
    unref_param(byte_len);
    hal_public_hook_func(_set_sta_bssid)(hal_vap, byte);
}

static INLINE__ osal_void hal_init_pm_info(hal_to_dmac_vap_stru *hal_vap)
{
    hal_public_hook_func(_init_pm_info)(hal_vap);
}

static INLINE__ osal_void hal_init_pm_info_sync(hal_to_dmac_vap_stru *hal_vap)
{
    hal_public_hook_func(_init_pm_info_sync)(hal_vap);
}

static INLINE__ osal_u16 *hal_pm_get_ext_inner_offset_diff(osal_void)
{
    return hal_public_hook_func(_pm_get_ext_inner_offset_diff)();
}

/* 设置管理帧，组播 广播数据帧除基本信息外的所有其他描述符字段 */
static INLINE__ osal_void hal_tx_non_ucast_data_set_dscr(hal_to_dmac_device_stru *hal_device,
    hal_tx_dscr_stru *tx_dscr, hal_tx_txop_feature_stru *txop_feature, hal_tx_txop_alg_stru *txop_alg,
    hal_tx_ppdu_feature_stru *ppdu_feature)
{
    hal_public_hook_func(_tx_non_ucast_data_set_dscr)(hal_device, tx_dscr, txop_feature, txop_alg, ppdu_feature);
}

/*****************************************************************************
 函 数 名  : hal_get_rate_80211g_table
 功能描述  : 获取80211g速率
*****************************************************************************/
static  INLINE__ osal_void hal_get_rate_80211g_table(const hal_to_dmac_device_stru *hal_device,
    const mac_data_rate_stru **rate)
{
    unref_param(hal_device);
    hal_public_hook_func(_get_rate_80211g_table)(rate);
}

/*****************************************************************************
 函 数 名  : hal_get_rate_80211g_num
 功能描述  : 获取80211g速率个数
*****************************************************************************/
static INLINE__ osal_void hal_get_rate_80211g_num(const hal_to_dmac_device_stru *hal_device, osal_u32 *data_num)
{
    unref_param(hal_device);
    hal_public_hook_func(_get_rate_80211g_num)(data_num);
}

/*****************************************************************************
 功能描述  : 设置重放攻击检测开关
*****************************************************************************/
static INLINE__ osal_void hal_set_replay_detect_en(osal_u8 enable)
{
    hal_public_hook_func(_set_replay_detect_en)(enable);
}
 
/*****************************************************************************
 功能描述  : 获取重放攻击检测开关
*****************************************************************************/
static INLINE__ osal_u8 hal_get_replay_detect_en(osal_void)
{
    return hal_public_hook_func(_get_replay_detect_en)();
}

osal_void hal_dump_phy_reg(osal_void);
osal_void hal_dump_mac_reg(osal_void);
osal_void hal_dump_stats(osal_void);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
/************************************ *****************************************
 功能描述  : 动态校准统计信息
*****************************************************************************/
osal_u64 hal_get_dyn_cali_send_pack_num(osal_void);
osal_void hal_set_dyn_cali_send_pack_num(osal_u64 value);
#endif
osal_void hal_pow_sync_tpc_code_to_dmac(hal_to_dmac_device_stru *hal_device, osal_u8 vap_id);
osal_u32 hal_pow_read_machw_resp_power_lut_entry(osal_u8 lut_index);
osal_u8 hal_save_machw_phy_pa_status(hal_to_dmac_device_stru *hal_device_base);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_ext_rom.h */
