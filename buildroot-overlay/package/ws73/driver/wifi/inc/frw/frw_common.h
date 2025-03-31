/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: frw adapter
 */

#ifndef __FRW_COMMON_H
#define __FRW_COMMON_H

#include "osal_types.h"
#include "frw_msg_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
osal_void cfg_msg_init(osal_u8 *data, osal_u32 data_len, osal_u8 *rsp, osal_u32 rsp_buf_len, frw_msg *cfg_info);
osal_s32 send_cfg_to_device(osal_u8 vap_id, osal_u16 msg_id, frw_msg *cfg_info, osal_bool sync);
osal_s32 send_sync_cfg_to_host(osal_u8 vap_id, osal_u16 msg_id, frw_msg *cfg_info);

osal_void frw_event_process_all_event_etc(osal_ulong data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
