/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag print log utils
 */

#ifndef __ZDIAG_LOG_UTIL_H__
#define __ZDIAG_LOG_UTIL_H__

#include "soc_diag_wdk.h"
#include "zdiag_log_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define DIAG_LOG_LEVEL_ALERT    0
#define DIAG_LOG_LEVEL_FATAL    1
#define DIAG_LOG_LEVEL_ERROR    2
#define DIAG_LOG_LEVEL_WARN     3
#define DIAG_LOG_LEVEL_NOTICE   4
#define DIAG_LOG_LEVEL_INFO     5
#define DIAG_LOG_LEVEL_DBG      6
#define DIAG_LOG_LEVEL_TRACE    7

#if defined(HAVE_PCLINT_CHECK)
#define ext_check_default_id(id) (id)
#else
#define ext_check_default_id(id) (((id) == 0) ? __LINE__ : (id))
#endif

#define ext_diag_log_msg_mk_id_e(id) ((((((td_u32)(ext_check_default_id(id))) << 4) & 0x3FFF0) + DIAG_LOG_LEVEL_ERROR) \
    + ((((td_u32)(THIS_FILE_ID)) & 0x3FFF) << 18))
#define ext_diag_log_msg_mk_id_w(id) ((((((td_u32)(ext_check_default_id(id))) << 4) & 0x3FFF0) + DIAG_LOG_LEVEL_WARN) \
    + ((((td_u32)(THIS_FILE_ID)) & 0x3FFF) << 18))
#define ext_diag_log_msg_mk_id_i(id) ((((((td_u32)(ext_check_default_id(id))) << 4) & 0x3FFF0) + DIAG_LOG_LEVEL_INFO) \
    + ((((td_u32)(THIS_FILE_ID)) & 0x3FFF) << 18))

#define ext_makeu32(a, b) ((osal_u32)(((osal_u16)(a)) | ((osal_u32)((osal_u16)(b))) << 16))
#define ext_diag_dev_log_msg_id(file, line, level) ext_makeu32(((((osal_u16)(line)) << 4) + (level)), (file))
#define ext_diag_dev_log_mk_mod_id(core, feature) ((osal_u32)(((osal_u32)(core) << 24) | (feature)))

#define CORE_ID 0
#define THIS_MOD_ID 0

#define ext_check_default_mod_id(id) (((id) == 0) ? THIS_MOD_ID : (id))
#define ext_diag_log_msg_mk_mod_id(src_mod) ((td_u32)(((td_u32)(ext_check_default_mod_id(src_mod))) | \
    ((td_u32)(CORE_ID)) << 24))

#if (defined(DIAG_PRINT_TO_SHELL))
#define diag_io_print(fmt, arg...) printf("[%s:%d] "fmt"\r\n", __func__, __LINE__, ##arg)
#else
#define diag_io_print(fmt, arg...)
#endif

#define ext_diag_error_log0(id, fmt) \
    zdiag_log_print0_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0))
#define ext_diag_error_log1(id, fmt, p1) \
    zdiag_log_print1_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), p1)
#define ext_diag_error_log2(id, fmt, p1, p2) \
    zdiag_log_print2_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), p1, p2)
#define ext_diag_error_log3(id, fmt, p1, p2, p3) \
do { \
    zdiag_log_msg3 log_msg = {p1, p2, p3}; \
    zdiag_log_print3_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_error_log4(id, fmt, p1, p2, p3, p4) \
do { \
    zdiag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    zdiag_log_print4_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_error_log_buff(id, log_info, data_size, fmt, type) \
    zdiag_log_print_buff_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), log_info, data_size)

#define ext_diag_warning_log0(id, fmt) \
    zdiag_log_print0_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0))
#define ext_diag_warning_log1(id, fmt, p1) \
    zdiag_log_print1_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), p1)
#define ext_diag_warning_log2(id, fmt, p1, p2) \
    zdiag_log_print2_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), p1, p2)
#define ext_diag_warning_log3(id, fmt, p1, p2, p3) \
do { \
    zdiag_log_msg3 log_msg = {p1, p2, p3}; \
    zdiag_log_print3_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_warning_log4(id, fmt, p1, p2, p3, p4) \
do { \
    zdiag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    zdiag_log_print4_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_warning_log_buff(id, log_info, data_size, fmt, type) \
    zdiag_log_print_buff_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), log_info, data_size)

#define ext_diag_info_log0(id, fmt)  \
    zdiag_log_print0_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0))
#define ext_diag_info_log1(id, fmt, p1)  \
    zdiag_log_print1_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), p1)
#define ext_diag_info_log2(id, fmt, p1, p2)  \
    zdiag_log_print2_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), p1, p2)
#define ext_diag_info_log3(id, fmt, p1, p2, p3)  \
do { \
    zdiag_log_msg3 log_msg = {p1, p2, p3}; \
    zdiag_log_print3_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_info_log4(id, fmt, p1, p2, p3, p4) \
do { \
    zdiag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    zdiag_log_print4_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_info_log_buff(id, log_info, data_size, fmt, type) \
    zdiag_log_print_buff_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), log_info, data_size)


#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of zdiag_log_util.h */
