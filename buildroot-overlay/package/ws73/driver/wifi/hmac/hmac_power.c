/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: 无。
 * Create: 2020-07-03
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_power.h"
#include "hmac_alg_notify.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_POWER_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

OSAL_STATIC osal_void hmac_pow_set_vap_tx_power_ext(const hmac_vap_stru *hmac_vap, hal_pow_set_type_enum_uint8 type)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
        // 刷新TB功率寄存器 初始化时刷新映射表 初始化或更新vap都需要刷新tb各速率功率表
        if (type == HAL_POW_SET_TYPE_INIT) {
            /* 初始化tb mcs tx power */
            hal_tb_tx_power_init(hmac_vap->channel.band);
        }
        /* 初始化HE TB PPDU发送功率表 */
        hal_tb_mcs_tx_power(hmac_vap->channel.band);
    }
#else
    unref_param(hmac_vap);
    unref_param(type);
#endif
}

/*****************************************************************************
 函 数 名  : hmac_pwr_device_init
 功能描述  : 上下电流程中host device_stru的初始化函数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_host_dev_init_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru     *hmac_device;
    osal_u32           loop;

    unref_param(msg);

    if (hmac_vap == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_device_init_etc:: hmac_device NULL pointer!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "{hmac_device_init_etc:: hmac_device[%d] NULL pointer!}",
            hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (loop = 0; loop < MAC_MAX_SUPP_CHANNEL; loop++) {
        hmac_device->ap_channel_list[loop].num_networks = 0;
        hmac_device->ap_channel_list[loop].ch_type      = MAC_CH_TYPE_NONE;
    }
    /* 补充上下电时候需要初始化的hmac_device_stru下的信息 */
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_host_dev_exit_etc
 功能描述  : 下电流程中host device_stru的去初始化函数
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_host_dev_exit_etc(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    unref_param(hmac_vap);
    unref_param(msg);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_pow_set_vap_tx_power
 功能描述  : vap侧初始化tx power
*****************************************************************************/
osal_void hmac_pow_set_vap_tx_power(hmac_vap_stru *hmac_vap, hal_pow_set_type_enum_uint8 type)
{
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    hal_vap_pow_info_stru *vap_pow_info = OSAL_NULL;
    osal_u8 cur_ch_num;
    wlan_channel_band_enum_uint8 freq_band;

    if (hmac_vap == OSAL_NULL) {
        return;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] {hmac_pow_set_vap_tx_power::hal_device null}", hmac_vap->vap_id);
        return;
    }

    /* 获取vap级别功率信息结构体 */
    vap_pow_info = &hmac_vap->vap_pow_info;
    hmac_vap->vap_pow_info.debug_flag = (type == HAL_POW_SET_TYPE_INIT) ?
        OSAL_FALSE : hmac_vap->vap_pow_info.debug_flag;

    /* 获取信道号，带宽，2G/5G */
    cur_ch_num = hmac_vap->channel.chan_number;
    freq_band = hmac_vap->channel.band;
    /* 功率RF寄存器控时不作功率更新 */
    if (hal_device->pow_rf_reg_ctl_flag == OSAL_TRUE) {
        /* 若是切信道，需要更新寄存器配置 */
        if (type == HAL_POW_SET_TYPE_REFRESH) {
            oam_warning_log1(0, 0, "vap_id[%d] {hmac_pow_set_vap_tx_power::refresh rf enable!}", hmac_vap->vap_id);

            hal_pow_set_rf_regctl_enable(hal_device, hal_device->pow_rf_reg_ctl_flag);
        }
        return;
    }

    if ((type == HAL_POW_SET_TYPE_INIT) || (type == HAL_POW_SET_TYPE_REFRESH)) {
        /* 获取国家码最大功率, 不区分速率 */
        mac_regclass_info_stru *regdom_info = hmac_get_channel_num_rc_info_etc(freq_band, cur_ch_num);
        /* 针对可能调用此钩子函数的地方，对应国家码的信道无效时做保护 */
        if (osal_unlikely(regdom_info == OSAL_NULL)) {
            /* 取MAX_Power = MIN[MAX_Power@11k, MAX_Power@TEST, MAX_Power@REG] */
            /* 默认取管制域最大发送功率 */
            vap_pow_info->reg_pow = MAC_RC_DEFAULT_MAX_TX_PWR * HAL_POW_PRECISION_SHIFT;
            oam_warning_log4(0, OAM_SF_PWR, "{hmac_pow_set_vap_tx_power::chn do not support band[%u] ch_num[%u] \
                reg_pow[%d] tx_power[%d]}", freq_band, cur_ch_num, vap_pow_info->reg_pow, hmac_vap->tx_power);
        } else {
            /* 取SAR标准功率限制值、所处管制类限制功率、管制域实际最大功率三者中较小值 */
            /* 增加配置最大发射功率的约束 */
            vap_pow_info->reg_pow = osal_min(regdom_info->max_tx_pwr, regdom_info->max_reg_tx_pwr *
                HAL_POW_PRECISION_SHIFT);
        }
#ifndef _PRE_BSLE_GATEWAY
        /* 初始化pow code表单 */
        hal_device_init_vap_pow_code(hal_device, vap_pow_info, type, &hmac_vap->channel);
        hmac_alg_pow_table_refresh_notify(hmac_vap);
        hal_pow_sync_tpc_code_to_dmac(hal_device, hmac_vap->vap_id);
#endif
    }

    hmac_pow_set_vap_tx_power_ext(hmac_vap, type);
}
// AT+SETRPWR and set_cal_rpwr 实现接口 在原有速率的功率值上 补偿功率
osal_s32 hmac_power_set_rate_power_offset(hmac_vap_stru *hmac_vap, const frw_msg *msg)
{
    osal_u32 msg_data;
    osal_u8 max_rate;
    osal_u8 min_rate;
    osal_s8 power_offset;
    osal_u8 offset = 0x8;   // 偏移8位
    osal_u8 freq_band;
    osal_u8 index;

    msg_data = *(osal_u32 *)msg->data;
    power_offset = (osal_s8)(msg_data & 0xff);
    msg_data = msg_data >> offset;
    min_rate = (msg_data & 0xff);
    msg_data = msg_data >> offset;
    max_rate = (msg_data & 0xff);

    freq_band = hmac_vap->channel.band;
    for (index = min_rate; index <= max_rate; index++) {
        hal_power_set_rate_power_offset(freq_band, index, power_offset);
    }
    // 调用刷新vap的速率功率表
    hmac_pow_set_vap_tx_power(hmac_vap, HAL_POW_SET_TYPE_REFRESH);
    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

