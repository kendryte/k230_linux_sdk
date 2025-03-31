/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header files shared by wlan_spec.h and wlan_spec_rom.h.
 * Create: 2021-09-18
 */

#ifndef __WLAN_SPEC_TYPE_ROM_H__
#define __WLAN_SPEC_TYPE_ROM_H__

#include "osal_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    枚举定义
*****************************************************************************/
/* TX 描述符 12行 b23:b22 phy工作模式; hmac设置phy速率逻辑使用, 故移至此处 */
typedef enum {
    WLAN_11B_PHY_PROTOCOL_MODE              = 0,    /* 11b CCK */
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE      = 1,    /* 11g/a OFDM */
    WLAN_HT_PHY_PROTOCOL_MODE               = 2,    /* 11n HT */
    WLAN_VHT_PHY_PROTOCOL_MODE              = 3,    /* 11ac VHT */
    WLAN_HE_SU_FORMAT                       = 4,    /* 11ax HE SU Format type */
    WLAN_HE_MU_FORMAT                       = 5,    /* 11ax HE MU Format type */
    WLAN_HE_EXT_SU_FORMAT                   = 6,    /* 11ax HE EXT SU Format type */
    WLAN_HE_TRIG_FORMAT                     = 7,    /* 11ax HE TRIG Format type */
    WLAN_PHY_PROTOCOL_BUTT
}wlan_phy_protocol_enum;
typedef osal_u8 wlan_phy_protocol_enum_uint8;
/* TPC 速率-功率表格查询索引值，请勿修改 */
typedef enum {
    WLAN_POW_PROTOCOL_11B = 0,   // 11b
    WLAN_POW_PROTOCOL_LEGACY_OFDM = 1,   // 11a/g
    WLAN_POW_PROTOCOL_NONLEGACY_OFDM = 2,    // 11n 11ac 11ax

    WLAN_POW_PROTOCOL_BUTT
} wlan_pow_protocol_enum;
typedef osal_u8 wlan_pow_protocol_enum_uint8;
/* 基本全带宽枚举 TPC 速率-功率表格查询索引值，请勿修改 */
typedef enum {
    WLAN_BANDWIDTH_20 = 0,                      /* 20MHz全带宽 */
    WLAN_BANDWIDTH_40 = 1,                      /* 40MHz全带宽 */
    WLAN_BANDWIDTH_2G_BUTT = 2,                 /* 2G只有40M带宽 */
    WLAN_BANDWIDTH_80 = WLAN_BANDWIDTH_2G_BUTT, /* 80MHz全带宽 */
    WLAN_BANDWIDTH_160 = 3,                     /* 160MHz全带宽 */
    WLAN_BANDWIDTH_BUTT
} wlan_bandwidth_type_enum;
typedef osal_u8 wlan_bandwidth_type_enum_uint8;
#define WLAN_POW_RATE_POW_CODE_INVALID_OFFSET 0xff
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_spec_rom.h */
