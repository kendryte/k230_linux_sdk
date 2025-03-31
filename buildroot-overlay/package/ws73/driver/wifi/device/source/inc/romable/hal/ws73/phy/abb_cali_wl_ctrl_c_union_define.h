/*
 * Copyright :Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * File name : abb_cali_wl_ctrl_c_union_define.h
 * Project line : * Department : * Author : xxx
 * Version : 1.0
 * Date : * Description : 项目描述信息
 * Others : Generated automatically by nManager V5.1
 * History : xxx 2022/07/02 14:12:02 Create file
 */
#ifndef __ABB_CALI_WL_CTRL_C_UNION_DEFINE_H__
#define __ABB_CALI_WL_CTRL_C_UNION_DEFINE_H__

typedef union {
    struct {
        osal_u32 x_CLwCiNLwhwBw6y_ : 1;
        osal_u32 x_CLwCiNLwhwRw6y_ : 1;
        osal_u32 xwsROsvtROoOlOCr_ : 1;
        osal_u32 xwsROsvtROoO6OCr_ : 1;
        osal_u32 xCiNLw_CLwGF6w6GoBw6ywh_ : 1;
        osal_u32 xsvtROwsROz4COCzxlOCrOo_ : 1;
        osal_u32 xCiNLw_CLwFG6w6GoBw6ywh_ : 1;
        osal_u32 xsvtROwsRO4zCOCzxlOCrOo_ : 1;
        osal_u32 xCiNLw_CLwCGSuoW_ : 1;
        osal_u32 xwsROz4CO4G4OsvtROCrOo_ : 1;
        osal_u32 x_CLwGF6wFuFwCiNLw6ywh_ : 1;
        osal_u32 xCiNLw_CLwFuFwGF6w06Qhwh_ : 1;
        osal_u32 xsvtROwsRO4G4Oz4CO3CAoOo_ : 1;
        osal_u32 xCiNLw_CLwSihBLi_wFG6wgWhwh_ : 1;
        osal_u32 xCiNLw_CLwSihBLi_wGF6wgWhwh_ : 1;
        osal_u32 xCiNLw_CLwSihBLi_wFG6wCGSuoWwh_ : 1;
        osal_u32 xCiNLw_CLwSihBLi_wGF6wCGSuoWwh_ : 1;
        osal_u32 xCiNLw_CLwSihBLi_wh_ : 1;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_rst_soft_n;

typedef union {
    struct {
        osal_u32 xFG6wy6Qw6oKWh_ : 1;
        osal_u32 x4zCOrCAOCxdmo_ : 1;
        osal_u32 xGF6wy6Qw6oKWh_ : 1;
        osal_u32 xz4COrCAOCxdmo_ : 1;
        osal_u32 xz4COsSy3OC3OCxdmo_ : 1;
        osal_u32 xGF6wCdj0w6yw6oKWh_ : 1;
        osal_u32 xz4COsSyZOC3OCxdmo_ : 1;
        osal_u32 xz4COsSyiOC3OCxdmo_ : 1;
        osal_u32 xGF6wCdjyw60w6oKWh_ : 1;
        osal_u32 xz4COsSyrOCrOCxdmo_ : 1;
        osal_u32 xGF6wCdjVw60w6oKWh_ : 1;
        osal_u32 xGF6wCdjkw60w6oKWh_ : 1;
        osal_u32 xTuywGF6wy6Qw6oKWh_ : 1;
        osal_u32 xqG3Oz4COrCAOCxdmo_ : 1;
        osal_u32 xqGrOz4CO3CAOCxdmo_ : 1;
        osal_u32 xqGrO4zCO3CAOCxdmo_ : 1;
        osal_u32 xTuywFG6wy6Qw6oKWh_ : 1;
        osal_u32 xqG3O4zCOrCAOCxdmo_ : 1;
        osal_u32 reserved_0 : 14;
    } bits;

    osal_u32 u32;
} u_soft_clken_0;

typedef union {
    struct {
        osal_u32 xGF6w6Qyw6oKWh_ : 1;
        osal_u32 xz4COCArOCxdmo_ : 1;
        osal_u32 xGF6wFuFw6Qyw6oKWh_ : 1;
        osal_u32 xz4CO4G4OCArOCxdmo_ : 1;
        osal_u32 xCIGOCzxlOCxdmo_ : 1;
        osal_u32 xCzGOCzxlOCxdmo_ : 1;
        osal_u32 xSiW6w6GoBw6oKWh_ : 1;
        osal_u32 xSgW6w6GoBw6oKWh_ : 1;
        osal_u32 xGF6w6oKw6Qyw6oKWh_ : 1;
        osal_u32 xz4COCxdOCArOCxdmo_ : 1;
        osal_u32 reserved_0 : 22;
    } bits;

    osal_u32 u32;
} u_soft_clken_1;

typedef union {
    struct {
        osal_u32 xFG6wy6QwuQwCWo_ : 1;
        osal_u32 x4zCOrCAOGAOsmx_ : 1;
        osal_u32 xz4COsSy3OC3OGAOsmx_ : 1;
        osal_u32 xGF6wCdj0w6ywuQwCWo_ : 1;
        osal_u32 xz4COsSyZOC3OGAOsmx_ : 1;
        osal_u32 xz4COsSyiOC3OGAOsmx_ : 1;
        osal_u32 xGF6wCdjyw60wuQwCWo_ : 1;
        osal_u32 xz4COsSyrOCrOGAOsmx_ : 1;
        osal_u32 xGF6wCdjVw60wuQwCWo_ : 1;
        osal_u32 xGF6wCdjkw60wuQwCWo_ : 1;
        osal_u32 xTuwFG6wgLwWhwy6QwCWo_ : 1;
        osal_u32 xqGO4zCO2ROmoOrCAOsmx_ : 1;
        osal_u32 xTuwGF6wgLwWhwy6QwCWo_ : 1;
        osal_u32 xqGOz4CO2ROmoOrCAOsmx_ : 1;
        osal_u32 x2RO4zCOCxdOtvwO4G4Osmx_ : 1;
        osal_u32 reserved_0 : 17;
    } bits;

    osal_u32 u32;
} u_clk_phase_sel_0;

typedef union {
    struct {
        osal_u32 xTuywFG6wy6Qw6oKCWo_ : 1;
        osal_u32 xqG3O4zCOrCAOCxdsmx_ : 1;
        osal_u32 xqGrO4zCO3CAOCxdsmx_ : 1;
        osal_u32 xTuywGF6wy6Qw6oKCWo_ : 1;
        osal_u32 xqG3Oz4COrCAOCxdsmx_ : 1;
        osal_u32 xqGrOz4CO3CAOCxdsmx_ : 1;
        osal_u32 xGjjw6L_ow6oKw6QhwCWo_ : 1;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_clk_phase_sel_1;

typedef union {
    struct {
        osal_u32 xGF6wTi_KwSiFWw6QywSGh_ : 2;
        osal_u32 xz4COqvwdOIv4mOCA3Osmx_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 xz4COqvwdOIv4mOCArOIzo_ : 2;
        osal_u32 xGF6wTi_KwSiFWw6Q0wCWo_ : 1;
        osal_u32 reserved_1 : 1;
        osal_u32 xqG3OCt2Oz4COtsOIzo_ : 2;
        osal_u32 xTuyw6NgwGF6wNCwCWo_ : 1;
        osal_u32 reserved_2 : 1;
        osal_u32 xTu0w6NgwGF6wNCwSGh_ : 2;
        osal_u32 xqGrOCt2Oz4COtsOsmx_ : 1;
        osal_u32 x6NgwCGSuoWwTi_KwSiFW_ : 2;
        osal_u32 reserved_3 : 15;
    } bits;

    osal_u32 u32;
} u_adc_work_mode_ctrl;

typedef union {
    struct {
        osal_u32 xGF6w6QywFBqwhdS_ : 2;
        osal_u32 xz4COCArO4lPOoSI_ : 2;
        osal_u32 xGF6wFuFw6QywFBqwhdS_ : 2;
        osal_u32 xz4CO4G4OCArO4lPOoSI_ : 2;
        osal_u32 xqxGAQO4lPOmo_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xGF6w6oKw6QywFBqwhdS_ : 2;
        osal_u32 xz4COCxdOCArO4lPOoSI_ : 2;
        osal_u32 reserved_1 : 16;
    } bits;

    osal_u32 u32;
} u_clk_div0;

typedef union {
    struct {
        osal_u32 xFG6wFWSwWhwSGhw6Qy_ : 1;
        osal_u32 x4zCO4mIOmoOsmxOCA3_ : 1;
        osal_u32 x4zCO4mIOmoOIzoOCAr_ : 1;
        osal_u32 xFG6wFWSwWhwCWow6Q0_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_dac_dem_en_man;

typedef union {
    struct {
        osal_u32 xFWSwQCj_ : 1;
        osal_u32 xFWSwSCj_ : 1;
        osal_u32 reserved_0 : 2;
        osal_u32 xFWSwCWo_ : 1;
        osal_u32 xu_gwWh_ : 1;
        osal_u32 reserved_1 : 2;
        osal_u32 x6oGwCLWuwQCj_ : 2;
        osal_u32 reserved_2 : 2;
        osal_u32 x6oGwCLWuwSCj_ : 2;
        osal_u32 reserved_3 : 18;
    } bits;

    osal_u32 u32;
} u_dac_dem_ctl;

typedef union {
    struct {
        osal_u32 xvsOCzxOIv4m_ : 1;
        osal_u32 xiCw6GowoiiuwCWo_ : 1;
        osal_u32 xiCw6GowBhqW_CW_ : 1;
        osal_u32 xvsOCzxOzCCOCQCxmOsmx_ : 2;
        osal_u32 xvsOCzxOwzo2mOsmx_ : 2;
        osal_u32 xiCw6GowFGLGwCuB_ : 6;
        osal_u32 reserved_0 : 3;
        osal_u32 xvsOCzxOpmwvOoSI_ : 12;
        osal_u32 xvsOCzxOz4CO4zRzOloP_ : 1;
        osal_u32 reserved_1 : 3;
    } bits;

    osal_u32 u32;
} u_os_cal_cfg;

typedef union {
    struct {
        osal_u32 xiCw6GowWhwywBw6y_ : 1;
        osal_u32 xvsOCzxOmoOrOlOC3_ : 1;
        osal_u32 xiCw6GowWhwVwBw6y_ : 1;
        osal_u32 xiCw6GowWhwkwBw6y_ : 1;
        osal_u32 xiCw6GowWhwywRw6y_ : 1;
        osal_u32 xvsOCzxOmoOrO6OC3_ : 1;
        osal_u32 xiCw6GowWhwVwRw6y_ : 1;
        osal_u32 xiCw6GowWhwkwRw6y_ : 1;
        osal_u32 xvsOCzxOmoO3OlOCr_ : 1;
        osal_u32 xiCw6GowWhw0wBw60_ : 1;
        osal_u32 xvsOCzxOmoOZOlOCr_ : 1;
        osal_u32 xvsOCzxOmoOiOlOCr_ : 1;
        osal_u32 xvsOCzxOmoO3O6OCr_ : 1;
        osal_u32 xiCw6GowWhw0wRw60_ : 1;
        osal_u32 xvsOCzxOmoOZO6OCr_ : 1;
        osal_u32 xvsOCzxOmoOiO6OCr_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_os_cal_en;

typedef union {
    struct {
        osal_u32 xvsO4zRzOmoO3OlOC3_ : 1;
        osal_u32 xiCwFGLGwWhw0wBw6y_ : 1;
        osal_u32 xvsO4zRzOmoOZOlOC3_ : 1;
        osal_u32 xvsO4zRzOmoOiOlOC3_ : 1;
        osal_u32 xvsO4zRzOmoO3O6OC3_ : 1;
        osal_u32 xiCwFGLGwWhw0wRw6y_ : 1;
        osal_u32 xvsO4zRzOmoOZO6OC3_ : 1;
        osal_u32 xvsO4zRzOmoOiO6OC3_ : 1;
        osal_u32 xiCwFGLGwWhwywBw60_ : 1;
        osal_u32 xvsO4zRzOmoOrOlOCr_ : 1;
        osal_u32 xiCwFGLGwWhwVwBw60_ : 1;
        osal_u32 xiCwFGLGwWhwkwBw60_ : 1;
        osal_u32 xiCwFGLGwWhwywRw60_ : 1;
        osal_u32 xvsO4zRzOmoOrO6OCr_ : 1;
        osal_u32 xiCwFGLGwWhwVwRw60_ : 1;
        osal_u32 xiCwFGLGwWhwkwRw60_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_os_cal_data_en;

typedef union {
    struct {
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQwy_ : 1;
        osal_u32 xvsOCzxO4zRzOsGlOwmtwmsAOr_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQwV_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQwk_ : 1;
        osal_u32 xvsOCzxO4zRzOsGlOwmtwmsAOK_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQwx_ : 1;
        osal_u32 xvsOCzxO4zRzOsGlOwmtwmsAOY_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQwp_ : 1;
        osal_u32 xvsOCzxO4zRzOsGlOwmtwmsAOn_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQwc_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQw0y_ : 1;
        osal_u32 xvsOCzxO4zRzOsGlOwmtwmsAOrr_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQw0V_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQw0k_ : 1;
        osal_u32 xvsOCzxO4zRzOsGlOwmtwmsAOrK_ : 1;
        osal_u32 xiCw6GowFGLGwCuBw_WN_WCQw0x_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_os_cal_data_spi_refresh;

typedef union {
    struct {
        osal_u32 xvsOCzxOtlolsAOCxwO3OlOC3_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_w0wBw6y_ : 1;
        osal_u32 xvsOCzxOtlolsAOCxwOZOlOC3_ : 1;
        osal_u32 xvsOCzxOtlolsAOCxwOiOlOC3_ : 1;
        osal_u32 xvsOCzxOtlolsAOCxwO3O6OC3_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_w0wRw6y_ : 1;
        osal_u32 xvsOCzxOtlolsAOCxwOZO6OC3_ : 1;
        osal_u32 xvsOCzxOtlolsAOCxwOiO6OC3_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_wywBw60_ : 1;
        osal_u32 xvsOCzxOtlolsAOCxwOrOlOCr_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_wVwBw60_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_wkwBw60_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_wywRw60_ : 1;
        osal_u32 xvsOCzxOtlolsAOCxwOrO6OCr_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_wVwRw60_ : 1;
        osal_u32 xiCw6GowNBhBCQw6o_wkwRw60_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_os_cal_state_clr;

typedef union {
    struct {
        osal_u32 xvsOCzxOtlolsAO3_ : 1;
        osal_u32 xiCw6GowNBhBCQw0_ : 1;
        osal_u32 xvsOCzxOtlolsAOZ_ : 1;
        osal_u32 xvsOCzxOtlolsAOi_ : 1;
        osal_u32 xiCw6GowNBhBCQwD_ : 1;
        osal_u32 xvsOCzxOtlolsAOa_ : 1;
        osal_u32 xiCw6GowNBhBCQwX_ : 1;
        osal_u32 xvsOCzxOtlolsAO9_ : 1;
        osal_u32 xiCw6GowNBhBCQwU_ : 1;
        osal_u32 xvsOCzxOtlolsAOc_ : 1;
        osal_u32 xvsOCzxOtlolsAOr3_ : 1;
        osal_u32 xiCw6GowNBhBCQw00_ : 1;
        osal_u32 xvsOCzxOtlolsAOrZ_ : 1;
        osal_u32 xvsOCzxOtlolsAOri_ : 1;
        osal_u32 xiCw6GowNBhBCQw0D_ : 1;
        osal_u32 xvsOCzxOtlolsAOra_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_os_cal_finish_sts;

typedef union {
    struct {
        osal_u32 xvsOCzxOsRzRmO3_ : 1;
        osal_u32 xiCw6GowCLGLWw0_ : 1;
        osal_u32 xvsOCzxOsRzRmOZ_ : 1;
        osal_u32 xvsOCzxOsRzRmOi_ : 1;
        osal_u32 xiCw6GowCLGLWwD_ : 1;
        osal_u32 xvsOCzxOsRzRmOa_ : 1;
        osal_u32 xiCw6GowCLGLWwX_ : 1;
        osal_u32 xvsOCzxOsRzRmO9_ : 1;
        osal_u32 xiCw6GowCLGLWwU_ : 1;
        osal_u32 xvsOCzxOsRzRmOc_ : 1;
        osal_u32 xvsOCzxOsRzRmOr3_ : 1;
        osal_u32 xiCw6GowCLGLWw00_ : 1;
        osal_u32 xvsOCzxOsRzRmOrZ_ : 1;
        osal_u32 xvsOCzxOsRzRmOri_ : 1;
        osal_u32 xiCw6GowCLGLWw0D_ : 1;
        osal_u32 xvsOCzxOsRzRmOra_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_os_cal_state_sts;

typedef union {
    struct {
        osal_u32 xiCw6Gow6d__wCL_ : 3;
        osal_u32 reserved_0 : 29;
    } bits;

    osal_u32 u32;
} u_os_cal_curr_st;

typedef union {
    struct {
        osal_u32 x6Guw6GowSiFW_ : 1;
        osal_u32 xCzGOCzxOzx2Osmx_ : 1;
        osal_u32 x6Guw6GowG66w6t6oWwCWo_ : 2;
        osal_u32 xCzGOCzxOsRzwROlo4m8_ : 1;
        osal_u32 x6GuwTWBgQLwCWo_ : 1;
        osal_u32 x6Guw6GowFG6wCWLLBhgwFotw6L_o_ : 1;
        osal_u32 reserved_0 : 1;
        osal_u32 x6Guw6Gow6GuwCWowSGh_ : 3;
        osal_u32 xCzGOCzxOsAOmoOIzo_ : 1;
        osal_u32 xCzGOCzxOGoOsmxOIzo_ : 1;
        osal_u32 reserved_1 : 19;
    } bits;

    osal_u32 u32;
} u_cap_cal_cfg;

typedef union {
    struct {
        osal_u32 xCzGOCzxOmoO3OlOC3_ : 1;
        osal_u32 x6Guw6GowWhw0wBw6y_ : 1;
        osal_u32 xCzGOCzxOmoOZOlOC3_ : 1;
        osal_u32 xCzGOCzxOmoOiOlOC3_ : 1;
        osal_u32 xCzGOCzxOmoO3O6OC3_ : 1;
        osal_u32 x6Guw6GowWhw0wRw6y_ : 1;
        osal_u32 xCzGOCzxOmoOZO6OC3_ : 1;
        osal_u32 xCzGOCzxOmoOiO6OC3_ : 1;
        osal_u32 x6Guw6GowWhwywBw60_ : 1;
        osal_u32 xCzGOCzxOmoOrOlOCr_ : 1;
        osal_u32 x6Guw6GowWhwVwBw60_ : 1;
        osal_u32 x6Guw6GowWhwkwBw60_ : 1;
        osal_u32 x6Guw6GowWhwywRw60_ : 1;
        osal_u32 xCzGOCzxOmoOrO6OCr_ : 1;
        osal_u32 x6Guw6GowWhwVwRw60_ : 1;
        osal_u32 x6Guw6GowWhwkwRw60_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cap_cal_en;

typedef union {
    struct {
        osal_u32 xgGBhw6GowWhwywBw6y_ : 1;
        osal_u32 x2zloOCzxOmoOrOlOC3_ : 1;
        osal_u32 xgGBhw6GowWhwVwBw6y_ : 1;
        osal_u32 xgGBhw6GowWhwkwBw6y_ : 1;
        osal_u32 xgGBhw6GowWhwywRw6y_ : 1;
        osal_u32 x2zloOCzxOmoOrO6OC3_ : 1;
        osal_u32 xgGBhw6GowWhwVwRw6y_ : 1;
        osal_u32 xgGBhw6GowWhwkwRw6y_ : 1;
        osal_u32 x2zloOCzxOmoO3OlOCr_ : 1;
        osal_u32 xgGBhw6GowWhw0wBw60_ : 1;
        osal_u32 x2zloOCzxOmoOZOlOCr_ : 1;
        osal_u32 x2zloOCzxOmoOiOlOCr_ : 1;
        osal_u32 x2zloOCzxOmoO3O6OCr_ : 1;
        osal_u32 xgGBhw6GowWhw0wRw60_ : 1;
        osal_u32 x2zloOCzxOmoOZO6OCr_ : 1;
        osal_u32 x2zloOCzxOmoOiO6OCr_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_gain_cal_en;

typedef union {
    struct {
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAO3_ : 1;
        osal_u32 x6Guw6GowFGLGwCuBw_WN_WCQw0_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOZ_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOi_ : 1;
        osal_u32 x6Guw6GowFGLGwCuBw_WN_WCQwD_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOa_ : 1;
        osal_u32 x6Guw6GowFGLGwCuBw_WN_WCQwX_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAO9_ : 1;
        osal_u32 x6Guw6GowFGLGwCuBw_WN_WCQwU_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOc_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOr3_ : 1;
        osal_u32 x6Guw6GowFGLGwCuBw_WN_WCQw00_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOrZ_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOri_ : 1;
        osal_u32 x6Guw6GowFGLGwCuBw_WN_WCQw0D_ : 1;
        osal_u32 xCzGOCzxO4zRzOsGlOwmtwmsAOra_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cap_cal_data_spi_refresh;

typedef union {
    struct {
        osal_u32 x6Guw6GowNBhBCQw6o_wywBw6y_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwOrOlOC3_ : 1;
        osal_u32 x6Guw6GowNBhBCQw6o_wVwBw6y_ : 1;
        osal_u32 x6Guw6GowNBhBCQw6o_wkwBw6y_ : 1;
        osal_u32 x6Guw6GowNBhBCQw6o_wywRw6y_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwOrO6OC3_ : 1;
        osal_u32 x6Guw6GowNBhBCQw6o_wVwRw6y_ : 1;
        osal_u32 x6Guw6GowNBhBCQw6o_wkwRw6y_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwO3OlOCr_ : 1;
        osal_u32 x6Guw6GowNBhBCQw6o_w0wBw60_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwOZOlOCr_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwOiOlOCr_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwO3O6OCr_ : 1;
        osal_u32 x6Guw6GowNBhBCQw6o_w0wRw60_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwOZO6OCr_ : 1;
        osal_u32 xCzGOCzxOtlolsAOCxwOiO6OCr_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_cap_cal_finish_clr;

typedef union {
    struct {
        osal_u32 x6Guw6GowNBhBCQwy_ : 1;
        osal_u32 xCzGOCzxOtlolsAOr_ : 1;
        osal_u32 x6Guw6GowNBhBCQwV_ : 1;
        osal_u32 x6Guw6GowNBhBCQwk_ : 1;
        osal_u32 xCzGOCzxOtlolsAOK_ : 1;
        osal_u32 x6Guw6GowNBhBCQwx_ : 1;
        osal_u32 xCzGOCzxOtlolsAOY_ : 1;
        osal_u32 x6Guw6GowNBhBCQwp_ : 1;
        osal_u32 xCzGOCzxOtlolsAOn_ : 1;
        osal_u32 x6Guw6GowNBhBCQwc_ : 1;
        osal_u32 x6Guw6GowNBhBCQw0y_ : 1;
        osal_u32 xCzGOCzxOtlolsAOrr_ : 1;
        osal_u32 x6Guw6GowNBhBCQw0V_ : 1;
        osal_u32 x6Guw6GowNBhBCQw0k_ : 1;
        osal_u32 xCzGOCzxOtlolsAOrK_ : 1;
        osal_u32 x6Guw6GowNBhBCQw0x_ : 1;
        osal_u32 x6Guw6GowW__i_wy_ : 1;
        osal_u32 xCzGOCzxOmwwvwOr_ : 1;
        osal_u32 x6Guw6GowW__i_wV_ : 1;
        osal_u32 x6Guw6GowW__i_wk_ : 1;
        osal_u32 xCzGOCzxOmwwvwOK_ : 1;
        osal_u32 x6Guw6GowW__i_wx_ : 1;
        osal_u32 xCzGOCzxOmwwvwOY_ : 1;
        osal_u32 x6Guw6GowW__i_wp_ : 1;
        osal_u32 xCzGOCzxOmwwvwOn_ : 1;
        osal_u32 x6Guw6GowW__i_wc_ : 1;
        osal_u32 x6Guw6GowW__i_w0y_ : 1;
        osal_u32 xCzGOCzxOmwwvwOrr_ : 1;
        osal_u32 x6Guw6GowW__i_w0V_ : 1;
        osal_u32 x6Guw6GowW__i_w0k_ : 1;
        osal_u32 xCzGOCzxOmwwvwOrK_ : 1;
        osal_u32 x6Guw6GowW__i_w0x_ : 1;
    } bits;

    osal_u32 u32;
} u_cap_cal_sts0;

typedef union {
    struct {
        osal_u32 xvPmwOmwwvwO3_ : 1;
        osal_u32 xiqW_wW__i_w0_ : 1;
        osal_u32 xvPmwOmwwvwOZ_ : 1;
        osal_u32 xvPmwOmwwvwOi_ : 1;
        osal_u32 xiqW_wW__i_wD_ : 1;
        osal_u32 xvPmwOmwwvwOa_ : 1;
        osal_u32 xiqW_wW__i_wX_ : 1;
        osal_u32 xvPmwOmwwvwO9_ : 1;
        osal_u32 xiqW_wW__i_wU_ : 1;
        osal_u32 xvPmwOmwwvwOc_ : 1;
        osal_u32 xvPmwOmwwvwOr3_ : 1;
        osal_u32 xiqW_wW__i_w00_ : 1;
        osal_u32 xvPmwOmwwvwOrZ_ : 1;
        osal_u32 xvPmwOmwwvwOri_ : 1;
        osal_u32 xiqW_wW__i_w0D_ : 1;
        osal_u32 xvPmwOmwwvwOra_ : 1;
        osal_u32 xiqW_wW__i_wgGBhwy_ : 1;
        osal_u32 xvPmwOmwwvwO2zloOr_ : 1;
        osal_u32 xiqW_wW__i_wgGBhwV_ : 1;
        osal_u32 xiqW_wW__i_wgGBhwk_ : 1;
        osal_u32 xvPmwOmwwvwO2zloOK_ : 1;
        osal_u32 xiqW_wW__i_wgGBhwx_ : 1;
        osal_u32 xvPmwOmwwvwO2zloOY_ : 1;
        osal_u32 xiqW_wW__i_wgGBhwp_ : 1;
        osal_u32 xvPmwOmwwvwO2zloOn_ : 1;
        osal_u32 xiqW_wW__i_wgGBhwc_ : 1;
        osal_u32 xiqW_wW__i_wgGBhw0y_ : 1;
        osal_u32 xvPmwOmwwvwO2zloOrr_ : 1;
        osal_u32 xiqW_wW__i_wgGBhw0V_ : 1;
        osal_u32 xiqW_wW__i_wgGBhw0k_ : 1;
        osal_u32 xvPmwOmwwvwO2zloOrK_ : 1;
        osal_u32 xiqW_wW__i_wgGBhw0x_ : 1;
    } bits;

    osal_u32 u32;
} u_cap_cal_sts1;

typedef union {
    struct {
        osal_u32 xCzGOCzxOqROCr_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt1;

typedef union {
    struct {
        osal_u32 x6Guw6GowTLw6V_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt2;

typedef union {
    struct {
        osal_u32 x6Guw6GowTLw6k_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt3;

typedef union {
    struct {
        osal_u32 xCzGOCzxOqROCK_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt4;

typedef union {
    struct {
        osal_u32 x6Guw6GowTLw6x_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt5;

typedef union {
    struct {
        osal_u32 xCzGOCzxOCSwwOsR_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_cap_cal_curr_st;

typedef union {
    struct {
        osal_u32 x6Guw6GowTLw60wCuB_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt_c1_spi;

typedef union {
    struct {
        osal_u32 xCzGOCzxOqROCZOsGl_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt_c2_spi;

typedef union {
    struct {
        osal_u32 xCzGOCzxOqROCiOsGl_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt_c3_spi;

typedef union {
    struct {
        osal_u32 x6Guw6GowTLw6DwCuB_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt_c4_spi;

typedef union {
    struct {
        osal_u32 xCzGOCzxOqROCaOsGl_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_cap_cal_wt_c5_spi;

typedef union {
    struct {
        osal_u32 gain : 19;
        osal_u32 reserved_0 : 1;
        osal_u32 xgGBhwdhBL_ : 12;
    } bits;

    osal_u32 u32;
} u_intr_gain_cal_gain;

typedef union {
    struct {
        osal_u32 xSwiW6wWhwywBw6y_ : 1;
        osal_u32 xIOvmCOmoOrOlOC3_ : 1;
        osal_u32 xSwiW6wWhwVwBw6y_ : 1;
        osal_u32 xSwiW6wWhwkwBw6y_ : 1;
        osal_u32 xSwiW6wWhwywRw6y_ : 1;
        osal_u32 xIOvmCOmoOrO6OC3_ : 1;
        osal_u32 xSwiW6wWhwVwRw6y_ : 1;
        osal_u32 xSwiW6wWhwkwRw6y_ : 1;
        osal_u32 xIOvmCOmoO3OlOCr_ : 1;
        osal_u32 xSwiW6wWhw0wBw60_ : 1;
        osal_u32 xIOvmCOmoOZOlOCr_ : 1;
        osal_u32 xIOvmCOmoOiOlOCr_ : 1;
        osal_u32 xIOvmCOmoO3O6OCr_ : 1;
        osal_u32 xSwiW6wWhw0wRw60_ : 1;
        osal_u32 xIOvmCOmoOZO6OCr_ : 1;
        osal_u32 xIOvmCOmoOiO6OCr_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_moec_en;

typedef union {
    struct {
        osal_u32 x6NgwjgwGjjw6GoBwWh_ : 1;
        osal_u32 reserved_0 : 31;
    } bits;

    osal_u32 u32;
} u_moec_bg_cali_en;

typedef union {
    struct {
        osal_u32 xSwiW6wiNNw6Ngw_WN_WCQwBw6y_ : 1;
        osal_u32 xSwiW6wiNNw6Ngw_WN_WCQwRw6y_ : 1;
        osal_u32 xIOvmCOvttOCt2OwmtwmsAOlOCr_ : 1;
        osal_u32 xIOvmCOvttOCt2OwmtwmsAO6OCr_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_moec_off_cfg_refresh;

typedef union {
    struct {
        osal_u32 xIOvmCOtlolsAOCxwO3OlOC3_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_w0wBw6y_ : 1;
        osal_u32 xIOvmCOtlolsAOCxwOZOlOC3_ : 1;
        osal_u32 xIOvmCOtlolsAOCxwOiOlOC3_ : 1;
        osal_u32 xIOvmCOtlolsAOCxwO3O6OC3_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_w0wRw6y_ : 1;
        osal_u32 xIOvmCOtlolsAOCxwOZO6OC3_ : 1;
        osal_u32 xIOvmCOtlolsAOCxwOiO6OC3_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_wywBw60_ : 1;
        osal_u32 xIOvmCOtlolsAOCxwOrOlOCr_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_wVwBw60_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_wkwBw60_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_wywRw60_ : 1;
        osal_u32 xIOvmCOtlolsAOCxwOrO6OCr_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_wVwRw60_ : 1;
        osal_u32 xSwiW6wNBhBCQw6o_wkwRw60_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_moec_state_clr;

typedef union {
    struct {
        osal_u32 xIOvmCO4vomO3_ : 1;
        osal_u32 xSwiW6wFihWw0_ : 1;
        osal_u32 xIOvmCO4vomOZ_ : 1;
        osal_u32 xIOvmCO4vomOi_ : 1;
        osal_u32 xSwiW6wFihWwD_ : 1;
        osal_u32 xIOvmCO4vomOa_ : 1;
        osal_u32 xSwiW6wFihWwX_ : 1;
        osal_u32 xIOvmCO4vomO9_ : 1;
        osal_u32 xSwiW6wFihWwU_ : 1;
        osal_u32 xIOvmCO4vomOc_ : 1;
        osal_u32 xIOvmCO4vomOr3_ : 1;
        osal_u32 xSwiW6wFihWw00_ : 1;
        osal_u32 xIOvmCO4vomOrZ_ : 1;
        osal_u32 xIOvmCO4vomOri_ : 1;
        osal_u32 xSwiW6wFihWw0D_ : 1;
        osal_u32 xIOvmCO4vomOra_ : 1;
        osal_u32 xSwiW6wiq_FwW__wy_ : 1;
        osal_u32 xIOvmCOvPw4OmwwOr_ : 1;
        osal_u32 xSwiW6wiq_FwW__wV_ : 1;
        osal_u32 xSwiW6wiq_FwW__wk_ : 1;
        osal_u32 xIOvmCOvPw4OmwwOK_ : 1;
        osal_u32 xSwiW6wiq_FwW__wx_ : 1;
        osal_u32 xIOvmCOvPw4OmwwOY_ : 1;
        osal_u32 xSwiW6wiq_FwW__wp_ : 1;
        osal_u32 xIOvmCOvPw4OmwwOn_ : 1;
        osal_u32 xSwiW6wiq_FwW__wc_ : 1;
        osal_u32 xSwiW6wiq_FwW__w0y_ : 1;
        osal_u32 xIOvmCOvPw4OmwwOrr_ : 1;
        osal_u32 xSwiW6wiq_FwW__w0V_ : 1;
        osal_u32 xSwiW6wiq_FwW__w0k_ : 1;
        osal_u32 xIOvmCOvPw4OmwwOrK_ : 1;
        osal_u32 xSwiW6wiq_FwW__w0x_ : 1;
    } bits;

    osal_u32 u32;
} u_moec_sts;

typedef union {
    struct {
        osal_u32 xSwiW6wiNNw6Ng_ : 17;
        osal_u32 reserved_0 : 3;
        osal_u32 xIOvmCOvttOCt2OIv4m_ : 1;
        osal_u32 reserved_1 : 3;
        osal_u32 xIOvmCOIz8Ow4smx_ : 2;
        osal_u32 xSwiW6wGouQGw6iSu_ : 2;
        osal_u32 xIOvmCOzCCOPvxsmx_ : 3;
        osal_u32 reserved_2 : 1;
    } bits;

    osal_u32 u32;
} u_moec_off_cfg;

typedef union {
    struct {
        osal_u32 xIOvmCOvsOCzxOt2_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_moec_report0;

typedef union {
    struct {
        osal_u32 xSwiW6wiCw6Gowjg_ : 17;
        osal_u32 reserved_0 : 15;
    } bits;

    osal_u32 u32;
} u_moec_report1;

typedef union {
    struct {
        osal_u32 xSiW6w6d__wCL_ : 4;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_moec_curr_st;

typedef union {
    struct {
        osal_u32 xIO2mCOmoO3_ : 1;
        osal_u32 xSwgW6wWhw0_ : 1;
        osal_u32 xIO2mCOmoOZ_ : 1;
        osal_u32 xIO2mCOmoOi_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_mgec_gain_cfg_mode;

typedef union {
    struct {
        osal_u32 xIO2mCO2zloOCt2OwmtwmsAOlOC3_ : 1;
        osal_u32 xIO2mCO2zloOCt2OwmtwmsAO6OC3_ : 1;
        osal_u32 xSwgW6wgGBhw6Ngw_WN_WCQwBw60_ : 1;
        osal_u32 xSwgW6wgGBhw6Ngw_WN_WCQwRw60_ : 1;
        osal_u32 reserved_0 : 4;
        osal_u32 xI2mCOCvmtOGwR_ : 1;
        osal_u32 xI2mCOzyowIOGwR_ : 1;
        osal_u32 reserved_1 : 2;
        osal_u32 xSwgW6wNBhBCQw6o_wBw6y_ : 1;
        osal_u32 xSwgW6wNBhBCQw6o_wRw6y_ : 1;
        osal_u32 xIO2mCOtlolsAOCxwOlOCr_ : 1;
        osal_u32 xIO2mCOtlolsAOCxwO6OCr_ : 1;
        osal_u32 xIO2mCO4vomO3_ : 1;
        osal_u32 xSwgW6wFihWw0_ : 1;
        osal_u32 xIO2mCO4vomOZ_ : 1;
        osal_u32 xIO2mCO4vomOi_ : 1;
        osal_u32 xSwgW6wW__0wy_ : 1;
        osal_u32 xIO2mCOmwwrOr_ : 1;
        osal_u32 xSwgW6wW__0wV_ : 1;
        osal_u32 xSwgW6wW__0wk_ : 1;
        osal_u32 xIO2mCOmwwZO3_ : 1;
        osal_u32 xSwgW6wW__Vw0_ : 1;
        osal_u32 xIO2mCOmwwZOZ_ : 1;
        osal_u32 xIO2mCOmwwZOi_ : 1;
        osal_u32 xIO2mCOmwwiO3_ : 1;
        osal_u32 xSwgW6wW__kw0_ : 1;
        osal_u32 xIO2mCOmwwiOZ_ : 1;
        osal_u32 xIO2mCOmwwiOi_ : 1;
    } bits;

    osal_u32 u32;
} u_mgec_gain_misc;

typedef union {
    struct {
        osal_u32 xIO2mCO2zloOCt2_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_mgec_gain_cfg;

typedef union {
    struct {
        osal_u32 xIO2mCOIz8Ow4smx_ : 2;
        osal_u32 xIO2mCOCvmt_ : 2;
        osal_u32 xIO2mCOzCCOPvxsmx_ : 3;
        osal_u32 reserved_0 : 1;
        osal_u32 xSwgW6wgGBhw6NgwSiFW_ : 1;
        osal_u32 reserved_1 : 23;
    } bits;

    osal_u32 u32;
} u_mgec_cal_cfg;

typedef union {
    struct {
        osal_u32 xIO2mCO2zloOCzxOr_ : 16;
        osal_u32 xSwgW6wgGBhw6GowV_ : 16;
    } bits;

    osal_u32 u32;
} u_mgec_gain_cal1_2;

typedef union {
    struct {
        osal_u32 xSwgW6wgGBhw6Gowk_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_mgec_gain_cal3;

typedef union {
    struct {
        osal_u32 xSgW6w6d__wCL_ : 5;
        osal_u32 reserved_0 : 27;
    } bits;

    osal_u32 u32;
} u_mgec_curr_st;

typedef union {
    struct {
        osal_u32 xz4CO2xlRCAORmsROmoOCA3Ol_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xz4CO2xlRCAORAwOCA3Ol_ : 9;
        osal_u32 reserved_1 : 3;
        osal_u32 xz4CO2xlRCAORmsROmoOCA3O6_ : 1;
        osal_u32 reserved_2 : 3;
        osal_u32 xz4CO2xlRCAORAwOCA3O6_ : 9;
        osal_u32 reserved_3 : 3;
    } bits;

    osal_u32 u32;
} u_glith_cfg_ch0;

typedef union {
    struct {
        osal_u32 xGF6wgoBL6QwhdSw6QywB_ : 12;
        osal_u32 xz4CO2xlRCAO4vomOCA3Ol_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xGF6wgoBL6QwhdSw6QywR_ : 12;
        osal_u32 xz4CO2xlRCAO4vomOCA3O6_ : 1;
        osal_u32 reserved_1 : 3;
    } bits;

    osal_u32 u32;
} u_glith_report_ch0;

typedef union {
    struct {
        osal_u32 xGF6wgoBL6QwLWCLwWhw6Q0wB_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xGF6wgoBL6QwLQ_w6Q0wB_ : 9;
        osal_u32 reserved_1 : 3;
        osal_u32 xGF6wgoBL6QwLWCLwWhw6Q0wR_ : 1;
        osal_u32 reserved_2 : 3;
        osal_u32 xGF6wgoBL6QwLQ_w6Q0wR_ : 9;
        osal_u32 reserved_3 : 3;
    } bits;

    osal_u32 u32;
} u_glith_cfg_ch1;

typedef union {
    struct {
        osal_u32 xz4CO2xlRCAOoSIOCArOl_ : 12;
        osal_u32 xGF6wgoBL6QwFihWw6Q0wB_ : 1;
        osal_u32 reserved_0 : 3;
        osal_u32 xz4CO2xlRCAOoSIOCArO6_ : 12;
        osal_u32 xGF6wgoBL6QwFihWw6Q0wR_ : 1;
        osal_u32 reserved_1 : 3;
    } bits;

    osal_u32 u32;
} u_glith_repor_ch1;

typedef union {
    struct {
        osal_u32 xzyyO4zCOl6Om8CAzo2mOCA3_ : 1;
        osal_u32 xzyyOz4COl6Om8CAzo2mOCA3_ : 1;
        osal_u32 xGjjwFG6wBRwWP6QGhgWw6Q0_ : 1;
        osal_u32 xGjjwGF6wBRwWP6QGhgWw6Q0_ : 1;
        osal_u32 reserved_0 : 28;
    } bits;

    osal_u32 u32;
} u_iq_exchange_en;

typedef union {
    struct {
        osal_u32 xqltlO4zRzOvSRGSROmo_ : 1;
        osal_u32 xFuFwFGLGwidLudLwWh_ : 1;
        osal_u32 xGjjwTowF6wBhudLwWh_ : 1;
        osal_u32 xGjjwTowCdj6oKw6L_owWh_ : 1;
        osal_u32 xGjjwTowCdj6oKw6L_owCT_ : 2;
        osal_u32 xCiNLwCdj6oKw6L_owqoF_ : 1;
        osal_u32 reserved_0 : 25;
    } bits;

    osal_u32 u32;
} u_work_mode;

typedef union {
    struct {
        osal_u32 xGF6wDLi0wNBNiwCLCw6o__ : 1;
        osal_u32 xGF6wFuFwDLi0wNBNiwCLCw6o__ : 1;
        osal_u32 reserved_0 : 30;
    } bits;

    osal_u32 u32;
} u_adc_fifo_sts_clr;

typedef union {
    struct {
        osal_u32 xz4C3OtltvOtSxxOCA3Ol_ : 1;
        osal_u32 xGF60wNBNiwNdoow6QywB_ : 1;
        osal_u32 xz4CZOtltvOtSxxOCA3Ol_ : 1;
        osal_u32 xz4CiOtltvOtSxxOCA3Ol_ : 1;
        osal_u32 xz4C3OtltvOtSxxOCA3O6_ : 1;
        osal_u32 xGF60wNBNiwNdoow6QywR_ : 1;
        osal_u32 xz4CZOtltvOtSxxOCA3O6_ : 1;
        osal_u32 xz4CiOtltvOtSxxOCA3O6_ : 1;
        osal_u32 xGF6ywNBNiwNdoow6Q0wB_ : 1;
        osal_u32 xz4CrOtltvOtSxxOCArOl_ : 1;
        osal_u32 xGF6VwNBNiwNdoow6Q0wB_ : 1;
        osal_u32 xGF6kwNBNiwNdoow6Q0wB_ : 1;
        osal_u32 xGF6ywNBNiwNdoow6Q0wR_ : 1;
        osal_u32 xz4CrOtltvOtSxxOCArO6_ : 1;
        osal_u32 xGF6VwNBNiwNdoow6Q0wR_ : 1;
        osal_u32 xGF6kwNBNiwNdoow6Q0wR_ : 1;
        osal_u32 xz4C3OtltvOtSxxOsRdOCA3Ol_ : 1;
        osal_u32 xGF60wNBNiwNdoowCLKw6QywB_ : 1;
        osal_u32 xz4CZOtltvOtSxxOsRdOCA3Ol_ : 1;
        osal_u32 xz4CiOtltvOtSxxOsRdOCA3Ol_ : 1;
        osal_u32 xz4C3OtltvOtSxxOsRdOCA3O6_ : 1;
        osal_u32 xGF60wNBNiwNdoowCLKw6QywR_ : 1;
        osal_u32 xz4CZOtltvOtSxxOsRdOCA3O6_ : 1;
        osal_u32 xz4CiOtltvOtSxxOsRdOCA3O6_ : 1;
        osal_u32 xGF6ywNBNiwNdoowCLKw6Q0wB_ : 1;
        osal_u32 xz4CrOtltvOtSxxOsRdOCArOl_ : 1;
        osal_u32 xGF6VwNBNiwNdoowCLKw6Q0wB_ : 1;
        osal_u32 xGF6kwNBNiwNdoowCLKw6Q0wB_ : 1;
        osal_u32 xGF6ywNBNiwNdoowCLKw6Q0wR_ : 1;
        osal_u32 xz4CrOtltvOtSxxOsRdOCArO6_ : 1;
        osal_u32 xGF6VwNBNiwNdoowCLKw6Q0wR_ : 1;
        osal_u32 xGF6kwNBNiwNdoowCLKw6Q0wR_ : 1;
    } bits;

    osal_u32 u32;
} u_adc_4to1_fifo_full_sts;

typedef union {
    struct {
        osal_u32 xz4C3OtltvOmIGRQOCA3Ol_ : 1;
        osal_u32 xGF60wNBNiwWSuLtw6QywB_ : 1;
        osal_u32 xz4CZOtltvOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4CiOtltvOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4C3OtltvOmIGRQOCA3O6_ : 1;
        osal_u32 xGF60wNBNiwWSuLtw6QywR_ : 1;
        osal_u32 xz4CZOtltvOmIGRQOCA3O6_ : 1;
        osal_u32 xz4CiOtltvOmIGRQOCA3O6_ : 1;
        osal_u32 xGF6ywNBNiwWSuLtw6Q0wB_ : 1;
        osal_u32 xz4CrOtltvOmIGRQOCArOl_ : 1;
        osal_u32 xGF6VwNBNiwWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6kwNBNiwWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6ywNBNiwWSuLtw6Q0wR_ : 1;
        osal_u32 xz4CrOtltvOmIGRQOCArO6_ : 1;
        osal_u32 xGF6VwNBNiwWSuLtw6Q0wR_ : 1;
        osal_u32 xGF6kwNBNiwWSuLtw6Q0wR_ : 1;
        osal_u32 xz4C3OtltvOmIGRQOsRdOCA3Ol_ : 1;
        osal_u32 xGF60wNBNiwWSuLtwCLKw6QywB_ : 1;
        osal_u32 xz4CZOtltvOmIGRQOsRdOCA3Ol_ : 1;
        osal_u32 xz4CiOtltvOmIGRQOsRdOCA3Ol_ : 1;
        osal_u32 xz4C3OtltvOmIGRQOsRdOCA3O6_ : 1;
        osal_u32 xGF60wNBNiwWSuLtwCLKw6QywR_ : 1;
        osal_u32 xz4CZOtltvOmIGRQOsRdOCA3O6_ : 1;
        osal_u32 xz4CiOtltvOmIGRQOsRdOCA3O6_ : 1;
        osal_u32 xGF6ywNBNiwWSuLtwCLKw6Q0wB_ : 1;
        osal_u32 xz4CrOtltvOmIGRQOsRdOCArOl_ : 1;
        osal_u32 xGF6VwNBNiwWSuLtwCLKw6Q0wB_ : 1;
        osal_u32 xGF6kwNBNiwWSuLtwCLKw6Q0wB_ : 1;
        osal_u32 xGF6ywNBNiwWSuLtwCLKw6Q0wR_ : 1;
        osal_u32 xz4CrOtltvOmIGRQOsRdOCArO6_ : 1;
        osal_u32 xGF6VwNBNiwWSuLtwCLKw6Q0wR_ : 1;
        osal_u32 xGF6kwNBNiwWSuLtwCLKw6Q0wR_ : 1;
    } bits;

    osal_u32 u32;
} u_adc_4to1_fifo_empty_sts;

typedef union {
    struct {
        osal_u32 xz4C3OtltvOzxOmIGRQOCA3Ol_ : 1;
        osal_u32 xGF60wNBNiwGowWSuLtw6QywB_ : 1;
        osal_u32 xz4CZOtltvOzxOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4CiOtltvOzxOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4C3OtltvOzxOmIGRQOCA3O6_ : 1;
        osal_u32 xGF60wNBNiwGowWSuLtw6QywR_ : 1;
        osal_u32 xz4CZOtltvOzxOmIGRQOCA3O6_ : 1;
        osal_u32 xz4CiOtltvOzxOmIGRQOCA3O6_ : 1;
        osal_u32 xGF6ywNBNiwGowWSuLtw6Q0wB_ : 1;
        osal_u32 xz4CrOtltvOzxOmIGRQOCArOl_ : 1;
        osal_u32 xGF6VwNBNiwGowWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6kwNBNiwGowWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6ywNBNiwGowWSuLtw6Q0wR_ : 1;
        osal_u32 xz4CrOtltvOzxOmIGRQOCArO6_ : 1;
        osal_u32 xGF6VwNBNiwGowWSuLtw6Q0wR_ : 1;
        osal_u32 xGF6kwNBNiwGowWSuLtw6Q0wR_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_adc_4to1_fifo_al_empty_sts;

typedef union {
    struct {
        osal_u32 xGF6wDLi0widLudLwWhw6QywSGh_ : 1;
        osal_u32 xz4COKRvrOvSRGSROmoOCArOIzo_ : 1;
        osal_u32 xz4COKRvrOvSRGSROmoO4G4OIzo_ : 1;
        osal_u32 xz4COKRvrOvSRGSROmoOCA3_ : 1;
        osal_u32 xGF6wDLi0widLudLwWhw6Q0_ : 1;
        osal_u32 xGF6wDLi0widLudLwWhwFuF_ : 1;
        osal_u32 reserved_0 : 26;
    } bits;

    osal_u32 u32;
} u_adc_4to1_output_en;

typedef union {
    struct {
        osal_u32 xz4C3O4G4OtltvOtSxxOCA3Ol_ : 1;
        osal_u32 xGF60wFuFwNBNiwNdoow6QywB_ : 1;
        osal_u32 xz4CZO4G4OtltvOtSxxOCA3Ol_ : 1;
        osal_u32 xz4CiO4G4OtltvOtSxxOCA3Ol_ : 1;
        osal_u32 xz4C3O4G4OtltvOtSxxOCA3O6_ : 1;
        osal_u32 xGF60wFuFwNBNiwNdoow6QywR_ : 1;
        osal_u32 xz4CZO4G4OtltvOtSxxOCA3O6_ : 1;
        osal_u32 xz4CiO4G4OtltvOtSxxOCA3O6_ : 1;
        osal_u32 xGF6ywFuFwNBNiwNdoow6Q0wB_ : 1;
        osal_u32 xz4CrO4G4OtltvOtSxxOCArOl_ : 1;
        osal_u32 xGF6VwFuFwNBNiwNdoow6Q0wB_ : 1;
        osal_u32 xGF6kwFuFwNBNiwNdoow6Q0wB_ : 1;
        osal_u32 xGF6ywFuFwNBNiwNdoow6Q0wR_ : 1;
        osal_u32 xz4CrO4G4OtltvOtSxxOCArO6_ : 1;
        osal_u32 xGF6VwFuFwNBNiwNdoow6Q0wR_ : 1;
        osal_u32 xGF6kwFuFwNBNiwNdoow6Q0wR_ : 1;
        osal_u32 xz4C3O4G4OtltvOtSxxOsRdOCA3Ol_ : 1;
        osal_u32 xGF60wFuFwNBNiwNdoowCLKw6QywB_ : 1;
        osal_u32 xz4CZO4G4OtltvOtSxxOsRdOCA3Ol_ : 1;
        osal_u32 xz4CiO4G4OtltvOtSxxOsRdOCA3Ol_ : 1;
        osal_u32 xz4C3O4G4OtltvOtSxxOsRdOCA3O6_ : 1;
        osal_u32 xGF60wFuFwNBNiwNdoowCLKw6QywR_ : 1;
        osal_u32 xz4CZO4G4OtltvOtSxxOsRdOCA3O6_ : 1;
        osal_u32 xz4CiO4G4OtltvOtSxxOsRdOCA3O6_ : 1;
        osal_u32 xGF6ywFuFwNBNiwNdoowCLKw6Q0wB_ : 1;
        osal_u32 xz4CrO4G4OtltvOtSxxOsRdOCArOl_ : 1;
        osal_u32 xGF6VwFuFwNBNiwNdoowCLKw6Q0wB_ : 1;
        osal_u32 xGF6kwFuFwNBNiwNdoowCLKw6Q0wB_ : 1;
        osal_u32 xGF6ywFuFwNBNiwNdoowCLKw6Q0wR_ : 1;
        osal_u32 xz4CrO4G4OtltvOtSxxOsRdOCArO6_ : 1;
        osal_u32 xGF6VwFuFwNBNiwNdoowCLKw6Q0wR_ : 1;
        osal_u32 xGF6kwFuFwNBNiwNdoowCLKw6Q0wR_ : 1;
    } bits;

    osal_u32 u32;
} u_adc_dpd_4to1_fifo_full_sts;

typedef union {
    struct {
        osal_u32 xz4C3O4G4OtltvOmIGRQOCA3Ol_ : 1;
        osal_u32 xGF60wFuFwNBNiwWSuLtw6QywB_ : 1;
        osal_u32 xz4CZO4G4OtltvOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4CiO4G4OtltvOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4C3O4G4OtltvOmIGRQOCA3O6_ : 1;
        osal_u32 xGF60wFuFwNBNiwWSuLtw6QywR_ : 1;
        osal_u32 xz4CZO4G4OtltvOmIGRQOCA3O6_ : 1;
        osal_u32 xz4CiO4G4OtltvOmIGRQOCA3O6_ : 1;
        osal_u32 xGF6ywFuFwNBNiwWSuLtw6Q0wB_ : 1;
        osal_u32 xz4CrO4G4OtltvOmIGRQOCArOl_ : 1;
        osal_u32 xGF6VwFuFwNBNiwWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6kwFuFwNBNiwWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6ywFuFwNBNiwWSuLtw6Q0wR_ : 1;
        osal_u32 xz4CrO4G4OtltvOmIGRQOCArO6_ : 1;
        osal_u32 xGF6VwFuFwNBNiwWSuLtw6Q0wR_ : 1;
        osal_u32 xGF6kwFuFwNBNiwWSuLtw6Q0wR_ : 1;
        osal_u32 xz4C3O4G4OtltvOmIGRQOsRdOCA3Ol_ : 1;
        osal_u32 xGF60wFuFwNBNiwWSuLtwCLKw6QywB_ : 1;
        osal_u32 xz4CZO4G4OtltvOmIGRQOsRdOCA3Ol_ : 1;
        osal_u32 xz4CiO4G4OtltvOmIGRQOsRdOCA3Ol_ : 1;
        osal_u32 xz4C3O4G4OtltvOmIGRQOsRdOCA3O6_ : 1;
        osal_u32 xGF60wFuFwNBNiwWSuLtwCLKw6QywR_ : 1;
        osal_u32 xz4CZO4G4OtltvOmIGRQOsRdOCA3O6_ : 1;
        osal_u32 xz4CiO4G4OtltvOmIGRQOsRdOCA3O6_ : 1;
        osal_u32 xGF6ywFuFwNBNiwWSuLtwCLKw6Q0wB_ : 1;
        osal_u32 xz4CrO4G4OtltvOmIGRQOsRdOCArOl_ : 1;
        osal_u32 xGF6VwFuFwNBNiwWSuLtwCLKw6Q0wB_ : 1;
        osal_u32 xGF6kwFuFwNBNiwWSuLtwCLKw6Q0wB_ : 1;
        osal_u32 xGF6ywFuFwNBNiwWSuLtwCLKw6Q0wR_ : 1;
        osal_u32 xz4CrO4G4OtltvOmIGRQOsRdOCArO6_ : 1;
        osal_u32 xGF6VwFuFwNBNiwWSuLtwCLKw6Q0wR_ : 1;
        osal_u32 xGF6kwFuFwNBNiwWSuLtwCLKw6Q0wR_ : 1;
    } bits;

    osal_u32 u32;
} u_adc_dpd_4to1_fifo_empty_sts;

typedef union {
    struct {
        osal_u32 xz4C3O4G4OtltvOzxOmIGRQOCA3Ol_ : 1;
        osal_u32 xGF60wFuFwNBNiwGowWSuLtw6QywB_ : 1;
        osal_u32 xz4CZO4G4OtltvOzxOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4CiO4G4OtltvOzxOmIGRQOCA3Ol_ : 1;
        osal_u32 xz4C3O4G4OtltvOzxOmIGRQOCA3O6_ : 1;
        osal_u32 xGF60wFuFwNBNiwGowWSuLtw6QywR_ : 1;
        osal_u32 xz4CZO4G4OtltvOzxOmIGRQOCA3O6_ : 1;
        osal_u32 xz4CiO4G4OtltvOzxOmIGRQOCA3O6_ : 1;
        osal_u32 xGF6ywFuFwNBNiwGowWSuLtw6Q0wB_ : 1;
        osal_u32 xz4CrO4G4OtltvOzxOmIGRQOCArOl_ : 1;
        osal_u32 xGF6VwFuFwNBNiwGowWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6kwFuFwNBNiwGowWSuLtw6Q0wB_ : 1;
        osal_u32 xGF6ywFuFwNBNiwGowWSuLtw6Q0wR_ : 1;
        osal_u32 xz4CrO4G4OtltvOzxOmIGRQOCArO6_ : 1;
        osal_u32 xGF6VwFuFwNBNiwGowWSuLtw6Q0wR_ : 1;
        osal_u32 xGF6kwFuFwNBNiwGowWSuLtw6Q0wR_ : 1;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_adc_dpd_4to1_fifo_al_empty_sts;

typedef union {
    struct {
        osal_u32 xFG6wSihBLi_wNBNiwiqW_NoiT_ : 1;
        osal_u32 x4zCOtltvOvPtOCxw_ : 1;
        osal_u32 xGF6wSihBLi_wNBNiwiqW_NoiT_ : 1;
        osal_u32 xz4COtltvOvPtOCxw_ : 1;
        osal_u32 xqxO4zCO2moOIvolRvwOtltvOvPmwtxvq_ : 1;
        osal_u32 xqxO4zCO2moOIvolRvwOtltvOvPtOCxw_ : 1;
        osal_u32 xqxOz4CO2moOIvolRvwOtltvOvPmwtxvq_ : 1;
        osal_u32 xqxOz4CO2moOIvolRvwOtltvOvPtOCxw_ : 1;
        osal_u32 xqxO4zCO2moOIvolRvwOtltvOSo4mwtxvq_ : 1;
        osal_u32 xTowFG6wgWhwSihBLi_wNBNiwdFNw6o__ : 1;
        osal_u32 xqxOz4CO2moOIvolRvwOtltvOSo4mwtxvq_ : 1;
        osal_u32 xTowGF6wgWhwSihBLi_wNBNiwdFNw6o__ : 1;
        osal_u32 reserved_0 : 20;
    } bits;

    osal_u32 u32;
} u_sample_monitor_fifo_overflow_sts;

typedef union {
    struct {
        osal_u32 xGjjwTowGF6w6L_oBhBw6ywVwSGh_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlini_c0_2_man;

typedef union {
    struct {
        osal_u32 xzyyOqxOz4COCRwxlolOC3OZOsmx_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlini_c0_2_sel;

typedef union {
    struct {
        osal_u32 xGjjwTowGF6w6L_oBhBw6ywVwCLC_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlini_c0_2_sts;

typedef union {
    struct {
        osal_u32 xGjjwTowGF6w6L_oBhRw6ywVwSGh_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlinq_c0_2_man;

typedef union {
    struct {
        osal_u32 xzyyOqxOz4COCRwxlo6OC3OZOsmx_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlinq_c0_2_sel;

typedef union {
    struct {
        osal_u32 xGjjwTowGF6w6L_oBhRw6ywVwCLC_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlinq_c0_2_sts;

typedef union {
    struct {
        osal_u32 xzyyOqxOz4COCRwxlolOCrOZOIzo_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlini_c1_2_man;

typedef union {
    struct {
        osal_u32 xGjjwTowGF6w6L_oBhBw60wVwCWo_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlini_c1_2_sel;

typedef union {
    struct {
        osal_u32 xzyyOqxOz4COCRwxlolOCrOZOsRs_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlini_c1_2_sts;

typedef union {
    struct {
        osal_u32 xzyyOqxOz4COCRwxlo6OCrOZOIzo_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlinq_c1_2_man;

typedef union {
    struct {
        osal_u32 xGjjwTowGF6w6L_oBhRw60wVwCWo_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlinq_c1_2_sel;

typedef union {
    struct {
        osal_u32 xzyyOqxOz4COCRwxlo6OCrOZOsRs_ : 16;
        osal_u32 reserved_0 : 16;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlinq_c1_2_sts;

typedef union {
    struct {
        osal_u32 xzyyOqxOz4COCRwxvSRlOC3OsRs_ : 8;
        osal_u32 xGjjwTowGF6w6L_oidLBw60wCLC_ : 8;
        osal_u32 xzyyOqxOz4COCRwxvSR6OC3OsRs_ : 8;
        osal_u32 xGjjwTowGF6w6L_oidLRw60wCLC_ : 8;
    } bits;

    osal_u32 u32;
} u_abb_wl_adc_ctrlout_sts;

typedef union {
    struct {
        osal_u32 xzyyOqxO4zCOCRwxloOC3OIzo_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 xGjjwTowFG6w6L_oBhw60wSGh_ : 8;
        osal_u32 reserved_1 : 8;
    } bits;

    osal_u32 u32;
} u_abb_wl_dac_ctrlin_man;

typedef union {
    struct {
        osal_u32 xzyyOqxO4zCOCRwxvSROC3OsRs_ : 8;
        osal_u32 reserved_0 : 8;
        osal_u32 xGjjwTowFG6w6L_oidLw60wCLC_ : 8;
        osal_u32 reserved_1 : 8;
    } bits;

    osal_u32 u32;
} u_abb_wl_dac_ctrlout_sts;

#endif
