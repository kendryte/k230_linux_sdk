/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: CRC16 table
 * Date: 2021-04-13
 */

#include "zdiag_lib_dbg.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "oal_types.h"
#include "oal_ext_util.h"
#include "oal_ext_if.h"
#include "zdiag_filter.h"
#include "zdiag_tx_proc.h"
#include "zdiag_adapt_os.h"
#include "zdiag_local_log.h"
#if defined(CONFIG_DIAG_SUPPORT_UART) && (CONFIG_DIAG_SUPPORT_UART == 1)
#include "zdiag_linux_uart.h"
#endif
#include "plat_misc.h"

#define ZDIAG_FILTER_MEMBER_PRINT_NUM 8

#ifdef CONFIG_DFX_SUPPORT_DEBUG
td_u8 g_zdiag_dfx_dbg_lvl = PLAT_LOG_ERR;
#endif

/* debug sysfs */
typedef struct kobject osal_kobject;

OSAL_STATIC oal_kobject *g_zdiag_dbg_syfs_object = OSAL_NULL;

#ifdef CONFIG_DFX_SUPPORT_DEBUG
td_u8 zdiag_dfx_dbg_level_get(void)
{
    return g_zdiag_dfx_dbg_lvl;
}

void zdiag_dfx_dbg_level_set(td_u8 log_level)
{
    g_zdiag_dfx_dbg_lvl = log_level;
}
#endif

OAL_STATIC osal_s32 zdiag_param_ptr_is_null(const void *ptr)
{
    return (((ptr) != OSAL_NULL) ? 1 : 0);
}

OAL_STATIC ssize_t zdiag_dbg_filter_info(char *buf, size_t buf_len)
{
    zdiag_ctx_stru *ctx = zdiag_get_ctx();
    return snprintf_s(buf, buf_len, buf_len,
#if defined(CONFIG_DIAG_SUPPORT_UART) && (CONFIG_DIAG_SUPPORT_UART == 1)
        "zdiag hso conn state: %u[2:uart,1:sock,0:off]\n"
        "zdiag uart rx bytes: %u\n"
        "zdiag uart tx state: %u\n"
#endif
        "zdiag_filter.enable: %u\n"
        "zdiag_filter.level: 0x%x\n"
        "zdiag_filter.tool_addr: %u\n"
        "zdiag_filter.last_rev_time: %u\n"
        "zdiag_filter.enable_id GROUP_NUM:%d\n"
        "zdiag tx_que_icnt:%u\n"
        "zdiag tx_que_ocnt:%u\n"
        "zdiag tx_que_len:%u\n"
        "zdiag tx_fail_cnt:%u\n"
        "zdiag tx_succ_cnt:%u\n"
        "zdiag tx_all_cnt:%u\n",
#if defined(CONFIG_DIAG_SUPPORT_UART) && (CONFIG_DIAG_SUPPORT_UART == 1)
        (td_u32)diag_hso_conn_state_get(),
        zdiag_uart_get_ctrl()->uart_rx_cnt,
        zdiag_uart_get_ctrl()->uart_tx_cnt,
#endif
        (td_u32)zdiag_get_filter_ctrl()->enable,
        (td_u32)zdiag_get_filter_ctrl()->level,
        (td_u32)zdiag_get_filter_ctrl()->tool_addr,
        zdiag_get_filter_ctrl()->last_rev_time,
        ZDIAG_FILTER_GROUP_NUM,
        ctx->tx_queue->icnt,
        ctx->tx_queue->ocnt,
        ctx->tx_queue->len,
        ctx->tx_fail_cnt,
        ctx->tx_succ_cnt,
        ctx->tx_all_cnt);
}

OAL_STATIC ssize_t zdiag_dbg_get_filter_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;
    size_t buf_len = PAGE_SIZE;
    td_u16 i;

    if ((dev == NULL) || (attr == NULL) || (buf == NULL))  {
        oal_print_err(KERN_ERR "buf ?null:%d, attr ?null:%d, buf ?null:%d. \n", zdiag_param_ptr_is_null(dev),
            zdiag_param_ptr_is_null(attr), zdiag_param_ptr_is_null(buf));
        return 0;
    }
    ret = zdiag_dbg_filter_info(buf, buf_len);
    if (ret < 0) {
        return ret;
    }
    for (i = 0; i < ZDIAG_FILTER_GROUP_NUM; ++i) {
        ret += snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "[%u]: %2u ",
            i, zdiag_get_filter_ctrl()->enable_id[i]);
        if (ret < 0) {
            return ret;
        }
        if ((i != 0) && (i % ZDIAG_FILTER_MEMBER_PRINT_NUM) == 0) {
            ret += snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "\n");
            if (ret < 0) {
                return ret;
            }
        }
    }
    ret += snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "\n");
    return ret;
}

OAL_STATIC DEVICE_ATTR(zdiag_filter_info, S_IRUGO, zdiag_dbg_get_filter_info, NULL);

OAL_STATIC ssize_t zdiag_dbg_get_log_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
    size_t buf_len = PAGE_SIZE;

    if ((dev == NULL) || (attr == NULL) || (buf == NULL))  {
        oal_print_err(KERN_ERR "buf ?null:%d, attr ?null:%d, buf ?null:%d. \n", zdiag_param_ptr_is_null(dev),
            zdiag_param_ptr_is_null(attr), zdiag_param_ptr_is_null(buf));
        return 0;
    }

    return snprintf_s(buf, buf_len, buf_len, "%d\n",
        zdiag_log_mode_get());
}

OAL_STATIC ssize_t zdiag_dbg_set_log_mode(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    osal_u32 val;
    int ret = 0;

    if ((dev == NULL) || (attr == NULL) || (buf == NULL))  {
        oal_print_err(KERN_ERR "dev[?null:%d] attr[?null:%d] buf[?null:%d]\n", zdiag_param_ptr_is_null(dev),
            zdiag_param_ptr_is_null(attr), zdiag_param_ptr_is_null(buf));
        return 0;
    }

    ret = sscanf_s(buf, "%u", &val);
    if (ret != 1) {
        oal_print_err(KERN_ERR "set log mode val need one char!\n");
        return 0;
    }

    oal_print_err(KERN_ERR " count:%zu, ret:%d, set log mode:%d\n", count, ret, val);

    zdiag_log_mode_set(val);

    return count;
}

OAL_STATIC DEVICE_ATTR(zdiag_log_mode, S_IRUGO, zdiag_dbg_get_log_mode, zdiag_dbg_set_log_mode);

#ifdef CONFIG_DFX_SUPPORT_DEBUG
OAL_STATIC ssize_t zdiag_dbg_get_print_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    size_t buf_len = PAGE_SIZE;

    if ((dev == NULL) || (attr == NULL) || (buf == NULL))  {
        oal_print_err(KERN_ERR "buf ?null:%d, attr ?null:%d, dev ?null:%d. \n", zdiag_param_ptr_is_null(buf),
            zdiag_param_ptr_is_null(attr), zdiag_param_ptr_is_null(dev));
        return 0;
    }

    return snprintf_s(buf, buf_len, buf_len, "local_log_dbg_level: %u\n",
        zdiag_dfx_dbg_level_get());
}

OAL_STATIC ssize_t zdiag_dbg_set_print_level(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count)
{
    osal_u32 val;
    int ret = 0;

    if ((dev == NULL) || (attr == NULL) || (buf == NULL))  {
        oal_print_err(KERN_ERR "buf ?null:%d, attr ?null:%d, buf ?null:%d. \n", zdiag_param_ptr_is_null(dev),
            zdiag_param_ptr_is_null(attr), zdiag_param_ptr_is_null(buf));
        return 0;
    }

    ret = sscanf_s(buf, "%u", &val);
    if (ret != 1) {
        oal_print_err(KERN_ERR " set value one char!\n");
        return 0;
    }

    oal_print_err(KERN_ERR " count:%zu, ret:%d, val:%d\n", count, ret, val);

    zdiag_dfx_dbg_level_set((td_u8)val);

    return count;
}

OAL_STATIC DEVICE_ATTR(zdiag_dbg_print_level, S_IRUGO, zdiag_dbg_get_print_level,
    zdiag_dbg_set_print_level);
#endif

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
OAL_STATIC ssize_t zdiag_dbg_get_chip_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    #define RANDOM_NUM 65
    td_u8 random[RANDOM_NUM];
    ssize_t ret = 0;
    size_t buf_len = PAGE_SIZE;
    td_u16 i;
    td_u32 ret_code;

    if ((dev == NULL) || (attr == NULL) || (buf == NULL))  {
        oal_print_err(KERN_ERR "buf ?null:%d, attr ?null:%d, buf ?null:%d. \n", zdiag_param_ptr_is_null(dev),
            zdiag_param_ptr_is_null(attr), zdiag_param_ptr_is_null(buf));
        return 0;
    }

    ret_code = uapi_drv_cipher_trng_get_random_bytes((td_u8 *)random, sizeof(random));
    ret += snprintf_s(buf + ret, buf_len - ret, buf_len - ret,
        "uapi_drv_cipher_trng_get_random_bytes NUM:[%d], ret_code:[%d]\ndata:", RANDOM_NUM, ret_code);
    if (ret_code != EXT_ERR_SUCCESS || ret < 0) {
        return ret;
    }

    for (i = 0; i < RANDOM_NUM; ++i) {
        if ((i != 0) && (i % ZDIAG_FILTER_MEMBER_PRINT_NUM) == 0) {
            ret += snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "\n");
            if (ret < 0) {
                return ret;
            }
        }

        ret += snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "[%2u]: %02X ", i, random[i]);
        if (ret < 0) {
            return ret;
        }
    }
    ret += snprintf_s(buf + ret, buf_len - ret, buf_len - ret, "\n");
    return ret;
}
#endif

#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
OAL_STATIC DEVICE_ATTR(zdiag_chip_info, S_IRUGO, zdiag_dbg_get_chip_info, NULL);
#endif

OAL_STATIC struct attribute *g_zdiag_sysfs_entries[] = {
    &dev_attr_zdiag_filter_info.attr,
    &dev_attr_zdiag_log_mode.attr,
#ifdef CONFIG_DFX_SUPPORT_DEBUG
    &dev_attr_zdiag_dbg_print_level.attr,
#endif
#ifdef CONFIG_PLAT_TRNG_TRIG_RPT_TEST
    &dev_attr_zdiag_chip_info.attr,
#endif
    NULL
};

OAL_STATIC struct attribute_group g_zdiag_attr_group = {
    .attrs = g_zdiag_sysfs_entries,
};

osal_s32 zdiag_dbg_sysfs_init(osal_void)
{
    osal_s32 ret;
    osal_kobject *root_object = NULL;

    g_zdiag_dbg_syfs_object = kobject_create_and_add("zdiag", root_object);
    if (g_zdiag_dbg_syfs_object == OSAL_NULL) {
        oal_print_err(KERN_ERR "{zdiag_dbg_sysfs_init::create zdiag object failed!}");
        return -OAL_EFAIL;
    }

    ret = sysfs_create_group(g_zdiag_dbg_syfs_object, &g_zdiag_attr_group);
    if (ret) {
        kobject_put(g_zdiag_dbg_syfs_object);
        oal_print_err(KERN_ERR "{zdiag_dbg_sysfs_init::sysfs create group failed!}");
        return ret;
    }

    return OAL_SUCC;
}

osal_s32 zdiag_dbg_sysfs_exit(osal_void)
{
    if (g_zdiag_dbg_syfs_object) {
        sysfs_remove_group(g_zdiag_dbg_syfs_object, &g_zdiag_attr_group);
        kobject_put(g_zdiag_dbg_syfs_object);
    }
    return OAL_SUCC;
}