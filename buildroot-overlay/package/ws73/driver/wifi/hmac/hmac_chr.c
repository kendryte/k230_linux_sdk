/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: 和CHR相关的接口函数实现
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_chr.h"
#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_CHR_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  3 静态函数声明
*****************************************************************************/

/*****************************************************************************
 4 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFX_CHR
osal_s32 hmac_chr_event_etc(osal_u8 err_code)
{
    osal_u8 i = 0;
    chr_err_code_info_stru type_name_array[] = {
        { CHR_ERR_CODE_PLL, "[PLL_WifiPllInit] Lock Fail" },
        { CHR_ERR_CODE_RX_SARADC_OFFSET, "[DigSarAdcCali] offset cali error" },
        { CHR_ERR_CODE_RX_SARADC_DELAY, "[DigSarAdcCali] cap cali error" },
        { CHR_ERR_CODE_RX_LPF_WIFI, "[LPFBwCali] wifi_rx_lpf_cal error" },
        { CHR_ERR_CODE_RX_LPF_BLE, "[LPFBwCali] ble_rx_lpf_cal error" },
        { CHR_ERR_CODE_RX_RFDC_WIFI, "[DCOffsetCali] wifi RFDC error" },
        { CHR_ERR_CODE_RX_RFDC_BLE, "[DCOffsetCali] ble RFDC error" },
        { CHR_ERR_CODE_RX_DIGDC, "[RxDigDcCali] error" },
        { CHR_ERR_CODE_RX_DIGDC_TRACK, "[RxDigDcCali] error(track mode)" },
        { CHR_ERR_CODE_RX_DC_OFFSET, "[RxDcOffsetCali] error" },
        { CHR_ERR_CODE_RX_IQ_CALI, "[IQCali] Cali error" },
        { CHR_ERR_CODE_RX_IQ_MANUAL, "[IQCaliManual] pwr_calc_real error" },
        { CHR_ERR_CODE_RX_LCTUNE, "[RX_LctuneDigCali] pwr_calc_real error" },
        { CHR_ERR_CODE_TX_LPF_WIFI, "[LPFTxBwCali] wifi_tx_lpf_cal error" },
        { CHR_ERR_CODE_TX_LPF_BLE, "[LPFTxBwCali] ble_tx_lpf_cal error" },
        { CHR_ERR_CODE_TX_PA, "[PACali] PACali error" },
        { CHR_ERR_CODE_TX_IQDC, "[IQDCCali1] error" },
        { CHR_ERR_CODE_TX_PADIG, "[PADigCali] pow cal done error" },
        { CHR_ERR_CODE_TX_PADIGPOWDB, "[PADigCali] PADigPowDbCali error" }
    };

    if (err_code >= CHR_ERR_CODE_BUTT) {
        wifi_printf("CHR Report::unkown err_code(%d).\r\n", err_code);
        return OAL_FAIL;
    }

    for (i = 0; i < sizeof(type_name_array) / sizeof(type_name_array[0]); i++) {
        if (err_code == type_name_array[i].sub_type) {
            wifi_printf("CHR Report::err_code(%d), %s.\r\n", err_code, type_name_array[i].sub_type_name);
            break;
        }
    }
    return OAL_SUCC;
}

osal_s32 hmac_config_chr_err_code_test(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    osal_s32 ret;
    osal_u8 err_code;
    if ((hmac_vap == OAL_PTR_NULL) || (msg->data == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_CFG, "{hmac_config_chr_err_code_test::NULL ptr, hmac_vap=%p, param=%p}",
            hmac_vap, msg->data);
        return OAL_ERR_CODE_PTR_NULL;
    }
    err_code = *(osal_u8 *)msg->data;
    oam_warning_log1(0, OAM_SF_CFG, "{hmac_config_chr_err_code_test::err_code[%d]}", err_code);
    /* 将消息传到hal侧进行并上报 */
    hal_chr_err_code_upload(err_code);
    return OAL_SUCC;
}

#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
