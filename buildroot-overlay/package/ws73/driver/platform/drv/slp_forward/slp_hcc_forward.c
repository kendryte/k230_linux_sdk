/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: host slp hcc source file
 * Author: Huanghe
 * Create: 2022-12-27
 */

#include "oam_ext_if.h"
#include "plat_hcc_srv.h"
#include "slp_hcc_forward.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_SLP_HCC_TRANSFER_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef _PRE_PLAT_SLP_UART_FORWARD

#define HCC_DMA_DATA_ALIGN 32

typedef struct {
    td_u8 start_symbol;
    td_u8 type;
    td_u8 count_h;
    td_u8 count_l;
} slp_uart_head;

#define SLP_UART_END_SIZE 1
#define SLP_UART_START_SYMBOL 0x7e
#define SLP_UART_END_SYMBOL 0xe7
#define SLP_UART_TYPE 0xff

td_u8 *slp_data_alloc(td_u8 *buf, td_u16 len, td_u16 *len_align)
{
    td_u32 ret;
    td_u8 *data;
    td_u8 *dst_buf;
    slp_hcc_adapt *slp_head;
    slp_uart_head *packet_head;
    td_u16 real_len = (td_u16)(len + sizeof(hcc_header) + sizeof(slp_hcc_adapt) + sizeof(slp_uart_head)
        + SLP_UART_END_SIZE);
    real_len = oal_round_up(real_len, HCC_DMA_DATA_ALIGN);
    data = osal_kmalloc(real_len, OSAL_GFP_KERNEL);
    if (data == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "osal_kmalloc failed\n");
        return OSAL_NULL;
    }
    slp_head = (slp_hcc_adapt *)(data + sizeof(hcc_header));
    packet_head = (slp_uart_head *)(slp_head + 1);
    packet_head->start_symbol = SLP_UART_START_SYMBOL;
    packet_head->type = SLP_UART_TYPE;
    packet_head->count_h = ((len>>8) & 0xff); // 8 移8个bit
    packet_head->count_l = (len & 0xff);

    ret = memcpy_s((td_u8 *)(packet_head + 1), len, buf, len);
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_ANY, "plat msg mcy failed, ret [%u]\n", ret);
        osal_kfree(data);
        return OSAL_NULL;
    }
    dst_buf = data + len + sizeof(hcc_header) + sizeof(slp_hcc_adapt) + sizeof(slp_uart_head);
    *dst_buf = SLP_UART_END_SYMBOL;
    *len_align = real_len;
    slp_head->len = (osal_u32)(len + sizeof(slp_uart_head) + SLP_UART_END_SIZE);
    return data;
}

td_u32 slp_send_data_to_device(td_u8 *buf, td_u16 len)
{
    td_u8 *msg_data;
    td_u16 len_align;
    td_u32 ret;
    hcc_transfer_param hcc_tx_param;

    msg_data = slp_data_alloc(buf, len, &len_align);
    if (msg_data == OSAL_NULL) {
        return EXT_ERR_FAILURE;
    }

    hcc_tx_param.service_type = HCC_ACTION_TYPE_SLP;
    hcc_tx_param.sub_type = 0;
    hcc_tx_param.queue_id = DATA_UDP_BK_QUEUE;
    hcc_tx_param.fc_flag = HCC_FC_NONE;
    hcc_tx_param.user_param = OSAL_NULL;
    ret = (td_u32)hcc_tx_data(HCC_CHANNEL_AP, msg_data, len_align, &hcc_tx_param);
    if (ret != EXT_ERR_SUCCESS) {
        osal_printk("slp_data_hcc_send_device failed, ret [%u]\n", ret);
        osal_kfree(msg_data);
    }
    return ret;
}

osal_u8 is_slp_en_complete = 0;
osal_u32 slp_receive_en_response(osal_u8 *data, osal_u16 len)
{
    osal_printk("receive_response D2H_PLAT_CFG_MSG_SLP_EN_COMPLETE!\n");
    is_slp_en_complete = 1; // 1 触发成功
    return EXT_ERR_SUCCESS;
}

osal_u8 is_slp_uart_baud_set_over = 0;
osal_u32 slp_recv_buad_set_response(osal_u8 *data, osal_u16 len)
{
    is_slp_uart_baud_set_over = 1; // 1 触发成功
    return EXT_ERR_SUCCESS;
}

osal_void slp_hcc_adapt_free(hcc_queue_type queue_id, osal_u8 *buf, osal_u8 *user_param)
{
    osal_kfree(buf);
}

osal_u32 slp_hcc_adapt_alloc(hcc_queue_type queue_id, osal_u32 len, osal_u8 **buf, osal_u8 **user_param)
{
    *buf = (osal_u8 *)osal_kmalloc(len, OSAL_GFP_KERNEL);
    if (*buf == OSAL_NULL) {
        return EXT_ERR_FAILURE;
    }
    *user_param = OSAL_NULL;
    return EXT_ERR_SUCCESS;
}

typedef osal_u32 (*slp_rx_data_callback)(osal_u8 *buf, osal_u32 len);

slp_rx_data_callback slp_rx_proc = OSAL_NULL;

osal_void slp_rx_proc_callback_register(slp_rx_data_callback cb)
{
    if (cb == OSAL_NULL) {
        return;
    }
    slp_rx_proc = cb;
}

osal_u32 slp_hcc_rx_proc(hcc_queue_type queue_id, osal_u8 subtype, osal_u8 *buf, osal_u32 len, osal_u8 *user_param)
{
    hcc_header *hcc_head;
    slp_hcc_adapt *slp_head;
    osal_u32 ret = EXT_ERR_SUCCESS;
    if (buf == TD_NULL) {
        osal_printk("hcc_slp_rx_proc buf null\n");
        return EXT_ERR_FAILURE;
    }
    hcc_head = (hcc_header *)buf;
    slp_head = (slp_hcc_adapt *)(hcc_head + 1);
    if (slp_rx_proc != OSAL_NULL) {
        ret = slp_rx_proc((osal_u8 *)(slp_head + 1), slp_head->len);
    }
    slp_hcc_adapt_free(queue_id, buf, user_param);
    return ret;
}


hcc_adapt_ops g_slp_hcc_adapt_ops = {
    .free = slp_hcc_adapt_free,
    .alloc = slp_hcc_adapt_alloc,
    .start_subq = OSAL_NULL,
    .stop_subq = OSAL_NULL,
    .rx_proc = slp_hcc_rx_proc,
};

osal_u32 slp_hcc_init(osal_void)
{
    osal_u32 ret;
    ret = hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_SLP, &g_slp_hcc_adapt_ops);
    if (ret != EXT_ERR_SUCCESS) {
        osal_printk("device slp service start fail %d!\r\n", ret);
    }
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_SLP_EN_COMPLETE, slp_receive_en_response);
    hcc_plat_msg_register(D2H_SLP_UART_BAUD_SET_COMPLETE, slp_recv_buad_set_response);
    return ret;
}

osal_void slp_hcc_exit(osal_void)
{
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_SLP);
}

osal_u32 slp_power_up(osal_u32 baud_rate)
{
    osal_u32 ret = 0;
    osal_u8 retry_time = 3; // 3 尝试3次，不成功则返回
    is_slp_en_complete = 0;
    while (!is_slp_en_complete && retry_time > 0) {
        ret = plat_msg_hcc_send((osal_u8 *)&baud_rate, sizeof(osal_u32), H2D_PLAT_CFG_MSG_SLP_EN);
        if (ret != EXT_ERR_SUCCESS) {
            osal_printk("plat_msg_hcc_send SLP_EN fail %d!\r\n", ret);
        }
        osal_msleep_uninterruptible(10); // 10 等待10ms等device设置完成回复
        retry_time--;
    }
    if (!is_slp_en_complete) {
        osal_printk("slp_power_up fail!\r\n");
        return EXT_ERR_FAILURE;
    }
    osal_printk("slp_power_up ok!\r\n");
    return EXT_ERR_SUCCESS;
}

osal_u32 slp_uart_baud_modify(osal_u32 baud_rate)
{
    osal_u32 ret = EXT_ERR_SUCCESS;
    osal_u8 retry_time = 3; // 3 尝试3次，不成功则返回
    is_slp_uart_baud_set_over = 0;
    while (!is_slp_uart_baud_set_over && retry_time > 0) {
        ret = plat_msg_hcc_send((osal_u8 *)&baud_rate, sizeof(osal_u32), H2D_PLAT_CFG_MSG_SLP_UART_BAUD_SET);
        if (ret != EXT_ERR_SUCCESS) {
            osal_printk("plat_msg_hcc_send slp_en fail %d!\r\n", ret);
        }
        osal_msleep_uninterruptible(5); // 5 等待5ms等device设置完成回复
        retry_time--;
    }
    if (!is_slp_uart_baud_set_over) {
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

osal_module_export(slp_send_data_to_device);
osal_module_export(slp_power_up);
osal_module_export(slp_uart_baud_modify);

#endif