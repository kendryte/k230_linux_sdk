/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_device_rom.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_REG_OPT_H__
#define __HAL_REG_OPT_H__

/*****************************************************************************
    头文件包含
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
osal_u32 hal_reg_write(osal_u32 addr, osal_u32 val);
osal_u32 hal_reg_write16(osal_u32 addr, osal_u16 val);
osal_u32 hal_reg_read(osal_u32 addr);
osal_u16 hal_reg_read16(osal_u32 addr);
osal_u32 hal_reg_write_bits_by_msg(osal_u32 reg_addr, osal_u8 offset, osal_u8 bits,
    osal_u32 reg_val, osal_u8 irq_lock);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_device.h */
