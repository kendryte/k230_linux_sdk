/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: tx beamforming algorithm
 */

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "alg_txbf.h"

#include "hal_device.h"

#include "hmac_feature_dft.h"
#include "oal_netbuf_data.h"
#include "frw_thread.h"
#include "alg_gla.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_TXBF_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
osal_u8 g_txbf_assoc_user;
osal_u8 g_txbf_last_rate;
/*****************************************************************************
  3 函数实现
*****************************************************************************/
OSAL_STATIC osal_void alg_txbf_init_bfee(osal_void);
OSAL_STATIC osal_u8 *alg_txbf_get_assoc_user_num(osal_void);
OSAL_STATIC osal_void alg_txbf_report_set_pow_rate(hal_to_dmac_device_stru *hal_device, osal_u8 rate);
OSAL_STATIC osal_u32 alg_txbf_add_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_u32 alg_txbf_del_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user);
OSAL_STATIC osal_u32 alg_txbf_rx_data_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf, hal_rx_statistic_stru *rx_stats);

osal_u8 *alg_txbf_get_assoc_user_num(osal_void)
{
    return &g_txbf_assoc_user;
}

/******************************************************************************
函 数 名  :alg_txbf_print_gla_info
功能描述  : TXBF算法图形化维测打印
 ******************************************************************************/
OSAL_STATIC osal_void alg_txbf_print_gla_info(osal_u8 rate)
{
    oal_bool_enum_uint8 switch_enable = alg_host_get_gla_switch_enable(ALG_GLA_ID_TXBF, ALG_GLA_USUAL_SWITCH);
    osal_u8 *assoc_user_num = alg_txbf_get_assoc_user_num();
    /* 仅单用户场景进行打印 */
    if ((switch_enable == OSAL_TRUE) && (*assoc_user_num == 1)) {
        oam_warning_log1(0, OAM_SF_TXBF, "[GLA][ALG][TXBF]:rate=%d", rate);
    }
}

osal_void alg_txbf_report_set_pow_rate(hal_to_dmac_device_stru *hal_device, osal_u8 rate)
{
    /* 速率发生变化时填写寄存器 */
    if (g_txbf_last_rate != rate) {
        hal_set_bf_rate(rate);
        hmac_bfee_report_pow_adjust_notify(hal_device);
        alg_txbf_print_gla_info(rate);
        g_txbf_last_rate = rate;
    }
}

OSAL_STATIC osal_u8 alg_txbf_get_rate_from_rssi(osal_s8 rssi)
{
    if (rssi > ALG_TXBF_NEAR_RSSI) {
        return WLAN_HE_MCS5;
    } else if (rssi < ALG_TXBF_FAR_RSSI) {
        return WLAN_HE_MCS0;
    } else if ((rssi < ALG_TXBF_NEAR_RSSI - ALG_TXBF_RSSI_GAP) && (rssi > ALG_TXBF_FAR_RSSI + ALG_TXBF_RSSI_GAP)) {
        return WLAN_HE_MCS3;
    } else {
        return g_txbf_last_rate;
    }
}

/******************************************************************************
 函 数 名  : alg_txbf_rx_data_process
 功能描述  : rx描述符判断是否需要提升action no ack的速率和功率
*****************************************************************************/
osal_u32 alg_txbf_rx_data_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buf, hal_rx_statistic_stru *rx_stats)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    oal_bool_enum_uint8 co_intf;
    hal_alg_intf_det_mode_enum_uint8 adj_intf;
    osal_u8 rate;
    osal_s8 rssi;
    osal_u8 *assoc_user_num = alg_txbf_get_assoc_user_num();

    /* 入参检查 */
    if (osal_unlikely(alg_is_null_ptr4(hmac_vap, hmac_user, buf, rx_stats))) {
        oam_warning_log0(0, OAM_SF_TXBF, "alg_txbf_rx_data_process::null pointers");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 检查hal device是否为空 */
    hal_device = hmac_vap->hal_device;
    if (osal_unlikely(alg_is_null_ptr1(hal_device))) {
        oam_warning_log1(0, OAM_SF_TXBF, "alg_txbf_rx_data_process::vap id[%d]hal device is null!", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (*assoc_user_num != hal_device->assoc_user_nums) {
        oam_warning_log2(0, OAM_SF_TXBF, "alg_txbf_rx_data_process::txbf user[%d] device user[%d] not eq!",
            *assoc_user_num, hal_device->assoc_user_nums);
    }

    /* 非单用户场景在删减用户中处理 */
    if (*assoc_user_num != 1) {
        return OAL_SUCC;
    }

    rssi = rx_stats->rssi_dbm;

    /* 获取当前的同频/邻叠频干扰状态 */
#ifdef _PRE_WLAN_FEATURE_INTF_DET
    co_intf = alg_intf_det_get_curr_coch_intf_type(hal_device);
    adj_intf = alg_intf_det_get_curr_adjch_intf_type(hal_device);
    if (osal_unlikely((co_intf == OAL_BUTT) || (adj_intf == HAL_ALG_INTF_DET_ADJINTF_BUTT))) {
        oam_warning_log0(0, OAM_SF_TXBF, "alg_txbf_rx_data_process::get co intf or adj intf fail!");
        /* 未成功获取当前干扰类型，默认存在干扰，恢复低速发送 */
        alg_txbf_report_set_pow_rate(hal_device, WLAN_HE_MCS0);
        return OAL_FAIL;
    }

    /* 根据用户距离的远近和当前的干扰状态决策是否需要开启高速发送action no ack帧 */
    if ((co_intf == OSAL_FALSE) &&
        ((adj_intf == HAL_ALG_INTF_DET_ADJINTF_NO) || (adj_intf == HAL_ALG_INTF_DET_STATE_PK))) {
        rate = alg_txbf_get_rate_from_rssi(rssi);
    } else {
        rate = WLAN_HE_MCS0;
    }

    alg_txbf_report_set_pow_rate(hal_device, rate);
#endif
    return OAL_SUCC;
}

osal_u32 alg_txbf_add_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u8 *assoc_user_num = alg_txbf_get_assoc_user_num();
    /* 入参检查 */
    if (osal_unlikely(alg_is_null_ptr3(hmac_vap, hmac_user, hmac_vap->hal_device))) {
        oam_warning_log0(0, OAM_SF_TXBF, "alg_txbf_add_user_process::null pointers!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 组播用户不处理 */
    if (hmac_user->is_multi_user == OSAL_TRUE) {
        return OAL_SUCC;
    }

    if ((++(*assoc_user_num)) != 1) {
        /* 多用户场景下，寄存器配置低速发送 */
        alg_txbf_report_set_pow_rate(hmac_vap->hal_device, WLAN_HE_MCS0);
    }

    return OAL_SUCC;
}

osal_u32 alg_txbf_del_user_process(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user)
{
    osal_u8 *assoc_user_num = alg_txbf_get_assoc_user_num();
    /* 入参检查 */
    if (osal_unlikely(alg_is_null_ptr3(hmac_vap, hmac_user, hmac_vap->hal_device))) {
        oam_warning_log0(0, OAM_SF_TXBF, "alg_txbf_del_user_process::null pointers!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 组播用户不处理 */
    if (hmac_user->is_multi_user == OSAL_TRUE) {
        return OAL_SUCC;
    }

    if ((--(*assoc_user_num)) == 0) {
        /* 去关联了最后一个用户，寄存器恢复默认配置 */
        alg_txbf_report_set_pow_rate(hmac_vap->hal_device, WLAN_HE_MCS0);
    }

    return OAL_SUCC;
}

/******************************************************************************
 函 数 名  : alg_hmac_txbf_init
 功能描述  : Txbf模块初始化，包括注册算法私有结构体和算法函数钩子
*****************************************************************************/
osal_s32 alg_hmac_txbf_init(osal_void)
{
    g_txbf_assoc_user = 0;
    g_txbf_last_rate = WLAN_HE_MCS_BUTT;
    /* bfee功能初始化 */
    alg_txbf_init_bfee();
    hmac_alg_register_rx_notify_func(HMAC_ALG_RX_TXBF, alg_txbf_rx_data_process);
    hmac_alg_register_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_TXBF, alg_txbf_add_user_process);
    hmac_alg_register_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_TXBF, alg_txbf_del_user_process);

    return OAL_SUCC;
}

/******************************************************************************
函 数 名  : alg_txbf_init_bfee
功能描述  : 初始化bfee能力
******************************************************************************/
osal_void alg_txbf_init_bfee(osal_void)
{
    osal_u8  min_group = 0;
    osal_u8  codebook = 1;

    /* 仅实现he bfee功能，he bfee初始化的部分 */
    /* 矩阵地址和空间分配, 73和63不同vap使用不同的地址，92不同vap使用相同的地址 */
    hal_set_txbf_he_buff_addr((osal_u32)(uintptr_t)(osal_u8 *)OAL_MEM_ADDR_TXBFEE_BUFF, WLAN_TXBFEE_BUFF_SIZE);
    /* 设置group,codebook信息 */
    hal_set_bfee_grouping_codebook(codebook, min_group);
    /* 是否使能反馈sounding矩阵 */
    hal_set_bfee_sounding_en(OSAL_TRUE);
}

/*****************************************************************************
 函 数 名  : alg_hmac_txbf_exit
 功能描述  : Txbf模块退出
             1)注销各种钩子
             2)释放空间
*****************************************************************************/
osal_void alg_hmac_txbf_exit(osal_void)
{
    hmac_alg_unregister_rx_notify_func(HMAC_ALG_RX_TXBF);
    hmac_alg_unregister_add_user_notify_func(HMAC_ALG_ADD_USER_NOTIFY_TXBF);
    hmac_alg_unregister_del_user_notify_func(HMAC_ALG_DEL_USER_NOTIFY_TXBF);
}

#endif /* #ifdef _PRE_WLAN_FEATURE_TXBF */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
