/*
 * Copyright (c) @CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: Header file of dmac public interface.
 */

#ifndef __FRW_HMAC_ADAPT_H__
#define __FRW_HMAC_ADAPT_H__

#include "osal_types.h"
#include "oal_net.h"
#include "frw_msg_rom.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_DMAC_ADAPT_H

osal_u32 frw_hmac_send_data(oal_netbuf_stru *netbuf, osal_u8 vap_id, osal_u8 data_type);
osal_u32 frw_hmac_send_data_adapt(oal_netbuf_stru *netbuf, osal_u8 data_type);
#endif // __FRW_HMAC_ADAPT_H__

