/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: host plat pm header file
 * Author: CompanyName
 */

#ifndef __PLAT_PM_H__
#define __PLAT_PM_H__

#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "soc_osal.h"
#include "oal_net.h"
#include "plat_pm_board.h"
#include "hcc_bus.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 Define macro
*****************************************************************************/
#define FIRMWARE_CFG_INIT_OK            0x01

#define SUCCESS                         (0)
#define FAILURE                         (1)

#define HOST_DISALLOW_TO_SLEEP          (0)
#define HOST_ALLOW_TO_SLEEP             (1)
#define HOST_WKING_FROM_SLEEP           (2)

#define WAIT_DEVACK_MSEC                (10)
#define WAIT_DEVACK_CNT                 (10)
#define WAIT_DEVACK_TIMEOUT_MSEC        (2000)

/* 超时时间要大于wkup dev work中的最长执行时间，否则超时以后进入DFR和work中会同时操作tty，导致冲突 */
#define WAIT_WKUPDEV_MSEC              (10000)

#define NEED_WAIT_NEXT_GPIO   0x55
#define WAIT_GPIO_INTR_DONE   0x5a

// 芯片上电状态枚举
typedef enum {
    POWER_STATE_SHUTDOWN = 0,
    POWER_STATE_OPEN     = 1,
    POWER_STATE_BUTT     = 2,
} power_state_enum;

// 业务加载状态枚举
typedef enum {
    PM_SVC_STATE_SHUTDOWN = 0,
    PM_SVC_STATE_OPEN     = 1,
    PM_SVC_STATE_BUTT     = 2,
} pm_svc_state_enum;

// wlan启动状态枚举
typedef enum {
    WLAN_STATE_SHUTDOWN = 0,
    WLAN_STATE_OPEN     = 1,
    WLAN_STATE_BUTT     = 2,
} wlan_state_enum;

// 动态调频投票档位, 只允许投票60M/120M/240M
enum pm_crg_freq_lvl_enum {
    PM_CRG_FREQ_LVL_60M,        // PLL时钟 60MHz
    PM_CRG_FREQ_LVL_120M,       // PLL时钟120MHz
    PM_CRG_FREQ_LVL_240M,       // PLL时钟240MHz
    PM_CRG_FREQ_LVL_SLEEP,      // 低功耗休眠时档位

    PM_CRG_FREQ_LVL_NUM
};

// 动态调频业务类型枚举
enum pm_fsm_service_id_enum {
    PM_FSM_SVC_BEGIN = 0,

    PM_FSM_SVC_WLAN = PM_FSM_SVC_BEGIN,
    PM_FSM_SVC_BT,
    PM_FSM_SVC_PLT,

    PM_FSM_SVC_END,
    PM_FSM_SVC_NUM = PM_FSM_SVC_END
};

/*****************************************************************************
  3 STRUCT DEFINE
*****************************************************************************/
/* private data for pm driver */
struct pm_drv_data
{
    /* wlan interface pointer */
    struct wlan_pm_s                *pst_wlan_pm_info;
    /* board customize info */
    pm_board_info                      *board;
    /* mutex for sync */
    osal_mutex host_mutex;
    /* flag for firmware cfg file init */
    osal_ulong firmware_cfg_init_flag;
    struct semaphore sr_wake_sema;     /* android suspend and resume sem */
    oal_wakelock_stru st_wakelock;
    struct completion dev_bsp_ready;
    struct completion dev_wlan_ready;
    osal_u64 data_int_count;
    osal_u64 wakeup_int_count;
    osal_u64 gpio_int_count;
};

// 动态调频控制结构
typedef struct {
    osal_u8 pm_crg_enable;
    osal_u8 pm_crg_max_req_freq;
    osal_u8 pm_crg_cur_freq;
    osal_u8 rsv1;

    osal_u8 pm_crg_req_freq[PM_FSM_SVC_NUM];
    osal_u8 rsv2;
} pm_crg_info_stru;

/*****************************************************************************
  4 EXTERN VARIABLE
*****************************************************************************/

/*****************************************************************************
  5 EXTERN FUNCTION
*****************************************************************************/
extern struct pm_drv_data * pm_get_drvdata(void);

int firmware_download_function(void);
#ifdef CONFIG_NO_SUPPORT_INI
osal_void hwifi_plat_pm_device_init_custom_param(void);
#endif
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
osal_s32 pm_device_detect_uart(osal_void);
#endif

osal_s32 pm_svc_power_on(void);
osal_s32 pm_svc_power_off(void);
osal_s32 pm_svc_power_reset(void);
osal_s32 low_power_init_etc(void);
void low_power_exit_etc(void);

// 动态调频对外接口
extern osal_u32 pm_crg_svc_vote(osal_u8 sid, osal_u8 freq_lvl);
extern oal_void pm_crg_enable_switch(oal_bool_enum_uint8 is_enable);
extern oal_int32 pm_crg_init(oal_void);
extern oal_void pm_crg_deinit(oal_void);
osal_s32 plat_pm_power_action(hcc_bus *pst_bus, osal_u32 action, struct pm_drv_data *pm_data);
osal_void pm_wakelocks_release_detect(struct pm_drv_data *pm_data);
osal_u32 pm_device_init_ready(osal_u8 *param);
osal_u32 pm_bsp_init_ready(osal_u8 *param);

#ifndef unref_param
#define unref_param(P)  ((P) = (P))
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

