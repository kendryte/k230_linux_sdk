/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_phy.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_PHY_H__
#define __HAL_PHY_H__

#include "hal_common_ops.h"
#include "hal_ext_if.h"
#include "hh503_phy_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_PHY_H


typedef enum {
    WITP_PHY_BANK_0 = 0,
    WITP_PHY_BANK_1,
    WITP_PHY_BANK_2,
    WITP_PHY_BANK_3,
    WITP_PHY_BANK_4,
    WITP_PHY_BANK_5,
    WITP_PHY_BANK_6,
    WITP_PHY_BANK_7,
    WITP_PHY_PHY0_CTRL,

    WITP_PHY_BANK_BUTT
}hh503_phy_bank_idx_enum;

typedef struct {
    wlan_channel_bandwidth_enum_uint8 en_bandwidth;
    osal_u8 bandwidth;
    osal_char sec20_offset;
    osal_char sec40_offset;
    osal_char sec80_offset;
} hh503_phy_bandwidth_stru;

typedef struct {
    osal_u32  xCt2OGzOIv4mOrryOrI_         : 2,
            x6NgwuGwSiFWw00jwVS_         : 2,
            xCt2OGzOIv4mOrryOa4aI_       : 2,
            xCt2OGzOIv4mOrryOrrI_        : 2,
            x6NgwuGwSiFWw00gwXS_         : 2,
            xCt2OGzOIv4mOrr2OcI_         : 2,
            xCt2OGzOIv4mOrr2OrZI_        : 2,
            x6NgwuGwSiFWw00gw0US_        : 2,
            xCt2OGzOIv4mOrr2OZKI_        : 2,
            xCt2OGzOIv4mOrr2OiYI_        : 2,
            x6NgwuGwSiFWw00gwDUS_        : 2,
            xCt2OGzOIv4mOrr2OaKI_        : 2,
            x6NgwuGwSiFWw00hwyS6CwVFDg_  : 2,
            xCt2OGzOIv4mOrroOrICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00hwVS6CwVFDg_  : 2,
            x6NgwuGwSiFWw00hwkS6CwVFDg_  : 2;
} hal_phy_pa_mode_0_code_stru;

typedef struct {
    osal_u32  xCt2OGzOIv4mOrroOKICsOZ4K2_     : 2,
            x6NgwuGwSiFWw00hwxS6CwVFDg_     : 2,
            xCt2OGzOIv4mOrroOYICsOZ4K2_     : 2,
            x6NgwuGwSiFWw00hwpS6CwVFDg_     : 2,
            xCt2OGzOIv4mOrroK3IO3ICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00hDySw0S6CwVFDg_  : 2,
            xCt2OGzOIv4mOrroK3IOZICsOZ4K2_  : 2,
            xCt2OGzOIv4mOrroK3IOiICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00hDySwDS6CwVFDg_  : 2,
            xCt2OGzOIv4mOrroK3IOaICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00hDySwXS6CwVFDg_  : 2,
            xCt2OGzOIv4mOrroK3IO9ICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00hDySwkVS6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrzCO3ICsOZ4K2_    : 2,
            x6NgwuGwSiFWw00G6w0S6CwVFDg_    : 2,
            xCt2OGzOIv4mOrrzCOZICsOZ4K2_    : 2;
} hal_phy_pa_mode_1_code_stru;

typedef struct {
    osal_u32  xCt2OGzOIv4mOrrzCOiICsOZ4K2_     : 2,
            x6NgwuGwSiFWw00G6wDS6CwVFDg_     : 2,
            xCt2OGzOIv4mOrrzCOaICsOZ4K2_     : 2,
            x6NgwuGwSiFWw00G6wXS6CwVFDg_     : 2,
            xCt2OGzOIv4mOrrzCO9ICsOZ4K2_     : 2,
            x6NgwuGwSiFWw00G6wUS6CwVFDg_     : 2,
            xCt2OGzOIv4mOrrzCOcICsOZ4K2_     : 2,
            xCt2OGzOIv4mOrrzCOr3ICsOZ4K2_    : 2,
            x6NgwuGwSiFWw00G6w00S6CwVFDg_    : 2,
            x6NgwuGwSiFWw00G6DySwyS6CwVFDg_  : 2,
            xCt2OGzOIv4mOrrzCK3IOrICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00G6DySwVS6CwVFDg_  : 2,
            x6NgwuGwSiFWw00G6DySwkS6CwVFDg_  : 2,
            xCt2OGzOIv4mOrrzCK3IOKICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00G6DySwxS6CwVFDg_  : 2,
            xCt2OGzOIv4mOrrzCK3IOYICsOZ4K2_  : 2;
} hal_phy_pa_mode_2_code_stru;

typedef struct {
    osal_u32  x6NgwuGwSiFWw00G6DySwpS6CwVFDg_  : 2,
            xCt2OGzOIv4mOrrzCK3IOnICsOZ4K2_  : 2,
            x6NgwuGwSiFWw00G6DySwcS6CwVFDg_  : 2,
            x6NgwuGwSiFWw00G6DySw0yS6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrzCK3IOrrICsOZ4K2_ : 2,
            reserved                   : 22;
} hal_phy_pa_mode_3_code_stru;

typedef struct {
    osal_u32  xCt2OGzOIv4mOrrz8Z3IO3ICsOZ4K2_ : 2,
            x6NgwuGwSiFWw00GPVySw0S6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrz8Z3IOZICsOZ4K2_ : 2,
            xCt2OGzOIv4mOrrz8Z3IOiICsOZ4K2_ : 2,
            x6NgwuGwSiFWw00GPVySwDS6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrz8Z3IOaICsOZ4K2_ : 2,
            x6NgwuGwSiFWw00GPVySwXS6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrz8Z3IO9ICsOZ4K2_ : 2,
            x6NgwuGwSiFWw00GPDySwyS6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrz8K3IOrICsOZ4K2_ : 2,
            x6NgwuGwSiFWw00GPDySwVS6CwVFDg_ : 2,
            x6NgwuGwSiFWw00GPDySwkS6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrz8K3IOKICsOZ4K2_ : 2,
            x6NgwuGwSiFWw00GPDySwxS6CwVFDg_ : 2,
            xCt2OGzOIv4mOrrz8K3IOYICsOZ4K2_ : 2,
            x6NgwuGwSiFWw00GPDySwpS6CwVFDg_ : 2;
} hal_phy_pa_mode_4_code_stru;

#if defined BOARD_FPGA_WIFI
typedef struct {
    osal_u32  lpf_gain    : 4,
            mod_pga_unit  : 6,
            mod_gm_unit   : 6,
            mod_slice_tpc : 2,
            pa_unit       : 4,
            pa_slice_idx  : 2,
            r_comp_sel    : 4,
            c_comp_sel    : 4;
} hal_phy_tpc_value_stru;
#else
typedef struct {
    osal_u32  lpf_gain    : 2,
            mod_pga_unit  : 6,
            mod_gm_unit   : 6,
            mod_slice_tpc : 2,
            pa_unit       : 4,
            pa_slice_idx  : 2,
            r_comp_sel    : 5,
            c_comp_sel    : 5;
} hal_phy_tpc_value_stru;
#endif

osal_void hh503_set_phy_channel_num(osal_u8 channel_num);
osal_void hh503_set_phy_channel_freq_factor(osal_u32 freq_factor);
osal_void hh503_device_enable_ftm(osal_void);
osal_void hh503_device_disable_ftm(osal_void);
#ifdef _PRE_WLAN_FEATURE_DFS
    osal_void hh503_enable_radar_det_cb(osal_u8 enable);
#endif

osal_void hh503_radar_config_reg_ext(hal_dfs_radar_type_enum_uint8 radar_type);

#ifdef BOARD_FPGA_WIFI
osal_void hal_initialize_phy(osal_void);
osal_void hh503_set_phy_gain(osal_void);
osal_void hh503_phy_set_cfg_pa_mode_code(osal_void);
#endif

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
osal_void hal_phy_do_sgl_tone_notch_coef(osal_u32 notch_filter_coef1, osal_u32 notch_filter_coef2,
    osal_u32 sgl_tone_0_2_car_and_en, osal_u32 sgl_tone_3_car);
osal_void hal_phy_do_sgl_tone_notch_weight(osal_u32 sgl_tone_0_7_weight, osal_u8 notch_ch);
#endif

osal_void hal_get_ch_measurement_result_ram(hal_ch_statics_irq_event_stru *ch_statics);

osal_void hal_get_ch_measurement_result(hal_ch_statics_irq_event_stru *ch_statics);

typedef osal_void (*hh503_initialize_phy_ext_cb)(osal_void);
typedef osal_void (*hh503_update_phy_by_bandwith_cb)(wlan_channel_bandwidth_enum_uint8 bandwidth);
typedef osal_void (*hh503_radar_config_reg_cb)(hal_dfs_radar_type_enum_uint8 radar_type);
typedef osal_void (*hh503_get_single_psd_sample_cb)(osal_u16 index, osal_char *psd_val);
typedef osal_void (*hh503_set_psd_en_cb)(osal_u32 reg_value);
typedef osal_void (*hh503_set_machw_phy_adc_freq_ext_cb)(hal_to_dmac_device_stru *hal_device,
    wlan_channel_bandwidth_enum_uint8 bandwidth);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
