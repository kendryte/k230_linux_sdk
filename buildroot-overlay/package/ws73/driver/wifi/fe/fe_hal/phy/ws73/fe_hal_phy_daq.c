/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: fe_hal_phy_daq.c
 * Create: 2023-01-16
 */
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "fe_hal_phy_daq.h"
#include "fe_hal_phy_reg_if_host.h"
#include "hal_soc_reg.h"
#include "hh503_phy_reg.h"
#ifdef _PRE_WLAN_ATE
#include "hal_ext_if_rom.h"
#else
#include "hal_ext_if.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_DAQ_PHY_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST
#define HAL_DAQ_REG_SIZE   (0x4)       // 寄存器长度 4字节
/* ============== PHY 数采接口 起始 ============ */
osal_void hal_sample_free_msg(osal_void)
{
    return;
}
osal_u32 hal_sample_alloc_msg(osal_u16 data_len)
{
    unref_param(data_len);
    return OAL_SUCC;
}
// 按照ram bit位配置维测空间
osal_void hal_config_chip_test_set_diag_ram(osal_u32 ram_bits)
{
    osal_u32 share_cfg = hal_reg_read(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG);
    osal_u32 sample_start_addr = HAL_DAQ_PHY_SAMPLE_MSG_ADDR;  // 维测起始地址
    osal_u32 ram_addr_offset = 0x8000;  // 每一块ram的大小
#ifndef BOARD_FPGA_WIFI
    hal_reg_write(DIAG_CTL_RB_CFG_HIGH_CLK_MODE_REG, 0x0);
#endif
    hal_reg_write(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, (share_cfg & (~ram_bits)));
    // 配置对应ram的起始地址 并配置使用功能为diag
    if ((ram_bits & BIT0) == BIT0) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM0_START_ADDR_REG, sample_start_addr);
        sample_start_addr += ram_addr_offset;
        hal_reg_write(CLDO_CTL_REG_BANK_RAM0_USE_CFG_REG, 0x2);
    }
    if ((ram_bits & BIT1) == BIT1) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM1_START_ADDR_REG, sample_start_addr);
        sample_start_addr += ram_addr_offset;
        hal_reg_write(CLDO_CTL_REG_BANK_RAM1_USE_CFG_REG, 0x2);
    }
    if ((ram_bits & BIT2) == BIT2) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM2_START_ADDR_REG, sample_start_addr);
        sample_start_addr += ram_addr_offset;
        hal_reg_write(CLDO_CTL_REG_BANK_RAM2_USE_CFG_REG, 0x2);
    }
    if ((ram_bits & BIT3) == BIT3) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM3_START_ADDR_REG, sample_start_addr);
        sample_start_addr += ram_addr_offset;
        hal_reg_write(CLDO_CTL_REG_BANK_RAM3_USE_CFG_REG, 0x2);
    }
    hal_reg_write(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, share_cfg);
}
// 按照ram bit位配置维测空间 恢复为cpu
osal_void hal_config_chip_test_recover_diag_ram(osal_u32 ram_bits)
{
    osal_u32 share_cfg = hal_reg_read(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG);
    hal_reg_write(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, (share_cfg & (~ram_bits)));
    // 配置对应ram的起始地址 并配置使用功能为diag
    if ((ram_bits & BIT0) == BIT0) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM0_USE_CFG_REG, 0x0);
        hal_reg_write(CLDO_CTL_REG_BANK_RAM0_START_ADDR_REG, 0x420000);
    }
    if ((ram_bits & BIT1) == BIT1) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM1_USE_CFG_REG, 0x0);
        hal_reg_write(CLDO_CTL_REG_BANK_RAM1_START_ADDR_REG, 0x428000);
    }
    if ((ram_bits & BIT2) == BIT2) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM2_USE_CFG_REG, 0x0);
        hal_reg_write(CLDO_CTL_REG_BANK_RAM2_START_ADDR_REG, 0x430000);
    }
    if ((ram_bits & BIT3) == BIT3) {
        hal_reg_write(CLDO_CTL_REG_BANK_RAM3_USE_CFG_REG, 0x0);
        hal_reg_write(CLDO_CTL_REG_BANK_RAM3_START_ADDR_REG, 0x438000);
    }
    hal_reg_write(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, share_cfg);
}
osal_void hal_config_chip_test_set_diag_param_phy_recovery(hal_to_dmac_device_mac_test_stru *hal_mac_device)
{
    // 触发模式需要配置PHY寄存器
    if (hal_mac_device->diag_param.diag_mode != 0) {
        hal_set_sample_cfg_0_cfg_phy_data_sample_clk_en(0x0);
        hal_set_sample_cfg_0_cfg_debug_sample_en(0x0);
        hal_set_wlbb_clr_sample_clear_sample_done(0x0);
        hal_set_wlbb_clr_sample_clear_sample_done(0x1);
        hal_set_wlbb_clr_sample_clear_sample_done(0x0);
    }
    hal_reg_write(DIAG_CTL_RB_CFG_MONITOR_CLOCK_REG, 0x0);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_GEN_SYNC_REG, 0x0);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_EN_REG, 0x0);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_MODE_REG, 0x0);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_NODE_SEL_REG, 0x0);
    hal_mac_device->diag_param.diag_mode = 0;
    hal_mac_device->diag_param.diag_basic_start_addr = 0;
    hal_mac_device->diag_param.diag_basic_end_addr = 0;
    hal_mac_device->diag_param.diag_read_addr = 0;
    hal_mac_device->diag_param.diag_sample_addr = 0;
    hal_mac_device->diag_param.diag_source = 0;
    hal_mac_device->diag_param.diag_cycle = 0;
    /* 事件上报配置phy寄存器 */
    if (hal_mac_device->diag_param.diag_phy_event_en != 0) {
        hal_set_sample_cfg_0_cfg_phy_data_sample_clk_en(0x0);
        hal_set_sample_cfg_0_cfg_phy_data_sample_cg_bps(0x0);
        hal_reg_write(HH503_PHY_BANK1_BASE_0x268, 0x0);
        hal_reg_write(DIAG_CTL_RB_DIAG_INT_EN_REG, 0x0);
#ifdef BOARD_FPGA_WIFI
        hal_reg_write(DIAG_CTL_RB_CFG_HIGH_CLK_MODE_REG, 0x0);
        hal_reg_write(CLK_RST_CRG_CFG_SYSLDO_CRG_CKSEL_CTL_REG, 0x0);
#endif
    }
#ifndef _PRE_WLAN_ATE
    hal_config_chip_test_recover_diag_ram(BIT3);
#endif
}

OSAL_STATIC osal_void hal_config_chip_test_set_diag_param_phy_begin_debug(hal_to_phy_test_diag_stru *param)
{
    hal_set_sample_cfg_2_cfg_debug_sample_by_len(param->cfg_debug_sample_len);
    hal_set_sample_cfg_0_cfg_debug_sample_start_mode(param->cfg_debug_sample_start_mode);
    // (触发)定长下end mode无效 配置为0
    param->cfg_debug_sample_end_mode = (param->cfg_debug_sample_len != 0) ? 0x0 : param->cfg_debug_sample_end_mode;
    hal_set_sample_cfg_0_cfg_debug_sample_end_mode(param->cfg_debug_sample_end_mode);
    // 拓展模式 直接配置tsf mac和延时 否则通过寄存器配置
    if (param->extend_mode != 0) {
        hal_set_sample_cfg_2_cfg_with_sample_delay_en(param->cfg_with_sample_delay_en);
        hal_set_sample_cfg_2_cfg_debug_sample_delay(param->cfg_debug_sample_delay);
        hal_set_sample_cfg_2_cfg_with_mac_tsf_en(param->cfg_with_mac_tsf_en);
        hal_set_sample_cfg_2_cfg_with_mac_info_en(param->cfg_with_mac_info_en);
    }
    hal_set_sample_cfg_0_cfg_debug_sample_en(0x0);
    // 进行触发条件的配置 后续适配
    hal_set_sample_cfg_0_cfg_debug_sample_en(0x1);
    // 非定长需要拉低cfg_debug_sample_en
    if (param->cfg_debug_sample_len == 0) {
        hal_set_sample_cfg_0_cfg_debug_sample_en(0x0);
    }
}

static osal_void hal_config_chip_test_set_diag_param_phy_event(hal_to_phy_test_diag_stru *param)
{
    hal_set_sample_cfg_0_cfg_phy_data_sample_clk_en(0x1);
    hal_set_sample_cfg_0_cfg_phy_data_sample_cg_bps(0x1);
    hal_reg_write(HH503_PHY_BANK1_BASE_0x268, param->event_rpt_addr); /* 配置事件上报 */
    hal_reg_write(DIAG_CTL_RB_DIAG_INT_EN_REG, 0x2);
}

osal_void hal_config_chip_test_set_diag_param_phy_begin(hal_to_phy_test_diag_stru *param,
    hal_to_dmac_device_mac_test_stru *hal_mac_device)
{
    // 使用ram3的作为数采空间 DIAG_CTL维测功能使用手册.docx 信息表 DIAG_RAM 地址 当前给定32K
    const osal_u32 sample_start_addr = HAL_DAQ_PHY_SAMPLE_MSG_ADDR;
    const osal_u32 sample_end_addr = (param->cfg_sample_length != 0) ?
        (sample_start_addr + HAL_DAQ_REG_SIZE * (param->cfg_sample_length - 1)) : (sample_start_addr + 0x7FFC);
    oal_bool_enum_uint8 is_debug = (param->cfg_sample_mode != 0) ? OSAL_TRUE : OSAL_FALSE;

    // 触发模式需要配置PHY寄存器
    if (is_debug) {
        hal_set_wlbb_clr_sample_clear_sample_done(0x0);
        hal_set_wlbb_clr_sample_clear_sample_done(0x1);
        hal_set_wlbb_clr_sample_clear_sample_done(0x0);
        hal_set_sample_cfg_0_cfg_phy_data_sample_clk_en(0x1);
    } else {
        hal_set_sample_cfg_0_cfg_phy_data_sample_cg_bps(0x1);
    }
#ifndef _PRE_WLAN_ATE
    hal_config_chip_test_set_diag_ram(BIT3);    // 配置ram给维测 业务上使用ram3 ate单独配置
#endif
    // 刷新版本后使用低频配置ram 高频执行数采
    hal_reg_write(DIAG_CTL_RB_CFG_HIGH_CLK_MODE_REG, 0x1);
#ifdef BOARD_FPGA_WIFI
    hal_reg_write(CLK_RST_CRG_CFG_SYSLDO_CRG_CKSEL_CTL_REG, 0x0);
    hal_reg_write(DIAG_CTL_RB_CFG_PKT_WR_RD_SEL_REG, 0xff);
#else
    hal_reg_write_bits(CLK_RST_CRG_CFG_SYSLDO_CRG_CKSEL_CTL_REG, NUM_3_BITS, NUM_2_BITS, 0);  // 选择phy时钟
#endif
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_GEN_SYNC_REG, 0x0);
    hal_reg_write(DIAG_CTL_RB_CFG_MONITOR_CLOCK_REG, 0x7);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_SEL_REG, 0x1);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_NODE_SEL_REG, param->cfg_sample_node_sel);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_MODE_REG, is_debug);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_LENGTH_REG, param->cfg_sample_length);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_START_ADDR_REG, sample_start_addr);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_END_ADDR_REG, sample_end_addr);
    hal_mac_device->diag_param.diag_basic_start_addr = hal_reg_read(DIAG_CTL_RB_CFG_SAMPLE_START_ADDR_REG);
    hal_mac_device->diag_param.diag_basic_end_addr = hal_reg_read(DIAG_CTL_RB_CFG_SAMPLE_END_ADDR_REG);
    hal_mac_device->diag_param.diag_mode = is_debug;
    hal_mac_device->diag_param.diag_cycle = is_debug;
    hal_mac_device->diag_param.diag_phy_event_en = param->event_rpt_addr & 0x1;
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_GEN_SYNC_REG, 0x1);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_EN_REG, 0x1);

    if (is_debug) {
        hal_config_chip_test_set_diag_param_phy_begin_debug(param);
    }

    /* phy事件上报开启 */
    if (hal_mac_device->diag_param.diag_phy_event_en != 0) {
        hal_config_chip_test_set_diag_param_phy_event(param);
    }
}

oal_bool_enum_uint8 hal_config_chip_test_set_diag_param_phy_query(hal_to_dmac_device_mac_test_stru *hal_mac_device)
{
    osal_u32 sample_done = hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_REG);
    if ((sample_done & 0x1) == 0x1) {
        hal_mac_device->diag_param.diag_sample_addr = hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_ADDR_REG); // 读取结束地址
        // 循环数采 起始读取地址是采样结束地址的下一个地址
        if (hal_mac_device->diag_param.diag_cycle != 0) {
            hal_mac_device->diag_param.diag_read_addr = hal_mac_device->diag_param.diag_sample_addr + HAL_DAQ_REG_SIZE;
            // 结束在尾部 那么读取start到end的数据
            if (hal_mac_device->diag_param.diag_read_addr > hal_mac_device->diag_param.diag_basic_end_addr) {
                hal_mac_device->diag_param.diag_read_addr = hal_mac_device->diag_param.diag_basic_start_addr;
            } else {
                // 结束不在尾部 那么起始读取采样结束地址+4 到 采样结束地址(这里处理为起始读取地址+实际内存长度
                // 在读取时需要处理长度)
                hal_mac_device->diag_param.diag_sample_addr = hal_mac_device->diag_param.diag_sample_addr +
                    HAL_DAQ_REG_SIZE - hal_mac_device->diag_param.diag_basic_start_addr +
                    hal_mac_device->diag_param.diag_basic_end_addr;
            }
        } else {
            hal_mac_device->diag_param.diag_read_addr = hal_mac_device->diag_param.diag_basic_start_addr;
        }
        // 刷新版本后 数采结束恢复低频 获取数据
        hal_reg_write(DIAG_CTL_RB_CFG_HIGH_CLK_MODE_REG, 0x0);
#ifdef BOARD_FPGA_WIFI
        hal_reg_write(CLK_RST_CRG_CFG_SYSLDO_CRG_CKSEL_CTL_REG, 0x10);
#else
        hal_reg_write_bits(CLK_RST_CRG_CFG_SYSLDO_CRG_CKSEL_CTL_REG, NUM_3_BITS, NUM_2_BITS, 0x2);  // 恢复总线时钟
#endif
        return OAL_TRUE;
    } else {
        // 获取当前存储数据所在地址
        hal_mac_device->diag_param.diag_sample_addr = hal_reg_read(DIAG_CTL_RB_PKE_MEM_OBS_SAMPLE_ADDR_REG);
        return OAL_FALSE;
    }
}
osal_u32 hal_config_chip_test_set_diag_param_phy_save_data(osal_u8 *data, osal_u32 data_len,
    hal_to_dmac_device_mac_test_stru *hal_mac_device)
{
    osal_u32 value;
    osal_u32 diag_real_addr = 0x0;
    osal_u32 offset = 0;

    if (!data || data_len == 0) {
        return offset;
    }

    while (hal_mac_device->diag_param.diag_read_addr <= hal_mac_device->diag_param.diag_sample_addr) {
        diag_real_addr =
            (hal_mac_device->diag_param.diag_read_addr > hal_mac_device->diag_param.diag_basic_end_addr) ?
            hal_mac_device->diag_param.diag_read_addr - hal_mac_device->diag_param.diag_basic_end_addr +
            hal_mac_device->diag_param.diag_basic_start_addr :
            hal_mac_device->diag_param.diag_read_addr;
        value = hal_reg_read(diag_real_addr);
        hal_mac_device->diag_param.diag_read_addr += HAL_DAQ_REG_SIZE;
        offset = offset % data_len;
        if (memcpy_s(data + offset, data_len - offset, &value, HAL_DAQ_REG_SIZE) != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "{hal_config_chip_test_set_diag_param_phy_save_data memcpy_s: failed!}");
            break;
        }
        /* 装满内存空间 返回 */
        offset += HAL_DAQ_REG_SIZE;
        if (offset >= data_len) {
            break;
        }
    }
    return offset;
}
/* ============== PHY 数采接口 结束 ============ */
// 事件上报依赖的维测中断使能开关
osal_void hal_config_diag_int_switch(osal_u8 flag)
{
    if (flag == OAL_FALSE) {
        hal_reg_write(DIAG_CTL_RB_DIAG_INT_EN_REG, 0x0);
    } else {
        hal_reg_write(DIAG_CTL_RB_DIAG_INT_EN_REG, 0x2);
    }
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // _PRE_WLAN_FEATURE_DAQ
