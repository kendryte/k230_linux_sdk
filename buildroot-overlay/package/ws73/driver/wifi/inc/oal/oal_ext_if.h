/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: oal对外公共接口头文件
 * Create: 2012年9月20日
 */

#ifndef __OAL_EXT_IF_H__
#define __OAL_EXT_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_util.h"
#include "oal_schedule.h"
#include "oal_net.h"
#include "oal_list.h"
#include "oal_queue.h"
#include "diag_log_common.h"

#include "frw_util.h"
#include "oal_fsm.h"

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "oal_fsm.h"
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_exception_rst.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 黑名单模式 */
typedef enum {
    CS_BLACKLIST_MODE_NONE,            /* 关闭         */
    CS_BLACKLIST_MODE_BLACK,           /* 黑名单       */
    CS_BLACKLIST_MODE_WHITE,           /* 白名单       */

    CS_BLACKLIST_MODE_BUTT
}cs_blacklist_mode_enum;
typedef osal_u8 cs_blacklist_mode_enum_uint8;

typedef enum {
    OAL_WIFI_STA_LEAVE          = 0,    // STA 离开
    OAL_WIFI_STA_JOIN           = 1,    // STA 加入

    OAL_WIFI_BUTT
}oal_wifi_sta_action_report_enum;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
static inline osal_u8 oal_bus_get_chip_num_etc(osal_void)
{
    return 1;
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_s32   oal_main_init_etc(osal_void);
extern osal_void    oal_main_exit_etc(osal_void);

#define DEFINE_GET_BUILD_VERSION_FUNC(mod)
#define GET_BUILD_VERSION_FUNC(mod)
#define OAL_RET_ON_MISMATCH(_mod, ret)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_ext_if.h */
