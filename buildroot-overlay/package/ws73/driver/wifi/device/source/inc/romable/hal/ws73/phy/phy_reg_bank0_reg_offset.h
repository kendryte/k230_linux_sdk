// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __PHY_REG_BANK0_REG_OFFSET_H__
#define __PHY_REG_BANK0_REG_OFFSET_H__
#ifdef BUILD_UT
#include "wifi_ut_stub.h"
#endif
#ifndef BUILD_UT
#define HH503_PHY_BANK0_BASE (0x40040000)
#else
#define HH503_PHY_BANK0_BASE (osal_u32)g_phy_reg_bank0_addr
#endif

#endif // __PHY_REG_BANK0_REG_OFFSET_H__
