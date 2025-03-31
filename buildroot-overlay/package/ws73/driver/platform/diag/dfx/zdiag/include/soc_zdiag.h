/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag api header file
 * This file should be changed only infrequently and with great care.
 */

#ifndef __SOC_ZDIAG_H__
#define __SOC_ZDIAG_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_config.h"

typedef struct {
    zdiag_addr peer_addr;
    td_u8 pad[3];
} diag_option;
#define DIAG_OPTION_INIT_VAL {0, {0, 0, 0}}

typedef ext_errno (*td_zdiag_cmd_f)(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);

/**
 * @ingroup  hct_diag
 * Structure for registration command
 */
typedef struct {
    td_u16 min_id;               /* Minimum DIAG ID */
    td_u16 max_id;               /* Maximum DIAG ID */
    td_zdiag_cmd_f fn_input_cmd; /* This Handler is used to process the HSO command. */
} td_zdiag_cmd_reg_obj;

typedef struct {
    td_u16 id;               /* *< Statistics ID. */
    td_u16 array_cnt;        /* *< Number of statistic structures. */
    td_u32 stat_packet_size; /* *< Size of a single statistic structure (unit: byte). */
    td_pvoid stat_packet;    /* *< Pointer to the statistic structure. */
} td_zdiag_sys_stat_obj;

typedef struct {
    td_u16 sn;  /* cmd sn */
    td_u8 ctrl; /* auto ack:0xfe,initiative ack:0 */
    td_u8 pad;
    td_u16 cmd_id;     /* cmd id */
    td_u16 param_size; /* param size */
    td_u8 *param;      /* data */
} msp_diag_ack_param;

ext_errno uapi_zdiag_register_cmd(TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl, td_u16 cmd_num);
ext_errno uapi_zdiag_report_packet(td_u16 cmd_id, diag_option * option, TD_CONST td_u8 *packet, td_u16 packet_size,
    td_bool sync);
ext_errno uapi_diag_report_sys_msg(td_u32 module_id, td_u32 msg_id, TD_CONST td_u8 *buf, td_u16 buf_size, td_u8 level);
ext_errno uapi_zdiag_register_ind(TD_CONST td_zdiag_cmd_reg_obj *cmd_tbl, td_u16 cmd_num);
ext_errno uapi_zdiag_run_cmd(td_u16 cmd_id, td_u8 *data, td_u16 data_size, diag_option *option);
ext_errno uapi_zdiag_report_ack(msp_diag_ack_param *ack, diag_option *option);
ext_errno uapi_zdiag_register_stat_obj(TD_CONST td_zdiag_sys_stat_obj *stat_obj_tbl, td_u16 obj_num);

ext_errno zdiag_report_packet_with_buf(td_u16 cmd_id, TD_CONST td_u8 *cmd_head, td_u16 head_size,
    TD_CONST td_u8 *cmd_body, td_u16 body_size);

/* psd命令钩子 */
typedef td_void (*psd_enable_callback)(td_u16 cmd_id, td_pvoid cmd_param, td_u16 cmd_param_size, diag_option *option);
td_void uapi_zdiag_psd_enable_unregister(td_void);
td_void uapi_zdiag_psd_enable_register(psd_enable_callback cb);
psd_enable_callback uapi_zdiag_psd_enable_get_cb(td_void);
#endif
