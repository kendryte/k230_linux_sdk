/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: board adapter source file
 * Author: CompanyName
 */

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#endif

#include "hcc_host_if.h"
#include "oal_types.h"
#include "plat_pm_board.h"
#include "plat_debug.h"
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#endif

#include "customize_wifi.h"
#include "plat_firmware.h"
#include "hcc.h"
#include "hcc_usb_host.h"
#include "hcc_if.h"
#include "plat_cali.h"
#include "plat_pm_board_ws73.h"
#if defined(CONFIG_HCC_SUPPORT_UART)
#include "plat_firmware_uart.h"
#include "hcc_uart_host.h"
#endif

#ifdef CONFIG_PLAT_SUPPORT_DFR
#include "plat_pm_dfr.h"
#endif
#include "mac_addr.h"
#include "hcc_heartbeat.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/* hcc写寄存器复位device配置 */
#define CLK_RST_CRG_CFG_BASE (0x4001f000)
#define CLK_RST_CRG_CFG_GLB_RST_CRG_CTL0_REG (CLK_RST_CRG_CFG_BASE + 0x118)    /* 全局复位控制寄存器 */
#define RST_MASK 0x10
#define RESET_DEVICE_DELAY_MS 50

#define HOST_WAIT_BOTTOM_INIT_TIMEOUT   1000 // 20000
#define BUS_POWER_PATCH_LAUCH_TIME   5000

reboot_notify g_reboot_func[REBOOT_NOTIFY_COUNT] = {0};
osal_void plat_register_reboot(reboot_notify func, plat_reboot_notify_type type)
{
    if (type < REBOOT_NOTIFY_COUNT) {
        g_reboot_func[type] = func;
    }
}

td_void plat_reboot_system(td_void)
{
    td_u32 i;
    static td_bool prepare_reboot = TD_FALSE;
    if (!prepare_reboot) {
        for (i = 0; i < REBOOT_NOTIFY_COUNT; i++) {
            if (g_reboot_func[i] != TD_NULL) {
                g_reboot_func[i]();
            }
        }
    }
    prepare_reboot = TD_TRUE;
}

/* 通过hcc设置device侧寄存器进行device侧复位 */
td_s32 ws73_board_power_reset_hcc(td_void)
{
    td_s32 ret;
    td_u32 val;

    ret = hcc_read_reg_force(HCC_CHANNEL_AP, CLK_RST_CRG_CFG_GLB_RST_CRG_CTL0_REG, &val);
    if (ret != EOK) {
        oal_print_err("reset_device_by_hcc::hcc_read_reg fail, ret=[%d] \n", ret);
        return ret;
    }
    val |= RST_MASK;
    ret = hcc_write_reg_force(HCC_CHANNEL_AP, CLK_RST_CRG_CFG_GLB_RST_CRG_CTL0_REG, val);
    if (ret != EOK) {
        oal_print_err("reset_device_by_hcc::hcc_write_reg fail, ret=[%d] \n", ret);
        return ret;
    }
    osal_mdelay(RESET_DEVICE_DELAY_MS);

    return EOK;
}

static td_s32 ws73_board_power_on_gpio(td_void)
{
    td_s32 ret;
    td_s32 power_gpio_idx = get_pm_board_info()->power_gpio;
#if defined(CONFIG_GPIO_USING_HI_DRV)
    ret = hi_drv_gpio_write_bit(power_gpio_idx, get_pm_board_info()->power_on_level);
#else
    ret = gpio_direction_output(power_gpio_idx, get_pm_board_info()->power_on_level);
#endif
    if (ret != EOK) {
        oal_print_err("ws73_board_power_on_gpio: set gpio[%d] high fail, ret=[%d] \n", power_gpio_idx, ret);
        return ret;
    }
    osal_mdelay(RESET_DEVICE_DELAY_MS);

    return EOK;
}

static td_s32 ws73_board_power_off_gpio(td_void)
{
    td_s32 ret;
    td_s32 power_gpio_idx = get_pm_board_info()->power_gpio;
#if defined(CONFIG_GPIO_USING_HI_DRV)
    ret = hi_drv_gpio_write_bit(power_gpio_idx, (GPIO_HIGHLEVEL - get_pm_board_info()->power_on_level));
#else
    ret = gpio_direction_output(power_gpio_idx, (GPIO_HIGHLEVEL - get_pm_board_info()->power_on_level));
#endif
    if (ret != EOK) {
        oal_print_err("ws73_board_power_off_gpio: set gpio[%d] low fail, ret=[%d] \n", power_gpio_idx, ret);
        return ret;
    }
    osal_mdelay(RESET_DEVICE_DELAY_MS);

    return EOK;
}
/* 通过拉GPIO进行device侧复位 */
td_s32 ws73_board_power_reset_gpio(td_void)
{
    td_s32 ret;

    ret = ws73_board_power_off_gpio();
    if (ret != EOK) {
        oal_print_err("ws73_board_power_reset_gpio: fail, ret=[%d] \n", ret);
        return ret;
    }

#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
    if (gpio_state_change_uart2cfg() != BOARD_SUCC) {
        oal_print_err("uart2cfg state change fail!\r\n");
        return OAL_FAIL;
    }
#endif

    ret = ws73_board_power_on_gpio();
    if (ret != EOK) {
        oal_print_err("ws73_board_power_reset_gpio: fail, ret=[%d] \n", ret);
        return ret;
    }

#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
    if (gpio_state_change_cfg2uart() != BOARD_SUCC) {
        oal_print_err("cfg2uart state change fail!\r\n");
        return OAL_FAIL;
    }
#endif

    return EOK;
}

/* device上电 */
td_s32 ws73_board_power_on(td_void)
{
    if (get_pm_board_info()->power_gpio_support) {
        return ws73_board_power_on_gpio();
    }
    return PM_BOARD_POWER_SUCCESS;
}

/* device下电 */
td_s32 ws73_board_power_off(td_void)
{
    if (get_pm_board_info()->power_gpio_support) {
        return ws73_board_power_off_gpio();
    } else {
        ws73_board_power_reset_hcc();
        return PM_BOARD_POWER_SUCCESS;
    }
}

/* 复位device */
td_s32 ws73_board_power_reset(td_bool sys_boot)
{
    td_s32 ret;
    if (sys_boot) {
        hcc_flow_ctrl_credit_unregister();
        plat_reboot_system();
    }
    if (get_pm_board_info()->power_gpio_support) {
        ret = ws73_board_power_reset_gpio();
    } else {
        ret = ws73_board_power_reset_hcc();
    }
    if (sys_boot) {
        hcc_deinit(HCC_CHANNEL_AP);
    }
    return ret;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
td_s32 plat_pm_power_action(hcc_bus *pst_bus, td_u32 action, struct pm_drv_data *pm_data)
{
    if (OAL_WARN_ON(pst_bus == TD_NULL)) {
        return EXT_ERR_FAILURE;
    }

    if (action == HCC_BUS_POWER_DOWN) {
        hcc_disable();
    }

    if (HCC_BUS_POWER_PATCH_LOAD_PREPARE == action) {
        /* close hcc */
        hcc_disable();
        init_completion(&pm_data->dev_bsp_ready);
    }

    if (HCC_BUS_POWER_PATCH_LAUCH == action) {
        if (wait_for_completion_timeout(&pm_data->dev_bsp_ready,
            (td_u32)msecs_to_jiffies(HOST_WAIT_BOTTOM_INIT_TIMEOUT)) == 0) {
            oal_print_err("wait bsp ready timeout %d ms ", HOST_WAIT_BOTTOM_INIT_TIMEOUT);

            if (wait_for_completion_timeout(&pm_data->dev_bsp_ready,
                (td_u32)msecs_to_jiffies(BUS_POWER_PATCH_LAUCH_TIME)) == 0) {
                oal_print_err("retry 5 second hold, still timeout");
                return EXT_ERR_FAILURE;
            }
        }
        hcc_enable();
    }

    hcc_bus_power_action(pst_bus, action);
    return EXT_ERR_SUCCESS;
}

/* 强行释放wakelock锁 */
td_void pm_wakelocks_release_detect(struct pm_drv_data *pm_data)
{
    /* before call this function , please make sure the rx/tx queue is empty and no data transfer!! */
    if (OAL_WARN_ON(pm_data == TD_NULL)) {
        return;
    }

    if (oal_wakelock_active(&pm_data->st_wakelock)) {
        pm_data->st_wakelock.lock_count = 1;
        oal_wake_unlock(&pm_data->st_wakelock);
    }
}

td_u32 pm_device_init_ready(td_u8 *param)
{
    struct pm_drv_data *pm_data = (struct pm_drv_data *)param;
    oal_print_err("===wlan READY==\n");
    complete(&pm_data->dev_wlan_ready);
    return OAL_SUCC;
}

td_u32 pm_bsp_init_ready(td_u8 *param)
{
    struct pm_drv_data *pm_data = (struct pm_drv_data *)param;
    oal_print_err("===BSP READY==\n");
    complete(&pm_data->dev_bsp_ready);
    hcc_switch_status(HCC_ON);
    return OAL_SUCC;
}

td_s32 plat_pm_bus_suspend(td_void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (OAL_WARN_ON(pm_data == TD_NULL)) {
        return EXT_ERR_FAILURE;
    }

    if (down_interruptible(&pm_data->sr_wake_sema)) {
        oal_print_err("sdio_wake_sema down failed.");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

td_s32 plat_pm_bus_resume(td_void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (OAL_WARN_ON(pm_data == TD_NULL)) {
        return EXT_ERR_FAILURE;
    }
    up(&pm_data->sr_wake_sema);
    return EXT_ERR_SUCCESS;
}

#elif defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
td_s32 plat_pm_power_action(hcc_bus *pst_bus, td_u32 action, td_u8 *pm_data)
{
    hcc_bus_power_action(pst_bus, action);
    return EXT_ERR_SUCCESS;
}

td_s32 plat_pm_bus_suspend(td_void)
{
    return EXT_ERR_SUCCESS;
}

td_s32 plat_pm_bus_resume(td_void)
{
    return EXT_ERR_SUCCESS;
}

#endif
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static td_s32 firmware_download_enter(td_void)
{
    td_s32 error;
#if defined(CONFIG_HCC_SUPPORT_UART)
    if (hcc_get_channel_bus(HCC_CHANNEL_AP)->bus_type == HCC_BUS_UART) {
        oal_print_err("uart firmware download\n");
        error = uart_download_firmware();
        if (error != EXT_ERR_SUCCESS) {
            oal_print_err("uart firmware download fail, ret %x\n", error);
            return error;
        }
        hcc_uart_set_transfer_mode(1); // 1 非boot阶段
        error = hcc_bus_reinit(hcc_get_channel_bus(HCC_CHANNEL_AP));
        if (error != EXT_ERR_SUCCESS) {
            oal_print_err("hcc reinit fail, ret %x\n", error);
            return error;
        }
    } else {
#endif
        error = firmware_download_function();
        if (error != PM_BOARD_POWER_SUCCESS) {
            oal_print_err("firmware download fail\n");
            error = PM_BOARD_POWER_FIRMWARE_DOWNLOAD_FAIL;
            return error;
        }
#if defined(CONFIG_HCC_SUPPORT_UART)
    }
#endif
    return EXT_ERR_SUCCESS;
}
#endif

static td_s32 ws73_board_reinit_hcc_bus(hcc_bus *pst_bus)
{
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
    if (pst_bus->bus_type == HCC_BUS_UART) {
        hcc_uart_set_transfer_mode(0); // 0 boot阶段
    }
#endif
    if (hcc_bus_reinit(pst_bus) != OAL_SUCC) {
        oal_print_err("hcc reinit failed!\n");
        return OAL_FAIL;
    } else {
        return OAL_SUCC;
    }
}

/* 业务首次加载, 激活device */
td_s32 ws73_board_service_enter(td_void)
{
    td_s32 ret;
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    // 1. 获取管理结构
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        oal_print_err("pm_data is NULL!\n");
        return OAL_FAIL;
    }
    // 2.hcc总线上电, 准备加载firmware
    plat_pm_power_action(pst_bus, HCC_BUS_POWER_UP, pm_data);
    plat_pm_power_action(pst_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE, pm_data);

    // 3.hcc 重新初始化
    if (ws73_board_reinit_hcc_bus(pst_bus) != OAL_SUCC) {
        oal_wake_unlock(&pm_data->st_wakelock);
        return OAL_FAIL;
    }
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    // 2.hcc总线上电, 准备加载firmware
    plat_pm_power_action(pst_bus, HCC_BUS_POWER_UP, TD_NULL);
    plat_pm_power_action(pst_bus, HCC_BUS_POWER_PATCH_LOAD_PREPARE, TD_NULL);
#endif

#ifdef ON_CHIP_LINUX_SDK
    // 4.下载firmware前，先配置cali需要的RAM2 RMA3
    cali_dyn_mem_cfg();
#endif

#if defined(BT_EM_BUFFER_CALI_SUPPORT) && (BT_EM_BUFFER_CALI_SUPPORT == 1)
    // 4.1 配置BT CALI用 EM BUFFER(RAM4) 复用过来的内存
    bsle_cali_dyn_mem_cfg();
#endif

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    // 5.下载firmware到device
    ret = firmware_download_enter();
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }
    // 6.firmware下载完毕, 刷新hcc状态
    ret = plat_pm_power_action(pst_bus, HCC_BUS_POWER_PATCH_LAUCH, pm_data);
#else
    ret = plat_pm_power_action(pst_bus, HCC_BUS_POWER_PATCH_LAUCH, TD_NULL);
#endif
    if (ret != 0) {
        oal_print_err("wlan_poweron HCC_BUS_POWER_PATCH_LAUCH by gpio fail ret=%d", ret);
        return PM_BOARD_POWER_BOOT_UP_FAIL;
    }

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    // 7.刷新状态标志
    pm_data->pst_wlan_pm_info->pm_power_state = POWER_STATE_OPEN;
    // 8.下发平台定制化参数
    hwifi_plat_pm_device_init_custom_param();
    hwifi_config_cmu_xo_trim_temp_comp();
#endif
    init_dev_addr();

    // 开启心跳
    hcc_heartbeat_start();
    hcc_heartbeat_device_enable();

    return PM_BOARD_POWER_SUCCESS;
}

/* 所有业务退出, 复位device */
td_s32 ws73_board_service_exit(td_void)
{
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    // 1.获取管理结构
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        oal_print_err("pm_data is NULL!\n");
        return OAL_FAIL;
    }
#endif

    // 停止心跳
    hcc_heartbeat_stop();
    // 2.HCC置为forbidden
    hcc_switch_status(HCC_BUS_FORBID);

    // 3.HCC POWER_DOWN处理
    hcc_bus_power_action(pst_bus, HCC_BUS_POWER_DOWN);
    osal_msleep(100); // sleep100ms, 等待内核完成资源释放相关处理

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    // 4.刷新状态标志
    pm_data->pst_wlan_pm_info->pm_power_state = POWER_STATE_SHUTDOWN;
#endif
    return OAL_SUCC;
}

EXPORT_SYMBOL(plat_register_reboot);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

