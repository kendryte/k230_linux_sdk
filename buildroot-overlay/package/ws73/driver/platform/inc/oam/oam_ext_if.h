/*
 * Copyright (c) CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: oam event source file
 * Author: Huanghe
 * Create: 2020-10-13
 */


#ifndef __OAM_EXT_IF_H__
#define __OAM_EXT_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "wifi_spec.h"
#include "oal_types.h"
#include "osal_types.h"
#include "oal_net.h"
#include "customize.h"
#include "soc_diag_wdk.h"

#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#if defined(_PRE_WLAN_PLAT_SUPPORT_HOST_HSO)
#include "zdiag_log_util.h"
#else
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "socket_diag_util.h"
#endif
#endif
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_EXT_IF_H

typedef enum {
    OAM_DATA_TYPE_LOG           = 0x01,
    OAM_DATA_TYPE_OTA           = 0x02,
    OAM_DATA_TYPE_TRACE         = 0x03,
    OAM_DATA_TYPE_EVENT         = 0x04,
    OAM_DATA_TYPE_LOG_SWITCH    = 0x05,
    OAM_DATA_TYPE_MEM_RW        = 0x09,
    OAM_DATA_TYPE_REG_RW        = 0x15,
    OAM_DATA_TYPE_CFG           = 0x0b,
    OAM_DATA_TYPE_GVAR_RW       = 0x27,
    OAM_DATA_TYPE_STRING        = 0x28,
    OAM_DATA_TYPE_STATS         = 0x29,
    OAM_DATA_TYPE_BT_PCM        = 0x2a,
    OAM_DATA_TYPE_DEVICE_LOG    = 0x40,
    OAM_DATA_TYPE_BUTT
} oam_data_type_enum;

typedef oal_uint8 oam_data_type_enum_uint8;

#if defined(_PRE_WLAN_PLAT_SUPPORT_HOST_HSO)

/* 日志宏定义 */
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
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
#define oam_info_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define oam_info_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define oam_info_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define oam_info_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)

#define oam_info_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type)

/* warning level log wapper macro */
#define oam_warning_log0(_uc_vap_0, _en_feature_0, fmt)
#define oam_warning_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define oam_warning_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define oam_warning_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define oam_warning_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)

/* error level log wapper macro */
#define oam_error_log0(_uc_vap_0, _en_feature_0, fmt)
#define oam_error_log1(_uc_vap_0, _en_feature_0, fmt, p1)
#define oam_error_log2(_uc_vap_0, _en_feature_0, fmt, p1, p2)
#define oam_error_log3(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3)
#define oam_error_log4(_uc_vap_0, _en_feature_0, fmt, p1, p2, p3, p4)
#endif

#define oam_info_log_alter(_uc_vap_0, _en_feature_0, fmt, num, args...)
#define oam_warning_log_alter(_uc_vap_0, _en_feature_0, fmt, num, args...)
#define oam_error_log_alter(_uc_vap_0, _en_feature_0, fmt, num, args...)

#define oam_warning_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    ext_diag_warning_log_buff(0, buffer, data_size, fmt, type)
#define oam_error_buf(_uc_vap_0, _en_feature_0, buffer, data_size, fmt, type) \
    ext_diag_error_log_buff(0, buffer, data_size, fmt, type)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oam_ext_if.h */
