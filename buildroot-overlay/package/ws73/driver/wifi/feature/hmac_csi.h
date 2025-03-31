/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: hmac_csi
 * Author:
 * Create: 2022-09-20
 */

#ifndef __HMAC_CSI_H__
#define __HMAC_CSI_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "mac_device_ext.h"
#include "hmac_device.h"
#include "hal_csi.h"
#include "wlan_types_common.h"
#include "hmac_feature_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CSI_H

/*****************************************************************************
  2 函数声明
*****************************************************************************/
typedef osal_void (*hmac_csi_exit_cb)(osal_void);

static osal_u32 hmac_csi_init_weakref(osal_void) __attribute__ ((weakref("hmac_csi_init"), used));
static osal_void hmac_csi_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_csi_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of dmac_csi_rom.h */
