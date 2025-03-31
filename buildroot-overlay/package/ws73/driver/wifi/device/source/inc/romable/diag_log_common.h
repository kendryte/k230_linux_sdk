/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal 11d api.
 * Create: 2022-04-29
 */

#ifndef __DIAG_LOG_COMMON_H__
#define __DIAG_LOG_COMMON_H__

#if defined (_PRE_WLAN_PLAT_SUPPORT_HOST_HSO)
#include "zdiag_log_util.h"
#else
#include "soc_diag_util.h"
#endif

#include "common_dft_rom.h"
#ifdef BOARD_EDA
#include "serial_dw.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if !(defined(_PRE_WLAN_PLAT_SUPPORT_HOST_PRINTK) || defined(BOARD_EDA))
/* 日志宏定义 */
#if defined(WSCFG_PLAT_DIAG_LOG_OUT) || !defined(_PRE_WLAN_PLAT_SUPPORT_HOST_HSO)
/* info level log wapper macro */
#define oam_info_log0(_uc_vap_0, _en_feature_0, fmt) ext_diag_info_log0(0, fmt)
#define oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1) ext_diag_info_log1(0, fmt, (td_u32)(p1))
#define oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2) ext_diag_info_log2(0, fmt, (td_u32)(p1), (td_u32)(p2))
#define oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    ext_diag_info_log3(0, fmt, (td_u32)(p1), (td_u32)(p2), (td_u32)(p3))
#define oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    ext_diag_info_log4(0, fmt, (td_u32)(p1), (td_u32)(p2), (td_u32)(p3), (td_u32)(p4))

#define oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    ext_diag_info_log_buff(0, buffer, data_size, fmt, type)

/* warning level log wapper macro */
#define oam_warning_log0(_uc_vap_0, _en_feature_0, fmt) \
    do {                                                \
        ext_diag_warning_log0(0, fmt);                  \
    } while (0)
#define oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1) \
    do {                                                    \
        ext_diag_warning_log1(0, fmt, (td_u32)(p1));        \
    } while (0)
#define oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)    \
    do {                                                           \
        ext_diag_warning_log2(0, fmt, (td_u32)(p1), (td_u32)(p2)); \
    } while (0)
#define oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)              \
    do {                                                                         \
        ext_diag_warning_log3(0, fmt, (td_u32)(p1), (td_u32)(p2), (td_u32)(p3)); \
    } while (0)
#define oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)                        \
    do {                                                                                       \
        ext_diag_warning_log4(0, fmt, (td_u32)(p1), (td_u32)(p2), (td_u32)(p3), (td_u32)(p4)); \
    } while (0)

/* error level log wapper macro */
#define oam_error_log0(_uc_vap_0, _en_feature_0, fmt) \
    do {                                              \
        ext_diag_error_log0(0, fmt);                  \
    } while (0)
#define oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1) \
    do {                                                  \
        ext_diag_error_log1(0, fmt, (td_u32)(p1));        \
    } while (0)
#define oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)    \
    do {                                                         \
        ext_diag_error_log2(0, fmt, (td_u32)(p1), (td_u32)(p2)); \
    } while (0)
#define oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)              \
    do {                                                                       \
        ext_diag_error_log3(0, fmt, (td_u32)(p1), (td_u32)(p2), (td_u32)(p3)); \
    } while (0)
#define oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)                        \
    do {                                                                                     \
        ext_diag_error_log4(0, fmt, (td_u32)(p1), (td_u32)(p2), (td_u32)(p3), (td_u32)(p4)); \
    } while (0)

#else
/* info level log wapper macro */
#define oam_info_log0(_uc_vap_0, _en_feature_0, fmt)
#define oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1) \
    do {                                                 \
        (void)(p1);                                      \
        oam_info_log0(_uc_vap_0, _en_feature_0, fmt);    \
    } while (0)
#define oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2) \
    do {                                                     \
        (void)(p2);                                          \
        oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1);    \
    } while (0)
#define oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    do {                                                         \
        (void)(p3);                                              \
        oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2);    \
    } while (0)
#define oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    do {                                                             \
        (void)(p4);                                                  \
        oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3);    \
    } while (0)

#define oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    do {                                                                     \
    } while (0)

/* warning level log wapper macro */
#define oam_warning_log0(_uc_vap_0, _en_feature_0, fmt)
#define oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1) \
    do {                                                    \
        (void)(p1);                                         \
        oam_warning_log0(_uc_vap_0, _en_feature_0, fmt);    \
    } while (0)
#define oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2) \
    do {                                                        \
        (void)(p2);                                             \
        oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1);    \
    } while (0)
#define oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    do {                                                            \
        (void)(p3);                                                 \
        oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2);    \
    } while (0)
#define oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    do {                                                                \
        (void)(p4);                                                     \
        oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3);    \
    } while (0)

/* error level log wapper macro */
#define oam_error_log0(_uc_vap_0, _en_feature_0, fmt)
#define oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1) \
    do {                                                  \
        (void)(p1);                                       \
        oam_error_log0(_uc_vap_0, _en_feature_0, fmt);    \
    } while (0)
#define oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2) \
    do {                                                      \
        (void)(p2);                                           \
        oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1);    \
    } while (0)
#define oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    do {                                                          \
        (void)(p3);                                               \
        oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2);    \
    } while (0)
#define oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    do {                                                              \
        (void)(p4);                                                   \
        oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3);    \
    } while (0)

#endif

#define oam_info_log_alter(_uc_vap_0, _en_feature_0, fmt, num, args...)
#define oam_warning_log_alter(_uc_vap_0, _en_feature_0, fmt, num, args...)
#define oam_error_log_alter(_uc_vap_0, _en_feature_0, fmt, num, args...)

#define oam_warning_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    ext_diag_warning_log_buff(0, buffer, data_size, fmt, type)
#define oam_error_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    ext_diag_error_log_buff(0, buffer, data_size, fmt, type)

#define OAM_INFO_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_info_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_INFO_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_INFO_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_INFO_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_INFO_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_INFO_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define OAM_WARNING_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_warning_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_WARNING_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_WARNING_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_WARNING_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_WARNING_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_WARNING_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_warning_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define OAM_ERROR_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_error_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_ERROR_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_ERROR_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_ERROR_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_ERROR_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_ERROR_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_error_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)
#elif defined(BOARD_EDA)
/* 日志宏定义 */
#define oam_info_log0(_uc_vap_0, _en_feature_0, fmt)
#define oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define oam_warning_log0(_uc_vap_0, _en_feature_0, fmt)    do { \
        debug_message("WARN0 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1)    do { \
        debug_message("WARN1 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)    do { \
        debug_message("WARN2 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)    do { \
        debug_message("WARN3 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)    do { \
        debug_message("WARN4 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)

#define oam_error_log0(_uc_vap_0, _en_feature_0, fmt)    do { \
        debug_message("ERROR0 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1)    do { \
        debug_message("ERROR1 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)    do { \
        debug_message("ERROR2 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)    do { \
        debug_message("ERROR3 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)
#define oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)    do { \
        debug_message("ERROR4 id=%d, line=%d\r\n", THIS_FILE_ID, __LINE__); \
    } while (0)

#define oam_warning_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)
#define oam_error_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define oam_info_log_alter(id, feature_id, fmt, num, arg...)
#define oam_warning_log_alter(id, feature_id, fmt, num, arg...)
#define oam_error_log_alter(id, feature_id, fmt, num, arg...)

#define OAM_INFO_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_info_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_INFO_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_INFO_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_INFO_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_INFO_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_INFO_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define OAM_WARNING_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_warning_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_WARNING_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_WARNING_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_WARNING_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_WARNING_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_WARNING_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_warning_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define OAM_ERROR_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_error_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_ERROR_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_ERROR_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_ERROR_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_ERROR_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_ERROR_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_error_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)
#else
/* 日志宏定义 */
#define oam_info_log0(_uc_vap_0, _en_feature_0, fmt)
#define oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define oam_warning_log0(_uc_vap_0, _en_feature_0, fmt)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__); \
        } \
    } while (0)
#define oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1); \
        } \
    } while (0)
#define oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1, p2); \
        } \
    } while (0)
#define oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1, p2, p3); \
        } \
    } while (0)
#define oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1, p2, p3, p4); \
        } \
    } while (0)

#define oam_error_log0(_uc_vap_0, _en_feature_0, fmt)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__); \
        } \
    } while (0)
#define oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1); \
        } \
    } while (0)
#define oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1, p2); \
        } \
    } while (0)
#define oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1, p2, p3); \
        } \
    } while (0)
#define oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)    do { \
        if (dft_get_log_level_switch()) { \
            osal_printk("[%d %d][%d]" fmt "\r\n", _uc_vap_0, _en_feature_0, __LINE__, p1, p2, p3, p4); \
        } \
    } while (0)

#define oam_warning_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)
#define oam_error_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define oam_info_log_alter(id, feature_id, fmt, num, arg...)
#define oam_warning_log_alter(id, feature_id, fmt, num, arg...)
#define oam_error_log_alter(id, feature_id, fmt, num, arg...)

#define OAM_INFO_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_info_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_INFO_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_INFO_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_INFO_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_INFO_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_INFO_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define OAM_WARNING_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_warning_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_WARNING_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_WARNING_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_WARNING_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_WARNING_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_WARNING_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_warning_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

#define OAM_ERROR_LOG0(_uc_vap_0, _en_feature_0, fmt) oam_error_log0(_uc_vap_0, _en_feature_0, fmt)
#define OAM_ERROR_LOG1(_uc_vap_0, _en_feature_0, fmt, p1) oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define OAM_ERROR_LOG2(_uc_vap_0, _en_feature_0, fmt, p1, p2) oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define OAM_ERROR_LOG3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3) \
    oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define OAM_ERROR_LOG4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4) \
    oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#define OAM_ERROR_BUF(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    oam_error_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of diag_log_adpt.h */
