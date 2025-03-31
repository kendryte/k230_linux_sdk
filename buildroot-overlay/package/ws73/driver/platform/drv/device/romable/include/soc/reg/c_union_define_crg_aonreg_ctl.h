/*
 * Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * File name     :  CRG_AONREG_CTL_c_union_define.h
 * Project line  :
 * Department    :
 * Author        :   SoC
 * Version       :  V100
 * Date          :  44039
 * Description   :   SERIES
 * Others        :  Generated automatically by nManager V5.1
 * History       :   SoC 2022/10/18 21:04:04 Create file
 */

#ifndef __C_UNION_DEFINE_CRG_AONREG_CTL_H__
#define __C_UNION_DEFINE_CRG_AONREG_CTL_H__

/* Define the union u_glb_rst_crg_delay */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: delay_time_rst_crg
        /*
         * crg复位展宽时间配置值
         * 4'd0:不展宽；
         * 4'd1:展宽1拍；
         * …
         * 4;d15:展宽15拍。
         */
        osal_u32 delay_time_rst_crg : 4; /* [3..0] */
        // Comment of field: delay_time_rst_logic
        /*
         * logic复位展宽时间配置值
         * 4'd0:不展宽；
         * 4'd1:展宽1拍；
         * …
         * 4;d15:展宽15拍。
         */
        osal_u32 delay_time_rst_logic : 4; /* [7..4] */
        // Comment of field: delay_time_rst_cpu
        /*
         * CPU复位展宽时间配置值
         * 4'd0:不展宽；
         * 4'd1:展宽1拍；
         * …
         * 4;d15:展宽15拍。
         */
        osal_u32 delay_time_rst_cpu : 4; /* [11..8] */
        osal_u32 delay_timer_rst_wdt_soft : 4; /* [15..12] */
        // Comment of field: reserved_0
        /*
         * reserved
         */
        osal_u32 reserved_0 : 16; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_glb_rst_crg_delay;

/* Define the union u_glb_rst_crg_wdt_mask */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 wdt_glb_ctl_mask : 1; /* [0] */
        osal_u32 wdt_rst_sysldo_gpio_mask : 1; /* [1] */
        osal_u32 wdt_rst_ioconfig_mask : 1; /* [2] */
        osal_u32 wdt_rst_cs_mask : 1; /* [3] */
        osal_u32 wdt_rst_jtag_po_mask : 1; /* [4] */
        osal_u32 wdt_rst_cpu_mask : 1; /* [5] */
        osal_u32 wdt_rst_bus_mask : 1; /* [6] */
        osal_u32 wdt_rst_pkt_mask : 1; /* [7] */
        osal_u32 wdt_rst_diag_ctl_mask : 1; /* [8] */
        osal_u32 wdt_rst_rf_wb_mask : 1; /* [9] */
        osal_u32 wdt_rst_hpm_mask : 1; /* [10] */
        osal_u32 wdt_rst_uart_mask : 1; /* [11] */
        osal_u32 wdt_rst_timer_mask : 1; /* [12] */
        osal_u32 wdt_rst_tsensor_mask : 1; /* [13] */
        osal_u32 wdt_rst_efuse_mask : 1; /* [14] */
        osal_u32 wdt_rst_coex_mask : 1; /* [15] */
        osal_u32 wdt_rst_rtc_mask : 1; /* [16] */
        osal_u32 wdt_rst_sdio_prst_mask : 1; /* [17] */
        osal_u32 wdt_rst_dma_mask      : 1   ; /* [18] */
        osal_u32 wdt_rst_uart_glp_mask : 1   ; /* [19] */
        // Comment of field: reserved_0
        /*
         * reserved
         */
        osal_u32 reserved_0            : 12  ; /* [31..20] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_glb_rst_crg_wdt_mask;

#endif /* __CRG_AONREG_CTL_C_UNION_DEFINE_H__ */
