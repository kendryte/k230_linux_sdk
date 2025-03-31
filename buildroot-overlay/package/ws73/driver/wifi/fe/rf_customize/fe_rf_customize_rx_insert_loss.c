/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: rf customize rx insert loss 接口
 * Create: 2022-12-15
 */
#include "fe_rf_customize_rx_insert_loss.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
/* rf 相关定制化结构体 */
static fe_custom_rx_insert_loss_stru g_rf_fe_rx_insert_loss = { 0 };

osal_void fe_custom_set_rx_insert_loss(osal_u8 band, osal_u8 *insert_loss, osal_u8 len)
{
    osal_u8 max_len = (band == WLAN_BAND_2G) ? HAL_RF_RX_INSERT_LOSS_NUM_2G : 0;
    osal_u8 *rx_loss = (band == WLAN_BAND_2G) ? g_rf_fe_rx_insert_loss.insert_loss_2g.insert_loss : OAL_PTR_NULL;
    osal_s32 ret;

    if (rx_loss == OAL_PTR_NULL) {
        return;
    }
    // 拷贝长度不可以大于目的空间大小
    if (len > max_len) {
        len = max_len;
    }

    ret = memcpy_s(rx_loss, max_len, insert_loss, len);
    if (ret != OAL_SUCC) {
        return;
    }
    return;
}
osal_u8 fe_custom_get_rx_insert_loss(osal_u8 band, osal_u8 hal_subband)
{
    osal_u8 max_len = (band == 0) ? HAL_RF_RX_INSERT_LOSS_NUM_2G : 0;
    osal_u8 *rx_loss = (band == 0) ? g_rf_fe_rx_insert_loss.insert_loss_2g.insert_loss : OAL_PTR_NULL;

    if (hal_subband >= max_len || rx_loss == OAL_PTR_NULL) {
        return 0;
    }
    return rx_loss[hal_subband];
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif