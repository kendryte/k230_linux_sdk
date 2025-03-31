/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: netbuf enum type head file.
 * Create: 2022-11-26
 */

#ifndef MAC_USER_COMMON_H
#define MAC_USER_COMMON_H

#include "mac_user_common_rom.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

#define MAC_INVALID_RA_LUT_IDX WLAN_ACTIVE_USER_MAX_NUM                    /* 不可用的RA LUT IDX */

/*****************************************************************************
 函 数 名  : mac_user_get_ra_lut_index
 功能描述  : 获取用户对应的ra lut index

*****************************************************************************/
static inline osal_u8 mac_user_get_ra_lut_index(osal_u8 *index_table, osal_u16 start, osal_u16 stop)
{
    return oal_get_lut_index(index_table, WLAN_ACTIVE_USER_IDX_BMAP_LEN, MAC_INVALID_RA_LUT_IDX, start, stop);
}
/*****************************************************************************
 函 数 名  : mac_user_del_ra_lut_index
 功能描述  : 删除用户对应的ra lut index

*****************************************************************************/
static inline osal_void mac_user_del_ra_lut_index(osal_u8 *index_table, osal_u8 ra_lut_index)
{
    oal_del_lut_index(index_table, ra_lut_index);
}
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of mac_user_common.h */