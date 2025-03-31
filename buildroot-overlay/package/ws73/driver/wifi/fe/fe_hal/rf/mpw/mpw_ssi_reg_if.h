/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: mpw ssi 操作寄存器接口
 * Create: 2022-10-12
 */

#ifndef __MPW_SSI_REG_IF_H__
#define __MPW_SSI_REG_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "osal_types.h"
#include "fe_rf_dev.h"
#include "hal_soc_rom.h"
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef BUILD_UT
#define SSI0_BASE_ADDR (osal_u32)g_rf_ssi0_mst_rb_base_addr
#define SSI1_BASE_ADDR (osal_u32)g_rf_ssi0_mst_rb_base_addr + 0x400
#else
#define SSI0_BASE_ADDR              (HH503_SSI0_MST_RB_BASE_ADDR) // wifi用，控制MPW0
#define SSI1_BASE_ADDR              (HH503_SSI1_MST_RB_BASE_ADDR) // wifi用，控制MPW1
#endif
#define SSI2_BASE_ADDR              (0x48009000) // bt用，控制MPW2
#define SSI3_BASE_ADDR              (0x48009400) // gnss、fm用，控制MPW3

#define SSI_ADDR                    (0x8000)

#define SSI_ADDR_REG                (0x30)    // SSI读写地址寄存器
#define SSI_RW_REG                  (0x34)    // SSI读、标志寄存器
#define SSI_WDATA_REG               (0x38)    // SSI写数据寄存器
#define SSI_RDATA_REG               (0x3c)    // SSI读数据寄存器
#define SSI_TRANS_DONE              (0x40)    // SSI读写完成指示寄存器
#define SSI_RST_N                   (0x44)
#define SSI_RD_ERR                  (0x48)
#define SSI_HW                      (0x4c)

/* SSI读写配置 */
#define SSIREAD                 (1)
#define SSIWRITE                (0)

/* 设置读写开始标志 */
#define SSIWORKFLAG             (1)
#define SSIWORKDONEFLAG         (0)

#define COMMONWORK_FLAG_INIT    (0x0)
#define COMMONWORK_FLAG_DOING   (0x1)
#define COMMONWORK_FLAG_DONE    (0x2)

#define WIFI_SSI1_COMMONWORK_FLAG_INIT    (0x10)
#define WIFI_SSI1_COMMONWORK_FLAG_DOING   (0x11)
#define WIFI_SSI1_COMMONWORK_FLAG_DONE    (0x12)

typedef enum {
    IAM_WIFI = 0,
    IAM_BFGN = 1
} curr_sys_type;

osal_void ssi_write_reg(osal_u32 ssi_base_addr, osal_u32 addr_offset, osal_u16 val);
osal_s8 ssi_read_reg(osal_u32 ssi_base_addr, osal_u32 addr_offset, osal_u16 *read_val);
osal_void ssi_check_reg_rw(osal_u32 ssi_base_addr, osal_u32 addr_offset, osal_u16 value);
osal_void fe_mpw_read_reg(const hal_rf_dev *rf_dev, osal_u32 reg_addr, osal_u16 *reg_val);
osal_void fe_mpw_write_reg(const hal_rf_dev *rf_dev, osal_u32 reg_addr, osal_u16 reg_val);

#endif /* end of mpw_ssi_reg_if.h */
