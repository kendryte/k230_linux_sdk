/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file of WLAN product specification macro definition.
 * Create: 2020-10-18
 */

#ifndef __WLAN_SPEC_H__
#define __WLAN_SPEC_H__

/*****************************************************************************
    其他头文件包含
*****************************************************************************/
#include "wlan_types_common.h"
#include "wlan_spec_type.h"
#include "wlan_spec_hh503.h"
#include "oal_types.h"
#include "oal_mem_pool.h"
#include "wlan_phy_common.h"
#include "wlan_spec_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef ALG_DEBUG_FLAG
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
#define ALG_DEBUG_FLAG 1
#else
#define ALG_DEBUG_FLAG 0
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of file */

