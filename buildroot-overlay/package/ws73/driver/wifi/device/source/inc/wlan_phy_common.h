/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wlan phy
 * Date: 2022-11-26
 */

#ifndef __WLAN_PHY_COMMON_H__
#define __WLAN_PHY_COMMON_H__

#include "wlan_phy_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// 仅11n时有效
typedef enum {
    WLAN_PHY_SMOOTH_TYPE_NOT_RECOMMEDED = 0,
    WLAN_PHY_SMOOTH_TYPE_RECOMMEDED     = 1,
    WLAN_PHY_SMOOTH_TYPE_BUTT
} wlan_phy_smoothing_mode_enum;

// 仅11n时有效
typedef enum {
    WLAN_PHY_UPLINK_FLAG_VALID      = 0,
    WLAN_PHY_UPLINK_FLAG_INVALID    = 1,
    WLAN_PHY_UPLINK_FLAG_BUTT
} wlan_phy_uplink_flag_enum;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_phy_common.h */
