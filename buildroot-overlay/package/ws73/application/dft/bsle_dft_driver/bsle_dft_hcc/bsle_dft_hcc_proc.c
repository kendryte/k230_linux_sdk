/*
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 */
#include "hcc_comm.h"
#include "hcc_if.h"
#include "hcc_adapt.h"
#include "bsle_dft_chan_kernel.h"
#include "td_base.h"

/**
  * @function : sle_hcc_adapt_free
  * @brief    : liteos的bsle host向device发送的消息,hcc需要额外分配sdio传输相应的32字节对齐的内存空间,
  * @brief    : 在hcc发送消息成功后,会进入此回调函数
  * @param    : queue_id [input] hcc通道收发消息类型id
  * @param    : buf [input]  需要被释放的hcc通道申请的内存空间
  * @param    : user_param [input]  需要被释放的bth申请的内存空间
  * @return   : void
  */
static td_void sle_hcc_adapt_free(hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param)
{
    if (user_param != TD_NULL) {
        osal_kfree(user_param);
    }
    if (buf != TD_NULL) {
        osal_kfree(buf);
    }
}

/**
  * @function : sle_hcc_adapt_alloc
  * @brief    : bsle device向liteos发送的消息,hcc接收到原始消息需要申请32字节对齐的内存空间
  * @param    : queue_id [input]  hcc通道收发消息类型id
  * @param    : len [input]  hcc申请保存数据的大小
  * @param    : buf [input]  hcc申请保存数据的内存空间地址
  * @param    : user_param [input]  当前未使用,业务可能需要的额外内存空间可以指向这里
  * @return   : 内存分配正常返回EXT_ERR_SUCCESS
  */
static td_u32 sle_hcc_adapt_alloc(hcc_queue_type queue_id, td_u32 len, td_u8 **buf, td_u8 **user_param)
{
    *buf = (td_u8 *)osal_kmalloc(len, OSAL_GFP_KERNEL);
    if (*buf == TD_NULL) {
        hcc_debug("sle_hcc_adapt_alloc failed\r\n");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

/**
  * @function : sle_hcc_adapt_rx
  * @brief    : bsle host的rx处理,user_param和buf共享同一片内存空间,user_param即是bsle_hcc_adapt_alloc函数中的buf
  * @param    : queue_id [input]  hcc通道收发消息类型id
  * @param    : len [input]  device发送消息的实际长度
  * @param    : buf [input]  device发送的实际消息的内存起始地址
  * @param    : user_param [input]  指向的是hcc原始分配的32字节对齐的起始内存空间地址
  * @return   : 接收业务处理正常返回EXT_ERR_SUCCESS
  */
static td_u32 sle_hcc_adapt_rx(hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len, td_u8 *user_param)
{
    if (buf == TD_NULL || user_param == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    sle_hci_chan_recv_frame(buf, len);

    osal_kfree(user_param);
    return EXT_ERR_SUCCESS;
}
static td_u32 hcc_bt_rx_proc(hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len, td_u8 *user_param)
{
    if (buf == TD_NULL || user_param == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    ble_hci_chan_recv_frame(buf, len);
    osal_kfree(user_param);
    return EXT_ERR_SUCCESS;
}
hcc_adapt_ops g_hcc_ble_adapt = {
    .free = sle_hcc_adapt_free,
    .alloc = sle_hcc_adapt_alloc,
    .start_subq = TD_NULL,
    .stop_subq = TD_NULL,
    .rx_proc = hcc_bt_rx_proc,
};

hcc_adapt_ops g_sle_hcc_adapt = {
    .free = sle_hcc_adapt_free,
    .alloc = sle_hcc_adapt_alloc,
    .start_subq = TD_NULL,
    .stop_subq = TD_NULL,
    .rx_proc = sle_hcc_adapt_rx,
};

/* Send frames from HCI layer */
void sle_hci_send_frame(uint8_t *data_buf, uint16_t len)
{
    int ret;
    /* 已和平台确认，sub_type和fc_flag值为0的话，则不生效 */
    hcc_transfer_param sle_transfer_param = {0};

    sle_transfer_param.service_type = HCC_ACTION_TYPE_SLE;
    /* sub_type类型业务可以在service_type上自定义，在device侧去区分做不同业务 */
    sle_transfer_param.sub_type = 0;
    sle_transfer_param.queue_id = SLE_DATA_QUEUE;
    /* fc_flag代表流控类型，当前蓝牙未使用 */
    sle_transfer_param.fc_flag = 0;
    sle_transfer_param.user_param = NULL;

    /**
     * 调用完tx_data后,若hcc返回成功,则进入到hcc_sle_rx_proc释放hcc通道申请的buf。
     * 因为netlink机制,此处不用释放data_buf.
     */
    ret = hcc_bt_tx_data(HCC_CHANNEL_AP, data_buf, len, &sle_transfer_param);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_debug("send tx data failed\n");
    }
}

void ble_hci_send_frame(uint8_t *data_buf, uint16_t len)
{
    int ret;
    /* 已和平台确认，sub_type和fc_flag值为0的话，则不生效 */
    hcc_transfer_param sle_transfer_param = {0};

    sle_transfer_param.service_type = HCC_ACTION_TYPE_BT;
    /* sub_type类型业务可以在service_type上自定义，在device侧去区分做不同业务 */
    sle_transfer_param.sub_type = 0;
    sle_transfer_param.queue_id = BT_DATA_QUEUE;
    /* fc_flag代表流控类型，当前蓝牙未使用 */
    sle_transfer_param.fc_flag = 0;
    sle_transfer_param.user_param = NULL;

    /**
     * 调用完tx_data后,若hcc返回成功,则进入到hcc_sle_rx_proc释放hcc通道申请的buf。
     * 因为netlink机制,此处无不用释放data_buf.
     */
    ret = hcc_bt_tx_data(HCC_CHANNEL_AP, data_buf, len, &sle_transfer_param);
    if (ret != EXT_ERR_SUCCESS) {
        hcc_debug("send tx data failed\n");
    }
}


ext_errno sle_hcc_init(void)
{
    return hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_SLE, &g_sle_hcc_adapt);
}

ext_errno ble_hcc_init(void)
{
    return hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BT, &g_hcc_ble_adapt);
}
