/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: Host board config header file.
 * Author: Huanghe
 * Create: 2021-06-01
 */

#ifndef __BOARD_H__
#define __BOARD_H__
/*****************************************************************************
  1 Include other Head file
*****************************************************************************/
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#include "oal_plat_type.h"
#include "hcc_bus.h"

/*****************************************************************************
  2 Define macro
*****************************************************************************/

#define BOARD_SUCC                           (0)
#define BOARD_FAIL                           (-1)

#define VERSION_FPGA                         (0)
#define VERSION_ASIC                         (1)

#define WIFI_TAS_DISABLE                     (0)
#define WIFI_TAS_ENABLE                      (1)

#define PMU_CLK_REQ_DISABLE                  (0)
#define PMU_CLK_REQ_ENABLE                   (1)

#define GPIO_LOWLEVEL                        (0)
#define GPIO_HIGHLEVEL                       (1)

#define NO_NEED_POWER_PREPARE                (0)
#define NEED_POWER_PREPARE                   (1)

#define PINMUX_SET_INIT                      (0)
#define PINMUX_SET_SUCC                      (1)

/* ini cfg */
#define INI_BT_BUS                                "board_info.bt_bus"
#define INI_WIFI_BUS                              "board_info.wlan_bus"

#define DOWNlOAD_MODE_SDIO                        "sdio"
#define DOWNlOAD_MODE_PCIE                        "pcie"
#define DOWNlOAD_MODE_UART                        "uart"
#define DOWNlOAD_MODE_USB                         "usb"

#define BOARD_VERSION_LEN                         (128)
#define DOWNLOAD_CHANNEL_LEN                      (64)
#define BT_BUS_NAME_LEN                           5

#define INI_SSI_DUMP_EN                           "ssi_dump_enable"

/*****************************************************************************
  3 STRUCT DEFINE
*****************************************************************************/
typedef struct bd_init_s
{
    osal_s32 (*wlan_power_off_etc)(void);
    osal_s32 (*wlan_power_on_etc)(void);
    osal_s32 (*board_power_on_etc)(osal_u32 subsystem);
    osal_s32 (*board_power_off_etc)(osal_u32 subsystem);
    osal_s32 (*board_power_reset)(osal_u32 subsystem);
} bd_init_t;

/* private data for pm driver */
typedef struct {
    /* board init ops */
    struct bd_init_s bd_ops;

    /* power */
    osal_s32 power_gpio;

    /* wakeup gpio */
    osal_s32 wkup_gpio_support;
    osal_s32 wlan_wakeup_host;
    osal_s32 bt_wakeup_host;
    osal_s32 host_wakeup_wlan;

    /* how to download firmware */
    osal_s32 wlan_bus;
    osal_s32 bt_bus;

    /* irq info */
    osal_u32 wlan_irq;
    osal_u32 bt_irq;

    /* uart info */
    const char * uart_port;
    osal_s32 uart_pclk;

    /* ini cfg */
    char* ini_file_name;
}board_info;

typedef struct {
    osal_u32 index;
    osal_u8 name[DOWNLOAD_CHANNEL_LEN + 1];
} bus_table;

enum board_power
{
    WLAN_POWER  = 0x0,
    BFGX_POWER  = 0x1,
    POWER_BUTT,
};
enum board_power_state
{
    BOARD_POWER_OFF = 0x0,
    BOARD_POWER_ON = 0x1,
};

board_info * get_board_info(void);
osal_s32 get_wlan_bus_type(void);
osal_s32 get_bt_bus_type(void);
osal_s32 hh503_board_init(void);
void hh503_board_exit(void);
extern osal_s32 board_host_wakeup_dev_set(int value);
extern osal_s32 board_get_host_wakeup_dev_stat(void);
extern osal_s32 board_wifi_tas_set(int value);
extern osal_s32 board_power_on_etc(osal_u32 subsystem);
extern osal_s32 board_power_off_etc(osal_u32 subsystem);
extern osal_s32 board_power_reset(osal_u32 subsystem);
extern osal_s32 board_wlan_gpio_power_on(void);
extern osal_s32 board_wlan_gpio_power_off(void);
extern int board_get_bwkup_gpio_val_etc(void);
extern int board_get_wlan_wkup_gpio_val_etc(void);
extern osal_s32 get_board_gpio_etc(const char *gpio_node, const char *gpio_prop, osal_s32 *physical_gpio);
extern void boart_power_state_change(osal_s32 flag);
extern osal_s32 get_board_custmize_etc(const char *cust_node, const char *cust_prop, const char **cust_prop_val);
extern osal_s32 get_board_dts_node_etc(struct device_node **np, const char *node_prop);
int board_is_support_wkup_gpio(void);
#endif

