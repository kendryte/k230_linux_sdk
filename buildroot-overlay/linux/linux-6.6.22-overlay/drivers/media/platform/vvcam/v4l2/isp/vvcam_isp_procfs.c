/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2023 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include "vvcam_isp_driver.h"
#include "vvcam_isp_procfs.h"

struct vvcam_isp_procfs {
    struct proc_dir_entry *pde;
    struct vvcam_isp_dev *isp_dev;
    struct mutex lock;
};

static int vvcam_isp_procfs_info_show(struct seq_file *sfile, void *offset)
{
	struct vvcam_isp_procfs *isp_proc;
    struct vvcam_isp_dev *isp_dev;
    struct media_pad *pad = NULL;
    int pad_idx = 0;
    int port = -1;

    isp_proc = (struct vvcam_isp_procfs *) sfile->private;

    mutex_lock(&isp_proc->lock);
    isp_dev = isp_proc->isp_dev;

    seq_printf(sfile, "/******sensor configuration******/\n");
    for (pad_idx = 0; pad_idx < VVCAM_ISP_PAD_NR; pad_idx++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
        pad = media_pad_remote_pad_first(&isp_dev->pads[pad_idx]);
#else
        pad = media_entity_remote_pad(&isp_dev->pads[pad_idx]);
#endif

        if (pad && (port != (pad_idx / VVCAM_ISP_CHN_MAX))) {
            port = pad_idx / VVCAM_ISP_CHN_MAX;
            seq_printf(sfile, "isp%d port%d:\n", isp_dev->id, port);
            seq_printf(sfile, "sensor   : %s\n", isp_dev->sensor_info[port].sensor);
            seq_printf(sfile, "mode     : %d\n", isp_dev->sensor_info[port].mode);
            seq_printf(sfile, "xml      : %s\n", isp_dev->sensor_info[port].xml);
            seq_printf(sfile, "manu_json: %s\n", isp_dev->sensor_info[port].manu_json);
            seq_printf(sfile, "auto_json: %s\n", isp_dev->sensor_info[port].auto_json);
        }
    }

    mutex_unlock(&isp_proc->lock);

	return 0;
}

static int vvcam_isp_procfs_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	return single_open(file, vvcam_isp_procfs_info_show, PDE_DATA(inode));
#else
	return single_open(file, vvcam_isp_procfs_info_show, pde_data(inode));
#endif
}

static int32_t vvcam_isp_proc_process(struct seq_file *sfile,
                        struct vvcam_isp_procfs *isp_proc, char *str_buf)
{
    struct vvcam_isp_dev *isp_dev;
    char *token = NULL, *cur = str_buf;
    char *val = NULL, *kv_cur = NULL, *end = NULL;
    char *const delim = " ";
    char *const kv_delim = "=";
    int port = 0;

    isp_dev = isp_proc->isp_dev;

    mutex_lock(&isp_proc->lock);

    while((token = strsep(&cur, delim))) {
        if (isdigit(*token)) {
            port = *token - '0';
            continue;
        }

        if ((port > VVCAM_ISP_CHN_MAX) || (port < 0)) {
            continue;
        }

        kv_cur = token;
        val = strsep(&kv_cur, kv_delim);
        if (val) {
            if (strcmp(val, "sensor") == 0) {
                val = strsep(&kv_cur, kv_delim);
                if (val) {
                    memset(isp_dev->sensor_info[port].sensor, 0, \
                                sizeof(isp_dev->sensor_info[port].sensor));
                    strncpy(isp_dev->sensor_info[port].sensor, val, strlen(val));
                }
            } else if (strcmp(val, "mode") == 0) {
                val = strsep(&kv_cur, kv_delim);
                if (val && isdigit(*val)) {
                    isp_dev->sensor_info[port].mode = (uint32_t)simple_strtoul(val, &end, 0);
                }
            } else if (strcmp(val, "xml") == 0) {
                val = strsep(&kv_cur, kv_delim);
                if (val) {
                    memset(isp_dev->sensor_info[port].xml, 0, \
                                sizeof(isp_dev->sensor_info[port].xml));
                    strncpy(isp_dev->sensor_info[port].xml, val, strlen(val));
                }
            } else if (strcmp(val, "manu_json") == 0) {
                val = strsep(&kv_cur, kv_delim);
                if (val) {
                    memset(isp_dev->sensor_info[port].manu_json, 0, \
                                sizeof(isp_dev->sensor_info[port].manu_json));
                    strncpy(isp_dev->sensor_info[port].manu_json, val, strlen(val));
                }
            } else if (strcmp(val, "auto_json") == 0) {
                val = strsep(&kv_cur, kv_delim);
                if (val) {
                    memset(isp_dev->sensor_info[port].auto_json, 0, \
                                sizeof(isp_dev->sensor_info[port].auto_json));
                    strncpy(isp_dev->sensor_info[port].auto_json, val, strlen(val));
                }
            }
        }
    }

    mutex_unlock(&isp_proc->lock);

    return 0;
}

static ssize_t vvcam_isp_procfs_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
    struct vvcam_isp_procfs *isp_proc;
    struct seq_file *sfile;
    char *str_buf;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	isp_proc = (struct vvcam_isp_procfs *) PDE_DATA(file_inode(file));
#else
	isp_proc = (struct vvcam_isp_procfs *) pde_data(file_inode(file));
#endif
    sfile = file->private_data;

    str_buf = (char *)kzalloc(count, GFP_KERNEL);
    if (!str_buf)
        return -ENOMEM;

    if (copy_from_user(str_buf, buffer, count))
        return -EFAULT;

    *(str_buf + count - 1) = '\0';

    vvcam_isp_proc_process(sfile, isp_proc, str_buf);

    kfree(str_buf);

    return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static const struct file_operations vvcam_isp_procfs_ops = {
    .open = vvcam_isp_procfs_open,
    .release = seq_release,
    .read = seq_read,
    .write = vvcam_isp_procfs_write,
    .llseek = seq_lseek,
};
#else
static const struct proc_ops vvcam_isp_procfs_ops = {
	.proc_open = vvcam_isp_procfs_open,
	.proc_release = seq_release,
	.proc_read = seq_read,
	.proc_write = vvcam_isp_procfs_write,
	.proc_lseek = seq_lseek,
};
#endif

struct finddir_callback {
    struct dir_context ctx;
    const char *name;
    int32_t files_cnt;
    bool found;
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
static int readdir_callback(struct dir_context *ctx, const char *name,
    int namelen, loff_t offset, u64 ino, unsigned int d_type) {
    struct finddir_callback *fc =
        container_of(ctx, struct finddir_callback, ctx);
    if (fc->found)
        return 0;

    if(strcmp(name, fc->name) == 0) {
        fc->found = true;
    }
    fc->files_cnt++;
    return 0;
}

#else
static bool readdir_callback(struct dir_context *ctx, const char *name,
        int namelen, loff_t offset, u64 ino, unsigned int d_type) {

    struct finddir_callback *fc =
        container_of(ctx, struct finddir_callback, ctx);
    if (fc->found)
        return true;
    if(strcmp(name, fc->name) == 0) {
        fc->found = true;
    }
    fc->files_cnt++;
    return true;
}
#endif

static int find_proc_dir_by_name(const char *root,
                const char *name, bool *found, int32_t *files_cnt) {
    struct file *pfile;
    int ret = 0;
    struct finddir_callback fc = {
        .ctx.actor = readdir_callback,
        .name = name,
        .found = false,
        .files_cnt = -2,
    };

    pfile = filp_open(root, O_RDONLY | O_DIRECTORY, 0);
    if (pfile->f_op->iterate_shared) {
        ret = pfile->f_op->iterate_shared(pfile, &fc.ctx);
    } else {
        ret = pfile->f_op->iterate(pfile, &fc.ctx);
    }

    if (ret == 0) {
        *found = fc.found;
    }

    if (files_cnt != NULL) {
        *files_cnt = fc.files_cnt;
    }

    filp_close(pfile, NULL);
    return ret;
}

int vvcam_isp_procfs_register(struct vvcam_isp_dev *isp_dev, unsigned long *pde)
{
    struct vvcam_isp_procfs *isp_proc;
    char isp_proc_name[32];
    int ret = 0;
    bool found = false;

    if (!isp_dev)
        return -1;
    sprintf(isp_proc_name, "vsi/isp_subdev%d", isp_dev->id);

    isp_proc = devm_kzalloc(isp_dev->dev,
                        sizeof(struct vvcam_isp_procfs), GFP_KERNEL);

    if (!isp_proc)
        return -ENOMEM;

    ret = find_proc_dir_by_name("/proc", "vsi", &found, NULL);
    if (ret == 0) {
        if (!found)
            proc_mkdir("vsi", NULL);
    } else {
        return -EFAULT;
    }

    isp_proc->isp_dev = isp_dev;
    isp_proc->pde = proc_create_data(isp_proc_name, 0664, NULL,
                                &vvcam_isp_procfs_ops, isp_proc);
    if (!isp_proc->pde)
        return -EFAULT;
    *pde = (unsigned long)&isp_proc->pde;

    mutex_init(&(isp_proc->lock));
	return 0;
}

void vvcam_isp_procfs_unregister(unsigned long pde)
{
    int ret = 0;
    bool found = false;
    int32_t files_cnt;
    struct vvcam_isp_procfs *isp_proc = (struct vvcam_isp_procfs*)pde;

    ret = find_proc_dir_by_name("/proc", "vsi", &found, NULL);
    if (ret == 0) {
        if (found) {
            proc_remove(isp_proc->pde);
            ret = find_proc_dir_by_name("/proc/vsi", "", &found, &files_cnt);
            if (files_cnt == 0) {
                remove_proc_subtree("vsi", NULL);
            }
        }
    }
}
