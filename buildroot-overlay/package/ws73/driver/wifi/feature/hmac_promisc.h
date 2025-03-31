/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hook function for wifi promiscuous.
 * Create: 2021-8-14
 */

#ifndef __HMAC_PROMISC_H__
#define __HMAC_PROMISC_H__

#include "osal_adapt.h"
#include "mac_frame.h"
#include "mac_vap_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_PROMISC_H

#define IEEE80211_MAC_HDR_LEN 26
#define TRIM_DATA_PAYLOAD_LEN 2000
#define IEEE80211_FRAME_TYPE_DATA 0x02
#define IEEE80211_FRAME_TYPE_MGMT 0x00
#define IEEE80211_FRAME_SUBTYPE_QOS 0x08
#define IEEE80211_FRAME_SUBTYPE_DISASSOC 0x0a

#define MAC_ADDR_LEN 6

#define DIRECTION_TX 0
#define DIRECTION_RX 1

/* WLAN 混杂模式状态结构体 */
typedef struct {
    osal_u32 promis_count[3][16][2];           /* 3 for frame type, 15 for frame subtype, 2 for frame mode */
} wlan_promisc_mode_stru; /* splited from proc_file_config_struct */

typedef struct {
    osal_u8 bit_num;
    osal_u8 frame_type;
    osal_u8 subtype_num;
    osal_u8 frame_mode;
} wlan_promisc_query_stru;

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
typedef osal_void (*hmac_promis_send_custom_frame_cb)(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf);
#endif
typedef osal_void (*hmac_rx_frame_promis_proc_cb)(hmac_vap_stru *hmac_vap, const oal_netbuf_stru *netbuf);
typedef osal_u32 (*hmac_sta_rx_deauth_req_proc_for_promisc_cb)(hmac_vap_stru *hmac_vap, const osal_u8 *mac_hdr);
typedef osal_u8 (*hmac_promisc_get_switch_cb)(void);

static osal_u32 hmac_promis_init_weakref(osal_void) __attribute__ ((weakref("hmac_promis_init"), used));
static osal_void hmac_promis_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_promis_deinit"), used));

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
