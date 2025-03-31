/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: host sdio driver os adapt.
 * Author: Huanghe
 * Create: 2020-12-20
 */


#ifndef __HCC_SDIO_HOST_H__
#define __HCC_SDIO_HOST_H__

#include "td_base.h"
#include "td_type.h"
#include "hcc_bus.h"
#include "hcc_queue.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HCC_SDIO_BLOCK_SIZE                     512         /* one size of data transfer block size,
                                                           * 64, 128, 256, 512, 1024 */
/* The max scatter buffers when host to device */
#define HCC_SDIO_HOST2DEV_SCATT_MAX       40
#define HCC_SDIO_HOST2DEV_SCATT_SIZE      64

/* The max scatter buffers when device to host */
#define HCC_SDIO_DEV2HOST_SCATT_MAX       63

/* The max scatt num of rx and tx */
#define HCC_SDIO_SCATT_MAX_NUM    (HCC_SDIO_DEV2HOST_SCATT_MAX)

/* 64B used to store the scatt info,1B means 1 pkt. */
#define HCC_SDIO_H2D_SCATT_BUFFLEN_ALIGN_BITS     3
/* 1 << 5 */
/* Host to device's descr align length depends on the
   CONFIG_HCC_SDIO_H2D_SCATT_LIST_ASSEMBLE */
#ifdef CONFIG_HCC_SDIO_H2D_SCATT_LIST_ASSEMBLE
#define HCC_SDIO_H2D_SCATT_BUFFLEN_ALIGN    8
#else
#define HCC_SDIO_H2D_SCATT_BUFFLEN_ALIGN    32
#endif
#define HCC_SDIO_H2D_SCATT_BUFFADDR_ALIGN   4

/* Device To Host,descr just request 4 bytes aligned,
  but 10 bits round [0~1023], so we also aligned to 32 bytes */
#define HCC_SDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS     5
/* 1 << 5 */
#ifdef CONFIG_HCC_SDIO_D2H_SCATT_LIST_ASSEMBLE
#define HCC_SDIO_D2H_SCATT_BUFFLEN_ALIGN    32
#else
#define HCC_SDIO_D2H_SCATT_BUFFLEN_ALIGN    512
#endif

#define HCC_SDIO_EXTEND_REG_COUNT         64

#define HSDIO_HOST2DEV_PKTS_MAX_LEN     1600
#define SDIO_MAX_XFER_LEN               (1024 * 128)

enum {
    SDIO_READ = 0,
    SDIO_WRITE,
    SDIO_OPT_BUTT
};

typedef struct _sdio_host_assem_info_ {
    td_u32 assemble_num;
    /* next assem pkts list */
    hcc_data_queue assembled_head;
} sdio_host_assem_info;

struct sdio_scatt {
    td_u32 max_scatt_num;
    struct scatterlist *sglist;
};

struct hcc_sdio_extend_func {
    td_u32 int_stat;
    td_u32 msg_stat;
    td_u32 xfer_count;
    td_u32 credit_info;
    td_u8  comm_reg[HCC_SDIO_EXTEND_REG_COUNT];
};

struct hsdio_func1_info {
    td_u32 func1_err_reg_info;
    td_u32 func1_err_int_count;
    td_u32 func1_ack_int_acount;
    td_u32 func1_msg_int_count;
    td_u32 func1_data_int_count;
    td_u32 func1_no_int_count;
};

struct hsdio_error_info {
    td_u32 rx_scatt_info_not_match;
};

typedef struct _hsdio_scatt_buff_ {
    /* record the tx scatt list assembled buffer */
    td_void *buff;
    td_u32 len;
} hsdio_scatt_buff;

struct oal_sdio {
    /* sdio work state, sleep , work or shutdown? */
    hcc_bus *pst_bus;

    td_u32 state;	            /* 总线当前状态 */
    struct sdio_func *func;
    td_u8 current_queue;
    td_u8 rsv[3];
    hcc_unc_struc descript_unc; /* 用来存储描述符的链表节点 */
    sdio_host_assem_info tx_assemble_info;
    hcc_data_queue rx_assemble_head;

    /* used to sg list sdio block align */
    td_u8 *sdio_align_buff;

    td_u64 sdio_int_count;

    td_u32 ul_sdio_suspend;
    td_u32 ul_sdio_resume;
    bus_pm_notify suspend_notify_func;
    bus_pm_notify resume_notify_func;
    struct sdio_scatt scatt_info[SDIO_OPT_BUTT];
    /* This is get from sdio , must alloc for dma,
      the extend area only can access by CMD53 */
    struct hcc_sdio_extend_func *sdio_extend;
    td_u32 func1_int_mask;
    struct hsdio_func1_info func1_stat;
    struct hsdio_error_info error_stat;

    hsdio_scatt_buff tx_scatt_buff;
    hsdio_scatt_buff rx_scatt_buff;
    td_void *rx_reserved_buff; /* use the mem when rx alloc mem failed! */
    td_u32 rx_reserved_buff_len;
};

#ifdef CONFIG_HCC_SUPPORT_SDIO
extern struct oal_sdio *g_sdio_handler;
#endif
static inline struct oal_sdio *oal_get_sdio_default_handler(osal_void)
{
#ifdef CONFIG_HCC_SUPPORT_SDIO
    return g_sdio_handler;
#else
    return TD_NULL;
#endif
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_SDIO_RESCAN
extern void hisi_sdio_rescan(int slot);
#endif
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
td_s32 sdio_rescan_device(hcc_bus *pst_bus);
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif

