/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: general purpose register header file.
 * Create: 2021-07-31
 */

#ifndef __HAL_GP_REG_H__
#define __HAL_GP_REG_H__

#include "osal_types.h"

#define GLB_CTRL_RB_BASE_ADDR   (0x40019000)
#define GLB_CTRL_RB_BASE_ADDR_0x380    (GLB_CTRL_RB_BASE_ADDR + 0x380)
#define GLB_CTRL_RB_BASE_ADDR_0x384    (GLB_CTRL_RB_BASE_ADDR + 0x384)
#define GLB_CTRL_RB_BASE_ADDR_0x388    (GLB_CTRL_RB_BASE_ADDR + 0x388)
#define GLB_CTRL_RB_BASE_ADDR_0x38C    (GLB_CTRL_RB_BASE_ADDR + 0x38C)
#define GLB_CTRL_RB_BASE_ADDR_0x390    (GLB_CTRL_RB_BASE_ADDR + 0x390)
#define GLB_CTRL_RB_BASE_ADDR_0x394    (GLB_CTRL_RB_BASE_ADDR + 0x394)
#define GLB_CTRL_RB_BASE_ADDR_0x398    (GLB_CTRL_RB_BASE_ADDR + 0x398)
#define GLB_CTRL_RB_BASE_ADDR_0x39c    (GLB_CTRL_RB_BASE_ADDR + 0x39c)
#define GLB_CTRL_RB_BASE_ADDR_0x404    (GLB_CTRL_RB_BASE_ADDR + 0x404)

#define GLB_COMMON_REG_NUM      9
#define GLB_COMMON_REG_ADDR_MIN (GLB_CTRL_RB_BASE_ADDR_0x380)
#define GLB_COMMON_REG_ADDR_MAX (GLB_CTRL_RB_BASE_ADDR_0x39c)
osal_void hal_gp_set_btcoex_wifi_status(osal_u32 val);
osal_u32 hal_gp_get_btcoex_wifi_status(osal_void);
osal_u32 hal_gp_get_btcoex_bt_status(osal_void);
osal_u32 hal_gp_get_btcoex_abort_time(osal_void);
#endif