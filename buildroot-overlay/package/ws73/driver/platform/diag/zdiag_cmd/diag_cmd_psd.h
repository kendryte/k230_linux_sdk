/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: diag cmd psd.
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DIAG_CMD_PSD_H__
#define __DIAG_CMD_PSD_H__

#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag.h"

ext_errno diag_cmd_psd_enable(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);

#endif