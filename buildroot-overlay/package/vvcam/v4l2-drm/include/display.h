#pragma once
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <xf86drmMode.h>
#include <xf86drm.h>
#include <stdbool.h>

#define MAX_PROPS 128

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
    uint32_t count_planes;
    uint32_t capacity_planes;
    drmModeAtomicReqPtr req;
    drmEventContext drm_event_ctx;
    struct display_plane** planes;
};

struct display_plane {
    struct display* display;
    drmModePlanePtr plane;
    drmModePropertyPtr props[MAX_PROPS];
    uint8_t props_count;
    uint32_t plane_id;
    unsigned int fourcc;
    bool first;
};

struct display_buffer {
    struct display_plane* plane;
    uint32_t handle;
    uint32_t stride, width, height;
    uint32_t size;
    int fd;
    uint32_t id;
    void* map;
};

void display_exit(struct display* display);

int display_init(unsigned device, struct display* display);

int display_get_plane(struct display* display, unsigned int fourcc, struct display_plane* display_plane);

int display_allocate_buffer(
    struct display_plane* plane, struct display_buffer* buffer,
    uint32_t width, uint32_t height
);

int display_commit_buffer(const struct display_buffer* buffer, uint32_t x, uint32_t y);

void display_wait_vsync(struct display* display);

#endif
