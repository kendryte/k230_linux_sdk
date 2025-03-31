/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Sending patch to device
 * Author: @CompanyNameTag
 */

#ifdef CONFIG_HCC_SUPPORT_UART

#include "osal_types.h"
#include "oal_kernel_file.h"
#include "plat_pm.h"
#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
#include "plat_sha256_calc.h"
#endif
#include "hcc_uart_host.h"
#include "plat_firmware_uart.h"
#include "customize_wifi.h"

#define UART_DOWNLOAD_ADDR_INFO_LEN    10
#define UART_DOWNLOAD_FILE_PATH_LEN    50

#define UART_BAUD_RATE_REG 0x40019410
#define UART_BAUD_RATE_REG_WIDTH 4

typedef struct {
    char addr[UART_DOWNLOAD_ADDR_INFO_LEN];
    char file_path[UART_DOWNLOAD_FILE_PATH_LEN];
} uart_download_stru;

uart_download_stru uart_download_info[] = {
    { "0x400000", CONFIG_FIRMWARE_BIN_PATH },
    { "0x430000", CONFIG_FIRMWARE_WIFICALI_PATH },
    { "0x440000", CONFIG_FIRMWARE_BSLECALI_PATH },
};

xmodem_pkt_t *g_pkt_buf = OSAL_NULL;

static unsigned short g_crc_table[CRC_TABLE_SIZE] = {
    /* CRC 余式表 */
    0X0000, 0X1021, 0X2042, 0X3063, 0X4084, 0X50A5, 0X60C6, 0X70E7,
    0X8108, 0X9129, 0XA14A, 0XB16B, 0XC18C, 0XD1AD, 0XE1CE, 0XF1EF,
    0X1231, 0X0210, 0X3273, 0X2252, 0X52B5, 0X4294, 0X72F7, 0X62D6,
    0X9339, 0X8318, 0XB37B, 0XA35A, 0XD3BD, 0XC39C, 0XF3FF, 0XE3DE,
    0X2462, 0X3443, 0X0420, 0X1401, 0X64E6, 0X74C7, 0X44A4, 0X5485,
    0XA56A, 0XB54B, 0X8528, 0X9509, 0XE5EE, 0XF5CF, 0XC5AC, 0XD58D,
    0X3653, 0X2672, 0X1611, 0X0630, 0X76D7, 0X66F6, 0X5695, 0X46B4,
    0XB75B, 0XA77A, 0X9719, 0X8738, 0XF7DF, 0XE7FE, 0XD79D, 0XC7BC,
    0X48C4, 0X58E5, 0X6886, 0X78A7, 0X0840, 0X1861, 0X2802, 0X3823,
    0XC9CC, 0XD9ED, 0XE98E, 0XF9AF, 0X8948, 0X9969, 0XA90A, 0XB92B,
    0X5AF5, 0X4AD4, 0X7AB7, 0X6A96, 0X1A71, 0X0A50, 0X3A33, 0X2A12,
    0XDBFD, 0XCBDC, 0XFBBF, 0XEB9E, 0X9B79, 0X8B58, 0XBB3B, 0XAB1A,
    0X6CA6, 0X7C87, 0X4CE4, 0X5CC5, 0X2C22, 0X3C03, 0X0C60, 0X1C41,
    0XEDAE, 0XFD8F, 0XCDEC, 0XDDCD, 0XAD2A, 0XBD0B, 0X8D68, 0X9D49,
    0X7E97, 0X6EB6, 0X5ED5, 0X4EF4, 0X3E13, 0X2E32, 0X1E51, 0X0E70,
    0XFF9F, 0XEFBE, 0XDFDD, 0XCFFC, 0XBF1B, 0XAF3A, 0X9F59, 0X8F78,
    0X9188, 0X81A9, 0XB1CA, 0XA1EB, 0XD10C, 0XC12D, 0XF14E, 0XE16F,
    0X1080, 0X00A1, 0X30C2, 0X20E3, 0X5004, 0X4025, 0X7046, 0X6067,
    0X83B9, 0X9398, 0XA3FB, 0XB3DA, 0XC33D, 0XD31C, 0XE37F, 0XF35E,
    0X02B1, 0X1290, 0X22F3, 0X32D2, 0X4235, 0X5214, 0X6277, 0X7256,
    0XB5EA, 0XA5CB, 0X95A8, 0X8589, 0XF56E, 0XE54F, 0XD52C, 0XC50D,
    0X34E2, 0X24C3, 0X14A0, 0X0481, 0X7466, 0X6447, 0X5424, 0X4405,
    0XA7DB, 0XB7FA, 0X8799, 0X97B8, 0XE75F, 0XF77E, 0XC71D, 0XD73C,
    0X26D3, 0X36F2, 0X0691, 0X16B0, 0X6657, 0X7676, 0X4615, 0X5634,
    0XD94C, 0XC96D, 0XF90E, 0XE92F, 0X99C8, 0X89E9, 0XB98A, 0XA9AB,
    0X5844, 0X4865, 0X7806, 0X6827, 0X18C0, 0X08E1, 0X3882, 0X28A3,
    0XCB7D, 0XDB5C, 0XEB3F, 0XFB1E, 0X8BF9, 0X9BD8, 0XABBB, 0XBB9A,
    0X4A75, 0X5A54, 0X6A37, 0X7A16, 0X0AF1, 0X1AD0, 0X2AB3, 0X3A92,
    0XFD2E, 0XED0F, 0XDD6C, 0XCD4D, 0XBDAA, 0XAD8B, 0X9DE8, 0X8DC9,
    0X7C26, 0X6C07, 0X5C64, 0X4C45, 0X3CA2, 0X2C83, 0X1CE0, 0X0CC1,
    0XEF1F, 0XFF3E, 0XCF5D, 0XDF7C, 0XAF9B, 0XBFBA, 0X8FD9, 0X9FF8,
    0X6E17, 0X7E36, 0X4E55, 0X5E74, 0X2E93, 0X3EB2, 0X0ED1, 0X1EF0
};

osal_s32 uart_send_data(char *data, osal_u32 len)
{
    return ps_write_tty(data, len, TD_FALSE);
}

osal_s32 uart_recv_data(char *data, osal_u32 recv_len)
{
    return ps_read_tty(data, recv_len);
}

osal_s32 uart_send_char(char ch)
{
    return ps_write_tty(&ch, 1, TD_FALSE);
}

osal_s32 uart_recv_char(char *ch)
{
    return ps_read_tty(ch, 1);
}

osal_s32 uart_recv_data_expect(char expect)
{
    static char recv_char;
    osal_s32 ret;
    osal_s32 i;

    ret = uart_recv_char(&recv_char);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        if (ret < 0) {
            oal_print_err("uart_recv_data_expect: recv fail\r\n");
            ret = EXT_ERR_FAILURE;
            break;
        }
        if (recv_char == expect) {
            ret = EXT_ERR_SUCCESS;
            break;
        } else if ((recv_char == NAK) || (recv_char == MSG_FORM_DRV_N) || (recv_char == MSG_FORM_DRV_C)) {
            oal_print_err("send again [0x%x]\r\n", recv_char);
            ret = EXT_ERR_FAILURE;
            break;
        } else {
            uart_send_char(MSG_FORM_DRV_N);
            oal_print_err("error result [0x%x], expect [0x%x], read result again\r\n", recv_char, expect);
        }
    }

    return ret;
}

osal_s32 uart_get_firmware_version(void)
{
    char send_data[RECV_DATA_MAX_LEN];
    char recv_data[RECV_DATA_MAX_LEN];
    osal_s32 ret;
    ret = snprintf_s(send_data, sizeof(send_data), (sizeof(send_data) - 1), "%s%c",
                     VER_CMD_KEYWORD, COMPART_KEYWORD);
    if (ret < EXT_ERR_SUCCESS) {
        return ret;
    }
    ret = uart_send_data(send_data, strlen(send_data));
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("uart_get_firmware_version:uart_send_data fail\r\n");
        return ret;
    }

    ret = uart_recv_data(recv_data, strlen(VER_EXPECT_VALUE));
    if (ret < 0) {
        oal_print_err("uart_get_firmware_version:uart_recv_data fail, ret %d\r\n", ret);
        return ret;
    }
    return memcmp(recv_data, VER_EXPECT_VALUE, strlen(VER_EXPECT_VALUE));
}

static osal_u16 calc_crc(osal_u8 *data, osal_u16 length)
{
    osal_u16 crc = 0;
    osal_u8 data_tmp;
    while (length > 0) {
        data_tmp = (osal_u8)(crc >> 8); /* crc右移8位 */
        crc = crc << 8; /* crc左移8位 */
        crc = crc ^ g_crc_table[data_tmp ^ (*data)];
        length--;
        data++;
    }
    return crc;
}

static osal_s32 file_xmodem_send_pkt(osal_u8 pkt_num, xmodem_pkt_t *pkt, osal_s32 len)
{
    osal_s32 ret;
    osal_s32 i;
    osal_u16 crc_value;

    if (pkt == NULL) {
        oal_print_err("pkt null.\n");
        return EXT_ERR_FAILURE;
    }

    pkt->header.sch = SOH;
    pkt->header.num = pkt_num;
    pkt->header.ant = ~pkt_num;

    // 不满足xmode发送长度，后面补0
    if (len < XMODEM_DATA_LEN) {
        oal_print_err("pkt_len %d\n, fill up to 1KB.", len);
        memset_s(&(pkt->data[len]), (XMODEM_DATA_LEN - len), 0, (XMODEM_DATA_LEN - len));
    }

    crc_value = calc_crc((osal_u8 *)pkt->data, XMODEM_DATA_LEN);
    pkt->crc.h = (crc_value & 0xFF00) >> 8; /* 8: high 8 bits */
    pkt->crc.l = crc_value & 0xFF;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        ret = uart_send_data((osal_char *)pkt, sizeof(xmodem_pkt_t));
        if (ret != 0) {
            oal_print_err("file_xmodem_send_pkt: uart send fail [0x%x]\r\n", ret);
            return ret;
        }

        if (uart_recv_data_expect(ACK) == EXT_ERR_SUCCESS) {
            return EXT_ERR_SUCCESS;
        } else {
            continue;
        }
    }

    return EXT_ERR_FAILURE;
}

static osal_s32 do_send_file_data(osal_void *fp, osal_s32 len)
{
    osal_s32 ret = EXT_ERR_FAILURE;
    osal_s32 remain_len = len;
    osal_s32 send_len;
    osal_u8 pkt_num = 1;

    while (1) {
        send_len = osal_klib_fread(g_pkt_buf->data, XMODEM_DATA_LEN, fp);
        if ((send_len > 0) && (send_len <= XMODEM_DATA_LEN)) {
            ret = file_xmodem_send_pkt(pkt_num++, g_pkt_buf, send_len);
            if (ret != EXT_ERR_SUCCESS) {
                oal_print_err(" uart send data[0x%x]:%d fail\r\n", len - remain_len, send_len);
                break;
            }
            remain_len = remain_len - send_len;
        } else if (send_len == 0) { /* 文件已经读取完成 */
            if (uart_send_char(EOT) != 0) {
                oal_print_err("send EOT fail\r\n");
            }

            ret = uart_recv_data_expect(ACK);
            if (ret != EXT_ERR_SUCCESS) {
                oal_print_err("wait ACK fail\r\n");
            }
            ret = uart_recv_data_expect(MSG_FORM_DRV_G);
            if (ret != EXT_ERR_SUCCESS) {
                oal_print_err("wait G fail\r\n");
            }

            break;
        } else { // read file err
            if (uart_send_char(CAN) != EXT_ERR_SUCCESS) {
                oal_print_err("send CAN fail\r\n");
            }
            oal_print_err("read file[0x%x] err\n", len - remain_len);
            break;
        }
    }

    return EXT_ERR_SUCCESS;
}

static osal_s32 uart_send_file_data(char *path)
{
    osal_s32 ret = EXT_ERR_FAILURE;
    osal_s32 len;
    osal_void *fp = OSAL_NULL;

    fp = osal_klib_fopen(path, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        oal_print_err("filp_open [%s] fail!!, fp=%p\n", path, fp);
        return -OAL_FAIL;
    }

    /* 获取file文件大小 */
    len = osal_klib_fseek(0, SEEK_END, fp);
#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
    if (len <= SHA256_HEADER_LENGTH) {
#else
    if (len <= 0) {
#endif
        oal_print_err("file size of %s is %d, err!!\n", path, len);
        osal_klib_fclose(fp);
        return -OAL_FAIL;
    }

    /* 定位到文件实际内容开头 */
    len = len - SHA256_HEADER_LENGTH; // skip bin's header area
    osal_klib_fseek(SHA256_HEADER_LENGTH, SEEK_SET, fp);

    ret = do_send_file_data(fp, len);
    osal_klib_fclose(fp);
    return ret;
}

osal_s32 uart_download_file(osal_char *addr, osal_char *path)
{
    osal_char send_data[RECV_DATA_MAX_LEN] = {0};
    osal_s32 ret;

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
    if (firmware_sha256_checksum(path) != OAL_SUCC) {
        oal_print_err("[%s] sha256sum check fail!!\n", path);
        return -OAL_FAIL;
    }
    oal_print_err("[%s] sha256sum check succ!!\n", path);
#endif

    ret = snprintf_s(send_data, sizeof(send_data), (sizeof(send_data) - 1), "%s%c%s%c",
                     FILES_CMD_KEYWORD, COMPART_KEYWORD, addr, COMPART_KEYWORD);
    if (ret < 0) {
        oal_print_err("uart_download_file: snprintf_s fail [0x%x]\r\n", ret);
        return ret;
    }

    // 发送命令和地址
    ret = uart_send_data(send_data, strlen(send_data));
    if (ret != 0) {
        oal_print_err("uart_download_file: send cmd fail [0x%x]\r\n", ret);
        return ret;
    }
    ret = uart_recv_data_expect(MSG_FORM_DRV_C);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("wait C fail\r\n");
        return ret;
    }

    // 发送文件数据
    ret = uart_send_file_data(path);
    if (ret != 0) {
        oal_print_err("send file data fail\r\n");
        return ret;
    }
    return ret;
}

osal_s32 uart_send_quit(void)
{
    char send_data[RECV_DATA_MAX_LEN];
    osal_s32 ret;
    ret = snprintf_s(send_data, sizeof(send_data), (sizeof(send_data) - 1), "%s%c",
                     QUIT_CMD_KEYWORD, COMPART_KEYWORD);
    if (ret < 0) {
        oal_print_err("uart_send_quit: snprintf_s fail [0x%x]\r\n", ret);
        return ret;
    }
    // 发送命令
    ret = uart_send_data(send_data, strlen(send_data));
    if (ret != 0) {
        oal_print_err("uart_send_quit: send cmd fail [0x%x]\r\n", ret);
        return ret;
    }
    return 0;
}

osal_s32 pre_download_writem(osal_void)
{
    osal_u32 cmu_xo_trim_param;
#ifdef CONFIG_NO_SUPPORT_INI
    cmu_xo_trim_param = CONFIG_INI_CMU_XO_TRIM;
#else
    cmu_xo_trim_param = ini_get_cmu_xo_trim();
#endif

    return uart_cmd_write_mem(WR_CMU_XO_TRIM_LEN, WR_CMU_XO_TRIM_ADDR, cmu_xo_trim_param);
}

osal_void uart_check_rx_data_empty(osal_void)
{
    char rx_data;
    while (uart_recv_char(&rx_data) > 0) {
        oal_print_err("RX:[%02x]\r\n", rx_data);
    }
}

osal_s32 uart_download_firmware(osal_void) // firmware_download_etc
{
    osal_u32 i;
    osal_u32 count;

    oal_print_err("g_pkt_buf[%d].\n", (osal_s32)sizeof(xmodem_pkt_t));
    uart_check_rx_data_empty();

    // malloc pkt buffer
    g_pkt_buf = (xmodem_pkt_t *)osal_kmalloc(sizeof(xmodem_pkt_t), OSAL_GFP_KERNEL);
    if (g_pkt_buf == OSAL_NULL) {
        oal_print_err("alloc g_pkt_buf[%d] failed.\n", (osal_s32)sizeof(xmodem_pkt_t));
        return EXT_ERR_FAILURE;
    }
    memset_s((void *)g_pkt_buf, sizeof(xmodem_pkt_t), 0, sizeof(xmodem_pkt_t));

    // version check
    if (uart_get_firmware_version() == 0) {
        oal_print_err("uart_get_firmware_version return ok\r\n");
    } else {
        oal_print_err("uart_get_firmware_version return fail\r\n");
        goto fail_one;
    }

    // writem
    if (pre_download_writem() != 0) {
        oal_print_err("uart writem return fail\r\n");
    }

    // writem 配置uart波特率
    if (uart_cmd_write_mem(UART_BAUD_RATE_REG_WIDTH, UART_BAUD_RATE_REG, HCC_UART_FW_BAUDRATE) != 0) {
        oal_print_emerg("set device baudrate fail!\r\n");
        goto fail_one;
    }
    // download file
    count = sizeof(uart_download_info) / sizeof(uart_download_stru);
    for (i = 0; i < count; i++) {
        if (uart_download_file(uart_download_info[i].addr, uart_download_info[i].file_path) == 0) {
            oal_print_err("uart_download_file succ: file_path = %s\r\n", uart_download_info[i].file_path);
        } else {
            oal_print_err("uart_download_file fail: file_path = %s\r\n", uart_download_info[i].file_path);
            goto fail_one;
        }
    }

    // quit -> enter firmware
    if (uart_send_quit() == 0) {
        oal_print_err("uart_send_quit return ok\r\n");
    } else {
        oal_print_err("uart_send_quit return fail\r\n");
    }

    osal_kfree(g_pkt_buf);
    return EXT_ERR_SUCCESS;

fail_one:
    osal_kfree(g_pkt_buf);

    return EXT_ERR_FAILURE;
}

osal_s32 uart_cmd_read_mem(osal_u32 addr, osal_u32 len)
{
    osal_s32 ret;
    osal_u32 i;
    osal_u8 recv_data;
    osal_char send_data[SEND_DATA_MAX_LEN];

    ret = snprintf_s(send_data, sizeof(send_data), (sizeof(send_data) - 1), "%s%c0x%x%c0x%x%c",
                     READM_CMD_KEYWORD, COMPART_KEYWORD, addr, COMPART_KEYWORD, len, COMPART_KEYWORD);
    if (ret < 0) {
        oal_print_err("uart_cmd_read_mem: snprintf_s fail [0x%x]\r\n", ret);
        return ret;
    }

    // 发送命令
    ret = uart_send_data(send_data, strlen(send_data));
    if (ret != 0) {
        oal_print_err("uart_cmd_read_mem: send cmd fail [0x%x]\r\n", ret);
        return ret;
    }

    for (i = 0; i < len; i++) {
        uart_recv_char(&recv_data);
        oal_print_err("recv_data[%u]: 0x%x", i, recv_data);
    }

    return 0;
}

osal_s32 uart_cmd_write_mem(osal_u32 width, osal_u32 addr, osal_u32 val)
{
    osal_char send_data[SEND_DATA_MAX_LEN];
    osal_s32 ret;

    ret = snprintf_s(send_data, sizeof(send_data), (sizeof(send_data) - 1), "%s%c0x%x%c0x%x%c0x%x%c",
                     WRITEM_CMD_KEYWORD, COMPART_KEYWORD, width, COMPART_KEYWORD, addr,
                     COMPART_KEYWORD, val, COMPART_KEYWORD);
    if (ret < 0) {
        oal_print_err("uart_cmd_write_mem: snprintf_s fail [0x%x]\r\n", ret);
        return ret;
    }

    // 发送命令
    ret = uart_send_data(send_data, strlen(send_data));
    if (ret != 0) {
        oal_print_err("uart_cmd_write_mem: send cmd fail [0x%x]\r\n", ret);
        return ret;
    }

    ret = uart_recv_data_expect(MSG_FORM_DRV_G);
    if (ret != 0) {
        oal_print_err("uart_cmd_write_mem: recv check fail [0x%x]\r\n", ret);
        return ret;
    }
    return 0;
}

osal_s32 uart_device_detect(osal_void)
{
    osal_u32 timeout_default = tty_read_timeout_get();
    tty_read_timeout_set(200); // 默认超时时间太长, 改为200ms
    uart_check_rx_data_empty();

    if (uart_get_firmware_version() != 0) {
        tty_read_timeout_set(timeout_default);
        oal_print_err("uart_get_firmware_version return fail\r\n");
        return OSAL_FAILURE;
    }

    tty_read_timeout_set(timeout_default);
    oal_print_warning("uart_get_firmware_version return ok\r\n");
    return OSAL_SUCCESS;
}
#endif // CONFIG_HCC_SUPPORT_UART

