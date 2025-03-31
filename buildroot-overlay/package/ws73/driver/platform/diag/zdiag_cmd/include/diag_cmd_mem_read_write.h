/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: diag cmd mem read write.
 * This file should be changed only infrequently and with great care.
 */

#ifndef DIAG_CMD_MEM_READ_WRITE_H
#define DIAG_CMD_MEM_READ_WRITE_H
#include "td_base.h"
#include "soc_errno.h"
#include "soc_zdiag.h"

ext_errno diag_cmd_mem32(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_mem16(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_mem8(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_w1(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_w2(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_w4(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_mem_read_and_write(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_cmd_mem_proc_param(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
ext_errno diag_reg_write(td_u32 addr, td_u32 val);
ext_errno diag_reg_read(td_u32 addr, td_u32 *read_val);
#ifdef _PRE_WLAN_REG_RW_WHITELIST
ext_errno reg_rw_check_addr(td_u32 start_addr, td_u32 bytes_cnt);
#endif

#endif
