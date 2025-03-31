
/*
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: hal alg func.
 * Create: 2022-1-12
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_alg_rts.h"
#include "hal_ext_if.h"
#include "hal_mac_reg.h"
#include "mac_frame.h"
#include "hal_mac.h"
#include "hmac_ext_if.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_ALG_RTS_C

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_RTS
/*****************************************************************************
功能描述  : 配置RTS初始化速率
*****************************************************************************/
osal_void hal_set_rts_rate_init(osal_void)
{
    u_rts_autorate3 rts_rate0_3;
    u_rts_autorate4 rts_rate4_7;
    rts_rate0_3.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x144);
    rts_rate4_7.u32 = hal_reg_read(HH503_MAC_CTRL1_BANK_BASE_0x148);
    /* 0 11b 1M long preamble */
    rts_rate0_3.bits.x6Ngw_LCw_GLWw_GhKwqGoywS6C_      = 0b0000; /* 0b0000 : 1M */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzx3Oyq_       = 0b00;   /* 0b00 : 20M */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzx3OGwvRvCvx_ = 0b0;    /* 0b0 : 11b */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzx3OGwmzIyxm_ = 0b1;    /* 0b1 : long preamble */
    /* 1 11b 5M5 */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzxrOICs_      = 0b0010; /* 0b0010 : 5M5 */
    rts_rate0_3.bits.x6Ngw_LCw_GLWw_GhKwqGo0wjT_       = 0b00;   /* 0b00 : 20M */
    rts_rate0_3.bits.x6Ngw_LCw_GLWw_GhKwqGo0wu_iLi6io_ = 0b0;    /* 0b0 : 11b */
    rts_rate0_3.bits.x6Ngw_LCw_GLWw_GhKwqGo0wu_WGSjoW_ = 0b0;    /* 0b0 : short preamble */
    /* 2 11b 11M */
    rts_rate0_3.bits.x6Ngw_LCw_GLWw_GhKwqGoVwS6C_      = 0b0011; /* 0b0011 : 11M */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzxZOyq_       = 0b00;   /* 0b00 : 20M */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzxZOGwvRvCvx_ = 0b0;    /* 0b0 : 11b */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzxZOGwmzIyxm_ = 0b0;    /* 0b0 : short preamble */
    /* 3 11g 6M bw 00-20M 11-duplocate 40M */
    rts_rate0_3.bits.x6Ngw_LCw_GLWw_GhKwqGokwS6C_      = 0b1011; /* 0b1011 : 6M */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzxiOyq_       = 0b00;   /* 0b00 : 20M */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzxiOGwvRvCvx_ = 0b1;    /* 0b1 : 11g */
    rts_rate0_3.bits.xCt2OwRsOwzRmOwzodOPzxiOGwmzIyxm_ = 0b0;    /* 0b0 : short preamble */
    /* 4 11g 12M bw 00-20M 11-duplocate 40M */
    rts_rate4_7.bits.xCt2OwRsOwzRmOwzodOPzxKOICs_      = 0b1010; /* 0b1010 : 12M */
    rts_rate4_7.bits.x6Ngw_LCw_GLWw_GhKwqGoDwjT_       = 0b00;   /* 0b00 : 20M */
    rts_rate4_7.bits.x6Ngw_LCw_GLWw_GhKwqGoDwu_iLi6io_ = 0b1;    /* 0b1 : 11g */
    rts_rate4_7.bits.x6Ngw_LCw_GLWw_GhKwqGoDwu_WGSjoW_ = 0b0;    /* 0b0 : short preamble */
    /* 5 11g 24M bw 00-20M 11-duplocate 40M */
    rts_rate4_7.bits.x6Ngw_LCw_GLWw_GhKwqGoxwS6C_      = 0b1001; /* 0b1001 : 24M */
    rts_rate4_7.bits.xCt2OwRsOwzRmOwzodOPzxaOyq_       = 0b00;   /* 0b00 : 20M */
    rts_rate4_7.bits.xCt2OwRsOwzRmOwzodOPzxaOGwvRvCvx_ = 0b1;    /* 0b1 : 11g */
    rts_rate4_7.bits.xCt2OwRsOwzRmOwzodOPzxaOGwmzIyxm_ = 0b0;    /* 0b0 : short preamble */
    /* 剩余两块速率寄存器暂时不适配，仅使用6个 */
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x144, rts_rate0_3.u32);
    hal_reg_write(HH503_MAC_CTRL1_BANK_BASE_0x148, rts_rate4_7.u32);
}
/*****************************************************************************
功能描述  : 配置cfg_rts_fail_tx_psdu_en
*****************************************************************************/
osal_void hal_set_rts_fail_tx_psdu_en(oal_bool_enum_uint8 x6Ngw_LCwNGBowLPwuCFdwWh_)
{
    u_pa_control1 pa_ctrl;
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.x6Ngw_LCwNGBowLPwuCFdwWh_ = x6Ngw_LCwNGBowLPwuCFdwWh_;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);
}
#endif /* _PRE_WLAN_FEATURE_RTS */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

