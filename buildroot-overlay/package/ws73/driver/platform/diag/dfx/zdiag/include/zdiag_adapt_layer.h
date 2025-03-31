/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag usr adapt
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_ADAPT_LAYER_H__
#define __ZDIAG_ADAPT_LAYER_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_common.h"
#include "zdiag_config.h"
#include "soc_zdiag_channel.h"
#ifdef __KERNEL__
#include "linux/kernel.h"
#else
#include "soc_log.h"
#endif

#define DIAG_HSO_MAX_MUX_PKT_SIZE 512
#define DIAG_HSO_VER_DEFAULT 0
#define DIAG_HSO_THIS_VER 0x11

#define USER_CMD_LIST_NUM 10 /* Maximum number of non-system command lists */

#define zdiag_printf(fmt, arg...)

td_u32 diag_adapt_get_msg_time(td_void);
diag_channel_id diag_adapt_dst_2_channel_id(zdiag_addr addr);
zdiag_addr diag_adapt_get_local_addr(td_void);
zdiag_addr diag_adapt_get_module_id(td_void);
zdiag_addr diag_adapt_get_module_id_adjusted(td_u32 module_id, td_u32 msg_id);
diag_addr_attribute diag_adapt_addr_2_attribute(zdiag_addr addr);
td_u32 diag_adapt_int_lock(td_void);
td_void diag_adapt_int_restore(td_u32 lock_stat);
td_u16 diag_adapt_crc16(td_u16 crc_start, TD_CONST td_u8 *buf, td_u32 len);
td_u32 diag_adapt_get_cur_second(td_void);
td_void *diag_adapt_malloc(unsigned size);
td_void diag_adapt_free(td_void *p);
ext_errno zdiag_adapt_init(td_void);
ext_errno zdiag_adapt_exit(td_void);

td_void diag_adapt_pkt_malloc(diag_pkt_malloc_param *param, diag_pkt_malloc_result *result);
td_void diag_adapt_pkt_free(diag_pkt_malloc_result *result);

#endif
