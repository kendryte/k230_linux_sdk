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


#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <linux/delay.h>
#include "vvcam_video_event.h"

static bool vvcam_video_event_subscribed(struct vvcam_video_dev *vvcam_vdev,
                                uint32_t type, uint32_t id)
{
    struct v4l2_fh *fh;
    unsigned long flags;
    struct v4l2_subscribed_event *sev;
    bool subscribed = false;

    spin_lock_irqsave(&vvcam_vdev->video->fh_lock, flags);

    list_for_each_entry(fh, &vvcam_vdev->video->fh_list, list) {
        list_for_each_entry(sev, &fh->subscribed, list) {
            if (sev->type == type && sev->id == id) {
                subscribed = true;
                break;
            }
        }
        if (subscribed)
            break;
    }

    spin_unlock_irqrestore(&vvcam_vdev->video->fh_lock, flags);

    return subscribed;
}

int vvcam_video_post_event(struct vvcam_video_dev *vvcam_vdev,
                        struct vvcam_video_event_pkg *event_pkg)
{
    struct v4l2_event event;
    int timeout_ms = 200000;
    int i = 0;

    memset(&event, 0, sizeof(event));

    event.type   = VVCAM_VIDEO_DEAMON_EVENT;
	event.id     = event_pkg->head.eid;
    memcpy(event.u.data, &event_pkg->head, sizeof(event_pkg->head));

    if (!vvcam_video_event_subscribed(vvcam_vdev, event.type, event.id))
        return -EINVAL;

    v4l2_event_queue(vvcam_vdev->video, &event);

    for (i = 0; i < timeout_ms; i++) {
        if (event_pkg->ack) {
            break;
        }
        usleep_range(5, 10);
    }

    if (event_pkg->ack == 0) {
        dev_err(vvcam_vdev->vvcam_mdev->dev,
            "%s post event %d time out\n",
            vvcam_vdev->video->name, event.id);
        return -EIO;
    }

    if (event_pkg->result) {
        return -EINVAL;
    }

    return 0;
}

int vvcam_video_create_pipeline_event(struct vvcam_video_dev *vvcam_vdev)
{
    struct vvcam_video_event_pkg *event_pkg = vvcam_vdev->event_shm.virt_addr;
    int ret;

    mutex_lock(&vvcam_vdev->event_shm.event_lock);

    event_pkg->head.eid = VVCAM_VEVENT_CREATE_PIPELINE;
    event_pkg->head.shm_addr = vvcam_vdev->event_shm.phy_addr;
    event_pkg->head.shm_size = vvcam_vdev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->ack = 0;
    event_pkg->result = 0;

    ret = vvcam_video_post_event(vvcam_vdev, event_pkg);

    mutex_unlock(&vvcam_vdev->event_shm.event_lock);

    return ret;
}

int vvcam_video_destroy_pipeline_event(struct vvcam_video_dev *vvcam_vdev)
{
    struct vvcam_video_event_pkg *event_pkg = vvcam_vdev->event_shm.virt_addr;
    int ret;

    mutex_lock(&vvcam_vdev->event_shm.event_lock);

    event_pkg->head.eid = VVCAM_VEVENT_DESTROY_PIPELINE;
    event_pkg->head.shm_addr = vvcam_vdev->event_shm.phy_addr;
    event_pkg->head.shm_size = vvcam_vdev->event_shm.size;
    event_pkg->head.data_size = 0;
    event_pkg->ack = 0;
    event_pkg->result = 0;

    ret = vvcam_video_post_event(vvcam_vdev, event_pkg);

    mutex_unlock(&vvcam_vdev->event_shm.event_lock);

    return ret;
}


