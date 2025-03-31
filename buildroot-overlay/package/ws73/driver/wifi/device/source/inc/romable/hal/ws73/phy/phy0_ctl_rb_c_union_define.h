// ******************************************************************************
// Copyright     :  Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.

#ifndef __PHY0_CTL_RB_C_UNION_DEFINE_H__
#define __PHY0_CTL_RB_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 xCtCw6LowBF_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_phy_ctl_id;

typedef union {
    struct {
        osal_u32 xCiNLw_CLwTuywFG6wy6Qwh_ : 1;
        osal_u32 xCiNLw_CLwTuywGF6wy6Qwh_ : 1;
        osal_u32 xCiNLw_CLwTuywuQtwUywh_ : 1;
        osal_u32 xCiNLw_CLwTuwuQtwh_ : 1;
        osal_u32 xCiNLw_CLwTuywuQtwLPwh_ : 1;
        osal_u32 xCiNLw_CLwTuywuQtw_Pwh_ : 1;
        osal_u32 xsvtROwsROqG3OGAQORw8Oo_ : 1;
        osal_u32 xsvtROwsROqG3OKKOo_ : 1;
        osal_u32 xsvtROwsROqGO4mCv4mwOo_ : 1;
        osal_u32 xCiNLw_CLwTuywuQtwNLSwh_ : 1;
        osal_u32 xsvtROwsROqG3OGAQOwm2Oo_ : 1;
        osal_u32 xsvtROwsROqG3O4zCOrCAOo_ : 1;
        osal_u32 xsvtROwsROqG3Oz4COrCAOo_ : 1;
        osal_u32 xCiNLw_CLwFuFwGF6wy6Qwh_ : 1;
        osal_u32 xsvtROwsRO4G4Oz4COrCAOo_ : 1;
        osal_u32 reserved_0 : 17;
    } bits;

    osal_u32 u32;
} u_wcbb_soft_reset;

typedef union {
    struct {
        osal_u32 xqG3O4zCO3CAOq4ROwsROsmx_ : 1;
        osal_u32 xqG3Oz4CO3CAOq4ROwsROsmx_ : 1;
        osal_u32 xqG3OGAQOn3Oq4ROwsROsmx_ : 1;
        osal_u32 xqGOGAQOq4ROwsROsmx_ : 1;
        osal_u32 xqG3OGAQOR8Oq4ROwsROsmx_ : 1;
        osal_u32 xqG3OGAQOw8Oq4ROwsROsmx_ : 1;
        osal_u32 xTuywuQtwL_PwTFLw_CLwCWo_ : 1;
        osal_u32 xTuywDDwTFLw_CLwCWo_ : 1;
        osal_u32 xTuwFW6iFW_wTFLw_CLwCWo_ : 1;
        osal_u32 xqG3OGAQOtRIOq4ROwsROsmx_ : 1;
        osal_u32 xTuywuQtw_WgwTFLw_CLwCWo_ : 1;
        osal_u32 xTuywFG6w06QwTFLw_CLwCWo_ : 1;
        osal_u32 xTuywGF6w06QwTFLw_CLwCWo_ : 1;
        osal_u32 x4G4Oz4CO3CAOq4ROwsROsmx_ : 1;
        osal_u32 xFuFwGF6w06QwTFLw_CLwCWo_ : 1;
        osal_u32 reserved_0 : 17;
    } bits;

    osal_u32 u32;
} u_wdt_rst_sel;

typedef union {
    struct {
        osal_u32 xqG3Owm2ltOCxdmo_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xqG3O2xyOYK3IOCxdmo_ : 1;
        osal_u32 xTuywXDyw6oKWh_ : 1;
        osal_u32 xTuywkVyw6oKWh_ : 1;
        osal_u32 xTuyw0Xyw6oKWh_ : 1;
        osal_u32 reserved_1 : 26;
    } bits;

    osal_u32 u32;
} u_wphy_soft_clken0;

typedef union {
    struct {
        osal_u32 xqG3OR8OZ4vIOCxdmo_ : 1;
        osal_u32 xqG3OK8OCxdmo_ : 1;
        osal_u32 xqG3OCzxlOw84tmOz2COCxdmo_ : 1;
        osal_u32 xqG3OR8O4tmOr4vIOCxdmo_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xTuywVPw6oKWh_ : 1;
        osal_u32 xqG3OsQoCO4zRzOGzRAOCxdmo_ : 1;
        osal_u32 xqG3OCCzOCxdmo_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 xTuywUyw6oKWh_ : 1;
        osal_u32 xqG3OKKOCxdmo_ : 1;
        osal_u32 xTuwFW6iFWw6oKWh_ : 1;
        osal_u32 xqGOGZIOCxdmo_ : 1;
        osal_u32 xTuywLPw6oKWh_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xTuyw6GoBwLPw6oKWh_ : 1;
        osal_u32 xTuywNLSw6oKWh_ : 1;
        osal_u32 reserved_3 : 1;
        osal_u32 xqG3OR8O4tmOr4vIOR8sCvOCxdmo_ : 1;
        osal_u32 reserved_4 : 13;
    } bits;

    osal_u32 u32;
} u_wphy_soft_clken1;

typedef union {
    struct {
        osal_u32 xqG3O4zCOCxdmo_ : 1;
        osal_u32 xqG3Oz4COCxdmo_ : 1;
        osal_u32 xTuyw_CCBwGF6w6oKWh_ : 1;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_wcbb_soft_clken2;

typedef union {
    struct {
        osal_u32 xTuyw0XywFBqwhdS_ : 4;
        osal_u32 xTuywkVywFBqwhdS_ : 4;
        osal_u32 xTuywXDywFBqwhdS_ : 4;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_wphy_clk_div_0;

typedef union {
    struct {
        osal_u32 xTuywUywFBqwhdS_ : 2;
        osal_u32 reserved_0 : 6;
        osal_u32 xTuwFW6iFWwFBqwhdS_ : 2;
        osal_u32 xqGOGZIO4lPOoSI_ : 2;
        osal_u32 reserved_1 : 20;
    } bits;

    osal_u32 u32;
} u_wphy_clk_div_1;

typedef union {
    struct {
        osal_u32 xqG3OK8O4lPOoSI_ : 2;
        osal_u32 reserved_0 : 2;
        osal_u32 xqG3OCzxlOw84tmOz2CO4lPOoSI_ : 2;
        osal_u32 reserved_1 : 2;
        osal_u32 xTuywLPwFNWw0FiSwFBqwhdSwSGh_ : 2;
        osal_u32 reserved_2 : 2;
        osal_u32 xqG3OR8O4tmOr4vIO4lPOoSI_ : 2;
        osal_u32 reserved_3 : 2;
        osal_u32 xTuywLPwFNWw0FiSwLPC6iwFBqwhdSwSGh_ : 2;
        osal_u32 reserved_4 : 2;
        osal_u32 xqG3OR8O4tmOr4vIOR8sCvO4lPOoSI_ : 2;
        osal_u32 reserved_5 : 10;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_div_2;

typedef union {
    struct {
        osal_u32 xTuywVPwFBqwhdS_ : 2;
        osal_u32 reserved_0 : 2;
        osal_u32 xqG3OsQoCO4zRzOGzRAO4lPOoSI_ : 2;
        osal_u32 reserved_1 : 2;
        osal_u32 xqG3OCCzO4lPOoSI_ : 2;
        osal_u32 reserved_2 : 6;
        osal_u32 xTuywNLSwFBqwhdS_ : 2;
        osal_u32 reserved_3 : 14;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_div_3;

typedef union {
    struct {
        osal_u32 reserved_0 : 4;
        osal_u32 xqG3Owm2ltO4lPOoSI_ : 2;
        osal_u32 reserved_1 : 26;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_div_4;

typedef union {
    struct {
        osal_u32 xqG3OR8O4lPOoSIOIzo_ : 7;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_div_5;

typedef union {
    struct {
        osal_u32 reserved_0 : 2;
        osal_u32 xTuywLPwFNWwVFiSwFBqwhdS_ : 2;
        osal_u32 reserved_1 : 28;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_div_6;

typedef union {
    struct {
        osal_u32 xTouQtywFBqwWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_clk_div_en;

typedef union {
    struct {
        osal_u32 reserved_0 : 1;
        osal_u32 xqG3On3OrryOCxdsmx_ : 1;
        osal_u32 xqG3OGAQOs3Otwm6OCxdsmx_ : 2;
        osal_u32 xqG3OttROCxdsmxOIzo_ : 1;
        osal_u32 reserved_1 : 3;
        osal_u32 xTuywNNLw6oKCWowSiF_ : 1;
        osal_u32 xTuywFW6iFW_wuQGCWwCWo_ : 1;
        osal_u32 xTuywuVSwuQGCWwCWo_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xqG3OsCvOCxdsmxOIzo_ : 1;
        osal_u32 xTuywC6iw6oKCWowSiF_ : 1;
        osal_u32 reserved_3 : 18;
    } bits;

    osal_u32 u32;
} u_wcbb_clk_sel;

typedef union {
    struct {
        osal_u32 xTuywL_PwC6iw6oKjw6oKih_ : 1;
        osal_u32 xqG3ORw8OsCvOCxdzOCxdvo_ : 1;
        osal_u32 xTuywNNLwLPw6oKih_ : 1;
        osal_u32 xTuywNNLw_Pw6oKih_ : 1;
        osal_u32 xqG3OZ8OttROCxdyOCxdvo_ : 1;
        osal_u32 xTuywVPwNNLw6oKGw6oKih_ : 1;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_clkmux_sts;

typedef union {
    struct {
        osal_u32 xqG3OR8O4tmOr4vIO4lPOoSIOsmx_ : 1;
        osal_u32 xTuywLPwFBqwhdSwCWo_ : 1;
        osal_u32 xqGOGZIO4lPOoSIOsmx_ : 1;
        osal_u32 xqGO4mCv4mwO4lPOoSIOsmx_ : 1;
        osal_u32 xqG3OR8O4tmOr4vIOR8sCvO4lPOoSIOsmx_ : 1;
        osal_u32 reserved_0 : 27;
    } bits;

    osal_u32 u32;
} u_div_num_sel;

typedef union {
    struct {
        osal_u32 xgojwu_W6QG_gWwWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_wl_phy_memory_sleep;

typedef union {
    struct {
        osal_u32 xTowuQtwSWSw6oKwGdLiw6gwWh_ : 1;
        osal_u32 xTowuQtwSWSw6oKwNi_6Wwih_ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_wl_phy_memory_auto;

typedef union {
    struct {
        osal_u32 xTowuQtwSG_gBhwGFbwG_ : 5;
        osal_u32 reserved_0 : 3;
        osal_u32 xqxOGAQOIzw2loOz4WOy_ : 5;
        osal_u32 reserved_1 : 19;
    } bits;

    osal_u32 u32;
} u_wl_phy_memory_adj;

typedef union {
    struct {
        osal_u32 xqxOGAQORGOwzIORIv4_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_wl_phy_tpram_tmod;

typedef union {
    struct {
        osal_u32 xqxOGAQOsGOwzIORIv4_ : 7;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_wl_phy_spram_tmod;

typedef union {
    struct {
        osal_u32 xqxOGAQOwtOwzIORIv4_ : 7;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_wl_phy_rf_ram_tmod;

typedef union {
    struct {
        osal_u32 xqxOGAQOICIOsRzRmOom8R_ : 2;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_wl_phy_md_cfg;

typedef union {
    struct {
        osal_u32 xTowuQtwFBGg_ : 8;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_wl_phy_diag;

typedef union {
    struct {
        osal_u32 wlbb_testmode : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_wlbb_testmode;

typedef union {
    struct {
        osal_u32 xCxmzwOszIGxmO4vom_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_wlbb_clr_sample;

typedef union {
    struct {
        osal_u32 xCxmzwOtltvOzxI_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_wlbb_clr_fifo;

typedef union {
    struct {
        osal_u32 xR8Ol6Om8CAzo2m_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xw8Ol6Om8CAzo2m_ : 1;
        osal_u32 reserved_1 : 27;
    } bits;

    osal_u32 u32;
} u_wlbb_iq_ex;

typedef union {
    struct {
        osal_u32 test_start : 1;
        osal_u32 sample_done : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_test_start;

typedef union {
    struct {
        osal_u32 x6NgwNi_6Ww_Wgw6oKwih_ : 1;
        osal_u32 x6NgwNi_6Ww_Wg0w6oKwih_ : 1;
        osal_u32 xCt2OtvwCmOwm2ZOCxdOvo_ : 1;
        osal_u32 xCt2OtvwCmOwm2iOCxdOvo_ : 1;
        osal_u32 x6NgwNi_6Ww_WgDw6oKwih_ : 1;
        osal_u32 xCt2OtvwCmOwm2aOCxdOvo_ : 1;
        osal_u32 x6NgwNi_6Ww_WgXw6oKwih_ : 1;
        osal_u32 xCt2OtvwCmOwm29OCxdOvo_ : 1;
        osal_u32 reserved_0 : 24;
    } bits;

    osal_u32 u32;
} u_reg_bank_clk_force_on;

#endif
