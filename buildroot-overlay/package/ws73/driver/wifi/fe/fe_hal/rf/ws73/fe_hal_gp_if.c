/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: 通用寄存器的接口
 */
#include "fe_hal_gp_if.h"
#ifdef _PRE_PRODUCT_ID_HOST
#include "hal_gp_reg.h"
#include "fe_extern_if_host.h"
#else
#include "hal_gp_reg_rom.h"
#include "hal_ext_if_rom.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define GLB_COMMON_REG8_ADDR   (GLB_CTRL_RB_BASE_ADDR + 0x400) /* ppa二级表锁 */

/******************************************************************************
 功能描述  : 读取ppa二级表锁状态
******************************************************************************/
osal_u32 fe_hal_gp_get_ppa_lut_lock_state(osal_void)
{
    return hal_reg_read(GLB_COMMON_REG8_ADDR);
}

/******************************************************************************
 功能描述  : 设置ppa二级表锁状态
******************************************************************************/
osal_void fe_hal_gp_set_ppa_lut_lock_state(osal_u32 val)
{
    hal_reg_write(GLB_COMMON_REG8_ADDR, val);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif