/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal and hmac public interface.
 * Author: Huanghe
 * Create: 2021-07-01
 */

/*****************************************************************************
    头文件包含
*****************************************************************************/
#include "frw_hmac.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_FRW_COMMON_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_void cfg_msg_init(osal_u8 *data, osal_u32 data_len, osal_u8 *rsp, osal_u32 rsp_buf_len, frw_msg *cfg_info)
{
    cfg_info->data_len = (osal_u16)data_len;
    cfg_info->data = data;
    cfg_info->rsp = rsp;
    cfg_info->rsp_buf_len = (osal_u16)rsp_buf_len;
    return;
}

osal_s32 send_cfg_to_device(osal_u8 vap_id, osal_u16 msg_id, frw_msg *cfg_info, osal_bool sync)
{
    osal_s32 ret;

    ret = frw_send_cfg_to_device(msg_id, vap_id, sync, TIME_OUT_MS, cfg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "{msg_id[%d] send_cfg_to_device::frw_send_cfg_to_device failed[%d].}", msg_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

osal_s32 send_sync_cfg_to_host(osal_u8 vap_id, osal_u16 msg_id, frw_msg *cfg_info)
{
    osal_s32 ret;

    ret = frw_sync_host_post_msg(msg_id, vap_id, TIME_OUT_MS, cfg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log2(0, OAM_SF_CFG,
            "{msg_id[%d] send_cfg_to_host::frw_host_post_msg failed[%d].}", msg_id, ret);
        return ret;
    }

    return OAL_SUCC;
}

osal_void frw_event_process_all_event_etc(osal_ulong data)
{
    unref_param(data);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
