/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag connect cmd.
 * This file should be changed only infrequently and with great care.
 */

#include "diag_cmd_connect.h"
#include "soc_zdiag.h"
#include "soc_diag_cmd_id.h"
#include "soc_diag_cmd.h"
#include "diag_adapt_layer.h"
#include "zdiag_filter.h"
#ifdef __KERNEL__
#include "zdiag_adapt_os.h"
#endif

#define DIAG_TEMP_MAGIC_1 0xFF

#ifdef HOST_SOCKET_DIAG
volatile td_bool g_hso_connect_status = 0;
td_bool get_hso_connect_status(td_void)
{
    return g_hso_connect_status;
}
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
volatile td_bool g_hso_support_dis_cmd = TD_TRUE;

td_void diag_dis_cmd_support_or_not(td_bool is_supported)
{
    g_hso_support_dis_cmd = is_supported;
}
#endif

TD_PRV ext_errno diag_cmd_hso_connect(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    td_u16 product_type = 1;
    td_u16 hso_dev_type = 0x0001;
    zdiag_addr tool_addr;
    diag_cmd_host_connect_cnf_stru cnf = { 0 };
    msp_diag_ack_param ack;
    ext_errno ret;

    cnf.imei[0] = 1;                 /* 0:item subscript */
    cnf.imei[1] = DIAG_TEMP_MAGIC_1; /* 1:item subscript */
    cnf.imei[2] = DIAG_TEMP_MAGIC_1; /* 2:item subscript */
    cnf.imei[3] = DIAG_TEMP_MAGIC_1; /* 3:item subscript */
    cnf.imei[4] = DIAG_TEMP_MAGIC_1; /* 4:item subscript */
    cnf.imei[5] = DIAG_TEMP_MAGIC_1; /* 5:item subscript */
    cnf.imei[6] = 0;                 /* 6:item subscript */
    cnf.imei[7] = 0;                 /* 7:item subscript */
    cnf.imei[8] = 1;                 /* 8:item subscript */
    cnf.imei[9] = 0;                 /* 9:item subscript */
    cnf.imei[10] = 1;                /* 10:item subscript */
    cnf.imei[11] = 0;                /* 11:item subscript */

    ack.sn = 0;
    ack.ctrl = 0;
    ack.cmd_id = DIAG_CMD_HOST_CONNECT;
    ack.param_size = (td_u16)sizeof(diag_cmd_host_connect_cnf_stru);
    ack.param = (td_u8 *)&cnf;

    uapi_unused(cmd_id);
    uapi_unused(cmd_param);
    uapi_unused(cmd_param_size);
    uapi_unused(hso_dev_type);
    uapi_unused(product_type);
    tool_addr = option->peer_addr;

    zdiag_set_enable(TD_TRUE, tool_addr);
    ret = uapi_zdiag_report_ack(&ack, option);
#ifdef HOST_SOCKET_DIAG
    g_hso_connect_status = 1;
#endif
    return ret;
}

TD_PRV ext_errno diag_cmd_host_disconn(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    uapi_unused(cmd_id);
    uapi_unused(cmd_param);
    uapi_unused(cmd_param_size);
    uapi_unused(option);
    zdiag_set_enable(TD_FALSE, option->peer_addr);
#ifdef HOST_SOCKET_DIAG
    g_hso_connect_status = 0;
#endif
    return EXT_ERR_NOT_SUPPORT;
}

ext_errno diag_cmd_hso_connect_disconnect(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    zdiag_print("diag run cmd_id =0x%x\r\n", cmd_id);
    switch (cmd_id) {
        case DIAG_CMD_HOST_CONNECT:
            if (diag_hcc_sync_cmd_to_dev(cmd_id, cmd_param, cmd_param_size, option) != EXT_ERR_SUCCESS) {
                zdiag_print("diag sync cmd to dev fail, cmd_id=0x%x\r\n", cmd_id);
            }
            return diag_cmd_hso_connect(cmd_id, cmd_param, cmd_param_size, option);
        case DIAG_CMD_HOST_DISCONNECT:
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            if (g_hso_support_dis_cmd == TD_FALSE) {
                return EXT_ERR_SUCCESS;
            }
#endif
            if (diag_hcc_sync_cmd_to_dev(cmd_id, cmd_param, cmd_param_size, option) != EXT_ERR_SUCCESS) {
                zdiag_print("diag sync cmd to dev fail, cmd_id=0x%x\r\n", cmd_id);
            }
            return diag_cmd_host_disconn(cmd_id, cmd_param, cmd_param_size, option);
        default:
            return EXT_ERR_NOT_SUPPORT;
    }
}
