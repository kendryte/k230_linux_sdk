/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: HMAC reset processing file.
 * Create: 2021-12-15
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_reset.h"
#include "mac_device_ext.h"
#include "mac_resource_ext.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_RESET_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*****************************************************************************
 功能描述  : hmac接收dmac抛回来的查询应答事件
*****************************************************************************/
static osal_void hmac_proc_query_response_event_ext(hmac_stat_info_rsp_stru *rsp, hmac_vap_stru *hmac_vap)
{
    hmac_vap->station_info_extend.distance = rsp->station_info_extend.distance;
    hmac_vap->station_info_extend.cca_intr = rsp->station_info_extend.cca_intr;
    hmac_vap->station_info_extend.snr_ant0  = rsp->station_info_extend.snr_ant0;
    hmac_vap->station_info_extend.snr_ant1  = rsp->station_info_extend.snr_ant1;
}

/*****************************************************************************
 功能描述  : hmac接收dmac抛回来的查询应答事件
*****************************************************************************/
osal_u32 hmac_proc_query_response_event(hmac_vap_stru *hmac_vap, osal_u8 len, osal_u8 *param)
{
    hmac_stat_info_rsp_stru *rsp = OSAL_NULL;
    unref_param(len);
    rsp = (hmac_stat_info_rsp_stru *)(param);
    hmac_vap->station_info.signal     = rsp->signal;
    hmac_vap->station_info.rx_packets = rsp->rx_packets;
    hmac_vap->station_info.tx_packets = rsp->tx_packets;
    hmac_vap->station_info.rx_bytes   = rsp->rx_bytes;
    hmac_vap->station_info.tx_bytes   = rsp->tx_bytes;
#ifdef KERNEL_4_9_ADAP
    hmac_vap->station_info.noise            = rsp->free_power;
    hmac_vap->station_info.chload           = rsp->chload;
#endif
    hmac_vap->station_info.tx_retries       = rsp->tx_retries;
    hmac_vap->station_info.rx_dropped_misc  = rsp->rx_dropped_misc;
    hmac_vap->station_info.tx_failed        = rsp->tx_failed;
    hmac_vap->station_info.txrate.mcs       = rsp->txrate.mcs;
    hmac_vap->station_info.txrate.legacy    = rsp->txrate.legacy;
    hmac_vap->station_info.txrate.nss       = rsp->txrate.nss;
    hmac_vap->station_info.txrate.flags     = rsp->txrate.flags;
    hmac_proc_query_response_event_ext(rsp, hmac_vap);
    return OAL_SUCC;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

