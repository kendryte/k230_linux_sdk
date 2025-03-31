/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag filter
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DIAG_CMD_FILTER_H__
#define __DIAG_CMD_FILTER_H__
#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag.h"

ext_errno diag_cmd_filter_set(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void diag_enable_all_switch(void);
void diag_enable_level_switch(td_u32 level);
#endif

#endif