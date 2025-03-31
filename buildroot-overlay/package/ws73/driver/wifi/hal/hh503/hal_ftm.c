/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_ftm.c.
 * Create: 2023-01-11
 */

#include "hal_common_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_FTM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_WLAN_FEATURE_FTM
/*****************************************************************************
 函 数 名  : hh503_get_ftm_time
 功能描述  : hh503_get_ftm_time
*****************************************************************************/
osal_u64 hh503_get_ftm_time(osal_u64 time)
{
    return time & 0xFFFFFFFFFFFF;
}
 
/*****************************************************************************
 函 数 名  : hh503_check_ftm_t4
 功能描述  : hh503_check_ftm_t4
*****************************************************************************/
osal_u64 hh503_check_ftm_t4(osal_u64 time)
{
    if ((time >> HH503_MAC_RPT_FTM_T4_TIMER_LSB_LEN) & HH503_MAC_RPT_FTM_T4_TIMER_VLD_MASK) {
        return time & 0xFFFFFFFFFFFF;
    } else {
        return 0;
    }
}
 
/*****************************************************************************
 函 数 名  : hh503_get_ftm_t4_intp
 功能描述  : hh503_get_ftm_t4_intp
*****************************************************************************/
osal_char hh503_get_ftm_t4_intp(osal_u64 time)
{
    osal_u8 time_intp = (osal_u8)(((time >> HH503_MAC_RPT_FTM_T4_TIMER_LSB_LEN) &
        HH503_MAC_RPT_FTM_T4_TIMER_INTP_MASK) >> HH503_MAC_RPT_FTM_T4_TIMER_INTP_OFFSET);
    return (osal_char)(time_intp << (OAL_BITS_PER_BYTE - HH503_MAC_RPT_FTM_T4_TIMER_INTP_LEN)) >>
           (OAL_BITS_PER_BYTE - HH503_MAC_RPT_FTM_T4_TIMER_INTP_LEN);
}
 
/*****************************************************************************
 函 数 名  : hh503_check_ftm_t2
 功能描述  : hh503_check_ftm_t2
*****************************************************************************/
osal_u64 hh503_check_ftm_t2(osal_u64 time)
{
    if ((time >> 0x20) & HH503_MAC_RPT_FTM_T2_TIMER_VLD_MASK) {
        return time & 0xFFFFFFFFFFFF;
    } else {
        return 0;
    }
}
 
/*****************************************************************************
 函 数 名  : hh503_get_ftm_t2_intp
 功能描述  : hh503_get_ftm_t2_intp
*****************************************************************************/
osal_char hh503_get_ftm_t2_intp(osal_u64 time)
{
    osal_u8 time_intp = (osal_u8)(((time >> HH503_MAC_RPT_FTM_T2_TIMER_LSB_LEN) &
        HH503_MAC_RPT_FTM_T2_TIMER_INTP_MASK) >> HH503_MAC_RPT_FTM_T2_TIMER_INTP_OFFSET);
    return (osal_char)(time_intp << (OAL_BITS_PER_BYTE - HH503_MAC_RPT_FTM_T2_TIMER_INTP_LEN)) >>
           (OAL_BITS_PER_BYTE - HH503_MAC_RPT_FTM_T2_TIMER_INTP_LEN);
}
 
/*****************************************************************************
 功能描述  : 读FTM tod
*****************************************************************************/
osal_u64 hh503_get_ftm_tod(osal_void)
{
    osal_u32 value;
 
    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xC8);
    return ((osal_u64)hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xC4) << 0x20) | ((osal_u64)value);
}
 
/*****************************************************************************
 功能描述  : 读FTM toa
*****************************************************************************/
osal_u64 hh503_get_ftm_toa(osal_void)
{
    osal_u32 value;
 
    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xD0);
    return ((osal_u64)hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xCC) << 0x20) | ((osal_u64)value);
}
 
/*****************************************************************************
 功能描述  : 读FTM t2
*****************************************************************************/
osal_u64 hh503_get_ftm_t2(osal_void)
{
    osal_u32 value;
 
    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xD8);
    return ((osal_u64)hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xD4) << 0x20) | ((osal_u64)value);
}
 
/*****************************************************************************
 功能描述  : 读FTM t3
*****************************************************************************/
osal_u64 hh503_get_ftm_t3(osal_void)
{
    osal_u32 value;
 
    value = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xE0);
    return ((osal_u64)hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xDC) << 0x20) | ((osal_u64)value);
}
 
/*****************************************************************************
 功能描述  : 设置ftm  总开关
*****************************************************************************/
osal_void hh503_set_ftm_enable(oal_bool_enum_uint8 ftm_status)
{
    if (ftm_status == OSAL_TRUE) {
        hh503_device_enable_ftm();
    } else {
        /* 恢复FIFO寄存器控制 */
        hh503_device_disable_ftm();
        hal_reg_write(HH503_PHY_BANK3_BASE_0x1B0, 0x0);
    }
 
    /* 置位 MAC FTM enable位 */
    hal_reg_write_bits(HH503_MAC_CTRL1_BANK_BASE_0x94, HH503_MAC_CFG_FTM_EN_OFFSET, HH503_MAC_CFG_FTM_EN_LEN, ftm_status);
    /* 置位 PHY FTM enable位 */
    hal_reg_write_bits(HH503_PHY_FTM_CFG, HH503_PHY_CFG_FTM_EN_OFFSET, HH503_PHY_CFG_FTM_EN_LEN, ftm_status);
}
 
/*****************************************************************************
 功能描述  : 设置ftm  initiator
*****************************************************************************/
osal_void hh503_set_ftm_sample(oal_bool_enum_uint8 ftm_status)
{
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x74, ftm_status);
}
 
/*****************************************************************************
 功能描述  : 获取FTM使能状态
*****************************************************************************/
osal_void hh503_get_ftm_ctrl_status(osal_u32 *ftm_status)
{
    *ftm_status = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x94);
}
 
/*****************************************************************************
 功能描述  : 获取FTM配置状态
*****************************************************************************/
osal_void hh503_get_ftm_config_status(osal_u32 *ftm_status)
{
    *ftm_status = hal_reg_read(HH503_PHY_FTM_CFG);
}
 
/*****************************************************************************
 功能描述  : 设置FTM使能状态
*****************************************************************************/
osal_void hh503_set_ftm_ctrl_status(osal_u32 ftm_status)
{
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x94, ftm_status);
}
 
/*****************************************************************************
 功能描述  : 设置FTM配置寄存器
*****************************************************************************/
osal_void hh503_set_ftm_config_status(osal_u32 ftm_status)
{
    hal_reg_write(HH503_PHY_FTM_CFG, ftm_status);
}
 
/*****************************************************************************
 功能描述  : 获取ftm帧的dialog_token
*****************************************************************************/
osal_u8 hh503_get_ftm_dialog(osal_void)
{
    osal_u32 ftm_dialog = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0xE4);
    return (osal_u8)(ftm_dialog & 0xff);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
