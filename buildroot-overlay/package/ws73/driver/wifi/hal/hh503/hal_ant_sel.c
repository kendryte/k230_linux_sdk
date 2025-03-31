/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_ant_sel.c.
 * Create: 2023-01-11
 */

#ifdef _PRE_WLAN_FEATURE_ANT_SEL
#include "hal_ant_sel_rom.h"
#include "hal_phy.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_ANT_SEL_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_u32 hal_get_ant_sel_phy_enable(osal_void)
{
    return hal_reg_read_bit(HH503_PHY_BANK1_BASE_0x0, BIT_OFFSET_1, NUM_1_BITS);
}

osal_void hal_set_ant_sel_phy_enable(osal_u8 phy_enable)
{
    hal_reg_write_bits(HH503_PHY_BANK1_BASE_0x0, BIT_OFFSET_1, NUM_1_BITS, phy_enable);
}

osal_u32 hal_get_ant_sel_phy_cfg_man(osal_void)
{
    return hal_reg_read_bit(HH503_PHY_BANK1_BASE_0x18, BIT_OFFSET_2, NUM_1_BITS);
}

osal_void hal_set_ant_sel_phy_cfg_man(osal_u8 cfg_man)
{
    hal_reg_write_bits(HH503_PHY_BANK1_BASE_0x18, BIT_OFFSET_2, NUM_1_BITS, cfg_man);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* _PRE_WLAN_FEATURE_ANT_SEL */
