/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: netbuf function.
 */

#include "oal_netbuf.h"
#include "oal_skbuff.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_OAL_NETBUF_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

oal_netbuf_stru* oal_netbuf_alloc(td_u32 size, td_u32 reserve, td_u32 align)
{
    oal_netbuf_stru *netbuf;
    td_u32       offset;

    if (align) {
        size += (align - 1);
    }

    // do not use dev_alloc_skb which designed for RX
    netbuf = dev_alloc_skb(size);
    if (OAL_UNLIKELY(netbuf == TD_NULL)) {
        return TD_NULL;
    }
    skb_reserve(netbuf, reserve);

    if (align) {
        offset = (td_s32)(((td_u32)(uintptr_t)netbuf->data) % (td_u32)align);
        if (offset) {
            skb_reserve(netbuf, align - offset);
        }
    }

    return netbuf;
}

td_u32 oal_netbuf_free(oal_netbuf_stru *netbuf)
{
    dev_kfree_skb_any(netbuf);
    return OAL_SUCC;
}

td_void  oal_netbuf_free_any(oal_netbuf_stru *netbuf)
{
    dev_kfree_skb_any(netbuf);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

