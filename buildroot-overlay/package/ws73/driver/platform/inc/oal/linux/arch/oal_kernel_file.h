/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: oal kernel file header
 * Author: Huanghe
 * Create: 2021-08-06
 */

#ifndef __OAL_LINUX_KERNEL_FILE_H__
#define __OAL_LINUX_KERNEL_FILE_H__

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

typedef struct file     oal_file;
// typedef mm_segment_t    oal_mm_segment_t;

// OAL_STATIC OAL_INLINE oal_mm_segment_t oal_get_fs(oal_void)
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

// OAL_STATIC OAL_INLINE oal_void oal_set_fs(oal_mm_segment_t fs)
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

OAL_STATIC OAL_INLINE oal_int oal_kernel_file_close(oal_file *pst_file)
{
    return filp_close(pst_file,NULL);
}

OAL_STATIC OAL_INLINE oal_int oal_sysfs_create_group(struct kobject *kobj,
                     const struct attribute_group *grp)
{
    return sysfs_create_group(kobj, grp);
}

OAL_STATIC OAL_INLINE oal_void oal_sysfs_remove_group(struct kobject *kobj,
                      const struct attribute_group *grp)
{
    sysfs_remove_group(kobj, grp);
}

OAL_STATIC OAL_INLINE int oal_debug_sysfs_create_group(struct kobject *kobj,
                     const struct attribute_group *grp)
{
    return sysfs_create_group(kobj, grp);
}

OAL_STATIC OAL_INLINE oal_void oal_debug_sysfs_remove_group(struct kobject *kobj,
                      const struct attribute_group *grp)
{
    sysfs_remove_group(kobj, grp);
}
#ifdef _PRE_CONFIG_CONN_EXT_SYSFS_SUPPORT
extern oal_kobject* oal_get_sysfs_root_object_etc(oal_void);
extern oal_void oal_put_sysfs_root_object_etc(oal_void);
extern oal_kobject* oal_get_sysfs_root_boot_object_etc(oal_void);
extern oal_kobject* oal_conn_sysfs_root_obj_init_etc(oal_void);
extern oal_void oal_conn_sysfs_root_obj_exit_etc(oal_void);
extern oal_void oal_conn_sysfs_root_boot_obj_exit_etc(oal_void);
#endif
extern oal_file* oal_kernel_file_open_etc(oal_uint8 *file_path,oal_int32 ul_attribute);
extern loff_t oal_kernel_file_size_etc(oal_file *pst_file);
extern ssize_t oal_kernel_file_read_etc(oal_file *pst_file, oal_uint8 *pst_buff, loff_t ul_fsize);
extern oal_int oal_kernel_file_write_etc(oal_file *pst_file,oal_uint8 *pst_buf,loff_t fsize);
extern oal_int oal_kernel_file_print_etc(oal_file *pst_file,const oal_int8 *pc_fmt,...);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of __OAL_LINUX_KERNEL_FILE_H__ */
