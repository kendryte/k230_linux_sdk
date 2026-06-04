#include <display.h>

#include <cstdio>
#include <cstring>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <riscv_vector.h>
#include <pthread.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <errno.h>

#define pr(fmt, ...) fprintf(stderr, "[display] " fmt "\n", ##__VA_ARGS__)
#define CKE(x, go) do { int e = (x); if (e) { pr("[display] "#x" error %d(%s) at line %d", e, strerror(-e), __LINE__); goto go; } } while (0)


// 从 display.c 复制的辅助函数
static uint32_t get_plane_property_id(const struct display_plane* plane, const char* name) {
    for (unsigned i = 0; i < plane->props_count; i++) {
        if (strcmp(name, plane->props[i]->name) == 0) {
            return plane->props[i]->prop_id;
        }
    }
    pr("plane prop %s not found", name);
    return 0xDEADDEAD;
}

static uint32_t get_crtc_property_id(const struct display* display, const char* name) {
    for (unsigned i = 0; i < display->crtc_props_count; i++) {
        if (strcmp(name, display->crtc_props[i]->name) == 0) {
            return display->crtc_props[i]->prop_id;
        }
    }
    pr("crtc prop %s not found", name);
    return 0xDEADDEAD;
}

static uint32_t get_conn_property_id(const struct display* display, const char* name) {
    for (unsigned i = 0; i < display->conn_props_count; i++) {
        if (strcmp(name, display->conn_props[i]->name) == 0) {
            return display->conn_props[i]->prop_id;
        }
    }
    pr("conn prop %s not found", name);
    return 0xDEADDEAD;
}

static int drm_add_plane_property(const struct display_plane* plane, drmModeAtomicReqPtr req, const char *name, uint64_t value) {
    int ret;
    uint32_t prop_id = get_plane_property_id(plane, name);
    if (prop_id == 0xDEADDEAD) return -1;
    ret = drmModeAtomicAddProperty(req, plane->plane_id, prop_id, value);
    if (ret < 0) { pr("drmModeAtomicAddProperty (%s,%lu) failed: %d(%s)", name, value, ret, strerror(errno)); return ret; }
    return 0;
}

static int drm_add_crtc_property(const struct display* display, drmModeAtomicReqPtr req, const char *name, uint64_t value) {
    int ret;
    uint32_t prop_id = get_crtc_property_id(display, name);
    if (prop_id == 0xDEADDEAD) return -1;
    ret = drmModeAtomicAddProperty(req, display->crtc_id, prop_id, value);
    if (ret < 0) { pr("drmModeAtomicAddProperty (%s,%lu) failed: %d(%s)", name, value, ret, strerror(errno)); return ret; }
    return 0;
}

static int drm_add_conn_property(const struct display* display, drmModeAtomicReqPtr req, const char *name, uint64_t value) {
    int ret;
    uint32_t prop_id = get_conn_property_id(display, name);
    if (prop_id == 0xDEADDEAD) return -1;
    ret = drmModeAtomicAddProperty(req, display->conn_id, prop_id, value);
    if (ret < 0) { pr("drmModeAtomicAddProperty (%s,%lu) failed: %d(%s)", name, value, ret, strerror(errno)); return ret; }
    return 0;
}


// 等待下一帧 vsync
static int wait_for_next_vblank(int fd) {
    drmVBlank vblank = {};
    vblank.request.type = DRM_VBLANK_RELATIVE;
    vblank.request.sequence = 1;  // 等待下一个 vblank
    vblank.request.signal = 0;
    return drmWaitVBlank(fd, &vblank);
}

int display_commit_buffer_new(const struct display_buffer* buffer, uint32_t x, uint32_t y) {
    uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK;
    struct display_plane* plane = buffer->plane;
    struct display* display = plane->display;
    drmModeAtomicReqPtr req = NULL;

    // if (display->req)
    //     display_wait_vsync(display);

    display->req = drmModeAtomicAlloc();
    if (!display->req) {
        pr("malloc error");
        return -ENOMEM;
    }
    req = display->req;

    if (plane->first) {
        drm_add_conn_property(display, req, "CRTC_ID", display->crtc_id);
        drm_add_crtc_property(display, req, "MODE_ID", display->blob_id);
        drm_add_crtc_property(display, req, "ACTIVE", 1);
        flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
        plane->first = false;
    }
    drm_add_plane_property(plane, req, "FB_ID", buffer->id);
    drm_add_plane_property(plane, req, "CRTC_ID", display->crtc_id);
    drm_add_plane_property(plane, req, "SRC_X", 0);
    drm_add_plane_property(plane, req, "SRC_Y", 0);
    drm_add_plane_property(plane, req, "SRC_W", buffer->width << 16);
    drm_add_plane_property(plane, req, "SRC_H", buffer->height << 16);
    drm_add_plane_property(plane, req, "CRTC_X", x);
    drm_add_plane_property(plane, req, "CRTC_Y", y);
    drm_add_plane_property(plane, req, "CRTC_W", buffer->width);
    drm_add_plane_property(plane, req, "CRTC_H", buffer->height);

    if(-EBUSY == drmModeAtomicCommit(display->fd, req, flags, NULL)){
        wait_for_next_vblank(display->fd);
        CKE(drmModeAtomicCommit(display->fd, req, flags, NULL), error);
    }


    drmModeAtomicFree(req);
    display->req = NULL;
    return 0;

error:
    drmModeAtomicFree(req);
    display->req = NULL;
    return -1;
}
