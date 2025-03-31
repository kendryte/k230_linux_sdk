/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag cmd process
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_CMD_DST_H__
#define __ZDIAG_CMD_DST_H__
#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag.h"
#include "zdiag_common.h"
ext_errno diag_pkt_router_run_cmd(msp_diag_head_req_stru *req, diag_option *option);
#endif
