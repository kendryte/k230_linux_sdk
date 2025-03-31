/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: abb spi reg header file.
 */

#ifndef __ABB_SPI_REG_RW_H__
#define __ABB_SPI_REG_RW_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "osal_types.h"
#include "hal_ext_if_rom.h"
#include "hal_soc_rom.h"
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define FPGA_CTL0_SSI0_ADDRESS          (HH503_SSI0_MST_RB_BASE_ADDR)
#define FPGA_CTL0_SSI1_ADDRESS          (HH503_SSI1_MST_RB_BASE_ADDR)

#define SPI_WRITE_MODE                  (0x0000)
#define SPI_READ_MODE                   (0x0001)

static inline osal_void hal_reg_write_base16(osal_u32 base, osal_u32 reg, osal_u32 value)
{
    hal_reg_write16(base + reg, value);
}

static inline osal_void hal_reg_setbits(osal_u32 addr, osal_u32 pos, osal_u32 bits, osal_u32 value)
{
    osal_u32 set_val;

    set_val = hal_reg_read(addr);
    osal_write_bits(&set_val, value, bits, pos);
    hal_reg_write16(addr, set_val);
}

osal_void ad9516_initi_cfg_enable(osal_u32 ssi_addr, osal_u32 spi_addr);
osal_void ad9516_config_reg_value_32(osal_u32 spi_addr, osal_u16 wr_high_16b_instru,
    osal_u16 low_16b_data);
osal_void sn1201019_initi_cfg_enable(osal_u32 ssi_addr, osal_u32 spi_addr);
osal_void sn1201019_config_reg_value_16(osal_u32 spi_addr, osal_u16 wr_instruct_and_data);
osal_void dac5689_initi_cfg_enable(osal_u32 ssi_addr, osal_u32 spi_addr);

osal_void dac5689_config_reg_value_16(osal_u32 spi_addr, osal_u16 wr_instruct_and_data);
osal_void dac5689_ch0_init(osal_u32 ssi_addr, osal_u32 spi_addr);
osal_void dac5689_ch1_init(osal_u32 ssi_addr, osal_u32 spi_addr);
osal_void sn1201019_ch0_init(osal_u32 ssi_addr, osal_u32 spi_addr);
osal_void sn1201019_ch1_init(osal_u32 ssi_addr, osal_u32 spi_addr);

#endif
