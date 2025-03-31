/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: diag cmd for mocked shell.
 */

#ifndef __DIAG_CMD_SHELL_H__
#define __DIAG_CMD_SHELL_H__

#ifdef CONFIG_SUPPORT_MOCKED_SHELL
#include "soc_zdiag.h"
#include "td_base.h"

typedef ext_errno (*diag_sh_handler)(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);

typedef struct {
    diag_sh_handler run_sh;
} diag_shell_api;

td_void diag_shell_api_register(diag_shell_api *api);

ext_errno diag_cmd_mocked_shell(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);

#endif
#endif
