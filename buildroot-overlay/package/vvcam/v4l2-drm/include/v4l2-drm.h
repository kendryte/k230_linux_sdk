#pragma once
#ifndef __V4L2_DRM_H__
#define __V4L2_DRM_H__

#define DRM_BUFFERING 2

#include "display.h"
#include <stdint.h>
#include <stdbool.h>
#include <linux/videodev2.h>

#ifdef __cplusplus
extern "C" {
#endif

struct v4l2_drm_video_buffer {
    void* mmap;
    int fd;
    unsigned index;
};

struct v4l2_drm_context {
    unsigned width;
    unsigned height;
    unsigned device;
    int video_fd;
    unsigned frame_count;
    uint32_t video_format;
    uint32_t display_format;
    bool display;
    unsigned buffer_num;
    struct display_plane* plane;
    struct display_buffer** display_buffers;
    struct v4l2_drm_video_buffer* buffers;
    unsigned offset_x;
    unsigned offset_y;
    bool flag_dqbuf;
    uint8_t wp;
    struct v4l2_buffer vbuffer;
    int buffer_hold[DRM_BUFFERING];
    bool flag_dump;
};


typedef int(*v4l2_drm_handler)(struct v4l2_drm_context* ctx, bool displayed);

void v4l2_drm_default_context(struct v4l2_drm_context* ctx);
// use /dev/dri/card0 as default
int v4l2_drm_setup(struct v4l2_drm_context context[], unsigned num, struct display** display);
/**
 *
 * @param fps Array of FPS output, NULL if not used
 */
int v4l2_drm_run(struct v4l2_drm_context ctx[], unsigned num, v4l2_drm_handler handler);

int v4l2_drm_start(const struct v4l2_drm_context* context);
int v4l2_drm_stop(const struct v4l2_drm_context* context);
int v4l2_drm_dump(struct v4l2_drm_context* context, int timeout);
int v4l2_drm_dump_release(struct v4l2_drm_context* context);

#ifdef __cplusplus
}
#endif

#endif
