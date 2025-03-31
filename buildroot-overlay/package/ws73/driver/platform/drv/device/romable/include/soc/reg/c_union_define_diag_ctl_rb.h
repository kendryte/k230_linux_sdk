// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
// File name     :  c_union_define_DIAG_CTL_RB.h
// Project line  :
// Department    :
// Author        :  ws73 SoC
// Version       :  V100
// Date          :  2020/7/27
// Description   :  ws73 SERIES
// Others        :  Generated automatically by nManager V4.1
// History       :  ws73 SoC 2022/07/08 15:00:45 Create file
// ******************************************************************************

#ifndef __C_UNION_DEFINE_DIAG_CTL_RB_H__
#define __C_UNION_DEFINE_DIAG_CTL_RB_H__

/* Define the union u_cfg_high_clk_mode */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_diag_high_mode : 1; /* [0]  */
        osal_u32 reserved_0 : 31;        /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_high_clk_mode;

/* Define the union u_cfg_pkt_ram_en_del */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 reserved_0 : 8;  /* [7..0]  */
        osal_u32 reserved_1 : 24; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_pkt_ram_en_del;

/* Define the union u_cfg_pkt_wr_rd_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_wr_rd_sel : 8; /* [7..0]  */
        osal_u32 reserved_0 : 24;   /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_pkt_wr_rd_sel;

/* Define the union u_cfg_pkt_ram_clk_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 reserved_0 : 8;  /* [7..0]  */
        osal_u32 reserved_1 : 24; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_pkt_ram_clk_en;

/* Define the union u_cfg_pc_ra_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_cpu_trace_sel : 1; /* [0]  */
        osal_u32 reserved_0 : 31;       /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_pc_ra_sel;

/* Define the union u_cfg_apb_diag_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 rfreg_apb_diag_en : 1; /* [0]  */
        osal_u32 wlmac_apb_diag_en : 1; /* [1]  */
        osal_u32 wlphy_apb_diag_en : 1; /* [2]  */
        osal_u32 reserved_0 : 29;       /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_apb_diag_en;

/* Define the union u_cfg_sample_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_sample_sel : 4; /* [3..0]  */
        osal_u32 reserved_0 : 28;    /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_sample_sel;

/* Define the union u_cfg_sample_node_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_sample_node_sel : 6; /* [5..0]  */
        osal_u32 reserved_0 : 26;         /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_sample_node_sel;

/* Define the union u_cfg_sample_length */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_sample_length : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;        /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_sample_length;

/* Define the union u_cfg_sample_mode */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_sample_mode : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;     /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_sample_mode;

/* Define the union u_cfg_sample_gen_sync */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_sample_sync : 1; /* [0]  */
        osal_u32 reserved_0 : 31;     /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_sample_gen_sync;

/* Define the union u_cfg_sample_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_sample_en : 1; /* [0]  */
        osal_u32 reserved_0 : 31;   /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_sample_en;

/* Define the union u_sample_done */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 sample_done : 1; /* [0]  */
        osal_u32 mac_hit_end : 1; /* [1]  */
        osal_u32 bus_bit_end : 1; /* [2]  */
        osal_u32 reserved_0 : 29; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_sample_done;

/* Define the union u_obs_diag_clk_dbg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 obs_sample_clk_dbg : 1; /* [0]  */
        osal_u32 reserved_0 : 31;        /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_obs_diag_clk_dbg;

/* Define the union u_loop_int_depth */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_fifo_length : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;      /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_loop_int_depth;

/* Define the union u_diag_int_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 diag_int_clr : 1; /* [0]  */
        osal_u32 diag_int_en : 1;  /* [1]  */
        osal_u32 reserved_0 : 30;  /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_diag_int_en;

/* Define the union u_diag_int */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 diag_mode1_int : 1; /* [0]  */
        osal_u32 diag_mode0_int : 1; /* [1]  */
        osal_u32 reserved_0 : 30;    /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_diag_int;

/* Define the union u_diag_dly */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_dly_cnt : 8; /* [7..0]  */
        osal_u32 cfg_dly_en : 1;  /* [8]  */
        osal_u32 dly_end_sts : 1; /* [9]  */
        osal_u32 reserved_0 : 22; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_diag_dly;

/* Define the union u_diag_bus_monitor */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_bus_monitor_en : 1; /* [0]  */
        osal_u32 reserved_0 : 31;        /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_diag_bus_monitor;

/* Define the union u_cfg_diag_jump */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_jump_length : 8; /* [7..0]  */
        osal_u32 cfg_jump_en : 1;     /* [8]  */
        osal_u32 reserved_0 : 23;     /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_diag_jump;

/* Define the union u_monitor_fifo_status */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 looping_sample_ovf : 1; /* [0]  */
        osal_u32 bfg_fifo_empty : 1;     /* [1]  */
        osal_u32 bfg_fifo_overflow : 1;  /* [2]  */
        osal_u32 reserved_0 : 29;        /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_monitor_fifo_status;

/* Define the union u_cfg_test_gen_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_gen_sel : 4; /* [3..0]  */
        osal_u32 reserved_0 : 28; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_test_gen_sel;

/* Define the union u_cfg_test_gen_mode */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_gen_mode : 2; /* [1..0]  */
        osal_u32 reserved_0 : 30;  /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_test_gen_mode;

/* Define the union u_cfg_test_gen_node */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_gen_node_sel : 6; /* [5..0]  */
        osal_u32 reserved_0 : 26;      /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_test_gen_node;

/* Define the union u_cfg_test_gen_length */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_gen_length : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;     /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_test_gen_length;

/* Define the union u_cfg_test_gen_offset */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_gen_offset : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;     /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_test_gen_offset;

/* Define the union u_cfg_test_gen_delay_num */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_gen_delay_num : 8; /* [7..0]  */
        osal_u32 reserved_0 : 24;       /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_test_gen_delay_num;

/* Define the union u_cfg_gen_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_gen_en : 1;  /* [0]  */
        osal_u32 reserved_0 : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_gen_en;

/* Define the union u_gen_done */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 gen_done : 1;    /* [0]  */
        osal_u32 reserved_0 : 31; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_gen_done;

/* Define the union u_cfg_monitor_clock */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_monitor_clken : 1;        /* [0]  */
        osal_u32 cfg_monitor_rd_src_clken : 1; /* [1]  */
        osal_u32 cfg_monitor_src_clken : 1;    /* [2]  */
        osal_u32 reserved_0 : 29;              /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_monitor_clock;

/* Define the union u_obs_gen_clk_dbg */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 obs_gen_clk_dbg : 1; /* [0]  */
        osal_u32 reserved_0 : 31;     /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_obs_gen_clk_dbg;

/* Define the union u_cfg_diag_mux */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_diag_mux : 4; /* [3..0]  */
        osal_u32 reserved_0 : 28;  /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_diag_mux;

/* Define the union u_cfg_group0_pin_sel_0_7 */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_pin_sel_0 : 3; /* [2..0]  */
        osal_u32 reserved_0 : 1;    /* [3]  */
        osal_u32 cfg_pin_sel_1 : 3; /* [6..4]  */
        osal_u32 reserved_1 : 1;    /* [7]  */
        osal_u32 cfg_pin_sel_2 : 3; /* [10..8]  */
        osal_u32 reserved_2 : 1;    /* [11]  */
        osal_u32 cfg_pin_sel_3 : 3; /* [14..12]  */
        osal_u32 reserved_3 : 1;    /* [15]  */
        osal_u32 cfg_pin_sel_4 : 3; /* [18..16]  */
        osal_u32 reserved_4 : 1;    /* [19]  */
        osal_u32 cfg_pin_sel_5 : 3; /* [22..20]  */
        osal_u32 reserved_5 : 1;    /* [23]  */
        osal_u32 cfg_pin_sel_6 : 3; /* [26..24]  */
        osal_u32 reserved_6 : 1;    /* [27]  */
        osal_u32 cfg_pin_sel_7 : 3; /* [30..28]  */
        osal_u32 reserved_7 : 1;    /* [31]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_group0_pin_sel_0_7;

/* Define the union u_cfg_clock_test_sel */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_clock_test_sel : 4; /* [3..0]  */
        osal_u32 reserved_0 : 28;        /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_clock_test_sel;

/* Define the union u_cfg_clock_test_div */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_clock_test_div : 4; /* [3..0]  */
        osal_u32 cfg_clock_test_en : 1;  /* [4]  */
        osal_u32 reserved_0 : 27;        /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_clock_test_div;

/* Define the union u_cfg_clock_test_gate_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_clock_test_gate_en : 8; /* [7..0]  */
        osal_u32 reserved_0 : 24;            /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_clock_test_gate_en;

/* Define the union u_cfg_clock_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_ref_cnt : 16; /* [15..0]  */
        osal_u32 reserved_0 : 16;  /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_clock_cnt;

/* Define the union u_cfg_clock_cnt_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 cfg_clk_cnt_en : 1; /* [0]  */
        osal_u32 reserved_0 : 31;    /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_cfg_clock_cnt_en;

/* Define the union u_rpt_clock_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 test_cnt_num : 31; /* [30..0]  */
        osal_u32 test_num_vld : 1;  /* [31]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_rpt_clock_cnt;

/* Define the union u_pcpu_load */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pcpu_load : 1;    /* [0]  */
        osal_u32 pcpu_load_ok : 1; /* [1]  */
        osal_u32 btdm_int_timeout_clr : 1; /* [2]  */
        osal_u32 btdm_int_timeout_1stick : 1; /* [3]  */
        osal_u32 reserved_0 : 28;  /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pcpu_load;

/* Define the union u_pcpu_pclrsp_en */
typedef union {
    /* Define the struct bits */
    struct {
        osal_u32 pcpu_pclrsp_en : 2   ; /* [1..0] */
        osal_u32 reserved_0 : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    osal_u32 u32;
} u_pcpu_pclrsp_en;

#endif /* __C_UNION_DEFINE_DIAG_CTL_RB_H__ */
