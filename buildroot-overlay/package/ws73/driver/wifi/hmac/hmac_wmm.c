/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * 文 件 名   : hmac_wmm.c
 * 生成日期   : 2022年9月21日
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_wmm.h"
#include "hmac_config.h"
#include "oal_ext_if.h"
#include "oal_types.h"
#include "hmac_ccpriv.h"
#include "dmac_ext_if_hcm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_WMM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 强制报文走指定队列,默认关闭 */
oal_bool_enum_uint8 g_set_ac_mode_enable = OAL_FALSE;
wlan_wme_ac_type_enum_uint8 g_set_ac_mode = WLAN_TIDNO_BEST_EFFORT;

typedef struct {
    osal_char *cmd_name;      /* 命令字符串 */
    osal_u32 set_cfg_id;    /* 命令对应处理函数 */
    osal_u32 get_cfg_id;    /* 命令对应处理函数 */
    osal_u32 cfg_min;
    osal_u32 cfg_max;
} edca_cmd_entry;

OAL_CONST edca_cmd_entry edca_param_table[] = {
    {
        "aifs", WLAN_MSG_W2H_CFG_EDCA_TABLE_AIFSN, WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_AIFSN,
        WLAN_QEDCA_TABLE_AIFSN_MIN, WLAN_QEDCA_TABLE_AIFSN_MAX
    },
    {
        "cwmin", WLAN_MSG_W2H_CFG_EDCA_TABLE_CWMIN, WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMIN,
        WLAN_QEDCA_TABLE_CWMIN_MIN, WLAN_QEDCA_TABLE_CWMIN_MAX
    },
    {
        "cwmax", WLAN_MSG_W2H_CFG_EDCA_TABLE_CWMAX, WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMAX,
        WLAN_QEDCA_TABLE_CWMAX_MIN, WLAN_QEDCA_TABLE_CWMAX_MAX
    },
    {
        "txop", WLAN_MSG_W2H_CFG_EDCA_TABLE_TXOP_LIMIT, WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_TXOP_LIMIT,
        WLAN_QEDCA_TABLE_TXOP_LIMIT_MIN, WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX
    },
    {
        "qaifs",  WLAN_MSG_W2H_CFG_QEDCA_TABLE_AIFSN, WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_AIFSN,
        WLAN_QEDCA_TABLE_AIFSN_MIN, WLAN_QEDCA_TABLE_AIFSN_MAX
    },
    {
        "qcwmin", WLAN_MSG_W2H_CFG_QEDCA_TABLE_CWMIN, WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMIN,
        WLAN_QEDCA_TABLE_CWMIN_MIN, WLAN_QEDCA_TABLE_CWMIN_MAX
    },
    {
        "qcwmax", WLAN_MSG_W2H_CFG_QEDCA_TABLE_CWMAX, WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMAX,
        WLAN_QEDCA_TABLE_CWMAX_MIN, WLAN_QEDCA_TABLE_CWMAX_MAX
    },
    {
        "qtxop",  WLAN_MSG_W2H_CFG_QEDCA_TABLE_TXOP_LIMIT, WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_TXOP_LIMIT,
        WLAN_QEDCA_TABLE_TXOP_LIMIT_MIN, WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX
    },
};
/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : hmac_config_open_wmm
 功能描述  : ?懈?新host侧mib信息位中的Qos位置
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_u32  hmac_config_open_wmm(hmac_vap_stru *hmac_vap, osal_u16 len, const osal_u8 *param)
{
    oal_bool_enum_uint8 wmm = *(oal_bool_enum_uint8 *)param;
    unref_param(len);
    hmac_config_wmm_switch(hmac_vap, wmm);
    /* 开关WMM，更新host侧mib信息位中的Qos位置 */
    mac_mib_set_dot11_qos_option_implemented(hmac_vap, wmm);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
 函 数 名  : hmac_config_set_qap_aifsn
 功能描述  : 设置AP的aifsn参数，需要操作寄存器
*****************************************************************************/
osal_s32 hmac_config_set_qap_aifsn(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user)
{
    osal_u8 ac;
    osal_u32 value;

    ac     = (osal_u8)param_user->ac;
    value  = param_user->value;

    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_qap_aifsn::invalid param, ac=%d value=%d.", hmac_vap->vap_id, ac, value);
        return OAL_FAIL;
    }

    mac_mib_set_qap_edca_table_aifsn(hmac_vap, ac, value);

    hal_vap_set_machw_aifsn_ac((wlan_wme_ac_type_enum_uint8)ac, (osal_u8)value);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_qap_cwmax
 功能描述  : 设置AP的cwmax参数，需要操作寄存器
*****************************************************************************/
osal_s32 hmac_config_set_qap_cwmax(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user)
{
    osal_u8 ac;
    osal_u8 cwmax_pre = 0;
    osal_u8 cwmax;
    osal_u8 cwmin = 0;

    ac     = (osal_u8)param_user->ac;
    cwmax  = (osal_u8)param_user->value;
    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_qap_cwmax::invalid param, ac=%d cwmax=%d.", hmac_vap->vap_id, ac, cwmax);
        return OAL_FAIL;
    }

    mac_mib_set_qap_edca_table_cwmax(hmac_vap, ac, (osal_u32)cwmax);

    hal_vap_get_edca_machw_cw(&cwmax_pre, &cwmin, ac);
    hal_vap_set_edca_machw_cw(cwmax, cwmin, ac);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_qap_cwmin
 功能描述  : 设置AP的cwmin参数，需要操作寄存器
*****************************************************************************/
osal_s32 hmac_config_set_qap_cwmin(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user)
{
    osal_u8 ac;
    osal_u8 cwmin;
    osal_u8 cwmax = 0;
    osal_u8 cwmin_pre = 0;

    ac     = (osal_u8)param_user->ac;
    cwmin  = (osal_u8)param_user->value;
    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_qap_cwmin::invalid param, ac=%d cwmin=%d.", hmac_vap->vap_id, ac, cwmin);
        return OAL_FAIL;
    }

    mac_mib_set_qap_edca_table_cwmin(hmac_vap, ac, (osal_u32)cwmin);

    hal_vap_get_edca_machw_cw(&cwmax, &cwmin_pre, ac);
    hal_vap_set_edca_machw_cw(cwmax, cwmin, ac);

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_config_set_qap_msdu_lifetime
 功能描述  : 设置AP的msdu_lifetime参数，需要操作寄存器
*****************************************************************************/
osal_s32 hmac_config_set_qap_msdu_lifetime(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user)
{
    osal_u32 ac;
    osal_u32 value;
    osal_u16 lifetime_bk = 0;
    osal_u16 lifetime_be = 0;
    osal_u16 lifetime_vi = 0;
    osal_u16 lifetime_vo = 0;
    osal_u16 pre_value = 0;

    ac     = param_user->ac;
    value  = param_user->value;

    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_qap_msdu_lifetime::invalid param, ac=%d value=%d.",
            hmac_vap->vap_id, ac, value);
        return OAL_FAIL;
    }

    switch (ac) {
        case WLAN_WME_AC_BK:
            hal_vap_get_machw_edca_bkbe_lifetime(&lifetime_be, &pre_value);
            hal_vap_set_machw_edca_bkbe_lifetime(lifetime_be, (osal_u16)value);
            break;

        case WLAN_WME_AC_BE:
            hal_vap_get_machw_edca_bkbe_lifetime(&pre_value, &lifetime_bk);
            hal_vap_set_machw_edca_bkbe_lifetime((osal_u16)value, lifetime_bk);
            break;

        case WLAN_WME_AC_VI:
            hal_vap_get_machw_edca_vivo_lifetime(&lifetime_vo, &pre_value);
            hal_vap_set_machw_edca_vivo_lifetime(lifetime_vo, (osal_u16)value);
            break;

        case WLAN_WME_AC_VO:
            hal_vap_get_machw_edca_vivo_lifetime(&pre_value, &lifetime_vi);
            hal_vap_set_machw_edca_vivo_lifetime((osal_u16)value, lifetime_vi);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_qap_txop_limit
 功能描述  : 设置AP的txop_limit参数，需要操作寄存器
*****************************************************************************/
osal_s32 hmac_config_set_qap_txop_limit(hmac_vap_stru *hmac_vap, hmac_config_edca_param *param_user)
{
    osal_u32  ac;
    osal_u32  value;
    osal_u16  txop_bk = 0;
    osal_u16  txop_be = 0;
    osal_u16  txop_vi = 0;
    osal_u16  txop_vo = 0;
    osal_u16  pre_value = 0;

    ac     = param_user->ac;
    value  = param_user->value;

    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_qap_txop_limit::invalid param, ac=%d value=%d.", hmac_vap->vap_id, ac, value);
        return OAL_FAIL;
    }

    mac_mib_set_qap_edca_table_txop_limit(hmac_vap, (osal_u8)ac, value);

    switch (ac) {
        case WLAN_WME_AC_BK:
            hal_vap_get_machw_txop_limit_bkbe(&txop_be, &pre_value);
            hal_vap_set_machw_txop_limit_bkbe(txop_be, (osal_u16)value);
            break;

        case WLAN_WME_AC_BE:
            hal_vap_get_machw_txop_limit_bkbe(&pre_value, &txop_bk);
            hal_vap_set_machw_txop_limit_bkbe((osal_u16)value, txop_bk);
            break;

        case WLAN_WME_AC_VI:
            hal_vap_get_machw_txop_limit_vivo(&txop_vo, &pre_value);
            hal_vap_set_machw_txop_limit_vivo(txop_vo, (osal_u16)value);
            break;

        case WLAN_WME_AC_VO:
            hal_vap_get_machw_txop_limit_vivo(&pre_value, &txop_vi);
            hal_vap_set_machw_txop_limit_vivo((osal_u16)value, txop_vi);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

osal_u32 hmac_config_set_wmm_qap_params(hmac_vap_stru *hmac_vap, hmac_config_wmm_para_stru *cfg_stru)
{
    hmac_config_edca_param param = {0};
    osal_u32 value;

    value = cfg_stru->value;
    param.ac = cfg_stru->ac;
    param.value = cfg_stru->value;

    switch (cfg_stru->cfg_id) {                                                    /* 根据sub-ioctl id填写WID */
        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_CWMIN:
            if ((value > WLAN_QEDCA_TABLE_CWMIN_MAX) || (value < WLAN_QEDCA_TABLE_CWMIN_MIN)) {
                return OAL_FAIL;
            }
            hmac_config_set_qap_cwmin(hmac_vap, &param);
            break;

        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_CWMAX:
            if ((value > WLAN_QEDCA_TABLE_CWMAX_MAX) || (value < WLAN_QEDCA_TABLE_CWMAX_MIN)) {
                return OAL_FAIL;
            }
            hmac_config_set_qap_cwmax(hmac_vap, &param);
            break;

        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_AIFSN:
            if ((value < WLAN_QEDCA_TABLE_AIFSN_MIN) || (value > WLAN_QEDCA_TABLE_AIFSN_MAX)) {
                return OAL_FAIL;
            }
            hmac_config_set_qap_aifsn(hmac_vap, &param);
            break;

        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_TXOP_LIMIT:
            if (value > WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX) {
                return OAL_FAIL;
            }
            hmac_config_set_qap_txop_limit(hmac_vap, &param);
            break;

        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_MSDU_LIFETIME:
            if (value > WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX) {
                return OAL_FAIL;
            }
            mac_mib_set_QAPEDCATableMSDULifetime(hmac_vap, (osal_u8)cfg_stru->ac, value);
            hmac_config_set_qap_msdu_lifetime(hmac_vap, &param);
            break;

        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_MANDATORY:
            if ((value != OAL_TRUE) && (value != OAL_FALSE)) {
                return OAL_FAIL;
            }
            mac_mib_set_qap_edca_table_mandatory(hmac_vap, (osal_u8)cfg_stru->ac, (osal_u8)value);
            break;

        default:
            return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_wmm_params_etc
 功能描述  : 设置频率
 输入参数  : event_hdr: 事件头
             len       : 参数长度
             param    : 参数
 输出参数  : 无
*****************************************************************************/
osal_s32 hmac_config_set_wmm_params(hmac_vap_stru *hmac_vap, hmac_config_wmm_para_stru *cfg_stru)
{
    osal_s32                  ret;
    osal_u32                  ac;
    osal_u32                  value;
    wlan_cfgid_enum_uint16      cfg_id;

    cfg_id    = (osal_u16)cfg_stru->cfg_id;
    ac        = cfg_stru->ac;
    value     = cfg_stru->value;

    if (hmac_vap->mib_info == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_set_wmm_params:mib is null}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log4(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_set_wmm_params::invalid param,cfg_id=%d, ac=%d, value=%d.}",
            hmac_vap->vap_id, cfg_id, ac, value);
        return OAL_FAIL;
    }

    ret = OAL_SUCC;

    switch (cfg_id) {                                                    /* 根据sub-ioctl id填写WID */
        case WLAN_MSG_W2H_CFG_EDCA_TABLE_CWMIN:
            if ((value > WLAN_QEDCA_TABLE_CWMIN_MAX) || (value < WLAN_QEDCA_TABLE_CWMIN_MIN)) {
                return OAL_FAIL;
            }
            mac_mib_set_edca_table_cwmin(hmac_vap, (osal_u8)ac, value);
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_CWMAX:
            if ((value > WLAN_QEDCA_TABLE_CWMAX_MAX) || (value < WLAN_QEDCA_TABLE_CWMAX_MIN)) {
                return OAL_FAIL;
            }
            mac_mib_set_edca_table_cwmax(hmac_vap, (osal_u8)ac, value);
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_AIFSN:
            if ((value < WLAN_QEDCA_TABLE_AIFSN_MIN) || (value > WLAN_QEDCA_TABLE_AIFSN_MAX)) {
                return OAL_FAIL;
            }
            mac_mib_set_edca_table_aifsn(hmac_vap, (osal_u8)ac, value);
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_TXOP_LIMIT:
            if (value > WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX) {
                return OAL_FAIL;
            }
            mac_mib_set_edca_table_txop_limit(hmac_vap, (osal_u8)ac, value);
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_MANDATORY:
            if ((value != OAL_TRUE) && (value != OAL_FALSE)) {
                return OAL_FAIL;
            }
            mac_mib_set_edca_table_mandatory(hmac_vap, (osal_u8)ac, (osal_u8)value);
            break;

        default:
            ret = (osal_s32)hmac_config_set_wmm_qap_params(hmac_vap, cfg_stru);
    }

    /* 改变EDCA参数 uc_wmm_params_update_count++ */
    if ((cfg_id >= WLAN_MSG_W2H_CFG_EDCA_TABLE_CWMIN) && (cfg_id <= WLAN_MSG_W2H_CFG_EDCA_TABLE_MANDATORY)) {
        hmac_vap->uc_wmm_params_update_count++;
    }

    return ret;
}
#endif

OSAL_STATIC osal_void hmac_config_get_wmm_qap_params(hmac_vap_stru *hmac_vap,
    const hmac_config_wmm_para_stru *cfg_stru, osal_u32 *mib_value, osal_u32 *reg_value)
{
    wlan_cfgid_enum_uint16 cfg_id = (osal_u16)cfg_stru->cfg_id;
    osal_u32 ac = cfg_stru->ac;
    osal_u8 cwmin, cwmax, aifsn;
    osal_u16 txop;

    switch (cfg_id) {                                                    /* 根据sub-ioctl id填写WID */
        case WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMIN:
            *mib_value = mac_mib_get_qap_edca_table_cwmin(hmac_vap, (osal_u8)ac);
            hal_vap_get_edca_machw_cw(&cwmax, &cwmin, (osal_u8)ac);
            *reg_value = cwmin;
            break;

        case WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMAX:
            *mib_value = mac_mib_get_qap_edca_table_cwmax(hmac_vap, (osal_u8)ac);
            hal_vap_get_edca_machw_cw(&cwmax, &cwmin, (osal_u8)ac);
            *reg_value = cwmax;
            break;

        case WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_AIFSN:
            *mib_value = mac_mib_get_qap_edca_table_aifsn(hmac_vap, (osal_u8)ac);
            hal_vap_get_machw_aifsn_ac((osal_u8)ac, &aifsn);
            *reg_value = aifsn;
            break;

        case WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_TXOP_LIMIT:
            *mib_value = mac_mib_get_qap_edca_table_txop_limit(hmac_vap, (osal_u8)ac);
            hal_vap_get_machw_txop_limit_ac((osal_u8)ac, &txop);
            *reg_value = txop;
            break;

        case WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_MSDU_LIFETIME:
            *mib_value = mac_mib_get_QAPEDCATableMSDULifetime(hmac_vap, (osal_u8)ac);
            break;

        case WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_MANDATORY:
            *mib_value = mac_mib_get_qap_edca_table_mandatory(hmac_vap, (osal_u8)ac);
            break;

        default:
            break;
    }
}

/*****************************************************************************
 函 数 名  : hmac_config_get_wmm_params
 功能描述  : 读取EDCA参数
 输入参数  : event_hdr: 事件头
 输出参数  : pus_len      : 参数长度
             param    : 参数
*****************************************************************************/
osal_s32 hmac_config_get_wmm_params(hmac_vap_stru *hmac_vap, hmac_config_wmm_para_stru *cfg_stru)
{
    osal_u32 mib_value = 0xFFFFFFFF;
    osal_u32 reg_value = 0xFFFFFFFF;
    wlan_cfgid_enum_uint16 cfg_id = (osal_u16)cfg_stru->cfg_id;
    osal_u32 ac = cfg_stru->ac;

    if (hmac_vap->mib_info == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "vap_id[%d]{hmac_config_get_wmm_params:mib is null}", hmac_vap->vap_id);
        return OAL_FAIL;
    }

    if (ac >= WLAN_WME_AC_BUTT) {
        oam_warning_log3(0, OAM_SF_CFG,
            "vap_id[%d]{hmac_config_get_wmm_params:invalid param, cfg_id=%d, ac=%d}", hmac_vap->vap_id, cfg_id, ac);
        return OAL_FAIL;
    }

    switch (cfg_id) {                                                    /* 根据sub-ioctl id填写WID */
        case WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMIN:
            mib_value = mac_mib_get_edca_table_cwmin(hmac_vap, (osal_u8)ac);
            break;

        case WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMAX:
            mib_value = mac_mib_get_edca_table_cwmax(hmac_vap, (osal_u8)ac);
            break;

        case WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_AIFSN:
            mib_value = mac_mib_get_edca_table_aifsn(hmac_vap, (osal_u8)ac);
            break;

        case WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_TXOP_LIMIT:
            mib_value = mac_mib_get_edca_table_txop_limit(hmac_vap, (osal_u8)ac);
            break;

        case WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_MANDATORY:
            mib_value = mac_mib_get_edca_table_mandatory(hmac_vap, (osal_u8)ac);
            break;

        default:
            hmac_config_get_wmm_qap_params(hmac_vap, cfg_stru, &mib_value, &reg_value);
            break;
    }
    wifi_printf("get_edca_param mib value[%d] reg value[%d]\r\n", mib_value, reg_value);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CFGID_DEBUG
osal_s32 hmac_config_open_wmm_cb(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_bool_enum_uint8 wmm;

    if ((hmac_vap == OSAL_NULL) || (msg->data == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_ANY,
            "{hmac_config_open_wmm_cb::hmac_vap/param is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    wmm = *(oal_bool_enum_uint8 *)msg->data;

    /* 针对配置vap做保护 */
    if (hmac_vap->vap_mode == WLAN_VAP_MODE_CONFIG) {
        oam_warning_log1(0, OAM_SF_CFG,
            "vap_id[%d] {hmac_config_open_wmm_cb::this is config vap! can't get info.}", hmac_vap->vap_id);
        return OAL_FAIL;
    }
    hmac_config_wmm_switch(hmac_vap, wmm);

    return OAL_SUCC;
}
#endif

/*****************************************************************************
 功能描述  : 关闭或者打开WMM，配置WMM相关寄存器
 输入参数  : hmac_device: DMAC VAP
 返 回 值: void
*****************************************************************************/
osal_void hmac_config_set_wmm_register(const hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 wmm)
{
    mac_wme_param_stru *wmm_temp;

    if (hmac_vap->hal_vap == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_set_wmm_register::hal_vap NULL,vap_id:%u.}", hmac_vap->vap_id);
        return;
    }

    wmm_temp = hmac_get_wmm_cfg_etc(hmac_vap->vap_mode);
    /* 关闭wmm */
    if (wmm == OSAL_FALSE) {
        /* 重新设置WMM参数 */
        hmac_config_set_wmm_close_cfg(wmm_temp);
        /* 关WMM，mib信息位中的Qos位置0 */
        mac_mib_set_dot11_qos_option_implemented(hmac_vap, OSAL_FALSE);
    } else {
        /* 重新设置WMM参数 */
        hmac_config_set_wmm_open_cfg(wmm_temp);
        /* 开WMM，mib信息位中的Qos位置1 */
        mac_mib_set_dot11_qos_option_implemented(hmac_vap, OSAL_TRUE);
    }
}

/*****************************************************************************
 功能描述  : 打开/关闭 WMM
 输入参数  : mac_vap:mac vap
 返 回 值: osal_u32
*****************************************************************************/
osal_void hmac_config_wmm_switch(hmac_vap_stru *hmac_vap, oal_bool_enum_uint8 wmm)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    osal_u8 vap_idx;
    hmac_vap_stru *vap_tmp;

    /* 配置状态没有变化，不再进行后续流程 */
    if (wmm == hmac_device->wmm) {
        return;
    }
    /* 设置dev中的wmm_en，使能或者关闭4通道 */
    hmac_device->wmm = wmm;

    /* 遍历所有vap */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        vap_tmp = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (vap_tmp == OSAL_NULL) {
            oam_error_log2(0, OAM_SF_CFG, "vap_id[%d] {hmac_config_wmm_switch::hmac_vap(%d) null.}", hmac_vap->vap_id,
                           hmac_device->vap_id[vap_idx]);
            continue;
        }
        hmac_config_set_wmm_register(vap_tmp, wmm);
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_machw_wmm
 功能描述  : 设置硬件wmm参数
*****************************************************************************/
osal_void  hmac_config_set_machw_wmm(const hmac_vap_stru *hmac_vap)
{
    wlan_wme_ac_type_enum_uint8     ac_type;
    /* 设置AIFSN */
    hal_vap_set_machw_aifsn_all_ac((osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BK),
                                   (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_BE),
                                   (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VI),
                                   (osal_u8)mac_mib_get_qap_edca_table_aifsn(hmac_vap, WLAN_WME_AC_VO));

    /* cwmin cwmax */
    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        hal_vap_set_edca_machw_cw((osal_u8)mac_mib_get_qap_edca_table_cwmax(hmac_vap, ac_type),
                                  (osal_u8)mac_mib_get_qap_edca_table_cwmin(hmac_vap, ac_type),
                                  ac_type);
    }

    /* txop */
    hal_vap_set_machw_txop_limit_bkbe((osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BE),
                                      (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_BK));
    hal_vap_set_machw_txop_limit_vivo((osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_VO),
                                      (osal_u16)mac_mib_get_qap_edca_table_txop_limit(hmac_vap, WLAN_WME_AC_VI));
}

/*****************************************************************************
 函 数 名  : hmac_set_wmm_open_cfg
 功能描述  : 打开WMM，配置WMM寄存器
*****************************************************************************/
osal_void  hmac_config_set_wmm_open_cfg(const mac_wme_param_stru  *wmm)
{
    wlan_wme_ac_type_enum_uint8     ac_type;

    if (wmm == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_wmm_open_cfg::param null.}");

        return;
    }
    for (ac_type = 0; ac_type < WLAN_WME_AC_BUTT; ac_type++) {
        hal_vap_set_edca_machw_cw((osal_u8)(wmm[ac_type].logcwmax), (osal_u8)(wmm[ac_type].logcwmin), ac_type);

        hal_vap_set_machw_aifsn_ac(ac_type, (osal_u8)wmm[ac_type].aifsn);
    }

    hal_vap_set_machw_txop_limit_bkbe((osal_u16)wmm[WLAN_WME_AC_BE].txop_limit,
        (osal_u16)wmm[WLAN_WME_AC_BK].txop_limit);
    hal_vap_set_machw_txop_limit_vivo((osal_u16)wmm[WLAN_WME_AC_VO].txop_limit,
        (osal_u16)wmm[WLAN_WME_AC_VI].txop_limit);
}

/*****************************************************************************
 函 数 名  : hmac_config_set_wmm_close_cfg
 功能描述  : 关闭WMM，配置WMM寄存器，注意，硬MAC在关闭WMM条件下使用VO发送后续帧，
             而张炜要求使用BE配置参数进行数据发送，折衷一下，使用BE配置VO
*****************************************************************************/
osal_void  hmac_config_set_wmm_close_cfg(const mac_wme_param_stru *wmm)
{
    if (wmm == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hmac_config_set_wmm_close_cfg::param null.}");

        return;
    }
    hal_vap_set_edca_machw_cw((osal_u8)wmm[WLAN_WME_AC_BE].logcwmax, (osal_u8)wmm[WLAN_WME_AC_BE].logcwmin,
        WLAN_WME_AC_VO);
    hal_vap_set_machw_aifsn_ac(WLAN_WME_AC_VO, (osal_u8)wmm[WLAN_WME_AC_BE].aifsn);
    hal_vap_set_machw_txop_limit_vivo((osal_u16)wmm[WLAN_WME_AC_BE].txop_limit,
        (osal_u16)wmm[WLAN_WME_AC_VI].txop_limit);
}

OSAL_STATIC osal_void hmac_set_ac_mode_cfg(wlan_wme_ac_type_enum_uint8 mode)
{
    if (mode == WLAN_WME_AC_BUTT) {
        g_set_ac_mode_enable = OAL_FALSE;
        oam_warning_log0(0, OAM_SF_CFG, "service api :ac mode has been reset.");
        return;
    }

    g_set_ac_mode_enable = OAL_TRUE;
    g_set_ac_mode = mode;
    oam_warning_log2(0, OAM_SF_CFG, "service api :ac mode %d,status %d.", g_set_ac_mode, g_set_ac_mode_enable);
    return;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_ac_mode
 功能描述  : 设置AC类型
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_set_ac_mode(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    wlan_wme_ac_type_enum_uint8 mode = *(wlan_wme_ac_type_enum_uint8 *)(msg->data);
    unref_param(hmac_vap);
    hmac_set_ac_mode_cfg(mode);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_set_ac_mode
 功能描述  : 设置AC类型
*****************************************************************************/
WIFI_HMAC_TCM_TEXT WIFI_TCM_TEXT osal_void hmac_set_ac_mode(mac_tx_ctl_stru *tx_ctl)
{
    if (g_set_ac_mode_enable != OAL_TRUE) {
        return;
    }
    if (mac_get_cb_is_vipframe(tx_ctl) == OAL_TRUE) {
        return;
    }
    mac_get_cb_wme_ac_type(tx_ctl)   = g_set_ac_mode;
    mac_get_cb_wme_tid_type(tx_ctl)  = wlan_wme_ac_to_tid(g_set_ac_mode);
    return;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OAL_STATIC osal_s32 hmac_ccpriv_set_edca_param(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 value;
    hmac_config_wmm_para_stru edca_cfg;
    osal_u32 cfg_max, cfg_min, cmd_idx;
    osal_u32 has_find = OSAL_FALSE;
    osal_s32 ret;

    memset_s(&edca_cfg, sizeof(edca_cfg), 0, sizeof(edca_cfg));

    /* 获取命令关键字 */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        return ret;
    }

    for (cmd_idx = 0; cmd_idx < OAL_ARRAY_SIZE(edca_param_table); cmd_idx++) {
        if (osal_strcmp(edca_param_table[cmd_idx].cmd_name, (const osal_char *)name) == 0) {
            edca_cfg.cfg_id = edca_param_table[cmd_idx].set_cfg_id;
            cfg_max = edca_param_table[cmd_idx].cfg_max;
            cfg_min = edca_param_table[cmd_idx].cfg_min;
            oam_warning_log2(0, OAM_SF_EDCA, "{hmac_ccpriv_set_edca_param::max[%d] min[%d]", cfg_max, cfg_min);
            has_find = OSAL_TRUE;
            break;
        }
    }
    if (has_find != OSAL_TRUE) {
        oam_error_log0(0, OAM_SF_EDCA, "{hmac_ccpriv_set_edca_param::input edca type error.}");
        return OAL_FAIL;
    }

    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_WME_AC_BE, WLAN_WME_AC_VO, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_EDCA, "{hmac_ccpriv_set_edca_param::set ac error,return.}");
        return OAL_FAIL;
    }
    edca_cfg.ac = (osal_u32)value;

    /* 解析value size: param num should be 0~cfg_max */
    ret = hmac_ccpriv_get_digit_with_check_max(&param, cfg_max, &edca_cfg.value);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_EDCA, "{hmac_ccpriv_set_edca_param::param out of range[max:%d].}", cfg_max);
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_EDCA, "{hmac_ccpriv_set_edca_param::cfg_id[%d] AC[%d] value[%d]}",
        edca_cfg.cfg_id, edca_cfg.ac, edca_cfg.value);

    ret = hmac_config_set_wmm_params(hmac_vap, &edca_cfg);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_EDCA, "{hmac_ccpriv_set_edca_param::return err code[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

OAL_STATIC osal_s32 hmac_ccpriv_get_edca_param(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_s32 value;
    hmac_config_wmm_para_stru edca_cfg;
    osal_u32 cmd_idx;
    osal_u32 has_find = OSAL_FALSE;
    osal_s32 ret;

    memset_s(&edca_cfg, sizeof(edca_cfg), 0, sizeof(edca_cfg));

    /* 获取命令关键字 */
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        return ret;
    }

    for (cmd_idx = 0; cmd_idx < OAL_ARRAY_SIZE(edca_param_table); cmd_idx++) {
        if (osal_strcmp(edca_param_table[cmd_idx].cmd_name, (osal_char *)name) == 0) {
            edca_cfg.cfg_id = edca_param_table[cmd_idx].get_cfg_id;
            has_find = OSAL_TRUE;
            break;
        }
    }
    if (has_find != OSAL_TRUE) {
        oam_warning_log0(0, OAM_SF_EDCA, "{hmac_ccpriv_get_edca_param::input edca type error.}");
        return OAL_FAIL;
    }

    ret = hmac_ccpriv_get_digit_with_range(&param, WLAN_WME_AC_BE, WLAN_WME_AC_VO, &value);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_EDCA, "{hmac_ccpriv_get_edca_param::get ac error,return.}");
        return OAL_FAIL;
    }
    edca_cfg.ac = (osal_u32)value;

    oam_warning_log2(0, OAM_SF_EDCA, "{hmac_ccpriv_get_edca_param::cfg_id[%d] AC[%d]}", edca_cfg.cfg_id, edca_cfg.ac);

    ret = hmac_config_get_wmm_params(hmac_vap, &edca_cfg);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_EDCA, "{hmac_ccpriv_get_edca_param::return err code[%d]!}", ret);
        return ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************************
 功能描述  : 支持强制报文走VI/VO/BK/BE close关闭
 使用方法  : echo "wlan0 set_ac_mode <ac_mode>"
******************************************************************************************/
OAL_STATIC osal_s32 hmac_ccpriv_set_ac_mode(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32                     ret;
    osal_u8                      index;
    wlan_wme_ac_type_enum_uint8  mode = 0;
    osal_s8    name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    OAL_CONST struct {
        osal_char *ac_param;
        wlan_wme_ac_type_enum_uint8 ac_param_mode;
    } ac_mode_array[] = {
        {"VO", WLAN_WME_AC_VO},
        {"VI", WLAN_WME_AC_VI},
        {"BE", WLAN_WME_AC_BE},
        {"BK", WLAN_WME_AC_BK},
        {"close", WLAN_WME_AC_BUTT}
    };
    unref_param(hmac_vap);
    ret = hmac_ccpriv_get_one_arg(&param, name, OAL_SIZEOF(name));
    if (ret != OAL_SUCC) {
        return ret;
    }

    for (index = 0; index < OAL_ARRAY_SIZE(ac_mode_array); index++) {
        if (osal_strcmp((osal_char *)name, ac_mode_array[index].ac_param) == 0) {
            mode = ac_mode_array[index].ac_param_mode;
            break;
        }
    }

    if (index == OAL_ARRAY_SIZE(ac_mode_array)) {
        wifi_printf("hmac_ccpriv_set_ac_mode::parameter error!\r\n");
        return OAL_FAIL;
    }

    hmac_set_ac_mode_cfg(mode);

    return OAL_SUCC;
}
#endif

osal_u32 hmac_wmm_init(osal_void)
{
    frw_msg_hook_register(WLAN_MSG_W2H_CFG_SET_AC_MODE, hmac_config_set_ac_mode);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_register((const osal_s8 *)"set_edca_params", hmac_ccpriv_set_edca_param);
    hmac_ccpriv_register((const osal_s8 *)"get_edca_params", hmac_ccpriv_get_edca_param);
    hmac_ccpriv_register((const osal_s8 *)"set_ac_mode", hmac_ccpriv_set_ac_mode);
#endif
    return OAL_SUCC;
}

osal_void hmac_wmm_deinit(osal_void)
{
    frw_msg_hook_unregister(WLAN_MSG_W2H_CFG_SET_AC_MODE);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    hmac_ccpriv_unregister((const osal_s8 *)"set_edca_params");
    hmac_ccpriv_unregister((const osal_s8 *)"get_edca_params");
    hmac_ccpriv_unregister((const osal_s8 *)"set_ac_mode");
#endif
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
