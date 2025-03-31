/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: cali init power for tpc/tb init
 * Create: 2022-10-14
 */

#include "fe_init_pow.h"
#include "fe_power_host.h"
#include "power_ctrl_spec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_CALI_INIT_POWER

/*****************************************************************************
    函数定义
*****************************************************************************/
/*
 * 函 数 名  : cali_set_init_ppa_code
 * 功能描述  : 设置一个通道的所有档位的upc参数
 */
OSAL_STATIC osal_void cali_set_init_ppa_code(const hal_device_stru *device,
    wlan_channel_band_enum_uint8 band)
{
    osal_u32 idx;
    osal_u16 pow_code;
    cali_pow_crtl_table_stru *ppa_lut;
    unref_param(band);

    for (idx = 0; idx < CALI_POW_PPA_LUT_NUM; idx++) {
        ppa_lut = cali_get_ppa_lut();
        pow_code = ppa_lut[idx].pow_code;
        fe_hal_phy_set_single_ppa_lut(device, pow_code, idx);
    }
}

/*
 * 函 数 名  : cali_set_init_pa_code
 * 功能描述  : 设置一个通道的所有档位的upc参数
 */
OSAL_STATIC osal_void cali_set_init_pa_code(const hal_device_stru *device,
    wlan_channel_band_enum_uint8 band)
{
    osal_u32 idx;
    const cali_pow_crtl_table_stru *pa_pow_ctrl = cali_get_pa_pow_ctrl(device);
    osal_u16 pow_code;
    unref_param(band);

    for (idx = 0; idx < CALI_POW_PPA_LUT_NUM; idx++) {
        pow_code = pa_pow_ctrl[idx].pow_code;
        fe_hal_phy_set_single_pa_lut(device, pow_code, idx);
    }
}

/*
 * 函 数 名  : cali_set_init_dbb_shadow_bound
 * 功能描述  : 设置dbb scale shadow的boundary
 */
OSAL_STATIC osal_void cali_set_init_dbb_shadow_bound(const hal_device_stru *device,
    wlan_channel_band_enum_uint8 band)
{
    osal_u8 bound;
    osal_u8 idx;
    unref_param(band);
    for (idx = 0; idx < CALI_DBB_DOUNDS; idx++) {
        bound = cali_get_dbb_shadow_bounds(idx);
        fe_hal_cfg_tpc_boundary(device, bound, idx);
    }
}

/*
 * 函 数 名  : cali_pow_init_pwr_code
 * 功能描述  : 初始化tx power code映射表
 * 1.日    期  : 2020年04月13日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_void cali_pow_init_pwr_code(hal_device_stru *device)
{
    wlan_channel_band_enum_uint8 band = device->hal_device_base.wifi_channel_status.band;

    cali_set_init_ppa_code(device, band);
    cali_set_init_pa_code(device, band);
    cali_set_init_dbb_shadow_bound(device, band);
    fe_hal_init_dbb_scale_shadow_to_0x80(device);
}

/*
 * 函 数 名  : cali_pow_set_tpc_mem_reg
 * 功能描述  : 初始化PHY寄存器表单TPC mem
 * 1.日    期  : 2020年3月13日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_void cali_pow_set_tpc_mem_reg(hal_device_stru *device,
    cali_pow_cal_gain_stru *pow_gain_table, cali_pow_distri_ratio_stru *pow_dist_table,
    const osal_u8 *dbb_pre_table, oal_bool_enum_uint8 is_dsss)
{
    osal_s16 pow_gain_val;
    osal_u8 tpc_idx = 0;
    osal_u8 tpc_end_idx;
    osal_u8 pow_gain_idx;
    cali_pow_cal_gain_stru *pow_gain = pow_gain_table;
    cali_pow_distri_ratio_stru *pow_dist = pow_dist_table;
    const osal_u8 *dbb_pre = dbb_pre_table;
    if (is_dsss) {
        pow_gain_val = CALI_2G_POWER_RANGE_DSSS_DELT_POW_MAX;
        tpc_idx = CALI_POW_DIST_2G_OFDM_TABLE_LEN;
        tpc_end_idx = CALI_POW_DIST_2G_OFDM_TABLE_LEN + CALI_POW_DIST_2G_DSSS_TABLE_LEN;
    } else {
        pow_gain_val = CALI_2G_POWER_RANGE_OFDM_DELT_POW_MAX;
        tpc_end_idx = CALI_POW_DIST_2G_OFDM_TABLE_LEN;
    }

    /* TPC RAM 初始化 */
    for (; tpc_idx < tpc_end_idx; tpc_idx++) { // ofdm的tpc_indx范围[0, 37), dsss的tpc_indx范围[37, 74)
        for (pow_gain_idx = 0; pow_gain_idx < CALI_DIST_POW_INTV_NUM; pow_gain_idx++) {
            pow_dist->dbb_scale_pre = *dbb_pre;
            pow_gain->pow_dist = pow_dist;
            pow_gain->tpc_idx = tpc_idx * CALI_DIST_POW_INTV_NUM + pow_gain_idx;
            pow_gain->pow_gain = pow_gain_val;

            fe_hal_pow_write_single_tpc_mem(device, pow_gain->tpc_idx,  // 此处下发给硬件的tpc_idx范围是[0,148)
                *((osal_u32*)pow_gain->pow_dist), is_dsss);
            pow_gain_val -= CALI_DIST_POW_INTERVAL;
            pow_gain++;
            dbb_pre++;
        }
        pow_dist++;
    }
}

/*
 * 函 数 名  : cali_refresh_phy_tpc_mem_reg
 * 功能描述  : 初始化PHY寄存器表单TPC mem
 * 1.日    期  : 2019年1月16日
 *   修改内容  : 新生成函数
 */
OSAL_STATIC osal_void cali_refresh_phy_tpc_mem_reg(hal_device_stru *device,
    wlan_channel_band_enum_uint8 band)
{
    osal_u8                   pow_table_idx;
    cali_pow_distri_ratio_stru *pow_dist_2g_table = NULL;
    cali_pow_cal_gain_stru *pow_cal_gain_2g_table = NULL;
    const osal_u8 *dbb_pre_table = NULL;
    device->hal_device_base.wifi_channel_status.band = band;
    fe_hal_pow_write_tpc_mem_prepare();
    // 循环dsss和ofdm两张表
    for (pow_table_idx = 0; pow_table_idx < CALI_POW_TABLE_LEN; pow_table_idx++) {
        pow_dist_2g_table = cali_get_2g_pow_dist_table(pow_table_idx);
        pow_cal_gain_2g_table = cali_get_pow_cal_gain_2g_table(pow_table_idx);
        dbb_pre_table = cali_get_dbb_pre_table(pow_table_idx);
        if (pow_cal_gain_2g_table == NULL || pow_dist_2g_table == NULL) {
            continue;
        }
        cali_pow_set_tpc_mem_reg(device, pow_cal_gain_2g_table, pow_dist_2g_table,
            dbb_pre_table, (pow_table_idx == 0 ? OAL_FALSE : OAL_TRUE)); /* 第一个元素是ofdm */
    }
    fe_hal_pow_write_tpc_mem_finish();
}

/*
 * 函 数 名  : cali_pow_initialize_tx_power
 * 功能描述  : 初始化tx power的硬件设置
 * 1.日    期  : 2020年04月13日
 *   修改内容  : 新生成函数
 */
osal_void cali_pow_initialize_tx_power(hal_to_dmac_device_stru *hal_to_dmac_device)
{
    hal_device_stru *device = (hal_device_stru *)hal_to_dmac_device;
    /* 初始化二级映射表 */
    cali_pow_init_pwr_code(device);
    /* 初始化PHY寄存器表单TPC mem */
    cali_refresh_phy_tpc_mem_reg(device, WLAN_BAND_2G);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
