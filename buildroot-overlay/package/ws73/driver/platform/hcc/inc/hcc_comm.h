/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: hcc commom types.
 * Author: CompanyName
 * Create: 2021-05-13
 */

#ifndef HCC_COMMON_TYPE_H
#define HCC_COMMON_TYPE_H

#include "td_base.h"
#include "hcc_types.h"
#include "soc_errno.h"

#include "soc_osal.h"
#include "osal_list.h"

#include "hcc_bus_types.h"
#include "hcc_queue.h"
#include "hcc_cfg.h"
#include "hcc_cfg_comm.h"

#if defined(CONFIG_HCC_SUPPORT_NON_OS)
#include "serial_dw.h"
#endif

typedef struct _hcc_handler_ hcc_handler;

typedef struct _hcc_serv_info_ {
    td_u32 service_type : 4;
    td_u32 bus_type : 4;
    td_u32 rsv : 24;
    osal_atomic service_enable;  /* enable after hcc_service_init */
    osal_atomic service_total_qlen;
    hcc_adapt_ops *adapt;
} hcc_serv_info;

typedef struct _hcc_service_list_ {
    struct osal_list_head  service_list;
    hcc_serv_info *service_info;
    osal_spinlock service_lock;
} hcc_service_list;

typedef struct _hcc_handler_list_ {
    struct osal_list_head  handler_list;
    hcc_handler *handler;
} hcc_handler_list;

#define HCC_HDR_LEN hcc_get_head_len()
#define HCC_TASK_NAME_DEFAULT "hcc_task"

/* HCC 打印接口 */
#ifdef CONFIG_HCC_DEBUG_PRINT

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define hcc_printf(fmt, arg...) printk("[HCC] "fmt, ##arg)
#define hcc_printf_null(fmt, arg...) printk("[HCC] [INFO] "fmt, ##arg)
#elif defined(CONFIG_HCC_SUPPORT_NON_OS)
#define hcc_printf(fmt, arg...)         debug_message("[HCC] "fmt, ##arg)
#define hcc_printf_null(fmt, arg...)    debug_message("[HCC] [INFO] "fmt, ##arg)
#else
#define hcc_printf(fmt, arg...)         printf("[HCC] "fmt, ##arg)
#define hcc_printf_null(fmt, arg...)    printf("[HCC] [INFO] "fmt, ##arg)
#endif

#else
#define hcc_printf(fmt, arg...)
#define hcc_printf_null(fmt, arg...)
#endif /* CONFIG_HCC_DEBUG_PRINT */

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define hcc_print_data(fmt, arg...)  printk(fmt, ##arg)
#ifdef CONTROLLER_CUSTOMIZATION
#define hcc_debug(fmt, arg...) printk(KERN_EMERG KBUILD_MODNAME "[HCC] " fmt, ##arg)
#else
#define hcc_debug(fmt, arg...) printk("[HCC] " fmt, ##arg)
#endif
#if defined(CONFIG_HCC_ERROR_PRINT)
#ifdef CONTROLLER_CUSTOMIZATION
#define hcc_printf_err_log(fmt, arg...) printk(KERN_EMERG KBUILD_MODNAME "[HCC] [ERROR]"fmt, ##arg)
#define hcc_println_err_log(fmt, arg...) printk(KERN_EMERG KBUILD_MODNAME "[HCC] [ERROR] - " fmt "\r\n", ##arg)
#else
#define hcc_printf_err_log(fmt, arg...) printk("[HCC] [ERROR]"fmt, ##arg)
#define hcc_println_err_log(fmt, arg...) printk("[HCC] [ERROR] - " fmt "\r\n", ##arg)
#endif
#else
#define hcc_printf_err_log(fmt, arg...)
#define hcc_println_err_log(fmt, arg...)
#endif
#define hcc_dfx_print(fmt, arg...)         printk("[HCC] [DFX] "fmt, ##arg)
#elif defined(CONFIG_HCC_SUPPORT_NON_OS)
#define hcc_print_data(fmt, arg...)         debug_message(fmt, ##arg)
#define hcc_debug(fmt, arg...) debug_message("[HCC] "fmt, ##arg)
#define hcc_printf_err_log(fmt, arg...) debug_message("[HCC] [ERROR] %s - %d "fmt, __FUNCTION__, __LINE__, ##arg)
#define hcc_println_err_log(fmt, arg...) debug_message("[HCC] [ERROR]-"fmt"[%s-%d]\r\n", ##arg, __FUNCTION__, __LINE__)
#define hcc_dfx_print(fmt, arg...) debug_message("[HCC] "fmt, ##arg)
#else
#define hcc_print_data(fmt, arg...)         printf(fmt, ##arg)
#define hcc_debug(fmt, arg...)         printf("[HCC] "fmt, ##arg)
#define hcc_printf_err_log(fmt, arg...) printf("[HCC] [ERROR] %s - %d "fmt, __FUNCTION__, __LINE__, ##arg)
#define hcc_println_err_log(fmt, arg...) printf("[HCC] [ERROR] - " fmt " [%s]-[%d]\r\n", ##arg, __FUNCTION__, __LINE__)
#define hcc_dfx_print(fmt, arg...)         printf("[HCC] "fmt, ##arg)
#endif


#ifdef CONFIG_HCC_DEBUG_PRINT
#undef CONFIG_HCC_DEBUG_PRINT  /* 防止打开debug宏后打开memcheck */
#ifdef CONFIG_HCC_DEBUG_PRINT
#define HCC_MEM_CHECK_ADDRESS 0x20024c80 /* 该地址修改为定位问题时实际被踩地址 */
extern td_u32 LOS_MemIntegrityCheck(TD_CONST td_void *pool);
#define HCC_DEBUG_MEM_CHECK do { \
    hcc_printf("check  %s-%d\r\n", __FUNCTION__, __LINE__); \
    LOS_MemIntegrityCheck((td_void *)HCC_MEM_CHECK_ADDRESS); \
    hcc_printf("check  %s-%d\r\n", __FUNCTION__, __LINE__); } while (0)
#else
#define HCC_DEBUG_MEM_CHECK
#endif
#define CONFIG_HCC_DEBUG_PRINT
#endif

#pragma pack(push, 1)
/* 5bytes */
typedef struct _hcc_header_ {
    td_u8 sub_type : 4;  /* sub type to hcc type,refer to hcc_action_type */
    td_u8 service_type : 4; /* main type to hcc type,refer to hcc_action_type */
    td_u8 queue_id;  /* 队列ID, TX-RX队列为对应关系 */
    td_u16 pay_len; /* payload的长度 */
} hcc_header;
#pragma pack(pop)

typedef enum _hcc_module_init_errno_ {
    HCC_MODULE_INIT_FAILED_INIT_STATE,
    HCC_MODULE_INIT_FAILED_SET_STATE,
    HCC_MODULE_INIT_FAILED_INIT_UNC_POOL,
    HCC_MODULE_INIT_FAILED_INIT_QUEUE,
    HCC_MODULE_INIT_FAILED_INIT_FLOW_CTRL,
    HCC_MODULE_INIT_FAILED_INIT_SERVICE,
    HCC_MODULE_INIT_FAILED_INIT_TASK,
    HCC_MODULE_INIT_FAILED_ADD_HANDLER,
} hcc_module_init_errno;

/*
 * 各个bus使用差异说明：
 * SDIO: h->D:  无论是否聚合都带描述符， && 发送之前需要发送控制帧标识队列
 *       D->h:  不论是否聚合都不带描述符，使用64个寄存器存储，第0个标识队列ID， && 发送之前不需要发送控制帧
 */
typedef enum _hcc_transfer_mode_ {
    HCC_SINGLE_MODE,        /* 单帧发送 */
    HCC_ASSEMBLE_MODE,      /* 聚合发送 */
    HCC_TRANS_MODE_BUTT,
    HCC_TRANS_MODE_INVALID = TD_U8_MAX
} hcc_transfer_mode;

typedef enum _hcc_flowctrl_type_ {
    HCC_FLOWCTRL_DATA,     // 被动接收  接收端的状态，（msg方式）
    HCC_FLOWCTRL_CREDIT,   // 发送前主动获取接收端的剩余包数量
    HCC_FLOWCTRL_BUTT,
    HCC_FLOWCTRL_TYPE_INVALID = 0xF
} hcc_flowctrl_type;

typedef enum _hcc_flowctrl_flag_ {
    HCC_FLOWCTRL_FLAG_OFF = 0,
    HCC_FLOWCTRL_FLAG_ON = 1
} hcc_flowctrl_flag;

#define HCC_FLOWCTRL_DEFAULT_LO_WATER_LINE 12
#define HCC_FLOWCTRL_DEFAULT_HI_WATER_LINE 40
#define HCC_FLOWCTRL_DEFAULT_CREDIT_BOTTOM_VALUE 5
#define HCC_FLOWCTRL_DEFAULT_CREDIT_TOP_VALUE 20
#define HCC_DEFAULT_QUEUE_TRANSFER_BURST_LIMIT 12
#define HCC_MAX_QUEUE_TRANSFER_BURST_LIMIT 255
#define HCC_TRANSMODE_SINGLE    0
#define HCC_TRANSMODE_ASSEMBLE  1
#define HCC_DISABLE TD_FALSE
#define HCC_ENABLE  TD_TRUE

typedef struct _hcc_transfer_resource_ {
    osal_wait hcc_transfer_wq;
    osal_wait hcc_fc_wq;

    td_u32 cur_trans_pkts;
    td_u8 rx_thread_enable : 1;
    td_u8 tx_thread_enable : 1;
    td_u8 task_run : 1;
    td_u8 rsv2 : 5;

    osal_task *hcc_transfer_thread_handler; // 主循环task
    hcc_trans_queue *hcc_queues[HCC_DIR_COUNT];
} hcc_trans_resource;

typedef hcc_unc_struc *(*alloc_unc_buf)(td_s32 len, td_u8 type);
typedef td_void (*free_unc_buf)(hcc_unc_struc *data);

typedef struct {
    td_u8 dir; /* hcc_queue_dir */
    td_u8 queue_id; /* hcc_queue_type */
    hcc_queue_ctrl queue_ctrl;
} hcc_queue_cfg; // 配置队列信息

struct _hcc_handler_ {
    hcc_channel_name channel_name;
    osal_atomic hcc_state; // HCC状态，使能、禁能、异常使用
    hcc_data_queue *unc_pool_head;
    hcc_trans_resource hcc_resource;
    hcc_service_list hcc_serv; /* hcc注册的服务 */
    hcc_service_list hcc_serv_del; /* hcc删除的服务 */
    hcc_bus *bus; /* 与当前逻辑通道对应的物理通道的结构 */
    hcc_queue_cfg *que_cfg;
    td_u8 que_max_cnt;
    td_u8 srv_max_cnt;
    td_u16 rsv;
};

typedef hcc_bus *(* hcc_bus_load_func)(td_void);
typedef td_void (* hcc_bus_unload_func)(td_void);
typedef struct _bus_load_ {
    hcc_bus_load_func load;
    hcc_bus_unload_func unload;
} bus_load_unload;
#endif /* HCC_COMMON_TYPE_H */
