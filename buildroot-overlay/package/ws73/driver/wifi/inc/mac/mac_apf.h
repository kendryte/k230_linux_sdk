/*
 * Copyright (c) CompanyNameMagicTag 2022-2023. All rights reserved.
 * Description: APF mac header.
 * Author: Huang
 * Create: 2022-10-14
 */

#ifndef MAC_APF_H
#define MAC_APF_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "frw_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_APF_H

#ifdef _PRE_WLAN_FEATURE_APF
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define APF_PROGRAM_MAX_LEN 512

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    APF_SET_FILTER_CMD,
    APF_GET_FILTER_CMD,
    APF_FILTER_CMD_BUFF
} mac_apf_cmd_type_enum;
typedef osal_u8 mac_apf_cmd_type_uint8;

typedef enum {
    OSAL_SWITCH_OFF  = 0,
    OSAL_SWITCH_ON   = 1,
    OSAL_SWITCH_BUTT
} osal_switch_enum;

typedef struct {
    mac_apf_cmd_type_uint8 cmd_type;
    osal_u16 program_len;
    osal_u8 *program;
} mac_apf_filter_cmd_stru;

#endif /* end of _PRE_WLAN_FEATURE_APF */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_apf.h */