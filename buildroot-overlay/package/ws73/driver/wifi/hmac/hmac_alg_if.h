/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: HMAC to ALG的API接口文件。
 * Create: 2020-7-3
 */

#ifndef __HMAC_ALG_IF_H__
#define __HMAC_ALG_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_alg_if_struct_define.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  10 函数声明
*****************************************************************************/
/***************** 子算法注册注销接口 ***************/
osal_u32  hmac_alg_register(hmac_alg_id_enum_uint32 alg_id);
osal_u32  hmac_alg_unregister(hmac_alg_id_enum_uint32 alg_id);
oal_bool_enum_uint8 hmac_alg_is_registered(hmac_alg_id_enum_uint32 alg_id);
hmac_alg_stru* hmac_alg_get_callback_stru(void);
/***************** 回调函数注册接口 ********************/
osal_u32  hmac_alg_register_tx_notify_func(hmac_alg_tx_notify_enum_uint8    notify_sub_type,
    p_alg_tx_notify_func             func);
osal_u32  hmac_alg_register_rx_notify_func(hmac_alg_rx_notify_enum_uint8    notify_sub_type,
    p_alg_rx_notify_func             func);
osal_u32  hmac_alg_register_cfg_channel_notify_func(hmac_alg_cfg_channel_notify_enum_uint8  notify_sub_type,
    p_alg_cfg_channel_notify_func      func);
osal_u32  hmac_alg_register_add_user_notify_func(hmac_alg_add_user_notify_enum_uint8 notify_sub_type,
    p_alg_add_assoc_user_notify_func     func);
osal_u32  hmac_alg_register_del_user_notify_func(hmac_alg_del_user_notify_enum_uint8 notify_sub_type,
    p_alg_delete_assoc_user_notify_func     func);
osal_u32  hmac_alg_register_tx_schedule_func(p_alg_tx_schedule_func  func);
osal_u32  hmac_alg_register_tx_schedule_timer_func(p_alg_tx_schedule_timer_func func);
osal_u32  hmac_alg_register_tid_update_notify_func(p_alg_update_tid_notify_func func);
osal_u32  hmac_alg_register_user_info_update_notify_func(p_alg_update_user_info_notify_func func);
osal_u32  hmac_alg_register_rx_mgmt_notify_func(hmac_alg_rx_mgmt_notify_enum_uint8 notify_sub_type,
    p_alg_rx_mgmt_notify_func func);
osal_void hmac_alg_register_tbtt_notify_func(p_alg_anti_intf_tbtt_notify_func func);
osal_u32 hmac_alg_register_para_cfg_notify_func(alg_param_cfg_notify_enum_uint8 notify_sub_type,
    p_alg_para_cfg_notify_func func);
osal_u32  hmac_alg_register_scan_param_notify(p_alg_scan_param_func func);
osal_u32  hmac_alg_register_scan_ch_complete_notify(p_alg_intf_det_scan_chn_cb_func func);
osal_u32  hmac_alg_register_vap_up_notify_func(hmac_alg_vap_up_notify_enum_uint8 notify_sub_type,
    p_alg_vap_up_notify_func func);
osal_u32  hmac_alg_register_vap_down_notify_func(hmac_alg_vap_down_notify_enum_uint8 notify_sub_type,
    p_alg_vap_down_notify_func func);
osal_u32  hmac_alg_register_pow_table_refresh_notify_func(p_alg_pow_table_refresh_notify_func func);
osal_u32  hmac_alg_register_update_rssi_level_notify_func(p_alg_update_cb_rssi_level_notify_func func);
osal_u32  hmac_alg_register_para_sync_notify_func(alg_param_sync_notify_enum notify_sub_type,
    p_alg_para_sync_notify_func func);
osal_u32  hmac_alg_register_txbf_pow_update_notify_func(p_alg_bfee_report_pow_adjust_notify func);
osal_u32  hmac_alg_register_distance_notify_func(alg_distance_notify_enum_uint8 notify_sub_type,
    p_alg_distance_notify_func func);

/***************** 回调函数注销接口 ********************/
osal_void hmac_alg_unregister_tbtt_notify_func(void);
osal_u32  hmac_alg_unregister_tx_notify_func(hmac_alg_tx_notify_enum_uint8    notify_sub_type);
osal_u32  hmac_alg_unregister_rx_notify_func(hmac_alg_rx_notify_enum_uint8    notify_sub_type);
osal_u32  hmac_alg_unregister_cfg_channel_notify_func(hmac_alg_cfg_channel_notify_enum_uint8 notify_sub_type);
osal_u32  hmac_alg_unregister_add_user_notify_func(hmac_alg_add_user_notify_enum_uint8 notify_sub_type);
osal_u32  hmac_alg_unregister_del_user_notify_func(hmac_alg_del_user_notify_enum_uint8 notify_sub_type);
osal_u32  hmac_alg_unregister_tx_schedule_func(void);
osal_u32  hmac_alg_unregister_tx_schedule_timer_func(void);
osal_u32  hmac_alg_unregister_tid_update_notify_func(void);
osal_u32  hmac_alg_unregister_user_info_update_notify_func(void);
osal_u32  hmac_alg_unregister_para_cfg_notify_func(alg_param_cfg_notify_enum_uint8 notify_sub_type);
osal_u32  hmac_alg_unregister_rx_mgmt_notify_func(hmac_alg_rx_mgmt_notify_enum_uint8 notify_sub_type);
osal_u32  hmac_alg_unregister_scan_param_notify(void);
osal_u32  hmac_alg_unregister_scan_ch_complete_notify(void);
osal_u32  hmac_alg_unregister_vap_up_notify_func(hmac_alg_vap_up_notify_enum_uint8 notify_sub_type);
osal_u32  hmac_alg_unregister_vap_down_notify_func(hmac_alg_vap_down_notify_enum_uint8 notify_sub_type);
osal_u32  hmac_alg_unregister_pow_table_refresh_notify_func(osal_void);
osal_u32  hmac_alg_unregister_update_rssi_level_notify_func(osal_void);
osal_u32  hmac_alg_unregister_para_sync_notify_func(alg_param_sync_notify_enum notify_sub_type);
osal_u32  hmac_alg_unregister_txbf_pow_update_notify_func(osal_void);
osal_u32  hmac_alg_unregister_distance_notify_func(alg_distance_notify_enum_uint8 notify_sub_type);

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
osal_u32 hmac_alg_register_anti_intf_switch_notify(p_alg_anti_intf_switch_func func);
osal_u32 hmac_alg_unregister_anti_intf_switch_notify(void);
osal_u32 hmac_alg_register_anti_intf_tx_time_notify(p_alg_anti_intf_tx_time_notify_func func);
osal_u32 hmac_alg_unregister_anti_intf_tx_time_notify(osal_void);
osal_u32 hmac_alg_anti_intf_get_tx_time(hal_to_dmac_device_stru *hal_device);
#endif

/***************** 注册数据结构接口 *********************/
osal_u32  hmac_alg_register_device_priv_stru(const hal_to_dmac_device_stru *hal_dev,
    hal_alg_device_stru_id_enum_uint8 dev_stru_type, osal_void *dev_stru);
osal_u32  hmac_alg_register_user_priv_stru(const hmac_user_stru *hmac_user,
    hmac_alg_user_stru_id_enum_uint8 user_stru_type, osal_void *user_stru);
osal_u32  hmac_alg_register_tid_priv_stru(hmac_user_stru *hmac_user,
    osal_u8 tid_no, hmac_alg_tid_stru_id_enum_uint8 tid_stru_type, osal_void *tid_stru);
/***************** 注销数据结构接口 *******************/
osal_u32  hmac_alg_unregister_device_priv_stru(const hal_to_dmac_device_stru *hal_dev,
    hal_alg_device_stru_id_enum_uint8 dev_stru_type);
osal_u32  hmac_alg_unregister_user_priv_stru(const hmac_user_stru *hmac_user,
    hmac_alg_user_stru_id_enum_uint8  user_stru_type);
osal_u32  hmac_alg_unregister_tid_priv_stru(const hmac_user_stru *hmac_user,
    osal_u8 tid_no, hmac_alg_tid_stru_id_enum_uint8 tid_stru_type);

/***************** 获取数据结构接口 *******************/
osal_u32  hmac_alg_get_device_priv_stru(const hal_to_dmac_device_stru  *hal_dev,
    hal_alg_device_stru_id_enum_uint8 dev_stru_type, osal_void **pp_dev_stru);
osal_u32  hmac_alg_get_user_priv_stru(const hmac_user_stru * const hmac_user,
    hmac_alg_user_stru_id_enum_uint8 user_stru_type, osal_void **pp_user_stru);
osal_u32  hmac_alg_get_tid_priv_stru(const hmac_user_stru * const hmac_user,
    osal_u8 tid_no, hmac_alg_tid_stru_id_enum_uint8 tid_stru_type, osal_void **pp_tid_stru);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of hmac_alg_if.h */
