/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy寄存器操作接口
 * Date: 2022-10-19
 */
#include "hh503_phy_reg.h"
#if defined(_PRE_WLAN_ATE) || defined(_PRE_WIFI_EDA)
#include "hal_ext_if_rom.h"
#else
#include "hal_ext_if.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#if !(defined(_PRE_WLAN_ATE) || defined(_PRE_WIFI_EDA))
#ifdef _PRE_WLAN_ONLINE_CALI
/*
 *  Function    : dyn_tx_power_cali_dyn_tx_power_cali
 *  Description : Set the value of the member TX_DYN_TX_POWER_CALI).dyn_tx_power_cali)
 *  Input       : osal_u32 dyn_tx_power_cali: 1 bits
 *  Return      : void
 */
osal_void hal_set_dyn_tx_power_cali_dyn_tx_power_cali(osal_u32 xFthwLPwuiTW_w6GoB_)
{
    osal_u32 tx_dyn_tx_power_cali = hal_reg_read(HH503_PHY_BANK3_BASE_0x11C);
    u_dyn_tx_power_cali val;
    val.u32 = tx_dyn_tx_power_cali;
    val.bits.xFthwLPwuiTW_w6GoB_ = xFthwLPwuiTW_w6GoB_;
    hal_reg_write(HH503_PHY_BANK3_BASE_0x11C, val.u32);
}
/*
 *  Function    : tx_power_accum_delay_power_accum_delay
 *  Description : Set the value of the member TX_POWER_ACCUM_DELAY).power_accum_delay)
 *  Input       : osal_u32 power_accum_delay: 20 bits
 *  Return      : void
 */
osal_void hal_set_tx_power_accum_delay_power_accum_delay(osal_u32 xGvqmwOzCCSIO4mxzQ_)
{
    osal_u32 tx_power_accum_delay = hal_reg_read(HH503_PHY_BANK3_BASE_0x1A4);
    u_tx_power_accum_delay val;
    val.u32 = tx_power_accum_delay;
    val.bits.xGvqmwOzCCSIO4mxzQ_ = xGvqmwOzCCSIO4mxzQ_;
    hal_reg_write(HH503_PHY_BANK3_BASE_0x1A4, val.u32);
}
/*
 *  Function    : cali_new_add_reg_cfg_fb_pwr_calc_prd
 *  Description : Set the value of the member CALI_NEW_ADD_REG).cfg_fb_pwr_calc_prd)
 *  Input       : osal_u32 cfg_fb_pwr_calc_prd: 4 bits
 *  Return      : void
 */
osal_void hal_set_cali_new_add_reg_cfg_fb_pwr_calc_prd(osal_u32 xCt2OtyOGqwOCzxCOGw4_)
{
    osal_u32 cali_new_add_reg = hal_reg_read(HH503_PHY_BANK3_BASE_0x350);
    u_cali_new_add_reg val;
    val.u32 = cali_new_add_reg;
    val.bits.xCt2OtyOGqwOCzxCOGw4_ = xCt2OtyOGqwOCzxCOGw4_;
    hal_reg_write(HH503_PHY_BANK3_BASE_0x350, val.u32);
}
#endif
/*
 *  Function    : tpc_ram_access_reg1_cfg_tpc_ram_period
 *  Description : Set the value of the member TPC_RAM_ACCESS_REG1).cfg_tpc_ram_period)
 *  Input       : osal_u32 cfg_tpc_ram_period: 1 bits
 *  Return      : void
 */
osal_void hal_set_tpc_ram_access_reg1_cfg_tpc_ram_period(osal_u32 xCt2ORGCOwzIOGmwlv4_)
{
    osal_u32 tpc_ram_access_reg1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1A4);
    u_tpc_ram_access_reg1 val;
    val.u32 = tpc_ram_access_reg1;
    val.bits.xCt2ORGCOwzIOGmwlv4_ = xCt2ORGCOwzIOGmwlv4_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1A4, val.u32);
}
/*
 *  Function    : tpc_ram_access_reg1_cfg_indirect_rw_mode
 *  Description : Set the value of the member TPC_RAM_ACCESS_REG1).cfg_indirect_rw_mode)
 *  Input       : osal_u32 cfg_indirect_rw_mode: 1 bits
 *  Return      : void
 */
osal_void hal_set_tpc_ram_access_reg1_cfg_indirect_rw_mode(osal_u32 x6NgwBhFB_W6Lw_TwSiFW_)
{
    osal_u32 tpc_ram_access_reg1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1A4);
    u_tpc_ram_access_reg1 val;
    val.u32 = tpc_ram_access_reg1;
    val.bits.x6NgwBhFB_W6Lw_TwSiFW_ = x6NgwBhFB_W6Lw_TwSiFW_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1A4, val.u32);
}
/*
 *  Function    : tpc_ram_access_reg1_cfg_tpc_table_access_addr
 *  Description : Set the value of the member TPC_RAM_ACCESS_REG1).cfg_tpc_table_access_addr)
 *  Input       : osal_u32 cfg_tpc_table_access_addr: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_ram_access_reg1_cfg_tpc_table_access_addr(osal_u32 xCt2ORGCORzyxmOzCCmssOz44w_)
{
    osal_u32 tpc_ram_access_reg1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1A4);
    u_tpc_ram_access_reg1 val;
    val.u32 = tpc_ram_access_reg1;
    val.bits.xCt2ORGCORzyxmOzCCmssOz44w_ = xCt2ORGCORzyxmOzCCmssOz44w_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1A4, val.u32);
}
#endif
/*
 *  Function    : tpc_ram_access_reg2_cfg_tpc_table_wr_data
 *  Description : Set the value of the member TPC_RAM_ACCESS_REG2).cfg_tpc_table_wr_data)
 *  Input       : osal_u32 cfg_tpc_table_wr_data: 26 bits
 *  Return      : void
 */
osal_void hal_set_tpc_ram_access_reg2_cfg_tpc_table_wr_data(osal_u32 x6NgwLu6wLGjoWwT_wFGLG_)
{
    osal_u32 tpc_ram_access_reg2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1A8);
    u_tpc_ram_access_reg2 val;
    val.u32 = tpc_ram_access_reg2;
    val.bits.x6NgwLu6wLGjoWwT_wFGLG_ = x6NgwLu6wLGjoWwT_wFGLG_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1A8, val.u32);
}
#if !(defined(_PRE_WLAN_ATE) || defined(_PRE_WIFI_EDA))
/*
 *  Function    : rf_ppa_code_lut0_cfg_rf_ppa_code_lut0
 *  Description : Set the value of the member RF_PPA_CODE_LUT0).cfg_rf_ppa_code_lut0)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut0: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut0_cfg_rf_ppa_code_lut0(osal_u32 xCt2OwtOGGzOCv4mOxSR3_)
{
    osal_u32 rf_ppa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x254);
    u_rf_ppa_code_lut0 val;
    val.u32 = rf_ppa_code_lut0;
    val.bits.xCt2OwtOGGzOCv4mOxSR3_ = xCt2OwtOGGzOCv4mOxSR3_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x254, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut0_cfg_rf_ppa_code_lut0
 *  Description : Get the value of the member RF_PPA_CODE_LUT0).cfg_rf_ppa_code_lut0)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut0: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut0_cfg_rf_ppa_code_lut0(osal_void)
{
    osal_u32 rf_ppa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x254);
    u_rf_ppa_code_lut0 val;
    val.u32 = rf_ppa_code_lut0;
    return val.bits.xCt2OwtOGGzOCv4mOxSR3_;
}
/*
 *  Function    : rf_ppa_code_lut0_cfg_rf_ppa_code_lut1
 *  Description : Set the value of the member RF_PPA_CODE_LUT0).cfg_rf_ppa_code_lut1)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut1: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut0_cfg_rf_ppa_code_lut1(osal_u32 x6Ngw_NwuuGw6iFWwodL0_)
{
    osal_u32 rf_ppa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x254);
    u_rf_ppa_code_lut0 val;
    val.u32 = rf_ppa_code_lut0;
    val.bits.x6Ngw_NwuuGw6iFWwodL0_ = x6Ngw_NwuuGw6iFWwodL0_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x254, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut0_cfg_rf_ppa_code_lut1
 *  Description : Get the value of the member RF_PPA_CODE_LUT0).cfg_rf_ppa_code_lut1)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut1: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut0_cfg_rf_ppa_code_lut1(osal_void)
{
    osal_u32 rf_ppa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x254);
    u_rf_ppa_code_lut0 val;
    val.u32 = rf_ppa_code_lut0;
    return val.bits.x6Ngw_NwuuGw6iFWwodL0_;
}
/*
 *  Function    : rf_ppa_code_lut1_cfg_rf_ppa_code_lut2
 *  Description : Set the value of the member RF_PPA_CODE_LUT1).cfg_rf_ppa_code_lut2)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut2: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut1_cfg_rf_ppa_code_lut2(osal_u32 xCt2OwtOGGzOCv4mOxSRZ_)
{
    osal_u32 rf_ppa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x258);
    u_rf_ppa_code_lut1 val;
    val.u32 = rf_ppa_code_lut1;
    val.bits.xCt2OwtOGGzOCv4mOxSRZ_ = xCt2OwtOGGzOCv4mOxSRZ_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x258, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut1_cfg_rf_ppa_code_lut2
 *  Description : Get the value of the member RF_PPA_CODE_LUT1).cfg_rf_ppa_code_lut2)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut2: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut1_cfg_rf_ppa_code_lut2(osal_void)
{
    osal_u32 rf_ppa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x258);
    u_rf_ppa_code_lut1 val;
    val.u32 = rf_ppa_code_lut1;
    return val.bits.xCt2OwtOGGzOCv4mOxSRZ_;
}
/*
 *  Function    : rf_ppa_code_lut1_cfg_rf_ppa_code_lut3
 *  Description : Set the value of the member RF_PPA_CODE_LUT1).cfg_rf_ppa_code_lut3)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut3: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut1_cfg_rf_ppa_code_lut3(osal_u32 xCt2OwtOGGzOCv4mOxSRi_)
{
    osal_u32 rf_ppa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x258);
    u_rf_ppa_code_lut1 val;
    val.u32 = rf_ppa_code_lut1;
    val.bits.xCt2OwtOGGzOCv4mOxSRi_ = xCt2OwtOGGzOCv4mOxSRi_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x258, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut1_cfg_rf_ppa_code_lut3
 *  Description : Get the value of the member RF_PPA_CODE_LUT1).cfg_rf_ppa_code_lut3)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut3: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut1_cfg_rf_ppa_code_lut3(osal_void)
{
    osal_u32 rf_ppa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x258);
    u_rf_ppa_code_lut1 val;
    val.u32 = rf_ppa_code_lut1;
    return val.bits.xCt2OwtOGGzOCv4mOxSRi_;
}
/*
 *  Function    : rf_ppa_code_lut2_cfg_rf_ppa_code_lut4
 *  Description : Set the value of the member RF_PPA_CODE_LUT2).cfg_rf_ppa_code_lut4)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut4: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut2_cfg_rf_ppa_code_lut4(osal_u32 x6Ngw_NwuuGw6iFWwodLD_)
{
    osal_u32 rf_ppa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x25C);
    u_rf_ppa_code_lut2 val;
    val.u32 = rf_ppa_code_lut2;
    val.bits.x6Ngw_NwuuGw6iFWwodLD_ = x6Ngw_NwuuGw6iFWwodLD_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x25C, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut2_cfg_rf_ppa_code_lut4
 *  Description : Get the value of the member RF_PPA_CODE_LUT2).cfg_rf_ppa_code_lut4)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut4: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut2_cfg_rf_ppa_code_lut4(osal_void)
{
    osal_u32 rf_ppa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x25C);
    u_rf_ppa_code_lut2 val;
    val.u32 = rf_ppa_code_lut2;
    return val.bits.x6Ngw_NwuuGw6iFWwodLD_;
}
/*
 *  Function    : rf_ppa_code_lut2_cfg_rf_ppa_code_lut5
 *  Description : Set the value of the member RF_PPA_CODE_LUT2).cfg_rf_ppa_code_lut5)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut5: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut2_cfg_rf_ppa_code_lut5(osal_u32 xCt2OwtOGGzOCv4mOxSRa_)
{
    osal_u32 rf_ppa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x25C);
    u_rf_ppa_code_lut2 val;
    val.u32 = rf_ppa_code_lut2;
    val.bits.xCt2OwtOGGzOCv4mOxSRa_ = xCt2OwtOGGzOCv4mOxSRa_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x25C, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut2_cfg_rf_ppa_code_lut5
 *  Description : Get the value of the member RF_PPA_CODE_LUT2).cfg_rf_ppa_code_lut5)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut5: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut2_cfg_rf_ppa_code_lut5(osal_void)
{
    osal_u32 rf_ppa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x25C);
    u_rf_ppa_code_lut2 val;
    val.u32 = rf_ppa_code_lut2;
    return val.bits.xCt2OwtOGGzOCv4mOxSRa_;
}
/*
 *  Function    : rf_ppa_code_lut3_cfg_rf_ppa_code_lut6
 *  Description : Set the value of the member RF_PPA_CODE_LUT3).cfg_rf_ppa_code_lut6)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut6: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut3_cfg_rf_ppa_code_lut6(osal_u32 x6Ngw_NwuuGw6iFWwodLX_)
{
    osal_u32 rf_ppa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x260);
    u_rf_ppa_code_lut3 val;
    val.u32 = rf_ppa_code_lut3;
    val.bits.x6Ngw_NwuuGw6iFWwodLX_ = x6Ngw_NwuuGw6iFWwodLX_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x260, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut3_cfg_rf_ppa_code_lut6
 *  Description : Get the value of the member RF_PPA_CODE_LUT3).cfg_rf_ppa_code_lut6)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut6: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut3_cfg_rf_ppa_code_lut6(osal_void)
{
    osal_u32 rf_ppa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x260);
    u_rf_ppa_code_lut3 val;
    val.u32 = rf_ppa_code_lut3;
    return val.bits.x6Ngw_NwuuGw6iFWwodLX_;
}
/*
 *  Function    : rf_ppa_code_lut3_cfg_rf_ppa_code_lut7
 *  Description : Set the value of the member RF_PPA_CODE_LUT3).cfg_rf_ppa_code_lut7)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut7: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut3_cfg_rf_ppa_code_lut7(osal_u32 xCt2OwtOGGzOCv4mOxSR9_)
{
    osal_u32 rf_ppa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x260);
    u_rf_ppa_code_lut3 val;
    val.u32 = rf_ppa_code_lut3;
    val.bits.xCt2OwtOGGzOCv4mOxSR9_ = xCt2OwtOGGzOCv4mOxSR9_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x260, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut3_cfg_rf_ppa_code_lut7
 *  Description : Get the value of the member RF_PPA_CODE_LUT3).cfg_rf_ppa_code_lut7)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut7: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut3_cfg_rf_ppa_code_lut7(osal_void)
{
    osal_u32 rf_ppa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x260);
    u_rf_ppa_code_lut3 val;
    val.u32 = rf_ppa_code_lut3;
    return val.bits.xCt2OwtOGGzOCv4mOxSR9_;
}
/*
 *  Function    : rf_ppa_code_lut4_cfg_rf_ppa_code_lut8
 *  Description : Set the value of the member RF_PPA_CODE_LUT4).cfg_rf_ppa_code_lut8)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut8: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut4_cfg_rf_ppa_code_lut8(osal_u32 x6Ngw_NwuuGw6iFWwodLU_)
{
    osal_u32 rf_ppa_code_lut4 = hal_reg_read(HH503_PHY_BANK2_BASE_0x264);
    u_rf_ppa_code_lut4 val;
    val.u32 = rf_ppa_code_lut4;
    val.bits.x6Ngw_NwuuGw6iFWwodLU_ = x6Ngw_NwuuGw6iFWwodLU_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x264, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut4_cfg_rf_ppa_code_lut8
 *  Description : Get the value of the member RF_PPA_CODE_LUT4).cfg_rf_ppa_code_lut8)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut8: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut4_cfg_rf_ppa_code_lut8(osal_void)
{
    osal_u32 rf_ppa_code_lut4 = hal_reg_read(HH503_PHY_BANK2_BASE_0x264);
    u_rf_ppa_code_lut4 val;
    val.u32 = rf_ppa_code_lut4;
    return val.bits.x6Ngw_NwuuGw6iFWwodLU_;
}
/*
 *  Function    : rf_ppa_code_lut4_cfg_rf_ppa_code_lut9
 *  Description : Set the value of the member RF_PPA_CODE_LUT4).cfg_rf_ppa_code_lut9)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut9: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut4_cfg_rf_ppa_code_lut9(osal_u32 xCt2OwtOGGzOCv4mOxSRc_)
{
    osal_u32 rf_ppa_code_lut4 = hal_reg_read(HH503_PHY_BANK2_BASE_0x264);
    u_rf_ppa_code_lut4 val;
    val.u32 = rf_ppa_code_lut4;
    val.bits.xCt2OwtOGGzOCv4mOxSRc_ = xCt2OwtOGGzOCv4mOxSRc_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x264, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut4_cfg_rf_ppa_code_lut9
 *  Description : Get the value of the member RF_PPA_CODE_LUT4).cfg_rf_ppa_code_lut9)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut9: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut4_cfg_rf_ppa_code_lut9(osal_void)
{
    osal_u32 rf_ppa_code_lut4 = hal_reg_read(HH503_PHY_BANK2_BASE_0x264);
    u_rf_ppa_code_lut4 val;
    val.u32 = rf_ppa_code_lut4;
    return val.bits.xCt2OwtOGGzOCv4mOxSRc_;
}
/*
 *  Function    : rf_ppa_code_lut5_cfg_rf_ppa_code_lut10
 *  Description : Set the value of the member RF_PPA_CODE_LUT5).cfg_rf_ppa_code_lut10)
 *  Input       : osal_u32 cfg_rf_ppa_code_lut10: 16 bits
 *  Return      : void
 */
osal_void hal_set_rf_ppa_code_lut5_cfg_rf_ppa_code_lut10(osal_u32 xCt2OwtOGGzOCv4mOxSRr3_)
{
    osal_u32 rf_ppa_code_lut5 = hal_reg_read(HH503_PHY_BANK2_BASE_0x268);
    u_rf_ppa_code_lut5 val;
    val.u32 = rf_ppa_code_lut5;
    val.bits.xCt2OwtOGGzOCv4mOxSRr3_ = xCt2OwtOGGzOCv4mOxSRr3_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x268, val.u32);
}
/*
 *  Function    : rf_ppa_code_lut5_cfg_rf_ppa_code_lut10
 *  Description : Get the value of the member RF_PPA_CODE_LUT5).cfg_rf_ppa_code_lut10)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_rf_ppa_code_lut10: 16 bits
 */
osal_u32 hal_get_rf_ppa_code_lut5_cfg_rf_ppa_code_lut10(osal_void)
{
    osal_u32 rf_ppa_code_lut5 = hal_reg_read(HH503_PHY_BANK2_BASE_0x268);
    u_rf_ppa_code_lut5 val;
    val.u32 = rf_ppa_code_lut5;
    return val.bits.xCt2OwtOGGzOCv4mOxSRr3_;
}
#endif
/*
 *  Function    : rf_pa_code_lut0_cfg_rf_pa_code_lut0
 *  Description : Set the value of the member RF_PA_CODE_LUT0).cfg_rf_pa_code_lut0)
 *  Input       : osal_u32 cfg_rf_pa_code_lut0: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut0(osal_u32 xCt2OwtOGzOCv4mOxSR3_)
{
    osal_u32 rf_pa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x274);
    u_rf_pa_code_lut0 val;
    val.u32 = rf_pa_code_lut0;
    val.bits.xCt2OwtOGzOCv4mOxSR3_ = xCt2OwtOGzOCv4mOxSR3_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x274, val.u32);
}
/*
 *  Function    : rf_pa_code_lut0_cfg_rf_pa_code_lut1
 *  Description : Set the value of the member RF_PA_CODE_LUT0).cfg_rf_pa_code_lut1)
 *  Input       : osal_u32 cfg_rf_pa_code_lut1: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut1(osal_u32 x6Ngw_NwuGw6iFWwodL0_)
{
    osal_u32 rf_pa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x274);
    u_rf_pa_code_lut0 val;
    val.u32 = rf_pa_code_lut0;
    val.bits.x6Ngw_NwuGw6iFWwodL0_ = x6Ngw_NwuGw6iFWwodL0_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x274, val.u32);
}
/*
 *  Function    : rf_pa_code_lut0_cfg_rf_pa_code_lut2
 *  Description : Set the value of the member RF_PA_CODE_LUT0).cfg_rf_pa_code_lut2)
 *  Input       : osal_u32 cfg_rf_pa_code_lut2: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut2(osal_u32 xCt2OwtOGzOCv4mOxSRZ_)
{
    osal_u32 rf_pa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x274);
    u_rf_pa_code_lut0 val;
    val.u32 = rf_pa_code_lut0;
    val.bits.xCt2OwtOGzOCv4mOxSRZ_ = xCt2OwtOGzOCv4mOxSRZ_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x274, val.u32);
}
/*
 *  Function    : rf_pa_code_lut0_cfg_rf_pa_code_lut3
 *  Description : Set the value of the member RF_PA_CODE_LUT0).cfg_rf_pa_code_lut3)
 *  Input       : osal_u32 cfg_rf_pa_code_lut3: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut0_cfg_rf_pa_code_lut3(osal_u32 xCt2OwtOGzOCv4mOxSRi_)
{
    osal_u32 rf_pa_code_lut0 = hal_reg_read(HH503_PHY_BANK2_BASE_0x274);
    u_rf_pa_code_lut0 val;
    val.u32 = rf_pa_code_lut0;
    val.bits.xCt2OwtOGzOCv4mOxSRi_ = xCt2OwtOGzOCv4mOxSRi_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x274, val.u32);
}
/*
 *  Function    : rf_pa_code_lut1_cfg_rf_pa_code_lut4
 *  Description : Set the value of the member RF_PA_CODE_LUT1).cfg_rf_pa_code_lut4)
 *  Input       : osal_u32 cfg_rf_pa_code_lut4: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut4(osal_u32 x6Ngw_NwuGw6iFWwodLD_)
{
    osal_u32 rf_pa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x278);
    u_rf_pa_code_lut1 val;
    val.u32 = rf_pa_code_lut1;
    val.bits.x6Ngw_NwuGw6iFWwodLD_ = x6Ngw_NwuGw6iFWwodLD_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x278, val.u32);
}
/*
 *  Function    : rf_pa_code_lut1_cfg_rf_pa_code_lut5
 *  Description : Set the value of the member RF_PA_CODE_LUT1).cfg_rf_pa_code_lut5)
 *  Input       : osal_u32 cfg_rf_pa_code_lut5: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut5(osal_u32 xCt2OwtOGzOCv4mOxSRa_)
{
    osal_u32 rf_pa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x278);
    u_rf_pa_code_lut1 val;
    val.u32 = rf_pa_code_lut1;
    val.bits.xCt2OwtOGzOCv4mOxSRa_ = xCt2OwtOGzOCv4mOxSRa_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x278, val.u32);
}
/*
 *  Function    : rf_pa_code_lut1_cfg_rf_pa_code_lut6
 *  Description : Set the value of the member RF_PA_CODE_LUT1).cfg_rf_pa_code_lut6)
 *  Input       : osal_u32 cfg_rf_pa_code_lut6: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut6(osal_u32 x6Ngw_NwuGw6iFWwodLX_)
{
    osal_u32 rf_pa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x278);
    u_rf_pa_code_lut1 val;
    val.u32 = rf_pa_code_lut1;
    val.bits.x6Ngw_NwuGw6iFWwodLX_ = x6Ngw_NwuGw6iFWwodLX_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x278, val.u32);
}
/*
 *  Function    : rf_pa_code_lut1_cfg_rf_pa_code_lut7
 *  Description : Set the value of the member RF_PA_CODE_LUT1).cfg_rf_pa_code_lut7)
 *  Input       : osal_u32 cfg_rf_pa_code_lut7: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut1_cfg_rf_pa_code_lut7(osal_u32 xCt2OwtOGzOCv4mOxSR9_)
{
    osal_u32 rf_pa_code_lut1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x278);
    u_rf_pa_code_lut1 val;
    val.u32 = rf_pa_code_lut1;
    val.bits.xCt2OwtOGzOCv4mOxSR9_ = xCt2OwtOGzOCv4mOxSR9_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x278, val.u32);
}
/*
 *  Function    : rf_pa_code_lut2_cfg_rf_pa_code_lut8
 *  Description : Set the value of the member RF_PA_CODE_LUT2).cfg_rf_pa_code_lut8)
 *  Input       : osal_u32 cfg_rf_pa_code_lut8: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut8(osal_u32 x6Ngw_NwuGw6iFWwodLU_)
{
    osal_u32 rf_pa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x27C);
    u_rf_pa_code_lut2 val;
    val.u32 = rf_pa_code_lut2;
    val.bits.x6Ngw_NwuGw6iFWwodLU_ = x6Ngw_NwuGw6iFWwodLU_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x27C, val.u32);
}
/*
 *  Function    : rf_pa_code_lut2_cfg_rf_pa_code_lut9
 *  Description : Set the value of the member RF_PA_CODE_LUT2).cfg_rf_pa_code_lut9)
 *  Input       : osal_u32 cfg_rf_pa_code_lut9: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut9(osal_u32 xCt2OwtOGzOCv4mOxSRc_)
{
    osal_u32 rf_pa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x27C);
    u_rf_pa_code_lut2 val;
    val.u32 = rf_pa_code_lut2;
    val.bits.xCt2OwtOGzOCv4mOxSRc_ = xCt2OwtOGzOCv4mOxSRc_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x27C, val.u32);
}
/*
 *  Function    : rf_pa_code_lut2_cfg_rf_pa_code_lut10
 *  Description : Set the value of the member RF_PA_CODE_LUT2).cfg_rf_pa_code_lut10)
 *  Input       : osal_u32 cfg_rf_pa_code_lut10: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut10(osal_u32 xCt2OwtOGzOCv4mOxSRr3_)
{
    osal_u32 rf_pa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x27C);
    u_rf_pa_code_lut2 val;
    val.u32 = rf_pa_code_lut2;
    val.bits.xCt2OwtOGzOCv4mOxSRr3_ = xCt2OwtOGzOCv4mOxSRr3_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x27C, val.u32);
}
/*
 *  Function    : rf_pa_code_lut2_cfg_rf_pa_code_lut11
 *  Description : Set the value of the member RF_PA_CODE_LUT2).cfg_rf_pa_code_lut11)
 *  Input       : osal_u32 cfg_rf_pa_code_lut11: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut2_cfg_rf_pa_code_lut11(osal_u32 x6Ngw_NwuGw6iFWwodL00_)
{
    osal_u32 rf_pa_code_lut2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x27C);
    u_rf_pa_code_lut2 val;
    val.u32 = rf_pa_code_lut2;
    val.bits.x6Ngw_NwuGw6iFWwodL00_ = x6Ngw_NwuGw6iFWwodL00_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x27C, val.u32);
}
/*
 *  Function    : rf_pa_code_lut3_cfg_rf_pa_code_lut12
 *  Description : Set the value of the member RF_PA_CODE_LUT3).cfg_rf_pa_code_lut12)
 *  Input       : osal_u32 cfg_rf_pa_code_lut12: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut12(osal_u32 xCt2OwtOGzOCv4mOxSRrZ_)
{
    osal_u32 rf_pa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x280);
    u_rf_pa_code_lut3 val;
    val.u32 = rf_pa_code_lut3;
    val.bits.xCt2OwtOGzOCv4mOxSRrZ_ = xCt2OwtOGzOCv4mOxSRrZ_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x280, val.u32);
}
/*
 *  Function    : rf_pa_code_lut3_cfg_rf_pa_code_lut13
 *  Description : Set the value of the member RF_PA_CODE_LUT3).cfg_rf_pa_code_lut13)
 *  Input       : osal_u32 cfg_rf_pa_code_lut13: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut13(osal_u32 xCt2OwtOGzOCv4mOxSRri_)
{
    osal_u32 rf_pa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x280);
    u_rf_pa_code_lut3 val;
    val.u32 = rf_pa_code_lut3;
    val.bits.xCt2OwtOGzOCv4mOxSRri_ = xCt2OwtOGzOCv4mOxSRri_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x280, val.u32);
}
/*
 *  Function    : rf_pa_code_lut3_cfg_rf_pa_code_lut14
 *  Description : Set the value of the member RF_PA_CODE_LUT3).cfg_rf_pa_code_lut14)
 *  Input       : osal_u32 cfg_rf_pa_code_lut14: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut14(osal_u32 x6Ngw_NwuGw6iFWwodL0D_)
{
    osal_u32 rf_pa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x280);
    u_rf_pa_code_lut3 val;
    val.u32 = rf_pa_code_lut3;
    val.bits.x6Ngw_NwuGw6iFWwodL0D_ = x6Ngw_NwuGw6iFWwodL0D_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x280, val.u32);
}
/*
 *  Function    : rf_pa_code_lut3_cfg_rf_pa_code_lut15
 *  Description : Set the value of the member RF_PA_CODE_LUT3).cfg_rf_pa_code_lut15)
 *  Input       : osal_u32 cfg_rf_pa_code_lut15: 8 bits
 *  Return      : void
 */
osal_void hal_set_rf_pa_code_lut3_cfg_rf_pa_code_lut15(osal_u32 xCt2OwtOGzOCv4mOxSRra_)
{
    osal_u32 rf_pa_code_lut3 = hal_reg_read(HH503_PHY_BANK2_BASE_0x280);
    u_rf_pa_code_lut3 val;
    val.u32 = rf_pa_code_lut3;
    val.bits.xCt2OwtOGzOCv4mOxSRra_ = xCt2OwtOGzOCv4mOxSRra_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x280, val.u32);
}

/*
 *  Function    : tpc_boundary_threshold1_cfg_tpc_boundary_0
 *  Description : Set the value of the member TPC_BOUNDARY_THRESHOLD1).cfg_tpc_boundary_0)
 *  Input       : osal_u32 cfg_tpc_boundary_0: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_boundary_threshold1_cfg_tpc_boundary_0(osal_u32 x6NgwLu6wjidhFG_twy_)
{
    osal_u32 tpc_boundary_threshold1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B0);
    u_tpc_boundary_threshold1 val;
    val.u32 = tpc_boundary_threshold1;
    val.bits.x6NgwLu6wjidhFG_twy_ = x6NgwLu6wjidhFG_twy_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B0, val.u32);
}
/*
 *  Function    : tpc_boundary_threshold1_cfg_tpc_boundary_1
 *  Description : Set the value of the member TPC_BOUNDARY_THRESHOLD1).cfg_tpc_boundary_1)
 *  Input       : osal_u32 cfg_tpc_boundary_1: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_boundary_threshold1_cfg_tpc_boundary_1(osal_u32 xCt2ORGCOyvSo4zwQOr_)
{
    osal_u32 tpc_boundary_threshold1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B0);
    u_tpc_boundary_threshold1 val;
    val.u32 = tpc_boundary_threshold1;
    val.bits.xCt2ORGCOyvSo4zwQOr_ = xCt2ORGCOyvSo4zwQOr_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B0, val.u32);
}
/*
 *  Function    : tpc_boundary_threshold1_cfg_tpc_boundary_2
 *  Description : Set the value of the member TPC_BOUNDARY_THRESHOLD1).cfg_tpc_boundary_2)
 *  Input       : osal_u32 cfg_tpc_boundary_2: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_boundary_threshold1_cfg_tpc_boundary_2(osal_u32 x6NgwLu6wjidhFG_twV_)
{
    osal_u32 tpc_boundary_threshold1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B0);
    u_tpc_boundary_threshold1 val;
    val.u32 = tpc_boundary_threshold1;
    val.bits.x6NgwLu6wjidhFG_twV_ = x6NgwLu6wjidhFG_twV_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B0, val.u32);
}
/*
 *  Function    : tpc_boundary_threshold2_cfg_tpc_boundary_3
 *  Description : Set the value of the member TPC_BOUNDARY_THRESHOLD2).cfg_tpc_boundary_3)
 *  Input       : osal_u32 cfg_tpc_boundary_3: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_3(osal_u32 x6NgwLu6wjidhFG_twk_)
{
    osal_u32 tpc_boundary_threshold2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B4);
    u_tpc_boundary_threshold2 val;
    val.u32 = tpc_boundary_threshold2;
    val.bits.x6NgwLu6wjidhFG_twk_ = x6NgwLu6wjidhFG_twk_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B4, val.u32);
}
/*
 *  Function    : tpc_boundary_threshold2_cfg_tpc_boundary_4
 *  Description : Set the value of the member TPC_BOUNDARY_THRESHOLD2).cfg_tpc_boundary_4)
 *  Input       : osal_u32 cfg_tpc_boundary_4: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_4(osal_u32 xCt2ORGCOyvSo4zwQOK_)
{
    osal_u32 tpc_boundary_threshold2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B4);
    u_tpc_boundary_threshold2 val;
    val.u32 = tpc_boundary_threshold2;
    val.bits.xCt2ORGCOyvSo4zwQOK_ = xCt2ORGCOyvSo4zwQOK_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B4, val.u32);
}
/*
 *  Function    : tpc_boundary_threshold2_cfg_tpc_boundary_5
 *  Description : Set the value of the member TPC_BOUNDARY_THRESHOLD2).cfg_tpc_boundary_5)
 *  Input       : osal_u32 cfg_tpc_boundary_5: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_5(osal_u32 x6NgwLu6wjidhFG_twx_)
{
    osal_u32 tpc_boundary_threshold2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B4);
    u_tpc_boundary_threshold2 val;
    val.u32 = tpc_boundary_threshold2;
    val.bits.x6NgwLu6wjidhFG_twx_ = x6NgwLu6wjidhFG_twx_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B4, val.u32);
}
/*
 *  Function    : tpc_boundary_threshold2_cfg_tpc_boundary_6
 *  Description : Set the value of the member TPC_BOUNDARY_THRESHOLD2).cfg_tpc_boundary_6)
 *  Input       : osal_u32 cfg_tpc_boundary_6: 8 bits
 *  Return      : void
 */
osal_void hal_set_tpc_boundary_threshold2_cfg_tpc_boundary_6(osal_u32 xCt2ORGCOyvSo4zwQOY_)
{
    osal_u32 tpc_boundary_threshold2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B4);
    u_tpc_boundary_threshold2 val;
    val.u32 = tpc_boundary_threshold2;
    val.bits.xCt2ORGCOyvSo4zwQOY_ = xCt2ORGCOyvSo4zwQOY_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B4, val.u32);
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_0
 *  Description : Set the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_0)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_0: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_0(osal_u32 x6NgwFjjwC6GoWwCQGFiTwy_)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    val.bits.x6NgwFjjwC6GoWwCQGFiTwy_ = x6NgwFjjwC6GoWwCQGFiTwy_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B8, val.u32);
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_0
 *  Description : Get the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_0)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_0: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_0(osal_void)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    return val.bits.x6NgwFjjwC6GoWwCQGFiTwy_;
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_1
 *  Description : Set the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_1)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_1: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_1(osal_u32 xCt2O4yyOsCzxmOsAz4vqOr_)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    val.bits.xCt2O4yyOsCzxmOsAz4vqOr_ = xCt2O4yyOsCzxmOsAz4vqOr_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B8, val.u32);
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_1
 *  Description : Get the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_1)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_1: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_1(osal_void)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    return val.bits.xCt2O4yyOsCzxmOsAz4vqOr_;
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_2
 *  Description : Set the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_2)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_2: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_2(osal_u32 x6NgwFjjwC6GoWwCQGFiTwV_)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    val.bits.x6NgwFjjwC6GoWwCQGFiTwV_ = x6NgwFjjwC6GoWwCQGFiTwV_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B8, val.u32);
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_2
 *  Description : Get the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_2)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_2: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_2(osal_void)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    return val.bits.x6NgwFjjwC6GoWwCQGFiTwV_;
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_3
 *  Description : Set the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_3)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_3: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow1_cfg_dbb_scale_shadow_3(osal_u32 x6NgwFjjwC6GoWwCQGFiTwk_)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    val.bits.x6NgwFjjwC6GoWwCQGFiTwk_ = x6NgwFjjwC6GoWwCQGFiTwk_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1B8, val.u32);
}
/*
 *  Function    : dbb_scale_shadow1_cfg_dbb_scale_shadow_3
 *  Description : Get the value of the member DBB_SCALE_SHADOW1).cfg_dbb_scale_shadow_3)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_3: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow1_cfg_dbb_scale_shadow_3(osal_void)
{
    osal_u32 dbb_scale_shadow1 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1B8);
    u_dbb_scale_shadow1 val;
    val.u32 = dbb_scale_shadow1;
    return val.bits.x6NgwFjjwC6GoWwCQGFiTwk_;
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_4
 *  Description : Set the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_4)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_4: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_4(osal_u32 xCt2O4yyOsCzxmOsAz4vqOK_)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    val.bits.xCt2O4yyOsCzxmOsAz4vqOK_ = xCt2O4yyOsCzxmOsAz4vqOK_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1BC, val.u32);
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_4
 *  Description : Get the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_4)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_4: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_4(osal_void)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    return val.bits.xCt2O4yyOsCzxmOsAz4vqOK_;
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_5
 *  Description : Set the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_5)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_5: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_5(osal_u32 x6NgwFjjwC6GoWwCQGFiTwx_)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    val.bits.x6NgwFjjwC6GoWwCQGFiTwx_ = x6NgwFjjwC6GoWwCQGFiTwx_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1BC, val.u32);
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_5
 *  Description : Get the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_5)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_5: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_5(osal_void)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    return val.bits.x6NgwFjjwC6GoWwCQGFiTwx_;
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_6
 *  Description : Set the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_6)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_6: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_6(osal_u32 xCt2O4yyOsCzxmOsAz4vqOY_)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    val.bits.xCt2O4yyOsCzxmOsAz4vqOY_ = xCt2O4yyOsCzxmOsAz4vqOY_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1BC, val.u32);
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_6
 *  Description : Get the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_6)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_6: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_6(osal_void)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    return val.bits.xCt2O4yyOsCzxmOsAz4vqOY_;
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_7
 *  Description : Set the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_7)
 *  Input       : osal_u32 cfg_dbb_scale_shadow_7: 8 bits
 *  Return      : void
 */
osal_void hal_set_dbb_scale_shadow2_cfg_dbb_scale_shadow_7(osal_u32 x6NgwFjjwC6GoWwCQGFiTwp_)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    val.bits.x6NgwFjjwC6GoWwCQGFiTwp_ = x6NgwFjjwC6GoWwCQGFiTwp_;
    hal_reg_write(HH503_PHY_BANK2_BASE_0x1BC, val.u32);
}
/*
 *  Function    : dbb_scale_shadow2_cfg_dbb_scale_shadow_7
 *  Description : Get the value of the member DBB_SCALE_SHADOW2).cfg_dbb_scale_shadow_7)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_dbb_scale_shadow_7: 8 bits
 */
osal_u32 hal_get_dbb_scale_shadow2_cfg_dbb_scale_shadow_7(osal_void)
{
    osal_u32 dbb_scale_shadow2 = hal_reg_read(HH503_PHY_BANK2_BASE_0x1BC);
    u_dbb_scale_shadow2 val;
    val.u32 = dbb_scale_shadow2;
    return val.bits.x6NgwFjjwC6GoWwCQGFiTwp_;
}
/*
 *  Function    : cfg_reserv_1_reg_1_reserv_1_wr_addr
 *  Description : Set the value of the member CFG_RESERV_1_REG_1).reserv_1_wr_addr)
 *  Input       : osal_u32 reserv_1_wr_addr: 10 bits
 *  Return      : void
 */
osal_void hal_set_cfg_reserv_1_reg_1_reserv_1_wr_addr(osal_u32 x_WCW_qw0wT_wGFF__)
{
    osal_u32 cfg_reserv_1_reg_1 = hal_reg_read(HH503_PHY_BANK3_BASE_0x2B4);
    u_cfg_reserv_1_reg_1 val;
    val.u32 = cfg_reserv_1_reg_1;
    val.bits.x_WCW_qw0wT_wGFF__ = x_WCW_qw0wT_wGFF__;
    hal_reg_write(HH503_PHY_BANK3_BASE_0x2B4, val.u32);
}
/*
 *  Function    : cfg_reserv_1_reg_1_cfg_reserv_1_wr_en
 *  Description : Set the value of the member CFG_RESERV_1_REG_1).cfg_reserv_1_wr_en)
 *  Input       : osal_u32 cfg_reserv_1_wr_en: 1 bits
 *  Return      : void
 */
osal_void hal_set_cfg_reserv_1_reg_1_cfg_reserv_1_wr_en(osal_u32 xCt2OwmsmwPOrOqwOmo_)
{
    osal_u32 cfg_reserv_1_reg_1 = hal_reg_read(HH503_PHY_BANK3_BASE_0x2B4);
    u_cfg_reserv_1_reg_1 val;
    val.u32 = cfg_reserv_1_reg_1;
    val.bits.xCt2OwmsmwPOrOqwOmo_ = xCt2OwmsmwPOrOqwOmo_;
    hal_reg_write(HH503_PHY_BANK3_BASE_0x2B4, val.u32);
}
/*
 *  Function    : cfg_reserv_1_reg_2_reserv_1_wr_value_0ch
 *  Description : Set the value of the member CFG_RESERV_1_REG_2).reserv_1_wr_value_0ch)
 *  Input       : osal_u32 reserv_1_wr_value_0ch: 16 bits
 *  Return      : void
 */
osal_void hal_set_cfg_reserv_1_reg_2_reserv_1_wr_value_0ch(osal_u32 x_WCW_qw0wT_wqGodWwy6Q_)
{
    osal_u32 cfg_reserv_1_reg_2 = hal_reg_read(HH503_PHY_BANK3_BASE_0x2B8);
    u_cfg_reserv_1_reg_2 val;
    val.u32 = cfg_reserv_1_reg_2;
    val.bits.x_WCW_qw0wT_wqGodWwy6Q_ = x_WCW_qw0wT_wqGodWwy6Q_;
    hal_reg_write(HH503_PHY_BANK3_BASE_0x2B8, val.u32);
}
/*
 *  Function    : he_tx_foc_power_boost_win_ctrl_cfg_tx_power_boost_bypass
 *  Description : Get the value of the member HE_TX_FOC_POWER_BOOST_WIN_CTRL).cfg_tx_power_boost_bypass)
 *  Input       : osal_void
 *  Return      : osal_u32 cfg_tx_power_boost_bypass: 1 bits
 */
osal_u32 hal_get_he_tx_foc_power_boost_win_ctrl_cfg_tx_power_boost_bypass(osal_void)
{
    osal_u32 he_tx_foc_power_boost_win_ctrl = hal_reg_read(HH503_PHY_BANK5_BASE_0x64);
    u_he_tx_foc_power_boost_win_ctrl val;
    val.u32 = he_tx_foc_power_boost_win_ctrl;
    return val.bits.xCt2OR8OGvqmwOyvvsROyQGzss_;
}
#ifdef _PRE_WLAN_FEATURE_DAQ
// phy bank1
/*
 *  Function    : sample_cfg_2_cfg_debug_sample_by_len
 *  Description : Set the value of the member SAMPLE_CFG_2).cfg_debug_sample_by_len)
 *  Input       : osal_u32 cfg_debug_sample_by_len: 1 bits
 *  Return      : void
 */
void hal_set_sample_cfg_2_cfg_debug_sample_by_len(osal_u32 x6NgwFWjdgwCGSuoWwjtwoWh_)
{
    osal_u32 sample_cfg_2 = hal_reg_read(HH503_PHY_BANK1_BASE_0x258);
    u_sample_cfg_2 val;
    val.u32 = sample_cfg_2;
    val.bits.x6NgwFWjdgwCGSuoWwjtwoWh_ = x6NgwFWjdgwCGSuoWwjtwoWh_;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x258, val.u32);
}
/*
 *  Function    : sample_cfg_0_cfg_debug_sample_end_mode
 *  Description : Set the value of the member SAMPLE_CFG_0).cfg_debug_sample_end_mode)
 *  Input       : osal_u32 cfg_debug_sample_end_mode: 6 bits
 *  Return      : void
 */
void hal_set_sample_cfg_0_cfg_debug_sample_end_mode(osal_u32 cfg_debug_sample_end_mode)
{
    osal_u32 sample_cfg_0 = hal_reg_read(HH503_PHY_BANK1_BASE_0x250);
    u_sample_cfg_0 val;
    val.u32 = sample_cfg_0;
    val.bits.cfg_debug_sample_end_mode = cfg_debug_sample_end_mode;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x250, val.u32);
}

/*
 *  Function    : sample_cfg_0_cfg_debug_sample_start_mode
 *  Description : Set the value of the member SAMPLE_CFG_0).cfg_debug_sample_start_mode)
 *  Input       : osal_u32 cfg_debug_sample_start_mode: 6 bits
 *  Return      : void
 */
void hal_set_sample_cfg_0_cfg_debug_sample_start_mode(osal_u32 cfg_debug_sample_start_mode)
{
    osal_u32 sample_cfg_0 = hal_reg_read(HH503_PHY_BANK1_BASE_0x250);
    u_sample_cfg_0 val;
    val.u32 = sample_cfg_0;
    val.bits.cfg_debug_sample_start_mode = cfg_debug_sample_start_mode;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x250, val.u32);
}
/*
 *  Function    : sample_cfg_2_cfg_with_sample_delay_en
 *  Description : Set the value of the member SAMPLE_CFG_2).cfg_with_sample_delay_en)
 *  Input       : osal_u32 cfg_with_sample_delay_en: 1 bits
 *  Return      : void
 */
void hal_set_sample_cfg_2_cfg_with_sample_delay_en(osal_u32 cfg_with_sample_delay_en)
{
    osal_u32 sample_cfg_2 = hal_reg_read(HH503_PHY_BANK1_BASE_0x258);
    u_sample_cfg_2 val;
    val.u32 = sample_cfg_2;
    val.bits.cfg_with_sample_delay_en = cfg_with_sample_delay_en;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x258, val.u32);
}
/*
 *  Function    : sample_cfg_2_cfg_debug_sample_delay
 *  Description : Set the value of the member SAMPLE_CFG_2).cfg_debug_sample_delay)
 *  Input       : osal_u32 cfg_debug_sample_delay: 8 bits
 *  Return      : void
 */
void hal_set_sample_cfg_2_cfg_debug_sample_delay(osal_u32 cfg_debug_sample_delay)
{
    osal_u32 sample_cfg_2 = hal_reg_read(HH503_PHY_BANK1_BASE_0x258);
    u_sample_cfg_2 val;
    val.u32 = sample_cfg_2;
    val.bits.cfg_debug_sample_delay = cfg_debug_sample_delay;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x258, val.u32);
}
/*
 *  Function    : sample_cfg_2_cfg_with_mac_tsf_en
 *  Description : Set the value of the member SAMPLE_CFG_2).cfg_with_mac_tsf_en)
 *  Input       : osal_u32 cfg_with_mac_tsf_en: 1 bits
 *  Return      : void
 */
void hal_set_sample_cfg_2_cfg_with_mac_tsf_en(osal_u32 cfg_with_mac_tsf_en)
{
    osal_u32 sample_cfg_2 = hal_reg_read(HH503_PHY_BANK1_BASE_0x258);
    u_sample_cfg_2 val;
    val.u32 = sample_cfg_2;
    val.bits.cfg_with_mac_tsf_en = cfg_with_mac_tsf_en;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x258, val.u32);
}

/*
 *  Function    : sample_cfg_2_cfg_with_mac_info_en
 *  Description : Set the value of the member SAMPLE_CFG_2).cfg_with_mac_info_en)
 *  Input       : osal_u32 cfg_with_mac_info_en: 1 bits
 *  Return      : void
 */
void hal_set_sample_cfg_2_cfg_with_mac_info_en(osal_u32 cfg_with_mac_info_en)
{
    osal_u32 sample_cfg_2 = hal_reg_read(HH503_PHY_BANK1_BASE_0x258);
    u_sample_cfg_2 val;
    val.u32 = sample_cfg_2;
    val.bits.cfg_with_mac_info_en = cfg_with_mac_info_en;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x258, val.u32);
}
/*
 *  Function    : sample_cfg_0_cfg_debug_sample_en
 *  Description : Set the value of the member SAMPLE_CFG_0).cfg_debug_sample_en)
 *  Input       : osal_u32 cfg_debug_sample_en: 1 bits
 *  Return      : void
 */
void hal_set_sample_cfg_0_cfg_debug_sample_en(osal_u32 xCt2O4myS2OszIGxmOmo_)
{
    osal_u32 sample_cfg_0 = hal_reg_read(HH503_PHY_BANK1_BASE_0x250);
    u_sample_cfg_0 val;
    val.u32 = sample_cfg_0;
    val.bits.xCt2O4myS2OszIGxmOmo_ = xCt2O4myS2OszIGxmOmo_;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x250, val.u32);
}
// phy0
/*
 *  Function    : wlbb_clr_sample_clear_sample_done
 *  Description : Set the value of the member WLBB_CLR_SAMPLE).clear_sample_done)
 *  Input       : osal_u32 clear_sample_done: 1 bits
 *  Return      : void
 */
void hal_set_wlbb_clr_sample_clear_sample_done(osal_u32 xCxmzwOszIGxmO4vom_)
{
    osal_u32 wlbb_clr_sample = hal_reg_read(HH503_PHY0_CTL_BASE_0xC8);
    u_wlbb_clr_sample val;
    val.u32 = wlbb_clr_sample;
    val.bits.xCxmzwOszIGxmO4vom_ = xCxmzwOszIGxmO4vom_;
    hal_reg_write(HH503_PHY0_CTL_BASE_0xC8, val.u32);
}
/*
 *  Function    : sample_cfg_0_cfg_phy_data_sample_clk_en
 *  Description : Set the value of the member SAMPLE_CFG_0).cfg_phy_data_sample_clk_en)
 *  Input       : osal_u32 cfg_phy_data_sample_clk_en: 1 bits
 *  Return      : void
 */
osal_void hal_set_sample_cfg_0_cfg_phy_data_sample_clk_en(osal_u32 x6NgwuQtwFGLGwCGSuoWw6oKwWh_)
{
    osal_u32 sample_cfg_0 = hal_reg_read(HH503_PHY_BANK1_BASE_0x250);
    u_sample_cfg_0 val;
    val.u32 = sample_cfg_0;
    val.bits.x6NgwuQtwFGLGwCGSuoWw6oKwWh_ = x6NgwuQtwFGLGwCGSuoWw6oKwWh_;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x250, val.u32);
}
/*
 *  Function    : sample_cfg_0_cfg_phy_data_sample_cg_bps
 *  Description : Set the value of the member SAMPLE_CFG_0).cfg_phy_data_sample_cg_bps)
 *  Input       : osal_u32 cfg_phy_data_sample_cg_bps: 1 bits
 *  Return      : void
 */
osal_void hal_set_sample_cfg_0_cfg_phy_data_sample_cg_bps(osal_u32 x6NgwuQtwFGLGwCGSuoWw6gwjuC_)
{
    osal_u32 sample_cfg_0 = hal_reg_read(HH503_PHY_BANK1_BASE_0x250);
    u_sample_cfg_0 val;
    val.u32 = sample_cfg_0;
    val.bits.x6NgwuQtwFGLGwCGSuoWw6gwjuC_ = x6NgwuQtwFGLGwCGSuoWw6gwjuC_;
    hal_reg_write(HH503_PHY_BANK1_BASE_0x250, val.u32);
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
