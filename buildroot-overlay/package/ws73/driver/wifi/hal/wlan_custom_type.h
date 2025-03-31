/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header files shared by wlan_types.h and wlan_types_rom.h.
 * Author: Huanghe
 * Create: 2021-09-18
 */

#ifndef __WLAN_CUSTOM_TYPES_BASE_H__
#define __WLAN_CUSTOM_TYPES_BASE_H__

#include "td_base.h"
#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


typedef enum {
    WLAN_OFDM_ACK_CTS_TYPE_24M = 0,
    WLAN_OFDM_ACK_CTS_TYPE_BUTT
} wlan_ofdm_ack_cts_type_enum;
typedef osal_u8 wlan_ofdm_ack_cts_type_enum_uint8;

typedef enum {
    WLAN_BAND_ASSEMBLE_20M                   = 0,
    WLAN_BAND_ASSEMBLE_SU_ER_242_TONE        = 0, /* 等同20M */
    WLAN_BAND_ASSEMBLE_SU_ER_106_TONE        = 1,
    WLAN_BAND_ASSEMBLE_40M                   = 2,
    WLAN_BAND_ASSEMBLE_40M_DUP               = 3,
    WLAN_BAND_ASSEMBLE_BUTT
} hal_channel_assemble_enum;
typedef osal_u8 hal_channel_assemble_enum_uint8;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_types.h */
