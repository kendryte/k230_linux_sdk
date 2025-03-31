/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag usr config
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_ADAPT_HCC_COMM_H__
#define __ZDIAG_ADAPT_HCC_COMM_H__
#include "td_base.h"
#include "soc_errno.h"

typedef enum _hcc_comm_id_sub_type {
    HCC_DIAG_CMD_HOST_CONNECT       = 0x5000,
    HCC_DIAG_CMD_HOST_DISCONNECT    = 0x5001
} hcc_comm_id_sub_type;

typedef struct {
    osal_u16 sync_id;       /* 同步命令ID */
    osal_u16 len;           /* DATA payload长度 */
    osal_u8 msg_body[0];    /* DATA payload字节 */
} zdiag_hcc_oam_data_stru;

#endif
