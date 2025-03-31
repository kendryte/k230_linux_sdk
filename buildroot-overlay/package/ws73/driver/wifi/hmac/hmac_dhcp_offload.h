/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * 文 件 名   : hmac_dhcp_offload.h
 * 生成日期   : 2022年10月14日
 * 功能描述   : hmac_dhcp_offload.c的头文件
 */

#ifndef __HMAC_DHCP_OFFLOAD_H__
#define __HMAC_DHCP_OFFLOAD_H__
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "msg_dhcp_offload_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_DHCP_OFFLOAD_H

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/* DHCP options */
enum dhcp_opt {
    DHCP_OPT_PAD                    = 0,
    DHCP_OPT_SUBNETMASK             = 1,
    DHCP_OPT_ROUTER                 = 3,
    DHCP_OPT_DNSSERVER              = 6,
    DHCP_OPT_HOSTNAME               = 12,
    DHCP_OPT_DNSDOMAIN              = 15,
    DHCP_OPT_MTU                    = 26,
    DHCP_OPT_BROADCAST              = 28,
    DHCP_OPT_STATICROUTE            = 33,
    DHCP_OPT_NISDOMAIN              = 40,
    DHCP_OPT_NISSERVER              = 41,
    DHCP_OPT_NTPSERVER              = 42,
    DHCP_OPT_VENDOR                 = 43,
    DHCP_OPT_IPADDRESS              = 50,
    DHCP_OPT_LEASETIME              = 51,
    DHCP_OPT_OPTIONSOVERLOADED      = 52,
    DHCP_OPT_MESSAGETYPE            = 53,
    DHCP_OPT_SERVERID               = 54,
    DHCP_OPT_PARAMETERREQUESTLIST   = 55,
    DHCP_OPT_MESSAGE                = 56,
    DHCP_OPT_MAXMESSAGESIZE         = 57,
    DHCP_OPT_RENEWALTIME            = 58,
    DHCP_OPT_REBINDTIME             = 59,
    DHCP_OPT_VENDORCLASSID          = 60,
    DHCP_OPT_CLIENTID               = 61,
    DHCP_OPT_USERCLASS              = 77,  /* RFC 3004 */
    DHCP_OPT_FQDN                   = 81,
    DHCP_OPT_DNSSEARCH              = 119, /* RFC 3397 */
    DHCP_OPT_CSR                    = 121, /* RFC 3442 */
    DHCP_OPT_SIXRD                  = 212, /* RFC 5969 */
    DHCP_OPT_MSCSR                  = 249, /* MS code for RFC 3442 */
    DHCP_OPT_END                    = 255
};

#define TIMER_SECONDS_TO_MS     1000

static osal_u32 hmac_dhcp_offload_init_weakref(osal_void)
__attribute__ ((weakref("hmac_dhcp_offload_init"), used));
static osal_void hmac_dhcp_offload_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_dhcp_offload_deinit"), used));

osal_void hmac_dhcp_offload_get_param(mac_dhcp_offload_param_sync *dhcp_param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif