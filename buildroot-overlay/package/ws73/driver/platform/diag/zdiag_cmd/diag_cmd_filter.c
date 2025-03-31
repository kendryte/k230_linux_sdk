/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag filter
 * This file should be changed only infrequently and with great care.
 */
#include "diag_cmd_filter.h"
#include "securec.h"
#include "soc_zdiag.h"
#include "soc_diag_cmd_id.h"
#include "soc_diag_cmd.h"
#include "zdiag_adapt_layer.h"
#include "diag_adapt_layer.h"
#include "zdiag_filter.h"
#include "zdiag_adapt_os.h"

TD_PRV ext_errno diag_cmd_filter_set_id(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    size_t i;
    diag_cmd_msg_cfg_req_stru *cfg = cmd_param;
    size_t cnt = (size_t)cmd_param_size / sizeof(diag_cmd_msg_cfg_req_stru);
    for (i = 0; i < cnt; i++) {
        zdiag_set_id_enable(cfg->module_id, cfg->switch_code);
        cfg++;
    }

    uapi_unused(cmd_id);
    uapi_unused(option);
    return EXT_ERR_SUCCESS;
}

TD_PRV ext_errno diag_cmd_filter_set_level(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size,
    diag_option *option)
{
    size_t i;
    diag_cmd_msg_cfg_req_stru *cfg = cmd_param;
    size_t cnt = (size_t)cmd_param_size / sizeof(diag_cmd_msg_cfg_req_stru);
    for (i = 0; i < cnt; i++) {
        zdiag_set_level_enable(cfg->module_id, cfg->switch_code);
        cfg++;
    }
    uapi_unused(cmd_id);
    uapi_unused(option);
    return EXT_ERR_SUCCESS;
}

ext_errno diag_cmd_filter_set(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    TD_ASSERT(cmd_param);
    switch (cmd_id) {
        case DIAG_CMD_MSG_CFG_SET_SYS:
        case DIAG_CMD_MSG_CFG_SET_DEV:
        case DIAG_CMD_MSG_CFG_SET_USR:
        case DIAG_CMD_MSG_CFG_SET_AIR:
            if (diag_hcc_sync_cmd_to_dev(cmd_id, cmd_param, cmd_param_size, option) != EXT_ERR_SUCCESS) {
                zdiag_print("diag sync cmd to dev fail, cmd_id=0x%x\r\n", cmd_id);
            }

            return diag_cmd_filter_set_id(cmd_id, cmd_param, cmd_param_size, option);
        case DIAG_CMD_MSG_CFG_SET_LEVEL:
            if (diag_hcc_sync_cmd_to_dev(cmd_id, cmd_param, cmd_param_size, option) != EXT_ERR_SUCCESS) {
                zdiag_print("diag sync cmd to dev fail, cmd_id=0x%x\r\n", cmd_id);
            }

            return diag_cmd_filter_set_level(cmd_id, cmd_param, cmd_param_size, option);
        default:
            return EXT_ERR_NOT_SUPPORT;
    }
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "soc_module.h"

#define ZDIAG_FILTER_MAX_LEVEL 8
#define EXT_DIAG_MODULE_SYS 5    /* Number of types that can be configured for the SYS MSG */

void diag_enable_all_switch(void)
{
    td_u16 i;
    td_u8 diag_module_sys[EXT_DIAG_MODULE_SYS] = {
        DIAG_MOD_ID_WIFI_HOST, DIAG_MOD_ID_WIFI_DEVICE, DIAG_MOD_ID_OAM_HOST, SOC_ID_BSP, SOC_ID_SYS};

    zdiag_set_enable(TD_TRUE, ACORE_HSO_ADDR);
    for (i = 0; i < ZDIAG_FILTER_MAX_LEVEL; ++i) {
        zdiag_set_level_enable(i, TD_TRUE);
    }

    for (i = 0; i < EXT_DIAG_MODULE_SYS; ++i) {
        zdiag_set_id_enable(diag_module_sys[i], TD_TRUE);
    }
}

void diag_enable_level_switch(td_u32 level)
{
    td_u16 i;
    td_u8 diag_module_sys[EXT_DIAG_MODULE_SYS] = {
        DIAG_MOD_ID_WIFI_HOST, DIAG_MOD_ID_WIFI_DEVICE, DIAG_MOD_ID_OAM_HOST, SOC_ID_BSP, SOC_ID_SYS};

    zdiag_set_enable(TD_TRUE, ACORE_HSO_ADDR);
    for (i = 0; i < uapi_min(level, ZDIAG_FILTER_MAX_LEVEL); ++i) {
        zdiag_set_level_enable(i, TD_TRUE);
    }

    for (i = 0; i < EXT_DIAG_MODULE_SYS; ++i) {
        zdiag_set_id_enable(diag_module_sys[i], TD_TRUE);
    }
}
#endif
