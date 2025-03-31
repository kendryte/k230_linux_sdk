/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: diag offline log utils for os: linux
 * This file should be changed only infrequently and with great care.
 */

#ifndef __ZDIAG_LOCAL_LOG_H__
#define __ZDIAG_LOCAL_LOG_H__

#include "td_base.h"
#include "soc_errno.h"
#include "osal_types.h"

#define OAM_FILE_PATH     "/tmp/diag_log" /* Default Save Path */

#define OAM_LOG_ENABLE    "oam_log_enable" /* save oam log enable */
#define OAM_LOG_LEVEL     "oam_log_level"  /* save oam log level */
#define OAM_LOG_PATH      "oam_log_path"   /* save oam log path */
#define OAM_LOG_SIZE      "oam_log_size"   /* Save each file size */
#define OAM_LOG_COUNT     "oam_log_count"  /* Maximum number of files to be saved */

enum DIAG_LOG_SWITCH {
    DIAG_LOG_DISABLE = 0,
    DIAG_LOG_ENABLE = 1,
};

/* Offline log header information, based on hso_msg_file_head */
typedef struct {
    int start_flag;
    int version;
    int file_type;
    int msg_version;
} diag_local_file_head;

td_u32 zdiag_offline_log_output(td_u8 *hcc_buf, td_u16 len);
void zdiag_local_log_init(void);
void test_zdiag_local_log_output(void);
void zdiag_local_log_cb_for_dev_bsp_ready(void);

#endif
