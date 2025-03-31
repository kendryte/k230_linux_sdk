/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc flow ctrl module completion.
 * Author: CompanyName
 * Create: 2021-06-30
 */

#include "hcc_flow_ctrl.h"
#include "soc_module.h"
#include "soc_osal.h"
#include "osal_def.h"

#include "hcc_comm.h"
#include "hcc_service.h"
#include "hcc_list.h"
#include "hcc.h"
#include "hcc_adapt.h"
#include "hcc_bus.h"
#include "hcc_dfx.h"
#include "hcc_channel.h"
#include "hcc_if.h"

#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL_ACTIVE
td_bool hcc_flow_ctrl_sched_check(hcc_trans_queue *queue)
{
    if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_DATA &&
        !queue->fc_para.fc_back_para.flow_ctrl_open &&
        !hcc_is_list_empty(&queue->queue_info)) {
        return TD_TRUE;
    }

    if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_CREDIT &&
        queue->fc_para.credit > queue->queue_ctrl->credit_bottom_value &&
        !hcc_is_list_empty(&queue->queue_info)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

ext_errno hcc_flow_ctrl_process(hcc_handler *hcc, hcc_trans_queue *queue)
{
    if (!queue->queue_ctrl->fc_enable) {
        return EXT_ERR_HCC_FC_PROC_UNBLOCK;
    }

    if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_DATA &&
        queue->fc_para.fc_back_para.flow_ctrl_open) {
        hcc_printf("can't send data, flow ctrl data effect\r\n");
        return EXT_ERR_HCC_FC_PROC_BLOCK;
    } else if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_CREDIT) {
        if (hcc_bus_get_credit(hcc->bus, &queue->fc_para.credit) != EXT_ERR_SUCCESS) {
            hcc_printf_err_log("get credit failed, q-%d\r\n", queue->queue_id);
            return EXT_ERR_HCC_FC_PROC_BLOCK;
        }

        if (queue->fc_para.credit < queue->queue_ctrl->credit_bottom_value) {
            hcc_printf("can't send data, flow ctrl credit effect\r\n");
            return EXT_ERR_HCC_FC_PROC_BLOCK;
        }
    }
    return EXT_ERR_HCC_FC_PROC_UNBLOCK;
}

td_u32 hcc_flowctrl_on_proc(td_u8 *data)
{
    hcc_handler *hcc = (hcc_handler *)data;
    hcc_queue_dir dir;
    hcc_queue_type q_id;
    hcc_trans_queue *hcc_queue;
    unsigned long flags;

    if (hcc == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    hcc_printf("rx msg flow on\r\n");
    for (dir = 0; dir < HCC_DIR_COUNT; dir++) {
        for (q_id = 0; q_id < hcc->que_max_cnt; q_id++) {
            hcc_queue = &hcc->hcc_resource.hcc_queues[dir][q_id];
            if (hcc_queue->queue_ctrl->flow_type != HCC_FLOWCTRL_DATA ||
                !hcc_queue->queue_ctrl->fc_enable) {
                continue;
            }
            if (!hcc_queue->fc_para.fc_back_para.flow_ctrl_open) {
                osal_spin_lock_irqsave(&hcc_queue->queue_info.data_queue_lock, &flags);
                hcc_queue->fc_para.fc_back_para.flow_ctrl_open = TD_TRUE;
                hcc_queue->fc_para.fc_back_para.fc_off_cnt++;
                osal_spin_unlock_irqrestore(&hcc_queue->queue_info.data_queue_lock, &flags);
                hcc_printf("flowctrl_flag on -> off, can't send\r\n");
            }
        }
    }
    return EXT_ERR_SUCCESS;
}

td_u32 hcc_flowctrl_off_proc(td_u8 *data)
{
    hcc_handler *hcc = (hcc_handler *)data;
    hcc_queue_dir dir;
    hcc_queue_type q_id;
    hcc_trans_queue *hcc_queue;
    unsigned long flags;

    if (hcc == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    hcc_printf("rx msg flow off\r\n");
    for (dir = 0; dir < HCC_DIR_COUNT; dir++) {
        for (q_id = 0; q_id < hcc->que_max_cnt; q_id++) {
            hcc_queue = &hcc->hcc_resource.hcc_queues[dir][q_id];
            if (hcc_queue->queue_ctrl->flow_type != HCC_FLOWCTRL_DATA ||
                !hcc_queue->queue_ctrl->fc_enable) {
                continue;
            }
            if (hcc_queue->fc_para.fc_back_para.flow_ctrl_open) {
                osal_spin_lock_irqsave(&hcc_queue->queue_info.data_queue_lock, &flags);
                hcc_queue->fc_para.fc_back_para.flow_ctrl_open = TD_FALSE;
                hcc_queue->fc_para.fc_back_para.fc_on_cnt++;
                osal_spin_unlock_irqrestore(&hcc_queue->queue_info.data_queue_lock, &flags);
                hcc_printf("flowctrl_flag off -> on, start send\r\n");
                hcc_sched_transfer(hcc);
            }
        }
    }
    return EXT_ERR_SUCCESS;
}
#endif
td_bool fc_below_low_waterline(hcc_trans_queue *hcc_queue)
{
    return hcc_list_len(&hcc_queue->queue_info) < hcc_queue->queue_ctrl->low_waterline;
}

td_bool fc_above_high_waterline(hcc_trans_queue *hcc_queue)
{
    return hcc_list_len(&hcc_queue->queue_info) > hcc_queue->queue_ctrl->high_waterline;
}

td_s32 fc_wait_queue_cond(TD_CONST td_void *param)
{
    hcc_trans_queue *hcc_queue = (hcc_trans_queue *)param;
    return fc_below_low_waterline(hcc_queue);
}

td_void fc_tx_stop_subq(hcc_handler *hcc, hcc_trans_queue *hcc_queue)
{
    hcc_serv_info *serv_info = hcc_get_serv_info(hcc, hcc_queue->queue_ctrl->service_type);
    if (serv_info == TD_NULL) {
        return;
    }

    if (!fc_below_low_waterline(hcc_queue) &&
        (!hcc_queue->fc_para.fc_back_para.is_stopped)) {
        td_ulong flags = 0;
        osal_spin_lock_irqsave(&hcc_queue->queue_info.data_queue_lock, &flags);
        hcc_printf("stop!\n");
        if (serv_info->adapt != TD_NULL) {
            if (serv_info->adapt->stop_subq != TD_NULL) {
                serv_info->adapt->stop_subq(hcc_queue->queue_id);
                hcc_dfx_service_stopsubq_cnt_increase(hcc_queue->queue_ctrl->service_type);
            }
            hcc_queue->fc_para.fc_back_para.is_stopped = TD_TRUE;
        }
        osal_spin_unlock_irqrestore(&hcc_queue->queue_info.data_queue_lock, &flags);
    }
}

td_void hcc_adapt_tx_start_subq(hcc_handler *hcc, hcc_trans_queue *hcc_queue)
{
    td_ulong flags = 0;
    hcc_serv_info *serv_info = hcc_get_serv_info(hcc, hcc_queue->queue_ctrl->service_type);
    hcc_flowctl_start_subq start_subq = TD_NULL;
    if (serv_info == TD_NULL) {
        return;
    }

    if (serv_info->adapt == TD_NULL) {
        return;
    }

    start_subq = serv_info->adapt->start_subq;
    osal_spin_lock_irqsave(&hcc_queue->queue_info.data_queue_lock, &flags);
    if (fc_below_low_waterline(hcc_queue)) {
        if (hcc_queue->fc_para.fc_back_para.is_stopped) {
            hcc_queue->fc_para.fc_back_para.is_stopped = TD_FALSE;
            if (start_subq != TD_NULL) {
                start_subq(hcc_queue->queue_id);
                hcc_dfx_service_startsubq_cnt_increase(hcc_queue->queue_ctrl->service_type);
            }
        }
        osal_wait_wakeup(&hcc->hcc_resource.hcc_fc_wq);
    }
    osal_spin_unlock_irqrestore(&hcc_queue->queue_info.data_queue_lock, &flags);
}

ext_errno hcc_flow_ctrl_pre_proc(hcc_handler *hcc, hcc_transfer_param *param, hcc_trans_queue *hcc_queue)
{
    td_s32 ret;

    if (param->fc_flag == HCC_FC_NONE) {
        return EXT_ERR_SUCCESS;
    }

    if ((param->fc_flag & HCC_FC_WAIT) && osal_in_interrupt()) {
        /* when in interrupt,can't sched! */
        param->fc_flag &= ~HCC_FC_WAIT;
    }

    /* flow control process */
    /* Block if fc */
    if (param->fc_flag & HCC_FC_WAIT) {
        ret = osal_wait_timeout_interruptible(
            &hcc->hcc_resource.hcc_fc_wq, fc_wait_queue_cond, hcc_queue, HCC_FC_PRE_PROC_WAIT_TIME);
        if (ret == 0) {
            hcc_printf_null("[WARN]hcc flow control wait event timeout! too much time locked");
            /* 一旦上层业务也开始流控，hcc本身需要主动解除流控，及时发包，避免空等 */
            if (hcc_queue->fc_para.fc_back_para.flow_ctrl_open == TD_TRUE) {
                unsigned long flags;
                osal_spin_lock_irqsave(&hcc_queue->queue_info.data_queue_lock, &flags);
                hcc_queue->fc_para.fc_back_para.flow_ctrl_open = TD_FALSE;
                hcc_queue->fc_para.fc_back_para.fc_on_cnt++;
                osal_spin_unlock_irqrestore(&hcc_queue->queue_info.data_queue_lock, &flags);
            }
        } else if (ret < 0) {
            hcc_printf_null("wifi task was interrupted by a signal");
            return EXT_ERR_FAILURE;
        }
    }

    /* control net layer if fc */
    if (param->fc_flag & HCC_FC_NET) {
        fc_tx_stop_subq(hcc, hcc_queue);
    }

    /* control net layer if fc */
    if (param->fc_flag & HCC_FC_DROP) {
        /* 10 netbufs to buff */
        if (fc_above_high_waterline(hcc_queue)) {
            hcc_dfx_queue_loss_pkt_increase(HCC_DIR_TX, hcc_queue->queue_id);
            return EXT_ERR_FAILURE;
        }
    }
    return EXT_ERR_SUCCESS;
}

ext_errno hcc_flow_ctrl_module_init(hcc_handler *hcc)
{
    td_u8 dir;
    td_u8 q_id;
    hcc_trans_queue *queue;
    if (hcc == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    for (dir = 0; dir < HCC_DIR_COUNT; dir++) {
        for (q_id = 0; q_id < hcc->que_max_cnt; q_id++) {
            queue = &hcc->hcc_resource.hcc_queues[dir][q_id];
            queue->fc_para.fc_back_para.is_stopped = TD_FALSE;
#ifdef CONFIG_HCC_SUPPORT_FLOW_CONTRL_ACTIVE
            if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_DATA) {
                queue->fc_para.fc_back_para.flow_ctrl_open = TD_FALSE;
            }
            if (queue->queue_ctrl->flow_type == HCC_FLOWCTRL_CREDIT) {
                queue->fc_para.credit = HCC_FLOWCTRL_DEFAULT_CREDIT_TOP_VALUE;
            }
#endif
        }
    }

    if (osal_wait_init(&hcc->hcc_resource.hcc_fc_wq) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

td_void hcc_flow_ctrl_module_deinit(hcc_handler *hcc)
{
    osal_wait_destroy(&hcc->hcc_resource.hcc_fc_wq);
}

ext_errno hcc_flow_ctrl_set_water_line(hcc_channel_name chl, hcc_queue_dir direction, td_u8 q_id,
    td_u8 low_line, td_u8 high_line)
{
    td_ulong flags;
    hcc_handler *hcc = hcc_get_handler(chl);
    if (hcc == TD_NULL) {
        return EXT_ERR_HCC_HANDLER_ERR;
    }

    if (direction >= HCC_DIR_COUNT || q_id >= hcc->que_max_cnt) {
        return EXT_ERR_HCC_PARAM_ERR;
    }

    flags = 0;
    osal_spin_lock_irqsave(&hcc->hcc_resource.hcc_queues[direction][q_id].queue_info.data_queue_lock, &flags);
    hcc->hcc_resource.hcc_queues[direction][q_id].queue_ctrl->low_waterline  = low_line;
    hcc->hcc_resource.hcc_queues[direction][q_id].queue_ctrl->high_waterline = high_line;
    osal_spin_unlock_irqrestore(&hcc->hcc_resource.hcc_queues[direction][q_id].queue_info.data_queue_lock, &flags);
    return EXT_ERR_SUCCESS;
}

ext_errno hcc_flow_ctrl_get_water_line(hcc_channel_name chl, hcc_queue_dir direction, td_u8 q_id,
    td_u8 *low_line, td_u8 *high_line)
{
    hcc_handler *hcc = hcc_get_handler(chl);
    if (hcc == TD_NULL) {
        return EXT_ERR_HCC_HANDLER_ERR;
    }

    if (direction >= HCC_DIR_COUNT || q_id >= hcc->que_max_cnt || low_line == TD_NULL || high_line == TD_NULL) {
        return EXT_ERR_HCC_PARAM_ERR;
    }
    *low_line = hcc->hcc_resource.hcc_queues[direction][q_id].queue_ctrl->low_waterline;
    *high_line = hcc->hcc_resource.hcc_queues[direction][q_id].queue_ctrl->high_waterline;
    return EXT_ERR_SUCCESS;
}

td_void *g_hcc_flow_ctrl_credit_fn = TD_NULL;
td_void hcc_flow_ctrl_credit_register(td_void *cb)
{
    g_hcc_flow_ctrl_credit_fn = cb;
}

td_void hcc_flow_ctrl_credit_unregister(td_void)
{
    g_hcc_flow_ctrl_credit_fn = TD_NULL;
}

#define HCC_FLOW_CTRL_WAIT_INTERVAL 1
#define HCC_FLOW_CTRL_FAIL_LIMIT 1024
td_u32 g_hcc_flow_ctrl_cnt = 0;
td_u16 hcc_flow_ctrl_credit_update(hcc_handler *hcc, hcc_trans_queue *queue, td_u16 remain_pkt_nums)
{
    static td_u16 tx_credit_max = 0;
    hcc_flow_ctrl_credit_cb callback = (hcc_flow_ctrl_credit_cb)g_hcc_flow_ctrl_credit_fn;

    if (callback == TD_NULL || queue->queue_ctrl->fc_enable == TD_FALSE ||
        queue->queue_ctrl->flow_type != HCC_FLOWCTRL_DATA) {
        return remain_pkt_nums;
    }

    if (tx_credit_max <= 1) {
        if (queue->queue_id != DATA_UDP_DATA_QUEUE) {
            if (!hcc_bus_is_busy(hcc->bus, HCC_DIR_TX)) {
                tx_credit_max = callback();
            }
        } else {
            tx_credit_max = callback();
        }
    }
#ifdef WSCFG_BUS_USB
    /* usb need reserve one control buf */
    tx_credit_max = (tx_credit_max >= 1) ? (tx_credit_max - 1) : 0;
#endif
    if (tx_credit_max == 0) {
        g_hcc_flow_ctrl_cnt++;
        if (g_hcc_flow_ctrl_cnt >= HCC_FLOW_CTRL_FAIL_LIMIT) {
            osal_msleep(HCC_FLOW_CTRL_WAIT_INTERVAL); /* 在流控时暂时让出CPU，确保总线不会被读寄存器操作一直占用 */
            g_hcc_flow_ctrl_cnt = 0;
        }
    } else {
        g_hcc_flow_ctrl_cnt = 0;
    }

    remain_pkt_nums = (remain_pkt_nums < tx_credit_max) ? remain_pkt_nums : tx_credit_max;
    tx_credit_max -= remain_pkt_nums;
    return remain_pkt_nums;
}

osal_module_export(hcc_flow_ctrl_set_water_line);
osal_module_export(hcc_flow_ctrl_get_water_line);
osal_module_export(hcc_flow_ctrl_credit_register);
osal_module_export(hcc_flow_ctrl_credit_unregister);
#endif
