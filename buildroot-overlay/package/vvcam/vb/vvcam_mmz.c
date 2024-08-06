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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/pm_runtime.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include "vvcam_mmz.h"
#include "vvcam_vb_driver.h"

static int vvcam_mmz_alloc(struct file *file,
            unsigned long *paddr, unsigned long size, char *name);
static int vvcam_mmz_mmap(struct file *file, struct vm_area_struct *vma);
static int vvcam_mmz_free(struct file *file, unsigned long paddr);
static int vvcam_mmz_free_all(struct file *file);

static void *vvcam_alloc_mmb_attr(struct vvcam_mmz *mmz)
{
    return devm_kzalloc(mmz->dev, sizeof(struct vvcam_mmb), GFP_KERNEL);
}

static void vvcam_free_mmb_attr(struct vvcam_mmz *mmz, struct vvcam_mmb *mmb)
{
    devm_kfree(mmz->dev, mmb);
}

static int vvcam_get_order(unsigned long size)
{
    int order;
    unsigned long pfn  = (size >> PAGE_SHIFT);

    order = -1;
    do {
        pfn >>= 1;
        order++;
    } while(pfn);

    return order;
}

int vvcam_mmz_create(void *allocator, struct device *dev,
                    const unsigned long phys_addr, const unsigned long size)
{
    int i;
    unsigned int order;
    struct vvcam_mmb *mmb;
    struct vvcam_mmz *mmz;
    struct vvcam_vb_mm_allocator *pmm_agent =
                    (struct vvcam_vb_mm_allocator *)allocator;

    mmz =(struct vvcam_mmz *)devm_kzalloc(dev,
                sizeof(struct vvcam_mmz), GFP_KERNEL);
    if (!mmz)
        return -ENOMEM;
    mmz->dev = dev;

    mmz->size = size;
    mmz->paddr = phys_addr;

    mmz->zone_start_pfn = (phys_addr >> PAGE_SHIFT);
    mmz->spanned_pages  = (size >> PAGE_SHIFT);

    order = vvcam_get_order(size);
    if (order >= VVCAM_MAX_ORDER)
        return -1;

    mmz->managed_pages = (1UL << order);
    for (i = 0; i < VVCAM_MAX_ORDER; i++) {
        INIT_LIST_HEAD(&mmz->free_area[i].free_list);
        mmz->free_area[i].nr_free = 0;
    }

    INIT_LIST_HEAD(&mmz->mmb_list);
    spin_lock_init(&mmz->lock);

    mmb = vvcam_alloc_mmb_attr(mmz);
    if (mmb == NULL)
        return -ENOMEM;

    mmb->mmz = mmz;
    mmb->phys_addr = phys_addr;
    mmb->size = (mmz->managed_pages << PAGE_SHIFT);
    mmb->order  = order;

    list_add_tail(&mmb->list, &mmz->free_area[mmb->order].free_list);
    mmz->free_area[mmb->order].nr_free += 1;

    pmm_agent->mm_dev = mmz;
    pmm_agent->alloc = vvcam_mmz_alloc;
    pmm_agent->mmap = vvcam_mmz_mmap;
    pmm_agent->free = vvcam_mmz_free;
    pmm_agent->free_all = vvcam_mmz_free_all;
    return 0;
}

static int vvcam_mmz_split_block(struct vvcam_mmz *mmz, struct vvcam_mmb *mmb)
{
    unsigned int order;
    unsigned long size;
    struct vvcam_mmb *block;

    if (mmb->order == VVCAM_MIN_ORDER)
        return -1;

    order = mmb->order - 1;
    size  = mmb->size / 2;

    block = vvcam_alloc_mmb_attr(mmz);
    if (block == NULL)
        return -ENOMEM;

    block->mmz       = mmz;
    block->phys_addr = mmb->phys_addr + size;
    block->size      = size;
    block->order     = order;

    mmb->size        = size;
    mmb->order       = order;

    list_add_tail(&mmb->list, &mmz->free_area[mmb->order].free_list);
    mmz->free_area[mmb->order].nr_free += 1;

    list_add_tail(&block->list, &mmz->free_area[block->order].free_list);
    mmz->free_area[block->order].nr_free += 1;

    return 0;
}

static struct vvcam_mmb *vvcam_mmz_alloc_block(struct vvcam_mmz *mmz, int order)
{
    struct vvcam_mmb *mmb = NULL;
    struct vvcam_mmb *block = NULL;
    int i = 0;

    if (order >= VVCAM_MAX_ORDER)
        return NULL;

    if (mmz->free_area[order].nr_free) {
        mmb = list_first_entry(&mmz->free_area[order].free_list,
                            struct vvcam_mmb, list);
        list_del(&mmb->list);
        mmz->free_area[order].nr_free--;
        return mmb;
    } else {
        block = NULL;

        for (i = order; i < VVCAM_MAX_ORDER; i++) {
            if (mmz->free_area[i].nr_free) {
                block = list_first_entry(&mmz->free_area[i].free_list,
                            struct vvcam_mmb, list);
                list_del(&block->list);
                mmz->free_area[i].nr_free--;
                break;
            }
        }
        if (block == NULL)
            return NULL;

        vvcam_mmz_split_block(mmz, block);
        mmb = vvcam_mmz_alloc_block(mmz, order);
        return mmb;
    }
}

struct vvcam_mmb * vvcam_mmz_get_buddy(struct vvcam_mmb *mmb)
{
    struct vvcam_mmz *mmz = mmb->mmz;
    unsigned int block_index;
    unsigned int buddy_index;
    unsigned long buddy_phys_addr;
    struct vvcam_mmb *block = NULL, *buddy_block = NULL;

    block_index = ((mmb->phys_addr >> PAGE_SHIFT) - mmz->zone_start_pfn) / (1 << mmb->order);
    buddy_index = block_index ^ 0x01;

    buddy_phys_addr = ((buddy_index * (1 << mmb->order) + mmz->zone_start_pfn) << PAGE_SHIFT);

    if (mmz->free_area[mmb->order].nr_free) {
        list_for_each_entry(block, &mmz->free_area[mmb->order].free_list, list) {
            if (!block) {
                break;
            }
            if (block->phys_addr == buddy_phys_addr) {
                buddy_block = block;
                break;
            }
        }
    }

    return buddy_block;
}

static void vvcam_mmz_free_block(struct vvcam_mmz *mmz, struct vvcam_mmb *mmb)
{
    struct vvcam_mmb *buddy_block = NULL;

    buddy_block = vvcam_mmz_get_buddy(mmb);

    if (buddy_block == NULL) {
        list_add_tail(&mmb->list, &mmz->free_area[mmb->order].free_list);
        mmz->free_area[mmb->order].nr_free++;
        return;
    }

    list_del(&buddy_block->list);
    mmz->free_area[buddy_block->order].nr_free--;

    //merge block;
    mmb->phys_addr =  (buddy_block->phys_addr > mmb->phys_addr) ?
                        mmb->phys_addr : buddy_block->phys_addr;
    mmb->size = mmb->size * 2;
    mmb->order += 1;
    vvcam_free_mmb_attr(mmz, buddy_block);

    vvcam_mmz_free_block(mmz, mmb);
    return;
}

static int vvcam_mmz_alloc(struct file *file,
            unsigned long *paddr, unsigned long size, char *name)
{
    struct vvcam_mmz *mmz;
    struct vvcam_vb_dev *vb_dev;
    struct vvcam_mmb *mmb;
    int order;

    vb_dev = file->private_data;
    mmz = vb_dev->allocator.mm_dev;

    if (size < (1UL << PAGE_SHIFT))
        size = (1UL << PAGE_SHIFT);

    order = vvcam_get_order(size);

    if (size > (1 << order) << PAGE_SHIFT) {
        order += 1;
    }

    mmb = vvcam_mmz_alloc_block(mmz, order);
    if (mmb == NULL)
        return -1;

    strncpy(mmb->name, name, sizeof(mmb->name));
    mmb->private = file;
    list_add_tail(&mmb->list, &mmz->mmb_list);

    *paddr = mmb->phys_addr;
    return 0;
}

static int vvcam_mmz_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long start = 0;
    unsigned long pfn = 0;
    unsigned long vm_len = 0;

    pfn = (start >> PAGE_SHIFT) + vma->vm_pgoff;
    vm_len =  vma->vm_end - vma->vm_start;
    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

    return remap_pfn_range(vma, vma->vm_start, pfn, vm_len, vma->vm_page_prot);
}

static int vvcam_mmz_free(struct file *file, unsigned long paddr)
{
    struct vvcam_mmz *mmz;
    struct vvcam_vb_dev *vb_dev;
    struct vvcam_mmb *mmb = NULL, *block = NULL;

    vb_dev = file->private_data;
    mmz = vb_dev->allocator.mm_dev;

    if (list_empty(&mmz->mmb_list))
        return -1;

    list_for_each_entry(block, &mmz->mmb_list, list) {
        if (block->phys_addr == paddr) {
            mmb = block;
            break;
        }
    }

    if (mmb == NULL)
        return -1;

    list_del(&mmb->list);

    vvcam_mmz_free_block(mmz, mmb);

    return 0;
}

int vvcam_mmz_free_all(struct file *file)
{
    struct vvcam_mmz *mmz;
    struct vvcam_vb_dev *vb_dev;
    struct vvcam_mmb *mmb = NULL, *block;

    vb_dev = file->private_data;
    mmz = vb_dev->allocator.mm_dev;

    if (list_empty(&mmz->mmb_list))
        return 0;

    list_for_each_entry_safe(mmb, block, &mmz->mmb_list, list) {
        if (mmb->private == file) {
            list_del(&mmb->list);
            vvcam_mmz_free_block(mmz, mmb);
        }
    }

    return 0;
}





