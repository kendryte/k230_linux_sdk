#pragma once
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <xf86drmMode.h>
#include <xf86drm.h>
#include <drm/drm_fourcc.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PROPS 128
#define DISPLAY_QUEUE_DEPTH 3

struct display {
    int fd;
    uint32_t conn_id, enc_id, crtc_id, blob_id;
    int crtc_idx;
    uint32_t width, height;
    uint32_t mmWidth, mmHeight;
    drmModeModeInfo mode;
    drmModeCrtcPtr crtc;
    drmModePropertyPtr crtc_props[MAX_PROPS];
    uint32_t crtc_props_count;
    drmModeConnectorPtr conn;
    drmModePropertyPtr conn_props[MAX_PROPS];
    uint32_t conn_props_count;
    drmModeAtomicReqPtr req;
    uint32_t commitFlags;
    drmEventContext drm_event_ctx;
    struct display_plane* planes;
};

struct display_buffer {
    struct display_buffer* next;
    struct display_plane* plane;
    uint32_t handle;
    uint32_t stride, width, height;
    uint32_t size;
    int dmabuf_fd;
    uint32_t id;
    void* map;
};

struct display_plane {
    struct display_plane* next;
    struct display* display;
    drmModePlanePtr plane;
    drmModePropertyPtr props[MAX_PROPS];
    uint8_t props_count;
    uint32_t plane_id;
    unsigned int fourcc;
    bool first;
    struct display_buffer* buffers;
};

void display_exit(struct display* display);
struct display* display_init(unsigned device);
struct display_plane* display_get_plane(struct display* display, unsigned int fourcc);
void display_free_plane(struct display_plane* plane);
struct display_buffer* display_allocate_buffer(struct display_plane* plane, uint32_t width, uint32_t height);
void display_free_plane(struct display_plane* plane);
void display_free_buffer(struct display_buffer* buffer);
int display_commit_buffer(const struct display_buffer* buffer, uint32_t x, uint32_t y);
int display_update_buffer(struct display_buffer* buffer, uint32_t x, uint32_t y);
int display_commit(struct display* display);
void display_wait_vsync(struct display* display);
void display_handle_vsync(struct display* display);

#ifdef __cplusplus
}
#endif

#endif
