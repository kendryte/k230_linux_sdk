/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: usb device interface
 * Author: CompanyName
 * Create: 2020-05-02
 */

#ifndef HCC_USB_DEVICE_HEADER
#define HCC_USB_DEVICE_HEADER

#ifdef CONFIG_HCC_SUPPORT_USB

#include "osal_types.h"
#include "hcc_bus_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

hcc_bus *hcc_usb_device_init(osal_void);

osal_void hcc_usb_device_deinit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // CONFIG_HCC_SUPPORT_USB

#endif

