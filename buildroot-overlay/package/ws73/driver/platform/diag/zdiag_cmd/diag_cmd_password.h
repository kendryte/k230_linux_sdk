/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag securec connect cmd.
 * This file should be changed only infrequently and with great care.
 */

#ifndef __DIAG_CMD_PASSWORD_H__
#define __DIAG_CMD_PASSWORD_H__
#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag.h"
ext_errno diag_cmd_password(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
#endif
