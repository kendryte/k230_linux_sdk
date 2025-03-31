/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: platform pm source file
 * Create: 2021-07-28
 */

/*****************************************************************************
  1 Header File Including
*****************************************************************************/
#include <linux/module.h>   /* kernel module definitions */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/irq.h>
#include <linux/mutex.h>
#include <linux/kernel.h>

#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>
#include <linux/tty.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>

#include "plat_pm_wlan.h"
#include "plat_pm_board.h"
#include "plat_debug.h"
#include "plat_firmware.h"
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
#include "plat_firmware_uart.h"
#endif
#include "plat_exception_rst.h"
#include "oam_ext_if.h"
#include "plat_pm.h"

#include "hcc_if.h"
#include "hcc_service.h"
#include "oal_schedule.h"
#include "plat_firmware.h"
#include "plat_hcc_srv.h"
#include "ini.h"

struct pm_drv_data *pm_drv_data_t = NULL;

struct pm_drv_data* pm_get_drvdata(void)
{
    return pm_drv_data_t;
}

static void pm_set_drvdata(struct pm_drv_data * data)
{
    pm_drv_data_t = data;
}

int firmware_download_function(void)
{
    osal_s32 ret = 0;
    unsigned long long total_time = 0;
    ktime_t start_time;
    ktime_t end_time;
    ktime_t trans_time;
    static unsigned long long max_time = 0;
    static unsigned long long count = 0;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if ((pm_data == NULL) || (pm_data->pst_wlan_pm_info->pst_bus == NULL)) {
        oal_print_err("pst_bus is null\n");
        return -FAILURE;
    }
    start_time = ktime_get();
    oal_wake_lock(&pm_data->st_wakelock);
    oal_print_err("firmware_download_etc begin\n");
    ret = firmware_download_etc();
    if (ret < 0) {
        oal_wake_unlock(&pm_data->st_wakelock);
        oal_print_err("firmware download fail!\n");
        return -FAILURE;
    }
    oal_wake_unlock(&pm_data->st_wakelock);

    oal_print_info("firmware_download_etc success\n");
    end_time = ktime_get();
    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    max_time = total_time > max_time ? total_time : max_time;
    count++;
    oal_print_warning("download firmware, count [%llu], current time [%llu]us, max time [%llu]us\n", count, total_time,
        max_time);

    return OAL_SUCC;
}

#ifdef CONFIG_NO_SUPPORT_INI
osal_void hwifi_plat_pm_device_init_custom_param()
{
    device_plt_pm_cfg_stru cfg = {0};
 
    /* 平台低功耗使能配置 */
    cfg.device_plt_pm_enable = CONFIG_INI_DEVICE_PLT_PM_ENABLE;
    cfg.device_awake_host_gpio_idx = CONFIG_INI_DEVICE_AWAKE_HOST_GPIO_IDX;
    cfg.device_awake_host_gpio_level = !!CONFIG_INI_DEVICE_AWAKE_HOST_GPIO_LEVEL;
 
    /* 是否使能device侧USB连接检测特性 */
#if defined(CONFIG_INI_SUPPORT_DEVICE_USB_LINK_CHECK) && (CONFIG_INI_SUPPORT_DEVICE_USB_LINK_CHECK != 0)
    cfg.device_usb_link_check = OSAL_TRUE;
#else
    cfg.device_usb_link_check = OSAL_FALSE;
#endif
    plat_msg_hcc_send((osal_u8 *)&cfg, sizeof(cfg), H2D_PLAT_CFG_MSG_PM_INI);
}
#endif

static oal_int32 low_power_remove(void)
{
    oal_int32 ret = 0;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        oal_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    osal_mutex_destroy(&pm_data->host_mutex);
    complete_all(&pm_data->dev_bsp_ready);
    reinit_completion(&pm_data->dev_bsp_ready);
    oal_wake_lock_exit(&pm_data->st_wakelock);

    wlan_pm_exit_etc();

    pm_set_drvdata(NULL);

    // free platform driver data struct
    kfree(pm_data);

    return ret;
}

static oal_int32 low_power_probe(void)
{
    struct pm_drv_data  *pm_data = NULL;
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    pm_data = kzalloc(sizeof(struct pm_drv_data), GFP_KERNEL);
    if (pm_data == NULL) {
        oal_print_err("no mem to allocate pm_data\n");
        goto PMDATA_MALLOC_FAIL;
    }
    pm_set_drvdata(pm_data);
    pm_data->pst_wlan_pm_info = wlan_pm_init_etc();
    if (pm_data->pst_wlan_pm_info == NULL) {
        oal_print_err("no mem to allocate wlan_pm_info\n");
        goto WLAN_INIT_FAIL;
    }
    pm_data->firmware_cfg_init_flag  = 0;
    pm_data->board  = get_pm_board_info();

    sema_init(&pm_data->sr_wake_sema, 1);
    if (pst_bus != TD_NULL && pst_bus->bus_ops != TD_NULL && pst_bus->bus_ops->pm_notify_register != TD_NULL) {
        pst_bus->bus_ops->pm_notify_register(pst_bus, plat_pm_bus_suspend, plat_pm_bus_resume);
    }

    oal_wake_lock_init(&pm_data->st_wakelock, "plat_lock");
    init_completion(&pm_data->dev_bsp_ready);
    /* init mutex */
    osal_mutex_init(&pm_data->host_mutex);
    return OAL_SUCC;
WLAN_INIT_FAIL:
    kfree(pm_data);
    pm_set_drvdata(NULL);
PMDATA_MALLOC_FAIL:
    return -ENOMEM;
}

oal_int32 low_power_init_etc(void)
{
    oal_int32 ret = -OAL_EFAIL;

    // 平台低功耗初始化
    ret = low_power_probe();
    if (ret != OAL_SUCC) {
        oal_print_err("low_power_init_etc: low_power_probe fail\n");
        return ret;
    }

    // 动态调频初始化
    ret = pm_crg_init();
    if (ret != OAL_SUCC) {
        oal_print_err("low_power_init_etc: pm_crg_init fail\n");
        low_power_remove();
        return ret;
    }
    pm_crg_enable_switch(OAL_TRUE);

    return OAL_SUCC;
}


void  low_power_exit_etc(void)
{
    pm_crg_enable_switch(OAL_FALSE);
    pm_crg_deinit();
    low_power_remove();
}

#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
osal_s32 pm_device_detect_uart(osal_void)
{
    osal_s32 ret;
    pm_board_info* bd_info;

    bd_info = get_pm_board_info();
    if (unlikely(bd_info == NULL)) {
        oal_print_err("board info is err!\r\n");
        return OSAL_FAILURE;
    }

    ret = bd_info->bd_ops.board_power_reset(TD_FALSE);
    if (ret != PM_BOARD_POWER_SUCCESS) {
        oal_print_err("board_power_reset fail, ret=[%d]\r\n", ret);
        return ret;
    }

    ret = uart_device_detect();
    if (ret != OSAL_SUCCESS) {
        oal_print_err("uart device detect fail!\r\n");
        return ret;
    }

    oal_print_warning("uart device detect succ!\r\n");
    return OSAL_SUCCESS;
}
#endif

/* 平台管理业务启停 begin */
osal_s32 pm_svc_power_on(void)
{
    osal_s32 error = PM_BOARD_POWER_SUCCESS;
    unsigned long long total_time = 0;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_download_time = 0;
    static unsigned long long num = 0;
    pm_board_info* bd_info = NULL;

    oal_print_info("wlan power on!\n");

    bd_info = get_pm_board_info();
    if (unlikely(bd_info == NULL)) {
        oal_print_err("board info is err\n");
        return OAL_FAIL;
    }
    start_time = ktime_get();

#if defined(CONFIG_SUPPORT_RESET_DEVICE_IN_INSMOD) && (CONFIG_SUPPORT_RESET_DEVICE_IN_INSMOD != 0)
    // 单板复位处理
    error = bd_info->bd_ops.board_power_reset(TD_FALSE);
    if (error != PM_BOARD_POWER_SUCCESS) {
        oal_print_err("board_power_reset fail, error=[%d]\r\n", error);
        return error;
    }
#else
    // 单板上电处理
    error = bd_info->bd_ops.board_power_on_etc();
    if (error != PM_BOARD_POWER_SUCCESS) {
        oal_print_err("board_power_on_fail, error=%d \n", error);
        return error;
    }
#endif

#if defined(CONFIG_SUPPORT_HCC_CONN_CHECK) && (CONFIG_SUPPORT_HCC_CONN_CHECK != 0)
    // HCC通路检测
    error = hcc_connect_state_check();
    if (error == OAL_SUCC) {
        // 检测成功
        oal_print_warning("HCC connection check succ!\r\n");
    } else {
        // 检测失败, 尝试复位73
        oal_print_warning("HCC connection check fail!\r\n");
        hcc_bus_reinit(hcc_get_channel_bus(HCC_CHANNEL_AP));
        hcc_switch_status(HCC_BUS_FORBID);
        bd_info->bd_ops.board_power_reset(TD_FALSE);
    }
#endif

    // 调用 wlan_power_on_etc 进行首次业务启动处理
    error = bd_info->bd_ops.wlan_power_on_etc();
    if (error != PM_BOARD_POWER_SUCCESS) {
        oal_print_err("wifi_power_on_fail, error=%d \n", error);
        return error;
    }

    // 保留时间统计
    end_time = ktime_get();
    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    if (total_time > max_download_time) {
        max_download_time = total_time;
    }

    num++;
    oal_print_warning("power on, count [%llu], current time [%llu]us, max time [%llu]us\n", num, total_time,
        max_download_time);
    return OAL_SUCC;
}
osal_module_export(pm_svc_power_on);

osal_s32 pm_svc_power_off(void)
{
    osal_s32 error;
    pm_board_info* bd_info = NULL;
    oal_print_info("wlan power off!\n");
    bd_info = get_pm_board_info();
    if (unlikely(bd_info == NULL)) {
        oal_print_err("board info is err\n");
        return -OAL_EFAIL;
    }

    // 业务关闭处理
    error = bd_info->bd_ops.wlan_power_off_etc();
    if (error != OAL_SUCC) {
        oal_print_err("wifi_power_off_fail, error=%d \n", error);
        return error;
    }

    // 单板下电处理
    error = bd_info->bd_ops.board_power_off_etc();
    if (error != PM_BOARD_POWER_SUCCESS) {
        oal_print_err("board_power_off_fail, error[%d] \n", error);
        return error;
    }

    return OAL_SUCC;
}
osal_module_export(pm_svc_power_off);

osal_s32 pm_svc_power_reset(void)
{
    osal_s32 error;
    pm_board_info* bd_info = NULL;
    oal_print_info("wlan power off!\n");
    bd_info = get_pm_board_info();
    if (unlikely(bd_info == NULL)) {
        oal_print_err("board info is err\n");
        return -OAL_EFAIL;
    }

    // 业务关闭处理
    error = bd_info->bd_ops.wlan_power_off_etc();
    if (error != OAL_SUCC) {
        oal_print_err("wifi_power_off_fail, error=%d \n", error);
        return error;
    }

    // 单板复位处理
    error = bd_info->bd_ops.board_power_reset(OSAL_FALSE);
    if (error != PM_BOARD_POWER_SUCCESS) {
        oal_print_err("board_power_reset_fail, error[%d] \n", error);
        return error;
    }

    return OAL_SUCC;
}
osal_module_export(pm_svc_power_reset);

/* 动态调频相关, 决策搬移到host侧进行 */
oal_uint32 plat_msg_hcc_send(oal_uint8 *buf, oal_uint16 len, oal_uint16 hcc_plat_msg_id);

pm_crg_info_stru g_pm_crg_info = {0};
osal_spinlock g_pm_crg_lock;

/* 下发消息调整当前频率档位 */
oal_uint32 pm_crg_send_freq(oal_uint8 freq_lvl)
{
    return plat_msg_hcc_send(&freq_lvl, sizeof(freq_lvl), H2D_PLAT_CFG_MSG_PM_CRG_VOTE);
}

/* 业务投票切换频率内部接口, 未加锁, 非平台代码不要调用 */
oal_uint32 pm_crg_switch_to_freq(oal_uint8 sid, oal_uint8 freq_lvl)
{
    oal_uint8 max_req_freq = PM_CRG_FREQ_LVL_60M;
    oal_uint32 idx;

    // 检查sid合法性
    if (sid >= PM_FSM_SVC_NUM) {
        return OAL_FAIL;
    }

    // 检查频率合法性
    if (freq_lvl > PM_CRG_FREQ_LVL_240M) {
        // 不允许投票TCXO时钟和60/120/240MHz以外的档位
        return OAL_FAIL;
    }

    // 记录频率档位投票
    if (g_pm_crg_info.pm_crg_req_freq[sid] == freq_lvl) {
        // 与当前频率相同, 返回
        return OAL_SUCC;
    }

    // 更新业务档位投票, 并刷新最大档位投票值
    g_pm_crg_info.pm_crg_req_freq[sid] = freq_lvl;
    for (idx = PM_FSM_SVC_BEGIN; idx < PM_FSM_SVC_END; idx++) {
        max_req_freq =
            g_pm_crg_info.pm_crg_req_freq[idx] > max_req_freq ? g_pm_crg_info.pm_crg_req_freq[idx] : max_req_freq;
    }

    // 若最大频率发生变化,发送消息切换频率
    if (g_pm_crg_info.pm_crg_max_req_freq == max_req_freq) {
        return OAL_SUCC;
    }

    g_pm_crg_info.pm_crg_max_req_freq = max_req_freq;

    // 若动态调频开关打开, 则下发消息
    if (g_pm_crg_info.pm_crg_enable) {
        g_pm_crg_info.pm_crg_cur_freq = g_pm_crg_info.pm_crg_max_req_freq;
        pm_crg_send_freq(g_pm_crg_info.pm_crg_cur_freq);
    }

    return OAL_SUCC;
}

/* 业务调频投票接口 */
oal_uint32 pm_crg_svc_vote(oal_uint8 sid, oal_uint8 freq_lvl)
{
    oal_uint32 ret;

    if (g_pm_crg_info.pm_crg_enable != OAL_TRUE) {
        oam_error_log0(0, 0, "pm_crg_svc_vote: pm_crg not enabled, return!");
        return OAL_FAIL;
    }

    osal_spin_lock(&g_pm_crg_lock);
    ret = pm_crg_switch_to_freq(sid, freq_lvl);
    osal_spin_unlock(&g_pm_crg_lock);
    return ret;
}

/* 动态调频开关控制 */
oal_void pm_crg_enable_switch(oal_bool_enum_uint8 is_enable)
{
    oam_warning_log2(0, 0, "pm_crg_enable_switch: set crg state [%u] to [%u], ",
        g_pm_crg_info.pm_crg_enable, OAL_TRUE);

    if (g_pm_crg_info.pm_crg_enable == is_enable) {
        return;
    }

    if (is_enable) {
        // 开启动态调频开关, 检查当前频率并触发一次切频
        if (g_pm_crg_info.pm_crg_cur_freq != g_pm_crg_info.pm_crg_max_req_freq) {
            g_pm_crg_info.pm_crg_cur_freq = g_pm_crg_info.pm_crg_max_req_freq;
            pm_crg_send_freq(g_pm_crg_info.pm_crg_cur_freq);
        }
        g_pm_crg_info.pm_crg_enable = OAL_TRUE;
    } else {
        // 关闭动态调频开关, 置位即可
        g_pm_crg_info.pm_crg_enable = OAL_FALSE;
    }
}

/* 动态调频初始化 */
oal_int32 pm_crg_init(oal_void)
{
    oal_int32 ret = osal_spin_lock_init(&g_pm_crg_lock);
    if (ret != OSAL_SUCCESS) {
        return OAL_FAIL;
    }

    g_pm_crg_info.pm_crg_enable = OAL_FALSE;
    g_pm_crg_info.pm_crg_max_req_freq = PM_CRG_FREQ_LVL_120M;
    g_pm_crg_info.pm_crg_cur_freq = PM_CRG_FREQ_LVL_120M;
    g_pm_crg_info.rsv1 = 0;

    g_pm_crg_info.pm_crg_req_freq[PM_FSM_SVC_WLAN] = PM_CRG_FREQ_LVL_120M;
    g_pm_crg_info.pm_crg_req_freq[PM_FSM_SVC_BT] = PM_CRG_FREQ_LVL_60M;
    g_pm_crg_info.pm_crg_req_freq[PM_FSM_SVC_PLT] = PM_CRG_FREQ_LVL_60M;

    g_pm_crg_info.rsv2 = 0;

    return OAL_SUCC;
}

oal_void pm_crg_deinit(oal_void)
{
    osal_spin_lock_destroy(&g_pm_crg_lock);
}

EXPORT_SYMBOL(pm_crg_svc_vote);
EXPORT_SYMBOL(pm_crg_enable_switch);