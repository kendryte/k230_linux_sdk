/*
 * Copyright (c) CompanyNameMagicTag. 2021-2023. All rights reserved.
 * Description: Header file of device and host frw_msg public interface.
 * Author: AuthorNameMagicTag
 * Create: 2021-09-16
 */

#ifndef __FRW_MSG_ROM_H__
#define __FRW_MSG_ROM_H__

#include "osal_types.h"

/*****************************************************************************
  1 宏定义
*****************************************************************************/
#define FRW_H2D_SYNC_MSG_ID_NOT_SUPPORT 199

/*****************************************************************************
  2 枚举定义
*****************************************************************************/
/* 此枚举用于定义从FRW模块以上各模块的初始化状态 */
typedef enum {
    FRW_INIT_STATE_START,                /* 初始化刚启动，即FRW初始化开始 */
    FRW_INIT_STATE_FRW_SUCC,             /* FRW模块初始化成功 */
    FRW_INIT_STATE_HAL_SUCC,             /* HAL模块初始化成功 */
    FRW_INIT_STATE_DMAC_CONFIG_VAP_SUCC, /* DMAC模块启动初始化成功，配置VAP已正常                */
    FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC, /* HMAC模块启动初始化成功，配置VAP已正常                */
    FRW_INIT_STATE_ALL_SUCC,             /* 此状态表示HMAC以上模块均已初始化成功 */

    FRW_INIT_STATE_BUTT
} frw_init_enum;
typedef osal_u16 frw_init_enum_uint16;

typedef enum {
    HCC_MSG_TYPE_CFG,         // 配置消息
    HCC_MSG_TYPE_RSP,         // 消息响应
    HCC_MSG_TYPE_MGMT_FRAME,  // 管理帧
    HCC_MSG_TYPE_DATA_FRAME,  // 数据帧
    HCC_MSG_TYPE_NUM
} hcc_msg_type;

typedef enum {
    FRW_POST_PRI_HIGH = 0,
    FRW_POST_PRI_LOW = 1,
    FRW_POST_PRI_NUM
} frw_post_pri_enum;
typedef osal_u8 frw_post_pri_enum_uint8;

typedef enum {
    FRW_NETBUF_2H_BEGIN = 0,
    FRW_NETBUF_D2H_BEGIN = FRW_NETBUF_2H_BEGIN,
    FRW_NETBUF_D2H_DMAC_MGNT_FRAME = FRW_NETBUF_D2H_BEGIN, // hmac_rx_mgmt_event_adapt
    FRW_NETBUF_D2H_DMAC_DATA_FRAME, // hmac_rx_data_event_adapt
    FRW_NETBUF_D2H_TX_FTM_ACK,
    FRW_NETBUF_D2H_NUM
} frw_netbuf_d2h_enum;

typedef enum {
    FRW_NETBUF_W2H_BEGIN = FRW_NETBUF_D2H_NUM,
    FRW_NETBUF_W2H_MGNT_FRAME = FRW_NETBUF_W2H_BEGIN,
    FRW_NETBUF_W2H_DATA_FRAME,
    FRW_NETBUF_W2H_NUM,
    FRW_NETBUF_2H_NUM = FRW_NETBUF_W2H_NUM,
} frw_netbuf_w2h_enum;

/*****************************************************************************
  3 结构体定义
*****************************************************************************/
typedef struct {
    osal_u8 vap_flag;
    osal_u8 drop;
    osal_u8 vap_id;
    osal_u16 msg_id;
} frw_flush_msg;

typedef struct {
    osal_u8 *data;  /* 配置消息的输入数据buffer指针，内存由调用者分配, 由frw释放 */
    osal_u8 *rsp;     /* 配置消息的同步调用，返回数据的buffer指针，返回数据buffer由接口分配，但需要调用者释放；
                         同步调用时，接口通过此参数向调用者回传rsp地址，异步调用时，接口会将此参数置为NULL。
                      */
    osal_u16 data_len; /* 配置消息的输入数据buffer长度 */
    osal_u16 rsp_buf_len; /* hmac下发到dmac时填写；同步调用，返回的数据长度，异步调用时，此参数会被接口置为0 */
    osal_u16 rsp_len;   /* 实际rsp长度，由dmac返回 */
    osal_u16 sync : 1;   /* 同步标志 */
    osal_u16 type : 7;   /* 消息类型 */
    osal_u16 rsv  : 8;   /* 保留字段 */
} frw_msg;

typedef struct {
    osal_u16    msg_id;
    osal_u8     vap_id;
    osal_u8     sync : 1;
    osal_u8     type : 3;
    osal_u8     rsv : 4;
    osal_s32    cb_ret;
    osal_u32    msg_seq;
    frw_msg     cfg;
} frw_msg_adapt;

/* record the data type by the hcc */
#pragma pack(push, 1)
/* 4B */
typedef struct frw_hcc_extend_hdr {
    union {
        osal_u16 msg_id;
        struct {
            osal_u8 nest_type;
            osal_u8 nest_sub_type;
        } nest;
    } msg;

    osal_u8 vap_id : 4;
    osal_u8 msg_type : 4;
    osal_u16 data_len;
    osal_u8 pay_type : 4;
    osal_u8 sub_type : 4;
    osal_u16 rsv;
} frw_hcc_extend_hdr_stru;
#pragma pack(pop)

static inline osal_u16 frw_get_head_len(osal_void)
{
    return sizeof(frw_hcc_extend_hdr_stru);
}

#endif // __FRW_MSG_H__
