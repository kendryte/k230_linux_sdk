/*
 * Copyright (c) CompanyNameMagicTag. 2021-2023. All rights reserved.
 * Description: Header file of device and host frw_hcc_common public interface.
 * Author: AuthorNameMagicTag
 * Create: 2021-09-16
 */

#ifndef __FRW_HCC_COMMMON_ROM_H__
#define __FRW_HCC_COMMMON_ROM_H__

#include "osal_types.h"
#include "hcc_cfg_comm.h"
#ifndef _PRE_WLAN_FEATURE_CENTRALIZE
#include "hcc_if.h"
#include "hcc_comm.h"
#endif

/*****************************************************************************
  1 宏定义
*****************************************************************************/

/*****************************************************************************
  2 枚举定义
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_CENTRALIZE
#define HCC_HEAD_SIZE 4
static inline osal_u16 hcc_get_head_len()
{
    return HCC_HEAD_SIZE;
}
#endif

#endif // __FRW_HCC_COMMMON_ROM_H__
