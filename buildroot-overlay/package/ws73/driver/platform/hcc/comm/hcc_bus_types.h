/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc bus types.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_BUS_TYPES_HEADER
#define HCC_BUS_TYPES_HEADER

#include "td_base.h"
#include "hcc_types.h"
#include "soc_osal.h"
#include "osal_list.h"
#include "hcc_queue.h"
#include "hcc_cfg.h"
#include "hcc_cfg_comm.h"
#include "securec.h"

#define HCC_BUS_STATE_TX    (1 << 0)
#define HCC_BUS_STATE_RX    (1 << 1)
#define HCC_BUS_STATE_ALL   (HCC_BUS_STATE_TX | HCC_BUS_STATE_RX)

/* Power Action */
typedef enum _hcc_bus_power_action_type_ {
    HCC_BUS_POWER_DOWN,               /* The action when wlan power down */
    HCC_BUS_POWER_UP,                 /* The action when wlan power up */
    HCC_BUS_POWER_PATCH_LOAD_PREPARE, /* The action before patch downlaod */
    HCC_BUS_POWER_PATCH_LAUCH,        /* The action after patch download before channel enabled */
    HCC_BUS_POWER_BUTT
} hcc_bus_power_action_type;

/*
 * bus层传输约束
 */
#define HCC_ADAPT_BUS_ADDR_ALIGN_MIN 1       /* 数据首地址对齐要求长度 */
#define HCC_ADAPT_BUS_LEN_ALIGN_MIN 1        /* 数据长度对齐要求长度 */
#define HCC_ADAPT_BUS_ASSEMBLE_CNT_MIN 1     /* 支持的最小聚合长度，为1则和不聚合一致 */
#define HCC_ADAPT_BUS_DESCR_ALIGN_BIT_MIN 0  /* 描述符需要左移几bit位，与描述符对齐长度相对应 */
typedef td_s32 (*bus_pm_notify)(td_void);
typedef struct _hcc_bus_ hcc_bus;
typedef struct _bus_dev_ops {
    td_u32 (*tx_proc)(hcc_bus *bus, hcc_trans_queue *queue, td_u16 *remain_pkt_nums);
    ext_errno (*send_and_clear_msg)(hcc_bus *bus, td_u32 msg);
    td_bool (*is_busy)(hcc_queue_dir dir);
    td_void (*update_credit)(hcc_bus *bus, td_u32 free_cnt);  /* credit流控方式，在SDIO上只有 device->host */
    td_s32 (*get_credit)(hcc_bus *bus, td_u32 *free_cnt);
#ifdef CONFIG_HCC_SUPPORT_REG_OPT
    td_s32 (*read_reg)(hcc_bus *bus, td_u32 addr, td_u32 *value);
    td_s32 (*write_reg)(hcc_bus *bus, td_u32 addr, td_u32 value);
#endif
#ifdef CONFIG_HCC_SUPPORT_PATCH_OPT
    td_s32 (*patch_read)(hcc_bus *pst_bus, td_u8 *buff, td_u32 len, td_u32 timeout);
    td_s32 (*patch_write)(hcc_bus *pst_bus, td_u8 *buff, td_u32 len);
#endif
    td_s32 (*reinit)(hcc_bus *pst_bus);                                         /* ip reinit */
    td_void (*stop_xfer)(td_void);
    td_s32 (*resume_xfer)(td_void);

    td_s32 (*power_action)(hcc_bus *pst_bus, hcc_bus_power_action_type action); /* do something before power on/off */
    td_s32 (*sleep_request)(hcc_bus *pst_bus);
    td_s32 (*wakeup_request)(hcc_bus *pst_bus);
    td_s32 (*pm_notify_register)(hcc_bus *pst_bus, bus_pm_notify suspend_func, bus_pm_notify resume_func);
} bus_dev_ops;

struct bus_msg_stru {
    hcc_msg_rx msg_rx;
    td_void *data;
    osal_atomic count;
    hcc_service_type service_type;
    td_u64 cpu_time; /* the time of the last come! */
};

typedef td_void (*hcc_bus_dfx_f)(td_void);
struct _hcc_bus_ {
    td_u32 bus_type : 4; /* Current bus type: HCC_BUS_TYPE */
    td_u32 max_trans_size : 12; /* Max size for each package */
    td_u32 addr_align : 8; /* Buffer address of each package must aligned to this */
    td_u32 len_align : 8;  /* Buffer length of each package must aligned to this */
    td_u32 max_assemble_cnt : 8;  /* Max assemble count if bus support assemble. Fill 1 if not support. */
    td_u32 descr_align_bit : 4;  /* Description aligned bits. */
    td_void *hcc;  /* HCC handler, struct type: hcc_handler */
    td_void *data; /* device driver structure reference */
    hcc_unc_struc *rsv_buf; /* 预留缓存，由bus层根据驱动需要申请。
                             * 使用场景（sdio为例）：驱动获取不到内存可使用该缓存，但不会传给hcc，
                             * 预留内存中的数据会被丢弃，以防止因获取不到内存导致驱动异常。
                             */
    hcc_bus_dfx_f hcc_bus_dfx;
    bus_dev_ops *bus_ops;

    struct bus_msg_stru msg[HCC_RX_MAX_MESSAGE];
    td_u32 last_msg;
    td_u32 state : 4;
    td_u32 tx_sched_count : 8; /* 发送聚合度 */
    td_u32 force_update_queue_id : 8; /* 是否需要强制发送hcc队列信息 */
    td_u32 cap_max_trans_size;
};

#endif /* HCC_BUS_TYPES_HEADER */
