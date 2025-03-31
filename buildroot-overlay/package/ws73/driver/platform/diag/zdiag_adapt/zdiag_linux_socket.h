/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: zdiag linux socket
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_LINUX_SOCKET_H__
#define __ZDIAG_LINUX_SOCKET_H__

#include "td_base.h"
#include "td_type.h"
#include "soc_errno.h"

#define ZDIAG_SOCKET_RX_TASK_PRIORITY   10
#define ZDIAG_SOCKET_TX_TASK_PRIORITY   10
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0))
#define OSAL_SO_RCVTIMEO SO_RCVTIMEO
#define OSAL_SO_SNDTIMEO SO_SNDTIMEO
#else
#define OSAL_SO_RCVTIMEO SO_RCVTIMEO_OLD
#define OSAL_SO_SNDTIMEO SO_SNDTIMEO_OLD
#endif
#endif

typedef enum {
    ZDIAG_SOCKET_DBG_NONE = 0,  /* None debug output */
    ZDIAG_SOCKET_DBG_PRINTK_ON, /* debug output to printk */
} zdiag_socket_dbg_mode_enum;

typedef struct {
    td_u8 diag_chan_id;
    td_u8 hso_addr;
    td_u16 port_num;
} zdiag_socket_kernel_init_param;

td_u32 zdiag_sock_output_buf(td_u8 *buf, td_u16 len);
ext_errno zdiag_socket_channel_init(td_void);
ext_errno zdiag_socket_channel_exit(td_void);

#endif
