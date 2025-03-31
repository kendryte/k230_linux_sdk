/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: oam log printing interface
 */

#ifndef __WLAN_PHY_COMMON_ROM_H__
#define __WLAN_PHY_COMMON_ROM_H__

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
// 11ax
typedef enum {
    WLAN_PHY_HE_GI_DOT8US   = 0,
    WLAN_PHY_HE_GI_1DOT6US  = 1,
    WLAN_PHY_HE_GI_3DOT2US  = 2,
    WLAN_PHY_HE_GI_BUTT
} wlan_phy_he_gi_type_enum;
typedef osal_u8 wlan_phy_he_gi_type_enum_uint8;

/* 重要:仅限描述符接口使用(表示发送该帧使用的pramble类型)，0表示short preamble; 1表示long preamble */
typedef enum {
    WLAN_PHY_PREAMBLE_SHORT_HTMF = 0,
    WLAN_PHY_PREAMBLE_LONG_HTGF = 1,
    WLAN_PHY_PREAMBLE_BUTT
} wlan_phy_preamble_type;
typedef osal_u8 wlan_phy_preamble_type_uint8;

typedef enum {
    WLAN_PHY_DCM_OFF = 0,
    WLAN_PHY_DCM_ON = 1,
    WLAN_PHY_DCM_BUTT
} wlan_phy_dcm_status;
typedef osal_u8 wlan_phy_dcm_status_uint8;

typedef enum {
    WLAN_PHY_HE_LTF_1X   = 0,
    WLAN_PHY_HE_LTF_2X   = 1,
    WLAN_PHY_HE_LTF_4X   = 2,
    WLAN_PHY_HE_LTF_BUTT
} wlan_phy_he_ltf_type_enum;
typedef osal_u8 wlan_phy_he_ltf_type_enum_uint8;

typedef enum {
    WLAN_PHY_CHAIN_SEL_BOTH_OFF  = 0,
    WLAN_PHY_CHAIN_SEL_TYPE_0    = 1,               // 开启通道0
    WLAN_PHY_CHAIN_SEL_TYPE_1    = 2,               // 开启通道1
    WLAN_PHY_CHAIN_SEL_BOTH_ON   = 3,
    WLAN_PHY_CHAIN_SEL_BUTT
} wlan_phy_chain_sel_enum;
typedef osal_u8 wlan_phy_chain_sel_enum_uint8;

typedef enum {
    WLAN_PHY_MAX_PE_FLD_VALUE_0     = 0,
    WLAN_PHY_MAX_PE_FLD_VALUE_8     = 1,
    WLAN_PHY_MAX_PE_FLD_VALUE_16    = 2,
    WLAN_PHY_MAX_PE_FLD_VALUE_BUTT
} wlan_phy_max_pe_fld_enum;

// 速率等级0的保护类型
typedef enum {
    WLAN_PHY_RTS_CTS_PROTECT_TYPE_DISABLE   = 0,    // 不发保护帧
    WLAN_PHY_RTS_CTS_PROTECT_TYPE_RTS       = 1,    // 发RTS帧
    WLAN_PHY_RTS_CTS_PROTECT_TYPE_SELFCTS   = 2,    // 发self cts帧
    WLAN_PHY_RTS_CTS_PROTECT_TYPE_ADJUST    = 3,    // 根据当前帧的帧长来决定是否发rts
    WLAN_PHY_RTS_CTS_PROTECT_TYPE_BUTT
} wlan_phy_rts_cts_protect_mode_enum;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wlan_phy_common_rom.h */
