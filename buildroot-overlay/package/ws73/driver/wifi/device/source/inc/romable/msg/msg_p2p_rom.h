/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: P2P initialization and adapt interface implementation.
 */

#ifndef MSG_P2P_ROM_H
#define MSG_P2P_ROM_H

#include "osal_types.h"

/* P2P noa中需要上报到device侧的信息 */
typedef struct {
    wlan_vap_mode_enum_uint8 vap_mode; /* vap模式 */
    oal_bool_enum_uint8 pause;
    osal_u8 reserved[2];
} p2p_info_report_stru;

#endif