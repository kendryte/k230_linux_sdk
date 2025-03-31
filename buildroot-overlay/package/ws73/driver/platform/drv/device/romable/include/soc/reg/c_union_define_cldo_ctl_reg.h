// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_CLDO_CTL_REG.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_CLDO_CTL_REG_H__
#define __C_UNION_DEFINE_CLDO_CTL_REG_H__

/* Define the union u_soft_int_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 soft_int0_en : 1; /* [0]  */
        osal_u32 soft_int1_en : 1; /* [1]  */
        osal_u32 soft_int2_en : 1; /* [2]  */
        osal_u32 reserved_0 : 29;  /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_soft_int_en;

/* Define the union u_soft_int_set */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 soft_int0_set : 1; /* [0]  */
        osal_u32 soft_int1_set : 1; /* [1]  */
        osal_u32 soft_int2_set : 1; /* [2]  */
        osal_u32 reserved_0 : 29;   /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_soft_int_set;

/* Define the union u_soft_int_clr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 soft_int0_clr : 1; /* [0]  */
        osal_u32 soft_int1_clr : 1; /* [1]  */
        osal_u32 soft_int2_clr : 1; /* [2]  */
        osal_u32 reserved_0 : 29;   /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_soft_int_clr;

/* Define the union u_soft_int_sts */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 soft_int0_sts : 1; /* [0]  */
        osal_u32 soft_int1_sts : 1; /* [1]  */
        osal_u32 soft_int2_sts : 1; /* [2]  */
        osal_u32 reserved_0 : 29;   /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_soft_int_sts;

/* Define the union u_mac_pkt_bit_mode */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 mac_pkt_bit_mode : 1; /* [0]  */
        osal_u32 reserved_0 : 31;      /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_mac_pkt_bit_mode;

/* Define the union u_pkt_ram_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pkt_pre_rd_move : 1; /* [0]  */
        osal_u32 pkt_fifo_rd_sel : 1; /* [1]  */
        osal_u32 reserved_0 : 30;     /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pkt_ram_cfg;

/* Define the union u_icm_priority */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bt_sub_icm_priority : 6;   /* [5..0]  */
        osal_u32 reserved_0 : 2;            /* [7..6]  */
        osal_u32 pkt_ram_icm_priority : 8;  /* [15..8]  */
        osal_u32 perp_apb_icm_priority : 6; /* [21..16]  */
        osal_u32 reserved_1 : 2;            /* [23..22]  */
        osal_u32 cpu_slv_icm_priority : 2;  /* [25..24]  */
        osal_u32 usb_cfg_icm_priority : 6;  /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_icm_priority;

/* Define the union u_cpu_ram_priority */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cpu_ram1_icm_priority : 8; /* [7..0]  */
        osal_u32 cpu_ram2_icm_priority : 8; /* [15..8]  */
        osal_u32 us_dbg_dma_icm_priority : 6; /* [21..16] */
        osal_u32 reserved_0            : 10; /* [31..22] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_icm_priority2;

/* Define the union u_soft_rst_mac_bus */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 soft_rst_mac_bus : 1; /* [0]  */
        osal_u32 soft_rst_soc_cpu : 1; /* [1]  */
        osal_u32 reserved_0 : 30;      /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_soft_rst_mac_bus;

/* Define the union u_bt_brg_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bt_h2h_cg_bypass : 1; /* [0]  */
        osal_u32 bt_h2h_sel : 1;       /* [1]  */
        osal_u32 reserved_0 : 30;      /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_bt_brg_cfg;

/* Define the union u_int_en_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 sdcc_wakeup_int_en : 1; /* [0]  */
        osal_u32 timing_gen_int_en : 1;  /* [1]  */
        osal_u32 reserved_0 : 30;        /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_int_en_ctl;

/* Define the union u_int_mask_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 timing_gen_int_mask : 1;  /* [0]  */
        osal_u32 sdcc_wakeup_int_mask : 1; /* [1]  */
        osal_u32 reserved_0 : 30;          /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_int_mask_ctl;

/* Define the union u_int_clr_ctl */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 sdcc_wakeup_int_clr : 1; /* [0]  */
        osal_u32 timing_gen_int_clr : 1;  /* [1]  */
        osal_u32 reserved_0 : 30;         /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_int_clr_ctl;

/* Define the union u_int_sts */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 sdcc_wakeup_int_sts : 1; /* [0]  */
        osal_u32 timing_gen_int_sts : 1;  /* [1]  */
        osal_u32 reserved_0 : 30;         /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_int_sts;

/* Define the union u_sdio_id */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 sdio_pid : 16; /* [15..0]  */
        osal_u32 sdio_vid : 16; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sdio_id;

/* Define the union u_icm_cg_pkt_ram_fifo_cg_cfg */
typedef union {
    struct {
        osal_u32    icm2x1_cpu_slv_auto_cg_bypass : 1; /* [0] */
        osal_u32    icm3x1_dbgtop_auto_cg_bypass : 1; /* [1] */
        osal_u32    icm3x1_usd_auto_cg_bypass : 1; /* [2] */
        osal_u32    icm3x1_usb_cfg_auto_cg_bypass : 1; /* [3] */
        osal_u32    icm3x1_perp_apb_auto_cg_bypass : 1; /* [4] */
        osal_u32    icm3x1_bt_sub_auto_cg_bypass : 1; /* [5] */
        osal_u32    icm4x1_pkt_ram_auto_cg_bypass : 1; /* [6] */
        osal_u32    icm4x1_cpu_ram1_auto_cg_bypass : 1; /* [7] */
        osal_u32    icm4x1_cpu_ram2_auto_cg_bypass : 1; /* [8] */
        osal_u32    pkt_mem_diag_fifo_clk_auto_cg_en : 1; /* [9] */
        osal_u32    pkt_mem_mac_fifo_clk_auto_cg_en : 1; /* [10] */
        osal_u32    diag_trng_en          : 1; /* [11] */
        osal_u32    trng_clk_div          : 2; /* [13..12] */
        osal_u32    reserved_0            : 18; /* [31..14] */
    } bits;
    osal_u32    u32;
} u_icm_cg_pkt_ram_fifo_cg_cfg;

/* Define the union u_core_stat_mode */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 core_sleep_mode : 1;      /* [0]  */
        osal_u32 core_debug_mode : 1;      /* [1]  */
        osal_u32 core_lockup_mode : 1;     /* [2]  */
        osal_u32 core_hard_fault_mode : 1; /* [3]  */
        osal_u32 reserved_0 : 28;          /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_core_stat_mode;

/* Define the union u_tcm_rom_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 reserved_0 : 7;    /* [6..0] */
        osal_u32 tcm_rom_ema : 3;   /* [9..7]  */
        osal_u32 reserved_1 : 22;   /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_tcm_rom_cfg;

/* Define the union u_mac_monitor */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 mac_monitor_en : 1;       /* [0] */
        osal_u32 mac_monitor_int_clr : 1;  /* [1] */
        osal_u32 mac_monitor_int_mask : 1; /* [2] */
        osal_u32 monitor_mac_addr_en : 8;  /* [10..3] */
        osal_u32 reserved_0 : 21;          /* [31..11] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_mac_monitor;

/* Define the union u_mac_rec_wr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 mac_rec_wr : 1;  /* [0]  */
        osal_u32 reserved_0 : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_mac_rec_wr;

/* Define the union u_mac_monitor_init_stst */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 mac_monitor_init_stst : 8; /* [7..0]  */
        osal_u32 reserved_0 : 24;           /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_mac_monitor_init_stst;

/* Define the union u_cpu_mem_share_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cpu_mem_share_memcken : 6; /* [5..0]  */
        osal_u32 reserved_0 : 26;           /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cpu_mem_share_cfg;

/* Define the union u_share_ram_sw_conflict_clr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 share_ram_sw_conflict_clr : 1; /* [0]  */
        osal_u32 reserved_0 : 31;               /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_share_ram_sw_conflict_clr;

/* Define the union u_share_ram_sw_conflict_addr */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 share_ram_sw_conflict_addr : 24; /* [23..0]  */
        osal_u32 reserved_0 : 8;                  /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_share_ram_sw_conflict_addr;


/* Define the union u_share_ram_sw_conflict_info */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 share_ram_sw_conflict_st : 1;      /* [0]  */
        osal_u32 share_ram_sw_conflict_ram_num : 3; /* [3..1]  */
        osal_u32 reserved_0 : 28;                   /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_share_ram_sw_conflict_info;


/* Define the union u_bank_ram0_use_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bank_ram0_use_cfg : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;       /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_bank_ram0_use_cfg;

/* Define the union u_bank_ram1_use_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bank_ram1_use_cfg : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;       /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_bank_ram1_use_cfg;

/* Define the union u_bank_ram2_use_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bank_ram2_use_cfg : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;       /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_bank_ram2_use_cfg;

/* Define the union u_bank_ram3_use_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bank_ram3_use_cfg : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;       /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_bank_ram3_use_cfg;

/* Define the union u_bank_ram4_use_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bank_ram4_use_cfg : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;       /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_bank_ram4_use_cfg;

/* Define the union u_bank_ram5_use_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 bank_ram5_use_cfg : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;       /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_bank_ram5_use_cfg;


/* Define the union u_dma_soc_cfg */
typedef union {
    struct {
        osal_u32    dma_auto_cg_bypass    : 1   ; /* [0] */
        osal_u32    cur_sts               : 1   ; /* [1] */
        osal_u32    reserved_0            : 30  ; /* [31..2] */
    } bits;
    osal_u32    u32;
} u_dma_soc_cfg;

#endif /* __C_UNION_DEFINE_CLDO_CTL_REG_H__ */
