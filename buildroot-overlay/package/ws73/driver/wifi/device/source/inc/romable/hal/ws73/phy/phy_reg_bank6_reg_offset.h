// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __PHY_REG_BANK6_REG_OFFSET_H__
#define __PHY_REG_BANK6_REG_OFFSET_H__
#ifdef BUILD_UT
#include "wifi_ut_stub.h"
#endif
#ifndef BUILD_UT
#define HH503_PHY_BANK6_BASE (0x40013800)
#else
#define HH503_PHY_BANK6_BASE (osal_u32)g_phy_reg_bank6_addr
#endif

#define HH503_PHY_BANK6_OFFEST_BUTT (0 + 4)

#endif // __PHY_REG_BANK6_REG_OFFSET_H__
