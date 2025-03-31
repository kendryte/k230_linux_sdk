/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: fe_equipment.c 的头文件
 */
#ifndef __FE_EQUIPMENT_H__
#define __FE_EQUIPMENT_H__

#include "wlan_types.h"
#include "mac_vap_ext.h"
#include "cali_online_common.h"
#include "fe_rf_customize_power_cali.h"
#include "hmac_mfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_MFG_TEST

#define FE_MFG_POW_PARAM_NUM         2 /* 测试功率点的数量 */
#define FE_MFG_POLYNOMIAL_NUM        3 /* 拟合曲线参数个数 */
#define FE_MFG_RSSI_COMP_2G_NUM      3 /* RSSI数量 与band划分对应 */

typedef struct {
    osal_s16 inst_power[FE_MFG_POW_PARAM_NUM]; /* 仪器测量功率值 由装备获取后填入 */
    osal_s16 exp_power[FE_MFG_POW_PARAM_NUM]; /* 目标功率 */
    osal_s16 chip_pdet[FE_MFG_POW_PARAM_NUM]; /* 芯片反馈电压 由功率校准阶段读取并写入 */
    osal_s16 power_curve[FE_CUS_POW_BUTT][FE_MFG_POLYNOMIAL_NUM]; /* 曲线参数 */
    osal_s16 curve_factor[FE_CUS_POW_BUTT][FE_MFG_POLYNOMIAL_NUM]; /* 曲线放大系数 */
} fe_mfg_power_ipa_para_stru;
typedef struct {
    osal_u8 save_flag; /* 0b11111 表示5个参数都已经保存 可以进行曲线拟合 11b情况下是0b111 */
    osal_u8 curve_idx;
    osal_u8 chain;
    osal_u8 protocol;
    osal_u8 tar_tpc[FE_MFG_POW_PARAM_NUM]; /* 保存tpc code */
    oal_bool_enum_uint8 curve_cal_mode; /* 曲线校准模式 0:关 1:开 在配置目标tpc code开启 计算完曲线参数关闭 */
    osal_u8 initialized_flag; /* 首次使用为0, 非首次使用为1 */
    fe_mfg_power_ipa_para_stru ipa_para;
} fe_mfg_power_cali_para_stru;
typedef struct {
    osal_s8 rssi_comp[FE_MFG_RSSI_COMP_2G_NUM];
    osal_u8 select_subband; /* bit0~2表示设置subband0~2, bit4~6表示获取subband0~2 */
} fe_mfg_rssi_comp_2g_stru;

fe_mfg_power_cali_para_stru *fe_mfg_get_cali_para(osal_void);
osal_void fe_mfg_tx_pow_save_equip_param(online_cali_para_stru *online_cali,
    osal_s16 pdet_cal, osal_s16 exp_pow);
osal_s32 fe_mfg_power_set_curve_factor(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_power_get_curve_factor(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_power_set_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_power_get_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_power_set_low_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_power_get_low_curve_param(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_power_set_tar_power(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_power_set_tar_cali_power(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_rssi_set_rssi_offset(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
osal_s32 fe_mfg_rssi_get_rssi_offset(hmac_vap_stru *hmac_vap, mfg_param_stru *mfg_param);
#endif // _PRE_WLAN_FEATURE_MFG_TEST
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __FE_EQUIPMENT_H__