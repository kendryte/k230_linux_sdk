/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: plat pm board source header file
 * Author: CompanyName
 */

#ifndef __PLAT_PM_BOARD_H__
#define __PLAT_PM_BOARD_H__

/*****************************************************************************
  1 Include other Head file
*****************************************************************************/

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#endif
#include "soc_osal.h"

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "linux/oal_debug.h"
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#include "liteos/oal_debug.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 Define macro
*****************************************************************************/

#define BOARD_SUCC                           (0)
#define BOARD_FAIL                           (-1)

#define GPIO_LOWLEVEL                        (0)
#define GPIO_HIGHLEVEL                       (1)

/* ini cfg */
#define INI_BT_BUS                                "board_info.bt_bus"
#define INI_WIFI_BUS                              "board_info.wlan_bus"

#define DOWNLOAD_MODE_SDIO                        "sdio"
#define DOWNLOAD_MODE_PCIE                        "pcie"
#define DOWNLOAD_MODE_UART                        "uart"
#define DOWNLOAD_MODE_USB                         "usb"

#define BOARD_VERSION_LEN                         (128)
#define DOWNLOAD_CHANNEL_LEN                      (64)
#define BT_BUS_NAME_LEN                           5

#define WAIT_BOARD_DRIVER_S                 10

/* 字符串转数字的进制数宏定义 */
#define NUM_BASE_10                         10

#define GPIO_IDX_INVALID                    (-1)
#define GPIO_IRQ_INVALID                    (-1)

#define POWER_ON_LEVEL_DEFAULT              GPIO_HIGHLEVEL

#define PROC_NAME_GPIO_POWER_ON             "power_gpio"
#define PROC_NAME_GPIO_DEVICE_WAKEUP_HOST   "wkup_gpio"
#define PROC_NAME_GPIO_DEVICE_UART          "uart_gpio"
/*****************************************************************************
  3 STRUCT DEFINE
*****************************************************************************/
typedef struct {
    osal_s32 (*wlan_power_off_etc)(osal_void);
    osal_s32 (*wlan_power_on_etc)(osal_void);
    osal_s32 (*board_power_on_etc)(osal_void);
    osal_s32 (*board_power_off_etc)(osal_void);
    osal_s32 (*board_power_reset)(osal_bool sys_reboot);
} pm_board_func;

typedef osal_s32 (*pm_board_cb)(void);

/* private data for pm driver */
typedef struct {
    /* board init ops */
    pm_board_func bd_ops;

    /* power */
    osal_s32 power_gpio_support;
    osal_s32 power_gpio;
    osal_s32 power_on_level;

    /* wakeup gpio */
    osal_s32 wkup_gpio_support;
    osal_s32 wkup_gpio_idx;
    osal_s32 wkup_gpio_irq;
    osal_s32 wlan_wakeup_host;
    osal_s32 host_wakeup_wlan;
    pm_board_cb wkup_gpio_cb;

    /* irq info */
    osal_u32 wlan_irq;

    /* ini cfg */
    char *ini_file_name;
} pm_board_info;

typedef struct {
    osal_u32 index;
    osal_u8 name[DOWNLOAD_CHANNEL_LEN + 1];
} pm_board_bus_table;

enum pm_board_power_exception_enum {
    PM_BOARD_POWER_FAIL = -1,
    PM_BOARD_POWER_SUCCESS = 0,
    PM_BOARD_POWER_PULL_POWER_GPIO_FAIL = 1,
    PM_BOARD_POWER_BOOT_UP_FAIL = 2,
    PM_BOARD_POWER_FIRMWARE_DOWNLOAD_FAIL = 3,
    PM_BOARD_POWER_ENUM_BUTT,
};

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_GPIO_USING_HI_DRV)
extern td_s32 hi_drv_gpio_set_dir_bit(td_u32 gpio_no, td_u32 dir_bit);
extern td_s32 hi_drv_gpio_write_bit(td_u32 gpio_no, td_u32 bit_value);
#endif
#endif

pm_board_info *get_pm_board_info(osal_void);
osal_s32 pm_board_init(osal_void);
osal_void pm_board_exit(osal_void);
extern osal_s32 pm_board_host_wakeup_dev_set(osal_s32 value);
extern osal_s32 pm_board_wifi_tas_set(osal_s32 value);
osal_s32 pm_board_is_support_wkup_gpio(osal_void);
osal_s32 is_gpio_idx_valid(osal_s32 gpio_idx);
osal_s32 plat_pm_bus_resume(osal_void);
osal_s32 plat_pm_bus_suspend(osal_void);
td_void pm_board_wkup_gpio_register(pm_board_cb cb);

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_bool pm_resume_flag_get(osal_void);
osal_void pm_resume_flag_set(osal_bool resume_flag);
osal_bool pm_suspend_flag_get(osal_void);
osal_void pm_suspend_flag_set(osal_bool suspend_flag);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __PLAT_PM_BOARD_H__ */
