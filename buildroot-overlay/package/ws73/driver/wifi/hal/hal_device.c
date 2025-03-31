/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Implementation for device / vap related operation.
 * Create: 2020-7-3
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_device.h"
#include "hal_chip.h"
#include "hal_power.h"
#include "hal_mac.h"
#include "hal_rf.h"
#include "hal_common_ops.h"
#include "hal_device_fsm.h"
#include "hmac_feature_dft.h"
#include "wlan_spec.h"
#include "wlan_msg.h"
#include "oal_ext_if.h"
#include "fe_tpc_rate_pow.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WIFI_HOST_HAL_DEVICE_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/* 1个chip内hal device个数决定了hal device定制化参数的组数 */
hal_cfg_cap_info_stru g_hal_cfg_custom = {
    /* nss num  */
    WLAN_HAL0_NSS_NUM, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
    /* 支持最大带宽能力 */
    WLAN_HAL0_BW_MAX_WIDTH,
    /* 是否支持双通道发送控制帧 */
    WLAN_HAL0_CONTROL_FRM_TX_DOUBLE_CHAIN_FLAG,

    /* phy chain  */
    WLAN_HAL0_PHY_CHAIN_SEL,
    /* single_tx_chain */
    WLAN_HAL0_SNGL_TX_CHAIN_SEL, /* 单通道下tx_select_chain需要和phy chain值一致 */
    /* rf chain  */
    WLAN_HAL0_RF_CHAIN_SEL,

    /* 是否支持2x1 stbc发送 */
    WLAN_HAL0_TX_STBC_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
    /* 是否支持stbc接收 */
    WLAN_HAL0_RX_STBC_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
    /* 是否支持单用户beamformer */
    WLAN_HAL0_SU_BFER_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
    /* 是否支持单用户beamformee */
    WLAN_HAL0_SU_BFEE_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
    /* 是否支持多用户beamformer */
    WLAN_HAL0_MU_BFER_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
    /* 是否支持多用户beamformee */
    WLAN_HAL0_MU_BFEE_IS_EN, /* 该值需与hal device保持一致，定制化打开后须统一刷成一致 */
    /* 是否支持11AX */
    WLAN_HAL0_11AX_IS_EN,
    /* 是否支持radar dector */
    WLAN_HAL0_RADAR_DETECTOR_IS_EN,

    /* 是否支持dpd */
    WLAN_HAL0_DPD_IS_EN,
    /* 是否支持11n Sounding */
    WLAN_HAL0_11N_SOUNDING,
    /* 是否支持Green Field */
    WLAN_HAL0_GREEN_FIELD,
    /* 是否支持TXOP PS */
    WLAN_HAL0_TXOPPS_IS_EN,
    /* 1024QAM */
    WLAN_HAL0_1024QAM_IS_EN,
    /* 是否支持nb */
    WLAN_HAL0_NB_IS_EN,
    /* 是否支持11n txbf */
    WLAN_HAL0_11N_TXBF_IS_EN,
    0,
    /* su_bfee能力 */
    WLAN_HAL0_SU_BFEE_NUM,
    /* phy2dscr chain */
    WLAN_HAL0_PHY_CHAIN_SEL,
};

/* 软件协议模式映射索引 */
const osal_u8 g_protocol_idx[WLAN_PROTOCOL_BUTT] = {
    /* WLAN_LEGACY_11A_MODE:0 */
    1,
    /* WLAN_LEGACY_11B_MODE:1 */
    0,
    /* WLAN_LEGACY_11G_MODE:2 */
    1,
    /* use_less:3~4 */
    0, 0,
    /* WLAN_HT_MODE:5 */
    2,
    /* WLAN_VHT_MODE:6 */
    3,
    /* WLAN_HT_ONLY_MODE:7 */
    2,
    /* WLAN_VHT_ONLY_MODE:8 */
    3,
    /* WLAN_HT_11G_MODE: 9 */
    2,
    /* WLAN_HE_MODE:10 */
    4,
};

osal_u8 g_m2s_mask = 0x3;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
osal_void hal_device_state_idle_set_rx_filter_reg(const hal_to_dmac_device_stru *hal_device)
{
    osal_u32 rx_filter_val = 0;

    if (hal_device->promis_switch) {
        oam_info_log0(0, OAM_SF_ANY, "{hal_device_state_idle_set_rx_filter_reg::promis switch is on, not set reg}");
        return;
    }

    rx_filter_val |= WLAN_BIP_REPLAY_FAIL_FLT | WLAN_CCMP_REPLAY_FAIL_FLT | WLAN_OTHER_CTRL_FRAME_FLT |
        WLAN_BCMC_MGMT_OTHER_BSS_FLT | WLAN_UCAST_MGMT_OTHER_BSS_FLT | WLAN_UCAST_DATA_OTHER_BSS_FLT;

    oam_warning_log2(0, OAM_SF_ANY,
        "{hal_device_state_idle_set_rx_filter_reg::rx_filter_val = 0x%x cur_state before entry is %d.}",
        rx_filter_val, hal_device->hal_dev_fsm.oal_fsm.cur_state);
    hal_set_rx_filter_reg(rx_filter_val);
}

osal_void hal_device_state_work_set_rx_filter_reg(hal_to_dmac_device_stru *hal_device)
{
    osal_u32 rx_filter_val = WLAN_UCAST_DATA_OTHER_BSS_FLT;

    if (hal_device->promis_switch) {
        oam_info_log0(0, OAM_SF_ANY, "{hal_device_state_work_set_rx_filter_reg::promis switch is on, not set reg}");
        return;
    }

    if (hal_device_find_is_ap_up(hal_device)) {
        rx_filter_val |= WLAN_UCAST_MGMT_OTHER_BSS_FLT;
    } else {
        rx_filter_val |= WLAN_BCMC_MGMT_OTHER_BSS_FLT | WLAN_UCAST_MGMT_OTHER_BSS_FLT;
    }
    oam_warning_log2(0, OAM_SF_ANY,
        "{hal_device_state_work_set_rx_filter_reg::rx_filter_val = 0x%x cur_state before entry is %d.}",
        rx_filter_val, hal_device->hal_dev_fsm.oal_fsm.cur_state);
    hal_set_rx_filter_reg(rx_filter_val);
}

osal_void hal_device_state_scan_set_rx_filter_reg(const hal_to_dmac_device_stru *hal_device)
{
    osal_u32 rx_filter_val = 0;

    if (hal_device->promis_switch) {
        oam_info_log0(0, OAM_SF_ANY, "{hal_device_state_scan_set_rx_filter_reg::promis switch is on, not set reg}");
        return;
    }

    rx_filter_val |= WLAN_BIP_REPLAY_FAIL_FLT | WLAN_CCMP_REPLAY_FAIL_FLT | WLAN_OTHER_CTRL_FRAME_FLT |
        WLAN_UCAST_DATA_OTHER_BSS_FLT;

    oam_warning_log2(0, OAM_SF_ANY,
        "{hal_device_state_scan_set_rx_filter_reg::rx_filter_val = 0x%x cur_state before entry is %d.}",
        rx_filter_val, hal_device->hal_dev_fsm.oal_fsm.cur_state);
    hal_set_rx_filter_reg(rx_filter_val);
}

/*****************************************************************************
 函 数 名  : hal_device_update_phy_chain
 功能描述  : tx chain更新后申请或释放rf dev
             注意:为了适配pll逻辑，siso切换siso时候要先申请另一个rf再释放当前rf，pll逻辑在rf申请和释放中处理
*****************************************************************************/
osal_u32 hal_device_update_phy_chain(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 set_channel)
{
    unref_param(hal_device);
    unref_param(set_channel);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hal_device_reset_bcn_rf_chain
 功能描述  : reset所有的sta的tbtt offset
*****************************************************************************/
osal_void hal_device_reset_bcn_rf_chain(hal_to_dmac_device_stru *hal_device, osal_u8 up_vap_num)
{
    osal_u8 hal_vap_id;
    osal_u8 vap_idx;
    hal_to_dmac_vap_stru *hal_vap = OAL_PTR_NULL;
    osal_u32 work_vap_bitmap = hal_device->work_vap_bitmap;

    for (vap_idx = 0; vap_idx < up_vap_num; vap_idx++) {
        hal_vap_id = oal_bit_find_first_bit_four_byte(work_vap_bitmap);
        work_vap_bitmap &= (~(osal_bit(hal_vap_id)));
        hal_get_hal_vap(hal_device, hal_vap_id, &hal_vap);
        if (hal_vap == OAL_PTR_NULL) {
            oam_error_log1(0, OAM_SF_ANY, "hal_device_reset_bcn_rx_chain:hal_vap[%d] is NULL", hal_vap_id);
            continue;
        }
        if (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_STA) {
            hal_pm_set_bcn_rf_chain(hal_vap, hal_device->cfg_cap_info->rf_chain);
        }
    }
}

osal_u8 hal_device_get_m2s_mask(osal_void)
{
    return g_m2s_mask;
}

hal_cfg_cap_info_stru *hal_device_get_cfg_custom_addr(osal_void)
{
    return &g_hal_cfg_custom;
}

/*****************************************************************************
 函 数 名  : hal_device_calc_up_vap_num
 功能描述  : 计算up的vap个数
*****************************************************************************/
osal_u8 hal_device_calc_up_vap_num(const hal_to_dmac_device_stru *hal_device)
{
    osal_u8 vap_idx;
    osal_u8 hal_vap_id;
    osal_u8 vap_num;
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;
    osal_u8 up_vap_num = 0;
    osal_u32 work_vap_bitmap;

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hal_device_calc_up_vap_num::hal device is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    work_vap_bitmap = hal_device->work_vap_bitmap;

    vap_num = (osal_u8)oal_bit_get_num_four_byte(hal_device->work_vap_bitmap);

    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        hal_vap_id = oal_bit_find_first_bit_four_byte(work_vap_bitmap);
        work_vap_bitmap &= (~(osal_bit(hal_vap_id)));
        hal_get_hal_vap(hal_device, hal_vap_id, &hal_vap);
        if (hal_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_CFG,
                "hal_device_calc_up_vap_num::hal device hal vap id[%d]hal_get_hal_vap null", hal_vap_id);
            return up_vap_num;
        }

        if (hal_vap->hal_vap_state == HAL_VAP_STATE_UP) {
            up_vap_num++;
        }
    }
    return up_vap_num;
}
/*****************************************************************************
 函 数 名  : hal_device_find_one_up_vap
 功能描述  : 寻找处在UP状态的VAP
*****************************************************************************/
osal_u32 hal_device_find_one_up_vap(hal_to_dmac_device_stru *hal_device, osal_u8 *mac_vap_id)
{
    osal_u8 hal_vap_id;
    osal_u8 vap_idx;
    osal_u8 vap_num;
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;
    osal_u32 work_vap_bitmap;

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hal_device_find_up_vap::hal_device is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    work_vap_bitmap = hal_device->work_vap_bitmap;

    if (work_vap_bitmap == 0) {
        oam_error_log1(0, OAM_SF_CFG, "hal_device_find_up_vap::no up vap[%d]", work_vap_bitmap);
        return OAL_FAIL;
    }

    vap_num = (osal_u8)oal_bit_get_num_four_byte(hal_device->work_vap_bitmap);

    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        hal_vap_id = oal_bit_find_first_bit_four_byte(work_vap_bitmap);
        work_vap_bitmap &= (~(osal_bit(hal_vap_id)));
        hal_get_hal_vap(hal_device, hal_vap_id, &hal_vap);
        if (hal_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_CFG, "hal_device_find_up_vap::hal vap id[%d]hal_get_hal_vap null", hal_vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (hal_vap->hal_vap_state == HAL_VAP_STATE_UP) {
            if (mac_vap_id == OSAL_NULL) {
                oam_error_log0(0, OAM_SF_CFG, "hal_device_find_up_vap::mac_vap_id is NULL");
                return OAL_ERR_CODE_PTR_NULL;
            }

            *mac_vap_id = hal_vap->mac_vap_id;
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hal_device_dbac_find_up_vap
 功能描述  : 寻找处在UP状态的VAP
*****************************************************************************/
osal_u8 hal_device_find_all_up_vap(const hal_to_dmac_device_stru *hal_device, osal_u8 *vap_id, osal_u8 lenth)
{
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;
    osal_u8 hal_vap_id;
    osal_u8 vap_idx;
    osal_u8 vap_num;
    osal_u8 up_vap_num = 0;
    osal_u32 work_vap_bitmap;

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hal_device_find_all_up_vap::hal_device is NULL");
        return 0;
    }
    work_vap_bitmap = hal_device->work_vap_bitmap;

    vap_num = (osal_u8)oal_bit_get_num_four_byte(hal_device->work_vap_bitmap);
    if (vap_num >= lenth) {
        oam_error_log2(0, OAM_SF_CFG, "hal_device_find_all_up_vap::up vap num[%d]> %d", vap_num, lenth);
        return 0;
    }

    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        hal_vap_id = oal_bit_find_first_bit_four_byte(work_vap_bitmap);
        work_vap_bitmap &= (~(osal_bit(hal_vap_id)));
        hal_get_hal_vap(hal_device, hal_vap_id, &hal_vap);
        if (hal_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_CFG,
                "hal_device_find_all_up_vap::hal vap id[%d]hal_get_hal_vap null", hal_vap_id);
            return 0;
        }

        if (hal_vap->hal_vap_state == HAL_VAP_STATE_UP) {
            if (vap_id == OSAL_NULL) {
                oam_error_log0(0, OAM_SF_CFG, "hal_device_find_all_up_vap::vap_id is NULL");
                return OAL_ERR_CODE_PTR_NULL;
            }
            vap_id[up_vap_num++] = hal_vap->mac_vap_id;
        }
    }

    return up_vap_num;
}

/*****************************************************************************
 函 数 名  : hal_find_is_ap_up
 功能描述  : 查找是否有已经UP的AP
*****************************************************************************/
oal_bool_enum_uint8 hal_device_find_is_ap_up(hal_to_dmac_device_stru *hal_device)
{
    osal_u8 hal_vap_id;
    osal_u8 vap_idx;
    osal_u8 vap_num;
    osal_u32 work_vap_bitmap;
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{hal_device_find_is_ap_up::hal_device is null}");
        return OSAL_FALSE;
    }
    work_vap_bitmap = hal_device->work_vap_bitmap;

    vap_num = (osal_u8)oal_bit_get_num_four_byte(hal_device->work_vap_bitmap);
    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        hal_vap_id = oal_bit_find_first_bit_four_byte(work_vap_bitmap);
        work_vap_bitmap &= (~(osal_bit(hal_vap_id)));
        hal_get_hal_vap(hal_device, hal_vap_id, &hal_vap);
        if (hal_vap == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_ANY, "{hal_device_find_is_ap_up::hal_vap null}");
            continue;
        }

        if (hal_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) {
            return OSAL_TRUE;
        }
    }

    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : mac_device_find_another_up_vap
 功能描述  : 寻找另一个UP状态的VAP
*****************************************************************************/
osal_u32 hal_device_find_another_up_vap(hal_to_dmac_device_stru *hal_device, osal_u8 vap_id_self,
    osal_u8 *mac_vap_id)
{
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;
    osal_u8 hal_vap_id;
    osal_u8 vap_idx;
    osal_u8 vap_num;
    osal_u32 work_vap_bitmap;

    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hal_device_find_another_up_vap::hal_device is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }
    work_vap_bitmap = hal_device->work_vap_bitmap;

    vap_num = (osal_u8)oal_bit_get_num_four_byte(hal_device->work_vap_bitmap);
    if (vap_num > WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE) {
        oam_error_log2(0, OAM_SF_CFG, "hal_device_find_another_up_vap::up vap num[%d]> %d",
            vap_num, WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE);
        return OAL_FAIL;
    }

    for (vap_idx = 0; vap_idx < vap_num; vap_idx++) {
        hal_vap_id = oal_bit_find_first_bit_four_byte(work_vap_bitmap);
        work_vap_bitmap &= (~(osal_bit(hal_vap_id)));
        hal_get_hal_vap(hal_device, hal_vap_id, &hal_vap);
        if (hal_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_CFG,
                "hal_device_find_another_up_vap::hal vap id[%d]hal_get_hal_vap null", hal_vap_id);
            return OAL_FAIL;
        }

        if (vap_id_self == hal_vap->mac_vap_id) {
            continue;
        }

        if (hal_vap->hal_vap_state == HAL_VAP_STATE_UP) {
            *mac_vap_id = hal_vap->mac_vap_id;
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

osal_void hal_device_inc_assoc_user_nums(hal_to_dmac_device_stru *hal_device)
{
    if (hal_device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_UM, "{hal_device_inc_assoc_user_nums::hal_device is NULL.}");
        return;
    }

    hal_device->assoc_user_nums++;
}

osal_void hal_device_dec_assoc_user_nums(hal_to_dmac_device_stru *hal_device)
{
    if (hal_device->assoc_user_nums == 0) {
        oam_error_log0(0, OAM_SF_UM, "{hal_device_dec_assoc_user_nums::assoc_user_nums is already zero.}");
    } else {
        hal_device->assoc_user_nums--;
    }
}

osal_void hh503_tx_init_dscr_queue(hal_to_dmac_device_stru *device)
{
    if (device == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hh503_tx_init_dscr_queue::device null.}\r\n");
        return;
    }
    return;
}

/*****************************************************************************
    芯片加密类型和协议加密类型互相转换
*  加密方式      软件加密方式对应数据值        硬件发送描述符对应数值
*  User Grp                 0
*   WEP-40                  1                           0
*   TKIP                    2                           1
*   CCMP                    4                           2
*   RES                     3                           3(对应不加密，设置硬件发送描述符bit3~0为3)
*   WEP-104                 5                           4
*   BIP                     6                           5
*   GCMP                    8                           6
*   GCMP-256                9                           7
*   CCMP-256                10                          8
*   BIP-CMAC-256            13                          9
*
 函 数 名  : hal_cipher_suite_to_ctype
 功能描述  : 将802.11 协议中的加密套件数值转换为witp硬件对应加密方式的数值。
*****************************************************************************/
hal_cipher_protocol_type_enum_uint8 hal_cipher_suite_to_ctype(wlan_ciper_protocol_type_enum_uint8 cipher_suite)
{
    hal_cipher_protocol_type_enum_uint8 ciper_type;

    switch (cipher_suite) {
        case WLAN_80211_CIPHER_SUITE_WEP_40:
            ciper_type = HAL_WEP40;
            break;
        case WLAN_80211_CIPHER_SUITE_TKIP:
            ciper_type = HAL_TKIP;
            break;
        case WLAN_80211_CIPHER_SUITE_CCMP:
            ciper_type = HAL_CCMP;
            break;
        case WLAN_80211_CIPHER_SUITE_WEP_104:
            ciper_type = HAL_WEP104;
            break;
        case WLAN_80211_CIPHER_SUITE_BIP:
            ciper_type = HAL_BIP;
            break;
        case WLAN_80211_CIPHER_SUITE_GCMP:
            ciper_type = HAL_GCMP;
            break;
        case WLAN_80211_CIPHER_SUITE_GCMP_256:
            ciper_type = HAL_GCMP_256;
            break;
        case WLAN_80211_CIPHER_SUITE_CCMP_256:
            ciper_type = HAL_CCMP_256;
            break;
        case WLAN_80211_CIPHER_SUITE_BIP_CMAC_256:
            ciper_type = HAL_BIP_256;
            break;
        default:
            ciper_type = HAL_NO_ENCRYP;
            break;
    }
    return ciper_type;
}

/*****************************************************************************
 函 数 名  : hal_ctype_to_cipher_suite
 功能描述  : 将witp硬件对应加密方式的数值转换为802.11 协议中的加密套件数值。
*****************************************************************************/
wlan_ciper_protocol_type_enum_uint8 hal_ctype_to_cipher_suite(hal_cipher_protocol_type_enum_uint8 cipher_type)
{
    wlan_ciper_protocol_type_enum_uint8 ciper_suite;

    switch (cipher_type) {
        case HAL_WEP40:
            ciper_suite = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
        case HAL_TKIP:
            ciper_suite = WLAN_80211_CIPHER_SUITE_TKIP;
            break;
        case HAL_CCMP:
            ciper_suite = WLAN_80211_CIPHER_SUITE_CCMP;
            break;
        case HAL_WEP104:
            ciper_suite = WLAN_80211_CIPHER_SUITE_WEP_104;
            break;
        case HAL_BIP:
            ciper_suite = WLAN_80211_CIPHER_SUITE_BIP;
            break;
        case HAL_GCMP:
            ciper_suite = WLAN_80211_CIPHER_SUITE_GCMP;
            break;
        case HAL_GCMP_256:
            ciper_suite = WLAN_80211_CIPHER_SUITE_GCMP_256;
            break;
        case HAL_CCMP_256:
            ciper_suite = WLAN_80211_CIPHER_SUITE_CCMP_256;
            break;
        default:
            ciper_suite = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
            break;
    }
    return ciper_suite;
}

/*****************************************************************************
 功能描述  : 同步需要的hal_vap成员变量到deivce侧
*****************************************************************************/
osal_void hal_device_sync_vap(hal_to_dmac_vap_stru *hal_vap)
{
    frw_msg msg = {0};
    osal_u8 hal_vap_info[2] = {0};
    hal_vap_info[0] = hal_vap->vap_id;
    hal_vap_info[1] = hal_vap->hal_vap_state;
    msg.data = hal_vap_info;
    msg.data_len = sizeof(hal_vap_info);
    frw_send_msg_to_device(0, WLAN_MSG_H2D_C_CFG_SYNC_HAL_VAP, &msg, OSAL_TRUE);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
