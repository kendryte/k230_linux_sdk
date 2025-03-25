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
#ifndef __VVCAM_EVENT_H__
#define __VVCAM_EVENT_H__

#define VVCAM_EVENT_IRQ_TYPE                0x00000000
#define VVCAM_VIDEO_IN_EVENT_IRQ_TYPE       0x00000001
typedef struct {
    uint32_t type;
    uint32_t id;
} vvcam_subscription_t;

typedef struct {
    uint32_t irq_value;
} vvcam_irqevent;

typedef struct {
    uint32_t type;
    uint32_t id;
    union {
        vvcam_irqevent irqevent;
    };
    uint64_t timestamp;
} vvcam_event_t;

#ifdef __KERNEL__
#include <linux/poll.h>

struct vvcam_event_dev {
    spinlock_t fh_lock;
    struct list_head fh_list;
};

struct vvcam_event_fh {
    struct list_head list;
    struct vvcam_event_dev *event_dev;
    wait_queue_head_t wait;
    unsigned int navailable;
    struct list_head subscribed;
    struct list_head available;
};

struct vvcam_kevent {
    struct list_head list;
    struct vvcam_subscribed_event *sev;
    vvcam_event_t event;
};

struct vvcam_subscribed_event {
    struct list_head list;
    uint32_t type;
    uint32_t id;

    struct vvcam_event_fh *event_fh;
    uint32_t elems;
    uint32_t in_use;
    uint32_t first;
    struct vvcam_kevent events[];
};

void vvcam_event_dev_init(struct vvcam_event_dev *event_dev);
void vvcam_event_fh_init(struct vvcam_event_dev *event_dev,
            struct vvcam_event_fh *event_fh);
void vvcam_event_fh_destroy(struct vvcam_event_dev *event_dev,
            struct vvcam_event_fh *event_fh);
int vvcam_event_subscribe(struct vvcam_event_fh *event_fh,
            const vvcam_subscription_t *sub, unsigned elems);
int vvcam_event_unsubscribe(struct vvcam_event_fh *event_fh,
            const vvcam_subscription_t *sub);
int vvcam_event_unsubscribe_all(struct vvcam_event_fh *event_fh);
void vvcam_event_queue(struct vvcam_event_dev *event_dev,
            const vvcam_event_t *event);
int vvcam_event_dequeue(struct vvcam_event_fh *event_fh, vvcam_event_t *event);
unsigned int vvcam_event_poll(struct file *file,
            struct vvcam_event_fh *event_fh, poll_table *wait);

#endif

#endif
