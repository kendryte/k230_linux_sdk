/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: Header file for hal_mac.c.
 * Create: 2020-7-3
 */

#ifndef __HAL_MAC_H__
#define __HAL_MAC_H__

#include "hal_ext_if_device.h"
#include "wlan_types_common.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "hal_mac_reg.h"
#include "hal_mac_reg_field.h"
#include "hal_phy_reg_field.h"
#include "hal_chip.h"
#include "hal_device.h"
#include "hal_soc.h"
#include "hal_reset.h"
#include "hal_device_fsm.h"
#include "hal_rf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_MAC_H

#define HAL_DSCR_BASE_ADDR 0x6
#define HH503_SIFSTIME 16
#define HH503_ACK_CTS_FRAME_LEN 14
#define HH503_LONG_PREAMBLE_LEN_A 16
#define HH503_LONG_PREAMBLE_LEN_B 144
#define HH503_PLCP_HEADER_LEN_A 4
#define HH503_PLCP_HEADER_LEN_B 48
#define HH503_LONG_PREAMBLE_TIME 192
#define HH503_SHORT_PREAMBLE_TIME 96
#define HH503_LEGACY_OFDM_TIME 20
#define HH503_HT_PHY_TIME 40
#define HH503_VHT_PHY_TIME 44

#define HH503_RTSTIME_11B_LONG 221
#define HH503_RTSTIME_NON_11B 27

#define HH503_CTSACKTIME_11B_LONG 212
#define HH503_CTSACKTIME_NON_11B 25

#define HH503_BATIME_11B_LONG 238
#define HH503_BATIME_NON_11B 30

#define HH503_MAC_CFG_INIT 0xC7BDFEDA

#define HH503_EIFSTIME_GONLY \
    (HH503_SIFSTIME + HH503_ACK_CTS_FRAME_LEN * 8 + HH503_PLCP_HEADER_LEN_A + HH503_LONG_PREAMBLE_LEN_A)
#define HH503_EIFSTIME_GMIXED \
    (HH503_SIFSTIME + HH503_ACK_CTS_FRAME_LEN * 8 + HH503_PLCP_HEADER_LEN_B + HH503_LONG_PREAMBLE_LEN_B)

#define HH503_LUT_READ_OPERN 0x0
#define HH503_LUT_WRITE_OPERN 0x1
#define HH503_LUT_REMOVE_OPERN 0x2
#define HH503_NUM_CE_LUT_UPDATE_ATTEMPTS 2

#define HH503_MAC_SEC_CHAN_SLOT_COUNT_INIT_VAL_FREQ_5 0x00000001
#define HH503_MAC_SEC_CHAN_SLOT_COUNT_INIT_VAL_FREQ_2 0x00000002

#define HH503_PHY_TX_MODE_DEFAULT 0x00000000

#ifdef _PRE_WLAN_FEATURE_DAQ
#define HAL_MAC_DIAG_DAQ_OFFSET 0x0
#endif

#ifdef _PRE_WLAN_DFT_STAT
#define HAL_DFT_REG_OTA_LEN 1024
#endif

#define BSRP_VAP_MAX 2

#define HH503_PA_LUT_UPDATE_TIMEOUT 2000
#define HH503_TX_SUSPEND_UPDATE_TIMEOUT 5000

static INLINE__ osal_u16 GetLow16Bits(osal_u32 value)
{
    return (osal_u16)(value & 0x0000FFFF);
}

static INLINE__ osal_u16 GetHigh16Bits(osal_u32 value)
{
    return (osal_u16)((value & 0xFFFF0000) >> 16);
}

typedef enum {
    MAC_CTRL0_BANK,
    MAC_CTRL1_BANK,
    MAC_CTRL2_BANK,
    MAC_CTRL3_BANK,
    MAC_CTRL4_BANK,
    MAC_RD0_BANK,
    MAC_RD1_BANK,
    MAC_RD2_BANK,
    MAC_LUT0_BANK,
    MAC_WLMAC_CTRL,

    WITP_MAC_BANK_BUTT
}hh503_mac_bank_idx_enum;

#ifdef _PRE_WLAN_FEATURE_P2P
extern osal_u16 g_noa_count;
#endif

typedef struct {
    osal_u32 tpc_ch0 : 8,
             dpd_tpc_lv_ch0    : 2,
             cfr_idx           : 2,
             reseved           : 20;
} hh503_tx_resp_phy_mode_reg_stru;

typedef struct {
    osal_u32 xLPwqW6Li_wLu6w6Qy_ : 8;
    osal_u32 xR8OPmCRvwOtwm6Oyzo4ql4RAOIv4m_ : 2;
    osal_u32 xR8OPmCRvwOsIvvRAlo2_ : 1;
    osal_u32 xLPwqW6Li_w6QGhhWow6iFW_ : 1;
    osal_u32 reserved : 20;
} hh503_tx_phy_mode_reg_stru;

typedef union {
    struct {
        osal_u32 xLPwqW6Li_wQWwoLNwLtuW_ : 2;
        osal_u32 xLPwqW6Li_wGhLwCWo_ : 2;
        osal_u32 xR8OPmCRvwORGCO4G4OxPO3_ : 2;
        osal_u32 xLPwqW6Li_wuWwFd_GLBih_ : 3;
        osal_u32 xLPwqW6Li_wF6S_ : 1;
        osal_u32 xLPwqW6Li_wS6Cw_GLW_ : 4;
        osal_u32 xR8OPmCRvwOGwvRvCvxOIv4m_ : 2;
        osal_u32 xR8OPmCRvwOAmOtxz2_ : 1;
        osal_u32 xR8OPmCRvwOGwmzIyxmOyvvsROtxz2_ : 1;
        osal_u32 xLPwqW6Li_wu_WGSjoW_ : 1;
        osal_u32 xLPwqW6Li_wgBwLtuW_ : 2;
        osal_u32 xR8OPmCRvwOCtwOl48_ : 2;
        osal_u32 reserve           : 9;
    } bits;
    osal_u32 u32;
} hh503_tx_phy_rate_stru;

typedef union {
    struct {
        osal_u32 xICsOwzRm_ : 4;
        osal_u32 protocol_mode : 2;
        osal_u32 reserve : 2;
    } bits;
    osal_u8 u8;
} hal_phy_rate_stru;

typedef osal_void (*p_hh503_dft_report_all_reg_state_cb)(hal_to_dmac_device_stru *hal_device);
typedef osal_void (*p_hh503_enable_radar_det_cb)(osal_u8 enable);
typedef osal_void (*p_hh503_set_psm_listen_interval_count_cb)(hal_to_dmac_vap_stru *hal_vap,
    osal_u16 interval_count);

typedef struct {
    p_hh503_dft_report_all_reg_state_cb hh503_dft_report_all_reg_state;
    p_hh503_enable_radar_det_cb hh503_enable_radar_det;
} hal_mac_rom_cb;


static INLINE__ osal_void hh503_mask_mac_error2_init_status(osal_u32 status)
{
    osal_u32 value = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x5C);

    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x5C, (value | status));
}


static INLINE__ osal_u32 hh503_get_subband_index(wlan_channel_band_enum_uint8 band, osal_u8 channel_idx,
    osal_u8 *subband_idx)
{
    if (band == WLAN_BAND_5G) {
        return OAL_FAIL;
    }
    if ((band == WLAN_BAND_2G) && (channel_idx >= HH503_RF_FREQ_2_CHANNEL_NUM)) {
        return OAL_FAIL;
    }
    *subband_idx = channel_idx;
    return OAL_SUCC;
}

typedef osal_void (*hh503_cb_mac_init_hw)(osal_void);
typedef osal_void (*hh503_set_prot_resp_frame_chain_ext_cb)(hal_to_dmac_device_stru *hal_device,
    osal_u8 chain_val);
typedef osal_void (*hh503_set_extlna_chg_cfg_ext_cb)(hal_to_dmac_device_stru *hal_device,
    oal_bool_enum_uint8 extlna_chg_bypass);
typedef osal_void (*hh503_set_primary_channel_ext_cb)(hal_to_dmac_device_stru *hal_device, osal_u8 channel_num,
    wlan_channel_band_enum_uint8 band, osal_u8 channel_idx, wlan_channel_bandwidth_enum_uint8 bandwidth);
typedef osal_void (*hh503_reset_hw_macphy_cb)(osal_bool hw_reset);

hal_device_stru *hh503_chip_get_device(osal_void);
osal_void hh503_ce_clear_all(osal_void);
osal_void hh503_get_freq_factor(osal_u8 freq_band, osal_u8 channel_num, osal_u32 *freq_factor);
#ifdef _PRE_WLAN_DFT_STAT
osal_void hh503_dft_report_all_reg_state_rom_cb(hal_to_dmac_device_stru *hal_device);
#endif

osal_void hh503_set_psm_dtim_count_cb(hal_to_dmac_vap_stru *hal_vap, osal_u8 dtim_count);
osal_u32  hh503_enable_ce(osal_void);
#ifdef _PRE_WLAN_FEATURE_DAQ
osal_void hal_pkt_ram_sample_deinit(osal_void);
osal_void hal_sample_daq_prepare_data(osal_void);
osal_u32 hal_sample_daq_get_data(osal_u32 diag_read_addr);
oal_bool_enum_uint8 hal_sample_daq_done(osal_void);
osal_void hal_show_mac_daq_reg_cfg_info(osal_void);
osal_void hal_show_daq_done_reg_info(osal_void);
#endif
#ifdef _PRE_WIFI_DEBUG
osal_void hal_show_linkloss_reg_info(osal_void);
#endif

osal_u32 hh503_wait_mac_done_timeout(osal_u32 reg_addr, osal_u32 bitmask, osal_u32 reg_busy_value,
    osal_u32 wait_10us_cnt);
osal_void hal_chan_update_beacon_rate(hh503_tx_phy_rate_stru *beacon_rate, const hal_tx_ctrl_desc_rate_stru *tx_dscr);

osal_void hal_set_machw_tx_resume(osal_void);

osal_void hal_vap_set_multi_bssid(osal_u8 *trans_bssid, osal_u8 maxbssid_indicator);

osal_void hh503_mac_ce_ctr_init(osal_void);
osal_u8 hal_save_machw_phy_pa_status(hal_to_dmac_device_stru *hal_device_base);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
