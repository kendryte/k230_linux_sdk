/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: host platform firmware source file
 * Author: Huanghe
 * Create: 2021-06-08
 */
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/namei.h>
#endif
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_debug.h"
#endif
#include "plat_pm_board.h"
#include "plat_firmware.h"

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_cali.h"
#include "plat_pm.h"
#include "oam_ext_if.h"
#include "oal_types.h"
#endif
#include "securec.h"
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "device_crash_info.h"
#endif
#include "plat_exception_rst.h"

#include "oal_kernel_file.h"

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
#include "plat_sha256_calc.h"
#endif

#include "plat_service.h"
#include "hcc_test.h"
#include "hcc_if.h"
#include "hcc_bus.h"
#ifdef CONFIG_HCC_SUPPORT_USB
#include "hcc_usb_host.h"
#endif
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "customize_wifi.h"
#endif

enum firmware_verify_stage {
    FIRMWARE_VERIFY_TRANS_START = 0,
    FIRMWARE_VERIFY_TRANS,
    FIRMWARE_VERIFY_TRANS_END,
    FIRMWARE_VERIFY_TRANS_START_END,
    FIRMWARE_VERIFY_BUTT,
};

#define  FORMAT_BYTE                  ((char)'1')
#define  FORMAT_WORD                  ((char)'2')
#define  FORMAT_LONG                  ((char)'4')

typedef struct firmware_download_stru {
    firmware_cfg_cmd_enum cmd_type;
    osal_char cmd_name[FIRMWARE_BOARD_INFO_NAME_LEN];
    osal_char cmd_para[FIRMWARE_BOARD_INFO_PARA_LEN];
} firmware_download_stru;


#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define FW_BIN_DOWNLOAD_CMD         "1,0x400000,"CONFIG_FIRMWARE_BIN_PATH
#define FW_ROMBIN_DOWNLOAD_CMD      "1,0x106400,/etc/ws73/ws73_rom.bin"
#define FW_WIFICALI_DOWNLOAD_CMD    "1,0x430000,"CONFIG_FIRMWARE_WIFICALI_PATH
#define FW_BSLECALI_DOWNLOAD_CMD    "1,0x440000,"CONFIG_FIRMWARE_BSLECALI_PATH
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#define FW_BIN_DOWNLOAD_CMD         "1,0x400000,"CONFIG_FIRMWARE_BIN_PATH
#define FW_ROMBIN_DOWNLOAD_CMD      "1,0x106400,/jffs0/ws73_rom.bin"
#define FW_WIFICALI_DOWNLOAD_CMD    "1,0x430000,"CONFIG_FIRMWARE_WIFICALI_PATH
#define FW_BSLECALI_DOWNLOAD_CMD    "1,0x440000,"CONFIG_FIRMWARE_BSLECALI_PATH
#endif

firmware_download_stru g_firmware_download_set[] = {
    {
        NUM_TYPE_CMD,
        "WRITEM",
        "" // 该命令的具体内容从ini文件中读取，后填充
    },
#ifdef ROMBIN_OPEN
    {
        FILE_TYPE_CMD,
        "FILES",
        FW_ROMBIN_DOWNLOAD_CMD
    },
#endif
    {
        FILE_TYPE_CMD,
        "FILES",
        FW_BIN_DOWNLOAD_CMD
    },
    {
        FILE_TYPE_CMD,
        "FILES",
        FW_WIFICALI_DOWNLOAD_CMD
    },
#if defined(BT_EM_BUFFER_CALI_SUPPORT) && (BT_EM_BUFFER_CALI_SUPPORT == 1)
    {
        FILE_TYPE_CMD,
        "FILES",
        FW_BSLECALI_DOWNLOAD_CMD
    },
#endif
};

osal_char g_device_version[VERSION_LEN];
osal_u8 *g_fw_data_buf = NULL;
osal_u32 g_fw_data_buf_len = 0;
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
osal_char *g_sha256_data_buf = NULL;
#endif
osal_mutex g_firmware_mutex = {0};

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_s32 pre_download_get_cmu_xo_trim(osal_void)
{
    osal_s32 ret;
    osal_u32 cmu_xo_trim_param;

#ifdef CONFIG_NO_SUPPORT_INI
    cmu_xo_trim_param = CONFIG_INI_CMU_XO_TRIM;
#else
    cmu_xo_trim_param = ini_get_cmu_xo_trim();
#endif

    ret = snprintf_s(g_firmware_download_set[0].cmd_para,
                     FIRMWARE_BOARD_INFO_PARA_LEN, FIRMWARE_BOARD_INFO_PARA_LEN,
                     "%d,0x%08x,0x%04x", WR_CMU_XO_TRIM_LEN, WR_CMU_XO_TRIM_ADDR, cmu_xo_trim_param);
    if (ret < 0) {
        oal_print_err("snprintf_s err!\n");
        return -OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif

osal_s32 firmware_recv_func(osal_u8 *data, osal_s32 len)
{
    osal_s32 l_len;
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);

    if (unlikely((data == NULL))) {
        oal_print_err("data is NULL\n ");
        return -OAL_FAIL;
    }

    if (unlikely((pst_bus == NULL))) {
        oal_print_err("pst_bus is NULL\n ");
        return -OAL_FAIL;
    }

    l_len = hcc_bus_patch_read(pst_bus, data, len, READ_MEG_TIMEOUT);
    oal_print_dbg("Receive l_len=[%d], data = [%s]\n", l_len, data);

    return l_len;
}

osal_s32 firmware_recv_timeout_func(osal_u8 *data, osal_s32 len, osal_u32 timeout)
{
    osal_s32 l_len;
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);

    if (unlikely((NULL == data))) {
        oal_print_err("data is NULL\n ");
        return -OAL_FAIL;
    }

    if (unlikely((NULL == pst_bus))) {
        oal_print_err("pst_bus is NULL\n ");
        return -OAL_FAIL;
    }

    l_len = hcc_bus_patch_read(pst_bus, data, len, timeout);
    oal_print_dbg("Receive l_len=[%d], data = [%s]\n", l_len, data);

    return l_len;
}

osal_s32 firmware_send_func(osal_u8 *data, osal_s32 data_max_len, osal_s32 len)
{
    osal_s32 l_ret;
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);

    if (unlikely((NULL == pst_bus))) {
        oal_print_err("pst_bus is NULL\n ");
        return -OAL_FAIL;
    }
    if (unlikely(len > data_max_len)) {
        oal_print_err("len[%d] > data_max_len[%d], FAIL! \n ", len, data_max_len);
        return -OAL_FAIL;
    }

    oal_print_dbg("len = %d\n", len);
#ifdef HW_DEBUG
#define MAX_DUMP_LEN 128
    print_hex_dump_bytes("firmware_send_func :", DUMP_PREFIX_ADDRESS, data, (len < MAX_DUMP_LEN ? len : MAX_DUMP_LEN));
#endif
    l_ret = hcc_bus_patch_write(pst_bus, data, len);

    return l_ret;
}

osal_s32 firmware_recv_expect_result_func(const osal_u8 *expect)
{
    osal_u8 auc_buf[RECV_BUF_LEN];
    osal_s32 l_len;
    osal_s32 i;

    if (strlen(expect) == 0) {
        oal_print_dbg("not wait device to respond!\n");
        return OAL_SUCC;
    }

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_len = firmware_recv_func(auc_buf, RECV_BUF_LEN);
        if (l_len < 0) {
            oal_print_err("recv result fail\n");
            continue;
        }

        if (memcmp(auc_buf, expect, strlen(expect)) == 0) {
            oal_print_dbg(" send OAL_SUCC, expect [%s] ok\n", expect);
            return OAL_SUCC;
        } else {
            oal_print_warning(" error result[%s], expect [%s], read result again\n", auc_buf, expect);
        }
    }

    return -OAL_FAIL;
}

osal_s32 firmware_recv_expect_result_timeout_func(const osal_u8 *expect, osal_u32 timeout)
{
    osal_u8 auc_buf[RECV_BUF_LEN];
    osal_s32 l_len;

    if (strlen(expect)== 0) {
        oal_print_dbg("not wait device to respond!\n");
        return OAL_SUCC;
    }

    memset_s(auc_buf, RECV_BUF_LEN, 0, RECV_BUF_LEN);
    l_len = firmware_recv_timeout_func(auc_buf, RECV_BUF_LEN, timeout);
    if (l_len < 0) {
        oal_print_err("recv result fail\n");
        return -OAL_FAIL;
    }

    if (memcmp(auc_buf, expect, strlen(expect)) == 0) {
        oal_print_dbg(" send OAL_SUCC, expect [%s] ok\n", expect);
        return OAL_SUCC;
    } else {
        oal_print_warning(" error result[%s], expect [%s], read result again\n", auc_buf, expect);
    }

    return -OAL_FAIL;
}

osal_s32 firmware_send_and_recv_expect_result_func(osal_u8 *data, osal_s32 len, const osal_u8 *expect)
{
    osal_s32 i;
    osal_s32 l_ret;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = firmware_send_func(data, len, len);
        if (l_ret < 0) {
            continue;
        }
        l_ret = firmware_recv_expect_result_func(expect);
        if (l_ret == 0) {
            return OAL_SUCC;
        }
    }

    return -OAL_FAIL;
}

static osal_s32 firmware_send_quit_func(osal_u8 *data, osal_s32 len)
{
    osal_s32 i;
    osal_s32 l_ret;

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        l_ret = firmware_send_func(data, len, len);
        if (l_ret >= 0) {
            l_ret = firmware_recv_expect_result_func(MSG_FROM_DEV_QUIT_OK);
#ifndef CONFIG_HCC_SUPPORT_USB
            if (l_ret == 0) {
                return OAL_SUCC;
            }
#endif
        } else {
            oal_print_warning("firmware_send_quit_func: send QUIT msg fail, ret=[%d].\r\n", l_ret);
        }

#ifdef CONFIG_HCC_SUPPORT_USB
        l_ret = hcc_usb_reload(hcc_get_channel_bus(HCC_CHANNEL_AP));
        if (l_ret == 0) {
            return OAL_SUCC;
        }
#endif
        oal_print_warning("firmware_send_quit_func: wait for response fail, retry! ret=[%d].\r\n", l_ret);
    }

    oal_print_err("firmware_send_quit_func: failed, ret=[%d].\r\n", l_ret);
    return -OAL_FAIL;
}


/* ****************************************************************************
 功能描述  : 将字符串转换成正整数
 输入参数  : string:输入的字符串
 输出参数  : number:字符串转换以后的正整数
 ***************************************************************************** */
#define DECIMAL_NUM 10
osal_s32 string_to_num_etc(osal_u8 *string, osal_s32 *number)
{
    osal_s32 i;
    osal_s32 l_num;

    if (NULL == string) {
        oal_print_err("string is NULL!\n");
        return -OAL_FAIL;
    }

    l_num = 0;
    for (i = 0; (string[i] >= '0') && (string[i] <= '9'); i++) {
        l_num = (l_num * DECIMAL_NUM) + (string[i] - '0');
    }

    *number = l_num;

    return OAL_SUCC;
}

osal_s32 firmware_check_version(void)
{
    osal_s32 ret;
    osal_s32 l_len;
    osal_s32 i;
    osal_u8 rec_buf[VERSION_LEN];

    for (i = 0; i < HOST_DEV_TIMEOUT; i++) {
        (osal_void)memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);

        ret = memcpy_s(rec_buf, VERSION_LEN, (osal_u8 *)VER_CMD_KEYWORD, strlen(VER_CMD_KEYWORD));
        if (ret != EOK) {
            return -OAL_FAIL;
        }

        l_len = (osal_s32)strlen(VER_CMD_KEYWORD);

        rec_buf[l_len] = COMPART_KEYWORD;
        l_len++;

        ret = firmware_send_func(rec_buf, VERSION_LEN, l_len);
        if (ret < 0) {
            oal_print_err("send version fail![%d]\n", i);
            continue;
        }

        (osal_void)memset_s(g_device_version, VERSION_LEN, 0, VERSION_LEN);
        (osal_void)memset_s(rec_buf, VERSION_LEN, 0, VERSION_LEN);
        msleep(1);

        ret = firmware_recv_func(rec_buf, VERSION_LEN);
        if (ret < 0) {
            oal_print_err("read version fail![%d]\n", i);
            continue;
        }

        ret = memcpy_s(g_device_version, VERSION_LEN, rec_buf, VERSION_LEN);
        if (ret != EOK) {
            return -OAL_FAIL;
        }

        oal_print_err("Device Version = [%s]\n", g_device_version);
        return OAL_SUCC;
    }

    return -OAL_FAIL;
}

osal_s32 firmware_number_type_cmd_send_etc(osal_u8 *key, osal_u8 *value)
{
    osal_s32 l_ret;
    size_t data_len;
    size_t value_len;
    size_t i;
    size_t n;
    osal_u8 auc_num[INT32_STR_LEN];
    osal_u8 buff_tx[SEND_BUF_LEN];

    value_len = strlen((osal_char *)value);

    (osal_void)memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
    (osal_void)memset_s(buff_tx, SEND_BUF_LEN, 0, SEND_BUF_LEN);

    data_len = 0;
    data_len = strlen(key);
    l_ret = memcpy_s(buff_tx, SEND_BUF_LEN, key, data_len);
    if (l_ret != EOK) {
        return -OAL_FAIL;
    }

    buff_tx[data_len] = COMPART_KEYWORD;
    data_len = data_len + 1;

    for (i = 0, n = 0; (i <= value_len) && (n < INT32_STR_LEN); i++) {
        if ((',' == value[i]) || (i == value_len)) {
            oal_print_dbg("auc_num = %s, i = %u, n = %u\n", auc_num, i, n);
            if (0 == n) {
                continue;
            }

            l_ret = memcpy_s((osal_u8 *)&buff_tx[data_len], SEND_BUF_LEN - data_len, auc_num, n);
            if (l_ret != EOK) {
                return -OAL_FAIL;
            }

            data_len = data_len + n;

            buff_tx[data_len] = COMPART_KEYWORD;
            data_len = data_len + 1;

            (osal_void)memset_s(auc_num, INT32_STR_LEN, 0, INT32_STR_LEN);
            n = 0;
        } else if (COMPART_KEYWORD == value[i]) {
            continue;
        } else {
            auc_num[n] = value[i];
            n++;
        }
    }

    l_ret = firmware_send_func(buff_tx, SEND_BUF_LEN, data_len);

    return l_ret;
}

/* ****************************************************************************
 函 数 名  : firmware_parse_file_cmd_etc
 功能描述  : 解析file命令参数
 输入参数  : string   : file命令的参数
             addr     : 发送的数据地址
             file_path: 发送文件的路径
**************************************************************************** */
osal_s32 firmware_parse_file_cmd_etc(osal_u8 *string, unsigned long *addr, osal_char **file_path)
{
    osal_u8 *tmp;
    osal_s32 count = 0;
    osal_char *after;

    if (string == NULL || addr == NULL || file_path == NULL) {
        oal_print_err("param is error!\n");
        return -OAL_FAIL;
    }

    /* 获得发送的文件的个数，此处必须为1，string字符串的格式必须是"1,0xXXXXX,file_path" */
    tmp = string;
    while (COMPART_KEYWORD == *tmp) {
        tmp++;
    }
    string_to_num_etc(tmp, &count);
    if (count != FILE_COUNT_PER_SEND) {
        oal_print_err("the count of send file must be 1, count = [%d]\n", count);
        return -OAL_FAIL;
    }

    /* 让tmp指向地址的首字母 */
    tmp = osal_strchr(string, ',');
    if (tmp == NULL) {
        oal_print_err("param string is err!\n");
        return -OAL_FAIL;
    } else {
        tmp++;
        while (COMPART_KEYWORD == *tmp) {
            tmp++;
        }
    }

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    *addr = simple_strtoul(tmp, &after, 16); // 16: 16进制
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    *addr = strtoul(tmp, &after, 16); // 16: 16进制
#else
    oal_print_err("unsupported operating system!\n");
    return -OAL_FAIL;
#endif

    oal_print_dbg("file to send addr:[0x%lx]\n", *addr);

    /* "1,0xXXXX,file_path" */
    /*         ^          */
    /*       after        */
    while (*after == COMPART_KEYWORD) {
        after++;
    }
    /* 跳过','字符 */
    after++;
    while (*after == COMPART_KEYWORD) {
        after++;
    }

    oal_print_dbg("after:[%s]\n", after);

    *file_path = after;

    return OAL_SUCC;
}

#define READ_DEVICE_MAX_BUF_SIZE 128
#define READ_WIDTH 4
#define PRINT_SIZE 8
#define GROUP_SIZE 32
osal_s32 read_device_reg(osal_u32 address, osal_u32 *value)
{
    osal_s32 ret;
    osal_s32 buf_len;
    osal_u8 buf_tx[READ_DEVICE_MAX_BUF_SIZE];
    osal_u8 buf_result[READ_DEVICE_MAX_BUF_SIZE];

    memset_s(buf_tx, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);
    memset_s(buf_result, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), READ_DEVICE_MAX_BUF_SIZE, "%s%c0x%x%c%d%c", RMEM_CMD_KEYWORD,
        COMPART_KEYWORD, address, COMPART_KEYWORD, READ_WIDTH, COMPART_KEYWORD);

    ret = firmware_send_func(buf_tx, READ_DEVICE_MAX_BUF_SIZE, buf_len);
    if (ret < 0) {
        oal_print_err("send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = firmware_recv_func(buf_result, READ_WIDTH);
    if (ret > 0) {
        *value = *((osal_u32 *)(buf_result));
        return 0;
    }

    oal_print_err("rsv result failed, ret=%d", ret);
    return -1;
}

/* write device regs by bootloader */
osal_s32 write_device_reg(osal_u32 address, osal_u32 value)
{
    osal_s32 ret;
    osal_s32 buf_len;
    osal_u8 buf_tx[READ_DEVICE_MAX_BUF_SIZE];

    memset_s(buf_tx, READ_DEVICE_MAX_BUF_SIZE, 0, READ_DEVICE_MAX_BUF_SIZE);

    buf_len = snprintf_s(buf_tx, sizeof(buf_tx), READ_DEVICE_MAX_BUF_SIZE, "%s%c%c%c0x%x%c%u%c",
        WMEM_CMD_KEYWORD, COMPART_KEYWORD, FORMAT_LONG, COMPART_KEYWORD, address, COMPART_KEYWORD,
        value, COMPART_KEYWORD);

    ret = firmware_send_func(buf_tx, READ_DEVICE_MAX_BUF_SIZE, buf_len);
    if (ret < 0) {
        oal_print_err("send msg [%s] failed, ret=%d", buf_tx, ret);
        return ret;
    }

    ret = firmware_recv_expect_result_func(MSG_FROM_DEV_WRITEM_OK);
    if (ret < 0) {
        oal_print_err("send msg [%s] recv failed, ret=%d", buf_tx, ret);
        return ret;
    }

    return 0;
}

#if defined(CONFIG_SUPPORT_HCC_CONN_CHECK) && (CONFIG_SUPPORT_HCC_CONN_CHECK != 0)
#ifdef CONFIG_HCC_SUPPORT_USB
#define REG_GP_REG0 (0x40019380)

osal_u32 hcc_usb_bulk_out_timeout_get(osal_void);
osal_void hcc_usb_bulk_out_timeout_set(osal_u32 timeout_ms);
#endif

osal_s32 hcc_connect_state_check(osal_void)
{
#ifdef CONFIG_HCC_SUPPORT_USB
    osal_s32 ret;
    osal_u32 val, usb_timeout_default;

    // 设置超时时间
    usb_timeout_default = hcc_usb_bulk_out_timeout_get();
    hcc_usb_bulk_out_timeout_set(500); // 设为500ms超时

    ret = read_device_reg(REG_GP_REG0, &val);

    // 恢复超时时间
    hcc_usb_bulk_out_timeout_set(usb_timeout_default);

    return ret;
#else
    return OAL_SUCC;
#endif
}
#endif

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_PLAT_SUPPORT_DFR)
static osal_s32 firmware_dev_crash_read_mem_etc(osal_u8 *puc_data_buf, os_kernel_file_stru *fp, osal_s32 read_size,
    osal_s32 retry_num, osal_u32 limit)
{
    osal_u32 readlen;
    osal_u32 sdio_transfer_limit = limit;
    osal_s32 size = read_size;
    osal_s32 l_ret = -OAL_FAIL;
    osal_s32 retry = retry_num;

    oal_print_dbg("recv len [%d]\n", size);
    while (size > 0) {
        readlen = min((osal_u32)size, sdio_transfer_limit);
        l_ret = recv_device_mem_etc(fp, puc_data_buf, size);
        if (l_ret > 0) {
            size -= l_ret;
        } else {
            oal_print_err("read error retry:%d\n", retry);
            --retry;
            if (!retry) {
                oal_print_err("retry fail\n");
                break;
            }
        }
    }
    return l_ret;
}

osal_s32 firmware_dev_crash_read_mem(osal_u8 *key, osal_u8 *value)
{
    osal_s32 l_ret = -OAL_FAIL;
    osal_s32 size;
    osal_s32 retry = 3;
    osal_u8 *flag;
    os_kernel_file_stru *fp;
    osal_u8 *puc_data_buf = NULL;
    osal_u32 sdio_transfer_limit = hcc_get_bus_max_trans_size(HCC_BUS_SDIO);

    sdio_transfer_limit = uapi_min(PAGE_SIZE, sdio_transfer_limit);

    flag = osal_strchr(value, ',');
    if (NULL == flag) {
        oal_print_err("RECV LEN ERROR..\n");
        return -OAL_FAIL;
    }
    flag++;
    oal_print_dbg("recv len [%s]\n", flag);
    while (COMPART_KEYWORD == *flag) {
        flag++;
    }

    string_to_num_etc(flag, &size);

    do {
        oal_print_info("try to malloc sdio mem read buf len is [%d]\n", sdio_transfer_limit);
        puc_data_buf = (osal_u8 *)osal_kmalloc(sdio_transfer_limit, (GFP_KERNEL | __GFP_NOWARN));
        if (NULL == puc_data_buf) {
            oal_print_warning("malloc mem len [%d] fail, continue to try in a smaller size\n", sdio_transfer_limit);
            sdio_transfer_limit = sdio_transfer_limit >> 1;
        }
    } while ((NULL == puc_data_buf) && (sdio_transfer_limit >= MIN_FIRMWARE_FILE_TX_BUF_LEN));

    if (NULL == puc_data_buf) {
        oal_print_err("puc_data_buf KMALLOC failed\n");
        return -OAL_FAIL;
    }

    fp = open_file_to_readm_etc(NULL, O_RDWR | O_CREAT | O_APPEND);
    if (IS_ERR(fp)) {
        oal_print_err("create file error,fp = 0x%p\n", fp);
        osal_kfree(puc_data_buf);
        return OAL_SUCC;
    }

    l_ret = firmware_number_type_cmd_send_etc(key, value);
    if (0 > l_ret) {
        oal_print_err("send %s,%s fail \n", key, value);
        filp_close(fp, NULL);
        osal_kfree(puc_data_buf);
        return l_ret;
    }

    l_ret = firmware_dev_crash_read_mem_etc(puc_data_buf, fp, size, retry, sdio_transfer_limit);

    filp_close(fp, NULL);
    osal_kfree(puc_data_buf);

    return l_ret;
}
#endif

static osal_s32 firmware_cmd_number_func_inner(osal_u8 *key, osal_u8 *value)
{
    osal_s32 l_ret = -OAL_FAIL;
    if (!strcmp((osal_char *)key, WMEM_CMD_KEYWORD)) {
        l_ret = firmware_number_type_cmd_send_etc(key, value);
        if (0 > l_ret) {
            oal_print_err("send key=[%s],value=[%s] fail\n", key, value);
            return l_ret;
        }

        l_ret = firmware_recv_expect_result_func(MSG_FROM_DEV_WRITEM_OK);
        if (0 > l_ret) {
            oal_print_err("recv expect result fail!\n");
            return l_ret;
        }
    } else if (!strcmp((osal_char *)key, JUMP_CMD_KEYWORD)) {
        l_ret = firmware_number_type_cmd_send_etc(key, value);
        if (0 > l_ret) {
            oal_print_err("send key=[%s],value=[%s] fail\n", key, value);
            return l_ret;
        }

        /* 100000ms timeout */
        l_ret = firmware_recv_expect_result_timeout_func(MSG_FROM_DEV_JUMP_OK, READ_MEG_JUMP_TIMEOUT);
        if (l_ret >= 0) {
            oal_print_info("JUMP OAL_SUCCess!\n");
        } else {
            oal_print_err("CMD JUMP timeout! l_ret=%d\n", l_ret);
        }
        return l_ret;
    } else if (!strcmp((osal_char *)key, SETPM_CMD_KEYWORD) ||
        !strcmp((osal_char *)key, SETBUCK_CMD_KEYWORD) ||
        !strcmp((osal_char *)key, SETSYSLDO_CMD_KEYWORD) ||
        !strcmp((osal_char *)key, SETNFCRETLDO_CMD_KEYWORD) ||
        !strcmp((osal_char *)key, SETPD_CMD_KEYWORD) ||
        !strcmp((osal_char *)key, SETNFCCRG_CMD_KEYWORD) ||
        !strcmp((osal_char *)key, SETABB_CMD_KEYWORD) ||
        !strcmp((osal_char *)key, SETTCXODIV_CMD_KEYWORD)) {
        l_ret = firmware_number_type_cmd_send_etc(key, value);
        if (0 > l_ret) {
            oal_print_err("send key=[%s],value=[%s] fail\n", key, value);
            return l_ret;
        }

        l_ret = firmware_recv_expect_result_func(MSG_FROM_DEV_SET_OK);
        if (0 > l_ret) {
            oal_print_err("recv expect result fail!\n");
            return l_ret;
        }
    } else if (!strcmp((osal_char *)key, RMEM_CMD_KEYWORD)) {
#if defined(CONFIG_PLAT_SUPPORT_DFR)
        l_ret = firmware_dev_crash_read_mem(key, value);
#endif
    }

    return l_ret;
}

osal_s32 firmware_cmd_number_func(osal_u8 *key, osal_u8 *value)
{
    osal_s32 l_ret = -OAL_FAIL;
    pm_board_info *board_info = NULL;

    board_info = get_pm_board_info();
    if (NULL == board_info) {
        oal_print_err("board_info is null!\n");
        return -OAL_FAIL;
    }

    if (!memcmp(key, VER_CMD_KEYWORD, strlen(VER_CMD_KEYWORD))) {
        l_ret = firmware_check_version();
        if (0 > l_ret) {
            oal_print_err("check version FAIL [%d]\n", l_ret);
            return -OAL_FAIL;
        }
    }

    l_ret = firmware_cmd_number_func_inner(key, value);
    return l_ret;
}
#endif

#define FIRMWARE_CMD_QUIT_BUF_LEN 8
osal_s32 firmware_cmd_quit_func(void)
{
    osal_s32 l_ret;
    osal_s32 l_len;
    osal_u8 *buf = OSAL_NULL;

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pm_board_info *board_info = NULL;

    board_info = get_pm_board_info();
    if (NULL == board_info) {
        oal_print_err("board_info is null!\n");
        return -OAL_FAIL;
    }
#endif

    buf = osal_kmalloc(FIRMWARE_CMD_QUIT_BUF_LEN, 0);
    if (buf == OSAL_NULL) {
        oal_print_err("malloc fail, len[%d] \n", FIRMWARE_CMD_QUIT_BUF_LEN);
        return -OAL_FAIL;
    }
    (osal_void)memset_s(buf, FIRMWARE_CMD_QUIT_BUF_LEN, 0, FIRMWARE_CMD_QUIT_BUF_LEN);

    l_ret = memcpy_s(buf, FIRMWARE_CMD_QUIT_BUF_LEN, (osal_u8 *)QUIT_CMD_KEYWORD, strlen(QUIT_CMD_KEYWORD));
    if (l_ret != EOK) {
        osal_kfree(buf);
        return -OAL_FAIL;
    }

    l_len = (osal_s32)strlen(QUIT_CMD_KEYWORD);

    buf[l_len] = COMPART_KEYWORD;
    l_len++;

    l_ret = firmware_send_quit_func(buf, l_len);
    osal_kfree(buf);

    return l_ret;
}

osal_u32 firmware_check_download_bin_addr(osal_u32 file_len, osal_ulong addr)
{
    if (!file_len || !addr) {
        oal_print_err("file check error file_len = 0x%x, file_addr = 0x%lx!\n", file_len, addr);
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
OAL_STATIC osal_s32 do_file_read_specified_len(osal_char *buf, osal_u32 len, osal_void *fp)
{
    osal_s32 read_len;
    osal_u32 remained;

    remained = len;
    while (remained > 0) {
        read_len = osal_klib_fread(buf, FILE_READ_SIZE <= remained ? FILE_READ_SIZE : remained, fp);
        if (read_len > 0) {
            oal_print_dbg("len of osal_klib_fread is [%d]\n", read_len);
        } else {
            oal_print_err("len of osal_klib_fread is [%d]\n", read_len);
            return -OAL_FAIL;
        }
        remained = remained - (osal_u32)read_len;
        buf = buf + read_len;
    }

    if (remained != 0) {
        oal_print_err("something wrong in do_file_read_specified_len, remained:%d.\n", remained);
        return -OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * content: 保存文件的内容，去掉 sha256sum header
 * len: 文件的长度，包含 header
 * sha256sum: 保存文件的 sha256sum header
 */
OAL_STATIC osal_u32 get_file_content(osal_void *fp, osal_char *content, osal_u32 len, osal_char *sha256sum)
{
    osal_s32 ret;

    osal_klib_fseek(0, SEEK_SET, fp);
    ret = do_file_read_specified_len(sha256sum, SHA256_HEADER_LENGTH, fp);
    if (ret != OAL_SUCC) {
        oal_print_err("do_file_read_specified_len err!\n");
        return -OAL_FAIL;
    }

    ret = do_file_read_specified_len(content, len - SHA256_HEADER_LENGTH, fp);
    if (ret != OAL_SUCC) {
        oal_print_err("do_file_read_specified_len err!\n");
        return -OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * buf: 待校验的内容，其中不包含 header
 * len: 待校验内容的长度，不包含 header
 * sum: 待校验文件的头部中的 sha256sum
 */
OAL_STATIC osal_s32 do_check_sha256sum(osal_char *buf, osal_u32 len, osal_char *checksum)
{
    osal_char calc_sum_arr[SHA256_HEADER_ARR_LEN] = {0};
    osal_char calc_sum[SHA256_HEADER_LENGTH + 1] = {0};
    osal_s32 i;
    osal_s32 ret;

    do_sha256(buf, len, calc_sum_arr);
    for (i = 0; i < SHA256_HEADER_ARR_LEN; i++) {
        ret = snprintf_s(calc_sum + SHA256_HEADER_BYTES_LEN * i,
                         SHA256_HEADER_LENGTH - SHA256_HEADER_BYTES_LEN * i + 1,
                         SHA256_HEADER_BYTES_LEN + 1, "%02x", (osal_u8)calc_sum_arr[i]);
        if (ret < 0) {
            oal_print_err("snprintf_s err!\n");
            return -OAL_FAIL;
        }
    }

    if (strncmp(calc_sum, checksum, SHA256_HEADER_LENGTH) != 0) {
        oal_print_err("do_check_sha256sum err!\n");
        return -OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC osal_void *plat_sha256_mem_alloc(osal_ulong size)
{
#if defined(CONFIG_SUPPORT_SHA256_MEM_USING_VMALLOC) && (CONFIG_SUPPORT_SHA256_MEM_USING_VMALLOC != 0)
    return osal_vmalloc(size);
#else
    return osal_kmalloc(size, 0);
#endif
}

OAL_STATIC osal_void plat_sha256_mem_free(osal_void *addr)
{
#if defined(CONFIG_SUPPORT_SHA256_MEM_USING_VMALLOC) && (CONFIG_SUPPORT_SHA256_MEM_USING_VMALLOC != 0)
    return osal_vfree(addr);
#else
    return osal_kfree(addr);
#endif
}

/*
 *                 File's Structure
 *
 * |<--- sha256sum header --->|<--- file content --->|
 * |         64 Bytes         |      *** Bytes       |
 */
osal_s32 firmware_sha256_checksum(const osal_char *path)
{
    osal_void *fp;
    osal_char sha256sum[SHA256_HEADER_LENGTH] = {0};
    osal_u32 file_len;
    osal_s32 tmp;
    osal_char *file_content = NULL;

    fp = osal_klib_fopen(path, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        oal_print_err("filp_open [%s] fail!!, fp=%p\n", path, fp);
        fp = NULL;
        return -OAL_FAIL;
    }
    oal_print_info("filp_open [%s] succ!!, fp=%p, fd=%d.\n", path, fp, *((osal_s32 *)fp));

    /* 获取file文件大小 */
    tmp = osal_klib_fseek(0, SEEK_END, fp);
    if (tmp <= 0 || tmp <= SHA256_HEADER_LENGTH || tmp > FIRMWARE_FILESIZE_MAX) {
        oal_print_err("file size of %s is err: %d.!!\n", path, tmp);
        goto one_out;
    }
    file_len = (osal_u32)tmp;

#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    file_content = g_sha256_data_buf;
    if (file_content == NULL) {
        oal_print_err("get g_sha256_data_buf fail!\n");
        goto one_out;
    }
#else
    file_content = (osal_char *)plat_sha256_mem_alloc(file_len);
    if (file_content == NULL) {
        oal_print_err("sha256 mem alloc fail!!, len:%d.\n", file_len);
        goto one_out;
    }
#endif

    if (get_file_content(fp, file_content, file_len, sha256sum) != OAL_SUCC) {
        oal_print_err("get_file_content fail!!.\n");
        goto two_out;
    }

    if (do_check_sha256sum(file_content, file_len - SHA256_HEADER_LENGTH, sha256sum) != OAL_SUCC) {
        oal_print_err("do_check_sha256sum fail!!.\n");
        goto two_out;
    }

#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    (osal_void)memset_s(file_content, FIRMWARE_FILESIZE_MAX, 0, FIRMWARE_FILESIZE_MAX);
#else
    plat_sha256_mem_free(file_content);
#endif
    osal_klib_fclose(fp);

    return OAL_SUCC;

two_out:
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    (osal_void)memset_s(file_content, FIRMWARE_FILESIZE_MAX, 0, FIRMWARE_FILESIZE_MAX);
#else
    plat_sha256_mem_free(file_content);
#endif

one_out:
    osal_klib_fclose(fp);
    fp = NULL;

    return -OAL_FAIL;
}
EXPORT_SYMBOL(firmware_sha256_checksum);
#endif

static osal_s32 read_and_send_file_etc(osal_u32 *send_offset, osal_void *fp, unsigned long addr, osal_u32 file_len)
{
    unsigned long addr_send;
    osal_s32 ret, rdlen;
    osal_u32 offset = 0;
    osal_u32 i, transmit_limit, per_send_len, send_count;
    osal_u8 buff_tx[SEND_BUF_LEN] = {0};

    transmit_limit = g_fw_data_buf_len;
    per_send_len = (transmit_limit > file_len) ? file_len : transmit_limit;
    send_count = (file_len + per_send_len - 1) / per_send_len;

    for (i = 0; i < send_count; i++) {
        osal_u32 trans_state = FIRMWARE_VERIFY_TRANS;

        rdlen = osal_klib_fread(g_fw_data_buf, per_send_len, fp);
        if (rdlen <= 0) {
            oal_print_err("len of kernel_read is error! ret=[%d], i=%d\n", rdlen, i);
            return -OAL_FAIL;
        }

        addr_send = addr + offset;
        if (send_count <= 1) {
            trans_state = FIRMWARE_VERIFY_TRANS_START_END;
        } else if (i == 0) {
            trans_state = FIRMWARE_VERIFY_TRANS_START;
        } else if (i == send_count - 1) {
            trans_state = FIRMWARE_VERIFY_TRANS_END;
        }

        ret = snprintf_s(buff_tx, SEND_BUF_LEN, SEND_BUF_LEN, "%s%c%d%c0x%lx%c0x%lx%c0x%lx%c", FILES_CMD_KEYWORD,
            COMPART_KEYWORD, FILE_COUNT_PER_SEND, COMPART_KEYWORD, addr_send, COMPART_KEYWORD, (unsigned long)rdlen,
            COMPART_KEYWORD, (unsigned long)trans_state, COMPART_KEYWORD);
        if (ret < 0) {
            oal_print_err("snprintf_s error, return FAIL \n");
            return -OAL_FAIL;
        }

        /* 发送地址 */
        ret = firmware_send_and_recv_expect_result_func(buff_tx, strlen(buff_tx), MSG_FROM_DEV_READY_OK);
        if (0 > ret) {
            return -OAL_FAIL;
        }

        /* Wait at least 5 ms */
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        oal_usleep_range(FILE_CMD_WAIT_TIME_MIN, FILE_CMD_WAIT_TIME_MAX);
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
        LOS_Udelay(5000); // 5000: 5ms
#endif
        /* 发送文件内容 */
        ret = firmware_send_and_recv_expect_result_func(g_fw_data_buf, rdlen, MSG_FROM_DEV_FILES_OK);
        if (0 > ret) {
            return -OAL_FAIL;
        }
        offset += (osal_u32)rdlen;
    }
    *send_offset = offset;

    return OAL_SUCC;
}
#ifndef SHA256_HEADER_LENGTH
#define SHA256_HEADER_LENGTH 64
#endif
static osal_s32 start_parse_file_etc(osal_char *path, unsigned long addr)
{
    osal_s32 ret;
    osal_u32 file_len;
    osal_u32 offset;
    osal_void *fp;

#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
    if (firmware_sha256_checksum(path) != OAL_SUCC) {
        oal_print_err("[%s] sha256sum check fail!!\n", path);
        return -OAL_FAIL;
    }
    oal_print_err("[%s] sha256sum check succ!!\n", path);
#endif

    fp = osal_klib_fopen(path, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        oal_print_err("filp_open [%s] fail!!, fp=%p\n", path, fp);
        return -OAL_FAIL;
    }

    /* 获取file文件大小 */
    file_len = osal_klib_fseek(0, SEEK_END, fp);
    if (file_len <= 0) {
        oal_print_err("file size of %s is 0!!\n", path);
        osal_klib_fclose(fp);
        return -OAL_FAIL;
    }

    /* skip bin's header area */
    file_len = file_len - SHA256_HEADER_LENGTH;

    if (firmware_check_download_bin_addr(file_len, addr) != OAL_TRUE) {
        oal_print_err("file is invalid!!\n");
        osal_klib_fclose(fp);
        return -OAL_FAIL;
    }

    /* 定位到文件实际内容开头 */
    osal_klib_fseek(SHA256_HEADER_LENGTH, SEEK_SET, fp);

    /* 解析并发送文件 */
    ret = read_and_send_file_etc(&offset, fp, addr, file_len);
    osal_klib_fclose(fp);
    if (ret != OAL_SUCC) {
        oal_print_err("parse and send file fail!\n");
        return ret;
    }

    if (offset != file_len) {
        oal_print_err("file send len is err! send len is [%d], file len is [%d]\n", offset, file_len);
        return -OAL_FAIL;
    }

    return OAL_SUCC;
}

osal_s32 exec_file_type_cmd_etc(osal_u8 *key, osal_u8 *value)
{
    unsigned long addr;
    osal_char *path;
    osal_s32 ret;

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pm_board_info *board_info = NULL;
#endif
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);

    if (pst_bus == NULL) {
        return -OAL_FAIL;
    }

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    board_info = get_pm_board_info();
    if (NULL == board_info) {
        oal_print_err("board_info is null!\n");
        return -OAL_FAIL;
    }
#endif

    /* 解析cmd */
    ret = firmware_parse_file_cmd_etc(value, &addr, &path);
    if (ret < 0) {
        oal_print_err("parse file cmd fail!\n");
        return ret;
    }

    oal_print_info("download firmware:%s addr:0x%x\n", path, (osal_u32)addr);

    /* 根据路径和地址进行文件解析发送处理 */
    ret = start_parse_file_etc(path, addr);
    if (ret < 0) {
        oal_print_err("parse file fail!\n");
    }
    return ret;
}

osal_s32 firmware_cmd_func(osal_s32 cmd_type, osal_u8 *cmd_name, osal_u8 *cmd_para)
{
    osal_s32 l_ret;
    switch (cmd_type) {
        case FILE_TYPE_CMD:
            oal_print_dbg(" command type FILE_TYPE_CMD\n");
            l_ret = exec_file_type_cmd_etc(cmd_name, cmd_para);
            break;
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        case NUM_TYPE_CMD:
            oal_print_dbg(" command type NUM_TYPE_CMD\n");
            l_ret = firmware_cmd_number_func(cmd_name, cmd_para);
            break;
#endif
        case QUIT_TYPE_CMD:
            oal_print_dbg(" command type QUIT_TYPE_CMD\n");
            l_ret = firmware_cmd_quit_func();
            break;
        default:
            oal_print_err("command type error[%d]\n", cmd_type);
            l_ret = -OAL_FAIL;
            break;
    }

    return l_ret;
}

osal_s32 firmware_download_bin_etc(void)
{
    osal_s32 l_ret;
    size_t i;
    size_t count;
    count = sizeof(g_firmware_download_set) / sizeof(g_firmware_download_set[0]);

    for (i = 0; i < count; i++) {
        l_ret = firmware_cmd_func(g_firmware_download_set[i].cmd_type,
            g_firmware_download_set[i].cmd_name, g_firmware_download_set[i].cmd_para);
        if (0 > l_ret) {
            return l_ret;
        }
    }

    return OAL_SUCC;
}

static osal_void firmware_buf_init(hcc_bus *pst_bus)
{
#if !(defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0))
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef PLAT_LITE_EXTREME
    g_fw_data_buf = (osal_u8 *)kmalloc(MAX_FIRMWARE_FILE_TX_BUF_LEN, GFP_KERNEL | __GFP_NOWARN);
    g_fw_data_buf_len = MAX_FIRMWARE_FILE_TX_BUF_LEN;
#else
    g_fw_data_buf = (osal_u8 *)oal_memtry_alloc(uapi_min(pst_bus->cap_max_trans_size, MAX_FIRMWARE_FILE_TX_BUF_LEN),
        MIN_FIRMWARE_FILE_TX_BUF_LEN, &g_fw_data_buf_len);
#endif
    g_fw_data_buf_len = OAL_ROUND_DOWN(g_fw_data_buf_len, 8); /* 按8对齐 */
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    g_fw_data_buf = (osal_u8 *)malloc(MAX_FIRMWARE_FILE_TX_BUF_LEN);
    g_fw_data_buf_len = MAX_FIRMWARE_FILE_TX_BUF_LEN;
#endif
#endif
}

static osal_void firmware_buf_deinit(void)
{
    // free memory after process
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    (osal_void)memset_s(g_fw_data_buf, g_fw_data_buf_len, 0, g_fw_data_buf_len);
#else
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_kfree(g_fw_data_buf);
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    free(g_fw_data_buf);
#endif
    g_fw_data_buf = NULL;
#endif
}

osal_s32 firmware_download_etc(void)
{
    hcc_bus *pst_bus;

    pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    if (NULL == pst_bus) {
        oal_print_err("firmware curr bus is null\n");
        return -OAL_FAIL;
    }

    osal_mutex_lock(&g_firmware_mutex);

    firmware_buf_init(pst_bus);

    if (NULL == g_fw_data_buf || (0 == g_fw_data_buf_len)) {
        osal_mutex_unlock(&g_firmware_mutex);
        oal_print_err("g_fw_data_buf KMALLOC failed, min request:%u\n", MIN_FIRMWARE_FILE_TX_BUF_LEN);
        return -OAL_FAIL;
    }

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    pre_download_get_cmu_xo_trim();
#endif

    if (firmware_download_bin_etc() != OAL_SUCC) {
        goto download_failed;
    }

    if (firmware_cmd_func(QUIT_TYPE_CMD, "", "") != OAL_SUCC) {
        goto download_failed;
    }

    firmware_buf_deinit();

    osal_mutex_unlock(&g_firmware_mutex);
    oal_print_err("firmware_download_etc::succ! \n");

    return OAL_SUCC;

download_failed:
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    (osal_void)memset_s(g_fw_data_buf, g_fw_data_buf_len, 0, g_fw_data_buf_len);
#else
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_kfree(g_fw_data_buf);
#elif defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    free(g_fw_data_buf);
#endif
    g_fw_data_buf = NULL;
#endif
    osal_mutex_unlock(&g_firmware_mutex);
    oal_print_err("download firmware fail\n");
    return -OAL_FAIL;
}

osal_s32 pm_firmware_init(osal_void)
{
    if (osal_mutex_init(&g_firmware_mutex) != OSAL_SUCCESS) {
        oal_print_err("pm_firmware_init: init spin_lock fail!\r\n");
        return OAL_FAIL;
    }
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    g_sha256_data_buf = (osal_char *)plat_sha256_mem_alloc(FIRMWARE_FILESIZE_MAX);
    if (g_sha256_data_buf == NULL) {
        oal_print_err("pm_firmware_init: g_sha256_data_buf malloc fail!\r\n");
        osal_mutex_destroy(&g_firmware_mutex);
        return OAL_FAIL;
    }
    g_fw_data_buf = (osal_u8 *)osal_kmalloc(MAX_FIRMWARE_FILE_TX_BUF_LEN, 0);
    if (g_fw_data_buf == NULL) {
        oal_print_err("pm_firmware_init: g_fw_data_buf malloc fail!\r\n");
        plat_sha256_mem_free(g_sha256_data_buf);
        g_sha256_data_buf = NULL;
        osal_mutex_destroy(&g_firmware_mutex);
        return OAL_FAIL;
    }
    g_fw_data_buf_len = MAX_FIRMWARE_FILE_TX_BUF_LEN;
#endif
    return OAL_SUCC;
}

osal_void pm_firmware_deinit(osal_void)
{
#if defined(CONFIG_SUPPORT_STATIC_FIRMWARE_MEM) && (CONFIG_SUPPORT_STATIC_FIRMWARE_MEM != 0)
    g_fw_data_buf_len = 0;
    if (g_fw_data_buf != NULL) {
        osal_kfree(g_fw_data_buf);
        g_fw_data_buf = NULL;
    }
    if (g_sha256_data_buf != NULL) {
        plat_sha256_mem_free(g_sha256_data_buf);
        g_sha256_data_buf = NULL;
    }
#endif
    osal_mutex_destroy(&g_firmware_mutex);
}
