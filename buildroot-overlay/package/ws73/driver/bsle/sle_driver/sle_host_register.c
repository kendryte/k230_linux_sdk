/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 */
#include <asm/unaligned.h>
#include "sle_dev.h"
#include "sle_tm_chba_proc.h"
#include "sle_hcc_proc.h"
#include "sle_host_register.h"

MODULE_LICENSE("GPL");

static uint8_t g_sle_state = SLE_OFF;
#define SLE_DFR_EVENT_DATA_LEN 7
static uint8_t sle_dfr_event[SLE_DFR_EVENT_DATA_LEN] = {0xa2, 0xaa, 0xfc, 0x02, 0x00, 0x00, 0x00};

static int32_t sle_recovery(void);
#if defined(CONFIG_PLAT_SUPPORT_DFR)
static int32_t sle_dfr_recovery(void);
#endif
static osal_void sle_stop(void);

void set_sle_state(uint8_t sle_state)
{
    printk(KERN_INFO"sle set sle state %u \r\n", sle_state);
    g_sle_state = sle_state;
}

uint8_t get_sle_state(void)
{
    return g_sle_state;
}

static void sle_send_dfr_event_to_host(void)
{
    printk(KERN_INFO"sle send dfr event to host enter\n");
    sle_dev_rx(sle_dfr_event, SLE_DFR_EVENT_DATA_LEN);
}

static int sle_hcc_rx_proc(uint8_t *buf, uint16_t len)
{
    if (g_sle_state == SLE_ON) {
        return sle_dev_rx(buf, len);
    } else {
        printk(KERN_INFO"sle hcc rx proc sle not open\r\n");
        return EOK;
    }
}

static int sle_dev_tx_proc(uint8_t *buf, uint16_t len)
{
    if (g_sle_state == SLE_ON) {
        return sle_hci_send_frame(buf, len);
    } else {
        printk(KERN_INFO"hcc tx proc sle not open\r\n");
        return EOK;
    }
}

static int32_t sle_host_init(void)
{
    int32_t ret;
    bool boot_finish;

    ret = pm_sle_open();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"pm_sle_open failed\n");
    } else {
        printk(KERN_INFO"pm_sle_open success\n");
    }

    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        printk(KERN_ERR"device boot not finish \n");
    }

    ret = sle_hcc_init();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"initial hcc sle service failed\n");
        return OAL_FAIL;
    }

    ret = sle_dev_init();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"sle dev init failed\n");
        return OAL_FAIL;
    }

    printk(KERN_INFO"sle device open success\n");

    sle_dev_tx_register(sle_dev_tx_proc); // 从 stack 发 数据到 btc
    sle_dev_rx_register(sle_hcc_rx_proc); // 从 btc 接收 数据 转发到 stack

    hci_kernel_init();
    // 注册suspend回调
    pm_sle_suspend_cb_host_register(sle_stop);
    // 注册resume回调
    pm_sle_resume_cb_host_register(sle_recovery);
#if defined(CONFIG_PLAT_SUPPORT_DFR)
    // 注册dfr回调
    plat_sle_exception_rst_register_etc(sle_dfr_recovery);
#endif
    g_sle_state = SLE_OFF;
    return OAL_SUCC;
}

static osal_void sle_host_deinit(void)
{
    int32_t ret;
    printk(KERN_INFO"enter:%s\n", __func__);
    pm_sle_suspend_cb_host_register(NULL);
    pm_sle_resume_cb_host_register(NULL);

    sle_dev_exit();

    ret = pm_sle_close();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"pm_sle_close failed\n");
    } else {
        printk(KERN_INFO"finish: pm_sle_close\n");
    }

    hbsle_hcc_customize_reset_device_status();
    sle_hcc_deinit();
    g_sle_state = SLE_OFF;
    sle_dev_tx_register(NULL);
    sle_dev_rx_register(NULL);
    printk(KERN_INFO"finish: H2D_MSG_SLE_CLOSE\n");
}

static osal_void sle_stop(void)
{
    g_sle_state = SLE_OFF;
}

static int32_t sle_recovery(void)
{
    int32_t ret;
    bool boot_finish;
    bool customize_received;
    printk(KERN_INFO"start sle recovery \n");
    /* 恢复HCC状态 */
    hcc_switch_status(HCC_ON);
    hbsle_hcc_customize_reset_device_status();
    /* 重新下发定制化配置 */
    ret = hbsle_hcc_custom_ini_data_buf();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"sle recovery::hcc ini data,fail ret=%d. \n", ret);
        return ret;
    }
    customize_received = hbsle_hcc_customize_get_device_status(BSLE_STATUS_CUSTOMIZE_RECEIVED);
    if (customize_received == false) {
        printk(KERN_ERR"customize data not received \n");
    }
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_SLE_OPEN, HCC_ACTION_TYPE_TEST);
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"sle recovery::ble/sle open,fail, ret=%d. \n", ret);
        return ret;
    }
    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        printk(KERN_ERR"device boot not finish \n");
    }
    g_sle_state = SLE_OFF;
    sle_send_dfr_event_to_host();
    return ret;
}

#if defined(CONFIG_PLAT_SUPPORT_DFR)
static int32_t sle_dfr_recovery(void)
{
    int32_t ret;
    printk(KERN_INFO"sle_dfr_recovery start \r\n");
    ret = sle_recovery();
    if (ret != OAL_SUCC) {
        printk(KERN_ERR"sle dfr recovery error\n");
    }
    return ret;
}
#endif

module_init(sle_host_init);
module_exit(sle_host_deinit);
EXPORT_SYMBOL(sle_tm_register_chba_recv_interface);
EXPORT_SYMBOL(sle_hci_register_chba_recv_interface); // chba 注册接收从btc侧发送的数据包
EXPORT_SYMBOL(sle_hci_recv_from_chba); // chba 发送数据包给btc侧
