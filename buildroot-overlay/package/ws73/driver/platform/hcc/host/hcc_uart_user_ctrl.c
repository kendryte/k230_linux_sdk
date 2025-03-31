/**
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 *
 * Description: UART octty Function Implementation
 * Author: @CompanyNameTag
 */

#ifdef CONFIG_HCC_SUPPORT_UART
/*****************************************************************************
  1 Include Head file
*****************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include "oneimage.h"
#include "securec.h"
#include "hcc_uart_host.h"
#include "hcc_uart_user_ctrl.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
/*****************************************************************************
  2 Define global variable
*****************************************************************************/
struct kobject *g_sysfs_ws73_bsle = NULL;

/*****************************************************************************
  3 Function implement
*****************************************************************************/
struct kobject *hcc_get_sys_object(td_void)
{
    return g_sysfs_ws73_bsle;
}
td_bool g_octty_started = TD_FALSE;

td_bool is_occtty_ready(td_void)
{
    return g_octty_started;
}
/* called by octty from hal to decide open or close uart */
static ssize_t show_install(struct kobject *kobj, struct kobj_attribute *attr, td_s8 *buf)
{
    static td_u8 cnt = 0;
    hcc_uart_priv_data *uart_priv = NULL;
    hcc_printf_err_log("in %s\n", __func__);
    if (NULL == buf) {
        hcc_printf_err_log("buf is NULL\n");
        return EXT_ERR_FAILURE;
    }

    uart_priv = hcc_get_uart_priv_data();
    if (NULL == uart_priv) {
        hcc_printf_err_log("uart_priv is NULL!\n");
        return EXT_ERR_FAILURE;
    }
    if (++cnt > 1) {
        g_octty_started = TD_TRUE;
    }
    return snprintf_s(buf, SNPRINT_LIMIT_TO_KERNEL, SNPRINT_LIMIT_TO_KERNEL, "%d\n", uart_priv->ldisc_install);
}

/* read by octty from hal to decide open which uart */
static ssize_t show_dev_name(struct kobject *kobj, struct kobj_attribute *attr, td_s8 *buf)
{
    hcc_uart_priv_data *uart_priv = NULL;
    hcc_printf_err_log("in %s\n", __func__);
    if (NULL == buf) {
        hcc_printf_err_log("buf is NULL\n");
        return EXT_ERR_FAILURE;
    }

    uart_priv = hcc_get_uart_priv_data();
    if (NULL == uart_priv) {
        hcc_printf_err_log("uart_priv is NULL!\n");
        return EXT_ERR_FAILURE;
    }
    return snprintf_s(buf, SNPRINT_LIMIT_TO_KERNEL, SNPRINT_LIMIT_TO_KERNEL, "%s\n", uart_priv->dev_name);
}

/* read by octty from hal to decide what baud rate to use */
static ssize_t show_baud_rate(struct kobject *kobj, struct kobj_attribute *attr, td_s8 *buf)
{
    hcc_uart_priv_data *uart_priv = NULL;
    hcc_printf_err_log("in %s\n", __func__);
    if (NULL == buf) {
        hcc_printf_err_log("buf is NULL\n");
        return EXT_ERR_FAILURE;
    }

    uart_priv = hcc_get_uart_priv_data();
    if (NULL == uart_priv) {
        hcc_printf_err_log("uart_priv is NULL!\n");
        return EXT_ERR_FAILURE;
    }

    return snprintf_s(buf, SNPRINT_LIMIT_TO_KERNEL, SNPRINT_LIMIT_TO_KERNEL, "%d\n", uart_priv->baud_rate);
}

/* read by octty from hal to decide whether or not use flow cntrl */
static ssize_t show_flow_cntrl(struct kobject *kobj, struct kobj_attribute *attr, td_s8 *buf)
{
    hcc_uart_priv_data *uart_priv = NULL;
    hcc_printf_err_log("in %s\n", __func__);

    if (NULL == buf) {
        hcc_printf_err_log("buf is NULL\n");
        return EXT_ERR_FAILURE;
    }

    uart_priv = hcc_get_uart_priv_data();
    if (NULL == uart_priv) {
        hcc_printf_err_log("uart_priv is NULL!\n");
        return EXT_ERR_FAILURE;
    }

    return snprintf_s(buf, SNPRINT_LIMIT_TO_KERNEL, SNPRINT_LIMIT_TO_KERNEL, "%d\n", uart_priv->flow_cntrl);
}

static struct kobj_attribute ldisc_install =
__ATTR(install, 0444, (void *)show_install, NULL);

static struct kobj_attribute uart_dev_name =
__ATTR(dev_name, 0444, (void *)show_dev_name, NULL);

static struct kobj_attribute uart_baud_rate =
__ATTR(baud_rate, 0444, (void *)show_baud_rate, NULL);

static struct kobj_attribute uart_flow_cntrl =
__ATTR(flow_cntrl, 0444, (void *)show_flow_cntrl, NULL);

static struct attribute *bsle_attrs[] = {
    &ldisc_install.attr,
    &uart_dev_name.attr,
    &uart_baud_rate.attr,
    &uart_flow_cntrl.attr,
    NULL,
};

static struct attribute_group bsle_attr_grp = {
    .attrs = bsle_attrs,
};

td_s32 bsle_user_ctrl_init(void)
{
    td_s32 status;

    g_sysfs_ws73_bsle = kobject_create_and_add("ws73_ps", NULL);
    if (NULL == g_sysfs_ws73_bsle) {
        hcc_printf_err_log("Failed to creat g_sysfs_hi110x_ps !!!\n ");
        return -EFAULT;
    }

    status = sysfs_create_group(g_sysfs_ws73_bsle, &bsle_attr_grp);
    if (status) {
        hcc_printf_err_log("failed to create g_sysfs_ws73_bsle sysfs entries\n");
        kobject_put(g_sysfs_ws73_bsle);
        return -EFAULT;
    }

    return 0;
}

void bfgx_user_ctrl_exit(void)
{
    if (g_sysfs_ws73_bsle != NULL) {
        sysfs_remove_group(g_sysfs_ws73_bsle, &bsle_attr_grp);
        kobject_put(g_sysfs_ws73_bsle);
    }
}

#endif
#endif