/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: HCC TEST
 * Author: CompanyName
 */

#include "hcc_test.h"
#include "hcc_comm.h"
#include "hcc_if.h"
#include "hcc.h"
#include "hcc_bus.h"
#include "soc_osal.h"
#include "hcc_channel.h"
#include "hcc_adapt.h"
#include "hcc_service.h"
#include "hcc_adapt_uart.h"
#include "securec.h"
#include "osal_adapt.h"

#ifdef CONFIG_HCC_SUPPORT_TEST
#define PKT_LEN_FOR_PERFORMANCE_TEST 1664
#define HCC_TEST_TASK_PRIORITY 4
#define HCC_TEST_TASK_STACK_SIZE 0x800
/* the memory will be exhausted in device when perform the performance test(send more than 40 packets in assemble mode)
 * we will reuse one buf without freeing to prevent allocation failure as we do not care about the contents of the
 * packet  in performance  test  */
static td_u8 *g_hcc_perf_test_buf = TD_NULL;

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
TD_PRV td_void hcc_test_stop_subq(hcc_queue_type queue_id)
{
    uapi_unused(queue_id);
    hcc_test_set_test_stop(TD_TRUE);
    hcc_printf("test stop subq\r\n");
}

TD_PRV td_void hcc_test_start_subq(hcc_queue_type queue_id)
{
    uapi_unused(queue_id);
    hcc_test_set_test_stop(TD_FALSE);
    hcc_printf("test start subq\r\n");
}
#endif

TD_PRV td_u32 hcc_adapt_test_alloc(hcc_queue_type queue_id, td_u32 len, td_u8 **buf, td_u8 **user_param)
{
    uapi_unused(queue_id);
    if (len == PKT_LEN_FOR_PERFORMANCE_TEST && g_hcc_perf_test_buf != TD_NULL) {
        *buf = g_hcc_perf_test_buf;
    } else {
        *buf = (td_u8 *)osal_kmalloc(len, OSAL_GFP_ATOMIC);
        if (*buf == TD_NULL) {
            hcc_printf_err_log("\r\n");
            return EXT_ERR_FAILURE;
        }

        if (len == PKT_LEN_FOR_PERFORMANCE_TEST) {
            g_hcc_perf_test_buf = *buf;
        }
    }

    hcc_printf("adapt alloc - %p\r\n", *buf);
    // 不需要memset成0，外面有赋值
    *user_param = TD_NULL;
    return EXT_ERR_SUCCESS;
}

td_void hcc_adapt_test_free(hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param)
{
    uapi_unused(queue_id);
    uapi_unused(user_param);
    hcc_printf("adapt free - %p\r\n", buf);
    if (buf != g_hcc_perf_test_buf) {
        osal_kfree(buf);
    }
}

hcc_adapt_ops g_hcc_test_adapt = {
    .free = hcc_adapt_test_free,
    .alloc = hcc_adapt_test_alloc,
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    .start_subq = hcc_test_start_subq,
    .stop_subq = hcc_test_stop_subq,
#endif
    .rx_proc = hcc_test_rx_proc,
};

td_u32 hcc_test_msg_rx_proc(td_u8 *data)
{
    uapi_unused(data);
    hcc_debug("hcc test msg\r\n");
    hcc_test();
    return EXT_SUCCESS;
}

TD_PRV td_void hcc_test_fill_frame_content(td_u8 *buf, td_u32 data_size, hcc_transfer_param *param)
{
    td_u32 test_tx_data = hcc_test_get_test_data();
    uapi_unused(param);
    memset_s(buf + sizeof(hcc_header), data_size - sizeof(hcc_header),
             test_tx_data, data_size - sizeof(hcc_header));
}

#ifndef CONFIG_HCC_SUPPORT_NON_OS
TD_PRV int hcc_test_thread_wait_cond(TD_CONST td_void *param)
{
    uapi_unused(param);
    return (osal_atomic_read(&hcc_test_get_test_info()->hcc_test_start) == HCC_TEST_TASK_START)
        || osal_adapt_kthread_should_stop();
}

TD_PRV td_s32 hcc_test_thread(td_void *data)
{
    td_s32 ret = 0;
    uapi_unused(data);
    for (;;) {
        ret = osal_wait_timeout_interruptible(
            &hcc_test_get_test_info()->hcc_test_wq, hcc_test_thread_wait_cond, TD_NULL, OSAL_WAIT_FOREVER);
        if (osal_adapt_kthread_should_stop()) {
            break;
        }

        if (ret == 0) {
            // timeout
            continue;
        }

        hcc_test();
        osal_atomic_set(&hcc_test_get_test_info()->hcc_test_start, HCC_TEST_TASK_STOP);
    }
    hcc_printf("hcc test transfer thread done!\n");
    return EXT_ERR_SUCCESS;
}
#endif

TD_PRV td_void hcc_test_task_init(hcc_test_info *test_info)
{
#ifdef CONFIG_HCC_SUPPORT_NON_OS
    uapi_unused(test_info);
#else
    osal_atomic_set(&test_info->hcc_test_start, HCC_TEST_TASK_STOP);
    if (osal_wait_init(&test_info->hcc_test_wq) != EXT_ERR_SUCCESS) {
        hcc_printf_err_log("failed\n");
        return;
    }

    test_info->hcc_test_thread_handler = osal_kthread_create(
        hcc_test_thread, TD_NULL, "hcc_test", HCC_TEST_TASK_STACK_SIZE);
    if (test_info->hcc_test_thread_handler == TD_NULL) {
        osal_wait_destroy(&test_info->hcc_test_wq);
        return;
    }
    osal_kthread_set_priority(test_info->hcc_test_thread_handler, HCC_TEST_TASK_PRIORITY);
#endif
}

#ifndef CONFIG_HCC_SUPPORT_NON_OS
TD_PRV td_void hcc_test_task_deinit(hcc_test_info *test_info)
{
    osal_kthread_destroy(test_info->hcc_test_thread_handler, TD_TRUE);
    osal_wait_destroy(&test_info->hcc_test_wq);
}
#endif

td_void hcc_test(td_void)
{
    td_u32 data_size = hcc_test_get_frame_size();
    hcc_transfer_param param = {
        .service_type = hcc_test_get_test_service(),
        .sub_type = HCC_TEST_SUBTYPE_DATA,
        .queue_id = hcc_test_get_test_queue_id(),
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
        .fc_flag = HCC_FC_WAIT,
#else
        .fc_flag = HCC_FC_NONE,
#endif
    };

    hcc_handler *hcc = hcc_get_handler(hcc_test_get_test_channel());
    td_u8 *buf = TD_NULL;
    td_u8 *user_param = TD_NULL;
    td_u32 i;
    td_u32 ret;
    td_u32 TD_CONST cnt = hcc_test_get_loop_cnt();
    hcc_debug("hcc_test start:%d\r\n", param.queue_id);
    hcc_test_start();
    if (cnt == 0 || hcc == TD_NULL || data_size < (sizeof(hcc_header) + sizeof(uart_packet_head) + 1)) {
        hcc_printf("test cnt is 0\r\n");
        return;
    }

    for (i = 0; i < cnt;) {
        if (hcc_test_is_test_stop()) {
            osal_udelay(10); // delay 10  us
        }

        ret = hcc_adapt_alloc_priv_buf(hcc, param.queue_id, data_size, &buf, &user_param);
        if (ret != EXT_ERR_SUCCESS) {
            hcc_printf_err_log("malloc user_param ERR\r\n");
            return;
        }
        hcc_printf_null("send cnt: %d - %d, %p, %p\r\n", i, data_size, buf, user_param);

        param.user_param = user_param;
        if (hcc->bus->bus_type == HCC_BUS_UART) {
            hcc_test_fill_frame_content(buf + sizeof(uart_packet_head),
                data_size - sizeof(uart_packet_head) - 1, &param);
        } else {
            hcc_test_fill_frame_content(buf, data_size, &param);
        }
        data_size &= ~(hcc->bus->len_align - 1);
        if (hcc_tx_data(hcc_test_get_test_channel(), buf, data_size, &param) == EXT_ERR_SUCCESS) {
            i++;
        } else {
            hcc_adapt_free_priv_buf(hcc, param.queue_id, buf, user_param);
        }
    }
}

td_void hcc_test_init(td_void)
{
    hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, &g_hcc_test_adapt);
    hcc_test_msg_init();
    hcc_test_task_init(hcc_test_get_test_info());
}

td_void hcc_test_deinit(td_void)
{
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST);
#ifndef CONFIG_HCC_SUPPORT_NON_OS
    hcc_test_task_deinit(hcc_test_get_test_info());
#endif
    if (g_hcc_perf_test_buf != TD_NULL) {
        osal_kfree(g_hcc_perf_test_buf);
        g_hcc_perf_test_buf = TD_NULL;
    }
}
#endif
