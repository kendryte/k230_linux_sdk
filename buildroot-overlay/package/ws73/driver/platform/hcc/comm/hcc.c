/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc module completion.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#include "td_base.h"
#include "securec.h"

#include "soc_module.h"
#include "soc_osal.h"
#include "osal_def.h"
#include "soc_errno.h"
#include "hcc_if.h"
#include "hcc_list.h"
#include "hcc_adapt.h"
#include "hcc_cfg.h"
#include "hcc_cfg_if.h"
#include "hcc_comm.h"
#include "hcc_bus.h"
#include "hcc_bus_types.h"
#include "hcc_flow_ctrl.h"
#include "hcc_channel.h"
#include "hcc_service.h"
#include "hcc_test.h"
#include "hcc_dfx.h"
#include "hcc_adapt_uart.h"
#include "hcc.h"
#include "hcc_heartbeat.h"

#define HCC_PROC_FAIL_MAX_NUM 50

td_s8 g_loglevel[BUS_LOG_SYMBOL_NUM][BUS_LOG_SYMBOL_SIZE] = { "[E]", "[W]", "[I]", "[D]", "[V]" };
td_u32 g_hcc_proc_fail_count = 0;
static td_bool g_hcc_exception_reset_work = TD_FALSE;

#if defined(CONFIG_PLAT_SUPPORT_DFR)
osal_u32 plat_exception_reset_process(osal_bool should_dump_trace);
#endif

td_void hcc_exception_reset_work(td_void)
{
    if (!g_hcc_exception_reset_work) {
#if defined(CONFIG_PLAT_SUPPORT_DFR)
        plat_exception_reset_process(OSAL_FALSE);
#endif
    }
}

td_void hcc_set_exception_status(td_bool status)
{
    g_hcc_exception_reset_work = status;
}

td_bool hcc_get_exception_status(td_void)
{
    return g_hcc_exception_reset_work;
}

td_void hcc_proc_fail_count_add(td_void)
{
    // 不具体关注数值, 不需要加锁
    g_hcc_proc_fail_count++;
    if (g_hcc_proc_fail_count >= HCC_PROC_FAIL_MAX_NUM) {
        hcc_printf_err_log("hcc proc fail count[%u] >= max num[%u], enter dfr!\r\n",
            g_hcc_proc_fail_count, HCC_PROC_FAIL_MAX_NUM);
        g_hcc_proc_fail_count = 0;
        hcc_exception_reset_work();
    }
}

td_void hcc_proc_fail_count_clear(td_void)
{
    g_hcc_proc_fail_count = 0;
}

td_u16 hcc_get_head_len(td_void)
{
    return sizeof(hcc_header);
}

td_void hcc_enable_rx_thread(hcc_channel_name chl, td_bool enable)
{
    if (hcc_get_handler(chl) != TD_NULL) {
        hcc_printf("enable/disable rx thread: %d\n", enable);
        hcc_get_handler(chl)->hcc_resource.rx_thread_enable = enable;
    }
}

td_void hcc_enable_tx_thread(hcc_channel_name chl, td_bool enable)
{
    if (hcc_get_handler(chl) != TD_NULL) {
        hcc_printf("enable/disable tx thread: %d\n", enable);
        hcc_get_handler(chl)->hcc_resource.tx_thread_enable = enable;
    }
}

TD_PRV td_bool hcc_sched_cond_check(hcc_trans_queue *queue)
{
    if (!queue->queue_ctrl->fc_enable) {
        if (hcc_is_list_empty(&queue->queue_info)) {
            return TD_FALSE;
        } else {
            return TD_TRUE;
        }
    } else {
#if defined(CONFIG_HCC_SUPPORT_FLOW_CONTRL) && defined(CONFIG_HCC_SUPPORT_FLOW_CONTRL_ACTIVE)
        return hcc_flow_ctrl_sched_check(queue);
#else
        return TD_TRUE;
#endif
    }
}

td_s32 hcc_stop_xfer(hcc_channel_name channel_name)
{
    td_u8 i;
    hcc_trans_queue *queue = TD_NULL;
    hcc_handler *hcc = hcc_get_handler(channel_name);
    hcc_queue_cfg *q_cfg;

    if (hcc == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    q_cfg = hcc->que_cfg;

    if (hcc->bus && hcc->bus->bus_ops && hcc->bus->bus_ops->stop_xfer) {
        hcc->bus->bus_ops->stop_xfer();
    }
    for (i = 0; i < (hcc->que_max_cnt << 1); i++) {
        if (q_cfg[i].queue_id >= hcc->que_max_cnt || q_cfg[i].dir >= HCC_DIR_COUNT) {
            hcc_printf_err_log("q:%d\n", q_cfg[i].queue_id);
            continue;
        }
        queue = &hcc->hcc_resource.hcc_queues[q_cfg[i].dir][q_cfg[i].queue_id];
        hcc_list_free(hcc, &queue->queue_info);
    }
    return EXT_ERR_SUCCESS;
}
td_void hcc_stop_xfer_etc(td_void)
{
    hcc_stop_xfer(HCC_CHANNEL_AP);
}

TD_PRV int hcc_thread_wait_queue_cond(TD_CONST td_void *param)
{
    td_u32 dir = 0;
    td_u32 q_id = 0;
    hcc_handler *hcc = (hcc_handler *)param;
    hcc_trans_queue *queue;

    if (!hcc->hcc_resource.task_run) {
        return TD_TRUE;
    }
    if (hcc->bus == TD_NULL || osal_atomic_read(&hcc->hcc_state) != HCC_ON || hcc->bus->state == OSAL_FALSE) {
        return TD_FALSE;
    }

    for (; dir < HCC_DIR_COUNT; dir++) {
        for (q_id = 0; q_id < hcc->que_max_cnt; q_id++) {
            queue = &hcc->hcc_resource.hcc_queues[dir][q_id];
            hcc_printf_null("%s-%d, qid:%d-%d, qlen:%d\r\n",
                __FUNCTION__, __LINE__, dir, q_id, hcc->hcc_resource.hcc_queues[dir][q_id].queue_info.qlen);
            if (hcc_sched_cond_check(queue) == TD_TRUE) {
                hcc_printf("queue not empty, condition true, id -%d, len - %d\r\n",
                           dir, hcc_list_len(&queue->queue_info));
                return TD_TRUE;
            }
        }
    }
    return TD_FALSE;
}

td_void hcc_print_hex_dump(td_u8 *data, td_u32 len, char *str)
{
#ifdef CONFIG_HCC_DEBUG_PRINT
    td_u32 i;
    if ((data == TD_NULL) || (str == TD_NULL)) {
        return;
    }
    hcc_print_data("\r\n%s: len = %u\r\n", str, len);
    for (i = 0; i < len; i++) {
        if (!(i & 0x000f)) {
            hcc_print_data("\r\n%04d: ", i);
        }
        hcc_print_data(" %02x", data[i]);
    }
    hcc_print_data("\r\n\r\n");
#else
    uapi_unused(data);
    uapi_unused(len);
    uapi_unused(str);
#endif
}

td_bool hcc_check_header_vaild(hcc_handler *hcc, hcc_header *hdr)
{
    if (hdr == TD_NULL) {
        hcc_printf("{%s: line %d, hdr is null", __FUNCTION__, __LINE__);
        return TD_FALSE;
    }

    // BT的HCI头不解析
    if ((hdr->service_type == HCC_ACTION_TYPE_BT) || (hdr->service_type == HCC_ACTION_TYPE_SLE)) {
        return TD_TRUE;
    }

    if (hdr->service_type >= hcc->srv_max_cnt ||
        hdr->queue_id >= hcc->que_max_cnt ||
        hdr->pay_len <= hcc_get_head_len()) {
        hcc_printf("hcc_check_header_vaild failed\n");
        return TD_FALSE;
    }

    return TD_TRUE;
}

hcc_service_type hcc_queue_id_2_service_type(hcc_handler *hcc, hcc_queue_dir dir, hcc_queue_type queue_id)
{
    if (hcc == TD_NULL) {
        return HCC_SERVICE_TYPE_MAX;
    }

    if (hcc->hcc_resource.hcc_queues[dir][queue_id].queue_ctrl == TD_NULL) {
        return HCC_SERVICE_TYPE_MAX;
    }

    return hcc->hcc_resource.hcc_queues[dir][queue_id].queue_ctrl->service_type;
}

hcc_service_type hcc_fuzzy_trans_queue_2_service(hcc_handler *hcc, hcc_queue_type queue_id)
{
    hcc_service_type serv = hcc_queue_id_2_service_type(hcc, HCC_DIR_TX, queue_id);
    if (serv < hcc->srv_max_cnt) {
        return serv;
    }
    return hcc_queue_id_2_service_type(hcc, HCC_DIR_RX, queue_id);
}

td_u8 *hcc_pre_rx_data(td_u8 *buf, hcc_header *head, td_u32 len, td_u8 **user_param)
{
    td_u8 *data_buf;

    if ((buf == NULL) || (len <= sizeof(hcc_header))) {
        hcc_printf_err_log("hcc rx buf is invalid\n");
        return buf;
    }

    if ((head->queue_id == BT_DATA_QUEUE) || (head->queue_id == SLE_DATA_QUEUE)) {
        data_buf = buf + sizeof(hcc_header);
        *user_param = buf;
    } else {
        data_buf = buf;
    }

    return data_buf;
}

td_u32 hcc_get_transfer_packet_num(hcc_handler *hcc)
{
    if (hcc == TD_NULL) {
        return 0;
    }
    return hcc->hcc_resource.cur_trans_pkts;
}

td_u16 hcc_tx_queue_proc(hcc_handler *hcc, hcc_trans_queue *queue)
{
    hcc_data_queue *head = TD_NULL;
    hcc_bus *bus = TD_NULL;
    td_u16 max_send_pkt_nums;
    td_u16 remain_pkt_nums;
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc is null");
        return 0;
    }
    head = &queue->queue_info;
    if (hcc_is_list_empty(head) || (!hcc->hcc_resource.task_run)) {
        return 0;
    }
    bus = hcc->bus;
    hcc_printf("list len:%d - burst limit:%d\r\n", hcc_list_len(head), queue->queue_ctrl->burst_limit);
    max_send_pkt_nums = osal_min(hcc_list_len(head), (td_u32)queue->queue_ctrl->burst_limit);
    remain_pkt_nums = hcc_flow_ctrl_credit_update(hcc, queue, max_send_pkt_nums);

    hcc_printf("remain pkt nums: %d\n", remain_pkt_nums);
    while (remain_pkt_nums > 0) {
#if defined(CONFIG_HCC_SUPPORT_FLOW_CONTRL) && defined(CONFIG_HCC_SUPPORT_FLOW_CONTRL_ACTIVE)
        if (hcc_flow_ctrl_process(hcc, queue) != EXT_ERR_HCC_FC_PROC_UNBLOCK) {
            break;
        }
#endif
        if (hcc_bus_tx_proc(bus, queue, &remain_pkt_nums) != EXT_ERR_SUCCESS) {
            hcc_proc_fail_count_add();
            break;
        } else {
            hcc_proc_fail_count_clear();
        }
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
        hcc_adapt_tx_start_subq(hcc, queue);
#endif
    }
    hcc_dfx_queue_total_pkt_increase(hcc_fuzzy_trans_queue_2_service(hcc, queue->queue_id), HCC_DIR_TX,
        queue->queue_id, max_send_pkt_nums - remain_pkt_nums);
    return max_send_pkt_nums - remain_pkt_nums;
}

td_u16 hcc_rx_queue_proc(hcc_handler *hcc, hcc_trans_queue *queue)
{
    td_s32 ret = EXT_ERR_SUCCESS;
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_header *hcc_head = TD_NULL;
    hcc_adapt_priv_rx_process rx_proc = TD_NULL;
    hcc_serv_info *serv_info = TD_NULL;
    td_u8 *buf = TD_NULL;
    hcc_data_queue *head = &queue->queue_info;
    td_u16 rx_queue_cnt = 0;

    if (hcc_is_list_empty(head)) {
        return 0;
    }

    hcc_printf("[HCC]Rx List len %d\r\n", hcc_list_len(head));
    unc_buf = hcc_list_dequeue(head);
    /* 依次处理队列中每个netbuf */
    while (unc_buf != TD_NULL) {
        hcc_head = (hcc_header *)unc_buf->buf;
        serv_info = hcc_get_serv_info(hcc, hcc_head->service_type);
        ret = EXT_ERR_FAILURE;
        if (serv_info == TD_NULL || !hcc_check_header_vaild(hcc, hcc_head)) {
            break;
        }

        if (serv_info->adapt == TD_NULL) {
            hcc_printf_err_log("srv[%d] null\n", hcc_head->service_type);
            break;
        }

        buf = hcc_pre_rx_data(unc_buf->buf, hcc_head, unc_buf->length, &unc_buf->user_param);

        ret = EXT_ERR_SUCCESS;
        rx_proc = serv_info->adapt->rx_proc;
        hcc_dfx_service_exp_rx_cnt_increase(hcc_head->service_type);

        if (rx_proc != TD_NULL) {
            hcc_dfx_service_rx_cnt_increase(hcc_head->service_type, queue->queue_id);
            rx_proc(queue->queue_id, hcc_head->sub_type, buf, hcc_head->pay_len, unc_buf->user_param);
            hcc_free_unc_buf(hcc, unc_buf);
        } else {
            hcc_adapt_mem_free(hcc, unc_buf);
            hcc_dfx_service_rx_err_cnt_increase(hcc_head->service_type);
            hcc_dfx_queue_loss_pkt_increase(HCC_DIR_RX, queue->queue_id);
        }
        if (rx_queue_cnt++ >= queue->queue_ctrl->burst_limit) {
            break;
        }
        unc_buf = hcc_list_dequeue(head);
    }

    if (ret != EXT_ERR_SUCCESS) {
        hcc_adapt_mem_free(hcc, unc_buf);
        hcc_dfx_queue_loss_pkt_increase(HCC_DIR_RX, queue->queue_id);
    }
    return rx_queue_cnt;
}

td_void hcc_change_state(osal_atomic *atomic, td_u32 state)
{
    td_u32 old_state;
    td_u32 new_state;
    if (atomic == TD_NULL) {
        return;
    }
    old_state = (td_u32)osal_atomic_read(atomic);
    osal_atomic_set(atomic, state);
    new_state = (td_u32)osal_atomic_read(atomic);
    if (old_state != new_state) {
        hcc_printf("state [%d]=>[%d]\n", old_state, new_state);
    }
}

TD_PRV td_void hcc_queue_process(hcc_handler *hcc)
{
    td_u8 i;
    hcc_trans_queue *queue = TD_NULL;
    hcc_queue_cfg *q_cfg = hcc->que_cfg;
    td_u16 pkt_proc = 0;

    for (i = 0; i < (hcc->que_max_cnt << 1); i++) {
        if (q_cfg[i].queue_id >= hcc->que_max_cnt || q_cfg[i].dir >= HCC_DIR_COUNT) {
            hcc_printf_err_log("hcc thread err qid:%d\n", q_cfg[i].queue_id);
            continue;
        }
        queue = &hcc->hcc_resource.hcc_queues[q_cfg[i].dir][q_cfg[i].queue_id];
#if defined(CONFIG_HCC_SUPPORT_FLOW_CONTRL) && defined(CONFIG_HCC_SUPPORT_FLOW_CONTRL_ACTIVE)
        if (hcc_flow_ctrl_process(hcc, queue) != EXT_ERR_HCC_FC_PROC_UNBLOCK) {
            continue;
        }
#endif
        hcc_printf_null("proc: dir - %d, qID - %d\n", q_cfg[i].dir, q_cfg[i].queue_id);
        switch (q_cfg[i].dir) {
            case HCC_DIR_TX:
                pkt_proc = hcc_tx_queue_proc(hcc, queue);
                break;

            case HCC_DIR_RX:
                pkt_proc = hcc_rx_queue_proc(hcc, queue);
                break;

            default:
                pkt_proc = 0;
                hcc_printf_err_log("hcc thread dir err qid:%d\n", q_cfg[i].dir);
                break;
        }
        hcc->hcc_resource.cur_trans_pkts += pkt_proc;
    }
}

td_s32 hcc_resume_xfer(hcc_channel_name channel_name)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    if (hcc == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    if (hcc->bus && hcc->bus->bus_ops && hcc->bus->bus_ops->resume_xfer) {
        hcc->bus->bus_ops->resume_xfer();
    }
    hcc_enable_switch(channel_name, TD_TRUE);
    return EXT_ERR_SUCCESS;
}

td_void hcc_resume_xfer_etc(td_void)
{
    hcc_resume_xfer(HCC_CHANNEL_AP);
}

td_void hcc_switch_status(hcc_state_enum value)
{
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc null\n");
        return;
    }
    osal_atomic_set(&(hcc->hcc_state), value);
    if (value == HCC_OFF || value == HCC_BUS_FORBID) {
        hcc_stop_xfer_etc();
    } else if (value == HCC_ON) {
        hcc_resume_xfer_etc();
    }
    return;
}

td_s32 hcc_transfer_thread(td_void *data)
{
    td_s32 ret = 0;
    hcc_handler *hcc = (hcc_handler *)data;
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc null");
        return EXT_ERR_FAILURE;
    }
    osal_kthread_set_priority(hcc->hcc_resource.hcc_transfer_thread_handler, HCC_TASK_PRIORITY);
    osal_kthread_set_affinity(hcc->hcc_resource.hcc_transfer_thread_handler, OSAL_CPU_3);
    while (hcc->hcc_resource.task_run) {
#if defined CONFIG_HCC_SUPPORT_NON_OS
        if (hcc_thread_wait_queue_cond(hcc) == TD_FALSE) {
            return 0;
        }
#else
        ret = osal_wait_timeout_interruptible(
            &hcc->hcc_resource.hcc_transfer_wq, hcc_thread_wait_queue_cond, hcc, OSAL_WAIT_FOREVER);
        if (ret == 0) {
            // timeout
            continue;
        }
        if (!hcc->hcc_resource.task_run) {
            break;
        }
#endif

        hcc_printf_null("[INFO] - %s, evt wait success\n", __FUNCTION__);
        hcc_queue_process(hcc);
    }
    hcc_printf("hcc transfer thread done!\n");
    return EXT_ERR_SUCCESS;
}

ext_errno hcc_message_register(hcc_channel_name channel_name, hcc_service_type service_type,
    hcc_rx_msg_type msg_id, hcc_msg_rx cb, td_u8 *cb_data)
{
    hcc_bus *bus = hcc_get_channel_bus(channel_name);
    struct bus_msg_stru *message = TD_NULL;
    if (bus == TD_NULL || msg_id >= HCC_RX_MAX_MESSAGE || cb == TD_NULL) {
        return EXT_ERR_HCC_PARAM_ERR;
    }

    message = &bus->msg[msg_id];
    message->service_type = service_type;
    message->msg_rx = cb;
    message->data = cb_data;
    hcc_printf_null("[success] hcc_message_register - %d\r\n", msg_id);
    osal_atomic_set(&message->count, 0);
    return EXT_ERR_SUCCESS;
}

ext_errno hcc_message_unregister(hcc_channel_name channel_name, hcc_service_type service_type,
    hcc_rx_msg_type msg_id)
{
    hcc_bus *bus = hcc_get_channel_bus(channel_name);
    struct bus_msg_stru *message = TD_NULL;
    if (bus == TD_NULL || msg_id >= HCC_RX_MAX_MESSAGE) {
        return EXT_ERR_HCC_PARAM_ERR;
    }
    uapi_unused(service_type);

    message = &bus->msg[msg_id];
    message->msg_rx = TD_NULL;
    message->service_type = HCC_SERVICE_TYPE_MAX;
    message->data = TD_NULL;
    return EXT_ERR_SUCCESS;
}

TD_PRV hcc_queue_ctrl *hcc_get_queue_ctrl_from_cfg(hcc_handler *hcc, hcc_queue_dir dir, hcc_queue_type q_id)
{
    td_u8 index;
    hcc_queue_cfg *que_cfg = hcc->que_cfg;
    for (index = 0; index < (hcc->que_max_cnt << 1); index++) {
        if (que_cfg[index].dir != dir || que_cfg[index].queue_id != q_id) {
            continue;
        }

        return &que_cfg[index].queue_ctrl;
    }
    return TD_NULL;
}

TD_PRV td_bool hcc_check_queue_cfg(hcc_queue_cfg *queue_cfg, td_u32 queue_len)
{
    td_u32 que_cnt[HCC_DIR_COUNT] = {0};
    td_u32 i;

    for (i = 0; i < queue_len; i++) {
        if (queue_cfg[i].dir < HCC_DIR_COUNT) {
            que_cnt[queue_cfg[i].dir]++;
        } else {
            return TD_FALSE;
        }
    }
    return (que_cnt[HCC_DIR_TX] == que_cnt[HCC_DIR_RX]);
}

TD_PRV td_s32 hcc_transfer_queue_init_fail(hcc_handler *hcc, td_u8 dir, td_u8 q_id, td_bool send_head)
{
    td_u8 j;
    hcc_trans_queue *hcc_queue = TD_NULL;

    if (dir > 0) {
        for (j = 0; j < hcc->que_max_cnt; j++) {
            hcc_queue = &hcc->hcc_resource.hcc_queues[0][j];
            hcc_list_head_deinit(&hcc_queue->queue_info);
            hcc_list_head_deinit(&hcc_queue->send_head);
        }
    }
    for (j = 0; j < q_id; j++) {
        hcc_queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_RX][j];
        hcc_list_head_deinit(&hcc_queue->queue_info);
        hcc_list_head_deinit(&hcc_queue->send_head);
    }
    if (send_head) {
        hcc_queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_RX][q_id];
        hcc_list_head_deinit(&hcc_queue->queue_info);
    }
    osal_kfree(hcc->hcc_resource.hcc_queues[HCC_DIR_TX]);
    osal_kfree(hcc->hcc_resource.hcc_queues[HCC_DIR_RX]);
    return EXT_ERR_FAILURE;
}

TD_PRV td_s32 hcc_transfer_queue_init(hcc_handler *hcc, hcc_queue_cfg *queue_cfg, td_u32 queue_len)
{
    td_u8 dir;
    td_u8 q_id;
    hcc_trans_queue *hcc_queue = TD_NULL;

    hcc->que_cfg = queue_cfg;
    hcc->que_max_cnt = (td_u8)(queue_len >> 1);

    if (!hcc_check_queue_cfg(queue_cfg, queue_len)) {
        return EXT_ERR_FAILURE;
    }
    hcc->hcc_resource.hcc_queues[HCC_DIR_TX] =
        osal_kmalloc(sizeof(hcc_trans_queue) * hcc->que_max_cnt, OSAL_GFP_KERNEL);
    if (hcc->hcc_resource.hcc_queues[HCC_DIR_TX] == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    hcc->hcc_resource.hcc_queues[HCC_DIR_RX] =
        osal_kmalloc(sizeof(hcc_trans_queue) * hcc->que_max_cnt, OSAL_GFP_KERNEL);
    if (hcc->hcc_resource.hcc_queues[HCC_DIR_RX] == TD_NULL) {
        osal_kfree(hcc->hcc_resource.hcc_queues[HCC_DIR_TX]);
        return EXT_ERR_FAILURE;
    }
    for (dir = 0; dir < HCC_DIR_COUNT; dir++) {
        for (q_id = 0; q_id < hcc->que_max_cnt; q_id++) {
            hcc_queue = &hcc->hcc_resource.hcc_queues[dir][q_id];
            if (hcc_list_head_init(&hcc_queue->queue_info) != EXT_ERR_SUCCESS) {
                return hcc_transfer_queue_init_fail(hcc, dir, q_id, TD_FALSE);
            }
            hcc_queue->queue_id = q_id;
            hcc_queue->queue_ctrl = hcc_get_queue_ctrl_from_cfg(hcc, dir, q_id);
            if (hcc_queue->queue_ctrl == TD_NULL) {
                return hcc_transfer_queue_init_fail(hcc, dir, q_id, TD_TRUE);
            }

            if (hcc_list_head_init(&hcc_queue->send_head) != EXT_ERR_SUCCESS) {
                return hcc_transfer_queue_init_fail(hcc, dir, q_id, TD_TRUE);
            }
        }
    }
    return EXT_ERR_SUCCESS;
}

TD_PRV td_void hcc_transfer_queue_deinit(hcc_handler *hcc)
{
    td_u8 dir;
    td_u8 q_id;
    hcc_trans_queue *hcc_queue = TD_NULL;

    for (dir = 0; dir < HCC_DIR_COUNT; dir++) {
        for (q_id = 0; q_id < hcc->que_max_cnt; q_id++) {
            hcc_queue = &hcc->hcc_resource.hcc_queues[dir][q_id];
            hcc_list_head_deinit(&hcc_queue->queue_info);
            hcc_list_head_deinit(&hcc_queue->send_head);
        }
    }
    osal_kfree(hcc->hcc_resource.hcc_queues[HCC_DIR_TX]);
    hcc->hcc_resource.hcc_queues[HCC_DIR_TX] = TD_NULL;
    osal_kfree(hcc->hcc_resource.hcc_queues[HCC_DIR_RX]);
    hcc->hcc_resource.hcc_queues[HCC_DIR_RX] = TD_NULL;
}

TD_PRV td_void hcc_service_resource_deinit(hcc_handler *hcc)
{
    osal_spin_lock_destroy(&hcc->hcc_serv.service_lock);
    osal_spin_lock_destroy(&hcc->hcc_serv_del.service_lock);
}

TD_PRV ext_errno hcc_service_resource_init(hcc_handler *hcc)
{
    OSAL_INIT_LIST_HEAD(&hcc->hcc_serv.service_list);
    OSAL_INIT_LIST_HEAD(&hcc->hcc_serv_del.service_list);
    hcc->hcc_serv.service_info = TD_NULL;
    hcc->hcc_serv_del.service_info = TD_NULL;
    if (osal_spin_lock_init(&hcc->hcc_serv.service_lock) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }
    if (osal_spin_lock_init(&hcc->hcc_serv_del.service_lock) != EXT_ERR_SUCCESS) {
        osal_spin_lock_destroy(&hcc->hcc_serv.service_lock);
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

ext_errno hcc_sched_transfer(hcc_handler *hcc)
{
#ifndef CONFIG_HCC_SUPPORT_NON_OS
    if (hcc == TD_NULL) {
        hcc_printf("%s,hcc is null\n", __FUNCTION__);
        return EXT_ERR_FAILURE;
    }
    hcc_printf("sched hcc transfer\n");
    osal_wait_wakeup(&hcc->hcc_resource.hcc_transfer_wq);
#endif
    return EXT_ERR_SUCCESS;
}

td_void hcc_enable_switch(hcc_channel_name chl, td_bool enable)
{
    hcc_handler *hcc = hcc_get_handler(chl);
    if (hcc != TD_NULL) {
        hcc_change_state(&hcc->hcc_state, enable);
    }
}

#ifdef CONFIG_HCC_SUPPORT_UNC_POOL
ext_errno hcc_set_unc_pool(hcc_data_queue *unc_pool, td_u32 unc_pool_size)
{
    td_u32 i;
    hcc_unc_struc *unc;

    hcc_list_head_init(unc_pool);
    unc = (hcc_unc_struc *)(unc_pool + 1);

    for (i = 0; i < unc_pool_size; i++) {
        hcc_list_add_tail(unc_pool, &unc[i]);
    }
    hcc_printf("%s success\e\n", __FUNCTION__);
    return EXT_ERR_SUCCESS;
}

TD_PRV td_void hcc_release_unc_pool(hcc_data_queue *unc_pool)
{
    if (unc_pool == TD_NULL) {
        return;
    }
    osal_spin_lock_destroy(&unc_pool->data_queue_lock);
}
#endif

TD_PRV ext_errno hcc_task_init(hcc_handler *hcc)
{
    hcc->hcc_resource.task_run = TD_TRUE;
#ifdef CONFIG_HCC_SUPPORT_NON_OS
    /* non os 不需要创建task */
    hcc->hcc_resource.rx_thread_enable = TD_FALSE;
    hcc->hcc_resource.tx_thread_enable = TD_FALSE;
    return EXT_ERR_SUCCESS;
#endif
    if (osal_wait_init(&hcc->hcc_resource.hcc_transfer_wq) != EXT_ERR_SUCCESS) {
        hcc_printf_err_log("wait\n");
        return EXT_ERR_FAILURE;
    }
    hcc->hcc_resource.hcc_transfer_thread_handler = osal_kthread_create(
        hcc_transfer_thread, (td_void *)hcc, hcc_get_task_name(hcc->channel_name), HCC_TRANS_THREAD_TASK_STACK_SIZE);
    if (hcc->hcc_resource.hcc_transfer_thread_handler == TD_NULL) {
        osal_wait_destroy(&hcc->hcc_resource.hcc_transfer_wq);
        hcc_printf("hcc thread create failed!\n");
        return EXT_ERR_FAILURE;
    }
    osal_kthread_set_priority(hcc->hcc_resource.hcc_transfer_thread_handler, HCC_TASK_PRIORITY);
    /* 默认使用hcc task处理队列中的数据 */
    hcc->hcc_resource.rx_thread_enable = TD_TRUE;
    hcc->hcc_resource.tx_thread_enable = TD_TRUE;
    return EXT_ERR_SUCCESS;
}

TD_PRV td_void hcc_task_exit(hcc_handler *hcc)
{
    hcc->hcc_resource.task_run = TD_FALSE;
#ifdef CONFIG_HCC_SUPPORT_NON_OS
    return;
#else
    osal_kthread_destroy(hcc->hcc_resource.hcc_transfer_thread_handler, TD_TRUE);
    osal_wait_destroy(&hcc->hcc_resource.hcc_transfer_wq);
#endif
}

TD_PRV td_void hcc_module_exit(hcc_channel_name channel_name, hcc_handler *hcc)
{
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    hcc_flow_ctrl_module_deinit(hcc);
#endif
    hcc_transfer_queue_deinit(hcc);
#ifdef CONFIG_HCC_SUPPORT_UNC_POOL
    hcc_release_unc_pool(hcc->unc_pool_head);
#endif
    hcc_delete_handler(channel_name);
    hcc = TD_NULL;
}

TD_PRV hcc_handler *hcc_module_init_error(hcc_handler *hcc, hcc_module_init_errno num)
{
    // 初始化过程导致函数过长，使用errno替换goto标签，并记录错误码
    switch (num) {
        case HCC_MODULE_INIT_FAILED_ADD_HANDLER:
            goto faild_add_handle;
            break;
        case HCC_MODULE_INIT_FAILED_INIT_TASK:
            goto faild_init_task;
            break;
        case HCC_MODULE_INIT_FAILED_INIT_SERVICE:
            goto faild_init_service;
            break;
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
        case HCC_MODULE_INIT_FAILED_INIT_FLOW_CTRL:
            goto faild_flow_ctrl;
            break;
#endif
        case HCC_MODULE_INIT_FAILED_INIT_QUEUE:
            goto faild_queue_init;
            break;
#ifdef CONFIG_HCC_SUPPORT_UNC_POOL
        case HCC_MODULE_INIT_FAILED_INIT_UNC_POOL:
            goto faild_release;
            break;
#endif
        case HCC_MODULE_INIT_FAILED_SET_STATE:
            goto faild_release;
            break;
        default:
            break;
    }
faild_add_handle:
    hcc_task_exit(hcc);
faild_init_task:
    hcc_service_resource_deinit(hcc);
faild_init_service:
    hcc_flow_ctrl_module_deinit(hcc);
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
faild_flow_ctrl:
#endif
    hcc_transfer_queue_deinit(hcc);
faild_queue_init:
#ifdef CONFIG_HCC_SUPPORT_UNC_POOL
    hcc_release_unc_pool(hcc->unc_pool_head);
#endif
faild_release:
    hcc_printf_err_log("module:%d\r\n", num);
    osal_kfree((td_u8 *)hcc);
    return TD_NULL;
}

TD_PRV hcc_handler *hcc_module_mem_alloc(hcc_channel_name channel_name, td_u32 unc_pool_size)
{
    hcc_handler *hcc;
    size_t total_len = 0;
#ifdef CONFIG_HCC_SUPPORT_UNC_POOL
    total_len = (td_u32)(sizeof(hcc_unc_struc) * unc_pool_size +
        sizeof(hcc_data_queue) + sizeof(hcc_handler));
    hcc = (hcc_handler *)osal_kmalloc(total_len, OSAL_GFP_KERNEL);
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc malloc\n");
        return hcc;
    }
    hcc->unc_pool_head = (hcc_data_queue *)(hcc + 1);
#else
    uapi_unused(unc_pool_size);
    total_len = sizeof(hcc_handler);
    hcc = (hcc_handler *)osal_kmalloc(sizeof(hcc_handler), OSAL_GFP_KERNEL);
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc malloc\n");
        return hcc;
    }
    hcc->unc_pool_head = TD_NULL;
#endif
    (td_void)memset_s(hcc, total_len, 0, total_len);

    hcc->channel_name = channel_name;
    return hcc;
}

TD_PRV hcc_handler *hcc_module_init(hcc_channel_init *init)
{
    hcc_handler *hcc = hcc_module_mem_alloc(init->channel_name, init->unc_pool_size);
    if (hcc == TD_NULL) {
        return hcc;
    }

    osal_atomic_set(&hcc->hcc_state, HCC_OFF);
    if (osal_atomic_read(&hcc->hcc_state) != HCC_OFF) {
        return hcc_module_init_error(hcc, HCC_MODULE_INIT_FAILED_SET_STATE);
    }

#ifdef CONFIG_HCC_SUPPORT_UNC_POOL
    if (hcc_set_unc_pool(hcc->unc_pool_head, init->unc_pool_size) != EXT_ERR_SUCCESS) {
        return hcc_module_init_error(hcc, HCC_MODULE_INIT_FAILED_INIT_UNC_POOL);
    }
#endif
    if (hcc_transfer_queue_init(hcc, init->queue_cfg, init->queue_len) != EXT_ERR_SUCCESS) {
        return hcc_module_init_error(hcc, HCC_MODULE_INIT_FAILED_INIT_QUEUE);
    }

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    if (hcc_flow_ctrl_module_init(hcc) != EXT_ERR_SUCCESS) {
        return hcc_module_init_error(hcc, HCC_MODULE_INIT_FAILED_INIT_FLOW_CTRL);
    }
#endif
    if (hcc_service_resource_init(hcc) != EXT_ERR_SUCCESS) {
        return hcc_module_init_error(hcc, HCC_MODULE_INIT_FAILED_INIT_SERVICE);
    }

    if (hcc_task_init(hcc) != EXT_ERR_SUCCESS) {
        return hcc_module_init_error(hcc, HCC_MODULE_INIT_FAILED_INIT_TASK);
    }

    if (hcc_add_handler(init->channel_name, hcc) != EXT_ERR_SUCCESS) {
        return hcc_module_init_error(hcc, HCC_MODULE_INIT_FAILED_ADD_HANDLER);
    }
    osal_atomic_set(&hcc->hcc_state, HCC_ON);
    hcc->srv_max_cnt = 1;
    hcc_printf("[success] - %s\n", __FUNCTION__);
    return hcc;
}

td_void hcc_deinit(hcc_channel_name channel_name)
{
    td_u32 i;
    hcc_handler *hcc = hcc_get_handler(channel_name);
    if (hcc == TD_NULL) {
        return;
    }
    hcc_heartbeat_deinit();
    for (i = 0; i < hcc->srv_max_cnt; i++) {
        hcc_service_deinit(channel_name, i);
    }
    osal_spin_lock_destroy(&hcc->hcc_serv.service_lock);
    hcc_task_exit(hcc);
    hcc_bus_unload(hcc->bus);
    hcc_dfx_deinit();
    for (i = 0; i < hcc->srv_max_cnt; i++) {
        hcc_del_service_list(hcc, i);
    }
    osal_spin_lock_destroy(&hcc->hcc_serv_del.service_lock);
    hcc_module_exit(channel_name, hcc);
}

TD_PRV ext_errno hcc_adapt_tx_param_check(hcc_handler *hcc, td_u8 *buf, td_u16 len, hcc_transfer_param *param)
{
    hcc_serv_info *serv_info = hcc_get_serv_info(hcc, param->service_type);
    if (hcc->bus == TD_NULL || serv_info == TD_NULL) {
        hcc_printf("srv[%d] or bus[%d] null\r\n", param->service_type, hcc->channel_name);
        return EXT_ERR_FAILURE;
    }

    if (param->service_type >= hcc->srv_max_cnt ||
        hcc_fuzzy_trans_queue_2_service(hcc, param->queue_id) != param->service_type) {
        hcc_printf_err_log("tx srv t-%d, q-%d\r\n", param->service_type, param->queue_id);
        return EXT_ERR_FAILURE;
    }

    if (param->fc_flag & (~HCC_FC_ALL)) {
        hcc_printf_err_log("tx fc-%d\r\n", param->fc_flag);
        return EXT_ERR_FAILURE;
    }

    if (param->queue_id >= hcc->que_max_cnt) {
        hcc_printf_err_log("tx qid-%d\r\n", param->queue_id);
        return EXT_ERR_FAILURE;
    }

    if (osal_atomic_read(&serv_info->service_enable) != HCC_ENABLE) {
        hcc_printf_err_log("tx srv[%d] null\r\n", param->service_type);
        return EXT_ERR_FAILURE;
    }

    if (!osal_is_aligned((uintptr_t)buf, hcc->bus->addr_align)) {
        return EXT_ERR_FAILURE;
    }

    if (len <= hcc_get_head_len() || !osal_is_aligned(len, hcc->bus->len_align) ||
        len > hcc->bus->max_trans_size) {
        hcc_printf_err_log("tx err: len[%d] min[%d] max[%d] aligned[%d]\r\n",
                           len, hcc_get_head_len(), hcc->bus->max_trans_size, hcc->bus->len_align);
        return EXT_ERR_FAILURE;
    }

    return EXT_SUCCESS;
}


TD_PRV td_void hcc_header_init(hcc_header *head, td_u32 len, hcc_transfer_param *param)
{
    if ((param->service_type != HCC_ACTION_TYPE_BT) && (param->service_type != HCC_ACTION_TYPE_SLE)) {
        /* fill the hcc header */
        head->service_type = (td_u8)param->service_type;
        head->sub_type = (td_u8)param->sub_type;
        head->queue_id = (td_u8)param->queue_id;
        head->pay_len = len;
    }
}

td_u32 hcc_init(hcc_channel_init *init)
{
    td_u32 ret = EXT_ERR_SUCCESS;
    hcc_handler *hcc;

    if (init == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    hcc = hcc_get_handler(init->channel_name);
    if ((hcc != TD_NULL) || (init->queue_cfg == TD_NULL) || (init->queue_len == 0)) {
        hcc_printf_err_log("param err\r\n");
        return EXT_ERR_FAILURE;
    }
    hcc = hcc_module_init(init);
    if (hcc == TD_NULL) {
        hcc_printf_err_log("module init\r\n");
        return EXT_ERR_FAILURE;
    }

    ret = hcc_bus_load(init->bus_type, hcc);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_task_exit(hcc);
        hcc_module_exit(init->channel_name, hcc);
        return ret;
    }

    ret = (td_u32)hcc_dfx_init();
    if (ret != EXT_ERR_SUCCESS) {
        hcc_task_exit(hcc);
        hcc_bus_unload(hcc->bus);
        hcc_module_exit(init->channel_name, hcc);
        return ret;
    }

    hcc_heartbeat_init();
    hcc_printf("hcc init succ");
    return ret;
}

td_void hcc_init_unc_buf(hcc_unc_struc *unc_buf, td_u8* buf, td_u32 len, hcc_transfer_param *param)
{
    unc_buf->list.next = TD_NULL;
    unc_buf->list.prev = TD_NULL;
    unc_buf->buf = buf;
    unc_buf->length = len;
    unc_buf->user_param = param->user_param;
    unc_buf->service_type = param->service_type;
    unc_buf->sub_type = param->sub_type;
    unc_buf->queue_id = param->queue_id;
}

ext_errno hcc_bt_tx_data(hcc_channel_name channel_name, td_u8 *data_buf, td_u16 len, hcc_transfer_param *param)
{
    td_s32 ret;
    hcc_handler *hcc = hcc_get_handler(channel_name);
    size_t buf_size;
    td_u8 *buf = 0;
    hcc_header *head = TD_NULL;
    td_u8 *user_param = TD_NULL;
    hcc_serv_info *serv_info = TD_NULL;

    if (hcc == TD_NULL || hcc->bus == TD_NULL || data_buf == TD_NULL || param == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    buf_size = align_next((len + sizeof(hcc_header)), hcc->bus->len_align);
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        buf_size = align_next((len + sizeof(hcc_header) + sizeof(uart_packet_head) + 1), hcc->bus->len_align);
    }

    serv_info = hcc_get_serv_info(hcc, param->service_type);
    if (serv_info == TD_NULL || serv_info->adapt == TD_NULL ||
        serv_info->adapt->alloc == TD_NULL || serv_info->adapt->free == TD_NULL) {
        hcc_printf_err_log("srv-%d null\n", param->service_type);
        return EXT_ERR_FAILURE;
    }

    ret = (td_s32)serv_info->adapt->alloc(param->queue_id, buf_size, &buf, &user_param);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_printf_err_log("srv alloc fail\r\n");
        return EXT_ERR_FAILURE;
    }
    (td_void)memset_s(buf, buf_size, 0, buf_size);

    if (hcc->bus->bus_type == HCC_BUS_UART) {
        head = (hcc_header *)(buf + sizeof(uart_packet_head));
        ret = memcpy_s(buf + sizeof(hcc_header) + sizeof(uart_packet_head), len, data_buf, len);
    } else {
        head = (hcc_header *)buf;
        ret = memcpy_s(buf + sizeof(hcc_header), len, data_buf, len);
    }
    if (ret != EOK) {
        ret = EXT_ERR_FAILURE;
        serv_info->adapt->free(param->queue_id, buf, user_param);
        return ret;
    }
    /* hcc header init */
    head->service_type = (td_u8)param->service_type;
    head->sub_type = (td_u8)param->sub_type;
    head->queue_id = (td_u8)param->queue_id;
    head->pay_len = len;

    ret = hcc_tx_data(HCC_CHANNEL_AP, buf, buf_size, param);
    if (ret == EXT_ERR_SUCCESS) {
        return ret;
    }
    serv_info->adapt->free(param->queue_id, buf, user_param);
    return ret;
}

ext_errno hcc_tx_data(hcc_channel_name channel_name, td_u8 *buf, td_u16 len, hcc_transfer_param *param)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    hcc_trans_queue *queue = TD_NULL;
    hcc_unc_struc *unc = TD_NULL;
    if (hcc == TD_NULL || buf == TD_NULL || param == TD_NULL) {
        hcc_printf_err_log("params null !\n");
        return EXT_ERR_HCC_PARAM_ERR;
    }
    if (osal_atomic_read(&hcc->hcc_state) != HCC_ON) {
        if (osal_atomic_read(&hcc->hcc_state) == HCC_OFF) {
            hcc_printf_err_log("off\r\n");
            return EXT_ERR_HCC_STATE_OFF;
        } else if (osal_atomic_read(&hcc->hcc_state) == HCC_BUS_FORBID) {
            /* 异常状态设置时机、异常处理 */
            hcc_printf_err_log("forbid\r\n");
            return EXT_ERR_HCC_STATE_EXCEPTION;
        } else {
            hcc_printf_err_log("invaild:%d", osal_atomic_read(&hcc->hcc_state));
            return EXT_ERR_HCC_PARAM_ERR;
        }
    }
    if (hcc_adapt_tx_param_check(hcc, buf, len, param) != EXT_ERR_SUCCESS) {
        return EXT_ERR_HCC_PARAM_ERR;
    }
    queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_TX][param->queue_id];
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    if (hcc_flow_ctrl_pre_proc(hcc, param, queue) != EXT_ERR_SUCCESS) {
        return EXT_ERR_HCC_FC_PRE_PROC_ERR;
    }
#endif
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        hcc_header_init((hcc_header *)(buf + sizeof(uart_packet_head)), len, param);
    } else {
        hcc_header_init((hcc_header *)buf, len, param);
    }
    unc = hcc_alloc_unc_buf(hcc);
    if (unc == TD_NULL) {
        hcc_dfx_unc_alloc_cnt_increase(HCC_DIR_TX, TD_FALSE);
        return EXT_ERR_HCC_BUILD_TX_BUF_ERR;
    }
    hcc_dfx_unc_alloc_cnt_increase(HCC_DIR_TX, TD_TRUE);
    hcc_init_unc_buf(unc, buf, len, param);
    hcc_print_hex_dump((td_u8*)unc->buf, sizeof(hcc_header), "unc head:");
    hcc_print_hex_dump((td_u8*)unc->buf + sizeof(hcc_header), unc->length - sizeof(hcc_header), "unc buf:");
    /* 插入hcc队列之前返回失败都由业务释放
     * 插入hcc队列之后异常都由HCC释放
     */
    hcc_list_add_tail(&queue->queue_info, unc);
    if (hcc->hcc_resource.tx_thread_enable) {
        hcc_bus *bus = hcc->bus;
        if (param->queue_id == DATA_TCP_DATA_QUEUE || param->queue_id == DATA_UDP_DATA_QUEUE) {
            if (hcc_list_len(&queue->queue_info) >= (td_u32)bus->tx_sched_count) {
                hcc_sched_transfer(hcc);
            }
        } else {
            hcc_sched_transfer(hcc);
        }
    } else {
        hcc_tx_queue_proc(hcc, queue);
    }
    return EXT_ERR_SUCCESS;
}

td_u32 hcc_get_tx_queue_len(hcc_channel_name channel_name, td_u32 queue_id)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    hcc_trans_queue *queue = TD_NULL;
    if (hcc == TD_NULL || queue_id >= hcc->que_max_cnt) {
        return 0;
    }

    queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_TX][queue_id];
    return queue->queue_info.qlen;
}

td_u32 hcc_get_tx_sched_len(hcc_channel_name channel_name, td_u32 queue_id)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    hcc_trans_queue *queue = TD_NULL;
    if (hcc == TD_NULL || queue_id >= hcc->que_max_cnt) {
        return 0;
    }

    queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_TX][queue_id];
    return queue->queue_ctrl->high_waterline;
}

td_u32 hcc_get_tx_mpdu_count(osal_void)
{
    return hcc_get_tx_queue_len(HCC_CHANNEL_AP, DATA_TCP_ACK_QUEUE) +
        hcc_get_tx_queue_len(HCC_CHANNEL_AP, DATA_TCP_DATA_QUEUE) +
        hcc_get_tx_queue_len(HCC_CHANNEL_AP, DATA_UDP_DATA_QUEUE);
}

td_u32 hcc_get_tx_sched_num(osal_void)
{
    td_u32 total_num = 0;
    td_u32 allow_num = 0;

    total_num = hcc_get_tx_queue_len(HCC_CHANNEL_AP, DATA_TCP_ACK_QUEUE) +
        hcc_get_tx_queue_len(HCC_CHANNEL_AP, DATA_TCP_DATA_QUEUE) +
        hcc_get_tx_queue_len(HCC_CHANNEL_AP, DATA_UDP_DATA_QUEUE);

    allow_num = hcc_get_tx_sched_len(HCC_CHANNEL_AP, DATA_UDP_DATA_QUEUE);

    return (allow_num > total_num) ? (allow_num - total_num) : 0;
}

osal_module_export(hcc_get_tx_sched_num);
osal_module_export(hcc_message_register);
osal_module_export(hcc_message_unregister);
osal_module_export(hcc_get_head_len);
osal_module_export(hcc_init);
osal_module_export(hcc_deinit);
osal_module_export(hcc_enable_switch);
osal_module_export(hcc_enable_rx_thread);
osal_module_export(hcc_enable_tx_thread);
osal_module_export(hcc_stop_xfer_etc);
osal_module_export(hcc_resume_xfer_etc);
osal_module_export(hcc_switch_status);
osal_module_export(hcc_tx_data);
osal_module_export(hcc_bt_tx_data);
