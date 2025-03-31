/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc adapt usb.
 * Author: CompanyName
 * Create: 2021-07-26
 */

#ifndef HCC_ADAPT_USB_HEADER
#define HCC_ADAPT_USB_HEADER

#ifdef CONFIG_HCC_SUPPORT_USB

#include "osal_types.h"
#include "hcc_bus_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

hcc_bus *hcc_adapt_usb_load(osal_void);

osal_void hcc_adapt_usb_unload(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // CONFIG_HCC_SUPPORT_USB

#endif
