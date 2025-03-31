/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Implementation for device / vap related operation.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "wlan_types_common.h"
#include "hal_common_ops.h"
#include "dmac_common_inc_rom.h"
#include "hcc_if.h"

#include "wlan_msg.h"
#include "hal_ext_if_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
osal_u8 plat_get_wifi_dfr_flag(osal_void);
#endif
#ifndef _PRE_BSLE_GATEWAY
static osal_u8 hal_check_excp_print(ext_errno err)
{
    /* DFR流程寄存器读写打印会刷屏, 屏蔽 */
#ifdef _PRE_WLAN_FEATURE_DFR
    return (err != EXT_ERR_SUCCESS) && (plat_get_wifi_dfr_flag() == OSAL_FALSE);
#else
    return (err != EXT_ERR_SUCCESS);
#endif
}
#endif
/*****************************************************************************
 功能描述  : 写SoC、MAC, PHY寄存器的值
*****************************************************************************/
osal_u32 hal_reg_write(osal_u32 addr, osal_u32 val)
{
    frw_msg msg_info = {0};
    reg_info reg = {0};

#ifndef _PRE_BSLE_GATEWAY
    if ((addr & 0xFFFF0000) == 0x40010000) {
        ext_errno err = hcc_write_reg(HCC_CHANNEL_AP, addr, val);
        if (hal_check_excp_print(err)) {
            wifi_printf(" %s() hcc_write_reg: %d \n", __func__, err);
            return 0xFFFFFFFF;
        } else {
            return OAL_SUCC;
        }
    }
#endif
    reg.addr = addr;
    reg.width = REG_ADDR_32BITS;
    reg.val = val;
    frw_msg_init((osal_u8 *)&reg, sizeof(reg), OSAL_NULL, 0, &msg_info);
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_REG_WRITE, &msg_info, OSAL_TRUE);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 写SoC、MAC, PHY寄存器的值
*****************************************************************************/
osal_u32 hal_reg_write16(osal_u32 addr, osal_u16 val)
{
    frw_msg msg_info = {0};
    reg_info reg = {0};

#ifndef _PRE_BSLE_GATEWAY
    if ((addr & 0xFFFF0000) == 0x40010000) {
        osal_u32 read_val = 0;
        ext_errno err = hcc_read_reg(HCC_CHANNEL_AP, addr & 0xFFFFFFFC, &read_val);
        if (hal_check_excp_print(err)) {
            wifi_printf(" %s() hal_reg_write16 read: %d \n", __func__, err);
            return 0xFFFFFFFF;
        }
        read_val = ((addr & 0x02) == 0) ? ((read_val & 0xFFFF0000) | val) : ((read_val & 0xFFFF) |
            ((osal_u32)val << 16)); /* 左移16位 */
        err = hcc_write_reg(HCC_CHANNEL_AP, addr & 0xFFFFFFFC, read_val);
        if (hal_check_excp_print(err)) {
            wifi_printf(" %s() hal_reg_write16: %d \n", __func__, err);
        } else {
            return OAL_SUCC;
        }
    }
#endif
    reg.addr = addr;
    reg.width = REG_ADDR_16BITS;
    reg.val = val;
    frw_msg_init((osal_u8 *)&reg, sizeof(reg), OSAL_NULL, 0, &msg_info);
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_REG_WRITE, &msg_info, OSAL_TRUE);
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 读取SoC, MAC, PHY寄存器的值
*****************************************************************************/
osal_u32 hal_reg_read(osal_u32 addr)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
    reg_info reg = {0};
    osal_u32 read_val = 0;

#ifndef _PRE_BSLE_GATEWAY
    if ((addr & 0xFFFF0000) == 0x40010000) {
        ext_errno err = hcc_read_reg(HCC_CHANNEL_AP, addr, &read_val);
        if (hal_check_excp_print(err)) {
            wifi_printf(" %s() hcc_read_reg: %d \n", __func__, err);
            return 0xFFFFFFFF;
        }
        return read_val;
    }
#endif
    reg.addr = addr;
    reg.width = REG_ADDR_32BITS;
    frw_msg_init((osal_u8 *)&reg, sizeof(reg), (osal_u8 *)&read_val, sizeof(osal_u32), &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_REG_READ, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        return 0xFFFFFFFF;
    }
    return read_val;
}

osal_u16 hal_reg_read16(osal_u32 addr)
{
    osal_s32 ret;
    frw_msg msg_info = {0};
    reg_info reg = {0};
    osal_u16 read_val = 0;

#ifndef _PRE_BSLE_GATEWAY
    if ((addr & 0xFFFF0000) == 0x40010000) {
        osal_u32 val = 0;
        ext_errno err = hcc_read_reg(HCC_CHANNEL_AP, addr & 0xFFFFFFFC, &val);
        if (hal_check_excp_print(err)) {
            wifi_printf(" %s() hcc_read_reg: %d \n", __func__, err);
            return 0xFFFF;
        }
        return ((addr & 0x02) != 0) ? (val >> 16) : (val & 0x0000FFFF); /* val右移16位 */
    }
#endif
    reg.addr = addr;
    reg.width = REG_ADDR_16BITS;
    frw_msg_init((osal_u8 *)&reg, sizeof(reg), (osal_u8 *)&read_val, sizeof(osal_u16), &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_REG_READ, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        return 0xFFFF;
    }
    return read_val;
}

/* 走消息给device写寄存器特定bits irq_lock标识device读写寄存器是否需要锁中断 */
osal_u32 hal_reg_write_bits_by_msg(osal_u32 reg_addr, osal_u8 offset, osal_u8 bits, osal_u32 reg_val, osal_u8 irq_lock)
{
    frw_msg msg_info = {0};
    reg_bit_info reg = {0};
    osal_u32 ret = OAL_SUCC;

    reg.addr = reg_addr;
    reg.offset = offset;
    reg.bits = bits;
    reg.val = reg_val;
    reg.irq_lock = irq_lock;

    frw_msg_init((osal_u8 *)&reg, sizeof(reg), (osal_u8 *)&ret, sizeof(ret), &msg_info);
    ret = frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_REG_WRITE_BITS, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        wifi_printf(" %s() msg_write reg: 0x%02x fail ret:%d\n", __func__, reg_addr, ret);
    }
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
