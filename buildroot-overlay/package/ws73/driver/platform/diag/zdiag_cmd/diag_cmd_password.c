/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag securec connect cmd.
 * This file should be changed only infrequently and with great care.
 */

#include "diag_cmd_password.h"
#include "securec.h"
#include "soc_zdiag.h"
#include "soc_diag_cmd_id.h"
#include "soc_diag_cmd.h"
#include "zdiag_adapt_layer.h"

TD_PRV ext_errno diag_cmd_password_pub_a(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    diag_cmd_conn_rsp_pub_b_stru conn_rsp_pub_b;
    msp_diag_ack_param ack;
    memset_s(&conn_rsp_pub_b, sizeof(diag_cmd_conn_rsp_pub_b_stru), 0, sizeof(diag_cmd_conn_rsp_pub_b_stru));

    conn_rsp_pub_b.ret_ind.err_no = EXT_DIAG_CONN_ERR_OK;
    uapi_unused(cmd_id);
    uapi_unused(cmd_param);
    uapi_unused(cmd_param_size);
    ack.sn = 0;
    ack.ctrl = 0;
    ack.cmd_id = DIAG_CMD_CONNECT_RANDOM;
    ack.param_size = (td_u16)sizeof(diag_cmd_conn_rsp_pub_b_stru);
    ack.param = (td_u8 *)&conn_rsp_pub_b;
    return uapi_zdiag_report_ack(&ack, option);
}

TD_PRV ext_errno diag_cmd_password_m_check(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size,
    diag_option *option)
{
    diag_cmd_conn_rsp_m_stru conn_rsp_m = { 0 };
    msp_diag_ack_param ack;
    memset_s(&conn_rsp_m, sizeof(diag_cmd_conn_rsp_m_stru), 0, sizeof(diag_cmd_conn_rsp_m_stru));

    uapi_unused(cmd_id);
    uapi_unused(cmd_param);
    uapi_unused(cmd_param_size);

    conn_rsp_m.ret = EXT_ERR_SUCCESS;
    conn_rsp_m.ret_ind.err_no = EXT_DIAG_CONN_ERR_OK;
    ack.sn = 0;
    ack.ctrl = 0;
    ack.cmd_id = DIAG_CMD_CONNECT_M_CHECK;
    ack.param_size = (td_u16)sizeof(diag_cmd_conn_rsp_m_stru);
    ack.param = (td_u8 *)&conn_rsp_m;
    return uapi_zdiag_report_ack(&ack, option);
}


ext_errno diag_cmd_password(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    zdiag_printf("diag_cmd_password\r\n");
    switch (cmd_id) {
        case DIAG_CMD_CONNECT_RANDOM:
            return diag_cmd_password_pub_a(cmd_id, cmd_param, cmd_param_size, option);
        case DIAG_CMD_CONNECT_M_CHECK:
            return diag_cmd_password_m_check(cmd_id, cmd_param, cmd_param_size, option);
        default:
            return EXT_ERR_NOT_SUPPORT;
    }
}
