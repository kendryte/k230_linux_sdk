/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Initialization and unmount of HAL module.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_main.h"
#include "frw_ext_if.h"
#include "oal_ext_if.h"
#include "hal_chip.h"
#include "hal_mac.h"
#include "hal_power.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_MAIN_C

/*****************************************************************************
 功能描述  : HAL模块初始化总入口，包含HAL模块内部所有特性的初始化
*****************************************************************************/
osal_s32 hal_hmac_main_init(osal_void)
{
    if (hal_chip_init() != OAL_SUCC) {
        return OAL_ERR_INIT_HAL_CHIP;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : HAL模块卸载
*****************************************************************************/
osal_void hal_hmac_main_exit(osal_void)
{
    hal_chip_exit();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
