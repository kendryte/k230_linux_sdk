/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac algorithm
 * Create: 2020-7-8
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_alg_notify.h"
#include "frw_timer.h"
#include "wlan_msg.h"
#include "hmac_user.h"
#include "hmac_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ALG_NOTIFY_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 算法框架主结构体 */
hmac_alg_stru g_hmac_alg_main;
/* 注册算法主结构体钩子, 钩子作为实际引用对象 */
hmac_alg_stru *g_hmac_algorithm_main = &g_hmac_alg_main;

osal_u8 g_dbac_cur_state = OSAL_FALSE;
hal_alg_device_stru g_hal_alg_device_stru;
hmac_alg_stru *hmac_alg_get_callback_stru(void)
{
    return g_hmac_algorithm_main;
}
/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_alg_unregister_tx_notify_func
 功能描述  : 子算法注册回调函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_tx_notify_func(hmac_alg_tx_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->pa_tx_notify_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_rx_notify_func
 功能描述  : 子算法注册回调函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_rx_notify_func(hmac_alg_rx_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->pa_rx_notify_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_tx_schedule_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_tx_schedule_func(osal_void)
{
    g_hmac_algorithm_main->tx_schedule_func = OSAL_NULL;

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_alg_unregister_tx_schedule_timer_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_tx_schedule_timer_func(osal_void)
{
    g_hmac_algorithm_main->tx_schedule_timer_func = OSAL_NULL;

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_alg_unregister_tid_update_notify_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_tid_update_notify_func(osal_void)
{
    g_hmac_alg_main.tid_update_func = OSAL_NULL;

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_alg_unregister_user_info_update_notify_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_user_info_update_notify_func(osal_void)
{
    g_hmac_alg_main.user_info_update_func = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_cfg_channel_notify_func
 功能描述  : 子算法注册设置信道回调函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_cfg_channel_notify_func(hmac_alg_cfg_channel_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->pa_cfg_channel_notify_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_add_user_notify_func
 功能描述  : 子算法注册填加关联用户的回调函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_del_user_notify_func(hmac_alg_del_user_notify_enum_uint8 notify_sub_type)
{
    g_hmac_alg_main.pa_delete_assoc_user_notify_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_add_user_notify_func
 功能描述  : 子算法注册填加去关联用户的回调函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_add_user_notify_func(hmac_alg_add_user_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->pa_add_assoc_user_notify_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_alg_unregister_rx_mgmt_notify_func
 功能描述  : 去注册子算法功能中对接收管理帧处理函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_rx_mgmt_notify_func(hmac_alg_rx_mgmt_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->rx_mgmt_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_cfg_btcoex_state_notify
 功能描述  : 处理sta模式发起关联
*****************************************************************************/
osal_u32 hmac_alg_cfg_btcoex_state_notify(hal_to_dmac_device_stru *hal_device, hmac_alg_bt_aggr_time_uint8 type)
{
    hmac_alg_stru *alg_stru = OSAL_NULL;
    osal_u8 index;

    alg_stru = g_hmac_algorithm_main;

    /* 调用相关回调函数 */
    for (index = 0; index < HMAC_ALG_CFG_BTCOEX_STATE_NOTIFY_BUTT; index++) {
        if (alg_stru->pa_cfg_btcoex_state_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_cfg_btcoex_state_notify_func[index](hal_device, type);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_free_tid_priv_stru
 功能描述  : 释放user下的tid私有数据结构
*****************************************************************************/
OSAL_STATIC osal_void hmac_alg_free_tid_priv_stru(hmac_user_stru *hmac_user)
{
    osal_u8 index;

    for (index = 0; index < WLAN_TID_MAX_NUM; index++) {
        if (hmac_user->tx_tid_queue[index].alg_priv == OSAL_NULL) {
            break;
        }

        oal_mem_free(hmac_user->tx_tid_queue[index].alg_priv, OAL_TRUE);

        hmac_user->tx_tid_queue[index].alg_priv = OSAL_NULL;
    }
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_tx_notify_func
 功能描述  : 子算法注册回调函数
*****************************************************************************/
osal_u32 hmac_alg_register_tx_notify_func(hmac_alg_tx_notify_enum_uint8 notify_sub_type, p_alg_tx_notify_func func)
{
    g_hmac_algorithm_main->pa_tx_notify_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_rx_notify_func
 功能描述  : 子算法注册回调函数
*****************************************************************************/
osal_u32 hmac_alg_register_rx_notify_func(hmac_alg_rx_notify_enum_uint8 notify_sub_type, p_alg_rx_notify_func func)
{
    g_hmac_algorithm_main->pa_rx_notify_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_tx_schedule_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_register_tx_schedule_func(p_alg_tx_schedule_func func)
{
    g_hmac_algorithm_main->tx_schedule_func = func;

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_alg_register_tx_schedule_timer_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_register_tx_schedule_timer_func(p_alg_tx_schedule_timer_func func)
{
    g_hmac_algorithm_main->tx_schedule_timer_func = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_user_info_update_notify_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_register_user_info_update_notify_func(p_alg_update_user_info_notify_func func)
{
    g_hmac_algorithm_main->user_info_update_func = func;

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_alg_register_tid_update_notify_func
 功能描述  : 注册调度函数
*****************************************************************************/
osal_u32 hmac_alg_register_tid_update_notify_func(p_alg_update_tid_notify_func func)
{
    g_hmac_algorithm_main->tid_update_func = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_cfg_channel_notify_func
 功能描述  : 子算法注册设置信道回调函数
*****************************************************************************/
osal_u32 hmac_alg_register_cfg_channel_notify_func(hmac_alg_cfg_channel_notify_enum_uint8 notify_sub_type,
    p_alg_cfg_channel_notify_func func)
{
    g_hmac_algorithm_main->pa_cfg_channel_notify_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_add_user_notify_func
 功能描述  : 子算法注册填加关联用户的回调函数
*****************************************************************************/
osal_u32 hmac_alg_register_add_user_notify_func(hmac_alg_add_user_notify_enum_uint8 notify_sub_type,
    p_alg_add_assoc_user_notify_func func)
{
    g_hmac_algorithm_main->pa_add_assoc_user_notify_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_del_user_notify_func
 功能描述  : 子算法注册填加关联用户的回调函数
*****************************************************************************/
osal_u32 hmac_alg_register_del_user_notify_func(hmac_alg_del_user_notify_enum_uint8 notify_sub_type,
    p_alg_delete_assoc_user_notify_func func)
{
    g_hmac_algorithm_main->pa_delete_assoc_user_notify_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_tbtt_notify_func
 功能描述  : 注册txbf/抗干扰算法功能中对接收管理帧处理函数
*****************************************************************************/
osal_void hmac_alg_register_tbtt_notify_func(p_alg_anti_intf_tbtt_notify_func func)
{
    g_hmac_algorithm_main->anti_intf_tbtt_func = func;

    return;
}

/*****************************************************************************
 函 数 名  : hmac_alg_anti_intf_tbtt_handler
 功能描述  : 弱干扰免疫使能开/关
*****************************************************************************/
osal_u32 hmac_alg_anti_intf_tbtt_handler(hmac_vap_stru *hmac_vap, hal_to_dmac_device_stru *hal_device)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANTI_INTF, "{hmac_alg_anti_intf_tbtt_handler:: ERROR INFO: hal_device=%p.}",
            (uintptr_t)hal_device);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (osal_unlikely(g_hmac_algorithm_main->anti_intf_tbtt_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_hmac_algorithm_main->anti_intf_tbtt_func(hmac_vap, hal_device);
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_tbtt_notify_func
 功能描述  : 注册txbf/抗干扰算法功能中对接收管理帧处理函数
*****************************************************************************/
osal_void hmac_alg_unregister_tbtt_notify_func(osal_void)
{
    g_hmac_algorithm_main->anti_intf_tbtt_func = OSAL_NULL;

    return;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_rx_mgmt_notify_func
 功能描述  : 注册txbf/抗干扰算法功能中对接收管理帧处理函数
*****************************************************************************/
osal_u32 hmac_alg_register_rx_mgmt_notify_func(hmac_alg_rx_mgmt_notify_enum_uint8 notify_sub_type,
    p_alg_rx_mgmt_notify_func func)
{
    g_hmac_algorithm_main->rx_mgmt_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_vap_up_notify_func
 功能描述  : vap up回调函数注册
*****************************************************************************/
osal_u32 hmac_alg_register_vap_up_notify_func(hmac_alg_vap_up_notify_enum_uint8 notify_sub_type,
    p_alg_vap_up_notify_func func)
{
    g_hmac_algorithm_main->pa_alg_vap_up_notify_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_vap_up_notify_func
 功能描述  : vap up回调函数注销
*****************************************************************************/
osal_u32 hmac_alg_unregister_vap_up_notify_func(hmac_alg_vap_up_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->pa_alg_vap_up_notify_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_vap_down_notify_func
 功能描述  : vap down回调函数注册
*****************************************************************************/
osal_u32 hmac_alg_register_vap_down_notify_func(hmac_alg_vap_down_notify_enum_uint8 notify_sub_type,
    p_alg_vap_down_notify_func func)
{
    g_hmac_algorithm_main->pa_alg_vap_down_notify_func[notify_sub_type] = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_vap_down_notify_func
 功能描述  : vap up回调函数注销
*****************************************************************************/
osal_u32 hmac_alg_unregister_vap_down_notify_func(hmac_alg_vap_down_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->pa_alg_vap_down_notify_func[notify_sub_type] = OSAL_NULL;

    return OAL_SUCC;
}


/*****************************************************************************
 功能描述  : 功率表更新通知TPC更新参数
*****************************************************************************/
osal_u32 hmac_alg_register_pow_table_refresh_notify_func(p_alg_pow_table_refresh_notify_func func)
{
    g_hmac_algorithm_main->alg_pow_table_refresh_notify_func = func;
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  : 功率表更新通知函数注销
******************************************************************************/
osal_u32 hmac_alg_unregister_pow_table_refresh_notify_func(osal_void)
{
    g_hmac_algorithm_main->alg_pow_table_refresh_notify_func = OSAL_NULL;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  :cb更新 通知tpc算法更新cb->rssi_level
*****************************************************************************/
osal_u32 hmac_alg_register_update_rssi_level_notify_func(p_alg_update_cb_rssi_level_notify_func func)
{
    g_hmac_algorithm_main->alg_update_cb_rssi_level_notify_func = func;
    return OAL_SUCC;
}

/******************************************************************************
  功能描述  :cb更新 通知tpc算法更新cb->rssi_level函数注销
******************************************************************************/
osal_u32 hmac_alg_unregister_update_rssi_level_notify_func(osal_void)
{
    g_hmac_algorithm_main->alg_update_cb_rssi_level_notify_func = OSAL_NULL;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_device_priv_stru
 功能描述  : 注册设备级别的结构体
*****************************************************************************/
osal_u32 hmac_alg_register_device_priv_stru(const hal_to_dmac_device_stru *hal_dev,
    hal_alg_device_stru_id_enum_uint8 dev_stru_type, osal_void *dev_stru)
{
    if (osal_unlikely((hal_dev == OSAL_NULL) || (dev_stru == OSAL_NULL) ||
        (dev_stru_type >= HAL_ALG_DEVICE_STRU_ID_BUTT))) {
        oam_error_log3(0, OAM_SF_ANY,
            "{hmac_alg_register_device_priv_stru:: ERROR INFO: hal_dev=%p, dev_stru=%p, dev_stru_type=%d.}",
            (uintptr_t)hal_dev, (uintptr_t)dev_stru, dev_stru_type);
        return OAL_FAIL;
    }

    g_hal_alg_device_stru.alg_info[dev_stru_type] = dev_stru;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_device_priv_stru
 功能描述  : 注册设备级别的结构体
*****************************************************************************/
osal_u32 hmac_alg_unregister_device_priv_stru(const hal_to_dmac_device_stru *hal_dev,
    hal_alg_device_stru_id_enum_uint8 dev_stru_type)
{
    if (osal_unlikely((hal_dev == OSAL_NULL) || (dev_stru_type >= HAL_ALG_DEVICE_STRU_ID_BUTT))) {
        oam_error_log2(0, OAM_SF_ANY,
            "{hmac_alg_unregister_device_priv_stru:: ERROR INFO: hal_dev=%p, dev_stru_type=%d.}",
            (uintptr_t)hal_dev, dev_stru_type);
        return OAL_FAIL;
    }

    g_hal_alg_device_stru.alg_info[dev_stru_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_user_priv_stru
 功能描述  : 注册USER私有数据结构体
****************************************************************************/
osal_u32 hmac_alg_register_user_priv_stru(const hmac_user_stru *hmac_user,
    hmac_alg_user_stru_id_enum_uint8 user_stru_type, osal_void *user_stru)
{
    hmac_alg_user_stru *alg_info = OSAL_NULL;

    if ((hmac_user == OSAL_NULL) || (user_stru == OSAL_NULL) || (user_stru_type >= HMAC_ALG_USER_STRU_ID_BUTT)) {
        oam_error_log3(0, OAM_SF_ANY,
            "{hmac_alg_register_user_priv_stru:: ERROR INFO: user=%p, user_stru=%p, user_stru_type=%d.}",
            (uintptr_t)hmac_user, (uintptr_t)user_stru, user_stru_type);

        return OAL_ERR_CODE_PTR_NULL;
    }

    alg_info = (hmac_alg_user_stru *)hmac_user->alg_priv;

    if (osal_unlikely(alg_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_alg_register_user_priv_stru:: alg_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    alg_info->alg_info[user_stru_type] = user_stru;
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_user_priv_stru
 功能描述  : 注册USER私有数据结构体
*****************************************************************************/
osal_u32 hmac_alg_unregister_user_priv_stru(const hmac_user_stru *hmac_user,
    hmac_alg_user_stru_id_enum_uint8 user_stru_type)
{
    hmac_alg_user_stru *alg_info = OSAL_NULL;

    if (osal_unlikely((hmac_user == OSAL_NULL) || (user_stru_type >= HMAC_ALG_USER_STRU_ID_BUTT))) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_alg_unregister_user_priv_stru:: ERROR INFO:user=%p, user_stru_type=%d.}",
            (uintptr_t)hmac_user, user_stru_type);

        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_info = (hmac_alg_user_stru *)hmac_user->alg_priv;

    if (osal_unlikely(alg_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "ERROR INFO: p_alg_priv is NULL PTR");

        return OAL_ERR_CODE_PTR_NULL;
    }

    alg_info->alg_info[user_stru_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_tid_priv_stru
 功能描述  : 注册TID私有数据结构体
*****************************************************************************/
osal_u32 hmac_alg_register_tid_priv_stru(hmac_user_stru *hmac_user, osal_u8 tid_no,
    hmac_alg_tid_stru_id_enum_uint8 tid_stru_type, osal_void *tid_stru)
{
    hmac_alg_tid_stru *alg_info = OSAL_NULL;

    if (osal_unlikely((hmac_user == OSAL_NULL) || (tid_stru == OSAL_NULL) || (tid_no >= WLAN_TID_MAX_NUM) ||
        (tid_stru_type >= HMAC_ALG_TID_STRU_ID_BUTT))) {
        oam_error_log4(0, OAM_SF_ANY,
            "{hmac_alg_register_tid_priv_stru::user=%p,tid_stru=%p,tid_no=%d, tid_stru_type=%d.}",
            (uintptr_t)hmac_user, (uintptr_t)tid_stru, tid_no, tid_stru_type);

        return OAL_FAIL;
    }

    alg_info = (hmac_alg_tid_stru *)hmac_user->tx_tid_queue[tid_no].alg_priv;

    if (osal_unlikely(alg_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "ERROR INFO:alg_info is NULL PTR.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    alg_info->alg_info[tid_stru_type] = tid_stru;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_tid_priv_stru
 功能描述  : 注册TID私有数据结构体
*****************************************************************************/
osal_u32 hmac_alg_unregister_tid_priv_stru(const hmac_user_stru *hmac_user, osal_u8 tid_no,
    hmac_alg_tid_stru_id_enum_uint8 tid_stru_type)
{
    hmac_alg_tid_stru *alg_info = OSAL_NULL;

    if (osal_unlikely((hmac_user == OSAL_NULL) || (tid_no >= WLAN_TID_MAX_NUM) ||
        (tid_stru_type >= HMAC_ALG_TID_STRU_ID_BUTT))) {
        oam_error_log3(0, OAM_SF_ANY,
            "{hmac_alg_unregister_tid_priv_stru:: ERROR INFO: user=%p, tid_no=%d, tid_stru_type=%d.}",
            (uintptr_t)hmac_user, tid_no, tid_stru_type);

        return OAL_FAIL;
    }

    alg_info = (hmac_alg_tid_stru *)hmac_user->tx_tid_queue[tid_no].alg_priv;

    if (osal_unlikely(alg_info == OSAL_NULL)) {
        oam_error_log3(0, 0, "vap_id[%d] alg_info is NULL PTR, user idx = %d, tid no = %d.", hmac_user->vap_id,
            hmac_user->assoc_id, tid_no);

        return OAL_ERR_CODE_PTR_NULL;
    }

    alg_info->alg_info[tid_stru_type] = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_get_device_priv_stru
 功能描述  : 获取TID下的私有数据结构体
*****************************************************************************/
WIFI_TCM_TEXT osal_u32 hmac_alg_get_device_priv_stru(const hal_to_dmac_device_stru *hal_dev,
    hal_alg_device_stru_id_enum_uint8 dev_stru_type, osal_void **pp_dev_stru)
{
    if (osal_unlikely((hal_dev == OSAL_NULL) || (pp_dev_stru == OSAL_NULL) ||
        (dev_stru_type >= HAL_ALG_DEVICE_STRU_ID_BUTT))) {
        oam_error_log3(0, OAM_SF_ANY,
            "{hmac_alg_get_device_priv_stru:: ERROR INFO: hal_dev=%p, dev_stru=%p, dev_stru_type=%d.}",
            (uintptr_t)hal_dev, (uintptr_t)pp_dev_stru, dev_stru_type);

        return OAL_FAIL;
    }

    *pp_dev_stru = g_hal_alg_device_stru.alg_info[dev_stru_type];
    if (osal_unlikely(*pp_dev_stru == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_alg_get_device_priv_stru::dev_stru null, type=%d}", dev_stru_type);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_get_user_priv_stru
 功能描述  : 获取USER私有数据结构体
*****************************************************************************/
WIFI_TCM_TEXT osal_u32 hmac_alg_get_user_priv_stru(const hmac_user_stru * const hmac_user,
    hmac_alg_user_stru_id_enum_uint8 user_stru_type, osal_void **pp_user_stru)
{
    hmac_alg_user_stru *alg_info = OSAL_NULL;

    if (osal_unlikely((pp_user_stru == OSAL_NULL) || (user_stru_type >= HMAC_ALG_USER_STRU_ID_BUTT) ||
        (hmac_user == OSAL_NULL))) {
        oam_warning_log2(0, OAM_SF_ANY,
            "{hmac_alg_get_user_priv_stru:: ERROR INFO: pp_user_stru=%d, user_stru_type=%d.}",
            pp_user_stru == OSAL_NULL, user_stru_type);
        return OAL_FAIL;
    }

    alg_info = (hmac_alg_user_stru *)hmac_user->alg_priv;

    if (osal_unlikely(alg_info == OSAL_NULL)) {
        if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
            oam_warning_log2(0, OAM_SF_ANY,
                "{hmac_alg_get_user_priv_stru:: assoc_id=%d, user_asoc_state=%d}",
                hmac_user->assoc_id, hmac_user->user_asoc_state);
        }

        return OAL_ERR_CODE_PTR_NULL;
    }

    *pp_user_stru = alg_info->alg_info[user_stru_type];

    if (osal_unlikely(*pp_user_stru == OSAL_NULL)) {
        oam_warning_log3(0, OAM_SF_ANY,
            "vap_id[%d] {hmac_alg_get_user_priv_stru::alg priv user_stru is null, type=%d, user idx = %d}",
            hmac_user->vap_id, user_stru_type, hmac_user->assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_get_tid_priv_stru
 功能描述  : 获取TID下的私有数据结构体
*****************************************************************************/
WIFI_TCM_TEXT osal_u32 hmac_alg_get_tid_priv_stru(const hmac_user_stru * const hmac_user, osal_u8 tid_no,
    hmac_alg_tid_stru_id_enum_uint8 tid_stru_type, osal_void **pp_tid_stru)
{
    hmac_alg_tid_stru *alg_info = OSAL_NULL;

    if (osal_unlikely((pp_tid_stru == OSAL_NULL) || (tid_no >= WLAN_TID_MAX_NUM) ||
        (tid_stru_type >= HMAC_ALG_TID_STRU_ID_BUTT))) {
        oam_error_log3(0, OAM_SF_ANY,
            "{hmac_alg_get_tid_priv_stru:: ERROR INFO: pp_tid_stru=%p, tid_no=%d, tid_stru_type=%d.}",
            (uintptr_t)pp_tid_stru, tid_no, tid_stru_type);

        return OAL_FAIL;
    }

    alg_info = (hmac_alg_tid_stru *)hmac_user->tx_tid_queue[tid_no].alg_priv;
    if (osal_unlikely(alg_info == OSAL_NULL)) {
        if (hmac_user->user_asoc_state != MAC_USER_STATE_ASSOC) {
            oam_warning_log2(0, OAM_SF_ANY, "{hmac_alg_get_tid_priv_stru:alg_priv null ptr.user state = %d,user_id=%d}",
                hmac_user->user_asoc_state, hmac_user->assoc_id);
        }

        return OAL_ERR_CODE_PTR_NULL;
    }

    *pp_tid_stru = alg_info->alg_info[tid_stru_type];

    if (osal_unlikely(*pp_tid_stru == OSAL_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_alg_get_tid_priv_stru::pp_tid_stru is null, tid_stru_type=%d}",
            tid_stru_type);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_cfg_channel_notify
 功能描述  : 处理设置信道
*****************************************************************************/
osal_u32 hmac_alg_cfg_channel_notify(hmac_vap_stru *hmac_vap, mac_alg_channel_bw_chg_type_uint8 type)
{
    hmac_alg_stru *alg_stru;
    osal_u8 index;
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_alg_cfg_channel_notify:: ERROR INFO: vap=%p.}",
            (uintptr_t)hmac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    alg_stru = g_hmac_algorithm_main;

    for (index = 0; index < HMAC_ALG_CFG_CHANNEL_NOTIFY_BUTT; index++) {
        if (alg_stru->pa_cfg_channel_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_cfg_channel_notify_func[index](hmac_vap, type);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_add_assoc_user_notify
 功能描述  : 当DMAC添加一个关联用户时，会调用该函数
*****************************************************************************/
osal_u32 hmac_alg_add_assoc_user_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_alg_user_stru *user_info = OSAL_NULL;
    hmac_alg_tid_stru *tid_info = OSAL_NULL;
    hmac_alg_stru *alg_stru = OSAL_NULL;
    osal_u8 index;
    osal_u8 loop;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "OAL_ERR_CODE_PTR_NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_user->alg_priv != OSAL_NULL) {
        hmac_alg_del_assoc_user_notify(hmac_vap, hmac_user);
    }

    /* 挂接用户级别的数据结构 */
    user_info = (hmac_alg_user_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_alg_user_stru), OAL_TRUE);
    if (osal_unlikely(user_info == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY,
            "hmac_alg_add_assoc_user_notify: alloc mem fail, hmac_alg_user_stru, pool id is OAL_MEM_POOL_ID_LOCAL");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    for (index = 0; index < HMAC_ALG_USER_STRU_ID_BUTT; index++) {
        user_info->alg_info[index] = OSAL_NULL;
    }

    hmac_user->alg_priv = user_info;

    /* 挂接TID级别的数据结构 */
    for (index = 0; index < WLAN_TID_MAX_NUM; index++) {
        tid_info = (hmac_alg_tid_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_alg_tid_stru), OAL_TRUE);
        if (osal_unlikely(tid_info == OSAL_NULL)) {
            oam_error_log0(0, OAM_SF_ANY,
                "hmac_alg_add_assoc_user_notify: alloc mem fail, hmac_alg_tid_stru, pool id is OAL_MEM_POOL_ID_LOCAL");
            oal_mem_free((osal_void *)user_info, OAL_TRUE);
            hmac_user->alg_priv = OSAL_NULL;

            hmac_alg_free_tid_priv_stru(hmac_user);

            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        for (loop = 0; loop < HMAC_ALG_TID_STRU_ID_BUTT; loop++) {
            tid_info->alg_info[loop] = OSAL_NULL;
        }

        hmac_user->tx_tid_queue[index].alg_priv = tid_info;
    }

    /* 调用相关回调函数 */
    alg_stru = g_hmac_algorithm_main;

    for (index = 0; index < HMAC_ALG_ADD_USER_NOTIFY_BUTT; index++) {
        if (alg_stru->pa_add_assoc_user_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_add_assoc_user_notify_func[index](hmac_vap, hmac_user);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : alg_notify_del_assoc_user
 功能描述  : 当DMAC删除一个关联用户时，会调用该函数
*****************************************************************************/
osal_u32 hmac_alg_del_assoc_user_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    hmac_alg_user_stru *user_info = OSAL_NULL;
    hmac_alg_stru *alg_stru = OSAL_NULL;
    osal_u8 index;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL))) {
        oam_error_log0(0, OAM_SF_ANY, "OAL_ERR_CODE_PTR_NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (hmac_user->alg_priv == OSAL_NULL) {
        return OAL_SUCC;
    }

    /* 调用相关回调函数 */
    alg_stru = g_hmac_algorithm_main;

    for (index = 0; index < HMAC_ALG_DEL_USER_NOTIFY_BUTT; index++) {
        if (alg_stru->pa_delete_assoc_user_notify_func[index] != OSAL_NULL) {
            alg_stru->pa_delete_assoc_user_notify_func[index](hmac_vap, hmac_user);
        }
    }

    /* 释放TID级别的数据结构 */
    hmac_alg_free_tid_priv_stru(hmac_user);

    /* 释放用户级别的数据结构 */
    user_info = hmac_user->alg_priv;

    if (user_info != OSAL_NULL) {
        oal_mem_free(user_info, OAL_TRUE);
        hmac_user->alg_priv = OSAL_NULL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_rx_mgmt_notify
 功能描述  : 将接收管理帧通知算法txbf/抗干扰模块
*****************************************************************************/
osal_u32 hmac_alg_rx_mgmt_notify(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *buf)
{
    hmac_alg_stru *alg_stru = OSAL_NULL;
    osal_u8 index;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (buf == OSAL_NULL))) {
        oam_error_log2(0, OAM_SF_ANY, "{hmac_alg_rx_mgmt_notify:: ERROR INFO: vap=%p, buf=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)buf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    alg_stru = g_hmac_algorithm_main;

    for (index = 0; index < HMAC_ALG_RX_MGMT_NOTIFY_BUTT; index++) {
        if (alg_stru->rx_mgmt_func[index] != OSAL_NULL) {
            alg_stru->rx_mgmt_func[index](hmac_vap, hmac_user, buf);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_rx_cntl_notify
 功能描述  : 将接收控制帧通知算法模块
*****************************************************************************/
osal_u32 hmac_alg_rx_cntl_notify(const hmac_vap_stru *hmac_vap, const hmac_user_stru *hmac_user,
    const oal_netbuf_stru *buf)
{
    hmac_alg_stru *alg_stru = OSAL_NULL;
    osal_u8 index;

    if (osal_unlikely((hmac_vap == OSAL_NULL) || (hmac_user == OSAL_NULL) || (buf == OSAL_NULL))) {
        oam_error_log3(0, OAM_SF_ANY,
            "{hmac_alg_rx_cntl_notify:: ERROR INFO: vap=%p, user=%p, buf=%p.}",
            (uintptr_t)hmac_vap, (uintptr_t)hmac_user, (uintptr_t)buf);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    alg_stru = g_hmac_algorithm_main;

    for (index = 0; index < HMAC_ALG_RX_CNTL_NOTIFY_BUTT; index++) {
        if (alg_stru->rx_cntl_func[index] != OSAL_NULL) {
            alg_stru->rx_cntl_func[index](hmac_vap, hmac_user, buf);
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register
 功能描述  : 算法注册
*****************************************************************************/
osal_u32 hmac_alg_register(hmac_alg_id_enum_uint32 alg_id)
{
    g_hmac_algorithm_main->alg_bitmap |= alg_id;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_register
 功能描述  : 算法注销
*****************************************************************************/
osal_u32 hmac_alg_unregister(hmac_alg_id_enum_uint32 alg_id)
{
    g_hmac_algorithm_main->alg_bitmap &= (~alg_id);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 查看算法是否注册
*****************************************************************************/
oal_bool_enum_uint8 hmac_alg_is_registered(hmac_alg_id_enum_uint32 alg_id)
{
    if ((g_hmac_algorithm_main->alg_bitmap & alg_id) == alg_id) {
        return OSAL_TRUE;
    } else {
        return OSAL_FALSE;
    }
}

#ifdef _PRE_WLAN_FEATURE_DBAC
/*****************************************************************************
 函 数 名: hmac_alg_dbac_pause
 功能描述  : 暂停DBAC
*****************************************************************************/
osal_s32 hmac_alg_dbac_pause(hmac_vap_stru *hmac_vap)
{
    osal_s32 ret;
    frw_msg msg = {0};

    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_dbac_cur_state = OSAL_TRUE;
    /* 抛事件至Device侧DMAC，同步删除VAP */
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_DBAC_PAUSE, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_alg_dbac_pause::frw_send_msg_to_device failed[%d].}", ret);
        return ret;
    }

    return ret;
}

/*****************************************************************************
 函 数 名: hmac_alg_dbac_resume
 功能描述  : 恢复DBAC
*****************************************************************************/
osal_s32 hmac_alg_dbac_resume(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 is_resume_channel)
{
    osal_s32 ret;
    frw_msg msg = {0};

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    msg.data = (osal_u8 *)&is_resume_channel;
    msg.data_len = (osal_u16)sizeof(is_resume_channel);
    g_dbac_cur_state = OSAL_FALSE;

    /* 抛事件至Device侧DMAC，同步删除VAP */
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_DEVICE_DBAC_RESUME, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_alg_dbac_resume::frw_send_msg_to_device failed[%d].}", ret);
        return ret;
    }

    return ret;
}

/*****************************************************************************
 函 数 名: hmac_alg_dbac_send_disassoc
 功能描述  : 发送去关联帧的保护
*****************************************************************************/
osal_s32 hmac_alg_dbac_send_disassoc(hmac_vap_stru *mac_vap, osal_u8 is_send)
{
    osal_s32 ret;
    frw_msg msg = {0};

    if (osal_unlikely(mac_vap == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    msg.data = (osal_u8 *)&is_send;
    msg.data_len = (osal_u16)sizeof(is_send);
    /* 抛事件至Device侧DMAC，同步删除VAP */
    ret = frw_send_msg_to_device(mac_vap->vap_id, WLAN_MSG_H2D_C_CFG_DEVICE_DBAC_DISASSOC_SEND, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_alg_dbac_send_disassoc:send_msg_to_device failed[%d].}", ret);
        return ret;
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_alg_dbac_is_pause
 功能描述  : 暂停DBAC
*****************************************************************************/
oal_bool_enum_uint8 hmac_alg_dbac_is_pause(hal_to_dmac_device_stru *hal_device)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_dbac_cur_state;
}
#endif

osal_void hmac_alg_vap_up_hmac_to_dmac(hmac_vap_stru *mac_vap)
{
    osal_s32 ret;
    frw_msg msg = {0};

    /* 抛事件至Device侧alg */
    ret = frw_send_msg_to_device(mac_vap->vap_id, WLAN_MSG_H2D_C_CFG_ALG_VAP_UP, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_CFG, "{frw_send_msg_to_device:: vap up [%d].}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_scan_param_notify
 功能描述  : 注册扫描参数同步到dmac
*****************************************************************************/
osal_u32 hmac_alg_register_scan_param_notify(p_alg_scan_param_func func)
{
    g_hmac_algorithm_main->scan_param_func = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_scan_param_notify
 功能描述  : 去注册扫描参数同步dmac
*****************************************************************************/
osal_u32 hmac_alg_unregister_scan_param_notify(osal_void)
{
    g_hmac_algorithm_main->scan_param_func = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_scan_param_sync
 功能描述  : 扫描参数同步
*****************************************************************************/
osal_u32 hmac_scan_param_sync(const mac_scan_req_stru *scan_req_params)
{
    if (osal_unlikely(scan_req_params == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANTI_INTF, "{hmac_scan_param_sync:: ERROR INFO: scan_req_params=%p.}",
            (uintptr_t)scan_req_params);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (osal_unlikely(g_hmac_algorithm_main->scan_param_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_hmac_algorithm_main->scan_param_func(scan_req_params);
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_scan_ch_complete_notify
 功能描述  : 注册扫描参数同步到dmac
*****************************************************************************/
osal_u32 hmac_alg_register_scan_ch_complete_notify(p_alg_intf_det_scan_chn_cb_func func)
{
    g_hmac_algorithm_main->scan_comp_cb_func = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_unregister_scan_param_notify
 功能描述  : 去注册扫描参数同步dmac
*****************************************************************************/
osal_u32 hmac_alg_unregister_scan_ch_complete_notify(osal_void)
{
    g_hmac_algorithm_main->scan_comp_cb_func = OSAL_NULL;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_scan_ch_complete_notify
 功能描述  : 扫描参数同步
*****************************************************************************/
osal_u32 hmac_alg_scan_ch_complete_notify(osal_void *param)
{
    if (osal_unlikely(g_hmac_algorithm_main->scan_comp_cb_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_hmac_algorithm_main->scan_comp_cb_func(param);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
/*****************************************************************************
 函 数 名  : hmac_alg_register_anti_intf_switch_notify
 功能描述  : 注册弱干扰免疫开/关配置回调
*****************************************************************************/
osal_u32 hmac_alg_register_anti_intf_switch_notify(p_alg_anti_intf_switch_func func)
{
    g_hmac_algorithm_main->anti_intf_switch_func = func;

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_alg_unregister_anti_intf_switch_notify
 功能描述  : 注册弱干扰免疫开/关配置回调
*****************************************************************************/
osal_u32 hmac_alg_unregister_anti_intf_switch_notify(osal_void)
{
    g_hmac_algorithm_main->anti_intf_switch_func = OSAL_NULL;

    return OAL_SUCC;
}
/*****************************************************************************
 函 数 名  : hmac_alg_anti_intf_switch
 功能描述  : 弱干扰免疫使能开/关
*****************************************************************************/
osal_u32 hmac_alg_anti_intf_switch(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 alg_enable)
{
    oal_bool_enum is_registered;
    /* 如果算法没被注册，直接返回 */
    is_registered = hmac_alg_is_registered(HMAC_ALG_ID_ANTI_INTF);
    if (osal_unlikely(is_registered == OSAL_FALSE)) {
        return OAL_SUCC;
    }

    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANTI_INTF, "{hmac_alg_anti_intf_switch:: ERROR INFO: hal_device=%p.}",
            (uintptr_t)hal_device);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (osal_unlikely(g_hmac_algorithm_main->anti_intf_switch_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_hmac_algorithm_main->anti_intf_switch_func(hal_device, alg_enable);
}

/*****************************************************************************
 函 数 名  : hmac_alg_register_anti_intf_tx_time_notify
 功能描述  : 注册弱干扰免疫tx time通知函数
*****************************************************************************/
osal_u32 hmac_alg_register_anti_intf_tx_time_notify(p_alg_anti_intf_tx_time_notify_func func)
{
    g_hmac_algorithm_main->anti_intf_tx_time_notify_func = func;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_alg_anti_intf_get_tx_time
 功能描述  : tx_time通知
*****************************************************************************/
osal_u32 hmac_alg_anti_intf_get_tx_time(hal_to_dmac_device_stru *hal_device)
{
    if (osal_unlikely(hal_device == OSAL_NULL)) {
        oam_error_log1(0, OAM_SF_ANTI_INTF, "{hmac_alg_anti_intf_get_tx_time:: ERROR INFO: hal_device=%p.}",
            (uintptr_t)hal_device);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (osal_unlikely(g_hmac_algorithm_main->anti_intf_tx_time_notify_func == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_hmac_algorithm_main->anti_intf_tx_time_notify_func(hal_device);
}


/*****************************************************************************
 函 数 名  : hmac_alg_unregister_anti_intf_tx_time_notify
 功能描述  : 去注册弱干扰免疫tx time通知函数
*****************************************************************************/
osal_u32 hmac_alg_unregister_anti_intf_tx_time_notify(osal_void)
{
    g_hmac_algorithm_main->anti_intf_tx_time_notify_func = OSAL_NULL;

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  :  注册算法参数同步钩子函数
*****************************************************************************/
osal_u32 hmac_alg_register_para_sync_notify_func(alg_param_sync_notify_enum notify_sub_type,
    p_alg_para_sync_notify_func func)
{
    g_hmac_algorithm_main->alg_para_sync_notify_func[notify_sub_type] = func;
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  :  算法参数同步钩子函数注销
******************************************************************************/
osal_u32 hmac_alg_unregister_para_sync_notify_func(alg_param_sync_notify_enum notify_sub_type)
{
    g_hmac_algorithm_main->alg_para_sync_notify_func[notify_sub_type] = OSAL_NULL;
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 功能描述  :  注册算法参数配置钩子函数
*****************************************************************************/
osal_u32 hmac_alg_register_para_cfg_notify_func(alg_param_cfg_notify_enum_uint8 notify_sub_type,
    p_alg_para_cfg_notify_func func)
{
    g_hmac_algorithm_main->alg_para_cfg_notify_func[notify_sub_type] = func;
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  :  算法参数配置钩子函数注销
******************************************************************************/
osal_u32 hmac_alg_unregister_para_cfg_notify_func(alg_param_cfg_notify_enum_uint8 notify_sub_type)
{
    g_hmac_algorithm_main->alg_para_cfg_notify_func[notify_sub_type] = OSAL_NULL;
    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  :  txbf report帧更新功率通知
*****************************************************************************/
osal_u32 hmac_alg_register_txbf_pow_update_notify_func(p_alg_bfee_report_pow_adjust_notify func)
{
    g_hmac_algorithm_main->bfee_report_pow_adjust_notify = func;
    return OAL_SUCC;
}

/******************************************************************************
 功能描述  :  txbf report帧更新功率通知注销
******************************************************************************/
osal_u32 hmac_alg_unregister_txbf_pow_update_notify_func(osal_void)
{
    g_hmac_algorithm_main->bfee_report_pow_adjust_notify = OSAL_NULL;
    return OAL_SUCC;
}

WIFI_HMAC_TCM_TEXT osal_void *hmac_alg_get_algorithm_main(osal_void)
{
    return (osal_void *)g_hmac_algorithm_main;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
