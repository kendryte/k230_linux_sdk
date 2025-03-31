/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: the header file of hcc public interfacer.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_INTERFACE_H
#define HCC_INTERFACE_H
#include "td_type.h"
#include "hcc_types.h"
#include "hcc_comm.h"
#include "hcc_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    hcc_channel_name channel_name;
    hcc_bus_type bus_type;
    hcc_queue_cfg *queue_cfg;
    td_u8 rsv;
    td_u8 queue_len;
    td_u16 unc_pool_size;
} hcc_channel_init;
/*
 * HCC通道初始化/去初始化
 * channel_name: 指定要初始化/去初始化的通道
 */
td_u32 hcc_init(hcc_channel_init *init);
td_void hcc_deinit(hcc_channel_name channel_name);

/* 获取逻辑通道句柄 */
hcc_handler *hcc_get_handler(hcc_channel_name channel_name);

/*
 * hcc_service_type :  业务枚举类型
 * hcc_channel_name :  通道枚举类型
 * hcc_adapt_ops    :  业务回调接口
 */
/*
 * 消息机制: 该机制只支持通知 msg id, 不支持传数据，若需要传输数据请使用 hcc_tx_data 接口;
 * 参数service_type: 业务类型，由于每个业务使用的bus可能不同，因此需要传入 service type, hcc才能用对应的bus传输.
 */
ext_errno hcc_message_register(hcc_channel_name channel_name, hcc_service_type service_type,
    hcc_rx_msg_type msg_id, hcc_msg_rx cb, td_u8 *cb_data);
ext_errno hcc_message_unregister(hcc_channel_name channel_name, hcc_service_type service_type,
    hcc_rx_msg_type msg_id);

/*
 * buf是hcc head + payloadl; hcc head由调用者分配空间，由hcc填充；
 * len:hcc head + payload的总长度; len一定要大于hcc头的长度
 * 返回值: 成功:EXT_ERR_SUCCESS， 失败:其他值
 * 若返回失败需要接口调用者释放内存；
 * 注:所有hcc收到的pkt长度都是包含了hcc头结构的长度。
 */
ext_errno hcc_tx_data(hcc_channel_name channel_name, td_u8 *buf, td_u16 len, hcc_transfer_param *param);
ext_errno hcc_bt_tx_data(hcc_channel_name channel_name, td_u8 *buf, td_u16 len, hcc_transfer_param *param);

/*
 * 可使用该接口获取 hcc 头结构的长度, hcc_tx_data 接口需要预留 hcc头结构的空间;
 * Rx callback 中业务也可以使用该接口获取hcc头长度，以偏移到实际 payload 所在位置;
 */
td_u16 hcc_get_head_len(td_void);

/*
 * 使能/禁能调度接收线程，禁能后将使用直调方式
 */
td_void hcc_enable_rx_thread(hcc_channel_name channel_name, td_bool enable);

/*
 * 使能/禁能调度发送线程，禁能后将使用直调方式
 */
td_void hcc_enable_tx_thread(hcc_channel_name channel_name, td_bool enable);

osal_u8 hcc_get_state(hcc_channel_name channel_name);

/* 获取hcc队列中TX方向包的数量，主要包括UDP TCP TCP ACK的三种队列 */
td_u32 hcc_get_tx_mpdu_count(osal_void);

/* 获取hcc调度阈值 */
td_u32 hcc_get_tx_sched_num(osal_void);

/* HCC state */
typedef enum {
    HCC_OFF = 0, /* 关闭状态，不允许调用HCC接口发送数据 */
    HCC_ON, /* 打开状态，允许调用HCC接口收发数据 */
    HCC_EXCEPTION, /* 异常状态 */
    HCC_BUS_FORBID, /* 禁止hcc bus reg 操作 */
    HCC_STATE_END
} hcc_state_enum;

td_void hcc_switch_status(hcc_state_enum value);

/*
 * 获取通道是否繁忙状态
 */
td_bool hcc_chan_is_busy(hcc_channel_name channel_name);

td_void hcc_enable_switch(hcc_channel_name channel_name, td_bool enable);

#define hcc_enable() hcc_enable_switch(HCC_CHANNEL_AP, TD_TRUE)
#define hcc_disable() hcc_enable_switch(HCC_CHANNEL_AP, TD_FALSE)

td_void hcc_send_panic_msg(td_void);
td_s32 hcc_resume_xfer(hcc_channel_name channel_name);
td_s32 hcc_stop_xfer(hcc_channel_name channel_name);

td_void hcc_stop_xfer_etc(td_void);
td_void hcc_resume_xfer_etc(td_void);

/*************************************************bus layer interface*******************************************/
ext_errno hcc_send_message(hcc_channel_name channel_name, hcc_tx_msg_type msg_id, hcc_service_type service_type);

/*
 * 读寄存器
 * chl: 指定通道的寄存器
 * addr: 寄存器地址数组（sdio:建议高16bit配置为0x4001，低2bit配置为0，
 *                       sdio只取addr中第2-15bit的值，其他bit的值忽略，
 *                      因此即使不按照建议配置addr，读取的也依然是0x4001XXXX寄存器。）
 * value: 保存寄存器值数组
 */
ext_errno hcc_read_reg(hcc_channel_name channel_name, td_u32 addr, td_u32 *value);

/*
 * 写寄存器
 * chl: 指定通道的寄存器
 * addr: 寄存器地址数组（sdio:建议高16bit配置为0x4001，低2bit配置为0，
 *                      sdio只取addr中第2-15bit的值，其他bit的值忽略，
 *                      因此即使不按照建议配置addr，被写入的也依然是0x4001XXXX寄存器。）
 * value: 保存寄存器值数组
 */
ext_errno hcc_write_reg(hcc_channel_name channel_name, td_u32 addr, td_u32 value);

/*
 * 读寄存器, 跳过hcc_state检查
 * chl: 指定通道的寄存器
 * addr: 寄存器地址数组（sdio:建议高16bit配置为0x4001，低2bit配置为0，
 *                       sdio只取addr中第2-15bit的值，其他bit的值忽略，
 *                      因此即使不按照建议配置addr，读取的也依然是0x4001XXXX寄存器。）
 * value: 保存寄存器值数组
 */
ext_errno hcc_read_reg_force(hcc_channel_name channel_name, td_u32 addr, td_u32 *value);

/*
 * 写寄存器, 跳过hcc_state检查
 * chl: 指定通道的寄存器
 * addr: 寄存器地址数组（sdio:建议高16bit配置为0x4001，低2bit配置为0，
 *                      sdio只取addr中第2-15bit的值，其他bit的值忽略，
 *                      因此即使不按照建议配置addr，被写入的也依然是0x4001XXXX寄存器。）
 * value: 保存寄存器值数组
 */
ext_errno hcc_write_reg_force(hcc_channel_name channel_name, td_u32 addr, td_u32 value);

/*
 * 获取通道传输的数据对齐长度
 */
td_u32 hcc_get_channel_align_len(hcc_channel_name channel_name);

/*
 * 业务数据包长超出bus允许的最大包长，业务再自己分发.
 * 注:所有hcc收到的pkt长度都是包含了hcc头结构的长度。
 */
td_u32 hcc_get_pkt_max_len(hcc_channel_name channel_name);

/*
 * 设置tx总线聚合层个数.
 */
td_void hcc_set_tx_sched_count(hcc_channel_name channel_name, td_u8 count);

static inline td_void hcc_sched_msg(td_void)
{
    return;
}

/*************************************************service layer interface*******************************************/

/*
 * hcc_service_type :  业务枚举类型
 * hcc_channel_name :  通道枚举类型
 * hcc_adapt_ops    :  业务回调接口
 */
/*
 * 业务初始化，业务类型对应的adapt结构，包括业务的内存分配、流控处理、Rx calbback;
 */
ext_errno hcc_service_init(hcc_channel_name channel_name, hcc_service_type service_type, hcc_adapt_ops *adapt);
ext_errno hcc_service_deinit(hcc_channel_name channel_name, hcc_service_type service_type);
/*
 * 更新流控credit值
 * 对流控方式为  HCC_FLOWCTRL_CREDIT  的队列才有效；
 * RX端在内存管理中更新credit值
 * TX端在发送时获取credit值，根据credit的水线决定是否发送；
 * 注意: 如果要使用该流控方式，需要在 hcc_service_init 之后使用 hcc_service_update_credit 更新下初始值，否则为0
 */
td_void hcc_service_update_credit(hcc_channel_name channel_name, hcc_service_type serv, td_u32 credit);
td_s32 hcc_service_get_credit(hcc_channel_name channel_name, hcc_service_type serv, td_u32 *credit);
td_bool hcc_service_is_busy(hcc_channel_name channel_name, hcc_service_type service_type);
td_void hcc_service_enable_switch(hcc_channel_name channel_name, hcc_service_type service_type, td_bool enable);

/*************************************************flow ctrl interface*******************************************/
/*
 * 获取/设置 流控水线值
 * chl: 逻辑通道
 * direction: 队列方向
 * q_id: 队列ID
 * low_line: 低水线值   // TODO  增加使用说明
 * high_line: 高水线值
 */
ext_errno hcc_flow_ctrl_set_water_line(hcc_channel_name channel_name, hcc_queue_dir direction, td_u8 q_id,
                                       td_u8 low_line, td_u8 high_line);
ext_errno hcc_flow_ctrl_get_water_line(hcc_channel_name channel_name, hcc_queue_dir direction, td_u8 q_id,
                                       td_u8 *low_line, td_u8 *high_line);
td_void hcc_flow_ctrl_credit_register(td_void *cb);
td_void hcc_flow_ctrl_credit_unregister(td_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* HCC_INTERFACE_H */
