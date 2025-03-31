/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: host platform dyn mem source file
 * Author: Huanghe
 * Create: 2021-06-08
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_firmware.h"
#include "plat_debug.h"
#include "plat_pm_board.h"
#include "plat_pm.h"
#include "ini.h"
#include "plat_cali.h"

#include <linux/version.h>
#endif
#include "soc_osal.h"
#include "hal_soc_reg.h"
#include "hcc_bus.h"
#include "hcc_if.h"
#include "efuse_opt.h"
#include "plat_hcc_msg_type.h"
#include "plat_hcc_srv.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
custom_cali_func g_custom_cali_func = NULL;
EXPORT_SYMBOL_GPL(g_custom_cali_func);
custom_cali_func g_custom_hmac_exit_later_func = NULL;
EXPORT_SYMBOL_GPL(g_custom_hmac_exit_later_func);

void set_custom_exit_later_func(custom_cali_func func)
{
    g_custom_hmac_exit_later_func = func;
}

custom_cali_func get_custom_exit_later_func(void)
{
    return g_custom_hmac_exit_later_func;
}

void set_custom_cali_func(custom_cali_func func)
{
    g_custom_cali_func = func;
}

custom_cali_func get_custom_cali_func(void)
{
    return g_custom_cali_func;
}

EXPORT_SYMBOL(set_custom_exit_later_func);
EXPORT_SYMBOL(set_custom_cali_func);
#endif

#define XO_TRIM_TEMP_COMP_NUM 8             // 频偏粗调温补值 20℃一档 ~-20,0,20,40,60,80,100,~
#define XO_TRIM_TEMP_COMP_TIMER_PERIOD 2000 // 频偏粗调温补定时器间隔 ms
#define XO_TRIM_COARSE_VAL_MAX 15           // 频偏粗调补偿最大值
osal_timer g_xo_trim_temp_timer = {0};

static td_s32 hwifi_config_get_efuse_temp_gear(td_u8 *temp_gear)
{
    td_u64 state = 0;
    if (uapi_efuse_get_lockstat(&state) != OAL_SUCC) {
        return INI_FAILED;
    }
    if ((((td_u64)1 << (EXT_EFUSE_LOCK_XO_PPM_1_ID - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
        if (uapi_efuse_read(EXT_EFUSE_XO_PPM_TEMPERATURE_2_ID, temp_gear, sizeof(td_u8)) != OAL_SUCC) {
            return INI_FAILED;
        }
    } else if ((((td_u64)1 << (EXT_EFUSE_LOCK_CMU_XO_TRIM_COARSE_ID - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
        if (uapi_efuse_read(EXT_EFUSE_XO_PPM_TEMPERATURE_1_ID, temp_gear, sizeof(td_u8)) != OAL_SUCC) {
            return INI_FAILED;
        }
    }
    return INI_SUCC;
}

td_void hwifi_config_get_cmu_xo_trim_temp_comp(td_void)
{
    td_u8 cmu_xo_trim_temp_param[XO_TRIM_TEMP_COMP_NUM] = {5, 9, 9, 8, 7, 6, 6, 9};
    td_s16 xo_trim_coarse;
    td_s16 xo_trim_diff;
    td_s16 xo_trim_temp;
    td_u8 temp_gear = 7; // efuse保存温度档位 10℃一档 7档表示30~40
    td_u8 i;
    u_cmu_xo_sig val;
#ifndef CONFIG_NO_SUPPORT_INI
    td_s32 ret;
    td_u8 param_len = 0;
    td_u8 ini_str[INI_READ_VALUE_LEN] = {0};
    ret = get_cust_conf_string_etc(INI_MODU_PLAT, "cmu_xo_trim_temp", ini_str, INI_READ_VALUE_LEN);
    if (ret != INI_SUCC) {
        return;
    }
    ret = ini_get_cust_item_list(ini_str, cmu_xo_trim_temp_param, INI_PARAM_BYTE_ONE, &param_len, INI_READ_VALUE_COUNT);
    if (ret != INI_SUCC || param_len != XO_TRIM_TEMP_COMP_NUM) {
        return;
    }
#endif
    if (hcc_read_reg(HCC_CHANNEL_AP, CMU_CTL_RB_CMU_XO_SIG_REG, &val.u32) != EXT_ERR_SUCCESS) {
        return;
    }

    if (hwifi_config_get_efuse_temp_gear(&temp_gear) != INI_SUCC) {
        return;
    }

    xo_trim_coarse = (td_u8)val.bits.rg_cmu_xo_trim_coarse;
    // 更新为补偿值索引 20℃一档
    temp_gear = ((temp_gear >> 1) >= XO_TRIM_TEMP_COMP_NUM) ? XO_TRIM_TEMP_COMP_NUM - 1 : (temp_gear >> 1);
    xo_trim_diff = (td_s16)cmu_xo_trim_temp_param[temp_gear] - xo_trim_coarse;
    for (i = 0; i < XO_TRIM_TEMP_COMP_NUM; i++) {
        xo_trim_temp = (td_s16)cmu_xo_trim_temp_param[i] - xo_trim_diff; // 取值范围0-15
        xo_trim_temp =
            (xo_trim_temp < 0) ? 0 : (xo_trim_temp > XO_TRIM_COARSE_VAL_MAX) ? XO_TRIM_COARSE_VAL_MAX : xo_trim_temp;
        cmu_xo_trim_temp_param[i] = xo_trim_temp;
    }
    plat_msg_hcc_send(cmu_xo_trim_temp_param, sizeof(cmu_xo_trim_temp_param), H2D_PLAT_CFG_MSG_XO_TRIM_TEMP_INI);
}

static td_void hwifi_update_cmu_xo_trim_temp_comp_timer_handler(uintptr_t data)
{
    td_u8 temp_gear = 7; // efuse保存温度档位 10℃一档 7档表示30~40

    plat_msg_hcc_send((td_u8 *)&temp_gear, sizeof(temp_gear), H2D_PLAT_CFG_MSG_UPDATE_XO_TRIM);
    osal_timer_mod(&g_xo_trim_temp_timer, XO_TRIM_TEMP_COMP_TIMER_PERIOD);
}

td_void hwifi_update_cmu_xo_trim_temp_comp_timer(td_void)
{
    // 定时更新
    g_xo_trim_temp_timer.handler = hwifi_update_cmu_xo_trim_temp_comp_timer_handler;
    g_xo_trim_temp_timer.interval = XO_TRIM_TEMP_COMP_TIMER_PERIOD;
    osal_timer_init(&g_xo_trim_temp_timer);
    osal_timer_start(&g_xo_trim_temp_timer);
}

#ifdef CONFIG_NO_SUPPORT_INI
osal_u8 hwifi_custom_get_xo_trim_temp_comp_sw(osal_void)
{
    return CONFIG_INI_XO_TRIM_TEMP_COMP_SW;
}
td_void hwifi_config_cmu_xo_trim_temp_comp(td_void)
{
    // 定制化开启频偏温度补偿时 刷新温补数据 并启动定时器 随温度刷新
    if (hwifi_custom_get_xo_trim_temp_comp_sw() == OSAL_TRUE) {
        hwifi_config_get_cmu_xo_trim_temp_comp();
        hwifi_update_cmu_xo_trim_temp_comp_timer();
    }
}
#endif

#endif

td_void cali_dyn_mem_cfg(td_void)
{
    hcc_switch_status(HCC_OFF);
    // 0x33: 0011_0011b close RAM2&RAM3's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x33);
    // 0x0: bank ram2 use as CPURAM
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM2_USE_CFG_REG, 0x0);
    // 0x0: bank ram3 use as CPURAM
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM3_USE_CFG_REG, 0x0);
    // 0x430000: cfg bank ram2 START_ADDR
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM2_START_ADDR_REG, 0x430000);
    // 0x438000: cfg bank ram3 START_ADDR
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM3_START_ADDR_REG, 0x438000);
    // 0x3F: 0011_1111b restore RAM2&RAM3's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x3F);
    hcc_switch_status(HCC_BUS_FORBID);
}

td_void bsle_cali_dyn_mem_cfg(td_void)
{
    hcc_switch_status(HCC_OFF);
    // 0x2F: 0010_1111b close RAM4's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x2F);
    // 0x0: bank ram4 use as CPURAM
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM4_USE_CFG_REG, 0x0);
    // 0x440000: cfg bank ram4 START_ADDR
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM4_START_ADDR_REG, 0x440000);
    // 0x3F: 0011_1111b restore RAM4's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x3F);
    hcc_switch_status(HCC_BUS_FORBID);
}
