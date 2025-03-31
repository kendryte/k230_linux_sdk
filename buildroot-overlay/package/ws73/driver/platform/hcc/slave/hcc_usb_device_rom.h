/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: usb device interface
 * Author: CompanyName
 * Create: 2020-05-02
 */

#ifndef HCC_USB_DEVICE_ROM_H
#define HCC_USB_DEVICE_ROM_H

#ifdef CONFIG_HCC_SUPPORT_USB

#include "osal_types.h"
#include "hcc_bus_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_void hcc_usb_queue_switch_count(osal_void);
osal_void hcc_usb_set_trans_count(hcc_queue_dir chan, osal_u32 cnt);
osal_void hcc_usb_set_misss_count(hcc_queue_dir chan, osal_u32 cnt);
osal_void hcc_usb_update_credit(hcc_bus *bus, osal_u32 free_cnt);
osal_u32 hcc_usbd_get_credit(osal_void);
osal_void hcc_usbd_set_rx_miss_pkt(osal_u32 miss_cnt);
hcc_bus *hcc_usb_soft_res_init(bus_dev_ops *usb_hcc_ops, osal_u32 rsv_len);

osal_s32 hcc_usb_reserv_buff_alloc(hcc_bus *usb_bus, osal_u32 resv_len);

osal_void hcc_usb_reserv_buff_free(hcc_bus *usb_bus);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // CONFIG_HCC_SUPPORT_USB

#endif

