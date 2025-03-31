/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: 和CHR相关的接口函数实现
 * Create: 2021-4-10
 */

#ifndef __HMAC_CHR_H__
#define __HMAC_CHR_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "frw_ext_if.h"
#include "frw_hmac.h"
#include "hal_mac_reg.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CHR_H

#ifdef _PRE_WLAN_FEATURE_DFX_CHR
typedef struct {
    osal_u8 sub_type;
    osal_char *sub_type_name;
    osal_u8 reserved[2];
} chr_err_code_info_stru;
typedef osal_u8 chr_cali_err_code_enum_uint8;
osal_s32 hmac_chr_event_etc(osal_u8 err_code);
osal_s32 hmac_config_chr_err_code_test(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_chr.h */
