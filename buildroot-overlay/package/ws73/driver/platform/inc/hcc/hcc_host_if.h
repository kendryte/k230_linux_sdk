/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: hcc driver implementatioin.
 * Author: Huanghe
 * Create: 2020-10-20
 */

#ifndef __HCC_HOST_IF_H__
#define __HCC_HOST_IF_H__

#include "osal_types.h"

typedef enum _hcc_chan_type_ {
    HCC_TX = 0x0,
    HCC_RX = 0x1,
    HCC_CHAN_DIR_COUNT
} hcc_chan_type;

typedef enum _plat_reboot_notify_type_ {
    WLAN_REBOOT_NOTIFY,
    REBOOT_NOTIFY_COUNT
} plat_reboot_notify_type;

typedef osal_void (*reboot_notify)(osal_void);
osal_void plat_register_reboot(reboot_notify func, plat_reboot_notify_type type);
#endif /* end of oal_hcc_if.h */
