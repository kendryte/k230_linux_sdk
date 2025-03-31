/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc dfx completion.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#include "hcc_dfx.h"
#include "hcc_comm.h"
#include "soc_errno.h"
#include "soc_osal.h"
#include "securec.h"
#include "securectype.h"
#include "hcc_cfg_comm.h"
#include "hcc_list.h"
#include "hcc.h"
#include "hcc_channel.h"
#include "hcc_bus.h"
#include "hcc_if.h"

#ifdef CONFIG_HCC_SUPPORT_DFX
static hcc_dfx g_hcc_dfx;
#endif

td_s32 hcc_dfx_init(td_void)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    td_s32 ret;
    memset_s(&g_hcc_dfx, sizeof(hcc_dfx), 0, sizeof(hcc_dfx));
    ret = osal_spin_lock_init(&g_hcc_dfx.hcc_dfx_lock);
    g_hcc_dfx.que_max_cnt = HCC_QUEUE_COUNT;
    g_hcc_dfx.srv_max_cnt = HCC_SERVICE_TYPE_MAX;
    return ret;
#else
    return EXT_ERR_SUCCESS;
#endif
}

td_void hcc_dfx_deinit(td_void)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    osal_spin_lock_destroy(&g_hcc_dfx.hcc_dfx_lock);
#endif
}

#ifdef CONFIG_HCC_SUPPORT_DFX
TD_PRV hcc_dfx *hcc_get_dfx(td_void)
{
    return &g_hcc_dfx;
}
#endif

td_void hcc_dfx_queue_total_pkt_increase(hcc_service_type serv_type, hcc_queue_dir dir, hcc_queue_type q_id, td_u8 cnt)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (q_id >= hcc_dfx_stat->que_max_cnt || serv_type >= hcc_dfx_stat->srv_max_cnt || dir >= HCC_DIR_COUNT) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->queue_stat[dir][q_id].total_pkts += cnt;
    if (dir == HCC_DIR_TX) { /* 此处只记录tx的数量，rx的可以使用其他值 */
        hcc_dfx_stat->service_stat[serv_type].bus_tx_succ += cnt;
    }

    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(serv_type);
    uapi_unused(dir);
    uapi_unused(q_id);
    uapi_unused(cnt);
#endif
}

td_void hcc_dfx_queue_loss_pkt_increase(hcc_queue_dir dir, hcc_queue_type q_id)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (q_id >= hcc_dfx_stat->que_max_cnt || dir >= HCC_DIR_COUNT) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->queue_stat[dir][q_id].loss_pkts++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(dir);
    uapi_unused(q_id);
#endif
}

td_void hcc_dfx_service_alloc_cb_cnt_increase(hcc_service_type service_type)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->service_stat[service_type].alloc_cb_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
#endif
}

td_void hcc_dfx_service_alloc_cnt_increase(hcc_service_type service_type, hcc_queue_type queue_id, td_bool success)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (queue_id >= hcc_dfx_stat->que_max_cnt || service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    if (success) {
        hcc_dfx_stat->service_stat[service_type].alloc_succ_cnt++;
        hcc_dfx_stat->queue_stat[HCC_DIR_RX][queue_id].alloc_succ_cnt++;
    } else {
        hcc_dfx_stat->service_stat[service_type].alloc_fail_cnt++;
        hcc_dfx_stat->queue_stat[HCC_DIR_RX][queue_id].alloc_fail_cnt++;
    }
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
    uapi_unused(queue_id);
    uapi_unused(success);
#endif
}

td_void hcc_dfx_service_free_cnt_increase(hcc_service_type service_type)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->service_stat[service_type].free_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
#endif
}

td_void hcc_dfx_unc_alloc_cnt_increase(hcc_queue_dir direction, td_bool success)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (direction >= HCC_DIR_COUNT) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    if (success) {
        hcc_dfx_stat->inner_stat.unc_alloc_succ[direction]++;
    } else {
        hcc_dfx_stat->inner_stat.unc_alloc_fail[direction]++;
    }
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(direction);
    uapi_unused(success);
#endif
}

td_void hcc_dfx_mem_free_cnt_increase(td_void)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->inner_stat.mem_free_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#endif
}

td_void hcc_dfx_unc_free_cnt_increase(td_void)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->inner_stat.unc_free_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#endif
}

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
td_void hcc_dfx_service_startsubq_cnt_increase(hcc_service_type service_type)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->service_stat[service_type].start_subq_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
#endif
}

td_void hcc_dfx_service_stopsubq_cnt_increase(hcc_service_type service_type)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->service_stat[service_type].stop_subq_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
#endif
}
#endif

td_void hcc_dfx_service_rx_cnt_increase(hcc_service_type service_type, hcc_queue_type queue_id)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (queue_id >= hcc_dfx_stat->que_max_cnt || service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->service_stat[service_type].rx_cnt++;
    hcc_dfx_stat->queue_stat[HCC_DIR_RX][queue_id].total_pkts++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
    uapi_unused(queue_id);
#endif
}

td_void hcc_dfx_service_exp_rx_cnt_increase(hcc_service_type service_type)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->service_stat[service_type].exp_rx_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
#endif
}

td_void hcc_dfx_service_rx_err_cnt_increase(hcc_service_type service_type)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    unsigned long flags;
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    if (service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    osal_spin_lock_irqsave(&hcc_dfx_stat->hcc_dfx_lock, &flags);
    hcc_dfx_stat->service_stat[service_type].rx_err_cnt++;
    osal_spin_unlock_irqrestore(&hcc_dfx_stat->hcc_dfx_lock, &flags);
#else
    uapi_unused(service_type);
#endif
}

#ifdef CONFIG_HCC_SUPPORT_DFX
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
td_void hcc_dfx_queue_fc_info_print(hcc_trans_queue *queue)
{
    if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_DATA) {
        hcc_dfx_print("fc:%d\r\n", queue->fc_para.fc_back_para.flow_ctrl_open);
        hcc_dfx_print("fc flag:%d\r\n", queue->fc_para.fc_back_para.is_stopped);
        hcc_dfx_print("fc_on_cnt: %d\r\n", queue->fc_para.fc_back_para.fc_on_cnt);
        hcc_dfx_print("fc_off_cnt: %d\r\n", queue->fc_para.fc_back_para.fc_off_cnt);
    }
    if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_CREDIT) {
        hcc_dfx_print("credit: %d, bottom value: %d\r\n",
            queue->fc_para.credit, queue->queue_ctrl->credit_bottom_value);
    }
}

td_void hcc_dfx_queue_ctrl_info_print(hcc_trans_queue *queue)
{
    hcc_dfx_print("service_type: %d\r\n", queue->queue_ctrl->service_type);
    hcc_dfx_print("transfer_mode: %d\r\n", queue->queue_ctrl->transfer_mode);
    hcc_dfx_print("fc_enable: %d\r\n", queue->queue_ctrl->fc_enable);
    hcc_dfx_print("flow_type: %d\r\n", queue->queue_ctrl->flow_type);
    hcc_dfx_print("low_waterline: %d\r\n", queue->queue_ctrl->low_waterline);
    hcc_dfx_print("high_waterline: %d\r\n", queue->queue_ctrl->high_waterline);
    hcc_dfx_print("group_id: %d\r\n", queue->queue_ctrl->group_id);
    hcc_dfx_print("burst_limit: %d\r\n", queue->queue_ctrl->burst_limit);
    hcc_dfx_print("credit_bottom_value: %d\r\n", queue->queue_ctrl->credit_bottom_value);
}
#endif
#endif

td_void hcc_dfx_queue_info_print(hcc_channel_name chl, hcc_queue_dir dir, hcc_queue_type q_id)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    hcc_trans_queue *queue;
    hcc_queue_stat *queue_stat;
    if (q_id >= hcc_dfx_stat->que_max_cnt || dir >= HCC_DIR_COUNT) {
        return;
    }

    queue = &(hcc_get_handler(chl)->hcc_resource.hcc_queues[dir][q_id]);
    queue_stat = &hcc_get_dfx()->queue_stat[dir][q_id];
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    hcc_dfx_queue_ctrl_info_print(queue);
    hcc_dfx_queue_fc_info_print(queue);
#endif
    hcc_dfx_print("current queue_len: %d\r\n", hcc_list_len(&queue->queue_info));
    hcc_dfx_print("total_pkts: %d\r\n", queue_stat->total_pkts);
    hcc_dfx_print("loss_pkts: %d\r\n", queue_stat->loss_pkts);
    hcc_dfx_print("alloc_succ_cnt: %d\r\n", queue_stat->alloc_succ_cnt);
    hcc_dfx_print("alloc_fail_cnt: %d\r\n", queue_stat->alloc_fail_cnt);
#else
    uapi_unused(chl);
    uapi_unused(dir);
    uapi_unused(q_id);
#endif
}

td_void hcc_dfx_service_info_print(hcc_service_type service_type)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    hcc_dfx *hcc_dfx_stat = hcc_get_dfx();
    hcc_service_stat *serv_stat;
    if (service_type >= hcc_dfx_stat->srv_max_cnt) {
        return;
    }
    serv_stat = &hcc_dfx_stat->service_stat[service_type];
    hcc_dfx_print("alloc_succ_cnt: %d\r\n", serv_stat->alloc_succ_cnt);
    hcc_dfx_print("alloc_cb_cnt: %d\r\n", serv_stat->alloc_cb_cnt);
    hcc_dfx_print("alloc_fail_cnt: %d\r\n", serv_stat->alloc_fail_cnt);
    hcc_dfx_print("free_cnt: %d\r\n", serv_stat->free_cnt);

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
    hcc_dfx_print("start_subq_cnt: %d\r\n", serv_stat->start_subq_cnt);
    hcc_dfx_print("stop_subq_cnt: %d\r\n", serv_stat->stop_subq_cnt);
#endif
    hcc_dfx_print("exp_rx_cnt: %d\r\n", serv_stat->exp_rx_cnt);
    hcc_dfx_print("rx_cnt: %d\r\n", serv_stat->rx_cnt);
    hcc_dfx_print("bus_tx_succ: %d\r\n", serv_stat->bus_tx_succ);

    hcc_dfx_print("rx_err_cnt: %d\r\n", serv_stat->rx_err_cnt);
#else
    uapi_unused(service_type);
#endif
}

td_void hcc_bus_dfx_statics_print(hcc_channel_name chl)
{
#ifdef CONFIG_HCC_SUPPORT_DFX
    hcc_handler *hcc = hcc_get_handler(chl);
    hcc_bus_dfx_f func;
    hcc_trans_queue *queue = TD_NULL;
    hcc_inner_stat *inner_stat = &hcc_get_dfx()->inner_stat;
    td_u8 i;
    hcc_queue_cfg *q_cfg;
    td_u32 queue_len;

    if (hcc == TD_NULL) {
        return;
    }
    func = hcc->bus->hcc_bus_dfx;
    q_cfg = hcc->que_cfg;
    hcc_dfx_print("alloc_unc ok tx[%u],rx[%u]\r\n", inner_stat->unc_alloc_succ[HCC_DIR_TX],
        inner_stat->unc_alloc_succ[HCC_DIR_RX]);
    hcc_dfx_print("alloc_unc err tx[%u],rx[%u]\r\n", inner_stat->unc_alloc_fail[HCC_DIR_TX],
        inner_stat->unc_alloc_fail[HCC_DIR_RX]);
    hcc_dfx_print("free unc: %u\r\n", inner_stat->unc_free_cnt);
    hcc_dfx_print("free_mem: %u\r\n", inner_stat->mem_free_cnt);

    for (i = 0; i < (hcc->que_max_cnt << 1); i++) {
        if (q_cfg[i].queue_id >= hcc->que_max_cnt || q_cfg[i].dir >= HCC_DIR_COUNT) {
            hcc_printf_err_log("q:%d\r\n", q_cfg[i].queue_id);
            continue;
        }

        queue = &hcc->hcc_resource.hcc_queues[q_cfg[i].dir][q_cfg[i].queue_id];
        queue_len = queue->queue_info.qlen;
        if (queue_len > 0) {
            hcc_dfx_print("queue id[%u] dir[%u] len[%u]\r\n", q_cfg[i].queue_id, q_cfg[i].dir, queue_len);
        }
        queue_len = queue->send_head.qlen;
        if (queue_len > 0) {
            hcc_dfx_print("sendind id[%u] len[%u]\r\n", q_cfg[i].queue_id, queue_len);
        }
    }
    if (func) {
        func();
    }
#else
    uapi_unused(chl);
#endif
}
