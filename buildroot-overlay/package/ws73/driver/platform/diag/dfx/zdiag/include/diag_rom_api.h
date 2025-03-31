/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag romable
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DIAG_ROM_API_H__
#define __DIAG_ROM_API_H__
#include "soc_zdiag.h"
typedef ext_errno (*diag_report_sys_msg_handler)(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size,
    td_u8 level);

typedef struct {
    diag_report_sys_msg_handler report_sys_msg;
} diag_rom_api;

td_void diag_rom_api_register(diag_rom_api *api);
#endif