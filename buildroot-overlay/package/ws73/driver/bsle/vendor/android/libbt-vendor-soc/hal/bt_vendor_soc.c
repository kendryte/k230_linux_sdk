/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: bt vendor source code
 * Author: AuthorNameMagicTag
 * Create: 2021-11-10
 */

/* Header File Including */
#include <fcntl.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <securec.h>
#include <securectype.h>
#include <unistd.h>

#include "bt_hci_bdroid.h"
#include "bt_vendor_soc.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "bt_vendor_soc"
#define BT_INFO    alog_info
#define BT_ERR   alog_error

typedef struct {
    bt_vendor_callbacks_t *cbacks;
    int uart_fd;
} bt_vendor_soc_stru;

bt_vendor_soc_stru g_vd = { NULL, -1 };

/* Copy from bt stack */
#define BD_ADDR_LEN                6    /* Device address length */

#define HCI_EVT_CMD_CMPL_STATUS_RET_BYTE 5
#define HCI_EVT_CMD_CMPL_OPCODE          3

unsigned char g_bd_addr[BD_ADDR_LEN] = {0};

/*
 * Define the preamble length for all HCI Commands.
 * This is 2-bytes for opcode and 1 byte for length
 */
#define HCIC_PREAMBLE_SIZE 3

#define BT_EVT_TO_LM_HCI_CMD       0x2000 /* HCI Command */

#define HCI_SET_BD_ADDR         0xFC32

#define UART_OPEN_RETRY_MAX_TIMES   3

/* Macros to get and put bytes to and from a stream (Little Endian format). */
#define uint16_to_stream(p, u16)           \
    do {                                   \
        *(p)++ = (uint8_t)(u16);             \
        *(p)++ = (uint8_t)((u16) >> 8);      \
    } while (0)
#define uint8_to_stream(p, u8) {    \
        *(p)++ = (uint8_t)(u8);       \
}
#define bdaddr_to_stream(p, a)                          \
    do {                                                \
        register int ijk;                               \
        for (ijk = 0; ijk < BD_ADDR_LEN; ijk++)         \
            *(p)++ = (uint8_t)(a)[BD_ADDR_LEN - 1 - ijk]; \
    } while (0)
#define stream_to_uint16(u16, p)                                      \
    do {                                                              \
        (u16) = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); \
        (p) += 2;                                                     \
    } while (0)

/* Function Definition */
static int hw_config_set_bdaddr(void);

static void int_cmd_cback(void *p_buf)
{
    if (p_buf == NULL || g_vd.cbacks == NULL || g_vd.cbacks->fwcfg_cb == NULL || g_vd.cbacks->dealloc == NULL) {
        BT_ERR("%s unexpected error", __FUNCTION__);
        return;
    }

    HC_BT_HDR *p_evt_buf = (HC_BT_HDR *)p_buf;
    uint8_t *p, status;
    uint16_t opcode;

    status = *((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_STATUS_RET_BYTE);
    p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_OPCODE;

    stream_to_uint16(opcode, p);
    BT_INFO("%s Opcode:0x%04X Status: %d", __FUNCTION__, opcode, status);
    if (opcode == HCI_SET_BD_ADDR) {
        /*
         * when uart_open() is sucessful, firmware download is happened
         * and, Since new stack expects fwcfg_cb we are returning SUCCESS
         */
        if (g_vd.cbacks->fwcfg_cb != NULL) {
            ALOGE("soc_op fwcfg_cb %s opcode: %d", __FUNCTION__, opcode);
            g_vd.cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);
        }
    } else {
        BT_ERR("%s Unknown Opcode:0x%04X Status: %d", __FUNCTION__, opcode, status);
    }
    g_vd.cbacks->dealloc(p_evt_buf);
}

static int set_bt_power(bt_vendor_power_state_t val)
{
    BT_INFO("set_bt_power: val=%d, ignore!", val);
    return 0;
}

/* Check if bluetooth address is null */
static int check_bdaddr(unsigned char *local_bdaddr)
{
    unsigned char index;

    /* Check if bluetooth address is null */
    for (index = 0; index < BD_ADDR_LEN; index++) {
        if (local_bdaddr[index] != 0) {
            return SUCCESS;
        }
    }

    BT_ERR("check_bdaddr failed, bdaddr invalid!");
    return FAILED;
}

/* Save bluetooth address when vendor init */
static void save_bdaddr(unsigned char *local_bdaddr)
{
    if ((local_bdaddr == NULL) || (check_bdaddr(local_bdaddr) == FAILED)) {
        return;
    }

    if (memcpy_s(g_bd_addr, BD_ADDR_LEN, local_bdaddr, BD_ADDR_LEN) != EOK) {
        BT_ERR("save addr failed");
        return;
    }
}

/* Set bluetooth address to device by send hci_command */
static int hw_config_set_bdaddr(void)
{
    HC_BT_HDR  *p_buf = NULL;
    unsigned char *pp = NULL;
    unsigned char retval;

    if (check_bdaddr(g_bd_addr) == FAILED) {
        BT_ERR("bluetooth address is null");
        return FAILED;
    }
    BT_INFO("%02x%02x%02x******", g_bd_addr[0], g_bd_addr[1], g_bd_addr[2]); // 02x format
    if (g_vd.cbacks == NULL || g_vd.cbacks->alloc == NULL || g_vd.cbacks->xmit_cb == NULL) {
        return FAILED;
    }
    p_buf = (HC_BT_HDR *)g_vd.cbacks->alloc(BT_HC_HDR_SIZE + HCIC_PREAMBLE_SIZE + BD_ADDR_LEN);
    if (p_buf == NULL) {
        return (FAILED);
    }
    pp = (uint8_t *)(p_buf + 1);
    p_buf->len = HCIC_PREAMBLE_SIZE + BD_ADDR_LEN;
    p_buf->offset = 0;
    p_buf->event = BT_EVT_TO_LM_HCI_CMD;
    uint16_to_stream(pp, HCI_SET_BD_ADDR);
    uint8_to_stream(pp, BD_ADDR_LEN);
    bdaddr_to_stream(pp, g_bd_addr);

    retval = g_vd.cbacks->xmit_cb(HCI_SET_BD_ADDR, p_buf, int_cmd_cback);
    return (retval ? SUCCESS : FAILED);
}

/*
 * 函 数 名  : uart_open()
 * 功能描述  : 打开节点
 */
static int uart_open(void)
{
    unsigned char try_index;
    ALOGI("bt uart_open");

    for (try_index = 0; try_index < UART_OPEN_RETRY_MAX_TIMES; try_index++) {
        g_vd.uart_fd = open(BT_UART_CTL, O_RDWR);
        if (g_vd.uart_fd < 0) {
            ALOGE("bt %s unable open %s, err: %d, retry times: %d", __FUNCTION__,
                  BT_UART_CTL, errno, try_index);
            /* wait device ready */
            if (try_index < (UART_OPEN_RETRY_MAX_TIMES - 1)) {
                usleep(1000000); // 1000000 ums
            }
        } else {
            break;
        }
    }

    ALOGI("bt uart_open exit");
    return (g_vd.uart_fd >= 0) ? SUCCESS : FAILED;
}

/*
 * 函 数 名  : uart_close()
 * 功能描述  : 关闭节点
 */
static int uart_close(void)
{
    ALOGI("bt uart_close");
    if (g_vd.uart_fd < 0) {
        ALOGE("bt %s uart_fd: %d", __FUNCTION__, g_vd.uart_fd);
        return FAILED;
    }

    close(g_vd.uart_fd);
    g_vd.uart_fd = -1;
    ALOGI("bt uart_close exit");
    return SUCCESS;
}

static int soc_power_ctrl_handler(void *param)
{
    if (param == NULL) {
        ALOGE("%s param is NULL", __FUNCTION__);
        return FAILED;
    }

    int *state = (int *)param;
    if (*state == BT_VND_PWR_OFF) {
        return set_bt_power(BT_VND_PWR_OFF);
    } else if (*state == BT_VND_PWR_ON) {
        return set_bt_power(BT_VND_PWR_ON);
    } else {
        return FAILED;
    }
}

static int soc_userial_open_handler(void *param)
{
    if (param == NULL) {
        ALOGE("%s param is NULL", __FUNCTION__);
        return FAILED;
    }
    int(*fd_array)[] = (int(*)[])param;
    int idx;
    if (uart_open() == SUCCESS) {
        for (idx = 0; idx < CH_MAX; idx++) {
            (*fd_array)[idx] = g_vd.uart_fd;
        }
        /* in the bluedroid, the result value 1 is sucessful */
        return 1;
    }

    return FAILED;
}

/* BLUETOOTH VENDOR INTERFACE LIBRARY FUNCTIONS */
/*
 * 函 数 名  : soc_init()
 * 功能描述  : BT VENDOR INTERFACE 初始化函数
 * 输出参数  : const bt_vendor_callbacks_t* p_cb: 给BT VENDOR使用的一组回调函数
 *             unsigned char *local_bdaddr: 本地蓝牙地址
 */
static int soc_init(const bt_vendor_callbacks_t *p_cb, unsigned char *local_bdaddr)
{
    BT_INFO("bt vendor init");
    if (p_cb == NULL) {
        BT_ERR("p_cb is null");
        return FAILED;
    }
    memset_s(&g_vd, sizeof(g_vd), 0, sizeof(g_vd));
    g_vd.uart_fd = -1;
    /* store reference to user callbacks */
    g_vd.cbacks = (bt_vendor_callbacks_t *)p_cb;
    /* Save bluetooth address when init */
    save_bdaddr(local_bdaddr);
    return SUCCESS;
}

/*
 * 函 数 名  : soc_op()
 * 功能描述  : BT VENDOR INTERFACE 消息处理函数
 * 输出参数  : bt_vendor_opcode_t opcode: 操作命令码
 *             void *param:               数据流
 */
static int soc_op(bt_vendor_opcode_t opcode, void *param)
{
    if (g_vd.cbacks == NULL) {
        ALOGE("%s g_vd.cbacks is NULL, ignore opcode: %d", __FUNCTION__, opcode);
        return FAILED;
    }
    ALOGE("soc_op %s opcode: %d", __FUNCTION__, opcode);
    switch (opcode) {
        case BT_VND_OP_POWER_CTRL: {
            return soc_power_ctrl_handler(param);
        }
        case BT_VND_OP_SCO_CFG: {
            /* dummy */
            return SUCCESS;
        }
        case BT_VND_OP_USERIAL_OPEN: {
            return soc_userial_open_handler(param);
        }
        case BT_VND_OP_USERIAL_CLOSE: {
            return uart_close();
        }
        case BT_VND_OP_FW_CFG: {
            g_vd.cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);
            return SUCCESS;
        }
        case BT_VND_OP_LPM_SET_MODE: {
            if (g_vd.cbacks->lpm_cb != NULL) {
                g_vd.cbacks->lpm_cb(BT_VND_OP_RESULT_SUCCESS);
            }
            return SUCCESS;
        }
        case BT_VND_OP_EPILOG: {
            if (g_vd.cbacks->epilog_cb != NULL) {
                g_vd.cbacks->epilog_cb(BT_VND_OP_RESULT_SUCCESS);
            }
            return SUCCESS;
        }
        default: {
            BT_ERR("%s Unhandled opcode: %d", __FUNCTION__, opcode);
            return FAILED;
        }
    }
}

/*
 * 函 数 名  : soc_cleanup()
 * 功能描述  : BT VENDOR INTERFACE 关闭接口函数
 */
static void soc_cleanup(void)
{
    BT_INFO("cleanup");
    (void)memset_s(&g_vd, sizeof(g_vd), 0, sizeof(g_vd));
}

__attribute__((visibility("default")))
const bt_vendor_interface_t BLUETOOTH_VENDOR_LIB_INTERFACE = {
    sizeof(bt_vendor_interface_t),
    soc_init,
    soc_op,
    soc_cleanup
};