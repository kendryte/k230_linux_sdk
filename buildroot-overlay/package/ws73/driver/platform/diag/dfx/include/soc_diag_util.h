/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: diag print log
 */

#ifndef __SOC_DIAG_UTIL_H__
#define __SOC_DIAG_UTIL_H__

#include "soc_diag_wdk.h"
#if defined DIAG_PRINT && defined CONFIG_SUPPORT_NEW_DIAG
#include "diag_oam_log.h"
#else
#include "soc_diag.h"
#endif

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

#if (defined(MAKE_PRIM_XML_PROCESS_IN))
#define ext_diag_error_log0(id, fmt) \
    {                                                                                                         \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_ERROR, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg0, _PRIM_END_   \
    }
#define ext_diag_error_log1(id, fmt, p0) \
    {                                                                                                         \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_ERROR, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg1, _PRIM_END_   \
    }
#define ext_diag_error_log2(id, fmt, p0, p1) \
    {                                                                                                         \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_ERROR, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg2, _PRIM_END_   \
    }
#define ext_diag_error_log3(id, fmt, p0, p1, p2) \
    {                                                                                                         \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_ERROR, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg3, _PRIM_END_   \
    }
#define ext_diag_error_log4(id, fmt, p0, p1, p2, p3) \
    {                                                                                                         \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_ERROR, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg4, _PRIM_END_   \
    }
#define ext_diag_error_log_buff(id, log_info, data_size, fmt, type) \
    {                                                                                                         \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_ERROR, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = type, _PRIM_END_            \
    }

#define ext_diag_warning_log0(id, fmt) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_WARN, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg0, _PRIM_END_  \
    }
#define ext_diag_warning_log1(id, fmt, p0) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_WARN, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg1, _PRIM_END_  \
    }
#define ext_diag_warning_log2(id, fmt, p0, p1) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_WARN, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg2, _PRIM_END_  \
    }
#define ext_diag_warning_log3(id, fmt, p0, p1, p2) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_WARN, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg3, _PRIM_END_  \
    }
#define ext_diag_warning_log4(id, fmt, p0, p1, p2, p3) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_WARN, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg4, _PRIM_END_  \
    }
#define ext_diag_warning_log_buff(id, log_info, data_size, fmt, type) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_WARN, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = type, _PRIM_END_           \
    }

#define ext_diag_info_log0(id, fmt) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_INFO, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg0, _PRIM_END_  \
    }
#define ext_diag_info_log1(id, fmt, p0) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_INFO, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg1, _PRIM_END_  \
    }
#define ext_diag_info_log2(id, fmt, p0, p1) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_INFO, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg2, _PRIM_END_  \
    }
#define ext_diag_info_log3(id, fmt, p0, p1, p2) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_INFO, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg3, _PRIM_END_  \
    }
#define ext_diag_info_log4(id, fmt, p0, p1, p2, p3) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_INFO, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = diag_log_msg4, _PRIM_END_  \
    }
#define ext_diag_info_log_buff(id, log_info, data_size, fmt, type) \
    {                                                                                                        \
        _PRIM_ST, _PRIM_PRI_ = DIAG_LOG_LEVEL_INFO, _PRIM_ID_ = id, _PRIM_SZ_ = fmt, _PRIM_LINE_ = __LINE__, \
        _PRIM_FILE_ = __FILE_NAME__, _PRIM_FILE_ID_ = __FILE_IDX__, _PRIM_TYPE_ = type, _PRIM_END_           \
    }

#elif (defined(DIAG_PRINT)) && !defined(CONFIG_SUPPORT_NEW_DIAG)

#define ext_diag_error_log0(id, fmt) \
    diag_log_msg0_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), 0)
#define ext_diag_error_log1(id, fmt, p1) \
    diag_log_msg1_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), 0, p1)
#define ext_diag_error_log2(id, fmt, p1, p2) \
    diag_log_msg2_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), 0, p1, p2)
#define ext_diag_error_log3(id, fmt, p1, p2, p3) \
do { \
    diag_log_msg3 log_msg = {p1, p2, p3}; \
    diag_log_msg3_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), 0, log_msg); \
} while (0)
#define ext_diag_error_log4(id, fmt, p1, p2, p3, p4) \
do { \
    diag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    diag_log_msg4_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), 0, log_msg); \
} while (0)
#define ext_diag_error_log_buff(id, log_info, data_size, fmt, type) \
    diag_log_msg_buffer_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), 0, log_info, data_size)

#define ext_diag_warning_log0(id, fmt) \
    diag_log_msg0_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), 0)
#define ext_diag_warning_log1(id, fmt, p1) \
    diag_log_msg1_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), 0, p1)
#define ext_diag_warning_log2(id, fmt, p1, p2) \
    diag_log_msg2_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), 0, p1, p2)
#define ext_diag_warning_log3(id, fmt, p1, p2, p3) \
do { \
    diag_log_msg3 log_msg = {p1, p2, p3}; \
    diag_log_msg3_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), 0, log_msg); \
} while (0)
#define ext_diag_warning_log4(id, fmt, p1, p2, p3, p4) \
do { \
    diag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    diag_log_msg4_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), 0, log_msg); \
} while (0)
#define ext_diag_warning_log_buff(id, log_info, data_size, fmt, type) \
    diag_log_msg_buffer_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), 0, log_info, data_size)

#define ext_diag_info_log0(id, fmt)  \
    diag_log_msg0_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), 0)
#define ext_diag_info_log1(id, fmt, p1)  \
    diag_log_msg1_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), 0, p1)
#define ext_diag_info_log2(id, fmt, p1, p2)  \
    diag_log_msg2_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), 0, p1, p2)
#define ext_diag_info_log3(id, fmt, p1, p2, p3)  \
do { \
    diag_log_msg3 log_msg = {p1, p2, p3}; \
    diag_log_msg3_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), 0, log_msg); \
} while (0)
#define ext_diag_info_log4(id, fmt, p1, p2, p3, p4) \
do { \
    diag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    diag_log_msg4_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), 0, log_msg); \
} while (0)
#define ext_diag_info_log_buff(id, log_info, data_size, fmt, type) \
    diag_log_msg_buffer_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), 0, log_info, data_size)


#elif (defined(DIAG_PRINT)) && defined(CONFIG_SUPPORT_NEW_DIAG)
#define ext_diag_error_log0(id, fmt) \
    oam_log_print0_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0))
#define ext_diag_error_log1(id, fmt, p1) \
    oam_log_print1_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), p1)
#define ext_diag_error_log2(id, fmt, p1, p2) \
    oam_log_print2_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), p1, p2)
#define ext_diag_error_log3(id, fmt, p1, p2, p3) \
do { \
    zdiag_log_msg3 log_msg = {p1, p2, p3}; \
    oam_log_print3_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_error_log4(id, fmt, p1, p2, p3, p4) \
do { \
    zdiag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    oam_log_print4_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_error_log_buff(id, log_info, data_size, fmt, type) \
    oam_log_print_buff_press_prv(ext_diag_log_msg_mk_id_e(id), ext_diag_log_msg_mk_mod_id(0), log_info, data_size)

#define ext_diag_warning_log0(id, fmt) \
    oam_log_print0_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0))
#define ext_diag_warning_log1(id, fmt, p1) \
    oam_log_print1_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), p1)
#define ext_diag_warning_log2(id, fmt, p1, p2) \
    oam_log_print2_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), p1, p2)
#define ext_diag_warning_log3(id, fmt, p1, p2, p3) \
do { \
    zdiag_log_msg3 log_msg = {p1, p2, p3}; \
    oam_log_print3_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_warning_log4(id, fmt, p1, p2, p3, p4) \
do { \
    zdiag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    oam_log_print4_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_warning_log_buff(id, log_info, data_size, fmt, type) \
    oam_log_print_buff_press_prv(ext_diag_log_msg_mk_id_w(id), ext_diag_log_msg_mk_mod_id(0), log_info, data_size)

#define ext_diag_info_log0(id, fmt)  \
    oam_log_print0_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0))
#define ext_diag_info_log1(id, fmt, p1)  \
    oam_log_print1_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), p1)
#define ext_diag_info_log2(id, fmt, p1, p2)  \
    oam_log_print2_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), p1, p2)
#define ext_diag_info_log3(id, fmt, p1, p2, p3)  \
do { \
    zdiag_log_msg3 log_msg = {p1, p2, p3}; \
    oam_log_print3_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_info_log4(id, fmt, p1, p2, p3, p4) \
do { \
    zdiag_log_msg4 log_msg = {p1, p2, p3, p4}; \
    oam_log_print4_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), &log_msg); \
} while (0)
#define ext_diag_info_log_buff(id, log_info, data_size, fmt, type) \
    oam_log_print_buff_press_prv(ext_diag_log_msg_mk_id_i(id), ext_diag_log_msg_mk_mod_id(0), log_info, data_size)
#else
#define ext_diag_error_log0(id, fmt) diag_io_print("[ERROR]"fmt"\r\n")
#define ext_diag_error_log1(id, fmt, p1) diag_io_print("[ERROR]"fmt" %d\r\n", p1)
#define ext_diag_error_log2(id, fmt, p1, p2) diag_io_print("[ERROR]"fmt" %d  %d\r\n", p1, p2)
#define ext_diag_error_log3(id, fmt, p1, p2, p3) diag_io_print("[ERROR]"fmt" %d %d %d\r\n", p1, p2, p3)
#define ext_diag_error_log4(id, fmt, p1, p2, p3, p4) diag_io_print("[ERROR]"fmt" %d %d %d %d\r\n", p1, p2, p3, p4)
#define ext_diag_error_log_buff(id, log_info, data_size, fmt, type)

#define ext_diag_warning_log0(id, fmt) diag_io_print("[WARNING]"fmt"\r\n")
#define ext_diag_warning_log1(id, fmt, p1) diag_io_print("[WARNING]"fmt" %d\r\n", p1)
#define ext_diag_warning_log2(id, fmt, p1, p2) diag_io_print("[WARNING]"fmt" %d %d\r\n", p1, p2)
#define ext_diag_warning_log3(id, fmt, p1, p2, p3) diag_io_print("[WARNING]"fmt" %d %d %d\r\n", p1, p2, p3)
#define ext_diag_warning_log4(id, fmt, p1, p2, p3, p4) diag_io_print("[WARNING]"fmt" %d %d %d %d\r\n", p1, p2, p3, p4)
#define ext_diag_warning_log_buff(id, log_info, data_size, fmt, type)

#define ext_diag_info_log0(id, fmt) diag_io_print("[INFO]"fmt"\r\n")
#define ext_diag_info_log1(id, fmt, p1) diag_io_print("[INFO]"fmt" %d\r\n", p1)
#define ext_diag_info_log2(id, fmt, p1, p2) diag_io_print("[INFO]"fmt" %d %d\r\n", p1, p2)
#define ext_diag_info_log3(id, fmt, p1, p2, p3) diag_io_print("[INFO]"fmt" %d %d %d\r\n", p1, p2, p3)
#define ext_diag_info_log4(id, fmt, p1, p2, p3, p4) diag_io_print("[INFO]"fmt" %d %d %d %d\r\n", p1, p2, p3, p4)
#define ext_diag_info_log_buff(id, log_info, data_size, fmt, type)
#endif /* end DIAG_PRINT */

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of soc_diag_util.h */
