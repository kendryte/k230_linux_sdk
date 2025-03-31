/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag connect cmd.
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DIAG_CMD_CONNECT_H__
#define __DIAG_CMD_CONNECT_H__

#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag.h"

#ifdef HOST_SOCKET_DIAG
td_bool get_hso_connect_status(td_void);
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
td_void diag_dis_cmd_support_or_not(td_bool is_supported);
#endif

ext_errno diag_cmd_hso_connect_disconnect(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size,
    diag_option *option);
#endif
