/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag oam log
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_LOG_ADAPT_H__
#define __ZDIAG_LOG_ADAPT_H__

#include "td_base.h"
#include "soc_errno.h"

#define ZDIAG_LOG_LEVEL_MASK 0x0F
#define MUX_PKT_VER 1

#define DIAG_OPTION_INIT_VAL {0, {0, 0, 0}}

#define MUX_START_FLAG 0xDEADBEEF

typedef struct {
    td_u32 data0;
} zdiag_log_msg1;

typedef struct {
    td_u32 data0;
    td_u32 data1;
} zdiag_log_msg2;

typedef struct {
    td_u32 data0;
    td_u32 data1;
    td_u32 data2;
} zdiag_log_msg3;

typedef struct {
    td_u32 data0;
    td_u32 data1;
    td_u32 data2;
    td_u32 data3;
} zdiag_log_msg4;

ext_errno zdiag_log_report_sys_msg_instance(
    td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size, td_u8 level);
ext_errno zdiag_log_print0_press_prv(td_u32 msg_id, td_u32 mod_id);
ext_errno zdiag_log_print1_press_prv(td_u32 msg_id, td_u32 mod_id, td_s32 param_1);
ext_errno zdiag_log_print2_press_prv(td_u32 msg_id, td_u32 mod_id, td_s32 param_1, td_s32 param_2);
ext_errno zdiag_log_print3_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg3 *olm);
ext_errno zdiag_log_print4_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg4 *olm);
ext_errno zdiag_log_print_buff_press_prv(td_u32 msg_id, td_u32 mod_id, td_u8 *data, td_u32 data_size);

#endif /* end of zdiag_log_adapt.h */
