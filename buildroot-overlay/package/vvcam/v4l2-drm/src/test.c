#include "common.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/mman.h>
#include <unistd.h>
#include <drm_fourcc.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <errno.h>

#define USE_DMABUF 1
#define DRM_BUFFERING 2

int drm_init(unsigned int fourcc, unsigned int buffer_num, uint32_t width, uint32_t height);
void drm_exit(void);
int drm_display(unsigned index);
void drm_handle_vsync(void);
void drm_get_resolution(unsigned* width, unsigned* height);
int drm_get_dmabuf_fd(unsigned index);
void* drm_get_map(unsigned index);
int drm_get_fd(void);

void help(const char* argv0) {
    pr("Usage: %s -W WIDTH -H HEIGHT -d VIDEO_DEVICE_NUMBER", argv0);
}

static int running = 0xff;

void sighandler(int x) {
    running = 0;
}

int main(int argc, char* argv[]) {
    int ch;
    int width = 0, height = 0;
    int device = 1;
    int buffer_num = DRM_BUFFERING + 3;
    bool enable_display = true;

    while((ch = getopt(argc, argv, "hW:H:d:n:")) != -1) {
        switch (ch) {
            case 'W':
                width = atoi(optarg);
                if (width < 0) {
                    pr("width must greater than 0");
                    return -1;
                }
                break;
            case 'H':
                height = atoi(optarg);
                if (height < 0) {
                    pr("width must greater than 0");
                    return -1;
                }
                break;
            case 'd':
                device = atoi(optarg);
                break;
            case 'n':
                buffer_num = atoi(optarg);
                break;
            case 'h':
            default:
                help(argv[0]);
                return -1;
        }
    }
    // display
    #if USE_DMABUF
    CKE(drm_init(DRM_FORMAT_NV12, buffer_num, width, height), error);
    #else
    CKE(drm_init(DRM_FORMAT_NV12, 3, width, height), error);
    #endif
    if (width == 0) {
        // screen resolution
        unsigned int w, h;
        drm_get_resolution(&w, &h);
        width = w;
    }
    if (height == 0) {
        // screen resolution
        unsigned int w, h;
        drm_get_resolution(&w, &h);
        height = h;
    }

    // v4l2
    char cam_device_path[100];
    sprintf(cam_device_path, "/dev/video%d", device);
    int cam_device = open(cam_device_path, O_RDWR | O_NONBLOCK);
    CKE(cam_device < 0, drm_exit);

    struct v4l2_capability capbility;
    CKE(ioctl(cam_device, VIDIOC_QUERYCAP, &capbility), close);
    pr("driver: %s", capbility.driver);
    pr("card: %s", capbility.card);
    pr("bus_info: %s", capbility.bus_info);

    struct v4l2_fmtdesc fmtdesc;
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(cam_device, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        pr("Suport Format %s",(char *)&fmtdesc.pixelformat);
        fmtdesc.index += 1;
    }

    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    CKE(ioctl(cam_device, VIDIOC_G_FMT, &format), close);
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    CKE(ioctl(cam_device, VIDIOC_S_FMT, &format), close);
    CKE(ioctl(cam_device, VIDIOC_G_FMT, &format), close);
    pr("Set Video Format: %ux%u@%s",
        format.fmt.pix.width, format.fmt.pix.height,
        (char*)&format.fmt.pix.pixelformat
    );

#if USE_DMABUF
#define V4L2_BUFFER_TYPE V4L2_MEMORY_DMABUF
#else
#define V4L2_BUFFER_TYPE V4L2_MEMORY_MMAP
    void** v4l2_buffer_map = malloc(sizeof(void*) * buffer_num);
#endif

    struct v4l2_requestbuffers request_buffer;
    memset(&request_buffer, 0, sizeof(request_buffer));
    request_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    request_buffer.memory = V4L2_BUFFER_TYPE;
    request_buffer.count = buffer_num;
    CKE(ioctl(cam_device, VIDIOC_REQBUFS, &request_buffer), close);

    struct v4l2_buffer buffer;
    for (unsigned i = 1; i < buffer_num; i++) {
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_BUFFER_TYPE;
        buffer.index = i;
        #if USE_DMABUF
        buffer.m.fd = drm_get_dmabuf_fd(i);
        buffer.length = width * height * 3 / 2;
        #else
        CKE(ioctl(cam_device, VIDIOC_QUERYBUF, &buffer), close);
        #endif
        CKE(ioctl(cam_device, VIDIOC_QBUF, &buffer), close);

        #if !USE_DMABUF
        v4l2_buffer_map[i] = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_device, buffer.m.offset);
        CKE(v4l2_buffer_map[i] == NULL, close);
        #endif
    }

    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    // run
    CKE(ioctl(cam_device, VIDIOC_STREAMON, &buffer.type), close);
    int buffer_hold[DRM_BUFFERING];
    for (unsigned i = 0; i < DRM_BUFFERING; i++) {
        buffer_hold[i] = -1;
    }
    unsigned buffer_hold_idx = 0;
    drm_display(0);
    int flag_dqbuf = 0;
    unsigned capture_fps = 0, display_fps = 0;
    int last_frame = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    while (running) {
        // FIXME: select?
        struct pollfd fds[2];
        fds[0].fd = cam_device;
        fds[0].events = POLLIN | POLLPRI;
        fds[0].revents = 0;
        fds[1].fd = drm_get_fd();
        fds[1].events = POLLIN | POLLPRI;
        fds[1].revents = 0;
        int ret = poll(fds, 2, 1000);
        if (((ret < 0) && (errno == EINTR)) || (ret == 0)) {
            continue;
        }

        if (fds[0].revents) {
            if (flag_dqbuf) {
                // drop last frame
                CKE(ioctl(cam_device, VIDIOC_QBUF, &buffer), conti);
            }
            buffer_hold_idx = (buffer_hold_idx + 1) % DRM_BUFFERING;
            if (buffer_hold[buffer_hold_idx] >= 0) {
                buffer.index = buffer_hold[buffer_hold_idx];
                CKE(ioctl(cam_device, VIDIOC_QBUF, &buffer), conti);
            }
            CKE(ioctl(cam_device, VIDIOC_DQBUF, &buffer), conti);
            buffer_hold[buffer_hold_idx] = buffer.index;
            flag_dqbuf = 1;
            capture_fps += 1;
        }

        if (fds[1].revents && (buffer_hold[buffer_hold_idx] >= 0)) {
            #if !USE_DMABUF
            memcpy(drm_get_map(buffer_hold_idx), v4l2_buffer_map[buffer_hold[buffer_hold_idx]], buffer.length);
            drm_handle_vsync();
            drm_display(buffer_hold_idx);
            #else
            drm_handle_vsync();
            drm_display(buffer_hold[buffer_hold_idx]);           
            #endif
            if (buffer_hold[buffer_hold_idx] != last_frame) {
                last_frame = buffer_hold[buffer_hold_idx];
                display_fps += 1;
            } 
            flag_dqbuf = 0;
        }

        // fps
        struct timeval tv2;
        gettimeofday(&tv2, NULL);
        if ((tv2.tv_sec - tv.tv_sec) * 1000000 + tv2.tv_usec - tv.tv_usec >= 1000000) {
            fprintf(stderr, "  capture: %u FPS, display: %u FPS    \r",
                capture_fps, display_fps
            );
            capture_fps = 0;
            display_fps = 0;
            tv = tv2;
        }

        continue;
        conti:
        pr("buffer idx: %u", buffer.index);
        continue;
    }
    fprintf(stderr, "\n");

close:
    close(cam_device);
drm_exit:
    drm_exit();
error:
    return 0;
}
