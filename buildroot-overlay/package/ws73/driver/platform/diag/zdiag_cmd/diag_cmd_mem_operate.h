/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag mem cmd.
 * This file should be changed only infrequently and with great care.
 */

#ifndef __DIAG_CMD_MEM_OPERATE_H__
#define __DIAG_CMD_MEM_OPERATE_H__
#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag.h"
td_u32 diag_cmd_mem_operate(td_u16 cmd_id, td_pvoid cmd_param, td_u16 param_size, diag_option *option);
#endif