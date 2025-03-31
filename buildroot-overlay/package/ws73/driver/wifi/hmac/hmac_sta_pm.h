/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 * Description: hmac_sta_pm_rom.c 的头文件。
 * Create: 2020-07-03
 */

#ifndef HMAC_STA_PM_H
#define HMAC_STA_PM_H

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hmac_vap.h"
#include "msg_psm_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define HMAC_ALLOW_SLEEP     (1)
#define HMAC_DISALLOW_SLEEP  (0)

#define HMAC_WAKUP_MSG_WAIT_TIMEOUT    (600)
#define HMAC_SLEEP_MSG_WAIT_TIMEOUT    (600)

#define HMAC_SLEEP_TIMER_PERIOD        (60)  /* 睡眠定时器20ms定时 */

#define HMAC_MSG_RETRY_NUM      (3)
#define HMAC_FAIL_MAX_TIMES     (3)               /* 连续多少次wakeup失败，可进入DFR流程 */
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* TIM processing result */
typedef enum {
    STA_PWR_SAVE_STATE_ACTIVE = 0,         /* active状态 */
    STA_PWR_SAVE_STATE_DOZE,               /* doze状态 */
    STA_PWR_SAVE_STATE_AWAKE,              /* wake状态 */

    STA_PWR_SAVE_STATE_BUTT                /* 最大状态 */
} sta_pwr_save_state_info;


enum HMAC_PM_STAGE {
    HMAC_PM_STAGE_INIT              = 0,  // 初始
    HMAC_SLEEP_REQ_SND              = 1,  // sleep request发送完成
    HMAC_SLEEP_ALLOW_RCV            = 2,  // 收到allow sleep response
    HMAC_SLEEP_DISALLOW_RCV         = 3,  // 收到allow sleep response
    HMAC_SLEEP_CMD_SND              = 4,  // 允许睡眠reg设置完成
    HMAC_WKUP_REQ_SND               = 5,
    HMAC_WKUP_RSP_RCV               = 6,
    HMAC_WKUP_CMD_SND               = 7,
    HMAC_PM_STAGE_BUTT
};

typedef enum {
    STA_SYS_PM_STATE = 0,
    STA_NOT_SLEEP_REASON_DUMP,
    STA_WAKUP_REASON_DUMP,
    STA_11B_BCN_INFO,
    STA_WAKEUP_TIME_DUMP,

    STA_PM_DEBUG_INFO_BUTT
} sta_pm_debug_info;

typedef enum {
    HMAC_NOT_SLEEP_INFO_START = 0,
    HMAC_NOT_SLEEP_SCAN = HMAC_NOT_SLEEP_INFO_START,
    HMAC_NOT_SLEEP_VAP_NUM,
    HMAC_NOT_SLEEP_PS_MODE,
    HMAC_NOT_SLEEP_HCC,
    HMAC_NOT_SLEEP_FRW,
    HMAC_NOT_SLEEP_TX_CNT,

    HMAC_NOT_SLEEP_INFO_BUTT
} hmac_not_sleep_type;
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct {
    osal_u8 wlan_pm_enable;             // pm使能开关
    osal_u8 wlan_dev_state;                  // wlan sleep state
    osal_u8 pm_stage;
    osal_u8 wakeup_err_count;         // 连续唤醒失败次数
    osal_u8 fail_sleep_count;         // 连续睡眠失败次数
    osal_u8 rsv[3];
    osal_u32 pm_tx_activity_cnt;
    frw_timeout_stru watchdog_timer;  // sleep watch dog
    osal_u32 wdg_timeout_cnt;

    osal_wait wakeup_done;
    osal_wait sleep_request_ack;
    osal_mutex sleep_state_mutex;
} hmac_sta_pm_ctrl_info;
/*****************************************************************************
  8 内联函数定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void hmac_pm_sta_attach(hmac_vap_stru *hmac_vap);
osal_void hmac_set_sta_pm_on_cb(hmac_vap_stru *hmac_vap, mac_cfg_ps_open_stru *pm_cfg);
osal_void hmac_config_mvap_set_sta_pm_on(hmac_vap_stru *hmac_vap, oal_bool_enum mvap_up);
osal_u32 hmac_config_set_pm_switch(const hmac_vap_stru *hmac_vap, osal_u8 len, const osal_u8 *param);
osal_s32 hmac_sta_pm_on(hmac_vap_stru *hmac_vap, mac_pm_switch_enum_uint8 enable,
    mac_pm_ctrl_type_enum_uint8 ctrl_type);

static osal_u32 hmac_sta_pm_init_weakref(osal_void) __attribute__ ((weakref("hmac_sta_pm_init"), used));
static osal_void hmac_sta_pm_deinit_weakref(osal_void) __attribute__ ((weakref("hmac_sta_pm_deinit"), used));

osal_void hmac_sta_pm_feed_wdg(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl);
osal_void hmac_sta_pm_stop_wdg(hmac_sta_pm_ctrl_info *hmac_sta_pm_ctrl);
osal_u32 hmac_sta_pm_ctrl_set(osal_u32 flag);
osal_u8 hmac_sta_pm_get_wlan_dev_state(osal_void);
osal_u32 hmac_sta_pm_wakeup_dev(uintptr_t *buf, osal_u32 len, osal_u32 data_type);
WIFI_HMAC_TCM_TEXT hmac_sta_pm_ctrl_info *hmac_sta_pm_get_ctrl(osal_void);
WIFI_HMAC_TCM_TEXT osal_u32 hmac_wifi_pm_get_host_allow_sleep(osal_void);

osal_bool hmac_sta_pm_achba_keepalive(osal_u8 enable);

typedef osal_u8 (*hmac_sta_pm_get_wlan_dev_state_cb)(osal_void);
typedef osal_u32 (*hmac_sta_pm_wakeup_dev_cb)(uintptr_t *buf, osal_u32 len, osal_u32 data_type);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_sts_pm.h */
