#pragma once
#ifndef __THREE_CAMERA_H__
#define __THREE_CAMERA_H__

#include <v4l2-drm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THREE_CAMERA_MAX 3

struct three_camera_context {
    struct v4l2_drm_context v4l2;
    unsigned video_layer; /* 1=video_1, 2=video_2, 3=video_3, 0=auto */
};

void three_camera_default_context(struct three_camera_context* ctx);
/* video_layer: 1=video_1, 2=video_2, 3=video_3 (K230 VO hardware layer) */
struct display_plane* three_camera_get_video_plane(struct display* display, unsigned int fourcc,
                                                 unsigned video_layer);
int three_camera_setup(struct three_camera_context context[], unsigned num,
                       struct display** display);

#ifdef __cplusplus
}
#endif

#endif
