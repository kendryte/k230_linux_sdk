/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: host platform pm dfr source file
 * Create: 2023-02
 */

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/namei.h>
#endif

#include "soc_osal.h"
#include "hcc.h"
#include "hcc_if.h"
#include "hcc_bus.h"
#include "plat_service.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "plat_pm_board.h"
#include "plat_firmware.h"
#include "plat_exception_rst.h"
#include "plat_hcc_srv.h"
#include "oal_kernel_file.h"
#include "plat_pm_dfr.h"

#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#include "last_dump.h"
#endif
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
#include <linux/gpio.h>
#include "hcc_uart_host.h"
#endif
#if defined(CONFIG_HCC_SUPPORT_USB)
#include "hcc_usb_host.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(CONFIG_PLAT_SUPPORT_DFR)

typedef struct {
    td_u32 t6;    /* X31 */
    td_u32 t5;    /* X30 */
    td_u32 t4;    /* X29 */
    td_u32 t3;    /* X28 */

    td_u32 a7;    /* X17 */
    td_u32 a6;    /* X16 */
    td_u32 a5;    /* X15 */
    td_u32 a4;    /* X14 */
    td_u32 a3;    /* X13 */
    td_u32 a2;    /* X12 */
    td_u32 a1;    /* X11 */
    td_u32 a0;    /* X10 */

    td_u32 t2;    /* X7 */
    td_u32 t1;    /* X6 */
    td_u32 t0;    /* X5 */

    td_u32 ra;    /* X1 */
} exc_context;

#pragma pack(push, 1)
#define SYSERR_PRINT_SIZE 32

// DFR连续尝试最大次数
#define DFR_RETRY_MAX_TIME 5

/********************* for firmware *******************************/
#define WCPU_PANIC_MEMDUMP_BASE_ADDR          0x4052B8
#define FW_HEAP_ADDR                          0x420000
#define FW_HEAP_START_ADDR                    (FW_HEAP_ADDR + 32 * 1024)
#define FW_HEAP_SAVE_LEN                      (32 * 1024)
#define FW_HEAP_LEN                           (64 * 1024)
#define FW_PATCH_START_ADDR                   0x41a5c0
#define FW_PATCH_LEN                          1024
#define FW_TEXT_START_ADDR                    (WCPU_PANIC_MEMDUMP_BASE_ADDR + 1024)
#define LEFT_NEED_SAVE_LEN                    (FW_HEAP_LEN - FW_HEAP_SAVE_LEN - FW_PATCH_LEN)
#define FW_TEXT_LEN                           (8 * 1024)
#define MEMORY_DFR_STACK_FRAME_LEN            1024

#define FW_BSSDATA_START_ADDR                 0x4185b0
#define FW_BSSDATA_LEN                        (0x41ace0 - 0x4185b0)
#define FW_STACK_ADDR                         0x425c00
#define FW_STACK_LEN                          (4 * 1024)
#define WIFI_TEXT_START_ADDR                  0x406f78
#define WIFI_TEXT_LEN                         (37 * 1024)

typedef struct {
    osal_u32 ccause;
    osal_u32 mcause;
    osal_u32 mtval;
    osal_u32 mstatus;
    osal_u32 mepc;
} wcpu_memdump_hdr_stru;

typedef struct {
    osal_u32 depth;
    osal_u32 stack_pc;
} backtrace_info;

typedef struct {
    osal_u32 stack[SYSERR_PRINT_SIZE];
    backtrace_info bk_info[SYSERR_PRINT_SIZE];
} stack_info;

typedef struct {
    osal_u32  ram_text_start;
    osal_u32  ram_text_end;
    osal_u32  stack_top;
    osal_u32  rom_text_start;
    osal_u32  rom_text_end;
} wl_text_info_t;

typedef struct {
    wcpu_memdump_hdr_stru hdr;                /* 24B */
    exc_context exc_reg;                      /* 64B */
    stack_info st_info_entry;                 /* 384B */
    wl_text_info_t text_info;
    osal_u32 sp;                              /* 4B */
    osal_u32 stack_end;   /* 4B */
    td_u32 panic_id;
    td_u32 panic_code;
} wcpu_panic_memdump_stru; /* 476B */

#pragma pack(pop)

struct st_wifi_dump_mem_info g_wifi_meminfo[] = {
    {
        .file_name = "fm_device_panic_mem",
        .mem_addr = WCPU_PANIC_MEMDUMP_BASE_ADDR,
        .size = MEMORY_DFR_STACK_FRAME_LEN, /* 0.5KB */
    },
    {
        .file_name = "fm_device_panic_bssdata",
        .mem_addr = FW_BSSDATA_START_ADDR, // 待定
        .size = FW_BSSDATA_LEN,

    },
    {
        .file_name = "fm_device_panic_heap_block0",
        .mem_addr = FW_STACK_ADDR,
        .size = FW_STACK_LEN,
    },
    {
        .file_name = "fm_device_panic_heap_block1",
        .mem_addr = WIFI_TEXT_START_ADDR, // text
        .size = WIFI_TEXT_LEN,
    }
};

struct st_wifi_dump_mem_info g_wifi_stack_info[] = {
    {
        .file_name = "fm_device_panic_mem",
        .mem_addr = WCPU_PANIC_MEMDUMP_BASE_ADDR,
        .size = MEMORY_DFR_STACK_FRAME_LEN, /* 0.5KB */
    },
    {
        .file_name = "fm_device_panic_bssdata",
        .mem_addr = FW_BSSDATA_START_ADDR,
        .size = FW_BSSDATA_LEN,

    },
    {
        .file_name = "fm_device_panic_full_stack",
        .mem_addr = FW_STACK_ADDR,
        .size = FW_STACK_LEN,
    }
};

/* 函数声明 */
osal_void dump_panic_msg(osal_void);
osal_s32 device_panic_msg_record_etc(osal_void);

/* 全局变量定义 */
// 标识wlan是否在自愈
osal_u8 g_wlan_dfr_recovery_flag = OSAL_FALSE;
osal_bool g_should_dump_trace = OSAL_FALSE;

osal_u32 g_dfr_retry_count = 0;

wifi_driver_event_cb g_wifi_driver_event_callback = NULL;

recovery_complete g_dfr_plat_complete_wifi_cb = NULL;
recovery_complete g_dfr_plat_complete_bt_cb = NULL;

recovery_complete g_dfr_plat_complete_sle_cb = NULL;

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_s32 g_dfr_status = DFR_ATOMIC_STATUS_IDLE;
osal_bool g_dfr_enable_flag = OSAL_FALSE;
osal_bool g_dfr_wait_flag = OSAL_FALSE;
osal_mutex g_dfr_mutex = {0};
struct workqueue_struct *wifi_exception_workqueue;
struct work_struct plat_exception_rst_work;
#endif

osal_u8 plat_get_wifi_dfr_flag(osal_void)
{
    return g_wlan_dfr_recovery_flag;
}
EXPORT_SYMBOL_GPL(plat_get_wifi_dfr_flag);

osal_void wlan_set_dfr_recovery_flag(osal_u8 dfr_flag)
{
    g_wlan_dfr_recovery_flag = dfr_flag;
}
EXPORT_SYMBOL_GPL(wlan_set_dfr_recovery_flag);


osal_s32 plat_wifi_exception_rst_register_etc(osal_void *data)
{
    if (data == NULL) {
        oal_print_err("plat_wifi_exception_rst_register_etc data null\n");
        return -EXCEPTION_FAIL;
    }
    g_dfr_plat_complete_wifi_cb = (recovery_complete)data;
    return EXCEPTION_SUCC;
}
EXPORT_SYMBOL_GPL(plat_wifi_exception_rst_register_etc);

osal_s32 plat_bt_exception_rst_register_etc(osal_void *data)
{
    if (data == NULL) {
        oal_print_err("plat_bt_exception_rst_register_etc data null\n");
        return -EXCEPTION_FAIL;
    }
    g_dfr_plat_complete_bt_cb = (recovery_complete)data;
    return EXCEPTION_SUCC;
}
EXPORT_SYMBOL_GPL(plat_bt_exception_rst_register_etc);

osal_s32 plat_sle_exception_rst_register_etc(osal_void *data)
{
    if (data == NULL) {
        oal_print_err("plat_sle_exception_rst_register_etc data null\n");
        return -EXCEPTION_FAIL;
    }
    g_dfr_plat_complete_sle_cb = (recovery_complete)data;
    return EXCEPTION_SUCC;
}
EXPORT_SYMBOL_GPL(plat_sle_exception_rst_register_etc);

oal_void set_wlan_flag_close(oal_void)
{
    struct wlan_pm_s    *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == NULL) {
        oal_print_err("pst_wlan_pm NULL\n");
        return;
    }
    pst_wlan_pm->pm_power_state = POWER_STATE_SHUTDOWN;
    pst_wlan_pm->wlan_state = PM_SVC_STATE_SHUTDOWN;
    pst_wlan_pm->pm_enable = OSAL_FALSE;
    oal_print_err("wlan_state WLAN_STATE_SHUTDOWN\n");
    return;
}
EXPORT_SYMBOL_GPL(set_wlan_flag_close);
osal_s32 device_panic_msg_record_etc(osal_void)
{
    oal_int32 ret;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    oal_print_err("in ...\n");

    if (NULL == pm_data) {
        oal_print_err("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    oal_wake_lock(&pm_data->st_wakelock);

    if (g_should_dump_trace != OSAL_FALSE) {
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
        hcc_uart_set_transfer_mode(0); // 0 boot阶段
#endif
        ret = hcc_bus_reinit(pm_data->pst_wlan_pm_info->pst_bus);
        if (ret != OAL_SUCC) {
            oal_wake_unlock(&pm_data->st_wakelock);
            oal_print_err("hcc reinit failed, ret:%d!\n", ret);
            return -FAILURE;
        }
    }
    hcc_switch_status(HCC_BUS_FORBID);

    if (g_should_dump_trace != OSAL_FALSE) {
        dump_panic_msg();
    }

    oal_wake_unlock(&pm_data->st_wakelock);
    return EXCEPTION_SUCC;
}

td_void hcc_reg_write_bits(td_u32 reg_addr, td_u8 offset, td_u8 bits, td_u32 reg_val)
{
    td_u32 value;
    ext_errno ret;
    ret = hcc_read_reg(HCC_CHANNEL_AP, reg_addr, &value);
    if (ret != EXT_ERR_SUCCESS) {
        osal_printk("hcc_read_reg fail");
    }
    value &= ~((((osal_u32)1 << bits) - 1) << offset);
    value |= ((reg_val & (((osal_u32)1 << bits) - 1)) << offset);
    ret = hcc_write_reg(HCC_CHANNEL_AP, reg_addr, value);
    if (ret != EXT_ERR_SUCCESS) {
        osal_printk("hcc_write_reg fail");
    }
}

td_void plat_dfr_trigger_panic(td_void)
{
#define PLAT_DFR_TRIGGER_HCC_MSG_SIZE 16
    td_u32 size[PLAT_DFR_TRIGGER_HCC_MSG_SIZE] = {0};
    td_u32 ret = 0;
    oal_print_err("plat_dfr_trigger_panic, id: %u\r\n", H2D_PLAT_CFG_MSG_TRIGGER_PANIC);
    ret = plat_msg_hcc_send((td_u8 *)size, sizeof(size), H2D_PLAT_CFG_MSG_TRIGGER_PANIC);
    oal_print_err("plat_dfr_trigger_panic, ret: %u\r\n", ret);
}
EXPORT_SYMBOL(plat_dfr_trigger_panic);

osal_s32 wifi_driver_event_callback_register(wifi_driver_event_cb event_cb)
{
    if (event_cb == NULL) {
        oal_print_err("invalid param!\n");
        return -OAL_FAIL;
    }

    if (g_wifi_driver_event_callback != NULL) {
        oal_print_err("wifi_driver_event_callback is already registered.\n");
        return -OAL_FAIL;
    }
    g_wifi_driver_event_callback = event_cb;

    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(wifi_driver_event_callback_register);

static osal_u8 g_uc_device_recovery_flag = 0;

td_u8 plat_is_device_in_recovery(td_void)
{
    return g_uc_device_recovery_flag;
}

td_void plat_update_device_recovery_flag(td_u8 flag)
{
    g_uc_device_recovery_flag = flag;
}

EXPORT_SYMBOL(plat_is_device_in_recovery);
EXPORT_SYMBOL(plat_update_device_recovery_flag);

void plat_exception_reset_work_etc(struct work_struct *work)
{
    osal_s32 dfr_status, ret;
    osal_bool dfr_fail_flag = OSAL_FALSE;

    plat_dfr_lock();
    dfr_status = plat_dfr_status_get();
    if (dfr_status != DFR_ATOMIC_STATUS_DFR_PREPARE) {
        plat_dfr_unlock();
        oal_print_err("dfr: status wrong, dfr_status[%d], return.\r\n", dfr_status);
        return;
    }
    plat_dfr_status_set(DFR_ATOMIC_STATUS_DFR_PROCESS);
    plat_dfr_unlock();

    oal_print_warning("dfr: reset work start.\r\n");
    hcc_switch_status(HCC_BUS_FORBID);

    // 0 device panic event callback exec
    if (g_wifi_driver_event_callback) {
        g_wifi_driver_event_callback(DEV_PANIC);
    }

#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
    gpio_state_change_cfg2uart();
#endif

    // 1.读取device panic信息
    device_panic_msg_record_etc();

    // 2.业务清理
    pm_suspend_service();

    // 3.device下电
    ret = pm_svc_power_off();
    if (ret != OAL_SUCC) {
        oal_print_err("dfr: pm_svc_power_off fail!. \r\n");
        dfr_fail_flag = OSAL_TRUE;
    }
    hcc_switch_status(HCC_BUS_FORBID);
#if defined(CONFIG_HCC_SUPPORT_USB)
    usb_reset_status();
#endif

    // 4.device重新初始化
    ret = pm_svc_power_on();
    if (ret != OAL_SUCC) {
        oal_print_err("dfr: pm_svc_open fail!. \r\n");
        dfr_fail_flag = OSAL_TRUE;
    }

    // 5.业务恢复
    pm_resume_service(&dfr_fail_flag);

    plat_update_device_recovery_flag(TD_FALSE);

    plat_dfr_lock();
    plat_dfr_status_set(DFR_ATOMIC_STATUS_IDLE);
    plat_dfr_unlock();

    if (dfr_fail_flag != OSAL_TRUE) {
        oal_print_warning("dfr: recover success!\r\n");
        g_dfr_retry_count = 0;
        return;
    }

    if (g_dfr_retry_count >= DFR_RETRY_MAX_TIME) {
        oal_print_err("dfr: recover failed, retried max times [%u], return.\r\n", g_dfr_retry_count);
        g_dfr_retry_count = 0;
        return;
    }

    g_dfr_retry_count++;
    oal_print_err("dfr: recover failed, retry! retried count[%u].\r\n", g_dfr_retry_count);
    plat_exception_reset_process(OSAL_FALSE);
}

osal_s32 plat_exception_init(osal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == OSAL_NULL) {
        return -OAL_FAIL;
    }
    hcc_message_register(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, D2H_MSG_DEVICE_PANIC, plat_exception_reset_work,
        (osal_u8 *)pst_wlan_pm);

    if (osal_mutex_init(&g_dfr_mutex) != OSAL_SUCCESS) {
        return -OAL_FAIL;
    }

    wifi_exception_workqueue = create_singlethread_workqueue("plat_exception_reset_queue");
    if (wifi_exception_workqueue == OSAL_NULL) {
        osal_mutex_destroy(&g_dfr_mutex);
        return -OAL_FAIL;
    }
    INIT_WORK(&plat_exception_rst_work, plat_exception_reset_work_etc);
    return OAL_SUCC;
}

osal_void plat_exception_exit(osal_void)
{
    cancel_work_sync(&plat_exception_rst_work);
    destroy_workqueue(wifi_exception_workqueue);
    osal_mutex_destroy(&g_dfr_mutex);
}

osal_void plat_dfr_enable_set(osal_bool enable)
{
    g_dfr_enable_flag = enable;
}

osal_s32 plat_dfr_status_get(osal_void)
{
    return g_dfr_status;
}

osal_void plat_dfr_status_set(osal_s32 dfr_status)
{
    g_dfr_status = dfr_status;
}

osal_bool plat_dfr_wait_flag_get(osal_void)
{
    return g_dfr_wait_flag;
}

osal_void plat_dfr_wait_flag_set(osal_bool flag)
{
    g_dfr_wait_flag = flag;
}

osal_void plat_dfr_lock(osal_void)
{
    osal_mutex_lock(&g_dfr_mutex);
}

osal_void plat_dfr_unlock(osal_void)
{
    osal_mutex_unlock(&g_dfr_mutex);
}

td_void set_hcc_bus_close(td_void)
{
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc != TD_NULL && hcc->bus != TD_NULL) {
        hcc->bus->state = DFR_BUS_STATE_CLOSE_REG;
    }
}
osal_module_export(set_hcc_bus_close);

// dfr
osal_u32 plat_exception_reset_process(osal_bool should_dump_trace)
{
    osal_s32 dfr_status;

    oal_print_err("plat_exception_reset_work: enter.\r\n");

    if (g_dfr_enable_flag != OSAL_TRUE) {
        oal_print_err("dfr: not enabled, return.\r\n");
        return SUCCESS;
    }

    plat_dfr_lock();
    dfr_status = plat_dfr_status_get();
    if (dfr_status >= DFR_ATOMIC_STATUS_DFR_PREPARE) {
        if (dfr_status == DFR_ATOMIC_STATUS_STR_RESUME) {
            plat_dfr_wait_flag_set(OSAL_TRUE);
        }
        plat_dfr_unlock();
        oal_print_err("dfr: already in dfr process, dfr_status[%d], return.\r\n", dfr_status);
        return SUCCESS;
    }
    plat_dfr_status_set(DFR_ATOMIC_STATUS_DFR_PREPARE);
    plat_dfr_unlock();

    g_should_dump_trace = should_dump_trace;
    set_hcc_bus_close();
    hcc_switch_status(HCC_BUS_FORBID);
    plat_update_device_recovery_flag(TD_TRUE);
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
    gpio_state_change_uart2cfg();
    hcc_uart_tx_thread_wakeup();
#endif
    oal_print_err("~~~~~~plat_exception_reset_work,init\n");
    queue_work(wifi_exception_workqueue, &plat_exception_rst_work);
    return SUCCESS;
}

osal_u32 plat_exception_reset_work(osal_u8 *data)
{
    return plat_exception_reset_process(OSAL_TRUE);
}

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
os_kernel_file_stru *open_file_to_readm_etc(osal_u8 *name, osal_s32 flags)
{
    os_kernel_file_stru *fp;
    osal_u8 *file_name = NULL;

    if (NULL == name) {
        file_name = CONFIG_PLAT_DFR_OUTPUT_PATH "/readm_wifi";
    } else {
        file_name = name;
    }

    fp = filp_open(file_name, flags, 0644); // 文件权限 0644
    return fp;
}

// osal_s32 recv_device_mem_etc(os_kernel_file_stru *fp, osal_u8 *puc_data_buf, osal_s32 len)
// {
//     osal_s32 l_ret = -OAL_FAIL;
//     mm_segment_t fs;
//     osal_u8 retry = 3;
//     osal_s32 lenbuf = 0;

//     if (OAL_IS_ERR_OR_NULL(fp)) {
//         oal_print_err("fp is error,fp = 0x%p\n", fp);
//         return -OAL_FAIL;
//     }

//     if (NULL == puc_data_buf) {
//         oal_print_err("puc_data_buf is NULL\n");
//         return -OAL_FAIL;
//     }

//     oal_print_dbg("expect recv len is [%d]\n", len);

//     fs = oal_get_fs();

//     oal_print_dbg("pos = %d\n", (int)fp->f_pos);
//     while (len > lenbuf) {
//         l_ret = firmware_recv_func(puc_data_buf + lenbuf, len - lenbuf);
//         if (l_ret > 0) {
//             lenbuf += l_ret;
//         } else {
//             retry--;
//             lenbuf = 0;
//             if (0 == retry) {
//                 l_ret = -OAL_FAIL;
//                 oal_print_err("time out\n");
//                 break;
//             }
//         }
//     }

//     if (len <= lenbuf) {
// #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
//         vfs_write(fp, puc_data_buf, len, &fp->f_pos);
// #else
//         kernel_write(fp, puc_data_buf, len, &fp->f_pos);
// #endif
//     }
// #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
//     vfs_fsync(fp, 0);
// #else
//     vfs_fsync(fp, fp->f_path.dentry, 0);
// #endif
//     oal_set_fs(fs);

//     return l_ret;
// }
osal_s32 recv_device_mem_etc(struct file *fp, osal_u8 *puc_data_buf, osal_s32 len)
{
    osal_s32 l_ret = -OAL_FAIL;
    osal_u8 retry = 3;
    osal_s32 lenbuf = 0;

    if (IS_ERR_OR_NULL(fp)) {
        oal_print_err("fp is error, fp = 0x%p\n", fp);
        return -OAL_FAIL;
    }

    if (puc_data_buf == NULL) {
        oal_print_err("puc_data_buf is NULL\n");
        return -OAL_FAIL;
    }

    oal_print_dbg("expect recv len is [%d]\n", len);

    oal_print_dbg("pos = %lld\n", fp->f_pos);

    while (len > lenbuf) {
        l_ret = firmware_recv_func(puc_data_buf + lenbuf, len - lenbuf);
        if (l_ret > 0) {
            lenbuf += l_ret;
        } else {
            retry--;
            lenbuf = 0;
            if (retry == 0) {
                l_ret = -OAL_FAIL;
                oal_print_err("time out\n");
                break;
            }
        }
    }

    if (len <= lenbuf) {
        /* 直接使用 kernel_write，无需权限切换 */
        l_ret = kernel_write(fp, puc_data_buf, len, &fp->f_pos);
        if (l_ret < 0) {
            oal_print_err("write failed: %d\n", l_ret);
            return -OAL_FAIL;
        }
    }

    /* 统一 vfs_fsync 调用方式 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, 0);
#endif

    return (lenbuf >= len) ? OSAL_SUCCESS : -OAL_FAIL;
}
#endif

static osal_u32 syserr_backtrace_print(wcpu_panic_memdump_stru *panic_dump_info)
{
    osal_u32 i = 0;
    osal_u32 stack_end = panic_dump_info->stack_end;
    stack_info *stk_hdr = &(panic_dump_info->st_info_entry);
    for (i = 0; i < stack_end; i++) {
        oal_print_err("backtrace %u -- pc = 0x%x\n", stk_hdr->bk_info[i].depth, stk_hdr->bk_info[i].stack_pc);
    }

    for (i = 0; i < SYSERR_PRINT_SIZE; i += 4) { /* one row print 4 words */
        oal_print_err("[stack: 0x%08x 0x%08x 0x%08x 0x%08x]\r\n",
            stk_hdr->stack[i], stk_hdr->stack[i + 1], stk_hdr->stack[i + 2], stk_hdr->stack[i + 3]); /* 偏移1/2/3 */
    }
    uapi_unused(stk_hdr);
    return 0;
}
static osal_void syserr_nonos_exc_print(wcpu_panic_memdump_stru *panic_dump_info)
{
    exc_context *exc_reg = &(panic_dump_info->exc_reg);
    wcpu_memdump_hdr_stru *memdump_hdr = &(panic_dump_info->hdr);
    oal_print_err("the panic id [%u], code [%u]\n", panic_dump_info->panic_id, panic_dump_info->panic_code);
    oal_print_err("ccause :0x%x\r\nmcause :0x%x\r\nmtval  :0x%08x\r\nmstatus:0x%08x\r\nmepc   :0x%x\r\n",
        (td_u8)memdump_hdr->ccause, memdump_hdr->mcause, memdump_hdr->mtval, memdump_hdr->mstatus,
        memdump_hdr->mepc);
    oal_print_err("ra:0x%x\n""sp:0x%x\n""t0:0x%x\n""t1:0x%x\n""t2:0x%x\n",
        exc_reg->ra, panic_dump_info->sp, exc_reg->t0, exc_reg->t1, exc_reg->t2);
    oal_print_err("a0:0x%x\n""a1:0x%x\n""a2:0x%x\n""a3:0x%x\n""a4:0x%x\n""a5:0x%x\n""a6:0x%x\n""a7:0x%x\n",
        exc_reg->a0, exc_reg->a1, exc_reg->a2, exc_reg->a3, exc_reg->a4, exc_reg->a5,
        exc_reg->a6, exc_reg->a7);
    oal_print_err("t3:0x%x\n""t4:0x%x\n""t5:0x%x\n""t6:0x%x\n",
        exc_reg->t3, exc_reg->t4, exc_reg->t5, exc_reg->t6);
    oal_print_err("backtrace %u -- pc = 0x%x\n", 0, memdump_hdr->mepc);
    uapi_unused(memdump_hdr);
    uapi_unused(exc_reg);
    syserr_backtrace_print(panic_dump_info);
    return;
}

osal_s32 hcc_read_device_mem_etc(struct st_wifi_dump_mem_info *pst_mem_dump_info, os_kernel_file_stru *fp,
    osal_u8 *puc_data_buf, osal_u32 ul_data_buf_len)
{
    osal_u8 buf_tx[SEND_BUF_LEN];
    osal_s32 ret = 0;
    osal_u32 size = 0;
    osal_u32 offset;
    osal_u32 remainder = pst_mem_dump_info->size;
    wcpu_panic_memdump_stru *device_dump_info;
    oal_print_err("read addr :[%lx], size [%u]\n", pst_mem_dump_info->mem_addr, remainder);
    offset = 0;
    while (remainder > 0) {
        (osal_void)memset_s(buf_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

        size = min(remainder, ul_data_buf_len);
        (osal_void)snprintf_s(buf_tx, sizeof(buf_tx), SEND_BUF_LEN, "%s%c0x%lx%c%u%c", RMEM_CMD_KEYWORD,
            COMPART_KEYWORD, pst_mem_dump_info->mem_addr + offset, COMPART_KEYWORD, size, COMPART_KEYWORD);
        firmware_send_func(buf_tx, SEND_BUF_LEN, strlen(buf_tx));

        osal_klib_fseek(offset, SEEK_SET, fp); // again move to the head of file
        ret = recv_device_mem_etc(fp, puc_data_buf, size);
        if (ret < 0) {
            oal_print_err("wifi mem dump fail, filename is [%s],ret=%d\n", pst_mem_dump_info->file_name, ret);
            break;
        }

#ifdef CONFIG_PRINTK
        if (0 == offset) {
            oal_int8 *pst_file_name =
                (pst_mem_dump_info->file_name ? ((oal_int8 *)pst_mem_dump_info->file_name) : (oal_int8 *)"default: ");
            if (!strcmp("fm_device_panic_mem", pst_file_name)) {
                device_dump_info = (wcpu_panic_memdump_stru *)puc_data_buf;
                syserr_nonos_exc_print(device_dump_info);
            }
        }
#endif

        offset += size;
        remainder -= size;
    }

    return ret;
}

#define READM_RAM_SIZE 10
#define RAM_INDEX_0 0
#define RAM_INDEX_1 1
#define RAM_INDEX_2 2
#define RAM_INDEX_3 3
#define RAM_INDEX_4 4
#define RAM_INDEX_5 5
#define RAM_INDEX_6 6

static osal_s32 wifi_device_mem_dump_dfx(struct st_wifi_dump_mem_info *pst_mem_dump_info, os_kernel_file_stru *fp)
{
    osal_u8 *puc_data_buf = NULL;
    osal_s32 file_len;
    osal_s32 rdlen;

    osal_klib_fseek(0, SEEK_SET, fp); // move fp->f_pos to the head of file
    file_len = osal_klib_fseek(0, SEEK_END, fp); // get size of file
    oal_print_err("dfx dump dev mem: file_name[%s], file_len[%d]\n", pst_mem_dump_info->file_name, file_len);
    puc_data_buf = (osal_u8 *)osal_kmalloc(file_len, 0);
    if (puc_data_buf == NULL) {
        oal_print_err("dfx dump puc_data_buf KMALLOC failed\n");
        return -OAL_FAIL;
    }
    memset_s(puc_data_buf, file_len, 0, file_len);
    osal_klib_fseek(0, SEEK_SET, fp); // again move to the head of file
    rdlen = osal_klib_fread(puc_data_buf, file_len, fp);
    if (rdlen > 0) {
        oal_print_dbg("osal_klib_fread [%d], for dfx dump dev info\n", rdlen);
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
        dfx_last_dump(pst_mem_dump_info->file_name, (uintptr_t)puc_data_buf, file_len);
#endif
    } else {
        oal_print_err("osal_klib_fread error, rdlen ret %d\n", rdlen);
        osal_kfree(puc_data_buf);
        return -OAL_FAIL;
    }
    osal_kfree(puc_data_buf);
    return OAL_SUCC;
}

#define FILE_NAME_LEN 100
#define FILE_MAX_CNT 3

osal_s32 wifi_device_mem_dump_get_stat_adapt(osal_char *file_name, struct kstat *file_stat)
{
    osal_s32 ret = OAL_SUCC;
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    struct path file_path;
    ret = user_path_at_empty(AT_FDCWD, file_name, LOOKUP_FOLLOW, &file_path, NULL);
    if (ret != 0) {
        return ret;
    }
    vfs_getattr(&file_path, file_stat, STATX_BASIC_STATS, AT_NO_AUTOMOUNT);
#elif (defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)))
    ret = vfs_stat(file_name, file_stat);
#endif
    return ret;
}

osal_u32 wifi_device_mem_dump_get_suffix(osal_u8 *file_name)
{
    osal_char final_filename[FILE_MAX_CNT][FILE_NAME_LEN] = {0};
    osal_ulong time = (osal_ulong)INT_MAX;
    struct kstat file_stat = {0};
    osal_u32 i;
    osal_u32 suffix = 0;
    osal_s32 ret;
    for (i = 0; i < FILE_MAX_CNT; i++) {
        (osal_void)memset_s(final_filename[i], sizeof(final_filename[i]), 0, sizeof(final_filename[i]));
        ret = snprintf_s(final_filename[i], sizeof(final_filename[i]), FILE_NAME_LEN,
            CONFIG_PLAT_DFR_OUTPUT_PATH "/%s_%d.bin", file_name, i);
        if (ret < 0) {
            oal_print_err("excute snprintf_s file[%s] failed, return [%d]\n", file_name, ret);
            return suffix;
        }
        if (wifi_device_mem_dump_get_stat_adapt(final_filename[i], &file_stat) != 0) {
            oal_print_err("file[%s] not existed, will create\n", final_filename[i]);
            suffix = i;
            break;
        }
        if (file_stat.mtime.tv_sec < time) {
            time = (osal_ulong)file_stat.mtime.tv_sec;
            suffix = i;
        }
    }
    return suffix;
}

osal_s32 wifi_device_mem_dump_get_filename(osal_u8 *final_filename, osal_u32 final_filename_len, osal_u8 *file_name)
{
    osal_s32 ret;
    (osal_void)memset_s(final_filename, final_filename_len, 0, final_filename_len);
    ret = snprintf_s(final_filename, final_filename_len, FILE_NAME_LEN, CONFIG_PLAT_DFR_OUTPUT_PATH "/%s_%d.bin",
        file_name, wifi_device_mem_dump_get_suffix(file_name));
    if (ret < 0) {
        oal_print_err("excute snprintf_s file[%s] failed, return [%d]\n", file_name, ret);
        return -OAL_FAIL;
    }
    return OAL_SUCC;
}

osal_s32 wifi_device_mem_dump(struct st_wifi_dump_mem_info *pst_mem_dump_info, osal_u32 count)
{
    os_kernel_file_stru *fp;
    osal_s32 ret = -OAL_FAIL;
    osal_u32 i;
    osal_char filename[FILE_NAME_LEN] = {0};

    ktime_t time_start, time_stop;
    oal_uint64 trans_us;
    osal_u8 *puc_data_buf = NULL;
    osal_u32 sdio_transfer_limit = 1024;
    sdio_transfer_limit = uapi_min(PAGE_SIZE * 2, sdio_transfer_limit); /* 2倍页大小 */

    oal_print_err("the read count [%u], mem dump data buf len is [%d]\n", count, sdio_transfer_limit);

    if (NULL == pst_mem_dump_info) {
        oal_print_err("pst_wifi_dump_info is NULL\n");
        return -OAL_FAIL;
    }

    puc_data_buf = (osal_u8 *)osal_kmalloc(sdio_transfer_limit, (GFP_KERNEL | __GFP_NOWARN));
    if (NULL == puc_data_buf) {
        oal_print_err("puc_data_buf KMALLOC failed\n");
        return -OAL_FAIL;
    }

    for (i = 0; i < count; i++) {
        time_start = ktime_get();
        /* 打开文件，准备接受wifi mem dump */
        if (wifi_device_mem_dump_get_filename(filename, FILE_NAME_LEN, pst_mem_dump_info[i].file_name) < 0) {
            break;
        }
        oal_print_info("readm %s\n", filename);

        fp = open_file_to_readm_etc(filename, O_RDWR | O_CREAT);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            oal_print_err("create file error,fp = 0x%p, filename is [%s]\n", fp, filename);
            break;
        }

        ret = hcc_read_device_mem_etc(&pst_mem_dump_info[i], fp, puc_data_buf, sdio_transfer_limit);
        if (ret < 0) {
            filp_close(fp, NULL);
            break;
        }
        oal_print_info("dfx_last_dump idx[%d], file_name[%s]\n", i, pst_mem_dump_info[i].file_name);
        if (wifi_device_mem_dump_dfx(&pst_mem_dump_info[i], fp) != OAL_SUCC) {
            oal_print_err("******wifi_device_mem_dump_dfx not ok!!\n");
        }
        filp_close(fp, NULL);
        time_stop = ktime_get();
        trans_us = (oal_uint64)ktime_to_us(ktime_sub(time_stop, time_start));
        oal_print_err("device get mem %s cost %llu us\n", filename, trans_us);
    }
    osal_kfree(puc_data_buf);

    return OAL_SUCC;
}


static osal_u8 g_panic_info_type = 0;
osal_void plat_set_panic_info_type(osal_u8 value)
{
    g_panic_info_type = value;
}

osal_void dump_panic_msg(osal_void)
{
    osal_u32 count;
    struct st_wifi_dump_mem_info *meminfo_head;
    if (g_panic_info_type == 0) {
        count = (osal_u32)(sizeof(g_wifi_meminfo) / sizeof(struct st_wifi_dump_mem_info));
        meminfo_head = g_wifi_meminfo;
    } else {
        count = (osal_u32)(sizeof(g_wifi_stack_info) / sizeof(struct st_wifi_dump_mem_info));
        meminfo_head = g_wifi_stack_info;
    }
    if (wifi_device_mem_dump(meminfo_head, count) != OAL_SUCC) {
        oal_print_err("******************wifi_device_mem_dump not ok!!\n");
    }
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
