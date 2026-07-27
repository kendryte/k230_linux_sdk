#include "three_camera.h"
#include <display.h>
#include <drm_fourcc.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define pr(fmt, ...) fprintf(stderr, "[three-camera] " fmt "\n", ##__VA_ARGS__)

static int v4l2_set_control(int fd, uint32_t id, int value)
{
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = id;
    ctrl.value = value;
    return ioctl(fd, VIDIOC_S_CTRL, &ctrl);
}

void three_camera_default_context(struct three_camera_context* ctx)
{
    v4l2_drm_default_context(&ctx->v4l2);
    ctx->video_layer = 0;
}

static uint32_t v4l2_to_drm(uint32_t fourcc)
{
    switch (fourcc) {
        case V4L2_PIX_FMT_NV12: return DRM_FORMAT_NV12;
        case V4L2_PIX_FMT_NV21: return DRM_FORMAT_NV21;
        case V4L2_PIX_FMT_NV16: return DRM_FORMAT_NV16;
        case V4L2_PIX_FMT_NV61: return DRM_FORMAT_NV61;
        default: return 0;
    }
}

static int compare_context_by_device(const void* a, const void* b)
{
    const struct three_camera_context* ca = a;
    const struct three_camera_context* cb = b;

    if (ca->v4l2.device < cb->v4l2.device) {
        return -1;
    }
    if (ca->v4l2.device > cb->v4l2.device) {
        return 1;
    }
    return 0;
}

static int open_and_format_video(struct v4l2_drm_context* ctx)
{
    char cam_device_path[64];
    struct v4l2_format format;
    int retry;

    snprintf(cam_device_path, sizeof(cam_device_path), "/dev/video%u", ctx->device);
    ctx->video_fd = open(cam_device_path, O_RDWR | O_NONBLOCK);
    if (ctx->video_fd < 0) {
        pr("open %s failed: %s", cam_device_path, strerror(errno));
        return -1;
    }

    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    if (ioctl(ctx->video_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        pr("%s VIDIOC_QUERYCAP failed: %s", cam_device_path, strerror(errno));
        return -1;
    }

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (retry = 0; retry < 5; retry++) {
        if (retry > 0) {
            usleep(100000 * (unsigned)retry);
        }
        if (ioctl(ctx->video_fd, VIDIOC_G_FMT, &format) == 0) {
            break;
        }
    }
    if (retry >= 5) {
        pr("%s VIDIOC_G_FMT failed: %s", cam_device_path, strerror(errno));
        return -1;
    }

    format.fmt.pix.pixelformat = ctx->video_format;
    format.fmt.pix.width = ctx->width & ~7U;
    format.fmt.pix.height = ctx->height & ~1U;
    ctx->width = format.fmt.pix.width;
    ctx->height = format.fmt.pix.height;

    if (ioctl(ctx->video_fd, VIDIOC_S_FMT, &format) < 0) {
        pr("%s VIDIOC_S_FMT %ux%u failed: %s", cam_device_path, ctx->width, ctx->height,
           strerror(errno));
        return -1;
    }

    if (ioctl(ctx->video_fd, VIDIOC_G_FMT, &format) == 0) {
        ctx->width = format.fmt.pix.width;
        ctx->height = format.fmt.pix.height;
    }

    if (ctx->hflip >= 0 && v4l2_set_control(ctx->video_fd, V4L2_CID_HFLIP, ctx->hflip) < 0) {
        pr("%s VIDIOC_S_CTRL HFLIP failed: %s", cam_device_path, strerror(errno));
        return -1;
    }
    if (ctx->vflip >= 0 && v4l2_set_control(ctx->video_fd, V4L2_CID_VFLIP, ctx->vflip) < 0) {
        pr("%s VIDIOC_S_CTRL VFLIP failed: %s", cam_device_path, strerror(errno));
        return -1;
    }

    return 0;
}

static int setup_display_plane(struct display* d, struct three_camera_context* tc)
{
    struct v4l2_drm_context* ctx = &tc->v4l2;
    unsigned j;

    if (ctx->display_format == 0) {
        ctx->display_format = v4l2_to_drm(ctx->video_format);
        if (ctx->display_format == 0) {
            return -1;
        }
    }
    d->drm_rotation = ctx->drm_rotation;
    if (tc->video_layer >= 1 && tc->video_layer <= 3) {
        ctx->plane = three_camera_get_video_plane(d, ctx->display_format, tc->video_layer);
    } else {
        ctx->plane = display_get_plane(d, ctx->display_format);
    }
    if (ctx->plane == NULL) {
        return -1;
    }

    for (j = 0; j < ctx->buffer_num; j++) {
        if (ctx->display_format == DRM_FORMAT_NV12 &&
            ((ctx->drm_rotation == rotation_90) || (ctx->drm_rotation == rotation_270))) {
            ctx->plane->drm_rotation = ctx->drm_rotation;
            if (display_allocate_buffer(ctx->plane, ctx->height, ctx->width) == NULL) {
                return -1;
            }
        } else if (display_allocate_buffer(ctx->plane, ctx->width, ctx->height) == NULL) {
            return -1;
        }
    }
    return 0;
}

static bool dmabuf_fd_used(int fd, const int* used_fds, unsigned used_count)
{
    for (unsigned i = 0; i < used_count; i++) {
        if (used_fds[i] == fd) {
            return true;
        }
    }
    return false;
}

static int queue_dmabuf_buffers(struct v4l2_drm_context* ctx, int* used_fds,
                                unsigned* used_count)
{
    struct v4l2_requestbuffers request_buffer;
    struct display_buffer* db;

    memset(&request_buffer, 0, sizeof(request_buffer));
    request_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    request_buffer.memory = V4L2_MEMORY_DMABUF;
    request_buffer.count = ctx->buffer_num;
    if (ioctl(ctx->video_fd, VIDIOC_REQBUFS, &request_buffer) < 0) {
        pr("/dev/video%u VIDIOC_REQBUFS failed: %s", ctx->device, strerror(errno));
        return -1;
    }

    ctx->buffers = calloc(ctx->buffer_num, sizeof(struct v4l2_drm_video_buffer));
    if (!ctx->buffers) {
        return -1;
    }
    for (unsigned bi = 0; bi < ctx->buffer_num; bi++) {
        ctx->buffers[bi].fd = -1;
    }

    ctx->display_buffers = calloc(ctx->buffer_num, sizeof(struct display_buffer*));
    if (!ctx->display_buffers) {
        return -1;
    }

    db = ctx->plane->buffers;
    for (unsigned j = 0; j < ctx->buffer_num; j++) {
        if (!db) {
            pr("/dev/video%u missing display buffer %u", ctx->device, j);
            return -1;
        }
        if (dmabuf_fd_used(db->dmabuf_fd, used_fds, *used_count)) {
            pr("/dev/video%u shares dmabuf fd %d with another camera", ctx->device, db->dmabuf_fd);
            return -1;
        }
        used_fds[(*used_count)++] = db->dmabuf_fd;

        memset(&ctx->vbuffer, 0, sizeof(ctx->vbuffer));
        ctx->vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ctx->vbuffer.memory = V4L2_MEMORY_DMABUF;
        ctx->vbuffer.index = j;
        ctx->vbuffer.m.fd = db->dmabuf_fd;
        ctx->vbuffer.length = db->size;
        if (ioctl(ctx->video_fd, VIDIOC_QBUF, &ctx->vbuffer) < 0) {
            pr("/dev/video%u VIDIOC_QBUF index %u fd %d failed: %s", ctx->device, j,
               db->dmabuf_fd, strerror(errno));
            return -1;
        }
        ctx->display_buffers[j] = db;
        ctx->buffers[j].mmap = db->map;
        ctx->buffers[j].fd = db->dmabuf_fd;
        ctx->buffers[j].index = j;
        db = db->next;
    }

    return 0;
}

static void cleanup_contexts(struct three_camera_context context[], unsigned num)
{
    for (unsigned j = 0; j < num; j++) {
        struct v4l2_drm_context* c = &context[j].v4l2;
        if (c->video_fd >= 0) {
            int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ioctl(c->video_fd, VIDIOC_STREAMOFF, &type);
            close(c->video_fd);
            c->video_fd = -1;
        }
        free(c->buffers);
        c->buffers = NULL;
        free(c->display_buffers);
        c->display_buffers = NULL;
    }
}

int three_camera_setup(struct three_camera_context context[], unsigned num, struct display** display)
{
    struct display* d = NULL;
    int used_fds[THREE_CAMERA_MAX * (DRM_BUFFERING + 3)];
    unsigned used_count = 0;
    unsigned i;
    bool need_display = false;

    if (num == 0) {
        return -1;
    }

    for (i = 0; i < num; i++) {
        context[i].v4l2.buffers = NULL;
        context[i].v4l2.display_buffers = NULL;
        context[i].v4l2.plane = NULL;
        if (context[i].v4l2.display) {
            need_display = true;
        }
    }

    /*
     * Open/format all cameras first (CREATE_PIPELINE on VIDIOC_G_FMT), then
     * display+QBUF so ISP CMA is stable. STREAMON only in v4l2_drm_run.
     * Sort by /dev/videoN so ISP ports probe in order (port2/i2c-4 last).
     */
    qsort(context, num, sizeof(context[0]), compare_context_by_device);
    for (i = 0; i < num; i++) {
        if (open_and_format_video(&context[i].v4l2) < 0) {
            cleanup_contexts(context, num);
            return -1;
        }
    }

    for (i = 0; i < num; i++) {
        struct v4l2_drm_context* ctx = &context[i].v4l2;

        if (!ctx->display) {
            continue;
        }

        if (d == NULL) {
            d = display_init(0);
            if (!d) {
                cleanup_contexts(context, num);
                return -1;
            }
        }

        if (setup_display_plane(d, &context[i]) < 0) {
            display_exit(d);
            cleanup_contexts(context, num);
            return -1;
        }

        if (queue_dmabuf_buffers(ctx, used_fds, &used_count) < 0) {
            display_exit(d);
            cleanup_contexts(context, num);
            return -1;
        }
    }

    if (display) {
        *display = need_display ? d : NULL;
    } else if (d) {
        display_exit(d);
    }
    return 0;
}
