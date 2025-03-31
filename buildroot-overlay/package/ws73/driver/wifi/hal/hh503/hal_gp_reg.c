/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: general purpose register
 * Create: 2021-12-15
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "hal_gp_reg.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_GP_REG_C

/*****************************************************************************
 功能描述  : gp reg写接口
*****************************************************************************/
static osal_void hh503_gp_reg_write(osal_u32 addr, osal_u32 val)
{
    if (addr < GLB_COMMON_REG_ADDR_MIN || addr > GLB_COMMON_REG_ADDR_MAX) {
        return;
    }

    hal_reg_write(addr, val);
}

/*****************************************************************************
 功能描述  : gp reg读接口
*****************************************************************************/
static osal_u32 hh503_gp_reg_read(osal_u32 addr)
{
    if (addr < GLB_COMMON_REG_ADDR_MIN || addr > GLB_COMMON_REG_ADDR_MAX) {
        return 0;
    }

    return hal_reg_read(addr);
}

osal_u32 hal_gp_get_mpdu_count(osal_void)
{
    hal_gp_reg_0_u gp;

    gp.u32 = hh503_gp_reg_read(GLB_CTRL_RB_BASE_ADDR_0x380);
    return (gp.bits.free_large_cnt > gp.bits.netbuf_reserve) ? (gp.bits.free_large_cnt - gp.bits.netbuf_reserve) : 0;
}

/*****************************************************************************
 功能描述  : 获取wifi aware 工作信道切离标记, 返回1表示切离,需要暂停host发包
*****************************************************************************/
osal_u32 hal_gp_get_sdp_chnl_switch_off(osal_void)
{
    hal_gp_reg_0_u gp;

    gp.u32 = hh503_gp_reg_read(GLB_CTRL_RB_BASE_ADDR_0x380);
    return gp.bits.sdp_chnl_switch_off;
}

/*****************************************************************************
 功能描述  : 获取slp_tx_ctrl
*****************************************************************************/
osal_u32 hal_gp_get_slp_tx_ctrl(osal_void)
{
    hal_gp_reg_0_u gp;

    gp.u32 = hh503_gp_reg_read(GLB_CTRL_RB_BASE_ADDR_0x380);
    return gp.bits.slp_tx_ctrl;
}

/*****************************************************************************
 功能描述  : 获取DBAC VAP停止发送标记
*****************************************************************************/
osal_u32 hal_gp_get_dbac_vap_stop_bitmap(osal_void)
{
    return hh503_gp_reg_read(GLB_CTRL_RB_BASE_ADDR_0x384);
}

/*****************************************************************************
 功能描述  : 清空DBAC VAP停止发送标记
*****************************************************************************/
osal_void hal_gp_clear_dbac_vap_stop_bitmap(osal_void)
{
    osal_u32 val = 0;
    hh503_gp_reg_write(GLB_CTRL_RB_BASE_ADDR_0x384, val);
}

/******************************************************************************
 功能描述  : 设置通用寄存器，此寄存器用于存放蓝牙共存特性WIFI状态
******************************************************************************/
osal_void hal_gp_set_btcoex_wifi_status(osal_u32 val)
{
    hh503_gp_reg_write(GLB_CTRL_RB_BASE_ADDR_0x388, val);
}

/******************************************************************************
 功能描述  : 读取通用寄存器，此寄存器用于存放蓝牙共存特性WIFI状态
******************************************************************************/
osal_u32 hal_gp_get_btcoex_wifi_status(osal_void)
{
    return hal_reg_read(GLB_CTRL_RB_BASE_ADDR_0x388);
}

/******************************************************************************
 功能描述  : 读取通用寄存器，此寄存器用于存放蓝牙共存特性BT状态
******************************************************************************/
osal_u32 hal_gp_get_btcoex_bt_status(osal_void)
{
    return hal_reg_read(GLB_CTRL_RB_BASE_ADDR_0x38C);
}

/******************************************************************************
 功能描述  : 读取通用寄存器，此寄存器用于存放蓝牙共存wifi被打断时间
******************************************************************************/
osal_u32 hal_gp_get_btcoex_abort_time(osal_void)
{
    return hal_reg_read(GLB_CTRL_RB_BASE_ADDR_0x404);
}