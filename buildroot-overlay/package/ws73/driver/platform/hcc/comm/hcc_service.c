/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc service.
 * Author: CompanyName
 * Create: 2021-09-11
 */

#include "securec.h"
#include "osal_def.h"
#include "hcc_comm.h"
#include "hcc_bus.h"
#include "hcc.h"
#include "hcc_channel.h"
#include "hcc_list.h"
#include "hcc_if.h"
#include "hcc_service.h"

hcc_service_list *hcc_get_serv_list(hcc_handler *hcc, hcc_service_type serv_type)
{
    unsigned long flags;
    hcc_service_list *serv_list = TD_NULL;
    if (hcc == TD_NULL) {
        return TD_NULL;
    }
    osal_spin_lock_irqsave(&hcc->hcc_serv.service_lock, &flags);
    serv_list = (hcc_service_list *)hcc->hcc_serv.service_list.next;
    while (&serv_list->service_list != &hcc->hcc_serv.service_list) {
        if (serv_list->service_info->service_type == serv_type) {
            osal_spin_unlock_irqrestore(&hcc->hcc_serv.service_lock, &flags);
            return serv_list;
        }
        serv_list = (hcc_service_list *)serv_list->service_list.next;
    }
    osal_spin_unlock_irqrestore(&hcc->hcc_serv.service_lock, &flags);
    return TD_NULL;
}

hcc_service_list *hcc_get_del_serv_list(hcc_handler *hcc, hcc_service_type serv_type)
{
    unsigned long flags;
    hcc_service_list *serv_list = TD_NULL;
    if (hcc == TD_NULL) {
        return TD_NULL;
    }

    osal_spin_lock_irqsave(&hcc->hcc_serv_del.service_lock, &flags);
    serv_list = (hcc_service_list *)hcc->hcc_serv_del.service_list.next;
    while (&serv_list->service_list != &hcc->hcc_serv_del.service_list) {
        if (serv_list->service_info->service_type == serv_type) {
            osal_spin_unlock_irqrestore(&hcc->hcc_serv_del.service_lock, &flags);
            return serv_list;
        }
        serv_list = (hcc_service_list *)serv_list->service_list.next;
    }
    osal_spin_unlock_irqrestore(&hcc->hcc_serv_del.service_lock, &flags);
    return TD_NULL;
}

hcc_serv_info *hcc_get_serv_info(hcc_handler *hcc, hcc_service_type serv_type)
{
    hcc_service_list *serv_list = hcc_get_serv_list(hcc, serv_type);
    if (serv_list == TD_NULL) {
        return TD_NULL;
    }
    return serv_list->service_info;
}

ext_errno hcc_service_init(hcc_channel_name channel_name, hcc_service_type service_type, hcc_adapt_ops *adapt)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    hcc_serv_info *serv_info = TD_NULL;
    if (hcc == TD_NULL) {
        hcc_printf_err_log("hcc null\n");
        return EXT_ERR_HCC_INIT_ERR;
    }

    if (adapt == TD_NULL) {
        return EXT_ERR_HCC_PARAM_ERR;
    }

    serv_info = hcc_add_service_list(hcc, service_type);
    if (serv_info == TD_NULL) {
        hcc_printf_err_log("add srv-%d\n", service_type);
        return EXT_ERR_FAILURE;
    }
    serv_info->adapt = adapt;
    serv_info->service_type = service_type;
    if (service_type + 1 > hcc->srv_max_cnt) {
        hcc->srv_max_cnt = service_type + 1;
    }
    osal_atomic_set(&serv_info->service_enable, HCC_ENABLE);
    hcc_del_service_list(hcc, service_type);
    hcc_printf("[success] - %s, %d\n", __FUNCTION__, service_type);
    return EXT_ERR_SUCCESS;
}

ext_errno hcc_service_deinit(hcc_channel_name channel_name, hcc_service_type service_type)
{
    unsigned long flags;
    hcc_handler *hcc = hcc_get_handler(channel_name);
    hcc_service_list *del = hcc_get_serv_list(hcc, service_type);
    if (del == TD_NULL) {
        return EXT_ERR_SUCCESS;
    }
    osal_spin_lock_irqsave(&hcc->hcc_serv.service_lock, &flags);
    osal_list_del(&del->service_list);
    osal_spin_unlock_irqrestore(&hcc->hcc_serv.service_lock, &flags);
    osal_spin_lock_irqsave(&hcc->hcc_serv_del.service_lock, &flags);
    osal_list_add_tail(&del->service_list, &hcc->hcc_serv_del.service_list);
    osal_spin_unlock_irqrestore(&hcc->hcc_serv_del.service_lock, &flags);
    return EXT_ERR_SUCCESS;
}

#ifdef CONFIG_HCC_SUPPORT_EXTEND_INTERFACE
td_bool hcc_service_is_busy(hcc_channel_name channel_name, hcc_service_type service_type)
{
    hcc_handler *hcc = hcc_get_handler(channel_name);
    td_u8 index;
    hcc_trans_queue *tx_queue = TD_NULL;
    hcc_trans_queue *rx_queue = TD_NULL;
    if (hcc == TD_NULL) {
        return TD_FALSE;
    }

    for (index = 0; index < hcc->que_max_cnt; index++) {
        tx_queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_TX][index];
        rx_queue = &hcc->hcc_resource.hcc_queues[HCC_DIR_RX][index];
        if ((tx_queue->queue_ctrl->service_type == service_type && !hcc_is_list_empty(&tx_queue->queue_info)) ||
            (rx_queue->queue_ctrl->service_type == service_type && !hcc_is_list_empty(&rx_queue->queue_info))) {
            return TD_TRUE;
        }
    }
    return TD_FALSE;
}

td_void hcc_service_enable_switch(hcc_channel_name channel_name, hcc_service_type service_type, td_bool enable)
{
    hcc_serv_info *serv_info = hcc_get_serv_info(hcc_get_handler(channel_name), service_type);
    if (serv_info != TD_NULL) {
        hcc_change_state(&serv_info->service_enable, enable);
    }
}

td_s32 hcc_service_get_credit(hcc_channel_name channel_name, hcc_service_type serv, td_u32 *credit)
{
    hcc_bus *bus = hcc_get_channel_bus(channel_name);
    if (bus == TD_NULL || credit == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    uapi_unused(serv);
    return hcc_bus_get_credit(bus, credit);
}

osal_module_export(hcc_service_get_credit);
osal_module_export(hcc_service_is_busy);
osal_module_export(hcc_service_enable_switch);
#endif

td_void hcc_service_update_credit(hcc_channel_name channel_name, hcc_service_type serv, td_u32 credit)
{
    hcc_bus *bus = hcc_get_channel_bus(channel_name);
    if (bus == TD_NULL) {
        return;
    }
    uapi_unused(serv);
    hcc_bus_update_credit(bus, credit);
}

hcc_serv_info *hcc_add_service_list(hcc_handler *hcc, hcc_service_type serv_type)
{
    unsigned long flags;
    hcc_service_list *service_new = TD_NULL;
    if (hcc_get_serv_info(hcc, serv_type) != TD_NULL) {
        return TD_NULL;
    }

    service_new = (hcc_service_list *)osal_kmalloc(sizeof(hcc_service_list) + sizeof(hcc_serv_info), OSAL_GFP_KERNEL);
    if (service_new == TD_NULL) {
        return TD_NULL;
    }
    service_new->service_info = (hcc_serv_info *)(service_new + 1);
    memset_s(service_new->service_info, sizeof(hcc_serv_info), 0, sizeof(hcc_serv_info));
    OSAL_INIT_LIST_HEAD(&service_new->service_list);

    osal_spin_lock_irqsave(&hcc->hcc_serv.service_lock, &flags);
    osal_list_add_tail(&service_new->service_list, &hcc->hcc_serv.service_list);
    osal_spin_unlock_irqrestore(&hcc->hcc_serv.service_lock, &flags);
    return service_new->service_info;
}

td_void hcc_del_service_list(hcc_handler *hcc, hcc_service_type serv_type)
{
    unsigned long flags;
    hcc_service_list *del = hcc_get_del_serv_list(hcc, serv_type);
    if (del == TD_NULL) {
        return;
    }
    osal_spin_lock_irqsave(&hcc->hcc_serv_del.service_lock, &flags);
    osal_list_del(&del->service_list);
    osal_spin_unlock_irqrestore(&hcc->hcc_serv_del.service_lock, &flags);
    del->service_info = TD_NULL;
    osal_kfree(del);
}

osal_module_export(hcc_service_update_credit);
osal_module_export(hcc_service_init);
osal_module_export(hcc_service_deinit);
