/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: Header file of hal_vap.c.
 * Create: 2023-01-11
 */

#ifndef HAL_VAP_H
#define HAL_VAP_H

#include "hal_common_ops.h"
#include "wlan_types_base_rom.h"
#include "hal_ext_if.h"
#include "hal_device.h"
#include "hal_mac_reg_field.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
 函 数 名  : hh503_get_vap_internal
 功能描述  : HAL内部获取VAP
*****************************************************************************/
static INLINE__ osal_void hh503_get_vap_internal(const hal_device_stru *device, osal_u8 vap_id,
    hal_to_dmac_vap_stru **hal_vap)
{
    /* 检查传入的vap id是否合理 */
    if (vap_id >= HAL_MAX_VAP_NUM) {
        *hal_vap = OAL_PTR_NULL;
        return;
    }
    *hal_vap = (hal_to_dmac_vap_stru *)(device->vap_list[vap_id]);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hal_vap.h */
