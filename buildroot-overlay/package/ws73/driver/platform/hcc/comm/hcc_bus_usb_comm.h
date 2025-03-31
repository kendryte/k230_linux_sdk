/*
 * Copyright (c) CompanyNameMagicTag. 2020-2021. All rights reserved.
 * Description: usb device&host common header
 * Author: CompanyName
 * Create: 2021-10-31
 */

#ifndef HCC_BUS_USB_COMM_HEADER
#define HCC_BUS_USB_COMM_HEADER

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HIUSB_DEV2HOST_SCATT_MAX 24 // 40->24 for reduce ram
#define HIUSB_HOST2DEV_SCATT_MAX 24 // 40->24 for reduce ram
#define HIUSB_PACKAGE_HEARDER_SIZE 92

#define DEIVICE_VENDOR_ID 0xFFFF
#define DEIVICE_PRODUCT_ID 0x3733

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // HCC_BUS_USB_COMM_HEADER

