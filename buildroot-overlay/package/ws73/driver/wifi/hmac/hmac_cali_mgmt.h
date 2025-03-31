/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: Header file for hmac_cali_mgmt.h.
 */

#ifndef __HMAC_CALI_MGMT_H__
#define __HMAC_CALI_MGMT_H__

#if defined(_PRE_PRODUCT_ID_HOST)
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "frw_ext_if.h"
#include "dmac_ext_if_hcm.h"
#include "hmac_vap.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_cali.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#ifndef CONFIG_NO_SUPPORT_INI
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_H
typedef struct {
    osal_u16 cali_data_len;
    osal_u8 cali_data_received;
    osal_u8 rsv;
} hmac_cali_data_header_stru;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_s32 hmac_save_cali_event_etc(osal_u8 *data, osal_u16 data_len);
extern osal_u32 hmac_send_cali_data_etc(hmac_vap_stru *hmac_vap);
osal_void hmac_init_cali_data_buffer_exit(osal_void);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
#endif /* end of hmac_mgmt_classifier.h */

