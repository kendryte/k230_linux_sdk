/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  ssi_master_regbank_reg_offset.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:05 Create file
 */

#ifndef __SSI_MASTER_REGBANK_REG_OFFSET_H__
#define __SSI_MASTER_REGBANK_REG_OFFSET_H__

/* SSI_MASTER_REGBANK Base address of Module's Register */
#define SSI_MASTER_REGBANK_BASE                       (0x0)

/******************************************************************************/
/*             SSI_MASTER_REGBANK Registers' Definitions                      */
/******************************************************************************/

#define SSI_MASTER_REGBANK_SM_BASE_HADDR_REG  (SSI_MASTER_REGBANK_BASE + 0x0)  /* 读写ssi_master外部高16位地址的配置寄存器 */
#define SSI_MASTER_REGBANK_AHB_SIZE_16_32_REG (SSI_MASTER_REGBANK_BASE + 0x2)  /* ahb总线操作位宽的配置寄存器 */
#define SSI_MASTER_REGBANK_SM_BIAS_ADDR_REG   (SSI_MASTER_REGBANK_BASE + 0x4)  /* ssi外部低16位地址的配置寄存器 */
#define SSI_MASTER_REGBANK_SM_STATUS_REG      (SSI_MASTER_REGBANK_BASE + 0x6)  /* 记录读/写AHB的状态 */
#define SSI_MASTER_REGBANK_SM_ERR_HADDR_REG   (SSI_MASTER_REGBANK_BASE + 0x8)  /* 读写AHB发生错误时，记录发生错误的高16比特地址 */
#define SSI_MASTER_REGBANK_SM_ERR_LADDR_REG   (SSI_MASTER_REGBANK_BASE + 0xA)  /* 读写AHB发生错误时，记录发生错误的低6比特地址 */
#define SSI_MASTER_REGBANK_SM_CLEAR_REG       (SSI_MASTER_REGBANK_BASE + 0xC)  /* ssi状态寄存器写清 */
#define SSI_MASTER_REGBANK_AON_CLKSEL_REG     (SSI_MASTER_REGBANK_BASE + 0xE)  /* AON时钟选择 */
#define SSI_MASTER_REGBANK_SEL_CTRL_REG       (SSI_MASTER_REGBANK_BASE + 0x10) /* 复位、使能信号选择 */
#define SSI_MASTER_REGBANK_SSI_CTRL_REG       (SSI_MASTER_REGBANK_BASE + 0x12) /* ssi复位、使能各sub */
#define SSI_MASTER_REGBANK_SSI_DIAG_REG       (SSI_MASTER_REGBANK_BASE + 0x18) /* 保留寄存器 */
#define SSI_MASTER_REGBANK_SSI_LOCK_REG_REG   (SSI_MASTER_REGBANK_BASE + 0x14) /* SSI锁定寄存器 */
#define SSI_MASTER_REGBANK_SSI_SYS_CTL_ID_REG (SSI_MASTER_REGBANK_BASE + 0x1C) /* SYS CTL ID寄存器 */
#define SSI_MASTER_REGBANK_SSI_GP_REG0_REG    (SSI_MASTER_REGBANK_BASE + 0x20) /* 通用寄存器 */
#define SSI_MASTER_REGBANK_SSI_GP_REG1_REG    (SSI_MASTER_REGBANK_BASE + 0x24) /* 通用寄存器 */
#define SSI_MASTER_REGBANK_SSI_GP_REG2_REG    (SSI_MASTER_REGBANK_BASE + 0x28) /* 通用寄存器 */
#define SSI_MASTER_REGBANK_SSI_GP_REG3_REG    (SSI_MASTER_REGBANK_BASE + 0x2C) /* 通用寄存器 */
#define SSI_MASTER_REGBANK_SSI_RDATA_L_REG    (SSI_MASTER_REGBANK_BASE + 0x30) /* 通用寄存器 */
#define SSI_MASTER_REGBANK_SSI_RDATA_H_REG    (SSI_MASTER_REGBANK_BASE + 0x34) /* 通用寄存器 */

#endif // __SSI_MASTER_REGBANK_REG_OFFSET_H__
