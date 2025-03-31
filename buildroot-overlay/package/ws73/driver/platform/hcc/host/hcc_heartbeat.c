/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: host heartbeat func \n
 *
 * History: \n
 * 2024-09-12, Create file. \n
 */

#include "hcc_heartbeat.h"
#include "oal_debug.h"
#include "hcc_if.h"
#include "hcc.h"
#include "plat_pm_dfr.h"

#if defined(CONFIG_HCC_SUPPORT_HEARTBEAT)

struct hcc_heartbeat_data g_heartbeat_host_data;

static osal_void hcc_heartbeat_timer_callback(uintptr_t data)
{
    osal_s32 ret;

    if (g_heartbeat_host_data.start == OSAL_TRUE) {
        ret = osal_timer_mod(&g_heartbeat_host_data.timer, HCC_HEARTBEAT_TIMER_INTERVAL_MS);
        if (ret != OSAL_SUCCESS) {
            oal_print_err("HCC heartbeat restart timer fail, ret=[%d]!\r\n", ret);
        }

        ret = osal_workqueue_schedule(&g_heartbeat_host_data.workq);
        if (ret != OSAL_TRUE) {
            oal_print_err("HCC heartbeat schedule work fail, ret=[%d]!\r\n", ret);
        }
    }
}

static osal_u32 hcc_heartbeat_ack_callback(td_u8 *data)
{
    g_heartbeat_host_data.check_cnt = 0;
    return OSAL_SUCCESS;
}

static osal_void hcc_heartbeat_work(osal_workqueue *work)
{
    osal_s32 ret;

    if (g_heartbeat_host_data.start == OSAL_FALSE) {
        return;
    }

    if (g_heartbeat_host_data.check_cnt >= HCC_HEARTBEAT_ERROR_THRESHOLD) {
        oal_print_err("HCC wait device heart beat timeout!\r\n");
#if defined(CONFIG_PLAT_SUPPORT_DFR)
        plat_exception_reset_process(OSAL_FALSE);
#endif
        return;
    }

    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HEART_BEAT, HCC_ACTION_TYPE_TEST);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("HCC heartbeat send msg fail, ret=[%d]!\r\n", ret);
    }

    g_heartbeat_host_data.check_cnt++;
}

osal_s32 hcc_heartbeat_device_enable(osal_void)
{
    osal_s32 ret;
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HEARTBEAT_ENABLE, HCC_ACTION_TYPE_TEST);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("HCC heartbeat enable send msg fail, ret=[%d]!\r\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

osal_s32 hcc_heartbeat_device_disable(osal_void)
{
    osal_s32 ret;
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HEARTBEAT_DISABLE, HCC_ACTION_TYPE_TEST);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("HCC heartbeat enable send msg fail, ret=[%d]!\r\n", ret);
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

osal_s32 hcc_heartbeat_start(osal_void)
{
    osal_s32 ret;

    g_heartbeat_host_data.check_cnt = 0;

    osal_mutex_lock(&g_heartbeat_host_data.lock);

    if (g_heartbeat_host_data.start == OSAL_TRUE) {
        oal_print_warning("HCC heartbeat start: already start, return.\r\n");
        osal_mutex_unlock(&g_heartbeat_host_data.lock);
        return OSAL_SUCCESS;
    }

    ret = osal_timer_start(&g_heartbeat_host_data.timer);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("HCC heartbeat start: timer start fail! ret=[%d].\r\n", ret);
        osal_mutex_unlock(&g_heartbeat_host_data.lock);
        return OSAL_FAILURE;
    }

    g_heartbeat_host_data.start = OSAL_TRUE;
    osal_mutex_unlock(&g_heartbeat_host_data.lock);

    osal_printk("HCC heartbeat start: succ!\r\n");
    return OSAL_SUCCESS;
}

osal_void hcc_heartbeat_stop(osal_void)
{
    osal_mutex_lock(&g_heartbeat_host_data.lock);

    if (g_heartbeat_host_data.start == OSAL_FALSE) {
        oal_print_warning("HCC heartbeat stop: already stop, return.\r\n");
        osal_mutex_unlock(&g_heartbeat_host_data.lock);
        return;
    }

    osal_timer_stop(&g_heartbeat_host_data.timer);
    osal_workqueue_flush(&g_heartbeat_host_data.workq);

    osal_mutex_unlock(&g_heartbeat_host_data.lock);
    osal_printk("HCC heartbeat stop: succ!\r\n");

    g_heartbeat_host_data.start = OSAL_FALSE;
}

osal_s32 hcc_heartbeat_init(osal_void)
{
    osal_s32 ret;

    ret = hcc_message_register(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, D2H_MSG_HEART_BEAT_ACK,
        hcc_heartbeat_ack_callback, TD_NULL);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("HCC heartbeat callback register fail! ret=[%d].\r\n", ret);
        goto hcc_msg_register_fail;
    }

    ret = osal_mutex_init(&g_heartbeat_host_data.lock);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("HCC heartbeat mutex init fail! ret=[%d].\r\n", ret);
        goto mutex_init_fail;
    }

    ret = osal_workqueue_init(&g_heartbeat_host_data.workq, hcc_heartbeat_work);
    if (ret != OSAL_SUCCESS) {
        goto work_init_fail;
    }

    g_heartbeat_host_data.timer.handler = hcc_heartbeat_timer_callback;
    g_heartbeat_host_data.timer.data = 0;
    g_heartbeat_host_data.timer.interval = HCC_HEARTBEAT_TIMER_INTERVAL_MS;
    ret = osal_timer_init(&g_heartbeat_host_data.timer);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("HCC heartbeat timer init fail! ret=[%d].\r\n", ret);
        goto timer_init_fail;
    }

    osal_printk("HCC heartbeat init succ!\r\n");
    return OSAL_SUCCESS;

timer_init_fail:
    osal_workqueue_destroy(&g_heartbeat_host_data.workq);
work_init_fail:
    osal_mutex_destroy(&g_heartbeat_host_data.lock);
mutex_init_fail:
    hcc_message_unregister(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, D2H_MSG_HEART_BEAT_ACK);
hcc_msg_register_fail:
    return OSAL_FAILURE;
}

osal_void hcc_heartbeat_deinit(osal_void)
{
    osal_timer_destroy(&g_heartbeat_host_data.timer);
    osal_workqueue_destroy(&g_heartbeat_host_data.workq);
    osal_mutex_destroy(&g_heartbeat_host_data.lock);
    hcc_message_unregister(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, D2H_MSG_HEART_BEAT_ACK);
}
#else

osal_s32 hcc_heartbeat_device_enable(osal_void)
{
    return OSAL_SUCCESS;
}

osal_s32 hcc_heartbeat_device_disable(osal_void)
{
    return OSAL_SUCCESS;
}

osal_s32 hcc_heartbeat_start(osal_void)
{
    return OSAL_SUCCESS;
}

osal_void hcc_heartbeat_stop(osal_void)
{
    return;
}

osal_s32 hcc_heartbeat_init(osal_void)
{
    return OSAL_SUCCESS;
}

osal_void hcc_heartbeat_deinit(osal_void)
{
    return;
}

#endif
