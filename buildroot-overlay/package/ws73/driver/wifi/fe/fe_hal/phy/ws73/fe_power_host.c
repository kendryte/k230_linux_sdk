/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy功能接口
 */
#if defined(_PRE_WLAN_ATE) || defined(_PRE_WIFI_EDA)
#include "hal_device_rom.h"
#include "fe_hal_phy_reg_if_pktmem.h"
#else
#include "fe_hal_phy_reg_if_host.h"
#include "hal_device.h"
#endif
#include "hal_phy.h"
#include "fe_power_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define HE_ER_SU_POWER_BOOST_POW_COMP 23    // 开启powerboost之后帧头功率高3dB 实测是2.3dB 单位0.1dB
#ifdef _PRE_WLAN_ONLINE_CALI
/*****************************************************************************
 函 数 名  : fe_hal_phy_config_online_pow
 功能描述  : 配置动态功率
*****************************************************************************/
osal_void fe_hal_phy_config_online_pow(const hal_device_stru *device)
{
    unref_param(device);
    hal_set_dyn_tx_power_cali_dyn_tx_power_cali(0x1);      // 动态功率校准使能
    hal_set_tx_power_accum_delay_power_accum_delay(0x200);   // 累加器延时512
    hal_set_cali_new_add_reg_cfg_fb_pwr_calc_prd(0x2);  // 累加时间选择范围在L-STF 和 L-LTF 考虑ersu能量高3dB情况 否则可能统计能量偏低
}

/*****************************************************************************
 函 数 名  : fe_hal_phy_recover_online_pow
 功能描述  : 恢复动态功率
*****************************************************************************/
osal_void fe_hal_phy_recover_online_pow(const hal_device_stru *device)
{
    unref_param(device);
    // 累加的配置不恢复 避免出现校准后再次校准的首帧累加能量不准确
    hal_set_dyn_tx_power_cali_dyn_tx_power_cali(0x0);        // 动态功率校准使能
}
#endif
#ifndef _PRE_WIFI_EDA
osal_u32 fe_hal_phy_get_single_ppa_lut(const hal_device_stru *device, osal_u16 index)
{
    unref_param(device);
    if (index == 0x0) {
        return hal_get_rf_ppa_code_lut0_cfg_rf_ppa_code_lut0();
    } else if (index == 0x1) {
        return hal_get_rf_ppa_code_lut0_cfg_rf_ppa_code_lut1();
    } else if (index == 0x2) {
        return hal_get_rf_ppa_code_lut1_cfg_rf_ppa_code_lut2();
    } else if (index == 0x3) {
        return hal_get_rf_ppa_code_lut1_cfg_rf_ppa_code_lut3();
    } else if (index == 0x4) {
        return hal_get_rf_ppa_code_lut2_cfg_rf_ppa_code_lut4();
    } else if (index == 0x5) {
        return hal_get_rf_ppa_code_lut2_cfg_rf_ppa_code_lut5();
    } else if (index == 0x6) {
        return hal_get_rf_ppa_code_lut3_cfg_rf_ppa_code_lut6();
    } else if (index == 0x7) {
        return hal_get_rf_ppa_code_lut3_cfg_rf_ppa_code_lut7();
    } else if (index == 0x8) {
        return hal_get_rf_ppa_code_lut4_cfg_rf_ppa_code_lut8();
    } else if (index == 0x9) {
        return hal_get_rf_ppa_code_lut4_cfg_rf_ppa_code_lut9();
    } else if (index == 0xa) {
        return hal_get_rf_ppa_code_lut5_cfg_rf_ppa_code_lut10();
    }
    return 0x0;
}

osal_void fe_hal_phy_set_single_ppa_lut(const hal_device_stru *device, osal_u16 upc_code, osal_u16 index)
{
    unref_param(device);
    if (index == 0x0) {
        hal_set_rf_ppa_code_lut0_cfg_rf_ppa_code_lut0(upc_code);
    } else if (index == 0x1) {
        hal_set_rf_ppa_code_lut0_cfg_rf_ppa_code_lut1(upc_code);
    } else if (index == 0x2) {
        hal_set_rf_ppa_code_lut1_cfg_rf_ppa_code_lut2(upc_code);
    } else if (index == 0x3) {
        hal_set_rf_ppa_code_lut1_cfg_rf_ppa_code_lut3(upc_code);
    } else if (index == 0x4) {
        hal_set_rf_ppa_code_lut2_cfg_rf_ppa_code_lut4(upc_code);
    } else if (index == 0x5) {
        hal_set_rf_ppa_code_lut2_cfg_rf_ppa_code_lut5(upc_code);
    } else if (index == 0x6) {
        hal_set_rf_ppa_code_lut3_cfg_rf_ppa_code_lut6(upc_code);
    } else if (index == 0x7) {
        hal_set_rf_ppa_code_lut3_cfg_rf_ppa_code_lut7(upc_code);
    } else if (index == 0x8) {
        hal_set_rf_ppa_code_lut4_cfg_rf_ppa_code_lut8(upc_code);
    } else if (index == 0x9) {
        hal_set_rf_ppa_code_lut4_cfg_rf_ppa_code_lut9(upc_code);
    } else if (index == 0xa) {
        hal_set_rf_ppa_code_lut5_cfg_rf_ppa_code_lut10(upc_code);
    }
}
#endif

osal_void fe_hal_phy_set_single_pa_lut(const hal_device_stru *device, osal_u16 upc_code, osal_u16 index)
{
    unref_param(device);
    if (index == 0x0) {
        hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut0(upc_code);
    } else if (index == 0x1) {
        hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut1(upc_code);
    } else if (index == 0x2) {
        hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut2(upc_code);
    } else if (index == 0x3) {
        hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut3(upc_code);
    } else if (index == 0x4) {
        hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut4(upc_code);
    } else if (index == 0x5) {
        hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut5(upc_code);
    } else if (index == 0x6) {
        hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut6(upc_code);
    } else if (index == 0x7) {
        hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut7(upc_code);
    } else if (index == 0x8) {
        hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut8(upc_code);
    } else if (index == 0x9) {
        hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut9(upc_code);
    } else if (index == 0xa) {
        hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut10(upc_code);
    } else if (index == 0xb) {
        hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut11(upc_code);
    } else if (index == 0xc) {
        hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut12(upc_code);
    } else if (index == 0xd) {
        hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut13(upc_code);
    } else if (index == 0xe) {
        hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut14(upc_code);
    } else if (index == 0xf) {
        hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut15(upc_code);
    }
}

// 写tpc之前的准备动作
osal_void fe_hal_pow_write_tpc_mem_prepare(void)
{
    hal_set_tpc_ram_access_reg1_cfg_tpc_ram_period(0x1);             // 打开读写使能
}

// 写tpc完成的配置动作
osal_void fe_hal_pow_write_tpc_mem_finish(void)
{
    hal_set_tpc_ram_access_reg1_cfg_tpc_ram_period(0x0);             // 关闭读写使能
}

/*
 * 函 数 名  : fe_hal_pow_write_single_tpc_mem
 * 功能描述  : 刷新一组PHY寄存器表单TPC mem
 * 1.日    期  : 2019年1月16日
 *   修改内容  : 新生成函数
 */
osal_void fe_hal_pow_write_single_tpc_mem(hal_device_stru *device, osal_u32 tpc_idx,
    osal_u32 pow_dist, oal_bool_enum_uint8 is_dsss)
{
    unref_param(device);
    unref_param(is_dsss);
    hal_set_tpc_ram_access_reg1_cfg_indirect_rw_mode(0x0);           // 写访问
    hal_set_tpc_ram_access_reg1_cfg_tpc_table_access_addr(tpc_idx);  // 地址为tpc code
    hal_set_tpc_ram_access_reg2_cfg_tpc_table_wr_data(pow_dist);     // 写入数据
}


/*
 * 函 数 名  : fe_hal_cfg_tpc_boundary
 * 功能描述  : 填写dbb scale shadow档位划分阈值
 */
osal_void fe_hal_cfg_tpc_boundary(const hal_device_stru *device, osal_u8 value, osal_u8 index)
{
    unref_param(device);
    if (index == 0x0) {
        hal_set_tpc_boundary_threshold1_cfg_tpc_boundary_0(value);
    } else if (index == 0x1) {
        hal_set_tpc_boundary_threshold1_cfg_tpc_boundary_1(value);
    } else if (index == 0x2) {
        hal_set_tpc_boundary_threshold1_cfg_tpc_boundary_2(value);
    } else if (index == 0x3) {
        hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_3(value);
    } else if (index == 0x4) {
        hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_4(value);
    } else if (index == 0x5) {
        hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_5(value);
    } else if (index == 0x6) {
        hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_6(value);
    }
}

/*
 * 函 数 名  : fe_hal_init_dbb_scale_shadow_to_0x80
 * 功能描述  : 根据档位index填写dbb scale shadow值
 */
osal_void fe_hal_init_dbb_scale_shadow_to_0x80(hal_device_stru *device)
{
    unref_param(device);
    hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_0(0x80);
    hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_1(0x80);
    hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_2(0x80);
    hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_3(0x80);
    hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_4(0x80);
    hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_5(0x80);
    hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_6(0x80);
    hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_7(0x80);
}

/*
 * 函 数 名  : fe_hal_cfg_dbb_scale_shadow
 * 功能描述  : 根据档位index填写dbb scale shadow值
 */
osal_void fe_hal_cfg_dbb_scale_shadow(hal_device_stru *device, osal_u8 value, osal_u8 index)
{
    unref_param(device);
    if (index == 0x0) {
        hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_0(value);
    } else if (index == 0x1) {
        hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_1(value);
    } else if (index == 0x2) {
        hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_2(value);
    } else if (index == 0x3) {
        hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_3(value);
    } else if (index == 0x4) {
        hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_4(value);
    } else if (index == 0x5) {
        hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_5(value);
    } else if (index == 0x6) {
        hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_6(value);
    } else if (index == 0x7) {
        hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_7(value);
    }
}

/*
 * 函 数 名  : fe_hal_get_dbb_scale_shadow
 * 功能描述  : 根据档位index读取dbb scale shadow值
 */
osal_u32 fe_hal_get_dbb_scale_shadow(hal_device_stru *device, osal_u8 index)
{
    unref_param(device);
    if (index == 0x0) {
        return hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_0();
    } else if (index == 0x1) {
        return hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_1();
    } else if (index == 0x2) {
        return hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_2();
    } else if (index == 0x3) {
        return hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_3();
    } else if (index == 0x4) {
        return hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_4();
    } else if (index == 0x5) {
        return hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_5();
    } else if (index == 0x6) {
        return hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_6();
    }
    return hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_7();
}
// 针对he er su 帧 开启powerboost时帧头功率LSTF LLTF偏高3dB 默认开启 因此开启时需要补偿3dB
osal_s16 fe_hal_rf_he_er_su_comp(osal_u8 protocol_mode)
{
    if (protocol_mode == WLAN_HE_EXT_SU_FORMAT) {   // WLAN_HE_EXT_SU_PPDU_TPYE
        if (hal_get_he_tx_foc_power_boost_win_ctrl_cfg_tx_power_boost_bypass() == 0x0) {
            return HE_ER_SU_POWER_BOOST_POW_COMP;
        }
    }
    return 0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
