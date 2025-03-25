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

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>

#include "vvcam_event.h"

void vvcam_event_dev_init(struct vvcam_event_dev *event_dev)
{
    spin_lock_init(&event_dev->fh_lock);
    INIT_LIST_HEAD(&event_dev->fh_list);
    return;
}

void vvcam_event_fh_init(struct vvcam_event_dev *event_dev,
            struct vvcam_event_fh *event_fh)
{
    unsigned long flags;

    INIT_LIST_HEAD(&event_fh->subscribed);
    INIT_LIST_HEAD(&event_fh->available);
    init_waitqueue_head(&event_fh->wait);
    event_fh->event_dev = event_dev;

    spin_lock_irqsave(&event_fh->event_dev->fh_lock, flags);
    list_add(&event_fh->list, &event_dev->fh_list);
    spin_unlock_irqrestore(&event_fh->event_dev->fh_lock, flags);

    return;
}

void vvcam_event_fh_destroy(struct vvcam_event_dev *event_dev,
            struct vvcam_event_fh *event_fh)
{
    unsigned long flags;

    spin_lock_irqsave(&event_fh->event_dev->fh_lock, flags);
    list_del(&event_fh->list);
    spin_unlock_irqrestore(&event_fh->event_dev->fh_lock, flags);

    return;
}

static unsigned sev_pos(const struct vvcam_subscribed_event *sev, unsigned idx)
{
    idx += sev->first;
    return idx >= sev->elems ? idx - sev->elems : idx;
}

static struct vvcam_subscribed_event *vvcam_event_subscribed(
            struct vvcam_event_fh *event_fh, uint32_t type, uint32_t id)
{
    struct vvcam_subscribed_event *sev;

    assert_spin_locked(&event_fh->event_dev->fh_lock);

    if (list_empty(&event_fh->subscribed))
        return NULL;

    list_for_each_entry(sev, &event_fh->subscribed, list)
        if (sev->type == type && sev->id == id)
            return sev;

    return NULL;
}

int vvcam_event_subscribe(struct vvcam_event_fh *event_fh,
            const vvcam_subscription_t *sub, unsigned elems)
{
    struct vvcam_subscribed_event *sev, *found_ev;
    unsigned long flags;
    unsigned i;

    if (elems < 1)
        elems = 1;

    sev = kzalloc(sizeof(*sev) + sizeof(struct vvcam_kevent) * elems, GFP_KERNEL);
    if (!sev)
        return -ENOMEM;

    for (i = 0; i < elems; i++) {
        sev->events[i].sev = sev;
    }

    sev->type = sub->type;
    sev->id = sub->id;
    sev->event_fh = event_fh;
    sev->elems = elems;

    spin_lock_irqsave(&event_fh->event_dev->fh_lock, flags);

    found_ev = vvcam_event_subscribed(event_fh, sub->type, sub->id);
    if (!found_ev)
        list_add(&sev->list, &event_fh->subscribed);

    spin_unlock_irqrestore(&event_fh->event_dev->fh_lock, flags);

    if (found_ev) {
        kfree(sev);
        return 0;
    }

    return 0;
}

int vvcam_event_unsubscribe(struct vvcam_event_fh *event_fh,
            const vvcam_subscription_t *sub)
{
    struct vvcam_subscribed_event *sev;
    unsigned long flags;
    int i;

    spin_lock_irqsave(&event_fh->event_dev->fh_lock, flags);

    sev = vvcam_event_subscribed(event_fh, sub->type, sub->id);
    if (sev != NULL) {
        for (i = 0; i < sev->in_use; i++) {
            list_del(&sev->events[sev_pos(sev, i)].list);
            event_fh->navailable--;
        }

        list_del(&sev->list);
        kfree(sev);
    }
    spin_unlock_irqrestore(&event_fh->event_dev->fh_lock, flags);

    return 0;
}

int vvcam_event_unsubscribe_all(struct vvcam_event_fh *event_fh)
{
    vvcam_subscription_t sub;
    struct vvcam_subscribed_event *sev;
    unsigned long flags;

    do {
        sev = NULL;

        spin_lock_irqsave(&event_fh->event_dev->fh_lock, flags);

        if (!list_empty(&event_fh->subscribed)) {
            sev = list_first_entry(&event_fh->subscribed,
                    struct vvcam_subscribed_event, list);
            sub.type = sev->type;
            sub.id = sev->id;
        }
        spin_unlock_irqrestore(&event_fh->event_dev->fh_lock, flags);

        if (sev)
            vvcam_event_unsubscribe(event_fh, &sub);

    } while (sev);

    return 0;
}

static void __vvcam_event_queue_fh(struct vvcam_event_fh *event_fh,
            const vvcam_event_t *event)
{
    struct vvcam_subscribed_event *sev;
    struct vvcam_kevent *kev;

    sev = vvcam_event_subscribed(event_fh, event->type, event->id);
    if (!sev)
        return;

    if (!sev->elems)
        return;

    if (sev->in_use == sev->elems) {
        kev = sev->events + sev_pos(sev, 0);
        list_del(&kev->list);
        sev->in_use--;
        sev->first = sev_pos(sev, 1);
        event_fh->navailable--;
    }

    kev = sev->events + sev_pos(sev, sev->in_use);

    kev->event = *event;
    sev->in_use++;

    list_add_tail(&kev->list, &event_fh->available);
    event_fh->navailable++;

    wake_up_all(&event_fh->wait);

    return;
}

void vvcam_event_queue(struct vvcam_event_dev *event_dev,
            const vvcam_event_t *event)
{
    struct vvcam_event_fh *event_fh;
    unsigned long flags;

    spin_lock_irqsave(&event_dev->fh_lock, flags);

    if (list_empty(&event_dev->fh_list)) {
        spin_unlock_irqrestore(&event_dev->fh_lock, flags);
        return;
    }

    list_for_each_entry(event_fh, &event_dev->fh_list, list)
        __vvcam_event_queue_fh(event_fh, event);

    spin_unlock_irqrestore(&event_dev->fh_lock, flags);

    return;
}

int vvcam_event_dequeue(struct vvcam_event_fh *event_fh, vvcam_event_t *event)
{
    struct vvcam_kevent *kev;
    unsigned long flags;

    spin_lock_irqsave(&event_fh->event_dev->fh_lock, flags);

    if (list_empty(&event_fh->available)) {
        spin_unlock_irqrestore(&event_fh->event_dev->fh_lock, flags);
        return -ENOENT;
    }

    kev = list_first_entry(&event_fh->available, struct vvcam_kevent, list);
    list_del(&kev->list);
    event_fh->navailable--;

    kev->sev->first = sev_pos(kev->sev, 1);
    kev->sev->in_use--;
    *event = kev->event;

    spin_unlock_irqrestore(&event_fh->event_dev->fh_lock, flags);

    return 0;
}

unsigned int vvcam_event_poll(struct file *file,
            struct vvcam_event_fh *event_fh, poll_table *wait)
{
    poll_wait(file, &event_fh->wait, wait);

    if (event_fh->navailable)
        return POLLPRI;

    return 0;
}
