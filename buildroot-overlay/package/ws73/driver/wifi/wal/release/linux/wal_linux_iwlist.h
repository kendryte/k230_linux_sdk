/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: linux ioctl iwlist头文件
 * Create: 2022年12月26日
 */

#ifndef WAL_LINUX_IWLIST_H
#define WAL_LINUX_IWLIST_H

/*****************************************************************************
  1 其他头文件包
*****************************************************************************/
#ifdef _PRE_WLAN_WIRELESS_EXT
#include "wal_config.h"
#include "hmac_ext_if.h"
#include "oal_net.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SCAN_ITEM_SIZE 768
#define PHYRATE_STREAM_MAX 8
#define HE_PHYRATE_STREAM_MAX 16
#define MAX_CUSTOM_LEN 64
#define WLAN_CAPABILITY_BSS (1 << 0)
#define IW_SCAN_DATA_BUFF_MAX (4096 * 8)
#define IW_SCAN_DER_LIMIT_BUFF_MAX (IW_SCAN_DATA_BUFF_MAX - SCAN_ITEM_SIZE * 2) /* 预留足够空间，以防超过iwlist范围 */
#define IW_SCAN_THIS_ESSID 0x0002
#define IW_SCAN_HACK 0x8000
#define IW_ESSID_MAX_SIZE 32

/* 内核所定义的 */
#define IW_QUAL_QUAL_INVALID    0x10
#define IW_QUAL_NOISE_INVALID   0x40
#define IW_QUAL_RCPI            0x80

struct iw_request_info;
osal_s8 *iwlist_translate_scan(mac_bss_dscr_stru *bss_dscr, struct iw_request_info *info,
    osal_s8 *start, osal_s8 *stop);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_iwlist.h */
#endif
