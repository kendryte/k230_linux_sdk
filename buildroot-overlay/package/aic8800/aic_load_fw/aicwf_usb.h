/**
 * aicwf_usb.h
 *
 * USB function declarations
 *
 * Copyright (C) AICSemi 2018-2020
 */

#ifndef _AICWF_USB_H_
#define _AICWF_USB_H_

#include <linux/usb.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/ieee80211.h>
#include <linux/semaphore.h>

#include "aicbluetooth_cmds.h"

#ifdef AICWF_USB_SUPPORT

/* USB Device ID */
#define USB_VENDOR_ID_AIC               0xA69C
#define USB_VENDOR_ID_AIC_V2            0x368B
#define USB_DEVICE_ID_AIC               0x8800
#define USB_DEVICE_ID_AIC_8801          0x8801

#define CHIP_REV_U01        0x1
#define CHIP_REV_U02        0x3
#define CHIP_REV_U03        0x7
#define CHIP_REV_U04        0xf
#define CHIP_REV_U05        0x1f
#define CHIP_SUB_REV_U04    0x20

enum AICWF_IC{
    PRODUCT_ID_AIC8800 =   0,
    PRODUCT_ID_AIC8801,
    PRODUCT_ID_AIC8800DC,
    PRODUCT_ID_AIC8800DW,
    PRODUCT_ID_AIC8800D80,
    PRODUCT_ID_AIC8800D81,
    PRODUCT_ID_AIC8800D80X2,
    PRODUCT_ID_AIC8800D81X2,
    PRODUCT_ID_AIC8800D89X2,
};


#define AICWF_USB_RX_URBS               (5)
#define AICWF_USB_TX_URBS               (100)
#define AICWF_USB_TX_LOW_WATER          (AICWF_USB_TX_URBS/4)
#define AICWF_USB_TX_HIGH_WATER         (AICWF_USB_TX_LOW_WATER*3)
#define AICWF_USB_MAX_PKT_SIZE          (2048)

#ifdef CONFIG_RFTEST
#define FW_RF_PATCH_BASE_NAME           "fw_patch.bin"
#define FW_RF_ADID_BASE_NAME            "fw_adid.bin"
#define FW_RF_BASE_NAME                 "fmacfw_rf.bin"
#define FW_PATCH_BASE_NAME              "fw_patch.bin"
#define FW_ADID_BASE_NAME               "fw_adid.bin"
#define FW_BASE_NAME                    "fmacfw.bin"
#define FW_BLE_SCAN_WAKEUP_NAME         "fw_ble_scan.bin"
#define FW_BLE_SCAN_AD_FILTER_NAME      "fw_ble_scan_ad_filter.bin"

#define FW_PATCH_BASE_NAME_PC           "fw_patch.bin"
#define FW_ADID_BASE_NAME_PC            "fw_adid.bin"
#define FW_BASE_NAME_PC                 "fmacfw.bin"
#define FW_RF_PATCH_BASE_NAME_PC        "fw_patch.bin"
#define FW_RF_ADID_BASE_NAME_PC         "fw_adid.bin"
#define FW_RF_BASE_NAME_PC              "fmacfw_rf.bin"
#define FW_PATCH_TEST_BASE_NAME      	"fw_patch_test.bin"
#define FW_PATCH_BASE_NAME_U03          "fw_patch_u03.bin"
#define FW_ADID_BASE_NAME_U03           "fw_adid_u03.bin"

#else
#define FW_PATCH_BASE_NAME              "fw_patch.bin"
#define FW_ADID_BASE_NAME               "fw_adid.bin"
#define FW_BASE_NAME                    "fmacfw.bin"
#endif

#define FW_PATCH_TABLE_NAME             "fw_patch_table.bin"
#define FW_PATCH_TABLE_NAME_U03         "fw_patch_table_u03.bin"
#define FW_USERCONFIG_NAME              "aic_userconfig.txt"
#define FW_M2D_OTA_NAME                 "m2d_ota.bin"

   /*8800 use 0x100000, 8800D80 use 0x160000*/
#define RAM_FW_BLE_SCAN_WAKEUP_ADDR_8800D80		0x00160000
#define RAM_FW_BLE_SCAN_WAKEUP_ADDR		0x00100000
#define RAM_FW_ADDR                     0x00110000
#define FW_RAM_ADID_BASE_ADDR           0x00161928
#define FW_RAM_PATCH_BASE_ADDR          0x00100000
#define FW_RAM_PATCH_BASE_ADDR_U03      0x00100000
#define FW_PATCH_TEST_BASE_ADDR         0x00100000

enum {
    FW_NORMAL_MODE,
    FW_TEST_MODE,
    FW_BLE_SCAN_WAKEUP_MODE,
    FW_M2D_OTA_MODE,
    FW_DPDCALIB_MODE,
    FW_BLE_SCAN_AD_FILTER_MODE,
};


#if 0
#define FW_NAME2                    FW_BASE_NAME".bin"
#define FW_PATCH_BIN_NAME           FW_PATCH_BASE_NAME".bin"
#define FW_ADID_BIN_NAME            FW_ADID_BASE_NAME".bin"
#endif

typedef enum {
    USB_TYPE_DATA         = 0X00,
    USB_TYPE_CFG          = 0X10,
    USB_TYPE_CFG_CMD_RSP  = 0X11,
    USB_TYPE_CFG_DATA_CFM = 0X12
} usb_type;

enum aicwf_usb_state {
    USB_DOWN_ST,
    USB_UP_ST,
    USB_SLEEP_ST
};

#define MAX_AD_FILTER_NUM        5// Max AD Filter num
#define MAX_GPIO_TRIGGER_NUM     2// Max user config num of gpio
#define MAX_ROLE_COMNO_IDX_NUM   2// Max num of ad role type combo,form( enum gpio_combo_idx) 

#define AD_ROLE_FLAG         0x0f
#define ROLE_COMBO_IDX_FLAG  0xf0

enum ad_role_type {
    ROLE_ONLY,// ROLE_ONLY will trigger wake up immediately.
    ROLE_COMBO,//ROLE_COMBO will trigger When all the conditions (ad_role == ROLE_COMBO,and ad_filter is matching)are met.
};

enum gpio_combo_idx {
    COMBO_0,
    COMBO_1,
};

enum gpio_trigger_bit {
    TG_IDX_0 = (1<<0),
    TG_IDX_1 = (1<<1),
};

struct wakeup_ad_data_filter {
    uint32_t ad_data_mask;
    uint8_t gpio_trigger_idx;
    uint8_t ad_role;//from enum ad_role_type 
    uint8_t ad_len;
    uint8_t ad_type;
    uint8_t ad_data[31];
};

struct ble_wakeup_param_t {
    uint32_t magic_num;// "BLES" = 0x53454C42
    uint32_t delay_scan_to;// timeout for start scan in ms
    uint32_t reboot_to;// timeout for reboot in ms
    uint32_t gpio_num[MAX_GPIO_TRIGGER_NUM];
    uint32_t gpio_dft_lvl[MAX_GPIO_TRIGGER_NUM];
    struct wakeup_ad_data_filter ad_filter[MAX_AD_FILTER_NUM];
};

struct aicwf_usb_buf {
    struct list_head list;
    struct aic_usb_dev *usbdev;
    struct urb *urb;
    struct sk_buff *skb;
    #ifdef CONFIG_USB_NO_TRANS_DMA_MAP
    u8 *data_buf;
    dma_addr_t data_dma_trans_addr;
    #endif
    bool cfm;
};

struct aic_usb_dev {
    struct rwnx_cmd_mgr cmd_mgr;
    struct aicwf_bus *bus_if;
    struct usb_device *udev;
    struct device *dev;
    struct aicwf_rx_priv* rx_priv;
    enum aicwf_usb_state state;

    struct usb_anchor rx_submitted;
    struct work_struct rx_urb_work;

    spinlock_t rx_free_lock;
    spinlock_t tx_free_lock;
    spinlock_t tx_post_lock;
    spinlock_t tx_flow_lock;

    struct list_head rx_free_list;
    struct list_head tx_free_list;
    struct list_head tx_post_list;

    uint bulk_in_pipe;
    uint bulk_out_pipe;
#ifdef CONFIG_USB_MSG_EP
	uint msg_out_pipe;
	uint use_msg_ep;
#endif

    int tx_free_count;
    int tx_post_count;

    struct aicwf_usb_buf usb_tx_buf[AICWF_USB_TX_URBS];
    struct aicwf_usb_buf usb_rx_buf[AICWF_USB_RX_URBS];

    int msg_finished;
    wait_queue_head_t msg_wait;
    ulong msg_busy;
    struct urb *msg_out_urb;
    #ifdef CONFIG_USB_NO_TRANS_DMA_MAP
    dma_addr_t cmd_dma_trans_addr;
    #endif
    
    u16 chipid;
    bool tbusy;
    bool app_cmp;
    u32 fw_version_uint;
};

extern void aicwf_usb_exit(void);
extern void aicwf_usb_register(void);
extern void aicwf_usb_tx_flowctrl(struct aic_usb_dev *usb_dev, bool state);
int usb_bustx_thread(void *data);
int usb_busrx_thread(void *data);
int aicwf_process_rxframes(struct aicwf_rx_priv *rx_priv);

#endif /* AICWF_USB_SUPPORT */
#endif /* _AICWF_USB_H_       */
