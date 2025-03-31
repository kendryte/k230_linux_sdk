/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 产测相关命令接口
 */

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_mfg.h"
#include "hmac_ccpriv.h"
#include "wlan_msg.h"
#include "soc_diag_wdk.h"
#include "fe_equipment.h"
#include "hmac_alg_config.h"
#include "hal_mfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_MFG_C

static osal_u8 g_mfg_mode = 0;

OAL_STATIC osal_s32 hmac_ccpriv_set_mfg_mode(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 value;
    osal_u8 mfg_mode;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_ccpriv_set_mfg_mode:: hmac_vap is null ptr}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = hmac_ccpriv_get_one_arg_digit(&param, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_mfg_mode::get mfg_mode return err_code [%d]!}", ret);
        wifi_printf_always("ERROR\n");
        return ret;
    }

    if (value < 0 || value > 1) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }
    mfg_mode = (osal_u8)value;
    hal_set_mfg_mode(mfg_mode);

    g_mfg_mode = mfg_mode;
    if (g_mfg_mode == 1) {
        ret = hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"tpc_mode 0");
        ret |= hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"aggr_enable 0");
        ret |= hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"edca_opt_en_sta 0");
    } else {
        ret = hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"tpc_mode 1");
        ret |= hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"aggr_enable 1");
        ret |= hmac_ccpriv_alg_cfg_etc(hmac_vap, (const osal_s8 *)"edca_opt_en_sta 1");
    }
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_ccpriv_set_mfg_mode::hmac_ccpriv_alg_cfg_etc return err_code [%d]!}", ret);
        wifi_printf_always("ERROR\n");
        return ret;
    }
    wifi_printf_always("OK\n");
    wifi_printf_always("mfg mode:%d\n", g_mfg_mode);

    return ret;
}

osal_u8 hmac_get_mfg_mode(osal_void)
{
    return g_mfg_mode;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_mfg_status(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    frw_init_enum_uint16 init_state = frw_get_init_state_etc();

    unref_param(hmac_vap);
    unref_param(param);
    if (init_state == FRW_INIT_STATE_ALL_SUCC) {
        wifi_printf_always("WiFi OK\n");
        return OAL_SUCC;
    }
    wifi_printf_always("ERROR\n");
    return OAL_FAIL;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_al_rx_rssi(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s16 rssi = 0;
    osal_u8 rx_loss = 0;
    frw_msg msg;

    unref_param(param);
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_PWR, "{hmac_ccpriv_get_al_rx_rssi:: hmac_vap is null ptr}");
        wifi_printf_always("ERROR\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    (osal_void)memset_s(&msg, sizeof(frw_msg), 0, sizeof(frw_msg));
    frw_msg_init(OSAL_NULL, 0, (osal_u8 *)&rssi, sizeof(rssi), &msg);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_GET_AL_RX_RSSI, &msg, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    rx_loss = hal_get_custom_rx_insert_loss(hmac_vap->channel.band, hmac_vap->channel.chan_idx);
    rssi += rx_loss;
    if (rssi > oal_get_real_rssi(OAL_RSSI_INIT_MARKER)) {
        rssi = oal_get_real_rssi(OAL_RSSI_INIT_MARKER);
    }
    wifi_printf_always("OK\r\n");
    wifi_printf_always("rssi: %d\r\n", rssi);
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_rssi_offset(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 channel;
    osal_s32 rssi_offset;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_MIN_CHANNEL_NUM, WLAN_CFG_MAX_CHANNEL_NUM, &channel);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rssi_offset::get rssi_offset return err_code [%d]!}", ret);
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_MIN_RSSI_OFFSET, WLAN_CFG_MAX_RSSI_OFFSET, &rssi_offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rssi_offset::get rssi_offset return err_code [%d]!}", ret);
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    mfg_param.param[0] = channel;
    mfg_param.param[1] = rssi_offset;

    ret = fe_mfg_rssi_set_rssi_offset(hmac_vap, &mfg_param);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_set_rssi_offset::fe_mfg_rssi_set_rssi_offset failed[%d].}", ret);
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_rssi_offset(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 channel;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_MIN_CHANNEL_NUM, WLAN_CFG_MAX_CHANNEL_NUM, &channel);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_set_rssi_offset::get rssi_offset return err_code [%d]!}", ret);
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    mfg_param.param[0] = channel;

    ret = fe_mfg_rssi_get_rssi_offset(hmac_vap, &mfg_param);
    if (ret != OAL_SUCC || mfg_param.output_param_num != 1) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_ccpriv_get_rssi_offset::fe_mfg_rssi_get_rssi_offset failed[%d].}", ret);
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    wifi_printf_always("OK\r\n");
    wifi_printf_always("rssi offset: %d\n", mfg_param.output_param[0]);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_curve_factor(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 flag;
    osal_u8 index;
    osal_s32 curve_factor;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    ret = hmac_ccpriv_get_one_arg_digit(&param, &flag);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    if (flag < MAC_CURVE_FACTOR_DEFAULT || flag > MAC_LOW_CURVE_FACTOR) {
        wifi_printf_always("ERROR\r\n");
        return OAL_FAIL;
    }

    mfg_param.param[0] = flag;
    if (flag == MAC_HIGH_CURVE_FACTOR || flag == MAC_LOW_CURVE_FACTOR) {
        for (index = 1; index <= WLAN_CFG_ONE_CURVE_FACTOR_NUM; index++) {
            ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_CURVE_FACTOR_MIN, WLAN_CFG_CURVE_FACTOR_MAX,
                &curve_factor);
            if (ret != OAL_SUCC) {
                wifi_printf_always("ERROR\r\n");
                return ret;
            }
            mfg_param.param[index] = curve_factor;
        }
    }

    ret = fe_mfg_power_set_curve_factor(hmac_vap, &mfg_param);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    wifi_printf_always("OK\r\n");
    return ret;
}

OAL_STATIC osal_void hmac_mfg_print_output_param(mfg_param_stru *mfg_param)
{
    osal_u8 index;
    for (index = 0; (index < mfg_param->output_param_num) && (index < WLAN_CFG_MAX_PARAM_NUM); index++) {
        wifi_printf_always("%4d ", mfg_param->output_param[index]);
    }
    wifi_printf_always("\r\n");
}

OAL_STATIC osal_s32 hmac_ccpriv_get_curve_factor(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    mfg_param_stru mfg_param;

    unref_param(param);
    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    ret = fe_mfg_power_get_curve_factor(hmac_vap, &mfg_param);
    if (ret != OAL_SUCC || mfg_param.output_param_num != WLAN_CFG_CURVE_FACTOR_NUM) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    hmac_mfg_print_output_param(&mfg_param);
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_tar_power(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 value;
    osal_u8 index;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    for (index = 0; index < WLAN_CFG_TES_POWER_NUM; index++) {
        ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_MIN_TAR_POWER, WLAN_CFG_MAX_TAR_POWER, &value);
        if (ret != OAL_SUCC) {
            wifi_printf_always("ERROR\r\n");
            return ret;
        }
        mfg_param.param[index] = value;
    }

    ret = fe_mfg_power_set_tar_power(hmac_vap, &mfg_param);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_cali_power(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 value;
    osal_u8 index;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    for (index = 0; index < WLAN_CFG_TES_POWER_NUM; index++) {
        ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_MIN_CALI_POWER, WLAN_CFG_MAX_CALI_POWER, &value);
        if (ret != OAL_SUCC) {
            wifi_printf_always("ERROR\r\n");
            return ret;
        }
        mfg_param.param[index] = value;
    }

    ret = fe_mfg_power_set_tar_cali_power(hmac_vap, &mfg_param);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    return ret;
}

OAL_STATIC osal_s32 hmac_set_curve_param(hmac_vap_stru *hmac_vap, const osal_s8 *param, osal_u8 mode)
{
    osal_s32 ret;
    osal_s32 value;
    osal_s32 flag;
    osal_u8 index;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    ret = hmac_ccpriv_get_one_arg_digit(&param, &flag);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_set_curve_param::get flag return err_code[%d].}", ret);
        return ret;
    }

    if (flag < 0 || flag > 1) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_set_curve_param::get constant_offset return err_code [%d]!}", ret);
        return OAL_FAIL;
    }

    mfg_param.param[0] = flag;
    if (flag == 1) {
        for (index = 1; index <= WLAN_CFG_MAX_POWER_PARAM_NUM; index++) {
            ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_CONSTANT_OFFSET_MIN,
                WLAN_CFG_CONSTANT_OFFSET_MAX, &value);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY,
                    "{hmac_set_curve_param::get constant_offset return err_code [%d]!}", ret);
                return ret;
            }
            mfg_param.param[index] = value;
        }
    }

    if (mode == WLAN_CFG_CONSTANT_OFFSET_HIGH_GAIN) {
        ret = fe_mfg_power_set_curve_param(hmac_vap, &mfg_param);
    } else if (mode == WLAN_CFG_CONSTANT_OFFSET_LOW_GAIN) {
        ret = fe_mfg_power_set_low_curve_param(hmac_vap, &mfg_param);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_set_curve_param::err mode!}");
        return OAL_FAIL;
    }
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_set_curve_param::fe_mfg_power_set_(low_)curve_param return err_code [%d]!}", ret);
        return ret;
    }
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_curve_param(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    ret = hmac_set_curve_param(hmac_vap, param, WLAN_CFG_CONSTANT_OFFSET_HIGH_GAIN);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_set_low_curve_param(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    ret = hmac_set_curve_param(hmac_vap, param, WLAN_CFG_CONSTANT_OFFSET_LOW_GAIN);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    return ret;
}

OAL_STATIC osal_s32 hmac_get_curve_param(hmac_vap_stru *hmac_vap, const osal_s8 *param, osal_u8 mode,
    mfg_param_stru *mfg_param)
{
    osal_s32 ret;

    unref_param(param);
    if (mode == WLAN_CFG_CONSTANT_OFFSET_HIGH_GAIN) {
        ret = fe_mfg_power_get_curve_param(hmac_vap, mfg_param);
    } else if (mode == WLAN_CFG_CONSTANT_OFFSET_LOW_GAIN) {
        ret = fe_mfg_power_get_low_curve_param(hmac_vap, mfg_param);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_get_curve_param::err mode!}");
        return OAL_FAIL;
    }

    if (ret != OAL_SUCC || mfg_param->output_param_num != WLAN_CFG_POWER_PARAM_NUM) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{hmac_get_curve_param::fe_mfg_power_get_power_comp return err_code [%d]!}", ret);
        return OAL_FAIL;
    }

    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_curve_param(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    ret = hmac_get_curve_param(hmac_vap, param, WLAN_CFG_CONSTANT_OFFSET_HIGH_GAIN, &mfg_param);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    /* 2: 功率偏移值索引 */
    hmac_mfg_print_output_param(&mfg_param);
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_low_curve_param(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    mfg_param_stru mfg_param;

    (osal_void)memset_s(&mfg_param, sizeof(mfg_param), 0, sizeof(mfg_param));
    ret = hmac_get_curve_param(hmac_vap, param, WLAN_CFG_CONSTANT_OFFSET_LOW_GAIN, &mfg_param);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    wifi_printf_always("OK\r\n");
    /* 2: 功率偏移值索引 */
    hmac_mfg_print_output_param(&mfg_param);
    return ret;
}

/*****************************************************************************
 函 数 名 : hmac_ccpriv_set_xo_trim_fine
 功能描述 : 设置细调频偏寄存器值
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_set_xo_trim_fine(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 xo_trim_fine = 0;
    osal_u32 fine_reg = 0;

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_FINE_MIN_NUM, WLAN_CFG_FINE_MAX_NUM, &xo_trim_fine);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    hal_set_xo_trim_fine((osal_u32)xo_trim_fine, &fine_reg);
    if ((osal_u32)xo_trim_fine != fine_reg) {
        wifi_printf_always("ERROR\r\n");
        return OAL_FAIL;
    }
    wifi_printf_always("OK\r\n");
    return ret;
}

/*****************************************************************************
 函 数 名 : hmac_ccpriv_set_xo_trim_coarse
 功能描述 : 设置粗调频偏寄存器值
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_set_xo_trim_coarse(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 xo_trim_coarse = 0;
    osal_u32 coarse_reg = 0;

    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_COARSE_MIN_NUM, WLAN_CFG_COARSE_MAX_NUM, &xo_trim_coarse);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    hal_set_xo_trim_coarse((osal_u32)xo_trim_coarse, &coarse_reg);
    if ((osal_u32)xo_trim_coarse != coarse_reg) {
        wifi_printf_always("ERROR\r\n");
        return OAL_FAIL;
    }
    wifi_printf_always("OK\r\n");
    return ret;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_cmu_xo_trim(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_u32 xo_trim_coarse = 0;
    osal_u32 xo_trim_fine = 0;

    unref_param(hmac_vap);
    unref_param(param);
    hal_get_xo_trim(&xo_trim_coarse, &xo_trim_fine);
    wifi_printf_always("OK\r\n");
    wifi_printf_always("xo_trim_coarse = %d , xo_trim_fine = %d\r\n", xo_trim_coarse, xo_trim_fine);
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_config_efuse_operate_with_echo(hmac_vap_stru *hmac_vap, efuse_operate_stru *param)
{
    osal_s32 ret;
    ret = hal_config_efuse_operate(hmac_vap, param);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
    } else {
        wifi_printf_always("OK\n");
    }

    return ret;
}

/*****************************************************************************
 功能描述  : 写rssi产测校准信息
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_set_efuse_rssi_offset(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 rssi_offset;
    osal_u8 index;
    osal_u8 rssi_param[RSSI_OFFSET_SIZE] = {0};
    efuse_operate_stru efuse_info;
    osal_char *name[] = {"rssi_offset_0", "rssi_offset_1", "rssi_offset_2"};

    for (index = 0; index < WLAN_CFG_CONSTANT_OFFSET_NUM; index++) {
        ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_MIN_RSSI_OFFSET,
            WLAN_CFG_MAX_RSSI_OFFSET, &rssi_offset);
        if (ret != OAL_SUCC) {
            wifi_printf_always("ERROR\r\n");
            return ret;
        }
        rssi_param[index] = (osal_u8)rssi_offset;
    }

    for (index = 0; index < RSSI_OFFSET_SIZE; index++) {
        (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
        efuse_info.op = EFUSE_OP_WRITE;
        if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, name[index], strlen(name[index])) != EOK) {
            wifi_printf_always("ERROR\r\n");
            return OAL_FAIL;
        }
        efuse_info.val[0] = rssi_param[index];

        ret = hal_config_efuse_operate(hmac_vap, &efuse_info);
        if (ret != OAL_SUCC) {
            wifi_printf_always("ERROR\r\n");
            return ret;
        }
    }

    wifi_printf_always("OK\n");
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 读取rssi产测校准信息
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_get_efuse_rssi_offset(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 index;
    efuse_operate_stru efuse_info;
    osal_char *name[] = {"rssi_offset_0", "rssi_offset_1", "rssi_offset_2"};

    unref_param(param);
    for (index = 0; index < RSSI_OFFSET_SIZE; index++) {
        (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
        efuse_info.op = EFUSE_OP_READ;
        (osal_void)memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, name[index], EFUSE_NAME_LEN);

        ret = hal_config_efuse_operate(hmac_vap, &efuse_info);
        if (ret != OAL_SUCC) {
            wifi_printf_always("ERROR\r\n");
            return ret;
        }
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 写efuse功率产测校准信息
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_efuse_write_power_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 index = 0;
    osal_s32 power_info[WLAN_CFG_ONE_POWER_PARAM_NUM] = {0};
    mfg_param_stru curve_factor = {0};
    mfg_param_stru curve_param = {0};
    mfg_param_stru curve_param_low = {0};
    efuse_operate_stru efuse_info;
    osal_char *pwr_str[] = {"curve_factor_h", "curve_factor_l", "11b_hign", "ofdm_20M_hign",
        "ofdm_40M_hign", "11b_low", "ofdm_20M_low", "ofdm_40M_low"};

    ret = fe_mfg_power_get_curve_factor(hmac_vap, &curve_factor);
    if (ret != OAL_SUCC || curve_factor.output_param_num != WLAN_CFG_CURVE_FACTOR_NUM) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    power_info[index++] = curve_factor.output_param[2]; // 2 高功率 放大系数索引
    power_info[index++] = curve_factor.output_param[5]; // 5 低功率 放大系数索引
    ret = hmac_get_curve_param(hmac_vap, param, WLAN_CFG_CONSTANT_OFFSET_HIGH_GAIN, &curve_param);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    power_info[index++] = curve_param.output_param[2];  // 2 高功率 11b 常数项索引
    power_info[index++] = curve_param.output_param[5];  // 5 高功率 20m 常数项索引
    power_info[index++] = curve_param.output_param[8];  // 8 高功率 40m 常数项索引
    ret = hmac_get_curve_param(hmac_vap, param, WLAN_CFG_CONSTANT_OFFSET_LOW_GAIN, &curve_param_low);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }
    power_info[index++] = curve_param_low.output_param[2];  // 2 低功率 11b 常数项索引
    power_info[index++] = curve_param_low.output_param[5];  // 5 低功率 20m 常数项索引
    power_info[index++] = curve_param_low.output_param[8];  // 8 低功率 40m 常数项索引

    for (index = 0; index < WLAN_CFG_ONE_POWER_PARAM_NUM; index++) {
        (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
        efuse_info.op = EFUSE_OP_WRITE;
        (osal_void)memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, pwr_str[index], EFUSE_NAME_LEN);
        efuse_info.val[0] = (osal_u8)(power_info[index] & 0xff);
        efuse_info.val[1] = (osal_u8)((power_info[index] & 0xff00) >> SIZE_8_BITS);
        ret = hal_config_efuse_operate(hmac_vap, &efuse_info);
        if (ret != OAL_SUCC) {
            wifi_printf_always("ERROR\r\n");
            return ret;
        }
    }
    wifi_printf_always("OK\r\n");
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 读取efuse功率产测校准信息
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_efuse_read_power_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u32 index;
    efuse_operate_stru efuse_info;
    osal_char *pwr_str[] = {"curve_factor_h", "curve_factor_l", "11b_hign", "11b_low", "ofdm_20M_hign",
        "ofdm_20M_low", "ofdm_40M_hign", "ofdm_40M_low"};

    unref_param(param);
    for (index = 0; index < WLAN_CFG_ONE_POWER_PARAM_NUM; index++) {
        (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
        efuse_info.op = EFUSE_OP_READ;
        (osal_void)memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, pwr_str[index], EFUSE_NAME_LEN);

        ret = hal_config_efuse_operate(hmac_vap, &efuse_info);
        if (ret != OAL_SUCC) {
            wifi_printf_always("ERROR\r\n");
            return ret;
        }
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 写efuse频偏产测校准信息
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_write_efuse_cmu_xo_trim(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u32 xo_trim_coarse;
    osal_u32 xo_trim_fine;

    unref_param(hmac_vap);
    unref_param(param);
    hal_get_xo_trim(&xo_trim_coarse, &xo_trim_fine);

    ret = hal_efuse_cmu_xo_trim_write((osal_u8)xo_trim_coarse, (osal_u8)xo_trim_fine);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }
    wifi_printf_always("OK\n");
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 读取efuse频偏产测校准信息
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_read_efuse_cmu_xo_trim(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 coarse_reg = 0;
    osal_u8 fine_reg = 0;

    unref_param(hmac_vap);
    unref_param(param);
    ret = hal_read_efuse_cmu_xo_trim(&coarse_reg, &fine_reg);
    if (ret != OAL_SUCC) {
        return ret;
    }
    wifi_printf_always("OK\n");
    wifi_printf_always("cmu_xo_trim_coarse = %d cmu_xo_trim_fine= %d\n", coarse_reg, fine_reg);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 写efuse温度档位
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_efuse_write_temp(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    efuse_operate_stru efuse_info;
    osal_s32 temp;
    osal_u8 gears;

    (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_CFG_TEMPER_MIN_LEVEL, WLAN_CFG_TEMPER_MAX_LEVEL, &temp);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\r\n");
        return ret;
    }

    gears = (osal_u8)((temp - WLAN_CFG_TEMPER_MIN_LEVEL) / WLAN_CFG_TEMPER_GEARS_STEP);
    efuse_info.val[0] = gears;
    efuse_info.op = EFUSE_OP_WRITE;
    efuse_info.len = sizeof(efuse_info.val[0]);
    if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, "temp", strlen("temp")) != EOK) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }

    return hmac_config_efuse_operate_with_echo(hmac_vap, &efuse_info);
}

/*****************************************************************************
 功能描述  : 读efuse温度档位
*****************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_read_efuse_temp(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    efuse_operate_stru efuse_info;

    unref_param(param);
    (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
    efuse_info.op = EFUSE_OP_READ;
    if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, "temp", strlen("temp")) != EOK) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }

    return hmac_config_efuse_operate_with_echo(hmac_vap, &efuse_info);
}

/*****************************************************************************************
 功能描述  : 写efuse产测标志位
******************************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_efuse_write_mfg_flag(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;

    unref_param(hmac_vap);
    unref_param(param);
    ret = hal_efuse_write_mfg_flag();
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
        return ret;
    }
    wifi_printf_always("OK\n");

    return ret;
}

/*****************************************************************************************
 功能描述  : 获取efuse功能块剩余组数
******************************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_efuse_get_remain(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s8 args[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    efuse_operate_stru efuse_info;

    (osal_void)memset_s(&efuse_info, sizeof(efuse_operate_stru), 0, sizeof(efuse_operate_stru));
    ret = hmac_ccpriv_get_one_arg(&param, args, sizeof(args));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_efuse_get_remain get arg return err_code [%d]!}", ret);
        return OAL_FAIL;
    }

    if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, args, osal_strlen((const osal_char *)args)) != EOK) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }
    efuse_info.op = EFUSE_OP_REMAIN;
    ret = hal_config_efuse_operate(hmac_vap, &efuse_info);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_efuse_status(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    efuse_operate_stru efuse_info;

    unref_param(param);
    (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
    efuse_info.op = EFUSE_OP_READ_ALL_MFG_DATA;
    ret = hal_config_efuse_operate(hmac_vap, &efuse_info);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
        return ret;
    }

    return OAL_SUCC;
}

static osal_s32 hmac_ccpriv_set_jtag_enable(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 jtag_enable;
    efuse_operate_stru efuse_info;

    ret = hmac_ccpriv_get_one_arg_digit(&param, &jtag_enable);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
        return ret;
    }

    if (jtag_enable < 0 || jtag_enable > 1) { /* 判断取值范围，模式只支持0/1 */
        wifi_printf_always("ERROR\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (jtag_enable == 0) {
        wifi_printf_always("OK\n");
        return OAL_SUCC;
    }

    (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
    efuse_info.val[0] = (osal_u8)((jtag_enable << 1) + jtag_enable);
    efuse_info.op = EFUSE_OP_WRITE;
    if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, "jtag", strlen("jtag")) != EOK) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }

    return hmac_config_efuse_operate_with_echo(hmac_vap, &efuse_info);
}

static osal_s32 hmac_ccpriv_get_jtag_enable(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    efuse_operate_stru efuse_info;

    unref_param(param);
    (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
    efuse_info.op = EFUSE_OP_READ;
    if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, "jtag", strlen("jtag")) != EOK) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }

    return hmac_config_efuse_operate_with_echo(hmac_vap, &efuse_info);
}

static osal_s32 hmac_ccpriv_set_ssi_spi_mask(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s32 ssi_spi_mask;
    efuse_operate_stru efuse_info;

    ret = hmac_ccpriv_get_one_arg_digit(&param, &ssi_spi_mask);
    if (ret != OAL_SUCC) {
        wifi_printf_always("ERROR\n");
        return ret;
    }

    if ((ssi_spi_mask < 0) || (ssi_spi_mask > 1)) {
        wifi_printf_always("ERROR\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (ssi_spi_mask == 0) {
        wifi_printf_always("OK\n");
        return OAL_SUCC;
    }

    (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
    efuse_info.val[0] = (osal_u8)((ssi_spi_mask << 1) + ssi_spi_mask);
    efuse_info.op = EFUSE_OP_WRITE;
    if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, "ssi", strlen("ssi")) != EOK) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }

    return hmac_config_efuse_operate_with_echo(hmac_vap, &efuse_info);
}

static osal_s32 hmac_ccpriv_get_ssi_spi_mask(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    efuse_operate_stru efuse_info;

    unref_param(param);
    (osal_void)memset_s(&efuse_info, sizeof(efuse_info), 0, sizeof(efuse_info));
    efuse_info.op = EFUSE_OP_READ;
    if (memcpy_s(efuse_info.efuse_name, EFUSE_NAME_LEN, "ssi", strlen("ssi")) != EOK) {
        wifi_printf_always("ERROR\n");
        return OAL_FAIL;
    }

    return hmac_config_efuse_operate_with_echo(hmac_vap, &efuse_info);
}

/*****************************************************************************************
 功能描述  : 获取芯片温度
******************************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_get_temp(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    unref_param(hmac_vap);
    unref_param(param);
    hal_get_temp();
    return OAL_SUCC;
}

osal_u32 hmac_mfg_init(osal_void)
{
    static const ccpriv_cmd_func_stru cmd_func[] = {
        {(const osal_s8 *)"set_mfg_mode", hmac_ccpriv_set_mfg_mode},
        {(const osal_s8 *)"get_mfg_status", hmac_ccpriv_get_mfg_status},
        {(const osal_s8 *)"get_al_rx_rssi", hmac_ccpriv_get_al_rx_rssi},
        {(const osal_s8 *)"set_rssi_offset", hmac_ccpriv_set_rssi_offset},
        {(const osal_s8 *)"get_rssi_offset", hmac_ccpriv_get_rssi_offset},
        {(const osal_s8 *)"set_curve_factor", hmac_ccpriv_set_curve_factor},
        {(const osal_s8 *)"get_curve_factor", hmac_ccpriv_get_curve_factor},
        {(const osal_s8 *)"set_tar_power", hmac_ccpriv_set_tar_power},
        {(const osal_s8 *)"cali_power", hmac_ccpriv_cali_power},
        {(const osal_s8 *)"set_curve_param", hmac_ccpriv_set_curve_param},
        {(const osal_s8 *)"set_low_curve_param", hmac_ccpriv_set_low_curve_param},
        {(const osal_s8 *)"get_curve_param", hmac_ccpriv_get_curve_param},
        {(const osal_s8 *)"get_low_curve_param", hmac_ccpriv_get_low_curve_param},
        {(const osal_s8 *)"set_xo_trim_coarse", hmac_ccpriv_set_xo_trim_coarse},
        {(const osal_s8 *)"set_xo_trim_fine", hmac_ccpriv_set_xo_trim_fine},
        {(const osal_s8 *)"get_cmu_xo_trim", hmac_ccpriv_get_cmu_xo_trim},
        {(const osal_s8 *)"get_temp", hmac_ccpriv_get_temp},

        {(const osal_s8 *)"efuse_write_mfg_flag", hmac_ccpriv_efuse_write_mfg_flag},
        {(const osal_s8 *)"set_efuse_rssi_offset", hmac_ccpriv_set_efuse_rssi_offset},
        {(const osal_s8 *)"get_efuse_rssi_offset", hmac_ccpriv_get_efuse_rssi_offset},
        {(const osal_s8 *)"efuse_write_power_info", hmac_ccpriv_efuse_write_power_info},
        {(const osal_s8 *)"efuse_read_power_info", hmac_ccpriv_efuse_read_power_info},
        {(const osal_s8 *)"efuse_write_cmu_xo_trim", hmac_ccpriv_write_efuse_cmu_xo_trim},
        {(const osal_s8 *)"efuse_read_cmu_xo_trim", hmac_ccpriv_read_efuse_cmu_xo_trim},
        {(const osal_s8 *)"efuse_write_temp", hmac_ccpriv_efuse_write_temp},
        {(const osal_s8 *)"efuse_read_temp", hmac_ccpriv_read_efuse_temp},
        {(const osal_s8 *)"efuse_status", hmac_ccpriv_efuse_status},
        {(const osal_s8 *)"efuse_remain", hmac_ccpriv_efuse_get_remain},
        {(const osal_s8 *)"set_jtag_enable", hmac_ccpriv_set_jtag_enable},
        {(const osal_s8 *)"get_jtag_enable", hmac_ccpriv_get_jtag_enable},
        {(const osal_s8 *)"set_ssi_spi_mask", hmac_ccpriv_set_ssi_spi_mask},
        {(const osal_s8 *)"get_ssi_spi_mask", hmac_ccpriv_get_ssi_spi_mask},
    };

    /* ccpriv命令注册 */
    hmac_ccpriv_register_array(cmd_func, osal_array_size(cmd_func));

    return OAL_SUCC;
}

osal_void hmac_mfg_deinit(osal_void)
{
    /* ccpriv命令注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"set_mfg_mode");
    hmac_ccpriv_unregister((const osal_s8 *)"get_mfg_status");
    hmac_ccpriv_unregister((const osal_s8 *)"get_al_rx_rssi");
    hmac_ccpriv_unregister((const osal_s8 *)"set_rssi_offset");
    hmac_ccpriv_unregister((const osal_s8 *)"get_rssi_offset");
    hmac_ccpriv_unregister((const osal_s8 *)"set_curve_factor");
    hmac_ccpriv_unregister((const osal_s8 *)"get_curve_factor");
    hmac_ccpriv_unregister((const osal_s8 *)"set_tar_power");
    hmac_ccpriv_unregister((const osal_s8 *)"cali_power");
    hmac_ccpriv_unregister((const osal_s8 *)"set_curve_param");
    hmac_ccpriv_unregister((const osal_s8 *)"set_low_curve_param");
    hmac_ccpriv_unregister((const osal_s8 *)"get_curve_param");
    hmac_ccpriv_unregister((const osal_s8 *)"get_low_curve_param");
    hmac_ccpriv_unregister((const osal_s8 *)"set_xo_trim_coarse");
    hmac_ccpriv_unregister((const osal_s8 *)"set_xo_trim_fine");
    hmac_ccpriv_unregister((const osal_s8 *)"get_cmu_xo_trim");
    hmac_ccpriv_unregister((const osal_s8 *)"get_temp");

    hmac_ccpriv_unregister((const osal_s8 *)"efuse_write_mfg_flag");
    hmac_ccpriv_unregister((const osal_s8 *)"set_efuse_rssi_offset");
    hmac_ccpriv_unregister((const osal_s8 *)"get_efuse_rssi_offset");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_write_power_info");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_read_power_info");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_write_cmu_xo_trim");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_read_cmu_xo_trim");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_write_temp");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_read_temp");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_status");
    hmac_ccpriv_unregister((const osal_s8 *)"efuse_remain");
    hmac_ccpriv_unregister((const osal_s8 *)"set_jtag_enable");
    hmac_ccpriv_unregister((const osal_s8 *)"get_jtag_enable");
    hmac_ccpriv_unregister((const osal_s8 *)"set_ssi_spi_mask");
    hmac_ccpriv_unregister((const osal_s8 *)"get_ssi_spi_mask");

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
#endif  /* #ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD */