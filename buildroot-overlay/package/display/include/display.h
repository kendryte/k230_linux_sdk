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


enum drm_rotation {
    rotation_0 = 0,
    rotation_90 = 1,
    rotation_180 = 2,
    rotation_270 = 3,
    rotation_reflect_x = 4,
    rotation_reflect_y = 5,
};
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
    enum drm_rotation drm_rotation;
    struct display_plane* planes; //plane链表头
    struct display_buffer* osd_disp_buffer;   //OSD layer buffer to commit on next vsync
    struct display_buffer* lvgl_disp_buffer;  //LVGL layer buffer to commit on next vsync
    uint32_t frame_count;                     //total committed frames
};




struct display_buffer {
    struct display_buffer* next;
    struct display_plane* plane;
    uint32_t handle;
    uint32_t stride, width, height;
    uint32_t size;
    int dmabuf_fd;
    uint32_t id;
    enum drm_rotation drm_rotation;
    void* map;
    bool imported;  //came from display_import_buffer: the memory belongs to someone else

    /*
     * Chroma kept in a second allocation of its own. Only the two-plane calls
     * below fill these in; everywhere else they are 0 / -1 / NULL and the
     * chroma sits inside the first allocation at offset stride x height.
     */
    uint32_t uv_handle;
    int uv_dmabuf_fd;
    uint32_t uv_size;
    void* uv_map;
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
    enum drm_rotation drm_rotation;
    struct display_buffer* buffers;
};

void display_exit(struct display* display);
struct display* display_init(unsigned device);
struct display_plane* display_get_plane(struct display* display, unsigned int fourcc);
void display_free_plane(struct display_plane* plane);
struct display_buffer* display_allocate_buffer(struct display_plane* plane, uint32_t width, uint32_t height);
/*
 * The same frame, but with each plane in an allocation of its own, so that it
 * can be exported as one dmabuf per plane. @nplanes 1 is display_allocate_buffer();
 * 2 is only valid for the semiplanar YUV formats.
 *
 * DRM does not mind either layout - it looks the GEM object up per plane, so a
 * framebuffer may be stitched together from two of them. V4L2 does mind: a
 * device that reports NV12 as two planes wants one dmabuf fd per plane, and the
 * VPU driver zeroes the per-plane data_offset on every QBUF, so aiming both
 * planes at one allocation with different offsets would lay the chroma on top
 * of the luma. That is the only reason this exists.
 */
struct display_buffer* display_allocate_buffer_planes(struct display_plane* plane, uint32_t width,
                                                      uint32_t height, unsigned nplanes);
/*
 * Show memory that was allocated elsewhere, typically a camera buffer exported
 * with VIDIOC_EXPBUF, without copying it. @stride is the bytes per line of the
 * first plane, 0 to derive it from @width. @dmabuf_fd stays owned by the
 * caller: display_free_buffer() will not close it.
 */
struct display_buffer* display_import_buffer(struct display_plane* plane, int dmabuf_fd,
                                             uint32_t width, uint32_t height, uint32_t stride);
/*
 * Describe memory that is already split one dmabuf per plane, as
 * display_allocate_buffer_planes(.., 2) produces. @nfds 1 behaves exactly like
 * display_import_buffer(); 2 puts the chroma at the start of the second fd.
 * None of the fds are closed by display_free_buffer().
 */
struct display_buffer* display_import_buffer_planes(struct display_plane* plane,
                                                    const int* dmabuf_fds, unsigned nfds,
                                                    uint32_t width, uint32_t height,
                                                    uint32_t stride);
void display_free_plane(struct display_plane* plane);
void display_free_buffer(struct display_buffer* buffer);
int display_commit_buffer(const struct display_buffer* buffer, uint32_t x, uint32_t y);
int display_update_buffer(struct display_buffer* buffer, uint32_t x, uint32_t y);
int display_commit(struct display* display);
void display_wait_vsync(struct display* display);
void display_handle_vsync(struct display* display);
int display_commit_buffer_noblock(const struct display_buffer* buffer, uint32_t x, uint32_t y);
#ifdef __cplusplus
}
#endif

#endif
