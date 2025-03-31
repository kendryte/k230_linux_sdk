/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hcc product configuration.
 * Author: CompanyName
 * Create: 2021-09-18
 */

#ifndef __HCC_CFG_COMM_H__
#define __HCC_CFG_COMM_H__

#include "td_base.h"
#include "osal_types.h"

typedef enum _hcc_bus_type_ {
    HCC_BUS_MIN = 0x0,
    HCC_BUS_USB = 0x0,
    HCC_BUS_SDIO = 0x1,
    HCC_BUS_IPC = 0x3,
    HCC_BUS_PCIE = 0x4,
    HCC_BUS_VIRTUAL = 0x5,
    HCC_BUS_UART = 0x6,
    HCC_BUS_BUTT,
    HCC_BUS_MAX  = 0xFF
} hcc_bus_type;

typedef enum _hcc_serv_main_type {
    HCC_ACTION_TYPE_BT = 0,        /* data from bt */
    HCC_ACTION_TYPE_OAM = 1,       /* data from oam,plat etc. */
    HCC_ACTION_TYPE_TEST = 2,      /* used for hcc transfer test,msg,dfx etc */
    HCC_ACTION_TYPE_SLP = 3,       /* used for device chr exception, discarded by hso now, now used for GLP */
    HCC_ACTION_TYPE_CUSTOMIZE = 4, /* used for wifi control message */
    HCC_ACTION_TYPE_BSLE_MSG = 5,  /* used for bsle msg */
    HCC_ACTION_TYPE_WIFI = 6,      /* data from wifi data transfer */
    HCC_SERVICE_VIRTUAL_HMAC = 7,      /* test for resv1 */
    HCC_SERVICE_VIRTUAL_DMAC = 8,      /* test for resv2 */
    HCC_SERVICE_FOR_HCC_INTERNAL = 9,  /* test for resv3 */
    HCC_ACTION_TYPE_SLE = 0xA,         /* data from sle */
    HCC_SERVICE_TYPE_MAX,
} hcc_service_type;

typedef enum _hcc_queue_type {
    CTRL_QUEUE = 0,                /* used for wifi control message */
    DATA_HI_QUEUE = 1,             /* used for wifi mgmt frame */
    DATA_LO_QUEUE = 2,             /* used for wifi data frame */
    DATA_UDP_BK_QUEUE = 3,         /* used for GLP */
    DATA_TCP_DATA_QUEUE = 4,       /* used for tcp data */
    DATA_TCP_ACK_QUEUE = 5,        /* used for tcp ack */
    DATA_UDP_DATA_QUEUE = 6,       /* used for udp data */
    BT_DATA_QUEUE = 7,             /* used for bt data */
    SLE_DATA_QUEUE = 8,            /* used for gle data */
    HCC_QUEUE_INTERNAL = 9,        /* used for oam,plat etc */
    BSLE_MSG_QUEUE = 10,           /* used for bgle msg */
    TEST_SDIO_SINGLE_QUEUE = 11,   /* used for hcc single test */
    TEST_SDIO_ASSEM_QUEUE = 12,    /* used for hcc assemble test */
    HCC_QUEUE_COUNT
} hcc_queue_type;

typedef enum _d2h_msg_type {
    D2H_MSG_BSP_READY = 0,         /* device boot msg */
    D2H_MSG_WLAN_READY = 1,        /* wlan ready msg */
    D2H_MSG_FLOWCTRL_OFF = 2,      /* can send data, now discard, wifi flowctrl self */
    D2H_MSG_FLOWCTRL_ON = 3,       /* can't send data, now discard, wifi flowctrl self */
    D2H_MSG_CREDIT_UPDATE = 4,     /* update high priority buffer credit value, not used */
    D2H_MSG_HIGH_PKT_LOSS = 5,     /* bus rx alloc fail, high pri pkts loss count */
    D2H_MSG_DEVICE_PANIC = 6,      /* system panic */
    D2H_MSG_WAKEUP_SUCC = 7,       /* Wakeup done */
    D2H_MSG_ALLOW_SLEEP = 8,       /* ALLOW Sleep */
    D2H_MSG_DISALLOW_SLEEP = 9,    /* DISALLOW Sleep */
    D2H_MSG_POWEROFF_ACK = 10,     /* Poweroff cmd ack */
    D2H_MSG_OPEN_BCPU_ACK = 11,    /* OPEN BCPU cmd ack */
    D2H_MSG_CLOSE_BCPU_ACK = 12,   /* CLOSE BCPU cmd ack */
    D2H_MSG_HALT_BCPU = 13,        /* halt bcpu ack */
    D2H_MSG_WCPU_PMCTRL_ACK = 14,
    D2H_MSG_HOST_SLP_ACK = 15,
    D2H_MSG_WCPU_WORK = 16,
    /* device to host pcie special msg, start from 25 */
    D2H_MSG_PCIE_HOST_READY_ACK = 17,
    D2H_MSG_BCPU_PANIC = 18,
    D2H_MSG_TEST = 19,
    D2H_MSG_TEST_START = 20,        /* hcc test 开始测试，时间在Host侧记录 */
    D2H_MSG_TEST_STOP = 21,         /* hcc test 结束测试，时间在Host侧记录 */
    D2H_MSG_REPORT_RSP_FAIL = 22,   /* frw sync msg rsp fail */
    D2H_MSG_WOW_SYNC_REPLY = 23,    /* wow sync reply */
    D2H_MSG_WOW_MIRROR_REPLY_SUCC = 24,  /* wow reply succ */
    D2H_MSG_WOW_MIRROR_REPLY_FAIL = 25,  /* wow reply fail */
    D2H_MSG_WOW_WAKE_HOST = 26,          /* wow exit wake host */
    D2H_MSG_HEART_BEAT_ACK = 27,         /* system heart beat */
    D2H_MSG_SCHED = 31,                  /* 用于flush发送失败的消息 */
    D2H_MSG_COUNT = 32, /* max support msg count */
} d2h_msg_type;

/* Host to device sdio message type */
typedef enum _h2d_msg_type {
    H2D_MSG_WLAN_OPEN = 0,  /* wlan open msg */
    H2D_MSG_WLAN_CLOSE = 1, /* wlan close msg */
    H2D_MSG_PANIC_DEVICE = 2,
    H2D_MSG_RESET_BCPU = 3,
    H2D_MSG_PM_BCPU_OFF = 4,
    H2D_MSG_HALT_BCPU = 5,
    H2D_MSG_WCPU_PM_ENABLE = 6,
    H2D_MSG_WCPU_PM_DISABLE = 7,
    H2D_MSG_WCPU_WKUP_BCPU = 8,
    H2D_MSG_HEARTBEAT_ENABLE = 9,
    H2D_MSG_HEARTBEAT_DISABLE = 10,
    H2D_MSG_PM_WCPU_ON = 11,
    H2D_MSG_PM_WCPU_OFF = 12,
    H2D_MSG_HOST_SLP = 13,
    H2D_MSG_HOST_WORK = 14,
    H2D_MSG_SLEEP_REQ = 15,
    H2D_MSG_BT_TRANS_START = 16, /* start received bt data from host */
    H2D_MSG_BT_TRANS_END = 17,   /* received bt data from host end */
    H2D_MSG_PM_DEBUG = 18,
    H2D_MSG_STOP_TEST = 19,
    /* host to device pcie special msg, start from 25 */
    H2D_MSG_PCIE_HOST_READY = 20,
    H2D_MSG_FLOWCTRL_ON = 21, /* can send data, force to open */
    H2D_MSG_FLOWCTRL_OFF = 22,
    H2D_MSG_TEST = 23,      /* msg for hcc test */
    H2D_MSG_TEST_LOOP = 24, /* 双向测试使用msg机制触发 */
    H2D_MSG_BT_OPEN = 25,
    H2D_MSG_BT_CLOSE = 26,
    H2D_MSG_WOW_SYNC = 27,
    H2D_MSG_HEART_BEAT = 28,
    H2D_MSG_SLE_OPEN = 29,
    H2D_MSG_SLE_CLOSE = 30,
    H2D_MSG_COUNT = 32, /* max support msg count */
} h2d_msg_type;

typedef struct {
    osal_u16 msg_id;
    osal_u16 len;
} hcc_plat_cfg_msg_header;

typedef enum _oam_sub_type {
    OAM_HCC_DEAULT_VALUE = 0,
    DUMP_REG = 1,
    DUMP_MEM = 2,
    CHANGE_LOG_LV = 3,
    WIFI_FREQ = 4,
    DEV_WIFI_BUG_ON = 5,
    DEV_BT_BUG_ON = 6,
    BCPU_CRASH_DUMP_INFO = 7,
    BCPU_CRASH_DUMP_STACK = 8,
    PLAT_MSG_TRANSFER = 9,
} oam_sub_type;
#endif /* __HCC_CFG_COMM_H__ */
