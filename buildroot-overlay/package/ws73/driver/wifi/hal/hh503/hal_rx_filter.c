/*
 * Copyright (c) CompanyNameMagicTag 2021. All rights reserved.
 * Description: hal rx fliter func.
 * Create: 2021-12-29
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "hal_mac.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_RX_FILTER_C

osal_void hal_mac_interrupt_clear(osal_void)
{
    hal_reg_write((uintptr_t)HH503_MAC_CTRL0_BANK_BASE_0x54, 0);
}

/*****************************************************************************
 功能描述  : 设置rx过滤常收状态
*****************************************************************************/
osal_void hal_rx_filter_set_alrx_state(hal_device_always_rx_state_enum al_rx_flag)
{
    u_rx_framefilt1 rx_filter;

    rx_filter.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x48);
    if (al_rx_flag == HAL_ALWAYS_RX_RESERVED) {
        rx_filter.bits.xCt2OIvolRvwOIv4mOmo_ = 1;
    }
    if (al_rx_flag == HAL_ALWAYS_RX_DISABLE) {
        rx_filter.bits.xCt2OIvolRvwOIv4mOmo_ = 0;
    }
    hal_reg_write((uintptr_t)HH503_MAC_CTRL0_BANK_BASE_0x48, rx_filter.u32);
}

/*****************************************************************************
 功能描述  : 设置rx帧过滤配置
            注: 入参value：
            bit0=1 :上报组播(广播)数据帧使能标志
            bit1=1 :上报单播数据包使能标志
            bit2=1 :上报组播(广播)管理帧使能标志
            bit3=1 :上报单播管理帧使能标志
*****************************************************************************/
osal_void hal_rx_filter_set_rx_flt_en(osal_u32 value)
{
    u_rx_framefilt1 value_temp;

    value_temp.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x48); /* 读取当前的过滤控制寄存器0x40080080的值 */

    value_temp.bits.x6NgwFB_W6LwSgSLwiLQW_wjCCwNoLwWh_ = 1;   /* BIT29 */
    value_temp.bits.x6NgwFB_W6LwFGLGwiLQW_wjCCwNoLwWh_ = 1;   /* BIT28 */
    value_temp.bits.x6NgwhihwFB_W6LwSgSLwNoLwWh_ = 1; /* BIT16 */
    value_temp.bits.x6NgwhihwFB_W6LwFGLGwNoLwWh_ = 1; /* BIT14 */
    value_temp.bits.x6NgwhihwFB_W6LwSgSLwiLQW_wjCCwNoLwWh_ = 1;   /* BIT13 */
    value_temp.bits.x6NgwiLQW_wjCCwFCLwGFF_wj6wNoLwWh_ = 1;   /* BIT12 */
    value_temp.bits.x6NgwhihwFB_W6LwFGLGwiLQW_wjCCwNoLwWh_ = 1;   /* BIT11 */
    value_temp.bits.x6Ngwj6S6wSgSLwiLQW_wjCCwNoLwWh_ = 1; /* BIT4 */
    value_temp.bits.x6Ngwj6S6wFGLGwiLQW_wjCCwNoLwWh_ = 1; /* BIT3 */

    if (value & BIT0) { /* bit 0 :上报组播(广播)数据帧使能标志 */
        value_temp.bits.x6Ngwj6S6wFGLGwiLQW_wjCCwNoLwWh_ = 0; /* BIT3 */
        value_temp.bits.x6NgwiLQW_wjCCwFCLwGFF_wj6wNoLwWh_ = 0; /* BIT12 */
        value_temp.bits.x6NgwhihwFB_W6LwFGLGwNoLwWh_ = 0; /* BIT14 */
    }
    if (value & BIT1) { /* bit 1 :上报单播数据包使能标志 */
        value_temp.bits.x6NgwhihwFB_W6LwFGLGwiLQW_wjCCwNoLwWh_ = 0; /* BIT11 */
        value_temp.bits.x6NgwhihwFB_W6LwFGLGwNoLwWh_ = 0; /* BIT14 */
        value_temp.bits.x6NgwFB_W6LwFGLGwiLQW_wjCCwNoLwWh_ = 0; /* BIT28 */
    }
    if (value & BIT2) { /* bit 2 :上报组播(广播)管理帧使能标志 */
        value_temp.bits.x6Ngwj6S6wSgSLwiLQW_wjCCwNoLwWh_ = 0; /* BIT4 */
        value_temp.bits.x6NgwhihwFB_W6LwSgSLwNoLwWh_ = 0; /* BIT16 */
    }
    if (value & BIT3) { /* bit 3 :上报单播管理帧使能标志 */
        value_temp.bits.x6NgwhihwFB_W6LwSgSLwiLQW_wjCCwNoLwWh_ = 0; /* BIT13 */
        value_temp.bits.x6NgwhihwFB_W6LwSgSLwNoLwWh_ = 0; /* BIT16 */
        value_temp.bits.x6NgwFB_W6LwSgSLwiLQW_wjCCwNoLwWh_ = 0; /* BIT29 */
    }

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x48, value_temp.u32);
}
