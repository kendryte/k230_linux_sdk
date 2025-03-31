/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 */
#include "hcc_comm.h"
#include "hcc_if.h"
#include "hcc_adapt.h"
#include "td_base.h"
#include "bt_module.h"
#include "bt_dev.h"
#include "ble_host.h"
#include "ble_hcc.h"

static ble_hcc_rx g_hcc_rx = NULL;

void ble_hcc_rx_register(ble_hcc_rx hcc_rx)
{
    g_hcc_rx = hcc_rx;
}

/**
  * @function : hcc_adapt_bt_free
  * @brief    : liteos的bsle host向device发送的消息,hcc需要额外分配sdio传输相应的32字节对齐的内存空间,
  * @brief    : 在hcc发送消息成功后,会进入此回调函数
  * @param    : queue_id [input] hcc通道收发消息类型id
  * @param    : buf [input]  需要被释放的hcc通道申请的内存空间
  * @param    : user_param [input]  需要被释放的bth申请的内存空间
  * @return   : void
  */
static td_void hcc_adapt_bt_free(hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param)
{
    if (user_param != TD_NULL) {
        osal_kfree(user_param);
    }
    if (buf != TD_NULL) {
        osal_kfree(buf);
    }
}


/**
  * @function : hcc_adapt_bt_alloc
  * @brief    : bsle device向liteos发送的消息,hcc接收到原始消息需要申请32字节对齐的内存空间
  * @param    : queue_id [input]  hcc通道收发消息类型id
  * @param    : len [input]  hcc申请保存数据的大小
  * @param    : buf [input]  hcc申请保存数据的内存空间地址
  * @param    : user_param [input]  业务可能需要的额外内存空间可以指向这里
  * @return   : 内存分配正常返回EXT_ERR_SUCCESS
  */
static td_u32 hcc_adapt_bt_alloc(hcc_queue_type queue_id, td_u32 len, td_u8 **buf, td_u8 **user_param)
{
    *buf = (td_u8 *)osal_kmalloc(len, OSAL_GFP_KERNEL);
    if (*buf == TD_NULL) {
        hcc_debug("sle_hcc_adapt_alloc failed\r\n");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}


/**
  * @function : hcc_bt_rx_proc
  * @brief    : bsle host的rx处理,user_param和buf共享同一片内存空间,user_param即是bsle_hcc_adapt_alloc函数中的buf
  * @param    : queue_id [input]  hcc通道收发消息类型id
  * @param    : len [input]  device发送消息的实际长度
  * @param    : buf [input]  device发送的实际消息的内存起始地址
  * @param    : user_param [input]  指向的是hcc原始分配的32字节对齐的起始内存空间地址
  * @return   : 接收业务处理正常返回EXT_ERR_SUCCESS
  */
static td_u32 hcc_bt_rx_proc(hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len, td_u8 *user_param)
{
    if (buf == TD_NULL || user_param == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    if (get_ble_state() == BLE_OFF) {
        hcc_debug("hcc bt rx proc ble close\r\n");
        osal_kfree(user_param);
        return EXT_ERR_FAILURE;
    }
    if (g_hcc_rx) {
        g_hcc_rx(buf, len);
    }
    osal_kfree(user_param);
    return EXT_ERR_SUCCESS;
}

hcc_adapt_ops g_hcc_ble_adapt = {
    .free = hcc_adapt_bt_free,
    .alloc = hcc_adapt_bt_alloc,
    .start_subq = TD_NULL,
    .stop_subq = TD_NULL,
    .rx_proc = hcc_bt_rx_proc,
};

/* Send frames from HCI layer */
int ble_hci_send_frame(uint8_t *buff, uint32_t len, void *user_param)
{
    int ret;
    hcc_transfer_param bt_transfer_param = { 0 };

    if (get_ble_state() == BLE_OFF) {
        hcc_debug("ble hci send frame close\r\n");
        return EXT_ERR_FAILURE;
    }

    if (len > BT_TX_MAX_FRAME) {
        hcc_debug("len is too large, len=%u\n", len);
        return -EINVAL;
    }

    bt_transfer_param.service_type = HCC_ACTION_TYPE_BT;
    bt_transfer_param.sub_type = 0;
    bt_transfer_param.queue_id = BT_DATA_QUEUE;
    bt_transfer_param.fc_flag = 0;
    bt_transfer_param.user_param = (td_u8 *)user_param;

    // 调用完tx_data后，若hcc返回成功，则进入到hcc_adapt_bt_free释放buf,否则只释放hci组装的skb消息
    ret = hcc_bt_tx_data(HCC_CHANNEL_AP, buff, len, &bt_transfer_param);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_debug("send tx data failed\n");
        return ret;
    }

    return ret;
}

ext_errno ble_hcc_init(void)
{
    return hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BT, &g_hcc_ble_adapt);
}

ext_errno ble_hcc_deinit(void)
{
    ext_errno ret;
    ret = hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BT);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_printf_err_log("hcc_service_deinit HCC_ACTION_TYPE_BT failed!");
    }
    return ret;
}