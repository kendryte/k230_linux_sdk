/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: zdiag ind producer
 * This file should be changed only infrequently and with great care.
 */
#ifndef __ZDIAG_IND_SRC_H__
#define __ZDIAG_IND_SRC_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_common.h"
#include "soc_zdiag.h"

typedef struct {
    zdiag_addr src;
    zdiag_addr dst;
    td_u16 cmd_id;
    td_u16 ctrl;
    td_u16 len;
    td_u8 data[0];
} zdiag_ind_msg;

typedef struct {
    zdiag_addr src;
    zdiag_addr dst;
    td_u16 cmd_id;
    td_u16 ctrl;
    td_u16 len;
    td_u8 *data;
} diag_ind_param;

typedef ext_errno (*report_msg_func)(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size,
    td_u8 level);
ext_errno uapi_zdiag_report_sys_msg_directly(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size,
    td_u8 level);
ext_errno uapi_zdiag_report_sys_msg_cache(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size,
    td_u8 level);

ext_errno uapi_zdiag_report_sys_msg_instance(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size,
    td_u8 level);

#endif
