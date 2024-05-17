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

#ifndef __VVCAM_MMZ_H__
#define __VVCAM_MMZ_H__

#define VVCAM_MMB_NAME_LEN 32
#define VVCAM_MAX_ORDER 18
#define VVCAM_MIN_ORDER 0

struct vvcam_free_area {
    struct list_head free_list;
    unsigned long nr_free;
};

struct vvcam_mmb {
    char name[VVCAM_MMB_NAME_LEN];
    struct vvcam_mmz *mmz;
    struct list_head list;
    unsigned long phys_addr;
    unsigned long size;
    unsigned int order;

    void *private;
};

struct vvcam_mmz {
    struct device *dev;
    unsigned long paddr;
    unsigned long size;
    /*
     * zone_start_pfn = zone_start_pfn >> PAGE_SHIFT
     * managed_pages = zone_end_pfn - zone_start_pfn
     */
    unsigned long zone_start_pfn;
    unsigned long spanned_pages;
    unsigned long managed_pages;

    struct vvcam_free_area free_area[VVCAM_MAX_ORDER];
    struct list_head mmb_list;
    spinlock_t lock;
};

int vvcam_mmz_create(void *allocator, struct device *dev,
            const unsigned long phys_addr, const unsigned long size);
#endif
