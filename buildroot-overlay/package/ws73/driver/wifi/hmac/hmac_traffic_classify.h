/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file for hmac_traffic_classify.c.
 * Create: 2021-12-15
 */

#ifndef __HMAC_TRAFFIC_CLASSIFY_H__
#define __HMAC_TRAFFIC_CLASSIFY_H__

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN

/*****************************************************************************
  1头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_tx_data.h"
#include "hmac_tx_amsdu.h"
#include "mac_frame.h"
#include "mac_data.h"
#include "hmac_frag.h"
#include "hmac_11i.h"
#include "hmac_user.h"

#include "hmac_m2u.h"

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
#include "hmac_proxy_arp.h"
#endif

#include "hmac_device.h"
#include "hmac_resource.h"

#include "hmac_tcp_opt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TRAFFIC_CLASSIFY_H

/*****************************************************************************
  2宏定义
*****************************************************************************/


/*****************************************************************************
  3结构体
*****************************************************************************/

/*****************************************************************************
    用户结构体: 包含了已识别业务、待识别业务序列
    这里借助hmac_user_stru，在hmac_user_stru结构体中添加宏定义字段:
    _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
*****************************************************************************/

/*****************************************************************************
    RTP头结构体:结构体中没有包含最后的CSRC标识符
*****************************************************************************/
typedef struct {
    /*---------------------------------------------------------------------------
                                    RTP头结构
    -----------------------------------------------------------------------------
    |version|P|X|   CSRC数   |M|          PT           |       序号             |
    |  2bit |1|1|    4bit    |1|        7bit           |         16bit          |
    -----------------------------------------------------------------------------
    |                               时间戳 32bit                                |
    -----------------------------------------------------------------------------
    |                                 SSRC 32bit                                |
    -----------------------------------------------------------------------------
    |               CSRC 每个CSRC标识符32bit 标识符个数由CSRC数决定             |
    ---------------------------------------------------------------------------*/

    osal_u8       version_and_csrc;    /* 版本号2bit、填充位(P)1bit、扩展位(X)1bit、CSRC数目4bit */
    osal_u8       payload_type;        /* 标记1bit、有效载荷类型(PT)7bit */
    osal_u16      rtp_idx;             /* RTP发送序号 */
    osal_u32      rtp_time_stamp;      /* 时间戳 */
    osal_u32      ssrc;                /* SSRC */
} hmac_tx_rtp_hdr;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
extern osal_void hmac_tx_traffic_classify_etc(mac_tx_ctl_stru     *tx_ctl,
    mac_ip_header_stru  *pst_ip,
    osal_u8           *puc_tid);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* endif _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN */

#endif  /* end of __HMAC_TRAFFIC_CLASSIFY_H__ */
