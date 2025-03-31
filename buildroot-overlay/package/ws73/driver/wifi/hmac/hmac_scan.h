/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_scan_rom.c 的头文件。
 * Create: 2020-07-03
 */

#ifndef __HMAC_SCAN_H__
#define __HMAC_SCAN_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "mac_device_ext.h"
#include "hmac_device.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SCAN_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_SCAN_CHANNEL_STATICS_PERIOD_US 15 /* MAC信道负载统计周期 15ms */
#define HMAC_SCAN_CHANNEL_MEAS_PERIOD_MS 15             /* PHY空闲信道估计窗长15ms(最大15ms) */
#define HMAC_SCAN_CTS_MAX_DURATION 32767
#define HMAC_SCAN_MAX_TIMER (60 * 1000)                  /* 支持最大的timer时间 */
#define HMAC_SCAN_CHANNEL_DWELL_TIME_MARGIN 2          /* ms */
#define HMAC_SCAN_P2PGO_SEND_BEACON_TIME 10            /* ms, go tbtt中断后过多久切信道 */
#define HMAC_SCAN_DBAC_SCAN_DELTA_TIME 1500            /* ms */
#define HMAC_SCAN_GO_MAX_SCAN_TIME 300                 /* ms */
#define HMAC_SCAN_CHANENL_NUMS_TO_PRINT_SWITCH_INFO 14 /* 扫描信道数少于此才打印出切换到XX信道的信息 */
#define HMAC_SCAN_TIMER_DEVIATION_TIME 20              /* 扫描定时器的执行误差时间ms */
#define MSEC_TO_SEC 1000
#define HMAC_SCAN_SHIFT  10                            /* mac扫描周期单位由1kus转换为1us */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    SCAN_CHECK_ASSOC_CHANNEL_LINKLOSS = 0,
    SCAN_CHECK_ASSOC_CHANNEL_CSA = 1,

    SCAN_CHECK_ASSOC_CHANNEL_BUTT
} scan_check_assoc_channel_enum;
typedef osal_u8 scan_check_assoc_channel_enum_uint8;

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef osal_void (*p_dmac_scan_get_ch_statics_measurement_result_cb)(const hal_to_dmac_device_stru *hal_device,
    hal_ch_statics_irq_event_stru *stats_result);
typedef osal_void (*p_dmac_scan_calcu_channel_ratio_cb)(hal_to_dmac_device_stru *hal_device);

typedef osal_u32 (*hmac_scan_req_entry_cb)(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_scan_req_stru * const scan_req_params);

typedef struct {
    p_dmac_scan_get_ch_statics_measurement_result_cb hmac_scan_get_ch_statics_measurement_result;
    p_dmac_scan_calcu_channel_ratio_cb hmac_scan_calcu_channel_ratio;
} hmac_scan_rom_cb;

typedef struct {
    osal_u8 channel_num_offset;
    wlan_channel_bandwidth_enum_uint8 bandwidth;
} hmac_scan_chan_offset_bandwidth;
/*****************************************************************************
  8 内联函数定义
*****************************************************************************/
/*****************************************************************************
 函 数 名  : hmac_scan_get_duty_cyc_ratio
 功能描述  : 根据信道扫描结果和空闲时长获取占空比
           [0-1000], 越大表示信道越忙
           理论上
           信道测量时间 = 信道空闲时间+ //能量低于CCA门限
           空口的干扰时间+能量高于CCA门限，但是无法解析的能量信号(临频/叠频干扰)
           信道发送时间+自身竞争获取到的发送时间
           信道接收时间+空口中别的节点发送的时间
           蓝牙中断的时间(optional)//蓝牙共存时，蓝牙收发占用的时间
           所以通过测量到的信道空闲时间就可以用来表征当前信道的繁忙度。
*****************************************************************************/
static INLINE__ osal_u32 hmac_scan_get_duty_cyc_ratio(
    const wlan_scan_chan_stats_stru* chan_result, osal_u32 total_free_time_us)
{
    return ((total_free_time_us > chan_result->total_stats_time_us) ? 0 :
        ((chan_result->total_stats_time_us - total_free_time_us) * 1000 /
        chan_result->total_stats_time_us));
}

/*****************************************************************************
 函 数 名  : hmac_scan_get_valid_free_time
 功能描述  : 判断每个周期的统计时间是否越界
*****************************************************************************/
static INLINE__ osal_u32 hmac_scan_get_valid_free_time(
    osal_u32 trx_time, osal_u32 total_stat_time, osal_u32 total_free_time)
{
    return ((total_stat_time >= (trx_time + total_free_time)) ? total_free_time :
        ((total_stat_time <= trx_time) ? 0 : (total_stat_time - trx_time)));
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/

osal_u32 hmac_scan_proc_scan_complete_event(hmac_vap_stru *hmac_vap, mac_scan_status_enum_uint8 scan_rsp_status);
osal_u32 hmac_scan_mgmt_filter(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf, oal_bool_enum_uint8 *report_bss,
    osal_u8 *go_on);
osal_u32 hmac_scan_check_bss_type(const osal_u8 *frame_body, const mac_scan_req_stru *scan_params);
osal_u32 hmac_scan_handle_scan_req_entry(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_scan_req_stru * const scan_req_params);
osal_void hmac_scan_switch_channel_off(const hmac_device_stru *hmac_device);
oal_bool_enum_uint8 hmac_scan_switch_channel_back(hmac_device_stru *hmac_device,
    hal_to_dmac_device_stru *hal_device);
osal_void hmac_switch_channel_off(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_channel_stru * const dst_chl, osal_u16 protect_time);
osal_void hmac_scan_begin(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device);
osal_void hmac_scan_end(hmac_device_stru *hmac_device);
osal_void hmac_scan_prepare_end(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device);
osal_void hmac_scan_abort(hmac_device_stru *hmac_device);

/* 中断事件处理函数 */
osal_s32 hmac_scan_channel_statistics_complete(hmac_vap_stru *hmac_vap, frw_msg *msg);
/* 初始化及释放函数 */
osal_u32 hmac_scan_init(hmac_device_stru *hmac_device);

/* 外部函数引用 */
osal_u32 hmac_scan_send_probe_req_frame(hmac_vap_stru *hmac_vap, const osal_u8 *bssid,
    const osal_char *ssid, osal_u32 ssid_len, osal_u8 is_send_cap_ie);
osal_void hmac_dbac_switch_channel_off(hmac_device_stru *hmac_device, hmac_vap_stru *mac_vap1, hmac_vap_stru *mac_vap2,
    const mac_channel_stru *dst, osal_u16 protect_time);
osal_void hmac_scan_switch_home_channel_work(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device);
osal_void hmac_scan_handle_switch_channel_back(hmac_device_stru *hmac_device, hal_to_dmac_device_stru *hal_device,
    const hal_scan_params_stru *hal_scan_params);
osal_void hmac_scan_calcu_channel_ratio(hal_to_dmac_device_stru *hal_device);
osal_void hmac_scan_one_channel_start(hal_to_dmac_device_stru *hal_device, oal_bool_enum_uint8 is_scan_start);

/* 扫描入口钩子 */
osal_u32 hmac_scan_handle_scan_req_entry_ext(hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap,
    const mac_scan_req_stru * const scan_req_params, osal_u8 *continue_flag);

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/* 扫描到的bss的老化时间，小于此值，下发新扫描请求时，不删除此bss信息 */
#define HMAC_SCAN_MAX_SCANNED_BSS_EXPIRE         25000   /* 25000 milliseconds */

/* 扫描结果中有效性高的时间范围，可用于漫游目标的挑选等操作 */
#define HMAC_SCAN_MAX_VALID_SCANNED_BSS_EXPIRE   5000    /* 5000 milliseconds */

/* 扫描结果中DFS信道有效性高的时间范围，可用于漫游目标的挑选等操作 */
#define HMAC_SCAN_MAX_VALID_SCANNED_DFS_EXPIRE   15000   /* 15000 milliseconds */

/* 扫描到的bss的rssi老化时间 */
#define HMAC_SCAN_MAX_SCANNED_RSSI_EXPIRE        1000    /* 1000 milliseconds */

#define HMAC_INIT_SCAN_TIMEOUT_MS   10000
/* 扫描到的bss的rssi信号门限，门限以下不考虑对obss的影响 */
#define HMAC_OBSS_RSSI_TH           (-85)

/* multi bssid场景中，maxbssid_indicator最大规格为8 */
#define HMAC_MULTI_MAX_BSSID_INDICATOR 8

/* 在Multiple BSSID-Index element中包含DTIM count/period字段的时候，其element len长度为3 */
#define MULTI_DTIM_PERIOD_COUNT_LEN 3

#define EXT_CAPA_GET_MULTI_BSSID_LEN 2

#define MULTI_BSSID_ELEMENT_MIN_LEN 5

typedef enum {
    SCAN_RANDOM_CLOSE = 0,
    SCAN_RANDOM_WITH_OUI = 1,
    SCAN_RANDOM_FORCE = 2,
    SCAN_RANDOM_MAX
} scan_random_mac_type;

typedef struct {
    mac_scanned_all_bss_info *all_bss_info;

    osal_u8 dtim_period;
    osal_u8 dtim_count;
    osal_u8 period_count_valid;
    osal_u8 resv;
} hmac_scanned_bssid_index_info;

/* 存储单个nontrans_bssid_profile信息 */
typedef struct {
    struct osal_list_head dlist_head;     /* 链表指针 */
    osal_u8  bssid_profile[WLAN_MGMT_NETBUF_SIZE];
    osal_u32 frame_len;
} hmac_scanned_nontrans_bssid_info;

typedef enum {
    PARA_TYPE_SCAN_PROBE_SEND_TIMES = 0,
    PARA_TYPE_SCAN_TIME = 1,
    PARA_TYPE_SCAN_COUNT = 2,
    PARA_TYPE_BUTT
} scan_para_type;

/* ini配置的扫描参数,  目前仅用于无up vap, 并且sta发起扫描时 */
typedef struct {
    osal_u8 probe_req_send_times; /* sta扫描probe req报文发送次数, 默认1次 */
    osal_u8 single_scan_time; /* sta某个信道单次扫描停留时间, 默认20ms */
    osal_u8 scan_count; /* sta某个信道扫描次数, 默认2次 */
    osal_u8 resv;
} hmac_scan_config_info;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
/* HMAC SCAN对外接口 */
extern osal_void hmac_scan_print_scanned_bss_info_etc(osal_u8 device_id);

extern mac_bss_dscr_stru *hmac_scan_find_scanned_bss_dscr_by_index_etc(osal_u8  device_id,
    osal_u32 bss_index);
extern hmac_scanned_bss_info *hmac_scan_find_scanned_bss_by_bssid_etc(hmac_bss_mgmt_stru *bss_mgmt,
    const osal_u8 *bssid);
extern osal_void *hmac_scan_get_scanned_bss_by_bssid(hmac_vap_stru *hmac_vap, osal_u8 *mac_addr);
extern osal_void hmac_scan_clean_scan(hmac_scan_stru  *scan);
extern osal_u32  hmac_scan_proc_scan_req_event_exception_etc(hmac_vap_stru *hmac_vap, osal_void *p_params);
extern osal_u32  hmac_scan_proc_scan_req_event_etc(hmac_vap_stru *hmac_vap, osal_void *p_params);
#ifdef _PRE_WLAN_FEATURE_PNO_SCAN
extern osal_u32  hmac_scan_proc_sched_scan_req_event_etc(hmac_vap_stru *hmac_vap, osal_void *p_params);

/* 停止定时器 */
osal_void hmac_scan_stop_pno_sched_scan_timer(mac_pno_sched_scan_mgmt_stru *pno_mgmt);
#endif
osal_u8 hmac_scan_get_scan_para_from_config(osal_u8 type);
osal_void hmac_scan_set_config_info(hmac_scan_config_info *config_info);

extern osal_void    hmac_scan_init_etc(hmac_device_stru *hmac_device);
extern osal_void    hmac_scan_exit_etc(hmac_device_stru *hmac_device);
extern osal_s32 hmac_scan_process_chan_result_event_etc(hmac_crx_chan_result_stru *chan_result_param);
extern osal_s32 hmac_scan_proc_scan_comp_event_etc(hmac_vap_stru *hmac_vap, mac_scan_rsp_stru *scan_rsp_info);
#ifdef _PRE_WLAN_FEATURE_WS92_MERGE
osal_s32 hmac_scan_clean_result(hmac_vap_stru *hmac_vap, frw_msg *msg);
#endif
extern osal_void hmac_scan_handle_channel_randomzie(mac_scan_req_stru *scan_params, osal_u8 channel_num_5g,
    osal_u8 channel_num_2g);
extern osal_u32 hmac_start_all_bss_of_device_etc(hmac_device_stru *hmac_device);
extern osal_void hmac_scan_set_sour_mac_addr_in_probe_req_etc(hmac_vap_stru *hmac_vap,
    osal_u8 *sour_mac_addr, oal_bool_enum_uint8 is_rand_mac_scan);
osal_u32 hmac_scan_proc_check_ssid(const osal_u8 *ssid, osal_u8 ssid_len);
osal_void hmac_scan_delete_bss(hmac_vap_stru *hmac_vap, const osal_u8 *bssid);
osal_s32 hmac_config_set_scan_param(hmac_vap_stru *hmac_vap, frw_msg *msg);
osal_void hmac_scan_set_probe_req_all_ie_cfg(osal_u8 probe_req_all_ie);
osal_u8 hmac_scan_get_probe_req_all_ie_cfg(osal_void);
osal_void hmac_scan_set_probe_req_del_wps_ie_cfg(osal_u8 probe_req_del_wps_ie);
osal_u8 hmac_scan_get_probe_req_del_wps_ie_cfg(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_scan.h */
