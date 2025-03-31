/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_edca.c.
 * Create: 2023-01-11
 */

#include "hal_edca.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_EDCA_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_bool_enum_uint8 g_wfa_aifsn_lock[WLAN_WME_AC_BUTT] = {OSAL_FALSE};
oal_bool_enum_uint8 g_wfa_cw_lock[WLAN_WME_AC_BUTT] = {OSAL_FALSE};

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
/*****************************************************************************
功能描述  : EDCA优化特性下清空mac部分统计寄存器
*****************************************************************************/
osal_void hh503_set_counter1_clear(osal_void)
{
    osal_u32 val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x13C);

    /* 先写1再写0 */
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x13C, val | oal_mask(0x3, 0x19));
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x13C, val & oal_mask_inverse(0x3, 0x19));
}
#endif


/*****************************************************************************
 功能描述  : hh503更新新的mu edca lifetime参数到寄存器
*****************************************************************************/
osal_void hh503_set_mu_edca_lifetime(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    osal_u32 val;
    osal_u32 reg_val;

    val = ((bk << HH503_MAC_CFG_AC_BK_MU_EDCA_TIMER_OFFSET) | (be << HH503_MAC_CFG_AC_BE_MU_EDCA_TIMER_OFFSET) |
           (vi << HH503_MAC_CFG_AC_VI_MU_EDCA_TIMER_OFFSET) | (vo << HH503_MAC_CFG_AC_VO_MU_EDCA_TIMER_OFFSET));

    reg_val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xCC);

    reg_val &= (~(HH503_MAC_CFG_AC_BK_MU_EDCA_TIMER_MASK | HH503_MAC_CFG_AC_BE_MU_EDCA_TIMER_MASK |
                  HH503_MAC_CFG_AC_VI_MU_EDCA_TIMER_MASK | HH503_MAC_CFG_AC_VO_MU_EDCA_TIMER_MASK));
    reg_val |= val;

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xCC, reg_val);
}

/*****************************************************************************
 功能描述  : 设置11ax mu edca aifs
*****************************************************************************/
osal_void hh503_set_mu_edca_aifsn(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    osal_u32 val;
    osal_u32 reg_val;

    val = ((vo << HH503_MAC_CFG_MU_VO_AIFS_SLOT_NUM_OFFSET) | (vi << HH503_MAC_CFG_MU_VI_AIFS_SLOT_NUM_OFFSET) |
           (bk << HH503_MAC_CFG_MU_BK_AIFS_SLOT_NUM_OFFSET) | (be << HH503_MAC_CFG_MU_BE_AIFS_SLOT_NUM_OFFSET));

    reg_val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x94);

    reg_val &= (~(HH503_MAC_CFG_MU_VO_AIFS_SLOT_NUM_MASK | HH503_MAC_CFG_MU_VI_AIFS_SLOT_NUM_MASK |
                  HH503_MAC_CFG_MU_BK_AIFS_SLOT_NUM_MASK | HH503_MAC_CFG_MU_BE_AIFS_SLOT_NUM_MASK));
    reg_val |= val;

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x94, reg_val);
}

/*****************************************************************************
 功能描述  : hh503更新新的mu edca cw值到寄存器中
*****************************************************************************/
osal_void hh503_set_mu_edca_cw(osal_u8 ac_type, osal_u8 cwmax, osal_u8 cwmin)
{
    osal_u32 val;
    osal_u32 reg_val;

    cwmax &= HH503_MAC_CFG_MU_BE_CWMIN_MASK;
    cwmin &= HH503_MAC_CFG_MU_BE_CWMIN_MASK;

    reg_val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x9C);

    switch (ac_type) {
        case WLAN_WME_AC_BK:
            val = ((osal_u32)cwmax << HH503_MAC_CFG_MU_BK_CWMAX_OFFSET) |
                  ((osal_u32)cwmin << HH503_MAC_CFG_MU_BK_CWMIN_OFFSET);
            reg_val &= (~(HH503_MAC_CFG_MU_BK_CWMAX_MASK | HH503_MAC_CFG_MU_BK_CWMIN_MASK));
            break;

        case WLAN_WME_AC_BE:
            val = ((osal_u32)cwmax << HH503_MAC_CFG_MU_BE_CWMAX_OFFSET) |
                  ((osal_u32)cwmin << HH503_MAC_CFG_MU_BE_CWMIN_OFFSET);
            reg_val &= (~(HH503_MAC_CFG_MU_BE_CWMAX_MASK | HH503_MAC_CFG_MU_BE_CWMIN_MASK));
            break;

        case WLAN_WME_AC_VI:
            val = ((osal_u32)cwmax << HH503_MAC_CFG_MU_VI_CWMAX_OFFSET) |
                  ((osal_u32)cwmin << HH503_MAC_CFG_MU_VI_CWMIN_OFFSET);
            reg_val &= (~(HH503_MAC_CFG_MU_VI_CWMAX_MASK | HH503_MAC_CFG_MU_VI_CWMIN_MASK));
            break;

        case WLAN_WME_AC_VO:
            val = ((osal_u32)cwmax << HH503_MAC_CFG_MU_VO_CWMAX_OFFSET) |
                  ((osal_u32)cwmin << HH503_MAC_CFG_MU_VO_CWMIN_OFFSET);
            reg_val &= (~(HH503_MAC_CFG_MU_VO_CWMAX_MASK | HH503_MAC_CFG_MU_VO_CWMIN_MASK));
            break;

        default:
            return;
    };

    reg_val |= val;

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x9C, reg_val);
}

osal_void hh503_set_affected_acs(osal_u8 affected_acs)
{
    osal_u32 val;
    osal_u32 reg_val;

    affected_acs &= 0xf; // 未找到对应的mask宏定义，暂时用数字

    reg_val = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x7C);

    val = (osal_u32)affected_acs << 0;
    reg_val &= (~(0xf));

    reg_val |= val;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x7C, reg_val);
}

osal_void hh503_close_mu_edca_func(osal_void)
{
    osal_u32 reg_val;
    hh503_mu_edca_func_en *edca_para = OSAL_NULL;

    reg_val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xC8);

    edca_para = (hh503_mu_edca_func_en *)&reg_val;

    edca_para->xCt2OISOm4CzOtSoCOmo_ = 0x0;

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xC8, reg_val);
}

osal_void hh503_set_mu_edca_func_en(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    osal_u32 reg_val;
    hh503_mu_edca_func_en *edca_para = OSAL_NULL;

    reg_val = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xC8);

    edca_para = (hh503_mu_edca_func_en *)&reg_val;

    edca_para->x6NgwG6wjKwSdwWF6GwFBCw6QGhhWowG66WCCwWh_ = (bk == 0) ? 0x1 : 0x0;
    edca_para->xCt2OzCOymOISOm4CzO4lsOCAzoomxOzCCmssOmo_ = (be == 0) ? 0x1 : 0x0;
    edca_para->x6NgwG6wqBwSdwWF6GwFBCw6QGhhWowG66WCCwWh_ = (vi == 0) ? 0x1 : 0x0;
    edca_para->x6NgwG6wqiwSdwWF6GwFBCw6QGhhWowG66WCCwWh_ = (vo == 0) ? 0x1 : 0x0;
    edca_para->xCt2OISOm4CzOtSoCOmo_ = 0x1;

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xC8, reg_val);
}


/*****************************************************************************
功能描述  : 开启EDCA功能
*****************************************************************************/
osal_void hh503_enable_machw_edca(osal_void)
{
    u_pa_control1 pa_ctrl;
    pa_ctrl.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x0);
    pa_ctrl.bits.x6NgwWF6GwWh_ = 1;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x0, pa_ctrl.u32);
}

/*****************************************************************************
 HH503 EDCA参数配置相关接口
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_aifsn_all_ac
*****************************************************************************/
osal_void hh503_vap_set_machw_aifsn_all_ac(osal_u8 bk, osal_u8 be, osal_u8 vi, osal_u8 vo)
{
    u_aifsn aifsn;

    aifsn.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x94);
    if (!g_wfa_aifsn_lock[WLAN_WME_AC_BE]) {
        aifsn.bits.xCt2OzCOymOzltsOsxvROoSI_ = be;
    }

    /* 有wfa配置指定ac的aifsn时，不做改动 */
    if (!g_wfa_aifsn_lock[WLAN_WME_AC_BK]) {
        aifsn.bits.x6NgwG6wjKwGBNCwCoiLwhdS_ = bk;
        }
    if (!g_wfa_aifsn_lock[WLAN_WME_AC_VI]) {
        aifsn.bits.x6NgwG6wqBwGBNCwCoiLwhdS_ = vi;
    }

    if (!g_wfa_aifsn_lock[WLAN_WME_AC_VO]) {
        aifsn.bits.x6NgwG6wqiwGBNCwCoiLwhdS_ = vo;
    }
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x94, aifsn.u32);
}

/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_aifsn_ac
*****************************************************************************/
osal_void hh503_vap_set_machw_aifsn_ac_atom(wlan_wme_ac_type_enum_uint8 ac, osal_u8 aifs)
{
    u_aifsn aifsn;

    aifsn.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x94);
    switch (ac) {
        case WLAN_WME_AC_BE:
            aifsn.bits.xCt2OzCOymOzltsOsxvROoSI_ = aifs;
            break;
        case WLAN_WME_AC_BK:
            aifsn.bits.x6NgwG6wjKwGBNCwCoiLwhdS_ = aifs;
            break;
        case WLAN_WME_AC_VI:
            aifsn.bits.x6NgwG6wqBwGBNCwCoiLwhdS_ = aifs;
            break;
        case WLAN_WME_AC_VO:
            aifsn.bits.x6NgwG6wqiwGBNCwCoiLwhdS_ = aifs;
            break;
        default:
            break;
    }

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x94, aifsn.u32);
}

/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_aifsn_ac
 功能描述  : 除wfa外通用配置接口
*****************************************************************************/
osal_void hh503_vap_set_machw_aifsn_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u8 aifs)
{
    /* 在BE-VO时与wfa配置互斥,wfa配置优先 */
    if ((ac <= WLAN_WME_AC_VO) && (g_wfa_aifsn_lock[ac] == OSAL_TRUE)) {
        return;
    }

    hh503_vap_set_machw_aifsn_ac_atom(ac, aifs);
}

/*****************************************************************************
 函 数 名  : hh503_vap_get_machw_aifsn_ac
*****************************************************************************/
osal_void hh503_vap_get_machw_aifsn_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u8 *aifs)
{
    u_aifsn reg_aifsn = {0};
    reg_aifsn.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x94);

    if (ac == WLAN_WME_AC_BE) {
        *aifs = reg_aifsn.bits.xCt2OzCOymOzltsOsxvROoSI_;
    } else if (ac == WLAN_WME_AC_BK) {
        *aifs = reg_aifsn.bits.x6NgwG6wjKwGBNCwCoiLwhdS_;
    } else if (ac == WLAN_WME_AC_VI) {
        *aifs = reg_aifsn.bits.x6NgwG6wqBwGBNCwCoiLwhdS_;
    } else if (ac == WLAN_WME_AC_VO) {
        *aifs = reg_aifsn.bits.x6NgwG6wqiwGBNCwCoiLwhdS_;
    }
}
#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_aifsn_ac_wfa
 功能描述  : wfa认证时，仅配置wfa指定参数，wfa不配置时，恢复原值
*****************************************************************************/
osal_void hh503_vap_set_machw_aifsn_ac_wfa(wlan_wme_ac_type_enum_uint8 ac, osal_u8 aifs,
    wlan_wme_ac_type_enum_uint8 wfa_lock)
{
    static osal_u8 val[WLAN_WME_AC_BUTT] = {0}; /* 保存使用wfa指定配置前的配置值 */

    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "{hh503_vap_set_machw_aifsn_ac_wfa::ac illegal[%d], fail to set reg val.}\r\n",
            ac);
        return;
    }

    /* 普通配置模式到wfa配置模式，保存之前的aifsn参数 */
    if ((wfa_lock == OSAL_TRUE) && (g_wfa_aifsn_lock[ac] == OSAL_FALSE)) {
        val[ac] = (hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x94) >> (0x4 * ac)) & 0xF;
    /* wfa配置模式到普通配置模式，恢复之前的aifsn参数 */
    } else if ((wfa_lock == OSAL_FALSE) && (g_wfa_aifsn_lock[ac] == OSAL_TRUE)) {
        hh503_vap_set_machw_aifsn_ac_atom(ac, (osal_u8)val[ac]);
    }

    g_wfa_aifsn_lock[ac] = wfa_lock;

    /* 配置wfa指定参数 */
    if (wfa_lock == OSAL_TRUE) {
        hh503_vap_set_machw_aifsn_ac_atom(ac, aifs);
    }
}
#endif

/*****************************************************************************
 函 数 名  : hh503_vap_set_edca_machw_cw_atom
*****************************************************************************/
osal_void hh503_vap_set_edca_machw_cw_atom(osal_u8 cwmax, osal_u8 cwmin, osal_u8 ac_type)
{
    u_cwminmaxac cwminmaxac;
    cwminmaxac.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x98);
    switch (ac_type) {
        case WLAN_WME_AC_BK:
            cwminmaxac.bits.xCt2OzCOydOCqIz8_ = cwmax;
            cwminmaxac.bits.x6NgwG6wjKw6TSBh_ = cwmin;
            break;

        case WLAN_WME_AC_BE:
            cwminmaxac.bits.x6NgwG6wjWw6TSGP_ = cwmax;
            cwminmaxac.bits.xCt2OzCOymOCqIlo_ = cwmin;
            break;

        case WLAN_WME_AC_VI:
            cwminmaxac.bits.xCt2OzCOPlOCqIz8_ = cwmax;
            cwminmaxac.bits.x6NgwG6wqBw6TSBh_ = cwmin;
            break;

        case WLAN_WME_AC_VO:
            cwminmaxac.bits.xCt2OzCOPvOCqIz8_ = cwmax;
            cwminmaxac.bits.x6NgwG6wqiw6TSBh_ = cwmin;
            break;

        default:
            break;
    }

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x98, cwminmaxac.u32);
}

/*****************************************************************************
 函 数 名  : hh503_vap_set_edca_machw_cw
*****************************************************************************/
osal_void hh503_vap_set_edca_machw_cw(osal_u8 cwmax, osal_u8 cwmin, osal_u8 ac_type)
{
    /* 在BE-VO时与wfa配置互斥,wfa配置优先 */
    if ((ac_type <= WLAN_WME_AC_VO) && (g_wfa_cw_lock[ac_type] == OSAL_TRUE)) {
        return;
    }

    hh503_vap_set_edca_machw_cw_atom(cwmax, cwmin, ac_type);
}

#ifdef _PRE_WLAN_CFGID_DEBUG
/*****************************************************************************
 函 数 名  : hh503_vap_set_edca_machw_cw_wfa
*****************************************************************************/
osal_void hh503_vap_set_edca_machw_cw_wfa(osal_u8 cwmaxmin, osal_u8 ac_type,
    wlan_wme_ac_type_enum_uint8 wfa_lock)
{
    static osal_u32 reg_val[WLAN_WME_AC_BUTT] = {0}; /* 保存使用wfa指定配置前的配置值 */

    u_cwminmaxac cwminmaxac;

    if (ac_type >= WLAN_WME_AC_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "{hh503_vap_set_edca_machw_cw_wfa::ac illegal[%d], fail to set reg val.}\r\n",
            ac_type);
        return;
    }

    cwminmaxac.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x98);

    /* 普通配置模式到wfa配置模式，保存之前的aifsn参数 */
    if ((wfa_lock == OSAL_TRUE) && (g_wfa_cw_lock[ac_type] == OSAL_FALSE)) {
        reg_val[ac_type] = cwminmaxac.u32 >> (8 * ac_type); /* 乘以8 */
    /* wfa配置模式到普通配置模式，恢复之前的aifsn参数 */
    } else if ((wfa_lock == OSAL_FALSE) && (g_wfa_cw_lock[ac_type] == OSAL_TRUE)) {
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x98, reg_val[ac_type]);
    }

    g_wfa_cw_lock[ac_type] = wfa_lock;

    /* 配置wfa指定参数 */
    if (wfa_lock == OSAL_TRUE) {
        cwminmaxac.u32 &= ~(0xff << (8 * ac_type)); /* 乘以8 */
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x98, cwminmaxac.u32 | (cwmaxmin << (8 * ac_type))); /* ac_type乘以8 */
    }
}
#endif

/*****************************************************************************
 函 数 名  : hh503_vap_get_edca_machw_cw
*****************************************************************************/
osal_void hh503_vap_get_edca_machw_cw(osal_u8 *cwmax, osal_u8 *cwmin, osal_u8 ac_type) // over
{
    u_cwminmaxac cwminmaxac;
    cwminmaxac.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x98);
    switch (ac_type) {
        case WLAN_WME_AC_BK:
            *cwmax = cwminmaxac.bits.xCt2OzCOydOCqIz8_;
            *cwmin = cwminmaxac.bits.x6NgwG6wjKw6TSBh_;
            break;

        case WLAN_WME_AC_BE:
            *cwmax = cwminmaxac.bits.x6NgwG6wjWw6TSGP_;
            *cwmin = cwminmaxac.bits.xCt2OzCOymOCqIlo_;
            break;

        case WLAN_WME_AC_VI:
            *cwmax = cwminmaxac.bits.xCt2OzCOPlOCqIz8_;
            *cwmin = cwminmaxac.bits.x6NgwG6wqBw6TSBh_;
            break;

        case WLAN_WME_AC_VO:
            *cwmax = cwminmaxac.bits.xCt2OzCOPvOCqIz8_;
            *cwmin = cwminmaxac.bits.x6NgwG6wqiw6TSBh_;
            break;

        default:
            break;
    }
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_edca_bkbe_lifetime
*****************************************************************************/
osal_void hh503_vap_set_machw_edca_bkbe_lifetime(osal_u16 be, osal_u16 bk)
{
    u_acbkbe_edca_lifetimelmt limit = {.u32 = 0};

    limit.bits.x6NgwG6wjKwoBNWLBSWwqGo_ = bk;
    limit.bits.xCt2OzCOymOxltmRlImOPzx_ = be;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xBC, limit.u32);
}

/*****************************************************************************
 函 数 名  : hh503_vap_get_machw_edca_bkbe_lifetime
*****************************************************************************/
osal_void hh503_vap_get_machw_edca_bkbe_lifetime(osal_u16 *be, osal_u16 *bk)
{
    u_acbkbe_edca_lifetimelmt limit;

    limit.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xBC);
    *bk = limit.bits.x6NgwG6wjKwoBNWLBSWwqGo_;
    *be = limit.bits.xCt2OzCOymOxltmRlImOPzx_;
}

/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_edca_vivo_lifetime
*****************************************************************************/
osal_void hh503_vap_set_machw_edca_vivo_lifetime(osal_u16 vo, osal_u16 vi)
{
    u_acvivo_edca_lifetimelmt limit = {.u32 = 0};

    limit.bits.x6NgwG6wqBwoBNWLBSWwqGo_ = vi;
    limit.bits.x6NgwG6wqiwoBNWLBSWwqGo_ = vo;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xC0, limit.u32);
}

/*****************************************************************************
 函 数 名  : hh503_vap_get_machw_edca_vivo_lifetime
*****************************************************************************/
osal_void hh503_vap_get_machw_edca_vivo_lifetime(osal_u16 *vo, osal_u16 *vi)
{
    u_acvivo_edca_lifetimelmt limit;

    limit.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xC0);
    *vi = limit.bits.x6NgwG6wqBwoBNWLBSWwqGo_;
    *vo = limit.bits.x6NgwG6wqiwoBNWLBSWwqGo_;
}
#endif

/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_txop_limit_bkbe
*****************************************************************************/
osal_void hh503_vap_set_machw_txop_limit_bkbe(osal_u16 be, osal_u16 bk)
{
    u_edca_txoplimit_acbkbe limit = {.u32 = 0};
    limit.bits.x6NgwG6wjKwLPiuwoBSBL_ = bk;
    limit.bits.xCt2OzCOymOR8vGOxlIlR_ = be;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xB0, limit.u32);
}

/*****************************************************************************
 函 数 名  : hh503_vap_get_machw_txop_limit_bkbe
*****************************************************************************/
osal_void hh503_vap_get_machw_txop_limit_bkbe(osal_u16 *be, osal_u16 *bk)
{
    u_edca_txoplimit_acbkbe limit;
    limit.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xB0);
    *be = limit.bits.xCt2OzCOymOR8vGOxlIlR_;
    *bk = limit.bits.x6NgwG6wjKwLPiuwoBSBL_;
}

/*****************************************************************************
 函 数 名  : hh503_vap_set_machw_txop_limit_vivo
*****************************************************************************/
osal_void hh503_vap_set_machw_txop_limit_vivo(osal_u16 vo, osal_u16 vi)
{
    u_edca_txoplimit_acvivo limit = {.u32 = 0};
    limit.bits.x6NgwG6wqBwLPiuwoBSBL_ = vi;
    limit.bits.x6NgwG6wqiwLPiuwoBSBL_ = vo;
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0xB4, limit.u32);
}

/*****************************************************************************
 函 数 名  : hh503_vap_get_machw_txop_limit_vivo
*****************************************************************************/
osal_void hh503_vap_get_machw_txop_limit_vivo(osal_u16 *vo, osal_u16 *vi)
{
    u_edca_txoplimit_acvivo limit;

    limit.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xB4);
    *vi = limit.bits.x6NgwG6wqBwLPiuwoBSBL_;
    *vo= limit.bits.x6NgwG6wqiwLPiuwoBSBL_;
}
/*****************************************************************************
 函 数 名  : hh503_vap_get_machw_txop_limit_ac
*****************************************************************************/
osal_void hh503_vap_get_machw_txop_limit_ac(wlan_wme_ac_type_enum_uint8 ac, osal_u16 *txop)
{
    u_edca_txoplimit_acvivo vivo_limit;
    u_edca_txoplimit_acbkbe bebk_limit;

    bebk_limit.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xB0);
    vivo_limit.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0xB4);

    if (ac == WLAN_WME_AC_BE) {
        *txop = bebk_limit.bits.xCt2OzCOymOR8vGOxlIlR_;
    } else if (ac == WLAN_WME_AC_BK) {
        *txop = bebk_limit.bits.x6NgwG6wjKwLPiuwoBSBL_;
    } else if (ac == WLAN_WME_AC_VI) {
        *txop = vivo_limit.bits.x6NgwG6wqBwLPiuwoBSBL_;
    } else if (ac == WLAN_WME_AC_VO) {
        *txop = vivo_limit.bits.x6NgwG6wqiwLPiuwoBSBL_;
    }
}
/*****************************************************************************
 函 数 名  : hh503_vap_set_txop_duration_threshold
*****************************************************************************/
osal_void hh503_vap_set_txop_duration_threshold(osal_u32 txop_duration_th, osal_u32 vap_idx)
{
    u_tx_param_val_pilot2 tx;
    tx.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x118);
    tx.bits.x6NgwLPiuwFd_GLBihwLQ_W_ = txop_duration_th;
    tx.bits.xCt2Orrz8OsRzOPzGOlo4m83_ = vap_idx;

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x118, tx.u32);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
