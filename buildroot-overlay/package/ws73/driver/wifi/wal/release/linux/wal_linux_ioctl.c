/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: wal_linux_ioctl.c file.
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wal_linux_ioctl.h"
#include "wlan_msg.h"
#include "wal_main.h"
#include "frw_ext_if.h"
#include "common_dft.h"

#include "hcc_host_if.h"
#include "plat_cali.h"

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_netbuf.h"
#include "soc_customize_wifi.h"
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "wal_linux_util.h"
#include "wal_linux_netdev.h"
#include "hmac_main.h"
#include "wal_regdb.h"
#include "hmac_dfr.h"
#ifdef _PRE_WLAN_FEATURE_DFR
#include "wal_linux_dfx.h"
#endif
#include "alg_main.h"
#include "hmac_device.h"
#ifdef _PRE_WLAN_SMOOTH_PHASE
#include "msg_smooth_phase_rom.h"
#endif
#include "wal_linux_ccpriv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_IOCTL_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 结构体定义
*****************************************************************************/
#define CMD_GET_PHY_RATE              "CMD_GET_PHY_RATE"
#define CMD_GET_SUPPORTED_BAND        "CMD_GET_SUPPORTED_BAND"
#define CMD_GET_CHAN_BW               "CMD_GET_CHAN_BW"
#define CMD_GET_CURRENT_TSF           "CMD_GET_CURRENT_TSF"
#define CMD_GET_NON_COEX_IFACE        "CMD_GET_NON_COEX_IFACE"
#define CMD_CHBA_PARAMS              "CMD_CHBA_PARAMS"

#define CMD_GET_PHY_RATE_LEN                     (strlen(CMD_GET_PHY_RATE))
#define CMD_GET_SUPPORTED_BAND_LEN               (strlen(CMD_GET_SUPPORTED_BAND))
#define CMD_GET_CHAN_BW_LEN                      (strlen(CMD_GET_CHAN_BW))
#define CMD_GET_CURRENT_TSF_LEN                  (strlen(CMD_GET_CURRENT_TSF))
#define CMD_GET_NON_COEX_IFACE_LEN               (strlen(CMD_GET_NON_COEX_IFACE))
#define CMD_CHBA_PARAMS_LEN                     (strlen(CMD_CHBA_PARAMS))

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC osal_void hwifi_config_rf_fe_custom_ini_tx_power(oal_net_device_stru *cfg_net_dev)
{
    wlan_cust_rf_fe_params *rf_fe_ini = hwifi_get_rf_fe_custom_ini();
    osal_s32 ret;

    if (rf_fe_ini->ini_ret & (1 << WLAN_CFG_INI_RF_FE_TYPE_POWER)) {
        ret = wal_async_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev), WLAN_MSG_W2H_CFG_SET_RF_FE_POWER,
            (osal_u8 *)&rf_fe_ini->rf_power, sizeof(rf_fe_ini->rf_power), FRW_POST_PRI_LOW);
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_config_rf_fe_custom_ini_tx_power::send msg:%d failed, return[%d]!}",
                WLAN_MSG_W2H_CFG_SET_RF_FE_POWER, ret);
        }
        rf_fe_ini->ini_ret &= (~(1 << WLAN_CFG_INI_RF_FE_TYPE_POWER));
    }
}
/*****************************************************************************
 函 数 名  : hwifi_force_refresh_rf_params
 功能描述  : regdomain改变时刷新txpwr和dbb scale
*****************************************************************************/
osal_u32 hwifi_force_refresh_rf_params(oal_net_device_stru *net_dev)
{
    /* 刷新射频前端定制化项功率参数 跟随国家码区域变更 */
#ifndef _PRE_WLAN_NO_SUPPORT_INI
    hwifi_config_init_etc(CUS_TAG_RF_FE_TX_POWER);
#endif
    hwifi_config_rf_fe_custom_ini_tx_power(net_dev);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 根据国家码设置管制域
*****************************************************************************/
OAL_STATIC osal_u32  wal_setcountry(oal_net_device_stru *net_dev, osal_s8 *para)
{
#ifdef _PRE_WLAN_FEATURE_11D
    osal_s32                        l_ret;
    l_ret = wal_util_set_country_code(net_dev, para);
    if (OAL_SUCC != l_ret) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_setcountry::wal_util_set_country_code err %d!}", l_ret);
        return (osal_u32)l_ret;
    }
#else
    oam_warning_log0(0, OAM_SF_ANY, "{wal_setcountry::_PRE_WLAN_FEATURE_11D is not define!}");
#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 国家码定制化
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_void hwifi_config_init_ini_country(oal_net_device_stru *cfg_net_dev)
{
    osal_s32 l_ret;
    osal_s8 *country_code = hwifi_get_country_code_etc();
#ifdef CONTROLLER_CUSTOMIZATION
    static osal_bool first_set_flag = OSAL_TRUE;

    if (first_set_flag != OSAL_TRUE) {
        return;
    }
    first_set_flag = OSAL_FALSE;
#endif
    // 根据hwifi_get_country_code_etc内容查管制域数据库, 下发配置
    if (wal_regdb_find_db_etc(country_code) == OSAL_NULL) {
        /* 配置文件若是非法国家码, 自动改为CN */
        hwifi_set_country_code_etc(DEFAULT_COUNTRY_CODE, COUNTRY_CODE_LEN);
    }
    l_ret = (osal_s32)wal_setcountry(cfg_net_dev, country_code);
    if (OAL_UNLIKELY(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hwifi_config_init_ini_country::wal_setcountry return err code [%d]!}", l_ret);
    }
}

/*****************************************************************************
 功能描述  : 日志相关
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
OAL_STATIC osal_void hwifi_config_init_ini_log(oal_net_device_stru *net_dev)
{
    osal_s32   ret;
    osal_s32   loglevel = OAM_LOG_LEVEL_WARNING;

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_SET_ALL_LOG_LEVEL,
        (osal_u8 *)&loglevel, OAL_SIZEOF(osal_s32));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hwifi_config_init_ini_log::return err code [%d]!}", ret);
        return;
    }
}

/*****************************************************************************
 函 数 名  : hwifi_check_pwr_ref_delta
 功能描述  : 检查定制化中的delta_rssi值是否超过阈值
*****************************************************************************/
osal_s8 hwifi_check_pwr_ref_delta(osal_s8 c_pwr_ref_delta)
{
    osal_slong c_ret;
    if (c_pwr_ref_delta > WAL_HIPRIV_PWR_REF_DELTA_HI) {
        c_ret = WAL_HIPRIV_PWR_REF_DELTA_HI;
    } else if (c_pwr_ref_delta < WAL_HIPRIV_PWR_REF_DELTA_LO) {
        c_ret = WAL_HIPRIV_PWR_REF_DELTA_LO;
    } else {
        c_ret = c_pwr_ref_delta;
    }

    return c_ret;
}
/*****************************************************************************
 函 数 名  : hwifi_cfg_front_end
 功能描述  : hw 2g 5g 前端
*****************************************************************************/

osal_void hwifi_cfg_pwr_ref_delta(mac_cfg_customize_rf *customize_rf)
{
    osal_u8 rf_idx;
    wlan_cfg_init cfg_id;
    osal_u32 pwr_ref_delta;

    /* 2G 20M/40M */
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        cfg_id = (rf_idx == WLAN_RF_CHANNEL_ZERO) ? WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4 :
                 WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4;
        pwr_ref_delta = (osal_u32)hwifi_get_init_value_etc(CUS_TAG_INI, cfg_id);
        customize_rf->delta_pwr_ref_cfg[rf_idx].cfg_delta_pwr_ref_rssi_2g[0] = hwifi_check_pwr_ref_delta(
            (osal_s8)(osal_u8)(pwr_ref_delta & 0xff));
        customize_rf->delta_pwr_ref_cfg[rf_idx].cfg_delta_pwr_ref_rssi_2g[1] = hwifi_check_pwr_ref_delta(
            (osal_s8)(osal_u8)((pwr_ref_delta >> BIT_OFFSET_8) & 0xff)); /* 右移8位，取7~15bit */
    }

    /* 5G 20M/40M/80M/160M */
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        cfg_id = (rf_idx == WLAN_RF_CHANNEL_ZERO) ? WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4 :
                 WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4;
        pwr_ref_delta = (osal_u32)hwifi_get_init_value_etc(CUS_TAG_INI, cfg_id);
        customize_rf->delta_pwr_ref_cfg[rf_idx].cfg_delta_pwr_ref_rssi_5g[0] = hwifi_check_pwr_ref_delta(
            (osal_s8)(osal_u8)(pwr_ref_delta & 0xff));
        customize_rf->delta_pwr_ref_cfg[rf_idx].cfg_delta_pwr_ref_rssi_5g[1] = hwifi_check_pwr_ref_delta(
            (osal_s8)(osal_u8)((pwr_ref_delta >> BIT_OFFSET_8) & 0xff)); /* 右移8位，取7~15bit */
        customize_rf->delta_pwr_ref_cfg[rf_idx].cfg_delta_pwr_ref_rssi_5g[2] = hwifi_check_pwr_ref_delta(
            (osal_s8)(osal_u8)((pwr_ref_delta >> BIT_OFFSET_16) & 0xff)); /* 右移16位，取16~23bit为第2位 */
        customize_rf->delta_pwr_ref_cfg[rf_idx].cfg_delta_pwr_ref_rssi_5g[3] = hwifi_check_pwr_ref_delta(
            (osal_s8)(osal_u8)((pwr_ref_delta >> BIT_OFFSET_24) & 0xff)); /* 右移24位，取24~31bit为第3位 */
    }
}

OAL_STATIC osal_u32 hwifi_cfg_set_2g_rf(mac_cfg_customize_rf *customize_rf)
{
    osal_u8 idx;
    osal_u32 mult4;
    osal_s8 mult4_rf[2];

    for (idx = 0; idx < MAC_NUM_2G_BAND; ++idx) {
        /* 获取各2p4g 各band 0.25db及0.1db精度的线损值 */
        mult4  = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START + idx);
        /* rf0 */
        mult4_rf[0]  = (osal_s8)(osal_u8)(mult4 & 0xFF);
        /* rf1 */
        mult4_rf[1]  = (osal_s8)(osal_u8)((mult4 >> 8) & 0xFF); /* 右移8位 */

        if (mult4_rf[0] >= RF_LINE_TXRX_GAIN_DB_2G_MIN && mult4_rf[0] <= RF_LINE_TXRX_GAIN_DB_MAX &&
            mult4_rf[1] >= RF_LINE_TXRX_GAIN_DB_2G_MIN && mult4_rf[1] <= RF_LINE_TXRX_GAIN_DB_MAX) {
            customize_rf->rf_gain_db_rf[0].ac_gain_db_2g[idx].rf_gain_db_mult4 = mult4_rf[0];
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_CFG, "{hwifi_cfg_front_end::ini_id[%d]value out of range, 2g mult4[0x%0x}!}",
                           WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START + idx, mult4);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hwifi_cfg_set_5g_rf(mac_cfg_customize_rf *customize_rf)
{
    osal_u8 idx;
    osal_u32 mult4;
    osal_s8 mult4_rf[2];

    for (idx = 0; idx < MAC_NUM_5G_BAND; ++idx) {
        /* 获取各5g 各band 0.25db及0.1db精度的线损值 */
        mult4  = hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START + idx);
        mult4_rf[0]  = (osal_s8)(osal_u8)(mult4 & 0xFF);
        mult4_rf[1]  = (osal_s8)(osal_u8)((mult4 >> 8) & 0xFF); /* 右移8位 */

        if (mult4_rf[0] <= RF_LINE_TXRX_GAIN_DB_MAX && mult4_rf[1] <= RF_LINE_TXRX_GAIN_DB_MAX) {
            customize_rf->rf_gain_db_rf[0].ac_gain_db_5g[idx].rf_gain_db_mult4  = mult4_rf[0];
        } else {
            /* 值超出有效范围 */
            oam_error_log2(0, OAM_SF_CFG, "{hwifi_cfg_front_end::ini_id[%d]value out of range, 5g mult4[0x%0x}!}",
                            WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START + idx, mult4);
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC osal_void hwifi_cfg_cca_thresh(mac_cfg_customize_rf *customize_rf)
{
    osal_s8 delta_cca_ed_high_20th_2g = (osal_s8)hwifi_get_init_value_etc(CUS_TAG_INI,
        WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G);
    osal_s8 delta_cca_ed_high_40th_2g = (osal_s8)hwifi_get_init_value_etc(CUS_TAG_INI,
        WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G);
    osal_s8 delta_cca_ed_high_20th_5g = (osal_s8)hwifi_get_init_value_etc(CUS_TAG_INI,
        WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G);
    osal_s8 delta_cca_ed_high_40th_5g = (osal_s8)hwifi_get_init_value_etc(CUS_TAG_INI,
        WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G);
    /* 检查每一项的调整幅度是否超出最大限制 */
    if (cus_delta_cca_ed_high_th_out_of_range(delta_cca_ed_high_20th_2g) ||
        cus_delta_cca_ed_high_th_out_of_range(delta_cca_ed_high_40th_2g) ||
        cus_delta_cca_ed_high_th_out_of_range(delta_cca_ed_high_20th_5g) ||
        cus_delta_cca_ed_high_th_out_of_range(delta_cca_ed_high_40th_5g)) {
        oam_error_log4(0, OAM_SF_ANY, "{hwifi_cfg_front_end::one or more delta cca ed high threshold out of range \
            [delta_20th_2g=%d, delta_40th_2g=%d, delta_20th_5g=%d, delta_40th_5g=%d], please check the value!}",
            delta_cca_ed_high_20th_2g, delta_cca_ed_high_40th_2g, delta_cca_ed_high_20th_5g,
            delta_cca_ed_high_40th_5g);
        /* set 0 */
        customize_rf->delta_cca_ed_high_20th_2g = 0;
        customize_rf->delta_cca_ed_high_40th_2g = 0;
        customize_rf->delta_cca_ed_high_20th_5g = 0;
        customize_rf->delta_cca_ed_high_40th_5g = 0;
    } else {
        customize_rf->delta_cca_ed_high_20th_2g = delta_cca_ed_high_20th_2g;
        customize_rf->delta_cca_ed_high_40th_2g = delta_cca_ed_high_40th_2g;
        customize_rf->delta_cca_ed_high_20th_5g = delta_cca_ed_high_20th_5g;
        customize_rf->delta_cca_ed_high_40th_5g = delta_cca_ed_high_40th_5g;
    }
}

OAL_STATIC osal_void hwifi_cfg_junction_temp(mac_cfg_customize_rf *customize_rf)
{
    const osal_s16 temp_diff_thr = 20;
    osal_s8 temp_diff = (osal_s8)hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_JUNCTION_TEMP_DIFF);
    /* 检查每一项的调整幅度是否超出最大限制 */
    if (temp_diff > temp_diff_thr || temp_diff < -temp_diff_thr) {
        oam_error_log2(0, OAM_SF_ANY, "hwifi_cfg_junction_temp::temp diff:%d/%d out of range!",
            temp_diff, temp_diff_thr);
        /* set 0 */
        temp_diff = 0;
    }
    customize_rf->junction_temp_diff = temp_diff;
}

OAL_STATIC osal_u32 hwifi_cfg_front_end(osal_u8 *param)
{
    mac_cfg_customize_rf       *customize_rf;

    customize_rf = (mac_cfg_customize_rf *)param;

    /* 配置: 2g rf */
    if (hwifi_cfg_set_2g_rf(customize_rf) != OAL_SUCC) {
        return OAL_FAIL;
    }

    hwifi_cfg_pwr_ref_delta(customize_rf);
    /* 通道radio cap */
    customize_rf->chn_radio_cap = 0XF;
    if (OAL_TRUE == mac_device_check_5g_enable_per_chip()) {
        /* 配置: 5g rf */
        /* 配置: fem口到天线口的负增益 */
        if (hwifi_cfg_set_5g_rf(customize_rf) != OAL_SUCC) {
            return OAL_FAIL;
        }
    }

    customize_rf->far_dist_pow_gain_switch = OAL_TRUE;
    customize_rf->far_dist_dsss_scale_promote_switch = OAL_TRUE;

    /* 配置: cca能量门限调整值 */
    hwifi_cfg_cca_thresh(customize_rf);
    /* 配置 结温偏差补偿值 */
    hwifi_cfg_junction_temp(customize_rf);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hwifi_config_init_ini_rf
 功能描述  : hw 2g 5g 前端定制化
*****************************************************************************/
OAL_STATIC osal_void hwifi_config_init_ini_rf(oal_net_device_stru *cfg_net_dev)
{
    osal_u32                  ret;
    osal_u16                  event_len = OAL_SIZEOF(mac_cfg_customize_rf);
    mac_cfg_customize_rf customize_rf;
    (osal_void)memset_s(&customize_rf, OAL_SIZEOF(customize_rf), 0, OAL_SIZEOF(customize_rf));

    /* 定制化下发不能超过事件内存长 */
    if (event_len > WAL_MSG_WRITE_MAX_LEN) {
        oam_error_log2(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::event size[%d] larger than msg size[%d]!}",
                       event_len, WAL_MSG_WRITE_MAX_LEN);
        return;
    }
    ret = hwifi_cfg_front_end((osal_u8 *)&customize_rf);
    if (OAL_SUCC != ret) {
        oam_error_log0(0, OAM_SF_ANY, "{hwifi_config_init_ini_rf::front end rf wrong value, not send cfg!}");
        return;
    }

    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev), WLAN_MSG_W2H_C_CFG_SET_CUS_RF,
        (osal_u8 *)&customize_rf, sizeof(customize_rf));
    if (OAL_UNLIKELY(OAL_SUCC != ret)) {
        oam_error_log1(0, OAM_SF_ANY,
            "{hwifi_config_init_ini_rf::EVENT[wal_sync_send2device_no_rsp] failed, return err code [%d]!}", ret);
    }
}

/*****************************************************************************
 函 数 名  : hwifi_config_init_dts_cali
 功能描述  : 定制化参数dts校准
*****************************************************************************/
OAL_STATIC osal_u32 hwifi_config_init_dts_cali(oal_net_device_stru *cfg_net_dev)
{
    osal_u32 ret;
    ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev), WLAN_MSG_W2H_C_CFG_SET_CUS_DTS_CALI,
        OAL_PTR_NULL, 0);
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_ANY, "{hwifi_config_init_dts_cali::wal_sync_post2hmac_no_rsp failed ret:%d}", ret);
        return ret;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void hwifi_config_rf_fe_custom_ini(oal_net_device_stru *cfg_net_dev)
{
    wlan_cust_rf_fe_params *rf_fe_ini = hwifi_get_rf_fe_custom_ini();
    wlan_cust_rf_fe_rssi_params rssi = { 0 };
    osal_s32 ret;

    if (rf_fe_ini->ini_ret & (1 << WLAN_CFG_INI_RF_FE_TYPE_PARAMS)) {
        ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev), WLAN_MSG_W2H_CFG_SET_RF_FE_RX_INSERT_LOSS,
            (osal_u8 *)&rf_fe_ini->rf_rx_loss, sizeof(rf_fe_ini->rf_rx_loss));
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_config_rf_fe_custom_ini::send msg:%d failed, return[%d]!}",
                WLAN_MSG_W2H_CFG_SET_RF_FE_RX_INSERT_LOSS, ret);
        }
        rf_fe_ini->ini_ret &= (~(1 << WLAN_CFG_INI_RF_FE_TYPE_PARAMS));
    }

    if (rf_fe_ini->ini_ret & (1 << WLAN_CFG_INI_RF_FE_TYPE_POWER_CALI)) {
        ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev), WLAN_MSG_W2H_CFG_SET_RF_FE_POWER_CALI,
            (osal_u8 *)&rf_fe_ini->rf_power_cali, sizeof(rf_fe_ini->rf_power_cali));
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_config_rf_fe_custom_ini::send msg:%d failed, return[%d]!}",
                WLAN_MSG_W2H_CFG_SET_RF_FE_POWER_CALI, ret);
        }
        rf_fe_ini->ini_ret &= (~(1 << WLAN_CFG_INI_RF_FE_TYPE_POWER_CALI));
    }

    if (rf_fe_ini->ini_ret & (1 << WLAN_CFG_INI_RF_FE_TYPE_RSSI)) {
        memcpy_s(&rssi, sizeof(rssi), &rf_fe_ini->rf_rssi, sizeof(wlan_cust_rf_fe_rssi_params));
        rssi.rssi_sel_bits = 0x7; /* bit0~2代表subband0~2写入 */
        ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(cfg_net_dev), WLAN_MSG_W2H_CFG_SET_RF_FE_RSSI,
            (osal_u8 *)&rssi, sizeof(rssi));
        if (OAL_UNLIKELY(ret != OAL_SUCC)) {
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_config_rf_fe_custom_ini::send msg:%d failed, return[%d]!}",
                WLAN_MSG_W2H_CFG_SET_RF_FE_RSSI, ret);
        }
        rf_fe_ini->ini_ret &= (~(1 << WLAN_CFG_INI_RF_FE_TYPE_RSSI));
    }
    // 功率相关参数解析 下发
    hwifi_config_rf_fe_custom_ini_tx_power(cfg_net_dev);
}

/*****************************************************************************
 功能描述  : 配置vap定制化
             不涉及wlan p2p网络设备的，均由配置vap配置host参数或下发至device
 输出参数  : 无
 返 回 值  : OAL_SUCC 或 失败错误码
*****************************************************************************/
osal_void hwifi_config_init_ini_main(oal_net_device_stru *cfg_net_dev)
{
    /* 国家码 */
    hwifi_config_init_ini_country(cfg_net_dev);
    /* 可维可测 */
    hwifi_config_init_ini_log(cfg_net_dev);
    /* RF */
    hwifi_config_init_ini_rf(cfg_net_dev);
    hwifi_config_rf_fe_custom_ini(cfg_net_dev);
}

osal_u32 hwifi_config_init_dts_main_etc(oal_net_device_stru *cfg_net_dev)
{
    osal_u32 ul_ret = OAL_SUCC;

    /* 校准 */
    if (hwifi_config_init_dts_cali(cfg_net_dev) != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 校准放到第一个进行 */
    return ul_ret;
}

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
osal_bool g_wifi_ever_opened = OSAL_FALSE;

osal_u32 hmac_main_exit_later(osal_void)
{
    /* wlan 没有打开过，直接退出 */
    if (g_wifi_ever_opened == OSAL_TRUE) {
        alg_hmac_main_exit();
        hmac_board_exit();

#ifdef _PRE_WLAN_FEATURE_WS63 // 回退冲突
        hal_main_exit();
#else
        hal_hmac_main_exit();
#endif
        g_wifi_ever_opened = OSAL_FALSE;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_SMOOTH_PHASE
osal_u32 wal_set_smooth_phase_en(oal_net_device_stru *net_dev)
{
    smooth_phase_stru  smooth_phase_info;
    osal_u32    ret;

    smooth_phase_info.enable = 1;
    smooth_phase_info.extra_value = 0;
    ret = wal_sync_send2device_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2D_C_CFG_SMOOTH_PHASE,
        (osal_u8 *)&smooth_phase_info, OAL_SIZEOF(smooth_phase_info));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_PWR, "{wal_set_smooth_phase_en::return err code [%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 函 数 名  : wal_custom_cali_etc
 功能描述  : 下发定制化信息，初始化射频，校准
*****************************************************************************/
osal_u32 wal_custom_cali_etc(osal_void)
{
    oal_net_device_stru *net_dev;
    osal_u32           ul_ret;

    if (g_wifi_ever_opened == OSAL_FALSE) {
        ul_ret = hmac_main_init_later();
        if (ul_ret != OAL_SUCC) {
            wifi_printf("wal_custom_cali_etc:hmac_main_init_later error code:%d\r\n", ul_ret);
            return OAL_FAIL;
        }

        g_wifi_ever_opened = OSAL_TRUE;
#ifdef _PRE_WLAN_FEATURE_DFR
    } else if (hmac_dfr_get_recovery_flag() == OSAL_TRUE) {
       /* 自愈流程重新初始化 */
        wal_dfr_main_reinit_later();
    }
#else
    }
#endif
    /* 抛消息到dev, 更新算法参数并触发dev 算法初始化 */
    alg_hmac_sync_param();
    net_dev = oal_dev_get_by_name("Featureid0");  /* 通过cfg vap0来下c0 c1校准 */
    if (net_dev == OAL_PTR_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    } else {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
        oam_warning_log0(0, OAM_SF_ANY, "{wal_custom_cali_etc::the net_device is already exist!}");
    }

    hwifi_config_init_ini_main(net_dev);

    if (g_uc_custom_cali_done_etc == OAL_TRUE) {
        /* 校准数据下发 */
        wal_send_cali_data_etc(net_dev);
    } else {
        g_uc_custom_cali_done_etc = OAL_TRUE;
    }

#ifdef _PRE_WLAN_SMOOTH_PHASE
    if (hwifi_get_smooth_phase_en() == OSAL_TRUE) {
        wal_set_smooth_phase_en(net_dev);
    }
#endif

    /* 下发参数 */
    ul_ret = hwifi_config_init_dts_main_etc(net_dev);

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_set_auto_freq_process_func
 功能描述  : 设置hcc中自动调频的回调函数
*****************************************************************************/
osal_s32 wal_set_custom_process_func_etc(osal_void)
{
    set_custom_cali_func(wal_custom_cali_etc);
    set_custom_exit_later_func(hmac_main_exit_later);
    return OAL_SUCC;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
