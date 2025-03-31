/*
 * Copyright (c) CompanyNameMagicTag 2014-2023. All rights reserved.
 * 文 件 名   : hmac_dfs.h
 * 生成日期   : 2014年10月16日
 * 功能描述   : hmac_dfs.c 的头文件
 */

#ifndef __HMAC_DFS_H__
#define __HMAC_DFS_H__

#ifdef _PRE_WLAN_FEATURE_DFS

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "mac_device_ext.h"
#include "hmac_vap.h"
#include "hmac_scan.h"
#include "hmac_feature_main.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFS_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_DFS_ONE_SEC_IN_MS    1000
#define HMAC_DFS_ONE_MIN_IN_MS    (60 * HMAC_DFS_ONE_SEC_IN_MS)

/* CAC检测时长，5600MHz ~ 5650MHz频段外，默认60秒 */
#define HMAC_DFS_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS           HMAC_DFS_ONE_MIN_IN_MS

/* CAC检测时长，5600MHz ~ 5650MHz频段内，默认10分钟 */
#define HMAC_DFS_CAC_IN_5600_TO_5650_MHZ_TIME_MS              (10 * HMAC_DFS_ONE_MIN_IN_MS)

/* Off-Channel CAC检测时长，5600MHz ~ 5650MHz频段外，默认6分钟 */
#define HMAC_DFS_OFF_CH_CAC_OUTOF_5600_TO_5650_MHZ_TIME_MS    (6 * HMAC_DFS_ONE_MIN_IN_MS)

/* Off-Channel CAC检测时长，5600MHz ~ 5650MHz频段内，默认60分钟 */
#define HMAC_DFS_OFF_CH_CAC_IN_5600_TO_5650_MHZ_TIME_MS       (60 * HMAC_DFS_ONE_MIN_IN_MS)

/* Non-Occupancy Period时长，默认30分钟 */
#define HMAC_DFS_NON_OCCUPANCY_PERIOD_TIME_MS                 (30 * HMAC_DFS_ONE_MIN_IN_MS)

/* Off-Channel CAC在工作信道上的驻留时长 */
#define HMAC_DFS_OFF_CHAN_CAC_PERIOD_TIME_MS                  15

/* Off-channel CAC在Off-channel信道上的驻留时长 */
#define HMAC_DFS_OFF_CHAN_CAC_DWELL_TIME_MS                   30

#if defined (_PRE_WLAN_FEATURE_DFS_OPTIMIZE) || defined (_PRE_WLAN_FEATURE_DFS_ENABLE)
#define ETSI_RADAR_CHIRP_CNT       1        /* ETSI chirp雷达中断阈值 */
#define FCC_RADAR_CHIRP_CNT        3        /* FCC chirp雷达中断阈值 */
#define MKK_RADAR_CHIRP_CNT        3        /* MKK chirp雷达中断阈值 */
#else
#define ETSI_RADAR_CHIRP_CNT       3        /* ETSI chirp雷达中断阈值 */
#define FCC_RADAR_CHIRP_CNT        4        /* FCC chirp雷达中断阈值 */
#define MKK_RADAR_CHIRP_CNT        4        /* MKK chirp雷达中断阈值 */
#endif

#define DFS_CAC_CTRL_TX_CLOSE_WITH_VAP_PAUSE 0
#define DFS_CAC_CTRL_TX_OPEN_WITH_VAP_UP 1
#define DFS_CAC_CTRL_TX_OPEN 2
#define DFS_CAC_CTRL_TX_CLOSE 3
/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef oal_bool_enum_uint8 (*p_dmac_radar_filter_cb)(frw_msg *msg);
typedef struct {
    p_dmac_radar_filter_cb hmac_radar_filter;
} hmac_chan_mgmt_rom_cb;

typedef struct {
    oal_bool_enum_uint8  dfs_switch;                          /* DFS使能开关
                                                                    bit0:dfs使能,
                                                                    bit1:标示AP因为DFS特性暂时处于邋wait_start */
    oal_bool_enum_uint8  cac_switch;
    oal_bool_enum_uint8  offchan_cac_switch;
    osal_u8              debug_level;                         /* bit0:打印雷达带业务，bit1:打印雷达无业务 */
    osal_u8              offchan_flag;                        /* bit0:0表示homechan,1表示offchan;
                                                                    bit1:0表示普通,1表示offchancac */
    osal_u8              offchan_num;
    osal_u8              timer_cnt;
    osal_u8              timer_end_cnt;
    osal_u8              cts_duration;
    osal_u8              hmac_channel_flag;                   /* dmac用于标示当前信道off or home */
    oal_bool_enum_uint8  dfs_init;
    osal_u8              custom_next_chnum;                   /* 应用层指定的DFS下一跳信道号 */
    osal_u32             dfs_cac_outof_5600_to_5650_time_ms;  /* CAC检测时长.5600MHz ~ 5650MHz频段外.默认60秒 */
    osal_u32             dfs_cac_in_5600_to_5650_time_ms;     /* CAC检测时长.5600MHz ~ 5650MHz频段内.默认10分钟 */
    osal_u32             off_chan_cac_outof_5600_to_5650_time_ms; /* Off-Channel CAC检测时长
                                                                        5600MHz ~ 5650MHz频段外.默认6分钟 */
    osal_u32             off_chan_cac_in_5600_to_5650_time_ms;    /* Off-Channel CAC检测时长
                                                                        5600MHz ~ 5650MHz频段内.默认60分钟 */
    osal_u16             dfs_off_chan_cac_opern_chan_dwell_time;  /* Off-channel CAC在工作信道上驻留时长 */
    osal_u16             dfs_off_chan_cac_off_chan_dwell_time;    /* Off-channel CAC在Off-Channel信道上驻留时长 */
    osal_u32             radar_detected_timestamp;
    osal_s32             l_radar_th;                                 /* 设置的雷达检测门限，单位db */

    osal_u32             custom_chanlist_bitmap;                  /* 应用层同步下来的信道列表
                                                                        (0x0000000F 36-48)   (0x000000F0 52-64)
                                                                        (0x000FFF00 100-144) (0x01F00000 149-165) */
    wlan_channel_bandwidth_enum_uint8   next_ch_width_type;       /* 设置的下一跳信道的带宽模式 */
    osal_u8              uac_resv[3];
    osal_u32             dfs_non_occupancy_period_time_ms;
    osal_u8              _rom[4];
} mac_dfs_info_stru;

typedef struct {
    osal_u8             chan_idx;            /* 信道索引 */
    osal_u8             device_id;           /* device id */
    osal_u8             auc_resv[2];
    frw_timeout_stru      dfs_nol_timer;     /* NOL节点定时器 */
    struct osal_list_head   entry;             /* NOL链表 */
} mac_dfs_nol_node_stru;

typedef struct {
    frw_timeout_stru      dfs_cac_timer;                   /* CAC定时器 */
    frw_timeout_stru      dfs_off_chan_cac_timer;          /* Off-Channel CAC定时器 */
    frw_timeout_stru      dfs_chan_dwell_timer;            /* 信道驻留定时器，定时器到期，切离该信道 */
    frw_timeout_stru      dfs_radar_timer;
    mac_dfs_info_stru     dfs_info;
    struct osal_list_head   dfs_nol;
    osal_u16 dfs_timeout;
    osal_u8 resv[2]; /* 保留2字节对齐 */
    osal_u32 dfs_cnt; /* 检测到的雷达个数 */
} mac_dfs_core_stru;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern osal_void hmac_dfs_init(hmac_device_stru *hmac_device);
extern osal_void hmac_dfs_deinit(hmac_device_stru *hmac_device);
extern osal_void hmac_dfs_channel_list_init_etc(hmac_device_stru *hmac_device);
extern osal_u32 hmac_dfs_recalculate_channel_etc(hmac_device_stru *hmac_device, osal_u8 *freq,
    wlan_channel_bandwidth_enum_uint8 *pen_bandwidth);
extern osal_void hmac_dfs_cac_start_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap);
extern osal_void hmac_dfs_cac_stop_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap, osal_u8 cac_tx_en);
extern osal_void hmac_dfs_off_cac_stop_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap);
extern osal_s32 hmac_dfs_radar_detect_event_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_dfs_radar_detect_event_test(osal_u8 vap_id);
extern osal_u32 hmac_dfs_ap_wait_start_radar_handler_etc(hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_dfs_ap_up_radar_handler_etc(hmac_vap_stru *hmac_vap);
extern osal_void hmac_dfs_radar_wait_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap);
extern osal_void test_dfs(osal_u8 vap_id);
extern osal_u32 hmac_dfs_start_bss_etc(hmac_vap_stru *hmac_vap);
extern osal_void hmac_dfs_off_chan_cac_start_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap);
extern oal_bool_enum_uint8 hmac_dfs_try_cac_etc(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap);
extern osal_u32 hmac_dfs_init_scan_hook_etc(hmac_scan_record_stru    *scan_record,
    hmac_device_stru *hmac_device);
extern oal_bool_enum_uint8 mac_vap_get_dfs_enable(hmac_vap_stru *hmac_vap);
void mac_dfs_set_cac_enable(hmac_device_stru *hmac_device, oal_bool_enum_uint8 val);
void mac_dfs_set_offchan_cac_enable(hmac_device_stru *hmac_device, oal_bool_enum_uint8 val);
oal_bool_enum_uint8 mac_dfs_get_offchan_cac_enable(hmac_device_stru *hmac_device);
oal_bool_enum_uint8 mac_dfs_get_cac_enable(hmac_device_stru *hmac_device);
void mac_dfs_set_dfs_enable(hmac_device_stru *hmac_device, oal_bool_enum_uint8 val);
oal_bool_enum_uint8 mac_dfs_get_dfs_enable(  hmac_device_stru *hmac_device);
void mac_dfs_set_debug_level(hmac_device_stru *hmac_device, osal_u8 debug_lev);
osal_u8 mac_dfs_get_debug_level(hmac_device_stru *hmac_device);
osal_void hmac_dfs_radar_detect_check(hal_to_dmac_device_stru *hal_device, hmac_device_stru *hmac_device,
    const hmac_vap_stru *hmac_vap);
osal_s32 hmac_cac_chan_ctrl_machw_tx_event_process(hmac_vap_stru *hmac_vap, osal_u8 cac_machw_en);
osal_s32 hmac_dfs_radar_detect_event(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_s32  hmac_config_set_country_for_dfs(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_config_cali_dfs_handle(hal_to_dmac_device_stru *hal_device, hmac_vap_stru *hmac_vap,
    const hmac_device_stru *hmac_device);
extern osal_void hmac_cac_chan_ctrl_machw_tx(hmac_vap_stru *hmac_vap, osal_u8 cac_machw_en);
osal_bool hmac_is_dfs_with_dbac_check(const hmac_vap_stru *hmac_vap, const hmac_device_stru *mac_device,
    const osal_u8 channel);
oal_bool_enum_uint8  hmac_dfs_need_for_cac(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif   /* end of _PRE_WLAN_FEATURE_DFS */

#endif /* end of hmac_dfs.h */
