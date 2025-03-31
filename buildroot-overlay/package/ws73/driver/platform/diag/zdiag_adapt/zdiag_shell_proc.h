/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: zdiag shell proc
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_SHELL_PROC_H__
#define __ZDIAG_SHELL_PROC_H__

#include "td_base.h"
#include "soc_errno.h"

ext_errno zdiag_shell_proc_init(td_void);
ext_errno zdiag_shell_proc_exit(td_void);

#endif /* end of zdiag_shell_proc.h */
