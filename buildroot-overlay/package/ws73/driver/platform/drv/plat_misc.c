/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: plat misc driver.
 * Author: CompanyName
 * Create: 2022-11-29
 */
#include <soc_osal.h>
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
#include <linux/random.h>
#include "oal_kernel_file.h"
#endif
#include "oal_debug.h"
#include "hcc_service.h"
#include "hcc.h"
#include "plat_hcc_srv.h"
#include "efuse_opt.h"
#include "mac_addr.h"
#include "tsensor.h"
#include "plat_pm_dfr.h"
#include "plat_pm_wlan.h"
#include "plat_pm_board.h"
#include "plat_misc.h"

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
#define PLAT_TRNG_RANDOM_TIME_OUT_MS 300

static plat_msg_ctl_struct g_random_st = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};

#endif
#define PLAT_H2D_TIME_OUT_MS 300
osal_s32 plat_read_wait_cond(const void *param)
{
    const plat_msg_ctl_struct* p_read_st = (const plat_msg_ctl_struct *)param;
    if (!p_read_st) {
        return OSAL_TRUE;
    }
    return (p_read_st->wait_cond == OSAL_TRUE);
}
// output_data:[0], result [1], readback_data_len (result+2) data area
#define READ_DATA_HEAD_LEN 8
osal_u32 send_message_to_device(plat_data_len_struct *input_data, plat_cfg_h2d_msg_type msg_type,
    plat_msg_ctl_struct *ctl_struct, plat_data_len_struct *output_data)
{
    td_s32 ret = EXT_ERR_FAILURE;
    td_u8 retry_time = 3;     // 3 retry 3 times

    if (input_data->data == TD_NULL || input_data->data_len == 0) {
        return EXT_ERR_TRNG_INVALID_PARAMETER;
    }

    ctl_struct->wait_cond = OSAL_FALSE;
    while ((ret != EXT_ERR_SUCCESS) && (retry_time > 0)) {
        ret = (td_u32)plat_msg_hcc_send(input_data->data, input_data->data_len, msg_type);
        if (ret != EXT_ERR_SUCCESS) {
            oal_print_err("host_read_efuse fail %d!\r\n", ret);
            retry_time--;
        }
    }

    ret = osal_wait_timeout_interruptible(&(ctl_struct->wait_r),
        plat_read_wait_cond, ctl_struct, PLAT_H2D_TIME_OUT_MS);
    if (ret == 0) {
        return EXT_ERR_TRNG_GET_TIMEOUT;
    } else if (ret < 0) {
        return EXT_ERR_TRNG_INVALID_PARAMETER;
    }
    if (ctl_struct->data_size == 0) {
        return EXT_ERR_FAILURE;
    }

    if (output_data->data != NULL && output_data->data_len > 0) {
        ret = memcpy_s(output_data->data, output_data->data_len, ctl_struct->data + READ_DATA_HEAD_LEN,
            *(osal_u32 *)(ctl_struct->data + sizeof(osal_u32)));
        if (ret != EOK) {
            oal_print_err("send_message_to_device memcpy_s failed\n");
            return EXT_ERR_MEMCPYS_FAIL;
        }
    }
    return EXT_ERR_SUCCESS;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
td_slong vfs_ioctl(struct file *filp, td_u32 cmd, td_ulong arg)
{
    td_s32 error = -ENOTTY;

    if (!filp->f_op->unlocked_ioctl) {
        goto out;
    }

    error = filp->f_op->unlocked_ioctl(filp, cmd, arg);
    if (error == -ENOIOCTLCMD)
        error = -ENOTTY;
    out:
    return error;
}

/* 写/dev/random中的随机数 */
td_s32 write_random_fd(td_u8 *data, td_u16 len)
{
    td_s32 ret;
    struct rand_pool_info* pool;
    struct file *fp;

    pool = (struct rand_pool_info *)osal_kmalloc((len + sizeof(struct rand_pool_info)), OSAL_GFP_KERNEL);
    if (pool == OSAL_NULL) {
        return EXT_ERR_MALLOC_FAILURE;
    }

    fp = filp_open("/dev/random", O_RDWR | O_NONBLOCK | O_APPEND, 0200); // 0200：只写权限
    if (!(fp) || IS_ERR(fp)) {
        fp = OSAL_NULL;
        osal_printk("open /dev/random failed");
        osal_kfree(pool);
        return EXT_ERR_FAILURE;
    }

    pool->buf_size = len;
    pool->entropy_count = 8 * pool->buf_size; // 一字节8位
    ret = memcpy_s(&pool->buf, len, data, len);
    if (ret != EOK) {
        osal_kfree(pool);
        filp_close(fp, NULL);
        return ret;
    }
    ret = vfs_ioctl(fp, RNDADDENTROPY, (unsigned long)(uintptr_t)pool);
    if (ret != 0) {
        osal_kfree(pool);
        filp_close(fp, NULL);
        osal_printk("ioctl fail\n");
        return ret;
    }

    filp_close(fp, NULL);
    osal_kfree(pool);
    return EXT_ERR_SUCCESS;
}
#endif

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
/*******************************************************************************
  函 数 名		: plat_trng_get_random_bytes
  功能描述		: plat获取trngs Random Data Bytes
  输入参数		:  size: get bytes number
  输出参数		:  random: trng random data, get_size: real trng bytes number;
  返 回 值		:  ret: EXT_ERR_SUCCESS of misc errors
*******************************************************************************/
td_u32 plat_trng_get_random_bytes(td_u8 *randnum, td_u32 size, td_u32 *get_size)
{
    td_u32 ret;
    plat_data_len_struct input_data = {.data = (osal_u8 *)&size, .data_len = (osal_u32)sizeof(size)};
    plat_data_len_struct output_data = {.data = randnum, .data_len = size};

    if (randnum == TD_NULL || size == 0 || get_size == TD_NULL) {
        return EXT_ERR_TRNG_INVALID_PARAMETER;
    }
    ret = send_message_to_device(&input_data, H2D_PLAT_CFG_MSG_SEND_TRNG, &g_random_st, &output_data);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("{plat_trng_get_random_bytes failed!}\r\n");
        return ret;
    }
    *get_size = *(osal_u32 *)(g_random_st.data + sizeof(osal_u32));

    return EXT_ERR_SUCCESS;
}

td_u32 uapi_drv_cipher_trng_get_random_bytes(td_u8 *randnum, td_u32 size)
{
    td_u32 wlen; /* words length */
    td_u32 blen; /* bytes length */
    td_u32 get_size = 0;
    td_u8 ran_data[WORD_WIDTH];
    td_u32 i;
    td_u32 ret;

    if (randnum == TD_NULL || size == 0) {
        return EXT_ERR_TRNG_INVALID_PARAMETER;
    }

    blen = size % WORD_WIDTH;
    wlen = size - blen;
    oal_print_dbg("uapi trng get, blen %d, wlen %d\n", blen, wlen);
    while (wlen > 0) {
        ret = plat_trng_get_random_bytes(randnum, wlen, &get_size);
        if (ret != EXT_ERR_SUCCESS) {
            return ret;
        }
        randnum += get_size;
        wlen -= get_size;
    }

    if (blen == 0) {
        return EXT_ERR_SUCCESS;
    }
    ret = plat_trng_get_random_bytes(ran_data, sizeof(ran_data), &get_size);
    if (ret != EXT_ERR_SUCCESS) {
        return ret;
    }
    for (i = 0; i < blen; i++) {
        randnum[i] = ran_data[i];
    }
    return EXT_ERR_SUCCESS;
}
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(uapi_drv_cipher_trng_get_random_bytes);
#endif
#endif

td_u32 plat_trng_get_random_saved(td_u8 *data, td_u16 len)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
    write_random_fd(data + READ_DATA_HEAD_LEN, *(td_u32 *)(data + sizeof(td_u32)));
#endif

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
    if ((len == 0) || (len > PLAT_TRNG_RANDOM_BYTES_MAX)) {
        g_random_st.data_size = 0;
        return EXT_ERR_FAILURE;
    }

    if (memcpy_s(g_random_st.data, len, data, len) != EOK) {
        g_random_st.data_size = 0;
    } else {
        g_random_st.data_size = len;
        g_random_st.wait_cond = OSAL_TRUE;
        osal_wait_wakeup(&g_random_st.wait_r);
        oal_print_dbg("Wakeup Wait\n");
    }
    oal_print_dbg("[TRNG] D2H data0:0x%02x, size:%d got!\r\n", data[0], g_random_st.data_size);
#endif
    return EXT_ERR_SUCCESS;
}
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define CCPRIV_CMD_NAME_MAX_LEN 80 /* 字符串中每个单词的最大长度(原20) */
#define CONVERT_TO_HEX 16
typedef struct {
    osal_s8 name[CCPRIV_CMD_NAME_MAX_LEN]; /* 命令名 */
    const osal_s8 *param; /* 参数 */
} plat_ccpriv_para;
typedef osal_u32(*plat_ccpriv_cmd_func)(osal_s8 *pc_param);

typedef struct {
    char *cmd_name;    /* 命令字符串 */
    plat_ccpriv_cmd_func func;         /* 命令对应处理函数 */
} plat_ccpriv_cmd_entry_stru;

osal_u32 plat_get_cmd_one_arg(const osal_s8 *cmd, osal_s8 *arg, osal_u32 arg_len, osal_u32 *cmd_offset)
{
    const osal_s8 *cmd_copy = OSAL_NULL;
    osal_u32 pos = 0;

    if ((cmd == OSAL_NULL) || (arg == OSAL_NULL) || (cmd_offset == OSAL_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    cmd_copy = cmd;

    /* 去掉字符串开始的空格 */
    while (*cmd_copy == ' ') {
        ++cmd_copy;
    }

    while ((*cmd_copy != ' ') && (*cmd_copy != '\0')) {
        arg[pos] = *cmd_copy;
        ++pos;
        ++cmd_copy;

        if (pos >= arg_len) {
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    arg[pos]  = '\0';

    /* 字符串到结尾，返回错误码 */
    if (pos == 0) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *cmd_offset = (osal_u32)(cmd_copy - cmd);

    return OAL_SUCC;
}
#define WLAN_MAC_ADDR_LEN 6
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
OAL_STATIC osal_u32  uapi_ccpriv_setmac(osal_s8 *param)
{
    osal_s32 ret = OAL_FAIL;
    osal_u32 arg_ret;
    osal_u32 off_set;
    osal_u8 arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};

    osal_u8 mac[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 data_len;

    arg_ret = plat_get_cmd_one_arg(param, arg, sizeof(arg), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("wal_get_cmd_one_arg return err_code [0x%x]\r\n", arg_ret);
        return arg_ret;
    }
    if ((plat_parse_mac_addr(arg, mac, &data_len, WLAN_MAC_ADDR_LEN) == OAL_SUCC) && (data_len == WLAN_MAC_ADDR_LEN)) {
        ret = efuse_write_dev_addr(mac, WLAN_MAC_ADDR_LEN);
    }
    return ret;
}

#define PID_VID_LEN 16
#define MAX_DATA 0xFFFF
OAL_STATIC osal_u32  uapi_ccpriv_setvidpid(osal_s8 *param)
{
    osal_s32 ret = OAL_FAIL;
    osal_u32 arg_ret;
    osal_u32 off_set;
    osal_u8 arg1[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8 arg2[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u16 pid;
    osal_u16 vid;
    osal_slong data;

    arg_ret = plat_get_cmd_one_arg(param, arg1, sizeof(arg1), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("plat_get_cmd_one_arg return err_code [%d]!\r\n", arg_ret);
        return arg_ret;
    }
    data = oal_strtol(arg1, OAL_PTR_NULL, PID_VID_LEN);
    if (data < 0 || data > MAX_DATA) {
        return OAL_FAIL;
    }
    pid = (osal_u16)data;
    param = param + off_set;
    arg_ret = plat_get_cmd_one_arg(param, arg2, sizeof(arg2), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("plat_get_cmd_one_arg return err_code [%d]!\r\n", arg_ret);
        return arg_ret;
    }
    data = oal_strtol(arg2, OAL_PTR_NULL, PID_VID_LEN);
    if (data < 0 || data > MAX_DATA) {
        return OAL_FAIL;
    }
    vid = (osal_u16)data;
    ret = efuse_write_pid_vid(pid, vid);
    return ret;
}

OAL_STATIC osal_u32  uapi_ccpriv_getvidpid(osal_s8 *param)
{
    osal_s32 ret = OAL_FAIL;
    osal_u16 pid;
    osal_u16 vid;

    ret = uapi_efuse_read(EXT_EFUSE_USB_SDIO_PID_ID, (osal_u8 *)(&pid), sizeof(pid));
    if (ret != OAL_SUCC) {
        oal_print_err("{read pid fail}\r\n");
        return OAL_FAIL;
    }
    ret = uapi_efuse_read(EXT_EFUSE_USB_SDIO1_VID_ID, (osal_u8 *)(&vid), sizeof(vid));
    if (ret != OAL_SUCC) {
        oal_print_err("{read vid fail}\r\n");
        return OAL_FAIL;
    }
    oal_print_production_info("vid = 0x%x, pid = 0x%x\r\n", vid, pid);
    return ret;
}

OAL_STATIC osal_u32  uapi_ccpriv_setproduction_test_flag(osal_s8 *param)
{
    osal_s32 ret = OAL_FAIL;
    osal_efuse_id target_id;
    osal_u8 data;

    if ((uapi_efuse_read(EXT_EFUSE_GROUP_USED_FLAG1_ID, &data, 1) == OAL_SUCC) && (data == 0)) {
        target_id = EXT_EFUSE_GROUP_USED_FLAG1_ID;
    } else if ((uapi_efuse_read(EXT_EFUSE_GROUP_USED_FLAG2_ID, &data, 1) == OAL_SUCC) && (data == 0)) {
        target_id = EXT_EFUSE_GROUP_USED_FLAG2_ID;
    } else {
        return OAL_FAIL;
    }
    data = 1;
    ret = uapi_efuse_write(target_id, &data, 1);
    return ret;
}

OAL_STATIC osal_u32  uapi_ccpriv_set_sparkmac(osal_s8 *param)
{
    osal_s32 ret = OAL_FAIL;
    osal_u32 arg_ret;
    osal_u32 off_set;
    osal_u8 arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};

    osal_u8 mac[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 data_len;

    arg_ret = plat_get_cmd_one_arg(param, arg, sizeof(arg), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("wal_get_cmd_one_arg return err_code [0x%d]!", arg_ret);
        return arg_ret;
    }
    if ((plat_parse_mac_addr(arg, mac, &data_len, WLAN_MAC_ADDR_LEN) == OAL_SUCC) && (data_len == WLAN_MAC_ADDR_LEN)) {
        ret = efuse_write_spark_addr(mac, WLAN_MAC_ADDR_LEN);
    }
    return ret;
}
#endif

#define D2H_MAX_DATA_LEN 10

td_u32 plat_d2h_msg_callback(plat_msg_ctl_struct *msg_ctl, td_u8 *data, td_u16 len)
{
    if ((len == 0) || (len > PLAT_TRNG_RANDOM_BYTES_MAX)) {
        msg_ctl->data_size = 0;
        return EXT_ERR_FAILURE;
    }
    if (memcpy_s(msg_ctl->data, len, data, len) != EOK) {
        msg_ctl->data_size = 0;
    } else {
        msg_ctl->data_size = len;
        msg_ctl->wait_cond = OSAL_TRUE;
        osal_wait_wakeup(&(msg_ctl->wait_r));
    }
    return EXT_ERR_SUCCESS;
}

#ifdef _PRE_WLAN_FEATURE_MFG_TEST
static plat_msg_ctl_struct g_gpio_test_ctl_struct = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};

td_u32 plat_do_gpio_test_callback(td_u8 *data, td_u16 len)
{
    return plat_d2h_msg_callback(&g_gpio_test_ctl_struct, data, len);
}

OAL_STATIC osal_u32  uapi_ccpriv_gpio_test(osal_s8 *param)
{
    osal_s32 ret = OAL_FAIL;
    osal_u32 arg_ret;
    osal_u32 off_set;
    osal_u8 arg[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u8 input_data[3] = {0};
    osal_u8 output_data;
    plat_data_len_struct input_data_st = {.data = input_data, .data_len = (osal_u8)sizeof(input_data)};
    plat_data_len_struct output_data_st = {.data = &output_data, .data_len = 1};

    arg_ret = plat_get_cmd_one_arg(param, arg, sizeof(arg), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("wal_get_cmd_one_arg return err_code [0x%x]!", arg_ret);
        return arg_ret;
    }
    input_data[0] = (osal_u8)oal_strtol(arg, NULL, CONVERT_TO_HEX);    // test type

    param = param + off_set;
    arg_ret = plat_get_cmd_one_arg(param, arg, sizeof(arg), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("wal_get_cmd_one_arg return err_code [0x%x]!", arg_ret);
        return arg_ret;
    }
    input_data[1] = (osal_u8)oal_strtol(arg, NULL, CONVERT_TO_HEX);   // gpio1

    param = param + off_set;
    arg_ret = plat_get_cmd_one_arg(param, arg, sizeof(arg), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("wal_get_cmd_one_arg return err_code [%d]!", arg_ret);
        return arg_ret;
    }
    input_data[2] = (osal_u8)oal_strtol(arg, NULL, CONVERT_TO_HEX);   // gpio2

    ret = send_message_to_device(&input_data_st, H2D_PLAT_CFG_MSG_GPIO_TEST, &g_gpio_test_ctl_struct, &output_data_st);
    if (ret != OAL_SUCC) {
        return ret;
    }
    if (output_data == 0) {
        return OAL_SUCC;
    } else {
        oal_print_production_info("GPIO %d GPIO %d mode %d failed!!!\r\n",
            input_data[1], input_data[2], input_data[0]);     /* 0/1/2是数组下标 */
        return OAL_FAIL;
    }
}
#endif

typedef union {
    osal_u32 ver;
    struct {
        osal_u32 d : 8; /* [ 7,  0] */
        osal_u32 c : 8; /* [15,  8] */
        osal_u32 b : 8; /* [23, 16] */
        osal_u32 a : 8; /* [31, 24] */
    } bits;
} version_u;

typedef union {
    osal_u32 ver;
    struct {
        osal_u32 c : 8; /* [7..0] */
        osal_u32 r : 8; /* [15..8] */
        osal_u32 v : 16; /* [31..16] */
    } bits;
} rom_version_u;

typedef struct {
    version_u device;
    rom_version_u device_rom;
} version_t;
static plat_msg_ctl_struct g_get_version_ctl_struct = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};
OAL_STATIC osal_u32  uapi_ccpriv_get_version(osal_s8 *param)
{
    osal_u32 ret = OAL_FAIL;
    osal_u8 input_data = 1;
    version_t version = {0};
    plat_data_len_struct input_data_st = {.data = &input_data, .data_len = 1};
    plat_data_len_struct output_data_st = {.data = (osal_u8 *)(&version), .data_len = (osal_u32)sizeof(version)};

    ret = send_message_to_device(&input_data_st, H2D_PLAT_CFG_MSG_GET_VERSION, &g_get_version_ctl_struct,
                                 &output_data_st);
    if (ret != OAL_SUCC) {
        return ret;
    }
    oal_print_production_info("SDK_Version:       1.10.110\r\n");
    oal_print_production_info("FirmWare_Version:  %d.%d.%d\r\n",
        version.device.bits.a, version.device.bits.b, version.device.bits.c);
    oal_print_production_info("Chip_Rom_Version: 1.0.0 \r\n");
    oal_print_production_info("SDK_Patch_Version: 0 \r\n");
    return OAL_SUCC;
}

td_u32 plat_get_version_test_callback(td_u8 *data, td_u16 len)
{
    return plat_d2h_msg_callback(&g_get_version_ctl_struct, data, len);
}


static const osal_s8 *g_chip_type_list[CHIP_TYPE_MAX] = {"U", "S", "E"};
static const osal_s8 *g_bus_type_list[BUS_TYPE_MAX] = {"USB", "SDIO", "UART"};

static plat_msg_ctl_struct g_get_chip_type_ctl_struct = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};

OAL_STATIC osal_u32 uapi_ccpriv_get_chip_type(osal_s8 *param)
{
    osal_u32 ret = OAL_FAIL;
    osal_u8 chip_type, bus_type;
    osal_u8 input_data = 0;
    osal_u8 output_data[2] = {0}; // 2: data len
    const osal_s8 *chip_type_str;
    const osal_s8 *bus_type_str;
    plat_data_len_struct input_data_st = {.data = &input_data, .data_len = (osal_u32)sizeof(input_data)};
    plat_data_len_struct output_data_st = {.data = output_data, .data_len = (osal_u32)sizeof(output_data)};

    ret = send_message_to_device(&input_data_st, H2D_PLAT_CFG_MSG_GET_CHIP_TYPE, &g_get_chip_type_ctl_struct,
                                 &output_data_st);
    if (ret != OAL_SUCC) {
        oal_print_err("get chip type fail, ret=[%u].\r\n", ret);
        return ret;
    }

    chip_type = output_data[0];
    bus_type = output_data[1];
    if (chip_type >= CHIP_TYPE_MAX || bus_type >= BUS_TYPE_MAX) {
        oal_print_err("Invalid chip[%u] bus[%u].\r\n", chip_type, bus_type);
        return OAL_FAIL;
    }

    chip_type_str = g_chip_type_list[chip_type];
    bus_type_str = g_bus_type_list[bus_type];
    oal_print_production_info("Chip WS73%s, bus %s.\r\n", chip_type_str, bus_type_str);

    return OAL_SUCC;
}

td_u32 plat_get_chip_type_callback(td_u8 *data, td_u16 len)
{
    return plat_d2h_msg_callback(&g_get_chip_type_ctl_struct, data, len);
}


#define MAX_READ_DATA_LEN 18

OAL_STATIC osal_u32  uapi_ccpriv_read_mac_efuse(osal_s8 *param)
{
    osal_u32 ret;
    osal_u8 index;
    osal_u8 mac_idx = EXT_EFUSE_IPV4_MAC_ADDR_03_ID;
    osal_u8 mac_zero[WLAN_MAC_ADDR_LEN] = {0};
    osal_u8 mac4_addr[WLAN_MAC_ADDR_LEN] = {0};

    /* 3是输入参数个数，input_data[0]表示测试结果，input_data[1]表示一个gpio管脚，input_data[2]表示另一个gpio管脚 */
    for (index = 0; index < 3; index++) {
        if (mac_idx > EXT_EFUSE_IPV4_MAC_ADDR_03_ID || mac_idx < EXT_EFUSE_IPV4_MAC_ADDR_01_ID) {
            oal_print_err("mac_idx error\r\n");
            return OAL_FAIL;
        }
        ret = uapi_efuse_read(mac_idx, mac4_addr, sizeof(mac4_addr));
        if (ret != OAL_SUCC) {
            oal_print_err("uapi_efuse_read_mac fail\r\n");
            return OAL_FAIL;
        }
        if (osal_memcmp(mac_zero, mac4_addr, WLAN_MAC_ADDR_LEN) == 0) {
            mac_idx--;
            continue;
        }
        break;
    }
    /* 1表示mac地址的第二位，2表示mac地址的第三位，3表示mac地址的第四位 */
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    oal_print_production_info("Mac Addr: %02x:%02x:%02x:%02x:%02x:%02x\n", mac4_addr[0], mac4_addr[1],
        mac4_addr[2], mac4_addr[3], mac4_addr[4], mac4_addr[5]);      /* mac地址前0/1/2/3/4/5位 */
#else
    oal_print_production_info("Mac Addr: %02x:%02x:%02x:%02x:**:**\n", mac4_addr[0], mac4_addr[1],
        mac4_addr[2], mac4_addr[3]);      /* mac地址前0/1/2/3位 */
#endif
    return OAL_SUCC;
}

OAL_STATIC osal_u32  uapi_ccpriv_read_sparkmac_efuse(osal_s8 *param)
{
    osal_u32 ret;
    osal_u8 mac_addr[WLAN_MAC_ADDR_LEN] = {0};

    ret = uapi_efuse_read(EXT_EFUSE_SLE_EFUSE_ID, mac_addr, sizeof(mac_addr));
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    /* 1表示mac地址的第二位，2表示mac地址的第三位，3表示mac地址的第四位 */
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    oal_print_production_info("Mac Addr: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1],
        mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);      /* mac地址前0/1/2/3/4/5位 */
#else
    oal_print_production_info("Mac Addr: %02x:%02x:%02x:%02x:**:**\n", mac_addr[0], mac_addr[1],
        mac_addr[2], mac_addr[3]);      /* mac地址前0/1/2/3位 */
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_MFG_TEST
OAL_STATIC osal_u32  uapi_ccpriv_read_dieid_efuse(osal_s8 *param)
{
    osal_s32 ret;
    osal_u8 index;
    osal_u8 chip_id;
    osal_u8 die_id[18] = {0}; // length 18

    ret = uapi_efuse_read(EXT_EFUSE_CHIP_ID, &chip_id, sizeof(chip_id));
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }

    ret = uapi_efuse_read(EXT_EFUSE_DIE_ID, die_id, sizeof(die_id));
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    oal_print_production_info("OK\n");
    oal_print_production_info("CHIP_ID: 0x%02x\n", chip_id);
    oal_print_production_info("DIE_ID: 0x");
    for (index = 0; index < sizeof(die_id); index++) {
        oal_print_production_info("%02x", die_id[index]);
    }
    oal_print_production_info("\n");
    /* 1表示mac地址的第二位，2表示mac地址的第三位，3表示mac地址的第四位 */
    return OAL_SUCC;
}
#endif

OAL_STATIC osal_u32  uapi_ccpriv_read_blemac(osal_s8 *param)
{
    osal_u32 ret;
    osal_u8 pc_addr[WLAN_MAC_ADDR_LEN] = {0};

    ret = get_dev_addr(pc_addr, WLAN_MAC_ADDR_LEN, IFTYPE_BLE);
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    oal_print_production_info("BLE Mac Addr: %02x:%02x:%02x:%02x:%02x:%02x\n", pc_addr[0],
        pc_addr[1], pc_addr[2], pc_addr[3], pc_addr[4], pc_addr[5]);      /* mac地址前0/1/2/3/4/5位 */
#else
    oal_print_production_info("BLE Mac Addr: %02x:%02x:%02x:%02x:**:**\n", pc_addr[0],
        pc_addr[1], pc_addr[2], pc_addr[3]);      /* mac地址前0/1/2/3位 */
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_MFG_TEST
#define MAX_RESERVED_DATA 0xFFFFFFFF
OAL_STATIC osal_u32  uapi_ccpriv_setcustom_reserved_bit(osal_s8 *param)
{
    osal_u32 arg_ret;
    osal_u32 off_set;
    osal_u8 arg1[CCPRIV_CMD_NAME_MAX_LEN] = {0};
    osal_u32 efuse_data;
    osal_slong data;
    osal_u8 locked;
    osal_u32 ret;

    arg_ret = plat_get_cmd_one_arg(param, arg1, sizeof(arg1), &off_set);
    if (arg_ret != OAL_SUCC) {
        oal_print_err("plat_get_cmd_one_arg return err_code [%d]!\r\n", arg_ret);
        return OAL_FAIL;
    }
    data = oal_strtol(arg1, OAL_PTR_NULL, PID_VID_LEN);
    if (data < 0 || data > MAX_RESERVED_DATA) {
        return OAL_FAIL;
    }
    efuse_data = (osal_u32)data;

    if (uapi_efuse_read(EXT_EFUSE_CUSTOM_RESERVE_LOCK_ID, &locked, sizeof(locked)) == OAL_SUCC) {
        if ((locked & 0x1) == 0) {
            ret = uapi_efuse_write(EXT_EFUSE_CUSTOM_RESERVE_ID, (osal_u8 *)(&efuse_data), sizeof(efuse_data));
            if (ret == OAL_SUCC) {
                return uapi_efuse_lock(EXT_EFUSE_CUSTOM_RESERVE_LOCK_ID);
            }
        }
    }
    return OAL_FAIL;
}

OAL_STATIC osal_u32  uapi_ccpriv_getcustom_reserved_bit(osal_s8 *param)
{
    osal_u32 efuse_data;

    if (uapi_efuse_read(EXT_EFUSE_CUSTOM_RESERVE_ID, (osal_u8 *)(&efuse_data), sizeof(efuse_data)) == OAL_SUCC) {
        oal_print_production_info("efuse_data=0x%x\r\n", efuse_data);
        return OAL_SUCC;
    }
    return OAL_FAIL;
}
#endif

#ifdef CONFIG_HCC_SUPPORT_UART
static plat_msg_ctl_struct g_temp_ctl_struct = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};
OAL_STATIC td_u32 plat_get_temp_callback(td_u8 *data, td_u16 len)
{
    return plat_d2h_msg_callback(&g_temp_ctl_struct, data, len);
}
 
plat_msg_ctl_struct *plat_get_temp_ctl_ptr(osal_void)
{
    return &g_temp_ctl_struct;
}
#endif

OAL_STATIC osal_u32 uapi_ccpriv_get_temp(osal_s8 *param)
{
    osal_u32 ret;
    osal_s16 temperature = 0;

    ret = uapi_tsensor_read_temperature(&temperature);
    if (ret != OAL_SUCC) {
        oal_print_err("ERROR\r\n");
        return ret;
    }
    oal_print_production_info("OK\r\n");
    oal_print_production_info("temperature %d\r\n", temperature);
    return OAL_SUCC;
}

#if defined(CONFIG_PLAT_SUPPORT_DFR) && (CONFIG_PLAT_SUPPORT_DFR != 0)
#if defined(CONFIG_PLAT_SUPPORT_DFR_TRIGGER) && (CONFIG_PLAT_SUPPORT_DFR_TRIGGER != 0)
OAL_STATIC osal_u32 uapi_ccpriv_trigger_dfr(osal_s8 *param)
{
    osal_u32 ret;

    ret = plat_exception_reset_process(OSAL_FALSE);
    if (ret != OAL_SUCC) {
        oal_print_production_info("Trigger DFR fail, ret=[%u]\r\n", ret);
    }
    oal_print_production_info("Trigger DFR succ!\r\n");
    return OAL_SUCC;
}
#endif
#endif

#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
OAL_STATIC osal_u32 uapi_ccpriv_detect_device(osal_s8 *param)
{
    osal_u32 ret;

    ret = (osal_u32)uapi_pm_device_detect();
    if (ret != OAL_SUCC) {
        oal_print_err("ERROR\r\n");
        return ret;
    }
    oal_print_production_info("OK\r\n");
    return OAL_SUCC;
}
#endif

td_u32 plat_suspend_system(td_void);
td_u32 plat_resume_system(td_void);
osal_u32 uapi_ccpriv_suspend_system(osal_s8 *pc_param)
{
    osal_u32 ret;

#ifdef CONFIG_HCC_SUPPORT_USB
    hcc_set_exception_status(OSAL_TRUE);
#endif
    ret = plat_suspend_system();
    if (ret == EXT_ERR_SUCCESS) {
        pm_resume_flag_set(OSAL_TRUE);
    }
    return ret;
}

osal_u32 uapi_ccpriv_resume_system(osal_s8 *pc_param)
{
    osal_u32 ret;

    ret = plat_resume_system();
#ifdef CONFIG_HCC_SUPPORT_USB
    hcc_set_exception_status(OSAL_FALSE);
#endif
    return ret;
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(uapi_ccpriv_suspend_system);
EXPORT_SYMBOL(uapi_ccpriv_resume_system);
#endif

OAL_STATIC OAL_CONST plat_ccpriv_cmd_entry_stru g_ast_ccpriv_cmd[] = {
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
            {"set_efuse_mac", uapi_ccpriv_setmac}, // "set_efuse_mac 11,22,33,44,55,66;"
            {"efuse_write_mfg_flag", uapi_ccpriv_setproduction_test_flag},  // "setproduction_test_flag 1/2"
            {"set_sle_mac", uapi_ccpriv_set_sparkmac},  // "set_sle_mac 11,22,33,44,55,66;"
            {"check_gpio", uapi_ccpriv_gpio_test}, // "check_gpio 2 0 3" gpio0,gpio3是否虚焊
            {"get_dieid", uapi_ccpriv_read_dieid_efuse}, // "get_dieid"
            {"efuse_write_vid_pid", uapi_ccpriv_setvidpid},    // "efuse_write_vid_pid 0x1111 0x2222"
            {"get_vid_pid", uapi_ccpriv_getvidpid},    // "get_vid_pid"
            {"set_customer_efuse", uapi_ccpriv_setcustom_reserved_bit},    // "set_customer_efuse 0x11111111"
            {"get_customer_efuse", uapi_ccpriv_getcustom_reserved_bit},    // "get_customer_efuse"
#endif
            {"get_version", uapi_ccpriv_get_version},  // "get_version"
            {"get_chip_type", uapi_ccpriv_get_chip_type},  // "get_chip_type"
            {"efuse_read_mac", uapi_ccpriv_read_mac_efuse}, // "efuse_read_mac"
            {"efuse_read_sle_mac", uapi_ccpriv_read_sparkmac_efuse}, // "efuse_read_sle_mac"
            {"get_ble_mac", uapi_ccpriv_read_blemac},    // "get_ble_mac"
            {"get_temp", uapi_ccpriv_get_temp}, // "get_temp"
#if defined(CONFIG_PLAT_SUPPORT_DFR) && (CONFIG_PLAT_SUPPORT_DFR != 0)
#if defined(CONFIG_PLAT_SUPPORT_DFR_TRIGGER) && (CONFIG_PLAT_SUPPORT_DFR_TRIGGER != 0)
            {"trigger_dfr", uapi_ccpriv_trigger_dfr}, // "trigger_dfr"
#endif
#endif
#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
            {"detect_device", uapi_ccpriv_detect_device}, // "detect_device"
#endif
            {"suspend_system", uapi_ccpriv_suspend_system},
            {"resume_system", uapi_ccpriv_resume_system}
};
OAL_STATIC osal_u32 plat_ccpriv_find_cmd(const osal_s8 *cmd_name, plat_ccpriv_cmd_entry_stru **cmd_id)
{
    osal_u32                cmd_idx;
    int                       l_ret;

    if ((cmd_name == OAL_PTR_NULL) || (cmd_id == OAL_PTR_NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *cmd_id = NULL;
    for (cmd_idx = 0; cmd_idx < (sizeof(g_ast_ccpriv_cmd) / sizeof((g_ast_ccpriv_cmd)[0])); cmd_idx++) {
        l_ret = osal_strcmp((const osal_s8 *)g_ast_ccpriv_cmd[cmd_idx].cmd_name, cmd_name);
        if (l_ret == 0) {
            *cmd_id = (plat_ccpriv_cmd_entry_stru *)&g_ast_ccpriv_cmd[cmd_idx];
            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}
#define CMD_NOT_SUPPORT 0x5a5a5a5a
OAL_STATIC osal_u32 wal_ccpriv_parse_cmd_etc(osal_s8 *pc_cmd)
{
    plat_ccpriv_cmd_entry_stru  *ccpriv_cmd_entry = NULL;
    osal_u32                  off_set = 0;
    osal_u32                  ul_ret;
    plat_ccpriv_para info = {0};

    // get real plat ccpriv order
    pc_cmd += off_set;
    ul_ret = plat_get_cmd_one_arg(pc_cmd, info.name, sizeof(info.name), &off_set);
    if (OAL_SUCC != ul_ret) {
        oal_print_err("get cmd name return err_code [0x%x]!\r\n", ul_ret);
        return ul_ret;
    }
    pc_cmd += off_set;
    /* 根据命令名找到命令枚举 */
    ul_ret = plat_ccpriv_find_cmd((const osal_s8 *)info.name, &ccpriv_cmd_entry);
    if (ul_ret == OAL_SUCC) {
        /* 调用命令对应的函数 */
        ul_ret = ccpriv_cmd_entry->func(pc_cmd);
        if (OAL_SUCC == ul_ret) {
            oal_print_production_info("OK.\n");
        } else {
            oal_print_production_info("ERROR.\n");
        }
        return ul_ret;
    }
    return CMD_NOT_SUPPORT;
}

typedef ssize_t     oal_ssize_t;
typedef size_t      oal_size_t;
typedef struct device_attribute     oal_device_attribute_stru;
typedef struct device               oal_device_stru;
#define OAL_DEVICE_ATTR             DEVICE_ATTR
#define OAL_S_IRUGO                 S_IRUGO
#define OAL_S_IWGRP                 S_IWGRP
#define OAL_S_IWUSR                 S_IWUSR

#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
/*****************************************************************************
 功能描述  : sys write函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 处理字节的长度

 修改历史      :
  1.日    期   : 2014年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
#ifdef CONTROLLER_CUSTOMIZATION
typedef oal_ssize_t (* sys_write_ptr)(struct kobject *kobj, struct kobj_attribute *attr,
    const char *pc_buffer, size_t count);
typedef oal_ssize_t (* sys_read_ptr)(struct kobject *kobj, struct kobj_attribute *attr, char *pc_buffer);
#else
typedef oal_ssize_t (* sys_write_ptr)(oal_device_stru *dev, oal_device_attribute_stru *attr,
    const char *pc_buffer, oal_size_t count);
typedef oal_ssize_t (* sys_read_ptr)(oal_device_stru *dev, oal_device_attribute_stru *attr, char *pc_buffer);
#endif

sys_write_ptr g_sys_write_ptr;
sys_read_ptr g_sys_read_ptr;

oal_void uapi_set_plat_ccpriv_opts(sys_write_ptr write_ptr, sys_read_ptr read_ptr)
{
    g_sys_write_ptr = write_ptr;
    g_sys_read_ptr = read_ptr;
}
EXPORT_SYMBOL(uapi_set_plat_ccpriv_opts);

#define WAL_HIPRIV_CMD_MAX_LEN 140
OAL_STATIC osal_u32 plat_ccpriv_proc(const char *pc_buffer, oal_size_t count)
{
    osal_s8  *pc_cmd;
    osal_u32 ul_ret;
    osal_u32 len = (osal_u32)count;

    if (len > WAL_HIPRIV_CMD_MAX_LEN) {
        oal_print_err("len>WAL_CCPRIV_CMD_MAX_LEN, len [0x%x]!\r\n", len);
        return -OAL_EINVAL;
    }

    pc_cmd = (osal_s8  *)osal_kmalloc(WAL_HIPRIV_CMD_MAX_LEN, OSAL_GFP_KERNEL);
    if (OAL_PTR_NULL == pc_cmd) {
        oal_print_err("alloc mem return null ptr!");
        return -OAL_ENOMEM;
    }

    memset_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, 0, WAL_HIPRIV_CMD_MAX_LEN);
    if (memcpy_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, pc_buffer, len) != EOK) {
        oal_print_err("memcpy_s error\r\n");
    }

    pc_cmd[len - 1] = '\0';

    ul_ret = wal_ccpriv_parse_cmd_etc(pc_cmd);

    osal_kfree(pc_cmd);
    return ul_ret;
}

#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC oal_ssize_t uapi_ccpriv_sys_write(struct kobject *dev, struct kobj_attribute *attr,
    const char *pc_buffer, size_t count)
#else
OAL_STATIC oal_ssize_t uapi_ccpriv_sys_write(oal_device_stru *dev, oal_device_attribute_stru *attr,
    const char *pc_buffer, oal_size_t count)
#endif
{
    if (plat_ccpriv_proc(pc_buffer, count) == CMD_NOT_SUPPORT) {
        if (g_sys_write_ptr != OSAL_NULL) {
            return g_sys_write_ptr(dev, attr, pc_buffer, count);
        }
    }
    return (osal_s32)count;
}

/*****************************************************************************
 功能描述  : sys read函数 空函数；防止编译警告
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 处理字节的长度

 修改历史      :
  1.日    期   : 2014年10月17日
    修改内容   : 新生成函数

*****************************************************************************/
#define SYS_READ_MAX_STRING_LEN (4096-40)   /* 当前命令字符长度20字节内，预留40保证不会超出 */
#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC oal_ssize_t uapi_ccpriv_sys_read(struct kobject *dev, struct kobj_attribute *attr, char *pc_buffer)
#else
OAL_STATIC oal_ssize_t uapi_ccpriv_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr, char *pc_buffer)
#endif
{
    if (g_sys_read_ptr != OSAL_NULL) {
        return g_sys_read_ptr(dev, attr, pc_buffer);
    } else {
        oal_print_err("g_sys_read_ptr==NULL!!!\r\n");
        return 0;
    }
}

struct kobject     *gp_sys_kobject_etc;

OAL_STATIC OAL_DEVICE_ATTR(ccpriv, (OAL_S_IRUGO | OAL_S_IWUSR), uapi_ccpriv_sys_read, uapi_ccpriv_sys_write);
OAL_STATIC struct attribute *ccpriv_sysfs_entries[] = {
    &dev_attr_ccpriv.attr,
    NULL
};

OAL_STATIC struct attribute_group ccpriv_attribute_group = {
    .attrs = ccpriv_sysfs_entries
};

#if defined(CONFIG_PLAT_SUPPORT_DFR) && (CONFIG_PLAT_SUPPORT_DFR != 0)
#if defined(CONFIG_PLAT_SUPPORT_DFR_TRIGGER) && (CONFIG_PLAT_SUPPORT_DFR_TRIGGER != 0)
#define DFR_STATE_STR_LEN 10

#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC oal_ssize_t uapi_dfr_state_sys_read(struct kobject *dev, struct kobj_attribute *attr, char *pc_buffer)
#else
OAL_STATIC oal_ssize_t uapi_dfr_state_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr, char *pc_buffer)
#endif
{
    osal_u32 buff_len = 0;

    oal_print_err("uapi_dfr_state_sys_read enter!\r\n");
    buff_len += snprintf_s(pc_buffer, DFR_STATE_STR_LEN, DFR_STATE_STR_LEN, "%d\n", plat_dfr_status_get());
    return buff_len;
}

OAL_STATIC OAL_DEVICE_ATTR(dfr_state, OAL_S_IRUGO, uapi_dfr_state_sys_read, NULL);
OAL_STATIC struct attribute *dfr_state_sysfs_entries[] = {
    &dev_attr_dfr_state.attr,
    NULL
};

OAL_STATIC struct attribute_group dfr_state_attribute_group = {
    .attrs = dfr_state_sysfs_entries
};
#endif
#endif

#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
#define DEVICE_DETECT_SUCC      0
#define DEVICE_DETECT_FAIL      1
#define DEVICE_DETECT_STR_LEN   10
#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC oal_ssize_t uapi_device_detect_sys_read(struct kobject *dev, struct kobj_attribute *attr, char *pc_buffer)
#else
OAL_STATIC oal_ssize_t uapi_device_detect_sys_read(oal_device_stru *dev, oal_device_attribute_stru *attr,
                                                   char *pc_buffer)
#endif
{
    osal_u32 ret, val;
    osal_u32 buff_len = 0;
    ret = uapi_pm_device_detect();
    val = (ret == OAL_SUCC) ? DEVICE_DETECT_SUCC : DEVICE_DETECT_FAIL;
    buff_len += snprintf_s(pc_buffer, DEVICE_DETECT_STR_LEN, DEVICE_DETECT_STR_LEN, "%d\n", val);
    return buff_len;
}

OAL_STATIC OAL_DEVICE_ATTR(device_detect, OAL_S_IRUGO, uapi_device_detect_sys_read, NULL);
OAL_STATIC struct attribute *device_detect_sysfs_entries[] = {
    &dev_attr_device_detect.attr,
    NULL
};

OAL_STATIC struct attribute_group device_detect_attribute_group = {
    .attrs = device_detect_sysfs_entries
};
#endif
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0))
// use sys filesystem instead
#else
/*****************************************************************************
 功能描述  : proc write函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 处理字节的长度

 修改历史      :
  1.日    期   : 2012年12月10日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 wal_ccpriv_proc_wr(oal_file_stru *file, const osal_s8 *pc_buffer, osal_u32 len,
    osal_void *p_data)
{
    osal_s8                    *pc_cmd;
    osal_u32                  ul_ret;

    if (len > WAL_HIPRIV_CMD_MAX_LEN) {
        oal_print_err("len>WAL_CCPRIV_CMD_MAX_LEN, len [0x%x]!\r\n", len);
        return -OAL_EINVAL;
    }

    pc_cmd = oal_mem_alloc(OAL_MEM_POOL_ID_LOCAL, WAL_HIPRIV_CMD_MAX_LEN, OAL_TRUE);
    if (unlikely(OAL_PTR_NULL == pc_cmd)) {
        oal_print_err("alloc mem return null ptr!\r\n");
        return -OAL_ENOMEM;
    }

    memset_s(pc_cmd, WAL_HIPRIV_CMD_MAX_LEN, 0, WAL_HIPRIV_CMD_MAX_LEN);

    ul_ret = osal_copy_from_user((osal_void *)pc_cmd, pc_buffer, len);
    /* copy_from_user函数的目的是从用户空间拷贝数据到内核空间，失败返回没有被拷贝的字节数，成功返回0 */
    if (ul_ret > 0) {
        oal_print_err("osal_copy_from_user return ul_ret[0x%x]!\r\n", ul_ret);
        oal_mem_free(pc_cmd, OAL_TRUE);

        return -OAL_EFAUL;
    }

    pc_cmd[len - 1] = '\0';

    ul_ret = wal_ccpriv_parse_cmd_etc(pc_cmd);
    if (OAL_SUCC != ul_ret) {
        oal_print_err("parse cmd return err code[0x%x]!\r\n", ul_ret);
    }

    oal_mem_free(pc_cmd, OAL_TRUE);

    return (osal_s32)len;
}
#endif

/*****************************************************************************
 功能描述  : 创建proc入口
 输入参数  : p_proc_arg: 创建proc参数，此处不使用
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2012年12月10日
    修改内容   : 新生成函数

*****************************************************************************/
typedef struct proc_dir_entry       oal_proc_dir_entry_stru;
OAL_STATIC oal_proc_dir_entry_stru *g_pst_proc_entry = OAL_PTR_NULL;
osal_u32 wal_ccpriv_create_proc_etc(osal_void *p_proc_arg)
{
#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
    osal_u32 ret;
#endif

#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0))
    g_pst_proc_entry = OAL_PTR_NULL;
#else
    /* S_IRUSR文件所有者具可读取权限, S_IWUSR文件所有者具可写入权限,
     * S_IRGRP用户组具可读取权限, S_IROTH其他用户具可读取权限 */
    /* 420十进制对应八进制是0644 linux模式定义 S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); */
    g_pst_proc_entry = oal_create_proc_entry(PLAT_HIPRIV_PROC_ENTRY_NAME, 420, NULL);
    if (OAL_PTR_NULL == g_pst_proc_entry) {
        oal_print_err("oal_create_proc_entry return null ptr!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_pst_proc_entry->data  = p_proc_arg;
    g_pst_proc_entry->nlink = 1;        /* linux创建proc默认值 */
    g_pst_proc_entry->read_proc  = OAL_PTR_NULL;

    g_pst_proc_entry->write_proc = (write_proc_t *)wal_ccpriv_proc_wr;
#endif

#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
    gp_sys_kobject_etc = oal_get_sysfs_root_object_etc();
    if (NULL == gp_sys_kobject_etc) {
        oal_print_err("get sysfs root object failed!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = (osal_u32)oal_debug_sysfs_create_group(gp_sys_kobject_etc, &ccpriv_attribute_group);
    if (ret) {
        oal_print_err("ccpriv_attribute_group create failed!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(CONFIG_PLAT_SUPPORT_DFR) && (CONFIG_PLAT_SUPPORT_DFR != 0)
#if defined(CONFIG_PLAT_SUPPORT_DFR_TRIGGER) && (CONFIG_PLAT_SUPPORT_DFR_TRIGGER != 0)
    ret = (osal_u32)oal_debug_sysfs_create_group(gp_sys_kobject_etc, &dfr_state_attribute_group);
    if (ret) {
        oal_print_err("dfr_state_attribute_group create failed!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
#endif

#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
    ret = (osal_u32)oal_debug_sysfs_create_group(gp_sys_kobject_etc, &device_detect_attribute_group);
    if (ret) {
        oal_print_err("device_detect_attribute_group create failed!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 删除proc
 输入参数  : 无
 输出参数  : 无
 返 回 值  : OAL_SUCC

 修改历史      :
  1.日    期   : 2012年12月10日
    修改内容   : 新生成函数

*****************************************************************************/

#define PLAT_HIPRIV_PROC_ENTRY_NAME   "ccpriv"
osal_u32 wal_ccpriv_remove_proc_etc(void)
{
    /* 卸载时删除sysfs */
#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
    if (NULL != gp_sys_kobject_etc) {
#if defined(CONFIG_SUPPORT_UART_DEVICE_DETECT) && (CONFIG_SUPPORT_UART_DEVICE_DETECT != 0)
        oal_debug_sysfs_remove_group(gp_sys_kobject_etc, &device_detect_attribute_group);
#endif
#if defined(CONFIG_PLAT_SUPPORT_DFR) && (CONFIG_PLAT_SUPPORT_DFR != 0)
#if defined(CONFIG_PLAT_SUPPORT_DFR_TRIGGER) && (CONFIG_PLAT_SUPPORT_DFR_TRIGGER != 0)
        oal_debug_sysfs_remove_group(gp_sys_kobject_etc, &dfr_state_attribute_group);
#endif
#endif
        oal_debug_sysfs_remove_group(gp_sys_kobject_etc, &ccpriv_attribute_group);
    }
#endif

    if (g_pst_proc_entry) {
        remove_proc_entry(PLAT_HIPRIV_PROC_ENTRY_NAME, NULL);
        g_pst_proc_entry = OAL_PTR_NULL;
    }

    oal_put_sysfs_root_object_etc();
    return OAL_SUCC;
}

#define UNIUQE_CODE_LENGTH 6
static plat_msg_ctl_struct g_unique_code_ctl_struct = {
    .data_size = 0,
    .wait_cond = OSAL_FALSE,
};

td_u32 uapi_get_unique_code(td_u8 *unique_code)
{
    td_u32 ret;
    osal_u8 in_data = 0;
    plat_data_len_struct input_data = {.data = &in_data, .data_len = sizeof(in_data)};
    plat_data_len_struct output_data = {.data = unique_code, .data_len = UNIUQE_CODE_LENGTH};

    if (unique_code == TD_NULL) {
        return EXT_ERR_FAILURE;
    }
    ret = send_message_to_device(&input_data, H2D_PLAT_CFG_MSG_GET_UNIQUE_CODE, &g_unique_code_ctl_struct,
        &output_data);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("{plat_get_unique_code failed!}\r\n");
        return ret;
    }

    return EXT_ERR_SUCCESS;
}

td_u32 plat_get_unique_code_msg_callback(td_u8 *data, td_u16 len)
{
    return plat_d2h_msg_callback(&g_unique_code_ctl_struct, data, len);
}
#endif

td_void plat_misc_init(td_void)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_u32           ul_ret;
#endif
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
    if (osal_wait_init(&g_random_st.wait_r) != OSAL_SUCCESS) {
        oal_print_err("init osal random wait_read failed\n");
    }
#endif
    oal_print_warning("plat_misc_init!\r\n");
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ul_ret = wal_ccpriv_create_proc_etc(OAL_PTR_NULL);
    if (ul_ret != EXT_ERR_SUCCESS) {
        oal_print_err("wal_ccpriv_create_proc_etc failed ret=0x%x\n", ul_ret);
    }
#ifdef CONFIG_HCC_SUPPORT_UART
    efuse_wait_init();
    if (osal_wait_init(&g_temp_ctl_struct.wait_r) != OSAL_SUCCESS) {
        oal_print_err("g_unique_code_ctl_struct wait_read failed\n");
    }
#endif
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    if (osal_wait_init(&g_gpio_test_ctl_struct.wait_r) != OSAL_SUCCESS) {
        oal_print_err("g_gpio_test_ctl_struct wait_read failed\n");
    }
#endif
    if (osal_wait_init(&g_get_version_ctl_struct.wait_r) != OSAL_SUCCESS) {
        oal_print_err("g_gpio_test_ctl_struct wait_read failed\n");
    }
    if (osal_wait_init(&g_get_chip_type_ctl_struct.wait_r) != OSAL_SUCCESS) {
        oal_print_err("g_gpio_test_ctl_struct wait_read failed\n");
    }
    if (osal_wait_init(&g_unique_code_ctl_struct.wait_r) != OSAL_SUCCESS) {
        oal_print_err("g_unique_code_ctl_struct wait_read failed\n");
    }
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_GPIO_TEST_COMPLETE, (hcc_plat_msg_callbck)plat_do_gpio_test_callback);
#endif
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_GET_VERSION_COMPLETE, (hcc_plat_msg_callbck)plat_get_version_test_callback);
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_GET_CHIP_TYPE_COMPLETE, (hcc_plat_msg_callbck)plat_get_chip_type_callback);
#ifdef CONFIG_HCC_SUPPORT_UART
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_READ_EFUSE_COMPLETE, (hcc_plat_msg_callbck)plat_efuse_get_read_data_saved);
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_WRITE_EFUSE_COMPLETE, (hcc_plat_msg_callbck)plat_efuse_get_write_data_saved);
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_GET_TEMP, (hcc_plat_msg_callbck)plat_get_temp_callback);
#endif
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_GET_UNIQUE_CODE, (hcc_plat_msg_callbck)plat_get_unique_code_msg_callback);
#endif
    hcc_plat_msg_register(D2H_PLAT_CFG_MSG_SEND_TRNG, (hcc_plat_msg_callbck)plat_trng_get_random_saved);
}

td_void plat_misc_exit(td_void)
{
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT
    osal_wait_destroy(&g_random_st.wait_r);
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_WLAN_FEATURE_MFG_TEST
    osal_wait_destroy(&g_gpio_test_ctl_struct.wait_r);
#endif
    osal_wait_destroy(&g_get_chip_type_ctl_struct.wait_r);
    osal_wait_destroy(&g_get_version_ctl_struct.wait_r);
    osal_wait_destroy(&g_unique_code_ctl_struct.wait_r);
#ifdef CONFIG_HCC_SUPPORT_UART
    efuse_wait_destroy();
    osal_wait_destroy(&g_temp_ctl_struct.wait_r);
#endif
    wal_ccpriv_remove_proc_etc();
#endif
}
