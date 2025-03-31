/*
 * Copyright (c) CompanyNameMagicTag 2012-2023. All rights reserved.
 * Description: oal_kernel_file.h 的头文件
 */

#ifndef OAL_LINUX_KERNEL_FILE_H
#define OAL_LINUX_KERNEL_FILE_H

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OAL_KERNEL_DS           KERNEL_DS

/* 文件属性 */
#define OAL_O_ACCMODE           O_ACCMODE
#define OAL_O_RDONLY            O_RDONLY
#define OAL_O_WRONLY            O_WRONLY
#define OAL_O_RDWR              O_RDWR
#define OAL_O_CREAT             O_CREAT
#define OAL_O_TRUNC             O_TRUNC
#define OAL_O_APPEND            O_APPEND

#define OAL_PRINT_FORMAT_LENGTH     200                     /* 打印格式字符串的最大长度 */

typedef struct file             oal_file;
// typedef mm_segment_t            oal_mm_segment_t;

// OAL_STATIC OAL_INLINE oal_mm_segment_t oal_get_fs(osal_void)
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

// OAL_STATIC OAL_INLINE osal_void oal_set_fs(oal_mm_segment_t fs)
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

// OAL_STATIC OAL_INLINE osal_void oal_set_ds(osal_void)
// {
// #if defined(LINUX_VERSION_CODE) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
//     oal_set_fs(get_ds());
// #else
// #ifdef CONFIG_SET_FS
//     oal_set_fs(KERNEL_DS);
// #else
//     force_uaccess_begin();
// #endif
// #endif
// }

OAL_STATIC OAL_INLINE int oal_debug_sysfs_create_group(struct kobject *kobj,
    const struct attribute_group *grp)
{
    return sysfs_create_group(kobj, grp);
}

OAL_STATIC OAL_INLINE osal_void oal_debug_sysfs_remove_group(struct kobject *kobj,
    const struct attribute_group *grp)
{
    sysfs_remove_group(kobj, grp);
}
#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
extern oal_kobject *oal_get_sysfs_root_object_etc(osal_void);
extern osal_void oal_put_sysfs_root_object_etc(osal_void);
extern oal_kobject *oal_get_sysfs_root_boot_object_etc(osal_void);
extern oal_kobject *oal_conn_sysfs_root_obj_init_etc(osal_void);
extern osal_void oal_conn_sysfs_root_obj_exit_etc(osal_void);
extern osal_void oal_conn_sysfs_root_boot_obj_exit_etc(osal_void);
#endif
extern oal_file *oal_kernel_file_open_etc(osal_u8 *file_path, osal_s32 attribute);
extern loff_t oal_kernel_file_size_etc(oal_file *file);
extern ssize_t oal_kernel_file_read_etc(oal_file *file, osal_u8 *buff, loff_t fsize);
extern osal_slong oal_kernel_file_write_etc(oal_file *file, osal_u8 *buf, loff_t fsize);
extern osal_slong oal_kernel_file_print_etc(oal_file *file, const osal_s8 *pc_fmt, ...);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_main */
