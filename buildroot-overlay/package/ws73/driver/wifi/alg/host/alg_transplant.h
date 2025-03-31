/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: algorithm transfer platform
 */

#ifndef __ALG_TRANSPLANT_H__
#define __ALG_TRANSPLANT_H__

/******************************************************************************
  1 其他头文件包含
******************************************************************************/
#include "wlan_types_common.h"
#include "oal_ext_if.h"
#include "dmac_ext_if_hcm.h"
#include "wlan_spec.h"
#include "alg_main.h"
#include "hmac_device.h"
#include "oam_ext_if.h"

#include "hal_device.h"

/******************************************************************************
  2 宏定义
******************************************************************************/
/* 系统时间封装 */

typedef osal_time_us_stru alg_time_us_stru;

/* 遍历VAP/USER操作封装 */
#define alg_device_foreach_vap(_pst_vap, _pst_device, _uc_vap_index)                  \
    for ((_uc_vap_index) = 0, (_pst_vap) = ((_pst_device)->vap_num > 0) ?             \
        ((hmac_vap_stru *)mac_res_get_hmac_vap((_pst_device)->vap_id[0])) :             \
        OSAL_NULL;                                                                      \
        (_uc_vap_index) < (_pst_device)->vap_num;                                     \
        (_uc_vap_index)++, (_pst_vap) = ((_uc_vap_index) < (_pst_device)->vap_num) ?  \
        ((hmac_vap_stru *)mac_res_get_hmac_vap((_pst_device)->vap_id[_uc_vap_index])) : \
        OSAL_NULL)                                                                      \
        if ((_pst_vap) != OSAL_NULL)
#define alg_vap_foreach_user(_pst_user, _pst_vap, _pst_list_pos)                                           \
    for ((_pst_list_pos) = (_pst_vap)->mac_user_list_head.next,                                        \
        (_pst_user) = osal_list_entry((_pst_list_pos), hmac_user_stru, user_dlist);                     \
        ((_pst_list_pos) != &((_pst_vap)->mac_user_list_head)) && ((_pst_list_pos) != OSAL_NULL); \
        (_pst_list_pos) = (_pst_list_pos)->next,                                                  \
        (_pst_user) = osal_list_entry((_pst_list_pos), hmac_user_stru, user_dlist))                     \
        if ((_pst_user) != OSAL_NULL)

#endif /* end of alg_transplant.h */
