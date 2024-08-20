#include "display.h"
#include <drm/drm_fourcc.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <xf86drmMode.h>
#include <xf86drm.h>
#include <drm_fourcc.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>

#define pr(fmt,...) fprintf(stderr,"[display] "fmt"\n", ##__VA_ARGS__)
#define CKE(x,go) do{int e=(x);if(e){pr("[display] "#x" error %d(%s) at line %d",e,strerror(-e),__LINE__);goto go;}}while(0)

static void page_flip_handler(int fd, unsigned int sequence, unsigned int tv_sec,
                  unsigned int tv_usec, void *user_data) {
    // do nothing
}

void display_exit(struct display* display) {
    close(display->fd);
    free(display);
}

struct display* display_init(unsigned device) {
    struct display* display;
    int flags;
    uint64_t has_dumb;
    char filename[64];
    unsigned i;
    drmModeRes *res;
    drmModeConnector *conn = NULL;
    drmModeEncoder *enc = NULL;

    display = malloc(sizeof(*display));
    display->planes = NULL;
    display->commitFlags = 0;
    display->req = NULL;

    snprintf(filename, sizeof(filename), "/dev/dri/card%u", device);
    display->fd = open(filename, O_RDWR | O_CLOEXEC);
    if (display->fd < 0) {
        perror("open /dev/dri/card error");
        free(display);
        return NULL;
    }
    if ((flags = fcntl(display->fd, F_GETFD)) < 0 || fcntl(display->fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
        perror("fcntl FD_CLOEXEC error");
        goto close;
    }
    CKE(drmGetCap(display->fd, DRM_CAP_DUMB_BUFFER, &has_dumb), close);
    CKE(has_dumb == 0, close);
    CKE(drmSetClientCap(display->fd, DRM_CLIENT_CAP_ATOMIC, 1), close);

    // find connector
    CKE((res = drmModeGetResources(display->fd)) == NULL, close);
    CKE(res->count_crtcs <= 0, free_res);
    for (i = 0; i < res->count_connectors; i++) {
        conn = drmModeGetConnector(display->fd, res->connectors[i]);
        if (!conn) {
            continue;
        }

        if (conn->connection == DRM_MODE_CONNECTED) {
            // pr("connector %d: connected", conn->connector_id);
        } else if (conn->connection == DRM_MODE_DISCONNECTED) {
            // pr("connector %d: disconnected", conn->connector_id);
        } else if (conn->connection == DRM_MODE_UNKNOWNCONNECTION) {
            // pr("connector %d: unknownconnection", conn->connector_id);
        } else {
            // pr("connector %d: unknown", conn->connector_id);
        }

        if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            display->conn = conn;
            break;
        }

        drmModeFreeConnector(conn);
        conn = NULL;
    }
    CKE(conn == NULL, free_res);
    display->conn_id = conn->connector_id;
    display->mmWidth = conn->mmWidth;
    display->mmHeight = conn->mmHeight;
    memcpy(&display->mode, conn->modes, sizeof(display->mode));

    CKE(drmModeCreatePropertyBlob(display->fd, &display->mode, sizeof(display->mode), &display->blob_id), free_con);

    // find encoder
    for (i = 0 ; i < res->count_encoders; i++) {
        enc = drmModeGetEncoder(display->fd, res->encoders[i]);
        if (!enc)
            continue;

        // pr("enc %u enc_id %d conn enc_id %d", i, enc->encoder_id, conn->encoder_id);

        if (enc->encoder_id == conn->encoder_id)
            break;

        drmModeFreeEncoder(enc);
        enc = NULL;
    }
    if (enc) {
        display->enc_id = enc->encoder_id;
        display->crtc_id = enc->crtc_id;
        drmModeFreeEncoder(enc);
    } else {
        /* Encoder hasn't been associated yet, look it up */
        for (i = 0; i < conn->count_encoders; i++) {
            int crtc, crtc_id = -1;

            enc = drmModeGetEncoder(display->fd, conn->encoders[i]);
            if (!enc)
                continue;

            for (crtc = 0 ; crtc < res->count_crtcs; crtc++) {
                uint32_t crtc_mask = 1 << crtc;

                crtc_id = res->crtcs[crtc];

                pr("enc_id %d crtc%d id %d mask %x possible %x", enc->encoder_id, crtc, crtc_id, crtc_mask, enc->possible_crtcs);

                if (enc->possible_crtcs & crtc_mask)
                    break;
            }

            if (crtc_id > 0) {
                display->enc_id = enc->encoder_id;
                pr("enc_id: %d", display->enc_id);
                display->crtc_id = crtc_id;
                pr("crtc_id: %d", display->crtc_id);
                break;
            }

            drmModeFreeEncoder(enc);
            enc = NULL;
        }

        if (!enc) {
            pr("no suitable encoder");
            goto free_con;
        }

        drmModeFreeEncoder(enc);
    }

    // pr("crtc: %u, conn: %u", display->crtc_id, display->conn_id);

    display->crtc_idx = -1;
    for (i = 0; i < res->count_crtcs; ++i) {
        if (display->crtc_id == res->crtcs[i]) {
            display->crtc_idx = i;
            break;
        }
    }

    if (display->crtc_idx == -1) {
        pr("CRTC not found");
        goto free_con;
    }

    display->width = display->mode.hdisplay;
    display->height = display->mode.vdisplay;

    display->crtc = drmModeGetCrtc(display->fd, display->crtc_id);
    if (!display->crtc) {
        pr("drmModeGetCrtc %u failed", display->crtc_id);
        goto free_con;
    }

    // get props
    drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(display->fd, display->crtc_id, DRM_MODE_OBJECT_CRTC);
    CKE(props == NULL, free_con);
    for (i = 0; (i < props->count_props) && (i < MAX_PROPS); i++) {
        display->crtc_props[i] = drmModeGetProperty(display->fd, props->props[i]);
        // pr("Added crtc prop %u:%s", display->crtc_props[i]->prop_id, display->crtc_props[i]->name);
    }
    display->crtc_props_count = props->count_props;
    drmModeFreeObjectProperties(props);
    props = drmModeObjectGetProperties(display->fd, display->conn_id, DRM_MODE_OBJECT_CONNECTOR);
    CKE(props == NULL, free_con);
    for (i = 0; (i < props->count_props) && (i < MAX_PROPS); i++) {
        display->conn_props[i] = drmModeGetProperty(display->fd, props->props[i]);
        // pr("Added conn prop %u:%s", display->conn_props[i]->prop_id, display->conn_props[i]->name);
    }
    display->conn_props_count = props->count_props;
    drmModeFreeObjectProperties(props);

    display->drm_event_ctx.version = DRM_EVENT_CONTEXT_VERSION;
    display->drm_event_ctx.page_flip_handler = page_flip_handler;

    return display;

free_con:
    drmModeFreeConnector(conn);
free_res:
    drmModeFreeResources(res);
close:
    close(display->fd);
    free(display);
    return NULL;
}

static bool check_plane_in_use(const struct display* display, uint32_t plane_id) {
    struct display_plane* plane = display->planes;
    while (plane != NULL) {
        if (plane->plane_id == plane_id) {
            return true;
        }
        plane = plane->next;
    }
    return false;
}

struct display_plane* display_get_plane(struct display* display, unsigned int fourcc) {
    drmModePlaneResPtr planes;
    drmModePlanePtr plane;
    unsigned i, j;

    planes = drmModeGetPlaneResources(display->fd);
    if (!planes) {
        pr("drmModeGetPlaneResources error");
        return NULL;
    }
    // pr("find %u planes", planes->count_planes);
    for (i = 0; i < planes->count_planes; i++) {
        plane = drmModeGetPlane(display->fd, planes->planes[i]);
        if (!plane) {
            pr("drmModeGetPlane error");
            break;
        }
        if (check_plane_in_use(display, plane->plane_id)) {
            continue;
        }
        if (!(plane->possible_crtcs & (1 << display->crtc_idx))) {
            drmModeFreePlane(plane);
            continue;
        }
        for (j = 0; j < plane->count_formats; j++) {
            // pr("plane %u format %c%c%c%c", plane->plane_id,
            //     (plane->formats[j] >> 0) & 0xff,
            //     (plane->formats[j] >> 8) & 0xff,
            //     (plane->formats[j] >> 16) & 0xff,
            //     (plane->formats[j] >> 24) & 0xff
            // );
            if (plane->formats[j] == fourcc) {
                goto found_plane;
            }
        }
        drmModeFreePlane(plane);
        plane = NULL;
    }
    drmModeFreePlaneResources(planes);
    pr("No suitable plane for %c%c%c%c", (fourcc >> 0) & 0xff, (fourcc >> 8) & 0xff, (fourcc >> 16) & 0xff, (fourcc >> 24) & 0xff);
    return NULL;

found_plane:
    drmModeFreePlaneResources(planes);

    // get props
    drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(display->fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
    if (props == NULL) {
        pr("get plane props failed");
        drmModeFreePlane(plane);
        return NULL;
    }
    struct display_plane* display_plane = malloc(sizeof(struct display_plane));
    display_plane->next = NULL;
    for (i = 0; i < props->count_props; i++) {
        display_plane->props[i] = drmModeGetProperty(display->fd, props->props[i]);
        // pr("Added plane prop %u:%s", display_plane->props[i]->prop_id, display_plane->props[i]->name);
    }
    display_plane->props_count = props->count_props;
    drmModeFreeObjectProperties(props);

    display_plane->display = display;
    display_plane->fourcc = fourcc;
    display_plane->plane_id = plane->plane_id;
    display_plane->plane = plane;
    display_plane->buffers = NULL;
    display_plane->next = display->planes;
    display->planes = display_plane;
    display_plane->first = true;

    return display_plane;
}

#define ALIGN_UP(x,a) ((x+a-1)/a*a)

struct display_buffer* display_allocate_buffer(struct display_plane* plane, uint32_t width, uint32_t height) {
    struct drm_mode_create_dumb creq;
    struct drm_mode_map_dumb mreq;
    struct drm_prime_handle prime;
    struct drm_mode_destroy_dumb destroy;
    uint32_t handles[4] = {0, 0, 0, 0}, pitches[4] = {0, 0, 0, 0}, offsets[4] = {0, 0, 0, 0};
    void* map;
    const struct display* display = plane->display;
    memset(&creq, 0, sizeof(creq));
    creq.width = width;
    creq.height = height;
    switch (plane->fourcc) {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
            creq.bpp = 8;
            creq.height = creq.height * 3 / 2;
            break;
        case DRM_FORMAT_BGR888:
        case DRM_FORMAT_RGB888:
            creq.bpp = 24;
            break;
        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_BGRA8888:
            creq.bpp = 32;
            break;
        default:
            pr("Unknown plane format fourcc %08x", plane->fourcc);
            return NULL;
    }
    CKE(drmIoctl(display->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq), error);
    // pr("create dumb handle(%u) pitch(%u) size(%llu)", creq.handle, creq.pitch, creq.size);

    // get dmabuf fd
    memset(&prime, 0, sizeof prime);
    prime.handle = creq.handle;
    CKE(ioctl(display->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime), free_dumb);

    // mmap buffer
    memset(&mreq, 0, sizeof(mreq));
    mreq.handle = creq.handle;
    CKE(drmIoctl(display->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq), free_dumb);
    map = mmap(NULL, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, display->fd, mreq.offset);
    if (map == MAP_FAILED) {
        pr("mmap dumb buffer failed: %s", strerror(errno));
        goto free_dumb;
    }

    // add dumb
    memset(map, 0, creq.size);
    handles[0] = creq.handle;
    pitches[0] = creq.pitch;
    offsets[0] = 0;
    if ((plane->fourcc == DRM_FORMAT_NV12) || (plane->fourcc == DRM_FORMAT_NV21)) {
        handles[1] = creq.handle;
        pitches[1] = pitches[0];
        offsets[1] = pitches[0] * height;
    }
    struct display_buffer* buffer = malloc(sizeof(struct display_buffer));
    CKE(drmModeAddFB2(display->fd, width, height, plane->fourcc, handles, pitches, offsets, &buffer->id, 0), munmap);
    buffer->handle = creq.handle;
    buffer->stride = creq.pitch;
    buffer->width = creq.width;
    buffer->height = height;
    buffer->size = creq.size;
    buffer->dmabuf_fd = prime.fd;
    buffer->map = map;
    buffer->plane = plane;
    buffer->next = plane->buffers;
    plane->buffers = buffer;

    return buffer;

munmap:
    free(buffer);
    munmap(map, creq.size);
free_dumb:
    memset(&destroy, 0, sizeof(destroy));
    destroy.handle = creq.handle;
    drmIoctl(display->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
error:
    return NULL;
}

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

static int drm_add_plane_property(const struct display_plane* plane, drmModeAtomicReqPtr req, const char *name, uint64_t value)
{
    int ret;
    uint32_t prop_id = get_plane_property_id(plane, name);

    if (prop_id == 0xDEADDEAD) {
        return -1;
    }

    ret = drmModeAtomicAddProperty(req, plane->plane_id, prop_id, value);
    if (ret < 0) {
        pr("drmModeAtomicAddProperty (%s,%lu) failed: %d(%s)", name, value, ret, strerror(errno));
        return ret;
    }

    return 0;
}

static int drm_add_crtc_property(const struct display* display, drmModeAtomicReqPtr req, const char *name, uint64_t value)
{
    int ret;
    uint32_t prop_id = get_crtc_property_id(display, name);

    if (prop_id == 0xDEADDEAD) {
        return -1;
    }

    ret = drmModeAtomicAddProperty(req, display->crtc_id, prop_id, value);
    if (ret < 0) {
        pr("drmModeAtomicAddProperty (%s,%lu) failed: %d(%s)", name, value, ret, strerror(errno));
        return ret;
    }

    return 0;
}

static int drm_add_conn_property(const struct display* display, drmModeAtomicReqPtr req, const char *name, uint64_t value)
{
    int ret;
    uint32_t prop_id = get_conn_property_id(display, name);

    if (prop_id == 0xDEADDEAD) {
        return -1;
    }

    ret = drmModeAtomicAddProperty(req, display->conn_id, prop_id, value);
    if (ret < 0) {
        pr("drmModeAtomicAddProperty (%s,%lu) failed: %d(%s)", name, value, ret, strerror(errno));
        return ret;
    }

    return 0;
}

int display_update_buffer(struct display_buffer* buffer, uint32_t x, uint32_t y) {
    struct display_plane* plane = buffer->plane;
    struct display* display = plane->display;
    display->commitFlags |= DRM_MODE_PAGE_FLIP_EVENT;

    if (display->req == NULL) {
        display->req = drmModeAtomicAlloc();
    }
    if (plane->first) {
        drm_add_conn_property(display, display->req, "CRTC_ID", display->crtc_id);

        drm_add_crtc_property(display, display->req, "MODE_ID", display->blob_id);
        drm_add_crtc_property(display, display->req, "ACTIVE", 1);

        display->commitFlags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
        plane->first = false;
    }
    drm_add_plane_property(plane, display->req, "FB_ID", buffer->id);
    drm_add_plane_property(plane, display->req, "CRTC_ID", display->crtc_id);
    drm_add_plane_property(plane, display->req, "SRC_X", 0);
    drm_add_plane_property(plane, display->req, "SRC_Y", 0);
    drm_add_plane_property(plane, display->req, "SRC_W", buffer->width << 16);
    drm_add_plane_property(plane, display->req, "SRC_H", buffer->height << 16);
    drm_add_plane_property(plane, display->req, "CRTC_X", x);
    drm_add_plane_property(plane, display->req, "CRTC_Y", y);
    drm_add_plane_property(plane, display->req, "CRTC_W", buffer->width);
    drm_add_plane_property(plane, display->req, "CRTC_H", buffer->height);

    return 0;
}

int display_commit(struct display* display) {
    int ret;
    if (display->req == NULL) {
        display->req = drmModeAtomicAlloc();
    }
    ret = drmModeAtomicCommit(display->fd, display->req, display->commitFlags, NULL);
    display->commitFlags = DRM_MODE_PAGE_FLIP_EVENT;
    if (ret) {
        drmModeAtomicFree(display->req);
        display->req = NULL;
        return -1;
    }
    return 0;
}

int display_commit_buffer(const struct display_buffer* buffer, uint32_t x, uint32_t y) {
    uint32_t flags = DRM_MODE_PAGE_FLIP_EVENT;
    struct display_plane* plane = buffer->plane;
    struct display* display = plane->display;
    drmModeAtomicReqPtr req = drmModeAtomicAlloc();
    display->req = req;

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

    CKE(drmModeAtomicCommit(display->fd, req, flags, NULL), error);

    return 0;

error:
    drmModeAtomicFree(req);
    return -1;
}

// not recomand, select display fd instead
void display_wait_vsync(struct display* display) {
    int ret;
    fd_set fds;

    if (display->req) {
        FD_ZERO(&fds);
        FD_SET(display->fd, &fds);

        do {
            ret = select(display->fd + 1, &fds, NULL, NULL, NULL);
        } while (ret == -1 && errno == EINTR);

        if (ret < 0) {
            pr("%s:select failed: %s",__func__,strerror(errno));
            drmModeAtomicFree(display->req);
            display->req = NULL;
            return;
        }

        if (FD_ISSET(display->fd, &fds)) {
            drmHandleEvent(display->fd, &display->drm_event_ctx);
        }

        drmModeAtomicFree(display->req);
        display->req = NULL;
    }
}

void display_handle_vsync(struct display* display) {
    drmHandleEvent(display->fd, &display->drm_event_ctx);
    drmModeAtomicFree(display->req);
    display->req = NULL;
}
