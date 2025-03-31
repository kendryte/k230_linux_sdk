/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag debug
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_DEBUG_H__
#define __ZDIAG_DEBUG_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_common.h"

td_void zdiag_debug_print_pkt_info(char *str, diag_pkt_ctrl *pkt_ctrl, td_u8 *pkt);
#endif
