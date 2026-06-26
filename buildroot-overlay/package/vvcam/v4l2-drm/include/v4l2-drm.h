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

struct v4l2_crop_size {
    uint32_t width;
    uint32_t height;
    uint32_t offset_x;
    uint32_t offset_y;
    uint32_t crop_en;
};

struct v4l2_drm_context {
    unsigned width;
    unsigned height;
    unsigned device;
    int video_fd;
    unsigned frame_count; //sensor 帧数
    uint32_t video_format;
    uint32_t display_format;
    bool display;
    unsigned buffer_num; //显示或者v4l2 需要的buffer数量；
    struct display_plane* plane; //所有display_plane的list头；
    struct display_buffer** display_buffers;
    struct v4l2_drm_video_buffer* buffers;
    struct v4l2_crop_size crop_size;
    unsigned offset_x;
    unsigned offset_y;
    bool flag_dqbuf;  // 是否已完成 DQBUF（出队）但尚未显示" 的标志
    uint8_t wp; //Write Position，是双缓冲环形队列的写指针
    struct v4l2_buffer vbuffer; //当前使用的v4l2_buffer
    int buffer_hold[DRM_BUFFERING];
    bool flag_dump; //dump 标记
    enum drm_rotation drm_rotation;
    int8_t hflip;
    int8_t vflip;
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
int v4l2_drm_run_v4l2_2_drm(struct v4l2_drm_context ctx[], unsigned num, v4l2_drm_handler handler);


int v4l2_drm_start(const struct v4l2_drm_context* context);
int v4l2_drm_stop(const struct v4l2_drm_context* context);
int v4l2_drm_dump(struct v4l2_drm_context* context, int timeout);
int v4l2_drm_dump_release(struct v4l2_drm_context* context);
extern bool v4l2_drm_run_v4l2_2_drm_need_run;
extern struct display_buffer *g_p_osd_disp_buffer;
#ifdef __cplusplus
}
#endif

#endif
