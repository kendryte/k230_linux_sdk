/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: diag default cmd tbl.
 * This file should be changed only infrequently and with great care.
 */

#include "diag_cmd_default_tbl.h"
#if !defined DIAG_CONFIG_FILE_INCLUDE
#error "should include zdiag_config.h"
#endif
#include "soc_zdiag.h"
#include "soc_diag_cmd_id.h"
#include "diag_cmd_password.h"
#include "diag_cmd_connect.h"
#include "diag_cmd_mem_operate.h"
#if defined(CONFIG_SUPPORT_SAMPLE_DATA)
#include "diag_sample_data.h"
#endif
#if defined(CONFIG_SUPPORT_MOCKED_SHELL)
#include "diag_cmd_shell.h"
#endif
#if defined(CONFIG_DFX_SUPPORT_DIAG_UART_CHANNEL)
#if CONFIG_DFX_SUPPORT_DIAG_UART_CHANNEL == DFX_YES
#include "zdiag_uart.h"
#endif
#endif
#include "diag_cmd_filter.h"
#include "diag_cmd_psd.h"
#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_DFX) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_DFX == DFX_YES)
#include "zdiag_dfx.h"
#endif
#include "diag_cmd_mem_read_write.h"

td_zdiag_cmd_reg_obj g_diag_default_cmd_tbl[] = {
#if defined(CONFIG_DFX_SUPPORT_DIAG_CONNECT_DIRECTLY_ROLE) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CONNECT_DIRECTLY_ROLE == DFX_YES)
    {DIAG_CMD_CONNECT_RANDOM, DIAG_CMD_CONNECT_M_CHECK, diag_cmd_password},
    {DIAG_CMD_HOST_CONNECT, DIAG_CMD_HOST_DISCONNECT, diag_cmd_hso_connect_disconnect},
    {DIAG_CMD_MEM_MEM32, DIAG_CMD_MEM_W4, diag_cmd_mem_read_and_write},
#endif
#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_VER) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_VER == DFX_YES)
    {ID_DIAG_CMD_CYCLE_READ_MEM, ID_DIAG_CMD_READ_MEM, diag_cmd_mem_operate},
#endif
#if defined(CONFIG_DFX_SUPPORT_DIAG_UART_CHANNEL) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_UART_CHANNEL == DFX_YES)
    {DIAG_CMD_CHANGE_UART_CONFIG, DIAG_CMD_CHANGE_UART_CONFIG, diag_cmd_modify_uart_config},
#endif
#if defined(CONFIG_SYSERR_SUPPORT_CRASH_DUMP)
    {DIAG_CMD_DUMP_SYSERR, DIAG_CMD_DUMP_SYSERR_MAX, diag_cmd_syserr_dump},
#endif
    {DIAG_CMD_LOG_AIR_IND, DIAG_CMD_MSG_CFG_SET_LEVEL, diag_cmd_filter_set},
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_SUPPORT_MOCKED_SHELL)
    {DIAG_CMD_MOCKED_SHELL, DIAG_CMD_MOCKED_SHELL, diag_cmd_mocked_shell},
#endif
#endif
#if defined(CONFIG_DFX_SUPPORT_DIAG_CMD_DFX) && defined(DFX_YES) && \
    (CONFIG_DFX_SUPPORT_DIAG_CMD_DFX == DFX_YES)
    {DIAG_CMD_ID_DIAG_DFX_REQ, DIAG_CMD_ID_DIAG_DFX_END, diag_dfx_cmd},
#endif
#if defined(CONFIG_DFX_SUPPORT_OS_INFO)
    {DIAG_CMD_GET_CPUP, DIAG_CMD_GET_TASK_INFO, diag_cmd_dfx_os},
#endif
    {DIAG_CMD_ID_PSD_ENABLE, DIAG_CMD_ID_PSD_ENABLE, diag_cmd_psd_enable},
#if defined(CONFIG_SUPPORT_SAMPLE_DATA)
    {DIAG_CMD_ID_BSLE_SAMPLE, DIAG_CMD_ID_WLAN_PHY_SAMPLE, diag_cmd_start_sample_data}
#endif
};

ext_errno uapi_diag_default_cmd_init(td_void)
{
    return uapi_zdiag_register_cmd(g_diag_default_cmd_tbl,
        sizeof(g_diag_default_cmd_tbl) / sizeof(g_diag_default_cmd_tbl[0]));
}
