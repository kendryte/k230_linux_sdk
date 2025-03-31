/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: host platform wlan source file
 * Author: Huanghe
 * Create: 2021-06-08
 */

/*****************************************************************************
  1 Header File Including
*****************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/irq.h>

#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>
#include <linux/pm_wakeup.h>

#include "hcc_usb_host.h"
#include "hcc_sdio_host.h"
#include "hcc.h"
#include "hcc_bus.h"

#include "plat_debug.h"
#include "plat_cali.h"
#include "plat_pm_board.h"
#include "plat_pm_board_ws73.h"
#include "plat_pm.h"
#include "plat_pm_dfr.h"
#include "plat_hcc_srv.h"
#include "oam_ext_if.h"
#include "hcc_if.h"
#include "hcc_cfg_comm.h"
#include "customize_bsle_ext.h"
#include "plat_pm_dfr.h"
#include "plat_pm_wlan.h"
#include "hcc_heartbeat.h"
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#include "diag_sample_data.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "customize_wifi.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_PM_WLAN_C

static osal_bool g_pm_is_bsle_open_close_cmd_enable = OSAL_TRUE;

struct wlan_pm_s* g_wlan_pm_info = OAL_PTR_NULL;

pm_wlan_suspend_cb g_pm_wifi_suspend_cb_host = NULL;
pm_wlan_resume_cb g_pm_wifi_resume_cb_host = NULL;

pm_ble_suspend_cb g_pm_ble_suspend_cb_host = NULL;
pm_ble_resume_cb g_pm_ble_resume_cb_host = NULL;
pm_bsle_enable_reply_cb g_pm_ble_enable_reply_cb_host = NULL;

pm_sle_suspend_cb g_pm_sle_suspend_cb_host = NULL;
pm_sle_resume_cb g_pm_sle_resume_cb_host = NULL;
pm_bsle_enable_reply_cb g_pm_sle_enable_reply_cb_host = NULL;

static pm_wow_wkup_cb g_pm_wow_wkup_cb_host = NULL;

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static RAW_NOTIFIER_HEAD(wifi_pm_chain);
#endif

#if defined(_PRE_MULTI_CORE_MODE) && defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_uint8 g_uc_custom_cali_done_etc = OAL_FALSE;
EXPORT_SYMBOL_GPL(g_uc_custom_cali_done_etc);
#endif
#endif
#endif
oal_bool_enum g_wlan_pm_switch_etc = OSAL_TRUE;
EXPORT_SYMBOL_GPL(g_wlan_pm_switch_etc);
oal_uint8 g_wlan_fast_check_cnt = 0x6;
EXPORT_SYMBOL_GPL(g_wlan_fast_check_cnt);

void pm_resume_process_host_work_etc(struct work_struct *work);
osal_s32 pm_svc_open(osal_u8 service, osal_bool is_insmod);
osal_s32 pm_svc_close(osal_u8 service, osal_u8 is_rmmod);

#define PLAT_PM_STR_TYPE_INI_STRING "str_lpm_mode"
#define PM_STR_TYPE_DEFAULT    0     // 73 power off
#define PM_STR_TYPE_LPM        1     // 73 enter low power mode
td_s32 g_pm_str_device_type = PM_STR_TYPE_DEFAULT;

static td_u32 plat_custom_get_str_type(td_void)
{
#ifndef CONFIG_NO_SUPPORT_INI
    td_u32 ret = EXT_ERR_SUCCESS;
    td_char ini_string[DOWNLOAD_CHANNEL_LEN] = {0};

    // 获取
    ret = (td_u32)get_cust_conf_string_etc(INI_MODU_PLAT, PLAT_PM_STR_TYPE_INI_STRING, ini_string,
        DOWNLOAD_CHANNEL_LEN);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("get ini_string[%s] custom configuration failed!\n", PLAT_PM_STR_TYPE_INI_STRING);
        return EXT_ERR_FAILURE;
    }
    g_pm_str_device_type = oal_strtol(ini_string, NULL, 10); /* 10 进制 */
#endif
    return EXT_ERR_SUCCESS;
}

static td_bool plat_check_str_lpm_type(td_void)
{
    return (g_pm_str_device_type == PM_STR_TYPE_LPM);
}

/* 获取全局wlan结构 */
struct wlan_pm_s*  wlan_pm_get_drv_etc(oal_void)
{
    return g_wlan_pm_info;
}
EXPORT_SYMBOL_GPL(wlan_pm_get_drv_etc);

void wlan_pm_para_init(struct wlan_pm_s *pst_wlan_pm)
{
    if (pst_wlan_pm == NULL) {
        return;
    }

    /* 初始化芯片自检work */
    pst_wlan_pm->pm_enable = OAL_FALSE;
    pst_wlan_pm->pm_power_state = POWER_STATE_SHUTDOWN;
    pst_wlan_pm->pm_svc_state[PM_SVC_WLAN] = PM_SVC_STATE_SHUTDOWN;
    pst_wlan_pm->pm_svc_state[PM_SVC_BLE] = PM_SVC_STATE_SHUTDOWN;
    pst_wlan_pm->pm_svc_state[PM_SVC_SLE] = PM_SVC_STATE_SHUTDOWN;
    pst_wlan_pm->wlan_state = WLAN_STATE_SHUTDOWN;

    plat_custom_get_str_type();
}

/* WLAN PM初始化接口 */
struct wlan_pm_s* wlan_pm_init_etc(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm;
    oal_int32 ret;
    pm_board_info *pst_board = get_pm_board_info();

    if (pst_board == OAL_PTR_NULL) {
        oal_print_err("wlan_pm_init_etc, get board info failed");
        return OAL_PTR_NULL;
    }
    pst_wlan_pm = kzalloc(sizeof(struct wlan_pm_s), GFP_KERNEL);
    if (pst_wlan_pm == NULL) {
        oal_print_err("no mem to allocate wlan_pm_data");
        return OAL_PTR_NULL;
    }
    pst_wlan_pm->pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    if (pst_wlan_pm->pst_bus == NULL) {
        oal_print_err("hcc bus is NULL, failed to create wlan_pm_wq!");
        kfree(pst_wlan_pm);
        return OAL_PTR_NULL;
    }

    ret = osal_spin_lock_init(&pst_wlan_pm->st_irq_lock);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("osal_spin_lock_init failed");
        kfree(pst_wlan_pm);
        return OAL_PTR_NULL;
    }

    g_wlan_pm_info = pst_wlan_pm;

    // 注册GPIO唤醒回调
    pm_board_wkup_gpio_register(pm_wkup_process_host);

    wlan_pm_para_init(pst_wlan_pm);

    return  pst_wlan_pm;
}

/* WLAN pm退出接口 */
oal_uint wlan_pm_exit_etc(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_SUCC;
    }
    osal_spin_lock_destroy(&pst_wlan_pm->st_irq_lock);
    kfree(pst_wlan_pm);
    g_wlan_pm_info = OAL_PTR_NULL;
    return OAL_SUCC;
}

/* wifi是否上电 */
oal_uint32 wlan_pm_is_poweron_etc(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    return (pst_wlan_pm->pm_power_state == POWER_STATE_OPEN);
}
EXPORT_SYMBOL_GPL(wlan_pm_is_poweron_etc);

/* 获取回调handler指针 */
struct wifi_srv_callback_handler* wlan_pm_get_wifi_srv_handler_etc(oal_void)
{
    return OSAL_NULL;
}
EXPORT_SYMBOL_GPL(wlan_pm_get_wifi_srv_handler_etc);

wlan_customize_init_cb g_wlan_cfg_init_cb = NULL;
void wlan_cfg_init_cb_reg(wlan_customize_init_cb cb_cfg)
{
    g_wlan_cfg_init_cb = cb_cfg;
}

void wlan_cfg_init_cb_unreg(void)
{
    g_wlan_cfg_init_cb = NULL;
}

/*
  wlan业务开启:
  1、如果WCPU没有开启则开启WCPU
  2、进行业务侧的配置等
*/
oal_int32 wlan_open_data_cfg(oal_void)
{
    custom_cali_func cali_func = NULL;
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    struct pm_drv_data *pm_data = pm_get_drvdata();
    struct wlan_pm_s    *pst_wlan_pm = wlan_pm_get_drv_etc();
    if ((pm_data == NULL) || (pst_bus == NULL) || (pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open_data_cfg::get hcc bus failed!");
        return OAL_FAIL;
    }
    /* 初始化配置定制化参数 */
    if (g_wlan_cfg_init_cb != NULL) {
        if (g_wlan_cfg_init_cb() != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_CFG, "wlan_open_data_cfg::hwifi_hcc_customize_h2d_data_cfg fail");
        }
    } else {
        oam_error_log0(0, OAM_SF_CFG, "g_wlan_cfg_init_cb is NULL");
        return OAL_FAIL;
    }
    init_completion(&pm_data->dev_wlan_ready);
    cali_func = get_custom_cali_func();
    if (cali_func == NULL) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open_data_cfg::NO g_custom_cali_func registered");
        return OAL_FAIL;
    }

    /* 如果校准下发成功则等待device ready；否则继续打开wifi */
    if (cali_func() == OAL_SUCC) {
        if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
            (td_s32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT)) == 0) {
            oam_error_log1(0, OAM_SF_PWR, "wlan_open_data_cfg: timeout retry %d !!!!!!", HOST_WAIT_BOTTOM_WIFI_TIMEOUT);
            hcc_sched_transfer(hcc_get_handler(HCC_CHANNEL_AP));
            if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
                (td_s32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT_QUARTERS)) == 0) {
                oam_error_log0(0, OAM_SF_PWR, "wlan_open_data_cfg::cali timeout again\n");
                pst_wlan_pm->pm_power_state = POWER_STATE_SHUTDOWN;
                return OAL_FAIL;
            }
        }
    }
    return OAL_SUCC;
}

static oal_int32 wlan_open_etc(osal_void)
{
    hcc_bus* pst_bus;
    struct wlan_pm_s    *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if ((pm_data == NULL)||(pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open::pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }

    osal_mutex_lock(&pm_data->host_mutex);
    if (pst_wlan_pm->wlan_state == WLAN_STATE_OPEN) {
        osal_mutex_unlock(&pm_data->host_mutex);
        oal_print_err("%s, already opend\n", __func__);
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    if (pst_bus == NULL) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open::get hcc bus failed!");
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }
    init_completion(&pm_data->dev_wlan_ready);
    wlan_power_open_cmd();

    if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
        (oal_uint32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT)) == 0) {
        oam_error_log1(0, OAM_SF_PWR, "wait device ready timeout... %d ms ", HOST_WAIT_BOTTOM_WIFI_TIMEOUT);
        if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
            (oal_uint32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT_QUARTERS)) == 0) {
            oam_error_log0(0, OAM_SF_PWR, "retry 5 second hold, still timeout");
            osal_mutex_unlock(&pm_data->host_mutex);
            return -OAL_ETIMEDOUT;
        } else {
            /* 强制调度成功，说明有可能是GPIO中断未响应 */
            oam_error_log0(0, OAM_SF_PWR, KERN_WARNING"[E]retry succ, maybe gpio interrupt issue");
        }
    }

    // 在dfr流程中走到此处时hcc为关闭状态, 需要打开(默认流程中此处为开)
    hcc_switch_status(HCC_ON);

    if (wlan_open_data_cfg() == OAL_FAIL) {
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }

    oam_warning_log0(0, OAM_SF_PWR, "wlan_open::wlan_open SUCC!!");
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_POWERUP_EVENT, (oal_void*)pst_wlan_pm); /* powerup chain */

    pst_wlan_pm->wlan_state = WLAN_STATE_OPEN;
    osal_mutex_unlock(&pm_data->host_mutex);

    return OAL_SUCC;
}

oal_int32 wlan_open(oal_void)
{
    oal_int32 ret;

    ret = pm_svc_open(PM_SVC_WLAN, OSAL_FALSE);
    if (ret != OAL_SUCC && ret != OAL_ERR_CODE_ALREADY_OPEN) {
        oam_error_log1(0, OAM_SF_PWR, "wlan_pm_open_etc fail %d!", ret);
        return OAL_FAIL;
    }

    return wlan_open_etc();
}

oal_int32 wlan_open_ex(oal_void)
{
    oal_int32 ret;

    ret = pm_svc_open(PM_SVC_WLAN, OSAL_TRUE);
    if (ret != OAL_SUCC && ret != OAL_ERR_CODE_ALREADY_OPEN) {
        oam_error_log1(0, OAM_SF_PWR, "wlan_pm_open_etc fail %d!", ret);
        return OAL_FAIL;
    }

    return wlan_open_etc();
}
EXPORT_SYMBOL_GPL(wlan_cfg_init_cb_reg);

EXPORT_SYMBOL_GPL(wlan_open);
EXPORT_SYMBOL_GPL(wlan_open_ex);

static osal_u32 wlan_close_etc(osal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();
    custom_cali_func exit_later_func = get_custom_exit_later_func();

    oam_warning_log0(0, OAM_SF_PWR, "wlan_pm_close_etc enter");
    if ((pm_data == NULL) || (pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_close_etc,pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }
    osal_mutex_lock(&pm_data->host_mutex);

    if ((pst_wlan_pm->pm_power_state == POWER_STATE_SHUTDOWN) ||
        (pst_wlan_pm->wlan_state == WLAN_STATE_SHUTDOWN)) {
        osal_mutex_unlock(&pm_data->host_mutex);
        oal_print_err("%s, already closed\n", __func__);
        return OAL_ERR_CODE_ALREADY_CLOSE;
    }

    if (wlan_power_close_cmd() != OAL_SUCC) {
        osal_mutex_unlock(&pm_data->host_mutex);
        oal_print_err("%s, close cmd fail\n", __func__);
        return OAL_FAIL;
    }

    if (exit_later_func != OSAL_NULL) {
        oal_print_err("%s, exit_later_func\n", __func__);
        exit_later_func();
    } else {
        oal_print_err("%s, miss exit_later_func\n", __func__);
    }

    pst_wlan_pm->wlan_state = WLAN_STATE_SHUTDOWN;
    osal_mutex_unlock(&pm_data->host_mutex);

    return OAL_SUCC;
}

/* close the Wi-Fi service and try to close the WCPU. */
oal_uint32 wlan_close(oal_void)
{
    osal_u32 ret;
    ret = wlan_close_etc();
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* try close wcpu */
    pm_svc_close(PM_SVC_WLAN, OSAL_FALSE);
    return OAL_SUCC;
}

oal_uint32 wlan_close_ex(oal_void)
{
    osal_u32 ret;
    ret = wlan_close_etc();
    if (ret != OAL_SUCC && ret != OAL_ERR_CODE_ALREADY_CLOSE) {
        return ret;
    }
    /* try close wcpu */
    pm_svc_close(PM_SVC_WLAN, OSAL_TRUE);
    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(wlan_close);
EXPORT_SYMBOL_GPL(wlan_close_ex);

oal_int32 wlan_power_open_cmd(oal_void)
{
    oal_int32 ret;
    oal_print_err("wlan_power_open_cmd");
    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_OPEN cmd");
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_WLAN_OPEN, HCC_ACTION_TYPE_TEST); /* can wakeup dev */
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_OPEN cmd succ ");
    return OAL_SUCC;
}

oal_int32 wlan_power_close_cmd(oal_void)
{
    oal_int32 ret;

    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_CLOSE cmd");
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_WLAN_CLOSE, HCC_ACTION_TYPE_TEST); /* can wakeup dev */
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_CLOSE cmd succ ");
    return OAL_SUCC;
}

oal_int32 bsle_open_close_cmd(oal_uint8 service, oal_uint8 is_open)
{
    oal_int32 ret;
    oal_uint32 msg_type;

    oal_print_warning("bsle_open_close_cmd, service[%u], type[%u].\r\n", service, is_open);
    if (service != PM_SVC_BLE && service != PM_SVC_SLE) {
        oal_print_err("Invalid service, fail!\r\n");
        return OAL_FAIL;
    }

    if (g_pm_is_bsle_open_close_cmd_enable != OSAL_TRUE) {
        oal_print_err("Cmd not enable, fail!\r\n");
        return OAL_FAIL;
    }

    if (service == PM_SVC_BLE) {
        msg_type = (is_open != 0) ? H2D_MSG_BT_OPEN : H2D_MSG_BT_CLOSE;
    } else {
        msg_type = (is_open != 0) ? H2D_MSG_SLE_OPEN : H2D_MSG_SLE_CLOSE;
    }

    ret = hcc_send_message(HCC_CHANNEL_AP, msg_type, HCC_ACTION_TYPE_TEST);
    if (ret != OAL_SUCC) {
        oal_print_err("hcc send msg fail, ret[%u]!\r\n", ret);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

oal_uint8 pm_svc_open_state_get(oal_uint8 svc_type)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == OSAL_NULL) {
        return PM_SVC_STATE_SHUTDOWN;
    }
    return pst_wlan_pm->pm_svc_state[svc_type];
}

oal_void pm_bsle_open(oal_void)
{
    // ble或sle业务, 只通知一次device启动业务
    /* 初始化配置定制化参数 */
    if (hbsle_hcc_customize_h2d_data_cfg() != OAL_SUCC) {
        hcc_debug("bsle_open_data_cfg::hbsle_hcc_customize_h2d_data_cfg fail\n");
    } else {
        hcc_debug("get bsle custom para data\n");
    }
    if (hbsle_hcc_customize_get_device_status(BSLE_STATUS_CUSTOMIZE_RECEIVED) == false) {
        hcc_debug("customize data not received \n");
    }

#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    // 注册bt数采回调
    diag_sample_data_register(ZDIAG_BSLE_SAMPLE_CB, hcc_adapt_bsle_msg_data_col_cmd_tx_proc);
#endif
}

oal_void pm_bsle_close(oal_void)
{
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BSLE_MSG);
}

oal_int32 pm_init_n_firmware_download(struct pm_drv_data *pm_data, osal_bool *is_first_svc)
{
    oal_int32 ret;
    oal_print_err("pm_svc_open::first svc open, pm_svc_power_on! \n");
    if (!pm_data->st_wakelock.lock_count) {
        /* make sure open only lock once */
        oal_wake_lock(&pm_data->st_wakelock);
    }

    ret = pm_svc_power_on();
    if (ret != OAL_SUCC) {
        // 初始化失败, 返回
        oal_wake_unlock(&pm_data->st_wakelock);
        oal_print_err("pm_svc_open::first svc open, pm_svc_power_on fail, ret=%d \n", ret);
        return ret;
    }
    *is_first_svc = OSAL_TRUE;
    oal_print_warning("pm_svc_open::first svc open, pm_svc_power_on succ! \n");
    init_completion(&pm_data->dev_wlan_ready);
    return OAL_SUCC;
}

/* 平台管理业务启停 begin */
/* 平台新增, 业务开启统一处理接口 */
osal_s32 pm_svc_open(osal_u8 service, osal_bool is_insmod)
{
    osal_bool is_first_svc = OSAL_FALSE;
    oal_int32 ret;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();

    // 用于区分wlan的insmod与业务启停
    if (!is_insmod) {
        oal_print_err("pm_svc_open::not insmod service[%u], return already opened. \n", service);
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    // 判断业务合法性
    if (service >= PM_SVC_NUM) {
        oal_print_err("pm_svc_open::invalid svc [%u] \n", service);
        return OAL_FAIL;
    }

    if ((pm_data == NULL) || (pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_open_etc:pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }

    osal_mutex_lock(&pm_data->host_mutex);

    // 原有流程判断, 暂时保留
    if (hcc_get_channel_bus(HCC_CHANNEL_AP) == NULL) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_open_etc::get hcc bus failed!");
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }

    // 判断该业务是否已经启动
    if (pst_wlan_pm->pm_svc_state[service] == PM_SVC_STATE_OPEN) {
        oal_print_err("pm_svc_open::svc [%u] already opened \n", service);
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    // 判断是否已有业务启动
    // 若为首个业务开启, 先进行初始化和firmware download
    if (!(pst_wlan_pm->pm_svc_state[PM_SVC_WLAN] || pst_wlan_pm->pm_svc_state[PM_SVC_BLE] ||
          pst_wlan_pm->pm_svc_state[PM_SVC_SLE])) {
        ret = pm_init_n_firmware_download(pm_data, &is_first_svc);
        if (ret != OAL_SUCC) {
            osal_mutex_unlock(&pm_data->host_mutex);
            return ret;
        }
#if defined(CONFIG_PLAT_SUPPORT_DFR)
        plat_dfr_enable_set(OSAL_TRUE);
#endif
    }

    // 对BSLE进行初始化之后的处理
    if (service == PM_SVC_WLAN) {
        // wlan业务启动, 通过hcc通知device侧启动业务
        oal_print_err("pm_svc_open::wlan open. \n");
    } else {
        if (!(pst_wlan_pm->pm_svc_state[PM_SVC_BLE] || pst_wlan_pm->pm_svc_state[PM_SVC_SLE])) {
            pm_bsle_open();
        }
        ret = bsle_open_close_cmd(service, OSAL_TRUE);
        if (ret != OAL_SUCC) {
            oal_print_err("pm_svc_open::ble/sle open, send message fail, ret=%d. \n", ret);
            if (is_first_svc) {
                oal_print_err("pm_svc_open::first svc fail, power off \n");
                pm_svc_power_off();
                oal_wake_unlock(&pm_data->st_wakelock);
            }
            osal_mutex_unlock(&pm_data->host_mutex);
            return ret;
        }
    }

    // 业务初始化成功, 更新标志位
    pst_wlan_pm->pm_svc_state[service] = OAL_TRUE;
    oal_wake_unlock(&pm_data->st_wakelock);
    osal_mutex_unlock(&pm_data->host_mutex);
    oal_print_err("pm_svc_open::SUCC! \n");
    return OAL_SUCC;
}

static osal_void pm_svc_close_etc(struct wlan_pm_s *pst_wlan_pm, struct pm_drv_data *pm_data)
{
    osal_s32 error;
    pm_board_info* bd_info = NULL;
    bd_info = get_pm_board_info();
    if (unlikely(bd_info == NULL)) {
        oal_print_err("pm_svc_close_etc board info is err\n");
        return;
    }

    // 业务关闭复位芯片
    error = pm_svc_power_reset();
    if (error != OAL_SUCC) {
        oal_print_err("pm_svc_close_etc wifi_power_off_fail, error=%d \n", error);
        return;
    }

    pst_wlan_pm->pm_power_state = POWER_STATE_SHUTDOWN;
    pst_wlan_pm->wlan_state = WLAN_STATE_SHUTDOWN;
    oal_wake_unlock(&pm_data->st_wakelock);
    pm_wakelocks_release_detect(pm_data);
}

/* 平台新增, 业务关闭统一接口 */
osal_s32 pm_svc_close(osal_u8 service, osal_u8 is_rmmod)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (!is_rmmod) {
        oal_print_err("pm_svc_close::not rmmod svc [%u], return. \n", service);
        return OAL_FAIL;
    }

    // 判断业务合法性
    if (service >= PM_SVC_NUM) {
        oal_print_err("pm_svc_close::invalid svc [%u] \n", service);
        return OAL_FAIL;
    }

    if ((pm_data == NULL) || (pst_wlan_pm == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_close_etc,pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }

    // 关闭流程中上锁后, 除业务已经关闭情况外, 处理失败也继续往下执行完毕再解锁
    osal_mutex_lock(&pm_data->host_mutex);

    // 判断该业务是否已经关闭
    if (pst_wlan_pm->pm_svc_state[service] == PM_SVC_STATE_SHUTDOWN) {
        oal_print_err("pm_svc_close::svc [%u] already closed \n", service);
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_ERR_CODE_ALREADY_CLOSE;
    }

    // 更新业务标志位
    pst_wlan_pm->pm_svc_state[service] = PM_SVC_STATE_SHUTDOWN;
    oal_print_err("pm_svc_close::cur_state: wlan[%u] ble[%u] sle[%u] \n",
        pst_wlan_pm->pm_svc_state[PM_SVC_WLAN],
        pst_wlan_pm->pm_svc_state[PM_SVC_BLE],
        pst_wlan_pm->pm_svc_state[PM_SVC_SLE]);

    // 分业务进行业务关闭处理, 处理失败时继续往下执行
    if (service == PM_SVC_WLAN) {
        oal_print_err("pm_svc_close::wlan close, send message. \n");
    } else {
        bsle_open_close_cmd(service, OSAL_FALSE);
        if (pst_wlan_pm->pm_svc_state[PM_SVC_BLE] == PM_SVC_STATE_SHUTDOWN &&
            pst_wlan_pm->pm_svc_state[PM_SVC_SLE] == PM_SVC_STATE_SHUTDOWN) {
            pm_bsle_close();
        }
    }

    // 所有业务都关闭时对device进行复位
    if (!(pst_wlan_pm->pm_svc_state[PM_SVC_WLAN] ||
          pst_wlan_pm->pm_svc_state[PM_SVC_BLE] ||
          pst_wlan_pm->pm_svc_state[PM_SVC_SLE])) {
        oal_print_err("pm_svc_close::all svc closed, deinit! \n");
#if defined(CONFIG_PLAT_SUPPORT_DFR)
        plat_dfr_enable_set(OSAL_FALSE);
#endif
        pm_svc_close_etc(pst_wlan_pm, pm_data);
    }

    osal_mutex_unlock(&pm_data->host_mutex);
    return OAL_SUCC;
}

osal_s32 pm_ble_open(void)
{
    return pm_svc_open(PM_SVC_BLE, OSAL_TRUE);
}

osal_s32 pm_ble_close(void)
{
    return pm_svc_close(PM_SVC_BLE, OSAL_TRUE);
}

osal_s32 pm_ble_enable(void)
{
    return bsle_open_close_cmd(PM_SVC_BLE, OSAL_TRUE);
}

osal_s32 pm_ble_disable(void)
{
    return bsle_open_close_cmd(PM_SVC_BLE, OSAL_FALSE);
}

osal_s32 pm_sle_open(void)
{
    return pm_svc_open(PM_SVC_SLE, OSAL_TRUE);
}

osal_s32 pm_sle_close(void)
{
    return pm_svc_close(PM_SVC_SLE, OSAL_TRUE);
}

osal_s32 pm_sle_enable(void)
{
    return bsle_open_close_cmd(PM_SVC_SLE, OSAL_TRUE);
}

osal_s32 pm_sle_disable(void)
{
    return bsle_open_close_cmd(PM_SVC_SLE, OSAL_FALSE);
}

static void pm_plt_shutdown_cb_host(void)
{
    osal_s32 ret;
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HOST_SLP, HCC_ACTION_TYPE_TEST);
    if (ret != EOK) {
        oal_print_err("pm_plt_suspend_cb_host: hcc_send_message fail, ret[%d] \r\n", ret);
    }

    osal_mdelay(100); // 避免消息阻塞, 等待100ms

    // 关闭HCC
    hcc_switch_status(HCC_BUS_FORBID);
}

void pm_shutdown_process_host(void)
{
    oal_print_warning("pm_shutdown_process_host! \r\n");
#if defined(CONFIG_PLAT_SUPPORT_DFR)
    wlan_set_dfr_recovery_flag(OSAL_TRUE);
    plat_update_device_recovery_flag(TD_TRUE);
    plat_dfr_enable_set(OSAL_FALSE);
#endif
    hcc_switch_status(HCC_BUS_FORBID);

    pm_plt_shutdown_cb_host();

    if (g_pm_wifi_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend_process_host: call g_pm_wifi_suspend_cb_host(). \r\n");
        g_pm_wifi_suspend_cb_host();
    }
    if (g_pm_ble_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend_process_host: call g_pm_ble_suspend_cb_host(). \r\n");
        g_pm_ble_suspend_cb_host();
    }
    if (g_pm_sle_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend: sle suspend. \r\n");
        g_pm_sle_suspend_cb_host();
    }
}

#if defined(CONFIG_PLAT_SUPPORT_DFR)
static void pm_suspend_dfr_proc(void)
{
    osal_s32 dfr_status;
    plat_dfr_lock();
    dfr_status = plat_dfr_status_get();
    if (dfr_status >= DFR_ATOMIC_STATUS_DFR_PROCESS) {
        plat_dfr_unlock();
        oal_print_err("suspend: dfr is in process, dfr_status[%d], wait.\r\n", dfr_status);
        while (dfr_status >= DFR_ATOMIC_STATUS_DFR_PROCESS) {
            osal_mdelay(100); // 等待100ms，保证处理完成
            plat_dfr_lock();
            dfr_status = plat_dfr_status_get();
            plat_dfr_unlock();
        }
        plat_dfr_lock();
    }
    plat_dfr_status_set(DFR_ATOMIC_STATUS_STR_SUSPEND);
    plat_dfr_unlock();

    wlan_set_dfr_recovery_flag(OSAL_TRUE);
    plat_update_device_recovery_flag(TD_TRUE);
}
#endif

void pm_suspend_service(void)
{
    hcc_switch_status(HCC_BUS_FORBID);

    if (g_pm_wifi_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend: wifi suspend. \r\n");
        g_pm_wifi_suspend_cb_host();
    }
    if (g_pm_ble_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend: ble suspend. \r\n");
        g_pm_ble_suspend_cb_host();
    }
    if (g_pm_sle_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend: sle suspend. \r\n");
        g_pm_sle_suspend_cb_host();
    }
}

void pm_suspend_process_host(void)
{
    osal_s32 ret;

#if defined(CONFIG_PLAT_SUPPORT_DFR)
    pm_suspend_dfr_proc();
#endif

    pm_suspend_service();

    if (plat_check_str_lpm_type()) {
        hcc_heartbeat_stop();
        hcc_heartbeat_device_disable();
        // 发送msg消息让device进入低功耗
        ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HOST_SLP, HCC_ACTION_TYPE_TEST);
    } else {
        ret = pm_svc_power_off();
    }
    if (ret != OAL_SUCC) {
        oal_print_err("pm_suspend type[%d] fail, ret[%d] \r\n", plat_check_str_lpm_type(), ret);
    }
    hcc_switch_status(HCC_BUS_FORBID);

#ifdef CONFIG_STR_SUSPEND_WAIT_USB_DISCONNECT
    hcc_usb_wait_disconnect();
#endif
    oal_print_warning("pm_suspend: success!. \r\n");
}

 osal_void pm_resume_service(osal_bool *dfr_flag)
{
    osal_s32 ret;

    // 重新禁止 hcc发数据
    hcc_switch_status(HCC_BUS_FORBID);
    // dfr自愈流程, 此处强制下发hcc控制消息(默认关),确保后续数据消息顺利下发
    hcc_force_update_queue_id(hcc_get_channel_bus(HCC_CHANNEL_AP), 1);
    if (g_pm_wifi_resume_cb_host != NULL) {
        ret = (osal_s32)g_pm_wifi_resume_cb_host();
        oal_print_warning("pm_resume: wifi resume, ret=[%d].\r\n", ret);
        if (ret != OAL_SUCC) {
            *dfr_flag = OSAL_TRUE;
        }
    }
    if (g_pm_ble_resume_cb_host != NULL) {
        ret = g_pm_ble_resume_cb_host();
        oal_print_warning("pm_resume: ble resume, ret=[%d].\r\n", ret);
        if (ret != OAL_SUCC) {
            *dfr_flag = OSAL_TRUE;
        }
    }
    if (g_pm_sle_resume_cb_host != NULL) {
        ret = g_pm_sle_resume_cb_host();
        oal_print_warning("pm_resume: sle resume, ret=[%d].\r\n", ret);
        if (ret != OAL_SUCC) {
            *dfr_flag = OSAL_TRUE;
        }
    }

    if (*dfr_flag != OSAL_TRUE) {
        hcc_switch_status(HCC_ON);
    }
}

static void plat_pm_resume_service(osal_bool *dfr_flag)
{
    pm_resume_service(dfr_flag);

    oal_print_warning("pm_resume: success!\r\n");
#if defined(CONFIG_PLAT_SUPPORT_DFR)
    plat_update_device_recovery_flag(TD_FALSE);
    plat_dfr_lock();
    plat_dfr_status_set(DFR_ATOMIC_STATUS_IDLE);
    if (plat_dfr_wait_flag_get() == OSAL_TRUE) {
        *dfr_flag = OSAL_TRUE;
        plat_dfr_wait_flag_set(OSAL_FALSE);
    }
    plat_dfr_unlock();

    // 若有处于等待中的DFR任务, 则触发
    if (*dfr_flag == OSAL_TRUE) {
        oal_print_warning("pm_resume work: dfr in waiting, process!\r\n");
        plat_exception_reset_process(OSAL_FALSE);
    }
#endif
}

#ifdef CONFIG_STR_RESUME_ASYNC
struct workqueue_struct *plat_pm_resume_workqueue;
struct work_struct plat_pm_resume_work;
static void plat_pm_resume_work_func(struct work_struct *work)
{
    osal_s32 ret;
    osal_bool dfr_flag = OSAL_FALSE;

    oal_print_warning("plat_pm_resume_work_func async!\r\n");
    ret = ws73_board_service_enter();
    if (ret != PM_BOARD_POWER_SUCCESS) {
        oal_print_err("srv resume fail, error=%d \n", ret);
        dfr_flag = OSAL_TRUE;
    }
    plat_pm_resume_service(&dfr_flag);
}

void pm_create_resume_workqueue(void)
{
    plat_pm_resume_workqueue = create_singlethread_workqueue("plat_pm_resume_queue");
    if (plat_pm_resume_workqueue != OSAL_NULL) {
        INIT_WORK(&plat_pm_resume_work, plat_pm_resume_work_func);
    }
}

void pm_destroy_resume_workqueue(void)
{
    cancel_work_sync(&plat_pm_resume_work);
    destroy_workqueue(plat_pm_resume_workqueue);
}

void pm_resume_process_host_async(void)
{
    osal_s32 ret;
    osal_bool dfr_flag = OSAL_FALSE;
    oal_print_warning("pm_resume_process_host async!\r\n");
    plat_dfr_lock();
    plat_dfr_status_set(DFR_ATOMIC_STATUS_STR_RESUME);
    plat_dfr_unlock();

    if (plat_check_str_lpm_type()) {
        // 发消息给device退出低功耗
        ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HOST_WORK, HCC_ACTION_TYPE_TEST);
        if (ret != EOK) {
            oal_print_err("pm_resume: hcc_send_message fail, ret[%d] \r\n", ret);
            dfr_flag = OSAL_TRUE;
        }
        hcc_heartbeat_start();
        hcc_heartbeat_device_enable();
    } else {
        ret = ws73_board_power_on();
        if (ret != PM_BOARD_POWER_SUCCESS) {
            oal_print_err("resume board_power_on_fail, error=%d \n", ret);
            dfr_flag = OSAL_TRUE;
        } else {
            queue_work(plat_pm_resume_workqueue, &plat_pm_resume_work);
            return;
        }
    }
    plat_pm_resume_service(&dfr_flag);
}
#else
void pm_resume_process_host(void)
{
    osal_s32 ret;
    osal_bool dfr_flag = OSAL_FALSE;
    oal_print_warning("pm_resume_process_host!\r\n");
#if defined(CONFIG_PLAT_SUPPORT_DFR)
    plat_dfr_lock();
    plat_dfr_status_set(DFR_ATOMIC_STATUS_STR_RESUME);
    plat_dfr_unlock();
#endif
    if (plat_check_str_lpm_type()) {
        // 发消息给device退出低功耗
        ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HOST_WORK, HCC_ACTION_TYPE_TEST);
        hcc_heartbeat_start();
        hcc_heartbeat_device_enable();
    } else {
        // 下载bin
        ret = pm_svc_power_on();
    }
    if (ret != OAL_SUCC) {
        oal_print_err("pm_resume type[%d] fail, ret[%d] \r\n", plat_check_str_lpm_type(), ret);
        dfr_flag = OSAL_TRUE;
    }

    plat_pm_resume_service(&dfr_flag);
}
#endif

osal_void pm_wifi_suspend_cb_host_register(pm_wlan_suspend_cb cb)
{
    g_pm_wifi_suspend_cb_host = cb;
}

osal_void pm_wifi_resume_cb_host_register(pm_wlan_resume_cb cb)
{
    g_pm_wifi_resume_cb_host = cb;
}

osal_void pm_ble_suspend_cb_host_register(pm_ble_suspend_cb cb)
{
    g_pm_ble_suspend_cb_host = cb;
}

osal_void pm_ble_resume_cb_host_register(pm_ble_resume_cb cb)
{
    g_pm_ble_resume_cb_host = cb;
}

osal_void pm_ble_enable_reply_cb_host_register(pm_bsle_enable_reply_cb cb)
{
    g_pm_ble_enable_reply_cb_host = cb;
}

pm_bsle_enable_reply_cb pm_ble_enable_reply_cb_host_get(osal_void)
{
    return g_pm_ble_enable_reply_cb_host;
}

osal_void pm_sle_suspend_cb_host_register(pm_sle_suspend_cb cb)
{
    g_pm_sle_suspend_cb_host = cb;
}

osal_void pm_sle_resume_cb_host_register(pm_sle_resume_cb cb)
{
    g_pm_sle_resume_cb_host = cb;
}

osal_void pm_sle_enable_reply_cb_host_register(pm_bsle_enable_reply_cb cb)
{
    g_pm_sle_enable_reply_cb_host = cb;
}

pm_bsle_enable_reply_cb pm_sle_enable_reply_cb_host_get(osal_void)
{
    return g_pm_sle_enable_reply_cb_host;
}

osal_s32 pm_wkup_process_host(osal_void)
{
    oal_print_warning("pm_wkup_process_host! \r\n");
    if (g_pm_wow_wkup_cb_host != NULL) {
        g_pm_wow_wkup_cb_host();
    }
    return OSAL_SUCCESS;
}

osal_void pm_wifi_wkup_cb_host_register(pm_wow_wkup_cb cb)
{
    g_pm_wow_wkup_cb_host = cb;
}

#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
osal_s32 uapi_pm_device_detect(osal_void)
{
#if defined(CONFIG_PLAT_UART_IO_CFG) && (CONFIG_PLAT_UART_IO_CFG != 0)
    osal_s32 ret;

    if (wlan_pm_is_poweron_etc() == OSAL_TRUE) {
        return OSAL_SUCCESS;
    }

    ret = pm_device_detect_uart();
    if (ret != OSAL_SUCCESS) {
        return ret;
    }

    oal_print_warning("UART device detect succ!\r\n");
    return OSAL_SUCCESS;
#else
    oal_print_warning("Not support UART IO CFG, fail!\r\n");
    return OSAL_FAILURE;
#endif
}
#endif

EXPORT_SYMBOL_GPL(pm_svc_open);
EXPORT_SYMBOL_GPL(pm_svc_close);
EXPORT_SYMBOL_GPL(pm_ble_open);
EXPORT_SYMBOL_GPL(pm_ble_close);
EXPORT_SYMBOL_GPL(pm_ble_enable);
EXPORT_SYMBOL_GPL(pm_ble_disable);
EXPORT_SYMBOL_GPL(pm_sle_open);
EXPORT_SYMBOL_GPL(pm_sle_close);
EXPORT_SYMBOL_GPL(pm_sle_enable);
EXPORT_SYMBOL_GPL(pm_sle_disable);
EXPORT_SYMBOL_GPL(pm_wifi_suspend_cb_host_register);
EXPORT_SYMBOL_GPL(pm_wifi_resume_cb_host_register);
EXPORT_SYMBOL_GPL(pm_ble_suspend_cb_host_register);
EXPORT_SYMBOL_GPL(pm_ble_resume_cb_host_register);
EXPORT_SYMBOL_GPL(pm_ble_enable_reply_cb_host_register);
EXPORT_SYMBOL_GPL(pm_sle_suspend_cb_host_register);
EXPORT_SYMBOL_GPL(pm_sle_resume_cb_host_register);
EXPORT_SYMBOL_GPL(pm_sle_enable_reply_cb_host_register);
EXPORT_SYMBOL_GPL(pm_wifi_wkup_cb_host_register);
#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
EXPORT_SYMBOL_GPL(uapi_pm_device_detect);
#endif
/* 平台管理业务启停 end */
