/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: host plat hcc service
 */
#ifndef __PLAT_HCC_SRV_H__
#define __PLAT_HCC_SRV_H__
#include "td_base.h"
#include "plat_hcc_msg_type.h"
#include "hcc_if.h"

typedef td_u32 (*hcc_plat_msg_callbck)(td_u8 *data, osal_u16 len);
td_u32 hcc_plat_msg_register(td_u16 msg_id, hcc_plat_msg_callbck cb);
td_u32 plat_msg_hcc_send(td_u8 *buf, td_u16 len, td_u16 hcc_plat_msg_id);
static inline td_u32 oal_round_up(td_u16 _old_len, td_u16 _align)
{
    return ((((_old_len) + ((_align)-1)) / (_align)) * (_align));
}

td_u32 hcc_plat_host_rx_proc(hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len, td_u8 *user_param);

#endif