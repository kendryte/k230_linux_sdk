/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag ind process
 * This file should be changed only infrequently and with great care.
 */

#include "zdiag_ind_dst.h"
#include "soc_zdiag.h"
#include "zdiag_adapt_layer.h"

typedef struct {
    TD_CONST td_zdiag_cmd_reg_obj *user_cmd_list[CONFIG_DIAG_IND_TBL_NUM];
    td_u16 aus_usert_cmd_num[CONFIG_DIAG_IND_TBL_NUM]; /* cmd obj num */
} diag_ind_ctrl;

TD_PRV diag_ind_ctrl g_diag_ind_ctrl;

TD_PRV inline diag_ind_ctrl *diag_get_ind_ctrl(td_void)
{
    return &g_diag_ind_ctrl;
}

TD_PRV td_zdiag_cmd_f diag_find_usr_ind_proc_func(td_u32 cmd_id)
{
    diag_ind_ctrl *cmd_ctrl = diag_get_ind_ctrl();
    td_u32 i;
    td_u16 k;
    for (i = 0; i < CONFIG_DIAG_IND_TBL_NUM; i++) {
        for (k = 0; k < cmd_ctrl->aus_usert_cmd_num[i]; k++) {
            TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl = cmd_ctrl->user_cmd_list[i];
            TD_CONST td_zdiag_cmd_reg_obj *cmd_list = &cmd_tbl[k];

            if ((cmd_id >= cmd_list->min_id) && (cmd_id <= cmd_list->max_id)) {
                td_zdiag_cmd_f cmd;
                cmd = (td_zdiag_cmd_f)cmd_list->fn_input_cmd;
                return cmd;
            }
        }
    }
    return TD_NULL;
}

ext_errno diag_pkt_router_run_ind(msp_diag_head_ind_stru *ind_head, diag_option *option)
{
    diag_option new_option;
    td_zdiag_cmd_f cmd_f = diag_find_usr_ind_proc_func(ind_head->cmd_id);
    if (cmd_f) {
        new_option = *option;
        cmd_f(ind_head->cmd_id, ind_head->param, ind_head->param_size, &new_option);
        return EXT_ERR_SUCCESS;
    }

    return EXT_ERR_FAILURE;
}

TD_PRV ext_errno diag_ind_tbl_check(TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl, td_u16 cmd_num)
{
    uapi_unused(cmd_tbl);
    uapi_unused(cmd_num);

    return EXT_ERR_SUCCESS;
}

ext_errno uapi_zdiag_register_ind(TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl, td_u16 cmd_num)
{
    ext_errno ret;
    td_u32 lock_stat;
    int i;
    diag_ind_ctrl *cmd_ctrl = diag_get_ind_ctrl();

    lock_stat = diag_adapt_int_lock();
    ret = diag_ind_tbl_check(cmd_tbl, cmd_num);
    if (ret != EXT_ERR_SUCCESS) {
        goto end;
    }

    for (i = 0; i < CONFIG_DIAG_IND_TBL_NUM; i++) {
        if ((cmd_ctrl->user_cmd_list[i] == TD_NULL) || (cmd_ctrl->aus_usert_cmd_num[i] == 0)) {
            cmd_ctrl->user_cmd_list[i] = cmd_tbl;
            cmd_ctrl->aus_usert_cmd_num[i] = cmd_num;
            ret = EXT_ERR_SUCCESS;
            goto end;
        }
    }
    ret = EXT_ERR_FAILURE;
end:
    diag_adapt_int_restore(lock_stat);
    return ret;
}
