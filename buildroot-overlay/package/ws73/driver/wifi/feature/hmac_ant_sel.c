/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: ANT_SEL hmac function.
 * Create: 2023-3-6
 */

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_ant_sel.h"
#include "hal_ant_sel_rom.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"
#include "msg_ant_sel_rom.h"
#include "wlan_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_ANT_SEL_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define MAX_ANT_SEL_CONFIG_NUM 4

/*****************************************************************************
 函 数 名  : hmac_ccpriv_set_ant_sel_control_type
 功能描述  : 配置天线分集参数: 1 表示来自mac控制, 0 表示来自phy寄存器控制
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_set_ant_sel_control_type(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 ant_sel;
    osal_s8 ac_arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};

    unref_param(hmac_vap);

    /* 获取第一个参数 */
    ret = hmac_ccpriv_get_one_arg(&param, ac_arg, OAL_SIZEOF(ac_arg));
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_ant_sel_param::get first param fail}");
        return ret;
    }

    ant_sel = (osal_u8)oal_atoi((const osal_s8 *)ac_arg);
    /* 多天线模式下单天线模式选择,cfg_ant_sel_mux的优先级大于cfg_one_ant_sel_man */
    if (ant_sel == HMAC_ANT_SEL_CONTROL_FROM_PHY) {
        hal_set_ant_sel_phy_enable(0);  /* 0: 来自phy处理后的多天线分集选择 */
        hal_set_ant_sel_phy_cfg_man(1); /* 1: 来自phy寄存器配置cfg_one_ant_sel */
    } else if (ant_sel == HMAC_ANT_SEL_CONTROL_FROM_MAC) {
        hal_set_ant_sel_phy_enable(1);  /* 1: 来自mac直通的多天线分集选择 */
        hal_set_ant_sel_phy_cfg_man(0); /* 0: 来自mac控制 */
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_ant_sel_param::invalid param}");
        return OAL_FAIL;
    }
    return ret;
}


/*****************************************************************************
 函 数 名  : hmac_ccpriv_ant_sel_set_tx_dscr
 功能描述  : mac控制：mac描述符控制天线通道 ,参数为 0 1 2 3
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_ant_sel_set_tx_dscr(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 ant_sel;
    osal_s8 ac_arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8 phy_enable, phy_cfg_man;
    mac_cfg_ant_sel_param_stru event_cfg_ant_sel;
    frw_msg msg_info;

    unref_param(hmac_vap);
    phy_enable = (osal_u8)hal_get_ant_sel_phy_enable();
    phy_cfg_man = (osal_u8)hal_get_ant_sel_phy_cfg_man();
    if (phy_enable != OSAL_TRUE || phy_cfg_man != OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_ant_sel_set_tx_dscr::disable for config mac ant_sel!}\n");
        return OAL_FAIL;
    }

    /* 获取第一个参数 */
    ret = hmac_ccpriv_get_one_arg(&param, ac_arg, OAL_SIZEOF(ac_arg));
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{hmac_ccpriv_set_ant_sel_param::get first param fail}");
        return ret;
    }

    /* ant_sel 取值 0 1 2 3 */
    ant_sel = (osal_u8)oal_atoi((const osal_s8 *)ac_arg);
    if (ant_sel >= MAX_ANT_SEL_CONFIG_NUM) {
        oam_warning_log0(0, OAM_SF_ANY, "hmac_ccpriv_ant_sel_set_tx_dscr::invalid parameter");
        return OAL_FAIL;
    }

    (osal_void)memset_s(&event_cfg_ant_sel, sizeof(mac_cfg_ant_sel_param_stru), 0, sizeof(mac_cfg_ant_sel_param_stru));
    event_cfg_ant_sel.cfg_type = 0;  /* 0: TX_DSCR */
    event_cfg_ant_sel.value = ant_sel;
    frw_msg_init((osal_u8 *)&event_cfg_ant_sel, sizeof(osal_u32), OSAL_NULL, 0, &msg_info);
    ret = frw_send_msg_to_device(hmac_vap->vap_id, WLAN_MSG_H2D_C_CFG_MAC_ANT_SEL_TX_DSCR, &msg_info, OSAL_TRUE);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "hmac_ccpriv_ant_sel_set_tx_dscr::send msg fail[%d]", ret);
    }
    return ret;
}

osal_u32 hmac_ant_sel_init(osal_void)
{
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"set_ant_sel_control_type", hmac_ccpriv_set_ant_sel_control_type);
    hmac_ccpriv_register((const osal_s8 *)"ant_sel_set_tx_dscr", hmac_ccpriv_ant_sel_set_tx_dscr);
    return OAL_SUCC;
}

osal_void hmac_ant_sel_deinit(osal_void)
{
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"set_ant_sel_control_type");
    hmac_ccpriv_unregister((const osal_s8 *)"ant_sel_set_tx_dscr");

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* #ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD */