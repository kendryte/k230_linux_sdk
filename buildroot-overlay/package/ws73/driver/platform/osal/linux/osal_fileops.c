/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/version.h>
#include "soc_osal.h"
#include "securec.h"
#include "osal_errno.h"
#include "osal_inner.h"

char *g_klib_store_path = NULL;

// static mm_segment_t os_get_fs(void)
// {
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
//     return get_fs();
// #else
// #ifdef CONFIG_SET_FS
//     return get_fs();
// #else
//     return force_uaccess_begin();
// #endif
// #endif
// }

// static void os_set_fs(mm_segment_t fs)
// {
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
//     set_fs(fs);
// #else
// #ifdef CONFIG_SET_FS
//     set_fs(fs);
// #else
//     force_uaccess_end(fs);
// #endif
// #endif
// }

// static void os_set_ds(void)
// {
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0))
//     os_set_fs(get_ds());
// #elif defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
//     os_set_fs(KERNEL_DS);
// #else
// #ifdef CONFIG_SET_FS
//     os_set_fs(KERNEL_DS);
// #else
//     force_uaccess_begin();
// #endif
// #endif
// }

static struct file *klib_fopen(const char *file, int flags, int mode)
{
    struct file *filp = filp_open(file, flags, mode);
    return (IS_ERR(filp)) ? NULL : filp;
}

static void klib_fclose(struct file *filp)
{
    if (filp != NULL) {
        filp_close(filp, NULL);
    }
    return;
}

static int klib_fwrite(const char *buf, unsigned long size, struct file *filp)
{
    int writelen;

    if (filp == NULL) {
        return -ENOENT;
    }

    /* 直接使用 kernel_write，无需权限切换 */
    writelen = kernel_write(filp, buf, size, &filp->f_pos);
    return writelen;
}

static int klib_fread(char *buf, unsigned long size, struct file *filp)
{
    int readlen;

    if (filp == NULL) {
        return -ENOENT;
    }

    /* 直接使用 kernel_read，无需权限切换 */
    readlen = kernel_read(filp, buf, size, &filp->f_pos);
    return readlen;
}

void *osal_klib_fopen(const char *file, int flags, int mode)
{
    if (file == NULL) {
        osal_log("file NULL!\n");
        return NULL;
    }

    return (void *)klib_fopen(file, flags, mode);
}
EXPORT_SYMBOL(osal_klib_fopen);

void osal_klib_fclose(void *filp)
{
    if (filp == NULL) {
        osal_log("filp NULL!\n");
        return;
    }

    klib_fclose((struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fclose);

int osal_klib_fwrite(const char *buf, unsigned long size, void *filp)
{
    if ((buf == NULL) || (filp == NULL)) {
        osal_log("buf&filp NULL!\n");
        return OSAL_FAILURE;
    }

    return klib_fwrite(buf, size, (struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fwrite);

int osal_klib_fread(char *buf, unsigned long size, void *filp)
{
    if ((buf == NULL) || (filp == NULL)) {
        osal_log("buf&filp NULL!\n");
        return OSAL_FAILURE;
    }

    return klib_fread(buf, size, (struct file *)filp);
}
EXPORT_SYMBOL(osal_klib_fread);

int osal_klib_fseek(long long offset, int whence, void *filp)
{
    int ret;
    loff_t res;

    if (filp == NULL) {
        osal_log("filp NULL!\n");
        return OSAL_FAILURE;
    }

    res = vfs_llseek(filp, offset, whence);
    ret = (int)res;
    if (res != (loff_t)ret) {
        ret = OSAL_EOVERFLOW;
    }

    return ret;
}
EXPORT_SYMBOL(osal_klib_fseek);

void osal_klib_fsync(void *filp)
{
    if (filp == NULL) {
        osal_log("filp NULL!\n");
        return;
    }

    vfs_fsync(filp, 0);
}
EXPORT_SYMBOL(osal_klib_fsync);

void osal_klib_set_store_path(char *path)
{
    g_klib_store_path = path;
}
EXPORT_SYMBOL(osal_klib_set_store_path);

int osal_klib_get_store_path(char *path, unsigned int path_size)
{
    int len;

#if defined (CONFIG_CLOSE_UART0) || defined (ANDROID_BUILD_USER)
    /* not support log when uart close or user version */
    if (osal_get_buildvariant() != OSAL_BUILDVARIANT_ENG) {
        return OSAL_SUCCESS;
    }
#endif

    if ((path == NULL) || (g_klib_store_path == NULL)) {
        osal_log("path Or g_klib_store_path NULL!\n");
        return OSAL_FAILURE;
    }

    len = strlen(g_klib_store_path) + 1;
    if (len > (int)path_size || path_size <= 1) {
        osal_log("path_size(%u) unvaild! len= %d.\n", path_size, len);
        return OSAL_FAILURE;
    }

    return memcpy_s(path, path_size, g_klib_store_path, len);
}
EXPORT_SYMBOL(osal_klib_get_store_path);
#if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
