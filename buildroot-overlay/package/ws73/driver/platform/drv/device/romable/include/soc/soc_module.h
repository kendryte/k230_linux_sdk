/*
 * Copyright (C) CompanyNameMagicTag 2009-2019. All rights reserved.
 * Description: Module ID definition.
 */

#ifndef __SOC_MODULE_H__
#define __SOC_MODULE_H__

#define SOC_INVALID_MODULE_ID (0xffffffff)

typedef enum {
    /* common */ /* CNcomment: 系统通用模块 */
    SOC_ID_RESERVED = 0x00,
    SOC_ID_LIBC = 0x01,
    SOC_ID_BSP = 0x02,
    SOC_ID_SYS = 0x03,
    SOC_ID_MAIN = 0x04,
    
    /* Peripheral */ /* CNcomment: 外设相关模块 */
    SOC_ID_VAU = 0x05,
    SOC_ID_DPU = 0x06,
    SOC_ID_AIDSP = 0x07,
    SOC_ID_CGRA = 0x08,

    /* Component */ /* CNcomment: 组件相关模块 */
    DIAG_MOD_ID_WIFI_HOST               = 0x10,
    DIAG_MOD_ID_WIFI_DEVICE             = 0x11,
    DIAG_MOD_ID_OAM_HOST                = 0x12,
    DIAG_MOD_ID_BSLE_HOST               = 0x15,
    DIAG_MOD_ID_BSLE_DEVICE             = 0x16,

    DIAG_MOD_ID_PLC_NM                  = 0x20,

    SOC_ID_MAX = 0x3F
} soc_mod_id;

#endif /* __SOC_MODULE_H__ */

