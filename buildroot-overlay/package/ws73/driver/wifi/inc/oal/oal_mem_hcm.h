/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: netbuf memory manage interface
 * Create: 2022-04-20
 */
#ifndef __OAL_MEM_HCM_H__
#define __OAL_MEM_HCM_H__
#include "oal_spinlock.h"
#include "oal_list.h"
#include "osal_types.h"

#include "soc_diag_wdk.h"
#include "wlan_types_common.h"
#include "oal_netbuf_ext.h"

#include "common_dft_rom.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_MEM_HCM_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/* 对于enhanced类型的申请接口与释放接口, 每一个内存块都包含一个4字节的头部 用来指向内存块管理结构体oal_mem_struc */
/* 整个内存块的结构如下所示 */
/* +-------------------+------------------------------------------+---------+--------------------------------+ */
/* | oal_mem_stru addr |                    payload               | dog tag |      4/8 byte       | | 4 byte | */
/* +-------------------+------------------------------------------+---------+--------------------------------| */

#define OAL_HMAC_DOG_TAG                            0x5b3a293e    /* 狗牌，用于检测内存越界 */
#define OAL_SDIO_FLOWCTRL_MAX                  114   /* ini可配sdio流控的最大值 */

#if defined(_PRE_WLAN_DFR_STAT) || defined(_PRE_WLAN_DFT_STAT)
#define WLAN_MEM_MAX_SUBPOOL_NUM               8     /* 内存池中出netbuf内存池外最大子内存池个数 */
#endif
#define HAL_NORMAL_RX_MAX_RX_OPT_BUFFS         128   /* 普通优先级描述符优化规格 */

#define oal_mem_alloc(_en_pool_id, _us_len, _uc_lock) \
    oal_mem_alloc_etc(THIS_FILE_ID, __LINE__, _en_pool_id, _us_len, _uc_lock)
#define oal_mem_free(_p_data, _uc_lock) \
    oal_mem_free_etc(THIS_FILE_ID, __LINE__, _p_data, _uc_lock)

/*****************************************************************************
  枚举名  : oal_mem_pool_id_enum_uint8
  协议表格:
  枚举说明: HOST侧内存池ID
*****************************************************************************/
typedef enum {
    OAL_MEM_POOL_ID_EVENT = 0,              /* 事件内存池 */
    OAL_MEM_POOL_ID_SHARED_DATA_PKT,        /* 共享数据帧内存池 */
    OAL_MEM_POOL_ID_SHARED_MGMT_PKT,        /* 共享管理帧内存池 */
    OAL_MEM_POOL_ID_LOCAL,                  /* 本地变量内存池  */
    OAL_MEM_POOL_ID_MIB,                    /* MIB内存池 */
    OAL_MEM_POOL_ID_SHARED_DSCR,            /* 共享描述符内存池 */

    OAL_MEM_POOL_ID_NETBUF,                 /* netbuf内存池 */
    OAL_MEM_POOL_ID_RX_DSCR,                /* 共享接收描述符内存池 */
    OAL_MEM_POOL_ID_TX_DSCR,                /* 共享发送描述符1内存池 */
    OAL_MEM_POOL_ID_BUTT
} oal_mem_pool_id_enum;
typedef osal_u8 oal_mem_pool_id_enum_uint8;

#ifdef _PRE_WLAN_DFR_STAT
/*****************************************************************************
  结构名  : oal_mem_stru
  结构说明: 内存块结构体
*****************************************************************************/
struct oal_mem_stru_tag {
    osal_u8 *data;                                  /* 存放数据的指针 */
    osal_u8 *origin_data;                           /* 记录数据的原始指针 */
    osal_u16 len;                                   /* 内存块的长度 */
    osal_u8 user_cnt : 4;                           /* 申请本内存块的用户计数 */
    oal_mem_state_enum_uint8 mem_state_flag : 4;    /* 内存块状态 */
    oal_mem_pool_id_enum_uint8 pool_id : 4;         /* 本内存属于哪一个内存池 */
    osal_u8 subpool_id : 4;                         /* 本内存是属于哪一级子内存池 */

    osal_ulong return_addr;                          /* alloc内存的函数地址 新增 */
};
typedef struct oal_mem_stru_tag oal_mem_stru;

/*****************************************************************************
  结构名  : oal_mem_subpool_stru
  结构说明: 子内存池结构体
*****************************************************************************/
typedef struct {
    osal_spinlock       spinlock;
    osal_u16            len;             /* 本子内存池的内存块长度 */
    osal_u16            free_cnt;        /* 本子内存池可用内存块数 */

    /* 记录oal_mem_stru可用内存索引表的栈顶元素，其内容为oal_mem_stru指针 */
    void                **ppst_free_stack;

    osal_u16            total_cnt;       /* 本子内存池内存块总数 */
    osal_u8             auc_resv[2];
} oal_mem_subpool_stru;

/*****************************************************************************
  结构名  : oal_mem_pool_stru
  结构说明: 内存池结构体
*****************************************************************************/
typedef struct {
    osal_u16              max_byte_len;        /* 本内存池可分配内存块最大长度 */
    osal_u8               subpool_cnt;         /* 本内存池一共有多少子内存池 */
    osal_u8               resv;
    /* 子内存池索引表数组 */
    oal_mem_subpool_stru    subpool_table[WLAN_MEM_MAX_SUBPOOL_NUM];

    osal_u16              mem_used_cnt;        /* 本内存池已用内存块 */
    osal_u16              mem_total_cnt;       /* 本内存池一共有多少内存块 */
    oal_mem_stru           *mem_start_addr;
} oal_mem_pool_stru;
#endif

/*****************************************************************************
  10 函数声明
*****************************************************************************/
osal_void *oal_mem_alloc_etc(osal_u32 file_id, osal_u32 line_num,
    oal_mem_pool_id_enum_uint8 pool_id, osal_u16 len, osal_u8 lock);
osal_u32 oal_mem_free_etc(osal_u32 file_id, osal_u32 line_num, osal_void *data, osal_u8 lock);

oal_netbuf_stru* oal_netbuf_alloc_ext(oal_netbuf_id_enum_uint8 netbuf_id,
    td_u32 size, oal_netbuf_priority_enum_uint8 pri);

oal_netbuf_stru *oal_mem_multi_netbuf_alloc(osal_u16 len);

osal_u8   *oal_netbuf_header_buf(const oal_netbuf_stru *netbuf);
const osal_u8 *oal_netbuf_header_buf_const(const oal_netbuf_stru *netbuf);
const osal_u8 *oal_netbuf_cb_buf_const(const oal_netbuf_stru *netbuf);
osal_u8   *oal_netbuf_cb_buf(oal_netbuf_stru *netbuf);

/*****************************************************************************
 功能描述  : 获取skb数据头部(偏移mac头后的数据)
*****************************************************************************/
static inline osal_u8 *oal_netbuf_wifi_data(const oal_netbuf_stru *netbuf)
{
    osal_u8 *payload = oal_netbuf_payload(netbuf);
    return (payload + OAL_HDR_TOTAL_LEN);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_mem_hcm.h */
