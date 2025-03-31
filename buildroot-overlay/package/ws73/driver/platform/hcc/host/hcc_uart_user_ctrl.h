/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 *
 * Description: UART octty header file
 * Author: @CompanyNameTag
 */

#ifndef __HCC_UART_USER_CTRL_H__
#define __HCC_UART_USER_CTRL_H__
#ifdef CONFIG_HCC_SUPPORT_UART
/*****************************************************************************
  1 Include other Head file
*****************************************************************************/

/*****************************************************************************
  2 Define macro
*****************************************************************************/
#define SNPRINT_LIMIT_TO_KERNEL (512)

/*****************************************************************************
  5 EXTERN FUNCTION
*****************************************************************************/
td_s32 bsle_user_ctrl_init(void);
extern void bfgx_user_ctrl_exit(void);
struct kobject *hcc_get_sys_object(td_void);
td_bool is_occtty_ready(td_void);
#endif
#endif

