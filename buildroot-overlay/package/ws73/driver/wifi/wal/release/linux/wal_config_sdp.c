/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Function related to wifi aware(NAN) feature.
 * Create: 2021-04-16
 */

#include "wal_config_sdp.h"
#include "hmac_sdp.h"
#include "wal_linux_cfgvendor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 接收底层上报的SDP数据信息传递给用户层回调函数，包括接收到的publish和follow up报文 */
osal_s32 wal_sdp_process_rx_data(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    oal_sdp_info *sdp_param = OAL_PTR_NULL;
    oal_wireless_dev_stru *wdev = OSAL_NULL;

    /* 获取用户数据结构体指针 */
    if ((hmac_vap == OSAL_NULL) || (hmac_vap->net_device == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_SDP, "wal_sdp_process_rx_data:vap null");
        return OAL_ERR_CODE_PTR_NULL;
    }

    wdev = hmac_vap->net_device->ieee80211_ptr;
    if (wdev == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SDP, "vap[%d]wal_sdp_process_rx_data:wdev null", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    sdp_param = (oal_sdp_info *)msg->data;
    if (sdp_param->data == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_SDP, "vap[%d]wal_sdp_process_rx_data:data null", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_info_log3(0, OAM_SF_SDP, "vap[%d] wal_sdp_process_rx_data come in, len %d, data[0][%d]",
        hmac_vap->vap_id, sdp_param->len, sdp_param->data[0]);
#if defined (_PRE_PRODUCT_ID_HOST) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    wal_cfgvendor_nan_event_report(wdev->wiphy, wdev, sdp_param->len, sdp_param->data);
#endif
    oal_mem_free(sdp_param->data, OAL_TRUE);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
