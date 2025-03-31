/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag cmd process
 * This file should be changed only infrequently and with great care.
 */

#include "zdiag_cmd_dst.h"
#include "soc_zdiag.h"
#include "zdiag_config.h"
#include "zdiag_adapt_layer.h"
#include "dfx_fault_event.h"

typedef struct {
    const td_zdiag_cmd_reg_obj *user_cmd_list[CONFIG_DIAG_CMD_TBL_NUM];
    td_u16 aus_usert_cmd_num[CONFIG_DIAG_CMD_TBL_NUM]; /* cmd obj num */
} diag_cmd_ctrl;

TD_PRV diag_cmd_ctrl g_diag_cmd_ctrl;

TD_PRV inline diag_cmd_ctrl *diag_get_cmd_ctrl(td_void)
{
    return &g_diag_cmd_ctrl;
}

TD_PRV ext_errno diag_cmd_tbl_check(TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl, td_u16 cmd_num)
{
    uapi_unused(cmd_tbl);
    uapi_unused(cmd_num);

    return EXT_ERR_SUCCESS;
}

TD_PRV td_zdiag_cmd_f diag_find_usr_cmd_proc_func(td_u32 cmd_id)
{
    diag_cmd_ctrl *cmd_ctrl = diag_get_cmd_ctrl();
    td_u32 i;
    td_u16 k;
    for (i = 0; i < CONFIG_DIAG_CMD_TBL_NUM; i++) {
        for (k = 0; k < cmd_ctrl->aus_usert_cmd_num[i]; k++) {
            TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl = cmd_ctrl->user_cmd_list[i];
            TD_CONST td_zdiag_cmd_reg_obj *cmd_list = &cmd_tbl[k];

            if ((cmd_id >= cmd_list->min_id) && (cmd_id <= cmd_list->max_id)) {
                td_zdiag_cmd_f cmd = (td_zdiag_cmd_f)cmd_list->fn_input_cmd;
                return cmd;
            }
        }
    }

    return TD_NULL;
}

ext_errno uapi_zdiag_register_cmd(TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl, td_u16 cmd_num)
{
    ext_errno ret;
    td_u32 lock_stat;
    int i;

    diag_cmd_ctrl *cmd_ctrl = diag_get_cmd_ctrl();
    lock_stat = diag_adapt_int_lock();
    ret = diag_cmd_tbl_check(cmd_tbl, cmd_num);
    if (ret != EXT_ERR_SUCCESS) {
#ifdef _PRE_WLAN_PLAT_URANUS
        dfx_fault_event_data(FAULT_DFX_DIAG_REGISTER_CMD_FAIL, TD_NULL, 0);
#endif
        goto end;
    }

    for (i = 0; i < CONFIG_DIAG_CMD_TBL_NUM; i++) {
        if ((cmd_ctrl->user_cmd_list[i] == TD_NULL) || (cmd_ctrl->aus_usert_cmd_num[i] == 0)) {
            ret = EXT_ERR_SUCCESS;
            cmd_ctrl->user_cmd_list[i] = cmd_tbl;
            cmd_ctrl->aus_usert_cmd_num[i] = cmd_num;
            goto end;
        }
    }
    ret = EXT_ERR_FAILURE;
end:
#ifdef _PRE_WLAN_PLAT_URANUS
    dfx_fault_event_data(FAULT_DFX_DIAG_REGISTER_CMD_FAIL, TD_NULL, 0);
#endif
    diag_adapt_int_restore(lock_stat);
    return ret;
}

ext_errno diag_pkt_router_run_cmd(msp_diag_head_req_stru *req, diag_option *option)
{
    td_zdiag_cmd_f cmd_f = diag_find_usr_cmd_proc_func(req->cmd_id);
    if (cmd_f) {
        diag_option temp_option = *option;
        cmd_f(req->cmd_id, req->param, req->param_size, &temp_option);
        return EXT_ERR_SUCCESS;
    }
    return EXT_ERR_FAILURE;
}
