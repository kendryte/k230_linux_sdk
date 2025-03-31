/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Header file for hal_csi.c.
 * Create: 2021-11-29
 */

#ifndef __HAL_CSI_H__
#define __HAL_CSI_H__

#include "osal_types.h"
#include "wlan_types_common.h"
#include "hal_device.h"
#include "hal_common_ops.h"
#include "mac_vap_ext.h"
#include "hh503_phy_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_CSI_H

#define HAL_CSI_MAX_USER_NUM  4
#define HAL_CSI_USR_0 0
#define HAL_CSI_USR_1 1
#define HAL_CSI_USR_2 2
#define HAL_CSI_USR_3 3

#define HAL_CSI_USR_ADDR_NUM 2

#define CSI_DATA_MAX_LEN 762
#define CSI_DATA_BLK_NUM 2

typedef enum {
    HAL_CSI_PPDU_NONE_HT      = 0,
    HAL_CSI_PPDU_HE_ER_SU     = 1,
    HAL_CSI_PPDU_HE_MU_MIMO   = 2,
    HAL_CSI_PPDU_HE_MU_OFDMA  = 3,
    HAL_CSI_PPDU_HT           = 4,
    HAL_CSI_PPDU_VHT          = 5,
    HAL_CSI_PPDU_FORMAT_BUTT
} hal_csi_ppdu_format_enum;

typedef enum {
    HAL_CSI_CNT_H_NUM_ERR     = 0,
    HAL_CSI_CNT_H_TIMMING_ERR = 1,
    HAL_CSI_CNT_TIMEOUT_ERR   = 2,
    HAL_CSI_CNT_SW_BLK_ERR    = 3,
    HAL_CSI_CNT_RPT_SUCC      = 4,
    HAL_CSI_RPT_CNT_BUTT
} hal_csi_rpt_cnt_enum;

typedef struct {
    osal_u8 usr_addr[WLAN_MAC_ADDR_LEN];

    osal_u8 cfg_csi_en : 1;
    osal_u8 cfg_addr_match_bypass : 1;
    osal_u8 cfg_match_ta_ra_sel : 1;
    osal_u8 cfg_mgmt_frame_en : 1;
    osal_u8 cfg_ctrl_frame_en : 1;
    osal_u8 cfg_data_frame_en : 1;
    osal_u8 cfg_rsv : 1;
    osal_u8 frm_subtype_match_en : 1;

    osal_u8 match_frame_subtype;

    osal_u8 ppdu_non_ht_en : 1;
    osal_u8 ppdu_he_er_su_en : 1;
    osal_u8 ppdu_he_mu_mimo_en : 1;
    osal_u8 ppdu_he_mu_ofdma_en : 1;
    osal_u8 ppdu_ht_en : 1;
    osal_u8 ppdu_vht_en : 1;
    osal_u8 ppdu_rsv_en : 2;

    osal_u8 cfg_rsv2;

    osal_u16 sample_period_ms;
} hal_csi_usr_attr;

typedef struct {
    osal_u8 csi_data_blk_num;
    osal_u16 csi_data_max_len;
    osal_u8 resv[1];
} hal_csi_buffer_stru;

typedef struct {
    osal_u32 cfg_rsv31 : 1;
    osal_u32 non_ht_bw_mismatch_center_freq_bypass_en : 1;
    osal_u32 non_ht_bw_mismatch_center_freq : 1;
    osal_u32 he_grouping_en : 1;
    osal_u32 direct_resp_ctrl_flt_en : 1;
    osal_u32 fcs_err_drop_en : 1;
    osal_u32 buffer_step_block_size : 10;
    osal_u32 buffer_block_total : 3;
    osal_u32 timeout_thr_us_cnt : 5;
    osal_u32 cfg_rsv7_5 : 3;
    osal_u32 cfg_rsv4_0 : 5;
} hal_csi_global_cfg;

typedef struct {
    osal_u8 h_num_err_cnt;
    osal_u8 h_timing_err_cnt;
    osal_u8 timeout_err_cnt;
    osal_u8 sw_blk_err_cnt;

    osal_u32 rpt_cnt_rsv31_19 : 13;
    osal_u32 rpt_csi_blk_index : 3;
    osal_u32 rpt_csi_succ_cnt : 16;
} hal_csi_stat;

osal_u32 hal_csi_enable(osal_u8 usr_id);
osal_u32 hal_csi_disable(osal_u8 usr_id);
osal_bool hal_csi_vap_is_close(osal_void);
osal_bool hal_csi_is_open(void);
osal_void hh503_csi_phy_open_channel(osal_void);

osal_u32 hal_csi_set_usr_attr(osal_u8 usr_id, const hal_csi_usr_attr *attr);
osal_u32 hal_csi_get_usr_attr(osal_u8 usr_id, hal_csi_usr_attr *attr);

osal_u32 hal_csi_set_buffer_config(hal_csi_buffer_stru *csi_buffer);
osal_u32 hal_csi_set_tsf(osal_u8 hal_vap_id);
osal_void hal_csi_set_ack_resp_flt(void);

osal_u32 hal_csi_stat_clr(osal_void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
