/*
* Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
* Description: mac csa sta
* Create: 2020-7-5
*/

#ifndef __HMAC_CSA_STA_H__
#define __HMAC_CSA_STA_H__
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_vap_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏
*****************************************************************************/
#define MAC_CSA_DISABLE_TIMEOUT 120000
#define MAC_CSA_SWITCH_FAIL_TRIGGER_SCAN_MAX 4

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    WLAN_STA_CSA_FSM_INIT = 0,
    WLAN_STA_CSA_FSM_START,
    WLAN_STA_CSA_FSM_SWITCH,
    WLAN_STA_CSA_FSM_WAIT,
    WLAN_STA_CSA_FSM_INVALID,
    WLAN_STA_CSA_FSM_BUTT
} wlan_sta_csa_fsm_enum;
typedef osal_u8 wlan_sta_csa_fsm_enum_uint8;

typedef enum {
    WLAN_STA_CSA_EVENT_GET_IE = 0,
    WLAN_STA_CSA_EVENT_TBTT,
    WLAN_STA_CSA_EVENT_RCV_BEACON,
    WLAN_STA_CSA_EVENT_RCV_PROBE_RSP,
    WLAN_STA_CSA_EVENT_SCAN_END,
    WLAN_STA_CSA_EVENT_TO_INIT,
    WLAN_STA_CSA_EVENT_BUTT
} wlan_sta_csa_event_enum;
typedef osal_u8 wlan_sta_csa_event_enum_uint8;

typedef struct {
    osal_u8 next_expect_cnt;
    osal_u8 csa_scan_after_tbtt;
    osal_u8 sta_csa_last_cnt;                /* 收到 ap上一次发送的切换个数 */
    oal_bool_enum_uint8 is_fsm_attached : 4; /* 状态机是否已经注册 */
    oal_bool_enum_uint8 expect_cnt_status : 4;
    osal_u8 associate_channel;          /* csa扫描获取probe response 帧信道 */
    oal_bool_enum_uint8 is_switch_fail; /* 记录上一次csa切换是否失败，防止频繁切换攻击 */
    osal_u8 auc_resv[2];                /* 2字节对齐 */
    oal_fsm_stru oal_fsm; /* csa状态机 */
    frw_timeout_stru csa_handle_timer;
} mac_sta_csa_fsm_info_stru;

typedef struct {
    osal_u8 new_channel;                             /* STA即将切换的新信道 */
    wlan_channel_bandwidth_enum_uint8 new_bandwidth; /* STA即将切换的新带宽 */
    osal_u8 csa_switch_fail_trigger_scan_cnt; /* CSA scan次数 */
    osal_u8 mode;
    mac_channel_stru old_channel; /* STA信道切换时保存切换前信道信息 */
    frw_timeout_stru csa_stop_timer;          /* CSA 停止定时器, the feature from dmac.ko */
    mac_sta_csa_fsm_info_stru sta_csa_fsm_info;
} hmac_vap_csa_stru;

/*****************************************************************************
  对外函数声明
*****************************************************************************/
typedef osal_bool (*hmac_sta_csa_fsm_is_waiting_cb)(hmac_vap_stru *hmac_vap);

static osal_u32 hmac_csa_sta_init_weakref(osal_void)
    __attribute__ ((weakref("hmac_csa_sta_init"), used));
static osal_void hmac_csa_sta_deinit_weakref(osal_void)
    __attribute__ ((weakref("hmac_csa_sta_deinit"), used));
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
