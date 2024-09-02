#include "common.h"
#include "display.h"
#include "v4l2-drm.h"
#include <display.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <unistd.h>

void v4l2_drm_default_context(struct v4l2_drm_context* ctx) {
    ctx->width = 640;
    ctx->height = 480;
    ctx->device = 0;
    ctx->video_fd = -1;
    ctx->video_format = V4L2_PIX_FMT_NV12;
    ctx->display_format = DRM_FORMAT_NV12;
    ctx->display = true;
    ctx->buffer_num = DRM_BUFFERING + 3;
    ctx->plane = NULL;
    ctx->flag_dqbuf = true;
    ctx->offset_x = 0;
    ctx->offset_y = 0;
    ctx->frame_count = 0;
    ctx->flag_dqbuf = false;
    for (unsigned i = 0; i < DRM_BUFFERING; i++) {
        ctx->buffer_hold[i] = -1;
    }
    ctx->wp = 0;
    ctx->flag_dump = false;
}

static uint32_t v4l2_to_drm(uint32_t fourcc) {
    switch (fourcc) {
        case V4L2_PIX_FMT_NV12: return DRM_FORMAT_NV12;
        case V4L2_PIX_FMT_NV21: return DRM_FORMAT_NV21;
        case V4L2_PIX_FMT_NV16: return DRM_FORMAT_NV16;
        case V4L2_PIX_FMT_NV61: return DRM_FORMAT_NV61;
        case V4L2_PIX_FMT_BGR24: return DRM_FORMAT_BGR888;
        case V4L2_PIX_FMT_RGB24: return DRM_FORMAT_RGB888;
        case V4L2_PIX_FMT_YUYV: return DRM_FORMAT_YUYV;
        default:
            pr(
                "no plane for video format %c%c%c%c",
                (fourcc >> 0) & 0xff,
                (fourcc >> 8) & 0xff,
                (fourcc >> 16) & 0xff,
                (fourcc >> 24) & 0xff
                );
            return 0;
    }
}

int v4l2_drm_setup(struct v4l2_drm_context context[], unsigned num, struct display** display) {
    struct display* d = NULL;
    if (display && *display) {
        d = *display;
    }
    for (unsigned i = 0; i < num; i++) {
        if (context[i].display && context[i].plane == NULL) {
            if (d == NULL) {
                d = display_init(0);
                CKE(d == NULL, close);
            }
            if (context[i].display_format == 0) {
                context[i].display_format = v4l2_to_drm(context[i].video_format);
                CKE(context[i].display_format == 0, close);
            }
            context[i].plane = display_get_plane(d, context[i].display_format);
            CKE(context[i].plane == NULL, close);
            context[i].buffers = malloc(sizeof(struct display_buffer*) * context[i].buffer_num);
            for (unsigned j = 0; j < context[i].buffer_num; j++) {
                CKE(display_allocate_buffer(context[i].plane, context[i].width, context[i].height) == NULL, close);
            }
        }
        char cam_device_path[64];
        snprintf(cam_device_path, sizeof(cam_device_path) - 1, "/dev/video%u", context[i].device);
        context[i].video_fd = open(cam_device_path, O_RDWR | O_NONBLOCK);
        CKE(context[i].video_fd < 0, close);

        struct v4l2_capability capbility;
        CKE(ioctl(context[i].video_fd, VIDIOC_QUERYCAP, &capbility), close);

        struct v4l2_fmtdesc fmtdesc;
        memset(&fmtdesc, 0, sizeof(fmtdesc));
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        while (ioctl(context[i].video_fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
            pr(
                "/dev/video%u support format %c%c%c%c",
                context[i].device,
                (fmtdesc.pixelformat >> 0) & 0xff,
                (fmtdesc.pixelformat >> 8) & 0xff,
                (fmtdesc.pixelformat >> 16) & 0xff,
                (fmtdesc.pixelformat >> 24) & 0xff
            );
            fmtdesc.index += 1;
        }

        struct v4l2_format format;
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        CKE(ioctl(context[i].video_fd, VIDIOC_G_FMT, &format), close);
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        format.fmt.pix.pixelformat = context[i].video_format;
        format.fmt.pix.width = context[i].width;
        format.fmt.pix.height = context[i].height;
        CKE(ioctl(context[i].video_fd, VIDIOC_S_FMT, &format), close);

        struct v4l2_requestbuffers request_buffer;
        memset(&request_buffer, 0, sizeof(request_buffer));
        request_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (context[i].display) {
            request_buffer.memory = V4L2_MEMORY_DMABUF;
        } else {
            request_buffer.memory = V4L2_MEMORY_MMAP;
        }
        request_buffer.count = context[i].buffer_num;
        CKE(ioctl(context[i].video_fd, VIDIOC_REQBUFS, &request_buffer), close);
        context[i].mmap = malloc(sizeof(void*) * context[i].buffer_num);
        if (context[i].display) {
            struct display_buffer* db = context[i].plane->buffers;
            for (unsigned j = 0; j < context[i].buffer_num; j++) {
                memset(&context[i].vbuffer, 0, sizeof(context[i].vbuffer));
                context[i].vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                context[i].vbuffer.memory = V4L2_MEMORY_DMABUF;
                context[i].vbuffer.index = j;
                context[i].vbuffer.m.fd = db->dmabuf_fd;
                context[i].vbuffer.length = db->size;
                CKE(ioctl(context[i].video_fd, VIDIOC_QBUF, &context[i].vbuffer), close);
                context[i].buffers[j] = db;
                context[i].mmap[j] = db->map;
                db = db->next;
            }
        } else {
            for (unsigned j = 0; j < context[i].buffer_num; j++) {
                memset(&context[i].vbuffer, 0, sizeof(context[i].vbuffer));
                context[i].vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                context[i].vbuffer.memory = V4L2_MEMORY_MMAP;
                context[i].vbuffer.index = j;
                CKE(ioctl(context[i].video_fd, VIDIOC_QUERYBUF, &context[i].vbuffer), close);
                CKE(ioctl(context[i].video_fd, VIDIOC_QBUF, &context[i].vbuffer), close);
                context[i].mmap[j] = mmap(
                    NULL,
                    context[i].vbuffer.length,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    context[i].video_fd,
                    context[i].vbuffer.m.offset
                );
                CKE(context[i].mmap[j] == MAP_FAILED, close);
            }
        }
        continue;

        close:
        for (unsigned j = 0; j <= i; j++) {
            close(context[i].video_fd);
            context[i].video_fd = -1;
        }
        if (d) {
            display_exit(d);
        }
        return -1;
    }
    if (display) {
        *display = d;
    }
    return 0;
}

static unsigned dump_count = 0;

static void dump_file(const struct v4l2_drm_context* ctx, unsigned channel) {
    char filename[128];
    snprintf(
        filename,
        sizeof(filename),
        "Image_%u_%u_%ux%u.%c%c%c%c",
        channel, dump_count,
        ctx->width, ctx->height,
        (ctx->video_format >> 0) & 0xff,
        (ctx->video_format >> 8) & 0xff,
        (ctx->video_format >> 16) & 0xff,
        (ctx->video_format >> 24) & 0xff
    );
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        pr("open %s error %d(%s)", filename, errno, strerror(errno));
        return;
    }
    fwrite(ctx->mmap[ctx->vbuffer.index], 1, ctx->vbuffer.length, f);
    fclose(f);
    pr("dump file to %s", filename);
}

int v4l2_drm_run(struct v4l2_drm_context context[], unsigned num, v4l2_drm_handler handler) {
    int flag_enable_display = 0;
    int display_fd;
    struct display* d = NULL;
    for (unsigned i = 0; i < num; i++) {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        CKE(ioctl(context[i].video_fd, VIDIOC_STREAMON, &type), streamerr);
        if (context[i].display) {
            if (flag_enable_display == 0) {
                // trig vsync
                display_commit_buffer(context[i].buffers[0], context[i].offset_x, context[i].offset_y);
            }
            flag_enable_display = 1;
            d = context[i].plane->display;
            display_fd = d->fd;
        }
        continue;
        streamerr:
        for (unsigned j = 0; j < i; j++) {
            ioctl(context[i].video_fd, VIDIOC_STREAMOFF, &type);
        }
        return -1;
    }
    uint32_t display_frame_count = 0;
    uint32_t frame_count_fps[num];
    memset(frame_count_fps, 0, sizeof(uint32_t) * num);
    struct pollfd fds[num + flag_enable_display];
    struct timeval tv, tv2;
    gettimeofday(&tv, NULL);
    while (1) {
        for (unsigned i = 0; i < num; i++) {
            fds[i].fd = context[i].video_fd;
            fds[i].events = POLLIN | POLLPRI;
            fds[i].revents = 0;
        }
        if (flag_enable_display) {
            fds[num].fd = display_fd;
            fds[num].events = POLLIN | POLLPRI;
            fds[num].revents = 0;
        }
        int ret = poll(fds, num + flag_enable_display, 1000);
        if (((ret < 0) && (errno == EINTR)) || (ret == 0)) {
            continue;
        } else if (ret < 0) {
            pr("poll error %d(%s)", errno, strerror(errno));
            break;
        }

        for (unsigned i = 0; i < num; i++) {
            context[i].vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (context[i].display) {
                context[i].vbuffer.memory = V4L2_MEMORY_DMABUF;
            } else {
                context[i].vbuffer.memory = V4L2_MEMORY_MMAP;
            }
            if (fds[i].revents) {
                if (context[i].flag_dqbuf) {
                    // drop frame
                    if (ioctl(context[i].video_fd, VIDIOC_DQBUF, &context[i].vbuffer)) {
                        continue;
                    }
                    context[i].frame_count += 1;
                    if (context[i].flag_dump) {
                        dump_file(&context[i], i);
                        context[i].flag_dump = false;
                    }
                    ioctl(context[i].video_fd, VIDIOC_QBUF, &context[i].vbuffer);
                    continue;
                }
                context[i].wp = (context[i].wp + 1) % DRM_BUFFERING;
                if (context[i].buffer_hold[context[i].wp] >= 0) {
                    // QBUF displayed frame
                    context[i].vbuffer.index = context[i].buffer_hold[context[i].wp];
                    ioctl(context[i].video_fd, VIDIOC_QBUF, &context[i].vbuffer);
                }
                // DQBUF
                if (ioctl(context[i].video_fd, VIDIOC_DQBUF, &context[i].vbuffer) < 0) {
                    // error, skip this frame
                    continue;
                }
                if (context[i].flag_dump) {
                    dump_file(&context[i], i);
                    context[i].flag_dump = false;
                }
                context[i].frame_count += 1;
                context[i].buffer_hold[context[i].wp] = context[i].vbuffer.index;
                context[i].flag_dqbuf = true;
            }
        }

        int ch = 0;
        if (handler) {
            ch = handler(context, flag_enable_display && fds[num].revents);
            switch (ch) {
                case -1:
                case 'q': goto streamoff;
                case 'd':
                    dump_count += 1;
                    for (unsigned i = 0; i < num; i++) {
                        context[i].flag_dump = true;
                    }
                    break;
                default:
                    break;
            }
        }

        if (flag_enable_display && fds[num].revents) {
            // display
            bool flag_check_source = false;
            for (unsigned i = 0; i < num; i++) {
                if ((context[i].buffer_hold[context[i].wp] >= 0) && (context[i].display)) {
                    flag_check_source = true;
                    break;
                }
            }
            if (!flag_check_source) {
                // skip
                continue;
            }
            display_handle_vsync(d);
            for (unsigned i = 0; i < num; i++) {
                if ((!context[i].display) || (context[i].buffer_hold[context[i].wp] < 0)) {
                    continue;
                }
                CKE(display_update_buffer(
                    context[i].buffers[context[i].buffer_hold[context[i].wp]],
                    context[i].offset_x, context[i].offset_y
                ), streamoff);
                context[i].flag_dqbuf = false;
            }
            CKE(display_commit(d), streamoff);
            display_frame_count += 1;
        }
    }
    streamoff:
    for (unsigned i = 0; i < num; i++) {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(context[i].video_fd, VIDIOC_STREAMOFF, &type);
        close(context[i].video_fd);
    }
    return 0;
}
