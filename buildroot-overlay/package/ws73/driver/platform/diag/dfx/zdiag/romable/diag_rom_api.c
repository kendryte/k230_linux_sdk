/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag ind producer
 * This file should be changed only infrequently and with great care.
 */
#include "diag_rom_api.h"
#include "soc_osal.h"

#ifdef __KERNEL__
#include <linux/module.h>
#endif

diag_rom_api g_diag_rom_api;

ext_errno uapi_diag_report_sys_msg(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size, td_u8 level)
{
    if (g_diag_rom_api.report_sys_msg != TD_NULL) {
        return g_diag_rom_api.report_sys_msg(module_id, msg_id, buf, buf_size, level);
    }
    return EXT_ERR_DIAG_NOT_SUPPORT;
}

td_void diag_rom_api_register(diag_rom_api *api)
{
    g_diag_rom_api = *api;
}
osal_module_export(uapi_diag_report_sys_msg);
