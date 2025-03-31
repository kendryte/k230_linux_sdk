/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc queue list.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_QUEUE_HEADER
#define HCC_QUEUE_HEADER

#include "td_base.h"
#include "soc_osal.h"
#include "osal_list.h"
#include "hcc_cfg_comm.h"

typedef struct _hcc_data_queue_ {
    struct osal_list_head  data_queue;
    td_u32        qlen;
    osal_spinlock data_queue_lock;
} hcc_data_queue;

typedef struct _hcc_unc_struc {
    td_u8 *buf;
    td_u32 length;
    td_u8 *user_param;
    td_u8  service_type : 4;
    td_u8  sub_type : 4;
    td_u8  queue_id;
    td_u16  rsv;
    struct osal_list_head list;
} hcc_unc_struc;

#pragma pack(push, 1)
typedef struct _hcc_queue_ctrl_ {
    td_u32 service_type : 4; // 队列所属的业务类型
    td_u32 sub_type : 4; // 队列所属的业务子类型
    td_u32 transfer_mode : 3; // 队列发送模式： 单条发送、聚合发送   接收mode也用该字节   // hcc_transfer_mode
    td_u32 fc_enable : 1;       // 队列流控使能，初始化默认打开
    td_u32 flow_type : 4;   // 队列流控类型,   hcc_flowctrl_type
    td_u32 low_waterline : 8;  // 队列的低水线
    td_u32 high_waterline : 8;  // 队列的高水线

    td_u32 group_id;          // 队列所属组，用于区分msg作用的组
    td_u8 burst_limit; // 队列每次处理数据包的上限，达到上限让出机会给其他队列处理；
    td_u8 credit_bottom_value; //  flow_type 为 HCC_FLOWCTRL_CREDIT 时，该字段生效，低于该值将不再发送；
} hcc_queue_ctrl;
#pragma pack(pop)

typedef struct _hcc_assem_info_ {
    union {
        td_u32 tx_descr_num;
        td_u32 rx_next_descr_num;
    } assemble_num;

    td_u8 assemble_max_num;
    /* next assem pkts list */
    hcc_data_queue assembled_head;
} hcc_assem_info;

typedef struct _hcc_trans_queue_ {
    hcc_data_queue queue_info;  // 队列的操作链表
    hcc_queue_ctrl *queue_ctrl; // 队列流控配置 //
    td_u8                queue_id; // 当前队列的ID
    td_u8                rsv;
    union {
        struct {
            td_u16 flow_ctrl_open : 1; // 队列流控标记，flow type为 HCC_FLOWCTRL_DATA 时使用   hcc_flowctrl_flag
            td_u16 is_stopped : 1;  // 队列流控状态，停止业务入列
            td_u16 fc_on_cnt : 7;
            td_u16 fc_off_cnt : 7;
        } fc_back_para;

        td_u32 credit;  /* flow type为 HCC_FLOWCTRL_CREDIT 时使用, 保存credit值 */
    } fc_para;

    hcc_data_queue send_head; /* 实际需要发送的链表 */
} hcc_trans_queue;

#endif /* HCC_QUEUE_HEADER */