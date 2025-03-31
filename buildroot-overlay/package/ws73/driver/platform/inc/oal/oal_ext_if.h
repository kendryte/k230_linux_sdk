/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: oal extern header file.
 * Author: Huanghe
 * Create: 2020-10-13
 */

#ifndef __OAL_EXT_IF_H__
#define __OAL_EXT_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_ext_util.h"
#include "oal_net.h"
#include "oal_list.h"
#include "oal_mm.h"
/* end infusion */
#if defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC) && defined(_PRE_MULTI_CORE_MODE)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif
#endif
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "arch/oal_workqueue.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum
{
    OAL_TRACE_ENTER_FUNC,
    OAL_TRACE_EXIT_FUNC,

    OAL_TRACE_DIRECT_BUTT
}oal_trace_direction_enum;
typedef oal_uint8 oal_trace_direction_enum_uint8;

#if defined(_PRE_WLAN_FEATURE_BLACKLIST_LEVEL) && defined(_PRE_WLAN_FEATURE_BLACKLIST_NONE) && \
    (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)

typedef oal_uint8 cs_blacklist_mode_enum_uint8;
#endif

#define OAL_WIFI_REPORT_STA_ACTION(_ul_ifindex, _ul_eventID, _p_arg, _l_size)

extern oal_uint32 oal_chip_get_version_etc(oal_void);
extern oal_uint8 oal_chip_get_device_num_etc(oal_uint32 ul_chip_ver);
extern oal_uint8 oal_board_get_service_vap_start_id(oal_void);

#ifdef _PRE_DEBUG_PROFILING
typedef int (*cyg_check_hook_t)(long this_func, long call_func, long direction);
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
static inline void reinit_completion(struct completion *x)
{
    x->done = 0;
}
#endif
#endif

#define DEFINE_GET_BUILD_VERSION_FUNC(mod)
#define GET_BUILD_VERSION_FUNC(mod)
#define OAL_RET_ON_MISMATCH(_mod, ret)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_ext_if.h */
