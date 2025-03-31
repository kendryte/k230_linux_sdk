/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include "bt_dev.h"
#include "ble_hcc.h"
#include "ble_host.h"

#if defined(CONFIG_PLAT_SUPPORT_DFR)
extern osal_s32 plat_bt_exception_rst_register_etc(osal_void *data);
#endif

#define BT_VENDOR_EVENT_LEN 4

static oal_uint8 g_ble_state = BLE_OFF;

static uint8_t bt_vendor_event_reset[BT_VENDOR_EVENT_LEN] = { 0x04, 0xff, 0x01, 0xc7 };

void set_ble_state(bool ble_state)
{
    hcc_debug("ble set ble state ble_state %d \r\n", ble_state);
    g_ble_state = ble_state;
}

uint8_t get_ble_state(void)
{
    return g_ble_state;
}

static int hcc_rx_proc(uint8_t *buf, uint32_t len)
{
    if (g_ble_state == BLE_ON) {
        return bt_dev_rx(buf, len);
    } else {
        hcc_debug("hcc rx proc ble not open\r\n");
        return EOK;
    }
}

static int dev_tx_proc(uint8_t *buf, uint16_t len)
{
    if (g_ble_state == BLE_ON) {
        return ble_hci_send_frame(buf, len, NULL);
    } else {
        hcc_debug("hcc tx proc ble not open\r\n");
        return EOK;
    }
}

static oal_int32 send_reset_event(void)
{
    return bt_dev_rx(bt_vendor_event_reset, BT_VENDOR_EVENT_LEN);
}

static oal_int32 ble_recovery(void)
{
    oal_int32 ret;
    bool customize_received;
    bool boot_finish;
    hcc_debug("start ble recovery \n");
    /* 恢复HCC状态 */
    hcc_switch_status(HCC_ON);
    hbsle_hcc_customize_reset_device_status();
    /* 重新下发定制化配置 */
    ret = hbsle_hcc_custom_ini_data_buf();
    if (ret != OAL_SUCC) {
        printk("bluetooth_dfr_recovery::hcc ini data,fail ret=%d. \n", ret);
        return ret;
    }
    customize_received = hbsle_hcc_customize_get_device_status(BSLE_STATUS_CUSTOMIZE_RECEIVED);
    if (customize_received == false) {
        printk("customize data not received \n");
    }
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_BT_OPEN, HCC_ACTION_TYPE_TEST);
    if (ret != OAL_SUCC) {
        printk("bluetooth_dfr_recovery::ble open,fail, ret=%d. \n", ret);
        return ret;
    }
    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        hcc_debug("device boot not finish \n");
    }
    send_reset_event();
    g_ble_state = BLE_OFF;
    return ret;
}

#if defined(CONFIG_PLAT_SUPPORT_DFR)
static void ble_dfr_recovery(void)
{
    oal_int32 ret = ble_recovery();
    if (ret != OAL_SUCC) {
        oal_print_err("ble_recovery error\n");
    }
}
#endif

static void ble_stop(void)
{
    g_ble_state = BLE_OFF;
}

static int ble_init(void)
{
    int ret;
    bool boot_finish;
    ret = pm_ble_open();
    if (ret != OAL_SUCC) {
        hcc_debug("pm_ble_open failed\n");
        return ret;
    }
    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        hcc_debug("device boot not finish \n");
    }

    ret = ble_hcc_init();
    if (ret != OAL_SUCC) {
        hcc_debug("ble_hcc_init failed\n");
        return ret;
    }
    ret = bt_dev_init();
    if (ret != OAL_SUCC) {
        hcc_debug("bt_dev_init failed\n");
        return ret;
    }
    ble_hcc_rx_register(hcc_rx_proc);
    bt_dev_tx_register(dev_tx_proc);

    /* 注册蓝牙休眠唤醒回调 */
    pm_ble_suspend_cb_host_register(ble_stop);
    pm_ble_resume_cb_host_register(ble_recovery);
#if defined(CONFIG_PLAT_SUPPORT_DFR)
    // 注册dfr回调
    plat_bt_exception_rst_register_etc(ble_dfr_recovery);
#endif
    g_ble_state = BLE_OFF;
    return ret;
}

static void ble_exit(void)
{
    int ret;

    /* 去注册蓝牙休眠唤醒回调 */
    pm_ble_suspend_cb_host_register(NULL);
    pm_ble_resume_cb_host_register(NULL);

    bt_dev_exit();
    ret = pm_ble_close();
    if (ret != OAL_SUCC) {
        hcc_debug("pm_ble_close failed\n");
    }
    hbsle_hcc_customize_reset_device_status();
    ret = ble_hcc_deinit();
    if (ret != OAL_SUCC) {
        hcc_debug("ble_hcc_deinit failed\n");
    }
    g_ble_state = BLE_OFF;
}

module_init(ble_init);
module_exit(ble_exit);

MODULE_LICENSE("GPL");