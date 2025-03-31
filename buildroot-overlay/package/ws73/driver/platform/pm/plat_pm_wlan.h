/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: host plat pm header file
 * Author: CompanyName
 */

#ifndef __PLAT_PM_WLAN_H__
#define __PLAT_PM_WLAN_H__

/*****************************************************************************
  1 Include other Head file
*****************************************************************************/
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>

#include <linux/fb.h>
#endif
#include "soc_osal.h"
#include "oal_net.h"
#include "oal_ext_if.h"
#include "hcc_bus.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WLAN_WAKUP_MSG_WAIT_TIMEOUT     (1000)
#define WLAN_SLEEP_MSG_WAIT_TIMEOUT     (10000)
#define WLAN_POWEROFF_ACK_WAIT_TIMEOUT  (1000)
#define WLAN_OPEN_BCPU_WAIT_TIMEOUT     (1000)
#define WLAN_HALT_BCPU_TIMEOUT          (1000)
#define WLAN_SLEEP_TIMER_PERIOD         (50)    /* 睡眠定时器50ms定时 */
#define WLAN_SLEEP_DEFAULT_CHECK_CNT    (6)     /* 默认100ms */
#define WLAN_SLEEP_LONG_CHECK_CNT       (8)     /* 20 入网阶段,延长至400ms */
#define WLAN_SLEEP_FAST_CHECK_CNT       (1)     /* fast sleep,20ms */
#define WLAN_WAKELOCK_HOLD_TIME         (500)   /* hold wakelock 500ms */

#define WLAN_SDIO_MSG_RETRY_NUM         (3)
#define WLAN_WAKEUP_FAIL_MAX_TIMES      (1)  /* 连续多少次wakeup失败，可进入DFR流程 */

#define HOST_WAIT_BOTTOM_WIFI_TIMEOUT   20000
#define HOST_WAIT_BOTTOM_WIFI_TIMEOUT_QUARTERS   5000
#define H2W_SLEEP_REQUEST_FORBID_LIMIT  60
#define H2W_TRY_WKUP_MAX_TIMES  3
#define WLAN_PM_MODULE               "[wlan]"


enum WLAN_PM_CPU_FREQ_ENUM {
    WLCPU_40MHZ     = 1,
    WLCPU_80MHZ     = 2,
    WLCPU_160MHZ    = 3,
    WLCPU_240MHZ    = 4,
    WLCPU_320MHZ    = 5,
    WLCPU_480MHZ    = 6,
};

enum WLAN_PM_SLEEP_STAGE {
    SLEEP_STAGE_INIT    = 0,  // 初始
    SLEEP_REQ_SND       = 1,  // sleep request发送完成
    SLEEP_ALLOW_RCV     = 2,  // 收到allow sleep response
    SLEEP_DISALLOW_RCV  = 3,  // 收到allow sleep response
    SLEEP_CMD_SND       = 4,  // 允许睡眠reg设置完成
};

enum WLAN_HOST_STATUS {
    HOST_NOT_SLEEP = 0,
    HOST_SLEEPED = 1,
};

/* 平台管理业务启停 begin */
enum pm_svc_type {
    PM_SVC_WLAN = 0,
    PM_SVC_BLE,
    PM_SVC_SLE,

    PM_SVC_NUM
};

#define ALLOW_IDLESLEEP     (1)
#define DISALLOW_IDLESLEEP  (0)

#define WIFI_PM_POWERUP_EVENT          (3)
#define WIFI_PM_POWERDOWN_EVENT        (2)
#define WIFI_PM_SLEEP_EVENT            (1)
#define WIFI_PM_WAKEUP_EVENT           (0)

/*****************************************************************************
  3 STRUCT DEFINE
*****************************************************************************/
typedef oal_bool_enum_uint8 (*wifi_srv_get_pm_pause_func)(osal_void);
typedef osal_void (*wifi_srv_open_notify)(oal_bool_enum_uint8);
typedef osal_void (*wifi_srv_pm_state_notify)(oal_bool_enum_uint8);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
typedef osal_void (*wifi_srv_data_wkup_print_en_func)(oal_bool_enum_uint8);
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
typedef struct workqueue_struct oal_workqueue_stru;
typedef struct work_struct oal_work_stru;
#endif

struct wifi_srv_callback_handler
{
    wifi_srv_get_pm_pause_func     p_wifi_srv_get_pm_pause_func;
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    wifi_srv_data_wkup_print_en_func     p_data_wkup_print_en_func;
#endif
};

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
struct wlan_pm_s {
    hcc_bus *pst_bus;              //保存oal_bus 的指针
    osal_u32 pm_enable;    // host 侧软件控制pm使能开关
    osal_u8 pm_power_state;  // 芯片上电状态
    osal_u8 pm_svc_state[PM_SVC_NUM]; // 业务加载状态
    osal_u16 rsv;
    osal_u32 wlan_state; // wlan业务启动状态
    osal_spinlock         st_irq_lock;
};
#endif

typedef struct wlan_memdump_s {
    osal_s32 addr;
    osal_s32 len;
    osal_s32 en;
} wlan_memdump_t;

/*****************************************************************************
  4 EXTERN VARIABLE
*****************************************************************************/
extern osal_u8 g_wlan_fast_check_cnt;
#if defined(_PRE_MULTI_CORE_MODE) && defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern osal_u8 g_uc_custom_cali_done_etc;
#endif
#endif
#endif

typedef void (*recovery_complete)(void);

typedef osal_s32 (*wlan_customize_init_cb)(osal_void);

typedef void (*pm_wow_wkup_cb)(void);
typedef void (*pm_wlan_suspend_cb)(void);
typedef osal_u32 (*pm_wlan_resume_cb)(void);
typedef void (*pm_ble_suspend_cb)(void);
typedef osal_s32 (*pm_ble_resume_cb)(void);
typedef void (*pm_sle_suspend_cb)(void);
typedef osal_s32 (*pm_sle_resume_cb)(void);

typedef void (*pm_bsle_enable_reply_cb)(osal_u8);

/*****************************************************************************
  5 EXTERN FUNCTION
*****************************************************************************/
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern struct wlan_pm_s*  wlan_pm_get_drv_etc(osal_void);
extern struct wlan_pm_s*  wlan_pm_init_etc(osal_void);
#endif
extern osal_ulong  wlan_pm_exit_etc(osal_void);
extern osal_u32 wlan_pm_is_poweron_etc(osal_void);
extern osal_void wlan_cfg_init_cb_reg(wlan_customize_init_cb cb_cfg);
extern osal_s32 wlan_open(osal_void);
extern osal_s32 wlan_open_ex(osal_void);
extern osal_u32 wlan_close(osal_void);
extern osal_u32 wlan_close_ex(osal_void);
struct wifi_srv_callback_handler* wlan_pm_get_wifi_srv_handler_etc(osal_void);
osal_s32 wlan_power_open_cmd(osal_void);
osal_s32 wlan_power_close_cmd(osal_void);
/* Increase or decrease the number of users accessing the device.
  @is_add: if true, after adding H2W users, the device system will be prevented from sleeping again.
           if false, will decrease user number, that mean no more access to device
*/
osal_u8 pm_svc_open_state_get(osal_u8 svc_type);

extern osal_void pm_wifi_suspend_cb_host_register(pm_wlan_suspend_cb cb);
extern osal_void pm_wifi_resume_cb_host_register(pm_wlan_resume_cb cb);
extern osal_void pm_ble_suspend_cb_host_register(pm_ble_suspend_cb cb);
extern osal_void pm_ble_resume_cb_host_register(pm_ble_resume_cb cb);
extern osal_void pm_ble_enable_reply_cb_host_register(pm_bsle_enable_reply_cb cb);
extern osal_void pm_sle_suspend_cb_host_register(pm_sle_suspend_cb cb);
extern osal_void pm_sle_resume_cb_host_register(pm_sle_resume_cb cb);
extern osal_void pm_sle_enable_reply_cb_host_register(pm_bsle_enable_reply_cb cb);
extern osal_void pm_wifi_wkup_cb_host_register(pm_wow_wkup_cb cb);

pm_wlan_suspend_cb pm_wifi_suspend_cb_host_get(osal_void);
pm_wlan_resume_cb pm_wifi_resume_cb_host_get(osal_void);
pm_ble_suspend_cb pm_ble_suspend_cb_host_get(osal_void);
pm_ble_resume_cb pm_ble_resume_cb_host_get(osal_void);
pm_bsle_enable_reply_cb pm_ble_enable_reply_cb_host_get(osal_void);
pm_sle_suspend_cb pm_sle_suspend_cb_host_get(osal_void);
pm_sle_resume_cb pm_sle_resume_cb_host_get(osal_void);
pm_bsle_enable_reply_cb pm_sle_enable_reply_cb_host_get(osal_void);

void pm_shutdown_process_host(void);
void pm_suspend_process_host(void);
osal_s32 pm_wkup_process_host(osal_void);
void pm_suspend_service(void);
osal_void pm_resume_service(osal_bool *dfr_flag);
#ifdef CONFIG_STR_RESUME_ASYNC
void pm_create_resume_workqueue(void);
void pm_destroy_resume_workqueue(void);
void pm_resume_process_host_async(void);
#else
void pm_resume_process_host(void);
#endif

#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
osal_s32 uapi_pm_device_detect(osal_void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
