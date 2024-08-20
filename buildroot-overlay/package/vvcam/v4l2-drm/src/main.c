#include "common.h"
#include "display.h"
#include <drm_fourcc.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <memory.h>
#include <semaphore.h>
#include <poll.h>
#include <signal.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <getopt.h>

#define DRM_BUFFERING 2

static int running = 0xff;

void sighandler(int x) {
    running = 0;
}

struct video_context {
    unsigned width;
    unsigned height;
    unsigned crop_x, crop_y, crop_w, crop_h;
    unsigned device;
    int video_fd;
    unsigned frame_count;
    uint32_t video_format;
    uint32_t display_format;
    bool display;
    unsigned buffer_num;
    struct display_plane* plane;
    struct display_buffer** buffers;
    unsigned offset_x;
    unsigned offset_y;
    bool flag_dqbuf;
    uint8_t wp;
    struct v4l2_buffer vbuffer;
    int buffer_hold[DRM_BUFFERING];
    bool flag_dump;
    void** mmap;
};

static void video_context_default(struct video_context* ctx) {
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
    ctx->crop_w = 0;
    ctx->crop_h = 0;
    ctx->crop_x = 0;
    ctx->crop_y = 0;
}

static uint32_t to_v4l2_fourcc(const char* fourcc) {
    return v4l2_fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
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

static unsigned dump_count = 0;

static void dump_file(const struct video_context* ctx, unsigned channel) {
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

static void help(const char* argv0) {
    printf("Usage: %s -d 1 -w 480 -h 320\n", argv0);
    printf(
        "Options:\n"
        "\t-d Video device number\n"
        "\t-w Width\n"
        "\t-h Height\n"
        "\t-n Buffer number\n"
        "\t-f Format, NV12/NV16\n"
        "\t-s Disable display\n"
        "\t-x | --crop-x Crop offset X\n"
        "\t-y | --crop-y Crop offset Y\n"
        "\t--crop-width  Crop width\n"
        "\t--crop-height Crop height\n"
    );
}

static int parse_cmd(int argc, char* argv[], struct video_context* context) {
    int ch;
    int option_index = 0;
    int context_idx = -1;

    struct option longopt[] = {
        {
            "crop-x",
            required_argument,
            NULL,
            'x'
        },
        {
            "crop-y",
            required_argument,
            NULL,
            'y'
        },
        {
            "crop-width",
            required_argument,
            NULL,
            257,
        },
        {
            "crop-height",
            required_argument,
            NULL,
            258
        },
        {0, 0, 0, 0}
    };

    while((ch = getopt_long_only(argc, argv, "w:h:d:n:f:sx:y:", longopt, &option_index)) != -1) {
        if ((context_idx < 0) && (ch != 'd')) {
            help(argv[0]);
            return -1;
        }
        switch (ch) {
            case 'w':
                context[context_idx].width = atoi(optarg);
                break;
            case 'h':
                context[context_idx].height = atoi(optarg);
                break;
            case 'd':
                context_idx += 1;
                video_context_default(&context[context_idx]);
                context[context_idx].device = atoi(optarg);
                break;
            case 'n':
                context[context_idx].buffer_num = atoi(optarg);
                break;
            case 'f':
                context[context_idx].video_format = to_v4l2_fourcc(optarg);
                context[context_idx].display_format = v4l2_to_drm(context[context_idx].video_format);
                if (context[context_idx].display_format == 0) {
                    context[context_idx].display = false;
                }
                break;
            case 's':
                // disable display
                context[context_idx].display = false;
                break;
            case 'x':
                context[context_idx].crop_x = atoi(optarg);
                break;
            case 'y':
                context[context_idx].crop_y = atoi(optarg);
                break;
            case 257:
                context[context_idx].crop_w = atoi(optarg);
                break;
            case 258:
                context[context_idx].crop_h = atoi(optarg);
                break;
            default:
                help(argv[0]);
                return -1;
        }
    }

    return context_idx + 1;
}

int main(int argc, char* argv[]) {
    struct video_context context[9];

    int parse_result = parse_cmd(argc, argv, context);
    if (parse_result < 0) {
        return -1;
    }

    unsigned context_count = parse_result;
    unsigned flag_enable_display = 0;
    for (unsigned i = 0; i < context_count; i++) {
        // print message
        pr("/dev/video%u: %ux%u@%c%c%c%c display(%u) crop: x(%u) y(%u) w(%u) h(%u)",
            context[i].device, context[i].width, context[i].height,
            (context[i].video_format >> 0) & 0xff,
            (context[i].video_format >> 8) & 0xff,
            (context[i].video_format >> 16) & 0xff,
            (context[i].video_format >> 24) & 0xff,
            context[i].display,
            context[i].crop_x, context[i].crop_y,
            context[i].crop_w, context[i].crop_h
        );
        if (context[i].display) {
            flag_enable_display = 1;
        }
    }
    struct display* display = NULL;
    if (flag_enable_display) {
        display = display_init(0);
        CKE(display == NULL, error);
    }
    for (unsigned i = 0; i < context_count; i++) {
        // TODO: fit size
        context[i].offset_y = i * 400;
        if (context[i].display) {
            context[i].plane = display_get_plane(display, context[i].display_format);
            CKE(context[i].plane == NULL, display_exit);
            context[i].buffers = malloc(sizeof(struct display_buffer) * context[i].buffer_num);
            for (unsigned j = 0; j < context[i].buffer_num; j++) {
                context[i].buffers[j] = display_allocate_buffer(context[i].plane, context[i].width, context[i].height);
                CKE(context[i].buffers[j] == NULL, display_exit);
            }
        }

        // v4l2
        char cam_device_path[64];
        snprintf(cam_device_path, sizeof(cam_device_path) - 1, "/dev/video%u", context[i].device);
        context[i].video_fd = open(cam_device_path, O_RDWR | O_NONBLOCK);
        CKE(context[i].video_fd < 0, display_exit);

        struct v4l2_capability capbility;
        CKE(ioctl(context[i].video_fd, VIDIOC_QUERYCAP, &capbility), close);
        // pr("driver: %s", capbility.driver);
        // pr("card: %s", capbility.card);
        // pr("bus_info: %s", capbility.bus_info);

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

        // set crop
        if (context[i].crop_x || context[i].crop_y || context[i].crop_w || context[i].crop_h) {
            if (context[i].crop_w == 0) {
                context[i].crop_w = context[i].width - context[i].crop_x;
            }
            if (context[i].crop_h == 0) {
                context[i].crop_h = context[i].height - context[i].crop_y;
            }

            struct v4l2_selection sel;
            memset(&sel,0,sizeof(sel));
            sel.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            sel.target = V4L2_SEL_TGT_CROP_BOUNDS;
            CKE(ioctl(context[i].video_fd, VIDIOC_G_SELECTION, &sel), close);
            pr(
                "/dev/video%u crop bound left(%u) top(%u) width(%u) height(%u)",
                context[i].device,
                sel.r.left, sel.r.top, sel.r.width, sel.r.height
            );
            sel.target = V4L2_SEL_TGT_CROP_DEFAULT;
            CKE(ioctl(context[i].video_fd, VIDIOC_G_SELECTION, &sel), close);
            sel.target = V4L2_SEL_TGT_CROP;
            sel.r.left = context[i].crop_x;
            sel.r.top = context[i].crop_y;
            sel.r.width = context[i].crop_w;
            sel.r.height = context[i].crop_h;
            CKE(ioctl(context[i].video_fd, VIDIOC_S_SELECTION, &sel), close);
        }

        struct v4l2_format format;
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        CKE(ioctl(context[i].video_fd, VIDIOC_G_FMT, &format), close);
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        format.fmt.pix.pixelformat = context[i].video_format;
        format.fmt.pix.width = context[i].width;
        format.fmt.pix.height = context[i].height;
        CKE(ioctl(context[i].video_fd, VIDIOC_S_FMT, &format), close);
        // pr("Set Video Format: %ux%u@%s",
        //     format.fmt.pix.width, format.fmt.pix.height,
        //     (char*)&format.fmt.pix.pixelformat
        // );

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

        // run
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        CKE(ioctl(context[i].video_fd, VIDIOC_STREAMON, &type), close);
    }

    if (flag_enable_display) {
        for (unsigned i = 0; i < context_count; i++) {
            if (context[i].display) {
                display_commit_buffer(context[i].buffers[0], 0, 0);
            }
        }
    }

    #define RECORD_FRAME 0
    #if RECORD_FRAME
    FILE* f = fopen("/tmp/record.yuv", "w");
    #endif

    int flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag)) {
        pr("can't set stdin non-block");
        goto streamoff;
    }

    fprintf(
        stderr,
        "Input character to select test option\n"
        "\td: dump image\n"
        "\tq: quit\n"
    );

    unsigned display_frame_count = 0;
    unsigned response = 0;
    struct timeval tv, tv2;
    gettimeofday(&tv, NULL);
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    while (running) {
        // FIXME: select?
        struct pollfd fds[context_count + flag_enable_display];
        for (unsigned i = 0; i < context_count; i++) {
            fds[i].fd = context[i].video_fd;
            fds[i].events = POLLIN | POLLPRI;
            fds[i].revents = 0;
        }
        if (flag_enable_display) {
            fds[context_count].fd = display->fd;
            fds[context_count].events = POLLIN | POLLPRI;
            fds[context_count].revents = 0;
        }
        int ret = poll(fds, context_count + flag_enable_display, 1000);
        response += 1;
        if (((ret < 0) && (errno == EINTR)) || (ret == 0)) {
            continue;
        } else if (ret < 0) {
            pr("poll error %d(%s)", errno, strerror(errno));
            break;
        }

        {
            char c;
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if ((n > 0) && (c != '\n')) {
                switch (c) {
                    case 'q': goto streamoff;
                    case 'd':
                        dump_count += 1;
                        for (unsigned i = 0; i < context_count; i++) {
                            context[i].flag_dump = true;
                        }
                        break;
                    default: break;
                }
            }
        }

        #define DEBUG_SEQ 0
        for (unsigned i = 0; i < context_count; i++) {
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
                    #if RECORD_FRAME
                    fwrite(context[i].mmap[context[i].vbuffer.index], 1, context[i].buffers[0]->size, f);
                    #endif
                    #if DEBUG_SEQ
                    pr("%u DQBUF %u --", i, context[i].vbuffer.index);
                    pr("%u QBUF  %u --", i, context[i].vbuffer.index);
                    #endif
                    ioctl(context[i].video_fd, VIDIOC_QBUF, &context[i].vbuffer);
                    continue;
                }
                context[i].wp = (context[i].wp + 1) % DRM_BUFFERING;
                if (context[i].buffer_hold[context[i].wp] >= 0) {
                    // QBUF displayed frame
                    context[i].vbuffer.index = context[i].buffer_hold[context[i].wp];
                    #if DEBUG_SEQ
                    pr("%u QBUF  %u", i, context[i].vbuffer.index);
                    #endif
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
                #if RECORD_FRAME
                fwrite(context[i].mmap[context[i].vbuffer.index], 1, context[i].buffers[0]->size, f);
                #endif
                #if DEBUG_SEQ
                pr("%u DQBUF %u", i, context[i].vbuffer.index);
                #endif
                context[i].frame_count += 1;
                context[i].buffer_hold[context[i].wp] = context[i].vbuffer.index;
                context[i].flag_dqbuf = true;
            }
        }
        if (flag_enable_display && fds[context_count].revents) {
            // display
            bool flag_check_source = false;
            for (unsigned i = 0; i < context_count; i++) {
                if ((context[i].buffer_hold[context[i].wp] >= 0) && (context[i].display)) {
                    flag_check_source = true;
                    break;
                }
            }
            if (!flag_check_source) {
                // skip
                continue;
            }
            display_handle_vsync(display);
            for (unsigned i = 0; i < context_count; i++) {
                if ((!context[i].display) || (context[i].buffer_hold[context[i].wp] < 0)) {
                    continue;
                }
                CKE(display_update_buffer(
                    context[i].buffers[context[i].buffer_hold[context[i].wp]],
                    context[i].offset_x, context[i].offset_y
                ), streamoff);
                context[i].flag_dqbuf = false;
                #if DEBUG_SEQ
                pr("%u DISP  %d", i, context[i].buffer_hold[context[i].wp]);
                #endif
            }
            CKE(display_commit(display), streamoff);
            display_frame_count += 1;
        }
        
        // FPS counter
        #define FPS_COUNTER !DEBUG_SEQ
        #if FPS_COUNTER
        gettimeofday(&tv2, NULL);
        uint64_t duration = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
        if (duration >= 1000000) {
            fprintf(stderr, " poll: %.2f, ", response * 1000000. / duration);
            response = 0;
            if (flag_enable_display) {
                fprintf(stderr, "display: %.2f, ", display_frame_count * 1000000. / duration);
                display_frame_count = 0;
            }
            for (unsigned i = 0; i < context_count; i++) {
                fprintf(stderr, "[%u]: %.2f, ", i, context[i].frame_count * 1000000. / duration);
                context[i].frame_count = 0;
            }
            fprintf(stderr, "          \r");
            fflush(stderr);
            gettimeofday(&tv, NULL);
        }
        #endif
    }
streamoff:
    fprintf(stderr, "\n");

    for (unsigned i = 0; i < context_count; i++) {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(context[i].video_fd, VIDIOC_STREAMOFF, &type);
    }

close:
    for (unsigned i = 0; i < context_count; i++) {
        if (context[i].video_fd >= 0) {
            close(context[i].video_fd);
        }
    }
    #if RECORD_FRAME
    fclose(f);
    #endif
display_exit:
    if (flag_enable_display)
        display_exit(display);
error:
    return 0;
}
