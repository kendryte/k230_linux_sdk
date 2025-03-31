/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: host plat hcc service source file
 * Author: Huanghe
 * Create: 2022-12-27
 */
#include "hcc_comm.h"
#include "plat_hcc_srv.h"
#if defined(CONFIG_HCC_SUPPORT_UART)
#include "hcc_adapt_uart.h"
#endif
#define HCC_DMA_DATA_ALIGN 32

static td_u8* plat_msg_alloc(td_u8 *buf, td_u16 len, td_u16 hcc_msg_id, td_u16 *len_align)
{
    td_s32 ret;
    td_u8 *data;
    hcc_header *hcc_head;
    hcc_plat_cfg_msg_header *plat_msg;
    size_t real_len = len + sizeof(hcc_plat_cfg_msg_header) + sizeof(hcc_header);
#ifdef CONFIG_HCC_SUPPORT_UART
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc == TD_NULL) {
        return TD_NULL;
    }
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        real_len += sizeof(uart_packet_head) + 1;
    }
#endif
    real_len = oal_round_up(real_len, HCC_DMA_DATA_ALIGN);
    data = osal_kmalloc(real_len, OSAL_GFP_KERNEL);
    if (data == OSAL_NULL) {
        hcc_printf_err_log("osal_kmalloc failed\n");
        return OSAL_NULL;
    }
    hcc_head = (hcc_header *)data;
#ifdef CONFIG_HCC_SUPPORT_UART
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        hcc_head = (hcc_header *)((td_u8 *)hcc_head + sizeof(uart_packet_head));
    }
#endif
    plat_msg = (hcc_plat_cfg_msg_header *)(hcc_head + 1);
    plat_msg->msg_id = hcc_msg_id;
    plat_msg->len = len;
    *len_align = real_len;
    ret = memcpy_s((td_u8 *)(plat_msg + 1), len, buf, len);
    if (ret != EOK) {
        hcc_printf_err_log("memcpy_s failed\n");
        osal_kfree(data);
        return OSAL_NULL;
    }
    return data;
}

static inline td_void hcc_plat_service_init_hdr(hcc_transfer_param *param, td_u32 main_type, td_u8 subtype,
    td_u8 queue_id, td_u16 fc_flag)
{
    param->service_type = main_type;
    param->sub_type = subtype;
    param->queue_id = queue_id;
    param->fc_flag = fc_flag;
}

td_u32 plat_msg_hcc_send(td_u8 *buf, td_u16 len, td_u16 hcc_plat_msg_id)
{
    td_u8 *msg_data;
    td_u16 len_align;
    td_u32 ret;
    hcc_transfer_param hcc_tx_param;

    msg_data = plat_msg_alloc(buf, len, hcc_plat_msg_id, &len_align);
    if (msg_data == OSAL_NULL) {
        return EXT_ERR_FAILURE;
    }
    hcc_plat_service_init_hdr(&hcc_tx_param, HCC_ACTION_TYPE_OAM, PLAT_MSG_TRANSFER, HCC_QUEUE_INTERNAL, HCC_FC_NONE);
    hcc_tx_param.user_param = OSAL_NULL;
    ret = (td_u32)hcc_tx_data(HCC_CHANNEL_AP, msg_data, len_align, &hcc_tx_param);
    if (ret != EXT_ERR_SUCCESS) {
        osal_kfree(msg_data);
    }
    return ret;
}

hcc_plat_msg_callbck g_hcc_host_plat_table[D2H_PLAT_CFG_MSG_END] = {0};
td_u32 hcc_plat_msg_register(td_u16 msg_id, hcc_plat_msg_callbck cb)
{
    if (msg_id >= D2H_PLAT_CFG_MSG_END) {
        return EXT_ERR_NOT_IMPLEMENTED;
    }
    g_hcc_host_plat_table[msg_id] = cb;
    return EXT_ERR_SUCCESS;
}
static td_u32 exce_plat_msg_callback(td_u16 msg_id, td_u8 *data, osal_u16 len)
{
    if (msg_id >= D2H_PLAT_CFG_MSG_END  || g_hcc_host_plat_table[msg_id] == OSAL_NULL) {
        return EXT_ERR_NOT_IMPLEMENTED;
    }
    return g_hcc_host_plat_table[msg_id](data, len);
}
td_u32 hcc_plat_host_rx_proc(hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len, td_u8 *user_param)
{
    osal_u32 ret;
    hcc_header *hcc_head;
    td_u16 msg_id;
    hcc_plat_cfg_msg_header *plat_msg;
    if (buf == OSAL_NULL) {
        hcc_printf_err_log("hcc plat buf null\n");
        return EXT_ERR_FAILURE;
    }
    hcc_head = (hcc_header *)buf;
    plat_msg = (hcc_plat_cfg_msg_header *)(hcc_head + 1);
    msg_id = plat_msg->msg_id;
    ret = exce_plat_msg_callback(plat_msg->msg_id, (td_u8 *)(plat_msg + 1), plat_msg->len);
    osal_kfree(buf);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_printf_err_log("%s failed, id[%u], ret[0x%x]\n", __func__, msg_id, ret);
        return ret;
    }
    return EXT_ERR_SUCCESS;
}