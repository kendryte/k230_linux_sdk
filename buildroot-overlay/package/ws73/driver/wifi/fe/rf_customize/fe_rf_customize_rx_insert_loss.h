/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize rx insert loss 头文件
 * Create: 2022-11-26
 */

#ifndef _FE_RF_CUSTOMIZE_RX_INSERT_LOSS_H_
#define _FE_RF_CUSTOMIZE_RX_INSERT_LOSS_H_

#include "osal_types.h"
#include "mac_vap_ext.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#define HAL_RF_RX_INSERT_LOSS_NUM_2G  3  /* 插入损耗数量 与band划分对应 */
typedef struct {
    osal_u8 insert_loss[HAL_RF_RX_INSERT_LOSS_NUM_2G];
    osal_u8 rsv;
} fe_custom_rx_insert_loss_2g_stru;

typedef struct {
    fe_custom_rx_insert_loss_2g_stru insert_loss_2g;
} fe_custom_rx_insert_loss_stru;

osal_void fe_custom_set_rx_insert_loss(osal_u8 band, osal_u8 *insert_loss, osal_u8 len);
osal_u8 fe_custom_get_rx_insert_loss(osal_u8 band, osal_u8 hal_subband);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // _PRE_PLAT_FEATURE_CUSTOMIZE
#endif  // _FE_RF_CUSTOMIZE_RX_INSERT_LOSS_H_
