/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc adapt for hcc private interface.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#include "td_base.h"
#include "hcc_if.h"

#include "soc_osal.h"
#include "osal_list.h"
#include "osal_def.h"
#include "securec.h"

#include "hcc_cfg.h"
#include "hcc_cfg_if.h"
#include "hcc_comm.h"
#include "hcc_service.h"
#include "hcc_list.h"
#include "hcc_flow_ctrl.h"
#include "hcc.h"
#include "hcc_channel.h"
#include "hcc_dfx.h"
#include "hcc_adapt.h"

hcc_unc_struc *hcc_alloc_unc_buf(hcc_handler *hcc)
{
    hcc_data_queue *unc_pool = hcc->unc_pool_head;
    hcc_unc_struc *unc_buf = TD_NULL;
    if (unc_pool == TD_NULL) {
        unc_buf = (hcc_unc_struc *)osal_kmalloc(sizeof(hcc_unc_struc), OSAL_GFP_ATOMIC);
    } else {
        unc_buf = hcc_list_dequeue(unc_pool);
    }
    return unc_buf;
}

td_void hcc_free_unc_buf(hcc_handler *hcc, hcc_unc_struc *unc_buf)
{
    hcc_data_queue *unc_pool = hcc->unc_pool_head;
    if (unc_pool == TD_NULL) {
        osal_kfree(unc_buf);
    } else {
        hcc_list_add_tail(unc_pool, unc_buf);
    }
    hcc_dfx_unc_free_cnt_increase();
}

td_u32 hcc_adapt_alloc_priv_buf(
    hcc_handler *hcc, hcc_queue_type queue_id, td_u32 buf_len, td_u8 **buf, td_u8 **user_param)
{
    hcc_service_type serv_type = hcc_fuzzy_trans_queue_2_service(hcc, queue_id);
    hcc_serv_info *serv_info = hcc_get_serv_info(hcc, serv_type);
    if (serv_info == TD_NULL || buf == TD_NULL || user_param == TD_NULL) {
        return EXT_ERR_FAILURE;
    }

    if (serv_info->adapt == TD_NULL) {
        hcc_printf_err_log("srv-%d null\n", serv_type);
        return EXT_ERR_FAILURE;
    }

    if (serv_info->adapt->alloc == TD_NULL) {
        hcc_printf_err_log("srv alloc null- %d\n", serv_type);
        return EXT_ERR_FAILURE;
    }

    hcc_dfx_service_alloc_cb_cnt_increase(serv_type);
    return serv_info->adapt->alloc(queue_id, buf_len, buf, user_param);
}

td_void hcc_adapt_free_priv_buf(hcc_handler *hcc, hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param)
{
    hcc_service_type serv_type = hcc_fuzzy_trans_queue_2_service(hcc, queue_id);
    hcc_serv_info *serv_info = hcc_get_serv_info(hcc, serv_type);
    if (serv_info == TD_NULL) {
        hcc_printf_err_log("srv err, q-%d, srv-%d\n", queue_id, serv_type);
        return;
    }

    if (serv_info->adapt == TD_NULL) {
        hcc_printf_err_log("srv- %d null\n", serv_type);
        return;
    }

    if (serv_info->adapt->free != TD_NULL) {
        serv_info->adapt->free(queue_id, buf, user_param);
        hcc_dfx_service_free_cnt_increase(serv_type);
        return;
    }
    hcc_printf_err_log("srv free-%d null\n", serv_type);
}

hcc_unc_struc *hcc_adapt_alloc_unc_buf(hcc_handler *hcc, td_u32 len, hcc_queue_type queue_id)
{
    hcc_service_type serv_type = hcc_fuzzy_trans_queue_2_service(hcc, queue_id);
    td_u8 *buf = TD_NULL;
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_transfer_param param = {
        .service_type = serv_type,
        .queue_id = queue_id,
    };

    if (hcc_adapt_alloc_priv_buf(hcc, queue_id, len, &buf, &param.user_param) != EXT_ERR_SUCCESS) {
        hcc_dfx_service_alloc_cnt_increase(param.service_type, queue_id, TD_FALSE);
        return TD_NULL;
    }

    hcc_dfx_service_alloc_cnt_increase(param.service_type, queue_id, TD_TRUE);
    unc_buf = hcc_alloc_unc_buf(hcc);
    if (unc_buf == TD_NULL) {
        hcc_adapt_free_priv_buf(hcc, queue_id, buf, param.user_param);
        hcc_dfx_unc_alloc_cnt_increase(HCC_DIR_RX, TD_FALSE);
    } else {
        hcc_init_unc_buf(unc_buf, buf, len, &param);
        hcc_dfx_unc_alloc_cnt_increase(HCC_DIR_RX, TD_TRUE);
    }
    return unc_buf;
}

/*****************************************************************************
 功能描述  : 释放私有数据结构类型
 输入参数  : data 通用数据结构指针
 返 回 值  : 无
*****************************************************************************/
td_void hcc_adapt_mem_free(hcc_handler *hcc, hcc_unc_struc *unc_buf)
{
    if (unc_buf == TD_NULL || hcc == TD_NULL) {
        hcc_printf_err_log("param err\n");
        return;
    }

    hcc_adapt_free_priv_buf(hcc, unc_buf->queue_id, unc_buf->buf, unc_buf->user_param);
    hcc_free_unc_buf(hcc, unc_buf);
    hcc_dfx_mem_free_cnt_increase();
    return;
}

td_void hcc_unc_buf_enqueue(hcc_handler *hcc, hcc_queue_type queue_id, hcc_unc_struc *unc_buf)
{
    hcc_trans_queue *queue = TD_NULL;
    hcc_printf("%s-%d, bus enqueue unc - %p\r\n", __FUNCTION__, __LINE__, unc_buf);
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc null\r\n");
        return;
    }

    queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_RX][queue_id];
    hcc_list_add_tail(&queue->queue_info, unc_buf);
#ifndef CONFIG_HCC_SUPPORT_NON_OS
    if (hcc->hcc_resource.rx_thread_enable) {
        hcc_sched_transfer(hcc);
    }
#endif
}

ext_errno hcc_bus_rx_handler(hcc_bus *bus, td_u8 queue_id, hcc_data_queue *assembled_head)
{
    hcc_unc_struc *unc_buf = TD_NULL;
    hcc_trans_queue *trans_que = TD_NULL;
    ext_errno ret = EXT_ERR_SUCCESS;
    hcc_handler *hcc = TD_NULL;
    if (bus == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    hcc = (hcc_handler *)bus->hcc;
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc null type:%d\n", bus->bus_type);
        return EXT_ERR_FAILURE;
    }
    if (queue_id >= hcc->que_max_cnt) {
        return EXT_ERR_FAILURE;
    }

    hcc_printf("hcc rx handler\n");
    trans_que = &hcc->hcc_resource.hcc_queues[HCC_DIR_RX][queue_id];
    unc_buf = hcc_list_dequeue(assembled_head);
    while (unc_buf != TD_NULL) {
        if (!hcc_check_header_vaild(hcc, (hcc_header *)unc_buf->buf)) {
            hcc_printf_err_log("data invalid len:%u\r\n", unc_buf->length);
            hcc_adapt_mem_free(hcc, unc_buf);
            unc_buf = hcc_list_dequeue(assembled_head);
            continue;
        }
        /* get the rx buf and enqueue */
        hcc_list_add_tail(&trans_que->queue_info, unc_buf);
        unc_buf = hcc_list_dequeue(assembled_head);
    }

    if (hcc->hcc_resource.rx_thread_enable) {
        /* sched hcc thread */
        ret = hcc_sched_transfer(hcc);
    } else if (!osal_in_interrupt()) {
        hcc_rx_queue_proc(hcc, trans_que);
    }

    return ret;
}
