/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: plat_main header file.
 * Author: Huanghe
 * Create: 2023-01-01
 */

#ifndef __PLAT_EXCEPTION_RST_H__
#define __PLAT_EXCEPTION_RST_H__

#include "soc_osal.h"
#include "oal_net.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 Define macro
*****************************************************************************/
#define WAIT_BFGX_READ_STACK_TIME   10000   /* 等待bfgx读栈操作完成的时间，5000毫秒 */
#define BFGX_BEAT_TIME              3       /* bfgx心跳超时时间为3秒钟 */
#define WIFI_DUMP_BCPU_TIMEOUT      1000    /* wifi dump bcpu数据等待时间 */

#define PLAT_EXCEPTION_RESET_IDLE   0   /* plat 没有在处理异常 */
#define PLAT_EXCEPTION_RESET_BUSY   1   /* plat 正在处理异常 */

#define BFGX_NOT_RECV_BEAT_INFO     0   /* host没有收到心跳消息 */
#define BFGX_RECV_BEAT_INFO         1   /* host收到心跳消息 */

#define RF_TEMERATURE_NORMAL        0   /* rf温度正常 */
#define RF_TEMERATURE_OVERHEAT      1   /* rf温度过热 */

#define EXCEPTION_SUCC              0
#define EXCEPTION_FAIL              1

#define DFR_TEST_ENABLE             0
#define DFR_TEST_DISABLE            1

#define BFGX_MEM_DUMP_BLOCK_COUNT   2       /* BFGX 上报的内存块个数 */
#define UART_READ_WIFI_MEM_TIMEOUT  10000
#define UART_HALT_WCPU_TIMEOUT      500

#define BFGX_MAX_RESET_CMD_LEN      9
#define BT_RESET_CMD_LEN            4
#define FM_RESET_CMD_LEN            1
#define GNSS_RESET_CMD_LEN          9
#define IR_RESET_CMD_LEN            4
#define NFC_RESET_CMD_LEN           4

#define SDIO_STORE_BFGX_REGMEM "readm_bfgx_sdio"
#define UART_STORE_BFGX_STACK "readm_bfgx_uart"
#define UART_STORE_WIFI_MEM "readm_wifi_uart"
#define SDIO_STORE_WIFI_MEM "readm_wifi_sdio"

#define BFGX_DUMP_PATH "/data/memdump"
#define WIFI_DUMP_PATH "/etc"

/* nfc buffer参数 */
#define OML_STATUS_ADD_LENGTH       13
#define NFCLOGLEN                   (4 * sizeof(osal_u32) + (OML_STATUS_ADD_LENGTH))
#define NFCLOGNUM                   100
#define OMLNFCDATABUFFLEN           ((NFCLOGNUM) * (NFCLOGLEN))
#define NFC_SEND_LEN_LIMIT          ((NFCLOGNUM) * (NFCLOGLEN))

#define MEMDUMP_ROTATE_QUEUE_MAX_LEN    10
#define ARP_TIMEOUT_MAX_TIMES           10


enum DFR_RST_SYSTEM_TYPE_E {
    DFR_SGL_SYS_RST = 0,
    DFR_ALL_SYS_RST = 1,
    DFR_SYS_RST_TYPE_BOTTOM,
};

enum SUBSYSTEM_E {
    SUB_SYSTEM_WIFI = 0,
    SUB_SYSTEM_BFGX = 1,
    SUB_SYSTEM_BOTTOM,
};

enum WIFI_THREAD_E {
    THREAD_IS_WIFI = 0,
    WIFI_THREAD_BOTTOM,
};

enum BFGX_THREAD_E {
    THREAD_BT = 0,
    THREAD_FM = 1,
    THREAD_GNSS = 2,
    THREAD_IR = 3,
    THREAD_NFC = 4,
    BFGX_THREAD_BOTTOM,
};

enum EXCEPTION_TYPE_E {
    /* bfgx */
    BFGX_BEATHEART_TIMEOUT  = 0,
    BFGX_LASTWORD_PANIC     = 1,    /* 临终遗言 */
    BFGX_TIMER_TIMEOUT      = 2,
    BFGX_ARP_TIMEOUT        = 3,
    BFGX_POWERON_FAIL       = 4,
    BFGX_WAKEUP_FAIL        = 5,

    /* wifi */
    WIFI_WATCHDOG_TIMEOUT   = 6,
    WIFI_POWERON_FAIL       = 7,
    WIFI_WAKEUP_FAIL        = 8,
    WIFI_DEVICE_PANIC       = 9,
    WIFI_TRANS_FAIL         = 10,
    SDIO_DUMPBCPU_FAIL,             /* sdio dump fail */

    EXCEPTION_TYPE_BOTTOM,
};

enum UART_WIFI_MEM_DUMP {
    WIFI_PUB_REG = 0,               /* wifi 公共寄存器 */
    WIFI_PRIV_REG = 1,              /* wifi 私有寄存器 */
    WIFI_MEM = 2,                   /* wifi memery */
    UART_WIFI_MEM_DUMP_BOTTOM,      /* bottom */
};

enum DUMP_CMD_TYPE {
    CMD_READM_WIFI_SDIO = 0,        /* wifi sdio read memery */
    CMD_READM_WIFI_UART = 1,        /* wifi uart read memery */
    CMD_READM_BFGX_UART = 2,        /* bt uart read memery */
    CMD_READM_BFGX_SDIO = 3,        /* bt sdio read memery */
    CMD_DUMP_BUFF,                  /* bottom */
};

enum excp_test_cfg_em {
    BFGX_POWEON_FAULT = 0,          /* bt 上电失败 */
    BFGX_POWEOFF_FAULT = 1,         /* bt 下电失败 */
    WIFI_WKUP_FAULT = 2,            /* wifi 唤醒失败 */
    EXCP_TEST_CFG_BOTT,             /* bottom */
};

struct st_uart_dump_wifi_mem_info {
    osal_u8 *file_name;
    osal_u32 size;
};

struct st_uart_dump_wifi_info
{
    osal_u32 cmd;
    osal_u32 total_size;
    osal_u32 block_count;
    struct st_uart_dump_wifi_mem_info *block_info;
};

struct st_exception_mem_info
{
    osal_u8 *exception_mem_addr;
    osal_u32 total_size;
    osal_u32 recved_size;
    osal_u8 *file_name;
};

struct st_wifi_dump_mem_info
{
    osal_ulong mem_addr;
    osal_u32 size;
    osal_u8 *file_name;
};

struct st_wifi_dfr_callback
{
    void (*wifi_recovery_complete)(void);
    void (*notify_wifi_to_recovery)(void);
};

struct st_bfgx_reset_cmd
{
    osal_u32 len;
    osal_u8 cmd[BFGX_MAX_RESET_CMD_LEN];
};

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
struct excp_type_info_s {
    osal_u32 excp_cnt;
    osal_u32 fail_cnt;
    osal_u32 rst_type_cnt[DFR_SYS_RST_TYPE_BOTTOM];
    ktime_t stime;
    osal_u64 maxtime;
};

struct st_exception_info {
    osal_u32 exception_reset_enable;
    osal_u32 subsys_type;
    osal_u32 excetion_type;

    atomic_t bfgx_beat_flag;
    atomic_t is_reseting_device;

    struct workqueue_struct *plat_exception_rst_workqueue;
    struct work_struct plat_exception_rst_work;
    struct work_struct uart_store_wifi_mem_to_file_work;
    struct timer_list bfgx_beat_timer;

    struct completion wait_read_bfgx_stack;
    struct completion wait_sdio_d2h_dump_ack;

    struct bt_plat_s *ps_plat_d;
    struct st_wifi_dfr_callback *wifi_callback;

    spinlock_t exception_spin_lock;

    struct excp_type_info_s etype_info[EXCEPTION_TYPE_BOTTOM];

    /* 下边的变量调试使用 */
    osal_u32 debug_beat_flag;
    osal_u32 subsystem_rst_en;

    /* wifi打开BCPU for sdio mem dump，该功能仅在调试时使用，默认关闭 */
    osal_u32 wifi_open_bcpu_enable;

    struct completion wait_uart_read_wifi_mem;
    struct completion wait_uart_halt_wcpu;

    /* wifi异常触发 */
    struct work_struct wifi_excp_worker;
    struct work_struct wifi_excp_recovery_worker;
    struct workqueue_struct *wifi_exception_workqueue;
    osal_u32 wifi_excp_type;

};
#endif

struct sdio_dump_bcpu_buff
{
    osal_u8 *mem_addr;
    osal_u32 data_limit;
    osal_u32 data_len;
};

typedef struct excp_info_str_s {
    osal_u32 id;
    char *name;
} excp_info_str_t;

/*****************************************************************************
  4 EXTERN VARIABLE
*****************************************************************************/

/*****************************************************************************
  5 全局变量定义
*****************************************************************************/

/*****************************************************************************
  6 EXTERN FUNCTION
*****************************************************************************/
extern osal_s32 plat_wifi_exception_rst_register_etc(void *data);
extern osal_s32 wifi_open_bcpu_set_etc(osal_u8 enable);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

