/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_dfs.c
 * 生成日期   : 2014年10月16日
 */


#ifdef _PRE_WLAN_FEATURE_DFS
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hmac_dfs.h"
#include "hmac_fsm.h"
#include "hmac_resource.h"
#include "wlan_msg.h"
#include "mac_regdomain.h"
#include "hmac_chan_mgmt.h"
#include "hmac_chan_mgmt.h"
#include "mac_vap_ext.h"
#include "hmac_vap.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HMAC_DFS_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  2 结构体定义
*****************************************************************************/


/*****************************************************************************
  3 宏定义
*****************************************************************************/
#define hmac_dfs_is_chan_weather_radar(_us_freq) \
    ((5600 <= (_us_freq)) && ((_us_freq) <= 5650))


/*****************************************************************************
  4 全局变量定义
*****************************************************************************/
dmac_chan_mgmt_rom_cb  g_dmac_chan_mgmt_rom_cb  = {
    OSAL_NULL,
};

/*****************************************************************************
  5 内部静态函数声明
*****************************************************************************/
OAL_STATIC osal_u32  hmac_dfs_nol_addchan(hmac_device_stru *hmac_device, osal_u8 chan_idx);
OAL_STATIC osal_u32  hmac_dfs_nol_delchan(hmac_device_stru *hmac_device, mac_dfs_nol_node_stru *nol_node);
OAL_STATIC osal_u32  hmac_dfs_off_chan_cac_off_ch_dwell_timeout(osal_void *p_arg);
OAL_STATIC osal_u32  hmac_dfs_off_chan_cac_opern_ch_dwell_timeout(osal_void *p_arg);


/*****************************************************************************
  4 函数实现
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_dfs_init
 功能描述  : DFS特性初始化函数
 输入参数  : hmac_device: MAC DEVICE指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void hmac_dfs_init(hmac_device_stru *hmac_device)
{
    mac_dfs_core_stru *dfs = OSAL_NULL;
    mac_dfs_info_stru *dfs_info = OSAL_NULL;

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (dfs == OSAL_NULL) {
        osal_void *mem_ptr = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, sizeof(mac_dfs_core_stru), OAL_TRUE);
        if (mem_ptr == OSAL_NULL) {
            oam_error_log0(0, OAM_SF_DFS, "hmac_dfs_init ftm malloc null!");
            return;
        }

        (osal_void)memset_s(mem_ptr, sizeof(mac_dfs_core_stru), 0, sizeof(mac_dfs_core_stru));
        hmac_device_feature_registered(hmac_device, WLAN_FEATURE_INDEX_DFS, mem_ptr);

        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_init malloc succ!}");
    } else {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_init already init, do not Re-malloc!}");
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    dfs->dfs_timeout = 0;

    /* 软件雷达检测默认关闭 */
    mac_dfs_set_dfs_enable(hmac_device, OAL_TRUE);
    /* debug默认关闭 */
    mac_dfs_set_debug_level(hmac_device, 0);

    /* 初始化Non-Occupancy List链表 */
    OSAL_INIT_LIST_HEAD(&(dfs->dfs_nol));

    /* CAC检测默认不使能 */
    mac_dfs_set_cac_enable(hmac_device, OAL_TRUE);
    /* OFFCHAN-CAC检测默认不使能 */
    mac_dfs_set_offchan_cac_enable(hmac_device, OAL_FALSE);

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    /* 设置CAC, Off-Channel CAC, etc... 超时时间 */
    dfs_info = &(dfs->dfs_info);

    dfs_info->dfs_cac_outof_5600_to_5650_time_ms      = HMAC_DFS_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->dfs_cac_in_5600_to_5650_time_ms         = HMAC_DFS_CAC_IN_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->off_chan_cac_outof_5600_to_5650_time_ms = HMAC_DFS_OFF_CH_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->off_chan_cac_in_5600_to_5650_time_ms    = HMAC_DFS_OFF_CH_CAC_IN_5600_TO_5650_MHZ_TIME_MS;
    dfs_info->dfs_off_chan_cac_opern_chan_dwell_time  = HMAC_DFS_OFF_CHAN_CAC_PERIOD_TIME_MS;
    dfs_info->dfs_off_chan_cac_off_chan_dwell_time    = HMAC_DFS_OFF_CHAN_CAC_DWELL_TIME_MS;
    dfs_info->dfs_non_occupancy_period_time_ms        = HMAC_DFS_NON_OCCUPANCY_PERIOD_TIME_MS;
    dfs_info->dfs_init                                = OAL_FALSE;

    /* 默认当前信道为home channel */
    dfs_info->offchan_flag = 0;
}

osal_void hmac_dfs_deinit(hmac_device_stru *hmac_device)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (dfs == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_DFS, "hmac_dfs_deinit dfs is null!");
        return;
    }

    if (dfs->dfs_cac_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(dfs->dfs_cac_timer));
    }

    if (dfs->dfs_off_chan_cac_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(dfs->dfs_off_chan_cac_timer));
    }

    if (dfs->dfs_chan_dwell_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(dfs->dfs_chan_dwell_timer));
    }

    if (dfs->dfs_radar_timer.is_registerd == OSAL_TRUE) {
        frw_destroy_timer_entry(&(dfs->dfs_radar_timer));
    }

    oal_mem_free(dfs, OAL_TRUE);

    hmac_device_feature_unregister(hmac_device, WLAN_FEATURE_INDEX_DFS);
    oam_warning_log0(0, OAM_SF_FTM, "{hmac_dfs_deinit!}");
}
/*****************************************************************************
 函 数 名  : hmac_dfs_channel_list_init_etc
 功能描述  : DFS信道列表初始化
 输入参数  : hmac_device: MAC DEVICE指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void  hmac_dfs_channel_list_init_etc(hmac_device_stru *hmac_device)
{
    mac_chan_status_enum_uint8    ch_status;
    osal_u8                     idx;
    osal_u32                    ul_ret;
    for (idx = 0; idx < MAC_MAX_SUPP_CHANNEL; idx++) {
        ul_ret = hmac_is_channel_idx_valid_etc(MAC_RC_START_FREQ_5, idx);
        if (ul_ret == OAL_SUCC) {
            if (mac_is_ch_in_radar_band(MAC_RC_START_FREQ_5, idx) == OAL_TRUE) {
                /* DFS信道 */
                ch_status = MAC_CHAN_DFS_REQUIRED;
            } else {
                ch_status = MAC_CHAN_AVAILABLE_ALWAYS;
            }
        } else {
            /* 管制域不支持 */
            ch_status = MAC_CHAN_NOT_SUPPORT;
        }

        hmac_device->ap_channel_list[idx].ch_status = ch_status;
    }
}

/*****************************************************************************
 函 数 名  : hmac_dfs_find_available_chan
 功能描述  : 从管制域支持的信道中，选择最低可用信道
 输入参数  : hmac_device: MAC DEVICE结构体指针
             hmac_vap   : MAC VAP结构体指针
 输出参数  : 无
 返 回 值  : 信道号
*****************************************************************************/
OAL_STATIC osal_u8  hmac_dfs_find_lowest_available_channel(hmac_device_stru *hmac_device)
{
    osal_u8     channel_num = 36;
    osal_u8     chan_idx;
    osal_u8     num_supp_chan = mac_get_num_supp_channel(hmac_device->max_band);
    osal_u32    ul_ret;

    for (chan_idx = 0; chan_idx < num_supp_chan; chan_idx++) {
        ul_ret = hmac_is_channel_idx_valid_etc(hmac_device->max_band, chan_idx);
        if (ul_ret != OAL_SUCC) {
            continue;
        }

        if ((hmac_device->ap_channel_list[chan_idx].ch_status != MAC_CHAN_NOT_SUPPORT) &&
            (hmac_device->ap_channel_list[chan_idx].ch_status != MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
            hmac_get_channel_num_from_idx_etc(hmac_device->max_band, chan_idx, &channel_num);

            return channel_num;
        }
    }

    /* should not be here */
    return channel_num;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_recalculate_channel_etc
 功能描述  : 是否需要重新计算(更改)可用信道
 输入参数  : hmac_device: MAC DEVICE结构体指针
 输出参数  : freq      : 信道号
 返 回 值  : OAL_TRUE : 需要
             OAL_FALSE: 不需要
*****************************************************************************/
osal_u32  hmac_dfs_recalculate_channel_etc(hmac_device_stru *hmac_device,
    osal_u8 *freq, wlan_channel_bandwidth_enum_uint8 *pen_bandwidth)
{
    mac_channel_list_stru    chan_info;
    osal_u8                 chan_idx;
    osal_u8                 loop;
    osal_u32                ul_ret;
    oal_bool_enum_uint8       recalc = OAL_FALSE;

    ul_ret = hmac_get_channel_idx_from_num_etc(hmac_device->max_band, *freq, &chan_idx);
    if (ul_ret != OAL_SUCC) {
        oam_error_log3(0, OAM_SF_DFS,
            "vap_id[%d] {hmac_dfs_recalculate_channel_etc::fail max_band:%d  freq:%x}",
            hmac_device->device_id, hmac_device->max_band, freq);
        return ul_ret;
    }

    hmac_get_ext_chan_info(chan_idx, *pen_bandwidth, &chan_info);

    for (loop = 0; loop < chan_info.channel; loop++) {
        chan_idx = chan_info.channels[loop].idx;

        if ((hmac_device->ap_channel_list[chan_idx].ch_status == MAC_CHAN_NOT_SUPPORT) ||
            (hmac_device->ap_channel_list[chan_idx].ch_status == MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
            recalc = OAL_TRUE;
            break;
        }
    }

    if (recalc == OAL_FALSE) {
        return OAL_FALSE;
    }

    /* 如果由于雷达干扰导致当前信道不可用，则需要从管制域支持的信道中，选择最低可用信道，带宽20MHz */
    *freq = hmac_dfs_find_lowest_available_channel(hmac_device);

    *pen_bandwidth = WLAN_BAND_WIDTH_20M;

    return OAL_TRUE;
}
/*****************************************************************************
 函 数 名  : hmac_dfs_cac_event_report
 功能描述  : cac事件上报到wal层
 输入参数  : event_mem: 事件内存指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
static OAL_INLINE osal_u32  hmac_dfs_cac_event_report(hmac_vap_stru *hmac_vap,
    hmac_cac_event_stru *cac_event)
{
    osal_u32 ret;
    frw_msg msg_info = {0};

    msg_info.data = (osal_u8 *)cac_event;
    msg_info.data_len = OAL_SIZEOF(hmac_cac_event_stru);

    ret = frw_asyn_host_post_msg(WLAN_MSG_H2W_CAC_REPORT, FRW_POST_PRI_LOW, hmac_vap->vap_id, &msg_info);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    return ret;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_radar_detect_event_etc
 功能描述  : HMAC 雷达中断处理入口函数
 输入参数  : event_mem: 事件内存指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_s32 hmac_dfs_radar_detect_event_etc(hmac_vap_stru *hmac_vap)
{
    hmac_misc_input_stru    misc_input;

    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_event_etc::null param.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    misc_input.type = HMAC_MISC_RADAR;
    hmac_fsm_call_func_ap_etc(hmac_vap, HMAC_FSM_INPUT_MISC, &misc_input);

    return OAL_SUCC;
}

osal_u32  hmac_dfs_radar_detect_event_test(osal_u8 vap_id)
{
    hmac_vap_stru          *hmac_vap;
    hmac_misc_input_stru    misc_input;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_event_test::hmac_vap is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    misc_input.type = HMAC_MISC_RADAR;

    hmac_fsm_call_func_ap_etc(hmac_vap, HMAC_FSM_INPUT_MISC, &misc_input);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_update_channel_list
 功能描述  : 更新信道信息
 输入参数  : hmac_device   : MAC DEVICE指针
             chan_idx      : 信道索引
             radar_detected: 是否检测到雷达
 输出参数  : hmac_device   : MAC DEVICE指针
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_dfs_update_available_channel_list(hmac_device_stru *hmac_device,
    osal_u8 chan_idx, oal_bool_enum_uint8 radar_detected)
{
    osal_u32    ul_ret;

    ul_ret = hmac_is_channel_idx_valid_etc(hmac_device->max_band, chan_idx);
    if (ul_ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_DFS, "{hmac_dfs_update_available_channel_list::chan_idx(%d) invalid!}", chan_idx);
        wifi_printf("[DFS]hmac_dfs_update_available_channel_list::ch is not available.\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (hmac_device->ap_channel_list[chan_idx].ch_status == MAC_CHAN_AVAILABLE_ALWAYS) {
        oam_warning_log1(0, OAM_SF_DFS,
            "{hmac_dfs_update_available_channel_list::Radar detected in Non-Radar Channel(%d)!}", chan_idx);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (radar_detected == OAL_TRUE) {
        hmac_device->ap_channel_list[chan_idx].ch_status = MAC_CHAN_BLOCK_DUE_TO_RADAR;
        wifi_printf("[DFS]ch status is changed to due to radar.\n");
    } else {
        hmac_device->ap_channel_list[chan_idx].ch_status = MAC_CHAN_AVAILABLE_TO_OPERATE;
    }

    hmac_config_ch_status_sync(hmac_device);

    return OAL_SUCC;
}

osal_void hmac_dfs_cac_timeout_fn_report(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    osal_u8                 idx;
    osal_u32                ul_ret;
    hmac_cac_event_stru       cac_event = {0};

    // 调用内核接口通知应用层CAC启动
    idx = hmac_vap->channel.chan_idx;
    ul_ret = hmac_get_channel_idx_from_num_etc(MAC_RC_START_FREQ_5, hmac_device->max_channel, &idx);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFS,
            "{[DFS]hmac_dfs_cac_timeout_fn_etc::hmac_get_channel_idx_from_num_etc failed=%d}", ul_ret);
    }
    cac_event.type = HMAC_CAC_FINISHED;
#ifdef _PRE_WLAN_SUPPORT_5G
    cac_event.freq = hmac_regdomain_get_freq_map_5g_etc()[idx].freq;
#endif
    cac_event.bw_mode = hmac_vap->channel.en_bandwidth;
    ul_ret = hmac_dfs_cac_event_report(hmac_vap, &cac_event);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFS,
            "{[DFS]hmac_dfs_cac_timeout_fn_etc::hmac_dfs_cac_event_report failed=%d}", ul_ret);
    }
}

/*****************************************************************************
 函 数 名  : hmac_dfs_cac_timeout_fn_etc
 功能描述  : Channel Availability Check(CAC) 超时函数
 输入参数  : p_arg: HMAC VAP的指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32 hmac_dfs_cac_timeout_fn_etc(osal_void *p_arg)
{
    hmac_device_stru         *hmac_device;
    hmac_vap_stru            *hmac_vap;
    mac_channel_list_stru    chan_info;
    osal_u8                  idx;
    mac_dfs_core_stru        *dfs = OSAL_NULL;

    if (OAL_UNLIKELY(p_arg == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_timeout_fn_etc::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;

    wifi_printf("[DFS]hmac_dfs_cac_timeout_fn_etc, CAC expired, channel number:%d.\n", hmac_vap->channel.chan_number);

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_timeout_fn_etc::hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_timeout_fn_etc dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs->dfs_info.timer_cnt++;

    if (dfs->dfs_info.timer_cnt != dfs->dfs_info.timer_end_cnt) {
        /* 启动 CAC 定时器 */
        frw_create_timer_entry(&dfs->dfs_cac_timer, hmac_dfs_cac_timeout_fn_etc,
            HMAC_DFS_ONE_MIN_IN_MS, hmac_vap, OAL_FALSE);
        return OAL_SUCC;
    }

    hmac_get_ext_chan_info(hmac_vap->channel.chan_idx,
        hmac_vap->channel.en_bandwidth, &chan_info);

    /* 将当前信道设置为工作信道 */
    for (idx = 0; idx < chan_info.channel; idx++) {
        (osal_void)hmac_dfs_update_available_channel_list(hmac_device, chan_info.channels[idx].idx, OAL_FALSE);
    }

    if (hmac_vap->vap_state == MAC_VAP_STATE_AP_WAIT_START) {
        /* 在该信道启动 BSS */
        hmac_start_all_bss_of_device_etc(hmac_device);
    } else if ((hmac_vap->vap_state == MAC_VAP_STATE_PAUSE) ||
        (hmac_vap->vap_state == MAC_VAP_STATE_UP)) {
        /* pause状态需要待定 */
        hmac_chan_restart_network_after_switch_etc(hmac_vap);
    }

    /* off-channel cac start */
    if (mac_dfs_get_offchan_cac_enable(hmac_device) == OAL_TRUE) {
        hmac_dfs_off_chan_cac_start_etc(hmac_device, hmac_vap);
    }

    /* 调用内核接口通知应用层CAC启动 */
    hmac_dfs_cac_timeout_fn_report(hmac_device, hmac_vap);

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_dfs_start_bss_etc
 功能描述  : setchannel时判断是否需要cac检测，不需要则启动vap
 输入参数  : hmac_vap: HMAC VAP的指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32  hmac_dfs_start_bss_etc(hmac_vap_stru *hmac_vap)
{
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_start_bss_etc::hmac_vap is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 在该信道启动 BSS */
    return hmac_start_bss_in_available_channel_etc(hmac_vap);
}


/*****************************************************************************
 函 数 名  : hmac_chan_get_cac_time
 功能描述  : 获取Channel Availability Check(CAC)检测时间
 输入参数  : hmac_device: MAC DEVICE指针
             hmac_vap   : MAC VAP指针
 输出参数  : 无
 返 回 值  : CAC检测时间
*****************************************************************************/
OAL_STATIC osal_u32  hmac_chan_get_cac_time(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    mac_regdomain_info_stru   *rd_info;
    mac_channel_list_stru     chan_info;
    osal_u8                  idx;
    osal_u32                 ul_ret;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_chan_get_cac_time dfs ptr is null!}");
        return 0;
    }

    hmac_get_regdomain_info_etc(&rd_info);

    if (rd_info->dfs_domain == MAC_DFS_DOMAIN_ETSI) {
        ul_ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band,
            hmac_vap->channel.chan_number, &idx);
        if (ul_ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_DFS,
                "{hmac_chan_get_cac_time::hmac_get_channel_idx_from_num_etc fail. band:%d, offchan_num:%u}",
                hmac_vap->channel.band, hmac_vap->channel.chan_number);
            return ul_ret;
        }
        hmac_get_ext_chan_info(idx, hmac_vap->channel.en_bandwidth, &chan_info);

        for (idx = 0; idx < chan_info.channel; idx++) {
            if (hmac_dfs_is_chan_weather_radar(chan_info.channels[idx].freq)) {
                return dfs->dfs_info.dfs_cac_in_5600_to_5650_time_ms;
            }
        }
    }

    return dfs->dfs_info.dfs_cac_outof_5600_to_5650_time_ms;
}


/*****************************************************************************
 函 数 名  : hmac_chan_get_off_chan_cac_time
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_chan_get_off_chan_cac_time(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    mac_channel_list_stru     chan_info;
    osal_u8                  idx;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_chan_get_off_chan_cac_time dfs ptr is null!}");
        return 0;
    }

    hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band,
        dfs->dfs_info.offchan_num, &idx);

    hmac_get_ext_chan_info(idx, hmac_vap->channel.en_bandwidth, &chan_info);

    for (idx = 0; idx < chan_info.channel; idx++) {
        if (hmac_dfs_is_chan_weather_radar(chan_info.channels[idx].freq)) {
            return dfs->dfs_info.off_chan_cac_in_5600_to_5650_time_ms;
        }
    }

    return dfs->dfs_info.off_chan_cac_outof_5600_to_5650_time_ms;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_cac_stop_etc
 功能描述  : Channel Availability Check(CAC)检测结束函数
 输入参数  : hmac_device: MAC DEVICE指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void hmac_dfs_cac_stop_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap, osal_u8 cac_tx_en)
{
    osal_u32  ul_ret;
    osal_u8   idx;
    hmac_cac_event_stru   cac_event;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop_etc dfs ptr is null!}");
        return;
    }
    if (cac_tx_en == OSAL_TRUE) {
        /* vap down/del之前将tx队列打开，避免softAP再次启动时报文无法发送 */
        hmac_cac_chan_ctrl_machw_tx(mac_vap, DFS_CAC_CTRL_TX_OPEN);
    }
    if (dfs->dfs_cac_timer.is_enabled == OAL_TRUE) {
        /* 关闭CAC检测时长定时器 */
        frw_destroy_timer_entry(&(dfs->dfs_cac_timer));
        dfs->dfs_cac_timer.is_enabled = OAL_FALSE;

        // 调用内核接口通知应用层CAC停止
        idx = hmac_vap->channel.chan_idx;
        cac_event.type = HMAC_CAC_ABORTED;
#ifdef _PRE_WLAN_SUPPORT_5G
        cac_event.freq = hmac_regdomain_get_freq_map_5g_etc()[idx].freq;
#endif
        cac_event.bw_mode = hmac_vap->channel.en_bandwidth;
        ul_ret = hmac_dfs_cac_event_report(hmac_vap, &cac_event);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_DFS, "{[DFS]hmac_dfs_cac_stop_etc::hmac_dfs_cac_event_report fail=%d}", ul_ret);
        }

        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop_etc::[DFS]CAC timer stopped.}");
    }
}

osal_void hmac_dfs_off_cac_stop_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_off_cac_stop_etc dfs ptr is null!}");
        return;
    }

    if (dfs->dfs_off_chan_cac_timer.is_enabled == OAL_TRUE) {
        /* 关闭OFF CAC检测时长定时器 */
        frw_destroy_timer_entry(&(dfs->dfs_off_chan_cac_timer));

        oam_info_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop_etc::[DFS]CAC big timer stopped.}");
    }
    if (dfs->dfs_chan_dwell_timer.is_enabled == OAL_TRUE) {
        /* 关闭OFFCAC信道驻留定时器 */
        frw_destroy_timer_entry(&(dfs->dfs_chan_dwell_timer));
        oam_info_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_stop_etc::[DFS]CAC timer stopped.}");
    }
    dfs->dfs_info.offchan_flag = 0;
}


/*****************************************************************************
 函 数 名  : hmac_dfs_cac_start_etc
 功能描述  : Channel Availability Check(CAC)检测开始函数
 输入参数  : hmac_device: MAC DEVICE指针
             hmac_vap  : HMAC VAP指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void hmac_dfs_cac_start_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    osal_u32   scan_time;
    osal_u32   ul_ret;
    osal_u8    idx;
    hmac_cac_event_stru cac_event;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_cac_start_etc dfs ptr is null!}");
        return;
    }

    /* 如果已经启动 CAC 定时器，则直接返回 */
    if (dfs->dfs_cac_timer.is_registerd == OAL_TRUE) {
        /* 关闭CAC检测时长定时器 */
        frw_destroy_timer_entry(&(dfs->dfs_cac_timer));
    }

    /* 设置 CAC 检测时间 */
    scan_time = hmac_chan_get_cac_time(hmac_device, hmac_vap);
    oam_warning_log2(0, OAM_SF_DFS, "start cac time=%d ms ch=%d\n",
        scan_time, hmac_vap->channel.chan_number);

    if (scan_time < HMAC_DFS_ONE_MIN_IN_MS) {
        dfs->dfs_info.timer_cnt     = 0;
        dfs->dfs_info.timer_end_cnt = 1;

        /* 启动 CAC 定时器 */
        frw_create_timer_entry(&dfs->dfs_cac_timer,
                               hmac_dfs_cac_timeout_fn_etc,
                               scan_time,
                               hmac_vap,
                               OAL_FALSE);
    } else {
        dfs->dfs_info.timer_cnt     = 0;
        dfs->dfs_info.timer_end_cnt = (osal_u8)(scan_time / HMAC_DFS_ONE_MIN_IN_MS);

        /* 启动 CAC 定时器 */
        frw_create_timer_entry(&dfs->dfs_cac_timer,
                               hmac_dfs_cac_timeout_fn_etc,
                               HMAC_DFS_ONE_MIN_IN_MS,
                               hmac_vap,
                               OAL_FALSE);
    }

    // 通知wal CAC启动
    idx = hmac_vap->channel.chan_idx;
    cac_event.type = HMAC_CAC_STARTED;
#ifdef _PRE_WLAN_SUPPORT_5G
    cac_event.freq = hmac_regdomain_get_freq_map_5g_etc()[idx].freq;
#endif
    cac_event.bw_mode = hmac_vap->channel.en_bandwidth;
    ul_ret = hmac_dfs_cac_event_report(hmac_vap, &cac_event);
    if (ul_ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFS, "{[DFS]hmac_dfs_cac_start_etc::hmac_dfs_cac_event_report failed=%d}", ul_ret);
    }

    oam_warning_log2(0, OAM_SF_DFS, "{[DFS]hmac_dfs_cac_start_etc, CAC start, channel number:%d, timer cnt:%d.}",
        hmac_vap->channel.chan_number, dfs->dfs_info.timer_end_cnt);
}

/*****************************************************************************
 函 数 名  : hmac_dfs_radar_wait_etc
 输入参数  : hmac_device: MAC DEVICE指针
             chan_idx   : 信道索引
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void hmac_dfs_radar_wait_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    hmac_vap_stru    *hmac_vap_tmp = OSAL_NULL;
    osal_u8           vap_idx;
    osal_u8           uc_5g_vap_cnt = 0;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_wait_etc dfs ptr is null!}");
        return;
    }

    if (!hmac_vap || (hmac_vap->channel.band != WLAN_BAND_5G)) {
        return;
    }

    /* 如果雷达使能没有开启直接返回 */
    if (mac_dfs_get_dfs_enable(hmac_device) == OAL_FALSE) {
        return;
    }

    /* 遍历device下的所有vap，当所有5G vap都down后，将当前信道状态置为MAC_CHAN_DFS_REQUIRED */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        hmac_vap_tmp = mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if (OAL_UNLIKELY(hmac_vap_tmp == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_DFS, "{hmac_dfs_radar_wait_etc::hmac_vap null, vap id=%d.",
                hmac_device->vap_id[vap_idx]);

            continue;
        }

        if ((hmac_vap_tmp->vap_state == MAC_VAP_STATE_UP || hmac_vap_tmp->vap_state ==
            MAC_VAP_STATE_AP_WAIT_START) && (hmac_vap_tmp->channel.band == WLAN_BAND_5G)) {
            uc_5g_vap_cnt++;
        }
    }

    if (uc_5g_vap_cnt == 0) {
        oam_warning_log1(0, OAM_SF_DFS,
            "{hmac_dfs_radar_wait_etc::reset dfs status for device[%d] after all 5g vap down", hmac_device->device_id);
        hmac_dfs_channel_list_init_etc(hmac_device);
        dfs->dfs_info.dfs_init = OAL_TRUE;
        hmac_config_ch_status_sync(hmac_device);
    }
}
/*****************************************************************************
 函 数 名  : hmac_dfs_nol_timeout_fn
 功能描述  : NOL超时函数，超时后将某一信道从NOL列表中剔除
 输入参数  : p_arg: NOL Node节点指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_dfs_nol_timeout_fn(osal_void *p_arg)
{
    hmac_device_stru         *hmac_device;
    mac_dfs_nol_node_stru   *nol_node;
    osal_u8                chan_num = 0;

    if (OAL_UNLIKELY(p_arg == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_timeout_fn::p_arg is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    nol_node = (mac_dfs_nol_node_stru *)p_arg;

    hmac_device = hmac_res_get_mac_dev_etc(nol_node->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_timeout_fn::hmac_device is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_get_channel_num_from_idx_etc(hmac_device->max_band, nol_node->chan_idx, &chan_num);

    oam_info_log1(0, OAM_SF_DFS,
        "{[DFS]hmac_dfs_nol_timeout_fn, Non-Occupancy Period expired, remove channel %d from NOL.}", chan_num);

    return hmac_dfs_nol_delchan(hmac_device, nol_node);
}

/*****************************************************************************
 函 数 名  : hmac_dfs_nol_addchan
 功能描述  : 将指定信道加入NOL列表
 输入参数  : hmac_device: MAC DEVICE指针
             chan_idx   : 信道索引
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_dfs_nol_addchan(hmac_device_stru *hmac_device, osal_u8 chan_idx)
{
    mac_dfs_nol_node_stru   *nol_node;
    oal_dlist_head_stru     *dlist_head = OSAL_NULL;
    oal_dlist_head_stru     *dlist_pos = OSAL_NULL;
    oal_dlist_head_stru     *dlist_temp = OSAL_NULL;
    osal_u8                chan_num = 0;
    osal_u32               ul_ret;
    osal_bool                need_add_nol = OSAL_TRUE;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果不可占用周期为0，则不添加新的nol信道 */
    if (dfs->dfs_info.dfs_non_occupancy_period_time_ms == 0) {
        return OAL_SUCC;
    }

    dlist_head = &(hmac_device->dfs.dfs_nol);
    if (osal_unlikely(dlist_head == OSAL_NULL)) {
        return OSAL_TRUE;
    }

    /* NOL节点检查，避免同一信道多次申请内存进行不可占用期标记
     * 如果该信道已被标记，则使用之前标记的节点 */
    oal_dlist_search_for_each_safe(dlist_pos, dlist_temp, dlist_head) {
        nol_node = oal_dlist_get_entry(dlist_pos, mac_dfs_nol_node_stru, entry);
        if (nol_node == OSAL_NULL) {
            continue;
        }
        if ((nol_node->chan_idx == chan_idx) && (nol_node->device_id == hmac_device->device_id)) {
            need_add_nol = OSAL_FALSE;
            break;
        }
    }

    if (need_add_nol == OSAL_TRUE) {
        nol_node = (mac_dfs_nol_node_stru *)oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL,
            OAL_SIZEOF(mac_dfs_nol_node_stru), OAL_TRUE);
        if (OAL_UNLIKELY(nol_node == OAL_PTR_NULL)) {
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        (osal_void)memset_s(nol_node, OAL_SIZEOF(mac_dfs_nol_node_stru), 0, OAL_SIZEOF(mac_dfs_nol_node_stru));
        nol_node->chan_idx  = chan_idx;
        nol_node->device_id = hmac_device->device_id;
        oal_dlist_add_tail(&(nol_node->entry), &(hmac_device->dfs.dfs_nol));
    }
    hmac_get_channel_num_from_idx_etc(hmac_device->max_band, chan_idx, &chan_num);
    oam_warning_log1(0, OAM_SF_DFS, "{[DFS]hmac_dfs_nol_addchan, add channel:%d need_add:%d}", chan_num, need_add_nol);

    /* 更新可用信道列列表 */
    ul_ret = hmac_dfs_update_available_channel_list(hmac_device, chan_idx, OAL_TRUE);
    if (ul_ret == OAL_SUCC) {
        /* 启动Non-Occupancy Peroid定时器 */
        frw_create_timer_entry(&nol_node->dfs_nol_timer, hmac_dfs_nol_timeout_fn,
            dfs->dfs_info.dfs_non_occupancy_period_time_ms, nol_node, OAL_FALSE);
    }

    return ul_ret;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_nol_delchan
 功能描述  : 将指定信道从NOL列表中剔除
 输入参数  : hmac_device: MAC DEVICE指针
             nol_node  : NOL Node节点指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_dfs_nol_delchan(hmac_device_stru *hmac_device, mac_dfs_nol_node_stru *nol_node)
{
    /* 更新可用信道列列表 */
    hmac_device->ap_channel_list[nol_node->chan_idx].ch_status = MAC_CHAN_DFS_REQUIRED;

    osal_list_del(&nol_node->entry);

    oal_mem_free(nol_node, OAL_FALSE);

    hmac_config_ch_status_sync(hmac_device);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_nol_clear
 功能描述  : 清空NOL列表
 输入参数  : p_arg: NOL Node head节点指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC osal_u32  hmac_dfs_nol_clear(struct osal_list_head *dlist_head)
{
    hmac_device_stru         *hmac_device;
    mac_dfs_nol_node_stru   *nol_node;
    struct osal_list_head     *dlist_pos = OAL_PTR_NULL;
    struct osal_list_head     *dlist_temp = OAL_PTR_NULL;

    if (OAL_UNLIKELY(dlist_head == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_clear::dlist_head is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    osal_list_for_each_safe(dlist_pos, dlist_temp, dlist_head) {
        nol_node = osal_list_entry(dlist_pos, mac_dfs_nol_node_stru, entry);
        hmac_device = hmac_res_get_mac_dev_etc(nol_node->device_id);
        if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
            oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_nol_clear::hmac_device is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 删除相应的Non-Occupancy Peroid定时器 */
        frw_destroy_timer_entry(&nol_node->dfs_nol_timer);

        (osal_void)hmac_dfs_nol_delchan(hmac_device, nol_node);
    }

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_dfs_channel_mark_radar
 功能描述  : 将主、次信道标记为检测到雷达，并将主、次信道加入Non-Occupancy List(NOL)列表
 输入参数  : hmac_device: MAC DEVICE指针
             hmac_vap   : MAC VAP指针
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_dfs_channel_mark_radar(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    mac_channel_list_stru   chan_info;
    osal_u8                 idx;
    osal_u32                cnt = 0;
    osal_u32                ul_ret;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_channel_mark_radar dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (dfs->dfs_info.offchan_flag == (BIT0 | BIT1)) {
        ul_ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, dfs->dfs_info.offchan_num, &idx);
        if (ul_ret != OAL_SUCC) {
            oam_error_log2(0, OAM_SF_DFS,
                "vap_id[%d] {hmac_dfs_channel_mark_radar::hmac_get_channel_idx_from_num_etc failed = %d.}",
                hmac_vap->vap_id, ul_ret);
            return OAL_FAIL;
        }

        hmac_get_ext_chan_info(idx, hmac_vap->channel.en_bandwidth, &chan_info);
    } else {
        hmac_get_ext_chan_info(hmac_vap->channel.chan_idx,
            hmac_vap->channel.en_bandwidth, &chan_info);
    }

    for (idx = 0; idx < chan_info.channel; idx++) {
        // 只要有一条信道标记成功，就需要切换信道
        if (hmac_dfs_nol_addchan(hmac_device, chan_info.channels[idx].idx) == OAL_SUCC) {
            cnt++;
        }
    }

    return (osal_u32)((cnt > 0) ? OAL_SUCC : OAL_FAIL);
}
#define HMAC_40MHZ_BITMAP_MASK     0x1EFFFFFF
#define HMAC_80MHZ_BITMAP_MASK     0x00FFFFFF
#define HMAC_160MHZ_BITMAP_MASK    0x00FFFF

OAL_STATIC osal_u32 hmac_dfs_channel_bw_support_80plusplus(wlan_channel_bandwidth_enum_uint8 en_bandwidth,
    osal_u8 num_supp_chan, osal_u32 chan_bitmap, osal_u8 target_idx)
{
    osal_u32 window = 0x3;   /* 11b */
    osal_u8  step = 2;    /* 2 */
    osal_u8  idx;
    osal_u8  offset = (en_bandwidth == WLAN_BAND_WIDTH_40PLUS) ? 0 : 1;
    chan_bitmap &= HMAC_40MHZ_BITMAP_MASK;

    /* channel 36 ~ 144 */
    for (idx = 0; idx < MAC_CHANNEL149; idx += step) {              /* skip channel 140 */
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }

    /* channel 149 ~ 161 */
    for (idx = MAC_CHANNEL149; idx < MAC_CHANNEL165; idx += step) { /* skip channel 165 */
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }

    /* channel 184 ~ 196 */
    for (idx = MAC_CHANNEL184; idx < num_supp_chan; idx += step) {
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            if ((idx + offset) == target_idx) {
                return OAL_SUCC;
            }
        }
    }
    return OAL_CONTINUE;
}

OAL_STATIC osal_u32 hmac_dfs_channel_bw_support(wlan_channel_bandwidth_enum_uint8 en_bandwidth,
    osal_u8 num_supp_chan, osal_u32 chan_bitmap, osal_u8 target_idx)
{
    osal_u32         window;
    osal_u8          step;
    osal_u8          idx;
    osal_u8          offset;
    osal_u32         ul_ret;
    /* 20MHz */
    if (en_bandwidth == WLAN_BAND_WIDTH_20M) {
        window = 0x1;   /* 1b */
        step = 1;

        for (idx = 0; idx < num_supp_chan; idx += step) {
            if (((window << idx) == (chan_bitmap & (window << idx))) &&
                (idx == target_idx)) {
                return OAL_SUCC;
            }
        }
    } else if (en_bandwidth < WLAN_BAND_WIDTH_80PLUSPLUS) {
        ul_ret = hmac_dfs_channel_bw_support_80plusplus(en_bandwidth, num_supp_chan, chan_bitmap, target_idx);
        if (ul_ret != OAL_CONTINUE) {
            return ul_ret;
        }
    } else if (en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) {    /* 80MHz */
        window = 0xF;    /* 1111b */
        step = 4;    /* 4 */
        chan_bitmap &= HMAC_80MHZ_BITMAP_MASK;
        switch (en_bandwidth) {
            case WLAN_BAND_WIDTH_80PLUSMINUS:
                offset = 2;    /* 2 */
                break;
            case WLAN_BAND_WIDTH_80MINUSPLUS:
                offset = 1;
                break;
            case WLAN_BAND_WIDTH_80MINUSMINUS:
                offset = 3;    /* 3 */
                break;
            default:
                offset = 0;
        }

        /* channel 36 ~ 161 */
        for (idx = 0; idx < MAC_CHANNEL165; idx += step) {           /* JP channel 184, 188, 192, 196, */
            /* 4条信道组不了80MHz, 参考802.11 ac Annex E */
            if (((window << idx) == (chan_bitmap & (window << idx))) &&
                ((idx + offset) == target_idx)) {
                return OAL_SUCC;
            }
        }
    }
    return OAL_FAIL;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_is_channel_support_bw
 功能描述  : 判断当前信道作为主信道时是否支持该带宽模式
 输入参数  : hmac_vap      : MAC VAP结构体指针
 输出参数  : new_channel  : 主信道号
             new_bandwidth: 带宽模式
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_dfs_is_channel_support_bw(hmac_vap_stru *hmac_vap,
    osal_u8 channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    hmac_device_stru   *hmac_device;
    osal_u8          num_supp_chan = mac_get_num_supp_channel(hmac_vap->channel.band);
    osal_u32         chan_bitmap = 0;
    osal_u8          idx;
    osal_u8          target_idx;
    osal_u32         ul_ret;
    mac_dfs_core_stru *dfs = OSAL_NULL;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_DFS,
            "vap_id[%d] {hmac_dfs_is_channel_support_bw::hmac_device null, device_id = %d.}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_is_channel_support_bw dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = hmac_get_channel_idx_from_num_etc(hmac_vap->channel.band, channel, &target_idx);
    if (ul_ret != OAL_SUCC) {
        oam_error_log2(0, OAM_SF_DFS,
            "vap_id[%d] {hmac_dfs_is_channel_support_bw::get channel idx failed(%d).}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_FAIL;
    }

hmac_dfs_is_channel_support_bw_update:

    /* 获取可用信道位图 */
    for (idx = 0; idx < num_supp_chan; idx++) {
        if ((hmac_device->ap_channel_list[idx].ch_status != MAC_CHAN_NOT_SUPPORT) &&
            (hmac_device->ap_channel_list[idx].ch_status != MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
            if ((!dfs->dfs_info.custom_chanlist_bitmap) ||
                (dfs->dfs_info.custom_chanlist_bitmap & (0x01 << idx))) {
                chan_bitmap |= (0x1 << idx);
            }
        }
    }

    if (!chan_bitmap) {
        // 此策略经产品讨论，当因为雷达检测导致已经无可用信道可跳时，将
        // 原来被标记为雷达的信道全部清掉，重新开始选择，然后CAC
        if (hmac_dfs_nol_clear(&dfs->dfs_nol) != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_DFS,
                "vap_id[%d] {hmac_dfs_is_channel_support_bw::hmac_dfs_nol_clear() Fail!}", hmac_vap->vap_id);

            return OAL_FAIL;
        }

        goto hmac_dfs_is_channel_support_bw_update;
    }

    return hmac_dfs_channel_bw_support(en_bandwidth, num_supp_chan, chan_bitmap, target_idx);
}

OAL_STATIC osal_void hmac_dfs_get_chan_bitmap(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    osal_u8 num_supp_chan, osal_u32 *chan_bitmap)
{
    osal_u8 chan_idx;
    osal_u32 bitmap = *chan_bitmap;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_get_chan_bitmap dfs ptr is null!}");
        return;
    }

    for (chan_idx = 0; chan_idx < num_supp_chan; chan_idx++) {
        if ((hmac_device->ap_channel_list[chan_idx].ch_status != MAC_CHAN_NOT_SUPPORT) &&
            (hmac_device->ap_channel_list[chan_idx].ch_status != MAC_CHAN_BLOCK_DUE_TO_RADAR)) {
            if ((!dfs->dfs_info.custom_chanlist_bitmap) ||
                (dfs->dfs_info.custom_chanlist_bitmap & (0x01 << chan_idx))) {
                bitmap |= (0x1 << chan_idx);
            }
        }
    }
    oam_warning_log3(0, OAM_SF_DFS,
        "vap_id[%d] {hmac_dfs_select_random_channel::custom_chanlist_bitmap=0x%x,chan_bitmap=0x%x!}",
        hmac_vap->vap_id, dfs->dfs_info.custom_chanlist_bitmap, bitmap);

    *chan_bitmap = bitmap;
}

/* 数组最大长度num_supp_chan已和数组首地址一起传入，保证数组下标不会越界 */
OAL_STATIC osal_void hmac_dfs_get_channel_width_20(osal_u32 chan_bitmap, osal_u8 num_supp_chan,
    osal_u8 *available_chan_idx, osal_u8 *available_chan_cnt, osal_u8 *chan_idx)
{
    osal_u32 window = 0x1;   /* 1b */
    osal_u8 step = 1;
    osal_u8 idx;
    osal_u8 cnt = 0;

    for (idx = 0; idx < num_supp_chan; idx += step) {
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            available_chan_idx[cnt++] = idx;
        }
    }

    *available_chan_cnt = cnt;
    *chan_idx = idx;
}

/* 数组最大长度num_supp_chan已和数组首地址一起传入，保证数组下标不会越界 */
OAL_STATIC osal_void hmac_dfs_get_channel_width_80plusplus(osal_u32 chan_bitmap, osal_u8 num_supp_chan,
    osal_u8 *available_chan_idx, osal_u8 *available_chan_cnt, osal_u8 *chan_idx)
{
    osal_u32 window = 0x3;   /* 11b */
    osal_u8 step = 2;    /* 2 */
    osal_u8 idx;
    osal_u8 cnt = 0;

    /* channel 36 ~ 144 */
    for (idx = 0; idx < MAC_CHANNEL149; idx += step) {              /* skip channel 140 */
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            available_chan_idx[cnt++] = idx;
        }
    }

    /* channel 149 ~ 161 */
    for (idx = MAC_CHANNEL149; idx < MAC_CHANNEL165; idx += step) { /* skip channel 165 */
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            available_chan_idx[cnt++] = idx;
        }
    }

    /* channel 184 ~ 196 */
    for (idx = MAC_CHANNEL184; idx < num_supp_chan; idx += step) {
        if ((window << idx) == (chan_bitmap & (window << idx))) {
            available_chan_idx[cnt++] = idx;
        }
    }

    *available_chan_cnt = cnt;
    *chan_idx = idx;
}

OAL_STATIC osal_void hmac_dfs_get_channel_idx(hmac_vap_stru *hmac_vap, osal_u8 *available_chan_idx,
    osal_u8 available_chan_cnt, wlan_channel_bandwidth_enum_uint8 try_bandwidth, osal_u8 *chan_idx)
{
    osal_u32 window;
    osal_u8 cur_idx;
    osal_u8 sub = 0;
    osal_u8 cur_sub;
    osal_u8 cur_channel;

    /* 避免选择到邻近信道 */
    for (window = 0; window < available_chan_cnt; window++) {
        cur_idx = available_chan_idx[window];
        switch (try_bandwidth) {
            case WLAN_BAND_WIDTH_40MINUS:
            case WLAN_BAND_WIDTH_80MINUSPLUS:
                cur_idx += 1;
                break;

            case WLAN_BAND_WIDTH_80PLUSMINUS:
                cur_idx += 2;    /* 2 */
                break;

            case WLAN_BAND_WIDTH_80MINUSMINUS:
                cur_idx += 3;    /* 3 */
                break;
            default:
                break;
        }

        hmac_get_channel_num_from_idx_etc(hmac_vap->channel.band, cur_idx, &cur_channel);
        cur_sub = OAL_ABSOLUTE_SUB(hmac_vap->channel.chan_number, cur_channel);
        /* 选择更远信道 */
        if (cur_sub > sub) {
            sub = cur_sub;
            *chan_idx = cur_idx;
        }
    }
}

OAL_STATIC osal_u32 hmac_dfs_select_channel(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device,
    wlan_channel_bandwidth_enum_uint8 *bandwidth, osal_u8 *channel_idx)
{
    osal_u8 num_supp_chan = mac_get_num_supp_channel(hmac_vap->channel.band);
    osal_u32 chan_bitmap = 0;
    osal_u8 available_chan_idx[MAC_CHANNEL_FREQ_5_BUTT] = {0};
    osal_u8 available_chan_cnt = 0;
    osal_u8 chan_idx = 0;
    wlan_channel_bandwidth_enum_uint8 try_bandwidth;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_select_channel dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (dfs->dfs_info.custom_chanlist_bitmap) {
        try_bandwidth = WLAN_BAND_WIDTH_20M; // 强制20M带宽
    } else {
        try_bandwidth = hmac_vap->channel.en_bandwidth;
    }

hmac_dfs_select_random_channel_update:
    /* 获取可用信道位图 */
    hmac_dfs_get_chan_bitmap(hmac_vap, hmac_device, num_supp_chan, &chan_bitmap);

    /* 20MHz */
    if (try_bandwidth == WLAN_BAND_WIDTH_20M) {
        hmac_dfs_get_channel_width_20(chan_bitmap, num_supp_chan, available_chan_idx,
                                      &available_chan_cnt, &chan_idx);
    } else if (try_bandwidth < WLAN_BAND_WIDTH_80PLUSPLUS) {
        chan_bitmap &= HMAC_40MHZ_BITMAP_MASK;
        hmac_dfs_get_channel_width_80plusplus(chan_bitmap, num_supp_chan, available_chan_idx,
                                              &available_chan_cnt, &chan_idx);
    } else if (try_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS) {    /* 80MHz */
        /* 80MHz上检测到有问题,固定选择36-64的信道,带宽80M */
        available_chan_idx[available_chan_cnt++] = MAC_CHANNEL36;
    } else {    /* 160MHz */
        /* 160MHz上检测到有问题,固定选择主信道36,带宽80M */
        available_chan_idx[available_chan_cnt++] = MAC_CHANNEL36;
        try_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
    }

    /* 如果找不到可用信道，从管制域支持的信道中，选择最低可用信道，带宽20MHz */
    if (available_chan_cnt == 0) {
        oam_warning_log1(0, OAM_SF_DFS,
            "vap_id[%d] {hmac_dfs_select_random_channel::No availabe channel,NOL will be cleared!}",
            hmac_vap->vap_id);
        /* 此策略经产品讨论，当因为雷达检测导致已经无可用信道可跳时，将
           原来被标记为雷达的信道全部清掉，重新开始选择，然后CAC */
        if (hmac_dfs_nol_clear(&dfs->dfs_nol) != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_DFS,
                "vap_id[%d] {hmac_dfs_select_random_channel::hmac_dfs_nol_clear() Fail!}", hmac_vap->vap_id);

            return OAL_FAIL;
        }
        try_bandwidth = WLAN_BAND_WIDTH_20M;
        goto hmac_dfs_select_random_channel_update; // 需要重新获取信道位图
    }

    hmac_dfs_get_channel_idx(hmac_vap, available_chan_idx, available_chan_cnt, try_bandwidth, &chan_idx);

    *channel_idx = chan_idx;
    *bandwidth = try_bandwidth;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_select_random_channel
 功能描述  : 挑选一条信道(对)
 输入参数  : hmac_vap      : MAC VAP结构体指针
 输出参数  : new_channel  : 挑选出来的主信道号
             pen_new_bandwidth: 挑选出来的带宽模式
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
OAL_STATIC osal_u32 hmac_dfs_select_random_channel(hmac_vap_stru *hmac_vap,
    osal_u8 *new_channel, wlan_channel_bandwidth_enum_uint8 *pen_new_bandwidth)
{
    hmac_device_stru *hmac_device = OAL_PTR_NULL;
    osal_u8 chan_idx = 0;
    wlan_channel_bandwidth_enum_uint8 chan_bandwidth;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log2(0, OAM_SF_DFS,
            "vap_id[%d] {hmac_dfs_select_random_channel::hmac_device null, device_id = %d.}",
            hmac_vap->vap_id, hmac_vap->device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 挑选可用信道 */
    if (hmac_dfs_select_channel(hmac_vap, hmac_device, &chan_bandwidth, &chan_idx) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 返回选择到的信道号和带宽 */
    hmac_get_channel_num_from_idx_etc(hmac_vap->channel.band, chan_idx, new_channel);

    *pen_new_bandwidth = chan_bandwidth;

    wifi_printf("[DFS]select_channel::bandwidth = %d, channelnum = %d.\n", chan_bandwidth, *new_channel);
    oam_warning_log3(0, OAM_SF_CHAN,
        "vap_id[%d] {hmac_dfs_select_random_channel::[DFS]select_channel::bandwidth = %d, channelnum = %d!}",
        hmac_vap->vap_id, chan_bandwidth, *new_channel);

    return OAL_SUCC;
}

OAL_STATIC osal_u32 hmac_dfs_ap_wait_start_radar_custom_next_chnum(osal_u8 *new_channel,
    wlan_channel_bandwidth_enum_uint8 *new_bandwidth, hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    osal_u32 ul_ret;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_custom_next_chnum dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *new_channel = dfs->dfs_info.custom_next_chnum;
    *new_bandwidth = hmac_vap->channel.en_bandwidth;

    oam_warning_log3(0, OAM_SF_DFS,
        "vap_id[%d] {hmac_dfs_ap_wait_start_radar_custom_next::next ch(%d) and bw(%d) set by user.}",
        hmac_vap->vap_id, *new_channel, *new_bandwidth);

    if (hmac_dfs_is_channel_support_bw(hmac_vap, *new_channel, *new_bandwidth) != OAL_SUCC) {
        oam_warning_log3(0, OAM_SF_DFS,
            "vap_id[%d] {hmac_dfs_ap_wait_start_radar_custom_next::new channel(%d) not support current bw mode(%d).}",
            hmac_vap->vap_id, *new_channel, *new_bandwidth);
        // 如果应用设定的下一跳信道不支持当前的带宽模式，那么按照驱动的随机策略选择
        ul_ret = hmac_dfs_select_random_channel(hmac_vap, new_channel, new_bandwidth);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_DFS,
                "vap_id[%d] {hmac_dfs_ap_wait_start_radar_custom_next::hmac_dfs_select_random_channel failed.}",
                hmac_vap->vap_id);
            return ul_ret;
        }
    }
    dfs->dfs_info.custom_next_chnum = 0;
    dfs->dfs_info.next_ch_width_type = WLAN_BAND_WIDTH_BUTT;

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_ap_wait_start_radar_handler_etc
 功能描述  : AP在WAIT START状态下的雷达检测处理
 输入参数  : hmac_vap: HMAC VAP指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32 hmac_dfs_ap_wait_start_radar_handler_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru                     *hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    osal_u8                              new_channel   = 0;
    wlan_channel_bandwidth_enum_uint8    new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    osal_u32                             ul_ret;
    mac_dfs_core_stru *dfs = OSAL_NULL;

    if (osal_unlikely(hmac_device == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler_etc::hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler_etc dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 关闭 CAC 检测 */
    hmac_dfs_cac_stop_etc(hmac_device, hmac_vap, OSAL_FALSE);

    /* 标记主、次信道检测到雷达 */
    if (hmac_dfs_channel_mark_radar(hmac_device, hmac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 重新选择一条信道 */
    if (dfs->dfs_info.custom_next_chnum) {
        ul_ret = hmac_dfs_ap_wait_start_radar_custom_next_chnum(&new_channel, &new_bandwidth, hmac_device, hmac_vap);
        if (ul_ret != OAL_SUCC) {
            return ul_ret;
        }
    } else {
        ul_ret = hmac_dfs_select_random_channel(hmac_vap, &new_channel, &new_bandwidth);
        if (ul_ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_DFS,
                "vap_id[%d] {hmac_dfs_ap_wait_start_radar_handler_etc::select channel failed.}", hmac_vap->vap_id);
            return ul_ret;
        }
    }

    ul_ret = hmac_is_channel_num_valid_etc(hmac_vap->channel.band, new_channel);
    if ((ul_ret != OAL_SUCC) || (new_bandwidth >= WLAN_BAND_WIDTH_BUTT)) {
        oam_warning_log3(0, OAM_SF_DFS, "vap_id[%d] {the selected channel[%d] or bandwidth[%d] is invalid.}",
            hmac_vap->vap_id, new_channel, new_bandwidth);
        return ul_ret;
    }

    /* 只需要设置硬件寄存器一次，然后同步一下软件vap的配置即可 */
    hmac_chan_multi_select_channel_mac_etc(hmac_vap, new_channel, new_bandwidth);
    /* 判断是否需要进行 */
    if (hmac_dfs_need_for_cac(hmac_device, hmac_vap) == OAL_TRUE) {
        hmac_dfs_cac_start_etc(hmac_device, hmac_vap);
        return OAL_SUCC;
    }
    /* 切换到非雷达信道,把vap状态置为up,恢复发送队列 */
    hmac_cac_chan_ctrl_machw_tx(hmac_vap, DFS_CAC_CTRL_TX_OPEN_WITH_VAP_UP);
    /* 否则，直接启动BSS */
    return hmac_start_bss_in_available_channel_etc(hmac_vap);
}

osal_u32 hmac_dfs_switch_channel_for_radar_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    osal_u8                            new_channel   = 0;
    wlan_channel_bandwidth_enum_uint8  new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    osal_void *fhook = hmac_get_feature_fhook(HMAC_FHOOK_CSA_AP_IS_SWITCH);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_switch_channel_for_radar_etc dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 关闭 CAC 检测 ，但是不改变硬件tx状态 */
    hmac_dfs_cac_stop_etc(hmac_device, hmac_vap, OSAL_FALSE);

    /* 标记主、次信道检测到雷达 */
    if (hmac_dfs_channel_mark_radar(hmac_device, hmac_vap) != OAL_SUCC) {
        return OAL_FAIL;
    }

    /* 如果AP已经准备进行信道切换，则直接返回，不做任何处理 */
    if (fhook != OSAL_NULL && ((hmac_csa_ap_is_ch_switch_cb)fhook)(hmac_vap->vap_id) == OSAL_TRUE) {
        return OAL_SUCC;
    }

    if (dfs->dfs_info.custom_next_chnum) {
        new_channel = dfs->dfs_info.custom_next_chnum;
        if (dfs->dfs_info.next_ch_width_type != WLAN_BAND_WIDTH_BUTT) {
            new_bandwidth = dfs->dfs_info.next_ch_width_type;
        } else {
            oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_switch_channel_for_radar_etc::user not set next ch bw.}");
            new_bandwidth = hmac_vap->channel.en_bandwidth;
        }

        oam_warning_log3(0, OAM_SF_CHAN, "vap_id[%d] {hmac_dfs_switch_channel_for_radar_etc::next ch(%d) and bw(%d).}",
            hmac_vap->vap_id, new_channel, new_bandwidth);

        if (hmac_dfs_is_channel_support_bw(hmac_vap, new_channel, new_bandwidth) != OAL_SUCC) ||
            (hmac_is_dfs_with_dbac_check(mac_vap, mac_device, new_channel) == OSAL_TRUE)) {
            oam_warning_log3(0, OAM_SF_DFS,
                "vap_id[%d] {hmac_dfs_switch_channel_for_radar_etc::new channel(%d) not support current bw mode(%d).}",
                hmac_vap->vap_id, new_channel, new_bandwidth);
            // 如果应用设定的下一跳信道不支持当前的带宽模式，那么按照驱动的随机策略选择
            if (hmac_dfs_select_random_channel(hmac_vap, &new_channel, &new_bandwidth) != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_DFS,
                    "vap_id[%d] {hmac_dfs_switch_channel_for_radar_etc::select random channel fail}", hmac_vap->vap_id);
                return OAL_FAIL;
            }
        }
        dfs->dfs_info.custom_next_chnum = 0;
        dfs->dfs_info.next_ch_width_type = WLAN_BAND_WIDTH_BUTT;
    } else {
        if (hmac_dfs_select_random_channel(hmac_vap, &new_channel, &new_bandwidth) != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_DFS,
                "vap_id[%d] {hmac_dfs_switch_channel_for_radar_etc: select_random_channel fail}", hmac_vap->vap_id);
            return OAL_FAIL;
        }
    }

    hmac_chan_multi_switch_to_new_channel_etc(hmac_vap, new_channel, new_bandwidth);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_off_chan_cac_timeout_fn
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_dfs_off_chan_cac_timeout_fn(osal_void *p_arg)
{
    hmac_device_stru     *hmac_device;
    hmac_vap_stru        *hmac_vap;
    mac_dfs_core_stru    *dfs = OSAL_NULL;

    if (OAL_UNLIKELY(p_arg == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_timeout_fn::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_timeout_fn::hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_timeout_fn dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs->dfs_info.timer_cnt++;

    if (dfs->dfs_info.timer_cnt != dfs->dfs_info.timer_end_cnt) {
        /* 启动OFF CHAN CAC 定时器 */
        frw_create_timer_entry(&dfs->dfs_off_chan_cac_timer,
                               hmac_dfs_off_chan_cac_timeout_fn,
                               HMAC_DFS_ONE_MIN_IN_MS,
                               hmac_vap,
                               OAL_FALSE);
        return OAL_SUCC;
    }

    wifi_printf("[DFS]hmac_dfs_off_chan_cac_timeout_fn::off-channel cac end.\n");

    /* 关闭Off-channel CAC 信道驻留定时器 */
    frw_destroy_timer_entry(&(dfs->dfs_chan_dwell_timer));
    dfs->dfs_info.offchan_flag = 0;

    return OAL_SUCC;
}


/*****************************************************************************
 函 数 名  : hmac_dfs_ap_up_radar_handler_etc
 功能描述  : AP在UP状态下的雷达检测处理
 输入参数  : hmac_vap: HMAC VAP指针
 输出参数  : 无
 返 回 值  : OAL_SUCC或其它错误码
*****************************************************************************/
osal_u32 hmac_dfs_ap_up_radar_handler_etc(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru                     *hmac_device;
    osal_u8                            offchan_flag;
    mac_dfs_core_stru *dfs = OSAL_NULL;

    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_ap_wait_start_radar_handler_etc::hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_error_log1(0, OAM_SF_DFS,
            "vap_id[%d] {hmac_dfs_ap_up_radar_handler_etc::hmac_device is null.}", hmac_vap->vap_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_set_dfs_enable dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    offchan_flag = dfs->dfs_info.offchan_flag;
    /* off-chan cac 期间 */
    if (offchan_flag & BIT1) {
        /* home channel检测到radar */
        if (!(offchan_flag & BIT0)) {
            oam_warning_log2(0, OAM_SF_DFS, "vap_id[%d] {[DFS]radar detected in channel %d.}",
                hmac_vap->vap_id, hmac_vap->channel.chan_number);

            /* 关闭Off-channel CAC 定时器 */
            frw_destroy_timer_entry(&(dfs->dfs_off_chan_cac_timer));

            hmac_dfs_off_chan_cac_timeout_fn(hmac_vap);

            return hmac_dfs_switch_channel_for_radar_etc(hmac_device, hmac_vap);
        } else {    /* off channel检测到radar */
            oam_warning_log2(0, OAM_SF_DFS, "vap_id[%d] {[DFS]radar detected in channel %d.}",
                hmac_vap->vap_id, dfs->dfs_info.offchan_num);
            /* 标记主、次信道检测到雷达 */
            hmac_dfs_channel_mark_radar(hmac_device, hmac_vap);
            return OAL_SUCC;
        }
    } else {    /* in service monitor期间 */
        return hmac_dfs_switch_channel_for_radar_etc(hmac_device, hmac_vap);
    }
}

/*****************************************************************************
 函 数 名  : hmac_dfs_off_chan_cac_opern_ch_dwell_timeout
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32  hmac_dfs_off_chan_cac_opern_ch_dwell_timeout(osal_void *p_arg)
{
    hmac_device_stru  *hmac_device = OSAL_NULL;
    hmac_vap_stru     *hmac_vap = OSAL_NULL;
    mac_dfs_core_stru  *dfs = OSAL_NULL;

    if (OAL_UNLIKELY(p_arg == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_opern_ch_dwell_timeout::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_opern_ch_dwell_timeout::hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_opern_ch_dwell_timeout dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 当前信道为offchan 标志 */
    dfs->dfs_info.offchan_flag = (BIT0 | BIT1);
    /* 启动Off-channel信道上检测时长定时器 */
    frw_create_timer_entry(&dfs->dfs_chan_dwell_timer,
                           hmac_dfs_off_chan_cac_off_ch_dwell_timeout,
                           dfs->dfs_info.dfs_off_chan_cac_off_chan_dwell_time,
                           hmac_vap,
                           OAL_FALSE);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_off_chan_cac_off_ch_dwell_timeout
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
OAL_STATIC osal_u32 hmac_dfs_off_chan_cac_off_ch_dwell_timeout(osal_void *p_arg)
{
    hmac_device_stru   *hmac_device = OSAL_NULL;
    hmac_vap_stru      *hmac_vap = OSAL_NULL;
    mac_dfs_core_stru  *dfs = OSAL_NULL;

    if (OAL_UNLIKELY(p_arg == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_off_ch_dwell_timeout::p_arg is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_vap = (hmac_vap_stru *)p_arg;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_off_ch_dwell_timeout::hmac_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_off_ch_dwell_timeout dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 当前信道为homechan标志 */
    dfs->dfs_info.offchan_flag = BIT1;
    /* 启动在工作信道上的工作时长定时器 */
    frw_create_timer_entry(&dfs->dfs_chan_dwell_timer,
                           hmac_dfs_off_chan_cac_opern_ch_dwell_timeout,
                           dfs->dfs_info.dfs_off_chan_cac_opern_chan_dwell_time,
                           hmac_vap,
                           OAL_FALSE);

    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_off_chan_cac_start_etc
 输入参数  : 无
 输出参数  : 无
*****************************************************************************/
osal_void hmac_dfs_off_chan_cac_start_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    osal_u32 scan_time;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_off_chan_cac_start_etc dfs ptr is null!}");
        return;
    }
    /* 如果已经启动OFF-CHAN-CAC 定时器, 直接返回 */
    if (dfs->dfs_off_chan_cac_timer.is_enabled == OAL_TRUE) {
        oam_info_log0(0, OAM_SF_DFS,
            "{hmac_dfs_off_chan_cac_start_etc::off-chan-cac is already started by another vap.}");
        return;
    }

    /* 设置 Off-Channel CAC 检测时间 */
    scan_time = hmac_chan_get_off_chan_cac_time(hmac_device, hmac_vap);

    dfs->dfs_info.timer_cnt     = 0;
    dfs->dfs_info.timer_end_cnt = (osal_u8)(scan_time / HMAC_DFS_ONE_MIN_IN_MS);

    /* 启动 CAC 定时器 */
    frw_create_timer_entry(&dfs->dfs_off_chan_cac_timer,
                           hmac_dfs_off_chan_cac_timeout_fn,
                           HMAC_DFS_ONE_MIN_IN_MS,
                           hmac_vap,
                           OAL_FALSE);

    oam_info_log2(0, OAM_SF_DFS,
        "{[DFS]hmac_dfs_off_chan_cac_start_etc, OFF-CHAN-CAC start, channel number:%d, scan time = %d.}",
        dfs->dfs_info.offchan_num, scan_time);

    (osal_void)hmac_dfs_off_chan_cac_opern_ch_dwell_timeout(hmac_vap);
}
/*****************************************************************************
 函 数 名  : hmac_dfs_try_cac_etc
 功能描述  : 尝试进行CAC
 输入参数  : hmac_devpst_hmac_devicemac_device
             hmac_vap_stru *hmac_vap
 输出参数  : 无
 返 回 值  : oal_bool_enum_uint8
*****************************************************************************/
oal_bool_enum_uint8 hmac_dfs_try_cac_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    mac_channel_stru *channel;
    mac_dfs_core_stru *dfs = OSAL_NULL;

    if (OAL_UNLIKELY(hmac_device == OAL_PTR_NULL || hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_dfs_try_cac_etc::device_base_info null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_try_cac_etc dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果已经启动 CAC 定时器，则直接返回 */
    if (dfs->dfs_cac_timer.is_registerd == OAL_TRUE) {
        oam_warning_log1(0, OAM_SF_ACS, "vap_id[%d] cac started\n", hmac_vap->vap_id);
        return OAL_TRUE;
    }

    // 至此，vap的信道结构为最终的信道,但是可能并未设置到硬件
    channel = &hmac_vap->channel;
    if (is_legacy_vap(hmac_vap)) {
        /* 如果需要进行 CAC 检测，这里启动定时器，直到CAC结束后才启动BSS(VAP UP) */
        if (hmac_dfs_need_for_cac(hmac_device, hmac_vap) == OAL_TRUE) {
            // 此时设置硬件信道
            hmac_chan_sync_etc(hmac_vap, channel->chan_number,
                channel->en_bandwidth, OAL_TRUE);
            /* 把vap状态置为pause, 抛事件给dmac关发送队列, 待cac超时后把vap状态置为up,恢复发送队列 */
            hmac_cac_chan_ctrl_machw_tx(hmac_vap, DFS_CAC_CTRL_TX_CLOSE_WITH_VAP_PAUSE);
            hmac_dfs_cac_start_etc(hmac_device, hmac_vap);
            dfs->dfs_info.dfs_switch &= ~BIT1;

            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}
/*****************************************************************************
 函 数 名  : hmac_dfs_init_scan_hook_etc
 功能描述  : 初始信道检查模块钩子函数，进行CAC检查
 输入参数  : hmac_scan_runtime_record_stru   *scan_record
             hmac_device_stru                 *dev
 输出参数  : 无
 返 回 值  : osal_u32
*****************************************************************************/
osal_u32 hmac_dfs_init_scan_hook_etc(hmac_scan_record_stru *scan_record,
    hmac_device_stru *hmac_device)
{
    osal_u8       vap_id;
    hmac_vap_stru    *hmac_vap = NULL;

    oam_warning_log0(0, OAM_SF_ACS, "dfs init scan hook run\n");

    for (vap_id = 0; vap_id < hmac_device->vap_num; vap_id++) {
        hmac_vap = mac_res_get_hmac_vap(hmac_device->vap_id[vap_id]);
        if ((hmac_vap != OAL_PTR_NULL) && (hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP)) {
            /* 只要当前device上已经开始进行CAC过程，那么同device的其他VAP不再进行 */
            /* CAC完成之后会统一设置信道并启动VAP */
            /* DBAC场景下依然可以满足 */
            if (hmac_dfs_try_cac_etc(hmac_device, hmac_vap) == OAL_TRUE) {
                oam_warning_log0(0, OAM_SF_ACS, "cac started\n");
                return OAL_SUCC;
            }
        }
    }

    oam_warning_log0(0, OAM_SF_ACS, "cac not need\n");
    return OAL_FAIL;
}

osal_void test_dfs(osal_u8 vap_id)
{
    hmac_vap_stru *hmac_vap;
    frw_msg radar_det_msg = {0};
    hal_radar_det_event_stru radar_det_info;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (OAL_UNLIKELY(hmac_vap == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{test_dfs::hmac_vap null.}");
        return;
    }

    (osal_void)memset_s(&radar_det_info, sizeof(hal_radar_det_event_stru), 0, sizeof(hal_radar_det_event_stru));

    /* 读取雷达控制寄存器 */
    radar_det_info.radar_type = 1;
    radar_det_msg.data = (osal_u8 *)(&radar_det_info);
    radar_det_msg.data_len = sizeof(hal_radar_det_event_stru);

    hmac_dfs_radar_detect_event(hmac_vap, &radar_det_msg);
}

osal_void hmac_dfs_radar_detect_log(const hal_radar_det_event_stru *radar_info,
    osal_u8 vap_id)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_log dfs ptr is null!}");
        return;
    }

    oam_warning_log3(0, OAM_SF_DFS, "vap_id[%d] {hmac_dfs_radar_detect_log::radar detected %u, radar type = %d!",
        vap_id, dfs->dfs_cnt, radar_info->radar_type);
    oam_warning_log4(0, OAM_SF_DFS,
        "{hmac_dfs_radar_detect_log::[DFS]radar_freq_offset=%d,radar_bw=%d,band=%d,channel_num=%d,working_bw=%d!",
        radar_info->radar_freq_offset, radar_info->radar_bw, radar_info->band, radar_info->working_bw);

    dfs->dfs_cnt++;
}

osal_s32 hmac_dfs_radar_detect_dispatch_event(hmac_vap_stru *hmac_vap, frw_msg *msg_info)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hal_radar_det_event_stru *radar_det_info = (hal_radar_det_event_stru *)msg_info->data;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    hal_to_dmac_device_stru *hal_device = OSAL_NULL;
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_dispatch_event dfs ptr is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_dfs_radar_detect_event::vap id [%d],hal_device null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 冗余检测，防止硬件误报，非雷达信道不应该产生雷达中断 且产生雷达中断的信道号与当前信道号不一致时也不上报 */
    if (radar_det_info->flag != RADAR_INFO_FLAG_DUMMY) {
        /* 当前radar info 没有保存带宽类型，暂时从VAP信息取用 */
        if (hmac_is_cover_dfs_channel(radar_det_info->band, hmac_vap->channel.en_bandwidth,
            radar_det_info->channel_num) == OSAL_FALSE) {
            oam_warning_log1(0, OAM_SF_DFS,
                "{hmac_dfs_radar_detect_event::[DFS]non radar channel[%d] should not generate radar event.",
                radar_det_info->channel_num);
            return OAL_SUCC;
        }

        if (radar_det_info->channel_num != hal_device->current_chan_number) {
            oam_error_log2(0, OAM_SF_DFS,
                "{hmac_dfs_radar_detect_event::radar irq generate by channel[%d] which is not current channel[%d].",
                radar_det_info->channel_num, hmac_device->max_channel);

            return OAL_SUCC;
        }

        /* 产生雷达中断的信道号与处于up或者wait start状态 vap的home信道号不一致时也不上报 */
        if (radar_det_info->channel_num != hmac_vap->channel.chan_number) {
            oam_warning_log2(0, OAM_SF_DFS,
                "{hmac_dfs_radar_detect_event::radar irq generate by channel[%d] which is not home channel[%d].",
                radar_det_info->channel_num, hmac_vap->channel.chan_number);
            return OAL_SUCC;
        }
    }

    if (mac_dfs_get_debug_level(hmac_device) & 0x1) {
        hmac_dfs_radar_detect_log(radar_det_info, hmac_vap->vap_id);
        /* 记录雷达检测时间戳 */
        dfs->dfs_info.radar_detected_timestamp = (osal_u32)osal_get_time_stamp_ms();
    }

    if (mac_dfs_get_debug_level(hmac_device) & 0x2) {
        hmac_dfs_radar_detect_log(radar_det_info, hmac_vap->vap_id);
        /* 使能雷达检测，不使能切换信道 */
        return OAL_SUCC;
    }

    /* 关闭硬件雷达检测 */
    hal_enable_radar_det(OSAL_FALSE);
    hmac_dfs_radar_detect_event_etc(hmac_vap);
    return OAL_SUCC;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_radar_detect_event
 功能描述  : DMAC 雷达中断处理入口函数
*****************************************************************************/
osal_s32 hmac_dfs_radar_detect_event(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_device_stru *hmac_device = hmac_res_get_mac_dev_etc(0);
    hmac_vap_stru *radar_dmac_vap = OSAL_NULL;
    oal_bool_enum_uint8 radar_report_ap  = OSAL_FALSE;
    osal_u8 vap_idx;

    unref_param(hmac_vap);
    if (g_dmac_chan_mgmt_rom_cb.hmac_radar_filter) {
        if (g_dmac_chan_mgmt_rom_cb.hmac_radar_filter(msg) == OAL_CB_RETURN) {
            return OAL_SUCC;
        }
    }

    /* 如果雷达检测没使能，则直接返回 */
    if (mac_dfs_get_dfs_enable(hmac_device) == OSAL_FALSE) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_event::[DFS]dfs is disabled.");
        return OAL_SUCC;
    }

    /* 找一个need report radar AP */
    for (vap_idx = 0; vap_idx < hmac_device->vap_num; vap_idx++) {
        radar_dmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_device->vap_id[vap_idx]);
        if ((radar_dmac_vap != OSAL_NULL) && (radar_dmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) &&
            (radar_dmac_vap->vap_state == MAC_VAP_STATE_UP || radar_dmac_vap->vap_state == MAC_VAP_STATE_PAUSE ||
            radar_dmac_vap->vap_state == MAC_VAP_STATE_AP_WAIT_START)) {
            radar_report_ap = OSAL_TRUE;
            break;
        }
    }

    if ((radar_report_ap == OSAL_FALSE) || (radar_dmac_vap == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_event_patch::no running ap,radar event ignor.");
        return OSAL_TRUE;
    }

    if (mac_vap_get_dfs_enable(radar_dmac_vap) != OSAL_TRUE) {
        oam_warning_log1(0, OAM_SF_DFS, "vap_id[%d] {hmac_dfs_radar_detect_event::vap [DFS] DISABLE.",
            radar_dmac_vap->vap_id);
        return OAL_SUCC;
    }

    return hmac_dfs_radar_detect_dispatch_event(radar_dmac_vap, msg);
}

/*****************************************************************************
 功能描述 : 获取当前vap是否支持dfs功能
*****************************************************************************/
oal_bool_enum_uint8 mac_vap_get_dfs_enable(hmac_vap_stru *hmac_vap)
{
    hmac_device_stru *hmac_device;

    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OAL_PTR_NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{mac_vap_get_dfs_enable::hmac_res_get_mac_dev_etc[%d] Error.}",
            hmac_vap->device_id);
        return OAL_FALSE;
    }

    /* 先判断vap是否支持，再判断mac_device的dfs开关是否打开 */
    if (hmac_vap->radar_detector_is_supp == OSAL_TRUE) {
        return mac_dfs_get_dfs_enable(hmac_device);
    }

    return OAL_FALSE;
}

osal_void mac_dfs_set_cac_enable(hmac_device_stru *hmac_device, oal_bool_enum_uint8 val)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_set_cac_enable dfs ptr is null!}");
        return;
    }
    dfs->dfs_info.cac_switch = val;
}

void mac_dfs_set_offchan_cac_enable(hmac_device_stru *hmac_device, oal_bool_enum_uint8 val)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_set_offchan_cac_enable dfs ptr is null!}");
        return;
    }
    dfs->dfs_info.offchan_cac_switch = val;
}

/*****************************************************************************
 功能描述 : ETSI标准雷达需要off-channel cac检测
*****************************************************************************/
oal_bool_enum_uint8 mac_dfs_get_offchan_cac_enable(hmac_device_stru *hmac_device)
{
    mac_regdomain_info_stru *rd_info;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_set_dfs_enable dfs ptr is null!}");
        return OAL_FALSE;
    }

    hmac_get_regdomain_info_etc(&rd_info);
    if (rd_info->dfs_domain == MAC_DFS_DOMAIN_ETSI) {
        return dfs->dfs_info.offchan_cac_switch;
    }

    return OAL_FALSE;
}

oal_bool_enum_uint8 mac_dfs_get_cac_enable(hmac_device_stru *hmac_device)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_get_cac_enable dfs ptr is null!}");
        return OAL_FALSE;
    }
    return dfs->dfs_info.cac_switch;
}

osal_void mac_dfs_set_dfs_enable(hmac_device_stru *hmac_device, oal_bool_enum_uint8 val)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_warning_log0(0, OAM_SF_DFS, "{mac_dfs_set_dfs_enable dfs ptr is null!}");
        return;
    }
    dfs->dfs_info.dfs_switch = val;

    /* 如果 软件雷达检测使能 关闭，则关闭CAC检测 */
    if (val == OSAL_FALSE) {
        dfs->dfs_info.cac_switch = OSAL_FALSE;
    }
}

oal_bool_enum_uint8 mac_dfs_get_dfs_enable(hmac_device_stru *hmac_device)
{
    mac_dfs_core_stru *dfs = OSAL_NULL;

    if (hmac_device == OSAL_NULL) {
        return OSAL_FALSE;
    }

    dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);
    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_get_dfs_enable dfs ptr is null!}");
        return OSAL_FALSE;
    }

    if (hmac_device->max_band == WLAN_BAND_5G) {
        return dfs->dfs_info.dfs_switch;
    }

    return OSAL_FALSE;
}

osal_void mac_dfs_set_debug_level(hmac_device_stru *hmac_device, osal_u8 debug_lev)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_set_debug_level dfs ptr is null!}");
        return;
    }
    dfs->dfs_info.debug_level = debug_lev;
}

osal_u8 mac_dfs_get_debug_level(hmac_device_stru *hmac_device)
{
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{mac_dfs_get_debug_level dfs ptr is null!}");
        return OAL_FALSE;
    }
    return dfs->dfs_info.debug_level;
}

osal_u32  hmac_mgmt_scan_dfs_timeout(osal_void *arg)
{
    unref_param(arg);
    /* 使能雷达检测 */
    hal_enable_radar_det(1);

    return OAL_SUCC;
}

osal_void hmac_dfs_radar_detect_check(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    const hmac_vap_stru *hmac_vap)
{
    oal_bool_enum_uint8             enable_dfs;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device,
        WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_DFS, "{hmac_dfs_radar_detect_check dfs ptr is null!}");
        return;
    }

    /* 使能雷达检测 */
    hal_enable_radar_det(1);

    enable_dfs = hmac_is_cover_dfs_channel(hmac_device->max_band, hmac_vap->channel.en_bandwidth,
        hmac_vap->channel.chan_number);
    if (dfs->dfs_timeout != 0 && enable_dfs == OSAL_TRUE) {
        hal_enable_radar_det(0);
        /* 启动定时器 */
        frw_create_timer_entry(&dfs->dfs_radar_timer, hmac_mgmt_scan_dfs_timeout, dfs->dfs_timeout, hal_device,
            OSAL_FALSE);
    } else {
        hal_enable_radar_det(enable_dfs);
    }
}
/*****************************************************************************
 函 数 名  : hmac_cac_chan_ctrl_machw_tx_event_process
 功能描述  : cac信道控制事件处理函数
*****************************************************************************/
osal_s32 hmac_cac_chan_ctrl_machw_tx_event_process(hmac_vap_stru *hmac_vap, osal_u8 cac_machw_en)
{
    hmac_vap_stru *mac_vap = OSAL_NULL;
    hal_to_dmac_vap_stru *hal_vap = OSAL_NULL;

    mac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(hmac_vap->vap_id);
    if (mac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_TX, "{hmac_cac_chan_ctrl_machw_tx_event_process::mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (cac_machw_en == DFS_CAC_CTRL_TX_CLOSE_WITH_VAP_PAUSE) {
        /* 暂停vap的发送 */
        hmac_vap_pause_tx(hmac_vap);
        /* 禁止硬件的全部发送 */
        hmac_chan_disable_machw_tx(hmac_vap);
    } else if (cac_machw_en == DFS_CAC_CTRL_TX_OPEN_WITH_VAP_UP) {
        hal_vap = hmac_vap->hal_vap;
        if (hal_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_cac_chan_ctrl_machw_tx_event_process::hal_vap null}", hmac_vap->vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* dmac vap状态pause->up */
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);
        /* 在新信道上恢复Beacon帧的发送 */
        hal_vap_beacon_resume(hal_vap);
        /* 恢复硬件发送 */
        hmac_chan_enable_machw_tx(hmac_vap);
    } else if (cac_machw_en == DFS_CAC_CTRL_TX_OPEN) {
        hal_vap = hmac_vap->hal_vap;
        if (hal_vap == OSAL_NULL) {
            oam_error_log1(0, OAM_SF_CFG,
                "vap_id[%d] {hmac_cac_chan_ctrl_machw_tx_event_process::hal_vap is null}", hmac_vap->vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 在新信道上恢复Beacon帧的发送 */
        hal_vap_beacon_resume(hal_vap);
        /* 恢复硬件发送 */
        hmac_chan_enable_machw_tx(hmac_vap);
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置国家码中dfs信息
 输入参数  : 无
 返 回 值: osal_u32
*****************************************************************************/
osal_s32 hmac_config_set_country_for_dfs(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    mac_dfs_domain_enum_uint8 dfs_domain = *(msg->data);
    hal_to_dmac_device_stru *hal_device = hmac_vap->hal_device;
    if (hal_device == OSAL_NULL) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{hmac_config_set_country_for_dfs::vap id [%d],hal_device null}", hmac_vap->vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hal_radar_config_reg(dfs_domain);

#if defined(_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined(_PRE_WLAN_FEATURE_DFS_ENABLE)
    hal_device->dfs_radar_filter.log_switch = 0;
    hal_device->dfs_radar_filter.radar_type = dfs_domain;
    hal_device->dfs_radar_filter.chirp_cnt = 0;

    hal_device->dfs_radar_filter.chirp_cnt_total = 0;
    hal_device->dfs_radar_filter.chirp_timeout = OSAL_FALSE;
    hal_device->dfs_radar_filter.dfs_normal_pulse_det.timeout = OSAL_FALSE;

    hal_device->dfs_radar_filter.crazy_report_cnt = OSAL_FALSE;
    hal_device->dfs_radar_filter.chirp_cnt_for_crazy_report_det = 0;

    /* 超时时间设置为2*60，即120s，转化单位要乘以1000 */
    hal_device->dfs_radar_filter.timer_disable_chirp_det.timeout = 2 * 60 * 1000;
    /* 超时时间设置为20s，转化单位要乘以1000 */
    hal_device->dfs_radar_filter.timer_crazy_report_det.timeout = 20 * 1000;

    /* 使能normal pulse定时器 */
    hal_device->dfs_radar_filter.dfs_normal_pulse_det.is_enabled = OSAL_TRUE;

    /* 使能chirp crazy report定时器 */
    hal_device->dfs_radar_filter.crazy_report_is_enabled = OSAL_TRUE;

    /* 误报过滤条件 */
    hal_device->dfs_radar_filter.dfs_pulse_check_filter.fcc_chirp_pow_diff = OSAL_TRUE;
    hal_device->dfs_radar_filter.dfs_pulse_check_filter.fcc_chirp_duration_diff = OSAL_TRUE;
    hal_device->dfs_radar_filter.dfs_pulse_check_filter.fcc_type4_duration_diff = OSAL_TRUE;
    hal_device->dfs_radar_filter.dfs_pulse_check_filter.fcc_chirp_eq_duration_num = OSAL_TRUE;
#endif

    hal_device->dfs_radar_filter.last_burst_timestamp = 0;
    hal_device->dfs_radar_filter.chirp_wow_wake_flag = 0;
    hal_device->dfs_radar_filter.last_burst_timestamp_for_chirp = 0;
    hal_device->dfs_radar_filter.chirp_enable = 1;
    /* 时间阈值设置为100 */
    hal_device->dfs_radar_filter.time_threshold = 100;

    if (dfs_domain == MAC_DFS_DOMAIN_ETSI) {
        hal_device->dfs_radar_filter.chirp_cnt_threshold = ETSI_RADAR_CHIRP_CNT;
        /* chirp时间阈值设置为100 */
        hal_device->dfs_radar_filter.chirp_time_threshold = 100;
        /* 时间阈值设置为100 */
        hal_device->dfs_radar_filter.time_threshold = 100;
    } else if (dfs_domain == MAC_DFS_DOMAIN_FCC) {
        hal_device->dfs_radar_filter.chirp_cnt_threshold = FCC_RADAR_CHIRP_CNT;
        /* chirp时间阈值设置为12000 */
        hal_device->dfs_radar_filter.chirp_time_threshold = 12000;
        /* 时间阈值设置为200 */
        hal_device->dfs_radar_filter.time_threshold = 200;
    } else if (dfs_domain == MAC_DFS_DOMAIN_MKK) {
        hal_device->dfs_radar_filter.chirp_cnt_threshold = MKK_RADAR_CHIRP_CNT;
        /* chirp时间阈值设置为12000 */
        hal_device->dfs_radar_filter.chirp_time_threshold = 12000;
        /* 时间阈值设置为200 */
        hal_device->dfs_radar_filter.time_threshold = 200;
    }

    oam_warning_log0(0, OAM_SF_DFS, "{hmac_config_set_country_for_dfs::set radar filter params.}");

    return OAL_SUCC;
}

osal_void hmac_config_cali_dfs_handle(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device)
{
    unref_param(hal_device);
    unref_param(hmac_vap);
    unref_param(hmac_device);

    /* 使能去使能雷达检测 */
    if ((hmac_vap->vap_mode == WLAN_VAP_MODE_BSS_AP) && (mac_vap_get_dfs_enable(hmac_vap) == OSAL_TRUE)) {
        oal_bool_enum_uint8 enable_dfs;
        enable_dfs = hmac_is_cover_dfs_channel(hmac_device->max_band,
                                               hmac_device->max_bandwidth,
                                               hmac_device->max_channel);
        hal_enable_radar_det(enable_dfs);
#if defined(_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined(_PRE_WLAN_FEATURE_DFS_ENABLE)
        if (hal_device->dfs_radar_filter.dfs_normal_pulse_det.timer.is_enabled == OSAL_TRUE) {
            /* 取消定时器 */
            frw_destroy_timer_entry(&(hal_device->dfs_radar_filter.dfs_normal_pulse_det.timer));
            hal_device->dfs_radar_filter.dfs_normal_pulse_det.timer_start = OSAL_FALSE;
        }
#endif
    }
    return;
}

/*****************************************************************************
 函 数 名  : hmac_cac_chan_disable_machw_tx
 功能描述  : cac 发送队列开关控制
 输入参数  : hmac_vap: MAC VAP结构体指针
 输入参数  : cac_machw_en: 发送队列控制开关,0->禁止硬件发送(Beacon、数据帧、ACK、RTS),1->恢复硬件发送(Beacon、数据帧、ACK、RTS)
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
osal_void  hmac_cac_chan_ctrl_machw_tx(hmac_vap_stru *hmac_vap, osal_u8 cac_machw_en)
{
    if (cac_machw_en == DFS_CAC_CTRL_TX_CLOSE_WITH_VAP_PAUSE) {
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_PAUSE);
    } else if (cac_machw_en == DFS_CAC_CTRL_TX_OPEN_WITH_VAP_UP) {
        hmac_vap_state_change_etc(hmac_vap, MAC_VAP_STATE_UP);
    }
    hmac_cac_chan_ctrl_machw_tx_event_process(hmac_vap, cac_machw_en);
}

/*****************************************************************************
 函 数 名  : hmac_is_dfs_with_dbac_check
 功能描述  : 判断该信道是否是雷达信道且与其他vap是否存在dbac
 输入参数  : mac_device: MAC DEVICE指针
             hmac_vap   : MAC VAP指针
             channel：信道
 输出参数  : 无
 返 回 值  : 如果是雷达信道且存在dbac,返回true，否则false
*****************************************************************************/
osal_bool hmac_is_dfs_with_dbac_check(const hmac_vap_stru *hmac_vap, const hmac_device_stru *mac_device,
    const osal_u8 channel)
{
    hmac_vap_stru     *index_mac_vap = OSAL_NULL;
    osal_u8           vap_idx;

    if (mac_is_dfs_channel(MAC_RC_START_FREQ_5, channel) != OSAL_TRUE) {
        return OSAL_FALSE;
    }

    for (vap_idx = 0; vap_idx < mac_device->vap_num; vap_idx++) {
        index_mac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(mac_device->vap_id[vap_idx]);
        if ((index_mac_vap == OSAL_NULL) || (hmac_vap->vap_id == index_mac_vap->vap_id) ||
            (index_mac_vap->vap_state != MAC_VAP_STATE_UP &&
            index_mac_vap->vap_state != MAC_VAP_STATE_PAUSE)) {
            continue;
        }

        if (channel != index_mac_vap->channel.chan_number) {
            oam_warning_log0(0, OAM_SF_CFG,
                "{hmac_is_dfs_with_dbac_check::The channel is different from other VAP channels.}");
            return OSAL_TRUE;
        }
    }
    return OSAL_FALSE;
}

/*****************************************************************************
 函 数 名  : hmac_dfs_need_for_cac
 功能描述  : 判断是否进行CAC检测
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
oal_bool_enum_uint8  hmac_dfs_need_for_cac(hmac_device_stru *hmac_device,
    hmac_vap_stru *hmac_vap)
{
    osal_u8               idx, ch;
    osal_u32              ul_ret;
    mac_channel_list_stru   chan_info;
    osal_u8               dfs_ch_cnt = 0;
    mac_channel_stru       *channel;
    mac_dfs_core_stru *dfs = (mac_dfs_core_stru *)hmac_device_get_feature_ptr(hmac_device, WLAN_FEATURE_INDEX_DFS);

    if (osal_unlikely(dfs == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_dfs_need_for_cac dfs ptr is null!}");
        return OAL_FALSE;
    }

    /* dfs使能位 */
    if (OAL_FALSE == mac_vap_get_dfs_enable(hmac_vap)) {
        oam_warning_log0(0, OAM_SF_DFS, "dfs not enable\n");
        return OAL_FALSE;
    }

    channel = &hmac_vap->channel;

    if (channel->band != WLAN_BAND_5G) {
        oam_warning_log0(0, OAM_SF_DFS, "dfs no need on 2G channel\n");
        return OAL_FALSE;
    }

    /* 获取信道索引 */
    ul_ret = hmac_get_channel_idx_from_num_etc(MAC_RC_START_FREQ_5, channel->chan_number, &idx);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret)) {
        oam_error_log2(0, OAM_SF_DFS, "get ch failed, band=%d ch=%d\n", MAC_RC_START_FREQ_5,
            channel->chan_number);
        return OAL_FALSE;
    }

    hmac_get_ext_chan_info(idx, channel->en_bandwidth, &chan_info);

    for (ch = 0; ch < chan_info.channel; ch++) {
        /* 信道状态 */
        if ((MAC_CHAN_DFS_REQUIRED !=
            hmac_device->ap_channel_list[chan_info.channels[ch].idx].ch_status) &&
            (MAC_CHAN_BLOCK_DUE_TO_RADAR !=
            hmac_device->ap_channel_list[chan_info.channels[ch].idx].ch_status)) {
            dfs_ch_cnt++;
        }
    }

    if (dfs_ch_cnt == ch) {
        oam_warning_log2(0, OAM_SF_DFS, "all of the ch(pri_ch=%d,bw=%d) are not dfs channel,not need cac\n",
            channel->chan_number, channel->en_bandwidth);
        return OAL_FALSE;
    }

    /* CAC使能位 */
    if (OAL_FALSE == mac_dfs_get_cac_enable(hmac_device)) {
        oam_warning_log0(0, OAM_SF_DFS, "cac not enabled");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif   /* end of _PRE_WLAN_FEATURE_DFS */

