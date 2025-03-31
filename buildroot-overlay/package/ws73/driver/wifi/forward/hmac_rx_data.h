/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file for hmac_rx_data.c.
 * Create: 2021-12-15
 */

#ifndef __HMAC_RX_DATA_H__
#define __HMAC_RX_DATA_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_resource.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_DATA_H

osal_u32 hmac_rx_data(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);

osal_void hmac_rx_process_data_ap_tcp_ack_opt_etc(oal_netbuf_head_stru *netbuf_header);
osal_u32 hmac_rx_process_data_sta_tcp_ack_opt_etc(hmac_vap_stru *hmac_vap,
    oal_netbuf_head_stru *netbuf_header);
osal_void hmac_rx_free_netbuf_list_etc(oal_netbuf_head_stru *netbuf_hdr, osal_u16 nums_buf);

osal_u32 hmac_rx_lan_frame_etc(oal_netbuf_head_stru *netbuf_header);
osal_void hmac_rx_lan_frame_classify_transmit_msdu_to_lan(hmac_vap_stru *hmac_vap, hmac_msdu_stru *msdu,
    oal_netbuf_head_stru *w2w_netbuf_hdr, hmac_user_stru *hmac_user);
osal_void hmac_rx_transmit_msdu_to_lan_promis(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_rx_data.h */
