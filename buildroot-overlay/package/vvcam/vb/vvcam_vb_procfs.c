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
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include "vvcam_mmz.h"
#include "vvcam_cma.h"
#include "vvcam_vb_driver.h"
#include "vvcam_vb_procfs.h"

struct mmb_info {
    char name[32];
    unsigned long size;
    unsigned long paddr;
    struct list_head list;
};

struct filed_mmb {
    void *private;
    struct list_head mbi_list; 
    struct list_head list;
};

struct vvcam_vb_proc {
    struct proc_dir_entry *pde;
    struct vvcam_vb_dev *vb_dev;
};


static void release_mmlist(struct list_head *mm_list)
{
    struct mmb_info *mbi = NULL, *old_mbi = NULL;
    struct filed_mmb *fmb = NULL, *old_fmb = NULL;
    if (!list_empty(mm_list)) {
        list_for_each_entry(fmb, mm_list, list) {
            list_for_each_entry(mbi, &fmb->mbi_list, list) {
                if (old_mbi) {
                    list_del(&old_mbi->list);
                    kfree(old_mbi);
                    old_mbi = NULL;
                }
                old_mbi = mbi;
            }
            
            if (old_fmb) {
                list_del(&old_fmb->list);
                kfree(old_fmb);
                old_fmb = NULL;
            }
            old_fmb = fmb;
        }

        if (old_mbi) {
            list_del(&old_mbi->list);
            kfree(old_mbi);
            old_mbi = NULL;
        }

        if (old_fmb) {
            list_del(&old_fmb->list);
            kfree(old_fmb);
            old_fmb = NULL;
        }
    }
}

static int vvcam_vb_get_mmz_info(struct vvcam_mmz *mmz,
                    struct list_head *mm_list, unsigned long *base,
                    unsigned long *size, unsigned long *used)
{
    struct vvcam_mmb *mmb = NULL;
    struct mmb_info *mbi = NULL;
    struct filed_mmb *fmb = NULL, *t_fmb = NULL;

    *size = mmz->size;
    *base = mmz->paddr;

    list_for_each_entry(mmb, &mmz->mmb_list, list) {
        fmb = NULL;
        if (!mmb) {
            break;
        }
        list_for_each_entry(t_fmb, mm_list, list) {
            if (!t_fmb) {
                break;
            }
            if (t_fmb->private == mmb->private) {
                fmb = t_fmb;
                t_fmb = NULL;
                break;
            }
        }

        if (!fmb) {
            fmb = kzalloc(sizeof(struct filed_mmb), GFP_KERNEL);
            if (!fmb) {
                release_mmlist(mm_list);
                return -ENOMEM;
            }
            INIT_LIST_HEAD(&fmb->mbi_list);
            list_add_tail(&fmb->list, mm_list);
        }

        mbi = kzalloc(sizeof(struct mmb_info), GFP_KERNEL);
        if (!mbi) {
            release_mmlist(mm_list);
            return -ENOMEM;
        }

        fmb->private = mmb->private;
        memcpy(mbi->name, mmb->name, sizeof(mmb->name));
        mbi->paddr = mmb->phys_addr;
        mbi->size = mmb->size;
        *used += mbi->size;

        list_add_tail(&mbi->list, &fmb->mbi_list);
    }
    return 0;
}

static int vvcam_vb_get_cma_info(struct vvcam_cma *cma,
                struct list_head *mm_list, unsigned long *used)
{
    struct vvcam_cma_buf *cma_buf = NULL;
    struct mmb_info *mbi = NULL;
    struct filed_mmb *fmb = NULL, *tmp = NULL;

    list_for_each_entry(cma_buf, &cma->buf_list, list) {
        fmb = NULL;
        if (!cma_buf) {
            break;
        }
        list_for_each_entry(tmp, mm_list, list) {
            if (!tmp) {
                break;
            }
            if (tmp->private == cma_buf->private) {
                fmb = tmp;
                tmp = NULL;
                break;
            }
        }

        if (!fmb) {
            fmb = kzalloc(sizeof(struct filed_mmb), GFP_KERNEL);
            if (!fmb) {
                release_mmlist(mm_list);
                return -ENOMEM;
            }
            INIT_LIST_HEAD(&fmb->mbi_list);
            list_add_tail(&fmb->list, mm_list);
        }

        mbi = kzalloc(sizeof(struct mmb_info), GFP_KERNEL);
        if (!mbi) {
            release_mmlist(mm_list);
            return -ENOMEM;
        }

        fmb->private = cma_buf->private;
        memcpy(mbi->name, cma_buf->name, sizeof(mbi->name));
        mbi->paddr = cma_buf->phys_addr;
        mbi->size = cma_buf->size;

        *used += cma_buf->size;

        list_add_tail(&mbi->list, &fmb->mbi_list);
    }
    return 0;
}

static int vvcam_vb_proc_info_show(struct seq_file *sfile, void *offset)
{
	struct vvcam_vb_proc *vb_proc;
    struct list_head mm_list;
    struct filed_mmb *fmb = NULL;
    struct mmb_info *mbi = NULL;

    unsigned long size = 0;
    unsigned long used = 0;
    unsigned long base = 0;
    int ret = 0;

	vb_proc = (struct vvcam_vb_proc *) sfile->private;

    INIT_LIST_HEAD(&mm_list);

    switch (vb_proc->vb_dev->type)
    {
    case VVCAM_VB_MMZ:
        ret = vvcam_vb_get_mmz_info(
            (struct vvcam_mmz*)vb_proc->vb_dev->allocator.mm_dev,
            &mm_list, &base, &size, &used);
        break;
    
    case VVCAM_VB_CMA:
        ret = vvcam_vb_get_cma_info(
            (struct vvcam_cma*)vb_proc->vb_dev->allocator.mm_dev,
            &mm_list, &used);
        break;
    
    default:
        ret = -EINVAL; 
        break;
    }

    if (ret) {
        seq_printf(sfile, "get mm info failed\n");
        return 0;
    }

    seq_printf(sfile, "DeviceName:\t%s\n", vb_proc->vb_dev->miscdev.name);
    seq_printf(sfile, "VB Type:\t%s\n",
        vb_proc->vb_dev->type == VVCAM_VB_MMZ ? "reserved mem" : "cma");
    if (vb_proc->vb_dev->type == VVCAM_VB_MMZ) {
        seq_printf(sfile, "Base:\t\t0x%-8lx\n", base);
        seq_printf(sfile, "Size:\t\t0x%-8lx\n", size);
        seq_printf(sfile, "Free:\t\t0x%-8lx\n", size - used);
    }
    seq_printf(sfile, "Used:\t\t0x%-8lx\n", used);

    seq_printf(sfile,
    "|------------------|----------------|---------------------|----------|\n");
    seq_printf(sfile, "|%-18s|%-16s|%-21s|%-10s|\n",
        "       File", "      Name", "        Range", "   Size");
    seq_printf(sfile,
    "|------------------|----------------|---------------------|----------|\n");
    if (!list_empty(&mm_list)) {
         list_for_each_entry(fmb, &mm_list, list) {
            list_for_each_entry(mbi, &fmb->mbi_list, list) {
                seq_printf(sfile, "|0x%-16p|%-16s|0x%08lx~0x%08lx|0x%-8lx|\n",
                fmb->private, mbi->name,
                mbi->paddr, mbi->paddr + mbi->size, mbi->size);
            }
            seq_printf(sfile,
    "|------------------|----------------|---------------------|----------|\n");
         }
    }

    release_mmlist(&mm_list);

	return 0;
}


static int vvcam_vb_proc_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	return single_open(file, vvcam_vb_proc_info_show, PDE_DATA(inode));
#else
	return single_open(file, vvcam_vb_proc_info_show, pde_data(inode));
#endif
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static const struct file_operations vvcam_vb_proc_ops = {
    .open = vvcam_vb_proc_open,
    .release = seq_release,
    .read = seq_read,
    .llseek = seq_lseek,
};
#else
static const struct proc_ops vvcam_vb_proc_ops = {
	.proc_open = vvcam_vb_proc_open,
	.proc_release = seq_release,
	.proc_read = seq_read,
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
        // ret = pfile->f_op->iterate(pfile, &fc.ctx);
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

int vvcam_vb_procfs_register(struct vvcam_vb_dev *vb_dev,
                                struct proc_dir_entry **pde)
{
    struct vvcam_vb_proc *vb_proc;
    struct platform_device *pdev = to_platform_device(vb_dev->dev);
    char vb_proc_name[32];
    int ret = 0;
    bool found = false;

    sprintf(vb_proc_name, "vsi/vb%d", pdev->id);
    vb_proc = devm_kzalloc(vb_dev->dev, 
                        sizeof(struct vvcam_vb_proc), GFP_KERNEL);
    if (!vb_proc)
        return -ENOMEM;

    vb_proc->vb_dev = vb_dev;
    ret = find_proc_dir_by_name("/proc", "vsi", &found, NULL);
    if (ret == 0) {
        if (!found)
            proc_mkdir("vsi", NULL);    
    } else {
        return -EFAULT;
    }

    vb_proc->vb_dev = vb_dev;
    *pde = proc_create_data(vb_proc_name, 0664, NULL,
                                &vvcam_vb_proc_ops, vb_proc);
    if (!*pde)
        return -EFAULT;

    vb_proc->pde = *pde;
	return 0;
}

void vvcam_vb_procfs_unregister(struct proc_dir_entry **pde)
{
    int ret = 0;
    bool found = false;
    int32_t files_cnt;

    ret = find_proc_dir_by_name("/proc", "vsi", &found, NULL);
    if (ret == 0) {
        if (found) {
            proc_remove(*pde);
            ret = find_proc_dir_by_name("/proc/vsi", "", &found, &files_cnt);
            if (files_cnt == 0) {
                remove_proc_subtree("vsi", NULL);
            }
        }
    }
}
