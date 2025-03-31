/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  USB_PERI_CFG_RB_c_union_define.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:04 Create file
 */

#ifndef __C_UNION_DEFINE_USB_PERI_CFG_RB_H__
#define __C_UNION_DEFINE_USB_PERI_CFG_RB_H__

/* Define the union u_usb2_ctrl_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: bus_filter_bypass
        /*
         * USB3 VBUS信号过滤器旁路控制
         * bus_filter_bypass[3]：reserved
         * bus_filter_bypass[2]：utmisrp_bvalid信号过滤失效
         * 0：不失效；
         * 1：失效。
         * bus_filter_bypass[1]：reserved
         * bus_filter_bypass[0]：utmiotg_vbusvalid信号过滤失效
         * 0：不失效；
         * 1：失效。
         * 注：Host模式时，必须设置为4'b1111。
         */
        osal_u32 bus_filter_bypass : 4; /* [3..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 1; /* [4] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 1; /* [5] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 1; /* [6] */
        // Comment of field: reserved_3
        /*
         * 保留。
         */
        osal_u32 reserved_3 : 1; /* [7] */
        // Comment of field: reserved_4
        /*
         * 保留。
         */
        osal_u32 reserved_4 : 1; /* [8] */
        // Comment of field: reserved_5
        /*
         * 保留。
         */
        osal_u32 reserved_5 : 11; /* [19..9] */
        // Comment of field: fladj_30mhz_reg
        /*
         * USB2时钟防抖控制。
         */
        osal_u32 fladj_30mhz_reg : 6; /* [25..20] */
        // Comment of field: reserved_6
        /*
         * 保留。
         */
        osal_u32 reserved_6 : 2; /* [27..26] */
        // Comment of field: usb2_phy_utmi_txbitstuffenable
        /*
         * USB2 PHY比特填充使能
         * 0：OPMODE = 3时，比特填充不使能；
         * 1：OPMODE = 3时，比特填充使能。
         */
        osal_u32 usb2_phy_utmi_txbitstuffenable : 1; /* [28] */
        // Comment of field: reserved_7
        /*
         * 保留。
         */
        osal_u32 reserved_7 : 3; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb2_ctrl_cfg;

/* Define the union u_usb2_ctrl_dbg */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: usb_ctrl_obs_sel
        /*
         * usb_ctrl_obs信号源选择，仅Debug时使用。
         */
        osal_u32 usb_ctrl_obs_sel : 4; /* [3..0] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 2; /* [5..4] */
        // Comment of field: reserved_1
        /*
         * 保留。
         */
        osal_u32 reserved_1 : 1; /* [6] */
        // Comment of field: reserved_2
        /*
         * 保留。
         */
        osal_u32 reserved_2 : 1; /* [7] */
        // Comment of field: usb2_vbus_utmisrp_bvalid_val
        /*
         * usb2_vbus_utmisrp_bvalid_val配置，仅Debug时使用。
         */
        osal_u32 usb2_vbus_utmisrp_bvalid_val : 1; /* [8] */
        // Comment of field: usb2_vbus_utmisrp_bvalid_cfg
        /*
         * utmisrp_bvalid信号源选择，仅Debug时使用。
         * 0：来自VBUS；
         * 1：来自usb2_vbus_utmisrp_bvalid_vall。
         */
        osal_u32 usb2_vbus_utmisrp_bvalid_cfg : 1; /* [9] */
        // Comment of field: reserved_3
        /*
         * 保留。
         */
        osal_u32 reserved_3 : 2; /* [11..10] */
        // Comment of field: usb_hs_dbg_addr
        /*
         * USB2 控制器AHB接口读取内容选择，仅Debug时使用。
         * 4'b0000：寄存器；
         * 4'b0010：RAM0；
         */
        osal_u32 usb_hs_dbg_addr : 4; /* [15..12] */
        // Comment of field: usb_ctrl_obs
        /*
         * USB2 控制器观测信号。
         */
        osal_u32 usb_ctrl_obs : 4; /* [19..16] */
        // Comment of field: reserved_4
        /*
         * 保留
         */
        osal_u32 reserved_4 : 12; /* [31..20] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb2_ctrl_dbg;

/* Define the union u_usb2_wkup_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: usb2_wkup_en
        /*
         * bit 1:usb2通过suspendm信号唤醒系统使能。
         * bit 0:usb2通过linestate信号唤醒系统使能。
         */
        osal_u32 usb2_wkup_en : 2; /* [1..0] */
        osal_u32 usb2_wkup_thresh_cfg : 10; /* [11..2] */
        // Comment of field: usb2_wkup_state
        /*
         * bit 13:usb2通过suspendm信号唤醒系统中断状态。
         * bit 0:usb2通过linestate信号唤醒系统中断状态。
         */
        osal_u32 usb2_wkup_state : 2; /* [13..12] */
        // Comment of field: reserved_0
        /*
         * 保留。
         */
        osal_u32 reserved_0 : 18; /* [31..14] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_usb2_wkup_cfg;

#endif /* __USB_PERI_CFG_RB_C_UNION_DEFINE_H__ */
