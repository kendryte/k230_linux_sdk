/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: netbuf enum type head file.
 * Create: 2022-07-04
 */

#ifndef OAL_NETBUF_COMMON_ROM_H
#define OAL_NETBUF_COMMON_ROM_H

#include "osal_types.h"
#include "frw_msg_rom.h"
#include "frw_hcc_common_rom.h"
#include "oal_net_pkt_rom.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/* 报文驱动私有扩展头封装格式示意：低地址|HCC头|FRW头|CB头|MAC头|Payload|高地址 */
#define OAL_HCC_HDR_LEN      hcc_get_head_len()
#define OAL_FRW_HDR_LEN      frw_get_head_len()
#define OAL_MAX_CB_LEN       32 /* 不能超过48字节，内核的CB数组只有48字节 */
#define OAL_MAX_MAC_HDR_LEN  36 /* 预留给MAC头的最大空间，考虑4地址场景 */
#define OAL_HDR_TOTAL_LEN    (OAL_HCC_HDR_LEN + OAL_FRW_HDR_LEN + OAL_MAX_CB_LEN + OAL_MAX_MAC_HDR_LEN)
/* 修改OAL_HDR_TOTAL_LEN 需要同步修改 OAL_MACHDR_LEN */

#define WLAN_MEM_NETBUF_ALRX                1600      /* 常发常收内存大小 */

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of oal_netbuf_common.h */