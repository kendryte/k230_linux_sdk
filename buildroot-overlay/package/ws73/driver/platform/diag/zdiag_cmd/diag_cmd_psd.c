/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 */
#include "diag_cmd_psd.h"
#include "soc_zdiag.h"
#include "soc_diag_cmd_id.h"
#include "soc_diag_cmd.h"
#include "zdiag_adapt_layer.h"
#include "securec.h"

/* psd使能命令 */
ext_errno diag_cmd_psd_enable(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option)
{
    psd_enable_callback psd_enable_cb = uapi_zdiag_psd_enable_get_cb();
    if (psd_enable_cb != TD_NULL) {
        psd_enable_cb(cmd_id, cmd_param, cmd_param_size, option);
    }
    return EXT_ERR_SUCCESS;
}
