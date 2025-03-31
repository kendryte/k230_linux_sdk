/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag oam log
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DIAG_OAM_LOG_H__
#define __DIAG_OAM_LOG_H__
#include "td_base.h"
#include "soc_errno.h"
#include "td_base.h"

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

ext_errno oam_log_print0_press_prv(td_u32 msg_id, td_u32 mod_id);
ext_errno oam_log_print1_press_prv(td_u32 msg_id, td_u32 mod_id, td_u32 param_1);
ext_errno oam_log_print2_press_prv(td_u32 msg_id, td_u32 mod_id, td_u32 param_1, td_u32 param_2);
ext_errno oam_log_print3_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg3 *olm);
ext_errno oam_log_print4_press_prv(td_u32 msg_id, td_u32 mod_id, zdiag_log_msg4 *olm);
ext_errno oam_log_print_buff_press_prv(td_u32 msg_id, td_u32 mod_id, td_u8 *data, td_u32 data_size);
#endif
