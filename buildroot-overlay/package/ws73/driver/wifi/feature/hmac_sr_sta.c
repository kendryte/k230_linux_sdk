/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description:
 * Date: 2020-07-07
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_sr_sta.h"
#include "hmac_mgmt_sta.h"
#include "hmac_feature_main.h"
#include "frw_util_notifier.h"
#include "hmac_feature_dft.h"
#include "hmac_feature_interface.h"
#include "hmac_ccpriv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_DEV_DMAC_SR_STA_ROM_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_DEVICE

OSAL_STATIC hmac_sta_sr_stru *g_hmac_sta_sr_info[WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT] = {
    OSAL_NULL, OSAL_NULL, OSAL_NULL, OSAL_NULL
};

/*****************************************************************************
  2 函数声明
*****************************************************************************/
OSAL_STATIC osal_void hmac_sta_up_update_psr(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_void hmac_sta_up_obss_pd_statistics(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_void hmac_sta_obss_pd_vfs_statistics(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_void hmac_sta_up_psr_statistics(hmac_vap_stru *hmac_vap);
OSAL_STATIC osal_s32 hmac_config_set_collision_on(hmac_vap_stru *hmac_vap, osal_u8 col_en);
OSAL_STATIC osal_u32 hmac_report_collision_callback(osal_void *ptr);
OSAL_STATIC osal_void hmac_report_collision(hmac_vap_stru *hmac_vap);

OSAL_STATIC osal_void hmac_psr_statistics_process(hmac_vap_stru *hmac_vap, dmac_sr_sta_vfs_stru hmac_sr_sta_vfs)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_vap_he_sr_config_info_stru *sr_cfg_info = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;
    osal_u32 vfs;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_psr_statistics_process::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_psr_statistics_process::sr null.}");
        return;
    }

    sr_cfg_info = &sr->he_sr_config_info;
    sr_handler = &sr->sr_handler;
    /* 100:相当于计算结果取2位小数 */
    vfs = (hmac_sr_sta_vfs.rpt_psr_sr_tx_num == 0) ? 0 : ((osal_u32)hmac_sr_sta_vfs.rpt_psr_sr_tx_success_num * 100 /
        (osal_u32)hmac_sr_sta_vfs.rpt_psr_sr_tx_num);
    if ((vfs >= 80) || (hmac_sr_sta_vfs.rpt_psr_sr_tx_num == 0)) { /* 成功率大于等于80 */
        sr_handler->vfs_state1 = OSAL_TRUE;
        sr_handler->enable_flag_psr = OSAL_FALSE;
        hmac_sta_up_psr_statistics(hmac_vap);
    } else {
        sr_cfg_info->offset -= 1;
        if (sr_cfg_info->offset == 0) {
            sr_handler->enable_flag_psr = OSAL_FALSE;
            hmac_sta_up_update_psr(hmac_vap);
            return;
        }

        if (sr_cfg_info->sr_ie_info.sr_control.srp_disallowed != 1) {
            hal_set_psr_offset(-(sr_cfg_info->offset));
        }
        sr_handler->enable_flag_psr = OSAL_FALSE;
        hmac_sta_up_psr_statistics(hmac_vap);
    }
}

OSAL_STATIC osal_u32 hmac_psr_statistics_callback(osal_void *ptr)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ptr;
    dmac_sr_sta_vfs_stru hmac_sr_sta_vfs = { 0 };

    hal_set_psr_statistics_end();
    hal_set_psr_vfs_get(&hmac_sr_sta_vfs);
    hal_set_psr_statistics_clear();

    hmac_psr_statistics_process(hmac_vap, hmac_sr_sta_vfs);
    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_sta_up_psr_statistics_timer(hmac_vap_stru *hmac_vap)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_psr_statistics_timer::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_psr_statistics_timer::sr null.}");
        return;
    }

    sr_handler = &sr->sr_handler;

    sr_handler->sr_psr_timeout = 15000; /* 15000:时间15秒 */
    if (sr_handler->vfs_state1 == OSAL_TRUE) {
        sr_handler->sr_psr_timeout = 55000; /* 55000:时间55秒 */
        sr_handler->vfs_state1 = OSAL_FALSE;
    }

    frw_create_timer_entry(&(sr_handler->psr_timer), hmac_psr_statistics_callback, sr_handler->sr_psr_timeout,
        hmac_vap, OSAL_FALSE);
}

OSAL_STATIC osal_void hmac_sta_up_psr_statistics(hmac_vap_stru *hmac_vap)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_psr_statistics::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_psr_statistics::sr null.}");
        return;
    }

    sr_handler = &sr->sr_handler;

    // 清零统计
    hal_set_psr_statistics_clear();

    if (sr_handler->enable_flag_psr == OSAL_FALSE) {
        hal_set_psr_statistics_start();
        hmac_sta_up_psr_statistics_timer(hmac_vap);
        sr_handler->enable_flag_psr = OSAL_TRUE;
    }
}

OSAL_STATIC osal_void hmac_sta_up_update_psr(hmac_vap_stru *hmac_vap)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_vap_he_sr_config_info_stru *sr_cfg_info = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_update_psr::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_update_psr::sr null.}");
        return;
    }

    sr_cfg_info = &sr->he_sr_config_info;

    sr_cfg_info->offset = 5; /* 5：偏移值为5 */

    hal_set_psrt_htc();
    hal_set_psr_offset(-(sr_cfg_info->offset));
    hmac_sta_up_psr_statistics(hmac_vap);
}

OSAL_STATIC osal_void hmac_obss_pd_vfs_process(hmac_vap_stru *hmac_vap, dmac_sr_sta_vfs_stru hmac_sr_sta_vfs)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_vap_he_sr_config_info_stru *sr_cfg_info = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;
    osal_u32 vfs;
    osal_s32 tx_pwr, tx_non;

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_obss_pd_vfs_process::sr null.}");
        return;
    }

    sr_handler = &sr->sr_handler;
    sr_cfg_info = &sr->he_sr_config_info;

    vfs = (osal_u32)hmac_sr_sta_vfs.rpt_obss_pd_tx_success_num * 100 / /* 100:相当于计算结果取2位小数 */
        (osal_u32)hmac_sr_sta_vfs.rpt_obss_pd_tx_num;
    if ((vfs >= 80) || (hmac_sr_sta_vfs.rpt_obss_pd_tx_num == 0)) { /* 成功率大于等于80 */
        sr_handler->vfs_state = OSAL_TRUE;
        sr_handler->enable_sr_vfs = OSAL_FALSE;
        hmac_sta_obss_pd_vfs_statistics(hmac_vap);
    } else {
        if ((sr_cfg_info->srg_pd == 0) && (sr_cfg_info->non_srg_pd == 0)) {
            sr_handler->enable_sr_vfs = OSAL_FALSE;
            sr_handler->enable_flag = OSAL_FALSE;
            sr_handler->vfs_state = OSAL_FALSE;
            return;
        }
        sr_cfg_info->srg_pd -= 2; /* srg_pd初始减2 */
        sr_cfg_info->non_srg_pd -= 2; /* non_srg_pd初始减2 */
        if (((sr_cfg_info->sr_pd_info.pd_min != 0) && (sr_cfg_info->srg_pd < sr_cfg_info->sr_pd_info.pd_min)) ||
            (sr_cfg_info->non_srg_pd < MAC_HE_SRG_PD_MIN)) {
            sr_handler->enable_sr_vfs = OSAL_FALSE;
            sr_handler->enable_flag = OSAL_FALSE;
            hmac_sta_up_obss_pd_statistics(hmac_vap);
            return;
        }

        if ((sr_cfg_info->sr_ie_info.sr_control.non_srg_obss_pd_sr_disallowed != 1) &&
            (sr_cfg_info->sr_ie_info.sr_control.non_srg_offset_present == 1)) {
            tx_non = 21 - (sr_cfg_info->non_srg_pd - MAC_HE_SRG_PD_MIN); /* 21:txpwr_ref */
            hal_set_non_pd((osal_u32)sr_cfg_info->non_srg_pd, (osal_u32)tx_non);
        }

        if (sr_cfg_info->sr_ie_info.sr_control.srg_information_present != 0) {
            tx_pwr = 21 - (sr_cfg_info->srg_pd -  /* 21:txpwr_ref */
                (MAC_HE_SRG_PD_MIN + sr_cfg_info->sr_ie_info.srg_obss_pd_offset_min));
            hal_set_srg_pd_etc((osal_u32)sr_cfg_info->srg_pd, (osal_u32)tx_pwr);
        }

        sr_handler->enable_sr_vfs = OSAL_FALSE;
        hmac_sta_obss_pd_vfs_statistics(hmac_vap);
    }
}

OSAL_STATIC osal_u32 hmac_obss_pd_vfs_callback(osal_void *ptr)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ptr;
    dmac_sr_sta_vfs_stru hmac_sr_sta_vfs = { 0 };

    hal_set_sr_vfs_end();
    // 获取结果
    hal_set_sr_vfs_get(&hmac_sr_sta_vfs);
    hal_set_sr_vfs_clear();

    hmac_obss_pd_vfs_process(hmac_vap, hmac_sr_sta_vfs);

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_sta_obss_pd_vfs_statistics_timer(hmac_vap_stru *hmac_vap)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_obss_pd_vfs_statistics_timer::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_obss_pd_vfs_statistics_timer::sr null.}");
        return;
    }

    sr_handler = &sr->sr_handler;

    sr_handler->sr_vfs_timeout = 20000; /* 20000:20秒 */
    if (sr_handler->vfs_state == OSAL_TRUE) {
        sr_handler->sr_vfs_timeout = 60000; /* 60000:60秒 */
        sr_handler->vfs_state = OSAL_FALSE;
    }

    frw_create_timer_entry(&(sr_handler->sr_vfs_timer), hmac_obss_pd_vfs_callback, sr_handler->sr_vfs_timeout,
        hmac_vap, OSAL_FALSE);
}

OSAL_STATIC osal_void hmac_sta_obss_pd_vfs_statistics(hmac_vap_stru *hmac_vap)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_obss_pd_vfs_statistics::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_obss_pd_vfs_statistics::sr null.}");
        return;
    }

    sr_handler = &sr->sr_handler;

    // 清零发送帧、发送成功帧统计
    hal_set_sr_vfs_clear();

    if (sr_handler->enable_sr_vfs == OSAL_FALSE) {
        // start发送帧、发送成功帧统计
        hal_set_sr_vfs_start();
        hmac_sta_obss_pd_vfs_statistics_timer(hmac_vap);
        sr_handler->enable_sr_vfs = OSAL_TRUE;
    }
}

OSAL_STATIC osal_void hmac_obss_pd_statistics_process(hmac_vap_stru *hmac_vap, dmac_sr_sta_srg_stru hmac_sr_sta_srg,
    dmac_sr_sta_non_srg_stru sr_sta_non_srg)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_vap_he_sr_config_info_stru *sr_cfg_info = OSAL_NULL;
    osal_s32 tx, tx_non;
    osal_u32 srg_count = 0;
    osal_u32 non_srg_count = 0;
    osal_u8 srg_gears = 0;
    osal_u8 non_srg_gears = 0;
    osal_u8 i;
    osal_s8 obss_pd_arry[4] = {SRG_GEARS_ZERO, SRG_GEARS_ONE, SRG_GEARS_TWO, SRG_GEARS_THREE};
    osal_s8 obss_non_pd_arry[4] = {NON_SRG_GEARS_ZERO, NON_SRG_GEARS_ONE, NON_SRG_GEARS_TWO, NON_SRG_GEARS_THREE};

    /* init 4 size of sta_srg_arry */
    osal_u16 sta_srg_arry[4] = {hmac_sr_sta_srg.rpt_srg_78_82_cnt, hmac_sr_sta_srg.rpt_srg_74_78_cnt,
        hmac_sr_sta_srg.rpt_srg_68_74_cnt, hmac_sr_sta_srg.rpt_srg_62_68_cnt};
    /* init 4 size of sta_non_srg_arry */
    osal_u16 sta_non_srg_arry[4] = {sr_sta_non_srg.rpt_non_srg_78_82_cnt, sr_sta_non_srg.rpt_non_srg_74_78_cnt,
        sr_sta_non_srg.rpt_non_srg_68_74_cnt, sr_sta_non_srg.rpt_non_srg_62_68_cnt};

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_obss_pd_statistics_process::sr null.}");
        return;
    }

    sr_cfg_info = &sr->he_sr_config_info;
    /* 获取到srg、non-srg最大个数区间（如果有多个，以接收能量最大区间为主）
       将区间的最大端值作为新的pd, 根据ie中的non_srg_offset_present和srg_information_present来决定是否设置新的pd、tx */
    /* size of array not beyond 4 */
    for (i = 0; i < 4; i++) {
        if ((srg_count <= sta_srg_arry[i]) && (obss_pd_arry[i] <= (sr_cfg_info->sr_pd_info.pd_max))) {
            srg_count = sta_srg_arry[i];
            srg_gears = i;
        }
        if ((non_srg_count <= sta_non_srg_arry[i]) && (obss_non_pd_arry[i] <= (sr_cfg_info->sr_pd_info.pd_max_non))) {
            non_srg_count = sta_non_srg_arry[i];
            non_srg_gears = i;
        }
    }
    sr_cfg_info->srg_pd = obss_pd_arry[srg_gears];
    sr_cfg_info->non_srg_pd = obss_non_pd_arry[non_srg_gears];

    if ((sr_cfg_info->sr_ie_info.sr_control.non_srg_obss_pd_sr_disallowed != 1) &&
        (sr_cfg_info->sr_ie_info.sr_control.non_srg_offset_present == 1)) {
        tx_non = 21 - (sr_cfg_info->non_srg_pd - MAC_HE_SRG_PD_MIN); /* 21:txpwr_ref */
        hal_set_non_pd((osal_u32)sr_cfg_info->non_srg_pd, (osal_u32)tx_non);
    }
    if (sr_cfg_info->sr_ie_info.sr_control.srg_information_present != 0) {
        tx = 21 - (sr_cfg_info->srg_pd - /* 21:txpwr_ref */
            (MAC_HE_SRG_PD_MIN + sr_cfg_info->sr_ie_info.srg_obss_pd_offset_min));
        hal_set_srg_pd_etc((osal_u32)sr_cfg_info->srg_pd, (osal_u32)tx);
    }

    hmac_sta_obss_pd_vfs_statistics(hmac_vap);
}

OSAL_STATIC osal_u32 hmac_obss_pd_statistics_callback(osal_void *ptr)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ptr;
    dmac_sr_sta_srg_stru hmac_sr_sta_srg = { 0 };
    dmac_sr_sta_non_srg_stru hmac_sr_sta_non_srg = { 0 };

    // end统计
    hal_set_sr_statistics_end();
    // 获取结果
    hal_set_sr_statistics_get(&hmac_sr_sta_srg, &hmac_sr_sta_non_srg);
    // 清零统计
    hal_set_sr_statistics_clear();

    hmac_obss_pd_statistics_process(hmac_vap, hmac_sr_sta_srg, hmac_sr_sta_non_srg);

    return 0;
}

OSAL_STATIC osal_void hmac_sta_up_obss_pd_statistics_timer(hmac_vap_stru *hmac_vap)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_obss_pd_statistics_timer::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_obss_pd_statistics_timer::sr null.}");
        return;
    }

    sr_handler = &sr->sr_handler;

    sr_handler->sr_obss_timeout = 60000; /* 60000:1分钟 */

    frw_create_timer_entry(&(sr_handler->sr_obss_timer), hmac_obss_pd_statistics_callback, sr_handler->sr_obss_timeout,
        hmac_vap, OSAL_FALSE);
}

OSAL_STATIC osal_void hmac_sta_up_obss_pd_statistics(hmac_vap_stru *hmac_vap)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_obss_pd_statistics::hmac_vap null.}");
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_SR, "{hmac_sta_up_obss_pd_statistics::sr null.}");
        return;
    }

    sr_handler = &sr->sr_handler;
    // 清零统计
    hal_set_sr_statistics_clear();
    if (sr_handler->enable_flag == OSAL_FALSE) {
        // start统计
        hal_set_sr_statistics_start();
        hmac_sta_up_obss_pd_statistics_timer(hmac_vap);
        sr_handler->enable_flag = OSAL_TRUE;
    }
}

OSAL_STATIC osal_void hmac_sta_up_update_spatial_reuse_lv(mac_vap_he_sr_pd *sr_pd_info,
    const mac_frame_he_spatial_reuse_parameter_set_ie_stru *sr_ie_info)
{
    if ((sr_ie_info->sr_control.non_srg_obss_pd_sr_disallowed != 1) &&
        (sr_ie_info->sr_control.non_srg_offset_present == 1)) {
        sr_pd_info->pd_max_non = (MAC_HE_SRG_PD_MIN + sr_ie_info->non_srg_boss_pd_offset_max);
        if (sr_ie_info->non_srg_boss_pd_offset_max == 0) {
            sr_pd_info->pd_max_non = (MAC_HE_SRG_PD_MIN + 13); /* 13:offset */
        }
        sr_pd_info->pd_lv_non = sr_pd_info->pd_max_non;
        sr_pd_info->tx_non = 21 - (sr_pd_info->pd_lv_non - MAC_HE_SRG_PD_MIN); /* 21:txpwr_ref */
    }
    if (sr_ie_info->sr_control.srg_information_present != 0) {
        sr_pd_info->pd_min = (MAC_HE_SRG_PD_MIN + sr_ie_info->srg_obss_pd_offset_min);
        sr_pd_info->pd_max = (MAC_HE_SRG_PD_MIN + sr_ie_info->srg_obss_pd_offset_max);
        if ((sr_ie_info->srg_obss_pd_offset_min == sr_ie_info->srg_obss_pd_offset_max) &&
            (sr_ie_info->srg_obss_pd_offset_max == 0)) {
            sr_pd_info->pd_max = (MAC_HE_SRG_PD_MIN + 19); /* 当AP无指示行为的时候默认设offset19 */
        }
        sr_pd_info->pd_lv = sr_pd_info->pd_max;
        sr_pd_info->tx = 21 - (sr_pd_info->pd_lv - sr_pd_info->pd_min); /* 21:txpwr_ref */
    }
}

OSAL_STATIC osal_s32 hmac_set_srg_bitmap(const mac_frame_he_spatial_reuse_parameter_set_ie_stru *sr_ie_info,
    mac_vap_he_sr_config_info_stru *sr_cfg_info)
{
    osal_u32 bitmap_low;
    osal_u32 bitmap_high;
    osal_u8 *bitmap = OSAL_NULL;
    errno_t ret = EOK;

    bitmap = (osal_u8 *)sr_ie_info->srg_bss_color_bitmap_info;
    if (bitmap == OSAL_NULL) {
        return OAL_FAIL;
    }

    if ((sr_ie_info->sr_control.srg_information_present != 0) &&
        (memcmp(sr_cfg_info->srg_bss_color_bitmap, bitmap, 8) != 0)) {         /* 8: subscript */
        bitmap_low = oal_join_word32(bitmap[0], bitmap[1], bitmap[2], bitmap[3]);  /* 2,3: subscript */
        bitmap_high = oal_join_word32(bitmap[4], bitmap[5], bitmap[6], bitmap[7]); /* 4,5,6,7: subscript */
        hal_set_srg_bss_color_bitmap(bitmap_low, bitmap_high);
        ret = memcpy_s(sr_cfg_info->srg_bss_color_bitmap, MAC_HE_SRG_BSS_COLOR_BITMAP_LEN, bitmap,
            MAC_HE_SRG_BSS_COLOR_BITMAP_LEN);
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_SR, "{hmac_set_srg_bitmap::srg_bss_color_bitmap memcpy error!.}");
        }
    }

    bitmap = (osal_u8 *)sr_ie_info->ac_srg_partial_bssid_bitmap;
    if (bitmap == OSAL_NULL) {
        return OAL_FAIL;
    }

    if ((sr_ie_info->sr_control.srg_information_present != 0) &&
        (memcmp(sr_cfg_info->srg_partial_bssid_bitmap, bitmap, 8) != 0)) {     /* 8: subscript */
        bitmap_low = oal_join_word32(bitmap[0], bitmap[1], bitmap[2], bitmap[3]);  /* 2,3: subscript */
        bitmap_high = oal_join_word32(bitmap[4], bitmap[5], bitmap[6], bitmap[7]); /* 4,5,6,7: subscript */
        hal_set_srg_partial_bssid_bitmap(bitmap_low, bitmap_high);
        ret = memcpy_s(sr_cfg_info->srg_partial_bssid_bitmap, MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN, bitmap,
            MAC_HE_SRG_PARTIAL_BSSID_BITMAP_LEN);
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_SR, "{hmac_set_srg_bitmap::srg_partial_bssid_bitmap memcpy error!.}");
        }
    }

    return ret;
}

OSAL_STATIC osal_void hmac_sta_up_update_spatial_reuse_params_ext(hmac_vap_stru *hmac_vap,
    const mac_vap_he_sr_pd *sr_pd_info, mac_vap_he_sr_config_info_stru *sr_cfg_info,
    const mac_frame_he_spatial_reuse_parameter_set_ie_stru *sr_ie_info, mac_sta_sr_handler *sr_handler)
{
    if ((sr_ie_info->sr_control.non_srg_offset_present == 0) &&
        (sr_ie_info->sr_control.srg_information_present == 0)) {
        if (sr_handler->enable_flag == OSAL_TRUE) {
            frw_destroy_timer_entry(&(sr_handler->sr_obss_timer));
            sr_handler->enable_flag = OSAL_FALSE;
        }
        hal_set_sr_ctrl(OSAL_FALSE);
        sr_cfg_info->enable_flag = OSAL_FALSE;
    }
    if (sr_ie_info->sr_control.srg_information_present !=
        sr_cfg_info->sr_ie_info.sr_control.srg_information_present) {
        if (sr_ie_info->sr_control.srg_information_present != 0) {
            hal_set_srg_pd_etc((osal_u32)sr_pd_info->pd_lv, (osal_u32)sr_pd_info->tx);
        } else {
            hal_set_srg_ele_off();
        }
        sr_cfg_info->sr_ie_info = *sr_ie_info;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (mac_mib_get_he_psr_option_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }
    if (sr_ie_info->sr_control.srp_disallowed != sr_cfg_info->sr_ie_info.sr_control.srp_disallowed) {
        if (sr_ie_info->sr_control.srp_disallowed != 1) {
            hal_set_psr_ctrl(OSAL_TRUE);
            hmac_sta_up_update_psr(hmac_vap);
        } else {
            if (sr_handler->enable_flag_psr == OSAL_TRUE) {
                frw_destroy_timer_entry(&(sr_handler->psr_timer));
                sr_handler->enable_flag_psr = OSAL_FALSE;
            }
            hal_set_psr_ctrl(OSAL_FALSE);
        }
        sr_cfg_info->sr_ie_info = *sr_ie_info;
    }
#endif
}

OSAL_STATIC osal_void hmac_sta_up_update_spatial_reuse_params(hmac_vap_stru *hmac_vap,
    mac_frame_he_spatial_reuse_parameter_set_ie_stru *sr_ie_info, mac_vap_11ax_param_config_enum_uint8 type)
{
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_vap_he_sr_config_info_stru *sr_cfg_info = OSAL_NULL;
    mac_vap_he_sr_pd sr_pd_info;
    mac_frame_he_sr_control_stru *sr_control = OSAL_NULL;

    if (hmac_vap == OSAL_NULL || sr_ie_info == OSAL_NULL) {
        return;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr == OSAL_NULL) {
        hal_set_sr_ctrl(OSAL_FALSE);
        hal_set_psr_ctrl(OSAL_FALSE);
        return;
    }

    sr_cfg_info = &sr->he_sr_config_info;
    sr_control = &sr_ie_info->sr_control;

    (osal_void)memset_s(&sr_pd_info, sizeof(mac_vap_he_sr_pd), 0, sizeof(mac_vap_he_sr_pd));

    if (type == MAC_VAP_11AX_PARAM_CONFIG_INIT) {
        memset_s(sr_cfg_info, sizeof(mac_vap_he_sr_config_info_stru), 0, sizeof(mac_vap_he_sr_config_info_stru));
    }
    if (sr_cfg_info->enable_flag == OSAL_FALSE) {
        if ((sr_control->non_srg_offset_present == 1) || (sr_control->srg_information_present == 1)) {
            hmac_sta_up_update_spatial_reuse_lv(&sr_pd_info, sr_ie_info);
            hal_set_sr_ctrl(OSAL_TRUE);
            hal_set_srg_ctrl();
            if ((sr_control->non_srg_obss_pd_sr_disallowed != 1) && (sr_control->non_srg_offset_present == 1)) {
                hal_set_non_pd((osal_u32)sr_pd_info.pd_lv_non, (osal_u32)sr_pd_info.tx_non);
            }
            if (sr_control->srg_information_present != 0) {
                hal_set_srg_pd_etc((osal_u32)sr_pd_info.pd_lv, (osal_u32)sr_pd_info.tx);
            }

            sr_cfg_info->sr_pd_info = sr_pd_info;
            sr_cfg_info->sr_ie_info = *sr_ie_info;
            hmac_sta_up_obss_pd_statistics(hmac_vap);
        }
#ifdef _PRE_WLAN_FEATURE_11AX
        if ((sr_control->srp_disallowed != 1) && (mac_mib_get_he_psr_option_implemented(hmac_vap) == OSAL_TRUE)) {
            hal_set_psr_ctrl(OSAL_TRUE);
            hmac_sta_up_update_psr(hmac_vap);
        }
#endif
        sr_cfg_info->enable_flag = OSAL_TRUE;
    } else {
        hmac_sta_up_update_spatial_reuse_params_ext(hmac_vap, &sr_pd_info, sr_cfg_info, sr_ie_info, &sr->sr_handler);
    }

    if (hmac_set_srg_bitmap(sr_ie_info, sr_cfg_info) != EOK) {
        oam_error_log1(0, OAM_SF_SR, "vap_id[%d] {hmac_sta_up_update_spatial_reuse_params::error!.}", hmac_vap->vap_id);
    }
}

OSAL_STATIC osal_bool hmac_sta_sr_continue_statistics(osal_void *notify_data)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)notify_data;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;
    mac_vap_he_sr_config_info_stru *sr_cfg_info = OSAL_NULL;

    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr != OSAL_NULL) {
        sr_cfg_info = &sr->he_sr_config_info;
        sr_handler = &sr->sr_handler;
        if (sr_cfg_info->enable_flag == OSAL_TRUE) {
            if (sr_handler->enable_flag == OSAL_TRUE) {
                // 继续统计
                hal_set_sr_statistics_continue();
            }
            if (sr_handler->enable_flag_psr == OSAL_TRUE) {
                // 继续统计
                hal_set_psr_statistics_continue();
            }
        }
    }

    return OSAL_TRUE;
}

OSAL_STATIC osal_bool hmac_sta_sr_stop_statistics(osal_void *notify_data)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)notify_data;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_sta_sr_stru *sr = OSAL_NULL;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;
    mac_vap_he_sr_config_info_stru *sr_cfg_info = OSAL_NULL;

    hmac_vap = mac_res_get_hmac_vap(hmac_device->scan_params.vap_id);
    if (osal_unlikely(hmac_vap == OSAL_NULL)) {
        return OSAL_FALSE;
    }

    sr = (hmac_sta_sr_stru *)g_hmac_sta_sr_info[hmac_vap->vap_id];
    if (sr != OSAL_NULL) {
        sr_handler = &sr->sr_handler;
        sr_cfg_info = &sr->he_sr_config_info;
        if (sr_cfg_info->enable_flag == OSAL_TRUE) {
            if (sr_handler->enable_flag == OSAL_TRUE) {
                // 暂停obss_pd统计
                hal_set_sr_statistics_stop();
            }
            if (sr_handler->enable_flag_psr == OSAL_TRUE) {
                // 暂停psr统计
                hal_set_psr_statistics_stop();
            }
        }
    }

    return OSAL_TRUE;
}

#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_bool hmac_sta_sr_enable(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    osal_void *mem_ptr = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;

    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (g_hmac_sta_sr_info[vap_id] != OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_SR, "vap_id[%d] hmac_sta_sr_enable mem already malloc!", vap_id);
        return OSAL_TRUE;
    }

    mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(hmac_sta_sr_stru), OAL_TRUE);
    if (mem_ptr == OSAL_NULL) {
        oam_error_log1(0, OAM_SF_SR, "vap_id[%d] hmac_sta_sr_enable malloc null!", vap_id);
        return OSAL_FALSE;
    }

    (osal_void)memset_s(mem_ptr, sizeof(hmac_sta_sr_stru), 0, sizeof(hmac_sta_sr_stru));
    g_hmac_sta_sr_info[vap_id] = (hmac_sta_sr_stru *)mem_ptr;

    return OSAL_TRUE;
}
#endif

OSAL_STATIC osal_bool hmac_sta_sr_disable(osal_void *notify_data)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)notify_data;
    mac_sta_sr_handler *sr_handler = OSAL_NULL;
    osal_u8 vap_id = hmac_vap->vap_id;

    if (vap_id >= WLAN_VAP_MAX_NUM_PER_DEVICE_LIMIT) {
        return OSAL_FALSE;
    }

    if (g_hmac_sta_sr_info[vap_id] == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CSA, "vap_id[%d] hmac_sta_sr_disable mem already free!", vap_id);
        return OSAL_TRUE;
    }

    sr_handler = &(g_hmac_sta_sr_info[vap_id]->sr_handler);
    if (sr_handler->sr_obss_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(sr_handler->sr_obss_timer));
    }

    if (sr_handler->psr_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(sr_handler->psr_timer));
    }

    if (sr_handler->sr_vfs_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(sr_handler->sr_vfs_timer));
    }

    oal_mem_free((osal_void *)g_hmac_sta_sr_info[vap_id], OAL_TRUE);
    g_hmac_sta_sr_info[vap_id] = OSAL_NULL;

    return OSAL_TRUE;
}

OSAL_STATIC osal_u16 hmac_encap_color_collision(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    oal_netbuf_stru *buffer, mac_color_area_stru *dth_color_area)
{
    hmac_color_event_rpt_elements_stru *rpt_elements = OSAL_NULL;
    osal_ulong index = 0;
    osal_u16 len = 0;
    osal_u8 *mac_header = OSAL_NULL;
    osal_u8 *payload_addr = OSAL_NULL;

    /* 入参检查 */
    if ((hmac_user == OSAL_NULL) || (buffer == OSAL_NULL)) {
        return len;
    }

    mac_header = oal_netbuf_header(buffer);
    payload_addr = mac_netbuf_get_payload(buffer);
    if (payload_addr == OSAL_NULL) {
        return len;
    }

    /*************************************************************************/
    /* Frame Control Field 中只需要设置Type/Subtype值，其他设置为0 */
    mac_hdr_set_frame_control(mac_header, WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    /* DA is address of STA addr */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR1_OFFSET, hmac_user->user_mac_addr);
    /* SA的值为本身的MAC地址 */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_station_id(hmac_vap));
    /* TA的值为VAP的BSSID */
    oal_set_mac_addr(mac_header + WLAN_HDR_ADDR3_OFFSET, hmac_vap->bssid);
    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(mac_header, 0);

    /* 设置Category */
    payload_addr[index++] = MAC_ACTION_CATEGORY_WNM;
    /* 设置Action */
    payload_addr[index++] = MAC_WNM_ACTION_EVENT_REPORT;
    /* 设置Dialog Token */
    payload_addr[index++] = 0;

    rpt_elements = (hmac_color_event_rpt_elements_stru *)(payload_addr + index);
    /* 设置Event Report */
    rpt_elements->eid = MAC_EID_EVENT_REPORT; /* 79:代表是event report */
    rpt_elements->length = sizeof(hmac_color_event_rpt_elements_stru) - MAC_IE_HDR_LEN; /* 19:代表ie长度 */
    rpt_elements->event_token = 0;
    rpt_elements->event_type = 4; /* 4:代表type是report */
    rpt_elements->event_report_status = 0;
    rpt_elements->event_tsf = dth_color_area->tsf_h;
    rpt_elements->event_tsf = (rpt_elements->event_tsf << 0x20) + dth_color_area->tsf_l;
    rpt_elements->event_report = dth_color_area->bss_color_bitmap_h;
    rpt_elements->event_report = dth_color_area->bss_color_bitmap_l + (rpt_elements->event_report << 0x20);
    index += sizeof(hmac_color_event_rpt_elements_stru);

    return (osal_u16)(index + MAC_80211_FRAME_LEN);
}

OSAL_STATIC osal_u32 hmac_tx_color_collision(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
    mac_color_area_stru *dth_color_area)
{
    osal_u16 frame_len;
    osal_u32 ret;
    mac_tx_ctl_stru *tx_ctl = OSAL_NULL;

    /* 将入参函数指针保存到用户结构体下 */
    oal_netbuf_stru *buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (buf == OSAL_NULL) {
        oam_error_log1(0, 0, "vap_id[%d] {hmac_tx_color_collision::malloc null.}", hmac_vap->vap_id);
        hmac_report_collision((hmac_vap_stru *)(hmac_vap));
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(buf, OAL_PTR_NULL);
    oal_set_netbuf_next(buf, OAL_PTR_NULL);

    /* 调用封装管理帧接口 */
    frame_len = hmac_encap_color_collision(hmac_vap, hmac_user, buf, dth_color_area);
    if (frame_len == 0) {
        hmac_report_collision((hmac_vap_stru *)(hmac_vap));
        hmac_dft_print_drop_frame_info(THIS_FILE_ID, __LINE__, 1, OAL_PTR_NULL);
        oal_netbuf_free(buf);
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    memset_s(oal_netbuf_cb(buf), OAL_NETBUF_CB_SIZE(), 0, OAL_NETBUF_CB_SIZE());
    tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(buf);
    mac_get_cb_tx_user_idx(tx_ctl) = (osal_u8)hmac_user->assoc_id;
    mac_get_cb_wme_ac_type(tx_ctl) = WLAN_WME_AC_MGMT;

    mac_get_cb_mpdu_len(tx_ctl) = frame_len;
    oal_netbuf_put(buf, frame_len);

    /* 抛事件让dmac将该帧发送 */
    ret = hmac_tx_mgmt_send_event_etc(hmac_vap, buf, frame_len);
    if (ret != OAL_SUCC) {
        oal_netbuf_free(buf);
        oam_error_log1(0, OAM_SF_ANY, "vap_id[%d] hmac_tx_color_collision:send event failed.", hmac_vap->vap_id);
    }

    hmac_report_collision(hmac_vap);
    return ret;
}

OSAL_STATIC osal_void hmac_report_collision(hmac_vap_stru *hmac_vap)
{
    mac_sta_collision_handler *collision_handler = OSAL_NULL;
    osal_s32 ret;
    osal_u8 col_en = OSAL_TRUE;

    collision_handler = &hmac_vap->collision_handler;
    collision_handler->collision_timeout = 8000; /* 8000:上报周期为8s */
    ret = hmac_config_set_collision_on(hmac_vap, col_en);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_report_collision::config collision en err.");
        return;
    }

    frw_create_timer_entry(&(collision_handler->collision_timer), hmac_report_collision_callback,
                           collision_handler->collision_timeout, hmac_vap, OAL_FALSE);
}

OSAL_STATIC oal_bool_enum_uint8 hmac_check_collision(const mac_color_area_stru *color_area, mac_he_hdl_stru he_hdl)
{
    if (he_hdl.he_oper_ie.bss_color.bss_color <= 31) { /* 31:代表颜色第31 */
        if ((color_area->bss_color_bitmap_l & (1 << he_hdl.he_oper_ie.bss_color.bss_color)) != 0) {
            return OSAL_TRUE;
        } else {
            return OSAL_FALSE;
        }
    } else {
        /* 32:代表颜色第32 */
        if ((color_area->bss_color_bitmap_h & (1 << (he_hdl.he_oper_ie.bss_color.bss_color - 32))) != 0) {
            return OSAL_TRUE;
        } else {
            return OSAL_FALSE;
        }
    }
}

OAL_STATIC osal_s32 hmac_config_collision_syn(hmac_vap_stru *hmac_vap, mac_color_area_stru *dth_color_area)
{
    hmac_user_stru *hmac_user = OSAL_NULL;
    mac_he_hdl_stru he_hdl;
    osal_bool state;

    if ((hmac_vap == OSAL_NULL) || (dth_color_area == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_COEX, "{hmac_config_collision_syn::vap or param null.}");
        hmac_report_collision(hmac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_user = mac_res_get_hmac_user_etc(hmac_vap->assoc_vap_id);
    if (osal_unlikely(hmac_user == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_COEX, "{hmac_config_collision_syn::user null.}");
        hmac_report_collision(hmac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_get_he_hdl(hmac_user, &he_hdl);
    state = hmac_check_collision(dth_color_area, he_hdl);
    if (state == OSAL_TRUE) {
        if (hmac_tx_color_collision(hmac_vap, hmac_user, dth_color_area) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_COEX, "{hmac_config_collision_syn::tx_color_collision fail.}");
            return OAL_FAIL;
        }
    } else {
        hmac_report_collision(hmac_vap);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_config_set_collision_on
 功能描述  : 设置collision冲突计算使能
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_config_set_collision_on(hmac_vap_stru *hmac_vap, osal_u8 col_en)
{
    mac_color_area_stru dth_color_area;

    if (col_en == OSAL_FALSE) {
        hal_color_rpt_en(OSAL_FALSE);
        hal_color_area_get(&dth_color_area);
        hh503_vap_tsf_get_64bit(hmac_vap->hal_vap, &dth_color_area.tsf_h, &dth_color_area.tsf_l);
        hmac_config_collision_syn(hmac_vap, &dth_color_area);
        hal_color_rpt_clr();
    } else {
        hal_color_rpt_en(OSAL_TRUE);
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_report_collision_callback(osal_void *ptr)
{
    hmac_vap_stru *hmac_vap = (hmac_vap_stru *)ptr;
    osal_s32 ret;
    osal_u8 col_en = OSAL_FALSE;

    ret = hmac_config_set_collision_on(hmac_vap, col_en);
    if (osal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_report_collision_callback::config collision err.");
        return (osal_u32)ret;
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_u32 hmac_ie_parse_spatial_reuse_parameter(osal_u8 *he_srp_ie,
    mac_frame_he_spatial_reuse_parameter_set_ie_stru *he_srp_value)
{
    osal_u8 *he_buffer = OSAL_NULL;
    mac_frame_he_sr_control_stru *he_sr_control = OSAL_NULL;

    if ((he_srp_ie == OSAL_NULL) || (he_srp_value == OSAL_NULL)) {
        oam_error_log2(0, OAM_SF_11AX,
            "hmac_ie_parse_spatial_reuse_parameter::param null,he_srp_ie[%p],he_srp_value[%p].",
            (uintptr_t)he_srp_ie, (uintptr_t)he_srp_value);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /************************ Spatial Reuse Parameter Set Element ***************************/
    /* ------------------------------------------------------------------------------ */
    /* | EID | LEN | Ext EID|SR Control |Non-SRG OBSS PD Max Offset | SRG OBSS PD Min Offset  | */
    /* ------------------------------------------------------------------------------ */
    /* |  1   |  1   |   1       |    1          |     0 or 1              | 0 or 1 | */
    /* ------------------------------------------------------------------------------ */
    /* ------------------------------------------------------------------------------ */
    /* |SRG OBSS PD Max Offset |SRG BSS Color Bitmap  | SRG Partial BSSID Bitmap |    */
    /* ------------------------------------------------------------------------------ */
    /* |    0 or 1                       |     0 or 8                     | 0 or 8  | */
    /* -------------------------------------------------------------------------------*/
    /**************************************************************************/

    he_buffer = he_srp_ie + 3;  /* he_srp_ie头部偏移3字节 */

    /* SR Control */
    he_sr_control = (mac_frame_he_sr_control_stru *)he_buffer;
    he_buffer += sizeof(mac_frame_he_sr_control_stru);
    (osal_void)memcpy_s((osal_u8 *)(&he_srp_value->sr_control), sizeof(mac_frame_he_sr_control_stru),
        (osal_u8 *)he_sr_control, sizeof(mac_frame_he_sr_control_stru));

    if (he_sr_control->non_srg_offset_present == 1) {
        he_srp_value->non_srg_boss_pd_offset_max = *he_buffer;
        he_buffer += 1;
    }

    if (he_sr_control->srg_information_present == 1) {
        /* SRG OBSS PD Min Offset */
        he_srp_value->srg_obss_pd_offset_min = *he_buffer;
        he_buffer += 1;

        /* SRG OBSS PD Max Offset */
        he_srp_value->srg_obss_pd_offset_max = *he_buffer;
        he_buffer += 1;

        /* SRG BSS Color Bitmap */
        (osal_void)memcpy_s((osal_u8 *)(&he_srp_value->srg_bss_color_bitmap_info),
            sizeof(he_srp_value->srg_bss_color_bitmap_info), (osal_u8 *)he_buffer, 8); /* 8:长度 */
        he_buffer += 8; /* 8:长度 */

        /* STG Partial BSSID Bitmap */
        (osal_void)memcpy_s((osal_u8 *)(&he_srp_value->ac_srg_partial_bssid_bitmap),
            sizeof(he_srp_value->ac_srg_partial_bssid_bitmap), (osal_u8 *)he_buffer, 8); /* 8:长度 */
        he_buffer += 8; /* 8:长度 */
    }

    return OAL_SUCC;
}

OSAL_STATIC osal_void hmac_sta_up_process_spatial_reuse_ie(hmac_vap_stru *hmac_vap, osal_u8 *payload,
    osal_u16 frame_len, osal_bool is_need_enable)
{
    osal_u32 ret;
    osal_u8 *ie = OSAL_NULL;
    mac_frame_he_spatial_reuse_parameter_set_ie_stru sr_ie_info;

    /* 支持11ax，才进行后续的处理 */
    if (mac_mib_get_he_option_implemented(hmac_vap) == OSAL_FALSE) {
        return;
    }

    ie = hmac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_SRP, payload, frame_len);
    if (ie == OSAL_NULL) {
        return;
    }

    memset_s(&sr_ie_info, sizeof(sr_ie_info), 0, sizeof(sr_ie_info));
    ret = hmac_ie_parse_spatial_reuse_parameter(ie, &sr_ie_info);
    if (ret != OAL_SUCC) {
        return;
    }

    if (is_need_enable) {
        hal_color_rpt_en(OSAL_TRUE);
    }

    hmac_sta_up_update_spatial_reuse_params(hmac_vap, &sr_ie_info, MAC_VAP_11AX_PARAM_CONFIG_UPDAT);

    return;
}
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
OSAL_STATIC osal_s32 hmac_ccpriv_spatial_reuse_enable(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_s8 ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8 sr_en_flag = 0;

    ret = hmac_ccpriv_get_one_arg(&param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "hmac_ccpriv_spatial_reuse_enable type return err_code [%d]", ret);
        return ret;
    }

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    sr_en_flag = ((osal_u8)oal_atoi((const osal_s8 *)ac_name)) & 0x1;
    if (sr_en_flag == OSAL_TRUE) {
        hmac_sta_sr_enable((osal_void *)(hmac_vap));
    } else {
        hmac_sta_sr_disable((osal_void *)(hmac_vap));
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取SR相关信息
*****************************************************************************/
OSAL_STATIC osal_s32 hmac_ccpriv_get_sr_info(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32             ret;
    osal_s8              cmd[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8              info;

    ret = hmac_ccpriv_get_one_arg(&param, cmd, OAL_SIZEOF(cmd));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{hmac_ccpriv_get_sr_info::return err_code [%d]}", ret);
        return ret;
    }
    info = (osal_u8)oal_atoi((const osal_s8 *)cmd);

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    hal_get_sr_info(info);

    return OAL_SUCC;
}

OSAL_STATIC osal_s32 hmac_ccpriv_collision_enable(hmac_vap_stru *hmac_vap, const osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 enable;
    osal_s8 ac_name[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_sta_collision_handler *collision_handler = OSAL_NULL;

    ret = hmac_ccpriv_get_one_arg(&param, ac_name, OAL_SIZEOF(ac_name));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ROAM, "uapi_ccpriv_collision_enable type return err_code [%d]", ret);
        return ret;
    }
    enable = (osal_u8)oal_atoi((const osal_s8 *)ac_name);

    if (hmac_vap == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    collision_handler = &hmac_vap->collision_handler;
    collision_handler->enable_flag = enable;

    wifi_printf("\n func[%s] line[%d] collision_handler->enable_flag[%u]\n",
        __func__, __LINE__, collision_handler->enable_flag);
    if (collision_handler->enable_flag) {
        hmac_report_collision(hmac_vap);
    } else {
        frw_destroy_timer_entry(&(collision_handler->collision_timer));
    }

    return OAL_SUCC;
}
#endif

osal_u32 hmac_sta_sr_init(osal_void)
{
    /* 消息接口注册 */
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_sta_sr_disable);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_SCAN_BEGIN, hmac_sta_sr_stop_statistics);
    frw_util_notifier_register(WLAN_UTIL_NOTIFIER_EVENT_SCAN_END, hmac_sta_sr_continue_statistics);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令注册 */
    hmac_ccpriv_register((const osal_s8 *)"sr_en", hmac_ccpriv_spatial_reuse_enable);
    hmac_ccpriv_register((const osal_s8 *)"get_sr_info", hmac_ccpriv_get_sr_info);
    hmac_ccpriv_register((const osal_s8 *)"collision_en", hmac_ccpriv_collision_enable);
#endif
    hmac_feature_hook_register(HMAC_FHOOK_SR_UPDATE_IE_INFO, hmac_sta_up_process_spatial_reuse_ie);

    return OSAL_SUCCESS;
}

osal_void hmac_sta_sr_deinit(osal_void)
{
    /* 消息接口注册 */
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_DEL_VAP, hmac_sta_sr_disable);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_SCAN_BEGIN, hmac_sta_sr_stop_statistics);
    frw_util_notifier_unregister(WLAN_UTIL_NOTIFIER_EVENT_SCAN_END, hmac_sta_sr_continue_statistics);
#ifdef _PRE_WLAN_SUPPORT_CCPRIV_CMD
    /* ccpriv命令去注册 */
    hmac_ccpriv_unregister((const osal_s8 *)"sr_en");
    hmac_ccpriv_unregister((const osal_s8 *)"get_sr_info");
    hmac_ccpriv_unregister((const osal_s8 *)"collision_en");
#endif
    hmac_feature_hook_unregister(HMAC_FHOOK_SR_UPDATE_IE_INFO);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
