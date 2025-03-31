/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 */

#ifndef __ZDIAG_STAT_H__
#define __ZDIAG_STAT_H__

#include <td_base.h>
#include "zdiag_common.h"
#include "zdiag_config.h"
#include "soc_zdiag.h"

typedef struct {
    TD_CONST td_zdiag_sys_stat_obj *stat_cmd_list[CONFIG_STAT_CMD_LIST_NUM];  /* Stat. cmd obj list */
    td_u16 aus_stat_cmd_num[CONFIG_STAT_CMD_LIST_NUM];
} zdiag_stat_ctrl;

typedef struct {
    td_u16 obj_id;
    td_pbyte object;
    td_u16 obj_size;
    td_u16 obj_cnt;
    diag_option *option;
    td_bool sync;
    td_u8 pad[2]; /* pad(2) */
} zdiag_report_stat_obj_stru;

ext_errno zdiag_report_stat_obj(zdiag_report_stat_obj_stru pkt);
ext_errno zdiag_query_stat_obj(td_u32 id, td_u32 *obj, td_u16 *obj_len, td_u16 *obj_cnt);

#endif /* __ZDIAG_STAT_H__ */
